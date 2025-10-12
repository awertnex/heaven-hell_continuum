#include <stdio.h>
#include <stdlib.h>

#include <engine/h/memory.h>
#include <engine/h/math.h>
#include <engine/h/logger.h>

#include "h/main.h"
#include "h/chunking.h"

/* chunk buffer, raw chunk data */
Chunk *chunk_buf = {0};

/* chunk pointer look-up table */
Chunk *chunk_tab[CHUNK_BUF_VOLUME] = {0};

/* chunk pointer pointer look-up table that points to chunk_tab addresses.
 * order of chunks based on distance away from player */
Chunk **chunk_order[CHUNK_BUF_VOLUME] = {0};

/* queue of chunks to be processed */
ChunkQueue chunk_queue = {0};

/* index = (chunk_tab index);
 * rate = number of blocks to generate per frame per chunk */
static void chunk_generate(u32 index, u32 rate);

static f32 terrain_noise(v3i32 coordinates, f32 amplitude, f32 frequency);
static void mesh_chunk(Chunk *chunk);

/* -- INTERNAL USE ONLY --; */
static void _mesh_chunk(Chunk *chunk);

static void serialize_chunk(Chunk *chunk, str *world_name);
static void deserialize_chunk(Chunk *chunk, str *world_name);

/* index = (chunk_tab index); */
static void chunk_buf_push(u32 index, v3i16 player_delta_chunk, u32 distance);

/* index = (chunk_tab index); */
static void chunk_buf_pop(u32 index);

/* rate_chunk = number of chunks to process per frame
 * rate_block = number of blocks to process per frame per chunk */
void chunk_queue_update(u32 rate_chunk, u32 rate_block);

static v3f32 table_rand[1024] = {0};
u8
init_chunking(void)
{
    if (!mem_alloc_memb((void*)&chunk_buf,
                CHUNK_BUF_VOLUME, sizeof(Chunk), "chunk_buf"))
        return -1;

    v3i32 center = {CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS};
    v3i32 pos = {0};
    f32 distance[CHUNK_BUF_VOLUME] = {0};
    u64 i, j;
    for (i = 0; i < 1024; ++i)
        table_rand[i].x = (f32)(rand() % 36000) / 100.0f;
    for (i = 0; i < 1024; ++i)
        table_rand[i].y = (f32)(rand() % 36000) / 100.0f;
    for (i = 0; i < 1024; ++i)
        table_rand[i].z = (f32)(rand() % 36000) / 100.0f;

    for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        pos =
            (v3i32){
                i % CHUNK_BUF_DIAMETER,
                (i / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
                i / CHUNK_BUF_LAYER,
            };
        distance[i] = distance_v3i32(pos, center);
    }
    for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
        chunk_order[i] = &chunk_tab[i];
    for (i = CHUNK_BUF_VOLUME - 1; i < CHUNK_BUF_VOLUME; --i)
        for (j = CHUNK_BUF_VOLUME - 1; j < CHUNK_BUF_VOLUME; --j)
            if (distance[j] < distance[i])
            {
                swap_bits_u32((u32*)&distance[i], (u32*)&distance[j]);
                swap_bits_u64((u64*)&chunk_order[i], (u64*)&chunk_order[j]);
            }
    return 0;
}

