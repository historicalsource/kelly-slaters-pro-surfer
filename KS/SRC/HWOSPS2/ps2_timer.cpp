#include "global.h"
//#pragma hdrstop

#include "ostimer.h"

const u_int PS2_TICK_RATE = PS2_CLOCK_SPEED;	// >> 16;   // This now has ~.25ms accuracy.
const float SECONDS_PER_TICK = 1.0f/PS2_TICK_RATE;

DEFINE_SINGLETON(master_clock)

// use timeGetTime because QueryPerformanceCounter has problems on some machines

time_value_t game_clock::delta = 0.0F;
uint32 game_clock::frames = 0;
uint64 game_clock::ticks = 0;


void game_clock::frame_advance(time_value_t _delta) 
{
  ++frames;
  assert(_delta>0 && delta<10.0F);
  delta = _delta; 
  ticks += uint64((int)(delta*10000.0F));
}

#ifdef FRAMERATE_LOCK
uint64 hires_clock_t::lock_ticks = (17 * PS2_TICK_RATE) / 1000;	// keep us from alternating between 60 and 30 (dc 03/08/02)
#endif

