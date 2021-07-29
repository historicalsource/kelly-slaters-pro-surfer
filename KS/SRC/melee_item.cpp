#include "global.h"

// BIGCULL #include "melee_item.h"
#include "wds.h"
#include "osdevopts.h"
#include "entityflags.h"
#include "entity_maker.h"
#include "sound_interface.h"
#include "sound_group.h"
#include "script_access.h"


#if defined(TARGET_XBOX)
#include "handheld_item.h"
#include "melee_item.h"
#endif /* TARGET_XBOX JIV DEBUG */



void melee_item::init_defaults()
{
  handheld_item::init_defaults();

  damage_mod = 1.0f;
  ext_range = 0.0f;
}

melee_item::melee_item( const entity_id& _id, unsigned int _flags )
:   handheld_item( _id, ENTITY_ITEM, _flags )
{
  init_defaults();
}


melee_item::melee_item( const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
:   handheld_item( _id, _flavor, _flags )
{
  init_defaults();
}


melee_item::~melee_item()
{
}

///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

melee_item::melee_item( chunk_file& fs,
            const entity_id& _id,
            entity_flavor_t _flavor,
            unsigned int _flags )
:   handheld_item( fs, _id, _flavor, _flags )
{
  init_defaults();
}

void melee_item::read_damage_chunk( chunk_file& fs )
{
  stringx label;
  serial_in( fs, &label );

  while(!(label == chunkend_label))
  {
    if(label == "damage_mod")
    {
      serial_in( fs, &damage_mod );
    }
    else if(label == "ext_range")
    {
      serial_in( fs, &ext_range);
    }
    else
    {
      error( fs.get_filename() + ": unknown keyword '" + label + "' in damage section" );
    }

    serial_in( fs, &label );
  }
}

bool melee_item::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "melee_effects:")
  {
    serial_in( fs, &label );

    while(!(label == chunkend_label))
    {
      if(label == "damage:")
      {
        read_damage_chunk( fs );
      }
      else if(label == "orientation:")
      {
        read_orientation_chunk( fs );
      }
      else if(label == "ai_info:")
      {
        read_ai_info_chunk( fs );
      }
      else if(label == "preload")
      {
        serial_in( fs, &label );
        g_world_ptr->create_preloaded_entity_or_subclass( label.c_str(),  empty_string );
      }
      else
      {
        error( fs.get_filename() + ": unknown keyword '" + label + "' in gun_effects section" );
      }

      serial_in( fs, &label );
    }

    return(true);
  }
  else
    return(handheld_item::handle_enx_chunk(fs, label));

}


///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

melee_item::melee_item( const stringx& item_type_filename,
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

entity* melee_item::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  melee_item* newit = NEW melee_item( _id, _flags );
  newit->copy_instance_data( *((melee_item *)this) );
  return (entity*)newit;
}


void melee_item::copy_instance_data( melee_item& b )
{
  handheld_item::copy_instance_data( b );

  damage_mod = b.damage_mod;
  ext_range = b.ext_range;
}
