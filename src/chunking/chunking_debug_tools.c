#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/assets/mesh/mesh.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shader_types.h"

#include "deps/fossil/h/time.h"

#include "deps/fossil/external/glad/glad.h"

#include "../settings/settings.h"

#include "../h/common.h"
#include "../h/assets.h"
#include "../h/diagnostics.h"
#include "../h/main.h"

#include "chunking.h"
#include "chunking_debug_tools.h"
#include "chunking_internal.h"

/* ---- section: definitions ------------------------------------------------ */

/*!
 *  @brief one entry in a chunk gizmo render buffer.
 */
typedef struct hhc_chunk_gizmo_entry
{
    u32 pos;
    u32 color;
    f32 offset; /* z offset in screen-space coordinates */
} hhc_chunk_gizmo_entry;

/*!
 *  @brief chunk gizmo render buffer data for chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
typedef struct hhc_chunk_gizmo
{
    b8 initialized;
    GLuint vao;
    GLuint vertex_buf;
    GLuint index_buf;
    GLuint data_buf;
    fsl_mem_handle handle;
    hhc_chunk_gizmo_entry *p; /* cached pointer from `handle` */
} hhc_chunk_gizmo;

/* ---- section: declarations ----------------------------------------------- */

static fsl_mem_arena memory_arena_chunk_debug_internal = {0};

/*!
 *  @brief 3D chunk gizmo to showcase chunk status.
 */
hhc_chunk_gizmo chunk_gizmo = {0};

static GLfloat vertex_data_cube[24] =
{
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f
};

static GLuint index_data_cube[36] =
{
    1, 5, 7, 1, 7, 3,
    2, 6, 4, 2, 4, 0,
    3, 7, 6, 3, 6, 2,
    0, 4, 5, 0, 5, 1,
    4, 6, 7, 4, 7, 5,
    0, 1, 3, 0, 3, 2
};

/* ---- section: implementation --------------------------------------------- */

u32 chunk_debug_init_internal(fsl_len chunk_count)
{
    u64 chunk_gizmo_stride = sizeof(hhc_chunk_gizmo_entry);

    if (fsl_mem_arena_init(&memory_arena_chunk_debug_internal,
                "chunk_debug_init().memory_arena_chunk_debug_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunk_debug_internal, &chunk_gizmo.handle,
                chunk_count * chunk_gizmo_stride,
                "chunk_debug_init().chunk_gizmo.handle") != FSL_ERR_SUCCESS)
        goto cleanup;

    chunk_gizmo.p = fsl_mem_handle_get(chunk_gizmo.handle);

    glGenVertexArrays(1, &chunk_gizmo.vao);
    glGenBuffers(1, &chunk_gizmo.vertex_buf);
    glGenBuffers(1, &chunk_gizmo.index_buf);
    glGenBuffers(1, &chunk_gizmo.data_buf);

    glBindVertexArray(chunk_gizmo.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo.vertex_buf);
    glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vertex_data_cube) * sizeof(GLfloat),
            vertex_data_cube, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_gizmo.index_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fsl_arr_len(index_data_cube) * sizeof(GLuint),
            index_data_cube, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo.data_buf);
    glBufferData(GL_ARRAY_BUFFER, chunk_count * chunk_gizmo_stride, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 2, GL_UNSIGNED_INT, chunk_gizmo_stride, (void*)0);
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, chunk_gizmo_stride, (void*)(2 * sizeof(u32)));
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    chunk_gizmo.initialized = TRUE;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    chunk_debug_free_internal();
    return *GAME_ERR;
}

void chunk_debug_free_internal(void)
{
    if (chunk_gizmo.initialized)
    {
        chunk_gizmo.initialized = FALSE;
        glDeleteBuffers(1, &chunk_gizmo.data_buf);
        glDeleteBuffers(1, &chunk_gizmo.index_buf);
        glDeleteBuffers(1, &chunk_gizmo.vertex_buf);
        glDeleteVertexArrays(1, &chunk_gizmo.vao);
    }

    fsl_mem_arena_free(&memory_arena_chunk_debug_internal,
            "chunk_debug_free_internal().memory_arena_chunk_debug_internal");
}

void chunk_debug_chunk_gizmo_draw(const fsl_camera *camera)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    m4f32 mat_transform = {0};
    m4f32 mat_offset = {0};

    mat_offset.a11 = 1.0f;
    mat_offset.a22 = 1.0f;
    mat_offset.a33 = 1.0f;
    mat_offset.a41 = ((f32)render->size.x - CHUNK_GIZMO_SCALE * 2.0f) / render->size.x;
    mat_offset.a42 = ((f32)render->size.y - CHUNK_GIZMO_SCALE * 2.0f) / render->size.y;
    mat_offset.a44 = 1.0f;

    mat_transform = camera->projection.target;
    mat_transform = fsl_multiply_m4f32(mat_transform, camera->projection.rotation);
    mat_transform = fsl_multiply_m4f32(mat_transform, camera->projection.orientation);
    mat_transform = fsl_multiply_m4f32(mat_transform, camera->projection.projection);
    mat_transform = fsl_multiply_m4f32(mat_transform, mat_offset);

    glUseProgram(shader_p[SHADER_GIZMO_CHUNK].asset.id);

    glUniform1f(uniform.gizmo_chunk.gizmo_offset, (f32)settings.chunk_buf_radius);
    glUniform2iv(uniform.gizmo_chunk.render_size, 1, (GLint*)&render->size);
    glUniform1i(uniform.gizmo_chunk.chunk_buf_diameter, settings.chunk_buf_diameter);
    glUniformMatrix4fv(uniform.gizmo_chunk.mat_projection, 1, GL_FALSE, (GLfloat*)&mat_transform);
    glUniform3f(uniform.gizmo_chunk.camera_position,
            -camera->yaw.cos * camera->pitch.cos,
            camera->yaw.sin * camera->pitch.cos,
            camera->pitch.sin);

    glDisable(GL_BLEND);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(chunk_gizmo.vao);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL, chunk_order.chunks_max);
    glEnable(GL_BLEND);
}

