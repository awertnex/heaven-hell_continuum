#include "fossil/deps/fossil/external/buildtool/buildtool.h"
#include "fossil/deps/fossil/h/buildtool_config.h"

#define DIR_SRC     "src/"
#define DIR_OUT     "Heaven-Hell Continuum/"
#define STR_OUT     DIR_OUT"hhc"

bt_buf cmd = {0};

static str str_cflags[][CMD_SIZE] =
{
    "-std=c89",
    "-Ofast",
    "-I."
};

static str str_cflags_debug[][CMD_SIZE] =
{
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wformat-truncation=0",
    "-ggdb"
};

static str str_libs[][CMD_SIZE] =
{
    "-lmvec"
};

static str str_files[][CMD_SIZE] =
{
    DIR_SRC"main.c",
    DIR_SRC"chunking/chunk_draw.c",
    DIR_SRC"chunking/chunking.c",
    DIR_SRC"chunking/chunking_debug_tools.c",
    DIR_SRC"terrain/terrain.c",
    DIR_SRC"terrain/perlin_noise.c",
    DIR_SRC"assets.c",
    DIR_SRC"common.c",
    DIR_SRC"dir.c",
    DIR_SRC"gui.c",
    DIR_SRC"input.c",
    DIR_SRC"player.c",
    DIR_SRC"world.c"
};

int main(int argc, char **argv)
{
    u32 i = 0;

    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    LOGWARNING(0, FALSE, "THIS VERSION SAVES CHUNKS AS SEPARATE FILES ON DISK, VERY DISK-HEAVY AND PERFORMANCE INTENSIVE\n\n");

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    if (
            copy_dir("fossil/deps/",    ".", FALSE) != ERR_SUCCESS ||
            copy_dir("fossil/lib/",     ".", FALSE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (is_dir_exists(DIR_OUT, FALSE) != ERR_SUCCESS)
        make_dir(DIR_OUT);

    cmd_push(&cmd, COMPILER);

    if (find_token("release", argc, argv))
    {
        LOGINFO(FALSE, "Building For Release..\n");
        cmd_push(&cmd, "-DHHC_RELEASE_BUILD");
    }
    else
    {
        LOGWARNING(0, FALSE, "Building in Debug Mode..\n");
        for (i = 0; i < arr_len(str_cflags_debug); ++i)
            cmd_push(&cmd, str_cflags_debug[i]);
    }

    for (i = 0; i < arr_len(str_cflags); ++i)
        cmd_push(&cmd, str_cflags[i]);

    for (i = 0; i < arr_len(str_libs); ++i)
        cmd_push(&cmd, str_libs[i]);

    fsl_engine_link_libs(&cmd);
    fsl_engine_set_runtime_path(&cmd);

    for (i = 0; i < arr_len(str_files); ++i)
        cmd_push(&cmd, str_files[i]);

    cmd_push(&cmd, "-o");
    cmd_push(&cmd, STR_OUT);
    cmd_ready(&cmd);

    if (
            copy_file("LICENSE",        DIR_OUT) != ERR_SUCCESS ||
            copy_file("version.txt",    DIR_OUT) != ERR_SUCCESS ||
            copy_dir("assets/",         DIR_OUT, FALSE) != ERR_SUCCESS ||
            copy_dir("fossil/fossil/",  DIR_OUT, TRUE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (exec(&cmd, "main().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}
