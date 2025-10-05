#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "h/core.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../include/stb_truetype_modified.h"

#include "h/dir.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/memory.h"

/* ---- section: declarations ----------------------------------------------- */

u32 keyboard_key[KEYBOARD_KEYS_MAX];
u32 keyboard_tab[KEYBOARD_KEYS_MAX] =
{
    GLFW_KEY_SPACE,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_COMMA,
    GLFW_KEY_MINUS,
    GLFW_KEY_PERIOD,
    GLFW_KEY_SLASH,
    GLFW_KEY_0,
    GLFW_KEY_1,
    GLFW_KEY_2,
    GLFW_KEY_3,
    GLFW_KEY_4,
    GLFW_KEY_5,
    GLFW_KEY_6,
    GLFW_KEY_7,
    GLFW_KEY_8,
    GLFW_KEY_9,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_EQUAL,
    GLFW_KEY_A,
    GLFW_KEY_B,
    GLFW_KEY_C,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_G,
    GLFW_KEY_H,
    GLFW_KEY_I,
    GLFW_KEY_J,
    GLFW_KEY_K,
    GLFW_KEY_L,
    GLFW_KEY_M,
    GLFW_KEY_N,
    GLFW_KEY_O,
    GLFW_KEY_P,
    GLFW_KEY_Q,
    GLFW_KEY_R,
    GLFW_KEY_S,
    GLFW_KEY_T,
    GLFW_KEY_U,
    GLFW_KEY_V,
    GLFW_KEY_W,
    GLFW_KEY_X,
    GLFW_KEY_Y,
    GLFW_KEY_Z,
    GLFW_KEY_LEFT_BRACKET,
    GLFW_KEY_BACKSLASH,
    GLFW_KEY_RIGHT_BRACKET,
    GLFW_KEY_GRAVE_ACCENT,
    GLFW_KEY_WORLD_1,
    GLFW_KEY_WORLD_2,

    GLFW_KEY_ESCAPE,
    GLFW_KEY_ENTER,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_INSERT,
    GLFW_KEY_DELETE,
    GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT,
    GLFW_KEY_DOWN,
    GLFW_KEY_UP,
    GLFW_KEY_PAGE_UP,
    GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_HOME,
    GLFW_KEY_END,
    GLFW_KEY_CAPS_LOCK,
    GLFW_KEY_SCROLL_LOCK,
    GLFW_KEY_NUM_LOCK,
    GLFW_KEY_PRINT_SCREEN,
    GLFW_KEY_PAUSE,
    GLFW_KEY_F1,
    GLFW_KEY_F2,
    GLFW_KEY_F3,
    GLFW_KEY_F4,
    GLFW_KEY_F5,
    GLFW_KEY_F6,
    GLFW_KEY_F7,
    GLFW_KEY_F8,
    GLFW_KEY_F9,
    GLFW_KEY_F10,
    GLFW_KEY_F11,
    GLFW_KEY_F12,
    GLFW_KEY_F13,
    GLFW_KEY_F14,
    GLFW_KEY_F15,
    GLFW_KEY_F16,
    GLFW_KEY_F17,
    GLFW_KEY_F18,
    GLFW_KEY_F19,
    GLFW_KEY_F20,
    GLFW_KEY_F21,
    GLFW_KEY_F22,
    GLFW_KEY_F23,
    GLFW_KEY_F24,
    GLFW_KEY_F25,
    GLFW_KEY_KP_0,
    GLFW_KEY_KP_1,
    GLFW_KEY_KP_2,
    GLFW_KEY_KP_3,
    GLFW_KEY_KP_4,
    GLFW_KEY_KP_5,
    GLFW_KEY_KP_6,
    GLFW_KEY_KP_7,
    GLFW_KEY_KP_8,
    GLFW_KEY_KP_9,
    GLFW_KEY_KP_DECIMAL,
    GLFW_KEY_KP_DIVIDE,
    GLFW_KEY_KP_MULTIPLY,
    GLFW_KEY_KP_SUBTRACT,
    GLFW_KEY_KP_ADD,
    GLFW_KEY_KP_ENTER,
    GLFW_KEY_KP_EQUAL,
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_LEFT_SUPER,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_RIGHT_ALT,
    GLFW_KEY_RIGHT_SUPER,
    GLFW_KEY_MENU,
}; /* keyboard_tab */

/* ---- section: declarations ----------------------------------------------- */

