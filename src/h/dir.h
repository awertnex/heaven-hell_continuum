#ifndef GAME_DIR_H
#define GAME_DIR_H

#include <engine/h/types.h>
#include <engine/h/dir.h>
#include "main.h"

#define DIR_MAX 128

enum Directories
{
    /* ---- root directories ------------------------------------------------ */

    DIR_LOGS,
    DIR_ASSETS,
    DIR_AUDIO,
    DIR_FONTS,
    DIR_LOOKUPS,
    DIR_MODELS,
    DIR_SHADERS,
    DIR_TEXTURES,
    DIR_BLOCKS,
    DIR_ENTITIES,
    DIR_GUI,
    DIR_ITEMS,
    DIR_LOGO,
    DIR_ENV,
    DIR_CONFIG,
    DIR_SCREENSHOTS,
    DIR_TEXT,
    DIR_WORLDS,
    DIR_ROOT_COUNT,

    /* ---- world directories ----------------------------------------------- */

    DIR_WORLD_CHUNKS = 0,
    DIR_WORLD_ENTITIES,
    DIR_WORLD_LOGS,
    DIR_WORLD_PLAYER,
    DIR_WORLD_COUNT,
}; /* Directories */

extern str PATH_ROOT[PATH_MAX];
extern str PATH_WORLD[PATH_MAX];
extern str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX];
extern str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX];

/*! @brief initialize all paths related to the game.
 *
 *  1. load the binary's root path into the global array 'PATH_ROOT'.
 *  2. add it to the global arrays 'DIR_ROOT' and 'DIR_WORLD' entries.
 *
 *  @remark will create all the directories if not found next to the binary.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 paths_init(void);

/*! @brief initialize world directory structure.
 *
 *  create world directories needed on disk.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 world_dir_init(const str *world_name);

/*! @brief load world and initialize files if not present.
 *
 *  load world data into memory if present and generate and write to disk if not:
 *  1. seed.txt, file containing world seed, can be changed,
 *     but changes only apply if world is loaded again.
 *
 *  @param seed = if file 'seed.txt' not present, it will be created and 'seed'
 *  will be written to it.
 *
 *  @remark if 'seed' is 0, a random seed will be generated.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 world_load(WorldInfo *world, const str *world_name, u64 seed);

#endif /* GAME_DIR_H */
