////////////////////////////////////////////////////////////////////////////////
/*
  item.cpp

  Items in game.  Inventory management and Item effects

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "osalloc.h"

//!#include "character.h"
#include "stringx.h"
#include "item.h"
#include "wds.h"
#include "msgboard.h"
#include "terrain.h"
#include "hwaudio.h"
#include "script_object.h"
//!#include "attrib.h"
#include "vm_thread.h"
#include "controller.h"
#include "interface.h"

#if _CONSOLE_ENABLE
#include "console.h"
#endif

//#include "dread_net.h"
#include "entity_maker.h"
#include "entityflags.h"
// BIGCULL #include "spiderman_controller.h"
// BIGCULL #include "damage_interface.h"
#include "sound_interface.h"


////////////////////////////////////////////////////////////////
//  item
////////////////////////////////////////////////////////////////


item::item( const entity_id& _id, unsigned int _flags )
  : entity( _id, ENTITY_ITEM, _flags )
{
  usage_type = INVALID;
  picked_up = false;
  pickup_timer = 0;

  set_radius(.25f);
  count = default_count = 1;
//  need_to_initialize = false;

#if _ENABLE_WORLD_EDITOR
  original_count = count;
#endif

  max_num = 10;

  icon_scale = 1.0f;

  interface_orientation = 225.0f;

  //set_gravity( false );
  //set_physical( false );

//  dread_net_use_cue = dread_net::UNDEFINED_AV_CUE;

  preload_script_called = false;
  item_script_called = false;
  linked = false;
}


item::item( const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
  : entity( _id, _flavor, _flags )
{
  usage_type = INVALID;
  picked_up = false;
  pickup_timer = 0;

  set_radius(.25f);
  default_count = count = 1;
//  need_to_initialize = false;

#if _ENABLE_WORLD_EDITOR
  original_count = count;
#endif

  max_num = 10;

  icon_scale = 1.0f;

  interface_orientation = 225.0f;

//  set_gravity( false );
//  set_physical( false );

//  dread_net_use_cue = dread_net::UNDEFINED_AV_CUE;

  preload_script_called = false;
  item_script_called = false;
  linked = false;
}


item::~item()
{
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

item::item( chunk_file& fs,
            const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
  : entity( fs, _id, _flavor, _flags )
{
  // TODO: handle flags correctly under NEWENT
  usage_type = INVALID;
  picked_up = false;
  pickup_timer = 0;

  set_radius(.25f);
  default_count = count = 1;
//  need_to_initialize = false;

#if _ENABLE_WORLD_EDITOR
  original_count = count;
#endif

  max_num = 10;

  icon_scale = 1.0f;

  interface_orientation = 225.0f;

//  set_gravity( false );
//  set_physical( false );

  set_active( true );
  set_stationary( false );

//  dread_net_use_cue = dread_net::UNDEFINED_AV_CUE;

  preload_script_called = false;
  item_script_called = false;
  linked = false;
}

void item::read_item_data( chunk_file& fs, stringx& label )
{
  // Now the actual data

  // get item name from filename
  filespec spec( fs.get_name() );
  name = spec.name;
  name.to_upper();

  stringx usage_type_name;
  serial_in( fs, &usage_type_name );
  if ( usage_type_name == "INSTANT" )
    usage_type = INSTANT;
  else if ( usage_type_name == "INVENTORY" )
    usage_type = INVENTORY;
  else if ( usage_type_name == "UTILITY" )
    usage_type = UTILITY;
  else if ( usage_type_name == "PERMANENT" )
    usage_type = PERMANENT;
  else if ( usage_type_name == "GUN" )
    usage_type = GUN;
  else if ( usage_type_name == "MELEE" )
    usage_type = MELEE;
  else if ( usage_type_name == "THROWN" )
    usage_type = THROWN;
  else if ( usage_type_name == "AMMO" )
    usage_type = AMMO;
  else if ( usage_type_name == "HEALTH" )
    usage_type = HEALTH;
  else if ( usage_type_name == "ARMOR" )
    usage_type = ARMOR;
  else if ( usage_type_name == "ENERGY" )
    usage_type = ENERGY;
  else
    error( fs.get_name() + ": invalid usage type: " + usage_type_name );

  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
  {
    if ( label == "count" )
    {
      serial_in( fs, &default_count );
      count = default_count;

      #if _ENABLE_WORLD_EDITOR
        original_count = count;
      #endif
    }
    else if ( label == "max" )
    {
      serial_in( fs, &max_num );
    }
    else if ( label == "icon_scale" )
    {
      serial_in( fs, &icon_scale );
    }
    else if ( label == "interface_orientation" )
    {
      serial_in( fs, &interface_orientation );
    }
/*
    else if ( label == "dread_net_cue" )
    {
      stringx cue;
      serial_in(fs, &cue);
      cue.to_upper();

      dread_net_use_cue = dread_net::get_cue_type(cue);
    }
*/
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in item section" );
    }
  }
}




