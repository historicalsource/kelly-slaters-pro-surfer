#include <string.h>
#include <assert.h>
#include <string.h>
#include <xtl.h>

#include "nvl_xbox.h"

#if defined(NVL_USE_WMV)    // Windows Media Viewer

#include <wmvxmo.h>

#define NVL_NUM_PACKETS   128
#define NVL_PACKET_SIZE   (2048*2*2)

enum
{
  NVL_FLAGS_PLAYING   = 0x00000001,
  NVL_FLAGS_STOPPED   = 0x00000002
};

struct nvlMovieSource
{
  char                      *filename;
  int                       bufsize;
  void                      *buf;
  int                       refc;
};

struct nvlMovie
{
  unsigned int              flags;
  nvlMovieSource            *src;
  nvlOutputBufferCallback   putFunc;
  void                      *data;
  int                       bufsize;
  LPWMVDECODER              pWMVDecoder;
  WMVVIDEOINFO              wmvVideoInfo;
  WAVEFORMATEX              wfx;
  REFERENCE_TIME            rtDuration;
  REFERENCE_TIME            rtStartTime;
  BOOL                      bFirstFrame;
  LPDIRECT3DTEXTURE8        pTexture;
  BYTE                      *pBits;
  int                       pitch;

  LPDIRECTSOUNDSTREAM       pStream;
  DWORD                     adwStatus[NVL_NUM_PACKETS];
  BYTE*                     pbSampleData;
  REFERENCE_TIME            rtTimestamp;
};

extern IDirect3DDevice8    * nglDev;

static int    nvlInitialized = 0;
static struct nvlSystemData_t
{
  nvlMovie                  *pMovies[NVL_MAX_MOVIE_NUM];
  LPDIRECTSOUND8            pDSound;
  int                       privateDSObject;
}             nvlSystemData;

