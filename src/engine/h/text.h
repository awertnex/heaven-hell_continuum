#ifndef MC_C_ENGINE_TEXT_H
#define MC_C_ENGINE_TEXT_H

#include "core.h"
#include "defines.h"

#define GLYPH_MAX   128

typedef struct Glyph
{
    GLuint id;
    v2i16 size;
    v2i16 bearing;
    u64 advance;
} Glyph;

extern Glyph glyphs[GLYPH_MAX];

/* ---- section: signatures ------------------------------------------------- */
int load_font(const str *file_name);
int init_freetype(void);

#endif /* MC_C_ENGINE_TEXT_H */
