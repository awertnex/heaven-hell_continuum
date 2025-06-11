#include "h/text.h"
#include "h/logger.h"

Glyph glyphs[GLYPH_MAX] = {0};

int init_freetype(void)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        LOGFATAL("%s\n", "Failed to Initialize FreeType, Process Aborted");
        return -1;
    }

    FT_Face face;
    str *font = "./fonts/code_saver_regular.otf";
    if (FT_New_Face(ft, font, 0, &face))
    {
        LOGFATAL("Failed to Load Font '%s', Process Aborted\n", font);
        FT_Done_FreeType(ft);
        return -1;
    }

    LOGINFO("Font Loaded '%s'\n", font);

    FT_Set_Pixel_Sizes(face, 0, 48);
    for (u8 c = 0; c < GLYPH_MAX; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            LOGERROR("Failed to Load Glyph '%c'", c);

        glyphs[c] =
            (Glyph){
                .size = (v2i16){face->glyph->bitmap.width, face->glyph->bitmap.rows},
                .bearing = (v2i16){face->glyph->bitmap_left, face->glyph->bitmap_top},
                .advance = face->glyph->advance.x,
            };

        glGenTextures(1, &glyphs['X'].id);
        glBindTexture(GL_TEXTURE_2D, glyphs['X'].id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (glyphs[c].id)
            LOGDEBUG("Glyph %d '%c' Loaded\n", glyphs['X'].id, c);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}

int load_font(const str *file_name)
{
    return 0;
}

