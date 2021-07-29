// script_lib_mfg.h
#ifndef _SCRIPT_LIB_MFG_H
#define _SCRIPT_LIB_MFG_H

#include "script_library_class.h"
#include "ostimer.h"
#include "signals.h"

class script_mfg : public signaller
{
public:
  script_mfg();
  virtual ~script_mfg();

/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////
public:
  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
    {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = signaller::N_SIGNALS - 1,
    #define MAC(label,str)  label,
    #include "script_mfg_signals.h"
    #undef MAC
    N_SIGNALS
    };

  // This static function must be implemented by every class which can generate
  // signals, and is called once only by the application for each such class;
  // the effect is to register the name and local id of each signal with the
  // signal_manager.  This call must be performed before any signal objects are
  // actually created for this class (via signaller::signal_ptr(); see signal.h).
  static void register_signals();

  static unsigned short get_signal_id( const char *name );

private:
  // Every descendant of signaller that expects to generate signals and has
  // defined its own local list of signal ids should implement this virtual
  // function for the construction of the signal list, so that it will reserve
  // exactly the number of signal pointers required, on demand.
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

protected:
  // This virtual function, used only for debugging purposes, returns the
  // name of the given local signal
  virtual const char* get_signal_name( unsigned short idx ) const;
};


///////////////////////////////////////////////////////////////////////////////
// script library class: mfg
///////////////////////////////////////////////////////////////////////////////

class slc_mfg_t : public script_library_class
  {
  public:
    // constructor required
    slc_mfg_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read an mfg value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of mfg
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef script_mfg* vm_script_mfg_t;

//extern script_mfg g_script_mfg;

#endif  // _SCRIPT_LIB_ENTITY_H
