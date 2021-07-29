#include <libmpeg.h>
#include <assert.h>
#include <libsdr.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <ctype.h>
#include <eekernel.h>
#include <sifdev.h>
#include <sif.h>
#include <sdrcmd.h>
#include <libcdvd.h>
#include <stdarg.h>

#include "nvlMPEG_ps2.h"

#ifdef DEBUG
void nvlPrintf( const char* Format, ... )
{
	static char Work[512];
	va_list args;
	va_start( args, Format );
	vsprintf( Work, Format, args );
	va_end( args );
	
	scePrintf( Work );
}
#else
inline void nvlPrintf( const char* Format, ... ) {}
#endif

#define AUTODMA_CH          0
#define AU_HEADER_SIZE      40
#define UNIT_SIZE           1024
#define PRESET_VALUE(count) (count)

#define NVLMPEG_STACK_SIZE  2048
#define NVLMPEG_IOP_BUFF_SIZE       (512*48)
#define STACK_SIZE          (16*1024)
#define MAX_MBX             (MAX_WIDTH/16)
#define MAX_MBY             (MAX_HEIGHT/16)

#define BUFF_SIZE           (N_READ_UNIT * READ_UNIT_SIZE)

#define DMA_ID_REFE                 0
#define DMA_ID_NEXT                 2
#define DMA_ID_REF                  3
#define REST                        2
#define TAG_ADDR(i)                 ((u_int)DmaAddr(f->tag + i))
#define DATA_ADDR(i)                ((u_int)f->data+VIBUF_ELM_SIZE*(i))
#define WRAP_ADDR(addr)             ((u_int)(f->data)+(((u_int)(addr)-(u_int)(f->data))%(VIBUF_ELM_SIZE*f->n)))
#define IsInRegion(i,start,len,n)   ((0 <= (((i)+(n)-(start))%(n)))&&((((i)+(n)-(start))%(n))<(len)))
#define FS(f)                       ((f->dmaStart+f->dmaN)*VIBUF_ELM_SIZE)
#define FN(f)                       ((f->n-REST-f->dmaN)*VIBUF_ELM_SIZE)

#define UNCMASK         0x0fffffff
#define UNCBASE         0x20000000

#define MAX_WIDTH       720
#define MAX_HEIGHT      576
#define DISP_WIDTH      640
#define DISP_HEIGHT     480

#define N_LDTAGS        (MAX_WIDTH/16*MAX_HEIGHT/16*6+10)
#define TS_NONE         (-1)

#define bound(val,x)    ((((val) + (x) - 1) / (x))*(x))
#define min(x,y)        (((x) > (y))? (y): (x))
#define max(x,y)        (((x)<(y))?(y):(x))
#define bss_align(val)  __attribute__ ((aligned(val))) __attribute__ ((section (".bss")))

#define AU_STATE_INIT   0
#define AU_STATE_PRESET 1
#define AU_STATE_PLAY   2
#define AU_STATE_PAUSE  3

#define AU_HDR_SIZE     (sizeof(SpuStreamHeader)+sizeof(SpuStreamBody))

#define READ_UNIT_SIZE  (64*1024)
#define N_READ_UNIT     5

#define VD_STATE_NORMAL 0
#define VD_STATE_ABORT  1
#define VD_STATE_FLUSH  2
#define VD_STATE_END    3

#define VIBUF_ELM_SIZE  2048
#define VIBUF_SIZE      256
#define VIBUF_TS_SIZE   (VIBUF_SIZE*2)

#define N_VOBUF         2

#define VOBUF_STATUS_   0
#define VOBUF_STATUS_TOPDONE    1
#define VOBUF_STATUS_FULL       2

#define ZERO_BUFF_SIZE          0x800

#define MAX_MBX         (MAX_WIDTH/16)
#define MAX_MBY         (MAX_HEIGHT/16)

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} Rect;

// float to int conversion
inline int nvlFTOI( float input )  // doesn't round according to C standard
{
  register float output;
  __asm__ volatile ("cvt.w.s %0, %1" : "=f" (output) : "f" (input) );
  return (int&)output;
//  f += 3 << 22;
//  int n=((*(int*)&f)&0x007fffff) - 0x00400000;
//  return n;
}
typedef struct
{
    unsigned char   v[MAX_WIDTH*MAX_HEIGHT*4];
} VoData;

typedef struct
{
    int             status;
    int             dummy[15];
    unsigned int    v[N_VOBUF][bound((N_LDTAGS+100)*4, 64)];
} VoTag;

typedef struct
{
    VoData          *data;
    VoTag           *tag;       // tag array for path3 transfer
    volatile int    write;      // write position
    volatile int    count;      // the number of images in VoBuf
    int             size;       // total number of elements in VoBuf
} VoBuf;

typedef struct
{
    long            pts;
    long            dts;
    int             pos;
    int             len;
} TimeStamp;

typedef struct
{
    u_long128       *data;      // data array
    u_long128       *tag;       // tag array
    int             n;          // the number of data/tag element in ViBuf
    int             dmaStart;   // DMA area start position
    int             dmaN;       // DMA area size
    int             readBytes;  // read area size
    int             buffSize;   // buffer size of ViBuf(bytes)
    sceIpuDmaEnv    env;        // DMA environment
    int             sema;       // semaphore
    int             isActive;   // flag to check CSC period
    long            totalBytes; // total bytes of data which sent to ViBuf

    // Time Stamp
    TimeStamp       *ts;        // time stamp array
    int             n_ts;       // time stamp array size
    int             count_ts;   // the number of time stamps in the array
    int             wt_ts;      // write position of time stamp array
} ViBuf;

typedef struct
{
    sceMpeg         mpeg;
    ViBuf           vibuf;
    unsigned int    state;
    int             sema;
    int             hid_endimage;   // handler to check the end of image transfer
    int             hid_vblank;     // vlbank handler
} VideoDec;

typedef struct
{
    int             isOnCD;
    int             size;
    sceCdlFILE      fp;
    unsigned char   *iopBuf;
    int             fd;
} StrFile;

typedef struct
{
    unsigned char   data[N_READ_UNIT*READ_UNIT_SIZE];
    int             put;
    int             count;
    int             size;
} ReadBuf;

typedef struct
{
    char    id[4];      // 'S''S''h''d'
    int     size;       // 24
    int     type;       // 0: 16bit big endian
                        // 1: 16bit little endian
                        // 2: SPU2-ADPCM (VAG) 
    int     rate;       // sampling rate
    int     ch;         // number of channels
    int     interSize;  // interleave size ... needs to be 512
    int     loopStart;  // loop start block address
    int     loopEnd;    // loop end block sddress
} SpuStreamHeader;

typedef struct
{
    char    id[4];      // 'S''S''b''d'
    int     size;       // size of audio data
} SpuStreamBody;

typedef struct
{
    int             state;

    // header of ADS format
    SpuStreamHeader sshd;
    SpuStreamBody   ssbd;
    int hdrCount;

    // audio buffer
    unsigned char   *data;
    int             put;
    int             count;
    int             size;
    int             totalBytes;

    // buffer on IOP
    int             iopBuff;
    int             iopBuffSize;
    int             iopLastPos;
    int             iopPausePos;
    int             totalBytesSent;
    int             iopZero;
} AudioDec;

typedef union
{
    u_long128   q;
    u_long      l[2];
    u_int       i[4];
    u_short     s[8];
    u_char      c[16];
} QWORD;

static sceGsDBuff       *pGSDB;
static VoData           *voBufData;
static VoTag            *voBufTag;
static u_long128        *viBufTag;
static unsigned char    *mpegWork;
static char             *defStack;
static unsigned char    *audioBuff;
static u_long128        *viBufData;
static char             *videoDecStack;
static TimeStamp        *timeStamp;
static int              isStarted, readrest, writerest;
static void             *(*nvlMPEGAllocFunc)(int alighnment, int size);
static void             (*nvlMPEGFreeFunc)(void*);
static char             *_0_buf;
static ReadBuf          *readBuf;
static int              videoDecTh, defaultTh, frd, isStrFileInit = 0;
static StrFile          infile;
static VideoDec         videoDec;
static AudioDec         audioDec;
static VoBuf            voBuf;
static volatile int     isCountVblank = 0, vblankCount = 0, isFrameEnd = 0, oddeven = 0, handler_error = 0;

