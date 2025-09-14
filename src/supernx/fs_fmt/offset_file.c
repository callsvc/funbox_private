#include <string.h>
#include <types.h>
#include <fs_fmt/offset_file.h>


void fs_offset_file_read(struct fsfile *file, void *output, const size_t size, const size_t offset) {
    const offset_file_t *setfile = (offset_file_t*)file;
    if (offset + size > fs_getsize(file))
        quit("invalid offset file");

    if (!setfile->buffer)
        fs_read(setfile->file, output, size, setfile->start + offset);
    else memcpy(output, &setfile->buffer[offset], size);
}
void fs_offset_file_write(struct fsfile *file, const void *input, const size_t size, const size_t offset) {
    const offset_file_t *setfile = (offset_file_t*)file;
    if (offset + size > fs_getsize(file))
        quit("invalid offset file");

    if (!setfile->buffer)
        fs_write(setfile->file, input, size, setfile->start + offset);
    else memcpy(&setfile->buffer[offset], input, size);
}
size_t fs_offset_file_getsize(const fsfile_t *file) {
    const offset_file_t *setfile = (const offset_file_t*)file;
    return setfile->size;
}

offset_file_t * offset_file_open(fsfile_t *base, const char *name, const size_t size, const uint64_t offset, const bool allocate) {
    offset_file_t *setfile = fb_malloc(sizeof(offset_file_t));

    strcpy(setfile->vfile.path, name);
    setfile->vfile.type = file_type_offsetfile;
    setfile->vfile.fs_getsize = fs_offset_file_getsize;
    setfile->vfile.fs_read = fs_offset_file_read;
    setfile->vfile.fs_write = fs_offset_file_write;

    setfile->file = base;
    setfile->size = size;
    setfile->start = offset;

    static constexpr size_t mim_buffer_size = 2 * 1024 * 1024;
    if (!fs_is_mapfile(setfile->file) && size < mim_buffer_size && allocate) {
        setfile->buffer = fb_malloc(size);
        fs_read(setfile->file, setfile->buffer, size, setfile->start);
    }

    return setfile;
}
void offset_file_close(const fsfile_t *base, offset_file_t *file) {
    if (base && file->file != base)
        quit("unable to close file");
    fb_free(file->buffer);
    fb_free(file);
}
