#include "osassert.h"
#include <string.h>
#include <dolphin.h>

#include "nvlstream_gc.h"

#define NVL_STREAM_QUEUE_SIZE       16
#define NVL_STREAM_STACK_SIZE       4096
#define NVL_STREAM_READ_BUFFER_NUM  3
#define NVL_STREAM_BUFFER_SIZE      (512*1024*NVL_STREAM_READ_BUFFER_NUM)

#ifndef NGL
extern void*  nglMemAlloc( unsigned int Size, unsigned int Align = 1 );
extern void   nglMemFree( void* Ptr );
#endif

enum nvlStreamMessages
{
  NVL_STREAM_MSG_INVALID  = 0,
  NVL_STREAM_MSG_SHUTDOWN,
  NVL_STREAM_MSG_READ_START,
  NVL_STREAM_MSG_READ_TOP,
//  NVL_STREAM_MSG_READ_WAIT,
  NVL_STREAM_MSG_READ_BRANK,
  NVL_STREAM_MSG_READ_END,
  NVL_STREAM_MSG_READ_CLOSE,
  NVL_STREAM_MSG_ERROR,
  NVL_STREAM_MSG_END
};

enum
{
  NVL_STREAM_INVALID            = 0,
  NVL_STREAM_FLAG_READY         = 0x00000001,
  NVL_STREAM_FLAG_RESET         = 0x00000002,
  NVL_STREAM_FLAG_CLOSE         = 0x00000004,
  NVL_STREAM_FLAG_WAITING_BRANK = 0x00000008,
  NVL_STREAM_FLAG_LOOPING       = 0x00000010,
  NVL_STREAM_FLAG_ERROR         = 0x00000020,
  NVL_STREAM_FLAG_REWINDING     = 0x00000040
};

enum nvlBufStatus
{
  NVL_BUF_STAT_BRANK    = 0,
  NVL_BUF_STAT_READING,
  NVL_BUF_STAT_READED,
  NVL_BUF_STAT_USE
};

typedef struct
{
  void          *p;
  s32           size;
  int           start;
  int           end;
  nvlBufStatus  stat;
} nvlReadDesc;

struct nvlStream
{
  char                    *filename;
  DVDFileInfo             file_info;
  s32                     write_offset;
  s32                     read_offset;
  s32                     read_length;
  volatile unsigned int   flags;
  unsigned char           *buf;
  s32                     buf_size;
  int                     private_buffer;
  OSMutex                 *mutex;
  nvlReadDesc             read_desc[NVL_STREAM_READ_BUFFER_NUM];
  int                     idx;
  volatile int            readingDVD;
};

static struct nvlStreamSystemData_t
{
  OSMessageQueue    msgQueue;
  OSMessage         msg[NVL_STREAM_QUEUE_SIZE];
  OSMessageQueue    errQueue;
  OSMessage         errMsg[NVL_STREAM_QUEUE_SIZE];
  OSThread          thread;
  unsigned char     *pStack;
  OSMutex           mutex[NVL_MAX_STREAM_NUM];
  int               mutex_in_use[NVL_MAX_STREAM_NUM];
  nvlStream         *pStream[NVL_MAX_STREAM_NUM];
  int               priority_stream;
}                     nvlStreamSystemData;
static volatile int   nvlStreamSystemRunning = 0, nvlStreamSystemInitialized = 0;
static int            nvlStreamMutexInitialized = 0;

static inline void *nvlStreamPackMsg( int stream_idx, int msg )
{
  assert(
          (stream_idx == -1 || (stream_idx >= 0 && stream_idx < NVL_MAX_STREAM_NUM)) &&
          msg > NVL_STREAM_MSG_INVALID && msg < NVL_STREAM_MSG_END
        );
  return (void*)(((unsigned int)stream_idx << 16) | msg);
}

static inline void nvlStreamUnpackMsg( void* stream_idx, void* msg, int packed_msg )
{
  assert( stream_idx && msg );
  *((int*)stream_idx) = packed_msg >> 16;
  *((int*)msg) = packed_msg & 0xffff;
  assert(
          (*(int*)stream_idx == -1 || (*(int*)stream_idx >= 0 && *(int*)stream_idx < NVL_MAX_STREAM_NUM)) &&
          *(int*)msg > NVL_STREAM_MSG_INVALID && *(int*)msg < NVL_STREAM_MSG_END
        );
}

