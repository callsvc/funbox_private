#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/types.h>
#include <fs/file.h>

#include <fs/dir.h>

size_t fs_file_get_size(const fsfile_t *file) {
    return file_getsize((const file_t*)file);
}
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void fs_file_read(fsfile_t *file, void *out, const size_t size, const size_t offset) {
    file_read((const file_t*)file, out, size, offset);
}

file_t * file_open(const char* path, const char* mode) {
    constexpr size_t buffer_len = 8 * 1024;

    if (access(path, F_OK))
        return nullptr;

    file_t *file = funbox_malloc(sizeof(file_t));
    file->buffer = funbox_malloc(sizeof(char) * buffer_len);

    strcpy(file->vfile.path, path);

    file->handle = fopen(path, mode);
    setbuffer(file->handle, file->buffer, buffer_len);

    file->vfile.fs_getsize = fs_file_get_size;
    file->vfile.fs_read = fs_file_read;

    return file;
}

size_t file_getsize(const file_t *file) {
    const ssize_t offset = ftell(file->handle);
    fseek(file->handle, 0, SEEK_END);
    const size_t result = ftell(file->handle);
    fseek(file->handle, offset, SEEK_SET);
    return result;
}

void file_read(const file_t *file, void *data, const size_t size, const size_t offset) {
    if (ftell(file->handle) != offset)
        fseek(file->handle, (int64_t)offset, SEEK_SET);
    fread(data, size, 1, file->handle);
}

const char * file_errorpath(const char *path) {
    if (access(path, F_OK))
        return "file not exists";
    return "none";
}
void file_close(file_t *file) {
    if (file->parent) {
        dir_close_file(file->parent, file);
        return;
    }
    if (file->handle)
        fclose(file->handle);

    funbox_free(file->buffer);
    funbox_free(file);
}