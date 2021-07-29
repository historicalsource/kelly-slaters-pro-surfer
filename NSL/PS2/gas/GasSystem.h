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
#include "instr_fifo_queue.h"
#include "gasdefines.h"
#include "../gas.h"
#ifndef ADPCM_TOOL
#include <libsd.h>
#else
typedef unsigned long u_long;
#endif

/*** constants ***/
#define MAX_QUEUED_INSTRUCTIONS 256
#define PS2_MAX_VOICES_PER_CORE 24
#define PS2_MAX_VOICES (PS2_MAX_VOICES_PER_CORE * 2)


#define MAX_NUM_HEAPS 20


// remove these
#define IOP_N_BUFFER      2 // how many buffers to use (more than 2 is no longer supported)
#define IOP_N_BUFFER_BITS 2 // the minimum number of bits required to represent n-buffer side


// Some debugging tools
// This is defined in the makefile now   #define DEBUG 1
//#define DEBUG_OUTPUT 1
//#define DEBUG_GOODIES 1

//#define HALT_ON_FAIL 1
//#define PROFILER_OUTPUT 1


// note to future programmers - these structures' members are ordered so that the OS can
// pack them into a small space (two shorts next to one another, four chars or one int value)
// so when editting them, try to preserve this ordering and add new members efficiently.
// thanks =) (GT-2/5/01)

// malkia - I've removed the ";1" from GAS_STREAM_VBC and added that to the code
// This is done to support streams with other names
// I've removed also GAS_HOST_STREAM_VBC as it's no longer used
#define GAS_STREAM_VBC        "\\STREAM\\STREAM.VBC"
extern char gas_root_dir[256];

//#define STREAM_FROM_HOST    1

//#pragma pack(push,1)

/*** GasSource ***/
typedef struct 
{
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  int aliasID;
#endif

#ifdef NSL_LOAD_SOURCE_BY_NAME
  char filename[FILENAME_LENGTH];
#endif

  unsigned int offset;
  unsigned size;
  unsigned padsize;
  // the pitch value that is 'one', that is, normal pitch
  unsigned short pitch_one;
#ifdef ADPCM_TOOL
  unsigned long freq;
  char *vag_buffer;
#endif
  // default volumes for this source (can be overriden in instance)
  unsigned short voll;
  unsigned short volr;

  union {
    struct { // ps2 cd/dvd drive
      unsigned int start_sector;
    } cd;
    struct { // resident in spu memory (no streaming, no stereo)
      unsigned addr;
    } spu;
  } src; 
  int sound_type;
  struct {
    unsigned char stereo:1;
    unsigned char loop:1;
    unsigned char loaded:1;
    unsigned char reverb:1;
#ifndef ADPCM_TOOL
    unsigned char src_type:1;
#else
    int src_type;
#endif
  } flag;

  int rawStreamHostFd;
} GasSource;


/*** GasInstance ***/
typedef struct 
{
  int source;

  unsigned short pitch;

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

#ifdef DEBUG_GOODIES
  unsigned int batch;
  unsigned int old_addr;
  unsigned int old_nax;
#endif

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
      unsigned int total_bytes_played;
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
    unsigned stopping:1;
  } flag;
  int played;
  int offset;
} GasInstance;

//#pragma pack(pop)


typedef struct 
{
  unsigned int in_use_instance;
  unsigned int in_use;
} voice_slot;


typedef struct 
{
  unsigned int in_use_instance;
  unsigned int in_use;
} free_instance_slot;


typedef struct 
{
  char spu_collection[FILENAME_LENGTH];
  unsigned int start;
  unsigned int end;
  short source_list_count;                                
} spu_heap_info;


