#include "global.h"

#include "gc_alloc.h"

#include <stdlib.h>
#include <string.h>
#include "file_manager.h"
#include "osfile.h"
#include "mustash.h"
int g_total_alloced = 0;
int g_total_blocks = 0;

#undef malloc
#undef free

//#define OS_ALLOC_CHECK
//#if defined(USER_EDQ)
//#define GC_MEM_TRACK
//#define TWO_HEAPS
//#endif
#ifndef GCOLDHEAP
#ifdef GC_MEM_TRACK
//#undef GC_MEM_TRACK
#error two great tastes that taste like crap together
#endif
#endif

void os_alloc_init_heap( void )
{
	// empty
}

void os_alloc_get_heap( void )
{
	// empty
}

// initialization/shutdown.
static int alloc_initialized = false;
////////////////////////////////
//////////////TWO HEAPS/////////
#ifdef TWO_HEAPS

#define GOOD_HEAP_SIZE      1024*4000
#define HERO_MAX_STASH_SIZE 1024*1024*3

struct os_heap
{
private:
  OSHeapHandle   handle;
  char           *start;
  char           *end;
  bool           open;

public:
  os_heap()
  {
    open= true;
    // handle= -1; start= end= NULL;
  }
  ~os_heap() {}
  //os_heap(void *start, void *end)  { handle = OSCreateHeap( start, end ); }
  //~os_heap(){ OSDestroyHeap(handle); }

  void  set_close()  {open= false;}
  void  set_open()   {open= true;}

  long  create(void *_start, void *_end)
  {
    start= (char*)_start; end= (char*)_end;
    memset(start, 0xbd, end - start);
    handle = OSCreateHeap( start, end );

    return check();
  }
  void  destroy() { OSDestroyHeap(handle); }
  void* malloc(u32 size)
  {
    void *p= NULL;
    //if(open)
    {
    //#undef  _DEBUG
      p= OSAllocFromHeap(handle, size);
    //#define _DEBUG
      /*
      if(p==NULL)
        set_close();
      */
    }
    return p;
  }

  bool  free(void *ptr)
  {
    if( (char*)ptr>=start && (char*)ptr<end && open)
    {
      if( *((char*)ptr) != 0xbd)
        OSFreeToHeap(handle, ptr);
      else
        OSReport("Free for memory wich was allocated but not used\n");
      //set_open();
      return true;
    }
    return false;
  }

  void  set_current()   { OSSetCurrentHeap(handle);  }
  long  reset()
  {
    destroy();
    return create(start, end);
  }
  long  check() { return OSCheckHeap(handle); }

} good_heap, bad_heap;

#endif
/////////////TWO HEAPS/////////

int gc_heapsize=0;

void os_alloc_init( void )
{
	void* arena_low = NULL;
	void* arena_high = NULL;
	OSHeapHandle heap = 0;

	if( alloc_initialized )
	{
		return;
	}

	// get arena boundaries
	arena_low = OSGetArenaLo( );
	arena_high = OSGetArenaHi( );

	// initialize the arena with a maximum of one heap
	// we remember arena low because it can be changed
	// due to the arena's bookkeeping data
#ifdef TWO_HEAPS
	arena_low = OSInitAlloc( arena_low, arena_high, 2 );
#else
	arena_low = OSInitAlloc( arena_low, arena_high, 1 );
#endif

#ifdef DEBUG
	gc_heapsize=(int) arena_high - (int) arena_low ;
	OSReport( "OSInitAlloc: creating heap of size %d\n",  (int) arena_high - (int) arena_low );
#endif
	// set it again
	OSSetArenaLo( arena_low );
	// 32-byte align
	arena_low = (void*) OSRoundUp32B( arena_low );
	arena_high = (void*) OSRoundDown32B( arena_high );
	// create a heap
	#ifdef TWO_HEAPS
	long  heap_size;
	heap_size= good_heap.create(arena_low, (char*)arena_low + GOOD_HEAP_SIZE);
	heap_size= bad_heap.create((char*)arena_low + GOOD_HEAP_SIZE, arena_high);
	good_heap.set_open();
	bad_heap.set_open();
	good_heap.set_current();
	#else
	heap = OSCreateHeap( arena_low, arena_high );
	// set it to be the current one
	OSSetCurrentHeap( heap );
	#endif
	// now our arena has moved after the heap alloc,
	// reset its boundaries
	arena_low = arena_high;
	OSSetArenaLo( arena_low );

	alloc_initialized = true;
}

