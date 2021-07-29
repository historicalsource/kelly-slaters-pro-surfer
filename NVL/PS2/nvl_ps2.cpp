#include <libipu.h>
#include <libdma.h>
#include <assert.h>
#include <eekernel.h>
#include <string.h>
#include <ctype.h>
#include <libcdvd.h>
#include <sifdev.h>
#include <malloc.h>

#include "nvl_ps2.h"
#include "nvlstream_ps2.h"

#define NVL_IPU_FILE_HEADER_SIZE      16
#define NVL_MAX_READ_DATA_SIZE        (128*1024*1024)
#define NVL_MAX_DMA_CHUNK_SIZE        (1024*1024-16)
#define NVL_TAG_BUFFER_SIZE           ((NVL_MAX_READ_DATA_SIZE/NVL_MAX_DMA_CHUNK_SIZE+1)<<4)

#define NVL_MOVIE_MIN_WIDTH           32
#define NVL_MOVIE_MAX_WIDTH           512
#define NVL_MOVIE_MIN_HEIGHT          32
#define NVL_MOVIE_MAX_HEIGHT          480
#define NVL_COMPRESSED_FRAME_MIN_SIZE 0x200

#define NVL_IPU_CTRL_VAL(a)           (((a)&~0x00000004)<<16)
#define NVL_DTD(a)                    (((a)>>2)&1)
#define NVL_DMA_ADDR(a)               ((unsigned int)(a)&0x0fffffff)
#define NVL_UNCACHED_ADDR(a)          (((unsigned int)(a)&0x0fffffff)|0x20000000)
#define NVL_BOUND(a,x)                ((((a)+(x)-1)/(x))*(x))
#define NVL_MIN(x,y)                  (((x)<(y))?(x):(y))

enum
{
  NVL_FLAG_NONE       = 0,
  NVL_FLAG_PAUSED     = 0x00000001,
  NVL_FLAG_PLAYING    = 0x00000002,
  NVL_FLAG_LOOPING    = 0x00000004,
  NVL_FLAG_STARTING   = 0x00000008,
//  NVL_FLAG_STOPPING   = 0x00000010
};

struct nvlMovieSource
{
  int                       refs;
  char                      *filename;
  void                      *buf;
  int                       bufsize;
};

struct nvlMovie
{
  nvlMovieSource            *src;
  nvlStream                 *s;
  int                       dataSize;
  int                       width;
  int                       height;
  volatile int              currentFrame;
  int                       framesTotal;
  void                      *decodeBuffer;
  void                      *tagBuffer;
  nvlOutputBufferCallback   putFunc;
  void                      *userData;
  volatile unsigned int     flags;
  sceIpuDmaEnv              dmaEnvironment;
  int                       dmaToIpuSize;
  int                       currentDmaSize;
};

static struct
{
  int                       fromIPUHandlerID;
  int                       toIPUHandlerID;
  int                       disableToIPU;
  int                       disableFromIPU;
  nvlMovie                  *pMovies[NVL_MAX_MOVIE_NUM];
  int                       runFullAdvance;
  volatile int              currentMovieIdx;
  volatile int              advanceOK;
  volatile int              ioPending;
  volatile int				initInterrupts;
}

              nvlSystemData;
static int    nvlSystemInitialized = 0;
static void*  (*nvlAllocFunc)(int, int);
static void   (*nvlFreeFunc)(void*);

void nvlWaitForVB();

static inline void nvlResetIPU()
{
#ifdef DEBUG
  int           res;
#endif

  sceIpuReset();
#ifdef DEBUG
  res =
#endif
  sceIpuSync( 0, 0 );
#ifdef DEBUG
  if( res != 0 )
  {
    asm( "break" );
  }
#endif
  sceIpuBCLR( 0 );
#ifdef DEBUG
  res =
#endif
  sceIpuSync( 0, 0 );
#ifdef DEBUG
  if( res != 0 )
  {
    asm( "break" );
  }
#endif
}

