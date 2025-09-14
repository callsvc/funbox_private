#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <fs/mapfile.h>
#include <core/types.h>

void mapfile_read(const mapfile_t *mapfile, void *output, const size_t size, const size_t offset) {
    memcpy(output, &mapfile->buffer[offset], size);
}

void fs_mapfile_read(fsfile_t *file, void *output, const size_t size, const size_t offset) {
    mapfile_read((mapfile_t*)file, output, size, offset);
}
void mapfile_write(const mapfile_t *mapfile, const void *input, const size_t size, const size_t offset) {
    memcpy(&mapfile->buffer[offset], input, size);
}

void fs_mapfile_write(fsfile_t *file, const void *input, const size_t size, const size_t offset) {
    mapfile_write((mapfile_t*)file, input, size, offset);
}
size_t mapfile_getsize(const mapfile_t *mapfile) {
    if (mapfile->size)
        return mapfile->size;
    static struct stat file_st;
    fstat(mapfile->fd, &file_st);
    return file_st.st_size;
}
size_t fs_mapfile_getsize(const fsfile_t *file) {
    return mapfile_getsize((const mapfile_t*)file);
}

mapfile_t * mapfile_open(const fsfile_t *file) {
    mapfile_t *mapfile = fb_malloc(sizeof(mapfile_t));
    strcpy(mapfile->vfile.path, fs_getpath(file));
    mapfile->vfile.type = file_type_mapfile;

    mapfile->fd = open(fs_getpath(file), O_RDONLY);
    const uint64_t len = fs_getsize(file);

    mapfile->buffer = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, mapfile->fd, 0);
    mapfile->vfile.fs_read = fs_mapfile_read;
    mapfile->vfile.fs_getsize = fs_mapfile_getsize;
    return mapfile;
}

mapfile_t * mapfile_open_2(const char *path, uint8_t *buffer, const size_t len) {
    mapfile_t *mapfile = fb_malloc(sizeof(mapfile_t));
    strcpy(mapfile->vfile.path, path);
    mapfile->vfile.type = file_type_mapfile;

    mapfile->size = len;
    mapfile->buffer = buffer;
    mapfile->fd = -1;
    mapfile->vfile.fs_read = fs_mapfile_read;
    mapfile->vfile.fs_write = fs_mapfile_write;
    mapfile->vfile.fs_getsize = fs_mapfile_getsize;

    return mapfile;
}

void mapfile_close(mapfile_t *mapfile) {
    const size_t len = mapfile_getsize(mapfile);

    if (mapfile->fd && mapfile->buffer)
        munmap(mapfile->buffer, len);
    else if (mapfile->buffer)
        fb_free(mapfile->buffer);

    if (mapfile->fd > 0)
        close(mapfile->fd);
    fb_free(mapfile);
}
