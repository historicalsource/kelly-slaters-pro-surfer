// GAS command support functions
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include <string.h>
#include "../gas.h" 
#include "GasSystem.h"


extern GasStatus gas_status;
// G A S   R P C   L O A D  A L L  S P U   S O U R C E S
//
// Loads all SPU sources

int _gas_rpc_load_all_spu_sources()
{
  const int block_size = MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE;
  char *block_buffer = gas.iop_cd_mono_buffers_top;
  int fd;
  int spu_image_size = 99999999999;
  int spu_heap_base;
  int entry, bytes_to_load;
  GasSource *sources = gas.source_list[gas.num_heaps -1];
  
  RECORDRA

  fd = _gas_rpc_get_fd( gas.spu_heaps[gas.num_heaps -1].spu_collection );

  if( fd < 0 )
  {
    gas_printf( "Could not load collection %s\n", gas.spu_heaps[gas.num_heaps -1].spu_collection );
    return -1;
  }
  else 
  {
    gas_printf( "Got collection pointer %s\n", gas.spu_heaps[gas.num_heaps -1].spu_collection );
  }

  // FIXME
  // Remember this shoddy code martin? 
  // CD type entries must be last on the list, cos no spu entries get loaded after a cd entry
  // This shouldn't cause any problems since the sound tool sorts snd files this
  // way anyway, but this really shouldn't be a requirement
  // -mjd 29/Jun/01
  spu_image_size = lseek(fd, 0, SEEK_END);
  if ( gas.spu_heap_end - gas.spu_heap_curr < spu_image_size)
    return 0;
  
  lseek(fd, 0, SEEK_SET);

  gas.spu_heaps[gas.num_heaps -1].start = gas.spu_heap_curr;
  spu_heap_base = gas.spu_heaps[gas.num_heaps-1].start;
  while( ( bytes_to_load = read( fd, block_buffer, block_size ) ) > 0 )
  {
    int bytes_loaded = _gas_rpc_dma_iop_to_spu_blocking(
      block_buffer, (unsigned char *)gas.spu_heap_curr, bytes_to_load );
    
    if( bytes_loaded != bytes_to_load )
      gas_printf( "What the...\n" );

    gas.spu_heap_curr += bytes_to_load;
  }
  gas.spu_heaps[gas.num_heaps -1].end = gas.spu_heap_curr -1;
  
  for( entry = 0; entry < gas.spu_heaps[gas.num_heaps -1].source_list_count; entry++ )
  {
    if( sources[entry].flag.src_type == SRC_TYPE_SPU )
      sources[entry].src.spu.addr = spu_heap_base + sources[entry].offset;
  }

  close( fd );
  gas_printf( "Done loading collection %s\n", gas.spu_heaps[gas.num_heaps -1].spu_collection );

  return 1;
}




//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   S P U   S O U R C E 
//
// Loads a new SPU source into SPU ram
int _gas_rpc_add_spu_source(int heap, int entry)
{
  if( gas.source_list[heap][entry].flag.loaded )
    return MAKE_SOURCE_ID(heap, entry);
  else
    return -1;
}

typedef struct 
{
	int	sampleRate;
	int sampleFormat;
	int	channelCount;
	int	chunkSize;
} gasStreamInfo;

static gasStreamInfo	gas_rawstream_info;
static int				gas_rawstream_called = 0;
static char				gas_rawstream_filename_buffer[ FILENAME_LENGTH ];
static char			*	gas_rawstream_filename = GAS_STREAM_VBC;

