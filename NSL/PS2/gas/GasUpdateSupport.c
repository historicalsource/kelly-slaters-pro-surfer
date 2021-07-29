// GAS update thread
#include <kernel.h>
#include <libcdvd.h>
#include <stdio.h>
#include <sys/file.h>
#include "../gas.h" 
#include "GasSystem.h"

static int ret_val_cd;
static int g_error_condition = 0;
static int g_try_count = 0;
#define MAX_DISK_OP_TRIES 5
#define RETRY_LOOP_ENTRY g_try_count = 0; do{
#define RETRY_LOOP_WHILE(cond) g_error_condition = ((cond)); g_try_count++; }while( ( g_try_count < MAX_DISK_OP_TRIES ) && g_error_condition );

extern sceCdRMode g_sce_cdrmode;
extern GasStatus gas_status;
#define CHANGED_NONE  0x00
#define CHANGED_LEFT  0x01
#define CHANGED_RIGHT 0x02
#define CHANGED_BOTH  (CHANGED_LEFT | CHANGED_RIGHT)


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   V O L U M E
//
// called from RPC and UPDATE threads
// interpolates the actual volume level to the target volume level
// to avoid SPU 'pops' and 'clicks'.
void _gas_update_volume(int inst)
{
  // Assumed semaphore locks: system_state_sema
  short voll_delta;
  short volr_delta;
  short rvoll_delta = 0;
  short rvolr_delta = 0;
  short voll_temp;
  short volr_temp;
#ifdef DEBUG
  static int debug_me_baby = 0;
  static int debug_inst = -1;
#endif

  short changed;

#ifdef DEBUG
  if ((debug_me_baby > 0) && (inst == debug_inst)) 
  {
    debug_me_baby--;
    _gas_debug_voice_status_dump_instance(inst);
  }
#endif
  RECORDRA
  // ramp volume to correct level
  if (gas.inst_list[inst].dampen_count > 0)
  {
    if (gas.inst_list[inst].flag.stereo)
    {
      rvoll_delta = (gas.inst_list[inst].target_rvoll*gas.dampen_level)/GAS_COMMAND_DAMPEN_ONE - gas.inst_list[inst].rvoll;
      rvolr_delta = (gas.inst_list[inst].target_rvolr*gas.dampen_level)/GAS_COMMAND_DAMPEN_ONE - gas.inst_list[inst].rvolr;
    }
    voll_delta = (gas.inst_list[inst].target_voll*gas.dampen_level)/GAS_COMMAND_DAMPEN_ONE - gas.inst_list[inst].voll;
    volr_delta = (gas.inst_list[inst].target_volr*gas.dampen_level)/GAS_COMMAND_DAMPEN_ONE - gas.inst_list[inst].volr;  
  }
  else 
  {
    if (gas.inst_list[inst].flag.stereo)
    {
      rvoll_delta = gas.inst_list[inst].target_rvoll - gas.inst_list[inst].rvoll;
      rvolr_delta = gas.inst_list[inst].target_rvolr - gas.inst_list[inst].rvolr;
    }
    voll_delta = gas.inst_list[inst].target_voll - gas.inst_list[inst].voll;
    volr_delta = gas.inst_list[inst].target_volr - gas.inst_list[inst].volr;  
  }
  if (voll_delta == 0 && volr_delta == 0 && rvoll_delta == 0 && rvolr_delta == 0)
  {
    // clear the dirty flag, this sound is clean as a whistle
    gas.inst_list[inst].flag.volume_touched = 0;

    // if we were fading this volume out to stop the sound, do the rest of the stop now
    if ( gas.inst_list[inst].flag.stopping == 1 )
    {
      gas.inst_list[inst].flag.stopping = 0;
      gas_stop_queued( inst );
    }
  }
  else
  {
    // clamp the volume deltas to safe levels
    if (_gas_check_inst_playing(inst))
    {
      voll_temp = voll_delta;
      volr_temp = volr_delta;
      changed = CHANGED_NONE;

      // volume clamping with proportionate adjustment to the other channel
      if (voll_delta >  GAS_SAFE_VOLUME_DELTA)      { changed |= CHANGED_LEFT;  voll_delta =  GAS_SAFE_VOLUME_DELTA; }
      else if (voll_delta < -GAS_SAFE_VOLUME_DELTA) { changed |= CHANGED_LEFT;  voll_delta = -GAS_SAFE_VOLUME_DELTA; }
      if (volr_delta >  GAS_SAFE_VOLUME_DELTA)      { changed |= CHANGED_RIGHT; volr_delta =  GAS_SAFE_VOLUME_DELTA; }
      else if (volr_delta < -GAS_SAFE_VOLUME_DELTA) { changed |= CHANGED_RIGHT; volr_delta = -GAS_SAFE_VOLUME_DELTA; }

      // careful!  Watch the logic, it's optimized, and not explicitly clear
      if (changed == CHANGED_BOTH && (voll_temp - voll_delta > volr_temp - volr_delta))
      {
        changed = CHANGED_LEFT;
      }
      if (changed & CHANGED_RIGHT && volr_temp != 0)
        voll_delta = (voll_temp * volr_delta) / volr_temp;
      else if (changed & CHANGED_LEFT && voll_temp != 0)
        volr_delta = (volr_temp * voll_delta) / voll_temp;
    }

    // send the update volumes
    gas.inst_list[inst].voll += voll_delta; 
    gas.inst_list[inst].volr += volr_delta;

    // dampen the values before passing them to the spu
 /*   if (gas.inst_list[inst].dampen_count > 0)
    {
      voll_temp = (gas.inst_list[inst].voll * gas.dampen_level) / GAS_COMMAND_DAMPEN_ONE;
      volr_temp = (gas.inst_list[inst].volr * gas.dampen_level) / GAS_COMMAND_DAMPEN_ONE;
    }
    else 
    {
      voll_temp = gas.inst_list[inst].voll;
      volr_temp = gas.inst_list[inst].volr;
    }
*/
    voll_temp = gas.inst_list[inst].voll;
    volr_temp = gas.inst_list[inst].volr;

    // update the spu
    gas_update_set_voice_volume (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
      voll_temp, volr_temp);

    if (gas.inst_list[inst].flag.stereo)
    {
      if (_gas_check_inst_playing(inst))
      {
        voll_temp = rvoll_delta;
        volr_temp = rvolr_delta;
        changed = CHANGED_NONE;

        // volume clamping with proportionate adjustment to the other channel
        if (rvoll_delta >  GAS_SAFE_VOLUME_DELTA)      { changed |= CHANGED_LEFT;  rvoll_delta =  GAS_SAFE_VOLUME_DELTA; }
        else if (rvoll_delta < -GAS_SAFE_VOLUME_DELTA) { changed |= CHANGED_LEFT;  rvoll_delta = -GAS_SAFE_VOLUME_DELTA; }
        if (rvolr_delta >  GAS_SAFE_VOLUME_DELTA)      { changed |= CHANGED_RIGHT; rvolr_delta =  GAS_SAFE_VOLUME_DELTA; }
        else if (rvolr_delta < -GAS_SAFE_VOLUME_DELTA) { changed |= CHANGED_RIGHT; rvolr_delta = -GAS_SAFE_VOLUME_DELTA; }

        // careful!  Watch the logic, it's optimized, and not explicitly clear
        if (changed == CHANGED_BOTH && (voll_temp - rvoll_delta > volr_temp - rvolr_delta))
        {
          changed = CHANGED_LEFT;
        }
        if (changed & CHANGED_RIGHT && volr_temp != 0)
          rvoll_delta = (voll_temp * rvolr_delta) / volr_temp;
        else if (changed & CHANGED_LEFT && voll_temp != 0)
          rvolr_delta = (volr_temp * rvoll_delta) / voll_temp;
      }

      // send the update volumes
      gas.inst_list[inst].rvoll += rvoll_delta; 
      gas.inst_list[inst].rvolr += rvolr_delta; 

      // dampen the values before passing them to the spu
  /*    if (gas.inst_list[inst].dampen_count > 0)
      {
        voll_temp = (gas.inst_list[inst].rvoll * gas.dampen_level) / GAS_COMMAND_DAMPEN_ONE;
        volr_temp = (gas.inst_list[inst].rvolr * gas.dampen_level) / GAS_COMMAND_DAMPEN_ONE;
      }
      else 
      {
        voll_temp = gas.inst_list[inst].rvoll;
        volr_temp = gas.inst_list[inst].rvolr;
      }
*/
      voll_temp = gas.inst_list[inst].rvoll;
      volr_temp = gas.inst_list[inst].rvolr;

      // update the spu
      gas_update_set_voice_volume (gas.inst_list[inst].flag.corer, gas.inst_list[inst].flag.voicer, 
        voll_temp, volr_temp);
    }
  }
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   I N I T   I O P   S T R E A M I N G
//
void _gas_update_init_iop_streaming(int inst, int source)
{
  // Assumed semaphore locks: system_state_sema
  unsigned char *load_to;
  int rawStreamHostFd;
  int sectors_to_load, left;
  int i;
  int sectors, divs, load_size;
  int nchunk, nchunk_sectors;
//  int offset;
  int push_it = 0;
RECORDRA
  // grab a free iop buffer and preload it
  if (gas.inst_list[inst].flag.stereo)
  {
    if (gas.inst_list[inst].iop_buf == -1)
    {
      printf("UGLY MESSAGE: Couldn't get an iop cd stereo buffer.  Queue contents:\n");
      gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_stereo_bufs.start, gas.free_cd_stereo_bufs.end, gas.free_cd_stereo_bufs.count);
      for (i=0; i<gas.free_cd_stereo_bufs.queue_max; ++i)
      {
        gas_printf("%d ", gas.free_cd_stereo_bufs.queue[i]);
      }
      gas_printf("\n");
    }
    gas.inst_list[inst].iop_buf_addr[0] = gas.iop_buffers_top + 
      (gas.inst_list[inst].iop_buf * CD_STEREO_BUFFER_SIZE);
    nchunk = CD_STEREO_BUFFER_N_CHUNK;
    nchunk_sectors = CD_STEREO_BUFFER_SECTORS_N_CHUNK;
  }
  else
  {
    if (gas.inst_list[inst].iop_buf == -1)
    {
      printf("UGLY MESSAGE: Couldn't get an iop cd mono buffer.  Queue contents:\n");
      gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_mono_bufs.start, gas.free_cd_mono_bufs.end, gas.free_cd_mono_bufs.count);
      for (i=0; i<gas.free_cd_mono_bufs.queue_max; ++i)
      {
        gas_printf("%d ", gas.free_cd_mono_bufs.queue[i]);
      }
      gas_printf("\n");
    }

    gas.inst_list[inst].iop_buf_addr[0] = gas.iop_cd_mono_buffers_top + 
      (gas.inst_list[inst].iop_buf * CD_MONO_BUFFER_SIZE);
    nchunk = CD_MONO_BUFFER_N_CHUNK;
    nchunk_sectors = CD_MONO_BUFFER_SECTORS_N_CHUNK;
  }

  gas.inst_list[inst].iop_load_amount[0] = 0;
  for (i=1; i<IOP_N_BUFFER; ++i)
  {
    gas.inst_list[inst].iop_buf_addr[i] = gas.inst_list[inst].iop_buf_addr[i-1] + nchunk;
    gas.inst_list[inst].iop_load_amount[i] = 0;
  }
  gas.inst_list[inst].curr_iop_buf_addr = gas.inst_list[inst].iop_buf_addr[0];
  
  if (gas.inst_list[inst].offset == 0)
    gas.inst_list[inst].src.cd.curr_sector = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].src.cd.start_sector;
  else
    gas.inst_list[inst].src.cd.curr_sector = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].src.cd.start_sector + gas.inst_list[inst].offset;

  gas.inst_list[inst].src.cd.sectors_remaining = (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].padsize/ PS2_CD_SECTOR_SIZE) - gas.inst_list[inst].offset ;

  // set up the load increment
  sectors = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].padsize / PS2_CD_SECTOR_SIZE;

  divs = sectors / nchunk_sectors;
  if (sectors % nchunk_sectors != 0 || divs == 0)
  {
    ++divs;
  }
  load_size = sectors / divs;
