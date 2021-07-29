#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include <string.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"


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

printf("Instance %d - Core %d voice %d volume L/R (0x%x/0x%x)\n", inst, core, voice, voll, volr);

  _gas_set_voice_volume (core, voice, voll, volr);
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

  printf("Search Filename: %s\n", filename);
  sceCdDiskReady(0);
  ret= sceCdSearchFile(&cdfp, filename);

  if(!ret)
  {
    // read from host disk
    printf("sceCdSearchFile fail :%d\nTrying to load the sound from the host disk.\n", ret);

    strcpy(filename, gas.host_prefix);
    strcat(filename, orig_filename);

    fd = open(filename, O_RDONLY);
 	  *size = lseek(fd, 0, 2);
	  if( *size <= 0 ) { printf( "\nCan't load VAG file to iop heap \n" ); return NULL; }
    if (*size > (MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE))
    {
      printf("Error.  This sound is too big %s to upload, size %d (640kb upload max)\n", orig_filename, *size);
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
      printf("Error.  This sound is too big %s to upload, size %d (640kb upload max)\n", orig_filename, *size);
      return NULL;
    }
    fd = open(filename, O_RDONLY);
	  if( fd <= 0 ) { printf( "\nCan't load VAG file to iop heap \n" ); return NULL; }

    // use the mono buffers (640kb) as a clearinghouse for the spu sound data
	  buffer = gas.iop_cd_mono_buffers_top;

	  read( fd, buffer, *size);
	  close(fd);
  }

  // set up the rest of the member data for this source

  return buffer;
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
printf("using core %d voice %d\n", *core, *voice);
  }
  else
    return 0;
  return 1;
}


/*** _gas_recycle_voice ***/
void _gas_recycle_voice(int core, int voice)
{
printf("Freeing core %d voice %d\n", core, voice);

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
	printf("Data transfer ...\n" );
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
      (u_char*)(iop_addr+off), (int)addr, (u_int)up_size );
  	printf("TSA0 = %x \n", sceSdGetAddr( pri_ch|SD_A_TSA ) );
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
      (u_char*)(iop_addr+off), (int)addr, (u_int)up_size );
  	printf("TSA1 = %x \n", sceSdGetAddr( sec_ch|SD_A_TSA ) );

    if (exit_flag)
      break;

    addr+=UPLOAD_SIZE;
    off+=UPLOAD_SIZE;
  }
  sceSdVoiceTransStatus(0, SD_TRANS_STATUS_WAIT );
  sceSdVoiceTransStatus(1, SD_TRANS_STATUS_WAIT );

  if (off == 0)
    off = size;
  printf("Total sent %d\n", off );
  return off;
}


/*** gas_spu_mem_dump ***/
// Dumps the contents of spu memory to disk, for debugging
int gas_spu_mem_dump()
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
      (u_char*)buffer, (int)addr, (u_int)readsize );
printf("dumping 0x%x thru 0x%x\n", addr, addr+readsize);
    // write it to the dump file
    n = write(fd, buffer, readsize);
    if (n != readsize)
      printf("Yikes!\n");
  }
  close(fd);
  FreeSysMemory(buffer);
  return 1;
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
      (u_char*)buffer, (int)addr, (u_int)readsize );
printf("clearing 0x%x thru 0x%x\n", addr, addr+readsize);
  }
  FreeSysMemory(buffer);
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
  gas.inst_list[inst].flag.playback_pending = 0;

  // init the source-type specific stuff that needs to happen before voices are locked
  switch (gas.source_list[source].flag.src_type)
  {
    case SRC_TYPE_SPU: break;
    case SRC_TYPE_EE:  break;

    case SRC_TYPE_IOP:
      // stereo+1 is either 1 or 2, which is the number of spu buffers minimum that we need
      printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
      if (gas.free_spu_bufs.count < (gas.inst_list[inst].flag.stereo + 1))
        return 0;
      break;

    case SRC_TYPE_CD:
      printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
      if ( gas.free_spu_bufs.count < (gas.inst_list[inst].flag.stereo + 1) )
        return 0;
      if (gas.inst_list[inst].flag.stereo == 1)
      {
        printf("\t\t\t\tfree stereo bufs %d\n", gas.free_cd_stereo_bufs.count);
        if (gas.free_cd_stereo_bufs.count < 1)
          return 0;
      }
      else
      {
        printf("\t\t\t\tfree mono bufs %d\n", gas.free_cd_mono_bufs.count);
        if (gas.free_cd_mono_bufs.count < 1)
          return 0;
      }
      break;

    default:
      return 0;
  }


  // reclaim any unused voices
  _gas_reclaim_voices();

  // search for an open voice
  if (_gas_get_voice(&i, &j) == 0)
    printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
  gas.inst_list[inst].flag.corel = i;
  gas.inst_list[inst].flag.voicel = j;

  // pick a r-channel voice for stereo sources
  if (gas.source_list[source].flag.stereo)
  {
    if (_gas_get_voice(&i, &j) == 0)
      printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
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

