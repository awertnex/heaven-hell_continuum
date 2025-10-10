#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include <engine/h/defines.h>
#include "dir.h"

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

extern u16 base_texture_size;
extern TextureLayout one_side;
extern TextureLayout two_side;
extern TextureLayout three_side;
extern TextureLayout three_side_alt;
extern TextureLayout four_side;
extern Block block[1023];               /* standard block array */

void init_textures();
void unload_textures();

#endif /* GAME_ASSETS_H */
