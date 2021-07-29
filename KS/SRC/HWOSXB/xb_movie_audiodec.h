/* SCEI CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 2.1
 */
/*
 *              Emotion Engine Library Sample Program
 *
 *                       - mpeg streaming -
 *
 *                         Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                            audiodec.h
 *                  header file for audio decoder
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 *       0.20           04.04.2000      kaol/ume     spu2 -> sd
 */
#ifndef _AUDIODEC_H_
#define _AUDIODEC_H_


// ///////////////////////////////////////////////////////
//
// Audio decoder state
//
#define AU_STATE_INIT		0
#define AU_STATE_PRESET		1
#define AU_STATE_PLAY		2
#define AU_STATE_PAUSE		3

#define AU_HDR_SIZE		(sizeof(SpuStreamHeader) 					+ sizeof(SpuStreamBody))

// ///////////////////////////////////////////////////////
//
// Spu stream header
//
typedef struct {
    char id[4];		// 'S''S''h''d'
    int size;		// 24
    int type;		// 0: 16bit big endian
    			// 1: 16bit little endian
			// 2: SPU2-ADPCM (VAG) 
    int rate;		// sampling rate
    int ch;		// number of channels
    int interSize;	// interleave size ... needs to be 512
    int loopStart;	// loop start block address
    int loopEnd;	// loop end block sddress
} SpuStreamHeader;

// ///////////////////////////////////////////////////////
//
// Spu stream body
//
typedef struct {
    char id[4];		// 'S''S''b''d'
    int size;		// size of audio data
} SpuStreamBody;

// ///////////////////////////////////////////////////////
//
// Audio decoder
//
typedef struct {
  int dummy;
} AudioDec;

// ///////////////////////////////////////////////////////
//
// Functions
//
typedef unsigned char u_char;

int audioDecCreate(
    AudioDec *ad,
    u_char *buff,
    int buffSize,
    int iopBuffSize
);

int audioDecDelete(AudioDec *ad);
void audioDecBeginPut(AudioDec *ad,
	u_char **ptr0, int *len0, u_char **ptr1, int *len1);
void audioDecEndPut(AudioDec *ad, int size);
int audioDecIsPreset(AudioDec *ad);
void audioDecStart(AudioDec *ad);
int audioDecSendToIOP(AudioDec *ad);
void audioDecReset(AudioDec *ad);
void audioDecPause(AudioDec *ad);
void audioDecResume(AudioDec *ad);

#endif _AUDIODEC_H_



