#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include <string.h>
#include "../gas.h" 
#include "GasSystem.h"


//-----------------------------------------------------------------------------------
// G A S   R P C   C R E A T E   S E M A
//
// if used_flag == 1, then the semaphore is used 'locked' by default
int gas_rpc_create_sema(int used_flag)
{
  int ret;
  struct SemaParam param;
  param.attr = SA_THFIFO;
  param.initCount = !used_flag;
  param.maxCount = 1;
  param.option = 0;

  ret = CreateSema( &param );
  switch (ret)
  {
    case KE_NO_MEMORY:       gas_printf("Error creating semaphore: Insufficient memory\n");        break;
    case KE_ILLEGAL_ATTR:    gas_printf("Error creating semaphore: Invalid attr specification\n"); break;
    case KE_ILLEGAL_CONTEXT: gas_printf("Error creating semaphore: Call was from exception handler or interrupt handler\n"); break;
    default: /* everything is peachy */ break;
  }
  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   D E L E T E   S E M A
//
void gas_rpc_delete_sema(int sema_id)
{
  int ret;

  ret = DeleteSema( sema_id );
  switch (ret)
  {
    case KE_ILLEGAL_CONTEXT: gas_printf("Delete sema call from exception handler or interrupt handler\n"); break;
    case KE_UNKNOWN_SEMID:   gas_printf("Delete sema error, specified semaphore does not exist\n"); break;
    default: /* everything is peachy */ break;
  }
}


//-----------------------------------------------------------------------------------
// G A S   R P C   L O A D   F I L E
//
char* _gas_rpc_load_file(char *orig_filename, int *size)
{
  // Assumed semaphore locks: system_state_sema
  // blocking read - not so good.  Fortunately it only happens at the start of a level
	int fd;
	char *buffer;
  int ret;
  char filename[64];

  sceCdlFILE cdfp;

  // find the file in the CD TOC
  strcpy(filename, "\\");
  strcat(filename, orig_filename);
  strcat(filename, ";1");

//  gas_printf("Search Filename: %s\n", filename);
  sceCdDiskReady(0);
  ret= sceCdSearchFile(&cdfp, filename);

//  gas_printf("Ret = %d\n", ret);

  if(!ret)
  {
    // read from host disk
    gas_printf("sceCdSearchFile fail :%d\nTrying to load the sound from the host disk.\n", ret);

    strcpy(filename, gas.host_prefix);
    strcat(filename, orig_filename);

    fd = open(filename, O_RDONLY);
 	  *size = lseek(fd, 0, 2);
	  if( *size <= 0 ) { gas_printf( "\nCan't load VAG file to iop heap \n" ); return NULL; }
    if (*size > (MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE))
    {
      gas_printf("Error.  This sound is too big %s to upload, size %d (640kb upload max)\n", orig_filename, *size);
      close(fd);
      return NULL;
    }
	  lseek( fd, 0, 0);

	  buffer = gas.iop_cd_mono_buffers_top;

	  read( fd, buffer, *size);
	  close(fd);
  }
  else
  {
    // read from cd
    strcpy(filename, "cdrom0:\\");
    strcat(filename, orig_filename);
    strcat(filename, ";1");

    *size = cdfp.size;
    if (*size > (MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE))
    {
      gas_printf("Error.  This sound is too big %s to upload, size %d (640kb upload max)\n", orig_filename, *size);
      return NULL;
    }
    fd = open(filename, O_RDONLY);
	  if( fd <= 0 ) { gas_printf( "\nCan't load VAG file to iop heap \n" ); return NULL; }

    // use the mono buffers (640kb) as a clearinghouse for the spu sound data
	  buffer = gas.iop_cd_mono_buffers_top;

	  read( fd, buffer, *size);
	  close(fd);
  }

  // set up the rest of the member data for this source

  return buffer;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S T R E A M E D   V O I C E   C O N T R O L L E R
//
// Encapsulates the sceSd calls to turn a sound on or off, supports stereo, as well as
// stopping (and removing instance) and pausing (restart at paused location) support
// for on and off requests
//
// inst is the instance to use, mode is the SCE voice command (either SD_S_KON or SD_S_KOFF)
// and store_pos_flag is 0 for most calls, but when non-zero and the mode is SD_S_KOFF, it
// sets the start position to the current position for pausing restart support.
void _gas_rpc_streamed_voice_controller(int inst, int mode, int store_pos_flag)
{
  // Assumed semaphore locks: system_state_sema
  int nax;
  RECORDRA;
//  sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);

//  gas_printf("Before\n");
//  _gas_debug_voice_envelope_dump();

  if (store_pos_flag && (mode == SD_S_KOFF))
  {
    nax = gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
#ifdef DEBUG_OUTPUT
gas_printf("Pause Target - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corel,
  gas.inst_list[inst].flag.voicel, nax);
#endif
    sceSdSetAddr(gas.inst_list[inst].flag.corel|SD_VA_SSA|(gas.inst_list[inst].flag.voicel<<1), nax );
    if (gas.inst_list[inst].flag.stereo == 1)
    {
      nax = gas_update_get_NAX(gas.inst_list[inst].flag.corer, (gas.inst_list[inst].flag.voicer<<1));
#ifdef DEBUG_OUTPUT
gas_printf("Pause Target(r) - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corer,
  gas.inst_list[inst].flag.voicer, nax);
#endif
      sceSdSetAddr(gas.inst_list[inst].flag.corer|SD_VA_SSA|(gas.inst_list[inst].flag.voicer<<1), nax );
    }
  }

  if ((gas.inst_list[inst].flag.stereo == 1) && gas.inst_list[inst].flag.corel == gas.inst_list[inst].flag.corer)
  {
  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|mode , ((0x1<<gas.inst_list[inst].flag.voicel) | 
      (0x1<<gas.inst_list[inst].flag.voicer)));
  }
  else
  {
  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|mode , 0x1<<gas.inst_list[inst].flag.voicel);
    if (gas.inst_list[inst].flag.stereo)
    	sceSdSetSwitch( gas.inst_list[inst].flag.corer|mode , 0x1<<gas.inst_list[inst].flag.voicer);
  }

//  gas_printf("After\n");
//  _gas_debug_voice_envelope_dump();
}


