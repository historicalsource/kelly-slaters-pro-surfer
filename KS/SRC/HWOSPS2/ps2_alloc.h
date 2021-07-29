//
// PS2 implementation of the allocation interface
//
#ifndef PS2_ALLOC_H
#define PS2_ALLOC_H

// initialization/shutdown.
void os_alloc_init();
void os_alloc_shutdown();

enum os_heaptype_t
  {
  HEAP_NONE = -1,
  HEAP_STATIC,
  HEAP_DYNAMIC,
  N_HEAPS
  };

void os_alloc_push_heap( os_heaptype_t heap );
void os_alloc_pop_heap();

os_heaptype_t os_alloc_get_current_heap();
os_heaptype_t os_alloc_get_associated_heap( void* p );

void* os_alloc_get_heap_base();
unsigned int os_alloc_get_heap_size();

// OS independant malloc interface.  
void* os_malloc(int size);
void os_free(void* p);

// 32 byte aligned malloc interface
void* os_malloc32(int size);
void os_free32(void* p);


#define PS2_SCRATCHPAD_ADDR 0x70000000
#define PS2_SCRATCHPAD_ADDR_4 (PS2_SCRATCHPAD_ADDR + (1024*8))
#define PS2_SCRATCHPAD_ADDR_2A (PS2_SCRATCHPAD_ADDR + (1024*12))
#define PS2_SCRATCHPAD_ADDR_2B (PS2_SCRATCHPAD_ADDR + (1024*14))
#define PS2_SCRATCHPAD_SIZE   (16 * 1024)
#define PS2_SCRATCHPAD_SIZE_8  (8 * 1024)
#define PS2_SCRATCHPAD_SIZE_4  (4 * 1024)
#define PS2_SCRATCHPAD_SIZE_2  (2 * 1024)

#include <string.h>

extern int g_spsave;
extern unsigned int g_spbuffer;

class scratchpad
{
  public:
    scratchpad()
    {
#ifdef BUILD_DEBUG
      // pad around a 'vitual' scratchpad with all sevens...
      // we'll check for these later to test for not busting out of our size limit
      memset(buffer8_pad, 0x77, PS2_SCRATCHPAD_SIZE);
      buffer8 = buffer8_pad + (PS2_SCRATCHPAD_SIZE / 4);
      memset(buffer8, 0, PS2_SCRATCHPAD_SIZE_8);

      memset(buffer4_pad, 0x77, PS2_SCRATCHPAD_SIZE);
      buffer4 = buffer4_pad + (PS2_SCRATCHPAD_SIZE / 4);
      memset(buffer4, 0, PS2_SCRATCHPAD_SIZE_4);

      memset(buffer2a_pad, 0x77, PS2_SCRATCHPAD_SIZE);
      buffer2a = buffer2a_pad + (PS2_SCRATCHPAD_SIZE / 4);
      memset(buffer2a, 0, PS2_SCRATCHPAD_SIZE_2);

      memset(buffer2b_pad, 0x77, PS2_SCRATCHPAD_SIZE);
      buffer2b = buffer2b_pad + (PS2_SCRATCHPAD_SIZE / 4);
      memset(buffer2b, 0, PS2_SCRATCHPAD_SIZE_2);

      // string identifying who is using the scratch pad buffer (debug only)
      buffer8_user[0] = '\0';
      buffer4_user[0] = '\0';
      buffer2a_user[0] = '\0';
      buffer2b_user[0] = '\0';
#else
      buffer8 = (unsigned char *)PS2_SCRATCHPAD_ADDR;
      buffer4 = (unsigned char *)PS2_SCRATCHPAD_ADDR_4;
      buffer2a = (unsigned char *)PS2_SCRATCHPAD_ADDR_2A;
      buffer2b = (unsigned char *)PS2_SCRATCHPAD_ADDR_2B;
      memset((void *)PS2_SCRATCHPAD_ADDR, 0, PS2_SCRATCHPAD_SIZE);
#endif
      buffer8_used = false;
      buffer4_used = false;
      buffer2a_used = false;
      buffer2b_used = false;
    }
    ~scratchpad()
    {
#ifdef BUILD_DEBUG
      check_buffer8();
      check_buffer4();
      check_buffer2('a');
      check_buffer2('b'); 
#endif      
    }
      
    // the string parameter should be filled in with a constant string identifying
    // the place in the code where we use this buffer.  It's only used in debug builds
    unsigned char *acquire_2kb_buffer(const char *debug_context_str)
    {
      if (buffer2a_used == false)
      {
        buffer2a_used = true;
#ifdef BUILD_DEBUG
        strncpy(buffer2a_user, debug_context_str, 255);
        buffer2a_user[255] = '\0';
#endif
        return buffer2a;
      }
      if (buffer2b_used == false)
      {
        buffer2b_used = true;
#ifdef BUILD_DEBUG
        strncpy(buffer2b_user, debug_context_str, 255);
        buffer2b_user[255] = '\0';
#endif
        return buffer2b;
      }
      // sorry...
      return NULL;
    }

