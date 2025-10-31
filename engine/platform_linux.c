#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "h/diagnostics.h"
#include "h/limits.h"
#include "h/logger.h"

int
make_dir(const str *path)
{
    int exit_code = mkdir(path, 0755);
    if (exit_code == 0)
        LOGTRACE(FALSE, "Directory Created '%s'\n", path);
    return exit_code;
}

u32
_get_path_absolute(const str *path, str *path_real)
{
    if (!realpath(path, path_real))
    {
        engine_err = ERR_GET_PATH_ABSOLUTE_FAIL;
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32
_get_path_bin_root(str *path)
{
    if (!readlink("/proc/self/exe", path, PATH_MAX))
    {
        LOGFATAL(FALSE, ERR_GET_PATH_BIN_ROOT_FAIL,
                "%s\n", "'get_path_bin_root()' Failed, Process Aborted");
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32
exec(buf *cmd, str *cmd_name)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        LOGERROR(TRUE, ERR_PROCESS_FORK_FAIL,
                "'%s' Fork Failed\n", cmd_name);
        return engine_err;
    }
    else if (pid == 0)
    {
        execvp((const str*)cmd->i[0], (str *const *)cmd->i);
        LOGERROR(TRUE, ERR_EXEC_FAIL, "'%s' Failed\n", cmd_name);
        return engine_err;
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        LOGERROR(TRUE, ERR_WAITPID_FAIL, "'%s' Waitpid Failed\n", cmd_name);
        return engine_err;
    }

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
        {
            LOGINFO(FALSE,
                    "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
        }
        else
        {
            engine_err = ERR_EXEC_PROCESS_NON_ZERO;
            LOGINFO(TRUE, "'%s' Exit Code: %d\n", cmd_name, exit_code);
            return engine_err;
        }
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        LOGFATAL(TRUE, ERR_EXEC_TERMINATE_BY_SIGNAL,
                "'%s' Terminated by Signal: %d, Process Aborted\n",
                cmd_name, sig);
        return engine_err;
    }
    else
    {
        LOGERROR(TRUE, ERR_EXEC_ABNORMAL_EXIT,
                "'%s' Exited Abnormally\n", cmd_name);
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32
_mem_map(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (*x != NULL)
    {
        engine_err = ERR_POINTER_NOT_NULL;
        return engine_err;
    }

    *x = mmap(NULL, size,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (*x == MAP_FAILED)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_MAP_FAIL,
                "%s[%p] Memory Map Failed, Process Aborted\n", name, NULL);
        return engine_err;
    }
    LOGTRACEEX(TRUE, file, line,
            "%s[%p] Memory Mapped [%"PRId64"B]\n", name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32
_mem_commit(void **x, void *offset, u64 size,
        const str *name, const str *file, u64 line)
{
    if (!x)
    {
        LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p][%p] Memory Commit [%"PRId64"B] Failed, Pointer NULL\n",
                name, x, offset, size);
        return engine_err;
    }

    if (mprotect(offset, size, PROT_READ | PROT_WRITE) != 0)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_COMMIT_FAIL,
                "%s[%p][%p] Memory Commit [%"PRId64"B] Failed, Process Aborted\n",
                name, *x, offset, size);
        return engine_err;
    }
    LOGTRACEEX(TRUE, file, line, "%s[%p][%p] Memory Committed [%"PRId64"B]\n",
            name, *x, offset, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void
_mem_unmap(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (!*x) return;
    munmap(*x, size);
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unmapped [%"PRId64"B]\n",
            name, *x, size);
    *x = NULL;
}

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif /* _GNU_SOURCE */
