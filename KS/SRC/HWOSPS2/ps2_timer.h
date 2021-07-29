#ifndef PS2_TIMER_H
#define PS2_TIMER_H
////////////////////////////////////////////////////////////////////////////////
/*
  ps2_timer.h

  *extremely* high-resolution timer
  not as high as the actual processor clock, but high
  some old machines may not support, in which case we'll get crash city
*/
////////////////////////////////////////////////////////////////////////////////

#include "hwmath.h"
#include "singleton.h"

#include <libpc.h>

__inline unsigned int volatile get_cpu_cycle(void)
{
    register int result;

    __asm__ volatile ("mfc0 %0,$9" : "=r" (result));

    return(result);
}

// time_value_t measures seconds
typedef rational_t time_value_t;
const time_value_t SMALL_TIME = (time_value_t)1e-5f;
const time_value_t LARGE_TIME = (time_value_t)1e25f;

#define PS2_CLOCK_SPEED 294912000

//#define FRAMERATE_LOCK	// This is pretty unreliable.  Using nglSetFrameLock instead (dc 03/28/02)

extern const u_int PS2_TICK_RATE;
extern const float SECONDS_PER_TICK;

// this class keeps track of real time only
class master_clock : public singleton
{
private:
  union 
  {
    uint64 total;
    struct 
    {
      uint32 lower;
      uint32 upper;
    } half;
  } ticks;

  uint64 elapsed()
  {
    uint32 curticks = get_cpu_cycle();

    // check for wrap around
    if (curticks < ticks.half.lower) 
      ++ticks.half.upper;

    ticks.half.lower = curticks;

    // returns time since last reset.
    return ticks.total;	// >> 16;
  }

public:
  master_clock()
  {
    ticks.total = get_cpu_cycle();
  }
 
  DECLARE_SINGLETON(master_clock)

  // called by app::tick - must happen so that the master clock (upon which all hires_clocks are
  // based) stays up to date.  Otherwise the limited size of a uint32 would wrap the clocks
  // around every 14 seconds. 
  void tick() 
  {
    elapsed();
  }

  friend class hires_clock_t;
  friend class Random;
};

// in app.cpp
extern bool g_master_clock_is_up;

class hires_clock_t 
{
  uint64 last_reset_ticks;
#ifdef FRAMERATE_LOCK
  static uint64 lock_ticks;
#endif

public:
  void reset() 
  {
    last_reset_ticks = master_clock::inst()->elapsed();
  }

  hires_clock_t()
  {
    if (g_master_clock_is_up == true)
      reset();
    else
      last_reset_ticks = 0;
  }

  time_value_t elapsed_and_reset()
  {
#ifdef FRAMERATE_LOCK
	wait_for_lock();
#endif
    uint64 cur_ticks = master_clock::inst()->elapsed();
    time_value_t el = (cur_ticks - last_reset_ticks) * SECONDS_PER_TICK;
    last_reset_ticks = cur_ticks;
    return el;
  }

  time_value_t elapsed() const
  {
    int ticks = master_clock::inst()->elapsed() - last_reset_ticks;
    return (float)ticks * SECONDS_PER_TICK;
  }

#ifdef FRAMERATE_LOCK
  // Wait until at least lock_ticks have expired since last reset
  void wait_for_lock() const 
  {
	  uint64 cur_ticks;
	  
	  do {
		  cur_ticks = master_clock::inst()->elapsed();
	  } while (cur_ticks - last_reset_ticks < lock_ticks);
  }
#endif
};
#endif
