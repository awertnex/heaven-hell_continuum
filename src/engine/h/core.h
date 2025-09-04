#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#define ENGINE_AUTHOR       "Author: Lily Awertnex"
#define ENGINE_NAME         "Heaven-Hell Continuum Engine"
#define ENGINE_VERSION      "0.1.0-beta"

#include "../../../include/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "../../../include/glfw3_modified.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../../include/stb_truetype_modified.h"

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
    f32 fovy;
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

typedef struct Glyph
{
    v2f32 size;
    v2i32 bearing;
    i32 advance;
    i32 x0, y0, x1, y1;         /* uv texture coordinates */
    b8 loaded;
} Glyph;

typedef struct Font
{
    str path[PATH_MAX];         /* font file name, assigned in load_font() automatically */
    u32 size;
    i32 baseline;
    i32 ascent, descent;        /* glyphs highest and lowest points' deviation from baseline in pixels */
    i32 line_gap;               /* gap between lines, from descent to next line's ascent */
    f32 line_height;            /* font line height in pixels (or advance y) */

    stbtt_fontinfo info;        /* used by stb_truetype.h's stbtt_InitFont() */
    u8 *buf;                    /* font file contents, used by stb_truetype.h's stbtt_InitFont() */
    u64 buf_len;                /* buf size in bytes */
    u8 *bitmap;                 /* memory block for all font glyph bitmaps */

    GLuint id;                  /* used by opengl's glGenTextures() */
    Glyph glyph[GLYPH_MAX];
} Font;

/* ---- section: signatures ------------------------------------------------- */

/*
 * return non-zero on failure;
 */
int init_glfw(void);

/*
 * return non-zero on failure;
 */
int init_window(Render *render);

/*
 * return non-zero on failure;
 */
int init_glad(void);

int init_shader(const str *shaders_dir, Shader *shader);

int init_shader_program(const str *shaders_dir, ShaderProgram *program);

int init_fbo(Render *render, GLuint *fbo, GLuint *color_buf, GLuint *rbo, Mesh *mesh_fbo);

/*
 * return FALSE (0) on failure;
 */
b8 generate_texture_atlas(GLuint *id, const GLint format, u32 size, void *buffer);

int generate_mesh_fbo(Mesh *mesh);

/* 
 * usage = GL_<x>_DRAW;
 */
int generate_mesh(Mesh *mesh, GLenum usage, GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);

void draw_mesh(Mesh *mesh);

void free_mesh(Mesh *mesh);

void update_camera_movement(Camera *camera);

void update_camera_perspective(Camera *camera, Projection *projection);

/*
 * load font from file at font_path;
 * allocate memory for font.buf and load file contents into it in binary format;
 * allocate memory for font.bitmap and render glyphs into it;
 *
 * size = font size;
 * font_path = font path;
 *
 * return FALSE (0) on failure;
 */
b8 load_font(Font *font, u32 size, const str *font_path);

void free_font(Font *font);

#endif /* ENGINE_CORE_H */

