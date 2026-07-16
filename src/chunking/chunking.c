#include "deps/fossil/common/limits.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/plugins/fsl_native/noise_sampler/noise_sampler.h"
#include "deps/fossil/string/string.h"

#include "deps/fossil/h/dir.h"

#include "../settings/settings.h"
#include "../terrain/terrain.h"

#include "../h/assets.h"
#include "../h/config_internal.h"
#include "../h/common.h"
#include "../h/diagnostics.h"
#include "../h/main.h"
#include "../h/world.h"

#include "chunk_work.h"
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
hhc_chunk_scheduler chunk_sched = {0};
static hhc_chunk_sampler chunk_sampler = {0};

static void (*chunk_work_func_internal[STATUS_CHUNK_COUNT])(hhc_chunk *chunk,
        chunk_work_budget budget, hhc_chunk_receipt *receipt) =
{
    chunk_idle_func_internal,
    chunk_load_func_internal,
    chunk_generate_func_internal,
    chunk_mesh_func_internal,
    chunk_light_func_internal,
    chunk_ao_func_internal,
    chunk_finish_func_internal,
    chunk_idle_func_internal,
    chunk_idle_func_internal,
    chunk_idle_func_internal
};

/* ---- section: implementation: chunk -------------------------------------- */

u32 chunking_init(v3i32 *player_chunk_delta)
{
    if (core.flag.chunks_initialized)
        return FSL_ERR_SUCCESS;

    if (chunks_max_init_internal() != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    chunk_sched.buckets_max = chunk_sphere_radius_get_internal(SET_RENDER_DISTANCE_MAX);

    if (settings.flag.render_distance_dirty)
    {
        settings.flag.render_distance_dirty = FALSE;
        chunk_buf_dump_internal();
        chunk_order.chunks_max = chunk_order.len[settings.render_distance];
    }

    if (fsl_mem_arena_init(&memory_arena_chunking_internal,
                "chunking_init().memory_arena_chunking_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_sched.handle_p,
                chunk_order.len[SET_RENDER_DISTANCE_MAX] * sizeof(hhc_chunk*),
                "chunking_init().chunk_sched.handle_p") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_sched.handle_bucket,
                chunk_sched.buckets_max * sizeof(hhc_chunk_bucket),
                "chunking_init().chunk_sched.handle_bucket") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_order.handle_p,
                chunk_order.len[SET_RENDER_DISTANCE_MAX] * sizeof(u32),
                "chunking_init().chunk_order.handle_p") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_order.handle_inv,
                CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().chunk_order.handle_inv") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_tab.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(hhc_chunk*),
                "chunking_init().chunk_tab.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_buf.handle,
                chunk_order.len[SET_RENDER_DISTANCE_MAX] * sizeof(hhc_chunk),
                "chunking_init().chunk_buf.handle") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (chunk_debug_init_internal(CHUNK_BUF_VOLUME_MAX) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (chunk_order_init_internal() != FSL_ERR_SUCCESS)
        goto cleanup;

    chunk_order.inv = fsl_mem_handle_get(chunk_order.handle_inv);
    chunk_order.p = fsl_mem_handle_get(chunk_order.handle_p);
    chunk_tab.p = fsl_mem_handle_get(chunk_tab.handle);
    chunk_buf.p = fsl_mem_handle_get(chunk_buf.handle);
    chunk_sched.bucket = fsl_mem_handle_get(chunk_sched.handle_bucket);
    chunk_sched.p = fsl_mem_handle_get(chunk_sched.handle_p);

    if (chunk_bucket_load_internal() != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_noise_sampler_init(
            &chunk_sampler.sampler,
            TERRAIN_NOISE_COUNT + BIOME_NOISE_COUNT, 8,
            (f64)(WORLD_RADIUS * CHUNK_DIAMETER),
            (f64)(WORLD_RADIUS * CHUNK_DIAMETER),
            (f64)(WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER),
            (f64)(WORLD_DIAMETER * CHUNK_DIAMETER),
            (f64)(WORLD_DIAMETER * CHUNK_DIAMETER),
            (f64)(WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER),
            (f64)(WORLD_MARGIN * CHUNK_DIAMETER),
            (f64)(WORLD_MARGIN * CHUNK_DIAMETER),
            (f64)(WORLD_MARGIN * CHUNK_DIAMETER)) != FSL_ERR_SUCCESS)
        goto cleanup;

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
    v3u32 center = {0};
    v3u32 pos = {0};
    u32 radius = 0;
    u32 diameter = 0;
    u32 chunk_count = 0;
    u32 i = 0;

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNKS_MAX);
    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
        goto load_from_disk;

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "Building `chunks_max` Look-up..\n");

    for (i = 0; i < SET_RENDER_DISTANCE_MAX + 1; ++i)
    {
        radius = chunk_sphere_radius_get_internal(i);
        diameter = i * 2 + 1;
        chunk_count = 0;
        center.x = i;
        center.y = i;
        center.z = i;

        for (pos.z = 0; pos.z < diameter; ++pos.z)
        {
            for (pos.y = 0; pos.y < diameter; ++pos.y)
            {
                for (pos.x = 0; pos.x < diameter; ++pos.x)
                {
                    if (fsl_distance_v3u32(pos, center) < radius)
                        ++chunk_count;
                }
            }
        }
        chunk_order.len[i] = chunk_count;
    }

    if (fsl_write_file(path, (SET_RENDER_DISTANCE_MAX + 1) * sizeof(u32),
                chunk_order.len, TRUE, FALSE) != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("`chunks_max` Look-up '%s' Exported\n", path));

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

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
    return *GAME_ERR;
}

u32 chunk_order_init_internal(void)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_ORDER);

    chunk_order.p = fsl_mem_handle_get(chunk_order.handle_p);
    chunk_order.inv = fsl_mem_handle_get(chunk_order.handle_inv);
    chunk_tab.p = fsl_mem_handle_get(chunk_tab.handle);
    chunk_sched.p = fsl_mem_handle_get(chunk_sched.handle_p);
    chunk_sched.bucket = fsl_mem_handle_get(chunk_sched.handle_bucket);

    if (!chunk_tab.p || !chunk_sched.p || !chunk_sched.bucket)
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

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

