#include "../engine/core.c"

#define DIR_SHADERS "src/engine/shaders/"

/* ---- declarations -------------------------------------------------------- */
Window render =
{
    .size = {854, 480},
    .title = "glfw_window",
};

Camera camera = {0};

ShaderProgram shader_program_default =
{
    .vertex =
    {
        .file_name = DIR_SHADERS"default.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = DIR_SHADERS"default.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_program_gizmo =
{
    .vertex =
    {
        .file_name = DIR_SHADERS"gizmo.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = DIR_SHADERS"gizmo.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

struct /* uniform */
{
    int matrix_projection;
} uniform;

struct /* matrix */
{
    m4f32 gizmo;
} matrix;

Projection projection = {0};

#define VBO_LEN_COH     24
#define EBO_LEN_COH     36
Mesh cube_of_happiness = {0};

#define VBO_LEN_GIZMO   51
#define EBO_LEN_GIZMO   90
Mesh gizmo = {0};

/* ---- callbacks ----------------------------------------------------------- */
void error_callback(int error, const char* message)
{
    LOGERROR("%s", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

/* ---- signatures ---------------------------------------------------------- */
void update_input(GLFWwindow *win, Camera *camera);
void bind_shader_uniforms(ShaderProgram *shader_program);
void update_shader_uniforms();
void generate_standard_meshes();
void draw_graphics();

int main(void)
{
    glfwSetErrorCallback(error_callback);
    /*temp*/ render.size = (v2i32){1080, 820};

    if (init_glfw() != 0)
        return -1;
    if (init_window(&render) != 0)
        return -1;
    if (init_glew(&render) != 0)
        return -1;

    glfwSwapInterval(1); /* VSync */

    /*temp*/ glfwSetWindowPos(render.window, 1920 - render.size.x, 0);

    /* ---- set mouse input ------------------------------------------------- */
    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGINFO("Raw Mouse Motion Enabled\n");
    }
    else LOGERROR("Raw Mouse Motion Not Supported\n");
    glfwGetCursorPos(render.window, &render.cursor.x, &render.cursor.y);
    glfwSetCursorPos(render.window, render.size.x / 2.0f, render.size.y / 2.0f);

    /* ---- set callbacks --------------------------------------------------- */
    glfwSetFramebufferSizeCallback(render.window, gl_frame_buffer_size_callback);
    glfwSetCursorPosCallback(render.window, gl_cursor_pos_callback);

    /* ---- graphics -------------------------------------------------------- */
    generate_standard_meshes();
    if (init_shader_program(&shader_program_default) != 0
            || init_shader_program(&shader_program_gizmo) != 0)
        goto cleanup;

    bind_shader_uniforms(&shader_program_gizmo);
    bind_shader_uniforms(&shader_program_default);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    camera =
        (Camera){
            .pos = (v3f32){-0.5f, -0.5f, 0.5f},
            .rot = (v3f32){0.0f, 0.0f, 315.0f},
            .fov = 70.0f,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.05f,
        };

    /* ---- main loop ------------------------------------------------------- */
    while (!glfwWindowShouldClose(render.window))
    {
        LOGINFO("  cursor[%7.2lf %7.2lf        ]", render.cursor.x, render.cursor.y);
        LOGINFO("   delta[%7.2lf %7.2lf        ]", render.cursor_delta.x, render.cursor_delta.y);
        LOGINFO("     xyz[%7.2f %7.2f %7.2f]", camera.pos.x, camera.pos.y, camera.pos.z);
        LOGINFO("pitchyaw[        %7.2f %7.2f]\n", camera.rot.y, camera.rot.z);
        update_camera_movement(render.cursor_delta, &camera);
        render.cursor_delta = (v2f64){0.0f, 0.0f};
        update_camera_perspective(&render, &camera, &projection);
        draw_graphics();

        glfwSwapBuffers(render.window);
        glfwPollEvents();
        update_input(render.window, &camera);
    }

cleanup: /* ----------------------------------------------------------------- */

    delete_mesh(&cube_of_happiness);
    delete_mesh(&gizmo);
    glDeleteProgram(shader_program_default.id);
    glDeleteProgram(shader_program_gizmo.id);
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return 0;
}

static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height)
{
    render.size = (v2i32){width, height};
    glViewport(0, 0, width, height);
}

static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    render.cursor_delta = (v2f64){xpos - render.cursor.x, ypos - render.cursor.y};
    render.cursor = (v2f64){xpos, ypos};
}

f64 movement_speed = 0.04f;
void update_input(GLFWwindow *win, Camera *camera)
{
    /* ---- movement -------------------------------------------------------- */
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera->pos.x += (movement_speed * camera->sin_yaw);
        camera->pos.y += (movement_speed * camera->cos_yaw);
    }

    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera->pos.x -= (movement_speed * camera->sin_yaw);
        camera->pos.y -= (movement_speed * camera->cos_yaw);
    }

    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera->pos.x -= (movement_speed * camera->cos_yaw);
        camera->pos.y += (movement_speed * camera->sin_yaw);
    }

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera->pos.x += (movement_speed * camera->cos_yaw);
        camera->pos.y -= (movement_speed * camera->sin_yaw);
    }

    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera->pos.z += movement_speed;
    }

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera->pos.z -= movement_speed;
    }

    /* ---- debug ----------------------------------------------------------- */
    if (glfwGetKey(win, GLFW_KEY_PAUSE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GL_TRUE);
}

void bind_shader_uniforms(ShaderProgram *shader_program)
{
    uniform.matrix_projection = 
        glGetUniformLocation(shader_program->id, "mat_projection");
}

void update_shader_uniforms()
{
    glUniformMatrix4fv(uniform.matrix_projection, 1, GL_FALSE, (GLfloat*)&projection.projection);
}

void generate_standard_meshes()
{
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
    
    const float THIC = 0.03f;
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

    generate_mesh(&cube_of_happiness, GL_STATIC_DRAW, VBO_LEN_COH, EBO_LEN_COH,
            vbo_data_coh, ebo_data_coh);

    generate_mesh(&gizmo, GL_STATIC_DRAW, VBO_LEN_GIZMO, EBO_LEN_GIZMO,
            vbo_data_gizmo, ebo_data_gizmo);
}

void draw_graphics()
{
    /* ---- ready ----------------------------------------------------------- */
    glClearColor(0.69f, 0.86f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    update_shader_uniforms();

    /* ---- draw ------------------------------------------------------------ */
    glUseProgram(shader_program_default.id);
    draw_mesh(&cube_of_happiness);

    glUseProgram(shader_program_gizmo.id);
    draw_mesh(&gizmo);

    /* ---- flush ----------------------------------------------------------- */
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