// for dma to spu
#define UPLOAD_SIZE 0x40000

//-----------------------------------------------------------------------------------
// G A S   R P C   D M A   I O P   T O   S P U   B L O C K I N G
//
// Performs a dma transfer of a block of memory in the iop to the spu, and
// blocks until the transfer is complete.  Could use some updating to make
// it more general
int _gas_rpc_dma_iop_to_spu_blocking(unsigned char *iop_addr, unsigned char *spu_addr, int size)
{
  // Assumed semaphore locks: system_state_sema
  // also assumes exclusive access to DMA channels
  int exit_flag = 0;
  unsigned char *addr;
  int off;
  int up_size = UPLOAD_SIZE;
  char pri_ch = 0;
  char sec_ch = 1;
  RECORDRA;

	// --- data transfer
#ifdef DEBUG_OUTPUT
	gas_printf("Data transfer ... " );
#endif
  exit_flag = 0;
  up_size = UPLOAD_SIZE;

  // if ch 0 is clogged
  if (sceSdVoiceTransStatus(0, SD_TRANS_STATUS_CHECK ) == 0)
  {
    pri_ch = 1;
    sec_ch = 0;
  }

  for (addr=spu_addr, off=0; exit_flag==0; )
  {
    if (size < up_size)
    {
      up_size = size;
      exit_flag = 1;
    }
    if (off >= size)
    {
      up_size -= off-size;
      off = size;
      exit_flag = 1;
    }
    sceSdVoiceTransStatus(pri_ch, SD_TRANS_STATUS_WAIT );
  	sceSdVoiceTrans( pri_ch, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
      (u_char*)(iop_addr+off), (u_char *)addr, (u_int)up_size );
#ifdef DEBUG_OUTPUT
  	gas_printf("TSA0 = %x ", sceSdGetAddr( pri_ch|SD_A_TSA ) );
#endif
    if (exit_flag)
      break;

    addr+=UPLOAD_SIZE;
    off+=UPLOAD_SIZE;

    if (off >= size)
    {
      up_size -= off-size;
      off = size;
      exit_flag = 1;
    }
    sceSdVoiceTransStatus(sec_ch, SD_TRANS_STATUS_WAIT );
  	sceSdVoiceTrans( sec_ch, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
      (u_char*)(iop_addr+off), (u_char *)addr, (u_int)up_size );
#ifdef DEBUG_OUTPUT
  	gas_printf("TSA1 = %x ", sceSdGetAddr( sec_ch|SD_A_TSA ) );
#endif
    if (exit_flag)
      break;

    addr+=UPLOAD_SIZE;
    off+=UPLOAD_SIZE;
  }
  sceSdVoiceTransStatus(0, SD_TRANS_STATUS_WAIT );
  sceSdVoiceTransStatus(1, SD_TRANS_STATUS_WAIT );

  if (off == 0)
    off = size;
#ifdef DEBUG_OUTPUT
  gas_printf("Total sent %d\n", off );
#endif
  return off;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   D M A   I O P   T O   S P U   N O N B L O C K I N G
//
int _gas_rpc_dma_iop_to_spu_nonblocking(unsigned char *iop_addr, unsigned char *spu_addr, int size)
{
  // Assumed semaphore locks: system_state_sema
  // also assumes exclusive access to DMA channels
  unsigned char *addr;
  char pri_ch = -1;

  RECORDRA;
  if (size >= UPLOAD_SIZE)
  {
    gas_printf("ERROR.  What are you doing, calling the nonblocking version with a size of %d?!?!  Trying to recover.\n",
      size);
    return _gas_rpc_dma_iop_to_spu_blocking(iop_addr, spu_addr, size);
  }

	// --- data transfer
#ifdef DEBUG_OUTPUT
	gas_printf("Data transfer ... (nonblocking)\n" );
#endif
  // if ch 0 is clogged
  if (sceSdVoiceTransStatus(0, SD_TRANS_STATUS_CHECK ) == 0)
  {
    if (sceSdVoiceTransStatus(1, SD_TRANS_STATUS_CHECK) == 1)
      pri_ch = 1;
  }
  else
  {
    pri_ch = 0;
  }
  if (pri_ch == -1)
    return -1;

	sceSdVoiceTrans( pri_ch, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
    (u_char*)(iop_addr), (u_char *)addr, (u_int)size );

#ifdef DEBUG_OUTPUT
  gas_printf("TSA0 = %x \n", sceSdGetAddr( pri_ch|SD_A_TSA ) );
  gas_printf("Total sent %d\n", size );
#endif
  return size;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   G E T   V O I C E
//
// locks down a voice for use by an instance
int _gas_rpc_get_voice(int *core, int *voice)
{
  // Assumed semaphore locks: system_state_sema
  unsigned short v;

  RECORDRA;
  if (gas.free_voices.count > 0)
  {
    v = fifo_queue_pop(&gas.free_voices);
    *core = v / PS2_MAX_VOICES_PER_CORE;
    *voice = v % PS2_MAX_VOICES_PER_CORE;
#ifdef DEBUG_OUTPUT
gas_printf("using core %d voice %d\n", *core, *voice);
#endif
  }
  else
    return 0;
  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S E T   V O I C E
//
// !!! needs to be updated to support adsr
// side == 0 is left channel, side == 1 is right channel
void
_gas_rpc_set_voice (int inst, int side, unsigned int spu_addr)
{
  // Assumed semaphore locks: system_state_sema
  int voice, core, voll, volr;
  RECORDRA;
  if (side == 0)
  {
    voice = gas.inst_list[inst].flag.voicel;
    core = gas.inst_list[inst].flag.corel;
    voll = gas.inst_list[inst].voll;
    volr = gas.inst_list[inst].volr;
  }
  else
  {
    voice = gas.inst_list[inst].flag.voicer;
    core = gas.inst_list[inst].flag.corer;
    voll = gas.inst_list[inst].rvoll;
    volr = gas.inst_list[inst].rvolr;
  }
#ifdef DEBUG_OUTPUT
gas_printf("Instance %d - Core %d voice %d volume L/R (0x%x/0x%x)\n", inst, core, voice, voll, volr);
#endif
//  _gas_set_voice_volume (core, voice, voll, volr);
	sceSdSetParam (core | (voice<<1) | SD_VP_PITCH, gas.inst_list[inst].pitch );
	sceSdSetAddr  (core | (voice<<1) | SD_VA_SSA, spu_addr );
  sceSdSetParam (core | (voice<<1) | SD_VP_ADSR1, 0x000f );
  sceSdSetParam (core | (voice<<1) | SD_VP_ADSR2, 0x1fc0 );		

  return;
}



#ifdef DEBUG

//-----------------------------------------------------------------------------------
// G A S   D E B U G   S P U   M E M   D U M P
//
// Dumps the contents of spu memory to disk, for debugging
void _gas_debug_spu_mem_dump()
{
  // Assumed semaphore locks: system_state_sema
static int dump_count = 0;
  int readsize = 0x40;
  char filename[] = "host0:smem_dump1.txt";
  int fd;
  unsigned char *buffer = (unsigned char *)AllocSysMemory(0, readsize, NULL);
  int addr;
  int i;
  int n;
  
  filename[15] += dump_count;
  dump_count++;
  
  fd = open(filename, O_WRONLY|O_TRUNC|O_CREAT);
  
  for (i=0; i<0x8000; ++i)
  {  
    // grab an image of smem
    memset(buffer, 0xbb, readsize);
    addr = (i*readsize);

    sceSdVoiceTransStatus(0, SD_TRANS_STATUS_WAIT);
    sceSdVoiceTrans(0, SD_TRANS_MODE_READ|SD_TRANS_BY_DMA, 
      (u_char*)buffer, (u_char *)addr, (u_int)readsize );
#ifdef DEBUG_OUTPUT
gas_printf("dumping 0x%x thru 0x%x\n", addr, addr+readsize);
#endif
    // write it to the dump file
    n = write(fd, buffer, readsize);
    if (n != readsize)
      gas_printf("Yikes!\n");
  }
  close(fd);
  FreeSysMemory(buffer);
}
int dump_level = 3;

//-----------------------------------------------------------------------------------
// G A S   D E B U G   V O I C E   S T A T U S   D U M P
//
void _gas_debug_voice_status_dump()
{
  // Assumed semaphore locks: system_state_sema
  int inst;

  RECORDRA;

  if (dump_level > 0)
  {
    gas_printf("Behold, here art the voice hogs (%d used):\n", gas.inst_list_count);
    for (inst=0; inst<PS2_MAX_VOICES; ++inst)
    {
      _gas_debug_voice_status_dump_instance(inst);
    }
  }
/*  if (dump_level > 1)
  {
    gas_printf("\nAux info\n");
    for (inst=0; inst<PS2_MAX_VOICES; ++inst)
    {
      _gas_debug_voice_status_dump_instance_aux(inst);
    }
  }*/
  if (dump_level > 2)
    _gas_debug_voice_envelope_dump();
}


void _gas_debug_voice_envelope_dump()
{
  int voice, endflags, endflags2, v, i;
  gas_printf("\nCore's 0 and 1, voice status\n");
  for (v=0; v<PS2_MAX_VOICES_PER_CORE; ++v)
  {
    voice = v << 1;
    endflags = sceSdGetParam(SD_CORE_0|SD_VP_ENVX|voice);
    endflags2 = sceSdGetParam(SD_CORE_1|SD_VP_ENVX|voice);
    if (gas.voice_used[v] != -1)
    {
      i = gas.voice_used[v] & GAS_INSTANCE_ID_MASK;
      gas_printf("%2d (0x%08x) i0x%08x p%d %8s\t", v, endflags, gas.voice_used[v], 
        (gas.inst_list[i].pause_count < 1), gas.source_list[gas.inst_list[i].source].short_filename);
    }
    else 
      gas_printf("%2d (0x%08x)\t\t\t\t", v, endflags);

    if (gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)] != -1)
    {
      i = gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)] & GAS_INSTANCE_ID_MASK;
      gas_printf("(0x%08x) i0x%08x p%d %8s\n", endflags, gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)], 
        (gas.inst_list[i].pause_count < 1), gas.source_list[gas.inst_list[i].source].short_filename);
    }
    else
      gas_printf("(0x%08x)\n", endflags2);
  }
  gas_printf("\n");
}

