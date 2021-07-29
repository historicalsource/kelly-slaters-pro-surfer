/*=======================================================================================*
 * GTreyarch Audio System (GAS) for the Playstation2
 *---------------------------------------------------------------------------------------*
 * Author - Greg Taylor (2/5/01)
 *---------------------------------------------------------------------------------------*
 * An audio library for the PS2 which is primarily a stand-alone IOP program.  This 
 * program (like many of the PS2 IRX libs from Sony) is loaded onto the IOP and responds
 * to RPC calls from the main program (running on the EE).
 *=======================================================================================*/
#ifndef GAS_HEADER
#define GAS_HEADER

#define FILENAME_LENGTH 48
#define SCE_VOLUME_MAX 0x3fff
#define GAS_INSTANCE_STEREO_FLAG_BIT 0x0080
#define GAS_MAX_INSTANCES 48

#define GAS_INSTANCE_ID_INCREMENT 0x0100
#define GAS_INSTANCE_ID_MASK      0x00FF

/*** standardized struct for rpc fn calls to the gas iop module ***/
typedef struct
{
  char string_arg[FILENAME_LENGTH];
  int int_arg[4];
} GasRpcArgs;


#define GAS_COMMAND_LIST_SIZE (sizeof(GasCommandEntry) * GAS_MAX_INSTANCES)
typedef struct // 8 bytes
{
  unsigned instance_id;
  
  unsigned short set_volume:1;
  unsigned short volume_right:15;
  unsigned short volume_left;
} GasCommandEntry;
#define GAS_COMMAND_VOLUME_MAX      0x3fff
#define GAS_COMMAND_PITCH_MAX       0x7fff
#define GAS_COMMAND_PITCH_ONE       0x0800
#define GAS_COMMAND_EFFECT_MIX_MAX  0x0fff
#define GAS_COMMAND_EFFECT_TYPE_MAX 0x000f
#define GAS_COMMAND_DAMPEN_ONE      100


typedef struct // 1 byte
{
  unsigned char is_valid:1;
  unsigned char is_ready:1;
  unsigned char is_playing:1;
  unsigned char unused_bits:5;
} GasStatusEntry;

typedef struct // 32 bytes
{
  int rpc_retval1;
  int rpc_retval2;
  GasStatusEntry instances[GAS_MAX_INSTANCES];
} GasStatus;

#define GAS_RPC_NON_BLOCKING_MASK 0x1000

// rpc-commands
#define GAS_RPC_INIT              0x1001
#define GAS_RPC_SHUTDOWN          0x1002
#define GAS_RPC_RESET             0x1003
#define GAS_RPC_ADD_SOURCE        0x0004
#define GAS_RPC_REMOVE_SOURCE     0x1005
#define GAS_RPC_ADD_INSTANCE      0x0008
#define GAS_RPC_PLAY_INSTANCE     0x1009
#define GAS_RPC_INSTANCE_VOLUME   0x000d// going out of style
#define GAS_RPC_INSTANCE_PITCH    0x100e
#define GAS_RPC_COMMAND_LIST      0x100f

#define GAS_RPC_DAMPEN_GUARD      0x1010
#define GAS_RPC_DAMPEN_INSTANCE   0x1020
#define GAS_RPC_UNDAMPEN_INSTANCE 0x1030
#define GAS_RPC_DAMPEN_ALL        0x1040
#define GAS_RPC_UNDAMPEN_ALL      0x1050

#define GAS_RPC_PAUSE_GUARD       0x1060
#define GAS_RPC_PAUSE_INSTANCE    0x1070
#define GAS_RPC_UNPAUSE_INSTANCE  0x1080
#define GAS_RPC_PAUSE_ALL         0x1090
#define GAS_RPC_UNPAUSE_ALL       0x10a0

#define GAS_RPC_STOP_INSTANCE     0x10b0
#define GAS_RPC_STOP_ALL          0x10c0

#define GAS_RPC_SET_MASTER_VOLUME 0x10d0

#define GAS_RPC_STATUS_IS_PLAYING 0x0010
#define GAS_RPC_STATUS_IS_READY   0x0020

// debugging calls
#define GAS_RPC_VOICE_STATUS      0x1e00
#define GAS_RPC_SPU_MEM_DUMP      0x1f00

// module ID number
#define GAS_DEV 0x00012345

#define MAX_CD_STEREO_STREAMS 2
#define MAX_CD_MONO_STREAMS 8
#define MAX_CD_STREAMS (MAX_CD_MONO_STREAMS + MAX_CD_STEREO_STREAMS)

#endif
