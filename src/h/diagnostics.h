#ifndef GAME_DIAGNOSTICS_H
#define GAME_DIAGNOSTICS_H

#include <engine/h/defines.h>
#include <engine/h/diagnostics.h>
#include <engine/h/limits.h>

#define GAME_ERR_OFFSET 512

enum GameErrorCodes
{
    ERR_MODE_INTERNAL_DEBUG_DISABLE = GAME_ERR_OFFSET,
    ERR_MODE_INTERNAL_COLLIDE_DISABLE,
    ERR_WORLD_EXISTS,
    ERR_WORLD_CREATION_FAIL,
}; /* GameErrorCodes */

#endif /* GAME_DIAGNOSTICS_H */
