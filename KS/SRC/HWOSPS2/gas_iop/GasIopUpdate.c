#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"

void _gas_process_cd_preloads();
void _gas_check_spu_streaming_and_command_list_needs();
void _gas_update_iop_to_spu_streams();
void _gas_update_cd_to_iop_streams();

#define UPDATE_STREAMS_INTERVAL_USEC (10 * 1000) // 10 microseconds (every 1/100 of a second)
extern struct SysClock g_clock;

extern sceCdRMode gSceCdRMode;

/*** _gas_update_streams ***/
// Checks the streaming data to see if anything needs updating and does that updating.
// it wakes up many times per second to do its thing.
unsigned int _gas_update_streams(void *common)
{
  _gas_process_cd_preloads();
  _gas_check_spu_streaming_and_command_list_needs();
  _gas_update_iop_to_spu_streams();
  _gas_update_cd_to_iop_streams();

  return g_clock.low;
}


/*** _gas_process_cd_preloads ***/
void _gas_process_cd_preloads()
{
  // if the cd is done, process cd preloads
  if (sceCdSync(1) == 0)
  {
    if (gas.preloading_instance != -1)
    {
      gas.inst_list[gas.preloading_instance].flag.ready = 1;
      gas.inst_list[gas.preloading_instance].flag.needs_spu_preload = 1;
      gas.preloading_instance = -1;
    }

    if (fifo_queue_size(&gas.cd_preloads_pending) > 0)
    { 
      gas.preloading_instance = fifo_queue_pop(&gas.cd_preloads_pending);
      _gas_init_iop_streaming(gas.preloading_instance, gas.inst_list[gas.preloading_instance].source);
    }
  }
}


/*** _gas_check_spu_streaming_and_command_list_needs ***/
// checks all instances to see who needs more data dma'ed to the spu
// and process all instances' commands pending
void _gas_check_spu_streaming_and_command_list_needs()
{
  unsigned char *nax;
  int inst;
  int we_did_stuff = 0;

  // check all playing streams to determine if any of them need new data
  for (inst=0; inst<gas.inst_list_count; ++inst)
  {
    if (gas.inst_list[inst].flag.used == 1)
    {
      // kick off delayed playback
      if (gas.inst_list[inst].flag.playback_pending == 1)
      {
        _gas_streamed_voice_controller(inst, SD_S_KON, 0);
        gas.inst_list[inst].flag.playback_pending = 0;
      }

      // process commands
      if (gas.commands_waiting == 1)
      {
        int checkme = (gas.command_list[inst].instance_id & GAS_INSTANCE_ID_MASK);
        we_did_stuff = 1;
        if (checkme == inst)
        {
          if (gas.command_list[inst].set_pitch == 1)
          {
            _gas_instance_pitch(inst, gas.command_list[inst].pitch);
          }
          if (gas.command_list[inst].set_volume == 1)
          {
            _gas_instance_volume(inst, gas.command_list[inst].volume_left, 
              gas.command_list[inst].volume_right);  // should also pass vols
          }

          switch (gas.command_list[inst].command)
          {
            case 0: /* no command */ break;

            case GAS_COMMAND_PLAY:     gas_status.rpc_retval = _gas_play_instance(inst); break;
            case GAS_COMMAND_STOP:     gas_status.rpc_retval = _gas_pause_instance(inst, 0); break;
            case GAS_COMMAND_PAUSE:    gas_status.rpc_retval = _gas_pause_instance(inst, 1); break;
            case GAS_COMMAND_UNPAUSE:  gas_status.rpc_retval = _gas_pause_instance(inst, -1); break;

            case GAS_COMMAND_DAMPEN:   printf("dampen command not supported yet\n"); break;
            case GAS_COMMAND_UNDAMPEN: printf("undampen command not supported yet\n"); break;
            case GAS_COMMAND_PAUSE_ALL:    printf("pause all command not supported yet\n"); break;
            case GAS_COMMAND_UNPAUSE_ALL:  printf("unpause all command not supported yet\n"); break;
            case GAS_COMMAND_DAMPEN_ALL:   printf("dampen all command not supported yet\n"); break;
            case GAS_COMMAND_UNDAMPEN_ALL: printf("undampen all command not supported yet\n"); break;
            default: printf("Unknown command %d\n", gas.command_list[inst].command); break;
          }
        }
      }      

      // check spu streaming needs
      if ((gas.inst_list[inst].pause_count < 1) && 
          (gas.inst_list[inst].flag.src_type != SRC_TYPE_SPU) && 
          (gas.inst_list[inst].spu_reload < 0) &&
          (gas.inst_list[inst].flag.ready == 1) &&
          (gas.inst_list[inst].flag.needs_spu_preload == 0))
      {
        if ((gas.inst_list[inst].spu_reload == RELOAD_FLAG_RECYCLE_ME))
        {
          gas.inst_list[inst].spu_reload = RELOAD_FLAG_DEAD;
          _gas_reclaim_voices();
          break;
        }

        // check NAX to determine if this sound has reached the end of the current spu-buf-half
        nax = (unsigned char *)sceSdGetAddr(gas.inst_list[inst].flag.corel|SD_VA_NAX|(gas.inst_list[inst].flag.voicel<<1));
  #ifdef NAX_ME_BABY
  printf("nax 0x%p vs crit 0x%p, bufside %d, reload %d\n", nax, gas.inst_list[inst].spu_buf_crit,
         gas.inst_list[inst].flag.spu_buf_side, gas.inst_list[inst].spu_reload);
  #endif

        if ((gas.inst_list[inst].flag.spu_buf_side == 0) && (nax >= gas.inst_list[inst].spu_buf_crit))
        {
          // flag this buffer for reload
          gas.inst_list[inst].spu_reload = RELOAD_FLAG_SIDE0;
          fifo_queue_push(&gas.reload_spu, inst);
        }
        else if ((gas.inst_list[inst].flag.spu_buf_side == 1) && (nax <= gas.inst_list[inst].spu_buf_crit) &&
          (nax >= (unsigned char *)SPU_MEMORY_TOP))
        {
          // flag this buffer for reload
          gas.inst_list[inst].spu_reload = RELOAD_FLAG_SIDE1;
          fifo_queue_push(&gas.reload_spu, inst);
        }
      }
    }

    // update status info
    gas_status.instances[inst].is_valid = gas.inst_list[inst].flag.used;
    gas_status.instances[inst].is_ready = gas.inst_list[inst].flag.used && gas.inst_list[inst].flag.ready;
    gas_status.instances[inst].is_playing = gas.inst_list[inst].flag.used && (gas.inst_list[inst].pause_count < 1);
  }
  if (we_did_stuff)
    gas.commands_waiting = 0;
}

