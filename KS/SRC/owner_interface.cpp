#include "global.h"

#include "owner_interface.h"
#include "slave_interface.h"
#include "entity.h"


void owner_interface::remove_slave(entity *give_me_liberty) 
{
  assert(my_entity);
  assert(give_me_liberty);
  assert(give_me_liberty->has_slave_ifc());
  assert(give_me_liberty->slave_ifc()->get_owner() == my_entity);

  give_me_liberty->slave_ifc()->clear_owner();
}

void owner_interface::add_slave(entity *enslave_me)
{
  assert(my_entity);
  assert(enslave_me);
  assert(enslave_me->has_slave_ifc());

  enslave_me->slave_ifc()->set_owner(my_entity);
}
