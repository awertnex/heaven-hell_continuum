#ifndef MC_C_ASSETS_H

#include "../dependencies/raylib-5.5/include/raylib.h"
#include "../engine/h/defines.h"
#include "dir.h"

enum BlockStates
{
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

enum BlockID
{
    grass = 0,
    dirt,
    stone,
    cobblestone,
    moss_stone,
    oak_wood_log,
    oak_wood_planks,
    sand,
    glass,
    nether_rack,
    bedrock,
    glow_stone,
    deepslate,
    cobbled_deepslate,
}; /* BlockID */

typedef struct Block
{
    str *name;
    u16 block_state;
    void *texture_layout;
    Texture texture;
} Block;

typedef struct TextureLayout
{
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
extern Block block[1023];               // standard block array

// ---- signatures -------------------------------------------------------------
void init_textures();
void unload_textures();

#define MC_C_ASSETS_H
#endif

