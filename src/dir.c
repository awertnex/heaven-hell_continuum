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
#include "h/dir.h"

str path_grandpath[PATH_MAX] = {0};
str path_subpath[PATH_MAX] = {0};
str path_launcherpath[PATH_MAX] = {0};
str path_worldpath[PATH_MAX] = {0};

str GRANDPATH_DIR[][NAME_MAX] =
{
    "lib/",
    "shaders/",
    "instances/",
};

str INSTANCE_DIR[][NAME_MAX] = 
{
    "models/",
    "resources/",
    "resources/fonts/",
    "resources/logo/",
    "resources/textures/",
    "resources/textures/blocks/",
    "resources/textures/environment/",
    "resources/textures/entities/",
    "resources/textures/gui/",
    "resources/textures/items/",
    "resources/shaders/",
    "resources/audio/",
    "worlds/",
    "screenshots/",
    "text/",
};

str WORLD_DIR[][NAME_MAX] = 
{
    "chunks/",
    "entities/",
    "log/",
    "player_data/",
};

/* ---- section: functions -------------------------------------------------- */

int init_paths(void)
{
    snprintf(path_grandpath, PATH_MAX, "%s", get_path_bin_root());
    check_slash(path_grandpath);

    LOGINFO("Main Directory Path '%s'\n", path_grandpath);

    str str_reg[PATH_MAX] = {0};
    for (u8 i = 0; i < 255 && i < arr_len(GRANDPATH_DIR); ++i)
    {
        snprintf(str_reg, PATH_MAX, "%s%s", path_grandpath, GRANDPATH_DIR[i]);
        snprintf(GRANDPATH_DIR[i], PATH_MAX, "%s", str_reg);

        if (!is_dir_exists(str_reg))
            make_dir(str_reg);
    }

    return 0;
}

int init_instance_directory(const str *instance_name)
{
    if (!is_dir_exists(path_grandpath))
    {
        LOGFATAL("Main Directory '%s' Not Found, Instance Creation Failed, Process Aborted\n", path_grandpath);
        return -1;
    }

    b8 make_dirs = TRUE;
    snprintf(path_subpath, PATH_MAX, "%s%s", GRANDPATH_DIR[DIR_ROOT_INSTANCES], instance_name);
    check_slash(path_subpath);

    if (make_dir(path_subpath) != 0)
    {
        /* do instance-opening stuff */
        LOGINFO("Instance Opened '%s'\n", instance_name);
        make_dirs = FALSE;
    }
    else
        LOGINFO("Instance Directory Created '%s'\n", path_subpath);

    if (make_dirs)
        LOGINFO("%s\n", "Building Instance Directory Structure..");

    str str_reg[PATH_MAX] = {0};
    for (u8 i = 0; (i < 255) && (i < arr_len(INSTANCE_DIR)); ++i)
    {
        snprintf(str_reg, PATH_MAX, "%s%s", path_subpath, INSTANCE_DIR[i]);
        snprintf(INSTANCE_DIR[i], PATH_MAX, "%s", str_reg);
        if (!make_dirs)
            continue;

        make_dir(str_reg);
        if (!is_dir_exists(str_reg))
        {
            LOGFATAL("Directory Creation Failed '%s', Process Aborted\n", str_reg);
            return -1;
        }
    }

    if (make_dirs)
        LOGINFO("Instance Created '%s'\n", instance_name);

    return 0;
}

void init_world_directory(const str *world_name)
{
    snprintf(path_worldpath, PATH_MAX, "%s%s", INSTANCE_DIR[DIR_WORLDS], world_name);
    check_slash(path_worldpath);

    if (make_dir(path_worldpath) != 0)
    {
        LOGERROR("World Already Exists '%s'\n", world_name);
        return;
    }

    LOGINFO("World Directory Created '%s'\n", world_name);

    str str_reg[PATH_MAX] = {0};
    LOGINFO("%s\n", "Building World Directory Structure..");
    for (u8 i = 0; i < 255 && i < arr_len(WORLD_DIR); ++i)
    {
        snprintf(str_reg, PATH_MAX, "%s%s", path_worldpath, WORLD_DIR[i]);
        make_dir(str_reg);

        if (!is_dir_exists(str_reg))
            LOGERROR("Directory Creation Failed '%s'\n", WORLD_DIR[i]);

        snprintf(WORLD_DIR[i], PATH_MAX, "%s", str_reg);
    }

    LOGINFO("World Created '%s'\n", world_name);
}

