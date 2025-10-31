#ifndef ENGINE_BUILD_H
#define ENGINE_BUILD_H

#include "defines.h"
#include "diagnostics.h"
#include "platform.h"

#define COMPILER        "gcc"EXE
#define CMD_MEMB        64
#define CMD_SIZE        512

enum BuildFlag
{
    FLAG_CMD_SHOW = 0x0001,
    FLAG_CMD_RAW =  0x0002,
}; /* BuildFlag */

/* can force-terminate process.
 *
 * build_src_name = build source file that's using this header,
 * build_bin_name = build binary file that's using this header,
 * including extension if needed */
void build_init(int argc, char **argv,
        const str *build_src_name, const str *build_bin_name);

/* can force-terminate process.
 * must not be called before function 'build_init()'.
 *
 * engine_dir = engine source dir,
 * out_dir = where to place the engine compiled library.
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 engine_build(const str *engine_dir, const str *out_dir);

/* link engine's dependencies with software */
void engine_link_libs(void);

u64 argv_compare(str *arg, int argc, str **argv);
b8 extension_evaluate(const str *file_name);
void extension_strip(const str *file_name, str *dest);
void cmd_push(const str *string);

/* finalize build command for execution */
void cmd_ready(void);

void cmd_free(void);

/* can force-terminate process */
void cmd_fail(void);

extern str *str_build_root;
extern buf cmd;

#endif /* ENGINE_BUILD_H */
