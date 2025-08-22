#include <sdl/sdl_app.h>
#include <SDL3/SDL_timer.h>

void sdl_app_count(sdl_app_t *app, const bool finish) {
    if (!finish) {
        app->last_frame = SDL_GetPerformanceCounter();
        return;
    }
    const uint64_t end_frame = SDL_GetPerformanceCounter();
    app->fps = 1.0f / ((float)(end_frame - app->last_frame) / SDL_GetPerformanceFrequency());
}
