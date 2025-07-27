// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <types.h>
#include <fs/dir.h>
#include <fs/userfs.h>

list_t *userfs_points = nullptr;
pthread_mutex_t points_mutex = PTHREAD_MUTEX_INITIALIZER;

const struct fuse_lowlevel_ops fuse_userfs_ops;

constexpr mode_t file_mode = S_IFREG | 0644;
constexpr mode_t dir_mode = S_IFDIR | 0755;

bool canwrite(const char *dir) {
    char buffer[0xF1];
    const char * target = strrchr(dir, '/') ? strrchr(dir, '/') : getcwd(buffer, sizeof(buffer));
    return access(target, W_OK) == 0;
}
const char * userfs_getname(const void *fsbase, bool fullpath) {
    return fullpath ? ((const userfs_base_t*)fsbase)->fullpath : ((const userfs_base_t*)fsbase)->filename;
}
fuse_ino_t userfs_getino(const void *fsbase) {
    return ((const userfs_base_t*)fsbase)->inode_value;
}
const char *fs_filename(const char* filepath) {
    if (strrchr(filepath, '/'))
        return strrchr(filepath, '/') + 1;
    return filepath;
}
const char * subdir(const char *path) {
    const char *subpath = strrchr(path, '/');
    if (!subpath)
        subpath = path;
    else subpath++;
    return subpath;
}

userfs_dir_t * udir_create(userfs_t *userfs, const char *dirname, const userfs_dir_t *parent) {
    userfs_dir_t *dir = fb_malloc(sizeof(userfs_dir_t));
    dir->children = list_create(0);

    userfs_base_t *fsbase = (userfs_base_t*)dir;
    if (parent)
        sprintf(fsbase->fullpath, "%s/%s", userfs_getname(parent, true), dirname);
    else strcpy(fsbase->fullpath, dirname);

    fsbase->filename = subdir(fsbase->fullpath);
    fsbase->type = userfs_type_dir;
    fsbase->inode_value = userfs->next_inode++;

    if (parent) {
        fsbase->parent_inode = ((userfs_base_t*)parent)->inode_value;
        list_push(parent->children, dir);
    }
    return dir;
}
void udir_destroy(userfs_dir_t *dir) {
    for (size_t i = 0; i < list_size(dir->children); i++) {
        userfs_base_t * basefs = list_get(dir->children, i);
        if (basefs->type == userfs_type_dir)
            udir_destroy((userfs_dir_t*)basefs);
        else fb_free(basefs);
    }
    list_destroy(dir->children);
    fb_free(dir);
}

void * userfs_loop(void *userdata) {
    userfs_t *userfs = userdata;
    while (userfs->online) {
        if (fuse_session_loop(userfs->se))
            userfs->online = false;
    }
    return nullptr;
}

userfs_t *userfs_create(const char *mp) {
    if (!canwrite(mp))
        return nullptr;
    if (access(mp, R_OK)) {
        dir_t *dir;
        if ((dir = dir_open(mp, "w")))
            dir_close(dir);
    }

    userfs_t *userfs = fb_malloc(sizeof(userfs_t));
    userfs->next_inode = 1;
    userfs->online = true;
    strcpy(userfs->mountname, mp);

    userfs->buffer = fb_malloc(2 * 1024 * 1024);

    char *argv_list[] = {"./funbox"};
    struct fuse_args fuse_args = FUSE_ARGS_INIT(1, argv_list);
    userfs->se = fuse_session_new(&fuse_args, &fuse_userfs_ops, sizeof(fuse_userfs_ops), userfs);
    fuse_opt_free_args(&fuse_args);
    fuse_set_signal_handlers(userfs->se);

    pthread_mutex_lock(&points_mutex);
    if (!userfs_points)
        userfs_points = list_create(0);

    list_push(userfs_points, userfs);

    userfs->root_files = udir_create(userfs, userfs->mountname, nullptr);
    assert(fuse_session_mount(userfs->se, mp) == 0);
    pthread_mutex_unlock(&points_mutex);

    pthread_create(&userfs->thread, nullptr, userfs_loop, userfs);

    return userfs;
}

typedef struct userfs_desc {
    fuse_req_t req;
    uint8_t *buffer;
    uint8_t *next;
    size_t size;

    fuse_ino_t parent;
    off_t off;
} userfs_desc_t;

size_t pointer_dist(void *a, void *b) {
    return (uint8_t*)b - (uint8_t*)a;
}

void fillfusedir(userfs_desc_t * desc, const struct stat *file, const char *pathname) {
    const size_t offset = desc->next - desc->buffer;
    if (desc->off) {
        desc->off -= fuse_add_direntry(desc->req, NULL, 0, pathname, nullptr, 0);
        return;
    }
    desc->next += fuse_add_direntry(desc->req, (char*)desc->next, desc->size - offset, pathname, file, offset);
}

