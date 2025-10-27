#include "h/assets.h"
#include "h/dir.h"

#include <engine/h/core.h>
#include <engine/h/math.h>
#include <engine/h/memory.h>
#include <engine/h/logger.h>

u32 base_texture_size;
TextureLayout one_side;
TextureLayout two_side;
TextureLayout three_side;
TextureLayout three_side_alt;
TextureLayout four_side;
static GLuint ssbo_texture_handles_id = 0;
static u64 ssbo_texture_handles[BLOCK_COUNT] = {0};

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
    {
        if (block_init(&blocks[i]) != ERR_SUCCESS)
            return *GAME_ERR;
        ssbo_texture_handles[i] = blocks[i].texture.handle;
    }

    glGenBuffers(1, &ssbo_texture_handles_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_texture_handles_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, BLOCK_COUNT * sizeof(u64),
                &ssbo_texture_handles, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void
assets_free(void)
{
    u32 i = 0;
    for (i = 0; i < BLOCK_COUNT; ++i)
        texture_free(&blocks[i].texture);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);
}

u32
block_init(Block *block)
{
    if (!block->name)
    {
        LOGERROR(FALSE, ERR_POINTER_NULL,
                "Failed to Initialize Block [%p], 'block.name' Empty\n",
                &block);
        return *GAME_ERR;
    }

    if (texture_init(&block->texture, block->texture.size,
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                stringf("%s%s", DIR_ROOT[DIR_BLOCKS],
                    block->name)) != ERR_SUCCESS)
        return *GAME_ERR;

    if (texture_generate(&block->texture, TRUE) != ERR_SUCCESS)
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
        .texture =
        {
            .size = (v2i32){16, 16},
        },
    }, /* Grass */

    { /* Dirt */
        .name = "dirt.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture =
        {
            .size = (v2i32){16, 16},
        },
    }, /* Dirt */

    { /* Dirtup */
        .name = "dirtup.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture =
        {
            .size = (v2i32){16, 16},
        },
    }, /* Dirtup */

    { /* Stone */
        .name = "stone.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture =
        {
            .size = (v2i32){16, 16},
        },
    }, /* Stone */

    { /* Sand */
        .name = "sand.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture =
        {
            .size = (v2i32){16, 16},
        },
    }, /* Sand */

    { /* Glass */
        .name = "glass.png",
        .state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture =
        {
            .size = (v2i32){16, 16},
        },
    }, /* Glass */
};

/* ---- special_blocks ------------------------------------------------------ */
/* ---- items --------------------------------------------------------------- */
/* ---- tools --------------------------------------------------------------- */
