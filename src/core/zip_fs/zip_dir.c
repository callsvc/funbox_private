#include <assert.h>
#include <string.h>

#include <types.h>
#include <zip_fs/zip.h>

// ReSharper disable once CppParameterMayBeConstPtrOrRef
zipfile_t * zipdir_open_file(zipdir_t *dir, const char *name, const char *mode) {
    assert(strchr(mode, 'r'));
    const zip_int64_t index = zip_name_locate(dir->zipfile, name, 0);

    zip_stat_t info;
    zip_stat_index(dir->zipfile, index, 0, &info);

    if (index)
        return zipfile_open(dir->zipfile, index, info.size);
    return nullptr;
}
// ReSharper disable once CppParameterNeverUsed
void zipdir_close_file(zipdir_t *dir, zipfile_t *file) {
    zipfile_close(file);
}

fsfile_t * fs_zipdir_open_file(fsdir_t *dir, const char *name, const char *mode) {
    return (fsfile_t*)zipdir_open_file((zipdir_t*)dir, name, mode);
}
void fs_zipdir_close_file(fsdir_t *dir, fsfile_t *file) {
    zipdir_close_file((zipdir_t*)dir, (zipfile_t*)file);
}

zipdir_t * zipdir_open(const char *path) {
    zipdir_t *dir = fb_malloc(sizeof(zipdir_t));
    dir->zipfile = zip_open(path, ZIP_RDONLY, 0);
    if (!dir->zipfile)
        oskill("can't open zip filename %s", path);

    dir->vdir.open_file = fs_zipdir_open_file;
    dir->vdir.close_file = fs_zipdir_close_file;
    return dir;
}
void zipdir_close(zipdir_t *dir) {
    zip_close(dir->zipfile);
    fb_free(dir);
}
