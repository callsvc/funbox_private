#include <string.h>
#include <types.h>
#include <fs/file.h>
#include <zip_fs/zip.h>
#include <bundle/loader.h>

size_t filesize(const char *path) {
    file_t *file = file_open(path, "r");
    if (!file) return 0;
    const size_t size = fs_getsize((fsfile_t*)file);
    file_close(file);
    return size;
}

loader_t * loader_create(const char *path) {
    loader_t *ld = fb_malloc(sizeof(loader_t));
    ld->ipa_pkg = (fsdir_t*)zipdir_open(path);

    ld->ipa_files = fs_list_all_files(ld->ipa_pkg);
    printf("sizeof the ipa file: %lu\n", filesize(path));
    printf("number of files inside this ipa: %lu\n", vector_size(ld->ipa_files));
#if 0
    fs_print_tree(ld->ipa_files);
#endif

    const size_t len = vector_size(ld->ipa_files);
    for (size_t i = 0; i < len && !ld->info_plist; i++) {
        const char * filepath = vector_get(ld->ipa_files, i);
        if (!strstr(filepath, "Info.plist"))
            continue;
        fsfile_t * plist = fs_open_file(ld->ipa_pkg, filepath, "r");
        ld->info_plist = plist_create(plist);

        printf("user-visible package name: %s\n", plist_getstr(ld->info_plist, "CFBundleDisplayName"));

        fs_close_file(ld->ipa_pkg, plist);
    }

    return ld;
}
void loader_destroy(loader_t *ld) {
    if (ld->info_plist)
        plist_destroy(ld->info_plist);
    if (ld->ipa_files)
        vector_destroy(ld->ipa_files);
    if (ld->ipa_pkg)
        zipdir_close((zipdir_t*)ld->ipa_pkg);
    fb_free(ld);
}