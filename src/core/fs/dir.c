#include <string.h>

#include <fs/dir.h>
#include <core/types.h>

fsfile_t * fs_dir_open_file(fsdir_t *fsdir, const char *path, const char *mode) {
    return (fsfile_t*)dir_open_file((dir_t*)fsdir, path, mode);
}

void fs_dir_close_file(fsdir_t *dir, fsfile_t *file) {
    dir_close_file((dir_t*)dir, (file_t*)file);
}

vector_t * fs_dir_list_all_files(const fsdir_t *dir) {
    return list_all_files(dir->path);
}

dir_t *dir_open(const char *path, const char *mode) {
    dir_t * dir = funbox_malloc(sizeof(dir_t));
    dir->cached_files = list_create(0);

    strcpy(dir->vdir.path, path);
    strcpy(dir->vdir.mode, mode);

    dir->vdir.open_file = fs_dir_open_file;
    dir->vdir.close_file = fs_dir_close_file;

    return dir;
}
file_t *dir_open_file(const dir_t *dir, const char *filepath, const char *mode) {
    for (size_t i = 0; i < list_size(dir->cached_files); i++) {
        file_t *file = list_get(dir->cached_files, i);
        if (strcmp(fs_getpath(file), filepath) == 0)
            return file;
    }

    if (*mode == 'w')
        if (*dir->vdir.mode != 'w')
            return NULL;

    if (!fs_exists(filepath))
        return NULL;
    file_t *file = file_open(filepath, "r");
    list_push(dir->cached_files, file);

    file->parent = dir;
    return file;
}
void dir_close_file(const dir_t *dir, file_t *file) {
    for (size_t i = 0; i < list_size(dir->cached_files); i++) {
        file_t *thisfile = list_get(dir->cached_files, i);
        if (thisfile != file)
            continue;

        thisfile->parent = NULL;
        list_drop(dir->cached_files, i);
        break;
    }
    file_close(file);
}

void dir_close(dir_t *dir) {
    for (size_t i = 0; i < list_size(dir->cached_files); i++) {
        file_t *thisfile = list_get(dir->cached_files, i);
        file_close(thisfile);
    }
    list_destroy(dir->cached_files);
    funbox_free(dir);
}