#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "../engine/h/defines.h"
#include "dir.h"

enum BlockStateFlags
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
    BLOCK_STATE_ANIMATED =  0x200,
}; /* BlockStateFlags */

enum BlockID
{
    grass,
    dirt,
    stone,
    sand,
    glass,
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

/* ---- section: declarations ----------------------------------------------- */

extern u16 base_texture_size;
extern TextureLayout one_side;
extern TextureLayout two_side;
extern TextureLayout three_side;
extern TextureLayout three_side_alt;
extern TextureLayout four_side;
extern Block block[1023];               /* standard block array */

/* ---- section: signatures ------------------------------------------------- */

void init_textures();
void unload_textures();

#endif /* GAME_ASSETS_H */

