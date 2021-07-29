/*
This file is officially defunct! remove it at your pleasure... (JDB 7-27-00)
*/

// conglom.cpp
#include "global.h"

//!#include "character.h"
#include "conglom.h"
#include "wds.h"
#include "light.h"
#include "oserrmsg.h"
#include "particle.h"
//!#include "ladder.h"
#include "pmesh.h"
#include "osdevopts.h"
#include "renderflav.h"
#include "iri.h"
// BIGCULL #include "turret.h"

#if defined(TARGET_XBOX)
#include "turret.h"
#endif /* TARGET_XBOX JIV DEBUG */

void turret::init_variables()
{
  bio_damage = 0;
  mechanical_damage = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Generic Constructors
///////////////////////////////////////////////////////////////////////////////

turret::turret( const entity_id& _id, unsigned int _flags )
  :   conglomerate( _id, ENTITY_TURRET, _flags )
{
  init_variables();
}

turret::turret( const entity_id& _id,
                            entity_flavor_t _flavor,
                            unsigned int _flags )
  :   conglomerate( _id, _flavor, _flags )
{
  init_variables();
}

turret::~turret()
{
}


///////////////////////////////////////////////////////////////////////////////
// File I/O
///////////////////////////////////////////////////////////////////////////////

turret::turret( chunk_file& fs,
                            const entity_id& _id,
                            entity_flavor_t _flavor,
                            unsigned int _flags )
  :   conglomerate( _id, _flavor, _flags )
{
  init_variables();
}



// This function allows parsing instance data according to entity type.
// If it recognizes the given stringx as a chunk of instance
// data for this type, it will parse the data; otherwise it will hand
// the parsing up to the parent class.
bool turret::parse_instance( const stringx& pcf, chunk_file& fs )
{
/*
  if ( pcf == stringx("scanner") )
  {
    // look for a member that is a scanner
    vector<entity*>::iterator i = members.begin();
    vector<entity*>::iterator i_end = members.end();
    for ( ; i!=i_end; i++ )
    {
      if ( (*i)->get_flavor() == ENTITY_SCANNER )
        break;
    }
    if ( i == i_end )
      error( get_id().get_val() + ": parse_instance(): no scanner member found in conglomerate" );
    else
      (*i)->parse_instance( pcf, fs );
  }
  else
*/
    return conglomerate::parse_instance( pcf, fs );

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* turret::make_instance( const entity_id& _id,
                                     unsigned int _flags ) const
{
  turret* newcg = NEW turret( _id, _flags );

  newcg->copy_instance_data( *this );

  return (entity*)newcg;
}


void turret::copy_instance_data( const turret& b )
{
  conglomerate::copy_instance_data( b );

  bio_damage = b.bio_damage;
  mechanical_damage = b.mechanical_damage;
}


void turret::read_damage_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "normal_damage")
    {
      serial_in( fs, &bio_damage );
      mechanical_damage = bio_damage;
    }
    else if(label == "bio_damage")
    {
      serial_in( fs, &bio_damage );
    }
    else if(label == "mechanical_damage")
    {
      serial_in( fs, &mechanical_damage );
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in damage section" );
    }

    serial_in( fs, &label );
  }
}



bool turret::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  // optional entity flags
  if ( label == "turret_info:" )
  {
    serial_in( fs, &label );

    while(!(label == chunkend_label))
    {
      if(label == "damage:")
      {
        read_damage_chunk( fs );
      }
      else
      {
        error( fs.get_filename() + ": unknown keyword '" + label + "' in turret_info section" );
      }

      serial_in( fs, &label );
    }

    return(true);
  }
  else
    return(conglomerate::handle_enx_chunk( fs, label ));
}
