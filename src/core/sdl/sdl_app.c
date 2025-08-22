#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <types.h>
#include <sdl/sdl_app.h>
void sdl_app_join(sdl_app_t *app) {

    SDL_Event event;
    for (; app->state; app->frame_count++) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                return;
            if (!app->ev_callback(app->app_data, &event))
                app->state = false;
        }
        SDL_RenderClear(app->renderer);
        app->frame_callback(app->app_data);

        SDL_RenderPresent(app->renderer);

        SDL_Delay(16);
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

    SDL_CreateWindowAndRenderer("title", 640, 480, 0, &app->main_window, &app->renderer);
    app->state = true;

    return app;
}

void sdl_app_destroy(sdl_app_t *app) {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->main_window);
    SDL_Quit();

    fb_free(app);
}