static nvlReadDesc* nvlStreamGetDescByStat( nvlStream* s, nvlBufStatus bstat )
{
  nvlReadDesc   *rv = NULL;

  assert( s );
  for( int i = 0; i < NVL_STREAM_READ_BUFFER_NUM; i++ )
  {
    if( s->read_desc[i].stat == bstat )
    {
      rv = &s->read_desc[i];
      break;
    }
  }
  return rv;
}

static int nvlStreamNext()
{
  static int  last = -1;
  int         rv = nvlStreamSystemData.priority_stream, i;

  assert( nvlStreamSystemInitialized );
  if( rv < 0 )
  {
    for( i = (last >= 0? last + 1 : 0);; ++i >= NVL_MAX_STREAM_NUM? (i = 0) : i )
    {
      nvlStream *s = nvlStreamSystemData.pStream[i];

      if( s )
      {
//        OSLockMutex( s->mutex );
        if(
            s->flags != NVL_STREAM_INVALID &&
            (s->flags & (NVL_STREAM_FLAG_RESET | NVL_STREAM_FLAG_CLOSE | NVL_STREAM_FLAG_WAITING_BRANK)) == 0
          )
        {
//          OSUnlockMutex( s->mutex );
          rv = i;
          break;
        }
//        OSUnlockMutex( s->mutex );
      }
      if( i == last )
      {
        break;
      }
    }
  }
  else
  {
    nvlStreamSystemData.priority_stream = -1;
  }
  return (last = rv);
/*

  nvlStream *s = nvlStreamSystemData.pStream[0];

  if( s )
  {
    OSLockMutex( s->mutex );
    if(
        s->flags != NVL_STREAM_INVALID &&
        (s->flags & (NVL_STREAM_FLAG_RESET | NVL_STREAM_FLAG_CLOSE | NVL_STREAM_FLAG_WAITING_BRANK)) == 0
      )
    {
      OSUnlockMutex( s->mutex );
      return 0;
    }
    OSUnlockMutex( s->mutex );
  }
return -1;
*/
}

static void nvlStreamCallback( s32 result, DVDFileInfo* file_info )
{
  int   i, msg;

  assert( nvlStreamSystemInitialized );
  assert( file_info );
  for( i = 0; i < NVL_MAX_STREAM_NUM; i++ )
  {
    if( nvlStreamSystemData.pStream[i] && &nvlStreamSystemData.pStream[i]->file_info == file_info )
    {
      break;
    }
  }
  assert( i < NVL_MAX_STREAM_NUM );

  if( result == -1 )
  {
    msg = NVL_STREAM_MSG_ERROR;
  }
  else
  {
    msg = NVL_STREAM_MSG_READ_END;
  }
  OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(i, msg), OS_MESSAGE_NOBLOCK );
}

static void nvlStreamReset( nvlStream* s )
{

  s->write_offset = 0;
  s->read_length = 0;
  s->flags &= ~( NVL_STREAM_FLAG_RESET | NVL_STREAM_FLAG_WAITING_BRANK );
  for( int i = 0; i < NVL_STREAM_READ_BUFFER_NUM; i++ )
  {
    s->read_desc[i].start = 0;
    s->read_desc[i].end = 0;
    s->read_desc[i].start = NVL_BUF_STAT_BRANK;
  }
}

static OSMutex *nvlStreamGetMutex()
{
  OSMutex   *rv = NULL;
  
  assert( nvlStreamSystemInitialized );
  for( int i = 0; i < NVL_MAX_STREAM_NUM; i++ )
  {
    if( !nvlStreamSystemData.mutex_in_use[i] )
    {
      rv = &nvlStreamSystemData.mutex[i];
      nvlStreamSystemData.mutex_in_use[i] = 1;
      break;
    }
  }
#ifdef DEBUG
  BOOL  res = OSTryLockMutex( rv );
  assert( res );
  OSUnlockMutex( rv );
#endif
  return rv;
}

static void nvlStreamReleaseMutex( OSMutex* mtx )
{
  int   i;

  assert( nvlStreamSystemInitialized );
  assert( mtx );

  BOOL  res = OSTryLockMutex( mtx );
  
  assert( res );
  for( i = 0; i < NVL_MAX_STREAM_NUM; i++ )
  {
    if( &nvlStreamSystemData.mutex[i] == mtx )
    {
      break;
    }
  }
  assert( i < NVL_MAX_STREAM_NUM );
  OSUnlockMutex( mtx );
  nvlStreamSystemData.mutex_in_use[i] = 0;
}

