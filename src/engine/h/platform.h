#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

#include "defines.h"

int make_dir(str *path);
str *_get_path_absolute(const str *path, str *path_real);
b8 _get_path_bin_root(str *buf);

#endif /* ENGINE_PLATFORM_H */
