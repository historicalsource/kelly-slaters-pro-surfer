
	// included only for compatibility with KS PCH build process
#include "global.h"

	// This contains everything you could ever want to know about the heap stuff
#include "heap.h"

//
// external symbols required to link - fully self contained otherwise
//   memset - only needed if the block wipe or sentry checking are used
//   strcpy - only needed if the heap debugging is used
//


// This should only be turned on to debug internal heap errors
//   It makes things run REALLY slowly

#if 0 //def EVAN
#define CHECKFORERRORS CheckConsistency()
#define CHECKFORERRORSNOWALLS CheckConsistencyNoWalls()
#else
#define CHECKFORERRORS ((void)0)
#define CHECKFORERRORSNOWALLS ((void)0)
#endif

	// magic number for heap blocks
#define HEAPMAGICID           0x7E07
#define NULLHEAPID            0xFF

	// sentry and wipe characters
#define HEADER_SENTRY_CHAR    0xAB
#define FOOTER_SENTRY_CHAR    0xBA
#define CLEAR_SENTRY_CHAR     0x00
#define HEAP_WIPE_ALLOC_CHAR  0xCD
#define HEAP_WIPE_FREE_CHAR   0xDD
#define HEAP_WIPE_CLEAR_CHAR  0x00

#define LO_BLOCK_MAGIC        "LoBl"
#define HI_BLOCK_MAGIC        "HiBl"


struct MemFlagDWord
{
	MemFlags magic  : 16;
	MemFlags heapid :  8;
	MemFlags flags  :  8;
};

class MemBlockInfo
{
	public:
	MemBlockInfo *prev;         // actual previous block
	MemBlockInfo *next;         // actual next block
	#ifdef HEAP_OFTYPE_LINKS
		MemBlockInfo *prevoftype;   // prev free / used block
		MemBlockInfo *nextoftype;   // next free / used block
	#endif
	MemSize size;               // size of block not including the MemBlockInfo
	MemFlagDWord flags;             // currently mbtUsed or mbtFree
	MemFlags blockid;           // leak detection
	#ifdef HEAP_DEBUG
		MemLine lineno;           // debug int
		MemDescription name;      // debug string
	#elif defined(HEAP_SENTRIES)
		MemFlags pre_sentry;      // in a non-debug heap we want a sentry here
	#endif

	MemBlockInfo();
	~MemBlockInfo();

	bool Contains( Pointer ptr, MemSize headersize );
	bool CanHold( MemSize size, MemSize align, MemFlags flags, MemSize headersize  );
};



#ifdef HEAP_ASSERTIONS
#define heapassert(exp) { if(!(exp)) Heap::HeapAssertFailed(#exp, __FILE__, __LINE__); }
#else
#define heapassert(exp) ((void)0)
#endif

inline MemSize RoundUp( MemSize size, MemSize align )
{
	return (size + ((align)-1)) & ~((align)-1);
}

inline MemSize RoundDown( MemSize size, MemSize align )
{
	return (size) & ~((align)-1);
}

inline PointerMath PtrAlign( PointerMath ptr, MemSize align )
{
	MemSize iptr=(MemSize) ptr;
	iptr = RoundUp(iptr,align); //(iptr + ((align)-1)) & ~((align)-1);
	return (PointerMath) iptr;
}

inline PointerMath PtrAlignLow( PointerMath ptr, MemSize align )
{
	MemSize iptr=(MemSize) ptr;
	iptr = RoundDown(iptr,align); //(iptr + ((align)-1)) & ~((align)-1);
	return (PointerMath) iptr;
}

//===================================================================
// MemBlockInfo class


	// note that in practice the ctor & dtor dont really get called because
	//   the blocks are created in place
MemBlockInfo::MemBlockInfo()
{
}

MemBlockInfo::~MemBlockInfo()
{
}


bool MemBlockInfo::Contains( Pointer ptr, MemSize headersize )
{
	PointerMath thatptr=(PointerMath) ptr;
	PointerMath thisptr=(PointerMath) this;
	if ( thatptr>=thisptr && thatptr<(thisptr+size+headersize)  )
	{
		return true;
	}
	return false;
}

bool MemBlockInfo::CanHold( MemSize msize, MemSize align, MemFlags flags, MemSize headersize )
{
	if ( size>msize+headersize )
	{
		if (flags & mafHigh)
		{
			PointerMath bblock=(PointerMath) this;
			PointerMath rblock=bblock;
			rblock += size-msize;
			PointerMath dblock=PtrAlignLow(rblock,align);
			dblock -= headersize;
			MemSizeDiff diff=dblock-bblock;
			return (diff>=0);
		}
		PointerMath thisptr=(PointerMath) this;
		thisptr += headersize;
		PointerMath thatptr=PtrAlign(thisptr,align);
		if ( size-(thatptr-thisptr) > headersize+msize )
		{
			return true;
		}
	}
	return false;
}




//===================================================================
// Heap class

//-------------------------------------------------------------------
// Heap class static stuff - mostly warning & error handling

HeapMessager *Heap::HeapWarning=NULL;
HeapMessager *Heap::HeapError=NULL;

void Heap::HeapAssertFailed( const char *exp, const char *file, int line_no )
{
	Error( "Heap assertion failed: %s:%u\n%s\n",file,line_no, exp);
}

void Heap::SetHeapMessagers( HeapMessager *warningmessager, HeapMessager *errormessager )
{
	HeapWarning=warningmessager;
	HeapError=errormessager;
}