static int      vblankHandler           ( int );
static int      handler_endimage        ( int );
static int      decBs0                  ( VideoDec* vd);
static int      mpegError               ( sceMpeg* mp, sceMpegCbDataError* cberror, void* anyData);
static int      mpegNodata              ( sceMpeg* mp, sceMpegCbData* cbdata, void* anyData);
static int      mpegStopDMA             ( sceMpeg* mp, sceMpegCbData* cbdata, void* anyData);
static int      mpegRestartDMA          ( sceMpeg* mp, sceMpegCbData* cbdata, void* anyData);
static int      mpegTS                  ( sceMpeg* mp, sceMpegCbDataTimeStamp* cbts, void* anyData);
static void     iopGetArea              ( int* pd0, int* d0, int* pd1, int* d1, AudioDec* ad, int pos );
static int      sendToIOP2area          ( int pd0, int d0, int pd1, int d1, u_char* ps0, int s0, u_char* ps1, int s1 );
static int      sendToIOP               ( int dst, u_char* src, int size );
static void     changeMasterVolume      ( u_int val );
static void     changeInputVolume       ( u_int val );
static int      audioDecCreate          ( AudioDec* ad, unsigned char* buff, int buffSize, int iopBuffSize );
static int      audioDecDelete          ( AudioDec* ad );
static void     audioDecBeginPut        ( AudioDec* ad, unsigned char** ptr0, int* len0, unsigned char** ptr1, int* len1 );
static void     audioDecEndPut          ( AudioDec* ad, int size );
static int      audioDecIsPreset        ( AudioDec* ad );
static void     audioDecStart           ( AudioDec* ad );
static int      audioDecSendToIOP       ( AudioDec* ad );
static void     audioDecReset           ( AudioDec* ad );
static void     audioDecPause           ( AudioDec* ad );
static void     audioDecResume          ( AudioDec* ad );
static void     setImageTag             ( unsigned int* tags, void* image, int index, int image_w, int image_h );
static void     startDisplay            ( int waitEven );
static void     endDisplay              ();
static void     readBufCreate           ( ReadBuf* buff );
static void     readBufDelete           ( ReadBuf* buff );
static int      readBufBeginPut         ( ReadBuf* buff, unsigned char** ptr );
static int      readBufEndPut           ( ReadBuf* buff, int size );
static int      readBufBeginGet         ( ReadBuf* buff, unsigned char** ptr );
static int      readBufEndGet           ( ReadBuf* buff, int size );
static int      strFileOpen             ( StrFile* file, char *filename );
static int      strFileClose            ( StrFile* file );
static int      strFileRead             ( StrFile* file, void* buff, int size );
static void     videoDecReset           ( VideoDec* vd );
static int      videoDecCreate          ( VideoDec* vd, unsigned char* mpegWork, int mpegWorkSize, u_long128* data, u_long128* tag, int tagSize, TimeStamp* pts, int n_pts );
static int      videoDecDelete          ( VideoDec* vd );
//static void     videoDecAbort           ( VideoDec* vd );
static u_int    videoDecGetState        ( VideoDec* vd );
static u_int    videoDecSetState        ( VideoDec* vd, u_int state );
static int      videoDecInputCount      ( VideoDec* vd );
//static int      videoDecInputSpaceCount ( VideoDec* vd );
//static void     videoDecSetDecodeMode   ( VideoDec* vd, int ni, int np, int nb );
static int      videoDecFlush           ( VideoDec* vd );
static int      videoDecIsFlushed       ( VideoDec* vd );
static int      videoDecSetStream       ( VideoDec* vd, int strType, int ch,	sceMpegCallback h, void *data );
static void     videoDecBeginPut        ( VideoDec* vd,	unsigned char** ptr0, int* len0, unsigned char** ptr1, int* len1 );
static void     videoDecEndPut          ( VideoDec* vd, int size) ;
static int      videoDecPutTs           ( VideoDec* vd, long pts_val, long dts_val, unsigned char* start, int len );
static void     videoDecMain            ( VideoDec* vd );
static int      viBufCreate             ( ViBuf* f, u_long128* data, u_long128* tag, int size, TimeStamp* ts, int n_ts );
static int      viBufReset              ( ViBuf* f );
static int      viBufAddDMA             ( ViBuf* f );
static int      viBufDelete             ( ViBuf* f );
static int      viBufStopDMA            ( ViBuf* f );
static int      viBufRestartDMA         ( ViBuf* f );
//static int      viBufIsActive           ( ViBuf* f );
static void     viBufBeginPut           ( ViBuf* f,	unsigned char** ptr0, int* len0, unsigned char** ptr1, int* len1 );
static void     viBufEndPut             ( ViBuf* f, int size );
static int      viBufPutTs              ( ViBuf* f, TimeStamp* ts );
static int      viBufGetTs              ( ViBuf* f, TimeStamp* ts );
static int      viBufCount              ( ViBuf* f );
static void     viBufFlush              ( ViBuf* f );
static void     voBufCreate             ( VoBuf* f, VoData* data, VoTag* tag, int size );
static void     voBufReset              ( VoBuf* f );
static int      voBufIsFull             ( VoBuf* f );
static void     voBufIncCount           ( VoBuf* f );
static VoData*  voBufGetData            ( VoBuf* f );
static void     voBufDelete             ( VoBuf* f );
static int      voBufIsEmpty            ( VoBuf* f );
static VoTag    *voBufGetTag            ( VoBuf* f );
static void     voBufDecCount           ( VoBuf* f );

static inline void *UncAddr( void *val )
{
    return (void*)(((unsigned)val & UNCMASK) | UNCBASE );
}

static int nvlMem2Cpy( unsigned char* pd0, int d0, unsigned char* pd1, int d1, unsigned char* ps0, int s0, unsigned char* ps1, int s1 )
{
    if( d0 + d1 < s0 + s1 )
    {
        return 0;
    }
    if( s0 >= d0 )
    {
        memcpy( pd0, ps0, d0 );
        memcpy( pd1, ps0 + d0, s0 - d0 );
        memcpy( pd1 + s0 - d0, ps1, s1 );
    }
    else
    {
        if( s1 >= d0 - s0 )
        {
            memcpy( pd0, ps0, s0 );
            memcpy( pd0 + s0, ps1, d0 - s0 );
            memcpy( pd1, ps1 + d0 - s0, s1 - d0 + s0 );
        }
        else
        {
            memcpy( pd0, ps0, s0 );
            memcpy( pd0 + s0, ps1, s1 );
        }
    }
    return s0 + s1;
}

static int videoCallback( sceMpeg* mp, sceMpegCbDataStr* str, void* data )
{
    ReadBuf         *rb = (ReadBuf*)data;
    unsigned char   *ps0 = str->data, *ps1 = rb->data, *pd0, *pd1;
    int             s0 = min( rb->data + rb->size - str->data, (int)str->len ), s1 = str->len - s0, d0, d1, len;

    videoDecBeginPut( &videoDec, &pd0, &d0, &pd1, &d1 );
    len = nvlMem2Cpy( (unsigned char*)UncAddr(pd0), d0, (unsigned char*)UncAddr(pd1), d1, ps0, s0, ps1, s1 );
    if( len > 0 )
    {
        int res = videoDecPutTs( &videoDec, str->pts, str->dts, pd0, len );
        assert( res );
    }
    videoDecEndPut( &videoDec, len );
    return len > 0;
}

static int pcmCallback( sceMpeg* mp, sceMpegCbDataStr* str, void* data )
{
    ReadBuf         *rb = (ReadBuf*)data;
    unsigned char   *ps0 = str->data + 4, *pd0, *pd1;
    int             s0, s1, d0, d1, len, ret;

    if( ps0 >= rb->data + rb->size )
    {
        ps0 -= rb->size;
    }
    len = str->len - 4;
    s0 = min( rb->data + rb->size - ps0, len );
    s1 = len - s0;
    audioDecBeginPut( &audioDec, &pd0, &d0, &pd1, &d1 );
    ret = nvlMem2Cpy( pd0, d0, pd1, d1, ps0, s0, rb->data, s1 );
    audioDecEndPut( &audioDec, ret );
    return ret > 0;
}

static void defMain(void *unused)
{
    while( 1 )
    {
        RotateThreadReadyQueue( NVLMPEG_PRIORITY );
    }
}

