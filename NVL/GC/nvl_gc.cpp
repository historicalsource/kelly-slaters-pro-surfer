#include <string.h>
#include <assert.h>
#include <dolphin.h>

#include "nvlstream_gc.h"
#include "nvl_gc.h"
extern "C"
{
  #include "hvqm4dec.h"
  #include "hvqm4audio.h"
}

#define NVL_MAX_LUMI                  255
#define NVL_HLF_LUMI                  128
#define NVL_Y_LUMI                    16
#define NVL_COLOR_CLIP_OFFSET         128
#define NVL_COLOR_CLIP_TABLE          (NVL_COLOR_CLIP_OFFSET + NVL_MAX_LUMI + 1 + NVL_COLOR_CLIP_OFFSET)
#define NVL_COLOR_SHIFT               6
#define NVL_COLOR_OFFSET              (((s16)(1 << (NVL_COLOR_SHIFT - 1))) + (NVL_COLOR_CLIP_OFFSET << NVL_COLOR_SHIFT))
#define NVL_FIX(x)                    ((s16)((x) * (1 << NVL_COLOR_SHIFT) + 0.5))
#define NVL_CONV_RV                   ( NVL_FIX(1.596))
#define NVL_CONV_GU                   (-NVL_FIX(0.391))
#define NVL_CONV_GV                   (-NVL_FIX(0.813))
#define NVL_CONV_BU                   ( NVL_FIX(2.018))
#define NVL_CONV_Y                    ( NVL_FIX(1.164))
#define NVL_CLIP(x)                   (nvlClipTable[( x ) >> NVL_COLOR_SHIFT])

#define NVL_AI_DMA_SIZE               4096
#define NVL_MOVIE_VOLUME              64

#define NVL_DECODE_THREAD_STACK_SIZE  8192

enum
{
  NVL_FLAG_NONE	        = 0,
  NVL_FLAG_PLAYING      = 0x00000001,
  NVL_FLAG_PAUSED       = 0x00000002,
  NVL_FLAG_PLAY_AUDIO   = 0x00000004,
  NVL_FLAG_PLAY_REPEAT  = 0x00000008
};

enum
{
  NVL_STATUS_NONE	      = 0,
  NVL_STATUS_PLAY	      = 1<<0,
  NVL_STATUS_PAUSE      = 1<<1,
  NVL_STATUS_STOP	      = 1<<2,
  NVL_STATUS_START      = 1<<3,
  NVL_STATUS_OPEN       = 1<<4,
  NVL_STATUS_READERROR  = 1<<29,
  NVL_STATUS_ERRORFLAG  = 1<<30
};

struct nvlMovieSource
{
  int           refs;
  char          *filename;
};

#define HVQM4_DEC_BUFFER_CNT	6
#define HVQM4_BPIC_BUFFER_START	2
#define DMA_BLOCK_SIZE		4096

#define READ_PRI    (8)
#define DECODE_PRI  (17)

typedef struct
{
  u8    *buf;
  u32   frame;
  u8    state;
  u8    reserve[3];
} HVQM4PlayerData;

struct nvlMovie
{
  HVQM4Header               sHeader ATTRIBUTE_ALIGN(32); 	       /* HVQM4MovieHeader */
  HVQM4GOPHeader            sGopHdr ATTRIBUTE_ALIGN(32);	       /* Current GOP header */
  HVQM4RecHeader            sRecHeader ATTRIBUTE_ALIGN(32);      /* Current record header */
  HVQM4SeqObj	              sSeqObj ATTRIBUTE_ALIGN(32);	       /* HVQM4 sequence buffer */
  ADPCMstate	              sAdpcmState[2] ATTRIBUTE_ALIGN(32);  /* Audio decord state */
  HVQM4PlayerData           sPlayerData[HVQM4_DEC_BUFFER_CNT];   /* Decoding buffer */
  HVQM4PlayerData           *psPlayerData[HVQM4_DEC_BUFFER_CNT]; /* Decoding buffer pointer */
  u8		                    *pSoundBuf;	    /* Sound buffer pointer */
  u8		                    *pSoundBufPtr;    /* Current sound buffer pointer */
  u8		                    *pSoundMaxPtr;    /* Sound buffer EOF pointer */
  u8		                    *pSoundDecPtr;    /* Sound buffer EOF pointer for decoding */
  u8		                    *pPicReadBuf;	    /* Temporary buffer pointer for picture data */
  u8		                    *pSoundReadBuf;   /* Temporary buffer pointer for sound data */

  u32		                    ulGopCount;
  u32		                    ulGopTopFrame;    /* First frame in GOP */
  u32		                    ulCurrentFrame;   /* Current frame number */
  u32		                    ulDecodeFrame;    /* The number of decoded frames */
  u32		                    ulDecodeTotal;    /* The total time of decoding */
  u32		                    ulDecodePicCnt;   /* The number of decoded images */
  u32		                    ulDecodeAuCnt;    /* The number of decoded audio */
  OSTime	                  ulStartTime;	    /* The timing of display */
  OSTime	                  ulPauseTime;	    /* The timing of pause */
  u64		                    ullAudioIndex;    /* Audio count */

  void	                    (*funcAudioDecodeEnd)(u8 *pAudio, u32 ulSize);  /* Functions to be called at the termination of decoding sound. */
  void	                    (*funcDrawPicture)(u32 ulCurrentFrame);  /* Functions to be called during drawing. */
  
  u8		                    cStatus; 	    /* Playback status */
  u8		                    cFlags; 	    /* Playback flags */
  u8		                    bDecode;	    /* decode flags */
  u8		                    cAudioSample;   /* Number of audio samples per second */
  u32		                    reserved1; 	    /* Reserved */
  void                      *userData;      /* Reserved for nvl */
//----------------------------------------------------------------------------------------------------
  u8                        hvqmflags;
  void                      *pBuffer;
  nvlMovieSource            *src;
  unsigned int              flags;
  nvlImageFormat            format;
  nvlOutputBufferCallback   putFunc;
  void                      *putFunc_data;
  u8                        *SoundBuffer[2];
  u32                       CurrentSoundBuffer;
  int                       top_clip,
                            left_clip,
                            width,
                            height;
  nvlStream                 *pStream;
};

#ifndef NGL
extern void*  nglMemAlloc ( unsigned int Size, unsigned int Align = 1 );
extern void   nglMemFree  ( void* Ptr );
#endif

static struct nvlSystemData_t
{
  u8            nvlAudioNullBuf[NVL_AI_DMA_SIZE] __attribute__((aligned (32)));
  nvlMovie      *pMovies[NVL_MAX_MOVIE_NUM];
  u8            volume;
  int           fb_width, fb_height;
  volatile int	decodeThreadIsRunning;
  OSThread      decodeThread;
  u8            *decodeThreadStack;
  volatile int  decodeThreadRunningFlag;
  AIDCallback   nvlOldAICallback;
  int           priority_movie;
} nvlSystemData;

static int  nvlSystemInitialized = 0, nvlHVQMDecoderInitialized = 0;
static u8   nvlClipTable[NVL_COLOR_CLIP_TABLE];

static const int  nvlVolumeShift = 16;
static const u16  nvlVolumeTable[128] =
{
    511,  1023,  1535,  2047,  2559,  3071,  3583,  4095,  4607,  5119,  5631,  6143,  6655,  7167,  7679,  8191,
   8703,  9215,  9727, 10239, 10751, 11263, 11775, 12287, 12799, 13311, 13823, 14335, 14847, 15359, 15871, 16383,
  16895, 17407, 17919, 18431, 18943, 19455, 19967, 20479, 20991, 21503, 22015, 22527, 23039, 23551, 24063, 24575,
  25087, 25599, 26111, 26623, 27135, 27647, 28159, 28671, 29183, 29695, 30207, 30719, 31231, 31743, 32255, 32767,
  33279, 33791, 34303, 34815, 35327, 35839, 36351, 36863, 37375, 37887, 38399, 38911, 39423, 39935, 40447, 40959,
  41471, 41983, 42495, 43007, 43519, 44031, 44543, 45055, 45567, 46079, 46591, 47103, 47615, 48127, 48639, 49151,
  49663, 50175, 50687, 51199, 51711, 52223, 52735, 53247, 53759, 54271, 54783, 55295, 55807, 56319, 56831, 57343,
  57855, 58367, 58879, 59391, 59903, 60415, 60927, 61439, 61951, 62463, 62975, 63487, 63999, 64511, 65023, 65535
};

