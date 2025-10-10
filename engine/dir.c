#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>

#include "h/platform.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"

u64
get_file_type(const str *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
    {
        return (S_ISREG(stats.st_mode) | (S_ISDIR(stats.st_mode) * 2));
    }

    LOGERROR("File '%s' Not Found\n", path);
    return 0;
}

b8
is_file(const str *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISREG(stats.st_mode))
        return TRUE;
    return FALSE;
}

b8
is_file_exists(const str *path, b8 log)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
            return TRUE;
        else
        {
            if (log) LOGERROR("'%s' is Not a Regular File\n", path);
            return FALSE;
        }
    }
    LOGERROR("File '%s' Not Found\n", path);
    return FALSE;
}

b8
is_dir(const str *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode))
        return TRUE;
    return FALSE;
}

b8
is_dir_exists(const str *path, b8 log)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
            return TRUE;
        else
        {
            if (log) LOGERROR("'%s' is Not a Directory\n", path);
            return FALSE;
        }
    }
    if (log) LOGERROR("Directory '%s' Not Found\n", path);
    return FALSE;
}

str *
get_file_contents(const str *path, u64 *file_len, const str *read_format)
{
    if (!is_file_exists(path, TRUE))
            return NULL;

    FILE *file = NULL;
    if ((file = fopen(path, read_format)) == NULL)
    {
        LOGERROR("File Opening '%s' Failed\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    str *file_contents = NULL;
    if (!mem_alloc((void*)&file_contents, len + 1, "file_contents"))
        goto cleanup;

    u64 cursor = fread(file_contents, 1, len, file);
    fclose(file);

    if (file_len) *file_len = cursor;
    return file_contents;

cleanup:
    (file) ? fclose(file) : 0;
    return NULL;
}

buf
get_dir_contents(const str *path)
{
    if (!path || !is_dir_exists(path, TRUE))
        return (buf){NULL};

    str *dir_path_absolute = get_path_absolute(path);
    if (!dir_path_absolute)
        goto cleanup;

    str dir_path_absolute_usable[PATH_MAX] = {0};
    snprintf(dir_path_absolute_usable, PATH_MAX, "%s", dir_path_absolute);

    DIR *dir = NULL;
    struct dirent *entry;
    buf contents = {NULL};

    dir = opendir(dir_path_absolute);
    while ((entry = readdir(dir)) != NULL)
        ++contents.memb;
    contents.memb -= 2;

    if (!contents.memb
            || !mem_alloc_buf(&contents, contents.memb,
                NAME_MAX, "dir_contents"))
        goto cleanup;

    rewinddir(dir);
    u64 i = 0;
    str path_full[PATH_MAX] = {0};
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strncmp(entry->d_name, ".\0", 2) ||
                !strncmp(entry->d_name, "..\0", 3))
            continue;

        contents.i[i] = contents.buf + (i * NAME_MAX);
        memcpy(contents.i[i], entry->d_name, NAME_MAX - 1);
        snprintf(path_full, PATH_MAX, "%s%s",
                dir_path_absolute_usable, entry->d_name);

        if (is_dir(path_full))
            check_slash(contents.i[i]);
        ++i;
    }

    closedir(dir);
    mem_free((void*)&dir_path_absolute,
            strlen(dir_path_absolute), "dir_path_absolute");

    return contents;

cleanup:
    if (dir != NULL)
        closedir(dir);
    mem_free((void*)&dir_path_absolute,
            strlen(dir_path_absolute), "dir_path_absolute");

    mem_free_buf((void*)&contents, "dir_contents");
    return (buf){NULL};
}

u64
get_dir_entry_count(const str *path)
{
    if (!path || !is_dir_exists(path, TRUE))
        return 0;

    DIR *dir = NULL;
    u64 count = 0;
    struct dirent *entry;
    dir = opendir(path);
    if (dir == NULL)
        return 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!strncmp(entry->d_name, ".\0", 2) ||
                !strncmp(entry->d_name, "..\0", 3))
            continue;
        ++count;
    }

    closedir(dir);
    return count;
}

u8
copy_file(const str *path, const str *destination,
        const str *read_format, const str *write_format)
{
    if (!is_file_exists(path, TRUE))
            return -1;

    str destination_string[PATH_MAX] = {0};
    snprintf(destination_string, PATH_MAX, "%s", destination);

    if (is_dir(destination))
        strncat(destination_string,
                strrchr(path, SLASH_NATIVE), PATH_MAX - 1);

    FILE *in_file = NULL;
    if ((in_file = fopen(destination_string, write_format)) == NULL)
    {
        LOGERROR("File Copying '%s' -> '%s' Failed\n",
                path, destination_string);
        return -1;
    }

    u64 len = 0;
    str *out_file = NULL;
    if ((out_file = get_file_contents(path, &len, read_format)) == NULL)
    {
        fclose(in_file);
        return -1;
    }

    fwrite(out_file, 1, len, in_file);
    LOGTRACE("File Copied '%s' -> '%s'\n", path, destination_string);

    fclose(in_file);
    return 0;
}

