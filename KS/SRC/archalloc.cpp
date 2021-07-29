////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  archalloc.cpp

  memory allocation tracking code

  it's redundant on the PC but the PC is a good test bed for it
  it's not redundant on PS2 or Gamecube

  ideas for improvement:
  - put a sentry at the beginning of memory allocations, not just the end
  - rewrite the archengine so it doesn't allocate so damn much crap

  - separate out into its own library and link last;  instead of
    redefining new, delete, malloc, at the precompiler level it would just use our malloc instead
    of the standard.  but then how would we put breakpoints into it?  maybe not a good idea.

  NB: Try not to mess with this file without talking to Jamie or Martin first.
      To enable memory tracking just modify g_debug_mem_track below in the
      appropriate place.

  NB: If the tags are taking up too much space reduce the MAX_STACK_DEPTH
      Setting it to 1 will mean nothing besides the original return address is recorded
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "global.h"
#ifndef TARGET_PS2
#pragma hdrstop
#endif

#include "heap.h"
#include "archalloc.h"
#include "debug.h"
#include "ini_parser.h"	// for os_developer_options
#include "semaphores.h"
#include "osfile.h"
#include "ngl.h"
#include <map>

typedef void *pvoid;
typedef char *pchar;

//-------------------------------------------------------
// These are the heaps - corrupt stuff here and all will be lost

static Heap  *currentheap=NULL;
static Heap  heaps[NUMBER_OF_HEAPS];
static unsigned int heapsize[NUMBER_OF_HEAPS];
static pchar heapmem[NUMBER_OF_HEAPS];
static pvoid heapdummyalloc[NUMBER_OF_HEAPS];

//-------------------------------------------------------
// Somewhat extreme heap checking for detecting heap errors

#ifdef EVAN
	// for when there's reason to believe something outside the memory manager
	//   is causing corruption
//#define EXTREME_HEAP_PARANOIA1
	// for when there's reason to believe something within the memory manager
	//   is causing corruption
//#define EXTREME_HEAP_PARANOIA2
#endif

#ifdef EXTREME_HEAP_PARANOIA1
#define CHECKTHEHEAP1 heaps[SYSTEM_HEAP].CheckConsistency();
#else
#define CHECKTHEHEAP1 ((void)0)
#endif
#ifdef EXTREME_HEAP_PARANOIA2
#define CHECKTHEHEAP2 heaps[SYSTEM_HEAP].CheckConsistency();
#define CHECKTHEHEAP heaps[SYSTEM_HEAP].CheckConsistency();
#else
#define CHECKTHEHEAP2 ((void)0)
#define CHECKTHEHEAP ((void)0)
#endif

//-------------------------------------------------------
// Which heap code are we using?

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#undef MEMTRACK
#endif /* TARGET_XBOX JIV DEBUG */

#ifndef GCOLDHEAP
// defined(TARGET_PS2) || defined(TARGET_XBOX) || defined(GCNEWHEAP)
		// new heap management stuff
	#define USENEWHEAP
		// starts heap even for global ctors called before main()
	#define USENEWHEAPFORSTATIC
	#undef MEMTRACK
#else
	#ifdef MEMTRACK
		#define MEMTAGS
	#endif
#endif


//-------------------------------------------------------
// Need access to the real ones

#undef malloc
#undef free
#undef memalign

////////////////////////////////////////////////////////////////////////////////////////////////////
//  forward declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

void* mem_malloc( size_t n, const char *descript, int line, int flags = 0 );
void* mem_trymalloc( size_t n, const char *descript, int line  );
void* mem_memalign( size_t boundary, size_t size, const char *, int line, int flags = 0 );
void  mem_free( void* block );

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef USENEWHEAP
static int mem_total_avail = 0;
static int mem_total_alloced = 0;
static int mem_water_mark = 0;
#endif

void llc_memory_log();

#include "hwrasterize.h"

void mem_print_status(int heapindex)
{
  nglPrintf( "Total free         %8u\n", mem_get_total_avail(heapindex)    );
  nglPrintf( "Largest free       %8u\n", mem_get_largest_avail(heapindex)  );
  nglPrintf( "Total allocated    %8u\n", mem_get_total_alloced(heapindex)  );
  nglPrintf( "Free + allocated   %8u\n", mem_get_total_avail(heapindex) + mem_get_total_alloced(heapindex)  );
}

