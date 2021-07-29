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
// G A S   R P C   G E T   F D
//
void my_strupr(char *lift_me_up);

int _gas_rpc_get_fd( char *file_name )
{
	int ret=-1;
  int fd = -1;
  int counter = 0;
  char filename[64];
  sceCdlFILE cdfp;

  my_strupr( file_name );

  // find the file in the CD TOC
  strcpy(filename, "\\");
  strcat(filename, file_name);
  strcat(filename, ";1");

  if( sceCdDiskReady(0) == SCECdNotReady )
  {
    gas_printf( "EEEK!! CD not ready?\n" );
  }
  while ((ret <= 0) && (counter++ < 5))
    ret= sceCdSearchFile(&cdfp, filename);

  
  if(ret<=0)
  {
    // read from host disk
    strcpy(filename, "host0:");
    strcat(filename, file_name);
    gas_printf("sceCdSearchFile fail :%d\nTrying to load %s from the host disk.\n", ret, filename);
  }
  else
  {
    // read from cd
    strcpy(filename, "cdrom0:\\");
    strcat(filename, file_name);
    strcat(filename, ";1");
  }

  counter = 0;
  while ((fd < 0) && (counter++ < 5))
    fd = open(filename, O_RDONLY);
  
  return fd;
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
  RECORDRA
//  sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);

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
  RECORDRA

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
    if (off + up_size >= size)
    {
      up_size = size - off;
      //off = size;
      exit_flag = 1;
    }
    sceSdVoiceTransStatus(pri_ch, SD_TRANS_STATUS_WAIT );
  	sceSdVoiceTrans( pri_ch, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
      (u_char*)(iop_addr+off), (u_char *)addr, (u_int)up_size );
#ifdef DEBUG_OUTPUT
  	gas_printf("TSA0 = %x ", sceSdGetAddr( pri_ch|SD_A_TSA ) );
#endif
    if (exit_flag)
		{
			off += up_size;
      break;
		}

    addr+=UPLOAD_SIZE;
    off+=UPLOAD_SIZE;

    if (off + up_size >= size)
    {
      up_size = size - off;
      //off = size;
      exit_flag = 1;
    }
    sceSdVoiceTransStatus(sec_ch, SD_TRANS_STATUS_WAIT );
  	sceSdVoiceTrans( sec_ch, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
      (u_char*)(iop_addr+off), (u_char *)addr, (u_int)up_size );
#ifdef DEBUG_OUTPUT
  	gas_printf("TSA1 = %x ", sceSdGetAddr( sec_ch|SD_A_TSA ) );
#endif
    if (exit_flag)
		{
			off += up_size;
      break;
		}

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
  unsigned char *addr = NULL;
  char pri_ch = -1;

  RECORDRA
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
int _gas_rpc_get_voice(int inst, int *core, int *voice)
{
  // Assumed semaphore locks: system_state_sema
  int v;

  RECORDRA
  v = _gas_find_and_claim_free_voice(inst);
  if (v != -1)
  {
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


int _gas_rpc_get_voice_stereo(int inst, int *core, int *voice, int *core2, int *voice2)
{
  // Assumed semaphore locks: system_state_sema
  int v1, v2;

  RECORDRA
  
  if (_gas_find_and_claim_free_voice_stereo(inst, &v1, &v2) != -1)
  {
    *core   = v1 / PS2_MAX_VOICES_PER_CORE;
    *voice  = v1 % PS2_MAX_VOICES_PER_CORE;
    *core2  = v2 / PS2_MAX_VOICES_PER_CORE;
    *voice2 = v2 % PS2_MAX_VOICES_PER_CORE;
    
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
  RECORDRA
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
gas_printf("Instance %d - Core %d voice %d volume L/R (0x%x/0x%x) SSA: 0x%x\n", inst, core, voice, voll, volr, spu_addr);
#endif
  if (voice == 31) 
  {
    printf("OH SHIT\n");
#ifdef HALT_ON_FAIL
      while(1);
#else 
    return;
#endif
  }
//  _gas_set_voice_volume (core, voice, voll, volr);
	sceSdSetParam (core | (voice<<1) | SD_VP_PITCH, gas.inst_list[inst].pitch );
	sceSdSetAddr  (core | (voice<<1) | SD_VA_SSA, spu_addr );
  sceSdSetParam (core | (voice<<1) | SD_VP_ADSR1, 0x000f );
  sceSdSetParam (core | (voice<<1) | SD_VP_ADSR2, 0x1fc0 );		

  return;
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

void gas_debug_shutdown_profiler()
{
  if (g_profiler) 
  {
		StopHardTimer(g_profiler);
		FreeHardTimer(g_profiler);
		g_profiler = 0;
  }

}

u_long gas_debug_profile_stop(u_long start_time) 
{
  u_long ret = GetTimerCounter(g_profiler);
  ret -= start_time;
  return (ret * GAS_PROFILER_MILLISECOND_MULTIPLIER) / ticksPerMillisecond;
}



//  void _gas_error_halt(const char *msg)
//  Stops execution and displays a message

void _gas_error_halt(const char *msg)
{
  printf(msg);
#ifdef HALT_ON_FAIL
  while (1);
#endif
}

//  int _gas_acquire_voice(int inst, int which_voice)
//  Grabs a voice
//  if voice is unavailable, halts

int _gas_acquire_voice(int inst, int which_voice)
{
  RECORDRA
  if (gas.free_voices[which_voice].in_use == 0) 
  {
    gas.free_voices[which_voice].in_use = 1;
    gas.free_voices[which_voice].in_use_instance = gas.inst_list[inst].instance_id;
    return 1;
  }
  else 
  {
    _gas_error_halt("Acquire_voice:  the voice has been taken!\n"); 
    return 0;
  }
}

//  int _gas_release_voice(int inst, int which_voice)
//  Frees the voice specified by which_voice and inst

int _gas_release_voice(int inst, int which_voice)
{
  RECORDRA
  if (gas.free_voices[which_voice].in_use == 1) 
  {
    if (gas.free_voices[which_voice].in_use_instance == gas.inst_list[inst].instance_id) 
    {
      gas.free_voices[which_voice].in_use = 0;
      gas.free_voices[which_voice].in_use_instance = 0;
      if (_gas_check_inst_playing(inst))
        _gas_rpc_streamed_voice_controller(inst, SD_S_KOFF, 0);

    } 
    else 
    {
      _gas_error_halt("Release_voice:  the instance_id's don't match!\n"); 
    }
  }
  else
  {
    _gas_error_halt("Release_voice:  the voice isn't in use\n"); 
  }
  return 1;
}


//  Finds and acquires a voice.

int _gas_find_and_claim_free_voice(int inst)
{
	int next;
	int limit;
	int source;

	RECORDRA

	source = gas.inst_list[inst].source;

	if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.reverb)
	{
		// The first half of the voices are from core 0 which is set up for reverb.

		next = 0;
		limit = PS2_MAX_VOICES / 2;
	}
	else
	{
		// The second half of the voices are from core 1 which is not set up for reverb.

		next = PS2_MAX_VOICES / 2;
		limit = PS2_MAX_VOICES;
	}

	for ( ; next < limit; ++next)
	{ 
		if (!gas.free_voices[next].in_use) 
		{
			if (_gas_acquire_voice(inst, next))
			{
				return next;
			}
		}
	}

	return -1;
}

int _gas_find_and_claim_free_voice_stereo(int inst, int *v1, int *v2)
{
	RECORDRA

	*v1 = _gas_find_and_claim_free_voice(inst);

	if (*v1 == -1)
	{
		*v2 = -1;
		return -1;
	}

	*v2 = _gas_find_and_claim_free_voice(inst);

	if (*v2 == -1)
	{
		_gas_release_voice(inst, *v1);
		*v1 = -1;
		return -1;
	}

	return 1;
}



int _gas_acquire_instance_slot(int which_instance_slot)
{
  RECORDRA
  if (gas.free_instances[which_instance_slot].in_use == 0) 
  {
    gas.free_instances[which_instance_slot].in_use = 1;
  }
  else 
  {
    _gas_error_halt("Acquire_insastance_slot:  the instance slot has been taken!\n"); 
  }
  return 1;
}


int _gas_release_instance_slot(int which_instance_slot)
{
  RECORDRA
  if (gas.free_instances[which_instance_slot].in_use == 1) 
  {
    gas.free_instances[which_instance_slot].in_use = 0;
    gas.free_instances[which_instance_slot].in_use_instance = 0;
  }
 /* else
  {
    //_gas_error_halt("Release_voice:  the instance isn't in use\n"); 
    
  }*/
  return 1;
}


int _gas_find_and_claim_free_instance_slot()
{
  int i =0;
  int ret_val = -1;
  RECORDRA
  while ((i < GAS_MAX_INSTANCES) && (ret_val == -1))
  { 
    if (!gas.free_instances[i].in_use) 
    {
      ret_val = _gas_acquire_instance_slot(i);
      return i;  
    }
    i++;
  }
  return -1;
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
  int fd = 0;
  unsigned char *buffer = (unsigned char *)AllocSysMemory(0, readsize, NULL);
  int addr;
  int i;
  int n;
  int counter = 0;
  filename[15] += dump_count;
  dump_count++;
  while ((fd <=0)  && (counter++ < 5))
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

  RECORDRA

  if (dump_level > 0)
  {
    gas_printf("Behold, here art the voice hogs (%d used):\n", gas.inst_list_count);
    for (inst=0; inst<PS2_MAX_VOICES; ++inst)
    {
      _gas_debug_voice_status_dump_instance(inst);
    }
  }
  if (dump_level > 1)
  {
    gas_printf("\nAux info\n");
    for (inst=0; inst<PS2_MAX_VOICES; ++inst)
    {
      _gas_debug_voice_status_dump_instance_aux(inst);
    }
  }

  if (dump_level > 2)
    _gas_debug_voice_envelope_dump();
}


void _gas_debug_voice_envelope_dump()
{

  int voice, endflags, endflags2, v, i;
  int source;
  gas_printf("\nCore's 0 and 1, voice status\n");
  for (v=0; v<PS2_MAX_VOICES_PER_CORE; ++v)
  {
    voice = v << 1;
    endflags = sceSdGetParam(SD_CORE_0|SD_VP_ENVX|voice);
    endflags2 = sceSdGetParam(SD_CORE_1|SD_VP_ENVX|voice);
    if (gas.voice_used[v] != -1)
    {
      i = gas.voice_used[v] & GAS_INSTANCE_ID_MASK;
      source = gas.inst_list[i].source;
#ifdef NSL_LOAD_SOURCE_BY_NAME
      gas_printf("%2d (0x%08x) i0x%08x p%d %8s\t", v, endflags, gas.voice_used[v], 
        (_gas_check_inst_playing(i)), gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].filename);
#elif defined(GAS_LOAD_SOURCE_BY_ALIAS)
      gas_printf("%2d (0x%08x) i0x%08x p%d %d\t", v, endflags, gas.voice_used[v], 
        (_gas_check_inst_playing(i)), gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].aliasID);
		
#endif
    }
    else 
      gas_printf("%2d (0x%08x)\t\t\t\t", v, endflags);

    if (gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)] != -1)
    {
      i = gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)] & GAS_INSTANCE_ID_MASK;
      source = gas.inst_list[i].source;
#ifdef NSL_LOAD_SOURCE_BY_NAME
      gas_printf("(0x%08x) i0x%08x p%d %8s\n", endflags, gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)], 
        (_gas_check_inst_playing(i)), gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].filename);