bool item::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "item:")
  {
    read_item_data( fs, label );

    return(true);
  }
  else if(label == "linked_items:")
  {
    for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
    {
      entity* itm;
      stringx entity_name = get_fname_wo_ext( label );
      entity_name.to_upper();
      stringx entity_dir = get_dir( label );
      itm = g_entity_maker->create_entity_or_subclass( entity_name,
                                                    entity_id::make_unique_id(),
                                                    po_identity_matrix,
                                                    entity_dir,
                                                    ACTIVE_FLAG | NONSTATIC_FLAG );

      if ( !itm->is_an_item() )
      {
        error( get_name() + " linked item: entity " + entity_name + " is not an item" );
      }
      else
      {
        ((item *)itm)->set_linked(true);
        add_item((item *)itm);
      }
    }

    return true;
  }
  else
    return entity::handle_enx_chunk(fs, label);
}



///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

item::item( const stringx& item_type_filename,
            const entity_id& _id,
            entity_flavor_t _flavor,
            bool _active,
            bool _stationary,
            bool delete_stream)
  :   entity( _id, _flavor )
{
  //vr_pmesh::push_optimize_static_mesh_for_d3d_disallowed();
  assert (my_fs == NULL);

  set_active( true );
  set_stationary( false );

  my_fs = NEW chunk_file();
  stringx ent_name(item_type_filename+".ent");
  my_fs->open(ent_name);

  stringx dummy;
  // Strip Entity Flavor
  serial_in(*my_fs, &dummy);

  serial_in( *my_fs, &dummy );
  assert( dummy == "item:" );
  read_item_data( *my_fs, dummy );

  // This is probably holding place for a value from the file <<<<.
  set_radius(0.25f);
  default_count = count = 1;
//  need_to_initialize = false;

#if _ENABLE_WORLD_EDITOR
  original_count = count;
#endif

  if ( delete_stream )
  {
    assert(my_fs);
    delete my_fs;
    my_fs = NULL;
  }

  picked_up = false;
  pickup_timer = 0;

  max_num = 10;

  icon_scale = 1.0f;

  interface_orientation = 225.0f * PI / 180.0f;

//  set_gravity( false );
//  set_physical( false );

//  dread_net_use_cue = dread_net::UNDEFINED_AV_CUE;

  preload_script_called = false;
  item_script_called = false;
  linked = false;
  //vr_pmesh::pop_optimize_static_mesh_for_d3d_disallowed();
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* item::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  item* newit = NEW item( _id, _flags );
  newit->copy_instance_data( *this );
  return (entity*)newit;
}


void item::copy_instance_data( const item& b )
{
  entity::copy_instance_data( b );

  usage_type = b.usage_type;
  default_count = count = b.default_count;

#if _ENABLE_WORLD_EDITOR
  original_count = count;
#endif

  max_num = b.max_num;
  name = b.name;

  icon_scale = b.icon_scale;

  interface_orientation = b.interface_orientation;

//  set_gravity( false );
//  set_physical( false );

  set_active( true );
  set_stationary( false );

//  dread_net_use_cue = b.dread_net_use_cue;
  linked = b.linked;
}