u32 chunk_order_build_internal(void)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    hhc_chunk_bucket_format *bucket_buf = NULL;
    u32 buckets_max = chunk_sched.buckets_max;
    u32 *distance_buf = NULL;
    u32 distance_cache = 0;
    v3i8 *pos_buf = NULL;
    v3i8 *data_buf = NULL;

    v3u32 center =
    {
        CHUNK_BUF_RADIUS_MAX,
        CHUNK_BUF_RADIUS_MAX,
        CHUNK_BUF_RADIUS_MAX
    };
    v3u32 pos = {0};
    u32 chunk_count = 0;
    u32 i = 0;

    if (fsl_mem_map((void*)&bucket_buf, buckets_max * sizeof(hhc_chunk_bucket_format),
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
                distance_cache = fsl_distance_v3u32(pos, center);
                if (distance_cache < buckets_max)
                {
                    ++bucket_buf[distance_cache].len;
                    ++bucket_buf[distance_cache].pos;
                    distance_buf[chunk_count] = distance_cache;
                    pos_buf[chunk_count].x = pos.x - SET_RENDER_DISTANCE_MAX;
                    pos_buf[chunk_count].y = pos.y - SET_RENDER_DISTANCE_MAX;
                    pos_buf[chunk_count].z = pos.z - SET_RENDER_DISTANCE_MAX;
                    ++chunk_count;
                }
            }
        }
    }

    for (i = 1; i < buckets_max; ++i)
        bucket_buf[i].pos += bucket_buf[i - 1].pos;

    for (i = 0; i < chunk_count; ++i)
        data_buf[--bucket_buf[distance_buf[i]].pos] = pos_buf[i];

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_ORDER);
    if (fsl_write_file(path, chunk_count * sizeof(v3i8), data_buf, TRUE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("`chunk_order` Look-up '%s' Exported\n", path));

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_BUCKET);
    if (fsl_write_file(path, buckets_max * sizeof(hhc_chunk_bucket_format), bucket_buf, TRUE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("`chunk_sched` Look-up '%s' Exported\n", path));

    fsl_mem_unmap((void*)&bucket_buf, buckets_max * sizeof(hhc_chunk_bucket_format),
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

    fsl_mem_unmap((void*)&bucket_buf, buckets_max * sizeof(hhc_chunk_bucket_format),
            "chunk_order_build_internal().bucket_buf");
    fsl_mem_unmap((void*)&distance_buf, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunk_order_build_internal().distance_buf");
    fsl_mem_unmap((void*)&pos_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
            "chunk_order_build_internal().pos_buf");
    fsl_mem_unmap((void*)&data_buf, CHUNK_BUF_VOLUME_MAX * sizeof(v3i8),
                "chunk_order_build_internal().data_buf");
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
        chunk_order.inv[index] = i;
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

u32 chunk_bucket_load_internal(void)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    hhc_chunk_bucket_format *file_contents = NULL;
    u64 file_len = 0;
    u32 i = 0;

    snprintf(path, FSL_PATH_CAP, "%s%s", GAME_DIR_NAME_LOOKUPS, GAME_FILE_NAME_LOOKUP_CHUNK_BUCKET);
    file_len = fsl_get_file_contents(path, (void*)&file_contents, FALSE);
    if (file_contents == NULL)
        return *GAME_ERR;

    for (i = 0; i < chunk_sched.buckets_max; ++i)
    {
        chunk_sched.bucket[i].pos = file_contents[i].pos;
        chunk_sched.bucket[i].len = file_contents[i].len;
        chunk_sched.bucket[i].push = file_contents[i].pos;
        chunk_sched.bucket[i].pop = file_contents[i].pos;
    }

    fsl_mem_free((void*)&file_contents, file_len,
            "chunk_bucket_load_internal().file_contents");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta, block_hit hit)
{
    v3i32 chunk_delta = {0};
    i32 render_distance = 0;

    if (settings.flag.render_distance_dirty)
    {
        settings.flag.render_distance_dirty = FALSE;
        chunk_buf_dump_internal();
        chunk_order.chunks_max = chunk_order.len[settings.render_distance];
        chunk_order_load_internal(settings.render_distance);
    }

    chunk_tab.index = chunk_index_get(player_chunk, hit.pos);
    chunk_receipt_print(&chunk_tab.p[settings.chunk_tab_center]->receipt,
            &chunk_tab.receipt_center);

    chunk_delta.x = player_chunk.x - player_chunk_delta->x;
    chunk_delta.y = player_chunk.y - player_chunk_delta->y;
    chunk_delta.z = player_chunk.z - player_chunk_delta->z;

    if (!(chunk_delta.x || chunk_delta.y || chunk_delta.z))
    {
        chunk_scheduler_update_internal();
        chunk_debug_chunk_gizmo_bake_internal();
        return;
    }

    render_distance = chunk_sphere_radius_get_internal(settings.render_distance);

    if (fsl_len_v3i32(chunk_delta) < render_distance)
    {
        chunk_tab_shift_internal(player_chunk, player_chunk_delta);
    }
    else
    {
        chunk_buf_dump_internal();
        *player_chunk_delta = player_chunk;
    }

    chunk_buf_update_internal(player_chunk_delta);
}

void chunking_free(void)
{
    u32 i = 0;

    fsl_noise_sampler_free(&chunk_sampler.sampler);

    if (chunk_tab.p)
    {
        for (; i < settings.chunk_buf_volume; ++i)
            if (chunk_tab.p[i])
                chunk_buf_pop_internal(chunk_tab.p[i]);
    }

    chunk_debug_free_internal();

    fsl_mem_arena_free(&memory_arena_chunking_internal,
            "chunking_free().memory_arena_chunking_internal");
}

u32 chunk_sphere_radius_get_internal(u32 radius)
{
    return radius ? radius * radius + 2 : 0;
}

hhc_chunk *chunk_resolved_get(u32 index, i32 x, i32 y, i32 z)
{
    x = (i32)floorf((f32)x / CHUNK_DIAMETER);
    y = (i32)floorf((f32)y / CHUNK_DIAMETER);
    z = (i32)floorf((f32)z / CHUNK_DIAMETER);
    return chunk_tab.p[index + x +
        y * settings.chunk_buf_diameter +
        z * settings.chunk_buf_layer];
}

u32 chunk_index_get(v3i32 chunk_pos, v3i64 pos)
{
    v3i32 offset = {0};
    u32 index = 0;

    offset.x = (i32)floor((f64)pos.x / CHUNK_DIAMETER) - chunk_pos.x + settings.chunk_buf_radius;
    offset.y = (i32)floor((f64)pos.y / CHUNK_DIAMETER) - chunk_pos.y + settings.chunk_buf_radius;
    offset.z = (i32)floor((f64)pos.z / CHUNK_DIAMETER) - chunk_pos.z + settings.chunk_buf_radius;
    index =
        offset.x +
        offset.y * settings.chunk_buf_diameter +
        offset.z * settings.chunk_buf_layer;

    if (index >= settings.chunk_buf_volume)
        return settings.chunk_tab_center;
    return index;
}

hhc_chunk_neighbors chunk_neighbors_get_internal(hhc_chunk *chunk)
{
    hhc_chunk_neighbors neighbors = {0};
    v3u32 chunk_tab_coordinates = {0};

    chunk_tab_coordinates.x = chunk->cti % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (chunk->cti / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = chunk->cti / settings.chunk_buf_layer;

    neighbors.ch = chunk;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        neighbors.px = chunk_tab.p[chunk->cti + 1];
    if (chunk_tab_coordinates.x > 0)
        neighbors.nx = chunk_tab.p[chunk->cti - 1];
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        neighbors.py = chunk_tab.p[chunk->cti + settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.y > 0)
        neighbors.ny = chunk_tab.p[chunk->cti - settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        neighbors.pz = chunk_tab.p[chunk->cti + settings.chunk_buf_layer];
    if (chunk_tab_coordinates.z > 0)
        neighbors.nz = chunk_tab.p[chunk->cti - settings.chunk_buf_layer];

    return neighbors;
}

void chunk_pos_set_internal(hhc_chunk *chunk, v3i32 player_chunk_delta, v3u32 chunk_tab_coordinates)
{
    v3u32 center = {0};

    center.x = settings.render_distance;
    center.y = settings.render_distance;
    center.z = settings.render_distance;

    chunk->pos_world.x = player_chunk_delta.x + chunk_tab_coordinates.x - settings.chunk_buf_radius;
    chunk->pos_world.y = player_chunk_delta.y + chunk_tab_coordinates.y - settings.chunk_buf_radius;
    chunk->pos_world.z = player_chunk_delta.z + chunk_tab_coordinates.z - settings.chunk_buf_radius;

    chunk->pos_wrap.x = fsl_mod_i32(chunk->pos_world.x + WORLD_RADIUS, WORLD_DIAMETER) - WORLD_RADIUS;
    chunk->pos_wrap.y = fsl_mod_i32(chunk->pos_world.y + WORLD_RADIUS, WORLD_DIAMETER) - WORLD_RADIUS;
    chunk->pos_wrap.z = fsl_mod_i32(chunk->pos_world.z + WORLD_RADIUS_VERTICAL,
            WORLD_DIAMETER_VERTICAL) - WORLD_RADIUS_VERTICAL;

    chunk->cti =
        chunk_tab_coordinates.x +
        chunk_tab_coordinates.y * settings.chunk_buf_diameter +
        chunk_tab_coordinates.z * settings.chunk_buf_layer;
    chunk->cpi = fsl_distance_v3u32(chunk_tab_coordinates, center);

    chunk->id =
        (u64)(chunk->pos_wrap.x & 0xffff) << 0x00 |
        (u64)(chunk->pos_wrap.y & 0xffff) << 0x10 |
        (u64)(chunk->pos_wrap.z & 0xffff) << 0x20;
}

void chunk_idle_func_internal(hhc_chunk *chunk,
        chunk_work_budget budget, hhc_chunk_receipt *receipt)
{
    (void)budget;
    (void)receipt;

    if (chunk->flag & FLAG_CHUNK_DIRTY_GENERATE)
        chunk->status = STATUS_CHUNK_GENERATING_BASE_TERRAIN;
    else if (chunk->flag & FLAG_CHUNK_DIRTY_MESH)
        chunk->status = STATUS_CHUNK_MESHING;
    else if (chunk->flag & FLAG_CHUNK_NON_AIR)
        chunk->status = STATUS_CHUNK_DONE;
    else
        chunk->status = STATUS_CHUNK_DONE_AIR;
}

void chunk_load_func_internal(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt)
{
#if MODE_INTERNAL_IMPORT_CHUNKS
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    (void)budget;

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, chunk->pos_wrap.x, chunk->pos_wrap.y, chunk->pos_wrap.z);

    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
        chunk_import_func_internal(path, chunk, receipt);
    else
        chunk->status = STATUS_CHUNK_LOADING + 1;
#else
    (void)chunk;
    (void)budget;
    (void)receipt;

    chunk->status = STATUS_CHUNK_LOADING + 1;
#endif /* MODE_INTERNAL_IMPORT_CHUNKS */
}

void chunk_generate_func_internal(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt)
{
    chunk_work_cost cost = 0;
    hhc_chunk_neighbors chunk_neighbors = {0};
    hhc_terrain_sample terrain = {0};
    v3i32 pos = {0};
    v2i32 pos_cheap_check = {0};
    b8 non_air = FALSE;

    chunk_neighbors = chunk_neighbors_get_internal(chunk);

    chunk_debug_chunk_gizmo_write_internal(chunk);

    fsl_noise_sampler_context_init(&chunk_sampler.sampler, &chunk_sampler.context,
            (f64)(chunk->pos_wrap.x * CHUNK_DIAMETER),
            (f64)(chunk->pos_wrap.y * CHUNK_DIAMETER),
            (f64)(chunk->pos_wrap.z * CHUNK_DIAMETER));

    pos.x = chunk->cursor % CHUNK_DIAMETER;
    pos.y = (chunk->cursor / CHUNK_DIAMETER) % CHUNK_DIAMETER;
    pos.z = chunk->cursor / CHUNK_LAYER;

    for (pos_cheap_check.y = 0; pos_cheap_check.y <= pos.y; ++pos_cheap_check.y)
    {
        for (pos_cheap_check.x = 0; pos_cheap_check.x < CHUNK_DIAMETER; ++pos_cheap_check.x)
        {
            cost += CHUNK_WORK_COST_CHEAP_CHECK;
            if (chunk->block[pos.z][pos_cheap_check.y][pos_cheap_check.x])
            {
                non_air = TRUE;
                goto begin_generation;
            }
        }
    }

begin_generation:

    /* `pos.x`, `pos.y` and `pos.z` reset at the end of their loops because they
     * should first pick up from where `chunk->cursor` left off last time. */
    fsl_noise_sampler_axis_init(&chunk_sampler.context, 2, pos.z);
    for (; pos.z < CHUNK_DIAMETER; ++pos.z, fsl_noise_sampler_axis_post_update(&chunk_sampler.context, 2))
    {
        fsl_noise_sampler_axis_pre_update(&chunk_sampler.context, 2);

        fsl_noise_sampler_axis_init(&chunk_sampler.context, 1, pos.y);
        for (; pos.y < CHUNK_DIAMETER; ++pos.y, fsl_noise_sampler_axis_post_update(&chunk_sampler.context, 1))
        {
            fsl_noise_sampler_axis_pre_update(&chunk_sampler.context, 1);
            cost += sampler_noise_axis_update_2d(&chunk_sampler.context, 1);

            fsl_noise_sampler_axis_init(&chunk_sampler.context, 0, pos.x);
            for (; pos.x < CHUNK_DIAMETER; ++pos.x, fsl_noise_sampler_axis_post_update(&chunk_sampler.context, 0))
            {
                fsl_noise_sampler_axis_pre_update(&chunk_sampler.context, 0);
                cost += sampler_noise_axis_update_2d(&chunk_sampler.context, 0);
                cost += sampler_noise_bake(&chunk_sampler.context);
                cost += terrain_shape(&terrain, &chunk_sampler.context);

                if (terrain.block_id)
                {
                    block_add_internal(&chunk_neighbors, pos.x, pos.y, pos.z, terrain.block_id);
                    non_air = TRUE;
                }

                if (cost >= (u32)budget)
                    goto finish_generation;
            }
            pos.x = 0;
        }

        if (!non_air)
        {
            chunk->flag |= FLAG_CHUNK_GENERATED;
            chunk->flag &= ~FLAG_CHUNK_DIRTY_GENERATE;
            chunk->status = STATUS_CHUNK_GENERATING_BASE_TERRAIN + 1;
            chunk->cursor = 0;

            chunk_debug_chunk_gizmo_write_internal(chunk);

            receipt->cost[CHUNK_RECEIPT_ITEM_GENERATE_TERRAIN] += cost;
            chunk->receipt.cost[CHUNK_RECEIPT_ITEM_GENERATE_TERRAIN] += cost;
            return;
        }

        pos.y = 0;
    }

finish_generation:

    chunk->cursor = pos.x + pos.y * CHUNK_DIAMETER + pos.z * CHUNK_LAYER;
    if (chunk->cursor >= CHUNK_VOLUME)
    {
        chunk->flag |= FLAG_CHUNK_GENERATED;
        chunk->flag &= ~FLAG_CHUNK_DIRTY_GENERATE;
        chunk->status = STATUS_CHUNK_GENERATING_BASE_TERRAIN + 1;
        chunk->cursor = 0;

        chunk_debug_chunk_gizmo_write_internal(chunk);
    }

    receipt->cost[CHUNK_RECEIPT_ITEM_GENERATE_TERRAIN] += cost;
    chunk->receipt.cost[CHUNK_RECEIPT_ITEM_GENERATE_TERRAIN] += cost;
}

void chunk_mesh_func_internal(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt)
{
    chunk_work_cost cost = 0;
    static u64 quad_buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u32 cur_buf = 0;

    u64 *quad_buf = &quad_buffer[cur_buf][0];
    u32 block = 0;
    u32 faces = 0;
    hhc_chunk_neighbors chunk_neighbors = {0};
    v3i64 i = {0};
    u32 j = 0;
    b8 should_render = FALSE;

    (void)budget;

    chunk_neighbors = chunk_neighbors_get_internal(chunk);

    chunk_debug_chunk_gizmo_write_internal(chunk);

    if (!(chunk->flag & FLAG_CHUNK_NON_AIR) ||
            !(chunk->flag & FLAG_CHUNK_DIRTY_MESH))
    {
        should_render = FALSE;
        cost = CHUNK_WORK_COST_MESH_AIR;
        goto finish_meshing;
    }

    j = 0;
    for (i.z = 0; i.z < CHUNK_DIAMETER; ++i.z)
    {
        for (i.y = 0; i.y < CHUNK_DIAMETER; ++i.y)
        {
            for (i.x = 0; i.x < CHUNK_DIAMETER; ++i.x)
            {
                block = chunk->block[i.z][i.y][i.x];
                if (block & MASK_BLOCK_ID)
                {
                    faces = block_faces_get_internal(&chunk_neighbors, i.x, i.y, i.z);
                    SET_BLOCK_FACES(chunk->block[i.z][i.y][i.x], faces);
                    if (faces & MASK_BLOCK_FACES)
                    {
                        should_render = TRUE;
                        quad_buf[j++] = block | faces |
                            (i.x << SHIFT_BLOCK_X) |
                            (i.y << SHIFT_BLOCK_Y) |
                            (i.z << SHIFT_BLOCK_Z);
                    }
                }
            }
        }
    }

    ++cur_buf;
    if (cur_buf >= BLOCK_BUFFERS_MAX)
        cur_buf = 0;

    cost = CHUNK_WORK_COST_MESH_NON_AIR;

finish_meshing:

    if (should_render)
    {
        chunk->flag |= FLAG_CHUNK_VISIBLE;
        chunk->status = STATUS_CHUNK_MESHING + 1;

        if (!chunk->mesh_deprecated.initialized)
        {
            chunk->mesh_deprecated.initialized = TRUE;

            glGenVertexArrays(1, &chunk->mesh_deprecated.vao);
            glGenBuffers(1, &chunk->mesh_deprecated.quad_buf);
            glGenBuffers(1, &chunk->mesh_deprecated.light_buf);

            glBindVertexArray(chunk->mesh_deprecated.vao);
            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.quad_buf);
            glBufferData(GL_ARRAY_BUFFER, j * sizeof(u64), quad_buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)sizeof(u32));

            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.light_buf);
            glBufferData(GL_ARRAY_BUFFER, j * sizeof(u8), chunk->light, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(u8), (void*)0);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.quad_buf);
            glBufferData(GL_ARRAY_BUFFER, j * sizeof(u64), quad_buf, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        chunk->mesh_deprecated.buf_len = j;
    }
    else
    {
        chunk->flag &= ~FLAG_CHUNK_VISIBLE;
        chunk->status = STATUS_CHUNK_FINISHING;

        if (chunk->mesh_deprecated.initialized)
        {
            chunk->mesh_deprecated.initialized = FALSE;
            glDeleteBuffers(1, &chunk->mesh_deprecated.light_buf);
            glDeleteBuffers(1, &chunk->mesh_deprecated.quad_buf);
            glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
        }
    }

    chunk_debug_chunk_gizmo_write_internal(chunk);

    receipt->cost[CHUNK_RECEIPT_ITEM_MESH] += cost;
    chunk->receipt.cost[CHUNK_RECEIPT_ITEM_MESH] += cost;
}

void chunk_light_func_internal(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt)
{
    chunk_work_cost cost = 0;
    static u8 light_buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    u8 *light_buf = &light_buffer[cur_buf][0];
    u32 block = 0;
    u8 light = 0;
    hhc_chunk_neighbors chunk_neighbors = {0};
    v3u64 i = {0};
    u32 j = 0;
    b8 should_render = TRUE;

    (void)budget;

    chunk_neighbors = chunk_neighbors_get_internal(chunk);

    chunk_debug_chunk_gizmo_write_internal(chunk);

    if (!(chunk->flag & FLAG_CHUNK_NON_AIR) ||
            !(chunk->flag & FLAG_CHUNK_DIRTY_MESH) ||
            !chunk->mesh_deprecated.initialized)
    {
        cost = CHUNK_WORK_COST_LIGHT_AIR;
        should_render = FALSE;
        goto finish_lighting;
    }

    j = 0;
    for (i.z = 0; i.z < CHUNK_DIAMETER; ++i.z)
    {
        for (i.y = 0; i.y < CHUNK_DIAMETER; ++i.y)
        {
            for (i.x = 0; i.x < CHUNK_DIAMETER; ++i.x)
            {
                block = chunk->block[i.z][i.y][i.x];
                if (block & MASK_BLOCK_ID)
                {
                    light = 15;
                    SET_BLOCK_LIGHT(chunk->light[i.z][i.y][i.x], light);
                    light_buf[j++] = light;
                }
            }
        }
    }

    ++cur_buf;
    if (cur_buf >= BLOCK_BUFFERS_MAX)
        cur_buf = 0;

    cost = CHUNK_WORK_COST_LIGHT_NON_AIR;

finish_lighting:

    if (should_render)
    {
        chunk->status = STATUS_CHUNK_LIGHTING + 1;

        glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh_deprecated.light_buf);
        glBufferData(GL_ARRAY_BUFFER, j * sizeof(u8), light_buf, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        chunk->mesh_deprecated.buf_len = j;
    }
    else
    {
        chunk->flag &= ~FLAG_CHUNK_VISIBLE;
        chunk->status = STATUS_CHUNK_FINISHING;

        if (chunk->mesh_deprecated.initialized)
        {
            chunk->mesh_deprecated.initialized = FALSE;
            glDeleteBuffers(1, &chunk->mesh_deprecated.light_buf);
            glDeleteBuffers(1, &chunk->mesh_deprecated.quad_buf);
            glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
        }
    }

    chunk_debug_chunk_gizmo_write_internal(chunk);

    receipt->cost[CHUNK_RECEIPT_ITEM_MESH] += cost;
    chunk->receipt.cost[CHUNK_RECEIPT_ITEM_MESH] += cost;
}

void chunk_ao_func_internal(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt)
{
    chunk_work_cost cost = 0;

    (void)budget;

    chunk_debug_chunk_gizmo_write_internal(chunk);

    /* TODO: do ao stuff */

    chunk->status = STATUS_CHUNK_AO + 1;

    chunk_debug_chunk_gizmo_write_internal(chunk);

    receipt->cost[CHUNK_RECEIPT_ITEM_MESH] += cost;
    chunk->receipt.cost[CHUNK_RECEIPT_ITEM_MESH] += cost;
}

void chunk_finish_func_internal(hhc_chunk *chunk, chunk_work_budget budget,
        hhc_chunk_receipt *receipt)
{
    (void)budget;

    if (chunk->flag & FLAG_CHUNK_NON_AIR)
    {
        if (chunk->flag & FLAG_CHUNK_VISIBLE)
            chunk->status = STATUS_CHUNK_DONE;
        else
            chunk->status = STATUS_CHUNK_DONE_NON_VISIBLE;
    }
    else
        chunk->status = STATUS_CHUNK_DONE_AIR;

#if MODE_INTERNAL_EXPORT_CHUNKS
    if (!(chunk->flag & FLAG_CHUNK_IMPORTED))
        chunk_export_func_internal(chunk, &receipt);
#else
    (void)receipt;
#endif /* MODE_INTERNAL_EXPORT_CHUNKS */

    chunk_debug_chunk_gizmo_write_internal(chunk);

    chunk->flag &= ~(FLAG_CHUNK_DIRTY_GENERATE | FLAG_CHUNK_DIRTY_MESH | FLAG_CHUNK_IMPORTED);
}

void chunk_export_func_internal(hhc_chunk *chunk, hhc_chunk_receipt *receipt)
{
    chunk_work_cost cost = 0;
    fsl_fs_path path[FSL_PATH_CAP] = {0};
    static u16 buf[CHUNK_VOLUME] = {0};
    u32 *blocks = (u32*)chunk->block;
    u32 i = 0;
    u32 j = 0;
    u32 rle = 0; /* run-length */

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, chunk->pos_wrap.x, chunk->pos_wrap.y, chunk->pos_wrap.z);

    if (!(chunk->flag & FLAG_CHUNK_NON_AIR))
    {
        buf[0] = 0 | FLAG_BLOCK_RLE;
        buf[1] = CHUNK_VOLUME;
        j = 2;
        cost = CHUNK_WORK_COST_EXPORT_AIR;
        goto finish_export;
    }

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

    cost = CHUNK_WORK_COST_EXPORT_NON_AIR;

finish_export:

    receipt->cost[CHUNK_RECEIPT_ITEM_EXPORT] += cost;
    chunk->receipt.cost[CHUNK_RECEIPT_ITEM_EXPORT] += cost;

    fsl_write_file(path, j * sizeof(u16), buf, TRUE, FALSE);
}

