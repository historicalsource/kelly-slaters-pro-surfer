#include "global.h"

#include "manip_obj.h"
#include "physical_interface.h"
#include "wds.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "terrain.h"
// BIGCULL #include "damage_interface.h"
#include "hwaudio.h"

void manip_obj::init()
{
  if(!has_physical_ifc())
    create_physical_ifc();

  assert(has_physical_ifc());

  physical_ifc()->set_gravity(true);
  physical_ifc()->set_bouncy(true);

  physical_ifc()->set_mass(1.0f);
  physical_ifc()->set_volume(1.0f);

  physical_ifc()->set_bounce_factor(0.75f);
  physical_ifc()->set_slide_factor(0.75f);

  physical_ifc()->disable();

  sound_device::inst()->load_sound( "crate" );

//  g_world_ptr->create_preloaded_entity_or_subclass( "bcrateboom",  empty_string );
//  entity *ent = g_entity_maker->acquire_entity( "bcrateboom", ACTIVE_FLAG|NONSTATIC_FLAG );
//  ent->set_visible(false);
}


manip_obj::manip_obj( const entity_id& _id, unsigned int _flags )
  : entity( _id, _flags )
{
  init();
}

manip_obj::manip_obj( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor,
        unsigned int _flags )
  : entity(fs, _id, _flavor, _flags)
{
  init();
}

entity* manip_obj::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  manip_obj* newit = NEW manip_obj( _id, _flags );
  newit->copy_instance_data( *((manip_obj *)this) );
  return (entity*)newit;
}

void manip_obj::copy_instance_data( const manip_obj& b )
{
  entity::copy_instance_data(b);

  assert(has_physical_ifc());
}

void manip_obj::frame_advance(time_value_t t)
{
//  vector3d old_pos =  // unused, remove me?
  get_abs_position();
  entity::frame_advance(t);

  assert(has_physical_ifc());

  if(physical_ifc()->is_enabled())
  {
    bool smash_it = false;

    if(physical_ifc()->has_bounced())
    {
      physical_ifc()->set_flag(physical_interface::_PHYS_BOUNCED, false);
      smash_it = true;
    }

    entity *best_ent = NULL;
    region_node_pset::const_iterator rn = get_regions().begin();
    region_node_pset::const_iterator rn_end = get_regions().end();
    for ( ; best_ent == NULL && rn!=rn_end; ++rn )
    {
      region* r = (*rn)->get_data();
      vector<entity*>::const_iterator i = r->get_entities().begin();
      vector<entity*>::const_iterator i_end = r->get_entities().end();

      for ( ; best_ent == NULL && i!=i_end; ++i )
      {
        entity* e = *i;
#if defined(TARGET_XBOX)
        // wtf happened to damage_ifc ? JIV FIXME
        STUB( "manip_obj::frame_advance wtf happened to damage_ifc" );

        if ( e && e != this && !e->is_hero() && e->is_visible() && false)
#else
        if ( e && e != this && !e->is_hero() && e->is_visible() && e->has_damage_ifc() && e->damage_ifc()->is_alive())
#endif /* TARGET_XBOX JIV DEBUG */
        {
          vector3d delta = (e->get_abs_position() - get_abs_position());
          rational_t rad = (e->get_radius() + get_radius());

          if(delta.length2() <= rad*rad)
          {
            best_ent = e;
            break;
          }
        }
      }
    }

    if(best_ent != NULL || smash_it)
      smash(best_ent);
  }
}


void manip_obj::smash(entity *target)
{
  set_visible(false);
  set_collisions_active(false);
  physical_ifc()->disable();

  pstring snd("crate");
  sound_device::inst()->play_3d_sound( snd, get_abs_position() );

/*
    po the_po = po_identity_matrix;
    the_po.set_position(get_abs_position());

//    entity *fx = g_entity_maker->acquire_entity( "bcrateboom", ACTIVE_FLAG|NONSTATIC_FLAG );
    entity *fx = g_world_ptr->add_time_limited_effect( "bcrateboom", the_po, 0.5f );
    fx->set_created_entity_default_active_status();
    fx->set_rel_po(the_po);
    fx->update_abs_po();
    fx->frame_done();
    fx->force_regions(this);
    fx->set_visible(true);
    fx->set_active(true);
    fx->frame_advance(0);
//    g_world_ptr->make_time_limited(fx, 0.5f);
//*/

#if defined(TARGET_XBOX)
  STUB( "manip_obj::smash wtf happened to damage_ifc" );
  
  if(target && false)
#else
  if(target && target->has_damage_ifc() && target->damage_ifc()->is_alive())
#endif /* TARGET_XBOX JIV DEBUG */
  {
    vector3d delta = (target->get_abs_position() - get_abs_position());
    delta.normalize();
#if defined(TARGET_XBOX)
  STUB( "manip_obj::smash wtf happened to damage_ifc" );
#else
    target->damage_ifc()->apply_damage(this, 50, DAMAGE_DIRECT, get_abs_position(), delta);
#endif /* TARGET_XBOX JIV DEBUG */
  }
}
