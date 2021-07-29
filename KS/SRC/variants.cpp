//////////////////////////////////////////////////////////////////////////////
/*
  variants.cpp

  Overflow stuff from actor.cpp due to Metrowerks choking on large files in DEBUG build.
  Home of the variant and variant_descriptor.
*/
///////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "variants.h"
#include "osdevopts.h"
#include "visrep.h"
#include "wds.h"
#include "mcs.h"
#include "profiler.h"
//P #include "memorycontext.h"
#include "project.h"
#include "triple.h"
//P #include "membudget.h"
//!#include "character.h"
#include "file_finder.h"
#include "entity_maker.h"


variant::variant( variant_descriptor *dsc )
{
  locked = false;
  my_actor = NULL;
  my_descriptor = dsc;
  // build entity instances for all attachments
  attachment_list::const_iterator i = dsc->get_attachments().begin();
  attachment_list::const_iterator i_end = dsc->get_attachments().end();
  for ( ; i!=i_end; ++i )
  {
    entity* base = (*i)->get_base_copy();
    entity* e = base->make_instance( entity_id::make_unique_id(), base->get_flags() );
    g_entity_maker->create_entity( e );
    e->set_flag(EFLAG_MISC_MEMBER_OF_VARIANT, true);
    e->set_visible( false );
    e->force_region( NULL );  // remove from world
    entities.push_back( e );
  }
}


void variant::set_visible( bool torf )
{
  vector<entity*>::const_iterator i = entities.begin();
  vector<entity*>::const_iterator i_end = entities.end();
  for ( ; i!=i_end; ++i )
  {
    entity* e = *i;
    e->set_visible( torf );
    /*  CTT 07/27/00: not sure why it was being done this way; maybe I'll find out later
    switch ( e->get_flavor() )
    {
    case ENTITY_ENTITY:
    case ENTITY_PHYSICAL:
    case ENTITY_CONGLOMERATE:
    case ENTITY_PARTICLE_GENERATOR:
    case ENTITY_BEAM:
      e->set_visible( torf );
      break;
    }
    */
  }
}


bool variant::acquire( actor* act )
{
  if ( locked )
    return false;
  // lock this variant
  set_locked( true );
  my_actor = act;
  // set material and color if requested
  if ( use_alternative_material() )
    my_actor->set_alternative_materials( get_my_material_name() );
  if ( use_alternative_color() )
    my_actor->set_render_color( get_my_color() );
  // attach attachments to actor
  vector<entity*>::const_iterator ei = entities.begin();
  vector<entity*>::const_iterator ei_end = entities.end();
  attachment_list::const_iterator ai = my_descriptor->get_attachments().begin();
  attachment_list::const_iterator ai_end = my_descriptor->get_attachments().end();
  for ( ; ei!=ei_end; ++ei,++ai )
  {
    entity* e = *ei;
    attachment_info* a = *ai;
    entity* parent = NULL;
    if ( my_actor->limb_valid(a->get_parent_id()) )
      parent = my_actor->limb_ptr( a->get_parent_id() )->get_body();
    else
      parent = my_actor->nonlimb_ptr( a->get_parent_id() );
    if ( parent == NULL )
    {
      error( my_actor->get_id().get_val() + ": failed to attach variant '" + my_descriptor->get_name() +
             "': no match for parent id " + anim_id_manager::inst()->get_label( a->get_parent_id() ) );
    }
    if ( a->is_flagged(attachment_info::USE_CHARACTER_MATERIAL) )
    {
      e->set_alternative_materials( get_my_material_name() );
    }
    if ( a->is_flagged(attachment_info::USE_CHARACTER_COLOR) )
    {
      e->set_render_color( get_my_color() );
    }
    else
    {
      e->set_render_color( color32_white );
    }
    e->unforce_regions();
    e->set_parent( parent );
    e->set_rel_po( a->get_rel_po() );
  }
  // make attachments visible
  set_visible( true );
  return true;
}


