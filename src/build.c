#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#define DIR_BIN             "bin/"
#define DIR_BIN_TESTS       "bin/tests/"
#define CMD_LEN             128
#define arr_len(arr)        (sizeof(arr) / sizeof(arr[0]))

#if defined __linux__
    #define ALLOC_CMD       (cmd = (char**) malloc(CMD_LEN * sizeof(char*)))
    #define ZERO_CMD        (memset(cmd, 0, CMD_LEN * sizeof(char*)))
    #define FREE_CMD        free(cmd)
    #define NULL_CMD        (cmd = NULL)
    #define COMPILER        "cc"
    #define EXTENSION       ""
    #define MKDIR(dir)      mkdir(dir, 0775);
char **cmd;
char str_libs[11][24] =
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
    #define ZERO_CMD
    #define FREE_CMD
    #define NULL_CMD
    #define COMPILER        "gcc"
    #define EXTENSION       ".exe"
    #define MKDIR(dir)      mkdir(dir);
char *cmd[CMD_LEN];
char str_libs[6][24] =
{
    "-lglfw",
    "-lGLEW",
    "-lGL",
    "-lgdi32",
    "-lwinmm",
    "-lm",
};
#endif /* PLATFORM */

/* ---- declarations -------------------------------------------------------- */
int cmd_pos = 0;
char str_main[32] = "main.c";
char str_cflags[7][24] =
{
    "-std=c99",
    "-ggdb",
    "-Wall",
    "-Wextra",
    "-Wno-missing-braces",
    "-Wpedantic",
    "-fno-builtin",
};
char str_out[2][48] =
{
    "-o",
    "../bin/minecraft_c",
};
char str_tests[5][24] =
{
    "chunk_loader",
    "chunk_pointer_table",
    "chunk_tab_shift",
    "function_pointer",
    "renderer",
};

/* ---- signatures ---------------------------------------------------------- */
int compare_argv(char *arg, int argc, char **argv);
int push_argv(int argc, char **argv);
int is_dir_exists(const char *path);
void fail_cmd();
void show_cmd();
void push_cmd(char *str);
void build_cmd(int argc, char **argv);
void execute_cmd();

int main(int argc, char **argv)
{
    if (!is_dir_exists(DIR_BIN))
    {
        MKDIR(DIR_BIN);
        fprintf(stderr, "Directory 'bin/' Created!\n");
    }

    if (!is_dir_exists(DIR_BIN_TESTS))
    {
        MKDIR(DIR_BIN_TESTS);
        fprintf(stderr, "Directory 'bin/tests/' Created!\n");
    }

    if (argc == 1)
    {
build_main:
        printf("Building minecraft.c..\n");
        build_cmd(argc, argv);
        push_argv(argc, argv);
        show_cmd(argc, argv);
        execute_cmd();
        fprintf(stderr, "minecraft.c Built 'bin/minecraft_c%s'\n", EXTENSION);
        return 0;
    }

    if (compare_argv("-v", argc, argv)
             || compare_argv("--verbose", argc, argv)
             || compare_argv("-a", argc, argv))
        goto build_main;


    if (!strncmp(argv[1], "launcher", 9))
    {
        printf("Building minecraft.c launcher..\n");
        snprintf(str_main, 32, "launcher/launcher.c");
        memset(str_libs[1], 0, sizeof(str_libs[1]));
        build_cmd(argc, argv);
        push_argv(argc, argv);
        show_cmd(argc, argv);
        execute_cmd();
        fprintf(stderr, "minecraft.c Launcher Built 'bin/minecraft_c%s'\n", EXTENSION);
        return 0;
    }
    else if (!strncmp(argv[1], "-l", 3)
            || !strncmp(argv[1], "--list", 7))
    {
        printf("Builds: \n    launcher\nTests:\n");
        for (int i = 0; i < arr_len(str_tests); ++i)
            printf("    %03d: %s\n", i, str_tests[i]);
        return 0;
    }
    else if (!strncmp(argv[1], "test", 5))
    {
        int test_index = atoi(argv[2]);

        if (!argv[2])
        {
            fprintf(stderr, "usage: ./build%s test [n]\n", EXTENSION);
            fail_cmd();
        }
        else if ((test_index < 0) || (test_index >= arr_len(str_tests)))
        {
            fprintf(stderr, "Error: '%s' Invalid, Try './build%s list' to List Available Options..\n", argv[2], EXTENSION);
            fail_cmd();
        }

        printf("Building test %03d '%s'..\n", test_index, str_tests[test_index]);
        snprintf(str_main, 32, "tests/%s.c", str_tests[test_index]);
        snprintf(str_out[1], 48, "../%stest_%s%s", DIR_BIN_TESTS, str_tests[test_index], EXTENSION);
        build_cmd(argc, argv);
        push_argv(argc, argv);
        show_cmd(argc, argv);
        execute_cmd();
        fprintf(stderr, "test %03d Built 'bin/tests/test_%s%s'\n", test_index, str_tests[test_index], EXTENSION);
        return 0;
    }
    else if (!strncmp(argv[1], "-h", 3)
            || !strncmp(argv[1], "--help", 7))
    {
        fprintf(stderr, "Usage: ./build [options]...\nOptions:\n%s\n%s\n%s\n%s\n%s\n%s\n",
                "    -h, --help     print this help",
                "    launcher       build the launcher",
                "    -l, --list     list all available options and tests",
                "    -v, --verbose  show build command",
                "    -a <arg>       append <arg> to build command",
                "    test [n]       build test 'n' from the 'list' command"
               );
        return 0;
    }

    fprintf(stderr, "Invalid '%s', Try './build help' to List Available Options..\n", argv[1]);
    fail_cmd();
    return -1;
}

