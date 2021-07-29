#ifndef _MELEE_ITEM_H_
#define _MELEE_ITEM_H_

// BIGCULL #include "handheld_item.h"
#include "simple_classes.h"

class melee_item : public handheld_item
{
private:
  rational_t damage_mod;
  rational_t ext_range;

  virtual void init_defaults();

public:
  melee_item( const entity_id& _id, unsigned int _flags );

  melee_item( const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );

  virtual ~melee_item();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_melee_item() const { return true; }

// NEWENT File I/O
public:
  melee_item( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor = ENTITY_ITEM,
        unsigned int _flags = 0 );


  virtual bool handle_enx_chunk( chunk_file& fs, stringx& lstr );

// Old File I/O
public:
  melee_item( const stringx& item_type_filename,
        const entity_id& _id = ANONYMOUS,
        entity_flavor_t _flavor = ENTITY_ITEM,
        bool _active = INACTIVE,
        bool _stationary = false );

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  virtual void copy_instance_data( melee_item& b );

  void read_damage_chunk( chunk_file& fs );

public:
  inline rational_t get_damage_mod() const  { return(damage_mod); }
  inline rational_t get_ext_range() const   { return(ext_range); }
};

#endif