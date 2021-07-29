// anim_flavor.cpp
#include "global.h"


#include "anim_flavor.h"


const chunk_flavor anim_track_flavors[] =
  {
#define MAC(a,b) chunk_flavor(#b),
#include "anim_flavors.h"
#undef MAC
  };


anim_track_flavor_t to_anim_track_flavor_t( const chunk_flavor& cf )
  {
  int i;
  for ( i=0; i<NUM_ANIM_TRACK_FLAVORS; i++ )
    {
    if ( cf == anim_track_flavors[i] )
      return (anim_track_flavor_t)i;
    }
  return UNDEFINED_TRACK;
  }
