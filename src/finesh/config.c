#include <config.h>
#include <stdlib.h>

config_t * config_create() {
    config_t *config = funbox_malloc(sizeof(config_t));
    config->procinfo = procinfo_create();
    config->package_filepath = nullptr;
    return config;
}
void config_destroy(config_t *config) {
    procinfo_destroy(config->procinfo);
    funbox_free(config);
}
