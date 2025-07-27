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
const char * usefs_getname(void *fsbase) {
    return ((userfs_base_t*)fsbase)->mpath;
}
ino_t usefs_getino(void *fsbase) {
    return ((userfs_base_t*)fsbase)->inode_value;
}

userfs_dir_t * udir_create(userfs_t *userfs, const char *name, const userfs_dir_t *parent) {
    userfs_dir_t *dir = fb_malloc(sizeof(userfs_dir_t));
    dir->children = list_create(0);

    strcpy(((userfs_base_t*)dir)->mpath, name);
    ((userfs_base_t*)dir)->type = userfs_type_dir;
    ((userfs_base_t*)dir)->inode_value = userfs->next_inode++;

    if (parent)
        list_push(parent->children, dir);
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
    userfs->next_inode = 3;
    userfs->online = true;
    strcpy(userfs->mountname, mp);

    userfs->buffer = fb_malloc(2 * 1024 * 1024);

    char *argv_list[] = {"./funbox"};
    struct fuse_args fuse_args = FUSE_ARGS_INIT(1, argv_list);
    userfs->se = fuse_session_new(&fuse_args, &fuse_userfs_ops, sizeof(fuse_userfs_ops), userfs);
    fuse_opt_free_args(&fuse_args);

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
    size_t size;
    uint8_t *next;
} userfs_desc_t;

size_t pointer_dist(void *a, void *b) {
    return (uint8_t*)b - (uint8_t*)a;
}

void fillfusedir(userfs_desc_t * desc, const struct stat *file, const char *pathname) {
    const size_t pos = desc->next - desc->buffer;
    desc->next += fuse_add_direntry(desc->req, (char*)desc->next, desc->size - pos, pathname, file, pos);
}

int32_t files_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    struct stat filest = { .st_ino = usefs_getino((void*)fsbase), };
    if (fsbase->type == userfs_type_file)
        filest.st_mode = file_mode;
    else if (fsbase->type == userfs_type_dir)
        filest.st_mode = dir_mode;

    fillfusedir(userdata, &filest, pathname);
    return 0;
}

int32_t exists_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    assert(strlen(fsbase->mpath));
    return strcmp(userdata, pathname) == 0;
}
typedef typeof(exists_cb) callback_t;
int32_t walk(const userfs_dir_t *dir, const char *pdir, callback_t callback, void *userdata);

static void userfs_readdir(const fuse_req_t req, const fuse_ino_t ino, const size_t size, const off_t off, struct fuse_file_info *fi) {
    const userfs_t *userfs = list_get(userfs_points, 0);
    (void)off; (void)fi;
    memset(userfs->buffer, 0, 2 * 1024 * 1024);
    userfs_desc_t dirsdesc = {.req = req, .buffer = userfs->buffer, .next = userfs->buffer, .size = size};

    if (ino != 1) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }
    const struct stat filest = {.st_ino = 1, .st_mode = dir_mode};
    fillfusedir(&dirsdesc, &filest, ".");
    fillfusedir(&dirsdesc, &filest, "..");

    walk(userfs->root_files, "", files_cb, &dirsdesc);

    fuse_reply_buf(req, (char*)userfs->buffer, dirsdesc.next - dirsdesc.buffer);
}

int32_t loadattr_cb(void *userdata, const userfs_base_t *fsbase, const char *pathname) {
    struct stat *st_info = userdata;
    assert(strlen(pathname));
    st_info->st_ino = usefs_getino((void*)fsbase);

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
    struct stat stbuf = {};
    (void)fi;

    stbuf.st_ino = ino;
    if (ino > 3) {
        const userfs_t *userfs = list_get(userfs_points, 0);
        walk(userfs->root_files, "", loadattr_cb, &stbuf);
    } else {
        stbuf.st_mode = dir_mode;
        stbuf.st_nlink = 2;
    }
    if (!stbuf.st_mode) {
        fuse_reply_err(req, ENOENT);
        return;
    }
    fuse_reply_attr(req, &stbuf, 1.0);
}

int32_t walk(const userfs_dir_t *dir, const char *pdir, callback_t callback, void *userdata) {
    int32_t result = 0;
    for (size_t i = 0; i < list_size(dir->children); i++) {
        userfs_base_t *basefs = list_get(dir->children, i);
        char *mount = strlen(pdir) ? fs_build_path(2, pdir, basefs->mpath) : fb_strdup(basefs->mpath);
        if ((result = callback(userdata, basefs, mount)))
            return result;
        if (basefs->type != userfs_type_file)
            walk((userfs_dir_t*)basefs, mount, callback, userdata);
        fb_free(mount);
    }
    return result;
}

void udir_addfile(userfs_t *userfs, userfs_dir_t *dir, fsfile_t *file, const char *filename) {
    userfs_file_t * file_slot = fb_malloc(sizeof(userfs_file_t));
    file_slot->file = file;
    ((userfs_base_t*)file_slot)->type = userfs_type_file;

    strcpy(((userfs_base_t*)file_slot)->mpath, filename ? filename : fs_getpath(file));
    file_slot->parent_inode = ((userfs_base_t*)dir)->inode_value;

    ((userfs_base_t*)file_slot)->inode_value = userfs->next_inode++;

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

const char * subdir(const char *path) {
    const char *subpath = strrchr(path, '/');
    if (!subpath)
        subpath = path;
    else subpath++;
    return subpath;
}

userfs_dir_t * udir_mkdir(userfs_t *userfs, userfs_dir_t *dir, const char *pathname) {
    const char * path = subdir(pathname);
    bool this_level = true;
    if (strrchr(pathname, '/')) {
        char * lastpath = fs_lastpath(pathname);
        this_level = strcmp(usefs_getname(dir), lastpath) == 0;
        fb_free(lastpath);
    }
    if (this_level && strcmp(usefs_getname(dir), pathname) == 0)
        return dir;

    userfs_dir_t *result = nullptr;

    for (size_t i = 0; i < list_size(dir->children); i++) {
        const userfs_base_t *basefs = list_get(dir->children, i);
        if (basefs->type == userfs_type_file)
            continue;

        const char *subpath = subdir(path);
        if (!this_level)
            if ((result = udir_mkdir(userfs, (userfs_dir_t*)basefs, subpath)))
                break;
    }
    if (this_level && !result)
        result = udir_create(userfs, path, dir);
    return result;
}

const char *fs_filename(const char* filepath) {
    if (strchr(filepath, '/'))
        return strrchr(filepath, '/') + 1;
    return filepath;
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
        if ((target = udir_mkdir(userfs, userfs->root_files, buildpath)) == nullptr)
            break;
        *(buildpath + strlen(buildpath)) = '?';
        rpath = rnext;
    }

    udir_addfile(userfs, target, file, fs_filename(mount));
    fb_free(buildpath);
}

void userfs_destroy(userfs_t *userfs) {
    userfs->online = false;
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

void userfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    if (ino < 2)
        fuse_reply_err(req, EISDIR);
    else
        fuse_reply_open(req, fi);
}
void userfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    if (ino < 2)
        fuse_reply_err(req, ENOTDIR);
    else
        fuse_reply_open(req, fi);
}

const struct fuse_lowlevel_ops fuse_userfs_ops = {
    .readdir = userfs_readdir,
    .getattr = userfs_getattr,
};

