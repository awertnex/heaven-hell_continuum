#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <linux/limits.h>

#include "h/dir.h"
#include "h/logger.h"
#include "h/memory.h"

b8 is_file_exists(const str *file_name)
{
    struct stat stats;
    if (stat(file_name, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
            return TRUE;
        else
        {
            LOGERROR("'%s' is Not a Regular File\n", file_name);
            return FALSE;
        }
    }
    LOGERROR("File '%s' Not Found\n", file_name);
    return FALSE;
}

b8 is_dir_exists(const str *dir_name)
{
    struct stat stats;
    if (stat(dir_name, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
            return TRUE;
        else
        {
            LOGERROR("'%s' is Not a Directory\n", dir_name);
            return FALSE;
        }
    }
    LOGERROR("Directory '%s' Not Found\n", dir_name);
    return FALSE;
}

str *get_file_contents(const str *file_name)
{
    FILE *file;
    if ((file = fopen(file_name, "r")) == NULL)
    {
        LOGERROR("File '%s' Not Found\n", file_name);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    str *contents = NULL;
    if (!mem_alloc((void*)&contents, len + 1, "file_contents"))
        goto cleanup;
    fread(contents, 1, len, file);
    contents[len] = '\0';
    fclose(file);
    return contents;

cleanup:
    fclose(file);
    return NULL;
}

/*
 * count = return number of entries inside directory;
 */
str **get_dir_contents(const str *dir_name, u64 *count)
{
    if (!dir_name || !is_dir_exists(dir_name))
        return NULL;

    str *path_absolute = get_path_absolute(dir_name);
    if (!path_absolute)
        goto cleanup;

    DIR *dir = NULL;
    u64 file_count = 0;
    struct dirent *entry;
    str **contents = NULL;

    dir = opendir(path_absolute);
    while ((entry = readdir(dir)))
        ++file_count;

    if (!file_count || !mem_alloc_memb((void*)&contents, file_count, NAME_MAX, "dir_contents"))
        goto cleanup;

    rewinddir(dir);
    u64 i = 0;
    while ((entry = readdir(dir)))
    {
        contents[i] = entry->d_name;
        ++i;
    }

    if (count != NULL)
        *count = file_count;

    closedir(dir);
    free(path_absolute);
    return contents;

cleanup:
    if (path_absolute != NULL)
        free(path_absolute);

    if (dir != NULL)
        closedir(dir);
    return NULL;
}

str *get_path_absolute(const str *path)
{
    if (strlen(path) >= PATH_MAX - 1)
    {
        LOGERROR("%s\n", "Path Too Long");
        return NULL;
    }

    if (!is_dir_exists(path))
        return NULL;

    str path_resolved[PATH_MAX] = {0};
    if (!realpath(path, path_resolved))
        return NULL;

    u64 len = strlen(path_resolved);
    str *path_absolute = NULL;
    if (!mem_alloc((void*)&path_absolute, len + 2, "path_absolute"))
        return NULL;

    strncpy(path_absolute, path_resolved, len);
    if (path_absolute[len - 1] != '/')
        strcat(path_absolute, "/\0");

    return path_absolute;
}

str *get_path_bin_root(void)
{
    str path_bin_root[PATH_MAX] = {0};
    if (!readlink("/proc/self/exe", path_bin_root, PATH_MAX - 1))
    {
        LOGFATAL("%s\n", "'/proc/self/exe' Not Found, Process Aborted");
        return NULL;
    }

    u64 len = strlen(path_bin_root);
    path_bin_root[len] = 0;
    str *result = NULL;
    if (!mem_alloc((void*)&result, len, "path_bin_root"))
        return NULL;

    strncpy(result, path_bin_root, PATH_MAX);

    return result;
}

