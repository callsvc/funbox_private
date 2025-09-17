
#include <string.h>
#include <types.h>
#include <fs/types.h>

bool hos_is_titleicon(fsfile_t *file) {
    uint8_t content[64 * 1024];
    fs_read(file, content, 2, 0);
    fs_read(file, content + 2, 2, fs_getsize(file) - 2);

    constexpr uint8_t marker[] = {0xFF, 0xD8, 0xFF, 0xD9};
    if (memcmp(content, marker, 4) != 0)
        return false;
    fs_read(file, content, 1024, 0);
    if (content[2] != 0xFF)
        return false;

    int32_t exif = 0, camera = 0, software = 0;

    for (size_t i = 0; i < fs_getsize(file); ) {
        const size_t rds = MIN(fs_getsize(file) - i, sizeof(content));
        fs_read(file, content, rds, i);
        if (!exif)
            exif = memmem(content, rds, "Exif\0\0", 6) != nullptr;

        if (exif) {
            if (!camera)
                camera = memmem(content, rds, "Nintendo co., ltd", strlen("Nintendo co., ltd")) != nullptr;
            if (!software)
                software = memmem(content, rds, "Nintendo AuthoringTool", strlen("Nintendo co., ltd")) != nullptr;
        }
        i += rds;
    }

    return exif && camera && software;
}
