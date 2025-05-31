#ifndef MC_C_RENDERING_H

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "defines.h"

// ---- definitions ------------------------------------------------------------
typedef struct Window
{
    GLFWwindow *window;
    GLFWimage icon;
    v2f64 cursor;
    v2i32 size;
    char title[20];
} Window;

typedef struct Vertex
{
    v3f32 pos;
    v2f32 uv;
} Vertex;

typedef struct Shader
{
    str *file_name;
    GLuint shader;
    GLuint type;
    GLchar source[1024 * 16];
    GLint loaded;
} Shader;

typedef struct Camera
{
    v3f32 pos;
    v3f32 target;
    f32 fov;
} Camera;

// ---- declarations -----------------------------------------------------------
extern Shader vertex_shader, fragment_shader;
extern GLuint vao, vbo, ebo, shader_program;

// ---- signatures -------------------------------------------------------------
int init_glfw();
int init_window(Window *render);
int init_glew(Window *render);

void update_rendering();
void draw_rendering();
void free_rendering();

void bind_buffers();
int load_shader_from_disk(Shader *shader);
int init_shader(Shader *shader);
int init_shader_program();
int init_shaders();

#define MC_C_RENDERING_H
#endif

