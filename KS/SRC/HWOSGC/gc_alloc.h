#ifndef GC_ALLOC_H
#define GC_ALLOC_H

#include <ar.h>
#include <arq.h>

// initialization/shutdown.
void os_alloc_init( void );
void os_alloc_shutdown( void );

enum os_heaptype_t {
  HEAP_NONE = -1,
  HEAP_STATIC,
  HEAP_DYNAMIC,
  N_HEAPS
};

//#define GC_MEM_TRACK
//#define TWO_HEAPS


void os_alloc_push_heap( os_heaptype_t heap );
void os_alloc_pop_heap( void );

os_heaptype_t os_alloc_get_current_heap( void );
os_heaptype_t os_alloc_get_associated_heap( void* p );

void* os_alloc_get_heap_base( void );
unsigned int os_alloc_get_heap_size( void );

// OS independant malloc interface.  
void* os_malloc( int size );
void os_free( void* p );

// 32 byte aligned malloc interface
void* os_malloc32( int size );
void os_free32( void* p );

#endif
