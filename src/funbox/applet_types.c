#include <fs/file.h>
#include <applet.h>

bool applet_nextps_isvalid(const char *filename) {
    char psxbin[0x80];
    strncpy(psxbin, filename, strlen(filename) - 4);
    strcat(psxbin, ".bin");

    return fs_exists(psxbin);
}
bool applet_nextps_isiso(const char *filename) {
    file_t *isofile = file_open(filename, "r");
    char isobuffer[0x32];
    fs_read((fsfile_t*)isofile, isobuffer, sizeof(isobuffer), 0x8020);

    const bool result = strstr(isobuffer, "PLAYSTATION") != nullptr;

    file_close(isofile);
    return result;
}

typedef bool (*criterias_t)(const char*);
struct applet_type_info {
    applet_type_e type;
    size_t count;
    const char **types;
    criterias_t *criterias;
};

const struct applet_type_info applets_info_list[] = {
    {APPLET_TYPE_NEXTPS, 3, (const char*[]){".bin", ".iso", ".cue"}, (criterias_t[]){nullptr, applet_nextps_isiso, applet_nextps_isvalid}}
};

applet_type_e applet_get_type_by_extension(const char *filename) {
    for (size_t applet_i = 0; applet_i < count_of(applets_info_list); applet_i++) {
        const struct applet_type_info *applet_info = &applets_info_list[applet_i];
        for (size_t i = 0; i < applet_info->count; i++) {
            if (strcmp(filename + strlen(filename) - 4, applet_info->types[i]) != 0)
                continue;

            if (applet_info->criterias[i] != nullptr)
                if (!applet_info->criterias[i](filename))
                    continue;
            return applet_info->type;
        }
    }
    return APPLET_TYPE_NONE;
}

applet_type_e applet_get_type(const char *filename) {
    file_t *file = file_open(filename, "r");
    if (!file)
        return APPLET_TYPE_NONE;
    const applet_type_e type = applet_get_type_by_extension(filename);

    if (fs_getsize((fsfile_t*)file) <= 512 * 1024 * 1024) {
        if (type == APPLET_TYPE_NEXTPS)
            return APPLET_TYPE_NEXTPS;
    }
    file_close(file);
    return type;
}