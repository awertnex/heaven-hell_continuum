#ifndef MC_C_ASSETS_H

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../engine/h/defines.h"
#include "dir.h"

enum BlockStates {
    BLOCK_STATE_SOLID =     0x1,
    BLOCK_STATE_FLUID =     0x2,
    BLOCK_STATE_EMISSIVE =  0x4,
    BLOCK_STATE_HOT =       0x8,
    BLOCK_STATE_COLD =      0x10,
    BLOCK_STATE_SLIPPERY =  0x20,
    BLOCK_STATE_BOUNCY =    0x40,
    BLOCK_STATE_SLOW =      0x80,
    BLOCK_STATE_QUICK =     0x100,
    BLOCK_STATE_ANIMATES =  0x200,
}; /* BlockStates */

enum BlockID {
    grass =                 1,
    dirt =                  2,
    stone =                 3,
    cobblestone =           4,
    moss_stone =            5,
    oak_wood_log =          6,
    oak_wood_planks =       7,
    sand =                  8,
    glass =                 9,
    nether_rack =           10,
    bedrock =               11,
    glow_stone =            12,
    deepslate =             13,
    cobbled_deepslate =     14,
}; /* BlockID */

typedef struct Block {
    u8 block_id;
    u16 block_state;
    void *texture_layout;
    void *texture;
} Block;

typedef struct TextureLayout {
    u8 px, py, pz;
    u8 nx, ny, nz;
} TextureLayout;

// ---- declarations -----------------------------------------------------------
extern u16 base_texture_size;
extern TextureLayout one_side;
extern TextureLayout two_side;
extern TextureLayout three_side;
extern TextureLayout three_side_alt;
extern TextureLayout four_side;

extern Texture2D texture_block_grass;
extern Texture2D texture_block_cobblestone;
extern Texture2D texture_block_cobbled_deepslate;

// ---- signatures -------------------------------------------------------------
void init_texture_layouts();
void init_textures();
void unload_textures();

#define MC_C_ASSETS_H
#endif