void variant::release()
{
  if ( locked )
  {
    set_visible( false );
    // detach all attachments
    vector<entity*>::const_iterator ei = entities.begin();
    vector<entity*>::const_iterator ei_end = entities.end();
    for ( ; ei!=ei_end; ++ei )
    {
      entity* e = *ei;
      e->set_parent( NULL );
      e->force_region( NULL );  // remove from world
    }
    // reset material and color if necessary
    if ( use_alternative_material() )
      my_actor->set_alternative_materials( NULL );
    if ( use_alternative_color() )
      my_actor->set_render_color( color32_white );
    // unlock this variant
    set_locked( false );
    my_actor = NULL;
  }
}


#define MAX_VARIANTS_NUM 5


variant_descriptor::variant_descriptor( const stringx& _name )
{
  name = _name;
  pool_size = 0;
  pool_capacity = MAX_VARIANTS_NUM;
  char_name = empty_string;

  AIStates[BRAIN_REACT_NONE]    = BRAIN_AI_STATE_SAFE_IDLE;
  AIStates[BRAIN_REACT_IDLE]    = BRAIN_AI_STATE_GUARD;
  AIStates[BRAIN_REACT_ALERTED] = BRAIN_AI_STATE_INVESTIGATE;
  AIStates[BRAIN_REACT_COMBAT]  = BRAIN_AI_STATE_NONE;

  default_reload_timer = 1.0f;
  reload_timer_variance = 0.5f;
  use_alt_material = false;
  use_alt_color = false;
  full_hit_points = -1;
}


variant_descriptor::~variant_descriptor()
{
  attachment_list::const_iterator i = attachments.begin();
  attachment_list::const_iterator i_end = attachments.end();
  for ( ; i!=i_end; ++i )
    delete *i;

  attachments.clear();
}


void variant_descriptor::set_character( const stringx& _name )
{
  char_name = _name;
}


void variant_descriptor::add_attachment( const stringx& ent_name, anim_id_t parent_id, const po& rel_po, const unsigned int flags )
{
  entity* base_copy = g_world_ptr->create_preloaded_entity_or_subclass( ent_name, "characters\\"+char_name+"\\" );
  attachments.push_back( NEW attachment_info(base_copy,parent_id,rel_po,flags) );
}


