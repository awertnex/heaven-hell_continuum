#include "deps/fossil/common/limits.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/string/string.h"

#include "deps/fossil/h/dir.h"

#include "../settings/settings.h"

#include "../h/assets.h"
#include "../h/config_internal.h"
#include "../h/common.h"
#include "../h/diagnostics.h"
#include "../h/main.h"
#include "../terrain/terrain.h"
#include "../h/world.h"

#include "chunk_scheduler.h"
#include "chunking.h"
#include "chunking_debug_tools.h"
#include "chunking_internal.h"

#include <stdio.h>
#include <math.h>

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief memory arena used to store all chunk buffers and data.
 */
static fsl_mem_arena memory_arena_chunking_internal = {0};

static hhc_chunk_buffer chunk_buf = {0};
hhc_chunk_table chunk_tab = {0};
hhc_chunk_order chunk_order = {0};
hhc_chunk_scheduler chunk_sched[CHUNK_SCHEDULERS_MAX] = {0};

/* ---- section: implementation --------------------------------------------- */

u32 chunking_init(v3i32 *player_chunk_delta)
{
    if (core.flag.chunks_initialized)
        return FSL_ERR_SUCCESS;

    if (chunks_max_init_internal() != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    if (settings.flag.render_distance_dirty)
    {
        settings.flag.render_distance_dirty = FALSE;
        chunk_order.chunks_max = chunk_order.len[settings.render_distance];
    }

    if (fsl_mem_arena_init(&memory_arena_chunking_internal,
                "chunking_init().memory_arena_chunking_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_order.handle,
                chunk_order.len[SET_RENDER_DISTANCE_MAX] * sizeof(u32),
                "chunking_init().chunk_order.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_tab.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(hhc_chunk*),
                "chunking_init().chunk_tab.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_buf.handle,
                chunk_order.len[SET_RENDER_DISTANCE_MAX] * sizeof(hhc_chunk),
                "chunking_init().chunk_buf.handle") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (chunk_debug_init_internal(chunk_order.len[SET_RENDER_DISTANCE_MAX]) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (chunk_order_init_internal() != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- init chunk parsing priority schedulers -------------------------- */

    if (chunk_scheduler_init_internal(&chunk_sched[0], CHUNK_SCHEDULER_ID_1ST,
                0,
                CHUNK_SCHEDULER_RADIUS_1ST,
                CHUNK_SCHEDULER_BUDGET_PRIORITY_HIGH) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (chunk_scheduler_init_internal(&chunk_sched[1], CHUNK_SCHEDULER_ID_2ND,
                CHUNK_SCHEDULER_RADIUS_1ST,
                CHUNK_SCHEDULER_RADIUS_2ND,
                CHUNK_SCHEDULER_BUDGET_PRIORITY_MID) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (chunk_scheduler_init_internal(&chunk_sched[2], CHUNK_SCHEDULER_ID_3RD,
                CHUNK_SCHEDULER_RADIUS_2ND,
                CHUNK_SCHEDULER_RADIUS_3RD,
                CHUNK_SCHEDULER_BUDGET_PRIORITY_LOW) != FSL_ERR_SUCCESS)
        goto cleanup;

    chunk_order.p = fsl_mem_handle_get(chunk_order.handle);
    chunk_tab.p = fsl_mem_handle_get(chunk_tab.handle);
    chunk_buf.p = fsl_mem_handle_get(chunk_buf.handle);
    chunk_sched[0].p = fsl_mem_handle_get(chunk_sched[0].schedule);
    chunk_sched[1].p = fsl_mem_handle_get(chunk_sched[1].schedule);
    chunk_sched[2].p = fsl_mem_handle_get(chunk_sched[2].schedule);

    core.flag.chunks_initialized = TRUE;

    chunk_buf_update_internal(player_chunk_delta);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    chunking_free();
    return *GAME_ERR;
}

u32 chunks_max_init_internal(void)
{
    str path[FSL_PATH_CAP] = {0};
    u32 *file_contents = NULL;
    u64 file_len = 0;
    u32 i = 0;
    u32 j = 0;
    u32 chunk_sphere_radius = 0;
    v3i32 center = {0};
    v3i32 coordinates = {0};
    u32 chunk_buf_diameter = 0;
    u32 chunk_buf_volume = 0;
    u32 chunk_count = 0;

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNKS_MAX);
    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
        goto load_from_disk;

    for (i = 0; i < SET_RENDER_DISTANCE_MAX + 1; ++i)
    {
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                fsl_logger_stringf("Building `chunks_max` Look-up, Progress [radius: %u/%u][%u%%]..\n",
                    i, SET_RENDER_DISTANCE_MAX + 1, (i * 100) / (SET_RENDER_DISTANCE_MAX + 1)));
        chunk_buf_diameter = i * 2 + 1;
        chunk_buf_volume =
            chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
        chunk_count = 0;
        chunk_sphere_radius = chunk_sphere_radius_get_internal(i);
        center.x = i;
        center.y = i;
        center.z = i;

        for (j = 0; j < chunk_buf_volume; ++j)
        {
            coordinates.x = j % chunk_buf_diameter;
            coordinates.y = (j / chunk_buf_diameter) % chunk_buf_diameter;
            coordinates.z = j / (chunk_buf_diameter * chunk_buf_diameter);
            if (fsl_distance_v3i32(coordinates, center) < chunk_sphere_radius)
                ++chunk_count;
        }
        chunk_order.len[i] = chunk_count;
    }

    if (fsl_write_file(path, (SET_RENDER_DISTANCE_MAX + 1) * sizeof(u32),
                chunk_order.len, TRUE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;

    LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("`chunks_max` Look-up '%s' Exported\n", path));

load_from_disk:

    file_len = fsl_get_file_contents(path, (void*)&file_contents, FALSE);
    if (*GAME_ERR != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < SET_RENDER_DISTANCE_MAX + 1; ++i)
        chunk_order.len[i] = file_contents[i];
    fsl_mem_free((void*)&file_contents, file_len, "chunks_max_init_internal().file_contents");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_mem_free((void*)&file_contents, file_len, "chunks_max_init_internal().file_contents");
    chunking_free();
    return *GAME_ERR;
}

u32 chunk_order_init_internal(void)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_ORDER);
    chunk_order.p = fsl_mem_handle_get(chunk_order.handle);
    chunk_tab.p = fsl_mem_handle_get(chunk_tab.handle);

    if (!chunk_tab.p)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize `chunk_order` Look-up, `chunk_tab.p` `NULL`\n");
        return *GAME_ERR;
    }

    if (fsl_is_file_exists(path, FALSE) != FSL_ERR_SUCCESS)
    {
        if (chunk_order_build_internal() != FSL_ERR_SUCCESS)
            return *GAME_ERR;
    }

    if (chunk_order_load_internal(settings.render_distance) != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    return *GAME_ERR;
}

u32 chunk_order_build_internal(void)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    u32 *bucket_buf = NULL;
    u32 bucket_len = 0;
    u32 *distance_buf = NULL;
    u32 distance_cache = 0;
    v3i8 *pos_buf = NULL;
    v3i8 *data_buf = NULL;

    v3i32 center =
    {
        CHUNK_BUF_RADIUS_MAX,
        CHUNK_BUF_RADIUS_MAX,
        CHUNK_BUF_RADIUS_MAX
    };
    v3i32 pos = {0};
    u32 radius = {0};
    u32 chunk_count = 0;
    u32 i = 0;

    radius = chunk_sphere_radius_get_internal(SET_RENDER_DISTANCE_MAX);

    bucket_len = fsl_distance_v3i32(pos, center);

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_ORDER);

    if (fsl_mem_map((void*)&bucket_buf, bucket_len * sizeof(u32),
                "chunk_order_build_internal().bucket_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mem_map((void*)&distance_buf, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunk_order_build_internal().distance_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mem_map((void*)&pos_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
                "chunk_order_build_internal().pos_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mem_map((void*)&data_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
                "chunk_order_build_internal().data_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- calculate distances --------------------------------------------- */

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "Building `chunk_order` Look-up..\n");

    for (pos.z = 0; pos.z < CHUNK_BUF_DIAMETER_MAX; ++pos.z)
    {
        for (pos.y = 0; pos.y < CHUNK_BUF_DIAMETER_MAX; ++pos.y)
        {
            for (pos.x = 0; pos.x < CHUNK_BUF_DIAMETER_MAX; ++pos.x)
            {
                distance_cache = fsl_distance_v3i32(pos, center);
                if (distance_cache < radius)
                {
                    ++bucket_buf[distance_cache];
                    distance_buf[chunk_count] = distance_cache;
                    pos_buf[chunk_count].x = pos.x - CHUNK_BUF_RADIUS_MAX;
                    pos_buf[chunk_count].y = pos.y - CHUNK_BUF_RADIUS_MAX;
                    pos_buf[chunk_count].z = pos.z - CHUNK_BUF_RADIUS_MAX;
                    ++chunk_count;
                }
            }
        }
    }

    for (i = 1; i < bucket_len; ++i)
        bucket_buf[i] += bucket_buf[i - 1];

    for (i = 0; i < chunk_count; ++i)
        data_buf[--bucket_buf[distance_buf[i]]] = pos_buf[i];

    /* ---- export lookup --------------------------------------------------- */

    if (fsl_write_file(path, chunk_count * sizeof(v3i8),
                data_buf, TRUE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;

    LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("`chunk_order` Look-up '%s' Exported\n", path));

    fsl_mem_unmap((void*)&bucket_buf, bucket_len * sizeof(u32),
            "chunk_order_build_internal().bucket_buf");
    fsl_mem_unmap((void*)&distance_buf, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunk_order_build_internal().distance_buf");
    fsl_mem_unmap((void*)&pos_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
            "chunk_order_build_internal().pos_buf");
    fsl_mem_unmap((void*)&data_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
                "chunk_order_build_internal().data_buf");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_mem_unmap((void*)&bucket_buf, bucket_len * sizeof(u32),
            "chunk_order_build_internal().bucket_buf");
    fsl_mem_unmap((void*)&distance_buf, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunk_order_build_internal().distance_buf");
    fsl_mem_unmap((void*)&pos_buf, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunk_order_build_internal().pos_buf");
    fsl_mem_unmap((void*)&data_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
                "chunk_order_build_internal().data_buf");
    fsl_mem_arena_pop(&chunk_order.handle, "chunk_order_build_internal().chunk_order.handle");
    chunk_order.p = NULL;
    return *GAME_ERR;
}

u32 chunk_order_load_internal(u32 render_distance)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    v3i8 *file_contents = NULL;
    u64 file_len = 0;
    u32 radius = render_distance;
    u32 diameter = radius * 2 + 1;
    u32 layer = diameter * diameter;
    u32 i = 0;
    u32 index = 0;

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_ORDER);
    file_len = fsl_get_file_contents(path, (void*)&file_contents, FALSE);
    if (file_contents == NULL)
        goto cleanup;

    for (i = 0; i < chunk_order.chunks_max; ++i)
    {
        index =
            file_contents[i].x + radius +
            (file_contents[i].y + radius) * diameter +
            (file_contents[i].z + radius) * layer;
        chunk_order.p[i] = index;
    }

    fsl_mem_free((void*)&file_contents, file_len,
            "chunk_order_load_internal().file_contents");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_mem_free((void*)&file_contents, file_len,
            "chunk_order_load_internal().file_contents");
    return *GAME_ERR;
}

void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta, block_hit hit)
{
    hhc_chunk **cursor = NULL;
    v3u32 _coordinates = {0};
    v3u32 _mirror_index = {0};
    v3u32 _target_index = {0};
    u32 mirror_index = 0;
    u32 target_index = 0;
    u8 is_on_edge = 0;
    v3u8 _is_on_edge = {0};
    u32 i = 0;
    u32 end = 0;
    v3i32 DELTA = {0};
    u8 AXIS = 0;
    i8 INCREMENT = 0;
    v3f32 DISTANCE = {0};
    i32 RENDER_DISTANCE = 0;
    v3u32 chunk_tab_coordinates = {0};

    if (settings.flag.render_distance_dirty)
    {
        settings.flag.render_distance_dirty = FALSE;
        chunk_order.chunks_max = chunk_order.len[settings.render_distance];
        chunk_order_load_internal(settings.render_distance);
    }

    chunk_tab.index = get_chunk_index(player_chunk, hit.pos);

    chunk_scheduler_update_internal(&chunk_sched[0],
            chunk_order.len[chunk_sched[0].radius_start],
            chunk_order.len[chunk_sched[0].radius_end],
            TRUE, TRUE);

    chunk_scheduler_update_internal(&chunk_sched[1],
            chunk_order.len[chunk_sched[1].radius_start],
            chunk_order.len[chunk_sched[1].radius_end],
            TRUE, !chunk_sched[0].count && chunk_sched[1].radius_end);

    chunk_scheduler_update_internal(&chunk_sched[2],
            chunk_order.len[chunk_sched[2].radius_start],
            chunk_order.chunks_max,
            TRUE, !chunk_sched[1].count && chunk_sched[2].radius_end);

    DELTA.x = player_chunk.x - player_chunk_delta->x;
    DELTA.y = player_chunk.y - player_chunk_delta->y;
    DELTA.z = player_chunk.z - player_chunk_delta->z;

    if (!(DELTA.x || DELTA.y || DELTA.z))
        return;

chunk_tab_shift:

    AXIS =
        DELTA.x > 0 ? STATE_CHUNK_SHIFT_PX :
        DELTA.x < 0 ? STATE_CHUNK_SHIFT_NX :
        DELTA.y > 0 ? STATE_CHUNK_SHIFT_PY :
        DELTA.y < 0 ? STATE_CHUNK_SHIFT_NY :
        DELTA.z > 0 ? STATE_CHUNK_SHIFT_PZ :
        DELTA.z < 0 ? STATE_CHUNK_SHIFT_NZ : 0;

    INCREMENT = (AXIS % 2 == 1) - (AXIS %2 == 0);
    DISTANCE.x = DELTA.x;
    DISTANCE.y = DELTA.y;
    DISTANCE.z = DELTA.z;
    RENDER_DISTANCE = chunk_sphere_radius_get_internal(settings.render_distance);

    if ((i32)fsl_len_v3f32(DISTANCE) > RENDER_DISTANCE)
    {
        for (i = 0; i < chunk_order.chunks_max; ++i)
        {
            cursor = &chunk_tab.p[chunk_order.p[i]];
            if (*cursor)
                chunk_buf_pop_internal(*cursor);
        }

        *player_chunk_delta = player_chunk;
        goto chunk_buf_push;
    }

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

    end = settings.chunk_buf_volume;
    for (i = 0; i < end; ++i)
    {
        if (!chunk_tab.p[i])
            continue;

        _coordinates.x = i % settings.chunk_buf_diameter;
        _coordinates.y = (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
        _coordinates.z = i / settings.chunk_buf_layer;

        _mirror_index.x = i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2;
        _mirror_index.y = _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                settings.chunk_buf_diameter + _coordinates.x;
        _mirror_index.z =
            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x;

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge.x = _coordinates.x == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab.p[i + 1];
                _is_on_edge.y = _coordinates.y == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab.p[i + settings.chunk_buf_diameter];
                _is_on_edge.z = _coordinates.z == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab.p[i + settings.chunk_buf_layer];
                break;

            case 1:
                _is_on_edge.x = _coordinates.x == 0 || !chunk_tab.p[i - 1];
                _is_on_edge.y = _coordinates.y == 0 || !chunk_tab.p[i - settings.chunk_buf_diameter];
                _is_on_edge.z = _coordinates.z == 0 || !chunk_tab.p[i - settings.chunk_buf_layer];
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
            chunk_tab.p[i]->flag &= ~(FLAG_CHUNK_LOADED | FLAG_CHUNK_VISIBLE);
            chunk_tab.p[i]->color = 0;
            if (chunk_tab.p[mirror_index])
                chunk_tab.p[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    /* ---- shift `chunk_tab` ----------------------------------------------- */

    i = (INCREMENT == 1) ? 0 : settings.chunk_buf_volume - 1;
    end = settings.chunk_buf_volume;
    for (; i < end; i += INCREMENT)
    {
        if (!chunk_tab.p[i])
            continue;

        _coordinates.x = i % settings.chunk_buf_diameter;
        _coordinates.y = (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
        _coordinates.z = i / settings.chunk_buf_layer;

        _mirror_index.x = i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2;
        _mirror_index.y =
            _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                 settings.chunk_buf_diameter + _coordinates.x;
        _mirror_index.z =
            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x;

        switch (INCREMENT)
        {
            case -1:
                _target_index.x = _coordinates.x == 0 ? i : i - 1;
                _target_index.y = _coordinates.y == 0 ? i : i - settings.chunk_buf_diameter;
                _target_index.z = _coordinates.z == 0 ? i : i - settings.chunk_buf_layer;
                break;

            case 1:
                _target_index.x = _coordinates.x == settings.chunk_buf_diameter - 1 ? i : i + 1;
                _target_index.y = _coordinates.y == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_diameter;
                _target_index.z = _coordinates.z == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_layer;
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

        if (chunk_tab.p[target_index])
            chunk_debug_chunk_gizmo_write_internal(chunk_tab.p[target_index]);

        chunk_tab.p[i] = chunk_tab.p[target_index];
        if (chunk_tab.p[i])
        {
            chunk_tab.p[i]->cti = i;

            chunk_tab_coordinates.x = i % settings.chunk_buf_diameter;
            chunk_tab_coordinates.y = (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
            chunk_tab_coordinates.z = i / settings.chunk_buf_layer;
            chunk_pos_set_internal(chunk_tab.p[i], *player_chunk_delta, chunk_tab_coordinates);

            if (chunk_tab.p[i]->flag & FLAG_CHUNK_EDGE)
            {
                chunk_tab.p[i]->flag &= ~FLAG_CHUNK_EDGE;
                chunk_tab.p[target_index] = NULL;
            }

            chunk_debug_chunk_gizmo_write_internal(chunk_tab.p[i]);
        }
    }

    if (DELTA.x || DELTA.y || DELTA.z)
    {
        DELTA.x = player_chunk.x - player_chunk_delta->x;
        DELTA.y = player_chunk.y - player_chunk_delta->y;
        DELTA.z = player_chunk.z - player_chunk_delta->z;
        goto chunk_tab_shift;
    }

chunk_buf_push:

    chunk_buf_update_internal(player_chunk_delta);
}

void chunking_free(void)
{
    if (chunk_tab.p)
    {
        u32 i = 0;
        for (; i < settings.chunk_buf_volume; ++i)
            if (chunk_tab.p[i])
                chunk_buf_pop_internal(chunk_tab.p[i]);
    }

    chunk_debug_free_internal();

    fsl_mem_arena_free(&memory_arena_chunking_internal,
            "chunking_free().memory_arena_chunking_internal");
}

u32 block_get_faces_internal(const hhc_chunk *chunk,
        const hhc_chunk *px, const hhc_chunk *nx,
        const hhc_chunk *py, const hhc_chunk *ny,
        const hhc_chunk *pz, const hhc_chunk *nz,
        i32 x, i32 y, i32 z)
{
    u32 block = GET_BLOCK_ID(chunk->block[z][y][x]);

    if (x == CHUNK_DIAMETER - 1)
    {
        if (!px || !px->block[z][y][0])
            block |= FLAG_BLOCK_FACE_PX;
    }
    else if (!chunk->block[z][y][x + 1])
        block |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        if (!nx || !nx->block[z][y][CHUNK_DIAMETER - 1])
            block |= FLAG_BLOCK_FACE_NX;
    }
    else if (!chunk->block[z][y][x - 1])
        block |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        if (!py || !py->block[z][0][x])
            block |= FLAG_BLOCK_FACE_PY;
    }
    else if (!chunk->block[z][y + 1][x])
        block |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        if (!ny || !ny->block[z][CHUNK_DIAMETER - 1][x])
            block |= FLAG_BLOCK_FACE_NY;
    }
    else if (!chunk->block[z][y - 1][x])
        block |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        if (!pz || !pz->block[0][y][x])
            block |= FLAG_BLOCK_FACE_PZ;
    }
    else if (!chunk->block[z + 1][y][x])
        block |= FLAG_BLOCK_FACE_PZ;

    if (z == 0)
    {
        if (!nz || !nz->block[CHUNK_DIAMETER - 1][y][x])
            block |= FLAG_BLOCK_FACE_NZ;
    }
    else if (!chunk->block[z - 1][y][x])
        block |= FLAG_BLOCK_FACE_NZ;

    return block;
}

block_hit block_hit_get(v3f64 origin, f64 start_x, f64 start_y, f64 start_z,
        f64 end_x, f64 end_y, f64 end_z, f64 distance_max)
{
    v3i64 world_volume_min = {-WORLD_DIAMETER, -WORLD_DIAMETER, -WORLD_DIAMETER_VERTICAL};
    v3i64 world_volume_max = {WORLD_DIAMETER, WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL};
    block_hit hit = {0};
    v3f64 delta = {0};
    v3f64 direction = {0};
    v3f64 distance = {0};
    v3i32 step = {1, 1, 1};
    hhc_chunk *chunk = NULL;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    i32 chx = 0;
    i32 chy = 0;
    i32 chz = 0;

    direction.x = end_x - start_x;
    direction.y = end_y - start_y;
    direction.z = end_z - start_z;
    direction = fsl_normalize_v3f64(direction);
    delta.x = direction.x == 0.0 ? INFINITY : fabs(1.0 / direction.x);
    delta.y = direction.y == 0.0 ? INFINITY : fabs(1.0 / direction.y);
    delta.z = direction.z == 0.0 ? INFINITY : fabs(1.0 / direction.z);

    hit.pos.x = (i64)floor(start_x);
    hit.pos.y = (i64)floor(start_y);
    hit.pos.z = (i64)floor(start_z);

    if (direction.x < 0.0f)
    {
        distance.x = (start_x - hit.pos.x) * delta.x;
        step.x = -1;
    }
    else distance.x = (hit.pos.x + 1.0 - start_x) * delta.x;

    if (direction.y < 0.0f)
    {
        distance.y = (start_y - hit.pos.y) * delta.y;
        step.y = -1;
    }
    else distance.y = (hit.pos.y + 1.0 - start_y) * delta.y;

    if (direction.z < 0.0f)
    {
        distance.z = (start_z - hit.pos.z) * delta.z;
        step.z = -1;
    }
    else distance.z = (hit.pos.z + 1.0 - start_z) * delta.z;

    while (fsl_min_v3f64(distance) < distance_max)
    {
        switch (fsl_min_axis_v3f64(distance))
        {
            case 1:
                hit.pos.x += step.x;
                distance.x += delta.x;
                hit.normal.x = -step.x;
                hit.normal.y = 0.0;
                hit.normal.z = 0.0;
                break;

            case 2:
                hit.pos.y += step.y;
                distance.y += delta.y;
                hit.normal.x = 0.0;
                hit.normal.y = -step.y;
                hit.normal.z = 0.0;
                break;

            case 3:
                hit.pos.z += step.z;
                distance.z += delta.z;
                hit.normal.x = 0.0;
                hit.normal.y = 0.0;
                hit.normal.z = -step.z;
                break;
        }

        x = hit.pos.x - (i32)floorf((f32)origin.x / CHUNK_DIAMETER) * CHUNK_DIAMETER;
        y = hit.pos.y - (i32)floorf((f32)origin.y / CHUNK_DIAMETER) * CHUNK_DIAMETER;
        z = hit.pos.z - (i32)floorf((f32)origin.z / CHUNK_DIAMETER) * CHUNK_DIAMETER;

        chx = floorf((f32)x / CHUNK_DIAMETER);
        chy = floorf((f32)y / CHUNK_DIAMETER);
        chz = floorf((f32)z / CHUNK_DIAMETER);
        chunk = chunk_tab.p[settings.chunk_tab_center +
            chx +
            chy * settings.chunk_buf_diameter +
            chz * settings.chunk_buf_layer];
        if (!chunk || !(chunk->flag & FLAG_CHUNK_GENERATED))
            continue;

        x = fsl_mod_i32(x, CHUNK_DIAMETER);
        y = fsl_mod_i32(y, CHUNK_DIAMETER);
        z = fsl_mod_i32(z, CHUNK_DIAMETER);
        if (!chunk->block[z][y][x])
            continue;

        if (!fsl_is_in_volume_i64(hit.pos, world_volume_min, world_volume_max))
            break;

        hit.block = &chunk->block[z][y][x];
        hit.hit = TRUE;
        break;
    }

    return hit;
}

void block_place(block_hit hit, enum block_id block_id)
{
    u32 index = chunk_tab.index;
    hhc_chunk **chunk = NULL;
    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    if (!hit.hit || (hit.normal.x == 0.0f && hit.normal.y == 0.0f && hit.normal.z == 0.0f))
        return;

    /* canonicalize block position */
    hit.pos.x -= chunk_tab.p[index]->pos.x * CHUNK_DIAMETER;
    hit.pos.y -= chunk_tab.p[index]->pos.y * CHUNK_DIAMETER;
    hit.pos.z -= chunk_tab.p[index]->pos.z * CHUNK_DIAMETER;

    /* get block position at normal direction */
    hit.pos.x += (i32)hit.normal.x;
    hit.pos.y += (i32)hit.normal.y;
    hit.pos.z += (i32)hit.normal.z;

    /* get the chunk the new block index is in and make sure it's within bounds */
    index += (i32)floorf((f32)hit.pos.x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)hit.pos.y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)hit.pos.z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume)
        return;

    hit.pos.x = fsl_mod_i32(hit.pos.x, CHUNK_DIAMETER);
    hit.pos.y = fsl_mod_i32(hit.pos.y, CHUNK_DIAMETER);
    hit.pos.z = fsl_mod_i32(hit.pos.z, CHUNK_DIAMETER);

    chunk = &chunk_tab.p[index];
    if ((*chunk)->block[hit.pos.z][hit.pos.y][hit.pos.x] || !block_id)
        return;

    chunk_tab_coordinates.x = index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = index / settings.chunk_buf_layer;

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

    block_add_internal(*chunk, px, nx, py, ny, pz, nz, hit.pos.x, hit.pos.y, hit.pos.z, block_id);
}

void block_add_internal(hhc_chunk *chunk,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z, enum block_id block_id)
{
    chunk->flag |= FLAG_CHUNK_DIRTY;
    SET_BLOCK_ID(chunk->block[z][y][x], block_id);
    chunk->block[z][y][x] |= 63 << SHIFT_BLOCK_LIGHT;

    if (x == CHUNK_DIAMETER - 1 && px && px->block[z][y][0])
        px->flag |= FLAG_CHUNK_DIRTY;

    if (x == 0 && nx && nx->block[z][y][CHUNK_DIAMETER - 1])
        nx->flag |= FLAG_CHUNK_DIRTY;

    if (y == CHUNK_DIAMETER - 1 && py && py->block[z][0][x])
        py->flag |= FLAG_CHUNK_DIRTY;

    if (y == 0 && ny && ny->block[z][CHUNK_DIAMETER - 1][x])
        ny->flag |= FLAG_CHUNK_DIRTY;

    if (z == CHUNK_DIAMETER - 1 && pz && pz->block[0][y][x])
        pz->flag |= FLAG_CHUNK_DIRTY;

    if (z == 0 && nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        nz->flag |= FLAG_CHUNK_DIRTY;

    block_evaluate_internal(chunk, px, nx, py, ny, pz, nz, x, y, z, block_id);
}

void block_break(block_hit hit)
{
    u32 index = chunk_tab.index;
    hhc_chunk **chunk = NULL;
    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    if (!hit.hit)
        return;

    /* canonicalize block position */
    hit.pos.x -= chunk_tab.p[index]->pos.x * CHUNK_DIAMETER;
    hit.pos.y -= chunk_tab.p[index]->pos.y * CHUNK_DIAMETER;
    hit.pos.z -= chunk_tab.p[index]->pos.z * CHUNK_DIAMETER;

    /* get the chunk the new block index is in and make sure it's within bounds */
    index += (i32)floorf((f32)hit.pos.x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)hit.pos.y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)hit.pos.z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume)
        return;

    hit.pos.x = fsl_mod_i32(hit.pos.x, CHUNK_DIAMETER);
    hit.pos.y = fsl_mod_i32(hit.pos.y, CHUNK_DIAMETER);
    hit.pos.z = fsl_mod_i32(hit.pos.z, CHUNK_DIAMETER);

    chunk = &chunk_tab.p[index];
    if (!(*chunk)->block[hit.pos.z][hit.pos.y][hit.pos.x])
        return;

    chunk_tab_coordinates.x = index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = index / settings.chunk_buf_layer;

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

    block_remove_internal(*chunk, px, nx, py, ny, pz, nz, hit.pos.x, hit.pos.y, hit.pos.z);
}

void block_remove_internal(hhc_chunk *chunk,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z)
{
    chunk->flag |= FLAG_CHUNK_DIRTY;
    chunk->block[z][y][x] = 0;

    if (x == CHUNK_DIAMETER - 1 && px && px->block[z][y][0])
        px->flag |= FLAG_CHUNK_DIRTY;

    if (x == 0 && nx && nx->block[z][y][CHUNK_DIAMETER - 1])
        nx->flag |= FLAG_CHUNK_DIRTY;

    if (y == CHUNK_DIAMETER - 1 && py && py->block[z][0][x])
        py->flag |= FLAG_CHUNK_DIRTY;

    if (y == 0 && ny && ny->block[z][CHUNK_DIAMETER - 1][x])
        ny->flag |= FLAG_CHUNK_DIRTY;

    if (z == CHUNK_DIAMETER - 1 && pz && pz->block[0][y][x])
        pz->flag |= FLAG_CHUNK_DIRTY;

    if (z == 0 && nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        nz->flag |= FLAG_CHUNK_DIRTY;
}

void block_evaluate_internal(hhc_chunk *chunk,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z, enum block_id block_id)
{
    if (z == CHUNK_DIAMETER - 1)
    {
        if (pz && pz->block[0][y][x])
        {
            pz->flag |= FLAG_CHUNK_DIRTY;

            if (GET_BLOCK_ID(chunk->block[z][y][x]) == BLOCK_GRASS)
                SET_BLOCK_ID(chunk->block[z][y][x], BLOCK_DIRT);
        }
    }

    if (z == 0)
    {
        if (nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            nz->flag |= FLAG_CHUNK_DIRTY;

            if (GET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x]) == BLOCK_GRASS)
                SET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x], BLOCK_DIRT);
        }
    }
    else if (GET_BLOCK_ID(chunk->block[z - 1][y][x]) == BLOCK_GRASS)
    {
        chunk->flag |= FLAG_CHUNK_DIRTY;

        SET_BLOCK_ID(chunk->block[z - 1][y][x], BLOCK_DIRT);
    }
}

