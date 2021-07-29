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
 * File:	hvqm4player.h
 *
 * (C) 2000-2001 HUDSON SOFT
 *
 */

#include <dolphin.h>
#include "hvqm4dec.h"
#include "hvqm4audio.h"

#ifndef _HVQM4PLAYER_H_
#define _HVQM4PLAYER_H_

#define HVQM4_DEC_BUFFER_CNT	6
#define HVQM4_BPIC_BUFFER_START	2
#define DMA_BLOCK_SIZE		4096

typedef struct {
  u8  *buf;
  u32 frame;
  u8  state;
  u8  reserve[3];
} HVQM4PlayerData;

typedef struct {
  HVQM4Header	  sHeader ATTRIBUTE_ALIGN(32); 	       /* HVQM4MovieHeader */
  HVQM4GOPHeader  sGopHdr ATTRIBUTE_ALIGN(32);	       /* Current GOP header */
  HVQM4RecHeader  sRecHeader ATTRIBUTE_ALIGN(32);      /* Current record header */
  HVQM4SeqObj	  sSeqObj ATTRIBUTE_ALIGN(32);	       /* HVQM4 sequence buffer */
  ADPCMstate	  sAdpcmState[2] ATTRIBUTE_ALIGN(32);  /* Audio decord state */
  HVQM4PlayerData sPlayerData[HVQM4_DEC_BUFFER_CNT];   /* Decoding buffer */
  HVQM4PlayerData *psPlayerData[HVQM4_DEC_BUFFER_CNT]; /* Decoding buffer pointer */
  u8		  *pSoundBuf;	    /* Sound buffer pointer */
  u8		  *pSoundBufPtr;    /* Current sound buffer pointer */
  u8		  *pSoundMaxPtr;    /* Sound buffer EOF pointer */
  u8		  *pSoundDecPtr;    /* Sound buffer EOF pointer for decoding */
  u8		  *pPicReadBuf;	    /* Temporary buffer pointer for picture data */
  u8		  *pSoundReadBuf;   /* Temporary buffer pointer for sound data */

  u32		  ulGopCount;
  u32		  ulGopTopFrame;    /* First frame in GOP */
  u32		  ulCurrentFrame;   /* Current frame number */
  u32		  ulDecodeFrame;    /* The number of decoded frames */
  u32		  ulDecodeTotal;    /* The total time of decoding */
  u32		  ulDecodePicCnt;   /* The number of decoded images */
  u32		  ulDecodeAuCnt;    /* The number of decoded audio */
  OSTime	  ulStartTime;	    /* The timing of display */
  OSTime	  ulPauseTime;	    /* The timing of pause */
  u64		  ullAudioIndex;    /* Audio count */

  u16		  usTop;	    /* Screen start position ( top ) */
  u16		  usLeft;	    /* Screen start position ( left ) */
  u16		  usWidth;	    /* Image width */
  u16		  usHeight;	    /* Image height */

  void	    ( *funcPictureDecodeEnd )( u8 *pPicture,u32 ulFrame );  /* Functions to be called at the termination of decoding picture. */
  void	    ( *funcAudioDecodeEnd )( u8 *pAudio,u32 ulSize );  /* Functions to be called at the termination of decoding sound. */
  void	    ( *funcDrawPicture )( u32 ulCurrentFrame );  /* Functions to be called during drawing. */

  u8		    cStatus; 	    /* Playback status */
  u8		    cFlags; 	    /* Playback flags */
  u8		    bDecode;	    /* decode flags */
  u8		    cAudioSample;   /* Number of audio samples per second */
  u32		    reserved1; 	    /* Reserved */
} HVQM4PlayerObj;

/* cStatus */
#define HVQM4PLAYER_STATUS_ERRORFLAG  ( 1 << 31 )
#define HVQM4PLAYER_STATUS_NULL	      ( 1 << 30 )
#define HVQM4PLAYER_STATUS_READERROR  ( 1 << 29 )
#define HVQM4PLAYER_STATUS_NONE	      ( 0 )
#define HVQM4PLAYER_STATUS_PLAY	      ( 1 << 0 )
#define HVQM4PLAYER_STATUS_PAUSE      ( 1 << 1 )
#define HVQM4PLAYER_STATUS_STOP	      ( 1 << 2 )
#define HVQM4PLAYER_STATUS_START      ( 1 << 3 )
#define HVQM4PLAYER_STATUS_OPEN       ( 1 << 4 )

/* cFlags */
#define HVQM4PLAYER_PLAY_NONE	  ( 0 )
#define HVQM4PLAYER_PLAY_PICTURE  ( 1 << 0 )
#define HVQM4PLAYER_PLAY_AUDIO    ( 1 << 1 )
#define HVQM4PLAYER_PLAY_REPEAT   ( 1 << 2 )

void HVQM4PlayerCreate( void **ppFrameBuff,int iFrameCnt );
void HVQM4PlayerRelease( void );
u32 HVQM4PlayerOpen( char *lpszFilename, HVQM4PlayerObj *pobj );
u32 HVQM4PlayerClose( HVQM4PlayerObj *pobj );
u32 HVQM4PlayerPlay( HVQM4PlayerObj *pobj,u8 flags );
u32 HVQM4PlayerPause( HVQM4PlayerObj *pobj );
u32 HVQM4PlayerStop( HVQM4PlayerObj *pobj );
u32 HVQM4PlayerStatus( HVQM4PlayerObj *pobj );
u32 HVQM4PlayerGetBufferSize( HVQM4PlayerObj *pobj );
u32 HVQM4PlayerSetBuffer ( HVQM4PlayerObj *pobj,u8 *pbuffer );
u16 HVQM4PlayerGetWidth( HVQM4PlayerObj *pobj );
u16 HVQM4PlayerGetHeight( HVQM4PlayerObj *pobj );
void HVQM4PlayerSetDrawScreen( HVQM4PlayerObj *pobj,u16 left,u16 top,u16 width,u16 height );
void HVQM4PlayerAudioNextFrame( HVQM4PlayerObj *pobj,u16 *pNextSoundBuffer,u32 ulSoundSize,u8 volume );

void HVQM4PlayerSetDonePictureDecode( HVQM4PlayerObj *pobj,void funcDonePictureDecode( u8 *pPicture,u32 ulFrame ) );
void HVQM4PlayerSetDoneAudioDecode( HVQM4PlayerObj *pobj,void funcDoneAudioDecode( u8 *pAudio,u32 ulSize ) );
void HVQM4PlayerSetDispPicture( HVQM4PlayerObj *pobj,void funcDispPicture( u32 ulCurrentFrame ) );
void HVQM4PlayerSetDoneDraw( void funcDoneDraw( void *pFrameBuffer,int iDrawFlag ) );
void HVQM4PlayerSetBeforeDraw( void funcBeforeDraw( void **ppFrameBuffer ) );

void HVQM4PlayerDrawPicture( void );

/* read thread status */
typedef enum
{
  TstNORMAL=0,
  TstSTOPING,
  TstCLOSING,
  TstCLOSED
}TST;

#endif _HVQM4PLAYER_H_