/*** GasSystemState ***/
typedef struct
{
  // semaphore to control access to this structure
  int system_state_sema;                                            // used
  int instr_list_sema;
  int cmd_list_sema;

	int	dampened_flag;
  int stream_from_host;
  // iop stuff
  unsigned char * iop_buffers_top;          // constant             // used
  unsigned char * iop_cd_mono_buffers_top;  // constant             // used

  // spu stuff
  short spu_buffer_num;  // constant                                // used
  int spu_heap_start;    // constant                                // used
  int spu_heap_end;      // constant                                // used
  int spu_heap_curr;                                                // used
  
  // queues  shouldm't be queues
  voice_slot free_voices[PS2_MAX_VOICES];
  //fifo_queue free_voices;              // target areas this and rpc stop correctly cleaning up state
  free_instance_slot free_instances[GAS_MAX_INSTANCES];
  //fifo_queue free_instances;
  fifo_queue free_cd_stereo_bufs;
  fifo_queue free_cd_mono_bufs;
  fifo_queue free_spu_bufs;

  // maybe still queues..
  fifo_queue reload_spu;
  fifo_queue reload_cd_streams;
  fifo_queue cd_preloads_pending;


  fifo_instr_queue incomingGasInstructions;

  // debugging stuff
  unsigned int voice_used[PS2_MAX_VOICES];

  // instance list
  GasInstance inst_list[GAS_MAX_INSTANCES];               // used
  short inst_list_count;                                  // used

  // source list
  GasSource *source_list[MAX_NUM_HEAPS];              // used
  unsigned char num_heaps;


  int host_streaming_fd;

  // command stuff
  GasCommandEntry command_list[GAS_MAX_INSTANCES];
  char  commands_waiting;
  char  proview_mode;                                     // used
  
  char  stereo;
  char  dvd_mode;
  
  // misc
  short preloading_instance;
  short dampen_level;
  char* host_prefix;                                      // not used
  
  // The spu heap info
  spu_heap_info spu_heaps[MAX_NUM_HEAPS];
} GasSystemState;



#define SPU_BUFFER_SIZE ( 2 * PS2_CD_SECTOR_SIZE )
#define SPU_BUFFER_HALF    ( PS2_CD_SECTOR_SIZE )

// SPU memory defines, in bytes
#define SPU_MEMORY_TOP		    0x5010
#define SPU_MEMORY_MAX 		    (2*1024*1024)
#define SPU_FX_WORK_AREA_SIZE	(0x20000)

/*** constants ***/
#define GAS_IOP_PRIORITY 50

#define SRC_TYPE_SPU  0x00
#define SRC_TYPE_CD   0x01
#define SRC_TYPE_NONE 0x02
 
#define RELOAD_FLAG_SIDE2         2
#define RELOAD_FLAG_SIDE1         1
#define RELOAD_FLAG_SIDE0         0
#define RELOAD_FLAG_NO_RELOAD    -1
#define RELOAD_FLAG_RECYCLE_ME   -2
#define RELOAD_FLAG_DEAD         -3

// this value can be tweaked to reduce audio pops (lower values == less pops, in theory, but
// try to keep it at the highest value without pops for performance reasons)
#define GAS_SAFE_VOLUME_DELTA     0x0800

#define VNUM_NO_VOICE 31

// for dampen and pause functions
#define MODE_STOP      0
#define MODE_PAUSE     1
#define MODE_UNPAUSE  -1
#define MODE_DAMPEN    1
#define MODE_UNDAMPEN -1
#define MODE_GUARD    -2

// in GasRpc.c
int gas_rpc_get_version();
void gas_rpc_init( GasRpcArgs *args );
void gas_rpc_reset();
void gas_rpc_shutdown();
int gas_rpc_load_snd_list( GasRpcArgs *args );
int gas_rpc_finalise_srcs();
void *gas_rpc_get_streaming_buffer();
void _gas_rpc_partial_init();
unsigned int gas_rpc_push_bank( GasRpcArgs *args );
unsigned int gas_rpc_pop_bank( );

extern GasSystemState gas;

