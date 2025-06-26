#include <unistd.h>

#include "src/engine/h/platform.h"
#include "src/engine/memory.c"
#include "src/engine/dir.c"
#include "src/engine/logger.c"

#define DIR_ROOT        "Heaven-Hell Continuum/"
#define DIR_ROOT_TESTS  "tests/"
#define DIR_SRC         "src/"
#define DIR_LAUNCHER    DIR_SRC"launcher/"
#define DIR_TESTS       DIR_SRC"tests/"
#define CMD_MEMB        64

/* ---- section: c_standard ------------------------------------------------- */

#ifndef __STDC_VERSION__
#define __STDC_VERSION__ 0L
#endif

#define STD_C99 199901L

const long C_STD = __STDC_VERSION__;

/* ---- section: platform --------------------------------------------------- */

#if defined(__linux__) || defined(__linux)
#include <glob.h>

#define PLATFORM        "linux/"
#define EXTENSION       ""
#define COMPILER        "gcc"EXTENSION

glob_t glob_buf = {0};

str str_libs[][32] =
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
};
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define PLATFORM        "win/"
#define EXTENSION       ".exe"
#define COMPILER        "gcc"EXTENSION

str str_libs[][24] =
{
    "-lm",
    "-lglfw",
    "-lGLEW",
    "-lGL",
    "-lgdi32",
    "-lwinmm",
};
#endif /* PLATFORM */

enum Flags
{
    STATE_TEST = 1,
    STATE_LAUNCHER,

    FLAG_SHOW_CMD =         0x01,
    FLAG_RAW_CMD =          0x02,
}; /* Flags */

/* ---- section: declarations ----------------------------------------------- */

u8 state = 0;
u16 flags = 0;
str *str_bin_root = NULL;
str str_src[PATH_MAX] = {0};            /* path: ./build.c */
str str_bin[PATH_MAX] = {0};            /* path: ./build%s, EXTENSION */
str str_bin_new[PATH_MAX] = {0};        /* path: ./build_new%s, EXTENSION */
str cmd_self_rebuild[PATH_MAX] = {0};   /* self-rebuild command to execute */
str_buf cmd = {NULL};
u64 cmd_pos = 0;
str_buf str_tests = {NULL};

str str_main[PATH_MAX] = DIR_SRC"main.c";

str str_cflags[][32] =
{
    "-std=c99",
    "-ggdb",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-fno-builtin",
    "-Wl,-rpath=$ORIGIN/lib/"PLATFORM,
};

str str_out[PATH_MAX] = DIR_ROOT"hhc";

/* ---- section: signatures ------------------------------------------------- */

void init_build(void);
b8 is_source_changed(void);
void rebuild_self(int argc, char **argv);
u64 compare_argv(str *arg, int argc, str **argv);
b8 evaluate_extension(const str *file_name);
void strip_extension(const str *file_name, str *dest);
void show_cmd(void);
void raw_cmd(void);
void push_cmd(const str *string);
void build_cmd(int argc, char **argv);
void clean_cmd(void);
void fail_cmd(void);
void help(void);
void list(void);
/* 
 * scrap function, for reference;
 */
//void push_glob(const str *pattern);

/* ---- section: main ------------------------------------------------------- */

int main(int argc, char **argv)
{
    init_build();

    if (C_STD != STD_C99)
    {
        LOGINFO("Rebuilding 'build%s' With -std=c99\n", EXTENSION);
        rebuild_self(argc, argv);
    }

    if (is_source_changed())
        rebuild_self(argc, argv);

    if (compare_argv("help", argc, argv))       help();
    if (compare_argv("list", argc, argv))       list();
    if (compare_argv("test", argc, argv))       state = STATE_TEST;
    if (compare_argv("launcher", argc, argv))   state = STATE_LAUNCHER;
    if (compare_argv("show", argc, argv))       flags |= FLAG_SHOW_CMD;
    if (compare_argv("raw", argc, argv))        flags |= FLAG_RAW_CMD;

    if (!is_dir_exists(DIR_SRC) ||
            !is_dir_exists(DIR_TESTS) ||
            !is_dir_exists(DIR_LAUNCHER))
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

    if (!exec((str *const[]){"cp", "-v", "LICENSE", DIR_ROOT, NULL}, "cp lib/") ||
            !exec((str *const[]){"cp", "-rv", "lib/", DIR_ROOT, NULL}, "cp lib/") ||
            !exec((str *const[]){"cp", "-rv", "resources/", DIR_ROOT, NULL}, "cp resources/") ||
            !exec((str *const[]){"cp", "-rv", "shaders/", DIR_ROOT, NULL}, "cp shaders/") ||
            !exec(cmd.entry, "build"))
        fail_cmd();

    clean_cmd();
    return 0;
}

/* ---- section: functions -------------------------------------------------- */

