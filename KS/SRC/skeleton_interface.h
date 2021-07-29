#ifndef ENTITY_SKELETON_INTERFACE_CLASS_HEADER
#define ENTITY_SKELETON_INTERFACE_CLASS_HEADER

#include "global.h"

#include "algebra.h"
#include "po.h"
#include "entity_interface.h"

class bone;
class entity;

class skeleton_interface : public entity_interface
{
  protected:
    // Allocated array of abs_po's.
    po *abs_po;
	po *h_abs_po;  //  for handedness changes
    int po_count;

  public:
    skeleton_interface(entity *_my_entity)
      : entity_interface(_my_entity)
    {
      assert(_my_entity != NULL);

      abs_po = NULL;
	  h_abs_po = NULL;
      po_count = 0;
    }

    virtual ~skeleton_interface()
    {
      if (abs_po != NULL)
      {
        delete []abs_po;
        abs_po = NULL;

		delete []h_abs_po;
		h_abs_po = NULL;

        po_count = 0;
      }
    }

    void initialize(const int &po_num)
    {
      assert(my_entity != NULL);
      assert(po_num > 0);
      assert(abs_po == NULL);
      assert(po_count == 0);

      abs_po = NEW po[po_num];
	  h_abs_po = NEW po[po_num];
      po_count = po_num;
    }

    po *get_bone_abs_po(const int &bone_idx)
    {
      assert(bone_idx >= 0 && bone_idx < po_count);
      assert(abs_po != NULL);

      return &abs_po[bone_idx];
    }

    void set_bone_abs_po(const int &bone_idx, const po &new_po)
    {
      assert(bone_idx >= 0 && bone_idx < po_count);
      assert(abs_po != NULL);

      abs_po[bone_idx] = new_po;
    }

    const po *get_ptr_to_bones() { assert(abs_po); return abs_po; }
	const po *get_ptr_to_handed_bones() { assert(h_abs_po); return h_abs_po; }
    const int bone_count() { return po_count; }

    void connect_bone_abs_po(const int &bone_idx, bone *new_bone);
};


#endif//ENTITY_SKELETON_INTERFACE_CLASS_HEADER
