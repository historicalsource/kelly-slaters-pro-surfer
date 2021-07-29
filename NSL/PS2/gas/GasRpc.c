// GAS system-control functions
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas.h" 
#include "GasSystem.h"


// ================================================================
// Global data

// class members
extern ModuleInfo Module;
GasSystemState gas;
sceCdRMode g_sce_cdrmode;
int g_update_sema __attribute__((aligned(16))) = -1;
int g_load_ee_sema __attribute__((aligned(16))) = -1;

int g_update_thread_id = 0;

int g_max_cd_stereo_streams = MAX_CD_STEREO_STREAMS;

#define UPDATE_STREAMS_INTERVAL_USEC (10 * 1000) // 10 milliseconds (every 1/100 of a second)
struct SysClock g_clock;

sceSdEffectAttr r_attr;
    
// internal functions
void  gas_rpc_system_init();
void _gas_rpc_system_reset();
void _gas_rpc_reset_internals();
void _gas_rpc_remove_sources(void);
void _gas_rpc_init_system_data();
void  gas_rpc_system_shutdown();

unsigned int gas_timer_interupt( void *common);


//-----------------------------------------------------------------------------------
// G A S   R P C   G E T   V E R S I O N
int gas_rpc_get_version()
{
  return Module.version;
}



//-----------------------------------------------------------------------------------
// G A S   I N I T


/*** gas_rpc_init ***/
// initializes the gas, to undo this, call gas_shutdown.  Both functions should only
// be called once per run of the application, and gas_reset calls should be used
// to do a 'soft' reset of the sound system (because a reset is less costly than
// a shutdown & re-init, that's why reset even exists, yo).
void gas_rpc_init( GasRpcArgs *args )
{
  int size;
  int proview_mode = args->int_arg[1];
  unsigned long memthing;

  // if we are using SN Systems' ProView, use snfile instead of host0 for host file accesses
  gas.host_prefix = "host0:";

  if (proview_mode == 1)
  {
    gas_printf( "Using proview\n" );
    gas.proview_mode = 1;
    g_max_cd_stereo_streams = PROVIEW_MAX_CD_STEREO_STREAMS;
  }
  else
  {
    gas_printf( "Not using proview.\n" );
    gas.proview_mode = 0;
    g_max_cd_stereo_streams = MAX_CD_STEREO_STREAMS;
  }

  // initialize the sound system
  sceSdInit(SD_INIT_COLD);
  gas.stereo = 1;
  // set up the CD reading mode
  g_sce_cdrmode.trycount = 0;
  g_sce_cdrmode.spindlctrl = SCECdSpinNom;
  g_sce_cdrmode.datapattern = SCECdSecS2048;
  gas.dvd_mode = args->int_arg[2];

  if (gas.dvd_mode)
  {
    //    Disk media: DVD
    // Output format: PCM
    //    Copy guard: normal (one generation recordable / default)
    sceSdSetCoreAttr (SD_C_SPDIF_MODE, (SD_SPDIF_MEDIA_DVD |
				  SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL));
  }
  else
  {
    //    Disk media: CD
    // Output format: PCM
    //    Copy guard: normal (one generation recordable / default)
    sceSdSetCoreAttr (SD_C_SPDIF_MODE, (SD_SPDIF_MEDIA_CD |
				  SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL));
  }

  // initialize GasSystemState constants (done only once, reset does the repeatable stuff)
  
  memset(gas.source_list, 0, sizeof(GasSource *)* MAX_NUM_HEAPS);

  //memset(gas.spu_heaps, '0', sizeof(spu_heap_info) * MAX_NUM_HEAPS);

  memthing = QueryMemSize();
  gas_printf( "IOP MEMORY   total memory size %12lu\n", memthing );
  memthing = QueryTotalFreeMemSize();
  gas_printf( "IOP MEMORY   total memory free %12lu\n", memthing );
  memthing = QueryMaxFreeMemSize();
  gas_printf( "IOP MEMORY   max allocatable   %12lu\n", memthing );
  

  // NOTE - if you change this value, update them in the ADPCM Tool, so that it can
  // accurately predict the limits of SPU ram
  gas.spu_buffer_num = (g_max_cd_stereo_streams * 2) + MAX_CD_MONO_STREAMS;

  size = (g_max_cd_stereo_streams * CD_STEREO_BUFFER_SIZE) + (MAX_CD_MONO_STREAMS * CD_MONO_BUFFER_SIZE); /* + 123104;*/
  gas.iop_buffers_top = (unsigned char *)AllocSysMemory(0, size, NULL);
  if( gas.iop_buffers_top == NULL )
	{
		gas_printf( "**********************************************\n");
    gas_printf( "Failed to allocate %d bytes for iop buffers\n", size );
		gas_printf( "**********************************************\n");
	}
  else
    gas_printf( "Allocated %u bytes for streaming buffers\n", size );


  gas.iop_cd_mono_buffers_top = gas.iop_buffers_top + (g_max_cd_stereo_streams * CD_STEREO_BUFFER_SIZE);

#ifdef DEBUG
  memset(gas.iop_buffers_top, 0, size);
#endif
 
  gas.host_streaming_fd = -1;
  gas.stream_from_host = 0; 


  
  gas_debug_init_profiler();
 

  gas_printf("BEFORE_INIT\n");

  gas_rpc_system_init();
  
	gas_rpc_reset();


  // start the update thread (stopped in shutdown)
  USec2SysClock(UPDATE_STREAMS_INTERVAL_USEC, &g_clock);
  SetAlarm(&g_clock, gas_timer_interupt, NULL);
	

}

