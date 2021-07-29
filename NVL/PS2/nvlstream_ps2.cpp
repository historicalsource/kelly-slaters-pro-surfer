#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <sifdev.h>
#include <eekernel.h>
#include <stdio.h>
#include <libcdvd.h>
#include <sifrpc.h>
#include <ctype.h>

#include "nvlstream_ps2.h"

#define NVL_STREAM_HOST_IO_SUPPORT

#define NVLSTREAM_THREAD_STACK_SIZE   4096
#define CALLBACK_THREAD_STACK_SIZE    4096
#define NVLSTREAM_MSG_NUM             8
#define CDVD_MSG_NUM                  1
#define NVL_STREAM_MIN_BUF_SIZE       0x100000
#define DVD_SECTOR_SIZE               2048
#define NVL_STREAM_MIN_READ_SIZE      DVD_SECTOR_SIZE
#define NVL_STREAM_MAX_READ_SIZE      (DVD_SECTOR_SIZE*512)   // 1MB
#define NVL_VB_PENDING_MAX            1
#define NVL_STREAM_MAX_TRIES          3600                    // 1 min. at 60 FPS
#define NVL_PROCESS_SORTED_NUM        1

typedef int nvlMsg;
typedef void (*callbackType)(int);

enum nvlMsgMode
{
  NVL_MSG_BLOCK,
  NVL_MSG_NOBLOCK
};

enum
{
  NVL_MSG_INVALID   = -1,
  NVL_MSG_MIN       = 0,
  NVL_MSG_SHUTDOWN  = NVL_MSG_MIN,
  NVL_MSG_VBLANK,
  NVL_MSG_READ,
  NVL_MSG_UNLOCK,
  NVL_MSG_REWIND,
  NVL_MSG_DESTROY,
  NVL_MSG_MAX,

  // CD callback messages
  NVL_MSG_CDVD_READ,
  NVL_MSG_CDVD_SEEK,
  NVL_MSG_CDVD_STANDBY,
  NVL_MSG_CDVD_STOP,
  NVL_MSG_CDVD_PAUSE,
  NVL_MSG_CDVD_BREAK,
};

enum
{
  NVL_STREAM_FLAG_INVALID                 = 0,
  NVL_STREAM_FLAG_ASYNC_READ_IN_PROGRESS  = 0x00000001,
  NVL_STREAM_FLAG_SYNC_READ_IN_PROGRESS   = 0x00000002,
  NVL_STREAM_FLAG_READ_IN_PROGRESS        = NVL_STREAM_FLAG_ASYNC_READ_IN_PROGRESS | NVL_STREAM_FLAG_SYNC_READ_IN_PROGRESS,
  NVL_STREAM_FLAG_LOOPING                 = 0x00000004,
  NVL_STREAM_FLAG_AT_EOF                  = 0x00000008,
  NVL_STREAM_FLAG_HOST_FILE               = 0x00000010
};

typedef struct nvlMsgQueue
{
  int     sema;
  nvlMsg  *array;
} nvlMsgQeue;

struct nvlStream
{
  int           idx;
  unsigned int  startSector;
  unsigned int  endSector;
  unsigned int  readSector;
  unsigned int  filesize;
  int           bytes_in_last_sect;
  int           bufsize;
  char          *buf;
  char          *endbuf;
  char          *pRead;
  char          *pWrite;
  char          *pLock;
  char          *pEndOfData;
  char          *pStartOfData;
  int           private_buffer;
  int           bitrate;        // Very, very rough approximation
  unsigned int  flags;
  int           available;
  nvlMutex      mtx;
  int           lp_skip;
  int           requireRewind;
#ifdef DEBUG
  int           bytesReadOffDisk;
#endif
#ifdef NVL_STREAM_HOST_IO_SUPPORT
  int           fh;
#endif
};

static struct nvlStreamSystemData_t
{
  char          *threadStack;
  ThreadParam   threadParam;
  int           threadID;
  int           mainThreadID;
  nvlMsgQueue   threadMsgQueue;
  nvlMsg        msg[NVLSTREAM_MSG_NUM];
  nvlMsgQueue   cdvdMsgQueue;
  nvlMsg        cdvdMsg[CDVD_MSG_NUM];
  int           vblankHandlerID;
  int           VBWasEnabled;
  int           vblankSema;
  nvlStream     *pStream[NVL_MAX_STREAM_NUM];
  int           streamList[NVL_MAX_STREAM_NUM];
  callbackType  prevCDVDCallback;
}                               nvlStreamSystemData;
static char                     nvlCallbackStack[CALLBACK_THREAD_STACK_SIZE] __attribute__ ((aligned(16)));
static int                      nvlStreamSystemInitialized = 0, nvlStreamSystemLibsInitialized = 0;
static nvlThreadHandleCallback  threadControlFunc = NULL;
static volatile int             nvlStreamSystemThreadIsRunning = 0, nvlVBMessagePending = 0;
static void                     (*nvlDVDErrorCallback)(int);
static volatile nvlStream       *nvlCurrentIOStream = NULL;
static void*                    (*nvlStreamAllocFunc)(int, int);
static void                     (*nvlStreamFreeFunc)(void*);

static void nvlInitMsgQueue( nvlMsgQueue* q, nvlMsg* msgArray, int depth )
{
  struct SemaParam  semaParam;

  assert( q && msgArray && depth );
  q->array = msgArray;
  semaParam.initCount = 0;
  semaParam.maxCount = depth;
  q->sema = CreateSema( &semaParam );
  assert( q->sema >= 0 );
}

static void nvlDestroyMsgQueue( nvlMsgQueue* q )
{
  assert( q );
#ifdef DEBUG
  int res =
#endif
  DeleteSema( q->sema );
#ifdef DEBUG
  assert( res == q->sema );
#endif
}

