// script_lib_sound_inst.h
#ifndef _SCRIPT_LIB_SOUND_INST_H
#define _SCRIPT_LIB_SOUND_INST_H

#ifdef GCCULL
#include "script_library_class.h"


class sound_instance;


///////////////////////////////////////////////////////////////////////////////
// script library class: sound_instance
///////////////////////////////////////////////////////////////////////////////

class slc_sound_instance_t : public script_library_class
{
public:
  // constructor required
  slc_sound_instance_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
};

// vm_stack data representation
typedef sound_instance* vm_sound_instance_t;

// pointer to single instance of library class
extern slc_sound_instance_t* slc_sound_instance;

#endif //GCCULL
#endif  // _SCRIPT_LIB_SOUND_INST_H