// in GasRpcCommand.c
void *gas_rpc_load_to_buffer( GasRpcArgs *args );
int  gas_rpc_trans_to_ee( GasRpcArgs *args );
int  gas_rpc_query_mem();
int  gas_rpc_add_source( GasRpcArgs *args );
int  gas_rpc_get_reverb( GasRpcArgs *args );
int  gas_rpc_set_reverb( GasRpcArgs *args );
int  gas_rpc_add_instance( GasRpcArgs *args );
int  gas_rpc_add_instance_with_offset( GasRpcArgs *args );
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
int  gas_rpc_source_type( GasRpcArgs *args );
int  gas_rpc_source_looping( GasRpcArgs *args);
int  gas_rpc_queue_stop ( GasRpcArgs *args );
int  gas_rpc_set_stereo( GasRpcArgs *args );
void gas_process_instruction_list();
void gas_stop_queued( int instance_id );
int gas_rpc_source_size( GasRpcArgs *args );
int gas_rpc_source_freq( GasRpcArgs *args );
void gas_rpc_set_host_stream( GasRpcArgs *args );
void gas_rpc_partial_shutdown( GasRpcArgs *args );
void gas_rpc_partial_init( GasRpcArgs *args );
void gas_rpc_set_root_dir( GasRpcArgs *args );
int gas_rpc_get_source_volume( GasRpcArgs *args );
int _gas_ready_for_play( int instance_id );
unsigned int gas_rpc_get_sound_position( GasRpcArgs *args );
unsigned int gas_rpc_source_length( GasRpcArgs *args );
unsigned int gas_rpc_source_padded_length( GasRpcArgs *args );
int  gas_rpc_printf( GasRpcArgs *args );


// in GasRpcCommandSupport.c
int  _gas_rpc_load_all_spu_sources();
int  _gas_rpc_add_spu_source(int heap, int entry);
int  _gas_rpc_add_cd_source(int heap,  int entry);
void _gas_rpc_play_spu_instance(int inst);
void _gas_rpc_play_cd_instance(int inst);
int  _gas_rpc_init_spu_streaming(int inst, int source);
int  _gas_rpc_preload_spu_from_iop(unsigned char *iop_buf_addr, int spu_buf_num, int spu_buf_side, int end);
int  _gas_rpc_pause_spu_instance(int inst, int pause_mode);
int  _gas_rpc_pause_cd_instance(int inst, int pause_mode);
int  _gas_rpc_init_new_instance(int inst, int source);

// in GasRpcUtility.c
int    gas_rpc_create_sema(int used_flag);
void   gas_rpc_delete_sema(int sema_id);
int   _gas_rpc_get_fd(char *filename);
void  _gas_rpc_streamed_voice_controller(int inst, int mode, int store_pos_flag);
int   _gas_rpc_dma_iop_to_spu_blocking(unsigned char *iop_addr, unsigned char *spu_addr, int size);
int   _gas_rpc_dma_iop_to_spu_nonblocking(unsigned char *iop_addr, unsigned char *spu_addr, int size);

int   _gas_rpc_get_voice(int inst, int *core, int *voice);
void  _gas_rpc_set_voice (int inst, int side, unsigned int spu_addr);
#ifdef DEBUG
void  _gas_debug_voice_status_dump();
void  _gas_debug_voice_status_dump_instance(int inst);
void  _gas_debug_voice_status_dump_instance_aux(int inst);
void  _gas_debug_voice_envelope_dump();
void  _gas_debug_spu_mem_dump();
void  _gas_debug_spu_mem_clear();
void  gas_debug_check_value(const char *context_str, int check_me, int low_end, int high_end);
#endif

int   _gas_find_and_claim_free_voice(int inst);
int   _gas_find_and_claim_free_voice_stereo(int inst, int *v1, int *v2);

int   _gas_release_voice(int inst, int which_voice);
int   _gas_acquire_voice(int inst, int which_voice);
void  _gas_error_halt(const char *msg);

int   _gas_release_instance_slot(int which_instance_slot);
int   _gas_acquire_instance_slot(int which_instance_slot);
int   _gas_find_and_claim_free_instance_slot();
int   _gas_rpc_get_voice_stereo(int inst_slot, int *core1, int *voice1, int *core2, int *voice2);

