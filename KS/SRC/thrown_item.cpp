#include "global.h"

#include "osalloc.h"

//!#include "character.h"
#include "stringx.h"
#include "item.h"
//!#include "character.h"
#include "wds.h"
#include "msgboard.h"
#include "terrain.h"
#include "hwaudio.h"
#include "script_object.h"
//!#include "attrib.h"
#include "vm_thread.h"
//!#include "character.h"
#include "beam.h"
#include "particle.h"
// BIGCULL #include "thrown_item.h"
#include "collide.h"
#include "osdevopts.h"
//#include "dread_net.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "switch_obj.h"
#include "physical_interface.h"
// BIGCULL #include "damage_interface.h"
#include "sound_interface.h"
#include "script_access.h"

#if defined(TARGET_XBOX)
#include "handheld_item.h"
#include "thrown_item.h"
#include "profiler.h"
#endif /* TARGET_XBOX JIV DEBUG */

pstring explode_sound("EXPLODE");
pstring throw_sound("THROW");
pstring bounce_sound("BOUNCE");
pstring arm_sound("ARM");
pstring grenade_loop_sound("GRENADE_LOOP");
pstring arm_loop_sound("ARM_LOOP");


void thrown_item_remove_live_grenade(signaller* sig, const char *data)
{
  assert(((grenade *)sig)->get_item_owner());

  thrown_item *itm = ((grenade *)sig)->get_item_owner();

  vector<grenade *>::iterator i = itm->live_grenades.begin();

  while(i != itm->live_grenades.end())
  {
    if((*i) == sig)
    {
      i = itm->live_grenades.erase(i);
      break;
    }
    else
      ++i;
  }
}



static void radio_detonate_callback(signaller* sig, const char *data)
{
  grenade *gren = (grenade *)data;

  if(g_world_ptr->is_entity_valid(gren))
    gren->detonate();
}


void thrown_item::init_defaults()
{
  handheld_item::init_defaults();

  damage = 50;

  direct_damage_mod = 1.0f;
  damage_radius = 5.0f;

  timer = 3.0f;

  explode_effect.sound = explode_sound;
  bounce_effect.sound = bounce_sound;
  throw_effect.sound = throw_sound;
  arm_effect.sound = arm_sound;

  radio_detonator_name = empty_string;
  radio_detonated_item_name = empty_string;

  bounce_factor = 0.5f;
  slide_factor = 0.65f;

  launch_vec = vector3d(0, 1, 1);
  launch_vec.normalize();
  launch_force = 5.0f;

  last_grenade_spawned = NULL;
  last_grenade_detonated = NULL;

  set_handheld_flag(_THROWN_CONTACT_DETONATE, true);
  set_handheld_flag(_THROWN_VOLATILE, true);
  set_handheld_flag(_THROWN_EXPLOSIVE, true);
  set_handheld_flag(_THROWN_COPY_VISREP, true);
  vis_ent = NULL;

  guided_percent = 1.0f;
  guided_accuracy = 1.0f;
  wobble_timer = 2.0f;
  turn_factor = 0.35f;
  accel_factor = 0.0f;
  accel_delay = 0.0f;
  guidance_delay = 0.0f;
  wobble_timer_var = 0.0f;
  guidance_delay_var = 0.0f;
  accel_delay_var = 0.0f;

  sticky_offset = 0.025f;

  arm_timer = 0.0f;

  trigger_radius = 3.0f;

  grenade_pool.clear();
  live_grenades.clear();

  affect_switches = false;
}

thrown_item::thrown_item( const entity_id& _id, unsigned int _flags )
:   handheld_item( _id, ENTITY_ITEM, _flags )
{
  init_defaults();
}


thrown_item::thrown_item( const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
:   handheld_item( _id, _flavor, _flags )
{
  init_defaults();
}


thrown_item::~thrown_item()
{
}

///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

thrown_item::thrown_item( chunk_file& fs,
            const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
:   handheld_item( fs, _id, _flavor, _flags )
{
  init_defaults();
}


void thrown_item::read_bounce_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

//  set_handheld_flag(_THROWN_BOUNCE, true);

  while(!(label == chunkend_label))
  {
    if(label == "effect")
    {
      serial_in( fs, &bounce_effect.static_fx );
    }
    else if(label == "script")
    {
      stringx script;
      serial_in( fs, &script );
      bounce_effect.set_script(script);
    }
    else if(label == "lifetime")
    {
      serial_in( fs, &bounce_effect.lifetime );
    }
    else if(label == "sticky")
    {
      set_handheld_flag(_THROWN_STICKY, true);
    }
    else if(label == "bouncy")
    {
      set_handheld_flag(_THROWN_BOUNCE, true);
    }
    else if(label == "sticky_orient")
    {
      set_handheld_flag(_THROWN_STICKY_ORIENT, true);
    }
    else if(label == "sticky_offset")
    {
      serial_in( fs, &sticky_offset );
    }
    else if(label == "bounce_factor")
    {
      serial_in( fs, &bounce_factor );
    }
    else if(label == "slide_factor")
    {
      serial_in( fs, &slide_factor );
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in bounce section" );
    }

    serial_in( fs, &label );
  }
}

void thrown_item::read_explode_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "effect")
    {
      serial_in( fs, &explode_effect.static_fx );
    }
    else if(label == "script")
    {
      stringx script;
      serial_in( fs, &script );
      explode_effect.set_script(script);
    }
    else if(label == "lifetime")
    {
      serial_in( fs, &explode_effect.lifetime );
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in explode section" );
    }

    serial_in( fs, &label );
  }
}

void thrown_item::read_arm_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "effect")
    {
      serial_in( fs, &arm_effect.static_fx );
    }
    else if(label == "arm_timer")
    {
      serial_in( fs, &arm_timer );
    }
    else if(label == "script")
    {
      stringx script;
      serial_in( fs, &script );
      arm_effect.set_script(script);
    }
    else if(label == "lifetime")
    {
      serial_in( fs, &arm_effect.lifetime );
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in arm section" );
    }

    serial_in( fs, &label );
  }
}

void thrown_item::read_throw_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "effect")
    {
      serial_in( fs, &throw_effect.static_fx );
    }
    else if(label == "script")
    {
      stringx script;
      serial_in( fs, &script );
      throw_effect.set_script(script);
    }
    else if(label == "lifetime")
    {
      serial_in( fs, &throw_effect.lifetime );
    }
    else if(label == "launch_vec")
    {
      serial_in( fs, &launch_vec );
      launch_vec.normalize();
    }
    else if(label == "launch_force")
    {
      serial_in( fs, &launch_force );
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in throw section" );
    }

    serial_in( fs, &label );
  }
}

