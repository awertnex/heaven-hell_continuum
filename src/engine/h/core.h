#ifndef MC_C_CORE_H
#define MC_C_CORE_H

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "defines.h"

/* ---- definitions --------------------------------------------------------- */
typedef struct Window
{
    GLFWwindow *window;
    GLFWimage icon;
    v2f64 cursor;
    v2f64 cursor_delta;
    v2i32 size;
    char title[20];
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
    GLuint shader;
    GLuint type;
    GLchar source[1024 * 16];
    GLint loaded;
} Shader;

typedef struct Camera
{
    v3f32 pos;
    v3f32 rot;
    f32 fov;
    f32 far;
    f32 near;
} Camera;

/* ---- declarations -------------------------------------------------------- */
extern Shader vertex_shader, fragment_shader;
extern GLuint shader_program;

/* ---- signatures ---------------------------------------------------------- */
int init_glfw();
int init_window(Window *render);
int init_glew(Window *render);

void update_rendering();
void draw_rendering();
void free_rendering();

void bind_mesh(Mesh *mesh, GLenum usage);
void delete_mesh(Mesh *mesh);
int load_shader_from_disk(Shader *shader);
int init_shader(Shader *shader);
int init_shader_program();
int init_shaders();

#endif /* MC_C_CORE_H */