BOOL nvlFindFreePacket( nvlMovie* m, DWORD * pdwIndex )
{
  for( int i = 0; i < NVL_NUM_PACKETS; i++ )
  {
    if( m->adwStatus[ i ] != XMEDIAPACKET_STATUS_PENDING )
    {
      *pdwIndex = i;
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL nvlMovieReady( nvlMovie* m )
{
  REFERENCE_TIME    rtCurrent;

  DirectSoundDoWork();
  nvlSystemData.pDSound->GetTime( &rtCurrent );
  rtCurrent -= m->rtStartTime;

  if( rtCurrent >= m->rtTimestamp )
  {
    return TRUE;
  }
  return FALSE;
}

static int nvlAdvanceMovie( nvlMovie* m )
{
  HRESULT   hr;
  BOOL      bGotVideo = FALSE;

  do
  {
    DWORD             dwIndex, audStat, audSize = 0, vidStat, vidSize = 0;
    BOOL              bCanDecodeAudio = nvlFindFreePacket( m, &dwIndex );
    XMEDIAPACKET      xmpAudio = {0}, xmpVideo = {0};
    REFERENCE_TIME    rtAudio, rtVideo;
    size_t            siz;

    assert( bCanDecodeAudio );

    xmpAudio.dwMaxSize        = NVL_PACKET_SIZE;
    xmpAudio.pvBuffer         = m->pbSampleData + dwIndex * NVL_PACKET_SIZE;
    xmpAudio.pdwStatus        = &audStat;
    xmpAudio.pdwCompletedSize = &audSize;
    xmpAudio.prtTimestamp     = &rtAudio;

    siz = xmpVideo.dwMaxSize  = m->wmvVideoInfo.dwWidth * m->wmvVideoInfo.dwHeight * m->wmvVideoInfo.dwOutputBitsPerPixel / 8;
    xmpVideo.pdwStatus        = &vidStat;
    xmpVideo.pdwCompletedSize = &vidSize;
    xmpVideo.prtTimestamp     = &rtVideo;
    xmpVideo.pvBuffer         = m->pBits;
    assert( siz >= xmpVideo.dwMaxSize );

    hr = m->pWMVDecoder->ProcessMultiple( bGotVideo ? NULL : &xmpVideo, &xmpAudio );

    if( hr == S_FALSE )
    {
      m->pStream->Discontinuity();
    }

    if( audSize > 0 )
    {
      xmpAudio.dwMaxSize        = audSize;
      xmpAudio.pdwCompletedSize = NULL;
      xmpAudio.pdwStatus        = &m->adwStatus[ dwIndex ];
      xmpAudio.prtTimestamp     = NULL;
      m->pStream->Process( &xmpAudio, NULL );
    }

    if( vidSize > 0 )
    {
      m->rtTimestamp = rtVideo;
      bGotVideo = TRUE;
    }

    assert( audSize == 0 || vidSize == 0 );
    assert( hr == DS_OK || ( audSize == 0 && vidSize == 0 ) );
  } while( hr == DS_OK );

  if( hr == DS_OK || hr == E_PENDING )
  {
    return S_OK;
  }
  else
  {
    m->flags &= ~NVL_FLAGS_PLAYING;
    m->flags |= NVL_FLAGS_STOPPED;
    return S_FALSE;
  }
}

void nvlInit( void *pDSound8 )
{
  if( !nvlInitialized )
  {
    memset( &nvlSystemData, 0, sizeof(nvlSystemData) );
    if( pDSound8 )
    {
      nvlSystemData.pDSound = (LPDIRECTSOUND8)pDSound8;
    }
    else
    {
#ifdef DEBUG
      HRESULT   hr =
#endif
      DirectSoundCreate( NULL, &nvlSystemData.pDSound, NULL );
#ifdef DEBUG
      assert( hr == DS_OK );
#endif
      nvlSystemData.privateDSObject = 1;
    }
    nvlInitialized = 1;
  }
}

void nvlReset()
{
  nvlShutdown();
  nvlInit();
}

void nvlShutdown()
{
  if( nvlInitialized )
  {
    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] )
      {
        nvlMovieSource  *s = nvlSystemData.pMovies[i]->src;
        nvlStopMovie( nvlSystemData.pMovies[i] );
        if( s->refc == 0 )
        {
          nvlReleaseMovieSource( s );
        }
      }
    }
    if( nvlSystemData.privateDSObject )
    {
      nvlSystemData.pDSound->Release();
    }
    nvlInitialized = 0;
  }
}

int nvlAdvance()
{
  int     rv = 0;
  size_t  dummy;

  assert( nvlInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    nvlMovie  *m = nvlSystemData.pMovies[i];
    void      *p;

    if( m && nvlMovieReady(m) )
    {
      if( m->bFirstFrame )
      {
        m->bFirstFrame = FALSE;
        m->pStream->Pause( DSSTREAMPAUSE_RESUME );
        nvlSystemData.pDSound->GetTime( &m->rtStartTime );
      }
      p = m->putFunc( m, &dummy, m->data );
      if( p )
      {
        memcpy( p, m->pBits, m->wmvVideoInfo.dwHeight * m->pitch );
      }
      nvlAdvanceMovie( m );
      rv = 1;
    }
  }
  return rv;
}