void thrown_item::read_damage_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "damage")
    {
      serial_in( fs, &damage );
    }
    else if(label == "radius")
    {
      serial_in( fs, &damage_radius );
    }
    else if(label == "direct_hit_modifier")
    {
      serial_in( fs, &direct_damage_mod );
    }
    else if(label == "no_contact_detonate")
    {
      set_handheld_flag(_THROWN_CONTACT_DETONATE, false);
    }
    else if(label == "hit_hero_only")
    {
      set_handheld_flag(_THROWN_HIT_HERO_ONLY, true);
    }
    else if(label == "hit_ai_only")
    {
      set_handheld_flag(_THROWN_HIT_AI_ONLY, true);
    }
    else if(label == "affect_switches")
    {
      affect_switches = true;
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in damage section" );
    }

    serial_in( fs, &label );
  }
}


void thrown_item::read_rocket_chunk( chunk_file& fs )
{
  set_handheld_flag(_THROWN_ROCKET, true);

  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "guided")
    {
      set_handheld_flag(_THROWN_GUIDED, true);
    }
    else if(label == "point_guided")
    {
      set_handheld_flag(_THROWN_POINT_GUIDED, true);
    }
    else if(label == "guided_percent")
    {
      serial_in( fs, &guided_percent );

      if(guided_percent > 1.0f)
        guided_percent = 1.0f;
      if(guided_percent < 0.0f)
        guided_percent = 0.0f;
    }
    else if(label == "guided_accuracy")
    {
      serial_in( fs, &guided_accuracy );
      if(guided_accuracy > 1.0f)
        guided_accuracy = 1.0f;
//      if(guided_accuracy < 0.0f)
//        guided_accuracy = 0.0f;
    }
    else if(label == "turn_factor")
    {
      serial_in( fs, &turn_factor);
    }
    else if(label == "accel_factor")
    {
      serial_in( fs, &accel_factor);
    }
    else if(label == "wobble_timer")
    {
      serial_in( fs, &wobble_timer );
    }
    else if(label == "guidance_delay")
    {
      serial_in( fs, &guidance_delay);
    }
    else if(label == "accel_delay")
    {
      serial_in( fs, &accel_delay);
    }
    else if(label == "wobble_timer_var")
    {
      serial_in( fs, &wobble_timer_var );
    }
    else if(label == "guidance_delay_var")
    {
      serial_in( fs, &guidance_delay_var);
    }
    else if(label == "accel_delay_var")
    {
      serial_in( fs, &accel_delay_var);
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in rocket section" );
    }

    serial_in( fs, &label );
  }
}


bool thrown_item::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "thrown_item_effects:")
  {
    serial_in( fs, &label );

    while(!(label == chunkend_label))
    {
      if(label == "damage:")
      {
        read_damage_chunk( fs );
      }
      else if(label == "explode:")
      {
        read_explode_chunk( fs );
      }
      else if(label == "arm:")
      {
        read_arm_chunk( fs );
      }
      else if(label == "throw:")
      {
        read_throw_chunk( fs );
      }
      else if(label == "bounce:")
      {
        read_bounce_chunk( fs );
      }
      else if(label == "orientation:")
      {
        read_orientation_chunk( fs );
      }
      else if(label == "ai_info:")
      {
        read_ai_info_chunk( fs );
      }
      else if(label == "rocket:")
      {
        read_rocket_chunk( fs );
      }
      else if(label == "timer")
      {
        set_handheld_flag(_THROWN_TIMER, true);
        serial_in( fs, &timer );
      }
      else if(label == "radio_detonator")
      {
        set_handheld_flag(_HANDHELD_RADIO_DETONATOR, true);
      }
      else if(label == "radio_detonated")
      {
        set_handheld_flag(_THROWN_RADIO_DETONATE, true);
      }
      else if(label == "radio_detonator_name")
      {
        serial_in( fs, &radio_detonator_name);
        radio_detonator_name.to_upper();

        entity* itm;
        stringx entity_name = get_fname_wo_ext( radio_detonator_name );
        entity_name.to_upper();
        stringx entity_dir = get_dir( radio_detonator_name );
        itm = g_entity_maker->create_entity_or_subclass( entity_name,
                                                      entity_id::make_unique_id(),
                                                      po_identity_matrix,
                                                      entity_dir,
                                                      ACTIVE_FLAG | NONSTATIC_FLAG );

        if ( !itm->is_an_item() )
        {
          error( get_name() + " radio_detonator_name item: entity " + entity_name + " is not an item" );
        }
        else if ( !itm->is_a_thrown_item() )
        {
          error( get_name() + " radio_detonator_name item: entity " + entity_name + " is not an thrown item" );
        }
        else if ( !((thrown_item *)itm)->is_a_radio_detonator() )
        {
          error( get_name() + " radio_detonator_name item: entity " + entity_name + " is not a radio detonator" );
        }
        else
        {
          ((thrown_item *)itm)->set_linked(true);
          ((thrown_item *)itm)->radio_detonated_item_name = item::get_name();
          assert(((thrown_item *)itm)->get_count() == 0);

          add_item((item *)itm);
        }
      }
      else if(label == "laser_detonated")
      {
        set_handheld_flag(_THROWN_LASER_DETONATE, true);
      }
      else if(label == "non_volatile")
      {
        set_handheld_flag(_THROWN_VOLATILE, false);
      }
      else if(label == "non_explosive")
      {
        set_handheld_flag(_THROWN_EXPLOSIVE, false);
      }
      else if(label == "trigger_radius")
      {
        set_handheld_flag(_THROWN_TRIGGER_RADIUS, true);
        serial_in( fs, &trigger_radius );
      }
      else if(label == "visual_entity")
      {
        stringx ent_name;
        serial_in(fs, &ent_name);
        ent_name.to_lower();

        vis_ent = g_entity_maker->create_entity_or_subclass(ent_name, entity_id::make_unique_id(), po_identity_matrix, empty_string);
        if(vis_ent == NULL)
          error("Visual Entity '%s' couldn't load!", ent_name.c_str());
        else
        {
          switch ( vis_ent->get_flavor() )
          {
            case ENTITY_ITEM:
              error( "Unsupported flavor for visual entity: %s", entity_flavor_names[vis_ent->get_flavor()] );
              break;

            default:
              break;
          }

          set_handheld_flag(_THROWN_COPY_VISREP, false);
          vis_ent->set_active(false);
          vis_ent->set_visible(false);

          if(vis_ent->has_physical_ifc())
            vis_ent->physical_ifc()->disable();

          //vis_ent->set_physical(false);
          //vis_ent->set_gravity(false);
        }
      }
      else if(label == "preload")
      {
        serial_in( fs, &label );
        g_world_ptr->create_preloaded_entity_or_subclass( label.c_str(),  empty_string );
      }
      else
      {
        error( fs.get_filename() + ": unknown keyword '" + label + "' in thrown_item_effects section" );
      }

      serial_in( fs, &label );
    }

    return(true);
  }
  else
    return(handheld_item::handle_enx_chunk( fs, label ));
}


///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