u32 chunk_sphere_radius_get_internal(u32 radius)
{
    return radius ? radius * radius + 2 : 0;
}

void chunk_pos_set_internal(hhc_chunk *chunk,
        v3i32 player_chunk_delta, v3u32 chunk_tab_coordinates)
{
    v3f32 chunk_pos = {0};
    v3i32 center = {0};
    v3i32 pos = {0};

    center.x = settings.render_distance;
    center.y = settings.render_distance;
    center.z = settings.render_distance;
    pos.x = chunk_tab_coordinates.x;
    pos.y = chunk_tab_coordinates.y;
    pos.z = chunk_tab_coordinates.z;

    chunk->pos_world.x = player_chunk_delta.x + chunk_tab_coordinates.x - settings.chunk_buf_radius;
    chunk->pos_world.y = player_chunk_delta.y + chunk_tab_coordinates.y - settings.chunk_buf_radius;
    chunk->pos_world.z = player_chunk_delta.z + chunk_tab_coordinates.z - settings.chunk_buf_radius;

    chunk->pos.x = fsl_mod_i32(chunk->pos_world.x + WORLD_RADIUS, WORLD_DIAMETER) - WORLD_RADIUS;
    chunk->pos.y = fsl_mod_i32(chunk->pos_world.y + WORLD_RADIUS, WORLD_DIAMETER) - WORLD_RADIUS;
    chunk->pos.z = fsl_mod_i32(chunk->pos_world.z + WORLD_RADIUS_VERTICAL,
            WORLD_DIAMETER_VERTICAL) - WORLD_RADIUS_VERTICAL;

    chunk->cti =
        chunk_tab_coordinates.x +
        chunk_tab_coordinates.y * settings.chunk_buf_diameter +
        chunk_tab_coordinates.z * settings.chunk_buf_layer;
    chunk->coi = fsl_distance_v3i32(pos, center);

    chunk->id =
        (u64)(chunk->pos.x & 0xffff) << 0x00 |
        (u64)(chunk->pos.y & 0xffff) << 0x10 |
        (u64)(chunk->pos.z & 0xffff) << 0x20;

    if (chunk->mesh_deprecated.initialized)
    {
        chunk_pos.x = (f32)chunk->pos_world.x * CHUNK_DIAMETER;
        chunk_pos.y = (f32)chunk->pos_world.y * CHUNK_DIAMETER;
        chunk_pos.z = (f32)chunk->pos_world.z * CHUNK_DIAMETER;

        glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.vbo_transform);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v3f32), &chunk_pos, GL_STATIC_DRAW);
    }
}