static nvlMsg nvlReceiveMsg( nvlMsgQueue* q, nvlMsgMode mode, nvlStream** s )
{
  nvlMsg  rv = NVL_MSG_INVALID;
  int     res;

  assert( q && s );
  if( mode == NVL_MSG_BLOCK )
  {
    res = WaitSema( q->sema );
  }
  else
  {
    res = PollSema( q->sema );
  }
  assert( !(mode == NVL_MSG_BLOCK && res != q->sema) );
  if( res == q->sema )
  {
    struct SemaParam  semaParam;

    res = ReferSemaStatus( q->sema, &semaParam );
    assert( res == q->sema && semaParam.currentCount >= 0 );
    rv = q->array[semaParam.currentCount];
    res = rv >> 16;
    assert( res >= -1 && res < NVL_MAX_STREAM_NUM );
    *s = ( res >= 0 )? nvlStreamSystemData.pStream[res] : NULL;
    assert( *s == NULL || (*s)->idx == res );
    rv &= 0xffff;
  }
  return rv;
}

/*
Commented out because it's not used so far which causes a compiler warning
// Must be called from an interrupt handler only.
static nvlMsg nvlIReceiveMsg( nvlMsgQueue* q, nvlStream** s )
{
  nvlMsg  rv = NVL_MSG_INVALID;
  int     res;

#ifdef DEBUG
  if( !q || !s )
  {
    asm( "break" );
  }
#endif
  res = iPollSema( q->sema );
  if( res == q->sema )
  {
    struct SemaParam  semaParam;

    res = iReferSemaStatus( q->sema, &semaParam );
    assert( res == q->sema && semaParam.currentCount >= 0 );
    rv = q->array[semaParam.currentCount];
    res = rv >> 16;
#ifdef DEBUG
    if( res < -1 || res >= NVL_MAX_STREAM_NUM )
    {
      asm( "break" );
    }
#endif
    *s = ( res >= 0 )? nvlStreamSystemData.pStream[res] : NULL;
#ifdef DEBUG
    if( *s != NULL && (*s)->idx != res );
    {
      asm( "break" );
    }
#endif
    rv &= 0xffff;
  }
  return rv;
}
*/

static void nvlSendMsg( nvlMsgQueue* q, nvlMsg msg, nvlStream* s )
{
  struct SemaParam  semaParam;
  int               res;

  msg = ( ((s)? s->idx : -1) << 16 ) | msg;
  DI();
  res = ReferSemaStatus( q->sema, &semaParam );
  assert( res == q->sema && semaParam.currentCount >= 0 );
  q->array[semaParam.currentCount] = msg;
  SignalSema( q->sema );
  EI();
}

// Must be called from an interrupt handler only.
static void nvlISendMsg( nvlMsgQueue* q, nvlMsg msg, nvlStream* s )
{
  struct SemaParam  semaParam;
#ifdef DEBUG
  int               res;
#endif

  msg = ( ((s)? s->idx : -1) << 16 ) | msg;
#ifdef DEBUG
  res =
#endif
  iReferSemaStatus( q->sema, &semaParam );
#ifdef DEBUG
  if( res != q->sema || semaParam.currentCount < 0 )
  {
    asm( "break" );
  }
#endif
  q->array[semaParam.currentCount] = msg;
  iSignalSema( q->sema );
}

void nvlWaitForVB()
{
  SignalSema( nvlStreamSystemData.vblankSema );
  PollSema( nvlStreamSystemData.vblankSema );
  WaitSema( nvlStreamSystemData.vblankSema );
}

static inline int nvlStreamReadOK( nvlStream* s )
{
  if(
      s == NULL ||
      (s->flags & NVL_STREAM_FLAG_READ_IN_PROGRESS) ||
      ((s->flags & NVL_STREAM_FLAG_AT_EOF) && !(s->flags & NVL_STREAM_FLAG_LOOPING) ) ||
      s->bufsize - s->available < NVL_STREAM_MIN_READ_SIZE
    )
  {
    return 0;
  }
  return 1;
}

/*
Return value:
 < 0  if stream indexed by "a" has higher priority than stream indexed by "b"
 > 0  if stream indexed by "a" has lower priority than stream indexed by "b"
== 0  if both streams are about the same priorities
*/
static inline int cmp( int a, int b )
{
  int         rv = 0;
  nvlStream   *sa = nvlStreamSystemData.pStream[a], *sb = nvlStreamSystemData.pStream[b];

  assert( a >= 0 && a < NVL_MAX_STREAM_NUM && b >= 0 && b < NVL_MAX_STREAM_NUM );

  if( !nvlStreamReadOK(sa) )
  {
    sa = NULL;
  }
  if( !nvlStreamReadOK(sb) )
  {
    sb = NULL;
  }

  if( sa == NULL )
  {
    if( sb ) rv = 1;
  }
  else if( sb == NULL )
  {
    rv = -1;
  }
  else
  {
    int a_pri, b_pri;

    if( sa->bitrate == 0 || sb->bitrate == 0 )
    {
      a_pri = sa->available;
      b_pri = sb->available;
    }
    else
    {
      a_pri = sa->available / sa->bitrate;
      b_pri = sb->available / sb->bitrate;
    }
    if( a_pri < b_pri )
    {
      rv = -1;
    }
    else if( a_pri > b_pri )
    {
      rv = 1;
    }
  }
  return rv;
}

static inline void sort()
{
  for( int i = 1; i < NVL_MAX_STREAM_NUM; i++ )
  {
    int cur = nvlStreamSystemData.streamList[i], l = 0, r = i - 1;

    while( l <= r )
    {
      int m = ( l + r ) >> 1;
      if( cmp(cur, nvlStreamSystemData.streamList[m]) < 0 )
      {
        r = m - 1;
      }
      else
      {
        l = m + 1;
      }
    }
/*
    for( int j = i - 1; j >= l; j-- )
    {
      nvlStreamSystemData.streamList[j + 1] = nvlStreamSystemData.streamList[j];
    }
*/
    memmove( &nvlStreamSystemData.streamList[l + 1], &nvlStreamSystemData.streamList[l], (i - l) * sizeof(int) );
    nvlStreamSystemData.streamList[l] = cur;
  }
}