nvlMovieSource *nvlLoadMovieSource( const char* filename, int buffer_size, void *buffer )
{
  nvlMovieSource  *rv = (nvlMovieSource*)malloc( sizeof(nvlMovieSource) );

  assert( nvlInitialized );
  assert( rv );
  assert( filename );
  memset( rv, 0, sizeof(nvlMovieSource) );
  rv->filename = (char*)malloc( strlen(filename) + 1 );
  assert( rv->filename );
  strcpy( rv->filename, filename );
  if( buffer_size == 0 )
  {
    buffer_size = NVL_NUM_PACKETS * NVL_PACKET_SIZE;
  }
  rv->bufsize = buffer_size;
  rv->buf = buffer;
  return rv;
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

void nvlReleaseMovieSource( nvlMovieSource *ms )
{
  assert( nvlInitialized );
  assert( ms );
  assert( ms->refc == 0 );
  free( ms->filename );
  free( ms );
}

nvlMovie *nvlAddMovie( nvlMovieSource* ms, nvlImageFormat format  )
{
  nvlMovie  *rv = (nvlMovie*)malloc( sizeof(nvlMovie) );

  assert( nvlInitialized );
  assert( rv );
  assert( ms );
  assert( format == NVL_IMAGE_FORMAT_YUY2 );  // WMV doesn't support other formats any more
  memset( rv, 0, sizeof(nvlMovie) );
  rv->src = ms;

  HRESULT   hr = WmvCreateDecoder( rv->src->filename, NULL, WMVVIDEOFORMAT_YUY2, NULL, &rv->pWMVDecoder );

  if( FAILED(hr) )
  {
    free( rv );
    return NULL;
  }
  ms->refc++;

  rv->bufsize = ms->bufsize;
  if( ms->buf )
  {
    rv->pbSampleData = (BYTE*)ms->buf;
  }
  else
  {
    rv->pbSampleData = (BYTE*)malloc( rv->bufsize );
  }
  assert( rv->pbSampleData );

  REFERENCE_TIME rtPreroll;

  hr = rv->pWMVDecoder->GetVideoInfo( &rv->wmvVideoInfo );
  assert( hr == DS_OK );
  hr = rv->pWMVDecoder->GetAudioInfo( &rv->wfx );
  assert( hr == DS_OK );
  hr = rv->pWMVDecoder->GetPlayDuration( &rv->rtDuration, &rtPreroll );
  assert( hr == DS_OK );
  rv->rtDuration -= rtPreroll;
  nvlSystemData.pDSound->AddRef();

  nglDev->CreateTexture (
                          rv->wmvVideoInfo.dwWidth,
                          rv->wmvVideoInfo.dwHeight,
                          0,
                          0,
                          D3DFMT_YUY2,
                          NULL,
                          &rv->pTexture
                        );

  D3DLOCKED_RECT  lr;

  rv->pTexture->LockRect( 0, &lr, NULL, 0 );
  rv->pBits = (BYTE *)lr.pBits;
  rv->pTexture->UnlockRect( 0 );
  rv->pitch = lr.Pitch;

  return rv;
}

nvlResult nvlPlayMovie( nvlMovie* m, nvlOutputBufferCallback func, void* user_data, int looping )
{
  DSSTREAMDESC    dssd = {0};
  HRESULT         hr;
  size_t          dummy;

  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] == NULL )
    {
      nvlSystemData.pMovies[i] = m;
      break;
    }
  }
  assert( nvlInitialized );
  assert( m && func );
  m->putFunc = func;
  m->data = user_data;
  dssd.dwFlags = 0;
  dssd.dwMaxAttachedPackets = NVL_NUM_PACKETS;
  dssd.lpwfxFormat = &m->wfx;
  hr = DirectSoundCreateStream( &dssd, &m->pStream );
  m->pStream->Pause( DSSTREAMPAUSE_PAUSE );
  m->bFirstFrame = TRUE;
  m->rtStartTime = 0;
  m->flags |= NVL_FLAGS_PLAYING;
  nvlAdvanceMovie( m );
  void *p = m->putFunc( m, &dummy, m->data );
  if( p )
  {
    memcpy( p, m->pBits, m->wmvVideoInfo.dwHeight * m->pitch );
  }
  return NVL_RESULT_SUCCESS;
}

void nvlStopMovie( nvlMovie* m )
{
  assert( nvlInitialized );
  m->pWMVDecoder->Release();
  m->pStream->Pause( DSSTREAMPAUSE_RESUME );
  m->pStream->Release();
  if( m->pTexture )
  {
    m->pTexture->Release();
  }
  if( m->src->buf == NULL )
  {
    assert( m->pbSampleData );
    free( m->pbSampleData );
  }
  m->src->refc--;
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] == m )
    {
      nvlSystemData.pMovies[i] = NULL;
    }
  }
  free( m );
}

