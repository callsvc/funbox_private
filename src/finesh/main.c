#include <stdio.h>

#include <bundle/pkglist.h>
#include <types.h>

int main() {
    envos_issafe();
    fb_create();

    config_t *config = config_create();
    pkg_list_all(config);

    config_destroy(config);

    puts("Tosh");
    return 0;
}