static void nvlReadDVD( nvlStream* s )
{
  if( nvlCurrentIOStream )
  {
    // It shouldn't happen cos we are processing some our stream
    assert( 0 && "nvlStream internal error." );
    return;
  }

  int   byte_num = 0;

  if( s->pLock && s->pLock >= s->pWrite )
  {
    byte_num = s->pLock - s->pWrite;
  }
  else if( s->pRead == NULL || s->pRead < s->pWrite || (s->pRead == s->pWrite && s->available == 0) )
  {
    byte_num = s->endbuf - s->pWrite;
  }
  else
  {
    assert( s->pRead >= s->pWrite );
    byte_num = s->pRead - s->pWrite;
  }
  if( byte_num < NVL_STREAM_MIN_READ_SIZE )
  {
    byte_num = 0;
  }
  else if( byte_num > NVL_STREAM_MAX_READ_SIZE )
  {
    byte_num = NVL_STREAM_MAX_READ_SIZE;
  }

  if( byte_num )
  {
    int         res, sector_num;
    sceCdRMode  mode =  {
                          1,            // ... + 1 = 2
                          SCECdSpinNom, // if errors happen change to SCECdSpinStm
                          SCECdSecS2048,// DVD_SECTOR_SIZE == 2048
                          0
                        };

    sector_num = byte_num / DVD_SECTOR_SIZE;
    if( (unsigned)sector_num > s->endSector - s->readSector + 1 )
    {
      sector_num = s->endSector - s->readSector + 1;
    }
    assert( s->readSector <= s->endSector && sector_num > 0 );
    nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
    if( s->flags & NVL_STREAM_FLAG_READ_IN_PROGRESS )
    {
      nvlUnlockMutex( &s->mtx );
      return;
    }
    s->flags |= NVL_STREAM_FLAG_SYNC_READ_IN_PROGRESS;
#ifdef DEBUG
    if( s->readSector == s->startSector )
    {
      s->bytesReadOffDisk = 0;
    }
#endif
    nvlUnlockMutex( &s->mtx );

readDVD:
    assert( nvlCurrentIOStream == NULL );
    nvlCurrentIOStream = s;
    for( int i = 0; sceCdRead(s->readSector, sector_num, s->pWrite, &mode) == 0; i++ )
    {
      assert( i < NVL_STREAM_MAX_TRIES );
      nvlWaitForVB();
    }
/*
    sceCdSync function with 0 as parameter which means block until the read's done crashes, so, here's
    a workaround for that.
*/
    nvlStream   *str;

    res = nvlReceiveMsg( &nvlStreamSystemData.cdvdMsgQueue, NVL_MSG_BLOCK, &str );
    assert( res == NVL_MSG_CDVD_READ && (str == s || str == NULL) );

    res = sceCdGetError();
    switch( res )
    {
    case SCECdErNO      : // No Error
      break;

    case SCECdErFAIL    : // sceCdGetError() function issue failed
    case SCECdErEOM     : // Outermost track reached during playback
    case SCECdErCUD     : // Not appropriate for disc in drive
    case SCECdErNORDY   : // Processing command
    case SCECdErCMD     : // Invalid command
    case SCECdErPRM     : // Invalid parameters
    case SCECdErILI     : // Invalid number of callbacks
    case SCECdErIPI     : // Address error
      assert( 0 && "Internal NVL error." );
      break;

    case SCECdErTRMOPN  : // Cover opened during playback
    case SCECdErREAD    : // Problem occurred during read
    case SCECdErABRT    : // Abort command received
    case SCECdErNODISC  : // No disk
      // These cases should be handled on the application side, i.e. a callback function required
      if( nvlDVDErrorCallback )
      {
        (*nvlDVDErrorCallback)( res );
      }
      goto readDVD;
      break;

    case SCECdErOPENS   : // I don't know what it is but it shouldn't happen
    default             :
      assert( 0 && "Unexpected error code." );
    }

    nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
    s->readSector += sector_num;
    byte_num = sector_num * DVD_SECTOR_SIZE;
    s->pWrite += byte_num;
    assert( s->pWrite <= s->endbuf );
    if( s->readSector > s->endSector )
    {
      assert( s->pEndOfData == NULL && "Buffer is bigger than file." );
      s->flags |= NVL_STREAM_FLAG_AT_EOF;
      byte_num -= DVD_SECTOR_SIZE - s->bytes_in_last_sect;
      s->pEndOfData = s->pWrite - DVD_SECTOR_SIZE + s->bytes_in_last_sect;
      if( s->flags & NVL_STREAM_FLAG_LOOPING )
      {
        if( s->pWrite == s->endbuf )
        {
          s->pWrite = s->buf;
        }
        s->pStartOfData = s->pWrite;
        s->readSector = s->startSector;
      }
    }
    else
    {
      s->flags &= ~NVL_STREAM_FLAG_AT_EOF;
    }
    s->available += byte_num;
#ifdef DEBUG
    s->bytesReadOffDisk += byte_num;
#endif
    if( s->pWrite == s->endbuf )
    {
      s->pWrite = s->buf;
    }
    if( s->pRead == NULL )
    {
      s->pRead = s->buf;
    }
    s->flags &= ~NVL_STREAM_FLAG_SYNC_READ_IN_PROGRESS;
    nvlUnlockMutex( &s->mtx );
  }
}

