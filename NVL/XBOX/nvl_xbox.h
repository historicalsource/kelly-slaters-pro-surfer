#ifndef NVL_XBOX_H
#define NVL_XBOX_H

#define NVL_MAX_MOVIE_NUM   4

enum nvlResult
{
  NVL_RESULT_ERROR          = -1, // Source file's not found or something like that. Critical errors are treated as fatal.
  NVL_RESULT_NONE           = 0,  // Nothing
  NVL_RESULT_SUCCESS        = 1,  // success
  NVL_RESULT_PLAYING,             // movie is playing
  NVL_RESULT_PAUSED,              // movie is paused
};

enum nvlImageFormat
{
  NVL_IMAGE_FORMAT_8P     = 0,  // No doc for this one
  NVL_IMAGE_FORMAT_24     = 1,  // Not supported on XBOX
  NVL_IMAGE_FORMAT_24R    = 2,  // Not supported on XBOX
  NVL_IMAGE_FORMAT_32     = 3,  // D3DFMT_LIN_X8R8G8B8
  NVL_IMAGE_FORMAT_32R    = 4,  // Not supported on XBOX
  NVL_IMAGE_FORMAT_32A    = 5,  // D3DFMT_LIN_A8R8G8B8
  NVL_IMAGE_FORMAT_32RA   = 6,  // Not supported on XBOX
  NVL_IMAGE_FORMAT_4444   = 7,  // D3DFMT_LIN_A4R4G4B4
  NVL_IMAGE_FORMAT_5551   = 8,  // D3DFMT_LIN_A1R5G5B5
  NVL_IMAGE_FORMAT_555    = 9,  // D3DFMT_LIN_R6G5B5
  NVL_IMAGE_FORMAT_565    = 10, // D3DFMT_LIN_R5G6B5
  NVL_IMAGE_FORMAT_655    = 11, // D3DFMT_LIN_R6G5B5      ??? needs verification
  NVL_IMAGE_FORMAT_664    = 12, // Not supported on XBOX
  NVL_IMAGE_FORMAT_YUY2   = 13, // BINK has format named YUY2 but it's a different one. Use for WMV only
  NVL_IMAGE_FORMAT_UYVY   = 14, // Not supported on XBOX
  NVL_IMAGE_FORMAT_YV12   = 15, // Not supported on XBOX

  NVL_IMAGE_FORMAT_RGBA32 = NVL_IMAGE_FORMAT_32A
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
void            nvlInit               ( void *pDSound8 = NULL );
// Reinitialize hardware and libraries
void            nvlReset              ();
// Shutdown NVL
void            nvlShutdown           ();
// advances a frame for all playing movies
int             nvlAdvance            ();

// Creates source of movie from a file
nvlMovieSource* nvlLoadMovieSource    ( const char* filename, int buffer_size = 0, void *buffer = NULL );
// Creates source of RAM-resident movie
nvlMovieSource* nvlLoadMovieSource    ( void* data, size_t );
// Registers source of movie using callbacks
nvlMovieSource* nvlLoadMovieSource    ( nvlInputBufferCallback );
// Releases all resources associated with movie source
void            nvlReleaseMovieSource ( nvlMovieSource* );

// Creates an nvlMovie struct, adds it to the system and return pointer to it
nvlMovie*       nvlAddMovie           ( nvlMovieSource*, nvlImageFormat format = NVL_IMAGE_FORMAT_YUY2 );
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
int             nvlMovieGetWidth      ( const nvlMovie* );
// get movie hight (in pixels)
int             nvlMovieGetHeight     ( const nvlMovie* );
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
// XBOX specific. Returns current relative movie time.
REFERENCE_TIME  nvlMovieGetTime       ( const nvlMovie* );
// XBOX specific. Returns movie duration.
REFERENCE_TIME  nvlMovieGetDuration   ( const nvlMovie* );
// Set sound volume. Volume range is 0 - 1.0 linear
void            nvlMovieSetVolume     ( nvlMovie* m, float volume );

#endif  // NVL_XBOX_H