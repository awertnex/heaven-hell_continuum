#include <stdio.h>
#include <stdlib.h>

#include "h/rendering.h"
#include "math.c"
#include "logger.c"

// ---- declarations -----------------------------------------------------------
GLuint shader_program;
const unsigned int shader_attribute = 0;

const GLfloat thickness = 0.1f;
GLfloat coordinate_vertices[39] =
{
    0.0f, 0.0f, 0.0f,
    thickness, thickness, 0.0f,
    thickness, 0.0f, thickness,
    0.0f, thickness, thickness,

    1.0f, 0.0f, 0.0f,
    1.0f, thickness, 0.0f,
    1.0f, 0.0f, thickness,

    0.0f, 1.0f, 0.0f,
    thickness, 1.0f, 0.0f,
    0.0f, 1.0f, thickness,

    0.0f, 0.0f, 1.0f,
    thickness, 0.0f, 1.0f,
    0.0f, thickness, 1.0f,
};

GLuint coordinate_indices[36] =
{
    0, 2, 6, 6, 4, 0,
    0, 4, 5, 5, 1, 0,

    0, 1, 8, 8, 7, 0,
    0, 7, 9, 9, 3, 0,

    0, 3, 12, 12, 10, 0,
    0, 10, 11, 11, 2, 0,
};

GLfloat vertices[18] =
{
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
};

GLuint indices[12] = {0, 1, 2, 2, 3, 0, 2, 3, 4, 4, 5, 2};

// ---- functions --------------------------------------------------------------
int init_glfw()
{
    if (!glfwInit())
    {
        LOGFATAL("%s", "Failed to Initialize GLFW, Process Aborted");
        glfwTerminate();
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return 0;
}

int init_window(Window *render)
{
    render->window = glfwCreateWindow(render->size.x, render->size.y, render->title, NULL, NULL);
    if (!render->window)
    {
        LOGFATAL("%s", "Failed to Initialize Window or OpenGL Context, Process Aborted");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(render->window);
    glfwSetWindowIcon(render->window, 1, &render->icon);
    return 0;
}

int init_glew(Window *render)
{
    if (glewInit() != GLEW_OK)
    {
        LOGFATAL("%s", "Failed to Initialize GLEW, Process Aborted");
        glfwTerminate();
        return -1;
    }
    return 0;
}

void bind_mesh(GLuint *vao, GLuint *vbo, GLuint *ebo)
{
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinate_vertices), coordinate_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(coordinate_indices), coordinate_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

int load_shader_from_disk(Shader *shader)
{
    FILE *shader_file;
    if ((shader_file = fopen(shader->file_name, "r")) == NULL)
    {
        LOGFATAL("File '%s' Not Found, Process Aborted", shader->file_name);
        return -1;
    }

    char c;
    int i = 0;
    while ((i < sizeof(shader->source) - 1)
            && (c = getc(shader_file)) != EOF)
    {
        shader->source[i] = c;
        ++i;
    }
    shader->source[i] = '\0';
    fclose(shader_file);
    return 0;
}

int init_shader(Shader *shader)
{
    if (load_shader_from_disk(shader) != 0)
        return -1;

    const GLchar *shader_source_loaded = shader->source;

    shader->shader = glCreateShader(shader->type);
    glShaderSource(shader->shader, 1, &shader_source_loaded, NULL);
    glCompileShader(shader->shader);

    char infoLog[512];
    glGetShaderiv(shader->shader, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        glGetShaderInfoLog(shader->loaded, 512, NULL, infoLog);
        LOGERROR("SHADER: %s '%s'", infoLog, shader->file_name);
        return -1;
    }
    else LOGDEBUG("SHADER: %d '%s' Loaded", shader->loaded, shader->file_name);
    return 0;
}

int init_shader_program()
{
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader.shader);
    glAttachShader(shader_program, fragment_shader.shader);
    glLinkProgram(shader_program);

    int success;
    char infoLog[512];
    glGetProgramiv(shader_program, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        LOGERROR("SHADER PROGRAM: %s", infoLog);
        return -1;
    }
    else LOGDEBUG("SHADER PROGRAM: %d", success);
    return 0;
}

int init_shaders()
{
    if (init_shader(&vertex_shader) != 0
            || init_shader(&fragment_shader) != 0
            || init_shader_program() != 0)
        return -1;

    glDeleteShader(vertex_shader.shader);
    glDeleteShader(fragment_shader.shader);
    return 0;
}

