/*
 * Code to support spu-resident sources 
 */
#include <kernel.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"

extern GasSystemState gas;


/*** _gas_add_spu_source ***/
// Uploads the sound data to the spu, returns 0 on error (usually not enough room),
// and 1 on success.
int _gas_add_spu_source(int entry)
{
	int size;
	char *wavBuffer;

  // check if there is room for this source
  if (gas.spu_heap_curr + gas.source_list[entry].size >= gas.spu_heap_end)
    return 0;

  gas.source_list[entry].src.spu.addr = gas.spu_heap_curr;

	if( (wavBuffer = _gas_load_file(gas.source_list[entry].full_filename, &size)) == NULL )
    return 0;

  if (size != gas.source_list[entry].size)
  {
gas_printf("Warning, the disk size(%d) and the entry size(%d) do not match for %s\n",
  size, gas.source_list[entry].size, gas.source_list[entry].full_filename);
    gas.source_list[entry].size = size;
  }

  // set up the markings for looping or one-shot sounds
  //_AdpcmSetMarkSTART(wavBuffer, size);
  if (gas.source_list[entry].flag.loop)
  {
    _AdpcmSetMarkLOOP(wavBuffer, size);
  }
  else
  {
    _AdpcmSetMarkSTOP(wavBuffer, size);
  }

  // do the upload
  if (_gas_dma_iop_to_spu_blocking(wavBuffer, (unsigned char *)gas.source_list[entry].src.spu.addr, size) != size)
    gas_printf("What the...\n");

  gas.spu_heap_curr += gas.source_list[entry].size;
  gas.source_list[entry].flag.loaded = 1;

  return 1;
}


/*** _gas_play_spu_instance ***/
// plays back an spu-hosted source
void _gas_play_spu_instance(int inst)
{
  short core = gas.inst_list[inst].flag.corel;
  short voice = gas.inst_list[inst].flag.voicel;
  int   addr = gas.inst_list[inst].src.spu.cur_addr;
  _gas_set_voice (inst, 0, addr);

	sceSdSetSwitch( core|SD_S_KON , 0x1<<voice);
}


/*** _gas_pause_spu_instance ***/
// Pauses, unpauses and stops an spu-hosted sound, based on the pause-mode.
// pauses (will honor a pause guard) with a +1 pause_mode, unpauses with a -1 
// with a 0 pause_mode it stops and recycles the instance
int _gas_pause_spu_instance(int inst, int pause_mode)
{ 
  int prev_count = gas.inst_list[inst].pause_count;
  int nax;

  // check for a stop command
  if (pause_mode == 0)
  {
    // make it so that reclaim will pick this instance up and recycle it
    gas.inst_list[inst].pause_count = 0;
    gas.inst_list[inst].flag.loop = 0;
	  sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);
  
    _gas_reclaim_voices();
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
    nax = sceSdGetAddr(gas.inst_list[inst].flag.corel|SD_VA_NAX|(gas.inst_list[inst].flag.voicel<<1));
gas_printf("Pause Target - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corel,
  gas.inst_list[inst].flag.voicel, nax);
    sceSdSetAddr(gas.inst_list[inst].flag.corel|SD_VA_SSA|(gas.inst_list[inst].flag.voicel<<1), nax );

  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);
  }
  return 1;
}

