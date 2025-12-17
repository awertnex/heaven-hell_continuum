#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <engine/h/types.h>

/*! @brief get time for logic purposes, like for random number generators
 *  that require a seed.
 *
 *  get an unsigned 64-bit integer value based on unix time,
 *  but added nanoseconds of unix time.
 *
 *  @remark not accurate to actual time.
 */
u64 get_time_logic(void);

/*! @brief get time since process started in seconds and fractional milliseconds.
 */
f64 get_time_f64(void);

b8 get_timer(f64 *time_start, f32 interval);

#endif /* GAME_LOGIC_H */
