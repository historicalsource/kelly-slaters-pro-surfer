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
#include "osdevopts.h"
#include "script_lib_vector3d.h"
#include "entity_maker.h"

// BIGCULL #include "handheld_item.h"
#include "sound_interface.h"
#include "sound_group.h"
#include "script_access.h"
#include "entityflags.h"
#if defined(TARGET_XBOX)
#include "handheld_item.h"
#endif /* TARGET_XBOX JIV DEBUG */







void handheld_item_effect::spawn(bool remain, const vector3d &pos, const vector3d &face, handheld_item *owner, entity *script_ent, entity *attach_ent, const vector3d &offset)
{
  kill();

  if(static_fx.length() > 0)
  {
    po the_po = po_identity_matrix;

    if(!attach_ent)
    {
      the_po.set_facing(face);
      the_po.set_position(pos);
    }
    else
      the_po.set_position(offset);

    fx = g_entity_maker->acquire_entity( static_fx, ACTIVE_FLAG|NONSTATIC_FLAG );
//    fx = g_entity_maker->create_entity_or_subclass( static_fx.c_str(), entity_id::make_unique_id(), the_po, "fx\\", ACTIVE_FLAG|NONSTATIC_FLAG );
    fx->set_created_entity_default_active_status();
    fx->set_rel_po_no_children(the_po);
    fx->set_visible(true);

    if(attach_ent)
      fx->link_ifc()->set_parent(attach_ent);

    fx->update_abs_po();

    fx->compute_sector(g_world_ptr->get_the_terrain());

    //g_world_ptr->add_dynamic_instanced_entity(fx);

    if(remain)
    {
      fx->force_regions( owner->get_owner() );
    }
    else
    {
      g_world_ptr->make_time_limited(fx, lifetime);
      fx = NULL;
    }
  }

  if(script.length() > 0)
    script::function(script, owner, script_ent, pos, face);

  if(sound.length() > 0)
  {
    if(owner->has_sound_ifc())
    {
      sg_entry* entry = owner->sound_ifc()->get_sound_group_entry(sound);
      if(entry)
      {
        if(remain)
        {
          loop_sound = owner->get_emitter()->create_sound(entry->name);
          loop_sound->set_ranges( (entry->min_dist >= 0.0f) ? entry->min_dist : loop_sound->get_min_dist(), (entry->max_dist >= 0.0f) ? entry->max_dist : loop_sound->get_max_dist() );
        	loop_sound->set_volume( entry->variance(entry->volume, entry->volume_var) );
	        loop_sound->set_pitch( entry->variance(entry->pitch, entry->pitch_var) );

          loop_sound->play(true);
        }
        else
        {
          sound_device::inst()->play_3d_sound( entry->name, pos, entry->variance(entry->volume, entry->volume_var), entry->variance(entry->pitch, entry->pitch_var), entry->min_dist, entry->max_dist);
          loop_sound = NULL;
        }
      }
    }
  }
}

void handheld_item_effect::kill()
{
  if(loop_sound != NULL)
  {
    loop_sound->stop();
    loop_sound->release();
    loop_sound= NULL;
  }

  if(fx != NULL)
  {
    if ( fx->get_entity_pool() == NULL )
      g_world_ptr->destroy_entity( fx );
    else
      g_entity_maker->release_entity( fx );

    fx = NULL;
  }
}

void handheld_item_effect::update(entity *owner)
{
  if(fx && owner)
    fx->force_regions(owner);
}





















handheld_item::handheld_item( const entity_id& _id, unsigned int _flags )
:   item( _id, ENTITY_ITEM, _flags )
{
  init_defaults();
}


