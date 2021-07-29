#ifndef _NGL_DMA_H
#define _NGL_DMA_H

// needed for u_int and u_long types
#ifndef _eetypes_h_
#include <eetypes.h>
#endif

#ifndef _LIBGRAPH_H
#include <libgraph.h>
#endif

// Memory access constants.
#define NGL_UNCACHED_ACCEL_MEM 0x30000000
#define NGL_UNCACHED_MEM       0x20000000
#define NGL_DMA_MEM            0x0FFFFFFF
#define NGL_SCRATCHPAD_MEM     0x70000000

// Maximum size of a DMA transfer in QWs (512k).
#define NGL_MAX_DMA_SIZE 32768

#define SCE_DMA_ID_REFE 0x0
#define SCE_DMA_ID_CNT  0x1
#define SCE_DMA_ID_NEXT 0x2
#define SCE_DMA_ID_REF  0x3
#define SCE_DMA_ID_REFS 0x4
#define SCE_DMA_ID_CALL 0x5
#define SCE_DMA_ID_RET  0x6
#define SCE_DMA_ID_END  0x7

// helpful macros
#define SCE_DMA_SET_TAG(qwc,pce,id,irq,addr,spr) \
							( ((u_long)(qwc)) \
							| (((u_long)(pce)) << 26) \
							| (((u_long)(id))  << 28) \
							| (((u_long)(irq)) << 31) \
							| (((u_long)(u_int)(addr)) << 32) \
							| (((u_long)(spr)) << 63) )

/*
#define SCE_DMA_SET_TAG( qwc, mark, id, next ) \
              ( ( ((u_long128)(qwc)) \
              | ( ((u_long128)(mark) << 16 ) \
              | ( ((u_long128)(id) << 24 ) \
              | ( ((u_long128)(next) << 32 ) \
*/              

