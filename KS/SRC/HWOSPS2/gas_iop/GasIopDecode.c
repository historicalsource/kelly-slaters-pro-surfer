// GAS decode and dispatch commands
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <string.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"

extern GasSystemState gas;

int _gas_init_new_instance(int inst, int source);


/*** rpc - GAS_IOP_ADD_SOURCE ***/
// returns the source id of the newly added source, this is used to create instances 
// returns -1 if the source name cannot be found or if the source cannot be created.
int gas_add_source(char *filename)
{
  int i;

  // search the sources for one called 'filename'
  for (i=0; i<gas.source_list_count; ++i)
  {
    if (strcmp(filename, gas.source_list[i].short_filename) == 0)
      break;
  }
  if (i == gas.source_list_count)
    return -1;

  // don't allow multiple loads of the same source
  if (gas.source_list[i].flag.loaded == 1)
    return i;

  // add the source differently depending on its src_type
  switch (gas.source_list[i].flag.src_type)
  {
    case SRC_TYPE_SPU: 
      if (_gas_add_spu_source(i) == 0)
        i = -1;
      break;
    case SRC_TYPE_EE:  break;
    case SRC_TYPE_IOP: 
      if (_gas_add_iop_source(i) == 0)
        i = -1;
      break;
    case SRC_TYPE_CD:
      if (_gas_add_cd_source(i) == 0)
        i = -1;
      break;
    default:
      return -1;
  }

  // return the index of the GasSource[] that this entry uses
  return i;
}


/*** rpc - GAS_IOP_ADD_INSTANCE ***/
// returns the instance id of the newly added instance, this is used to refer to this
// instance for modifying, playing, etc.
// returns -1 if the source cannot be found or if the instance cannot be created.
int gas_add_instance(int source)
{
  int i, ret_val;

  if (source >= gas.source_list_count || source < 0)
  {
    return -1;
  }

  if (gas.free_instances.count > 0)
  {
    // use a recycled instance
    i = fifo_queue_pop(&gas.free_instances);
gas_printf("using recycled instance %d\n", i);
  }
  else if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
  {
    // try to reclaim one
    _gas_reclaim_voices();
gas_printf("Attempting to reclaim a voice or two\n");
    if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
      return -1;
  }
  else
  {
    // add the instance to the list of instances
    i = gas.inst_list_count;
gas_printf("creating new instance %d\n", i);
  }
  gas.inst_list[i].source = source;
  gas.inst_list[i].instance_id += GAS_INSTANCE_ID_INCREMENT;

  if (_gas_init_new_instance(i, source) == 0)
    return -1;

  // return the index of the GasSource[] that this entry uses
  if (i == gas.inst_list_count)
    gas.inst_list_count++;

  ret_val = gas.inst_list[i].instance_id;
  if (gas.inst_list[i].flag.stereo)
    ret_val |= GAS_INSTANCE_STEREO_FLAG_BIT;
  return ret_val;
}


/*** gas_play_instance ***/
int gas_play_instance(int instance_id, unsigned short volume_left, unsigned short volume_right)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;

  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  if (inst >= gas.inst_list_count || inst < 0)
    return 0;

  if (gas.inst_list[inst].pause_count < 1)
    return 1; // instance already playing

  // force an update in _gas_update_volume
  gas.inst_list[inst].voll = volume_left + 1;

  gas_instance_volume(instance_id, volume_left, volume_right);
  _gas_update_volume(inst);

  gas.inst_list[inst].pause_count = 0;

  switch (gas.inst_list[inst].flag.src_type)
  {
    case SRC_TYPE_SPU: 
      _gas_play_spu_instance(inst);
      break;
    case SRC_TYPE_EE:  
      break;
    case SRC_TYPE_IOP:
      _gas_play_iop_instance(inst);
      break;
    case SRC_TYPE_CD:
      _gas_play_cd_instance(inst);
      break;
    default:
      return 0;
  }
  
  return 1;
}


/*** gas_pause_instance ***/
int gas_pause_instance(int instance_id, int pause_mode)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  int ret;

  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

