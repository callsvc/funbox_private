#include <assert.h>

#include <disk.h>
disk_t * disk_create() {
    disk_t *disk = fb_malloc(sizeof(disk_t));
    return disk;
}
void disk_reset(disk_t *disk, const char *filepath) {
    if (disk->file && filepath)
        file_close(disk->file);
    disk->file = file_open(filepath, "r");
    if (!disk->file)
        oskill("bootable file not found");
}

void disk_read_sector(const disk_t *disk, const uint16_t sec, uint8_t *dest) {
    assert(sec % 512 == 0);
    fs_read((fsfile_t*)disk->file, dest, 512, sec * 512);
}

void disk_destroy(disk_t *disk) {
    file_close(disk->file);
    disk->file = nullptr;
    fb_free(disk);
}
