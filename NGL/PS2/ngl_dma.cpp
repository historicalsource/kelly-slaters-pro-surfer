
// sony ps includes
#include <libdma.h>
// other ngl library includes
#include "ngl_ps2.h"
// include for this file
#include "ngl_dma.h"

///////////////////////////////////////////////////////////////////////////////
//
//  private vars
//
///////////////////////////////////////////////////////////////////////////////

tD_CTRL*	_nglDmaCTRL = (tD_CTRL*)D_CTRL;
tD_STAT*	_nglDmaSTAT = (tD_STAT*)D_STAT;
tD_PCR*		_nglDmaPCR 	= (tD_PCR*)D_PCR;

tGIF_STAT* 	_nglGifSTAT = (tGIF_STAT*)GIF_STAT;

tVIF1_STAT* _nglVif1STAT = (tVIF1_STAT*)VIF1_STAT;
tVIF_CODE* 	_nglVif1CODE = (tVIF_CODE*)VIF1_CODE;

///////////////////////////////////////////////////////////////////////////////
//
//  public functions
//
///////////////////////////////////////////////////////////////////////////////

void _nglDmaInit()
{
	// nothing to do right now, but maybe in the future
  nglPrintf( "DMA Init: CTRL: %x STAT %x PCR %x\n", *D_CTRL, *D_STAT, *D_PCR );
}         

void _nglDmaSetWaitGif()
{
	_nglDmaPCR->CPC1 = 0;	// set for vif1
	_nglDmaPCR->CPC2 = 1;  // set for gif
	_nglDmaPCR->CPC8 = 0;  // set for fromspr
	_nglDmaPCR->CPC9 = 0;  // set for tospr
}

void _nglDmaSetWaitVif1()
{
	_nglDmaPCR->CPC1 = 1;	// set for vif1
	_nglDmaPCR->CPC2 = 0;  // set for gif
	_nglDmaPCR->CPC8 = 0;  // set for fromspr
	_nglDmaPCR->CPC9 = 0;  // set for tospr
}

void _nglDmaSetWaitFromSPR()
{
	_nglDmaPCR->CPC1 = 0;	// set for vif1
	_nglDmaPCR->CPC2 = 0;  // set for gif
	_nglDmaPCR->CPC8 = 1;  // set for fromspr
	_nglDmaPCR->CPC9 = 0;  // set for tospr
}

void _nglDmaSetWaitToSPR()
{
	_nglDmaPCR->CPC1 = 0;	// set for vif1
	_nglDmaPCR->CPC2 = 0;  // set for gif
	_nglDmaPCR->CPC8 = 0;  // set for fromspr
	_nglDmaPCR->CPC9 = 1;  // set for tospr
}

#if 0

// this dma wait is for debugging, if it prints count: 1 then you know that
// it didn't actually wait for the DMA to complete.  (or possibly there was
// no DMA in progress ...)
void _nglDmaWait()
{
	register int count = 0;

	__asm__ volatile ("
		.set noreorder
		sync.l
		sync.p
		j dma_wait_loop
		nop
		.align 8
	dma_wait_loop:
		bc0t dma_wait_done
		addiu %0, %0, 1
		bc0t dma_wait_done
		addiu %0, %0, 1
		bc0t dma_wait_done
		addiu %0, %0, 1
		bc0f dma_wait_loop
		addiu %0, %0, 1
	dma_wait_done:
		.set reorder
	" :             
	"=r" (count) );

	nglPrintf("count: %d\n", count);
}

#elif 1

void _nglDmaWait()
{
	__asm__ volatile ("
		.set noreorder
		sync.l
		sync.p
		j dma_wait_loop
		nop
		.align 8
	dma_wait_loop:
		bc0t dma_wait_done
		nop
		bc0t dma_wait_done
		nop
		bc0t dma_wait_done
		nop
		bc0f dma_wait_loop
		nop                                              
	dma_wait_done:
		.set reorder
	" );
}

#else

void _nglDmaWait()
{
  sceGsSyncPath( 0, 0 );
}

#endif