// This function allows the entity to any post-level-load initialization
void item::initialize()
{
//  if ( need_to_initialize )
  {
    spawn_item_script();
  }
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

// return true if given item is considered the same (for inventory purposes)
bool item::is_same_item( const item& b ) const
{
  if ( get_usage_type() != b.get_usage_type() )
    return false;
  switch ( get_usage_type() )
  {
    case INVENTORY:
    case UTILITY:
    case PERMANENT:
      return false;
    case GUN:
    case THROWN:
      return get_name() == b.get_name();
    default:
      break;
  }
  return true;
}


void item::render(camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct)
{
  if ( count > 0 )
  {
    entity::render( camera_link, detail, flavor, entity_translucency_pct );
  }
}


bool item::check_for_pickup()
{
  bool outval = false;
  #ifdef GCCULL
  static hires_clock_t gulp_timer;

  pickup_timer -= g_world_ptr->get_cur_time_inc();

  if ( pickup_timer < 0 )
    pickup_timer = 0;

  if ( count && pickup_timer==0 && !was_spawned())
  {
    if ( g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->get_hit_points() > 0 && g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->point_in_radius( get_abs_position() ) )
    {
      int old_num = get_number();

      outval = give_to_entity( g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player()) );

      // item pick up effects
      extern int global_frame_counter;
      if ( ( outval || get_number()!=old_num ) && global_frame_counter > 5 )
      {
        // Don't over-do the 'gulp' item pick-up sound if we get a lot of stuff at once.
        if (gulp_timer.elapsed() > 0.2f)
        {
          gulp_timer.reset();
          #ifdef GCCULL
          if(g_world_ptr->get_hero_ptr(0)->has_sound_ifc())
          {
            static pstring pickup("PICK_UP");
            g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->sound_ifc()->play_3d_sound_grp(pickup);
          }
          #endif
        }
      }

      if ( outval )
        remove_from_terrain();
    }
  }
	#endif
	
  return outval;
}

