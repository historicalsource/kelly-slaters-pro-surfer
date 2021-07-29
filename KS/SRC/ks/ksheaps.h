

#ifndef _ksheaps_h
#define _ksheaps_h

enum KSHeapIDs {
	SYSTEM_HEAP  = 0,
	DEBUG_HEAP      ,
	COMMON_HEAP     ,
	BEACH_HEAP      ,
	SURFER_HEAP     ,
	SURFER_HEAP2    ,
	NUMBER_OF_HEAPS
};

//
// Heap parentage:
//   The system heap, and debug heap are created from system memory.
//   All the other heaps are children of the system heap.
//

#endif




