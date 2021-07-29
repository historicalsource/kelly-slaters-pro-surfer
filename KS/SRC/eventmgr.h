#ifndef EVENTMGR_H
#define EVENTMGR_H
/*-------------------------------------------------------------------------------------------------------

  EVENTMGR.H - Generic global event manager 

  Maintains a list of callbacks to be executed whenever certain events
  happen.  For instance if the application gains or loses focus, all
  registered event handlers are notified by receiving an ET_ACTIVATE event.
  The callbacks do things like give up the display, input devices, etc.

-------------------------------------------------------------------------------------------------------*/
#include "types.h"
#include "singleton.h"

#include <list>


enum event_t
{
  ET_QUIT,  // app is terminating (param always 0) (usually not needed for consoles)
  ET_ACTIVATE, // param is 0 if deactivating, or 1 if activating (not needed for consoles)
  // add more events here
};


class event_handler;


class event_mgr : public singleton
{
public:
  DECLARE_SINGLETON(event_mgr);

  // Register/unregister an event handler with the system.
  void register_handler(event_handler* h);
  void unregister_handler(event_handler* h);

  // Send an event to the system.  All registered handlers will be notified.
  void dispatch(event_t type, uint32 param=0);

private:
  list<event_handler *> handlers;
};


// Derive from this class to implement an event_handler, and 
// instances will automatically register themselves with the event_mgr
class event_handler
{
public:
  event_handler()  { event_mgr::inst()->register_handler  (this); }
  virtual ~event_handler() { event_mgr::inst()->unregister_handler(this); }
  virtual void handle_event(event_t type, uint32 param) = 0;
};


#endif // EVENTMGR_H