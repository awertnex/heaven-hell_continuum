#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <engine/h/diagnostics.h>
#include <engine/h/dir.h>
#include <engine/h/memory.h>
#include <engine/h/math.h>
#include <engine/h/logger.h>
#include <engine/h/platform.h>

#include "h/main.h"
#include "h/assets.h"
#include "h/chunking.h"
#include "h/dir.h"

u64 CHUNKS_MAX[SET_RENDER_DISTANCE_MAX + 1] = {0};

/* chunk buffer, raw chunk data */
static Chunk *chunk_buf = NULL;
static u64 chunk_buf_cursor = 0;

/* chunk pointer look-up table that points to chunk_buf addresses.
 * mapping of chunk_buf entries to chunk positions in 3d space */
Chunk **chunk_tab = NULL;

/* chunk pointer pointer look-up table that points to chunk_tab addresses.
 * order of chunk_tab based on distance away from player */
Chunk ***CHUNK_ORDER = NULL;

/* queues of chunks to be processed */
ChunkQueue CHUNK_QUEUE_1 = {0};
ChunkQueue CHUNK_QUEUE_2 = {0};
ChunkQueue CHUNK_QUEUE_3 = {0};

/* -- INTERNAL USE ONLY -- */
static void _block_place(Chunk *chunk,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 coordinates, u32 x, u32 y, u32 z, BlockID block_id);

/* -- INTERNAL USE ONLY -- */
static void _block_break(Chunk *chunk,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 coordinates, u32 x, u32 y, u32 z);

/* index = (chunk_tab index),
 * rate = number of blocks to process per chunk per frame */
static void chunk_generate(Chunk **chunk, u32 rate);

static f32 terrain_noise(v3i32 coordinates, f32 amplitude, f32 frequency);
static void chunk_mesh_update(Chunk *chunk);

/* -- INTERNAL USE ONLY -- */
static void chunk_mesh_init(Chunk *chunk);

/* -- INTERNAL USE ONLY -- */
static void _chunk_serialize(Chunk *chunk, str *world_name);

/* -- INTERNAL USE ONLY -- */
static void _chunk_deserialize(Chunk *chunk, str *world_name);

/* -- INTERNAL USE ONLY --;
 *
 * index = (chunk_tab index) */
static void _chunk_buf_push(u32 index, v3i16 player_delta_chunk);

/* -- INTERNAL USE ONLY --;
 *
 * index = (chunk_tab index) */
static void _chunk_buf_pop(u32 index);