chunk_work_cost chunk_load_internal(hhc_chunk *chunk, chunk_scheduler_budget budget)
{
    chunk_work_cost cost = 0;
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    if (!chunk || chunk->flag & FLAG_CHUNK_GENERATED)
        return 0;

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, chunk->pos.x, chunk->pos.y, chunk->pos.z);

#if MODE_INTERNAL_IMPORT_CHUNKS
    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
        cost += chunk_import_internal(path, chunk);
#endif /* MODE_INTERNAL_IMPORT_CHUNKS */
    cost += chunk_generate_internal(chunk, budget);
    return cost;
}

chunk_work_cost chunk_generate_internal(hhc_chunk *chunk, chunk_scheduler_budget budget)
{
    chunk_work_cost cost = 0;
    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    /* one for terrain, four for world overflow sampling */
    v3i32 coordinates[4] = {0};

    /* one for terrain, four for world overflow sampling */
    hhc_terrain_noise terrain_noise[4] = {0};

    v3i32 world_pos_from_chunk_pos = {0};
    hhc_terrain terrain_info = {0};
    v3u8 blend_world_margin = {0};
    v3f32 blend_factor = {0};
    f32 blend_factor_scale = 1.0f / (WORLD_MARGIN * CHUNK_DIAMETER * 2);
    v3f32 overflow_offset = {0};
    v3f32 overflow_edge = {0};
    v3f32 overflow_sign = {0};
    i32 x = 0, y = 0, z = 0;

    if (chunk->cursor == CHUNK_VOLUME)
    {
        chunk->flag |= FLAG_CHUNK_GENERATED;
        cost = chunk_mesh_update_internal(chunk);
        return cost;
    }

    world_pos_from_chunk_pos.x = chunk->pos_world.x * CHUNK_DIAMETER;
    world_pos_from_chunk_pos.y = chunk->pos_world.y * CHUNK_DIAMETER;
    world_pos_from_chunk_pos.z = chunk->pos_world.z * CHUNK_DIAMETER;

    /* ---- section: setup world margin noise blending ---------------------- */

    if (chunk->pos_world.x >= WORLD_RADIUS - WORLD_MARGIN)
    {
        blend_world_margin.x = 1;
        overflow_offset.x = -WORLD_DIAMETER * CHUNK_DIAMETER;
        overflow_edge.x = -WORLD_RADIUS * CHUNK_DIAMETER;
        overflow_sign.x = 1.0f;
    }
    else if (chunk->pos_world.x < -WORLD_RADIUS + WORLD_MARGIN)
    {
        blend_world_margin.x = 1;
        overflow_offset.x = WORLD_DIAMETER * CHUNK_DIAMETER;
        overflow_edge.x = WORLD_RADIUS * CHUNK_DIAMETER;
        overflow_sign.x = -1.0f;
    }

    if (chunk->pos_world.y >= WORLD_RADIUS - WORLD_MARGIN)
    {
        blend_world_margin.y = 1;
        overflow_offset.y = -WORLD_DIAMETER * CHUNK_DIAMETER;
        overflow_edge.y = -WORLD_RADIUS * CHUNK_DIAMETER;
        overflow_sign.y = 1.0f;
    }
    else if (chunk->pos_world.y < -WORLD_RADIUS + WORLD_MARGIN)
    {
        blend_world_margin.y = 1;
        overflow_offset.y = WORLD_DIAMETER * CHUNK_DIAMETER;
        overflow_edge.y = WORLD_RADIUS * CHUNK_DIAMETER;
        overflow_sign.y = -1.0f;
    }

    if (chunk->pos_world.z >= WORLD_RADIUS_VERTICAL - WORLD_MARGIN)
    {
        blend_world_margin.z = 1;
        overflow_offset.z = -WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER;
        overflow_edge.z = -WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER;
        overflow_sign.z = 1.0f;
    }
    else if (chunk->pos_world.z < -WORLD_RADIUS_VERTICAL + WORLD_MARGIN)
    {
        blend_world_margin.z = 1;
        overflow_offset.z = WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER;
        overflow_edge.z = WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER;
        overflow_sign.z = -1.0f;
    }

    /* ---- end section: setup world margin noise blending ------------------ */

    chunk_tab_coordinates.x = chunk->cti % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (chunk->cti / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = chunk->cti / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = chunk_tab.p[chunk->cti + 1];
    if (chunk_tab_coordinates.x > 0)
        nx = chunk_tab.p[chunk->cti - 1];
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = chunk_tab.p[chunk->cti + settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.y > 0)
        ny = chunk_tab.p[chunk->cti - settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = chunk_tab.p[chunk->cti + settings.chunk_buf_layer];
    if (chunk_tab_coordinates.z > 0)
        nz = chunk_tab.p[chunk->cti - settings.chunk_buf_layer];

    x = chunk->cursor % CHUNK_DIAMETER;
    y = (chunk->cursor / CHUNK_DIAMETER) % CHUNK_DIAMETER;
    z = chunk->cursor / CHUNK_LAYER;

    /* `x`, `y` and `z` reset at the end of their loops because they should first pick up from
     * where `chunk->cursor` left off last time. */
    coordinates[0].z = z + world_pos_from_chunk_pos.z;
    for (; z < CHUNK_DIAMETER; ++z, ++coordinates[0].z)
    {
        coordinates[0].y = y + world_pos_from_chunk_pos.y;
        for (; y < CHUNK_DIAMETER; ++y, ++coordinates[0].y)
        {
            coordinates[0].x = x + world_pos_from_chunk_pos.x;
            for (; x < CHUNK_DIAMETER; ++x, ++coordinates[0].x)
            {
                terrain_noise[0] = world.terrain_noise_func(coordinates[0]);
                cost += terrain_noise[0].cost;

                coordinates[1].x = coordinates[0].x + overflow_offset.x;
                coordinates[1].y = coordinates[0].y;
                coordinates[1].z = coordinates[0].z;
                coordinates[2].x = coordinates[0].x;
                coordinates[2].y = coordinates[0].y + overflow_offset.y;
                coordinates[2].z = coordinates[0].z;

                if (blend_world_margin.x && blend_world_margin.y)
                {
                    coordinates[3].x = coordinates[1].x;
                    coordinates[3].y = coordinates[2].y;
                    coordinates[3].z = coordinates[0].z;

                    blend_factor.x = 0.5f -
                        ((overflow_edge.x - (f32)coordinates[1].x) * overflow_sign.x) *
                        blend_factor_scale;
                    blend_factor.y = 0.5f -
                        ((overflow_edge.y - (f32)coordinates[2].y) * overflow_sign.y) *
                        blend_factor_scale;

                    terrain_noise[1] = world.terrain_noise_func(coordinates[1]);
                    terrain_noise[2] = world.terrain_noise_func(coordinates[2]);
                    terrain_noise[3] = world.terrain_noise_func(coordinates[3]);
                    terrain_noise[0] = terrain_noise_bilerp(
                            &terrain_noise[0], &terrain_noise[1],
                            &terrain_noise[2], &terrain_noise[3],
                            blend_factor.x, blend_factor.y);

                    cost += terrain_noise[0].cost;
                    cost += terrain_noise[1].cost;
                    cost += terrain_noise[2].cost;
                    cost += terrain_noise[3].cost;
                }
                else if (blend_world_margin.x)
                {
                    blend_factor.x = 0.5f -
                        ((overflow_edge.x - (f32)coordinates[1].x) * overflow_sign.x) *
                        blend_factor_scale;

                    terrain_noise[1] = world.terrain_noise_func(coordinates[1]);
                    terrain_noise[0] =
                        terrain_noise_lerp(&terrain_noise[0], &terrain_noise[1], blend_factor.x);

                    cost += terrain_noise[0].cost;
                    cost += terrain_noise[1].cost;
                }
                else if (blend_world_margin.y)
                {
                    blend_factor.y = 0.5f -
                        ((overflow_edge.y - (f32)coordinates[2].y) * overflow_sign.y) *
                        blend_factor_scale;

                    terrain_noise[1] = world.terrain_noise_func(coordinates[2]);
                    terrain_noise[0] =
                        terrain_noise_lerp(&terrain_noise[0], &terrain_noise[1], blend_factor.y);

                    cost += terrain_noise[0].cost;
                    cost += terrain_noise[1].cost;
                }

                terrain_info = world.terrain_func(coordinates[0], &terrain_noise[0]);

                if (terrain_info.block_id)
                {
                    block_add_internal(chunk, px, nx, py, ny, pz, nz, x, y, z, terrain_info.block_id);
                }

                if (cost >= (u32)budget)
                    goto finish_generation;
            }
            x = 0;
        }
        y = 0;
    }

finish_generation:

    chunk->cursor = x + y * CHUNK_DIAMETER + z * CHUNK_LAYER;
    return cost;
}

chunk_work_cost chunk_mesh_update_internal(hhc_chunk *chunk)
{
    chunk_work_cost cost = 0;
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};
    v3f32 chunk_pos = {0};

    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *start = (u32*)chunk->block;
    u32 *end = start + CHUNK_VOLUME;
    u32 *curr = start;
    u64 block_index = 0;
    u32 block_cache = 0;
    v3u64 pos = {0};
    b8 should_render = FALSE;

    chunk_tab_coordinates.x = chunk->cti % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (chunk->cti / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = chunk->cti / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = chunk_tab.p[chunk->cti + 1];
    if (chunk_tab_coordinates.x > 0)
        nx = chunk_tab.p[chunk->cti - 1];
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = chunk_tab.p[chunk->cti + settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.y > 0)
        ny = chunk_tab.p[chunk->cti - settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = chunk_tab.p[chunk->cti + settings.chunk_buf_layer];
    if (chunk_tab_coordinates.z > 0)
        nz = chunk_tab.p[chunk->cti - settings.chunk_buf_layer];

    for (; curr < end; ++curr)
    {
        if (*curr & MASK_BLOCK_ID)
        {
            block_index = curr - start;
            pos.x = block_index % CHUNK_DIAMETER;
            pos.y = (block_index / CHUNK_DIAMETER) % CHUNK_DIAMETER;
            pos.z = block_index / CHUNK_LAYER;
            block_cache = block_get_faces_internal(chunk, px, nx, py, ny, pz, nz, pos.x, pos.y, pos.z);
            if (block_cache & MASK_BLOCK_FACES)
            {
                should_render = TRUE;
                SET_BLOCK_LIGHT(block_cache, 63);
                *(cursor++) = block_cache |
                    (pos.x & 0xf) << SHIFT_BLOCK_X |
                    (pos.y & 0xf) << SHIFT_BLOCK_Y |
                    (pos.z & 0xf) << SHIFT_BLOCK_Z;
            }
            cost += CHUNK_WORK_COST_MESH_NON_AIR;
        }
        else
            cost += CHUNK_WORK_COST_MESH_AIR;
    }

    ++cur_buf;
    if (cur_buf >= BLOCK_BUFFERS_MAX)
        cur_buf = 0;

    if (should_render)
    {
        chunk->flag |= FLAG_CHUNK_VISIBLE;
        chunk->color = CHUNK_GIZMO_COLOR_VISIBLE;

        if (!chunk->mesh_deprecated.initialized)
        {
            chunk->mesh_deprecated.initialized = TRUE;

            chunk_pos.x = (f32)chunk->pos_world.x * CHUNK_DIAMETER;
            chunk_pos.y = (f32)chunk->pos_world.y * CHUNK_DIAMETER;
            chunk_pos.z = (f32)chunk->pos_world.z * CHUNK_DIAMETER;

            glGenVertexArrays(1, &chunk->mesh_deprecated.vao);
            glGenBuffers(1, &chunk->mesh_deprecated.vbo);
            glGenBuffers(1, &chunk->mesh_deprecated.vbo_transform);

            glBindVertexArray(chunk->mesh_deprecated.vao);
            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.vbo);
            glBufferData(GL_ARRAY_BUFFER, (cursor - buf) * sizeof(u64), buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)sizeof(u32));

            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.vbo_transform);
            glBufferData(GL_ARRAY_BUFFER, sizeof(v3f32), &chunk_pos, GL_STATIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(v3f32), (void*)0);
            glVertexAttribDivisor(2, 1);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.vbo);
            glBufferData(GL_ARRAY_BUFFER, (cursor - buf) * sizeof(u64), buf, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        chunk->mesh_deprecated.vbo_len = cursor - buf;
    }
    else
    {
        chunk->flag &= ~FLAG_CHUNK_VISIBLE;
        chunk->color = CHUNK_GIZMO_COLOR_LOADED;

        if (chunk->mesh_deprecated.initialized)
        {
            chunk->mesh_deprecated.initialized = FALSE;
            glDeleteBuffers(1, &chunk->mesh_deprecated.vbo_transform);
            glDeleteBuffers(1, &chunk->mesh_deprecated.vbo);
            glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
        }
    }

    chunk->flag &= ~FLAG_CHUNK_DIRTY;
    chunk_debug_chunk_gizmo_write_internal(chunk);

    return cost;
}

chunk_work_cost chunk_export_internal(hhc_chunk *chunk)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};
    static u16 buf[CHUNK_VOLUME] = {0};
    u32 *blocks = (u32*)chunk->block;
    u32 i = 0;
    u32 j = 0;
    u32 rle = 0; /* run-length */

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, chunk->pos.x, chunk->pos.y, chunk->pos.z);

    for (; i < CHUNK_VOLUME; ++j, i += rle, blocks += rle)
    {
        buf[j] = *blocks;
        rle = fsl_rle(blocks, sizeof(u32), CHUNK_VOLUME - i);
        if (rle > 1)
        {
            buf[j++] |= FLAG_BLOCK_RLE;
            buf[j] = rle;
        }
    }

    fsl_write_file(path, j * sizeof(u16), buf, TRUE, FALSE);

    return CHUNK_WORK_COST_EXPORT;
}

