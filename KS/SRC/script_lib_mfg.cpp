// script_lib_mfg.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

// by the way, mfg stands for Message From God

#include "global.h"

#include "script_lib_signal.h"
#include "script_lib_mfg.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "inputmgr.h"
#include "joystick.h"
#include "game.h"


// read an mfg value (by id) from a stream
void slc_mfg_t::read_value(chunk_file& fs,char* buf)
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find mfg and write value to buffer
  id.to_upper();
  *(vm_script_mfg_t*)buf = (vm_script_mfg_t)find_instance(id);
}

// find named instance of mfg
unsigned slc_mfg_t::find_instance(const stringx& n) const
{
  if (n=="NULL") return (unsigned)0;
  if (n=="MFG") return (unsigned)g_game_ptr->get_script_mfg();

  return (unsigned)0;
}






// script library function:  mfg::raise_signal( num sig );
class slf_mfg_raise_signal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_mfg_raise_signal_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_script_mfg_t me;
      vm_num_t sig;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      int sig = (int)parms->sig;
      if(sig >= 0 && sig < 32)
      {
        parms->me->raise_signal((script_mfg::SIGNAL_0+sig));
      }
#ifdef TARGET_PC
      else
      {
        warning("The MFG only has 0-31 as valid signals, you passed in %d", sig);
      }
#endif

      SLF_DONE;
      }
  };
//slf_mfg_raise_signal_t slf_mfg_raise_signal(slc_mfg,"raise_signal(num)");







void register_mfg_lib()
{
  // pointer to single instance of library class
  slc_mfg_t* slc_mfg = NEW slc_mfg_t("script_mfg",4,"signaller");

  NEW slf_mfg_raise_signal_t(slc_mfg,"raise_signal(num)");
}













script_mfg::script_mfg()
{
}

script_mfg::~script_mfg()
{
}




/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

static const char* script_mfg_signal_names[] =
{
  #define MAC(label,str)  str,
  #include "script_mfg_signals.h"
  #undef MAC
};

unsigned short script_mfg::get_signal_id( const char *name )
{
  unsigned idx;

  for( idx = 0; idx < (sizeof(script_mfg_signal_names)/sizeof(char*)); ++idx )
  {
    unsigned offset = strlen(script_mfg_signal_names[idx])-strlen(name);

    if( offset > strlen( script_mfg_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&script_mfg_signal_names[idx][offset]) )
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
void script_mfg::register_signals()
{
  // for descendant class, replace "script_mfg" with appropriate string
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "script_mfg_signals.h"
  #undef MAC
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* script_mfg::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= (unsigned short)PARENT_SYNC_DUMMY )
    return signaller::get_signal_name( idx );
  else
    return script_mfg_signal_names[idx-PARENT_SYNC_DUMMY-1];
}


