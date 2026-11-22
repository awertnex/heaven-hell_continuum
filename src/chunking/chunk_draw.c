#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shader_types.h"

#include "../h/main.h"
#include "../settings/settings.h"
#include "../h/diagnostics.h"

#include "chunking.h"
#include "chunking_internal.h"

fsl_mem_arena memory_arena_chunk_draw_internal = {0};
hhc_chunk_draw chunk_draw = {0};

u32 chunk_draw_init_internal(void)
{
    if (fsl_mem_arena_init(&memory_arena_chunk_draw_internal,
                "chunk_draw_init_internal().memory_arena_chunk_draw_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunk_draw_internal, &chunk_draw.handle,
                sizeof(hhc_chunk_mesh),
                "chunk_draw_init_internal().chunk_draw.handle") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    chunk_draw.p = fsl_mem_handle_get(chunk_draw.handle);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void chunk_draw_free_internal(void)
{
    fsl_mem_arena_free(&memory_arena_chunk_draw_internal,
            "chunk_draw_free_internal().memory_arena_chunk_draw_internal");
}

void chunk_draw_push(hhc_chunk *chunk)
{
    chunk_draw.p = fsl_mem_handle_get(chunk_draw.handle);
}

void chunk_draw_update_internal(void)
{
    chunk_draw.p = fsl_mem_handle_get(chunk_draw.handle);
}

void chunk_draw_render(fsl_camera *camera,
        v3f32 sun_rotation, v3f32 sky_light, v3f32 moon_light)
{
}
