#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <types.h>

#include <fs_fmt/pfs.h>
#include <fs_fmt/offset_file.h>
bool pfs_is_pfs(fsfile_t *file) {
    const char pfsmagic[] = "PFS0";
    uint8_t buffer[strlen(pfsmagic) + sizeof(int)];
    fs_read(file, buffer, sizeof(buffer), 0);

    if (memcmp(pfsmagic, buffer, strlen(pfsmagic)) == 0)
        return (int32_t*)(buffer + sizeof(buffer) - 4) > 0;
    return false;
}

struct pfs_header {
    uint32_t magic; // Magic value, should be "PFS0"
    uint32_t entry_count; // Count of entries in this PFS
    uint32_t str_table; // Contains all filename and paths strings for all resident files
    uint32_t _pad0;
};

struct partition_entry {
    uint64_t offset;
    uint64_t size;
    uint32_t filename_offset;
    uint32_t _pad0;
};

void pfs_validate(const pfs_t *pfs, const uint64_t file_size) {
    const uint64_t max_size = 1ULL << 48;
    assert(file_size < max_size);

    size_t files_sizes = 0;
    for (size_t i = 0; i < vector_size(pfs->files); i++) {
        files_sizes += ((pfs_file_t*)vector_get(pfs->files, i))->size;
    }

    fprintf(stderr, "number of bytes discarded from this PFS: %lu\n", file_size - files_sizes);
    if (files_sizes >= max_size || files_sizes > file_size)
        oskill("PFS is too large");
}

void pfs_getall(const pfs_t *pfs, fsfile_t * file, const uint64_t tableoffset, size_t *offset, const struct pfs_header *pfs_header) {
    char *strtable = funbox_malloc(pfs_header->str_table);
    fs_read(file, strtable, pfs_header->str_table, tableoffset);
    const size_t filedata = tableoffset + pfs_header->str_table;

    for (size_t i = 0; i < pfs_header->entry_count; i++) {
        struct partition_entry pfs_entry;
        fs_read(file, &pfs_entry, sizeof(pfs_entry), *offset);
        pfs_file_t pfs_file = {
            .offset = filedata + pfs_entry.offset,
            .size = pfs_entry.size,
        };
        strcpy(pfs_file.filename, strtable + pfs_entry.filename_offset);

        *offset += sizeof(pfs_entry);

        vector_emplace(pfs->files, &pfs_file);
    }
    funbox_free(strtable);
}

fsfile_t * fs_pfs_open_file(fsdir_t *dir, const char * path, const char * mode) {
    const pfs_t * pfs = (pfs_t*)dir;
    if (*mode == 'w')
        oskill("unable to open file for writing");
    for (size_t i = 0; i < vector_size(pfs->files); i++) {
        const pfs_file_t * file = vector_get(pfs->files, i);

        if (strcmp(file->filename, path) != 0)
            continue;

        return (fsfile_t*)offset_file_open(pfs->basefio, file->filename, file->size, file->offset);
    }
    return NULL;
}

void fs_pfs_close_file(fsdir_t *dir, fsfile_t *file) {
    const pfs_t * pfs = (pfs_t*)dir;
    offset_file_close(pfs->basefio, (offset_file_t*)file);
}

vector_t *fs_pfs_list_all_files(const fsdir_t *dir) {
    pfs_t * pfs = (pfs_t*)dir;
    if (pfs->files_paths)
        return vector_clone(pfs->files_paths);

    vector_t * result = vector_create(0, 0);
    for (size_t i = 0; i < vector_size(pfs->files); i++)
        vector_emplace(result, ((pfs_file_t*)vector_get(pfs->files, i))->filename);

    pfs->files_paths = vector_clone(result);
    return result;
}

pfs_t * pfs_create(fsfile_t *file) {
    pfs_t * pfs = funbox_malloc(sizeof(pfs_t));
    pfs->basefio = file;

    strcpy(pfs->vdir.path, fs_getpath(file));

    strcpy(pfs->vdir.mode, "r");
    pfs->vdir.open_file = fs_pfs_open_file;
    pfs->vdir.close_file = fs_pfs_close_file;
    pfs->vdir.fs_list_all_files = fs_pfs_list_all_files;

    struct pfs_header pfs_header;
    fs_read(file, &pfs_header, sizeof(pfs_header), 0);
    size_t offset = sizeof(pfs_header);

    pfs->files = vector_create(pfs_header.entry_count, sizeof(pfs_file_t));

    const size_t tableoffset = offset + sizeof(struct partition_entry) * pfs_header.entry_count;
    pfs_getall(pfs, file, tableoffset, &offset, &pfs_header);

    pfs_validate(pfs, fs_getsize(file));
    return pfs;
}

void pfs_print_files(const pfs_t *pfs) {
    fputs("content of PFS0: ", stderr);
    for (size_t i = 0; i < vector_size(pfs->files); i++) {
        pfs_file_t * file = vector_get(pfs->files, i);
        fprintf(stderr, "%s (size : %#lX) ", file->filename, file->size);
    }
    fputs("\n", stderr);
}

void pfs_destroy(pfs_t *pfs) {
    vector_destroy(pfs->files_paths);
    vector_destroy(pfs->files);
    funbox_free(pfs);
}
