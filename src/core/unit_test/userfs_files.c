#include <unistd.h>
#include <fs/file.h>
#include <fs/userfs.h>

int main() {
    userfs_t * filesystem = userfs_create("DIR");

    file_t *selfbin = file_open("userfs_files", "r");

    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/binary/files/file.bin");
    sleep(5);

    userfs_destroy(filesystem);
    file_close(selfbin);
}
