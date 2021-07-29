// GAS system-control functions
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"

//#define NAX_ME_BABY

// ================================================================
// Global data

// class members
GasSystemState gas;

sceCdRMode gSceCdRMode;

#define UPDATE_STREAMS_INTERVAL_USEC (10 * 1000) // 10 microseconds (every 1/100 of a second)
struct SysClock g_clock;

void _gas_check_spu_streaming_needs();
void _gas_update_iop_to_spu_streams();
void _gas_update_cd_to_iop_streams();
unsigned int _gas_update_streams(void *common);


// ================================================================
// dispatch-able functions

/*** gas_init ***/
// initializes the gas, to undo this, call gas_shutdown.  Both functions should only
// be called once per run of the application, and gas_reset calls should be used
// to do a 'soft' reset of the sound system (because a reset is less costly than
// a shutdown & re-init, that's why reset even exists, yo).
void
gas_init (int spu_buffers, char *list_filename, int file_size, int proview_mode)
{
  int size;

  if (proview_mode == 1)
  {
    gas.host_prefix = "snfile:";
    gas.proview_mode = 1;
  }
  else
  {
    gas.host_prefix = "host0:";
    gas.proview_mode = 0;
  }

  // initialize the cd system
  sceSdInit(0);
//  sceCdInit(SCECdINIT);

  // set up the CD reading mode
  gSceCdRMode.trycount = 0;
  gSceCdRMode.spindlctrl = SCECdSpinNom;
  gSceCdRMode.datapattern = SCECdSecS2048;

  //    Disk media: CD
  // Output format: PCM
  //    Copy guard: normal (one generation recordable / default)
  sceSdSetCoreAttr (SD_C_SPDIF_MODE, (SD_SPDIF_MEDIA_CD |
				SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL));

  gas.source_list = NULL;
  gas.spu_buffer_num = spu_buffers;
  size = (MAX_CD_STEREO_STREAMS * CD_STEREO_BUFFER_SIZE) + (MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE);
  gas.iop_buffers_top = (unsigned char *)AllocSysMemory(0, size, NULL);
  gas.iop_cd_mono_buffers_top = gas.iop_buffers_top + (MAX_CD_STEREO_STREAMS * CD_STEREO_BUFFER_SIZE);

  memset(gas.iop_buffers_top, 0, size);

  _gas_init_system_data();

  // Initialize the various queues (Allocates memory, which is freed in shutdown)
  fifo_queue_init(&gas.free_voices, PS2_MAX_VOICES);
  fifo_queue_init(&gas.free_instances, PS2_MAX_VOICES);
  fifo_queue_init(&gas.free_cd_stereo_bufs, MAX_CD_STEREO_STREAMS);
  fifo_queue_init(&gas.free_cd_mono_bufs, MAX_CD_MONO_STREAMS);
  fifo_queue_init(&gas.free_spu_bufs, spu_buffers);
  fifo_queue_init(&gas.reload_spu, spu_buffers);
  fifo_queue_init(&gas.reload_cd_streams, MAX_CD_STREAMS);
  fifo_queue_init(&gas.cd_preloads_pending, MAX_CD_STREAMS);
  
  gas_reset(list_filename, file_size);

  USec2SysClock(UPDATE_STREAMS_INTERVAL_USEC, &g_clock);
  SetAlarm(&g_clock, _gas_update_streams, NULL);
}


//*** _gas_init_system_data ***/
// initializes data in the GasSystemState structure
void _gas_init_system_data()
{
  int i;

  gas.inst_list_count = 0;
  for (i=0; i<PS2_MAX_VOICES; ++i)
    gas.inst_list[i].instance_id = i;

  gas.preloading_instance = -1;

  // reserve stream buffers in the spu
  gas.spu_heap_start = SPU_MEMORY_TOP + (SPU_BUFFER_SIZE * gas.spu_buffer_num);
  gas.spu_heap_end = SPU_MEMORY_MAX;
  gas.spu_heap_curr = gas.spu_heap_start;
}


