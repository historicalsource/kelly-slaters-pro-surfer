#include "global.h"

#include "damage_interface.h"
#include "entity.h"
//#include "brain.h"
#include "wds.h"
#include "collide.h"
#include "terrain.h"
//#include "dread_net.h"
// BIGCULL #include "handheld_item.h"
#include "app.h"
#include "game.h"
#include "msgboard.h"
#include "hwaudio.h"

#if defined(TARGET_XBOX)
#include "handheld_item.h"
#endif /* TARGET_XBOX JIV DEBUG */

bool god_mode_cheat = false;

damage_interface::damage_interface(entity *ent)
  : entity_interface(ent),
  hit_points("", 1000, 0, 1000),
  armor_points("", 0, 0, 1000)
{
  flags = 0;
  destroy_lifetime = 1.0f;
  destroy_sound = empty_string;
  destroy_fx = empty_string;
  destroy_script = empty_string;

  damage_mod = 1.0f;

//  dread_net_cue = dread_net::UNDEFINED_AV_CUE;
}

damage_interface::~damage_interface()
{
}

void damage_interface::copy(damage_interface *b)
{
  set_max_hit_points(b->get_max_hit_points());
  set_max_armor_points(b->get_max_armor_points());

  set_hit_points(b->get_hit_points());
  set_armor_points(b->get_armor_points());

  flags = b->flags;
  destroy_lifetime = b->destroy_lifetime;
  destroy_sound = b->destroy_sound;
  destroy_fx = b->destroy_fx;
  destroy_script = b->destroy_script;

//  dread_net_cue = b->dread_net_cue;
}

int damage_interface::apply_damage(entity *attacker, int damage, eDamageType type, const vector3d &pos, const vector3d &dir, int flags, const stringx &wounded_anim)
{
  int       new_hp  = get_hit_points();
  int       new_ap  = get_armor_points();

  // cases in which we do nothing:
  if ( new_hp <= 0              // have no hit points
    || damage == 0              // receiving no damage
    || (my_entity->is_hero() && god_mode_cheat)  // hero in god mode
    || my_entity->is_invulnerable()
    )
  {
    return 0;
  }

  // set up damage info (before raising signal)
  if ( attacker )
  {
    if ( attacker->is_a_handheld_item() )
    {
      dmg_info.attacker = ((handheld_item *)attacker)->get_owner();
      dmg_info.attacker_itm = (item *)attacker;
    }
    else
    {
      dmg_info.attacker = attacker;
      dmg_info.attacker_itm = NULL;
    }
  }
  else
  {
    dmg_info.attacker = NULL;
    dmg_info.attacker_itm = NULL;
  }

  dmg_info.damage = damage;
  dmg_info.dir = dir;
  dmg_info.loc = pos;
  dmg_info.type = type;
  dmg_info.push_wounded = true;
  dmg_info.push_death = true;
  dmg_info.flags = flags;
  dmg_info.wounded_anim = wounded_anim;

  // raise signal indicating that I have been damaged
  my_entity->raise_signal( entity::DAMAGED );

  // Damage calculations MUST come AFTER the signal, because the signal can change the damage in a callback (see bosses)
  dmg_info.damage = (int)(((rational_t)dmg_info.damage)*damage_mod);

  if ( dmg_info.damage < 0 )
    dmg_info.damage = 0;

/*
  // raise appropriate dread_net cue(s)
  switch ( dmg_info.type )
  {
  case DAMAGE_MELEE:
  case DAMAGE_NONBLOCK_MELEE:
  case DAMAGE_KNOCKING_DOWN:
    g_world_ptr->get_dread_net()->add_cue(dread_net::get_cue_type("MELEE_HIT"), dmg_info.attacker );
    break;
  default:
    break;
  }
*/

  if(dmg_info.damage > 0)
  {
    #ifdef TARGET_PC
    char    outbuf[100];
    sprintf( outbuf, "%s takes %d damage", my_entity->get_name().c_str(), dmg_info.damage );
    app::inst()->get_game()->get_message_board()->post( stringx(outbuf), 2.0F );
    #endif
  
    // apply damage
    new_ap -= dmg_info.damage;
    if ( new_ap < 0 )
    {
      new_hp += new_ap;
      new_ap = 0;
    }
    if ( new_hp < 0 )
      new_hp = 0;

    set_hit_points( new_hp );
    set_armor_points( new_ap );

    new_hp = get_hit_points();
    new_ap = get_armor_points();

    // play sound, etc.
    if ( new_hp > 0 )
    {
      if(dmg_info.damage > 0)
      {
        if(dmg_info.push_wounded)
        {
  //        if ( !my_entity->is_hero() && my_entity->get_brain() != NULL )
  //          my_entity->get_brain()->push_state_wounded( dmg_info.damage );
        }
      }
    }
    else
    {
      if ( has_destroy_script() )
      {
        // disable brain for scripted death sequence
  //      if(my_entity->get_brain())
  //        my_entity->get_brain()->set_active( false );
      }
      else if ( dmg_info.push_death )
      {
  //      if ( !my_entity->is_hero() && my_entity->get_brain() != NULL )
  //        my_entity->get_brain()->push_state_dying( dmg_info.damage );
      }

      my_entity->raise_signal( entity::DESTROYED );

      apply_destruction_fx();
    }

    set_flag(_DAMAGED_THIS_FRAME, true);
  }

  return(dmg_info.damage);
}

