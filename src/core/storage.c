#include <stdlib.h>
#include <string.h>
#include <types.h>

#include <zip.h>
#include <core/storage.h>
#include <fs/mapfile.h>

storage_t * stg = nullptr;

void storage_init() {
    stg = fb_malloc(sizeof(storage_t));
    stg->dirs = list_create(sizeof(dirreg_t));

    FILE * file_max = fopen("/proc/sys/fs/file-max", "r");
    char *buffer = fb_malloc(16);
    size_t size = 16;
    if (getline(&buffer, &size, file_max))
        if (buffer && size)
            stg->file_max = strtoul(buffer, nullptr, 0);

    fb_free(buffer);
    fclose(file_max);

    storage_opendir(stg, "logs", "LOGS_DIR");
}
void storage_zip(const storage_t * this_stg, fsfile_t *file) {
    if (file->type != file_type_mapfile)
        return;
    if (fs_getsize(file))
        return;
    const int32_t fd = ((mapfile_t*)file)->fd;
    zip_t *zip = zip_fdopen(fd, ZIP_CREATE, nullptr);

    for (size_t i = 1; i < list_size(this_stg->dirs); i++) {
        dir_t * dir = ((dirreg_t*)list_get(this_stg->dirs, i))->dir;

        vector_t *files = fs_list_all_files((fsdir_t*)dir);
        for (size_t j = 0; j < vector_size(files); j++) {
            fsfile_t * dir_file = fs_open_file((fsdir_t*)dir, vector_get(files, j), "r");

            vector_t *buffer = fs_getfile(dir_file);
            zip_source_t * zip_src = zip_source_buffer(zip, vector_begin(buffer), vector_size(buffer), 0);

            zip_file_add(zip, fs_getpath(dir_file), zip_src, ZIP_FL_ENC_UTF_8);
            zip_source_free(zip_src);

            vector_destroy(buffer);

            fs_close_file((fsdir_t*)dir, dir_file);
        }
        vector_destroy(files);
    }

    zip_close(zip);
}
void storage_opendir(const storage_t *this_stg, const char *pdir, const char *dirname) {
    char path[100];
    *path = '?';
    const dirreg_t * root_dir = list_get(this_stg->dirs, 0);
    if (root_dir)
        sprintf(strchr(path, '?'), "%s?", fs_getpath(root_dir->dir));

    sprintf(strchr(path, '?'), "%s", pdir);
    dir_t * dir = dir_open(path, "w");
    if (!dir)
        return;

    dirreg_t dircnt = {.dir = dir};
    strcpy(dircnt.name, dirname);
    *(dirreg_t*)list_emplace(this_stg->dirs) = dircnt;
}


void storage_destroy() {
    for (size_t i = 0; i < list_size(stg->dirs); i++) {
        dir_close(((dirreg_t*)list_get(stg->dirs, i))->dir);
    }
    list_destroy(stg->dirs);
    fb_free(stg);
}