#ifndef BUILD_FINAL
void Heap::Warning( const char* Format, ... )
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  if ( HeapWarning )
    (*HeapWarning)( Work );
}

void Heap::Error( const char* Format, ... )
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  if ( HeapError )
    (*HeapError)( Work );
}

void Heap::Dump( HeapMessager *outmess, const char* Format, ... )
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  if ( outmess )
    (*outmess)( Work );
}
#else
inline void Heap::Warning( const char* Format, ... ) {}
inline void Heap::Error( const char* Format, ... ) {}
inline void Heap::Dump( HeapMessager *outmess, const char* Format, ... ) {}
#endif

//-------------------------------------------------------------------
// Heap class ctor / dtor

	// note that in practice the ctor & dtor dont really get called in time
	//   for them to be useful because the heap gets allocated on the
	//   first call to new which happens before the new for the heap
Heap::Heap()
{
	#if 0  // The heap constructor gets called AFTER the first memory allocation
	fullhead=NULL;
	usedhead=NULL;
	freehead=NULL;
	fulltail=NULL;
	usedtail=NULL;
	freetail=NULL;
	heapstart=NULL;
	heapsize=0;
	allocatedblocks=0;
	loblock=NULL;
	hiblock=NULL;
	#endif
}

Heap::~Heap()
{
	FreeHeapMemory();
}

//-------------------------------------------------------------------
// Set minimum allocation and alignment

void Heap::SetGranularity( MemSize foralloc, MemSize foralign )
{
	minalloc=foralloc;
	minalign=foralign;
}

MemSize Heap::HeaderSize( void ) const
{
	return RoundUp(sizeof(MemBlockInfo),minalloc);
}

MemSize Heap::FooterSize( void ) const
{
	#ifdef HEAP_SENTRIES
	return RoundUp(sizeof(MemFlags),minalloc);
	#else
	return 0;
	#endif
}

//-------------------------------------------------------------------
// Initialize heap with some memory

void Heap::AddHeapMemory( Pointer ptr, MemSize size )
{
	if ( minalloc<1 ) minalloc=1;
	if ( minalign<1 ) minalign=1;
	FreeHeapMemory();
	heapassert(ptr);
	heapassert(fullhead==NULL);
	heapassert(size > HeaderSize());
	fullhead = (MemBlockInfo *) ptr;
	fullhead->size=size-HeaderSize();
	fullhead->flags.flags=mbtFree;
	fullhead->next=NULL;
	fullhead->prev=NULL;
	#ifdef HEAP_OFTYPE_LINKS
		fullhead->nextoftype=NULL;
		fullhead->prevoftype=NULL;
	#endif
	#ifdef HEAP_DEBUG
		strcpy(fullhead->name,"FREE");
		fullhead->lineno=0;
	#endif
	fulltail=fullhead;
	#ifdef HEAP_OFTYPE_LINKS
		freehead=fullhead;
		freetail=fullhead;
		heapassert(usedhead==NULL);
		heapassert(usedtail==NULL);
	#endif
	heapstart=ptr;
	heapsize=size;
	lastalloc=1;
	SetBlockSentry(fullhead);
	fullhead->flags.magic = HEAPMAGICID;
	fullhead->flags.heapid = GetHeapID();
	statsuptodate=false;
	loblock=Allocate(4,minalign,mafNone,"LOWBLOCK",0);
	hiblock=Allocate(4,minalign,mafHigh,"HIGHBLOCK",0);
	InitLoHi(loblock,hiblock);
	CHECKFORERRORS;
}

//-------------------------------------------------------------------
// Initialize heap with some memory

void Heap::FreeHeapMemory( void )
{
	heapstart=NULL;
	heapsize=0;
	fullhead=NULL;
	fulltail=NULL;
	loblock=NULL;
	hiblock=NULL;
	#ifdef HEAP_OFTYPE_LINKS
		usedhead=NULL;
		freehead=NULL;
		usedtail=NULL;
		freetail=NULL;
	#endif
	allocatedblocks=0;
	locked=false;
	statsuptodate=false;
	numblocks=0;
	numfree=0;
	numused=0;
	memblocks=0;
	memfree=0;
	memused=0;
	ovrblocks=0;
	ovrfree=0;
	ovrused=0;
	largestfree=0;
}

void Heap::MergeAdjacentHeap( Heap &srcheap )
{

}

//-------------------------------------------------------------------
// Allocate a block from the heap

Pointer Heap::Allocate( MemSize size, MemSize align, MemFlags flags, const char *name, int line )
{
	statsuptodate=false;
	CHECKFORERRORS;
	#ifdef HEAP_SENTRIES
		size += FooterSize();
	#endif
	size  = RoundUp(size,minalloc);
	align = RoundUp(align,minalign); //align<minalign ? minalign : align;
	#ifdef HEAP_ASSERTIONS
		MemSize allocbefore=allocatedblocks;
	#endif
	MemBlockInfo *splitme=FindFree(size,align,flags);
	CHECKFORERRORS;
	if ( splitme )
	{
		heapassert(splitme->size > size+HeaderSize());
		Pointer rv=AllocateBlock(splitme,size,align,flags,name,line);
		CHECKFORERRORS;
		if ( rv )
		{
			heapassert(allocbefore+1==allocatedblocks);
			heapassert(PtrAlign((PointerMath)rv,align)==(PointerMath) rv);
			MemBlockInfo *mbi=PtrToBlock(rv);
			mbi->flags.magic = HEAPMAGICID;
			mbi->flags.heapid = GetHeapID();
			#ifdef HEAP_SENTRIES
				SetBlockSentry(mbi);
			#endif
			#ifdef HEAP_WIPE_ALLOC
				WipeBlock(mbi,HEAP_WIPE_ALLOC_CHAR);
			#endif
			#ifdef HEAP_WIPE_CLEAR
				WipeBlock(mbi,HEAP_WIPE_CLEAR_CHAR);
			#endif

			CHECKFORERRORS;
		}
		return rv;
	}
	else
	{
		heapassert(allocbefore==allocatedblocks);
		Warning("Unable to allocate %u bytes\n",size);
		return NULL;
	}
}

