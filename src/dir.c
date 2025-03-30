#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include "h/main.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/logic.h"

str mc_c_grandpath[PATH_MAX] = {0};
str mc_c_subpath[PATH_MAX] = {0};
str mc_c_launcher_path[PATH_MAX] = {0};
str instance_directory_structure[17][NAME_MAX] = {0};
str world_directory_structure[3][NAME_MAX] = {0};

// ---- functions --------------------------------------------------------------
void init_grandpath_directory()
{
    snprintf(mc_c_grandpath, strlen(getenv("HOME")) + 14, "%s/minecraft.c/", getenv("HOME"));
    if (!mkdir(mc_c_grandpath, 0775))
        LOGINFO("Main Directory Created 'HOME/minecraft.c/'");
    else
        LOGINFO("Main Directory Path '%s/minecraft.c/'", getenv("HOME"));
}

FILE *instance;
FILE *info;
void init_instance_directory(str *instance_name)
{
    str string[PATH_MAX];
    if ((info = fopen("src/info/dir.txt", "r")))
    {
        LOGINFO("Loading Instance Directory Structure..");
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
        LOGFATAL("File Not Found 'src/info/dir.txt', Instance Creation Aborted");
        state &= ~STATE_ACTIVE;
        return;
    }

    snprintf(mc_c_subpath, sizeof(mc_c_subpath), "%s%s", mc_c_grandpath, instance_name);
    for (u16 i = 0; i < (PATH_MAX - 1); ++i)
        if (mc_c_subpath[i + 1] == 0 && mc_c_subpath[i] != '/')
        {
            strncat(mc_c_subpath, "/", 2);
            break;
        }

    if (!mkdir(mc_c_subpath, 0775))
    {
        LOGINFO("Instance Directory Created '%s'", mc_c_subpath);
        LOGINFO("Building Instance Directory Structure:");

        memset(string, 0, PATH_MAX);
        for (u8 i = 0; i < 255 && instance_directory_structure[i][0] != 0; ++i)
        {
            snprintf(string,
                    strlen(mc_c_subpath) + strlen(instance_directory_structure[i]),
                    "%s%s", mc_c_subpath, instance_directory_structure[i]);

            mkdir(string, 0775);
            LOGINFO("Directory Created '%s/%s'", instance_name, instance_directory_structure[i]);
        }
        LOGINFO("Instance Creation Complete '%s'", instance_name);
    }
    else LOGINFO("Instance Opened '%s'", instance_name);

    // TODO: make an instance executable and launch it using the launcher screen
}

void init_world_directory()
{
    //TODO: init world directory
}
