#include "global.h"
//#pragma hdrstop

#include "ostimer.h"

DEFINE_SINGLETON(master_clock)

// use timeGetTime because QueryPerformanceCounter has problems on some machines

//  const long XB_CLOCK_SPEED = 294912000;
/*	Superseded by actual queried values (dc 03/07/02)
const long XB_CLOCK_SPEED = 733000000L;
const u_int XB_TICK_RATE = XB_CLOCK_SPEED>>16;   // This now has ~.25ms accuracy.
const float SECONDS_PER_TICK = 1.0f/XB_TICK_RATE;
*/
  
time_value_t game_clock::delta = 0.0F;
uint32 game_clock::frames = 0;
uint64 game_clock::ticks = 0;


master_clock::master_clock()
{
  ticks.total = get_cpu_cycle();
}

void game_clock::frame_advance(time_value_t _delta) 
{
  ++frames;
  assert(_delta>0 && delta<10.0F);
  delta = _delta; 
  ticks += uint64(delta*10000.0F);
}

uint64 master_clock::elapsed( void )
{
  ticks.total = get_cpu_cycle();

  return uint64(ticks.total);
}

#ifdef FRAMERATE_LOCK
uint64 hires_clock_t::lock_ticks = 0;
#endif

float hires_clock_t::get_frequency( void )
{
  LARGE_INTEGER counts_per_sec;
  
  BOOL noErr = QueryPerformanceFrequency( &counts_per_sec );

  assert( noErr );

#ifdef FRAMERATE_LOCK
  lock_ticks = (17 * counts_per_sec.QuadPart) / 1000;	// keep us from alternating between 60 and 30 (dc 03/08/02)
#endif

  // system clock speed in msec / cycle
  return (float) (1.0 / double( counts_per_sec.QuadPart ));
}

hires_clock_t::hires_clock_t()
{
  mFreq = 0.0f;

  if (g_master_clock_is_up == true)
    reset();
  else
    last_reset_ticks = 0;

  mFreq = get_frequency();
}

time_value_t hires_clock_t::elapsed() const
{
  uint64 ticks = master_clock::inst()->elapsed() - last_reset_ticks;
  return ((float)ticks) * mFreq;
}

time_value_t hires_clock_t::elapsed_and_reset(void)
{
  const time_value_t small_time = 0.00001f;
  const time_value_t big_time = 1.0000f;  

#ifdef FRAMERATE_LOCK
  wait_for_lock();
#endif

  uint64 cur_ticks = master_clock::inst()->elapsed();

  assert( cur_ticks );

  if((cur_ticks - last_reset_ticks) == 0)
  {
    return small_time;
  }

  assert( cur_ticks - last_reset_ticks );

  float el = (cur_ticks - last_reset_ticks) * mFreq;

  last_reset_ticks = cur_ticks;

  if( el >= big_time )
  {
    el = big_time;
  }
  else if ( el <= small_time )
  {
    el = small_time;
  }

  return el;
}

#ifdef FRAMERATE_LOCK
// Wait until at least lock_ticks have expired since last reset
void hires_clock_t::wait_for_lock() const 
{
  uint64 cur_ticks;

  do {
    cur_ticks = master_clock::inst()->elapsed();
  } while (cur_ticks - last_reset_ticks < lock_ticks);
}
#endif