// in GasRpcSoundFileParser.c
int _gas_rpc_load_list_file(char *level_name, int file_size, GasSource **file_list, short *file_list_count);

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
int  _gas_update_recycle_instance_if_finished( unsigned short v );
int  _gas_update_reclaim_voices();
int  _gas_update_recycle_instance(int inst);
void _gas_update_recycle_voice(int inst, int core, int voice);
unsigned gas_update_get_NAX(int core,int ch);
int _gas_check_inst_playing(unsigned short inst);

int _gas_open_rawstream( char *filename, int sampleRate, int sampleFormat, int channelCount, int chunkSize );


/*** macros ***/
#define PS2
void Debug_Record();
#if DEBUG
#define RECORDRA Debug_Record();
#else
#define RECORDRA
#endif

// profiling
extern int g_profiler;
#define gas_debug_profile_start() GetTimerCounter(g_profiler)
u_long gas_debug_profile_stop(u_long start_time);
void gas_debug_init_profiler();
void gas_debug_shutdown_profiler();
#define GAS_PROFILER_MILLISECOND_MULTIPLIER 1000
#define SAFE_SPU_POLL_TIME (1)
#define gas_printf printf

#define _ADPCM_MARK_START 0x04
#define _ADPCM_MARK_LOOP  0x02
#define _ADPCM_MARK_END   0x01
#define _ADPCM_MARK_STOP1 0x00000700
#define _ADPCM_MARK_STOP2 0x00000000

#ifndef ADPCM_TOOL
#define _AdpcmSetMarkSTART(a,s) { \
  *((unsigned char *)((a)+1)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_START); \
  *((unsigned char *)((a)+0x10+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	_ADPCM_MARK_LOOP; \
	FlushDcache (); }
#else 
#define _AdpcmSetMarkSTART(a,s) { \
  *((unsigned char *)((a)+1)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_START); \
  *((unsigned char *)((a)+0x10+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	_ADPCM_MARK_LOOP; \
	}
#endif 


#ifndef ADPCM_TOOL
#define _AdpcmSetMarkEND(a,s) { \
  *((unsigned char *)((a)+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+0x10+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_END); \
	FlushDcache (); }
#else 
#define _AdpcmSetMarkEND(a,s) { \
  *((unsigned char *)((a)+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+0x10+1)) = \
	_ADPCM_MARK_LOOP; \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_END); \
	}

#endif 


#ifndef ADPCM_TOOL
#define _AdpcmSetMarkLOOP(a,s) { \
  *((unsigned char *)((a)+0x10+1)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_START); \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_END); \
	FlushDcache (); }
#else
#define _AdpcmSetMarkLOOP(a,s) { \
  *((unsigned char *)((a)+0x10+1)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_START); \
  *((unsigned char *)((a)+(s)-0x0f)) = \
	(_ADPCM_MARK_LOOP | _ADPCM_MARK_END); \
  }
#endif
#ifndef ADPCM_TOOL
#define _AdpcmSetMarkSTOP(a,s) { \
  *((unsigned *)((a)+(s)-0x10)) = (_ADPCM_MARK_STOP1); \
  *((unsigned *)((a)+(s)-0x0C)) = (_ADPCM_MARK_STOP2); \
  *((unsigned *)((a)+(s)-0x08)) = (_ADPCM_MARK_STOP2); \
  *((unsigned *)((a)+(s)-0x04)) = (_ADPCM_MARK_STOP2); \
	FlushDcache (); }
#else
#define _AdpcmSetMarkSTOP(a,s) { \
  *((unsigned *)((a)+(s)-0x10)) = (_ADPCM_MARK_STOP1); \
  *((unsigned *)((a)+(s)-0x0C)) = (_ADPCM_MARK_STOP2); \
  *((unsigned *)((a)+(s)-0x08)) = (_ADPCM_MARK_STOP2); \
  *((unsigned *)((a)+(s)-0x04)) = (_ADPCM_MARK_STOP2); \
	}

#endif
#endif