void chunk_import_func_internal(const fsl_fs_path *path, hhc_chunk *chunk,
        hhc_chunk_receipt *receipt)
{
    chunk_work_cost cost = 0;
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

    chunk->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_IMPORTED | FLAG_CHUNK_DIRTY_MESH | FLAG_CHUNK_GENERATED;
    chunk->status = STATUS_CHUNK_MESHING;
    chunk->pos_wrap.x = pos_cache[0];
    chunk->pos_wrap.y = pos_cache[1];
    chunk->pos_wrap.z = pos_cache[2];

    file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    i = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(buf, 1, i, file);
    fclose(file);

    if (i == 2 * sizeof(u16) && buf[0] & FLAG_BLOCK_RLE && !buf[1])
    {
        cost = CHUNK_WORK_COST_IMPORT_AIR;
        goto finish_import;
    }

    chunk->flag |= FLAG_CHUNK_NON_AIR;

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

    cost = CHUNK_WORK_COST_IMPORT_NON_AIR;

finish_import:

    receipt->cost[CHUNK_RECEIPT_ITEM_IMPORT] += cost;
    chunk->receipt.cost[CHUNK_RECEIPT_ITEM_IMPORT] += cost;
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
    u32 seed = 0;
    u32 end = chunk_buf.cursor;
    b8 pushed = FALSE;

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
                glDeleteBuffers(1, &chunk->mesh_deprecated.light_buf);
                glDeleteBuffers(1, &chunk->mesh_deprecated.quad_buf);
                glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
            }
            *chunk = nochunk;
            chunk->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY_GENERATE | FLAG_CHUNK_DIRTY_MESH;
            chunk->status = STATUS_CHUNK_LOADING;

            chunk_pos_set_internal(chunk, player_chunk_delta, chunk_tab_coordinates);

            seed = chunk->pos_wrap.x ^ 823948;
            seed ^= seed << 12;
            seed ^= chunk->pos_wrap.y ^ 323423;
            seed ^= seed << 15;
            seed ^= chunk->pos_wrap.z ^ 211534;
            seed ^= seed << 18;
            seed = (u32)fsl_hash_fnv1a_u64(&seed, sizeof(u32));

            chunk->debug_color_bias = (i8)fsl_rand_u32(seed) * CHUNK_DEBUG_COLOR_BIAS_INFLUENCE;

            chunk_tab.p[index] = chunk;
            chunk_debug_chunk_gizmo_write_internal(chunk);
            pushed = TRUE;
        }

        ++chunk_buf.cursor;
        if (chunk_buf.cursor >= chunk_order.chunks_max)
            chunk_buf.cursor = 0;

        if (pushed)
            return;
    } while (chunk_buf.cursor != end);

    LOGWARNING(FSL_ERR_BUFFER_FULL,
            FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "Failed to Push to `chunk_buf`, Buffer Full\n");
}

