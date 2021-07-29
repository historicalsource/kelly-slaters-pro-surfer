/*

character.cpp
Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

Root class of game characters (Heroes and NPCS).  Characters are distinguished from
other entities by having hit_points, inventories, brains (or manual controllers) etc.
*/
#include "global.h"

//!#include "character.h"

#include "app.h"
//!#include "attrib.h"
#include "hwaudio.h"
#include "brain.h"
//#include "fxman.h"
#include "game.h"
#include "mcs.h"
#include "fcs.h"
#include "controller.h"
#include "msgboard.h"
#include "hwmath.h"
#include "wds.h"
//!#include "charhead.h"
#include "terrain.h"
#include "capsule.h"
#include "vm_thread.h"
#include "hinge.h"
#include "filespec.h"
#include "actuator.h"
#include "billboard.h"
#include "osdevopts.h"
#include "ladder.h"
#include "debug_tools.h"
#include "particle.h"
#include "soundgrp.h"
//P #include "memorycontext.h"
#include "entityflags.h"
#include "shock_mods.h"
#include "profiler.h"
#include "geomgr.h"
#include "collide.h"
#if defined(TARGET_MKS)
#include "sg_syhw.h"
#include "sg_gd.h"
#include "sg_pdvib.h"
#endif

// BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
//!#include "char_group.h"

stringx char3_errstr("no_match");