#ifdef NVL_STREAM_HOST_IO_SUPPORT
static void nvlReadHost( nvlStream* s )
{
  int   byte_num = 0, res;

  if( s->pLock && s->pLock >= s->pWrite )
  {
    byte_num = s->pLock - s->pWrite;
  }
  else if( s->pRead == NULL || s->pRead < s->pWrite || (s->pRead == s->pWrite && s->available == 0) )
  {
    byte_num = s->endbuf - s->pWrite;
    assert( byte_num == 0 || byte_num >= DVD_SECTOR_SIZE );
  }
  else
  {
    assert( s->pRead >= s->pWrite );
    byte_num = s->pRead - s->pWrite;
  }
  if( byte_num < NVL_STREAM_MIN_READ_SIZE )
  {
    byte_num = 0;
  }
  else if( byte_num > NVL_STREAM_MAX_READ_SIZE )
  {
    byte_num = NVL_STREAM_MAX_READ_SIZE;
  }
  byte_num &= ~( DVD_SECTOR_SIZE - 1 );

  if( byte_num )
  {
    nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
    if( s->flags & NVL_STREAM_FLAG_READ_IN_PROGRESS )
    {
      nvlUnlockMutex( &s->mtx );
      return;
    }
    s->flags |= NVL_STREAM_FLAG_SYNC_READ_IN_PROGRESS;
    nvlUnlockMutex( &s->mtx );

    res = sceRead( s->fh, s->pWrite, byte_num );
    FlushCache( WRITEBACK_DCACHE );
    assert( res >= 0 );

    nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
    s->pWrite += byte_num;
    assert( s->pWrite <= s->endbuf );
    if( res < byte_num )
    {
      assert( s->pEndOfData == NULL && "Buffer is bigger than file." );
      s->flags |= NVL_STREAM_FLAG_AT_EOF;
      s->pEndOfData = s->pWrite - byte_num + res;
      if( s->flags & NVL_STREAM_FLAG_LOOPING )
      {
        if( s->pWrite >= s->endbuf )
        {
          s->pWrite = s->buf;
        }
        s->pStartOfData = s->pWrite;
        sceLseek( s->fh, s->lp_skip, SCE_SEEK_SET );
      }
    }
    else if( s->flags & NVL_STREAM_FLAG_AT_EOF )
    {
#ifdef DEBUG
      s->bytesReadOffDisk = 0;
#endif
      s->flags &= ~NVL_STREAM_FLAG_AT_EOF;
    }
    s->available += res;
#ifdef DEBUG
    s->bytesReadOffDisk += res;
#endif
    if( s->pWrite == s->endbuf )
    {
      s->pWrite = s->buf;
    }
    if( s->pRead == NULL )
    {
      s->pRead = s->buf;
    }
    s->flags &= ~NVL_STREAM_FLAG_SYNC_READ_IN_PROGRESS;
    nvlUnlockMutex( &s->mtx );
  }
}
#endif  // NVL_STREAM_HOST_IO_SUPPORT

static void nvlStreamSystemThread( void* data )
{
  if( threadControlFunc )
  {
    (*threadControlFunc)( NVL_TC_REGISTER, 0, NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY );
    (*threadControlFunc)( NVL_TC_CHANGE_MAIN_PRI, 0, NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY + 1 );
  }
  else
  {
#ifdef DEBUG
    int   mainThreadPri =
#endif
    ChangeThreadPriority( nvlStreamSystemData.mainThreadID, NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY + 1 );
#ifdef DEBUG
    assert( mainThreadPri >= 0 );
#endif
  }
  for(;;)
  {
    nvlStream   *s;
    nvlMsg      msg = nvlReceiveMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_BLOCK, &s );

    switch( msg )
    {
    case NVL_MSG_READ     :
      assert( s );
#ifdef NVL_STREAM_HOST_IO_SUPPORT
      if( s->flags & NVL_STREAM_FLAG_HOST_FILE )
      {
        nvlReadHost( s );
      }
      else
#endif
      nvlReadDVD( s );
      break;
    case NVL_MSG_UNLOCK   :
      assert( s );
      nvlStreamUnlock( s );
      break;
    case NVL_MSG_REWIND   :
      assert( s );
      nvlStreamRewind( s );
      break;
    case NVL_MSG_DESTROY  :
      assert( s && s->buf );
      if( !(s->flags & NVL_STREAM_FLAG_READ_IN_PROGRESS) )
      {
        int res = nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
        assert( res );
        assert( nvlStreamSystemData.pStream[s->idx] == s );
        nvlStreamSystemData.pStream[s->idx] = NULL;
        nvlDestroyMutex( &s->mtx );
#ifdef NVL_STREAM_HOST_IO_SUPPORT
        if( s->flags & NVL_STREAM_FLAG_HOST_FILE )
        {
          sceClose( s->fh );
        }
#endif
        if( s->private_buffer )
        {
          if( nvlStreamFreeFunc )
          {
            (*nvlStreamFreeFunc)( s->buf );
          }
          else
          {
            free( s->buf );
          }
        }
        if( nvlStreamFreeFunc )
        {
          (*nvlStreamFreeFunc)( s );
        }
        else
        {
          free( s );
        }
      }
      else
      {
        assert( 0 && "Have to figure out what to do in this case." );
      }
      break;
    case NVL_MSG_VBLANK   :
      assert( s == NULL );
      sort();
      for( int i = 0; i < NVL_PROCESS_SORTED_NUM; i++ )
      {
        assert( i < NVL_MAX_STREAM_NUM && i < NVLSTREAM_MSG_NUM );
        s = nvlStreamSystemData.pStream[nvlStreamSystemData.streamList[i]];
        if( !nvlStreamReadOK(s) )
        {
          break;
        }
        nvlSendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_READ, s );
      }
      nvlVBMessagePending--;
      break;
    case NVL_MSG_SHUTDOWN :
      goto exit_thread;
    default               :
      assert( 0 && "Unexpected message." );
      break;
    }
  }
exit_thread:
  nvlStreamSystemThreadIsRunning = 0;
  ExitThread();
}

static int nvlVBlankHandler( int )
{
  if( nvlStreamSystemInitialized )
  {
    iSignalSema( nvlStreamSystemData.vblankSema );
    if( nvlVBMessagePending < NVL_VB_PENDING_MAX )
    {
      nvlISendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_VBLANK, NULL );
      nvlVBMessagePending++;
    }
  }
  ExitHandler();
  return 0;
}

