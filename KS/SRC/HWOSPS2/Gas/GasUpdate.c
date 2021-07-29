// GAS update thread
#include <kernel.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas.h" 
#include "GasSystem.h"

extern struct SysClock g_clock;
extern GasStatus gas_status;  // EE
extern sceCdRMode g_sce_cdrmode;
extern int g_update_sema;

void _gas_update_add_instance_requests();
void _gas_update_check_spu_streaming_needs();
void _gas_update_process_cd_preloads();
void _gas_update_iop_to_spu_streams();
void _gas_update_cd_to_iop_streams();

void _gas_update_add_instance( int source_id );

void gas_debug_check_instances();

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   T H R E A D
//
// Does any updating required, has exclusive access to iop resources: CD and DMA
// it wakes up many times per second to do its thing.
unsigned int gas_update_thread(void *common)
{
  // make sure we have the resources we need
  while (1)
  {
    RECORDRA;
    WaitSema(g_update_sema);
    RECORDRA;

    if (PollSema(gas.system_state_sema) == KE_OK)
    {
      gas_debug_check_instances();
  //    _gas_update_add_instance_requests();  // not used yet
      RECORDRA;
      _gas_update_process_cd_preloads();
      RECORDRA;
      _gas_update_check_spu_streaming_needs();
      RECORDRA;
      _gas_update_iop_to_spu_streams();
      RECORDRA;
      _gas_update_cd_to_iop_streams();
      RECORDRA;

      gas_debug_check_instances();

      SignalSema(gas.system_state_sema);
      RECORDRA;

    }
  }

  return g_clock.low;
}

int quiet_down = 1;
int shut_down_voice = -1;
int shut_down_core = 0;


void gas_debug_check_instances()
{
  int i, nax;
  int kill_it;
  if (quiet_down == 0)
    gas_printf("cd streams active:\n");
  for (i=0; i<gas.inst_list_count; ++i)
  {
    kill_it = 0;
    if (gas.inst_list[i].flag.used == 1 && 
        gas.inst_list[i].flag.src_type == SRC_TYPE_CD &&
        gas.inst_list[i].pause_count < 1 && 
       (gas.inst_list[i].flag.needs_spu_preload == 0))
    {
      if (gas.inst_list[i].spu_buf_crit < (unsigned char *)0x5000)
      {
        printf("bad, very bad.\n");
        kill_it = 1;
      }
      nax = gas_update_get_NAX(gas.inst_list[i].flag.corel, (gas.inst_list[i].flag.voicel<<1));

if (quiet_down == 0)
  gas_printf("%s 0x%x - nax 0x%x vs crit 0x%p, bufside %d, reload %d\n", gas.source_list[gas.inst_list[i].source].short_filename,
           gas.inst_list[i].instance_id, nax, gas.inst_list[i].spu_buf_crit,
           gas.inst_list[i].flag.spu_buf_side, gas.inst_list[i].spu_reload);

      nax -= (int)gas.inst_list[i].spu_buf_crit;
      if (nax < 0)
        nax = -nax;
      if (nax > 0x800)
      {
        printf("uber bad.\n");
        kill_it = 1;
#ifdef HALT_ON_FAIL
        while(1) /* do nothing */;
#endif
      }
    }
    if (kill_it == 1)
    {
      _gas_update_recycle_instance(i);
    }
  }
  if (shut_down_voice != -1)
  {
  	sceSdSetSwitch( shut_down_core|SD_S_KOFF, (0x1<<shut_down_voice));
    shut_down_voice = -1;
  }
}

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   P R O C E S S   C D   P R E L O A D S
//
void _gas_update_process_cd_preloads()
{
  // Assumed semaphore locks: system_state_sema
  RECORDRA;
  if (sceCdSync(1) == 0)
  {
    RECORDRA;
    // if the cd is done, process cd preloads
    if (gas.preloading_instance != -1)
    {
      if (gas.inst_list[gas.preloading_instance].flag.used == 1)
      {
        gas.inst_list[gas.preloading_instance].flag.ready = 1;
        gas.inst_list[gas.preloading_instance].flag.needs_spu_preload = 1;
        gas.preloading_instance = -1;
      }
      else 
      {
        gas_debug_check_value("FREE INSTANCES PUSH", gas.preloading_instance, 0, GAS_MAX_INSTANCES);
        fifo_queue_push(&gas.free_instances, gas.preloading_instance);
        gas.preloading_instance = -1;
      }
    }

    if (gas.cd_preloads_pending.count > 0)
    { 
      gas.preloading_instance = fifo_queue_pop(&gas.cd_preloads_pending);
      if (gas.preloading_instance == -1)
        gas_printf("Big time error.  Why is this happening(1)?\n");

      if (gas.inst_list[gas.preloading_instance].flag.used == 1)
        _gas_update_init_iop_streaming(gas.preloading_instance, gas.inst_list[gas.preloading_instance].source);
      else
        gas.preloading_instance = -1;
    }
  }
  RECORDRA;
}