bool nvlMPEGLoad( char *bsfilename, void* pGSDBuf )
{
    struct ThreadParam  th_param;

    assert( bsfilename && pGSDBuf );
    pGSDB = (sceGsDBuff*)pGSDBuf;

    if(false == strFileOpen( &infile, bsfilename ))
        return false;

    // allocate memory for all needs
    if( nvlMPEGAllocFunc )
    {
        voBufData = (VoData*)(*nvlMPEGAllocFunc)( 64, N_VOBUF * sizeof(VoData) );
        assert( voBufData );
        voBufTag = (VoTag*)(*nvlMPEGAllocFunc)( 64, N_VOBUF * sizeof(VoTag) );
        assert( voBufTag );
        viBufTag = (u_long128*)(*nvlMPEGAllocFunc)( 64, (VIBUF_SIZE + 1) * sizeof(u_long128) );
        assert( viBufTag );
        mpegWork = (unsigned char*)(*nvlMPEGAllocFunc)( 64, SCE_MPEG_BUFFER_SIZE(MAX_WIDTH, MAX_HEIGHT) );
        assert( mpegWork );
        defStack = (char*)(*nvlMPEGAllocFunc)( 64, NVLMPEG_STACK_SIZE );
        assert( defStack );
        audioBuff = (unsigned char*)(*nvlMPEGAllocFunc)( 64, NVLMPEG_IOP_BUFF_SIZE * 2 );
        assert( audioBuff );
        viBufData = (u_long128*)(*nvlMPEGAllocFunc)( 64, VIBUF_SIZE * VIBUF_ELM_SIZE / 16 * sizeof(u_long128) );
        assert( viBufData );
        videoDecStack = (char*)(*nvlMPEGAllocFunc)( 64, STACK_SIZE );
        assert( videoDecStack );
        timeStamp = (TimeStamp*)(*nvlMPEGAllocFunc)( 1, VIBUF_TS_SIZE * sizeof(TimeStamp) );
        assert( timeStamp );
        _0_buf = (char*)(*nvlMPEGAllocFunc)( 64, ZERO_BUFF_SIZE );
        assert( _0_buf );
        readBuf = (ReadBuf*)(*nvlMPEGAllocFunc)( 64, sizeof(ReadBuf) );
        assert( readBuf );
    }
    else
    {
        voBufData = (VoData*)memalign( 64, N_VOBUF * sizeof(VoData) );
        assert( voBufData );
        voBufTag = (VoTag*)memalign( 64, N_VOBUF * sizeof(VoTag) );
        assert( voBufTag );
        viBufTag = (u_long128*)memalign( 64, (VIBUF_SIZE + 1) * sizeof(u_long128) );
        assert( viBufTag );
        mpegWork = (unsigned char*)memalign( 64, SCE_MPEG_BUFFER_SIZE(MAX_WIDTH, MAX_HEIGHT) );
        assert( mpegWork );
        defStack = (char*)memalign( 64, NVLMPEG_STACK_SIZE );
        assert( defStack );
        audioBuff = (unsigned char*)memalign( 64, NVLMPEG_IOP_BUFF_SIZE * 2 );
        assert( audioBuff );
        viBufData = (u_long128*)memalign( 64, VIBUF_SIZE * VIBUF_ELM_SIZE / 16 * sizeof(u_long128) );
        assert( viBufData );
        videoDecStack = (char*)memalign( 64, STACK_SIZE );
        assert( videoDecStack );
        timeStamp = (TimeStamp*)malloc( VIBUF_TS_SIZE * sizeof(TimeStamp) );
        assert( timeStamp );
        _0_buf = (char*)memalign( 64, ZERO_BUFF_SIZE );
        assert( _0_buf );
        readBuf = (ReadBuf*)memalign( 64, sizeof(ReadBuf) );
        assert( readBuf );
    }

    *D_CTRL = (*D_CTRL | 0x003);
    *D_STAT = 0x4; // clear D_STAT.CIS2

    readBufCreate( readBuf );
    sceMpegInit();
    videoDecCreate( &videoDec, mpegWork, SCE_MPEG_BUFFER_SIZE(MAX_WIDTH, MAX_HEIGHT), viBufData, viBufTag, VIBUF_SIZE, timeStamp, VIBUF_TS_SIZE );
    sceSdRemoteInit();
    sceSdRemote( 1, rSdInit, SD_INIT_COLD );
    audioDecCreate( &audioDec, audioBuff, NVLMPEG_IOP_BUFF_SIZE * 2, NVLMPEG_IOP_BUFF_SIZE );
    videoDecSetStream( &videoDec, sceMpegStrM2V, 0, (sceMpegCallback)videoCallback, readBuf );
    videoDecSetStream( &videoDec, sceMpegStrPCM, 0, (sceMpegCallback)pcmCallback, readBuf );
    voBufCreate( &voBuf, (VoData*)UncAddr(voBufData), voBufTag, N_VOBUF );

    th_param.entry = defMain;
    th_param.stack = defStack;
    th_param.stackSize = NVLMPEG_STACK_SIZE;
    th_param.initPriority = NVLMPEG_PRIORITY;
    th_param.gpReg = &_gp;
    th_param.option = 0;
    defaultTh = CreateThread( &th_param );
    StartThread( defaultTh, NULL );

    th_param.entry = videoDecMain;
    th_param.stack = videoDecStack;
    th_param.stackSize = STACK_SIZE;
    th_param.initPriority = NVLMPEG_PRIORITY;
    th_param.gpReg = &_gp;
    th_param.option = 0;
    videoDecTh = CreateThread( &th_param );
    StartThread( videoDecTh, &videoDec );

    videoDec.hid_vblank = AddIntcHandler( INTC_VBLANK_S, vblankHandler, 0 );
    EnableIntc(INTC_VBLANK_S);

    videoDec.hid_endimage = AddDmacHandler( DMAC_GIF, handler_endimage, 0 );
    EnableDmac( DMAC_GIF );

    isStarted = 0;
    readrest = infile.size;
    writerest = infile.size;
}

void nvlMPEGStop()
{
    /*while( !videoDecFlush(&videoDec) )
    {
        RotateThreadReadyQueue( NVLMPEG_PRIORITY );
    }

    while( !videoDecIsFlushed(&videoDec) && videoDecGetState(&videoDec) != VD_STATE_END )
    {
        RotateThreadReadyQueue( NVLMPEG_PRIORITY );
    }*/

    endDisplay();
    audioDecReset( &audioDec );

    readBufDelete( readBuf );
    voBufDelete(&voBuf);

    TerminateThread(videoDecTh);
    DeleteThread(videoDecTh);

    TerminateThread(defaultTh);
    DeleteThread(defaultTh);

    DisableDmac(DMAC_GIF);
    RemoveDmacHandler(DMAC_GIF, videoDec.hid_endimage);

    DisableIntc(INTC_VBLANK_S);
    RemoveIntcHandler(INTC_VBLANK_S, videoDec.hid_vblank);

    videoDecDelete(&videoDec);
    audioDecDelete(&audioDec);

    strFileClose(&infile);

    if( nvlMPEGFreeFunc )
    {
        (*nvlMPEGFreeFunc)( voBufData );
        (*nvlMPEGFreeFunc)( voBufTag );
        (*nvlMPEGFreeFunc)( viBufTag );
        (*nvlMPEGFreeFunc)( mpegWork );
        (*nvlMPEGFreeFunc)( defStack );
        (*nvlMPEGFreeFunc)( audioBuff );
        (*nvlMPEGFreeFunc)( viBufData );
        (*nvlMPEGFreeFunc)( videoDecStack );
        (*nvlMPEGFreeFunc)( timeStamp );
        (*nvlMPEGFreeFunc)( _0_buf );
        (*nvlMPEGFreeFunc)( readBuf );
    }
    else
    {
        free( voBufData );
        free( voBufTag );
        free( viBufTag );
        free( mpegWork );
        free( defStack );
        free( audioBuff );
        free( viBufData );
        free( videoDecStack );
        free( timeStamp );
        free( _0_buf );
        free( readBuf );
    }
}

int nvlMPEGAdvance()
{
    unsigned char   *put_ptr, *get_ptr;
    int putsize, getsize, count, proceed;

    // writerest > 4: to skip the last 4 bytes
    if( writerest > 4 && videoDecGetState(&videoDec) != VD_STATE_END )
    {
        putsize = readBufBeginPut( readBuf, &put_ptr );
        if( readrest > 0 && putsize >= READ_UNIT_SIZE )
        {
            count = strFileRead( &infile, put_ptr, READ_UNIT_SIZE );
            readBufEndPut( readBuf, count );
            readrest -= count;
        }

        RotateThreadReadyQueue( NVLMPEG_PRIORITY );

        getsize = readBufBeginGet( readBuf, &get_ptr );
        if( getsize > 0 )
        {
            proceed = sceMpegDemuxPssRing( &videoDec.mpeg, get_ptr, getsize, readBuf->data, readBuf->size );
            readBufEndGet( readBuf, proceed );
            writerest -= proceed;
        }

        audioDecSendToIOP( &audioDec );

        if( !isStarted && voBufIsFull(&voBuf) && audioDecIsPreset(&audioDec) )
        {
            startDisplay( 1 );
            audioDecStart( &audioDec );
            isStarted = 1;
        }
        return true;
    }
    return false;
}

void nvlMPEGSetMemoryAllocCallback( void* (*func)(int alighnment, int size) )
{
    nvlMPEGAllocFunc = func;
}

void nvlMPEGSetMemoryFreeCallback( void (*func)(void*) )
{
    nvlMPEGFreeFunc = func;
}

static inline void *DmaAddr( void *val )
{
    return (void*)( (unsigned)val & UNCMASK );
}

