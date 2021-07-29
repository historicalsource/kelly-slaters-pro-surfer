// GAS command support functions
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include <string.h>
#include "../gas.h" 
#include "GasSystem.h"


//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   S P U   S O U R C E 
//
// Loads a new SPU source into SPU ram
int _gas_rpc_add_spu_source(int entry)
{
  // Assumed semaphore locks: system_state_sema
	int size;
	char *data_buffer;
  RECORDRA;

	if( (data_buffer = _gas_rpc_load_file(gas.source_list[entry].full_filename, &size)) == NULL )
    return -1;

  gas.source_list[entry].src.spu.addr = gas.spu_heap_curr;

  if (size != gas.source_list[entry].size)
  {
gas_printf("Warning, the disk size(%d) and the entry size(%d) do not match for %s\n",
  size, gas.source_list[entry].size, gas.source_list[entry].full_filename);
    gas.source_list[entry].size = size;
  }

  // set up the markings for looping or one-shot sounds
  //_AdpcmSetMarkSTART(data_buffer, size);
  if (gas.source_list[entry].flag.loop)
  {
    _AdpcmSetMarkLOOP(data_buffer, size);
  }
  else
  {
    _AdpcmSetMarkSTOP(data_buffer, size);
  }

  // do the upload
  if (_gas_rpc_dma_iop_to_spu_blocking(data_buffer, (unsigned char *)gas.source_list[entry].src.spu.addr, size) != size)
    gas_printf("What the...\n");

  gas.spu_heap_curr += gas.source_list[entry].size;
  gas.source_list[entry].flag.loaded = 1;
  return entry;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   C D   S O U R C E 
//
// prepares a CD source structure
int _gas_rpc_add_cd_source(int entry)
{
  // Assumed semaphore locks: system_state_sema
  char filename[64];
  int ret;

  sceCdlFILE cdfp;
  RECORDRA;

  // find the file in the CD TOC
  strcpy(filename, "\\");
  strcat(filename, gas.source_list[entry].full_filename);

#ifdef DEBUG_OUTPUT
  gas_printf("Search Filename: %s\n", filename);
#endif
  sceCdDiskReady(0);
  ret= sceCdSearchFile(&cdfp, filename);

  if(!ret)
  {
    // read from host disk
    gas_printf("sceCdSearchFile fail :%d\nThis stream will not be played.\n", ret);
    return -1;
  }
  else
  {
    // read from cd
    if (cdfp.size != gas.source_list[entry].size)
    {
      gas_printf("Warning, the disk size(%d) and the entry size(%d) do not match for %s\n",
        cdfp.size, gas.source_list[entry].size, filename);
      gas.source_list[entry].size = cdfp.size;
    }

    gas.source_list[entry].src.cd.start_sector = cdfp.lsn;
  }

  // set up the rest of the member data for this source
  gas.source_list[entry].flag.loaded = 1;

  return entry;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P L A Y   S P U   I N S T A N C E
//
// plays back an spu-hosted source
void _gas_rpc_play_spu_instance(int inst)
{
  // Assumed semaphore locks: system_state_sema
  short core = gas.inst_list[inst].flag.corel;
  short voice = gas.inst_list[inst].flag.voicel;
  int   addr = gas.inst_list[inst].src.spu.cur_addr;
  RECORDRA;
  _gas_rpc_set_voice (inst, 0, addr);

	sceSdSetSwitch( core|SD_S_KON , 0x1<<voice);
  gas.inst_list[inst].pause_count = 0;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P L A Y   C D   I N S T A N C E
//
// begins playback of an cd-based source
void _gas_rpc_play_cd_instance(int inst)
{
  // Assumed semaphore locks: system_state_sema
  unsigned addr;

  RECORDRA;
  if (gas.inst_list[inst].flag.ready) 
  {
    // make sure our spu upload is there.
    if (gas.inst_list[inst].flag.needs_spu_preload) 
    {
//      gas_printf("Performing SPU preload (%d)\n", inst);
      _gas_rpc_init_spu_streaming(inst, gas.inst_list[inst].source);
    }
    else
    {
//      gas_printf("Skipping SPU preload (%d)\n", inst);
    }

    addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * SPU_BUFFER_SIZE);
    gas.inst_list[inst].flag.needs_spu_preload = 0;

    _gas_rpc_set_voice (inst, 0, addr);
    if (gas.inst_list[inst].flag.stereo)
    {
      addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufr * SPU_BUFFER_SIZE);
      _gas_rpc_set_voice (inst, 1, addr);
    }
    _gas_rpc_streamed_voice_controller(inst, SD_S_KON, 0);
    gas.inst_list[inst].pause_count = 0;
  }
  else
  {
    gas_printf("Error.  Attempted to playback a CD instance before it was ready, play request ignored.\n");
  }
}


//-----------------------------------------------------------------------------------
// G A S   R P C   I N I T   S P U   S T R E A M I N G
//
// use carefully, curr_iop_buf_addr and last_iop_buf_addr should be initialized correctly
// for the source type before calling
int _gas_rpc_init_spu_streaming(int inst, int source)
{
  int i;
  RECORDRA;
  // grab a free spu buffer and preload it
  gas.inst_list[inst].spu_bufl = fifo_queue_pop(&gas.free_spu_bufs);
  if (gas.inst_list[inst].spu_bufl == -1)
  {
    printf("UGLY MESSAGE: Couldn't get an spu buffer.  Queue contents:\n");
    gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_spu_bufs.start, gas.free_spu_bufs.end, gas.free_spu_bufs.count);
    for (i=0; i<gas.free_spu_bufs.queue_max; ++i)
    {
      gas_printf("%d ", gas.free_spu_bufs.queue[i]);
    }
    gas_printf("\n");
  }
  if (_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 0) == -1)
  {
    // abort
    gas_debug_check_value("INIT SPU STREAMING ABORT", gas.inst_list[inst].spu_bufl, 0, gas.spu_buffer_num);
    fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
    return -1;
  }
  gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;

  if (gas.source_list[source].flag.stereo)
  {
    // grab a free spu buffer and preload it
    gas.inst_list[inst].spu_bufr = fifo_queue_pop(&gas.free_spu_bufs);
    if (gas.inst_list[inst].spu_bufr == -1)
    {
      printf("UGLY MESSAGE: Couldn't get an spu buffer.  Queue contents:\n");
      gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_spu_bufs.start, gas.free_spu_bufs.end, gas.free_spu_bufs.count);
      for (i=0; i<gas.free_spu_bufs.queue_max; ++i)
      {
        gas_printf("%d ", gas.free_spu_bufs.queue[i]);
      }
      gas_printf("\n");
    }

    if (_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 0) == -1)
    {
      // abort
      gas_debug_check_value("INIT SPU STREAMING ABORT 2L", gas.inst_list[inst].spu_bufl, 0, gas.spu_buffer_num);
      gas_debug_check_value("INIT SPU STREAMING ABORT 2R", gas.inst_list[inst].spu_bufr, 0, gas.spu_buffer_num);
      fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
      fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufr);
      gas.inst_list[inst].curr_iop_buf_addr -= SPU_BUFFER_HALF;
      return -1;
    }
    gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;
  }

  // preload the second half of the spu buffer
  _gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 1);
  gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;

  if (gas.source_list[source].flag.stereo)
  {
    _gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 1);
    gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;
  }
  
  // set up the critical address to the end of side 0, for the left voice
  // the critical address is used to determine if we can start loading the other buffer
  gas.inst_list[inst].spu_buf_crit = (unsigned char *)SPU_MEMORY_TOP + 
    (gas.inst_list[inst].spu_bufl * SPU_BUFFER_SIZE) + SPU_BUFFER_HALF;

  gas.inst_list[inst].flag.spu_buf_side = 0;
  // Set up the reload of side 1
