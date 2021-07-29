// GAS command functions
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas.h" 
#include "GasSystem.h"
#include <sif.h>

extern GasStatus gas_status;
extern int g_load_ee_sema;


char gas_root_dir[256] = "PS2SOUND";
static int intr_oldstat; // for disable/enable intr

//should these be in the gas struct?
//i think not 
static int temp_buff_offset = 0;
static char using_persistant_temp = 0;
static int stash_fd = -1;
static int last_offset = -1;

extern int g_max_cd_stereo_streams;

#ifdef DEBUG_GOODIES
static int batch_no = 0;
#endif
void gas_play_queued(int instance);

void *gas_rpc_get_streaming_buffer()
{
  return gas.iop_buffers_top;
}


void gas_pre_stop_queued( int instance_id );

#define GAS_ADDING    0x1
#define GAS_PLAYING   0x2
#define GAS_STOPPING  0x4


// Ok.. this has never been tested.
#ifdef PARANOID

void gas_paranoid_check_instruction_list()
{
  int check_vals[GAS_MAX_INSTANCES][2];
  int size = fifo_instr_size(&gas.incomingGasInstructions);
  int i=0;
  for (i=0; i < GAS_MAX_INSTANCES; i++)
  {
    check_vals[i][0]  -1;
    check_vals[i][1] = 0;
  }

  for (i=0; i < size; i++)
  {
    GasInstruction *p =  fifo_instr_pop(&gas.incomingGasInstructions);;
    int instance_id = p->int_arg;;
    int cmd = p->rpc_command;
    
    if (check_vals[instance_id & GAS_INSTANCE_ID_MASK][0] == -1)
    {
      // ALL GOOD
      check_vals[instance_id & GAS_INSTANCE_ID_MASK][0] = instance_id;
      switch (cmd)
      {
        case GAS_RPC_PLAY_INSTANCE:
          check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] |= GAS_PLAYING;
          break;
        case GAS_RPC_STOP_INSTANCE:
          check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] |= GAS_STOPPING;
          break;
        case GAS_RPC_ADD_INSTANCE:
          check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] |= GAS_ADDING;
          break;
      }
      fifo_instr_push(&gas.incomingGasInstructions, p);
    } 

    // All following cases have 2 commands for the same inst slot
    else if (check_vals[instance_id & GAS_INSTANCE_ID_MASK][0] != instance_id)
    {
      // Mark both commands as bad
      
      gas_printf("WOAH... VERY BAD.  TWO COMMANDS ON gas.instructionlist with same slot but diff id's!  %d/%d\n", 
        check_vals[instance_id & GAS_INSTANCE_ID_MASK][0], 
        instance_id);

      check_vals[instance_id & GAS_INSTANCE_ID_MASK][0] = 0xBADDEAD;
      gas_stop_queued(p->int_arg & GAS_INSTANCE_ID_MASK);
    }
    else // if (check_vals[instance & GAS_INSTANCE_ID_MASK][0] == instance_id)
    {
      // Hmm.. this is a special case.  We have two commands for same instance
      switch (cmd)
      {
        case GAS_RPC_PLAY_INSTANCE:
          check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] |= GAS_PLAYING;
          break;
        case GAS_RPC_STOP_INSTANCE:
          check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] |= GAS_STOPPING;
          break;
        case GAS_RPC_ADD_INSTANCE:
          check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] |= GAS_ADDING;
          break;
      }
      
      if (check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] == (GAS_PLAYING | GAS_STOPPING))
      {
        check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] = GAS_STOPPING;
      }
      else if ((check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] == (GAS_ADDING | GAS_STOPPING)) ||
              (check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] == (GAS_ADDING | GAS_PLAYING)) ||
              (check_vals[instance_id & GAS_INSTANCE_ID_MASK][1] == (GAS_PLAYING | GAS_ADDING | GAS_STOPPING)))
      {
        check_vals[instance_id & GAS_INSTANCE_ID_MASK[0] = 0xBADDEAD;
      }
    }
  }

  size = fifo_instr_size(&gas.incomingGasInstructions);

  for (i=0; i < size; i++)
  { 
    GasInstruction *p =  fifo_instr_pop(&gas.incomingGasInstructions);
    int instance_id = p->int_arg;
    int cmd = p->rpc_command;
    int myCmd;
    if (check_vals[instance_id & GAS_INSTANCE_ID_MASK][0] != 0xBADDEAD)
    {
      switch (cmd)
      {
        case GAS_RPC_PLAY_INSTANCE:  myCmd = GAS_PLAYING;     break;
        case GAS_RPC_STOP_INSTANCE:  myCmd = GAS_STOPPING;    break;
        case GAS_RPC_ADD_INSTANCE:   myCmd = GAS_ADDING;      break;
      }
      if (!(myCmd & check_vals[instance_id & GAS_INSTANCE_ID_MASK][1]))
        p->rpc_command = GAS_RPC_STOP_INSTANCE;
      
      fifo_queue_push(&gas.incomingGasInstructions, p);
    }
  }
}

#endif



void gas_process_instruction_list()
{
  RECORDRA
#ifdef DEBUG_GOODIES
  batch_no++;
#endif

#ifdef PARANOID
  WaitSema(gas.instr_list_sema); 
  gas_paranoid_check_instruction_list();
  SignalSema(gas.instr_list_sema); 
#endif

  while(fifo_instr_size(&gas.incomingGasInstructions))
  {
    
    GasInstruction *p;
    WaitSema(gas.instr_list_sema); 
    p = fifo_instr_pop(&gas.incomingGasInstructions);
    switch(p->rpc_command)
    { 
      case GAS_RPC_ADD_INSTANCE:
        _gas_rpc_init_new_instance(p->short_arg & GAS_INSTANCE_ID_MASK, p->int_arg);
        break;
      case GAS_RPC_PLAY_INSTANCE:
        //printf("GAS PLAYING %s\n", gas.source_list[gas.inst_list[p->short_arg].source].filename);
        gas_play_queued(p->int_arg);
        break;
      case GAS_RPC_STOP_INSTANCE:
        gas_pre_stop_queued(p->int_arg);
        break;
    }
    SignalSema(gas.instr_list_sema);
  }
}


/***********************************************************
 *  A bunch of getters and setters
 *  
 *
 ************************************************************/

void gas_rpc_set_root_dir( GasRpcArgs *args )
{
  strcpy(gas_root_dir, args->string_arg);
}


int gas_rpc_get_source_volume( GasRpcArgs *args )
{
  // We assume that the left volume is the same as the 
  // right
  int source = args->int_arg[0];
  return volume_to_percentile(gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].voll);
}


int gas_rpc_source_type( GasRpcArgs *args ) 
{
  int source = args->int_arg[0];
  return gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].sound_type;
} 

int gas_rpc_source_looping( GasRpcArgs *args )
{
  int source = args->int_arg[0];

  return (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.loop?1:0);
}

