

#include <applet.h>

#include <sdl/sdl_app.h>

#include <fs/dir.h>
char* fb_getrom(const char *rom_path, applet_type_e *type) {
    const char *user_list[] = { "~/Documents/games/nextps" };
    *type = APPLET_TYPE_NONE;

    char *result = nullptr;
    for (size_t i = 0; !fs_exists(rom_path) && i < count_of(user_list); i++) {
        char userpath[PATH_MAX];
        strcpy(userpath, "/home/correia");
        strcat(userpath, user_list[i] + 1);

        if (!fs_exists(userpath))
            create_directories(userpath, false);

        if (strstr(userpath, rom_path) == nullptr)
            continue;
        // dir_t *games = dir_open(userpath, "r");
        // vector_t *files = fs_list_all_files((fsdir_t*)games);
        // dir_close(games);
        vector_t *files = list_all_files(userpath);

        if (vector_size(files)) {
            result = fs_build_path(2, userpath, vector_get(files, 0));
        }
        vector_destroy(files);
    }

    if (fs_exists(rom_path))
        result = fb_strdup(rom_path);

    if (result)
        *type = applet_get_type(result);
    return result;
}

int main(const int argc, char **argv) {
    const char *rom_path = argc > 1 ? argv[1] : (const char*)nullptr;
    applet_type_e type;
    char * applet_arg = fb_getrom(rom_path, &type);

    procinfo_t *info = procinfo_create();

    applet_info_t *applet = applet_create(type);

    const char * applet_args[] = {applet_name[type], applet_arg, nullptr};

    strcpy(info->libname, applet_args[0]);
    info->sdl_toolkit = sdl_app_create(info, applet->updateinput, applet->stepframe);

    applet->main(info, count_of(applet_args) - 1, applet_args);
    fb_free(applet_arg);

    sdl_app_destroy(info->sdl_toolkit);

    applet_destroy(applet);
    procinfo_destroy(info);
}
