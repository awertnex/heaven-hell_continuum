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
#include "src/engine/platform_linux.c"

#define EXTENSION       ""
#define COMPILER        "gcc"EXTENSION
#define STR_OUT         DIR_ROOT"hhc"

str str_children[][32] =
{
    DIR_SRC"chunking.c",
    DIR_SRC"dir.c",
    DIR_SRC"gui.c",
    DIR_SRC"input.c",
    DIR_SRC"logic.c",
    DIR_SRC"engine/core.c",
    DIR_SRC"engine/dir.c",
    DIR_SRC"engine/logger.c",
    DIR_SRC"engine/math.c",
    DIR_SRC"engine/memory.c",
    DIR_SRC"engine/platform_linux.c",
};

str str_libs[][32] =
{
    "-lm",
    //"-lpthread",
    "-lglfw",
    //"-lXrandr",
    //"-lXi",
    //"-lX11",
    //"-lXxf86vm",
    //"-ldl",
    //"-lXinerama",
};

#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include "src/engine/platform_windows.c"

#define EXTENSION       ".exe"
#define COMPILER        "gcc"EXTENSION
#define STR_OUT         "\""DIR_ROOT"hhc"EXTENSION"\""

str str_children[][32] =
{
    DIR_SRC"chunking.c",
    DIR_SRC"dir.c",
    DIR_SRC"gui.c",
    DIR_SRC"input.c",
    DIR_SRC"logic.c",
    DIR_SRC"engine/core.c",
    DIR_SRC"engine/dir.c",
    DIR_SRC"engine/logger.c",
    DIR_SRC"engine/math.c",
    DIR_SRC"engine/memory.c",
    DIR_SRC"engine/platform_windows.c",
};

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
    STATE_LAUNCHER,

    FLAG_SHOW_CMD = 1,
    FLAG_RAW_CMD,
}; /* Flags */

/* ---- section: declarations ----------------------------------------------- */

u8 state = 0;
u16 flags = 0;
str *str_bin_root = NULL;
str str_src[PATH_MAX] = {0};            /* path: ./build.c */
str str_bin[PATH_MAX] = {0};            /* path: ./build%s, EXTENSION */
str str_bin_new[PATH_MAX] = {0};        /* path: ./build_new%s, EXTENSION */
str str_bin_old[PATH_MAX] = {0};        /* path: ./build_old%s, EXTENSION */
buf cmd = {NULL};
u64 cmd_pos = 0;
buf str_tests = {NULL};

str str_main[PATH_MAX] = DIR_SRC"main.c";

str str_cflags[][32] =
{
    "-Wl,-rpath=$ORIGIN/lib/"PLATFORM,
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-fno-builtin",
};

str str_out[PATH_MAX] = STR_OUT;

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
void free_cmd(void);
void fail_cmd(void);
void help(void);
void list(void);

/* ---- section: main ------------------------------------------------------- */

