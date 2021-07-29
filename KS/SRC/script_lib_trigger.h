// script_lib_trigger.h
#ifndef _SCRIPT_LIB_TRIGGER_H
#define _SCRIPT_LIB_TRIGGER_H


#include "script_library_class.h"


class trigger;


///////////////////////////////////////////////////////////////////////////////
// script library class: trigger
///////////////////////////////////////////////////////////////////////////////

class slc_trigger_t : public script_library_class
  {
  public:
    // constructor required
    slc_trigger_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a trigger value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of trigger
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef trigger* vm_trigger_t;

// pointer to single instance of library class
extern slc_trigger_t* slc_trigger;


#endif  // _SCRIPT_LIB_TRIGGER_H
