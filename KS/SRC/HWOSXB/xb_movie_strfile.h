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
 *                           strfile.h
 *                 header file for file streaming
 *
 *       Version        Date            Design   Log
 *  --------------------------------------------------------------------
 *       0.10           02.29.2000      umemura  the first version
 */
#ifndef _STRFILE_H_
#define _STRFILE_H_

#include <eetypes.h>
#include <libcdvd.h>

// ////////////////////////////////////////////////////////////////
//
//  Structure to read data from CD/DVD or HD
//
typedef struct {
    int isOnCD;		// CD/DVD or HD
    int size;

    sceCdlFILE fp;	// for CD/DVD stream
    u_char *iopBuf;

    int fd;		// for HD stream
} StrFile;

int strFileOpen(StrFile *file, char *filename);
int strFileClose(StrFile *file);
int strFileRead(StrFile *file, void *buff, int size);

#endif // _STRFILE_H_
