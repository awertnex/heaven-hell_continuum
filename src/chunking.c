#include <stdlib.h>

#include <engine/h/memory.h>
#include <engine/h/math.h>
#include <engine/h/logger.h>

#include "h/main.h"
#include "h/chunking.h"
#include "h/logic.h"

/* chunk buffer, raw chunk data */
static Chunk *chunk_buf = {0};

/* chunk pointer look-up table */
Chunk *chunk_tab[CHUNK_BUF_VOLUME] = {0};

static struct Globals
{
    f32 opacity;
} globals;

/* index = (chunk_tab index); */
static void generate_chunk(u32 index);

static f32 terrain_noise(v3i32 coordinates);

/* index = (chunk_tab index); */
static void mesh_chunk(u32 index);

static void serialize_chunk(Chunk *chunk, str *world_name);
static void deserialize_chunk(Chunk *chunk, str *world_name);

/* index = (chunk_tab index); */
static void push_chunk_buf(u32 index, v3i16 player_delta_chunk);

/* index = (chunk_tab index); */
static void pop_chunk_buf(u32 index);

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

    for (u16 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        v3u32 coordinates = index_to_coordinates_v3u32(i, CHUNK_BUF_DIAMETER);

        if (distance_v3i32(
                    (v3i32){
                    CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                    (v3i32){
                    coordinates.x, coordinates.y, coordinates.z}) <
                RENDER_DISTANCE)
        {
            if (chunk_tab[i] == NULL)
                push_chunk_buf(i, player_delta_chunk);

            /* TODO: grab chunks from disk if previously generated */
            if (chunk_tab[i] &&
                    !(chunk_tab[i]->flag & FLAG_CHUNK_GENERATED))
                generate_chunk(i);
        }
        else if (chunk_tab[i] &&
                chunk_tab[i]->flag & FLAG_CHUNK_LOADED)
            pop_chunk_buf(i);
    }
}

void
free_chunking(void)
{
    mem_free((void*)&chunk_buf, CHUNK_BUF_VOLUME * sizeof(Chunk), "chunk_buf");
}

void
add_block(u32 index, u32 x, u32 y, u32 z)
{
    v3u32 coordinates = index_to_coordinates_v3u32(index, CHUNK_BUF_DIAMETER);
    u8 is_on_edge = 0;

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + 1] == NULL);

        if (!is_on_edge && chunk_tab[index + 1]->block[z][y][get_mirror_axis(x)])
            chunk_tab[index + 1]->block[z][y][get_mirror_axis(x)] &= ~NEGATIVE_X;
        else chunk_tab[index]->block[z][y][x] |= POSITIVE_X;
    }
    else if (chunk_tab[index]->block[z][y][x + 1])
        chunk_tab[index]->block[z][y][x + 1] &= ~NEGATIVE_X;
    else chunk_tab[index]->block[z][y][x] |= POSITIVE_X;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || (chunk_tab[index - 1] == NULL);

        if (!is_on_edge && chunk_tab[index - 1]->block[z][y][get_mirror_axis(x)])
            chunk_tab[index - 1]->block[z][y][get_mirror_axis(x)] &= ~POSITIVE_X;
        else chunk_tab[index]->block[z][y][x] |= NEGATIVE_X;
    }
    else if (chunk_tab[index]->block[z][y][x - 1])
        chunk_tab[index]->block[z][y][x - 1] &= ~POSITIVE_X;
    else chunk_tab[index]->block[z][y][x] |= NEGATIVE_X;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x])
            chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x] &= ~NEGATIVE_Y;
        else chunk_tab[index]->block[z][y][x] |= POSITIVE_Y;
    }
    else if (chunk_tab[index]->block[z][y + 1][x])
        chunk_tab[index]->block[z][y + 1][x] &= ~NEGATIVE_Y;
    else chunk_tab[index]->block[z][y][x] |= POSITIVE_Y;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) ||
            (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && (chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x]))
            chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x] &= ~POSITIVE_Y;
        else chunk_tab[index]->block[z][y][x] |= NEGATIVE_Y;
    }
    else if (chunk_tab[index]->block[z][y - 1][x])
        chunk_tab[index]->block[z][y - 1][x] &= ~POSITIVE_Y;
    else chunk_tab[index]->block[z][y][x] |= NEGATIVE_Y;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x])
            chunk_tab[index + CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x] &= ~NEGATIVE_Z;
        else chunk_tab[index]->block[z][y][x] |= POSITIVE_Z;
    }
    else if (chunk_tab[index]->block[z + 1][y][x])
        chunk_tab[index]->block[z + 1][y][x] &= ~NEGATIVE_Z;
    else chunk_tab[index]->block[z][y][x] |= POSITIVE_Z;
    
    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) ||
            (chunk_tab[index - CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x])
            chunk_tab[index - CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x] &= ~POSITIVE_Z;
        else chunk_tab[index]->block[z][y][x] |= NEGATIVE_Z;
    }
    else if (chunk_tab[index]->block[z - 1][y][x])
        chunk_tab[index]->block[z - 1][y][x] &= ~POSITIVE_Z;
    else chunk_tab[index]->block[z][y][x] |= NEGATIVE_Z;

    chunk_tab[index]->block[z][y][x] |= NOT_EMPTY;
    chunk_tab[index]->block[z][y][x] |=
        (((u64)x & 0xf) << 32) |
        (((u64)y & 0xf) << 36) |
        (((u64)z & 0xf) << 40);
}