void chunk_buf_pop_internal(hhc_chunk *chunk)
{
    u32 index_popped = chunk - chunk_buf.p;

    if (chunk->mesh_deprecated.initialized)
    {
        chunk->mesh_deprecated.initialized = FALSE;
        glDeleteBuffers(1, &chunk->mesh_deprecated.light_buf);
        glDeleteBuffers(1, &chunk->mesh_deprecated.quad_buf);
        glDeleteVertexArrays(1, &chunk->mesh_deprecated.vao);
    }

    chunk->flag = 0;
    chunk_debug_chunk_gizmo_write_internal(chunk);

    if (chunk_buf.cursor > index_popped)
        chunk_buf.cursor = index_popped;
    chunk_tab.p[chunk->cti] = NULL;
}

void chunk_buf_dump_internal(void)
{
    hhc_chunk *chunk = NULL;
    u32 i = 0;
    u32 end = chunk_order.chunks_max;

    if (!chunk_tab.p)
        return;

    for (; i < end; ++i)
    {
        chunk = chunk_tab.p[chunk_order.p[i]];
        if (chunk)
        {
            chunk->flag = 0;
            chunk->status = 0;
            chunk_debug_chunk_gizmo_write_internal(chunk);
            chunk_tab.p[chunk_order.p[i]] = NULL;
        }
    }

    chunk_buf.cursor = 0;
}

