#ifndef GAME_DIR_H
#define GAME_DIR_H

#include <engine/h/types.h>
#include <engine/h/dir.h>
#include "main.h"

#define DIR_MAX 128

enum Directories
{
    /* ---- grandpath directories ------------------------------------------- */

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

extern u32 *game_dir_err;
extern str PATH_ROOT[PATH_MAX];
extern str PATH_WORLD[PATH_MAX];
extern str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX];
extern str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX];

/* return non-zero on failure and game_err is set accordingly */
u32 paths_init(void);

/* return non-zero on failure and game_err is set accordingly */
u32 world_dir_init(const str *world_name);

#endif /* GAME_DIR_H */
