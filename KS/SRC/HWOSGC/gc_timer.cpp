#include "global.h"

#include "ostimer.h"

DEFINE_SINGLETON( master_clock )

time_value_t game_clock::delta = 0.0f;
uint32 game_clock::frames = 0;
uint64 game_clock::ticks = 0;

void game_clock::frame_advance( time_value_t _delta )
{
  ++frames;
  assert( _delta > 0 && delta < 10.0f );
  delta = _delta;
  ticks += (uint64) ( delta * 10000.0f );
}