int main(int argc, char **argv)
{
    init_build();

    if (C_STD != STD_C99)
    {
        LOGINFO("%s\n", "Rebuilding Self With -std=c99..");
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

    str cmd_asset_in[PATH_MAX] = {0};
    str cmd_asset_out[PATH_MAX] = {0};

    snprintf(cmd_asset_in, PATH_MAX - 1, "%sLICENSE", str_bin_root);
    snprintf(cmd_asset_out, PATH_MAX - 1, "%s"DIR_ROOT"LICENSE", str_bin_root);
    normalize_slash(cmd_asset_in);
    normalize_slash(cmd_asset_out);
    if (copy_file(cmd_asset_in, cmd_asset_out) != 0) goto cleanup;

    snprintf(cmd_asset_out, PATH_MAX - 1, "%s"DIR_ROOT"lib/", str_bin_root);
    normalize_slash(cmd_asset_out);
    make_dir(cmd_asset_out);

    snprintf(cmd_asset_in, PATH_MAX - 1, "%slib/"PLATFORM, str_bin_root);
    snprintf(cmd_asset_out, PATH_MAX - 1, "%s"DIR_ROOT"lib/"PLATFORM, str_bin_root);
    normalize_slash(cmd_asset_in);
    normalize_slash(cmd_asset_out);
    if (copy_dir(cmd_asset_in, cmd_asset_out, 1) != 0) goto cleanup;

    snprintf(cmd_asset_in, PATH_MAX - 1, "%sresources/", str_bin_root);
    snprintf(cmd_asset_out, PATH_MAX - 1, "%s"DIR_ROOT"resources/", str_bin_root);
    normalize_slash(cmd_asset_in);
    normalize_slash(cmd_asset_out);
    if (copy_dir(cmd_asset_in, cmd_asset_out, 1) != 0) goto cleanup;

    snprintf(cmd_asset_in, PATH_MAX - 1, "%sshaders/", str_bin_root);
    snprintf(cmd_asset_out, PATH_MAX - 1, "%s"DIR_ROOT"resources/shaders/", str_bin_root);
    normalize_slash(cmd_asset_in);
    normalize_slash(cmd_asset_out);
    if (copy_dir(cmd_asset_in, cmd_asset_out, 1) != 0) goto cleanup;

    if (!exec(&cmd, "build")) goto cleanup;

    free_cmd();
    return 0;

cleanup:
    fail_cmd();
}

/* ---- section: functions -------------------------------------------------- */

void init_build(void)
{
    str_bin_root = get_path_bin_root();
    if (str_bin_root == NULL)
        fail_cmd();

    check_slash(str_bin_root);
    normalize_slash(str_bin_root);
    snprintf(str_src, PATH_MAX, "%sbuild.c", str_bin_root);
    snprintf(str_bin, PATH_MAX, "%sbuild%s", str_bin_root, EXTENSION);
    snprintf(str_bin_new, PATH_MAX, "%sbuild_new%s", str_bin_root, EXTENSION);
    snprintf(str_bin_old, PATH_MAX, "%sbuild_old%s", str_bin_root, EXTENSION);
}

b8 is_source_changed(void)
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
        LOGERROR("File 'build%s' Not Found\n", EXTENSION);

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return TRUE;

    return FALSE;
}

void rebuild_self(int argc, char **argv)
{
    if (!mem_alloc_buf(&cmd, 16, PATH_MAX, "cmd"))
        fail_cmd();

    LOGINFO("%s\n", "Rebuilding Self..");

    snprintf(cmd.i[0], PATH_MAX - 1, "%s", COMPILER);
    snprintf(cmd.i[1], PATH_MAX - 1, "%s", str_src);
    normalize_slash(cmd.i[1]);
    snprintf(cmd.i[2], PATH_MAX - 1, "%s", "-Wall");
    snprintf(cmd.i[3], PATH_MAX - 1, "%s", "-Wextra");
    snprintf(cmd.i[4], PATH_MAX - 1, "%s", "-std=c99");
    snprintf(cmd.i[5], PATH_MAX - 1, "%s", "-fno-builtin");
    snprintf(cmd.i[6], PATH_MAX - 1, "%s", "-o");
    snprintf(cmd.i[7], PATH_MAX - 1, "%s", str_bin_new);
    normalize_slash(cmd.i[7]);

#if defined(__linux__) || defined(__linux)
    cmd.i[8] = NULL;
#endif

    if (exec(&cmd, "cmd"))
    {
        LOGINFO("%s\n", "Self Rebuild Success");
        rename(str_bin, str_bin_old);
        rename(str_bin_new, str_bin);
        remove(str_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL("'build%s' Failed, Process Aborted\n", EXTENSION);
        goto cleanup;
    }

    LOGFATAL("%s\n", "Self-Rebuild Failed, Process Aborted");
    goto cleanup;

    return;

cleanup:
    mem_free_buf(&cmd, "cmd");
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
        if (!cmd.i[i]) break;
        printf("    %.3d: %s\n", i, (str*)cmd.i[i]);
    }

    if (!(flags & FLAG_RAW_CMD))
        putchar('\n');
}

