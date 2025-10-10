#include <stdlib.h>

#include <engine/h/memory.h>
#include <engine/h/math.h>
#include <engine/h/logger.h>

#include "h/main.h"
#include "h/chunking.h"

/* queue of chunks to be processed */
static ChunkQueue chunk_queue = {0};

/* chunk buffer, raw chunk data */
Chunk *chunk_buf = {0};

/* chunk pointer look-up table */
Chunk *chunk_tab[CHUNK_BUF_VOLUME] = {0};

/* index = (chunk_tab index); */
static void generate_chunk(u32 index);

static f32 terrain_noise(v3i32 coordinates);

/* index = (chunk_tab index); */
static void mesh_chunk(Chunk *chunk, b8 generate);

static void serialize_chunk(Chunk *chunk, str *world_name);
static void deserialize_chunk(Chunk *chunk, str *world_name);

/* index = (chunk_tab index); */
static void chunk_buf_push(u32 index, v3i16 player_delta_chunk, u32 distance);

/* index = (chunk_tab index); */
static void chunk_buf_pop(u32 index);

static void chunk_queue_sort(void);


u8
init_chunking(void)
{
    if (mem_alloc_memb((void*)&chunk_buf,
                CHUNK_BUF_VOLUME, sizeof(Chunk), "chunk_buf"))
        return 0;

cleanup:
    free_chunking();
    return -1;
}

