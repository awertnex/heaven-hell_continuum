#include <math.h>

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#include "../engine/h/defines.h"
#include "../engine/h/memory.h"
#include "../engine/h/logger.h"
#include "../engine/logger.c"

#define MC_C_OFF                    (Color){0x10, 0x10, 0x10, 0xff}
#define MC_C_GRAY                   (Color){0x40, 0x40, 0x40, 0xff}
#define MC_C_RED                    (Color){0xff, 0x78, 0x78, 0xff}
#define MC_C_GREEN                  (Color){0x78, 0xff, 0x78, 0xff}
#define MC_C_BLUE                   (Color){0x78, 0x78, 0xff, 0xff}
#define CHUNK_BUF_RADIUS            8
#define CHUNK_BUF_DIAMETER          ((CHUNK_BUF_RADIUS * 2) + 1)
#define CHUNK_BUF_ELEMENTS          (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)
#define CHUNK_TAB_CENTER            (CHUNK_BUF_RADIUS + (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER))
#define CHUNK_TAB_INDEX(x, y)       (x + (y * CHUNK_BUF_DIAMETER))
#define CHUNK_BUF_POS_X             50.0f
#define CHUNK_BUF_POS_Y             250.0f
#define CHUNK_TAB_POS_X             700.0f
#define CHUNK_TAB_POS_Y             CHUNK_BUF_POS_Y
#define RECT_SIZE                   32.0f
#define MARGIN                      2.0f
#define draw_chunk(x, y, col)                                                   \
    DrawRectangleRounded(                                                       \
        (Rectangle){                                                            \
        (f32)(((f32)(x) * (RECT_SIZE + MARGIN)) + CHUNK_TAB_POS_X),             \
        (f32)(((f32)(y) * (RECT_SIZE + MARGIN)) + CHUNK_TAB_POS_Y),             \
        RECT_SIZE, RECT_SIZE                                                    \
        },                                                                      \
        0.3f, 1, col)

