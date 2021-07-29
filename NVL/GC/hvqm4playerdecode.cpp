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
 * File:	hvqm4playerdecode.c
 *
 * (C) 2000-2001 HUDSON SOFT
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hvqm4player.h"
#include "nvlstream_gc.h"

/*---------------------------------------------------------------------------*
    Variables
 *---------------------------------------------------------------------------*/

extern int		iInitializePlayer;
extern HVQM4PlayerObj	*PlayerObjPtr;
static int		bDecodeThread = 0;

/*---------------------------------------------------------------------------*
    Declaration of function prototype 
 *---------------------------------------------------------------------------*/

void *DecodeThreadFunc( void *param );
static int VideoDecode( HVQM4PlayerObj *pobj,u32 wFormat,u32 dwFrame );
static int OneDecode( HVQM4PlayerObj *pobj );
int IsDecodeThread( void );
s32 HVQM4Read2Buf(void *addr,s32 length);

/*---------------------------------------------------------------------------*
    Check thread status
 *---------------------------------------------------------------------------*/

int IsDecodeThread( void ){
  return bDecodeThread;
}

/*---------------------------------------------------------------------------*
    Thread to be decoded
 *---------------------------------------------------------------------------*/

void HVQM4StopRead(void);
TST HVQM4GetStatus(void);
void *DecodeThreadFunc( void *param ){
  int		    j,flag = 0,ret = 0;
  HVQM4PlayerObj    *pobj = NULL;
  #pragma unused( param )
  
  bDecodeThread = 1;
  while( iInitializePlayer ){
    pobj = PlayerObjPtr;
    if( !pobj ) continue;
    if( !( pobj->cStatus & HVQM4PLAYER_STATUS_PLAY ) ) continue;
    ret = OneDecode( pobj );
    if( ret == 2 ){
      if( pobj->cFlags & HVQM4PLAYER_PLAY_REPEAT ){
	do {
	  if( pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ){
	    pobj->ulCurrentFrame = ( u32 )( ( ( pobj->ullAudioIndex * 1000000 / ( 4 * pobj->sHeader.audioinfo.samples_per_sec ) ) + OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) ) / pobj->sHeader.usec_per_frame );
	  } else {
	    pobj->ulCurrentFrame = ( u32 )( OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) / pobj->sHeader.usec_per_frame );
	  }
	} while( pobj->ulCurrentFrame < pobj->sHeader.video_total );
	for( j = 0 ; j < HVQM4_DEC_BUFFER_CNT ; j++ ){
	  pobj->sPlayerData[j].state = 0;
	}
  	if( HVQM4Read2Buf( ( void * )( &( pobj->sHeader ) ),( s32 )sizeof( HVQM4Header ) ) == -1 ){
#ifdef _DEBUG
	  OSReport( "Error: broken file.\n" );
#endif
	  pobj->cStatus = HVQM4PLAYER_STATUS_STOP;
	  PlayerObjPtr = NULL;
          pobj->bDecode = 0;
	  continue;
	}
	if( HVQM4Read2Buf( ( void * )&( pobj->sGopHdr ),( s32 )sizeof( HVQM4GOPHeader ) ) == -1 ){
#ifdef _DEBUG
	  OSReport( "Error: broken file.\n" );
#endif
	  pobj->cStatus = HVQM4PLAYER_STATUS_STOP;
	  PlayerObjPtr = NULL;
          pobj->bDecode = 0;
	  continue;
	}
	pobj->cStatus |= HVQM4PLAYER_STATUS_START;
        pobj->ulGopCount = 0;
        pobj->ulGopTopFrame = 0;
        pobj->ulDecodePicCnt = 0;
	pobj->ulDecodeAuCnt = 0;
	pobj->ulDecodeTotal = 0;
	pobj->ulDecodeFrame = 0;
	pobj->ulStartTime = 0;
	pobj->ullAudioIndex = 0;
	pobj->pSoundBufPtr = pobj->pSoundDecPtr = pobj->pSoundBuf;
      } else {
	do {
	  if( pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ){
	    pobj->ulCurrentFrame = ( u32 )( ( ( pobj->ullAudioIndex * 1000000 / ( 4 * pobj->sHeader.audioinfo.samples_per_sec ) ) + OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) ) / pobj->sHeader.usec_per_frame );
	  } else {
	    pobj->ulCurrentFrame = ( u32 )( OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) / pobj->sHeader.usec_per_frame );
	  }
	} while( pobj->ulCurrentFrame < pobj->sHeader.video_total );

	PlayerObjPtr = NULL;
        HVQM4StopRead();
        while( HVQM4GetStatus() != TstNORMAL ){};
	pobj->cStatus = HVQM4PLAYER_STATUS_STOP;
      }
    } else if( ret == 1 ){
#ifdef _DEBUG
      OSReport( "Error: broken file.\n" );
#endif
      pobj->cStatus = HVQM4PLAYER_STATUS_STOP;
      PlayerObjPtr = NULL;
    }
    pobj->bDecode = 0;
  }
