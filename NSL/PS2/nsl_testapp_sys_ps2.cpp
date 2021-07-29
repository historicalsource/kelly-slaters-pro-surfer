#include <eekernel.h>
#include <libsdr.h>
#include <sdrcmd.h>
#include <libdev.h>
#include <libpad.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <libcdvd.h>
#include <stdio.h>

#include "fifo_queue.h"


#include "nsl_testapp_sys.h"

//#define _MOD_ROOT    "host0:/usr/local/sce/iop/modules/"
#define _MOD_ROOT    "host0:"
#define _GAS_ROOT    
#define _MOD_SIO2MAN "sio2man.irx"
#define _MOD_PADMAN  "padman.irx"
#define _MOD_SDR     "sdrdrv.irx"
#define _MOD_LIBSD   "libsd.irx"
#define _MOD_GAS_IOP "gas.irx"



u_long128 pad_dma_buf[scePadDmaBufferMax]  __attribute__((aligned (64)));



#if (defined(CD_BOOT)||defined(DVD_BOOT))
#define IOPRP_IMG "cdrom0:\\"IOP_IMAGE_FILE";1"
#else
#define IOPRP_IMG "host0:"IOP_IMAGE_FILE
#endif
  

void systemInit (void)
{ 
  /* wait for DiskReady */
  sceCdInit(0/*SCECdINIT*/);
#if defined(DVD_BOOT)
  sceCdMmode(SCECdDVD);
#else
  sceCdMmode(SCECdCD);
#endif
  
  /* reboot IOP, replace default modules */
  while( !sceSifRebootIop(IOPRP_IMG) );
  while( !sceSifSyncIop() );
  sceSifInitRpc(0);
  
  /* reinitialize */
  sceCdInit(0/*SCECdINIT*/);
#if defined(DVD_BOOT)
  sceCdMmode(SCECdDVD);
#else
  sceCdMmode(SCECdCD);
#endif
  sceFsReset();

  // Reboot IOP with new IMG flash
  sceSifInitRpc(0);

  printf ("loading modules ...\n");

  while (sceSifLoadModule (_MOD_ROOT _MOD_SIO2MAN, 0, NULL) < 0) 
  {
    printf ("  loading %s failed\n", _MOD_SIO2MAN);
  }
  
  while (sceSifLoadModule(_MOD_ROOT _MOD_LIBSD, 0, NULL) < 0) 
  {
    printf("  loading libsd.irx failed\n");
  }

  while (sceSifLoadModule (_MOD_ROOT _MOD_PADMAN, 0, NULL) < 0) 
  {
    printf ("  loading %s failed\n", _MOD_PADMAN);
  }
  scePadInit(0);
  scePadPortOpen(0, 0, pad_dma_buf);

/*  while (sceSifLoadModule ("host0:gas.irx", 0, NULL) < 0) 
  {
    printf("  loading gas.irx failed\n");
  }
  ChangeThreadPriority(GetThreadId(), 50);
*/
  printf ("loading modules ... done.\n");
  return;

}




void checkControlPad ( fifo_queue<int> *controllerCommands ) 
{
  u_int paddata;
  u_char rdata [32]; 

  // These are for debouncing buttons
  static bool sentO=0 , sentS=0 , sentT=0, sentX=0, sentStart;
  
  // Controller
  #define _PAD_DATA (0xffff ^ ((rdata[2] << 8) | rdata[3]))
  if (scePadRead (0, 0, rdata) > 0) 
    paddata = _PAD_DATA;
  else                             
    paddata = 0;

  #ifdef DEBUG
  if (paddata != 0) 
    printf ("buttons 0x%08x\n", paddata);
  #endif

  // Controller operation
  #define _VOL_DELTA 0x80
  if (paddata & SCE_PADLup) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B5);
  } 
  else if (paddata & SCE_PADLdown) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B6);
  } 
  else if (paddata & SCE_PADLleft) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B7);
  }
  else if (paddata & SCE_PADLright) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B8);
  }
  else if (paddata & SCE_PADL1) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B9);
  }
  else if (paddata & SCE_PADL2) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B10);
  }
  else if (paddata & SCE_PADR1) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B11);
  }
  else if (paddata & SCE_PADR2) 
  {
    controllerCommands->push((unsigned short)NSL_TESTBUTTON_B12);
  }
  if (paddata & SCE_PADstart) {
    if (sentStart == 0) {
      controllerCommands->push((unsigned short)NSL_TESTBUTTON_START);
      sentStart = 1;
    } 
  } else {
    sentStart = 0;
  }
  if (paddata & SCE_PADRright)
  {
    if (sentO == 0)
    {
      sentO=1;
      controllerCommands->push((unsigned short)NSL_TESTBUTTON_B2);
    }
  }
  else
  {
    sentO = 0;
  }
  if (paddata & SCE_PADRdown)
  {
    if (sentX == 0)
    {
      sentX = 1;
      controllerCommands->push((unsigned short)NSL_TESTBUTTON_B3);
    }
  }
  else
  {
    sentX = 0;
  }
  if (paddata & SCE_PADRleft)
  {
    if (sentS == 0)
    {
      sentS = 1;
      controllerCommands->push((unsigned short)NSL_TESTBUTTON_B4);
    }
  }
  else
  {
    sentS = 0;
  }
  if (paddata & SCE_PADRup)
  {
    if (sentT == 0)
    {
      sentT = 1;
      controllerCommands->push((unsigned short)NSL_TESTBUTTON_B1);
    }
  }
  else
  {
    sentT = 0;
  }

//  VSync();

}


int started = 0;

void start_timer() 
{
  started = get_cpu_cycle();
}

int check_time() 
{ 
  return get_cpu_cycle() - started;
}