static inline void nvlDMAtoIPU( void* tag )
{
#ifdef DEBUG
  if( tag == NULL || ((unsigned int)tag & 7) )
  {
    asm( "break" );
  }
#endif
	__asm__ ("
		.set noreorder
		sync.l
		nop
		.set reorder
	");
  *D_STAT  = 1 << 4;
  DPUT_D4_QWC( 0 );
  DPUT_D4_TADR( (unsigned int)NVL_DMA_ADDR(tag) );
  DPUT_D4_CHCR( 0x105 );
	__asm__ ("
		.set noreorder
		sync.l
		nop
		.set reorder
	");
}

static inline void nvlDMAfromIPU( void* dst, size_t size )
{
#ifdef DEBUG
  if( dst == NULL || ((unsigned int)dst & 7) || size == 0 )
  {
    asm( "break" );
  }
#endif
	__asm__ ("
		.set noreorder
		sync.l
		nop
		.set reorder
	");
  *D_STAT  = 1 << 3;
  DPUT_D3_QWC( size >> 4 );
  DPUT_D3_MADR( (volatile unsigned int)NVL_DMA_ADDR(dst) );
  DPUT_D3_CHCR( 0x100 );
	__asm__ ("
		.set noreorder
		sync.l
		nop
		.set reorder
	");
}

static inline void nvlScTag( unsigned int *p, void *addr, unsigned int id, unsigned int qwc )
{
//  *(unsigned long*)p = (unsigned long)addr << 32 | (unsigned long)id << 28 | (unsigned long)qwc;
  *(unsigned long*)p = ((unsigned long)addr << 32) | (0x1UL << 31) | ((unsigned long)id << 28) | (unsigned long)qwc;
}

static inline void nvlSetTagDMAtoIPU( void *tag, void *p, int size )
{
  int           chunksize;
  unsigned int  *unctag = (unsigned int*)NVL_UNCACHED_ADDR( tag );
  unsigned char *dmap = (unsigned char*)NVL_DMA_ADDR( p );

  while( size )
  {
    chunksize = NVL_MIN( size, NVL_MAX_DMA_CHUNK_SIZE );
    size -= chunksize;
//    nvlScTag( unctag, dmap, (size)? 3: 0, NVL_BOUND(chunksize, 16) >> 4 );
    nvlScTag( unctag, dmap, (size)? 3: 0, chunksize >> 4 );
    unctag += 4;
    dmap += chunksize;
  }
  // Make sure that doubleword after last tag entry is 0
  nvlScTag( unctag, 0, 0, 0 );
}

static inline void nvlMovieUpdateDMAStatus( nvlMovie* m )
{
  m->currentDmaSize = m->dmaEnvironment.d4qwc << 4;
  for( unsigned long *pTag = (unsigned long*)m->dmaEnvironment.d4tadr; *pTag; pTag += 2 )
  {
    m->currentDmaSize += (int)(*pTag & 0xffffUL) << 4;
  }
}

static inline void nvlFeedIPU( nvlMovie* m )
{
  void  *data;
  int   s = m->dmaToIpuSize;

  data = nvlStreamGet( m->s, &s );
  assert( !((s & 0xf) && !nvlStreamEOS(m->s)) );

  if( data && s )
  {
    nvlSetTagDMAtoIPU( m->tagBuffer, data, s );
    nvlDMAtoIPU( m->tagBuffer );
    m->currentDmaSize = s;
  }
  else// if( !nvlStreamEOS(m->s) )
  {
    nvlSystemData.ioPending = 1;
  }
}

static inline void nvlIFeedIPU( nvlMovie* m )
{
  void  *data;
  int   s = m->dmaToIpuSize;

  data = nvlIStreamGet( m->s, &s );
#ifdef DEBUG
  if( (s & 0xf) && !nvlStreamEOS(m->s) )
  {
    asm( "break" );
  }
#endif

  if( data && s )
  {
    nvlSetTagDMAtoIPU( m->tagBuffer, data, s );
    nvlDMAtoIPU( m->tagBuffer );
    m->currentDmaSize = s;
  }
  else// if( !nvlStreamEOS(m->s) )
  {
    nvlSystemData.ioPending = 1;
  }
}

static inline void nvlDecodeFrame( nvlMovie* m )
{
  unsigned int  ipuFlags;

  assert( m && (m->flags & (NVL_FLAG_PLAYING|NVL_FLAG_STARTING)) );
  if( m->flags & NVL_FLAG_STARTING )
  {
    nvlResetIPU();
    nvlFeedIPU( m );
    DPUT_IPU_CTRL( 0 );
    sceIpuStopDMA( &m->dmaEnvironment );
    m->flags &= ~NVL_FLAG_STARTING;
    m->flags |= NVL_FLAG_PLAYING;
  }
  sceIpuRestartDMA( &m->dmaEnvironment );
  assert( ((m->width * m->height) << 2) <= NVL_MAX_DMA_CHUNK_SIZE );
  nvlDMAfromIPU( m->decodeBuffer, (m->width * m->height) << 2 );
  // decode flags section
  sceIpuFDEC( 0 );
  sceIpuSync( 0, 0 );
  ipuFlags = ( sceIpuGetFVdecResult() >> 24 ) & 0xff;
  sceIpuSync( 0, 0 );
  sceIpuFDEC( 8 );
  sceIpuSync( 0, 0 );
  // set flags
  DPUT_IPU_CTRL( NVL_IPU_CTRL_VAL(ipuFlags) );
  sceIpuIDEC
  (
    SCE_IPU_IDEC_RGB32,
    SCE_IPU_IDEC_NODITHER,
    SCE_IPU_IDEC_NOOFFSET,
    NVL_DTD(ipuFlags),
    1,
    0
  );
  sceIpuSync( 1, 0 );
  nvlSystemData.advanceOK = 0;
}

static inline void nvlIDecodeFrame( nvlMovie* m )
{
  unsigned int  ipuFlags;

#ifdef DEBUG
  if( !m || (m->flags & (NVL_FLAG_PLAYING|NVL_FLAG_STARTING)) == 0 )
  {
    asm( "break" );
  }
#endif
  if( m->flags & NVL_FLAG_STARTING )
  {
    nvlResetIPU();
    nvlIFeedIPU( m );
    DPUT_IPU_CTRL( 0 );
    sceIpuStopDMA( &m->dmaEnvironment );
    m->flags &= ~NVL_FLAG_STARTING;
    m->flags |= NVL_FLAG_PLAYING;
  }
  sceIpuRestartDMA( &m->dmaEnvironment );
#ifdef DEBUG
  if( ((m->width * m->height) << 2) > NVL_MAX_DMA_CHUNK_SIZE )
  {
    asm( "break" );
  }
#endif
  nvlDMAfromIPU( m->decodeBuffer, (m->width * m->height) << 2 );
  // decode flags section
  sceIpuFDEC( 0 );
  sceIpuSync( 0, 0 );
  ipuFlags = ( sceIpuGetFVdecResult() >> 24 ) & 0xff;
  sceIpuSync( 0, 0 );
  sceIpuFDEC( 8 );
  sceIpuSync( 0, 0 );
  // set flags
  DPUT_IPU_CTRL( NVL_IPU_CTRL_VAL(ipuFlags) );
  sceIpuIDEC
  (
    SCE_IPU_IDEC_RGB32,
    SCE_IPU_IDEC_NODITHER,
    SCE_IPU_IDEC_NOOFFSET,
    NVL_DTD(ipuFlags),
    1,
    0
  );
  sceIpuSync( 1, 0 );
  nvlSystemData.advanceOK = 0;
}

static int nvlDMAFromIPUHandler( int ch )
{
  if( nvlSystemData.initInterrupts )
  {
    ExitHandler();
    return -1;
  }

  register nvlMovie *m = nvlSystemData.pMovies[nvlSystemData.currentMovieIdx];
  register int      i;

#ifdef DEBUG
  if( m == NULL )
  {
    asm( "break" );
  }
#endif
  m->currentFrame++;
  if( m->currentFrame == m->framesTotal - 1 )
  {
    // There is a problem decoding the last frame's delimiter code.
    // Let's just skip the last frame for now.
    m->currentFrame = m->framesTotal;
  }
  sceIpuFDEC( 32 );
  sceIpuSync( 0, 0 );

  sceIpuStopDMA( &m->dmaEnvironment );
  nvlMovieUpdateDMAStatus( m );

  for( i = nvlSystemData.currentMovieIdx + 1; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    m = nvlSystemData.pMovies[i];
    if(
        m &&
        (m->flags & (NVL_FLAG_PLAYING | NVL_FLAG_STARTING)) &&
        !(m->flags & NVL_FLAG_PAUSED) &&
        m->currentFrame < m->framesTotal &&
        m->currentDmaSize + nvlStreamAvailable(m->s) >= NVL_COMPRESSED_FRAME_MIN_SIZE
      )
    {
      break;
    }
  }
  if( i < NVL_MAX_MOVIE_NUM )
  {
    nvlSystemData.currentMovieIdx = i;
    nvlIDecodeFrame( m );
  }
  else
  {
    nvlSystemData.advanceOK = 1;
  }

  ExitHandler();
  return -1;
}

static int nvlDMAToIPUHandler( int ch )
{
  if( nvlSystemData.initInterrupts )
  {
    ExitHandler();
    return -1;
  }

  register nvlMovie *m = nvlSystemData.pMovies[nvlSystemData.currentMovieIdx];

#ifdef DEBUG
  if( m == NULL )
  {
    asm( "break" );
  }
#endif
  nvlIFeedIPU( m );
  ExitHandler();
  return -1;
}

void nvlInit()
{
  if( !nvlSystemInitialized )
  {
    memset( &nvlSystemData, 0, sizeof(nvlSystemData) );
    sceIpuInit();
    nvlSystemData.advanceOK = 1;
	nvlSystemData.initInterrupts = 1;
    nvlSystemData.fromIPUHandlerID = AddDmacHandler( DMAC_FROM_IPU, nvlDMAFromIPUHandler, 0 );
    assert( nvlSystemData.fromIPUHandlerID >= 0 );
    nvlSystemData.disableFromIPU = EnableDmac( DMAC_FROM_IPU );
    nvlSystemData.toIPUHandlerID = AddDmacHandler( DMAC_TO_IPU, nvlDMAToIPUHandler, 0 );
    assert( nvlSystemData.toIPUHandlerID >= 0 );
    nvlSystemData.disableToIPU = EnableDmac( DMAC_TO_IPU );
	nvlSystemData.initInterrupts = 0;
    nvlSystemInitialized = 1;
  }
}

void nvlReset()
{
  nvlShutdown();
  nvlInit();
}

void nvlShutdown()
{
  if( nvlSystemInitialized )
  {
    nvlStopAllMovies();

    DisableDmac( DMAC_FROM_IPU );
    RemoveDmacHandler( DMAC_FROM_IPU, nvlSystemData.fromIPUHandlerID );
    if( !nvlSystemData.disableFromIPU )
    {
      EnableDmac( DMAC_FROM_IPU );
    }
    DisableDmac( DMAC_TO_IPU );
    RemoveDmacHandler( DMAC_TO_IPU, nvlSystemData.toIPUHandlerID );
    if( !nvlSystemData.disableToIPU )
    {
      EnableDmac( DMAC_TO_IPU );
    }

    nvlSystemInitialized = 0;
  }
}

void nvlStopAllMovies()
{
  if( nvlSystemInitialized )
  {
    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] )
      {
        nvlMovieSource  *src = nvlSystemData.pMovies[i]->src;

        nvlStopMovie( nvlSystemData.pMovies[i] );
        if( src && src->refs == 0 )
        {
          nvlReleaseMovieSource( src );
        }
      }
    }
  }
}

void nvlPauseAllMovies()
{
  if( nvlSystemInitialized )
  {
    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] )
      {
        nvlPauseMovie( nvlSystemData.pMovies[i] );
      }
    }
  }
}

