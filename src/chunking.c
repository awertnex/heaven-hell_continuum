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
#include "h/terrain.h"

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

/* index = 'chunk_tab' index,
 * rate = number of blocks to process per chunk per frame,
 * terrain = the final terrain noise function to execute */
static void chunk_generate(Chunk **chunk, u32 rate, b8 terrain());

/* -- INTERNAL USE ONLY -- */
static void chunk_mesh_init(Chunk *chunk);

/* -- INTERNAL USE ONLY -- */
static void chunk_mesh_update(Chunk *chunk);

/* -- INTERNAL USE ONLY -- */
static void _chunk_serialize(Chunk *chunk, str *world_name);

/* -- INTERNAL USE ONLY -- */
static void _chunk_deserialize(Chunk *chunk, str *world_name);

/* -- INTERNAL USE ONLY --;
 *
 * index = 'chunk_tab' index */
static void _chunk_buf_push(u32 index, v3i16 player_chunk_delta);

/* -- INTERNAL USE ONLY --;
 *
 * index = 'chunk_tab' index */
static void _chunk_buf_pop(u32 index);

/* -- INTERNAL USE ONLY -- */
static void _chunk_queue_update(ChunkQueue *q);

u32 chunking_init(void)
{
    str CHUNK_ORDER_lookup_file_name[PATH_MAX] = {0};
    str CHUNKS_MAX_lookup_file_name[PATH_MAX] = {0};
    u32 *CHUNK_ORDER_lookup_file_contents = NULL;
    u64 *CHUNKS_MAX_lookup_file_contents = NULL;
    u64 i, j, k, file_len;

    u32 render_distance = 0;
    v3i32 center = {0};
    v3i32 coordinates = {0};
    u32 *distance = NULL;
    u32 *index = NULL;
    u64 chunk_buf_diameter = 0;
    u64 chunk_buf_volume = 0;
    u64 chunks_max = 0;

    if (mem_map((void*)&chunk_buf, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk),
                "chunking_init().chunk_buf") != ERR_SUCCESS)
        return *GAME_ERR;

    if (
            mem_map((void*)&chunk_tab, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk*),
                "chunking_init().chunk_tab") != ERR_SUCCESS ||

            mem_map((void*)&CHUNK_ORDER, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_ORDER") != ERR_SUCCESS ||

            mem_map((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().distance") != ERR_SUCCESS ||

            mem_map((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
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
    if (*GAME_ERR != ERR_SUCCESS ||
            CHUNK_ORDER_lookup_file_contents == NULL)
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

        LOGTRACE(FALSE, "%s\n", "Writing CHUNKS_MAX Lookup To File..\n");

        if (write_file(CHUNKS_MAX_lookup_file_name,
                sizeof(u64), SET_RENDER_DISTANCE_MAX + 1,
                &CHUNKS_MAX, "wb", TRUE) != ERR_SUCCESS)
            goto cleanup;
    }

    file_len = get_file_contents(CHUNKS_MAX_lookup_file_name,
            (void*)&CHUNKS_MAX_lookup_file_contents, sizeof(u64), "rb", FALSE);
    if (*GAME_ERR != ERR_SUCCESS ||
            CHUNKS_MAX_lookup_file_contents == NULL)
        goto cleanup;

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

    if (
            mem_map((void*)&CHUNK_QUEUE_1.queue,
                CHUNK_QUEUE_1ST_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE_1.queue") != ERR_SUCCESS ||

            mem_map((void*)&CHUNK_QUEUE_2.queue,
                CHUNK_QUEUE_2ND_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE_2.queue") != ERR_SUCCESS ||

            mem_map((void*)&CHUNK_QUEUE_3.queue,
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

void chunking_update(v3i16 player_chunk, v3i16 *player_chunk_delta)
{
    v3u32 _coordinates, _mirror_index, _target_index;
    u32 mirror_index, target_index;
    u8 is_on_edge;
    v3u8 _is_on_edge;
    i64 i;
    v3i16 DELTA;
    u8 AXIS;
    i8 INCREMENT;
    i32 RENDER_DISTANCE;
    Chunk ***cursor;

    _chunk_queue_update(&CHUNK_QUEUE_1);
    if (!CHUNK_QUEUE_1.count && CHUNK_QUEUE_2.size)
    {
        _chunk_queue_update(&CHUNK_QUEUE_2);
        if (!CHUNK_QUEUE_2.count && CHUNK_QUEUE_3.size)
            _chunk_queue_update(&CHUNK_QUEUE_3);
    }

    if (!(flag & FLAG_MAIN_CHUNK_BUF_DIRTY))
        return;

chunk_tab_shift:

    DELTA =
    (v3i16){
        player_chunk.x - player_chunk_delta->x,
        player_chunk.y - player_chunk_delta->y,
        player_chunk.z - player_chunk_delta->z,
    };

    AXIS =
        DELTA.x > 0 ? STATE_CHUNK_SHIFT_PX :
        DELTA.x < 0 ? STATE_CHUNK_SHIFT_NX :
        DELTA.y > 0 ? STATE_CHUNK_SHIFT_PY :
        DELTA.y < 0 ? STATE_CHUNK_SHIFT_NY :
        DELTA.z > 0 ? STATE_CHUNK_SHIFT_PZ :
        DELTA.z < 0 ? STATE_CHUNK_SHIFT_NZ : 0;

    INCREMENT = (AXIS % 2 == 1) - (AXIS %2 == 0);
    RENDER_DISTANCE = settings.render_distance * settings.render_distance + 2;

    if ((i32)len_v3f32(
                (v3f32){DELTA.x, DELTA.y, DELTA.z}) > RENDER_DISTANCE)
    {
        cursor = CHUNK_ORDER + CHUNKS_MAX[settings.render_distance] - 1;
        for (; cursor >= CHUNK_ORDER; --cursor)
            if (**cursor) _chunk_buf_pop(*cursor - chunk_tab);

        *player_chunk_delta = player_chunk;
        goto chunk_buf_push;
    }

    /* this keeps chunk_buf from exploding on a chunk_tab shift */
    chunk_buf_cursor = 0;

    switch (AXIS)
    {
        case STATE_CHUNK_SHIFT_PX:
        case STATE_CHUNK_SHIFT_NX:
            player_chunk_delta->x += INCREMENT;
            break;

        case STATE_CHUNK_SHIFT_PY:
        case STATE_CHUNK_SHIFT_NY:
            player_chunk_delta->y += INCREMENT;
            break;

        case STATE_CHUNK_SHIFT_PZ:
        case STATE_CHUNK_SHIFT_NZ:
            player_chunk_delta->z += INCREMENT;
            break;

        default:
            goto chunk_buf_push;
    }

    /* ---- mark chunks on-edge --------------------------------------------- */

    for (i = 0; i < settings.chunk_buf_volume; ++i)
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
            i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2,

            _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                settings.chunk_buf_diameter + _coordinates.x,

            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x,
        };

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge =
                    (v3u8){
                        _coordinates.x == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab[i + 1],

                        _coordinates.y == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab[i + settings.chunk_buf_diameter],

                        _coordinates.z == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab[i + settings.chunk_buf_layer],
                    };
                break;

            case 1:
                _is_on_edge =
                    (v3u8){
                        _coordinates.x == 0 || !chunk_tab[i - 1],

                        _coordinates.y == 0 ||
                            !chunk_tab[i - settings.chunk_buf_diameter],

                        _coordinates.z == 0 ||
                            !chunk_tab[i - settings.chunk_buf_layer],
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

    /* ---- shift 'chunk_tab' ----------------------------------------------- */

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
            i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2,

            _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                 settings.chunk_buf_diameter + _coordinates.x,

            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x,
        };

        switch (INCREMENT)
        {
            case -1:
                _target_index = (v3u32){
                        _coordinates.x == 0 ? i : i - 1,
                        _coordinates.y == 0 ? i : i - settings.chunk_buf_diameter,
                        _coordinates.z == 0 ? i : i - settings.chunk_buf_layer};
                break;

            case 1:
                _target_index = (v3u32){
                        _coordinates.x == settings.chunk_buf_diameter - 1 ? i : i + 1,

                        _coordinates.y == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_diameter,

                        _coordinates.z == settings.chunk_buf_diameter - 1 ?
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
        if (chunk_tab[i])
        {
            if (chunk_tab[i]->flag & FLAG_CHUNK_EDGE)
                chunk_tab[target_index] = NULL;

            chunk_tab[i]->flag &= ~(FLAG_CHUNK_EDGE | FLAG_CHUNK_QUEUED);
        }
    }

    if (flag & FLAG_MAIN_CHUNK_BUF_DIRTY)
    {
        if (DELTA.x || DELTA.y || DELTA.z)
            goto chunk_tab_shift;
        else
            flag &= ~FLAG_MAIN_CHUNK_BUF_DIRTY;
    }

chunk_buf_push:

    for (i = 0; i < CHUNKS_MAX[settings.render_distance]; ++i)
    {
        Chunk **p = CHUNK_ORDER[i];
        u32 index = p - chunk_tab;
        if (!*p) _chunk_buf_push(index, *player_chunk_delta);
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
    if (chunk_tab[index]->block[z][y][x] || !block_id) return;
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
    _block_place(*chunk, px, nx, py, ny, pz, nz, coordinates, x, y, z, block_id);
}

static void _block_place(Chunk *chunk,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 coordinates, u32 x, u32 y, u32 z, BlockID block_id)
{
    u8 is_on_edge;

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

    chunk->flag |= FLAG_CHUNK_DIRTY;
}

void block_break(u32 index, u32 x, u32 y, u32 z)
{
    if (!chunk_tab[index]->block[z][y][x]) return;
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
    u8 is_on_edge;
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

static void chunk_generate(Chunk **chunk, u32 rate, b8 terrain())
{
    u32 index;
    v3u32 chunk_tab_coordinates;
    Chunk *ch = NULL,
          *px = NULL, *nx = NULL,
          *py = NULL, *ny = NULL,
          *pz = NULL, *nz = NULL;
    u32 x, y, z;

    if (!chunk || !*chunk || !terrain) return;
    ch = *chunk;

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

    x = ch->cursor % CHUNK_DIAMETER;
    y = (ch->cursor / CHUNK_DIAMETER) % CHUNK_DIAMETER;
    z = ch->cursor / CHUNK_LAYER;
    for (; z < CHUNK_DIAMETER && rate; ++z)
    {
        for (; y < CHUNK_DIAMETER && rate; ++y)
        {
            for (; x < CHUNK_DIAMETER && rate; ++x)
            {
                v3i32 coordinates =
                {
                    x + ch->pos.x * CHUNK_DIAMETER,
                    y + ch->pos.y * CHUNK_DIAMETER,
                    z + ch->pos.z * CHUNK_DIAMETER,
                };

                if (terrain(coordinates))
                {
                    _block_place(ch, px, nx, py, ny, pz, nz,
                            chunk_tab_coordinates, x, y, z, BLOCK_GRASS);

                    if (z == 0)
                    {
                        if (nz && nz->block[CHUNK_DIAMETER - 1][y][x])
                        {
                            SET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x], BLOCK_DIRT);
                            nz->flag |= FLAG_CHUNK_DIRTY;
                        }
                    }
                    else if (ch->block[z - 1][y][x])
                    {
                        SET_BLOCK_ID(ch->block[z - 1][y][x], BLOCK_DIRT);
                        ch->flag |= FLAG_CHUNK_DIRTY;
                    }

                    if (z == CHUNK_DIAMETER - 1 && pz && pz->block[0][y][x])
                        SET_BLOCK_ID(ch->block[z][y][x], BLOCK_DIRT);
                }
                --rate;
            }
            x = 0;
        }
        y = 0;
    }
    ch->cursor = x + y * CHUNK_DIAMETER + z * CHUNK_LAYER;

    if (ch->cursor == CHUNK_VOLUME && !(ch->flag & FLAG_CHUNK_GENERATED))
        chunk_mesh_init(ch);
}

static void chunk_mesh_init(Chunk *chunk)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *i = (u32*)chunk->block;
    u32 *end = i + CHUNK_VOLUME;
    u64 index;
    v3u64 pos;
    b8 should_render = FALSE;

    for (; i < end; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            index = CHUNK_VOLUME - (end - i);
            pos =
                (v3u64){
                    index % CHUNK_DIAMETER,
                    (index / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    index / CHUNK_LAYER,
                };
            *(cursor++) = *i |
                (pos.x & 0xf) << SHIFT_BLOCK_X |
                (pos.y & 0xf) << SHIFT_BLOCK_Y |
                (pos.z & 0xf) << SHIFT_BLOCK_Z;
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    chunk->vbo_len = len;

    if (!chunk->vao) glGenVertexArrays(1, &chunk->vao);
    if (!chunk->vbo) glGenBuffers(1, &chunk->vbo);

    glBindVertexArray(chunk->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64), buf, GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)sizeof(u32));
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

static void chunk_mesh_update(Chunk *chunk)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    if (!chunk->vbo || !chunk->vao) return;
    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *i = (u32*)chunk->block;
    u32 *end = i + CHUNK_VOLUME;
    u64 index;
    v3u64 pos;
    b8 should_render = FALSE;

    for (; i < end; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            index = CHUNK_VOLUME - (end - i);
            pos =
                (v3u64){
                    index % CHUNK_DIAMETER,
                    (index / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    index / CHUNK_LAYER,
                };
            *(cursor++) = *i |
                (pos.x & 0xf) << SHIFT_BLOCK_X |
                (pos.y & 0xf) << SHIFT_BLOCK_Y |
                (pos.z & 0xf) << SHIFT_BLOCK_Z;
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    chunk->vbo_len = len;

    glBindVertexArray(chunk->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64), buf, GL_DYNAMIC_DRAW);

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

/* TODO: make chunk_serialize() */
/* TODO: add 'version' byte for serialization */
static void chunk_serialize(Chunk *chunk, str *world_name)
{
}

/* TODO: make chunk_deserialize() */
static void chunk_deserialize(Chunk *chunk, str *world_name)
{
}

static void _chunk_buf_push(u32 index, v3i16 player_chunk_delta)
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
            if (chunk->vbo) glDeleteBuffers(1, &chunk->vbo);
            if (chunk->vao) glDeleteVertexArrays(1, &chunk->vao);
            *chunk = (Chunk){0};

            chunk->pos =
                (v3i16){
                    player_chunk_delta.x + coordinates.x - settings.chunk_buf_radius,
                    player_chunk_delta.y + coordinates.y - settings.chunk_buf_radius,
                    player_chunk_delta.z + coordinates.z - settings.chunk_buf_radius,
                };

            chunk->id =
                (chunk->pos.x & 0xffff) << 0x00 |
                (chunk->pos.y & 0xffff) << 0x10 |
                (chunk->pos.z & 0xffff) << 0x20;

            chunk->color = COLOR_CHUNK_LOADED;
            chunk->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY;
            chunk_tab[index] = chunk;
            ++chunk_buf_cursor;
            return;
        }

    LOGERROR(FALSE, ERR_BUFFER_FULL, "'%s'\n", "chunk_buf Full");
}

static void _chunk_buf_pop(u32 index)
{
    u32 index_popped = chunk_tab[index] - chunk_buf;

    if (chunk_tab[index]->vbo)
    {
        glDeleteBuffers(1, &chunk_tab[index]->vbo);
        chunk_tab[index]->vbo = 0;
    }
    if (chunk_tab[index]->vao)
    {
        glDeleteVertexArrays(1, &chunk_tab[index]->vao);
        chunk_tab[index]->vao = 0;
    }

    chunk_tab[index]->flag = 0;
    if (chunk_buf_cursor > index_popped)
        chunk_buf_cursor = index_popped;
    chunk_tab[index] = NULL;
}

/* TODO: grab chunks from disk if previously generated */
static void _chunk_queue_update(ChunkQueue *q)
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
            else chunk_generate(q->queue[i], rate_block, terrain_land);
            if (!((*q->queue[i])->flag & FLAG_CHUNK_DIRTY))
            {
                (*q->queue[i])->flag &= ~FLAG_CHUNK_QUEUED;
                q->queue[i] = NULL;
                if (q->count > 0) --q->count;
            }
            --rate_chunk;
        }
}

u32 get_target_chunk_index(v3i16 player_chunk, v3i64 player_target_delta)
{
    v3i16 offset =
    {
        (i16)floorf((f64)player_target_delta.x / CHUNK_DIAMETER) -
            player_chunk.x + settings.chunk_buf_radius,

        (i16)floorf((f64)player_target_delta.y / CHUNK_DIAMETER) -
            player_chunk.y + settings.chunk_buf_radius,

        (i16)floorf((f64)player_target_delta.z / CHUNK_DIAMETER) -
            player_chunk.z + settings.chunk_buf_radius,
    };
    return
        offset.x +
        offset.y * settings.chunk_buf_diameter +
        offset.z * settings.chunk_buf_layer;
}
