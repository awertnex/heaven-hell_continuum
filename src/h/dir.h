#ifndef DIR_H

#include <limits.h>
#include <defines.h>

enum DirectoryStructure
{
    MC_C_DIR_BIN = 0,
    MC_C_DIR_SAVES = 1,
    MC_C_DIR_RESOURCES = 2,
    MC_C_DIR_TEXTURES = 3,
    MC_C_DIR_SOUNDS = 4,
    MC_C_DIR_INFO = 5,
    MC_C_DIR_SCREENSHOTS = 6,

    MC_C_DIR_ADVANCEMENTS = 0,
    MC_C_DIR_CHUNKS = 1,
    MC_C_DIR_ENTITIES = 2,
}; /* DirectoryStructure */

// ---- declarations -----------------------------------------------------------
extern str mc_c_grandpath[PATH_MAX];
extern str mc_c_subpath[PATH_MAX];
extern str instance_directory_structure[16][NAME_MAX];
extern str world_directory_structure[4][NAME_MAX];

#define DIR_H
#endif
