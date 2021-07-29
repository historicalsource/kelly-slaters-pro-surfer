// script_lib_anim.h
#ifndef _SCRIPT_LIB_ANIM_H
#define _SCRIPT_LIB_ANIM_H


#include "script_library_class.h"


class entity_anim_tree;


///////////////////////////////////////////////////////////////////////////////
// script library class: anim
///////////////////////////////////////////////////////////////////////////////

class slc_anim_t : public script_library_class
  {
  public:
    // constructor required
    slc_anim_t(const char* n,int sz) : script_library_class(n,sz) {}
  };

// vm_stack data representation
// CTT 04/04/00: TEMPORARY:
// for safety, this should be an integer handle value, but I don't have time to
// implement that right now
typedef entity_anim_tree* vm_anim_t;

// pointer to single instance of library class
extern slc_anim_t* slc_anim;


#endif  // _SCRIPT_LIB_ANIM_H