void
update_chunking(v3i16 player_delta_chunk)
{
    const u32 RENDER_DISTANCE = (u32)powf(settings.render_distance, 2.0f) + 2;

    u32 i = 0;
    for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        Chunk *p = chunk_tab[i];
        v3u32 coordinates =
        {
            i % CHUNK_BUF_DIAMETER,
            (i / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
            i / CHUNK_BUF_LAYER,
        };
        u32 distance = distance_v3i32(
                (v3i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                (v3i32){coordinates.x, coordinates.y, coordinates.z});

        if (distance < RENDER_DISTANCE)
        {
            if (!p) chunk_buf_push(i, player_delta_chunk, distance);
        }
        else if (p && p->flag & FLAG_CHUNK_LOADED)
            chunk_buf_pop(i);
    }
}

void
free_chunking(void)
{
    u32 i = 0;
    for (; i < CHUNK_BUF_VOLUME; ++i)
        if (chunk_tab[i])
            chunk_buf_pop(i);
    mem_free((void*)&chunk_buf, CHUNK_BUF_VOLUME * sizeof(Chunk), "chunk_buf");
}

#if 1
void
add_block(u32 index, u32 x, u32 y, u32 z)
{
    Chunk *chunk = chunk_tab[index];
    u8 is_on_edge = 0;
    v3u32 coordinates =
    {
        index % CHUNK_BUF_DIAMETER,
        (index / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
        index / CHUNK_BUF_LAYER,
    };

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + 1] == NULL);

        if (!is_on_edge && chunk_tab[index + 1]->block[z][y][0])
        {
            chunk_tab[index + 1]->block[z][y][0] &= ~FLAG_BLOCK_FACE_NX;
            chunk_tab[index + 1]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PX;
    }
    else if (chunk->block[z][y][x + 1])
        chunk->block[z][y][x + 1] &= ~FLAG_BLOCK_FACE_NX;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || (chunk_tab[index - 1] == NULL);

        if (!is_on_edge && chunk_tab[index - 1]->block[z][y][CHUNK_DIAMETER - 1])
        {
            chunk_tab[index - 1]->block[z][y][CHUNK_DIAMETER - 1] &= ~FLAG_BLOCK_FACE_PX;
            chunk_tab[index - 1]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NX;
    }
    else if (chunk->block[z][y][x - 1])
        chunk->block[z][y][x - 1] &= ~FLAG_BLOCK_FACE_PX;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][0][x])
        {
            chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][0][x] &= ~FLAG_BLOCK_FACE_NY;
            chunk_tab[index + CHUNK_BUF_DIAMETER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PY;
    }
    else if (chunk->block[z][y + 1][x])
        chunk->block[z][y + 1][x] &= ~FLAG_BLOCK_FACE_NY;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) ||
            (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][CHUNK_DIAMETER - 1][x]))
        {
            chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][CHUNK_DIAMETER - 1][x] &= ~FLAG_BLOCK_FACE_PY;
            chunk_tab[index - CHUNK_BUF_DIAMETER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NY;
    }
    else if (chunk->block[z][y - 1][x])
        chunk->block[z][y - 1][x] &= ~FLAG_BLOCK_FACE_PY;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_LAYER]->block[0][y][x])
        {
            chunk_tab[index + CHUNK_BUF_LAYER]->block[0][y][x] &= ~FLAG_BLOCK_FACE_NZ;
            chunk_tab[index + CHUNK_BUF_LAYER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    }
    else if (chunk->block[z + 1][y][x])
        chunk->block[z + 1][y][x] &= ~FLAG_BLOCK_FACE_NZ;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    
    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) ||
            (chunk_tab[index - CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_LAYER]->block[CHUNK_DIAMETER - 1][y][x])
        {
            chunk_tab[index - CHUNK_BUF_LAYER]->block[CHUNK_DIAMETER - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
            chunk_tab[index - CHUNK_BUF_LAYER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;
    }
    else if (chunk->block[z - 1][y][x])
        chunk->block[z - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;

    chunk->block[z][y][x] |= FLAG_BLOCK_NOT_EMPTY;
    chunk->flag |= FLAG_CHUNK_DIRTY;
}
#else
void
add_block(u32 index, u32 x, u32 y, u32 z)
{
    Chunk *chunk = chunk_tab[index];
    u8 is_on_edge = 0;
    v3u32 coordinates =
    {
        index % CHUNK_BUF_DIAMETER,
        (index / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
        index / CHUNK_BUF_LAYER,
    };

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + 1] == NULL);

        if (!is_on_edge && chunk_tab[index + 1]->block[z][y][0])
        {
            chunk_tab[index + 1]->block[z][y][0] &= ~FLAG_BLOCK_FACE_NX;
            chunk_tab[index + 1]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PX;
    }
    else if (chunk->block[z][y][x + 1])
        chunk->block[z][y][x + 1] &= ~FLAG_BLOCK_FACE_NX;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || (chunk_tab[index - 1] == NULL);

        if (!is_on_edge && chunk_tab[index - 1]->block[z][y][CHUNK_DIAMETER - 1])
        {
            chunk_tab[index - 1]->block[z][y][CHUNK_DIAMETER - 1] &= ~FLAG_BLOCK_FACE_PX;
            chunk_tab[index - 1]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NX;
    }
    else if (chunk->block[z][y][x - 1])
        chunk->block[z][y][x - 1] &= ~FLAG_BLOCK_FACE_PX;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][0][x])
        {
            chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][0][x] &= ~FLAG_BLOCK_FACE_NY;
            chunk_tab[index + CHUNK_BUF_DIAMETER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PY;
    }
    else if (chunk->block[z][y + 1][x])
        chunk->block[z][y + 1][x] &= ~FLAG_BLOCK_FACE_NY;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) ||
            (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][CHUNK_DIAMETER - 1][x]))
        {
            chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][CHUNK_DIAMETER - 1][x] &= ~FLAG_BLOCK_FACE_PY;
            chunk_tab[index - CHUNK_BUF_DIAMETER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NY;
    }
    else if (chunk->block[z][y - 1][x])
        chunk->block[z][y - 1][x] &= ~FLAG_BLOCK_FACE_PY;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_LAYER]->block[0][y][x])
        {
            chunk_tab[index + CHUNK_BUF_LAYER]->block[0][y][x] &= ~FLAG_BLOCK_FACE_NZ;
            chunk_tab[index + CHUNK_BUF_LAYER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    }
    else if (chunk->block[z + 1][y][x])
        chunk->block[z + 1][y][x] &= ~FLAG_BLOCK_FACE_NZ;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    
    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) ||
            (chunk_tab[index - CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_LAYER]->block[CHUNK_DIAMETER - 1][y][x])
        {
            chunk_tab[index - CHUNK_BUF_LAYER]->block[CHUNK_DIAMETER - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
            chunk_tab[index - CHUNK_BUF_LAYER]->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;
    }
    else if (chunk->block[z - 1][y][x])
        chunk->block[z - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;

    chunk->block[z][y][x] |= FLAG_BLOCK_NOT_EMPTY;
    chunk->block[z][y][x] |=
        (((u64)x & 0xf) << 32) |
        (((u64)y & 0xf) << 36) |
        (((u64)z & 0xf) << 40);
    chunk->flag |= FLAG_CHUNK_DIRTY;
}
#endif /* add_block() */

void
remove_block(u32 index, u32 x, u32 y, u32 z)
{
    Chunk *chunk = chunk_tab[index];
    u8 is_on_edge = 0;
    v3u32 coordinates =
    {
        index % CHUNK_BUF_DIAMETER,
        (index / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
        index / CHUNK_BUF_LAYER,
    };

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + 1] == NULL);

        if (!is_on_edge && chunk_tab[index + 1]->block[z][y][0])
            chunk_tab[index + 1]->block[z][y][0] |= FLAG_BLOCK_FACE_NX;
    }
    else if (chunk->block[z][y][x + 1])
        chunk->block[z][y][x + 1] |= FLAG_BLOCK_FACE_NX;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || (chunk_tab[index - 1] == NULL);

        if (!is_on_edge && chunk_tab[index - 1]->block[z][y][CHUNK_DIAMETER - 1])
            chunk_tab[index - 1]->block[z][y][CHUNK_DIAMETER - 1] |= FLAG_BLOCK_FACE_PX;
    }
    else if (chunk->block[z][y][x - 1])
        chunk->block[z][y][x - 1] |= FLAG_BLOCK_FACE_PX;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][0][x])
            chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][0][x] |= FLAG_BLOCK_FACE_NY;
    }
    else if (chunk->block[z][y + 1][x])
        chunk->block[z][y + 1][x] |= FLAG_BLOCK_FACE_NY;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) ||
            (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][CHUNK_DIAMETER - 1][x])
            chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][CHUNK_DIAMETER - 1][x] |= FLAG_BLOCK_FACE_PY;
    }
    else if (chunk->block[z][y - 1][x])
        chunk->block[z][y - 1][x] |= FLAG_BLOCK_FACE_PY;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_LAYER]->block[0][y][x])
            chunk_tab[index + CHUNK_BUF_LAYER]->block[0][y][x] |= FLAG_BLOCK_FACE_NZ;
    }
    else if (chunk->block[z + 1][y][x])
        chunk->block[z + 1][y][x] |= FLAG_BLOCK_FACE_NZ;

    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) ||
            (chunk_tab[index - CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_LAYER]->block[CHUNK_DIAMETER - 1][y][x])
            chunk_tab[index - CHUNK_BUF_LAYER]->block[CHUNK_DIAMETER - 1][y][x] |= FLAG_BLOCK_FACE_PZ;
    }
    else if (chunk->block[z - 1][y][x])
        chunk->block[z - 1][y][x] |= FLAG_BLOCK_FACE_PZ;

    chunk->block[z][y][x] = 0;
    chunk->flag |= FLAG_CHUNK_DIRTY;
}

