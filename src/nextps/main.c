#include <assert.h>
#include <ps.h>
#include <cpu.h>
#include <stdio.h>

#include <types.h>
#include <sdl/sdl_app.h>

#include <fs/types.h>

typedef struct psx {
    bus_t * bus;
    cpu_t *cpu;

    SDL_Keycode last_key;
} psx_t;
procinfo_t *nextps;
bool nextps_updateinput(void *arg, const SDL_Event *event) {
    if (event->type == SDL_EVENT_KEY_DOWN)
        if (event->key.key == SDLK_ESCAPE)
            return false;
    ((psx_t*)((procinfo_t*)arg)->user_ptr)->last_key = event->key.key;

    return true;
}

void nextps_stepframe(void *arg) {
    const psx_t *psx = (psx_t *)((procinfo_t*)arg)->user_ptr;

    for (size_t ticks = 0; !psx->cpu->maskint && ticks < 10; ticks++) {
        cpu_run(psx->cpu);
        if (psx->cpu->pc == 0xBFC80000)
            break;
    }
}

int nextps_main(procinfo_t *proc, const int32_t argc, const char **argv) {
    assert(strcmp(argv[argc - 2], "nextps") == 0);
    nextps = proc;

    psx_t * psxone = fb_malloc(sizeof(psxone));
    psxone->bus = bus_create();
    psxone->cpu = cpu_create(psxone->bus);

    fprintf(stderr, "app cache dir: %s\n", fs_get_cache(nextps));

    cpu_reset(psxone->cpu);

    proc->user_ptr = psxone;
    sdl_app_join(proc->sdl_toolkit);

    cpu_destroy(psxone->cpu);
    bus_destroy(psxone->bus);

    fb_free(psxone);
    return 0;
}
