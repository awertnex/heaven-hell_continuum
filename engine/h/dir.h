#ifndef ENGINE_DIR_H
#define ENGINE_DIR_H

#include "defines.h"

/* return file type to file_type.
 *
 * return 0 on failure and engine_err is set accordingly */
u64 get_file_type(const str *path_type);

/* return non-zero on failure and engine_err is set accordingly */
u32 is_file(const str *path);

/* log = enable/disable logging.
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 is_file_exists(const str *path, b8 log);

/* return non-zero on failure and engine_err is set accordingly */
u32 is_dir(const str *path);

/* log = enable/disable logging.
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 is_dir_exists(const str *path, b8 log);

/* return calloc'd buf of file contents, not NULL terminated,
 * return file size in bytes to file_len.
 *
 * read_format = read file at path in specified format (fopen() parameter).
 *
 * return NULL on failure and engine_err is set accordingly */
str *get_file_contents(const str *path, u64 size, u64 *file_len,
        const str *format);

/* return directory entries at dir_path.
 *
 * return (buf){NULL} on failure and engine_err is set accordingly */
buf get_dir_contents(const str *path);

/* return dir entry count at path.
 *
 * engine_err is set accordingly on failure */
u64 get_dir_entry_count(const str *path);

/* copy file at path into file/directory at destination.
 *
 * read_format = read file at path in specified format (fopen() parameter),
 * write_format = write new file at destination in specified format
 * (fopen() parameter).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 copy_file(const str *path, const str *destination,
        const str *read_format, const str *write_format);

/* copy directory at path into directory at destination.
 *
 * overwrite TRUE will copy contents at path
 * into directory at destination,
 * overwrite FALSE will copy directory at path itself
 * into directory at destination,
 *
 * read_format = read files within path in specified format
 * (fopen() parameter),
 * write_format = write new files at destination in specified format
 * (fopen() parameter).
 *
 * return non-zero on failure and engine_err is set accordingly */
u32 copy_dir(const str *path, const str *destination, b8 overwrite,
        const str *read_format, const str *write_format);

/* return non-zero on failure and engine_err is set accordingly */
u32 write_file(const str *path, u64 size, u64 length, void *buf,
        const str *write_format, b8 log);

/* return NULL on failure and engine_err is set accordingly */
str *get_path_absolute(const str *path);

/* return calloc'd string of executable's path, slash and null terminated.
 *
 * return NULL on failure and engine_err is set accordingly */
str *get_path_bin_root(void);

/* append '/' onto path if path not ending in '/', null terminated.
 *
 * on failure, engine_err is set accordingly */
void check_slash(str *path);

/* normalize all slashes to '/' or '\' based on operating system.
 *
 * on failure, engine_err is set accordingly */
void normalize_slash(str *path);

/* change all '\\' to '\'.
 *
 * on failure, engine_err is set accordingly */
void posix_slash(str *path);

/* return path retracted to its parent directory.
 *
 * on failure, engine_err is set accordingly */
str *retract_path(str *path);

#endif /* ENGINE_DIR_H */