void _gas_debug_voice_status_dump_instance(int inst)
{
  int endx, endflags, voice;
  if (gas.inst_list[inst].flag.src_type != SRC_TYPE_SPU)
    endx = -1;
  else
  {
    endflags = sceSdGetSwitch(gas.inst_list[inst].flag.corel|SD_S_ENDX);

    if (gas.inst_list[inst].flag.src_type == SRC_TYPE_SPU)
    {
      voice = gas.inst_list[inst].flag.voicel;
      voice = 1 << voice;
      endx = endflags & voice;
      endx = endx != 0;
    }
  }
  // check if this instance is still playing, and whether it has active flags set, etc
  gas_printf("%8s: 0x%04x\tst(%d) l(%d) endx(%2d) (u%d p%d t%d s%d(%3d/%3d) r%d n%d) == check(%d) %s\n", 
    gas.source_list[gas.inst_list[inst].source].short_filename,
    gas.inst_list[inst].instance_id, 
//      gas.inst_list[inst].flag.voicel, gas.inst_list[inst].flag.corel, 
//      gas.inst_list[inst].voll, gas.inst_list[inst].volr, 
    gas.inst_list[inst].flag.stereo,
    gas.inst_list[inst].flag.loop,
    endx,
    gas.inst_list[inst].flag.used, 
    (gas.inst_list[inst].pause_count < 1)?1:0, 
    (gas.inst_list[inst].flag.src_type != SRC_TYPE_SPU),
    (gas.inst_list[inst].spu_reload < 0),
    (gas.inst_list[inst].spu_reload),
    (gas.inst_list[inst].iop_reload),
    gas.inst_list[inst].flag.ready, 
    (gas.inst_list[inst].flag.needs_spu_preload == 0),
       ((gas.inst_list[inst].flag.used) &&
        (gas.inst_list[inst].pause_count < 1) && 
        (gas.inst_list[inst].flag.src_type != SRC_TYPE_SPU) && 
        (gas.inst_list[inst].spu_reload < 0) &&
        (gas.inst_list[inst].flag.ready == 1) &&
        (gas.inst_list[inst].flag.needs_spu_preload == 0)),
    (gas.inst_list[inst].flag.used)?"USED":"");
}


