// script_lib_signal.h
#ifndef _SCRIPT_LIB_SIGNAL_H
#define _SCRIPT_LIB_SIGNAL_H

#include "script_library_class.h"


class signaller;


///////////////////////////////////////////////////////////////////////////////
// script library class: signaller
///////////////////////////////////////////////////////////////////////////////

class slc_signaller_t : public script_library_class
  {
  public:
    // constructor required
    slc_signaller_t( const char* n, int sz, const char* p=NULL )
    :   script_library_class( n, sz, p )
      {
      }
  };

// vm_stack data representation
typedef signaller* vm_signaller_t;

// pointer to single instance of library class
extern slc_signaller_t* slc_signaller;


#endif  // _SCRIPT_LIB_SIGNAL_H
