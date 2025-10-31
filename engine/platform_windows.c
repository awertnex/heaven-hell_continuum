#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <windows.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/platform.h"

int
make_dir(const str *path)
{
    int exit_code = mkdir(path);
    if (exit_code == 0)
        LOGINFO(FALSE, "Directory Created '%s'\n", path);
    return exit_code;
}

u32
_get_path_absolute(const str *path, str *path_real)
{
    if (!GetFullPathNameA(path, PATH_MAX, path_real, NULL))
    {
        engine_err = ERR_GET_PATH_ABSOLUTE_FAIL;
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

#include <stdio.h>
u32
_get_path_bin_root(str *path)
{
    if (strlen(_pgmptr) + 1 >= STRING_MAX)
    {
        LOGFATAL(FALSE, ERR_GET_PATH_BIN_ROOT_FAIL,
                "%s\n", "'get_path_bin_root()' Failed, Process Aborted");
        return FALSE;
    }
    str temp[STRING_MAX] = {0};
    u64 cursor = 0;
    strncpy(temp, _pgmptr, STRING_MAX);
    retract_path(temp);
    while (temp[cursor] != '\0' &&
            temp[cursor] != '\\' &&
            cursor < STRING_MAX)
        ++cursor;
    if (cursor + 1 >= STRING_MAX)
        return FALSE;
    strncpy(path, temp, cursor);
    strncat(path, "\\", STRING_MAX - cursor);
    strncat(path, temp + cursor, STRING_MAX - cursor);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32
exec(buf *cmd, str *cmd_name)
{
    u32 i = 0;
    str *cmd_cat = NULL;
    STARTUPINFOA        startup_info = {0};
    PROCESS_INFORMATION process_info = {0};

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!cmd->loaded || !cmd->buf)
    {
        LOGERROR(TRUE, ERR_BUFFER_EMPTY,
                "exec '%s' Failed, cmd Empty\n", cmd_name);
        return engine_err;
    }

    if (mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb,
            stringf("exec().%s", cmd_name)) != ERR_SUCCESS)
        return engine_err;

    for (i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(NULL, cmd_cat, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup_info, &process_info))
    {
        LOGFATAL(TRUE, ERR_EXEC_FAIL,
                "'%s' Fork Failed, Process Aborted\n", cmd_name);
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeProcess(process_info.hProcess, &exit_code);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (exit_code == 0)
        LOGINFO(FALSE, "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
    else
    {
        engine_err = ERR_EXEC_PROCESS_NON_ZERO;
        LOGINFO(TRUE, "'%s' Exit Code: %d\n", cmd_name, exit_code);
        goto cleanup;
    }

    mem_free((void*)&cmd_cat, cmd->memb * cmd->size,
            stringf("exec().%s", cmd_name));
    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:
    mem_free((void*)&cmd_cat, cmd->memb * cmd->size,
            stringf("exec().%s", cmd_name));
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

    *x = VirtualAlloc(NULL, size,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!*x)
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

    VirtualAlloc(*(u8*)x + (u8*)offset, size, MEM_COMMIT, PAGE_READWRITE);
    if (!*(u8*)x + (u8*)offset)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_COMMIT_FAIL,
                "%s[%p][%p] Memory Commit [%"PRId64"B] Failed, Process Aborted\n",
                name, *x, offset, size);
        return engine_err;
    }
    LOGTRACEEX(TRUE, file, line, "%s[%p][%p] Memory Committed [%"PRId64"B]\n",
            name, x, offset, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void
_mem_unmap(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (!*x) return;
    VirtualFree(x, 0, MEM_RELEASE);
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unmapped [%"PRId64"B]\n",
            name, *x, size);
    *x = NULL;
}
