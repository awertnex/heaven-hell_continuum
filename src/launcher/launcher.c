#include <raylib.h>

#include "launcher.h"
#include "../h/logger.h"

u8 state = 0;
v2i16 render_size = {500, 720};

void init_launcher()
{
    state |= STATE_ACTIVE;
    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(render_size.x, render_size.y, "minecraft.c Launcher");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    if (LOGGING_DEBUG)
        SetWindowState(FLAG_WINDOW_TOPMOST);
}

int main(void) // ---- main ----------------------------------------------------
{
    init_launcher();

    while (state & STATE_ACTIVE)
    {
        update_launcher_input();
        update_launcher();
    }

    CloseWindow();
    return 0;
}

void update_launcher()
{
    if (IsWindowResized())
        render_size = (v2i16){GetRenderWidth(), GetRenderHeight()};

    BeginDrawing();
    ClearBackground(COL_BG);

    EndDrawing();
}

void update_launcher_input()
{
    if (IsKeyPressed(KEY_Q))
        state &= ~STATE_ACTIVE;
}
