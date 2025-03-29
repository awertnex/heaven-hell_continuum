#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <defines.h>

#include "h/main.h"
#include "h/logger.h"

str mc_c_grandpath[PATH_MAX] = {0};
str mc_c_subpath[PATH_MAX] = {0};
str mc_c_launcher_path[PATH_MAX] = {0};
str directory_structure[16][NAME_MAX] =
{
    "bin/",
    "saves/",
    "resources/",
    "resources/textures/",
    "resources/sounds/",
    "resources/info/",
    "screenshots/",
};
enum DirectoryStructure
{
    MC_C_DIR_BIN = 0,
    MC_C_DIR_SAVES = 1,
    MC_C_DIR_RESOURCES = 2,
    MC_C_DIR_TEXTURES = 3,
    MC_C_DIR_SOUNDS = 4,
    MC_C_DIR_INFO = 5,
    MC_C_DIR_SCREENSHOTS = 6,
}; /* DirectoryStructure */

void init_grandpath_directory()
{
    if (!mkdir(mc_c_grandpath, 0775))
        LOGINFO("Main Instances Directory Created at 'HOME/minecraft.c");
    else
        LOGINFO("Main Instances Directory Exists at 'HOME/minecraft.c");
}

FILE *instance;
void init_instance_directory(str *instance_name)
{
    snprintf(mc_c_subpath, strlen(mc_c_grandpath) + strlen(instance_name), "%s%s", mc_c_grandpath, instance_name);
    snprintf(mc_c_launcher_path, strlen(mc_c_subpath) + 14, "%s/minecraft_c", mc_c_subpath);

    if (mkdir(mc_c_subpath, 0775))
    {
        instance = fopen(instance_name, "r+");
        LOGINFO("Instance Opened: %s", instance_name);
    }
    else
    {
        instance = fopen(instance_name, "r+");
        LOGINFO("Instance Created: %s", instance_name);
    }

    if (instance != 0) fclose(instance);
}