static void setImageTag( u_int *tags, void *image, int index, int image_w, int image_h )
{
    const u_long    giftag[2] = { SCE_GIF_SET_TAG(0, 0, 0, 0, 0, 1), 0x000000000000000eL },
                    giftag_eop[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), 0x000000000000000eL };
    u_int           dbp, dbw, dpsm, dir, dsax, dsay, rrw, rrh, xdir;
    int             mbx = image_w >> 4, mby = image_h >> 4;
    Rect            tex, poly;
    sceGifPacket    packet;	

    sceGifPkInit( &packet, (u_long128*)UncAddr(tags) );
    sceGifPkReset( &packet );
    if( index == 0 )
    {
        dbp = ( bound(MAX_WIDTH, 64) * bound((MAX_HEIGHT / 2), 32) * 2 ) / 64;
        dbw = bound( MAX_WIDTH, 64 ) / 64;
        dpsm = SCE_GS_PSMCT32;
        dir = 0;
        dsax = 0;
        dsay = 0;
        rrw = 16;
        rrh = 16;
        xdir = 0;
        sceGifPkCnt(&packet, 0, 0, 0);
        sceGifPkOpenGifTag( &packet, *(u_long128*)&giftag );
        sceGifPkAddGsAD( &packet, SCE_GS_BITBLTBUF, SCE_GS_SET_BITBLTBUF(0, 0, 0, dbp, dbw, dpsm) );
        sceGifPkAddGsAD( &packet, SCE_GS_TRXREG, SCE_GS_SET_TRXREG(rrw, rrh));
        sceGifPkCloseGifTag( &packet );
        for( int i = 0; i < mbx; i++ )
        {
            for( int j = 0; j < mby; j++ )
            {
                sceGifPkCnt( &packet, 0, 0, 0 );
                sceGifPkOpenGifTag( &packet, *(u_long128*)giftag );
                sceGifPkAddGsAD( &packet, SCE_GS_TRXPOS, SCE_GS_SET_TRXPOS(0, 0, 16*i+dsax, 16*j+dsay, 0) );
                sceGifPkAddGsAD( &packet, SCE_GS_TRXDIR, SCE_GS_SET_TRXDIR(xdir) );
                sceGifPkCloseGifTag( &packet );
                u_long* const   tag = (u_long*)sceGifPkReserve( &packet, 4 );
                tag[0] = SCE_GIF_SET_TAG( 16 * 16 * 4 / 16, 0, 0, 0, 2, 0 );
                tag[1] = 0;
                sceGifPkRef( &packet, (u_long128*)DmaAddr(image), 16 * 16 * 4 / 16, 0, 0, 0 );
                image = (u_char*)image + 16 * 16 * 4;
            }
        }
    }

    tex.x = 8;
    tex.y = 8;
    tex.w = image_w << 4;
    tex.h = image_h << 4;

/*
    poly.x = ( 2048 - (DISP_WIDTH / 2) ) << 4;
    poly.y = ( 2048 - (DISP_HEIGHT / 2) / 2 ) << 4;
    poly.w = DISP_WIDTH << 4;
    poly.h = ( DISP_HEIGHT / 2 ) << 4;
*/
    poly.x = ( 2048 - (image_w >> 1) ) << 4;
    poly.y = ( 2048 - (image_h >> 2) ) << 4;
    poly.w = image_w << 4;
    poly.h = image_h << 3;
  
    sceGifPkEnd(&packet, 0, 0, 0);

    sceGifPkOpenGifTag( &packet, *(u_long128*)giftag_eop );
    sceGifPkAddGsAD( &packet, SCE_GS_TEXFLUSH, 0 );
    sceGifPkAddGsAD( &packet, SCE_GS_TEX1_1, SCE_GS_SET_TEX1_1(0, 0, 1, 1, 0, 0, 0) );
    sceGifPkAddGsAD( &packet, SCE_GS_TEX0_1, SCE_GS_SET_TEX0_1((bound(MAX_WIDTH, 64) * bound((MAX_HEIGHT/2), 32) * 2)/64, bound(MAX_WIDTH, 64)/64, SCE_GS_PSMCT32, 10, 10, 0, 1, 0, 0, 0, 0, 0) );
    sceGifPkAddGsAD( &packet, SCE_GS_PRIM, SCE_GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0) );
    sceGifPkAddGsAD( &packet, SCE_GS_UV, SCE_GS_SET_UV(tex.x, tex.y) ); 
    sceGifPkAddGsAD( &packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(poly.x, poly.y, 0) ); 
    sceGifPkAddGsAD( &packet, SCE_GS_UV, SCE_GS_SET_UV(tex.x + tex.w, tex.y + tex.h) ); 
    sceGifPkAddGsAD( &packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(poly.x + poly.w, poly.y + poly.h, 0) );
    sceGifPkCloseGifTag( &packet );
    sceGifPkTerminate( &packet );
}

static int vblankHandler( int val )
{
    sceDmaChan  *dmaGif_loadimage = sceDmaGetChan( SCE_DMA_GIF );

    oddeven = ( *GS_CSR >> 13 ) & 1;
    if( isCountVblank )
    {
        VoTag   *tag;

        vblankCount++;
        handler_error = sceGsSyncPath( 1, 0 );
        if( !handler_error )
        {
            tag = voBufGetTag( &voBuf );
            if( !tag )
            {
                frd++;
                ExitHandler();
                return 0;
            }
            sceGsSetHalfOffset( (oddeven&1)?(sceGsDrawEnv1*)UncAddr(&pGSDB->draw1) : (sceGsDrawEnv1*)UncAddr(&pGSDB->draw0), 2048, 2048, oddeven ^ 0x1 );
            if( (oddeven == 0) && (tag->status == VOBUF_STATUS_FULL) )
            {
                sceGsSwapDBuff( pGSDB, 0 );
                sceGsSyncPath( 0, 0 );
                sceDmaSend( dmaGif_loadimage, (u_long128*)((u_int)tag->v[0]) );
                tag->status = VOBUF_STATUS_TOPDONE;
            }
            else if( oddeven == 1 && tag->status == VOBUF_STATUS_TOPDONE )
            {
                sceGsSwapDBuff( pGSDB, 1 );
                sceGsSyncPath( 0, 0 );
                sceDmaSend( dmaGif_loadimage, (u_long128*)((u_int)tag->v[1]) );
                tag->status = VOBUF_STATUS_;
                isFrameEnd = 1;
            }
        }
    }
    ExitHandler();
    return 0;
}

static int handler_endimage( int val )
{
    if( isFrameEnd )
    {
        voBufDecCount( &voBuf );
        isFrameEnd = 0;
    }
    ExitHandler();
    return 0;
}

static void startDisplay( int waitEven )
{
    while( sceGsSyncV(0) == waitEven )
        ;
    frd = 0;
    isCountVblank = 1;
    vblankCount = 0;
}

static void endDisplay()
{
    isCountVblank =  0;
    frd = 0;
}

static void readBufCreate( ReadBuf* b )
{
    b->put = b->count = 0;
    b->size = BUFF_SIZE;
}

static void readBufDelete( ReadBuf* b )
{
}

static int readBufBeginPut( ReadBuf *b, unsigned char** ptr )
{
    int size = b->size - b->count;
    if( size )
    {
        *ptr = b->data + b->put;
    }
    return size;
}

static int readBufEndPut( ReadBuf* b, int size )
{
    int size_ok = min( b->size - b->count, size );

    b->put = ( b->put + size_ok ) % b->size;
    b->count += size_ok;
    return size_ok;
}

static int readBufBeginGet( ReadBuf* b, unsigned char** ptr )
{
    if( b->count )
    {
        *ptr = b->data + (b->put - b->count + b->size) % b->size;
    }
    return b->count;
}

static int readBufEndGet( ReadBuf *b, int size )
{
    int size_ok = min( b->count, size );

    b->count -= size_ok;
    return size_ok;
}

static int videoDecCreate( VideoDec* vd, u_char* mpegWork, int mpegWorkSize, u_long128* data, u_long128* tag, int tagSize, TimeStamp* pts, int n_pts )
{
    sceMpegCreate( &vd->mpeg, mpegWork, mpegWorkSize );
    sceMpegAddCallback( &vd->mpeg, sceMpegCbError, (sceMpegCallback)mpegError, NULL );
    sceMpegAddCallback( &vd->mpeg, sceMpegCbNodata, mpegNodata, NULL );
    sceMpegAddCallback( &vd->mpeg, sceMpegCbStopDMA, mpegStopDMA, NULL );
    sceMpegAddCallback( &vd->mpeg, sceMpegCbRestartDMA, mpegRestartDMA, NULL );
    sceMpegAddCallback( &vd->mpeg, sceMpegCbTimeStamp, (sceMpegCallback)mpegTS, NULL );
    videoDecReset(vd);
    viBufCreate(&vd->vibuf, data, tag, tagSize, pts, n_pts);
    return 1;
}

/*
static void videoDecSetDecodeMode( VideoDec* vd, int ni, int np, int nb )
{
    sceMpegSetDecodeMode( &vd->mpeg, ni, np, nb );
}
*/

static int videoDecSetStream( VideoDec* vd, int strType, int ch, sceMpegCallback cb, void* data )
{
    sceMpegAddStrCallback( &vd->mpeg, (sceMpegStrType)strType, ch, cb, data );
    return 1;
}

