#include <string.h>

#include <manifest.h>
#include <types.h>

manifest_t * manifest_create(fsfile_t *file) {
    manifest_t *manifest = funbox_malloc(sizeof(manifest_t));
    manifest->attrs = set_create();

    size_t position = 0;
    for (const char * line; (line = fs_readline(file, &position)); ) {
        char key[100], keyval[100];
        const size_t middle = strchr(line, ':') - line;
        funbox_strncpy(key, line, middle);

        funbox_strncpy(keyval, line + middle, strlen(line) - middle);
        trim(keyval);

        set_set(manifest->attrs, setval_string, setval_string, key, &keyval[2]);
    }
    return manifest;
}

const char * manifest_get_str(const manifest_t *manifest, const char *name) {
    return set_get(manifest->attrs, setval_string, name);
}
void manifest_destroy(manifest_t *manifest) {
    set_destroy(manifest->attrs);
    funbox_free(manifest);
}