/*
//---------------------------------------------------------------
void character::clear_brain_id_maps()
{
  brain_id_map_iterator bidmit = brain_id_map.begin();
  while( bidmit != brain_id_map.end() )
  {
    brain_id_map_anims_iterator bidmait = (*bidmit).second.begin();
    while( bidmait != (*bidmit).second.end() )
    {
      ++bidmait;
    }
    // TELL WORLD TO DO SOMETHING TO KILL (*bidmait) ??
    ++bidmit;
  }
}

//---------------------------------------------------------------
void character::add_brain_id_map_entry( stringx& id, float percent, stringx& animtree )
{
  brain_id_map[ id ].push_back( pair<float,stringx>(percent,animtree) );
}

//---------------------------------------------------------------
void character::read_brain_anim_stuff( const stringx& anims_filename )
{
  int           i;
	chunk_file    f;
  stringx       in, filename, inanm;
  char tc[ 256 ];

  if ( !g_world_ptr->exists(anims_filename, stringx(".brn"), &filename) ) return;

  i = anims_filename.size();
  while( --i )
  {
    if( anims_filename[ i ] == '\\' )
      break;
  }
  strncpy( tc, anims_filename.c_str(), ++i );
  tc[ i ] = 0;
  stringx anims_directory( tc );

  f.open( filename, os_file::FILE_READ | chunk_file::FILE_TEXT );
  for( serial_in(f, &in); in != chunkend_label; serial_in(f, &in) )
  {
    // DO SOMETHING WITH in HERE
    stringx section_name = in;
    // PARSE MORE
    serial_in( f, &in );
    float percent = atof( in.c_str() );
    // PARSE MORE
    serial_in( f, &in );
    inanm = anims_directory + in + ".anm";

    load_anim( inanm );

    add_brain_id_map_entry( section_name, percent, inanm );
  }
  f.close();
}

//---------------------------------------------------------------
stringx& character::extract_random_brain_id_map_anim( stringx& id )
{
  float per_total = 0;

  brain_id_map_anims_iterator bidmait = brain_id_map[ id ].begin();
  while( bidmait != brain_id_map[ id ].end() )
  {
    per_total += (*bidmait).first;
    ++bidmait;
  }

  float r1 = random();

  float roll = per_total * r1;

  per_total = 0;
  bidmait = brain_id_map[ id ].begin();
  while( bidmait != brain_id_map[ id ].end() )
  {
    per_total += (*bidmait).first;
    if( per_total >= roll )
    {
      return( (*bidmait).second );
    }
    ++bidmait;
  }

  //warning( "extract_random_brain_id_map_anim(" + id + "): could not find a mapped anim corresponding to ROLL=" + ftos(roll) );
  return( char3_errstr );
}

//---------------------------------------------------------------
stringx& character::extract_given_brain_id_map_anim( stringx& id, int idx )
{
  brain_id_map_iterator bidmit = brain_id_map.begin();
  while( bidmit != brain_id_map.end() )
  {
    if( (*bidmit).first == id )
    {
      if( idx < (*bidmit).second.size() )
      {
        return( brain_id_map[ id ][ idx ].second );
      }
      else
      {
        warning( "extract_given_brain_id_map_anim(" + id + "," + itos(idx) + "): given index is out of range for given state" );
      }
    }
    ++bidmit;
  }
  return( char3_errstr );
}
//---------------------------------------------------------------
// CTT 05/10/00: PROBABLY TEMPORARY:
// This function was created (by Patrick) strictly to support the WOUNDED state,
// and is not really conformant to the design of brain_id_map; we will probably
// invent something better after E3.
stringx& character::extract_given_brain_id_map_anim_by_number( stringx& id, int idx )
{
  brain_id_map_anims_iterator bidmait = brain_id_map[ id ].begin();
  while( bidmait != brain_id_map[ id ].end() )
  {
    if( idx <= (*bidmait).first )
    {
      return( (*bidmait).second );
    }
    ++bidmait;
  }
  warning( "brain does not contain an animation mapping for ID=" + id + " NUM=" + itos(idx) );
  if ( brain_id_map[id].size() )
    return brain_id_map[id].back().second;
  else
    return( char3_errstr );
}
*/
//---------------------------------------------------------------
entity *character::determine_nearest_target()
{
  collision_capsule *mycap = get_updated_damage_capsule();
  entity *closest = 0;
  float clodist = 10000.0f;

  int i;

  {
    i = app::inst()->get_game()->get_world()->get_num_active_characters();
    while( i-- )
    {
      character *pc = app::inst()->get_game()->get_world()->get_active_character( i );
      if( pc != this )
      {
        if( pc && pc->are_collisions_active() && pc->has_entity_collision() )
        {
          vector3d mypos = get_abs_position();
          vector3d pcpos = pc->get_abs_position();
          vector3d vec = mypos - pcpos;
          vector3d invvec = pcpos - mypos;
          rational_t len = vec.length();

          if( (len*1.2 < (get_radius()+pc->get_radius())) && (len < clodist) )
          {
            rational_t ab = angle_between( invvec, get_abs_po().get_facing() );
            if( ab <= 1.0 )
            {
                clodist = len;
                closest = pc;
            }
          }
        }
      }
    }

    if( ! closest )
    {
      world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
      world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
      entity* pc;
      for ( ; ei!=ei_end; ei++ )
      {
        pc = *ei;
        if( pc && pc != this && pc->is_destroyable() )
        {
          vector3d mypos = get_abs_position();
          vector3d pcpos = pc->get_abs_position();
          vector3d vec = mypos - pcpos;
          vector3d invvec = pcpos - mypos;
          rational_t len = vec.length();
          
          if( len*1.2 < (get_radius()+pc->get_radius()) && (len < clodist) )
          {
            rational_t ab = angle_between( invvec, get_abs_po().get_facing() );
            if( ab <= 1.0 )
            {
                clodist = len;
                closest = pc;
            }
          }
        }
      }
    }
  }
  return( closest );
}

bool character::is_in_melee_range( character *chr_pt )
{
  
  vector3d diff = chr_pt->get_abs_position() - get_abs_position();
  rational_t dist2 = diff.length2();
  rational_t rad = chr_pt->get_attack_range();
  return( (dist2 <= rad*rad) ? true : false );
}


void character::set_physical( bool p )
{
  if ( is_physical() != p )
  {
    entity::set_physical( p );
    // invalidate physical data as necessary
    last_floor_offset = 0.0f;
    last_floor_entity = NULL;
  }
}
