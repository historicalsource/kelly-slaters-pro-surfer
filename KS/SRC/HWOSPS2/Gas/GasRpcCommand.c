// GAS command functions
#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas.h" 
#include "GasSystem.h"


//-----------------------------------------------------------------------------------
// G A S   R P C   A D D   S O U R C E 
//
// returns the source id of the newly added source, this is used to create instances 
// returns -1 if the source name cannot be found or if the source cannot be created.
int gas_rpc_add_source( GasRpcArgs *args )
{
  int i;
  char *filename = args->string_arg;

  RECORDRA;
  // search the sources for one called 'filename'
  for (i=0; i<gas.source_list_count; ++i)
  {
    if (strcmp(filename, gas.source_list[i].short_filename) == 0)
      break;
  }
  if (i == gas.source_list_count)
    return -1;

  WaitSema(gas.system_state_sema);
  RECORDRA;

  // don't allow multiple loads of the same source
  if (gas.source_list[i].flag.loaded == 1)
  {
    SignalSema(gas.system_state_sema);
    return i;
  }

  // check if there is room for this source
  if (gas.spu_heap_curr + gas.source_list[i].size >= gas.spu_heap_end)
  {
    SignalSema(gas.system_state_sema);
    return -1;
  }
  
  // add the source differently depending on its src_type
  switch (gas.source_list[i].flag.src_type)
  {
    case SRC_TYPE_SPU: i = _gas_rpc_add_spu_source(i); break;
    case SRC_TYPE_CD:  i = _gas_rpc_add_cd_source(i);  break;
  }

  SignalSema(gas.system_state_sema);

  // return the index of the GasSource[] that this entry uses
  return i;
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
  int i, ret_val;
#ifdef PROFILER_OUTPUT
  u_int wait_time;
  u_int final_time;
#endif

  RECORDRA;
  if (source >= gas.source_list_count || source < 0 || gas.source_list[source].flag.loaded == 0)
  {
    return -1;
  }
if (strcmp(gas.source_list[source].short_filename, "SILENT") == 0)
  return -1;

#ifdef PROFILER_OUTPUT
  wait_time = gas_debug_profile_start();
#endif
  WaitSema(gas.system_state_sema);
#ifdef PROFILER_OUTPUT
  final_time = gas_debug_profile_stop(wait_time);
#endif
  RECORDRA;
#ifdef PROFILER_OUTPUT
  gas_printf("Sema wait time %u (%ums)\n", final_time, final_time / GAS_PROFILER_MILLISECOND_MULTIPLIER);
#endif

#ifdef DEBUG_OUTPUT
  gas_printf("%s: ", gas.source_list[source].short_filename);
#endif
  if (gas.free_instances.count > 0)
  {
    // use a recycled instance
    i = fifo_queue_pop(&gas.free_instances);
    if (gas.inst_list[i].flag.used == 1)
    {
      gas_printf("This instance is already used for 0x%x\n", gas.inst_list[i].instance_id);
#ifdef HALT_ON_FAIL
      while (1);
#endif
    }
#ifdef DEBUG_OUTPUT
    gas_printf("using recycled instance %d\n", i);
#endif
  }
  else if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
  {
    // try to reclaim one
    _gas_update_reclaim_voices();
#ifdef DEBUG_OUTPUT
    gas_printf("Attempting to reclaim a voice or two\n");
#endif
    if (gas.inst_list_count+1 >= PS2_MAX_VOICES)
    {
      SignalSema(gas.system_state_sema);
      return -1;
    }
  }
  else
  {
    // add the instance to the list of instances
    i = gas.inst_list_count;
#ifdef DEBUG_OUTPUT
    gas_printf("creating new instance %d\n", i);
#endif
  }
  gas.inst_list[i].source = source;
  gas.inst_list[i].instance_id += GAS_INSTANCE_ID_INCREMENT;

//_gas_debug_voice_status_dump();
  if (_gas_rpc_init_new_instance(i, source) == 0)
  {
    SignalSema(gas.system_state_sema);
    return -1;
  }

  // return the index of the GasSource[] that this entry uses
  if (i == gas.inst_list_count)
    gas.inst_list_count++;

  ret_val = gas.inst_list[i].instance_id;
  if (gas.inst_list[i].flag.stereo)
    ret_val |= GAS_INSTANCE_STEREO_FLAG_BIT;

  SignalSema(gas.system_state_sema);

  return ret_val;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P L A Y   I N S T A N C E
//
int gas_rpc_play_instance( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  unsigned short volume_left = args->int_arg[1];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  RECORDRA;

  WaitSema(gas.system_state_sema);
  RECORDRA;
  
  if (gas.inst_list[inst].instance_id != instance_id)
  {
    SignalSema(gas.system_state_sema);
    return 0;
  }

  if (inst >= gas.inst_list_count || inst < 0)
  {
    SignalSema(gas.system_state_sema);
    return 0;
  }

  if (gas.inst_list[inst].pause_count < 1)
  {
    SignalSema(gas.system_state_sema);
    return 1; // instance already playing
  }

  // force an update in _gas_update_volume
  gas.inst_list[inst].voll = volume_left + 1;

  _gas_update_instance_volume( inst, args->int_arg[1], args->int_arg[2] );
  _gas_update_volume(inst);

#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Play\n", gas.source_list[gas.inst_list[inst].source].short_filename, gas.inst_list[inst].instance_id);
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
      SignalSema(gas.system_state_sema);
      return 0;
  }

  SignalSema(gas.system_state_sema);
 
  return 1;
}