// Call these function to assert that no one is allocing or freeing memory
// without us knowing.  (dc 01/11/02)
static int mem_avail_last_time = -1;
void mem_set_status()
{
	mem_avail_last_time = mem_get_total_avail();
}
void mem_check_status()
{
	assert(mem_get_total_avail() == mem_avail_last_time);
}

void mem_error( size_t size, bool fatal, const char *desc, int line )
{
	PointerMath pheap=(PointerMath) heapmem[SYSTEM_HEAP];
	PointerMath pheapend=pheap;
	unsigned int hsize=heapsize[SYSTEM_HEAP];
	pheapend += hsize;

  int heapindex=-1;
  for (int i=0; i < NUMBER_OF_HEAPS; i++)
  {
    if (currentheap == &heaps[i])
      heapindex = i;
  }
  nglPrintf( "Tried to allocate %8u from heap %d\n", size, heapindex);
  mem_print_status(heapindex);
	char terr[512];
  sprintf( terr, "Memory allocation failed %u bytes line %d file %s\nHeap st 0x%8.8X sz 0x%8.8X en 0x%8.8X ", size, line, desc, (unsigned int) pheap, hsize, (unsigned int) pheapend);
	mem_dump_heap(heapindex);
	mem_summarize_all_heaps();
	if ( fatal )
	{
		error(terr);
	}
	else
	{
  	low_level_console_print( "%s\n", terr );
  	low_level_console_flush();
	}



}


//-------------------------------------------------------
// Malloc and heap locking

static bool malloc_locked=false;

void mem_lock_malloc( bool onOff )
{
#if defined(TARGET_PS2)
	// To be added when memory locking is better enforced (dc 01/08/02)
	if (malloc_locked ^ onOff)
		nglPrintf(PRINT_GREEN "Memory locking going %s\n" PRINT_BLACK, onOff ? "on" : "off");

	malloc_locked=onOff;
#endif // !defined(TARGET_XBOX)
}

bool mem_malloc_locked( void )
{
	return malloc_locked;
}

void mem_lock_heap( int heapindex, bool onOff )
{
	heaps[heapindex].SetHeapLock(onOff);
}

bool mem_heap_locked( int heapindex )
{
	return heaps[heapindex].GetHeapLock();
}


#if defined(BUILD_DEBUG)

void check_malloc_lock( void )
{
  if ( malloc_locked )
  {
    int breakpointhere=0;
    breakpointhere=1;
  }
	#ifdef USINGSTATICSTLALLOCATIONS
  assert(!malloc_locked);
	#endif
}

#define CHECK_MALLOC_LOCK check_malloc_lock()

#else

#define CHECK_MALLOC_LOCK  (( void )0)

#endif


//-------------------------------------------------------
// Leak checking

