#include <sys/wait.h>
#include <unistd.h>
#include <glob.h>

#include "src/engine/h/memory.h"
#include "src/engine/dir.c"
#include "src/engine/logger.c"

#define DIR_BIN             "bin/"
#define DIR_BIN_TESTS       "bin/tests/"
#define DIR_SRC             "src/"
#define DIR_ENGINE          "src/engine/"
#define DIR_LAUNCHER        "src/launcher/"
#define DIR_TESTS           "src/tests/"
#define CMD_MEMB            512

#if defined __linux__
    #define ALLOC_CMD       mem_alloc_memb((void*)&cmd, CMD_MEMB, NAME_MAX, "cmd")
    #define FREE_CMD        mem_free((void*)&cmd, CMD_MEMB * NAME_MAX, "cmd")
    #define COMPILER        "cc"
    #define EXTENSION       ""
    #define MKDIR(dir)      mkdir(dir, 0775);
str **cmd;
glob_t glob_buf = {0};
str str_libs[11][24] =
{
    "-lglfw",
    "-lGLEW",
    "-lGL",
    "-lXrandr",
    "-lXi",
    "-lX11",
    "-lXxf86vm",
    "-lpthread",
    "-ldl",
    "-lXinerama",
    "-lm",
};
#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__
    #define ALLOC_CMD
    #define FREE_CMD
    #define COMPILER        "gcc"
    #define EXTENSION       ".exe"
    #define MKDIR(dir)      mkdir(dir);
str *cmd[CMD_MEMB];
glob_t glob_buf = {0};
str str_libs[6][24] =
{
    "-lglfw",
    "-lGLEW",
    "-lGL",
    "-lgdi32",
    "-lwinmm",
    "-lm",
};
#endif /* PLATFORM */

enum Flags
{
    STATE_TEST = 1,
    STATE_LAUNCHER,
    STATE_ENGINE,

    FLAG_INCLUDE_RAYLIB =   0x01,
    FLAG_SHOW_CMD =         0x02,
}; /* Flags */

/* ---- declarations -------------------------------------------------------- */
u8 state = 0;
u16 flags = 0;
u64 cmd_pos = 0;
str str_main[48] = "main.c";
str str_cflags[7][24] =
{
    "-std=c99",
    "-ggdb",
    "-Wall",
    "-Wextra",
    "-Wno-missing-braces",
    "-Wpedantic",
    "-fno-builtin",
};
str str_out[48] = DIR_BIN"minecraft_c";
str str_tests[5][24] =
{
    "chunk_loader",
    "chunk_pointer_table",
    "chunk_tab_shift",
    "function_pointer",
    "renderer",
};

/* ---- signatures ---------------------------------------------------------- */
u64 compare_argv(str *arg, int argc, str **argv);
b8 evaluate_extension(const str *file_name);
void strip_extension(const str *file_name);
void show_cmd();
void push_cmd(str *str);
void build_cmd();
void push_glob(const str *pattern);
void execute_cmd();
void fail_cmd();
void build_test(int argc, char ** argv);
void help();
void list();

int main(int argc, char **argv)
{
    if (compare_argv("help", argc, argv))       help();
    if (compare_argv("list", argc, argv))       list();
    if (compare_argv("test", argc, argv))       state = STATE_TEST;
    if (compare_argv("launcher", argc, argv))   state = STATE_LAUNCHER;
    if (compare_argv("engine", argc, argv))     state = STATE_ENGINE;
    if (compare_argv("raylib", argc, argv))     flags |= FLAG_INCLUDE_RAYLIB;
    if (compare_argv("show", argc, argv))       flags |= FLAG_SHOW_CMD;

    if (0
            || !is_dir_exists(DIR_SRC)
            || !is_dir_exists(DIR_ENGINE)
            || !is_dir_exists(DIR_TESTS)
            || !is_dir_exists(DIR_LAUNCHER)
       )
        return -1;

    build_cmd(argc, argv);
    if (flags & FLAG_SHOW_CMD)
        show_cmd();

    if (!is_dir_exists(DIR_BIN))
    {
        MKDIR(DIR_BIN);
        LOGINFO("Directory '%s' Created", DIR_BIN);
    }

    if (!is_dir_exists(DIR_BIN_TESTS))
    {
        MKDIR(DIR_BIN_TESTS);
        LOGINFO("Directory '%s' Created", DIR_BIN_TESTS);
    }

    execute_cmd();
    return 0;
}