thrown_item::thrown_item( const stringx& item_type_filename,
            const entity_id& _id,
            entity_flavor_t _flavor,
            bool _active,
            bool _stationary )
:   handheld_item( item_type_filename, _id, _flavor, _active, _stationary)
{
  init_defaults();
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* thrown_item::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  thrown_item* newit = NEW thrown_item( _id, _flags );
  newit->copy_instance_data( *((thrown_item *)this) );
  return (entity*)newit;
}


void thrown_item::copy_instance_data( thrown_item& b )
{
  handheld_item::copy_instance_data( b );

  damage = b.damage;

  direct_damage_mod = b.direct_damage_mod;
  damage_radius = b.damage_radius;

  timer = b.timer;

  explode_effect = b.explode_effect;
  bounce_effect = b.bounce_effect;
  throw_effect = b.throw_effect;
  arm_effect = b.arm_effect;

  radio_detonator_name = b.radio_detonator_name;
  radio_detonated_item_name = b.radio_detonated_item_name;

  bounce_factor = b.bounce_factor;
  slide_factor = b.slide_factor;

  launch_vec = b.launch_vec;
  launch_force = b.launch_force;

  vis_ent = b.vis_ent;
  guided_percent = b.guided_percent;
  guided_accuracy = b.guided_accuracy;
  wobble_timer = b.wobble_timer;
  turn_factor = b.turn_factor;
  accel_factor = b.accel_factor;
  accel_delay = b.accel_delay;
  guidance_delay = b.guidance_delay;
  wobble_timer_var = b.wobble_timer_var;
  guidance_delay_var = b.guidance_delay_var;
  accel_delay_var = b.accel_delay_var;

  sticky_offset = b.sticky_offset;
  arm_timer = b.arm_timer;
  trigger_radius = b.trigger_radius;
}


grenade *thrown_item::get_new_grenade()
{
  vector<grenade *>::iterator g = grenade_pool.begin();
  vector<grenade *>::iterator g_end = grenade_pool.end();

  grenade *gren = NULL;

  while(gren == NULL && g != g_end)
  {
    if((*g) && (*g)->ready_for_spawning)
    {
      gren = (*g);
      break;
    }

    ++g;
  }

  if(gren == NULL)
  {
    gren = NEW grenade(entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_PHYSICS|EFLAG_PHYSICS_MOVING|EFLAG_MISC_NONSTATIC|EFLAG_PHYSICS_COLLISIONS_ACTIVE|EFLAG_PHYSICS_COLLIDE_TERRAIN));

    assert(gren != NULL && gren->has_physical_ifc());

    gren->physical_ifc()->set_gravity(!is_rocket());

    if(get_handheld_flag(_THROWN_COPY_VISREP))
      gren->copy_visrep(this);
    else if(vis_ent)
    {
      gren->visual_entity = vis_ent->make_instance(entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC));
      assert(gren->visual_entity != NULL);

      if(gren->visual_entity->has_physical_ifc())
        gren->visual_entity->physical_ifc()->disable();

      gren->visual_entity->set_collisions_active(false);
    }
    else
      error("No visual representation for grenade: %s!", item::get_name().c_str());

    gren->physical_ifc()->set_mass((has_physical_ifc() && physical_ifc()->get_mass() > 0.0f) ? physical_ifc()->get_mass() : 1.0f);
    gren->physical_ifc()->set_volume((has_physical_ifc() && physical_ifc()->get_volume() > 0.0f) ? physical_ifc()->get_volume() : 1.0f);
    gren->grenade_scale = drawn_offset.get_scale();

    gren->item_owner = this;

    gren->set_collisions_active(false);

    if(has_sound_ifc())
    {
      if(!gren->has_sound_ifc())
        gren->create_sound_ifc();

      gren->sound_ifc()->copy(sound_ifc());
    }

    g_world_ptr->add_dynamic_instanced_entity( gren );
    g_world_ptr->guarantee_active( gren );

    if ( gren->visual_entity != NULL )
      g_world_ptr->add_dynamic_instanced_entity( gren->visual_entity );

    grenade_pool.push_back(gren);
  }

  gren->ready_for_spawning = false;
  gren->detonated = false;
  gren->use_owner_last_pos = true;


  gren->physical_ifc()->set_velocity(ZEROVEC);
  gren->physical_ifc()->set_angular_velocity(ZEROVEC);
  gren->physical_ifc()->set_acceleration_factor(ZEROVEC);
  gren->physical_ifc()->set_acceleration_correction_factor(ZEROVEC);
  gren->physical_ifc()->set_last_acceleration_correction_factor(ZEROVEC);

  return(gren);
}


rational_t g_min_gren_sound_range = 7.5f;
rational_t g_max_gren_sound_range = 30.0f;

