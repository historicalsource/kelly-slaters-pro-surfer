#ifndef ENTITY_SLAVE_INTERFACE_CLASS_HEADER
#define ENTITY_SLAVE_INTERFACE_CLASS_HEADER

#include "entity_interface.h"

class entity;
class owner_interface;

class slave_interface : public entity_interface
{
  private:
    entity * my_owner;
    entity * next_slave;

    void set_owner(entity *new_owner);
    void clear_owner();
    void set_next_slave(entity *new_slave) { next_slave = new_slave; }

  public:
    slave_interface(entity *_my_entity)
      : entity_interface(_my_entity)
    {
      my_owner = NULL;
      next_slave = NULL;
    }

    const entity * get_owner() const { return my_owner; }

    const entity * get_next_slave() const { return next_slave; }

    friend class owner_interface;
};

#endif//ENTITY_SLAVE_INTERFACE_CLASS_HEADER
 