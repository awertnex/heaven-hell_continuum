#include <math.h>

#include "h/chunking.h"
#include "h/logic.h"

#include "engine/h/memory.h"
#include "engine/h/logger.h"

Chunk *chunk_buf = {0};                         // chunk buffer, raw chunk data
Chunk *chunk_tab[CHUNK_BUF_ELEMENTS] = {NULL};  // chunk pointer look-up table
v2u16 chunk_tab_coordinates;                    // pointer arithmetic redundancy optimization
v3u32 block_coordinates;                        // pointer arithmetic redundancy optimization
u16 chunk_tab_index = 0;                        // player relative chunk tab access
struct Globals globals =
{
    .world_height = WORLD_HEIGHT_NORMAL,
    .opacity = 0,
    .block_count = 0,
    .quad_count = 0,
};

u8 init_chunking(void)
{
    MC_C_ALLOC(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
    return 0;

cleanup:
    free_chunking();
    return -1;
}

void free_chunking(void)
{
    MC_C_FREE(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
}

// index = (chunk_tab index);
void add_block(u16 index, u8 x, u8 y, u16 z)
{
    chunk_tab_coordinates = (v2u16){index % CHUNK_BUF_DIAMETER, index / CHUNK_BUF_DIAMETER};
    u16 mirror_index = {0};
    u8 is_on_edge = 0;

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge =
            (chunk_tab_coordinates.x == CHUNK_BUF_DIAMETER - 1)
            || (chunk_tab[index + 1] == NULL);
        if (is_on_edge)
            chunk_tab[index]->block[z][y][x] |= POSITIVE_X;
        else
        {
            mirror_index = get_mirror_axis(x);
            (chunk_tab[index + 1]->block[z][y][mirror_index])
                ? (chunk_tab[index + 1]->block[z][y][mirror_index] &= ~NEGATIVE_X)
                : (chunk_tab[index]->block[z][y][x] |= POSITIVE_X);
        }
    }
    else chunk_tab[index]->block[z][y][x + 1]
        ? (chunk_tab[index]->block[z][y][x + 1] &= ~NEGATIVE_X)
            : (chunk_tab[index]->block[z][y][x] |= POSITIVE_X);

    if (x == 0)
    {
        is_on_edge =
            (chunk_tab_coordinates.x == 0) || (chunk_tab[index - 1] == NULL);
        if (is_on_edge)
            chunk_tab[index]->block[z][y][x] |= NEGATIVE_X;
        else
        {
            mirror_index = get_mirror_axis(x);
            (chunk_tab[index - 1]->block[z][y][mirror_index])
                ? (chunk_tab[index - 1]->block[z][y][mirror_index] &= ~POSITIVE_X)
                : (chunk_tab[index]->block[z][y][x] |= NEGATIVE_X);
        }
    }
    else chunk_tab[index]->block[z][y][x - 1]
        ? (chunk_tab[index]->block[z][y][x - 1] &= ~POSITIVE_X)
            : (chunk_tab[index]->block[z][y][x] |= NEGATIVE_X);

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge =
            (chunk_tab_coordinates.y == CHUNK_BUF_DIAMETER - 1)
            || (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);
        if (is_on_edge)
            chunk_tab[index]->block[z][y][x] |= POSITIVE_Y;
        else
        {
            mirror_index = get_mirror_axis(y);
            (chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][mirror_index][x])
                ? (chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][mirror_index][x] &= ~NEGATIVE_Y)
                : (chunk_tab[index]->block[z][y][x] |= POSITIVE_Y);
        }
    }
    else chunk_tab[index]->block[z][y + 1][x]
        ? (chunk_tab[index]->block[z][y + 1][x] &= ~NEGATIVE_Y)
            : (chunk_tab[index]->block[z][y][x] |= POSITIVE_Y);

    if (y == 0)
    {
        is_on_edge =
            (chunk_tab_coordinates.y == 0)
            || (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);
        if (is_on_edge)
            chunk_tab[index]->block[z][y][x] |= NEGATIVE_Y;
        else
        {
            mirror_index = get_mirror_axis(y);
            (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][mirror_index][x])
                ? (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][mirror_index][x] &= ~POSITIVE_Y)
                : (chunk_tab[index]->block[z][y][x] |= NEGATIVE_Y);
        }
    }
    else chunk_tab[index]->block[z][y - 1][x]
        ? (chunk_tab[index]->block[z][y - 1][x] &= ~POSITIVE_Y)
            : (chunk_tab[index]->block[z][y][x] |= NEGATIVE_Y);

    if (z == globals.world_height - WORLD_BOTTOM - 1)
    {
        chunk_tab[index]->block[z][y][x] |= POSITIVE_Z;
    }
    else chunk_tab[index]->block[z + 1][y][x]
        ? (chunk_tab[index]->block[z + 1][y][x] &= ~NEGATIVE_Z)
            : (chunk_tab[index]->block[z][y][x] |= POSITIVE_Z);
    
    if (z == 0)
    {
        chunk_tab[index]->block[z][y][x] |= NEGATIVE_Z;
    }
    else chunk_tab[index]->block[z - 1][y][x]
        ? (chunk_tab[index]->block[z - 1][y][x] &= ~POSITIVE_Z)
            : (chunk_tab[index]->block[z][y][x] |= NEGATIVE_Z);

    chunk_tab[index]->block[z][y][x] |= NOT_EMPTY;

    if (get_block_index(x, y, z) >= chunk_tab[index]->block_parse_limit)
        chunk_tab[index]->block_parse_limit = get_block_index(x, y, z);
}

