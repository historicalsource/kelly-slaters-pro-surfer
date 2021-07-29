// script_lib_switch.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_signal.h"
#include "script_lib_switch.h"
// BIGCULL#include "switch_obj.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"
#include "entityflags.h"
#include "entity_maker.h"

#if defined(TARGET_XBOX)
#include "switch_obj.h"
#endif /* TARGET_XBOX JIV DEBUG */


// read an switch_obj value (by id) from a stream
void slc_switch_obj_t::read_value(chunk_file& fs,char* buf)
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find switch_obj and write value to buffer
  id.to_upper();
  *(vm_switch_obj_t*)buf = (vm_switch_obj_t)find_instance(id);
}

// find named instance of switch_obj
unsigned slc_switch_obj_t::find_instance(const stringx& n) const
{
  if (n=="NULL") return (unsigned)0;
  const switch_obj* r = (switch_obj*)entity_manager::inst()->find_entity(entity_id(n.c_str()),IGNORE_FLAVOR,FIND_ENTITY_UNKNOWN_OK);
  if (!r)
    {
    error( "switch_obj " + n + " not found" );
    }
  return (unsigned)r;
}

// script library function:  num switch_obj::get_state()
class slf_switch_get_state_t : public script_library_class::function
{
public:
  // constructor required
  slf_switch_get_state_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_switch_obj_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result;

    result = parms->me->get_state();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_switch_get_state_t slf_switch_get_state(slc_switch_obj,"get_state()");


// script library function:  switch_obj::set_state(num)
class slf_switch_obj_set_state_t : public script_library_class::function
{
public:
  // constructor required
  slf_switch_obj_set_state_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_switch_obj_t me;
    vm_num_t state;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->state != 0.0f)
      parms->me->set_state(switch_obj::_SWITCH_ON);
    else
      parms->me->set_state(switch_obj::_SWITCH_OFF);

    SLF_DONE;
  }
};
//slf_switch_obj_set_state_t slf_switch_obj_set_state(slc_switch_obj,"set_state(num)");




void register_switch_lib()
{
  // pointer to single instance of library class
  slc_switch_obj_t* slc_switch_obj = NEW slc_switch_obj_t("switch_obj",4,"signaller");

  NEW slf_switch_get_state_t(slc_switch_obj,"get_state()");
  NEW slf_switch_obj_set_state_t(slc_switch_obj,"set_state(num)");
}