//  gas.inst_list[inst].spu_reload = RELOAD_FLAG_SIDE1;
//  fifo_queue_push(&gas.reload_spu, inst);
  return 0;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P R E L O A D   S P U   F R O M   I O P
//
// dma the first batch of stream data from the iop to the spu, should happen before play happens
int _gas_rpc_preload_spu_from_iop(unsigned char *iop_buf_addr, int spu_buf_num, int spu_buf_side)
{
  unsigned char * spu_buf_addr = (unsigned char *)SPU_MEMORY_TOP + (spu_buf_num * SPU_BUFFER_SIZE)
    + (spu_buf_side * SPU_BUFFER_HALF);

  RECORDRA;
  if (spu_buf_side == 0)
  {
    _AdpcmSetMarkSTART(iop_buf_addr, SPU_BUFFER_HALF);
  }
  else
  {
    _AdpcmSetMarkEND(iop_buf_addr, SPU_BUFFER_HALF);
  }
//gas_printf("iop preload side %d iop addr 0x%x spu addr 0x%p\n", spu_buf_side, iop_buf_addr, spu_buf_addr);
  return _gas_rpc_dma_iop_to_spu_blocking(iop_buf_addr, spu_buf_addr, SPU_BUFFER_HALF);
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P A U S E   S P U   I N S T A N C E
//
// Pauses, unpauses and stops an spu-hosted sound, based on the pause-mode.
// pauses (will honor a pause guard) with a +1 pause_mode, unpauses with a -1 
// with a 0 pause_mode it stops and recycles the instance
int _gas_rpc_pause_spu_instance(int inst, int pause_mode)
{ 
  // Assumed semaphore locks: system_state_sema
  int prev_count = gas.inst_list[inst].pause_count;
  int nax;

  RECORDRA;
  // check for a stop command
  if (pause_mode == MODE_STOP)
  {
    _gas_update_recycle_instance(inst);
    return 1;
  }
  else if (pause_mode == MODE_GUARD)
  {
    // set up a pause guard
    gas.inst_list[inst].pause_count = -1;
    return 1;
  }

  // do pause or unpause command
  gas.inst_list[inst].pause_count += pause_mode;
  if (gas.inst_list[inst].pause_count < 0)
    gas.inst_list[inst].pause_count = 0;

  if (prev_count > 0 && gas.inst_list[inst].pause_count == 0)
  {
  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KON , 0x1<<gas.inst_list[inst].flag.voicel);
  }
  else if (prev_count <= 0 && gas.inst_list[inst].pause_count > 0) 
  {
    nax = gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
#ifdef DEBUG_OUTPUT
gas_printf("Pause Target - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corel,
  gas.inst_list[inst].flag.voicel, nax);
#endif
    sceSdSetAddr(gas.inst_list[inst].flag.corel|SD_VA_SSA|(gas.inst_list[inst].flag.voicel<<1), nax );

  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);
  }
  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P A U S E   C D   I N S T A N C E