v3f32 random(f32 x0, f32 y0)
{
    const u32 w = 8 * sizeof(unsigned);
    const u32 s = w / 2; 
    u32 a = (i32)x0, b = (i32)y0;
    a *= 3284157443;
 
    b ^= a << s | a >> w - s;
    b *= 1911520717;
 
    a ^= b << s | b >> w - s;
    a *= 2048419325;
    f32 random = a * (PI / ~(~0u >> 1));
    
    return
        (v3f32){
            sin(random),
                cos(random),
                1.0f,
        };
}

f32 gradient(f32 x0, f32 y0, f32 x, f32 y)
{
    v3f32 grad = random(x0, y0);
    f32 dx = x0 - x;
    f32 dy = y0 - y;
    return ((dx * grad.x) + (dy * grad.y));
}

f32 interp(f32 a, f32 b, f32 scale)
{
    return (b - a) * (3.0f - scale * 2.0f) * scale * scale + a;
}

static f32
terrain_noise(v3i32 coordinates, f32 amplitude, f32 frequency)
{
    f32 root = 0;
    f32 x = (f32)coordinates.x / frequency;
    f32 y = (f32)coordinates.y / frequency;
    i32 x0 = (i32)x;
    i32 y0 = (i32)y;
    i32 x1 = x0 + 1;
    i32 y1 = y0 + 1;

    f32 sx = x - (f32)x0;
    f32 sy = y - (f32)y0;

    f32 n0 = gradient(x0, y0, x, y);
    f32 n1 = gradient(x1, y0, x, y);
    f32 ix0 = interp(n0, n1, sx);

    n0 = gradient(x0, y1, x, y);
    n1 = gradient(x1, y1, x, y);
    f32 ix1 = interp(n0, n1, sx);

    return interp(ix0, ix1, sy) * amplitude;
}