int  mem_init_checkpoint( bool onOff )
{
  //g_debug_mem_track = onOff;      // turn this off for fast loads in debug builds
	return mem_set_checkpoint();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int  mem_set_checkpoint()
{
	#ifdef USENEWHEAP
		return heaps[SYSTEM_HEAP].GetCurrentMemMarker();
	#else
	  return 0;
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void mem_check_leaks( unsigned flags )
{
}

static int lc=0;
void mem_leak_prep(void)
{
	lc=mem_set_checkpoint();
}

void mem_leak_test(bool strict)
{
	mem_check_heap();

	if ( lc>0 )
    mem_check_leaks_since_checkpoint(lc,MEM_DEBUG_OUT|MEM_FILE_OUT|(strict?MEM_HALT:0));
}


static int ttyline=0;
bool dumpleakstofile=false;
bool dumpleakstotty =true ;
static os_file memlog;

void mem_leak_dumper( const char *str )
{
	if ( dumpleakstotty )
	{
		if ( ttyline++>16 )
		{
			ttyline=0;
			//fflush(stdout);
			#ifdef NGL
			nglFlip();
			#endif
			for (int i=0; i<1000000; i++) continue;
		}
		//memlogf("  %8u %20s\n",mti->size,mti->file);
		nglPrintf(str); //debug_print("%s",str);
	}
	if ( dumpleakstofile )
	{
  	memlog.write( (void *) str, strlen( str ) );
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////



void mem_check_leaks_since_checkpoint( int checkpoint, unsigned flags )
{
	#if defined(BUILD_DEBUG) || defined(EVAN) || defined(KEVIN) || defined(USER_MJD)
	  #if defined(USER_JDB)
	    flags |= MEM_FILE_OUT;
	  #endif

		#ifndef BUILD_DEBUG
	    flags &= ~MEM_FILE_OUT;
		#endif

	  // can't use log stuff here cos it allocates on each write call
	  // eck!!!

		dumpleakstofile = (0!=( flags & MEM_FILE_OUT ));
		dumpleakstotty  = (0!=( flags & MEM_DEBUG_OUT ));
	  if( flags & MEM_FILE_OUT )
	  {
	    memlog.open( "memory_log.txt", os_file::FILE_WRITE );
	  }

		int leaks=heaps[SYSTEM_HEAP].DumpAllocSinceMarker( checkpoint, &mem_leak_dumper );

	  if( flags & MEM_FILE_OUT )
	    memlog.close();

	  if( leaks>0 && (flags & MEM_HALT) )
		{
			nglPrintf("Leaks detected stopping\n");
			assert(0);
		}
	#endif
}

#ifndef TARGET_PS2
#pragma warning( disable : 4700 )  // local variable 'hack_variable_to_get_at_stack' used without initialized
#endif

#if defined(TARGET_XBOX)
	#include "hwosxb/xb_archalloc.cpp"
#elif defined(TARGET_GC)
	#include "hwosgc/gc_archalloc.cpp"
#elif defined(TARGET_PS2)
	#include "hwosps2/ps2_archalloc.cpp"
#else
	#error No archalloc for the Atari 1600
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
int mem_get_total_blocks()
{
	#ifdef USENEWHEAP
		return heaps[SYSTEM_HEAP].GetNumBlocks();
	#else
	  return 0;
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int mem_get_total_alloced(int heapindex)
{
	#ifdef USENEWHEAP
		return heaps[heapindex].GetTotalUsed();
	#else
	  return mem_total_alloced;
	#endif
}

int mem_get_total_mem(int heapindex)
{
	#ifdef USENEWHEAP
		return heaps[heapindex].GetTotalSize();
	#else
	  return mem_total_avail;
	#endif
}

int mem_get_total_avail(int heapid )
{
	#ifdef USENEWHEAP
		return heaps[heapid].GetTotalFree();
	#else
	#define MAX_BLOCKS 500
		int total_avail = 0;
		void *blockptr[MAX_BLOCKS] = {0};
		int numblocks;
		int avail;
		for (numblocks = 0; numblocks < MAX_BLOCKS; ++numblocks)
		{
			avail=0;
			for ( int i=31; i>=0; i-- )
			{
				void *cptr=malloc( avail + (1<<i) );
				if ( cptr )
				{
					avail += 1<<i;
					free(cptr);
				}
			}
			if (!avail) break;
			blockptr[numblocks] = malloc(avail);
			total_avail += avail;
		}
		for (int j = 0; j < numblocks; ++j)
		{
			free(blockptr[j]);
		}
		return total_avail;
	#endif
}

typedef void *pvoid;
#ifdef TARGET_GC
int mem_raw_largest_avail(int heapindex=0 )
#else
int mem_raw_largest_avail(int heapindex=SYSTEM_HEAP)
#endif
{
	#ifdef USENEWHEAP
		return heaps[heapindex].GetLargestFree();
	#else
		int avail=0;
		for ( int i=31; i>=0; i-- )
		{
			void *cptr=malloc( avail + (1<<i) );
			if ( cptr )
			{
				avail += 1<<i;
				free(cptr);
			}
		}
		return avail;
	#endif
}

int mem_get_largest_avail(int heapindex)
{
  return mem_raw_largest_avail(heapindex)-128; // 128 for potential header & footer
}




////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
void mem_debug_out_alloced( char *s )
{
  if( g_debug_mem_track )
  {
    int ram = mem_get_total_alloced();
    debug_print( stringx(s)+" CURRENT RAM = "+itos(ram)+"\n" );
    debug_print( "Heap memory high water mark: %d", mem_water_mark );
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void mem_check_heap( void )
{
	heaps[SYSTEM_HEAP].CheckConsistency();
	heaps[DEBUG_HEAP].CheckConsistency();
}

void mem_check_stack_collisions( void )
{
	heaps[SYSTEM_HEAP].CheckStackCollision();
}


bool mem_check_leaks_between_checkpoints(int checkpoint1, int checkpoint2, unsigned flags )
{
	#ifdef BUILD_DEBUG
		#ifdef USENEWHEAP
		  #if defined(USER_JDB)
		    flags |= MEM_FILE_OUT;
		  #endif

		  // can't use log stuff here cos it allocates on each write call
		  // eck!!!

			dumpleakstofile = (0!=( flags & MEM_FILE_OUT ));
			dumpleakstotty  = (0!=( flags & MEM_DEBUG_OUT ));

		  if( flags & MEM_FILE_OUT )
		  {
		    memlog.open( "memory_log.txt", os_file::FILE_WRITE );
		  }

		  int leaks = heaps[SYSTEM_HEAP].DumpAllocBetweenMarkers(checkpoint1, checkpoint2, &mem_leak_dumper);
		  if( flags & MEM_FILE_OUT )
		    memlog.close();
		  return leaks;
		#else
		  return false;
		#endif
	#else
	  return false;
	#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////

//void mem_dump_entry( mem_tags_t_it mti );

void memlogf( char* format, ... )
{
  static char work[1024];
  va_list args;
  va_start( args, format );
  vsprintf( work, format, args );
  va_end( args );
  memlog.write( work, strlen( work ) );
}

void heapdumper( const char *str )
{
	#ifndef TARGET_GC
  memlog.write( (void *) str, strlen( str ) );
	#else
	nglPrintf("%s",str);
	#endif
}

void mem_dump_heap( int heapid )
{
	#if defined(BUILD_DEBUG) || defined(EVAN) || defined(KEVIN) || defined(USER_MJD)
		#if !defined( TARGET_GC ) //&& defined( USER_MJD )
			STOP_PS2_PC;
			bool was_locked = false;
			if(os_file::is_system_locked())
			{
				os_file::system_unlock();
				was_locked = true;
			}
			if ( os_developer_options::is_inst() )
			{
				nglPrintf("ARCHALLOC:\tBeginning memory dump. Game may lock up for a minute or two...\n");
				stringx memdumpfile = os_developer_options::inst()->get_string (os_developer_options::STRING_MEM_DUMP_FILE);
				memlog.open( memdumpfile.c_str(), os_file::FILE_WRITE );
				heaps[heapid].DumpHeap(&heapdumper);
				memlog.close();
				nglPrintf("ARCHALLOC:\t...Completed memory dump.\n");
			}
			if(was_locked) os_file::system_lock();
			START_PS2_PC;
		#else
			nglPrintf("ARCHALLOC:\tBeginning memory dump. Game may lock up for a minute or two...\n");
			heaps[heapid].DumpHeap(&heapdumper);
			nglPrintf("ARCHALLOC:\t...Completed memory dump.\n");
		#endif
	#endif
}

void mem_summarize_heap(int heapid)
{
	#if defined(BUILD_DEBUG) || defined(EVAN)
		#ifndef TARGET_GC
			bool was_locked = false;
			if(os_file::is_system_locked())
			{
				os_file::system_unlock();
				was_locked = true;
			}
			if ( os_developer_options::is_inst() )
			{
				nglPrintf("ARCHALLOC: summary of heap %d\n",heapid);
				stringx memdumpfile = os_developer_options::inst()->get_string (os_developer_options::STRING_MEM_DUMP_FILE);
				memlog.open( memdumpfile.c_str(), os_file::FILE_WRITE );
				heaps[heapid].DumpSummary(&heapdumper);
				memlog.close();
			}
			if(was_locked) os_file::system_lock();
		#else
			nglPrintf("ARCHALLOC: summary of heap %d\n",heapid);
			heaps[heapid].DumpSummary(&heapdumper);
		#endif
	#endif
}

void mem_summarize_all_heaps( void )
{
  for (int i=0; i < NUMBER_OF_HEAPS; i++)
  {
		mem_summarize_heap(i);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


	// This allocates any memory above 32MB on the ps2
void take_the_top_mem( void )
{
}

void free_the_top_mem( void )
{
	free(heapmem[DEBUG_HEAP]);
}



void mem_set_current_heap( int heapid )
{
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);
	currentheap=&heaps[heapid];
}
KSHeapIDs mem_get_current_heap()
{
  for (int i=0; i < NUMBER_OF_HEAPS; i++)
  {
    if (&heaps[i] == currentheap)
      return (KSHeapIDs)i;
  }
  assert(0);
  return (KSHeapIDs)0;
}
typedef Heap *pHeap;

const int maxheapsinstack=8;

int heapstackptr=0;
pHeap heapstack[maxheapsinstack];

void mem_push_current_heap( int heapid )
{
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);
	assert(heapstackptr<maxheapsinstack-1);
	heapstackptr++;
	mem_set_current_heap(heapid);
}

void mem_pop_current_heap( void )
{
	assert(heapstackptr>0);
	heapstackptr--;
	currentheap=heapstack[heapstackptr];
}

//      Heres a little song about this next procedure:
//        he's got HIGH HEAPS, He's got HIGH HEAPS,
//        He's got high apple pie in the sky HEAPS.
//        So anytime you're feelin' low
//        'Stead of lettin' go
//        Just remember that ant.
//        Oops! There goes another rubber tree.


void mem_create_heap_high( int heapid, int size )
{
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);
	mem_destroy_heap(heapid);

	heapmem[heapid]=(char *) mem_malloc(size,"Child Heap", heapid, mafHigh);
	if ( heapmem[heapid] )
	{
		heapsize[heapid]=size;
		heaps[heapid].SetGranularity(MEM_ALLOC_GRANULARITY,MEM_ALIGN_GRANULARITY);
		heaps[heapid].SetHeapID(heapid);
		heaps[heapid].AddHeapMemory( heapmem[heapid], heapsize[heapid] );
		heapdummyalloc[heapid]=heaps[heapid].Allocate(4,1,0,"Dummy allocation",0);
		#ifdef BUILD_DEBUG
			heaps[heapid].CheckConsistency();
		#endif
	}
}

void mem_create_heap( int heapid, int size )
{
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);
	mem_destroy_heap(heapid);

	heapmem[heapid]=(char *) mem_malloc(size,"Child Heap", heapid);
	if ( heapmem[heapid] )
	{
		heapsize[heapid]=size;
		heaps[heapid].SetGranularity(MEM_ALLOC_GRANULARITY,MEM_ALIGN_GRANULARITY);
		heaps[heapid].SetHeapID(heapid);
		heaps[heapid].AddHeapMemory( heapmem[heapid], heapsize[heapid] );
		heapdummyalloc[heapid]=heaps[heapid].Allocate(4,1,0,"Dummy allocation",0);
		#ifdef BUILD_DEBUG
			heaps[heapid].CheckConsistency();
		#endif
	}

}

void mem_destroy_heap( int heapid )
{
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);
  if (heapdummyalloc[heapid])
    heaps[heapid].Deallocate(heapdummyalloc[heapid]);
	if ( heaps[heapid].GetNumUsedBlocks() )
	{
		error("Attempted to kill a heap that still has allocations in it\n");
		assert(0);
	}
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);
	heaps[heapid].FreeHeapMemory();
	if ( heapmem[heapid] )
	{
    mem_free(heapmem[heapid]);
	}
	heapmem[heapid]=NULL;
	heapsize[heapid]=0;
	heapdummyalloc[heapid]=NULL;
}

void mem_create_subheap( int parentheapid, int heapid, int size )
{
	assert(heapid>=0 && heapid<NUMBER_OF_HEAPS);

	mem_push_current_heap(parentheapid);
	mem_create_heap( heapid, size );
	mem_pop_current_heap();
}

void mem_merge_heaps( int srcheapid, int trgheapid )
{
	assert(0); // this is not and probably never will be supported
	assert(srcheapid>=0 && srcheapid<NUMBER_OF_HEAPS);
	assert(trgheapid>=0 && trgheapid<NUMBER_OF_HEAPS);
	heaps[trgheapid].MergeAdjacentHeap(heaps[srcheapid]);
	heaps[srcheapid].FreeHeapMemory();
	heapmem[srcheapid]=NULL;
	heapsize[srcheapid]=0;
	heapdummyalloc[srcheapid]=NULL;
}

static unsigned int mem_tested_largest_avail()
{
	unsigned int avail=0;
	for ( int i=30; i>=0; i-- )
	{
    void *cptr=osmem_alloc( avail + (1<<i) );

		if ( cptr!=NULL )
		{
			avail += (1<<i);
			osmem_free(cptr);
		}
	}
	return avail;
}

static void *mem_return_largest_available_block( unsigned int *size )
{
	*size=mem_tested_largest_avail();
  void *r=osmem_alloc(*size);
	if ( r==NULL )
	{
		assert(0);
		*size=0;
	}
	return r;
}

void KSHeapError( const char* Text );
void KSDebugPrint( const char* Text );
void KSCriticalError( const char* Text );

void mem_check_heap_init( void )
{
	#ifdef USENEWHEAP
	static bool memsysinit=false;
	if ( !memsysinit )
	{
		CreateAllSemaphores();
		#if defined(BUILD_BOOTABLE) //|| defined(TARGET_GC)
		g_debug.simulate32meg=true;
		#endif
		for ( int i=0; i<NUMBER_OF_HEAPS; i++ )
		{
			heaps[i].SetHeapID(i);
			heaps[i].FreeHeapMemory();
			heapsize[i]=0;
			heapmem[i]=NULL;
			heapdummyalloc[i]=NULL;
		}

		#ifdef TARGET_PS2
			CheckMemorySizeMB();
		#endif
		#ifdef TARGET_GC
			os_alloc_init();
		#endif
		Heap::SetHeapMessagers(&KSDebugPrint,&KSHeapError);

		#ifndef USENEWHEAP
  		g_debug_mem_track = (128==CheckMemorySizeMB());	// causes spurious DMA warnings in debugger (dc 08/23/01)
		#endif
		memsysinit=true;

    char *safetyvalve=(char *) osmem_alloc(SAFETY_VALVE_SIZE);
		unsigned int allsize=0;
		char *allmem=(char *) mem_return_largest_available_block( &allsize );
		if ( (allsize> (unsigned int) (DEBUG_HEAPADDR-allmem))
  				&& g_debug.simulate32meg
		   )
		{
			heapsize[SYSTEM_HEAP]=DEBUG_HEAPADDR-allmem;
			osmem_free(allmem);
      heapmem[SYSTEM_HEAP]=(char *) osmem_alloc(heapsize[SYSTEM_HEAP]);
			assert(heapmem[SYSTEM_HEAP]);
			heapmem[DEBUG_HEAP]=(char *) mem_return_largest_available_block( &heapsize[DEBUG_HEAP] );
		}
		else
		{
			heapsize[SYSTEM_HEAP]=allsize;
			heapmem[SYSTEM_HEAP]=allmem;
			heapmem[DEBUG_HEAP]=NULL;
			heapsize[DEBUG_HEAP]=0;
		}
		assert(heapmem[SYSTEM_HEAP]);

		warning("ARCHALLOC: System heap size is %u bytes\n",heapsize[SYSTEM_HEAP]);

		heaps[SYSTEM_HEAP].SetGranularity(MEM_ALLOC_GRANULARITY,MEM_ALIGN_GRANULARITY);
		heaps[SYSTEM_HEAP].SetHeapID(SYSTEM_HEAP);
		heaps[SYSTEM_HEAP].AddHeapMemory( heapmem[SYSTEM_HEAP], heapsize[SYSTEM_HEAP] );
		heapdummyalloc[SYSTEM_HEAP]=heaps[SYSTEM_HEAP].Allocate(4,1,0,"Dummy allocation",0);
		heaps[SYSTEM_HEAP].CheckConsistency();


		if ( heapmem[DEBUG_HEAP] )
		{
			heaps[DEBUG_HEAP].SetGranularity(MEM_ALLOC_GRANULARITY,MEM_ALIGN_GRANULARITY);
			heaps[DEBUG_HEAP].SetHeapID(DEBUG_HEAP);
			heaps[DEBUG_HEAP].AddHeapMemory( heapmem[DEBUG_HEAP], heapsize[DEBUG_HEAP] );
			heapdummyalloc[DEBUG_HEAP]=heaps[DEBUG_HEAP].Allocate(4,1,0,"Dummy allocation",0);
			heaps[DEBUG_HEAP].CheckConsistency();
		}

		currentheap=&heaps[SYSTEM_HEAP];
    heapstack[heapstackptr] = &heaps[SYSTEM_HEAP];
    osmem_free(safetyvalve);
	}
	#endif
}


void tag_allocation( size_t size,
                    void* chunk,
                    bool array,
										int line,
										const char *file )
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void free_tag( void* chunk, bool array )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static bool memsizechecked=false;
static int  memsizemb=0;


#if 0
static pvoid checkptr[32];
#endif

int CheckMemorySizeMB(void)
{
	if ( !memsizechecked )
	{
  	void* alloc = malloc( 96 * 1024 * 1024 );
		if ( alloc )
		{
			memsizemb=128;
			free(alloc);
		}
		else
		{
			memsizemb=32;
		}
		memsizechecked=true;
#if 0
		#if 0
		mem_total_avail = mem_get_largest_avail();
		#else
		for ( int i=31; i>=0; i-- )
		{
			checkptr[i]=malloc( 1<<i );
			if ( checkptr[i] )
			{
				mem_total_avail += 1<<i;
			}
		}
		for ( int i=0; i<32; i++ )
		{
			if ( checkptr[i] )
			{
				free(checkptr[i]);
			}
		}
		#endif
#endif
	}
	return memsizemb;
}



