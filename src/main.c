#include <string.h>

#include <engine/h/core.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>

#include "h/main.h"
#include "h/settings.h"
#include "h/chunking.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/logic.h"
#include "h/input.h"
#include "h/voxel.h"

Render render =
{
    .title = GAME_NAME": "GAME_VERSION,
    .size = {854, 480},
};

Settings settings =
(Settings){
    .reach_distance       = SETTING_REACH_DISTANCE_MAX,
    .lerp_speed           = SETTING_LERP_SPEED_DEFAULT,
    .mouse_sensitivity    = SETTING_MOUSE_SENSITIVITY_DEFAULT / 256.0f,
    .render_distance      = CHUNK_BUF_RADIUS,
    .target_fps           = SETTING_TARGET_FPS_DEFAULT,
    .gui_scale            = SETTING_GUI_SCALE_DEFAULT,
};

u32 state = 0;
f64 game_start_time = 0;
u64 game_tick = 0;
u64 game_days = 0;
Projection projection = {0};
Uniform uniform = {0};

Player lily =
{
    .name = "Lily",
    .pos = {0.0f},
    .scl = {0.6f, 0.6f, 1.8f},
    .collision_check_start = {0.0f},
    .collision_check_end = {0.0f},
    .pitch = 0.0f,
    .yaw = 0.0f,
    .sin_pitch = 0.0f, .cos_pitch = 0.0f,
    .sin_yaw = 0.0f, .cos_yaw = 0.0f,
    .eye_height = SETTING_PLAYER_EYE_HEIGHT,
    .mass = 2.0f,
    .movement_speed = SETTING_PLAYER_SPEED_WALK,
    .container_state = 0,
    .perspective = 0,
    .camera_distance = SETTING_CAMERA_DISTANCE_MAX,
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
FBO fbo_text = {0};
FBO fbo_text_msaa = {0};
FBO fbo_post_processing = {0};

Mesh mesh_fbo = {0};
Mesh mesh_skybox = {0};
Mesh mesh_cube_of_happiness = {0};
Mesh mesh_gizmo = {0};

/* ---- callbacks ----------------------------------------------------------- */
static void callback_error(int error, const char* message)
{
    LOGERROR("GLFW: %s\n", message);
}
static void callback_framebuffer_size(
        GLFWwindow* window, int width, int height);
static void callback_key(
        GLFWwindow *window, int key, int scancode, int action, int mods);

/* ---- signatures ---------------------------------------------------------- */
void generate_standard_meshes(void);
void bind_shader_uniforms(void);
void update_input(Player *player);
b8 init_world(str *name);
void update_world(Player *player);
void draw_skybox(void);
void draw_world(void);
void draw_hud(void);
void draw_everything(void);

static void
callback_framebuffer_size(
        GLFWwindow* window, int width, int height)
{
    render.size = (v2i32){width, height};
    lily.camera.ratio = (f32)width / (f32)height;
    glViewport(0, 0, render.size.x, render.size.y);

    realloc_fbo(&render, &fbo_skybox, FALSE, 4);
    realloc_fbo(&render, &fbo_world, FALSE, 4);
    realloc_fbo(&render, &fbo_world_msaa, TRUE, 4);
    realloc_fbo(&render, &fbo_hud, FALSE, 4);
    realloc_fbo(&render, &fbo_hud_msaa, TRUE, 4);
    realloc_fbo(&render, &fbo_text, FALSE, 4);
    realloc_fbo(&render, &fbo_text_msaa, TRUE, 4);
    realloc_fbo(&render, &fbo_post_processing, FALSE, 4);
}

static void
callback_key(
        GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void
generate_standard_meshes(void)
{
    const u32 VBO_LEN_SKYBOX    = 24;
    const u32 EBO_LEN_SKYBOX    = 36;
    const u32 VBO_LEN_COH       = 24;
    const u32 EBO_LEN_COH       = 36;
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
        0, 1, 8, 8, 7, 0,
        0, 7, 9, 9, 3, 0,
        0, 3, 12, 12, 10, 0,
        0, 10, 11, 11, 2, 0,
        2, 13, 14, 14, 6, 2,
        3, 9, 15, 15, 13, 3,
        2, 11, 16, 16, 13, 2,
        13, 16, 12, 12, 3, 13,
        4, 6, 14, 14, 5, 4,
        7, 8, 15, 15, 9, 7,
        10, 12, 16, 16, 11, 10,
        1, 5, 14, 14, 13, 1,
        1, 13, 15, 15, 8, 1,
    };

    if (generate_mesh(&mesh_skybox, GL_STATIC_DRAW,
                VBO_LEN_SKYBOX, EBO_LEN_SKYBOX,
                vbo_data_skybox, ebo_data_skybox) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Skybox' Mesh Generated");

    if (generate_mesh(&mesh_cube_of_happiness, GL_STATIC_DRAW,
                VBO_LEN_COH, EBO_LEN_COH,
                vbo_data_coh, ebo_data_coh) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Cube of Happiness' Mesh Generated");

    if (generate_mesh(&mesh_gizmo, GL_STATIC_DRAW,
                VBO_LEN_GIZMO, EBO_LEN_GIZMO,
                vbo_data_gizmo, ebo_data_gizmo) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Gizmo' Mesh Generated");

    return;

cleanup:
    LOGERROR("%s\n", "Mesh Generation Failed");
}

void
bind_shader_uniforms(void)
{
    font.uniform.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    font.uniform.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    font.uniform.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

    font_bold.uniform.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    font_bold.uniform.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    font_bold.uniform.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

    font_mono.uniform.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    font_mono.uniform.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    font_mono.uniform.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

    font_mono_bold.uniform.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    font_mono_bold.uniform.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    font_mono_bold.uniform.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

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

    uniform.defaults.mat_perspective =
        glGetUniformLocation(shader_default.id, "mat_perspective");
    uniform.defaults.camera_position =
        glGetUniformLocation(shader_default.id, "camera_position");
    uniform.defaults.sun_rotation =
        glGetUniformLocation(shader_default.id, "sun_rotation");
    uniform.defaults.sky_color =
        glGetUniformLocation(shader_default.id, "sky_color");

    uniform.gizmo.mat_translation =
        glGetUniformLocation(shader_gizmo.id, "mat_translation");
    uniform.gizmo.mat_rotation =
        glGetUniformLocation(shader_gizmo.id, "mat_rotation");
    uniform.gizmo.mat_orientation =
        glGetUniformLocation(shader_gizmo.id, "mat_orientation");
    uniform.gizmo.mat_projection =
        glGetUniformLocation(shader_gizmo.id, "mat_projection");

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
    uniform.voxel.camera_position =
        glGetUniformLocation(shader_voxel.id, "camera_position");
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
}

void
update_input(Player *player)
{
    /* ---- jumping --------------------------------------------------------- */
    if (is_key_hold(bind_jump))
    {
        if (player->state & FLAG_FLYING)
        {
            player->raw_pos.z += player->movement_speed;
            player->pos.z =
                lerp_f32(player->pos.z,
                        player->raw_pos.z,
                        player->pos_lerp_speed.z, render.frame_delta);
        }

        if (player->state & FLAG_CAN_JUMP)
        {
            player->vel.z +=
                SETTING_PLAYER_JUMP_HEIGHT;
            player->state &= ~FLAG_CAN_JUMP;
        }
    }
    if (is_key_press_double(bind_jump))
        player->state ^= FLAG_FLYING;

    /* ---- sneaking -------------------------------------------------------- */
    if (is_key_hold(bind_sneak))
    {
        if (player->state & FLAG_FLYING)
            player->raw_pos.z -= player->movement_speed;
        else player->state |= FLAG_SNEAKING;
    }
    else player->state &= ~FLAG_SNEAKING;

    /* ---- sprinting ------------------------------------------------------- */
    if (is_key_hold(bind_sprint) && is_key_hold(bind_walk_forwards))
        player->state |= FLAG_SPRINTING;
    else if (is_key_release(bind_walk_forwards))
        player->state &= ~FLAG_SPRINTING;

    /* ---- movement -------------------------------------------------------- */
    if (is_key_hold(bind_strafe_left))
    {
        player->raw_pos.x += (player->movement_speed * player->sin_yaw);
        player->raw_pos.y += (player->movement_speed * player->cos_yaw);
    }

    if (is_key_hold(bind_strafe_right))
    {
        player->raw_pos.x -= (player->movement_speed * player->sin_yaw);
        player->raw_pos.y -= (player->movement_speed * player->cos_yaw);
    }

    if (is_key_hold(bind_walk_backwards))
    {
        player->raw_pos.x -= (player->movement_speed * player->cos_yaw);
        player->raw_pos.y += (player->movement_speed * player->sin_yaw);
    }

    if (is_key_hold(bind_walk_forwards))
    {
        player->raw_pos.x += (player->movement_speed * player->cos_yaw);
        player->raw_pos.y -= (player->movement_speed * player->sin_yaw);
    }
    if (is_key_press_double(bind_walk_forwards))
        player->state |= FLAG_SPRINTING;

    player->pos.x = lerp_f32(player->pos.x, player->raw_pos.x,
                player->pos_lerp_speed.x, render.frame_delta);

    player->pos.y = lerp_f32(player->pos.y, player->raw_pos.y,
            player->pos_lerp_speed.y, render.frame_delta);

    player->pos.z = player->raw_pos.z;

    /* ---- gameplay -------------------------------------------------------- */
    if (is_key_hold(bind_attack_or_destroy))
    {
        if ((state & FLAG_PARSE_TARGET)
                && !(state & FLAG_CHUNK_BUF_DIRTY)
                && (chunk_tab[chunk_tab_index] != NULL))
        {
            remove_block(chunk_tab_index,
                    lily.delta_target.x,
                    lily.delta_target.y,
                    lily.delta_target.z);
        }
    }

    if (is_key_press(bind_pick_block))
    {
    }

    if (is_key_hold(bind_use_item_or_place_block))
    {
        if ((state & FLAG_PARSE_TARGET)
                && !(state & FLAG_CHUNK_BUF_DIRTY)
                && (chunk_tab[chunk_tab_index] != NULL))
        {
            add_block(chunk_tab_index,
                    lily.delta_target.x,
                    lily.delta_target.y,
                    lily.delta_target.z);
        }
    }

    /* ---- inventory ------------------------------------------------------- */
    for (u32 i = 0; i < 10; ++i)
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
        state ^= FLAG_HUD;

    if (is_key_press(bind_toggle_debug))
    {
        if (state & FLAG_DEBUG)
            state &= ~(FLAG_DEBUG | FLAG_DEBUG_MORE);
        else
        {
            state |= FLAG_DEBUG;
            if (is_key_hold(KEY_LEFT_SHIFT))
                state |= FLAG_DEBUG_MORE;
        }
    }

    if (is_key_press(bind_toggle_perspective))
    {
        if (player->perspective < 4)
            ++player->perspective;
        else player->perspective = 0;
    }

    /* ---- debug ----------------------------------------------------------- */
#if !RELEASE_BUILD
    if (is_key_press(KEY_TAB))
        state ^= FLAG_SUPER_DEBUG;
#endif /* RELEASE_BUILD */
}

b8
init_world(str *name)
{
    if (!strlen(name))
        return FALSE;

    init_world_directory(name);

    if (init_chunking() != 0)
        return FALSE;

    update_player(&render, &lily, CHUNK_DIAMETER,
            WORLD_RADIUS, WORLD_RADIUS_VERTICAL,
            WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL);
    set_player_block(&lily, 32700, 270, 2);
    lily.delta_chunk = lily.chunk;
    lily.delta_target =
        (v3i64){
            (i64)lily.target.x,
            (i64)lily.target.y,
            (i64)lily.target.z,
        };

    state |= (FLAG_HUD | FLAG_WORLD_LOADED);
    disable_cursor;
    center_cursor;
    return TRUE;
}

void
update_world(Player *player)
{
    game_tick = 6000 + (u64)(render.frame_start * 20) -
        (SETTING_DAY_TICKS_MAX * game_days);

    if (game_tick >= SETTING_DAY_TICKS_MAX)
        ++game_days;

    if (state_menu_depth || (state & FLAG_SUPER_DEBUG))
        show_cursor;
    else disable_cursor;

    update_camera_movement_player(&render, player);
    update_camera_perspective(&player->camera, &projection);
    update_player(&render, &lily, CHUNK_DIAMETER,
            WORLD_RADIUS, WORLD_RADIUS_VERTICAL,
            WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL);
    update_player_target(&lily.target, &lily.delta_target);
    update_collision_static(&lily);

    chunk_tab_index = get_target_chunk_index(lily.chunk, lily.delta_target);
    (chunk_tab_index >= CHUNK_BUF_VOLUME)
        ? chunk_tab_index = CHUNK_TAB_CENTER : 0;

    if (state & FLAG_CHUNK_BUF_DIRTY)
    {
        shift_chunk_tab(lily.chunk, &lily.delta_chunk);
        update_chunking(lily.delta_chunk);
        state &= ~FLAG_CHUNK_BUF_DIRTY;
    }

    /* ---- player targeting ------------------------------------------------ */
    if (is_in_volume_i64(
                lily.delta_target,
                (v3i64){
                -WORLD_DIAMETER, -WORLD_DIAMETER, -WORLD_DIAMETER_VERTICAL},
                (v3i64){
                WORLD_DIAMETER, WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL}))
        state |= FLAG_PARSE_TARGET;
    else state &= ~FLAG_PARSE_TARGET;

    /* TODO: make a function 'index_to_bounding_box()' */
    //if (GetRayCollisionBox(GetScreenToWorldRay(cursor, lily.camera),
    //(BoundingBox){&lily.previous_target}).hit)
    //{
    //}
}

void
draw_skybox(void)
{
    skybox_data.time = (f32)game_tick / (f32)SETTING_DAY_TICKS_MAX;
    skybox_data.sun_rotation =
        (v3f32){
            -cos((skybox_data.time * 360.0f) * DEG2RAD) + 1.0f,
            -cos((skybox_data.time * 360.0f) * DEG2RAD) + 1.0f,
            12.0f,
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
            (GLfloat*)&projection.rotation);

    glUniformMatrix4fv(uniform.skybox.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection.orientation);

    glUniformMatrix4fv(uniform.skybox.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection.projection);

    glUniform3fv(uniform.skybox.sun_rotation, 1,
            (GLfloat*)&skybox_data.sun_rotation);

    glUniform3fv(uniform.skybox.sky_color, 1,
            (GLfloat*)&skybox_data.color);

    draw_mesh(&mesh_skybox);
}

void
draw_world(void)
{
    glUseProgram(shader_voxel.id);

    glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection.perspective);

    glUniform3fv(uniform.voxel.camera_position, 1,
            (GLfloat*)&lily.camera.pos);

    glUniform3fv(uniform.voxel.sun_rotation, 1,
            (GLfloat*)&skybox_data.sun_rotation);

    glUniform3fv(uniform.voxel.sky_color, 1,
            (GLfloat*)&skybox_data.color);

    draw_chunk_tab(&uniform);
}

void
draw_hud(void)
{
    glUseProgram(shader_gizmo.id);

    glUniformMatrix4fv(uniform.gizmo.mat_translation, 1, GL_FALSE,
            (GLfloat*)&projection.target);

    glUniformMatrix4fv(uniform.gizmo.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&projection.rotation);

    glUniformMatrix4fv(uniform.gizmo.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection.orientation);

    glUniformMatrix4fv(uniform.gizmo.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection.projection);

    if (state & FLAG_DEBUG)
        draw_mesh(&mesh_gizmo);

    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_gizmo_chunk.id);

    glUniform2iv(uniform.gizmo_chunk.render_size, 1, (GLint*)&render.size);
    glUniform1i(uniform.gizmo_chunk.render_distance, settings.render_distance);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_translation, 1, GL_FALSE,
            (GLfloat*)&projection.target);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&projection.rotation);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection.orientation);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection.projection);

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

    if (state & FLAG_DEBUG)
        draw_chunk_gizmo(&mesh_cube_of_happiness);
}

