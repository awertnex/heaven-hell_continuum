#include <string.h>
#include <inttypes.h>

#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>
#include <engine/h/platform.h>

#include "h/main.h"
#include "h/assets.h"
#include "h/chunking.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/logic.h"

u32 engine_err = ERR_SUCCESS;
u32 *const GAME_ERR = (u32*)&engine_err;
u32 chunk_tab_index = 0;

Render render =
{
    .title = GAME_NAME": "GAME_VERSION,
    .size = {1380, 1080 - 24},
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
static ShaderProgram shader[SHADER_COUNT] = {0};
Texture texture[TEXTURE_COUNT] = {0};
Font font[FONT_COUNT];
static Projection projection_world = {0};
static Projection projection_hud = {0};
static Uniform uniform = {0};
static Mesh mesh[MESH_COUNT] = {0};
static FBO fbo[FBO_COUNT] = {0};

static Player lily =
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

static struct /* skybox_data */
{
    f32 time;
    v3f32 sun_rotation;
    v3f32 color;
} skybox_data;

/* ---- callbacks ----------------------------------------------------------- */

static void callback_error(int error, const char* message)
{
    (void)error;
    LOGERROR(TRUE, ERR_GLFW, "GLFW: %s\n", message);
}

static void callback_framebuffer_size(
        GLFWwindow* window, int width, int height);

static void callback_key(
        GLFWwindow *window, int key, int scancode, int action, int mods);

static void callback_scroll(
        GLFWwindow *window, double xoffset, double yoffset);

/* ---- signatures ---------------------------------------------------------- */

static void shaders_init(void);
static void bind_shader_uniforms(void);
static void generate_standard_meshes(void);
void update_render_settings(void);
static void input_update(Player *player);
u32 world_init(str *name);
static void world_update(Player *player);
static void draw_everything(void);

static void
callback_framebuffer_size(
        GLFWwindow* window, int width, int height)
{
    (void)window;

    render.size = (v2i32){width, height};
    lily.camera.ratio = (f32)width / (f32)height;
    lily.camera_hud.ratio = (f32)width / (f32)height;
    glViewport(0, 0, render.size.x, render.size.y);

    fbo_realloc(&render, &fbo[FBO_SKYBOX], FALSE, 4);
    fbo_realloc(&render, &fbo[FBO_WORLD], FALSE, 4);
    fbo_realloc(&render, &fbo[FBO_WORLD_MSAA], TRUE, 4);
    fbo_realloc(&render, &fbo[FBO_HUD], FALSE, 4);
    fbo_realloc(&render, &fbo[FBO_HUD_MSAA], TRUE, 4);
    fbo_realloc(&render, &fbo[FBO_UI], FALSE, 4);
    fbo_realloc(&render, &fbo[FBO_TEXT], FALSE, 4);
    fbo_realloc(&render, &fbo[FBO_TEXT_MSAA], TRUE, 4);
    fbo_realloc(&render, &fbo[FBO_POST_PROCESSING], FALSE, 4);
}

static void
callback_key(
        GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void callback_scroll(
        GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    lily.camera.zoom =
        clamp_f64(lily.camera.zoom + yoffset * CAMERA_ZOOM_SPEED,
                0.0f, CAMERA_ZOOM_MAX);
}

void
update_render_settings(void)
{
    settings.ndc_scale = (v2f32){2.0f / render.size.x, 2.0f / render.size.y};
    settings.fps = 1 / render.frame_delta;
    settings.lerp_speed = SET_LERP_SPEED_DEFAULT;
}

static void
shaders_init(void)
{
    shader[SHADER_FBO] =
        (ShaderProgram){
            .name = "fbo",
            .vertex.file_name = "fbo.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "fbo.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_DEFAULT] =
        (ShaderProgram){
            .name = "default",
            .vertex.file_name = "default.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "default.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_UI] =
        (ShaderProgram){
            .name = "ui",
            .vertex.file_name = "ui.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "ui.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_UI_9_SLICE] =
        (ShaderProgram){
            .name = "ui_9_slice",
            .vertex.file_name = "ui_9_slice.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .geometry.file_name = "ui_9_slice.geom",
            .geometry.type = GL_GEOMETRY_SHADER,
            .fragment.file_name = "ui_9_slice.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_TEXT] =
        (ShaderProgram){
            .name = "text",
            .vertex.file_name = "text.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .geometry.file_name = "text.geom",
            .geometry.type = GL_GEOMETRY_SHADER,
            .fragment.file_name = "text.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_SKYBOX] =
        (ShaderProgram){
            .name = "skybox",
            .vertex.file_name = "skybox.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "skybox.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_GIZMO] =
        (ShaderProgram){
            .name = "gizmo",
            .vertex.file_name = "gizmo.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "gizmo.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_GIZMO_CHUNK] =
        (ShaderProgram){
            .name = "gizmo_chunk",
            .vertex.file_name = "gizmo_chunk.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "gizmo_chunk.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_POST_PROCESSING] =
        (ShaderProgram){
            .name = "post_processing",
            .vertex.file_name = "post_processing.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "post_processing.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_VOXEL] =
        (ShaderProgram){
            .name = "voxel",
            .vertex.file_name = "voxel.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .geometry.file_name = "voxel.geom",
            .geometry.type = GL_GEOMETRY_SHADER,
            .fragment.file_name = "voxel.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

    shader[SHADER_BOUNDING_BOX] =
        (ShaderProgram){
            .name = "bounding_box",
            .vertex.file_name = "bounding_box.vert",
            .vertex.type = GL_VERTEX_SHADER,
            .fragment.file_name = "bounding_box.frag",
            .fragment.type = GL_FRAGMENT_SHADER,
        };

}

static void
bind_shader_uniforms(void)
{
    uniform.defaults.offset =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "offset");
    uniform.defaults.scale =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "scale");
    uniform.defaults.mat_rotation =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "mat_rotation");
    uniform.defaults.mat_perspective =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "mat_perspective");
    uniform.defaults.sun_rotation =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "sun_rotation");
    uniform.defaults.sky_color =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "sky_color");

    uniform.ui.ndc_scale =
        glGetUniformLocation(shader[SHADER_UI].id, "ndc_scale");
    uniform.ui.position =
        glGetUniformLocation(shader[SHADER_UI].id, "position");
    uniform.ui.offset =
        glGetUniformLocation(shader[SHADER_UI].id, "offset");
    uniform.ui.texture_size =
        glGetUniformLocation(shader[SHADER_UI].id, "texture_size");
    uniform.ui.size =
        glGetUniformLocation(shader[SHADER_UI].id, "size");
    uniform.ui.alignment =
        glGetUniformLocation(shader[SHADER_UI].id, "alignment");
    uniform.ui.tint =
        glGetUniformLocation(shader[SHADER_UI].id, "tint");

    uniform.ui_9_slice.ndc_scale =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "ndc_scale");
    uniform.ui_9_slice.position =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "position");
    uniform.ui_9_slice.size =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "size");
    uniform.ui_9_slice.alignment =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "alignment");
    uniform.ui_9_slice.tint =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "tint");
    uniform.ui_9_slice.slice =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "slice");
    uniform.ui_9_slice.slice_size =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "slice_size");
    uniform.ui_9_slice.texture_size =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "texture_size");
    uniform.ui_9_slice.sprite_size =
        glGetUniformLocation(shader[SHADER_UI_9_SLICE].id, "sprite_size");

    uniform.font.char_size =
        glGetUniformLocation(shader[SHADER_TEXT].id, "char_size");
    uniform.font.font_size =
        glGetUniformLocation(shader[SHADER_TEXT].id, "font_size");
    uniform.font.text_color =
        glGetUniformLocation(shader[SHADER_TEXT].id, "text_color");

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
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "camera_position");
    uniform.skybox.mat_rotation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_rotation");
    uniform.skybox.mat_orientation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_orientation");
    uniform.skybox.mat_projection =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_projection");
    uniform.skybox.sun_rotation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "sun_rotation");
    uniform.skybox.sky_color =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "sky_color");

    uniform.gizmo.mat_translation =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_translation");
    uniform.gizmo.mat_rotation =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_rotation");
    uniform.gizmo.mat_orientation =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_orientation");
    uniform.gizmo.mat_projection =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_projection");
    uniform.gizmo.color =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "gizmo_color");

    uniform.gizmo_chunk.render_size =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "render_size");
    uniform.gizmo_chunk.render_distance =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "render_distance");
    uniform.gizmo_chunk.mat_translation =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_translation");
    uniform.gizmo_chunk.mat_rotation =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_rotation");
    uniform.gizmo_chunk.mat_orientation =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_orientation");
    uniform.gizmo_chunk.mat_projection =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_projection");
    uniform.gizmo_chunk.cursor =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "cursor");
    uniform.gizmo_chunk.size =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "size");
    uniform.gizmo_chunk.camera_position =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "camera_position");
    uniform.gizmo_chunk.sky_color =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "sky_color");
    uniform.gizmo_chunk.color =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "chunk_color");

    uniform.post_processing.time =
        glGetUniformLocation(shader[SHADER_POST_PROCESSING].id, "time");

    uniform.voxel.mat_perspective =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "mat_perspective");
    uniform.voxel.player_position =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "player_position");
    uniform.voxel.sun_rotation =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "sun_rotation");
    uniform.voxel.sky_color =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "sky_color");
    uniform.voxel.chunk_position =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "chunk_position");
    uniform.voxel.color =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "voxel_color");
    uniform.voxel.opacity =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "opacity");

    uniform.bounding_box.mat_perspective =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "mat_perspective");
    uniform.bounding_box.position =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "position");
    uniform.bounding_box.size =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "size");
    uniform.bounding_box.color =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "box_color");
}