    unsigned char *acquire_4kb_buffer(const char *debug_context_str)
    {
      if (buffer4_used == false)
      {
        buffer4_used = true;
#ifdef BUILD_DEBUG
        strncpy(buffer4_user, debug_context_str, 255);
        buffer4_user[255] = '\0';
#endif
        return buffer4;
      }
      return NULL;
    }

    unsigned char *acquire_8kb_buffer(const char *debug_context_str)
    {
      if (buffer8_used == false)
      {
        buffer8_used = true;
#ifdef BUILD_DEBUG
        strncpy(buffer8_user, debug_context_str, 255);
        buffer8_user[255] = '\0';
#endif
        return buffer8;
      }
      return NULL;
    }

    void release_2kb_buffer(const char *debug_context_str, unsigned char *buffer)
    {
      if (buffer == buffer2a)
      {
#ifdef BUILD_DEBUG
        assert(strcmp(debug_context_str, buffer2a_user) == 0);
        assert(check_buffer2('a'));
#endif
        buffer2a_used = false;
      }
      else 
      {
        assert(buffer == buffer2b);
#ifdef BUILD_DEBUG
        assert(strcmp(debug_context_str, buffer2b_user) == 0);
        assert(check_buffer2('a'));
#endif
        buffer2b_used = false;
      }
    }

    void release_4kb_buffer(const char *debug_context_str, unsigned char *buffer)
    {
      if (buffer == buffer4)
      {
#ifdef BUILD_DEBUG
        assert(strcmp(debug_context_str, buffer4_user) == 0);
        assert(check_buffer4());
#endif
        buffer4_used = false;
      }
    }

    void release_8kb_buffer(const char *debug_context_str, unsigned char *buffer)
    {
      if (buffer == buffer8)
      {
#ifdef BUILD_DEBUG
        assert(strcmp(debug_context_str, buffer8_user) == 0);
        assert(check_buffer8());
#endif
        buffer8_used = false;
      }
    }

    unsigned char *buffer8;
    unsigned char *buffer4;
    unsigned char *buffer2a;
    unsigned char *buffer2b;

  private:
    bool buffer8_used;
    bool buffer4_used;
    bool buffer2a_used;
    bool buffer2b_used;

#ifdef BUILD_DEBUG
    unsigned char buffer8_pad[PS2_SCRATCHPAD_SIZE];
    unsigned char buffer4_pad[PS2_SCRATCHPAD_SIZE];
    unsigned char buffer2a_pad[PS2_SCRATCHPAD_SIZE];
    unsigned char buffer2b_pad[PS2_SCRATCHPAD_SIZE];
    char buffer8_user[256];
    char buffer4_user[256];
    char buffer2a_user[256];
    char buffer2b_user[256];

    bool check_buffer8();
    bool check_buffer4();
    bool check_buffer2(char which_buffer);
    bool check_buffer(unsigned char *where, int how_much);
#endif
};

extern scratchpad g_scratchpad;

#ifndef DONT_USE_SCRATCHPAD_FOR_STACK
#define move_stack_to_scratchpad(aquired_buffer) \
{ \
  assert(g_spbuffer == 0); \
  assert(g_spsave == 0); \
  if (aquired_buffer == g_scratchpad.buffer8) \
  { \
    g_spbuffer = (unsigned int)(aquired_buffer) + PS2_SCRATCHPAD_SIZE_8 - 16; \
  } \
  else if (aquired_buffer == g_scratchpad.buffer4) \
  { \
    g_spbuffer = (unsigned int)(aquired_buffer) + PS2_SCRATCHPAD_SIZE_4 - 16; \
  } \
  else if (aquired_buffer == g_scratchpad.buffer2a || aquired_buffer == g_scratchpad.buffer2b) \
  { \
    g_spbuffer = (unsigned int)(aquired_buffer) + PS2_SCRATCHPAD_SIZE_2 - 16; \
  } \
  assert (g_spbuffer != 0); \
  asm __volatile__("la  $8, g_spsave
                    sw  $29, 0($8)
                    la  $8, g_spbuffer
                    lw  $29, 0($8)"); \
}
#else
#define move_stack_to_scratchpad(aquired_buffer)
#endif

#ifndef DONT_USE_SCRATCHPAD_FOR_STACK
#define restore_stack_from_scratchpad() \
{ \
  assert(g_spsave != 0); \
  asm __volatile__("la  $8, g_spsave 
                    lw  $29, 0($8)"); \
  g_spbuffer = 0; \
  g_spsave = 0; \
}
#else
#define restore_stack_from_scratchpad()
#endif


#endif
