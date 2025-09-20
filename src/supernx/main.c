

#include <fs/file.h>
#include <nx_sys.h>


int main() {
    nx_sys_t *nx_sys = nx_sys_create();
    nx_get_all_loaders(nx_sys);
    fb_get_heap_usage(nullptr);

    if (nx_get_games_count(nx_sys))
        nx_load_first_one(nx_sys);

    for (size_t i = 0; i < nx_get_games_count(nx_sys); i++) {
        if (!nx_sys->loader)
            break;
        vector_t * logo = loader_get_logo(nx_getgame(nx_sys, i));

        char buffer[20];
        char * output_logo = fs_build_path(3, "/tmp", to_str64(loader_get_program_id(nx_getgame(nx_sys, i)), buffer, 16), "icon_AmericanEnglish.jpeg");
        file_t * logo_file = file_open(output_logo, "rw", false);

        fs_write((fsfile_t*)logo_file, vector_begin(logo), vector_size(logo), 0);
        if (!hos_is_titleicon((fsfile_t*)logo_file))
            quit("WTF!!!!");

        vector_destroy(logo);
        fb_free(output_logo);
        file_close(logo_file);
    }

    nx_sys_destroy(nx_sys);
}