unsigned int gas_timer_interupt( void *common)
{
  iSignalSema(g_update_sema);	// Signal our update_thread to be called
  return g_clock.low;
}

int gas_rpc_create_update_thread()
{
  struct ThreadParam param;
  int	thid;

  param.attr         = TH_C;
  param.entry        = gas_update_thread;
  param.initPriority = GAS_IOP_PRIORITY-2;
  param.stackSize    = 0x1000;
  param.option = 0;
  thid = CreateThread(&param);
  return thid;
}


/*** gas_rpc_system_init ***/
// portions of the init fn that are specific to the GasSystemState structure
void gas_rpc_system_init()
{

  // semaphore to control access to this structure
  gas.system_state_sema = gas_rpc_create_sema(0);
  gas.instr_list_sema   = gas_rpc_create_sema(0);
  gas.cmd_list_sema     = gas_rpc_create_sema(0);
  // Initialize the various queues (Allocates memory, which is freed in shutdown)
  // fifo_queue_init( &gas.free_voices, PS2_MAX_VOICES );
  //fifo_queue_init( &gas.free_instances, GAS_MAX_INSTANCES );
  fifo_queue_init( &gas.free_cd_stereo_bufs, g_max_cd_stereo_streams );
  fifo_queue_init( &gas.free_cd_mono_bufs, MAX_CD_MONO_STREAMS );
  fifo_queue_init( &gas.free_spu_bufs, gas.spu_buffer_num );
  fifo_queue_init( &gas.reload_spu, gas.spu_buffer_num );
  fifo_queue_init( &gas.reload_cd_streams, MAX_CD_STREAMS );
  fifo_queue_init( &gas.cd_preloads_pending, MAX_CD_STREAMS );
  fifo_instr_init( &gas.incomingGasInstructions, MAX_QUEUED_INSTRUCTIONS );
  g_update_sema = gas_rpc_create_sema(0);
  g_load_ee_sema = gas_rpc_create_sema(0);
  
  // This needs to go here
  gas.preloading_instance = -1;

  if( gas.host_streaming_fd >= 0 )
  {
    close( gas.host_streaming_fd );
    gas.host_streaming_fd = -1;
  }
  g_update_thread_id = gas_rpc_create_update_thread();
  StartThread( g_update_thread_id, (u_long)NULL );
  // debugging   memthing = QueryMemSize();

#ifdef DEBUG
  _gas_debug_spu_mem_clear();
#endif

	gas_printf("END_SYSINIT\n");
}



//-----------------------------------------------------------------------------------
// G A S   R E S E T

