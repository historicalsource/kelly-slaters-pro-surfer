#ifndef OSTIMER_H
#define OSTIMER_H
////////////////////////////////////////////////////////////////////////////////
/*
  ostimer.h

  *extremely* high-resolution timer
  not as high as the actual processor clock, but high
*/
////////////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PC)
#include "hwospc\w32_timer.h"
#elif defined(TARGET_MKS)
#include "hwosmks\set5_timer.h"
#elif defined(TARGET_PS2)
#include "hwosps2\ps2_timer.h"
#elif defined(TARGET_NULL)
#include "hwosnull\null_timer.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_timer.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_timer.h"
#endif


// this is the interface for the main game to provide game_clock_t with
// frame advances.  Should use game_clock_t to actually access this
// info, unless a single frame delta since last frame is sufficient.
class game_clock
{
  friend class game_clock_t;

  static time_value_t delta;
#if !defined(TARGET_PS2) && !defined(TARGET_XBOX) && !defined(TARGET_GC)
  static uint32 ticks;
#else
  static uint64 ticks;
#endif
  static uint32 frames;

public:
  static void frame_advance(time_value_t _delta);
  static inline time_value_t get_delta_t()
  {
    return delta;
  }
  static inline uint32 get_total_frames()
  {
    return frames;
  }
};


// this class provides a similar interface to hires_clock_t, measuring
// elapsed game time instead of system time.
class game_clock_t
{
  uint32 ticks;
public:
  game_clock_t()
  {
    ticks = game_clock::ticks;
  }
  void reset() // records time, future calls to elapsed are measured against.
  {
    ticks = game_clock::ticks;
  }
  time_value_t elapsed_and_reset()  // returns time since last reset, and resets.
  {
    time_value_t elapsed = (game_clock::ticks - ticks)*0.0001F;
    ticks = game_clock::ticks;
    return elapsed;
  }
  time_value_t elapsed() const  // returns time since last reset.
  {
    return (game_clock::ticks - ticks)*0.0001F;
  }
};


#endif // OSTIMER_H
