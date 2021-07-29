#ifndef XB_FILE_H
#define XB_FILE_H
/*-------------------------------------------------------------------------------------------------------
  XB_FILE.H - xb implementation of basic binary os_file i/o adapter class.
-------------------------------------------------------------------------------------------------------*/
#include "stringx.h"

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif /* INVALID_HANDLE_VALUE JIV DEBUG */

class os_file
{
  public:
    enum mode_flags
    {
      FILE_READ   = 1,
      FILE_WRITE  = 2,
      FILE_MODIFY = 3,    // opens the file for writing, w/o deleting existing contents. (how to set new EOF? huh? --Sean)
      FILE_APPEND = 4
    };

    // ctors & dtor.
    os_file();
    os_file(const stringx & _name, int _flags);
    ~os_file();

    void open(const stringx & _name, int _flags);
    void close();

    // read/write return number of bytes read/written.
    int read(void * data, int bytes, bool async = false);
    int write(void * data, int bytes);

    // returns file size
    int get_size(); 

    enum filepos_t
    {
      FP_BEGIN,
      FP_CURRENT,
      FP_END
    };
    // set file pointer 
    void set_fp( int pos, filepos_t base );
    unsigned int get_fp(); // relative to beginning

    // state queries
    inline const stringx & get_name() const { return name; }
    inline bool is_open() const { return opened; }           // returns true after a successful open call.
    inline bool at_eof() const;               // check this after a read operation.

    // once this is set, all os_file open operations that specify use_root are opened relative to this directory.
    static void set_root_dir(const stringx & dir);
    static const char* get_root_dir() { return root_dir; }

    // By default,the dir above root (e.g. \die2 to \die2\data)
    static void set_pre_root_dir(const stringx & dir);
    static const char* get_pre_root_dir() { return pre_root_dir; }

    // file system queries
    static bool file_exists(const stringx& name);
    static bool directory_exists(const stringx& name);

    // returns true if file1 is newer than file2.
    static bool is_file_newer(const stringx& file1, const stringx& file2);

    bool operator!() const { return false; }

    static void system_lock() { system_locked = true; }
    static void system_unlock() { system_locked = false; }
    static bool is_system_locked() { return system_locked; }

    bool is_from_cd() { return from_cd; }

    enum {MAX_DIR_LEN=256};
    
  private:
    // common to all os_file implementations
    stringx name;
    int   flags;
    bool  opened;
    bool  from_cd;


    static char root_dir[MAX_DIR_LEN];
    static char pre_root_dir[MAX_DIR_LEN];

    static bool system_locked;
    
    // implementation stuff
    FILE *io;
    bool check_for_errors(int sce_error_code, char *_activity);
//    bool try_to_open_from_cd();
    bool try_to_open_from_disk(char drive_letter = 'D');

    friend class movieplayer;
};            

// iop module loader
bool load_iop_module(const char *module_name, const char *debug_path);

// for debugging via file on the host system
typedef os_file* host_system_file_handle;
os_file* const INVALID_HOST_SYSTEM_FILE_HANDLE = NULL;

enum host_fopen_flags_t
  {
  HOST_RWA_MASK = 0x0003,
  HOST_READ     = 0x0000,
  HOST_WRITE    = 0x0001,
  HOST_APPEND   = 0x0002,

  HOST_TB_MASK  = 0x0004,
  HOST_TEXT     = 0x0000,
  HOST_BINARY   = 0x0004,
  };

host_system_file_handle host_fopen( const char* fname, host_fopen_flags_t flags );
void                    host_fclose( host_system_file_handle fp );
void                    host_fprintf( host_system_file_handle fp, const char* fmt, ... );
int                     host_read( host_system_file_handle fp, char* buf, int len );

#endif