//  _gas_update_volume(inst); // for unpause, set volume to be correct

  if (gas.inst_list[inst].flag.used == 1)
  {
    switch (gas.inst_list[inst].flag.src_type)
    {
      case SRC_TYPE_SPU: 
        ret = _gas_pause_spu_instance(inst, pause_mode);
        break;
      case SRC_TYPE_EE:  
        break;
      case SRC_TYPE_IOP: 
        ret = _gas_pause_iop_instance(inst, pause_mode);
        break;
      case SRC_TYPE_CD:  
        ret = _gas_pause_cd_instance(inst, pause_mode);
        break;
      default:
        return 0;
    }
  }
  return ret;
}


/*** gas_remove_source ***/
int gas_remove_source(int source_id)
{
  if (source_id == -1)
    return 0;
  if (gas.source_list[source_id].flag.loaded)
  {
    switch (gas.source_list[source_id].flag.src_type)
    {
      case SRC_TYPE_SPU: 
        // we don't allow deletion of this kind of source, remove_all to remove these
        break;
      case SRC_TYPE_IOP:
        _gas_remove_iop_source(source_id);
        break;
      case SRC_TYPE_CD: 
        gas.source_list[source_id].flag.loaded = 0;
        break;
      case SRC_TYPE_EE: 
        gas.source_list[source_id].flag.loaded = 0;
        break;
    }
  }
  return 1;
}

/*** gas_command_list ***/
// issues a new command list for differed processing (by the update loop)
int gas_command_list(void *data)
{
  GasCommandEntry *commands = (GasCommandEntry *)data;

//  gas_printf("Command list id:0x%x\n", commands[0].instance_id);
  memcpy(gas.command_list, commands, sizeof(GasCommandEntry) * GAS_MAX_INSTANCES);

  gas.commands_waiting = 1;

  return 0;
}


/*** _gas_instance_volume ***/
int gas_instance_volume(int instance_id, int voll, int volr)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
//gas_printf("Instance(u) 0x%x/0x%x - Core %d voice %d volume L/R (0x%x/0x%x)\n", instance_id, inst, 
//      gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
//      gas.inst_list[inst].voll, gas.inst_list[inst].volr);
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  // set the target volumes
  if (gas.inst_list[inst].flag.stereo == 0)
  {
    gas.inst_list[inst].target_voll = voll;
    gas.inst_list[inst].target_volr = volr;
  }
  else
  {
    gas.inst_list[inst].target_voll = voll;
    gas.inst_list[inst].target_volr = 0;
    gas.inst_list[inst].target_rvoll = 0;
    gas.inst_list[inst].target_rvolr = volr;
  }
  gas.inst_list[inst].flag.volume_touched = 1;
//  _gas_update_volume(inst);
  return 1;
}


