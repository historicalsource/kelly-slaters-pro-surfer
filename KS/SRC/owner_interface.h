#ifndef ENTITY_OWNER_INTERFACE_CLASS_HEADER
#define ENTITY_OWNER_INTERFACE_CLASS_HEADER

#include "global.h"

#include "entity_interface.h"

class entity;

class owner_interface : public entity_interface
{
  protected:
    entity * my_slave;

    void set_first_slave(entity *new_slave) { my_slave = new_slave; }

  public:
    owner_interface(entity *_my_entity)
      : entity_interface(_my_entity)  
    {
      my_slave = NULL;
    }

    const entity * get_first_slave() const { return my_slave; }

    void add_slave(entity *good_kid);
    void remove_slave(entity *bad_kid);

    friend class slave_interface;
};



#endif//ENTITY_OWNER_INTERFACE_CLASS_HEADER
 