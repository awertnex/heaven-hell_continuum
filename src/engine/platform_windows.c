#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <windows.h>

#include "h/platform.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"

int make_dir(const str *path)
{
    int exit_code = mkdir(path);
    if (exit_code == 0)
        LOGINFO("Directory Created '%s'\n", path);

    return exit_code;
}

b8 _get_path_absolute(const str *path, str *path_real)
{
    if (!GetFullPathNameA(path, PATH_MAX, path_real, NULL))
        return FALSE;
    return TRUE;
}

b8 _get_path_bin_root(str *path)
{
    if (!GetModuleFileNameA(NULL, path, PATH_MAX - 1))
    {
        LOGFATAL("%s\n", "'GetModuleFileNameA()' Failed, Process Aborted");
        return FALSE;
    }

    retract_path(path);

    return TRUE;
}

b8 exec(buf *cmd, str *cmd_name)
{
    str *cmd_cat = NULL;
    STARTUPINFOA        startup_info;
    PROCESS_INFORMATION process_info;

    if (!cmd->loaded || !cmd->buf)
    {
        LOGERROR("exec '%s' Failed, cmd Empty\n", cmd_name);
        return FALSE;
    }

    mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb, cmd_name);
    for (u64 i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(cmd_cat, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info))
    {
        LOGFATAL("'%s' Fork Failed, Process Aborted\n", cmd_name);
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    LPDWORD exit_code = 0;
    GetExitCodeProcess(process_info.hProcess, exit_code);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (exit_code == 0)
        LOGINFO("'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
    else
    {
        LOGINFO("'%s' Exit Code: %d\n", cmd_name, exit_code);
        goto cleanup;
    }

    mem_free((void*)&cmd_cat, cmd->memb * cmd->size, cmd_name);
    return TRUE;

cleanup:
    mem_free((void*)&cmd_cat, cmd->memb * cmd->size, cmd_name);
    return FALSE;
}