void nvlUnpauseAllMovies()
{
  if( nvlSystemInitialized )
  {
    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] )
      {
        nvlUnpauseMovie( nvlSystemData.pMovies[i] );
      }
    }
  }
}

nvlMovieSource *nvlLoadMovieSource( void* data, size_t )
{
  assert( 0 && "Not implemented." );
  return NULL;
}

nvlMovieSource *nvlLoadMovieSource( nvlInputBufferCallback )
{
  assert( 0 && "Not implemented." );
  return NULL;
}

nvlMovieSource *nvlLoadMovieSource( const char* filename, int buffer_size, void* buffer )
{
  nvlMovieSource  *rv = NULL;
  char            hostStr[] = "HOST";
  const char      *sp, *hp;
  int             hostFile = 1, found = 0;
  sceCdlFILE      fp;

  assert( nvlSystemInitialized );
  assert( filename );
  for( sp = filename, hp = hostStr; *sp && *hp; sp++, hp++ )
  {
    if( toupper(*sp) != *hp )
    {
      hostFile = 0;
      break;
    }
  }
  if( hostFile && ((sp[0] == ':') || ((sp[0] == '0' || sp[0] == '1') && sp[1] == ':')) )
  {
    int fh = sceOpen( filename, SCE_RDONLY );

    if( fh >= 0 )
    {
      found = 1;
      sceClose( fh );
    }
  }
  else if( sceCdSearchFile(&fp, filename) )
  {
    found = 1;
  }
  if( found )
  {
    if( nvlAllocFunc )
    {
      rv = (nvlMovieSource*)(*nvlAllocFunc)( 1, sizeof(nvlMovieSource) );
    }
    else
    {
      rv = (nvlMovieSource*)malloc( sizeof(nvlMovieSource) );
    }
    assert( rv );
    memset( rv, 0, sizeof(nvlMovieSource) );
    if( nvlAllocFunc )
    {
      rv->filename = (char*)(*nvlAllocFunc)( 1, strlen(filename) + 1 );
    }
    else
    {
      rv->filename = (char*)malloc( strlen(filename) + 1 );
    }
    assert( rv );
    strcpy( rv->filename, filename );
    rv->buf = buffer;
    rv->bufsize = buffer_size;
  }
  return rv;
}

