// script_lib_item.h
#ifndef _SCRIPT_LIB_ITEM_H
#define _SCRIPT_LIB_ITEM_H


#include "script_library_class.h"


class item;


///////////////////////////////////////////////////////////////////////////////
// script library class: item
///////////////////////////////////////////////////////////////////////////////

class slc_item_t : public script_library_class
  {
  public:
    // constructor required
    slc_item_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a item value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of item
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef item* vm_item_t;

// pointer to single instance of library class
extern slc_item_t* slc_item;


#endif  // _SCRIPT_LIB_ITEM_H
