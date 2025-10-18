#include <unistd.h>

#include "engine/h/platform.h"
#include "engine/h/defines.h"
#include "engine/memory.c"
#include "engine/dir.c"
#include "engine/logger.c"

#define DIR_ROOT        "Heaven-Hell Continuum/"
#define DIR_ROOT_TESTS  "tests/"
#define DIR_SRC         "src/"
#define DIR_TESTS       DIR_SRC"tests/"
#define CMD_MEMB        64
#define CMD_SIZE        512
#define ASSET_COUNT     3

#if defined(__STDC_VERSION__)
    #define STD __STDC_VERSION__
#elif defined(__STDC__)
    #define STD 89
#endif /* STD */

#define STD_C99 199901L

#if PLATFORM_LINUX
#include "engine/platform_linux.c"

#define COMPILER        "gcc"EXE
#define STR_OUT         DIR_ROOT"hhc"

str str_libs[][24] =
{
    "-lm",
    "-lglfw",
};

#elif PLATFORM_WIN
#include "engine/platform_windows.c"

#define COMPILER        "gcc"EXE
#define STR_OUT         "\""DIR_ROOT"hhc"EXE"\""

str str_libs[][24] =
{
    "-lm",
    "-lglfw3",
    "-lgdi32",
    "-lwinmm",
};

#endif /* PLATFORM */

enum Flags
{
    STATE_TEST = 1,

    FLAG_SHOW_CMD = 1,
    FLAG_RAW_CMD,
}; /* Flags */

/* ---- section: declarations ----------------------------------------------- */

u8 state = 0;
u16 flags = 0;
str *str_bin_root = NULL;
str str_src[CMD_SIZE] = {0};            /* path: ./build.c */
str str_bin[CMD_SIZE] = {0};            /* path: ./build%s, EXE */
str str_bin_new[CMD_SIZE] = {0};        /* path: ./build_new%s, EXE */
str str_bin_old[CMD_SIZE] = {0};        /* path: ./build_old%s, EXE */
str str_dir_root[CMD_SIZE] = {0};       /* bundle directory name */
buf cmd = {NULL};
u64 cmd_pos = 0;
buf str_tests = {NULL};

str str_main[CMD_SIZE] = DIR_SRC"main.c";

str str_cflags[][48] =
{
    ("-Wl,-rpath=$ORIGIN/lib/"PLATFORM),
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-fno-builtin",
};

str str_out[CMD_SIZE] = STR_OUT;

/* ---- section: signatures ------------------------------------------------- */

void init_build(void);
b8 is_source_changed(void);
void rebuild_self(char **argv);
u64 compare_argv(str *arg, int argc, str **argv);
b8 evaluate_extension(const str *file_name);
void strip_extension(const str *file_name, str *dest);
void show_cmd(void);
void raw_cmd(void);
void push_cmd(const str *string);
void build_main(void);
void build_cmd(int argc, char **argv);
void build_test(char **argv);
void free_cmd(void);
void fail_cmd(void);
void help(void);
void list(void);

/* ---- section: functions -------------------------------------------------- */

void
init_build(void)
{
    str_bin_root = get_path_bin_root();
    if (str_bin_root == NULL)
        fail_cmd();

    check_slash(str_bin_root);
    normalize_slash(str_bin_root);
    snprintf(str_src, CMD_SIZE, "%sbuild.c", str_bin_root);
    snprintf(str_bin, CMD_SIZE, "%sbuild%s", str_bin_root, EXE);
    snprintf(str_bin_new, CMD_SIZE, "%sbuild_new%s", str_bin_root, EXE);
    snprintf(str_bin_old, CMD_SIZE, "%sbuild_old%s", str_bin_root, EXE);
}