void
remove_block(u32 index, u32 x, u32 y, u32 z)
{
    v3u32 coordinates = index_to_coordinates_v3u32(index, CHUNK_BUF_DIAMETER);

    u8 is_on_edge = 0;

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + 1] == NULL);

        if (!is_on_edge && chunk_tab[index + 1]->block[z][y][get_mirror_axis(x)])
            chunk_tab[index + 1]->block[z][y][get_mirror_axis(x)] |= NEGATIVE_X;
    }
    else if (chunk_tab[index]->block[z][y][x + 1])
        chunk_tab[index]->block[z][y][x + 1] |= NEGATIVE_X;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || (chunk_tab[index - 1] == NULL);

        if (!is_on_edge && chunk_tab[index - 1]->block[z][y][get_mirror_axis(x)])
            chunk_tab[index - 1]->block[z][y][get_mirror_axis(x)] |= POSITIVE_X;
    }
    else if (chunk_tab[index]->block[z][y][x - 1])
        chunk_tab[index]->block[z][y][x - 1] |= POSITIVE_X;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x])
            chunk_tab[index + CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x] |= NEGATIVE_Y;
    }
    else if (chunk_tab[index]->block[z][y + 1][x])
        chunk_tab[index]->block[z][y + 1][x] |= NEGATIVE_Y;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) ||
            (chunk_tab[index - CHUNK_BUF_DIAMETER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x])
            chunk_tab[index - CHUNK_BUF_DIAMETER]->block[z][get_mirror_axis(y)][x] |= POSITIVE_Y;
    }
    else if (chunk_tab[index]->block[z][y - 1][x])
        chunk_tab[index]->block[z][y - 1][x] |= POSITIVE_Y;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
            (chunk_tab[index + CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index + CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x])
            chunk_tab[index + CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x] |= NEGATIVE_Z;
    }
    else if (chunk_tab[index]->block[z + 1][y][x])
        chunk_tab[index]->block[z + 1][y][x] |= NEGATIVE_Z;

    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) ||
            (chunk_tab[index - CHUNK_BUF_LAYER] == NULL);

        if (!is_on_edge && chunk_tab[index - CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x])
            chunk_tab[index - CHUNK_BUF_LAYER]->block[get_mirror_axis(z)][y][x] |= POSITIVE_Z;
    }
    else if (chunk_tab[index]->block[z - 1][y][x])
        chunk_tab[index]->block[z - 1][y][x] |= POSITIVE_Z;

    chunk_tab[index]->block[z][y][x] = 0;
}

static f32
terrain_noise(v3i32 coordinates)
{
    return 0.0f;
}

