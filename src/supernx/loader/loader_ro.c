#include <string.h>

#include <loader/types.h>
#include <loader/nsp.h>

vector_t * loader_get_logo(loader_base_t *loader) {
    return loader->loader_get_logo(loader);
}

uint64_t loader_get_program_id(loader_base_t *loader) {
    return loader->loader_get_program_id(loader);
}

loader_type_e loader_get_extension_type(const fsfile_t *file) {
    const char *ext = strrchr(file->path, '.');
    if (ext && strcmp(ext + 1, "nsp") == 0)
        return loader_nsp_type;

    return loader_unknown_type;
}
loader_type_e loader_get_rom_type(fsfile_t *file) {
    if (loader_get_extension_type(file) == loader_nsp_type)
        if (nsp_is_nsp(file))
            return loader_nsp_type;

    return loader_unknown_type;
}