static Mesh mesh_text = {0};
static struct TextInfo
{
    /* 
     * current cursor position between start_text()
     * and render_text() for internal buffer
     */
    Font *font;
    Glyphf glyph[GLYPH_MAX];
    f32 font_size;
    f32 line;
    v2f32 screen_size;
    GLuint cursor;
} text_info;

/* ---- section: windowing -------------------------------------------------- */

int
init_glfw(b8 multisample)
{
    if (!glfwInit())
    {
        LOGFATAL("%s\n", "Failed to Initialize GLFW, Process Aborted");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (multisample)
        glfwWindowHint(GLFW_SAMPLES, 4);
    return 0;
}

int
init_window(Render *render)
{
    render->window = glfwCreateWindow(render->size.x, render->size.y,
            render->title, NULL, NULL);

    if (!render->window)
    {
        LOGFATAL("%s\n", "Failed to Initialize Window or OpenGL Context,"
                "Process Aborted");
        return -1;
    }

    glfwMakeContextCurrent(render->window);
    return 0;
}

int
init_glad(void)
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOGFATAL("%s\n", "Failed to Initialize GLAD, Process Aborted");
        return -1;
    }

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3))
    {
        LOGFATAL("OpenGL 4.3+ Required, Current Version '%d.%d',"
                "Process Aborted\n", GLVersion.major, GLVersion.minor);
        return -1;
    }

    LOGDEBUG("OpenGL:    %s\n", glGetString(GL_VERSION));
    LOGDEBUG("GLSL:      %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOGDEBUG("Vendor:    %s\n", glGetString(GL_VENDOR));
    LOGDEBUG("Renderer:  %s\n", glGetString(GL_RENDERER));

    return 0;
}

/* ---- section: shaders ---------------------------------------------------- */

int
init_shader(const str *shaders_dir, Shader *shader)
{
    if (!shader->type)
        return 0;

    str str_reg[PATH_MAX] = {0};
    snprintf(str_reg, PATH_MAX, "%s%s", shaders_dir, shader->file_name);

    if ((shader->source = get_file_contents(str_reg, NULL, "r")) == NULL)
        return -1;
    (shader->id) ? glDeleteShader(shader->id) : 0;

    shader->id = glCreateShader(shader->type);
    glShaderSource(shader->id, 1, (const GLchar**)&shader->source, NULL);
    glCompileShader(shader->id);

    mem_free((void*)&shader->source, strlen(shader->source), "shader.source");

    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        char log[2048];
        glGetShaderInfoLog(shader->id, 2048, NULL, log);
        LOGERROR("Shader '%s':\n%s\n", shader->file_name, log);
        return -1;
    }
    else LOGINFO("Shader %d '%s' Loaded\n", shader->id, shader->file_name);

    return 0;
}

int
init_shader_program(const str *shaders_dir, ShaderProgram *program)
{
    if (init_shader(shaders_dir, &program->vertex) != 0)
        return -1;
    if (init_shader(shaders_dir, &program->geometry) != 0)
        return -1;
    if (init_shader(shaders_dir, &program->fragment) != 0)
        return -1;
    (program->id) ? glDeleteProgram(program->id) : 0;

    program->id = glCreateProgram();
    if (program->vertex.id)
        glAttachShader(program->id, program->vertex.id);
    if (program->geometry.id)
        glAttachShader(program->id, program->geometry.id);
    if (program->fragment.id)
        glAttachShader(program->id, program->fragment.id);
    glLinkProgram(program->id);

    glGetProgramiv(program->id, GL_LINK_STATUS, &program->loaded);
    if (!program->loaded)
    {
        char log[2048];
        glGetProgramInfoLog(program->id, 2048, NULL, log);
        LOGERROR("Shader Program '%s':\n%s\n", program->name, log);
        return -1;
    }
    else LOGINFO("Shader Program %d '%s' Loaded\n",
            program->id, program->name);

    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->geometry.loaded)
        glDeleteShader(program->geometry.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

    return 0;
}

/* ---- section: meat ------------------------------------------------------- */

