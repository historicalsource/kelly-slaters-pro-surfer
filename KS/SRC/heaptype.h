
//
// This file is used to set application specific flags for configuring the
//   heap manager
//

#ifndef _heaptype_h
#define _heaptype_h


#ifndef BUILD_FINAL
		// Heap can generate assertions
	#define HEAP_ASSERTIONS
		// Heap contains description information
	#define HEAP_DEBUG
		// Heap can generate assertions
	#define HEAP_LOCKING
#ifndef TARGET_GC
		// Heap contains sentry blocks to detect out-of-bounds writes
	#define HEAP_SENTRIES
		// Heap returns allocated blocks filled with zeroes
	//#define HEAP_WIPE_CLEAN
		// Heap returns allocated blocks filled with garbage
	#define HEAP_WIPE_ALLOC
		// Heap fills deleted blocks with garbage
	#define HEAP_WIPE_FREE
#endif
#endif

	// the heap already has enough overhead on the GC
//#ifndef TARGET_GC
		// Heap maintains free and used lists
	#define HEAP_OFTYPE_LINKS
//#endif


#endif
