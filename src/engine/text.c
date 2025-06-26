#define STB_TRUETYPE_IMPLEMENTATION
#include "../../include/stb_truetype_modified.h"

#include "h/text.h"
#include "h/logger.h"

Glyph glyphs[GLYPH_MAX] = {0};

int init_text(const unsigned char *buffer)
{
    stbtt_fontinfo font;
    stbtt_InitFont(&font, buffer, stbtt_GetFontOffsetForIndex(buffer, 0));
    return 0;
}

int load_font(const str *file_name)
{
    return 0;
}

