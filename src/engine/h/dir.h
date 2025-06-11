#ifndef MC_C_ENGINE_DIR_H
#define MC_C_ENGINE_DIR_H

#include <sys/stat.h>

#include "defines.h"
#include "memory.h"

#define DIR_MAX     64

/* ---- section: signatures ------------------------------------------------- */

b8 is_file_exists(const str *file_name);
b8 is_dir(const str *name);
b8 is_dir_exists(const str *dir_name);
str *get_file_contents(const str *file_name);
str_buf get_dir_contents(const str *dir_name);
str *get_path_absolute(const str *path);
str *get_path_bin_root(void);
int make_dir(str *path, u16 mode);
void check_slash(str *path);

#endif /* MC_C_ENGINE_DIR_H */
