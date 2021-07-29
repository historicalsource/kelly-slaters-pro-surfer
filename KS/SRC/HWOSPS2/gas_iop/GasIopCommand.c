#include <kernel.h>
#include <sys/file.h>
#include <libcdvd.h>
#include <stdio.h>
#include "../gas_iop.h" 
#include "GasIopSystem.h"


/*** gas_play_instance ***/
int _gas_play_instance(int inst)
{
  if (gas.inst_list[inst].pause_count < 1)
    return 1; // instance already playing

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


/*** _gas_pause_instance ***/
int _gas_pause_instance(int inst, int pause_mode)
{
  int ret;

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


/*** _gas_instance_volume ***/
int _gas_instance_volume(int inst, unsigned short voll, unsigned short volr)
{
gas_printf("Instance(u) 0x%x - Core %d voice %d volume L/R (0x%x/0x%x)\n", inst, 
      gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
      gas.inst_list[inst].voll, gas.inst_list[inst].volr);
  if (gas.inst_list[inst].flag.stereo == 0)
  {
    gas.inst_list[inst].voll = (voll * PS2_VOLUME_MAX) / GAS_COMMAND_VOLUME_MAX;
    gas.inst_list[inst].volr = (volr * PS2_VOLUME_MAX) / GAS_COMMAND_VOLUME_MAX;

    _gas_set_voice_volume (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
      gas.inst_list[inst].voll, gas.inst_list[inst].volr);
  }
  else
  {
    gas.inst_list[inst].voll = (voll * PS2_VOLUME_MAX) / GAS_COMMAND_VOLUME_MAX;
    gas.inst_list[inst].volr = 0;
    gas.inst_list[inst].rvoll = 0;
    gas.inst_list[inst].rvolr = (volr * PS2_VOLUME_MAX) / GAS_COMMAND_VOLUME_MAX;
    _gas_set_voice_volume (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
      gas.inst_list[inst].voll, gas.inst_list[inst].volr);
    _gas_set_voice_volume (gas.inst_list[inst].flag.corer, gas.inst_list[inst].flag.voicer, 
      gas.inst_list[inst].rvoll, gas.inst_list[inst].rvolr);
  }

  return 1;
}


/*** _gas_instance_pitch ***/
int _gas_instance_pitch(int inst, unsigned short pitch)
{
  gas.inst_list[inst].pitch = (pitch * gas.source_list[gas.inst_list[inst].source].pitch_one) / GAS_COMMAND_PITCH_MAX;
  _gas_set_voice_pitch (gas.inst_list[inst].flag.corel, gas.inst_list[inst].flag.voicel, 
    gas.inst_list[inst].pitch);
  return 1;
}


/*** _gas_set_voice_volume ***/
void _gas_set_voice_volume (int core, int voice, int voll, int volr)
{
  sceSdSetParam (core | (voice<<1) | SD_VP_VOLL, voll );
	sceSdSetParam (core | (voice<<1) | SD_VP_VOLR, volr );
}


/*** _gas_set_voice_pitch ***/
void _gas_set_voice_pitch (int core, int voice, int pitch)
{
  sceSdSetParam (core | (voice<<1) | SD_VP_VOLL, pitch );
}