void _gas_update_volume(int inst)
{
  short voll_delta;
  short volr_delta;
  short rvoll_delta = 0;
  short rvolr_delta = 0;

  // ramp volume to correct level
  if (gas.inst_list[inst].flag.stereo)
  {
    rvoll_delta = gas.inst_list[inst].target_rvoll - gas.inst_list[inst].rvoll;
    rvolr_delta = gas.inst_list[inst].target_rvolr - gas.inst_list[inst].rvolr;
  }
  voll_delta = gas.inst_list[inst].target_voll - gas.inst_list[inst].voll;
  volr_delta = gas.inst_list[inst].target_volr - gas.inst_list[inst].volr;

  if (voll_delta == 0 && volr_delta == 0 && rvoll_delta == 0 && rvolr_delta == 0)
  {
    // clear the dirty flag, this sound is clean as a whistle
    gas.inst_list[inst].flag.volume_touched = 0;
  }
  else
  {
    // clamp the volume deltas to safe levels
    if (gas.inst_list[inst].flag.used && (gas.inst_list[inst].pause_count < 1))
    {
      if (voll_delta >  GAS_SAFE_VOLUME_DELTA)  voll_delta =  GAS_SAFE_VOLUME_DELTA;  
      if (voll_delta < -GAS_SAFE_VOLUME_DELTA)  voll_delta = -GAS_SAFE_VOLUME_DELTA; 
      if (volr_delta >  GAS_SAFE_VOLUME_DELTA)  volr_delta =  GAS_SAFE_VOLUME_DELTA; 
      if (volr_delta < -GAS_SAFE_VOLUME_DELTA)  volr_delta = -GAS_SAFE_VOLUME_DELTA; 
    }

    // send the update volumes
    gas.inst_list[inst].voll += voll_delta; 
    gas.inst_list[inst].volr += volr_delta; 
    _gas_set_voice_volume (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
      gas.inst_list[inst].voll, gas.inst_list[inst].volr);

    if (gas.inst_list[inst].flag.stereo)
    {
      if (gas.inst_list[inst].flag.used && (gas.inst_list[inst].pause_count < 1))
      {
        if (rvoll_delta >  GAS_SAFE_VOLUME_DELTA)  rvoll_delta =  GAS_SAFE_VOLUME_DELTA; 
        if (rvoll_delta < -GAS_SAFE_VOLUME_DELTA)  rvoll_delta = -GAS_SAFE_VOLUME_DELTA; 
        if (rvolr_delta >  GAS_SAFE_VOLUME_DELTA)  rvolr_delta =  GAS_SAFE_VOLUME_DELTA; 
        if (rvolr_delta < -GAS_SAFE_VOLUME_DELTA)  rvolr_delta = -GAS_SAFE_VOLUME_DELTA; 
      }
      // send the update volumes
      gas.inst_list[inst].rvoll += rvoll_delta; 
      gas.inst_list[inst].rvolr += rvolr_delta; 
      _gas_set_voice_volume (gas.inst_list[inst].flag.corer, gas.inst_list[inst].flag.voicer, 
        gas.inst_list[inst].rvoll, gas.inst_list[inst].rvolr);
    }
  }
}



int gas_instance_pitch(int instance_id, int pitch)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  gas.inst_list[inst].pitch = (pitch * gas.source_list[gas.inst_list[inst].source].pitch_one) / 1000;
  _gas_set_voice_pitch (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
    gas.inst_list[inst].pitch);
  return 1;
}


int gas_status_is_playing(int instance_id)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  return gas.inst_list[inst].flag.used && (gas.inst_list[inst].pause_count < 1);
}

int gas_status_is_ready(int instance_id)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  return gas.inst_list[inst].flag.used && gas.inst_list[inst].flag.ready;
}

void _gas_set_voice_volume (int core, int voice, int voll, int volr)
{
  sceSdSetParam (core | (voice<<1) | SD_VP_VOLL, voll );
	sceSdSetParam (core | (voice<<1) | SD_VP_VOLR, volr );
}


void _gas_set_voice_pitch (int core, int voice, int pitch)
{
  sceSdSetParam (core | (voice<<1) | SD_VP_VOLL, pitch );
}

/*** _gas_set_voice ***/
// !!! needs to be updated to support adsr
// side == 0 is left channel, side == 1 is right channel
void
_gas_set_voice (int inst, int side, unsigned int spu_addr)
{
  int voice, core, voll, volr;
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
gas_printf("Instance %d - Core %d voice %d volume L/R (0x%x/0x%x)\n", inst, core, voice, voll, volr);
//  _gas_set_voice_volume (core, voice, voll, volr);
	sceSdSetParam (core | (voice<<1) | SD_VP_PITCH, gas.inst_list[inst].pitch );
	sceSdSetAddr  (core | (voice<<1) | SD_VA_SSA, spu_addr );
  sceSdSetParam (core | (voice<<1) | SD_VP_ADSR1, 0x000f );
  sceSdSetParam (core | (voice<<1) | SD_VP_ADSR2, 0x1fc0 );		

  return;
}


