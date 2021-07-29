/*

  generator.cpp

    This is a set of derived classes of force_generator, which are different ways of
    applying forces to entities.

*/
#include "global.h"

//!#include "character.h"
#include "generator.h"
//!#include "actuator.h"
//#include "physent.h"
#include "wds.h"
#include "osdevopts.h"
//!#include "rigid.h"
#include "hinge.h"
#include "physical_interface.h"
#include "time_interface.h"
//#include "projectile.h"
//!#include "attrib.h"
#include "shock_mods.h"
#include "terrain.h"

rational_t g_gravity = 9.8f;

//////////////////////////////////////////////////////////////////////////////// 
//  linear_force_generator
//////////////////////////////////////////////////////////////////////////////// 
linear_force_generator::linear_force_generator(entity * _ent,vector3d _dir) : force_generator()
  {
  ent = _ent;
  dir = _dir;
  }


//////////////////////////////////////////////////////////////////////////////// 
//  gravity_generator
//////////////////////////////////////////////////////////////////////////////// 

void gravity_generator::frame_advance(time_value_t time_inc)
{
  vector<entity *>::const_iterator ei;
	if(os_developer_options::inst()->get_gravity_enabled()){
//		for(int i = 0; i < g_world_ptr->get_num_entities(); i++){
//			if(g_world_ptr->is_entity_valid(i)){
  for (ei = g_world_ptr->get_active_entities().begin();ei<g_world_ptr->get_active_entities().end();ei++)
    {
    entity *ent = (*ei); //g_world_ptr->get_entity(i);
//    if ( ent && ent->is_flagged(EFLAG_PHYSICS) && ent->is_gravity() && !ent->is_stationary() )
    if ( ent && ent->has_physical_ifc() && ent->physical_ifc()->is_enabled() && ent->physical_ifc()->is_gravity() && !ent->is_stationary() )
      {
/*
#if defined(TARGET_PC)
          if (ent->get_flavor()==ENTITY_ENTITY)
            {
            stringx composite = stringx("Active, non-stationary entity: ") + ent->get_id().get_val();
            error(composite.c_str());
            }
#endif
*/
      vector3d force = YVEC * (-g_gravity * ent->physical_ifc()->get_mass() * (CALC_ENTITY_TIME_DILATION(time_inc, ent)) * ent->physical_ifc()->get_gravity_multiplier());
      ent->physical_ifc()->apply_force_increment(force, physical_interface::CONTINUOUS);
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////////////// 
//  launcher
//////////////////////////////////////////////////////////////////////////////// 
void launcher::frame_advance(time_value_t time_inc)
  {
  if (doit)
    {
    vector3d vel;
    rational_t impulse_accel = __fsqrt((*att_get_launch_height)(att)*g_gravity*2);
    if (os_developer_options::inst()->get_gravity_enabled())
      {
      ent->get_velocity(&vel);
      if (vel.y>0)
        impulse_accel -= vel.y;
      if (impulse_accel<0) impulse_accel = 0;
      }

      if(ent->has_physical_ifc())
      {
        rational_t impulse = impulse_accel*ent->physical_ifc()->get_mass();
        ent->physical_ifc()->apply_force_increment(dir*impulse, physical_interface::INSTANT);
      }
/*!    if( ent->get_flavor() == ENTITY_CHARACTER )
      {
      ((character*)ent)->play_sound_group(stringx("jump"));
      }
!*/
    }
  }
