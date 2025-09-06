#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "h/core.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/memory.h"

/* ---- section: windowing -------------------------------------------------- */

int init_glfw(void)
{
    if (!glfwInit())
    {
        LOGFATAL("%s\n", "Failed to Initialize GLFW, Process Aborted");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return 0;
}

int init_window(Render *render)
{
    render->window = glfwCreateWindow(render->size.x, render->size.y, render->title, NULL, NULL);
    if (!render->window)
    {
        LOGFATAL("%s\n", "Failed to Initialize Window or OpenGL Context, Process Aborted");
        return -1;
    }

    glfwMakeContextCurrent(render->window);
    //glfwSetWindowIcon(render->window, 1, &render->icon); /* TODO: set window icon correctly */
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    return 0;
}

int init_glad(void)
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOGFATAL("%s\n", "Failed to Initialize GLAD, Process Aborted");
        return -1;
    }

    if (GLVersion.major < 3 || (GLVersion.major == 3 && GLVersion.minor < 3))
    {
        LOGFATAL("OpenGL 3.3+ Required, Current Version '%d.%d', Process Aborted\n", GLVersion.major, GLVersion.minor);
        return -1;
    }

    LOGINFO("OpenGL:    %s\n", glGetString(GL_VERSION));
    LOGINFO("GLSL:      %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOGINFO("Vendor:    %s\n", glGetString(GL_VENDOR));
    LOGINFO("Renderer:  %s\n", glGetString(GL_RENDERER));

    return 0;
}

/* ---- section: shaders ---------------------------------------------------- */

int init_shader(const str *shaders_dir, Shader *shader)
{
    str str_reg[PATH_MAX] = {0};
    snprintf(str_reg, PATH_MAX, "%s%s", shaders_dir, shader->file_name);

    if ((shader->source = get_file_contents(str_reg, NULL, "r")) == NULL)
        return -1;
    if (shader->id)
        glDeleteShader(shader->id);

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

int init_shader_program(const str *shaders_dir, ShaderProgram *program)
{
    if (init_shader(shaders_dir, &program->vertex) != 0)
        return -1;
    if (init_shader(shaders_dir, &program->fragment) != 0)
        return -1;
    if (program->id)
        glDeleteProgram(program->id);

    program->id = glCreateProgram();
    glAttachShader(program->id, program->vertex.id);
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
    else LOGINFO("Shader Program %d '%s' Loaded\n", program->id, program->name);

    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

    return 0;
}

/* ---- section: meat ------------------------------------------------------- */

int init_fbo(Render *render, GLuint *fbo, GLuint *color_buf, GLuint *rbo, Mesh *mesh_fbo)
{
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    /* ---- color buffer ---------------------------------------------------- */
    glGenTextures(1, color_buf);
    glBindTexture(GL_TEXTURE_2D, *color_buf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, render->size.x, render->size.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *color_buf, 0);

    /* ---- render buffer --------------------------------------------------- */
    glGenRenderbuffers(1, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, *rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, render->size.x, render->size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *rbo);

    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGFATAL("FBO '%d': Status '%d' Not Complete, Process Aborted\n", *fbo, status);
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* ---- mesh data ------------------------------------------------------- */
    if (mesh_fbo == NULL || mesh_fbo->vbo_data != NULL) return 0;

    mesh_fbo->vbo_len = 24;
    GLfloat vbo_data[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
    };

    if (!mem_alloc((void*)&mesh_fbo->vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len, "mesh_fbo.vbo_data"))
        goto cleanup;
    memcpy(mesh_fbo->vbo_data, vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len);

    /* ---- bind mesh ------------------------------------------------------- */
    glGenVertexArrays(1, &mesh_fbo->vao);
    glGenBuffers(1, &mesh_fbo->vbo);

    glBindVertexArray(mesh_fbo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_fbo->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_fbo->vbo_len * sizeof(GLfloat), mesh_fbo->vbo_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;

cleanup:
    free_mesh(mesh_fbo);
    return -1;
}

b8 generate_texture(GLuint *id, const GLint format, u32 width, u32 height, void *buffer)
{
    if (width <= 2 || height <= 2)
    {
        LOGERROR("Texture Generation '%d' Failed, Size Too Small\n", *id);
        return FALSE;
    }

    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    return TRUE;
}

int generate_mesh(Mesh *mesh, GLenum usage, GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data)
{
    if (!mem_alloc((void*)&mesh->vbo_data, sizeof(GLfloat) * vbo_len, "mesh.vbo_data"))
        goto cleanup;
    if (!mem_alloc((void*)&mesh->ebo_data, sizeof(GLuint) * ebo_len, "mesh.vbo_data"))
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

    glBufferData(GL_ARRAY_BUFFER, mesh->vbo_len * sizeof(GLfloat), mesh->vbo_data, usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo_len * sizeof(GLuint), mesh->ebo_data, usage);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;

cleanup:
    free_mesh(mesh);
    return -1;
}

void draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->ebo_len, GL_UNSIGNED_INT, 0);
}

void free_mesh(Mesh *mesh)
{
    if (mesh->vbo_data == NULL || mesh->ebo_data == NULL) return;
    mem_free((void*)&mesh->vbo_data, sizeof(GLfloat) * mesh->vbo_len, "mesh.vbo_data");
    mem_free((void*)&mesh->ebo_data, sizeof(GLuint) * mesh->ebo_len, "mesh.vbo_data");
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
}

