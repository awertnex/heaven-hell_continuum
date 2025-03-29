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
texture_layout OneSide;
texture_layout TwoSide;
texture_layout ThreeSide;
texture_layout ThreeSideAlt;
texture_layout FourSide;

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
    OneSide =       (texture_layout){0, 0,                  0,                      0,                  0,                  0};
    TwoSide =       (texture_layout){0, 0,                  base_texture_size,      0,                  0,                  base_texture_size};
    ThreeSide =     (texture_layout){0, 0,                  base_texture_size,      0,                  0,                  (base_texture_size*2)};
    ThreeSideAlt =  (texture_layout){0, base_texture_size,  (base_texture_size*2),  0,                  base_texture_size,  (base_texture_size*2)};
    FourSide =      (texture_layout){0, base_texture_size,  (base_texture_size*2),  base_texture_size,  base_texture_size,  (base_texture_size*3)};
}

void init_textures()
{
    base_texture_size = 16;

    str string[PATH_MAX + NAME_MAX] = {0};
    for (u8 i = 0; i < 3; ++i)
    {
        snprintf(string,
                strlen(mc_c_subpath) + strlen(instance_directory_structure[MC_C_DIR_BLOCK]) + strlen(block_names[i]),
                "%s%s%s",
                mc_c_subpath, instance_directory_structure[MC_C_DIR_BLOCK], block_names[i]);

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
block BlockGrass = 
{
    Grass,
    BLOCK_STATE_SOLID,
    &ThreeSide,
    &textures[TEXTURE_BLOCK_GRASS],
}; /* Block Grass */

block BlockCobbleStone =
{
    CobbleStone,
    BLOCK_STATE_SOLID,
    &OneSide,
    &textures[TEXTURE_BLOCK_COBBLESTONE],
}; /* Block Cobblestone */

block BlockCobbledDeepslate =
{
    CobbledDeepSlate,
    BLOCK_STATE_SOLID,
    &OneSide,
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