unsigned int gas_rpc_source_length( GasRpcArgs *args )
{

  // So, we need to calc the length in seconds
  // So we first multiply the sound size * 3.5
  // to undo the compression effects.
  unsigned int source = args->int_arg[0];
  unsigned int freq = (((gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one*48000)/4096));
  unsigned long realSize = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].size*35/10;
  // We return the "Thousandths of seconds" because we have no floating point here
  // So multiply by 1000 
  
  // Divide by two for stereo tracks
  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
  {
    realSize = realSize / 2;
  }

  // We have 16bit samples, so we divide by 2 bytes to get
  // the number of samples
  realSize = realSize / 2;
  realSize *= 100;
  // Now, size (in samples) / samples per sec = size in sec
  realSize = realSize / freq;
  realSize*=10;
  
  return realSize;
}


unsigned int gas_rpc_get_sound_position( GasRpcArgs *args )
{
  unsigned int instance_id = args->int_arg[0];
  unsigned int inst = instance_id & GAS_INSTANCE_ID_MASK;
  unsigned int source = gas.inst_list[inst].source;
  unsigned int pos = 0, nax = 0;
  unsigned int freq = (((gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one*48000)/4096));
  unsigned int addr = 0;
  if (gas.inst_list[inst].instance_id != instance_id)
  {
    gas_printf("Error.. instance_id in gas_rpc_get_sound_position does not match gas' version\n");
    return 0;
  }

  if (!gas.inst_list[inst].played)
    return 0;
  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.src_type == SRC_TYPE_SPU)
  {
    nax = gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
    addr = (unsigned char *)SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * 
      SPU_BUFFER_SIZE) + ((gas.inst_list[inst].spu_reload) * SPU_BUFFER_HALF);
    pos = nax - addr;
       
    pos *= 35/10;

    // times 100 ( of the 1000 we use to move up to milliseconds)
    // / 2 for 16 bit samples
    pos *= 500;
    pos = pos / freq;
    return pos;
  }
  else
  {
    // How much we've streamed
    pos = gas.inst_list[inst].src.cd.total_bytes_played;

    nax = gas_update_get_NAX(gas.inst_list[inst].flag.corel, (gas.inst_list[inst].flag.voicel<<1));
    addr = (unsigned char *)SPU_MEMORY_TOP + (gas.inst_list[inst].spu_bufl * 
      SPU_BUFFER_SIZE) + ((gas.inst_list[inst].spu_reload) * SPU_BUFFER_HALF);
    pos += (int)nax - addr;

    // uncompress
    pos *= 35/10;
    // times 100 ( of the 1000 we use to move up to milliseconds)
    // 2 for 16 bit samples
    pos *= 50;
    pos = pos/freq;
    // 10*100 = 1000 = milliseconds
    pos *= 10; 
    return pos;
    

  }
  
}


unsigned int gas_rpc_source_padded_length( GasRpcArgs *args )
{

  // So, we need to calc the length in seconds
  // So we first multiply the sound size * 3.5
  // to undo the compression effects.
  unsigned int source = args->int_arg[0];
  unsigned int freq = (((gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one*48000)/4096));
  unsigned int realSize = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].padsize*35/10;
  // We return the "Thousandths of seconds" because we have no floating point here
  // So multiply by 1000 
  
  // Divide by two for stereo tracks
  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
  {
    realSize = realSize / 2;
  }

  // We have 16bit samples, so we divide by 2 bytes to get
  // the number of samples
  realSize *= 50;
  // Now, size (in samples) / samples per sec = size in sec
  realSize = realSize / freq;
  realSize*=10;
  
  return realSize;
}

int gas_rpc_source_freq( GasRpcArgs *args )
{
  int source = args->int_arg[0];
  return ((gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one*48000)/4096);
}

void gas_rpc_set_host_stream(GasRpcArgs *args )
{
  gas.stream_from_host = args->int_arg[0]>0?1:0;
}





/********************************************
 *  Some buffer loading stuff
 ********************************************/

void *gas_rpc_load_to_buffer( GasRpcArgs *args )
{
  int counter = 0;
  int offset = args->int_arg[0];
  int size = args->int_arg[1];

  char *filename = args->string_arg;
  RECORDRA
  WaitSema( g_load_ee_sema );
  temp_buff_offset = offset;

  if( size > (g_max_cd_stereo_streams * CD_STEREO_BUFFER_SIZE) + (MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE) /*+ 123104*/ )
    using_persistant_temp = 0;
  else
    using_persistant_temp = 1;

  if( stash_fd > 0 )
  {
    close( stash_fd );
    stash_fd = -1;
  }
  while ((stash_fd < 0) && (counter++ < 5))
    stash_fd = _gas_rpc_get_fd( filename );

  if( stash_fd < 0 )
  {
    gas_printf( "Could not open %s\n", filename );
    return NULL;
  }

  if( using_persistant_temp )
  {
    lseek(stash_fd, offset, SEEK_SET);
    if( read( stash_fd, (void *)gas.iop_buffers_top, size ) != size )
    {
      gas_printf( "read size mismatch!! Pay attention to me dork!\n" );
    }
    FlushDcache(); // ensure this is all wrote back
    close( stash_fd );
    stash_fd = -1;
  }

  last_offset = -1;
  return (void *)gas.iop_buffers_top;
}

int gas_rpc_trans_to_ee( GasRpcArgs *args )
{
  unsigned int offset = args->int_arg[0];
  unsigned int dest  = args->int_arg[1];
  unsigned int size  = args->int_arg[2];
  int did;
  sceSifDmaData ddata;
  RECORDRA

  if( using_persistant_temp == 0 )
  {
    int bytes_read;

    if( stash_fd < 0 )
    {
      gas_printf( "Ack! stash file isn't open!! Whats going on?\nThis a big error\n" );
      return -1;
    }

    if( last_offset != offset )
    {
      lseek( stash_fd, temp_buff_offset + offset, SEEK_SET );
      bytes_read = read( stash_fd, (void *)gas.iop_buffers_top, size );
      FlushDcache(); // ensure this is all wrote back
      last_offset = offset;

      if( bytes_read != size )
      {
        gas_printf( "bytes_read != size\n" );
        return -1;
      }
    }
    
    ddata.data = (int)gas.iop_buffers_top;
  }
  else
    ddata.data = (int)(gas.iop_buffers_top + offset);

  ddata.addr = dest;
  ddata.size = size;
  ddata.mode = 0;

  if( ddata.data % 4 ) gas_printf( "data misaligned\n" );
  if( ddata.addr % 16 ) gas_printf( "addr misaligned\n" );
  if( ddata.size % 4 ) gas_printf( "size misaligned\n" );

//  gas_printf( "Setting dma trans\n" );
//  gas_printf( "Src: 0x%X\nDest: 0x%X\nSize: %u\n", ddata.data, ddata.addr, ddata.size );
  CpuSuspendIntr( &intr_oldstat );
  did = sceSifSetDma( &ddata, 1 );
  
//  gas_printf( "did: %d\n", did );

  while (sceSifDmaStat(did) >= 0);
  CpuResumeIntr( intr_oldstat );
//  gas_printf( "transfer complete\n" );

  return size;
}