/*** gas_rpc_reset ***/
// a soft reset of the system, no partial shutdown call is required prior to calling
// this function, because reset calls that on its own.
// Performs a soft reset on the gas and should be done every level.
void gas_rpc_reset()
{
	// Stop all the sounds.
	gas_rpc_pause_all(NULL, MODE_STOP);


	// Core one is to be used for sound which require reverb.

	// Turnoff all sounds.
	sceSdSetParam(SD_CORE_0 | SD_P_MVOLL, 0x3fff);
	sceSdSetParam(SD_CORE_0 | SD_P_MVOLR, 0x3fff);

	// Effect workarea end address.
	sceSdSetAddr(SD_CORE_0 | SD_A_ESA, SPU_MEMORY_MAX - 0x20000);
	sceSdSetAddr(SD_CORE_0 | SD_A_EEA, SPU_MEMORY_MAX - 1);

	// Set reverb attribute.
	r_attr.depth_L = 0;
	r_attr.depth_R = 0;
	r_attr.mode = SD_REV_MODE_HALL | SD_REV_MODE_CLEAR_WA;
	sceSdSetEffectAttr(SD_CORE_0, &r_attr);

	// Enable reverb.
	sceSdSetCoreAttr(SD_CORE_0 | SD_C_EFFECT_ENABLE, 1);
	sceSdSetParam(SD_CORE_0 | SD_P_EVOLL, 0x3fff);
	sceSdSetParam(SD_CORE_0 | SD_P_EVOLR, 0x3fff);


	// Core two is to be used for sound which should not have reverb.

	// Turnoff all sounds.
	sceSdSetParam(SD_CORE_1 | SD_P_MVOLL, 0x3fff);
	sceSdSetParam(SD_CORE_1 | SD_P_MVOLR, 0x3fff);

	// Disable reverb.
	sceSdSetCoreAttr(SD_CORE_1 | SD_C_EFFECT_ENABLE, 0);
	sceSdSetParam(SD_CORE_1 | SD_P_EVOLL, 0x0);
	sceSdSetParam(SD_CORE_1 | SD_P_EVOLR, 0x0);


	WaitSema(gas.system_state_sema);
	RECORDRA

	_gas_rpc_system_reset();
	SignalSema(gas.system_state_sema);
}


/*** _gas_rpc_system_reset ***/
// portions of the gas reset specific to the GasSystemState structure
void _gas_rpc_system_reset()
{
  // Assumed semaphore locks: system_state_sema
  int i;

//_gas_debug_voice_status_dump();

  // clean up old stuff
  _gas_rpc_reset_internals();
  _gas_rpc_init_system_data();


  // clear out the queues
  //fifo_queue_clear(&gas.free_instances);
  fifo_queue_clear(&gas.reload_spu);
  fifo_queue_clear(&gas.reload_cd_streams);
  fifo_queue_clear(&gas.cd_preloads_pending);
  fifo_instr_clear(&gas.incomingGasInstructions);

  for (i=0; i<PS2_MAX_VOICES; ++i)
    gas.voice_used[i] = -1;

  // These queues are expected to be loaded with stuff, so pre-load them
  // fifo_queue_clear(&gas.free_voices);
  for (i=0; i<GAS_MAX_INSTANCES; ++i)
    //fifo_queue_push(&gas.free_voices, i);
    gas.free_voices[i].in_use = 0;

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

}

/*** _gas_rpc_partial_init ***/
// called by GAS_RPC_PARTIAL_INIT, to re-initialize the sound system after a partial_shutdown
// rpc call.
void _gas_rpc_partial_init()
{ 
  int i;

  sceSdInit(SD_INIT_COLD);
  for( i = 0; i < 2; i++ )
	{
		sceSdSetParam( i|SD_P_MVOLL , 0x3fff ) ;
		sceSdSetParam( i|SD_P_MVOLR , 0x3fff ) ;
	}
  if (gas.dvd_mode)
  {
    //    Disk media: DVD
    // Output format: PCM
    //    Copy guard: normal (one generation recordable / default)
    sceSdSetCoreAttr (SD_C_SPDIF_MODE, (SD_SPDIF_MEDIA_DVD |
				  SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL));
  }
  else
  {
    //    Disk media: CD
    // Output format: PCM
    //    Copy guard: normal (one generation recordable / default)
    sceSdSetCoreAttr (SD_C_SPDIF_MODE, (SD_SPDIF_MEDIA_CD |
				  SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL));
  }
  sceCdDiskReady(0);
}


