#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define DIR_BIN "bin"

#if defined __linux__
    #define ALLOC_CMD (cmd = (char**) malloc(64*sizeof(char*)))
    #define ZERO_CMD (memset(cmd, 0, 64*sizeof(char*)))
    #define FREE_CMD free(cmd)
    #define COMPILER "cc"
    #define EXTENSION ""
    #define MKDIR_BIN mkdir("bin/", 0775);
char** cmd;
char str_libs[3][24] = {"-lraylib", "-lm", 0};

#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__
    #define ALLOC_CMD
    #define ZERO_CMD
    #define FREE_CMD
    #define COMPILER "gcc"
    #define EXTENSION ".exe"
    #define MKDIR_BIN mkdir("bin");
char* cmd[64];
char str_libs[5][24] = {"-lraylib", "-lgdi32", "-lwinmm", "-lm", 0};
#endif // PLATFORM

char str_main[32] = "main.c";
char str_cflags[8][24] = {"-Wall", "-Wextra", "-ggdb", "-Wno-missing-braces", "-Wpedantic", "-std=c99", "-fno-builtin", 0};
char str_out[3][32] = {"-o", "../bin/minecraft_c", 0};
char str_tests[4][24] =
{
    "chunk_loader",
    "chunk_pointer_table",
    "function_pointer",
    0,
};

int is_dir_exists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
        if (S_ISDIR(stats.st_mode))
            return 1;
    return 0;
}

int build_cmd()
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

    if (!chdir("src/"))
        execvp(COMPILER, cmd);
    else
    {
        printf("Error: 'src/' Directory Not Found, Aborting Build\n");
        return 1;
    }

    FREE_CMD;
    return 0;
}

int main(int argc, char **argv)
{
    perror("execvp");
    if (!is_dir_exists(DIR_BIN))
    {
        MKDIR_BIN;
        printf("Directory 'bin/' Created!\n");
    }


    if (argc == 1)
    {
        printf("Building minecraft.c..\n");
        if (build_cmd()) return -1;
        return 0;
    }

    if (!strncmp(argv[1], "list", 4))
    {
        printf("List: \n  %s\n",
                "launcher");
        printf("Tests: \n");
        for (int i = 0; i < 64 && str_tests[i][0]; ++i)
            printf("  test %03d: %s\n", i, str_tests[i]);
    }
    else if (!strncmp(argv[1], "launcher", 8))
    {
        printf("Building minecraft.c launcher..\n");
        snprintf(str_main, 32, "launcher/launcher.c");
        memset(str_libs[1], 0, sizeof(str_libs[1]));
        snprintf(str_out[1], 32, "../bin/launcher%s", EXTENSION);
        if(build_cmd()) return -1;
    }
    else if (!strncmp(argv[1], "test", 4))
    {
        if (!argv[2])
        {
            printf("usage: ./build test [n]\n");
            return -1;
        }
        else if ((atoi(argv[2]) < 0) || (atoi(argv[2]) >= (sizeof(str_tests)/sizeof(str_tests[0])) - 1))
        {
            printf("'%c' Invalid, Try './build list' to List Available Options..\n", argv[2][0]);
            return -1;
        }

        printf("Building test %03d '%s'..\n", (argv[2][0]) - 48, str_tests[argv[2][0] - 48]);
        snprintf(str_main, 32, "tests/%s.c", str_tests[argv[2][0] - 48]);
        snprintf(str_out[1], 32, "../test_%s%s", str_tests[argv[2][0] - 48], EXTENSION);
        if (build_cmd()) return -1;
    }
    else if (!strncmp(argv[1], "--help", 6))
    {
        printf("usages:\n  %s\n  %s\n  %s\n  %s\n",
                "./build launcher   (build the launcher)",
                "./build list       (list all available options and tests)",
                "./build test [n]   (build test 'n' from the 'list' command)",
                "./build help       (print this list again)");
    }
    else
    {
        printf("Invalid '%s', Try './build help' to List Available Options..\n", argv[1]);
        return -1;
    }

    return 0;
}
