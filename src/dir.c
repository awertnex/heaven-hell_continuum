#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <defines.h>

#include "h/main.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/logic.h"

str mc_c_grandpath[PATH_MAX] = {0};
str mc_c_subpath[PATH_MAX] = {0};
str instance_directory_structure[16][NAME_MAX] = {0};
str world_directory_structure[4][NAME_MAX] = {0};

// ---- functions --------------------------------------------------------------
void init_grandpath_directory()
{
    if (!mkdir(mc_c_grandpath, 0775))
        LOGINFO("'minecraft.c' Directory Created at 'HOME/minecraft.c'");
    else
        LOGINFO("'minecraft.c' Directory Exists at 'HOME/minecraft.c'");
}

FILE *instance;
FILE *info;
void init_instance_directory(str *instance_name)
{
    if ((info = fopen("src/info/dir.txt", "r")))
    {
        LOGINFO("Loading Instance Directory Structure");
        str string[PATH_MAX];
        for (u16 i = 0, j = 0, stage = 0; i < 0xFFF && fgets(string, PATH_MAX, info); ++i, ++j)
        {
            if (!strncmp(string, "\n", 1))
                continue;

            if (!strncmp(string, "--main\n", 7))
            {
                fgets(string, PATH_MAX, info);
                stage = 1;
                j = 0;
            }

            if (!strncmp(string, "--world\n", 8))
            {
                fgets(string, PATH_MAX, info);
                stage = 2;
                j = 0;
            }

            switch (stage)
            {
                case 0:
                    break;

                case 1:
                    snprintf(instance_directory_structure[j], strlen(string), "%s", string);
                    break;

                case 2:
                    snprintf(world_directory_structure[j], strlen(string), "%s", string);
                    break;
            }
        }
        fclose(info);
        LOGINFO("Instance Directory Structure Loaded!");
    }
    else
    {
        LOGFATAL("File 'src/info/instance_directory_structure.txt' Not Found, Aborting Instance Creation");
        state &= ~STATE_ACTIVE;
        return;
    }

    snprintf(mc_c_subpath, strlen(mc_c_grandpath) + strlen(instance_name), "%s%s", mc_c_grandpath, instance_name);

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