void Heap::WipeBlock( MemBlockInfo *block, char wipeto )
{
	heapassert(block);
	PointerMath blockdata=(PointerMath) block;
	blockdata += HeaderSize();
	memset(blockdata,wipeto,block->size-FooterSize());
}

void Heap::InitLoHi( Pointer lo, Pointer hi )
{
	heapassert(lo);
	heapassert(hi);
	memcpy(lo,LO_BLOCK_MAGIC,4);
	memcpy(hi,HI_BLOCK_MAGIC,4);
}

void Heap::CheckLoHi( Pointer lo, Pointer hi ) const
{
	heapassert(lo);
	heapassert(hi);
	heapassert(0==memcmp(lo,LO_BLOCK_MAGIC,4));
	heapassert(0==memcmp(hi,HI_BLOCK_MAGIC,4));
}



void Heap::SetBlockSentry( MemBlockInfo *block )
{
	heapassert(block);
	#ifdef HEAP_SENTRIES
	 	PointerMath footer=(PointerMath) block;
 		footer += HeaderSize() + block->size - FooterSize();
 		memset(footer,FOOTER_SENTRY_CHAR,FooterSize());
		#ifndef HEAP_DEBUG
 			memset(&block->pre_sentry,HEADER_SENTRY_CHAR,sizeof(MemFlags));
		#endif
	#endif
}

void Heap::ClearBlockSentry( MemBlockInfo *block )
{
	heapassert(block);
	#ifdef HEAP_SENTRIES
 		PointerMath footer=(PointerMath) block;
 		footer += HeaderSize() + block->size - FooterSize();
 		memset(footer,CLEAR_SENTRY_CHAR,FooterSize());
		#ifndef HEAP_DEBUG
 			memset(&block->pre_sentry,CLEAR_SENTRY_CHAR,sizeof(MemFlags));
		#endif
	#endif
}

bool Heap::TestBlockSentry( const MemBlockInfo *block ) const
{
	heapassert(block);
	#ifdef HEAP_SENTRIES
 		PointerMath footer=(PointerMath) block;
		#ifndef HEAP_DEBUG
	 		PointerMath header=(PointerMath) (&block->pre_sentry);
		#endif
 		footer += HeaderSize() + block->size - FooterSize();
		for ( unsigned int i=0; i<FooterSize(); i++ )
		{
	 		if (footer[i]!=FOOTER_SENTRY_CHAR)
			{
				Warning("Sentry byte %u stomped\n");
				return false;
			}
		}
		#ifndef HEAP_DEBUG
			for ( unsigned int i=0; i<sizeof(MemFlags); i++ )
			{
 				if (header[i]!=HEADER_SENTRY_CHAR) return false;
			}
		#endif
	#endif
	return TRUE;
}



//-------------------------------------------------------------------
// Free a block to the heap

void Heap::Deallocate( Pointer ptr )
{
	if ( ptr==NULL ) return;
	#ifdef HEAP_ASSERTIONS
		heapassert(IsThisYours(ptr));
	#else
		if (!IsThisYours(ptr)) return;
	#endif
	#ifdef HEAP_ASSERTIONS
		MemSize allocbefore=allocatedblocks;
	#endif
	MemBlockInfo *killme=PtrToBlock(ptr); //FindUsed(ptr);
	if ( killme )
	{
		statsuptodate=false;
		heapassert(TestBlockSentry(killme));
		#ifdef HEAP_WIPE_FREE
			WipeBlock(killme,HEAP_WIPE_FREE_CHAR);
		#endif
		#ifdef EVAN
			if ( killme->size > 8192 )
			{
				Warning("Deleting big block: %u\n",killme->size);

			}
		#endif
		FreeBlock(killme);
		heapassert(allocbefore-1==allocatedblocks);
	}
	else
	{
		heapassert(allocbefore==allocatedblocks);
		Warning("Unable to delete block\n");
	}
}

bool Heap::DoYouContain( const Pointer ptr ) const
{
	PointerMath lo=(PointerMath) heapstart;
	PointerMath hi=lo+heapsize;
	PointerMath check=(PointerMath) ptr;
	check -= HeaderSize();
	return (check>=lo && check<hi);
}

	// internal version of IsThisYours
bool Heap::IsThisMine( const MemBlockInfo *mbi ) const
{
	return ( mbi->flags.heapid==GetHeapID() && mbi->flags.magic==HEAPMAGICID );
}

bool Heap::IsThisYours( const Pointer ptr ) const
{
	return ( HasMemory() && DoYouContain( ptr ) && IsThisMine(PtrToBlock(ptr)) );
}

void Heap::CheckHeapLock( void )
{
	#ifdef HEAP_LOCKING
		heapassert( !locked );
	#endif
}

