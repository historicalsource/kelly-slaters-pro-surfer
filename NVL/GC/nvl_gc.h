#ifndef NVL_GC_H
#define NVL_GC_H

#include <stddef.h>

#define NVL_MAX_MOVIE_NUM         4

enum nvlResult
{
  NVL_RESULT_ERROR          = -1, // Source file's not found or something like that. Critical errors are treated as fatal.
  NVL_RESULT_SUCCESS        = 1,  // success
  NVL_RESULT_PLAYING,             // movie is playing
  NVL_RESULT_PAUSED               // movie is paused
};

enum nvlImageFormat
{
  NVL_IMAGE_FORMAT_YUV411,        // Original output format
  NVL_IMAGE_FORMAT_YUV422,        // Framebuffer format
  NVL_IMAGE_FORMAT_RGBA32,
  NVL_IMAGE_FORMAT_RGBA565,
  NVL_IMAGE_FORMAT_RGBA5A3
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
// Set framebuffer width and height
void            nvlSetFramebufferGC   ( int framebuf_width, int framebuf_height );

// Creates source of movie from a file
nvlMovieSource* nvlLoadMovieSource    ( const char* filename, int buffer_size = 0, void *buffer = NULL );
// Creates source of RAM-resident movie
nvlMovieSource* nvlLoadMovieSource    ( void* data, size_t );
// Registers source of movie using callbacks
nvlMovieSource* nvlLoadMovieSource    ( nvlInputBufferCallback );
// Releases all resources associated with movie source
void            nvlReleaseMovieSource ( nvlMovieSource* );

// Creates an nvlMovie struct, adds it to the system and return pointer to it
nvlMovie*       nvlAddMovie           ( nvlMovieSource*, nvlImageFormat format = NVL_IMAGE_FORMAT_RGBA32 );
// Starts playing movie, the last parameter is a user data value passed back to the nvlOutputBufferCallback function
nvlResult       nvlPlayMovie          ( nvlMovie*, nvlOutputBufferCallback, void* user_data = NULL, int looping = 0 );
// Stops movie and removes it from the system - the movie can't be restarted
void            nvlStopMovie          ( nvlMovie* );
// pause the movie
nvlResult       nvlPauseMovie         ( nvlMovie* );
// unpause the movie
nvlResult       nvlUnpauseMovie       ( nvlMovie* );
// get movie status
nvlResult       nvlMovieStatus        ( const nvlMovie* );
// get movie width (in pixels)
int             nvlMovieGetWidth      ( nvlMovie* );
// get movie hight (in pixels)
int             nvlMovieGetHeight      ( nvlMovie* );
// get total number of frames
int             nvlMovieGetFrameNum   ( const nvlMovie* );
// get current frame
int             nvlMovieGetFrame      ( const nvlMovie* );

// stop all movies
void            nvlStopAllMovies      ();
// pause all movies
void            nvlPauseAllMovies     ();
// unpause all movies
void            nvlUnpauseAllMovies   ();
// Set sound volume. Volume range is 0 - 1.0 linear
void            nvlMovieSetVolume     ( nvlMovie* m, float volume );
// set additional parameters in case if movie is rendered directly to framebuffer
void nvlMovieSetParametersGC( nvlMovie* m, int top, int left, int width, int height );

#endif  // NVL_GC_H