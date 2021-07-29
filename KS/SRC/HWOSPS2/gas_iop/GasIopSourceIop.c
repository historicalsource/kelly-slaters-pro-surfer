/*
 * Code to support iop-resident sources 
 */
#include <kernel.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"

extern GasSystemState gas;

void _gas_streamed_voice_controller(int inst, int mode, int store_pos_flag);


/*** _gas_add_iop_source ***/
// uploads a sound to iop memory
int _gas_add_iop_source(int entry)
{
	int size;
	char *wavBuffer;

  // check if there is room for this source
	if( (wavBuffer = _gas_load_file(gas.source_list[entry].full_filename, &size)) == NULL )
    return 0;

  if (size != gas.source_list[entry].size)
  {
    gas_printf("Warning, the disk size(%d) and the entry size(%d) do not match for %s\n",
      size, gas.source_list[entry].size, gas.source_list[entry].full_filename);
    gas.source_list[entry].size = size;
  }

  gas.source_list[entry].src.iop.iop_buf_addr = wavBuffer;
  gas.source_list[entry].flag.loaded = 1;

  return 1;
}


/*** _gas_play_iop_instance ***/
// begins playback of an iop-based source
void _gas_play_iop_instance(int inst)
{
  unsigned addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * SPU_BUFFER_SIZE);
  _gas_set_voice (inst, 0, addr);
  if (gas.inst_list[inst].flag.stereo)
  {
    addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufr * SPU_BUFFER_SIZE);
    _gas_set_voice (inst, 1, addr);
  }
  _gas_streamed_voice_controller(inst, SD_S_KON, 0);
}



/*** _gas_pause_iop_instance ***/
// pauses with a +1 pause_mode, unpauses with a -1, will honor a pause guard
// with a 0 pause_mode it stops and recycles the instance
int _gas_pause_iop_instance(int inst, int pause_mode)
{ 
  int prev_count = gas.inst_list[inst].pause_count;

  // check for a stop command
  if (pause_mode == 0)
  {
    // make it so that reclaim will pick this instance up and recycle it
    gas.inst_list[inst].pause_count = 0;
    gas.inst_list[inst].flag.loop = 0;
    gas.inst_list[inst].spu_reload = RELOAD_FLAG_RECYCLE_ME;

    _gas_streamed_voice_controller(inst, SD_S_KOFF, 0);
  
    _gas_reclaim_voices();
    return 1;
  }

  // do pause or unpause command
  gas.inst_list[inst].pause_count += pause_mode;
  if (gas.inst_list[inst].pause_count < 0)
    gas.inst_list[inst].pause_count = 0;

  if (prev_count > 0 && gas.inst_list[inst].pause_count == 0)
  {
    _gas_streamed_voice_controller(inst, SD_S_KON, 0);
  }
  else if (prev_count <= 0 && gas.inst_list[inst].pause_count > 0) 
  {
    _gas_streamed_voice_controller(inst, SD_S_KOFF, 1);
  }
  return 1;
}


