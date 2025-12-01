#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>

#include "h/diagnostics.h"
#include "h/platform.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"

u64 get_file_type(const str *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
        return (S_ISREG(stats.st_mode) | (S_ISDIR(stats.st_mode) * 2));

    LOGERROR(TRUE, ERR_FILE_NOT_FOUND, "File '%s' Not Found\n", path);
    return 0;
}

u32 is_file(const str *path)
{
    if (is_file_exists(path, FALSE) != ERR_SUCCESS)
        return engine_err;

    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISREG(stats.st_mode))
    {
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    engine_err = ERR_IS_NOT_FILE;
    return engine_err;
}

u32 is_file_exists(const str *path, b8 log)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
        {
            engine_err = ERR_SUCCESS;
            return engine_err;
        }
        else
        {
            if (log)
            {
                LOGERROR(TRUE, ERR_IS_NOT_FILE,
                        "'%s' is Not a Regular File\n", path);
            }
            else engine_err = ERR_IS_NOT_FILE;
            return engine_err;
        }
    }

    if (log)
    {
        LOGERROR(TRUE, ERR_FILE_NOT_FOUND, "File '%s' Not Found\n", path);
    }
    else engine_err = ERR_FILE_NOT_FOUND;
    return engine_err;
}

u32 is_dir(const str *path)
{
    if (is_dir_exists(path, FALSE) != ERR_SUCCESS)
        return engine_err;

    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode))
    {
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    engine_err = ERR_IS_NOT_DIR;
    return engine_err;
}

u32 is_dir_exists(const str *path, b8 log)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
        {
            engine_err = ERR_SUCCESS;
            return engine_err;
        }
        else
        {
            if (log)
            {
                LOGERROR(TRUE, ERR_IS_NOT_DIR,
                        "'%s' is Not a Directory\n", path);
            }
            else engine_err = ERR_IS_NOT_DIR;
            return engine_err;
        }
    }

    if (log)
    {
        LOGERROR(TRUE, ERR_DIR_NOT_FOUND,
                "Directory '%s' Not Found\n", path);
    }
    else engine_err = ERR_DIR_NOT_FOUND;
    return engine_err;
}