static void
chunk_generate(u32 index, u32 rate)
{
    Chunk *chunk = chunk_tab[index];
    if (!chunk) return;
    u32 *i = &chunk->cursor;
    for (; *i < CHUNK_VOLUME && rate; ++*i)
    {
        v3u32 pos =
        {
            *i % CHUNK_DIAMETER,
            (*i / CHUNK_DIAMETER) % CHUNK_DIAMETER,
            *i / (CHUNK_DIAMETER * CHUNK_DIAMETER),
        };
        v3i32 coordinates =
        {
            pos.x + (chunk->pos.x * CHUNK_DIAMETER),
            pos.y + (chunk->pos.y * CHUNK_DIAMETER),
            pos.z + (chunk->pos.z * CHUNK_DIAMETER),
        };

        if (terrain_noise(coordinates, 40.0f, 64.0f) > coordinates.z)
            add_block(index, pos.x, pos.y, pos.z);
        --rate;
    }

    if (chunk->cursor == CHUNK_VOLUME &&
            !(chunk->flag & FLAG_CHUNK_GENERATED))
        _mesh_chunk(chunk);
}

static void
mesh_chunk(Chunk *chunk)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;
    if (!chunk || !chunk->vbo || !chunk->vao) return;
    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *i = &chunk->block[0][0][0];
    u32 *p = i + CHUNK_VOLUME;
    u64 index = 0;
    v3u64 pos = {0};
    b8 should_render = FALSE;
    for (; i < p; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            index = CHUNK_VOLUME - (u64)(p - i);
            pos =
                (v3u64){
                    index % CHUNK_DIAMETER,
                    (index / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    index / (CHUNK_DIAMETER * CHUNK_DIAMETER),
                };
            *(cursor++) = (u64)*i |
                (((u64)pos.x & 0xf) << 32) |
                (((u64)pos.y & 0xf) << 36) |
                (((u64)pos.z & 0xf) << 40);
        }
    cur_buf = ++cur_buf % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    chunk->vbo_len = len;

    glBindVertexArray(chunk->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64),
            buf, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    chunk->flag &= ~FLAG_CHUNK_DIRTY;

    if (should_render)
    {
        chunk->flag |= FLAG_CHUNK_RENDER;
        chunk->color = COLOR_CHUNK_RENDER;
    }
    else
    {
        chunk->flag &= ~FLAG_CHUNK_RENDER;
        chunk->color = COLOR_CHUNK_LOADED;
    }
}
static void
_mesh_chunk(Chunk *chunk)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    if (!chunk) return;
    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *i = &chunk->block[0][0][0];
    u32 *p = i + CHUNK_VOLUME;
    u64 index = 0;
    v3u64 pos = {0};
    b8 should_render = FALSE;
    for (; i < p; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            index = CHUNK_VOLUME - (u64)(p - i);
            pos =
                (v3u64){
                    index % CHUNK_DIAMETER,
                    (index / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    index / (CHUNK_DIAMETER * CHUNK_DIAMETER),
                };
            *(cursor++) = (u64)*i |
                (((u64)pos.x & 0xf) << 32) |
                (((u64)pos.y & 0xf) << 36) |
                (((u64)pos.z & 0xf) << 40);
        }
    cur_buf = ++cur_buf % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    chunk->vbo_len = len;

    (chunk->vbo) ? glDeleteBuffers(1, &chunk->vbo) : 0;
    (chunk->vao) ? glDeleteVertexArrays(1, &chunk->vao) : 0;

    glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
    glBindVertexArray(chunk->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64),
            buf, GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64),
            (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64),
            (void*)sizeof(u32));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    chunk->flag |= FLAG_CHUNK_GENERATED | FLAG_CHUNK_DIRTY;

    if (should_render)
    {
        chunk->flag |= FLAG_CHUNK_RENDER;
        chunk->color = COLOR_CHUNK_RENDER;
    }
    else
    {
        chunk->flag &= ~FLAG_CHUNK_RENDER;
        chunk->color = COLOR_CHUNK_LOADED;
    }
}

