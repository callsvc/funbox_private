#include <stdlib.h>
#include <horizon/hos.h>

#include <types.h>
#include <fs/dir.h>
#include <zip_fs/zip.h>
#include <fs_fmt/content_archive.h>

#include <hle_services/svt/title_ver.h>

struct service_descriptor {
    service_load_func_t callback;
    char* program_id;
};
struct service_descriptor services_list[] = {
    {svt_load, "0100000000000809ULL"},
};

void hos_firmware_load(hos_t *hos, file_t *file) {
    zipdir_t * zipfirm = zipdir_open_2(file);
    if (!zipfirm)
        return;
    vector_t *files = fs_list_all_files((const fsdir_t*)zipfirm);
    for (size_t i = 0; i < vector_size(files); i++) {
        content_archive_t * nca = content_archive_create(hos->kdb, (fsdir_t*)zipfirm, vector_get(files, i));
        if (nca->type != content_type_meta) {
            for (size_t j = 0; j < count_of(services_list); j++)
                if (nca->program_id == strtoul(services_list[j].program_id, nullptr, 16))
                    services_list[j].callback(nca, hos);
        }
        content_archive_destroy(nca);
    }
    vector_destroy(files);

    zipdir_close(zipfirm);
}

void hos_enable(const char *workdir, hos_t *hos, loader_base_t*) {

    hos->services = list_create(0);

    dir_t * dir = dir_open(workdir, "r");
    file_t * firm = dir_open_file(dir, "nx/firmware.zip", "r");
    if (firm) {
        hos_firmware_load(hos, firm);
        dir_close_file(dir, firm);
    }
    dir_close(dir);

}
void hos_disable(hos_t *hos) {
    if (hos->services)
        list_destroy(hos->services);
    hos->services = nullptr;
}


size_t hos_getprocess_count(hos_t*) {
    return 0;
}
void * hos_continue(hos_t*) {
    return nullptr;
}