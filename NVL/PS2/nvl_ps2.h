#ifndef NVL_PS2_H
#define NVL_PS2_H

#define NVL_MAX_MOVIE_NUM         4

enum nvlResult
{
  NVL_RESULT_ERROR          = -1, // Source file's not found or something like that. Critical errors are treated as fatal.
  NVL_RESULT_SUCCESS        = 1,  // success
  NVL_RESULT_PLAYING,             // movie is playing
  NVL_RESULT_PAUSED,              // movie is paused
};

typedef struct nvlMovieSource     nvlMovieSource;
typedef struct nvlMovie           nvlMovie;

// Input callback type. Should return pointer to next chunk of input MPEG data and size of the chunk (last parameter),
// will be given by nvlMovie struct pointer
typedef void* (*nvlInputBufferCallback)( nvlMovie*, size_t* );
// Output callback type. Should return pointer to next output buffer (for texture) and it's size (second parameter),
// will be given by nvlMovie struct pointer
// the last parameter is the user data value passed to the nvlPlayMovie function
typedef void* (*nvlOutputBufferCallback)( nvlMovie*, size_t*, void* );

// Init hardware and libraries
void            nvlInit               ();
// Reinitialize hardware and libraries
void            nvlReset              ();
// Shutdown NVL
void            nvlShutdown           ();
// advances a frame for all playing movies
void            nvlAdvance            ();

// Creates source of movie from a file
nvlMovieSource* nvlLoadMovieSource    ( const char* filename, int buffer_size = 0, void *buffer = NULL );
// Creates source of RAM-resident movie
nvlMovieSource* nvlLoadMovieSource    ( void* data, size_t );
// Registers source of movie using callbacks
nvlMovieSource* nvlLoadMovieSource    ( nvlInputBufferCallback );
// Releases all resources associated with movie source
void            nvlReleaseMovieSource ( nvlMovieSource* );

// Creates an nvlMovie struct, adds it to the system and return pointer to it
nvlMovie*       nvlAddMovie           ( nvlMovieSource* );
// Starts playing movie, the last parameter is a user data value passed back to the nvlOutputBufferCallback function
nvlResult       nvlPlayMovie          ( nvlMovie*, nvlOutputBufferCallback, void* userData = NULL, int looping = 0 );
// Stops movie and removes it from the system - the movie can't be restarted
void            nvlStopMovie          ( nvlMovie* );
// Pause the movie
nvlResult       nvlPauseMovie         ( nvlMovie* );
// Unpause the movie
nvlResult       nvlUnpauseMovie       ( nvlMovie* );
// Get movie status
nvlResult       nvlMovieStatus        ( const nvlMovie* );
// Get movie width (in pixels)
int             nvlMovieGetWidth      ( const nvlMovie* );
// Get movie hight (in pixels)
int             nvlMovieGetHeight      ( const nvlMovie* );
// Get total number of frames
int             nvlMovieGetFrameNum   ( const nvlMovie* );
// Get current frame
int             nvlMovieGetFrame      ( const nvlMovie* );
// Synchronize movie with sound
void            nvlMovieSync          ( nvlMovie*, float localTime );

// Stop all movies
void            nvlStopAllMovies      ();
// Pause all movies
void            nvlPauseAllMovies     ();
// Unpause all movies
void            nvlUnpauseAllMovies   ();

// Set callback for memory allocation
void            nvlSetMemoryAllocCallback ( void* (*func)(int alighnment, int size) );
// Set callback for memory free
void            nvlSetMemoryFreeCallback ( void (*func)(void*) );

#endif  // NVL_PS2_H