void Heap::CheckStackCollision( void ) const
{
	if ( heapstart )
	{
			// if any of these fail then the stack has probably
			//   collided with the heap
		CheckLoHi(loblock,hiblock);
		heapassert(TestBlockSentry(fulltail));
		#ifdef HEAP_DEBUG
		heapassert(0==strcmp(fulltail->name,"HIGHBLOCK"));
		#endif
		heapassert(TestBlockSentry(fullhead));
		#ifdef HEAP_DEBUG
		heapassert(0==strcmp(fullhead->name,"LOWBLOCK"));
		#endif
	}
}

void Heap::CheckConsistencyNoWalls( void ) const
{
	heapassert(heapstart == (Pointer) fullhead);
	MemBlockInfo *block=fullhead;
	MemSize totalsize=0;
	MemSize totalusedblocks=0;
	CheckStackCollision();
	if ( block )
	{
		heapassert(block->prev==NULL);
		heapassert( (Pointer) block == heapstart );
	}
	else
	{
		heapassert(fulltail==NULL);
		#ifdef HEAP_OFTYPE_LINKS
			heapassert(usedhead==NULL);
			heapassert(usedtail==NULL);
			heapassert(freehead==NULL);
			heapassert(freetail==NULL);
		#endif
	}
	while ( block )
	{
		totalsize += HeaderSize() + block->size;
		#ifndef HEAP_OFTYPE_LINKS
			if ( block->flags.flags & mbtUsed )
				totalusedblocks++;
		#endif
		if ( block->next==NULL )
		{
			heapassert(fulltail==block);
		}
		else
		{
			heapassert(block->next->prev==block);
			#ifdef HEAP_ASSERTIONS
				PointerMath pblock=(PointerMath) block;
				PointerMath nblock=(PointerMath) block->next;
				heapassert((nblock-pblock) == (int) (HeaderSize() + block->size) );
			#endif
		}
		block=block->next;
	}
	heapassert(totalsize==heapsize);
	#ifdef HEAP_OFTYPE_LINKS
	block=freehead;
	if ( block )
	{
		heapassert(block->prevoftype==NULL);
	}
	else
	{
		heapassert(freetail==NULL);
	}
	while ( block )
	{
		if ( block->nextoftype==NULL )
		{
			heapassert(freetail==block);
		}
		else
		{
			heapassert(block->nextoftype->prevoftype==block);
		}
		block=block->nextoftype;
	}
	block=usedhead;
	if ( block )
	{
		heapassert(block->prevoftype==NULL);
	}
	else
	{
		heapassert(usedtail==NULL);
	}
	while ( block )
	{
		totalusedblocks++;
		if ( block->nextoftype==NULL )
		{
			heapassert(usedtail==block);
		}
		else
		{
			heapassert(block->nextoftype->prevoftype==block);
		}
		block=block->nextoftype;
	}

	heapassert( totalusedblocks == allocatedblocks );
	#endif

	Warning("Heap checks out okay\n");
}

void Heap::CheckConsistency( void ) const
{
	heapassert(heapstart == (Pointer) fullhead);
	const MemBlockInfo *block=fullhead;
	MemSize totalsize=0;
	MemSize totalusedblocks=0;
	CheckStackCollision();
	if ( block )
	{
		heapassert(block->prev==NULL);
		heapassert( (Pointer) block == heapstart );
	}
	else
	{
		heapassert(fulltail==NULL);
		#ifdef HEAP_OFTYPE_LINKS
			heapassert(usedhead==NULL);
			heapassert(usedtail==NULL);
			heapassert(freehead==NULL);
			heapassert(freetail==NULL);
		#endif
	}
	while ( block )
	{
		heapassert(IsThisMine(block));
		heapassert(TestBlockSentry(block));
		totalsize += HeaderSize() + block->size;
		#ifndef HEAP_OFTYPE_LINKS
			if ( block->flags.flags & mbtUsed )
				totalusedblocks++;
		#endif
		if ( block->next==NULL )
		{
			heapassert(fulltail==block);
		}
		else
		{
			heapassert(block->next->prev==block);
			#ifdef HEAP_ASSERTIONS
				PointerMath pblock=(PointerMath) block;
				PointerMath nblock=(PointerMath) block->next;
				heapassert((nblock-pblock) == (int) (HeaderSize() + block->size) );
			#endif
		}
		block=block->next;
	}
	heapassert(totalsize==heapsize);
	#ifdef HEAP_OFTYPE_LINKS
	block=freehead;
	if ( block )
	{
		heapassert(block->prevoftype==NULL);
	}
	else
	{
		heapassert(freetail==NULL);
	}
	while ( block )
	{
		if ( block->nextoftype==NULL )
		{
			heapassert(freetail==block);
		}
		else
		{
			heapassert(block->nextoftype->prevoftype==block);
		}
		block=block->nextoftype;
	}
	block=usedhead;
	if ( block )
	{
		heapassert(block->prevoftype==NULL);
	}
	else
	{
		heapassert(usedtail==NULL);
	}
	while ( block )
	{
		totalusedblocks++;
		if ( block->nextoftype==NULL )
		{
			heapassert(usedtail==block);
		}
		else
		{
			heapassert(block->nextoftype->prevoftype==block);
		}
		block=block->nextoftype;
	}

	heapassert( totalusedblocks == allocatedblocks );
	#endif

	Warning("Heap checks out okay\n");
}

