/*
This file is officially defunct! remove it at your pleasure... (JDB 7-27-00)
*/

#ifndef _TURRET_H_
#define _TURRET_H_

#include "conglom.h"


class turret : public conglomerate
{
// Data
private:
  int bio_damage;
  int mechanical_damage;

  void init_variables();

protected:
  void copy_instance_data( const turret& b );

  void read_damage_chunk( chunk_file& fs );

// Generic Constructors
public:
  turret( const entity_id& _id, unsigned int _flags );
  turret( const entity_id& _id = ANONYMOUS,
                entity_flavor_t _flavor = ENTITY_TURRET,
                unsigned int _flags = 0 );

  virtual ~turret();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_turret() const { return true; }

public:
  // File I/O
  turret( chunk_file& fs,
                const entity_id& _id,
                entity_flavor_t _flavor = ENTITY_TURRET,
                unsigned int _flags = 0 );
  
  // This function allows parsing instance data according to entity type.
  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );

  
  
  // Instancing
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  // Misc.
  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );
};


#endif