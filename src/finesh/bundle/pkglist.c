#include <config.h>

#include <fs/types.h>

void pkg_list_all(const config_t *config) {
    const char* workdir = config->procinfo->proc_cwd;
    vector_t *files = list_all_files(workdir);

    vector_destroy(files);
}