static void nvlStreamDestroyInternal( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( nvlStreamSystemData.pStream[s->idx] == s );
  OSLockMutex( s->mutex );
  DVDClose( &s->file_info );
  s->flags = NVL_STREAM_INVALID;
  nvlStreamSystemData.pStream[s->idx] = NULL;
  OSUnlockMutex( s->mutex );
  nvlStreamReleaseMutex( s->mutex );
  if( s->private_buffer )
  {
    nglMemFree( s->buf );
  }
  nglMemFree( s->filename );
  nglMemFree( s );
}

static void *nvlStreamSystemThreadFunc( void* )
{
  int         idx, msg;
  OSMessage   packed_msg;
  nvlStream   *s;
  BOOL        res;
  nvlReadDesc *pDesc;

  assert( nvlStreamSystemInitialized );
  nvlStreamSystemRunning = 1;

  for(;;)
  {
    OSReceiveMessage( &nvlStreamSystemData.msgQueue, &packed_msg, OS_MESSAGE_BLOCK );
    nvlStreamUnpackMsg( &idx, &msg, (int)packed_msg );
    assert( idx < NVL_MAX_STREAM_NUM );
    if( idx < 0 )
    {
      continue;
    }
    s = nvlStreamSystemData.pStream[idx];
    switch( msg )
    {
    case NVL_STREAM_MSG_READ_START  :
      assert( s );
      OSLockMutex( s->mutex );
      if( s->readingDVD )
      {
//        OSReceiveMessage( &nvlStreamSystemData.msgQueue, &packed_msg, OS_MESSAGE_BLOCK );
//        OSSendMessage( &nvlStreamSystemData.msgQueue, packed_msg, OS_MESSAGE_NOBLOCK );
        OSUnlockMutex( s->mutex );
        break;
      }
      s->flags &= ~NVL_STREAM_FLAG_REWINDING;
      pDesc = nvlStreamGetDescByStat( s, NVL_BUF_STAT_BRANK );
      if( pDesc )
      {
//        s->flags &= ~NVL_STREAM_FLAG_WAITING_BRANK;
        s->read_length = ((int)DVDGetLength(&s->file_info) - s->write_offset < pDesc->size)? (int)DVDGetLength(&s->file_info) - s->write_offset : pDesc->size;
        if( s->read_length <= 0 )
        {
          // we are beyond the end of the file
          OSUnlockMutex( s->mutex );
          break;
        }
        s->read_length = (s32)OSRoundUp32B( s->read_length );
        s->readingDVD = 1;
        res = DVDReadAsync(
                            &s->file_info,
                            pDesc->p,
                            s->read_length,
                            s->write_offset,
                            nvlStreamCallback
                          );
        assert( res );
        pDesc->start = s->write_offset;
        pDesc->stat = NVL_BUF_STAT_READING;
      }
      else
      {
        s->flags |= NVL_STREAM_FLAG_WAITING_BRANK;
//        OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(idx, NVL_STREAM_MSG_READ_WAIT), OS_MESSAGE_NOBLOCK );
      }
      OSUnlockMutex( s->mutex );
      break;
    case NVL_STREAM_MSG_READ_END    :
      assert( s );
      OSLockMutex( s->mutex );
      pDesc = nvlStreamGetDescByStat( s, NVL_BUF_STAT_READING );
      assert( pDesc );
      s->write_offset += s->read_length;
      pDesc->end = pDesc->start + s->read_length;
      pDesc->stat = NVL_BUF_STAT_READED;
      if( s->flags & NVL_STREAM_FLAG_RESET )
      {
        nvlStreamReset( s );
      }
      s->readingDVD = 0;
      OSUnlockMutex( s->mutex );
      if( s->flags & NVL_STREAM_FLAG_CLOSE )
      {
        nvlStreamDestroyInternal( s );
      }
      if( s->write_offset >= DVDGetLength(&s->file_info)/* && (s->flags & NVL_STREAM_FLAG_LOOPING)*/ )
      {
        s->write_offset = 0;
      }
      OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(nvlStreamNext(), NVL_STREAM_MSG_READ_START), OS_MESSAGE_NOBLOCK );
      break;
//    case NVL_STREAM_MSG_READ_WAIT   :
//      assert( s );
//      {
//        wait for brank?
//        break;
//      }
    case NVL_STREAM_MSG_READ_BRANK  :
      assert( s );
      if( s->flags & NVL_STREAM_FLAG_WAITING_BRANK )
      {
        s->flags &= ~NVL_STREAM_FLAG_WAITING_BRANK;
        OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(idx, NVL_STREAM_MSG_READ_START), OS_MESSAGE_NOBLOCK );
//        OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(nvlStreamNext(), NVL_STREAM_MSG_READ_START), OS_MESSAGE_NOBLOCK );
      }
      break;
    case NVL_STREAM_MSG_READ_TOP    :
      assert( s );
      OSLockMutex( s->mutex );
      pDesc = nvlStreamGetDescByStat( s, NVL_BUF_STAT_READING );
      if( !pDesc )
      {
        nvlStreamReset( s );
        OSUnlockMutex( s->mutex );
        OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(nvlStreamNext(), NVL_STREAM_MSG_READ_START), OS_MESSAGE_NOBLOCK );
        break;
      }
      else
      {
        s->flags |= NVL_STREAM_FLAG_RESET;
      }
      OSUnlockMutex( s->mutex );
      break;
    case NVL_STREAM_MSG_READ_CLOSE  :
      assert( s );
      OSLockMutex( s->mutex );
      pDesc = nvlStreamGetDescByStat( s, NVL_BUF_STAT_READING );
      if( pDesc )
      {
        s->flags |= NVL_STREAM_FLAG_CLOSE;
        OSUnlockMutex( s->mutex );
      }
      else
      {
        OSUnlockMutex( s->mutex );
        nvlStreamDestroyInternal( s );
      }
      break;
    case NVL_STREAM_MSG_ERROR       :
      assert( s );
      s->flags |= NVL_STREAM_FLAG_ERROR;
      break;
    case NVL_STREAM_MSG_SHUTDOWN    :
      goto terminate;