static void videoDecBeginPut( VideoDec* vd, u_char** ptr0, int* len0, u_char** ptr1, int* len1 )
{
    viBufBeginPut( &vd->vibuf, ptr0, len0, ptr1, len1 );
}

static void videoDecEndPut( VideoDec* vd, int size )
{
    viBufEndPut( &vd->vibuf, size );
}

static void videoDecReset( VideoDec* vd )
{
    vd->state = VD_STATE_NORMAL;
}

static int videoDecDelete( VideoDec* vd )
{
    viBufDelete( &vd->vibuf );
    sceMpegDelete( &vd->mpeg );
    return 1;
}

/*
static void videoDecAbort( VideoDec* vd )
{
    vd->state = VD_STATE_ABORT;
}
*/

static u_int videoDecGetState( VideoDec* vd )
{
    return vd->state;
}

static u_int videoDecSetState( VideoDec* vd, u_int state )
{
    u_int old = vd->state;

    vd->state = state;
    return old;
}

static int videoDecPutTs( VideoDec* vd, long pts_val, long dts_val, u_char* start, int len )
{
    TimeStamp   ts;

    ts.pts = pts_val;
    ts.dts = dts_val;
    ts.pos = start - (u_char*)vd->vibuf.data;
    ts.len = len;
    return viBufPutTs( &videoDec.vibuf, &ts );
}

static int videoDecInputCount( VideoDec* vd )
{
    return viBufCount(&vd->vibuf);
}

/*
static int videoDecInputSpaceCount( VideoDec* vd )
{
    u_char  *ptr0, *ptr1;
    int     len0, len1;

    videoDecBeginPut( vd, &ptr0, &len0, &ptr1, &len1 );
    return len0 + len1;
}
*/

static int videoDecFlush( VideoDec* vd )
{
    u_char  *pd0, *pd1, *pd0Unc, *pd1Unc, seq_end_code[4] = { 0x00, 0x00, 0x01, 0xb7 };
    int d0, d1, len;

    videoDecBeginPut( vd, &pd0, &d0, &pd1, &d1 );
    if( d0 + d1 < 4 )
    {
        return 0;
    }
    pd0Unc = (u_char*)UncAddr( pd0 );
    pd1Unc = (u_char*)UncAddr( pd1 );
    len = nvlMem2Cpy( pd0Unc, d0, pd1Unc, d1, seq_end_code, 4, NULL, 0 );
    videoDecEndPut( &videoDec, len );
    viBufFlush( &vd->vibuf );
    if( vd->state == VD_STATE_NORMAL )
    {
        vd->state = VD_STATE_FLUSH;
    }
    return 1;
}

static int videoDecIsFlushed( VideoDec* vd )
{
    return videoDecInputCount( vd ) == 0 && sceMpegIsRefBuffEmpty( &vd->mpeg );
}

static void videoDecMain( VideoDec* vd )
{
    viBufReset( &vd->vibuf );
    voBufReset( &voBuf );
    decBs0( vd );
    while( voBuf.count )
        ;
    videoDecSetState( vd, VD_STATE_END );
}

static int decBs0( VideoDec* vd )
{
    VoData      *voData;
    sceIpuRGB32 *rgb32;
    int         ret, status = 1;
    sceMpeg     *mp = &vd->mpeg;

    while( !sceMpegIsEnd(&vd->mpeg) )
    {
        if( videoDecGetState(vd) == VD_STATE_ABORT )
        {
            status = -1;
            nvlPrintf("decode thread: aborted\n");
            break;
        }
        while( !(voData = voBufGetData(&voBuf)) )
        {
            RotateThreadReadyQueue( NVLMPEG_PRIORITY );
        }
        rgb32 = (sceIpuRGB32*)voData->v;
        ret = sceMpegGetPicture( mp, rgb32, MAX_WIDTH / 16 * MAX_HEIGHT / 16 );
        if( ret < 0 )
        {
            nvlPrintf("sceMpegGetPicture() decode error");
        }
        if( mp->frameCount == 0 )
        {
            int i, image_w, image_h;

            image_w = mp->width;
            image_h = mp->height;
            for( i = 0; i < voBuf.size; i++ )
            {
                setImageTag( voBuf.tag[i].v[0], voBuf.data[i].v, 0, image_w, image_h );
                setImageTag( voBuf.tag[i].v[1], voBuf.data[i].v, 1, image_w, image_h );
            }
        }
        voBufIncCount( &voBuf );
        RotateThreadReadyQueue( NVLMPEG_PRIORITY );
    }
    sceMpegReset( mp );
    return status;
}

static int mpegError( sceMpeg* mp, sceMpegCbDataError* cberror, void* anyData )
{
    nvlPrintf( "%s\n", cberror->errMessage );
    return 1;
}

static int mpegNodata( sceMpeg* mp, sceMpegCbData* cbdata, void* anyData )
{
    RotateThreadReadyQueue( NVLMPEG_PRIORITY );
    viBufAddDMA( &videoDec.vibuf );
    return 1;
}

static int mpegStopDMA( sceMpeg* mp, sceMpegCbData* cbdata, void* anyData )
{
    viBufStopDMA( &videoDec.vibuf );
    return 1;
}

static int mpegRestartDMA( sceMpeg* mp, sceMpegCbData* cbdata, void* anyData )
{
    viBufRestartDMA( &videoDec.vibuf );
    return 1;
}

static int mpegTS( sceMpeg* mp, sceMpegCbDataTimeStamp* cbts, void* anyData )
{
    TimeStamp ts;

    viBufGetTs(&videoDec.vibuf, &ts );
    cbts->pts = ts.pts;
    cbts->dts = ts.dts;
    return 1;
}

static void voBufCreate( VoBuf* f, VoData* data, VoTag* tag, int size )
{
    f->data = data;
    f->tag = tag;
    f->size = size;
    f->count = 0;
    f->write = 0;
    for( int i = 0; i < size; i++ )
    {
        f->tag[i].status = VOBUF_STATUS_;
    }
}

static void voBufDelete( VoBuf* f )
{
}

static void voBufReset( VoBuf* f )
{
    f->count = 0;
    f->write = 0;
}

static int voBufIsFull( VoBuf* f )
{
    return f->count == f->size;
}

static void voBufIncCount( VoBuf* f )
{
    DI();
    f->tag[f->write].status = VOBUF_STATUS_FULL;
    f->count++;
    f->write = (f->write + 1) % f->size;
    EI();
}

static VoData *voBufGetData( VoBuf* f )
{
    return voBufIsFull( f )? (VoData*)NULL: f->data + f->write;
}

static int voBufIsEmpty( VoBuf* f )
{
    return f->count == 0;
}

static VoTag *voBufGetTag( VoBuf* f )
{
    return voBufIsEmpty( f )? (VoTag*)NULL: f->tag + ( (f->write - f->count + f->size) % f->size );
}

static void voBufDecCount( VoBuf* f )
{
    if( f->count > 0 )
    {
        f->count--;
    }
}

static int strFileOpen( StrFile* file, char* filename )
{
    int     ret;
    char    *body = NULL, fn[256], devname[64], nullstr[] = "", cdendstr[] = ";1";

    body = index( filename, ':' );
    if( body )
    {
        int dlen;

        dlen = body - filename;
        strncpy( devname, filename, dlen );
        devname[dlen] = 0;
        body += 1;
        if( !strcmp(devname, "cdrom0") )
        {
            int     len = strlen(body);
            char    *tail;

            file->isOnCD = 1;
            for( int i = 0; i < len; i++ )
            {
                if( body[i] == '/' )
                {
                    body[i] = '\\';
                }
                body[i] = toupper(body[i]);
            }
            tail = index(filename, ';')? nullstr: cdendstr;
            sprintf( fn, "%s%s", body, tail );
        }
        else
        {
            file->isOnCD = 0;
            sprintf( fn, "%s:%s", devname, body );
        }
    }
    else
    {
        body = filename;
        strcpy( devname, "host0" );
        file->isOnCD = 0;
        sprintf( fn, "%s:%s", devname, body );
    }
    nvlPrintf("file: %s\n", fn);
    if( file->isOnCD )
    {
        sceCdRMode  mode;

        if( !isStrFileInit )
        {
            /*sceCdInit( SCECdINIT );
            sceCdMmode( SCECdCD );*/
            sceCdDiskReady( 0 );
            isStrFileInit = 1;
        }
        file->iopBuf = (unsigned char*)sceSifAllocIopHeap( ((2048 * 80) + 16) );
        sceCdStInit( 80, 5, bound((u_int)file->iopBuf, 16) );
        if( !sceCdSearchFile(&file->fp, fn) )
        {
            nvlPrintf("Cannot open '%s'(sceCdSearchFile)\n", fn);
			strFileClose(file);
            return 0;
        }
        file->size = file->fp.size;
        mode.trycount = 0;
        mode.spindlctrl = SCECdSpinStm;
        mode.datapattern = SCECdSecS2048;
        sceCdStStart( file->fp.lsn, &mode );
    }
    else
    {
        file->fd = sceOpen( fn, SCE_RDONLY );
        if( file->fd < 0 )
        {
            nvlPrintf( "Cannot open '%s'(sceOpen)\n", fn );
            return 0;
        }
        file->size = sceLseek( file->fd, 0, SCE_SEEK_END );
        if( file->size < 0 )
        {
            nvlPrintf("sceLseek() fails (%s): %d\n", fn, file->size);
            sceClose(file->fd);
            return 0;
        }
        ret = sceLseek(file->fd, 0, SCE_SEEK_SET);
        if( ret < 0 )
        {
            nvlPrintf( "sceLseek() fails (%s)\n", fn );
            sceClose( file->fd );
            return 0;
        }
    }
    return 1;
}

