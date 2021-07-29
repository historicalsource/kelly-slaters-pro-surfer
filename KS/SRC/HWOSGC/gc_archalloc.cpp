
// GC specific archalloc stuff

#define ZEROALLOCATEDMEM

#define GC_MEM_START  (( const char * ) 0x80000000)
#define GC_MEM_SIZE   ( 0x01800000 )
static const char *DEBUG_HEAPADDR = ( const char * ) GC_MEM_START + GC_MEM_SIZE;
#define MEM_ALLOC_GRANULARITY 32
#define MEM_ALIGN_GRANULARITY 32
#define SAFETY_VALVE_SIZE 256*1024

#define osmem_alloc(s) os_malloc(s)
#define osmem_free(p)  os_free(p)

void  mem_free( void* block );
void os_alloc_init( void );
extern int gc_heapsize;

////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// First we'll do xbox-specific memory allocation stuff
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//  unlike dmalloc, which links in as your malloc, we have to redefine NEW and delete to use
//  our malloc
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USENEWHEAP  // otherwise these are in gc_alloc.cpp

void *operator new(size_t size, unsigned int flags, const char *file, int line )
{
  return mem_malloc( size, file, line );
	//return operator new(size);
}


void *operator new[](size_t size, unsigned int flags, const char *file, int line )
{
  return mem_malloc(size, file, line);
	//return operator new(size);
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

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_malloc( size_t s )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_malloc( s, "arch_malloc", 0 );
	#else
		return operator new(s);
	#endif
}

void* arch_malloc( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_malloc( s, desc, line );
	#else
		return operator new(s);
	#endif
}

void* arch_mallochigh( size_t s )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_malloc( s, "arch_malloc", 0, mafHigh );
	#else
		return operator new(s);
	#endif
}

void* arch_mallochigh( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_malloc( s, desc, line, mafHigh );
	#else
		return operator new(s);
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_trymalloc( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_trymalloc( s, desc, line );
	#else
		return operator new(s);
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_memalign( size_t boundary, size_t s, const char *file, int line )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_memalign( boundary?boundary:1, s, file, line );
	#else
		return operator new(s);
	#endif
}

void* arch_memalignhigh( size_t boundary, size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	return mem_memalign( boundary?boundary:1, s, desc, line, mafHigh );
	#else
		return operator new(s);
	#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void arch_free( void* p )
{
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAP
  	mem_free( p );
	#else
		::operator delete(p);
	#endif
}

static size_t total_mem_allocated = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
void* mem_malloc( size_t size, const char *file, int line, int flags )
{

#ifdef USENEWHEAP  // otherwise these are in gc_alloc.cpp
	#ifdef USENEWHEAPFORSTATIC
		mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAPFORSTATIC
		mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	static unsigned long int counter=0;
	counter++;
	#ifdef EVAN
		if ( size>1024*1024 )
		{
			nglPrintf("%s:%d  malloc(%u)\n",file,line,size);
		}
	#endif

	//int flags=0;

	CHECKTHEHEAP1;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( size, MEM_ALIGN_GRANULARITY, flags, file, line  ) : malloc( size );
	#else
  void* alloc = currentheap->Allocate( size, MEM_ALIGN_GRANULARITY, flags, file, line );
	#endif
	CHECKTHEHEAP2;

  if( alloc==NULL )
  {
    mem_error( size, true, file, line );
  }
  //tag_allocation( size, alloc, false, line, file );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
#else
	return operator new(size);
#endif

	#if 0
  total_mem_allocated += size;

  // This is a huge hack.  It wastes 20 bytes of space for every dynamic
  // memory allocation.  Something definitely needs to be done about this.
#pragma todo("make memory allocation not waste 20 bytes for every allocation")
  return mem_memalign(16, size, file, line, flags);

  /*
  total_mem_allocated += size;

  size_t *start_address;
  size_t *retval = start_address = (size_t *) malloc( size + sizeof(size_t) * 2);
  assert( retval );

  *retval = size;
  retval++;
  *retval = (size_t)start_address;      // alignment
  retval++;

//  nglPrintf("MEM: Allocated at %x: %d bytes, extra space = %d, alignment = 1\n", start_address, size, sizeof(size_t) * 2);
//  nglPrintf("MEM:   returned = %x\n", retval);

  return (void *) retval;
*/
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* mem_trymalloc( size_t size, const char *file, int line )
{
#ifdef USENEWHEAP  // otherwise these are in gc_alloc.cpp
	#ifdef USENEWHEAPFORSTATIC
		mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	#ifdef USENEWHEAPFORSTATIC
		mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	static unsigned long int counter=0;
	counter++;
	#ifdef EVAN
		if ( size>1024*1024 )
		{
			nglPrintf("%s:%d  malloc(%u)\n",file,line,size);
		}
	#endif

	int flags=0;
	CHECKTHEHEAP1;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( size ) : malloc( size );
	#else
  void* alloc = currentheap->Allocate( size, MEM_ALIGN_GRANULARITY, flags, file, line );
	#endif
	CHECKTHEHEAP2;

  if( alloc==NULL )
  {
    mem_error( size, false, file, line );
  }
  //tag_allocation( size, alloc, false, line, file );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
#else
	return operator new(size);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void* mem_memalign( size_t boundary, size_t size, const char *file, int line, int flags )
{
#ifdef USENEWHEAP  // otherwise these are in gc_alloc.cpp
	#ifdef USENEWHEAPFORSTATIC
	mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	#ifdef EVAN
		if ( size>1024*1024 )
		{
			nglPrintf("%s:%d  memalign(%u,%u)\n",file,line,boundary,size);
		}
	#endif
	assert(boundary);
	size_t a=boundary;
	size_t s=size;
	#ifdef USENEWHEAP
	CHECKTHEHEAP1;
	//int flags=0;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( s, a ) : memalign( a,s );
	#else
  void* alloc = currentheap->Allocate( s, a, flags, file, line );
	#endif
	CHECKTHEHEAP2;
	//currentheap->DumpHeap();
	#else
	//s+=MEMPADSIZE;
  void* alloc = memalign( a,s );
	#endif
  if( alloc==NULL )
  {
    mem_error( size, true, file, line );
  }
  //tag_allocation( size, alloc, false, line, file );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
#else
	return operator new(size);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void mem_free( void* alloc )
{
  if(!alloc)
    return;

	CHECK_MALLOC_LOCK;
#ifdef USENEWHEAP
	CHECKTHEHEAP1;
		// find the heap this block was allocated from
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
		return;
		#ifdef USENEWHEAPFORSTATIC
			assert(0);
		#endif
		free(alloc);
	}
	CHECKTHEHEAP2;
#else
	::operator delete(alloc);
#endif

}



