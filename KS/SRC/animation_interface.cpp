#include "global.h"

#include "animation_interface.h"
#include "entity_interface.h"
#include "entity.h"
#include "entity_anim.h"
#include "file_finder.h"
#include "osdevopts.h"
#include "random.h"

animation_interface::animation_interface(entity *ent)
  : entity_interface(ent)
{
}

animation_interface::~animation_interface()
{
}


void animation_interface::copy(animation_interface *b)
{
  flags = b->flags;

  //copy aimation info
  anim_info_id_map.clear();
  anim_info_id_map_const_iterator bidmit = b->get_anim_info_id_map().begin();
  while( bidmit != b->get_anim_info_id_map().end() )
  {
    stringx id = (*bidmit).first;

    anim_info_id_map_anims_const_iterator bidmait = (*bidmit).second.begin();
    while( bidmait != (*bidmit).second.end() )
    {
      add_anim_info_id_map_entry( id, (*bidmait).percent, (*bidmait).anim, (*bidmait).damage, (*bidmait).recover_time, (*bidmait).recover_var, (*bidmait).flags );
      ++bidmait;
    }

    ++bidmit;
  }
}










void animation_interface::add_anim_info_id_map_entry( const stringx& id, rational_t _per, const stringx &_anim, int _dam, rational_t _rec, rational_t _rec_var, int _flags )
{
  anim_info_id_map[id].push_back( anim_info(_per, _anim, _dam, _rec, _rec_var, _flags) );
}

void animation_interface::read_animations_chunk(chunk_file &fs, const stringx &anims_directory)
{
  stringx in;
  for( serial_in(fs, &in); in != chunkend_label; serial_in(fs, &in) )
  {
    if(in == "anim")
    {
      stringx anim_id;
      serial_in(fs, &anim_id);

      for( serial_in(fs, &in); in != chunkend_label; serial_in(fs, &in) )
      {
        if(in == "anim_set")
          read_anim_sub_chunk(fs, anim_id, anims_directory);
        else
          error("Bad chunk ID '%s' in 'anim' section of %s!", in.c_str(), fs.get_filename().c_str());
      }
    }
    else
      error("Bad chunk ID '%s' in 'animations:' section of %s!", in.c_str(), fs.get_filename().c_str());
  }
}

void animation_interface::read_anim_sub_chunk(chunk_file &fs, const stringx &anim_id, const stringx &anims_directory)
{
  stringx in;
  rational_t percent = 0.0f;
  stringx inanm = empty_string;
  int damage = 0;
  rational_t recover = -1.0f;
  rational_t recover_var = -1.0f;
  int flags = 0;

  serial_in(fs, &percent);

  for( serial_in(fs, &in); in != chunkend_label; serial_in(fs, &in) )
  {
    if(in == "damage")
    {
      serial_in(fs, &damage);
    }
    else if(in == "recover_time")
    {
      serial_in(fs, &recover);
    }
    else if(in == "recover_var")
    {
      serial_in(fs, &recover_var);
    }
    else if(in == "reverse")
    {
      flags |= ANIM_IFC_REVERSE;
    }
    else if(in == "anim_file")
    {
      serial_in(fs, &inanm);
    }
    else
      error("Bad chunk ID '%s' in 'anim_set:' section (%s) of %s!", in.c_str(), anim_id.c_str(), fs.get_filename().c_str());
  }

  if(inanm.length() > 0)
  {
    if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
    {
      inanm = inanm + entity_track_tree::extension( );
      g_world_ptr->get_ett_manager()->load( inanm );
    }
    else
    {
      inanm = anims_directory + inanm + entity_track_tree::extension( );
      my_entity->load_anim(inanm);
    }
  }

  add_anim_info_id_map_entry( anim_id, percent, inanm, damage, recover, recover_var, flags );
}

void animation_interface::read_animations(chunk_file &f, const stringx& anims_directory)
{
  anim_info_id_map.clear();

  stringx in;
  for( serial_in(f, &in); in != chunkend_label && in.length() > 0; serial_in(f, &in) )
  {
    if(in == "animations:")
    {
      read_animations_chunk(f, anims_directory);
    }
    else
      error("Bad chunk ID '%s' in %s!", in.c_str(), f.get_filename().c_str());
  }
}

