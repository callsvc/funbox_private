#include <ps.h>
#include <cpu.h>

#include <types.h>
#include <sdl/sdl_app.h>
typedef struct psx {
    bus_t * bus;
    cpu_t *cpu;
} psx_t;

bool update_input(__attribute__((unused)) void *arg, const SDL_Event *event) {
    if (event->type == SDL_EVENT_KEY_DOWN)
        if (event->key.key == SDLK_ESCAPE)
            return false;
    return true;
}

void update_frame(void *arg) {
    const psx_t *psx = (psx_t *)arg;

    for (size_t ticks = 0; !psx->cpu->maskint && ticks < 10; ticks++) {
        cpu_run(psx->cpu);
        if (psx->cpu->pc == 0xBFC80000)
            break;
    }
}

int main() {

    psx_t * psxone = fb_malloc(sizeof(psxone));
    psxone->bus = bus_create();
    psxone->cpu = cpu_create(psxone->bus);

    cpu_reset(psxone->cpu);
    sdl_app_t *app = sdl_app_create(psxone, update_input, update_frame);

    sdl_app_join(app);

    cpu_destroy(psxone->cpu);
    bus_destroy(psxone->bus);

    sdl_app_destroy(app);
    fb_free(psxone);
}