static void
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

    if (mesh_generate(&mesh[MESH_SKYBOX], GL_STATIC_DRAW,
                VBO_LEN_SKYBOX, EBO_LEN_SKYBOX,
                vbo_data_skybox, ebo_data_skybox) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, "Skybox");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, "Skybox");

    if (mesh_generate(&mesh[MESH_CUBE_OF_HAPPINESS], GL_STATIC_DRAW,
                VBO_LEN_COH, EBO_LEN_COH,
                vbo_data_coh, ebo_data_coh) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, "Cube of Happiness");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, "Cube of Happiness");

    if (mem_alloc((void*)&mesh[MESH_PLAYER].vbo_data,
                sizeof(GLfloat) * VBO_LEN_PLAYER,
                "generate_standard_meshes().mesh[MESH_PLAYER].vbo_data") !=
            ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, "Player");
        goto cleanup;
    }

    memcpy(mesh[MESH_PLAYER].vbo_data, vbo_data_player,
            sizeof(GLfloat) * VBO_LEN_PLAYER);

    mesh[MESH_PLAYER].vbo_len = VBO_LEN_PLAYER;
    glGenVertexArrays(1, &mesh[MESH_PLAYER].vao);
    glGenBuffers(1, &mesh[MESH_PLAYER].vbo);
    glBindVertexArray(mesh[MESH_PLAYER].vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh[MESH_PLAYER].vbo);

    glBufferData(GL_ARRAY_BUFFER, VBO_LEN_PLAYER * sizeof(GLfloat),
            mesh[MESH_PLAYER].vbo_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT,
            GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT,
            GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    LOG_MESH_GENERATE(ERR_SUCCESS, "Player");

    if (mesh_generate(&mesh[MESH_GIZMO], GL_STATIC_DRAW,
                VBO_LEN_GIZMO, EBO_LEN_GIZMO,
                vbo_data_gizmo, ebo_data_gizmo) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, "Gizmo");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, "Gizmo");

    *GAME_ERR = ERR_SUCCESS;
    return;

