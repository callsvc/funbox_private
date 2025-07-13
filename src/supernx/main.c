

#include <fs/file.h>
#include <nx_sys.h>


int main() {
    nx_sys_t *nx_sys = nx_sys_create();
    nx_get_all_loaders(nx_sys);

    if (nx_get_games_count(nx_sys))
        nx_load_first_one(nx_sys);

    for (size_t i = 0; i < nx_get_games_count(nx_sys); i++) {
        if (!nx_sys->loader)
            break;
        vector_t * logo = loader_get_logo(nx_sys->loader);

        char * output_logo = fs_build_path(3, "/tmp", to_str64(loader_get_program_id(nx_sys->loader), 16), "NintendoLogo.png");
        file_t * logo_file = file_open(output_logo, "w");

        fs_write((fsfile_t*)logo_file, vector_begin(logo), vector_size(logo), 0);

        vector_destroy(logo);
        fb_free(output_logo);
        file_close(logo_file);
    }

    nx_sys_destroy(nx_sys);
}
