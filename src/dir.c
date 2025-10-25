#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <engine/h/core.h>
#include <engine/h/platform.h>
#include <engine/h/dir.h>
#include <engine/h/logger.h>
#include <engine/h/memory.h>
#include "h/main.h"
#include "h/diagnostics.h"
#include "h/dir.h"

u32 *const GAME_DIR_ERR = (u32*)&engine_err;
str PATH_ROOT[PATH_MAX] = {0};
str PATH_WORLD[PATH_MAX] = {0};
str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX] = {0};
str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX] = {0};

u32
grandpath_dir_init(void)
{
    snprintf(DIR_ROOT[DIR_LIB],             NAME_MAX, "%s", "lib/");
    snprintf(DIR_ROOT[DIR_LIB_PLATFORM],    NAME_MAX, "%s%s", "lib/", PLATFORM);
    snprintf(DIR_ROOT[DIR_LOGS],            NAME_MAX, "%s", "logs/");
    snprintf(DIR_ROOT[DIR_ASSETS],          NAME_MAX, "%s", "assets/");
    snprintf(DIR_ROOT[DIR_AUDIO],           NAME_MAX, "%s", "assets/audio/");
    snprintf(DIR_ROOT[DIR_FONTS],           NAME_MAX, "%s", "assets/fonts/");
    snprintf(DIR_ROOT[DIR_LOOKUPS],         NAME_MAX, "%s", "assets/lookups/");
    snprintf(DIR_ROOT[DIR_MODELS],          NAME_MAX, "%s", "assets/models/");
    snprintf(DIR_ROOT[DIR_SHADERS],         NAME_MAX, "%s", "assets/shaders/");
    snprintf(DIR_ROOT[DIR_TEXTURES],        NAME_MAX, "%s", "assets/textures/");
    snprintf(DIR_ROOT[DIR_BLOCKS],          NAME_MAX, "%s", "assets/textures/blocks/");
    snprintf(DIR_ROOT[DIR_ENTITIES],        NAME_MAX, "%s", "assets/textures/entities/");
    snprintf(DIR_ROOT[DIR_GUI],             NAME_MAX, "%s", "assets/textures/gui/");
    snprintf(DIR_ROOT[DIR_ITEMS],           NAME_MAX, "%s", "assets/textures/items/");
    snprintf(DIR_ROOT[DIR_LOGO],            NAME_MAX, "%s", "assets/textures/logo/");
    snprintf(DIR_ROOT[DIR_SKYBOXES],        NAME_MAX, "%s", "assets/textures/skyboxes/");
    snprintf(DIR_ROOT[DIR_SCREENSHOTS],     NAME_MAX, "%s", "screenshots/");
    snprintf(DIR_ROOT[DIR_TEXT],            NAME_MAX, "%s", "text/");
    snprintf(DIR_ROOT[DIR_WORLDS],          NAME_MAX, "%s", "worlds/");

    snprintf(DIR_WORLD[DIR_WORLD_CHUNKS],   NAME_MAX, "%s", "chunks/");
    snprintf(DIR_WORLD[DIR_WORLD_ENTITIES], NAME_MAX, "%s", "entities/");
    snprintf(DIR_WORLD[DIR_WORLD_LOGS],     NAME_MAX, "%s", "logs/");
    snprintf(DIR_WORLD[DIR_WORLD_PLAYER],   NAME_MAX, "%s", "player/");

    str *path_bin_root = NULL;
    str string[PATH_MAX] = {0};
    u32 i = 0;

    path_bin_root = get_path_bin_root();
    if (*GAME_DIR_ERR != ERR_SUCCESS)
        return *GAME_DIR_ERR;

    snprintf(PATH_ROOT, PATH_MAX, "%s", path_bin_root);
    mem_free((void*)&path_bin_root, strlen(path_bin_root), "path_bin_root");

    LOGINFO(TRUE, "Creating Main Directories '%s'..\n", PATH_ROOT);

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", PATH_ROOT, DIR_ROOT[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(DIR_ROOT[i], PATH_MAX, "%s", string);

        if (is_dir_exists(string, FALSE) != ERR_SUCCESS)
            make_dir(string);
    }

    LOGINFO(TRUE, "Checking Main Directories '%s'..\n", PATH_ROOT);

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (!is_dir_exists(DIR_ROOT[i], TRUE) != ERR_SUCCESS)
            return *GAME_DIR_ERR;

    LOGINFO(TRUE, "Main Directory Created '%s'\n", PATH_ROOT);
    *GAME_DIR_ERR = ERR_SUCCESS;
    return *GAME_DIR_ERR;
}

u32
world_dir_init(const str *world_name)
{
    str string[PATH_MAX] = {0};
    u32 i = 0;

    if (is_dir_exists(PATH_ROOT, TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "World Creation '%s' Failed\n", world_name);
        return *GAME_DIR_ERR;
    }

    if (is_dir_exists(DIR_ROOT[DIR_WORLDS], TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "World Creation '%s' Failed\n", world_name);
        return *GAME_DIR_ERR;
    }

    snprintf(PATH_WORLD, PATH_MAX, "%s%s", DIR_ROOT[DIR_WORLDS], world_name);
    check_slash(PATH_WORLD);
    normalize_slash(PATH_WORLD);

    if (is_dir_exists(PATH_WORLD, FALSE) == ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_EXISTS,
                "World Already Exists '%s'\n", world_name);
        return *GAME_DIR_ERR;
    }

    make_dir(PATH_WORLD);

    LOGINFO(FALSE, "Creating World Directories '%s'..\n", PATH_WORLD);

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", PATH_WORLD, DIR_WORLD[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(DIR_WORLD[i], PATH_MAX, "%s", string);

        make_dir(string);
    }

    LOGINFO(FALSE, "Checking World Directories '%s'..\n", PATH_WORLD);

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
        if (is_dir_exists(DIR_WORLD[i], TRUE) != ERR_SUCCESS)
            return *GAME_DIR_ERR;

    LOGINFO(FALSE, "World Created '%s'\n", world_name);
    *GAME_DIR_ERR = ERR_SUCCESS;
    return *GAME_DIR_ERR;
}
