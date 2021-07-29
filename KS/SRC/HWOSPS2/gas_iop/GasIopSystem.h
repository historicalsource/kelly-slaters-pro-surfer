/*=======================================================================================*
 * Groovin' Audio System (GAS) for the Playstation2
 *---------------------------------------------------------------------------------------*
 * Author - Greg Taylor (2/5/01)
 *---------------------------------------------------------------------------------------*
 * An audio library for the PS2 which is primarily a stand-alone IOP program.  This 
 * program (like many of the PS2 IRX libs from Sony) is loaded onto the IOP and responds
 * to RPC calls from the main program (running on the EE).
 *=======================================================================================*/
#ifndef GAS_IOP_SYSTEM_HEADER
#define GAS_IOP_SYSTEM_HEADER

#include "iop_fifo_queue.h" 
#include "../gas_iop.h"
#include <libsd.h>

/*** constants ***/
#define PS2_MAX_VOICES_PER_CORE 24
#define PS2_MAX_VOICES (PS2_MAX_VOICES_PER_CORE * 2)

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
      int fd; // host disk (debug only) - not fully supported yet
    } cd;
    struct { // ee memory (not yet implemented)
      int foo;
    } ee;   
    struct { // iop memory
      unsigned char *iop_buf_addr;
    } iop;
    struct { // resident in spu memory (no streaming, no stereo)
      unsigned addr;
    } spu;
  } src; 

  struct {
    unsigned char stereo:1;
    unsigned char loop:1;
    unsigned char loaded:1;
    unsigned char src_type:2;
    unsigned char host_disk:1;
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
      int fd;
    } cd;
    struct { // ee memory
      int foo;
    } ee;       
    struct { // iop memory
      int foo;
    } iop;   
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


/*** GasIopSystemState ***/
typedef struct
{
  // iop stuff
  unsigned char * iop_buffers_top;  // constant
  unsigned char * iop_cd_mono_buffers_top;  // constant
  short iop_buffer_num;  // constant

  // spu stuff
  short spu_buffer_num;  // constant
  int spu_heap_start;    // constant
  int spu_heap_end;      // constant
  int spu_heap_curr;

  // queues
  fifo_queue free_voices;
  fifo_queue free_instances;
  fifo_queue free_cd_stereo_bufs;
  fifo_queue free_cd_mono_bufs;
  fifo_queue free_spu_bufs;
  fifo_queue reload_spu;
  fifo_queue reload_cd_streams;
  fifo_queue cd_preloads_pending;

  // instance list
  GasInstance inst_list[GAS_MAX_INSTANCES];
  short inst_list_count;

  // source list
  short source_list_count;
  GasSource *source_list;

  // command stuff
  GasCommandEntry command_list[GAS_MAX_INSTANCES];
  char  commands_waiting;
  char  proview_mode;

  // misc
  short preloading_instance;
  char *host_prefix;

} GasSystemState;


/*** constants ***/
#define BASE_priority  45
 

// this value can be tweaked to reduce audio pops (lower values == less pops, in theory, but
// try to keep it at the highest value without pops for performance reasons)
#define GAS_SAFE_VOLUME_DELTA     0x0100

#define RELOAD_FLAG_SIDE2         2
#define RELOAD_FLAG_SIDE1         1
#define RELOAD_FLAG_SIDE0         0
#define RELOAD_FLAG_NO_RELOAD    -1
#define RELOAD_FLAG_RECYCLE_ME   -2
#define RELOAD_FLAG_DEAD         -3

#define VOICE(x) (1 << (x))
#define VNUM_NO_VOICE 31

#define SRC_TYPE_SPU  0x00
#define SRC_TYPE_EE   0x01
#define SRC_TYPE_IOP  0x02
#define SRC_TYPE_CD   0x03

#define VOLUME_LR(l, r) ((l) << 16 | (r))

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

// for dma to spu
#define UPLOAD_SIZE 0x40000

// in GasIopSystem.c
extern GasSystemState gas;

// in GasIopCommand.c
int _gas_play_thread (int status);
int _gas_dma_int (void* common);
SD_IRQ_CBProc _gas_spu_int (void);

// in GasIopSystem.c
void  gas_init (int spu_buffers, char *list_filename, int file_size, int using_proview);
void  gas_shutdown ();
int   gas_reset (char *list_filename, int file_size);
void  gas_remove_instances();

void _gas_remove_iop_source(int which);
void _gas_partial_shutdown();
void _gas_init_system_data();
void  gas_spu_mem_dump();
void _gas_spu_mem_clear();

// in GasIopDecode.c
int    gas_add_source(char *filename);
int    gas_remove_source(int source_id);
int    gas_add_instance(int source_id);
int    gas_play_instance(int instance_id, unsigned short volume_left, unsigned short volume_right);
int    gas_pause_instance(int instance_id, int pause_mode);
int    gas_instance_volume(int instance_id, int volume_left, int volume_right);
int    gas_instance_pitch(int instance_id, int pitch);
int    gas_status_is_playing(int instance_id);
int    gas_status_is_ready(int instance_id);
int    gas_command_list(void *data);
void   gas_voice_status_dump();

int   _gas_load_list_file(char *filename, int file_size, GasSource **file_list, short *file_list_count);
void  _gas_update_volume(int inst);
void  _gas_set_voice (int inst, int side, unsigned int spu_addr);
void  _gas_set_voice_volume (int core, int voice, int voll, int volr);
void  _gas_set_voice_pitch (int core, int voice, int pitch);
char* _gas_load_file(char *orig_filename, int *size);
int   _gas_reclaim_voices();
int   _gas_dma_iop_to_spu_blocking(unsigned char *iop_addr, unsigned char *spu_addr, int size);

// in GasIopSourceSpu.c
void _gas_play_spu_instance(int inst);
int  _gas_pause_spu_instance(int inst, int pause_mode);
int  _gas_add_spu_source(int entry);

// in GasIopSourceIop.c
void _gas_play_iop_instance(int inst);
int  _gas_add_iop_source(int entry);
int  _gas_pause_iop_instance(int inst, int pause_mode);
int  _gas_init_iop_instance(int inst, int source);
int  _gas_init_spu_streaming(int inst, int source);
void _gas_streamed_voice_controller(int inst, int mode, int store_pos_flag);

// in GasIopSourceCd.c
void _gas_play_cd_instance(int inst);
int  _gas_add_cd_source(int entry);
int  _gas_pause_cd_instance(int inst, int pause_mode);
int  _gas_init_cd_instance(int inst, int source);
void _gas_process_cd_preloads();

// externs
extern GasIopStatus gas_status;  // EE

/*** macros ***/
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

#define gas_printf printf

#endif