//      nvlStreamSystemRunning = 0;
//      OSExitThread( 0 );
//      break;
    default:
      assert( 0 && "Unknown message." );
      break;
    }
  }
terminate:
  nvlStreamSystemRunning = 0;
//  OSExitThread( 0 );
  return NULL;
}

void nvlStreamSystemInit( int )
{
  OSMessage   msg;

  if( !nvlStreamMutexInitialized )
  {
    for( int i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      OSInitMutex( &nvlStreamSystemData.mutex[i] );
    }
    nvlStreamMutexInitialized = 1;
  }
  if( !nvlStreamSystemInitialized )
  {
    for( int i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      if( nvlStreamSystemData.pStream[i] )
      {
        nvlStreamSystemData.pStream[i] = NULL;
        nvlStreamSystemData.mutex_in_use[i] = 0;
      }
    }
    nvlStreamSystemData.priority_stream = -1;
    nvlStreamSystemData.pStack = (unsigned char*)nglMemAlloc( NVL_STREAM_STACK_SIZE, 32 );
    assert( nvlStreamSystemData.pStack );
    OSInitMessageQueue( &nvlStreamSystemData.msgQueue, nvlStreamSystemData.msg, NVL_STREAM_QUEUE_SIZE );
    OSInitMessageQueue( &nvlStreamSystemData.errQueue, nvlStreamSystemData.errMsg, NVL_STREAM_QUEUE_SIZE );
    while( OSReceiveMessage(&nvlStreamSystemData.msgQueue, &msg, OS_MESSAGE_NOBLOCK) )
      ;
    while( OSReceiveMessage(&nvlStreamSystemData.errQueue, &msg, OS_MESSAGE_NOBLOCK) )
      ;
    OSCreateThread(
                    &nvlStreamSystemData.thread,
                    nvlStreamSystemThreadFunc,
                    NULL,
                    nvlStreamSystemData.pStack + NVL_STREAM_STACK_SIZE,
                    NVL_STREAM_STACK_SIZE,
                    NVL_STREAM_SYSTEM_THREAD_INIT_PRIORITY,
                    OS_THREAD_ATTR_DETACH
                  );
    nvlStreamSystemInitialized = 1;
    OSResumeThread( &nvlStreamSystemData.thread );
  }
}

void nvlStreamSystemShutdown()
{
  if( nvlStreamSystemInitialized )
  {
    for( int i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      if( nvlStreamSystemData.pStream[i] )
      {
        nvlStreamDestroyInternal( nvlStreamSystemData.pStream[i] );
      }
    }
    OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(0, NVL_STREAM_MSG_SHUTDOWN), OS_MESSAGE_NOBLOCK );
    while( nvlStreamSystemRunning )
    {
      OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(0, NVL_STREAM_MSG_SHUTDOWN), OS_MESSAGE_NOBLOCK );
      VIWaitForRetrace();
    }
    nglMemFree( nvlStreamSystemData.pStack );
    nvlStreamSystemInitialized = 0;
  }
}

