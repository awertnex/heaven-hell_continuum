#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "h/limits.h"
#include "h/logger.h"

int
make_dir(const str *path)
{
    int exit_code = mkdir(path, 0755);
    if (exit_code == 0)
        LOGTRACE("Directory Created '%s'\n", path);
    return exit_code;
}

b8
_get_path_absolute(const str *path, str *path_real)
{
    if (!realpath(path, path_real))
        return FALSE;
    return TRUE;
}

b8
_get_path_bin_root(str *path)
{
    if (!readlink("/proc/self/exe", path, PATH_MAX))
    {
        LOGFATAL("%s\n", "'get_path_bin_root()' Failed, Process Aborted");
        return FALSE;
    }
    return TRUE;
}

b8
exec(buf *cmd, str *cmd_name)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        LOGERROR("'%s' Fork Failed\n", cmd_name);
        return FALSE;
    }
    else if (pid == 0)
    {
        execvp((const str*)cmd->i[0], (str *const *)cmd->i);
        LOGERROR("'%s' Failed\n", cmd_name);
        _exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        LOGERROR("'%s' Waitpid Failed\n", cmd_name);
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
        LOGFATAL("'%s' Terminated by Signal: %d, Process Aborted\n",
                cmd_name, sig);
        return FALSE;
    }
    else
    {
        LOGERROR("'%s' Exited Abnormally\n", cmd_name);
        return FALSE;
    }

    return TRUE;
}

b8 _mem_map(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x != NULL)
        return TRUE;

    *x = mmap(NULL, size,
            PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (*x == NULL)
    {
        LOGFATALV(file, line, "%s[%p] Memory Allocation Failed, Process Aborted\n",
                name, NULL);
        return FALSE;
    }
    LOGTRACEV(file, line, "%s[%p] Memory Allocated[%lldB]\n",
            name, (void*)(uintptr_t)(*x), size);
    return TRUE;
}

void _mem_commit(void** x, u64 offset, u64 size,
        const str *name, const str *file, u64 line)
{
    mprotect(*x + offset, size, PROT_READ | PROT_WRITE);
}

void _mem_unmap(void** x, u64 size,
        const str *name, const str *file, u64 line)
{
}
#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif /* _GNU_SOURCE */