void damage_interface::apply_destruction_fx()
{
//  if(dread_net_cue != dread_net::UNDEFINED_AV_CUE)
//    g_world_ptr->get_dread_net()->add_cue((dread_net::eAVCueType)dread_net_cue, my_entity);

  if ( has_destroy_fx() )
  {
    po the_po = po_identity_matrix;
    the_po.set_position(my_entity->get_abs_position());
    g_world_ptr->add_time_limited_effect( destroy_fx.c_str(), the_po, destroy_lifetime );
  }

  if ( has_destroy_script() )
  {
    my_entity->spawn_entity_script_function( get_destroy_script() );
  }

  if ( !remain_visible() )
  {
    my_entity->set_visible( false );
  
    if( !remain_collision() )
    {
      my_entity->set_collisions_active( false );
      my_entity->set_walkable( false );
      my_entity->set_repulsion( false );
    }
  }

  if( no_collision() )
  {
    my_entity->set_collisions_active( false );
    my_entity->set_walkable( false );
    my_entity->set_repulsion( false );
  }

  if ( !remain_active() )
  {
    my_entity->set_active( false );
    my_entity->set_actionable( false );
  }

  if ( has_destroy_sound() )
  {
 /*   if ( my_entity->get_emitter() )
      my_entity->get_emitter()->play_sound( destroy_sound );*/
    if(1)
      ;// JIV DEBUG
    else
    {
      assert(0);
//      sound_device::inst()->play_sound( destroy_sound );
    }
  }
}


void damage_interface::read_enx_data( chunk_file& fs, stringx& lstr )
{
  serial_in( fs, &lstr );

  while(!(lstr == chunkend_label))
  {
    if(lstr == "hit_points")
    {
      rational_t val, max;
      serial_in( fs, &val );
      serial_in( fs, &max );

      hit_points.set_max(max);
      hit_points.set(val);
    }
    else if(lstr == "armor_points")
    {
      rational_t val, max;
      serial_in( fs, &val );
      serial_in( fs, &max );

      armor_points.set_max(max);
      armor_points.set(val);
    }
    else if(lstr == "effect")
    {
      set_flag(_DESTROY_FX, true);
      serial_in( fs, &destroy_fx );
    }
    else if(lstr == "lifetime")
    {
      serial_in( fs, &destroy_lifetime );
    }
    else if(lstr == "script")
    {
      set_flag(_DESTROY_SCRIPT, true);
      serial_in( fs, &destroy_script );
    }
    else if(lstr == "remain_active")
    {
      set_flag(_REMAIN_ACTIVE, true);
    }
    else if(lstr == "remain_visible")
    {
      set_flag(_REMAIN_VISIBLE, true);
    }
    else if(lstr == "no_collision")
    {
      set_flag(_NO_COLLISION, true);
    }
    else if(lstr == "remain_collision")
    {
      set_flag(_REMAIN_COLLISION, true);
    }
    else if(lstr == "single_blow")
    {
      set_flag(_SINGLE_BLOW, true);
    }
/*
    else if(lstr == "dread_net_cue")
    {
      stringx cue;
      serial_in(fs, &cue);
      cue.to_upper();
      dread_net_cue = dread_net::get_cue_type(cue);
    }
*/
    else if(lstr == "destroy_sound")
    {
      set_flag(_DESTROY_SOUND, true);
      serial_in( fs, &destroy_sound );

//      sound_device::inst()->load_sound( destroy_sound );
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + lstr + "' in damage_interface section" );
    }

    serial_in( fs, &lstr );
  }

  if ( is_flagged(_DESTROY_FX) )
    g_world_ptr->create_preloaded_entity_or_subclass( destroy_fx.c_str(),  empty_string );
}


void damage_interface::frame_advance(time_value_t t)
{
  set_flag(_DAMAGED_LAST_FRAME, is_flagged(_DAMAGED_THIS_FRAME));
  set_flag(_DAMAGED_THIS_FRAME, false);

  if(is_flagged(_DAMAGED_LAST_FRAME))
    last_dmg_info = dmg_info;
}

bool damage_interface::get_ifc_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("HIT_POINTS", hit_points);
  IFC_INTERNAL_GET_MACRO("ARMOR_POINTS", armor_points);
  IFC_INTERNAL_GET_MACRO("MAX_HIT_POINTS", get_max_hit_points());
  IFC_INTERNAL_GET_MACRO("MAX_ARMOR_POINTS", get_max_armor_points());

  return(false);
}

bool damage_interface::set_ifc_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("HIT_POINTS", hit_points);
  IFC_INTERNAL_SET_MACRO("ARMOR_POINTS", armor_points);
  IFC_INTERNAL_FUNC_MACRO("MAX_HIT_POINTS", set_max_hit_points(val));
  IFC_INTERNAL_FUNC_MACRO("MAX_ARMOR_POINTS", set_max_armor_points(val));

  return(false);
}
