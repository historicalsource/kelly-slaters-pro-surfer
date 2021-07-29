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
 *                            defs.h
 *           global definitions for mpeg streaming program
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 *       1.00           08.04.2000      ywashizu    using library
 */
#ifndef _DEFS_H_
#define _DEFS_H_

#define UNCMASK 0x0fffffff
#define UNCBASE 0x20000000

#define TV_MODE SCE_GS_NTSC
#define DISP_WIDTH 640
#define DISP_HEIGHT 480
#define MAX_WIDTH 720
#define MAX_HEIGHT 576

#define N_LDTAGS (MAX_WIDTH/16 * MAX_HEIGHT/16 * 6 + 10)
#define TS_NONE (-1)

#define bound(val, x) ((((val) + (x) - 1) / (x))*(x))
#define bss_align(val) \
    __attribute__ ((aligned(val))) __attribute__ ((section (".bss")))

void ErrMessage(char *message);
void switchThread();
void proceedAudio();

#endif // _DEFS_H_
