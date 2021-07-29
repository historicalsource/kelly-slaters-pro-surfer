#if 0
////////////////////////////////////////////////////////////////////////////////
/*
  soundgrp.cpp

  The sound_group class.  Was originally a part of character.h, but wanted a
  global one, so here it is.

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "hwaudio.h"
#include "soundgrp.h"
#include "oserrmsg.h"

void sound_group_list_t::play_sound_group( const stringx &sref, int idx, int pri, const stringx *who)
  {
  if( (idx >= 0) && (idx < (int)size()) )
    {
    // IF TOLD TO PLAY A GROUP BY NUMBER DIRECTLY (IMPROBABLE)
    sound_device::inst()->play_sound( idx );
    }
  else if( who )
    {
    // PRIORITIZED WITHIN A HERD
    // A HERD BEING ANY NAMED GROUP, EVEN IF OF ONE MEMBER
    sound_group_list_t::iterator sgrp;
    for( sgrp = begin(); sgrp!=end(); sgrp++ )
      {
      if( (*sgrp).first == sref )
        {
        rational_t r = random();
        rational_t p = 0.0f;
        for( int i = 0; i < (int)(*sgrp).second->indices.size(); ++i )
          {
          p += (*sgrp).second->probabilities[ i ];
          if( r <= p )
            {
            // Play Here
            sound_device::inst()->play_sound( (*sgrp).second->sound_ids[ i ] );
            break;
            }
          }
        }
      }
    }
  else if( 1 ||pri )
    {
    // PRIORITIZED GLOBALLY
    sound_group_list_t::iterator sgrp;
    for( sgrp = begin(); sgrp!=end(); sgrp++ )
      {
      if( (*sgrp).first == sref )
        {
        rational_t r = random();
        rational_t p = 0.0f;
        for( unsigned i = 0; i < (*sgrp).second->indices.size(); ++i )
          {
          p += (*sgrp).second->probabilities[ i ];
          if( r <= p )
            {
            // Play Here
            sound_device::inst()->play_sound( (*sgrp).second->sound_ids[ i ] );
            break;
            }
          }
        }
      }
    }
  }

void sound_group_list_t::play_sound_group( const stringx &sref, sound_emitter* emit)
  {
	if(1)
    {
    // PRIORITIZED GLOBALLY
    sound_group_list_t::iterator sgrp;
    for( sgrp = begin(); sgrp!=end(); sgrp++ )
      {
      if( (*sgrp).first == sref )
        {
        rational_t r = random();
        rational_t p = 0.0f;
        for( unsigned i = 0; i < (*sgrp).second->indices.size(); ++i )
          {
          p += (*sgrp).second->probabilities[ i ];
          if( r <= p )
            {
            // Play Here
 			if(!emit)
              sound_device::inst()->play_sound( (*sgrp).second->sound_ids[ i ] );
			else
			  emit->play_sound( (sound_id_t) (*sgrp).second->sound_ids[ i ] );
            break;
            }
          }
        }
      }
    }
  }

//--------------------------------------------------------------
void sound_group_list_t::play_sound_group_looping( const stringx &sref )
{
  sound_group_list_t::iterator sgrp;
  for( sgrp = begin(); sgrp!=end(); sgrp++ )
  {
    if( (*sgrp).first == sref )
    {
      for( unsigned i = 0; i < (*sgrp).second->indices.size(); ++i )
      {
        //sound_device::inst()->play_sound_looping( (*sgrp).second->sound_ids[ i ] );
        assert(0);
      }
    }
  }
}

//--------------------------------------------------------------
void sound_group_list_t::kill_sound_group_looping( const stringx &sref )
{
  sound_group_list_t::iterator sgrp;
  for( sgrp = begin(); sgrp!=end(); sgrp++ )
  {
    if( (*sgrp).first == sref )
    {
      for( int i = 0; i < (int)(*sgrp).second->indices.size(); ++i )
      {
        sound_device::inst()->kill_sound( (*sgrp).second->sound_ids[ i ] );
      }
    }
  }
}

//--------------------------------------------------------------
bool sound_group_list_t::sound_group_exists( const stringx& sref )
  {
  sound_group_list_t::iterator sgrp;
  for( sgrp = begin(); sgrp!=end(); sgrp++ )
    {
    if( (*sgrp).first == sref )
      {
      return( true );
      }
    }
  return( false );
  }

//--------------------------------------------------------------
int sound_group_list_t::get_num_sound_group_sounds( const stringx& sref )
  {
  sound_group_list_t::iterator sgrp;
  for( sgrp = begin(); sgrp!=end(); sgrp++ )
    {
    if( (*sgrp).first == sref )
      {
      return( (*sgrp).second->indices.size() );
      }
    }
  return( 0 );
  }

void serial_in(chunk_file& io, sound_group_list_t * sg)
  {
  stringx in;
  serial_in( io, &in );
  while ( in != chunkend_label )
    {
    assert( in == "soundgrp" );
    serial_in( io, &in );
    stringx sound_group_name = in;
    sound_group * new_sg = NEW sound_group;
    rational_t sound_probability;
    int sound_index = 0;
    serial_in( io, &in );
    while ( in != chunkend_label )
      {
      if (in=="pitchvar")
        {
        serial_in (io, &(new_sg->pitch_variance));
        }
      else
        {
        stringx sound_file_name = in;
        serial_in( io, &sound_probability );
        new_sg->indices.push_back( sound_index );
        new_sg->probabilities.push_back( sound_probability );
        new_sg->sound_ids.push_back( sound_device::inst()->load_sound( sound_file_name ) );
        sound_index++;
        }

      serial_in( io, &in );
      }
    if ( (sg->insert( sound_group_list_t::value_type( sound_group_name, new_sg ) )).second==false )
        {
        stringx composite = "Duplicate sound_group: " + sound_group_name + " in " + io.get_name();
        error(composite.c_str());
        }
    serial_in( io, &in );
    }
  }


void sound_group_list_t::clear()
{
  const_iterator i = begin();
  const_iterator i_end = end();
  for ( ; i!=i_end; ++i )
    delete (*i).second;
  map<stringx , sound_group *>::clear();
}

#endif
