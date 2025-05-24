#include <math.h>

#include "../dependencies/raylib-5.5/include/raylib.h"
#include "../dependencies/raylib-5.5/include/rlgl.h"

#define VECTOR2_TYPES
#include "../engine/h/defines.h"
#include "../engine/h/memory.h"
#include "../engine/logger.c"

#define MC_C_OFF                    (Color){0x10, 0x10, 0x10, 0xff}
#define MC_C_GRAY                   (Color){0x40, 0x40, 0x40, 0xff}
#define MC_C_RED                    (Color){0xff, 0x78, 0x78, 0xff}
#define MC_C_OVERWRITE              (Color){0xff, 0x80, 0x37, 0xff}
#define MC_C_GREEN                  (Color){0x78, 0xff, 0x78, 0xff}
#define MC_C_LIGHT_GREEN            (Color){0x78, 0xff, 0x78, 0xb8}
#define MC_C_BLUE                   (Color){0x78, 0x78, 0xff, 0xff}

#define CHUNK_BUF_RADIUS            8
#define CHUNK_BUF_DIAMETER          ((CHUNK_BUF_RADIUS * 2) + 1)
#define CHUNK_BUF_ELEMENTS          (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)
#define CHUNK_TAB_CENTER            (CHUNK_BUF_RADIUS + (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER))

#define CHUNK_BUF_POS_X             50.0f
#define CHUNK_BUF_POS_Y             250.0f
#define CHUNK_TAB_POS_X             700.0f
#define CHUNK_TAB_POS_Y             CHUNK_BUF_POS_Y
#define RECT_SIZE                   32.0f
#define MARGIN                      2.0f
#define LEGEND_POS_X                CHUNK_TAB_POS_X
#define LEGEND_POS_Y                20.0f

