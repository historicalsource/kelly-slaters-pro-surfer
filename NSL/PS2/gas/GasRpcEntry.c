/*=======================================================================================*
 * Ghetto Audio System (GAS) for the Playstation2
 *---------------------------------------------------------------------------------------*
 * Author - Greg Taylor (2/5/01)
 *---------------------------------------------------------------------------------------*
 * An audio library for the PS2 which is primarily a stand-alone IOP program.  This 
 * program (like many of the PS2 IRX libs from Sony) is loaded onto the IOP and responds
 * to RPC calls from the main program (running on the EE).
 *---------------------------------------------------------------------------------------*
 * GAS module entry point
 *=======================================================================================*/

#include <kernel.h>
#include <stdio.h>
#include <sif.h>
#include <sifrpc.h>
#include "../gas.h"
#include "GasSystem.h"

#define GAS_VERSION_MAJOR 0x02
#define GAS_VERSION_MINOR 0x01

ModuleInfo Module = { "gas_driver", ((GAS_VERSION_MAJOR<<8)|GAS_VERSION_MINOR) };

extern int gas_rpc_loop (void);

int start (void)
{
  struct ThreadParam param;
  int th;

  CpuEnableIntr();

  if (! sceSifCheckInit ())
    sceSifInit ();
  sceSifInitRpc (0);


  gas_printf ("GAS driver version %02u.%u\n", (Module.version&0xff00)>>8, Module.version&0x00ff);

  param.attr         = TH_C;
  param.entry        = gas_rpc_loop;
  param.initPriority = GAS_IOP_PRIORITY;
  param.stackSize    = 0x800;
  param.option       = 0;
  th = CreateThread (&param);
  if (th > 0) 
  {
    StartThread (th, 0);
    gas_printf (" Exit GAS loader thread \n");
    return 0;
  }
  else
  {
    return 1;
  }
}

static char rpc_arg [GAS_COMMAND_LIST_SIZE];	// EE
GasStatus gas_status;  // EE

