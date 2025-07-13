#include <types.h>
#include <zip_fs/zip.h>


void zipfile_read(zipfile_t *file, void *out, const size_t size, const size_t offset) {
    if (offset) {
        if (zip_ftell(file->zipentry) > offset) {
            zip_fclose(file->zipentry);
            file->zipentry = zip_fopen_index(file->zip, file->index, 0);
        }
        while (zip_ftell(file->zipentry) != offset)
            zip_fread(file->zipentry, out, 1);
    }
    zip_fread(file->zipentry, out, size);
}

void fs_zipfile_read(fsfile_t *file, void *out, const size_t size, const size_t offset) {
    zipfile_read((zipfile_t*)file, out, size, offset);
}
size_t fs_zipfile_getsize(const fsfile_t *file) {
    return zipfile_getsize((const zipfile_t*)file);
}

size_t zipfile_getsize(const zipfile_t *file) {
    return file->zipentry ? file->size : 0ULL;
}

zipfile_t * zipfile_open(zip_t *zip, const size_t index, const size_t size) {
    zipfile_t *file = fb_malloc(sizeof(zipfile_t));
    file->zipentry = zip_fopen_index(zip, index, 0);
    file->zip = zip;
    file->index = index;

    file->size = size;

    file->vfile.fs_read = fs_zipfile_read;
    file->vfile.fs_getsize = fs_zipfile_getsize;

    return file;
}
void zipfile_close(zipfile_t *file) {
    zip_fclose(file->zipentry);
    fb_free(file);
}