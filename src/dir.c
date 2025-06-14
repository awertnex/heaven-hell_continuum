#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "engine/h/core.h"
#include "engine/h/platform.h"
#include "engine/h/dir.h"
#include "engine/h/logger.h"
#include "engine/h/memory.h"
#include "h/main.h"
#include "h/platform.h"
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

/* ---- section: functions -------------------------------------------------- */

int init_paths()
{
    if (RELEASE_BUILD)
        snprintf(path_grandpath, PATH_MAX,
                "%s/%sHeaven-Hell Continuum/", getenv(PATH_HOME), PATH_ROAMING);
    else
        snprintf(path_grandpath, PATH_MAX,
                "%sHeaven-Hell Continuum", get_path_bin_root());

    check_slash(path_grandpath);

    if (make_dir(path_grandpath) != 0)
    {
        LOGINFO("Main Directory Path '%s'\n", path_grandpath);
        return 0;
    }

    if (!is_dir_exists(path_grandpath))
    {
        LOGFATAL("Main Directory Creation '%s' Failed, Process Aborted\n", path_grandpath);
        return -1;
    }

    LOGINFO("Main Directory Created '%s'\n", path_grandpath);

    str str_reg[PATH_MAX] = {0};
    LOGINFO("%s\n", "Building Main Directory Structure:");
    for (u8 i = 0; i < 255 && i < arr_len(GRANDPATH_DIR); ++i)
    {
        snprintf(str_reg, PATH_MAX, "%s%s", path_grandpath, GRANDPATH_DIR[i]);
        make_dir(str_reg);

        if (is_dir_exists(str_reg))
            LOGINFO("'%s'\n", GRANDPATH_DIR[i]);
        else
        {
            LOGFATAL("Directory Creation Failed '%s', Process Aborted\n", GRANDPATH_DIR[i]);
            return -1;
        }
    }

    LOGINFO("Main Directory Structure Created '%s'\n", path_grandpath);

    return 0;
}

int init_instance_directory(const str *instance_name)
{
    if (!is_dir_exists(path_grandpath))
    {
        LOGFATAL("Main Directory '%s' Not Found, Instance Creation Failed, Process Aborted\n", path_grandpath);
        return -1;
    }

    snprintf(path_subpath, sizeof(path_subpath), "%s%s%s", path_grandpath, GRANDPATH_DIR[DIR_INSTANCES], instance_name);
    check_slash(path_subpath);

    if (make_dir(path_subpath) != 0)
    {
        /* do instance-opening stuff */
        LOGINFO("Instance Opened '%s'\n", instance_name);
        return 0;
    }

    LOGINFO("Instance Directory Created '%s'\n", path_subpath);

    str str_reg[PATH_MAX] = {0};
    LOGINFO("%s\n", "Building Instance Directory Structure:");
    for (u8 i = 0; (i < 255) && (i < arr_len(INSTANCE_DIR)); ++i)
    {
        snprintf(str_reg, PATH_MAX, "%s%s", path_subpath, INSTANCE_DIR[i]);
        make_dir(str_reg);

        if (is_dir_exists(str_reg))
            LOGINFO("'%s'\n", INSTANCE_DIR[i]);
        else LOGERROR("Directory Creation Failed '%s'\n", INSTANCE_DIR[i]);
    }

    LOGINFO("Instance Created '%s'\n", instance_name);
    /* TODO: create instance binary and show in launcher */

    return 0;
}

void init_world_directory(const str *world_name)
{
    snprintf(path_worldpath, strlen(path_subpath) + 8 + strlen(world_name),
            "%s%s%s", path_subpath, INSTANCE_DIR[DIR_SAVES], world_name);

    check_slash(path_worldpath);

    if (make_dir(path_worldpath) != 0)
    {
        LOGINFO("World Loaded '%s'\n", world_name);
        return;
    }

    LOGINFO("World Directory Created '%s'\n", world_name);

    str str_reg[PATH_MAX] = {0};
    LOGINFO("%s\n", "Building World Directory Structure:");
    for (u8 i = 0; i < 255 && i < arr_len(WORLD_DIR); ++i)
    {
        snprintf(str_reg, PATH_MAX, "%s%s", path_worldpath, WORLD_DIR[i]);
        make_dir(str_reg);

        if (is_dir_exists(str_reg))
            LOGINFO("'%s'\n", WORLD_DIR[i]);
        else LOGERROR("Directory Creation Failed '%s'\n", WORLD_DIR[i]);
    }
    LOGINFO("World Created '%s'\n", world_name);
}