void
update_chunking(v3i16 player_delta_chunk)
{
    const u32 RENDER_DISTANCE = (u32)powf(settings.render_distance, 2.0f) + 2;

    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
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
    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
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

static f32
terrain_noise(v3i32 coordinates)
{
    return 0.0f;
}

static void
generate_chunk(u32 index)
{
    Chunk *chunk = chunk_tab[index];
    if (!chunk) return;

    for (u8 z = 0; z < CHUNK_DIAMETER; ++z)
    {
        for (u8 y = 0; y < CHUNK_DIAMETER; ++y)
        {
            for (u8 x = 0; x < CHUNK_DIAMETER; ++x)
            {
                v3i32 coordinates =
                {
                    x + (chunk->pos.x * CHUNK_DIAMETER),
                    y + (chunk->pos.y * CHUNK_DIAMETER),
                    z + (chunk->pos.z * CHUNK_DIAMETER),
                };

                f32 sin_x = (u32)((sinf((f32)coordinates.x * DEG2RAD)
                             + 1.0f) * 80.0f) + 2;
                f32 sin_y = (u32)((sinf((f32)coordinates.y * DEG2RAD)
                             + 1.0f) * 80.0f) + 2;

                if ((f32)coordinates.z <= sin_x - 3.0f &&
                        (f32)coordinates.z <= sin_y - 3.0f)
                {
                    add_block(index, x, y, z);
                    chunk->color = COLOR_CHUNK_RENDER;
                }
            }
        }
    }

    if (!(chunk->flag & FLAG_CHUNK_GENERATED))
        mesh_chunk(chunk, TRUE);
}

static void
mesh_chunk(Chunk *chunk, b8 generate)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    if (!chunk) return;
    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    {
        u32 *i = &chunk->block[0][0][0];
        u32 *p = i + CHUNK_VOLUME;
        u64 current = 0;
        v3u64 coordinates = {0};
        for (; i < p; ++i)
            if (*i & MASK_BLOCK_FACES)
            {
                current = CHUNK_VOLUME - (u64)(p - i);
                coordinates =
                (v3u64){
                    current % CHUNK_DIAMETER,
                    (current / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    current / (CHUNK_DIAMETER * CHUNK_DIAMETER),
                };
                *(cursor++) = (u64)*i |
                    (((u64)coordinates.x & 0xf) << 32) |
                    (((u64)coordinates.y & 0xf) << 36) |
                    (((u64)coordinates.z & 0xf) << 40);
            }
    }
    cur_buf = ++cur_buf % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    chunk->vbo_len = len;

    if (generate)
    {
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

        chunk->flag |= FLAG_CHUNK_GENERATED | FLAG_CHUNK_DIRTY;
    }
    else
    {
        glBindVertexArray(chunk->vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
        glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64),
                buf, GL_DYNAMIC_DRAW);

        chunk->flag &= ~FLAG_CHUNK_DIRTY;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    u32 *i = &chunk->block[0][0][0];
    u32 *p = i + CHUNK_VOLUME;
    for (; i < p; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            chunk->flag |= FLAG_CHUNK_RENDER;
            return;
        }
    chunk->flag &= ~FLAG_CHUNK_RENDER;
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

static void
chunk_queue_sort(void)
{
    Chunk *chunk_0 = NULL;
    Chunk *chunk_1 = NULL;
    Chunk *chunk_temp = NULL;
    u32 *index_0 = NULL;
    u32 *index_1 = NULL;
    u32 index_temp = 0;
    for (u32 i = 0; i < CHUNK_QUEUE_MAX; ++i)
    {
        chunk_0 = chunk_queue.chunk[i];
        chunk_1 = chunk_queue.chunk[i];

        index_0 = &chunk_queue.index[i];
        index_1 = &chunk_queue.index[i];

        for (u32 j = i; j < CHUNK_QUEUE_MAX; ++j)
        {
            if (chunk_0 && chunk_1 &&
                    chunk_0->distance > chunk_1->distance)
            {
                chunk_temp = chunk_0;
                *chunk_0 = *chunk_1;
                *chunk_1 = *chunk_temp;

                index_temp = *index_0;
                *index_0 = *index_1;
                *index_1 = index_temp;
            }
            ++chunk_1;
            ++index_1;
        }
        LOGWARNING("Dist: %d %d\n", chunk_1->distance, *index_1);
    }
    LOGWARNING("%s\n", "");
}

/* TODO: grab chunks from disk if previously generated */
void
chunk_queue_update(u32 rate)
{
    /* ---- push chunk queue ------------------------------------------------ */
    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        Chunk *p = chunk_tab[i];
        if (p && (p->flag & FLAG_CHUNK_DIRTY) &&
                !(p->flag & FLAG_CHUNK_QUEUED))
        {
            for (u32 j = 0; j < CHUNK_QUEUE_MAX; ++j)
                if (!chunk_queue.chunk[j] &&
                        !(p->flag & FLAG_CHUNK_QUEUED))
                {
                    p->flag |= FLAG_CHUNK_QUEUED;
                    chunk_queue.chunk[j] = p;
                    chunk_queue.index[j] = i;
                    (chunk_queue.count < CHUNK_QUEUE_MAX) ?
                        ++chunk_queue.count : 0;
                }
        }
    }

    if (!chunk_queue.count) return;
    rate = clamp_u32(rate, 1, CHUNK_QUEUE_MAX - 1);

    //chunk_queue_sort();

    /* ---- generate enqueued chunks ---------------------------------------- */
    for (u32 i = chunk_queue.count - 1; i < CHUNK_QUEUE_MAX; --i)
        if (chunk_queue.chunk[i])
        {
            rate = i - rate;
            for (u32 j = i; j > rate && j < CHUNK_QUEUE_MAX; --j)
                if (chunk_queue.chunk[j])
                {
                    if (chunk_queue.chunk[j]->flag & FLAG_CHUNK_GENERATED)
                        mesh_chunk(chunk_queue.chunk[j], FALSE);
                    else generate_chunk(chunk_queue.index[j]);
                    chunk_queue.chunk[j]->flag &= ~FLAG_CHUNK_QUEUED;
                    chunk_queue.chunk[j] = NULL;
                    (chunk_queue.count > 0) ? --chunk_queue.count : 0;
                }
            break;
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
        for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
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

    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
        if (chunk_tab[i])
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_EDGE;
            chunk_tab[i]->flag |= FLAG_CHUNK_DIRTY;
        }

    /* ---- mark chunks on-edge --------------------------------------------- */
    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
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
    for (u32 i = (INCREMENT == 1) ? 0 : CHUNK_BUF_VOLUME - 1;
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