/*
static nvlMovie *nvlNextDecode()
{
  nvlMovie  *rv;

  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    rv = nvlSystemData.pMovies[i];
    if( rv )
    {
      break;
    }
  }
  return rv;
}
*/

static nvlMovie *nvlNextDecode()
{
  static int  last = -1;
  int         rv = nvlSystemData.priority_movie, i;

  assert( nvlSystemInitialized );
  if( rv < 0 )
  {
    for( i = (last >= 0? last + 1 : 0);; ++i >= NVL_MAX_MOVIE_NUM? (i = 0) : i )
    {
      nvlMovie *m = nvlSystemData.pMovies[i];

      if( m && (m->flags & NVL_FLAG_PLAYING) )
      {
        rv = i;
        break;
      }
      if( i == last )
      {
        break;
      }
    }
  }
  else
  {
    nvlSystemData.priority_movie = -1;
  }
  return nvlSystemData.pMovies[last = rv];
}

static int nvlDecodeVideo( nvlMovie *pobj, u32 wFormat, u32 dwFrame )
{
  u32               dwCurrentFrame,
                    dwPrevFrame = ( !pobj->ulDecodeFrame )? 0 : (u32)( (pobj->ulDecodeTotal / pobj->ulDecodeFrame) / pobj->sHeader.usec_per_frame );
  HVQM4PlayerData   *tmp;
  u8                cnt;
  int               flag = 0;

  if( pobj->cFlags & NVL_FLAG_PLAY_AUDIO )
  {
    dwCurrentFrame = ( !pobj->ullAudioIndex )? 0 : (u32)( (pobj->ullAudioIndex * 1000000 / (4 * pobj->sHeader.audioinfo.samples_per_sec)) / pobj->sHeader.usec_per_frame );
  }
  else
  {
    dwCurrentFrame = ( !pobj->ulStartTime )? 0 : (u32)( OSTicksToMicroseconds(OSGetTime() - pobj->ulStartTime) / pobj->sHeader.usec_per_frame );
  }
  dwPrevFrame = ( dwCurrentFrame ) ? dwPrevFrame + 1 : 0;

  switch( wFormat )
  {
  case HVQM4_VIDEO_I_PIC:
  case HVQM4_VIDEO_P_PIC:
    for( cnt = HVQM4_BPIC_BUFFER_START ; cnt < HVQM4_DEC_BUFFER_CNT ; cnt++ )
    {
      if( pobj->psPlayerData[cnt]->state != 1 || dwCurrentFrame > pobj->psPlayerData[cnt]->frame )
      {
        tmp = pobj->psPlayerData[cnt];
        pobj->psPlayerData[cnt] = pobj->psPlayerData[0];
        pobj->psPlayerData[0] = pobj->psPlayerData[1];
        pobj->psPlayerData[1] = tmp;
        flag = 1;
        break;
      }
    }
    if( !flag )
    {
      return 2;
    }
    pobj->psPlayerData[1]->state = 0;
    if( dwCurrentFrame + dwPrevFrame > dwFrame + pobj->ulGopTopFrame )
    {
      return 3;
    }
    if( wFormat == HVQM4_VIDEO_I_PIC )
    {
      HVQM4DecodeIpic( &pobj->sSeqObj, pobj->pPicReadBuf, pobj->psPlayerData[1]->buf );
    }
    else
    {
	    if( !pobj->psPlayerData[0]->state )
	    {
        return 3;
      }
      HVQM4DecodePpic( &pobj->sSeqObj, pobj->pPicReadBuf, pobj->psPlayerData[1]->buf, pobj->psPlayerData[0]->buf );
    }
    pobj->psPlayerData[1]->frame = dwFrame + pobj->ulGopTopFrame;
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    pobj->psPlayerData[1]->state = 1;
    break;
  case HVQM4_VIDEO_B_PIC:
    if( !pobj->psPlayerData[0]->state || !pobj->psPlayerData[1]->state || dwCurrentFrame + dwPrevFrame > dwFrame + pobj->ulGopTopFrame )
    {
      return 3;
    }
    for( cnt = HVQM4_BPIC_BUFFER_START ; cnt < HVQM4_DEC_BUFFER_CNT ; cnt++ )
    {
      if( pobj->psPlayerData[cnt]->state != 1 || dwCurrentFrame > pobj->psPlayerData[cnt]->frame )
      {
        pobj->psPlayerData[cnt]->state = 0;
        HVQM4DecodeBpic( &pobj->sSeqObj, pobj->pPicReadBuf, pobj->psPlayerData[cnt]->buf, pobj->psPlayerData[0]->buf, pobj->psPlayerData[1]->buf );
        pobj->psPlayerData[cnt]->frame = dwFrame + pobj->ulGopTopFrame;
        if( pobj->cStatus == NVL_STATUS_STOP )
        {
          return 0;
        }
        pobj->psPlayerData[cnt]->state = 1;
        return 0;
      }
    }
    return 2;
  default :
    return 1;
  }
  return 0;
}

