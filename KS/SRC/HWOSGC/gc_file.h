#ifndef GC_FILE_H
#define GC_FILE_H

#include <dolphin/dvd.h>

#include <fio.h>

#include "stringx.h"

#include "nsl.h"

// for debugging via file on the host system
typedef FILE* host_system_file_handle;
#define INVALID_HOST_SYSTEM_FILE_HANDLE NULL

enum host_fopen_flags_t {
  HOST_READ     = 0x0000,
  HOST_WRITE    = 0x0001,
  HOST_APPEND   = 0x0002,
  HOST_RWA_MASK = 0x0003,

  HOST_BINARY   = 0x0000,
  HOST_TEXT     = 0x0004,
  HOST_TB_MASK  = 0x0004,
};

enum host_seek_mode_t {
	HOST_BEGIN = 0x0000,
	HOST_CUR   = 0x0001,
	HOST_END   = 0x0002,
};

host_system_file_handle host_fopen( const char* fname, host_fopen_flags_t flags );
void host_fclose( host_system_file_handle fp );

int host_fseek( host_system_file_handle fp, int offset, host_seek_mode_t mode );

int host_read( host_system_file_handle fp, void* buf, int len );
int host_write( host_system_file_handle fp, const void* buf, int len );

void host_fprintf( host_system_file_handle fp, const char* fmt, ... );

int host_get_size( host_system_file_handle fp );




#define INVALID_HANDLE_VALUE -1

class os_file
{
public:
  enum mode_flags {
    FILE_READ   = 1,
    FILE_WRITE  = 2,
    FILE_MODIFY = 3,
    FILE_APPEND = 4
  };

  os_file( );
  os_file( const stringx& _name, int _flags );
  ~os_file( );

  void open( const stringx& _name, int _flags);
  void close( void );

  // read/write return number of bytes read/written.
  int read( void* data, int bytes, bool async = false  );
  int write( void* data, int bytes );

  int get_size( void );

  enum filepos_t {
    FP_BEGIN = 0,
    FP_CURRENT,
    FP_END
  };

  // set file pointer
  void set_fp( int pos, filepos_t base );
  unsigned int get_fp( void );

  // state queries
  inline const stringx& get_name( void ) const { return name; }
  inline bool is_open( void ) const { return opened; }
  bool at_eof( void );

  // once this is set, all os_file open operations that
  // specify use_root are opened relative to this directory.
  static void set_root_dir( const stringx& dir );
  static const char* get_root_dir( void ) { return root_dir; }

  // by default,the dir above root (e.g. /SM to /SM/data)
  static void set_pre_root_dir( const stringx& dir );
  static const char* get_pre_root_dir( void ) { return pre_root_dir; }

  // file system queries
  static bool file_exists( const stringx& name );
  static bool directory_exists( const stringx& name );

  // returns true if file1 is newer than file2.
  static bool is_file_newer( const stringx& file1, const stringx& file2 );

  bool operator!() const { return false; }

  static void system_lock( void ) { system_locked = true; }
  static void system_unlock( void ) { system_locked = false; }
  static bool is_system_locked( void ) { return system_locked; }

  bool is_from_cd( void ) { return from_cd; }

  enum {
  	MAX_DIR_LEN = 256
  };

private:
  // common to all os_file implementations
  stringx name;
  int flags;
  bool opened;
  bool from_cd;

  static char root_dir[MAX_DIR_LEN];
  static char pre_root_dir[MAX_DIR_LEN];

  static bool system_locked;

	// implementation-specific
  DVDFileInfo io;
  int offset;

  bool try_to_open_from_cd();
  bool try_to_open_from_disk();

  friend class movieplayer;
  FILE *host_file;
};

///////////////////////////////////////////////////
#define RESET_FADE_FRAMES_NUM 60

struct fade_volume
{
  float	 sfxVol;
	float  ambientVol;
  float  musicVol;
  float  voiceVol;
  float  movieVol;

  void get_volumes()
  {
    sfxVol=      nslGetVolume( NSL_SOURCETYPE_SFX );
    ambientVol=  nslGetVolume( NSL_SOURCETYPE_AMBIENT );
    musicVol=    nslGetVolume( NSL_SOURCETYPE_MUSIC );
    voiceVol=    nslGetVolume( NSL_SOURCETYPE_VOICE );
    movieVol=    nslGetVolume( NSL_SOURCETYPE_MOVIE );
  }

  void set_volumes(float i)
  {
    nslSetVolume( NSL_SOURCETYPE_SFX,     sfxVol * i );
    nslSetVolume( NSL_SOURCETYPE_AMBIENT, ambientVol * i );
    nslSetVolume( NSL_SOURCETYPE_MUSIC,   musicVol * i );
    nslSetVolume( NSL_SOURCETYPE_VOICE,   voiceVol * i );
    nslSetVolume( NSL_SOURCETYPE_MOVIE,   movieVol * i );
  }
};

class gc_states
{
  ///////reset///////
  static bool          last_reset_state;
  static fade_volume   reset_volume;
  static u32           reset;
public:
  gc_states(){};
  ~gc_states(){};
  static void show_message(char *msg);
  static void do_screen_fade();
  static void do_sound_fade();
  static void do_reset();
  static void poll_reset();
  static void poll_disc_states();
  static void poll_states();
};

#endif