chunk_work_cost chunk_import_internal(const fsl_fs_path *path, hhc_chunk *chunk)
{
    FILE *file = NULL;
    str file_name[FSL_ID_CAP] = {0};
    str *cursor = file_name;
    i64 pos_cache[3] = {0};
    static u16 buf[CHUNK_VOLUME] = {0};
    u32 *blocks = (u32*)chunk->block;
    u32 i = 0;
    u32 j = 0;
    u32 rle = 0;

    fsl_get_base_name(path, file_name, FSL_ID_CAP);

    for (i = 0; i < 3; ++i)
    {
        fsl_convert_str_to_i64(cursor, &pos_cache[i]);
        while (cursor && *cursor++ != '.')
        {}
        if (!cursor)
            break;
    }

    chunk->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_IMPORTED | FLAG_CHUNK_DIRTY;
    chunk->pos.x = pos_cache[0];
    chunk->pos.y = pos_cache[1];
    chunk->pos.z = pos_cache[2];

    file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    i = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(buf, 1, i, file);
    fclose(file);

    if (i == 8 && buf[0] & FLAG_BLOCK_RLE && !buf[1])
        return CHUNK_WORK_COST_IMPORT_AIR;

    for (i = 0; i < CHUNK_VOLUME && j < CHUNK_VOLUME; ++i)
    {
        if (buf[i] & FLAG_BLOCK_RLE)
        {
            buf[i] &= ~FLAG_BLOCK_RLE;
            rle = buf[i + 1];
            while (rle--)
                blocks[j++] = buf[i];
            ++i;
        }
        else
            blocks[j++] = buf[i];
    }

    chunk->cursor = j;

    return CHUNK_WORK_COST_IMPORT_NON_AIR;
}

