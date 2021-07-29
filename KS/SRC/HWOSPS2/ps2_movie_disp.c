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
 *                            disp.c
 *                    functions for Display
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 *       1.00           08.04.2000      ywashizu    using library 
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include "ps2_movie_defs.h"
#include "ps2_movie_disp.h"
#include "ps2_movie_videodec.h"
#include "ps2_movie_vobuf.h"

extern sceGsDBuff disp_db;

volatile int isCountVblank = 0;
volatile int vblankCount = 0;
volatile int isFrameEnd = 0;
volatile int oddeven = 0;
volatile int handler_error = 0;

extern int frd;

typedef struct {
  int x, y, w, h;
} Rect;

///////////////////////////////////////////////////////////////////
//
// Delete previous image of frame buffer and texture buffer
//
void clearGsMem(int r, int g, int b, int disp_width, int disp_height)
{
  const u_long giftag_clear[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), 
				 0x000000000000000eL };
  sceGifPacket packet;	
  u_long128* packetBase = (u_long128*)memalign(16, sizeof(u_long128)*6);

  sceDmaChan* dmaGif = sceDmaGetChan(SCE_DMA_GIF);

  // setting GIF tag for disp_db.draw0 structure
  SCE_GIF_CLEAR_TAG(&disp_db.giftag0);
  disp_db.giftag0.NLOOP = 8;
  disp_db.giftag0.EOP = 1;
  disp_db.giftag0.NREG = 1;
  disp_db.giftag0.REGS0 = 0xe; // A_D
  
  // define GS memory as a one big drawing area
  // "bound(DISP_HEIGHT/2, 32)*2" is for frame buffer * 2
  // "bound(DISP_HEIGHT, 32)*2" is for texture buffer
  sceGsSetDefDrawEnv(&disp_db.draw0, SCE_GS_PSMCT32, disp_width,
		     bound(disp_height/2, 32)*2 + bound(disp_height, 32)*2,
		     0, 0);
  *(u_long *)&disp_db.draw0.xyoffset1 = SCE_GS_SET_XYOFFSET_1(0, 0);

  FlushCache(0);
  sceGsSyncPath(0, 0);
  sceGsPutDrawEnv(&disp_db.giftag0);

  sceGifPkInit(&packet, (u_long128*)packetBase);
  sceGifPkReset(&packet);

  sceGifPkEnd(&packet, 0, 0, 0);
  
  // packet for a big sprite polygon
  {
    sceGifPkOpenGifTag(&packet, *(u_long128*)&giftag_clear);
    sceGifPkAddGsAD(&packet, SCE_GS_PRIM,
		    SCE_GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    sceGifPkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(r, g, b, 0, 0));
    sceGifPkAddGsAD(&packet, SCE_GS_XYZ2,
		    SCE_GS_SET_XYZ2(0 << 4,0 << 4, 0)); 
    sceGifPkAddGsAD(&packet, SCE_GS_XYZ2,
		    SCE_GS_SET_XYZ2( MAX_WIDTH << 4, MAX_HEIGHT*5 << 4, 0)); 
    sceGifPkCloseGifTag(&packet);
  }

  sceGifPkTerminate(&packet);

  FlushCache(0);
  sceGsSyncPath(0, 0);
  sceDmaSend(dmaGif, (u_long128*)((u_int)packet.pBase));
  sceGsSyncPath(0, 0);

  free(packetBase);
}