static int nvlDecode( nvlMovie *pobj )
{
  u16		  wType, wFormat;
  u32		  dwSize, dwFrame, dwSamples, diff;
  s32		  ret = 0;
  OSTime	time,time2;
  
  pobj->bDecode = 1;
  if( pobj->ulDecodePicCnt == pobj->sGopHdr.vidrec_number && pobj->ulDecodeAuCnt == pobj->sGopHdr.audrec_number )
  {
    pobj->ulGopTopFrame += pobj->sGopHdr.vidrec_number;
    pobj->ulGopCount++;
    if( pobj->ulGopCount < pobj->sHeader.gop_total )
    {
      if( pobj->cStatus == NVL_STATUS_STOP )
      {
        return 0;
      }
      ret = nvlStreamGetCopy( pobj->pStream, &pobj->sGopHdr, sizeof(HVQM4GOPHeader) );
      if( ret == -1 )
      {
        return 1;
      }
      pobj->ulDecodePicCnt = 0;
      pobj->ulDecodeAuCnt = 0;
    }
    else
    {
      return 2;
    }
  }

  ret = nvlStreamGetCopy( pobj->pStream, &pobj->sRecHeader, sizeof(HVQM4RecHeader) );
  if( ret == -1 )
  {
    return 1;
  }
  wType = pobj->sRecHeader.type;
  wFormat = pobj->sRecHeader.format;
  dwSize = pobj->sRecHeader.size;

  if( wType == HVQM4_VIDEO )
  {
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    ret = nvlStreamGetCopy( pobj->pStream, pobj->pPicReadBuf, sizeof(u32) );
    if( ret == -1 )
    {
      return 1;
    }
    dwFrame = ( (u32*)pobj->pPicReadBuf )[0];
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    ret = nvlStreamGetCopy( pobj->pStream, pobj->pPicReadBuf, (s32)(dwSize - 4) );
    if( ret == -1 )
    {
      return 1;
    }

DECODERETRY:
    time = OSGetTime();
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0; 
    }
    switch( nvlDecodeVideo(pobj, wFormat, dwFrame) )
    {
    case 1:
      return 1;
    case 2:
      goto DECODERETRY;
    case 3:
#ifdef _DEBUG
      OSReport( "Skip: decode failed\n" );
#endif
      break;
    default:
      if( pobj->cStatus == NVL_STATUS_STOP )
      {
        return 0;
      }
      time2 = OSGetTime();
      pobj->ulDecodeTotal += OSTicksToMicroseconds( time2 - time );
      pobj->ulDecodeFrame++;
    }
    pobj->ulDecodePicCnt++;
  }
  else if( wType == HVQM4_AUDIO )
  {
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    ret = nvlStreamGetCopy( pobj->pStream, pobj->pSoundReadBuf, sizeof(u32) );
    if( ret == -1 )
    {
      return 1;
    }
    dwSamples = ( (u32*)pobj->pSoundReadBuf )[0];
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    ret = nvlStreamGetCopy( pobj->pStream, pobj->pSoundReadBuf, (s32)(dwSize - 4) );
    if( ret == -1 )
    {
      return 1;
    }
    diff = (u32)( pobj->pSoundDecPtr + dwSamples * 4 );

DECODERETRY2:
    if( diff >= (u32)pobj->pSoundMaxPtr )
    {
      diff -= ( u32 )( pobj->pSoundMaxPtr - pobj->pSoundBuf );
    }
    if( (u32)pobj->pSoundDecPtr < (u32)pobj->pSoundBufPtr && (u32)diff >= (u32)pobj->pSoundBufPtr )
    {
      goto DECODERETRY2;
    }

    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    HVQM4ADPCMDecode( pobj->pSoundReadBuf, pobj->pSoundDecPtr, pobj->sHeader.audioinfo.channels, wFormat, dwSamples, pobj->sAdpcmState );
    if( (u32)pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr < (u32)pobj->pSoundDecPtr )
    {
      dwSize = (u32)pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr + (u32)( pobj->pSoundMaxPtr - pobj->pSoundBuf ) - (u32)pobj->pSoundDecPtr;
    }
    else
    {
      dwSize = (u32)pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr - (u32)pobj->pSoundDecPtr;
    }
    if( pobj->funcAudioDecodeEnd )
    {
      pobj->funcAudioDecodeEnd( pobj->pSoundDecPtr,dwSize );
    }
    if( pobj->cStatus == NVL_STATUS_STOP )
    {
      return 0;
    }
    pobj->pSoundDecPtr = (u8*)pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr;
    pobj->ulDecodeAuCnt++;
  }
  else
  {
    return 1;
  }
  return 0;
}

static void *nvlDecodeThreadFunc( void *param )
{
#pragma unused( param )
  int       j, flag = 0, ret = 0;
  nvlMovie  *pobj = NULL;
  
  nvlSystemData.decodeThreadIsRunning = 1;
  while( nvlSystemData.decodeThreadIsRunning > 0 )
  {
    pobj = nvlNextDecode();
    if( !pobj ) continue;
    if( !( pobj->cStatus & NVL_STATUS_PLAY ) ) continue;
    ret = nvlDecode( pobj );
    if( ret == 2 )
    {
      if( pobj->cFlags & NVL_FLAG_PLAY_REPEAT )
      {
        do
        {
	        if( pobj->cFlags & NVL_FLAG_PLAY_AUDIO )
	        {
            pobj->ulCurrentFrame = ( u32 )( ( ( pobj->ullAudioIndex * 1000000 / ( 4 * pobj->sHeader.audioinfo.samples_per_sec ) ) + OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) ) / pobj->sHeader.usec_per_frame );
          }
          else
          {
            pobj->ulCurrentFrame = ( u32 )( OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) / pobj->sHeader.usec_per_frame );
          }
        } while( pobj->ulCurrentFrame < pobj->sHeader.video_total );
        for( j = 0 ; j < HVQM4_DEC_BUFFER_CNT ; j++ )
        {
          pobj->sPlayerData[j].state = 0;
        }
        if( nvlStreamGetCopy( pobj->pStream, (void*)&pobj->sHeader,(s32)sizeof(HVQM4Header) ) == -1 )
        {
#ifdef _DEBUG
          OSReport( "Error: broken file.\n" );
#endif
          pobj->cStatus = NVL_STATUS_STOP;
          pobj->bDecode = 0;
          continue;
        }
        if( nvlStreamGetCopy( pobj->pStream, (void*)&pobj->sGopHdr,( s32 )sizeof( HVQM4GOPHeader ) ) == -1 )
        {
#ifdef _DEBUG
	        OSReport( "Error: broken file.\n" );
#endif
          pobj->cStatus = NVL_STATUS_STOP;
          pobj->bDecode = 0;
          continue;
        }
	      pobj->cStatus |= NVL_STATUS_START;
        pobj->ulGopCount = 0;
        pobj->ulGopTopFrame = 0;
        pobj->ulDecodePicCnt = 0;
      	pobj->ulDecodeAuCnt = 0;
      	pobj->ulDecodeTotal = 0;
      	pobj->ulDecodeFrame = 0;
      	pobj->ulStartTime = 0;
      	pobj->ullAudioIndex = 0;
      	pobj->pSoundBufPtr = pobj->pSoundDecPtr = pobj->pSoundBuf;
      }
      else
      {
        do
        {
          if( pobj->cFlags & NVL_FLAG_PLAY_AUDIO )
          {
            pobj->ulCurrentFrame = ( u32 )( ( ( pobj->ullAudioIndex * 1000000 / ( 4 * pobj->sHeader.audioinfo.samples_per_sec ) ) + OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) ) / pobj->sHeader.usec_per_frame );
          }
          else
          {
            pobj->ulCurrentFrame = ( u32 )( OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) / pobj->sHeader.usec_per_frame );
	        }
        } while( pobj->ulCurrentFrame < pobj->sHeader.video_total );

        nvlStreamRewind( pobj->pStream );
        while( !nvlStreamIsReady(pobj->pStream) ){};
	      pobj->cStatus = NVL_STATUS_STOP;
      }
    }
    else if( ret == 1 )
    {
#ifdef _DEBUG
      OSReport( "Error: broken file.\n" );
#endif
      pobj->cStatus = NVL_STATUS_STOP;
    }
    pobj->bDecode = 0;
  }
  if( pobj )
  {
    pobj->bDecode = 0;
  }
  nvlSystemData.decodeThreadIsRunning = 0;
  return NULL;
}

static void nvlkMoviePlayInternal( nvlMovie *m, u8 flags )
{
  int ret;

  assert( nvlSystemInitialized );
  assert( m );

  if( m->cStatus == NVL_STATUS_NONE || m->cStatus == NVL_STATUS_STOP )
  {
    m->cFlags = flags;

    if( m->cStatus == NVL_STATUS_STOP )
    {
      ret = nvlStreamGetCopy( m->pStream, NULL, (s32)sizeof(HVQM4Header) );
      assert( ret == (s32)sizeof(HVQM4Header) );
    }
    ret = nvlStreamGetCopy( m->pStream, (void*)&m->sGopHdr, (s32)sizeof(HVQM4GOPHeader) );
    assert( ret == (s32)sizeof(HVQM4GOPHeader) );
    if( (flags & NVL_FLAG_PLAY_AUDIO) && m->cAudioSample == 0xff )
    {
      m->cFlags = (u8)( m->cFlags - NVL_FLAG_PLAY_AUDIO );
    }
    m->ulGopCount = 0;
    m->ulGopTopFrame = 0;
    m->ulDecodePicCnt = 0;
    m->ulDecodeAuCnt = 0;
    m->ulDecodeTotal = 0;
    m->ulDecodeFrame = 0;
    m->ulStartTime = 0;
    m->ullAudioIndex = 0;
    m->pSoundBufPtr = m->pSoundDecPtr = m->pSoundBuf;

    m->cStatus = NVL_STATUS_PLAY | NVL_STATUS_START;
  }
  else if( m->cStatus & NVL_STATUS_PAUSE )
  {
    m->cStatus = NVL_STATUS_PLAY;
    m->ulStartTime += OSGetTime() - m->ulPauseTime;
  }
}