void nvlReleaseMovieSource( nvlMovieSource* src )
{
  assert( nvlSystemInitialized );
  assert( src && src->filename );
  assert( src->refs == 0 && "The moviesource is in use." );
  if( nvlFreeFunc )
  {
    (*nvlFreeFunc)( src->filename );
    (*nvlFreeFunc)( src );
  }
  else
  {
    free( src->filename );
    free( src );
  }
}

int nvlMovieGetWidth( const nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
  return m->width;
}

int nvlMovieGetHeight( const nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
  return m->height;
}

int nvlMovieGetFrameNum( const nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
  return m->framesTotal;
}

int nvlMovieGetFrame( const nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
  return m->currentFrame;
}

nvlResult nvlPauseMovie( nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
  if( m->flags & NVL_FLAG_PAUSED )
  {
    return NVL_RESULT_PAUSED;
  }
  else
  {
    m->flags &= ~NVL_FLAG_PLAYING;
    m->flags |= NVL_FLAG_PAUSED;
    return NVL_RESULT_SUCCESS;
  }
}

nvlResult nvlUnpauseMovie( nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
  if( m->flags & NVL_FLAG_PAUSED )
  {
    m->flags &= ~NVL_FLAG_PAUSED;
    m->flags |= NVL_FLAG_PLAYING;
    return NVL_RESULT_SUCCESS;
  }
  else
  {
    return NVL_RESULT_PLAYING;
  }
}

