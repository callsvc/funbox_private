#include <stdio.h>

#include <bundle/pkglist.h>
#include <bundle/loader.h>

int main() {
    config_t *config = config_create();
    pkg_list_all(config);

    if (!vector_empty(config->ipa_array)) {
        loader_t * loader = loader_create(vector_get(config->ipa_array, 0));
        loader_destroy(loader);
    }
    config_destroy(config);

    puts("Tosh");
    return 0;
}
