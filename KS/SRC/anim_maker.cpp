// anim_maker.cpp :: now contains functions for creating anims
//                   since we now want to be able to have animations
//                   outside the wds

#include "global.h"

#include "anim_maker.h"
#include "entity.h"
#include "entity_anim.h"
#include "wds.h"
#include "entity_maker.h"
#include "widget_entity.h"

#if defined(TARGET_XBOX)
#include "profiler.h"
#endif // TARGET_XBOX JIV DEBUG

#include <cstdarg>


extern world_dynamics_system *g_world_ptr;


anim_maker::anim_maker()
{
}


anim_maker::~anim_maker()
{
}



// Construct a NEW anim from the given track, add it to the world (if not part
// of a widget) and attach it to its entities.
// NOTE: the track is indicated by filename, but needs to be loaded into the
// entity track bank (see entity_anim.h) before calling this function.
entity_anim_tree* anim_maker::create_anim( const stringx& filename,
                                                   entity* ent,
                                                   unsigned short flags,
                                                   time_value_t start_time,
                                                   int priority,
                                                   short loop,
                                                   entity_widget *owning_widget )
{
  assert( ent );
  entity_track_tree* track = find_entity_track( filename );
  if ( track == NULL )
  {
    // track not loaded
    error( filename + ": must load anim before playing" );
  }
  return create_anim( filename, *track, ent, flags, start_time, priority, loop, owning_widget );
}


entity_anim_tree* anim_maker::create_anim( const stringx& name,
                                           const entity_track_tree& track,
                                           entity* ent,
                                           unsigned short flags,
                                           time_value_t start_time,
                                           int priority,
                                           short loop,
                                           entity_widget *owning_widget )
{
 	START_PROF_TIMER( proftimer_create_anim1 );
  assert( ent );
  entity_anim_tree* new_anim = NULL;
  if ( flags & ANIM_REVERSE )
    start_time = track.get_duration() - start_time;

  new_anim = NEW entity_anim_tree( name, ent, track, flags, start_time, priority, loop );

  if ( !owning_widget )
  {
    g_world_ptr->add_anim( new_anim );
  }
  else
  {
    owning_widget->add_anim( new_anim );
		assert(0);
  }
 	STOP_PROF_TIMER( proftimer_create_anim1 );
  return new_anim;
}


bool alwaysrebuildanims=true;

// reconstruct the given anim tree using the given data
void anim_maker::create_anim( entity_anim_tree* cached_anim,
                              const stringx& name,
                              const entity_track_tree& track,
                              unsigned short flags,
                              time_value_t start_time,
                              int priority,
                              short loop,
                              entity_widget *owning_widget )
{
  assert( cached_anim && cached_anim->get_entity() );

 	START_PROF_TIMER( proftimer_create_anim2 );
  if ( flags & ANIM_REVERSE )
    start_time = track.get_duration() - start_time;

	if ( alwaysrebuildanims )
  	cached_anim->construct( name, track, flags, start_time, priority, loop );

  if ( !owning_widget )
  {
    g_world_ptr->add_anim( cached_anim );
  }
  else
  {
    owning_widget->add_anim( cached_anim );
		assert(0);
  }
 	STOP_PROF_TIMER( proftimer_create_anim2 );
}



entity_anim_tree* anim_maker::create_anim( const stringx& name,
                                           const entity_track_tree& _tracka,
                                           const entity_track_tree& _trackb,
                                           rational_t blenda,
                                           rational_t blendb,
                                           entity* ent,
                                           unsigned short flags,
                                           time_value_t start_time,
                                           int priority,
                                           short loop,
                                           entity_widget *owning_widget )
{
 	START_PROF_TIMER( proftimer_create_anim3 );
  assert( ent );
#if defined(EVAN) && defined(PROJECT_KELLYSLATER)
	assert(false);
#endif
  entity_anim_tree* new_anim = NULL;
  if ( flags & ANIM_REVERSE )
    start_time = _tracka.get_duration() - start_time;

  new_anim = NEW entity_anim_tree( name, ent, _tracka, _trackb, blenda, blendb, flags, start_time, priority, loop );

  if ( !owning_widget )
  {
    g_world_ptr->add_anim( new_anim );
  }
  else
  {
    owning_widget->add_anim( new_anim );
		assert(0);
  }
 	STOP_PROF_TIMER( proftimer_create_anim3 );
  return new_anim;
}


// reconstruct the given anim tree using the given data
void anim_maker::create_anim( entity_anim_tree* cached_anim,
                              const stringx& name,
                              const entity_track_tree& _tracka,
                              const entity_track_tree& _trackb,
                              rational_t blenda,
                              rational_t blendb,
                              unsigned short flags,
                              time_value_t start_time,
                              int priority,
                              short loop,
                              entity_widget *owning_widget )
{
 	START_PROF_TIMER( proftimer_create_anim4 );
  assert( cached_anim && cached_anim->get_entity() );
#if defined(EVAN) && defined(PROJECT_KELLYSLATER)
	assert(false);
#endif

  if ( flags & ANIM_REVERSE )
    start_time = _tracka.get_duration() - start_time;

  cached_anim->construct( name, _tracka, _trackb, blenda, blendb, flags, start_time, priority, loop );

  if ( !owning_widget )
  {
    g_world_ptr->add_anim( cached_anim );
  }
  else
  {
    owning_widget->add_anim( cached_anim );
		assert(0);
  }
 	STOP_PROF_TIMER( proftimer_create_anim4 );
}
