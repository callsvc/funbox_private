#include <config.h>

config_t * config_create() {
    config_t *config = fb_malloc(sizeof(config_t));
    config->procinfo = procinfo_create();
    config->package_filepath = nullptr;
    config->ipa_array = vector_create(0, 0);

    return config;
}
void config_destroy(config_t *config) {
    procinfo_destroy(config->procinfo);
    vector_destroy(config->ipa_array);
    fb_free(config);
}