/*** _gas_load_file ***/
char* _gas_load_file(char *orig_filename, int *size)
{
	int fd;
	char *buffer;
  int ret;
  char filename[64];

  sceCdlFILE cdfp;

  // find the file in the CD TOC
  strcpy(filename, "\\");
  strcat(filename, orig_filename);
  strcat(filename, ";1");

  gas_printf("Search Filename: %s\n", filename);
  sceCdDiskReady(0);
  ret= sceCdSearchFile(&cdfp, filename);

  gas_printf("Ret = %d\n", ret);

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
    gas_printf("Attempting a blocking open call\n");
    fd = open(filename, O_RDONLY);
	  if( fd <= 0 ) { gas_printf( "\nCan't load VAG file to iop heap \n" ); return NULL; }
    gas_printf("Pew, we made it back alive\n");

    // use the mono buffers (640kb) as a clearinghouse for the spu sound data
	  buffer = gas.iop_cd_mono_buffers_top;

	  read( fd, buffer, *size);
    gas_printf("Pew, got past the read\n");
	  close(fd);
    gas_printf("Pew, closed it ok\n");
  }

  // set up the rest of the member data for this source

  return buffer;
}

void   gas_voice_status_dump()
{
  int inst;
  int endx, endflags, voice;

  gas_printf("Behold, here art the voice hogs (%d used):\n", gas.inst_list_count);
  for (inst=0; inst<PS2_MAX_VOICES; ++inst)
  {
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
    gas_printf("%8s: 0x%04x\tst(%d) l(%d) endx(%2d) (u%d p%d t%d s%d(%2d/%2d) r%d n%d) == check(%d) %s\n", 
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
  _gas_reclaim_voices();
}

/*** _gas_get_voice ***/
// locks down a voice for use by an instance
int _gas_get_voice(int *core, int *voice)
{
  unsigned short v;

  if (gas.free_voices.count > 0)
  {
    v = fifo_queue_pop(&gas.free_voices);
    *core = v / PS2_MAX_VOICES_PER_CORE;
    *voice = v % PS2_MAX_VOICES_PER_CORE;
gas_printf("using core %d voice %d\n", *core, *voice);
  }
  else
    return 0;
  return 1;
}


/*** _gas_recycle_voice ***/
void _gas_recycle_voice(int core, int voice)
{
gas_printf("Freeing core %d voice %d\n", core, voice);

  sceSdSetSwitch( core|SD_S_KOFF , 0x1<<voice);

  // free its voice
  fifo_queue_push_front(&gas.free_voices, ((core * PS2_MAX_VOICES_PER_CORE) + voice));
}


/*** _gas_recycle_instance ***/
void _gas_recycle_instance(int inst)
{
  char free_spu_stuff;
  char free_iop_stuff;

  // free its spu buffer
  switch (gas.inst_list[inst].flag.src_type)
  {
    case SRC_TYPE_IOP:
      free_spu_stuff = 1;
      free_iop_stuff = 0;
      break;

    case SRC_TYPE_CD:
    case SRC_TYPE_EE:
      free_spu_stuff = 1;
      free_iop_stuff = 1;
      break;

    default: // spu sounds
      free_spu_stuff = 0;
      free_iop_stuff = 0;
      break;
  }

  if (free_spu_stuff)
  {
    // release the spu buffers
    fifo_queue_push_front(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl); 
    if (gas.inst_list[inst].flag.stereo)
    {
      fifo_queue_push_front(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufr); 
    }
  }
  if (free_iop_stuff)
  {
    if (gas.inst_list[inst].flag.stereo)
      fifo_queue_push_front(&gas.free_cd_stereo_bufs, gas.inst_list[inst].iop_buf);
    else
      fifo_queue_push_front(&gas.free_cd_mono_bufs, gas.inst_list[inst].iop_buf);
  }
}


/*** _gas_reclaim_voices ***/
// checks to see if we can recycle any of the instances and their voices
int _gas_reclaim_voices()
{
  unsigned short v;
  int endflags;
  unsigned int voice;
  int hitme;

  for (v=0; v<gas.inst_list_count; ++v)
  {
    // check if this instance is still playing, and whether it has active flags set, etc
    if ((gas.inst_list[v].flag.used == 1) && 
        (gas.inst_list[v].pause_count < 1) && 
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
      }
      else
        hitme = (gas.inst_list[v].spu_reload == RELOAD_FLAG_DEAD);

      if (hitme)
      {
        _gas_recycle_voice(gas.inst_list[v].flag.corel, gas.inst_list[v].flag.voicel);
        _gas_recycle_instance(v);

        fifo_queue_push(&gas.free_instances, v);
        gas.inst_list[v].flag.used = 0;

        if (gas.inst_list[v].flag.stereo)
        {
          _gas_recycle_voice(gas.inst_list[v].flag.corer, gas.inst_list[v].flag.voicer);
        }
      }
    }
  }

  return 1;
}


/*** _gas_dma_iop_to_spu_blocking ***/
// Performs a dma transfer of a block of memory in the iop to the spu, and
// blocks until the transfer is complete.  Could use some updating to make
// it more general
int _gas_dma_iop_to_spu_blocking(unsigned char *iop_addr, unsigned char *spu_addr, int size)
{
  int exit_flag = 0;
  unsigned char *addr;
  int off;
  int up_size = UPLOAD_SIZE;
  char pri_ch = 0;
  char sec_ch = 1;

	// --- data transfer
	gas_printf("Data transfer ...\n" );
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
  	gas_printf("TSA0 = %x \n", sceSdGetAddr( pri_ch|SD_A_TSA ) );
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
  	gas_printf("TSA1 = %x \n", sceSdGetAddr( sec_ch|SD_A_TSA ) );

    if (exit_flag)
      break;

    addr+=UPLOAD_SIZE;
    off+=UPLOAD_SIZE;
  }
  sceSdVoiceTransStatus(0, SD_TRANS_STATUS_WAIT );
  sceSdVoiceTransStatus(1, SD_TRANS_STATUS_WAIT );

  if (off == 0)
    off = size;
  gas_printf("Total sent %d\n", off );
  return off;
}

