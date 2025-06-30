#include "h/text.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/memory.h"

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

    if (!stbtt_InitFont(&font->info, font->buf, stbtt_GetFontOffsetForIndex(font->buf, 0)))
    {
        LOGINFO("Font Initializing '%s' Failed\n", font_path);
        goto cleanup;
    }

    if (!mem_alloc_memb((void*)&font->bitmap, GLYPH_MAX, size * size, font_path))
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
        void *bitmap_offset = font->bitmap + (i * size * size);

        stbtt_GetGlyphHMetrics(&font->info, glyph_index, &g->advance, &g->bearing.x);
        stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph_index, scale, scale, 0.0f, 0.0f, &g->x0, &g->y0, &g->x1, &g->y1);
        g->size.x = g->x1 - g->x0;
        g->size.y = g->y1 - g->y0;
        stbtt_MakeGlyphBitmapSubpixel(&font->info, bitmap_offset, size, size, g->size.x, scale, scale, 0.0f, 0.0f, glyph_index);

        font->glyph[i].loaded = TRUE;
    }

    return TRUE;

cleanup:
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

