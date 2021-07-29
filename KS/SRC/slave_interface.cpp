#include "global.h"

#include "owner_interface.h"
#include "slave_interface.h"
#include "entity.h"

void slave_interface::clear_owner()
{
  // remove me from my owner's slaves list
  if (my_owner != NULL)
  {
    // We shouldn't be clearing a owner if the owner has no slaves.
    owner_interface *owner_ifc = my_owner->owner_ifc();
    assert(owner_ifc->get_first_slave());

    // check if we are the first slave of our owner.
    if (owner_ifc->get_first_slave() == my_entity)
    {
      owner_ifc->set_first_slave(next_slave);
      my_owner = NULL;
      next_slave = NULL;
    }
    else
    {
      // We are later in the slaves list
      const entity *prev = owner_ifc->get_first_slave();
      const entity *curr = prev->slave_ifc()->get_next_slave();

      while (curr != NULL)
      {
        if (curr == my_entity)
        {
          // I found myself in the slaves list
          prev->slave_ifc()->set_next_slave(next_slave);
          my_owner = NULL;
          next_slave = NULL;
          break;
        }
        // Go to the next sibling
        prev = curr;
        curr = curr->slave_ifc()->get_next_slave();
      }
      // We shouldn't be clearing the owner if we aren't a slave of this owner...
      assert(curr != NULL);
    }
  }
}

void slave_interface::set_owner(entity *new_owner)
{
  // add me to my NEW owner's slaves list
  assert(new_owner != NULL);
  assert(new_owner->has_owner_ifc());

  owner_interface *owner_ifc = new_owner->owner_ifc();

  // check if we are the first slave of our owner
  if (owner_ifc->get_first_slave() == NULL)
  {
    owner_ifc->set_first_slave(my_entity);
    next_slave = NULL;
  }
  else
  {
    next_slave = (entity *)owner_ifc->get_first_slave();
    owner_ifc->set_first_slave(my_entity);
  }
  my_owner = new_owner;
}