void thrown_item::spawn_grenade(vector3d dir, rational_t force)
{
  last_grenade_spawned = get_new_grenade();

  assert(last_grenade_spawned->has_physical_ifc());

  if(is_rocket())
  {
    if(!last_grenade_spawned->physical_ifc()->has_guidance_sys())
      last_grenade_spawned->physical_ifc()->create_guidance_sys(guidance_system::_GUIDANCE_ROCKET);

    assert(last_grenade_spawned->physical_ifc()->guidance_sys()->get_type() == guidance_system::_GUIDANCE_ROCKET);

    rocket_guidance_sys *guide = (rocket_guidance_sys *)last_grenade_spawned->physical_ifc()->guidance_sys();

    if(is_guided())
    {
      guide->set_guided(ZERO_TO_ONE <= guided_percent);
      guide->set_point_guided(false);
    }
    else if(is_point_guided())
    {
      guide->set_point_guided(ZERO_TO_ONE <= guided_percent);
      guide->set_guided(false);
    }

    guide->set_target(get_owner()->get_current_target());
    guide->set_target_pos(get_owner()->get_current_target_pos());
    guide->set_guided_accuracy(guided_accuracy);

    guide->set_accel_delay(get_accel_delay());
    guide->set_guidance_delay(get_guidance_delay());
    guide->set_wobble_timer(get_wobble_timer());

    guide->set_accel_delay_var(get_accel_delay_var());
    guide->set_guidance_delay_var(get_guidance_delay_var());
    guide->set_wobble_timer_var(get_wobble_timer_var());

    guide->set_accel_factor(get_accel_factor());
    guide->set_turn_factor(get_turn_factor());

    po the_po = po_identity_matrix;
    the_po.set_facing(dir);
    last_grenade_spawned->set_rel_po(the_po);
  }
  else
  {
    last_grenade_spawned->physical_ifc()->set_angular_velocity(vector3d(PLUS_MINUS_ONE*PI, PLUS_MINUS_ONE*PI, PLUS_MINUS_ONE*PI));
    last_grenade_spawned->physical_ifc()->set_gravity(true);
  }

  last_grenade_spawned->physical_ifc()->set_sticky(is_sticky());
  last_grenade_spawned->physical_ifc()->set_sticky_offset(get_sticky_offset());

  // Must be tru so that rockets and grenades can detonate on contact with terrain
  last_grenade_spawned->physical_ifc()->set_bouncy(true);
  last_grenade_spawned->physical_ifc()->set_bounce_factor(bounce_factor);
  last_grenade_spawned->physical_ifc()->set_slide_factor(slide_factor);


  if(vis_item)
    last_grenade_spawned->set_rel_position(vis_item->get_abs_position());
  else
    last_grenade_spawned->set_rel_position(owner->get_abs_position());

  last_grenade_spawned->get_emitter()->set_position(last_grenade_spawned->get_abs_position());

  if(last_grenade_spawned->visual_entity)
  {
    last_grenade_spawned->visual_entity->set_active(true);
    last_grenade_spawned->visual_entity->set_visible(true);
    last_grenade_spawned->visual_entity->set_rel_po(last_grenade_spawned->get_abs_po());

    if(last_grenade_spawned->visual_entity->is_a_conglomerate())
      ((conglomerate *)last_grenade_spawned->visual_entity)->frame_done_including_members();
    else
      last_grenade_spawned->visual_entity->frame_done();

    last_grenade_spawned->visual_entity->set_stationary(false);

    if(last_grenade_spawned->visual_entity->has_physical_ifc())
      last_grenade_spawned->visual_entity->physical_ifc()->disable();

    last_grenade_spawned->visual_entity->set_collisions_active(false);
  }

  last_grenade_spawned->set_collisions_active(false);
  last_grenade_spawned->set_stationary(false);
  last_grenade_spawned->set_active(true);
  last_grenade_spawned->set_visible(true);
  last_grenade_spawned->timer = timer;
  last_grenade_spawned->arm_timer = arm_timer;

  if(!is_rocket() && last_grenade_spawned->arm_timer > 0.0f) last_grenade_spawned->allow_hit_owner = true;

  last_grenade_spawned->physical_ifc()->enable();

  if(last_grenade_spawned->physical_ifc()->has_guidance_sys())
    last_grenade_spawned->physical_ifc()->guidance_sys()->launch(dir, force);
  else
    last_grenade_spawned->physical_ifc()->apply_force_increment(dir*force, physical_interface::INSTANT);

  last_grenade_spawned->compute_sector(g_world_ptr->get_the_terrain());

  if(last_grenade_spawned->has_sound_ifc())
  {
    assert(last_grenade_spawned->sound_id == 0);
    sg_entry* entry = last_grenade_spawned->sound_ifc()->play_looping_3d_sound_grp(grenade_loop_sound);
    if(entry)
      last_grenade_spawned->sound_id = entry->last_id_played;
  }


  if(is_radio_detonated())
  {
    thrown_item *detonator = get_radio_detonator();

    signal* sptr = NULL;

    sptr = detonator != NULL ? detonator->signal_ptr( entity::RADIO_DETONATE ) : owner->signal_ptr( entity::RADIO_DETONATE );
    assert(sptr);
    if( sptr )
      sptr->add_callback( radio_detonate_callback, (char *)last_grenade_spawned, true );

    if(detonator != NULL)
      detonator->inc_count();
  }

  live_grenades.push_back(last_grenade_spawned);

  signal* sptr = NULL;
  sptr = last_grenade_spawned->signal_ptr( grenade::EXPLODE );
  assert(sptr);
  if( sptr )
    sptr->add_callback( thrown_item_remove_live_grenade, NULL, true );

  throw_effect.spawn(false, last_grenade_spawned->get_abs_position(), last_grenade_spawned->get_abs_po().get_facing(), this, last_grenade_spawned);
}

thrown_item *thrown_item::get_radio_detonator() const
{
  thrown_item *detonator = NULL;
  if(is_radio_detonated() && radio_detonator_name.length() > 0)
    detonator = (thrown_item *)get_owner()->find_item_by_name(radio_detonator_name);

  return(detonator);
}

thrown_item *thrown_item::get_radio_detonated_item() const
{
  thrown_item *detonated_item = NULL;
  if(is_a_radio_detonator() && radio_detonated_item_name.length() > 0)
    detonated_item = (thrown_item *)get_owner()->find_item_by_name(radio_detonated_item_name);

  return(detonated_item);
}

void thrown_item::apply_effects( entity* ent )
{
//  if(get_count() != 0)
  {
    entity *tmp_owner = NULL;

    if(owner)
      tmp_owner = owner;
    else
      tmp_owner = ent;

    if(tmp_owner)
    {
      if(is_a_radio_detonator())
      {
        raise_signal(entity::RADIO_DETONATE);
      }
      else
      {
        vector3d vec = launch_vec;

        if(vis_item)
          vec = vis_item->get_abs_po().fast_8byte_non_affine_xform(vec);
        else
          vec = tmp_owner->get_abs_po().fast_8byte_non_affine_xform(vec);

        vec.normalize();
        spawn_grenade(vec, launch_force);
      }

      item::apply_effects( tmp_owner );

//      if(get_count() > 0)
//        dec_count();
    }
  }
}

vector3d thrown_item::invert_launch_vec(const vector3d &vec)
{
  vector3d new_vec = vec;

  if(vis_item)
    new_vec = vis_item->get_abs_po().fast_non_affine_inverse_xform(new_vec);
  else if(owner)
    new_vec = owner->get_abs_po().fast_non_affine_inverse_xform(new_vec);

  return(new_vec);
}


void thrown_item::frame_advance( time_value_t t )
{
  if(is_a_radio_detonator() && get_count() <= 0 && get_owner() != NULL && get_owner()->is_hero() /*! && ((character *)get_owner())->get_cur_item() == this !*/)
  {
    holster();
    hide();
//!    ((character *)get_owner())->get_player_controller()->next_inv_item();
  }

  item::frame_advance(t);
}