/*** _gas_init_new_instance ***/
int _gas_init_new_instance(int inst, int source)
{
  int i, j;
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
    case SRC_TYPE_EE:  break;

    case SRC_TYPE_IOP:
      // stereo+1 is either 1 or 2, which is the number of spu buffers minimum that we need
      gas_printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
      if (gas.free_spu_bufs.count < (gas.inst_list[inst].flag.stereo + 1))
        return 0;
      break;

    case SRC_TYPE_CD:
      gas_printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
      if ( gas.free_spu_bufs.count < (gas.inst_list[inst].flag.stereo + 1) )
        return 0;
      if (gas.inst_list[inst].flag.stereo == 1)
      {
        gas_printf("\t\t\t\tfree stereo bufs %d\n", gas.free_cd_stereo_bufs.count);
        if (gas.free_cd_stereo_bufs.count < 1)
          return 0;
      }
      else
      {
        gas_printf("\t\t\t\tfree mono bufs %d\n", gas.free_cd_mono_bufs.count);
        if (gas.free_cd_mono_bufs.count < 1)
          return 0;
      }
      break;

    default:
      return 0;
  }
  gas_printf("\t\t\t\tfree voices %d\n", gas.free_voices.count);


  // reclaim any unused voices
  _gas_reclaim_voices();

  // search for an open voice
  if (_gas_get_voice(&i, &j) == 0)
    gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
  gas.inst_list[inst].flag.corel = i;
  gas.inst_list[inst].flag.voicel = j;

  // pick a r-channel voice for stereo sources
  if (gas.source_list[source].flag.stereo)
  {
    if (_gas_get_voice(&i, &j) == 0)
      gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
    gas.inst_list[inst].flag.corer = i;
    gas.inst_list[inst].flag.voicer = j;
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

    case SRC_TYPE_EE:  break;

    case SRC_TYPE_IOP:
      if (_gas_init_iop_instance(inst, source) == 0)
        return 0;
      break;

    case SRC_TYPE_CD:
      if (_gas_init_cd_instance(inst, source) == 0)
        return 0;
      break;

    default:
      return 0;
  }

  return 1;
}