void init_build(void)
{
    str_bin_root = get_path_bin_root();
    if (str_bin_root == NULL)
        fail_cmd();

    snprintf(str_src, PATH_MAX, "%sbuild.c", str_bin_root);
    snprintf(str_bin, PATH_MAX, "%sbuild%s", str_bin_root, EXTENSION);
    snprintf(str_bin_new, PATH_MAX, "%sbuild_new%s", str_bin_root, EXTENSION);
    snprintf(cmd_self_rebuild, PATH_MAX, "%s %s -Wall -Wextra -std=c99 -fno-builtin -o %s", COMPILER, str_src, str_bin_new);
}

b8 is_source_changed(void)
{
    struct stat stats;
    unsigned long mtime_src = 0;
    unsigned long mtime_bin = 0;

    if (stat(str_src, &stats) == 0)
        mtime_src = stats.st_mtime;
    else
        LOGERROR("%s\n", "File 'build.c' Not Found");

    if (stat(str_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
        LOGERROR("File 'build%s' Not Found\n", EXTENSION);

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return TRUE;

    return FALSE;
}

void rebuild_self(int argc, char **argv)
{
    LOGINFO("%s\n", "Rebuilding Self..");
    int self_rebuild_success = system(cmd_self_rebuild);
    if (self_rebuild_success != -1 &&
            WIFEXITED(self_rebuild_success) &&
            WEXITSTATUS(self_rebuild_success) == 0)
    {
        LOGINFO("%s\n", "Self Rebuild Success");
        remove(str_bin);
        rename(str_bin_new, str_bin);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL("'build%s' Failed, Process Aborted\n", EXTENSION);
        fail_cmd();
    }

    LOGFATAL("%s\n", "Self-Rebuild Failed, Process Aborted");
    fail_cmd();
}

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

void show_cmd(void)
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

void raw_cmd(void)
{
    printf("\nRAW:\n");
    for (u32 i = 0; i < CMD_MEMB; ++i)
    {
        if (!cmd.entry[i]) break;
        printf("%s ", cmd.entry[i]);
    }

    printf("%s", "\n\n");
}

void push_cmd(const str *string)
{
    if (cmd_pos >= CMD_MEMB - 1)
    {
        LOGERROR("%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= PATH_MAX - 1)
    {
        LOGERROR("string '%s' Too Long\n", string);
        return;
    }

    strncpy(cmd.entry[cmd_pos], string, PATH_MAX);
    ++cmd_pos;
}

void build_cmd(int argc, char **argv)
{
    if (!mem_alloc_str_buf(&cmd, CMD_MEMB, PATH_MAX, "cmd"))
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
            snprintf(str_main, PATH_MAX, "%s%s.c", DIR_TESTS, str_tests.entry[test_index]);
            snprintf(str_out, PATH_MAX, "./%s%s%s", DIR_ROOT_TESTS, str_tests.entry[test_index], EXTENSION);
            push_cmd(str_main);
            break;

        case STATE_LAUNCHER:
            snprintf(str_main, PATH_MAX, "%slauncher.c", DIR_LAUNCHER);
            push_cmd(str_main);
            snprintf(str_out, PATH_MAX, "./%slauncher%s", DIR_ROOT, EXTENSION);
            break;

        default:
            push_cmd(str_main);
            break;
    }

    /* ---- cflags ---------------------------------------------------------- */
    for (u32 i = 0; i < arr_len(str_cflags); ++i)
        push_cmd(str_cflags[i]);

    /* ---- libs ------------------------------------------------------------ */
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

void clean_cmd(void)
{
    mem_free((void*)&str_bin_root, PATH_MAX, "str_bin_root");
    mem_free_str_buf(&str_tests, NAME_MAX, "str_tests");
    mem_free_str_buf(&cmd, PATH_MAX, "cmd");
}

void fail_cmd(void)
{
    clean_cmd();
    _exit(EXIT_FAILURE);
}

void help(void)
{
    LOGINFO("%s%s%s%s%s%s",
            "Usage: ./build [options]...\n",
            "Options:\n",
            "    help       print this help\n",
            "    list       list all available options and tests\n",
            "    show       show build command\n",
            "    raw        show build command, raw\n");

    exit(EXIT_SUCCESS);
}

void list(void)
{
    printf("%s%s%s%s%s",
            "Options:\n",
            "    engine\n",
            "    launcher\n",
            "    test [n]\n",
            "Tests:\n");

    str_tests = get_dir_contents(DIR_TESTS);
    if (!str_tests.loaded)
    {
        LOGERROR("%s\n", "Fetching Tests Failed");
        fail_cmd();
    }

    str *str_tests_temp = NULL;
    if (!mem_alloc((void*)&str_tests_temp, NAME_MAX, "str_tests_temp"))
    {
        LOGERROR("%s\n", "Fetching Tests Failed");
        fail_cmd();
    }

    sort_str_buf(&str_tests);
    for (u32 i = 0; i < str_tests.count; ++i)
    {
        if (!evaluate_extension(str_tests.entry[i]))
            continue;

        strip_extension(str_tests.entry[i], str_tests_temp);
        printf("    %03d: %s\n", i, str_tests_temp);
    }

    mem_free((void*)&str_tests_temp, NAME_MAX, "str_tests_temp");
    clean_cmd();
    exit(EXIT_SUCCESS);
}

