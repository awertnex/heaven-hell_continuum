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
#include "h/dir.h"

str PATH_ROOT[PATH_MAX] = {0};
str PATH_WORLD[PATH_MAX] = {0};
str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX] = {0};
str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX] = {0};

i32
grandpath_dir_init(void)
{
    snprintf(DIR_ROOT[DIR_LIB],             NAME_MAX, "%s", "lib/");
    snprintf(DIR_ROOT[DIR_LIB_PLATFORM],    NAME_MAX, "%s%s", "lib/", PLATFORM);
    snprintf(DIR_ROOT[DIR_LOGS],            NAME_MAX, "%s", "logs/");
    snprintf(DIR_ROOT[DIR_RESOURCES],       NAME_MAX, "%s", "resources/");
    snprintf(DIR_ROOT[DIR_AUDIO],           NAME_MAX, "%s", "resources/audio/");
    snprintf(DIR_ROOT[DIR_FONTS],           NAME_MAX, "%s", "resources/fonts/");
    snprintf(DIR_ROOT[DIR_LOOKUPS],         NAME_MAX, "%s", "resources/lookups/");
    snprintf(DIR_ROOT[DIR_MODELS],          NAME_MAX, "%s", "resources/models/");
    snprintf(DIR_ROOT[DIR_SHADERS],         NAME_MAX, "%s", "resources/shaders/");
    snprintf(DIR_ROOT[DIR_TEXTURES],        NAME_MAX, "%s", "resources/textures/");
    snprintf(DIR_ROOT[DIR_BLOCKS],          NAME_MAX, "%s", "resources/textures/blocks/");
    snprintf(DIR_ROOT[DIR_ENTITIES],        NAME_MAX, "%s", "resources/textures/entities/");
    snprintf(DIR_ROOT[DIR_GUI],             NAME_MAX, "%s", "resources/textures/gui/");
    snprintf(DIR_ROOT[DIR_ITEMS],           NAME_MAX, "%s", "resources/textures/items/");
    snprintf(DIR_ROOT[DIR_LOGO],            NAME_MAX, "%s", "resources/textures/logo/");
    snprintf(DIR_ROOT[DIR_SKYBOXES],        NAME_MAX, "%s", "resources/textures/skyboxes/");
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
    snprintf(PATH_ROOT, PATH_MAX, "%s", path_bin_root);
    mem_free((void*)&path_bin_root, strlen(path_bin_root), "path_bin_root");

    LOGINFO("%s\n", "Creating Directories..");

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", PATH_ROOT, DIR_ROOT[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(DIR_ROOT[i], PATH_MAX, "%s", string);

        if (!is_dir_exists(string, FALSE))
            make_dir(string);
    }

    LOGINFO("%s\n", "Checking Directories..");

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (!is_dir_exists(DIR_ROOT[i], TRUE))
            return -1;

    LOGINFO("Main Directory Created '%s'\n", PATH_ROOT);
    return 0;
}

i32
world_dir_init(const str *world_name)
{
    str string[PATH_MAX] = {0};
    u32 i = 0;

    if (!is_dir_exists(PATH_ROOT, TRUE))
    {
        LOGERROR("World Creation '%s' Failed\n", world_name);
        return -1;
    }

    if (!is_dir_exists(DIR_ROOT[DIR_WORLDS], TRUE))
    {
        LOGERROR("World Creation '%s' Failed\n", world_name);
        return -1;
    }

    snprintf(PATH_WORLD, PATH_MAX, "%s%s", DIR_ROOT[DIR_WORLDS], world_name);
    check_slash(PATH_WORLD);
    normalize_slash(PATH_WORLD);

    if (is_dir_exists(PATH_WORLD, FALSE))
    {
        LOGERROR("World Already Exists '%s'\n", world_name);
        return 0;
    }

    make_dir(PATH_WORLD);

    LOGINFO("%s\n", "Creating World Directories..");

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", PATH_WORLD, DIR_WORLD[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(DIR_WORLD[i], PATH_MAX, "%s", string);

        make_dir(string);
    }

    LOGINFO("%s\n", "Checking Directories..");

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
        if (!is_dir_exists(DIR_WORLD[i], TRUE))
            return -1;

    LOGINFO("World Created '%s'\n", world_name);
}
