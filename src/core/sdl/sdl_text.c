#include <sdl/sdl_app.h>

#include <fs/file.h>
#include <types.h>
static SDL_Color text_color = {255, 255, 255, 255}; // white color (RGBA)
void sdl_app_printf(sdl_app_t *app, const int32_t text_x, const int32_t text_y, const char *fmt, ...) {
    va_list args = {};
    va_start(args, fmt);

    char buffer[1024];
    vsnprintf(buffer, 1024, fmt, args);

    if (!app->font) {
        if (!TTF_Init())
            quit("can't initialize the TTF library");

        file_t * libfont = file_open("fonts/LiberationSans-Regular.ttf", "r");
        if (libfont)
            app->font = TTF_OpenFont("fonts/LiberationSans-Regular.ttf", 48);
        file_close(libfont);
    }
    SDL_Surface * text_surface = TTF_RenderText_Blended(app->font, buffer, 0, text_color);

    const int32_t text_w = text_surface->w;
    const int32_t text_h = text_surface->h;
    const SDL_FRect text_rect = {text_x, text_y, text_w, text_h};

    if (!text_surface)
        quit("SDL ran out of memory");

    // combines what's in the surface with our text layer into a single image
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(app->renderer, text_surface);
    SDL_DestroySurface(text_surface);
    SDL_RenderTexture(app->renderer, text_texture, &text_rect, nullptr);
    SDL_DestroyTexture(text_texture);

    va_end(args);
}

