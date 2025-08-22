#pragma once
#include <SDL3/SDL_events.h>

#include <types.h>
typedef enum applet_type {
    APPLET_TYPE_FINESH,
    APPLET_TYPE_HEYDS,
    APPLET_TYPE_NESZINHO,
    APPLET_TYPE_NEXTPS,
    APPLET_TYPE_PLAYME,
    APPLET_TYPE_PSEVER,
    APPLET_TYPE_REALITY64,
    APPLET_TYPE_REDOS,
    APPLET_TYPE_SMALLGBA,
    APPLET_TYPE_STARSYS2,
    APPLET_TYPE_SUPERNX,
    APPLET_TYPE_NONE,
} applet_type_e;

typedef int32_t (*func_main_t)(procinfo_t*, int32_t, const char **);
typedef void (*func_stepframe_t)(void *);
typedef bool (*func_updateinput_t)(void *, const SDL_Event *);

typedef struct applet_info {
    applet_type_e type;
    void *base;

    func_main_t main;
    func_stepframe_t stepframe;
    func_updateinput_t updateinput;
} applet_info_t;

extern const char * applet_name[];

applet_type_e applet_get_type(const char *);

applet_info_t *applet_create(applet_type_e);
void applet_destroy(applet_info_t *);