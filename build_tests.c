#include "engine/build.c"

#define DIR_ROOT        "tests/"
#define DIR_SRC         "src/tests/"
#define ASSET_COUNT     3
#define NEW_DIR_COUNT   3

u32 *const BUILD_ERR = &engine_err;

#if PLATFORM_LINUX
    #define STR_OUT         DIR_ROOT"hhc"
#elif PLATFORM_WIN
    #define STR_OUT         "\""DIR_ROOT"hhc"EXE"\""
#endif /* PLATFORM */

str str_out_dir[CMD_SIZE] = {0}; /* bundle directory name */
buf str_tests = {NULL};
str str_main[CMD_SIZE] = {0};
str str_out[CMD_SIZE] = STR_OUT;

void build_test(char **argv);
void list(void);

b8
extension_evaluate(const str *file_name)
{
    u64 len = strlen(file_name);
    if (len > 2 && strncmp(file_name + len - 2, ".c", 2) == 0)
        return TRUE;
    return FALSE;
}

void
extension_strip(const str *file_name, str *dest)
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
list(void)
{
    printf("%s",
            "Options:\n"
            "    test [n]\n"
            "Tests:\n");

    str_tests = get_dir_contents(DIR_SRC);
    if (*BUILD_ERR != ERR_SUCCESS)
    {
        LOGERROR(*BUILD_ERR, "%s\n", "Fetching Tests Failed");
        cmd_fail();
    }

    str *str_tests_temp = NULL;
    if (mem_alloc((void*)&str_tests_temp, NAME_MAX,
                "list().str_tests_temp") != ERR_SUCCESS)
    {
        LOGERROR(*BUILD_ERR, "%s\n", "Fetching Tests Failed");
        cmd_fail();
    }

    u32 i = 0;
    for (; i < str_tests.memb; ++i)
    {
        if (!extension_evaluate(str_tests.i[i]))
            continue;

        extension_strip(str_tests.i[i], str_tests_temp);
        printf("    %03d: %s\n", i, str_tests_temp);
    }

    mem_free((void*)&str_tests_temp, NAME_MAX, "list().str_tests_temp");
    mem_free_buf(&str_tests, "list().str_tests");
    *BUILD_ERR = ERR_SUCCESS;
    exit(*BUILD_ERR);
}

void
build_test(char **argv)
{
    if (!argv[1])
    {
        LOGERROR(ERR_POINTER_NULL, "Usage: ./build%s test [n]\n", EXE);
        cmd_fail();
        _exit(0);
    }

    u32 i = 0;
    str temp[CMD_SIZE] = {0};
    u64 test_index = atoi(argv[1]);
    str_tests.memb = get_dir_entry_count(DIR_SRC);
    if (*BUILD_ERR != ERR_SUCCESS)
        cmd_fail();
    if (test_index >= str_tests.memb)
    {
        LOGERROR(FALSE, ERR_BUFFER_OVERFLOW,
                "'%s' Invalid, Try './build%s list' to List Available Options..\n",
                argv[1], EXE);
        cmd_fail();
    }
    str_tests = get_dir_contents(DIR_SRC);
    if (*BUILD_ERR != ERR_SUCCESS)
        cmd_fail();
    if (mem_alloc_buf(&cmd, CMD_MEMB, CMD_SIZE,
                "build_test().cmd") != ERR_SUCCESS)
        cmd_fail();

    for (i = 0; i < str_tests.memb; ++i)
        extension_strip(str_tests.i[i], str_tests.i[i]);

    snprintf(str_out_dir, CMD_SIZE, "%s%s%s/",
            str_build_root, DIR_ROOT, (str*)str_tests.i[test_index]);

    cmd_push(COMPILER);
    cmd_push(stringf(DIR_SRC"%s.c", str_tests.i[test_index]));
    cmd_push(stringf("-I%s", str_build_root));
    cmd_push("-std=c99");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-fno-builtin");
    cmd_push("-Wl,-rpath=$ORIGIN/lib/"PLATFORM);
    engine_link_libs();
    cmd_push("-o");
    cmd_push(stringf(DIR_ROOT"%s/%s"EXE,
                str_tests.i[test_index], str_tests.i[test_index]));
    cmd_ready();
}

int
main(int argc, char **argv)
{
    build_init(argc, argv, "build_tests.c", "build_tests"EXE);
    if (argv_compare("list", argc, argv)) list();
    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return *BUILD_ERR;

    build_test(argv);

    str str_mkdir[NEW_DIR_COUNT][CMD_SIZE] = {0};
    str str_from[ASSET_COUNT][CMD_SIZE] = {0};
    str str_to[ASSET_COUNT][CMD_SIZE] = {0};
    snprintf(str_mkdir[0],  CMD_SIZE, "%s", DIR_ROOT);
    snprintf(str_mkdir[1],  CMD_SIZE, "%s", str_out_dir);
    snprintf(str_mkdir[2],  CMD_SIZE, "%slib/", str_out_dir);
    snprintf(str_from[0],   CMD_SIZE, "%sLICENSE", str_build_root);
    snprintf(str_from[1],   CMD_SIZE, "%sengine/lib/"PLATFORM, str_build_root);
    snprintf(str_from[2],   CMD_SIZE, "%sresources/", str_build_root);
    snprintf(str_to[0],     CMD_SIZE, "%sLICENSE", str_out_dir);
    snprintf(str_to[1],     CMD_SIZE, "%slib/"PLATFORM, str_out_dir);
    snprintf(str_to[2],     CMD_SIZE, "%sresources/", str_out_dir);

    u32 i = 0;
    for (; i < NEW_DIR_COUNT; ++i)
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