/*** _gas_mark_up_data_for_spu ***/
void _gas_mark_up_data_for_spu(int which)
{
  if (gas.inst_list[which].spu_reload == RELOAD_FLAG_SIDE0)
  { 
    _AdpcmSetMarkSTART (gas.inst_list[which].curr_iop_buf_addr, SPU_BUFFER_HALF); 
  }
  else
  { 
    _AdpcmSetMarkEND   (gas.inst_list[which].curr_iop_buf_addr, SPU_BUFFER_HALF);
  }
}


/*** _gas_do_iop_to_spu_transfer ***/
void _gas_do_iop_to_spu_transfer(int prim, int which, unsigned char *spu_addr)
{
//  printf("DMA ch%d iop 0x%p to spu 0x%p, size %d\n", prim, 
//    gas.inst_list[which].curr_iop_buf_addr, spu_addr, SPU_BUFFER_HALF);

  sceSdVoiceTrans( prim, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
    (u_char*)(gas.inst_list[which].curr_iop_buf_addr), (int)spu_addr, 
    (u_int)SPU_BUFFER_HALF );
  gas.inst_list[which].curr_iop_buf_addr += SPU_BUFFER_HALF;
//printf("dma finished\n");
}


/*** _gas_update_iop_to_spu_streams ***/
// Updates any iop to spu streams that need to happen
void _gas_update_iop_to_spu_streams()
{
  int dma0_stat, dma1_stat;
  char last_one, mark_end;
  unsigned char *addr;
  int inst;
  int prim;

  // check if any spu buffers need reloading
  while (gas.reload_spu.count > 0)
  {
    dma0_stat = sceSdVoiceTransStatus(0, SD_TRANS_STATUS_CHECK );
    dma1_stat = sceSdVoiceTransStatus(1, SD_TRANS_STATUS_CHECK );

    // if both channels are being used, try again at the next update
    if ((dma0_stat == 0) || (dma1_stat == 0))
      break;

    // get the instance that needs spu refreshing
    inst = fifo_queue_pop(&gas.reload_spu);
    prim = 0; 

    if (gas.inst_list[inst].spu_reload < 0)
      while(1) printf("Error.\n");
      
    // determine the spu address
    addr = (unsigned char *)SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * 
      SPU_BUFFER_SIZE) + ((gas.inst_list[inst].spu_reload) * SPU_BUFFER_HALF);

    // check the iop addr to determine if we need to loop or stop dma-ing
    last_one = 0;
    mark_end = 0;
    if (gas.inst_list[inst].curr_iop_buf_addr >= gas.inst_list[inst].last_iop_buf_addr)
    {
      if (gas.inst_list[inst].flag.src_type == SRC_TYPE_IOP)
      {
        last_one = 1;

        if (gas.inst_list[inst].flag.loop)
        {
          _gas_mark_up_data_for_spu(inst);
        }
        else
        {
          // mark this next transfer block with an end block
          mark_end = 1;
          _AdpcmSetMarkSTOP((gas.inst_list[inst].curr_iop_buf_addr), SPU_BUFFER_HALF);
        }
      }
      else // ee or cd streaming
      {
        if ( gas.inst_list[inst].flag.src_type == SRC_TYPE_CD &&
             gas.inst_list[inst].iop_reload == RELOAD_FLAG_RECYCLE_ME && 
            (gas.inst_list[inst].src.cd.sectors_remaining == gas.inst_list[inst].iop_load_amount[gas.inst_list[inst].flag.iop_buf_side] ||
             gas.inst_list[inst].src.cd.sectors_remaining == 0))
        {
          mark_end = 1;
          _AdpcmSetMarkSTOP((gas.inst_list[inst].curr_iop_buf_addr), SPU_BUFFER_HALF);
        }
        else
        {
          _gas_mark_up_data_for_spu(inst);
        }
        last_one = 1;
      }
    }
    else
    {
      _gas_mark_up_data_for_spu(inst);
    }
//printf("iop reload side %d iop addr 0x%p spu addr 0x%p\n", gas.inst_list[inst].spu_reload, gas.inst_list[inst].curr_iop_buf_addr, addr);
    _gas_do_iop_to_spu_transfer(0, inst, addr);

    // if stereo, reload the right channel
    if (gas.inst_list[inst].flag.stereo)
    {
      if (last_one && mark_end)
      {
        // last block, mark it as a stop block
        _AdpcmSetMarkSTOP(gas.inst_list[inst].curr_iop_buf_addr, SPU_BUFFER_HALF);
      }
      else
      {
        // mark it as per usual
        _gas_mark_up_data_for_spu(inst);
      }
      // determine the spu address for the right channel
      addr = (unsigned char *)SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufr * 
        SPU_BUFFER_SIZE) + ((gas.inst_list[inst].spu_reload) * SPU_BUFFER_HALF);

      // do the transfer
      // wait on the non-primary channel for stereo
//printf("iop reload side %d iop addr 0x%p spu addr 0x%p (r)\n", gas.inst_list[inst].spu_reload, gas.inst_list[inst].curr_iop_buf_addr, addr);
      _gas_do_iop_to_spu_transfer(1, inst, addr);
    }

    if (last_one && (mark_end == 0))
    {
      // last one, loop the buffer back to the beginning
      if (gas.inst_list[inst].flag.src_type == SRC_TYPE_IOP)
        gas.inst_list[inst].curr_iop_buf_addr = gas.source_list[gas.inst_list[inst].source].src.iop.iop_buf_addr;
      else if (gas.inst_list[inst].flag.src_type == SRC_TYPE_CD)
      {
        if (gas.inst_list[inst].flag.small_sound)
        {
          if (gas.inst_list[inst].flag.loop)
          {
            // loop back to the beginning
            gas.inst_list[inst].curr_iop_buf_addr = gas.inst_list[inst].iop_buf_addr[0]; 
          }
          else
          {
            // small sound that has finished, mark it for removal
            mark_end = 1;
          }
        }
        else
        {
          // switch iop buffers and begin load on the one we just finished
          gas.inst_list[inst].iop_reload = gas.inst_list[inst].flag.iop_buf_side;

          if (fifo_queue_push(&gas.reload_cd_streams, inst) == 0)
            printf("Error.  Disk stream queue push operation failed\n");
//printf("pushed inst %d, count %d, iop_reload %d\n", inst, gas.reload_cd_streams.count, gas.inst_list[inst].iop_reload);
//printf("Finished with side %d, flagging it for reloading\n", gas.inst_list[inst].iop_reload);
          gas.inst_list[inst].flag.iop_buf_side++;
          if (gas.inst_list[inst].flag.iop_buf_side >= IOP_N_BUFFER)
            gas.inst_list[inst].flag.iop_buf_side = 0;
          gas.inst_list[inst].curr_iop_buf_addr = gas.inst_list[inst].iop_buf_addr[gas.inst_list[inst].flag.iop_buf_side]; 

          gas.inst_list[inst].last_iop_buf_addr =
            gas.inst_list[inst].curr_iop_buf_addr + 
            (gas.inst_list[inst].iop_load_amount[gas.inst_list[inst].flag.iop_buf_side] * PS2_CD_SECTOR_SIZE) -
            ((gas.inst_list[inst].flag.stereo + 1) * SPU_BUFFER_HALF);
        }
      }
    }

    gas.inst_list[inst].flag.spu_buf_side = 1 - gas.inst_list[inst].flag.spu_buf_side;
    if (mark_end == 1)
      gas.inst_list[inst].spu_reload = RELOAD_FLAG_RECYCLE_ME;
    else
      gas.inst_list[inst].spu_reload = RELOAD_FLAG_NO_RELOAD;
  }
}


