#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fs/types.h>

fsfile_t * fs_open_file(fsdir_t *dir, const char *path, const char *mode) {
    return dir->open_file(dir, path, mode);
}
void fs_close_file(fsdir_t *dir, fsfile_t *file) {
    dir->close_file(dir, file);
}

void fs_read(fsfile_t *file, void *out, const size_t size, const size_t offset) {
    file->fs_read(file, out, size, offset);
}

size_t fs_getsize(const fsfile_t *file) {
    return file->fs_getsize(file);
}
vector_t * fs_list_all_files(const fsdir_t *dir) {
    return dir->fs_list_all_files(dir);
}

vector_t * fs_filebytes(fsfile_t *file) {
    const size_t size = file->fs_getsize(file);
    if (!size)
        return nullptr;
    vector_t *result = vector_create(size, sizeof(uint8_t));
    vector_setsize(result, size);
    fs_read(file, vector_begin(result), size, 0);
    return result;
}

bool fs_is_mapfile(const fsfile_t *file) {
    return file->type == *(uint32_t*)"MAPFILE";
}

int32_t fs_exists(const char *path) {
    return access(path, F_OK) == 0;
}

const char * fs_getpath(const void *fsaddr) {
    return fsaddr;
}

const char * fs_readline(fsfile_t *file, size_t *offset) {
    if (*offset >= fs_getsize(file))
        return nullptr;

    fs_read(file, file->buffer, sizeof(file->buffer), *offset);

    const char *line = file->buffer;
    char *newline = strchr(line, '\n');
    bool isline = true;
    do {
        if (!isline && ((line = newline + 1)))
            newline = strchr(line, '\n');
        *newline = '\0';
    } while ((isline = strlen(line)) == false);
    *offset += strlen(line) + 1;
    return line;
}

vector_t * fs_grep(const fsdir_t *dir, const char *exp) {
    const vector_t * files = fs_list_all_files(dir);
    vector_t * result = vector_create(0, 0);

    for (size_t i = 0; i < vector_size(files); i++) {
        const char * filepath = vector_get(files, i);
        const size_t exp_len = strlen(exp);
        const size_t fp_len = strlen(filepath);

        if (*exp == '*') {
            if (strcmp(filepath + fp_len - exp_len + 1, &exp[1]) == 0)
                vector_emplace(result, filepath);
        } else if (*(exp + exp_len - 1) == '*')
            if (fp_len > exp_len)
                if (strncmp(filepath, exp, exp_len - 1) == 0)
                    vector_emplace(result, filepath);
    }
    vector_destroy((vector_t*)files);
    return result;
}

vector_t * fs_getbytes(fsfile_t *file, const size_t size, const uint64_t offset) {
    if (size + offset > fs_getsize(file))
        return nullptr;
    vector_t *result = vector_create(size, sizeof(uint8_t));
    vector_setsize(result, size);
    fs_read(file, vector_begin(result), size, offset);

    return result;
}
vector_t * fs_getfile(fsfile_t *file) {
    return fs_getbytes(file, fs_getsize(file), 0);
}