void animation_interface::read_anim_info_file( const stringx& anims_filename )
{
  int           i;
	chunk_file    f;
  stringx       filename;
  char          tc[256];

  if ( !file_finder_exists(anims_filename, stringx(".ani"), &filename) ) 
  {
    error("File '%s' not found!", anims_filename.c_str());
    return;
  }
  i = anims_filename.size();
  while( --i )
  {
    if( anims_filename[ i ] == '\\' )
      break;
  }
  strncpy( tc, anims_filename.c_str(), ++i );
  tc[i] = 0;
  stringx anims_directory( tc );

  f.open( filename, os_file::FILE_READ | chunk_file::FILE_TEXT );

  read_animations(f, anims_directory);

  f.close();
}


const stringx& animation_interface::extract_random_anim_info_id_map_anim( const stringx& id, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags )
{
  rational_t per_total = 0;

  anim_info_id_map_anims_iterator bidmait = anim_info_id_map[id].begin();
  while( bidmait != anim_info_id_map[id].end() )
  {
    per_total += (*bidmait).percent;
    ++bidmait;
  }

  rational_t r1 = random();

  rational_t roll = per_total * r1;

  per_total = 0;
  bidmait = anim_info_id_map[id].begin();
  while( bidmait != anim_info_id_map[ id ].end() )
  {
    per_total += (*bidmait).percent;
    if( per_total >= roll )
    {
      if( damage_value != NULL )  *damage_value  = (*bidmait).damage;
      if( recover != NULL )       *recover       = (*bidmait).recover_time;
      if( recover_var != NULL )   *recover_var   = (*bidmait).recover_var;
      if( flags != NULL )         *flags         = (*bidmait).flags;

      if( (*bidmait).anim.length() > 0 ) return( (*bidmait).anim );
    }
    ++bidmait;
  }

  //warning( "extract_random_anim_info_id_map_anim(" + id + "): could not find a mapped anim corresponding to ROLL=" + ftos(roll) );
  return( empty_string );
}

//---------------------------------------------------------------
const stringx& animation_interface::extract_given_anim_info_id_map_anim( const stringx& id, int idx, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags )
{
  anim_info_id_map_iterator bidmit = anim_info_id_map.begin();
  while( bidmit != anim_info_id_map.end() )
  {
    if( (*bidmit).first == id )
    {
      if( idx < (int)(*bidmit).second.size() )
      {
        if( damage_value != NULL )  *damage_value  = anim_info_id_map[id][idx].damage;
        if( recover != NULL )       *recover       = anim_info_id_map[id][idx].recover_time;
        if( recover_var != NULL )   *recover_var   = anim_info_id_map[id][idx].recover_var;
        if( flags != NULL )         *flags         = anim_info_id_map[id][idx].flags;

        if( anim_info_id_map[id][idx].anim.length() > 0 ) return( anim_info_id_map[id][idx].anim );
      }
      else
      {
        warning( "extract_given_anim_info_id_map_anim(" + id + "," + itos(idx) + "): given index is out of range for given state" );
      }
    }
    ++bidmit;
  }
  return( empty_string );
}

//---------------------------------------------------------------
// this has been fixed to get the greatest number that is below the damage (or the first one)
const stringx& animation_interface::extract_given_anim_info_id_map_anim_by_number( const stringx& id, int idx, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags )
{
  anim_info_id_map_anims_iterator bidmait = anim_info_id_map[ id ].begin();
  anim_info_id_map_anims_iterator best = bidmait;

  while( bidmait != anim_info_id_map[ id ].end() )
  {
    if( (*bidmait).percent <= idx && (*bidmait).percent >= (*best).percent)
      best = bidmait;

    ++bidmait;
  }

  if( best != anim_info_id_map[ id ].end())
  {
    if( damage_value != NULL ) *damage_value = (*best).damage;
    if( recover != NULL )      *recover      = (*best).recover_time;
    if( recover_var != NULL )  *recover_var  = (*best).recover_var;
    if( flags != NULL )        *flags        = (*best).flags;

    if( (*best).anim.length() > 0 ) return( (*best).anim );
  }

  return( empty_string );
}


animation_interface::anim_info::anim_info()
{
  percent = 0;
  anim = empty_string;
  damage = 0;
  recover_time = -1.0f;
  recover_var = -1.0f;
  flags = 0;
}


animation_interface::anim_info::anim_info( rational_t _per, const stringx &_anim, int _dam, rational_t _rec, rational_t _rec_var, int _flags )
{
  percent = _per;
  anim = _anim;
  damage = _dam;
  recover_time = _rec;
  recover_var = _rec_var;
  flags = _flags;
}
