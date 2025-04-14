#ifndef ASSETS_H

#include "../dependencies/raylib-5.5/src/raylib.h"
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
    Grass =             1,
    Dirt =              2,
    Stone =             3,
    CobbleStone =       4,
    MossStone =         5,
    OakWoodLog =        6,
    OakWoodPlanks =     7,
    Sand =              8,
    Glass =             9,
    NetherRack =        10,
    BedRock =           11,
    GlowStone =         12,
    Deepslate =         13,
    CobbledDeepSlate =  14,
}; /* BlockID */

typedef struct block
{
    u8 blockId;
    u16 blockState;
    void *textureLayout;
    void *texture;
} block;

typedef struct texture_layout
{
    u8 px, py, pz;
    u8 nx, ny, nz;
} texture_layout;

// ---- declarations -----------------------------------------------------------
extern u16 baseTextureSize;
extern texture_layout OneSide;
extern texture_layout TwoSide;
extern texture_layout ThreeSide;
extern texture_layout ThreeSideAlt;
extern texture_layout FourSide;

extern Texture2D textureBlockGrass;
extern Texture2D textureBlockCobblestone;
extern Texture2D textureBlockCobbledDeepslate;

// ---- signatures -------------------------------------------------------------
void init_texture_layouts();
void init_textures();
void unload_textures();

#define ASSETS_H
#endif