// index = (chunk_tab index);
void remove_block(u16 index, u8 x, u8 y, u16 z)
{
    chunk_tab_coordinates = (v2u16){index % CHUNK_BUF_DIAMETER, index / CHUNK_BUF_DIAMETER};
    u16 mirror_index = {0};
    u8 is_on_edge = 0;

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge =
            (chunk_tab_coordinates.x == CHUNK_BUF_DIAMETER - 1)
            || (chunk_tab[index + 1] == NULL);
        if (!is_on_edge)
        {
            mirror_index = get_mirror_axis(x);
            (chunk_tab[index + 1]->block[z][y][mirror_index])
                ? (chunk_tab[index + 1]->block[z][y][mirror_index] |= NEGATIVE_X)
                : 0;
        }
    }
    else chunk_tab[index]->block[z][y][x + 1]
        ? (chunk_tab[index]->block[z][y][x + 1] |= NEGATIVE_X) : 0;

    if (x == 0)
    {
        is_on_edge =
            (chunk_tab_coordinates.x == 0) || (chunk_tab[index - 1] == NULL);
        if (!is_on_edge)
        {
            mirror_index = get_mirror_axis(x);
            (chunk_tab[index - 1]->block[z][y][mirror_index])
                ? (chunk_tab[index - 1]->block[z][y][mirror_index] |= POSITIVE_X) : 0;
        }
    }
    else chunk_tab[index]->block[z][y][x - 1]
        ? (chunk_tab[index]->block[z][y][x - 1] |= POSITIVE_X) : 0;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge =
            (chunk_tab_coordinates.y == CHUNK_BUF_DIAMETER - 1)
            || (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);
        if (!is_on_edge)
        {
            mirror_index = get_mirror_axis(y);
            (chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][mirror_index][x])
                ? (chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][mirror_index][x] |= NEGATIVE_Y)
                : 0;
        }
    }
    else chunk_tab[index]->block[z][y + 1][x]
        ? (chunk_tab[index]->block[z][y + 1][x] |= NEGATIVE_Y) : 0;

    if (y == 0)
    {
        is_on_edge =
            (chunk_tab_coordinates.y == 0)
            || (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);
        if (!is_on_edge)
        {
            mirror_index = get_mirror_axis(y);
            (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][mirror_index][x])
                ? (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][mirror_index][x] |= POSITIVE_Y)
                : 0;
        }
    }
    else chunk_tab[index]->block[z][y - 1][x]
        ? (chunk_tab[index]->block[z][y - 1][x] |= POSITIVE_Y) : 0;

    if (z < globals.world_height - WORLD_BOTTOM - 1)
        chunk_tab[index]->block[z + 1][y][x]
            ? (chunk_tab[index]->block[z + 1][y][x] |= NEGATIVE_Z) : 0;
    
    if (z > 0)
        chunk_tab[index]->block[z - 1][y][x]
            ? (chunk_tab[index]->block[z - 1][y][x] |= POSITIVE_Z) : 0;

    chunk_tab[index]->block[z][y][x] = 0;

    if (get_block_index(x, y, z) == chunk_tab[index]->block_parse_limit)
    {
        for (u32 i = chunk_tab[index]->block_parse_limit;
                i < CHUNK_MAX_BLOCKS && i >= 0; --i)
        {
            if (i == 0)
            {
                chunk_tab[index]->block_parse_limit = 0;
                break;
            }

            block_coordinates = get_block_coordinates(i);
            if (chunk_tab[index]->block[block_coordinates.z][block_coordinates.y][block_coordinates.x])
            {
                chunk_tab[index]->block_parse_limit = i;
                break;
            }
        }
    }
}