#ifdef _DEBUG
  OSReport( "End of Decode Thread.\n" );
#endif
  bDecodeThread = 0;
  return NULL;
}

/*---------------------------------------------------------------------------*
    Decode one object
 *---------------------------------------------------------------------------*/

void HVQM4StopRead(void);
static int OneDecode( HVQM4PlayerObj *pobj ){
  u16		  wType;
  u16		  wFormat;
  u32		  dwSize;
  u32		  dwFrame;
  u32		  dwSamples;
  s32		  ret = 0;
  OSTime	  time,time2;
  u32		  diff;
  
  pobj->bDecode = 1;
  if( pobj->ulDecodePicCnt == pobj->sGopHdr.vidrec_number && pobj->ulDecodeAuCnt == pobj->sGopHdr.audrec_number ){
    pobj->ulGopTopFrame += pobj->sGopHdr.vidrec_number;
    pobj->ulGopCount++;
    if( pobj->ulGopCount < pobj->sHeader.gop_total ){
      if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
      ret = HVQM4Read2Buf( ( void * )&( pobj->sGopHdr ),sizeof( HVQM4GOPHeader ) );
      if( ret == -1 ) return 1;
      pobj->ulDecodePicCnt = 0;
      pobj->ulDecodeAuCnt = 0;
    } else {
      return 2;
    }
  }

  ret = HVQM4Read2Buf( ( void * )( &( pobj->sRecHeader ) ),sizeof( HVQM4RecHeader ) );
  if( ret == -1 ) return 1;
  wType = pobj->sRecHeader.type;
  wFormat = pobj->sRecHeader.format;
  dwSize = pobj->sRecHeader.size;

  if( wType == HVQM4_VIDEO ){

    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
    ret = HVQM4Read2Buf( ( void * )pobj->pPicReadBuf,sizeof( u32 ) );
    if( ret == -1 ) return 1;
    dwFrame = ( ( u32 * )pobj->pPicReadBuf )[0];

    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
    ret = HVQM4Read2Buf( ( void * )pobj->pPicReadBuf,( s32 )( dwSize - 4 ) );
    if( ret == -1 ) return 1;

DECODERETRY:
    time = OSGetTime();
    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0; 
    switch( VideoDecode( pobj,wFormat,dwFrame ) ){
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
        if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
	time2 = OSGetTime();
        pobj->ulDecodeTotal += OSTicksToMicroseconds( time2 - time );
	pobj->ulDecodeFrame++;
	break;
    }
    pobj->ulDecodePicCnt++;
  } else if( wType == HVQM4_AUDIO ){
    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
    ret = HVQM4Read2Buf( ( void * )pobj->pSoundReadBuf,sizeof( u32 ) );
    if( ret == -1 ) return 1;
    dwSamples = ( ( u32 * )pobj->pSoundReadBuf )[0];

    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
    ret = HVQM4Read2Buf( ( void * )pobj->pSoundReadBuf,( s32 )( dwSize - 4 ) );
    if( ret == -1 ) return 1;

    diff = ( u32 )( pobj->pSoundDecPtr + dwSamples * 4 );
DECODERETRY2:
    if( diff >= ( u32 )pobj->pSoundMaxPtr ) diff -= ( u32 )( pobj->pSoundMaxPtr - pobj->pSoundBuf );
    if( ( u32 )pobj->pSoundDecPtr < ( u32 )pobj->pSoundBufPtr && ( u32 )diff >= ( u32 )pobj->pSoundBufPtr ){
#ifdef _DEBUG
      OSReport( "Audio Decode Retry\n" );
#endif
      goto DECODERETRY2;
    }

    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
    HVQM4ADPCMDecode( pobj->pSoundReadBuf,pobj->pSoundDecPtr,pobj->sHeader.audioinfo.channels,wFormat,dwSamples,pobj->sAdpcmState );
    if( ( u32 )pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr < ( u32 )pobj->pSoundDecPtr ){
      dwSize = ( u32 )pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr + ( u32 )( pobj->pSoundMaxPtr - pobj->pSoundBuf ) - ( u32 )pobj->pSoundDecPtr;
    } else {
      dwSize = ( u32 )pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr - ( u32 )pobj->pSoundDecPtr;
    }
    if( pobj->funcAudioDecodeEnd ) pobj->funcAudioDecodeEnd( pobj->pSoundDecPtr,dwSize );

    if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
    pobj->pSoundDecPtr = ( u8 * )pobj->sAdpcmState[pobj->sHeader.audioinfo.channels - 1].outPtr;
    pobj->ulDecodeAuCnt++;
  } else {
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*
  Decode image
 *---------------------------------------------------------------------------*/

static int VideoDecode( HVQM4PlayerObj *pobj,u32 wFormat,u32 dwFrame ){
  u32 dwPrevFrame = ( !pobj->ulDecodeFrame ) ? 0 : ( u32 )( ( pobj->ulDecodeTotal / pobj->ulDecodeFrame ) / ( pobj->sHeader.usec_per_frame ) );
  u32 dwCurrentFrame;
  HVQM4PlayerData  *tmp;
  u8  cnt;
  int flag = 0;

  if( pobj->cFlags & HVQM4PLAYER_PLAY_AUDIO ){
    dwCurrentFrame = ( !pobj->ullAudioIndex ) ? 0 : ( u32 )( ( pobj->ullAudioIndex * 1000000 / ( 4 * pobj->sHeader.audioinfo.samples_per_sec ) ) / pobj->sHeader.usec_per_frame );
  } else {
    dwCurrentFrame = ( !pobj->ulStartTime ) ? 0 : ( u32 )( OSTicksToMicroseconds( OSGetTime() - pobj->ulStartTime ) / pobj->sHeader.usec_per_frame );
  }
  dwPrevFrame = ( dwCurrentFrame ) ? dwPrevFrame + 1 : 0;

  switch( wFormat ){
    case HVQM4_VIDEO_I_PIC:
    case HVQM4_VIDEO_P_PIC:
      for( cnt = HVQM4_BPIC_BUFFER_START ; cnt < HVQM4_DEC_BUFFER_CNT ; cnt++ ){
	if( pobj->psPlayerData[cnt]->state != 1 || dwCurrentFrame > pobj->psPlayerData[cnt]->frame ){
	  tmp = pobj->psPlayerData[cnt];
	  pobj->psPlayerData[cnt] = pobj->psPlayerData[0];
	  pobj->psPlayerData[0] = pobj->psPlayerData[1];
	  pobj->psPlayerData[1] = tmp;
	  flag = 1;
	  break;
	}
      }
      if( !flag ) return 2;
      pobj->psPlayerData[1]->state = 0;
      if( ( dwCurrentFrame + dwPrevFrame ) > dwFrame + pobj->ulGopTopFrame ){
	return 3;
      }
      if( wFormat == HVQM4_VIDEO_I_PIC ){
	HVQM4DecodeIpic( &pobj->sSeqObj,pobj->pPicReadBuf,pobj->psPlayerData[1]->buf );
      } else {
	if( !pobj->psPlayerData[0]->state ){
	  return 3;
	}
        HVQM4DecodePpic( &pobj->sSeqObj,pobj->pPicReadBuf,pobj->psPlayerData[1]->buf,pobj->psPlayerData[0]->buf );
      }
      pobj->psPlayerData[1]->frame = dwFrame + pobj->ulGopTopFrame;
      if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
      pobj->psPlayerData[1]->state = 1;
      break;
    case HVQM4_VIDEO_B_PIC:
      if( !pobj->psPlayerData[0]->state ) return 3;
      if( !pobj->psPlayerData[1]->state ) return 3;
      if( ( dwCurrentFrame + dwPrevFrame ) > dwFrame + pobj->ulGopTopFrame ){
	return 3;
      }
      for( cnt = HVQM4_BPIC_BUFFER_START ; cnt < HVQM4_DEC_BUFFER_CNT ; cnt++ ){
	if( pobj->psPlayerData[cnt]->state != 1 || dwCurrentFrame > pobj->psPlayerData[cnt]->frame ){
	  pobj->psPlayerData[cnt]->state = 0;
	  HVQM4DecodeBpic( &pobj->sSeqObj,pobj->pPicReadBuf,pobj->psPlayerData[cnt]->buf,pobj->psPlayerData[0]->buf,pobj->psPlayerData[1]->buf );
	  pobj->psPlayerData[cnt]->frame = dwFrame + pobj->ulGopTopFrame;
          if( pobj->cStatus == HVQM4PLAYER_STATUS_STOP ) return 0;
	  pobj->psPlayerData[cnt]->state = 1;
	  return 0;
	}
      }
      return 2;
    default : return 1;
  }
  return 0;
}

/* end */
