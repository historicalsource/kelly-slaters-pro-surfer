#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dolphin.h>

#include "nvlstream_gc.h"

#define BUFSIZE     (512*1024)
#define BUFNUM      (3)
#define READ_PRI    (8)
#define FLAG_CLOSE  (0x01)
#define FLAG_TOP    (0x02)

enum RST
{
  RstBRANK    =0,
  RstREADING,
  RstREADED,
  RstUSE
};

enum ReadMsg
{
  READ_START=0,
  READ_END,
  READ_WAIT,
  READ_BRANK,
  READ_TOP,
  READ_CLOSE,
  READ_ERR
};

typedef struct {
  void *pBuf;
  u32 Size;
  s32 Start;
  s32 End;
  RST Status;
}HVQM4Read;

extern void* nglMemAlloc( unsigned int Size, unsigned int Align = 1 );
extern void nglMemFree( void* Ptr );

int                   InitFlag = 0;

//static u8             ReadBuffer[BUFSIZE*BUFNUM] ATTRIBUTE_ALIGN(32);
static u8             *ReadBuffer;

static HVQM4Read      ReadObj[BUFNUM];
static s32            ReadOffset;
static s32            ReadFileSize;

static OSMessageQueue MessageQueue;
static OSMessage      MessageArray[16];

static OSMessageQueue ErrQueue;
static OSMessage      ErrArray[16];

static OSThread       Thread;
static u8             ThreadStack[4096];

static OSMutex        Mutex;
static TST            Status = TstCLOSED;
static u32            ReadErr = 0;

static DVDFileInfo    *pfileInfo = NULL;

void HVQM4CloseRead(void);
void HVQM4StopRead(void);
TST HVQM4GetStatus(void);
s32 HVQM4Read2Buf(void *addr,s32 length);
void HVQM4CreateReadThread(char *pStr);
s32 HVQM4ErrStatus(void);
void HVQM4SendErr(void);

/*---------------------------------------------------------------------------*
    Name:               callback
    Description:        callback function for DVDReadAsync.
    Arguments:          result      -1 if read fails, file size if succeeded.
                        fileInfo    fileInfo for the file transferred.
    Returns:            none
 *---------------------------------------------------------------------------*/

static void CallBack(s32 result,DVDFileInfo* fileInfo)
{
#pragma unused(fileInfo)

  if(result==-1){
    OSSendMessage(&MessageQueue,(void *)READ_ERR,OS_MESSAGE_NOBLOCK);
  }
  else{
    OSSendMessage(&MessageQueue,(void *)READ_END,OS_MESSAGE_NOBLOCK);
  }
}

/*---------------------------------------------------------------------------*
    Name:               GetStatus
    Description:        Searches buffer of specified status.
                        Lock Mutex before using.
    Arguments:          status     buffer status
    Returns:            Succeed    pointer to HVQM4Read structure 
                        Faild      NULL pointer
 *---------------------------------------------------------------------------*/