int32_t files_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    struct stat filest = { .st_ino = userfs_getino(fsbase), };
    userfs_desc_t *info_file = userdata;

    if (fsbase->parent_inode != info_file->parent)
        return 0;

    if (fsbase->type == userfs_type_file) {
        filest.st_mode = file_mode;
    }
    else if (fsbase->type == userfs_type_dir) {
        filest.st_mode = dir_mode;
    }
    fillfusedir(info_file, &filest, fs_filename(pathname));
    if (info_file->next >= info_file->buffer + info_file->size)
        return 1;
    return 0;
}
int32_t exists_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    assert(strlen(fsbase->fullpath) && pathname);
    return strcmp(userdata, fsbase->fullpath) == 0;
}
typedef typeof(exists_cb) callback_t;
userfs_base_t * walk(const userfs_dir_t *dir, const char *pdir, callback_t callback, void *userdata);

static void userfs_readdir(const fuse_req_t req, const fuse_ino_t ino, const size_t size, const off_t off, struct fuse_file_info *fi) {
    const userfs_t *userfs = list_get(userfs_points, 0);
    (void)off; (void)fi;
    userfs_desc_t readinfo = {.req = req, .buffer = userfs->buffer, .next = userfs->buffer, .size = size, .parent = ino, .off = off};

    if (!off) {
        const struct stat filest = {.st_ino = 1, .st_mode = dir_mode};
        fillfusedir(&readinfo, &filest, ".");
        fillfusedir(&readinfo, &filest, "..");
    }
    walk(userfs->root_files, "", files_cb, &readinfo);

    fuse_reply_buf(req, (char*)userfs->buffer, readinfo.next - readinfo.buffer);
}

int32_t loadattr_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    struct stat *st_info = userdata;
    assert(strlen(pathname));
    if (userfs_getino(fsbase) != st_info->st_ino)
        return 0;

    if (fsbase->type == userfs_type_file) {
        st_info->st_mode = file_mode;
        st_info->st_nlink = 1;
        st_info->st_size = fs_getsize(((userfs_file_t*)fsbase)->file);
    } else {
        st_info->st_mode = dir_mode;
        st_info->st_nlink = 2;
    }
    return 1;
}

static void userfs_getattr(const fuse_req_t req, const fuse_ino_t ino, struct fuse_file_info *fi) {
    (void)fi;
    struct stat stbuf = {.st_ino = ino};
    const userfs_t *userfs = list_get(userfs_points, 0);

    if (ino < 3) {
        stbuf.st_mode = dir_mode;
        stbuf.st_nlink = 2;
    } else {
        walk(userfs->root_files, "", loadattr_cb, &stbuf);
    }
    fuse_reply_attr(req, &stbuf, 1.0);
}

userfs_base_t* walk(const userfs_dir_t *dir, const char *pdir, callback_t callback, void *userdata) {
    if (!strlen(pdir)) {
        if (callback(userdata, (userfs_base_t*)dir, userfs_getname(dir, true)))
            return (userfs_base_t*)dir;
    }

    for (size_t i = 0; i < list_size(dir->children); i++) {
        userfs_base_t *basefs = list_get(dir->children, i);
        typeof(basefs) tempfs = nullptr;
        char *mount = strlen(pdir) ? fs_build_path(2, pdir, basefs->filename) : fb_strdup(basefs->filename);
        if (basefs->type != userfs_type_file)
            tempfs = walk((userfs_dir_t*)basefs, mount, callback, userdata);
        if (callback(userdata, basefs, mount))
            tempfs = basefs;

        fb_free(mount);
        if (tempfs)
            return tempfs;
    }
    return nullptr;
}

void udir_addfile(userfs_t *userfs, userfs_dir_t *dir, fsfile_t *file, const char *filepath) {
    userfs_file_t * file_slot = fb_malloc(sizeof(userfs_file_t));
    file_slot->file = file;

    userfs_base_t *basefs = (userfs_base_t*)file_slot;
    basefs->type = userfs_type_file;

    strcpy(basefs->fullpath, filepath ? filepath : fs_getpath(file));
    basefs->filename = fs_filename(((userfs_base_t*)file_slot)->fullpath);
    basefs->parent_inode = ((userfs_base_t*)dir)->inode_value;

    basefs->inode_value = userfs->next_inode++;

    list_push(dir->children, file_slot);
}

char * fs_lastpath(const char *path) {
    const char * last = strrchr(path, '/');
    if (!last)
        return nullptr;
    char * buffer = fb_malloc(30);
    last += 1;
    const char * middle = strchr(path, '/');
    const char * sub = middle + 1 == last ? path : middle + 1;
    fb_strcopy(buffer, sub, last - sub - 1);

    return buffer;
}

userfs_dir_t * udir_mkdir(userfs_t *userfs, userfs_dir_t *dir, const char *pathname) {
    if (strcmp(pathname, userfs_getname(dir, true)) == 0)
        return dir;

    userfs_dir_t *result = nullptr;
    for (size_t i = 0; i < list_size(dir->children); i++) {
        const userfs_base_t *basefs = list_get(dir->children, i);
        if (basefs->type == userfs_type_file)
            continue;

        const char *dirpath = userfs_getname(basefs, true);
        if (strncmp(dirpath, pathname, strlen(dirpath)) == 0)
            if ((result = udir_mkdir(userfs, (userfs_dir_t*)basefs, pathname)))
                break;
    }
    if (!result)
        result = udir_create(userfs, subdir(pathname), dir);
    return result;
}