// turn off gas so someone else can use the sound chip
void gas_rpc_partial_shutdown( GasRpcArgs *args )
{
  RECORDRA
  gas_printf("Shutting down core GAS module functioning\n");
  WaitSema( g_load_ee_sema );
}

// turn gas back on, after letting someone else use the sound chip
void gas_rpc_partial_init( GasRpcArgs *args )
{
  RECORDRA
  gas_printf("Re-initializing GAS module\n");
  _gas_rpc_partial_init();

  SignalSema( g_load_ee_sema );
}


int gas_rpc_query_mem()
{
  RECORDRA
  return QueryTotalFreeMemSize();
}

extern void my_strupr(char *);
//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   S O U R C E 
//
// returns the source id of the newly added source, this is used to create instances 
// returns -1 if the source name cannot be found or if the source cannot be created.
int gas_rpc_add_source( GasRpcArgs *args )
{
  int i,j=0;
  char found= 0;
  int in_heap = -1;
  char *filename = args->string_arg;
  my_strupr(filename);

  RECORDRA
  // search the sources for one called 'filename'
  while (j < gas.num_heaps && !found)
  {
    for (i=0; i<gas.spu_heaps[j].source_list_count; ++i)
    {
#ifdef NSL_LOAD_SOURCE_BY_NAME
      if (strcmp(filename, gas.source_list[j][i].filename) == 0)
      {
#elif defined NSL_LOAD_SOURCE_BY_ALIAS
     if (gas.source_list[j][i].aliasID == args->int_arg[0])
	 {
#endif
        found = 1;
        in_heap = j;
        break;
      }
    }
    j++;
  }

  if (!found)
  {
    SignalSema(gas.system_state_sema);
    return -1;
  }

  WaitSema(gas.system_state_sema);
  RECORDRA

  // don't allow multiple loads of the same source
  if (gas.source_list[in_heap][i].flag.loaded == 1)
  {
    SignalSema(gas.system_state_sema);
    return MAKE_SOURCE_ID(in_heap, i);
  }

  // check if there is room for this source
  if (gas.source_list[in_heap][i].flag.src_type == SRC_TYPE_SPU && 
      gas.spu_heap_curr + gas.source_list[in_heap][i].padsize >= gas.spu_heap_end)
  {
    SignalSema(gas.system_state_sema);
    return -1;
  }
  
  // add the source differently depending on its src_type
  switch (gas.source_list[in_heap][i].flag.src_type)
  {
    case SRC_TYPE_SPU: i = _gas_rpc_add_spu_source(in_heap, i); break;
    case SRC_TYPE_CD:  i = _gas_rpc_add_cd_source(in_heap, i);  break;
  }

  SignalSema(gas.system_state_sema);

  // return the index of the GasSource[] that this entry uses
  return i;
}

//-----------------------------------------------------------------------------------
// G A S   R P C   S E T   S T R E A M   F I L E N A M E
//
int gas_rpc_open_rawstream( GasRpcArgs *args )
{
	return _gas_open_rawstream( args->string_arg, args->int_arg[0], args->int_arg[1], args->int_arg[2], args->int_arg[3] );
}

int  gas_rpc_get_reverb( GasRpcArgs *args )
{
	int i = 0;
	int heap = 0;
	int found = 0;

	char *filename = args->string_arg;
	my_strupr(filename);

	while (heap < gas.num_heaps && !found)
	{
		while (i < gas.spu_heaps[heap].source_list_count && !found)
		{

#ifdef NSL_LOAD_SOURCE_BY_NAME

			if (strcmp(filename, gas.source_list[heap][i].filename) == 0)
			{

#elif defined NSL_LOAD_SOURCE_BY_ALIAS

			if (gas.source_list[heap][i].aliasID == args->int_arg[0])
			{
#endif

				found = 1;
			}
			else
			{
				++i;
			}
		}

		if (!found)
		{
			++heap;
		}
	}

	return gas.source_list[heap][i].flag.reverb;
}


int  gas_rpc_set_reverb( GasRpcArgs *args )
{
	int i = 0;
	int heap = 0;
	int found = 0;

	char *filename = args->string_arg;
	my_strupr(filename);

	while (heap < gas.num_heaps && !found)
	{
		while (i < gas.spu_heaps[heap].source_list_count && !found)
		{

#ifdef NSL_LOAD_SOURCE_BY_NAME

			if (strcmp(filename, gas.source_list[heap][i].filename) == 0)
			{

#elif defined NSL_LOAD_SOURCE_BY_ALIAS

			if (gas.source_list[heap][i].aliasID == args->int_arg[0])
			{
#endif

				found = 1;
			}
			else
			{
				++i;
			}
		}

		if (!found)
		{
			++heap;
		}
	}

	gas.source_list[heap][i].flag.reverb = args->int_arg[1];

	return 0;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   I N S T A N C E
//
// returns the instance id of the newly added instance, this is used to refer to this
// instance for modifying, playing, etc.
// returns -1 if the source cannot be found or if the instance cannot be created.
int gas_rpc_add_instance( GasRpcArgs *args )
{
  int source = args->int_arg[0];
  int inst_slot, ret_val,v1 = -1, v2 = -1, core1, core2, voice1, voice2;
  GasInstruction g;
#ifdef DEBUG_OUTPUT
  int i;
#endif
  

#ifdef PROFILER_OUTPUT
  u_int wait_time;
  u_int final_time;
#endif

  RECORDRA
  if (GET_SOURCE_BANK(source) > gas.num_heaps || 
      GET_SOURCE_SLOT(source) >= gas.spu_heaps[GET_SOURCE_BANK(source)].source_list_count ||
      source < 0 || 
      gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.loaded == 0)
  {
    return -1;
  }
//if (strcmp(gas.source_list[source].filename, "SILENT") == 0)
//  return -1;

#ifdef PROFILER_OUTPUT
  wait_time = gas_debug_profile_start();
#endif
  WaitSema(gas.instr_list_sema);
#ifdef PROFILER_OUTPUT
  final_time = gas_debug_profile_stop(wait_time);
#endif
  RECORDRA
#ifdef PROFILER_OUTPUT
  gas_printf("Sema wait time %u (%ums)\n", final_time, final_time / GAS_PROFILER_MILLISECOND_MULTIPLIER);
#endif

#ifdef DEBUG_OUTPUT
  gas_printf("%s: ", gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].filename);
#endif

  inst_slot = _gas_find_and_claim_free_instance_slot();
  //if (gas.free_instances.count > 0)
  /*if (i >= 0)
  {
    // use a recycled instance
    //i = fifo_queue_pop(&gas.free_instances);

#ifdef DEBUG_OUTPUT
    gas_printf("using recycled instance %d\n", i);
#endif
  }
  else if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
  {
    // try to reclaim one
    //_gas_update_reclaim_voices();
#ifdef DEBUG_OUTPUT
    gas_printf("Attempting to reclaim a voice or two\n");
#endif
    //if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
    //{
      SignalSema(gas.system_state_sema);
      return -1;
    //}
  }
  else
  {
    // add the instance to the list of instances
    i = gas.inst_list_count;
#ifdef DEBUG_OUTPUT
    gas_printf("creating new instance %d\n", i);
#endif
  }*/

    
  if ((inst_slot < 0) || (inst_slot >= PS2_MAX_VOICES))
  {
    SignalSema(gas.instr_list_sema);
    return -1;
  }
  if (inst_slot == gas.inst_list_count)
    gas.inst_list_count++;

  gas.inst_list[inst_slot].instance_id += GAS_INSTANCE_ID_INCREMENT;

  // This is being set before the voice is secured because it effects which voice should be secured.
  gas.inst_list[inst_slot].source = source;

  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
  {

    // This proc does a quick search for 2
    if (_gas_rpc_get_voice_stereo(inst_slot, &core1, &voice1, &core2, &voice2) == 0)  
    {

#ifdef DEBUG_OUTPUT
      gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
      _gas_debug_voice_status_dump();
#endif
      _gas_release_instance_slot(inst_slot);
      SignalSema(gas.instr_list_sema);
      return -1;
    }

    gas.inst_list[inst_slot].flag.corel         = core1;
    gas.inst_list[inst_slot].flag.voicel        = voice1;
    gas.inst_list[inst_slot].flag.corer         = core2;
    gas.inst_list[inst_slot].flag.voicer        = voice2;

    v1 = (core1 * PS2_MAX_VOICES_PER_CORE) + voice1;
    v2 = (core2 * PS2_MAX_VOICES_PER_CORE) + voice2;
    
    // Shouldn't need to do this..
    if ((gas.voice_used[v2] != -1) || (gas.voice_used[v1] != -1))
    {

#ifdef DEBUG_OUTPPUT
      gas_printf("Hey man, this voice (one of 2 stereo) is already used (%d) by 0x%x\n", v1, gas.voice_used[v1]);
#endif

#ifdef HALT_ON_FAIL
      while(1);
#endif

    }
    
    gas.voice_used[v1] = gas.inst_list[inst_slot].instance_id;
    gas.voice_used[v2] = gas.inst_list[inst_slot].instance_id;
  }
  else // Mono
  {

    if (_gas_rpc_get_voice(inst_slot, &core1, &voice1) == 0) 
    {
#ifdef DEBUG_OUTPUT
      gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
      _gas_debug_voice_status_dump();
#endif
      _gas_release_instance_slot(inst_slot);
      SignalSema(gas.instr_list_sema);
      return -1;
    }

    gas.inst_list[inst_slot].flag.corel = core1;
    gas.inst_list[inst_slot].flag.voicel = voice1;

    v1 = (core1 * PS2_MAX_VOICES_PER_CORE) + voice1;
    if ((gas.voice_used[v1] != -1))
    {

#ifdef DEBUG_OUTPPUT
      gas_printf("Hey man, this voice is already used (%d) by 0x%x\n", v1, gas.voice_used[v1]);
#endif

#ifdef HALT_ON_FAIL
      while(1);
#endif

    }
    gas.voice_used[v1] = gas.inst_list[inst_slot].instance_id;

  }
  
  gas.inst_list[inst_slot].flag.stereo = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo;
  switch (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.src_type)
  {
    case SRC_TYPE_SPU: break;

    case SRC_TYPE_CD:

		// hack the source with user specified START / LENGTH of streamed data
		if( args->int_arg[2] && args->int_arg[3] )
		{
			int samples = args->int_arg[3];
			int bytes	= ((samples + 15)>>4) * 28;
			int padded	= (bytes + PS2_CD_SECTOR_SIZE - 1) & ~( PS2_CD_SECTOR_SIZE - 1 );
			gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].src.cd.start_sector = args->int_arg[2] / PS2_CD_SECTOR_SIZE;
			gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].padsize = padded;
			gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].size = bytes;

			//gas_printf( "MALKIA: streamLoad: ofs=%p len=%p\n", args->int_arg[2], args->int_arg[3]);
		}

      // check spu bufs
#ifdef DEBUG_OUTPUT
      gas_printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
#endif

      if ( gas.free_spu_bufs.count < (gas.inst_list[inst_slot].flag.stereo + 1) )
      {
        _gas_release_voice(inst_slot, v1);
        gas.voice_used[v1] = -1;
        if (gas.inst_list[inst_slot].flag.stereo == 1)
        {
          _gas_release_voice(inst_slot, v2);
          gas.voice_used[v2] = -1;
        }
        _gas_release_instance_slot(inst_slot);
        SignalSema(gas.instr_list_sema);
        return -1;
      }
      //check cd bufs
      if (gas.inst_list[inst_slot].flag.stereo == 1)
      {

#ifdef DEBUG_OUTPUT
        gas_printf("\t\t\t\tfree stereo bufs %d\n", gas.free_cd_stereo_bufs.count);
#endif

        if (gas.free_cd_stereo_bufs.count < 1)
        {
          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          _gas_release_voice(inst_slot, v2);
          gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }

        gas.inst_list[inst_slot].iop_buf = fifo_queue_pop(&gas.free_cd_stereo_bufs);
        if (gas.inst_list[inst_slot].iop_buf == -1)
        {

#ifdef DEBUG_OUTPUT
          printf("UGLY MESSAGE(rpc): Couldn't get an iop cd stereo buffer.  Queue contents:\n");
          gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_stereo_bufs.start, gas.free_cd_stereo_bufs.end, gas.free_cd_stereo_bufs.count);
          for (i=0; i<gas.free_cd_stereo_bufs.queue_max; ++i)
          {
            gas_printf("%d ", gas.free_cd_stereo_bufs.queue[i]);
          }
          gas_printf("\n");
#endif

          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          _gas_release_voice(inst_slot, v2);
          gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }
      }
      else  // MONO
      {

#ifdef DEBUG_OUTPUT
        gas_printf("\t\t\t\tfree mono bufs %d\n", gas.free_cd_mono_bufs.count);
#endif

        if (gas.free_cd_mono_bufs.count < 1)
        {
          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          //_gas_release_voice(inst_slot, v2);
          //gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }

        gas.inst_list[inst_slot].iop_buf = fifo_queue_pop(&gas.free_cd_mono_bufs);

        if (gas.inst_list[inst_slot].iop_buf == -1)
        {
#ifdef DEBUG_OUTPUT
          printf("UGLY MESSAGE(rpc): Couldn't get an iop cd mono buffer.  Queue contents:\n");
          gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_mono_bufs.start, gas.free_cd_mono_bufs.end, gas.free_cd_mono_bufs.count);
          for (i=0; i<gas.free_cd_mono_bufs.queue_max; ++i)
          {
            gas_printf("%d ", gas.free_cd_mono_bufs.queue[i]);
          }
          gas_printf("\n");
#endif
          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          // _gas_release_voice(inst_slot, v2);
          // gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }
      }
      break;

    default:
      _gas_release_voice(inst_slot, v1);
      gas.voice_used[v1] = -1;
      if (gas.voice_used[v2] == inst_slot)
      {
        _gas_release_voice(inst_slot, v2);
        gas.voice_used[v2] = -1;
      }
      _gas_release_instance_slot(inst_slot);  
      SignalSema(gas.instr_list_sema);
      return -1;
  }
 
  if ((voice1 < 0) || (voice1 > 23))
  {
    gas_printf("WOAH... BIG PROBS (1)\n");
  }
  else
  {
    if (gas.voice_used[v1] == -1)
      gas_printf("WOAH... BIG PROBS (2)\n");
  }
 
  if (gas.inst_list[inst_slot].flag.stereo)
  {
    if ((voice2 < 0) || (voice2 > 23))
    {
      gas_printf("WOAH... BIG PROBS (3)\n");
    }
    else
    {
      if (gas.voice_used[v2]==-1)
        gas_printf("WOAH... BIG PROBS (4)\n");
    }
  }

  // Init

  gas.inst_list[inst_slot].source = source;
  gas.inst_list[inst_slot].pause_count = 1;
  gas.inst_list[inst_slot].played = 0;
  gas.inst_list[inst_slot].flag.used = 1;
  gas_status.instances[inst_slot].is_valid = 1;

  // We should have buffers and voices now.  
  // Add it to the 
  g.int_arg = source;
  g.short_arg = gas.inst_list[inst_slot].instance_id;
  g.rpc_command = GAS_RPC_ADD_INSTANCE;
  fifo_instr_push(&gas.incomingGasInstructions, &g);
  


//_gas_debug_voice_status_dump();
  /*if (_gas_rpc_init_new_instance(inst_slot, source) == 0)
  {
    SignalSema(gas.system_state_sema);
    return -1;
  }*/

  // return the index of the GasSource[] that this entry uses
  gas.inst_list[inst_slot].offset = 0;

  ret_val = gas.inst_list[inst_slot].instance_id;

  if (gas.inst_list[inst_slot].flag.stereo)
    ret_val |= GAS_INSTANCE_STEREO_FLAG_BIT;

  SignalSema(gas.instr_list_sema);
  gas_process_instruction_list();

  return ret_val;
}





