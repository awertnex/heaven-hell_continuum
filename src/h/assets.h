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
    BLOCK_TEXTURE_WOOD_BIRCH_LOG_SIDE,
    BLOCK_TEXTURE_WOOD_BIRCH_LOG_TOP,
    BLOCK_TEXTURE_WOOD_BIRCH_PLANKS,
    BLOCK_TEXTURE_WOOD_CHERRY_LOG_SIDE,
    BLOCK_TEXTURE_WOOD_CHERRY_LOG_TOP,
    BLOCK_TEXTURE_WOOD_CHERRY_PLANKS,
    BLOCK_TEXTURE_WOOD_OAK_LOG_SIDE,
    BLOCK_TEXTURE_WOOD_OAK_LOG_TOP,
    BLOCK_TEXTURE_WOOD_OAK_PLANKS,
    BLOCK_TEXTURE_COUNT,
}; /* BlockTexture */

typedef enum BlockID
{
    BLOCK_NONE,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_DIRTUP,
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_GLASS,
    BLOCK_WOOD_BIRCH_LOG,
    BLOCK_WOOD_BIRCH_PLANKS,
    BLOCK_WOOD_CHERRY_LOG,
    BLOCK_WOOD_CHERRY_PLANKS,
    BLOCK_WOOD_OAK_LOG,
    BLOCK_WOOD_OAK_PLANKS,
    BLOCK_COUNT,
} BlockID;

typedef enum BlockState
{
    BLOCK_STATE_SOLID = 1,
} BlockState;

typedef struct Block
{
    str name[NAME_MAX];
    BlockState state;
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