static HVQM4Read *GetStatus(RST Status)
{
  HVQM4Read *pRead;
  int i;

  for(i=0,pRead=ReadObj;i<BUFNUM;i++,pRead++){
    if(pRead->Status==Status){
      return pRead;
    }
  }

  return NULL;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4ReadThread
    Description:    Reads specified h4m data.
    Arguments:      param      data file name
    Returns:        Null pointer
 *---------------------------------------------------------------------------*/

static void *HVQM4ReadThread(void *param)
{
  HVQM4Read *pRead;
  DVDFileInfo fileInfo;
  OSMessage msg;
  s32 fileSize;
  s32 fileSize32B;
  s32 offset;
  s32 length;
  int flag;
  int i;

  if(FALSE==DVDOpen((char *)param,&fileInfo)){
    OSHalt("Cannot open file");
  }

  pfileInfo=&fileInfo;

  // Get the size of the file
  ReadFileSize=(s32)DVDGetLength(&fileInfo);
  ReadErr=0;

 restart:
  Status=TstNORMAL;

  // Initialize flag
  flag=0;

  // Initialize buffer
  OSLockMutex(&Mutex);
  for(i=0,pRead=ReadObj;i<BUFNUM;i++,pRead++){
    pRead->pBuf=&ReadBuffer[BUFSIZE*i];
    pRead->Size=BUFSIZE;
    pRead->Status=RstBRANK;
  }
  OSUnlockMutex(&Mutex);
 repeat:
  // Get the size of the file
  fileSize=ReadFileSize;
  fileSize32B=(s32)OSRoundUp32B(fileSize);

  // Sends starting message to thread
  OSSendMessage(&MessageQueue,(void *)READ_START,OS_MESSAGE_NOBLOCK);

  for(offset=0;;){
    OSReceiveMessage(&MessageQueue,&msg,OS_MESSAGE_BLOCK);
#ifdef _DEBUG
    OSReport("Msg=%d\n",(int)msg);
#endif
    switch((int)msg){
    case READ_START:
      OSLockMutex(&Mutex);
      
      pRead=GetStatus(RstBRANK);
      if(pRead){
	length=(fileSize32B<BUFSIZE?fileSize32B:BUFSIZE);
	if(FALSE==DVDReadAsync(&fileInfo,pRead->pBuf,length,offset,CallBack)){
	  OSHalt("Error occurred when issuing read");
	}
	pRead->Start=offset;
	pRead->Status=RstREADING;
      }
      else{
	OSSendMessage(&MessageQueue,(void *)READ_WAIT,OS_MESSAGE_NOBLOCK);
      }

      OSUnlockMutex(&Mutex);
      break;
    case READ_END:
      OSLockMutex(&Mutex);

      pRead=GetStatus(RstREADING);
      if(!pRead){
        // Hardly possible to occur
	OSHalt("Status error\n");
      }

      offset+=length;
      fileSize32B-=length;
      fileSize-=length;

      pRead->End=pRead->Start+length;
      pRead->Status=RstREADED;

      OSUnlockMutex(&Mutex);

      // If thread completes
      if(flag&FLAG_CLOSE){
	goto close;
      }

      //  If reading from begining
      if(flag&FLAG_TOP){
	goto restart;
      }

      // If file terminates
      if(fileSize<=0){
	// repeat
	goto repeat;
      }
      OSSendMessage(&MessageQueue,(void *)READ_START,OS_MESSAGE_NOBLOCK);
      break;
    case READ_WAIT:
      OSReceiveMessage(&MessageQueue,&msg,OS_MESSAGE_BLOCK);

      if((int)msg==READ_BRANK){
	msg=(void*)READ_START;
      }

      OSSendMessage(&MessageQueue,msg,OS_MESSAGE_NOBLOCK);
      break;
    case READ_BRANK:
      break;
    case READ_TOP:
      Status=TstSTOPING;
      // If reading
      OSLockMutex(&Mutex);
      pRead=GetStatus(RstREADING);
      OSUnlockMutex(&Mutex);
      if(!pRead){
	goto restart;
      }
#ifdef _DEBUG
      OSReport("FLAG_TOP\n");
#endif
      flag|=FLAG_TOP;    // Put on the flag " Read from beginnig ".
      break;
    case READ_CLOSE:
      Status=TstCLOSING;
      // If reading
      OSLockMutex(&Mutex);
      pRead=GetStatus(RstREADING);
      OSUnlockMutex(&Mutex);
      if((!pRead)||(ReadErr)||(DVD_STATE_FATAL_ERROR==DVDGetFileInfoStatus(pfileInfo))){
	goto close;
      }
#ifdef _DEBUG
      OSReport("FLAG_CLOSE\n");
#endif
      flag|=FLAG_CLOSE;    // Put on the flag " Thread terminates "
      break;
    case READ_ERR:
      // Set error flag
      ReadErr=1;
      break;
    default:
      // Hardlly possible to occur
      OSHalt("Undfined message\n");
      break;
    }
  }
 close:
#ifdef _DEBUG
  OSReport("end read thread\n");
#endif

  // close file
  pfileInfo=NULL;
  DVDClose(&fileInfo);

  Status=TstCLOSED;
  return 0;
}
/*---------------------------------------------------------------------------*
    Name:           ReadUSE
    Description:    Reads data from the buffer being used.
                    Does not read data if addr is NULL.
    Arguments:      addr       buffer address 
                    length     read length
                    offset     Location to start reading
    Returns:        read data size
 *---------------------------------------------------------------------------*/

static s32 ReadUSE(void *addr,s32 length,s32 offset)
{
  HVQM4Read *pRead;
  OSMessage msg;
  int i;

  if(OSReceiveMessage(&ErrQueue,&msg,OS_MESSAGE_NOBLOCK)){
    return -1;
  }

  OSLockMutex(&Mutex);
  // If data in the buffer being used?
  for(i=0,pRead=ReadObj;i<BUFNUM;i++,pRead++){
    if(pRead->Status==RstUSE){
      if((pRead->Start<=offset)&&(offset<pRead->End)){
	if(offset+length>pRead->End){
	  if(pRead->End-pRead->Start==BUFSIZE){
	    length=pRead->End-offset;
	  }
	}

	if(addr){
	  memcpy(addr,((u8 *)pRead->pBuf+offset-pRead->Start),(u32)length);
	}

	OSUnlockMutex(&Mutex);
	return length;
      }
    }
  }
  OSUnlockMutex(&Mutex);

  return 0;
}

/*---------------------------------------------------------------------------*
    Name:           ReadREADED
    Description:    Reads data from buffer that have been read.
                    Does not read data if addr is NULL.
    Arguments:      addr       buffer address 
                    length     data size (bytes) to be read
                    offset     Location to start reading
    Returns:        data       Size (bytes) that had beed read
 *---------------------------------------------------------------------------*/

static s32 ReadREADED(void *addr,s32 length,s32 offset)
{
  HVQM4Read *pRead;
  HVQM4Read *pRead1;
  OSMessage msg;
  int i;

  for(;;){
    if(OSReceiveMessage(&ErrQueue,&msg,OS_MESSAGE_NOBLOCK)){
      break;
    }

    OSLockMutex(&Mutex);
    for(i=0,pRead=ReadObj;i<BUFNUM;i++,pRead++){
      if(pRead->Status==RstREADED){
	if((pRead->Start<=offset)&&(offset<pRead->End)){

	  if(offset+length>pRead->End){
            // Hardly possible to occur
	    OSHalt("error \n");
	  }

	  if(addr){
	    memcpy(addr,((u8 *)pRead->pBuf+offset-pRead->Start),(u32)length);
	  }

	  pRead1=GetStatus(RstUSE);
	  if(pRead1){
	    pRead1->Status=RstBRANK;
	  }

	  pRead->Status=RstUSE;

	  OSUnlockMutex(&Mutex);

	  if(pRead1){
	    OSSendMessage(&MessageQueue,(void *)READ_BRANK,OS_MESSAGE_NOBLOCK);
	  }

	  return length;
	}
      }
    }
    OSUnlockMutex(&Mutex);
  }

  return -1;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4ErrStatus
    Description:    Return to state.
    Arguments:      none
    Returns:        status
 *---------------------------------------------------------------------------*/

s32 HVQM4ErrStatus(void)
{
  if(ReadErr==1){
    return DVD_STATE_FATAL_ERROR;
  }
  if(pfileInfo!=NULL){
    return DVDGetFileInfoStatus(pfileInfo);
  }
  return DVD_STATE_IGNORED;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4SendErr
    Description:    Sends error message to " thread for reading data ".
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/

void HVQM4SendErr(void)
{
  OSSendMessage(&ErrQueue,(void *)READ_ERR,OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4CloseRead
    Description:    Sends termination message to " thread for reading data ".
                    Takes time until the thread terminates. 
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/

void HVQM4CloseRead(void)
{
  OSSendMessage(&MessageQueue,(void *)READ_CLOSE,OS_MESSAGE_NOBLOCK);
  nglMemFree( ReadBuffer );
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4StopRead
    Description:    Sends stop message to " thread for reading data ".
                    Actually reads from begining. 
    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/

void HVQM4StopRead(void)
{
  OSSendMessage(&MessageQueue,(void *)READ_TOP,OS_MESSAGE_NOBLOCK);
  ReadOffset=0;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4GetStatus
    Description:    Get read thread status.
    Arguments:      none
    Returns:        status
 *---------------------------------------------------------------------------*/
TST HVQM4GetStatus(void)
{
  return Status;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4Read2Buf
    Description:    Reads buffer data from buffer into.
    Arguments:      addr       buffer address 
                    length     data size to be read (bytes)
    Returns:        -1 if read fails, data size if succeeded.
 *---------------------------------------------------------------------------*/

s32 HVQM4Read2Buf(void *addr,s32 length)
{
  s32 len;

  // for repeat
  if(ReadFileSize<ReadOffset+length){
    ReadOffset=0;
  }

  if((len=ReadUSE(addr,length,ReadOffset))<0){
    return -1;
  }
  if(len!=length){
    if(ReadREADED((addr==NULL?NULL:(u8 *)addr+len),length-len,ReadOffset+len)<0){
      return -1;
    }
  }

  ReadOffset+=length;

  return length;
}

/*---------------------------------------------------------------------------*
    Name:           HVQM4CreateReadThread
    Description:    Generates " thread for reading data ".
    Arguments:      pStr    file name 
    Returns:        none
 *---------------------------------------------------------------------------*/

void HVQM4CreateReadThread(char *pStr)
{
  OSMessage msg;

  ReadBuffer = (u8*)nglMemAlloc( BUFSIZE*BUFNUM, 32 );
  if(!InitFlag){
    // Initialize message queue
    OSInitMessageQueue
      (
       &MessageQueue,                                // pointer to message queue
       MessageArray,                                 // pointer to message boxes
       sizeof(MessageArray)/sizeof(MessageArray[0])  // # of message boxes
       );

    OSInitMessageQueue
      (
       &ErrQueue,                                // pointer to message queue
       ErrArray,                                 // pointer to message boxes
       sizeof(ErrArray)/sizeof(ErrArray[0])  // # of message boxes
       );
    // Initialize Mutex
    OSInitMutex(&Mutex);
    InitFlag=1;
  }

  for(;OSReceiveMessage(&MessageQueue,&msg,OS_MESSAGE_NOBLOCK);){
    ;
  }
  for(;OSReceiveMessage(&ErrQueue,&msg,OS_MESSAGE_NOBLOCK);){
    ;
  }
  ReadOffset=0;
  pfileInfo=NULL;

  // Generates thread to read
  OSCreateThread
    (
     &Thread,				// ptr to the thread to init
     HVQM4ReadThread,			// ptr to the start routine
     pStr,				// param passed to start routine
     ThreadStack+sizeof(ThreadStack),	// initial stack address
     sizeof(ThreadStack),		// stack size
     READ_PRI,				// scheduling priority
     OS_THREAD_ATTR_DETACH		// detached
     );
  // Starts thread
  OSResumeThread(&Thread);
}

/* end */

