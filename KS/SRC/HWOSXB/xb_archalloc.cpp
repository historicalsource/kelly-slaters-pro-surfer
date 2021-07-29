// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

// XBOX specific archalloc stuff

#define XBOX_MEM_START  (( const char * ) 0x80000000)
#define XBOX_MEM_SIZE   ( 0x04000000 )
static const char *DEBUG_HEAPADDR = ( const char * ) XBOX_MEM_START + XBOX_MEM_SIZE;
#define MEM_ALLOC_GRANULARITY 16
#define MEM_ALIGN_GRANULARITY 16
#define SAFETY_VALVE_SIZE 34*1024*1024	// argh ngl and microsoft allocate tons of mem w/o going through us


#define osmem_alloc(s) XPhysicalAlloc(s, MAXULONG_PTR, 16, PAGE_READWRITE)
#define osmem_free(p)  XPhysicalFree(p)



////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// First we'll do xbox-specific memory allocation stuff
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//  unlike dmalloc, which links in as your malloc, we have to redefine NEW and delete to use
//  our malloc
////////////////////////////////////////////////////////////////////////////////////////////////////


void *operator new(size_t size, unsigned int flags, const char *file, int line )
{
  return mem_malloc( size, file, line );
}


void *operator new[](size_t size, unsigned int flags, const char *file, int line )
{
  return mem_malloc(size, file, line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  unlike dmalloc, which links in as your malloc, we have to redefine NEW and delete to use
//  our malloc
////////////////////////////////////////////////////////////////////////////////////////////////////
void *operator new( size_t size )
{
	CHECK_MALLOC_LOCK;
  return mem_malloc( size, "unknown", 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void *operator new[](size_t size)
{
	CHECK_MALLOC_LOCK;
  return mem_malloc(size, "unknown", 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void operator delete(void *p)
{
	CHECK_MALLOC_LOCK;
  mem_free( p );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void operator delete[](void *p)
{
	CHECK_MALLOC_LOCK;
  mem_free( p );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_malloc( size_t s )
{
	CHECK_MALLOC_LOCK;
  return mem_malloc( s, "arch_malloc", 0 );
}

void* arch_malloc( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  return mem_malloc( s, desc, line );
}

void* arch_mallochigh( size_t s )
{
	CHECK_MALLOC_LOCK;
  return mem_malloc( s, "arch_malloc", 0, mafHigh );
}

void* arch_mallochigh( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  return mem_malloc( s, desc, line, mafHigh );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_trymalloc( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  return mem_trymalloc( s, desc, line );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_memalign( size_t boundary, size_t s, const char *file, int line )
{
	CHECK_MALLOC_LOCK;
  return mem_memalign( boundary?boundary:1, s, file, line );
}

void* arch_memalignhigh( size_t boundary, size_t s, const char *file, int line )
{
	CHECK_MALLOC_LOCK;
  return mem_memalign( boundary?boundary:1, s, file, line, mafHigh );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void arch_free( void* p )
{
	CHECK_MALLOC_LOCK;
  mem_free( p );
}

static size_t total_mem_allocated = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
void* mem_malloc( size_t size, const char *file, int line, int flags )
{

	CHECK_MALLOC_LOCK;

  total_mem_allocated += size;

  return mem_memalign(16, size, file, line, flags);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* mem_trymalloc( size_t size, const char *file, int line )
{
	CHECK_MALLOC_LOCK;
  return mem_malloc( size, file, line );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void* mem_memalign( size_t boundary, size_t size, const char *file, int line, int flags )
{

  	#ifdef USENEWHEAPFORSTATIC
	mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	#if 0 //def EVAN
		if ( size>1024*1024 )
		{
			nglPrintf("%s:%d  memalign(%u,%u)\n",descript,line,boundary,size);
		}
	#endif
	assert(boundary);
	size_t a=boundary;
	size_t s=size;
  GETSEMA(AllocMemorySema);
	#ifdef USENEWHEAP
	//CHECKTHEHEAP;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( s, a, flags ) : memalign( a,s );
	#else
  void* alloc = currentheap->Allocate( s, a, flags, file, line );
	#endif
	CHECKTHEHEAP;
	//currentheap->DumpHeap();
	#else
	s+=MEMPADSIZE;
  void* alloc = memalign( a,s );
	#endif
  RELEASESEMA(AllocMemorySema);
  if( alloc==NULL )
  {
    mem_error( size, true, file, line );
  }
  //tag_allocation( size, alloc, false, line, descript );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif

static void *breakonaddress = (void *) 0x800617D0;
static int breakonsize = 176;
if (alloc == breakonaddress && size == breakonsize) 
{
	__asm { int 3 };
}

  return (void*)(((char *)alloc));

  /*
  void *p=_aligned_malloc(size, boundary);
#ifdef ZEROALLOCATEDMEM
	if (p) memset(p,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (p) memset(p,0xCD,size);
#endif
	return p;
  */
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void mem_free( void* alloc )
{
  	#ifdef USENEWHEAPFORSTATIC
	mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	if ( alloc==NULL ) return;
  GETSEMA(AllocMemorySema);
	#ifdef USENEWHEAP
	//CHECKTHEHEAP;
	bool found=false;
	for ( int i=NUMBER_OF_HEAPS-1; i>=0; i-- )
	{
		if ( heaps[i].IsThisYours(alloc) )
		{
			found=true;
  		heaps[i].Deallocate(alloc);
			break;
		}
	}
	if ( !found )
	{
    RELEASESEMA(AllocMemorySema);
		return;
		#ifdef USENEWHEAPFORSTATIC
			assert(0);
		#endif
		free(alloc);
	}
	CHECKTHEHEAP;
	#else
  free ((void*)(((char *)alloc)));
	#endif
  RELEASESEMA(AllocMemorySema);
}