b8
is_source_changed(void)
{
    unsigned long mtime_src = 0;
    unsigned long mtime_bin = 0;

    struct stat stats;

    if (stat(str_src, &stats) == 0)
        mtime_src = stats.st_mtime;
    else
        LOGERROR("%s\n", "File 'build.c' Not Found");

    if (stat(str_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
        LOGERROR("File 'build%s' Not Found\n", EXE);

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return TRUE;

    return FALSE;
}

void
rebuild_self(char **argv)
{
    if (!mem_alloc_buf(&cmd, 16, CMD_SIZE, "cmd"))
        fail_cmd();

    LOGINFO("%s\n", "Rebuilding Self..");

    snprintf(cmd.i[0], CMD_SIZE, "%s", COMPILER);
    snprintf(cmd.i[1], CMD_SIZE, "%s", str_src);
    normalize_slash(cmd.i[1]);
    snprintf(cmd.i[2], CMD_SIZE, "%s", "-std=c99");
    snprintf(cmd.i[3], CMD_SIZE, "%s", "-Wall");
    snprintf(cmd.i[4], CMD_SIZE, "%s", "-Wextra");
    snprintf(cmd.i[5], CMD_SIZE, "%s", "-fno-builtin");
    snprintf(cmd.i[6], CMD_SIZE, "%s",
            "-Wno-implicit-function-declaration");
    snprintf(cmd.i[7], CMD_SIZE, "-I%s", str_bin_root);

    snprintf(cmd.i[8], CMD_SIZE, "%s", "-o");
    snprintf(cmd.i[9], CMD_SIZE, "%s", str_bin_new);
    normalize_slash(cmd.i[9]);

#if PLATFORM_LINUX
    cmd.i[10] = NULL;
#endif

    if (exec(&cmd, "cmd"))
    {
        LOGINFO("%s\n", "Self Rebuild Success");
        rename(str_bin, str_bin_old);
        rename(str_bin_new, str_bin);
        remove(str_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL("'build%s' Failed, Process Aborted\n", EXE);
        fail_cmd();
    }

    LOGFATAL("%s\n", "Self-Rebuild Failed, Process Aborted");
    fail_cmd();
}

u64
compare_argv(str *arg, int argc, char **argv)
{
    if (argc == 1)
        return 0;

    u32 i = 1;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

b8
evaluate_extension(const str *file_name)
{
    u64 len = strlen(file_name);
    if (len > 2 && strncmp(file_name + len - 2, ".c", 2) == 0)
        return TRUE;
    return FALSE;
}

void
strip_extension(const str *file_name, str *dest)
{
    u64 len = strlen(file_name);
    str file_name_buf[NAME_MAX] = {0};
    if (len > 2 && strncmp(file_name + len - 2, ".c", 2) == 0)
    {
        snprintf(file_name_buf, len - 1, "%s", file_name);
        strncpy(dest, file_name_buf, NAME_MAX);
    }
}

void
show_cmd(void)
{
    printf("\nCMD:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!cmd.i[i]) break;
        printf("    %.3d: %s\n", i, (str*)cmd.i[i]);
    }

    if (!(flags & FLAG_RAW_CMD))
        putchar('\n');
}

void
raw_cmd(void)
{
    printf("\nRAW:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!cmd.i[i]) break;
        printf("%s ", (str*)cmd.i[i]);
    }

    printf("%s", "\n\n");
}

void
push_cmd(const str *string)
{
    if (cmd_pos >= CMD_MEMB - 1)
    {
        LOGERROR("%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= CMD_SIZE - 1)
    {
        LOGERROR("string '%s' Too Long\n", string);
        return;
    }

    strncpy(cmd.i[cmd_pos], string, CMD_SIZE);
    ++cmd_pos;
}

void
build_main(void)
{
    snprintf(str_dir_root, CMD_SIZE, "%s%s", str_bin_root, DIR_ROOT);

    if (!mem_alloc_buf(&cmd, CMD_MEMB, CMD_SIZE, "cmd"))
        fail_cmd();

    str temp[CMD_SIZE] = {0};

    push_cmd(COMPILER);
    push_cmd(DIR_SRC"main.c");

    /* ---- children -------------------------------------------------------- */
    push_cmd(DIR_SRC"chunking.c");
    push_cmd(DIR_SRC"dir.c");
    push_cmd(DIR_SRC"gui.c");
    push_cmd(DIR_SRC"input.c");
    push_cmd(DIR_SRC"logic.c");
    push_cmd("engine/core.c");
    push_cmd("engine/dir.c");
    push_cmd("engine/logger.c");
    push_cmd("engine/math.c");
    push_cmd("engine/memory.c");
    push_cmd("engine/platform_"_PLATFORM".c");

    /* ---- includes -------------------------------------------------------- */
    push_cmd(stringf("-I%s", str_bin_root));
    snprintf(temp, CMD_SIZE - 1, "%sengine/include/glad/glad.c", str_bin_root);
    normalize_slash(temp);
    push_cmd(temp);

    /* ---- cflags ---------------------------------------------------------- */
    push_cmd(("-Wl,-rpath=$ORIGIN/lib/"PLATFORM));
    push_cmd("-std=c99");
    push_cmd("-ggdb");
    push_cmd("-Wall");
    push_cmd("-Wextra");
    push_cmd("-fno-builtin");
    push_cmd("-Wno-implicit-function-declaration");

    /* ---- libs ------------------------------------------------------------ */
    snprintf(temp, CMD_SIZE - 1, "-L%slib/"PLATFORM, str_bin_root);
    posix_slash(temp);
    push_cmd(temp);
    u32 i = 0;
    for (; i < arr_len(str_libs); ++i)
        push_cmd(str_libs[i]);

    /* ---- out ------------------------------------------------------------- */
    push_cmd("-o");
    push_cmd(STR_OUT);

#if PLATFORM_LINUX
    cmd.i[cmd_pos] = NULL;
#endif
}

void
build_cmd(int argc, char **argv)
{
    if (!mem_alloc_buf(&cmd, CMD_MEMB, CMD_SIZE, "cmd"))
        fail_cmd();

    u32 i = 0;
    str temp[CMD_SIZE] = {0};

    push_cmd(COMPILER);
    push_cmd(str_main);

    /* ---- children -------------------------------------------------------- */
    push_cmd("engine/core.c");
    push_cmd("engine/dir.c");
    push_cmd("engine/logger.c");
    push_cmd("engine/math.c");
    push_cmd("engine/memory.c");
    push_cmd("engine/platform_"_PLATFORM".c");

    /* ---- includes -------------------------------------------------------- */
    push_cmd(stringf("-I%s", str_bin_root));
    snprintf(temp, CMD_SIZE - 1, "%sengine/include/glad/glad.c", str_bin_root);
    normalize_slash(temp);
    push_cmd(temp);

    /* ---- cflags ---------------------------------------------------------- */
    if (compare_argv("raylib", argc, argv))
        push_cmd(("-Wl,-rpath=$ORIGIN/lib/"PLATFORM));
    else
    {
        posix_slash(str_cflags[0]);
        push_cmd(str_cflags[0]);
    }
    for (i = 1; i < arr_len(str_cflags); ++i)
    {
        normalize_slash(str_cflags[i]);
        push_cmd(str_cflags[i]);
    }

    /* ---- libs ------------------------------------------------------------ */
    snprintf(temp, CMD_SIZE - 1, "-L%slib/"PLATFORM, str_bin_root);
    posix_slash(temp);
    push_cmd(temp);
    if (compare_argv("raylib", argc, argv))
    {
        push_cmd("-lm");
        push_cmd("-lraylib");
    }
    else
    {
        for (i = 0; i < arr_len(str_libs); ++i)
            push_cmd(str_libs[i]);
    }

    /* ---- out ------------------------------------------------------------- */
    push_cmd("-o");
    push_cmd(str_out);

#if PLATFORM_LINUX
    cmd.i[cmd_pos] = NULL;
#endif
}

void
build_test(char **argv)
{
    if (!argv[2])
    {
        LOGERROR("Usage: ./build%s test [n]\n", EXE);
        fail_cmd();
    }
    u32 i = 0;
    str temp[CMD_SIZE] = {0};
    u64 test_index = atoi(argv[2]);
    str_tests.memb = get_dir_entry_count(DIR_TESTS);
    if (test_index >= str_tests.memb)
    {
        LOGERROR("'%s' Invalid, Try './build%s list'"
                "to List Available Options..\n", argv[2], EXE);
        fail_cmd();
    }
    str_tests = get_dir_contents(DIR_TESTS);
    if (!str_tests.loaded)
        fail_cmd();
    if (!mem_alloc_buf(&cmd, CMD_MEMB, CMD_SIZE, "cmd"))
        fail_cmd();

    for (i = 0; i < str_tests.memb; ++i)
        strip_extension(str_tests.i[i], str_tests.i[i]);

    snprintf(str_dir_root, CMD_SIZE, "%s%s%s/",
            str_bin_root, DIR_ROOT_TESTS, (str*)str_tests.i[test_index]);

    /* ---- start ----------------------------------------------------------- */
    push_cmd(COMPILER);
    push_cmd(stringf("%s%s.c", DIR_TESTS, str_tests.i[test_index]));

    /* ---- children -------------------------------------------------------- */
    push_cmd("engine/core.c");
    push_cmd("engine/dir.c");
    push_cmd("engine/logger.c");
    push_cmd("engine/math.c");
    push_cmd("engine/memory.c");
    push_cmd("engine/platform_"_PLATFORM".c");

    /* ---- includes -------------------------------------------------------- */
    push_cmd(stringf("-I%s", str_bin_root));
    snprintf(temp, CMD_SIZE, "%sengine/include/glad/glad.c", str_bin_root);
    normalize_slash(temp);
    push_cmd(temp);

    /* ---- cflags ---------------------------------------------------------- */
    push_cmd(("-Wl,-rpath=$ORIGIN/lib/"PLATFORM));
    push_cmd("-std=c99");
    push_cmd("-Wall");
    push_cmd("-Wextra");
    push_cmd("-fno-builtin");

    /* ---- libs ------------------------------------------------------------ */
    snprintf(temp, CMD_SIZE, "-L%slib/"PLATFORM, str_bin_root);
    push_cmd(temp);
    for (i = 0; i < arr_len(str_libs); ++i)
        push_cmd(str_libs[i]);

    /* ---- out ------------------------------------------------------------- */
    push_cmd("-o");
    push_cmd(stringf("./%s%s/%s%s", DIR_ROOT_TESTS,
                str_tests.i[test_index], str_tests.i[test_index], EXE));

#if PLATFORM_LINUX
    cmd.i[cmd_pos] = NULL;
#endif
}

void
free_cmd(void)
{
    mem_free((void*)&str_bin_root, CMD_SIZE, "str_bin_root");
    mem_free_buf(&str_tests, "str_tests");
    mem_free_buf(&cmd, "cmd");
}

void
fail_cmd(void)
{
    free_cmd();
    _exit(EXIT_FAILURE);
}

void
help(void)
{
    printf("%s",
            "Usage: ./build [options]...\n"
            "Options:\n"
            "    help       print this help\n"
            "    list       list all available options and tests\n"
            "    show       show build command\n"
            "    raw        show build command, raw\n");
    exit(EXIT_SUCCESS);
}

void
list(void)
{
    printf("%s",
            "Options:\n"
            "    engine\n"
            "    launcher\n"
            "    test [n]\n"
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

    u32 i = 0;
    for (; i < str_tests.memb; ++i)
    {
        if (!evaluate_extension(str_tests.i[i]))
            continue;

        strip_extension(str_tests.i[i], str_tests_temp);
        printf("    %03d: %s\n", i, str_tests_temp);
    }

    mem_free((void*)&str_tests_temp, NAME_MAX, "str_tests_temp");
    mem_free_buf(&str_tests, "str_tests");
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    log_level = LOGLEVEL_INFO;
    if (compare_argv("LOGFATAL", argc, argv)) log_level = LOGLEVEL_FATAL;
    if (compare_argv("LOGERROR", argc, argv)) log_level = LOGLEVEL_ERROR;
    if (compare_argv("LOGWARN", argc, argv)) log_level = LOGLEVEL_WARNING;
    if (compare_argv("LOGINFO", argc, argv)) log_level = LOGLEVEL_INFO;
    if (compare_argv("LOGDEBUG", argc, argv)) log_level = LOGLEVEL_DEBUG;
    if (compare_argv("LOGTRACE", argc, argv)) log_level = LOGLEVEL_TRACE;

    init_build();

    if (STD != STD_C99)
    {
        LOGINFO("%s\n", "Rebuilding Self With -std=c99..");
        rebuild_self(argv);
    }

    if (is_source_changed())
        rebuild_self(argv);

    if (compare_argv("help", argc, argv))       help();
    if (compare_argv("list", argc, argv))       list();
    if (compare_argv("test", argc, argv))       state = STATE_TEST;
    if (compare_argv("show", argc, argv))       flags |= FLAG_SHOW_CMD;
    if (compare_argv("raw", argc, argv))        flags |= FLAG_RAW_CMD;

    if (!is_dir_exists(DIR_SRC, TRUE) ||
            !is_dir_exists(DIR_TESTS, TRUE))
        return -1;

    if (!state)
        build_main();
    else if (state == STATE_TEST)
        build_test(argv);
    else
        build_cmd(argc, argv);

    if (flags & FLAG_SHOW_CMD)                  show_cmd();
    if (flags & FLAG_RAW_CMD)                   raw_cmd();

    if (!is_dir_exists(str_dir_root, FALSE))
        make_dir(str_dir_root);

    if (state == STATE_TEST && !is_dir_exists(DIR_ROOT_TESTS, FALSE))
        make_dir(DIR_ROOT_TESTS);

    str str_from[ASSET_COUNT][CMD_SIZE] = {0};
    str str_to[ASSET_COUNT][CMD_SIZE] = {0};
    str str_mkdir[ASSET_COUNT][CMD_SIZE] = {0};

    snprintf(str_from[0], CMD_SIZE, "%sLICENSE", str_bin_root);
    snprintf(str_from[1], CMD_SIZE, "%slib/"PLATFORM, str_bin_root);
    snprintf(str_from[2], CMD_SIZE, "%sresources/", str_bin_root);
    snprintf(str_to[0], CMD_SIZE, "%sLICENSE", str_dir_root);
    snprintf(str_to[1], CMD_SIZE,
            "%slib/"PLATFORM, str_dir_root);
    snprintf(str_to[2], CMD_SIZE,
            "%sresources/", str_dir_root);
    snprintf(str_mkdir[0], CMD_SIZE, "%slib/", str_dir_root);

    u32 i = 0;
    for (; i < ASSET_COUNT; ++i)
    {
        normalize_slash(str_from[i]);
        normalize_slash(str_to[i]);
    }
    normalize_slash(str_mkdir[0]);
    make_dir(str_mkdir[0]);

    for (i = 0; i < ASSET_COUNT; ++i)
    {
        if (is_file(str_from[i]))
        {
            if (copy_file(str_from[i], str_to[i], "r", "w") != 0)
                goto cleanup;
        }
        else if (copy_dir(str_from[i], str_to[i], TRUE, "r", "w") != 0)
            goto cleanup;
    }

    if (!exec(&cmd, "build"))
        goto cleanup;

    free_cmd();
    return 0;

cleanup:
    fail_cmd();
    return -1;
}
