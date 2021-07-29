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
 *                            util.c
 *                      utility functions
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 *       0.20           02.29.2000      umemura     openFile() deleted
 */
#include <stdio.h>
#include <string.h>
#include <sifdev.h>
#include "ps2_movie_defs.h"
#include "ps2_movie_vobuf.h"

// ///////////////////////////////////////////////////////////////
//
// Load modules
//
/*
void loadModule(char *moduleName)
{
    while (sceSifLoadModule(moduleName, 0, "") < 0) {
    	printf("Cannot load '%s'\n", moduleName);
    }
    printf("load '%s'\n", moduleName);
}

*/