//
// pauses with a +1 pause_mode, unpauses with a -1, will honor a pause guard
// with a 0 pause_mode it stops and recycles the instance
int _gas_rpc_pause_cd_instance(int inst, int pause_mode)
{ 
  // Assumed semaphore locks: system_state_sema
  int prev_count = gas.inst_list[inst].pause_count;

  RECORDRA;
  // check for a stop command
  if (pause_mode == MODE_STOP)
  {
#ifdef DEBUG_OUTPUT
gas_printf("Recycle due to stop\n");
#endif
    return _gas_update_recycle_instance(inst);
  }
  else if (pause_mode == MODE_GUARD)
  {
    // set up a pause guard
    gas.inst_list[inst].pause_count = -1;
    return 1;
  }

  // do pause or unpause command
  gas.inst_list[inst].pause_count += pause_mode;
  if (gas.inst_list[inst].pause_count < 0)
    gas.inst_list[inst].pause_count = 0;

  if (prev_count > 0 && gas.inst_list[inst].pause_count == 0)
  {
    _gas_rpc_streamed_voice_controller(inst, SD_S_KON, 0);
  }
  else if (prev_count <= 0 && gas.inst_list[inst].pause_count > 0) 
  {
    _gas_rpc_streamed_voice_controller(inst, SD_S_KOFF, 1);
  }
  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   I N I T   N E W   I N S T A N C E
//
// Initializes a new instance
int _gas_rpc_init_new_instance(int inst, int source)
{
  // Assumed semaphore locks: system_state_sema
  int i, j, v;
  RECORDRA;
  gas.inst_list[inst].pitch = gas.source_list[source].pitch_one;

  // volumes for this instance
  gas.inst_list[inst].voll = gas.source_list[source].voll;
  gas.inst_list[inst].volr = gas.source_list[source].volr;
  gas.inst_list[inst].rvoll = gas.source_list[source].rvoll;
  gas.inst_list[inst].rvolr = gas.source_list[source].rvolr;

  gas.inst_list[inst].target_voll = gas.source_list[source].voll;
  gas.inst_list[inst].target_volr = gas.source_list[source].volr;
  gas.inst_list[inst].target_rvoll = gas.source_list[source].rvoll;
  gas.inst_list[inst].target_rvolr = gas.source_list[source].rvolr;

  gas.inst_list[inst].pause_count = 1;  // start out initially paused
  gas.inst_list[inst].dampen_count = 0;

  gas.inst_list[inst].spu_reload = RELOAD_FLAG_NO_RELOAD;
  gas.inst_list[inst].iop_reload = RELOAD_FLAG_NO_RELOAD;

  // only used in non-spu instances
  gas.inst_list[inst].spu_buf_crit = NULL;
  gas.inst_list[inst].curr_iop_buf_addr = NULL;
  gas.inst_list[inst].last_iop_buf_addr = NULL;
  gas.inst_list[inst].spu_bufl = -1;
  gas.inst_list[inst].spu_bufr = -1;
  gas.inst_list[inst].flag.spu_buf_side = 0;

  // only used in ee and cd streaming instances
  gas.inst_list[inst].flag.iop_buf_side = 0;
  gas.inst_list[inst].iop_buf = -1;

  gas.inst_list[inst].flag.src_type = gas.source_list[source].flag.src_type;
  gas.inst_list[inst].flag.loop = gas.source_list[source].flag.loop;
  gas.inst_list[inst].flag.stereo = gas.source_list[source].flag.stereo;
  gas.inst_list[inst].flag.used = 1;
  gas.inst_list[inst].flag.small_sound = 0;
  gas.inst_list[inst].flag.ready = 1;         // reset to 0 for cd instances, later
  gas.inst_list[inst].flag.needs_spu_preload = 0;
  gas.inst_list[inst].flag.volume_touched = 0;

  // init the source-type specific stuff that needs to happen before voices are locked
  switch (gas.source_list[source].flag.src_type)
  {
    case SRC_TYPE_SPU: break;

    case SRC_TYPE_CD:
#ifdef DEBUG_OUTPUT
      gas_printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
#endif
      if ( gas.free_spu_bufs.count < (gas.inst_list[inst].flag.stereo + 1) )
        return 0;
      if (gas.inst_list[inst].flag.stereo == 1)
      {
#ifdef DEBUG_OUTPUT
        gas_printf("\t\t\t\tfree stereo bufs %d\n", gas.free_cd_stereo_bufs.count);
#endif
        if (gas.free_cd_stereo_bufs.count < 1)
          return 0;

        gas.inst_list[inst].iop_buf = fifo_queue_pop(&gas.free_cd_stereo_bufs);
        if (gas.inst_list[inst].iop_buf == -1)
        {
          printf("UGLY MESSAGE(rpc): Couldn't get an iop cd stereo buffer.  Queue contents:\n");
          gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_stereo_bufs.start, gas.free_cd_stereo_bufs.end, gas.free_cd_stereo_bufs.count);
          for (i=0; i<gas.free_cd_stereo_bufs.queue_max; ++i)
          {
            gas_printf("%d ", gas.free_cd_stereo_bufs.queue[i]);
          }
          gas_printf("\n");
        }
      }
      else
      {
#ifdef DEBUG_OUTPUT
        gas_printf("\t\t\t\tfree mono bufs %d\n", gas.free_cd_mono_bufs.count);
#endif
        if (gas.free_cd_mono_bufs.count < 1)
          return 0;
        gas.inst_list[inst].iop_buf = fifo_queue_pop(&gas.free_cd_mono_bufs);

        if (gas.inst_list[inst].iop_buf == -1)
        {
          printf("UGLY MESSAGE(rpc): Couldn't get an iop cd mono buffer.  Queue contents:\n");
          gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_mono_bufs.start, gas.free_cd_mono_bufs.end, gas.free_cd_mono_bufs.count);
          for (i=0; i<gas.free_cd_mono_bufs.queue_max; ++i)
          {
            gas_printf("%d ", gas.free_cd_mono_bufs.queue[i]);
          }
          gas_printf("\n");
        }
      }
      break;

    default:
      return 0;
  }
