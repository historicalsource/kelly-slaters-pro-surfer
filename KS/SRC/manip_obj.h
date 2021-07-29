#ifndef _MANIP_OBJ_H_
#define _MANIP_OBJ_H_

#include "entity.h"

class manip_obj : public entity
{
protected:
  void init();

  virtual void copy_instance_data( const manip_obj& b );

public:
  manip_obj( const entity_id& _id, unsigned int _flags );

  manip_obj( chunk_file& fs,
          const entity_id& _id,
          entity_flavor_t _flavor = ENTITY_MANIP,
          unsigned int _flags = 0 );

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  virtual void frame_advance(time_value_t t);

  void smash(entity *target = NULL);
};

#endif