static int strFileClose( StrFile* file )
{
    if( file->isOnCD )
    {
        sceCdStStop();
        sceSifFreeIopHeap( (void *)file->iopBuf );
    }
    else
    {
        sceClose( file->fd );
    }
    return 1;
}

static int strFileRead( StrFile* file, void* buff, int size )
{
    int count;

    if( file->isOnCD )
    {
        u_int err;

        count = sceCdStRead( size >> 11, (u_int *)buff, STMBLK, &err );
        count <<= 11;
    }
    else
    {
        count = sceRead( file->fd, buff, size );
    }
    return count;
}

static int audioDecCreate( AudioDec* ad, u_char* buff, int buffSize, int iopBuffSize )
{
    ad->state = AU_STATE_INIT;
    ad->hdrCount = 0;
    ad->data = buff;
    ad->put = 0;
    ad->count = 0;
    ad->size = buffSize;
    ad->totalBytes = 0;
    ad->totalBytesSent = 0;
    ad->iopBuffSize = iopBuffSize;
    ad->iopLastPos = 0;
    ad->iopPausePos = 0;
    ad->iopBuff = (int)sceSifAllocIopHeap( iopBuffSize );
    if( ad->iopBuff == NULL )
    {
        nvlPrintf( "Cannot allocate IOP memory\n" );
        return 0;
    }
//    printf( "IOP memory 0x%08x(size:%d) is allocated\n", ad->iopBuff, iopBuffSize );
    ad->iopZero = (int)sceSifAllocIopHeap( ZERO_BUFF_SIZE );
    if( ad->iopZero == NULL )
    {
        nvlPrintf( "Cannot allocate IOP memory\n" );
        return 0;
    }
//    nvlPrintf( "IOP memory 0x%08x(size:%d) is allocated\n", ad->iopZero, ZERO_BUFF_SIZE );

    memset( _0_buf, 0, ZERO_BUFF_SIZE );
    sendToIOP( ad->iopZero, (unsigned char*)_0_buf, ZERO_BUFF_SIZE );
    changeMasterVolume( 0x3fff );
    return 1;
}

static int audioDecDelete( AudioDec* ad )
{
    sceSifFreeIopHeap( (void *)ad->iopBuff );
    sceSifFreeIopHeap( (void *)ad->iopZero );
    changeMasterVolume( 0 );
    return 1;
}

static void audioDecPause( AudioDec* ad )
{
    int ret;

    ad->state = AU_STATE_PAUSE;
    changeInputVolume( 0 );
    ret = sceSdRemote( 1, rSdBlockTrans, AUTODMA_CH, SD_TRANS_MODE_STOP, NULL, 0 ) & 0x00FFFFFF;
    ad->iopPausePos = ret - ad->iopBuff;
    sceSdRemote( 1, rSdVoiceTrans, AUTODMA_CH, SD_TRANS_MODE_WRITE | SD_TRANS_BY_DMA, ad->iopZero, 0x4000, ZERO_BUFF_SIZE );
}

static void audioDecResume( AudioDec* ad )
{
    changeInputVolume( 0x7fff );
    sceSdRemote( 1, rSdBlockTrans, AUTODMA_CH, SD_TRANS_MODE_WRITE_FROM | SD_BLOCK_LOOP, ad->iopBuff, (ad->iopBuffSize / UNIT_SIZE) * UNIT_SIZE, ad->iopBuff + ad->iopPausePos );
    ad->state = AU_STATE_PLAY;
}

static void audioDecStart( AudioDec* ad )
{
    audioDecResume( ad );
}

static void audioDecReset( AudioDec* ad )
{
    audioDecPause(ad);
    ad->state = AU_STATE_INIT;
    ad->hdrCount = 0;
    ad->put = 0;
    ad->count = 0;
    ad->totalBytes = 0;
    ad->totalBytesSent = 0;
    ad->iopLastPos = 0;
    ad->iopPausePos = 0;
}

static void audioDecBeginPut( AudioDec* ad, u_char** ptr0, int* len0, u_char** ptr1, int* len1 )
{
    int len;

    if( ad->state == AU_STATE_INIT )
    {
        *ptr0 = (u_char*)&ad->sshd + ad->hdrCount;
        *len0 = AU_HDR_SIZE - ad->hdrCount;
        *ptr1 = (u_char*)ad->data;
        *len1 = ad->size;
        return;
    }
    len = ad->size - ad->count;
    if( ad->size -  ad->put >= len )
    {
        *ptr0 = ad->data + ad->put;
        *len0 = len;
        *ptr1 = NULL;
        *len1 = 0;
    }
    else
    {
        *ptr0 = ad->data + ad->put;
        *len0 = ad->size - ad->put;
        *ptr1 = ad->data;
        *len1 = len - ( ad->size - ad->put );
    }
}

static void audioDecEndPut( AudioDec* ad, int size )
{
    if( ad->state == AU_STATE_INIT )
    {
        int hdr_add = min( size, (int)(AU_HDR_SIZE - ad->hdrCount) );

        ad->hdrCount += hdr_add;
        if( ad->hdrCount >= (int)AU_HDR_SIZE )
        {
            ad->state = AU_STATE_PRESET;
/*
            nvlPrintf( "-------- audio information --------------------\n" );
            nvlPrintf(
                    "[%c%c%c%c]\n"
                    "header size:                            %d\n"
                    "type(0:PCM big, 1:PCM little, 2:ADPCM): %d\n"
                    "sampling rate:                          %dHz\n"
                    "channels:                               %d\n"
                    "interleave size:                        %d\n"
                    "interleave start block address:         %d\n"
                    "interleave end block address:           %d\n",
                    ad->sshd.id[0],
                    ad->sshd.id[1],
                    ad->sshd.id[2],
                    ad->sshd.id[3],
                    ad->sshd.size,
                    ad->sshd.type,
                    ad->sshd.rate,
                    ad->sshd.ch,
                    ad->sshd.interSize,
                    ad->sshd.loopStart,
                    ad->sshd.loopEnd
                );
            nvlPrintf(
                    "[%c%c%c%c]\n"
                    "data size:                              %d\n",
                    ad->ssbd.id[0],
                    ad->ssbd.id[1],
                    ad->ssbd.id[2],
                    ad->ssbd.id[3],
                    ad->ssbd.size
                );
*/
        }
        size -= hdr_add;
    }
    ad->put = ( ad->put + size ) % ad->size;
    ad->count += size;
    ad->totalBytes += size;
}

static int audioDecIsPreset( AudioDec* ad )
{
    return ad->totalBytesSent >= PRESET_VALUE( ad->iopBuffSize );
}

static int audioDecSendToIOP( AudioDec* ad )
{
    int     pd0, pd1, d0, d1, s0, s1, count_sent = 0, countAdj, pos = 0;
    u_char  *ps0, *ps1;

    switch( ad->state )
    {
    case AU_STATE_INIT  :
        return 0;
        break;
    case AU_STATE_PRESET:
        pd0 = ad->iopBuff + ( ad->totalBytesSent ) % ad->iopBuffSize;
        d0 = ad->iopBuffSize - ad->totalBytesSent;
        pd1 = 0;
        d1 = 0;
        break;
    case AU_STATE_PLAY  :
        pos = ( (sceSdRemote(1, rSdBlockTransStatus, AUTODMA_CH) & 0x00FFFFFF) - ad->iopBuff );
        iopGetArea( &pd0, &d0, &pd1, &d1, ad, pos );
        break;
    case AU_STATE_PAUSE :
        return 0;
        break;
    }

    ps0 = ad->data + ( ad->put - ad->count + ad->size ) % ad->size;
    ps1 = ad->data;
    countAdj = ( ad->count / UNIT_SIZE ) * UNIT_SIZE;
    s0 = min( ad->data + ad->size - ps0, countAdj );
    s1 = countAdj - s0;
    if( d0 + d1 >= UNIT_SIZE && s0 + s1 >= UNIT_SIZE )
    {
        count_sent = sendToIOP2area(pd0, d0, pd1, d1, ps0, s0, ps1, s1);
    }
    ad->count -= count_sent;
    ad->totalBytesSent += count_sent;
    ad->iopLastPos = ( ad->iopLastPos + count_sent ) % ad->iopBuffSize;
    return count_sent;
}