#define draw_chunk_buf_index(i, col)                                            \
    DrawRectangleRounded(                                                       \
        (Rectangle){                                                            \
        (f32)((((i) % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_BUF_POS_X),               \
        (f32)((floorf((f32)(i) / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_BUF_POS_Y),    \
        RECT_SIZE, RECT_SIZE},                                                  \
        0.3f, 1, col)

#define draw_chunk_tab_index(i, col)                                            \
    DrawRectangleRounded(                                                       \
        (Rectangle){                                                            \
        (f32)((((i) % CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_TAB_POS_X),               \
        (f32)((floorf((f32)(i) / CHUNK_BUF_DIAMETER) * (RECT_SIZE + MARGIN)) + CHUNK_TAB_POS_Y),    \
        RECT_SIZE, RECT_SIZE},                                                  \
        0.3f, 1, col)

enum ChunkFlags
{
    FLAG_ACTIVE =           0x01,

    FLAG_CHUNK_LOADED =     0x01,
    FLAG_CHUNK_RENDER =     0x02,
    FLAG_CHUNK_DIRTY =      0x04,
}; /* ChunkFlags */

typedef struct /* Chunk */
{
    v2i16 pos;                                  // (world X Y) / CHUNK_DIAMETER
    u32 id;                                     // (pos.x << 16) + pos.y
    u32 i[24 * 24];
    Model model;
    Material mat;
    u8 flag;
    u8 edge_value;                              // chunk marking for chunk_tab shifting logic
    u32 block_parse_limit;                      // final occurrence of non-air blocks in chunk
} Chunk;

// ---- declarations -----------------------------------------------------------
Font font;
f32 font_size = 32;
f32 font_size_small = 14;
u8 flag = 0;
u8 render_distance = 1;
u8 data_view = 0;
Chunk *chunk_buf = {0};
Chunk *chunk_tab[CHUNK_BUF_ELEMENTS] = {NULL};
v2u16 chunk_tab_coordinates = {0};
u8 chunk_parse_lock = 0;
u8 shift_direction = 1;
v2i16 player_chunk = {0};
v2i16 player_delta_chunk = {0};

// ---- signatures -------------------------------------------------------------
b8 is_distance_within(u16 distance, v2i32 start, v2i32 end);
Chunk *push_chunk_buf(v2i16 player_chunk, v2u16 pos);
void update_chunk_tab(v2i16 player_chunk);
void shift_chunk_tab(v2i16 player_chunk, v2i16 *player_delta_chunk);
void draw_chunk_buf(u32 i);
void init_chunking();
void free_chunking();
void parse_input();
void draw_gui();

Chunk *push_chunk_buf(v2i16 player_delta_chunk, v2u16 pos)
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
        if (!(chunk_buf[i].flag & FLAG_CHUNK_LOADED))
        {
            memset(&chunk_buf[i], 0, sizeof(Chunk));
            chunk_buf[i].flag = FLAG_CHUNK_LOADED;
            chunk_buf[i].pos =
                (v2i16){
                    player_delta_chunk.x + (pos.x - CHUNK_BUF_RADIUS),
                    player_delta_chunk.y + (pos.y - CHUNK_BUF_RADIUS)
                };
            return &chunk_buf[i];
        }
    return NULL;
}

void update_chunk_tab(v2i16 player_delta_chunk)
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        chunk_tab_coordinates = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};
        if (is_distance_within(render_distance,
                    (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                    (v2i32){chunk_tab_coordinates.x, chunk_tab_coordinates.y}))
        {
            if (chunk_tab[i] == NULL)
                chunk_tab[i] = push_chunk_buf(player_delta_chunk, chunk_tab_coordinates);
        }
        else if (chunk_tab[i] != NULL)
            if (chunk_tab[i]->flag & FLAG_CHUNK_LOADED)
            {
                memset(chunk_tab[i], 0, sizeof(Chunk));
                chunk_tab[i] = NULL;
            }
    }
}

void shift_chunk_tab(v2i16 player_chunk, v2i16 *player_delta_chunk)
{
    if (chunk_parse_lock) return;
    chunk_parse_lock = 1;

    v2i16 delta = {
        player_chunk.x - player_delta_chunk->x,
        player_chunk.y - player_delta_chunk->y};
    u8 direction =
        (delta.x > 0) ? 1 : (delta.x < 0) ? 2 :
        (delta.y > 0) ? 3 : (delta.y < 0) ? 4 : 0;
    i8 increment = (direction == 1 || direction == 3) - (direction == 2 || direction == 4);
    v2u16 index = {0};
    u16 mirror_index = 0;
    u16 target_index = 0;
    u8 is_on_edge = 0;

    player_delta_chunk->x += ((direction == 1) - (direction == 2));
    player_delta_chunk->y += ((direction == 3) - (direction == 4));

    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
        if (chunk_tab[i] != NULL)
            chunk_tab[i]->edge_value = 0;

    // ---- mark chunks on-edge ------------------------------------------------
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_tab[i] == NULL) continue;
        index = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};
        switch (direction)
        {
            case 1: // ---- positive x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (index.x * 2);
                is_on_edge =
                    (index.x == 0) ? 1 : (chunk_tab[i - 1] == NULL);
                break;

            case 2: // ---- negative x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (index.x * 2);
                is_on_edge =
                    (index.x == CHUNK_BUF_DIAMETER - 1) ? 1 : (chunk_tab[i + 1] == NULL);
                break;

            case 3: // ---- positive y -----------------------------------------
                mirror_index =
                    ((CHUNK_BUF_DIAMETER - 1 - index.y) * CHUNK_BUF_DIAMETER) + index.x;
                is_on_edge =
                    (index.y == 0) ? 1 : (chunk_tab[i - CHUNK_BUF_DIAMETER] == NULL);
                break;

            case 4: // ---- negative y -----------------------------------------
                mirror_index =
                    ((CHUNK_BUF_DIAMETER - 1 - index.y) * CHUNK_BUF_DIAMETER) + index.x;
                is_on_edge =
                    (index.y == CHUNK_BUF_DIAMETER - 1) ? 1 :
                    (chunk_tab[i + CHUNK_BUF_DIAMETER] == NULL);
                break;
        }

        if (is_on_edge)
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_RENDER;
            chunk_tab[i]->flag &= ~FLAG_CHUNK_LOADED;
            if (chunk_tab[mirror_index] != NULL)
                chunk_tab[mirror_index]->edge_value = 1;
        }
    }

    // ---- shift chunk_tab ----------------------------------------------------
    for (u16 i = (increment == 1) ? 0 : CHUNK_BUF_ELEMENTS - 1;
            i >= 0 && i < CHUNK_BUF_ELEMENTS;
            i += increment)
    {
        if (chunk_tab[i] == NULL) continue;
        index = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};
        switch (direction)
        {
            case 1: // ---- positive x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (index.x * 2);
                target_index = (index.x == CHUNK_BUF_DIAMETER - 1) ? i : i + 1;
                break;

            case 2: // ---- negative x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (index.x * 2);
                target_index = (index.x == 0) ? i : i - 1;
                break;

            case 3: // ---- positive y -----------------------------------------
                mirror_index = 
                    ((CHUNK_BUF_DIAMETER - 1 - index.y) * CHUNK_BUF_DIAMETER) + index.x;
                target_index = (index.y == CHUNK_BUF_DIAMETER - 1) ? i : i + CHUNK_BUF_DIAMETER;
                break;

            case 4: // ---- negative y -----------------------------------------
                mirror_index =
                    ((CHUNK_BUF_DIAMETER - 1 - index.y) * CHUNK_BUF_DIAMETER) + index.x;
                target_index = (index.y == 0) ? i : i - CHUNK_BUF_DIAMETER;
                break;
        }

        chunk_tab[i] = chunk_tab[target_index];
        if (chunk_tab[i]->edge_value)
            chunk_tab[target_index] = NULL;
    }
    update_chunk_tab(*player_delta_chunk);
    chunk_parse_lock = 0;
}