int
init_fbo(Render *render, FBO *fbo, Mesh *mesh_fbo,
        b8 multisample, u32 samples, b8 flip_vertical)
{
    free_fbo(fbo);

    glGenFramebuffers(1, &fbo->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    if (multisample)
    {
        /* ---- color buffer ------------------------------------------------ */
        glGenTextures(1, &fbo->color_buf);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA,
                render->size.x, render->size.y, GL_TRUE);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */
        glGenRenderbuffers(1, &fbo->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT24, render->size.x, render->size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER, fbo->rbo);
    }
    else
    {
        /* ---- color buffer ------------------------------------------------ */
        glGenTextures(1, &fbo->color_buf);
        glBindTexture(GL_TEXTURE_2D, fbo->color_buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                render->size.x, render->size.y, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */
        glGenRenderbuffers(1, &fbo->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                render->size.x, render->size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER, fbo->rbo);
    }

    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGFATAL("FBO '%d': Status '%d' Not Complete, Process Aborted\n",
                fbo->fbo, status);
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* ---- mesh data ------------------------------------------------------- */
    if (mesh_fbo == NULL || mesh_fbo->vbo_data != NULL) return 0;

    mesh_fbo->vbo_len = 16;
    GLfloat vbo_data[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
    };

    if (flip_vertical)
    {
        vbo_data[3] = 1.0f;
        vbo_data[7] = 0.0f;
        vbo_data[11] = 0.0f;
        vbo_data[15] = 1.0f;
    }

    if (!mem_alloc((void*)&mesh_fbo->vbo_data,
                sizeof(GLfloat) * mesh_fbo->vbo_len, "mesh_fbo.vbo_data"))
        goto cleanup;

    memcpy(mesh_fbo->vbo_data, vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len);

    /* ---- bind mesh ------------------------------------------------------- */
    glGenVertexArrays(1, &mesh_fbo->vao);
    glGenBuffers(1, &mesh_fbo->vbo);

    glBindVertexArray(mesh_fbo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_fbo->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_fbo->vbo_len * sizeof(GLfloat),
            mesh_fbo->vbo_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;

cleanup:
    free_fbo(fbo);
    free_mesh(mesh_fbo);
    return -1;
}

int
realloc_fbo(Render *render, FBO *fbo, b8 multisample, u32 samples)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    if (multisample)
    {
        /* ---- color buffer ------------------------------------------------ */
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA,
                render->size.x, render->size.y, GL_TRUE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT24, render->size.x, render->size.y);
    }
    else
    {
        /* ---- color buffer ------------------------------------------------ */
        glBindTexture(GL_TEXTURE_2D, fbo->color_buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                render->size.x, render->size.y, 0, GL_RGBA, GL_FLOAT, NULL);

        /* ---- render buffer ----------------------------------------------- */
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                render->size.x, render->size.y);
    }

    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGFATAL("FBO '%d': Status '%d' Not Complete, Process Aborted\n",
                &fbo->fbo, status);

        free_fbo(fbo);
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

void
free_fbo(FBO *fbo)
{
    fbo->rbo ? glDeleteFramebuffers(1, &fbo->rbo) : 0;
    fbo->color_buf ? glDeleteTextures(1, &fbo->color_buf) : 0;
    fbo->fbo ? glDeleteFramebuffers(1, &fbo->fbo) : 0;
};

b8
generate_texture(GLuint *id, const GLint format,
        u32 width, u32 height, void *buffer)
{
    if (width <= 2 || height <= 2)
    {
        LOGERROR("Texture Generation '%d' Failed, Size Too Small\n", *id);
        return FALSE;
    }

    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format,
            width, height, 0, format, GL_UNSIGNED_BYTE, buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    return TRUE;
}

int
generate_mesh(Mesh *mesh, GLenum usage,
        GLuint vbo_len, GLuint ebo_len,
        GLfloat *vbo_data, GLuint *ebo_data)
{
    if (!mem_alloc((void*)&mesh->vbo_data,
                sizeof(GLfloat) * vbo_len, "mesh.vbo_data"))
        goto cleanup;

    if (!mem_alloc((void*)&mesh->ebo_data,
                sizeof(GLuint) * ebo_len, "mesh.vbo_data"))
        goto cleanup;

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

    glBufferData(GL_ARRAY_BUFFER,
            mesh->vbo_len * sizeof(GLfloat), mesh->vbo_data, usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            mesh->ebo_len * sizeof(GLuint), mesh->ebo_data, usage);

    glVertexAttribPointer(0, 3, GL_FLOAT,
            GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;

cleanup:
    free_mesh(mesh);
    return -1;
}

void
draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->ebo_len, GL_UNSIGNED_INT, 0);
}

