// script_lib_controller.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_signal.h"
#include "script_lib_controller.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "inputmgr.h"
#include "joystick.h"


script_controller* script_pad = NULL;

void construct_script_controllers()
{
  assert( script_pad == NULL );
  script_pad = NEW script_controller[2];
}

void destruct_script_controllers()
{
  delete[] script_pad;
  script_pad = NULL;
}

// read an controller value (by id) from a stream
void slc_controller_t::read_value(chunk_file& fs,char* buf)
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find controller and write value to buffer
  id.to_upper();
  *(vm_script_controller_t*)buf = (vm_script_controller_t)find_instance(id);
}

// find named instance of controller
unsigned slc_controller_t::find_instance(const stringx& n) const
{
  if (n=="NULL") return (unsigned)0;
  if (n=="CONTROLLER_1") return (unsigned)&script_pad[0];
  if (n=="CONTROLLER_2") return (unsigned)&script_pad[1];

  return (unsigned)0;
}


void register_controller_lib()
{
  // pointer to single instance of library class
  NEW slc_controller_t("script_controller",4,"signaller");

//  NEW slf_controller_get_state_t(slc_controller,"get_state()");
//  NEW slf_controller_set_state_t(slc_controller,"set_state(num)");
}













script_controller::script_controller()
{
}

script_controller::~script_controller()
{
}

/*
void script_controller::clear_callbacks()
{
  for(int i=0; i<n_signals(); ++i)
    signal_ptr(i)->clear_callbacks();
}
*/

void script_controller::update()
{
  input_device *dev = input_mgr::inst()->get_device(JOYSTICK_DEVICE);

  if(dev)
  {
    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_START), JOY_PS2_START) == AXIS_MAX)
      raise_signal(START_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_START), JOY_PS2_START) == AXIS_MIN)
      raise_signal(START_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNX), JOY_PS2_BTNX) == AXIS_MAX)
      raise_signal(X_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNX), JOY_PS2_BTNX) == AXIS_MIN)
      raise_signal(X_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNO), JOY_PS2_BTNO) == AXIS_MAX)
      raise_signal(CIRCLE_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNO), JOY_PS2_BTNO) == AXIS_MIN)
      raise_signal(CIRCLE_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNSQ), JOY_PS2_BTNSQ) == AXIS_MAX)
      raise_signal(SQUARE_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNSQ), JOY_PS2_BTNSQ) == AXIS_MIN)
      raise_signal(SQUARE_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNTR), JOY_PS2_BTNTR) == AXIS_MAX)
      raise_signal(TRIANGLE_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNTR), JOY_PS2_BTNTR) == AXIS_MIN)
      raise_signal(TRIANGLE_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNL1), JOY_PS2_BTNL1) == AXIS_MAX)
      raise_signal(L1_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNL1), JOY_PS2_BTNL1) == AXIS_MIN)
      raise_signal(L1_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNL2), JOY_PS2_BTNL2) == AXIS_MAX)
      raise_signal(L2_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNL2), JOY_PS2_BTNL2) == AXIS_MIN)
      raise_signal(L2_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNR1), JOY_PS2_BTNR1) == AXIS_MAX)
      raise_signal(R1_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNR1), JOY_PS2_BTNR1) == AXIS_MIN)
      raise_signal(R1_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNR2), JOY_PS2_BTNR2) == AXIS_MAX)
      raise_signal(R2_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_BTNR2), JOY_PS2_BTNR2) == AXIS_MIN)
      raise_signal(R2_RELEASED);


    // Still needs some fixing
    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) > 0.0f && dev->get_axis_state(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) == AXIS_MAX)
      raise_signal(RIGHT_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) < 0.0f && dev->get_axis_old_state(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) == AXIS_MAX)
      raise_signal(RIGHT_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) < 0.0f && dev->get_axis_state(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) == AXIS_MIN)
      raise_signal(LEFT_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) > 0.0f && dev->get_axis_old_state(dev->get_axis_id(JOY_PS2_LX), JOY_PS2_LX) == AXIS_MIN)
      raise_signal(LEFT_RELEASED);


    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) > 0.0f && dev->get_axis_state(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) == AXIS_MAX)
      raise_signal(DOWN_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) < 0.0f && dev->get_axis_old_state(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) == AXIS_MAX)
      raise_signal(DOWN_RELEASED);

    if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) < 0.0f && dev->get_axis_state(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) == AXIS_MIN)
      raise_signal(UP_PRESSED);
    else if(dev->get_axis_delta(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) > 0.0f && dev->get_axis_old_state(dev->get_axis_id(JOY_PS2_LY), JOY_PS2_LY) == AXIS_MIN)
      raise_signal(UP_RELEASED);
  }
}



/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

static const char* script_controller_signal_names[] =
{
  #define MAC(label,str)  str,
  #include "script_controller_signals.h"
  #undef MAC
};

unsigned short script_controller::get_signal_id( const char *name )
{
  unsigned idx;

  for( idx = 0; idx < (sizeof(script_controller_signal_names)/sizeof(char*)); ++idx )
  {
    unsigned offset = strlen(script_controller_signal_names[idx])-strlen(name);

    if( offset > strlen( script_controller_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&script_controller_signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
  }

  // not found
  return signaller::get_signal_id( name );
}

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void script_controller::register_signals()
{
  // for descendant class, replace "script_controller" with appropriate string
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "script_controller_signals.h"
  #undef MAC
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* script_controller::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= (unsigned short)PARENT_SYNC_DUMMY )
    return signaller::get_signal_name( idx );
  else
    return script_controller_signal_names[idx-PARENT_SYNC_DUMMY-1];
}


