#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#include "../engine/h/defines.h"
#include "../engine/h/memory.h"
#include "../engine/h/logger.h"
#include "../engine/logger.c"

#define MC_C_RED                (Color){0xff, 0x78, 0x78, 0xff}
#define MC_C_GREEN              (Color){0x78, 0xff, 0x78, 0xff}
#define MC_C_BLUE               (Color){0x78, 0x78, 0xff, 0xff}
#define CHUNK_BUF_SIZE          289
#define CHUNK_BUF_RADIUS        8
#define CHUNK_BUF_DIAMETER      ((CHUNK_BUF_RADIUS * 2) + 1)
#define CHUNK_BUF_ELEMENTS      (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)
#define CHUNK_TAB_CENTER        ((CHUNK_BUF_RADIUS + 1) + ((CHUNK_BUF_RADIUS + 1) * CHUNK_BUF_DIAMETER))
#define CHUNK_TAB_INDEX(x, y)   (x + (y * CHUNK_BUF_DIAMETER))
#define CHUNK_TAB_POS_X         300.0f
#define CHUNK_TAB_POS_Y         (720.0f / 2.0f)
#define rect(x, y, col)         DrawRectangleRounded((Rectangle){x + CHUNK_TAB_POS_X, y + CHUNK_TAB_POS_Y, 16.0f, 16.0f}, 0.3f, 1, col)

enum ChunkStates
{
    STATE_ACTIVE =          0x1,

    STATE_CHUNK_LOADED =    0x1,
    STATE_CHUNK_DIRTY =     0x2,
}; /* Chunk States */

typedef struct Chunk
{
    v2i16 pos;
    u32 id;
    u32 i[24][24];
    u8 state;
} Chunk;

u8 state = 0;
u8 render_distance = 1;
Chunk *chunk_buf = {0};
void *chunk_tab[CHUNK_BUF_ELEMENTS] = {0};

void init_chunking();
void free_chunking();
void parse_input();

void parse_chunks()
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_buf[i].state & STATE_CHUNK_LOADED)
            rect((f32)i, 0.0f, MC_C_RED);
    }
    rect(16.0f, 0.0f, MC_C_GREEN);
    rect(32.0f, 0.0f, MC_C_BLUE);
}

int main(void)
{
    // ---- main_init ----------------------------------------------------------
    state = 0 | STATE_ACTIVE;
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Chunk Pointer Table");
    SetExitKey(KEY_Q);

    init_chunking();

    while ((state & STATE_ACTIVE) && (!WindowShouldClose()))
    {
        // ---- main_loop ------------------------------------------------------

        BeginDrawing();
        ClearBackground(BLACK);
        parse_input();

        printf("%s\n", "what?"); //temp
        DrawText(TextFormat("Render Distance: %d", render_distance), 200.0f, 200.0f, 16, RAYWHITE);
        parse_chunks();

        EndDrawing();
    }

    // ---- main_close ---------------------------------------------------------
    CloseWindow();
    free_chunking();
    return 0;
}

void init_chunking()
{
    MC_C_ALLOC(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));

cleanup:
    free_chunking();
    exit(-1);
}

void free_chunking()
{
    MC_C_FREE(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
}

void parse_input()
{
    if (IsKeyPressed(KEY_ONE))
        render_distance = 1;

    if (IsKeyPressed(KEY_TWO))
        render_distance = 2;

    if (IsKeyPressed(KEY_THREE))
        render_distance = 3;

    if (IsKeyPressed(KEY_FOUR))
        render_distance = 4;

    if (IsKeyPressed(KEY_FIVE))
        render_distance = 5;

    if (IsKeyPressed(KEY_SIX))
        render_distance = 6;

}