// returns true if the inventory did NOT already have a copy of the item
bool item::give_to_entity(entity *target)
{
  bool outval = false;

  // first decide whether this is the first time an item of this kind has been picked up
  switch ( get_usage_type() )
  {
    case UTILITY:
    case INVENTORY:
    case GUN:
    case THROWN:
      outval = target->find_like_item(this)? false : true;
      break;
    case INSTANT:
    case PERMANENT:
      outval = true;
      break;
    case AMMO:
    case HEALTH:
    case ARMOR:
    case ENERGY:
      outval = false;
      break;

    default:
      assert(0);
      break;
  }

  // if the character had not previously picked up an item of this kind,
  // automatically spawn and run the special X_callbacks(item) script function
  if ( outval )
  {
    if ( g_world_ptr->get_current_level_global_script_object() )
      spawn_item_script();
//    else
//      need_to_initialize = true;
  }

  int old_count = get_number();
  bool force_raise_signal = false;

  switch( get_usage_type() )
  {
    case INSTANT:
      #if _CONSOLE_ENABLE
      console_log("%s picked up a '%s' (instant)", target->get_name().c_str(), get_name().c_str());
      #endif
      apply_effects( target );
      set_count( 0 );
      break;

    case UTILITY:
    case INVENTORY:
    case GUN:
    case THROWN:
      {
#if _CONSOLE_ENABLE
        int old = get_number();
#endif

        bool new_item = target->add_item( this );

        if ( new_item )
        {
          // this entity needs to frame_advance() even when ineligible by the usual rules
          g_world_ptr->guarantee_active( this );
          force_raise_signal = true;
        }

#if _CONSOLE_ENABLE
        if(new_item || old != get_number())
          console_log("%s picked up a '%s' (x%d)", target->get_name().c_str(), get_name().c_str(), new_item ? get_number() : old - get_number());
#endif
      }
      break;

    case AMMO:
      {
#if 0 // BIGCULL
        if(target->is_hero())
        {
          g_spiderman_controller_ptr->set_webbing_carts(g_spiderman_controller_ptr->get_webbing_carts() + count);
          set_count(0);
        }
#endif
      }
      break;
/*
      if(target->get_ammo_points() + get_count() > target->get_max_ammo_points())
      {
        int delta = (target->get_max_ammo_points() - target->get_ammo_points());
        if(delta)
        {
          target->inc_ammo_points(delta);
          set_count(get_count() - delta);

          #if _CONSOLE_ENABLE
          console_log("%s picked up ammo (x%d, left x%d)", target->get_name().c_str(), delta, get_count());
          #endif
        }
      }
      else
      {
        #if _CONSOLE_ENABLE
        console_log("%s picked up ammo (x%d)", target->get_name().c_str(), get_count());
        #endif

        target->inc_ammo_points(get_count());
        set_count(0);
      }
      break;
*/

    case HEALTH:
      {
        error ("Health items not supported in KS.");
#if 0 // BIGCULL
        if(target->has_damage_ifc())
        {
          target->damage_ifc()->inc_hit_points(count);
          set_count(0);
        }
#endif // BIGCULL
      }
      break;
/*
      if(target->get_hit_points() + get_count() > target->get_full_hit_points())
      {
        int delta = (target->get_full_hit_points() - target->get_hit_points());
        if(delta)
        {
          target->inc_hit_points(delta);
          set_count(get_count() - delta);

          #if _CONSOLE_ENABLE
          console_log("%s picked up health (x%d, left x%d)", target->get_name().c_str(), delta, get_count());
          #endif
        }
      }
      else
      {
        #if _CONSOLE_ENABLE
        console_log("%s picked up health (x%d)", target->get_name().c_str(), get_count());
        #endif

        target->inc_hit_points(get_count());
        set_count(0);
      }
      break;
*/

    case ARMOR:
      {
        error ("Armor items not supported in KS.");
#if 0 // BIGCULL
        if(target->has_damage_ifc())
        {
          target->damage_ifc()->inc_armor_points(count);
          set_count(0);
        }
#endif //BIGCULL
      }
      break;
/*
      if ( target->get_armor_points() < target->get_full_armor_points() )
      {
        #if _CONSOLE_ENABLE
        console_log("%s picked up armor (x%d)", target->get_name().c_str(), get_count());
        #endif

        target->inc_armor_points( get_count() );
        set_count( 0 );
      }
      break;
*/
/*

    case ENERGY:
      if ( target->get_nanotech_energy() < target->get_full_nanotech_energy() )
      {
        #if _CONSOLE_ENABLE
        console_log("%s picked up nanotech energy (x%d)", target->get_name().c_str(), get_count());
        #endif

        target->inc_nanotech_energy( get_count() );
        set_count( 0 );
      }
      break;
*/
/*
    case PERMANENT:
    {
      #if _CONSOLE_ENABLE
      console_log("%s picked up a '%s' (perm)", target->get_name().c_str(), get_name().c_str());
      #endif

      target->add_permanent_item(this);
      apply_effects( target );

      force_raise_signal = true;
    }
    break;
*/
    default:
      assert(0);
      break;
  }

  if(old_count != get_number() || force_raise_signal)
    raise_signal( PICKUP );

  if ( outval || get_count()==0 )
    picked_up = true;

  if ( target == g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player()) )
  {
//    target->get_player_controller()->add_inv_item();
    disgorge_items(target);
  }

//  if(g_world_ptr->get_dread_net())
//    g_world_ptr->get_dread_net()->add_cue(dread_net::get_cue_type("PICKUP_ITEM"), target);

  return outval;
}


