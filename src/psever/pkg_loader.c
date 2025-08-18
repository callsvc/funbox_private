#include <types.h>

#include <pfs.h>
#include <pkg_loader.h>
struct pkg_table_entry {
    uint32_t id, filename_offset;
    uint32_t flags1, flags2;
    uint32_t offset;
    uint32_t size;
    uint64_t pad0; // // blank padding
};

#define PSEVER_PKG_MAGIC 0x7F434E54
void pkg_get_files(__attribute__((unused)) const pkg_loader_t *loader, const pkg_header_t *pkg_header, fsfile_t *file) {
    const size_t count = to_little32(&pkg_header->pkg_file_count);
    const size_t table = to_little32(&pkg_header->pkg_table_offset);

    for (size_t i = 0; i < count; i++) {
        struct pkg_table_entry pkg_entry;
        fs_read(file, &pkg_entry, sizeof(pkg_entry), table + sizeof(pkg_entry) * i);
        fprintf(stderr, "file id : %u\n", to_little32(&pkg_entry.id));
    }
    pfs_t *pfs = pfs_create(file);
    pfs_destroy(pfs);
}

pkg_loader_t * pkg_create(fsfile_t *file) {
    pkg_loader_t * loader = fb_malloc(sizeof(pkg_loader_t));

    pkg_header_t pkg_header;
    fs_read(file, &pkg_header, sizeof(pkg_header), 0);

    if (to_little32(&pkg_header.pkg_magic) != PSEVER_PKG_MAGIC)
        quit("not a pkg file");

    fprintf(stderr, "pkg file type: %u\n", to_little32(&pkg_header.pkg_type));
    pkg_get_files(loader, &pkg_header, file);
    return loader;
}
void pkg_destroy(pkg_loader_t *loader) {
    fb_free(loader);
}