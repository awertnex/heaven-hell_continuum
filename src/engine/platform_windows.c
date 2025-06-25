#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"

int make_dir(str *path)
{
    int exit_code = mkdir(path);
    if (exit_code == 0)
        LOGINFO("Directory Created '%s'\n", path);

    return exit_code;
}

/* TODO: work on windows support for _get_path_absolute() */
b8 _get_path_absolute(const str *path, str *path_real)
{
    if (!realpath(path, path_real))
        return FALSE;
    return TRUE;
}

/* TODO: work on windows support for _get_path_bin_root() */
b8 _get_path_bin_root(str *buf)
{
    if (!GetModuleFileNameA(NULL, buf, PATH_MAX - 1))
    {
        LOGFATAL("%s\n", "'get_path_bin_root()' Failed, Process Aborted");
        return FALSE;
    }

    return TRUE;
}

/* TODO: work on windows support for exec() */
b8 exec(str *const *cmd, str *cmd_name)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        LOGFATAL("'%s' Fork Failed, Process Aborted\n", cmd_name);
        return FALSE;
    }
    else if (pid == 0)
    {
        execvp(cmd[0], cmd);
        LOGFATAL("'%s' Failed, Process Aborted\n", cmd_name);
        _exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        LOGFATAL("'%s' Waitpid Failed, Process Aborted\n", cmd_name);
        return FALSE;
    }

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
            LOGINFO("'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
        else
        {
            LOGINFO("'%s' Exit Code: %d\n", cmd_name, exit_code);
            return FALSE;
        }
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        LOGFATAL("'%s' Terminated by Signal: %d, Process Aborted\n", cmd_name, sig);
        return FALSE;
    }
    else
    {
        LOGERROR("'%s' Exited Abnormally, Process Aborted\n", cmd_name);
        return FALSE;
    }

    return TRUE;
}