static void iopGetArea( int *pd0, int *d0, int *pd1, int *d1, AudioDec *ad, int pos )
{
    int len = ( pos + ad->iopBuffSize - ad->iopLastPos - UNIT_SIZE ) % ad->iopBuffSize;
    len = ( len / UNIT_SIZE ) * UNIT_SIZE;
    if( ad->iopBuffSize -  ad->iopLastPos >= len )
    {
        *pd0 = ad->iopBuff + ad->iopLastPos;
        *d0 = len;
        *pd1 = 0;
        *d1 = 0;
    }
    else
    {
        *pd0 = ad->iopBuff + ad->iopLastPos;
        *d0 = ad->iopBuffSize - ad->iopLastPos;
        *pd1 = ad->iopBuff;
        *d1 = len - (ad->iopBuffSize - ad->iopLastPos);
    }
}

static int sendToIOP2area( int pd0, int d0, int pd1, int d1, u_char *ps0, int s0, u_char *ps1, int s1 )
{
    if( d0 + d1 < s0 + s1 )
    {
        int diff = (s0 + s1) - (d0 + d1);

        if( diff >= s1 )
        {
            s0 -= ( diff - s1 );
            s1 = 0;
        }
        else
        {
            s1 -= diff;
        }
    }
    if( s0 >= d0 )
    {
        sendToIOP( pd0, ps0, d0 );
        sendToIOP( pd1, ps0 + d0, s0 - d0 );
        sendToIOP( pd1 + s0 - d0, ps1, s1 );
    }
    else
    {
        if( s1 >= d0 - s0 )
        {
            sendToIOP( pd0, ps0, s0 );
            sendToIOP( pd0 + s0, ps1, d0 - s0 );
            sendToIOP( pd1, ps1 + d0 - s0, s1 - (d0 - s0) );
        }
        else
        {
            sendToIOP( pd0, ps0, s0 );
            sendToIOP( pd0 + s0, ps1, s1 );
        }
    }
    return s0 + s1;
}

static int sendToIOP( int dst, u_char *src, int size )
{
    sceSifDmaData   transData;
    int             did;

    if( size <= 0 )
    {
        return 0;
    }
    transData.data = (u_int)src;
    transData.addr = (u_int)dst;
    transData.size = size;
    transData.mode = 0;
    FlushCache( 0 );
    did = sceSifSetDma( &transData, 1 );
    while( sceSifDmaStat(did) >= 0 )
        ;
    return size;
}

static void changeMasterVolume( u_int val )
{
    for( int i = 0; i < 2; i++ )
    {
        sceSdRemote( 1, rSdSetParam, i | SD_P_MVOLL, val );
        sceSdRemote( 1, rSdSetParam, i | SD_P_MVOLR, val );
    }
}

static void changeInputVolume( u_int val )
{
    sceSdRemote( 1, rSdSetParam, AUTODMA_CH | SD_P_BVOLL, val );
    sceSdRemote( 1, rSdSetParam, AUTODMA_CH | SD_P_BVOLR, val );
}

static inline int IsPtsInRegion( int tgt, int pos, int len, int size )
{
    int tgt1 = (tgt + size - pos) % size;
    return tgt1 < len;
}

static int getFIFOindex( ViBuf* f, void *addr )
{
    if( addr == DmaAddr(f->tag + (f->n + 1)) )
    {
        return 0;
    }
    else
    {
        return ((u_int)addr - (u_int)f->data) / VIBUF_ELM_SIZE;
    }
}

static void setD3_CHCR( u_int val )
{
    DI();
    *D_ENABLEW = (*D_ENABLER)|0x00010000;   // pause all channels
    *D3_CHCR = val;
    *D_ENABLEW = (*D_ENABLER)&~0x00010000;  // restart all channels
    EI();
}

static void setD4_CHCR( u_int val )
{
    DI();
    *D_ENABLEW = (*D_ENABLER)|0x00010000;   // pause all channels
    *D4_CHCR = val;
    *D_ENABLEW = (*D_ENABLER)&~0x00010000;  // restart all channels
    EI();
}

static void scTag2( QWORD* q, void* addr, u_int id, u_int qwc )
{
    q->l[0] = (u_long)(u_int)addr << 32 | (u_long)id << 28 | (u_long)qwc << 0;
}

static int viBufCreate( ViBuf* f, u_long128* data, u_long128* tag, int size, TimeStamp* ts, int n_ts )
{
    struct SemaParam param;

    f->data = data;
    f->tag = (u_long128*)UncAddr( tag );
    f->n = size;
    f->buffSize = size * VIBUF_ELM_SIZE;
    f->ts = ts;
    f->n_ts = n_ts;
    param.initCount = 1;
    param.maxCount = 1;
    f->sema = CreateSema( &param );
    viBufReset(f);
    f->totalBytes = 0;
    return TRUE;
}

static int viBufReset( ViBuf* f )
{
    int i;

    f->dmaStart = 0;
    f->dmaN = 0;
    f->readBytes = 0;
    f->isActive = TRUE;
    f->count_ts = 0;
    f->wt_ts = 0;
    for( i = 0; i < f->n_ts; i++ )
    {
        f->ts[i].pts = TS_NONE;
        f->ts[i].dts = TS_NONE;
        f->ts[i].pos = 0;
        f->ts[i].len = 0;
    }
    for( i = 0; i < f->n; i++ )
    {
        scTag2( (QWORD*)(f->tag + i), DmaAddr((char*)f->data + VIBUF_ELM_SIZE * i), DMA_ID_REF, VIBUF_ELM_SIZE / 16 );
    }
    scTag2( (QWORD*)(f->tag + i), DmaAddr(f->tag), DMA_ID_NEXT, 0 );

    *D4_QWC = 0;
    *D4_MADR = (u_int)DmaAddr( f->data );
    *D4_TADR = (u_int)DmaAddr( f->tag );
    setD4_CHCR( (0 << 8) | (1 << 2) | 1 );

    return TRUE;
}

static void viBufBeginPut( ViBuf *f, u_char **ptr0, int *len0, u_char **ptr1, int *len1 )
{
    int es, en, fs, fn;

    WaitSema( f->sema );
    fs = FS( f );
    fn = FN( f );
    es = ( fs + f->readBytes ) % f->buffSize;
    en = fn - f->readBytes;
    if( f->buffSize - es >= en )
    {
        *ptr0 = (u_char*)f->data + es;
        *len0 = en;
        *ptr1 = NULL;
        *len1 = 0;
    }
    else
    {
        *ptr0 = (u_char*)f->data + es;
        *len0 = f->buffSize - es;
        *ptr1 = (u_char*)f->data;
        *len1 = en - (f->buffSize - es);
    }
    SignalSema(f->sema);
}

static void viBufEndPut( ViBuf* f, int size )
{
    WaitSema( f->sema );
    f->readBytes += size;
    f->totalBytes += size;
    SignalSema(f->sema);
}

static int viBufAddDMA( ViBuf* f )
{
    int i, index, id, last, isNewData = 0, consume, read_start, read_n;
    u_int   d4chcr;

    WaitSema(f->sema);
    if( !f->isActive )
    {
        nvlPrintf( "DMA ADD not active\n" );
        SignalSema( f->sema );
        return false;
    }

    ///////////////////////////////////////////
    // STOP DMA ch4
    //
    // d4chcr:
    //	(1) DMA was running
    //	    ORIGINAL DMA tag
    //	(2) DMA was not running
    //	    REFE tag
    setD4_CHCR( (DMA_ID_REFE<<28) | (0<<8) | (1<<2) | 1 );
    d4chcr = *D4_CHCR;
    // update dma pointer using D4_MADR
    index = getFIFOindex( f, (void*)*D4_MADR );
    consume = ( index + f->n - f->dmaStart ) % f->n;
    f->dmaStart = ( f->dmaStart + consume ) % f->n;
    f->dmaN -= consume;
    // update read pointer
    read_start = ( f->dmaStart + f->dmaN ) % f->n;
    read_n = f->readBytes / VIBUF_ELM_SIZE;
    f->readBytes %= VIBUF_ELM_SIZE;
    // the last REFE -> REF
    if( read_n > 0 )
    {
        last = ( f->dmaStart + f->dmaN - 1 + f->n ) % f->n;
        scTag2( (QWORD*)(f->tag + last), (char*)f->data + VIBUF_ELM_SIZE * last, DMA_ID_REF, VIBUF_ELM_SIZE / 16 );
        isNewData = 1;
    }
    index = read_start;
    for( i = 0; i < read_n; i++ )
    {
        id = (i == read_n - 1)? DMA_ID_REFE: DMA_ID_REF;
        scTag2( (QWORD*)(f->tag + index), (char*)f->data + VIBUF_ELM_SIZE * index, id, VIBUF_ELM_SIZE / 16 );
        index = ( index + 1 ) % f->n;
    }
    f->dmaN += read_n;
    // RESTART DMA ch4
    if( f->dmaN )
    {
        if( isNewData )
        {
            // change ref/refe ----> ref
            d4chcr = (d4chcr & 0x0fffffff) | (DMA_ID_REF << 28);
        }
        setD4_CHCR(d4chcr | 0x100);
    }
    SignalSema(f->sema);
    return TRUE;
}

