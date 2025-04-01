#include <stdio.h>
#include <unistd.h>

#define PANIC(str) fprintf(stderr, "%s\n", str)
char* args[] =
{
    /* CC */        "/usr/bin/cc",
    /* MAIN */      "main.c",
    /* CHILDREN */  "logger.c", "assets.c", "chunking.c", "dir.c", "gui.c", "keymaps.c", "logic.c", "super_debugger.c",
    /* CFLAGS */    "-Wall", "-Wextra", "-ggdb", "-Wpedantic",
    /* LIBS */      "-lraylib", "-lm",
    /* OUT */       "-o", "../minecraft",
    NULL,
};

const char* tests[] =
{
};

int main(int argc, char **argv)
{
    if (!chdir("src/"))
        execv("/usr/bin/cc", (char* const*)args);
    else PANIC("Error: 'src/' Directory Not Found");

    return 0;
}
