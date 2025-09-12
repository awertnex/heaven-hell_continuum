#include "h/main.h"
#include "engine/core.c"
#include "engine/dir.c"
#include "engine/logger.c"
#include "engine/math.c"
#include "engine/memory.c"

#include "h/setting.h"
#include "chunking.c"
#include "dir.c"
#include "keymaps.c"

/* ---- section: declarations ----------------------------------------------- */

b8 logging = 0;
Render render =
{
    .title = ENGINE_NAME": "ENGINE_VERSION,
    .size = {854, 480},
};

Settings setting =
{
    .reach_distance =       SETTING_REACH_DISTANCE_MAX,
    .fov =                  SETTING_FOV_DEFAULT,
    .mouse_sensitivity =    SETTING_MOUSE_SENSITIVITY_DEFAULT * 0.065f,
    .render_distance =      SETTING_RENDER_DISTANCE_DEFAULT,
    .gui_scale =            SETTING_GUI_SCALE_DEFAULT,
};

u32 state = 0;
f64 game_start_time = 0;
u64 game_tick = 0;
u64 game_days = 0;
Camera camera = {0};
Projection projection = {0};
Uniform uniform = {0};

ShaderProgram shader_fbo =
{
    .name = "fbo",
    .vertex =
    {
        .file_name = "fbo.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = "fbo.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_default =
{
    .name = "default",
    .vertex =
    {
        .file_name = "default.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = "default.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_text =
{
    .name = "text",
    .vertex =
    {
        .file_name = "text.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = "text.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_skybox =
{
    .name = "skybox",
    .vertex =
    {
        .file_name = "skybox.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = "skybox.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_gizmo =
{
    .name = "gizmo",
    .vertex =
    {
        .file_name = "gizmo.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = "gizmo.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

struct /* skybox_data */
{
    f32 time;
    v3f32 sun_rotation;
    v3f32 color;
} skybox_data;

GLuint fbo_skybox;
GLuint fbo_world;
GLuint fbo_hud;
GLuint fbo_text;

GLuint color_buf_skybox;
GLuint color_buf_world;
GLuint color_buf_hud;
GLuint color_buf_text;

GLuint rbo_skybox;
GLuint rbo_world;
GLuint rbo_hud;
GLuint rbo_text;

Mesh mesh_fbo = {0};
Mesh mesh_fbo_flipped = {0};

#define VBO_LEN_SKYBOX  24
#define EBO_LEN_SKYBOX  36
Mesh mesh_skybox = {0};

#define VBO_LEN_COH     24
#define EBO_LEN_COH     36
Mesh mesh_cube_of_happiness = {0};

#define VBO_LEN_GIZMO   51
#define EBO_LEN_GIZMO   90
Mesh mesh_gizmo = {0};

/* ---- section: callbacks -------------------------------------------------- */

void error_callback(int error, const char* message)
{
    LOGERROR("GLFW: %s\n", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
static void gl_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

/* ---- section: signatures ------------------------------------------------- */

void log_stuff();
void update_input(GLFWwindow *window, Camera *camera);
void generate_standard_meshes();
void bind_shader_uniforms();
void update_world();
void draw_skybox();
void draw_world();
void draw_hud();
void draw_everything();

void render_font_atlas_example();

/* ---- section: main ------------------------------------------------------- */

Font font = {0}; // TODO: put in gui.c

int main(void)
{
    // TODO: put in gui.c
    str path[PATH_MAX] = {0};
    snprintf(path, PATH_MAX, "%s%s", INSTANCE_DIR[DIR_FONTS], "dejavu-fonts-ttf-2.37/dejavu_sans_mono_ansi_bold.ttf");

    glfwSetErrorCallback(error_callback);
    /*temp*/ render.size = (v2i32){1024, 1024};

    if (!RELEASE_BUILD)
        LOGDEBUG("%s\n", "DEVELOPMENT BUILD");

    if (MODE_DEBUG)
        LOGDEBUG("%s\n", "Debugging Enabled");

    if (init_paths() != 0 ||
            init_instance_directory("new_instance") != 0) /* TODO: make editable instance name */
        return -1;

    if (init_glfw() != 0 ||
            init_window(&render) != 0 ||
            init_glad() != 0)
    {
        glfwTerminate();
        return -1;
    }

    /*temp*/ glfwSetWindowSizeLimits(render.window, 100, 70, 1920, 1080);
    /*temp*/ glfwSetWindowPos(render.window, 1920 - render.size.x, 0);

    /* ---- section: set mouse input ---------------------------------------- */

    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGINFO("%s\n", "GLFW: Raw Mouse Motion Enabled");
    }
    else LOGERROR("%s\n", "GLFW: Raw Mouse Motion Not Supported");
    glfwGetCursorPos(render.window, &render.mouse_position.x, &render.mouse_position.y);

    /* ---- section: set callbacks ------------------------------------------ */

    glfwSetFramebufferSizeCallback(render.window, gl_frame_buffer_size_callback);
    gl_frame_buffer_size_callback(render.window, render.size.x, render.size.y);

    glfwSetCursorPosCallback(render.window, gl_cursor_pos_callback);
    gl_cursor_pos_callback(render.window, render.mouse_position.x, render.mouse_position.y);

    glfwSetKeyCallback(render.window, gl_key_callback);
    gl_key_callback(render.window, 0, 0, 0, 0);

    /* ---- section: graphics ----------------------------------------------- */

    if (init_shader_program(GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_skybox) != 0 ||
            init_shader_program(GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_default) != 0 ||
            init_shader_program(GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_gizmo) != 0 ||
            init_shader_program(GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_fbo) != 0 ||
            init_shader_program(GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_text) != 0 ||
            init_fbo(&render, &fbo_skybox, &color_buf_skybox, &rbo_skybox, &mesh_fbo, 0) != 0 ||
            init_fbo(&render, &fbo_world, &color_buf_world, &rbo_world, &mesh_fbo, 0) != 0 ||
            init_fbo(&render, &fbo_hud, &color_buf_hud, &rbo_hud, &mesh_fbo, 0) != 0 ||
            init_fbo(&render, &fbo_text, &color_buf_text, &rbo_text, &mesh_fbo_flipped, 1) != 0)
        goto cleanup;

    glfwSwapInterval(0); /* vsync off */
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!load_font(&font, 128, path)) // TODO: put in gui.c
        goto cleanup;

    camera =
        (Camera){
            .pos = (v3f32){-1.0f, 1.0f, 1.0f},
            .rot = (v3f32){0.0f, 0.0f, 0.0f},
            .fovy = 70.0f,
            .ratio = (f32)render.size.x / (f32)render.size.y,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.05f,
        };

    bind_shader_uniforms();
    game_start_time = glfwGetTime();

section_menu_title: /* ---- section: title menu ----------------------------- */
section_menu_pause: /* ---- section: pause menu ----------------------------- */
section_main: /* ---- section: main loop ------------------------------------ */

    generate_standard_meshes();

    while (!glfwWindowShouldClose(render.window))
    {
        render.frame_start = glfwGetTime() - game_start_time;
        render.frame_delta = render.frame_start - render.frame_last;
        render.frame_last = render.frame_start;

        //get_mouse_position(&render, &render.mouse_position);
        //get_mouse_movement(render.mouse_last, &render.mouse_delta);

        if (logging) log_stuff();

        update_world();
        draw_everything();

        glfwSwapBuffers(render.window);
        glfwPollEvents();
        update_input(render.window, &camera);
    }

cleanup: /* ----------------------------------------------------------------- */
    free_font(&font);
    free_mesh(&mesh_skybox);
    free_mesh(&mesh_cube_of_happiness);
    free_mesh(&mesh_gizmo);
    glDeleteFramebuffers(1, &fbo_skybox);
    glDeleteFramebuffers(1, &fbo_world);
    glDeleteFramebuffers(1, &fbo_hud);
    glDeleteProgram(shader_default.id);
    glDeleteProgram(shader_skybox.id);
    glDeleteProgram(shader_gizmo.id);
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return 0;
}

static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height)
{
    render.size = (v2i32){width, height};
    camera.ratio = (f32)width / (f32)height;
    glViewport(0, 0, render.size.x, render.size.y);
}

static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    render.mouse_delta = (v2f64){xpos - render.mouse_position.x, ypos - render.mouse_position.y};
    render.mouse_position = (v2f64){xpos, ypos};

    camera.rot.y += render.mouse_delta.y;
    camera.rot.z += render.mouse_delta.x;
}

static void gl_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT))
        init_shader_program(GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_default);

    if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        logging ^= 1;
}

void log_stuff()
{
    printf("---------------------------------------]\n"
            "          mouse[%7.2lf %7.2lf        ]\n"
            "          delta[%7.2lf %7.2lf        ]\n"
            "     camera xyz[%7.2f %7.2f %7.2f]\n"
            "  camera yawpch[        %7.2f %7.2f]\n"
            "          ratio[%7.2f                ]\n\n"
            "    ticks, days[%-7ld %-15ld]\n"
            "    skybox time[%-23.2f]\n"
            "   sun rotation[%7.2f %7.2f %7.2f]\n"
            "          color[%7.2f %7.2f %7.2f]\n\n"

            "    frame_start[%-23.6f]\n"
            "    frame_delta[%-23.6f]\n"
            "            fps[%-23.0f]\n"
            "---------------------------------------]\n\n",

            render.mouse_position.x, render.mouse_position.y,
            render.mouse_delta.x, render.mouse_delta.y, camera.pos.x, camera.pos.y, camera.pos.z,
            camera.rot.y, camera.rot.z,
            camera.ratio,

            game_tick,
            game_days,
            skybox_data.time,
            skybox_data.sun_rotation.x, skybox_data.sun_rotation.y, skybox_data.sun_rotation.z,
            skybox_data.color.x, skybox_data.color.y, skybox_data.color.z,

            render.frame_start,
            render.frame_delta,
            floor(render.frame_start));
}

f32 movement_speed = 5.0f;
void update_input(GLFWwindow *window, Camera *camera)
{
    movement_speed = 5.0f * render.frame_delta;

    /* ---- sprinting ------------------------------------------------------- */
    if (glfwGetKey(window, bind_sprint) == GLFW_PRESS)
        movement_speed += 12.0f * render.frame_delta;

    /* ---- jumping --------------------------------------------------------- */
    if (glfwGetKey(window, bind_jump) == GLFW_PRESS)
    {
        camera->pos.z += movement_speed;
    }

    /* ---- sneaking -------------------------------------------------------- */
    if (glfwGetKey(window, bind_sneak) == GLFW_PRESS)
    {
        camera->pos.z -= movement_speed;
    }

    /* ---- movement -------------------------------------------------------- */
    if (glfwGetKey(window, bind_strafe_left) == GLFW_PRESS)
    {
        camera->pos.x += (movement_speed * camera->sin_yaw);
        camera->pos.y += (movement_speed * camera->cos_yaw);
    }

    if (glfwGetKey(window, bind_strafe_right) == GLFW_PRESS)
    {
        camera->pos.x -= (movement_speed * camera->sin_yaw);
        camera->pos.y -= (movement_speed * camera->cos_yaw);
    }

    if (glfwGetKey(window, bind_walk_backwards) == GLFW_PRESS)
    {
        camera->pos.x -= (movement_speed * camera->cos_yaw);
        camera->pos.y += (movement_speed * camera->sin_yaw);
    }

    if (glfwGetKey(window, bind_walk_forwards) == GLFW_PRESS)
    {
        camera->pos.x += (movement_speed * camera->cos_yaw);
        camera->pos.y -= (movement_speed * camera->sin_yaw);
    }
}

void generate_standard_meshes()
{
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
        1.0f, 1.0f, 1.0f,
        2.0f, 1.0f, 1.0f,
        1.0f, 2.0f, 1.0f,
        2.0f, 2.0f, 1.0f,
        1.0f, 1.0f, 2.0f,
        2.0f, 1.0f, 2.0f,
        1.0f, 2.0f, 2.0f,
        2.0f, 2.0f, 2.0f,
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
    
    GLfloat THIC = 0.05f;
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

    if (generate_mesh(&mesh_skybox, GL_STATIC_DRAW, VBO_LEN_SKYBOX, EBO_LEN_SKYBOX,
            vbo_data_skybox, ebo_data_skybox) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Skybox' Mesh Generated");

    if (generate_mesh(&mesh_cube_of_happiness, GL_STATIC_DRAW, VBO_LEN_COH, EBO_LEN_COH,
            vbo_data_coh, ebo_data_coh) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Cube of Happiness' Mesh Generated");

    if (generate_mesh(&mesh_gizmo, GL_STATIC_DRAW, VBO_LEN_GIZMO, EBO_LEN_GIZMO,
            vbo_data_gizmo, ebo_data_gizmo) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Gizmo' Mesh Generated");

    return;

cleanup:
    LOGERROR("%s\n", "Mesh Generation Failed");
}

void bind_shader_uniforms()
{
    uniform.text.row = glGetUniformLocation(shader_text.id, "row");
    uniform.text.col = glGetUniformLocation(shader_text.id, "col");
    uniform.text.char_size = glGetUniformLocation(shader_text.id, "char_size");
    uniform.text.glyph_size = glGetUniformLocation(shader_text.id, "glyph_size");
    uniform.text.ndc_size = glGetUniformLocation(shader_text.id, "ndc_size");
    uniform.text.offset = glGetUniformLocation(shader_text.id, "offset");
    uniform.text.advance = glGetUniformLocation(shader_text.id, "advance");
    uniform.text.bearing = glGetUniformLocation(shader_text.id, "bearing");
    uniform.text.text_color = glGetUniformLocation(shader_text.id, "text_color");

    uniform.skybox.camera_position = glGetUniformLocation(shader_skybox.id, "camera_position");
    uniform.skybox.mat_rotation = glGetUniformLocation(shader_skybox.id, "mat_rotation");
    uniform.skybox.mat_orientation = glGetUniformLocation(shader_skybox.id, "mat_orientation");
    uniform.skybox.mat_projection = glGetUniformLocation(shader_skybox.id, "mat_projection");
    uniform.skybox.sun_rotation = glGetUniformLocation(shader_skybox.id, "sun_rotation");
    uniform.skybox.sky_color = glGetUniformLocation(shader_skybox.id, "sky_color");

    uniform.defaults.mat_perspective = glGetUniformLocation(shader_default.id, "mat_perspective");
    uniform.defaults.camera_position = glGetUniformLocation(shader_default.id, "camera_position");
    uniform.defaults.sun_rotation = glGetUniformLocation(shader_default.id, "sun_rotation");
    uniform.defaults.sky_color = glGetUniformLocation(shader_default.id, "sky_color");

    uniform.gizmo.render_ratio = glGetUniformLocation(shader_gizmo.id, "ratio");
    uniform.gizmo.mat_target = glGetUniformLocation(shader_gizmo.id, "mat_translation");
    uniform.gizmo.mat_rotation = glGetUniformLocation(shader_gizmo.id, "mat_rotation");
    uniform.gizmo.mat_orientation = glGetUniformLocation(shader_gizmo.id, "mat_orientation");
    uniform.gizmo.mat_projection = glGetUniformLocation(shader_gizmo.id, "mat_projection");
}

void update_world()
{
    game_tick = (floor(render.frame_start * 9000)) - (SETTING_DAY_TICKS_MAX * game_days);
    if (game_tick >= SETTING_DAY_TICKS_MAX)
        ++game_days;

    update_camera_movement(&camera);
    update_camera_perspective(&camera, &projection);
}

void draw_skybox()
{
    skybox_data.time = (f32)game_tick / (f32)SETTING_DAY_TICKS_MAX;
    skybox_data.sun_rotation = (v3f32){-cos((skybox_data.time * 360.0f) * DEG2RAD) + 1.0f, -cos((skybox_data.time * 360.0f) * DEG2RAD) + 1.0f, 12.0f};

    f32 intensity =     0.0039f;
    f32 mid_day =       fabsf(sinf(1.5f * sinf(skybox_data.time * PI)));
    f32 pre_burn =      fabsf(sinf(powf(sinf((skybox_data.time + 0.33f) * PI * 1.2f), 16.0f)));
    f32 burn =          fabsf(sinf(1.5f * powf(sinf((skybox_data.time + 0.124f) * PI * 1.6f), 32.0f)));
    f32 burn_boost =    fabsf(powf(sinf((skybox_data.time + 0.212f) * PI * 1.4f), 64.0f));
    f32 mid_night =     fabsf(sinf(powf(2.0f * cosf(skybox_data.time * PI), 3.0f)));

    skybox_data.color =
    (v3f32){
        (mid_day * 171.0f) + (burn * 85.0f) + (mid_night * 1.0f) + (pre_burn * 13.0f) + (burn_boost * 76.0f),
        (mid_day * 229.0f) + (burn * 42.0f) + (mid_night * 4.0f) + (pre_burn * 7.0f) + (burn_boost * 34.0f),
        (mid_day * 255.0f) + (burn * 19.0f) + (mid_night * 14.0f) + (pre_burn * 20.0f),
    };

    skybox_data.color =
        (v3f32){
            clamp_f32(skybox_data.color.x * intensity, 0.0f, 1.0f),
            clamp_f32(skybox_data.color.y * intensity, 0.0f, 1.0f),
            clamp_f32(skybox_data.color.z * intensity, 0.0f, 1.0f),
        };

    glUseProgram(shader_skybox.id);
    glUniform3fv(uniform.skybox.camera_position, 1, (GLfloat*)&camera.pos);
    glUniformMatrix4fv(uniform.skybox.mat_rotation, 1, GL_FALSE, (GLfloat*)&projection.rotation);
    glUniformMatrix4fv(uniform.skybox.mat_orientation, 1, GL_FALSE, (GLfloat*)&projection.orientation);
    glUniformMatrix4fv(uniform.skybox.mat_projection, 1, GL_FALSE, (GLfloat*)&projection.projection);
    glUniform3fv(uniform.skybox.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.skybox.sky_color, 1, (GLfloat*)&skybox_data.color);
    draw_mesh(&mesh_skybox);
}

void draw_world()
{
    glUseProgram(shader_default.id);
    glUniformMatrix4fv(uniform.defaults.mat_perspective, 1, GL_FALSE, (GLfloat*)&projection.perspective);
    glUniform3fv(uniform.defaults.camera_position, 1, (GLfloat*)&camera.pos);
    glUniform3fv(uniform.defaults.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.defaults.sky_color, 1, (GLfloat*)&skybox_data.color);
    draw_mesh(&mesh_cube_of_happiness);
}

void draw_hud()
{
    glUseProgram(shader_gizmo.id);
    glUniform1f(uniform.gizmo.render_ratio, (GLfloat)camera.ratio);
    glUniformMatrix4fv(uniform.gizmo.mat_target, 1, GL_FALSE, (GLfloat*)&projection.target);
    glUniformMatrix4fv(uniform.gizmo.mat_rotation, 1, GL_FALSE, (GLfloat*)&projection.rotation);
    glUniformMatrix4fv(uniform.gizmo.mat_orientation, 1, GL_FALSE, (GLfloat*)&projection.orientation);
    glUniformMatrix4fv(uniform.gizmo.mat_projection, 1, GL_FALSE, (GLfloat*)&projection.projection);
    draw_mesh(&mesh_gizmo);
}

void draw_everything()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_skybox);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    draw_skybox();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_world);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_world();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_hud);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_hud();
    render_font_atlas_example(); /*temp*/

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_fbo.id);
    glBindVertexArray(mesh_fbo.vao);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, color_buf_skybox);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, color_buf_world);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, color_buf_hud);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

/* ---- section: testing ---------------------------------------------------- */

void render_font_atlas_example()
{
    glUseProgram(shader_text.id);
    glBindVertexArray(mesh_fbo_flipped.vao);
    glBindTexture(GL_TEXTURE_2D, font.id);
    draw_text("FPS:", &font, FONT_SIZE_DEFAULT, (v3f32){0.0f, 0.0f, 0.0f}, (v4u8){0xff, 0xff, 0xff, 0xff}, 0, 0);
    draw_text("XYZ:", &font, FONT_SIZE_DEFAULT, (v3f32){0.0f, FONT_SIZE_DEFAULT, 0.0f}, (v4u8){0xff, 0xff, 0xff, 0xff}, 0, 0);
    draw_text("Lgbubu!labubu!", &font, FONT_SIZE_DEFAULT, (v3f32){0.0f, FONT_SIZE_DEFAULT * 2.0f, 0.0f}, (v4u8){0xff, 0xff, 0xff, 0xff}, 0, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_text(const str *text, Font *font, f32 size, v3f32 pos, v4u8 color, i8 align_x, i8 align_y)
{
    u64 len = strlen(text);
    if (len <= 0) return;

    f32 scale = stbtt_ScaleForPixelHeight(&font->info, size);
    f32 advance = 0.0f;
    Glyph *g = NULL;

    v2f32 screen_size =
    {
        2.0f / render.size.x,
        2.0f / render.size.y,
    };

    v2f32 glyph_size =
    {
        size * screen_size.x * 0.5f,
        size * screen_size.y * 0.5f,
    };

    v2f32 ndc_size =
    {
        scale * screen_size.x,
        scale * screen_size.y,
    };

    pos.x *= screen_size.x;
    pos.y *= screen_size.y;

    v2f32 offset =
    {
        -1.0f + glyph_size.x + pos.x,
        1.0f - glyph_size.y - pos.y,
    };

    offset.y += (align_y - 1) * (font->scale.y / 2.0f) * ndc_size.y;
    if (align_x)
    {
        // TODO: calculate string width, divide by 2, subtract from font->projection.a41
    }

    v4f32 text_color =
    {
        (f32)color.x / 0xff,
        (f32)color.y / 0xff,
        (f32)color.z / 0xff,
        (f32)color.w / 0xff,
    };

    glUniform1f(uniform.text.char_size, font->char_size);
    glUniform2fv(uniform.text.glyph_size, 1, (GLfloat*)&glyph_size);
    glUniform2fv(uniform.text.ndc_size, 1, (GLfloat*)&ndc_size);
    glUniform2fv(uniform.text.offset, 1, (GLfloat*)&offset);
    glUniform4fv(uniform.text.text_color, 1, (GLfloat*)&text_color);
    for (u64 i = 0; i < len; ++i)
    {
        g = &font->glyph[text[i]];

        glUniform1i(uniform.text.row, text[i] / FONT_ATLAS_CELL_RESOLUTION);
        glUniform1i(uniform.text.col, text[i] % FONT_ATLAS_CELL_RESOLUTION);
        glUniform1f(uniform.text.advance, advance + g->bearing.x);
        glUniform1f(uniform.text.bearing, font->descent - g->bearing.y);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        advance += g->advance;
    }
}