extern rational_t g_gravity;
vector3d thrown_item::calculate_target_vector(const vector3d &target, const vector3d &from)
{
  if(is_rocket())
  {
    vector3d dir = target - from;
    dir.normalize();
    dir *= launch_force;

    return(dir);
  }
  else
  {
/*
    vector3d dir = ZEROVEC;
    rational_t mass = (has_physical_ifc() && physical_ifc()->get_mass() > 0.0f) ? physical_ifc()->get_mass() : 1.0f;

    assert(mass > 0.0f);

	  vector3d   rel_targ = target - from;
	  vector2d   P((rel_targ - YVEC * rel_targ.y).length(), rel_targ.y);
	  rational_t Vh, Vy;
	  rational_t r = launch_force;

	  // perform the computation
	  if(__fabs(P.x) > SMALL_DIST)
    {
		  rational_t norm2P = P.x * P.x + P.y * P.y;
		  rational_t discriminant = (-g_gravity * P.y + r * r) * (-g_gravity * P.y + r * r) - g_gravity * g_gravity * norm2P;
		  if(discriminant > 0.0f)
      {
			  Vh = P.x * __fsqrt(((-g_gravity * P.y + r * r) + __fsqrt(discriminant)) / (2.0f * norm2P));
			  Vy = (Vh * Vh * P.y + 0.5f * g_gravity * P.x * P.x) / (Vh * P.x); // = +/- (__fsqrt(r * r - Vh * Vh));
		  }
		  else
      {
			  Vh=1.0f;
			  Vy=0.0f;
		  }
	  }
	  else
    {
		  Vh = 1.0f;
		  Vy = 0.0f;
	  }

	  dir = (rel_targ - (YVEC * rel_targ.y)).set_length(Vh);
	  dir.y = Vy;

    // dir is in velocity, need to multiply by mass for force
    dir *= mass * 0.5f;

    return(dir);
//*/
//*
    rational_t xzvel = 6.0f;

    vector3d delta = target - from;

    rational_t delta_y = delta.y;
    delta.y = 0.0f;
    rational_t delta_xz = delta.length();
    if(delta_xz > 0.0f)
    {
      delta *= (1.0f / delta_xz);
    }


    rational_t vely;
    rational_t time = delta_xz / xzvel;

    rational_t grav_mod = g_gravity*0.45f;

    if(__fabs(delta_y) <= 2.0f)
      vely = (0.5f*grav_mod*time);
    else
    {
      if(delta_y <= 0.0f)
      {
        vely = (grav_mod * 0.5f * 0.75f);
      }
      else
      {
        vely = (delta_y/time) + (0.5f*grav_mod*time);
      }
    }

    return((vely*YVEC) + (xzvel*delta));
//*/
  }
}








grenade::grenade( const entity_id& _id, unsigned int _flags )
:   entity( _id, ENTITY_ENTITY, _flags )
{
  item_owner = NULL;
  timer = 0.0f;
  grenade_scale = 1.0f;

  sound_id = 0;
  arm_sound_id = 0;

  detonated = false;
  visual_entity = NULL;
  allow_hit_owner = false;

  armed = false;

  arm_timer = 0.0f;

  laser_beam = NULL;
  beam_length = -1.0f;

  ready_for_spawning = true;

  beam_update_timer = 0.0f;

  stuck_parent_was_alive_last_frame = true;

  if(!has_physical_ifc())
    create_physical_ifc();
}

grenade::~grenade()
{
  if(sound_ifc())
  {
    if(sound_id > 0)
    {
      sound_ifc()->kill_sound(sound_id);
      sound_id = 0;
    }

    if(arm_sound_id > 0)
    {
      sound_ifc()->kill_sound(arm_sound_id);
      arm_sound_id = 0;
    }
  }
}

void grenade::detonate(entity *ent)
{
  if(detonated)
    return;

  assert(item_owner);

  detonated = true;

  vector3d blast_pos = get_abs_position();

  if(item_owner->last_grenade_spawned == this)
    item_owner->last_grenade_spawned = NULL;

  item_owner->set_detonate_position(blast_pos);
  item_owner->last_grenade_detonated = this;
  item_owner->raise_signal( item::DETONATE );

  raise_signal( grenade::EXPLODE );

  item_owner->explode_effect.spawn(false, get_abs_position(), get_abs_po().get_facing(), item_owner, this);

  if(item_owner->damage != 0)
  {
    static vector<region_node*> regs;
    static vector<entity*> damageables;
    regs.clear();
    damageables.clear();

    vector<grenade*> grenades;
    grenades.clear();

    vector<entity*>::const_iterator i;
    vector<entity*>::const_iterator i_end;
    vector3d vec;
    rational_t len2;

    // build list of regions that can be affected by blast
    build_region_list_radius(&regs, get_primary_region(), blast_pos, item_owner->damage_radius);

    // build list of entities that are affected by blast
    vector<region_node*>::iterator rn = regs.begin();
    vector<region_node*>::iterator rn_end = regs.end();
    for ( ; rn!=rn_end; ++rn )
    {
      region* r = (*rn)->get_data();
      i = r->get_entities().begin();
      i_end = r->get_entities().end();

      for ( ; i!=i_end; ++i )
      {
        entity* e = *i;
#if defined(TARGET_XBOX)
        STUB( "grenade::detonate" );

        if ( e && e != this && e->is_visible() && false )
#else
        if ( e && e != this && e->is_visible() && ((e->has_damage_ifc() && e->damage_ifc()->is_alive()) || e->is_a_grenade() || (item_owner->affects_switches() && e->get_flavor() == ENTITY_SWITCH)) )

#endif /* TARGET_XBOX JIV DEBUG */
        {
          // TODO: maybe make a line from grenade to target, if interrupted by terrain, boxes, etc, do NO damage
          // Basically, allow for cover to avoid damage (JDB)
          vec = blast_pos - e->get_abs_position();
          len2 = vec.length2();
          if ( e == ent || len2 < item_owner->damage_radius*item_owner->damage_radius )
          {
            if(e->is_a_grenade())
            {
              grenade *g = (grenade *)e;

              if(!g->detonated && g->item_owner->is_volatile() && g->is_visible() && g != this)
                grenades.push_back( (grenade *)e );
            }
            else if(e->get_flavor() == ENTITY_SWITCH)
            {
              ((switch_obj *)e)->flick();
            }
            else
              damageables.push_back( e );
          }
        }
      }
    }

    // now apply the damage
    i = damageables.begin();
    i_end = damageables.end();
    for ( ; i!=i_end; ++i )
    {
      entity* e = *i;
      vec = e->get_abs_position() - blast_pos;
      vec.normalize();

      int d;
      if ( e == ent )
        d = (int)(item_owner->damage * item_owner->direct_damage_mod);
      else
        d = item_owner->damage;

#if defined(TARGET_XBOX)
      STUB( "grendate::detonate" );
#else
      e->damage_ifc()->apply_damage( item_owner, d, item_owner->is_explosive() ? DAMAGE_EXPLOSIVE : DAMAGE_DIRECT_DIRECTIONAL, blast_pos, vec);
#endif /* TARGET_XBOX JIV DEBUG */
    }

    // now detonate other grenades
    vector<grenade *>::iterator gi = grenades.begin();
    vector<grenade *>::iterator gi_end = grenades.end();
    for ( ; gi!=gi_end; ++gi )
    {
      grenade *gren = (*gi);
      if(!gren->detonated && gren->item_owner->is_volatile())
        gren->detonate();
    }
  }

  thrown_item *detonator = item_owner->get_radio_detonator();
  if(detonator != NULL)
    detonator->dec_count();

  clear();
}

