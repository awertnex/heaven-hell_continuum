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

// ---- callbacks --------------------------------------------------------------
void error_callback(int error, const char* message)
{
    LOGERROR("%s", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

// ---- signatures -------------------------------------------------------------
void update_cursor_delta();
void update_input(GLFWwindow *win);
void bind_shader_uniforms();
void update_shader_uniforms();
void update_camera_movement(Camera *camera);
void update_camera_perspective(Camera *camera);
void draw_graphics();

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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    camera =
        (Camera){
            .pos = (v3f32){-0.5f, -0.5f, -0.5f},
            .rot = (v3f64){0.0f, 35.0f, 315.0f},
            .fov = 70.0f,
            .far = 1000.0f,
            .near = 0.05f,
        };
    //camera.pos = (v3f32){0.0f, 0.0f, 0.0f};
    //camera.rot = (v3f64){0.0f, 0.0f, 0.0f};

    glfwSetCursorPos(render.window, 3.0f, 3.0f);
    // ---- main loop ----------------------------------------------------------
    while (!glfwWindowShouldClose(render.window))
    {
        update_cursor_delta();
        LOGINFO("  cursor[%7.2f %7.2f      ]", render.cursor.x, render.cursor.y);
        LOGINFO("   delta[%7.2f %7.2f      ]", render.cursor_delta.x, render.cursor_delta.y);
        LOGINFO("     xyz[%7.2f %7.2f %7.2f]", camera.pos.x, camera.pos.y, camera.pos.z);
        LOGINFO("pitchyaw[      %7.2f %7.2f]\n", camera.rot.y, camera.rot.z);
        update_camera_movement(&camera);
        update_camera_perspective(&camera);
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

static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    glfwGetCursorPos(window, &render.cursor.x, &render.cursor.y);
}

void update_cursor_delta()
{
    if (render.cursor_last.x != render.cursor.x)
        render.cursor_delta.x = render.cursor.x - render.cursor_last.x;
    if (render.cursor_last.y != render.cursor.y)
        render.cursor_delta.y = render.cursor.y - render.cursor_last.y;
    glfwGetCursorPos(render.window, &render.cursor_last.x, &render.cursor_last.y);
}

f64 movement_speed = 0.04f;
void update_input(GLFWwindow *win)
{
    // ---- movement -----------------------------------------------------------
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

    // ---- debug --------------------------------------------------------------
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
    const f64 ANGLE = 90.0f;
    const f64 RANGE = 360.0f;

    camera->rot.y += render.cursor_delta.y;
    camera->rot.z += render.cursor_delta.x;

    camera->rot.z = fmod(camera->rot.z, RANGE);
    if (camera->rot.z < 0.0f) camera->rot.z += RANGE;
    camera->rot.y = clamp_f64(camera->rot.y, -ANGLE, ANGLE);

    sinpitch =  sin((camera->rot.y) * DEG2RAD);
    cospitch =  cos((camera->rot.y) * DEG2RAD);
    sinyaw =    sin((camera->rot.z) * DEG2RAD);
    cosyaw =    cos((camera->rot.z) * DEG2RAD);
}

void update_camera_perspective(Camera *camera)
{
    // ---- translation --------------------------------------------------------
    matrix.view =
            (m4f32){
            1.0f,           0.0f,           0.0f,           0.0f,
            0.0f,           1.0f,           0.0f,           0.0f,
            0.0f,           0.0f,           1.0f,           0.0f,
            -camera->pos.x, -camera->pos.y, -camera->pos.z, 1.0f,
            };

    // ---- rotation: yaw ------------------------------------------------------
    matrix.view = matrix_multiply(matrix.view,
            (m4f32){
            cosyaw,     sinyaw, 0.0f, 0.0f,
            -sinyaw,    cosyaw, 0.0f, 0.0f,
            0.0f,       0.0f,   1.0f, 0.0f,
            0.0f,       0.0f,   0.0f, 1.0f,
            });

    // ---- rotation: pitch ----------------------------------------------------
    matrix.view = matrix_multiply(matrix.view,
            (m4f32){
            cospitch,   0.0f,   sinpitch,   0.0f,
            0.0f,       1.0f,   0.0f,       0.0f,
            -sinpitch,  0.0f,   cospitch,   0.0f,
            0.0f,       0.0f,   0.0f,       1.0f,
            });

    // ---- orientation: z-up --------------------------------------------------
    matrix.view = matrix_multiply(matrix.view,
        (m4f32){
            0.0f,   0.0f, -1.0f, 0.0f,
            -1.0f,  0.0f, 0.0f, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
        });

    // ---- projection ---------------------------------------------------------
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
            0.0f,           0.0f,   clip,   -1.0f,
            0.0f,           0.0f,   offset, 0.0f,
            });
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
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


/*
   frustumLH_ZO(left, right, bottom, top, nearVal, farVal)
   {
   m00 = (2 * nearVal) / (right - left);
   m11 = (2 * nearVal) / (top - bottom);
   m20 = -(right + left) / (right - left);
   m21 = -(top + bottom) / (top - bottom);
   m22 = farVal / (farVal - nearVal);
   m32 = -(farVal * nearVal) / (farVal - nearVal);

   matrix =
   {
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, 1, 0,
   };
   }

   frustumLH_NO(left, right, bottom, top, nearVal, farVal)
   {
    m00 = (2 * nearVal) / (right - left);
    m11 = (2 * nearVal) / (top - bottom);
    m20 = -(right + left) / (right - left);
    m21 = -(top + bottom) / (top - bottom);
    m22 = (farVal + nearVal) / (farVal - nearVal);
    m32 = - (2 * farVal * nearVal) / (farVal - nearVal);

   matrix =
   {
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, 1, 0,
   };
   }

   frustumRH_ZO(left, right, bottom, top, nearVal, farVal)
   {
    m00 = (2 * nearVal) / (right - left);
    m11 = (2 * nearVal) / (top - bottom);
    m20 = (right + left) / (right - left);
    m21 = (top + bottom) / (top - bottom);
    m22 = farVal / (nearVal - farVal);
    m32 = -(farVal * nearVal) / (farVal - nearVal);

   matrix =
   {
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, -1, 0,
   };
   }

   frustumRH_NO(left, right, bottom, top, nearVal, farVal)
   {
    m00 = (2 * nearVal) / (right - left);
    m11 = (2 * nearVal) / (top - bottom);
    m20 = (right + left) / (right - left);
    m21 = (top + bottom) / (top - bottom);
    m22 = - (farVal + nearVal) / (farVal - nearVal);
    m32 = - (2 * farVal * nearVal) / (farVal - nearVal);

   matrix =
   {
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, 0, 0,
   0, 0, -1, 0,
   };
}


perspectiveRH_ZO(fovy, aspect, cNear, cFar)
{
    assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

    fov = tan(fovy / 2);
    clip = cFar / (cNear - cFar);
    offset = -(cFar * cNear) / (cFar - cNear);

    matrix =
    {
        1 / (aspect * fov), 0,          0,      0,
        0,                  1 / fov,    0,      0,
        0,                  0,          clip,   offset,
        0,                  0,          -1,     0,
    };

}

perspectiveRH_NO(fovy, aspect, cNear, cFar)
{
    assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

    fov = tan(fovy / 2);
    clip = - (cFar + cNear) / (cFar - cNear);
    offset = - (2 * cFar * cNear) / (cFar - cNear);

    matrix =
    {
        1 / (aspect * fov), 0,          0,      0,
        0,                  1 / fov,    0,      0,
        0,                  0,          clip,   offset,
        0,                  0,          -1,     0,
    };
}

perspectiveLH_ZO(fovy, aspect, cNear, cFar)
{
    assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

    fov = tan(fovy / 2);
    clip = cFar / (cFar - cNear);
    offset = -(cFar * cNear) / (cFar - cNear);

    matrix =
    {
        1 / (aspect * fov), 0,          0,      0,
        0,                  1 / fov,    0,      0,
        0,                  0,          clip,   offset,
        0,                  0,          1,      0,
    };
}

perspectiveLH_NO(T fovy, T aspect, T cNear, T cFar)
{
    assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

    const fov = tan(fovy / 2);
    clip = (cFar + cNear) / (cFar - cNear);
    offset = - (2 * cFar * cNear) / (cFar - cNear);

    matrix =
    {
        1 / (aspect * fov), 0,          0,      0,
        0,                  1 / fov,    0,      0,
        0,                  0,          clip,   offset,
        0,                  0,          1,      0,
    };
}
*/

