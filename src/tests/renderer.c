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
    int matrix_perspective;
} uniform;

struct /* matrix */
{
    m4f32 model;
    m4f32 view;
    m4f32 projection;
    m4f32 perspective;
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
void bind_shader_uniforms();
void update_camera_movement();
void update_camera_perspective(Camera *camera);
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    camera =
        (Camera){
            .pos = (v3f32){-1.0f, -0.5f, 0.5f},
            .rot = (v3f64){-1.0f, 0.0f, 57.0f},
            .fov = 45.0f,
            .far = 1000.0f,
            .near = 0.05f,
        };

    // ---- main loop ----------------------------------------------------------
    while (!glfwWindowShouldClose(render.window))
    {
        printf("     xyz[%7.2f %7.2f %7.2f]\npitchyaw[%7.2lf %7.2lf        ]\n\n", //temp
                camera.pos.x,
                camera.pos.y,
                camera.pos.z,
                camera.rot.x,
                camera.rot.z);

        update_cursor_delta();

        update_camera_movement();
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
    uniform.matrix_perspective= 
        glGetUniformLocation(shader_program, "mat_perspective");
}

void update_camera_movement()
{
    static const f64 PITCH_MAX = 90.0f;
    static const f64 YAW_MAX = 360.0f;

    camera.rot.x -= render.cursor_delta.y;
    camera.rot.z += render.cursor_delta.x;

    camera.rot.z = fmod(camera.rot.z, YAW_MAX);
    if (camera.rot.z < 0.0f) camera.rot.z += YAW_MAX;
    camera.rot.x = clamp_f64(camera.rot.x, -PITCH_MAX, PITCH_MAX);

    sinpitch =  sin((camera.rot.x + PITCH_MAX) * DEG2RAD);
    cospitch =  cos((camera.rot.x + PITCH_MAX) * DEG2RAD);
    sinyaw =    sin(camera.rot.z * DEG2RAD);
    cosyaw =    cos(camera.rot.z * DEG2RAD);
}

void update_camera_perspective(Camera *camera)
{
    // ---- rotation: x --------------------------------------------------------
    matrix.view =
        (m4f32){
            1.0f,   0.0f,       0.0f,       0.0f,
            0.0f,   cospitch,   -sinpitch,  0.0f,
            0.0f,   sinpitch,   cospitch,   0.0f,
            0.0f,   0.0f,       0.0f,       1.0f,
        };

    // ---- rotation: z --------------------------------------------------------
    matrix.view = matrix_multiply(matrix.view,
            (m4f32){
            cosyaw, -sinyaw,    0.0f,       0.0f,
            sinyaw, cosyaw,     0.0f,       0.0f,
            0.0f,   0.0f,       1.0f,       0.0f,
            0.0f,   0.0f,       0.0f,       1.0f,
            });

    // ---- translation --------------------------------------------------------
    matrix.view = matrix_multiply(matrix.view,
            (m4f32){
            1.0f,   0.0f,       0.0f,       -camera->pos.x,
            0.0f,   1.0f,       0.0f,       -camera->pos.y,
            0.0f,   0.0f,       1.0f,       -camera->pos.z,
            0.0f,   0.0f,       0.0f,       -1.0f,
            });

    // ---- projection ---------------------------------------------------------
    f32 far = camera->far;
    f32 near = camera->near;

    f32 ratio = (f32)render.size.y / (f32)render.size.x;
    f32 fov = 1.0f / tanf(camera->fov * DEG2RAD);
    f32 clip = far / (far - near);
    f32 offset = (-far * near) / (far - near);

    matrix.projection =
        (m4f32){
            ratio * fov,    0.0f,   0.0f,   0.0f,
            0.0f,                   fov,    0.0f,   0.0f,
            0.0f,           0.0f,   clip,   offset,
            0.0f,           0.0f,   1.0f,   0.0f,
        };

    matrix.perspective = matrix_multiply(matrix.projection, matrix.view);
}

void update_shader_uniforms()
{
    glUniformMatrix4fv(uniform.matrix_perspective, 1, GL_TRUE, (GLfloat*)&matrix.perspective);
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

f64 movement_speed = 0.04f;
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


/*
	frustumLH_ZO(left, right, bottom, top, nearVal, farVal)
	{
		matrix[0][0] = (2 * nearVal) / (right - left);
		matrix[1][1] = (2 * nearVal) / (top - bottom);
		matrix[2][0] = -(right + left) / (right - left);
		matrix[2][1] = -(top + bottom) / (top - bottom);
		matrix[2][2] = farVal / (farVal - nearVal);
		matrix[2][3] = 1;
		matrix[3][2] = -(farVal * nearVal) / (farVal - nearVal);
	}

	frustumLH_NO(T left, T right, T bottom, T top, T nearVal, T farVal)
	{
		matrix[0][0] = (2 * nearVal) / (right - left);
		matrix[1][1] = (2 * nearVal) / (top - bottom);
		matrix[2][0] = -(right + left) / (right - left);
		matrix[2][1] = -(top + bottom) / (top - bottom);
		matrix[2][2] = (farVal + nearVal) / (farVal - nearVal);
		matrix[2][3] = 1;
		matrix[3][2] = - (2 * farVal * nearVal) / (farVal - nearVal);
	}

	frustumRH_ZO(T left, T right, T bottom, T top, T nearVal, T farVal)
	{
		matrix[0][0] = (2 * nearVal) / (right - left);
		matrix[1][1] = (2 * nearVal) / (top - bottom);
		matrix[2][0] = (right + left) / (right - left);
		matrix[2][1] = (top + bottom) / (top - bottom);
		matrix[2][2] = farVal / (nearVal - farVal);
		matrix[2][3] = -1;
		matrix[3][2] = -(farVal * nearVal) / (farVal - nearVal);
	}

	frustumRH_NO(T left, T right, T bottom, T top, T nearVal, T farVal)
	{
		matrix[0][0] = (2 * nearVal) / (right - left);
		matrix[1][1] = (2 * nearVal) / (top - bottom);
		matrix[2][0] = (right + left) / (right - left);
		matrix[2][1] = (top + bottom) / (top - bottom);
		matrix[2][2] = - (farVal + nearVal) / (farVal - nearVal);
		matrix[2][3] = -1;
		matrix[3][2] = - (2 * farVal * nearVal) / (farVal - nearVal);
	}


	perspectiveRH_ZO(T fovy, T aspect, T cNear, T cFar)
	{
		assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

		T const fov = tan(fovy / 2);

		matrix[0][0] = 1 / (aspect * fov);
		matrix[1][1] = 1 / (fov);
		matrix[2][2] = cFar / (cNear - cFar);
		matrix[2][3] = - 1;
		matrix[3][2] = -(cFar * cNear) / (cFar - cNear);
	}

	perspectiveRH_NO(T fovy, T aspect, T cNear, T cFar)
	{
		assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

		T const fov = tan(fovy / 2);

		matrix[0][0] = 1 / (aspect * fov);
		matrix[1][1] = 1 / (fov);
		matrix[2][2] = - (cFar + cNear) / (cFar - cNear);
		matrix[2][3] = - 1;
		matrix[3][2] = - (2 * cFar * cNear) / (cFar - cNear);
	}

	perspectiveLH_ZO(T fovy, T aspect, T cNear, T cFar)
	{
		assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

		T const fov = tan(fovy / 2);

		matrix[0][0] = 1 / (aspect * fov);
		matrix[1][1] = 1 / (fov);
		matrix[2][2] = cFar / (cFar - cNear);
		matrix[2][3] = 1;
		matrix[3][2] = -(cFar * cNear) / (cFar - cNear);
	}

	perspectiveLH_NO(T fovy, T aspect, T cNear, T cFar)
	{
		assert(abs(aspect - std::numeric_limits<T>::epsilon()) > 0);

		T const fov = tan(fovy / 2);

		matrix[0][0] = 1 / (aspect * fov);
		matrix[1][1] = 1 / (fov);
		matrix[2][2] = (cFar + cNear) / (cFar - cNear);
		matrix[2][3] = 1;
		matrix[3][2] = - (2 * cFar * cNear) / (cFar - cNear);
	}
*/

