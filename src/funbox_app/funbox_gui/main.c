#include <stdbool.h>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

int main() {
    InitWindow(600, 600, "funbox");
    SetTargetFPS(60);

    bool show_messagebox = false;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message"))
            show_messagebox = true;

        if (show_messagebox) {
            const int result = GuiMessageBox((Rectangle){ 85, 70, 250, 100 }, "#191#Message Box", "Hi! This is a message!", "Nice;Cool");
            if (result >= 0)
                show_messagebox = false;
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}