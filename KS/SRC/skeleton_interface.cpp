#include "global.h"

#include "skeleton_interface.h"
#include "entity.h"

void skeleton_interface::connect_bone_abs_po(const int &bone_idx, bone *new_bone)
{
  assert(bone_idx >= 0 && bone_idx < po_count);
  assert(abs_po != NULL);
  assert(new_bone->my_abs_po == &new_bone->my_rel_po);

  new_bone->my_abs_po = &abs_po[bone_idx];
  new_bone->my_handed_abs_po = &h_abs_po[bone_idx];
  new_bone->set_part_of_skeleton(true);
}