handheld_item::handheld_item( const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
:   item( _id, _flavor, _flags )
{
  init_defaults();
}


handheld_item::handheld_item( chunk_file& fs,
            const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
:   item( fs, _id, _flavor, _flags )
{
  init_defaults();
}


handheld_item::handheld_item( const stringx& item_type_filename,
            const entity_id& _id,
            entity_flavor_t _flavor,
            bool _active,
            bool _stationary )
:   item( item_type_filename, _id, _flavor, _active, _stationary)
{
  init_defaults();
}

handheld_item::~handheld_item()
{
}

void handheld_item::init_defaults()
{
  owner = NULL;
  vis_item = NULL;

  item_id = item::name;
//  vis_axis = _Z;

  handheld_flags = 0;

  drawn_limb_name = "BIP01 R HAND";
  holster_limb_name = empty_string;
  drawn_offset = po_identity_matrix;
  holster_offset = po_identity_matrix;

  set_handheld_flag(_HANDHELD_DRAWN, false);

  visible_on_character = false;

#if _VIS_ITEM_DEBUG_HELPER
  render_axis = false;
  drawn_scale = 1.0f;
  drawn_rot = ZEROVEC;
  drawn_pos = ZEROVEC;

  holster_scale = 1.0f;
  holster_rot = ZEROVEC;
  holster_pos = ZEROVEC;
#endif
}

entity* handheld_item::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  handheld_item* newit = NEW handheld_item( _id, _flags );
  newit->copy_instance_data( *((handheld_item *)this) );
  return (entity*)newit;
}


void handheld_item::copy_instance_data( handheld_item& b )
{
  item::copy_instance_data( b );

  handheld_flags = b.handheld_flags;

  drawn_limb_name = b.drawn_limb_name;
  holster_limb_name = b.holster_limb_name;
  drawn_offset = b.drawn_offset;
  holster_offset = b.holster_offset;

  ai_info = b.ai_info;
//  vis_axis = b.vis_axis;

#if _VIS_ITEM_DEBUG_HELPER
  drawn_scale = b.drawn_scale;
  drawn_rot = b.drawn_rot;
  drawn_pos = b.drawn_pos;

  holster_scale = b.holster_scale;
  holster_rot = b.holster_rot;
  holster_pos = b.holster_pos;

  render_axis = b.render_axis;
#endif
}


void handheld_item::read_orientation_chunk( chunk_file& fs )
{
#if !_VIS_ITEM_DEBUG_HELPER
  rational_t drawn_scale = 1.0f;
  vector3d drawn_rot = ZEROVEC;
  vector3d drawn_pos = ZEROVEC;

  rational_t holster_scale = 1.0f;
  vector3d holster_rot = ZEROVEC;
  vector3d holster_pos = ZEROVEC;
#endif

  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "render_axis")
    {
#if _VIS_ITEM_DEBUG_HELPER
      render_axis = true;
#endif
    }
    else if(label == "drawn_limb")
    {
      serial_in( fs, &drawn_limb_name );
    }
    else if(label == "drawn_offset")
    {
      serial_in( fs, &drawn_pos );
    }
    else if(label == "drawn_scale")
    {
      serial_in( fs, &drawn_scale );
    }
    else if(label == "drawn_rotation")
    {
      serial_in( fs, &drawn_rot );
    }
    else if(label == "holster_limb")
    {
      serial_in( fs, &holster_limb_name );
      if(holster_limb_name == "NONE")
        holster_limb_name = empty_string;
    }
    else if(label == "holster_offset")
    {
      serial_in( fs, &holster_pos );
    }
    else if(label == "holster_rotation")
    {
      serial_in( fs, &holster_rot );
    }
    else if(label == "holster_scale")
    {
      serial_in( fs, &holster_scale );
    }
/*
    else if(label == "x_axis")
    {
      vis_axis = _X;
    }
    else if(label == "y_axis")
    {
      vis_axis = _Y;
    }
    else if(label == "z_axis")
    {
      vis_axis = _Z;
    }
    else if(label == "-x_axis")
    {
      vis_axis = _NEGX;
    }
    else if(label == "-y_axis")
    {
      vis_axis = _NEGY;
    }
    else if(label == "-z_axis")
    {
      vis_axis = _NEGZ;
    }
*/
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in orientation section" );
    }

    serial_in( fs, &label );
  }

  po tmp1;

  drawn_offset = po_identity_matrix;
  drawn_offset.set_position( drawn_pos );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_x( drawn_rot.x * (PI / 180.0f) );
  drawn_offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_y( drawn_rot.y * (PI / 180.0f) );
  drawn_offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_z( drawn_rot.z * (PI / 180.0f) );
  drawn_offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_scale( vector3d(drawn_scale, drawn_scale, drawn_scale) );
  drawn_offset.add_increment( &tmp1 );



  holster_offset = po_identity_matrix;
  holster_offset.set_position( holster_pos );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_x( holster_rot.x * (PI / 180.0f) );
  holster_offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_y( holster_rot.y * (PI / 180.0f) );
  holster_offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_z( holster_rot.z * (PI / 180.0f) );
  holster_offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_scale( vector3d(holster_scale, holster_scale, holster_scale) );
  holster_offset.add_increment( &tmp1 );
}

