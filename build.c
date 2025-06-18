#include <sys/wait.h>
#include <unistd.h>
#include <glob.h>

#include "src/engine/h/platform.h"
#include "src/engine/memory.c"
#include "src/engine/dir.c"
#include "src/engine/logger.c"

#define DIR_ROOT        "Heaven-Hell Continuum/"
#define DIR_ROOT_TESTS  "tests/"
#define DIR_SRC         "src/"
#define DIR_LAUNCHER    DIR_SRC"launcher/"
#define DIR_TESTS       DIR_SRC"tests/"
#define CMD_MEMB        128

#if defined(__linux__) || defined(__linux)
#define PLATFORM        "linux/"
#define COMPILER        "gcc"
#define EXTENSION       ""
str str_libs[][32] =
{
    "-lm",
    //"-lpthread",
    "-I ./include/",
    "./lib/linux/libglfw.so",
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
#define PLATFORM        "win/"
#define COMPILER        "gcc"
#define EXTENSION       ".exe"
str str_libs[][24] =
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

    FLAG_INCLUDE_RAYLIB =   0x01,
    FLAG_SHOW_CMD =         0x02,
    FLAG_RAW_CMD =          0x04,
}; /* Flags */

/* ---- section: declarations ----------------------------------------------- */

u8 state = 0;
u16 flags = 0;
str_buf cmd = {NULL};
u64 cmd_pos = 0;
str_buf str_tests = {NULL};
glob_t glob_buf = {0};

str *cmd_cp_lib[] = {"cp", "-rv", "lib/", DIR_ROOT, NULL};
str *cmd_cp_shaders[] = {"cp", "-rv", "shaders/", DIR_ROOT, NULL};
str *cmd_cp_resources[] = {"cp", "-rv", "resources/", DIR_ROOT, NULL};

str str_main[NAME_MAX] = DIR_SRC"main.c";

str str_cflags[][32] =
{
    "-std=c99",
    "-ggdb",
    "-Wall",
    "-Wextra",
    "-Wno-missing-braces",
    "-Wpedantic",
    "-fno-builtin",
    "-Wl,-rpath,$ORIGIN/lib/"PLATFORM,
};

str str_out[NAME_MAX] = DIR_ROOT"hhc";

/* ---- section: signatures ------------------------------------------------- */

u64 compare_argv(str *arg, int argc, str **argv);
b8 evaluate_extension(const str *file_name);
void strip_extension(const str *file_name, str *dest);
void show_cmd();
void raw_cmd();
void push_cmd(str *string);
void build_cmd();
void exec(str **command, const str *command_name);
void fail_cmd();
void build_test(int argc, char ** argv);
void help();
void list();
/* 
 * scrap function, for reference;
 */
//void push_glob(const str *pattern);

/* ---- section: main ------------------------------------------------------- */

int main(int argc, char **argv)
{
    if (compare_argv("help", argc, argv))       help();
    if (compare_argv("list", argc, argv))       list();
    if (compare_argv("test", argc, argv))       state = STATE_TEST;
    if (compare_argv("launcher", argc, argv))   state = STATE_LAUNCHER;
    if (compare_argv("raylib", argc, argv))     flags |= FLAG_INCLUDE_RAYLIB;
    if (compare_argv("show", argc, argv))       flags |= FLAG_SHOW_CMD;
    if (compare_argv("raw", argc, argv))        flags |= FLAG_RAW_CMD;

    if (0
            || !is_dir_exists(DIR_SRC)
            || !is_dir_exists(DIR_TESTS)
            || !is_dir_exists(DIR_LAUNCHER)
       )
        return -1;

    build_cmd(argc, argv);
    if (flags & FLAG_SHOW_CMD)
        show_cmd();

    if (flags & FLAG_RAW_CMD)
        raw_cmd();

    if (!is_dir_exists(DIR_ROOT))
        make_dir(DIR_ROOT);

    if (state == STATE_TEST && !is_dir_exists(DIR_ROOT_TESTS))
        make_dir(DIR_ROOT_TESTS);

    exec(cmd_cp_lib, "cp lib/");
    exec(cmd_cp_shaders, "cp shaders/");
    exec(cmd_cp_resources, "cp resources/");
    exec(cmd.entry, "build");
    mem_free_str_buf((str_buf*)&cmd, NAME_MAX, "cmd");
    return 0;
}

/* ---- section: functions -------------------------------------------------- */
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

    if (!(flags & FLAG_RAW_CMD))
        putchar('\n');
}

void raw_cmd()
{
    printf("\nRAW:\n");
    for (u32 i = 0; i < CMD_MEMB; ++i)
    {
        if (!cmd.entry[i]) break;
        printf("%s ", cmd.entry[i]);
    }

    printf("%s", "\n\n");
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
            snprintf(str_out, NAME_MAX, "./%stest_%s%s", DIR_ROOT_TESTS, str_tests.entry[test_index], EXTENSION);
            push_cmd(str_main);
            break;

        case STATE_LAUNCHER:
            snprintf(str_main, NAME_MAX, "%slauncher.c", DIR_LAUNCHER);
            push_cmd(str_main);
            snprintf(str_out, NAME_MAX, "./%slauncher%s", DIR_ROOT, EXTENSION);
            break;

        default:
            push_cmd(str_main);
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

/*
 * scrap function, for reference;
 */
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

/*
 * command = command;
 * args = command arguments;
 * command_name = command name (for logging);
 */
void exec(str **command, const str *command_name)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        LOGERROR("'%s' Fork Failed, Process Aborted\n", command_name);
        goto cleanup;
    }
    else if (pid == 0)
    {
        execvp(command[0], (str *const *)command);
        LOGERROR("'%s' Failed, Process Aborted\n", command_name);
        goto cleanup;
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        LOGFATAL("'%s' Waitpid Failed, Process Aborted", command_name);
        goto cleanup;
    }

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
            LOGINFO("'%s' Success, Exit Code: %d\n", command_name, exit_code);
        else
        {
            LOGINFO("'%s' Exit Code: %d\n", command_name, exit_code);
            goto cleanup;
        }
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        LOGFATAL("'%s' Terminated by Signal: %d, Process Aborted\n", command_name, sig);
        goto cleanup;
    }
    else
    {
        LOGERROR("'%s' Exited Abnormally, Process Aborted\n", command_name);
        goto cleanup;
    }

    return;

cleanup:
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
            "    show       show build command",
            "    raw        show build command, raw");

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