static u32 nvlMovieStatusInternal( nvlMovie *m )
{
  u32   rv = NVL_STATUS_NONE;

  assert( m );
  if( m->cStatus & NVL_STATUS_STOP )
  {
    rv = (u32)NVL_STATUS_STOP;
  }
  switch( nvlStreamGetErrorStatus(m->pStream) )
  {
  case DVD_STATE_FATAL_ERROR  :
    rv = (u32)( NVL_STATUS_ERRORFLAG | NVL_STATUS_READERROR );
  case DVD_STATE_IGNORED      :
  case DVD_STATE_END          :
    break;
  default                     :
    rv = (u32)( NVL_STATUS_ERRORFLAG | nvlStreamGetErrorStatus(m->pStream) );
  }
  if( m->cStatus & NVL_STATUS_PAUSE )
  {
    rv = (u32)( m->cStatus & (NVL_STATUS_PAUSE | NVL_STATUS_OPEN) );
  }
  if( m->cStatus & NVL_STATUS_PLAY )
  {
    rv = (u32)NVL_STATUS_PLAY;
  }
  return rv;
}

static int nvlPauseMovieInternal( nvlMovie *m )
{
  assert( nvlSystemInitialized );
  assert( m );
  if( !(m->cStatus & NVL_STATUS_PLAY) )
  {
    return 1;
  }
  m->cStatus = NVL_STATUS_PAUSE;
  m->ulPauseTime = OSGetTime();
  return 0;
}

/*
static inline u8 do_col(s32 col)
{ 
  col >>= NVL_COLOR_SHIFT;
  col-=128;
  if(col<0)     col=0; else
  if(col>255)   col=255;
  
  return (u8)col;
}
*/

static inline void nvlPack32( unsigned char* pix, s32 r, s32 g, s32 b )
{
  *pix        = NVL_CLIP( 0xff );
  *(pix +  1) = NVL_CLIP( r );
  *(pix + 32) = NVL_CLIP( g );
  *(pix + 33) = NVL_CLIP( b );
/*
  *pix        = 0xff;
  *(pix +  1) = do_col(r);
  *(pix + 32) = do_col(g);
  *(pix + 33) = do_col(b);
*/
}

static inline void nvlPack16( void* pix, s32 r, s32 g, s32 b, int type )
{
  u16   *ptr = (u16*)pix, tmp;

  if( type == GX_TF_RGB565 )
  {
    tmp = GXPackedRGB565( NVL_CLIP(r), NVL_CLIP(g), NVL_CLIP(b) );
  }
  else if( type == GX_TF_RGB5A3 )
  {
    tmp = GXPackedRGB5A3( NVL_CLIP(r), NVL_CLIP(g), NVL_CLIP(b), 0xff );
  }
  *ptr = tmp;
}

static inline void nvlYUV411_YUV422 (
                                      u8 *pSrc,
                                      u8 *pDst,
                                      int top,
                                      int left,
                                      int outputWidth,
                                      int outputHeight,
                                      int imageWidth,
                                      int imageHeight,
                                      int frambuf_width,
                                      int frambuf_height
                                    )
{
  int   px, py, start_x, end_x, start_y, end_y;
  u32   u, v, *xfbptr, *xfbptr1;
  u8    *yP1, *yP2, *uP, *vP;

  if( top + outputHeight > frambuf_height )
  {
    outputHeight = frambuf_height - top;
  }
  if( outputHeight > imageHeight )
  {
    outputHeight = imageHeight;
  }
  if( left + outputWidth > frambuf_width )
  {
    outputWidth = frambuf_width - left;
  }
  if( outputWidth > imageWidth ) outputWidth = imageWidth;

  start_y = ( top < 0 )? -top : 0;
  end_y = ( imageHeight < outputHeight )? imageHeight : outputHeight;
  if( end_y - start_y > frambuf_height - top )
  {
    end_y = frambuf_height - top + start_y;
  }
  start_x = ( left < 0 )? -left : 0;
  end_x = ( imageWidth < outputWidth )? imageWidth : outputWidth;
  if( end_x - start_x > frambuf_width - left )
  {
    end_x = frambuf_width - left + start_x;
  }

  yP1 = pSrc;
  yP2 = yP1 + imageWidth;
  uP = yP1 + imageWidth * imageHeight;
  vP = uP + ( imageWidth >> 1 ) * ( imageHeight >> 1 );

  if( top < 0 )
  {
    top = 0;
  }
  if( left < 0 )
  {
    left = 0;
  }
  xfbptr = (u32*)( pDst + top * (frambuf_width << 1) + (left << 1) );
  frambuf_width >>= 1;

  for( py = start_y ; py < end_y ; py += 2, xfbptr += frambuf_width << 1 )
  {
    for( xfbptr1 = xfbptr, px = start_x ; px < end_x ; px += 8, xfbptr1 += 4 )
    {
      int idxY = py * imageWidth + px;
      int idxUV = ( py >> 1 ) * ( imageWidth >> 1 ) + ( px >> 1 );

      u = (u32)( uP[idxUV + 0] << 16 );
      v = vP[idxUV + 0];

      xfbptr1[0] = ( (yP1[idxY + 0] << 24) | u ) | ( (yP1[idxY + 1] << 8) | v );
      xfbptr1[frambuf_width] = ( (yP2[idxY + 0] << 24) | u) | ( (yP2[idxY + 1] << 8) | v );

      u = (u32)( uP[idxUV + 1] << 16 );
      v = vP[idxUV + 1];

      xfbptr1[1] = ( (yP1[idxY + 2] << 24) | u ) | ( (yP1[idxY + 3] << 8) | v );
      xfbptr1[frambuf_width + 1] = ( (yP2[idxY + 2] << 24) | u ) | ( (yP2[idxY + 3] << 8) | v );

      u = (u32)( uP[idxUV + 2] << 16 );
      v = vP[idxUV + 2];

      xfbptr1[2] = ( (yP1[idxY + 4] << 24) | u ) | ( (yP1[idxY + 5] << 8) | v );
      xfbptr1[frambuf_width + 2] = ( (yP2[idxY + 4] << 24) | u ) | ( (yP2[idxY + 5] << 8) | v );

      u = (u32)( uP[idxUV + 3] << 16 );
      v = vP[idxUV + 3];

      xfbptr1[3] = ( (yP1[idxY + 6] << 24) | u ) | ( (yP1[idxY + 7] << 8) | v );
      xfbptr1[frambuf_width + 3] = ( (yP2[idxY + 6] << 24) | u ) | ( (yP2[idxY + 7] << 8) | v );
    }
    assert( (u8*)xfbptr <= pDst + frambuf_width * frambuf_height * 4 );
  }
}

/*
static inline void nvlYUV411_RGBA32( u8 *pSrc, u8 *pDst, int outputWidth, int outputHeight )
{
  u8	*ycmp, *ucmp, *vcmp;
  int	h_size = outputWidth  >> 1;
  int	v_size = outputHeight >> 1;
  int	onelinebyte = outputWidth * 4;
  int	i,j;
  int	wCount = 0,hCount = 0;

  ycmp = pSrc;
  pSrc += ( outputWidth * outputHeight );
  ucmp = pSrc;
  pSrc += ( h_size * v_size );
  vcmp = pSrc;

  for( j = v_size ; j > 0 ; j-- ){
    u8* yup = ycmp;
    u8* ydn = ( ycmp += outputWidth );
    u8* imgline = pDst + ( hCount >> 1 ) * outputWidth * 16;
    imgline += ( hCount & 1 ) ? 16 : 0;
    wCount = 0;
    for( i = h_size ; i > 0 ; i-- ){
      long v = *vcmp++ - NVL_HLF_LUMI;
      long u = *ucmp++ - NVL_HLF_LUMI;
      long cv_r =               NVL_CONV_RV * v + NVL_COLOR_OFFSET;
      long cv_g = NVL_CONV_GU * u + NVL_CONV_GV * v + NVL_COLOR_OFFSET;
      long cv_b = NVL_CONV_BU * u               + NVL_COLOR_OFFSET;
      long y;
      u8* imgup = imgline + ( wCount >> 1 ) * 64;
      u8* imgdn;

      imgup += ( wCount & 1 ) ? 4 : 0;
      imgdn = imgup + 8;

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack32( imgup,y + cv_r,y + cv_g,y + cv_b );

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack32( imgup + 2,y + cv_r,y + cv_g,y + cv_b );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack32( imgdn,y + cv_r,y + cv_g,y + cv_b );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack32( imgdn + 2,y + cv_r,y + cv_g,y + cv_b );
      wCount++;
    }
    hCount++;
    ycmp += outputWidth;
  }
}
*/