#ifdef DEBUG_OUTPUT
  gas_printf("raw loadsize is %d ", load_size);
#endif
  if (sectors % divs != 0)
  {
    ++load_size;
#ifdef DEBUG_OUTPUT
  gas_printf("+1 (sectors vs divs) ");
#endif
  }
  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo && ((load_size % 2) != 0))
  {
    // need an even number to load.
    ++load_size;
#ifdef DEBUG_OUTPUT
  gas_printf("+1 (make it even) ");
#endif
  }
  gas.inst_list[inst].iop_target_load_amount = load_size;
#ifdef DEBUG_OUTPUT
gas_printf("= %d\n", load_size);
#endif
    if (gas.inst_list[inst].flag.stereo && load_size % 2 != 0)
    {
      gas_printf("Ah dang\n");
#ifdef HALT_ON_FAIL
      while(1);
#endif
    }
  // set up preload size
  gas.inst_list[inst].flag.iop_buf_side = 0;
  if (gas.inst_list[inst].src.cd.sectors_remaining >= gas.inst_list[inst].iop_target_load_amount)
  {
    sectors_to_load = gas.inst_list[inst].iop_target_load_amount;
    for (i=0; i<IOP_N_BUFFER; ++i)
    {
      gas.inst_list[inst].iop_load_amount[i] = gas.inst_list[inst].iop_target_load_amount;
    }
    gas.inst_list[inst].iop_reload = RELOAD_FLAG_SIDE1;
    push_it = 1;
  }
  else
  {
    sectors_to_load = gas.inst_list[inst].src.cd.sectors_remaining;
    for (left = sectors_to_load, i=0; i<IOP_N_BUFFER; ++i)
    {
      if (left >= gas.inst_list[inst].iop_target_load_amount)
      {
        gas.inst_list[inst].iop_load_amount[i] = gas.inst_list[inst].iop_target_load_amount;
        left -= gas.inst_list[inst].iop_target_load_amount;
      }
      else
      {
        gas.inst_list[inst].iop_load_amount[i] = left;
        left = 0;
      }
    }

    // don't do any more streaming
    gas.inst_list[inst].iop_reload = RELOAD_FLAG_NO_RELOAD;
    gas.inst_list[inst].flag.small_sound = 1;
  }

  load_to = gas.inst_list[inst].curr_iop_buf_addr;

  rawStreamHostFd = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].rawStreamHostFd;
  if( rawStreamHostFd != -1 )
  {
     lseek( rawStreamHostFd, gas.inst_list[inst].src.cd.curr_sector * PS2_CD_SECTOR_SIZE, SEEK_SET );
     read( rawStreamHostFd, load_to, sectors_to_load * PS2_CD_SECTOR_SIZE );
  }
  else if( gas.host_streaming_fd < 0 )
  {
    sceCdSync(0);
    RETRY_LOOP_ENTRY;
    ret_val_cd = sceCdRead(gas.inst_list[inst].src.cd.curr_sector, sectors_to_load, load_to, &g_sce_cdrmode);
    RETRY_LOOP_WHILE( ret_val_cd <= 0 );
  }
  else
  {
    lseek( gas.host_streaming_fd, gas.inst_list[inst].src.cd.curr_sector * PS2_CD_SECTOR_SIZE, SEEK_SET );
    read( gas.host_streaming_fd, load_to, sectors_to_load * PS2_CD_SECTOR_SIZE );
  }

  gas.inst_list[inst].last_iop_buf_addr =
    gas.inst_list[inst].iop_buf_addr[0] + 
    (gas.inst_list[inst].iop_load_amount[0] * PS2_CD_SECTOR_SIZE) - 
    ((gas.inst_list[inst].flag.stereo + 1) * SPU_BUFFER_HALF);

  gas.inst_list[inst].src.cd.curr_sector += sectors_to_load;
  gas.inst_list[inst].src.cd.sectors_remaining -= sectors_to_load;
  if (gas.inst_list[inst].iop_reload < -1)
    gas_printf("That's extremely not right! ******************************\n");
  if (push_it) 
  {
#ifdef DEBUG_OUTPUT
    gas_printf("push %d\n", inst);
#endif
    fifo_queue_push(&gas.reload_cd_streams, inst);
  }
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   M A R K   U P   D A T A   F O R   S P U
//
void _gas_update_mark_up_data_for_spu(int which)
{
  RECORDRA
  // Assumed semaphore locks: system_state_sema
  if (gas.inst_list[which].spu_reload == RELOAD_FLAG_SIDE0)
  { 
    _AdpcmSetMarkSTART (gas.inst_list[which].curr_iop_buf_addr, SPU_BUFFER_HALF); 
  }
  else if (gas.inst_list[which].spu_reload == RELOAD_FLAG_SIDE1)
  { 
    _AdpcmSetMarkEND   (gas.inst_list[which].curr_iop_buf_addr, SPU_BUFFER_HALF);
  }
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   D O   I O P   T O   S P U   T R A N S F E R
//
void _gas_update_do_iop_to_spu_transfer(int prim, int which, unsigned char *spu_addr)
{
  // Assumed semaphore locks: system_state_sema
//  gas_printf("DMA ch%d iop 0x%p to spu 0x%p, size %d\n", prim, 
//    gas.inst_list[which].curr_iop_buf_addr, spu_addr, SPU_BUFFER_HALF);
RECORDRA
  sceSdVoiceTrans( prim, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
    (u_char*)(gas.inst_list[which].curr_iop_buf_addr), (u_char *)spu_addr, 
    (u_int)SPU_BUFFER_HALF );
  gas.inst_list[which].curr_iop_buf_addr += SPU_BUFFER_HALF;
//gas_printf("dma finished\n");
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   I N S T A N C E   V O L U M E
//
// set the target volumes
void _gas_update_instance_volume( int inst, short voll, short volr )
{
  RECORDRA
  // Assumed semaphore locks: system_state_sema
  if ((gas.inst_list[inst].flag.stereo == 0))
  {
    gas.inst_list[inst].target_voll = voll;
    gas.inst_list[inst].target_volr = volr;
  }
  else
  { 
    if (gas.stereo) 
    {
      gas.inst_list[inst].target_voll = voll;
      gas.inst_list[inst].target_volr = 0;
      gas.inst_list[inst].target_rvoll = 0;
      gas.inst_list[inst].target_rvolr = volr;
    } 
    else 
    {
      gas.inst_list[inst].target_voll = voll;
      gas.inst_list[inst].target_volr = voll;
      gas.inst_list[inst].target_rvoll = volr;
      gas.inst_list[inst].target_rvolr = volr;
    }
  }
  gas.inst_list[inst].flag.volume_touched = 1;
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   S E T   V O I C E   V O L U M E
//
void gas_update_set_voice_volume (int core, int voice, int voll, int volr)
{
  RECORDRA
  sceSdSetParam (core | (voice<<1) | SD_VP_VOLL, voll & 0x7fff );
	sceSdSetParam (core | (voice<<1) | SD_VP_VOLR, volr  & 0x7fff);
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   S E T   V O I C E   P I T C H
//
void gas_update_set_voice_pitch (int core, int voice, int pitch)
{
  RECORDRA
  sceSdSetParam (core | (voice<<1) | SD_VP_PITCH, pitch );
}

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   R E C Y C L E   I N S T A N C E   I F   F I N I S H E D
//
// returns 1 if it killed the instance, 0 otherwise
int _gas_check_inst_playing( unsigned short inst )
{
  return ((gas.inst_list[inst].flag.used == 1) && 
          (gas.inst_list[inst].pause_count < 1) && 
          (gas.inst_list[inst].played == 1));
}

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   R E C Y C L E   I N S T A N C E   I F   F I N I S H E D
//
// returns 1 if it killed the instance, 0 otherwise
int _gas_update_recycle_instance_if_finished( unsigned short v )
{
  // Assumed semaphore locks: system_state_sema
  int endflags;
  unsigned int voice;
  int hitme;
  int ret_val = 0;
	int nax;
	int naxed_out=0;
  RECORDRA
  // check if this instance is still playing, and whether it has active flags set, etc
  if (_gas_check_inst_playing(v) &&  
      (gas.inst_list[v].flag.loop == 0))
  {
    endflags = sceSdGetSwitch(gas.inst_list[v].flag.corel|SD_S_ENDX);

    // We have to determine if a voice is playing differently for spu and non-spu hosted
    // sounds.  'hitme' stores the result either way
    if (gas.inst_list[v].flag.src_type == SRC_TYPE_SPU)
    {
      voice = gas.inst_list[v].flag.voicel;
      voice = 1 << voice;
      hitme = endflags & voice;
			naxed_out=1;
    }
    else
		{

      hitme = (gas.inst_list[v].spu_reload == RELOAD_FLAG_DEAD);
			if (hitme)
			{

				nax = gas_update_get_NAX(gas.inst_list[v].flag.corel, (gas.inst_list[v].flag.voicel<<1));

				// We want to make sure the sound playing in the spu finishes the last 
				// little bit left in the buffer
				// The nax loops in the last 0x10 after the buffer half
				// So we check for it here
				//  --KES 06/21/02
				if (gas.inst_list[v].flag.spu_buf_side == 1)
				{
					if ((nax <= (int)gas.inst_list[v].spu_buf_crit) && (nax >= ((int)gas.inst_list[v].spu_buf_crit - 0x10)))
					{
						naxed_out=1;
					}
				}
				else
				{
					if ((nax <= ((int)gas.inst_list[v].spu_buf_crit +0x800)) && (nax >= (((int)gas.inst_list[v].spu_buf_crit - 0x10 + 0x800))))
					{
						naxed_out=1;
					}
				}
			}
		}

		// do we stop this?
    if (hitme && naxed_out)
    {
#ifdef DEBUG_OUTPUT
printf("Reclaiming inst 0x%x %s\n", gas.inst_list[v].instance_id, gas.source_list[gas.inst_list[v].source].filename);
#endif
			//gas_pre_stop_queued(v);
      _gas_update_recycle_instance(v);
      ret_val = 1;
    }
  }
  return ret_val;
}

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   R E C L A I M   V O I C E S
//
// checks to see if we can recycle any of the instances and their voices
int _gas_update_reclaim_voices()
{
  // Assumed semaphore locks: system_state_sema
  unsigned short v;

  RECORDRA
  for (v=0; v<gas.inst_list_count; ++v)
  {
    // check if this instance is still playing, and whether it has active flags set, etc
    _gas_update_recycle_instance_if_finished(v);
  }

  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   R E C Y C L E   I N S T A N C E
//
int _gas_update_recycle_instance(int inst)
{
  // Assumed semaphore locks: system_state_sema
  GasInstance *this = &gas.inst_list[inst];
  int v;
RECORDRA
  if (this->flag.used == 1)
  {
#ifdef DEBUG_OUTPUT
gas_printf("Begin recycle of instance %d\n", inst);
#endif
    // remove this instance from any async queues
    if ( fifo_queue_find(&gas.reload_spu, inst, 0) )
    {
#ifdef DEBUG
      gas_printf("\nWoah, that could have been bad. (0x%x)\n", inst);
      _gas_debug_voice_status_dump_instance(inst);
      _gas_debug_voice_status_dump_instance_aux(inst);
      fifo_queue_print(&gas.reload_spu);
#endif
      fifo_queue_find(&gas.reload_spu, inst, 1);
#ifdef DEBUG
      fifo_queue_print(&gas.reload_spu);
#endif
    }
    if ( fifo_queue_find(&gas.reload_cd_streams, inst, 0) )
    {
#ifdef DEBUG
      gas_printf("\nWoah, that could have been very bad. (0x%x)\n", inst);
      _gas_debug_voice_status_dump_instance(inst);
      _gas_debug_voice_status_dump_instance_aux(inst);
      fifo_queue_print(&gas.reload_cd_streams);
#endif
      fifo_queue_find(&gas.reload_cd_streams, inst, 1);
#ifdef DEBUG
      fifo_queue_print(&gas.reload_cd_streams);
#endif
    }
    if ( fifo_queue_find(&gas.cd_preloads_pending, inst, 0) )
    {
#ifdef DEBUG
      gas_printf("\nWoah, that could have been uber bad. (0x%x)\n", inst);
      _gas_debug_voice_status_dump_instance(inst);
      _gas_debug_voice_status_dump_instance_aux(inst);
      fifo_queue_print(&gas.cd_preloads_pending);
#endif
      fifo_queue_find(&gas.cd_preloads_pending, inst, 1);
#ifdef DEBUG
      fifo_queue_print(&gas.cd_preloads_pending);
#endif
    }
    if ( gas.preloading_instance == inst )
    {
#ifdef DEBUG
      gas_printf("\nWoah, that could have been crazy bad. (0x%x)\n", inst);
      _gas_debug_voice_status_dump_instance(inst);
#endif
//      gas.preloading_instance = -1;
    }
    else
    {
#ifdef DEBUG
      gas_debug_check_value("FREE INSTANCES PUSH", inst, 0, GAS_MAX_INSTANCES);
#endif
      _gas_release_instance_slot(inst);
      //fifo_queue_push(&gas.free_instances, inst);
    }

//    _gas_rpc_streamed_voice_controller(inst, SD_S_KOFF, 0);

v = (this->flag.corel * PS2_MAX_VOICES_PER_CORE) + this->flag.voicel;
if (gas.voice_used[v] != this->instance_id)
{
  gas_printf("Hey man, this voice is used by somebody else (%d). I am 0x%x, he is 0x%x\n", v, this->instance_id, gas.voice_used[v]);
#ifdef HALT_ON_FAIL
  while(1);
#endif
}
gas.voice_used[v] = -1;
    _gas_update_recycle_voice(this->instance_id&GAS_INSTANCE_ID_MASK, this->flag.corel, this->flag.voicel);
    this->flag.corel = 0;
    this->flag.voicel = VNUM_NO_VOICE;

    if (gas.inst_list[inst].spu_bufl >= 0 && gas.inst_list[inst].spu_bufl < gas.spu_buffer_num)
      fifo_queue_push_front(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl); 
    this->spu_bufl = 0xdd;

    if (this->flag.stereo == 1)
    {
v = (this->flag.corer * PS2_MAX_VOICES_PER_CORE) + this->flag.voicer;
if (gas.voice_used[v] != this->instance_id)
{
  gas_printf("Hey man, this voice is used by somebody else (%d). I am 0x%x, he is 0x%x\n", v, this->instance_id, gas.voice_used[v]);
#ifdef HALT_ON_FAIL
  while(1);
#endif
}
gas.voice_used[v] = -1;
      _gas_update_recycle_voice(this->instance_id&GAS_INSTANCE_ID_MASK,this->flag.corer, this->flag.voicer);
      this->flag.corer = 0;
      this->flag.voicer = VNUM_NO_VOICE;

      if (gas.inst_list[inst].spu_bufr >= 0 && gas.inst_list[inst].spu_bufr < gas.spu_buffer_num)
        fifo_queue_push_front(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufr); 
    }
    this->spu_bufr = 0xdd;

    if (gas.inst_list[inst].iop_buf >= 0)
    {
      if (this->flag.src_type != SRC_TYPE_CD)
        gas_printf("What the hell?  This non-cd instance has an iop buffer?\n");

      if (gas.inst_list[inst].flag.stereo)
        fifo_queue_push_front(&gas.free_cd_stereo_bufs, gas.inst_list[inst].iop_buf);
      else
        fifo_queue_push_front(&gas.free_cd_mono_bufs, gas.inst_list[inst].iop_buf);
    }
    this->iop_buf = 0xdd;

    this->pause_count = 1;
    this->spu_reload = 0xdd; // garbage value
    this->iop_reload = 0xdd; // garbage value

    // only used in non-spu instances
    this->spu_buf_crit = (unsigned char *)0xdeadf00d;
    this->curr_iop_buf_addr = (unsigned char *)0xdeadf00d;
    this->last_iop_buf_addr = (unsigned char *)0xdeadf00d;

    this->iop_buf_addr[0] = (unsigned char *)0xdeadf00d;
    this->iop_buf_addr[1] = (unsigned char *)0xdeadf00d;

    this->iop_load_amount[0] = 0xdeadf00d;
    this->iop_load_amount[1] = 0xdeadf00d;
    this->iop_target_load_amount = (unsigned char)0xdeadf00d;

    // src_type specific stuff
    switch (this->flag.src_type)
    {
      case SRC_TYPE_CD:
        this->src.cd.curr_sector = 0xdeadf00d;
        this->src.cd.sectors_remaining = 0xdeadf00d;
        break;
      case SRC_TYPE_SPU:
        this->src.spu.cur_addr = 0xdeadf00d;
        break;
    }

    this->flag.used = 0;
    gas_status.instances[inst].is_valid = 0;
#ifdef DEBUG_OUTPUT
gas_printf("End recycle of instance %d\n", inst);
#endif

    return 1;
  }
  return 0;
}

void fifo_queue_print(fifo_queue *fq)
{
  int i;
  gas_printf("Queue: s%d e%d c%d/%d: ", fq->start, fq->end, fq->count, fq->queue_max);
  for (i=0; i<fq->queue_max; ++i)
  {
    if (i >= fq->start && i <= fq->end && fq->count > 0)
      gas_printf("[%d]", fq->queue[i]);
    else
      gas_printf(" %d ", fq->queue[i]);
  }
  gas_printf("\n");
}

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   R E C Y C L E   V O I C E
//
void _gas_update_recycle_voice(int inst, int core, int voice)
{
  // Assumed semaphore locks: system_state_sema
  RECORDRA
#ifdef DEBUG_OUTPUT
gas_printf("Freeing core %d voice %d\n", core, voice);
#endif
  gas_update_set_voice_pitch( core, voice, 0);
  gas_update_set_voice_volume( core, voice, 0, 0);
  sceSdSetSwitch( core|SD_S_KOFF , 0x1<<voice);
#ifdef DEBUG
  gas_debug_check_value("FREE VOICE", voice, 0, PS2_MAX_VOICES_PER_CORE);
#endif
  // free its voice
  _gas_release_voice(inst, ((core * PS2_MAX_VOICES_PER_CORE) + voice));
  //fifo_queue_push_front(&gas.free_voices, ((core * PS2_MAX_VOICES_PER_CORE) + voice));
}



//-----------------------------------------------------------------------------------
// G A S   U P D A T E   G E T   N A X
//
unsigned gas_update_get_NAX(int core,int ch)
{
  unsigned pos,pos2,pos3;
  int counter =0;
  RECORDRA
	while(1)
	{
		pos=sceSdGetAddr(core|ch|SD_VA_NAX);
		pos2=sceSdGetAddr(core|ch|SD_VA_NAX);
		pos3=sceSdGetAddr(core|ch|SD_VA_NAX);

		if (pos == pos2)
			return(pos);
		else if (pos2 == pos3)
			return(pos2);
    if (counter == 1000000)
    {
      gas_printf("Getting NAX looping too much!\n");
      counter =0;
    }
	}
}