void draw_chunk_buf(u32 i)
{
    chunk_tab_coordinates = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};

    draw_chunk_buf_index(i, MC_C_OFF);
    if (&chunk_buf[i] != NULL)
    {
        draw_chunk_buf_index(i, MC_C_OFF);
        if (chunk_buf[i].flag & FLAG_CHUNK_LOADED)
        {
            if (chunk_buf[i].edge_value)
                draw_chunk_buf_index(i, MC_C_LIGHT_GREEN);
            else
                draw_chunk_buf_index(i, MC_C_GREEN);

            DrawTextEx(font, TextFormat(" %d", &chunk_buf[i] - &chunk_buf[0]),
                    (Vector2){
                    CHUNK_BUF_POS_X + (chunk_tab_coordinates.x * (RECT_SIZE + MARGIN)),
                    CHUNK_BUF_POS_Y + (chunk_tab_coordinates.y * (RECT_SIZE + MARGIN))
                    },
                    font_size_small, 1, BLACK);
        }
    }

    draw_chunk_tab_index(i, MC_C_OFF);
    if ((chunk_tab[i] == NULL)
            && is_distance_within(render_distance,
                (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                (v2i32){chunk_tab_coordinates.x, chunk_tab_coordinates.y}))
        draw_chunk_tab_index(i, MC_C_RED);

    if (chunk_tab[i] != NULL)
    {
        if (chunk_tab[i]->edge_value)
            draw_chunk_tab_index(i, MC_C_LIGHT_GREEN);
        else
            draw_chunk_tab_index(i, MC_C_GREEN);

        if (data_view)
            DrawTextEx(font, TextFormat(" %d\n %d", chunk_tab[i]->pos.x, chunk_tab[i]->pos.y),
                    (Vector2){
                    CHUNK_TAB_POS_X + (chunk_tab_coordinates.x * (RECT_SIZE + MARGIN)),
                    CHUNK_TAB_POS_Y + (chunk_tab_coordinates.y * (RECT_SIZE + MARGIN))
                    },
                    font_size_small, 1, BLACK);
        else
            DrawTextEx(font, TextFormat(" %d", chunk_tab[i] - chunk_buf),
                    (Vector2){
                    CHUNK_TAB_POS_X + (chunk_tab_coordinates.x * (RECT_SIZE + MARGIN)),
                    CHUNK_TAB_POS_Y + (chunk_tab_coordinates.y * (RECT_SIZE + MARGIN))
                    },
                    font_size_small, 1, BLACK);
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
    flag = FLAG_ACTIVE;
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1400, 920, "Chunk Pointer Table");
    SetTargetFPS(30);
    SetExitKey(KEY_Q);

    font = LoadFont("fonts/code_saver_regular.otf");
    init_chunking();

    while ((flag & FLAG_ACTIVE) && (!WindowShouldClose()))
    {
        // ---- main_loop ------------------------------------------------------
        while (player_chunk.x - player_delta_chunk.x || player_chunk.y - player_delta_chunk.y)
            shift_chunk_tab(player_chunk, &player_delta_chunk);

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
    if (IsKeyPressed(KEY_ENTER))
        update_chunk_tab(player_chunk);

    if ((IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) && render_distance < CHUNK_BUF_RADIUS)
        ++render_distance;

    if ((IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) && render_distance > 1)
        --render_distance;

    if (IsKeyPressed(KEY_TAB))
        data_view = !data_view;

    if (IsKeyDown(KEY_D))
    {
        shift_direction = 1;
        ++player_chunk.x;
    }

    if (IsKeyDown(KEY_A))
    {
        shift_direction = 2;
        --player_chunk.x;
    }

    if (IsKeyDown(KEY_S))
    {
        shift_direction = 3;
        ++player_chunk.y;
    }

    if (IsKeyDown(KEY_W))
    {
        shift_direction = 4;
        --player_chunk.y;
    }
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
    DrawTextEx(font, TextFormat("%s\n%s\n%s\n%s\n%s\n%s",
                "ENTER:  load chunks within render distance",
                "UP:     increase render distance",
                "DOWN:   decrease render distance",
                "TAB:    change data view: index/chunk pos",
                "WASD:   mark edge chunks",
                "SHIFT + WASD: mark chunks & shift chunk tab"),
            (Vector2){10.0f, 10.0f},
            font_size, 1, MC_C_BLUE);

    DrawTextEx(font, TextFormat("Render Distance: %d   Shift Direction: %d",
                render_distance, shift_direction),
            (Vector2){LEGEND_POS_X, 180.0f},
            font_size, 1, MC_C_GREEN);

    // ---- legend -------------------------------------------------------------
    DrawRectangleRounded((Rectangle){LEGEND_POS_X, LEGEND_POS_Y, RECT_SIZE / 2, RECT_SIZE / 2},
            0.3f, 1, MC_C_OFF);

    DrawRectangleRounded((Rectangle){LEGEND_POS_X, LEGEND_POS_Y + ((RECT_SIZE / 2) * 2), (RECT_SIZE / 2), (RECT_SIZE / 2)},
            0.3f, 1, MC_C_GREEN);

    DrawRectangleRounded((Rectangle){LEGEND_POS_X, LEGEND_POS_Y + ((RECT_SIZE / 2) * 4), (RECT_SIZE / 2), (RECT_SIZE / 2)},
            0.3f, 1, MC_C_RED);

    DrawRectangleRounded((Rectangle){LEGEND_POS_X, LEGEND_POS_Y + ((RECT_SIZE / 2) * 6), (RECT_SIZE / 2), (RECT_SIZE / 2)},
            0.3f, 1, MC_C_BLUE);

    DrawTextEx(font, "Usable/Not Null",
            (Vector2){
            LEGEND_POS_X + (RECT_SIZE + MARGIN),
            LEGEND_POS_Y},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Chunk Loaded",
            (Vector2){
            LEGEND_POS_X + (((RECT_SIZE / 2) + MARGIN) * 2),
            LEGEND_POS_Y + ((RECT_SIZE / 2) * 2)},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Render Surface",
            (Vector2){
            LEGEND_POS_X + (((RECT_SIZE / 2) + MARGIN) * 2),
            LEGEND_POS_Y + ((RECT_SIZE / 2) * 4)},
            font_size, 1, MC_C_GRAY);

    DrawTextEx(font, "Chunk Loaded, but Out of Render Surface",
            (Vector2){
            LEGEND_POS_X + (((RECT_SIZE / 2) + MARGIN) * 2),
            LEGEND_POS_Y + ((RECT_SIZE / 2) * 6)},
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

    for (u32 i = 0; i < CHUNK_BUF_DIAMETER; ++i)
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

