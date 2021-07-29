// vm_stack.cpp
#include "global.h"

#include "vm_stack.h"
#include "script_object.h"
#include <memory>

// CLASS vm_stack

// Constructors

vm_stack::vm_stack(int sa, vm_thread * _my_thread)
  {
  assert(!(sa&3));
  salloc = sa;
  buffer = NEW char[sa];
  SP = buffer;
  my_thread = _my_thread;
  }

vm_stack::~vm_stack()
  {
  delete[] buffer;
  }


// Methods

vm_num_t vm_stack::pop_num()
  {
  pop(sizeof(vm_num_t));
  return *(vm_num_t*)SP;
  }

bool vm_stack::push( const char* src, int n )
  {
  memcpy(SP,src,n);
  move_SP( n );
#if REPORT_OVERFLOW
  // check for stack overflow
  if ( size() > capacity() )
    return false;
#endif
  return true;
  }


// Internal Methods

void vm_stack::init(int sa)
  {
  assert(!(sa&3));
  if (buffer)
    delete[] buffer;
  salloc = sa;
  buffer = NEW char[sa];
  SP = buffer;
  }
