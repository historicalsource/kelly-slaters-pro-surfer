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
 *                            vobuf.h
 *              header file for video output buffer
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 *       1.00           08.04.2000      ywashizu    using library
 */
#ifndef _VOBUF_H_
#define _VOBUF_H_

// ////////////////////////////////////////////////////////////////
//
//  Definitions
//
#define N_VOBUF 2

#define VOBUF_STATUS_    0
#define VOBUF_STATUS_TOPDONE  1
#define VOBUF_STATUS_FULL     2

#define MAX_MBX		(MAX_WIDTH/16)
#define MAX_MBY		(MAX_HEIGHT/16)

// ////////////////////////////////////////////////////////////////
//
//  Element data for video output buffer
//
typedef struct {
    u_char v[MAX_WIDTH * MAX_HEIGHT * 4];
} VoData;

// ////////////////////////////////////////////////////////////////
//
//  Tag for video output buffer
//
typedef struct {
    int status;		// status
    int dummy[15];	// this is to adjust D$ line
    u_int v[N_VOBUF][bound((N_LDTAGS+100)*4, 64)];
} VoTag;

// ////////////////////////////////////////////////////////////////
//
//  Video output buffer
//
typedef struct {
    VoData *data;	    // data array
    VoTag *tag;		    // tag array for path3 transfer
    volatile int write;	    // write position
    volatile int count;	    // the number of images in VoBuf
    int size;		    // total number of elements in VoBuf
} VoBuf;

// ////////////////////////////////////////////////////////////////
//
// Functions called from decoding thread
//
struct _Display;

void voBufCreate(
    VoBuf *f,
    VoData *data,
    VoTag *tag,
    int size
);
void voBufReset(VoBuf *f);
int voBufSetTags(VoBuf *f, struct _Display *d);
int voBufIsFull(VoBuf *f);
void voBufIncCount(VoBuf *f);
VoData *voBufGetData(VoBuf *f);
void voBufDelete(VoBuf *f);

// ////////////////////////////////////////////////////////////////
//
// Functions called from interrupt handler
//
int voBufIsEmpty(VoBuf *f);
VoTag *voBufGetTag(VoBuf *f);
void voBufDecCount(VoBuf *f);

extern VoBuf voBuf;

#endif // _VOBUF_H_