void Heap::DumpSummary( HeapMessager *dumpto ) const
{
	MemSize marker=0;
	MemSize endMarker=10000000; // some really big number
	HeapMessager *output = dumpto ? dumpto : HeapWarning;

	if ( !HasMemory() )
	{
		Dump(output,"Heap summary ID: %u This heap has no memory                 \n",heapid);
		return;
	}
	Dump(output,"Heap summary ID: %u Start: %p Size: %u                      \n",heapid,heapstart,heapsize);

	MemBlockInfo *block=fullhead;
	MemSize fullsize=0;
	MemSize freesize=0;
	MemSize usedsize=0;
	MemSize fullblks=0;
	MemSize freeblks=0;
	MemSize usedblks=0;
	while ( block )
	{
		if (( block->blockid >= marker ) && (block->blockid < endMarker))
		{
			fullsize += HeaderSize() + block->size;
			fullblks++;
			if ( block->flags.flags & mbtUsed )
			{
				usedsize += HeaderSize() + block->size;
				usedblks++;
			}
			else
			{
				freesize += HeaderSize() + block->size;
				freeblks++;
			}
		}
		block=block->next;
	}
	if ( fullblks )
	{
		if ( usedblks )
			Dump(output,"        Allocated memory in %5u blocks %8u                \n",usedblks,usedsize);
		if ( freeblks )
			Dump(output,"        Available memory in %5u blocks %8u                \n",freeblks,freesize);
		Dump(output,"        Total     memory in %5u blocks %8u                \n",fullblks,fullsize);
	}
}

void Heap::DumpHeap( HeapMessager *dumpto ) const
{
	DumpAllocSinceMarker(0,dumpto);
}

MemSize Heap::DumpAllocSinceMarker( MemSize marker, HeapMessager *dumpto ) const
{
  return DumpAllocBetweenMarkers(marker, 100000000, dumpto);
}

MemSize Heap::DumpAllocBetweenMarkers( MemSize marker, MemSize endMarker, HeapMessager *dumpto ) const
{
	HeapMessager *output = dumpto ? dumpto : HeapWarning;
	bool headershown=false;

	MemBlockInfo *block=fullhead;
	MemSize fullsize=0;
	MemSize freesize=0;
	MemSize usedsize=0;
	MemSize fullblks=0;
	MemSize freeblks=0;
	MemSize usedblks=0;
	while ( block )
	{
		if (( block->blockid >= marker ) && (block->blockid < endMarker))
		{
			if ( !headershown ) // don't show header unless needed
			{
				headershown=true;
				if ( marker )
					Dump(output,"Dumping Allocations since marker\n");
				else
					Dump(output,"Dumping Heap\n");
				#ifdef HEAP_DEBUG
					Dump(output,"  Line  Description                      Size     Address\n");
					Dump(output,"  ----- -------------------------------- -------- ----------\n");
				#else
					Dump(output,"  F Size     Address\n");
					Dump(output,"  - -------- ----------\n");
				#endif
			}

			if ( marker>0 )
			{
				//heapassert( !(block->flags.flags & mbtUsed));
        // I'm pretty sure this wasn't quite right
        // I added a leak on purpose, and this assert fired.
        // We're trying to make sure that all these blocks think they
        // are used.  So we should assert that block->flags.flags & mbtUsed
        //   - K S  2/13/02
        heapassert( block->flags.flags & mbtUsed );
			}


			#ifdef HEAP_DEBUG
				Dump(output,"  %5u %32s %8u 0x%08X\n",block->lineno,block->name, block->size, (int)(((PointerMath) block)+HeaderSize()) );
			#else
				Dump(output,"  %c %8u 0x%08X\n",(block->flags.flags & mbtUsed) ? 'U' : 'F', block->size, (int)(((PointerMath) block)+HeaderSize()) );
			#endif

			fullsize += HeaderSize() + block->size;
			fullblks++;
			if ( block->flags.flags & mbtUsed )
			{
				usedsize += HeaderSize() + block->size;
				usedblks++;
			}
			else
			{
				freesize += HeaderSize() + block->size;
				freeblks++;
			}
		}
		block=block->next;
	}
	if ( fullblks )
	{
		Dump(output,"        ================================ ========           \n");
		if ( usedblks )
			Dump(output,"        Allocated memory in %5u blocks %8u                \n",usedblks,usedsize);
		if ( freeblks )
			Dump(output,"        Available memory in %5u blocks %8u                \n",freeblks,freesize);
		Dump(output,"        Total     memory in %5u blocks %8u                \n",fullblks,fullsize);
	}
	return fullblks;
}



//-------------------------------------------------------------------
// Find a block that can hold a desired allocation

MemBlockInfo *Heap::FindFree( MemSize size, MemSize align, MemFlags flags )
{
	#ifdef HEAP_OFTYPE_LINKS
	MemBlockInfo *block=(flags & mafHigh) ? freetail : freehead;
	while ( block && !block->CanHold(size+FooterSize(),align,flags,HeaderSize()) )
	{
		#ifdef EVAN
		MemBlockInfo *nblock=(flags & mafHigh) ? block->prevoftype : block->nextoftype;
		if ( nblock && ((MemSize)nblock) < 0x100 )
		{
			Error("The next block is corrupt\n");
		}
		#endif
		block=(flags & mafHigh) ? block->prevoftype : block->nextoftype;
	}
	#else
	MemBlockInfo *block=(flags & mafHigh) ? fulltail : fullhead;
	while ( block && ((block->flags.flags & mbtUsed) || !block->CanHold(size+FooterSize(),align,flags,HeaderSize())) )
	{
		block=(flags & mafHigh) ? block->prev : block->next;
	}
	#endif
	return block;
}