/*
void grenade::bounce(vector3d old_pos, vector3d new_pos, vector3d hit, vector3d hit_norm, entity *hit_entity, time_value_t t)
{
  assert(has_physical_ifc());

  set_rel_position(hit + (hit_norm * (get_visual_radius() > 0.0f ? get_visual_radius()*0.15f : 0.05f)));

  if(item_owner->is_sticky())
  {
    if(item_owner->has_bounce_sound())
    {
      if(get_emitter())
        get_emitter()->play_sound( item_owner->bounce_sound, item_owner->bounce_sound_var );
      else
      {
        assert(0);
        sound_device::inst()->play_sound( item_owner->bounce_sound, item_owner->bounce_sound_var );
      }
    }

    physical_ifc()->set_velocity(ZEROVEC);
    physical_ifc()->set_gravity(false);
    physical_ifc()->set_angular_velocity(ZEROVEC);

    if(item_owner->is_sticky_orient())
    {
      po my_po = po_identity_matrix;
      my_po.set_facing(hit_norm);
      my_po.set_position(hit + (hit_norm*item_owner->get_sticky_offset()));

      set_rel_po(my_po);
    }

    if(hit_entity != NULL)
    {
      po relpo = get_abs_po() * hit_entity->get_abs_po().inverse();
//!      set_parent( hit_entity );
      set_rel_po( relpo );

      stuck_parent_was_alive_last_frame = hit_entity->is_alive();
    }

    stuck = true;
  }
  else
  {
    vector3d vel, dir;
    vel = physical_ifc()->get_velocity();

    // reverse it
    vel *= -1;

    rational_t vel_len = vel.length();

    if(item_owner->has_bounce_sound() && vel_len >= 2)
    {
      if(get_emitter())
        get_emitter()->play_sound( item_owner->bounce_sound, item_owner->bounce_sound_var );
      else
      {
        assert(0);
        sound_device::inst()->play_sound( item_owner->bounce_sound, item_owner->bounce_sound_var );
      }
    }

    // calculate NEW bounce direction
    rational_t inv_vel_len = 1.0f / vel_len;

    dir.x = vel.x*inv_vel_len;
    dir.y = vel.y*inv_vel_len;
    dir.z = vel.z*inv_vel_len;

    po the_po;
    rational_t dot_prod = dot(dir, hit_norm);
    vector3d cross_prod = cross(dir, hit_norm);

    the_po.set_rot((cross_prod), -2.0f*fast_acos(dot_prod));
    vel = the_po.non_affine_slow_xform(vel);

    // cheapo, better would be to make it multiply by bounce_factor in dir of normal and slide_factor in other 2 ortho dir's
    vel.x *= item_owner->slide_factor;
    vel.y *= item_owner->bounce_factor;
    vel.z *= item_owner->slide_factor;

    physical_ifc()->set_velocity(vel);
  }
}
*/

bool grenade::check_if_hit(entity *ent, const vector3d &old_pos, const vector3d &new_pos, vector3d &hit_pos)
{
  if(ent)
  {
    if(!physical_ifc()->is_stuck())
    {
      vector3d hitn;
      return(ent->test_combat_target(old_pos, new_pos, &hit_pos, &hitn));
    }
    else
    {
      rational_t radius = item_owner->has_trigger_radius() ? item_owner->get_trigger_radius() : (ent->get_radius() > 0.0f ? ent->get_radius() : 1.0);
      hit_pos = get_abs_position();
      return((get_abs_position() - ent->get_abs_position()).length2() <= (radius*radius));
    }
  }

  return(false);
}


entity *grenade::check_hit(const vector3d &old_pos, const vector3d &new_pos, vector3d &hit_pos)
{
  if(get_primary_region())
  {
    if(item_owner->hit_hero_only())
    {
      if ((g_world_ptr->get_hero_ptr() != item_owner->owner || allow_hit_owner) && check_if_hit(g_world_ptr->get_hero_ptr(), old_pos, new_pos, hit_pos))
        return(g_world_ptr->get_hero_ptr());
    }
    else
    {
      vector<entity*>::const_iterator i = get_primary_region()->get_data()->get_entities().begin();
      vector<entity*>::const_iterator i_end = get_primary_region()->get_data()->get_entities().end();

      for ( ; i<i_end; i++ )
      {
        entity* e = *i;
#if defined(TARGET_XBOX)
        STUB( "grenade::check_hit" );

        if (e && e->is_visible() && false && (e != item_owner->owner || allow_hit_owner) && ((item_owner && !item_owner->hit_ai_only()) || e->is_hero() || e->has_ai_ifc()))
#else
        if (e && e->is_visible() && e->has_damage_ifc() && e->damage_ifc()->is_alive() && (e != item_owner->owner || allow_hit_owner) && ((item_owner && !item_owner->hit_ai_only()) || e->is_hero() || e->has_ai_ifc()))

#endif /* TARGET_XBOX JIV DEBUG */
        {
          if(check_if_hit(e, old_pos, new_pos, hit_pos))
            return(e);
        }
      }
    }
  }
//  else
//    assert(0);

  return(NULL);
}