void handheld_item::read_ai_info_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    label.to_lower();

    if(label == "min_range")
    {
      serial_in(fs, &ai_info.min_dist);
    }
    else if(label == "max_range")
    {
      serial_in(fs, &ai_info.max_dist);
    }
    else if(label == "rifle")
    {
      ai_info.set_flag(ai_item_info::_RIFLE, true);
      ai_info.set_flag(ai_item_info::_SHOTGUN, false);
      ai_info.set_flag(ai_item_info::_PISTOL, false);
      ai_info.set_flag(ai_item_info::_LAUNCHER, false);
      ai_info.set_flag(ai_item_info::_HAND_GRENADE, false);
    }
    else if(label == "shotgun")
    {
      ai_info.set_flag(ai_item_info::_RIFLE, false);
      ai_info.set_flag(ai_item_info::_SHOTGUN, true);
      ai_info.set_flag(ai_item_info::_PISTOL, false);
      ai_info.set_flag(ai_item_info::_LAUNCHER, false);
      ai_info.set_flag(ai_item_info::_HAND_GRENADE, false);
    }
    else if(label == "pistol")
    {
      ai_info.set_flag(ai_item_info::_RIFLE, false);
      ai_info.set_flag(ai_item_info::_SHOTGUN, false);
      ai_info.set_flag(ai_item_info::_PISTOL, true);
      ai_info.set_flag(ai_item_info::_LAUNCHER, false);
      ai_info.set_flag(ai_item_info::_HAND_GRENADE, false);
    }
    else if(label == "launcher")
    {
      ai_info.set_flag(ai_item_info::_RIFLE, false);
      ai_info.set_flag(ai_item_info::_SHOTGUN, false);
      ai_info.set_flag(ai_item_info::_PISTOL, false);
      ai_info.set_flag(ai_item_info::_LAUNCHER, true);
      ai_info.set_flag(ai_item_info::_HAND_GRENADE, false);
    }
    else if(label == "hand_grenade")
    {
      ai_info.set_flag(ai_item_info::_RIFLE, false);
      ai_info.set_flag(ai_item_info::_SHOTGUN, false);
      ai_info.set_flag(ai_item_info::_PISTOL, false);
      ai_info.set_flag(ai_item_info::_LAUNCHER, false);
      ai_info.set_flag(ai_item_info::_HAND_GRENADE, true);
    }
    else if(label == "close_range" || label == "near_range")
    {
      ai_info.set_flag(ai_item_info::_NEAR_RANGE, true);
      ai_info.set_flag(ai_item_info::_MID_RANGE, false);
      ai_info.set_flag(ai_item_info::_FAR_RANGE, false);
    }
    else if(label == "mid_range")
    {
      ai_info.set_flag(ai_item_info::_NEAR_RANGE, false);
      ai_info.set_flag(ai_item_info::_MID_RANGE, true);
      ai_info.set_flag(ai_item_info::_FAR_RANGE, false);
    }
    else if(label == "far_range" || label == "long_range")
    {
      ai_info.set_flag(ai_item_info::_NEAR_RANGE, false);
      ai_info.set_flag(ai_item_info::_MID_RANGE, false);
      ai_info.set_flag(ai_item_info::_FAR_RANGE, true);
    }
    else if(label == "dual")
    {
      ai_info.set_flag(ai_item_info::_DUAL_WEAPON, true);
    }
    else if(label == "single")
    {
      ai_info.set_flag(ai_item_info::_DUAL_WEAPON, false);
    }
    else if(label == "automatic")
    {
      ai_info.set_flag(ai_item_info::_AUTOMATIC, true);
    }
    else if(label == "pump_action")
    {
      ai_info.set_flag(ai_item_info::_PUMP_ACTION, true);
    }
    else if(label == "non_pump_action")
    {
      ai_info.set_flag(ai_item_info::_PUMP_ACTION, false);
    }
    else if(label == "non_automatic")
    {
      ai_info.set_flag(ai_item_info::_AUTOMATIC, false);
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in orientation section" );
    }

    serial_in( fs, &label );
  }
}


bool handheld_item::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  return(item::handle_enx_chunk( fs, label ));
}


void handheld_item::create_visual_item()
{
  if(!vis_item)
  {
    vis_item = NEW visual_item(entity_id::make_unique_id(), (unsigned int)(EFLAG_MISC_NONSTATIC));
    vis_item->copy_visrep(this);

  #if _VIS_ITEM_DEBUG_HELPER
    vis_item->render_axis = render_axis;
  #endif

    assert(owner);
    vis_item->set_rel_position(owner->get_abs_position());

    g_world_ptr->add_dynamic_instanced_entity(vis_item);
    vis_item->set_owner( owner );
  }
}

bool handheld_item::is_brain_weapon() const       { return get_handheld_flag(_HANDHELD_BRAIN_WEAPON); }
bool handheld_item::is_common_brain_weapon() const{ return get_handheld_flag(_HANDHELD_COMMON_BRAIN_WEAPON); }
bool handheld_item::is_a_radio_detonator() const  { return get_handheld_flag(_HANDHELD_RADIO_DETONATOR); }

visual_item* handheld_item::get_visual_item() const { return vis_item; }
entity* handheld_item::get_owner() const { return owner; }
void handheld_item::set_owner(entity *own) { assert(own); owner = own; }

