/*=======================================================================================*
 * Groovin' Audio System (GAS) for the Playstation2
 *---------------------------------------------------------------------------------------*
 * Author - Greg Taylor (2/5/01)
 *---------------------------------------------------------------------------------------*
 * An audio library for the PS2 which is primarily a stand-alone IOP program.  This 
 * program (like many of the PS2 IRX libs from Sony) is loaded onto the IOP and responds
 * to RPC calls from the main program (running on the EE).
 *=======================================================================================*/
#ifndef GAS_SYSTEM_HEADER
#define GAS_SYSTEM_HEADER

/////////////////////////////// The UGLY Line /////////////////////////////////

// todo
// - add command buffers
// - do add_source fn command buffer style
// - do remove_source fn command buffer style
// - do add_instance (spu) fn command buffer style
// - do play_instance (spu) fn command buffer style
// - do add_instance (cd) fn command buffer style
// - do play_instance (cd) fn command buffer style
// etc.

// todo
// - make the source list search faster

#include "iop_fifo_queue.h" 
#include "../gas.h"
#include <libsd.h>

/*** constants ***/
#define PS2_MAX_VOICES_PER_CORE 24
#define PS2_MAX_VOICES (PS2_MAX_VOICES_PER_CORE * 2)

// remove these
#define IOP_N_BUFFER      2 // how many buffers to use (more than 2 is no longer supported)
#define IOP_N_BUFFER_BITS 2 // the minimum number of bits required to represent n-buffer side


// note to future programmers - these structures' members are ordered so that the OS can
// pack them into a small space (two shorts next to one another, four chars or one int value)
// so when editting them, try to preserve this ordering and add new members efficiently.
// thanks =) (GT-2/5/01)


/*** GasSource ***/
typedef struct 
{
  char full_filename[FILENAME_LENGTH];
  char short_filename[12];
  unsigned size;

  // the pitch value that is 'one', that is, normal pitch
  unsigned short pitch_one;

  // default volumes for this source (can be overriden in instance)
  unsigned short voll;
  unsigned short volr;
  unsigned short rvoll;
  unsigned short rvolr;

  union {
    struct { // ps2 cd/dvd drive
      unsigned int start_sector;
    } cd;
    struct { // resident in spu memory (no streaming, no stereo)
      unsigned addr;
    } spu;
  } src; 

  struct {
    unsigned char stereo:1;
    unsigned char loop:1;
    unsigned char loaded:1;
    unsigned char src_type:1;
  } flag;
} GasSource;


/*** GasInstance ***/
typedef struct 
{
  unsigned short pitch;

  short source;

  // volumes for this instance
  short voll;   // mono or left channel volumes
  short volr;
  short rvoll;  // (stereo only) right channel volumes
  short rvolr;

  short target_voll;   // mono or left channel volumes
  short target_volr;
  short target_rvoll;  // (stereo only) right channel volumes
  short target_rvolr;

  char pause_count;      // ref count of pause requests
  char dampen_count;     // ref count of dampen requests

// questionable....
  char spu_reload;
  char iop_reload;

  // only used in non-spu instances
  unsigned char *spu_buf_crit;
  unsigned char *curr_iop_buf_addr;
  unsigned char *last_iop_buf_addr;
  char spu_bufl;
  char spu_bufr;
  char iop_buf;

  // only used in ee and cd streaming instances
  unsigned char *iop_buf_addr[IOP_N_BUFFER];  // IOP SMEM address of the start of buffer for this sound (dbl buffered)

  // The instance_id of this instance on the outside
  // (& with the GAS_INSTANCE_ID_MASK (below) to get the internal instance id)
  unsigned int   instance_id; 

  unsigned int   iop_load_amount[IOP_N_BUFFER];
  unsigned char  iop_target_load_amount;      

  // src_type specific stuff
  union {
    struct { // ps2 cd/dvd drive
      unsigned int curr_sector;
      unsigned int sectors_remaining;
    } cd;
    struct { // resident in spu memory (no streaming, no stereo)
      unsigned cur_addr;
    } spu;
  } src; 

  struct {
    unsigned spu_buf_side:1; // not used in all instances
    unsigned voicel:5;
    unsigned voicer:5;
    unsigned corel:1;
    unsigned corer:1; 

    unsigned iop_buf_side:IOP_N_BUFFER_BITS; // not used in all instances
    unsigned src_type:2;
    unsigned loop:1;
    unsigned stereo:1;
    unsigned used:1;
    unsigned small_sound:1;
    unsigned ready:1;                // cd instance has been preloaded
    unsigned needs_spu_preload:1;
    unsigned volume_touched:1;
  } flag;
} GasInstance;



