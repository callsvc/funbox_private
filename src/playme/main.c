#include <types.h>
#include <vm/coffee.h>

#include <zip_fs/zip.h>
#include <manifest.h>
#include <string.h>

void startvm(coffee_t * vm, const vector_t * class_bytes) {
    coffee_reset_state(vm);
    vector_t *classes = vector_create(0, sizeof(span_t));
    const span_t firstclass = {vector_begin(class_bytes), vector_size(class_bytes)};

    vector_emplace(classes, &firstclass);

    coffee_run(vm, classes);

    vector_destroy(classes);
}

// https://nikita36078.github.io/J2ME_Docs/
// https://docs.oracle.com/javame/config/cldc/ref-impl/midp2.0/jsr118/javax/microedition/midlet/MIDlet.html
int main() {
    coffee_t * vm = coffee_vm();

    zipdir_t * micromid = zipdir_open("micro.jar");
    fsfile_t *manifest = fs_open_file((fsdir_t*)micromid, "META-INF/MANIFEST.MF", "r");

    manifest_t * entries = manifest_create(manifest);
    fs_close_file((fsdir_t*)micromid, manifest);

    const char *mainclass = manifest_get_str(entries, "MIDlet-1");

    char *classpath = strings_concat(2, strrchr(mainclass, ',') + 2, ".class");
    fsfile_t *mainfile = fs_open_file((fsdir_t*)micromid, classpath, "r");
    fb_free(classpath);

    if (!mainfile)
        oskill("can't open the main class file specified by the manifest");

    vector_t *content = fs_filebytes(mainfile);
    fs_close_file((fsdir_t*)micromid, mainfile);
    startvm(vm, content);
    vector_destroy(content);

    zipdir_close(micromid);
    manifest_destroy(entries);

    coffee_destroy(vm);
    return 0;
}
