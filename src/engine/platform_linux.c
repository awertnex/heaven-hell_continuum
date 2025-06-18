#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

#include "h/dir.h"
#include "h/logger.h"

int make_dir(str *path)
{

    int exit_code = mkdir(path, 0775);

    if (exit_code == 0)
        LOGTRACE("Directory Created '%s'\n", path);
    else
        LOGTRACE("Directory Exists '%s'\n", path);

    return exit_code;
}

str *_get_path_absolute(const str *path, str *path_real)
{
    return realpath(path, path_real);
}

b8 _get_path_bin_root(str *buf)
{
    if (!readlink("/proc/self/exe", buf, PATH_MAX - 1))
    {
        LOGFATAL("%s\n", "'/proc/self/exe' Not Found, Process Aborted");
        return FALSE;
    }

    return TRUE;
}

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif /* _GNU_SOURCE */

