// script_lib_actor.h
#ifndef _SCRIPT_LIB_ACTOR_H
#define _SCRIPT_LIB_ACTOR_H


#include "script_library_class.h"


class actor;


///////////////////////////////////////////////////////////////////////////////
// script library class: actor
///////////////////////////////////////////////////////////////////////////////

class slc_actor_t : public script_library_class
  {
  public:
    // constructor required
    slc_actor_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a actor value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of actor
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef actor* vm_actor_t;

// pointer to single instance of library class
extern slc_actor_t* slc_actor;


#endif  // _SCRIPT_LIB_ACTOR_H
