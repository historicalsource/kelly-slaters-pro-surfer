/**********************************************************************
ご注意

1. 株式会社ハドソンが提供する本媒体に含まれる、技術資料、図面、規格、コ
   ンピュータ・ソフトウェア、アルゴリズム及びその他の情報（以下、併せて
   「本秘密情報」といいます）の著作権、知的所有権その他の権利は、株式会
   社ハドソンまたは株式会社ハドソンに使用を許諾した者に帰属します。
2. 本秘密情報は、株式会社ハドソンが許諾する目的以外の目的で使用、複製す
   ることができません。
3. 本秘密情報を、第三者へ開示することはできません。
4. 本秘密情報を、改変して使用することはできません。
5. 株式会社ハドソンは、本秘密情報の使用により生じたいかなる損害に関して
   も、一切責任を負わないものとします。

Notices

1. Copyrights, any other intellectual property right and other rights 
   to and in the technical material, charts, specifications computer 
   software, algorithm and any other information (hereinafter called 
   "the Confidential Information") included in the media which is 
   provided by Hudson Soft Co., Ltd. shall belong to Hudson Soft Co., 
   Ltd. or its licensors.
2. No part of the Confidential Information may be reproducedor used 
   for any purpose other than that licensed by Hudson Soft Co., Ltd.
3. No part of the Confidential Information may be disclosed to any 
   third party.
4. No part of the Confidential Information may be revised in  order to 
   use it.
5. In no case shall Hudson Soft Co., Ltd. be held responsible for any 
   loss or damage arising from the use of the Confidential Information.
**********************************************************************/

/*
 * File:	hvqm4player.c
 *
 * (C) 2000-2001 HUDSON SOFT
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hvqm4player.h"
#include "nvlstream_gc.h"

/*---------------------------------------------------------------------------*
    Variables
 *---------------------------------------------------------------------------*/

static void	**pFrameBufPtr = NULL;
static int	iFrameBufCnt = 0;
static int	iCurrentFrameBuf = 0;
int	iInitializePlayer = 0;

static OSThread DecodeThread;
static u8       DecodeThreadStack[1024*8];
extern void	*DecodeThreadFunc( void *param );

static void ( *DrawDone )( void *pFrameBuffer,int iDrawFlag ) = NULL;
static void ( *BeforeDraw )( void **pFrameBuffer ) = NULL;

HVQM4PlayerObj	*PlayerObjPtr;

#define VOLUME_SHIFT 16
u16 VOLUME_TABLE[128] = { 
    511, 1023, 1535, 2047, 2559, 3071, 3583, 4095, 4607, 5119, 5631, 6143, 6655, 7167, 7679, 8191,
   8703, 9215, 9727,10239,10751,11263,11775,12287,12799,13311,13823,14335,14847,15359,15871,16383,
  16895,17407,17919,18431,18943,19455,19967,20479,20991,21503,22015,22527,23039,23551,24063,24575,
  25087,25599,26111,26623,27135,27647,28159,28671,29183,29695,30207,30719,31231,31743,32255,32767,
  33279,33791,34303,34815,35327,35839,36351,36863,37375,37887,38399,38911,39423,39935,40447,40959,
  41471,41983,42495,43007,43519,44031,44543,45055,45567,46079,46591,47103,47615,48127,48639,49151,
  49663,50175,50687,51199,51711,52223,52735,53247,53759,54271,54783,55295,55807,56319,56831,57343,
  57855,58367,58879,59391,59903,60415,60927,61439,61951,62463,62975,63487,63999,64511,65023,65535
};

/*---------------------------------------------------------------------------*
    Function prototype declarations
 *---------------------------------------------------------------------------*/

