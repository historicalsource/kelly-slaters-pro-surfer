/*-------------------------------------------------------------------------------------------------------

  EVENTMGR.CPP - Event manager implementation

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "eventmgr.h"

DEFINE_SINGLETON(event_mgr);

void event_mgr::register_handler(event_handler* h)
{
  handlers.push_back(h);
}

void event_mgr::unregister_handler(event_handler* h)
{
  handlers.remove(h);
}

void event_mgr::dispatch(event_t type, uint32 param)
{
  for (list<event_handler *>::const_iterator it = handlers.begin(); it != handlers.end(); ++it)
    (*it)->handle_event( type, param );
}