//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   I N S T A N C E
//
// returns the instance id of the newly added instance, this is used to refer to this
// instance for modifying, playing, etc.
// returns -1 if the source cannot be found or if the instance cannot be created.
int gas_rpc_add_instance_with_offset( GasRpcArgs *args )
{
  int source = args->int_arg[0];
  int inst_slot, ret_val,v1 = -1, v2 = -1, core1, core2, voice1, voice2;
  int offset = args->int_arg[1];
  GasInstruction g;
#ifdef DEBUG_OUTPUT
  int i;
#endif
  

#ifdef PROFILER_OUTPUT
  u_int wait_time;
  u_int final_time;
#endif

  RECORDRA
  if (GET_SOURCE_SLOT(source) >= gas.spu_heaps[GET_SOURCE_BANK(source)].source_list_count || source < 0 || gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.loaded == 0)
  {
    return -1;
  }
//if (strcmp(gas.source_list[source].filename, "SILENT") == 0)
//  return -1;

#ifdef PROFILER_OUTPUT
  wait_time = gas_debug_profile_start();
#endif
  WaitSema(gas.instr_list_sema);
#ifdef PROFILER_OUTPUT
  final_time = gas_debug_profile_stop(wait_time);
#endif
  RECORDRA
#ifdef PROFILER_OUTPUT
  gas_printf("Sema wait time %u (%ums)\n", final_time, final_time / GAS_PROFILER_MILLISECOND_MULTIPLIER);
#endif

#ifdef DEBUG_OUTPUT
  gas_printf("%s: ", gas.source_list[source].filename);
#endif

  inst_slot = _gas_find_and_claim_free_instance_slot();
  //if (gas.free_instances.count > 0)
  /*if (i >= 0)
  {
    // use a recycled instance
    //i = fifo_queue_pop(&gas.free_instances);

#ifdef DEBUG_OUTPUT
    gas_printf("using recycled instance %d\n", i);
#endif
  }
  else if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
  {
    // try to reclaim one
    //_gas_update_reclaim_voices();
#ifdef DEBUG_OUTPUT
    gas_printf("Attempting to reclaim a voice or two\n");
#endif
    //if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
    //{
      SignalSema(gas.system_state_sema);
      return -1;
    //}
  }
  else
  {
    // add the instance to the list of instances
    i = gas.inst_list_count;
#ifdef DEBUG_OUTPUT
    gas_printf("creating new instance %d\n", i);
#endif
  }*/

    
  if ((inst_slot < 0) || (inst_slot >= PS2_MAX_VOICES))
  {
    SignalSema(gas.instr_list_sema);
    return -1;
  }
  if (inst_slot == gas.inst_list_count)
    gas.inst_list_count++;

  gas.inst_list[inst_slot].instance_id += GAS_INSTANCE_ID_INCREMENT;

  // This is being set before the voice is secured because it effects which voice should be secured.
  gas.inst_list[inst_slot].source = source;

  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
  {

    // This proc does a quick search for 2
    if (_gas_rpc_get_voice_stereo(inst_slot, &core1, &voice1, &core2, &voice2) == 0)  
    {

#ifdef DEBUG_OUTPUT
      gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
      _gas_debug_voice_status_dump();
#endif
      _gas_release_instance_slot(inst_slot);
      SignalSema(gas.instr_list_sema);
      return -1;
    }

    gas.inst_list[inst_slot].flag.corel         = core1;
    gas.inst_list[inst_slot].flag.voicel        = voice1;
    gas.inst_list[inst_slot].flag.corer         = core2;
    gas.inst_list[inst_slot].flag.voicer        = voice2;

    v1 = (core1 * PS2_MAX_VOICES_PER_CORE) + voice1;
    v2 = (core2 * PS2_MAX_VOICES_PER_CORE) + voice2;
    
    // Shouldn't need to do this..
    if ((gas.voice_used[v2] != -1) || (gas.voice_used[v1] != -1))
    {

#ifdef DEBUG_OUTPPUT
      gas_printf("Hey man, this voice (one of 2 stereo) is already used (%d) by 0x%x\n", v1, gas.voice_used[v1]);
#endif

#ifdef HALT_ON_FAIL
      while(1);
#endif

    }
    
    gas.voice_used[v1] = gas.inst_list[inst_slot].instance_id;
    gas.voice_used[v2] = gas.inst_list[inst_slot].instance_id;
  }
  else // Mono
  {

    if (_gas_rpc_get_voice(inst_slot, &core1, &voice1) == 0) 
    {
#ifdef DEBUG_OUTPUT
      gas_printf("Houston, we have a problem, there are only 48 instances, and 48 voices, so what's up?\n");
      _gas_debug_voice_status_dump();
#endif
      _gas_release_instance_slot(inst_slot);
      SignalSema(gas.instr_list_sema);
      return -1;
    }

    gas.inst_list[inst_slot].flag.corel = core1;
    gas.inst_list[inst_slot].flag.voicel = voice1;

    v1 = (core1 * PS2_MAX_VOICES_PER_CORE) + voice1;
    if ((gas.voice_used[v1] != -1))
    {

#ifdef DEBUG_OUTPPUT
      gas_printf("Hey man, this voice is already used (%d) by 0x%x\n", v1, gas.voice_used[v1]);
#endif

#ifdef HALT_ON_FAIL
      while(1);
#endif

    }
    gas.voice_used[v1] = gas.inst_list[inst_slot].instance_id;

  }
  
  gas.inst_list[inst_slot].flag.stereo = gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo;
  switch (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.src_type)
  {
    case SRC_TYPE_SPU: break;

    case SRC_TYPE_CD:
      // check spu bufs
#ifdef DEBUG_OUTPUT
      gas_printf("\t\t\t\tfree spu bufs %d\n", gas.free_spu_bufs.count);
#endif

      if ( gas.free_spu_bufs.count < (gas.inst_list[inst_slot].flag.stereo + 1) )
      {
        _gas_release_voice(inst_slot, v1);
        gas.voice_used[v1] = -1;
        if (gas.inst_list[inst_slot].flag.stereo == 1)
        {
          _gas_release_voice(inst_slot, v2);
          gas.voice_used[v2] = -1;
        }
        _gas_release_instance_slot(inst_slot);
        SignalSema(gas.instr_list_sema);
        return -1;
      }
      //check cd bufs
      if (gas.inst_list[inst_slot].flag.stereo == 1)
      {

#ifdef DEBUG_OUTPUT
        gas_printf("\t\t\t\tfree stereo bufs %d\n", gas.free_cd_stereo_bufs.count);
#endif

        if (gas.free_cd_stereo_bufs.count < 1)
        {
          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          _gas_release_voice(inst_slot, v2);
          gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }

        gas.inst_list[inst_slot].iop_buf = fifo_queue_pop(&gas.free_cd_stereo_bufs);
        if (gas.inst_list[inst_slot].iop_buf == -1)
        {

#ifdef DEBUG_OUTPUT
          printf("UGLY MESSAGE(rpc): Couldn't get an iop cd stereo buffer.  Queue contents:\n");
          gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_stereo_bufs.start, gas.free_cd_stereo_bufs.end, gas.free_cd_stereo_bufs.count);
          for (i=0; i<gas.free_cd_stereo_bufs.queue_max; ++i)
          {
            gas_printf("%d ", gas.free_cd_stereo_bufs.queue[i]);
          }
          gas_printf("\n");
#endif

          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          _gas_release_voice(inst_slot, v2);
          gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }
      }
      else  // MONO
      {

#ifdef DEBUG_OUTPUT
        gas_printf("\t\t\t\tfree mono bufs %d\n", gas.free_cd_mono_bufs.count);
#endif

        if (gas.free_cd_mono_bufs.count < 1)
        {
          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          //_gas_release_voice(inst_slot, v2);
          //gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }

        gas.inst_list[inst_slot].iop_buf = fifo_queue_pop(&gas.free_cd_mono_bufs);

        if (gas.inst_list[inst_slot].iop_buf == -1)
        {
#ifdef DEBUG_OUTPUT
          printf("UGLY MESSAGE(rpc): Couldn't get an iop cd mono buffer.  Queue contents:\n");
          gas_printf("Queue contents s(%d) e(%d) count(%d): ", gas.free_cd_mono_bufs.start, gas.free_cd_mono_bufs.end, gas.free_cd_mono_bufs.count);
          for (i=0; i<gas.free_cd_mono_bufs.queue_max; ++i)
          {
            gas_printf("%d ", gas.free_cd_mono_bufs.queue[i]);
          }
          gas_printf("\n");
#endif
          _gas_release_voice(inst_slot, v1);
          gas.voice_used[v1] = -1;
          // _gas_release_voice(inst_slot, v2);
          // gas.voice_used[v2] = -1;
          _gas_release_instance_slot(inst_slot);
          SignalSema(gas.instr_list_sema);
          return -1;
        }
      }
      break;

    default:
      _gas_release_voice(inst_slot, v1);
      gas.voice_used[v1] = -1;
      if (gas.voice_used[v2] == inst_slot)
      {
        _gas_release_voice(inst_slot, v2);
        gas.voice_used[v2] = -1;
      }
      _gas_release_instance_slot(inst_slot);  
      SignalSema(gas.instr_list_sema);
      return -1;
  }
 
  if ((voice1 < 0) || (voice1 > 23))
  {
    gas_printf("WOAH... BIG PROBS (1)\n");
  }
  else
  {
    if (gas.voice_used[v1] == -1)
      gas_printf("WOAH... BIG PROBS (2)\n");
  }
 
  if (gas.inst_list[inst_slot].flag.stereo)
  {
    if ((voice2 < 0) || (voice2 > 23))
    {
      gas_printf("WOAH... BIG PROBS (3)\n");
    }
    else
    {
      if (gas.voice_used[v2]==-1)
        gas_printf("WOAH... BIG PROBS (4)\n");
    }
  }

  // Init

  gas.inst_list[inst_slot].source = source;
  gas.inst_list[inst_slot].pause_count = 1;
  gas.inst_list[inst_slot].played = 0;
  gas.inst_list[inst_slot].flag.used = 1;
  gas_status.instances[inst_slot].is_valid = 1;

  // We should have buffers and voices now.  
  // Add it to the 
  g.int_arg = source;
  g.short_arg = gas.inst_list[inst_slot].instance_id;
  g.rpc_command = GAS_RPC_ADD_INSTANCE;
  fifo_instr_push(&gas.incomingGasInstructions, &g);
  


//_gas_debug_voice_status_dump();
  /*if (_gas_rpc_init_new_instance(inst_slot, source) == 0)
  {
    SignalSema(gas.system_state_sema);
    return -1;
  }*/

  // return the index of the GasSource[] that this entry uses
  
  if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.src_type == SRC_TYPE_SPU)
  {
    
    // SPU
    //offset is in milliseconds (viewed as uncompressed)
    // offset *(10compressed/35 uncompressed) *1s/1000ms *freq (samples/sec) * 2 bytes / sample = offset in bytes!
    
    unsigned int byte_offset = offset;
    unsigned int freq = (((gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one*48000)/4096));
    
    // re ordered so as to not loose precision
    // 16 bit samples (2 bytes per sample)
    byte_offset *= 2;
    // convert between time and samples
    byte_offset*= freq/1000;
    // compress
    byte_offset*=10;
    byte_offset/=35;
    // convert to seconds

    // This is set for CD preloads
    // ignore it here
    gas.inst_list[inst_slot].offset = byte_offset;
  }
  else
  {
    unsigned int byte_offset = offset;
    unsigned int freq = (((gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one*48000)/4096));
    //offset is in milliseconds (viewed as uncompressed)
    // offset *(10compressed/35 uncompressed) *1s/1000ms *freq (samples/sec) * 2 bytes / sample = offset in bytes!

    // re ordered so as to not loose precision
    // 16 bit samples (2 bytes per sample, 4 if stereo)
    byte_offset *= 2;
    // May be stereo.. 
    if (gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].flag.stereo)
      byte_offset *= 2;
    // convert between time and samples
    byte_offset*= freq/1000;
    // compress
    byte_offset*=10;
    byte_offset/=35;
    // convert to seconds
    
    
    // calc the offset in sectors
    gas.inst_list[inst_slot].offset = byte_offset / PS2_CD_SECTOR_SIZE;

  }


  ret_val = gas.inst_list[inst_slot].instance_id;
  if (gas.inst_list[inst_slot].flag.stereo)
    ret_val |= GAS_INSTANCE_STEREO_FLAG_BIT;

  SignalSema(gas.instr_list_sema);
  gas_process_instruction_list();

  return ret_val;
}