/*** GasSystemState ***/
typedef struct
{
  // semaphore to control access to this structure
  int system_state_sema;                                            // used
	int	dampened_flag;

  // iop stuff
  unsigned char * iop_buffers_top;          // constant             // used
  unsigned char * iop_cd_mono_buffers_top;  // constant             // used

  // spu stuff
  short spu_buffer_num;  // constant                                // used
  int spu_heap_start;    // constant                                // used
  int spu_heap_end;      // constant                                // used
  int spu_heap_curr;                                                // used

  // queues
  fifo_queue free_voices;
  fifo_queue free_instances;
  fifo_queue free_cd_stereo_bufs;
  fifo_queue free_cd_mono_bufs;
  fifo_queue free_spu_bufs;
  fifo_queue reload_spu;
  fifo_queue reload_cd_streams;
  fifo_queue cd_preloads_pending;

  // debugging stuff
  unsigned int voice_used[PS2_MAX_VOICES];

  // instance list
  GasInstance inst_list[GAS_MAX_INSTANCES];               // used
  short inst_list_count;                                  // used

  // source list
  short source_list_count;                                // used
  GasSource *source_list;                                 // used

  // command stuff
  GasCommandEntry command_list[GAS_MAX_INSTANCES];
  char  commands_waiting;
  char  proview_mode;                                     // used

  // misc
  short preloading_instance;
  short dampen_level;
  char *host_prefix;                                      // used
  
} GasSystemState;



#define PS2_CD_SECTOR_SIZE 2048

// note: keep iop_buffer_half an even multiple of spu_buffer_size
#define CD_STEREO_BUFFER_SECTORS 160
#define CD_STEREO_BUFFER_SECTORS_N_CHUNK (CD_STEREO_BUFFER_SECTORS / IOP_N_BUFFER)
#define CD_STEREO_BUFFER_SIZE ( PS2_CD_SECTOR_SIZE * CD_STEREO_BUFFER_SECTORS )
#define CD_STEREO_BUFFER_N_CHUNK ( PS2_CD_SECTOR_SIZE * CD_STEREO_BUFFER_SECTORS_N_CHUNK )

#define CD_MONO_BUFFER_SECTORS 40
#define CD_MONO_BUFFER_SECTORS_N_CHUNK (CD_MONO_BUFFER_SECTORS / IOP_N_BUFFER)
#define CD_MONO_BUFFER_SIZE ( PS2_CD_SECTOR_SIZE * CD_MONO_BUFFER_SECTORS )
#define CD_MONO_BUFFER_N_CHUNK ( PS2_CD_SECTOR_SIZE * CD_MONO_BUFFER_SECTORS_N_CHUNK )

#define SPU_BUFFER_SIZE ( 2 * PS2_CD_SECTOR_SIZE )
#define SPU_BUFFER_HALF    ( PS2_CD_SECTOR_SIZE )

// SPU memory defines, in bytes
#define SPU_MEMORY_TOP		    0x5010
#define SPU_MEMORY_MAX		    (2*1024*1024)

/*** constants ***/
#define GAS_IOP_PRIORITY 50

#define SRC_TYPE_SPU  0x00
#define SRC_TYPE_CD   0x01
 
#define DEBUG 1
//#define DEBUG_OUTPUT 1
#define HALT_ON_FAIL 1
//#define PROFILER_OUTPUT 1

#define RELOAD_FLAG_SIDE2         2
#define RELOAD_FLAG_SIDE1         1
#define RELOAD_FLAG_SIDE0         0
#define RELOAD_FLAG_NO_RELOAD    -1
#define RELOAD_FLAG_RECYCLE_ME   -2
#define RELOAD_FLAG_DEAD         -3

// this value can be tweaked to reduce audio pops (lower values == less pops, in theory, but
// try to keep it at the highest value without pops for performance reasons)
#define GAS_SAFE_VOLUME_DELTA     0x0010

#define VNUM_NO_VOICE 31

// for dampen and pause functions
#define MODE_STOP      0
#define MODE_PAUSE     1
#define MODE_UNPAUSE  -1
#define MODE_DAMPEN    1
#define MODE_UNDAMPEN -1
#define MODE_GUARD    -2

// in GasRpc.c
void gas_rpc_init( GasRpcArgs *args );
void gas_rpc_reset( GasRpcArgs *args );
void gas_rpc_shutdown();
extern GasSystemState gas;

// in GasRpcCommand.c
int  gas_rpc_add_source( GasRpcArgs *args );
int  gas_rpc_add_instance( GasRpcArgs *args );
int  gas_rpc_play_instance( GasRpcArgs *args );
int  gas_rpc_pause_instance( GasRpcArgs *args, int pause_mode );
int  gas_rpc_pause_all( GasRpcArgs *args, int pause_mode );
int  gas_rpc_instance_volume( GasRpcArgs *args );
int  gas_rpc_instance_pitch( GasRpcArgs *args );
void gas_rpc_command_list( void *data );
int  gas_rpc_status_is_playing( GasRpcArgs *args );
int  gas_rpc_status_is_ready( GasRpcArgs *args );
int  gas_rpc_dampen_instance( GasRpcArgs *args, int dampen_mode);
int  gas_rpc_dampen_all( GasRpcArgs *args, int dampen_mode);
int  gas_rpc_set_master_volume( GasRpcArgs *args );