static void nvlStreamSystemCallback( int cause )
{
  nvlMsg  msg = NVL_MSG_INVALID;
  switch( cause )
  {
  case SCECdFuncRead    :
    msg = NVL_MSG_CDVD_READ;
    break;
  case SCECdFuncSeek    :
    msg = NVL_MSG_CDVD_SEEK;
    break;
  case SCECdFuncStandby :
    msg = NVL_MSG_CDVD_STANDBY;
    break;
  case SCECdFuncStop    :
    msg = NVL_MSG_CDVD_STOP;
    break;
  case SCECdFuncPause   :
    msg = NVL_MSG_CDVD_PAUSE;
    break;
  case SCECdFuncBreak   :
    msg = NVL_MSG_CDVD_BREAK;
    break;
  default               :
    assert( 0 && "Crappy parameter." );
    break;
  }
  if( nvlCurrentIOStream )
  {
    nvlSendMsg( &nvlStreamSystemData.cdvdMsgQueue, msg, (nvlStream*)nvlCurrentIOStream );
    nvlCurrentIOStream = NULL;
  }
  else if( nvlStreamSystemData.prevCDVDCallback )
  {
    (*nvlStreamSystemData.prevCDVDCallback)( cause );
  }
}

void nvlStreamSystemSetThreadHandleCallback( nvlThreadHandleCallback c )
{
  threadControlFunc = c;
}

void nvlStreamSetBitRate( nvlStream* s, int bitrate )
{
  assert( s );
  s->bitrate = bitrate;
}

void nvlStreamSystemInit( int initCDVD )
{
  int               res;
  struct SemaParam  semaParam;

  if( initCDVD && !nvlStreamSystemLibsInitialized )
  {
    res = sceCdInit( SCECdINIT );
    assert( res );
    res = sceCdMmode( SCECdDVD );
    assert( res );
    sceCdDiskReady( 0 );
    nvlStreamSystemLibsInitialized = 1;
  }
  if( !nvlStreamSystemInitialized )
  {
    memset( &nvlStreamSystemData, 0, sizeof(nvlStreamSystemData) );

    for( int i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      nvlStreamSystemData.streamList[i] = i;
    }

    nvlInitMsgQueue( &nvlStreamSystemData.threadMsgQueue, nvlStreamSystemData.msg, NVLSTREAM_MSG_NUM );
    nvlInitMsgQueue( &nvlStreamSystemData.cdvdMsgQueue, nvlStreamSystemData.cdvdMsg, CDVD_MSG_NUM );

    semaParam.initCount = 1;
    semaParam.maxCount = 1;
    nvlStreamSystemData.vblankSema = CreateSema( &semaParam );
    assert( nvlStreamSystemData.vblankSema >= 0 );

    nvlStreamSystemData.vblankHandlerID = AddIntcHandler( INTC_VBLANK_S, nvlVBlankHandler, -1 );
    assert( nvlStreamSystemData.vblankHandlerID >= 0 );
    nvlStreamSystemData.VBWasEnabled = EnableIntc( INTC_VBLANK_S ) == 0;

    res = sceCdInitEeCB( NVL_STREAM_SYSTEM_CALLBACK_THREAD_PRIORITY, nvlCallbackStack, CALLBACK_THREAD_STACK_SIZE );
    assert( res >= 0 );
    nvlStreamSystemData.prevCDVDCallback = (callbackType)sceCdCallback( nvlStreamSystemCallback );

    if( nvlStreamAllocFunc )
    {
      nvlStreamSystemData.threadStack = (char*)(*nvlStreamAllocFunc)( 16, NVLSTREAM_THREAD_STACK_SIZE );
    }
    else
    {
      nvlStreamSystemData.threadStack = (char*)memalign( 16, NVLSTREAM_THREAD_STACK_SIZE );
    }
    assert( nvlStreamSystemData.threadStack );

    nvlStreamSystemData.mainThreadID = GetThreadId(); // assuming that this func. is called from the main thread
    nvlStreamSystemData.threadParam.entry = nvlStreamSystemThread;
    nvlStreamSystemData.threadParam.stack = nvlStreamSystemData.threadStack;
    nvlStreamSystemData.threadParam.stackSize = NVLSTREAM_THREAD_STACK_SIZE;
    nvlStreamSystemData.threadParam.gpReg = &_gp;
    nvlStreamSystemData.threadParam.initPriority = NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY;
    nvlStreamSystemData.threadID = CreateThread( &nvlStreamSystemData.threadParam );
    assert( nvlStreamSystemData.threadID >= 0 );

    nvlStreamSystemThreadIsRunning = 1;

    res = StartThread( nvlStreamSystemData.threadID, (void*)nvlStreamSystemData.threadParam.option );
    assert( res == nvlStreamSystemData.threadID );

    nvlStreamSystemInitialized = 1;
  }
}

void nvlStreamSystemShutdown( int exitCDVD )
{
  if( nvlStreamSystemInitialized )
  {
    for( int i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      nvlStream *s = nvlStreamSystemData.pStream[i];

      if( s )
      {
        nvlStreamDestroy( s );
      }
    }

    nvlSendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_SHUTDOWN, NULL );
    for( int i = 0; nvlStreamSystemThreadIsRunning; i++ )
    {
      assert( i < NVL_STREAM_MAX_TRIES );
      nvlWaitForVB();
    }
    if( nvlStreamFreeFunc )
    {
      (*nvlStreamFreeFunc)( nvlStreamSystemData.threadStack );
    }
    else
    {
      free( nvlStreamSystemData.threadStack );
    }

    sceCdCallback( nvlStreamSystemData.prevCDVDCallback );

    DisableIntc( INTC_VBLANK_S );
    RemoveIntcHandler( INTC_VBLANK_S, nvlStreamSystemData.vblankHandlerID );
    if( nvlStreamSystemData.VBWasEnabled )
    {
      EnableIntc( INTC_VBLANK_S );
    }

    nvlDestroyMsgQueue( &nvlStreamSystemData.threadMsgQueue );
    nvlDestroyMsgQueue( &nvlStreamSystemData.cdvdMsgQueue );

    nvlStreamSystemInitialized = 0;
  }
  if( exitCDVD && nvlStreamSystemLibsInitialized )
  {
    sceCdInit( SCECdEXIT );
    nvlStreamSystemLibsInitialized = 0;
  }
}