/* TODO: make serialize_chunk() */
/* TODO: add 'version' byte for serialization */
static void
serialize_chunk(Chunk *chunk, str *world_name)
{
}

/* TODO: make deserialize_chunk() */
static void
deserialize_chunk(Chunk *chunk, str *world_name)
{
}

static void
chunk_buf_push(u32 index, v3i16 player_delta_chunk, u32 distance)
{
    v3u32 coordinates =
    {
        index % CHUNK_BUF_DIAMETER,
        (index / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
        index / CHUNK_BUF_LAYER,
    };

    for (u16 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if (chunk_buf[i].flag & FLAG_CHUNK_LOADED) continue;

        chunk_buf[i] = (Chunk){0};
        chunk_buf[i].pos =
            (v3i16){
                player_delta_chunk.x + (coordinates.x - CHUNK_BUF_RADIUS),
                player_delta_chunk.y + (coordinates.y - CHUNK_BUF_RADIUS),
                player_delta_chunk.z + (coordinates.z - CHUNK_BUF_RADIUS),
            };
        chunk_buf[i].id =
            ((u64)(chunk_buf[i].pos.x & 0xffff) << 32) |
            ((u64)(chunk_buf[i].pos.y & 0xffff) << 16) |
            ((u64)(chunk_buf[i].pos.z & 0xffff));
        chunk_buf[i].distance = distance;

        chunk_buf[i].flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY;
        chunk_buf[i].color = COLOR_CHUNK_LOADED;
        chunk_tab[index] = &chunk_buf[i];
        return;
    }
    LOGERROR("%s\n", "chunk_buf Full");
}

static void
chunk_buf_pop(u32 index)
{
    chunk_tab[index]->vbo ? glDeleteBuffers(1, &chunk_tab[index]->vbo) : 0;
    chunk_tab[index]->vao ? glDeleteVertexArrays(1, &chunk_tab[index]->vao) : 0;
    *chunk_tab[index] = (Chunk){0};
    chunk_tab[index] = NULL;
}

/* TODO: grab chunks from disk if previously generated */
void
chunk_queue_update(u32 rate_chunk, u32 rate_block)
{
    /* ---- push chunk queue ------------------------------------------------ */
    u32 i, j, *k = &chunk_queue.cursor;
    if (chunk_queue.count == CHUNK_QUEUE_MAX) goto generate;
    for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        Chunk *chunk = **(chunk_order + i);
        if (chunk && (chunk->flag & FLAG_CHUNK_DIRTY))
        {
            if (!(chunk->flag & FLAG_CHUNK_QUEUED) &&
                    !(chunk_queue.chunk[*k]))
            {
                chunk->flag |= FLAG_CHUNK_QUEUED;
                chunk_queue.chunk[*k] = chunk;
                chunk_queue.index[*k] = chunk_order[i] - chunk_tab;
                (chunk_queue.count < CHUNK_QUEUE_MAX) ?
                    ++chunk_queue.count : 0;
            }
            *k = ++*k % CHUNK_QUEUE_MAX;
        }
    }

    if (!chunk_queue.count) return;

generate:
    rate_chunk = clamp_u32(rate_chunk, 1, CHUNK_QUEUE_MAX - 1);
    rate_block = clamp_u32(rate_block, 1, BLOCK_PARSE_RATE_MAX - 1);

    /* ---- generate enqueued chunks ---------------------------------------- */
    for (i = 0; i < CHUNK_QUEUE_MAX && rate_chunk; ++i)
        if (chunk_queue.chunk[i])
        {
            if (chunk_queue.chunk[i]->flag & FLAG_CHUNK_GENERATED)
                mesh_chunk(chunk_queue.chunk[i]);
            else chunk_generate(chunk_queue.index[i], rate_block);
            chunk_queue.chunk[i]->flag &= ~FLAG_CHUNK_QUEUED;
            chunk_queue.chunk[i] = NULL;
            (chunk_queue.count > 0) ? --chunk_queue.count : 0;
            --rate_chunk;
        }
}

