#include "global.h"

#include "link_interface.h"
#include "bone.h"


link_interface::~link_interface()
{
  assert(my_bone);

  clear_parent();

  // this used to call remove_child but it optimized badly on VC++
  // so I duplicated code to make it understand
  while(my_child != NULL)
  {
    assert(my_child->has_link_ifc());
    assert(my_bone);
    assert(my_child->link_ifc()->get_parent() == my_bone);

    bone* bad_kid = my_child;
    set_first_child(bad_kid->link_ifc()->my_brother);
    bad_kid->link_ifc()->my_parent = NULL;
    bad_kid->link_ifc()->my_brother = NULL;
  }


  // Only delete the abs_po of our container bone if we created it.
  if (link_manages_abs_po && my_bone->my_abs_po != &my_bone->my_rel_po)
    delete my_bone->my_abs_po;
}


// Only called by my parents, telling me to update my abs po information.
void link_interface::update_abs_po_family()
{
  assert(my_bone);

  // update my children
  if (my_child != NULL)
  {
    assert (my_child->has_link_ifc());

#if defined(TARGET_XBOX)
    if(my_parent)
    {
      assert( my_parent->get_rel_po().get_position().is_valid());
      assert( my_parent->get_abs_po().get_position().is_valid());      
    }
    
    if(my_brother)
    {
      assert( my_brother->get_rel_po().get_position().is_valid());
      assert( my_brother->get_abs_po().get_position().is_valid());      
    }
    
#endif /* TARGET_XBOX JIV DEBUG */
    
    my_child->update_abs_po();

#if defined(TARGET_XBOX)
    if(my_child)
    {
      assert( my_child->get_rel_po().get_position().is_valid());
      assert( my_child->get_abs_po().get_position().is_valid());      
    }
#endif /* TARGET_XBOX JIV DEBUG */
   

    // update my brothers (part of update children)
    bone *it = my_child;
    assert(it->has_link_ifc());
    while(it->link_ifc()->my_brother != NULL)
    {
      it->link_ifc()->my_brother->update_abs_po();
      it = it->link_ifc()->my_brother;
      assert(!it || it->has_link_ifc());
    }
  }
}

void link_interface::update_handed_abs_po_family()
{
  assert(my_bone);

  // update my children
  if (my_child != NULL)
  {
    assert (my_child->has_link_ifc());
    my_child->update_handed_abs_po();

    // update my brothers (part of update children)
    bone *it = my_child;
    assert(it->has_link_ifc());
    while(it->link_ifc()->my_brother != NULL)
    {
      it->link_ifc()->my_brother->update_handed_abs_po();
      it = it->link_ifc()->my_brother;
      assert(!it || it->has_link_ifc());
    }
  }
}

void link_interface::set_handed_axis_family(int axis)
{
	assert(my_bone);

  // update my children
  if (my_child != NULL)
  {
    assert (my_child->has_link_ifc());
    my_child->set_handed_axis(axis);

    // update my brothers (part of update children)
    bone *it = my_child;
    assert(it->has_link_ifc());
    while(it->link_ifc()->my_brother != NULL)
    {
      it->link_ifc()->my_brother->set_handed_axis(axis);
      it = it->link_ifc()->my_brother;
      assert(!it || it->has_link_ifc());
    }
  }
}

void link_interface::remove_child(bone *bad_kid)
{
  assert(my_bone);
  assert(bad_kid);
  assert(bad_kid->has_link_ifc());
  assert(bad_kid->link_ifc()->get_parent() == my_bone);

  bad_kid->link_ifc()->clear_parent();
}

void link_interface::clear_parent()
{
  // remove me from my parent's children list, and from the sibling list
  if (my_parent != NULL)
  {
    // We shouldn't be clearing a parent if the parent has no children.
    link_interface *parent_ifc = my_parent->link_ifc();
    assert(parent_ifc->get_first_child());

    // check if we are the first child of our parent.
    if (parent_ifc->get_first_child() == my_bone)
    {
      parent_ifc->set_first_child(my_brother);
      my_parent = NULL;
      my_brother = NULL;
    }
    else
    {
      // We are later in the sibling list
      const bone *prev = parent_ifc->get_first_child();
      const bone *curr = prev->link_ifc()->get_next_sibling();

      while (curr != NULL)
      {
        if (curr == my_bone)
        {
          // I found myself in the sibling list
          prev->link_ifc()->set_next_sibling(my_brother);
          my_parent = NULL;
          my_brother = NULL;
          break;
        }
        // Go to the next sibling
        prev = curr;
        curr = curr->link_ifc()->get_next_sibling();
      }
      // We shouldn't be clearing the parent if we aren't a child of this parent...
      assert(curr != NULL);
    }
  }
}

void link_interface::add_child(bone *good_kid)
{
  assert(my_bone);
  assert(good_kid);
  assert(good_kid->has_link_ifc());

  good_kid->link_ifc()->set_parent(my_bone);
}

