#include <unistd.h>

#include "h/build.h"
#include "memory.c"
#include "dir.c"
#include "logger.c"

#if PLATFORM_LINUX
    #include "platform_linux.c"
#elif PLATFORM_WIN
    #include "platform_windows.c"
#endif /* PLATFORM */

#if defined(__STDC_VERSION__)
    #define STD __STDC_VERSION__
#elif defined(__STDC__)
    #define STD 89
#endif /* STD */

#define STD_C99 199901L

u32 engine_err = ERR_SUCCESS;
static u32 flag = 0;
str *str_build_root = NULL;
static str str_build_src[CMD_SIZE] = {0};
static str str_build_bin[CMD_SIZE] = {0};
static str str_build_bin_new[CMD_SIZE] = {0};
static str str_build_bin_old[CMD_SIZE] = {0};
buf cmd = {NULL};
static u64 cmd_pos = 0;

static u32 is_build_source_changed(void);

/* can force-terminate process */
static void self_rebuild(char **argv);

static void cmd_show(void);
static void cmd_raw(void);
static void help(void);

void
build_init(int argc, char **argv,
        const str *build_src_name, const str *build_bin_name)
{
    if (argv_compare("help", argc, argv))       help();
    if (argv_compare("show", argc, argv))       flag |= FLAG_CMD_SHOW;
    if (argv_compare("raw", argc, argv))        flag |= FLAG_CMD_RAW;

    logger_init(TRUE, argc, argv);

    if (argv_compare("LOGFATAL", argc, argv)) log_level_max = LOGLEVEL_FATAL;
    if (argv_compare("LOGERROR", argc, argv)) log_level_max = LOGLEVEL_ERROR;
    if (argv_compare("LOGWARN", argc, argv)) log_level_max = LOGLEVEL_WARNING;
    if (argv_compare("LOGINFO", argc, argv)) log_level_max = LOGLEVEL_INFO;
    if (argv_compare("LOGDEBUG", argc, argv)) log_level_max = LOGLEVEL_DEBUG;
    if (argv_compare("LOGTRACE", argc, argv)) log_level_max = LOGLEVEL_TRACE;

    str_build_root = get_path_bin_root();
    if (engine_err != ERR_SUCCESS)
        cmd_fail();

    check_slash(str_build_root);
    normalize_slash(str_build_root);

    snprintf(str_build_src, CMD_SIZE, "%s%s", str_build_root, build_src_name);
    normalize_slash(str_build_src);

    snprintf(str_build_bin, CMD_SIZE, "%s%s",
            str_build_root, build_bin_name);
    snprintf(str_build_bin_new, CMD_SIZE, "%s%s_new",
            str_build_root, build_bin_name);
    snprintf(str_build_bin_old, CMD_SIZE, "%s%s_old",
            str_build_root, build_bin_name);

    if (STD != STD_C99)
    {
        LOGINFO(FALSE, "%s\n", "Rebuilding Self With -std=c99..");
        self_rebuild(argv);
    }

    if (is_build_source_changed() == ERR_SUCCESS)
    {
        LOGINFO(FALSE, "%s\n", "Rebuilding Self..");
        self_rebuild(argv);
    }

    if (mem_alloc_buf(&cmd, CMD_MEMB, CMD_SIZE, "cmd") != ERR_SUCCESS)
        cmd_fail();
}

