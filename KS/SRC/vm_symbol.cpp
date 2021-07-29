// vm_symbol.cpp
#include "global.h"

#include "vm_symbol.h"

#ifdef _SCC  // script compiler support
//#include "symbol.h"
#endif

#include "chunkfile.h"


// CLASS vm_symbol

// Constructors

vm_symbol::vm_symbol()
:   type_name(),
    name(),
    offset(0)
  {
  }

vm_symbol::vm_symbol(const vm_symbol& b)
:   type_name(b.type_name),
    name(b.name),
    offset(b.offset)
  {
  }

#ifdef _SCC  // script compiler support
vm_symbol::vm_symbol(const symbol& b)
:   type_name(b.get_type().get_name()),
    name(b.get_name()),
    offset(b.get_offset())
  {
  }
#endif


// Friends

#ifndef NO_SERIAL_IN
void serial_in(chunk_file& io,vm_symbol* s)
  {
  serial_in(io,&s->type_name);
  serial_in(io,&s->name);
  serial_in(io,&s->offset);
  }
#endif

#if !defined(NO_SERIAL_OUT)
void serial_out(chunk_file& io,const vm_symbol& s)
  {
  serial_out(io,s.type_name);
  serial_out(io,s.name);
  serial_out(io,s.offset);
  }
#endif