void
free_mesh(Mesh *mesh)
{
    mesh->ebo ? glDeleteBuffers(1, &mesh->ebo) : 0;
    mesh->vbo ? glDeleteBuffers(1, &mesh->vbo) : 0;
    mesh->vao ? glDeleteVertexArrays(1, &mesh->vao) : 0;

    mem_free((void*)&mesh->vbo_data,
            sizeof(GLfloat) * mesh->vbo_len, "mesh.vbo_data");

    mem_free((void*)&mesh->ebo_data,
            sizeof(GLuint) * mesh->ebo_len, "mesh.vbo_data");

    *mesh = (Mesh){0};
}

/* ---- section: camera ----------------------------------------------------- */

void
update_camera_movement(Camera *camera)
{
    const f32 ANGLE = 90.0f;
    const f32 RANGE = 360.0f;

    camera->rot.z = fmodf(camera->rot.z, RANGE);
    if (camera->rot.z < 0.0f) camera->rot.z += RANGE;
    camera->rot.y = clamp_f32(camera->rot.y, -ANGLE, ANGLE);

    camera->sin_pitch = sinf((camera->rot.y) * DEG2RAD);
    camera->cos_pitch = cosf((camera->rot.y) * DEG2RAD);
    camera->sin_yaw =   sinf((camera->rot.z) * DEG2RAD);
    camera->cos_yaw =   cosf((camera->rot.z) * DEG2RAD);
}

void
update_camera_perspective(Camera *camera, Projection *projection)
{
    const f32 SPCH = camera->sin_pitch;
    const f32 CPCH = camera->cos_pitch;
    const f32 SYAW = camera->sin_yaw;
    const f32 CYAW = camera->cos_yaw;

    f32 ratio = camera->ratio;
    f32 fovy = 1.0f / tanf((camera->fovy / 2.0f) * DEG2RAD);
    f32 far = camera->far;
    f32 near = camera->near;
    f32 clip = -(far + near) / (far - near);
    f32 offset = -(2.0f * far * near) / (far - near);

    /* ---- target ---------------------------------------------------------- */
    projection->target =
        (m4f32){
            1.0f,           0.0f,           0.0f,   0.0f,
            0.0f,           1.0f,           0.0f,   0.0f,
            0.0f,           0.0f,           1.0f,   0.0f,
            -CYAW * -CPCH,  SYAW * -CPCH,   -SPCH,  1.0f,
        };

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
            CYAW,   SYAW, 0.0f, 0.0f,
            -SYAW,  CYAW, 0.0f, 0.0f,
            0.0f,   0.0f, 1.0f, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
        };

    /* ---- rotation: pitch ------------------------------------------------- */
    projection->rotation = matrix_multiply(projection->rotation,
            (m4f32){
            CPCH,   0.0f, SPCH, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            -SPCH,  0.0f, CPCH, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
            });

    /* ---- orientation: z-up ----------------------------------------------- */
    projection->orientation =
        (m4f32){
            0.0f,   0.0f, -1.0f,    0.0f,
            -1.0f,  0.0f, 0.0f,     0.0f,
            0.0f,   1.0f, 0.0f,     0.0f,
            0.0f,   0.0f, 0.0f,     1.0f,
        };

    /* ---- view ------------------------------------------------------------ */
    projection->view =
        matrix_multiply(projection->translation,
                matrix_multiply(
                    projection->rotation, projection->orientation));

    /* ---- projection ------------------------------------------------------ */
    projection->projection =
        (m4f32){
            fovy / ratio,   0.0f,   0.0f,   0.0f,
            0.0f,           fovy,   0.0f,   0.0f,
            0.0f,           0.0f,   clip,  -1.0f,
            0.0f,           0.0f,   offset, 0.0f,
        };

    projection->perspective =
        matrix_multiply(projection->view, projection->projection);
}

/* ---- section: input ------------------------------------------------------ */

void
update_mouse_movement(Render *render)
{
    glfwGetCursorPos(render->window,
            &render->mouse_position.x, &render->mouse_position.y);
    render->mouse_delta =
        (v2f64){
            render->mouse_position.x - render->mouse_last.x,
            render->mouse_position.y - render->mouse_last.y,
        };
    render->mouse_last = render->mouse_position;
}

static inline b8 _is_key_press(const u32 key)
{
    return (keyboard_key[key] == KEY_PRESS);
}

static inline b8 _is_key_listen_double(const u32 key)
{
    return (keyboard_key[key] == KEY_LISTEN_DOUBLE);
}