static inline void nvlYUV411_RGBA32( u8 *pSrc, u8 *pDst, int outputWidth, int outputHeight )
{
  u8	*ycmp, *ucmp, *vcmp;
  int	h_size = outputWidth  >> 1;
  int	v_size = outputHeight >> 1;
  int	onelinebyte = outputWidth * 4;
  int	i,j;
  int	wCount = 0,hCount = 0;

  ycmp = pSrc;
  pSrc += ( outputWidth * outputHeight );
  ucmp = pSrc;
  pSrc += ( h_size * v_size );
  vcmp = pSrc;

  for( j = v_size ; j > 0 ; j-- ){
    u8* yup = ycmp;
    u8* ydn = ( ycmp += outputWidth );
    u8* imgline = pDst + ( hCount >> 1 ) * outputWidth * 16;
    imgline += ( hCount & 1 ) ? 16 : 0;
    wCount = 0;
    for( i = h_size ; i > 0 ; i-- ){
      s32 v = *vcmp++ - NVL_HLF_LUMI;
      s32 u = *ucmp++ - NVL_HLF_LUMI;
      s32 cv_r =               NVL_CONV_RV * v + NVL_COLOR_OFFSET;
      s32 cv_g = NVL_CONV_GU * u + NVL_CONV_GV * v + NVL_COLOR_OFFSET;
      s32 cv_b = NVL_CONV_BU * u               + NVL_COLOR_OFFSET;
      s32 y;
      u8* imgup = imgline + ( wCount >> 1 ) * 64;
      u8* imgdn;

      imgup += ( wCount & 1 ) ? 4 : 0;
      imgdn = imgup + 8;

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack32( imgup,y + cv_r,y + cv_g,y + cv_b );

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack32( imgup + 2,y + cv_r,y + cv_g,y + cv_b );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack32( imgdn,y + cv_r,y + cv_g,y + cv_b );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack32( imgdn + 2,y + cv_r,y + cv_g,y + cv_b );
      wCount++;
    }
    hCount++;
    ycmp += outputWidth;
  }
}

static inline void nvlYUV411_RGB5A3( u8 *pSrc, u8 *pDst, int outputWidth, int outputHeight )
{
  u8	*ycmp, *ucmp, *vcmp;
  int h_size = outputWidth  >> 1;
  int v_size = outputHeight >> 1;
  int onelinebyte = outputWidth * 4;
  int i,j;
	int	wCount = 0,hCount = 0;

  ycmp = pSrc;
	pSrc += ( outputWidth * outputHeight );
  ucmp = pSrc;
	pSrc += ( h_size * v_size );
  vcmp = pSrc;

  for( j = v_size ; j > 0 ; j-- ){
    u8* yup = ycmp;
    u8* ydn = ( ycmp += outputWidth );
		u8* imgline = pDst + ( hCount >> 1 ) * outputWidth * 8;
		imgline += ( hCount & 1 ) ? 16 : 0;
		wCount = 0;
    for( i = h_size ; i > 0 ; i-- ){
      s32 v = *vcmp++ - NVL_HLF_LUMI;
      s32 u = *ucmp++ - NVL_HLF_LUMI;
      s32 cv_r =               NVL_CONV_RV * v + NVL_COLOR_OFFSET;
      s32 cv_g = NVL_CONV_GU * u + NVL_CONV_GV * v + NVL_COLOR_OFFSET;
      s32 cv_b = NVL_CONV_BU * u               + NVL_COLOR_OFFSET;
      s32 y;
			u8* imgup = imgline + ( wCount >> 1 ) * 32;
			u8* imgdn;

			imgup += ( wCount & 1 ) ? 4 : 0;
			imgdn = imgup + 8;

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack16( imgup,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB5A3 );

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack16( imgup + 2,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB5A3 );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack16( imgdn,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB5A3 );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack16( imgdn + 2,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB5A3 );
			wCount++;
    }
		hCount++;
    ycmp += outputWidth;
  }
}

static inline void nvlYUV411_RGB565( u8 *pSrc, u8 *pDst, int outputWidth, int outputHeight )
{
  u8	*ycmp, *ucmp, *vcmp;
  int h_size = outputWidth  >> 1;
  int v_size = outputHeight >> 1;
  int onelinebyte = outputWidth * 4;
  int i,j;
	int	wCount = 0,hCount = 0;

  ycmp = pSrc;
	pSrc += ( outputWidth * outputHeight );
  ucmp = pSrc;
	pSrc += ( h_size * v_size );
  vcmp = pSrc;

  for( j = v_size ; j > 0 ; j-- ){
    u8* yup = ycmp;
    u8* ydn = ( ycmp += outputWidth );
		u8* imgline = pDst + ( hCount >> 1 ) * outputWidth * 8;
		imgline += ( hCount & 1 ) ? 16 : 0;
		wCount = 0;
    for( i = h_size ; i > 0 ; i-- ){
      s32 v = *vcmp++ - NVL_HLF_LUMI;
      s32 u = *ucmp++ - NVL_HLF_LUMI;
      s32 cv_r =               NVL_CONV_RV * v + NVL_COLOR_OFFSET;
      s32 cv_g = NVL_CONV_GU * u + NVL_CONV_GV * v + NVL_COLOR_OFFSET;
      s32 cv_b = NVL_CONV_BU * u               + NVL_COLOR_OFFSET;
      s32 y;
			u8* imgup = imgline + ( wCount >> 1 ) * 32;
			u8* imgdn;

			imgup += ( wCount & 1 ) ? 4 : 0;
			imgdn = imgup + 8;

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack16( imgup,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB565 );

      y = ( NVL_CONV_Y * ( *yup++ - NVL_Y_LUMI ) );
      nvlPack16( imgup + 2,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB565 );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack16( imgdn,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB565 );

      y = ( NVL_CONV_Y * ( *ydn++ - NVL_Y_LUMI ) );
      nvlPack16( imgdn + 2,y + cv_r,y + cv_g,y + cv_b,GX_TF_RGB565 );
			wCount++;
    }
		hCount++;
    ycmp += outputWidth;
  }
}