/*** _gas_rpc_reset_internals ***/
// called by reset and shutdown to free up any resources that are level specific.  Flushes
// iop and spu sounds and buffers, and basically shuts down all that needs to be shutdown
// for a reset to happen correctly.
void _gas_rpc_reset_internals()
{
  int i;
  // Assumed semaphore locks: system_state_sema
  for (i=0; i < gas.num_heaps; i++)
  {
    if (gas.source_list[i] != NULL)
      FreeSysMemory(gas.source_list[i]);
    gas.source_list[i]=NULL;
    
  }

  
  _gas_rpc_remove_sources();
  gas.num_heaps = 0;
}


/*** gas_rpc_remove_sources ***/
// free anything created by the sources
void _gas_rpc_remove_sources(void)
{
  // Assumed semaphore locks: system_state_sema
/* -- no resources are being allocated per source.  If this ever changes, clear them out here --

  int i;
  for (i=0; i<gas.source_list_count; ++i)
  {
    switch (gas.source_list[i].flag.src_type)
    {
      case SRC_TYPE_SPU: // you don't remove spu sources this way, we clear them in bulk
        break;
      case SRC_TYPE_CD:  break;
      default:           break;
    }
  }
*/
}


//*** _gas_init_system_data ***/
// initializes data in the GasSystemState structure
void _gas_rpc_init_system_data()
{
  // Assumed semaphore locks: system_state_sema
  int i;

  gas.inst_list_count = 0;
  for (i=0; i<GAS_MAX_INSTANCES; ++i)
    gas.inst_list[i].instance_id = i;

  gas.preloading_instance = -1;
  gas.dampen_level = 80; // default dampening level
  // reserve stream buffers in the spu

  // NOTE - if you change these formulae, update them in the ADPCM Tool, so that it can
  // accurately predict the limits of SPU ram
  gas.spu_heap_start = SPU_MEMORY_TOP + (SPU_BUFFER_SIZE * gas.spu_buffer_num);
  gas.spu_heap_end = SPU_MEMORY_MAX - (2 * SPU_FX_WORK_AREA_SIZE);
//  gas.spu_heap_end = SPU_MEMORY_MAX - (SPU_FX_WORK_AREA_SIZE);
  gas.spu_heap_curr = gas.spu_heap_start;
}



//-----------------------------------------------------------------------------------
// G A S   S H U T D O W N

/*** gas_rpc_shutdown ***/
// undoes what init and reset does.  should only be called when shutting down the app,
// use reset to do a soft reset, that's what it's for.
void gas_rpc_shutdown (void)
{
	unsigned long memthing;
  WaitSema( gas.system_state_sema );
  RECORDRA
  TerminateThread(g_update_thread_id);
	DeleteThread(g_update_thread_id);
  SignalSema(g_update_sema);
  CancelAlarm(gas_timer_interupt, NULL);
    
  _gas_rpc_reset_internals();

  gas_rpc_system_shutdown();
  
	gas_debug_shutdown_profiler();

  if (gas.iop_buffers_top != NULL)
    FreeSysMemory(gas.iop_buffers_top);

  // shut down the sound and cd systems
  sceSdBlockTrans( 0, SD_TRANS_MODE_STOP, NULL, 0 );
  sceSdSetCoreAttr (SD_CORE_0 | SD_C_IRQ_ENABLE, 0);
	sceSdSetSwitch( SD_CORE_0|SD_S_KOFF , 0xFFFFFF );
	sceSdSetSwitch( SD_CORE_1|SD_S_KOFF , 0xFFFFFF );
  sceCdStStop();

  return;
}


/*** gas_rpc_system_shutdown ***/
void gas_rpc_system_shutdown()
{

  gas_rpc_delete_sema( gas.system_state_sema );
  gas.system_state_sema = -1;
  
	gas_rpc_delete_sema( g_update_sema );
  g_update_sema = -1;
  
	gas_rpc_delete_sema( gas.instr_list_sema );
  gas.instr_list_sema = -1;
  
	gas_rpc_delete_sema( gas.cmd_list_sema );
  gas.cmd_list_sema = -1;

  gas_rpc_delete_sema( g_load_ee_sema );
	g_load_ee_sema = -1;
  

  // free the memory allocated for each queue
  //fifo_queue_free(&gas.free_voices);
  fifo_queue_free(&gas.free_cd_mono_bufs);
  fifo_queue_free(&gas.free_cd_stereo_bufs);
  fifo_queue_free(&gas.free_spu_bufs);
  fifo_queue_free(&gas.reload_spu);
//  fifo_queue_free(&gas.free_instances);
  fifo_queue_free(&gas.reload_cd_streams);
  fifo_queue_free(&gas.cd_preloads_pending);
  fifo_instr_free(&gas.incomingGasInstructions);
	
	if( gas.host_streaming_fd >= 0 )
  {
    close( gas.host_streaming_fd );
    gas.host_streaming_fd = -1;
  }
}