void chunk_tab_shift_internal(v3i32 player_chunk, v3i32 *player_chunk_delta)
{
    u32 index_bound_start = 0;
    u32 index_bound_end = 0;
    v3u32 index_offsets_positive = {0};
    v3u32 index_offsets_negative = {0};
    u32 *index_bound = NULL;
    u32 *index_bound_inv = NULL;
    u32 *index_offset = NULL;
    u32 *index_offset_inv = NULL;
    v3u32 mirror_indices = {0};
    u32 *mirror_index = NULL;
    u32 target_index = 0;
    u32 tail_index = 0;
    b8 is_on_edge = FALSE;
    v3u32 i = {0};
    u32 j = 0;
    v4u32 start = {0};
    u32 end = 0;
    u32 *coordinate = NULL;
    v3i32 DELTA = {0};
    u8 AXIS = 0;
    i8 INCREMENT = 0;

    index_bound_start = 0;
    index_bound_end = settings.chunk_buf_diameter - 1;
    index_offsets_positive.x = 1;
    index_offsets_positive.y = settings.chunk_buf_diameter;
    index_offsets_positive.z = settings.chunk_buf_layer;
    index_offsets_negative.x = -1;
    index_offsets_negative.y = -settings.chunk_buf_diameter;
    index_offsets_negative.z = -settings.chunk_buf_layer;
    end = settings.chunk_buf_diameter;

shift_whichever_axis_that_needs_shifting:

    DELTA.x = player_chunk.x - player_chunk_delta->x;
    DELTA.y = player_chunk.y - player_chunk_delta->y;
    DELTA.z = player_chunk.z - player_chunk_delta->z;

    if (DELTA.x > 0)
    {
        AXIS = 1;
        INCREMENT = 1;
        index_offset = &index_offsets_negative.x;
        index_offset_inv = &index_offsets_positive.x;
    }
    else if (DELTA.x < 0)
    {
        AXIS = 1;
        INCREMENT = -1;
        index_offset = &index_offsets_positive.x;
        index_offset_inv = &index_offsets_negative.x;
    }
    else if (DELTA.y > 0)
    {
        AXIS = 2;
        INCREMENT = 1;
        index_offset = &index_offsets_negative.y;
        index_offset_inv = &index_offsets_positive.y;
    }
    else if (DELTA.y < 0)
    {
        AXIS = 2;
        INCREMENT = -1;
        index_offset = &index_offsets_positive.y;
        index_offset_inv = &index_offsets_negative.y;
    }
    else if (DELTA.z > 0)
    {
        AXIS = 3;
        INCREMENT = 1;
        index_offset = &index_offsets_negative.z;
        index_offset_inv = &index_offsets_positive.z;
    }
    else if (DELTA.z < 0)
    {
        AXIS = 3;
        INCREMENT = -1;
        index_offset = &index_offsets_positive.z;
        index_offset_inv = &index_offsets_negative.z;
    }

    switch (AXIS)
    {
        case 1:
            DELTA.x -= INCREMENT;
            player_chunk_delta->x += INCREMENT;
            mirror_index = &mirror_indices.x;
            coordinate = &i.x;
            break;

        case 2:
            DELTA.y -= INCREMENT;
            player_chunk_delta->y += INCREMENT;
            mirror_index = &mirror_indices.y;
            coordinate = &i.y;
            break;

        case 3:
            DELTA.z -= INCREMENT;
            player_chunk_delta->z += INCREMENT;
            mirror_index = &mirror_indices.z;
            coordinate = &i.z;
            break;

        default:
            *player_chunk_delta = player_chunk;
            return;
    }

    switch (INCREMENT)
    {
        case -1:
            index_bound = &index_bound_end;
            index_bound_inv = &index_bound_start;
            start.x = settings.chunk_buf_diameter - 1;
            start.y = settings.chunk_buf_diameter - 1;
            start.z = settings.chunk_buf_diameter - 1;
            start.w = settings.chunk_buf_volume - 1;
            break;

        case 1:
            index_bound = &index_bound_start;
            index_bound_inv = &index_bound_end;
            start.x = 0;
            start.y = 0;
            start.z = 0;
            start.w = 0;
            break;

        default:
            *player_chunk_delta = player_chunk;
            return;
    }

    /* ---- mark chunks on-edge --------------------------------------------- */

    j = 0;
    for (i.z = 0; i.z < end; ++i.z)
    {
        for (i.y = 0; i.y < end; ++i.y)
        {
            for (i.x = 0; i.x < end; ++i.x, ++j)
            {
                if (!chunk_tab.p[j])
                    continue;

                is_on_edge = *coordinate == *index_bound || !chunk_tab.p[j + *index_offset];
                if (is_on_edge)
                {
                    mirror_indices.x = j + settings.chunk_buf_diameter - 1 - i.x * 2;
                    mirror_indices.y = i.z * settings.chunk_buf_layer +
                        (settings.chunk_buf_diameter - 1 - i.y) * settings.chunk_buf_diameter + i.x;
                    mirror_indices.z = (settings.chunk_buf_diameter - 1 - i.z) * settings.chunk_buf_layer +
                        i.y * settings.chunk_buf_diameter + i.x;

                    chunk_tab.p[j]->flag &= ~(FLAG_CHUNK_LOADED | FLAG_CHUNK_VISIBLE);
                    chunk_tab.p[j]->status = 0;
                    chunk_debug_chunk_gizmo_write_internal(chunk_tab.p[j]);

                    if (chunk_tab.p[*mirror_index])
                        chunk_tab.p[*mirror_index]->flag |= FLAG_CHUNK_EDGE;
                }
            }
        }
    }

    /* ---- shift `chunk_tab` ----------------------------------------------- */

    j = start.w;
    for (i.z = start.z; i.z < end; i.z += INCREMENT)
    {
        for (i.y = start.y; i.y < end; i.y += INCREMENT)
        {
            for (i.x = start.x; i.x < end; i.x += INCREMENT, j += INCREMENT)
            {
                if (!chunk_tab.p[j])
                    continue;

                target_index = *coordinate == *index_bound_inv ? j : j + *index_offset_inv;
                tail_index = *coordinate == *index_bound ? j : j + *index_offset;
                is_on_edge = *coordinate == *index_bound || !chunk_tab.p[j + *index_offset];

                chunk_tab.p[j] = chunk_tab.p[target_index];
                if (chunk_tab.p[j])
                {
                    chunk_pos_set_internal(chunk_tab.p[j], *player_chunk_delta, i);

                    if (chunk_tab.p[j]->flag & FLAG_CHUNK_EDGE)
                    {
                        chunk_tab.p[j]->flag &= ~FLAG_CHUNK_EDGE;
                        chunk_tab.p[target_index] = NULL;
                    }

                    chunk_debug_chunk_gizmo_write_internal(chunk_tab.p[j]);
                }
            }
        }
    }

    if (DELTA.x || DELTA.y || DELTA.z)
        goto shift_whichever_axis_that_needs_shifting;
}

