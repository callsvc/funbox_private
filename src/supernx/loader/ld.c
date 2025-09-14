#include <string.h>

#include <loader/types.h>
#include <loader/nsp.h>
#include <loader/nsz.h>

vector_t * loader_get_logo(loader_base_t *loader) {
    return loader->loader_get_logo(loader);
}

uint64_t loader_get_program_id(loader_base_t *loader) {
    return loader->loader_get_program_id(loader);
}

loader_type_e loader_get_extension_type(const fsfile_t *file) {
    const char *ext = strrchr(file->path, '.');
    if (ext && strcmp(ext, ".nsp") == 0)
        return loader_nsp_type;
    if (ext && strcmp(ext, ".nsz") == 0)
        return loader_nsz_type;

    return loader_unknown_type;
}
loader_type_e loader_get_rom_type(fsfile_t *file) {
    switch (loader_get_extension_type(file)) {
        case loader_nsp_type:
            if (nsp_is_nsp(file))
                return loader_nsp_type;
        case loader_nsz_type:
            if (nsz_is_nsz(file))
                return loader_nsz_type;
        case loader_unknown_type:
        default:
    }

    return loader_unknown_type;
}