//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   C D   S O U R C E 
//
// prepares a CD source structure
int _gas_rpc_add_cd_source(int heap, int entry)
{
  // Assumed semaphore locks: system_state_sema
  int isRawStream;
  char *rawStreamFilename;
  int ret, offset;
  char fname[256];
  sceCdlFILE cdfp;

  RECORDRA

  rawStreamFilename = gas_rawstream_filename; 
  gas_rawstream_filename = GAS_STREAM_VBC;

  isRawStream = gas_rawstream_called;	  
  gas_rawstream_called = 0;

  sceCdDiskReady(0);
  strcpy(fname, "\\");
  strcat(fname, gas_root_dir);
  strcat(fname, rawStreamFilename);
  strcat(fname, ";1");

  // Restore the stream filename for the next call to be again the default one (e.g. GAS_STREAM_VBC)

  ret= sceCdSearchFile(&cdfp, fname);

  gas.source_list[heap][entry].rawStreamHostFd = -1;
  offset = gas.source_list[heap][entry].offset;
  if(!ret)
  {
	if( isRawStream )
	{
		// MALKIA: That's my case - separate files streamed from host
		int fd;

		strcpy(fname, gas_root_dir);
		strcat(fname, rawStreamFilename );

		fd = _gas_rpc_get_fd( fname );
		if( fd < 0 )
		{
			gas_printf( "failed to open %s\n", fname );
			return -1;
		}

		gas.source_list[heap][entry].rawStreamHostFd = fd;
  	    gas.source_list[heap][entry].src.cd.start_sector = ( offset / PS2_CD_SECTOR_SIZE );
	}
	else
	{
		if( gas.host_streaming_fd < 0 )
		{
		  if (gas.stream_from_host)
		  {
        
			strcpy(fname, gas_root_dir);
			strcat(fname, rawStreamFilename );
			gas.host_streaming_fd = _gas_rpc_get_fd( fname );
      
		  }
		  if( gas.host_streaming_fd < 0 )
		  {
#ifdef DEBUG_OUTPUT
			gas_printf( "could not get (host) stream\n" );
#endif
			return -1;
		  }
		}
		if( gas.host_streaming_fd >= 0 )
		  gas.source_list[heap][entry].src.cd.start_sector = ( offset / PS2_CD_SECTOR_SIZE );
		else
		  return -1;
	}
  }
  else
  {
    // read from cd
    gas.source_list[heap][entry].src.cd.start_sector = cdfp.lsn
      + ( offset / PS2_CD_SECTOR_SIZE );
  }

  // set up the rest of the member data for this source
  gas.source_list[heap][entry].flag.loaded = 1;
#ifdef NSL_LOAD_SOURCE_BY_NAME
#ifdef DEBUG_OUTPUT
  gas_printf( "loaded %s\n", gas.source_list[heap][entry].filename );
#endif
#elif defined(NSL_LOAD_SOURCE_BY_ALIAS)
  gas_printf( "loaded %d\n", gas.source_list[heap][entry].aliasID );	
#endif
  return MAKE_SOURCE_ID(heap, entry);
}

