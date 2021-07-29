#ifndef PS2_TIMER_H
#define PS2_TIMER_H

#include "hwmath.h"
#include "singleton.h"

#include <dolphin/os.h>

// time_value_t measures seconds
typedef rational_t time_value_t;
const time_value_t SMALL_TIME = (time_value_t) 1e-5f;
const time_value_t LARGE_TIME = (time_value_t) 1e25f;

#define SECONDS_PER_TICK ( 1./(float)OS_TIMER_CLOCK )

// this class keeps track of real time only
class master_clock : public singleton
{
private:
	uint64 ticks;

  uint64 elapsed( void )
  {
    ticks = OSGetTime( );

		return ticks;
  }

public:
  master_clock( )
  {
    ticks = OSGetTime( );
  }
 
  DECLARE_SINGLETON( master_clock )

  void tick( void )
  {
    elapsed( );
  }

  friend class Random;
  friend class hires_clock_t;
};

// in app.cpp
extern bool g_master_clock_is_up;

class hires_clock_t 
{
  uint64 last_reset_ticks;

public:
  hires_clock_t( )
  {

    if( g_master_clock_is_up == true ) {
      reset( );
    } else {
      last_reset_ticks = 0;
    }

  }

  time_value_t elapsed( void ) const
  {
    uint64 ticks = master_clock::inst()->elapsed( ) - last_reset_ticks;
    time_value_t r = ticks * SECONDS_PER_TICK;
    
    return r;
  }
  
  void reset( void )
  {
    last_reset_ticks = master_clock::inst()->elapsed( );
  }

  time_value_t elapsed_and_reset( void )
  {
    uint64 cur_ticks = master_clock::inst()->elapsed( );
    time_value_t el = ( cur_ticks - last_reset_ticks ) * SECONDS_PER_TICK;
    last_reset_ticks = cur_ticks;
    
    return el;
  }

};

#endif