/* ---- functions ----------------------------------------------------------- */
u64 compare_argv(str *arg, int argc, char **argv)
{
    if (argc == 1)
        return 0;

    for (u64 i = 1; i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

b8 evaluate_extension(const str *file_name)
{
    u64 len = strlen(file_name);
    if (len > 2 && strncmp(file_name + len - 2, ".c", 2) == 0)
        return TRUE;
    return FALSE;
}

void strip_extension(const str *file_name)
{
    u64 len = strlen(file_name);
    str file_name_stripped[NAME_MAX] = {0};
    if (len > 2 && strncmp(file_name + len - 2, ".c", 2) == 0)
    {
        snprintf(file_name_stripped, len - 2, "%s", file_name);
        file_name = file_name_stripped;
    }
}

void show_cmd()
{
    printf("\nCMD:\n");
    for (u32 i = 0; i < CMD_MEMB; ++i)
    {
        if (!cmd[i]) break;
        printf("    %.3d: %s\n", i, cmd[i]);
    }
    putchar('\n');
}

void push_cmd(str *str)
{
    if (cmd_pos >= CMD_MEMB - 1)
    {
        LOGERROR("%s\n", "cmd Full, Process Aborted");
        show_cmd();
        fail_cmd();
    }

    cmd[cmd_pos] = str;
    cmd[cmd_pos + 1] = NULL;
    ++cmd_pos;
}

void build_cmd(int argc, char **argv)
{
    if (!ALLOC_CMD)
        fail_cmd();

    push_cmd(COMPILER);
    switch (state)
    {
        case STATE_TEST:
            if (!argv[2])
            {
                LOGERROR("Usage: ./build%s test [n]\n", EXTENSION);
                fail_cmd();
            }

            u32 test_index = atoi(argv[2]);
            if ((test_index < 0) || (test_index >= arr_len(str_tests)))
            {
                LOGERROR("'%s' Invalid, Try './build%s list' to List Available Options..\n", argv[2], EXTENSION);
                fail_cmd();
            }

            snprintf(str_main, 48, "%s%s.c", DIR_TESTS, str_tests[test_index]);
            snprintf(str_out, 48, "./%stest_%s%s", DIR_BIN_TESTS, str_tests[test_index], EXTENSION);
            push_cmd(str_main);
            break;

        case STATE_LAUNCHER:
            push_glob(DIR_LAUNCHER"*.c");
            snprintf(str_out, 48, "./%slauncher%s", DIR_BIN, EXTENSION);
            break;

        case STATE_ENGINE:
            push_glob(DIR_ENGINE"*.c");
            snprintf(str_out, 48, "./%sengine%s", DIR_BIN, EXTENSION);
            break;

        default:
            push_glob(DIR_SRC"*.c");
            break;
    }

    /* ---- cflags ---------------------------------------------------------- */
    for (u32 i = 0; i < arr_len(str_cflags); ++i)
        push_cmd(str_cflags[i]);

    /* ---- libs ------------------------------------------------------------ */
    if (flags & FLAG_INCLUDE_RAYLIB)
        push_cmd("-lraylib");
    for (u32 i = 0; i < arr_len(str_libs); ++i)
        push_cmd(str_libs[i]);

    /* ---- out ------------------------------------------------------------- */
    push_cmd("-o");
    push_cmd(str_out);
}

void push_glob(const str *pattern)
{
    b8 ret = glob(pattern, 0, NULL, &glob_buf);
    if (ret != 0)
    {
        LOGFATAL("Glob() Failed, Pattern: %s, Process Aborted\n", pattern);
        globfree(&glob_buf);
        fail_cmd();
    }

    for (u64 i = 0; i < glob_buf.gl_pathc; ++i)
    {
        cmd[cmd_pos] = glob_buf.gl_pathv[i];
        ++cmd_pos;
    }
}

void execute_cmd()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        LOGERROR("%s\n", "Fork Failed, Process Aborted");
        goto cleanup;
    }
    else if (pid == 0)
    {
        execvp(COMPILER, cmd);
        LOGERROR("%s\n", "Build Failed, Process Aborted");
        goto cleanup;
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
    {
        if (status == 0)
            LOGINFO("'%s' Built\n", str_out);
        LOGINFO("Build Exit Code: %d\n", WEXITSTATUS(status));

        if (WEXITSTATUS(status))
            goto cleanup;
    }
    else
    {
        LOGERROR("%s\n", "Build Exited Abnormally, Process Aborted");
        goto cleanup;
    }

    globfree(&glob_buf);
    exit(0);

cleanup:
    globfree(&glob_buf);
    fail_cmd();
}

void fail_cmd()
{
    FREE_CMD;
    exit(-1);
}

void build_test(int argc, char ** argv)
{
}

void help()
{
    LOGINFO("Usage: ./build [options]...\nOptions:\n%s\n%s\n%s\n%s\n%s\n%s\n",
            "    help       print this help",
            "    list       list all available options and tests",
            "    test [n]   build test 'n' from the 'list' command",
            "    show       show build command"
           );

    exit(0);
}

void list()
{
    printf("Options:\n%s\n%s\n%s\n%s\n%s\n%s\nTests:\n",
            "    all",
            "    engine",
            "    main",
            "    launcher",
            "    raylib",
            "    test"
          );

    for (u32 i = 0; i < arr_len(str_tests); ++i)
        printf("    %03d: %s\n", i, str_tests[i]);
    exit(0);
}