void grenade::frame_advance( time_value_t t )
{
  START_PROF_TIMER(proftimer_grenade);

  if(!detonated)
  {
    if(physical_ifc()->is_stuck() && has_parent())
    {
/*
      if(!link_ifc()->get_parent()->is_alive() && stuck_parent_was_alive_last_frame)
      {
        if(item_owner->is_laser_detonated() && armed)
        {
          detonate();
          return;
        }
        else
        {
          //stuck = false;
          po the_po = get_abs_po();
          set_parent( NULL );
          set_rel_po(the_po);
          set_velocity(ZEROVEC);
          set_gravity(true);
          set_angular_velocity(ZEROVEC);
        }
      }

      stuck_parent_was_alive_last_frame = has_parent() ? link_ifc()->get_parent()->is_alive() : true;
*/
      vector3d hit_loc;
      if(get_primary_region() == NULL || !in_world(get_abs_position(), 0, ZEROVEC, get_primary_region(), hit_loc))
      {
        if(item_owner->damage_radius > 0.0f)
          detonate();
        else
          clear(true);

        STOP_PROF_TIMER(proftimer_grenade);
        return;
      }
    }
    if(!allow_hit_owner)
    {
      rational_t radius = item_owner->has_trigger_radius() ? item_owner->get_trigger_radius() : (item_owner->get_owner()->get_radius() > 0.0f ? item_owner->get_owner()->get_radius() : 1.0);
      allow_hit_owner = !((get_abs_position() - item_owner->get_owner()->get_abs_position()).length2() <= (radius*radius));
    }

    if(arm_timer > 0.0f)
    {
      arm_timer -= t;
      if(arm_timer < 0.0f)
      {
        arm_timer = 0.0f;
      }
    }

    if(!armed && is_ready_to_be_armed())
    {
      armed = true;

      item_owner->last_grenade_armed = this;

      item_owner->arm_effect.spawn(false, get_abs_position(), get_abs_po().get_facing(), item_owner, this);
      if(has_sound_ifc())
      {
        assert(arm_sound_id == 0);
        sg_entry* entry = sound_ifc()->play_looping_3d_sound_grp(arm_loop_sound);
        if(entry)
          arm_sound_id = entry->last_id_played;
      }

      if(item_owner->is_laser_detonated())
      {
        if(laser_beam == NULL)
        {
          laser_beam = g_world_ptr->add_beam( entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC) );
          //g_world_ptr->guarantee_active( laser_beam );

          assert(laser_beam != NULL);

          laser_beam->set_created_entity_default_active_status();

          laser_beam->set_beam_flag(beam::NO_CLIP_TO_HERO);
          laser_beam->set_beam_flag(beam::NO_CLIP_TO_BEAMABLE);
          laser_beam->set_beam_flag(beam::NO_CLIP_TO_WORLD);
        }

        laser_beam->set_visible(true);
        laser_beam->set_active(true);
        beam_length = -1.0f;
        beam_update_timer = 0.0f;
      }

      item_owner->raise_signal( item::ARMED );
    }

    if(armed)
    {
      if(item_owner->has_timer() && timer > 0.0f)
      {
        timer -= t;

        if(timer < 0.0f)
          timer = 0.0f;
      }

      if(beam_update_timer > 0.0f)
      {
        beam_update_timer -= t;

        if(beam_update_timer < 0.0f)
          beam_update_timer = 0.0f;
      }
    }

    if(!item_owner->is_radio_detonated() && !item_owner->is_laser_detonated() && item_owner->has_timer() && timer <= 0)
    {
      if(item_owner->damage_radius > 0.0f)
        detonate();
      else
        clear(true);
    }
    else
    {
      vector3d old_pos = (use_owner_last_pos && item_owner->get_owner() != NULL) ? item_owner->get_owner()->get_abs_position() : get_abs_position();
//      bool use_owner_region = false; // unused -- remove me?
      use_owner_last_pos = false;
      vector3d new_pos = old_pos;

      if ( !physical_ifc()->is_stuck() )
      {
        //assert(item_owner->get_owner() != NULL);

        entity::frame_advance(t);
        new_pos = get_abs_position();

        po tmp1 = po_identity_matrix;
        tmp1.set_scale( vector3d(grenade_scale, grenade_scale, grenade_scale) );
        po my_po = get_abs_po();
        my_po.add_increment( &tmp1 );
        set_rel_po(my_po);

        if(physical_ifc()->has_bounced() && (allow_hit_owner || physical_ifc()->get_bounce_ent() != item_owner->get_owner()))
        {
#if defined(TARGET_XBOX)
          STUB( "grenade::frame_advance" );

          if(true)
          {
            // do nothing JIV
          }
#else
          if(!item_owner->is_radio_detonated() && !item_owner->is_laser_detonated() && armed && (!(item_owner->has_bounce() || item_owner->is_sticky()) || (item_owner->detonate_on_contact() && physical_ifc()->get_bounce_ent() && physical_ifc()->get_bounce_ent()->has_damage_ifc())))
            detonate((physical_ifc()->get_bounce_ent() && physical_ifc()->get_bounce_ent()->has_damage_ifc()) ? physical_ifc()->get_bounce_ent() : NULL);

#endif /* TARGET_XBOX JIV DEBUG */
          else if((item_owner->is_sticky() || item_owner->has_bounce()) && physical_ifc()->get_velocity().length2() >= 2.0f)
            item_owner->bounce_effect.spawn(false, physical_ifc()->get_bounce_pos(), physical_ifc()->get_bounce_norm(), item_owner, this);
        }
      }
      else if(item_owner->is_laser_detonated() && armed)
      {
        rational_t old_beam_length = beam_length;

        if(beam_update_timer <= 0.0f)
        {
          entity *hit_entity = NULL;
          vector3d target_pos, hit_norm;
          vector3d start = get_abs_position();

          find_intersection( start, start + (get_abs_po().get_facing()*100.0f),
                             get_primary_region(),
                             FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                             &target_pos, &hit_norm,
                             NULL, &hit_entity );

          beam_length = (start - target_pos).length();

          laser_beam->set_point_to_point(start, target_pos);

          laser_beam->compute_sector(g_world_ptr->get_the_terrain());

          beam_update_timer = (link_ifc()->get_parent() != NULL) ? 0.0f : 0.25f;
        }

        // if the beam gets shorter, detonate
        if((old_beam_length > 0.0f && __fabs(beam_length - old_beam_length) >= 0.1f) || beam_length <= 0.1f)
        {
          detonate();
        }
        else
        {
          old_beam_length = beam_length;

          vector3d start = get_abs_position();
          vector3d end = start + (get_abs_po().get_facing() * beam_length);
          vector3d delta = (end - start);
          vector3d hit, norm;

          vector<region_node*>      regs;
          // get_primary_region might be null!
          build_region_list( &regs, get_primary_region(), start, delta );

          for( vector<region_node*>::const_iterator ri = regs.begin(); ri != regs.end() && !detonated; ++ri )
          {
            vector<entity*>::const_iterator i = (*ri)->get_data()->get_entities().begin();
            vector<entity*>::const_iterator i_end = (*ri)->get_data()->get_entities().end();

            for ( ; i<i_end && !detonated; ++i )
            {
              entity* e = *i;
              if ( e )
              {
                // any character other than the shooter can be a combat target
                if ((e->is_visible() || e == g_world_ptr->get_hero_ptr()) && (e != g_world_ptr->get_hero_ptr() /*! ||
                    !g_world_ptr->get_hero_ptr()->is_stealth() !*/) && e->allow_targeting() && (!item_owner->hit_ai_only() ||
                    e->is_hero() || e->has_ai_ifc()) && e->test_combat_target(start, end, &hit, &norm, 1.0f, false))
                {
                  detonate();
                }
              }
            }
          }
        }
      }

      if(!item_owner->is_radio_detonated() && !item_owner->is_laser_detonated() && item_owner->detonate_on_contact() && !detonated && armed && (!physical_ifc()->is_stuck() || item_owner->damage_radius > 0.0f))
      {
        vector3d hit_pos;
        entity *ent = check_hit(old_pos, new_pos, hit_pos);
        if(ent/* && (!ent->is_a_conglomerate() || !((conglomerate *)ent)->has_member(item_owner->get_owner()))*/)
        {
          if(has_parent())
            hit_pos = link_ifc()->get_parent()->get_abs_po().inverse().slow_xform(hit_pos);
          set_rel_position(hit_pos);

          detonate(item_owner->has_trigger_radius() ? NULL : ent);
        }
      }

      if(visual_entity)
      {
        visual_entity->set_rel_po(get_abs_po());
        visual_entity->force_regions(this);
      }
    }
  }

  STOP_PROF_TIMER(proftimer_grenade);
}


void grenade::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
  if(visual_entity)
    return;
  else
    entity::render(camera_link, detail, flavor, entity_translucency_pct);
}


