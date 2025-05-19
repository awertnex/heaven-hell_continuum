#ifndef MC_C_DIR_H

#include "main.h"
#include "../engine/h/defines.h"

// ---- from linux/limits.h ----------------------------------------------------
#ifndef NAME_MAX
    #define NAME_MAX    255
#endif // NAME_MAX
#ifndef PATH_MAX
    #define PATH_MAX    4096
#endif // PATH_MAX
// ---- from linux/limits.h ----------------------------------------------------

enum Directories
{
    /* Grandpath Directories */
    DIR_INSTANCES =         0,

    /* Instance Directories */
    DIR_BIN =               0,
    DIR_MODELS =            1,
    DIR_RESOURCES =         2,
    DIR_TEXTURES =          3,
    DIR_BLOCKS =            4,
    DIR_ENVIRONMENT =       5,
    DIR_ENTITIES =          6,
    DIR_FONT =              7,
    DIR_GUI =               8,
    DIR_ITEMS =             9,
    DIR_LOGO =              10,
    DIR_MISC =              11,
    DIR_PAINTINGS =         12,
    DIR_SOUNDS =            13,
    DIR_INFO =              14,
    DIR_SAVES =             15,
    DIR_SCREENSHOTS =       16,
    DIR_TEXT =              17,

    /* World Directories */
    DIR_ADVANCEMENTS =      0,
    DIR_CHUNKS =            1,
    DIR_ENTITIES_WORLD =    2,
    DIR_LOGS =              3,
    DIR_PLAYER_DATA =       4,
}; /* Directories */

#define GRANDPATH_DIR_COUNT 1
#define INSTANCE_DIR_COUNT  18
#define WORLD_DIR_COUNT     5

// ---- declarations -----------------------------------------------------------
extern str mc_c_grandpath[PATH_MAX];
extern str mc_c_subpath[PATH_MAX];
extern str mc_c_launcher_path[PATH_MAX];
extern str grandpath_dir[GRANDPATH_DIR_COUNT][NAME_MAX];
extern str instance_dir[INSTANCE_DIR_COUNT][NAME_MAX];
extern str world_dir[WORLD_DIR_COUNT][NAME_MAX];

// ---- signatures -------------------------------------------------------------
void init_paths();
int is_dir_exists(const char *path);
int init_instance_directory(str *instance_name);
void init_world_directory(str *world_name);

#define MC_C_DIR_H
#endif

