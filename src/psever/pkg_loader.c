#include <types.h>
#include <pkg_loader.h>

pkg_loader_t * pkg_create(fsfile_t *file) {
    pkg_loader_t * loader = fb_malloc(sizeof(pkg_loader_t));

    fs_read(file, &loader->file_header, sizeof(loader->file_header), 0);

    fprintf(stderr, "pkg file type: %u\n", big32(&loader->file_header.pkg_type));
    return loader;
}
void pkg_destroy(pkg_loader_t *loader) {
    fb_free(loader);
}