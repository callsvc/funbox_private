#include <config.h>
#include <string.h>

#include <fs/types.h>
#include <fs/file.h>

void pkg_list_all(const config_t *config) {
    const char* workdir = config->procinfo->current_dir;
    vector_t *files = list_all_files(workdir);

    for (size_t i = 0; i < vector_size(files); i++) {
        const char * path = vector_get(files, i);
        if (strcmp(path + strlen(path) - 4, ".ipa"))
            continue;

        file_t *file = file_open(path, "r");
        uint32_t buffer;
        file_read(file, &buffer, 4, 0);

        constexpr uint32_t pkzip = 0x4034B50;
        if (buffer == pkzip)
            vector_emplace(config->ipa_array, path);

        file_close(file);
    }

    vector_destroy(files);
}
