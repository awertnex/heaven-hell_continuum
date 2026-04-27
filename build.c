#include "deps/buildtool/buildtool.h"
#include "deps/fossil/build.h"

#define DIR_SRC     "src/"
#define DIR_OUT     "Heaven-Hell Continuum/"
#define STR_OUT     DIR_OUT"hhc"

static str str_cflags[][CMD_SIZE] =
{
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wformat-truncation=0",
    "-ggdb",
};

static str str_files[][CMD_SIZE] =
{
    DIR_SRC"main.c",
    DIR_SRC"assets.c",
    DIR_SRC"chunking.c",
    DIR_SRC"common.c",
    DIR_SRC"dir.c",
    DIR_SRC"gui.c",
    DIR_SRC"input.c",
    DIR_SRC"player.c",
    DIR_SRC"terrain.c",
    DIR_SRC"world.c",
};

int main(int argc, char **argv)
{
    u32 i = 0;

    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    cmd_push(NULL, COMPILER);

    if (find_token("release", argc, argv))
    {
        LOGINFO(FALSE, "%s\n", "Building For Release..");
        cmd_push(NULL, "-DHHC_RELEASE_BUILD");
    }
    else
        for (i = 0; i < arr_len(str_cflags); ++i)
            cmd_push(NULL, str_cflags[i]);

    for (i = 0; i < arr_len(str_files); ++i)
        cmd_push(NULL, str_files[i]);

    cmd_push(NULL, "-I.");
    cmd_push(NULL, "-std=c99");
    cmd_push(NULL, "-Ofast");
    fsl_engine_link_libs(NULL);
    fsl_engine_set_runtime_path(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, STR_OUT);
    cmd_ready(NULL);

    if (exec(&_cmd, "main()._cmd") != ERR_SUCCESS)
        cmd_fail(NULL);

    if (
            copy_file("LICENSE",        DIR_OUT) != ERR_SUCCESS ||
            copy_dir("assets/",         DIR_OUT, FALSE) != ERR_SUCCESS ||
            copy_dir("fossil/fossil/",  DIR_OUT, TRUE) != ERR_SUCCESS ||
            copy_dir("fossil/lib/", ".", FALSE) != ERR_SUCCESS ||
            copy_dir("fossil/deps/", ".", FALSE) != ERR_SUCCESS)
        cmd_fail(NULL);

    build_err = ERR_SUCCESS;
    return build_err;
}
