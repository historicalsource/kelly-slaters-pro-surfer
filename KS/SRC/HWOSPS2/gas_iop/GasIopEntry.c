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
#include "../gas_iop.h"
#include "GasIopSystem.h"

ModuleInfo Module = { "gas_iop_driver", 0x0001 };

extern int gas_rpc_loop (void);

int start (void)
{
  struct ThreadParam param;
  int th;

  CpuEnableIntr();

  if (! sceSifCheckInit ())
    sceSifInit ();
  sceSifInitRpc (0);

  gas_printf ("GAS iop driver version 0.01\n");

  param.attr         = TH_C;
  param.entry        = gas_rpc_loop;
  param.initPriority = BASE_priority - 2;
  param.stackSize    = 0x800;
  param.option       = 0;
  th = CreateThread (&param);
  if (th > 0) 
  {
    StartThread (th, 0);
    gas_printf (" Exit GAS iop loader thread \n");
    return 0;
  }
  else
  {
    return 1;
  }
}

static char rpc_arg [GAS_COMMAND_LIST_SIZE];	// EE
GasIopStatus gas_status;  // EE

/*** dispatch ***/
static void*
dispatch (unsigned int command, void *data_, int size)
{ 
  int ret = 0;
  GasRpcArgs * args = ((GasRpcArgs *) data_);
/*
  if (command != GAS_IOP_COMMAND_LIST)
  {
    gas_printf("Dispatch [%04x] %s, 0x%x, 0x%x, 0x%x, 0x%x\n",
	    command, args->string_arg, args->int_arg[0], args->int_arg[1],
      args->int_arg[2], args->int_arg[3]);
  }
*/
  switch (command) 
  {
    case GAS_IOP_INIT:	        gas_init (args->int_arg[0], args->string_arg, 
                                          args->int_arg[1], args->int_arg[2]);  break;
    case GAS_IOP_SHUTDOWN:	    gas_shutdown ();                                break;
    case GAS_IOP_RESET:   	    ret = gas_reset (args->string_arg, args->int_arg[0]); break;

    case GAS_IOP_ADD_SOURCE:     ret = gas_add_source (args->string_arg);              break;
    case GAS_IOP_REMOVE_SOURCE:  ret = gas_remove_source (args->int_arg[0]);           break;
    case GAS_IOP_ADD_INSTANCE:   ret = gas_add_instance (args->int_arg[0]);            break;
    case GAS_IOP_COMMAND_LIST:   ret = gas_command_list (data_);                       break;
    case GAS_IOP_INSTANCE_PITCH: ret = gas_instance_pitch(args->int_arg[0], args->int_arg[1]); break;
//    case GAS_IOP_INSTANCE_VOLUME:ret = gas_instance_volume(args->int_arg[0], args->int_arg[1], args->int_arg[2]); break;

    case GAS_IOP_STATUS_IS_PLAYING: ret = gas_status_is_playing(args->int_arg[0]); break;
    case GAS_IOP_STATUS_IS_READY:   ret = gas_status_is_ready(args->int_arg[0]);   break;

    case GAS_IOP_PLAY_INSTANCE:    ret = gas_play_instance(args->int_arg[0], args->int_arg[1], args->int_arg[2]);      break;
    case GAS_IOP_PAUSE_INSTANCE:   ret = gas_pause_instance(args->int_arg[0], +1); break;
    case GAS_IOP_UNPAUSE_INSTANCE: ret = gas_pause_instance(args->int_arg[0], -1); break;
    case GAS_IOP_STOP_INSTANCE:    ret = gas_pause_instance(args->int_arg[0],  0); break;

    case GAS_IOP_VOICE_STATUS:      gas_voice_status_dump (); break;
    case GAS_IOP_SPU_MEM_DUMP:  gas_spu_mem_dump(); break;
    default:
	    gas_printf ("GAS IOP driver error: unknown command %d \n", command);
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
  sceSifRegisterRpc (&sd, GAS_IOP_DEV, dispatch, (void*)rpc_arg, NULL, NULL, &qd);
  
  sceSifRpcLoop (&qd);

  return 0;
}
