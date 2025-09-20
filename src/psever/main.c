#include <stdio.h>

#include <pkg_loader.h>

int main() {
    file_t *pkgfile = file_open("GAME.pkg", "r", false);
    if (!pkgfile)
        return -1;

    uint8_t pkg_header[0x1000];
    file_read(pkgfile, pkg_header, 0x1000, 0);

    pkg_loader_t * loader = pkg_create((fsfile_t*)pkgfile);

    puts("Hello world!");

    pkg_destroy(loader);
    file_close(pkgfile);
}