//-------------------------------------------------------------------
// Find a block that contains a pointer

MemBlockInfo *Heap::FindUsed( Pointer ptr )
{
	MemBlockInfo *block=usedhead;
	while ( block && !block->Contains(ptr,HeaderSize()) )
	{
		block=block->nextoftype;
	}
	return block;
}

Pointer Heap::AllocateBlock( MemBlockInfo *block, MemSize size, MemSize align, MemFlags flags, const char *name, int line )
{
	MemBlockInfo *newblock=AllocSplit(block,size,align,flags);
	CHECKFORERRORS;
	#ifdef HEAP_DEBUG
		if ( name )
			strncpy(newblock->name,name,32);
		else
			strcpy(newblock->name,"unknown");
		newblock->name[31]=0;
		newblock->lineno=line;
	#endif
	newblock->blockid = lastalloc++;
	//MemBlockInfo *newblock=(flags & mafHigh) ? block->next : block;
	MoveFreeToUsed(newblock);
	CHECKFORERRORS;
	PointerMath rblock=(PointerMath) newblock;
	heapassert(newblock->size >= size );
	heapassert(newblock->flags.flags & mbtUsed );
	Pointer rv=(Pointer)(rblock+HeaderSize());
	return rv;
}

void Heap::FreeBlock( MemBlockInfo *block )
{
	MoveUsedToFree(block);
	#ifdef HEAP_OFTYPE_LINKS
	if ( block->next && block->nextoftype == block->next )
	{
		MergeBlock(block);
	}
	if ( block->prev && block->prevoftype == block->prev )
	{
		MergeBlock(block->prev);
	}
	#else
	if ( block->next && ( block->next->flags.flags == block->flags.flags ) )
	{
		MergeBlock(block);
	}
	if ( block->prev && ( block->prev->flags.flags == block->flags.flags ) )
	{
		MergeBlock(block->prev);
	}
	#endif
}


// merges a block with the block after it

void Heap::MergeBlock( MemBlockInfo *block )
{
	heapassert(block);
	heapassert(block->next);
	if ( (block->flags.flags & mbtUsed) != (block->next->flags.flags & mbtUsed)  )
	{
		if ( (block->flags.flags & mbtUsed) )
		{
			MoveFreeToUsed(block->next);
		}
		else
		{
			MoveUsedToFree(block->next);
		}
	}
	heapassert( (block->flags.flags & mbtUsed) == (block->next->flags.flags & mbtUsed)  );

	block->next->flags.magic = 0;
	block->next->flags.heapid = NULLHEAPID;

	block->size += block->next->size + HeaderSize();

	MemBlockInfo *nblock=block->next;

		// remove the block from the full list
	MemBlockInfo *prev=nblock->prev;
	MemBlockInfo *next=nblock->next;
	if ( prev )
	{
		prev->next=next;
	}
	else
	{
		// this can never happen
		heapassert(fullhead==nblock);
		fullhead=next;
	}
	if ( next )
	{
		next->prev=prev;
	}
	else
	{
		heapassert(fulltail==nblock);
		fulltail=block;
	}

		// remove the block from the of type list

	#ifdef HEAP_OFTYPE_LINKS
		// remove the block from the full list
	MemBlockInfo *prevoftype=nblock->prevoftype;
	MemBlockInfo *nextoftype=nblock->nextoftype;
	if ( prevoftype )
	{
		prevoftype->nextoftype=nextoftype;
	}
	else
	{
		// this can never happen
		if ( (block->flags.flags & mbtUsed) )
		{
			heapassert(usedhead==nblock);
			usedhead=nextoftype;
		}
		else
		{
			heapassert(freehead==nblock);
			freehead=nextoftype;
		}
	}
	if ( nextoftype )
	{
		nextoftype->prevoftype=prevoftype;
	}
	else
	{
		if ( (block->flags.flags & mbtUsed) )
		{
			heapassert(usedtail==nblock);
			usedtail=prevoftype;
		}
		else
		{
			heapassert(freetail==nblock);
			freetail=prevoftype;
		}
	}
	#endif
	block->flags.magic = HEAPMAGICID;
	block->flags.heapid = GetHeapID();
	SetBlockSentry(block);
}