void chunk_scheduler_update_internal(void)
{
    chunk_work_budget budget = settings.frame_budget;
    hhc_chunk_receipt noreceipt = {0};
    hhc_chunk_receipt receipt = {0};
    u32 i = 0;
    u32 end = chunk_order.chunks_max;
    hhc_chunk *chunk = NULL;
    hhc_chunk_bucket *bucket = NULL;
    u32 bucket_end = 0;

    if (chunk_sched.count >= end)
        goto pop;

    if (budget <= 0)
        return;

    for (i = 0; i < end && chunk_sched.count < end && budget > 0; ++i)
    {
        chunk = chunk_tab.p[chunk_order.p[i]];
        if (chunk)
        {
            bucket = &chunk_sched.bucket[chunk->cpi];

            if (
                    (chunk->flag & FLAG_CHUNK_DIRTY_GENERATE ||
                     chunk->flag & FLAG_CHUNK_DIRTY_MESH) &&
                    !chunk_sched.p[bucket->push])
            {
                budget -= chunk_scheduler_push_internal(chunk);
            }
        }
        budget -= CHUNK_WORK_COST_SCAN;
    }

pop:

    if (!chunk_sched.count || budget <= 0)
        return;

    end = chunk_sched.buckets_max;
    for (i = 0; i < end && chunk_sched.count && budget > 0; ++i)
    {
        bucket = &chunk_sched.bucket[i];

        if (bucket->count)
        {
            bucket_end = bucket->pos + bucket->len;
            do
            {
                chunk = chunk_sched.p[bucket->pop];

                if (chunk->cpi != i ||
                        !(chunk->flag & FLAG_CHUNK_QUEUED))
                {
                    chunk->flag &= ~FLAG_CHUNK_QUEUED;
                    chunk_sched.p[bucket->pop] = NULL;
                    --chunk_sched.count;
                    --bucket->count;
                    ++bucket->pop;
                    if (bucket->pop >= bucket_end)
                        bucket->pop = bucket->pos;
                    continue;
                }

                if (!(chunk->flag & FLAG_CHUNK_LOADED))
                {
                    budget -= chunk_scheduler_pop_internal(chunk);
                    continue;
                }

                chunk_work_func_internal[chunk->status](chunk, budget, &receipt);

                chunk_receipt_evaluate(&receipt, chunk->cpi);
                chunk_receipt_evaluate(&chunk->receipt, chunk->cpi);
                budget -= receipt.total;
                receipt = noreceipt;

                if (!chunk->status || chunk->status > STATUS_CHUNK_FINISHING)
                {
                    budget -= chunk_scheduler_pop_internal(chunk);
                }
            } while (bucket->pop != bucket->push && bucket->count && budget > 0);
        }
    }

    chunk_sched.priority = i;
}