static inline b8 _is_key_hold(const u32 key)
{
    return (keyboard_key[key] == KEY_HOLD);
}

static inline b8 _is_key_hold_double(const u32 key)
{
    return (keyboard_key[key] == KEY_HOLD_DOUBLE);
}

static inline b8 _is_key_release(const u32 key)
{
    return (keyboard_key[key] == KEY_RELEASE);
}

static inline b8 _is_key_release_double(const u32 key)
{
    return (keyboard_key[key] == KEY_RELEASE_DOUBLE);
}

void
update_key_states(Render *render)
{
    static f64 key_press_start_time[KEYBOARD_KEYS_MAX];
    for (u32 i = 0; i < KEYBOARD_KEYS_MAX; ++i)
    {
        b8 key_press =
            (glfwGetKey(render->window, keyboard_tab[i]) == GLFW_PRESS);
        b8 key_release =
            (glfwGetKey(render->window, keyboard_tab[i]) == GLFW_RELEASE);

        if (key_press && !keyboard_key[i])
        {
            keyboard_key[i] = KEY_PRESS;
            key_press_start_time[i] = render->frame_start;
            continue;
        }
        else if (key_press && _is_key_listen_double(i))
        {
            if ((glfwGetTime() - key_press_start_time[i])
                    < KEYBOARD_DOUBLE_PRESS_TIME)
                keyboard_key[i] = KEY_PRESS_DOUBLE;
            else
                keyboard_key[i] = KEY_PRESS;
            key_press_start_time[i] = render->frame_start;
            continue;
        }

        if (key_release)
        {
            if (_is_key_hold(i))
                keyboard_key[i] = KEY_RELEASE;
            else if (_is_key_hold_double(i))
                keyboard_key[i] = KEY_RELEASE_DOUBLE;
            continue;
        }

        if (_is_key_press(i))           keyboard_key[i] = KEY_HOLD;
        if (is_key_press_double(i))     keyboard_key[i] = KEY_HOLD_DOUBLE;
        if (_is_key_release(i))         keyboard_key[i] = KEY_LISTEN_DOUBLE;
        if (_is_key_release_double(i))  keyboard_key[i] = KEY_IDLE;
    }
}

/* ---- section: font ------------------------------------------------------- */

b8
init_font(Font *font, u32 resolution, const str *font_path)
{
    if (resolution <= 2)
    {
        LOGERROR("Font Initialization '%s' Failed, Font Size Too Small\n",
                font_path);
        return FALSE;
    }

    if (strlen(font_path) >= PATH_MAX)
    {
        LOGERROR("Font Initialization '%s' Failed, Font Path Too Long\n",
                font_path);
        return FALSE;
    }

    if (!is_file_exists(font_path))
        return FALSE;

    font->buf = (u8*)get_file_contents(font_path, &font->buf_len, "rb");
    if (font->buf == NULL)
        return FALSE;

    if (!stbtt_InitFont(&font->info, font->buf, 0))
    {
        LOGERROR("Font Initializing '%s' Failed, 'stbtt_InitFont' Failed\n",
                font_path);
        goto cleanup;
    }

    if (!mem_alloc((void*)&font->bitmap,
                GLYPH_MAX * resolution * resolution, font_path))
        goto cleanup;

    u8 *canvas = {NULL};
    if (!mem_alloc((void*)&canvas,
                resolution * resolution, "font_glyph_canvas"))
        goto cleanup;

    snprintf(font->path, PATH_MAX, "%s", font_path);

    stbtt_GetFontVMetrics(&font->info,
            &font->ascent, &font->descent, &font->line_gap);

    font->resolution = resolution;
    font->char_size = 1.0f / FONT_ATLAS_CELL_RESOLUTION;
    font->line_height = font->ascent - font->descent + font->line_gap;
    font->size = resolution;

    f32 scale = stbtt_ScaleForPixelHeight(&font->info, resolution);
    for (i32 i = 0; i < GLYPH_MAX; ++i)
    {
        int glyph_index = stbtt_FindGlyphIndex(&font->info, i);
        if (!glyph_index)
            continue;

        Glyph *g = &font->glyph[i];

        stbtt_GetGlyphHMetrics(&font->info, glyph_index,
                &g->advance, &g->bearing.x);

        stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph_index,
                1.0f, 1.0f, 0.0f, 0.0f, &g->x0, &g->y0, &g->x1, &g->y1);

        g->bearing.y = g->y0;
        g->scale.x = g->x1 - g->x0;
        g->scale.y = g->y1 - g->y0;
        (g->scale.x > font->scale.x) ? font->scale.x = g->scale.x : 0;
        (g->scale.y > font->scale.y) ? font->scale.y = g->scale.y : 0;

        u8 col = i % FONT_ATLAS_CELL_RESOLUTION;
        u8 row = i / FONT_ATLAS_CELL_RESOLUTION;
        if (!stbtt_IsGlyphEmpty(&font->info, glyph_index))
        {
            stbtt_MakeGlyphBitmapSubpixel(&font->info, canvas,
                    resolution, resolution, resolution,
                    scale, scale, 0.0f, 0.0f, glyph_index);

            void *bitmap_offset =
                font->bitmap +
                (col * resolution) +
                (row * resolution * resolution * FONT_ATLAS_CELL_RESOLUTION);

            for (u32 y = 0; y < resolution; ++y)
            {
                for (u32 x = 0; x < resolution; ++x)
                    memcpy((bitmap_offset + x +
                                (y * resolution * FONT_ATLAS_CELL_RESOLUTION)),
                            (canvas + x + (y * resolution)), 1);
            }
            mem_zero((void*)&canvas,
                    resolution * resolution, "font_glyph_canvas");
        }
        g->texture_sample.x = col * font->char_size;
        g->texture_sample.y = row * font->char_size;
        font->glyph[i].loaded = TRUE;
    }

    if (!generate_texture(&font->id, GL_RED,
                FONT_ATLAS_CELL_RESOLUTION * resolution,
                FONT_ATLAS_CELL_RESOLUTION * resolution, font->bitmap))
        goto cleanup;

    mem_free((void*)&canvas, resolution * resolution, "font_glyph_canvas");
    return TRUE;