// ////////////////////////////////////////////////////////////////
//
// Set tags for image transfer using path3
//
void setImageTag(u_int *tags, void *image, int index, int image_w, int image_h)
{
  u_int dbp, dbw, dpsm;
  u_int dir, dsax, dsay;
  u_int rrw, rrh;
  u_int xdir;
  int mbx = image_w >> 4;
  int mby = image_h >> 4;
  int i, j;
  Rect tex;
  Rect poly;

  sceGifPacket packet;	
  const u_long giftag[2] = { SCE_GIF_SET_TAG(0, 0, 0, 0, 0, 1), 
			     0x000000000000000eL };
  const u_long giftag_eop[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), 
				 0x000000000000000eL };
  sceGifPkInit(&packet, (u_long128*)UncAddr(tags));
  sceGifPkReset(&packet);
  
  if (index == 0) {  // set image data to packet

    //  set params
    dbp = (bound(MAX_WIDTH, 64) * bound((MAX_HEIGHT/2), 32) * 2)/64;
    dbw = bound(MAX_WIDTH, 64)/64;
    dpsm = SCE_GS_PSMCT32;

    dir = 0;
    dsax = 0;
    dsay = 0;

    rrw = 16;
    rrh = 16;

    xdir = 0;

    sceGifPkCnt(&packet, 0, 0, 0);

    // eop == 0
    {    
      sceGifPkOpenGifTag(&packet, *(u_long128*)&giftag);
      sceGifPkAddGsAD(&packet, SCE_GS_BITBLTBUF,
		      SCE_GS_SET_BITBLTBUF(0, 0, 0, dbp, dbw, dpsm));
      sceGifPkAddGsAD(&packet, SCE_GS_TRXREG, SCE_GS_SET_TRXREG(rrw, rrh));
      sceGifPkCloseGifTag(&packet);
    }

    // //////////////////////////////////////
    // 
    //  create packet for image data
    // 
    for (i = 0; i < mbx; i++) {
      for (j = 0; j < mby; j++) {

	sceGifPkCnt(&packet, 0, 0, 0);

	// eop == 0
	{
	  sceGifPkOpenGifTag(&packet, *(u_long128*)giftag);
	  sceGifPkAddGsAD(&packet, SCE_GS_TRXPOS,
			  SCE_GS_SET_TRXPOS(0, 0, 16*i+dsax, 16*j+dsay, 0));
	  sceGifPkAddGsAD(&packet, SCE_GS_TRXDIR, SCE_GS_SET_TRXDIR(xdir));
	  sceGifPkCloseGifTag(&packet);
	}
	
	{
	  u_long* const tag = (u_long*)sceGifPkReserve(&packet, 4);
	  tag[0] = SCE_GIF_SET_TAG(16*16*4/16, 0, 0, 0, 2, 0);
		tag[1] = 0;
	}
	sceGifPkRef(&packet, (u_long128 *)DmaAddr(image), 16*16*4/16, 0, 0, 0);
	image = (u_char*)image + 16*16*4;
      }
    }
  }

  tex.x = 8; // 0.5
  tex.y = 8; // 0.5
  tex.w = image_w << 4;
  tex.h = image_h << 4;
  
  poly.x = (2048 - 320) << 4;
  poly.y = (2048 - (DISP_HEIGHT/2)/2) << 4;
  poly.w = DISP_WIDTH << 4;
  poly.h = (DISP_HEIGHT/2) << 4;
  
  // --------------------------------
  sceGifPkEnd(&packet, 0, 0, 0);

  // eop == 1
  {
    sceGifPkOpenGifTag(&packet, *(u_long128*)giftag_eop);
    sceGifPkAddGsAD(&packet, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&packet, SCE_GS_TEX1_1,
		    SCE_GS_SET_TEX1_1(0, 0, 1, 1, 0, 0, 0));
    sceGifPkAddGsAD(&packet, SCE_GS_TEX0_1,
		    SCE_GS_SET_TEX0_1((bound(MAX_WIDTH, 64) 
				       * bound((MAX_HEIGHT/2), 32) * 2)/64,
				      bound(MAX_WIDTH, 64)/64, SCE_GS_PSMCT32, 
				      10, 10, 0, 1, 0, 0, 0, 0, 0));
    sceGifPkAddGsAD(&packet, SCE_GS_PRIM,
		    SCE_GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    sceGifPkAddGsAD(&packet, SCE_GS_UV, SCE_GS_SET_UV(tex.x, tex.y)); 
    sceGifPkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(poly.x, poly.y, 0)); 
    sceGifPkAddGsAD(&packet, SCE_GS_UV,
		    SCE_GS_SET_UV(tex.x + tex.w, tex.y + tex.h)); 
    sceGifPkAddGsAD(&packet, SCE_GS_XYZ2, 
		    SCE_GS_SET_XYZ2(poly.x + poly.w, poly.y + poly.h, 0)); 
    sceGifPkCloseGifTag(&packet);
  }

  // finish making packet
  sceGifPkTerminate(&packet);
}

// /////////////////////////////////////////////////////////////////////
//
// vblank handler
//
int vblankHandler(int val)
{
  sceDmaChan* dmaGif_loadimage = sceDmaGetChan(SCE_DMA_GIF);
  oddeven = ((*GS_CSR) >> 13) & 1; // odd == 1, even == 0

  if (isCountVblank)
  {
    VoTag *tag;
    vblankCount++;

    handler_error = sceGsSyncPath(1, 0);
    if (!handler_error)
    {
      // no error
      tag = voBufGetTag(&voBuf);
      if (!tag)
      {
        frd++;
        ExitHandler();
        return 0;
      }

      sceGsSetHalfOffset((oddeven&1)?(sceGsDrawEnv1*)UncAddr(&disp_db.draw1)
        :(sceGsDrawEnv1*)UncAddr(&disp_db.draw0),
        2048, 2048, oddeven^0x1);

      if ((oddeven == 0) && (tag->status == VOBUF_STATUS_FULL))
      {
        sceGsSwapDBuff(&disp_db, 0);
        
        // Load image data to GS using path3
        sceGsSyncPath(0, 0);
        sceDmaSend(dmaGif_loadimage, (u_long128*)((u_int)tag->v[0]));
        
        tag->status = VOBUF_STATUS_TOPDONE;
        
      }
      else if ((oddeven == 1) && tag->status == VOBUF_STATUS_TOPDONE)
      {
        sceGsSwapDBuff(&disp_db, 1);
        
        // Load image data to GS using path3
        sceGsSyncPath(0, 0);
        sceDmaSend(dmaGif_loadimage, (u_long128*)((u_int)tag->v[1]));
        
        tag->status = VOBUF_STATUS_;
        
        isFrameEnd = 1;
      }
    }
  }

  ExitHandler();
  return 0;
}

// ///////////////////////////////////////////////////////////////
// 
//  Handler to check the end of image transfer
// 
int handler_endimage(int val)
{
  if (isFrameEnd) {
    voBufDecCount(&voBuf);
    isFrameEnd = 0;
  }
  ExitHandler();
  return 0;
}

// ///////////////////////////////////////////////////////////////////
// 
//  Wait until even/odd field
//  Start to count vblank
// 
void startDisplay(int waitEven)
{
  // wait untill even field
  while (sceGsSyncV(0) == waitEven)
    ;
  
  frd = 0;
  isCountVblank = 1;
  vblankCount = 0;
}

// ///////////////////////////////////////////////////////////////////
// 
//  Stop to count vblank
// 
void endDisplay()
{
  isCountVblank =  0;
  frd = 0;
}