int _gas_ready_for_play( int instance_id )
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;

  if (gas.inst_list[inst].instance_id != instance_id)
  {
    return 0;
  }

  // Check those voices
  if (gas.inst_list[inst].flag.voicel == VNUM_NO_VOICE)
  { 
    return 0;
  }
  if (gas.inst_list[inst].flag.stereo && 
      (gas.inst_list[inst].flag.voicer == VNUM_NO_VOICE))
  {
    return 0;
  }


  if (inst >= gas.inst_list_count || inst < 0)
  {
    return 0;
  }

  if (_gas_check_inst_playing(inst))
  {
    return 0; // instance already playing
  }

  return 1;
}

//-----------------------------------------------------------------------------------
// G A S   R P C   P L A Y   I N S T A N C E
//
int gas_rpc_play_instance( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  unsigned short volume_left = args->int_arg[1];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  GasInstruction g;
  RECORDRA

  WaitSema(gas.instr_list_sema);
  RECORDRA
  
  if (!_gas_ready_for_play(instance_id))
  {
    SignalSema(gas.instr_list_sema);
    return 1;
  }

  _gas_update_instance_volume( inst, args->int_arg[1], args->int_arg[2] );
  // force an update in _gas_update_volume
  g.int_arg = instance_id;
  //g.short_arg = inst;
  g.rpc_command = GAS_RPC_PLAY_INSTANCE;
  fifo_instr_push(&gas.incomingGasInstructions, &g);

  SignalSema(gas.instr_list_sema);
  return 1;
}

