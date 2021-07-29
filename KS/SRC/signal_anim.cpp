// signal_anim.cpp
#include "global.h"

#include "signal_anim.h"
#include "debug.h"

//!#include "character.h"
#include "camera.h"
#include "marker.h"
#include "mic.h"
#include "particle.h"
//!#include "rigid.h"
//!#include "limb.h"
#include "item.h"
//#include "projectile.h"
#include "beam.h"
#include "conglom.h"
//!#include "ladder.h"
#include "light.h"
#include "profiler.h"
#include "entity.h"


#if !defined(NO_SERIAL_IN)
void signal_track::internal_serial_in( chunk_file& fs )
  {
  chunk_flavor cf;
  entity_flavor_t entity_flavor = IGNORE_FLAVOR;

  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
    {
    if( cf == chunk_flavor("keys") )
      {
      int keys, cn;

      serial_in( fs, &keys );
      num_signals = 0;
      signals = NEW signal_key[ keys ];

      for( cn = 0; cn < keys; cn++ )
        {
        time_value_t t;
        stringx s;

        serial_in( fs, &t );
        serial_in( fs, &s );

        assert( entity_flavor != IGNORE_FLAVOR );
        add_key( t, entity_flavor, s );
        }
      }

    if( cf == chunk_flavor("entity") )
      {
      stringx s;
      int cn;

      // read entity type
      //
      serial_in( fs, &s );

      for( cn = 0; cn < (int)NUM_ENTITY_FLAVORS; cn++ )
        {
        if( entity_flavor_names[cn] == s )
          {
          entity_flavor = (entity_flavor_t)cn;
          break;
          }
        }

        if( cn >= NUM_ENTITY_FLAVORS )
          error( fs.get_name() + ": unknown entity type in signal_track : " + s );
      }
    }
  }
#endif

#if !defined(NO_SERIAL_OUT)
void signal_track::internal_serial_out( chunk_file& fs ) const
  {
    // see signal_trk.cpp in tools
  }
#endif

void signal_track::add_key( time_value_t timestamp, entity_flavor_t entity_flavor, stringx signaldata )
{
  signal_id_t s = 0;
  int idx = 0;
  signal_key* key_pt = 0;

  while( 1 )
  {
    const char *currname = signaldata.c_str();
    int curridx;

    // skip
    while( currname[idx] == '\n' || currname[idx] == '\r' )
      idx++;

    curridx = idx;

    // search end or end of line
    while( currname[idx] && currname[idx] != '\n' && currname[idx] != '\r' )
      idx++;

    stringx oneline = signaldata.substr( curridx, idx-curridx );

    if( oneline.size() )
    {
      switch( entity_flavor )
      {
/*!        case ENTITY_ACTOR:
          s = actor::get_signal_id( oneline.c_str() );
          break;
!*/
        case ENTITY_CAMERA:
          s = camera::get_signal_id( oneline.c_str() );
          break;
/*!
        case ENTITY_CHARACTER:
          s = character::get_signal_id( oneline.c_str() );
          break;
!*/
        case ENTITY_ENTITY:
          s = entity::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_MARKER:
          s = marker::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_MIC:
          s = mic::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_LIGHT_SOURCE:
          s = light_source::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_PARTICLE_GENERATOR:
          s = particle_generator::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_ITEM:
          s = item::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_CONGLOMERATE:
          s = conglomerate::get_signal_id( oneline.c_str() );
          break;
        case ENTITY_BEAM:
          s = beam::get_signal_id( oneline.c_str() );
          break;

        case ENTITY_PHYSICAL:
        case ENTITY_MOBILE:
        case ENTITY_LIGHT:
          s = (unsigned short)-1;
          break;
        default:
          break;
      }

      if( s == (signal_id_t)-1 )
      {
        stringx name( currname );
        error( "Unknown signal " + name + " for entity " + entity_flavor_names[entity_flavor] );
      }

      if( !key_pt )
        key_pt = NEW signal_key( timestamp );

      key_pt->add_value( s );
    }

    if( !currname[idx] )
      break;
  }

  if( key_pt )
  {
    signals[ num_signals ] = *key_pt;
    num_signals++;
    delete key_pt; // PEH BETA LOCK this line
  }
}

signal_anim::signal_anim()
: anim<signal_key>()
{
}

signal_anim::~signal_anim()
{
}

void signal_anim::construct( const signal_track& track,
                             unsigned short anim_flags )
{
  anim<signal_key>::construct( anim_flags );
  signals.resize(0);
  for( int i=0; i< track.num_signals; i++ )
  {
    signals.push_back( track.signals[i] );
  }
  last_get_time = 0;
}

void signal_anim::set_time( time_value_t t )
{
  anim<signal_key>::set_time( t );
}

void signal_anim::frame_advance( const anim_control_t& ac, vector<signal_id_t>* dest )
{
  anim<signal_key>::frame_advance( ac, NULL );

  dest->resize(0);

  // return list of signals for time range between now and last call
  time_value_t curtime = ac.get_time();
  vector<signal_key>::const_iterator i = signals.begin();
  vector<signal_key>::const_iterator i_end = signals.end();
  for ( ; i!=i_end; ++i )
  {
    time_value_t sig_time = (*i).get_time();
    bool add;
    if ( ac.is_reverse() )
    {
      if ( ( last_get_time>=curtime && (sig_time<curtime || sig_time>=last_get_time) )
        || ( last_get_time<curtime && sig_time>last_get_time && sig_time<=curtime )
        )
      {
        add = false;
      }
      else
        add = true;
    }
    else
    {
      if ( ( last_get_time>=curtime && (sig_time<curtime || sig_time>=last_get_time) )
        || ( last_get_time<curtime && sig_time>last_get_time && sig_time<=curtime )
        )
      {
        add = true;
      }
      else
        add = false;
    }
    if ( add )
    {
      unsigned signal_flags = (*i).get_value();
      for( int i=0;i<32;i++ )
      {
        if( signal_flags & (1<<i) )
          dest->push_back( i );
      }
/*
      vector<signal_id_t>::const_iterator si = sigs.begin();
      vector<signal_id_t>::const_iterator si_end = sigs.end();
      for ( ; si!=si_end; ++si )
        dest->push_back( *si );*/
    }
  }

  last_get_time = curtime;
}