void
draw_everything(void)
{
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_skybox.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_skybox();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_world_msaa.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_world();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_world_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_world.fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_hud_msaa.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_hud();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_hud_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_hud.fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    if (state & FLAG_DEBUG)
            draw_debug_info(&lily,
                    skybox_data.time,
                    skybox_data.color,
                    skybox_data.sun_rotation,
                    &render, &shader_text, &fbo_text);

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_post_processing.fbo);
    glUseProgram(shader_fbo.id);
    glBindVertexArray(mesh_fbo.vao);
    glBindTexture(GL_TEXTURE_2D, fbo_skybox.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo_world.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo_hud.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    if (state & FLAG_DEBUG)
    {
        glBindTexture(GL_TEXTURE_2D, fbo_text.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_post_processing.id);
    glBindVertexArray(mesh_fbo.vao);
    glBindTexture(GL_TEXTURE_2D, fbo_post_processing.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int
main(int argc, char **argv)
{
    get_path_bin_root();
    return 0;

    if ((argc > 2) && !strncmp(argv[1], "LOGLEVEL", 8))
    {
        if (!strncmp(argv[2], "FATAL", 5))
            log_level = LOGLEVEL_FATAL;
        else if (!strncmp(argv[2], "ERROR", 5))
            log_level = LOGLEVEL_ERROR;
        else if (!strncmp(argv[2], "WARN", 4))
            log_level = LOGLEVEL_WARNING;
        else if (!strncmp(argv[2], "INFO", 4))
            log_level = LOGLEVEL_INFO;
        else if (!strncmp(argv[2], "DEBUG", 5))
            log_level = LOGLEVEL_DEBUG;
        else if (!strncmp(argv[2], "TRACE", 5))
            log_level = LOGLEVEL_TRACE;
    }

    glfwSetErrorCallback(callback_error);
    /*temp*/ render.size = (v2i32){1280, 720};

    if (!RELEASE_BUILD)
        LOGDEBUG("%s\n", "DEVELOPMENT BUILD");

    if (MODE_INTERNAL_DEBUG)
        LOGWARNING("%s\n", "'MODE_INTERNAL_DEBUG' Disabled");
    else LOGDEBUG("%s\n", "Debugging Enabled");

    if (MODE_INTERNAL_COLLIDE)
        LOGWARNING("%s\n", "'MODE_INTERNAL_COLLIDE' Disabled");

    if (init_paths() != 0 ||
            create_instance("new_instance") != 0)
        return -1;

    if (init_glfw(FALSE) != 0 ||
            init_window(&render) != 0 ||
            init_glad() != 0)
    {
        glfwTerminate();
        return -1;
    }

    /*temp*/ glfwSetWindowPos(render.window, 1920 - render.size.x, 25);
    /*temp*/ glfwSetWindowSizeLimits(render.window, 100, 70, 1920, 1080);

    state =
        FLAG_ACTIVE |
        FLAG_PARSE_CURSOR |
        FLAG_DEBUG;

    /* ---- set mouse input ------------------------------------------------- */
    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGINFO("%s\n", "GLFW: Raw Mouse Motion Enabled");
    }
    else LOGERROR("%s\n", "GLFW: Raw Mouse Motion Not Supported");
    glfwGetCursorPos(render.window,
            &render.mouse_position.x,
            &render.mouse_position.y);

    /* ---- set callbacks --------------------------------------------------- */
    glfwSetFramebufferSizeCallback(render.window, callback_framebuffer_size);
    callback_framebuffer_size(render.window, render.size.x, render.size.y);

    glfwSetKeyCallback(render.window, callback_key);
    callback_key(render.window, 0, 0, 0, 0);

    /* ---- graphics -------------------------------------------------------- */
    if (
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_fbo, "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_default, "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_text, "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_gizmo, "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_gizmo_chunk, "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_skybox, "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_post_processing,
                "r") != 0 ||
            init_shader_program(
                INSTANCE_DIR[DIR_SHADERS], &shader_voxel, "r") != 0)
        goto cleanup;

    if(
            init_fbo(&render, &fbo_skybox, &mesh_fbo, FALSE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_world,       NULL, FALSE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_world_msaa,  NULL, TRUE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_hud,         NULL, FALSE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_hud_msaa,    NULL, TRUE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_text,        NULL, FALSE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_text_msaa,   NULL, TRUE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_post_processing, NULL, FALSE, 4, FALSE) != 0)
        goto cleanup;

    glfwSwapInterval(0); /* vsync off */
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    if (init_gui() != 0 ||
            init_text() != 0)
        goto cleanup;

    /*temp off
    init_super_debugger(&render.size);
    */

    lily.camera =
        (Camera){
            .fovy = SETTING_FOV_DEFAULT,
            .ratio = (f32)render.size.x / (f32)render.size.y,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.05f,
        };

    bind_shader_uniforms();
    game_start_time = glfwGetTime();

section_menu_title:

section_menu_pause:

section_main:

    if (!(state & FLAG_WORLD_LOADED))
    {
        init_world("Poop Consistency Tester");
        init_voxel();
    }

    generate_standard_meshes();

    while (!glfwWindowShouldClose(render.window))
    {
        render.frame_start = glfwGetTime() - game_start_time;
        render.frame_delta = render.frame_start - render.frame_last;
        render.frame_delta_square = pow(render.frame_delta, 2.0f);
        render.frame_last = render.frame_start;
        glfwPollEvents();
        update_key_states(&render);
        update_input(&lily);

        update_mouse_movement(&render);
        update_render_settings(&render);
        update_world(&lily);
        draw_everything();

        glfwSwapBuffers(render.window);

        if (!(state & FLAG_WORLD_LOADED))
            goto section_menu_title;

        if (state & FLAG_PAUSED)
            goto section_menu_pause;
    }

cleanup: /* ----------------------------------------------------------------- */

    free_gui();
    free_mesh(&mesh_fbo);
    free_mesh(&mesh_skybox);
    free_mesh(&mesh_cube_of_happiness);
    free_mesh(&mesh_gizmo);
    free_fbo(&fbo_skybox);
    free_fbo(&fbo_world);
    free_fbo(&fbo_world_msaa);
    free_fbo(&fbo_hud);
    free_fbo(&fbo_hud_msaa);
    free_fbo(&fbo_text);
    free_fbo(&fbo_text_msaa);
    free_fbo(&fbo_post_processing);
    free_text();
    free_shader_program(&shader_fbo);
    free_shader_program(&shader_default);
    free_shader_program(&shader_text);
    free_shader_program(&shader_skybox);
    free_shader_program(&shader_gizmo);
    free_shader_program(&shader_gizmo_chunk);
    free_shader_program(&shader_voxel);
    free_shader_program(&shader_post_processing);
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return 0;
}