#elif defined (NSL_LOAD_SOURCE_BY_ALIAS)
	  gas_printf("(0x%08x) i0x%08x p%d %d\n", endflags, gas.voice_used[(v+PS2_MAX_VOICES_PER_CORE)], 
        (_gas_check_inst_playing(i)), gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].aliasID);
#endif
    }
    else
      gas_printf("(0x%08x)\n", endflags2);
  }
  gas_printf("\n");

}

void _gas_debug_voice_status_dump_instance(int inst)
{

  int endx = 0;
  int endflags, voice;
  int nax = gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
  int source = gas.inst_list[inst].source;
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
#ifdef NSL_LOAD_SOURCE_BY_NAME
  gas_printf("%8s: 0x%04x\tst(%d) l(%d) endx(%2d) (u%d p%d t%d s%d(%3d/%3d) r%d n%d) == check(%d) nax:0x%08x %s\n", 
    gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].filename,
    gas.inst_list[inst].instance_id, 
//      gas.inst_list[inst].flag.voicel, gas.inst_list[inst].flag.corel, 
//      gas.inst_list[inst].voll, gas.inst_list[inst].volr, 
    gas.inst_list[inst].flag.stereo,
    gas.inst_list[inst].flag.loop,
    endx,
    gas.inst_list[inst].flag.used, 
    (_gas_check_inst_playing(inst))?1:0, 
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
     nax,
    (gas.inst_list[inst].flag.used)?"USED":"");
#endif


#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  gas_printf("%d: 0x%04x\tst(%d) l(%d) endx(%2d) (u%d p%d t%d s%d(%3d/%3d) r%d n%d) == check(%d) nax:0x%08x %s\n", 
    gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].aliasID,
    gas.inst_list[inst].instance_id, 
//      gas.inst_list[inst].flag.voicel, gas.inst_list[inst].flag.corel, 
//      gas.inst_list[inst].voll, gas.inst_list[inst].volr, 
    gas.inst_list[inst].flag.stereo,
    gas.inst_list[inst].flag.loop,
    endx,
    gas.inst_list[inst].flag.used, 
    (_gas_check_inst_playing(inst))?1:0, 
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
     nax,
    (gas.inst_list[inst].flag.used)?"USED":"");
#endif

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
  RECORDRA
  memset(buffer, 0x00, readsize);
  for (i=0; i<0x10; ++i)
  {  
    // grab an image of smem
    addr = (i*readsize);


    sceSdVoiceTrans(0, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, 
      (u_char*)buffer, (u_char *)addr, (u_int)readsize );
    sceSdVoiceTransStatus(0, SD_TRANS_STATUS_WAIT);
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


#endif