/*** gas_reset ***/
// a soft reset of the system, no partial shutdown call is required prior to calling
// this function, because reset calls that on its own.
// Performs a soft reset on the gas and should be done every level.
int gas_reset (char *list_filename, int file_size)
{
  char i;

  // clean up old stuff
  _gas_partial_shutdown();
  _gas_init_system_data();

  // debugging only
  _gas_spu_mem_clear();

  for( i = 0; i < 2; i++ )
	{
		sceSdSetParam( i|SD_P_MVOLL , 0x3fff ) ;
		sceSdSetParam( i|SD_P_MVOLR , 0x3fff ) ;
	}

  // clear out the queues
  fifo_queue_clear(&gas.free_instances);
  fifo_queue_clear(&gas.reload_spu);
  fifo_queue_clear(&gas.reload_cd_streams);
  fifo_queue_clear(&gas.cd_preloads_pending);

  // These queues are expected to be loaded with stuff, so pre-load them
  fifo_queue_clear(&gas.free_voices);
  for (i=0; i<gas.free_voices.queue_max; ++i)
    fifo_queue_push(&gas.free_voices, i);

  fifo_queue_clear(&gas.free_cd_stereo_bufs);
  for (i=0; i<gas.free_cd_stereo_bufs.queue_max; ++i)
    fifo_queue_push(&gas.free_cd_stereo_bufs, i);

  fifo_queue_clear(&gas.free_cd_mono_bufs);
  for (i=0; i<gas.free_cd_mono_bufs.queue_max; ++i)
    fifo_queue_push(&gas.free_cd_mono_bufs, i);

  fifo_queue_clear(&gas.free_spu_bufs);
  for (i=0; i<gas.free_spu_bufs.queue_max; ++i)
    fifo_queue_push(&gas.free_spu_bufs, i);

  // stall for the CD system before we read the list file
  sceCdDiskReady(0);

  _gas_load_list_file(list_filename, file_size, &gas.source_list, &gas.source_list_count);

  return 1;
}


/*** gas_shutdown ***/
// undoes what init and reset does.  should only be called when shutting down the app,
// use reset to do a soft reset, that's what it's for.
void
gas_shutdown (void)
{
  _gas_partial_shutdown();

  // free the memory allocated for each queue
  fifo_queue_free(&gas.free_voices);
  fifo_queue_free(&gas.free_cd_mono_bufs);
  fifo_queue_free(&gas.free_cd_stereo_bufs);
  fifo_queue_free(&gas.free_spu_bufs);
  fifo_queue_free(&gas.reload_spu);
  fifo_queue_free(&gas.free_instances);
  fifo_queue_free(&gas.reload_cd_streams);

  if (gas.iop_buffers_top != NULL)
    FreeSysMemory(gas.iop_buffers_top);

  // shut down the sound and cd systems
  sceSdBlockTrans( 0, SD_TRANS_MODE_STOP, NULL, 0 );
  sceSdSetCoreAttr (SD_CORE_0 | SD_C_IRQ_ENABLE, 0);
	sceSdSetSwitch( SD_CORE_0|SD_S_KOFF , 0xFFFFFF );
  sceCdStStop();

  return;
}

void _gas_remove_sources(void);


/*** _gas_partial_shutdown ***/
// called by reset and shutdown to free up any resources that are level specific.  Flushes
// iop and spu sounds and buffers, and basically shuts down all that needs to be shutdown
// for a reset to happen correctly.
void _gas_partial_shutdown()
{
  if (gas.source_list != NULL)
    FreeSysMemory(gas.source_list);

  _gas_remove_sources();
}


/*** _gas_remove_iop_source ***/
// be careful calling this function, if an instance still exists that uses it, bad things
// could happen.
void _gas_remove_iop_source(int which)
{
  if (gas.source_list[which].src.iop.iop_buf_addr != NULL)
    FreeSysMemory((void *)gas.source_list[which].src.iop.iop_buf_addr);
  gas.source_list[which].src.iop.iop_buf_addr = NULL;
  gas.source_list[which].flag.loaded = 0;
}