void
shift_chunk_tab(v3i16 player_chunk, v3i16 *player_delta_chunk)
{
    v3u32 coordinates = {0};
    u32 mirror_index = 0;
    v3u32 _mirror_index = {0};
    u32 target_index = 0;
    u8 is_on_edge = 0;
    u32 i = 0;
    const v3i16 DELTA =
    {
        player_chunk.x - player_delta_chunk->x,
        player_chunk.y - player_delta_chunk->y,
        player_chunk.z - player_delta_chunk->z,
    };
    const u8 AXIS =
        (DELTA.x > 0) ? STATE_CHUNK_SHIFT_PX :
        (DELTA.x < 0) ? STATE_CHUNK_SHIFT_NX :
        (DELTA.y > 0) ? STATE_CHUNK_SHIFT_PY :
        (DELTA.y < 0) ? STATE_CHUNK_SHIFT_NY :
        (DELTA.z > 0) ? STATE_CHUNK_SHIFT_PZ :
        (DELTA.z < 0) ? STATE_CHUNK_SHIFT_NZ : 0;
    const i8 INCREMENT = (AXIS % 2 == 1) - (AXIS %2 == 0);
    const i32 RENDER_DISTANCE = (i32)powf(settings.render_distance, 2.0f) + 2;

    if (distance_v3i32(
                (v3i32){
                player_chunk.x,
                player_chunk.y,
                player_chunk.z},
                (v3i32){
                player_delta_chunk->x,
                player_delta_chunk->y,
                player_delta_chunk->z}) > RENDER_DISTANCE)
    {
        for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
            if (chunk_tab[i])
                chunk_buf_pop(i);

        *player_delta_chunk = player_chunk;
        return;
    }

    switch (AXIS)
    {
        case STATE_CHUNK_SHIFT_PX:
        case STATE_CHUNK_SHIFT_NX:
            player_delta_chunk->x += INCREMENT;
            break;

        case STATE_CHUNK_SHIFT_PY:
        case STATE_CHUNK_SHIFT_NY:
            player_delta_chunk->y += INCREMENT;
            break;

        case STATE_CHUNK_SHIFT_PZ:
        case STATE_CHUNK_SHIFT_NZ:
            player_delta_chunk->z += INCREMENT;
            break;
    }

    for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
        if (chunk_tab[i])
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_EDGE;
            chunk_tab[i]->flag |= FLAG_CHUNK_DIRTY;
        }

    /* ---- mark chunks on-edge --------------------------------------------- */
    for (i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if (!chunk_tab[i]) continue;

        v3u32 coordinates =
        {
            i % CHUNK_BUF_DIAMETER,
            (i / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
            i / CHUNK_BUF_LAYER,
        };

        _mirror_index =
        (v3u32){
            i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2),

            (coordinates.z * CHUNK_BUF_LAYER) +
                ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) *
                 CHUNK_BUF_DIAMETER) + coordinates.x,

            ((CHUNK_BUF_DIAMETER - 1 - coordinates.z) * CHUNK_BUF_LAYER) +
                (coordinates.y * CHUNK_BUF_DIAMETER) + coordinates.x,
        };

        v3u8 _is_on_edge = {0};

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge =
                    (v3u8){
                        (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
                            (chunk_tab[i + 1] == NULL),

                        (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
                            (chunk_tab[i + CHUNK_BUF_DIAMETER] == NULL),

                        (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
                            (chunk_tab[i + CHUNK_BUF_LAYER] == NULL),
                    };
                break;

            case 1:
                _is_on_edge =
                    (v3u8){
                        (coordinates.x == 0) || (chunk_tab[i - 1] == NULL),

                        (coordinates.y == 0) ||
                            (chunk_tab[i - CHUNK_BUF_DIAMETER] == NULL),

                        (coordinates.z == 0) ||
                            (chunk_tab[i - CHUNK_BUF_LAYER] == NULL),
                    };
                break;
        }

        switch (AXIS)
        {
            case STATE_CHUNK_SHIFT_PX:
            case STATE_CHUNK_SHIFT_NX:
                mirror_index = _mirror_index.x;
                is_on_edge = _is_on_edge.x;
                break;

            case STATE_CHUNK_SHIFT_PY:
            case STATE_CHUNK_SHIFT_NY:
                mirror_index = _mirror_index.y;
                is_on_edge = _is_on_edge.y;
                break;

            case STATE_CHUNK_SHIFT_PZ:
            case STATE_CHUNK_SHIFT_NZ:
                mirror_index = _mirror_index.z;
                is_on_edge = _is_on_edge.z;
                break;
        }

        if (is_on_edge)
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_RENDER;
            chunk_tab[i]->flag &= ~FLAG_CHUNK_LOADED;
            chunk_tab[i]->color = 0;
            if (chunk_tab[mirror_index])
                chunk_tab[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    /* ---- shift chunk_tab ------------------------------------------------- */
    for (i = (INCREMENT == 1) ? 0 : CHUNK_BUF_VOLUME - 1;
            i < CHUNK_BUF_VOLUME; i += INCREMENT)
    {
        if (!chunk_tab[i]) continue;

        v3u32 coordinates =
        {
            i % CHUNK_BUF_DIAMETER,
            (i / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER,
            i / CHUNK_BUF_LAYER,
        };

        _mirror_index =
        (v3u32){
            i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2),

            (coordinates.z * CHUNK_BUF_LAYER) +
                ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) *
                 CHUNK_BUF_DIAMETER) + coordinates.x,

            ((CHUNK_BUF_DIAMETER - 1 - coordinates.z) * CHUNK_BUF_LAYER) +
                (coordinates.y * CHUNK_BUF_DIAMETER) + coordinates.x,
        };

        v3u32 _target_index = {0};

        switch (INCREMENT)
        {
            case -1:
                _target_index = (v3u32){
                        (coordinates.x == 0) ? i : i - 1,
                        (coordinates.y == 0) ? i : i - CHUNK_BUF_DIAMETER,
                        (coordinates.z == 0) ? i : i - CHUNK_BUF_LAYER};
                break;

            case 1:
                _target_index = (v3u32){
                        (coordinates.x == CHUNK_BUF_DIAMETER - 1) ? i : i + 1,

                        (coordinates.y == CHUNK_BUF_DIAMETER - 1) ?
                            i : i + CHUNK_BUF_DIAMETER,

                        (coordinates.z == CHUNK_BUF_DIAMETER - 1) ?
                            i : i + CHUNK_BUF_LAYER};
                break;
        }

        switch (AXIS)
        {
            case STATE_CHUNK_SHIFT_PX:
            case STATE_CHUNK_SHIFT_NX:
                mirror_index = _mirror_index.x;
                target_index = _target_index.x;
                break;

            case STATE_CHUNK_SHIFT_PY:
            case STATE_CHUNK_SHIFT_NY:
                mirror_index = _mirror_index.y;
                target_index = _target_index.y;
                break;

            case STATE_CHUNK_SHIFT_PZ:
            case STATE_CHUNK_SHIFT_NZ:
                mirror_index = _mirror_index.z;
                target_index = _target_index.z;
                break;
        }

        chunk_tab[i] = chunk_tab[target_index];
        if (chunk_tab[i] && chunk_tab[i]->flag & FLAG_CHUNK_EDGE)
            chunk_tab[target_index] = NULL;
    }
}