static void nvlConvert( u8 *pPicture, u32 ulFrame, void *data )
{
#pragma unused( ulFrame )

  nvlMovie    *hvqm = (nvlMovie*)data;
  nvlMovie    *m = (nvlMovie*)hvqm->userData;
  int         wdth = nvlMovieGetWidth( hvqm ), hght = nvlMovieGetHeight( hvqm );
  size_t      outp_buf_siz;
  u8          *outBuf;
#ifdef _DEBUG
  size_t      res;
#endif

  assert( hvqm && m );
  switch( m->format )
  {
  case NVL_IMAGE_FORMAT_YUV411  :
#ifdef _DEBUG
    res =
#endif
    outp_buf_siz = (size_t)( wdth * hght + wdth * hght / 2 );
    outBuf = (u8*)(*m->putFunc)( m, &outp_buf_siz, m->putFunc_data );
    assert( res == outp_buf_siz );
    memcpy( pPicture, outBuf, outp_buf_siz );
    break;
  case NVL_IMAGE_FORMAT_YUV422  :
    outp_buf_siz = (size_t)( wdth * hght * 2 );
    outBuf = (u8*)(*m->putFunc)( m, &outp_buf_siz, m->putFunc_data );
    nvlYUV411_YUV422( pPicture, outBuf, m->top_clip, m->left_clip, m->width, m->height, wdth, hght, nvlSystemData.fb_width, nvlSystemData.fb_height );
    break;
  case NVL_IMAGE_FORMAT_RGBA32  :
#ifdef _DEBUG
    res =
#endif
    outp_buf_siz = (size_t)( wdth * hght * 4 );
    outBuf = (u8*)(*m->putFunc)( m, &outp_buf_siz, m->putFunc_data );
    assert( outBuf && res == outp_buf_siz );
    nvlYUV411_RGBA32( pPicture, outBuf, wdth, hght );
    break;
  case NVL_IMAGE_FORMAT_RGBA565 :
#ifdef _DEBUG
    res =
#endif
    outp_buf_siz = (size_t)( wdth * hght * 2 );
    outBuf = (u8*)(*m->putFunc)( m, &outp_buf_siz, m->putFunc_data );
    assert( outBuf && res == outp_buf_siz );
    nvlYUV411_RGB565( pPicture, outBuf, wdth, hght );
    break;
  case NVL_IMAGE_FORMAT_RGBA5A3 :
#ifdef _DEBUG
    res =
#endif
    outp_buf_siz = (size_t)( wdth * hght * 2 );
    outBuf = (u8*)(*m->putFunc)( m, &outp_buf_siz, m->putFunc_data );
    assert( outBuf && res == outp_buf_siz );
    nvlYUV411_RGB5A3( pPicture, outBuf, wdth, hght );
    break;
  default                       :
    assert( 0 && "Invalid format." );
    break;
  }
  DCStoreRange( outBuf, outp_buf_siz );
}

static void nvlAudioCallBack( void )
{
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    nvlMovie  *p = nvlSystemData.pMovies[i];

    if( p && p->flags & NVL_FLAG_PLAYING )
    {
#ifndef NO_AUDIO
// FIXME_BEGIN
      p->CurrentSoundBuffer ^= 1;
      memset( p->SoundBuffer[p->CurrentSoundBuffer], 0, NVL_AI_DMA_SIZE );
#endif
      if(
          p &&
          (p->cStatus & NVL_STATUS_PLAY) &&
          (p->cFlags & NVL_FLAG_PLAY_AUDIO) &&
          !(p->cStatus & NVL_STATUS_START) &&
          !(p->pSoundBufPtr < p->pSoundDecPtr && (p->pSoundBufPtr + NVL_AI_DMA_SIZE > p->pSoundDecPtr || (p->cStatus & NVL_STATUS_PAUSE)))
        )
      {
        BOOL oldInterrupts = OSEnableInterrupts();

        s16 *inptr = (s16*)p->pSoundBufPtr;

        for( int i = 0 ; i < (NVL_AI_DMA_SIZE >> 1) ; i++ )
        {
          if( &inptr[i] >= ( s16 * )p->pSoundMaxPtr )
          {
            inptr = (s16*)( p->pSoundBuf - i * 2 );
          }
          ((u16*)p->SoundBuffer[p->CurrentSoundBuffer])[i] += (s16)( (((s32)inptr[i] * nvlVolumeTable[nvlSystemData.volume & 0x7f] >> nvlVolumeShift) & 0x7fff) + (inptr[i] & 0x8000) );
        }

        p->pSoundBufPtr += NVL_AI_DMA_SIZE;
        p->ullAudioIndex += NVL_AI_DMA_SIZE;
        p->ulStartTime = OSGetTime();
        if( p->pSoundBufPtr >= p->pSoundMaxPtr ) p->pSoundBufPtr = p->pSoundBufPtr - p->pSoundMaxPtr + p->pSoundBuf;
        OSRestoreInterrupts( oldInterrupts );
      }
#ifndef NO_AUDIO
      DCFlushRange( p->SoundBuffer[p->CurrentSoundBuffer], NVL_AI_DMA_SIZE );
      AIInitDMA( (u32)p->SoundBuffer[p->CurrentSoundBuffer], NVL_AI_DMA_SIZE );
      break;
#endif
    }
#ifndef NO_AUDIO
    else
    {
      AIInitDMA( (u32) nvlSystemData.nvlAudioNullBuf, NVL_AI_DMA_SIZE );
    }
// FIXME_END
#endif
  }
}

static void nvlAudioInit( void )
{
  AIInit( NULL );
  nvlSystemData.nvlOldAICallback = AIRegisterDMACallback( nvlAudioCallBack );
   AISetDSPSampleRate( AI_SAMPLERATE_48KHZ );
  memset( nvlSystemData.nvlAudioNullBuf, 0, NVL_AI_DMA_SIZE );
  DCFlushRange( nvlSystemData.nvlAudioNullBuf, NVL_AI_DMA_SIZE );
  AIInitDMA( (u32) nvlSystemData.nvlAudioNullBuf, NVL_AI_DMA_SIZE );
  AIStartDMA( );
}

static void nvlAudioShutdown( void )
{
  AIStopDMA( );
  AIRegisterDMACallback( nvlSystemData.nvlOldAICallback );
  AIReset( );
}

void nvlInit()
{
  if( !nvlHVQMDecoderInitialized )
  {
    HVQM4InitDecoder();
    nvlHVQMDecoderInitialized = 1;
  }
  if( !nvlSystemInitialized )
  {
    nvlAudioInit();
    memset( &nvlSystemData, 0, sizeof(nvlSystemData) );
    for( int i = 0, n = -NVL_COLOR_CLIP_OFFSET; i < NVL_COLOR_CLIP_TABLE; i++, n++)
    {
      nvlClipTable[i] = (u8)( ((n < 0) ? 0 : ((n > NVL_MAX_LUMI ) ? NVL_MAX_LUMI : n)) );
    }
    nvlSystemData.volume = NVL_MOVIE_VOLUME;
    nvlSystemData.fb_width = 640;
    nvlSystemData.fb_height = 480;
    nvlSystemData.priority_movie = -1;

    nvlSystemData.decodeThreadStack = (u8*)nglMemAlloc( NVL_DECODE_THREAD_STACK_SIZE, 32 );
    assert( nvlSystemData.decodeThreadStack );
    OSCreateThread(
                    &nvlSystemData.decodeThread,
                    nvlDecodeThreadFunc,
                    NULL,
                    nvlSystemData.decodeThreadStack + NVL_DECODE_THREAD_STACK_SIZE,
                    NVL_DECODE_THREAD_STACK_SIZE,
                    DECODE_PRI,
                    OS_THREAD_ATTR_DETACH
                  );
    nvlSystemData.decodeThreadRunningFlag = 1;
    OSResumeThread( &nvlSystemData.decodeThread );

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
    for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
    {
      if( nvlSystemData.pMovies[i] && (nvlSystemData.pMovies[i]->flags & NVL_FLAG_PLAYING) )
      {
        nvlMovieSource  *src = nvlSystemData.pMovies[i]->src;

        nvlStopMovie( nvlSystemData.pMovies[i] );
        if( src->refs == 0 )
        {
          nvlReleaseMovieSource( src );
        }
      }
    }

    nvlSystemData.decodeThreadIsRunning = -1;
    while( nvlSystemData.decodeThreadIsRunning )
    {
      VIWaitForRetrace();
    }
    nglMemFree( nvlSystemData.decodeThreadStack );

    nvlAudioShutdown();
    nvlSystemInitialized = 0;
  }
}