void item::spawn_preload_script()
{
  if(!preload_script_called)
  {
    preload_script_called = true;

    // there might be a script function for preloading additional assets needed by item
    stringx preload_func_name = get_name() + "_preload()";
    preload_func_name.to_lower();
    script_object* gso = g_world_ptr->get_current_level_global_script_object();
    assert( gso );
    int fidx = gso->find_func( preload_func_name );
    if ( fidx >= 0 )
    {
      script_object::instance* gsoi = g_world_ptr->get_current_level_global_script_object_instance();
      assert( gsoi );
      // spawn thread for function
      vm_thread* newt = gsoi->add_thread( &gso->get_func(fidx) );
      // run the NEW thread immediately
      gsoi->run_single_thread( newt, false );
    }
  }
}


void item::spawn_item_script()
{
  if(!item_script_called)
  {
    item_script_called = true;

    // search for matching script function
    stringx fname = get_name() + "_callbacks(item)";
    fname.to_lower();
    script_object* gso = g_world_ptr->get_current_level_global_script_object();
    int fidx = gso->find_func( fname );
    if ( fidx >= 0 )
    {
      script_object::instance* gsoi = g_world_ptr->get_current_level_global_script_object_instance();
      // spawn thread for function
      char parms[4];
      *(unsigned int*)parms = (unsigned int)this;
      vm_thread* newt = gsoi->add_thread( &gso->get_func(fidx), parms );
      // run the NEW thread immediately
      gsoi->run_single_thread( newt, false );
    }
  }
}


void item::preload()
{
  entity::preload();
  spawn_preload_script();
}


bool item::is_picked_up()
{
  return picked_up;
}


const rational_t ITEM_ROTATION_SPEED = PI;

void item::frame_advance( time_value_t t )
{
  if ( !is_picked_up() )
  {
    if ( has_physical_ifc() )
      entity::frame_advance( t );
    else
    {
      // rotate item about absolute (non-affine) Y axis
      po r;
      r.set_rotate_y( ITEM_ROTATION_SPEED * t );
      po newpo = get_rel_po();
      vector3d p = newpo.get_position();
      newpo.set_position( ZEROVEC );

      fast_po_mul(newpo, newpo, r);
//      newpo = newpo * r;

      newpo.fixup();
      newpo.scale( icon_scale );
      newpo.set_position( p );
      set_rel_po( newpo );
    }
  }
}


void item::apply_effects( entity* target )
{
  raise_signal( USE );

//  if(dread_net_use_cue != dread_net::UNDEFINED_AV_CUE)
//    g_world_ptr->get_dread_net()->add_cue((dread_net::eAVCueType)dread_net_use_cue, target);
}



// This function allows parsing instance data according to entity type.
// If it recognizes the given chunk_flavor as a chunk of instance
// data for this type, it will parse the data; otherwise it will hand
// the parsing up to the parent class.
bool item::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("item") )
  {
    stringx cf;
    for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
    {
      if ( cf == stringx("count") )
      {
        serial_in( fs, &count );

        #if _ENABLE_WORLD_EDITOR
          original_count = count;
        #endif
      }
    }
  }
  else
    return entity::parse_instance( pcf, fs );

  return true;
}

/*!
int item::get_max_allowed(character *chr)
{
  return max_num;
}
!*/

/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void item::register_signals()
{
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "item_signals.h"
  #undef MAC
}

static const char* item_signal_names[] =
{
  #define MAC(label,str)  str,
  #include "item_signals.h"
  #undef MAC
};

