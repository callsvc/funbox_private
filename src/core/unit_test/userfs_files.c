#include <unistd.h>
#include <fs/file.h>
#include <fs/userfs.h>

int main() {
    userfs_t * filesystem = userfs_create("DIR");

    file_t *selfbin = file_open("userfs_files", "r");

    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/1binary/files/file.bin");
    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/binary/files/file1.bin");
    /*
    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/binary1/files/file2.bin");
    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/binary/file3s/file3.bin");
    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/bin2ary/files/file4.bin");
    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/binary/files/file5.bin");
    userfs_mountfile(filesystem, (fsfile_t*)selfbin, "DIR/bi3nary/files/file6.bin");
    */
    sleep(100);

    userfs_destroy(filesystem);
    file_close(selfbin);
}
