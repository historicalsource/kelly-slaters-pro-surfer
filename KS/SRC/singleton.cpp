////////////////////////////////////////////////////////////////////////////////
/*
  singleton
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "singleton.h"

/*
auto_singleton* auto_singleton::singleton_list;

void auto_singleton::delete_all()
{
  static bool called_once = false;
  assert( !called_once );
  called_once = true;
  auto_singleton* list_iterator = singleton_list;
  while (list_iterator)
  {
    auto_singleton* delete_me = list_iterator;
    list_iterator = delete_me->next_singleton;
    delete delete_me;
  }
}*/