/*** _gas_update_cd_to_iop_streams ***/
// Updates any cd to iop streams that need to happen
void _gas_update_cd_to_iop_streams()
{
  int source;
#if 0
  int offset;
#endif
  unsigned char *load_to;
  int sectors_to_load;
  int inst;
  int load_side;
  int sync = sceCdSync(1);

  if ((sync == 0) && (gas.reload_cd_streams.count > 0))
  {
    // the cd is free, so lets start a stream
    inst = fifo_queue_pop(&gas.reload_cd_streams);
    load_side = gas.inst_list[inst].iop_reload;

    if (gas.inst_list[inst].iop_reload == -1)
    {
      printf("Error.  CD streaming buffer underrun.\n");
      return;
    }

    if (gas.inst_list[inst].src.cd.sectors_remaining >= gas.inst_list[inst].iop_target_load_amount)
    {
      sectors_to_load = gas.inst_list[inst].iop_target_load_amount;
    }
    else if (gas.inst_list[inst].src.cd.sectors_remaining <= 0)
    {
      if (gas.inst_list[inst].flag.loop)
      {
        // loop back to the beginning
        gas.inst_list[inst].src.cd.curr_sector = gas.source_list[gas.inst_list[inst].source].src.cd.start_sector;
        gas.inst_list[inst].src.cd.sectors_remaining = gas.source_list[gas.inst_list[inst].source].size / PS2_CD_SECTOR_SIZE;
        sectors_to_load = gas.inst_list[inst].iop_target_load_amount;
      }
      else
      {
        // stop streaming
        gas.inst_list[inst].iop_reload = RELOAD_FLAG_RECYCLE_ME;
        return;
      }
    }
    else
    {
      // not a full buffer's worth
      sectors_to_load = gas.inst_list[inst].src.cd.sectors_remaining;

      if (gas.inst_list[inst].flag.loop)
      {
        // loop back to the beginning
        gas.inst_list[inst].iop_reload = RELOAD_FLAG_NO_RELOAD;
      }
      else
      {
        // stop streaming
        gas.inst_list[inst].iop_reload = RELOAD_FLAG_RECYCLE_ME;
      }
    }

    load_to = gas.inst_list[inst].iop_buf_addr[load_side];

printf("Instance %d, loadside/readside %d/%d  load amount %d\n", inst, load_side, gas.inst_list[inst].flag.iop_buf_side, sectors_to_load);

    source = gas.inst_list[inst].source;
#if 0
    if (gas.source_list[source].flag.host_disk == 1)
    {
      if (gas.source_list[source].src.cd.fd == -1)
      {
        if (_gas_add_cd_source(source,O_RDONLY|O_NBLOCK) != 1)
          return;
      }
      offset = gas.inst_list[inst].src.cd.curr_sector * PS2_CD_SECTOR_SIZE;
   	  lseek(gas.source_list[source].src.cd.fd, offset, 0);
      offset = read(gas.source_list[source].src.cd.fd, load_to, sectors_to_load * PS2_CD_SECTOR_SIZE);
    }
    else
    {
#endif
      sceCdSync(0);
      sceCdRead(gas.inst_list[inst].src.cd.curr_sector, sectors_to_load, load_to, &gSceCdRMode);
#if 0
    }
#endif
    gas.inst_list[inst].iop_load_amount[load_side] = (sectors_to_load);
    if (gas.inst_list[inst].iop_reload == load_side)
    {
      gas.inst_list[inst].iop_reload = RELOAD_FLAG_NO_RELOAD;
      gas.inst_list[inst].src.cd.curr_sector += sectors_to_load;
      gas.inst_list[inst].src.cd.sectors_remaining -= sectors_to_load;
    }
    else
    {
      if (gas.inst_list[inst].iop_reload != RELOAD_FLAG_RECYCLE_ME)
      {
        // loop back to the beginning
        gas.inst_list[inst].src.cd.curr_sector = gas.source_list[gas.inst_list[inst].source].src.cd.start_sector;
        gas.inst_list[inst].src.cd.sectors_remaining = gas.source_list[gas.inst_list[inst].source].size / PS2_CD_SECTOR_SIZE;
      }
    }
  }
}
