#include <assert.h>
#include <stdio.h>
#include <string.h>

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
} level3_header_t;

_Static_assert(sizeof(level3_header_t) == 80);

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

constexpr uint64_t rfs_inv = 0xFFFFFFFF;
void rfs_fillpath(const romfs_t *rfs, const size_t size, const char *path, const bool dir) {
    char *pathout = strchr(rfs->pathbuild, '?');
    memcpy(pathout, path, size);
    if (dir)
        memcpy(pathout + size, "/?\0", 3);
    else pathout[size] = '\0';
}
void rfs_cleanpath(romfs_t *rfs, const bool dir) {
    char * last = strrchr(rfs->pathbuild, '/');
    if (last && dir)
        *last = '\0';
    else if (last)
        memcpy(last, "/?\0", 3);
    else
        memcpy(rfs->pathbuild, "?\0", 2);

    if (dir)
        rfs_cleanpath(rfs, false);
}

void romfs_addfile(const romfs_t *rfs, fsfile_t *file) {
    for (size_t i = 0; i < list_size(rfs->files); i++)
        if (strcmp(fs_getpath(list_get(rfs->files, i)), fs_getpath(file)) == 0)
            quit("file %s already exists inside this romfs", fs_getpath(file));

    fprintf(stderr, "\t- %s\n", fs_getpath(file));
    list_push(rfs->files, file);
}

void rfs_getallfiles(romfs_t *rfs, uint32_t file_offset) {
    do {
        const romfs_file_t *file = (romfs_file_t*)&rfs->metafiles[file_offset];
        rfs_fillpath(rfs, file->file_namesize, file->filename, false);

        offset_file_t *rfsfile = offset_file_open(rfs->basefile, rfs->pathbuild, file->filesize, rfs->data_offset + file->filedata_offset, false);
#if 0
        uint8_t buffer[1024];
        fs_read((fsfile_t*)rfsfile, buffer, MIN(fs_getsize((fsfile_t*)rfsfile), sizeof(buffer)), 0);
#endif
        romfs_addfile(rfs, (fsfile_t*)rfsfile);

        rfs_cleanpath(rfs, false);
        file_offset = file->sibling_offset;
    } while (file_offset != rfs_inv);
}
void rfs_getreaddir(romfs_t *rfs, uint32_t dir_offset) {
    do {
        const romfs_directory_t *dir = (romfs_directory_t*)&rfs->metadirs[dir_offset];
        if (dir->dir_namesize) {
            rfs_fillpath(rfs, dir->dir_namesize, dir->dirname, true);
        }

        if (dir->subdir_offset != rfs_inv) {
            rfs_getreaddir(rfs, dir->subdir_offset);
        }
        if (dir->file_offset != rfs_inv) {
            rfs_getallfiles(rfs, dir->file_offset);
        }

        rfs_cleanpath(rfs, true);
        dir_offset = dir->sibling_offset;
    } while (dir_offset != rfs_inv);
}

typedef struct file_override {
    const char *path; // path to override
    fsfile_t * romfs_file; // file to be overridden
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
        if (strcmp(fs_getpath(list_get(rfs->files, i)), path) == 0)
            return list_get(rfs->files, i);
    }
    return nullptr;
}
fsfile_t * fs_romfs_open_file(struct fsdir *dir, const char *path, const char *mode) {
    return (fsfile_t*)romfs_open_file((romfs_t*)dir, path, mode);
}

romfs_t * romfs_create_2() {
    romfs_t *rfs = fb_malloc(sizeof(romfs_t));
    rfs->files = list_create(0);

    rfs->files = list_create(0);
    rfs->override_files = list_create(sizeof(file_override_t));
    rfs->vdir.open_file = fs_romfs_open_file;
    return rfs;
}

romfs_t *romfs_create(fsfile_t *file) {
    romfs_t *rfs = fb_malloc(sizeof(romfs_t));
    rfs->basefile = file;
    rfs->files = list_create(0);
    rfs->override_files = list_create(sizeof(file_override_t));

    rfs->vdir.open_file = fs_romfs_open_file;

    fprintf(stderr, "files in this romfs: \n");
    level3_header_t header;
    fs_read(file, &header, sizeof(header), 0);
    assert(header.size == sizeof(level3_header_t));
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
        fsfile_t *romfs_file = list_get(rfs->files, i);
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
        offset_file_t *file = list_get(rfs->files, i);
        offset_file_close(rfs->basefile, file);
    }
    list_destroy(rfs->files); list_destroy(rfs->override_files);
    fb_free(rfs);
}
