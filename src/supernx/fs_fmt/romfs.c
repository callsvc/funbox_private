#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <fs/mapfile.h>
#include <fs_fmt/offset_file.h>
#include <fs_fmt/romfs.h>

#include <types.h>

#pragma pack(push, 1)
typedef struct type_header_info {
    uint64_t ht_offset;
    uint64_t ht_size;
    uint64_t metadata_offset;
    uint64_t metadata_size;
} type_header_info_t;
typedef struct level3_header {
    uint64_t size;
    type_header_info_t directory;
    type_header_info_t file;
    uint64_t data_offset;
} romfs_header_t;

_Static_assert(sizeof(romfs_header_t) == 80);

typedef struct romfs_directory {
    uint32_t parent_offset; // self if root
    uint32_t sibling_offset;
    uint32_t subdir_offset;
    uint32_t file_offset;

    uint32_t hash; // in the same hash table bucket
    uint32_t dir_namesize;
    char dirname[];
} romfs_directory_t;

typedef struct romfs_file {
    uint32_t parent_offset;
    uint32_t sibling_offset;

    uint64_t filedata_offset;
    uint64_t filesize;

    uint32_t hash;
    uint32_t file_namesize;
    char filename[];
} romfs_file_t;

#pragma pack(pop)

constexpr uint64_t romfs_inval = 0xFFFFFFFF;
void rfs_fillpath(const romfs_t *rfs, const size_t size, const char *path, const bool dir) {
    char *pathout = strchr(rfs->pathbuild, '?');
    memcpy(pathout, path, size);
    if (dir) memcpy(pathout + size, "/?\0", 3);
    else pathout[size] = '\0';
}
void rfs_cleanpath(romfs_t *rfs, const bool dir) {
    char * last = strrchr(rfs->pathbuild, '/');
    if (last && dir) *last = '\0';
    else if (last) memcpy(last, "/?\0", 3);
    else memcpy(rfs->pathbuild, "?\0", 2);
    if (dir)
        rfs_cleanpath(rfs, false);
}

typedef struct romfs_file_node {
    fsfile_t *high_file;
    char pathname[180];
    size_t offset;
    size_t filesize;
} romfs_file_node_t;

const char * file_node_path(const romfs_file_node_t * file_node) {
    if (file_node->high_file)
        return fs_getpath(file_node->high_file);
    return file_node->pathname;
}

void romfs_addfile_2(const romfs_t *rfs, const romfs_file_node_t *file_ent, bool verify) {
    const size_t size = list_size(rfs->files);
    for (size_t i = 0; i < size && verify; i++) {
        const romfs_file_node_t *file = list_get(rfs->files, i);
        if (strcmp(file_node_path(file), file_node_path(file_ent)) == 0)
            quit("file %s already exists inside this romfs", file_node_path(file));
    }

    // printf("\t- %s\n", file_node_path(file_ent));
    memcpy(list_emplace(rfs->files), file_ent, sizeof(romfs_file_node_t));
}
void romfs_addfile(const romfs_t *rfs, fsfile_t *file) {
    const romfs_file_node_t file_ent = {.high_file = file};
    romfs_addfile_2(rfs, &file_ent, true);
}

void rfs_getallfiles(romfs_t *rfs, uint32_t file_offset) {
    do {
        const romfs_file_t *file = (romfs_file_t*)&rfs->metafiles[file_offset];
        rfs_fillpath(rfs, file->file_namesize, file->filename, false);

        romfs_file_node_t file_ent = { .offset = rfs->data_offset + file->filedata_offset, .filesize = file->filesize };
        strcpy(file_ent.pathname, rfs->pathbuild);
#if 0
        uint8_t buffer[1024];
        fs_read((fsfile_t*)rfsfile, buffer, MIN(fs_getsize((fsfile_t*)rfsfile), sizeof(buffer)), 0);
#endif
        romfs_addfile_2(rfs, &file_ent, false);

        rfs_cleanpath(rfs, false);
        file_offset = file->sibling_offset;
    } while (file_offset != romfs_inval);
}
void rfs_getreaddir(romfs_t *rfs, uint32_t dir_offset) {
    do {
        const romfs_directory_t *dir = (romfs_directory_t*)&rfs->metadirs[dir_offset];
        if (dir->dir_namesize) {
            rfs_fillpath(rfs, dir->dir_namesize, dir->dirname, true);
        }

        if (dir->subdir_offset != romfs_inval) {
            rfs_getreaddir(rfs, dir->subdir_offset);
        }
        if (dir->file_offset != romfs_inval) {
            rfs_getallfiles(rfs, dir->file_offset);
        }

        rfs_cleanpath(rfs, true);
        dir_offset = dir->sibling_offset;
    } while (dir_offset != romfs_inval);
}

