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

// This is needed for the alias vs name build options.
#include "../common/ProjectOptions.h"

#define FILENAME_LENGTH 48
#define SCE_VOLUME_MAX 0x3fff
#define GAS_INSTANCE_STEREO_FLAG_BIT 0x0080
#define GAS_MAX_INSTANCES 48

#define MAX_QUEUED_INSTANCES 10

#define GAS_INSTANCE_ID_INCREMENT 0x0100
#define GAS_INSTANCE_ID_MASK      0x00FF
#define GAS_INVALID_ID            -1

#define GAS_SOURCE_BANK_MASK      0xFF
#define GAS_SOURCE_BANK_INCREMENT 0x100
#define GAS_SOURCE_INST_MASK      0xFFFFFF00
#define GET_SOURCE_SLOT(x) ((x & GAS_SOURCE_INST_MASK) >> 8)
#define GET_SOURCE_BANK(x) (x & GAS_SOURCE_BANK_MASK)
#define MAKE_SOURCE_ID(heap, index) ((index << 8) | heap)
/*** standardized struct for rpc fn calls to the gas iop module ***/
typedef struct
{
  char string_arg[FILENAME_LENGTH];
  int int_arg[4];
} GasRpcArgs;


//#define GAS_COMMAND_LIST_SIZE (sizeof(GasCommandEntry) * GAS_MAX_INSTANCES)
#define GAS_COMMAND_LIST_SIZE (sizeof(GasCommandEntry) * 64)

typedef struct // 12 bytes
{
  // We want to add pausing, but screwing this up too much
  // The number of bytes for all entries must be a 
  // multiple of 64.  So we need 2 more bytes
  // one of which can be stolen from volume_left
  // leaving one more bit.  So for now, we will 
  // simply use one
  // We should have 64 entries to make it a multiple
  // of 64.  
  unsigned instance_id;
  
  short set_volume:1;
  short set_paused:1;
  short set_pitch:1;
  short paused:1;
  short unused:12;
  short pitch;
  short volume_right;
  short volume_left;
    

} GasCommandEntry;

// depricated.
//#define SOUND_TYPE_SFX      0
//#define SOUND_TYPE_MUSIC    1
//#define SOUND_TYPE_AMBIENT  2
//#define SOUND_TYPE_VOICE    3


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
#define GAS_RPC_GET_VERSION       0x00ff
#define GAS_RPC_INIT              0x1001
#define GAS_RPC_SHUTDOWN          0x1002
#define GAS_RPC_RESET             0x1003
#define GAS_RPC_ADD_SOURCE        0x0004
#define GAS_RPC_SET_REVERB        0x0005 // I've added these two and I'm assuming these are just unique
#define GAS_RPC_GET_REVERB        0x0006 // ids. I've picked two numbers not yet used.
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

#define GAS_RPC_LOAD_TO_BUFFER    0x00e0
#define GAS_RPC_TRANS_TO_EE       0x00e1
#define GAS_RPC_LOAD_SND_LIST     0x00e2
#define GAS_RPC_PLAY_IOP_BUFFER   0x10e3
#define GAS_RPC_FINALISE_SRCS     0x00e4
#define GAS_RPC_QUERY_MEM         0x00e5
#define GAS_RPC_GET_STREAMING_BUFFER 0x00e6
#define GAS_RPC_SET_STEREO        0x00e7
#define GAS_RPC_GET_SOURCE_TYPE   0x00e8
#define GAS_RPC_GET_SOURCE_LOOPING   0x00e9
#define GAS_RPC_GET_SOURCE_LENGTH 0x00ea
#define GAS_RPC_GET_SOURCE_FREQ   0x00eb
#define GAS_RPC_SET_HOST_STREAM   0x00ec
#define GAS_RPC_PARTIAL_SHUTDOWN  0x00ed // stop the update thread, halt GAS while the EE does stuff
#define GAS_RPC_PARTIAL_INIT      0x00ee // return GAS to its normal state, re-initialize sound system
#define GAS_RPC_SET_ROOT_DIR      0x00ef
#define GAS_RPC_GET_SRC_VOLUME    0x00f0
#define GAS_RPC_GET_SOURCE_PADDED_LENGTH 0x00f1
#define GAS_RPC_GET_SOUND_POSITION 0x00f2
#define GAS_RPC_ADD_INSTANCE_WITH_OFFSET 0x00f3
#define GAS_RPC_PUSH_BANK         0x00f4
#define GAS_RPC_POP_BANK          0x00f5
#define GAS_RPC_GET_SOURCE_BANK   0x00f6
#define GAS_RPC_PRINTF            0x1f10

#define	GAS_RPC_OPEN_RAWSTREAM	   0x00F7
// module ID number
#define GAS_DEV 0x00012345

unsigned short percentile_to_volume(int percentile);
int volume_to_percentile(unsigned short volume);

#endif
