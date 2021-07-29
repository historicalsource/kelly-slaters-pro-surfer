
// PS2 specific archalloc stuff

#define PS2_MEM_START  (( const char * ) 0x00100000)
#define PS2_MEM_SIZE   ( 0x02000000 )
//#define PS2_STACK_SIZE ( 0x201584 )
extern unsigned long _stack_size;	// defined in app.cmd; have to refer to it in a weird way (dc 06/29/02)

// Must be a #define, not a variable, otherwise it won't get initialized before its first use.  (dc 06/29/02)
#define DEBUG_HEAPADDR (( const char * ) PS2_MEM_START + ( PS2_MEM_SIZE - (int) &_stack_size))
#define MEM_ALLOC_GRANULARITY 16
#define MEM_ALIGN_GRANULARITY 16
#define SAFETY_VALVE_SIZE 0

#define osmem_alloc(s) malloc(s)
#define osmem_free(p)  free(p)

////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Now the non-xbox versions of all the memory allocation stuff
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
//  unlike dmalloc, which links in as your malloc, we have to redefine NEW and delete to use
//  our malloc
////////////////////////////////////////////////////////////////////////////////////////////////////
void *operator new(size_t size, unsigned int flags, const char *descript, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  void* alloc = mem_malloc( size, descript, line );
  return alloc;
}


