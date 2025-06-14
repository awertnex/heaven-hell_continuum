#ifndef GAME_DIR_H
#define GAME_DIR_H

#include <linux/limits.h>

#include "main.h"
#include "../engine/h/defines.h"
#include "../engine/h/dir.h"

#define DIR_MAX     64

/* ---- section: definitions ------------------------------------------------ */
enum Directories
{
    /* ---- source directories ---------------------------------------------- */
    DIR_SOURCE =            0,
    DIR_SOURCE_SHADERS,

    /* ---- grandpath directories ------------------------------------------- */
    DIR_INSTANCES =         0,

    /* ---- instance directories -------------------------------------------- */
    DIR_BIN =               0,
    DIR_MODELS,
    DIR_RESOURCES,
    DIR_TEXTURES,
    DIR_BLOCKS,
    DIR_ENVIRONMENT,
    DIR_ENTITIES,
    DIR_FONT,
    DIR_GUI,
    DIR_ITEMS,
    DIR_LOGO,
    DIR_MISC,
    DIR_PAINTINGS,
    DIR_SHADERS,
    DIR_SOUNDS,
    DIR_INFO,
    DIR_SAVES,
    DIR_SCREENSHOTS,
    DIR_TEXT,

    /* ---- world directories ----------------------------------------------- */
    DIR_ADVANCEMENTS =      0,
    DIR_CHUNKS,
    DIR_ENTITIES_WORLD,
    DIR_LOGS,
    DIR_PLAYER_DATA,
}; /* Directories */

/* ---- section: declarations ----------------------------------------------- */

extern str path_grandpath[PATH_MAX];
extern str path_subpath[PATH_MAX];
extern str path_launcherpath[PATH_MAX];
extern str path_worldpath[PATH_MAX];
extern const str GRANDPATH_DIR[][NAME_MAX];
extern const str INSTANCE_DIR[][NAME_MAX];
extern const str WORLD_DIR[][NAME_MAX];

/* ---- section: signatures ------------------------------------------------- */

void init_paths();
int init_instance_directory(str *instance_name);
void init_world_directory(str *world_name);

#endif /* GAME_DIR_H */

