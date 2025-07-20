#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <types.h>
#include <fs/types.h>
#include <fs/dir.h>
#include <fs/mapfile.h>


fsfile_t * fs_open_file(fsdir_t *dir, const char *path, const char *mode) {
    return dir->open_file(dir, path, mode);
}
void fs_close_file(fsdir_t *dir, fsfile_t *file) {
    dir->close_file(dir, file);
}

void fs_read(fsfile_t *file, void *output, const size_t size, const size_t offset) {
    file->fs_read(file, output, size, offset);
}

void fs_write(fsfile_t *file, const void *input, const size_t size, const size_t offset) {
    file->fs_write(file, input, size, offset);
}

void create_directories(const char *path) {
    char buffer[0x10 * 3];
    for (const char *subpath = path; (subpath = strchr(*subpath == '/' ? subpath + 1 : subpath, '/')); ) {
        fb_strcopy(buffer, path, subpath - path);
        mkdir(buffer, 0755);
        if (subpath == strrchr(path, '/'))
            break;
    }
}

void touch(const char *path) {
    create_directories(path);
    close(open(path, O_RDWR | O_CREAT, 0644));
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

bool fs_isfdvalid(const int32_t fd) {
    dir_t *fds = dir_open("/proc/self/fd", "r");
    char buffer[45];
    fsfile_t *file = fs_open_file((fsdir_t*)fds, to_str64(fd, buffer, 10), "r");
    const bool result = file != nullptr;
    if (file)
        fs_close_file((fsdir_t*)fds, file);
    dir_close(fds);
    return result;
}

bool fs_is_mapfile(const fsfile_t *file) {
    if (file->type == file_type_mapfile)
        return fs_isfdvalid(((mapfile_t*)file)->fd);
    return false;
}

int32_t fs_exists(const char *path) {
    return access(path, F_OK) == 0;
}

bool fs_exists_in_fsdir(const vector_t *files, const char *path) {
    for (size_t i = 0; i < vector_size(files); i++)
        if (strcmp(path, vector_get(files, i)) == 0)
            return true;
    return false;
}

const char * fs_getpath(const void *fsaddr) {
    return fsaddr;
}

size_t fs_path_depth(const char *path) {
    size_t count = 0;
    for (const char *path_p = path; *path_p; path_p++)
        if (*path_p == '/')
            count++;
    return count;
}

void fs_print_tree(const vector_t *files) {
    printf("files in this folder: \n");
    for (size_t i = 0; i < vector_size(files); i++) {
        const char *path = vector_get(files, i);
        const int32_t depth = fs_path_depth(path);
        printf("%*s%s\n", depth * 4, "", path);
    }
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
