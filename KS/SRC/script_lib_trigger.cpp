// script_lib_trigger.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_trigger.h"
#include "terrain.h"
#include "trigger.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "script_lib_vector3d.h"
#include "script_lib_entity.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: trigger
///////////////////////////////////////////////////////////////////////////////

// read a trigger value (by id) from a stream
void slc_trigger_t::read_value( chunk_file& fs, char* buf )
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find trigger and write value to buffer
  *(vm_trigger_t*)buf = (vm_trigger_t)find_instance(id);
}

// find named instance of trigger
unsigned slc_trigger_t::find_instance(const stringx& n) const
{
	return (unsigned)trigger_manager::inst()->find_instance(n);
}


// script library function:  vector3d trigger::get_abs_position()
class slf_trigger_get_position_t : public script_library_class::function
{
public:
  // constructor required
  slf_trigger_get_position_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_trigger_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_vector3d_t result = parms->me->get_abs_position();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_trigger_get_position_t slf_trigger_get_position(slc_trigger,"get_abs_position()");


// script library function:  entity trigger::get_triggered_ent()
class slf_trigger_get_triggered_ent_t : public script_library_class::function
{
public:
  // constructor required
  slf_trigger_get_triggered_ent_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_trigger_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = parms->me->get_triggered_ent();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_trigger_get_triggered_ent_t slf_trigger_get_triggered_ent(slc_trigger,"get_triggered_ent()");


// script library function:  trigger::set_active( num torf )
class slf_trigger_set_active_t : public script_library_class::function
{
public:
  // constructor required
  slf_trigger_set_active_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_trigger_t me;
    vm_num_t torf;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_active( parms->torf != 0 );
    SLF_DONE;
  }
};
//slf_trigger_set_active_t slf_trigger_set_active(slc_trigger,"set_active(num)");


///////////////////////////////////////////////////////////////////////////////
// Global script functions supporting trigger class
///////////////////////////////////////////////////////////////////////////////

// script library function:  trigger create_trigger( vector3d p, num r )
class slf_create_point_trigger_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_point_trigger_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_vector3d_t p;
    vm_num_t r;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    trigger* result = trigger_manager::inst()->new_point_trigger( parms->p, parms->r );
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_point_trigger_t slf_create_point_trigger("create_trigger(vector3d,num)");

// script library function:  trigger create_box_trigger( entity e )
class slf_create_box_trigger_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_box_trigger_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t e;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    trigger* result = trigger_manager::inst()->new_box_trigger( parms->e );
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_box_trigger_t slf_create_box_trigger("create_box_trigger(str)");

// script library function:  trigger create_trigger( entity e, num r )
class slf_create_entity_trigger_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_entity_trigger_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t e;
    vm_num_t r;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    trigger* result = trigger_manager::inst()->new_entity_trigger( parms->e, parms->r );
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_entity_trigger_t slf_create_entity_trigger("create_trigger(entity,num)");

// script library function:  set_use_any_char( num u )
class slf_trigger_set_use_any_char_t : public script_library_class::function
{
public:
  // constructor required
  slf_trigger_set_use_any_char_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_trigger_t me;
    vm_num_t u;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack,entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_use_any_char( parms->u != 0.0f );
    SLF_DONE;
  }
};
//slf_trigger_set_use_any_char_t slf_trigger_set_use_any_char(slc_trigger,"set_use_any_char(num)");



void register_trigger_lib()
{
  // pointer to single instance of library class
  slc_trigger_t* slc_trigger = NEW slc_trigger_t("trigger",4,"signaller");

  NEW slf_trigger_get_triggered_ent_t(slc_trigger,"get_triggered_ent()");
  NEW slf_trigger_get_position_t(slc_trigger,"get_abs_position()");
  NEW slf_trigger_set_active_t(slc_trigger,"set_active(num)");
  NEW slf_trigger_set_use_any_char_t(slc_trigger,"set_use_any_char(num)");
  NEW slf_create_point_trigger_t("create_trigger(vector3d,num)");
  NEW slf_create_entity_trigger_t("create_trigger(entity,num)");
  NEW slf_create_box_trigger_t("create_trigger(entity)");
}
