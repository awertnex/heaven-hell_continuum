#ifndef MC_C_ENGINE_PLATFORM_H
#define MC_C_ENGINE_PLATFORM_H

#include "defines.h"

int make_dir(str *path);
str *_get_path_absolute(const str *path, str *path_real);
b8 _get_path_bin_root(str *buf);

#endif /*  MC_C_ENGINE_PLATFORM_H */