nvlMovie *nvlAddMovie( nvlMovieSource* src )
{
  nvlMovie  *rv;
  nvlStream *s;

  assert( nvlSystemInitialized );
  assert( src && src->filename );
  s = nvlStreamCreate( src->filename, src->bufsize, src->buf );
  if( s )
  {
    int   i, size;
    char  *pchar;
    short *pshort;
    int   *pint;

    if( nvlAllocFunc )
    {
      rv = (nvlMovie*)(*nvlAllocFunc)( 1, sizeof(nvlMovie) );
    }
    else
    {
      rv = (nvlMovie*)malloc( sizeof(nvlMovie) );
    }
    assert( rv );
    memset( rv, 0, sizeof(nvlMovie) );
    rv->src = src;
    src->refs++;
    rv->s = s;

    // Check if the header is correct
    size = 4;
    pchar = (char*)nvlStreamGet( s, &size );
    assert( size == 4 && pchar[0] == 'i' && pchar[1] == 'p' && pchar[2] == 'u' && pchar[3] == 'm' );

    // Get the data size
    size = 4;
    pint = (int*)nvlStreamGet( s, &size );
    assert( size == 4 && *pint > 0 );
    rv->dataSize = *pint;

    // Get width
    size = 2;
    pshort = (short*)nvlStreamGet( s, &size );
    assert( size == 2 && *pshort >= NVL_MOVIE_MIN_WIDTH && *pshort <= NVL_MOVIE_MAX_WIDTH && !(*pshort & 0xf) );
    rv->width = *pshort;

    // Get height
    size = 2;
    pshort = (short*)nvlStreamGet( s, &size );
    assert( size == 2 && *pshort >= NVL_MOVIE_MIN_HEIGHT && *pshort <= NVL_MOVIE_MAX_HEIGHT && !(*pshort & 0xf) );
    rv->height = *pshort;

    // Get number of frames
    size = 4;
    pint = (int*)nvlStreamGet( s, &size );
    assert( size == 4 && *pint > 0 );
    rv->framesTotal = *pint;

    // 2nd parameter should be multiplied by framerate but the framerate is the same for all movies.
    nvlStreamSetBitRate ( s, rv->dataSize / rv->framesTotal );

    if( nvlAllocFunc )
    {
      rv->decodeBuffer = (*nvlAllocFunc)( 64, rv->width * rv->height << 2 );
      rv->tagBuffer = (*nvlAllocFunc)( 64, NVL_TAG_BUFFER_SIZE );
    }
    else
    {
      rv->decodeBuffer = memalign( 64, rv->width * rv->height << 2 );
      rv->tagBuffer = memalign( 64, NVL_TAG_BUFFER_SIZE );
    }
    assert( rv->decodeBuffer );
    assert( rv->tagBuffer );

    rv->dmaToIpuSize = ~0xf & NVL_MIN( NVL_MAX_DMA_CHUNK_SIZE, nvlStreamReqSize(rv->s) );

    for( i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] == NULL )
      {
        break;
      }
    }
    assert( i < NVL_MAX_MOVIE_NUM );
    nvlSystemData.pMovies[i] = rv;
  }
  return rv;
}

