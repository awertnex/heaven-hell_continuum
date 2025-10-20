#include <string.h>
#include <inttypes.h>

#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>

#include "h/main.h"
#include "h/chunking.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/logic.h"

u32 engine_err = ERR_SUCCESS;
u32 *const GAME_ERR = (u32*)&engine_err;

Render render =
{
    .title = GAME_NAME": "GAME_VERSION,
    .size = {1920, 862},
};

Settings settings =
(Settings){
    .reach_distance       = SET_REACH_DISTANCE_MAX,
    .lerp_speed           = SET_LERP_SPEED_DEFAULT,
    .mouse_sensitivity    = SET_MOUSE_SENSITIVITY_DEFAULT / 400.0f,
    .render_distance      = CHUNK_BUF_RADIUS,
    .target_fps           = SET_TARGET_FPS_DEFAULT,
    .gui_scale            = SET_GUI_SCALE_DEFAULT,
};

u64 flag = 0;
f64 game_start_time = 0;
u64 game_tick = 0;
u64 game_days = 0;
Projection projection_world = {0};
Projection projection_hud = {0};
Uniform uniform = {0};
Font font[FONT_COUNT];
Texture texture[TEXTURE_COUNT] = {0};

Player lily =
{
    .name = "Lily",
    .pos = {0.0f},
    .size = {0.6f, 0.6f, 1.8f},
    .collision_check_pos = {0.0f},
    .collision_check_size = {0.0f},
    .pitch = 0.0f,
    .yaw = 0.0f,
    .sin_pitch = 0.0f, .cos_pitch = 0.0f,
    .sin_yaw = 0.0f, .cos_yaw = 0.0f,
    .eye_height = SET_PLAYER_EYE_HEIGHT,
    .mass = 2.0f,
    .movement_speed = SET_PLAYER_SPEED_WALK,
    .container_state = 0,
    .perspective = 0,
    .camera_distance = SET_CAMERA_DISTANCE_MAX,
    .spawn_point = {0},
};

