#ifndef GAME_DIR_H
#define GAME_DIR_H

#include "main.h"
#include "../engine/h/defines.h"
#include "../engine/h/dir.h"
#include "../engine/h/limits.h"

#define DIR_MAX     64

/* ---- section: definitions ------------------------------------------------ */

enum Directories
{
    /* ---- source code directories ----------------------------------------- */
    DIR_SOURCE =            0,
    DIR_SOURCE_SHADERS,

    /* ---- grandpath directories ------------------------------------------- */
    DIR_ROOT_LIB =          0,
    DIR_ROOT_LIB_PLATFORM,
    DIR_ROOT_RESOURCES,
    DIR_ROOT_SHADERS,
    DIR_ROOT_INSTANCES,

    /* ---- instance directories -------------------------------------------- */
    DIR_MODELS =            0,
    DIR_RESOURCES,
    DIR_FONTS,
    DIR_LOGO,
    DIR_TEXTURES,
    DIR_BLOCKS,
    DIR_SKYBOXES,
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

int init_paths(void);

int init_grandpath_directory(void);

// TODO: make "create_instance & create_world" functions
int create_instance(const str *instance_name);

int init_instance_directory(const str *instance_name);

int init_instance_files(void);

int create_world(const str *world_name);

void init_world_directory(const str *world_name);

void init_world_files(const str *world_name);

#endif /* GAME_DIR_H */