void nvlStreamSystemReset()
{
  nvlStreamSystemShutdown();
  nvlStreamSystemInit();
}

nvlStream *nvlStreamCreate( const char* filename, int buffer_size, void *buf, int loop_skip, int rewind_required )
{
  nvlStream   *rv = NULL;
  sceCdlFILE  fp;
  int         i;

  assert( nvlStreamSystemInitialized );
  assert( filename );
  assert( buffer_size == 0 || buffer_size >= NVL_STREAM_MIN_BUF_SIZE );

#ifdef NVL_STREAM_HOST_IO_SUPPORT
  char        hostStr[] = "HOST";
  const char  *sp, *hp;
  int         hostFile = 1;

  for( sp = filename, hp = hostStr; *sp && *hp; sp++, hp++ )
  {
    if( toupper(*sp) != *hp )
    {
      hostFile = 0;
      break;
    }
  }
  if( hostFile && ((sp[0] == ':') || ((sp[0] == '0' || sp[0] == '1') && sp[1] == ':')) )
  {
    // The source file is located on the host
    int res = sceOpen( filename, SCE_RDONLY );

    if( res >= 0 )
    {
      if( nvlStreamAllocFunc )
      {
        rv = (nvlStream*)(*nvlStreamAllocFunc)( 1, sizeof(nvlStream) );
      }
      else
      {
        rv = (nvlStream*)malloc( sizeof(nvlStream) );
      }
      assert( rv );
      memset( rv, 0, sizeof(nvlStream) );

      nvlInitMutex( &rv->mtx );
      i = nvlLockMutex( &rv->mtx, NVL_MUTEX_NOBLOCK );
      assert( i );
      rv->fh = res;
      rv->flags |= NVL_STREAM_FLAG_HOST_FILE;
    }
  }
  else
#endif
  if( sceCdSearchFile(&fp, filename) )
  {
    if( nvlStreamAllocFunc )
    {
      rv = (nvlStream*)(*nvlStreamAllocFunc)( 1, sizeof(nvlStream) );
    }
    else
    {
      rv = (nvlStream*)malloc( sizeof(nvlStream) );
    }
    assert( rv );
    memset( rv, 0, sizeof(nvlStream) );

    nvlInitMutex( &rv->mtx );
    i = nvlLockMutex( &rv->mtx, NVL_MUTEX_NOBLOCK );
    assert( i );

    rv->readSector = rv->startSector = fp.lsn;
    rv->filesize = fp.size;
    rv->endSector = rv->startSector + rv->filesize / DVD_SECTOR_SIZE;
    rv->bytes_in_last_sect = rv->filesize % DVD_SECTOR_SIZE;
    if( rv->bytes_in_last_sect == 0 )
    {
      rv->endSector--;
    }
  }

  if( rv )
  {
    if( buffer_size == 0 )
    {
      assert( buf == NULL );
      rv->bufsize = NVL_STREAM_MIN_BUF_SIZE;
    }
    else
    {
      rv->bufsize = buffer_size;
    }
    rv->bufsize &= ~( DVD_SECTOR_SIZE - 1 );
    if( buf == NULL )
    {
      rv->private_buffer = 1;
      if( nvlStreamAllocFunc )
      {
        rv->buf = (char*)(*nvlStreamAllocFunc)( 64, rv->bufsize );
      }
      else
      {
        rv->buf = (char*)memalign( 64, rv->bufsize );
      }
      assert( rv->buf );
    }
    else
    {
      rv->buf = (char*)buf;
    }
    rv->pWrite = rv->buf;
    rv->endbuf = rv->buf + rv->bufsize;

    for( i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      if( nvlStreamSystemData.pStream[i] == NULL )
      {
        break;
      }
    }
    assert( i < NVL_MAX_STREAM_NUM );
    rv->idx = i;
    rv->lp_skip = loop_skip;
    rv->requireRewind = rewind_required;
    nvlUnlockMutex( &rv->mtx );

//    nvlStreamSystemData.pStream[i] = rv;
//    nvlSendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_READ, rv );

#ifdef NVL_STREAM_HOST_IO_SUPPORT
    if( rv->flags & NVL_STREAM_FLAG_HOST_FILE )
    {
      nvlReadHost( rv );
    }
    else
#endif
    nvlReadDVD( rv );
    nvlStreamSystemData.pStream[i] = rv;
  }
  return rv;
}

void nvlStreamDestroy( nvlStream* s )
{
  int   i;

  assert( nvlStreamSystemInitialized );
  assert( s );

  i = nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
  assert( i );

  for( i = 0; s->flags & NVL_STREAM_FLAG_READ_IN_PROGRESS; i++ )
  {
    assert( i < NVL_STREAM_MAX_TRIES );
    nvlUnlockMutex( &s->mtx );
    nvlWaitForVB();
    nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
  }

  assert( nvlStreamSystemData.pStream[s->idx] == s );
  nvlStreamSystemData.pStream[s->idx] = NULL;

  nvlDestroyMutex( &s->mtx );

#ifdef NVL_STREAM_HOST_IO_SUPPORT
  if( s->flags & NVL_STREAM_FLAG_HOST_FILE )
  {
    sceClose( s->fh );
  }
#endif

  if( s->private_buffer )
  {
    assert( s->buf );
    if( nvlStreamFreeFunc )
    {
      (*nvlStreamFreeFunc)( s->buf );
    }
    else
    {
      free( s->buf );
    }
  }
  if( nvlStreamFreeFunc )
  {
    (*nvlStreamFreeFunc)( s );
  }
  else
  {
    free( s );
  }
}

void nvlStreamUnlock( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( s );
  nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
  s->pLock = NULL;
  nvlUnlockMutex( &s->mtx );
}