cleanup:
    mesh_free(&mesh[MESH_PLAYER]);
}

static void
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
                CHUNK_DIAMETER), BLOCK_STONE);
    }

    /* ---- inventory ------------------------------------------------------- */

    u32 i = 0;
    for (; i < 10; ++i)
        if (is_key_press(bind_hotbar_slot[i]) ||
                is_key_press(bind_hotbar_slot_kp[i]))
            hotbar_slot_selected = i + 1;

    if (is_key_press(bind_inventory))
    {
        if ((player->container_state & STATE_CONTR_INVENTORY_SURVIVAL) &&
                state_menu_depth)
        {
            state_menu_depth = 0;
            player->container_state &= ~STATE_CONTR_INVENTORY_SURVIVAL;
        }
        else if (!(player->container_state & STATE_CONTR_INVENTORY_SURVIVAL) &&
                !state_menu_depth)
        {
            state_menu_depth = 1;
            player->container_state |= STATE_CONTR_INVENTORY_SURVIVAL;
        }

        if (!(player->container_state & STATE_CONTR_INVENTORY_SURVIVAL) &&
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
        player->perspective = (player->perspective + 1) % MODE_CAMERA_COUNT;

    if (is_key_press(bind_toggle_zoom))
        player->flag ^= FLAG_PLAYER_ZOOMER;

    /* ---- debug ----------------------------------------------------------- */

#if !GAME_RELEASE_BUILD
    if (is_key_press(bind_toggle_super_debug))
        flag ^= FLAG_MAIN_SUPER_DEBUG;
#endif /* GAME_RELEASE_BUILD */
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
    set_player_pos(&lily, 346.5f, 203.5f, -43.0f);
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

static void
world_update(Player *player)
{
    game_tick = 8000 + (u64)(render.frame_start * 20) -
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

    chunk_queue_update(&CHUNK_QUEUE_1);
    if (!CHUNK_QUEUE_1.count && CHUNK_QUEUE_2.size)
    {
        chunk_queue_update(&CHUNK_QUEUE_2);
        if (!CHUNK_QUEUE_2.count && CHUNK_QUEUE_3.size)
            chunk_queue_update(&CHUNK_QUEUE_3);
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

static void
draw_everything(void)
{
    /* ---- draw skybox ----------------------------------------------------- */

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_SKYBOX].fbo);
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

    glUseProgram(shader[SHADER_SKYBOX].id);
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

    glBindVertexArray(mesh[MESH_SKYBOX].vao);
    glDrawElements(GL_TRIANGLES, mesh[MESH_SKYBOX].ebo_len, GL_UNSIGNED_INT, 0);

    /* ---- draw world ------------------------------------------------------ */

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_WORLD_MSAA].fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader[SHADER_VOXEL].id);
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

        glUseProgram(shader[SHADER_DEFAULT].id);
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

        glBindVertexArray(mesh[MESH_PLAYER].vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh[MESH_PLAYER].vbo_len);
    }

    /* ---- draw player target bounding box --------------------------------- */

    glUseProgram(shader[SHADER_BOUNDING_BOX].id);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);

    if ((flag & FLAG_MAIN_PARSE_TARGET) && (flag & FLAG_MAIN_HUD) &&
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

        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
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

        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
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

        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

#if MODE_INTERNAL_CHUNK_QUEUE_VISUALIZER
    if (!(flag & FLAG_MAIN_DRAW_CHUNK_QUEUE_VISUALIZER))
        goto framebuffer_blit_chunk_queue_visualizer;

    /* ---- draw player chunk queue visualizer ------------------------------ */

    glUseProgram(shader[SHADER_BOUNDING_BOX].id);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);
    glUniform3f(uniform.bounding_box.size,
            CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);

    cursor = CHUNK_ORDER;
    end = CHUNK_ORDER + CHUNK_QUEUE_1.size;
    u32 i = 0;
    for (; cursor < end; ++i, ++cursor)
    {
        if (!((**cursor)->flag & FLAG_CHUNK_QUEUED)) continue;
        glUniform3f(uniform.bounding_box.position,
                (**cursor)->pos.x * CHUNK_DIAMETER,
                (**cursor)->pos.y * CHUNK_DIAMETER,
                (**cursor)->pos.z * CHUNK_DIAMETER);

        glUniform4f(uniform.bounding_box.color, 0.6f, 0.9f, 0.3f, 1.0f);
        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    if (CHUNK_QUEUE_2.size)
    {
        end += CHUNK_QUEUE_2.size;
        for (i = 0; cursor < end; ++i, ++cursor)
        {
            if (!((**cursor)->flag & FLAG_CHUNK_QUEUED)) continue;
            glUniform3f(uniform.bounding_box.position,
                    (**cursor)->pos.x * CHUNK_DIAMETER,
                    (**cursor)->pos.y * CHUNK_DIAMETER,
                    (**cursor)->pos.z * CHUNK_DIAMETER);

            glUniform4f(uniform.bounding_box.color, 0.9f, 0.6f, 0.3f, 1.0f);
            glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
            glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
        }
    }

    if (CHUNK_QUEUE_3.size)
    {
        end += CHUNK_QUEUE_3.size;
        for (i = 0; cursor < end; ++i, ++cursor)
        {
            if (!((**cursor)->flag & FLAG_CHUNK_QUEUED)) continue;
            glUniform3f(uniform.bounding_box.position,
                    (**cursor)->pos.x * CHUNK_DIAMETER,
                    (**cursor)->pos.y * CHUNK_DIAMETER,
                    (**cursor)->pos.z * CHUNK_DIAMETER);

            glUniform4f(uniform.bounding_box.color, 0.9f, 0.3f, 0.3f, 1.0f);
            glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
            glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
        }
    }

framebuffer_blit_chunk_queue_visualizer:
#endif /* MODE_INTERNAL_CHUNK_QUEUE_VISUALIZER */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[FBO_WORLD_MSAA].fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[FBO_WORLD].fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /* ---- draw hud -------------------------------------------------------- */

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_HUD_MSAA].fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader[SHADER_GIZMO].id);

    glUniformMatrix4fv(uniform.gizmo.mat_translation, 1, GL_FALSE,
            (GLfloat*)&projection_hud.target);
    glUniformMatrix4fv(uniform.gizmo.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&projection_hud.rotation);
    glUniformMatrix4fv(uniform.gizmo.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection_hud.orientation);
    glUniformMatrix4fv(uniform.gizmo.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection_hud.projection);

    glBindVertexArray(mesh[MESH_GIZMO].vao);
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
        glUseProgram(shader[SHADER_GIZMO_CHUNK].id);

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
            glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
            glDrawElements(GL_TRIANGLES,
                    mesh[MESH_CUBE_OF_HAPPINESS].ebo_len, GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[FBO_HUD_MSAA].fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[FBO_HUD].fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /* ---- draw overlays --------------------------------------------------- */

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_UI].fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (flag & FLAG_MAIN_HUD)
    {
        glUseProgram(shader[SHADER_UI].id);
        glBindVertexArray(mesh[MESH_UNIT].vao);
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
        glUseProgram(shader[SHADER_UI_9_SLICE].id);
        glBindVertexArray(mesh[MESH_UNIT].vao);

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

    text_start(0, FONT_SIZE_DEFAULT, &font[FONT_MONO_BOLD], &render,
            &shader[SHADER_TEXT], &fbo[FBO_TEXT], TRUE);
    text_push(stringf("FPS               [%d]\n", settings.fps),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render((settings.fps > 60) ?
            COLOR_TEXT_MOSS : COLOR_DIAGNOSTIC_ERROR, TRUE);

    text_push(stringf("\n"
                "FRAME TIME        [%.2lf]\n"
                "FRAME DELTA       [%.5lf]\n"
                "    TICKS         [%"PRId64"]\n"
                "    DAYS          [%"PRId64"]\n",
                render.frame_start,
                render.frame_delta,
                game_tick, game_days),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_TEXT_MOSS, TRUE);

    text_push(stringf("\n\n\n\n\n"
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
    text_render(COLOR_TEXT_DEFAULT, TRUE);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n\n\n"
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
    text_render(COLOR_DIAGNOSTIC_NONE, TRUE);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n\n\n"
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
    text_render(DIAGNOSTIC_COLOR_SUCCESS, TRUE);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n\n\n"
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
    text_render(COLOR_DIAGNOSTIC_ERROR, TRUE);

    text_push(stringf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                "MOUSE XY          [%.2f %.2f]\n"
                "DELTA XY          [%.2f %.2f]\n"
                "RENDER RATIO      [%.4f]\n"
                "SKYBOX TIME       [%.2f]\n"
                "SKYBOX RGB        [%.2f %.2f %.2f]\n"
                "SUN ANGLE         [%.2f %.2f %.2f]\n",
                render.mouse_position.x, render.mouse_position.y,
                render.mouse_delta.x, render.mouse_delta.y,
                (f32)render.size.x / render.size.y,
                skybox_data.time,
                skybox_data.color.x,
                skybox_data.color.y,
                skybox_data.color.z,
                skybox_data.sun_rotation.x,
                skybox_data.sun_rotation.y,
                skybox_data.sun_rotation.z),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0);
    text_render(COLOR_DIAGNOSTIC_INFO, TRUE);

    text_push(stringf(
                "CHUNK QUEUE 1 [%d/%"PRId64"]\n"
                "CHUNK QUEUE 2 [%d/%"PRId64"]\n"
                "CHUNK QUEUE 3 [%d/%"PRId64"]\n"
                "TOTAL CHUNKS [%"PRId64"]\n",
                CHUNK_QUEUE_1.count, CHUNK_QUEUE_1.size,
                CHUNK_QUEUE_2.count, CHUNK_QUEUE_2.size,
                CHUNK_QUEUE_3.count, CHUNK_QUEUE_3.size,
                CHUNKS_MAX[SET_RENDER_DISTANCE]),
            (v2f32){render.size.x - SET_MARGIN, SET_MARGIN},
            TEXT_ALIGN_RIGHT, 0);
    text_render(COLOR_TEXT_DEFAULT, TRUE);

    text_start(0, FONT_SIZE_DEFAULT,
            &font[FONT_MONO], &render, &shader[SHADER_TEXT], &fbo[FBO_TEXT], FALSE);
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
    text_render(DIAGNOSTIC_COLOR_TRACE, TRUE);
    text_stop();

    /* ---- post processing ------------------------------------------------- */

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_POST_PROCESSING].fbo);
    glUseProgram(shader[SHADER_FBO].id);
    glBindVertexArray(mesh[MESH_UNIT].vao);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_SKYBOX].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_WORLD].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    if ((flag & FLAG_MAIN_DEBUG) && (flag & FLAG_MAIN_HUD))
    {
        glBindTexture(GL_TEXTURE_2D, fbo[FBO_HUD].color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindTexture(GL_TEXTURE_2D, fbo[FBO_TEXT].color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_UI].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    /* ---- everything ------------------------------------------------------ */

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader[SHADER_POST_PROCESSING].id);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mesh[MESH_UNIT].vao);
    glUniform1ui(uniform.post_processing.time,
            ((u32)(render.frame_start * settings.target_fps) & 511) + 1);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_POST_PROCESSING].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int