chunk_work_cost chunk_scheduler_push_internal(hhc_chunk *chunk)
{
    hhc_chunk_bucket *bucket = &chunk_sched.bucket[chunk->cpi];
    u32 bucket_end = bucket->pos + bucket->len;
    chunk->flag |= FLAG_CHUNK_QUEUED;
    chunk_sched.p[bucket->push] = chunk;
    ++chunk_sched.count;
    ++bucket->count;
    ++bucket->push;
    if (bucket->push >= bucket_end)
        bucket->push = bucket->pos;
    return CHUNK_WORK_COST_PUSH;
}

chunk_work_cost chunk_scheduler_pop_internal(hhc_chunk *chunk)
{
    hhc_chunk_bucket *bucket = &chunk_sched.bucket[chunk->cpi];
    u32 bucket_end = bucket->pos + bucket->len;
    chunk->flag &= ~FLAG_CHUNK_QUEUED;
    chunk_sched.p[bucket->pop] = NULL;
    --chunk_sched.count;
    --bucket->count;
    ++bucket->pop;
    if (bucket->pop >= bucket_end)
        bucket->pop = bucket->pos;
    return CHUNK_WORK_COST_POP;
}

/* ---- section: implementation: block -------------------------------------- */

u32 block_faces_get_internal(hhc_chunk_neighbors *chunk_neighbors, i32 x, i32 y, i32 z)
{
    hhc_chunk_neighbors *cn = chunk_neighbors;
    u32 faces = 0;

    if (x == CHUNK_DIAMETER - 1)
    {
        if (!cn->px || !cn->px->block[z][y][0])
            faces |= FLAG_BLOCK_FACE_PX;
    }
    else if (!cn->ch->block[z][y][x + 1])
        faces |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        if (!cn->nx || !cn->nx->block[z][y][CHUNK_DIAMETER - 1])
            faces |= FLAG_BLOCK_FACE_NX;
    }
    else if (!cn->ch->block[z][y][x - 1])
        faces |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        if (!cn->py || !cn->py->block[z][0][x])
            faces |= FLAG_BLOCK_FACE_PY;
    }
    else if (!cn->ch->block[z][y + 1][x])
        faces |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        if (!cn->ny || !cn->ny->block[z][CHUNK_DIAMETER - 1][x])
            faces |= FLAG_BLOCK_FACE_NY;
    }
    else if (!cn->ch->block[z][y - 1][x])
        faces |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        if (!cn->pz || !cn->pz->block[0][y][x])
            faces |= FLAG_BLOCK_FACE_PZ;
    }
    else if (!cn->ch->block[z + 1][y][x])
        faces |= FLAG_BLOCK_FACE_PZ;

    if (z == 0)
    {
        if (!cn->nz || !cn->nz->block[CHUNK_DIAMETER - 1][y][x])
            faces |= FLAG_BLOCK_FACE_NZ;
    }
    else if (!cn->ch->block[z - 1][y][x])
        faces |= FLAG_BLOCK_FACE_NZ;

    return faces;
}

block_hit block_hit_get(v3f64 origin, f64 start_x, f64 start_y, f64 start_z,
        f64 end_x, f64 end_y, f64 end_z, f64 distance_max)
{
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

        hit.block = &chunk->block[z][y][x];
        hit.hit = TRUE;
        break;
    }

    return hit;
}