// index = (chunk_tab index);
void generate_chunk(u16 index) // TODO: make this function
{
    u16 sin_x = 0, sin_y = 0;

    for (u16 z = 0; z < 20; ++z)
        for (u8 y = 0; y < CHUNK_DIAMETER; ++y)
            for (u8 x = 0; x < CHUNK_DIAMETER; ++x)
            {
                sin_x = (u16)((sin(((f32)x + (chunk_tab[index]->pos.x * CHUNK_DIAMETER))
                                / 15) + 1) * 10) + 2;
                sin_y = (u16)((sin(((f32)y + (chunk_tab[index]->pos.y * CHUNK_DIAMETER))
                                / 15) + 1) * 10) + 2;

                if (z <= sin_x && z <= sin_y)
                    add_block(index, x, y, z);
            }
}

// TODO: add 'version' byte for serialization
void serialize_chunk(Chunk *chunk, str *world_name) // TODO: make this function
{
}

void deserialize_chunk(Chunk *chunk, str *world_name) // TODO: make this function
{
}

// pos = (chunk_tab coordinates);
Chunk *push_chunk_buf(v2i16 player_delta_chunk, v2u16 pos)
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_buf[i].flag & FLAG_CHUNK_LOADED) continue;

        memset(&chunk_buf[i], 0, sizeof(Chunk));
        chunk_buf[i] =
            (Chunk){
                .pos = (v2i16){
                    player_delta_chunk.x + (pos.x - CHUNK_BUF_RADIUS),
                    player_delta_chunk.y + (pos.y - CHUNK_BUF_RADIUS)},
                .flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_RENDER,
            };
        chunk_buf[i].id = ((chunk_buf[i].pos.x & 0xffff) << 16) + (chunk_buf[i].pos.y & 0xffff);
        return &chunk_buf[i];
    }
    return NULL;
}

// index = (chunk_tab index);
Chunk *pop_chunk_buf(u16 index)
{
    *chunk_tab[index] = (Chunk){0};
    return NULL;
}

void update_chunk_tab(v2i16 player_delta_chunk)
{
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        chunk_tab_coordinates = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};
        if (get_distance(
                    (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                    (v2i32){chunk_tab_coordinates.x, chunk_tab_coordinates.y})
                < ((u32)powf(setting.render_distance, 2) + 2))
        {
            if (chunk_tab[i] == NULL)
                chunk_tab[i] = push_chunk_buf(player_delta_chunk, chunk_tab_coordinates);

            if (chunk_tab[i] != NULL)
                generate_chunk(i); // TODO: grab chunks from disk if previously generated
        }
        else if ((chunk_tab[i] != NULL) && (chunk_tab[i]->flag & FLAG_CHUNK_LOADED))
                chunk_tab[i] = pop_chunk_buf(i);
    }
}