int _gas_open_rawstream( char *filename, int sampleRate, int sampleFormat, int channelCount, int chunkSize )
{
	if( gas_rawstream_called )
	{
		gas_printf( "gas_raw_stream_called is TRUE - something bad happened! call malkia!\n" );
	}
	// MALKIA - 23 April 2002 - Prepended '\' in front of the speech file name
	// File names must be given as relative for example: speech\\ann.aud, not \\speech\ann.aud
	gas_rawstream_filename_buffer[0] = '\\';
	strncpy( &gas_rawstream_filename_buffer[1], filename, FILENAME_LENGTH-1 );
	gas_rawstream_filename = gas_rawstream_filename_buffer;
	gas_rawstream_info.sampleRate	= sampleRate;
	gas_rawstream_info.sampleFormat = sampleFormat;
	gas_rawstream_info.channelCount	= channelCount;
	gas_rawstream_info.chunkSize	= chunkSize;
	gas_rawstream_called = 1;
	return 0;
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

#ifdef DEBUG_GOODIES
    gas.inst_list[inst].old_addr = addr;
#endif

  RECORDRA
  _gas_rpc_set_voice (inst, 0, addr);

	sceSdSetSwitch( core|SD_S_KON , 0x1<<voice);
  gas.inst_list[inst].pause_count = 0;
  gas.inst_list[inst].played = 1;
  gas_status.instances[inst].is_playing = 1;
  gas_status.instances[inst].is_ready = 1;
  gas_status.instances[inst].is_valid = 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P L A Y   C D   I N S T A N C E
//
// begins playback of an cd-based source
void _gas_rpc_play_cd_instance(int inst)
{
  // Assumed semaphore locks: system_state_sema
  unsigned addr;

  RECORDRA
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

#ifdef DEBUG_GOODIES
    gas.inst_list[inst].old_addr = addr;
#endif

    gas.inst_list[inst].flag.needs_spu_preload = 0;

    _gas_rpc_set_voice (inst, 0, addr);
    if (gas.inst_list[inst].flag.stereo)
    {
      addr = SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufr * SPU_BUFFER_SIZE);
      _gas_rpc_set_voice (inst, 1, addr);
    }
    _gas_rpc_streamed_voice_controller(inst, SD_S_KON, 0);
    gas.inst_list[inst].pause_count = 0;
    gas.inst_list[inst].played = 1;
    gas_status.instances[inst].is_playing = 1;
    gas_status.instances[inst].is_ready = 1;
    gas_status.instances[inst].is_valid = 1;
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
  RECORDRA
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



	if (gas.inst_list[inst].curr_iop_buf_addr >= gas.inst_list[inst].last_iop_buf_addr)
	{
		if (_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 0, 1) == -1)
		{
			// abort
	#ifdef DEBUG
			gas_debug_check_value("INIT SPU STREAMING ABORT", gas.inst_list[inst].spu_bufl, 0, gas.spu_buffer_num);
	#endif
			fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
			return -1;
		}
	}
	else
	{
		if (_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 0, 0) == -1)
		{
			// abort
	#ifdef DEBUG
			gas_debug_check_value("INIT SPU STREAMING ABORT", gas.inst_list[inst].spu_bufl, 0, gas.spu_buffer_num);
	#endif
			fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
			return -1;
		}
	
	}
	

  gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;


  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
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


		if (gas.inst_list[inst].curr_iop_buf_addr >= gas.inst_list[inst].last_iop_buf_addr)
		{
			if (_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 0, 1) == -1)
			{
				// abort
	#ifdef DEBUG
				gas_debug_check_value("INIT SPU STREAMING ABORT 2L", gas.inst_list[inst].spu_bufl, 0, gas.spu_buffer_num);
				gas_debug_check_value("INIT SPU STREAMING ABORT 2R", gas.inst_list[inst].spu_bufr, 0, gas.spu_buffer_num);
	#endif
				fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
				fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufr);
				gas.inst_list[inst].curr_iop_buf_addr -= SPU_BUFFER_HALF;
				return -1;
			}
		}
		else
		{
			if (_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 0, 0) == -1)
			{
				// abort
	#ifdef DEBUG
				gas_debug_check_value("INIT SPU STREAMING ABORT 2L", gas.inst_list[inst].spu_bufl, 0, gas.spu_buffer_num);
				gas_debug_check_value("INIT SPU STREAMING ABORT 2R", gas.inst_list[inst].spu_bufr, 0, gas.spu_buffer_num);
	#endif
				fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufl);
				fifo_queue_push(&gas.free_spu_bufs, gas.inst_list[inst].spu_bufr);
				gas.inst_list[inst].curr_iop_buf_addr -= SPU_BUFFER_HALF;
				return -1;
			}


		}
    gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;
		// Must come after _gas_rpc_preload_spu_from_iop
  }

	if (gas.inst_list[inst].curr_iop_buf_addr > gas.inst_list[inst].last_iop_buf_addr)
	{
		gas.inst_list[inst].spu_reload = RELOAD_FLAG_RECYCLE_ME;
	}


	if (gas.inst_list[inst].spu_reload != RELOAD_FLAG_RECYCLE_ME)
	{
		// Is this a 4k sound?  IE it will be done loading to the SPU after this proc?
		// Then we need to put in the stop markers


		// preload the second half of the spu buffer
		if (gas.inst_list[inst].curr_iop_buf_addr >= gas.inst_list[inst].last_iop_buf_addr)
			_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 1,1);
		else
			_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufl, 1,0);
		
		gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;

		if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
		{
			if (gas.inst_list[inst].curr_iop_buf_addr >= gas.inst_list[inst].last_iop_buf_addr)
				_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 1,1);
			else
				_gas_rpc_preload_spu_from_iop(gas.inst_list[inst].curr_iop_buf_addr, gas.inst_list[inst].spu_bufr, 1,0);
			gas.inst_list[inst].curr_iop_buf_addr += SPU_BUFFER_HALF;
		}


		if (gas.inst_list[inst].curr_iop_buf_addr > gas.inst_list[inst].last_iop_buf_addr)
		{
			gas.inst_list[inst].spu_reload = RELOAD_FLAG_RECYCLE_ME;
		}


		// set up the critical address to the end of side 0, for the left voice
		// the critical address is used to determine if we can start loading the other buffer
		gas.inst_list[inst].spu_buf_crit = (unsigned char *)SPU_MEMORY_TOP + 
			(gas.inst_list[inst].spu_bufl * SPU_BUFFER_SIZE) + SPU_BUFFER_HALF;

		gas.inst_list[inst].flag.spu_buf_side = 0;
		// Set up the reload of side 1
		//  gas.inst_list[inst].spu_reload = RELOAD_FLAG_SIDE1;
		//  fifo_queue_push(&gas.reload_spu, inst);
	}
  return 0;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P R E L O A D   S P U   F R O M   I O P