void chunk_buf_update_internal(v3i32 *player_chunk_delta)
{
    i32 i = 0;
    i32 end = chunk_order.chunks_max;

    for (; i < end; ++i)
    {
        if (!chunk_tab.p[chunk_order.p[i]])
            chunk_buf_push_internal(chunk_order.p[i], *player_chunk_delta);
    }
}

void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta)
{
    hhc_chunk nochunk = {0};
    v3u32 chunk_tab_coordinates = {0};
    hhc_chunk *chunk = NULL;
    v3u64 seed;
    v3u8 color_variant;
    u32 end = chunk_buf.cursor;

    chunk_tab_coordinates.x = index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = index / settings.chunk_buf_layer;

    do
    {
        if (!(chunk_buf.p[chunk_buf.cursor].flag & FLAG_CHUNK_LOADED))
        {
            chunk = &chunk_buf.p[chunk_buf.cursor];
            if (chunk->mesh_deprecated.initialized)
            {
                chunk->mesh_deprecated.initialized = FALSE;
                glDeleteBuffers(1, &chunk->mesh_deprecated.vbo_transform);
                glDeleteBuffers(1, &chunk->mesh_deprecated.vbo);
                glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
            }
            *chunk = nochunk;

            chunk_pos_set_internal(chunk, player_chunk_delta, chunk_tab_coordinates);

            chunk->color = CHUNK_GIZMO_COLOR_LOADED;

            seed.x = chunk_tab_coordinates.x * index + player_chunk_delta.z;
            seed.y = chunk_tab_coordinates.y * index + player_chunk_delta.x;
            seed.z = chunk_tab_coordinates.z * index + player_chunk_delta.y;
            color_variant.x =
                (u8)(fsl_map_range_f64((f64)(fsl_rand_u64(seed.x) % 0xff), 0.0, 0xff,
                            1.0 - CHUNK_GIZMO_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);
            color_variant.y =
                (u8)(fsl_map_range_f64((f64)(fsl_rand_u64(seed.y) % 0xff), 0.0, 0xff,
                            1.0 - CHUNK_GIZMO_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);
            color_variant.z =
                (u8)(fsl_map_range_f64((f64)(fsl_rand_u64(seed.z) % 0xff), 0.0, 0xff,
                            1.0 - CHUNK_GIZMO_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);

            chunk->color_variant = 0 |
                (color_variant.x << 0x18) |
                (color_variant.y << 0x10) |
                (color_variant.z << 0x08);

            chunk->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY;
            chunk_tab.p[index] = chunk;
            chunk_debug_chunk_gizmo_write_internal(chunk);

            ++chunk_buf.cursor;
            if (chunk_buf.cursor >= chunk_order.chunks_max)
                chunk_buf.cursor = 0;
            return;
        }

        ++chunk_buf.cursor;
        if (chunk_buf.cursor >= chunk_order.chunks_max)
            chunk_buf.cursor = 0;
    } while (chunk_buf.cursor != end);

    LOGERROR(FSL_ERR_BUFFER_FULL,
            FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "Failed to Push to `chunk_buf`, Buffer Full\n");
}

void chunk_buf_pop_internal(hhc_chunk *chunk)
{
    u32 index_popped = chunk - chunk_buf.p;

    if (chunk->mesh_deprecated.initialized)
    {
        chunk->mesh_deprecated.initialized = FALSE;
        glDeleteBuffers(1, &chunk->mesh_deprecated.vbo_transform);
        glDeleteBuffers(1, &chunk->mesh_deprecated.vbo);
        glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
    }

    chunk->flag = 0;
    chunk_debug_chunk_gizmo_write_internal(chunk);

    if (chunk_buf.cursor > index_popped)
        chunk_buf.cursor = index_popped;
    chunk_tab.p[chunk->cti] = NULL;
}

u32 chunk_scheduler_init_internal(hhc_chunk_scheduler *sched, chunk_scheduler_id id,
        chunk_scheduler_radius radius_start, chunk_scheduler_radius radius_end,
        chunk_scheduler_budget budget)
{
    if (fsl_mem_arena_push(&memory_arena_chunking_internal, &sched->schedule,
                (chunk_order.len[radius_end] - chunk_order.len[radius_start]) * sizeof(hhc_chunk*),
                "chunk_scheduler_init_internal().sched->schedule") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    sched->id = id;
    sched->radius_start = radius_start;
    sched->radius_end = fsl_clamp_u32(SET_RENDER_DISTANCE_MAX, 0, radius_end);
    sched->budget = budget;
    sched->p = fsl_mem_handle_get(sched->schedule);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void chunk_scheduler_update_internal(hhc_chunk_scheduler *sched, u32 start, u32 end,
        b8 should_push, b8 should_pop)
{
    hhc_chunk *chunk = NULL;
    u32 push = sched->cursor_push;
    u32 pop = sched->cursor_pop;
    i32 budget = sched->budget;
    u32 sched_len = chunk_order.len[sched->radius_end] - chunk_order.len[sched->radius_start];
    u32 radius_start = 0;
    u32 radius_end = 0;

    radius_start = chunk_sphere_radius_get_internal(sched->radius_start);
    radius_end = chunk_sphere_radius_get_internal(sched->radius_end);

    if (!should_push || sched->count >= sched_len)
        goto pop;

    if (budget <= 0)
        return;

    for (; start < end && sched->count < sched_len && budget > 0; ++start)
    {
        chunk = chunk_tab.p[chunk_order.p[start]];
        if (chunk)
        {
            if (chunk->flag & FLAG_CHUNK_DIRTY &&
                    !sched->p[push] &&
                    ((!(chunk->flag & FLAG_CHUNK_QUEUED) &&
                    (chunk->coi >= radius_start && chunk->coi < radius_end)) ||
                    (chunk->coi >= radius_start && chunk->coi < radius_end)))
            {
                chunk->flag |= FLAG_CHUNK_QUEUED;
                sched->p[push] = chunk;
                ++push;
                if (push >= sched_len)
                    push = 0;
                ++sched->count;
                budget -= CHUNK_WORK_COST_PUSH;
            }
        }
        budget -= CHUNK_WORK_COST_SCAN;
    }

    sched->cursor_push = push;

pop:

    if (!should_pop || !sched->count || budget <= 0)
        return;

    for (start = 0, end = sched_len; start < end && sched->count && budget > 0; ++start)
    {
        if (sched->p[pop])
        {
            chunk = sched->p[pop];

            if (!(chunk->flag & FLAG_CHUNK_LOADED) ||
                    chunk->coi < radius_start ||
                    chunk->coi >= radius_end)
            {
                chunk->flag &= ~FLAG_CHUNK_QUEUED;
                sched->p[pop] = NULL;
                --sched->count;
                ++pop;
                if (pop >= sched_len)
                    pop = 0;
                budget -= CHUNK_WORK_COST_POP;
                continue;
            }

            if (chunk->flag & FLAG_CHUNK_GENERATED)
                budget -= chunk_mesh_update_internal(chunk);
            else
                budget -= chunk_load_internal(chunk, budget);

            if (!(chunk->flag & FLAG_CHUNK_DIRTY))
            {
#if MODE_INTERNAL_EXPORT_CHUNKS
                if (!(chunk->flag & FLAG_CHUNK_IMPORTED))
                    chunk_export_internal(chunk);
#endif /* MODE_INTERNAL_EXPORT_CHUNKS */

                chunk->flag &= ~(FLAG_CHUNK_QUEUED | FLAG_CHUNK_IMPORTED);
                sched->p[pop] = NULL;
                --sched->count;
                ++pop;
                if (pop >= sched_len)
                    pop = 0;
                budget -= CHUNK_WORK_COST_POP;
            }
        }
        else
        {
            ++pop;
            if (pop >= sched_len)
                pop = 0;
            budget -= CHUNK_WORK_COST_POP;
        }
    }

    sched->cursor_pop = pop;
}

u32 *get_block_resolved(hhc_chunk *chunk, i32 x, i32 y, i32 z)
{
    x = fsl_mod_i32(x, CHUNK_DIAMETER);
    y = fsl_mod_i32(y, CHUNK_DIAMETER);
    z = fsl_mod_i32(z, CHUNK_DIAMETER);
    return &chunk->block[z][y][x];
}

hhc_chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z)
{
    x = (i32)floorf((f32)x / CHUNK_DIAMETER);
    y = (i32)floorf((f32)y / CHUNK_DIAMETER);
    z = (i32)floorf((f32)z / CHUNK_DIAMETER);
    return chunk_tab.p[index + x +
        y * settings.chunk_buf_diameter +
        z * settings.chunk_buf_layer];
}

u32 get_chunk_index(v3i32 chunk_pos, v3i64 pos)
{
    v3i32 offset = {0};
    u32 index = 0;

    offset.x = pos.x / CHUNK_DIAMETER - chunk_pos.x + settings.chunk_buf_radius,
    offset.y = pos.y / CHUNK_DIAMETER - chunk_pos.y + settings.chunk_buf_radius,
    offset.z = pos.z / CHUNK_DIAMETER - chunk_pos.z + settings.chunk_buf_radius,
    index =
        offset.x +
        offset.y * settings.chunk_buf_diameter +
        offset.z * settings.chunk_buf_layer;

    if (index >= settings.chunk_buf_volume)
        return settings.chunk_tab_center;
    return index;
}