//-----------------------------------------------------------------------------------
// G A S   R P C   P A U S E   I N S T A N C E
//
int gas_rpc_pause_instance( GasRpcArgs *args , int pause_mode )
{
  int instance_id = args->int_arg[0];
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  int ret;

  RECORDRA;
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  WaitSema(gas.system_state_sema);
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Stop\n", gas.source_list[gas.inst_list[inst].source].short_filename, gas.inst_list[inst].instance_id);
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
  int ret;

  RECORDRA;
  if (gas.inst_list[inst].instance_id != instance_id)
    return 0;

  WaitSema(gas.system_state_sema);

  if (gas.inst_list[inst].flag.used == 1)
  {
    switch (dampen_mode)
    {
      case MODE_GUARD: 
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Dampen guard\n", gas.source_list[gas.inst_list[inst].source].short_filename, gas.inst_list[inst].instance_id);
#endif
        gas.inst_list[inst].dampen_count = -1;
        break;

      case MODE_UNDAMPEN:
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Undampen\n", gas.source_list[gas.inst_list[inst].source].short_filename, gas.inst_list[inst].instance_id);
#endif
        gas.inst_list[inst].dampen_count = 0;
        break;

      case MODE_DAMPEN:
#ifdef DEBUG_OUTPUT
  gas_printf("%s(0x%x): Dampen\n", gas.source_list[gas.inst_list[inst].source].short_filename, gas.inst_list[inst].instance_id);
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
  int ret;
  int i;

  RECORDRA;
  WaitSema(gas.system_state_sema);
 
  for (i=0; i<gas.inst_list_count; ++i)
  {
    if (gas.inst_list[i].flag.used == 1)
    {
      if (dampen_mode == MODE_UNDAMPEN)
			{
				dampened_flag = 0;
        gas.inst_list[i].dampen_count = 0;
			}
      else if (dampen_mode == MODE_DAMPEN)
			{
				dampened_flag = 1;
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
  int ret;
  int i;

  RECORDRA;
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
  RECORDRA;

  WaitSema(gas.system_state_sema);
  RECORDRA;
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

  RECORDRA;
  WaitSema(gas.system_state_sema);
  RECORDRA;
  if (gas.inst_list[inst].instance_id != instance_id)
  {
    SignalSema(gas.system_state_sema);
    return 0;
  }

  gas.inst_list[inst].pitch = (pitch * gas.source_list[gas.inst_list[inst].source].pitch_one) / 1000;
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
  RECORDRA;

  WaitSema(gas.system_state_sema);
  RECORDRA;
  memcpy(gas.command_list, commands, sizeof(GasCommandEntry) * GAS_MAX_INSTANCES);
  gas.commands_waiting = 1;
  SignalSema(gas.system_state_sema);
}


//-----------------------------------------------------------------------------------
// G A S   R P C   S T A T U S   I S   P L A Y I N G
//
int gas_rpc_status_is_playing( GasRpcArgs *args )
{
  int instance_id = args->int_arg[0];
  int ret = 0;
  int inst = instance_id & GAS_INSTANCE_ID_MASK;
  RECORDRA;

  WaitSema(gas.system_state_sema);
  RECORDRA;
  if (gas.inst_list[inst].instance_id == instance_id)
    ret = gas.inst_list[inst].flag.used && (gas.inst_list[inst].pause_count < 1);
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
  RECORDRA;

  WaitSema(gas.system_state_sema);
  RECORDRA;
  if (gas.inst_list[inst].instance_id == instance_id)
    ret = gas.inst_list[inst].flag.used && gas.inst_list[inst].flag.ready;
  SignalSema(gas.system_state_sema);

  return ret;
}


