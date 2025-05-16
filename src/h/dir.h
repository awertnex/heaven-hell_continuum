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

enum DirectoryStructure
{
    MC_C_DIR_BIN = 0,
    MC_C_DIR_SAVES = 1,
    MC_C_DIR_RESOURCES = 2,
    MC_C_DIR_TEXTURES = 3,
    MC_C_DIR_BLOCK = 4,
    MC_C_DIR_ENVIRONMENT = 5,
    MC_C_DIR_ENTITY = 6,
    MC_C_DIR_FONT = 7,
    MC_C_DIR_GUI = 8,
    MC_C_DIR_ITEM = 9,
    MC_C_DIR_MISC = 10,
    MC_C_DIR_PAINTING = 11,
    MC_C_DIR_SOUNDS = 12,
    MC_C_DIR_INFO = 13,
    MC_C_DIR_SCREENSHOTS = 14,

    MC_C_DIR_ADVANCEMENTS = 0,
    MC_C_DIR_CHUNKS = 1,
    MC_C_DIR_ENTITIES = 2,
}; /* DirectoryStructure */

// ---- declarations -----------------------------------------------------------
extern str mc_c_grandpath[PATH_MAX];
extern str mc_c_subpath[PATH_MAX];
extern str mc_c_launcher_path[PATH_MAX];
extern str instance_dir_structure[17][NAME_MAX];
extern str world_dir_structure[3][NAME_MAX];

// ---- signatures -------------------------------------------------------------
void init_paths();
void init_instance_directory(str *instance_name, u16 *state, u8 STATE_ACTIVE);
void init_world_directory(str *world_name);

#define MC_C_DIR_H
#endif

