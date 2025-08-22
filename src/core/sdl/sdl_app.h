#pragma once

#include <SDL3/SDL_events.h>
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;

typedef bool(*ev_callback_t)(void *, const SDL_Event*);
typedef void(*frame_callback_t)(void *);

typedef struct sdl_app {
    SDL_Window *main_window;
    SDL_Renderer *renderer;

    ev_callback_t ev_callback;
    frame_callback_t frame_callback;

    bool state;
    uint64_t frame_count;

    void *app_data;
} sdl_app_t;

sdl_app_t *sdl_app_create(void *, ev_callback_t, frame_callback_t);
void sdl_app_join(sdl_app_t *);
void sdl_app_destroy(sdl_app_t *);