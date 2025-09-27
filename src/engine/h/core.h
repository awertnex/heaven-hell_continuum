#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#define ENGINE_AUTHOR       "Lily Awertnex"
#define ENGINE_NAME         "Fossil Engine"
#define ENGINE_VERSION      "0.1.0"

#include "../../../include/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "../../../include/glfw3_modified.h"
#include "../../../include/stb_truetype_modified.h"

#include "defines.h"
#include "platform.h"
#include "limits.h"

/* ---- section: definitions ------------------------------------------------ */

#define KEYBOARD_KEYS_MAX 120
#define KEYBOARD_DOUBLE_PRESS_TIME_THRESHOLD 0.4f

#define FONT_ATLAS_CELL_RESOLUTION 16
#define FONT_RESOLUTION_DEFAULT 64
#define FONT_SIZE_DEFAULT 22.0f

enum KeyboardKeyState
{
    KEY_IDLE,
    KEY_PRESS,
    KEY_HOLD,
    KEY_RELEASE,
    KEY_LISTEN_DOUBLE,
    KEY_PRESS_DOUBLE,
    KEY_HOLD_DOUBLE,
    KEY_RELEASE_DOUBLE,
}; /* KeyboardKeyState */

enum KeyboardKeys
{
    KEY_SPACE,
    KEY_APOSTROPHE,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,
    KEY_EQUAL,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFT_BRACKET,
    KEY_BACKSLASH,
    KEY_RIGHT_BRACKET,
    KEY_GRAVE_ACCENT,
    KEY_WORLD_1,
    KEY_WORLD_2,

    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,
    KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_ALT,
    KEY_RIGHT_SUPER,
    KEY_MENU,
}; /* KeyboardKeys */

typedef struct Render
{
    GLFWwindow *window;
    char title[128];
    v2i32 size;
    GLFWimage icon;
    v2f64 mouse_position;
    v2f64 mouse_last;
    v2f64 mouse_delta;
    f64 frame_start;
    f64 frame_last;
    f64 frame_delta;
    f64 frame_delta_square;
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
    Shader geometry;
    Shader fragment;
} ShaderProgram;

typedef struct FBO
{
    GLuint fbo;
    GLuint color_buf;
    GLuint rbo;
} FBO;

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
    v2i32 scale;
    v2i32 bearing;
    i32 advance;
    i32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
} Glyph;

typedef struct Glyphf
{
    v2f32 scale;
    v2f32 bearing;
    f32 advance;
    f32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
} Glyphf;

typedef struct Font
{
    str path[PATH_MAX];         /* font file name, assigned in load_font() automatically */
    u32 resolution;             /* glyph bitmap diameter in bytes */
    f32 char_size;              /* for font atlas sampling */
    i32 ascent;                 /* glyphs highest points' deviation from baseline */
    i32 descent;                /* glyphs lowest points' deviation from baseline */
    i32 line_gap;
    i32 line_height;
    v2i32 scale;                /* biggest glyph bounding box in pixels */

    stbtt_fontinfo info;        /* used by stb_truetype.h's stbtt_InitFont() */
    u8 *buf;                    /* font file contents, used by stb_truetype.h's stbtt_InitFont() */
    u64 buf_len;                /* buf size in bytes */
    u8 *bitmap;                 /* memory block for all font glyph bitmaps */

    GLuint id;                  /* used by opengl's glGenTextures() */
    Glyph glyph[GLYPH_MAX];

    struct /* uniform */
    {
        GLint char_size;
        GLint font_size;
        GLint screen_size;
        GLint advance;
        GLint bearing;
        GLint text_color;
    } uniform;

} Font;

/* ---- section: declarations ----------------------------------------------- */

extern u32 keyboard_key[KEYBOARD_KEYS_MAX];
extern u32 keyboard_tab[KEYBOARD_KEYS_MAX];

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

int init_fbo(Render *render, FBO *fbo, Mesh *mesh_fbo, b8 flip_vertical);

int realloc_fbo(Render *render, FBO *fbo);

void free_fbo(GLuint *fbo, GLuint *color_buf, GLuint *rbo);

/*
 * return FALSE (0) on failure;
 */
b8 generate_texture(GLuint *id, const GLint format, u32 width, u32 height, void *buffer);

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
 * update internal key states: press, double-press, hold, release;
 */
void update_key_states(Render *render);

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_press(const u32 key)
{return (keyboard_key[key] == KEY_PRESS);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_listen_double(const u32 key)
{return (keyboard_key[key] == KEY_LISTEN_DOUBLE);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_hold(const u32 key)
{return (keyboard_key[key] == KEY_HOLD);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_hold_double(const u32 key)
{return (keyboard_key[key] == KEY_HOLD_DOUBLE);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_release(const u32 key)
{return (keyboard_key[key] == KEY_RELEASE);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_release_double(const u32 key)
{return (keyboard_key[key] == KEY_RELEASE_DOUBLE);}

static inline b8 is_key_press(const u32 key)
{return (keyboard_key[key] == KEY_PRESS || keyboard_key[key] == KEY_PRESS_DOUBLE);}

static inline b8 is_key_press_double(const u32 key)
{return (keyboard_key[key] == KEY_PRESS_DOUBLE);}

static inline b8 is_key_hold(const u32 key)
{return (keyboard_key[key] == KEY_HOLD || keyboard_key[key] == KEY_HOLD_DOUBLE);}

static inline b8 is_key_release(const u32 key)
{return (keyboard_key[key] == KEY_RELEASE || keyboard_key[key] == KEY_RELEASE_DOUBLE);}

/*
 * load font from file at font_path;
 * allocate memory for font.buf and load file contents into it in binary format;
 * allocate memory for font.bitmap and render glyphs onto it;
 * generate square texture of diameter "size * 16" and bake bitmap onto it;
 *
 * size = font size & character bitmap diameter;
 * font_path = font path;
 *
 * return FALSE (0) on failure;
 */
b8 init_font(Font *font, u32 size, const str *font_path);

void free_font(Font *font);

u8 init_text(void);

void start_text(u64 length, Render *render);

void push_text(const str *text, f32 size, v2f32 pos, i8 align_x, i8 align_y, Font *font);

void render_text(f32 size, v3f32 pos, u32 color, i8 align_x, i8 align_y, Font *font, ShaderProgram *program, FBO *fbo);

/*
 * does update Font.projection;
 *
 * size = font height in pixels;
 * color = hex format: 0xrrggbbaa;
 */
void draw_text(Render *render, Font *font, const str *text, f32 size, v3f32 pos, u32 color, i8 align_x, i8 align_y);

#endif /* ENGINE_CORE_H */