/* ---- section: camera ----------------------------------------------------- */

void update_camera_movement(Camera *camera)
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

void update_camera_perspective(Camera *camera, Projection *projection)
{
    f32 spch = camera->sin_pitch;
    f32 cpch = camera->cos_pitch;
    f32 syaw = camera->sin_yaw;
    f32 cyaw = camera->cos_yaw;

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
            -cyaw * -cpch,  syaw * -cpch,   -spch,  1.0f,
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
            0.0f,   0.0f, -1.0f,    0.0f,
            -1.0f,  0.0f, 0.0f,     0.0f,
            0.0f,   1.0f, 0.0f,     0.0f,
            0.0f,   0.0f, 0.0f,     1.0f,
        };

    /* ---- view ------------------------------------------------------------ */
    projection->view =
        matrix_multiply(projection->translation,
                matrix_multiply(projection->rotation, projection->orientation));

    /* ---- projection ------------------------------------------------------ */
    projection->projection =
        (m4f32){
            fovy / ratio,   0.0f,   0.0f,   0.0f,
            0.0f,           fovy,   0.0f,   0.0f,
            0.0f,           0.0f,   clip,  -1.0f,
            0.0f,           0.0f,   offset, 0.0f,
        };

    projection->perspective = matrix_multiply(projection->view, projection->projection);
}

/* ---- section: input ------------------------------------------------------ */

v2f64 get_mouse_position(Render *render, v2f64 *mouse_position) /* TODO: get mouse position */
{
    return (v2f64){0.0f, 0.0f};
}

v2f64 get_mouse_movement(v2f64 mouse_position, v2f64 *mouse_last) /* TODO: get mouse movement */
{
    v2f64 delta =
    {
        mouse_position.x - mouse_last->x,
        mouse_position.y - mouse_last->y
    };

    *mouse_last = mouse_position;
    return delta;
}

/* ---- section: font ------------------------------------------------------- */

b8 load_font(Font *font, u32 size, const str *font_path)
{
    if (size <= 2)
    {
        LOGERROR("Font Loading '%s' Failed, Font Size Too Small\n", font_path);
        return FALSE;
    }

    if (strlen(font_path) >= PATH_MAX)
    {
        LOGERROR("Font Loading '%s' Failed, Font Path Too Long\n", font_path);
        return FALSE;
    }

    if (!is_file_exists(font_path))
        return FALSE;

    font->buf = (u8*)get_file_contents(font_path, &font->buf_len, "rb");
    if (font->buf == NULL)
        return FALSE;

    if (!stbtt_InitFont(&font->info, font->buf, 0))
    {
        LOGINFO("Font Initializing '%s' Failed\n", font_path);
        goto cleanup;
    }

    if (!mem_alloc((void*)&font->bitmap, GLYPH_MAX * size * size, font_path))
        goto cleanup;

    u8 *canvas = {NULL};
    if (!mem_alloc((void*)&canvas, size * size, "font_glyph_canvas"))
        goto cleanup;

    f32 scale = stbtt_ScaleForPixelHeight(&font->info, size);
    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    snprintf(font->path, PATH_MAX, "%s", font_path);
    font->size = size;
    font->baseline = (int)(font->ascent * scale);
    font->line_height = font->ascent - font->descent + font->line_gap;

    for (i32 i = 0; i < GLYPH_MAX; ++i)
    {
        int glyph_index = stbtt_FindGlyphIndex(&font->info, i);
        if (!glyph_index)
            continue;

        Glyph *g = &font->glyph[i];

        stbtt_GetGlyphHMetrics(&font->info, glyph_index, &g->advance, &g->bearing.x);
        stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph_index, scale, scale, 0.0f, 0.0f, &g->x0, &g->y0, &g->x1, &g->y1);
        g->size.x = g->x1 - g->x0;
        g->size.y = g->y1 - g->y0;
        stbtt_MakeGlyphBitmapSubpixel(&font->info, canvas, g->size.x, g->size.y, size, scale, scale, 0.0f, 0.0f, glyph_index);

        u8 row = i / 16;
        u8 col = i % 16;
        void *bitmap_offset = font->bitmap + (col * size) + (row * size * size * 16);
        for (u32 y = 0; y < size; ++y)
        {
            for (u32 x = 0; x < size; ++x)
                memcpy((bitmap_offset + x + (y * size * 16)), (canvas + x + (y * size)), 1);
        }

        mem_zero((void*)&canvas, size * size, "font_glyph_canvas");
        font->glyph[i].loaded = TRUE;
    }

    if (!generate_texture(&font->id, GL_RED, 16 * size, 16 * size, font->bitmap))
        goto cleanup;

    mem_free((void*)&canvas, size * size, "font_glyph_canvas");
    return TRUE;

cleanup:
    mem_free((void*)&canvas, size * size, "font_glyph_canvas");
    mem_free((void*)&font->buf, font->buf_len, font->path);
    mem_free((void*)&font->bitmap, GLYPH_MAX * size * size, font->path);
    *font = (Font){0};

    return FALSE;
}

void free_font(Font *font)
{
    mem_free((void*)&font->buf, font->buf_len, "file_contents");
    mem_free((void*)&font->bitmap, GLYPH_MAX * font->size * font->size, font->path);
    *font = (Font){0};
}