/*** dispatch ***/
static void*
dispatch (unsigned int command, void *data_, int size)
{ 
  int ret = 0;
  GasRpcArgs * args = ((GasRpcArgs *) data_);
/*
  if (command != GAS_RPC_COMMAND_LIST)
  {
    gas_printf("Dispatch [%04x] %s, 0x%x, 0x%x, 0x%x, 0x%x\n",
	    command, args->string_arg, args->int_arg[0], args->int_arg[1],
      args->int_arg[2], args->int_arg[3]);
  }
*/

  switch (command) 
  {
    case GAS_RPC_GET_STREAMING_BUFFER: ret = (int)gas_rpc_get_streaming_buffer(); break;
    case GAS_RPC_GET_VERSION:    ret = gas_rpc_get_version(); break;
    case GAS_RPC_INIT:	         gas_rpc_init( args );              break;
    case GAS_RPC_SHUTDOWN:	     gas_rpc_shutdown();                break;
    case GAS_RPC_RESET:   	     gas_rpc_reset( args );             break;

    case GAS_RPC_ADD_SOURCE:     ret = gas_rpc_add_source( args );      break;
    case GAS_RPC_GET_REVERB:     ret = gas_rpc_get_reverb( args );      break;
    case GAS_RPC_SET_REVERB:     ret = gas_rpc_set_reverb( args );      break;
    case GAS_RPC_REMOVE_SOURCE: /* ret = gas_rpc_remove_source( args ); */ break;
    case GAS_RPC_ADD_INSTANCE:   ret = gas_rpc_add_instance( args );    break;
    case GAS_RPC_COMMAND_LIST:   gas_rpc_command_list( data_ );         break;
    case GAS_RPC_INSTANCE_PITCH: ret = gas_rpc_instance_pitch( args );  break;
    case GAS_RPC_INSTANCE_VOLUME:ret = gas_rpc_instance_volume( args ); break;

    case GAS_RPC_STATUS_IS_PLAYING: ret = gas_rpc_status_is_playing( args ); break;
    case GAS_RPC_STATUS_IS_READY:   ret = gas_rpc_status_is_ready( args );   break;

    case GAS_RPC_PLAY_INSTANCE:    ret = gas_rpc_play_instance( args );      break;

    // dampen functions
    case GAS_RPC_DAMPEN_GUARD:     ret = gas_rpc_dampen_instance( args, MODE_GUARD); break;
    case GAS_RPC_DAMPEN_INSTANCE:  ret = gas_rpc_dampen_instance( args, MODE_DAMPEN); break;
    case GAS_RPC_UNDAMPEN_INSTANCE:ret = gas_rpc_dampen_instance( args, MODE_UNDAMPEN); break;

    case GAS_RPC_DAMPEN_ALL:       ret = gas_rpc_dampen_all( args, MODE_DAMPEN); break;
    case GAS_RPC_UNDAMPEN_ALL:     ret = gas_rpc_dampen_all( args, MODE_UNDAMPEN); break;

    // pause/stop functions
    case GAS_RPC_STOP_INSTANCE:    ret = gas_rpc_queue_stop( args ); break;
    case GAS_RPC_STOP_ALL:         ret = gas_rpc_pause_all( args, MODE_STOP ); break;

    case GAS_RPC_PAUSE_GUARD:      ret = gas_rpc_pause_instance( args, MODE_GUARD ); break;
    case GAS_RPC_PAUSE_INSTANCE:   ret = gas_rpc_pause_instance( args, MODE_PAUSE ); break;
    case GAS_RPC_UNPAUSE_INSTANCE: ret = gas_rpc_pause_instance( args, MODE_UNPAUSE ); break;

    case GAS_RPC_PAUSE_ALL:        ret = gas_rpc_pause_all( args, MODE_PAUSE ); break;
    case GAS_RPC_UNPAUSE_ALL:      ret = gas_rpc_pause_all( args, MODE_UNPAUSE ); break;

    case GAS_RPC_SET_MASTER_VOLUME: ret = gas_rpc_set_master_volume( args ); break;

    case GAS_RPC_LOAD_TO_BUFFER:   ret = (int)gas_rpc_load_to_buffer( args ); break;
    case GAS_RPC_TRANS_TO_EE:      ret = gas_rpc_trans_to_ee( args ); break;
    case GAS_RPC_LOAD_SND_LIST:    ret = gas_rpc_load_snd_list( args ); break;
    case GAS_RPC_FINALISE_SRCS:    ret = gas_rpc_finalise_srcs( args ); break;
    case GAS_RPC_QUERY_MEM:        ret = gas_rpc_query_mem(); break;
#ifdef DEBUG
    case GAS_RPC_VOICE_STATUS:     _gas_debug_voice_status_dump(); break;
    case GAS_RPC_SPU_MEM_DUMP:     _gas_debug_spu_mem_dump(); break;
#endif
    case GAS_RPC_GET_SOURCE_TYPE:  ret = gas_rpc_source_type( args ); break;
    case GAS_RPC_GET_SOURCE_LOOPING:ret = gas_rpc_source_looping( args ); break;
    case GAS_RPC_SET_STEREO:       ret = gas_rpc_set_stereo( args ); break;
    case GAS_RPC_GET_SOURCE_LENGTH:ret = gas_rpc_source_length( args ); break;
    case GAS_RPC_GET_SOURCE_PADDED_LENGTH:
                                   ret = gas_rpc_source_padded_length( args ); break;
    case GAS_RPC_GET_SOURCE_FREQ:  ret = gas_rpc_source_freq( args ); break;
    case GAS_RPC_SET_HOST_STREAM:  gas_rpc_set_host_stream( args );  break;
    case GAS_RPC_PARTIAL_SHUTDOWN: gas_rpc_partial_shutdown( args ); break;
    case GAS_RPC_PARTIAL_INIT:     gas_rpc_partial_init( args ); break;
    case GAS_RPC_SET_ROOT_DIR:     gas_rpc_set_root_dir( args ); break;
    case GAS_RPC_GET_SRC_VOLUME:   ret = gas_rpc_get_source_volume( args ); break;
    case GAS_RPC_GET_SOUND_POSITION: ret = gas_rpc_get_sound_position( args ); break;
    case GAS_RPC_ADD_INSTANCE_WITH_OFFSET: ret = gas_rpc_add_instance_with_offset( args ); break;
    case GAS_RPC_PUSH_BANK:        ret = gas_rpc_push_bank( args ); break;
    case GAS_RPC_POP_BANK:         ret = gas_rpc_pop_bank( ); break;
    case GAS_RPC_GET_SOURCE_BANK:  ret = gas_rpc_get_source_bank( args ); break;
    case GAS_RPC_OPEN_RAWSTREAM:   ret = gas_rpc_open_rawstream( args ); break;
    case GAS_RPC_PRINTF:           ret = gas_rpc_printf( args ); break;
   default:
	    gas_printf ("GAS driver error: unknown command %d \n", command);
	    break;
  }

  // set up the return structure
  gas_status.rpc_retval1 = ret;

//  gas_printf("! return value = %x \n", ret); 
  return (void*)(&gas_status);
}

/*** rpc_gas_loop ***/
int
gas_rpc_loop (void)
{
  sceSifQueueData qd;
  sceSifServeData sd;

  CpuEnableIntr ();
  EnableIntr (INUM_DMA_4);
  EnableIntr (INUM_DMA_7);
  EnableIntr (INUM_SPU);
  
  sceSifInitRpc (0);
  sceSifSetRpcQueue (&qd, GetThreadId ());
  sceSifRegisterRpc (&sd, GAS_DEV, dispatch, (void*)rpc_arg, NULL, NULL, &qd);
  
  sceSifRpcLoop (&qd);

  return 0;
}