void shift_chunk_tab(v2i16 player_chunk, v2i16 *player_delta_chunk)
{
    v2i16 delta = {
        player_chunk.x - player_delta_chunk->x,
        player_chunk.y - player_delta_chunk->y};

    if (get_distance(
            (v2i32){player_chunk.x, player_chunk.y},
            (v2i32){player_delta_chunk->x, player_delta_chunk->y})
            > (u32)powf(SETTING_RENDER_DISTANCE_DEFAULT, 2) + 2)
    {
        for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
            if (chunk_tab[i] != NULL)
                chunk_tab[i] = pop_chunk_buf(i);

        *player_delta_chunk = player_chunk;
        return;
    }

    // direction = (1 = x, 2 = -x, 3 = y, 4 = -y, 5 = z, 6 = -z);
    u8 direction =
        (delta.x > 0) ? 1 : (delta.x < 0) ? 2 :
        (delta.y > 0) ? 3 : (delta.y < 0) ? 4 : 0;
    i8 increment = (direction == 1 || direction == 3) - (direction == 2 || direction == 4);
    v2u16 coordinates = {0};
    u16 mirror_index = 0;
    u16 target_index = 0;
    u8 is_on_edge = 0;

    player_delta_chunk->x += ((direction == 1) - (direction == 2));
    player_delta_chunk->y += ((direction == 3) - (direction == 4));

    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
        if (chunk_tab[i] != NULL)
            chunk_tab[i]->flag &= ~FLAG_CHUNK_EDGE;

    // ---- mark chunks on-edge ------------------------------------------------
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_tab[i] == NULL) continue;
        coordinates = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};
        switch (direction)
        {
            case 1: // ---- positive x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2);
                is_on_edge =
                    (coordinates.x == 0) || (chunk_tab[i - 1] == NULL);
                break;

            case 2: // ---- negative x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2);
                is_on_edge =
                    (coordinates.x == CHUNK_BUF_DIAMETER - 1) || (chunk_tab[i + 1] == NULL);
                break;

            case 3: // ---- positive y -----------------------------------------
                mirror_index =
                    ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) * CHUNK_BUF_DIAMETER) + coordinates.x;
                is_on_edge =
                    (coordinates.y == 0) || (chunk_tab[i - CHUNK_BUF_DIAMETER] == NULL);
                break;

            case 4: // ---- negative y -----------------------------------------
                mirror_index =
                    ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) * CHUNK_BUF_DIAMETER) + coordinates.x;
                is_on_edge =
                    (coordinates.y == CHUNK_BUF_DIAMETER - 1)
                    || (chunk_tab[i + CHUNK_BUF_DIAMETER] == NULL);
                break;
        }

        if (is_on_edge)
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_RENDER;
            chunk_tab[i]->flag &= ~FLAG_CHUNK_LOADED;
            if (chunk_tab[mirror_index] != NULL)
                chunk_tab[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    // ---- shift chunk_tab ----------------------------------------------------
    for (u16 i = (increment == 1) ? 0 : CHUNK_BUF_ELEMENTS - 1;
            i >= 0 && i < CHUNK_BUF_ELEMENTS;
            i += increment)
    {
        if (chunk_tab[i] == NULL) continue;
        coordinates = (v2u16){i % CHUNK_BUF_DIAMETER, i / CHUNK_BUF_DIAMETER};
        switch (direction)
        {
            case 1: // ---- positive x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2);
                target_index = (coordinates.x == CHUNK_BUF_DIAMETER - 1) ? i : i + 1;
                break;

            case 2: // ---- negative x -----------------------------------------
                mirror_index = i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2);
                target_index = (coordinates.x == 0) ? i : i - 1;
                break;

            case 3: // ---- positive y -----------------------------------------
                mirror_index = 
                    ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) * CHUNK_BUF_DIAMETER) + coordinates.x;
                target_index = (coordinates.y == CHUNK_BUF_DIAMETER - 1) ? i : i + CHUNK_BUF_DIAMETER;
                break;

            case 4: // ---- negative y -----------------------------------------
                mirror_index =
                    ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) * CHUNK_BUF_DIAMETER) + coordinates.x;
                target_index = (coordinates.y == 0) ? i : i - CHUNK_BUF_DIAMETER;
                break;
        }

        chunk_tab[i] = chunk_tab[target_index];
        if (chunk_tab[i]->flag & FLAG_CHUNK_EDGE)
            chunk_tab[target_index] = NULL;
    }
}