void os_alloc_shutdown( void )
{
	alloc_initialized = false;
}

void os_alloc_push_heap( void )
{
	// empty
}

void os_alloc_push_heap( os_heaptype_t heap )
{
	// empty
}

void os_alloc_pop_heap( void )
{
	// empty
}

void os_forced_alloc_pop_heap( void )
{
	// empty
}

os_heaptype_t os_alloc_get_current_heap( void )
{
  return HEAP_DYNAMIC;
}

os_heaptype_t os_alloc_get_associated_heap( void* p )
{
  return HEAP_DYNAMIC;
}

void* os_alloc_get_heap_base( void )
{
  return OSGetArenaLo( );
}

unsigned int os_alloc_get_heap_size( void )
{
  return 0;
}

#define ENSURE_ALLOC_INIT( ) \
	do { \
		if( !alloc_initialized ) { \
			os_alloc_init( ); \
		} \
	} while( 0 )

//#define OS_ALLOC_CHECK 1

#ifdef OS_ALLOC_CHECK

static int _os_n_allocs = 0;
static int _os_alloced = 0;
static int _os_largest = 0;

// OS-independent malloc routines
static void os_alloc_check( void )
{
	long l = OSCheckHeap( __OSCurrHeap );

	if( l < 0 ) {
		error( "OS heap inconsistent!\n" );
	}

}

#endif

void mal_malloc(void *_adr, int _size);
void mal_free(void *_adr);

void* os_malloc( int n )
{
	ENSURE_ALLOC_INIT( );
	if ( n<1 ) n=1;

	n = OSRoundUp32B( n );

#ifdef OS_ALLOC_CHECK
	++_os_n_allocs;
	_os_alloced += n;

	if( n > _os_largest )
	{
		_os_largest = n;
	}

	os_alloc_check( );
#endif
#ifdef TWO_HEAPS
  void *p= good_heap.malloc(n);
  if(!p)
  {
    p= bad_heap.malloc(n);
    if(!p) OSReport("Failed in alloc from both heaps\n");
  }
#else
	assert(	n == OSRoundUp32B( n ) );
	void* p = OSAlloc( n );
#endif
#ifdef OS_ALLOC_CHECK
	os_alloc_check( );
#endif

#ifdef GC_MEM_TRACK
  mal_malloc(p, n);
#endif

	if (p) memset(p,0,n);

	return p;
}

void os_free( void* p )
{

	if( p ) {
#ifdef OS_ALLOC_CHECK
		os_alloc_check( );
#endif

#ifdef TWO_HEAPS
    if( !bad_heap.free(p) )
      good_heap.free(p);
#else
	  OSFree( p );
#endif

#ifdef OS_ALLOC_CHECK
		os_alloc_check( );
#endif

#ifdef GC_MEM_TRACK
  mal_free(p);
#endif
	}

}

// 32 byte aligned malloc interface
void* os_malloc32( int n )
{
	return os_malloc( n );
}

void os_free32( void* p )
{
	os_free( p );
}

#ifdef GCOLDHEAP
void* operator new( size_t n )
{
	return os_malloc( n );
}

void* operator new[]( size_t n )
{
	void *rv=os_malloc( n );
	return rv;
	//return os_malloc( n );
}

void operator delete( void* p )
{
	os_free( p );
}

void operator delete[]( void* p )
{
	os_free( p );
}
#endif


#ifdef TWO_HEAPS

static bool     mal_stash_allocated= false;

void mal_release_heap()
{
  long heap_size;
  stash::release_stash_bufferspace();
  //good_heap.set_close(); //let close it forever

  heap_size= bad_heap.reset();
  //bad_heap.set_current(); //all levels(+ngl) alloc should be here
  mal_stash_allocated= false;
}

void mal_alloc_stash(int size)
{
  if(!size)
    size= BIG_ASS_BUFFER_SIZE;
  else
    size+= HERO_MAX_STASH_SIZE;

  if( !mal_stash_allocated )
  {
    mal_stash_allocated= true;
    stash::acquire_stash_bufferspace( size );
  }
}

