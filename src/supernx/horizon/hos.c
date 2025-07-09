#include <horizon/hos.h>
#include <types.h>

#include <fs/file.h>
#include <fs/types.h>
#include <fs/dir.h>

void find_keys(keys_db_t *keys, const dir_t *dir, const char *keyname) {
    char *keypath = fs_build_path(2, "keys", keyname);
    if (fs_exists(keypath)) {
        file_t *file = dir_open_file(dir, keypath, "r");
        if (file)
            keys_db_load(keys, (fsfile_t*)file);
        dir_close_file(dir, file);
    } else {
        oskill("can't find keys");
    }
    funbox_free(keypath);
}

hos_t *hos_create(const char *dir) {
    hos_t * hos = funbox_malloc(sizeof(hos_t));
    hos->kdb = keys_db_create();

    dir_t *wdir = dir_open(dir, "r");
    if (!wdir)
        oskill("can't open directory");

    find_keys(hos->kdb, wdir, "title.keys");
    find_keys(hos->kdb, wdir, "prod.keys");

    dir_close(wdir);
    return hos;
}

void hos_destroy(hos_t *hos) {
    keys_db_destroy(hos->kdb);
    funbox_free(hos);
}