variant* variant_descriptor::acquire( actor* act )
{
  variant* rv = NULL;
  variant_list::const_iterator vi = pool.begin();

  if ( pool_size )
  {
    for ( ; vi!=pool.end(); ++vi )
    {
      variant* v = *vi;
      if ( !v->is_locked() )
      {
        rv = v;
        break;
      }
    }
  }
  if ( !rv && !full() )
  {
    rv = NEW variant( this );
    pool.push_back( rv );
    pool_size++;
  }

  if ( rv )
  {
    bool res = rv->acquire( act );
    assert( res );
  }
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// variant brain data



void variant_descriptor::add_brain_id_map_entry( const stringx& id, rational_t _per, const stringx _anim[_BRAIN_MAX_REACTION_STATES - 1], int _dam, rational_t _rec, rational_t _rec_var, int _flags )
{
  brain_id_map[id].push_back( brain_anim(_per, _anim, _dam, _rec, _rec_var, _flags) );
}

void variant_descriptor::read_animations_chunk(chunk_file &fs, const stringx &anims_directory)
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

void variant_descriptor::read_anim_sub_chunk(chunk_file &fs, const stringx &anim_id, const stringx &anims_directory)
{
  stringx in;
  rational_t percent = 0.0f;
  stringx inanm[_BRAIN_MAX_REACTION_STATES - 1];
  int damage = 0;
  rational_t recover = -1.0f;
  rational_t recover_var = -1.0f;
  int flags = 0;

  for( int j = 0; j < (_BRAIN_MAX_REACTION_STATES - 1); ++j )
    inanm[j] = empty_string;

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
    else if(in == "unblockable")
    {
      flags |= _BRAIN_ANIM_UNBLOCKABLE;
    }
    else if(in == "knockdown")
    {
      flags |= _BRAIN_ANIM_KNOCKDOWN;
    }
    else if(in == "stun")
    {
      flags |= _BRAIN_ANIM_STUN;
    }
    else
    {
      bool found = false;
      for(int i = 1; i<_BRAIN_MAX_REACTION_STATES && !found; ++i)
      {
        if(in == reaction_levels[i])
        {
          serial_in(fs, &inanm[i-1]);
          found = true;
          break;
        }
      }

      if(!found)
        error("Bad chunk ID '%s' in 'anim_set:' section (%s) of %s!", in.c_str(), anim_id.c_str(), fs.get_filename().c_str());
    }
  }

  for( int i = 0; i < (_BRAIN_MAX_REACTION_STATES - 1); ++i )
  {
    if(inanm[i].length() > 0)
    {
      inanm[i] = anims_directory + inanm[i] + entity_track_tree::extension( );
      if ( !entity_track_bank.find_instance(inanm[i]) )
      {
        chunk_file fs;
        fs.open( inanm[i] );
//P        int alloc0 = memtrack::get_total_alloced();
//P        g_memory_context.push_context( "ANIMS" );
        entity_track_tree* et = NEW entity_track_tree;
        serial_in( fs, et );
/*P        g_memory_context.pop_context();
        membudget_t::inst()->use( membudget_t::ANIMS, memtrack::get_total_alloced()-alloc0 );
P*/
        entity_track_bank.insert_new_object( et, inanm[i] );

      }
    }
  }

  add_brain_id_map_entry( anim_id, percent, inanm, damage, recover, recover_var, flags );
}

void variant_descriptor::read_preloads_chunk(chunk_file &fs, const stringx &anims_directory)
{
  stringx in;
  for( serial_in(fs, &in); in != chunkend_label; serial_in(fs, &in) )
  {
    stringx load = anims_directory + in;
    g_world_ptr->create_preloaded_entity_or_subclass( load.c_str(),  empty_string );
  }
}

void variant_descriptor::read_brain_anim_stuff( const stringx& anims_filename )
{
  int           i;
	chunk_file    f;
  stringx       in, filename;
  char          tc[256];

  if ( !file_finder_exists(anims_filename, stringx(".brn"), &filename) ) return;

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

  for( serial_in(f, &in); in != chunkend_label && in.length() > 0; serial_in(f, &in) )
  {
    if(in == "animations:")
    {
      read_animations_chunk(f, anims_directory);
    }
    else if( in == "preloads:")
    {
      read_preloads_chunk( f, anims_directory);
    }
    else
      error("Bad chunk ID '%s' in %s!", in.c_str(), filename.c_str());
  }

  f.close();
}


const stringx& variant_descriptor::extract_random_brain_id_map_anim( stringx& id, eReactionState reaction_level, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags )
{
  rational_t per_total = 0;

  brain_id_map_anims_iterator bidmait = brain_id_map[id].begin();
  while( bidmait != brain_id_map[id].end() )
  {
    per_total += (*bidmait).percent;
    ++bidmait;
  }

  rational_t r1 = random();

  rational_t roll = per_total * r1;

  per_total = 0;
  bidmait = brain_id_map[id].begin();
  while( bidmait != brain_id_map[ id ].end() )
  {
    per_total += (*bidmait).percent;
    if( per_total >= roll )
    {
      if( damage_value != NULL )  *damage_value  = (*bidmait).damage;
      if( recover != NULL )       *recover       = (*bidmait).recover_time;
      if( recover_var != NULL )   *recover_var   = (*bidmait).recover_var;
      if( flags != NULL )         *flags         = (*bidmait).flags;

      int i;
      for( i = reaction_level - 1; i >= 0; i-- )
        if( (*bidmait).anims[i].length() > 0 ) return( (*bidmait).anims[i] );
//      if( i < 0 ) error( no_match_errstr );
    }
    ++bidmait;
  }

  //warning( "extract_random_brain_id_map_anim(" + id + "): could not find a mapped anim corresponding to ROLL=" + ftos(roll) );
  return( no_match_errstr );
}