//
// dma the first batch of stream data from the iop to the spu, should happen before play happens
int _gas_rpc_preload_spu_from_iop(unsigned char *iop_buf_addr, int spu_buf_num, int spu_buf_side, int end)
{
  unsigned char * spu_buf_addr = (unsigned char *)SPU_MEMORY_TOP + (spu_buf_num * SPU_BUFFER_SIZE)
    + (spu_buf_side * SPU_BUFFER_HALF);

  RECORDRA
	if (end)
	{
		_AdpcmSetMarkSTOP(iop_buf_addr, SPU_BUFFER_HALF)
	}
	else
	{
		if (spu_buf_side == 0)
		{
			_AdpcmSetMarkSTART(iop_buf_addr, SPU_BUFFER_HALF);
		}
		else if (spu_buf_side == 1)
		{
			_AdpcmSetMarkEND(iop_buf_addr, SPU_BUFFER_HALF);
		}
	}
//gas_printf("iop preload side %d iop addr 0x%x spu addr 0x%p\n", spu_buf_side, iop_buf_addr, spu_buf_addr);
  return  _gas_rpc_dma_iop_to_spu_blocking(iop_buf_addr, spu_buf_addr, SPU_BUFFER_HALF);
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

  RECORDRA
  // check for a stop command
  if (pause_mode == MODE_STOP)
  {
    gas_status.instances[inst].is_valid=0;
    gas_status.instances[inst].is_playing=0;
    gas_status.instances[inst].is_ready=0;
    _gas_update_recycle_instance(inst);
    return 1;
  }
  else if (pause_mode == MODE_GUARD)
  {
    // set up a pause guard
    // Not supported any more..
    // we do it in the NSL
    //gas.inst_list[inst].pause_count = -1;
    return 1;
  }

  // do pause or unpause command
  //  No more pause counting here!
  // We pause toggle instead!
  gas.inst_list[inst].pause_count += pause_mode;
  if (gas.inst_list[inst].pause_count < 0)
    gas.inst_list[inst].pause_count = 0;
  else if (gas.inst_list[inst].pause_count > 1)
    gas.inst_list[inst].pause_count = 1;

  // If it was paused and isn't now...
  if (prev_count > 0 && gas.inst_list[inst].pause_count == 0)
  {
    // pitching to 0 is Jason Page's magic way of pausing without clicks
    gas_update_set_voice_pitch (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, gas.inst_list[inst].pitch);
//  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KON , 0x1<<gas.inst_list[inst].flag.voicel);
  }
  // it wasn't pause and is now..
  else if (prev_count <= 0 && gas.inst_list[inst].pause_count > 0) 
  {
#ifdef DEBUG_OUTPUT
    nax = gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
gas_printf("Pause Target - Core %d Voice %d is at 0x%x\n", gas.inst_list[inst].flag.corel,
  gas.inst_list[inst].flag.voicel, nax);
#endif
    gas_update_set_voice_pitch(gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 0);
//    sceSdSetAddr(gas.inst_list[inst].flag.corel|SD_VA_SSA|(gas.inst_list[inst].flag.voicel<<1), nax );
//  	sceSdSetSwitch( gas.inst_list[inst].flag.corel|SD_S_KOFF , 0x1<<gas.inst_list[inst].flag.voicel);
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

  RECORDRA
  // check for a stop command
  if (pause_mode == MODE_STOP)
  {
#ifdef DEBUG_OUTPUT
gas_printf("Recycle due to stop\nPause Target - Core %d Voice %d\n", gas.inst_list[inst].flag.corel,
  gas.inst_list[inst].flag.voicel);
#endif

    gas_status.instances[inst].is_valid=0;
    gas_status.instances[inst].is_playing=0;
    gas_status.instances[inst].is_ready=0;

    return _gas_update_recycle_instance(inst);
  }
  else if (pause_mode == MODE_GUARD)
  {
    // set up a pause guard
    // No longer supported here.
    // only in NSL
    // gas.inst_list[inst].pause_count = -1;
    return 1;
  }

  // do pause or unpause command
  gas.inst_list[inst].pause_count += pause_mode;
  if (gas.inst_list[inst].pause_count < 0)
    gas.inst_list[inst].pause_count = 0;
  if (gas.inst_list[inst].pause_count > 1)
    gas.inst_list[inst].pause_count = 1;

  // This is still cool
  if (prev_count > 0 && gas.inst_list[inst].pause_count == 0)
  {
    // Jason Page's cool way to pause a sound, by pitching it to zero and back to unpause
    gas_update_set_voice_pitch (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, gas.inst_list[inst].pitch);
    if (gas.inst_list[inst].flag.stereo == 1)
      gas_update_set_voice_pitch (gas.inst_list[inst].flag.corer, gas.inst_list[inst].flag.voicer, gas.inst_list[inst].pitch);
//    _gas_rpc_streamed_voice_controller(inst, SD_S_KON, 0);
  }
  else if (prev_count <= 0 && gas.inst_list[inst].pause_count > 0) 
  {
    gas_update_set_voice_pitch (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 0);
    if (gas.inst_list[inst].flag.stereo == 1)
      gas_update_set_voice_pitch (gas.inst_list[inst].flag.corer, gas.inst_list[inst].flag.voicer, 0);

//    _gas_rpc_streamed_voice_controller(inst, SD_S_KOFF, 1);
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
// unused?   int i, j, v;
  RECORDRA
  gas.inst_list[inst].pitch = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one;

  // volumes for this instance
  if(( gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo ) && ( gas.stereo ))
  {
    gas.inst_list[inst].voll = 0;
    gas.inst_list[inst].volr = 0;
    gas.inst_list[inst].rvoll = 0;
    gas.inst_list[inst].rvolr = 0;

    gas.inst_list[inst].target_voll = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].voll;
    gas.inst_list[inst].target_volr = 0;
    gas.inst_list[inst].target_rvoll = 0;
    gas.inst_list[inst].target_rvolr = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].volr;
  }
  else
  {
    gas.inst_list[inst].voll = 0;
    gas.inst_list[inst].volr = 0;
    gas.inst_list[inst].rvoll = 0;
    gas.inst_list[inst].rvolr = 0;

    gas.inst_list[inst].target_voll = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].voll;
    gas.inst_list[inst].target_volr = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].voll;
    gas.inst_list[inst].target_rvoll = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].volr;
    gas.inst_list[inst].target_rvolr = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].volr;
  }
