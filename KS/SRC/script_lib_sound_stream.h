// script_lib_sound_stream.h
#ifndef _SCRIPT_LIB_SOUND_STREAM_H
#define _SCRIPT_LIB_SOUND_STREAM_H


#ifdef GCCULL

#include "script_library_class.h"


//class sound_stream;


///////////////////////////////////////////////////////////////////////////////
// script library class: sound_stream
///////////////////////////////////////////////////////////////////////////////

class slc_sound_stream_t : public script_library_class
{
public:
  // constructor required
  slc_sound_stream_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
};

// vm_stack data representation
typedef sound_stream* vm_sound_stream_t;

// pointer to single instance of library class
extern slc_sound_stream_t* slc_sound_stream;


// stream lists
extern list<vm_sound_stream_t> g_cine_streams;
extern list<vm_sound_stream_t> g_stream_list;

#endif

#endif  // _SCRIPT_LIB_SOUND_STREAM_H