nvlResult nvlPauseMovie( nvlMovie* )
{
  assert( 0 && "Not implemented" );
  return NVL_RESULT_ERROR;
}

nvlResult nvlUnpauseMovie( nvlMovie* )
{
  assert( 0 && "Not implemented" );
  return NVL_RESULT_ERROR;
}

nvlResult nvlMovieStatus( const nvlMovie* m )
{
  nvlResult   rv;

  assert( nvlInitialized );
  assert( m );
  if( m->flags & NVL_FLAGS_PLAYING )
  {
    rv = NVL_RESULT_PLAYING;
  }
  else if( m->flags & NVL_FLAGS_STOPPED )
  {
    rv = NVL_RESULT_NONE;
  }
  else
  {
    rv = NVL_RESULT_ERROR;
  }
  return rv;
}

int nvlMovieGetWidth( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->wmvVideoInfo.dwWidth;
}

int nvlMovieGetHeight( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->wmvVideoInfo.dwHeight;
}

int nvlMovieGetFrameNum( const nvlMovie* )
{
  assert( nvlInitialized );
  return 0;
}

int nvlMovieGetFrame( const nvlMovie* )
{
  assert( nvlInitialized );
  return 0;
}

void nvlStopAllMovies()
{
  assert( nvlInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] )
    {
      nvlStopMovie( nvlSystemData.pMovies[i] );
    }
  }
}

void nvlPauseAllMovies()
{
  assert( 0 && "Not implemented" );
}

void nvlUnpauseAllMovies()
{
  assert( 0 && "Not implemented" );
}

REFERENCE_TIME nvlMovieGetTime( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->rtTimestamp;
}

REFERENCE_TIME nvlMovieGetDuration( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->rtDuration;
}

void nvlMovieSetVolume( nvlMovie* m, float volume )
{
  assert( nvlInitialized );
  assert( m && m->pStream );
  LONG  vol = ( volume > 0 )? (LONG)( 4342.9448 * log10(volume) ) : DSBVOLUME_MIN;
  if( vol > DSBVOLUME_MAX )
  {
    vol = DSBVOLUME_MAX;
  }
  else if( vol < DSBVOLUME_MIN )
  {
    vol = DSBVOLUME_MIN;
  }
  m->pStream->SetVolume( vol );
}

#elif defined(NVL_USE_BINK)   // BINK compression

#include "bink.h"

enum
{
  NVL_FLAG_INVALID    = 0,
  NVL_FLAG_LOOPING    = 0x00000001,
  NVL_FLAG_PLAYING    = 0x00000002,
  NVL_FLAG_PAUSED     = 0x00000004
};

struct nvlMovieSource
{
  char                      *filename;
  int                       refc;
};

struct nvlMovie
{
  nvlMovieSource            *src;
  HBINK                     bink;
  nvlOutputBufferCallback   putFunc;
  void                      *data;
  unsigned int              flags;
  u32                       format;
};

static const u32  nvlFormatTable[] =  // Keep in sync with nvlImageFormat enum
{
  BINKSURFACE8P,
  BINKSURFACE24,
  BINKSURFACE24R,
  BINKSURFACE32,
  BINKSURFACE32R,
  BINKSURFACE32A,
  BINKSURFACE32RA,
  BINKSURFACE4444,
  BINKSURFACE5551,
  BINKSURFACE555,
  BINKSURFACE565,
  BINKSURFACE655,
  BINKSURFACE664,
  BINKSURFACEYUY2,
  BINKSURFACEUYVY,
  BINKSURFACEYV12,
  BINKSURFACEMASK,
};

static struct nvlSystemData_t
{
  nvlMovie                  *pMovies[NVL_MAX_MOVIE_NUM];
  LPDIRECTSOUND8            pDSound;
  int                       privateDSObject;
}             nvlSystemData;
static int    nvlInitialized = 0;