void nvlStreamSystemReset()
{
  nvlStreamSystemShutdown();
  nvlStreamSystemInit();
}

nvlStream* nvlStreamCreate( char* filename, int buffer_size, void *buf, int looping )
{
  int         i;
  nvlStream   *rv = (nvlStream*)nglMemAlloc( sizeof(nvlStream) );

  assert( nvlStreamSystemInitialized );
  assert( rv );
  assert( filename );
  memset( rv, 0, sizeof(nvlStream) );
  if( DVDOpen(filename, &rv->file_info) == FALSE )
  {
    nglMemFree( rv );
    rv = NULL;
  }
  else
  {
    rv->filename = (char*)nglMemAlloc( strlen(filename) + 1 );
    assert( rv->filename );
    strcpy( rv->filename, filename );
    if( buffer_size == 0 )
    {
      assert( buf == NULL );
      rv->buf_size = NVL_STREAM_BUFFER_SIZE;
    }
    else
    {
      rv->buf_size = buffer_size;
    }
    if( buf )
    {
      rv->buf = (unsigned char*)buf;
    }
    else
    {
      rv->private_buffer = 1;
      rv->buf = (unsigned char*)nglMemAlloc( (u32)rv->buf_size, 32 );
    }
    memset( rv->read_desc, 0, sizeof(rv->read_desc) );
    for( i = 0; i < NVL_STREAM_READ_BUFFER_NUM; i++ )
    {
      rv->read_desc[i].size = rv->buf_size / NVL_STREAM_READ_BUFFER_NUM;
      rv->read_desc[i].p = &rv->buf[i * rv->read_desc[i].size];
    }
    if( looping )
    {
      rv->flags |= NVL_STREAM_FLAG_LOOPING;
    }
    rv->mutex = nvlStreamGetMutex();
    assert( rv->mutex );
    rv->flags |= NVL_STREAM_FLAG_READY;
    for( i = 0; i < NVL_MAX_STREAM_NUM; i++ )
    {
      if( nvlStreamSystemData.pStream[i] == NULL )
      {
        nvlStreamSystemData.pStream[i] = rv;
        rv->idx = i;
        break;
      }
    }
    assert( i < NVL_MAX_STREAM_NUM );
  }
  OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(i, NVL_STREAM_MSG_READ_START), OS_MESSAGE_NOBLOCK );
  return rv;
}

void nvlStreamDestroy( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( s && s->flags != NVL_STREAM_INVALID );
  OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(s->idx, NVL_STREAM_MSG_READ_CLOSE), OS_MESSAGE_NOBLOCK );
}

static int nvlStreamReadUse( nvlStream* s, void *addr, int bytes, int offset )
{
  nvlReadDesc   *pDesc;
  OSMessage     msg;
  int           i;

//  assert( nvlStreamSystemInitialized );
//  assert( s && s->flags != NVL_STREAM_INVALID );
  if( OSReceiveMessage(&nvlStreamSystemData.errQueue, &msg, OS_MESSAGE_NOBLOCK) )
  {
    int   idx, msgval;

    nvlStreamUnpackMsg( &idx, &msgval, (int)msg );
    if( idx == s->idx )
    {
      return -1;
    }
    else
    {
      OSSendMessage( &nvlStreamSystemData.errQueue, &msg, OS_MESSAGE_NOBLOCK );
    }
  }

  OSLockMutex( s->mutex );
  for( i = 0, pDesc = s->read_desc; i < NVL_STREAM_READ_BUFFER_NUM; i++, pDesc++ )
  {
    if( pDesc->stat == NVL_BUF_STAT_USE )
    {
      if( pDesc->start <= offset && offset < pDesc->end )
      {
        if( offset + bytes > pDesc->end )
        {
          if( pDesc->end - pDesc->start == NVL_STREAM_BUFFER_SIZE / NVL_STREAM_READ_BUFFER_NUM )
          {
            bytes = pDesc->end - offset;
          }
        }

        if( addr )
        {
          memcpy( addr, (u8*)pDesc->p + offset - pDesc->start, (size_t)bytes );
        }

        OSUnlockMutex( s->mutex );
        return bytes;
      }
    }
  }
  OSUnlockMutex( s->mutex );
  return 0;
}

