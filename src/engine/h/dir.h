#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "defines.h"

/* ---- section: signatures ------------------------------------------------- */

/* return file type.
 *
 * return 0 on failure */
u64 get_file_type(const str *path);

/* return FALSE (0) on failure */
b8 is_file(const str *path);

/* return FALSE (0) on failure */
b8 is_file_exists(const str *path);

/* return FALSE (0) on failure */
b8 is_dir(const str *path);

/* return FALSE (0) on failure */
b8 is_dir_exists(const str *path);

/* copy file at path into file/directory at destination.
 *
 * return non-zero on failure */
u8 copy_file(const str *path, const str *destination);

/* copy directory at path into directory at destination.
 *
 * overwrite TRUE will copy contents at path
 * into directory at destination,
 * overwrite FALSE will copy directory at path itself
 * into directory at destination.
 *
 * return non-zero on failure */
u8 copy_dir(const str *path, const str *destination, b8 overwrite);

/* return calloc'd buf of file contents,
 * return file size in bytes + 1 to file_len.
 *
 * return NULL on failure */
void *get_file_contents(const str *path, u64 *file_len, const str *format);

/* return directory entries at dir_path.
 *
 * return (buf){NULL} on failure */
buf get_dir_contents(const str *path);

u64 get_dir_entry_count(const str *path);

str *get_path_absolute(const str *path);

/* return calloc'd string of executable's path, slash and null terminated.
 * return NULL on failure */
str *get_path_bin_root(void);

/* append '/' onto path if path not ending in '/', null terminated */
void check_slash(str *path);

/* normalize all slashes to '/' or '\' based on operating system */
void normalize_slash(str *path);

/* change all '\\' to '\' */
void posix_slash(str *path);

/* retract path to its parent directory.
 *
 * return path */
str *retract_path(str *path);

#endif /* ENGINE_DIR_H */