void nvlInit( void *pDSound8 )
{
  if( !nvlInitialized )
  {
    memset( &nvlSystemData, 0, sizeof(nvlSystemData) );
    nvlSystemData.pDSound = (IDirectSound8*)pDSound8;

    if( nvlSystemData.pDSound == NULL )
    {
      DirectSoundCreate( NULL, &nvlSystemData.pDSound, NULL );
      nvlSystemData.privateDSObject = 1;
    }
    BinkSoundUseDirectSound( nvlSystemData.pDSound );
    nvlInitialized = 1;
  }
}

void nvlReset()
{
  nvlShutdown();
  nvlInit();
}

void nvlShutdown()
{
  if( nvlInitialized )
  {
    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] )
      {
        nvlMovieSource  *s = nvlSystemData.pMovies[i]->src;
        nvlStopMovie( nvlSystemData.pMovies[i] );
        if( s->refc == 0 )
        {
          nvlReleaseMovieSource( s );
        }
      }
    }
    if( nvlSystemData.privateDSObject )
    {
      nvlSystemData.pDSound->Release();
    }
    nvlInitialized = 0;
  }
}

nvlMovieSource *nvlLoadMovieSource( const char* filename, int buffer_size, void *buffer )
{
  WIN32_FIND_DATA   find;
  HANDLE            find_handle;
  nvlMovieSource    *rv = NULL;

  assert( nvlInitialized );
  assert( filename );
  if( (find_handle = FindFirstFile(filename, &find)) != INVALID_HANDLE_VALUE )
  {
    rv = (nvlMovieSource*)malloc( sizeof(nvlMovieSource) );
    assert( rv );
    memset( rv, 0, sizeof(nvlMovieSource) );
    rv->filename = (char*)malloc( strlen(filename) + 1 );
    assert( rv->filename );
    strcpy( rv->filename, filename );
  }
  FindClose( find_handle );
  return rv;
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

void nvlReleaseMovieSource( nvlMovieSource *ms )
{
  assert( nvlInitialized );
  assert( ms );
  assert( ms->refc == 0 );
  free( ms->filename );
  free( ms );
}

nvlMovie *nvlAddMovie( nvlMovieSource* s, nvlImageFormat format )
{
  assert( nvlInitialized );
  assert( s );
  assert( format >= NVL_IMAGE_FORMAT_8P && format <= NVL_IMAGE_FORMAT_YV12 );

  nvlMovie  *rv = (nvlMovie*)malloc( sizeof(nvlMovie) );

  assert( rv );
  memset( rv, 0, sizeof(nvlMovie) );
  rv->src = s;
  rv->format = nvlFormatTable[format];
  rv->bink = BinkOpen( s->filename, 0 );
  if( !rv->bink )
  {
    free( rv );
    rv = NULL;
  }
  BinkPause( rv->bink, 1 );
  rv->flags |= NVL_FLAG_PAUSED;
  s->refc++;
  return rv;
}

nvlResult nvlPlayMovie( nvlMovie* m, nvlOutputBufferCallback func, void* user_data, int looping )
{
  assert( nvlInitialized );
  assert( m && func );
  m->putFunc = func;
  m->data = user_data;
  if( looping )
  {
    m->flags |= NVL_FLAG_LOOPING;
  }
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] == NULL )
    {
      nvlSystemData.pMovies[i] = m;
      break;
    }
  }
  BinkPause( m->bink, 0 );
  m->flags &= ~NVL_FLAG_PAUSED;
  m->flags |= NVL_FLAG_PLAYING;
  return NVL_RESULT_SUCCESS;
}

static void nvlAdvanceMovie( nvlMovie* m )
{
  void    *outp_buf;
  size_t  pitch;

  assert( nvlInitialized );
  assert( m );
  BinkDoFrame( m->bink );
  outp_buf = (*m->putFunc)( m, &pitch, m->data );
  assert( outp_buf && pitch );
  BinkCopyToBuffer( m->bink, outp_buf, pitch, m->bink->Height, 0, 0, m->format );
//  BinkCopyToBuffer( m->bink, outp_buf, pitch, m->bink->Height, 0, 0, m->format | BINKCOPYALL );

	if ((m->flags & NVL_FLAG_LOOPING) || (m->bink->Frames != m->bink->LastFrameNum))
	  BinkNextFrame( m->bink );
	else
	{
    m->flags &= ~NVL_FLAG_PLAYING;
    m->flags |= NVL_FLAG_PAUSED;
	}
}