void _gas_debug_voice_status_dump_instance_aux(int inst)
{
  gas_printf("\t\tbl%d br%d vl%d%d vr%d%d ib%d\n", 
    gas.inst_list[inst].spu_bufl, 
    gas.inst_list[inst].spu_bufr, 
    gas.inst_list[inst].flag.corel, 
    gas.inst_list[inst].flag.voicel, 
    gas.inst_list[inst].flag.corer, 
    gas.inst_list[inst].flag.voicer, 
    gas.inst_list[inst].iop_buf);
}


//-----------------------------------------------------------------------------------
// G A S   D E B U G   S P U   M E M   C L E A R
//
// also for debugging, clears out spu memory, so that we don't get garbage from previous
// runs in spu dumps.  A clear of some sort will probably be good to do even on a release
// build.
void _gas_debug_spu_mem_clear()
{
  // Assumed semaphore locks: system_state_sema
  int readsize = 0x20000;
  unsigned char *buffer = (unsigned char *)AllocSysMemory(0, readsize, NULL);
  int addr;
  int i;
  
  memset(buffer, 0x00, readsize);
  for (i=0; i<0x10; ++i)
  {  
    // grab an image of smem
    addr = (i*readsize);

    sceSdVoiceTransStatus(0, SD_TRANS_STATUS_WAIT);
    sceSdVoiceTrans(0, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
      (u_char*)buffer, (u_char *)addr, (u_int)readsize );
gas_printf("clearing 0x%x thru 0x%x\n", addr, addr+readsize);
  }
  FreeSysMemory(buffer);
}