unsigned short item::get_signal_id( const char *name )
{
  int idx;

  for( idx = 0; idx < (int)(sizeof(item_signal_names)/sizeof(char*)); ++idx )
  {
    int offset = strlen(item_signal_names[idx])-strlen(name);

    if( offset > (int)strlen( item_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&item_signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
  }

  // not found
  return entity::get_signal_id( name );
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* item::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= PARENT_SYNC_DUMMY )
    return entity::get_signal_name( idx );
  return item_signal_names[idx-PARENT_SYNC_DUMMY-1];
}


#if _VIS_ITEM_DEBUG_HELPER
beam *visual_item::visual_item_beamer = NULL;
#endif

visual_item::visual_item( const entity_id& _id, unsigned int _flags )
  :   entity( _id, ENTITY_ENTITY, _flags )
{
#if _VIS_ITEM_DEBUG_HELPER
  render_axis = false;
#endif

//  set_gravity( false );
//  set_physical( false );
  owner = NULL;
}

visual_item::~visual_item()
{
#if _VIS_ITEM_DEBUG_HELPER
  if ( visual_item_beamer != NULL )
  {
    delete visual_item_beamer;
    visual_item_beamer = NULL;
  }
#endif
}

void visual_item::set_placement(entity *_owner, const stringx& limb, const po& offset, bool drawn)
{
  owner = _owner;

  assert(owner);

  switch(owner->get_flavor())
  {
/*!
    case ENTITY_CHARACTER:
    {
      character *tmp_owner = (character *)owner;
      if(tmp_owner->is_active())
      {
        entity *lb;

        anim_id_t aid = anim_id_manager::inst()->anim_id( limb );

        if ( tmp_owner->limb_valid(aid) )
          lb = tmp_owner->limb_ptr(aid)->get_body();
        else if((lb = tmp_owner->has_limb_tree() ? tmp_owner->nonlimb_ptr(aid) : NULL) == NULL)
        {
          aid = drawn ? RIGHT_HAND : RIGHT_UPPER_LEG;

          if ( tmp_owner->limb_valid(aid) )
            lb = tmp_owner->limb_ptr(aid)->get_body();
//          else
//            error("No valid attach limb/nonlimb");
        }

        set_parent( lb );
      }
    }
    break;
!*/
    case ENTITY_CONGLOMERATE:
    {
      conglomerate *conglom = (conglomerate *)owner;

      entity *node = conglom->get_member(limb);
      if ( node )
        link_ifc()->set_parent( node );
      else
        link_ifc()->set_parent( owner );
    }
    break;

    default:
    {
      link_ifc()->set_parent(owner);
    }
    break;
  }

  set_rel_po(offset);

  set_visible(true);
}


#if _VIS_ITEM_DEBUG_HELPER
bool g_render_vis_item_debug_info = false;

void visual_item::set_placement(entity *_owner, const stringx& limb, rational_t s, const vector3d& p, const vector3d& r, bool drawn)
{
  owner = _owner;

  assert(owner);

  switch(owner->get_flavor())
  {
/*!    case ENTITY_CHARACTER:
    {
      character *tmp_owner = (character *)owner;
      if(tmp_owner->is_active())
      {
        entity* lb = tmp_owner;

        anim_id_t aid = anim_id_manager::inst()->anim_id( limb );
        if ( tmp_owner->limb_valid(aid) )
          lb = tmp_owner->limb_ptr(aid)->get_body();
        else if((lb = tmp_owner->has_limb_tree() ? tmp_owner->nonlimb_ptr(aid) : NULL) == NULL)
        {
          aid = drawn ? RIGHT_HAND : RIGHT_UPPER_LEG;

          if ( tmp_owner->limb_valid(aid) )
            lb = tmp_owner->limb_ptr(aid)->get_body();
//          else
//            error("No valid attach limb/nonlimb");
        }

        set_parent( lb );
      }
    }
    break;
!*/
    case ENTITY_CONGLOMERATE:
    {
      conglomerate *conglom = (conglomerate *)owner;

      entity *node = conglom->get_member( limb );
      if ( node )
        link_ifc()->set_parent( node );
      else
        link_ifc()->set_parent( owner );
    }
    break;

    default:
    {
      link_ifc()->set_parent(owner);
    }
    break;
  }

  alter_placement( s, p, r );
  set_visible( true );
}

void visual_item::alter_placement(rational_t s, const vector3d& p, const vector3d& r)
{
  po offset;
  po tmp1;

  offset = po_identity_matrix;
  offset.set_position( p );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_x( DEG_TO_RAD(r.x) );
  offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_y( DEG_TO_RAD(r.y) );
  offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_rotate_z( DEG_TO_RAD(r.z) );
  offset.add_increment( &tmp1 );

  tmp1 = po_identity_matrix;
  tmp1.set_scale( vector3d(s, s, s) );
  offset.add_increment( &tmp1 );

  set_rel_po(offset);
}

#endif


light_manager* visual_item::get_light_set()
{
  if(owner)
  {
    if(owner->is_visible())
      return owner->get_light_set();
  }
  return NULL;
}

render_flavor_t visual_item::render_passes_needed() const
{
  render_flavor_t passes = entity::render_passes_needed();
/*!
  if ( owner && owner->is_a_character() )
  {
    character* c = (character*)owner;
    if ( c->is_visible()
      && c->is_alive_or_dying()
      && c->render_passes_needed() & RENDER_TRANSLUCENT_PORTION
      && ( c->get_death_fade_alpha()<1.0f || c->get_translucency_for_stealth()<1.0f )
      )
    {
      passes = RENDER_TRANSLUCENT_PORTION;
    }
  }
!*/
  return passes;
}

void visual_item::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
#if _VIS_ITEM_DEBUG_HELPER
  if ( render_axis )
  {
    if ( visual_item_beamer == NULL )
      visual_item_beamer = NEW beam(entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC));

    if ( is_visible() )
    {
      vector3d vec = get_abs_position();
      entity *par = (entity *)link_ifc()->get_parent();

      visual_item_beamer->set_point_to_point(vec, vec + (par ? par->get_abs_po().fast_8byte_non_affine_xform(XVEC) : XVEC));
      visual_item_beamer->set_beam_color(color32_red);
      visual_item_beamer->frame_advance(0);
      visual_item_beamer->render(visual_item_beamer->get_max_polys(), RENDER_TRANSLUCENT_PORTION | ((visual_item_beamer->get_flags()&EFLAG_MISC_NO_CLIP_NEEDED)?0:RENDER_CLIPPED_FULL_DETAIL), 1.0f);

      visual_item_beamer->set_point_to_point(vec, vec + (par ? par->get_abs_po().fast_8byte_non_affine_xform(YVEC) : YVEC));
      visual_item_beamer->set_beam_color(color32_green);
      visual_item_beamer->frame_advance(0);
      visual_item_beamer->render(visual_item_beamer->get_max_polys(), RENDER_TRANSLUCENT_PORTION | ((visual_item_beamer->get_flags()&EFLAG_MISC_NO_CLIP_NEEDED)?0:RENDER_CLIPPED_FULL_DETAIL), 1.0f);

      visual_item_beamer->set_point_to_point(vec, vec + (par ? par->get_abs_po().fast_8byte_non_affine_xform(ZVEC) : ZVEC));
      visual_item_beamer->set_beam_color(color32_blue);
      visual_item_beamer->frame_advance(0);
      visual_item_beamer->render(visual_item_beamer->get_max_polys(), RENDER_TRANSLUCENT_PORTION | ((visual_item_beamer->get_flags()&EFLAG_MISC_NO_CLIP_NEEDED)?0:RENDER_CLIPPED_FULL_DETAIL), 1.0f);
    }
  }