static int nvlStreamReadReaded( nvlStream* s, void *addr, int bytes, int offset )
{
  nvlReadDesc *pDesc, *pDesc1;
  OSMessage   msg;
  int         i;

//  assert( nvlStreamSystemInitialized );
//  assert( s && s->flags != NVL_STREAM_INVALID );
  for(;;)
  {
    if( OSReceiveMessage(&nvlStreamSystemData.errQueue, &msg, OS_MESSAGE_NOBLOCK) )
    {
      int   idx, msgval;

      nvlStreamUnpackMsg( &idx, &msgval, (int)msg );
      if( idx == s->idx )
      {
        break;
      }
      else
      {
        OSSendMessage( &nvlStreamSystemData.errQueue, &msg, OS_MESSAGE_NOBLOCK );
      }
    }
    OSLockMutex( s->mutex );
    for( i = 0, pDesc = s->read_desc; i < NVL_STREAM_READ_BUFFER_NUM; i++, pDesc++ )
    {
      if( pDesc->stat == NVL_BUF_STAT_READED )
      {
        if( pDesc->start <= offset && offset < pDesc->end )
        {
          assert( offset + bytes <= pDesc->end );
          if( addr )
          {
            memcpy( addr, (u8*)pDesc->p + offset - pDesc->start, (size_t)bytes );
          }
          pDesc1 = nvlStreamGetDescByStat( s, NVL_BUF_STAT_USE );
          if( pDesc1 )
          {
            pDesc1->stat = NVL_BUF_STAT_BRANK;
            OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(s->idx, NVL_STREAM_MSG_READ_BRANK), OS_MESSAGE_NOBLOCK);
          }
          pDesc->stat = NVL_BUF_STAT_USE;
          OSUnlockMutex( s->mutex );
          return bytes;
	      }
      }
    }
    OSUnlockMutex( s->mutex );
  }
  return -1;
}

int nvlStreamGetCopy( nvlStream* s, void* dest, int bytes )
{
  s32 len;

  assert( nvlStreamSystemInitialized );
  if( s == NULL || s->flags == NVL_STREAM_INVALID )
  {
    return -1;
  }
  if( DVDGetLength(&s->file_info) < s->read_offset + bytes )
  {
    s->read_offset = 0;
  }

  if( (len = nvlStreamReadUse(s, dest, bytes, s->read_offset)) < 0 )
  {
    return -1;
  }
  if( len != bytes )
  {
    if( nvlStreamReadReaded(s, (dest == NULL? NULL : (u8*)dest + len), bytes-len, s->read_offset + len) < 0 )
    {
      return -1;
    }
  }
  s->read_offset += bytes;
  return bytes;
}

void nvlStreamUnlock( nvlStream* )
{
  assert( 0 && "Not implemented." );
}

int nvlStreamSeek( nvlStream* s, int offset, int origin )
{
#pragma unused(s,offset,origin)
  assert( 0 && "Not implemented." );
  return 0;
}

int nvlStreamEOS( nvlStream* )
{
  assert( 0 && "Not implemented." );
  return 0;
}

void nvlStreamRewind( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( s && s->flags != NVL_STREAM_INVALID );
  s->flags |= NVL_STREAM_FLAG_REWINDING;
  OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(s->idx, NVL_STREAM_MSG_READ_TOP), OS_MESSAGE_NOBLOCK );
}

int nvlStreamSystemIsRunning()
{
  return nvlStreamSystemRunning;
}

void nvlStreamSetError( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( s && s->flags != NVL_STREAM_INVALID );
  OSSendMessage( &nvlStreamSystemData.msgQueue, nvlStreamPackMsg(s->idx, NVL_STREAM_MSG_ERROR), OS_MESSAGE_NOBLOCK );
}

int nvlStreamGetErrorStatus( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( s && s->flags != NVL_STREAM_INVALID );
  if( s->flags & NVL_STREAM_FLAG_ERROR )
  {
    return DVD_STATE_FATAL_ERROR;
  }
  if( nvlStreamSystemRunning )
  {
    return DVDGetFileInfoStatus( &s->file_info );
  }
  return DVD_STATE_IGNORED;
}

int nvlStreamIsReady( nvlStream* s )
{
  assert( nvlStreamSystemInitialized );
  assert( s );
  if(
      s->flags == NVL_STREAM_INVALID ||
      !(s->flags & NVL_STREAM_FLAG_READY) ||
      (s->flags & (NVL_STREAM_FLAG_RESET | NVL_STREAM_FLAG_CLOSE | NVL_STREAM_FLAG_ERROR | NVL_STREAM_FLAG_REWINDING))
    )
  {
    return 0;
  }
  return 1;
}
