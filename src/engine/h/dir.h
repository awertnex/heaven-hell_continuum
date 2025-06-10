#ifndef MC_C_ENGINE_DIR_H
#define MC_C_ENGINE_DIR_H

#include "defines.h"

/* ---- from linux/limits.h ------------------------------------------------- */
#ifndef NAME_MAX
    #define NAME_MAX    255
#endif /* NAME_MAX */
#ifndef PATH_MAX
    #define PATH_MAX    4096
#endif /* PATH_MAX */

/* ---- signatures ---------------------------------------------------------- */
b8 is_file_exists(const str *file_name);
b8 is_dir_exists(const str *dir_name);
str *get_file_contents(const str *file_name);
str **get_dir_contents(const str *dir_name, u64 *count);
str *get_path_absolute(const str *path);

#endif /* MC_C_ENGINE_DIR_H */