void grenade::clear(bool force_detonated)
{
  set_visible(false);
  set_active(false);

  if(laser_beam != NULL)
  {
    laser_beam->set_active(false);
    laser_beam->set_visible(false);
  }

  if(sound_ifc())
  {
    if(sound_id > 0)
    {
      sound_ifc()->kill_sound(sound_id);
      sound_id = 0;
    }

    if(arm_sound_id > 0)
    {
      sound_ifc()->kill_sound(arm_sound_id);
      arm_sound_id = 0;
    }
  }

  if(visual_entity)
  {
    visual_entity->set_active(false);
    visual_entity->set_visible(false);
  }

  timer = 0.0f;
  allow_hit_owner = false;
  armed = false;

  arm_timer = 0.0f;
  beam_length = -1.0f;

  beam_update_timer = 0.0f;

  link_ifc()->clear_parent();

  ready_for_spawning = true;
  use_owner_last_pos = true;
  stuck_parent_was_alive_last_frame = true;

  physical_ifc()->set_flag((physical_interface::_PHYS_BOUNCED | physical_interface::_PHYS_STUCK), false);

  if(force_detonated)
  {
    thrown_item *detonator = item_owner->get_radio_detonator();
    if(detonator != NULL)
      detonator->dec_count();

    raise_signal( grenade::EXPLODE );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void grenade::register_signals()
  {
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "grenade_signals.h"
  #undef MAC
  }

static const char* thrown_item_signal_names[] =
  {
  #define MAC(label,str)  str,
  #include "grenade_signals.h"
  #undef MAC
  };

unsigned short grenade::get_signal_id( const char *name )
  {
  unsigned idx;

  for( idx = 0; idx < (sizeof(thrown_item_signal_names)/sizeof(char*)); idx++ )
    {
    unsigned offset = strlen(thrown_item_signal_names[idx])-strlen(name);

    if( offset > strlen( thrown_item_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&thrown_item_signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
    }

  // not found
  return entity::get_signal_id( name );
  }

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* grenade::get_signal_name( unsigned short idx ) const
  {
  assert( idx < N_SIGNALS );
  if ( idx <= PARENT_SYNC_DUMMY )
    return entity::get_signal_name( idx );
  else
    return thrown_item_signal_names[idx-PARENT_SYNC_DUMMY-1];
  }




/*
rocket::rocket( const entity_id& _id, unsigned int _flags )
  : grenade(_id, _flags)
{
  wobble_timer = 0.0f;
  target = NULL;
  guided = false;
  point_guided = false;
  force = 0.0f;
  guidance_delay = 0.0f;
  accel_delay = 0.0f;
  target_pos = ZEROVEC;
}

rocket::~rocket()
{
}

void rocket::frame_advance( time_value_t t )
{
  assert(has_physical_ifc());

  if(!detonated)
  {
    wobble_timer -= t;
    if(wobble_timer <= 0.0f)
      wobble_timer = 0.0f;

    guidance_delay -= t;
    if(guidance_delay <= 0.0f)
      guidance_delay = 0.0f;

    accel_delay -= t;
    if(accel_delay <= 0.0f)
      accel_delay = 0.0f;

    vector3d pos = get_abs_position();

    if(guided && (target == NULL || !target->allow_targeting() || (!target->is_visible() && target != g_world_ptr->get_hero_ptr())))
    {
      // acquire a NEW target if possible
      target = NULL;

      if(guidance_delay <= 0.0f)
      {
        static vector<region_node*> regs;
        regs.clear();

        rational_t min_len2 = -1.0f;

        // build list of regions that can be affected by blast
        build_region_list_radius(&regs, get_primary_region(), pos, 20.0f);

        // build list of entities that are affected by blast
        vector<region_node*>::iterator rn = regs.begin();
        vector<region_node*>::iterator rn_end = regs.end();
        for ( ; rn!=rn_end; ++rn )
        {
          region* r = (*rn)->get_data();

          vector<entity*>::const_iterator i = r->get_entities().begin();
          vector<entity*>::const_iterator i_end = r->get_entities().end();

          for ( ; i!=i_end; ++i )
          {
            entity* e = *i;
            if ( e && (e->is_visible() || e == g_world_ptr->get_hero_ptr()) && e->allow_targeting() )
            {
              rational_t len2 = (pos - e->get_abs_position()).length2();
              if ( min_len2 == -1.0f || len2 < min_len2 )
              {
                target = e;
                min_len2 = len2;
              }
            }
          }
        }
      }

      if(target == NULL)
        guidance_delay = item_owner->get_guidance_delay_with_var();
    }

    if((point_guided || guided) && guidance_delay <= 0.0f)
    {
      if(target != NULL || point_guided)
      {
        if(target != NULL && guided)
          target_pos = target->get_abs_position();

        vector3d delta = target_pos - pos;
        vector3d face = get_abs_po().get_facing();

        rational_t dist = delta.length();
        if(dist > 0.0f)
        {
          delta *= 1.0f/dist;
          rational_t dot_prod = dot(delta, face);

          if(item_owner->get_guided_accuracy() == 1.0f)
          {
            po the_po = po_identity_matrix;
            the_po.set_facing(delta);
            the_po.set_position(pos);

            set_rel_po(the_po);
            physical_ifc()->set_angular_velocity(ZEROVEC);
          }
          else if(item_owner->get_guided_accuracy() > 0.0f && dot_prod <= item_owner->get_guided_accuracy())
          {
            vector3d axis = cross(delta, face);
            axis.normalize();
            physical_ifc()->set_angular_velocity(axis * (PI * item_owner->get_turn_factor()));
          }
          else if(wobble_timer <= 0.0f)
          {
            vector3d axis = vector3d(PLUS_MINUS_ONE, PLUS_MINUS_ONE, PLUS_MINUS_ONE);
            axis.normalize();

            physical_ifc()->set_angular_velocity(axis * (PI * item_owner->get_turn_factor()) * random(2) ? 1 : -1));
            wobble_timer = item_owner->get_wobble_timer_with_var();
          }
        }
      }
      else if(wobble_timer <= 0.0f)
      {
        vector3d axis = vector3d(PLUS_MINUS_ONE, PLUS_MINUS_ONE, PLUS_MINUS_ONE);
        axis.normalize();

        physical_ifc()->set_angular_velocity(axis * (PI * item_owner->get_turn_factor()) * random(2) ? 1 : -1));
        wobble_timer = item_owner->get_wobble_timer_with_var();
      }
    }

    physical_ifc()->set_velocity(ZEROVEC);
    physical_ifc()->apply_force_increment(get_abs_po().get_facing()*force, physical_interface::INSTANT);

    if(accel_delay <= 0.0f)
      force += item_owner->get_accel_factor()*t;
  }

  grenade::frame_advance(t);
}

void rocket::clear()
{
  wobble_timer = 0.0f;
  target = NULL;
  guided = false;
  force = 0.0f;
  guidance_delay = 0.0f;
  accel_delay = 0.0f;

  grenade::clear();
}
*/