void chunk_debug_chunk_gizmo_write_internal(const hhc_chunk *chunk)
{
    v3u32 chunk_pos = {0};
    v4u32 chunk_color = {0};
    u32 index = chunk_order.inv[chunk->cti];
    u64 stride = sizeof(hhc_chunk_gizmo_entry);

    chunk_gizmo.p[index].color = 0;
    chunk_gizmo.p[index].offset = 0.1f;

    if (chunk->flag & FLAG_CHUNK_VISIBLE)
        chunk_gizmo.p[index].offset = 0.0f;

    if (chunk->flag & FLAG_CHUNK_LOADED || chunk->flag & FLAG_CHUNK_VISIBLE)
    {
        chunk_pos.x = chunk->cti % settings.chunk_buf_diameter;
        chunk_pos.y = (chunk->cti / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
        chunk_pos.z = chunk->cti / settings.chunk_buf_layer;

        chunk_color.x = (chunk->color >> 0x18) & 0xff;
        chunk_color.y = (chunk->color >> 0x10) & 0xff;
        chunk_color.z = (chunk->color >> 0x08) & 0xff;
        chunk_color.w = (chunk->color >> 0x00) & 0xff;

        chunk_color.x = (chunk_color.x + ((chunk->color_variant >> 0x18) & 0xff)) / 2;
        chunk_color.y = (chunk_color.y + ((chunk->color_variant >> 0x10) & 0xff)) / 2;
        chunk_color.z = (chunk_color.z + ((chunk->color_variant >> 0x08) & 0xff)) / 2;

        chunk_gizmo.p[index].pos =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo.p[index].color =
            (chunk_color.x << 0x18) |
            (chunk_color.y << 0x10) |
            (chunk_color.z << 0x08) |
            (chunk_color.w << 0x00);
    }
}

void chunk_debug_chunk_gizmo_bake_internal(void)
{
    static u64 bake_interval = 0;
    u64 refresh_rate = FSL_SEC2NSEC / 20;

    if (fsl_on_time_interval(&bake_interval, refresh_rate, render->time))
    {
        glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo.data_buf);
        glBufferData(GL_ARRAY_BUFFER, chunk_order.chunks_max * sizeof(hhc_chunk_gizmo_entry),
                chunk_gizmo.p, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void chunk_debug_scheduler_visualizer_draw(const fsl_camera *camera)
{
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);
    u32 i = 0;
    hhc_chunk *chunk = NULL;
    hhc_chunk_bucket bucket = {0};
    u32 bucket_end = 0;
    u32 distance = 0;
    f32 distance_normalized = 0.0f;
    v4f32 color = {0};

    distance = chunk_sphere_radius_get_internal(settings.render_distance);

    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(mesh_p[MESH_CUBE_OF_HAPPINESS].vao);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&camera->projection.perspective);
    glUniform3f(uniform.bounding_box.size,
            CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);

    for (; i < chunk_sched.buckets_max; ++i)
    {
        bucket = chunk_sched.bucket[i];

        if (bucket.count)
        {
            bucket_end = bucket.pos + bucket.len;
            do
            {
                chunk = chunk_sched.p[bucket.pop];

                distance_normalized = (f32)chunk->cpi / distance;
                color.x = fsl_map_range_f32(distance_normalized, 0.0f, 1.0f, 0.3f, 0.9f);
                color.y = fsl_map_range_f32(distance_normalized, 0.0f, 1.0f, 0.9f, 0.3f);
                color.z = fsl_map_range_f32(distance_normalized, 0.0f, 1.0f, 0.3f, 0.3f);
                color.w = fsl_map_range_f32(distance_normalized, 0.0f, 1.0f, 1.0f, 0.3f);

                glUniform3f(uniform.bounding_box.position,
                        (f32)(chunk->pos_world.x * CHUNK_DIAMETER),
                        (f32)(chunk->pos_world.y * CHUNK_DIAMETER),
                        (f32)(chunk->pos_world.z * CHUNK_DIAMETER));
                glUniform4fv(uniform.bounding_box.color, 1, (GLfloat*)&color);
                glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);

                ++bucket.pop;
                if (bucket.pop >= bucket_end)
                    bucket.pop = bucket.pos;
            } while (bucket.pop != bucket.push);
        }
    }
}
