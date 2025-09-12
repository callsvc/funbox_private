#include <nx_sys.h>

#include <types.h>
#include <fs/mapfile.h>
#include <fs/dir.h>

#include <loader/nsp.h>


nx_sys_t *nx_sys_create() {
    nx_sys_t *nx = fb_malloc(sizeof(nx_sys_t));
    nx->procinfo = procinfo_create();
    nx->hos = hos_create(nx->procinfo->current_dir);

    nx->games = list_create(sizeof(game_file_t));
    return nx;
}

void nx_get_all_loaders(const nx_sys_t *nx) {
    char * gamesdir = fs_build_path(2, nx->procinfo->current_dir, "games_files");

    dir_t *dir = dir_open(gamesdir, "r");
    vector_t *files = list_all_files(gamesdir);
    for (size_t i = 0; i < vector_size(files); i++) {
        file_t *file = dir_open_file(dir, vector_get(files, i), "r");
        if (!file)
            continue;

        const loader_type_e rom_type = loader_get_rom_type((fsfile_t*)file);
        if (rom_type == loader_unknown_type)
            quit("Are you kidding???");

        game_file_t *typed = list_emplace(nx->games);
        typed->file = (fsfile_t*)mapfile_open((const fsfile_t*)file);
        typed->type = rom_type;

        dir_close_file(dir, file);
    }

    vector_destroy(files);
    dir_close(dir);
    fb_free(gamesdir);
}

void nx_load_first_one(nx_sys_t *nx) {
    if (!nx_get_games_count(nx))
        return;
    const game_file_t *roimage = list_get(nx->games, 0);
    if (roimage->type == loader_nsp_type)
        nx->loader = (loader_base_t*)nsp_create(roimage->file, nx->hos->kdb);
    else return;

    hos_enable(nx->procinfo->current_dir, nx->hos, nx->loader);
    while (hos_getprocess_count(nx->hos)) {
        hos_continue(nx->hos);
    }
    hos_disable(nx->hos);
}
size_t nx_get_games_count(const nx_sys_t *nx) {
    return list_size(nx->games);
}

void nx_sys_destroy(nx_sys_t *nx) {
    if (nx->loader)
        nsp_destroy((nsp_t*)nx->loader);

    for (size_t i = 0; i < list_size(nx->games); i++) {
        const game_file_t *file = list_get(nx->games, i);
        mapfile_close((mapfile_t*)file->file);
    }
    list_destroy(nx->games);

    hos_destroy(nx->hos);
    procinfo_destroy(nx->procinfo);
    fb_free(nx);
}