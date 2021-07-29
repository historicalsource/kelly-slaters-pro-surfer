// script_lib_vector3d.h
#ifndef _SCRIPT_LIB_VECTOR3D_H
#define _SCRIPT_LIB_VECTOR3D_H


#include "script_library_class.h"
#include "algebra.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: vector3d
///////////////////////////////////////////////////////////////////////////////

class slc_vector3d_t : public script_library_class
  {
  public:
    // constructor required
    slc_vector3d_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a vector3d value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
  };

// vm_stack data representation
typedef vector3d vm_vector3d_t;

// pointer to single instance of library class
extern slc_vector3d_t* slc_vector3d;



#endif  // _SCRIPT_LIB_VECTOR3D_H
