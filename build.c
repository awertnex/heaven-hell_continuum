#include <sys/wait.h>
#include <unistd.h>
#include <glob.h>

#include "src/engine/platform_linux.c"
#include "src/engine/platform_windows.c"
#include "src/engine/memory.c"
#include "src/engine/dir.c"
#include "src/engine/logger.c"

#define DIR_BIN         "bin/"
#define DIR_BIN_TESTS   "bin/tests/"
#define DIR_SRC         "src/"
#define DIR_ENGINE      "src/engine/"
#define DIR_LAUNCHER    "src/launcher/"
#define DIR_TESTS       "src/tests/"
#define CMD_MEMB        128

#if defined(__linux__) || defined(__linux)
#define COMPILER        "cc"
#define EXTENSION       ""
str str_libs[][24] =
{
    "-lm",
    //"-lpthread",
    "-lglfw",
    "-lGLEW",
    "-lGL",
    //"-lXrandr",
    //"-lXi",
    //"-lX11",
    //"-lXxf86vm",
    //"-ldl",
    //"-lXinerama",
    "-lfreetype",
};
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define COMPILER        "gcc"
#define EXTENSION       ".exe"
str str_libs[6][24] =
{
    "-lm",
    "-lglfw",
    "-lGLEW",
    "-lGL",
    "-lgdi32",
    "-lwinmm",
    "-lfreetype",
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

/* ---- section: declarations ----------------------------------------------- */

u8 state = 0;
u16 flags = 0;
str_buf cmd = {NULL};
u64 cmd_pos = 0;
str_buf str_tests = {NULL};
glob_t glob_buf = {0};
str str_main[NAME_MAX] = DIR_SRC"main.c";

str str_cflags[][28] =
{
    "-std=c99",
    "-ggdb",
    "-Wall",
    "-Wextra",
    "-Wno-missing-braces",
    "-Wpedantic",
    "-fno-builtin",
};

str str_out[NAME_MAX] = DIR_BIN"heaven-hell_continuum";

/* ---- section: signatures ------------------------------------------------- */

u64 compare_argv(str *arg, int argc, str **argv);
b8 evaluate_extension(const str *file_name);
void strip_extension(const str *file_name, str *dest);
void show_cmd();
void push_cmd(str *string);
void build_cmd();
void push_glob(const str *pattern);
void execute_cmd();
void fail_cmd();
void build_test(int argc, char ** argv);
void help();
void list();

/* ---- section: main ------------------------------------------------------- */

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
        make_dir(DIR_BIN);
        LOGINFO("Directory '%s' Created", DIR_BIN);
    }

    if (!is_dir_exists(DIR_BIN_TESTS))
    {
        make_dir(DIR_BIN_TESTS);
        LOGINFO("Directory '%s' Created", DIR_BIN_TESTS);
    }

    execute_cmd();
    mem_free_str_buf((str_buf*)&cmd, NAME_MAX, "cmd");
    globfree(&glob_buf);
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

void strip_extension(const str *file_name, str *dest) // TODO: make this function
{
    u64 len = strlen(file_name);
    str file_name_buf[NAME_MAX] = {0};
    if (len > 2 && strncmp(file_name + len - 2, ".c", 2) == 0)
    {
        snprintf(file_name_buf, len - 1, "%s", file_name);
        strncpy(dest, file_name_buf, NAME_MAX);
    }
}

void show_cmd()
{
    printf("\nCMD:\n");
    for (u32 i = 0; i < CMD_MEMB; ++i)
    {
        if (!cmd.entry[i]) break;
        printf("    %.3d: %s\n", i, cmd.entry[i]);
    }
}

void push_cmd(str *string)
{
    if (cmd_pos >= CMD_MEMB - 1)
        LOGERROR("%s\n", "cmd Full");

    strncpy(cmd.entry[cmd_pos], string, NAME_MAX);
    ++cmd_pos;
}

