#ifndef SKY_H
#define SKY_H


#include "entity.h"


class sky : public entity
{
public:
  sky( const entity_id& eid, entity_flavor_t _flavor=ENTITY_SKY );

  sky( chunk_file& fs, const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags );

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  void copy_instance_data( const sky& b ) { entity::copy_instance_data(b); }

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_sky() const { return true; }

/////////////////////////////////////////////////////////////////////////////
// render interface
public:
  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );
};


inline sky* find_sky( const entity_id& id )
{
  return (sky*)entity_manager::inst()->find_entity( id, ENTITY_SKY );
}


#endif  // SKY_H