void gas_play_queued(int instance_id)
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  
  if (!_gas_ready_for_play(instance_id))
  {
#ifdef DEBUG_OUTPUT
    gas_printf("Something funny while playing from queue (me: %d  gas: %d)\n", instance_id, gas.inst_list[inst].instance_id);
#endif
    return;
  }

  _gas_update_volume(inst);

#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Play\n", gas.source_list[gas.inst_list[inst].source].filename, gas.inst_list[inst].instance_id);
#endif

//_gas_debug_voice_status_dump();

  switch (gas.inst_list[inst].flag.src_type)
  {
    case SRC_TYPE_SPU: 
      _gas_rpc_play_spu_instance(inst);
      break;
    case SRC_TYPE_CD:
      _gas_rpc_play_cd_instance(inst);
      break;
    default:
  }
#ifdef DEBUG_GOODIES
  gas.inst_list[inst].batch = batch_no;
#endif
  gas.inst_list[inst].played = 1;
  gas_status.instances[inst].is_playing = 1;
  gas_status.instances[inst].is_ready = 1;
  gas_status.instances[inst].is_valid = 1;


}


// For the queued stops

int gas_rpc_queue_stop ( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  GasInstruction g;

  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;
  g.int_arg = instance_id;
  g.rpc_command = GAS_RPC_STOP_INSTANCE;
  fifo_instr_push(&gas.incomingGasInstructions, &g);
  return 1;
}

