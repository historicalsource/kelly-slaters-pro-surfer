#ifndef ENTITY_LINK_INTERFACE_CLASS_HEADER
#define ENTITY_LINK_INTERFACE_CLASS_HEADER

#include "global.h"

#include "algebra.h"
#include "po.h"
#include "entity_interface.h"

class bone;

class link_interface : public bone_interface
{
  protected:
    bone * my_parent;
    bone * my_child;
    bone * my_brother;
    bool link_manages_abs_po; // if more flags show up here, make this into a flag-word or bitfield flags

    void set_first_child(bone *new_child) { my_child = new_child; }
    void set_next_sibling(bone *new_sibling) { my_brother = new_sibling; }

    friend class bone;

  public:
    link_interface(bone *_my_bone)
      : bone_interface(_my_bone)
    {
      my_parent = NULL;
      my_child = NULL;
      my_brother = NULL;
      link_manages_abs_po = true;
    }

    ~link_interface();

    // these used to be const, but I there are situations where you'd want to manipulate
    // the data, so I un-consted them.  jdf 4-11-01
    bone * get_parent() const { return my_parent; }
    bone * get_first_child() const { return my_child; }
    bone * get_next_sibling() const { return my_brother; }

    void update_abs_po_family();
	void update_handed_abs_po_family();
	void set_handed_axis_family(int axis);

    void set_parent(bone *new_parent);
    void clear_parent();

    void add_child(bone *good_kid);
    void remove_child(bone *bad_kid);

    void link_does_not_manage_abs_po()
    {
      link_manages_abs_po = false;
    }
};


#endif//ENTITY_LINK_INTERFACE_CLASS_HEADER
 