#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shader_types.h"

#include "../h/main.h"
#include "../settings/settings.h"
#include "../h/diagnostics.h"

#include "chunking.h"
#include "chunking_internal.h"

hhc_chunk_draw chunk_draw = {0};

static struct /* chunk_draw_core */
{
    fsl_mem_arena memory_arena;
    b8 unit_quad_initialized;
    GLuint unit_quad;
} chunk_draw_core = {0};

u32 chunk_draw_init_internal(void)
{
    u32 unit_quad_len = 4;

    if (fsl_mem_arena_init(&chunk_draw_core.memory_arena,
                "chunk_draw_init_internal().chunk_draw_core.memory_arena") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&chunk_draw_core.memory_arena, &chunk_draw.handle,
                chunk_order.chunks_max * sizeof(hhc_chunk_mesh),
                "chunk_draw_init_internal().chunk_draw.handle") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    chunk_draw.p = fsl_mem_handle_get(chunk_draw.handle);
    chunk_draw.len = chunk_order.chunks_max;

    if (!chunk_draw_core.unit_quad_initialized)
    {
        chunk_draw_core.unit_quad_initialized = TRUE;
        glGenBuffers(1, &chunk_draw_core.unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, chunk_draw_core.unit_quad);
        glBufferData(GL_ARRAY_BUFFER, unit_quad_len * 5 * sizeof(f32), NULL, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void chunk_draw_free_internal(void)
{
    hhc_chunk_draw nochunk_draw = {0};
    u32 i = 0;

    if (chunk_draw.p)
    {
        for (; i < chunk_order.chunks_max && i < chunk_draw.len; ++i)
            if (chunk_draw.p[i].initialized)
                chunk_draw_pop_internal(chunk_draw.p[i].chunk);
    }

    if (chunk_draw_core.unit_quad_initialized)
    {
        chunk_draw_core.unit_quad_initialized = FALSE;
        glDeleteBuffers(1, &chunk_draw_core.unit_quad);
    }

    fsl_mem_arena_free(&chunk_draw_core.memory_arena,
            "chunk_draw_free_internal().chunk_draw_core.memory_arena");

    chunk_draw = nochunk_draw;
}

void chunk_draw_push_internal(hhc_chunk *chunk, u32 *quad_buf, u8 *light_buf,
        u32 buf_len)
{
    hhc_chunk_mesh *mesh = NULL;
    u32 end = chunk_draw.cursor;
    b8 pushed = FALSE;

    if (chunk->mesh && chunk->mesh->initialized)
    {
        if (quad_buf)
        {
            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh->quad_buf);
            glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u32), quad_buf, GL_DYNAMIC_DRAW);
        }
        if (light_buf)
        {
            glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh->light_buf);
            glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u8), light_buf, GL_DYNAMIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        chunk->mesh->buf_len = buf_len;
        return;
    }

    if (chunk_draw.count >= chunk_order.chunks_max)
        goto cleanup;

    do
    {
        if (!(chunk_draw.p[chunk_draw.cursor].initialized))
        {
            mesh = &chunk_draw.p[chunk_draw.cursor];

            mesh->initialized = TRUE;
            glGenVertexArrays(1, &mesh->vao);
            glGenBuffers(1, &mesh->quad_buf);
            glGenBuffers(1, &mesh->light_buf);

            glBindVertexArray(mesh->vao);

            glBindBuffer(GL_ARRAY_BUFFER, chunk_draw_core.unit_quad);

            glBindBuffer(GL_ARRAY_BUFFER, mesh->quad_buf);
            glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u32), quad_buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(u32), (void*)0);
            glVertexAttribDivisor(2, 1);

            glBindBuffer(GL_ARRAY_BUFFER, mesh->light_buf);
            glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u8), light_buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(3);
            glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(u8), (void*)0);
            glVertexAttribDivisor(3, 1);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            mesh->buf_len = buf_len;
            mesh->chunk = chunk;
            chunk->mesh = mesh;
            ++chunk_draw.count;

            pushed = TRUE;
        }

        ++chunk_draw.cursor;
        if (chunk_draw.cursor >= chunk_draw.len)
            chunk_draw.cursor = 0;

        if (pushed)
            return;
    } while (chunk_draw.cursor != end);

cleanup:

    LOGWARNING(FSL_ERR_BUFFER_FULL,
            FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "Failed to Push to `chunk_draw`, Buffer Full\n");
}

void chunk_draw_pop_internal(hhc_chunk *chunk)
{
    hhc_chunk_mesh nomesh = {0};
    hhc_chunk_mesh *mesh = chunk->mesh;

    if (mesh && mesh->initialized)
    {
        mesh->initialized = FALSE;
        glDeleteBuffers(1, &mesh->light_buf);
        glDeleteBuffers(1, &mesh->quad_buf);
        glDeleteVertexArrays(1, &mesh->vao);

        chunk->mesh = NULL;
        *mesh = nomesh;
        --chunk_draw.count;
    }
}

void chunk_draw_render(fsl_camera *camera,
        v3f32 sun_rotation, v3f32 sky_light, v3f32 moon_light)
{
}