void userfs_mountfile(userfs_t *userfs, fsfile_t *file, const char *mount) {
    if (walk(userfs->root_files, "", exists_cb, (char*)mount))
        return;

    char *buildpath = fb_malloc(100);
    *buildpath = '?';
    const char *endspath = strrchr(mount, '/');

    userfs_dir_t *target = nullptr;
    for (const char *rpath = mount; rpath < endspath; ) {
        const char *rnext = strchr(rpath, '/');
        if (!rnext)
            break;
        rnext += 1;
        const char *fmt = *buildpath == '?' ? "%s" : "/%s";
        snprintf(strchr(buildpath, '?'), *fmt != '/' ? rnext - rpath : rnext - rpath + 1, fmt, rpath);
        if (!((target = (userfs_dir_t*)walk(userfs->root_files, "", exists_cb, buildpath))))
            if (!((target = udir_mkdir(userfs, userfs->root_files, buildpath))))
                break;
        *(buildpath + strlen(buildpath)) = '?';
        rpath = rnext;
    }
    if (target)
        udir_addfile(userfs, target, file, mount);
    fb_free(buildpath);
}

void userfs_destroy(userfs_t *userfs) {
    userfs->online = false;
    fuse_session_exit(userfs->se);
    pthread_join(userfs->thread, nullptr);

    pthread_mutex_lock(&points_mutex);
    fuse_session_unmount(userfs->se);
    fuse_session_destroy(userfs->se);

    for (size_t i = 0; i < list_size(userfs_points); i++) {
        if (list_get(userfs_points, i) != userfs)
            continue;
        list_drop(userfs_points, i);
        break;
    }
    udir_destroy(userfs->root_files);
    if (!list_size(userfs_points)) {
        list_destroy(userfs_points);
        userfs_points = nullptr;
    }

    fb_free(userfs->buffer);
    fb_free(userfs);
    pthread_mutex_unlock(&points_mutex);
}

int32_t getinode_cb(void *userdata, const userfs_base_t *basefs, const char *pathname) {
    (void)pathname;
    if (basefs->inode_value == *(fuse_ino_t*)userdata)
        return 1;
    return 0;
}

#define USERFS_OPEN_INODE(req, ino, fi, _type, error)\
    do {\
        const userfs_t *userfs = list_get(userfs_points, 0);\
        const userfs_base_t *basefs = walk(userfs->root_files, "", getinode_cb, &ino);\
        if (!basefs)\
            fuse_reply_err(req, EBADF);\
        else if (basefs->type == _type)\
            fuse_reply_open(req, fi);\
        else\
            fuse_reply_err(req, error);\
    } while (0)


void userfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    USERFS_OPEN_INODE(req, ino, fi, userfs_type_file, EISDIR);
}

void userfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    USERFS_OPEN_INODE(req, ino, fi, userfs_type_dir, ENOTDIR);
}

typedef struct lookup_inode {
    struct fuse_entry_param entry;
    const char *target;
    fuse_ino_t parent;
    bool filled;
} lookup_inode_t;

static int32_t fillentry_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    lookup_inode_t *lookup = userdata;
    const char * filename = fs_filename(pathname);

    if (strcmp(filename, lookup->target) && lookup->parent != fsbase->parent_inode)
        return 0;

    lookup->entry.ino = fsbase->inode_value;
    lookup->entry.attr_timeout = 1.0; lookup->entry.entry_timeout = 1.0;
    lookup->entry.attr.st_nlink = 1;
    if (fsbase->type == userfs_type_file) {
        const userfs_file_t *file = (userfs_file_t*)fsbase;
        lookup->entry.attr.st_mode = file_mode;
        lookup->entry.attr.st_size = fs_getsize(file->file);
    }
    else lookup->entry.attr.st_mode = dir_mode;

    lookup->filled = true;
    return 1;
}

void userfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
    lookup_inode_t lookup = { .target = name, .parent = parent };
    memset(&lookup.entry, 0, sizeof(lookup.entry));

    const userfs_t *userfs = list_get(userfs_points, 0);
    walk(userfs->root_files, "", fillentry_cb, &lookup);

    (void)parent;
    if (lookup.filled)
        fuse_reply_entry(req, &lookup.entry);
    else fuse_reply_err(req, ENOENT);

}

void userfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
    (void)fi;
    const userfs_t *userfs = list_get(userfs_points, 0);
    userfs_base_t *basefs = walk(userfs->root_files, "", getinode_cb, &ino);
    if (!basefs) {
        fuse_reply_err(req, EBADF);
        return;
    }
    if (basefs->type != userfs_type_file) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }
    const userfs_file_t *file = (userfs_file_t*)basefs;
    fs_read(file->file, userfs->buffer, size, off);
    fuse_reply_buf(req, (char*)userfs->buffer, size);

}

const struct fuse_lowlevel_ops fuse_userfs_ops = {
    .readdir = userfs_readdir,
    .getattr = userfs_getattr,
    .open = userfs_open,
    .opendir = userfs_opendir,
    .lookup = userfs_lookup,
    .read = userfs_read
};