void *operator new[](size_t size, unsigned int flags, const char *descript, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  void* alloc = mem_malloc( size, descript, line );
  if( alloc==NULL )
  {
    mem_error( size, true, descript, line );
    //error("MALLOC FAILED!");
  }
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//  unlike dmalloc, which links in as your malloc, we have to redefine NEW and delete to use
//  our malloc
////////////////////////////////////////////////////////////////////////////////////////////////////
void *operator new( size_t size )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  void* alloc = mem_malloc( size, "operator new", 0 );
  return alloc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void *operator new[](size_t size)
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  void* alloc = mem_malloc( size, "operator new[]", 0 );
  if( alloc==NULL )
  {
    mem_error( size, true, "operator new[]", 0 );
    //error("MALLOC FAILED!");
  }
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void operator delete(void *p)
{
	CHECK_MALLOC_LOCK;
	if ( p )
  	mem_free( p );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void operator delete[](void *p)
{
	CHECK_MALLOC_LOCK;
  //if( g_debug_mem_track )
  //  free_tag( p, true );
	mem_free(p);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_malloc( size_t s )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_malloc( s, "arch_malloc", 0 );
}

void* arch_malloc( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_malloc( s, desc?desc:"arch_malloc", line );
}

void* arch_mallochigh( size_t s )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_malloc( s, "arch_malloc", 0, mafHigh );
}

void* arch_mallochigh( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_malloc( s, desc?desc:"arch_malloc", line, mafHigh );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_trymalloc( size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_trymalloc( s, desc?desc:"arch_trymalloc", line );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* arch_memalign( size_t boundary, size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_memalign( boundary, s, desc?desc:"arch_memalign",line );
}

void* arch_memalignhigh( size_t boundary, size_t s, const char *desc, int line )
{
	CHECK_MALLOC_LOCK;
  //CheckMemorySizeMB();
  return mem_memalign( boundary, s, desc?desc:"arch_memalign",line,mafHigh );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void arch_free( void* p )
{
	CHECK_MALLOC_LOCK;
  mem_free( p );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void mem_check_heap_init( void );


#define DEFAULTPS2ALIGN 16


void* mem_malloc( size_t size, const char *descript, int line, int flags )
{
	#ifdef USENEWHEAPFORSTATIC
		mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	static unsigned long int counter=0;
	counter++;
	#ifdef EVAN
		if ( size>1024*1024 )
		{
			nglPrintf("%s:%d  malloc(%u)\n",descript,line,size);
		}
	#endif

	//int flags=0;

  GETSEMA(AllocMemorySema);
	CHECKTHEHEAP1;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( size, DEFAULTPS2ALIGN, flags, descript, line  ) : malloc( size );
	#else
  void* alloc = currentheap->Allocate( size, DEFAULTPS2ALIGN, flags, descript, line );
	#endif
	CHECKTHEHEAP2;
  RELEASESEMA(AllocMemorySema);

  if( alloc==NULL )
  {
    mem_error( size, true, descript, line );
    //error("MALLOC FAILED!");
  }
  //tag_allocation( size, alloc, false, line, descript );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* mem_trymalloc( size_t size, const char *descript, int line )
{
	#ifdef USENEWHEAPFORSTATIC
	mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	static unsigned long int counter=0;
	counter++;
  GETSEMA(AllocMemorySema);
	#ifdef USENEWHEAP
	CHECKTHEHEAP1;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( size ) : malloc( size );
	#else
  void* alloc = currentheap->Allocate( size, DEFAULTPS2ALIGN, 0, descript, line );
	#endif
	CHECKTHEHEAP2;
	#else
  void* alloc = malloc( size+MEMPADSIZE );
	#endif
  RELEASESEMA(AllocMemorySema);
  if( alloc==NULL )
  {
    mem_error( size, false, descript, line );
    //error("MALLOC FAILED!");
  }
	//else
	//	tag_allocation( size, alloc, false, line, descript );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef memalign
#error goddamit!
#endif

extern _PTR memalign _PARAMS ((size_t, size_t));


void* mem_memalign( size_t boundary, size_t size, const char *descript, int line, int flags )
{
	#ifdef USENEWHEAPFORSTATIC
	mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	#ifdef EVAN
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
	CHECKTHEHEAP1;
//	int flags=0;
	#ifndef USENEWHEAPFORSTATIC
  void* alloc = currentheap->HasMemory() ? currentheap->Allocate( s, a, flags, descript, line ) : memalign( a,s );
	#else
  void* alloc = currentheap->Allocate( s, a, flags, descript, line );
	#endif
	CHECKTHEHEAP2;
	//currentheap->DumpHeap();
	#else
	s+=MEMPADSIZE;
  void* alloc = memalign( a,s );
	#endif
  RELEASESEMA(AllocMemorySema);
  if( alloc==NULL )
  {
    mem_error( size, false, descript, line );
  }
  //tag_allocation( size, alloc, false, line, descript );
#ifdef ZEROALLOCATEDMEM
	memset(alloc,0,size);
#endif
#ifdef TRASHALLOCATEDMEM
	if (alloc) memset(alloc,0xCD,size);
#endif
  return (void*)(((char *)alloc)); //+MEMPREPADSIZE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void mem_free( void* alloc )
{
	#ifdef USENEWHEAPFORSTATIC
	mem_check_heap_init();
	#endif
	CHECK_MALLOC_LOCK;
	if ( alloc==NULL ) return;

  //if( g_debug_mem_track )
  //  free_tag( alloc, false );

	#ifdef USENEWHEAP
		// find the heap this block was allocated from
	bool found=false;
	CHECKTHEHEAP1;
	for ( int i=NUMBER_OF_HEAPS-1; i>=0; i-- )
	{
		if ( heaps[i].IsThisYours(alloc) )
		//if ( heaps[i].DoYouContain(alloc) )
		{
			found=true;
      GETSEMA(AllocMemorySema);
  		heaps[i].Deallocate(alloc);
      RELEASESEMA(AllocMemorySema);
			break;
		}
	}
	if ( !found )
	{
		#ifdef USENEWHEAPFORSTATIC
			assert(0);
		#endif
		free(alloc);
	}
	CHECKTHEHEAP2;
	#else
  free ((void*)(((char *)alloc)-MEMPREPADSIZE));
	#endif

  //free( alloc );
}


/*	The gcc libraries allocate some memory through calls like sprintf, and this memory is re-used
	without ever being freed.  If left unchecked, these allocations fragment the heap.  Whenever
	we clean up, we should call this function to remove the fragments. (dc 02/05/02)
*/
#ifdef TARGET_PS2
// fprintf, sprintf, etc. all you _dtoa_r to convert from double to ascii
// dtoa uses bigints which are allocated but never freed and are instead
// added to a freelist in the _REENT structure
// There are 15 different BigInt sizes (the second argument of _Balloc)
// We preallocate enough bigints here so they are never allocated in the
// middle of the heap -mjd
extern "C" void *_Balloc( void *, int k );
extern "C" void _Bfree( void *, void *);
extern "C" void *__smakebuf( FILE * );

#define _PreallocKMax 4
#define _PreallocFreelistMax 5
const u_int _PreallocStdoutBufSize = 0x400;	// value that the library uses

void PreallocBallocMem(void)
{
	void *temp_ptr[8];
	int i;
	
	for( i = 0; i < _PreallocKMax; i++ )
	{
		int j;
		for( j = 0; j < _PreallocFreelistMax; j++ )
		{
			temp_ptr[j] = _Balloc( _REENT, i );
		}
		for( j = 0; j < _PreallocFreelistMax; j++ )
		{
			_Bfree( _REENT, temp_ptr[j] );
		}
	}
	
	// Also reserve some memory for the printf buffer
	__smakebuf(_impure_ptr->_stdout);
}

// Allocate memory for balloc from a static buffer.  In this case, don't call FreeBallocMem.
void UseStaticBallocMem(void)
{
	static char Balloc_buffer[8096];
	static char *buf = Balloc_buffer;
	_impure_ptr->_freelist = (_Bigint **) buf;
	buf += _PreallocKMax * sizeof(*(_impure_ptr->_freelist));
	for (u_int k = 0; k < _PreallocKMax; ++k)
	{
		int x = 1 << k;
		_Bigint *rv = _impure_ptr->_freelist[k] = (_Bigint *) buf;
		buf += sizeof(_Bigint) + (x-1) * sizeof(unsigned long);
		rv->_k = k;
		rv->_maxwds = x;
		rv->_sign = rv->_wds = 0;

		for (u_int i = 0; i < _PreallocFreelistMax; ++i)
		{
			rv->_next = (_Bigint *) buf;
			rv = rv->_next;
			buf += sizeof(_Bigint) + (x-1) * sizeof(unsigned long);
			rv->_k = k;
			rv->_maxwds = x;
			rv->_sign = rv->_wds = 0;
		}

		rv->_next = NULL;
	}

	// Also reserve some memory for the printf buffer
	_impure_ptr->_stdout->_bf._size = _PreallocStdoutBufSize;
	_impure_ptr->_stdout->_bf._base = (unsigned char *) buf;
	buf += _PreallocStdoutBufSize;

	assert(buf <= Balloc_buffer + sizeof(Balloc_buffer));
}

// Allocate memory for balloc using our own malloc function.
void ArchallocBallocMem(void)
{
	_impure_ptr->_freelist =  (_Bigint **) mem_malloc(
			_PreallocKMax * sizeof(*(_impure_ptr->_freelist)),
			__FUNCTION__, __LINE__);
	for (u_int k = 0; k < _PreallocKMax; ++k)
	{
		int x = 1 << k;
		_Bigint *rv = _impure_ptr->_freelist[k] = (_Bigint *) mem_malloc(
			sizeof(_Bigint) + (x-1) * sizeof(unsigned long),
			__FUNCTION__, __LINE__);
		rv->_k = k;
		rv->_maxwds = x;
		rv->_sign = rv->_wds = 0;

		for (u_int i = 0; i < _PreallocFreelistMax; ++i)
		{
			rv->_next = (_Bigint *) mem_malloc(
				sizeof(_Bigint) + (x-1) * sizeof(unsigned long),
				__FUNCTION__, __LINE__);
			rv = rv->_next;
			rv->_k = k;
			rv->_maxwds = x;
			rv->_sign = rv->_wds = 0;
		}

		rv->_next = NULL;
	}

	// Also reserve some memory for the printf buffer
	_impure_ptr->_stdout->_bf._size = _PreallocStdoutBufSize;
	_impure_ptr->_stdout->_bf._base = (unsigned char *) mem_malloc(
			_PreallocStdoutBufSize,
			__FUNCTION__, __LINE__);
}

#define _Kmax 15	// matches definition in mprec.c
void FreeBallocMem(void)
{
	_Bigint **freelist = _impure_ptr->_freelist;	// defined in reent.h
	if (freelist)
	{
		for (u_int k = 0; k < _Kmax; ++k)
		{
			_Bigint *rv;
			while ((rv = freelist[k]) != NULL)
			{
				freelist[k] = rv->_next;	// will be NULL last time through
				free(rv);
			}
		}
		free(freelist);
		_impure_ptr->_freelist = NULL;
	}
}

void *StlSmallAlloc(size_t bytes)
{
#define STL_SMALL_ALLOC_SIZE (100 * 1024)
	static char StlSmallAllocBuf[STL_SMALL_ALLOC_SIZE];
	static char *StlSmallAllocPtr = StlSmallAllocBuf;

	void *retval = StlSmallAllocPtr;
	StlSmallAllocPtr += bytes;
	assertmsg (StlSmallAllocPtr - StlSmallAllocBuf <= STL_SMALL_ALLOC_SIZE,
		"Need to increase STL_SMALL_ALLOC_SIZE in StlSmallAlloc().\n");
	return retval;
}

/*
void CleanupStlSmallAllocList(void)
{
	memset(my_alloc::_S_free_list, 0, sizeof(my_alloc::_S_free_list));
	while (my_alloc::_S_small_alloc_index > 0)
	{
		--my_alloc::_S_small_alloc_index;
		mem_free(my_alloc::_S_small_alloc_list[my_alloc::_S_small_alloc_index]);
	}
}
*/


#endif