#ifdef BATCH_GOODIES
  gas.inst_list[inst].batch = 0;
  gas.inst_list[inst].old_nax = 0;
  gas.inst_list[inst].old_addr = 0;
#endif
  gas.inst_list[inst].pause_count = 1;
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
  gas.inst_list[inst].flag.src_type = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.src_type;
  if (gas.inst_list[inst].flag.src_type == SRC_TYPE_SPU)
    gas.inst_list[inst].iop_buf = -1;

  gas.inst_list[inst].flag.loop = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.loop;
  gas.inst_list[inst].flag.stereo = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo;
  gas.inst_list[inst].flag.used = 1;
  gas.inst_list[inst].flag.small_sound = 0;
  gas.inst_list[inst].flag.ready = 0;         // reset to 0 for cd instances, later
  gas.inst_list[inst].flag.needs_spu_preload = 0;
  gas.inst_list[inst].flag.volume_touched = 1;
  gas.inst_list[inst].flag.stopping = 0;
  /*

*/
  // init the source-type specific stuff for this instance
  switch (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.src_type)
  {
    case SRC_TYPE_SPU: 
      gas.inst_list[inst].flag.ready = 1;
      gas_status.instances[inst].is_ready = 1;

      gas.inst_list[inst].src.spu.cur_addr = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].src.spu.addr + gas.inst_list[inst].offset;

      break;

    case SRC_TYPE_CD:
      gas.inst_list[inst].flag.ready = 0;
      gas_status.instances[inst].is_ready = 0;
      // we deal with the offset when we process thre preloading
      fifo_queue_push(&gas.cd_preloads_pending, inst);
      break;

    default:
      return 0;
  }

  return 1;
}
