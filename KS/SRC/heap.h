
#ifndef _heap_h
#define _heap_h

// application specific heap settings
#include "heaptype.h"

class MemBlockInfo;

typedef unsigned char *PointerMath;
typedef void *Pointer;
typedef size_t MemSize;
typedef int MemSizeDiff;
typedef unsigned int MemFlags;
typedef char MemDescription[32];
typedef int MemLine;

typedef void HeapMessager( const char *message );

enum MemoryBlockTypes
{
	mbtFree      = 0x00000000,
	mbtUsed      = 0x00000001,
};

enum MemoryAllocationFlags
{
	mafNone      = 0x00,
	mafHigh      = 0x01,
};


class Heap
{
	static HeapMessager *HeapWarning;
	static HeapMessager *HeapError;

	public:
	static void SetHeapMessagers( HeapMessager *warningmessager, HeapMessager *errormessager );
	static void HeapAssertFailed( const char *exp, const char *file, int line_no );

	protected:
	static void Warning( const char* Format, ... );
	static void Error( const char* Format, ... );
	static void Dump( HeapMessager *mess, const char* Format, ... );

		// you toucha deese data youself an youse a dead man
	MemBlockInfo *fullhead;
	MemBlockInfo *fulltail;
	#ifdef HEAP_OFTYPE_LINKS
		MemBlockInfo *usedhead;
		MemBlockInfo *usedtail;
		MemBlockInfo *freehead;
		MemBlockInfo *freetail;
	#endif

	MemSize heapsize;
	Pointer heapstart;
	MemSize allocatedblocks;
	MemSize minalloc;
	MemSize minalign;
	MemSize lastalloc;
	MemFlags heapid;

	Pointer loblock;
	Pointer hiblock;

	bool locked;

		// usage stats
	bool    statsuptodate;
	MemSize numblocks;       // total number of blocks
	MemSize numfree;         // number of free blocks
	MemSize numused;         // number of used blocks
	MemSize memblocks;       // total non-overhead memory
	MemSize memfree;         // total free memory
	MemSize memused;         // total used memory
	MemSize ovrblocks;       // overhead in all blocks
	MemSize ovrfree;         // overhead in free blocks
	MemSize ovrused;         // overhead in used blocks
	MemSize largestfree;     // largest allocatable size


	public:
	Heap();
	~Heap();

		// configuration and initialization
	void AddHeapMemory( Pointer ptr, MemSize size );
	void FreeHeapMemory( void );
	void SetGranularity( MemSize foralloc, MemSize foralign );
	bool HasMemory( void ) const { return heapsize>0 && NULL!=heapstart; }
	MemFlags GetHeapID( void ) const { return heapid; }
	void SetHeapID( MemFlags id ) { heapid=id; }

	void MergeAdjacentHeap( Heap &srcheap );

		// informational
	MemSize GetLargestFree( void )    { CheckHeapStats(); return largestfree; }
	MemSize GetTotalFree( void )      { CheckHeapStats(); return memfree+ovrfree; }
	MemSize GetTotalUsed( void )      { CheckHeapStats(); return memused+ovrused; }
	MemSize GetTotalSize( void )      { CheckHeapStats(); return memblocks+ovrblocks; }
	MemSize GetFree( void )           { CheckHeapStats(); return memfree; }
	MemSize GetUsed( void )           { CheckHeapStats(); return memused; }
	MemSize GetSize( void )           { CheckHeapStats(); return memblocks; }
	MemSize GetFreeOverhead( void )   { CheckHeapStats(); return ovrfree; }
	MemSize GetUsedOverhead( void )   { CheckHeapStats(); return ovrused; }
	MemSize GetSizeOverhead( void )   { CheckHeapStats(); return ovrblocks; }
	MemSize GetNumFreeBlocks( void )  { CheckHeapStats(); return numfree; }
	MemSize GetNumUsedBlocks( void )  { CheckHeapStats(); return numused; }
	MemSize GetNumBlocks( void )      { CheckHeapStats(); return numblocks; }

		// allocation, deallocation
	Pointer Allocate( MemSize size, MemSize align=1, MemFlags flags=mafNone, const char *name=NULL, int line=0 );
	void Deallocate( Pointer ptr );

		// check for containment
	bool IsThisYours( const Pointer ptr ) const;

		// Diagnostics and debugging
	void CheckConsistency( void ) const;
	void CheckStackCollision( void ) const;
	void DumpHeap( HeapMessager *dumpto=NULL ) const;
	void DumpSummary( HeapMessager *dumpto=NULL ) const;

	MemSize GetCurrentMemMarker( void ) { return lastalloc; }
	MemSize DumpAllocSinceMarker( MemSize startMarker, HeapMessager *dumpto=NULL ) const;
  MemSize DumpAllocBetweenMarkers( MemSize startMarker, MemSize endMarker, HeapMessager *dumpto=NULL ) const;

		// Heap locking
	void SetHeapLock( bool onOff ) { locked=onOff; }
	bool GetHeapLock( void ) { return locked; }

	protected:
	MemBlockInfo *PtrToBlock( Pointer ptr ) const {	return (MemBlockInfo *) (((PointerMath) ptr)-HeaderSize()); }
	Pointer BlockToPtr( MemBlockInfo *block ) { return (Pointer) (((PointerMath) block)+HeaderSize()); }

	MemBlockInfo *FindFree( MemSize size, MemSize align, MemFlags flags );
	MemBlockInfo *FindUsed( Pointer ptr );

		// mark a block allocated and split it up if necessary
	Pointer AllocateBlock( MemBlockInfo *block, MemSize size, MemSize align, MemFlags flags, const char *name, int line );
		// mark a block as free and merge with any surrounding free blocks
	void FreeBlock( MemBlockInfo *block );

		// move a block from the free list to the used list
	void MoveFreeToUsed( MemBlockInfo *block );
		// move a block from the used list to the free list
	void MoveUsedToFree( MemBlockInfo *block );

		// shift a block to a higher address, adding surplus to size of previous block
	MemBlockInfo *ShiftBlock( MemBlockInfo *block, MemSizeDiff diff );
		// simple block splitter
	MemBlockInfo *SplitBlock( MemBlockInfo *block, MemSize blocksize );
		// split a block in two (or 3) return the part that works for allocation
	MemBlockInfo *AllocSplit( MemBlockInfo *block, MemSize size, MemSize align, MemFlags flags );
		// merge a block with the block after it
	void MergeBlock( MemBlockInfo *block );

		// update heap statistics
	void CheckHeapStats( void );

	MemSize HeaderSize( void ) const;
	MemSize FooterSize( void ) const;

	void SetBlockSentry( MemBlockInfo *block );
	void ClearBlockSentry( MemBlockInfo *block );
	bool TestBlockSentry( const MemBlockInfo *block ) const;

	void WipeBlock( MemBlockInfo *block, char wipeto );

	void CheckConsistencyNoWalls( void ) const;

	void CheckHeapLock( void );

	bool IsThisMine( const MemBlockInfo *ptr ) const;
	bool DoYouContain( const Pointer ptr ) const;

	void InitLoHi( Pointer lo, Pointer hi );
	void CheckLoHi( Pointer lo, Pointer hi ) const;

};





#endif


