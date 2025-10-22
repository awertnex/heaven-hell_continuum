#include "engine/build.c"

#define DIR_ROOT        "Heaven-Hell Continuum/"
#define DIR_SRC         "src/"
#define ASSET_COUNT     3
#define NEW_DIR_COUNT   2

u32 *const BUILD_ERR = &engine_err;
str str_out_dir[CMD_SIZE] = {0}; /* bundle directory name */

#if PLATFORM_LINUX
    #define STR_OUT     DIR_ROOT"hhc"
    str str_libs[][CMD_SIZE] =
    {
        "-lm",
        "-lglfw",
        "-lfossil",
    };
#elif PLATFORM_WIN
    #define STR_OUT     "\""DIR_ROOT"hhc"EXE"\""
    str str_libs[][CMD_SIZE] =
    {
        "-lgdi32",
        "-lwinmm",
        "-lm",
        "-lglfw3",
        "-lfossil",
    };
#endif /* PLATFORM */

int
main(int argc, char **argv)
{
    build_init(argc, argv, "build.c", "build"EXE);
    if (argv_compare("engine", argc, argv))
        engine_build(
                stringf("%sengine/", str_build_root),
                stringf("%slib/"_PLATFORM, str_build_root)
                );

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return *BUILD_ERR;

    snprintf(str_out_dir, CMD_SIZE, "%s%s", str_build_root, DIR_ROOT);

    u32 i = 0;
    cmd_push(COMPILER);
    cmd_push(DIR_SRC"main.c");
    cmd_push(DIR_SRC"chunking.c");
    cmd_push(DIR_SRC"dir.c");
    cmd_push(DIR_SRC"gui.c");
    cmd_push(DIR_SRC"input.c");
    cmd_push(DIR_SRC"logic.c");
    cmd_push("-I.");
    cmd_push("-Wl,-rpath=$ORIGIN/lib/"PLATFORM);
    cmd_push("-std=c99");
    cmd_push("-ggdb");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-fno-builtin");
    cmd_push("-Llib/"PLATFORM);
    for (i = 0; i < arr_len(str_libs); ++i)
        cmd_push(str_libs[i]);
    cmd_push("-o");
    cmd_push(STR_OUT);
    cmd_ready();

    if (flag & FLAG_CMD_SHOW) cmd_show();
    if (flag & FLAG_CMD_RAW) cmd_raw();

    str str_mkdir[NEW_DIR_COUNT][CMD_SIZE] = {0};
    str str_from[ASSET_COUNT][CMD_SIZE] = {0};
    str str_to[ASSET_COUNT][CMD_SIZE] = {0};
    snprintf(str_mkdir[0],  CMD_SIZE, "%s", str_out_dir);
    snprintf(str_mkdir[1],  CMD_SIZE, "%slib/", str_out_dir);
    snprintf(str_from[0],   CMD_SIZE, "%sLICENSE", str_build_root);
    snprintf(str_from[1],   CMD_SIZE, "%slib/"PLATFORM, str_build_root);
    snprintf(str_from[2],   CMD_SIZE, "%sresources/", str_build_root);
    snprintf(str_to[0],     CMD_SIZE, "%sLICENSE", str_out_dir);
    snprintf(str_to[1],     CMD_SIZE, "%slib/"PLATFORM, str_out_dir);
    snprintf(str_to[2],     CMD_SIZE, "%sresources/", str_out_dir);

    for (i = 0; i < NEW_DIR_COUNT; ++i)
    {
        normalize_slash(str_mkdir[i]);
        make_dir(str_mkdir[i]);
    }

    for (i = 0; i < ASSET_COUNT; ++i)
    {
        normalize_slash(str_from[i]);
        normalize_slash(str_to[i]);
        if (is_file(str_from[i]) == ERR_SUCCESS)
            copy_file(str_from[i], str_to[i], "rb", "wb");
        else copy_dir(str_from[i], str_to[i], TRUE, "rb", "wb");

        if (*BUILD_ERR != ERR_SUCCESS)
            goto cleanup;
    }

    if (exec(&cmd, "build") != ERR_SUCCESS)
        goto cleanup;

    cmd_free();
    *BUILD_ERR = ERR_SUCCESS;
    return *BUILD_ERR;

cleanup:
    cmd_fail();
    return *BUILD_ERR;
}
