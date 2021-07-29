// script_lib_entity.h
#ifndef _SCRIPT_LIB_SWITCH_H
#define _SCRIPT_LIB_SWITCH_H


#include "script_library_class.h"


class switch_obj;


///////////////////////////////////////////////////////////////////////////////
// script library class: entity
///////////////////////////////////////////////////////////////////////////////

class slc_switch_obj_t : public script_library_class
  {
  public:
    // constructor required
    slc_switch_obj_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read an switch_obj value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of switch_obj
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef switch_obj* vm_switch_obj_t;

// pointer to single instance of library class
extern slc_switch_obj_t* slc_switch_obj;


#endif  // _SCRIPT_LIB_ENTITY_H
