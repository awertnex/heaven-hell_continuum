/* ==== section table ==========================================================
_section_blocks ================================================================
_section_special_blocks ========================================================
_section_items =================================================================
_section_tools =================================================================
*/

#include "h/assets.h"

// ---- variables --------------------------------------------------------------
u16 base_texture_size;
texture_layout OneSide;
texture_layout TwoSide;
texture_layout ThreeSide;
texture_layout ThreeSideAlt;
texture_layout FourSide;

Texture2D texture_block_grass;
Texture2D texture_block_cobblestone;
Texture2D texture_block_cobbled_deepslate;

// ---- functions --------------------------------------------------------------
void init_texture_layouts()
{
	OneSide =		(texture_layout){0, 0,					0,						0,					0,					0};
	TwoSide =		(texture_layout){0, 0,					base_texture_size,		0,					0,					base_texture_size};
	ThreeSide =		(texture_layout){0, 0,					base_texture_size,		0,					0,					(base_texture_size*2)};
	ThreeSideAlt =	(texture_layout){0, base_texture_size,	(base_texture_size*2),	0,					base_texture_size,	(base_texture_size*2)};
	FourSide =		(texture_layout){0, base_texture_size,	(base_texture_size*2),	base_texture_size,	base_texture_size,	(base_texture_size*3)};
}

void init_textures()
{
	base_texture_size = 16;
	texture_block_grass =				LoadTexture(DIR_TEXTURES_BLOCKS"grass_block_side.png");
	texture_block_cobblestone =			LoadTexture(DIR_TEXTURES_BLOCKS"cobblestone.png");
	texture_block_cobbled_deepslate =	LoadTexture(DIR_TEXTURES_BLOCKS"cobbled_deepslate.png");
}

void unload_textures()
{
	UnloadTexture(texture_block_grass);
	UnloadTexture(texture_block_cobblestone);
	UnloadTexture(texture_block_cobbled_deepslate);
}

// =============================================================================
// _section_blocks =============================================================
// =============================================================================

block BlockGrass = 
{
	Grass,
	BLOCK_STATE_SOLID,
	&ThreeSide,
	&texture_block_grass,
}; /* Block Grass */

block BlockCobbleStone =
{
	CobbleStone,
	BLOCK_STATE_SOLID,
	&OneSide,
	&texture_block_cobblestone,
}; /* Block Cobblestone */

block BlockCobbledDeepslate =
{
	CobbledDeepSlate,
	BLOCK_STATE_SOLID,
	&OneSide,
	&texture_block_cobbled_deepslate,
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
