#ifndef MC_C_CORE_H
#define MC_C_CORE_H

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "defines.h"

/* ---- definitions --------------------------------------------------------- */
typedef struct Window
{
    GLFWwindow *window;
    GLFWimage icon;
    v2f64 cursor;
    v2f64 cursor_delta;
    v2i32 size;
    char title[128];
} Window;

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
    GLchar source[1024 * 16];   /* shader file source code */
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
    m4f32 translation;
    m4f32 rotation;
    m4f32 orientation;
    m4f32 view;
    m4f32 projection;
    m4f32 perspective;
} Projection;

/* ---- signatures ---------------------------------------------------------- */
int init_glfw(void);
int init_window(Window *render);
int init_glew(void);

int load_shader_from_disk(Shader *shader);
int init_shader(Shader *shader);
int init_shader_program(ShaderProgram *program);

int generate_mesh(Mesh *mesh, GLenum usage, GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);
void draw_mesh(Mesh *mesh);
void delete_mesh(Mesh *mesh);

void update_camera_movement(v2f64 cursor_delta, Camera *camera);
void update_camera_perspective(Camera *camera, Projection *projection);

#endif /* MC_C_CORE_H */

