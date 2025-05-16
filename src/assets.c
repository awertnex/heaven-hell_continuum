/* ==== section table ==========================================================
_section_blocks ================================================================
_section_special_blocks ========================================================
_section_items =================================================================
_section_tools =================================================================
*/

#include <stdio.h>
#include <string.h>

#include "h/assets.h"
#include "h/dir.h"

// ---- variables --------------------------------------------------------------
u16 base_texture_size;
TextureLayout one_side;
TextureLayout two_side;
TextureLayout three_side;
TextureLayout three_side_alt;
TextureLayout four_side;

Texture2D textures[3] = {0};
enum TextureNames
{
    TEXTURE_BLOCK_GRASS = 0,
    TEXTURE_BLOCK_COBBLESTONE = 1,
    TEXTURE_BLOCK_COBBLED_DEEPSLATE = 2,
}; /* TextureNames */
str block_names[3][NAME_MAX] =
{
    "grass_block_side.png",
    "cobblestone.png",
    "cobbled_deepslate.png",
};

// ---- functions --------------------------------------------------------------
void init_texture_layouts()
{
    one_side =          (TextureLayout){0, 0,                   0,                      0,                  0,                  0};
    two_side =          (TextureLayout){0, 0,                   base_texture_size,      0,                  0,                  base_texture_size};
    three_side =        (TextureLayout){0, 0,                   base_texture_size,      0,                  0,                  (base_texture_size*2)};
    three_side_alt =    (TextureLayout){0, base_texture_size,   (base_texture_size*2),  0,                  base_texture_size,  (base_texture_size*2)};
    four_side =         (TextureLayout){0, base_texture_size,   (base_texture_size*2),  base_texture_size,  base_texture_size,  (base_texture_size*3)};
}

void init_textures()
{
    base_texture_size = 16;

    str string[PATH_MAX + NAME_MAX] = {0};
    for (u8 i = 0; i < 3; ++i)
    {
        snprintf(string,
                strlen(mc_c_subpath) + strlen(instance_dir_structure[MC_C_DIR_BLOCK]) + strlen(block_names[i]),
                "%s%s%s",
                mc_c_subpath, instance_dir_structure[MC_C_DIR_BLOCK], block_names[i]);

        textures[i] = LoadTexture(string);
    }
}

void unload_textures()
{
    for (u8 i = 0; i < 3; ++i)
        UnloadTexture(textures[i]);
}

// =============================================================================
// _section_blocks =============================================================
// =============================================================================
Block block_grass =
{
    grass,
    BLOCK_STATE_SOLID,
    &three_side,
    &textures[TEXTURE_BLOCK_GRASS],
}; /* Block Grass */

Block block_cobblestone =
{
    cobblestone,
    BLOCK_STATE_SOLID,
    &one_side,
    &textures[TEXTURE_BLOCK_COBBLESTONE],
}; /* Block Cobblestone */

Block block_cobbled_deepslate =
{
    cobbled_deepslate,
    BLOCK_STATE_SOLID,
    &one_side,
    &textures[TEXTURE_BLOCK_COBBLED_DEEPSLATE],
}; /* Block Cobbled Deepslate */

// =============================================================================
// _section_special_blocks =====================================================
// =============================================================================


// =============================================================================
// _section_items ==============================================================
// =============================================================================


// =============================================================================
// _section_tools ==============================================================
// =============================================================================

