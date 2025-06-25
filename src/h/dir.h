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
    DIR_ROOT_LIB =          0,
    DIR_ROOT_SHADERS,
    DIR_ROOT_INSTANCES,

    /* ---- instance directories -------------------------------------------- */
    DIR_MODELS =            0,
    DIR_RESOURCES,
    DIR_FONT,
    DIR_LOGO,
    DIR_TEXTURES,
    DIR_BLOCKS,
    DIR_ENVIRONMENT,
    DIR_ENTITIES,
    DIR_GUI,
    DIR_ITEMS,
    DIR_SHADERS,
    DIR_AUDIO,
    DIR_WORLDS,
    DIR_SCREENSHOTS,
    DIR_TEXT,

    /* ---- world directories ----------------------------------------------- */
    DIR_WORLD_CHUNKS =      0,
    DIR_WORLD_ENTITIES,
    DIR_WORLD_LOG,
    DIR_WORLD_PLAYER_DATA,
}; /* Directories */

/* ---- section: declarations ----------------------------------------------- */

extern str path_grandpath[PATH_MAX];
extern str path_subpath[PATH_MAX];
extern str path_launcherpath[PATH_MAX];
extern str path_worldpath[PATH_MAX];
extern str GRANDPATH_DIR[][NAME_MAX];
extern str INSTANCE_DIR[][NAME_MAX];
extern str WORLD_DIR[][NAME_MAX];

/* ---- section: signatures ------------------------------------------------- */

int init_paths();
int init_instance_directory(const str *instance_name);
void init_world_directory(const str *world_name);

#endif /* GAME_DIR_H */

