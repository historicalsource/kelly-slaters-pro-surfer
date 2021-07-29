/*
 * Code to support cd-streamed sources 
 *
 * host-disk streaming is not fully implemented
 */

#include <kernel.h>
#include <libcdvd.h>
#include <sys/file.h>
#include <string.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"

extern GasSystemState gas;
extern sceCdRMode gSceCdRMode;


/*** _gas_add_cd_source ***/
// uploads a sound to iop memory
int _gas_add_cd_source(int entry)
{
  char filename[64];
  int ret;
//	int size;

  sceCdlFILE cdfp;

  // find the file in the CD TOC
  strcpy(filename, "\\");
  strcat(filename, gas.source_list[entry].full_filename);

  gas_printf("Search Filename: %s\n", filename);
  sceCdDiskReady(0);
  ret= sceCdSearchFile(&cdfp, filename);

  if(!ret)
  {
    // read from host disk
    gas.source_list[entry].flag.host_disk = 1;
    gas_printf("sceCdSearchFile fail :%d\nThis stream will not be played.\n", ret);
#if 0
    gas_printf("sceCdSearchFile fail :%d\nAttempting to read from host disk\n", ret);

    strcpy(filename, gas.host_prefix);
    strcat(filename, gas.source_list[entry].full_filename);
    filename[strlen(filename)-2] = '\0';

    if ((gas.source_list[entry].src.cd.fd = open (filename, flags)) < 0) 
    {
      gas_printf("host disk file open failed. %s \n", filename);
      return 0;
    }

   	size = lseek(gas.source_list[entry].src.cd.fd, 0, 2);
   	if( size <= 0 ) 
    { 
      gas_printf( "\nCan't load VAG file to iop buffer\n" ); 
      return 0; 
    }
  	lseek( gas.source_list[entry].src.cd.fd, 0, 0);

    if (size != gas.source_list[entry].size)
    {
      gas_printf("Warning, the disk size(%d) and the entry size(%d) do not match for %s\n",
        size, gas.source_list[entry].size, filename);
      gas.source_list[entry].size = size;
    }
    gas.source_list[entry].src.cd.start_sector = 0;

    gas_printf("fd %d, source# %d\n", gas.source_list[entry].src.cd.fd, entry);
#else
    return 0;
#endif
  }
  else
  {
    // read from cd
    gas.source_list[entry].flag.host_disk = 0;

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

  return 1;
}


/*** _gas_play_cd_instance ***/
// begins playback of an cd-based source
void _gas_play_cd_instance(int inst)
{
  unsigned addr;

  if (gas.inst_list[inst].flag.ready) 
  {
    // make sure our spu upload is there.
    if (gas.inst_list[inst].flag.needs_spu_preload)
      _gas_init_spu_streaming(inst, gas.inst_list[inst].source);

    addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * SPU_BUFFER_SIZE);
    gas.inst_list[inst].flag.needs_spu_preload = 0;

    _gas_set_voice (inst, 0, addr);
    if (gas.inst_list[inst].flag.stereo)
    {
      addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufr * SPU_BUFFER_SIZE);
      _gas_set_voice (inst, 1, addr);
    }
    _gas_streamed_voice_controller(inst, SD_S_KON, 0);
  }
  else
  {
    gas_printf("Error.  Attempted to playback a CD instance before it was ready, play request ignored.\n");
  }
}



/*** _gas_pause_cd_instance ***/
// pauses with a +1 pause_mode, unpauses with a -1, will honor a pause guard
// with a 0 pause_mode it stops and recycles the instance
int _gas_pause_cd_instance(int inst, int pause_mode)
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


/*** _gas_init_iop_streaming ***/
void _gas_init_iop_streaming(int inst, int source)
{
  unsigned char *load_to;
  int sectors_to_load, left;
  int i;
  int sectors, divs, load_size;
  int nchunk, nchunk_sectors;
//  int offset;
  int push_it = 0;

  // grab a free iop buffer and preload it
  if (gas.inst_list[inst].flag.stereo)
  {
    gas.inst_list[inst].iop_buf = fifo_queue_pop(&gas.free_cd_stereo_bufs);
    gas.inst_list[inst].iop_buf_addr[0] = gas.iop_buffers_top + 
      (gas.inst_list[inst].iop_buf * CD_STEREO_BUFFER_SIZE);
    nchunk = CD_STEREO_BUFFER_N_CHUNK;
    nchunk_sectors = CD_STEREO_BUFFER_SECTORS_N_CHUNK;
  }
  else
  {
    gas.inst_list[inst].iop_buf = fifo_queue_pop(&gas.free_cd_mono_bufs);
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

  gas.inst_list[inst].src.cd.curr_sector = gas.source_list[source].src.cd.start_sector;
  gas.inst_list[inst].src.cd.sectors_remaining = gas.source_list[source].size / PS2_CD_SECTOR_SIZE;

  // set up the load increment
  sectors = gas.source_list[source].size / PS2_CD_SECTOR_SIZE;

  divs = sectors / nchunk_sectors;
  if (sectors % nchunk_sectors != 0)
  {
    ++divs;
  }
  load_size = sectors / divs;
  if (sectors % divs != 0)
  {
    ++load_size;
  }
  if (gas.source_list[source].flag.stereo && ((load_size >> 1) != load_size))
  {
    // need an even number to load.
    ++load_size;
  }
  gas.inst_list[inst].iop_target_load_amount = load_size;
gas_printf("Loadsize is %d\n", load_size);
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

  sceCdSync(0);
  sceCdRead(gas.inst_list[inst].src.cd.curr_sector, sectors_to_load, load_to, &gSceCdRMode);

  gas.inst_list[inst].last_iop_buf_addr =
    gas.inst_list[inst].iop_buf_addr[0] + 
    (gas.inst_list[inst].iop_load_amount[0] * PS2_CD_SECTOR_SIZE) - 
    ((gas.inst_list[inst].flag.stereo + 1) * SPU_BUFFER_HALF);

  gas.inst_list[inst].src.cd.curr_sector += sectors_to_load;
  gas.inst_list[inst].src.cd.sectors_remaining -= sectors_to_load;
  if (push_it)
    fifo_queue_push(&gas.reload_cd_streams, inst);
}


/*** _gas_init_cd_instance ***/
// does the initialization of an cd instance that is specific to an cd instance
int _gas_init_cd_instance(int inst, int source)
{
  gas.inst_list[inst].flag.ready = 0;
  fifo_queue_push(&gas.cd_preloads_pending, inst);

  return 1;
}

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