cleanup:
    mem_free((void*)&canvas, resolution * resolution, "font_glyph_canvas");
    free_font(font);
    return FALSE;
}

void
free_font(Font *font)
{
    mem_free((void*)&font->buf, font->buf_len, "file_contents");
    mem_free((void*)&font->bitmap,
            GLYPH_MAX * font->resolution * font->resolution, font->path);

    *font = (Font){0};
}

/* ---- section: text ------------------------------------------------------- */

u8
init_text(void)
{
    if (!mem_alloc((void*)&mesh_text.vbo_data,
                STRING_MAX * sizeof(GLfloat) * 4, "mesh_text.vbo_data"))
        goto cleanup;

    mesh_text.vbo_len = STRING_MAX * 4;
    mesh_text.ebo_len = STRING_MAX;

    if (!mesh_text.vao)
    {
        glGenVertexArrays(1, &mesh_text.vao);
        glGenBuffers(1, &mesh_text.vbo);

        glBindVertexArray(mesh_text.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh_text.vbo);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    return 0;

cleanup:
    free_mesh(&mesh_text);
    LOGFATAL("%s\n", "Failed to Initialize Text, Process Aborted");
    return -1;
}

void
start_text(
        u64 length, f32 size, Font *font,
        Render *render, ShaderProgram *program, FBO *fbo, b8 clear)
{
    if (!length)
        length = STRING_MAX;
    else if (length > mesh_text.ebo_len)
    {
        if (!mem_realloc((void*)&mesh_text.vbo_data,
                    length * sizeof(GLfloat) * 4,
                    "mesh_text.vbo_data"))
            goto cleanup;
        mesh_text.vbo_len = length * 4;
        mesh_text.ebo_len = length;
    }

    text_info.font = font;
    text_info.font_size = size;
    text_info.line = 0.0f;
    text_info.screen_size.x = 2.0f / render->size.x;
    text_info.screen_size.y = 2.0f / render->size.y;
    text_info.cursor = 0;

    glUseProgram(program->id);
    glBindTexture(GL_TEXTURE_2D, text_info.font->id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
    return;

cleanup:
    free_mesh(&mesh_text);
    LOGERROR("%s\n", "Failed to Start Text");
}

void
push_text(const str *text, v2f32 pos, i8 align_x, i8 align_y)
{
    if (!mesh_text.vbo_data)
    {
        LOGERROR("%s\n", "Failed to Push Text, 'mesh_text.vbo_data' Null");
        return;
    }

    u64 len = strlen(text);
    if (len >= STRING_MAX)
    {
        LOGERROR("%s\n", "Failed to Push Text, Text Too Long");
        return;
    }

    if (text_info.cursor + len >= mesh_text.ebo_len)
    {
        if (!mem_realloc((void*)&mesh_text.vbo_data,
                    (mesh_text.vbo_len + STRING_MAX) *
                    sizeof(GLfloat) * 4, "mesh_text.vbo_data"))
        {
            free_mesh(&mesh_text);
            LOGERROR("%s\n", "Failed to Push Text");
        }
        mesh_text.vbo_len += STRING_MAX * 4;
        mesh_text.ebo_len += STRING_MAX;
    }

    f32 scale = stbtt_ScaleForPixelHeight(
            &text_info.font->info, text_info.font_size);

    v2f32 ndc_size =
    {
        scale * text_info.screen_size.x,
        scale * text_info.screen_size.y,
    };
    pos.x *= text_info.screen_size.x;
    pos.y *= text_info.screen_size.y;
    pos.y -= (align_y - 1.0f) * (text_info.font->scale.y) * ndc_size.y;

    Glyphf *g = NULL;
    for (u64 i = 0; i < GLYPH_MAX; ++i)
    {
        if (!text_info.font->glyph[i].loaded)
            continue;
        g = &text_info.glyph[i];

        g->bearing.x = text_info.font->glyph[i].bearing.x * ndc_size.x;
        g->bearing.y = text_info.font->glyph[i].bearing.y * ndc_size.y;
        g->advance = text_info.font->glyph[i].advance * ndc_size.x;
        g->texture_sample.x = text_info.font->glyph[i].texture_sample.x;
        g->texture_sample.y = text_info.font->glyph[i].texture_sample.y;
        g->loaded = TRUE;
    }

    f32 advance = 0.0f;
    f32 descent = text_info.font->descent * ndc_size.y;
    f32 line_height = text_info.font->line_height;
    for (u64 i = 0; i < len; ++i)
    {
        g = &text_info.glyph[text[i]];
        if (text[i] == '\n')
        {
            advance = 0.0f;
            text_info.line += (line_height * ndc_size.y);
            continue;
        }
        if (text[i] == '\t')
        {
            advance +=
                (text_info.glyph[' '].advance * TEXT_TAB_SIZE);
            continue;
        }

        mesh_text.vbo_data[(text_info.cursor * 4) + 0] =
            pos.x + advance + g->bearing.x;

        mesh_text.vbo_data[(text_info.cursor * 4) + 1] =
            -pos.y - descent - text_info.line - g->bearing.y;

        mesh_text.vbo_data[(text_info.cursor * 4) + 2] =
            g->texture_sample.x;

        mesh_text.vbo_data[(text_info.cursor * 4) + 3] =
            g->texture_sample.y;

        advance += g->advance;
        ++text_info.cursor;
    }
}

void
render_text(u32 color)
{
    if (!mesh_text.vbo_data)
    {
        LOGERROR("%s\n", "Failed to Render Text, 'mesh_text.vbo_data' Null");
        return;
    }

    glBindVertexArray(mesh_text.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_text.vbo);
    glBufferData(GL_ARRAY_BUFFER,
            mesh_text.vbo_len * sizeof(GLfloat),
            mesh_text.vbo_data, GL_DYNAMIC_DRAW);

    v2f32 font_size =
    {
        text_info.font_size * text_info.screen_size.x * 0.5f,
        text_info.font_size * text_info.screen_size.y * 0.5f,
    };

    v4f32 text_color =
    {
        (f32)((color >> 0x18) & 0xff) / 0xff,
        (f32)((color >> 0x10) & 0xff) / 0xff,
        (f32)((color >> 0x08) & 0xff) / 0xff,
        (f32)((color >> 0x00) & 0xff) / 0xff,
    };

    glUniform1f(text_info.font->uniform.char_size, text_info.font->char_size);
    glUniform2fv(text_info.font->uniform.font_size, 1, (GLfloat*)&font_size);
    glUniform4fv(text_info.font->uniform.text_color, 1, (GLfloat*)&text_color);

    glDrawArrays(GL_POINTS, 0, text_info.cursor);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    memset(mesh_text.vbo_data, 0, text_info.cursor);
    text_info.cursor = 0;
    text_info.line = 0;
}

void
stop_text(void)
{
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
free_text(void)
{
    free_mesh(&mesh_text);
}
