#ifndef ENTITY_MOVABLE_INTERFACE_CLASS_HEADER
#define ENTITY_MOVABLE_INTERFACE_CLASS_HEADER

#include "global.h"

#include "algebra.h"
#include "ostimer.h"
#include "po.h"
#include "entity_interface.h"

class entity;

class movable_interface : public entity_interface
{
  protected:
    entity *my_entity;

    bool frame_delta_valid;
    bool last_frame_delta_valid;
    po frame_delta;
    time_value_t frame_time;

  public:
    movable_interface(entity *new_entity)
    {
      my_entity = new_entity;
      invalidate_deltas();
    }

    bool is_frame_delta_valid() const { return frame_delta_valid; }
    bool is_last_frame_delta_valid() const { return last_frame_delta_valid; }
    const po & get_frame_delta() const { return frame_delta; }
    const time_value_t &get_frame_time() const { return frame_time; }
    void invalidate_deltas()
    {
      frame_delta_valid=false;
      last_frame_delta_valid=false;
    }

    vector3d get_frame_abs_delta_position( bool first_time = true, const vector3d& rel_delta_pos = ZEROVEC ) const;
    
    void set_frame_delta(po const & bob, time_value_t t);
    void invalidate_frame_delta();
};


#endif // ENTITY_MOVABLE_INTERFACE_CLASS_HEADER