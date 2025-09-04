#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "defines.h"

/* ---- section: signatures ------------------------------------------------- */

/*
 * return FALSE (0) on failure;
 */
b8 is_file_exists(const str *file_path);

/*
 * return FALSE (0) on failure;
 */
b8 is_dir(const str *path);

/*
 * return FALSE (0) on failure;
 */
b8 is_dir_exists(const str *dir_path);

/*
 * return calloc'd buf of file contents;
 * return file size in bytes + 1 to file_len;
 *
 * return NULL on failure;
 */
void *get_file_contents(const str *file_path, u64 *file_len, const str *format);

/*
 * return directory entries at dir_path;
 * return (buf){NULL} on failure;
 */
buf get_dir_contents(const str *dir_path);

u64 get_dir_entry_count(const str *dir_path);

str *get_path_absolute(const str *path);

/*
 * return calloc'd string of executable's path, slash and null terminated;
 * return NULL on failure;
 */
str *get_path_bin_root(void);

/*
 * append '/' onto path if '/' not present, null terminated;
 */
void check_slash(str *path);

#endif /* ENGINE_DIR_H */