void link_interface::set_parent(bone *new_parent)
{
  // add me to my NEW parent's children list, and from the sibling list
  if(new_parent == NULL)
  {
    clear_parent();
  }
  else if(new_parent != my_parent)
  {
    if(my_parent != NULL)
      clear_parent();

    assert(new_parent->has_link_ifc());

    // create an abs_po for my_bone if there isn't a skeleton
    if (link_manages_abs_po == true && my_bone->my_abs_po == &my_bone->my_rel_po)
    {
      my_bone->my_abs_po = NEW po(my_bone->my_rel_po * new_parent->get_abs_po());
    }

    link_interface *parent_ifc = new_parent->link_ifc();

    // check if we are the first child of our parent.
    if (parent_ifc->get_first_child() == NULL)
    {
      parent_ifc->set_first_child(my_bone);
    }
    else
    {
      const bone *prev = parent_ifc->get_first_child();
      const bone *curr = prev->link_ifc()->get_next_sibling();

      while (curr != NULL)
      {
        prev = curr;
        curr = curr->link_ifc()->get_next_sibling();
      }
      // tack myself on at the end of the list (to preserve order of children)
      prev->link_ifc()->set_next_sibling(my_bone);
    }
    my_parent = new_parent;
    my_brother = NULL;
  }
}


/*
void link_interface::clear_parent()
{
  if (my_parent) {
    my_parent->erase_child(this);

    my_parent = NULL;


    if ( get_parent() )
    {
      get_parent()->ci->children.erase( find(get_parent()->ci->children.begin(), get_parent()->ci->children.end(), this) );
      pi->parent = NULL;
      pi->parent_key = 0;
    }
  }
}


link_interface* the_parent;

void link_interface::set_parent( link_interface* p )
{
  link_interface* oldp = get_parent();
  clear_parent();
  if ( p )
  {
    create_parent_info( p );
  }
  if ( pi && p!=oldp )
  {
    ++pi->local_key;
    frame_done();
    update_abs_po();
  }
}

// change parent without affecting position in world
void link_interface::set_parent_rel( link_interface* p )
{
  link_interface* oldp = get_parent();
  clear_parent();
  if ( p )
  {
    create_parent_info( p );
  }
  if ( pi && p!=oldp )
  {
    if ( p == NULL )
      my_po = pi->my_abs_po;
    else
    {
      pi->my_abs_po = my_po;
      my_po = my_po * p->get_abs_po().inverse();
    }
  }
}

void link_interface::create_parent_info( link_interface* p )
{
  // dynamic allocation allowed even on static entities because we make a
  // special effort to save these pointers as necessary in save_dynamic_heap();
  // see dynamic save/restore support section, below, and app.cpp
  if ( !pi )
    pi = NEW parent_info;
  the_parent = p;
  pi->parent = p;
  if ( !get_parent()->pi )
  {
    get_parent()->pi = NEW parent_info;
    get_parent()->pi->my_abs_po=po_identity_matrix;
  }
  if ( !get_parent()->ci )
    get_parent()->ci = NEW children_info;
  get_parent()->ci->children.push_back( this );
}

const po& entity::get_abs_po() const
{
  if(pi && pi->parent)
  {
	  return pi->my_abs_po;
  }
  else
    return my_po;
}

void entity::set_rel_position(const vector3d& p)
{
  #ifdef DEBUG
  if (g_entity_posn_checker /*! && get_flavor()==ENTITY_CHARACTER !* /)
  {
    vector3d oldpos,newpos;
    oldpos = get_abs_position();
    my_po.set_rel_position(p);
    update_abs_po();
    newpos = get_abs_position();
    assert((oldpos-newpos).length2()<100);
  }
  else
  #endif
  {
    my_po.set_rel_position(p);
    update_abs_po();
  }
}

void entity::set_abs_position(const vector3d &p)
{
  my_po.set_rel_position(p);
  if (pi)
  {
    pi->my_abs_po = my_po;
  }
  po_changed();
}

void entity::set_orientation(const orientation& o)
{
  my_po.set_orientation(o);
  update_abs_po();
}

void entity::set_orientation(const po& p)
{
  my_po.set_orientation(p);
  update_abs_po();
}

void entity::set_rel_po(const po & the_po)
{
  #ifdef DEBUG
/*!  if (g_entity_posn_checker && get_flavor()==ENTITY_CHARACTER)
  {
    vector3d oldpos,newpos;
    oldpos = get_abs_position();

    my_po = the_po;
    update_abs_po();

    newpos = get_abs_position();
    assert((oldpos-newpos).length2()<100);
  }
  else
!* /
  #endif
  {
    my_po = the_po;
    update_abs_po();
  }
}

void entity::po_changed()
{
  // derived classes may want to know when po changes
  // we just want to make sure po isn't corrupt
  vector3d pos=get_abs_position();
  assert(pos.is_valid());
  if (get_flavor() != ENTITY_CAMERA)
    assert(!get_rel_po().has_nonuniform_scaling());
}



*/