int gas_rpc_load_snd_list( GasRpcArgs *args )
{
  char *list_file = args->string_arg;
  int list_file_size = args->int_arg[0];
  int i;

  WaitSema(gas.system_state_sema);
  gas.num_heaps++;
  if (_gas_rpc_load_list_file(list_file, list_file_size, &gas.source_list[gas.num_heaps-1], &gas.spu_heaps[gas.num_heaps-1].source_list_count) == 0)
  {
    gas.num_heaps--;
    SignalSema(gas.system_state_sema);
    return 0;
  }

  // flag all spu sources as loaded
  for( i = 0; i < gas.spu_heaps[gas.num_heaps-1].source_list_count; i++ )
  {
    if( gas.source_list[gas.num_heaps-1][i].flag.src_type == SRC_TYPE_SPU )
      gas.source_list[gas.num_heaps-1][i].flag.loaded = 1;
  }

  SignalSema(gas.system_state_sema);
  return gas.spu_heaps[gas.num_heaps-1].source_list_count;
}

int gas_rpc_finalise_srcs()
{
  int retval = 1;
  WaitSema(gas.system_state_sema);
  if (!_gas_rpc_load_all_spu_sources())
    retval = 0;

  SignalSema(gas.system_state_sema);
  SignalSema(g_load_ee_sema);
  gas_printf("Leaving finalise sources\n");
  return retval;
}


unsigned int gas_rpc_push_bank( GasRpcArgs *args )
{
 
  if (!gas_rpc_load_snd_list( args ))
    return 0;


  if(!gas_rpc_finalise_srcs())
  {
    gas.num_heaps--;
    FreeSysMemory(gas.source_list[gas.num_heaps-1]);
    return 0;
  }

  return 1;
}
unsigned int gas_rpc_get_source_bank( GasRpcArgs *args  )
{
  RECORDRA
  // search the sources for one called 'filename'
  return GET_SOURCE_BANK(args->int_arg[0]);
}
unsigned int gas_rpc_pop_bank( void )
{
  int i;
  if (gas.num_heaps == 0)
    return 0;

  for (i=0; i < GAS_MAX_INSTANCES; i++)
  {
    if(GET_SOURCE_BANK(gas.inst_list[i].source) == gas.num_heaps -1) 
    {
      gas_stop_queued(i);
    }
  }

  memset(gas.source_list[gas.num_heaps - 1], 0, sizeof(GasSource *)*gas.spu_heaps[gas.num_heaps -1].source_list_count);
  memset(&gas.spu_heaps[gas.num_heaps - 1], 0, sizeof(spu_heap_info));
  FreeSysMemory(gas.source_list[gas.num_heaps-1]);

  gas.num_heaps--;
  return 1;
}
#define MAX_RETURN_ADDRESSES 20

int RA[MAX_RETURN_ADDRESSES]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int NextRA=0;

/*register*/ int _retaddr;
void Debug_Record()
{
  // WTB - modified so NextRA points to the last one we entered, 
  // this way it's easy to look at RA[NextRA] in the
  // debugger and see the most recent one.
  ++NextRA;
  if (NextRA>=MAX_RETURN_ADDRESSES) NextRA=0;
  #if defined (PSX) || defined (PS2)
  __asm__ volatile ("move %0,$31" : "=r"(_retaddr) : : "%0");
  #elif defined(WIN32)
  __asm mov _retaddr,esp
  #elif defined(DREAMCAST)
  __asm__ volatile ("sts pr,%0" : "=r"(_retaddr) : : "%0");
  #else
  #error unsupported processor
  #endif
  RA[NextRA]=_retaddr;
}