void raw_cmd(void)
{
    printf("\nRAW:\n");
    for (u32 i = 0; i < CMD_MEMB; ++i)
    {
        if (!cmd.i[i]) break;
        printf("%s ", (str*)cmd.i[i]);
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

    strncpy(cmd.i[cmd_pos], string, PATH_MAX);
    ++cmd_pos;
}

void build_cmd(int argc, char **argv)
{
    if (!mem_alloc_buf(&cmd, CMD_MEMB, PATH_MAX, "cmd"))
        fail_cmd();

    str temp[PATH_MAX] = {0};

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
            str_tests.memb = get_dir_entry_count(DIR_TESTS);
            if (test_index <= 0 || test_index >= str_tests.memb)
            {
                LOGERROR("'%s' Invalid, Try './build%s list' to List Available Options..\n", argv[2], EXTENSION);
                fail_cmd();
            }

            if (!mem_alloc_buf(&str_tests, str_tests.memb, NAME_MAX, "str_tests"))
                fail_cmd();


            sort_buf(&str_tests);
            snprintf(str_main, PATH_MAX, "%s%s.c", DIR_TESTS, (str*)str_tests.i[test_index]);
            snprintf(str_out, PATH_MAX, "./%s%s%s", DIR_ROOT_TESTS, (str*)str_tests.i[test_index], EXTENSION);
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

    /* ---- children -------------------------------------------------------- */
    for (u32 i = 0; i < arr_len(str_children); ++i)
    {
        normalize_slash(str_children[i]);
        push_cmd(str_children[i]);
    }

    /* ---- includes -------------------------------------------------------- */
    snprintf(temp, PATH_MAX - 1, "%sinclude/glad/glad.c", str_bin_root);
    normalize_slash(temp);
    push_cmd(temp);

    /* ---- cflags ---------------------------------------------------------- */
    posix_slash(str_cflags[0]);
    push_cmd(str_cflags[0]);
    for (u32 i = 1; i < arr_len(str_cflags); ++i)
    {
        normalize_slash(str_cflags[i]);
        push_cmd(str_cflags[i]);
    }

    /* ---- libs ------------------------------------------------------------ */
    snprintf(temp, PATH_MAX - 1, "-L%slib/"PLATFORM, str_bin_root);
    posix_slash(temp);
    push_cmd(temp);

    for (u32 i = 0; i < arr_len(str_libs); ++i)
        push_cmd(str_libs[i]);

    /* ---- out ------------------------------------------------------------- */
    push_cmd("-o");
    push_cmd(str_out);

#if defined(__linux__) || defined(__linux)
    cmd.i[cmd_pos] = NULL;
#endif
}

/*
 * scrap function, for reference;
 */
#if 0
#include <glob.h>

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
        strncpy(cmd.i[cmd_pos], glob_buf.gl_pathv[i], NAME_MAX);
        ++cmd_pos;
    }
}
#endif /* 0 */

void free_cmd(void)
{
    mem_free((void*)&str_bin_root, PATH_MAX, "str_bin_root");
    mem_free_buf(&str_tests, "str_tests");
    mem_free_buf(&cmd, "cmd");
}

void fail_cmd(void)
{
    free_cmd();
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

    sort_buf(&str_tests);
    for (u32 i = 0; i < str_tests.memb; ++i)
    {
        if (!evaluate_extension(str_tests.i[i]))
            continue;

        strip_extension(str_tests.i[i], str_tests_temp);
        printf("    %03d: %s\n", i, str_tests_temp);
    }

    mem_free((void*)&str_tests_temp, NAME_MAX, "str_tests_temp");
    free_cmd();
    exit(EXIT_SUCCESS);
}