static void nvlAdvanceMovie( nvlMovie* m )
{
  int		    j, flag = 0;
  OSTime    diffTime;

  assert( nvlSystemInitialized );
  assert( m );
  if( !(m->cFlags & NVL_FLAG_PLAY_AUDIO) && !(m->cStatus & NVL_STATUS_OPEN) && nvlStreamGetErrorStatus(m->pStream) == DVD_STATE_COVER_OPEN )
  {
    nvlPauseMovieInternal( m );
    m->cStatus |= NVL_STATUS_OPEN;
  }
  if(
      nvlMovieStatusInternal(m) == (NVL_STATUS_OPEN | NVL_STATUS_PAUSE) ||
      nvlMovieStatusInternal(m) == (DVD_STATE_COVER_CLOSED | NVL_STATUS_ERRORFLAG)
    )
  {
    nvlkMoviePlayInternal( m, m->cFlags );
  }
  if( !(m->cStatus & NVL_STATUS_PLAY) )
  {
    return;
  }
  if( m->cStatus & NVL_STATUS_START )
  {
    for( j = 0; j < HVQM4_DEC_BUFFER_CNT; j++ )
    {
      if( m->sPlayerData[j].state != 1 )
      {
        return;
      }
    }
    if( m->cFlags & NVL_FLAG_PLAY_AUDIO )
    {
      m->ullAudioIndex = 0;
    }
    else
    {
      m->ulStartTime = OSGetTime();
    }
    m->cStatus = NVL_STATUS_PLAY;
  }
  if( m->cFlags & NVL_FLAG_PLAY_AUDIO )
  {
    diffTime = OSGetTime() - m->ulStartTime;
    m->ulCurrentFrame = (u32)( ((m->ullAudioIndex * 1000000 / (4 * m->sHeader.audioinfo.samples_per_sec)) + OSTicksToMicroseconds(diffTime)) / m->sHeader.usec_per_frame );
  }
  else
  {
    m->ulCurrentFrame = (u32)( OSTicksToMicroseconds(OSGetTime() - m->ulStartTime) / m->sHeader.usec_per_frame );
  }
  for( j = 0; j < HVQM4_DEC_BUFFER_CNT; j++ )
  {
    if( m->sPlayerData[j].state == 1 && m->ulCurrentFrame == m->sPlayerData[j].frame )
    {
      nvlConvert( m->sPlayerData[j].buf, m->ulCurrentFrame, m );
      flag = 1;
      m->sPlayerData[j].state = 2;
    }
  }
  if( m->funcDrawPicture )
  {
    m->funcDrawPicture( m->ulCurrentFrame );
  }
}

void nvlAdvance()
{
  assert( nvlSystemInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    nvlMovie  *m = nvlSystemData.pMovies[i];

    if( m && (m->flags & NVL_FLAG_PLAYING) )
    {
      nvlAdvanceMovie( m );
    }
  }
}

nvlMovieSource* nvlLoadMovieSource( const char* filename, int buffer_size, void *buffer )
{
#pragma unused( buffer_size, buffer )
  char    local[256];

  assert( nvlSystemInitialized );
  assert( filename );
  strncpy( local, filename, sizeof(local) );
  local[255] = '\0';
  for( char* lp = local; *lp; lp++ )
  {
    if( *lp == '\\' )
    {
      *lp = '/';
    }
  }
  if( DVDConvertPathToEntrynum(local) < 0 )
  {
    return NULL;
  }

  nvlMovieSource    *rv = (nvlMovieSource*)nglMemAlloc( sizeof(nvlMovieSource) );

  assert( rv );
  memset( rv, 0, sizeof(nvlMovieSource) );
  rv->filename = (char*)nglMemAlloc( strlen(local) + 1 );
  strcpy( rv->filename, local );
  return rv;
}

nvlMovieSource* nvlLoadMovieSource( void*, size_t )
{
  assert( 0 && "Not implemented." );
  return NULL;
}

nvlMovieSource* nvlLoadMovieSource( nvlInputBufferCallback )
{
  assert( 0 && "Not implemented." );
  return NULL;
}

void nvlReleaseMovieSource( nvlMovieSource *arg )
{
  assert( nvlSystemInitialized );
  assert( arg && arg->refs == 0 );
  nglMemFree( arg->filename );
  nglMemFree( arg );
}

nvlMovie* nvlAddMovie( nvlMovieSource *src, nvlImageFormat format )
{
  nvlMovie    *rv = (nvlMovie*)nglMemAlloc( sizeof(nvlMovie) );
  u32         res;
  int         i;

  assert( nvlSystemInitialized );
  assert( rv && src );
  assert( format >= NVL_IMAGE_FORMAT_YUV411 && format <= NVL_IMAGE_FORMAT_RGBA5A3 );
  memset( rv, 0, sizeof(nvlMovie) );
  rv->src = src;
  rv->pStream = nvlStreamCreate( src->filename );
  while( !nvlStreamIsReady(rv->pStream) )
    ;
  i = nvlStreamGetCopy( rv->pStream, (void*)&rv->sHeader, (s32)sizeof(HVQM4Header) );
  assert( i == (int)sizeof(HVQM4Header) );
  assert( strncmp(HVQM4_FILEVERSION, (char*)rv->sHeader. file_version, strlen(HVQM4_FILEVERSION )) == 0 );
  rv->cAudioSample = 0xff;
  if( rv->sHeader.audioinfo.samples_per_sec )
  {
// FIXME_BEGIN
    assert( rv->sHeader.audioinfo.samples_per_sec == 48042 || rv->sHeader.audioinfo.samples_per_sec == 32028 );
    if( rv->sHeader.audioinfo.samples_per_sec == 48042 )
    {
      assert( AIGetDSPSampleRate() == AI_SAMPLERATE_48KHZ );
      rv->cAudioSample = AI_SAMPLERATE_48KHZ;
    } 
    else  // rv->sHeader.audioinfo.samples_per_sec == 32028
    {
      assert( AIGetDSPSampleRate() == AI_SAMPLERATE_32KHZ );
      rv->cAudioSample = AI_SAMPLERATE_32KHZ;
    }
  }
// FIXME_END
  HVQM4InitSeqObj( &rv->sSeqObj,&rv->sHeader.videoinfo );
  assert( HVQM4BuffSize(&rv->sSeqObj) != 0 );
  for( i = 0 ; i < HVQM4_DEC_BUFFER_CNT ; i++ )
  {
    rv->psPlayerData[i] = &rv->sPlayerData[i];
  }
  rv->format = format;

  u32   h_samp_MUL_v_samp = (u32)HVQM4GetSamplingRateH( &rv->sSeqObj ) * (u32)HVQM4GetSamplingRateV( &rv->sSeqObj ),
        bufsize = ( (OSRoundUp32B(rv->sHeader.max_audio_record_size) + DMA_BLOCK_SIZE - 1) / DMA_BLOCK_SIZE ) * DMA_BLOCK_SIZE * 4 * ( HVQM4_DEC_BUFFER_CNT + 1 );

  res = OSRoundUp32B( rv->sHeader.max_frame_size );
  res += OSRoundUp32B( rv->sHeader.max_audio_record_size );
  res += ( (OSRoundUp32B(rv->sHeader.max_audio_record_size) + DMA_BLOCK_SIZE - 1) / DMA_BLOCK_SIZE ) * DMA_BLOCK_SIZE * 4 * ( HVQM4_DEC_BUFFER_CNT + 1 );
  res += OSRoundUp32B( (HVQM4GetWidth(&rv->sSeqObj) * HVQM4GetHeight(&rv->sSeqObj) * (h_samp_MUL_v_samp + 2)) / (h_samp_MUL_v_samp) ) * HVQM4_DEC_BUFFER_CNT;
  res += OSRoundUp32B( HVQM4BuffSize(&rv->sSeqObj) );
  rv->pBuffer = nglMemAlloc( OSRoundUp32B(res), 32 );
  memset( rv->pBuffer, 0, OSRoundUp32B(res) );
  HVQM4SetBuffer( &rv->sSeqObj, rv->pBuffer );
  rv->pPicReadBuf = (unsigned char*)rv->pBuffer + OSRoundUp32B( HVQM4BuffSize(&rv->sSeqObj) );
  rv->pSoundReadBuf = rv->pPicReadBuf + OSRoundUp32B( rv->sHeader.max_frame_size );
  memset( rv->pSoundReadBuf, 0, bufsize );
  rv->sAdpcmState[0].bufstart = rv->sAdpcmState[1].bufstart = rv->pSoundBuf = rv->pSoundReadBuf + OSRoundUp32B( rv->sHeader.max_audio_record_size );
  rv->pSoundBufPtr = rv->pSoundDecPtr = rv->pSoundBuf;
  rv->sAdpcmState[0].bufend = rv->sAdpcmState[1].bufend = rv->pSoundMaxPtr = rv->sPlayerData[0].buf = rv->pSoundBuf + bufsize;
  for( i = 1 ; i < HVQM4_DEC_BUFFER_CNT ; i++ )
  {
    rv->sPlayerData[i].buf = rv->sPlayerData[i-1].buf + OSRoundUp32B( (HVQM4GetWidth(&rv->sSeqObj) * HVQM4GetHeight(&rv->sSeqObj) * (h_samp_MUL_v_samp + 2)) / h_samp_MUL_v_samp );
  }
  rv->userData = rv;
  src->refs++;

  for( i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] == NULL )
    {
      break;
    }
  }
  assert( i < NVL_MAX_MOVIE_NUM );
  nvlSystemData.pMovies[i] = rv;

  rv->SoundBuffer[0] = (u8*)nglMemAlloc( NVL_AI_DMA_SIZE, 32 );
  assert( rv->SoundBuffer[0] );
  rv->SoundBuffer[1] = (u8*)nglMemAlloc( NVL_AI_DMA_SIZE, 32 );
  assert( rv->SoundBuffer[1] );
  memset( rv->SoundBuffer[rv->CurrentSoundBuffer], 0, NVL_AI_DMA_SIZE );
  DCFlushRange( rv->SoundBuffer[rv->CurrentSoundBuffer], NVL_AI_DMA_SIZE );
  AIInitDMA( (u32)rv->SoundBuffer[rv->CurrentSoundBuffer], NVL_AI_DMA_SIZE );
  AIStartDMA();
  return rv;
}

