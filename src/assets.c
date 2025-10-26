#include "h/assets.h"
#include "h/dir.h"

#include <engine/h/math.h>
#include <engine/h/memory.h>
#include <engine/h/logger.h>

u32 base_texture_size;
TextureLayout one_side;
TextureLayout two_side;
TextureLayout three_side;
TextureLayout three_side_alt;
TextureLayout four_side;

/* ---- functions ----------------------------------------------------------- */

u32
assets_init(void)
{
    u32 i = 0;
    base_texture_size = 16;

    /* ---- texture layouts ------------------------------------------------- */

    one_side = (TextureLayout){0, 0, 0, 0, 0, 0};
    two_side = (TextureLayout){0, 0, base_texture_size,
        0, 0, base_texture_size};
    three_side = (TextureLayout){0, 0, base_texture_size,
            0, 0, (base_texture_size * 2)};
    three_side_alt = (TextureLayout){
        0, base_texture_size, (base_texture_size * 2),
            0, base_texture_size, (base_texture_size * 2)};
    four_side = (TextureLayout){
        0, base_texture_size, (base_texture_size * 2),
            base_texture_size, base_texture_size,  (base_texture_size * 3)};

    for (i = 0; i < BLOCK_COUNT; ++i)
        if (block_init(&blocks[i]) != ERR_SUCCESS)
            return *GAME_ERR;
}

void
assets_free(void)
{
    u32 i = 0;
    for (i = 0; i < BLOCK_COUNT; ++i)
        texture_free(&blocks[i].texture);
}

u32
block_init(Block *block)
{
    printf("block: %s\n", block->name);
    if (!block->name)
    {
        LOGERROR(FALSE, ERR_POINTER_NULL,
                "%s\n", "Failed to Load Texture, 'block.name' Empty");
        return *GAME_ERR;
    }

    if (texture_init(&block->texture, (v2i32){16, 16},
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                stringf("%s%s", DIR_ROOT[DIR_BLOCKS],
                    block->name)) != ERR_SUCCESS)
        return *GAME_ERR;

    if (texture_generate(&block->texture) != ERR_SUCCESS)
        goto cleanup;

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
cleanup:
    texture_free(&block->texture);
    return *GAME_ERR;
}

/* ---- blocks -------------------------------------------------------------- */

Block blocks[BLOCK_COUNT] =
{
    { /* Grass */
        .name = "grass.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &three_side,
        .texture = 0,
    }, /* Grass */

    { /* Dirt */
        .name = "dirt.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture = 0,
    }, /* Dirt */

    { /* Dirtup */
        .name = "dirtup.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture = 0,
    }, /* Dirtup */

    { /* Stone */
        .name = "stone.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
    }, /* Stone */

    { /* Sand */
        .name = "sand.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
    }, /* Sand */

    { /* Glass */
        .name = "glass.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
    }, /* Glass */
};

/* ---- special_blocks ------------------------------------------------------ */
/* ---- items --------------------------------------------------------------- */
/* ---- tools --------------------------------------------------------------- */
