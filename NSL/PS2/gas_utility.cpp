#include <eekernel.h>
#include <stdio.h>
#include <sif.h>
#include <sifrpc.h>
#include "string.h"
#include "gas.h"
#include "../common/nsl.h"
#include "nsl_ps2.h"


#define DATA_SIZE_COMMAND GAS_COMMAND_LIST_SIZE
#define DATA_SIZE_NORMAL  sizeof(GasRpcArgs)

GasStatus g_RPC_status;

static unsigned char sbuf [GAS_COMMAND_LIST_SIZE] __attribute__((aligned (64)));
static unsigned char rbuf [GAS_COMMAND_LIST_SIZE] __attribute__((aligned (64)));
static sceSifClientData cd;

int nslPs2GasRpcInit (void)
{
  sceSifInitRpc (0);
  while (1) 
  {
    if (sceSifBindRpc (&cd, GAS_DEV, 0) < 0) 
    { 
	    //nglPrintf("error: sceSifBindRpc \n");
	    return (-1);
    }
    if (cd.serve != 0) 
      break;
  }
  return 0;
}


/*** gas_rpc ***/
int nslPs2GasRpc (int command, const char *strarg, int intarg0, int intarg1, int intarg2, int intarg3 )
{
  GasRpcArgs args;
  int arg_size = 0;
  int mode;
  int ret;
static int last_command = 0;

  if (command != GAS_RPC_COMMAND_LIST)
  {
    if (strarg != NULL)
    {
      strncpy(args.string_arg, strarg, 48);
      args.string_arg[47] = '\0'; // cap it, just in case
    }
    else
      args.string_arg[0] = '\0';
    args.int_arg[0] = intarg0;
    args.int_arg[1] = intarg1;
    args.int_arg[2] = intarg2;
    args.int_arg[3] = intarg3;
    arg_size = DATA_SIZE_NORMAL;
    memcpy(sbuf, (unsigned int *)(&args), sizeof(args));

    // blocking call to the iop
    mode = 0;
    ret = sceSifCallRpc (&cd, command, mode, (void *) sbuf, arg_size,
      (void *) rbuf, sizeof(GasStatus), NULL, NULL);
  }
  else
  {
    // send up a command list (blocking)
    arg_size = DATA_SIZE_COMMAND;
    memcpy(sbuf, (unsigned int *)strarg, arg_size);

    mode = 0;
    ret = sceSifCallRpc (&cd, command, mode, (void *) sbuf, arg_size,
      (void *) rbuf, sizeof(GasStatus), NULL, NULL);
  }

  last_command = command;

  if (mode == SIF_RPCM_NOWAIT)
  {
    return ret;
  }
  else
  {
    memcpy(&g_RPC_status, rbuf, sizeof(GasStatus));
    last_command = 0;

    // Run the update
    for (int i=0; i < NSL_NUM_SOUNDS; i++) 
    {
      if (nsl.soundSlots[i].gasInstanceId != (nlUint32)GAS_INVALID_ID) 
      {
        if (g_RPC_status.instances[GAS_INSTANCE_ID_MASK&nsl.soundSlots[i].gasInstanceId].is_valid) 
        {
          nsl.soundSlots[i].isReallyPlaying = g_RPC_status.instances[GAS_INSTANCE_ID_MASK&nsl.soundSlots[i].gasInstanceId].is_playing;
          nsl.soundSlots[i].used = true;
          nsl.soundSlots[i].isReallyReady = g_RPC_status.instances[GAS_INSTANCE_ID_MASK&nsl.soundSlots[i].gasInstanceId].is_ready;
          if (nsl.soundSlots[i].isPlaying)
            nsl.soundSlots[i].isReady = true;
          else 
            nsl.soundSlots[i].isReady = nsl.soundSlots[i].isReallyReady;
        } 
        else 
        {
          // Fix up emitters
          if (nsl.soundSlots[i].myEmitter != NSL_GLOBAL_EMITTER_ID)
            nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(nsl.soundSlots[i].myEmitter)].emittedSounds.find(nsl.soundSlots[i].myId, true);
          _nslClearSoundSlot(i, false);
          nsl.soundSlotsUsedCount--;
        }
      } 

    }

    //printf("retval1 = %d  retval2 = %d\n", g_RPC_status.rpc_retval1, g_RPC_status.rpc_retval2);
    return g_RPC_status.rpc_retval1;
  }
}
