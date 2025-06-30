#ifndef ENGINE_TEXT_H
#define ENGINE_TEXT_H

#include "core.h"
#include "defines.h"
#include "limits.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../../include/stb_truetype_modified.h"

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

#endif /* ENGINE_TEXT_H */

