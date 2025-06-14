#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#define ENGINE_AUTHOR       "Author: Lily Awertnex"
#define ENGINE_NAME         "Heaven-Hell Continuum Engine"
#define ENGINE_VERSION      "0.1.0-beta"

#define GLEW_STATIC
#include "../../../include/glew_modified.h"
#define GLFW_INCLUDE_NONE
#include "../../../include/glfw3_modified.h"

#include "defines.h"
#include "platform.h"

/* ---- section: definitions ------------------------------------------------ */

typedef struct Render
{
    GLFWwindow *window;
    char title[128];
    v2i32 size;
    GLFWimage icon;
    v2f64 mouse_position;
    v2f64 mouse_last;
    v2f64 mouse_delta;
} Render;

typedef struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint vbo_len;
    GLuint ebo_len;
    GLfloat *vbo_data;
    GLfloat *ebo_data;
} Mesh;

typedef struct Shader
{
    str *file_name;
    GLuint id;                  /* used by opengl's "glCreateShader()" */
    GLuint type;                /* GL_<x>_SHADER */
    GLchar *source;             /* shader file source code */
    GLint loaded;               /* used by opengl's "glGetShaderiv()" */
} Shader;

typedef struct ShaderProgram
{
    str *name;                  /* for stress-free debugging */
    GLuint id;                  /* used by opengl's glCreateProgram() */
    GLint loaded;               /* used by opengl's "glGetProgramiv()" */
    Shader vertex;
    Shader fragment;
} ShaderProgram;

typedef struct Camera
{
    v3f32 pos;
    v3f32 rot;
    f32 sin_pitch;
    f32 cos_pitch;
    f32 sin_yaw;
    f32 cos_yaw;
    f32 fov;
    f32 ratio;
    f32 far;
    f32 near;
} Camera;

typedef struct Projection
{
    m4f32 target;
    m4f32 translation;
    m4f32 rotation;
    m4f32 orientation;
    m4f32 view;
    m4f32 projection;
    m4f32 perspective;
} Projection;

/* ---- section: signatures ------------------------------------------------- */

int init_glfw(void);
int init_window(Render *render);
int init_glew(void);
int init_freetype(void);

int init_shader(Shader *shader);
int init_shader_program(ShaderProgram *program);

int init_fbo(Render *render, GLuint *fbo, GLuint *color_buf, GLuint *rbo, Mesh *mesh_fbo);
int generate_mesh_fbo(Mesh *mesh);
int generate_mesh(Mesh *mesh, GLenum usage, GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);
void draw_mesh(Mesh *mesh);
void delete_mesh(Mesh *mesh);

void update_camera_movement(Camera *camera);
void update_camera_perspective(Camera *camera, Projection *projection);

#endif /* ENGINE_CORE_H */