nvlResult nvlPlayMovie( nvlMovie* m, nvlOutputBufferCallback cbck, void* user_data, int looping )
{
  assert( nvlSystemInitialized );
  assert( m && cbck );
  assert( !(m->flags & NVL_FLAG_PLAYING) );
  m->hvqmflags = (NVL_FLAG_NONE | NVL_FLAG_PLAY_AUDIO);
//  m->hvqmflags = (NVL_FLAG_NONE);
  if( looping )
  {
    m->hvqmflags |= NVL_FLAG_PLAY_REPEAT;
  }
  m->putFunc = cbck;
  m->putFunc_data = user_data;
  nvlkMoviePlayInternal( m, m->hvqmflags );
  m->flags |= NVL_FLAG_PLAYING;
  return NVL_RESULT_SUCCESS;
}

void nvlStopMovie( nvlMovie* m )
{
  int   i;

  assert( nvlSystemInitialized );
  assert( m && (m->flags & NVL_FLAG_PLAYING) );
  m->cStatus = NVL_STATUS_STOP;
  while( m->bDecode )
  {
    VIWaitForRetrace();
  };
  nvlStreamDestroy( m->pStream );
  for( i = 0; i < NVL_MAX_MOVIE_NUM ; i++ )
  {
    if( nvlSystemData.pMovies[i] == m )
    {
      break;
    }
  }
  assert( i < NVL_MAX_MOVIE_NUM );
  nvlSystemData.pMovies[i] = NULL;
  m->src->refs--;
  nglMemFree( m->pBuffer );
  nglMemFree( m->SoundBuffer[0] );
  nglMemFree( m->SoundBuffer[1] );
  nglMemFree( m );
}

nvlResult nvlPauseMovie( nvlMovie* m )
{
  assert( nvlSystemInitialized );
  if( m && (m->flags & NVL_FLAG_PLAYING) )
  {
    if( m->flags & NVL_FLAG_PAUSED )
    {
      return NVL_RESULT_PAUSED;
    }
#ifdef _DEBUG
    int res =
#endif
    nvlPauseMovieInternal( m );
    assert( res == 0 );
    m->flags |= NVL_RESULT_PAUSED;
  }
  return NVL_RESULT_SUCCESS;
}

nvlResult nvlUnpauseMovie( nvlMovie* m )
{
  assert( nvlSystemInitialized );
  assert( m && (m->flags & NVL_FLAG_PLAYING) );
  if( m->flags & NVL_FLAG_PAUSED )
  {
    nvlkMoviePlayInternal( m, m->hvqmflags );
    m->flags &= ~NVL_FLAG_PAUSED;
    return NVL_RESULT_SUCCESS;
  }
  return NVL_RESULT_PLAYING;
}

nvlResult nvlMovieStatus( const nvlMovie* m )
{
  assert( nvlSystemInitialized );
  assert( m );
  if( m->flags & NVL_FLAG_PAUSED )
  {
    return NVL_RESULT_PAUSED;
  }
  else if( m->flags & NVL_FLAG_PLAYING )
  {
    return NVL_RESULT_PLAYING;
  }
  else
  {
    return NVL_RESULT_SUCCESS;  // don't know what it would be exactly but everything is OK
  }
}

int nvlMovieGetWidth( nvlMovie* m )
{
  assert( nvlSystemInitialized );
  assert( m );
  return (int)HVQM4GetWidth( &m->sSeqObj );
}

int nvlMovieGetHeight( nvlMovie* m )
{
  assert( nvlSystemInitialized );
  assert( m );
  return (int)HVQM4GetHeight( &m->sSeqObj );
}

int nvlMovieGetFrameNum( const nvlMovie* m )
{
  return (int)m->sHeader.video_total;
}

int nvlMovieGetFrame( const nvlMovie* m )
{
  int   rv = (int)m->ulCurrentFrame;

  if( rv < 0 || rv > nvlMovieGetFrameNum(m) + 10 )
  {
    rv = 0;
  }
  return rv;
}

void nvlStopAllMovies()
{
  assert( nvlSystemInitialized );
  for( int i = 0; i < NVL_MAX_MOVIE_NUM; i++ )
  {
    if( nvlSystemData.pMovies[i] && (nvlSystemData.pMovies[i]->flags & NVL_FLAG_PLAYING) )
    {
      nvlMovieSource  *src = nvlSystemData.pMovies[i]->src;

      nvlStopMovie( nvlSystemData.pMovies[i] );
      if( src->refs == 0 )
      {
        nvlReleaseMovieSource( src );
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

void nvlMovieSetParametersGC( nvlMovie* m, int left, int top, int width, int height )
{
  assert( nvlSystemInitialized );
  assert( m );
  m->top_clip = top;
  m->left_clip = left;
  m->width = width;
  m->height = height;
}

void nvlMovieSetVolume( nvlMovie* m, float volume )
{
#pragma unused(m)
  assert( nvlSystemInitialized );
  assert( volume >= 0 && volume <= 1.0f );
  nvlSystemData.volume = (u8)( volume * 64.0f );
}

void nvlSetFramebufferGC( int framebuf_width, int framebuf_height )
{
  nvlSystemData.fb_width = framebuf_width;
  nvlSystemData.fb_height = framebuf_height;
}
