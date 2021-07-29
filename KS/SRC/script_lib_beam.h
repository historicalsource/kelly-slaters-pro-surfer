// script_lib_beam.h
#ifndef _SCRIPT_LIB_BEAM_H
#define _SCRIPT_LIB_BEAM_H


#include "script_library_class.h"


class beam;


///////////////////////////////////////////////////////////////////////////////
// script library class: beam
///////////////////////////////////////////////////////////////////////////////

class slc_beam_t : public script_library_class
  {
  public:
    // constructor required
    slc_beam_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a beam value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of beam
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef beam* vm_beam_t;

// pointer to single instance of library class
extern slc_beam_t* slc_beam;


#endif  // _SCRIPT_LIB_BEAM_H
