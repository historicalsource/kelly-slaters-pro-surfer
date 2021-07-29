// script_lib_signal.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_signal.h"
#include "signals.h"
#include "vm_stack.h"
#include "vm_thread.h"
//#include "dread_net.h"
#include "wds.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: signaller
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// script library function:  signaller::enable_signals()
//
class slf_signaller_enable_signals_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_signaller_enable_signals_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_signaller_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->enable();
      SLF_DONE;
      }
  };
//slf_signaller_enable_signals_t slf_signaller_enable_signals(slc_signaller,"enable_signals()");


///////////////////////////////////////////////////////////////////////////////
// script library function:  signaller::disable_signals()
//
class slf_signaller_disable_signals_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_signaller_disable_signals_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_signaller_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->disable();
      SLF_DONE;
      }
  };
//slf_signaller_disable_signals_t slf_signaller_disable_signals(slc_signaller,"disable_signals()");

///////////////////////////////////////////////////////////////////////////////
// script library function:  signaller::clear_callbacks()
//
class slf_signaller_clear_callbacks_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_signaller_clear_callbacks_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_signaller_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->clear_script_callbacks();
      SLF_DONE;
      }
  };
//slf_signaller_clear_callbacks_t slf_signaller_clear_callbacks(slc_signaller,"clear_callbacks()");

///////////////////////////////////////////////////////////////////////////////
// script library function:  signaller::clear_callback(str)
//
class slf_signaller_clear_callback_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_signaller_clear_callback_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_signaller_t me;
      vm_str_t str;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->clear_script_callback(*parms->str);
      SLF_DONE;
      }
  };
//slf_signaller_clear_callback_t slf_signaller_clear_callback(slc_signaller,"clear_callback(str)");


/*
// script library function:  signaller::add_to_dread_net( num )
class slf_signaller_add_to_dread_net_t : public script_library_class::function
{
public:
  // constructor required
  slf_signaller_add_to_dread_net_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_signaller_t me;
    vm_num_t flags;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    g_world_ptr->get_dread_net()->add_signaller(parms->me, parms->flags);
    SLF_DONE;
  }
};
//slf_signaller_add_to_dread_net_t slf_signaller_add_to_dread_net( slc_signaller, "add_to_dread_net(num)" );



// script library function:  signaller::remove_from_dread_net()
class slf_signaller_remove_from_dread_net_t : public script_library_class::function
{
public:
  // constructor required
  slf_signaller_remove_from_dread_net_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_signaller_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    g_world_ptr->get_dread_net()->remove_signaller(parms->me);
    SLF_DONE;
  }
};
//slf_signaller_remove_from_dread_net_t slf_signaller_remove_from_dread_net( slc_signaller, "remove_from_dread_net()" );
*/



void register_signal_lib()
{
  // pointer to single instance of library class
  slc_signaller_t* slc_signaller = NEW slc_signaller_t("signaller",4);

  NEW slf_signaller_enable_signals_t(slc_signaller,"enable_signals()");
  NEW slf_signaller_disable_signals_t(slc_signaller,"disable_signals()");
  NEW slf_signaller_clear_callbacks_t(slc_signaller,"clear_callbacks()");
  NEW slf_signaller_clear_callback_t(slc_signaller,"clear_callback(str)");

//  NEW slf_signaller_add_to_dread_net_t( slc_signaller, "add_to_dread_net(num)" );
//  NEW slf_signaller_remove_from_dread_net_t( slc_signaller, "remove_from_dread_net()" );
}