// Must be called from an interrupt handler only.
int nvlIStreamUnlock( nvlStream* s )
{
  int rv = 1;
#ifdef DEBUG
  if( !nvlStreamSystemInitialized || !s )
  {
    asm( "break" );
  }
#endif
  if( nvlILockMutex(&s->mtx) )
  {
    s->pLock = NULL;
    nvlIUnlockMutex( &s->mtx );
  }
  else
  {
    nvlISendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_UNLOCK, s );
    rv = 0;
  }
  return rv;
}

int nvlStreamSeek( nvlStream*, int offset, int origin )
{
  assert( 0 && "Not implemented." );
  return 0;
}

void nvlStreamSetLoopSkip( nvlStream* s, int loop_skip, int rewind_required )
{
#ifdef DEBUG
  if( !nvlStreamSystemInitialized || !s )
  {
    asm( "break" );
  }
  if( s->pEndOfData || s->pStartOfData )
  {
    // It's too late to change those values
    asm( "break" );
  }
#endif
  if( loop_skip >= 0 )
  {
    s->flags |= NVL_STREAM_FLAG_LOOPING;
  }
  s->lp_skip = loop_skip;
  s->requireRewind = rewind_required;
}

int nvlStreamEOS( nvlStream* s )
{
#ifdef DEBUG
  if( !nvlStreamSystemInitialized || s == NULL )
  {
    asm( "break" );
  }
#endif
  if( s->pEndOfData )
  {
    return s->pRead == s->pEndOfData;
  }
  return ( s->flags & NVL_STREAM_FLAG_AT_EOF ) && s->available == 0;
}

int nvlStreamRewind( nvlStream* s  )
{
  assert( nvlStreamSystemInitialized && s && s->pStartOfData );
  if( (s->flags & NVL_STREAM_FLAG_LOOPING) && s->pStartOfData )
  {
    int   delta;

    assert( s->pEndOfData );
    nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
    if( s->pEndOfData >= s->pRead )
    {
      delta = s->pEndOfData - s->pRead;
    }
    else
    {
      delta = s->endbuf - s->pRead + s->pEndOfData - s->buf;
    }
    assert( delta <= s->available );
    s->available -= delta;
    s->pLock = NULL;
    s->pRead = s->pStartOfData;
    s->pEndOfData = s->pStartOfData = NULL;
    nvlUnlockMutex( &s->mtx );
    if( !(s->flags & NVL_STREAM_FLAG_HOST_FILE) && s->lp_skip > 0 )
    {
      delta = s->lp_skip;
      nvlStreamGet( s, &delta, 0 );
      assert( delta == s->lp_skip );
    }
    return 1;
  }
  return 0;
}

int nvlIStreamRewind( nvlStream* s  )
{
  int rv = 0;
#ifdef DEBUG
  if( !nvlStreamSystemInitialized || s == NULL || s->pEndOfData == NULL )
  {
    asm( "break" );
  }
#endif
  if( (s->flags & NVL_STREAM_FLAG_LOOPING) && s->pStartOfData )
  {
    int   delta;

#ifdef DEBUG
    if( s->pEndOfData == NULL )
    {
      asm( "break" );
    }
#endif
    if( nvlILockMutex(&s->mtx) )
    {
      if( s->pEndOfData >= s->pRead )
      {
        delta = s->pEndOfData - s->pRead;
      }
      else
      {
        delta = s->endbuf - s->pRead + s->pEndOfData - s->buf;
      }
#ifdef DEBUG
      if( delta > s->available )
      {
        asm( "break;" );
      }
#endif
      s->available -= delta;
      s->pLock = NULL;
      s->pRead = s->pStartOfData;
      s->pEndOfData = s->pStartOfData = NULL;
      nvlIUnlockMutex( &s->mtx );
      if( s->lp_skip > 0 )
      {
        delta = s->lp_skip;
        nvlIStreamGet( s, &delta, 0 );
#ifdef DEBUG
        if( delta != s->lp_skip )
        {
          asm( "break;" );
        }
#endif
      }
      rv = 1;
    }
    else
    {
      nvlISendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_REWIND, s );
      rv = -1;
    }
  }
  return rv;
}

static inline int nvlIsAtFrameDelimiter( void* pData )
{
  register char *p = (char*)pData - 4, i, j;
  char          c[4] __attribute__((aligned (4)));

  for( i = 0; i < 8; i++ )
  {
    for( j = 0; j < 4; j++ )
    {
      c[j] = p[i + j];
    }
    if( *(unsigned int*)c == 0xB0010000 )
    {
      return 1;
    }
  }
  return 0;
}

