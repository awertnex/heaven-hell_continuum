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

b8 is_dir(const str *name)
{
    struct stat stats;
    if (stat(name, &stats) == 0 && S_ISDIR(stats.st_mode))
        return TRUE;
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
    if (!is_file_exists(file_name))
            return NULL;

    FILE *file = NULL;
    if ((file = fopen(file_name, "r")) == NULL)
    {
        LOGERROR("File '%s' Not Accessible\n", file_name);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    str *file_contents = NULL;
    if (!mem_alloc((void*)&file_contents, len + 1, "file_contents"))
        goto cleanup;
    fread(file_contents, 1, len, file);
    file_contents[len] = 0;
    fclose(file);
    return file_contents;

cleanup:
    fclose(file);
    return NULL;
}

/*
 * returns directory contents inside dir_name;
 * count = return number of entries inside directory;
 */
str_buf get_dir_contents(const str *dir_name)
{
    if (!dir_name || !is_dir_exists(dir_name))
        return (str_buf){NULL};

    str *dir_name_absolute = get_path_absolute(dir_name);
    if (!dir_name_absolute)
        goto cleanup;

    str dir_name_absolute_usable[PATH_MAX] = {0};
    snprintf(dir_name_absolute_usable, PATH_MAX, "%s", dir_name_absolute);

    DIR *dir = NULL;
    struct dirent *entry;
    str_buf contents = {NULL};

    dir = opendir(dir_name_absolute);
    while ((entry = readdir(dir)))
        ++contents.count;

    if (!contents.count
            || !mem_alloc((void*)&contents.entry, contents.count * sizeof(str*), "dir_contents")
            || !mem_alloc_memb((void*)&contents.buf, contents.count, NAME_MAX, "dir_contents_buffer"))
        goto cleanup;

    rewinddir(dir);
    u64 i = 0;
    str path_full[PATH_MAX] = {0};
    while ((entry = readdir(dir)))
    {
        contents.entry[i] = contents.buf + (i * NAME_MAX);
        memcpy(contents.entry[i], entry->d_name, NAME_MAX - 1);
        snprintf(path_full, PATH_MAX, "%s%s", dir_name_absolute_usable, entry->d_name);
        if (is_dir(path_full))
            check_slash(contents.entry[i]);
        ++i;
    }

    closedir(dir);
    mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute), "dir_name_absolute");
    return contents;

cleanup:
    if (dir != NULL)
        closedir(dir);
    mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute), "dir_name_absolute");
    mem_free((void*)&contents, sizeof(contents), "dir_contents");
    return (str_buf){NULL};
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

    str path_absolute[PATH_MAX] = {0};
    if (!realpath(path, path_absolute))
        return NULL;

    u64 len = strlen(path_absolute);
    str *result = NULL;
    if (!mem_alloc((void*)&result, len + 1, "path_absolute"))
        return NULL;

    strncpy(result, path_absolute, len);
    check_slash(result);

    return result;
}

/*
 * returns calloc'd string of the executable's current directory, slash and null terminated;
 */
str *get_path_bin_root(void)
{
    str path_bin_root[PATH_MAX] = {0};
    if (!readlink("/proc/self/exe", path_bin_root, PATH_MAX - 1))
    {
        LOGFATAL("%s\n", "'/proc/self/exe' Not Found, Process Aborted");
        return NULL;
    }

    u64 len = strlen(path_bin_root);
    if (len >= PATH_MAX - 1)
    {
        LOGFATAL("Path Too Long '%s', Process Aborted", path_bin_root);
        return NULL;
    }

    path_bin_root[len] = 0;
    str *result = NULL;
    if (!mem_alloc((void*)&result, len + 1, "path_bin_root"))
        return NULL;

    strncpy(result, path_bin_root, len);

    char *last_slash = strrchr(result, '/');
    if (last_slash)
        *last_slash = 0;
    check_slash(result);

    return result;
}

/*
 * appends '/' onto path if there's none, null terminated;
 */
void check_slash(str *path)
{
    if (path == NULL)
        return;

    u64 len = strlen(path);
    if (len >= PATH_MAX)
        return;

    if (path[len - 1] == '/')
        return;

    path[len] = '/';
    path[len + 1] = 0;
}

