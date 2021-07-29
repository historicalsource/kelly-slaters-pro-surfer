#ifndef NVLSTREAM_PS2_H
#define NVLSTREAM_PS2_H

enum nvlMutexMode
{
  NVL_MUTEX_BLOCK,
  NVL_MUTEX_NOBLOCK
};

typedef int nvlMutex;

// Init a mutual exclusion
void  nvlInitMutex    ( nvlMutex* mtx );
// Deinit a mutual exclusion
void  nvlDestroyMutex ( nvlMutex* mtx );
// Lock a mutex
int   nvlLockMutex    ( nvlMutex* mtx, nvlMutexMode mode );
// Unlock a mutex.
void  nvlUnlockMutex  ( nvlMutex* mtx );
// Lock a mutex from an interrupr handler
int   nvlILockMutex   ( nvlMutex* mtx );
// Unlock a mutex from an interrupr handler
void  nvlIUnlockMutex ( nvlMutex* mtx );

#define NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY      4
#define NVL_STREAM_SYSTEM_CALLBACK_THREAD_PRIORITY  1

enum NVL_THREAD_CONTROL_CODE
{
  NVL_TC_REGISTER         = 1111, // register thread
  NVL_TC_UNREGISTER       = 1112, // unregister thread
  NVL_TC_GET_MAIN_PRI     = 1113, // get main thread priority
  NVL_TC_GET_PRI          = 1114, // get thread's priority
  NVL_TC_CHANGE_MAIN_PRI  = 1115, // change main thread priority
  NVL_TC_CHANGE_PRI       = 1116, // change thread priority
  NVL_TC_GET_HIGHEST      = 1117, // get highest thread priority
  NVL_TC_GET_LOWEST       = 1118, // get lowest thread priority
  NVL_TC_GET_UNIQUE_HIGH  = 1119, // get next "free" priority higher than given
  NVL_TC_GET_UNIQUE_LOW   = 2221, // get next "free" priority lower than given
  NVL_TC_GET_NUM          = 2222  // get number of registered threads
};

typedef struct nvlStream  nvlStream;
typedef int (*nvlThreadHandleCallback)( int func, int id, int priority );

const int   NVL_MAX_STREAM_NUM = 8;

// Initialize and start nvl stream system. Important: has to be called after intialization of any code that
// uses its own CD/DVD callback handler
void nvlStreamSystemInit      ( int initCDVD = 0 );
// Shutdown nvl stream system
void nvlStreamSystemShutdown  ( int exitCDVD = 0 );
// Reset nvl stream system
void nvlStreamSystemReset     ();
// Sets acces to thread manipulation function ( see nvlThreadHandleCallback type and enum NVL_THREAD_CONTROL_CODE )
void nvlStreamSystemSetThreadHandleCallback ( nvlThreadHandleCallback );

// Create a stream.
nvlStream*  nvlStreamCreate       ( const char* filename, int buffer_size = 0, void *buf = NULL, int loop_skip = -1, int rewind_required = 1 );
// Stop any operation on the stream and destroy it
void        nvlStreamDestroy      ( nvlStream* );
// Set stream to be destroyed later and return immediately.
void        nvlStreamDestroyAsync ( nvlStream* );
// Get data with or with no(probably should never happen) lock.
void*       nvlStreamGet          ( nvlStream*, int *bytes, int lock = 1 );
// Get data with or with no(probably should never happen) lock. Must be called from interrupt handler only.
void*       nvlIStreamGet         ( nvlStream*, int *bytes, int lock = 1 );
// Unlocks last used data
void        nvlStreamUnlock       ( nvlStream* );
// Unlocks last used data from an interrupt handler only. Returns false if the stream will be unlocked later, true otherwise.
int         nvlIStreamUnlock      ( nvlStream* );
// Get data with no lock but with copy
int         nvlStreamGetCopy      ( nvlStream*, void* buf, int bytes );
// Get data with no lock but with copy.  Must be called from interrupt handler only.
int         nvlIStreamGetCopy     ( nvlStream*, void* buf, int bytes );
// Do we need to implement this function?
int         nvlStreamSeek         ( nvlStream*, int offset, int origin );
// Returns true if end of stream is reached. Can be called from interrupt handler.
int         nvlStreamEOS          ( nvlStream* );
// Set stream to loop. Can be called from interrupt handler.
void        nvlStreamSetLoopSkip  ( nvlStream*, int loop_skip, int rewind_required );
// Rewind the stream.
int         nvlStreamRewind       ( nvlStream* );
// Rewind the stream from an interrupt handler only. Returns 1 if the stream has been rewound,
// -1 if the stream will be rewoun later and 0 if the stream hasn't reached the end of the file
int         nvlIStreamRewind      ( nvlStream* );
// Set approx. bitrate
void        nvlStreamSetBitRate   ( nvlStream*, int bitrate );
// Returns size of data available (in bytes)
int         nvlStreamAvailable    ( nvlStream* );
// Returns maximum recommended data request size for non-stop streaming
int         nvlStreamReqSize      ( nvlStream* );

// Set callback for memory allocation
void        nvlStreamSetMemoryAllocCallback ( void* (*func)(int alighnment, int size) );
// Set callback for memory free
void        nvlStreamSetMemoryFreeCallback  ( void (*func)(void*) );

#endif  // NVLSTREAM_PS2_H