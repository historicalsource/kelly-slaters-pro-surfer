#include "movable_interface.h"
#include "entity.h"

// Rel pos is guarenteed to be ZEROVEC when first_time = true;
vector3d movable_interface::get_frame_abs_delta_position( bool first_time, const vector3d& rel_delta_pos ) const
{
  vector3d deltapos=ZEROVEC;
  if (!first_time)
  {
    deltapos = my_entity->get_rel_po().non_affine_inverse_scaled_xform(rel_delta_pos);
  }
  if (frame_delta_valid)
    deltapos += frame_delta.get_position();

  if (my_entity->has_parent())
  {
    deltapos = ((entity *)my_entity->link_ifc()->get_parent())->movable_ifc()->get_frame_abs_delta_position(false,deltapos);
  }
  return deltapos;
}

void movable_interface::set_frame_delta(po const & bob, time_value_t t)
{
  frame_delta = frame_delta*bob;
  frame_time = t;
  frame_delta_valid = true;
}


void movable_interface::invalidate_frame_delta()
{
  last_frame_delta_valid = frame_delta_valid;
  frame_delta_valid = false;
  frame_delta=po_identity_matrix;
}