u16 get_target_chunk_index(v2i16 player_chunk, v3i32 player_delta_target)
{
    v2i16 offset =
    {
        (i16)floorf((f32)player_delta_target.x / CHUNK_DIAMETER) - player_chunk.x + CHUNK_BUF_RADIUS,
        (i16)floorf((f32)player_delta_target.y / CHUNK_DIAMETER) - player_chunk.y + CHUNK_BUF_RADIUS,
    };
    return offset.x + (offset.y * CHUNK_BUF_DIAMETER);
}

void draw_chunk_tab(Texture *tex /* temp texturing */)
{
    if (state & FLAG_DEBUG_MORE)
        globals.opacity = 200;
    else
        globals.opacity = 255;

    rlPushMatrix();
    rlSetTexture(tex->id); //temp texturing
    rlBegin(RL_QUADS);

    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if ((chunk_tab[i] == NULL) || !(chunk_tab[i]->flag & FLAG_CHUNK_RENDER))
            continue;

        rlTranslatef(
                (f32)(chunk_tab[i]->pos.x * CHUNK_DIAMETER),
                (f32)(chunk_tab[i]->pos.y * CHUNK_DIAMETER),
                (f32)WORLD_BOTTOM);

        for (u32 j = 0; j <= chunk_tab[i]->block_parse_limit; ++j)
        {
            block_coordinates = get_block_coordinates(j);
            draw_block(chunk_tab[i],
                    block_coordinates.x,
                    block_coordinates.y,
                    block_coordinates.z);

            rlTranslatef(1.0f, 0.0f, 0.0f);
            if (block_coordinates.x == CHUNK_DIAMETER - 1)
            {
                rlTranslatef(-(f32)CHUNK_DIAMETER, 1.0f, 0.0f);
                if (block_coordinates.y == CHUNK_DIAMETER - 1)
                    rlTranslatef(0.0f, -(f32)CHUNK_DIAMETER, 1.0f);
            }
        }

        block_coordinates = get_block_coordinates(chunk_tab[i]->block_parse_limit + 1);
        rlTranslatef(
                -(f32)block_coordinates.x - (f32)(chunk_tab[i]->pos.x * CHUNK_DIAMETER),
                -(f32)block_coordinates.y - (f32)(chunk_tab[i]->pos.y * CHUNK_DIAMETER),
                -(f32)block_coordinates.z - (f32)WORLD_BOTTOM);
    }

    rlEnd();
    rlPopMatrix();
    rlSetTexture(0); //temp texturing
}

