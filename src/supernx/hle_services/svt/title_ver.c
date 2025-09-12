#include <stdio.h>
#include <string.h>
#include <fs_fmt/offset_file.h>
#include <fs_fmt/content_archive.h>

#include <types.h>
#include <horizon/hos.h>


#include <hle_services/svt/title_ver.h>
void svt_display(const struct svt_format *title) {
    fprintf(stdout, "platform string %s\n", title->platform);
    fprintf(stdout, "version hash: %s\n", title->version_hash);
    fprintf(stdout, "display version: %s\n", title->display_version);
    fprintf(stdout, "display title: %s\n", title->display_title);
}

void svt_load(content_archive_t *nca, hos_t *hos) {
    svt_service_t *service = fb_malloc(sizeof(svt_service_t));

    const romfs_t * main_fs = content_archive_get_fs(nca, 0, false);
    fsfile_t * file = fs_open_file((fsdir_t*)main_fs, "file", "r");

    fs_read(file, &service->content, sizeof(service->content), 0);

    const struct svt_format *title = &service->content;
    svt_display(title);

    if (strcmp(title->display_version, "18.0.0") == 0)
        if (strcmp(title->version_hash, "12dd67abcd6e05f44dc8a526e5af9c1f14202c8b") != 0)
            quit("firmware hash incorrect, possible data corruption");

    offset_file_close(nullptr, (offset_file_t*)file);
    list_push(hos->services, service);
}