int nvlAdvance()
{
  int   rv = 0;

  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    nvlMovie  *m = nvlSystemData.pMovies[i];

    if( m && (m->flags & NVL_FLAG_PLAYING) && !BinkWait(m->bink) )
    {
      rv = 1;
      nvlAdvanceMovie( m );
    }
  }
  return rv;
}

int nvlMovieGetFrameNum( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->bink->Frames;
}

int nvlMovieGetFrame( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->bink->FrameNum;
}

void nvlStopMovie( nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  BinkClose( m->bink );
  m->src->refc--;
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] == m )
    {
      nvlSystemData.pMovies[i] = NULL;
    }
  }
  free( m );
}

nvlResult nvlMovieStatus( const nvlMovie* m )
{
  nvlResult   rv;

  assert( nvlInitialized );
  assert( m );
  if( m->flags & NVL_FLAG_PLAYING )
  {
    rv = NVL_RESULT_PLAYING;
  }
  else if( m->flags & NVL_FLAG_PAUSED )
  {
    rv = NVL_RESULT_PAUSED;
  }
  else
  {
    rv = NVL_RESULT_ERROR;
  }
  return rv;
}

int nvlMovieGetWidth( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->bink->Width;
}

int nvlMovieGetHeight( const nvlMovie* m )
{
  assert( nvlInitialized );
  assert( m );
  return m->bink->Height;
}

void nvlStopAllMovies()
{
  assert( nvlInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] )
    {
      nvlStopMovie( nvlSystemData.pMovies[i] );
    }
  }
}

void nvlPauseAllMovies()
{
  assert( nvlInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] )
    {
      nvlPauseMovie( nvlSystemData.pMovies[i] );
    }
  }
}

void nvlUnpauseAllMovies()
{
  assert( nvlInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] )
    {
      nvlUnpauseMovie( nvlSystemData.pMovies[i] );
    }
  }
}

REFERENCE_TIME nvlMovieGetTime( const nvlMovie* m )
{
  assert( m );
  return (REFERENCE_TIME)( (float)nvlMovieGetFrame(m) / (30.0f * 100e-9) );  // I'm assuming 30 FPS
}

REFERENCE_TIME nvlMovieGetDuration( const nvlMovie* m )
{
  assert( m );
  return (REFERENCE_TIME)( (float)nvlMovieGetFrameNum(m) / (30.0f * 100e-9) );  // I'm assuming 30 FPS
}

nvlResult nvlPauseMovie( nvlMovie* m )
{
  nvlResult rv = NVL_RESULT_ERROR;

  if( m )
  {
    if( m->flags & NVL_FLAG_PLAYING )
    {
      BinkPause( m->bink, 1 );
      rv = NVL_RESULT_PAUSED;
    }
    else if( m->flags & NVL_FLAG_PAUSED )
    {
      rv = NVL_RESULT_SUCCESS;
    }
  }
  return rv;
}

nvlResult nvlUnpauseMovie( nvlMovie* m )
{
  nvlResult rv = NVL_RESULT_ERROR;

  if( m )
  {
    if( m->flags & NVL_FLAG_PAUSED )
    {
      BinkPause( m->bink, 0 );
      rv = NVL_RESULT_PLAYING;
    }
    else if( m->flags & NVL_FLAG_PLAYING )
    {
      rv = NVL_RESULT_SUCCESS;
    }
  }
  return rv;
}

void nvlMovieSetVolume( nvlMovie* m, float volume )
{
  assert( nvlInitialized );
  assert( m );
  assert( volume >= 0 && volume <= 1.0f );
  BinkSetVolume( m->bink, 0, s32(volume * 32768.0f) );
}

#else
#error Implementation method must be defined.
#endif