#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include "h/main.h"
#include "h/dir.h"
#include "engine/h/logger.h"
#include "engine/h/math.h"

FILE *instance;
str mc_c_grandpath[PATH_MAX] = {0};
str mc_c_subpath[PATH_MAX] = {0};
str mc_c_launcher_path[PATH_MAX] = {0};
str mc_c_world_path[PATH_MAX] = {0};
str grandpath_dir[GRANDPATH_DIR_COUNT][NAME_MAX] =
{
    "instances/",
};
str instance_dir[INSTANCE_DIR_COUNT][NAME_MAX] = 
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
str world_dir[WORLD_DIR_COUNT][NAME_MAX] = 
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
    snprintf(mc_c_grandpath, strlen(getenv(MC_C_HOME)) + 22,
            "%s/%sminecraft.c/", getenv(MC_C_HOME), MC_C_ROAMING); /* TODO: test if ROAMING is correct */

    if (!mc_c_mkdir(mc_c_grandpath, 0775))
        LOGINFO("%s", "Main Directory Created 'HOME/minecraft.c/'");
    else
        LOGINFO("Main Directory Path '%s/%sminecraft.c/'", getenv("HOME"), MC_C_ROAMING);
#else
    LOGINFO("%s", "Test Instance Directory Path 'test_instance/'");
#endif /* RELEASE_MODE */
}

int is_dir_exists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
        if (S_ISDIR(stats.st_mode))
            return 1;
    return 0;
}

int init_instance_directory(str *instance_name)
{
    snprintf(mc_c_subpath, sizeof(mc_c_subpath), "%s%s", mc_c_grandpath, instance_name);
    for (u16 i = 0; i < PATH_MAX - 1; ++i)
        if (mc_c_subpath[i + 1] == 0 && mc_c_subpath[i] != '/')
        {
            strncat(mc_c_subpath, "/", 2);
            break;
        }

    if (mc_c_mkdir(mc_c_subpath, 0775))
    {
        LOGINFO("Instance Opened '%s'", instance_name);
        return 0;
    }

    str str_reg[PATH_MAX] = {0};
    LOGINFO("Instance Directory Created '%s'", mc_c_subpath);
    LOGINFO("%s", "Building Instance Directory Structure:");
    for (u8 i = 0;
            (i < 255) && (i < arr_len(instance_dir)) && (instance_dir[i][0] != 0);
            ++i)
    {
        snprintf(str_reg,
                strlen(mc_c_subpath) + strlen(instance_dir[i]),
                "%s%s", mc_c_subpath, instance_dir[i]);

        if (!is_dir_exists(str_reg))
        {
            mc_c_mkdir(str_reg, 0775);
            LOGINFO("%s", instance_dir[i]);
        }
    }
    LOGINFO("Instance Created '%s'", instance_name);
    /* TODO: create instance binary and show in launcher */
    return 0;
}

void init_world_directory(str *world_name)
{
    snprintf(mc_c_world_path, strlen(mc_c_subpath) + 8 + strlen(world_name),
            "%s%s%s/", mc_c_subpath, instance_dir[DIR_SAVES], world_name);
    if (mc_c_mkdir(mc_c_world_path, 0775))
    {
        LOGINFO("World Loaded '%s'", world_name);
        return;
    }

    str str_reg[PATH_MAX] = {0};
    LOGINFO("World Directory Created '%s'", world_name);
    LOGINFO("%s", "Building World Directory Structure:");
    for (u8 i = 0;
            (i < 255) && (i < arr_len(world_dir)) && (world_dir[i][0] != 0);
            ++i)
    {
        snprintf(str_reg,
                strlen(mc_c_world_path) + strlen(world_dir[i]),
                "%s%s", mc_c_world_path, world_dir[i]);

        mc_c_mkdir(str_reg, 0775);
        LOGINFO("Directory Created '%s'", world_dir[i]);
    }
    LOGINFO("World Created '%s'", world_name);
}

