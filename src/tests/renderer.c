#include "../logic.c"
#include "../engine/rendering.c"

// ---- declarations -----------------------------------------------------------
Window render =
{
    .size = {1280, 720},
    .title = "glfw_window",
};
int uniform_cursor_pos;
int uniform_render_size;

// ---- signatures -------------------------------------------------------------
void error_callback(int error, const char* message)
{
    LOGERROR("%s", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double cursor_x, double cursor_y);
static void gl_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void draw_graphics();

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!init_glfw())
        return -1;
    if (!init_window(&render))
        return -1;
    if (!init_glew(&render))
        return -1;

    // ---- set callbacks ------------------------------------------------------
    glfwSetFramebufferSizeCallback(render.window, gl_frame_buffer_size_callback);
    glfwSetCursorPosCallback(render.window, gl_cursor_pos_callback);
    glfwSetKeyCallback(render.window, gl_key_callback);

    bind_buffers();
    if (init_shaders() != 0)
        goto cleanup;

    uniform_cursor_pos =
        glGetUniformLocation(shader_program, "cursor_pos");
    uniform_render_size =
        glGetUniformLocation(shader_program, "render_size");

    // ---- main loop ----------------------------------------------------------
    while (!glfwWindowShouldClose(render.window))
    {
        draw_graphics();

        glfwSwapBuffers(render.window);
        glfwWaitEvents();
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

static void gl_cursor_pos_callback(GLFWwindow* window, double cursor_x, double cursor_y)
{
    glfwGetCursorPos(window, &render.cursor.x, &render.cursor.y);
}

static void gl_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // ---- movement -----------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        LOGINFO("%s", "movement");

    // ---- debug --------------------------------------------------------------
    if ((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_Q)) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void draw_graphics()
{
    // ---- ready shaders ------------------------------------------------------
    glClearColor(0.69f, 0.86f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);

    // ---- set shader uniforms ------------------------------------------------
    glUniform2f(uniform_cursor_pos,
            render.cursor.x, render.cursor.y);
    glUniform2f(uniform_render_size,
            render.size.x, render.size.y);

    // ---- draw ---------------------------------------------------------------
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

