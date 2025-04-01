#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char** cmd = {0};

char str_exec[24] = "/usr/bin/cc";
char str_main[32] = "main.c";
char str_children[16][24] = {"logger.c", "assets.c", "chunking.c", "dir.c", "gui.c", "keymaps.c", "logic.c", "super_debugger.c"};
char str_cflags[4][24] = {"-Wall", "-Wextra", "-ggdb", "-Wpedantic"};
char str_libs[2][24] = {"-lraylib", "-lm"};
char str_out[3][32] = {"-o", "../minecraft"};
char str_tests[3][24] =
{
    "chunk_loader",
    "function_pointer",
};

void append_cmd();

int main(int argc, char **argv) // ---- main -----------------------------------
{
    cmd = (char**)calloc(64, sizeof(char*));
    if (argc > 1)
    {
        if (!strncmp(argv[1], "list", 4))
        {
            printf("List: \n  %s\n",
                    "launcher");
            printf("Tests: \n");
            for (int i = 0; i < 64 && str_tests[i][0]; ++i)
                printf("  test %03d: %s\n", i, str_tests[i]);
            free(cmd);
            return 0;
        }
        else if (!strncmp(argv[1], "launcher", 8))
        {
            printf("Building minecraft.c launcher..\n");

            snprintf(str_main, 32, "launcher/launcher.c");
            memset(str_children, 0, sizeof(str_children));
            memset(str_libs[1], 0, sizeof(str_libs[1]));
            snprintf(str_out[1], 32, "../launcher");
        }
        else if (!strncmp(argv[1], "test", 4))
        {
            if (!argv[2])
            {
                printf("usage: ./build test [n]\n");
                free(cmd);
                return 0;
            }

            if (((argv[2][0] - 48) < 0) || ((argv[2][0] - 48) > sizeof(str_tests)/sizeof(str_tests[0])))
            {
                printf("Invalid '%c', Try './build list' to List Available Options..\n", argv[2][0]);
                free(cmd);
                return 0;
            }

            printf("Building test %03d: %s..\n", (argv[2][0]) - 48, str_tests[(argv[2][0]) - 48]);
            snprintf(str_main, 32, "tests/%s.c", str_tests[(argv[2][0]) - 48]);
            memset(str_children, 0, sizeof(str_children));
            snprintf(str_out[1], 32, "../test_%s", str_tests[(argv[2][0]) - 48]);
        }
        else if (!strncmp(argv[1], "help", 4))
        {
            printf("usages:\n  %s\n  %s\n  %s\n  %s\n",
                    "./build list       (list all available options and tests)",
                    "./build test [n]   (build test 'n' from the 'list' command)",
                    "./build launcher   (build the launcher)",
                    "./build help       (print this list again)");
            free(cmd);
            return 0;
        }
        else
        {
            printf("Invalid '%s', Try './build help' to List Available Options..\n", argv[1]);
            free(cmd);
            return 0;
        }
    }
    else printf("Building main.c..\n");

    append_cmd();

    if (!chdir("src/"))
        execv("/usr/bin/cc", (char* const*)cmd);
    else printf("Error: 'src/' Directory Not Found");

    free(cmd);
    return 0;
}

void append_cmd()
{
    for (int i = 2, j = 0, stage = 0; i < 64; ++i)
    {
        switch (stage)
        {
            case 0:
                cmd[0] = str_exec;
                cmd[1] = str_main;
                ++stage;
                --i;
                break;

            case 1:
                if (!str_children[j][0])
                {
                    ++stage;
                    --i;
                    j = 0;
                    continue;
                }
                cmd[i] = str_children[j];
                ++j;
                break;

            case 2:
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

            case 4:
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
                    return;
                }
                cmd[i] = str_out[j];
                ++j;
                break;
        }
    }
}