// in GasRpcCommandSupport.c
int  _gas_rpc_add_spu_source(int entry);
int  _gas_rpc_add_cd_source(int entry);
void _gas_rpc_play_spu_instance(int inst);
void _gas_rpc_play_cd_instance(int inst);
int  _gas_rpc_init_spu_streaming(int inst, int source);
int  _gas_rpc_preload_spu_from_iop(unsigned char *iop_buf_addr, int spu_buf_num, int spu_buf_side);
int  _gas_rpc_pause_spu_instance(int inst, int pause_mode);
int  _gas_rpc_pause_cd_instance(int inst, int pause_mode);
int  _gas_rpc_init_new_instance(int inst, int source);

// in GasRpcUtility.c
int    gas_rpc_create_sema(int used_flag);
void   gas_rpc_delete_sema(int sema_id);
char* _gas_rpc_load_file(char *orig_filename, int *size);
void  _gas_rpc_streamed_voice_controller(int inst, int mode, int store_pos_flag);
int   _gas_rpc_dma_iop_to_spu_blocking(unsigned char *iop_addr, unsigned char *spu_addr, int size);
int   _gas_rpc_dma_iop_to_spu_nonblocking(unsigned char *iop_addr, unsigned char *spu_addr, int size);

int   _gas_rpc_get_voice(int *core, int *voice);
void  _gas_rpc_set_voice (int inst, int side, unsigned int spu_addr);
void  _gas_debug_voice_status_dump();
void  _gas_debug_voice_status_dump_instance(int inst);
void  _gas_debug_voice_status_dump_instance_aux(int inst);
void  _gas_debug_voice_envelope_dump();
void  _gas_debug_spu_mem_dump();
void  _gas_debug_spu_mem_clear();
void  gas_debug_check_value(const char *context_str, int check_me, int low_end, int high_end);

// in GasRpcSoundFileParser.c
int _gas_rpc_load_list_file(char *filename, int file_size, GasSource **file_list, short *file_list_count);

// in GasUpdate.c
unsigned int gas_update_thread(void *common);

// in GasUpdateSupport.c
void _gas_update_volume(int inst);
void _gas_update_init_iop_streaming(int inst, int source);
void _gas_update_mark_up_data_for_spu(int which);
void _gas_update_do_iop_to_spu_transfer(int prim, int which, unsigned char *spu_addr);
void _gas_update_instance_volume( int inst, short voll, short volr );
void  gas_update_set_voice_volume (int core, int voice, int voll, int volr);
void  gas_update_set_voice_pitch (int core, int voice, int pitch);
int  _gas_update_reclaim_voices();
int  _gas_update_recycle_instance(int inst);
void _gas_update_recycle_voice(int core, int voice);unsigned gas_update_get_NAX(int core,int ch);


/*** macros ***/
#define PS2
void Debug_Record();
#if DEBUG
#define RECORDRA Debug_Record() 
#else
#define RECORDRA NULL
#endif

// profiling
extern int g_profiler;
#define gas_debug_profile_start() GetTimerCounter(g_profiler)
u_long gas_debug_profile_stop(u_long start_time);
void gas_debug_init_profiler();
#define GAS_PROFILER_MILLISECOND_MULTIPLIER 1000

#define gas_printf printf

#define _ADPCM_MARK_START 0x04
#define _ADPCM_MARK_LOOP  0x02
#define _ADPCM_MARK_END   0x01
#define _ADPCM_MARK_STOP1 0x00000700
#define _ADPCM_MARK_STOP2 0x00000000


#define _AdpcmSetMarkSTART(a,s) { \
  *((unsigned char *)((a)+1)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_START); \
  *((unsigned char *)((a)+0x10+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	_ADPCM_MARK_LOOP; \
	FlushDcache (); }

#define _AdpcmSetMarkEND(a,s) { \
  *((unsigned char *)((a)+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+0x10+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_END); \
	FlushDcache (); }

#define _AdpcmSetMarkLOOP(a,s) { \
  *((unsigned char *)((a)+0x10+1)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_START); \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_END); \
	FlushDcache (); }

#define _AdpcmSetMarkSTOP(a,s) { \
  *((unsigned *)((a)+(s)-0x10)) = (_ADPCM_MARK_STOP1); \
  *((unsigned *)((a)+(s)-0x0C)) = (_ADPCM_MARK_STOP2); \
  *((unsigned *)((a)+(s)-0x08)) = (_ADPCM_MARK_STOP2); \
  *((unsigned *)((a)+(s)-0x04)) = (_ADPCM_MARK_STOP2); \
	FlushDcache (); }

#endif