void gas_pre_stop_queued( int instance_id )
{
  int inst = instance_id & GAS_INSTANCE_ID_MASK;

  // don't stop it right away, ramp it down and mark it for death
  if (gas.inst_list[inst].flag.used == 1)
  {
    gas.inst_list[inst].flag.stopping = 1;
    _gas_update_instance_volume( inst, 0, 0 );
  }
}

void gas_stop_queued( int inst )
{
  // assume we receive the inst, not instance id
  if (gas.inst_list[inst].flag.used == 1)
  {
    switch (gas.inst_list[inst].flag.src_type)
    {
      case SRC_TYPE_SPU: 
        _gas_rpc_pause_spu_instance(inst, MODE_STOP);
        break;
      case SRC_TYPE_CD:  
        _gas_rpc_pause_cd_instance(inst, MODE_STOP);
        break;
      default:
        return;
    }
  }

}


//-----------------------------------------------------------------------------------
// G A S   R P C   P A U S E   I N S T A N C E
//

int gas_rpc_pause_instance( GasRpcArgs *args , int pause_mode )
{
  int instance_id = args->int_arg[0];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  int ret = 0;

  RECORDRA
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  WaitSema(gas.system_state_sema);
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Stop\n", gas.source_list[gas.inst_list[inst].source].filename, gas.inst_list[inst].instance_id);
#endif
  if (gas.inst_list[inst].flag.used == 1)
  {
    switch (gas.inst_list[inst].flag.src_type)
    {
      case SRC_TYPE_SPU: 
        ret = _gas_rpc_pause_spu_instance(inst, pause_mode);
        break;
      case SRC_TYPE_CD:  
        ret = _gas_rpc_pause_cd_instance(inst, pause_mode);
        break;
      default:
        SignalSema(gas.system_state_sema);
        return 0;
    }
  }
  SignalSema(gas.system_state_sema);
  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   D A M P E N   I N S T A N C E
//
// int_arg[0] = instance id;  int_arg[1] = new system dampen level
int gas_rpc_dampen_instance( GasRpcArgs *args, int dampen_mode)
{
  int instance_id = args->int_arg[0];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  int ret = 0;

  RECORDRA
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  WaitSema(gas.system_state_sema);

  if (gas.inst_list[inst].flag.used == 1)
  {
    switch (dampen_mode)
    {
      case MODE_GUARD: 
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Dampen guard\n", gas.source_list[gas.inst_list[inst].source].filename, gas.inst_list[inst].instance_id);
#endif
        gas.inst_list[inst].dampen_count = -1;
        break;

      case MODE_UNDAMPEN:
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Undampen\n", gas.source_list[gas.inst_list[inst].source].filename, gas.inst_list[inst].instance_id);
#endif
        gas.inst_list[inst].dampen_count = 0;
        break;

      case MODE_DAMPEN:
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Dampen\n", gas.source_list[gas.inst_list[inst].source].filename, gas.inst_list[inst].instance_id);
#endif
        gas.inst_list[inst].dampen_count++;
        break;

      default:
        gas_printf("Error.  Bad dampen parameter %d\n", dampen_mode);
        break;
    }
    gas.dampen_level = args->int_arg[1];
  }
  SignalSema(gas.system_state_sema);
  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   D A M P E N   A L L 
//
// int_arg[0] = new system dampen level
int  gas_rpc_dampen_all( GasRpcArgs *args, int dampen_mode)
{
  int ret = 0;
  int i;

  RECORDRA
  WaitSema(gas.system_state_sema);
 
  for (i=0; i<gas.inst_list_count; ++i)
  {
    if (gas.inst_list[i].flag.used == 1)
    {
      if (dampen_mode == MODE_UNDAMPEN)
			{
			//	dampened_flag = 0;
        gas.inst_list[i].dampen_count = 0;
			}
      else if (dampen_mode == MODE_DAMPEN)
			{
		//		dampened_flag = 1;
        gas.inst_list[i].dampen_count++;
			}
      else
        gas_printf("Error.  Bad dampen parameter %d\n", dampen_mode);
    }
  }
  gas.dampen_level = args->int_arg[0];
  SignalSema(gas.system_state_sema);

  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P A U S E   A L L 
//
// Also does a 'stop all' command
int  gas_rpc_pause_all( GasRpcArgs *args, int pause_mode)
{
  int ret = 0;
  int i;

  RECORDRA
  WaitSema(gas.system_state_sema);
 
  for (i=0; i<gas.inst_list_count; ++i)
  {
    if (gas.inst_list[i].flag.used == 1)
    {
      switch (gas.inst_list[i].flag.src_type)
      {
        case SRC_TYPE_SPU: 
          ret = _gas_rpc_pause_spu_instance(i, pause_mode);
          break;
        case SRC_TYPE_CD:  
          ret = _gas_rpc_pause_cd_instance(i, pause_mode);
          break;
        default:
          SignalSema(gas.system_state_sema);
          return 0;
      }
    }
  }
  if (pause_mode == MODE_STOP) 
  {
    int i;
    for (i=0; i < GAS_MAX_INSTANCES; i++) 
    {
      gas.inst_list[i].played = 0;
      gas_status.instances[i].is_playing = 0;
      gas_status.instances[i].is_ready = 0;
      gas_status.instances[i].is_valid = 0;
    }
  	sceSdSetSwitch( SD_CORE_0|SD_S_KOFF , 0xFFFFFF );
  	sceSdSetSwitch( SD_CORE_1|SD_S_KOFF , 0xFFFFFF );
  }
  SignalSema(gas.system_state_sema);

  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S E T   M A S T E R   V O L U M E
//
int gas_rpc_set_master_volume( GasRpcArgs *args )
{
  int i, vol = args->int_arg[0];
  for( i = 0; i < 2; i++ )
	{
		sceSdSetParam( i|SD_P_MVOLL , vol ) ;
		sceSdSetParam( i|SD_P_MVOLR , vol ) ;
	}
  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   I N S T A N C E   V O L U M E
//
int gas_rpc_instance_volume( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  RECORDRA

  WaitSema(gas.system_state_sema);
  RECORDRA
  if (gas.inst_list[inst].instance_id != instance_id)
  {
    SignalSema(gas.system_state_sema);
    return 0;
  }
  _gas_update_instance_volume( inst, args->int_arg[1], args->int_arg[2] );
  SignalSema(gas.system_state_sema);

  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   I N S T A N C E   P I T C H
//
int gas_rpc_instance_pitch( GasRpcArgs *args )
{                           
  int instance_id = args->int_arg[0];
  int pitch = args->int_arg[1];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  int source = gas.inst_list[inst].source;
  RECORDRA
  WaitSema(gas.system_state_sema);
  RECORDRA
  if (gas.inst_list[inst].instance_id != instance_id)
  {
    SignalSema(gas.system_state_sema);
    return 0;
  }
  
  gas.inst_list[inst].pitch = (pitch * gas.source_list[GET_SOURCE_BANK(source)][GET_SOURCE_SLOT(source)].pitch_one) / GAS_COMMAND_PITCH_ONE;
  gas_update_set_voice_pitch (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
    gas.inst_list[inst].pitch);

  SignalSema(gas.system_state_sema);
  return 1;
}

//-----------------------------------------------------------------------------------
// G A S   R P C   C O M M A N D   L I S T
//
// issues a new command list for differed processing (by the update loop)
void gas_rpc_command_list(void *data)
{
  GasCommandEntry *commands = (GasCommandEntry *)data;
  RECORDRA

  WaitSema(gas.cmd_list_sema);
  RECORDRA
  memcpy(gas.command_list, commands, sizeof(GasCommandEntry) * GAS_MAX_INSTANCES);
  gas.commands_waiting = 1;
  SignalSema(gas.cmd_list_sema);
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S T A T U S   I S   P L A Y I N G
//
int gas_rpc_status_is_playing( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  int ret = 0;
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  RECORDRA

  WaitSema(gas.system_state_sema);
  RECORDRA
  if (gas.inst_list[inst].instance_id == instance_id)
    ret = _gas_check_inst_playing(inst);
  SignalSema(gas.system_state_sema);

  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S T A T U S   I S   R E A D Y
//
int gas_rpc_status_is_ready( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  int ret = 0;
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  RECORDRA

  WaitSema(gas.system_state_sema);
  RECORDRA
  if (gas.inst_list[inst].instance_id == instance_id)
    ret = gas.inst_list[inst].flag.used && gas.inst_list[inst].flag.ready;
  SignalSema(gas.system_state_sema);
  
  return ret;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S E T   S T E R E O
//
int gas_rpc_set_stereo( GasRpcArgs *args )
{
  gas.stereo = args->int_arg[0];
  return gas.stereo;
}


//-----------------------------------------------------------------------------------
// G A S   P R I N T F
//
int gas_rpc_printf( GasRpcArgs *args )
{
  gas_printf(args->string_arg);
  return 1;
}