u8
copy_dir(const str *path, const str *destination, b8 overwrite,
        const str *read_format, const str *write_format)
{
    if (!is_dir_exists(path, TRUE))
        return -1;

    buf dir_contents = {0};
    dir_contents = get_dir_contents(path);
    if (!dir_contents.loaded)
        return -1;

    str path_string[PATH_MAX] = {0};
    snprintf(path_string, PATH_MAX, "%s", path);
    check_slash(path_string);

    str destination_string[PATH_MAX] = {0};
    snprintf(destination_string, PATH_MAX, "%s", destination);
    check_slash(destination_string);

    if (is_dir_exists(destination_string, FALSE) && !overwrite)
    {
        strncat(destination_string,
                strrchr(path_string, SLASH_NATIVE), PATH_MAX - 1);
        check_slash(destination_string);
    }
    else make_dir(destination_string);

    str in_dir[PATH_MAX] = {0};
    str out_dir[PATH_MAX] = {0};
    for (u64 i = 0; i < dir_contents.memb; ++i)
    {
        snprintf(in_dir, PATH_MAX - 1, "%s%s",
                path_string, (str*)dir_contents.i[i]);

        snprintf(out_dir, PATH_MAX - 1, "%s%s",
                destination_string, (str*)dir_contents.i[i]);

        if (is_dir(in_dir))
        {
            copy_dir(in_dir, out_dir, 1, read_format, write_format);
            continue;
        }
        copy_file(in_dir, out_dir, read_format, write_format);
    }

    LOGTRACE("Directory Copied '%s' -> '%s'\n", path, destination_string);
    return 0;
}

str *
get_path_absolute(const str *path)
{
    if (strlen(path) >= PATH_MAX - 1)
    {
        LOGERROR("%s\n", "Path Too Long");
        return NULL;
    }

    if (!is_dir_exists(path, TRUE))
        return NULL;

    str path_absolute[PATH_MAX] = {0};
    if (!_get_path_absolute(path, path_absolute))
        return NULL;

    u64 len = strlen(path_absolute);
    str *result = NULL;
    if (!mem_alloc((void*)&result, sizeof(str*) * (len + 1), "path_absolute"))
        return NULL;

    strncpy(result, path_absolute, len);
    check_slash(result);

    return result;
}

str *
get_path_bin_root(void)
{
    str path_bin_root[PATH_MAX] = {0};
    if (!_get_path_bin_root(path_bin_root))
        return NULL;

    u64 len = strlen(path_bin_root);
    if (len >= PATH_MAX - 1)
    {
        LOGFATAL("Path Too Long '%s', Process Aborted\n", path_bin_root);
        return NULL;
    }

    path_bin_root[len] = 0;
    str *result = NULL;
    if (!mem_alloc((void*)&result, PATH_MAX, "path_bin_root"))
        return NULL;

    strncpy(result, path_bin_root, len);

    char *last_slash = strrchr(result, '/');
    if (last_slash)
        *last_slash = 0;
    check_slash(result);

    return result;
}

void
check_slash(str *path)
{
    if (path == NULL)
        return;

    u64 len = strlen(path);
    if (len >= PATH_MAX - 1)
        return;

    if (path[len - 1] == SLASH_NATIVE)
        return;

    if (path[len - 1] == SLASH_NON_NATIVE)
    {
        path[len - 1] = SLASH_NATIVE;
        path[len] = 0;
        return;
    }

    path[len] = SLASH_NATIVE;
    path[len + 1] = 0;
}

void
normalize_slash(str *path)
{
    if (path == NULL)
        return;

    u64 len = strlen(path);
    for (u64 i = 0; i < len; ++i)
    {
        if (path[i] == SLASH_NON_NATIVE)
            path[i] = SLASH_NATIVE;
    }
}

void
posix_slash(str *path)
{
    if (path == NULL)
        return;

    u64 len = strlen(path);
    for (u64 i = 0; i < len; ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }
}

str *
retract_path(str *path)
{
    u64 len = strlen(path);
    if (len <= 1) return path;

    check_slash(path);
    normalize_slash(path);

    u8 stage = 0;
    for (u64 i = 0; i < len; ++i)
    {
        if (stage == 1 && path[len - i - 1] == SLASH_NATIVE)
            break;
        if (path[len - i - 1])
        {
            path[len - i - 1] = 0;
            stage = 1;
        }
    }

    return path;
}
