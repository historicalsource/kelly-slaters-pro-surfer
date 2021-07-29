#include "global.h"
//#pragma hdrstop
//  #include <malloc.h>

#ifndef NULL
#define NULL 0
#endif

#include "xb_alloc.h"
#include "osdevopts.h"

//  #undef malloc
//  #undef free

int g_total_alloced = 0;
int g_total_blocks = 0;

void os_alloc_init_heap()
{
}

void os_alloc_get_heap()
{
}

// initialization/shutdown.
int alloc_initialized = false;

void os_alloc_init()
{
}


void os_alloc_shutdown()
{
}


void os_alloc_push_heap( os_heaptype_t heap )
{
}


void os_forced_alloc_pop_heap( )
{
}


void os_alloc_push_heap()
{
}

void os_alloc_pop_heap()
{
}


os_heaptype_t os_alloc_get_current_heap()
{
  return HEAP_NONE;
}

os_heaptype_t os_alloc_get_associated_heap( void* p )
{
  return HEAP_NONE;
}

void* os_alloc_get_heap_base()
{
  return NULL;
}

unsigned int os_alloc_get_heap_size()
{
  return 0;
}

// OS independant malloc interface.  
void* os_malloc(int size)
{
  return malloc(size);
}
 
void os_free(void* p)
{
  free( p );
}

// 32 byte aligned malloc interface
void* os_malloc32(int size)
{
  return malloc(size);
}

void os_free32(void* p)
{
  free(p);
}