u16
get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target)
{
    v3i16 offset =
    {
        (i16)floorf((f64)player_delta_target.x / CHUNK_DIAMETER) -
            player_chunk.x + CHUNK_BUF_RADIUS,

        (i16)floorf((f64)player_delta_target.y / CHUNK_DIAMETER) -
            player_chunk.y + CHUNK_BUF_RADIUS,

        (i16)floorf((f64)player_delta_target.z / CHUNK_DIAMETER) -
            player_chunk.z + CHUNK_BUF_RADIUS,
    };
    return offset.x +
        (offset.y * CHUNK_BUF_DIAMETER) +
        (offset.z * CHUNK_BUF_LAYER);
}

#ifdef FUCK // TODO: undef FUCK
/* raylib/rmodels.c/DrawLine3D refactored; */
void
draw_line_3d(v3i32 pos_0, v3i32 pos_1, v4u8 color)
{
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
    rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}

/* raylib/rmodels.c/DrawCubeWires refactored; */
void
draw_block_wires(v3i32 pos)
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

/* raylib/rmodels.c/DrawCubeWires refactored; */
void
draw_bounding_box(Vector3 origin, Vector3 scl, Color col)
{
    rlPushMatrix();
    rlTranslatef(
            origin.x - (scl.x / 2),
            origin.y - (scl.y / 2),
            origin.z - (scl.z / 2));
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

void
draw_bounding_box_clamped(Vector3 origin, Vector3 scl, Color col)
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

#endif // TODO: undef FUCK