int nax_this = -1;
int nax_in_stereo = 0;

//-----------------------------------------------------------------------------------
// G A S   U P D A T E   C H E C K   S P U   S T R E A M I N G   N E E D S
//
// checks all instances to see who needs more data dma'ed to the spu
void _gas_update_check_spu_streaming_needs()
{
  // Assumed semaphore locks: system_state_sema
  unsigned char *nax;
  int inst;
  int we_did_stuff = 0;

  RECORDRA;
  // check all playing streams to determine if any of them need new data
  for (inst=0; inst<gas.inst_list_count; ++inst)
  {
    if (gas.inst_list[inst].flag.used == 1)
    { 
if (nax_this == inst)
{
  nax = (unsigned char *)gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
  gas_printf("nax(l) 0x%p vs crit 0x%p, bufside %d, reload %d\n", nax, gas.inst_list[inst].spu_buf_crit,
         gas.inst_list[inst].flag.spu_buf_side, gas.inst_list[inst].spu_reload);
  if (gas.inst_list[inst].flag.stereo && nax_in_stereo)
  {
    nax = (unsigned char *)gas_update_get_NAX(gas.inst_list[inst].flag.corer, (gas.inst_list[inst].flag.voicer<<1));
    gas_printf("nax(r) 0x%p vs crit 0x%p, bufside %d, reload %d\n", nax, gas.inst_list[inst].spu_buf_crit,
           gas.inst_list[inst].flag.spu_buf_side, gas.inst_list[inst].spu_reload);
  }
}
      if (gas.commands_waiting == 1)
      {
        int checkme = (gas.command_list[inst].instance_id & GAS_INSTANCE_ID_MASK);
        we_did_stuff = 1;
        if (checkme == inst)
        {
          if (gas.command_list[inst].set_volume == 1)
          {
            _gas_update_instance_volume(inst, gas.command_list[inst].volume_left, 
              gas.command_list[inst].volume_right);
          }
        }
      }      

      // process volume updates
      if (gas.inst_list[inst].flag.volume_touched == 1)
      {
        _gas_update_volume(inst);
      }

      if ((gas.inst_list[inst].pause_count < 1) && 
          (gas.inst_list[inst].flag.src_type != SRC_TYPE_SPU) && 
          (gas.inst_list[inst].spu_reload < 0) &&
          (gas.inst_list[inst].flag.ready == 1) &&
          (gas.inst_list[inst].flag.needs_spu_preload == 0))
      {
        if ((gas.inst_list[inst].spu_reload == RELOAD_FLAG_RECYCLE_ME) ||
            (gas.inst_list[inst].spu_reload == RELOAD_FLAG_DEAD))
        {
          gas.inst_list[inst].spu_reload = RELOAD_FLAG_DEAD;
          _gas_update_reclaim_voices();
          break;
        }

        // check NAX to determine if this sound has reached the end of the current spu-buf-half
        nax = (unsigned char *)gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
  #ifdef NAX_ME_BABY
  gas_printf("nax 0x%p vs crit 0x%p, bufside %d, reload %d\n", nax, gas.inst_list[inst].spu_buf_crit,
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


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   I O P   T O   S P U   S T R E A M S
//
// Updates any iop to spu streams that need to happen
void _gas_update_iop_to_spu_streams()
{
  // Assumed semaphore locks: system_state_sema
  int dma0_stat, dma1_stat;
  char last_one, mark_end;
  unsigned char *addr;
  int inst;
  int prim;

  RECORDRA;
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
    if (inst == -1)
      gas_printf("Big time error.  Why is this happening?(2)\n");
    prim = 0; 

    if (gas.inst_list[inst].flag.used == 0)
      continue;

    if (gas.inst_list[inst].spu_reload < 0)
    {
      gas_printf("Error.  Bad spu reload side value, something fishy's going on.\n");
      continue;
    }
      
    // determine the spu address
    addr = (unsigned char *)SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * 
      SPU_BUFFER_SIZE) + ((gas.inst_list[inst].spu_reload) * SPU_BUFFER_HALF);

    // check the iop addr to determine if we need to loop or stop dma-ing
    last_one = 0;
    mark_end = 0;
    if (gas.inst_list[inst].curr_iop_buf_addr >= gas.inst_list[inst].last_iop_buf_addr)
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
        _gas_update_mark_up_data_for_spu(inst);
      }
      last_one = 1;
    }
    else
    {
      _gas_update_mark_up_data_for_spu(inst);
    }
//gas_printf("iop reload side %d iop addr 0x%p spu addr 0x%p\n", gas.inst_list[inst].spu_reload, gas.inst_list[inst].curr_iop_buf_addr, addr);
    _gas_update_do_iop_to_spu_transfer(0, inst, addr);

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
        _gas_update_mark_up_data_for_spu(inst);
      }
      // determine the spu address for the right channel
      addr = (unsigned char *)SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufr * 
        SPU_BUFFER_SIZE) + ((gas.inst_list[inst].spu_reload) * SPU_BUFFER_HALF);

      // do the transfer
      // wait on the non-primary channel for stereo
//gas_printf("iop reload side %d iop addr 0x%p spu addr 0x%p (r)\n", gas.inst_list[inst].spu_reload, gas.inst_list[inst].curr_iop_buf_addr, addr);
      _gas_update_do_iop_to_spu_transfer(1, inst, addr);
    }

    if (last_one && (mark_end == 0))
    {
      // last one, loop the buffer back to the beginning
      if (gas.inst_list[inst].flag.src_type == SRC_TYPE_CD)
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

          if (gas.inst_list[inst].iop_reload < 0)
            gas_printf("That's not right! *             ** * *            *********************\n");
#ifdef DEBUG_OUTPUT
          gas_printf("PUSH %d\n", inst);
#endif
          if (fifo_queue_push(&gas.reload_cd_streams, inst) == 0)
            gas_printf("Error.  Disk stream queue push operation failed\n");
//gas_printf("pushed inst %d, count %d, iop_reload %d\n", inst, gas.reload_cd_streams.count, gas.inst_list[inst].iop_reload);
//gas_printf("Finished with side %d, flagging it for reloading\n", gas.inst_list[inst].iop_reload);
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


//-----------------------------------------------------------------------------------
// G A S   U P D A T E   C D   T O   I O P   S T R E A M S
//
// Updates any cd to iop streams that need to happen
void _gas_update_cd_to_iop_streams()
{
  // Assumed semaphore locks: system_state_sema
  int source;
  unsigned char *load_to;
  int sectors_to_load;
  int inst;
  int load_side;
  int sync = sceCdSync(1);
  RECORDRA;

  if ((sync == 0) && (gas.reload_cd_streams.count > 0))
  {
    // the cd is free, so lets start a stream
    inst = fifo_queue_pop(&gas.reload_cd_streams);
    if (inst == -1)
      gas_printf("Big time error.  Why is this happening?(3)\n");

    if (gas.inst_list[inst].flag.used == 0)
      return;

    load_side = gas.inst_list[inst].iop_reload;

    if (gas.inst_list[inst].iop_reload == -1)
    {
      gas_printf("Error.  CD streaming buffer underrun.\n");
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
    if (gas.inst_list[inst].flag.stereo && sectors_to_load % 2 != 0)
    {
      printf("Ah dang\n");
#ifdef HALT_ON_FAIL
      while(1);
#endif
    }

    load_to = gas.inst_list[inst].iop_buf_addr[load_side];

#ifdef DEBUG_OUTPUT
gas_printf("Instance %s(%d) loadside/readside %d/%d  load amount %d\n", gas.source_list[gas.inst_list[inst].source].short_filename, 
           inst, load_side, gas.inst_list[inst].flag.iop_buf_side, sectors_to_load);
#endif
    source = gas.inst_list[inst].source;

    sceCdSync(0);
    sceCdRead(gas.inst_list[inst].src.cd.curr_sector, sectors_to_load, load_to, &g_sce_cdrmode);

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