#define draw_chunk_buf_index(i, col)                                            \
    DrawRectangleRounded(                                                       \
        (Rectangle){                                                            \
        (f32)(((i % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_BUF_POS_X),                 \
        (f32)((floorf((f32)(i) / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_BUF_POS_Y),    \
        RECT_SIZE, RECT_SIZE                                                    \
        },                                                                      \
        0.3f, 1, col)

#define draw_chunk_tab_index(i, col)                                                \
    DrawRectangleRounded(                                                       \
        (Rectangle){                                                            \
        (f32)(((i % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_TAB_POS_X),                 \
        (f32)((floorf((f32)(i) / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_TAB_POS_Y),    \
        RECT_SIZE, RECT_SIZE                                                    \
        },                                                                      \
        0.3f, 1, col)

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

// ---- declarations -----------------------------------------------------------
Font font;
f32 font_size = 32;
f32 font_size_small = 14;
u8 state = 0;
u8 render_distance = 1;
u8 data_view = 0;
Chunk *chunk_buf = {0};
Chunk *chunk_tab[CHUNK_BUF_ELEMENTS] = {0};
v2i16 player_chunk = {1, -1};

const v2f32 legend_pos = {CHUNK_TAB_POS_X, 20.0f};

// ---- signatures -------------------------------------------------------------
b8 is_distance_within(u16 distance, v2i32 start, v2i32 end);
void push_chunk_buf(v2i16 *player_chunk, v2i16 pos);
void update_chunk_buf(v2i16 *player_chunk);
void draw_chunk_buf(u16 i);
void init_chunking();
void free_chunking();
void parse_input();
void draw_gui();

void push_chunk_buf(v2i16 *player_chunk, v2i16 pos)
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_buf[i].state ^ STATE_CHUNK_LOADED)
        {
            memset(&chunk_buf[i], 0, sizeof(Chunk));
            chunk_buf[i].state |= STATE_CHUNK_LOADED;
            chunk_buf[i].pos =
                (v2i16){
                    player_chunk->x + (pos.x - CHUNK_BUF_RADIUS),
                    player_chunk->y + (pos.y - CHUNK_BUF_RADIUS)
                };
            chunk_tab[CHUNK_TAB_INDEX(pos.x, pos.y)] = &chunk_buf[i];
            return;
        }
    }
}

void update_chunk_buf(v2i16 *player_chunk)
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (is_distance_within(render_distance,
                    (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                    (v2i32){i % CHUNK_BUF_DIAMETER, (i16)floorf((f32)i / CHUNK_BUF_DIAMETER)}))
            if (chunk_tab[i] == NULL)
                push_chunk_buf(player_chunk,
                        (v2i16){
                        i % CHUNK_BUF_DIAMETER,
                        (i16)floorf((f32)i / CHUNK_BUF_DIAMETER)});
    }
}

void draw_chunk_buf(u16 i)
{
    draw_chunk_buf_index(i, MC_C_OFF);
    draw_chunk_tab_index(i, MC_C_OFF);

    if (&chunk_buf[i] != NULL)
    {
        draw_chunk_buf_index(i, MC_C_OFF);
        if (chunk_buf[i].state & STATE_CHUNK_LOADED)
        {
            draw_chunk_buf_index(i, MC_C_GREEN);
            DrawTextEx(font, TextFormat("%d", &chunk_buf[i] - &chunk_buf[0]),
                    (Vector2){
                    CHUNK_BUF_POS_X + ((i % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)),
                    CHUNK_BUF_POS_Y + (floorf((f32)i / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN))
                    },
                    font_size_small, 1, BLACK);
        }
    }

    if (is_distance_within(render_distance,
                (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                (v2i32){i % CHUNK_BUF_DIAMETER, (i32)floorf((f32)i / CHUNK_BUF_DIAMETER)}))
        draw_chunk_tab_index(i, MC_C_RED);

    if (chunk_tab[i] != NULL)
    {
        if (chunk_tab[i]->state & STATE_CHUNK_LOADED)
        {
            if (is_distance_within(render_distance,
                        (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                        (v2i32){i % CHUNK_BUF_DIAMETER, (i32)floorf((f32)i / CHUNK_BUF_DIAMETER)}))
                draw_chunk_tab_index(i, MC_C_GREEN);
            else
                draw_chunk_tab_index(i, MC_C_BLUE);

            if (data_view)
                DrawTextEx(font, TextFormat("%d,%d", chunk_tab[i]->pos.x, chunk_tab[i]->pos.y),
                        (Vector2){
                        CHUNK_TAB_POS_X + ((i % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)),
                        CHUNK_TAB_POS_Y + (floorf((f32)i / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN))
                        },
                        font_size_small, 1, BLACK);
            else
                DrawTextEx(font, TextFormat("%d", chunk_tab[i] - &chunk_buf[0]),
                        (Vector2){
                        CHUNK_TAB_POS_X + ((i % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)),
                        CHUNK_TAB_POS_Y + (floorf((f32)i / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN))
                        },
                        font_size_small, 1, BLACK);
        }
    }
}

b8 is_distance_within(u16 distance, v2i32 start, v2i32 end)
{
    if (powf(start.x - end.x, 2) + powf(start.y - end.y, 2) < (distance * distance) + 2)
        return TRUE;
    return FALSE;
}

int main(void)
{
    // ---- main_init ----------------------------------------------------------
    state = (0 | STATE_ACTIVE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1400, 920, "Chunk Pointer Table");
    SetExitKey(KEY_Q);

    font = LoadFont("fonts/code_saver_regular.otf");
    init_chunking();

    while ((state & STATE_ACTIVE) && (!WindowShouldClose()))
    {
        // ---- main_loop ------------------------------------------------------

        BeginDrawing();
        ClearBackground(BLACK);
        parse_input();

        for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
            draw_chunk_buf(i);

        draw_gui();

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
    return;

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
    if ((IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) && render_distance < CHUNK_BUF_RADIUS)
        ++render_distance;

    if ((IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) && render_distance > 1)
        --render_distance;

    if (IsKeyPressed(KEY_ENTER))
    {
        for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
            update_chunk_buf(&player_chunk);
    }

    if (IsKeyPressed(KEY_TAB))
        data_view = ~data_view;

}

void draw_gui()
{
    // ---- title --------------------------------------------------------------
    DrawTextEx(font, "Chunk Buf",
            (Vector2){CHUNK_BUF_POS_X, CHUNK_BUF_POS_Y - font_size},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Chunk Tab",
            (Vector2){CHUNK_TAB_POS_X, CHUNK_TAB_POS_Y - font_size},
            font_size, 1, MC_C_GRAY);

    // ---- info ---------------------------------------------------------------
    DrawTextEx(font, TextFormat("%s\n%s\n%s\n%s",
                "ENTER:  load chunks within render distance",
                "UP:     increase render distance",
                "DOWN:   decrease render distance",
                "TAB:    change data view: index/chunk pos"),
            (Vector2){10.0f, 10.0f},
            font_size, 1, MC_C_BLUE);

    DrawTextEx(font, TextFormat("Render Distance: %d", render_distance),
            (Vector2){10.0f, 150.0f},
            font_size, 1, MC_C_GREEN);

    // ---- legend -------------------------------------------------------------
    DrawRectangleRounded((Rectangle){legend_pos.x, legend_pos.y, RECT_SIZE / 2, RECT_SIZE / 2},
            0.3f, 1, MC_C_OFF);

    DrawRectangleRounded((Rectangle){legend_pos.x, legend_pos.y + ((RECT_SIZE / 2) * 2), (RECT_SIZE / 2), (RECT_SIZE / 2)},
            0.3f, 1, MC_C_GREEN);

    DrawRectangleRounded((Rectangle){legend_pos.x, legend_pos.y + ((RECT_SIZE / 2) * 4), (RECT_SIZE / 2), (RECT_SIZE / 2)},
            0.3f, 1, MC_C_RED);

    DrawRectangleRounded((Rectangle){legend_pos.x, legend_pos.y + ((RECT_SIZE / 2) * 6), (RECT_SIZE / 2), (RECT_SIZE / 2)},
            0.3f, 1, MC_C_BLUE);

    DrawTextEx(font, "Usable/Not Null",
            (Vector2){
            legend_pos.x + (RECT_SIZE + MARGIN),
            legend_pos.y},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Chunk Loaded",
            (Vector2){
            legend_pos.x + (((RECT_SIZE / 2) + MARGIN) * 2),
            legend_pos.y + ((RECT_SIZE / 2) * 2)},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Render Surface",
            (Vector2){
            legend_pos.x + (((RECT_SIZE / 2) + MARGIN) * 2),
            legend_pos.y + ((RECT_SIZE / 2) * 4)},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Chunk Loaded, but Out of Render Surface",
            (Vector2){
            legend_pos.x + (((RECT_SIZE / 2) + MARGIN) * 2),
            legend_pos.y + ((RECT_SIZE / 2) * 6)},
            font_size, 1, MC_C_GRAY);

    // ---- ruler --------------------------------------------------------------
    DrawTextEx(font, "X",
            (Vector2){
            CHUNK_TAB_POS_X + (CHUNK_BUF_DIAMETER * (RECT_SIZE + MARGIN)) + RECT_SIZE,
            CHUNK_TAB_POS_Y - RECT_SIZE},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Y",
            (Vector2){
            CHUNK_TAB_POS_X - RECT_SIZE,
            CHUNK_TAB_POS_Y + (CHUNK_BUF_DIAMETER * (RECT_SIZE + MARGIN)) + RECT_SIZE},
            font_size, 1, MC_C_GRAY);

    for (int i = 0; i < CHUNK_BUF_DIAMETER; ++i)
    {
        DrawTextEx(font, TextFormat("%d", i),
            (Vector2){
                CHUNK_TAB_POS_X + (CHUNK_BUF_DIAMETER * (RECT_SIZE + MARGIN)) + RECT_SIZE,
                CHUNK_TAB_POS_Y + (i * (RECT_SIZE + MARGIN)) + 4.0f},
                font_size, 1, MC_C_GRAY);

        DrawTextEx(font, TextFormat("%d", i),
                (Vector2){
                CHUNK_TAB_POS_X + (i * (RECT_SIZE + MARGIN)) + 4.0f,
                CHUNK_TAB_POS_Y + (CHUNK_BUF_DIAMETER * (RECT_SIZE + MARGIN)) + RECT_SIZE},
                font_size, 1, MC_C_GRAY);
    }
}