static void
generate_chunk(u32 index)
{
    if (!chunk_tab[index])
        return;

    for (u8 z = 0; z < CHUNK_DIAMETER; ++z)
    {
        for (u8 y = 0; y < CHUNK_DIAMETER; ++y)
        {
            for (u8 x = 0; x < CHUNK_DIAMETER; ++x)
            {
                v3i32 coordinates =
                {
                    x + (chunk_tab[index]->pos.x * CHUNK_DIAMETER),
                    y + (chunk_tab[index]->pos.y * CHUNK_DIAMETER),
                    z + (chunk_tab[index]->pos.z * CHUNK_DIAMETER),
                };

                f32 sin_x = (u32)((sinf((f32)coordinates.x * DEG2RAD)
                             + 1.0f) * 80.0f) + 2;
                f32 sin_y = (u32)((sinf((f32)coordinates.y * DEG2RAD)
                             + 1.0f) * 80.0f) + 2;

                if (chunk_tab[index]->block[z][y][x])
                    add_block(index, x, y, z);
                else if ((f32)coordinates.z <= sin_x - 3.0f &&
                        (f32)coordinates.z <= sin_y - 3.0f)
                {
                    add_block(index, x, y, z);
                    chunk_tab[index]->flag |=
                        FLAG_CHUNK_RENDER;
                    chunk_tab[index]->color = COLOR_CHUNK_RENDER;
                }
            }
        }
    }

    if (chunk_tab[index]->flag & FLAG_CHUNK_GENERATED)
    {
        mesh_chunk(index);
        return;
    }

    chunk_tab[index]->vbo ?
        glDeleteBuffers(1, &chunk_tab[index]->vbo) : 0;
    chunk_tab[index]->vao ?
        glDeleteVertexArrays(1, &chunk_tab[index]->vao) : 0;

    glGenVertexArrays(1, &chunk_tab[index]->vao);
    glGenBuffers(1, &chunk_tab[index]->vbo);

    glBindVertexArray(chunk_tab[index]->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_tab[index]->vbo);

    glBufferData(GL_ARRAY_BUFFER, CHUNK_VOLUME * sizeof(u64),
            chunk_tab[index]->block, GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64),
            (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64),
            (void*)sizeof(u32));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void
