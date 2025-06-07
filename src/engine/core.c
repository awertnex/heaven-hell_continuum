#include <stdio.h>
#include <stdlib.h>

#include "h/core.h"
#include "h/memory.h"
#include "math.c"
#include "logger.c"

/* ---- functions ----------------------------------------------------------- */
int init_glfw(void)
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
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    return 0;
}

int init_glew(void)
{
    if (glewInit() != GLEW_OK)
    {
        LOGFATAL("%s", "Failed to Initialize GLEW, Process Aborted");
        glfwTerminate();
        return -1;
    }
    return 0;
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

    shader->id = glCreateShader(shader->type);
    glShaderSource(shader->id, 1, &shader_source_loaded, NULL);
    glCompileShader(shader->id);

    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        char log[512];
        glGetShaderInfoLog(shader->id, 512, NULL, log);
        LOGERROR("SHADER: '%s' %s", shader->file_name, log);
        return -1;
    }
    else LOGDEBUG("SHADER: %d '%s' Loaded", shader->id, shader->file_name);

    return 0;
}

int init_shader_program(ShaderProgram *program)
{
    if (init_shader(&program->vertex) != 0)
        return -1;
    if (init_shader(&program->fragment) != 0)
        return -1;

    program->id = glCreateProgram();
    glAttachShader(program->id, program->vertex.id);
    glAttachShader(program->id, program->fragment.id);
    glLinkProgram(program->id);

    glGetProgramiv(program->id, GL_LINK_STATUS, &program->loaded);
    if (!program->loaded)
    {
        char log[512];
        glGetProgramInfoLog(program->id, 512, NULL, log);
        LOGERROR("SHADER PROGRAM: '%s' %s", program->name, log);
        return -1;
    }
    else LOGDEBUG("SHADER PROGRAM: %d '%s' Loaded", program->id, program->name);

    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

    return 0;
}

/* usage = GL_<x>_DRAW; */
int generate_mesh(Mesh *mesh, GLenum usage, GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data)
{
    MC_C_ALLOC(mesh->vbo_data, sizeof(GLfloat) * vbo_len);
    MC_C_ALLOC(mesh->ebo_data, sizeof(GLuint) * ebo_len);

    mesh->vbo_len = vbo_len;
    mesh->ebo_len = ebo_len;

    memcpy(mesh->vbo_data, vbo_data, sizeof(GLfloat) * vbo_len);
    memcpy(mesh->ebo_data, ebo_data, sizeof(GLuint) * ebo_len);

    /* ---- bind mesh ------------------------------------------------------- */
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    glBufferData(GL_ARRAY_BUFFER, mesh->vbo_len * sizeof(GLfloat), mesh->vbo_data, usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo_len * sizeof(GLuint), mesh->ebo_data, usage);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return 0;

cleanup:
    delete_mesh(mesh);
    return -1;
}

void draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->ebo_len, GL_UNSIGNED_INT, 0);
}

void delete_mesh(Mesh *mesh)
{
    if (mesh->vbo_data == NULL || mesh->ebo_data == NULL) return;

    MC_C_CLEAR_MEM(mesh->vbo_data, sizeof(GLfloat) * mesh->vbo_len);
    MC_C_FREE(mesh->vbo_data, sizeof(GLfloat) * mesh->vbo_len);

    MC_C_CLEAR_MEM(mesh->ebo_data, sizeof(GLuint) * mesh->ebo_len);
    MC_C_FREE(mesh->ebo_data, sizeof(GLuint) * mesh->ebo_len);

    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
}

void update_camera_movement(v2f64 cursor_delta, Camera *camera)
{
    const f32 ANGLE = 90.0f;
    const f32 RANGE = 360.0f;

    camera->rot.y += cursor_delta.y;
    camera->rot.z += cursor_delta.x;

    camera->rot.z = fmodf(camera->rot.z, RANGE);
    if (camera->rot.z < 0.0f) camera->rot.z += RANGE;
    camera->rot.y = clamp_f32(camera->rot.y, -ANGLE, ANGLE);

    camera->sin_pitch = sinf((camera->rot.y) * DEG2RAD);
    camera->cos_pitch = cosf((camera->rot.y) * DEG2RAD);
    camera->sin_yaw =   sinf((camera->rot.z) * DEG2RAD);
    camera->cos_yaw =   cosf((camera->rot.z) * DEG2RAD);
}

void update_camera_perspective(Camera *camera, Projection *projection)
{
    f32 spch = camera->sin_pitch;
    f32 cpch = camera->cos_pitch;
    f32 syaw = camera->sin_yaw;
    f32 cyaw = camera->cos_yaw;

    f32 ratio = camera->ratio;
    f32 fov = 1.0f / tanf((camera->fov / 2.0f) * DEG2RAD);
    f32 far = camera->far;
    f32 near = camera->near;
    f32 clip = -(far + near) / (far - near);
    f32 offset = -(2.0f * far * near) / (far - near);

    /* ---- translation ----------------------------------------------------- */
    projection->translation =
        (m4f32){
            1.0f,           0.0f,           0.0f,           0.0f,
            0.0f,           1.0f,           0.0f,           0.0f,
            0.0f,           0.0f,           1.0f,           0.0f,
            -camera->pos.x, -camera->pos.y, -camera->pos.z, 1.0f,
        };

    /* ---- rotation: yaw --------------------------------------------------- */
    projection->rotation =
        (m4f32){
            cyaw,   syaw, 0.0f, 0.0f,
            -syaw,  cyaw, 0.0f, 0.0f,
            0.0f,   0.0f, 1.0f, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
        };

    /* ---- rotation: pitch ------------------------------------------------- */
    projection->rotation = matrix_multiply(projection->rotation,
            (m4f32){
            cpch,   0.0f, spch, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            -spch,  0.0f, cpch, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
            });

    /* ---- orientation: z-up ----------------------------------------------- */
    projection->orientation =
        (m4f32){
            0.0f,   0.0f, -1.0f, 0.0f,
            -1.0f,  0.0f, 0.0f, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
        };

    /* ---- view ------------------------------------------------------------ */
    projection->view =
        matrix_multiply(projection->translation,
                matrix_multiply(projection->rotation, projection->orientation));

    /* ---- projection ------------------------------------------------------ */
    projection->projection =
        (m4f32){
            fov / ratio,    0.0f,   0.0f,   0.0f,
            0.0f,           fov,    0.0f,   0.0f,
            0.0f,           0.0f,   clip,  -1.0f,
            0.0f,           0.0f,   offset, 0.0f,
        };

    projection->perspective = matrix_multiply(projection->view, projection->projection);
}

