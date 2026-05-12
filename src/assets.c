#include "deps/fossil/common/config.h"
#include "deps/fossil/common/types.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shaders.h"

#include "deps/fossil/string.h"

#include "h/assets.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/main.h"

#include <stdio.h>

fsl_mem_handle fbo = {0};
fsl_mem_handle texture = {0};
fsl_mem_handle mesh = {0};
fsl_mem_handle shader = {0};
fsl_mem_handle blocks = {0};
static fsl_mem_handle block_textures = {0};
static GLuint ssbo_texture_indices_id = 0;
static u32 ssbo_texture_indices[BLOCK_COUNT * 6] = {0};
static GLuint ssbo_texture_handles_id = 0;
static u64 ssbo_texture_handles[TEXTURE_BLOCK_COUNT] = {0};
fsl_font *font[FONT_COUNT] = {0};

u32 assets_init(void)
{
    u32 i = 0;
    u32 j = 0;
    fsl_fbo *fbo_p = NULL;
    fsl_texture *texture_p = NULL;
    fsl_shader_program *shader_p = NULL;
    block *blocks_p = NULL;
    fsl_texture *block_textures_p = NULL;
    fsl_font *font_p = NULL;

    font_p = fsl_mem_handle_get(fsl_font, fsl_font_buf);
    font[FONT_REG] =        &font_p[FSL_FONT_INDEX_DEJAVU_SANS];
    font[FONT_REG_BOLD] =   &font_p[FSL_FONT_INDEX_DEJAVU_SANS_BOLD];
    font[FONT_MONO] =       &font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO];
    font[FONT_MONO_BOLD] =  &font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD];

    if (
            fsl_mem_arena_push(&memory_arena_internal, &fbo,
                FBO_COUNT * sizeof(fsl_fbo),
                "assets_init().fbo") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &texture,
                TEXTURE_COUNT * sizeof(fsl_texture),
                "assets_init().texture") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &mesh,
                MESH_COUNT * sizeof(fsl_mesh),
                "assets_init().mesh") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &shader,
                SHADER_COUNT * sizeof(fsl_shader_program),
                "assets_init().shader") != FSL_ERR_SUCCESS ||
 
            fsl_mem_arena_push(&memory_arena_internal, &blocks,
                 BLOCK_COUNT * sizeof(block),
                "assets_init().blocks") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &block_textures,
                TEXTURE_BLOCK_COUNT * sizeof(fsl_texture),
                "assets_init().block_textures") != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- framebuffers ---------------------------------------------------- */

    fbo_p = fsl_mem_handle_get(fsl_fbo, fbo);
    if (
            fsl_fbo_init(&fbo_p[FBO_SKYBOX],     NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_WORLD],      NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_WORLD_MSAA], NULL, TRUE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_HUD],        NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_HUD_MSAA],   NULL, TRUE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_POST_PROCESSING], NULL, FALSE, 4) != FSL_ERR_SUCCESS)

    /* ---- shaders --------------------------------------------------------- */

    shader_p = fsl_mem_handle_get(fsl_shader_program, shader);
     if (
            fsl_asset_set_metadata(&shader_p[SHADER_DEFAULT].asset, FSL_ASSET_SHADER_PROGRAM, "Default", "default", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_DEFAULT].vertex.asset, FSL_ASSET_SHADER, "Default", "default", "default.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_DEFAULT].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_DEFAULT].fragment.asset, FSL_ASSET_SHADER, "Default", "default", "default.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p[SHADER_SKYBOX].asset, FSL_ASSET_SHADER_PROGRAM, "Skybox", "skybox", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_SKYBOX].vertex.asset, FSL_ASSET_SHADER, "Skybox", "skybox", "skybox.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_SKYBOX].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_SKYBOX].fragment.asset, FSL_ASSET_SHADER, "Skybox", "skybox", "skybox.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO].asset, FSL_ASSET_SHADER_PROGRAM, "Gizmo", "gizmo", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO].vertex.asset, FSL_ASSET_SHADER, "Gizmo", "gizmo", "gizmo.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO].fragment.asset, FSL_ASSET_SHADER, "Gizmo", "gizmo", "gizmo.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO_CHUNK].asset, FSL_ASSET_SHADER_PROGRAM, "Gizmo Chunk", "gizmo_chunk", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO_CHUNK].vertex.asset, FSL_ASSET_SHADER, "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO_CHUNK].geometry.asset, FSL_ASSET_SHADER, "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.geom", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_GIZMO_CHUNK].fragment.asset, FSL_ASSET_SHADER, "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p[SHADER_POST_PROCESSING].asset, FSL_ASSET_SHADER_PROGRAM, "Post Processing", "post_processing", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_POST_PROCESSING].vertex.asset, FSL_ASSET_SHADER, "Post Processing", "post_processing", "post_processing.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_POST_PROCESSING].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_POST_PROCESSING].fragment.asset, FSL_ASSET_SHADER, "Post Processing", "post_processing", "post_processing.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p[SHADER_VOXEL].asset, FSL_ASSET_SHADER_PROGRAM, "Voxel", "voxel", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_VOXEL].vertex.asset, FSL_ASSET_SHADER, "Voxel", "voxel", "voxel.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_VOXEL].geometry.asset, FSL_ASSET_SHADER, "Voxel", "voxel", "voxel.geom", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_VOXEL].fragment.asset, FSL_ASSET_SHADER, "Voxel", "voxel", "voxel.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p[SHADER_BOUNDING_BOX].asset, FSL_ASSET_SHADER_PROGRAM, "Bounding Box", "bounding_box", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_BOUNDING_BOX].vertex.asset, FSL_ASSET_SHADER, "Bounding Box", "bounding_box", "bounding_box.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_BOUNDING_BOX].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&shader_p[SHADER_BOUNDING_BOX].fragment.asset, FSL_ASSET_SHADER, "Bounding Box", "bounding_box", "bounding_box.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
            goto cleanup;

    if (
            fsl_shader_program_init(&shader_p[SHADER_DEFAULT]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader_p[SHADER_GIZMO]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader_p[SHADER_GIZMO_CHUNK]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader_p[SHADER_SKYBOX]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader_p[SHADER_POST_PROCESSING]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader_p[SHADER_VOXEL]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader_p[SHADER_BOUNDING_BOX]) != FSL_ERR_SUCCESS)
         goto cleanup;

    /* ---- textures -------------------------------------------------------- */

    if (
            fsl_texture_init(&texture[TEXTURE_CROSSHAIR],
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                GAME_DIR_NAME_GUI"crosshair.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_ITEM_BAR],
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                GAME_DIR_NAME_GUI"item_bar.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_VAL],
                GL_RED, GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE,
                GAME_DIR_NAME_ENV"skybox_val.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_HORIZON],
                GL_RED, GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE,
                GAME_DIR_NAME_ENV"skybox_horizon.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_STARS],
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                GAME_DIR_NAME_ENV"skybox_stars.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SUN],
                    GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                    GAME_DIR_NAME_ENV"sun.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_MOON],
                    GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                    GAME_DIR_NAME_ENV"moon.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < TEXTURE_COUNT; ++i)
        if (fsl_texture_generate(&texture[i], FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;

    /* ---- block textures -------------------------------------------------- */

    if (
            block_texture_init(TEXTURE_BLOCK_GRASS_SIDE, "grass_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_GRASS_TOP, "grass_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_DIRT, "dirt.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_DIRTUP, "dirtup.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_STONE, "stone.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_SAND, "sand.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_GLASS, "glass.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE, "wood_birch_log_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP, "wood_birch_log_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_PLANKS, "wood_birch_planks.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE, "wood_cherry_log_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP, "wood_cherry_log_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_PLANKS, "wood_cherry_planks.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE, "wood_oak_log_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_TOP, "wood_oak_log_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_PLANKS, "wood_oak_planks.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_BLOOD, "block_blood.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
        ssbo_texture_handles[i] = block_textures[i].handle;

    blocks_init();

    for (i = 0, j = 0; i < BLOCK_COUNT; ++i)
    {
        ssbo_texture_indices[j++] = blocks[i].texture_index[0];
        ssbo_texture_indices[j++] = blocks[i].texture_index[1];
        ssbo_texture_indices[j++] = blocks[i].texture_index[2];
        ssbo_texture_indices[j++] = blocks[i].texture_index[3];
        ssbo_texture_indices[j++] = blocks[i].texture_index[4];
        ssbo_texture_indices[j++] = blocks[i].texture_index[5];
    }

    glGenBuffers(1, &ssbo_texture_indices_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_texture_indices_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, BLOCK_COUNT * sizeof(u32) * 6,
                &ssbo_texture_indices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SHADER_BUFFER_BINDING_SSBO_TEXTURE_INDICES,
            ssbo_texture_indices_id);

    glGenBuffers(1, &ssbo_texture_handles_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_texture_handles_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TEXTURE_BLOCK_COUNT * sizeof(u64),
                &ssbo_texture_handles, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SHADER_BUFFER_BINDING_SSBO_TEXTURE_HANDLES,
            ssbo_texture_handles_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    assets_free();
    return *GAME_ERR;
}

void assets_free(void)
{
    u32 i = 0;
    if (block_textures)
        for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
            fsl_texture_free(&block_textures[i]);

    for (i = 0; i < TEXTURE_COUNT; ++i)
        fsl_texture_free(&texture[i]);

    if (ssbo_texture_indices_id)
        glDeleteBuffers(1, &ssbo_texture_indices_id);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);
}

u32 block_texture_init(u32 index, str *name)
{
    if (!name)
    {
        HHC_LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Failed to Initialize Texture [%p], 'name' NULL\n",
                &block_textures[index]));
        goto cleanup;
    }

    if (fsl_texture_init(&block_textures[index],
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                fsl_stringf("%s%s", DIR_ROOT[DIR_BLOCKS], name)) != FSL_ERR_SUCCESS ||

            fsl_texture_generate(&block_textures[index], TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_texture_free(&block_textures[index]);
    return *GAME_ERR;
}

void blocks_init(void)
{
    snprintf(blocks[BLOCK_NONE].name, NAME_MAX, "%s", "block_none");

    snprintf(blocks[BLOCK_GRASS].name, NAME_MAX, "%s", "block_grass");
    blocks[BLOCK_BLOOD].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_BLOOD].texture_index[0] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[1] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[2] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[3] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[4] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[5] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].friction = FRICTION_BLOCK_WET;

    snprintf(blocks[BLOCK_GRASS].name, NAME_MAX, "%s", "block_grass");
    blocks[BLOCK_GRASS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_GRASS].texture_index[0] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[1] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[2] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[3] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[4] = TEXTURE_BLOCK_GRASS_TOP;
    blocks[BLOCK_GRASS].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_GRASS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_DIRT].name, NAME_MAX, "%s", "block_dirt");
    blocks[BLOCK_DIRT].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_DIRT].texture_index[0] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[1] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[2] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[3] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[4] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_DIRTUP].name, NAME_MAX, "%s", "block_dirtup");
    blocks[BLOCK_DIRTUP].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_DIRTUP].texture_index[0] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[1] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[2] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[3] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[4] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[5] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_STONE].name, NAME_MAX, "%s", "block_stone");
    blocks[BLOCK_STONE].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_STONE].texture_index[0] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[1] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[2] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[3] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[4] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[5] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_SAND].name, NAME_MAX, "%s", "block_sand");
    blocks[BLOCK_SAND].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_SAND].texture_index[0] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[1] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[2] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[3] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[4] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[5] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_GLASS].name, NAME_MAX, "%s", "block_glass");
    blocks[BLOCK_GLASS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_GLASS].texture_index[0] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[1] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[2] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[3] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[4] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[5] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_BIRCH_LOG].name, NAME_MAX, "%s", "block_wood_birch_log");
    blocks[BLOCK_WOOD_BIRCH_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks[BLOCK_WOOD_BIRCH_LOG].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_BIRCH_PLANKS].name, NAME_MAX, "%s", "block_wood_birch_planks");
    blocks[BLOCK_WOOD_BIRCH_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_CHERRY_LOG].name, NAME_MAX, "%s", "block_wood_cherry_log");
    blocks[BLOCK_WOOD_CHERRY_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks[BLOCK_WOOD_CHERRY_LOG].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_CHERRY_PLANKS].name, NAME_MAX, "%s", "block_wood_cherry_planks");
    blocks[BLOCK_WOOD_CHERRY_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_OAK_LOG].name, NAME_MAX, "%s", "block_wood_oak_log");
    blocks[BLOCK_WOOD_OAK_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks[BLOCK_WOOD_OAK_LOG].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_OAK_PLANKS].name, NAME_MAX, "%s", "block_wood_oak_planks");
    blocks[BLOCK_WOOD_OAK_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].friction = FRICTION_BLOCK_HARD;
}

/* ---- special_blocks ------------------------------------------------------ */

/* ---- items --------------------------------------------------------------- */

/* ---- tools --------------------------------------------------------------- */

