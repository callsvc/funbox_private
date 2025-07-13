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
size_t mapfile_getsize(const mapfile_t *mapfile) {
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
    mapfile->vfile.type = *(uint32_t*)"MAPFILE";

    mapfile->fd = open(fs_getpath(file), O_RDONLY);
    const uint64_t len = fs_getsize(file);

    mapfile->buffer = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, mapfile->fd, 0);
    mapfile->vfile.fs_read = fs_mapfile_read;
    mapfile->vfile.fs_getsize = fs_mapfile_getsize;
    return mapfile;
}

void mapfile_close(mapfile_t *mapfile) {
    const size_t len = mapfile_getsize(mapfile);

    if (mapfile->buffer)
        munmap(mapfile->buffer, len);
    close(mapfile->fd);
    fb_free(mapfile);
}
