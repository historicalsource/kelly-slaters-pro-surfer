#ifndef ARCHALLOC_H
#define ARCHALLOC_H
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  archalloc.h

  memory allocation tracking code

  it's redundant on the PC but the PC is a good test bed for it
  it's not redundant on PS2 or Gamecube

  In bootable builds the only purpose this code serves is to check for malloc returning NULL, but
  a valuable purpose that is.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

// JIV DEBUG
#if defined(TARGET_XBOX)
#include "global.h"
#endif /* TARGET_XBOX JIV DEBUG */

#ifdef TARGET_GC
#include "osalloc.h"
#endif

#include "staticmem.h"

#include "ksheaps.h"   // for heap ids

#if (defined TARGET_PC) || (defined TARGET_XBOX) || (defined TARGET_GC)
extern "C" void* memalign( size_t alignment, size_t bytes );
#endif


// Heap manglement calls

	// create a subheap in the current heap
void mem_create_heap( int heapid, int size );

	// create a subheap in upper end of the current heap
void mem_create_heap_high( int heapid, int size );

	// create a subheap in the specified parent heap
void mem_create_subheap( int parentheapid, int heapid, int size );
	// destroy a heap
void mem_destroy_heap( int heapid );
	// heap merging - may not be supported yet
void mem_merge_heaps( int srcheapid, int trgheapid );
	// select the heap for allocations to use
void mem_set_current_heap( int heapid );
// get the heap allocations are using
KSHeapIDs mem_get_current_heap();

	// push the current heap onto a stack and switch to a new heap
void mem_push_current_heap( int heapid );
	// pop a previously pushed heap
void mem_pop_current_heap( void );



// Debug Output of memory allocated
//void mem_debug_out_alloced( char *s );
int  mem_get_total_blocks();
int  mem_get_total_alloced(int heapid = SYSTEM_HEAP);
int mem_get_total_avail(int heapid = SYSTEM_HEAP);
int mem_get_largest_avail(int heapid = SYSTEM_HEAP);
int mem_get_total_mem(int heapindex = SYSTEM_HEAP);
void mem_check_heap_init( void );

// flags for leak printer
enum
{
  MEM_DEBUG_OUT=1,
  MEM_FILE_OUT=2,
  MEM_HALT=4
};

void mem_summarize_heap(int heapid=SYSTEM_HEAP);
void mem_summarize_all_heaps( void );

void mem_dump_heap(int heapid=SYSTEM_HEAP);

void mem_check_heap();
void mem_check_stack_collisions();

bool mem_check_leaks_between_checkpoints(int checkpoint1, int checkpoint2, unsigned flags = MEM_DEBUG_OUT | MEM_FILE_OUT); // Checks that all memory between two checkpoints is freed

int  mem_init_checkpoint( bool onOff );                        // returns checkpoint id
int  mem_set_checkpoint();                        // returns checkpoint id
void mem_leak_prep(void);
void mem_leak_test( bool strict=false );
void mem_check_leaks_since_checkpoint( int checkpoint, unsigned flags = MEM_DEBUG_OUT );
void mem_check_leaks( unsigned flags = MEM_DEBUG_OUT );  // will build memory_log file if MEM_FILE_OUT is on



void mem_lock_malloc( bool onOff );
bool mem_malloc_locked( void );
void mem_lock_heap( int heapindex, bool onOff );
bool mem_heap_locked( int heapindex );

// NOTE:  checking "leaks" is just printing a list of allocated memory;  it can be used
// as a snapshot to find your memory bottlenecks

#if defined(TARGET_XBOX)
	#define memalign(a,s) arch_memalign(a,s,__FILE__, __LINE__ )
	#define malloc(s)     arch_malloc(s,__FILE__, __LINE__ )
	#define NEW           new(0,__FILE__,__LINE__)
	#define free(p)       arch_free(p)
#elif defined(TARGET_GC)
	#ifndef GCOLDHEAP
		#define NEW           new(0,__FILE__,__LINE__)
	#else
		#define NEW           new
	#endif
	#define memalign(a,s) arch_memalign(a,s,__FILE__, __LINE__ )
	#define malloc(s)     arch_malloc(s,__FILE__, __LINE__ )
	#define free(p)       arch_free(p)
#else
	#if 1 //defined(BUILD_DEBUG)
		#define memalign(a,s) arch_memalign(a,s,__PRETTY_FUNCTION__,0)
		#define malloc(s)     arch_malloc(s,__PRETTY_FUNCTION__,0)
		#define NEW           new(0,__PRETTY_FUNCTION__,0)
	#else
		#define memalign      arch_memalign
		#define malloc        arch_malloc
		#define NEW           new
	#endif
	#define free          arch_free
#endif /* TARGET_XBOX JIV FIXME */

void* arch_malloc( size_t size );
void* arch_malloc( size_t size, const char *desc, int line=0 );
void* arch_mallochigh( size_t size );
void* arch_mallochigh( size_t size, const char *desc, int line=0 );
void* arch_trymalloc( size_t size, const char *desc=NULL, int line=0  );
void* arch_memalign( size_t boundary, size_t size, const char *desc=NULL, int line=0  );
void* arch_memalignhigh( size_t boundary, size_t size, const char *desc=NULL, int line=0  );
void  arch_free( void* block );

#ifndef GCOLDHEAP
void *operator new(size_t size);
void *operator new[](size_t size);
void *operator new(size_t size, unsigned int flags, const char *descript, int line );
void *operator new[](size_t size, unsigned int flags, const char *descript, int line );
void operator delete( void* block );
void operator delete[]( void* block );
#endif

int CheckMemorySizeMB(void);

#ifdef TARGET_PS2
void PreallocBallocMem(void);
void FreeBallocMem(void);
void CleanupStlSmallAllocList(void);
#endif

#endif // ARCHALLOC_H