typedef struct file_override {
    const char *path; // path to override
    romfs_file_node_t * romfs_file; // file to be overridden
    fsfile_t *file; // actual file, could be anything
} file_override_t;

offset_file_t * romfs_open_file(const romfs_t *rfs, const char * path, const char *mode) {
    assert(mode && *mode == 'r');

    for (size_t i = 0; i < list_size(rfs->override_files); i++) {
        const file_override_t * file_over = list_get(rfs->override_files, i);
        if (strcmp(file_over->path, path) == 0)
            return (offset_file_t*)file_over->file;
    }

    for (size_t i = 0; i < list_size(rfs->files); i++) {
        if (strcmp(file_node_path(list_get(rfs->files, i)), path) != 0)
            continue;
        const romfs_file_node_t * file = list_get(rfs->files, i);
        if (file->high_file)
            return (offset_file_t*)file->high_file;
        return offset_file_open(rfs->basefile, file->pathname, file->filesize, file->offset, false);
    }
    return nullptr;
}
void fs_romfs_close_file(fsdir_t *parent, fsfile_t *file) {
    const romfs_t *rfs = (romfs_t*)parent;
    if (file->type == file_type_offsetfile)
        offset_file_close(rfs->basefile, (offset_file_t*)file);
    else if (file->type == file_type_mapfile)
        mapfile_close((mapfile_t*)file);

    for (size_t i = 0; i < list_size(rfs->files); i++)
        if (((romfs_file_node_t*)list_get(rfs->files, i))->high_file == file)
            list_drop(rfs->files, i);
}

fsfile_t * fs_romfs_open_file(struct fsdir *dir, const char *path, const char *mode) {
    return (fsfile_t*)romfs_open_file((romfs_t*)dir, path, mode);
}

romfs_t * romfs_create_2() {
    romfs_t *rfs = fb_malloc(sizeof(romfs_t));
    rfs->files = list_create(sizeof(romfs_file_node_t));

    rfs->override_files = list_create(sizeof(file_override_t));
    rfs->vdir.open_file = fs_romfs_open_file;
    rfs->vdir.close_file = fs_romfs_close_file;
    return rfs;
}

vector_t * fs_romfs_list_all_files(const struct fsdir *dir) {
    const romfs_t * rfs = (romfs_t*)dir;
    vector_t * files = vector_create(0, 0);

    for (size_t i = 0; i < list_size(rfs->files); i++)
        vector_emplace(files, file_node_path(list_get(rfs->files, i)));

    return files;
}

romfs_t *romfs_create(fsfile_t *file) {
    romfs_t *rfs = fb_malloc(sizeof(romfs_t));
    rfs->basefile = file;
    rfs->files = list_create(sizeof(romfs_file_node_t));
    rfs->override_files = list_create(sizeof(file_override_t));

    rfs->vdir.open_file = fs_romfs_open_file;
    rfs->vdir.close_file = fs_romfs_close_file;
    rfs->vdir.fs_list_all_files = fs_romfs_list_all_files;

    fprintf(stderr, "files in this romfs: \n");
    romfs_header_t header;
    fs_read(file, &header, sizeof(header), 0);
    assert(header.size == sizeof(romfs_header_t));
    rfs->data_offset = header.data_offset;

    rfs->metadirs = fb_malloc(header.directory.metadata_size);
    fs_read(rfs->basefile, rfs->metadirs, header.directory.metadata_size, header.directory.metadata_offset);

    rfs->metafiles = fb_malloc(header.file.metadata_size);
    fs_read(rfs->basefile, rfs->metafiles, header.file.metadata_size, header.file.metadata_offset);

    *rfs->pathbuild = '?';
    rfs_getreaddir(rfs, 0);
    fb_free(rfs->metadirs); fb_free(rfs->metafiles);
    rfs->metadirs = nullptr;
    rfs->metafiles = nullptr;

    return rfs;
}

size_t romfs_override(const romfs_t *rfs, fsfile_t *file, const char *path) {
    for (size_t i = 0; i < list_size(rfs->files); i++) {
        romfs_file_node_t *romfs_file = list_get(rfs->files, i);
        if (strcmp(fs_getpath(romfs_file), path) != 0)
            continue;

        file_override_t fileover = {
            .path = path, .file = file, .romfs_file = romfs_file
        };
        list_push(rfs->override_files, &fileover);
        return i;
    }
    return 0;
}

void romfs_destroy(romfs_t *rfs) {

    for (size_t i = 0; i < list_size(rfs->files); i++) {
        const romfs_file_node_t *file = list_get(rfs->files, i);
        if (!file->high_file)
            continue;
        fs_close_file((fsdir_t*)rfs, file->high_file);
    }
    list_destroy(rfs->files); list_destroy(rfs->override_files);
    fb_free(rfs);
}
