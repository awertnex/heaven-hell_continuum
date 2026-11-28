#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"

#include "../h/diagnostics.h"
#include "../h/main.h"

#include "chunking.h"
#include "chunking_internal.h"

#include <stddef.h>

typedef struct hhc_draw_arrays_indirect_command
{
    u32 count;
    u32 instance_count;
    u32 first;
    u32 base_instance;
} hhc_draw_arrays_indirect_command;

hhc_chunk_draw chunk_draw = {0};
fsl_mem_arena memory_arena_chunk_draw_internal = {0};

fsl_mem_handle handle_draw_command = {0};
static hhc_draw_arrays_indirect_command *draw_command = NULL;
GLuint vao_draw_command = 0;
GLuint vbo_draw_command = 0;

static void chunk_draw_render_v1_internal(void);
static void chunk_draw_render_v2_internal(void);
static void chunk_draw_render_v3_internal(void);

u32 chunk_draw_init_internal(void)
{
    if (fsl_mem_arena_init(&memory_arena_chunk_draw_internal,
                "chunk_draw_init_internal().memory_arena_chunk_draw_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunk_draw_internal, &chunk_draw.handle,
                chunk_order.chunks_max * sizeof(hhc_chunk_mesh),
                "chunk_draw_init_internal().chunk_draw.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunk_draw_internal, &handle_draw_command,
                chunk_order.chunks_max * sizeof(hhc_draw_arrays_indirect_command),
                "chunk_draw_init_internal().handle_draw_command") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    chunk_draw.p = fsl_mem_handle_get(chunk_draw.handle);
    chunk_draw.len = chunk_order.chunks_max;

    draw_command = fsl_mem_handle_get(handle_draw_command);

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

    fsl_mem_arena_free(&memory_arena_chunk_draw_internal,
            "chunk_draw_free_internal().memory_arena_chunk_draw_internal");

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

            glBindBuffer(GL_ARRAY_BUFFER, mesh->quad_buf);
            glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u32), quad_buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u32), (void*)0);
            glVertexAttribDivisor(0, 1);

            glBindBuffer(GL_ARRAY_BUFFER, mesh->light_buf);
            glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u8), light_buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(1);
            glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(u8), (void*)0);
            glVertexAttribDivisor(1, 1);

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

        if (chunk_draw.count)
            --chunk_draw.count;
    }
}

void chunk_draw_render(void)
{
    switch (core.debug.chunk_draw_version)
    {
        case 0:
            chunk_draw_render_v1_internal();
            break;
        case 1:
            chunk_draw_render_v2_internal();
            break;
        case 2:
            chunk_draw_render_v3_internal();
            break;
    }
}

static void chunk_draw_render_v1_internal(void)
{
    hhc_chunk *chunk = NULL;
    fsl_len draw_count = chunk_draw.count;
    i64 i = chunk_order.chunks_max - 1;

    for (; i >= 0 && draw_count; --i)
    {
        chunk = chunk_tab.p[chunk_order.p[i]];
        if (chunk && chunk->mesh)
        {
            glUniform3f(uniform.voxel.block_world_offset,
                    chunk->pos_world.x * CHUNK_DIAMETER,
                    chunk->pos_world.y * CHUNK_DIAMETER,
                    chunk->pos_world.z * CHUNK_DIAMETER);
            glBindVertexArray(chunk->mesh->vao);
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, chunk->mesh->buf_len);
            --draw_count;
        }
    }
}

static void chunk_draw_render_v2_internal(void)
{
    hhc_chunk_mesh *mesh = NULL;
    fsl_len draw_count = chunk_draw.count;
    u32 i = 0;

    for (; i < chunk_draw.len && draw_count; ++i)
    {
        mesh = &chunk_draw.p[i];
        if (mesh->initialized)
        {
            glUniform3f(uniform.voxel.block_world_offset,
                    mesh->chunk->pos_world.x * CHUNK_DIAMETER,
                    mesh->chunk->pos_world.y * CHUNK_DIAMETER,
                    mesh->chunk->pos_world.z * CHUNK_DIAMETER);
            glBindVertexArray(mesh->vao);
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, mesh->buf_len);
            --draw_count;
        }
    }
}

static void chunk_draw_render_v3_internal(void) /* TODO: make the indirect rendering function */
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vbo_draw_command);
    glMultiDrawArraysIndirect(GL_TRIANGLE_FAN, draw_command, chunk_draw.count,
            sizeof(hhc_draw_arrays_indirect_command));
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}
