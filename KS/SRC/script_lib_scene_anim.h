// script_lib_scene_anim.h
#ifndef _SCRIPT_LIB_SCENE_ANIM_H
#define _SCRIPT_LIB_SCENE_ANIM_H


#include "script_library_class.h"
#include "scene_anim.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: scene_anim
///////////////////////////////////////////////////////////////////////////////

class slc_scene_anim_t : public script_library_class
  {
  public:
    // constructor required
    slc_scene_anim_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef scene_anim_handle_t vm_scene_anim_t;

// pointer to single instance of library class
extern slc_scene_anim_t* slc_scene_anim;


#endif  // _SCRIPT_LIB_scene_anim_H