#ifdef DEBUG_OUTPUT
  gas_printf("\t\t\t\tfree voices %d\n", gas.free_voices.count);
#endif

  // reclaim any unused voices
  _gas_update_reclaim_voices();

  // search for an open voice
  if (_gas_rpc_get_voice(&i, &j) == 0) 
  {
    gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
    _gas_debug_voice_status_dump();
  }
  gas.inst_list[inst].flag.corel = i;
  gas.inst_list[inst].flag.voicel = j;
v = (i * PS2_MAX_VOICES_PER_CORE) + j;
if (gas.voice_used[v] != -1)
{
  gas_printf("Hey man, this voice is already used (%d) by 0x%x\n", v, gas.voice_used[v]);
#ifdef HALT_ON_FAIL
  while(1);
#endif
}
if (gas.inst_list[inst].instance_id < 0)
{
  gas_printf("What's the story here?  This guy has a negative id 0x%x\n", inst);
#ifdef HALT_ON_FAIL
  while(1);
#endif
}

gas.voice_used[v] = gas.inst_list[inst].instance_id;

  // pick a r-channel voice for stereo sources
  if (gas.source_list[source].flag.stereo)
  {
    if (_gas_rpc_get_voice(&i, &j) == 0)  
    {
      gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
      _gas_debug_voice_status_dump();
    }
    gas.inst_list[inst].flag.corer = i;
    gas.inst_list[inst].flag.voicer = j;
v = (i * PS2_MAX_VOICES_PER_CORE) + j;
if (gas.voice_used[v] != -1)
{
  gas_printf("Hey man, this voice is already used (%d) by 0x%x\n", v, gas.voice_used[v]);
#ifdef HALT_ON_FAIL
  while(1);
#endif
}
gas.voice_used[v] = gas.inst_list[inst].instance_id;
  }
  else
  {
    gas.inst_list[inst].flag.voicer = VNUM_NO_VOICE;
    gas.inst_list[inst].flag.corer = 0; 
  }

  // init the source-type specific stuff for this instance
  switch (gas.source_list[source].flag.src_type)
  {
    case SRC_TYPE_SPU: 
      gas.inst_list[inst].src.spu.cur_addr = gas.source_list[source].src.spu.addr;
      break;

    case SRC_TYPE_CD:
      gas.inst_list[inst].flag.ready = 0;
      fifo_queue_push(&gas.cd_preloads_pending, inst);
      break;

    default:
      return 0;
  }

  return 1;
}