void gas_debug_check_value(const char *context_str, int check_me, int low_end, int high_end)
{
  if (check_me < low_end || check_me > high_end)
  {
    gas_printf("Uber bad.  We caught a bad push in context %s (check %d, low %d, hi %d)\n", context_str,
      check_me, low_end, high_end);
    // force a halt -- the only way I could think of.
#ifdef HALT_ON_FAIL
    while(1) /* do nothing */;
#endif
  }
}

int g_profiler = 0;
int ticksPerMillisecond = 1;

void gas_debug_init_profiler()
{
  if (!g_profiler) 
  {
    struct SysClock clock;
    USec2SysClock(1000, &clock);
    ticksPerMillisecond = clock.low;
    gas_printf("%d ticks per millisecond.\n", ticksPerMillisecond);
    g_profiler = AllocHardTimer(TC_SYSCLOCK, 32, 256);
    SetupHardTimer(g_profiler, TC_SYSCLOCK, TM_NO_GATE, 1);
    StartHardTimer(g_profiler);
  }
}

u_long gas_debug_profile_stop(u_long start_time) 
{
  u_long ret = GetTimerCounter(g_profiler);
  ret -= start_time;
  return (ret * GAS_PROFILER_MILLISECOND_MULTIPLIER) / ticksPerMillisecond;
}

#endif