MemBlockInfo *Heap::ShiftBlock( MemBlockInfo *block, MemSizeDiff diff )
{
	heapassert(block);

		// If this assertion fails then an attempt to do an aligned allocation
		//   at the very start of the heap was made that would have left a
		//   residual memory block too small for a header at the start.
		// To fix this problem make a 1 byte allocation when the heap is created
	heapassert(block->prev);

	heapassert(diff > 0);
	heapassert((MemSize) diff==RoundUp(diff,minalign));
	heapassert(IsThisMine(block));
	heapassert(IsThisMine(block->prev));
	heapassert(TestBlockSentry(block));
	heapassert(TestBlockSentry(block->prev));
	block->prev->size += diff;
	MemBlockInfo *prev=block->prev;
	MemBlockInfo *next=block->next;
	#ifdef HEAP_OFTYPE_LINKS
		MemBlockInfo *prevoftype=block->prevoftype;
		MemBlockInfo *nextoftype=block->nextoftype;
	#endif
	MemFlags      tflags=block->flags.flags;
	MemSize       tsize =block->size;
	#ifdef HEAP_DEBUG
		MemDescription tname;
		MemLine tline;
		strcpy(tname,block->name);
		tline=block->lineno;
	#endif

	block->flags.magic = 0;
	block->flags.heapid = NULLHEAPID;
	MemBlockInfo *retblock=NULL;
	//PointerMath oblock=((PointerMath) block)+diff;
	retblock=(MemBlockInfo *) (((PointerMath) block)+diff);

	if ( next )
	{
		heapassert(((PointerMath) next)-((PointerMath) retblock) > (int) (HeaderSize()+FooterSize()));
		heapassert(((PointerMath) next)-((PointerMath) retblock) == (int) (HeaderSize()+tsize-diff));
	}

	retblock->prev=prev;
	retblock->next=next;
	#ifdef HEAP_OFTYPE_LINKS
		retblock->prevoftype=prevoftype;
		retblock->nextoftype=nextoftype;
	#endif
	retblock->flags.flags=tflags;
	retblock->size =tsize-diff;
	#ifdef HEAP_DEBUG
		strcpy(retblock->name,tname);
		retblock->lineno=tline;
	#endif
	if ( prev )
	{
		prev->next=retblock;
	}
	else
	{
		fullhead=retblock;
	}
	if ( next )
	{
		next->prev=retblock;
	}
	else
	{
		fulltail=retblock;
	}
	#ifdef HEAP_OFTYPE_LINKS
	if ( prevoftype )
	{
		prevoftype->nextoftype=retblock;
	}
	else
	{
		if ( tflags & mbtUsed )
		{
			usedhead=retblock;
		}
		else
		{
			freehead=retblock;
		}
	}
	if ( nextoftype )
	{
		nextoftype->prevoftype=retblock;
	}
	else
	{
		if ( tflags & mbtUsed )
		{
			usedtail=retblock;
		}
		else
		{
			freetail=retblock;
		}
	}
	#endif
	retblock->flags.magic = HEAPMAGICID;
	retblock->flags.heapid = GetHeapID();
	CHECKFORERRORSNOWALLS;
	if ( prev )
		SetBlockSentry(prev);
	CHECKFORERRORSNOWALLS;
	SetBlockSentry(retblock);
	CHECKFORERRORS;
	return retblock;
}

MemBlockInfo *Heap::SplitBlock( MemBlockInfo *block, MemSize blocksize )
{
	heapassert(blocksize > HeaderSize());
	heapassert(block->size > blocksize+HeaderSize());
	PointerMath oblock=(PointerMath) block;
	PointerMath nblock=oblock+blocksize;
	MemBlockInfo *newblock=(MemBlockInfo * ) nblock;
	newblock->size = block->size - blocksize;
	newblock->flags.magic = HEAPMAGICID;
	newblock->flags.heapid = GetHeapID();
	newblock->flags.flags = block->flags.flags;
	newblock->prev=block;
	newblock->blockid=0;
	#ifdef HEAP_OFTYPE_LINKS
	newblock->prevoftype=block;
	#endif
	newblock->next=block->next;
	#ifdef HEAP_OFTYPE_LINKS
	newblock->nextoftype=block->nextoftype;
	#endif
	#ifdef HEAP_DEBUG
		strcpy(newblock->name,block->name);
		newblock->lineno=block->lineno;
	#endif
	block->next=newblock;
	#ifdef HEAP_OFTYPE_LINKS
	block->nextoftype=newblock;
	#endif
	if ( newblock->next )
	{
		newblock->next->prev=newblock;
	}
	else
	{
		fulltail=newblock;
	}
	#ifdef HEAP_OFTYPE_LINKS
	if ( newblock->nextoftype )
	{
		newblock->nextoftype->prevoftype=newblock;
	}
	else
	{
		if ( newblock->flags.flags & mbtUsed )
		{
			usedtail=newblock;
		}
		else
		{
			freetail=newblock;
		}
	}
	#endif
	//block->size-=blocksize;
	block->size=blocksize-HeaderSize();
	block->flags.magic = HEAPMAGICID;
	block->flags.heapid = GetHeapID();
	SetBlockSentry(block);
	SetBlockSentry(newblock);
	return block;
}