#endif

///////////////////////////////////////////////////////
// mem track
///////////////////////////////////////////////////////
#ifdef GC_MEM_TRACK

#define HEAP_USAGE_LOG_NAME   "heap_usg.log"
#define MAL_MAX               80000
//#define MAL_EMPTY_MAX          6000

//1024*10*(sizeof(mal_elm))
struct  mal_elm
{
  void  *adr;
  u32   size;
  u32   owner;
};

static mal_elm  mal_array[MAL_MAX];
static s32      mal_max_num= 0;

static u16      mal_empty_array[MAL_MAX];
static s32      mal_empty_num;

static s32      mal_num= 0;
static s32      mal_bytes= 0;
static s32      mal_bytes_max= 0;
static char     mal_last_level[256];

//static char     *mal_good_heap_max= NULL;
///////////////////////////////////////////////////////
void mal_malloc(void *_adr, int _size)
{
  s32 i;

  if(mal_empty_num)
  {
    mal_empty_num--;
    i= mal_empty_array[mal_empty_num];
    mal_empty_array[mal_empty_num]= 0;
  }
  else
  {
    if(mal_max_num > MAL_MAX)
    {
      debug_print("Mem Track error: too much mallocs\n");
      return;
    }
    i= mal_max_num;
    mal_max_num++;
  }

  mal_array[ i ].adr= _adr;
  mal_array[ i ].size= _size;
  mal_array[ i ].owner= 0;

  mal_num++;
  mal_bytes+= _size;

  if(mal_bytes_max<mal_bytes)
    mal_bytes_max= mal_bytes;

}


///////////////////////////////////////////////////////
void mal_free(void *_adr)
{
  s32 i;
  //for(; mal_max_num>0 && !mal_array[mal_max_num-1].adr; mal_max_num--) ;

  for(i= mal_max_num-1; i>=0 && mal_array[i].adr != _adr; i--);
  if( i<0 )
  {
    debug_print("Mem Track error: failed in free %x \n", _adr);
    return;
  }

  mal_num--;
  mal_bytes-= mal_array[i].size;

  if( mal_empty_num > MAL_MAX )
  {
    debug_print("Mem Track error: too much frees\n", _adr);
    return;
  }

  mal_empty_array[mal_empty_num++]= i;

  mal_array[i].adr= 0;
  mal_array[i].size= 0;
  mal_array[i].owner= 0;
}
///////////////////////////////////////////////////////
void mal_heap_usage_log()
{
  extern char g_scene_name[256];
  char      buf[256];
  char      *fl;
  s32       n;
  host_system_file_handle log;

  //OSDumpHeap(mal_heap);

  if( mal_last_level[0] == 0 )
  {

    log= host_fopen( HEAP_USAGE_LOG_NAME, (host_fopen_flags_t)HOST_READ );
    if(log==INVALID_HANDLE_VALUE)
      log= host_fopen( HEAP_USAGE_LOG_NAME, (host_fopen_flags_t)HOST_WRITE );
    host_fclose(log);

    strcpy(mal_last_level, g_scene_name);
    //log.close();
    return;
  }

  log= host_fopen( HEAP_USAGE_LOG_NAME, (host_fopen_flags_t)HOST_READ );

  if(log==INVALID_HANDLE_VALUE)
    return;

  n= host_get_size( log );
	assert(	n == OSRoundUp32B( n ) );
  fl= (char*)OSAlloc( n + 256);

  host_read( log, fl, n );

  fl[n]= 0;

  sprintf(buf, "---------------------------------------\nallocated max bytes:%9i   %s\n", mal_bytes_max, mal_last_level);

  strcat(fl, buf);

  host_fclose(log);

  log= host_fopen( HEAP_USAGE_LOG_NAME, (host_fopen_flags_t)HOST_WRITE );

  if(log==INVALID_HANDLE_VALUE)
  {
    OSFree(fl);
    return;
  }

  n= host_write( log, fl, strlen(fl) );
  OSFree(fl);

  host_fclose(log);


  mal_bytes_max= mal_bytes;
  strcpy(mal_last_level, g_scene_name);;
}
///////////////////////////////////////////////////////
#endif //GC_MEM_TRACK


