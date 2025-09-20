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

file_t * file_open(const char* path, const char* mode, const bool paging) {
    constexpr size_t buffer_len = 32 * 1024;
    if (access(path, F_OK)) {
        if (fs_isro(mode))
            return nullptr;
        touch(path);
    }

    file_t *file = fb_malloc(sizeof(file_t));

    strcpy(file->vfile.path, path);
    file->vfile.type = file_type_file;

    file->handle = fopen(path, fs_isro(mode) ? "r" : "r+");
    if (!file->handle)
        quit("? maybe you suck!");

    if (fs_isro(mode)) {
        file->buffer = fb_malloc(sizeof(char) * buffer_len);
        setbuffer(file->handle, file->buffer, buffer_len);
    } else {
        setbuf(file->handle, nullptr);
    }
    file->vfile.fs_getsize = fs_file_get_size;
    file->vfile.fs_write = fs_file_write;
    file->vfile.fs_read = fs_file_read;

    if (paging)
        file->page_list = vector_create(0, sizeof(struct iovec));

    return file;
}

size_t file_getsize(const file_t *file) {
    const ssize_t offset = ftell(file->handle);
    fseek(file->handle, 0, SEEK_END);
    const size_t result = ftell(file->handle);
    fseek(file->handle, offset, SEEK_SET);
    return result;
}

void file_rw(const file_t *file, void *data, const size_t size, const size_t offset, const bool write) {
    if (!write && file->page_list && !vector_empty(file->page_list))
        file_flush(file);

    const size_t rwoffset = ftell(file->handle);
    if (offset != -1 && rwoffset != offset)
        fseek(file->handle, (int64_t)offset, SEEK_SET);

    if (write) {
        if (!file->buffer && file->page_list) {
            const span_t content = {.data = data, .size = size};
            vector_emplace(file->page_list, &content);
        } else if (fwrite(data, size, 1, file->handle) != 1) {
            quit("error writing to file!");
        }
    } else {
        if (fread(data, size, 1, file->handle) != 1)
            quit("can't read %ld bytes at offset %ld, file path: %s", size, fs_getpath(file));
    }
    if (offset != -1)
        fseek(file->handle, rwoffset, SEEK_SET);
}

void file_write(const file_t *file, const void *input, const size_t size, const size_t offset) {
    file_rw(file, (void*)input, size, offset, true);
}
void file_read(const file_t *file, void *output, const size_t size, const size_t offset) {
    file_rw(file, output, size, offset, false);
}
void file_flush(const file_t *file) {
    fflush(file->handle);

    if (!file->page_list)
        return;
    const struct iovec *iolist = vector_begin(file->page_list);
    const uint64_t offset = ftell(file->handle);
    if (!vector_empty(file->page_list)) {
        const int64_t result = pwritev(fileno(file->handle), iolist, vector_size(file->page_list), (off_t)offset);
        fseek(file->handle, result, SEEK_CUR);
    }
    vector_setsize(file->page_list, 0);
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
    if (file->page_list)
        vector_destroy(file->page_list);
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