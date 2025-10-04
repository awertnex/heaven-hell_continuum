#include "h/assets.h"
#include "h/dir.h"

#include "engine/h/math.h"
#include "engine/h/memory.h"

/* ---- declarations -------------------------------------------------------- */

u16 base_texture_size;
TextureLayout one_side;
TextureLayout two_side;
TextureLayout three_side;
TextureLayout three_side_alt;
TextureLayout four_side;
Block block[1023];              /* standard block array */

/* ---- functions ----------------------------------------------------------- */

void
init_textures()
{
    base_texture_size = 16;

    /* ---- texture layouts ------------------------------------------------- */
    one_side = (TextureLayout){0, 0, 0, 0, 0, 0};
    two_side =
        (TextureLayout){0, 0, base_texture_size, 0, 0, base_texture_size};

    three_side =
        (TextureLayout){0, 0, base_texture_size, 0, 0, (base_texture_size*2)};

    three_side_alt = (TextureLayout){
        0, base_texture_size, (base_texture_size*2),
            0, base_texture_size, (base_texture_size*2)};

    four_side = (TextureLayout){
        0, base_texture_size, (base_texture_size*2),
            base_texture_size, base_texture_size,  (base_texture_size*3)};

    for (u16 i = 0; i < arr_len(block) && block[i].name; ++i)
        block[i].texture =
            LoadTexture(TextFormat("%s%s%s.png",
                        path_subpath,
                        INSTANCE_DIR[DIR_BLOCKS],
                        block[i].name));
}

void
unload_textures()
{
    for (u16 i = 0; i < arr_len(block); ++i)
        UnloadTexture(block[i].texture);
}

/* ---- section: blocks ----------------------------------------------------- */

Block block[1023] =
{
    { /* Block Grass */
        .name = "grass",
        .block_state = BLOCK_STATE_SOLID,
        .texture_layout = &three_side,
        .texture = 0,
    }, /* Block Grass */

    { /* Block Dirt */
        .name = "dirt",
        .block_state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture = 0,
    }, /* Block Dirt */

    { /* Block Stone */
        .name = "stone",
        .block_state = BLOCK_STATE_SOLID,
        .texture_layout = &one_side,
        .texture = 0,
    }, /* Block Stone */
};

/* ---- section: special_blocks --------------------------------------------- */


/* ---- section: items ------------------------------------------------------ */


/* ---- section: tools ------------------------------------------------------ */
