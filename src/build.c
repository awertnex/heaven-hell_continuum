#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#define DIR_BIN             "bin/"
#define DIR_BIN_TESTS       "bin/tests/"

#if defined __linux__
    #define ALLOC_CMD       (cmd = (char**) malloc(64 * sizeof(char*)))
    #define ZERO_CMD        (memset(cmd, 0, 64 * sizeof(char*)))
    #define FREE_CMD        free(cmd)
    #define NULL_CMD        (cmd = NULL)
    #define COMPILER        "cc"
    #define EXTENSION       ""
    #define MKDIR_BIN       mkdir("bin/", 0775);
    #define MKDIR_BIN_TESTS mkdir("bin/tests/", 0775);
char **cmd;
char str_libs[3][24] = {"-lraylib", "-lm", NULL};

#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__
    #define ALLOC_CMD
    #define ZERO_CMD
    #define FREE_CMD
    #define NULL_CMD
    #define COMPILER        "gcc"
    #define EXTENSION       ".exe"
    #define MKDIR_BIN       mkdir("bin/");
    #define MKDIR_BIN_TESTS mkdir("bin/tests/");
char *cmd[64];
char str_libs[5][24] = {"-lraylib", "-lgdi32", "-lwinmm", "-lm", NULL};
#endif // PLATFORM

char str_main[32] = "main.c";
char str_cflags[8][24] = {"-Wall", "-Wextra", "-ggdb", "-Wno-missing-braces", "-Wpedantic", "-std=c99", "-fno-builtin", NULL};
char str_out[3][48] = {"-o", "../bin/minecraft_c", NULL};
char str_tests[4][24] =
{
    "chunk_loader",
    "chunk_pointer_table",
    "function_pointer",
    NULL,
};
int str_tests_members = (int)(sizeof(str_tests) / sizeof(str_tests[0])) - 1;

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

int is_dir_exists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
        if (S_ISDIR(stats.st_mode))
            return 1;
    return 0;
}

void build_cmd()
{
    ALLOC_CMD;
    ZERO_CMD;
    for (int i = 2, j = 0, stage = 0, parse = 1; i < 64 && parse; ++i)
    {
        switch (stage)
        {
            case 0:
                cmd[0] = COMPILER;
                cmd[1] = str_main;
                ++stage;
                --i;
                break;

            case 1:
                if (!str_cflags[j][0])
                {
                    ++stage;
                    --i;
                    j = 0;
                    continue;
                }
                cmd[i] = str_cflags[j];
                ++j;
                break;

            case 2:
                if (!str_libs[j][0])
                {
                    ++stage;
                    --i;
                    j = 0;
                    continue;
                }
                cmd[i] = str_libs[j];
                ++j;
                break;

            case 3:
                if (!str_out[j][0])
                {
                    cmd[i] = NULL;
                    parse = 0;
                    break;
                }
                cmd[i] = str_out[j];
                ++j;
                break;
        }
    }
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

int main(int argc, char **argv)
{
    if (!is_dir_exists(DIR_BIN))
    {
        MKDIR_BIN;
        fprintf(stderr, "Directory 'bin/' Created!\n");
    }

    if (!is_dir_exists(DIR_BIN_TESTS))
    {
        MKDIR_BIN_TESTS;
        fprintf(stderr, "Directory 'bin/tests/' Created!\n");
    }

    if (argc == 1)
    {
        printf("Building minecraft.c..\n");
        build_cmd();
        execute_cmd();
        fprintf(stderr, "minecraft.c Built 'bin/minecraft_c%s'\n", EXTENSION);
        return 0;
    }

    if (!strncmp(argv[1], "launcher", 9))
    {
        printf("Building minecraft.c launcher..\n");
        snprintf(str_main, 32, "launcher/launcher.c");
        memset(str_libs[1], 0, sizeof(str_libs[1]));
        build_cmd();
        execute_cmd();
        fprintf(stderr, "minecraft.c Launcher Built 'bin/minecraft_c%s'\n", EXTENSION);
    }
    else if (!strncmp(argv[1], "list", 5))
    {
        printf("Builds: \n    launcher\nTests:\n");

        for (int i = 0; i < str_tests_members; ++i)
            printf("    %03d: %s\n", i, str_tests[i]);

    }
    else if (!strncmp(argv[1], "test", 5))
    {
        int test_index = atoi(argv[2]);

        if (!argv[2])
        {
            fprintf(stderr, "usage: ./build%s test [n]\n", EXTENSION);
            fail_cmd();
        }
        else if ((test_index < 0) || (test_index >= (str_tests_members)))
        {
            fprintf(stderr, "Error: '%s' Invalid, Try './build%s list' to List Available Options..\n", argv[2], EXTENSION);
            fail_cmd();
        }

        printf("Building test %03d '%s'..\n", test_index, str_tests[test_index]);
        snprintf(str_main, 32, "tests/%s.c", str_tests[test_index]);
        snprintf(str_out[1], 48, "../%stest_%s%s", DIR_BIN_TESTS, str_tests[test_index], EXTENSION);
        build_cmd();
        execute_cmd();
        fprintf(stderr, "test %03d Built 'bin/tests/test_%s%s'\n", test_index, str_tests[test_index], EXTENSION);
    }
    else if (!strncmp(argv[1], "help", 5))
    {
        fprintf(stderr, "usages:\n  %s\n  %s\n  %s\n  %s\n",
                "./build launcher   (build the launcher)",
                "./build list       (list all available options and tests)",
                "./build test [n]   (build test 'n' from the 'list' command)",
                "./build help       (print this list again)");
    }
    else
    {
        fprintf(stderr, "Invalid '%s', Try './build help' to List Available Options..\n", argv[1]);
        fail_cmd();
    }

    return 0;
}

