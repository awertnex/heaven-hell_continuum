#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <engine/h/core.h>
#include <engine/h/platform.h>
#include <engine/h/dir.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>
#include <engine/h/memory.h>
#include <engine/h/string.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/main.h"

str PATH_ROOT[PATH_MAX] = {0};
str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX] =
{
    [DIR_LOGS] = "logs/",
    [DIR_ASSETS] = "assets/",
    [DIR_AUDIO] = "assets/audio/",
    [DIR_FONTS] = "assets/fonts/",
    [DIR_LOOKUPS] = "assets/lookups/",
    [DIR_MODELS] = "assets/models/",
    [DIR_SHADERS] = "assets/shaders/",
    [DIR_TEXTURES] = "assets/textures/",
    [DIR_BLOCKS] = "assets/textures/blocks/",
    [DIR_ENTITIES] = "assets/textures/entities/",
    [DIR_GUI] = "assets/textures/gui/",
    [DIR_ITEMS] = "assets/textures/items/",
    [DIR_LOGO] = "assets/textures/logo/",
    [DIR_ENV] = "assets/textures/env/",
    [DIR_CONFIG] = "config/",
    [DIR_SCREENSHOTS] = "screenshots/",
    [DIR_TEXT] = "text/",
    [DIR_WORLDS] = "worlds/",
};

str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX] =
{
    [DIR_WORLD_CHUNKS] = "chunks/",
    [DIR_WORLD_ENTITIES] = "entities/",
    [DIR_WORLD_LOGS] = "logs/",
    [DIR_WORLD_PLAYER] = "player/",
};

u32 paths_init(void)
{
    str *path_bin_root = NULL;
    u32 i;

    path_bin_root = get_path_bin_root();
    if (*GAME_ERR != ERR_SUCCESS)
        return *GAME_ERR;

    snprintf(PATH_ROOT, PATH_MAX, "%s", path_bin_root);
    mem_free((void*)&path_bin_root, strlen(path_bin_root),
            "paths_init().path_bin_root");

    LOGINFO(TRUE, "Creating Main Directories '%s'..\n", PATH_ROOT);

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (is_dir_exists(DIR_ROOT[i], FALSE) != ERR_SUCCESS)
            make_dir(DIR_ROOT[i]);

    LOGINFO(TRUE, "Checking Main Directories '%s'..\n", PATH_ROOT);

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (!is_dir_exists(DIR_ROOT[i], TRUE) != ERR_SUCCESS)
            return *GAME_ERR;

    LOGINFO(TRUE, "Main Directory Created '%s'\n", PATH_ROOT);
    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}
