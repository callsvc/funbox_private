#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <types.h>
#include <sdl/sdl_app.h>

void sdl_app_count(sdl_app_t *, bool);
void sdl_app_join(sdl_app_t *app) {

    SDL_Event event;
    SDL_SetRenderVSync(app->renderer, 1);
    for (; app->state; app->frame_count++) {
        sdl_app_count(app, false);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                return;
            if (!app->ev_callback(app->app_data, &event))
                app->state = false;
        }
        SDL_RenderClear(app->renderer);
        app->frame_callback(app->app_data);

        if (app->show_fps)
            sdl_app_printf(app, 0, 0, "fps: %u frame: %lu", (int32_t)app->fps, app->frame_count);

        SDL_RenderPresent(app->renderer);
        SDL_Delay(16);

        sdl_app_count(app, true);
    }
}

sdl_app_t *sdl_app_create(void *userdata, const ev_callback_t callback, const frame_callback_t frame_callback) {
    sdl_app_t *app = fb_malloc(sizeof(sdl_app_t));
    app->app_data = userdata;
    app->ev_callback = callback;
    app->frame_callback = frame_callback;

    SDL_SetAppMetadata("funbox", "version", "com.callsvc.funbox");

    if (!SDL_Init(SDL_INIT_VIDEO))
        quit("couldn't initialize SDL: %s", SDL_GetError());

    SDL_CreateWindowAndRenderer("untitle", 640, 480, SDL_WINDOW_HIGH_PIXEL_DENSITY, &app->main_window, &app->renderer);
    app->state = true;
    app->show_fps = true;

    return app;
}
void sdl_app_settitle(const sdl_app_t *app, const char *title) {
    SDL_SetWindowTitle(app->main_window, title);
}

void sdl_app_destroy(sdl_app_t *app) {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->main_window);

    if (app->font) {
        TTF_CloseFont(app->font);
        TTF_Quit();
    }
    SDL_Quit();

    fb_free(app);
}
