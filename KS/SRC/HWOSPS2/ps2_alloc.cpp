#include "global.h"
//#pragma hdrstop

#ifndef NULL
#define NULL 0
#endif

#include "ps2_alloc.h"
#include "osdevopts.h"

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
 
int g_spsave = 0;
unsigned int g_spbuffer = 0;
scratchpad g_scratchpad;

#ifdef BUILD_DEBUG
bool scratchpad::check_buffer2(char which_buffer)
{
  unsigned char *pre_pad_start = NULL;
  unsigned char *post_pad_start = NULL;
  int post_pad_length = 0;
  if (which_buffer == 'a' || which_buffer == 'A')
  {
    pre_pad_start = buffer2a_pad;
    post_pad_start = buffer2a + PS2_SCRATCHPAD_SIZE_2;
  }
  else // assume buffer b
  {
    pre_pad_start = buffer2b_pad;
    post_pad_start = buffer2b + PS2_SCRATCHPAD_SIZE_2;
  }
  post_pad_length = (PS2_SCRATCHPAD_SIZE - (PS2_SCRATCHPAD_SIZE / 4) - PS2_SCRATCHPAD_SIZE_2);
  return (check_buffer(pre_pad_start, (PS2_SCRATCHPAD_SIZE / 4)) == true &&
          check_buffer(post_pad_start, post_pad_length) == true);
}

bool scratchpad::check_buffer4()
{
  unsigned char *post_pad_start = NULL;
  int post_pad_length = 0;

  post_pad_start = buffer4 + PS2_SCRATCHPAD_SIZE_4;
  post_pad_length = (PS2_SCRATCHPAD_SIZE - (PS2_SCRATCHPAD_SIZE / 4) - PS2_SCRATCHPAD_SIZE_4);

  return (check_buffer(buffer4_pad, (PS2_SCRATCHPAD_SIZE / 4)) == true &&
          check_buffer(post_pad_start, post_pad_length) == true);
}

bool scratchpad::check_buffer8()
{
  unsigned char *post_pad_start = NULL;
  int post_pad_length = 0;

  post_pad_start = buffer8 + PS2_SCRATCHPAD_SIZE_8;
  post_pad_length = (PS2_SCRATCHPAD_SIZE - (PS2_SCRATCHPAD_SIZE / 4) - PS2_SCRATCHPAD_SIZE_8);

  return (check_buffer(buffer8_pad, (PS2_SCRATCHPAD_SIZE / 4)) == true &&
          check_buffer(post_pad_start, post_pad_length) == true);
}

bool scratchpad::check_buffer(unsigned char *_where, int how_long)
{
  unsigned int *where = (unsigned int *)_where;
  // should be a multiple of 4-bytes (sizeof(int)), if this engine ever goes 64-bit, god help us all.
  assert(how_long % 4 == 0);
  int stop_here = how_long/4;
  for (int i=0; i<stop_here; ++i)
  {
    // Check if the memory was bashed, if so, assert out
    unsigned check = *(where + i);
    assert(check == 0x77777777);
  }
  return true;
}

#endif