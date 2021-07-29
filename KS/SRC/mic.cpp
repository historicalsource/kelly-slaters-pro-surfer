////////////////////////////////////////////////////////////////////////////////
/*
  mic

  I got two turntables and a microphone.

  It's an entity so we can move it around using the same engine
  as everything else.  Which is very beautiful.
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "mic.h"

#include "hwaudio.h"
#include "game.h"
extern vector3d up_vector;

mic::mic( entity* _parent, 
          const entity_id& _entity_id, 
          entity_flavor_t _flavor ) :
    entity(_entity_id,_flavor)
  {
  link_ifc()->set_parent( _parent );
  }

void mic::frame_advance(time_value_t t)
  {
  last_position = get_abs_position();
  entity::frame_advance(t);
  }

void mic::adjust_listener(void)
  {
  #ifdef GCCULL
  sound_device::inst()->set_listener_position( get_abs_position() );
  vector3d temp;
  get_velocity(&temp);
  sound_device::inst()->set_listener_velocity( temp );

//  vector3d my_up = get_abs_po().non_affine_slow_xform(up_vector);
  vector3d my_up = get_abs_po().fast_8byte_non_affine_xform(up_vector);

	get_direction(&temp);
  sound_device::inst()->set_listener_orientation( temp, my_up );
  #endif
  }