static ShaderProgram shader_fbo =
{
    .name = "fbo",
    .vertex.file_name = "fbo.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "fbo.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_default =
{
    .name = "default",
    .vertex.file_name = "default.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "default.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_ui =
{
    .name = "ui",
    .vertex.file_name = "ui.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "ui.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_ui_9_slice =
{
    .name = "ui_9_slice",
    .vertex.file_name = "ui_9_slice.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .geometry.file_name = "ui_9_slice.geom",
    .geometry.type = GL_GEOMETRY_SHADER,
    .fragment.file_name = "ui_9_slice.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_text =
{
    .name = "text",
    .vertex.file_name = "text.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .geometry.file_name = "text.geom",
    .geometry.type = GL_GEOMETRY_SHADER,
    .fragment.file_name = "text.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_skybox =
{
    .name = "skybox",
    .vertex.file_name = "skybox.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "skybox.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_gizmo =
{
    .name = "gizmo",
    .vertex.file_name = "gizmo.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "gizmo.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_gizmo_chunk =
{
    .name = "gizmo_chunk",
    .vertex.file_name = "gizmo_chunk.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "gizmo_chunk.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_post_processing =
{
    .name = "post_processing",
    .vertex.file_name = "post_processing.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "post_processing.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_voxel =
{
    .name = "voxel",
    .vertex.file_name = "voxel.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .geometry.file_name = "voxel.geom",
    .geometry.type = GL_GEOMETRY_SHADER,
    .fragment.file_name = "voxel.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_bounding_box =
{
    .name = "bounding_box",
    .vertex.file_name = "bounding_box.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "bounding_box.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static struct /* skybox_data */
{
    f32 time;
    v3f32 sun_rotation;
    v3f32 color;
} skybox_data;

FBO fbo_skybox = {0};
FBO fbo_world = {0};
FBO fbo_world_msaa = {0};
FBO fbo_hud = {0};
FBO fbo_hud_msaa = {0};
FBO fbo_ui = {0};
FBO fbo_text = {0};
FBO fbo_text_msaa = {0};
FBO fbo_post_processing = {0};

Mesh mesh_unit = {0};
Mesh mesh_skybox = {0};
Mesh mesh_cube_of_happiness = {0};
Mesh mesh_player = {0};
Mesh mesh_gizmo = {0};

/* ---- callbacks ----------------------------------------------------------- */
static void callback_error(int error, const char* message)
{
    LOGERROR(ERR_GLFW, "GLFW: %s\n", message);
}
static void callback_framebuffer_size(
        GLFWwindow* window, int width, int height);
static void callback_key(
        GLFWwindow *window, int key, int scancode, int action, int mods);

static void callback_scroll(
        GLFWwindow *window, double xoffset, double yoffset);

/* ---- signatures ---------------------------------------------------------- */
void generate_standard_meshes(void);
void bind_shader_uniforms(void);
void update_render_settings(void);
void input_update(Player *player);
u32 world_init(str *name);
void world_update(Player *player);
void draw_everything(void);

static void
callback_framebuffer_size(
        GLFWwindow* window, int width, int height)
{
    render.size = (v2i32){width, height};
    lily.camera.ratio = (f32)width / (f32)height;
    lily.camera_hud.ratio = (f32)width / (f32)height;
    glViewport(0, 0, render.size.x, render.size.y);

    fbo_realloc(&render, &fbo_skybox, FALSE, 4);
    fbo_realloc(&render, &fbo_world, FALSE, 4);
    fbo_realloc(&render, &fbo_world_msaa, TRUE, 4);
    fbo_realloc(&render, &fbo_hud, FALSE, 4);
    fbo_realloc(&render, &fbo_hud_msaa, TRUE, 4);
    fbo_realloc(&render, &fbo_ui, FALSE, 4);
    fbo_realloc(&render, &fbo_text, FALSE, 4);
    fbo_realloc(&render, &fbo_text_msaa, TRUE, 4);
    fbo_realloc(&render, &fbo_post_processing, FALSE, 4);
}

static void
callback_key(
        GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void callback_scroll(
        GLFWwindow *window, double xoffset, double yoffset)
{
    lily.camera.zoom =
        clamp_f64(lily.camera.zoom + yoffset * CAMERA_ZOOM_SPEED,
                0.0f, CAMERA_ZOOM_MAX);
}

void
update_render_settings(void)
{
    settings.ndc_scale = (v2f32){2.0f / render.size.x, 2.0f / render.size.y};
    settings.lerp_speed = SET_LERP_SPEED_DEFAULT;
}

void
generate_standard_meshes(void)
{
    const u32 VBO_LEN_SKYBOX    = 24;
    const u32 EBO_LEN_SKYBOX    = 36;
    const u32 VBO_LEN_COH       = 24;
    const u32 EBO_LEN_COH       = 36;
    const u32 VBO_LEN_PLAYER    = 216;
    const u32 VBO_LEN_GIZMO     = 51;
    const u32 EBO_LEN_GIZMO     = 90;

    GLfloat vbo_data_skybox[] =
    {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    GLuint ebo_data_skybox[] =
    {
        0, 1, 5, 5, 4, 0,
        1, 3, 7, 7, 5, 1,
        3, 2, 6, 6, 7, 3,
        2, 0, 4, 4, 6, 2,
        4, 5, 7, 7, 6, 4,
        0, 2, 3, 3, 1, 0,
    };

    GLfloat vbo_data_coh[] =
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    GLuint ebo_data_coh[] =
    {
        0, 4, 5, 5, 1, 0,
        1, 5, 7, 7, 3, 1,
        3, 7, 6, 6, 2, 3,
        2, 6, 4, 4, 0, 2,
        4, 6, 7, 7, 5, 4,
        0, 1, 3, 3, 2, 0,
    };

    GLfloat vbo_data_player[] =
    {
        /* pos            normals */
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, /* px */
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, /* nx */
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, /* py */
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, /* ny */
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, /* pz */
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, /* nz */
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    };

    const GLfloat THIC = 0.06f;
    GLfloat vbo_data_gizmo[] =
    {
        0.0f, 0.0f, 0.0f,
        THIC, THIC, 0.0f,
        THIC, 0.0f, THIC,
        0.0f, THIC, THIC,
        1.0f, 0.0f, 0.0f,
        1.0f, THIC, 0.0f,
        1.0f, 0.0f, THIC,
        0.0f, 1.0f, 0.0f,
        THIC, 1.0f, 0.0f,
        0.0f, 1.0f, THIC,
        0.0f, 0.0f, 1.0f,
        THIC, 0.0f, 1.0f,
        0.0f, THIC, 1.0f,
        THIC, THIC, THIC,
        1.0f, THIC, THIC,
        THIC, 1.0f, THIC,
        THIC, THIC, 1.0f,
    };

    GLuint ebo_data_gizmo[] =
    {
        0, 2, 6, 6, 4, 0,
        0, 4, 5, 5, 1, 0,
        1, 5, 14, 14, 13, 1,
        2, 13, 14, 14, 6, 2,
        4, 6, 14, 14, 5, 4,

        0, 7, 9, 9, 3, 0,
        0, 1, 8, 8, 7, 0,
        1, 13, 15, 15, 8, 1,
        3, 9, 15, 15, 13, 3,
        7, 8, 15, 15, 9, 7,

        0, 3, 12, 12, 10, 0,
        0, 10, 11, 11, 2, 0,
        2, 11, 16, 16, 13, 2,
        13, 16, 12, 12, 3, 13,
        10, 12, 16, 16, 11, 10,
    };

    if (mesh_generate(&mesh_skybox, GL_STATIC_DRAW,
                VBO_LEN_SKYBOX, EBO_LEN_SKYBOX,
                vbo_data_skybox, ebo_data_skybox) != ERR_SUCCESS)
        goto cleanup;
    LOGINFO("%s\n", "'Skybox' Mesh Generated");

    if (mesh_generate(&mesh_cube_of_happiness, GL_STATIC_DRAW,
                VBO_LEN_COH, EBO_LEN_COH,
                vbo_data_coh, ebo_data_coh) != ERR_SUCCESS)
        goto cleanup;
    LOGINFO("%s\n", "'Cube of Happiness' Mesh Generated");

    if (mem_alloc((void*)&mesh_player.vbo_data,
                sizeof(GLfloat) * VBO_LEN_PLAYER,
                "mesh_player.vbo_data") != ERR_SUCCESS)
        goto cleanup;

    memcpy(mesh_player.vbo_data, vbo_data_player,
            sizeof(GLfloat) * VBO_LEN_PLAYER);

    mesh_player.vbo_len = VBO_LEN_PLAYER;
    glGenVertexArrays(1, &mesh_player.vao);
    glGenBuffers(1, &mesh_player.vbo);
    glBindVertexArray(mesh_player.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_player.vbo);

    glBufferData(GL_ARRAY_BUFFER, VBO_LEN_PLAYER * sizeof(GLfloat),
            mesh_player.vbo_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT,
            GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT,
            GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    LOGINFO("%s\n", "'Player' Mesh Generated");

    if (mesh_generate(&mesh_gizmo, GL_STATIC_DRAW,
                VBO_LEN_GIZMO, EBO_LEN_GIZMO,
                vbo_data_gizmo, ebo_data_gizmo) != ERR_SUCCESS)
        goto cleanup;
    LOGINFO("%s\n", "'Gizmo' Mesh Generated");

    *GAME_ERR = ERR_SUCCESS;
    return;

cleanup:
    mesh_free(&mesh_player);
    LOGERROR(ERR_MESH_GENERATION_FAIL,
            "%s\n", "Mesh Generation Failed");
}

void
bind_shader_uniforms(void)
{
    uniform.defaults.offset =
        glGetUniformLocation(shader_default.id, "offset");
    uniform.defaults.scale =
        glGetUniformLocation(shader_default.id, "scale");
    uniform.defaults.mat_rotation =
        glGetUniformLocation(shader_default.id, "mat_rotation");
    uniform.defaults.mat_perspective =
        glGetUniformLocation(shader_default.id, "mat_perspective");
    uniform.defaults.sun_rotation =
        glGetUniformLocation(shader_default.id, "sun_rotation");
    uniform.defaults.sky_color =
        glGetUniformLocation(shader_default.id, "sky_color");

    uniform.ui.ndc_scale =
        glGetUniformLocation(shader_ui.id, "ndc_scale");
    uniform.ui.position =
        glGetUniformLocation(shader_ui.id, "position");
    uniform.ui.offset =
        glGetUniformLocation(shader_ui.id, "offset");
    uniform.ui.texture_size =
        glGetUniformLocation(shader_ui.id, "texture_size");
    uniform.ui.size =
        glGetUniformLocation(shader_ui.id, "size");
    uniform.ui.alignment =
        glGetUniformLocation(shader_ui.id, "alignment");
    uniform.ui.tint =
        glGetUniformLocation(shader_ui.id, "tint");

    uniform.ui_9_slice.ndc_scale =
        glGetUniformLocation(shader_ui_9_slice.id, "ndc_scale");
    uniform.ui_9_slice.position =
        glGetUniformLocation(shader_ui_9_slice.id, "position");
    uniform.ui_9_slice.size =
        glGetUniformLocation(shader_ui_9_slice.id, "size");
    uniform.ui_9_slice.alignment =
        glGetUniformLocation(shader_ui_9_slice.id, "alignment");
    uniform.ui_9_slice.tint =
        glGetUniformLocation(shader_ui_9_slice.id, "tint");
    uniform.ui_9_slice.slice =
        glGetUniformLocation(shader_ui_9_slice.id, "slice");
    uniform.ui_9_slice.slice_size =
        glGetUniformLocation(shader_ui_9_slice.id, "slice_size");
    uniform.ui_9_slice.texture_size =
        glGetUniformLocation(shader_ui_9_slice.id, "texture_size");
    uniform.ui_9_slice.sprite_size =
        glGetUniformLocation(shader_ui_9_slice.id, "sprite_size");

    uniform.font.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    uniform.font.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    uniform.font.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

    font[FONT_REG].uniform.char_size = uniform.font.char_size;
    font[FONT_REG].uniform.font_size = uniform.font.font_size;
    font[FONT_REG].uniform.text_color = uniform.font.text_color;
    font[FONT_REG_BOLD].uniform.char_size = uniform.font.char_size;
    font[FONT_REG_BOLD].uniform.font_size = uniform.font.font_size;
    font[FONT_REG_BOLD].uniform.text_color = uniform.font.text_color;
    font[FONT_MONO].uniform.char_size = uniform.font.char_size;
    font[FONT_MONO].uniform.font_size = uniform.font.font_size;
    font[FONT_MONO].uniform.text_color = uniform.font.text_color;
    font[FONT_MONO_BOLD].uniform.char_size = uniform.font.char_size;
    font[FONT_MONO_BOLD].uniform.font_size = uniform.font.font_size;
    font[FONT_MONO_BOLD].uniform.text_color = uniform.font.text_color;

    uniform.skybox.camera_position =
        glGetUniformLocation(shader_skybox.id, "camera_position");
    uniform.skybox.mat_rotation =
        glGetUniformLocation(shader_skybox.id, "mat_rotation");
    uniform.skybox.mat_orientation =
        glGetUniformLocation(shader_skybox.id, "mat_orientation");
    uniform.skybox.mat_projection =
        glGetUniformLocation(shader_skybox.id, "mat_projection");
    uniform.skybox.sun_rotation =
        glGetUniformLocation(shader_skybox.id, "sun_rotation");
    uniform.skybox.sky_color =
        glGetUniformLocation(shader_skybox.id, "sky_color");

    uniform.gizmo.mat_translation =
        glGetUniformLocation(shader_gizmo.id, "mat_translation");
    uniform.gizmo.mat_rotation =
        glGetUniformLocation(shader_gizmo.id, "mat_rotation");
    uniform.gizmo.mat_orientation =
        glGetUniformLocation(shader_gizmo.id, "mat_orientation");
    uniform.gizmo.mat_projection =
        glGetUniformLocation(shader_gizmo.id, "mat_projection");
    uniform.gizmo.color =
        glGetUniformLocation(shader_gizmo.id, "gizmo_color");

    uniform.gizmo_chunk.render_size =
        glGetUniformLocation(shader_gizmo_chunk.id, "render_size");
    uniform.gizmo_chunk.render_distance =
        glGetUniformLocation(shader_gizmo_chunk.id, "render_distance");
    uniform.gizmo_chunk.mat_translation =
        glGetUniformLocation(shader_gizmo_chunk.id, "mat_translation");
    uniform.gizmo_chunk.mat_rotation =
        glGetUniformLocation(shader_gizmo_chunk.id, "mat_rotation");
    uniform.gizmo_chunk.mat_orientation =
        glGetUniformLocation(shader_gizmo_chunk.id, "mat_orientation");
    uniform.gizmo_chunk.mat_projection =
        glGetUniformLocation(shader_gizmo_chunk.id, "mat_projection");
    uniform.gizmo_chunk.cursor =
        glGetUniformLocation(shader_gizmo_chunk.id, "cursor");
    uniform.gizmo_chunk.size =
        glGetUniformLocation(shader_gizmo_chunk.id, "size");
    uniform.gizmo_chunk.camera_position =
        glGetUniformLocation(shader_gizmo_chunk.id, "camera_position");
    uniform.gizmo_chunk.sky_color =
        glGetUniformLocation(shader_gizmo_chunk.id, "sky_color");
    uniform.gizmo_chunk.color =
        glGetUniformLocation(shader_gizmo_chunk.id, "chunk_color");

    uniform.voxel.mat_perspective =
        glGetUniformLocation(shader_voxel.id, "mat_perspective");
    uniform.voxel.player_position =
        glGetUniformLocation(shader_voxel.id, "player_position");
    uniform.voxel.sun_rotation =
        glGetUniformLocation(shader_voxel.id, "sun_rotation");
    uniform.voxel.sky_color =
        glGetUniformLocation(shader_voxel.id, "sky_color");
    uniform.voxel.chunk_position =
        glGetUniformLocation(shader_voxel.id, "chunk_position");
    uniform.voxel.color =
        glGetUniformLocation(shader_voxel.id, "voxel_color");
    uniform.voxel.opacity =
        glGetUniformLocation(shader_voxel.id, "opacity");

    uniform.bounding_box.mat_perspective =
        glGetUniformLocation(shader_bounding_box.id, "mat_perspective");
    uniform.bounding_box.position =
        glGetUniformLocation(shader_bounding_box.id, "position");
    uniform.bounding_box.size =
        glGetUniformLocation(shader_bounding_box.id, "size");
    uniform.bounding_box.color =
        glGetUniformLocation(shader_bounding_box.id, "box_color");
}

void
input_update(Player *player)
{
    /* ---- jumping --------------------------------------------------------- */
    if (is_key_hold(bind_jump))
    {
        if (player->flag & FLAG_PLAYER_FLYING)
            player->pos.z += player->movement_speed;

        if (player->flag & FLAG_PLAYER_CAN_JUMP)
        {
            player->vel.z +=
                SET_PLAYER_JUMP_HEIGHT;
            player->flag &= ~FLAG_PLAYER_CAN_JUMP;
        }
    }
    if (is_key_press_double(bind_jump))
        player->flag ^= FLAG_PLAYER_FLYING;

    /* ---- sneaking -------------------------------------------------------- */
    if (is_key_hold(bind_sneak))
    {
        if (player->flag & FLAG_PLAYER_FLYING)
            player->pos.z -= player->movement_speed;
        else player->flag |= FLAG_PLAYER_SNEAKING;
    }
    else player->flag &= ~FLAG_PLAYER_SNEAKING;

    /* ---- sprinting ------------------------------------------------------- */
    if (is_key_hold(bind_sprint) && is_key_hold(bind_walk_forwards))
        player->flag |= FLAG_PLAYER_SPRINTING;
    else if (is_key_release(bind_walk_forwards))
        player->flag &= ~FLAG_PLAYER_SPRINTING;

    /* ---- movement -------------------------------------------------------- */
    if (is_key_hold(bind_strafe_left))
    {
        player->pos.x += (player->movement_speed * player->sin_yaw);
        player->pos.y += (player->movement_speed * player->cos_yaw);
    }

    if (is_key_hold(bind_strafe_right))
    {
        player->pos.x -= (player->movement_speed * player->sin_yaw);
        player->pos.y -= (player->movement_speed * player->cos_yaw);
    }

    if (is_key_hold(bind_walk_backwards))
    {
        player->pos.x -= (player->movement_speed * player->cos_yaw);
        player->pos.y += (player->movement_speed * player->sin_yaw);
    }

    if (is_key_hold(bind_walk_forwards))
    {
        player->pos.x += (player->movement_speed * player->cos_yaw);
        player->pos.y -= (player->movement_speed * player->sin_yaw);
    }
    if (is_key_press_double(bind_walk_forwards))
        player->flag |= FLAG_PLAYER_SPRINTING;

    player->pos_last.x = player->pos_smooth.x;
    player->pos_last.y = player->pos_smooth.y;
    player->pos_last.z = player->pos_smooth.z;

    player->pos_smooth.x = lerp_f32(player->pos_smooth.x, player->pos.x,
                player->pos_lerp_speed.x, render.frame_delta);

    player->pos_smooth.y = lerp_f32(player->pos_smooth.y, player->pos.y,
            player->pos_lerp_speed.y, render.frame_delta);

    player->pos_smooth.z =
        lerp_f32(player->pos_smooth.z, player->pos.z,
                player->pos_lerp_speed.z, render.frame_delta);

    /* ---- gameplay -------------------------------------------------------- */
    if (glfwGetMouseButton(render.window,
                bind_attack_or_destroy) == GLFW_PRESS &&
            (flag & FLAG_MAIN_PARSE_TARGET) &&
            !(flag & FLAG_MAIN_CHUNK_BUF_DIRTY) &&
            chunk_tab[chunk_tab_index])
    {
        block_break(chunk_tab_index,
            lily.delta_target.x - (chunk_tab[chunk_tab_index]->pos.x *
                CHUNK_DIAMETER),
            lily.delta_target.y - (chunk_tab[chunk_tab_index]->pos.y *
                CHUNK_DIAMETER),
            lily.delta_target.z - (chunk_tab[chunk_tab_index]->pos.z *
                CHUNK_DIAMETER));
    }

    if (is_key_press(bind_pick_block))
    {
    }

    if (glfwGetMouseButton(render.window,
                bind_use_item_or_place_block) == GLFW_PRESS &&
            (flag & FLAG_MAIN_PARSE_TARGET) &&
            !(flag & FLAG_MAIN_CHUNK_BUF_DIRTY) &&
            chunk_tab[chunk_tab_index])
    {
        block_place(chunk_tab_index,
            lily.delta_target.x - (chunk_tab[chunk_tab_index]->pos.x *
                CHUNK_DIAMETER),
            lily.delta_target.y - (chunk_tab[chunk_tab_index]->pos.y *
                CHUNK_DIAMETER),
            lily.delta_target.z - (chunk_tab[chunk_tab_index]->pos.z *
                CHUNK_DIAMETER));
    }

    /* ---- inventory ------------------------------------------------------- */
    u32 i = 0;
    for (; i < 10; ++i)
        if (is_key_press(bind_hotbar_slot[i]) ||
                is_key_press(bind_hotbar_slot_kp[i]))
            hotbar_slot_selected = i + 1;

    if (is_key_press(bind_inventory))
    {
        if ((player->container_state & STATE_CONTR_INVENTORY) &&
                state_menu_depth)
        {
            state_menu_depth = 0;
            player->container_state &= ~STATE_CONTR_INVENTORY;
        }
        else if (!(player->container_state & STATE_CONTR_INVENTORY) &&
                !state_menu_depth)
        {
            state_menu_depth = 1;
            player->container_state |= STATE_CONTR_INVENTORY;
        }

        if (!(player->container_state & STATE_CONTR_INVENTORY) &&
                state_menu_depth)
            --state_menu_depth;
    }

    /* ---- miscellaneous --------------------------------------------------- */
    if (is_key_press(bind_toggle_hud))
        flag ^= FLAG_MAIN_HUD;

    if (is_key_press(bind_toggle_debug))
    {
        if (flag & FLAG_MAIN_DEBUG)
            flag &= ~(FLAG_MAIN_DEBUG | FLAG_MAIN_DEBUG_MORE);
        else
        {
            flag |= FLAG_MAIN_DEBUG;
            if (is_key_hold(KEY_LEFT_SHIFT))
                flag |= FLAG_MAIN_DEBUG_MORE;
        }
    }

    if (is_key_press(bind_toggle_perspective))
    {
        if (player->perspective < 4)
            ++player->perspective;
        else player->perspective = 0;
    }

    if (is_key_press(bind_toggle_zoom))
        player->flag ^= FLAG_PLAYER_ZOOMER;

    /* ---- debug ----------------------------------------------------------- */
#if !RELEASE_BUILD
    if (is_key_press(KEY_TAB))
        flag ^= FLAG_MAIN_SUPER_DEBUG;
#endif /* RELEASE_BUILD */
}

u32
world_init(str *name)
{
    if (!strlen(name))
    {
        *GAME_ERR = ERR_POINTER_NULL;
        return *GAME_ERR;
    }

    world_dir_init(name);
    if (*GAME_ERR != ERR_SUCCESS && *GAME_ERR != ERR_WORLD_EXISTS)
        return *GAME_ERR;

    chunking_init();
    if (*GAME_ERR != ERR_SUCCESS)
        return *GAME_ERR;

    player_state_update(&render, &lily, CHUNK_DIAMETER,
            WORLD_RADIUS, WORLD_RADIUS_VERTICAL,
            WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL);
    set_player_block(&lily, 0, 0, 0);
    lily.spawn_point =
        (v3i64){
            (i64)lily.pos.x,
            (i64)lily.pos.y,
            (i64)lily.pos.z
        };
    lily.delta_target =
        (v3i64){
            (i64)lily.target.x,
            (i64)lily.target.y,
            (i64)lily.target.z,
        };

    chunking_update(lily.delta_chunk);
    flag |= (FLAG_MAIN_HUD | FLAG_MAIN_WORLD_LOADED);
    lily.flag |= FLAG_PLAYER_FLYING | FLAG_PLAYER_ZOOMER;
    disable_cursor;
    center_cursor;

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}

void
world_update(Player *player)
{
    game_tick = 2000 + (u64)(render.frame_start * 20) -
        (SET_DAY_TICKS_MAX * game_days);

    if (game_tick >= SET_DAY_TICKS_MAX)
        ++game_days;

    if (state_menu_depth || (flag & FLAG_MAIN_SUPER_DEBUG))
        show_cursor;
    else disable_cursor;

    player_state_update(&render, &lily, CHUNK_DIAMETER,
            WORLD_RADIUS, WORLD_RADIUS_VERTICAL,
            WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL);

    static b8 use_mouse = TRUE;
    use_mouse = (!state_menu_depth && !(flag & FLAG_MAIN_SUPER_DEBUG));
    player_camera_movement_update(&render, player, use_mouse);

    update_camera_perspective(&player->camera, &projection_world);
    update_camera_perspective(&player->camera_hud, &projection_hud);
    player_target_update(&lily);

#if MODE_INTERNAL_COLLIDE
        player_collision_update(&lily, &chunk_tab[chunk_tab_index]);
#endif /* MODE_INTERNAL_COLLIDE */

    chunk_tab_index = get_target_chunk_index(lily.chunk, lily.delta_target);
    (chunk_tab_index >= CHUNK_BUF_VOLUME)
        ? chunk_tab_index = CHUNK_TAB_CENTER : 0;

    if (flag & FLAG_MAIN_CHUNK_BUF_DIRTY)
    {
        flag &= ~FLAG_MAIN_CHUNK_BUF_DIRTY;
        chunk_tab_shift(lily.chunk, &lily.delta_chunk);
        chunking_update(lily.delta_chunk);
    }

    chunk_queue_update(&CHUNK_QUEUE.cursor_1, &CHUNK_QUEUE.count_1,
            CHUNK_QUEUE.priority_1, CHUNK_QUEUE_1ST_ID, 0,
            CHUNK_QUEUE.size_1,
            CHUNK_PARSE_RATE_PRIORITY_HIGH, BLOCK_PARSE_RATE);
    if (!CHUNK_QUEUE.count_1 && CHUNK_QUEUE.size_2)
    {
        chunk_queue_update(&CHUNK_QUEUE.cursor_2, &CHUNK_QUEUE.count_2,
                CHUNK_QUEUE.priority_2, CHUNK_QUEUE_2ND_ID,
                CHUNK_QUEUE.size_1,
                CHUNK_QUEUE.size_2,
                CHUNK_PARSE_RATE_PRIORITY_MID, BLOCK_PARSE_RATE);
        if (!CHUNK_QUEUE.count_2 && CHUNK_QUEUE.size_3)
        {
            chunk_queue_update(&CHUNK_QUEUE.cursor_3, &CHUNK_QUEUE.count_3,
                    CHUNK_QUEUE.priority_3, CHUNK_QUEUE_3RD_ID,
                    CHUNK_QUEUE.size_1 + CHUNK_QUEUE.size_2,
                    CHUNK_QUEUE.size_3,
                    CHUNK_PARSE_RATE_PRIORITY_LOW, BLOCK_PARSE_RATE);
        }
    }

    /* ---- player targeting ------------------------------------------------ */
    if (is_in_volume_i64(
                lily.delta_target,
                (v3i64){
                -(WORLD_DIAMETER * CHUNK_DIAMETER),
                -(WORLD_DIAMETER * CHUNK_DIAMETER),
                -WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER},
                (v3i64){
                WORLD_DIAMETER * CHUNK_DIAMETER,
                WORLD_DIAMETER * CHUNK_DIAMETER,
                WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER}))
        flag |= FLAG_MAIN_PARSE_TARGET;
    else flag &= ~FLAG_MAIN_PARSE_TARGET;

    /* TODO: make a function 'index_to_bounding_box()' */
    //if (GetRayCollisionBox(GetScreenToWorldRay(cursor, lily.camera),
    //(BoundingBox){&lily.previous_target}).hit)
    //{
    //}
}

void
draw_everything(void)
{
    /* ---- draw skybox ----------------------------------------------------- */
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_skybox.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skybox_data.time = (f32)game_tick / SET_DAY_TICKS_MAX;
    skybox_data.sun_rotation =
        (v3f32){
            cos(skybox_data.time * PI * 2.0f),
            cos(skybox_data.time * PI * 2.0f) * 0.3f,
            sin(skybox_data.time * PI * 2.0f),
        };

    f32 intensity = 0.0039f;
    f32 mid_day =       fabsf(sinf(1.5f * sinf(skybox_data.time * PI)));

    f32 pre_burn =      fabsf(sinf(powf(sinf(
                        (skybox_data.time + 0.33f) * PI * 1.2f), 16.0f)));

    f32 burn =          fabsf(sinf(1.5f * powf(sinf(
                        (skybox_data.time + 0.124f) * PI * 1.6f), 32.0f)));

    f32 burn_boost =    fabsf(powf(sinf(
                    (skybox_data.time + 0.212f) * PI * 1.4f), 64.0f));

    f32 mid_night =     fabsf(sinf(powf(2.0f * cosf(
                        skybox_data.time * PI), 3.0f)));

    skybox_data.color =
        (v3f32){
            (mid_day * 171.0f) + (burn * 85.0f) + (mid_night * 1.0f) +
                (pre_burn * 13.0f) + (burn_boost * 76.0f),

            (mid_day * 229.0f) + (burn * 42.0f) + (mid_night * 4.0f) +
                (pre_burn * 7.0f) + (burn_boost * 34.0f),

            (mid_day * 255.0f) + (burn * 19.0f) + (mid_night * 14.0f) +
                (pre_burn * 20.0f),
        };

    skybox_data.color =
        (v3f32){
            clamp_f32(skybox_data.color.x * intensity, 0.0f, 1.0f),
            clamp_f32(skybox_data.color.y * intensity, 0.0f, 1.0f),
            clamp_f32(skybox_data.color.z * intensity, 0.0f, 1.0f),
        };

    glUseProgram(shader_skybox.id);
    glUniform3fv(uniform.skybox.camera_position, 1,
            (GLfloat*)&lily.camera.pos);
    glUniformMatrix4fv(uniform.skybox.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&projection_world.rotation);
    glUniformMatrix4fv(uniform.skybox.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection_world.orientation);
    glUniformMatrix4fv(uniform.skybox.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection_world.projection);
    glUniform3fv(uniform.skybox.sun_rotation, 1,
            (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.skybox.sky_color, 1,
            (GLfloat*)&skybox_data.color);

    glBindVertexArray(mesh_skybox.vao);
    glDrawElements(GL_TRIANGLES, mesh_skybox.ebo_len, GL_UNSIGNED_INT, 0);

    /* ---- draw world ------------------------------------------------------ */
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_world_msaa.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_voxel.id);
    glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);
    glUniform3f(uniform.voxel.player_position,
            lily.pos_smooth.x, lily.pos_smooth.y,
            lily.pos.z + lily.eye_height);
    glUniform3fv(uniform.voxel.sun_rotation, 1,
            (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.voxel.sky_color, 1,
            (GLfloat*)&skybox_data.color);

    f32 opacity = 1.0f;
    if (flag & FLAG_MAIN_DEBUG_MORE)
        opacity = 0.75f;

    glUniform1f(uniform.voxel.opacity, opacity);
    glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_STONE].id);

    static Chunk ***cursor = NULL;
    static Chunk ***end = NULL;
    static Chunk *chunk = NULL;
    cursor = CHUNK_ORDER;
    end = cursor + CHUNKS_MAX[SET_RENDER_DISTANCE];
    for (; **cursor && cursor < end; ++cursor)
    {
        chunk = **cursor;
        if (!(chunk->flag & FLAG_CHUNK_RENDER))
            continue;

        v3f32 chunk_position =
        {
            (f32)(chunk->pos.x * CHUNK_DIAMETER),
            (f32)(chunk->pos.y * CHUNK_DIAMETER),
            (f32)(chunk->pos.z * CHUNK_DIAMETER),
        };

        glUniform3fv(uniform.voxel.chunk_position, 1,
                (GLfloat*)&chunk_position);

        glBindVertexArray(chunk->vao);
        glDrawArrays(GL_POINTS, 0, chunk->vbo_len);
    }

    /* ---- draw player ----------------------------------------------------- */
    if (lily.perspective != MODE_CAMERA_1ST_PERSON)
    {

        glUseProgram(shader_default.id);
        glUniform3fv(uniform.defaults.scale, 1, (GLfloat*)&lily.size);
        glUniform3f(uniform.defaults.offset,
                lily.pos_smooth.x, lily.pos_smooth.y, lily.pos_smooth.z);
        glUniformMatrix4fv(uniform.defaults.mat_rotation, 1, GL_FALSE,
                (GLfloat*)(f32[]){
                lily.cos_yaw, lily.sin_yaw, 0.0f, 0.0f,
                -lily.sin_yaw, lily.cos_yaw, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f});
        glUniformMatrix4fv(uniform.defaults.mat_perspective, 1, GL_FALSE,
                (GLfloat*)&projection_world.perspective);
        glUniform3fv(uniform.defaults.sun_rotation, 1,
                (GLfloat*)&skybox_data.sun_rotation);
        glUniform3fv(uniform.defaults.sky_color, 1,
                (GLfloat*)&skybox_data.color);

        glBindVertexArray(mesh_player.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh_player.vbo_len);
    }

    /* ---- draw player target bounding box --------------------------------- */
    glUseProgram(shader_bounding_box.id);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);

    if ((flag & FLAG_MAIN_PARSE_TARGET) &&
            (flag & FLAG_MAIN_HUD) &&
            chunk_tab[chunk_tab_index] &&
            chunk_tab[chunk_tab_index]->block
            [lily.delta_target.z - (chunk_tab[chunk_tab_index]->pos.z *
                CHUNK_DIAMETER)]
            [lily.delta_target.y - (chunk_tab[chunk_tab_index]->pos.y *
                CHUNK_DIAMETER)]
            [lily.delta_target.x - (chunk_tab[chunk_tab_index]->pos.x *
                CHUNK_DIAMETER)] & FLAG_BLOCK_NOT_EMPTY)
    {
        glUniform3f(uniform.bounding_box.position,
                lily.delta_target.x,
                lily.delta_target.y,
                lily.delta_target.z);
        glUniform3f(uniform.bounding_box.size, 1.0f, 1.0f, 1.0f);
        glUniform4f(uniform.bounding_box.color, 0.0f, 0.0f, 0.0f, 1.0f);

        glBindVertexArray(mesh_cube_of_happiness.vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw player chunk bounding box ---------------------------------- */
    if (flag & FLAG_MAIN_DEBUG_MORE)
    {
        glUniform3f(uniform.bounding_box.position,
                lily.chunk.x * CHUNK_DIAMETER,
                lily.chunk.y * CHUNK_DIAMETER,
                lily.chunk.z * CHUNK_DIAMETER);
        glUniform3f(uniform.bounding_box.size,
                CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);
        glUniform4f(uniform.bounding_box.color, 0.9f, 0.6f, 0.3f, 1.0f);

        glBindVertexArray(mesh_cube_of_happiness.vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw player collision check bounding box ------------------------ */
    if (flag & FLAG_MAIN_DEBUG_MORE)
    {
        glUniform3f(uniform.bounding_box.position,
                lily.collision_check_pos.x,
                lily.collision_check_pos.y,
                lily.collision_check_pos.z);
        glUniform3f(uniform.bounding_box.size,
                lily.collision_check_size.x,
                lily.collision_check_size.y,
                lily.collision_check_size.z);
        glUniform4f(uniform.bounding_box.color, 0.3f, 0.6f, 0.9f, 1.0f);

        glBindVertexArray(mesh_cube_of_happiness.vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

#if MODE_INTERNAL_CHUNK_QUEUE_VISUALIZER
    if (!(flag & FLAG_MAIN_DRAW_CHUNK_QUEUE_VISUALIZER))
        goto framebuffer_blit_chunk_queue_visualizer;

    /* ---- draw player chunk queue visualizer ------------------------------ */
    glUseProgram(shader_bounding_box.id);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);
    glUniform3f(uniform.bounding_box.size,
            CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);

    cursor = CHUNK_ORDER;
    end = CHUNK_ORDER + CHUNK_QUEUE.size_1;
    u32 i = 0;
    for (; cursor < end; ++i, ++cursor)
    {
        if (!((**cursor)->flag & FLAG_CHUNK_QUEUED)) continue;
        glUniform3f(uniform.bounding_box.position,
                (**cursor)->pos.x * CHUNK_DIAMETER,
                (**cursor)->pos.y * CHUNK_DIAMETER,
                (**cursor)->pos.z * CHUNK_DIAMETER);

        glUniform4f(uniform.bounding_box.color, 0.6f, 0.9f, 0.3f, 1.0f);
        glBindVertexArray(mesh_cube_of_happiness.vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    if (CHUNK_QUEUE.size_2)
    {
        end += CHUNK_QUEUE.size_2;
        for (i = 0; cursor < end; ++i, ++cursor)
        {
            if (!((**cursor)->flag & FLAG_CHUNK_QUEUED)) continue;
            glUniform3f(uniform.bounding_box.position,
                    (**cursor)->pos.x * CHUNK_DIAMETER,
                    (**cursor)->pos.y * CHUNK_DIAMETER,
                    (**cursor)->pos.z * CHUNK_DIAMETER);

            glUniform4f(uniform.bounding_box.color, 0.9f, 0.6f, 0.3f, 1.0f);
            glBindVertexArray(mesh_cube_of_happiness.vao);
            glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
        }
    }

    if (CHUNK_QUEUE.size_3)
    {
        end = CHUNK_ORDER + CHUNKS_MAX[SET_RENDER_DISTANCE];
        for (i = 0; cursor < end; ++i, ++cursor)
        {
            if (!((**cursor)->flag & FLAG_CHUNK_QUEUED)) continue;
            glUniform3f(uniform.bounding_box.position,
                    (**cursor)->pos.x * CHUNK_DIAMETER,
                    (**cursor)->pos.y * CHUNK_DIAMETER,
                    (**cursor)->pos.z * CHUNK_DIAMETER);

            glUniform4f(uniform.bounding_box.color, 0.9f, 0.3f, 0.3f, 1.0f);
            glBindVertexArray(mesh_cube_of_happiness.vao);
            glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
        }
    }

framebuffer_blit_chunk_queue_visualizer:
#endif /* MODE_INTERNAL_CHUNK_QUEUE_VISUALIZER */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_world_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_world.fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /* ---- draw hud -------------------------------------------------------- */
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_hud_msaa.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_gizmo.id);

    glUniformMatrix4fv(uniform.gizmo.mat_translation, 1, GL_FALSE,
            (GLfloat*)&projection_hud.target);
    glUniformMatrix4fv(uniform.gizmo.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&projection_hud.rotation);
    glUniformMatrix4fv(uniform.gizmo.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection_hud.orientation);
    glUniformMatrix4fv(uniform.gizmo.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection_hud.projection);

    glBindVertexArray(mesh_gizmo.vao);
    glUniform3f(uniform.gizmo.color, 1.0f, 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 30,
            GL_UNSIGNED_INT, (void*)0);
    glUniform3f(uniform.gizmo.color, 0.0f, 1.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 30,
            GL_UNSIGNED_INT, (void*)(30 * sizeof(GLuint)));
    glUniform3f(uniform.gizmo.color, 0.0f, 0.0f, 1.0f);
    glDrawElements(GL_TRIANGLES, 30,
            GL_UNSIGNED_INT, (void*)(60 * sizeof(GLuint)));

    if (flag & FLAG_MAIN_DEBUG_MORE)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_gizmo_chunk.id);

        glUniform2iv(uniform.gizmo_chunk.render_size, 1,
                (GLint*)&render.size);
        glUniform1i(uniform.gizmo_chunk.render_distance,
                settings.render_distance);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_translation,
                1, GL_FALSE, (GLfloat*)&projection_hud.target);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_rotation,
                1, GL_FALSE, (GLfloat*)&projection_hud.rotation);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_orientation,
                1, GL_FALSE, (GLfloat*)&projection_hud.orientation);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_projection,
                1, GL_FALSE, (GLfloat*)&projection_hud.projection);

        v3f32 camera_position =
        {
            -(lily.cos_yaw * lily.cos_pitch),
            (lily.sin_yaw * lily.cos_pitch),
            lily.sin_pitch,
        };

        glUniform3fv(uniform.gizmo_chunk.camera_position, 1,
                (GLfloat*)&camera_position);

        glUniform3fv(uniform.gizmo_chunk.sky_color, 1,
                (GLfloat*)&skybox_data.color);

        cursor = CHUNK_ORDER;
        for (; **cursor && cursor < end; ++cursor)
        {
            chunk = **cursor;
            u32 i = *cursor - chunk_tab;
            v3f32 pos =
            {
                (f32)(i % CHUNK_BUF_DIAMETER),
                (f32)((i / CHUNK_BUF_DIAMETER) % CHUNK_BUF_DIAMETER),
                (f32)i / CHUNK_BUF_LAYER,
            };
            pos = sub_v3f32(pos, (v3f32){
                    CHUNK_BUF_RADIUS + 0.5f,
                    CHUNK_BUF_RADIUS + 0.5f,
                    CHUNK_BUF_RADIUS + 0.5f});
            glUniform3fv(uniform.gizmo_chunk.cursor, 1,
                    (GLfloat*)&pos);

            f32 pulse = (sinf((pos.z * 0.3f) -
                        (render.frame_start * 5.0f)) * 0.1f) + 0.9f;
            glUniform1f(uniform.gizmo_chunk.size, pulse);

            v4f32 color =
            {
                (f32)((chunk->color >> 24) & 0xff) / 0xff,
                (f32)((chunk->color >> 16) & 0xff) / 0xff,
                (f32)((chunk->color >> 8) & 0xff) / 0xff,
                (f32)((chunk->color & 0xff)) / 0xff,
            };
            glUniform4fv(uniform.gizmo_chunk.color, 1, (GLfloat*)&color);
            glBindVertexArray(mesh_cube_of_happiness.vao);
            glDrawElements(GL_TRIANGLES,
                    mesh_cube_of_happiness.ebo_len, GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_hud_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_hud.fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /* ---- draw overlays --------------------------------------------------- */
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_ui.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (flag & FLAG_MAIN_HUD)
    {
        glUseProgram(shader_ui.id);
        glBindVertexArray(mesh_unit.vao);
        glUniform2fv(uniform.ui.ndc_scale, 1, (GLfloat*)&settings.ndc_scale);

        /* ---- crosshair --------------------------------------------------- */
        if (!(flag & FLAG_MAIN_DEBUG))
        {
            glUniform2i(uniform.ui.position,
                    render.size.x / 2, render.size.y / 2);
            glUniform2f(uniform.ui.offset, 0.0f, 0.0f);
            glUniform2iv(uniform.ui.texture_size, 1,
                    (GLint*)&texture[TEXTURE_CROSSHAIR].size);
            glUniform2i(uniform.ui.size, 8, 8);
            glUniform2i(uniform.ui.alignment, 0, 0);
            glUniform4f(uniform.ui.tint, 1.0f, 1.0f, 1.0f, 1.0f);

            glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_CROSSHAIR].id);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        /* ---- item bar ---------------------------------------------------- */
        glUniform2i(uniform.ui.position, render.size.x / 2, render.size.y);
        glUniform2f(uniform.ui.offset, 84.5f, 18.0f);
        glUniform2iv(uniform.ui.texture_size, 1,
                (GLint*)&texture[TEXTURE_ITEM_BAR].size);
        glUniform2iv(uniform.ui.size, 1,
                (GLint*)&texture[TEXTURE_ITEM_BAR].size);
        glUniform2i(uniform.ui.alignment, 1, 1);
        glUniform4f(uniform.ui.tint, 1.0f, 1.0f, 1.0f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_ITEM_BAR].id);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    /* ---- draw super debug ------------------------------------------------ */
    if (flag & FLAG_MAIN_SUPER_DEBUG)
    {
        glUseProgram(shader_ui_9_slice.id);
        glBindVertexArray(mesh_unit.vao);

        glUniform2fv(uniform.ui_9_slice.ndc_scale, 1,
                (GLfloat*)&settings.ndc_scale);

        glUniform2i(uniform.ui_9_slice.position, SET_MARGIN, SET_MARGIN);
        glUniform2i(uniform.ui_9_slice.size,
                400, render.size.y - (SET_MARGIN * 2));
        glUniform2i(uniform.ui_9_slice.alignment, -1, -1);
        glUniform4f(uniform.ui_9_slice.tint, 1.0f, 1.0f, 1.0f, 0.7f);
        glUniform1i(uniform.ui_9_slice.slice, TRUE);
        glUniform1f(uniform.ui_9_slice.slice_size, 8.0f);
        glUniform2iv(uniform.ui_9_slice.texture_size, 1,
                (GLint*)&texture[TEXTURE_SDB_ACTIVE].size);
        glUniform2i(uniform.ui_9_slice.sprite_size,
                texture[TEXTURE_SDB_ACTIVE].size.x / 2,
                texture[TEXTURE_SDB_ACTIVE].size.y / 2);

        glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_SDB_ACTIVE].id);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    glEnable(GL_DEPTH_TEST);

    /* ---- draw debug info ------------------------------------------------- */
    text_start(0, FONT_SIZE_DEFAULT,
            &font[FONT_MONO_BOLD], &render, &shader_text, &fbo_text, TRUE);
    u32 fps = (u32)(1.0f / render.frame_delta);
    text_push(stringf("FPS               [%d]\n", fps),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render((fps > 60) ? COLOR_TEXT_MOSS : COLOR_DIAGNOSTIC_ERROR);

    text_push(stringf("\n"
                "FRAME TIME        [%.2lf]\n"
                "FRAME DELTA       [%.5lf]\n",
                render.frame_start,
                render.frame_delta),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_TEXT_MOSS);

    text_push(stringf("\n\n\n"
                "PLAYER NAME       [%s]\n"
                "PLAYER XYZ        [%.2f %.2f %.2f]\n"
                "PLAYER BLOCK      [%d %d %d]\n"
                "PLAYER CHUNK      [%d %d %d]\n"
                "CURRENT CHUNK     [%d %d %d]\n"
                "PLAYER PITCH      [%.2f]\n"
                "PLAYER YAW        [%.2f]\n",
                lily.name,
                lily.pos_smooth.x, lily.pos_smooth.y, lily.pos_smooth.z,
                (i32)floorf(lily.pos_smooth.x),
                (i32)floorf(lily.pos_smooth.y),
                (i32)floorf(lily.pos_smooth.z),
                (chunk_tab[CHUNK_TAB_CENTER]) ?
                chunk_tab[CHUNK_TAB_CENTER]->pos.x : 0,
                (chunk_tab[CHUNK_TAB_CENTER]) ?
                chunk_tab[CHUNK_TAB_CENTER]->pos.y : 0,
                (chunk_tab[CHUNK_TAB_CENTER]) ?
                chunk_tab[CHUNK_TAB_CENTER]->pos.z : 0,
                lily.chunk.x, lily.chunk.y, lily.chunk.z,
                lily.pitch, lily.yaw),
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_TEXT_DEFAULT);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n"
                "PLAYER OVERFLOW X [%s]\n"
                "PLAYER OVERFLOW Y [%s]\n"
                "PLAYER OVERFLOW Z [%s]\n",
                (lily.flag & FLAG_PLAYER_OVERFLOW_X) ?
                (lily.flag & FLAG_PLAYER_OVERFLOW_PX) ?
                "        " : "        " : "NONE",
                (lily.flag & FLAG_PLAYER_OVERFLOW_Y) ?
                (lily.flag & FLAG_PLAYER_OVERFLOW_PY) ?
                "        " : "        " : "NONE",
                (lily.flag & FLAG_PLAYER_OVERFLOW_Z) ?
                (lily.flag & FLAG_PLAYER_OVERFLOW_PZ) ?
                "        " : "        " : "NONE"),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_DIAGNOSTIC_NONE);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n"
                "                   %s\n"
                "                   %s\n"
                "                   %s\n",
                (lily.flag & FLAG_PLAYER_OVERFLOW_X) &&
                (lily.flag & FLAG_PLAYER_OVERFLOW_PX) ? "POSITIVE" : "",
                (lily.flag & FLAG_PLAYER_OVERFLOW_Y) &&
                (lily.flag & FLAG_PLAYER_OVERFLOW_PY) ? "POSITIVE" : "",
                (lily.flag & FLAG_PLAYER_OVERFLOW_Z) &&
                (lily.flag & FLAG_PLAYER_OVERFLOW_PZ) ? "POSITIVE" : ""),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(DIAGNOSTIC_COLOR_SUCCESS);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n"
                "                   %s\n"
                "                   %s\n"
                "                   %s\n",
                (lily.flag & FLAG_PLAYER_OVERFLOW_X) &&
                !(lily.flag & FLAG_PLAYER_OVERFLOW_PX) ? "NEGATIVE" : "",
                (lily.flag & FLAG_PLAYER_OVERFLOW_Y) &&
                !(lily.flag & FLAG_PLAYER_OVERFLOW_PY) ? "NEGATIVE" : "",
                (lily.flag & FLAG_PLAYER_OVERFLOW_Z) &&
                !(lily.flag & FLAG_PLAYER_OVERFLOW_PZ) ? "NEGATIVE" : ""),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_DIAGNOSTIC_ERROR);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n\n\n\n"
                "MOUSE XY          [%.2f %.2f]\n"
                "DELTA XY          [%.2f %.2f]\n"
                "RENDER RATIO      [%.4f]\n"
                "TICKS             [%"PRId64"]  DAYS [%"PRId64"]\n"
                "SKYBOX TIME       [%.2f]\n"
                "SKYBOX RGB        [%.2f %.2f %.2f]\n"
                "SUN ANGLE         [%.2f %.2f %.2f]\n",
                render.mouse_position.x, render.mouse_position.y,
                render.mouse_delta.x, render.mouse_delta.y,
                (f32)render.size.x / render.size.y,
                game_tick, game_days,
                skybox_data.time,
                skybox_data.color.x,
                skybox_data.color.y,
                skybox_data.color.z,
                skybox_data.sun_rotation.x,
                skybox_data.sun_rotation.y,
                skybox_data.sun_rotation.z),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_DIAGNOSTIC_INFO);

    text_push(stringf(
                "CHUNK QUEUE 1 [%d] SIZE [%ld]\n"
                "CHUNK QUEUE 2 [%d] SIZE [%ld]\n"
                "CHUNK QUEUE 3 [%d] SIZE [%ld]\n"
                "TOTAL CHUNKS [%ld]\n",
                CHUNK_QUEUE.count_1, CHUNK_QUEUE.size_1,
                CHUNK_QUEUE.count_2, CHUNK_QUEUE.size_2,
                CHUNK_QUEUE.count_3, CHUNK_QUEUE.size_3,
                CHUNKS_MAX[SET_RENDER_DISTANCE]),
            (v2f32){render.size.x - SET_MARGIN, SET_MARGIN},
            TEXT_ALIGN_RIGHT, 0);
    text_render(COLOR_TEXT_DEFAULT);

    text_start(0, FONT_SIZE_DEFAULT,
            &font[FONT_MONO], &render, &shader_text, &fbo_text, FALSE);
    text_push(stringf(
                "Game:     %s v%s\n"
                "Engine:   %s v%s\n"
                "Author:   %s\n"
                "OpenGL:   %s\n"
                "GLSL:     %s\n"
                "Vendor:   %s\n"
                "Renderer: %s\n",
                GAME_NAME, GAME_VERSION,
                ENGINE_NAME, ENGINE_VERSION, ENGINE_AUTHOR,
                glGetString(GL_VERSION),
                glGetString(GL_SHADING_LANGUAGE_VERSION),
                glGetString(GL_VENDOR),
                glGetString(GL_RENDERER)),
            (v2f32){SET_MARGIN, render.size.y - SET_MARGIN},
            0, TEXT_ALIGN_BOTTOM);
    text_render(DIAGNOSTIC_COLOR_TRACE);
    text_stop();

    /* ---- post processing ------------------------------------------------- */
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_post_processing.fbo);
    glUseProgram(shader_fbo.id);
    glBindVertexArray(mesh_unit.vao);
    glBindTexture(GL_TEXTURE_2D, fbo_skybox.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo_world.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    /* ---- everything ------------------------------------------------------ */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_post_processing.id);
    glBindVertexArray(mesh_unit.vao);
    glBindTexture(GL_TEXTURE_2D, fbo_post_processing.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glUseProgram(shader_fbo.id);
    if ((flag & FLAG_MAIN_DEBUG) && (flag & FLAG_MAIN_HUD))
    {
        glBindTexture(GL_TEXTURE_2D, fbo_hud.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindTexture(GL_TEXTURE_2D, fbo_text.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindTexture(GL_TEXTURE_2D, fbo_ui.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int
main(int argc, char **argv)
{
    if (logger_init() != ERR_SUCCESS)
        return *GAME_ERR;

    if (argc > 2 && !strncmp(argv[1], "LOGLEVEL", 8))
    {
        if (!strncmp(argv[2], "FATAL", 5))
            log_level_max = LOGLEVEL_FATAL;
        else if (!strncmp(argv[2], "ERROR", 5))
            log_level_max = LOGLEVEL_ERROR;
        else if (!strncmp(argv[2], "WARN", 4))
            log_level_max = LOGLEVEL_WARNING;
        else if (!strncmp(argv[2], "INFO", 4))
            log_level_max = LOGLEVEL_INFO;
        else if (!strncmp(argv[2], "DEBUG", 5))
            log_level_max = LOGLEVEL_DEBUG;
        else if (!strncmp(argv[2], "TRACE", 5))
            log_level_max = LOGLEVEL_TRACE;

    }

    glfwSetErrorCallback(callback_error);

    if (!RELEASE_BUILD)
        LOGDEBUG("%s\n", "DEVELOPMENT BUILD");

    if (!MODE_INTERNAL_DEBUG)
    {
        LOGWARNING(ERR_MODE_INTERNAL_DEBUG_DISABLE,
                "%s\n", "'MODE_INTERNAL_DEBUG' Disabled");
    }
    else LOGDEBUG("%s\n", "Debugging Enabled");

    if (!MODE_INTERNAL_COLLIDE)
    {
        LOGWARNING(ERR_MODE_INTERNAL_COLLIDE_DISABLE,
                "%s\n", "'MODE_INTERNAL_COLLIDE' Disabled");
    }

    if (grandpath_dir_init() != ERR_SUCCESS)
        return *GAME_ERR;

    if (
            glfw_init(FALSE) != ERR_SUCCESS ||
            window_init(&render) != ERR_SUCCESS ||
            glad_init() != ERR_SUCCESS)
        goto cleanup;

    /*temp*/ glfwSetWindowPos(render.window, 0, 24);
    /*temp*/ glfwSetWindowSizeLimits(render.window, 100, 70, 1920, 1080);

    flag =
        FLAG_MAIN_ACTIVE |
        FLAG_MAIN_PARSE_CURSOR |
        FLAG_MAIN_DEBUG |
        FLAG_MAIN_DRAW_CHUNK_QUEUE_VISUALIZER;

    /* ---- set mouse input ------------------------------------------------- */
    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGINFO("%s\n", "GLFW: Raw Mouse Motion Enabled");
    }
    else
    {
        LOGERROR(ERR_GLFW_RAW_MOUSE_MOTION_NOT_SUPPORT,
                "%s\n", "GLFW: Raw Mouse Motion Not Supported");
    }
    glfwGetCursorPos(render.window,
            &render.mouse_position.x,
            &render.mouse_position.y);

    /* ---- set callbacks --------------------------------------------------- */
    glfwSetFramebufferSizeCallback(render.window, callback_framebuffer_size);
    callback_framebuffer_size(render.window, render.size.x, render.size.y);

    glfwSetKeyCallback(render.window, callback_key);
    callback_key(render.window, 0, 0, 0, 0);

    glfwSetScrollCallback(render.window, callback_scroll);
    callback_scroll(render.window, 0.0f, 0.0f);

    /* ---- set graphics ---------------------------------------------------- */
    if (
            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_fbo, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_default, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_ui, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_ui_9_slice, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_text, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_gizmo, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_gizmo_chunk, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_skybox, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_post_processing, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader_voxel, "r") != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                    &shader_bounding_box, "r") != ERR_SUCCESS)
        goto cleanup;

    if(
            fbo_init(&render,
                &fbo_skybox, &mesh_unit, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_world,       NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_world_msaa,  NULL, TRUE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_hud,         NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_hud_msaa,    NULL, TRUE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_ui,          NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_text,        NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_text_msaa,   NULL, TRUE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo_post_processing, NULL, FALSE, 4) != ERR_SUCCESS)
        goto cleanup;

    glfwSwapInterval(MODE_INTERNAL_VSYNC);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    update_render_settings();

    if (
            gui_init() != ERR_SUCCESS ||
            text_init() != ERR_SUCCESS)
        goto cleanup;

    /*temp off
    init_super_debugger(&render.size);
    */

    lily.camera =
        (Camera){
            .fovy = SET_FOV_DEFAULT,
            .fovy_smooth = SET_FOV_DEFAULT,
            .ratio = (f32)render.size.x / (f32)render.size.y,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.03f,
        };

    lily.camera_hud =
        (Camera){
            .fovy = SET_FOV_DEFAULT,
            .fovy_smooth = SET_FOV_DEFAULT,
            .ratio = (f32)render.size.x / (f32)render.size.y,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.03f,
        };

    bind_shader_uniforms();
    game_start_time = glfwGetTime();

section_menu_title:

section_menu_pause:

section_main:

    if (!(flag & FLAG_MAIN_WORLD_LOADED) &&
            world_init("Poop Consistency Tester") != ERR_SUCCESS)
            goto cleanup;

    generate_standard_meshes();

    while (!glfwWindowShouldClose(render.window) && (flag & FLAG_MAIN_ACTIVE))
    {
        render.frame_start = glfwGetTime() - game_start_time;
        render.frame_delta = render.frame_start - render.frame_last;
        render.frame_last = render.frame_start;

        /* cursor mode change jitter prevention */
        if (!(flag & FLAG_MAIN_PARSE_CURSOR))
        {
            flag |= FLAG_MAIN_PARSE_CURSOR;
            render.mouse_delta = (v2f64){0.0f, 0.0f};
        }

        glfwPollEvents();
        update_key_states(&render);
        input_update(&lily);

        update_mouse_movement(&render);
        update_render_settings();
        world_update(&lily);
        draw_everything();

        glfwSwapBuffers(render.window);

        if (!(flag & FLAG_MAIN_WORLD_LOADED))
            goto section_menu_title;

        if (flag & FLAG_MAIN_PAUSED)
            goto section_menu_pause;
    }

cleanup: /* ----------------------------------------------------------------- */

    gui_free();
    mesh_free(&mesh_unit);
    mesh_free(&mesh_skybox);
    mesh_free(&mesh_cube_of_happiness);
    mesh_free(&mesh_gizmo);
    chunking_free();
    fbo_free(&fbo_skybox);
    fbo_free(&fbo_world);
    fbo_free(&fbo_world_msaa);
    fbo_free(&fbo_hud);
    fbo_free(&fbo_hud_msaa);
    fbo_free(&fbo_text);
    fbo_free(&fbo_text_msaa);
    fbo_free(&fbo_post_processing);
    text_free();
    shader_program_free(&shader_fbo);
    shader_program_free(&shader_default);
    shader_program_free(&shader_text);
    shader_program_free(&shader_skybox);
    shader_program_free(&shader_gizmo);
    shader_program_free(&shader_gizmo_chunk);
    shader_program_free(&shader_voxel);
    shader_program_free(&shader_post_processing);
    logger_close();
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return *GAME_ERR;
}