void block_place(block_hit hit, enum block_id block_id)
{
    u32 index = chunk_tab.index;
    hhc_chunk_neighbors cn = {0};

    if (!hit.hit || (hit.normal.x == 0.0f && hit.normal.y == 0.0f && hit.normal.z == 0.0f))
        return;

    /* canonicalize block position */
    hit.pos.x -= chunk_tab.p[index]->pos_world.x * CHUNK_DIAMETER;
    hit.pos.y -= chunk_tab.p[index]->pos_world.y * CHUNK_DIAMETER;
    hit.pos.z -= chunk_tab.p[index]->pos_world.z * CHUNK_DIAMETER;

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

    cn = chunk_neighbors_get_internal(chunk_tab.p[index]);
    if (cn.ch->block[hit.pos.z][hit.pos.y][hit.pos.x] || !block_id)
        return;

    block_add_internal(&cn, hit.pos.x, hit.pos.y, hit.pos.z, block_id);
}

void block_break(block_hit hit)
{
    u32 index = chunk_tab.index;
    hhc_chunk_neighbors cn = {0};

    if (!hit.hit)
        return;

    /* canonicalize block position */
    hit.pos.x -= chunk_tab.p[index]->pos_world.x * CHUNK_DIAMETER;
    hit.pos.y -= chunk_tab.p[index]->pos_world.y * CHUNK_DIAMETER;
    hit.pos.z -= chunk_tab.p[index]->pos_world.z * CHUNK_DIAMETER;

    /* get the chunk the new block index is in and make sure it's within bounds */
    index += (i32)floorf((f32)hit.pos.x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)hit.pos.y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)hit.pos.z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume)
        return;

    hit.pos.x = fsl_mod_i32(hit.pos.x, CHUNK_DIAMETER);
    hit.pos.y = fsl_mod_i32(hit.pos.y, CHUNK_DIAMETER);
    hit.pos.z = fsl_mod_i32(hit.pos.z, CHUNK_DIAMETER);

    cn = chunk_neighbors_get_internal(chunk_tab.p[index]);
    if (!cn.ch->block[hit.pos.z][hit.pos.y][hit.pos.x])
        return;

    block_remove_internal(&cn, hit.pos.x, hit.pos.y, hit.pos.z);
}

void block_add_internal(hhc_chunk_neighbors *chunk_neighbors,
        i32 x, i32 y, i32 z, enum block_id block_id)
{
    hhc_chunk_neighbors *cn = chunk_neighbors;

    cn->ch->flag |= FLAG_CHUNK_DIRTY_MESH | FLAG_CHUNK_NON_AIR;
    SET_BLOCK_ID(cn->ch->block[z][y][x], block_id);
    SET_BLOCK_LIGHT(cn->ch->light[z][y][x], 15);

    if (x == CHUNK_DIAMETER - 1 && cn->px && cn->px->block[z][y][0])
        cn->px->flag |= FLAG_CHUNK_DIRTY_MESH;
    else if (x == 0 && cn->nx && cn->nx->block[z][y][CHUNK_DIAMETER - 1])
        cn->nx->flag |= FLAG_CHUNK_DIRTY_MESH;

    if (y == CHUNK_DIAMETER - 1 && cn->py && cn->py->block[z][0][x])
        cn->py->flag |= FLAG_CHUNK_DIRTY_MESH;
    else if (y == 0 && cn->ny && cn->ny->block[z][CHUNK_DIAMETER - 1][x])
        cn->ny->flag |= FLAG_CHUNK_DIRTY_MESH;

    if (z == CHUNK_DIAMETER - 1 && cn->pz && cn->pz->block[0][y][x])
        cn->pz->flag |= FLAG_CHUNK_DIRTY_MESH;
    else if (z == 0 && cn->nz && cn->nz->block[CHUNK_DIAMETER - 1][y][x])
        cn->nz->flag |= FLAG_CHUNK_DIRTY_MESH;

    block_evaluate_internal(cn, x, y, z, block_id);
}

void block_remove_internal(hhc_chunk_neighbors *chunk_neighbors,
        i32 x, i32 y, i32 z)
{
    hhc_chunk_neighbors *cn = chunk_neighbors;
    cn->ch->flag |= FLAG_CHUNK_DIRTY_MESH;
    cn->ch->block[z][y][x] = 0;

    if (x == CHUNK_DIAMETER - 1 && cn->px && cn->px->block[z][y][0])
        cn->px->flag |= FLAG_CHUNK_DIRTY_MESH;
    else if (x == 0 && cn->nx && cn->nx->block[z][y][CHUNK_DIAMETER - 1])
        cn->nx->flag |= FLAG_CHUNK_DIRTY_MESH;

    if (y == CHUNK_DIAMETER - 1 && cn->py && cn->py->block[z][0][x])
        cn->py->flag |= FLAG_CHUNK_DIRTY_MESH;
    else if (y == 0 && cn->ny && cn->ny->block[z][CHUNK_DIAMETER - 1][x])
        cn->ny->flag |= FLAG_CHUNK_DIRTY_MESH;

    if (z == CHUNK_DIAMETER - 1 && cn->pz && cn->pz->block[0][y][x])
        cn->pz->flag |= FLAG_CHUNK_DIRTY_MESH;
    else if (z == 0 && cn->nz && cn->nz->block[CHUNK_DIAMETER - 1][y][x])
        cn->nz->flag |= FLAG_CHUNK_DIRTY_MESH;
}

void block_evaluate_internal(hhc_chunk_neighbors *chunk_neighbors,
        i32 x, i32 y, i32 z, enum block_id block_id)
{
    hhc_chunk_neighbors *cn = chunk_neighbors;

    if (z == CHUNK_DIAMETER - 1)
    {
        if (cn->pz && cn->pz->block[0][y][x])
        {
            cn->pz->flag |= FLAG_CHUNK_DIRTY_MESH;

            if (GET_BLOCK_ID(cn->ch->block[z][y][x]) == BLOCK_GRASS)
                SET_BLOCK_ID(cn->ch->block[z][y][x], BLOCK_DIRT);
        }
    }

    if (z == 0)
    {
        if (cn->nz && cn->nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            cn->nz->flag |= FLAG_CHUNK_DIRTY_MESH;

            if (GET_BLOCK_ID(cn->nz->block[CHUNK_DIAMETER - 1][y][x]) == BLOCK_GRASS)
                SET_BLOCK_ID(cn->nz->block[CHUNK_DIAMETER - 1][y][x], BLOCK_DIRT);
        }
    }
    else if (GET_BLOCK_ID(cn->ch->block[z - 1][y][x]) == BLOCK_GRASS)
    {
        cn->ch->flag |= FLAG_CHUNK_DIRTY_MESH;

        SET_BLOCK_ID(cn->ch->block[z - 1][y][x], BLOCK_DIRT);
    }
}

u32 *block_resolved_get(hhc_chunk *chunk, i32 x, i32 y, i32 z)
{
    x = fsl_mod_i32(x, CHUNK_DIAMETER);
    y = fsl_mod_i32(y, CHUNK_DIAMETER);
    z = fsl_mod_i32(z, CHUNK_DIAMETER);
    return &chunk->block[z][y][x];
}