void handheld_item::holster(bool make_visible)
{
  if ( !vis_item )
    create_visual_item();

  if ( holster_limb_name.length() > 0 )
  {
    if(owner->get_flavor() == ENTITY_CONGLOMERATE)
    {
      #if _VIS_ITEM_DEBUG_HELPER
      vis_item->set_placement( owner, holster_limb_name, holster_scale, holster_pos, holster_rot, false );
      #else
      vis_item->set_placement( owner, holster_limb_name, holster_offset, false );
      #endif

      if(make_visible)
      {
        vis_item->set_visible( true );
        vis_item->compute_sector( g_world_ptr->get_the_terrain() );
        visible_on_character = true;
      }
    }
    else
    {
      if(make_visible)
      {
        vis_item->set_visible( false );
        visible_on_character = false;
      }
    }
  }
  else
  {
    vis_item->set_visible( false );
    visible_on_character = false;
  }

  set_handheld_flag( _HANDHELD_DRAWN, false );
}

void handheld_item::draw(bool make_visible)
{
  if ( !vis_item )
    create_visual_item();

  if(drawn_limb_name.length() <= 0 && owner->get_flavor() == ENTITY_CONGLOMERATE)
    warning("HandHeld item %s needs a drawn_limb in the orientation chunk for character '%s'!", item::get_name().c_str(), owner->get_name().c_str());

  if ( owner->get_flavor() == ENTITY_CONGLOMERATE)
  {
    #if _VIS_ITEM_DEBUG_HELPER
    vis_item->set_placement( owner, drawn_limb_name, drawn_scale, drawn_pos, drawn_rot, true );
    #else
    vis_item->set_placement( owner, drawn_limb_name, drawn_offset, true );
    #endif

    if(make_visible)
    {
      vis_item->set_visible( true );
      vis_item->compute_sector( g_world_ptr->get_the_terrain() );

      visible_on_character = true;
    }
  }
  else
  {
    if(make_visible)
    {
      vis_item->set_visible( false );
      visible_on_character = false;
    }
  }

  set_handheld_flag( _HANDHELD_DRAWN, true );
}

void handheld_item::hide()
{
  if ( !vis_item )
    create_visual_item();

  vis_item->set_visible( false );
  visible_on_character = false;
}

void handheld_item::show()
{
  if ( !vis_item )
    create_visual_item();

  vis_item->set_visible( true );
  visible_on_character = true;
}

void handheld_item::set_visibility(bool val)
{
  if ( !vis_item )
    create_visual_item();

  vis_item->set_visible( val && visible_on_character );
}


void handheld_item::detach()
{
  if ( !vis_item )
    create_visual_item();

  vis_item->link_ifc()->clear_parent();
  vis_item->set_visible( false );
}

void handheld_item::frame_advance( time_value_t t )
{
  item::frame_advance( t );
  // make sure vis_item is forced to same region(s) as owner
  if ( vis_item && owner )
    vis_item->force_regions( owner );
}

vector3d handheld_item::get_vis_facing()
{
  if(vis_item)
  {
/*
    switch(vis_axis)
    {
      case _X:
        return(vis_item->get_abs_po().get_x_facing());
        break;

      case _Y:
        return(vis_item->get_abs_po().get_y_facing());
        break;

      case _Z:
        return(vis_item->get_abs_po().get_z_facing());
        break;

      case _NEGX:
        return(-vis_item->get_abs_po().get_x_facing());
        break;

      case _NEGY:
        return(-vis_item->get_abs_po().get_y_facing());
        break;

      case _NEGZ:
        return(-vis_item->get_abs_po().get_z_facing());
        break;

      default:
        break;
    }
*/
    return(vis_item->get_abs_po().get_facing());
  }

  return(owner != NULL ? owner->get_abs_po().get_z_facing() : get_abs_po().get_z_facing());
}

vm_thread *handheld_item::exec_item_script_function( const stringx& function_name, const vector3d &pos, const vector3d &norm )
{
  vm_thread* nt = NULL;

  if(function_name.length() > 0)
  {
    stringx actual_name = function_name + "(item,vector3d,vector3d)";
    actual_name.to_lower();

    script_object* so = g_world_ptr->get_current_level_global_script_object();
    script_object::instance* inst = g_world_ptr->get_current_level_global_script_object_instance();

    if(so != NULL && inst != NULL)
    {
      int fidx = so->find_func(actual_name);

      if(fidx >= 0)
      {
        nt = inst->add_thread( &so->get_func(fidx) );

        item *itm = this;
        nt->get_data_stack().push( (char*)&itm, 4 );

        vm_vector3d_t vec = pos;
        nt->get_data_stack().push( (char*)&vec, sizeof(vec) );
        vec = norm;
        nt->get_data_stack().push( (char*)&vec, sizeof(vec) );

        // run the NEW thread immediately
        inst->run_single_thread( nt, false );
      }
      else
        warning("Script function '%s' not found for handheld_item '%s'!", actual_name.c_str(), get_name().c_str());
    }
  }

  return nt;
}


