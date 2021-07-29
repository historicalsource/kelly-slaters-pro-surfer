// vm_symbol_list.h
#ifndef _VM_SYMBOL_LIST_H
#define _VM_SYMBOL_LIST_H


#include "vm_symbol.h"
#include <list>

#ifndef __GNUC__
#pragma warning(disable:4786)  // disable 255-character debug identifier limit warning
#endif

typedef list<vm_symbol> vm_symbol_list;


#endif  // _VM_SYMBOL_LIST_H