// raylib/rmodels.c/DrawCube refactored;
void draw_block(Chunk *chunk, u8 x, u8 y, u16 z)
{
    if (chunk->block[z][y][x] & POSITIVE_X)
    {
        if (LOGGER_DEBUG)
            rlColor4ub(150, 150, 137, globals.opacity);
        else rlColor4ub(200, 210, 90, globals.opacity);
        
        rlNormal3f(1.0f, 0.0f, 0.0f); //temp texturing
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
    }

    if (chunk->block[z][y][x] & NEGATIVE_X)
    {
        if (LOGGER_DEBUG)
            rlColor4ub(135, 135, 123, globals.opacity);
        else rlColor4ub(236, 17, 90, globals.opacity);

        rlNormal3f(-1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
    }

    if (chunk->block[z][y][x] & POSITIVE_Y)
    {
        if (LOGGER_DEBUG)
            rlColor4ub(155, 155, 142, globals.opacity);
        else rlColor4ub(200, 248, 246, globals.opacity);

        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
    }

    if (chunk->block[z][y][x] & NEGATIVE_Y)
    {
        if (LOGGER_DEBUG)
            rlColor4ub(140, 140, 123, globals.opacity);
        else rlColor4ub(28, 14, 50, globals.opacity);

        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
    }

    if (chunk->block[z][y][x] & POSITIVE_Z)
    {
        if (LOGGER_DEBUG)
            rlColor4ub(176, 176, 160, globals.opacity);
        else rlColor4ub(250, 18, 5, globals.opacity);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
    }

    if (chunk->block[z][y][x] & NEGATIVE_Z)
    {
        if (LOGGER_DEBUG)
            rlColor4ub(115, 115, 104, globals.opacity);
        else rlColor4ub(200, 40, 203, globals.opacity);

        rlNormal3f(0.0f, 0.0f, -1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
    }
}

// raylib/rmodels.c/DrawLine3D refactored;
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color)
{
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
    rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}

// raylib/rmodels.c/DrawCubeWires refactored;
void draw_block_wires(v3i32 pos)
{
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlBegin(RL_LINES);
    rlColor4ub(0, 0, 0, 150);

    rlVertex3f(0.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 0.0f, 1.0f);

    rlVertex3f(1.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 1.0f, 1.0f);

    rlVertex3f(1.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 1.0f, 1.0f);

    rlVertex3f(0.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 1.0f);

    rlVertex3f(0.0f, 0.0f, 0.0f);
    rlVertex3f(1.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 0.0f, 0.0f);
    rlVertex3f(1.0f, 1.0f, 0.0f);

    rlVertex3f(1.0f, 1.0f, 0.0f);
    rlVertex3f(0.0f, 1.0f, 0.0f);

    rlVertex3f(0.0f, 1.0f, 0.0f);
    rlVertex3f(0.0f, 0.0f, 0.0f);

    rlVertex3f(0.0f, 0.0f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 1.0f, 1.0f);
    rlVertex3f(1.0f, 1.0f, 0.0f);

    rlVertex3f(0.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 1.0f, 0.0f);

    rlEnd();
    rlPopMatrix();
}

// raylib/rmodels.c/DrawCubeWires refactored;
void draw_bounding_box(Vector3 origin, Vector3 scl, Color col)
{
    rlPushMatrix();
    rlTranslatef(
            origin.x - (scl.x / 2),
            origin.y - (scl.y / 2),
            origin.z);
    rlBegin(RL_LINES);
    rlColor4ub(col.r, col.g, col.b, col.a);

    rlVertex3f(0.0f,    0.0f,   scl.z);
    rlVertex3f(scl.x,   0.0f,   scl.z);

    rlVertex3f(scl.x,   0.0f,   scl.z);
    rlVertex3f(scl.x,   scl.y,  scl.z);

    rlVertex3f(scl.x,   scl.y,  scl.z);
    rlVertex3f(0.0f,    scl.y,  scl.z);

    rlVertex3f(0.0f,    scl.y,  scl.z);
    rlVertex3f(0.0f,    0.0f,   scl.z);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(scl.x,   0.0f,   0.0f);

    rlVertex3f(scl.x,   0.0f,   0.0f);
    rlVertex3f(scl.x,   scl.y,  0.0f);

    rlVertex3f(scl.x,   scl.y,  0.0f);
    rlVertex3f(0.0f,    scl.y,  0.0f);

    rlVertex3f(0.0f,    scl.y,  0.0f);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(0.0f,    0.0f,   scl.z);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(scl.x,   0.0f,   scl.z);
    rlVertex3f(scl.x,   0.0f,   0.0f);

    rlVertex3f(scl.x,   scl.y,  scl.z);
    rlVertex3f(scl.x,   scl.y,  0.0f);

    rlVertex3f(0.0f,    scl.y,  scl.z);
    rlVertex3f(0.0f,    scl.y,  0.0f);

    rlEnd();
    rlPopMatrix();
}

void draw_bounding_box_clamped(Vector3 origin, Vector3 scl, Color col)
{
    Vector3 start = (Vector3){
            floorf(origin.x - 2.0f),
            floorf(origin.y - 2.0f),
            floorf(origin.z - 1.0f),
        };

    Vector3 end = (Vector3){
            ceilf(scl.x + 4.0f),
            ceilf(scl.y + 4.0f),
            ceilf(scl.z + 3.0f),
        };

    rlPushMatrix();
    rlTranslatef(start.x, start.y, start.z);
    rlBegin(RL_LINES);
    rlColor4ub(col.r, col.g, col.b, col.a);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(0.0f,    0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  end.z);
    rlVertex3f(0.0f,    0.0f,   end.z);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(0.0f,    0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  end.z);

    rlEnd();
    rlPopMatrix();
}