u32 chunking_init(void)
{
    str CHUNK_ORDER_lookup_file_name[PATH_MAX] = {0};
    str CHUNKS_MAX_lookup_file_name[PATH_MAX] = {0};
    u32 *CHUNK_ORDER_lookup_file_contents = NULL;
    u64 *CHUNKS_MAX_lookup_file_contents = NULL;
    u64 file_len = 0;

    u32 render_distance = 0;
    v3i32 center = {0};
    v3i32 coordinates = {0};
    u32 *distance = NULL;
    u32 *index = NULL;
    u64 chunk_buf_diameter = 0;
    u64 chunk_buf_volume = 0;
    u64 chunks_max = 0;
    u64 i = 0, j = 0, k = 0;

    if (mem_map((void*)&chunk_buf, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk),
                "chunking_init().chunk_buf") != ERR_SUCCESS)
        return *GAME_ERR;

    if (mem_map((void*)&chunk_tab, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk*),
                "chunking_init().chunk_tab") != ERR_SUCCESS)
        goto cleanup;

    if (mem_map((void*)&CHUNK_ORDER, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_ORDER") != ERR_SUCCESS)
        goto cleanup;

    if (mem_map((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().distance") != ERR_SUCCESS)
        goto cleanup;

    if (mem_map((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().index") != ERR_SUCCESS)
        goto cleanup;

    /* ---- build distance lookups ------------------------------------------ */
    for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
    {
        chunk_buf_diameter = (i * 2) + 1;
        chunk_buf_volume =
            chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
        render_distance = (i * i) + 2;
        center = (v3i32){i, i, i};

        snprintf(CHUNK_ORDER_lookup_file_name, PATH_MAX,
                "%slookup_chunk_order_0x%02"PRIx64".bin",
                DIR_ROOT[DIR_LOOKUPS], i);

        if (is_file_exists(CHUNK_ORDER_lookup_file_name, FALSE) != ERR_SUCCESS)
        {
            for (j = 0; j < chunk_buf_volume; ++j)
            {
                LOGTRACE(FALSE,
                        "Building CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x] Progress [%ld/%ld]..\n",
                        i, SET_RENDER_DISTANCE_MAX, j, chunk_buf_volume);

                coordinates =
                    (v3i32){
                        j % chunk_buf_diameter,
                        (j / chunk_buf_diameter) % chunk_buf_diameter,
                        j / (chunk_buf_diameter * chunk_buf_diameter),
                    };
                distance[j] = distance_v3i32(coordinates, center);
                index[j] = j;
            }

            LOGTRACE(FALSE,
                    "Sorting CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x]..\n",
                    i, SET_RENDER_DISTANCE_MAX);

            for (j = 0; j < chunk_buf_volume; ++j)
                for (k = 0; k < chunk_buf_volume; ++k)
                    if (distance[j] < distance[k])
                    {
                        swap_bits_u32(&distance[j], &distance[k]);
                        swap_bits_u32(&index[j], &index[k]);
                    }

            LOGTRACE(FALSE,
                    "Writing CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x] To File..\n",
                    i, SET_RENDER_DISTANCE_MAX);

            if (write_file(CHUNK_ORDER_lookup_file_name, sizeof(u32),
                        chunk_buf_volume, index, "wb", TRUE) != ERR_SUCCESS)
                goto cleanup;
        }
    }

    snprintf(CHUNK_ORDER_lookup_file_name, PATH_MAX,
            "%slookup_chunk_order_0x%02x.bin",
            DIR_ROOT[DIR_LOOKUPS], settings.render_distance);

        file_len = get_file_contents(CHUNK_ORDER_lookup_file_name,
                (void*)&CHUNK_ORDER_lookup_file_contents,
                sizeof(u32), "rb", FALSE);
        if (CHUNK_ORDER_lookup_file_contents == NULL)
            goto cleanup;

    for (i = 0; i < settings.chunk_buf_volume; ++i)
        CHUNK_ORDER[i] = &chunk_tab[CHUNK_ORDER_lookup_file_contents[i]];
    mem_free((void*)&CHUNK_ORDER_lookup_file_contents, file_len,
            "chunking_init().CHUNK_ORDER_lookup_file_contents");

    /* ---- build CHUNKS_MAX lookup ----------------------------------------- */
    snprintf(CHUNKS_MAX_lookup_file_name, PATH_MAX,
            "%slookup_chunks_max.bin", DIR_ROOT[DIR_LOOKUPS]);

    if (is_file_exists(CHUNKS_MAX_lookup_file_name, FALSE) != ERR_SUCCESS)
    {
        for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
        {
            LOGTRACE(FALSE,
                    "Building CHUNKS_MAX Lookup, Progress [%"PRId64"/%d]..\n",
                    i, SET_RENDER_DISTANCE_MAX);
            chunk_buf_diameter = (i * 2) + 1;
            chunk_buf_volume =
                chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
            chunks_max = 0;
            render_distance = (i * i) + 2;
            center = (v3i32){i, i, i};

            for (j = 0; j < chunk_buf_volume; ++j)
            {
                coordinates =
                    (v3i32){
                        j % chunk_buf_diameter,
                        (j / chunk_buf_diameter) % chunk_buf_diameter,
                        j / (chunk_buf_diameter * chunk_buf_diameter),
                    };
                if (distance_v3i32(coordinates, center) < render_distance)
                    ++chunks_max;
            }

            CHUNKS_MAX[i] = chunks_max;
        }

        LOGTRACE(FALSE,
                "%s\n", "Writing CHUNKS_MAX Lookup To File..\n");

        if (write_file(CHUNKS_MAX_lookup_file_name,
                sizeof(u64), SET_RENDER_DISTANCE_MAX + 1,
                &CHUNKS_MAX, "wb", TRUE) != ERR_SUCCESS)
            goto cleanup;
    }

        file_len = get_file_contents(CHUNKS_MAX_lookup_file_name,
                (void*)&CHUNKS_MAX_lookup_file_contents,
                sizeof(u64), "rb", FALSE);
        if (CHUNKS_MAX_lookup_file_contents == NULL)
            return engine_err;

    for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
        CHUNKS_MAX[i] = CHUNKS_MAX_lookup_file_contents[i];
    mem_free((void*)&CHUNKS_MAX_lookup_file_contents, file_len,
            "chunking_init().CHUNKS_MAX_lookup_file_contents");

    /* ---- init chunk parsing priority queues ------------------------------ */
    CHUNK_QUEUE_1.id = CHUNK_QUEUE_1ST_ID;
    CHUNK_QUEUE_1.offset = 0;
    CHUNK_QUEUE_1.size =
        (u64)clamp_i64(CHUNKS_MAX[settings.render_distance],
                0, CHUNK_QUEUE_1ST_MAX);
    CHUNK_QUEUE_1.rate_chunk = CHUNK_PARSE_RATE_PRIORITY_HIGH;
    CHUNK_QUEUE_1.rate_block = BLOCK_PARSE_RATE;

    CHUNK_QUEUE_2.id = CHUNK_QUEUE_2ND_ID;
    CHUNK_QUEUE_2.offset = CHUNK_QUEUE_1ST_MAX;
    CHUNK_QUEUE_2.size =
        (u64)clamp_i64(CHUNKS_MAX[settings.render_distance] -
                CHUNK_QUEUE_2.offset, 0, CHUNK_QUEUE_2ND_MAX);
    CHUNK_QUEUE_2.rate_chunk = CHUNK_PARSE_RATE_PRIORITY_MID;
    CHUNK_QUEUE_2.rate_block = BLOCK_PARSE_RATE;

    CHUNK_QUEUE_3.id = CHUNK_QUEUE_3RD_ID;
    CHUNK_QUEUE_3.offset = CHUNK_QUEUE_1ST_MAX + CHUNK_QUEUE_2ND_MAX;
    CHUNK_QUEUE_3.size =
        (u64)clamp_i64(CHUNKS_MAX[settings.render_distance] -
            CHUNK_QUEUE_3.offset, 0, CHUNK_QUEUE_3RD_MAX);
    CHUNK_QUEUE_3.rate_chunk = CHUNK_PARSE_RATE_PRIORITY_LOW;
    CHUNK_QUEUE_3.rate_block = BLOCK_PARSE_RATE;

    if (mem_map((void*)&CHUNK_QUEUE_1.queue,
                CHUNK_QUEUE_1ST_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE_1.queue") != ERR_SUCCESS)
        goto cleanup;

    if (mem_map((void*)&CHUNK_QUEUE_2.queue,
                CHUNK_QUEUE_2ND_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE_2.queue") != ERR_SUCCESS)
        goto cleanup;

    if (mem_map((void*)&CHUNK_QUEUE_3.queue,
                CHUNK_QUEUE_3RD_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE_3.queue") != ERR_SUCCESS)
        goto cleanup;

    mem_unmap((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_free().distance");

    mem_unmap((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_free().index");

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;

cleanup:
    mem_unmap((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_free().distance");

    mem_unmap((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_free().index");

    chunking_free();
    return *GAME_ERR;
}

void chunking_update(v3i16 player_delta_chunk)
{
    u32 i = 0;
    for (; i < CHUNKS_MAX[settings.render_distance]; ++i)
    {
        Chunk **p = CHUNK_ORDER[i];
        u32 index = p - chunk_tab;
        if (!*p) _chunk_buf_push(index, player_delta_chunk);
    }
}

void chunking_free(void)
{
    if (chunk_tab)
    {
        u32 i = 0;
        for (; i < settings.chunk_buf_volume; ++i)
            if (chunk_tab[i])
                _chunk_buf_pop(i);
    }

    mem_unmap((void*)&chunk_buf,
            CHUNK_BUF_VOLUME_MAX * sizeof(Chunk),
            "chunking_free().chunk_buf");

    mem_unmap((void*)&chunk_tab,
            CHUNK_BUF_VOLUME_MAX * sizeof(Chunk*),
            "chunking_free().chunk_tab");

    mem_unmap((void*)&CHUNK_ORDER,
            CHUNK_BUF_VOLUME_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_ORDER");

    mem_unmap((void*)&CHUNK_QUEUE_1.queue,
            CHUNK_QUEUE_1ST_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_QUEUE_1.queue");

    mem_unmap((void*)&CHUNK_QUEUE_2.queue,
            CHUNK_QUEUE_2ND_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_QUEUE_2.queue");

    mem_unmap((void*)&CHUNK_QUEUE_3.queue,
            CHUNK_QUEUE_3RD_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_QUEUE_3.queue");
}

void block_place(u32 index, u32 x, u32 y, u32 z, BlockID block_id)
{
    if (chunk_tab[index]->block[z][y][x]) return;
    v3u32 coordinates =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };

    Chunk **chunk = &chunk_tab[index];
    Chunk *px = *(chunk + 1);
    Chunk *nx = *(chunk - 1);
    Chunk *py = *(chunk + settings.chunk_buf_diameter);
    Chunk *ny = *(chunk - settings.chunk_buf_diameter);
    Chunk *pz = *(chunk + settings.chunk_buf_layer);
    Chunk *nz = *(chunk - settings.chunk_buf_layer);
    _block_place(*chunk, px, nx, py, ny, pz, nz, coordinates, x, y, z,
            block_id);
}

static void _block_place(Chunk *chunk,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 coordinates, u32 x, u32 y, u32 z, BlockID block_id)
{
    u8 is_on_edge = 0;

    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;
    SET_BLOCK_ID(chunk->block[z][y][x], block_id);

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == settings.chunk_buf_diameter - 1) || !px;
        if (!is_on_edge && px->block[z][y][0])
        {
            px->block[z][y][0] &= ~FLAG_BLOCK_FACE_NX;
            px->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PX;
    }
    else if (chunk->block[z][y][x + 1])
        chunk->block[z][y][x + 1] &= ~FLAG_BLOCK_FACE_NX;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || !nx;
        if (!is_on_edge && nx->block[z][y][CHUNK_DIAMETER - 1])
        {
            nx->block[z][y][CHUNK_DIAMETER - 1] &= ~FLAG_BLOCK_FACE_PX;
            nx->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NX;
    }
    else if (chunk->block[z][y][x - 1])
        chunk->block[z][y][x - 1] &= ~FLAG_BLOCK_FACE_PX;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == settings.chunk_buf_diameter - 1) || !py;
        if (!is_on_edge && py->block[z][0][x])
        {
            py->block[z][0][x] &= ~FLAG_BLOCK_FACE_NY;
            py->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PY;
    }
    else if (chunk->block[z][y + 1][x])
        chunk->block[z][y + 1][x] &= ~FLAG_BLOCK_FACE_NY;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) || !ny;
        if (!is_on_edge && ny->block[z][CHUNK_DIAMETER - 1][x])
        {
            ny->block[z][CHUNK_DIAMETER - 1][x] &= ~FLAG_BLOCK_FACE_PY;
            ny->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NY;
    }
    else if (chunk->block[z][y - 1][x])
        chunk->block[z][y - 1][x] &= ~FLAG_BLOCK_FACE_PY;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == settings.chunk_buf_diameter - 1) || !pz;
        if (!is_on_edge && pz->block[0][y][x])
        {
            pz->block[0][y][x] &= ~FLAG_BLOCK_FACE_NZ;
            pz->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    }
    else if (chunk->block[z + 1][y][x])
        chunk->block[z + 1][y][x] &= ~FLAG_BLOCK_FACE_NZ;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    
    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) || !nz;
        if (!is_on_edge && nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            nz->block[CHUNK_DIAMETER - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
            nz->flag |= FLAG_CHUNK_DIRTY;
        }
        else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;
    }
    else if (chunk->block[z - 1][y][x])
        chunk->block[z - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
    else chunk->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;

    chunk->block[z][y][x] |= FLAG_BLOCK_NOT_EMPTY;
    chunk->flag |= FLAG_CHUNK_DIRTY;
}

void block_break(u32 index, u32 x, u32 y, u32 z)
{
    v3u32 coordinates =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };

    Chunk **chunk = &chunk_tab[index];
    Chunk *px = *(chunk + 1);
    Chunk *nx = *(chunk - 1);
    Chunk *py = *(chunk + settings.chunk_buf_diameter);
    Chunk *ny = *(chunk - settings.chunk_buf_diameter);
    Chunk *pz = *(chunk + settings.chunk_buf_layer);
    Chunk *nz = *(chunk - settings.chunk_buf_layer);
    _block_break(*chunk, px, nx, py, ny, pz, nz, coordinates, x, y, z);
}

static void _block_break(Chunk *chunk,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 coordinates, u32 x, u32 y, u32 z)
{
    u8 is_on_edge = 0;
    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;
    z %= CHUNK_DIAMETER;

    if (x == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.x == settings.chunk_buf_diameter - 1) || !px;
        if (!is_on_edge && px->block[z][y][0])
        {
            px->block[z][y][0] |= FLAG_BLOCK_FACE_NX;
            px->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (chunk->block[z][y][x + 1])
        chunk->block[z][y][x + 1] |= FLAG_BLOCK_FACE_NX;

    if (x == 0)
    {
        is_on_edge = (coordinates.x == 0) || !nx;
        if (!is_on_edge && nx->block[z][y][CHUNK_DIAMETER - 1])
        {
            nx->block[z][y][CHUNK_DIAMETER - 1] |= FLAG_BLOCK_FACE_PX;
            nx->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (chunk->block[z][y][x - 1])
        chunk->block[z][y][x - 1] |= FLAG_BLOCK_FACE_PX;

    if (y == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.y == settings.chunk_buf_diameter - 1) || !py;
        if (!is_on_edge && py->block[z][0][x])
        {
            py->block[z][0][x] |= FLAG_BLOCK_FACE_NY;
            py->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (chunk->block[z][y + 1][x])
        chunk->block[z][y + 1][x] |= FLAG_BLOCK_FACE_NY;

    if (y == 0)
    {
        is_on_edge = (coordinates.y == 0) || !ny;
        if (!is_on_edge && ny->block[z][CHUNK_DIAMETER - 1][x])
        {
            ny->block[z][CHUNK_DIAMETER - 1][x] |= FLAG_BLOCK_FACE_PY;
            ny->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (chunk->block[z][y - 1][x])
        chunk->block[z][y - 1][x] |= FLAG_BLOCK_FACE_PY;

    if (z == CHUNK_DIAMETER - 1)
    {
        is_on_edge = (coordinates.z == settings.chunk_buf_diameter - 1) || !pz;
        if (!is_on_edge && pz->block[0][y][x])
        {
            pz->block[0][y][x] |= FLAG_BLOCK_FACE_NZ;
            pz->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (chunk->block[z + 1][y][x])
        chunk->block[z + 1][y][x] |= FLAG_BLOCK_FACE_NZ;

    if (z == 0)
    {
        is_on_edge = (coordinates.z == 0) || !nz;
        if (!is_on_edge && nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            nz->block[CHUNK_DIAMETER - 1][y][x] |= FLAG_BLOCK_FACE_PZ;
            nz->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (chunk->block[z - 1][y][x])
        chunk->block[z - 1][y][x] |= FLAG_BLOCK_FACE_PZ;

    chunk->block[z][y][x] = 0;
    chunk->flag |= FLAG_CHUNK_DIRTY;
}

static v3f32 random(f32 x0, f32 y0, u32 seed)
{
    const u32 w = 8 * sizeof(u64);
    const u32 s = w / 2; 
    u32 a = (i32)x0 + seed + 5000, b = (i32)y0 - seed - 5000;
    a *= 3284157443;
 
    b ^= a << s | a >> (w - s);
    b *= 1911520717;
 
    a ^= b << s | b >> (w - s);
    a *= 2048419325;
    f32 final = a * (PI / ~(~0u >> 1));
    
    return (v3f32){
        sin(final),
            cos(final),
            1.0f,
    };
}

static f32 gradient(f32 x0, f32 y0, f32 x, f32 y)
{
    v3f32 grad = random(x0, y0, SET_TERRAIN_SEED_DEFAULT);
    f32 dx = x0 - x;
    f32 dy = y0 - y;
    return ((dx * grad.x) + (dy * grad.y));
}

static f32 interp(f32 a, f32 b, f32 scale)
{
    return (b - a) * (3.0f - scale * 2.0f) * scale * scale + a;
}

static f32 terrain_noise(v3i32 coordinates, f32 amplitude, f32 frequency)
{
    f32 x = (f32)coordinates.x / frequency;
    f32 y = (f32)coordinates.y / frequency;
    i32 x0 = (i32)floorf(x);
    i32 y0 = (i32)floorf(y);
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

static void chunk_generate(Chunk **chunk, u32 rate)
{
    u32 index = 0;
    v3u32 chunk_tab_coordinates = {0};
    Chunk *px, *nx, *py, *ny, *pz, *nz;
    u32 i = 0;

    if (!*chunk) return;

    index = chunk - chunk_tab;
    chunk_tab_coordinates =
        (v3u32){
            index % settings.chunk_buf_diameter,
            (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
            index / settings.chunk_buf_layer,
        };

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = *(chunk + 1);
    if (chunk_tab_coordinates.x > 0)
        nx = *(chunk - 1);
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = *(chunk + settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.y > 0)
        ny = *(chunk - settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = *(chunk + settings.chunk_buf_layer);
    if (chunk_tab_coordinates.z > 0)
        nz = *(chunk - settings.chunk_buf_layer);

    for (i = (*chunk)->cursor; i < CHUNK_VOLUME && rate; ++i)
    {
        v3u32 pos =
        {
            i % CHUNK_DIAMETER,
            (i / CHUNK_DIAMETER) % CHUNK_DIAMETER,
            i / CHUNK_LAYER,
        };

        v3i32 coordinates =
        {
            pos.x + ((*chunk)->pos.x * CHUNK_DIAMETER),
            pos.y + ((*chunk)->pos.y * CHUNK_DIAMETER),
            pos.z + ((*chunk)->pos.z * CHUNK_DIAMETER),
        };

        f32 elevation = clamp_f32(
            terrain_noise(coordinates, 1.0f, 329.0f) + 0.5f, 0.0f, 1.0f);

        f32 influence = terrain_noise(coordinates, 1.0, 50.0f);

        f32 terrain = 0.0f;
        terrain = terrain_noise(coordinates, 250.0f, 256.0f) * elevation;
        terrain += terrain_noise(coordinates, 30.0f, 40.0f) * elevation;
        terrain += (terrain_noise(coordinates, 10.0f, 10.0f) * influence);
        terrain += expf(-terrain_noise(coordinates, 8.0f, 150.0f));

        if (terrain > coordinates.z)
            _block_place(*chunk, px, nx, py, ny, pz, nz,
                    chunk_tab_coordinates, pos.x, pos.y, pos.z, BLOCK_GRASS);
        --rate;
    }
    (*chunk)->cursor = i;

    if ((*chunk)->cursor == CHUNK_VOLUME &&
            !((*chunk)->flag & FLAG_CHUNK_GENERATED))
        chunk_mesh_init(*chunk);
}

static void chunk_mesh_update(Chunk *chunk)
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
                    index / CHUNK_LAYER,
                };
            *(cursor++) = (u64)*i |
                (((u64)pos.x & 0xf) << 32) |
                (((u64)pos.y & 0xf) << 36) |
                (((u64)pos.z & 0xf) << 40);
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
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

static void chunk_mesh_init(Chunk *chunk)
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
                    index / CHUNK_LAYER,
                };
            *(cursor++) = (u64)*i |
                (((u64)pos.x & 0xf) << 32) |
                (((u64)pos.y & 0xf) << 36) |
                (((u64)pos.z & 0xf) << 40);
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
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

/* TODO: make chunk_serialize() */
/* TODO: add 'version' byte for serialization */
static void chunk_serialize(Chunk *chunk, str *world_name)
{
}

/* TODO: make chunk_deserialize() */
static void chunk_deserialize(Chunk *chunk, str *world_name)
{
}

static void _chunk_buf_push(u32 index, v3i16 player_delta_chunk)
{
    v3u32 coordinates =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };

    Chunk *chunk = &chunk_buf[chunk_buf_cursor];
    Chunk *end = chunk_buf + CHUNKS_MAX[settings.render_distance];
    for (; chunk < end; ++chunk)
        if (!(chunk->flag & FLAG_CHUNK_LOADED))
        {
            *chunk = (Chunk){0};
            chunk->pos =
                (v3i16){
                    player_delta_chunk.x + (coordinates.x - settings.chunk_buf_radius),
                    player_delta_chunk.y + (coordinates.y - settings.chunk_buf_radius),
                    player_delta_chunk.z + (coordinates.z - settings.chunk_buf_radius),
                };
            chunk->id =
                ((u64)(chunk->pos.x & 0xffff) << 0x00) |
                ((u64)(chunk->pos.y & 0xffff) << 0x10) |
                ((u64)(chunk->pos.z & 0xffff) << 0x20);
            chunk->color = COLOR_CHUNK_LOADED;
            chunk->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY;
            chunk_tab[index] = chunk;
            ++chunk_buf_cursor;
            return;
        }
    LOGERROR(FALSE,
            ERR_BUFFER_FULL, "'%s'\n", "chunk_buf Full");
}

static void _chunk_buf_pop(u32 index)
{
    u32 index_popped = chunk_tab[index] - chunk_buf;

    chunk_tab[index]->vbo ?
        glDeleteBuffers(1, &chunk_tab[index]->vbo) : 0;
    chunk_tab[index]->vao ?
        glDeleteVertexArrays(1, &chunk_tab[index]->vao) : 0;

    chunk_tab[index]->flag = 0;
    if (chunk_buf_cursor > index_popped)
        chunk_buf_cursor = index_popped;
    chunk_tab[index] = NULL;
}

/* TODO: grab chunks from disk if previously generated */
void chunk_queue_update(ChunkQueue *q)
{
    if (!MODE_INTERNAL_LOAD_CHUNKS) return;

    u32 i;
    u64 size = q->size;
    u32 cursor = q->cursor;
    u32 rate_chunk = q->rate_chunk;
    u32 rate_block = q->rate_block;
    Chunk ***chunk = CHUNK_ORDER + q->offset;
    Chunk ***end = NULL;

    if (q->count == size)
        goto generate_and_mesh;

    /* ---- push chunk queue ------------------------------------------------ */
    if (q->id == CHUNK_QUEUE_LAST_ID)
        end = CHUNK_ORDER + CHUNKS_MAX[settings.render_distance];
    else end = CHUNK_ORDER + q->offset + size;
    for (; chunk < end && q->count < size; ++chunk)
    {
        if (**chunk && ((**chunk)->flag & FLAG_CHUNK_DIRTY) &&
                !((**chunk)->flag & FLAG_CHUNK_QUEUED) &&
                !(q->queue[cursor]))
        {
            q->queue[cursor] = *chunk;
            (**chunk)->flag |= FLAG_CHUNK_QUEUED;
            ++q->count;
            cursor = (cursor + 1) % size;
        }
    }

    q->cursor = cursor;
    if (!q->count) return;

generate_and_mesh:
    for (i = 0; i < size && rate_chunk; ++i)
        if (q->queue[i])
        {
            if ((*q->queue[i])->flag & FLAG_CHUNK_GENERATED)
                chunk_mesh_update(*q->queue[i]);
            else chunk_generate(q->queue[i], rate_block);
            if (!((*q->queue[i])->flag & FLAG_CHUNK_DIRTY))
            {
                (*q->queue[i])->flag &= ~FLAG_CHUNK_QUEUED;
                q->queue[i] = NULL;
                if (q->count > 0) --q->count;
            }
            --rate_chunk;
        }
}

void chunk_tab_shift(v3i16 player_chunk, v3i16 *player_delta_chunk)
{
    /* TODO: bake 'mirror_index' and 'target_index' into a lookup on startup */
    v3u32 _coordinates = {0};
    u32 mirror_index = 0;
    v3u32 _mirror_index = {0};
    u32 target_index = 0;
    u8 is_on_edge = 0;
    i64 i = 0;
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

    if ((i32)distance_v3i32(
                (v3i32){
                player_chunk.x,
                player_chunk.y,
                player_chunk.z},
                (v3i32){
                player_delta_chunk->x,
                player_delta_chunk->y,
                player_delta_chunk->z}) > RENDER_DISTANCE)
    {
        for (i = 0; i < (i64)CHUNKS_MAX[settings.render_distance]; ++i)
            if (*CHUNK_ORDER[i])
                _chunk_buf_pop(CHUNK_ORDER[i] - chunk_tab);

        *player_delta_chunk = player_chunk;
        return;
    }

    /* this keeps chunk_buf from exploding on a chunk_tab shift */
    chunk_buf_cursor = 0;

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

    for (i = 0; i < settings.chunk_buf_volume; ++i)
        if (chunk_tab[i])
        {
            chunk_tab[i]->flag &= ~FLAG_CHUNK_EDGE;
            chunk_tab[i]->flag |= FLAG_CHUNK_DIRTY;
        }

    /* ---- mark chunks on-edge --------------------------------------------- */
    for (i = 0; i < settings.chunk_buf_volume; ++i)
    {
        if (!chunk_tab[i]) continue;
        chunk_tab[i]->flag &= ~FLAG_CHUNK_QUEUED;

        _coordinates =
        (v3u32){
            i % settings.chunk_buf_diameter,
            (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
            i / settings.chunk_buf_layer,
        };

        _mirror_index =
        (v3u32){
            i + settings.chunk_buf_diameter - 1 - (_coordinates.x * 2),

            (_coordinates.z * settings.chunk_buf_layer) +
                ((settings.chunk_buf_diameter - 1 - _coordinates.y) *
                 settings.chunk_buf_diameter) + _coordinates.x,

            ((settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer) +
                (_coordinates.y * settings.chunk_buf_diameter) + _coordinates.x,
        };

        v3u8 _is_on_edge = {0};

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge =
                    (v3u8){
                        (_coordinates.x == settings.chunk_buf_diameter - 1) ||
                            (chunk_tab[i + 1] == NULL),

                        (_coordinates.y == settings.chunk_buf_diameter - 1) ||
                            (chunk_tab[i + settings.chunk_buf_diameter] == NULL),

                        (_coordinates.z == settings.chunk_buf_diameter - 1) ||
                            (chunk_tab[i + settings.chunk_buf_layer] == NULL),
                    };
                break;

            case 1:
                _is_on_edge =
                    (v3u8){
                        (_coordinates.x == 0) || (chunk_tab[i - 1] == NULL),

                        (_coordinates.y == 0) ||
                            (chunk_tab[i - settings.chunk_buf_diameter] == NULL),

                        (_coordinates.z == 0) ||
                            (chunk_tab[i - settings.chunk_buf_layer] == NULL),
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
            chunk_tab[i]->flag &= ~(FLAG_CHUNK_LOADED | FLAG_CHUNK_RENDER);
            chunk_tab[i]->color = 0;
            if (chunk_tab[mirror_index])
                chunk_tab[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    /* ---- shift chunk_tab ------------------------------------------------- */
    for (i = (INCREMENT == 1) ? 0 : settings.chunk_buf_volume - 1;
            i < settings.chunk_buf_volume && i >= 0; i += INCREMENT)
    {
        if (!chunk_tab[i]) continue;

        _coordinates =
        (v3u32){
            i % settings.chunk_buf_diameter,
            (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
            i / settings.chunk_buf_layer,
        };

        _mirror_index =
        (v3u32){
            i + settings.chunk_buf_diameter - 1 - (_coordinates.x * 2),

            (_coordinates.z * settings.chunk_buf_layer) +
                ((settings.chunk_buf_diameter - 1 - _coordinates.y) *
                 settings.chunk_buf_diameter) + _coordinates.x,

            ((settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer) +
                (_coordinates.y * settings.chunk_buf_diameter) + _coordinates.x,
        };

        v3u32 _target_index = {0};

        switch (INCREMENT)
        {
            case -1:
                _target_index = (v3u32){
                        (_coordinates.x == 0) ? i : i - 1,
                        (_coordinates.y == 0) ? i : i - settings.chunk_buf_diameter,
                        (_coordinates.z == 0) ? i : i - settings.chunk_buf_layer};
                break;

            case 1:
                _target_index = (v3u32){
                        (_coordinates.x == settings.chunk_buf_diameter - 1) ? i : i + 1,

                        (_coordinates.y == settings.chunk_buf_diameter - 1) ?
                            i : i + settings.chunk_buf_diameter,

                        (_coordinates.z == settings.chunk_buf_diameter - 1) ?
                            i : i + settings.chunk_buf_layer};
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
        {
            chunk_tab[target_index]->flag &= ~FLAG_CHUNK_QUEUED;
            chunk_tab[target_index] = NULL;
        }
    }
}

u32 get_target_chunk_index(v3i16 player_chunk, v3i64 player_delta_target)
{
    v3i16 offset =
    {
        (i16)floorf((f64)player_delta_target.x / CHUNK_DIAMETER) -
            player_chunk.x + settings.chunk_buf_radius,

        (i16)floorf((f64)player_delta_target.y / CHUNK_DIAMETER) -
            player_chunk.y + settings.chunk_buf_radius,

        (i16)floorf((f64)player_delta_target.z / CHUNK_DIAMETER) -
            player_chunk.z + settings.chunk_buf_radius,
    };
    return offset.x +
        (offset.y * settings.chunk_buf_diameter) +
        (offset.z * settings.chunk_buf_layer);
}

#ifdef FUCK // TODO: undef FUCK
/* raylib/rmodels.c/DrawLine3D refactored; */
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, v4u8 color)
{
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
    rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}

/* raylib/rmodels.c/DrawCubeWires refactored; */
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

/* raylib/rmodels.c/DrawCubeWires refactored; */
void draw_bounding_box(Vector3 origin, Vector3 scl, Color col)
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
#endif // TODO: undef FUCK
