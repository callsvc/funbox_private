#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef bool(*ev_callback_t)(void *, const SDL_Event*);
typedef void(*frame_callback_t)(void *);

typedef struct sdl_app {
    SDL_Window *main_window;
    SDL_Renderer *renderer;
    TTF_Font *font;

    ev_callback_t ev_callback;
    frame_callback_t frame_callback;

    bool state;
    uint64_t frame_count;

    uint64_t last_frame;
    float fps;

    void *app_data;

    bool show_fps;
} sdl_app_t;

sdl_app_t *sdl_app_create(void *, ev_callback_t, frame_callback_t);
void sdl_app_join(sdl_app_t *);
void sdl_app_printf(sdl_app_t*, int32_t, int32_t, const char *, ...);
void sdl_app_destroy(sdl_app_t *);