void *nvlStreamGet( nvlStream* s, int *bytes, int lock )
{
  void    *rv = NULL;

  assert( nvlStreamSystemInitialized && s );
  nvlLockMutex( &s->mtx, NVL_MUTEX_BLOCK );
  if( s->pRead && s->available )
  {
    int   bytes_avalable = s->available;

    if( s->pEndOfData && s->pEndOfData >= s->pRead )
    {
      bytes_avalable = s->pEndOfData - s->pRead;
    }
    else if( s->pWrite <= s->pRead )
    {
      bytes_avalable = s->endbuf - s->pRead;
    }
    if( *bytes > bytes_avalable )
    {
      *bytes = bytes_avalable;
    }
    rv = s->pRead;
    s->pRead += *bytes;
    s->available -= *bytes;
    if( s->pRead == s->endbuf )
    {
      s->pRead = s->buf;
    }
    else if( s->pRead == s->pEndOfData && (s->flags & NVL_STREAM_FLAG_LOOPING) && !s->requireRewind )
    {
      assert( s->pStartOfData );
      s->pRead = s->pStartOfData;
      s->pEndOfData = s->pStartOfData = NULL;
      if( s->lp_skip > 0 )
      {
        assert( s->available >= s->lp_skip );
        assert( !(s->pWrite >= s->pRead && s->lp_skip > s->pWrite - s->pRead) );
        assert( !(s->pWrite < s->pRead && s->lp_skip > s->endbuf - s->pRead) );
        s->pRead += s->lp_skip;
      }
    }
    if( lock )
    {
      s->pLock = (char*)rv;
    }
    else
    {
      s->pLock = NULL;
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    assert( !nvlIsAtFrameDelimiter(rv) );
    assert( !nvlIsAtFrameDelimiter((char*)rv + *bytes) );
  }
  nvlUnlockMutex( &s->mtx );
  if( !rv )
  {
    *bytes = 0;
  }
  return rv;
}

void *nvlIStreamGet( nvlStream* s, int *bytes, int lock )
{
  void    *rv = NULL;

#ifdef DEBUG
  if( nvlStreamSystemInitialized == 0 || s == NULL )
  {
    asm( "break" );
  }
#endif
  if( nvlILockMutex(&s->mtx) )
  {
    if( s->pRead && s->available )
    {
      int   bytes_avalable = s->available;

      if( s->pEndOfData && s->pEndOfData >= s->pRead )
      {
        bytes_avalable = s->pEndOfData - s->pRead;
      }
      else if( s->pWrite <= s->pRead )
      {
        bytes_avalable = s->endbuf - s->pRead;
      }
      if( *bytes > bytes_avalable )
      {
        *bytes = bytes_avalable;
      }
      rv = s->pRead;
      s->pRead += *bytes;
      s->available -= *bytes;
      if( s->pRead == s->endbuf )
      {
        s->pRead = s->buf;
      }
      else if( s->pRead == s->pEndOfData && (s->flags & NVL_STREAM_FLAG_LOOPING) && !s->requireRewind )
      {
#ifdef DEBUG
        if( !s->pStartOfData )
        {
          asm( "break" );
        }
#endif
        s->pRead = s->pStartOfData;
        s->pEndOfData = s->pStartOfData = NULL;
        if( s->lp_skip > 0 )
        {
#ifdef DEBUG
          if(
              s->available < s->lp_skip ||
              (s->pWrite >= s->pRead && s->lp_skip > s->pWrite - s->pRead) ||
              (s->pWrite < s->pRead && s->lp_skip > s->endbuf - s->pRead)
            )
          {
            asm( "break" );
          }
#endif
          s->pRead += s->lp_skip;
        }
      }
      if( lock )
      {
        s->pLock = (char*)rv;
      }
      else
      {
        s->pLock = NULL;
      }
#ifdef DEBUG
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      if( nvlIsAtFrameDelimiter(rv) || nvlIsAtFrameDelimiter((char*)rv + *bytes) )
      {
        asm( "break" );
      }
#endif
    }
    nvlIUnlockMutex( &s->mtx );
  }
  if( !rv )
  {
    *bytes = 0;
  }
  return rv;
}

int nvlStreamAvailable( nvlStream* s )
{
  int rv;

#ifdef DEBUG
  if( nvlStreamSystemInitialized == 0 || s == NULL )
  {
    asm( "break" );
  }
#endif
  if( s->pEndOfData == NULL )
  {
    rv = s->available;
  }
  else
  {
    if( s->pEndOfData && s->pEndOfData >= s->pRead )
    {
      rv = s->pEndOfData - s->pRead;
    }
    else
    {
      rv = s->endbuf - s->pRead + s->pEndOfData - s->buf;
    }
  }
//#ifdef DEBUG
//  if( rv > s->available )
//  {
//    asm( "break" );
//  }
//#endif
  return rv;
}

int nvlStreamReqSize( nvlStream* s )
{
#ifdef DEBUG
  if( nvlStreamSystemInitialized == 0 || s == NULL )
  {
    asm( "break" );
  }
#endif
  return s->bufsize >> 2; // !!! for now, can be modified later
}

void nvlStreamDestroyAsync( nvlStream* s )
{
  assert( nvlStreamSystemInitialized && s );
  nvlSendMsg( &nvlStreamSystemData.threadMsgQueue, NVL_MSG_DESTROY, s );
}

void nvlStreamSetMemoryAllocCallback( void* (*func)(int alighnment, int size) )
{
  nvlStreamAllocFunc = func;
}

void nvlStreamSetMemoryFreeCallback( void (*func)(void*) )
{
  nvlStreamFreeFunc = func;
}

//------------------------------------------------------------------------------------------------------------------//
// Mutex implementation.

void nvlInitMutex( nvlMutex* mtx )
{
  struct SemaParam  semaParam;

  assert( mtx );
  semaParam.initCount = 1;
  semaParam.maxCount = 1;
  *mtx = CreateSema( &semaParam );
  assert( *mtx >= 0 );
}

void nvlDestroyMutex( nvlMutex* mtx )
{
  assert( mtx );
#ifdef DEBUG
  int res =
#endif
  DeleteSema( *mtx );
#ifdef DEBUG
  assert( res == *mtx );
#endif
}

int nvlLockMutex( nvlMutex* mtx, nvlMutexMode mode )
{
  int res;

  assert( mtx );
  if( mode == NVL_MUTEX_BLOCK )
  {
    res = WaitSema( *mtx );
  }
  else
  {
    res = PollSema( *mtx );
    if( res != *mtx )
    {
      return 0;
    }
  }
  assert( res == *mtx );
  return 1;
}

void nvlUnlockMutex( nvlMutex* mtx )
{
  assert( mtx );
#ifdef DEBUG
  int res =
#endif
  SignalSema( *mtx );
#ifdef DEBUG
  assert( res == *mtx );
#endif
}

int nvlILockMutex( nvlMutex* mtx )
{
  int res;

#ifdef DEBUG
  if( !mtx )
  {
    asm( "break" );
  }
#endif
  res = iPollSema( *mtx );
  if( res != *mtx )
  {
    return 0;
  }
  return 1;
}

void nvlIUnlockMutex( nvlMutex* mtx )
{
#ifdef DEBUG
  if( !mtx )
  {
    asm( "break" );
  }
  int res =
#endif
  iSignalSema( *mtx );
#ifdef DEBUG
  assert( res == *mtx );
#endif
}