// inlines to implement sync.l properly
static inline void sync_l()
{
	__asm__ ("
		.set noreorder
		sync.l
		nop
		.set reorder
	");
}

// inlines to implement sync.p properly
static inline void sync_p()
{
	__asm__ ("
		.set noreorder
		sync.p
		nop
		.set reorder
	");
}

// simple implementation of a dma library
void _nglDmaInit();
void _nglDmaWait();

void _nglDmaSetWaitGif();
void _nglDmaSetWaitVif1();

void _nglDmaSetWaitFromSPR();
void _nglDmaSetWaitToSPR();

// NOTE: the first sync.l makes sure we've written uncached accel to memory
// the second one makes sure the dma starts before we move on.

#define _nglDmaStartGifNormal(addr, qwc)						                \
  {                                                                 \
	  sync_l();														                            \
	  *D_STAT  = 1 << 2;                                              \
	  *D2_QWC  = qwc;                                                 \
	  *D2_MADR = (u_int)(addr) & NGL_DMA_MEM;                         \
	  *D2_TADR = 0x00;                                                \
	  *D2_CHCR = 1 | (0<<2) | (0<<4) | (0<<6) | (0<<7) | (1<<8);      \
	  sync_l();														                            \
  }

#define _nglDmaStartGifSourceChain(addr, tte)						            \
  {                                                                 \
	  sync_l();														                            \
	  *D_STAT  = 1 << 2;                                              \
	  *D2_QWC  = 0x00;                                                \
	  *D2_MADR = 0x00;                                                \
	  *D2_TADR = (u_int)(addr) & NGL_DMA_MEM;                         \
	  *D2_CHCR = 1 | (1<<2) | (0<<4) | (tte<<6) | (0<<7) | (1<<8);    \
	  sync_l();														                            \
  }

#define _nglDmaStartVif1Normal(addr, qwc)						                \
  {                                                                 \
	  sync_l();														                            \
	  *D_STAT  = 1 << 1;                                              \
	  *D1_QWC  = qwc;                                                 \
	  *D1_MADR = (u_int)(addr) & NGL_DMA_MEM;                         \
	  *D1_TADR = 0x00;                                                \
	  *D1_CHCR = 1 | (0<<2) | (0<<4) | (0<<6) | (0<<7) | (1<<8);      \
	  sync_l();														                            \
  }

#define _nglDmaStartVif1SourceChain(addr, tte)             			    \
  {                                                                 \
	  sync_l();														                            \
	  *D_STAT  = 1 << 1;                                              \
	  *D1_QWC  = 0x00;                                                \
	  *D1_MADR = 0x00;                                                \
	  *D1_TADR = (u_int)(addr) & NGL_DMA_MEM;                         \
	  *D1_CHCR = 1 | (1<<2) | (0<<4) | (tte<<6) | (0<<7) | (1<<8);    \
	  sync_l();														                            \
  }

#define _nglDmaStartFromSPRNormal(dst, src, qwc)             			  \
  {                                                                 \
	  sync_l();														                            \
	  *D_STAT  = 1 << 8;                                              \
	  *D8_QWC  = qwc;                                                 \
	  *D8_SADR = (u_int)(src) & 0x3fff;                               \
	  *D8_MADR = (u_int)(dst) & NGL_DMA_MEM;                          \
	  *D8_CHCR = 1 | (0<<2) | (0<<4) | (0<<6) | (0<<7) | (1<<8);      \
	  sync_l();														                            \
  }

#define _nglDmaStartToSPRNormal(dst, src, qwc)             			    \
  {                                                                 \
	  sync_l();														                            \
	  *D_STAT  = 1 << 9;                                              \
	  *D9_QWC  = qwc;                                                 \
	  *D9_SADR = (u_int)(dst) & 0x3fff;                               \
	  *D9_MADR = (u_int)(src) & NGL_DMA_MEM;                          \
	  *D9_CHCR = 1 | (0<<2) | (0<<4) | (1<<6) | (0<<7) | (1<<8);      \
	  sync_l();											 	                                \
  }                                   // ^ see s5.10 ee user manual

typedef struct 
{
	u_int Addr;
	void* Dest, *Src;

	// Use these members to perform processing once Continue returns, but be sure
	// to check that DataPtr != NULL first.
	void* Data;
	u_int Size;
} nglDmaSPADBuf;

typedef struct 
{
	nglDmaSPADBuf* Cur;
	nglDmaSPADBuf Buf[2];
} nglDmaSPADProcess;

// Call this function to initialize a scratchpad process.  Addr1 and Addr2 are offsets in
// SPAD RAM, 0..16383.  It's up to you to not overflow a buffer by passing in too large
// a chunk. 
inline void nglDmaInitSPAD( nglDmaSPADProcess* P, u_int Addr1, u_int Addr2 )
{
	memset( P, 0, sizeof(nglDmaSPADProcess) );
	P->Buf[0].Addr = Addr1;
	P->Buf[1].Addr = Addr2;
	P->Cur = P->Buf;
}

// Call this function repeatedly during processing.  Each time you call it, pass the next
// chunk of data to be transferred.  The value returned is the actual location of your data 
// (as a usable pointer) in the SPAD.
//
// Note that the returned value will not correspond to the data you passed in, it will match
// an earlier chunk that is now ready for processing.  Also the first time you call it the
// function will return NULL as no data is ready for processing yet.
//
// Once you have passed all your data to this function, continue calling it passing NULL for
// Dest, Src and 0 for size until the function returns NULL signalling that all your data
// has been processed.
//
// Also note that unless your data was already 16 byte aligned, the value returned may not be 
// the same address as the buffers you called Init with.
inline nglDmaSPADBuf* nglDmaContinueSPAD( nglDmaSPADProcess* P, void* Dest, void* Src, u_int Size )
{
	if ( P->Cur->Dest )
	{
		while ( *D1_CHCR & D_CHCR_STR_M );
		_nglDmaStartFromSPRNormal( (u_int)P->Cur->Dest & ~0xF, P->Cur->Addr, ( P->Cur->Size + 15 ) / 16 );
	}

	P->Cur->Src = Src;
	P->Cur->Dest = Dest;
	P->Cur->Size = Size;

	if ( Src )
	{
		while ( *D1_CHCR & D_CHCR_STR_M );
		_nglDmaStartToSPRNormal( P->Cur->Addr, (u_int)Src & ~0xF, ( Size + 15 ) / 16 );
	}

	P->Cur++;
	if ( P->Cur - P->Buf >= (int)(sizeof(P->Buf)/sizeof(P->Buf[0])) )
		P->Cur = P->Buf;

	if ( P->Cur->Src )
		P->Cur->Data = (void*)( NGL_SCRATCHPAD_MEM + P->Cur->Addr * 16 + ( (u_int)P->Cur->Src & 0xF ) );
	else
		P->Cur->Data = NULL;
	return P->Cur;
}

#endif // _NGL_DMA_H