static u32
is_build_source_changed(void)
{
    unsigned long mtime_src = 0;
    unsigned long mtime_bin = 0;

    struct stat stats;

    if (stat(str_build_src, &stats) == 0)
        mtime_src = stats.st_mtime;
    else
    {
        LOGERROR(FALSE, ERR_FILE_NOT_FOUND,
                "%s\n", "Build Source File Not Found");
        return engine_err;
    }

    if (stat(str_build_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
    {
        LOGERROR(FALSE, ERR_FILE_NOT_FOUND,
                "%s\n", "File 'build"EXE"' Not Found\n");
        return engine_err;
    }

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return ERR_SUCCESS;

    engine_err = ERR_SOURCE_NOT_CHANGE;
    return engine_err;
}

static void
self_rebuild(char **argv)
{
    if (mem_alloc_buf(&cmd, 16, CMD_SIZE, "cmd") != ERR_SUCCESS)
        cmd_fail();

    cmd_push(COMPILER);
    cmd_push(stringf("%s", str_build_src));
    cmd_push("-ggdb");
    cmd_push("-std=c99");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-fno-builtin");
    cmd_push("-Wno-implicit-function-declaration");
    cmd_push("-o");
    cmd_push(str_build_bin_new);
    cmd_ready();

    if (flag & FLAG_CMD_SHOW) cmd_show();
    if (flag & FLAG_CMD_RAW) cmd_raw();

    if (exec(&cmd, "cmd") == ERR_SUCCESS)
    {
        LOGINFO(FALSE, "%s\n", "Self Rebuild Success");
        rename(str_build_bin, str_build_bin_old);
        rename(str_build_bin_new, str_build_bin);
        remove(str_build_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL(FALSE, engine_err,
                "'build%s' Failed, Process Aborted\n", EXE);
        cmd_fail();
    }

    LOGFATAL(FALSE, engine_err,
            "%s\n", "Self-Rebuild Failed, Process Aborted");
    cmd_fail();
}

u32
engine_build(const str *engine_dir, const str *out_dir)
{
    if (
            is_dir_exists(engine_dir, TRUE) != ERR_SUCCESS ||
            is_dir_exists(out_dir, TRUE) != ERR_SUCCESS)
        return engine_err;

    str engine_dir_processed[CMD_SIZE] = {0};
    str out_dir_processed[CMD_SIZE] = {0};
    memcpy(engine_dir_processed, engine_dir, CMD_SIZE);
    memcpy(out_dir_processed, out_dir, CMD_SIZE);

    check_slash(engine_dir_processed);
    check_slash(out_dir_processed);
    cmd_push(COMPILER);
    cmd_push(stringf("%score.c", engine_dir_processed));
    cmd_push(stringf("%sdir.c", engine_dir_processed));
    cmd_push(stringf("%slogger.c", engine_dir_processed));
    cmd_push(stringf("%smath.c", engine_dir_processed));
    cmd_push(stringf("%smemory.c", engine_dir_processed));
    cmd_push(stringf("%splatform_"_PLATFORM".c", engine_dir_processed));
    cmd_push(stringf("%sinclude/glad/glad.c", engine_dir_processed));
    cmd_push("-I.");
    cmd_push("-shared");
    cmd_push("-fPIC");
    cmd_push("-std=c99");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-fno-builtin");
    cmd_push("-Wno-implicit-function-declaration");
    cmd_push("-o");
    cmd_push(stringf("%s"ENGINE_NAME_LIB, out_dir_processed));
    cmd_ready();

    if (flag & FLAG_CMD_SHOW) cmd_show();
    if (flag & FLAG_CMD_RAW) cmd_raw();

    if (exec(&cmd, "build") != ERR_SUCCESS)
        cmd_fail();

    cmd_free();
    engine_err = ERR_SUCCESS;
    _exit(engine_err);
    return engine_err;
}

u64
argv_compare(str *arg, int argc, char **argv)
{
    if (argc == 1)
        return 0;

    u32 i = 1;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

static void
cmd_show(void)
{
    printf("\nCMD:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!cmd.i[i]) break;
        printf("    %.3d: %s\n", i, (str*)cmd.i[i]);
    }

    if (!(flag & FLAG_CMD_RAW))
        putchar('\n');
}

static void
cmd_raw(void)
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
cmd_push(const str *string)
{
    if (cmd_pos >= CMD_MEMB - 1)
    {
        LOGERROR(FALSE, ERR_BUFFER_FULL, "%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= CMD_SIZE - 1)
    {
        LOGERROR(FALSE, ERR_STRING_TOO_LONG,
                "string '%s' Too Long\n", string);
        return;
    }

    LOGTRACE(FALSE,
            "Pushing String '%s' to cmd.i[%"PRId64"]..\n", string, cmd_pos);
    strncpy(cmd.i[cmd_pos++], string, CMD_SIZE);
}

void
cmd_ready(void)
{
#if PLATFORM_LINUX
    cmd.i[cmd_pos] = NULL;
#endif
}

void
cmd_free(void)
{
    mem_free((void*)&str_build_root, CMD_SIZE, "str_build_root");
    mem_free_buf(&cmd, "cmd");
    cmd_pos = 0;
}

void
cmd_fail(void)
{
    cmd_free();
    _exit(engine_err);
}

static void
help(void)
{
    printf("%s",
            "Usage: ./build [options]...\n"
            "Options:\n"
            "    help       print this help\n"
            "    show       show build command\n"
            "    raw        show build command, raw\n");
    exit(ERR_SUCCESS);
}
