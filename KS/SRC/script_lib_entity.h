// script_lib_entity.h
#ifndef _SCRIPT_LIB_ENTITY_H
#define _SCRIPT_LIB_ENTITY_H


#include "script_library_class.h"


class entity;


///////////////////////////////////////////////////////////////////////////////
// script library class: entity
///////////////////////////////////////////////////////////////////////////////

class slc_entity_t : public script_library_class
  {
  public:
    // constructor required
    slc_entity_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read an entity value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of entity
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef entity* vm_entity_t;

// pointer to single instance of library class
extern slc_entity_t* slc_entity;


#endif  // _SCRIPT_LIB_ENTITY_H
