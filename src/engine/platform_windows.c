#include "h/dir.h"
#include "h/logger.h"

int make_dir(str *path)
{

    int exit_code = mkdir(path);

    if (exit_code == 0)
        LOGINFO("Directory Created '%s'\n", path);
    else
        LOGINFO("Directory Exists '%s'\n", path);

    return exit_code;
}

/* TODO: make windows support for these functions */
str *_get_path_absolute(const str *path, str *path_real)
{
    return realpath(path, path_real);
}

b8 _get_path_bin_root(str *buf)
{
    if (!readlink("", buf, PATH_MAX - 1))
    {
        LOGFATAL("%s\n", "'' Not Found, Process Aborted");
        return FALSE;
    }

    return TRUE;
}

