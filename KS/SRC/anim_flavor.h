#ifndef _ANIM_FLAVOR_H
#define _ANIM_FLAVOR_H


#include "chunkfile.h"
#include "oserrmsg.h"


enum anim_track_flavor_t
  {
  UNDEFINED_TRACK = -1,
#define MAC(a,b) a,
#include "anim_flavors.h"
#undef MAC
  NUM_ANIM_TRACK_FLAVORS
  };


extern const chunk_flavor anim_track_flavors[];


anim_track_flavor_t to_anim_track_flavor_t( const chunk_flavor& cf );


#if !defined(NO_SERIAL_IN)
inline void serial_in( chunk_file& fs, anim_track_flavor_t* flavor )
  {
  chunk_flavor cf;
  serial_in( fs, &cf );
  *flavor = to_anim_track_flavor_t( cf );
  if ( *flavor == UNDEFINED_TRACK )
    error( (fs.get_name() + ": bad anim track flavor").c_str() );
  }
#endif

#if !defined(NO_SERIAL_OUT)
inline void serial_out( chunk_file& fs, const anim_track_flavor_t& flavor )
  {
  serial_out( fs, anim_track_flavors[flavor] );
  }
#endif


#endif  // _ANIM_FLAVOR_H
