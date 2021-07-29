#ifndef MIC_H
#define MIC_H
////////////////////////////////////////////////////////////////////////////////
/*
  mic.h

  I got two turntables and a microphone.

  It's an entity so we can move it around using the same engine
  as everything else.  Which is very beautiful.
*/
////////////////////////////////////////////////////////////////////////////////
#include "algebra.h"
#include "po.h"

#include "entity.h"

// the basic microphone, which stays relative to its parent entity
class mic: public entity
  {
  public:
    mic( entity* _parent, 
          const entity_id& entity, 
          entity_flavor_t _flavor = ENTITY_MIC);

    // apply this microphone to the sound library
    void adjust_listener();
    void frame_advance(time_value_t t);
//!!    vector3d get_velocity();
  private:
    vector3d last_position;
  };

inline mic* find_mic( const entity_id& id )
  {
  return (mic*)entity_manager::inst()->find_entity( id, ENTITY_MIC );
  }

#endif