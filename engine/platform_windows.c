#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <windows.h>

#include "h/platform.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"

int
make_dir(const str *path)
{
    int exit_code = mkdir(path);
    if (exit_code == 0)
        LOGINFO("Directory Created '%s'\n", path);
    return exit_code;
}

b8
_get_path_absolute(const str *path, str *path_real)
{
    if (!GetFullPathNameA(path, PATH_MAX, path_real, NULL))
        return FALSE;
    return TRUE;
}

#include <stdio.h>
b8
_get_path_bin_root(str *path)
{
    str temp[PATH_MAX] = {0};
    u64 cursor = 0;
    if (!GetModuleFileNameA(NULL, temp, PATH_MAX))
    {
        LOGFATAL("%s\n", "'get_path_bin_root()' Failed, Process Aborted");
        return FALSE;
    }
    retract_path(temp);
    while (temp[cursor] != '\0' && temp[cursor] != '\\' && cursor < PATH_MAX)
        ++cursor;
    if (cursor + 1 >= PATH_MAX)
        return FALSE;
    strncpy(path, temp, cursor);
    strncat(path, "\\", PATH_MAX - cursor);
    strncat(path, temp + cursor, PATH_MAX - cursor);
    printf("OPEN. CURSOR. %s\n", path);
    return TRUE;
}

b8
exec(buf *cmd, str *cmd_name)
{
    str *cmd_cat = NULL;
    STARTUPINFOA        startup_info = {0};
    PROCESS_INFORMATION process_info = {0};

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!cmd->loaded || !cmd->buf)
    {
        LOGERROR("exec '%s' Failed, cmd Empty\n", cmd_name);
        return FALSE;
    }

    mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb, cmd_name);
    for (u64 i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(NULL, cmd_cat, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup_info, &process_info))
    {
        LOGFATAL("'%s' Fork Failed, Process Aborted\n", cmd_name);
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeProcess(process_info.hProcess, &exit_code);

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
