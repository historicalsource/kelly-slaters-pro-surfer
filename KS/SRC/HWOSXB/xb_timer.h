#ifndef XB_TIMER_H
#define XB_TIMER_H
////////////////////////////////////////////////////////////////////////////////
/*
  xb_timer.h

  *extremely* high-resolution timer
  not as high as the actual processor clock, but high
  some old machines may not support, in which case we'll get crash city
*/
////////////////////////////////////////////////////////////////////////////////

#include "global.h"
#include "hwmath.h"
#include "singleton.h"
#include "types.h"	// For uint64

/*  #include <libpc.h> */

__inline uint64 volatile get_cpu_cycle(void)
{
    register LARGE_INTEGER result;

    BOOL succedded = QueryPerformanceCounter(&result);

    assert(succedded);

    if(succedded)
      return result.QuadPart;

    DWORD error = GetLastError();
    
    return 0;
}

// time_value_t measures seconds
typedef float time_value_t;
const time_value_t SMALL_TIME = (time_value_t)1e-5f;
const time_value_t LARGE_TIME = (time_value_t)1e25f;

/*  #define XB_CLOCK_SPEED 294912000 */
/*  #define XB_CLOCK_SPEED 600000000 */
extern const long XB_CLOCK_SPEED;
extern const u_int XB_TICK_RATE;
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

  uint64 elapsed();

public:
  master_clock();
 
  DECLARE_SINGLETON(master_clock)

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
  float mFreq;
  uint64 last_reset_ticks;
#ifdef FRAMERATE_LOCK
  static uint64 lock_ticks;
#endif

  float get_frequency( void );
public:
  hires_clock_t();

  void reset() 
  {
    last_reset_ticks = master_clock::inst()->elapsed();
  }

  time_value_t elapsed_and_reset();
  time_value_t elapsed() const;
#ifdef FRAMERATE_LOCK
  void wait_for_lock() const;
#endif
};
#endif
