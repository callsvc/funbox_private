#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include <core/types.h>
#include <fs/file.h>

#include <fs/dir.h>

size_t fs_file_get_size(const fsfile_t *file) {
    return file_getsize((const file_t*)file);
}
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void fs_file_read(fsfile_t *file, void *output, const size_t size, const size_t offset) {
    file_read((const file_t*)file, output, size, offset);
}
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void fs_file_write(fsfile_t *file, const void *input, const size_t size, const size_t offset) {
    file_write((const file_t*)file, input, size, offset);
}

file_t * file_open(const char* path, const char* mode) {
    constexpr size_t buffer_len = 32 * 1024;
    if (access(path, F_OK)) {
        if (*mode == 'r')
            return nullptr;
        touch(path);
    }

    file_t *file = fb_malloc(sizeof(file_t));

    strcpy(file->vfile.path, path);
    file->vfile.type = file_type_file;

    file->handle = fopen(path, mode);
    if (!file->handle)
        quit("? maybe you suck!");

    if (!(strchr(mode, 'r') && strchr(mode, 'w'))) {
        file->buffer = fb_malloc(sizeof(char) * buffer_len);
        setbuffer(file->handle, file->buffer, buffer_len);
    } else {
        setbuf(file->handle, nullptr);
    }
    file->vfile.fs_getsize = fs_file_get_size;
    file->vfile.fs_write = fs_file_write;
    file->vfile.fs_read = fs_file_read;

    file->write_stall = vector_create(0, sizeof(struct iovec));

    return file;
}

size_t file_getsize(const file_t *file) {
    const ssize_t offset = ftell(file->handle);
    fseek(file->handle, 0, SEEK_END);
    const size_t result = ftell(file->handle);
    fseek(file->handle, offset, SEEK_SET);
    return result;
}

void file_write(const file_t *file, const void *input, const size_t size, const size_t offset) {
    if (!vector_empty(file->write_stall))
        file_flush(file);
    size_t roff = 0;
    if ((roff = ftell(file->handle)) != offset)
        fseek(file->handle, (int64_t)offset, SEEK_SET);
    fwrite(input, size, 1, file->handle);
    if (roff != offset)
        fseek(file->handle, roff, SEEK_SET);
}

void file_swrite(const file_t *file, const void *src, const size_t size) {
    if (!size && !src)
        return;
    const struct {
        const void *buffer;
        size_t size;
    } stall = {.buffer = src, .size = size};
    _Static_assert(sizeof(stall) == sizeof(struct iovec));
    vector_emplace(file->write_stall, &stall);
}

void file_flush(const file_t *file) {
    fflush(file->handle);

    if (vector_empty(file->write_stall))
        return;
    const struct iovec *iolist = vector_begin(file->write_stall);
    const uint64_t offset = ftell(file->handle);
    const int64_t result = pwritev(fileno(file->handle), iolist, (int)vector_size(file->write_stall), (off_t)offset);

    fseek(file->handle, result, SEEK_CUR);
    vector_setsize(file->write_stall, 0);
}

void file_read(const file_t *file, void *output, const size_t size, const size_t offset) {
    size_t woff = 0;
    if ((woff = ftell(file->handle)) != offset)
        fseek(file->handle, offset, SEEK_SET);
    if (fread(output, size, 1, file->handle) != 1)
        quit("can't read %ld bytes at offset %ld, file path: %s", fs_getpath(file));
    if (woff != offset)
        fseek(file->handle, woff, SEEK_SET);
}

const char * file_errorpath(const char *path) {
    if (access(path, F_OK))
        return "file does not exist";
    struct stat file_stat;
    if (stat(path, &file_stat) == 0) {
        if (!(file_stat.st_mode & S_IRUSR))
            return "can't read from the file";
        if (!(file_stat.st_mode & S_IWUSR))
            return "can't write to the file";
    }
    return "none";
}
void file_close(file_t *file) {
    file_flush(file);
    vector_destroy(file->write_stall);
    if (file->parent) {
        dir_close_file(file->parent, file);
        return;
    }
    if (file->handle)
        fclose(file->handle);

    if (file->buffer)
        fb_free(file->buffer);
    fb_free(file);
}