//---------------------------------------------------------------
const stringx& variant_descriptor::extract_given_brain_id_map_anim( stringx& id, int idx, eReactionState reaction_level, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags )
{
  brain_id_map_iterator bidmit = brain_id_map.begin();
  while( bidmit != brain_id_map.end() )
  {
    if( (*bidmit).first == id )
    {
      if( idx < (*bidmit).second.size() )
      {
        if( damage_value != NULL )  *damage_value  = brain_id_map[id][idx].damage;
        if( recover != NULL )       *recover       = brain_id_map[id][idx].recover_time;
        if( recover_var != NULL )   *recover_var   = brain_id_map[id][idx].recover_var;
        if( flags != NULL )         *flags         = brain_id_map[id][idx].flags;

        int i;
        for( i = reaction_level - 1; i >= 0; i-- )
          if( brain_id_map[id][idx].anims[i].length() > 0 ) return( brain_id_map[id][idx].anims[i] );
//        if( i < 0 ) error( no_match_errstr );
      }
      else
      {
        warning( "extract_given_brain_id_map_anim(" + id + "," + itos(idx) + "): given index is out of range for given state" );
      }
    }
    ++bidmit;
  }
  return( no_match_errstr );
}

//---------------------------------------------------------------
// this has been fixed to get the greatest number that is below the damage (or the first one)
const stringx& variant_descriptor::extract_given_brain_id_map_anim_by_number( stringx& id, int idx, eReactionState reaction_level, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags )
{
  int i;
  brain_id_map_anims_iterator bidmait = brain_id_map[ id ].begin();
  brain_id_map_anims_iterator best = bidmait;

  while( bidmait != brain_id_map[ id ].end() )
  {
    if( (*bidmait).percent <= idx && (*bidmait).percent >= (*best).percent)
      best = bidmait;

    ++bidmait;
  }

  if( best != brain_id_map[ id ].end())
  {
    if( damage_value != NULL ) *damage_value = (*best).damage;
    if( recover != NULL )      *recover      = (*best).recover_time;
    if( recover_var != NULL )  *recover_var  = (*best).recover_var;
    if( flags != NULL )        *flags        = (*best).flags;

    for( i = reaction_level - 1; i >= 0; i-- )
      if( (*best).anims[i].length() > 0 ) return( (*best).anims[i] );
  }

/*
  warning( "brain does not contain an animation mapping for ID=" + id + " NUM=" + itos(idx) );
  if ( brain_id_map[id].size() )
  {
    if( damage_value != NULL ) *damage_value = (*bidmait).damage;
    for( i = reaction_level - 1; i >= 0; i-- )
      if( brain_id_map[id].back().anims[i].length() > 0 ) return brain_id_map[id].back().second[i];
//    if( i < 0 ) error( no_match_errstr );
  }
*/
  return( no_match_errstr );
}


variant_descriptor::brain_anim::brain_anim()
{
  percent = 0;
  for( int i = 0; i < _BRAIN_MAX_REACTION_STATES - 1; i++ ) anims[i] = empty_string;
  damage = 0;
  recover_time = -1.0f;
  recover_var = -1.0f;
  flags = 0;
}


variant_descriptor::brain_anim::brain_anim( rational_t _per, const stringx _anim[_BRAIN_MAX_REACTION_STATES - 1], int _dam, rational_t _rec, rational_t _rec_var, int _flags )
{
  percent = _per;
  for( int i = 0; i < _BRAIN_MAX_REACTION_STATES - 1; i++ ) anims[i] = _anim[i];
  damage = _dam;
  recover_time = _rec;
  recover_var = _rec_var;
  flags = _flags;
}
