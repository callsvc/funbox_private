#include <dlfcn.h>
#include <fs/file.h>
#include <types.h>

#include <applet.h>
const char * applet_name[] = {"finish", "heyds", "neszinho", "nextps", "playme", "psever", "reality64", "redos", "smallgba", "starsys2", "supernx", nullptr};

void *applet_loadlibrary(applet_type_e);
applet_info_t *applet_create(const applet_type_e type) {
    applet_info_t *applet = fb_malloc(sizeof(applet_info_t));
    applet->type = type;
    applet->base = applet_loadlibrary(type);

    char funcsym[0x100];
    sprintf(funcsym, "%s_main", applet_name[type]);
    applet->main = dlsym(applet->base, funcsym);

    sprintf(funcsym, "%s_stepframe", applet_name[type]);
    applet->stepframe = dlsym(applet->base, funcsym);

    sprintf(funcsym, "%s_updateinput", applet_name[type]);
    applet->updateinput = dlsym(applet->base, funcsym);

    return applet;
}

void applet_destroy(applet_info_t *applet) {
    if (applet->base) {
        dlclose(applet->base);
    }

    fb_free(applet);
}

void *applet_loadlibrary(const applet_type_e type) {
    char applet_path[0x100];
    sprintf(applet_path, "./lib%s.so", applet_name[type]);

    if (!fs_exists(applet_path))
        quit("applet not found: %s", applet_path);

    void *library = dlopen(applet_path, RTLD_NOW);
    if (!library)
        quit("can't load the applet because: %s", dlerror());
    return library;
}