/*** _gas_streamed_voice_controller ***/
// Encapsulates the sceSd calls to turn a sound on or off, supports stereo, as well as
// stopping (and removing instance) and pausing (restart at paused location) support
// for on and off requests
//
// inst is the instance to use, mode is the SCE voice command (either SD_S_KON or SD_S_KOFF)
// and store_pos_flag is 0 for most calls, but when non-zero and the mode is SD_S_KOFF, it
// sets the start position to the current position for pausing restart support.
void _gas_streamed_voice_controller(int inst, int mode, int store_pos_flag)
{
  int nax;
  sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);

  if (store_pos_flag && (mode == SD_S_KOFF))
  {
    nax = sceSdGetAddr(gas.inst_list[inst].flag.corel|SD_VA_NAX|(gas.inst_list[inst].flag.voicel<<1));
gas_printf("Pause Target - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corel,
  gas.inst_list[inst].flag.voicel, nax);
    sceSdSetAddr(gas.inst_list[inst].flag.corel|SD_VA_SSA|(gas.inst_list[inst].flag.voicel<<1), nax );
    if (gas.inst_list[inst].flag.stereo == 1)
    {
      nax = sceSdGetAddr(gas.inst_list[inst].flag.corer|SD_VA_NAX|(gas.inst_list[inst].flag.voicer<<1));
gas_printf("Pause Target(r) - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corer,
  gas.inst_list[inst].flag.voicer, nax);
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
}


int _gas_dma_iop_to_spu_nonblocking(unsigned char *iop_addr, unsigned char *spu_addr, int size)
{
  unsigned char *addr;
  char pri_ch = -1;

  if (size >= UPLOAD_SIZE)
  {
    gas_printf("ERROR.  What are you doing, calling the nonblocking version with a size of %d?!?!  Trying to recover.\n",
      size);
    return _gas_dma_iop_to_spu_blocking(iop_addr, spu_addr, size);
  }

	// --- data transfer
	gas_printf("Data transfer ... (nonblocking)\n" );

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

  gas_printf("TSA0 = %x \n", sceSdGetAddr( pri_ch|SD_A_TSA ) );
  gas_printf("Total sent %d\n", size );
  return size;
}

/*** _gas_preload_spu_from_iop ***/
// dma the first batch of stream data from the iop to the spu, should happen before play happens
int _gas_preload_spu_from_iop(unsigned char *iop_buf_addr, int spu_buf_num, int spu_buf_side)
{
  unsigned char * spu_buf_addr = (unsigned char *)SPU_MEMORY_TOP + (spu_buf_num * SPU_BUFFER_SIZE) + 
    (spu_buf_side * SPU_BUFFER_HALF);

  _AdpcmSetMarkSTART (iop_buf_addr, SPU_BUFFER_HALF);
//gas_printf("iop preload side %d iop addr 0x%x spu addr 0x%p\n", spu_buf_side, iop_buf_addr, spu_buf_addr);
  return _gas_dma_iop_to_spu_blocking(iop_buf_addr, spu_buf_addr, SPU_BUFFER_HALF);
}


/*** _gas_init_spu_streaming ***/
// use carefully, curr_iop_buf_addr and last_iop_buf_addr should be initialized correctly
// for the source type before calling
int _gas_init_spu_streaming(int inst, int source)
{
  // grab a free spu buffer and preload it
  gas.inst_list[inst].spu_bufl = fifo_queue_pop(&gas.free_spu_bufs);
  if (_gas_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 0) == -1)
  {
    // abort
    fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
    return -1;
  }
  gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;

  if (gas.source_list[source].flag.stereo)
  {
    // grab a free spu buffer and preload it
    gas.inst_list[inst].spu_bufr = fifo_queue_pop(&gas.free_spu_bufs);
    if (_gas_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 0) == -1)
    {
      // abort
      fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
      fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufr);
      gas.inst_list[inst].curr_iop_buf_addr -= SPU_BUFFER_HALF;
      return -1;
    }
    gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;
  }
  
  // set up the critical address to the end of side 0, for the left voice
  // the critical address is used to determine if we can start loading the other buffer
  gas.inst_list[inst].spu_buf_crit = (unsigned char *)SPU_MEMORY_TOP + 
    (gas.inst_list[inst].spu_bufl * SPU_BUFFER_SIZE) + SPU_BUFFER_HALF;

  // Set up the reload of side 1
  gas.inst_list[inst].flag.spu_buf_side = 1;
  gas.inst_list[inst].spu_reload = RELOAD_FLAG_SIDE1;
  fifo_queue_push(&gas.reload_spu, inst);
  return 0;
}


/*** _gas_init_iop_instance ***/
// does the initialization of an iop instance that is specific to an iop instance
int _gas_init_iop_instance(int inst, int source)
{
  int ret;

  // set up the iop buffer positions, last_buf_addr is the last buffer (either 1 sector for
  // mono or 2 for stereo) that we upload before looping or shutting down the instance (oneshot)
  gas.inst_list[inst].curr_iop_buf_addr = gas.source_list[source].src.iop.iop_buf_addr;
  gas.inst_list[inst].last_iop_buf_addr = gas.source_list[source].size + 
    gas.source_list[source].src.iop.iop_buf_addr - 
    ((gas.inst_list[inst].flag.stereo + 1) * SPU_BUFFER_HALF);

  ret = _gas_init_spu_streaming(inst, source);

  return 1;
}
