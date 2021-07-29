#ifndef _NVLSTREAM_GC_H_
#define _NVLSTREAM_GC_H_

#define NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY  8
#define NVL_MAX_STREAM_NUM                      8

typedef struct nvlStream nvlStream;

// Initialize and start nvl stream system. Parameter for compatibility.
void nvlStreamSystemInit        ( int arg = 0 );
// Shutdown nvl stream system
void nvlStreamSystemShutdown    ();
// Reset nvl stream system
void nvlStreamSystemReset       ();
// Returns true if the stream system is running
int  nvlStreamSystemIsRunning   ();

// Create a stream.
nvlStream*  nvlStreamCreate         ( char* filename, int buffer_size = 0, void *buf = NULL, int looping = 1 );
// Stop any operation on the stream and destroy it
void        nvlStreamDestroy        ( nvlStream* );
// Get data. Returns number of bytes read; -1 if error happened
int         nvlStreamGetCopy        ( nvlStream*, void* dest, int bytes );
// Unlocks last used data
void        nvlStreamUnlock         ( nvlStream* );
// Do we need to implement this function?
int         nvlStreamSeek           ( nvlStream*, int offset, int origin );
// Returns true if end of stream is reached
int         nvlStreamEOS            ( nvlStream* );
// Rewind the stream
void        nvlStreamRewind         ( nvlStream* );
// Set error flag (corrupted data and so on)
void        nvlStreamSetError       ( nvlStream* );
int         nvlStreamGetErrorStatus ( nvlStream* );
int         nvlStreamIsReady        ( nvlStream* );

#endif