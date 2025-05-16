#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include "h/main.h"
#include "h/dir.h"
#include "engine/h/logger.h"

str mc_c_grandpath[PATH_MAX] = {0};
str mc_c_subpath[PATH_MAX] = {0};
str mc_c_launcher_path[PATH_MAX] = {0};
str mc_c_world_path[PATH_MAX] = {0};
str instance_dir_structure[17][NAME_MAX] = {0};
str world_dir_structure[3][NAME_MAX] = {0};
u16 instance_arr_memb = sizeof(instance_dir_structure) / sizeof(instance_dir_structure[0]);
u16 world_arr_memb = sizeof(world_dir_structure) / sizeof(world_dir_structure[0]);

// ---- functions --------------------------------------------------------------
void init_paths()
{
#if RELEASE_MODE
    snprintf(mc_c_grandpath, strlen(getenv(MC_C_HOME)) + 22,
            "%s/%sminecraft.c/", getenv(MC_C_HOME), MC_C_ROAMING); // TODO: test if ROAMING is correct

    if (!mc_c_mkdir(mc_c_grandpath, 0775))
        LOGINFO("%s", "Main Directory Created 'HOME/minecraft.c/'");
    else
        LOGINFO("Main Directory Path '%s/%sminecraft.c/'", getenv("HOME"), MC_C_ROAMING);
#else
    LOGINFO("%s", "Test Instance Directory Path 'test_instance/'");
#endif // RELEASE_MODE
}

FILE *instance;
FILE *info;
void init_instance_directory(str *instance_name, u16 *state, u8 FLAG_ACTIVE)
{
    str str_reg[PATH_MAX] = {0};
    if ((info = fopen("src/info/dir.txt", "r")))
    {
        LOGINFO("%s", "Loading Instance Directory Structure..");
        for (u16 i = 0, j = 0, stage = 0; i < 0xfff && fgets(str_reg, PATH_MAX, info); ++i, ++j)
        {
            if (!strncmp(str_reg, "\n", 1))
                continue;

            if (!strncmp(str_reg, "--main\n", 7))
            {
                fgets(str_reg, PATH_MAX, info);
                stage = 1;
                j = 0;
            }

            if (!strncmp(str_reg, "--world\n", 8))
            {
                fgets(str_reg, PATH_MAX, info);
                stage = 2;
                j = 0;
            }

            switch (stage)
            {
                case 1: snprintf(instance_dir_structure[j], strlen(str_reg), "%s", str_reg);
                    break;
                case 2: snprintf(world_dir_structure[j], strlen(str_reg), "%s", str_reg);
                    break;
            }
        }
        fclose(info);
        LOGINFO("%s", "Instance Directory Structure Loaded!");
        LOGINFO("%s", "World Directory Structure Loaded!");
    }
    else
    {
        LOGFATAL("%s", "File Not Found 'src/info/dir.txt', Instance Creation Aborted");
        *state &= ~FLAG_ACTIVE;
        return;
    }

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
        return;
    }

    memset(str_reg, 0, PATH_MAX);
    LOGINFO("Instance Directory Created '%s'", mc_c_subpath);
    LOGINFO("%s", "Building Instance Directory Structure:");
    for (u8 i = 0; i < 255 && i < instance_arr_memb && instance_dir_structure[i][0] != 0; ++i)
    {
        snprintf(str_reg,
                strlen(mc_c_subpath) + strlen(instance_dir_structure[i]),
                "%s%s", mc_c_subpath, instance_dir_structure[i]);

        mc_c_mkdir(str_reg, 0775);
        LOGINFO("Directory Created '%s/%s'", instance_name, instance_dir_structure[i]);
    }
    LOGINFO("Instance Created '%s'", instance_name);
    // TODO: make an instance executable and launch it using the launcher screen
}

void init_world_directory(str *world_name)
{
    str str_reg[PATH_MAX] = {0};
    u16 arr_len = sizeof(world_dir_structure) / sizeof(world_dir_structure[0]);
    for (u16 i = 0; i < strlen(world_name) - 1; ++i)
        if (world_name[i + 1] == 0 && world_name[i] == '/')
        {
            world_name[i] = 0;
            break;
        }

    snprintf(mc_c_world_path, strlen(mc_c_subpath) + 8 + strlen(world_name),
            "%s%s%s/", mc_c_subpath, "saves/", world_name);
    LOGFATAL("%s", mc_c_world_path);
    if (mc_c_mkdir(mc_c_world_path, 0775))
    {
        LOGINFO("World Loaded '%s'", world_name);
        return;
    }

    LOGINFO("World Directory Created '%s'", world_name);
    LOGINFO("%s", "Building World Directory Structure:");
    for (u8 i = 0; i < 255 && i < world_arr_memb && world_dir_structure[i][0] != 0; ++i)
    {
        snprintf(str_reg,
                strlen(mc_c_world_path) + strlen(world_dir_structure[i]),
                "%s%s", mc_c_world_path, world_dir_structure[i]);

        mc_c_mkdir(str_reg, 0775);
        LOGINFO("Directory Created '%s/%s'", world_name, world_dir_structure[i]);
    }
    LOGINFO("World Created '%s'", world_name);
}