static int viBufStopDMA( ViBuf* f )
{
    WaitSema( f->sema );
    f->isActive = FALSE;
    setD4_CHCR( (0<<8) | (1<<2) | 1 );  // STR: 0, DIR: 1
    f->env.d4madr = *D4_MADR;
    f->env.d4tadr = *D4_TADR;
    f->env.d4qwc =  *D4_QWC;
    f->env.d4chcr = *D4_CHCR;
    while( DGET_IPU_CTRL() & 0xf0 )
        ;
    setD3_CHCR( (0<<8) | 0 );           // STR: 0, DIR: 0
    f->env.d3madr = *D3_MADR;
    f->env.d3qwc =  *D3_QWC;
    f->env.d3chcr = *D3_CHCR;
    f->env.ipubp = DGET_IPU_BP();
    f->env.ipuctrl = DGET_IPU_CTRL();
    SignalSema(f->sema);
    return TRUE;
}

static int viBufRestartDMA( ViBuf* f )
{
    int     bp, fp, ifc, index, index_next, id;
    u_int   d4madr_next, d4qwc_next, d4tadr_next, d4chcr_next;

    WaitSema( f->sema );
    bp = f->env.ipubp & 0x7f;
    fp = ( f->env.ipubp >> 16 ) & 0x3;
    ifc = ( f->env.ipubp >> 8 ) & 0xf;
    d4madr_next = f->env.d4madr - ( (fp + ifc) << 4 );
    d4qwc_next = f->env.d4qwc + ( fp + ifc );
    d4tadr_next = f->env.d4tadr;
    d4chcr_next = f->env.d4chcr | 0x100;
    // check wrap around
    if( d4madr_next < (u_int)f->data )
    {
        d4qwc_next = ( DATA_ADDR(0) - d4madr_next ) >> 4;
        d4madr_next += (u_int)( f->n * VIBUF_ELM_SIZE );
        d4tadr_next = TAG_ADDR( 0 );
        id = ( f->env.d4madr == DATA_ADDR(0) || f->env.d4madr == DATA_ADDR(f->n) )? DMA_ID_REFE : DMA_ID_REF;
        d4chcr_next = (f->env.d4chcr & 0x0fffffff) | (id << 28) | 0x100;
        if( !IsInRegion(0, f->dmaStart, f->dmaN, f->n) )
        {
            f->dmaStart = f->n - 1;
            f->dmaN++;
        }
    }
    else if( (index = getFIFOindex(f, (void*)f->env.d4madr)) != (index_next = getFIFOindex(f, (void*)d4madr_next)) )
    {
        d4tadr_next = TAG_ADDR( index );
        d4qwc_next = ( DATA_ADDR(index) - d4madr_next ) >> 4;
        id = ( WRAP_ADDR(f->env.d4madr) == DATA_ADDR((f->dmaStart + f->dmaN) % f->n) )? DMA_ID_REFE : DMA_ID_REF;
        d4chcr_next = ( f->env.d4chcr & 0x0fffffff ) | (id << 28) | 0x100;
        if( !IsInRegion(index_next, f->dmaStart, f->dmaN, f->n) )
        {
            f->dmaStart = index_next;
            f->dmaN++;
        }
    }
    if( f->env.d3madr && f->env.d3qwc )
    {
        *D3_MADR = f->env.d3madr;
        *D3_QWC  = f->env.d3qwc;
        setD3_CHCR( f->env.d3chcr | 0x100 );
    }
    if( f->dmaN )
    {
        while( sceIpuIsBusy() )
            ;
        sceIpuBCLR( bp );
        while( sceIpuIsBusy() )
            ;
    }
    *D4_MADR = d4madr_next;
    *D4_TADR = d4tadr_next;
    *D4_QWC  = d4qwc_next;
    if( f->dmaN )
    {
        setD4_CHCR( d4chcr_next );
    }
    *IPU_CTRL = f->env.ipuctrl;
    f->isActive = TRUE;
    SignalSema(f->sema);
    return true;
}

static int viBufDelete( ViBuf* f )
{
    setD4_CHCR( (0<<8) | (1<<2) | 1 );  // STR: 0, chain, DIR: 1
    *D4_QWC = 0;
    *D4_MADR = 0;
    *D4_TADR = 0;
    DeleteSema( f->sema );
    return TRUE;
}

/*
static int viBufIsActive( ViBuf* f )
{
    int ret;

    WaitSema(f->sema);
    ret = f->isActive;
    SignalSema( f->sema );
    return ret;
}
*/

static int viBufCount( ViBuf* f )
{
    int ret;

    WaitSema( f->sema );
    ret = f->dmaN * VIBUF_ELM_SIZE + f->readBytes;
    SignalSema( f->sema );
    return ret;
}

static void viBufFlush( ViBuf* f )
{
    WaitSema( f->sema );
    f->readBytes = bound( f->readBytes, VIBUF_ELM_SIZE );
    SignalSema( f->sema );
}

static int viBufModifyPts( ViBuf* f, TimeStamp* new_ts )
{
    TimeStamp   *ts;
    int         rd = ( f->wt_ts - f->count_ts + f->n_ts ) % f->n_ts, datasize =  VIBUF_ELM_SIZE * f->n, loop = 1;

    if( f->count_ts > 0 )
    {
        while( loop )
        {
            ts = f->ts + rd;
            if( ts->len == 0 || new_ts->len == 0 )
            {
                break;
            }
            if( IsPtsInRegion(ts->pos, new_ts->pos, new_ts->len, datasize) )
            {
                int len = min( new_ts->pos + new_ts->len - ts->pos, ts->len );

                ts->pos = (ts->pos + len) % datasize;
                ts->len -= len;
                if( ts->len == 0 )
                {
                    if( ts->pts >= 0 )
                    {
                        ts->pts = TS_NONE;
                        ts->dts = TS_NONE;
                        ts->pos = 0;
                        ts->len = 0;
                    }
                    f->count_ts = max(f->count_ts - 1, 0);
                }
            }
            else
            {
                loop = 0;
            }
            rd = (rd + 1) % f->n_ts;
        }
    }
    return 0;
}

static int viBufPutTs( ViBuf* f, TimeStamp* ts )
{
    int ret = 0;

    WaitSema( f->sema );
    if( f->count_ts < f->n_ts )
    {
        viBufModifyPts( f, ts );
        if( ts->pts >= 0 || ts->dts >= 0 )
        {
            f->ts[f->wt_ts].pts = ts->pts;
            f->ts[f->wt_ts].dts = ts->dts;
            f->ts[f->wt_ts].pos = ts->pos;
            f->ts[f->wt_ts].len = ts->len;
            f->count_ts++;
            f->wt_ts = (f->wt_ts + 1) % f->n_ts;
        }
        ret = 1;
    }
    SignalSema( f->sema );
    return ret;
}

static int viBufGetTs( ViBuf* f, TimeStamp* ts )
{
    u_int   d4madr = *D4_MADR, ipubp = DGET_IPU_BP(), d4madr_next, stop;
    int     bp, fp = (ipubp >> 16) & 0x3, ifc = (ipubp >> 8) & 0xf, datasize, isEnd = 0, tscount, wt;

    WaitSema( f->sema );
    bp = f->env.ipubp & 0x7f;
    d4madr_next = d4madr - ( (fp + ifc) << 4 );
    datasize =  VIBUF_ELM_SIZE * f->n;
    ts->pts = TS_NONE;
    ts->dts = TS_NONE;
    stop = ( d4madr_next + (bp >> 3) + datasize - (u_int)f->data ) % datasize;
    tscount = f->count_ts;
    wt = f->wt_ts;
    for( int i = 0; i < tscount && !isEnd; i++ )
    {
        int rd = ( wt - tscount + f->n_ts + i ) % f->n_ts;

        if( IsPtsInRegion(stop, f->ts[rd].pos, f->ts[rd].len, datasize) )
        {
            ts->pts = f->ts[rd].pts;
            ts->dts = f->ts[rd].dts;
            f->ts[rd].pts = TS_NONE;
            f->ts[rd].dts = TS_NONE;
            isEnd = 1;
            f->count_ts -= min( 1, f->count_ts );
        }
    }
    SignalSema( f->sema );
    return 1;
}

void nvlMPEGSetVolume ( float vol )
{
    if( vol < 0 ) vol = 0;
    else if( vol > 1.0f ) vol = 1.0f;
    changeInputVolume( nvlFTOI(16383.0f * vol) );
}