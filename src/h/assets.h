#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include <engine/h/types.h>
#include <engine/h/limits.h>
#include "dir.h"

enum BlockTexture
{
    BLOCK_TEXTURE_GRASS_SIDE,
    BLOCK_TEXTURE_GRASS_TOP,
    BLOCK_TEXTURE_DIRT,
    BLOCK_TEXTURE_DIRTUP,
    BLOCK_TEXTURE_STONE,
    BLOCK_TEXTURE_SAND,
    BLOCK_TEXTURE_GLASS,
    BLOCK_TEXTURE_COUNT,
}; /* BlockTexture */

typedef enum BlockID
{
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_DIRTUP,
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_GLASS,
    BLOCK_COUNT,
} BlockID;

typedef struct Block
{
    str name[NAME_MAX];
    u16 state;
    u32 texture_index[6]; /* px, nx, py, ny, pz, nz */
} Block;

extern Block *blocks;

/* return non-zero on failure and '*GAME_ERR' is set accordingly */
u32 assets_init(void);

void assets_free(void);

/* index = 'block_textures' index.
 *
 * return non-zero on failure and '*GAME_ERR' is set accordingly */
u32 block_texture_init(u32 index, v2i32 size, str *name);

/* return non-zero on failure and '*GAME_ERR' is set accordingly */
void blocks_init(void);

#endif /* GAME_ASSETS_H */