u64 get_file_contents(const str *path, void **destination,
        u64 size, const str *read_format, b8 terminate)
{
    if (is_file_exists(path, TRUE) != ERR_SUCCESS)
            return 0;

    FILE *file = NULL;
    if ((file = fopen(path, read_format)) == NULL)
    {
        LOGERROR(TRUE, ERR_FILE_OPEN_FAIL,
                "File Opening '%s' Failed\n", path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (mem_alloc(destination, len + ((terminate) ? 1 : 0),
                "get_file_contents().destination") != ERR_SUCCESS)
        goto cleanup;

    u64 cursor = fread(*destination, size, len, file);

    fclose(file);
    if (terminate) ((u8*)(*destination))[len] = 0;
    engine_err = ERR_SUCCESS;
    return cursor;

cleanup:
    if (file) fclose(file);
    return 0;
}

Buf get_dir_contents(const str *path)
{
    if (path == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return (Buf){NULL};
    }

    if (is_dir_exists(path, TRUE) != ERR_SUCCESS)
        return (Buf){NULL};

    str *dir_path_absolute = get_path_absolute(path);
    if (!dir_path_absolute)
        goto cleanup;

    str dir_path_absolute_usable[PATH_MAX] = {0};
    snprintf(dir_path_absolute_usable, PATH_MAX, "%s", dir_path_absolute);

    DIR *dir = NULL;
    struct dirent *entry;
    Buf contents = {NULL};

    dir = opendir(dir_path_absolute);
    if (dir == NULL)
    {
        engine_err = ERR_DIR_OPEN_FAIL;
        goto cleanup;
    }

    while ((entry = readdir(dir)) != NULL)
        ++contents.memb;
    contents.memb -= 2;

    if (!contents.memb || mem_alloc_buf(&contents, contents.memb,
                NAME_MAX, "get_dir_contents().dir_contents") != ERR_SUCCESS)
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

        if (is_dir(path_full) == ERR_SUCCESS)
            check_slash(contents.i[i]);
        ++i;
    }

    closedir(dir);
    mem_free((void*)&dir_path_absolute, strlen(dir_path_absolute),
            "get_dir_contents().dir_path_absolute");

    engine_err = ERR_SUCCESS;
    return contents;

cleanup:
    if (dir != NULL)
        closedir(dir);
    mem_free((void*)&dir_path_absolute, strlen(dir_path_absolute),
            "get_dir_contents().dir_path_absolute");
    mem_free_buf((void*)&contents,
            "get_dir_contents().dir_contents");
    return (Buf){NULL};
}

u64 get_dir_entry_count(const str *path)
{
    if (path == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return 0;
    }

    if (is_dir_exists(path, TRUE) != ERR_SUCCESS)
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

    engine_err = ERR_SUCCESS;
    return count;
}

u32 copy_file(const str *path, const str *destination,
        const str *read_format, const str *write_format)
{
    if (is_file_exists(path, TRUE) != ERR_SUCCESS)
            return engine_err;

    str destination_string[PATH_MAX] = {0};
    snprintf(destination_string, PATH_MAX, "%s", destination);

    if (is_dir(destination) == ERR_SUCCESS)
        strncat(destination_string,
                strrchr(path, SLASH_NATIVE), PATH_MAX - 1);

    FILE *in_file = NULL;
    if ((in_file = fopen(destination_string, write_format)) == NULL)
    {
        LOGERROR(FALSE, ERR_FILE_OPEN_FAIL,
                "File Copying '%s' -> '%s' Failed\n",
                path, destination_string);
        return engine_err;
    }

    str *out_file = NULL;
    u64 len = get_file_contents(path, (void*)&out_file, 1, read_format, FALSE);
    if (out_file == NULL)
    {
        fclose(in_file);
        return engine_err;
    }

    fwrite(out_file, 1, len, in_file);
    LOGTRACE(FALSE, "File Copied '%s' -> '%s'\n", path, destination_string);
    engine_err = ERR_SUCCESS;
    fclose(in_file);
    return engine_err;
}

u32 copy_dir(const str *path, const str *destination, b8 overwrite,
        const str *read_format, const str *write_format)
{
    if (is_dir_exists(path, TRUE) != ERR_SUCCESS)
        return engine_err;

    Buf dir_contents = {0};
    dir_contents = get_dir_contents(path);
    if (!dir_contents.loaded)
        return engine_err;

    str path_string[PATH_MAX] = {0};
    snprintf(path_string, PATH_MAX, "%s", path);
    check_slash(path_string);

    str destination_string[PATH_MAX] = {0};
    snprintf(destination_string, PATH_MAX, "%s", destination);
    check_slash(destination_string);

    if (is_dir_exists(destination_string, FALSE) == ERR_SUCCESS && !overwrite)
    {
        strncat(destination_string,
                strrchr(path_string, SLASH_NATIVE), PATH_MAX - 1);
        check_slash(destination_string);
    }
    else make_dir(destination_string);

    str in_dir[PATH_MAX] = {0};
    str out_dir[PATH_MAX] = {0};
    u64 i = 0;
    for (; i < dir_contents.memb; ++i)
    {
        snprintf(in_dir, PATH_MAX - 1, "%s%s",
                path_string, (str*)dir_contents.i[i]);

        snprintf(out_dir, PATH_MAX - 1, "%s%s",
                destination_string, (str*)dir_contents.i[i]);

        if (is_dir(in_dir) == ERR_SUCCESS)
        {
            copy_dir(in_dir, out_dir, 1, read_format, write_format);
            continue;
        }
        copy_file(in_dir, out_dir, read_format, write_format);
    }

    LOGTRACE(FALSE,
            "Directory Copied '%s' -> '%s'\n", path, destination_string);
    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 write_file(const str *path, u64 size, u64 length, void *buf,
        const str *write_format, b8 log)
{
    FILE *file = NULL;
    if ((file = fopen(path, write_format)) == NULL)
    {
        if (log)
        {
            LOGERROR(TRUE, ERR_FILE_OPEN_FAIL,
                    "File Opening '%s' Failed\n", path);
        }
        else engine_err = ERR_FILE_OPEN_FAIL;
        return engine_err;
    }

    fwrite(buf, size, length, file);
    fclose(file);
    LOGTRACE(FALSE, "File Written '%s'\n", path);
    engine_err = ERR_SUCCESS;
    return engine_err;
}

str *get_path_absolute(const str *path)
{
    if (strlen(path) >= PATH_MAX - 1)
    {
        LOGERROR(TRUE, ERR_GET_PATH_ABSOLUTE_FAIL, "%s\n", "Path Too Long");
        return NULL;
    }

    if (is_dir_exists(path, TRUE) != ERR_SUCCESS)
        return NULL;

    str path_absolute[PATH_MAX] = {0};
    if (_get_path_absolute(path, path_absolute) != ERR_SUCCESS)
        return NULL;

    u64 len = strlen(path_absolute);
    str *result = NULL;
    if (mem_alloc((void*)&result, sizeof(str*) * (len + 1),
                "get_path_absolute().path_absolute") != ERR_SUCCESS)
        return NULL;

    engine_err = ERR_SUCCESS;
    strncpy(result, path_absolute, len);
    check_slash(result);
    return result;
}

str *get_path_bin_root(void)
{
    str path_bin_root[PATH_MAX] = {0};
    if (_get_path_bin_root(path_bin_root) != ERR_SUCCESS)
        return NULL;

    u64 len = strlen(path_bin_root);
    if (len >= PATH_MAX - 1)
    {
        LOGFATAL(TRUE, ERR_PATH_TOO_LONG,
                "Path Too Long '%s', Process Aborted\n", path_bin_root);
        return NULL;
    }

    path_bin_root[len] = 0;
    str *result = NULL;
    if (mem_alloc((void*)&result, PATH_MAX,
                "get_path_bin_root().path_bin_root") != ERR_SUCCESS)
        return NULL;

    strncpy(result, path_bin_root, len);

    char *last_slash = strrchr(result, '/');
    if (last_slash)
        *last_slash = 0;
    check_slash(result);
    normalize_slash(result);

    engine_err = ERR_SUCCESS;
    return result;
}

void check_slash(str *path)
{
    if (path == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return;
    }

    u64 len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        engine_err = ERR_PATH_TOO_LONG;
        return;
    }

    if (path[len - 1] == SLASH_NATIVE || path[len - 1] == SLASH_NON_NATIVE)
    {
        engine_err = ERR_SUCCESS;
        return;
    }

    path[len] = SLASH_NATIVE;
    path[len + 1] = 0;
    engine_err = ERR_SUCCESS;
}

void normalize_slash(str *path)
{
    if (path == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return;
    }

    u64 len = strlen(path);
    u64 i = 0;
    for (; i < len; ++i)
    {
        if (path[i] == SLASH_NON_NATIVE)
            path[i] = SLASH_NATIVE;
    }

    engine_err = ERR_SUCCESS;
}

void posix_slash(str *path)
{
    if (path == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return;
    }

    u64 len = strlen(path);
    u64 i = 0;
    for (; i < len; ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }

    engine_err = ERR_SUCCESS;
}

str *retract_path(str *path)
{
    if (path == NULL)
    {
        engine_err = ERR_POINTER_NULL;
        return NULL;
    }

    u64 len = strlen(path);
    if (len <= 1) return path;

    u8 stage = 0;
    u64 i = 0;
    for (; i < len; ++i)
    {
        if (stage == 1 &&
                (path[len - i - 1] == SLASH_NATIVE ||
                 path[len - i - 1] == SLASH_NON_NATIVE))
            break;
        if (path[len - i - 1])
        {
            path[len - i - 1] = 0;
            stage = 1;
        }
    }

    engine_err = ERR_SUCCESS;
    return path;
}
