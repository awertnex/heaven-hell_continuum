#include "../engine/core.c"

#define DIR_SHADERS "src/engine/shaders/"

/* ---- declarations -------------------------------------------------------- */
Window render =
{
    .size = {854, 480},
    .title = "glfw_window",
};

Camera camera = {0};

Shader vertex_shader =
{
    .file_name = DIR_SHADERS"default.vert",
    .shader = 0,
    .type = GL_VERTEX_SHADER,
};

Shader fragment_shader =
{
    .file_name = DIR_SHADERS"default.frag",
    .shader = 0,
    .type = GL_FRAGMENT_SHADER,
};

struct /* uniform */
{
    int matrix_projection;
} uniform;

struct /* matrix */
{
    m4f32 translation;
    m4f32 rotation;
    m4f32 orientation;
    m4f32 view;
    m4f32 projection;
    m4f32 gizmo;
} matrix;

f32 sinpitch, cospitch, sinyaw, cosyaw;
GLuint vao0, vbo0, ebo0;
GLuint vao1, vbo1, ebo1;

/* ---- callbacks ----------------------------------------------------------- */
void error_callback(int error, const char* message)
{
    LOGERROR("%s", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

/* ---- signatures ---------------------------------------------------------- */
void update_input(GLFWwindow *win);
void bind_shader_uniforms();
void update_shader_uniforms();
void update_camera_movement(Camera *camera);
void update_camera_perspective(Camera *camera);
void draw_graphics();

int main(void)
{
    glfwSetErrorCallback(error_callback);
    /*temp*/ render.size = (v2i32){1080, 720};

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
    bind_mesh(&vao0, &vbo0, &ebo0, VERTEX_DATA_GIZMO, INDEX_COUNT_GIZMO);
    if (init_shaders() != 0)
        goto cleanup;

    bind_shader_uniforms();

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
        update_camera_movement(&camera);
        render.cursor_delta = (v2f64){0.0f, 0.0f};
        update_camera_perspective(&camera);
        draw_graphics();

        glfwSwapBuffers(render.window);
        glfwPollEvents();
        update_input(render.window);
    }

cleanup: /* ----------------------------------------------------------------- */

    glDeleteVertexArrays(1, &vao0);
    glDeleteBuffers(1, &vbo0);
    glDeleteBuffers(1, &ebo0);
    glDeleteProgram(shader_program);
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
void update_input(GLFWwindow *win)
{
    /* ---- movement -------------------------------------------------------- */
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.pos.x += (movement_speed * sinyaw);
        camera.pos.y += (movement_speed * cosyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.pos.x -= (movement_speed * sinyaw);
        camera.pos.y -= (movement_speed * cosyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.pos.x -= (movement_speed * cosyaw);
        camera.pos.y += (movement_speed * sinyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.pos.x += (movement_speed * cosyaw);
        camera.pos.y -= (movement_speed * sinyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.pos.z += movement_speed;
    }

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.pos.z -= movement_speed;
    }

    /* ---- debug ----------------------------------------------------------- */
    if (glfwGetKey(win, GLFW_KEY_PAUSE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GL_TRUE);
}

void bind_shader_uniforms()
{
    uniform.matrix_projection = 
        glGetUniformLocation(shader_program, "mat_projection");
}

void update_shader_uniforms()
{
    glUniformMatrix4fv(uniform.matrix_projection, 1, GL_FALSE, (GLfloat*)&matrix.projection);
}

void update_camera_movement(Camera *camera)
{
    const f32 ANGLE = 90.0f;
    const f32 RANGE = 360.0f;

    camera->rot.y += render.cursor_delta.y;
    camera->rot.z += render.cursor_delta.x;

    camera->rot.z = fmod(camera->rot.z, RANGE);
    if (camera->rot.z < 0.0f) camera->rot.z += RANGE;
    camera->rot.y = clamp_f32(camera->rot.y, -ANGLE, ANGLE);

    sinpitch =  sinf((camera->rot.y) * DEG2RAD);
    cospitch =  cosf((camera->rot.y) * DEG2RAD);
    sinyaw =    sinf((camera->rot.z) * DEG2RAD);
    cosyaw =    cosf((camera->rot.z) * DEG2RAD);
}

void update_camera_perspective(Camera *camera)
{
    /* ---- translation ----------------------------------------------------- */
    matrix.translation =
        (m4f32){
            1.0f,           0.0f,           0.0f,           0.0f,
            0.0f,           1.0f,           0.0f,           0.0f,
            0.0f,           0.0f,           1.0f,           0.0f,
            -camera->pos.x, -camera->pos.y, -camera->pos.z, 1.0f,
        };

    /* ---- rotation: yaw --------------------------------------------------- */
    matrix.rotation =
        (m4f32){
            cosyaw,     sinyaw, 0.0f, 0.0f,
            -sinyaw,    cosyaw, 0.0f, 0.0f,
            0.0f,       0.0f,   1.0f, 0.0f,
            0.0f,       0.0f,   0.0f, 1.0f,
        };

    /* ---- rotation: pitch ------------------------------------------------- */
    matrix.rotation = matrix_multiply(matrix.rotation,
            (m4f32){
            cospitch,   0.0f,   sinpitch,   0.0f,
            0.0f,       1.0f,   0.0f,       0.0f,
            -sinpitch,  0.0f,   cospitch,   0.0f,
            0.0f,       0.0f,   0.0f,       1.0f,
            });

    /* ---- orientation: z-up ----------------------------------------------- */
    matrix.orientation =
        (m4f32){
            0.0f,   0.0f, -1.0f, 0.0f,
            -1.0f,  0.0f, 0.0f, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
        };

    /* ---- view ------------------------------------------------------------ */
    matrix.view =
        matrix_multiply(matrix.translation,
            matrix_multiply(matrix.rotation, matrix.orientation));

    /* ---- projection ------------------------------------------------------ */
    f32 ratio = (f32)render.size.x / (f32)render.size.y;
    f32 fov = 1.0f / tanf((camera->fov / 2.0f) * DEG2RAD);
    f32 far = camera->far;
    f32 near = camera->near;
    f32 clip = -(far + near) / (far - near);
    f32 offset = -(2.0f * far * near) / (far - near);

    matrix.projection = matrix_multiply(matrix.view,
            (m4f32){
            fov / ratio,    0.0f,   0.0f,   0.0f,
            0.0f,           fov,    0.0f,   0.0f,
            0.0f,           0.0f,   clip,  -1.0f,
            0.0f,           0.0f,   offset, 0.0f,
            });
}

void draw_graphics()
{
    /* ---- ready shaders --------------------------------------------------- */
    glClearColor(0.69f, 0.86f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);

    /* ---- draw ------------------------------------------------------------ */
    update_shader_uniforms();

    glBindVertexArray(vao0);
    glDrawElements(GL_TRIANGLES, 90, GL_UNSIGNED_INT, 0);
}