/* ---- functions ----------------------------------------------------------- */
int compare_argv(char *arg, int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

int push_argv(int argc, char **argv)
{
    if (argc == 1) return 0;

    int argc_add = 0;
    if ((argc_add = compare_argv("-a", argc, argv)))
    {
        if (argc_add == argc - 1)
            fprintf(stderr, "%s", "ERROR: '-a', Argument Empty\n");
        else
        {
            push_cmd(argv[argc_add + 1]);
            return 1;
        }
    }
    return 0;
}

int is_dir_exists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
        if (S_ISDIR(stats.st_mode))
            return 1;
    return 0;
}

void fail_cmd()
{
    if (cmd)
    {
        ZERO_CMD;
        FREE_CMD;
        NULL_CMD;
    }
    exit(-1);
}

void show_cmd(int argc, char **argv)
{
    if (compare_argv("-v", argc, argv)
            || compare_argv("--verbose", argc, argv))
    {
        printf("\nCMD:\n");
        for (int i = 0; i < CMD_LEN; ++i)
        {
            if (!cmd[i]) break;
            printf("    %.3d: %s\n", i, cmd[i]);
        }
        putchar('\n');
    }
}

void push_cmd(char *str)
{
    if (cmd_pos == CMD_LEN)
    {
        fprintf(stderr, "Error: cmd Full, Process Aborted\n");
        fail_cmd();
    }

    cmd[cmd_pos] = str;
    ++cmd_pos;
}

void build_cmd(int argc, char **argv)
{
    ALLOC_CMD;
    ZERO_CMD;

    /* ---- main ------------------------------------------------------------ */
    push_cmd(COMPILER);
    push_cmd(str_main);

    /* ---- cflags ---------------------------------------------------------- */
    for (int i = 0; i < arr_len(str_cflags); ++i)
        push_cmd(str_cflags[i]);

    /* ---- libs ------------------------------------------------------------ */
    for (int i = 0; i < arr_len(str_libs); ++i)
        push_cmd(str_libs[i]);

    /* ---- out ------------------------------------------------------------- */
    for (int i = 0; i < arr_len(str_out); ++i)
        push_cmd(str_out[i]);
}

void execute_cmd()
{
    if (chdir("src/"))
    {
        fprintf(stderr, "Error: Directory 'src/' Not Found, Process Aborted\n");
        fail_cmd();
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Error: Fork Failed, Process Aborted\n");
        fail_cmd();
    }
    else if (pid == 0)
    {
        execvp(COMPILER, cmd);
        fprintf(stderr, "Error: Build Failed, Process Aborted\n");
        fail_cmd();
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            fprintf(stderr, "Build Exit Code: %d\n", WEXITSTATUS(status));

            if (WEXITSTATUS(status))
                fail_cmd();
        } else
            fprintf(stderr, "Error: Build Exited Abnormally, Process Aborted\n");
    }
}