/*** _gas_remove_sources ***/
// free anything created by the sources
void _gas_remove_sources(void)
{
  int i;
  for (i=0; i<gas.source_list_count; ++i)
  {
    switch (gas.source_list[i].flag.src_type)
    {
      case SRC_TYPE_SPU: // you don't remove spu sources this way, we clear them in bulk
        break;
      case SRC_TYPE_EE:  break;
      case SRC_TYPE_IOP: 
        _gas_remove_iop_source(i);
        break;
      case SRC_TYPE_CD:
#if 0
        if (gas.source_list[i].flag.host_disk)
        {
          gas.source_list[i].flag.host_disk = 0;
          close(gas.source_list[i].src.cd.fd);
          gas.source_list[i].src.cd.fd = -1;
        }
#endif
        break;
      default:
        break;
    }
  }
}


/*** gas_spu_mem_dump ***/
// Dumps the contents of spu memory to disk, for debugging
void gas_spu_mem_dump()
{
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
gas_printf("dumping 0x%x thru 0x%x\n", addr, addr+readsize);
    // write it to the dump file
    n = write(fd, buffer, readsize);
    if (n != readsize)
      gas_printf("Yikes!\n");
  }
  close(fd);
  FreeSysMemory(buffer);
}


/*** _gas_spu_mem_clear ***/
// also for debugging, clears out spu memory, so that we don't get garbage from previous
// runs in spu dumps.  A clear of some sort will probably be good to do even on a release
// build.
void _gas_spu_mem_clear()
{
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


/*** _gas_update_streams ***/
// Checks the streaming data to see if anything needs updating and does that updating.
// it wakes up many times per second to do its thing.
unsigned int _gas_update_streams(void *common)
{
  _gas_process_cd_preloads();
  _gas_check_spu_streaming_needs();
  _gas_update_iop_to_spu_streams();
  _gas_update_cd_to_iop_streams();

  return g_clock.low;
}


/*** _gas_check_spu_streaming_needs ***/
// checks all instances to see who needs more data dma'ed to the spu
void _gas_check_spu_streaming_needs()
{
  unsigned char *nax;
  int inst;
  int we_did_stuff = 0;

  // check all playing streams to determine if any of them need new data
  for (inst=0; inst<gas.inst_list_count; ++inst)
  {
    if (gas.inst_list[inst].flag.used == 1)
    { 
      if (gas.commands_waiting == 1)
      {
        int checkme = (gas.command_list[inst].instance_id & GAS_INSTANCE_ID_MASK);
        we_did_stuff = 1;
        if (checkme == inst)
        {
          if (gas.command_list[inst].set_volume == 1)
          {
            gas_instance_volume(gas.command_list[inst].instance_id, gas.command_list[inst].volume_left, 
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
        if ((gas.inst_list[inst].spu_reload == RELOAD_FLAG_RECYCLE_ME))
        {
          gas.inst_list[inst].spu_reload = RELOAD_FLAG_DEAD;
          _gas_reclaim_voices();
          break;
        }

        // check NAX to determine if this sound has reached the end of the current spu-buf-half
        nax = (unsigned char *)sceSdGetAddr(gas.inst_list[inst].flag.corel|SD_VA_NAX|(gas.inst_list[inst].flag.voicel<<1));
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
//  gas_printf("DMA ch%d iop 0x%p to spu 0x%p, size %d\n", prim, 
//    gas.inst_list[which].curr_iop_buf_addr, spu_addr, SPU_BUFFER_HALF);

  sceSdVoiceTrans( prim, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
    (u_char*)(gas.inst_list[which].curr_iop_buf_addr), (u_char *)spu_addr, 
    (u_int)SPU_BUFFER_HALF );
  gas.inst_list[which].curr_iop_buf_addr += SPU_BUFFER_HALF;
//gas_printf("dma finished\n");
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
      while(1) gas_printf("Error.\n");
      
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
//gas_printf("iop reload side %d iop addr 0x%p spu addr 0x%p\n", gas.inst_list[inst].spu_reload, gas.inst_list[inst].curr_iop_buf_addr, addr);
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
//gas_printf("iop reload side %d iop addr 0x%p spu addr 0x%p (r)\n", gas.inst_list[inst].spu_reload, gas.inst_list[inst].curr_iop_buf_addr, addr);
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

    load_to = gas.inst_list[inst].iop_buf_addr[load_side];

gas_printf("Instance %d, loadside/readside %d/%d  load amount %d\n", inst, load_side, gas.inst_list[inst].flag.iop_buf_side, sectors_to_load);

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