MemBlockInfo *Heap::AllocSplit( MemBlockInfo *block, MemSize size, MemSize align, MemFlags flags )
{
	heapassert( block->CanHold( size, align, flags, HeaderSize()) );

	if (flags & mafHigh)
	{
		MemBlockInfo *retblock=NULL;
		PointerMath bblock=(PointerMath) block;
		PointerMath rblock=bblock;
		rblock += block->size-size;
		PointerMath dblock=PtrAlignLow(rblock,align);
		dblock -= HeaderSize();

		MemSizeDiff diff=dblock-bblock;

		if ( diff > (MemSizeDiff)(HeaderSize()+FooterSize()+minalloc) )
		{
			MemBlockInfo *newblock=SplitBlock(block,diff);
			CHECKFORERRORS;
			retblock=newblock->next;
		}
		else if ( diff>0 )
		{
			retblock=ShiftBlock(block,diff);
			CHECKFORERRORS;
		}
		else if ( diff==0 )
		{
			retblock=block;
		}
		else
		{
			Error("Internal heap error: this block should not have been selected\n");
		}
		return retblock;
	}
	else
	{
		//
		// Possible situations:
		//   For block start
		//   - Block starts at current pos
		//     : change block type to used
		//   - Block should start more than 1 block header size from current pos
		//     : create a new block at that position
		//   - Block should start less than 1 block hearer size from current pos
		//     : destroy this block and merge it into predecessor then create new one at new pos
		//
		//
		MemBlockInfo *retblock=NULL;
		PointerMath rblock=(PointerMath) block;
		rblock += HeaderSize();
		PointerMath dblock=PtrAlign(rblock,align);
		MemSizeDiff diff=dblock-rblock;
		if ( dblock==rblock )
		{
		//   - Block starts at current pos
		//     : change block type to used
			retblock=block;
		}
		else if ( 1 ) //dblock-rblock <= (int) HeaderSize() )
		{
		//   - Block should start less than 1 block hearer size from current pos
		//     : destroy this block and merge it into predecessor then create new one at new pos
			retblock=ShiftBlock(block,diff);
			CHECKFORERRORS;
			retblock=retblock;
		}
		else
		{
		//   - Block should start more than 1 block header size from current pos
		//     : create a new block at that position
			MemBlockInfo *newblock=SplitBlock(block,diff);
			CHECKFORERRORS;
			retblock=newblock->next;
		}
		if ( retblock->size - size >  (int) 2*HeaderSize() )
		{
			retblock=SplitBlock(retblock,size+HeaderSize());
			CHECKFORERRORS;
		}
		return retblock;
	}

	return block;
}

void Heap::MoveFreeToUsed( MemBlockInfo *block )
{
	heapassert( !(block->flags.flags & mbtUsed) );

		// Step 1 - Remove from the list of used blocks

	#ifdef HEAP_OFTYPE_LINKS
	MemBlockInfo *prevfree=block->prevoftype;
	MemBlockInfo *nextfree=block->nextoftype;
	if ( prevfree )
	{
		prevfree->nextoftype=nextfree;
	}
	else
	{
		heapassert(freehead==block);
		freehead=nextfree;
	}
	if ( nextfree )
	{
		nextfree->prevoftype=prevfree;
	}
	else
	{
		heapassert(freetail==block);
		freetail=prevfree;
	}

		// Step 2 - Add to the list of free blocks

	MemBlockInfo *prevused=block->prev;
	while ( prevused && !(prevused->flags.flags & mbtUsed))
	{
		prevused=prevused->prev;
	}
	MemBlockInfo *nextused=block->next;
	while ( nextused && !(nextused->flags.flags & mbtUsed))
	{
		nextused=nextused->next;
	}
	block->prevoftype=prevused;
	if ( prevused )
	{
		prevused->nextoftype=block;
	}
	else
	{
		usedhead=block;
	}
	block->nextoftype=nextused;
	if ( nextused )
	{
		nextused->prevoftype=block;
	}
	else
	{
		usedtail=block;
	}
	#endif
	block->flags.flags |= mbtUsed;
	allocatedblocks++;
}



void Heap::MoveUsedToFree( MemBlockInfo *block )
{
	heapassert( block->flags.flags & mbtUsed );

		// Step 1 - Remove from the list of used blocks

	#ifdef HEAP_OFTYPE_LINKS
	MemBlockInfo *prevused=block->prevoftype;
	MemBlockInfo *nextused=block->nextoftype;
	if ( prevused )
	{
		prevused->nextoftype=nextused;
	}
	else
	{
		heapassert(usedhead==block);
		usedhead=nextused;
	}
	if ( nextused )
	{
		nextused->prevoftype=prevused;
	}
	else
	{
		heapassert(usedtail==block);
		usedtail=prevused;
	}

		// Step 2 - Add to the list of free blocks

	MemBlockInfo *prevfree=block->prev;
	while ( prevfree && (prevfree->flags.flags & mbtUsed))
	{
		prevfree=prevfree->prev;
	}
	MemBlockInfo *nextfree=block->next;
	while ( nextfree && (nextfree->flags.flags & mbtUsed))
	{
		nextfree=nextfree->next;
	}
	block->prevoftype=prevfree;
	if ( prevfree )
	{
		prevfree->nextoftype=block;
	}
	else
	{
		freehead=block;
	}
	block->nextoftype=nextfree;
	if ( nextfree )
	{
		nextfree->prevoftype=block;
	}
	else
	{
		freetail=block;
	}
	#endif

	block->flags.flags &= ~mbtUsed;
	allocatedblocks--;
	block->blockid=0;
	#ifdef HEAP_DEBUG
		strcpy(block->name,"FREE");
		block->lineno=0;
	#endif
}

void Heap::CheckHeapStats( void )
{
	if ( !statsuptodate )
	{
		statsuptodate=true;
		numblocks=0;
		numfree=0;
		numused=0;
		memblocks=0;
		memfree=0;
		memused=0;
		ovrblocks=0;
		ovrfree=0;
		ovrused=0;
		largestfree=0;
		MemBlockInfo *block=fullhead;
		while ( block )
		{
			numblocks++;
			memblocks+=block->size-FooterSize();
			ovrblocks+=HeaderSize()+FooterSize();
			if ( block->flags.flags & mbtUsed )
			{
				numused++;
				memused+=block->size-FooterSize();
				ovrused+=HeaderSize()+FooterSize();
			}
			else
			{
				numfree++;
				memfree+=block->size-FooterSize();
				ovrfree+=HeaderSize()+FooterSize();
				if ( block->size-FooterSize() > largestfree )
				{
					largestfree=block->size-FooterSize();
				}
			}
			block=block->next;
		}
	}
}





