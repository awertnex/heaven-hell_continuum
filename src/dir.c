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
    "lib/"PLATFORM"/",
    "resources/",
    "resources/shaders/",
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
    "resources/textures/skyboxes/",
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

int
init_paths(void)
{
    str *path_bin_root = NULL;

    path_bin_root = get_path_bin_root();
    snprintf(path_grandpath, PATH_MAX, "%s", path_bin_root);

    LOGINFO("Main Directory Path '%s'\n", path_grandpath);

    str string[PATH_MAX] = {0};
    u64 len = arr_len(GRANDPATH_DIR);
    for (u8 i = 0; i < 255 && i < len; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", path_grandpath, GRANDPATH_DIR[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(GRANDPATH_DIR[i], PATH_MAX, "%s", string);

        if (!is_dir_exists(string))
            make_dir(string);
    }

    mem_free((void*)&path_bin_root, strlen(path_bin_root), "path_bin_root");
    return 0;
}

int
create_instance(const str *instance_name)
{
    if (init_instance_directory(instance_name) != 0)
        return -1;
    if (init_instance_files() != 0)
        return -1;
    LOGINFO("Instance Created '%s'\n", instance_name);
    return 0;
}

/* TODO: make editable instance name for init_instance_directory() */
int
init_instance_directory(const str *instance_name)
{
    if (!is_dir_exists(path_grandpath))
    {
        LOGFATAL("Main Directory '%s' Not Found, Instance Creation Failed,"
                "Process Aborted\n", path_grandpath);
        return -1;
    }

    b8 make_dirs = TRUE;
    snprintf(path_subpath, PATH_MAX, "%s%s",
            GRANDPATH_DIR[DIR_ROOT_INSTANCES], instance_name);

    check_slash(path_subpath);
    normalize_slash(path_subpath);

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

    str string[PATH_MAX] = {0};
    u64 len = arr_len(INSTANCE_DIR);
    for (u8 i = 0; (i < 255) && (i < len); ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", path_subpath, INSTANCE_DIR[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(INSTANCE_DIR[i], PATH_MAX, "%s", string);
        if (!make_dirs)
        {
            if (!is_dir_exists(string))
                return -1;
            continue;
        }

        make_dir(string);
        if (!is_dir_exists(string))
        {
            LOGFATAL("Directory Creation Failed '%s',"
                    "Process Aborted\n", string);
            return -1;
        }
    }

    return 0;
}

int
init_instance_files()
{
    if (is_dir_exists(path_subpath))
    {
        copy_dir(GRANDPATH_DIR[DIR_ROOT_RESOURCES],
                INSTANCE_DIR[DIR_RESOURCES], 1);

        copy_dir(GRANDPATH_DIR[DIR_ROOT_SHADERS],
                INSTANCE_DIR[DIR_SHADERS], 1);
        return 0;
    }
    LOGFATAL("Instance Directory '%s' Not Found,"
            "Instance File Creation Failed, Process Aborted\n", path_subpath);
    return -1;
}

void
init_world_directory(const str *world_name)
{
    snprintf(path_worldpath, PATH_MAX, "%s%s",
            INSTANCE_DIR[DIR_WORLDS], world_name);

    check_slash(path_worldpath);
    normalize_slash(path_worldpath);

    if (make_dir(path_worldpath) != 0)
    {
        LOGERROR("World Already Exists '%s'\n", world_name);
        return;
    }

    LOGINFO("World Directory Created '%s'\n", world_name);
    LOGINFO("%s\n", "Building World Directory Structure..");
    str string[PATH_MAX] = {0};
    u64 len = arr_len(WORLD_DIR);
    for (u8 i = 0; i < 255 && i < len; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", path_worldpath, WORLD_DIR[i]);
        check_slash(string);
        normalize_slash(string);
        make_dir(string);

        if (!is_dir_exists(string))
            LOGERROR("Directory Creation Failed '%s'\n", WORLD_DIR[i]);

        snprintf(WORLD_DIR[i], PATH_MAX, "%s", string);
    }

    LOGINFO("World Created '%s'\n", world_name);
}