void build_cmd(int argc, char **argv)
{
    if (!mem_alloc_str_buf((str_buf*)&cmd, CMD_MEMB, NAME_MAX, "cmd"))
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
            str_tests.count = get_dir_entry_count(DIR_TESTS);
            if (test_index <= 0 || test_index >= str_tests.count)
            {
                LOGERROR("'%s' Invalid, Try './build%s list' to List Available Options..\n", argv[2], EXTENSION);
                fail_cmd();
            }

            if (!mem_alloc_str_buf((str_buf*)&str_tests, str_tests.count, NAME_MAX, "str_tests"))
                fail_cmd();


            sort_str_buf(&str_tests);
            snprintf(str_main, NAME_MAX, "%s%s.c", DIR_TESTS, str_tests.entry[test_index]);
            snprintf(str_out, NAME_MAX, "./%stest_%s%s", DIR_BIN_TESTS, str_tests.entry[test_index], EXTENSION);
            push_cmd(str_main);
            break;

        case STATE_LAUNCHER:
            snprintf(str_main, NAME_MAX, "%s%s", DIR_LAUNCHER, "launcher.c");
            push_cmd(str_main);
            push_glob(DIR_LAUNCHER"*.c");
            snprintf(str_out, NAME_MAX, "./%slauncher%s", DIR_BIN, EXTENSION);
            break;

        case STATE_ENGINE:
            push_cmd(str_main);
            push_glob(DIR_ENGINE"*.c");
            snprintf(str_out, NAME_MAX, "./%sengine%s", DIR_BIN, EXTENSION);
            break;

        default:
            push_cmd(str_main);
            push_glob(DIR_SRC"*.c");
            break;
    }

    /* ---- cflags ---------------------------------------------------------- */
    for (u32 i = 0; i < arr_len(str_cflags); ++i)
        push_cmd(str_cflags[i]);

    /* ---- libs ------------------------------------------------------------ */
    if (flags & FLAG_INCLUDE_RAYLIB)
        push_cmd("-L:./lib/libraylib.a");
    for (u32 i = 0; i < arr_len(str_libs); ++i)
        push_cmd(str_libs[i]);

    /* ---- out ------------------------------------------------------------- */
    push_cmd("-o");
    push_cmd(str_out);

    cmd.entry[cmd_pos] = NULL;
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
        if (strstr(glob_buf.gl_pathv[i], str_main))
            continue;
        strncpy(cmd.entry[cmd_pos], glob_buf.gl_pathv[i], NAME_MAX);
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
        execvp(COMPILER, cmd.entry);
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

cleanup:
    globfree(&glob_buf);
    fail_cmd();
}

void fail_cmd()
{
    mem_free_str_buf(&str_tests, NAME_MAX, "str_tests");
    mem_free_str_buf((str_buf*)&cmd, NAME_MAX, "cmd");
    exit(-1);
}

void help()
{
    LOGINFO("Usage: ./build [options]...\nOptions:\n%s\n%s\n%s\n%s\n",
            "    help       print this help",
            "    list       list all available options and tests",
            "    test [n]   build test 'n' from the 'list' command",
            "    show       show build command");

    exit(0);
}

void list()
{
    printf("Options:\n%s\n%s\n%s\n%s\nTests:\n",
            "    engine",
            "    launcher",
            "    raylib",
            "    test [n]");

    str_tests = get_dir_contents(DIR_TESTS);
    if (!str_tests.loaded)
    {
        mem_free_str_buf(&str_tests, NAME_MAX, "str_tests");
        return;
    }

    str *str_tests_temp = NULL;
    if (!mem_alloc((void*)&str_tests_temp, NAME_MAX, "str_tests_temp"))
        return;

    sort_str_buf(&str_tests);
    for (u32 i = 0; i < str_tests.count; ++i)
    {
        if (!evaluate_extension(str_tests.entry[i]))
            continue;

        strip_extension(str_tests.entry[i], str_tests_temp);
        printf("    %03d: %s\n", i, str_tests_temp);
    }

    mem_free_str_buf(&str_tests, NAME_MAX, "str_tests");
    mem_free((void*)&str_tests_temp, NAME_MAX, "str_tests_temp");
    exit(0);
}

