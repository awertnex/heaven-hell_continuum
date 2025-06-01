#define VECTOR4_TYPES
#define MATRIX4_TYPES
#include "../engine/rendering.c"

#define DIR_SHADERS "src/engine/shaders/"

// ---- declarations -----------------------------------------------------------
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
    m4f32 model;
    m4f32 view;
    m4f32 projection;
} matrix;

f64 sinpitch, cospitch, sinyaw, cosyaw;
GLuint vao, vbo, ebo;

// ---- signatures -------------------------------------------------------------
void error_callback(int error, const char* message)
{
    LOGERROR("%s", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void update_cursor_delta();
void bind_shader_uniforms();
void update_camera_movement();
void update_camera_transforms();
void update_shader_uniforms();
void draw_graphics();
void update_input(GLFWwindow *win);

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (init_glfw() != 0)
        return -1;
    if (init_window(&render) != 0)
        return -1;
    if (init_glew(&render) != 0)
        return -1;

    // ---- set callbacks ------------------------------------------------------
    glfwSetFramebufferSizeCallback(render.window, gl_frame_buffer_size_callback);
    glfwSetCursorPosCallback(render.window, gl_cursor_pos_callback);

    bind_mesh(&vao, &vbo, &ebo);
    if (init_shaders() != 0)
        goto cleanup;

    bind_shader_uniforms();

    camera =
        (Camera){
            .pos = (v3f32){-1.0f, -0.5f, 0.5f},
            .rot = (v3f32){0.0f, 0.0f, -100.0f},
            .fov = 70.0f,
        };

    // ---- main loop ----------------------------------------------------------
    while (!glfwWindowShouldClose(render.window))
    {
        update_cursor_delta();

        update_camera_movement();
        update_camera_transforms();
        draw_graphics();

        render.cursor_delta.x = 0.0f;
        render.cursor_delta.y = 0.0f;
        glfwSwapBuffers(render.window);
        glfwPollEvents();
        update_input(render.window);
    }

cleanup: // --------------------------------------------------------------------

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
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

void update_cursor_delta()
{
    if (render.cursor_last.x != render.cursor.x)
        render.cursor_delta.x = render.cursor.x - render.cursor_last.x;
    if (render.cursor_last.y != render.cursor.y)
        render.cursor_delta.y = render.cursor.y - render.cursor_last.y;
    glfwGetCursorPos(render.window, &render.cursor_last.x, &render.cursor_last.y);
}

static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    glfwGetCursorPos(window, &render.cursor.x, &render.cursor.y);
}

void bind_shader_uniforms()
{
    uniform.matrix_projection = 
        glGetUniformLocation(shader_program, "mat_projection");
}

void update_camera_movement()
{
    camera.rot.x -= render.cursor_delta.y;
    camera.rot.z += render.cursor_delta.x;

    camera.rot.z = fmod(camera.rot.z, 360.0f);
    if (camera.rot.z < 0.0f) camera.rot.z += 360.0f;
    if (camera.rot.x > 90.0f) camera.rot.x = 90.0f;
    else if (camera.rot.x < -90.0f) camera.rot.x = -90.0f;

    sinpitch =  sin((camera.rot.x + 90.0f) * DEG2RAD);
    cospitch =  cos((camera.rot.x + 90.0f) * DEG2RAD);
    sinyaw =    sin(camera.rot.z * DEG2RAD);
    cosyaw =    cos(camera.rot.z * DEG2RAD);
}

void update_camera_transforms()
{
    matrix.model =
        (m4f32){
            1.0f, 0.0f, 0.0f, -camera.pos.x,
            0.0f, 1.0f, 0.0f, -camera.pos.y,
            0.0f, 0.0f, 1.0f, -camera.pos.z,
            0.0f, 0.0f, 0.0f, -1.0f,
        };

    matrix.view = matrix_multiply(
            (m4f32){ // ---- rotation x ----------------------------------------
            1.0f,   0.0f,       0.0f,       0.0f,
            0.0f,   cospitch,   -sinpitch,  0.0f,
            0.0f,   sinpitch,   cospitch,   0.0f,
            0.0f,   0.0f,       0.0f,       1.0f,
            },
            (m4f32){ // ---- rotation z ----------------------------------------
            cosyaw, -sinyaw,    0.0f,   0.0f,
            sinyaw, cosyaw,     0.0f,   0.0f,
            0.0f,   0.0f,       1.0f,   0.0f,
            0.0f,   0.0f,       0.0f,   1.0f,
            });

    matrix.view = matrix_multiply(matrix.view, matrix.model);

    f32 clip_far = GL_CLIP_DISTANCE0;
    f32 clip_near = 0.01f;

    f32 ratio = (f32)render.size.y / (f32)render.size.x;
    f32 fov = 1.0f / tanf(camera.fov / 2.0f);
    f32 clip = clip_far / (clip_far - clip_near);
    f32 offset = (-clip_far * clip_near) / (clip_far - clip_near);

    matrix.projection =
        (m4f32){
            ratio * fov, 0.0f, 0.0f, 0.0f,
            0.0f, fov, 0.0f, 0.0f,
            0.0f, 0.0f, clip, offset,
            0.0f, 0.0f, 1.0f, 0.0f,
        };

    matrix.projection = matrix_multiply(matrix.projection, matrix.view);
}

void update_shader_uniforms()
{
    glUniformMatrix4fv(uniform.matrix_projection, 1, GL_TRUE, (GLfloat*)&matrix.projection);
}

void draw_graphics()
{
    // ---- ready shaders ------------------------------------------------------
    glClearColor(0.69f, 0.86f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);

    // ---- draw ---------------------------------------------------------------
    update_shader_uniforms();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
}

f64 movement_speed = 0.1f;
void update_input(GLFWwindow *win)
{
    // ---- movement -----------------------------------------------------------
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.pos.x += (movement_speed * cosyaw);
        camera.pos.y -= (movement_speed * sinyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.pos.x -= (movement_speed * cosyaw);
        camera.pos.y += (movement_speed * sinyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.pos.x += (movement_speed * sinyaw);
        camera.pos.y += (movement_speed * cosyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.pos.x -= (movement_speed * sinyaw);
        camera.pos.y -= (movement_speed * cosyaw);
    }

    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.pos.z += movement_speed;
    }

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.pos.z -= movement_speed;
    }

    // ---- debug --------------------------------------------------------------
    if (glfwGetKey(win, GLFW_KEY_PAUSE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GL_TRUE);
}