#endif

  entity::render( camera_link, detail, flavor, entity_translucency_pct );

/*!
  if ( owner && owner->is_a_character() )
  {
    character* c = (character*)owner;
    if ( c->is_visible() && c->is_alive_or_dying() )
    {
      float new_translucency_pct = entity_translucency_pct;
      #if !defined(TARGET_MKS)
      if ( flavor & RENDER_TRANSLUCENT_PORTION )
      #endif
      {
        new_translucency_pct *= c->get_translucency_for_stealth();
        new_translucency_pct = min( new_translucency_pct, c->get_death_fade_alpha() );
      }
      entity::render( detail, flavor, new_translucency_pct );
    }
  }
!*/
}

















morphable_item::morphable_item( const entity_id& _id, unsigned int _flags )
  :   item( _id, _flags )
{
  ranges.resize(0);
  old_count = -1;
}


morphable_item::morphable_item( const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
  :   item( _id, _flavor, _flags )
{
  ranges.resize(0);
  old_count = -1;
}


morphable_item::~morphable_item()
{
  dump_ranges();
  old_count = -1;
}


///////////////////////////////////////////////////////////////////////////////
// File I/O
///////////////////////////////////////////////////////////////////////////////

morphable_item::morphable_item( const stringx& item_type_filename,
            const entity_id& _id,
            entity_flavor_t _flavor,
            bool _active,
            bool _stationary )
  :   item( item_type_filename, _id, _flavor, _active, _stationary, SKIP_DELETE_STREAM)
{
  assert(my_fs);

  ranges.resize(0);

  // it doesn't make sense for a morphable item not to be visible
  set_visible( true );

  stringx label;

  serial_in( *my_fs, &label );

  while(!(label== chunkend_label) && label.length())
  {
    if ( label == "item:" )
    {
      read_item_data( *my_fs, label );
    }
    else if ( label == "morph_ranges:" )
    {
      read_morph_ranges(*my_fs, label);
    }
    else
      error( my_fs->get_filename() + ": unknown keyword '" + label + "' in morphable_item section" );

    // get the next label
    serial_in( *my_fs, &label );
  }

  assert(my_fs);
  delete my_fs;
  my_fs = NULL;

  old_count = -1;
}


void morphable_item::read_morph_ranges( chunk_file& fs, stringx& label )
{
  assert(label == "morph_ranges:");

  serial_in( fs, &label );

  while(!(label== chunkend_label))
  {
    if(label == "range")
    {
      morphable_item_range *range = NEW morphable_item_range();

      serial_in( fs, &range->low );

      serial_in( fs, &label );
      if(label == "-")
      {
        serial_in( fs, &range->high );
        serial_in( fs, &range->vis_rep );
      }
      else
      {
        range->vis_rep = label;
      }

      range->vis_rep = fs.get_dir() + range->vis_rep;

      visual_rep* v = load_new_visual_rep( range->vis_rep, 0 );
      if( v->get_type()==VISREP_PMESH )
        static_cast<vr_pmesh*>(v)->shrink_memory_footprint();

      ranges.push_back(range);
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in morph_ranges section" );
    }

    // get the next label
    serial_in( fs, &label );
  }
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* morphable_item::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  morphable_item* newit = NEW morphable_item( _id, _flags );
  newit->copy_instance_data( *this );
  return (entity*)newit;
}


void morphable_item::copy_instance_data( const morphable_item& b )
{
  item::copy_instance_data( b );

  // it doesn't make sense for a morphable item not to be visible
  set_visible( true );

  dump_ranges();

  vector<morphable_item_range *>::const_iterator range = b.ranges.begin();
  vector<morphable_item_range *>::const_iterator range_end = b.ranges.end();

  for( ; range != range_end; ++range)
    ranges.push_back((*range)->make_instance());

  old_count = -1;
}

void morphable_item::frame_advance(time_value_t t)
{
  item::frame_advance(t);

  if(count != old_count)
    set_range_visrep(count);
}

void morphable_item::set_range_visrep(int cnt)
{
  vector<morphable_item_range *>::iterator range = ranges.begin();
  vector<morphable_item_range *>::iterator range_end = ranges.end();

  for( ; range != range_end; ++range)
  {
    if((*range)->in_range(count) && !(*range)->in_range(old_count))
    {
      change_visrep((*range)->vis_rep);
      set_flag(EFLAG_GRAPHICS, 1);
      break;
    }
  }

  old_count = count;
}

void morphable_item::dump_ranges()
{
  vector<morphable_item_range*>::iterator range_iter;

  for(range_iter = ranges.begin(); range_iter != ranges.end(); )
  {
    morphable_item_range *range = (*range_iter);

    if(range)
    {
      delete range;
      range_iter = ranges.erase( range_iter );
    }
    else
      ++range_iter;
  }
}
