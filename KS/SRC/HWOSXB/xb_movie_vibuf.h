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
 *                            vibuf.h
 *               header file for video input buffer
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 */
#ifndef _VIBUF_H_
#define _VIBUF_H_

#include <eekernel.h>
#include <libipu.h>

#define VIBUF_ELM_SIZE 2048
#define VIBUF_SIZE 256
#define VIBUF_TS_SIZE (VIBUF_SIZE*2)

// ////////////////////////////////////////////////////////////////
//
//  Time stamp
//
typedef struct {
    long pts;
    long dts;
    int pos;
    int len;
} TimeStamp;

// ////////////////////////////////////////////////////////////////
//
//  Video input buffer
//
typedef struct {
    u_long128 *data;	// data array
    u_long128 *tag;	// tag array
    int n;		// the number of data/tag element in ViBuf
    int dmaStart;	// DMA area start position
    int dmaN;		// DMA area size
    int readBytes;	// read area size
    int buffSize;	// buffer size of ViBuf(bytes)
    sceIpuDmaEnv env;	// DMA environment
    int sema;		// semaphore
    int isActive;	// flag to check CSC period
    long totalBytes;	// total bytes of data which sent to ViBuf

    // Time Stamp
    TimeStamp *ts;	// time stamp array
    int n_ts;		// time stamp array size
    int count_ts;	// the number of time stamps in the array
    int wt_ts;		// write position of time stamp array

} ViBuf;

// ////////////////////////////////////////////////////////////////
//
//  Functions
//
int viBufCreate(ViBuf *f,
    u_long128 *data, u_long128 *tag, int size,
    TimeStamp *ts, int n_ts);
int viBufReset(ViBuf *f);
int viBufAddDMA(ViBuf *f);
int viBufDelete(ViBuf *f);
int viBufStopDMA(ViBuf *f);
int viBufRestartDMA(ViBuf *f);
void viBufPrint(ViBuf *f);
int viBufIsActive(ViBuf *f);
void viBufBeginPut(ViBuf *f,
	u_char **ptr0, int *len0, u_char **ptr1, int *len1);
void viBufEndPut(ViBuf *f, int size);
int viBufPutTs(ViBuf *f, TimeStamp *ts);
int viBufGetTs(ViBuf *f, TimeStamp *ts);
int viBufCount(ViBuf *f);
void viBufFlush(ViBuf *f);

#endif // _VIBUF_H_

