#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "defines.h"

/* ---- section: signatures ------------------------------------------------- */

b8 is_file_exists(const str *file_name);
b8 is_dir(const str *name);
b8 is_dir_exists(const str *dir_name);
str *get_file_contents(const str *file_name);
str_buf get_dir_contents(const str *dir_name);
u64 get_dir_entry_count(const str *dir_name);
str *get_path_absolute(const str *path);
str *get_path_bin_root(void);
void check_slash(str *path);

#endif /* ENGINE_DIR_H */