mesh_chunk(u32 index)
{
    glBindVertexArray(chunk_tab[index]->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_tab[index]->vbo);

    glBufferData(GL_ARRAY_BUFFER, CHUNK_VOLUME * sizeof(u64),
            chunk_tab[index]->block, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
push_chunk_buf(u32 index, v3i16 player_delta_chunk)
{
    v3u32 coordinates = index_to_coordinates_v3u32(index, CHUNK_BUF_DIAMETER);

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

        chunk_buf[i].flag = FLAG_CHUNK_LOADED;
        chunk_buf[i].color = COLOR_CHUNK_LOADED;
        chunk_tab[index] = &chunk_buf[i];
        return;
    }
    LOGERROR("%s\n", "chunk_buf Full");
}

static void
pop_chunk_buf(u32 index)
{
    chunk_tab[index]->vbo ?
        glDeleteBuffers(1, &chunk_tab[index]->vbo) : 0;
    chunk_tab[index]->vao ?
        glDeleteVertexArrays(1, &chunk_tab[index]->vao) : 0;

    *chunk_tab[index] = (Chunk){0};
    chunk_tab[index] = NULL;
}

void
shift_chunk_tab(v3i16 player_chunk, v3i16 *player_delta_chunk)
{
    v3u32 coordinates = {0};
    u32 mirror_index = 0;
    u32 target_index = 0;
    u8 is_on_edge = 0;
    const v3i16 DELTA =
    {
        player_chunk.x - player_delta_chunk->x,
        player_chunk.y - player_delta_chunk->y,
        player_chunk.z - player_delta_chunk->z,
    };
    const u8 AXIS =
        (DELTA.x > 0) ? SHIFT_PX : (DELTA.x < 0) ? SHIFT_NX :
        (DELTA.y > 0) ? SHIFT_PY : (DELTA.y < 0) ? SHIFT_NY :
        (DELTA.z > 0) ? SHIFT_PZ : (DELTA.z < 0) ? SHIFT_NZ : 0;
    const i8 INCREMENT = (AXIS % 2 == 1) - (AXIS %2 == 0);
    const b8 TOO_FAR = distance_v3f32(
            (v3f32){
            (f32)player_chunk.x, (f32)player_chunk.y, (f32)player_chunk.z},
            (v3f32){
            (f32)player_delta_chunk->x,
            (f32)player_delta_chunk->y,
            (f32)player_delta_chunk->z}) >
        powf(settings.render_distance, 2.0f) + 2.0f;

    if (TOO_FAR)
    {
        for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
            if (chunk_tab[i] != NULL)
                pop_chunk_buf(i);

        *player_delta_chunk = player_chunk;
        return;
    }

    switch (AXIS)
    {
        case SHIFT_PX:
        case SHIFT_NX:
            player_delta_chunk->x += INCREMENT;
            break;

        case SHIFT_PY:
        case SHIFT_NY:
            player_delta_chunk->y += INCREMENT;
            break;

        case SHIFT_PZ:
        case SHIFT_NZ:
            player_delta_chunk->z += INCREMENT;
            break;
    }

    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
        if (chunk_tab[i] != NULL)
            chunk_tab[i]->flag &= ~FLAG_CHUNK_EDGE;

    /* ---- mark chunks on-edge --------------------------------------------- */
    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if (chunk_tab[i] == NULL) continue;

        coordinates = index_to_coordinates_v3u32(i, CHUNK_BUF_DIAMETER);

        v3u32 _mirror_index =
        {
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
            case SHIFT_PX:
            case SHIFT_NX:
                mirror_index = _mirror_index.x;
                is_on_edge = _is_on_edge.x;
                break;

            case SHIFT_PY:
            case SHIFT_NY:
                mirror_index = _mirror_index.y;
                is_on_edge = _is_on_edge.y;
                break;

            case SHIFT_PZ:
            case SHIFT_NZ:
                mirror_index = _mirror_index.z;
                is_on_edge = _is_on_edge.z;
                break;
        }

        if (is_on_edge)
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_RENDER;
            chunk_tab[i]->flag &= ~FLAG_CHUNK_LOADED;
            chunk_tab[i]->color = 0;
            if (chunk_tab[mirror_index] != NULL)
                chunk_tab[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    /* ---- shift chunk_tab ------------------------------------------------- */
    for (u32 i = (INCREMENT == 1) ? 0 : CHUNK_BUF_VOLUME - 1;
            i < CHUNK_BUF_VOLUME; i += INCREMENT)
    {
        if (chunk_tab[i] == NULL) continue;

        coordinates = index_to_coordinates_v3u32(i, CHUNK_BUF_DIAMETER);

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
            case SHIFT_PX:
            case SHIFT_NX:
                target_index = _target_index.x;
                break;

            case SHIFT_PY:
            case SHIFT_NY:
                target_index = _target_index.y;
                break;

            case SHIFT_PZ:
            case SHIFT_NZ:
                target_index = _target_index.z;
                break;
        }

        chunk_tab[i] = chunk_tab[target_index];
        if (chunk_tab[i] != NULL && chunk_tab[i]->flag & FLAG_CHUNK_EDGE)
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

void
draw_chunk_tab(Uniform *uniform)
{
    if (state & FLAG_DEBUG_MORE)
        globals.opacity = 0.2f;
    else
        globals.opacity = 1.0f;
    glUniform1f(uniform->voxel.opacity, globals.opacity);

    for (u16 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if ((chunk_tab[i] == NULL) ||
                !(chunk_tab[i]->flag & FLAG_CHUNK_RENDER))
            continue;

        v3f32 chunk_position =
        {
            (f32)(chunk_tab[i]->pos.x * CHUNK_DIAMETER),
            (f32)(chunk_tab[i]->pos.y * CHUNK_DIAMETER),
            (f32)(chunk_tab[i]->pos.z * CHUNK_DIAMETER),
        };

        glUniform3fv(uniform->voxel.chunk_position, 1,
                (GLfloat*)&chunk_position);

        glBindVertexArray(chunk_tab[i]->vao);
        glDrawArrays(GL_POINTS, 0, CHUNK_VOLUME);
    }
    glBindVertexArray(0);
}

void
draw_chunk_gizmo(Mesh *mesh)
{
    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if ((chunk_tab[i] == NULL) ||
                !(chunk_tab[i]->flag & FLAG_CHUNK_RENDER))
            continue;

        v3f32 cursor = index_to_coordinates_v3f32(i, CHUNK_BUF_DIAMETER);
        cursor = sub_v3f32(cursor, (v3f32){
                CHUNK_BUF_RADIUS + 0.5f,
                CHUNK_BUF_RADIUS + 0.5f,
                CHUNK_BUF_RADIUS + 0.5f});
        glUniform3fv(uniform.gizmo_chunk.cursor, 1,
                (GLfloat*)&cursor);

        f32 pulse = (sinf((cursor.z * 0.3f) -
                    (render.frame_start * 5.0f)) * 0.1f) + 0.9f;
        glUniform1f(uniform.gizmo_chunk.size, pulse);

        v4f32 color =
        {
            (f32)((chunk_tab[i]->color >> 24) & 0xff) / 0xff,
            (f32)((chunk_tab[i]->color >> 16) & 0xff) / 0xff,
            (f32)((chunk_tab[i]->color >> 8) & 0xff) / 0xff,
            (f32)((chunk_tab[i]->color & 0xff)) / 0xff,
        };
        glUniform4fv(uniform.gizmo_chunk.color, 1, (GLfloat*)&color);
        draw_mesh(mesh);
    }
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