nvlResult nvlPlayMovie( nvlMovie* m, nvlOutputBufferCallback putFunc, void* userData, int looping )
{
  assert( nvlSystemInitialized && m && putFunc && m->flags == NVL_FLAG_NONE );
  m->putFunc = putFunc;
  m->userData = userData;
  m->flags |= NVL_FLAG_STARTING;
  if( looping )
  {
    m->flags |= NVL_FLAG_LOOPING;
    nvlStreamSetLoopSkip( m->s, NVL_IPU_FILE_HEADER_SIZE, true );
  }
  return NVL_RESULT_SUCCESS;
}

unsigned gAdvance = 0;
void nvlAdvance()
{
  nvlMovie  *m;

  if( nvlSystemInitialized )
  {
    if( nvlSystemData.ioPending )
    {
      // I'll deal with this later if there is any need of it. Looks like it's not an issue any more.
/*
      m = nvlSystemData.pMovies[nvlSystemData.currentMovieIdx];
      assert( m );
      nvlFeedIPU( m );
      nvlSystemData.ioPending = 0;
*/
    }

    if( nvlSystemData.runFullAdvance && nvlSystemData.advanceOK )
    {
      // copy and rearrange image to the output buffer
      // this will be eliminated when we have MOVIE_TEXTURE flag in NGL, hopefully soon
      for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
      {
        m = nvlSystemData.pMovies[i];
        if( m && (m->flags & NVL_FLAG_PLAYING) )
        {
          size_t  outputBufSize = m->width * m->height << 2;
          void    *targ = (*m->putFunc)( m, &outputBufSize, m->userData );
          int     mbx = m->width >> 4, mby = m->height >> 4;

          assert( outputBufSize >= (size_t)(m->width * m->height << 2) );
          for( int y = 0; y < mby; y++ )
          {
            for( int x = 0; x < mbx; x++ )
            {
              int *src = (int*)NVL_UNCACHED_ADDR( m->decodeBuffer ) + ( y * mbx << 8 ) + ( x << 8 ),
                  *dst = (int*)NVL_UNCACHED_ADDR( targ ) + ( y << 8 ) * mbx + ( x << 4 );

              for( int i = 0; i < 16; i++ )
              {
                memcpy( dst + (mbx * i << 4), src + (i << 4), sizeof(int) * 16 );
              }
            }
          }
        }
      }
    }

    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      m = nvlSystemData.pMovies[i];
/*
      if( m && (m->flags & NVL_FLAG_STOPPING) )
      {
        // Remove stopped movies
        int j;

        for( j = 0; j < NVL_MAX_MOVIE_NUM; j++ )
        {
          if( nvlSystemData.pMovies[j] == m )
          {
            break;
          }
        }
        assert( j < NVL_MAX_MOVIE_NUM );
        nvlSystemData.pMovies[j] = NULL;
        m->src->refs--;
        nvlStreamDestroyAsync( m->s );
        if( nvlFreeFunc )
        {
          (*nvlFreeFunc)( m->decodeBuffer );
          (*nvlFreeFunc)( m->tagBuffer );
          (*nvlFreeFunc)( m );
        }
        else
        {
          free( m->decodeBuffer );
          free( m );
        }
      }
      else */if( m && m->currentFrame == m->framesTotal && (m->flags & NVL_FLAG_LOOPING) )
      {
        // Rewind movies that are looping and have reached the end
        m->flags &= ~NVL_FLAG_PLAYING;
        m->flags |= NVL_FLAG_STARTING;
        m->currentFrame = 0;
        nvlStreamRewind( m->s );
      }
    }

    if( nvlSystemData.runFullAdvance && !nvlSystemData.advanceOK )
    {
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      int xxx = 0;
    }
    if( nvlSystemData.runFullAdvance && nvlSystemData.advanceOK )
    {
      for( nvlSystemData.currentMovieIdx = 0; nvlSystemData.currentMovieIdx < NVL_MAX_MOVIE_NUM; nvlSystemData.currentMovieIdx++ )
      {
        m = nvlSystemData.pMovies[nvlSystemData.currentMovieIdx];
        if(
            m &&
            (m->flags & (NVL_FLAG_PLAYING | NVL_FLAG_STARTING)) &&
            !(m->flags & NVL_FLAG_PAUSED) &&
            m->currentFrame < m->framesTotal &&
            m->currentDmaSize + nvlStreamAvailable(m->s) >= NVL_COMPRESSED_FRAME_MIN_SIZE
          )
        {
          gAdvance++;
          nvlDecodeFrame( m );
          break;
        }
      }
    }
    nvlSystemData.runFullAdvance ^= 1;
  }
}

