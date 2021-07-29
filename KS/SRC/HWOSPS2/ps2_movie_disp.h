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
 *                            disp.h
 *                     header file for Display
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 *       0.20           01.19.2000      umemura     members are modified
 *       1.00           08.04.2000      ywashizu    using library
 */
#ifndef _DISP_H_
#define _DISP_H_

#include <libgraph.h>

// ////////////////////////////////////////////////////////////////
//
// Functions
//
void clearGsMem(int r, int g, int b, int disp_width, int disp_height);
void setImageTag(u_int *tags, void *image, int index,
		 int image_w, int image_h);
void startDisplay(int waitEven);
void endDisplay();

#endif _DISP_H_
