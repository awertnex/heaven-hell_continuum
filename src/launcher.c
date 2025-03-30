#include <raylib.h>

#include "h/launcher.h"
#include "h/keymaps.h"

v2i16 render_size = {500, 720};

void init_launcher()
{
    state |= STATE_LAUNCHER;
    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(render_size.x, render_size.y, "minecraft.c Launcher");
    SetWindowState(FLAG_WINDOW_UNDECORATED);
}

void update_launcher()
{
    BeginDrawing();
    ClearBackground(COL_LAUNCHER_BG);

    //TODO: fix segfault while calling draw_text_centered() but not DrawText()
    draw_text_centered(font_regular,
            MC_C_VERSION,
            (v2i16){(f32)render_size.x/2, 20},
            font_size, 2, COL_TEXT_DEFAULT, 1);

    EndDrawing();
}

u8 close_launcher()
{
    CloseWindow();
    //TODO: if error, return non 0 and terminate active
    return 0;
}

void update_launcher_input()
{
    if (IsKeyPressed(BIND_QUIT))
    {
        state &= ~STATE_ACTIVE;
    }
}