void nvlStopMovie( nvlMovie* m )
{
  assert( nvlSystemInitialized && m );
/*
  m->flags &= ~NVL_FLAG_PLAYING;
  m->flags |= NVL_FLAG_STOPPING;
  m->src->refs--;
*/
  int i;

  while( !nvlSystemData.advanceOK )
  {
	  // Processing a frame
	  nvlWaitForVB();
  }

  for( i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] == m )
    {
      break;
    }
  }
  assert( i < NVL_MAX_MOVIE_NUM );
  nvlSystemData.pMovies[i] = NULL;
  m->src->refs--;
  nvlStreamDestroy( m->s );
  if( nvlFreeFunc )
  {
    (*nvlFreeFunc)( m->decodeBuffer );
    (*nvlFreeFunc)( m->tagBuffer );
    (*nvlFreeFunc)( m );
  }
  else
  {
    free( m->decodeBuffer );
    free( m->tagBuffer );
    free( m );
  }
}

nvlResult nvlMovieStatus( const nvlMovie* m )
{
  if( m->flags & (NVL_FLAG_PLAYING|NVL_FLAG_STARTING) )
  {
    return NVL_RESULT_PLAYING;
  }
  else if( m->flags & NVL_FLAG_PAUSED )
  {
    return NVL_RESULT_PAUSED;
  }
  else
  {
    return NVL_RESULT_ERROR;
  }
}

void nvlSetMemoryAllocCallback ( void* (*func)(int alighnment, int size) )
{
  nvlAllocFunc = func;
}

void nvlSetMemoryFreeCallback ( void (*func)(void*) )
{
  nvlFreeFunc = func;
}

//-----------------------------------------------------------------------------------------------------------//

void nvlMovieSync( nvlMovie* m, float localTime )
{
}