main(int argc, char **argv)
{
    glfwSetErrorCallback(callback_error);
    if (logger_init(GAME_RELEASE_BUILD, argc, argv) != ERR_SUCCESS)
        return *GAME_ERR;

    if (!GAME_RELEASE_BUILD)
        LOGDEBUG(FALSE, "%s\n", "DEVELOPMENT BUILD");

    if (!MODE_INTERNAL_DEBUG)
    {
        LOGWARNING(FALSE, ERR_MODE_INTERNAL_DEBUG_DISABLE,
                "%s\n", "'MODE_INTERNAL_DEBUG' Disabled");
    }
    else LOGDEBUG(FALSE, "%s\n", "Debugging Enabled");

    if (!MODE_INTERNAL_COLLIDE)
    {
        LOGWARNING(FALSE, ERR_MODE_INTERNAL_COLLIDE_DISABLE,
                "%s\n", "'MODE_INTERNAL_COLLIDE' Disabled");
    }

    if (paths_init() != ERR_SUCCESS)
        return *GAME_ERR;

    if (
            glfw_init(FALSE) != ERR_SUCCESS ||
            window_init(&render) != ERR_SUCCESS ||
            glad_init() != ERR_SUCCESS)
        goto cleanup;

    /*temp*/ glfwSetWindowPos(render.window, 1920 - render.size.x, 24);
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
        LOGINFO(FALSE, "%s\n", "GLFW: Raw Mouse Motion Enabled");
    }
    else LOGERROR(FALSE, ERR_GLFW,
            "%s\n", "GLFW: Raw Mouse Motion Not Supported");
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

    shaders_init();

    if (
            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_FBO]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_DEFAULT]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_UI]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_UI_9_SLICE]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_TEXT]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_GIZMO]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_GIZMO_CHUNK]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_SKYBOX]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_POST_PROCESSING]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                &shader[SHADER_VOXEL]) != ERR_SUCCESS ||

            shader_program_init(DIR_ROOT[DIR_SHADERS],
                    &shader[SHADER_BOUNDING_BOX]) != ERR_SUCCESS)
        goto cleanup;

    if(
            fbo_init(&render,
                &fbo[FBO_SKYBOX], &mesh[MESH_UNIT], FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_WORLD],       NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_WORLD_MSAA],  NULL, TRUE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_HUD],         NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_HUD_MSAA],    NULL, TRUE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_UI],          NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_TEXT],        NULL, FALSE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_TEXT_MSAA],   NULL, TRUE, 4) != ERR_SUCCESS ||

            fbo_init(&render,
                &fbo[FBO_POST_PROCESSING], NULL, FALSE, 4) != ERR_SUCCESS)
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
            assets_init() != ERR_SUCCESS ||
            text_init(&shader[SHADER_TEXT]) != ERR_SUCCESS)
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

section_world_loaded:

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

    u32 i = 0;
    assets_free();
    gui_free();
    chunking_free();
    for (i = 0; i < MESH_COUNT; ++i)
        mesh_free(&mesh[i]);
    for (i = 0; i < FBO_COUNT; ++i)
        fbo_free(&fbo[i]);
    text_free();
    for (i = 0; i < SHADER_COUNT; ++i)
        shader_program_free(&shader[i]);
    logger_close();
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return *GAME_ERR;
}
