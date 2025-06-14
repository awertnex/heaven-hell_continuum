#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "engine/h/core.h"
#include "engine/h/platform.h"
#include "engine/h/dir.h"
#include "engine/h/logger.h"
#include "engine/h/memory.h"
#include "h/main.h"
#include "h/dir.h"

FILE *instance;
str path_grandpath[PATH_MAX] = {0};
str path_subpath[PATH_MAX] = {0};
str path_launcherpath[PATH_MAX] = {0};
str path_worldpath[PATH_MAX] = {0};
const str GRANDPATH_DIR[][NAME_MAX] =
{
    "instances/",
};

const str INSTANCE_DIR[][NAME_MAX] = 
{
    "bin/",
    "models/",
    "resources/",
    "resources/textures/",
    "resources/textures/blocks/",
    "resources/textures/environment/",
    "resources/textures/entities/",
    "resources/textures/font/",
    "resources/textures/gui/",
    "resources/textures/items/",
    "resources/textures/logo/",
    "resources/textures/misc/",
    "resources/textures/paintings/",
    "resources/shaders/",
    "resources/sounds/",
    "resources/info/",
    "saves/",
    "screenshots/",
    "text/",
};

const str WORLD_DIR[][NAME_MAX] = 
{
    "advancements/",
    "chunks/",
    "entities/",
    "logs/",
    "player_data/",
};

/* ---- functions ----------------------------------------------------------- */
void init_paths()
{
#if RELEASE_MODE
    snprintf(path_grandpath, strlen(getenv(ENV_HOME)) + 22,
            "%s/%sHeaven-Hell Continuum/", getenv(ENV_HOME), ENV_ROAMING); /* TODO: test if ROAMING is correct */

    if (!make_dir(path_grandpath))
        LOGINFO("%s\n", "Main Directory Created 'HOME/Heaven-Hell Continuum/'");
    else
        LOGINFO("Main Directory Path '%s/%sHeaven-Hell Continuum/'\n", getenv("HOME"), ENV_ROAMING);
#else
    LOGINFO("%s\n", "Test Instance Directory Path 'test_instance/'");
#endif /* RELEASE_MODE */
}

int init_instance_directory(str *instance_name)
{
    snprintf(path_subpath, sizeof(path_subpath), "%s%s", path_grandpath, instance_name);
    for (u16 i = 0; i < PATH_MAX - 1; ++i)
        if (path_subpath[i + 1] == 0 && path_subpath[i] != '/')
        {
            strncat(path_subpath, "/", 2);
            break;
        }

    if (make_dir(path_subpath))
    {
        LOGINFO("Instance Opened '%s'\n", instance_name);
        return 0;
    }

    str str_reg[PATH_MAX] = {0};
    LOGINFO("Instance Directory Created '%s'\n", path_subpath);
    LOGINFO("%s\n", "Building Instance Directory Structure:");
    for (u8 i = 0;
            (i < 255) && (i < arr_len(INSTANCE_DIR)) && (INSTANCE_DIR[i][0] != 0);
            ++i)
    {
        snprintf(str_reg,
                strlen(path_subpath) + strlen(INSTANCE_DIR[i]),
                "%s%s", path_subpath, INSTANCE_DIR[i]);

        if (!is_dir_exists(str_reg))
        {
            make_dir(str_reg);
            LOGINFO("%s\n", INSTANCE_DIR[i]);
        }
    }
    LOGINFO("Instance Created '%s'\n", instance_name);
    /* TODO: create instance binary and show in launcher */
    return 0;
}

void init_world_directory(str *world_name)
{
    snprintf(path_worldpath, strlen(path_subpath) + 8 + strlen(world_name),
            "%s%s%s/", path_subpath, INSTANCE_DIR[DIR_SAVES], world_name);
    if (make_dir(path_worldpath))
    {
        LOGINFO("World Loaded '%s'\n", world_name);
        return;
    }

    str str_reg[PATH_MAX] = {0};
    LOGINFO("World Directory Created '%s'\n", world_name);
    LOGINFO("%s\n", "Building World Directory Structure:");
    for (u8 i = 0;
            (i < 255) && (i < arr_len(WORLD_DIR)) && (WORLD_DIR[i][0] != 0);
            ++i)
    {
        snprintf(str_reg,
                strlen(path_worldpath) + strlen(WORLD_DIR[i]),
                "%s%s", path_worldpath, WORLD_DIR[i]);

        make_dir(str_reg);
        LOGINFO("Directory Created '%s'\n", WORLD_DIR[i]);
    }
    LOGINFO("World Created '%s'\n", world_name);
}