s32 HVQM4Read2Buf(void *addr,s32 length);
void HVQM4StopRead(void);
TST HVQM4GetStatus(void);
void HVQM4CreateReadThread(char *pStr);
void HVQM4CloseRead(void);
s32 HVQM4ErrStatus(void);
void HVQM4SendErr(void);
int IsDecodeThread( void );

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerAudioNextFrame
    Description:    Callback function for Audio. This function is not called 
                    when HVQM4PLAYER_PLAY_AUDIO is not set.
     Arguments:     none
     Returns:       none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerAudioNextFrame( HVQM4PlayerObj *pobj,u16 *pNextSoundBuffer,u32 ulSoundSize,u8 volume ){
  int	i;
  s16	*inptr;
  s32	temp;
  BOOL	oldInterrupts;

  if( !pobj ) return;

  if( !( pobj->cStatus & HVQM4PLAYER_STATUS_PLAY ) ) return; 
  if( pobj->cStatus & HVQM4PLAYER_STATUS_START ) return; 
  if( pobj && pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ){
    if( pobj && pobj->pSoundBufPtr < pobj->pSoundDecPtr ){
      if( ( pobj && ( pobj->pSoundBufPtr + ulSoundSize ) > pobj->pSoundDecPtr || pobj->cStatus & HVQM4PLAYER_STATUS_PAUSE ) ){
	return;
      }
    }
    oldInterrupts = OSEnableInterrupts();
    inptr = ( s16 * )pobj->pSoundBufPtr;

    for( i = 0 ; i < ulSoundSize >> 1 ; i++ ){
      if( &inptr[i] >= ( s16 * )pobj->pSoundMaxPtr ){
	inptr = ( s16 * )( pobj->pSoundBuf - i * 2 );
      }
      temp = ( s32 )inptr[i];
      pNextSoundBuffer[i] += ( s16 )( ( ( temp * VOLUME_TABLE[volume & 0x7f] >> VOLUME_SHIFT ) & 0x7fff ) + ( inptr[i] & 0x8000 ) );
    }

    pobj->pSoundBufPtr += ulSoundSize;
    pobj->ullAudioIndex += ulSoundSize;
    pobj->ulStartTime = OSGetTime();
    if( pobj->pSoundBufPtr >= pobj->pSoundMaxPtr ) pobj->pSoundBufPtr = pobj->pSoundBufPtr - pobj->pSoundMaxPtr + pobj->pSoundBuf;
    OSRestoreInterrupts( oldInterrupts );
  }
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerCreate
     Description:    Initializes the player. This starts the decoding thread 
                     and playback thread. The argument ppFrameBuff references 
                     an array of pointers to allocated frame buffers, and 
                     iFrameCnt specifies the number of frame buffers. If 
                     iFrameCnt is zero, the player does not transfer decoded 
                     images to the frame buffer.
     Arguments:      pframebuf    Array of pointers to frame buffers
                     iframecnt    Count of frame buffers
     Returns:        none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerCreate( void **ppFrameBuff,int iFrameCnt ){
  pFrameBufPtr = ppFrameBuff;
  iFrameBufCnt = iFrameCnt;
  PlayerObjPtr = NULL;

  if( iInitializePlayer ) return;

  /* Initialize HVQM4 Movie */
  HVQM4InitDecoder();

  /* Create picture decode thread */
  OSCreateThread(
    &DecodeThread,
    DecodeThreadFunc,
    NULL,
    DecodeThreadStack + sizeof( DecodeThreadStack ),
    sizeof( DecodeThreadStack ),
    31,
    OS_THREAD_ATTR_DETACH );

  /* Start picture decode thread */
  OSResumeThread( &DecodeThread );

  iInitializePlayer = 1;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerRelease
     Description:    This releases the decoding thread and playback thread 
                     started by HVQM4PlayerCreate().
     Arguments:      none
     Returns:        none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerRelease( void ){

  pFrameBufPtr = NULL;
  iFrameBufCnt = 0;
  iInitializePlayer = 0;
  while( IsDecodeThread() ){
    VIWaitForRetrace();
  }
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4CreateReadThread/HVQM4PlayerOpen
     Description:    Opens and initializes HVQM4 file.
                     lpszfilename specifies the file to open. The parameters 
                     required for playback are set in pobj. 
     Arguments:      lpszfilename    Name of file to be played back
                     pobj            Pointer to HVQM4 player status
     Returns:        0               Normal termination
                     Nonzero         Error
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerOpen( char *lpszFilename, HVQM4PlayerObj *pobj ){
  s32	ret = 0;
  int	i;

  HVQM4CreateReadThread(lpszFilename);

  memset( pobj,0,sizeof( HVQM4PlayerObj ) );

  while( HVQM4GetStatus() != TstNORMAL ){};

  /* Read header */
  ret = HVQM4Read2Buf( ( void * )( &( pobj->sHeader ) ),( s32 )sizeof( HVQM4Header ) );
  if( ret == -1 ){
#ifdef _DEBUG
    OSReport( "Error: broken file\n" );
#endif
    return 1;
  }

  if( strncmp( HVQM4_FILEVERSION,( char * )pobj->sHeader.file_version,strlen( HVQM4_FILEVERSION ) ) != 0 ){
#ifdef _DEBUG
    OSReport( "Error: Invalid version\n" );
#endif
    return 1;
  }

  pobj->cAudioSample = 0xff;
  if( pobj->sHeader.audioinfo.samples_per_sec ){
    if( pobj->sHeader.audioinfo.samples_per_sec == 48042) {
      if ( AIGetDSPSampleRate() != AI_SAMPLERATE_48KHZ ) {
        #ifdef _DEBUG
          OSReport( "Error: Invalid audio samples per second \n" );
        #endif
	HVQM4StopRead();
	while( HVQM4GetStatus() != TstNORMAL ){};
        return 1;
      } else {
        pobj->cAudioSample = AI_SAMPLERATE_48KHZ;
      }
    } else if( pobj->sHeader.audioinfo.samples_per_sec == 32028 ) {
      if ( AIGetDSPSampleRate() != AI_SAMPLERATE_32KHZ ) {
        #ifdef _DEBUG
          OSReport( "Error: Invalid audio samples per second \n" );
        #endif
	HVQM4StopRead();
	while( HVQM4GetStatus() != TstNORMAL ){};
        return 1;
      } else {
        pobj->cAudioSample = AI_SAMPLERATE_32KHZ;
      }
    } else {
      #ifdef _DEBUG
        OSReport( "Error: Invalid audio samples per second \n" );
      #endif
      HVQM4StopRead();
      while( HVQM4GetStatus() != TstNORMAL ){};
      return 1;
    }
  }

  HVQM4InitSeqObj( &pobj->sSeqObj,&pobj->sHeader.videoinfo );
  if( ( HVQM4BuffSize( &pobj->sSeqObj ) ) == 0 ){
#ifdef _DEBUG
    OSReport( "Error: failed to allocate the working buffer\n" );
#endif
    return 1;
  }

  pobj->usWidth = HVQM4GetWidth( &pobj->sSeqObj );
  pobj->usHeight = HVQM4GetHeight( &pobj->sSeqObj );
  pobj->usTop = ( u16 )( (480 - pobj->usHeight) / 2 );
  pobj->usLeft = ( u16 )( (640 - pobj->usWidth) / 2 );
  
  for( i = 0 ; i < HVQM4_DEC_BUFFER_CNT ; i++ ){
    pobj->psPlayerData[i] = &pobj->sPlayerData[i];
  }
  return 0;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4CloseRead/HVQM4PlayerClose
     Description:    Closes HVQM4 file specified by pobj.   
     Arguments:      pobj       Pointer to HVQM4 player status
     Returns:        0          Normal termination
                     Nonzero    Error
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerClose( HVQM4PlayerObj *pobj ) {
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
#ifdef _DEBUG
  OSReport( "Check: Call HVQM4PlayerClose.\n" );
#endif
  HVQM4CloseRead();
  memset( pobj,0,sizeof( HVQM4PlayerObj ) );
  while( HVQM4GetStatus() != TstCLOSED ){};
  return 0;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerPlay
     Description:    Starts playback of the HVQM4 sequence specified by pobj.
                     If playback is paused, this resumes the playback.
                     If multiple sequences are played back, the operation is undefined.
                     The argument flags contains the following flags.
                       #define HVQM4PLAYER_PLAY_NONE	(0)
                       #define HVQM4PLAYER_PLAY_PICTURE	(1<<0)
                       #define HVQM4PLAYER_PLAY_AUDIO	(1<<1)
                       #define HVQM4PLAYER_PLAY_REPEAT	(1<<2)
     Arguments:      pobj       Pointer to HVQM4 player status
                     flags      Flags playback
     Returns:        0          Normal termination
                     Nonzero    Error
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerPlay( HVQM4PlayerObj *pobj,u8 flags ){
  s32	ret = 0;

  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }

  if( pobj->cStatus == HVQM4PLAYER_STATUS_NONE || pobj->cStatus == HVQM4PLAYER_STATUS_STOP ){
    pobj->cFlags = flags;

    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ){
      if( HVQM4Read2Buf( NULL,( s32 )sizeof( HVQM4Header ) ) == -1 ){
  #ifdef _DEBUG
	OSReport( "Error: broken file\n" );
  #endif
	return 1;
      }
    }
    ret = HVQM4Read2Buf( ( void * )&( pobj->sGopHdr ),( s32 )sizeof( HVQM4GOPHeader ) );
    if( ret == -1 ){
  #ifdef _DEBUG
      OSReport( "Error: broken file\n" );
  #endif
      return 1;
    }
    if(( flags & HVQM4PLAYER_PLAY_AUDIO ) && (pobj->cAudioSample == 0xff)) {
      pobj->cFlags = (u8)(pobj->cFlags - HVQM4PLAYER_PLAY_AUDIO);
    }
    pobj->ulGopCount = 0;
    pobj->ulGopTopFrame = 0;
    pobj->ulDecodePicCnt = 0;
    pobj->ulDecodeAuCnt = 0;
    pobj->ulDecodeTotal = 0;
    pobj->ulDecodeFrame = 0;
    pobj->ulStartTime = 0;
    pobj->ullAudioIndex = 0;
    pobj->pSoundBufPtr = pobj->pSoundDecPtr = pobj->pSoundBuf;

    PlayerObjPtr = pobj;
    pobj->cStatus = HVQM4PLAYER_STATUS_PLAY | HVQM4PLAYER_STATUS_START;
  } else if( pobj->cStatus & HVQM4PLAYER_STATUS_PAUSE ){
    pobj->cStatus = HVQM4PLAYER_STATUS_PLAY;
    pobj->ulStartTime += OSGetTime() - pobj->ulPauseTime;
  }
  return 0;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerPause
     Description:    Pauses playback of the HVQM4 sequence specified by pobj.
     Arguments:      pobj       Pointer to HVQM4 player status
     Returns:        0          Normal termination
                     Nonzero    Error
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerPause( HVQM4PlayerObj *pobj ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
  if( !( pobj->cStatus & HVQM4PLAYER_STATUS_PLAY ) ) return 1;
  pobj->cStatus = HVQM4PLAYER_STATUS_PAUSE;
  pobj->ulPauseTime = OSGetTime();
  return 0;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4StopRead
     Description:    Stops playback or pause of HVQM4 sequence specified by pobj, 
                     and put back the stream pointer to the beginnig.
     Arguments:      pobj       Pointer to HVQM4 player status
     Returns:        0          Normal termination
                     Nonzero    Error
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerStop( HVQM4PlayerObj *pobj ){
  int	i;

  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
  PlayerObjPtr = NULL;

  pobj->cStatus = HVQM4PLAYER_STATUS_STOP;
  if( HVQM4ErrStatus() == DVD_STATE_FATAL_ERROR ){
    HVQM4SendErr();
  } else {
    HVQM4StopRead();
    while( HVQM4GetStatus() != TstNORMAL ){};
  }
  while( pobj->bDecode ){
    VIWaitForRetrace();
  };

  for( i = 0 ; i < HVQM4_DEC_BUFFER_CNT ; i++ ){
    pobj->sPlayerData[i].state = 0;
  }
  pobj->ulGopCount = 0;
  pobj->ulGopTopFrame = 0;
  pobj->ulDecodePicCnt = 0;
  pobj->ulDecodeAuCnt = 0;
  pobj->ulDecodeTotal = 0;
  pobj->ulDecodeFrame = 0;
  pobj->ulStartTime = 0;
  pobj->ullAudioIndex = 0;
  pobj->pSoundBufPtr = pobj->pSoundDecPtr = pobj->pSoundBuf;
  return 0;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerStatus
     Description:    Return to state.
     Arguments:      pobj       Pointer to HVQM4 player status
     Returns:        status
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerStatus( HVQM4PlayerObj *pobj ){

  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return ( u32 )( HVQM4PLAYER_STATUS_ERRORFLAG | HVQM4PLAYER_STATUS_NULL );
  }

  if( pobj->cStatus & HVQM4PLAYER_STATUS_STOP ) return ( u32 )HVQM4PLAYER_STATUS_STOP;
  switch( HVQM4ErrStatus() ){
    case DVD_STATE_FATAL_ERROR : return ( u32 )( HVQM4PLAYER_STATUS_ERRORFLAG | HVQM4PLAYER_STATUS_READERROR );
    case DVD_STATE_IGNORED :
    case DVD_STATE_END : break;
    default : return ( u32 )( HVQM4PLAYER_STATUS_ERRORFLAG | HVQM4ErrStatus() );
  }
  if( pobj->cStatus & HVQM4PLAYER_STATUS_PAUSE ) return ( u32 )( pobj->cStatus & ( HVQM4PLAYER_STATUS_PAUSE | HVQM4PLAYER_STATUS_OPEN ) );
  if( pobj->cStatus & HVQM4PLAYER_STATUS_PLAY ) return ( u32 )HVQM4PLAYER_STATUS_PLAY;
  return ( u32 )HVQM4PLAYER_STATUS_NONE;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerGetBufferSize
     Description:    Gets the work buffer size required for playback of the 
                     HVQM4 sequence specified by pobj.
     Arguments:      pobj       Pointer to HVQM4 player status
     Returns:        Required buffer size (bytes)
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerGetBufferSize( HVQM4PlayerObj *pobj ){
  u32	ret = 0;
  u32	width,height,h_samp,v_samp;

  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
  width = HVQM4GetWidth( &pobj->sSeqObj );
  height = HVQM4GetHeight( &pobj->sSeqObj );
  h_samp = HVQM4GetSamplingRateH( &pobj->sSeqObj );
  v_samp = HVQM4GetSamplingRateV( &pobj->sSeqObj );

  ret = OSRoundUp32B( pobj->sHeader.max_frame_size );
  ret += OSRoundUp32B( pobj->sHeader.max_audio_record_size );
  ret += ( ( OSRoundUp32B( pobj->sHeader.max_audio_record_size ) + DMA_BLOCK_SIZE - 1 ) / DMA_BLOCK_SIZE ) * DMA_BLOCK_SIZE * 4 * ( HVQM4_DEC_BUFFER_CNT + 1 );
  ret += OSRoundUp32B( (width * height * (h_samp * v_samp + 2)) / (h_samp * v_samp) ) * HVQM4_DEC_BUFFER_CNT;
  ret += OSRoundUp32B( HVQM4BuffSize( &pobj->sSeqObj ) );
  return ret;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerSetBuffer
     Description:    Specifies the work buffer to be used for playback of the 
                     HVQM4 sequence specified by pobj.
     Arguments:      pobj       Pointer to HVQM4 player status
                     pbuffer    Pointer to work buffer
     Returns:        0          Normal termination
                     Nonzero    Error
 *---------------------------------------------------------------------------*/

u32 HVQM4PlayerSetBuffer( HVQM4PlayerObj *pobj,u8 *pbuffer ){
  u32	width,height,h_samp,v_samp,bufsize;
  int	i;

  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
  width = HVQM4GetWidth( &pobj->sSeqObj );
  height = HVQM4GetHeight( &pobj->sSeqObj );
  h_samp = HVQM4GetSamplingRateH( &pobj->sSeqObj );
  v_samp = HVQM4GetSamplingRateV( &pobj->sSeqObj );
  bufsize = ( ( OSRoundUp32B( pobj->sHeader.max_audio_record_size ) + DMA_BLOCK_SIZE - 1 ) / DMA_BLOCK_SIZE ) * DMA_BLOCK_SIZE * 4 * ( HVQM4_DEC_BUFFER_CNT + 1 );

  HVQM4SetBuffer( &pobj->sSeqObj,pbuffer );
  pobj->pPicReadBuf = ( pbuffer + OSRoundUp32B( HVQM4BuffSize( &pobj->sSeqObj ) ) );
  pobj->pSoundReadBuf = ( pobj->pPicReadBuf + OSRoundUp32B( pobj->sHeader.max_frame_size ) );
  memset( pobj->pSoundReadBuf,0,bufsize );
  pobj->sAdpcmState[0].bufstart = pobj->sAdpcmState[1].bufstart = pobj->pSoundBuf = ( pobj->pSoundReadBuf + OSRoundUp32B( pobj->sHeader.max_audio_record_size ) );
  pobj->pSoundBufPtr = pobj->pSoundDecPtr = pobj->pSoundBuf;
  pobj->sAdpcmState[0].bufend = pobj->sAdpcmState[1].bufend = pobj->pSoundMaxPtr = pobj->sPlayerData[0].buf = ( pobj->pSoundBuf + bufsize );
  for( i = 1 ; i < HVQM4_DEC_BUFFER_CNT ; i++ ){
    pobj->sPlayerData[i].buf = ( pobj->sPlayerData[i-1].buf + OSRoundUp32B( (width * height * (h_samp * v_samp + 2)) / (h_samp * v_samp) ) );
  }
  return 0;
}

/*---------------------------------------------------------------------------*
     Name:           HVQM4PlayerGetWidth
     Description:    Returns the image width (pixels) of the HVQM4 sequence 
                     specified by pobj.   
     Arguments:      pobj    Pointer to HVQM4 player status
     Returns:        Image width (pixels)
 *---------------------------------------------------------------------------*/

u16 HVQM4PlayerGetWidth( HVQM4PlayerObj *pobj ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
  return HVQM4GetWidth( &pobj->sSeqObj );
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerGetHeight
    Description:    Returns the image height (pixels) of the HVQM4 sequence 
                    specified by pobj.     
    Arguments:      pobj    Pointer to HVQM4 player status
    Returens:       Image height (pixels)
 *---------------------------------------------------------------------------*/

u16 HVQM4PlayerGetHeight( HVQM4PlayerObj *pobj ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return 1;
  }
  return HVQM4GetHeight( &pobj->sSeqObj );
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerSetDonePictureDecode
    Description:    Registers a callback function to be called when decoding 
                    of the video records of the HVQM4 sequence specified by 
                    pobj is completed. A pointer to the buffer holding the 
                    decoded image is passed in the pPicture argument of 
                    funcDonePictureDecode. The frame number of the image (
                    number in display order from the beginning of the sequence ) 
                    is passed in ulFrame. Changing the display sequence of the 
                    images is done within the player.
    Arguments:     pobj                     Pointer to HVQM4 player status
                   funcDonePictureDecode    Pointer to callback function
    Returens:      none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerSetDonePictureDecode( HVQM4PlayerObj *pobj,void funcDonePictureDecode( u8 *pPicture,u32 ulFrame, void *data ) ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return;
  }
  pobj->funcPictureDecodeEnd = funcDonePictureDecode;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerSetDoneAudioDecode
    Description:    Registers a callback function to be called when decoding of 
                    the audio records of the HVQM4 sequence specified by pobj 
                    is completed. A pointer to the buffer holding the decoded 
                    audio is passed in the pAudio argument of funcDoneAudioDecode. 
                    The size of the audio data is passed in ulSize.
    Arguments:      pobj                   Pointer to HVQM4 player status
                    funcDoneAudioDecode    Pointer to callback function
    Returens:       none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerSetDoneAudioDecode( HVQM4PlayerObj *pobj,void funcDoneAudioDecode( u8 *pAudio,u32 ulSize ) ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return;
  }
  pobj->funcAudioDecodeEnd = funcDoneAudioDecode;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerSetDispPicture
    Description:    Registers a callback function to be called at the timing of 
                    the display of an image of which decoding has been completed. 
                    The current frame number is passed in the ulCurrentFrame 
                    argument of funcDispPicture.
    Arguments:      pobj               Pointer to HVQM4 player status
                    funcDispPicture    Pointer to callback function
    Returens:       none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerSetDispPicture( HVQM4PlayerObj *pobj,void funcDispPicture( u32 ulCurrentFrame ) ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return;
  }
  pobj->funcDrawPicture = funcDispPicture;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerSetDrawScreen
    Description:    Sets the screen for use when the player draws directly 
                    in an external frame buffer.
                    If the screen is set to extend beyond the region of the 
                    external frame buffer, operation is not guaranteed.
    Arguments:      pobj         Pointer to HVQM4 player status
                    left, top    Screen start position
                    width        Screen width
                    height       Screen height
    Returens:       none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerSetDrawScreen( HVQM4PlayerObj *pobj,u16 left,u16 top,u16 width,u16 height ){
  if( !pobj ){
#ifdef _DEBUG
    OSReport( "Error: pobj is NULL\n" );
#endif
    return;
  }
  pobj->usTop = top;
  pobj->usLeft = left;
  pobj->usWidth = width;
  pobj->usHeight = height;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerSetBeforeDraw
    Description:    Registers a callback function to be called immediately 
                    before the player renders an image to the frame buffer.
                    A pointer to the frame buffer which will be rendered to 
                    is passed in the ppFrameBuff argument of funcBeforeDraw.
                    However, if the pointer to the frame buffer was not set 
                    with HVQM4PlayerCreate() (when iFrameCnt was specified 
                    as zero), NULL is passed.
    Arguments:      funcBeforeDraw    Pointer to callback function
    Returens:       none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerSetBeforeDraw( void funcDrawStart( void **ppFrameBuffer ) ){
  BeforeDraw = funcDrawStart;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerSetDoneDraw
    Description:    Registers a callback function to be called immediately after 
                    the player has rendered an image to the frame buffer.
                    A pointer to the frame buffer which has rendered to is passed 
                    in the pFrameBuff argument of funcDoneDraw. However, if the 
                    pointer to the frame buffer was not set with HVQM4PlayerCreate() 
                    (when iFrameCnt was specified as zero), NULL is passed.
    Arguments:      funcDoneDraw    Pointer to callback function
    Returens:       none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerSetDoneDraw( void funcDoneDraw( void *pFrameBuffer,int iDrawFlag ) ){
  DrawDone = funcDoneDraw;
}

/* Rearrange YUV data after decoding */ 
#define FBWidth	    ( 640 / 2 )
#define FBHeight    ( 480 / 2 )

static void Yuv4112yuv(u8 *pSrc,u8 *pDst,int top,int left,int nWidth,int nHeight,int oWidth,int oHeight){
  u16	px,py;
  u32	*xfb,*xfbptr;
  u8	*yP1,*yP2,*uP,*vP;
  u32	u,v;
  int	dWidth;
  int	dY;

  if( nWidth > oWidth ) nWidth=oWidth;
  if( nHeight > oHeight ) nHeight=oHeight;

  xfb = ( u32 * )pDst;

  /* Draw image on XFB */
  yP1 = pSrc;
  yP2 = yP1 + oWidth;
  uP = yP1 + oWidth * oHeight;
  vP = uP + ( oWidth >> 1 ) * ( oHeight >> 1 );

  dWidth = oWidth - nWidth;
  dY = 2 * FBWidth - oWidth / 2;

  xfbptr = xfb + top * FBWidth + left / 2;
  for( py = 0 ; py < nHeight ; py += 2 ){
    for( px = 0 ; px < oWidth ; px += 8 ){
      u = ( u32 )( ( uP[0] ) << 16 );
      v = vP[0];
      if( px < nWidth ) xfbptr[0] = ( ( ( yP1[0] ) << 24 ) | u ) | ( ( ( yP1[1] ) << 8 ) | v );
      if( px < nWidth ) xfbptr[FBWidth] = ( ( ( yP2[0] ) << 24) | u ) | ( ( ( yP2[1] ) << 8 ) | v );

      u = ( u32 )( ( uP[1] ) << 16 );
      v = vP[1];
      if( px < nWidth ) xfbptr[1] = ( ( ( yP1[2] ) << 24 ) | u ) | ( ( ( yP1[3] ) << 8 ) | v );
      if( px < nWidth ) xfbptr[FBWidth + 1] = ( ( ( yP2[2] ) << 24 ) | u ) | ( ( ( yP2[3] ) << 8 ) | v );

      u = ( u32 )( ( uP[2] ) << 16 );
      v = vP[2];
      if( px < nWidth ) xfbptr[2] = ( ( ( yP1[4] ) << 24 ) | u ) | ( ( ( yP1[5] ) << 8 ) | v );
      if( px < nWidth ) xfbptr[FBWidth + 2] = ( ( ( yP2[4] ) << 24 ) | u ) | ( ( ( yP2[5]) << 8 ) | v );

      u = ( u32 )( ( uP[3] ) << 16 );
      v = vP[3];
      if( px < nWidth ) xfbptr[3] = ( ( ( yP1[6] ) << 24 ) | u ) | ( ( ( yP1[7] ) << 8 ) | v );
      if( px < nWidth ) xfbptr[FBWidth + 3] = ( ( ( yP2[6] ) << 24 ) | u ) | ( ( ( yP2[7]) << 8 ) | v );

      uP += 4;
      vP += 4;
      yP1 += 8;
      yP2 += 8;
      xfbptr += 4;
    }

    yP1 = yP2;
    yP2 += oWidth;
    xfbptr += dY;
  }
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4PlayerDrawPicture
    Description:    The actual drawing function, which must be called on 
                    the display timing.
    Arguments:	    none
    Returens:	    none
 *---------------------------------------------------------------------------*/

void HVQM4PlayerDrawPicture( void ){
  int		    j;
  u32		    fbSize;
  int		    flag = 0;
  void		    *CurrentFrameBufPtr = pFrameBufPtr[iCurrentFrameBuf];
  HVQM4PlayerObj    *pobj = NULL;
  OSTime	    diffTime;

  if( iInitializePlayer ){
    pobj = PlayerObjPtr;
    if( !pobj ) return;

    if( !( pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ) && !( pobj->cStatus & HVQM4PLAYER_STATUS_OPEN ) && HVQM4ErrStatus() == DVD_STATE_COVER_OPEN ){
      HVQM4PlayerPause( pobj );
      pobj->cStatus |= HVQM4PLAYER_STATUS_OPEN;
    }
    if( HVQM4PlayerStatus( pobj ) == ( HVQM4PLAYER_STATUS_OPEN | HVQM4PLAYER_STATUS_PAUSE ) ||
      HVQM4PlayerStatus( pobj ) == ( DVD_STATE_COVER_CLOSED | HVQM4PLAYER_STATUS_ERRORFLAG ) ){
      HVQM4PlayerPlay( pobj,pobj->cFlags );
    }
    if( BeforeDraw ) BeforeDraw( &CurrentFrameBufPtr );
    if( !( pobj->cStatus & HVQM4PLAYER_STATUS_PLAY ) ) return;
    if( pobj->cStatus & HVQM4PLAYER_STATUS_START ){
      for( j = 0 ; j < HVQM4_DEC_BUFFER_CNT ; j++ ){
	if( pobj->sPlayerData[j].state != 1 ){
          flag = 1;
        }
      }
      if( flag ) return;
      if( pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ){
  	pobj->ullAudioIndex = 0;
      } else {
	pobj->ulStartTime = OSGetTime();
      }
      pobj->cStatus = HVQM4PLAYER_STATUS_PLAY;
    }
    if( pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ){
      diffTime = OSGetTime() - pobj->ulStartTime;
      pobj->ulCurrentFrame = ( u32 )( ( ( pobj->ullAudioIndex * 1000000 / ( 4 * pobj->sHeader.audioinfo.samples_per_sec ) ) + OSTicksToMicroseconds( diffTime ) ) / pobj->sHeader.usec_per_frame );
    } else {
      pobj->ulCurrentFrame = ( u32 )( OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) / pobj->sHeader.usec_per_frame );
    }
    for( j = 0; j < HVQM4_DEC_BUFFER_CNT ; j++ ){
      if( pobj->sPlayerData[j].state == 1 && pobj->ulCurrentFrame == pobj->sPlayerData[j].frame ){
        if( pobj->cFlags & HVQM4PLAYER_PLAY_PICTURE ){
  	  Yuv4112yuv( pobj->sPlayerData[j].buf,(unsigned char*)CurrentFrameBufPtr,(int)pobj->usTop,(int)pobj->usLeft,(int)pobj->usWidth,pobj->usHeight,(int)HVQM4PlayerGetWidth( pobj ),(int)HVQM4PlayerGetHeight( pobj ) );
	  fbSize = 640 * 480 * 2;
	  DCStoreRange( CurrentFrameBufPtr,fbSize );
	}
        if( pobj->funcPictureDecodeEnd ) pobj->funcPictureDecodeEnd( pobj->sPlayerData[j].buf,pobj->ulCurrentFrame, (void*)pobj );
	flag = 1;
	pobj->sPlayerData[j].state = 2;
      }
    }
    if( pobj->funcDrawPicture ) pobj->funcDrawPicture( pobj->ulCurrentFrame );
  }
  if( DrawDone ){
    DrawDone( CurrentFrameBufPtr,flag );
    iCurrentFrameBuf = ( ( iCurrentFrameBuf + 1 ) >= iFrameBufCnt ) ? 0 : iCurrentFrameBuf + 1;
  }
}

/* end */
