#include "h/core.h"
#include "h/diagnostics.h"
#include "h/logger.h"
#include "h/memory.h"

static Mesh mesh_text = {0};
static struct /* text_core */
{
    Font *font;
    Glyphf glyph[GLYPH_MAX];
    f32 font_size;
    f32 line;
    v2f32 ndc_scale;

    /* iterator for internal buffer between text_start() and text_render() */
    GLuint cursor;

    struct /* uniform */
    {
        GLint offset;
        GLint ndc_scale;
    } uniform;
} text_core;

u32 text_init(ShaderProgram *program)
{
    if (mem_alloc((void*)&mesh_text.vbo_data,
                STRING_MAX * sizeof(GLfloat) * 4,
                "text_init().mesh_text.vbo_data") != ERR_SUCCESS)
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

    text_core.uniform.offset =
        glGetUniformLocation(program->id, "offset");
    text_core.uniform.ndc_scale =
        glGetUniformLocation(program->id, "ndc_scale");

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:
    mesh_free(&mesh_text);
    LOGFATAL(FALSE, ERR_TEXT_INIT_FAIL,
            "%s\n", "Failed to Initialize Text, Process Aborted");
    return engine_err;
}

void text_start(u64 length, f32 size, Font *font,
        Render *render, ShaderProgram *program, FBO *fbo, b8 clear)
{
    if (!length)
        length = STRING_MAX;
    else if (length > mesh_text.ebo_len)
    {
        if (mem_realloc((void*)&mesh_text.vbo_data,
                    length * sizeof(GLfloat) * 4,
                    "text_start().mesh_text.vbo_data") != ERR_SUCCESS)
            goto cleanup;
        mesh_text.vbo_len = length * 4;
        mesh_text.ebo_len = length;
    }

    text_core.font = font;
    text_core.font_size = size;
    text_core.line = 0.0f;
    text_core.ndc_scale.x = 2.0f / render->size.x;
    text_core.ndc_scale.y = 2.0f / render->size.y;
    text_core.cursor = 0;

    glUseProgram(program->id);
    glBindTexture(GL_TEXTURE_2D, text_core.font->id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
    return;

cleanup:
    mesh_free(&mesh_text);
    LOGERROR(FALSE, engine_err, "%s\n", "Failed to Start Text");
}

void text_push(const str *text, v2f32 pos, i8 align_x, i8 align_y)
{
    if (!mesh_text.vbo_data)
    {
        LOGERROR(FALSE, ERR_BUFFER_EMPTY,
                "%s\n", "Failed to Push Text, 'mesh_text.vbo_data' Null");
        return;
    }

    u64 len = strlen(text);
    if (len >= STRING_MAX)
    {
        LOGERROR(FALSE, ERR_STRING_TOO_LONG,
                "%s\n", "Failed to Push Text, Text Too Long");
        return;
    }

    if (text_core.cursor + len >= mesh_text.ebo_len)
    {
        if (mem_realloc((void*)&mesh_text.vbo_data,
                    (mesh_text.vbo_len + STRING_MAX) * sizeof(GLfloat) * 4,
                    "text_push().mesh_text.vbo_data") != ERR_SUCCESS)
        {
            mesh_free(&mesh_text);
            LOGERROR(FALSE, engine_err, "%s\n", "Failed to Push Text");
            return;
        }
        mesh_text.vbo_len += STRING_MAX * 4;
        mesh_text.ebo_len += STRING_MAX;
    }

    f32 scale = stbtt_ScaleForPixelHeight(
            &text_core.font->info, text_core.font_size);

    v2f32 ndc_size =
    {
        scale * text_core.ndc_scale.x,
        scale * text_core.ndc_scale.y,
    };
    pos.x *= text_core.ndc_scale.x;
    pos.y *= text_core.ndc_scale.y;
    pos.y += text_core.font->scale.y * ndc_size.y;


    Glyphf *g = NULL;
    u64 i = 0;
    for (; i < GLYPH_MAX; ++i)
    {
        if (!text_core.font->glyph[i].loaded)
            continue;
        g = &text_core.glyph[i];

        g->bearing.x = text_core.font->glyph[i].bearing.x * ndc_size.x;
        g->bearing.y = text_core.font->glyph[i].bearing.y * ndc_size.y;
        g->advance = text_core.font->glyph[i].advance * ndc_size.x;
        g->texture_sample.x = text_core.font->glyph[i].texture_sample.x;
        g->texture_sample.y = text_core.font->glyph[i].texture_sample.y;
        g->loaded = TRUE;
    }

    f32 advance = 0.0f;
    f32 descent = text_core.font->descent * ndc_size.y;
    f32 line_height = text_core.font->line_height;
    for (i = 0; i < len; ++i)
    {
        g = &text_core.glyph[(u64)text[i]];
        if (text[i] == '\n')
        {
            i64 j = 0;
            if (align_x == TEXT_ALIGN_CENTER)
                for (j = 1; (i64)i - j >= 0 && text[i - j] != '\n'; ++j)
                    mesh_text.vbo_data[text_core.cursor - (j * 4)] -=
                        advance / 2.0f;
            else if (align_x == TEXT_ALIGN_RIGHT)
                for (j = 1; (i64)i - j >= 0 && text[i - j] != '\n'; ++j)
                    mesh_text.vbo_data[text_core.cursor - (j * 4)] -=
                        advance;

            advance = 0.0f;
            text_core.line += (line_height * ndc_size.y);
            continue;
        }
        if (text[i] == '\t')
        {
            advance +=
                (text_core.glyph[' '].advance * TEXT_TAB_SIZE);
            continue;
        }

        mesh_text.vbo_data[text_core.cursor++] =
            pos.x + advance + g->bearing.x;

        mesh_text.vbo_data[text_core.cursor++] =
            -pos.y - descent - text_core.line - g->bearing.y;

        mesh_text.vbo_data[text_core.cursor++] =
            g->texture_sample.x;

        mesh_text.vbo_data[text_core.cursor++] =
            g->texture_sample.y;

        advance += g->advance;
    }

    if (align_y == TEXT_ALIGN_CENTER)
        for (i = 0; i < text_core.cursor; i += 4)
            mesh_text.vbo_data[i + 1] += text_core.line / 2.0f;
    else if (align_y == TEXT_ALIGN_BOTTOM)
        for (i = 0; i < text_core.cursor; i += 4)
            mesh_text.vbo_data[i + 1] += text_core.line;
}

void text_render(u32 color, b8 shadow)
{
    if (!mesh_text.vbo_data)
    {
        LOGERROR(FALSE, ERR_BUFFER_EMPTY,
                "%s\n", "Failed to Render Text, 'mesh_text.vbo_data' Null");
        return;
    }

    glBindVertexArray(mesh_text.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_text.vbo);
    glBufferData(GL_ARRAY_BUFFER,
            mesh_text.vbo_len * sizeof(GLfloat),
            mesh_text.vbo_data, GL_DYNAMIC_DRAW);

    glUniform1f(text_core.font->uniform.char_size, text_core.font->char_size);
    glUniform2f(text_core.font->uniform.font_size,
            text_core.font_size * text_core.ndc_scale.x,
            text_core.font_size * text_core.ndc_scale.y);
    glUniform2fv(text_core.uniform.ndc_scale, 1,
            (GLfloat*)&text_core.ndc_scale);

    if (shadow)
    {
        glUniform4f(text_core.font->uniform.text_color,
                (f32)((TEXT_COLOR_SHADOW >> 0x18) & 0xff) / 0xff,
                (f32)((TEXT_COLOR_SHADOW >> 0x10) & 0xff) / 0xff,
                (f32)((TEXT_COLOR_SHADOW >> 0x08) & 0xff) / 0xff,
                (f32)((TEXT_COLOR_SHADOW >> 0x00) & 0xff) / 0xff);
        glUniform2f(text_core.uniform.offset,
                TEXT_OFFSET_SHADOW, TEXT_OFFSET_SHADOW);
        glDrawArrays(GL_POINTS, 0, (text_core.cursor / 4));
    }

    glUniform4f(text_core.font->uniform.text_color,
            (f32)((color >> 0x18) & 0xff) / 0xff,
            (f32)((color >> 0x10) & 0xff) / 0xff,
            (f32)((color >> 0x08) & 0xff) / 0xff,
            (f32)((color >> 0x00) & 0xff) / 0xff);
    glUniform2f(text_core.uniform.offset, 0.0f, 0.0f);
    glDrawArrays(GL_POINTS, 0, (text_core.cursor / 4));

    text_core.cursor = 0;
    text_core.line = 0;
}

void text_stop(void)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void text_free(void)
{
    mesh_free(&mesh_text);
}
