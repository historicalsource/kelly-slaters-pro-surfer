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
 *                            strfile.c
 *                   functions for file streaming
 *
 *       Version        Date            Design   Log
 *  --------------------------------------------------------------------
 *       0.10           02.29.2000      umemura  the first version
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <eekernel.h>
#include <sifdev.h>
#include "ps2_movie_defs.h"
#include "ps2_movie_strfile.h"

int isStrFileInit = 0;

// ////////////////////////////////////////////////////////////////
//
//  Open a file to read and check its size
//
//	< file name conversion >
//	   = from HD =
//	    dir/file.pss           -> host0:dir/file.pss
//	    host0:dir/file.pss     -> host0:dir/file.pss
//
//	   = from CD/DVD =
//	    cdrom0:\dir\file.pss;1 -> cdrom0:\DIR\FILE.PSS;1
//	    cdrom0:/dir/file.pss;1 -> cdrom0:\DIR\FILE.PSS;1
//	    cdrom0:/dir/file.pss   -> cdrom0:\DIR\FILE.PSS;1
//
int strFileOpen(StrFile *file, char *filename)
{
    int ret;
    char *body = NULL;
    char fn[256];
    char devname[64];

    body = index(filename, ':');

    if (body) {
    	int dlen;

	// copy device name
	dlen = body - filename;
	strncpy(devname, filename, dlen);
	devname[dlen] = 0;

	body += 1;

	if (!strcmp(devname, "cdrom0")) { // CD/DVD
	    int i;
	    int len = strlen(body);
	    const char *tail;

	    file->isOnCD = 1;

	    for (i = 0; i < len; i++) {
	        if (body[i] == '/') {
		    body[i] = '\\';
		}
		body[i] = toupper(body[i]);
	    }

	    tail = (index(filename, ';'))? "": ";1";
	    sprintf(fn, "%s%s", body, tail);

	} else {			// HD
	    file->isOnCD = 0;
	    sprintf(fn, "%s:%s", devname, body);
	}
    } else {				// HD (default)
    	body = filename;
	strcpy(devname, "host0");
	file->isOnCD = 0;
	sprintf(fn, "%s:%s", devname, body);
    }

    debug_print("file: %s\n", fn);

    if (file->isOnCD) {
	sceCdRMode mode;

    	if (!isStrFileInit) {
	    sceCdInit(SCECdINIT);
	    sceCdMmode(SCECdCD);
	    sceCdDiskReady(0);

	    isStrFileInit = 1;
	}

	file->iopBuf = (u_char *)sceSifAllocIopHeap((2048 * 80) + 16);
	sceCdStInit(80, 5, bound((u_int)file->iopBuf, 16));

	if(!sceCdSearchFile(&file->fp, fn)){

	    debug_print("Cannot open '%s'(sceCdSearchFile)\n", fn);
	    return 0;
	}

	file->size = file->fp.size;
	mode.trycount = 0;
	mode.spindlctrl = SCECdSpinStm;
	mode.datapattern = SCECdSecS2048;
	sceCdStStart(file->fp.lsn, &mode);

    } else {

	file->fd = sceOpen(fn, SCE_RDONLY);
	if (file->fd < 0) {
	    debug_print("Cannot open '%s'(sceOpen)\n", fn);
	    return 0;
	}
	file->size = sceLseek(file->fd, 0, SCE_SEEK_END);
	if (file->size < 0) {
	    debug_print("sceLseek() fails (%s): %d\n", fn, file->size);
	    sceClose(file->fd);
	    return 0;
	}

	ret = sceLseek(file->fd, 0, SCE_SEEK_SET);
	if (ret < 0) {
	    debug_print("sceLseek() fails (%s)\n", fn);
	    sceClose(file->fd);
	    return 0;
	}
    }

    return 1;
}

// ////////////////////////////////////////////////////////////////
//
//  Close a file
//
int strFileClose(StrFile *file)
{
    if (file->isOnCD) {
    	sceCdStStop();
        sceSifFreeIopHeap((void *)file->iopBuf);
    } else {
	sceClose(file->fd);
    }
    return 1;
}

// ////////////////////////////////////////////////////////////////
//
//  Read data
//
int strFileRead(StrFile *file, void *buff, int size)
{
    int count;
    if (file->isOnCD) {
	u_int err;

        count= sceCdStRead(size >> 11, (u_int *)buff, STMBLK, &err);
	count <<= 11;

    } else {
	count = sceRead(file->fd, buff, size);
    }
    return count;
}

