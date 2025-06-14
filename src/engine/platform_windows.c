#include "h/core.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/math.h"

int make_dir(str *path)
{
    return mkdir(path);
}

