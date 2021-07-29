// vm_symbol.h
#ifndef _VM_SYMBOL_H
#define _VM_SYMBOL_H


#include "stringx.h"
#include "chunkfile.h"
class symbol;


class vm_symbol
  {
  // Data
  protected:
    stringx type_name;
    stringx name;
    int offset;

  // Constructors
  public:
    vm_symbol();
    vm_symbol(const vm_symbol& b);
    vm_symbol(const symbol& b);  // script compiler support

  // Methods
  public:
    const stringx& get_type_name() const { return type_name; }
    const stringx& get_name() const      { return name; }
    int get_offset() const               { return offset; }

  // Friends
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io,vm_symbol* s);
#endif
#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io,const vm_symbol& s);
#endif
  };


#endif  // _VM_SYMBOL_H
