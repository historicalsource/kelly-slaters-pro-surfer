/*-------------------------------------------------------------------------------------------------------
  PS2_FILE.CPP 
-------------------------------------------------------------------------------------------------------*/
#ifdef ARCH_ENGINE
#include "global.h"
#include "osdevopts.h"
#endif
//#pragma hdrstop

#include "ps2_file.h"
//#include "profiler.h"
#include <sifdev.h>
#include <libcdvd.h>
#include "ngl_ps2.h"

/*-------------------------------------------------------------------------------------------------------
  os_file implementation 
-------------------------------------------------------------------------------------------------------*/
char os_file::root_dir[MAX_DIR_LEN];
char os_file::pre_root_dir[MAX_DIR_LEN];
bool os_file::system_locked = false;

extern stringx locale_dir;

#if defined(BUILD_BOOTABLE)
#define MAX_DISK_OP_TRIES 300
#elif defined (BUILD_FINAL)
#define MAX_DISK_OP_TRIES 300
#else
#define MAX_DISK_OP_TRIES 10
#endif

static bool g_error_condition = false;
static int g_try_count = 0;

static sceCdRMode g_sce_rmode = { /*try_count*/0, /*spindlctrl*/ SCECdSpinNom, /*datapattern*/ SCECdSecS2048 };
char os_file::sector_buffer[PS2_CD_SECTOR_SIZE] __attribute__((aligned(64)));;


#define RETRY_LOOP_ENTRY g_try_count = 0; do{
#define RETRY_LOOP_WHILE(cond) g_error_condition = ((cond)); g_try_count++; }while( ( g_try_count < MAX_DISK_OP_TRIES ) && g_error_condition );

#define RETRY_LOOP_WHILE_INF(cond) g_error_condition = ((cond)); g_try_count++; }while( g_error_condition );


#define ON_RETRY_ERROR if( g_error_condition )


//static const stringx host0 = "host0:";

/*
extern profiler_timer proftimer_file_exists;
extern profiler_timer proftimer_file_open;
extern profiler_timer proftimer_file_read;
*/


/*** constructors ***/
os_file::os_file()
{
  flags=0;
  opened=false;
  eof=true;
  from_cd = false;
  curr_fp = -1;
}

os_file::os_file(const stringx & _name, int _flags)
{
  flags=0;
  opened=false;
  eof=true;
  from_cd = false;
  curr_fp = -1;
  open(_name,_flags);
}


/*** destructor ***/
os_file::~os_file()
{
  if (is_open()) 
    close();
}


/*** check_system_locked ***/
static inline void check_system_locked( const stringx& name )
{
  if ( os_file::is_system_locked()  && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_MOVE_EDITOR))
    warning( name + ": os_file system is locked; no file access allowed" );
}

bool os_file::try_to_open_from_cd()
{
  char filename[256];
  char cd_filename[256];
  int ret = 0;
  opened = false;

  filespec spec(name);
  if (spec.name.length() > 8 || spec.ext.length() > 4)
  {
//    if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
//      nglPrintf("File %s is not in 8.3 format\n", name.c_str());
    return false;
  }

  // check the filename is within reasonable limits with cdrom0:\ and ;1 added
  assert(name.length() < 245);

  strcpy(filename, name.c_str());
  strupr(filename);
  if (name != "GAME.INI" && os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
    nglPrintf("CD Search Filename: '%s'\n", filename);


  RETRY_LOOP_ENTRY;
    int ready_ret;
    ready_ret = sceCdDiskReady(0);
  RETRY_LOOP_WHILE( ready_ret == SCECdNotReady );
  
  ON_RETRY_ERROR return false;

  strcpy(cd_filename, "\\");
  strcat(cd_filename, filename);
  strcat(cd_filename, ";1");
  RETRY_LOOP_ENTRY;
    ret = sceCdSearchFile( &io.cd, cd_filename );
  RETRY_LOOP_WHILE( ret == 0 );

  ON_RETRY_ERROR return false;

  opened = true;
  eof = false;
  from_cd = true;
  curr_fp = 0;

  return true;
}

bool os_file::try_to_open_from_disk()
{
  // read from host disk

  // until we implement a DVD/CD reader, we'll get all of our data from the host
  if (strncmp(name.c_str(), nglHostPrefix, strlen(nglHostPrefix)) != 0)
    name=nglHostPrefix+name;

//  nglPrintf("opening file %s on the host disk\n", name.c_str());
  int mode=-1;

  // convert the flags to Sony's fileio flags 
  switch (flags)
  {
    case FILE_READ:   mode=SCE_RDONLY;              break;
    case FILE_WRITE:  mode=SCE_WRONLY | SCE_CREAT | SCE_TRUNC;  break;
    case FILE_APPEND: mode=SCE_WRONLY | SCE_CREAT | SCE_APPEND; break;
    case FILE_MODIFY: mode=SCE_RDWR;                break;
  }
  assert(mode!=-1);

  // attempt to open the file
  RETRY_LOOP_ENTRY;
    io.host = sceOpen((char *)name.c_str(), mode);
  RETRY_LOOP_WHILE( io.host < 0 );  

  ON_RETRY_ERROR return false;

  opened = true;
  eof = false;
  from_cd = false;
  curr_fp = 0;

  return opened;
}

void create_stash_file_list_entry(stringx name);

/*** open ***/
void os_file::open(const stringx & _name, int _flags)
{
  bool ret = false;
  assert(!opened);

  check_system_locked( _name );

  name = _name;
  flags = _flags;

  if (( _flags & FILE_READ) &&
    (_name == "GAME.INI" ||	// next line will crash unless game.ini already loaded (dc 01/07/02)
    !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_CD)))
  {
    ret = try_to_open_from_cd();
    if( ret ) return;
  }

  if (ret == false)
  {
    ret = try_to_open_from_disk();
  }

  if (ret == false)
  {
//    let's fail silently
//    nglPrintf("Error.  Could not open file %s on either the cd or the host disk.\n", name.c_str());
    opened = false;
  }
}

void os_file::set_root_dir(const stringx & dir)
{
  assert( dir.size()<MAX_DIR_LEN );
  strcpy( root_dir, dir.c_str() );
}

void os_file::set_pre_root_dir(const stringx & dir)
{
  assert( dir.size()<MAX_DIR_LEN );
  strcpy( pre_root_dir, dir.c_str() );
}

/*** check_for_errors ***/
bool os_file::check_for_errors(int sce_error_code, char *_activity)
{
  // returns true if there were any errors
  bool ret = true;
  stringx activity = _activity;

  switch ( sce_error_code )
  {
    case SCE_ENXIO: // No such device or address
    error("No such device or address, cannot "+activity+" file "+name);
    break;

    case SCE_EBADF:	// Bad file number
    error("bad file number, cannot "+activity+" file "+name);
    break;

    case SCE_ENODEV: // No such device
    error("No such device, cannot "+activity+" file "+name);
    break;

    case SCE_EINVAL: // Invalid argument
    error("Invalid argument, cannot "+activity+" file "+name);
    break;

    case SCE_EMFILE:	// Too many open files
    error("Too many open files, cannot "+activity+" file "+name);
    break;

    case SCE_EBINDMISS:
    error("PS2 file error 'EBINDMISS' whatever that means, cannot "+activity+" file "+name);
    break;

    case SCE_ECALLMISS:
    error("PS2 file error 'ECALLMISS' whatever that means, cannot "+activity+" file "+name);
    break;

    case SCE_ETYPEMISS:
    error("PS2 file error 'ETYPEMISS' whatever that means, cannot "+activity+" file "+name);
    break;

    case SCE_ELOADMISS:
    error("PS2 file error 'ELOADMISS' whatever that means, cannot "+activity+" file "+name);
    break;

    default: // everything went ok
    ret = false;
    break;
  }

  return ret;
}

// note: these functions are heavily protected since they are used by classes with complicated logic.

/*** close ***/
void os_file::close()
{
  if (!opened)
  {
	error("Attempt to close file %s, which was not successfully opened.\n",name.c_str());
  }

  assert(opened);
	int ret;

  if( !from_cd )
  {
    RETRY_LOOP_ENTRY;
      ret = sceClose(io.host);
    RETRY_LOOP_WHILE( ret < 0 );
  }

  opened=false;
  eof=true;
  from_cd=false;
  curr_fp = -1;
}


/*** read ***/
int os_file::read(void * data, int bytes, bool async)
{
  int ret = -1;
  static bool prev_async = false;
  int err = SCECdErNO;
//  char errortxt[10];
//  proftimer_file_read.start();
  int retryCount = -1;
  static int last_current_sector =0, last_read_amt;
  static char *last_read_loc;
  if (!opened)
  {
	error("Attempt to read from missing file %s\n",name.c_str());
  }

  // returns number of bytes read.
  assert(opened);
  assert(flags&FILE_READ);
  assert(data);
  assert(bytes>0);
  
  if (!eof)
  {
    if( !from_cd )
    {
      RETRY_LOOP_ENTRY;
      retryCount++;
        ret = sceRead(io.host, data, bytes );
      RETRY_LOOP_WHILE( ret < 0 );

      ON_RETRY_ERROR { eof = true; return -1; };
      curr_fp += ret;

      if( ret < bytes )
        eof = true;
    }
    else
    {
      if( prev_async )
      {
        // patch up the last read
        sceCdSync( 0 );
        err = sceCdGetError();
        while(err  != SCECdErNO)
        {
          
          RETRY_LOOP_ENTRY;
            retryCount++;
            ret = sceCdRead( last_current_sector, last_read_amt, last_read_loc, &g_sce_rmode );
          RETRY_LOOP_WHILE_INF( ret <= 0 );

          ON_RETRY_ERROR { eof = true; return -1; };
          sceCdSync( 0 );
          err = sceCdGetError();
          
        }

      }
      char *_data = (char *)data;
      /* initial pad */
      int bytes_read = 0;
      int start_pad_extra = curr_fp % PS2_CD_SECTOR_SIZE;
      int start_pad_bytes = PS2_CD_SECTOR_SIZE - start_pad_extra;
      start_pad_bytes = ( ( bytes < start_pad_bytes ) ? bytes : start_pad_bytes );
      int current_sector = io.cd.lsn + ( curr_fp / PS2_CD_SECTOR_SIZE );
      if( start_pad_extra != 0 )
      {

        do 
        {
          RETRY_LOOP_ENTRY;
          retryCount++;
            ret = sceCdRead( current_sector, 1, sector_buffer, &g_sce_rmode );
          RETRY_LOOP_WHILE_INF( ret <= 0 );

          ON_RETRY_ERROR { eof = true; return -1; };
          sceCdSync( 0 );
            //RotateThreadReadyQueue( 0 );
          err = sceCdGetError();
          //sprintf(errortxt, "%d", err);
          //assert(err == SCECdErNO);
        } while (err != SCECdErNO);
        
        memcpy( _data, sector_buffer + start_pad_extra, start_pad_bytes );
        _data += start_pad_bytes;
        curr_fp += start_pad_bytes;
        bytes -= start_pad_bytes;
        bytes_read += start_pad_bytes;
        current_sector++;
      }


      int left = io.cd.size - curr_fp;
      int actual_bytes = ( ( bytes < left ) ? bytes : left );
      int extra_bytes = actual_bytes % PS2_CD_SECTOR_SIZE;
      int sectors = actual_bytes / PS2_CD_SECTOR_SIZE;

      if( sectors > 0 )
      {
        do 
        {
          last_current_sector = current_sector;
          last_read_loc = _data;
          last_read_amt = sectors;
          
          RETRY_LOOP_ENTRY;
            retryCount++;
            ret = sceCdRead( current_sector, sectors, _data, &g_sce_rmode );
          RETRY_LOOP_WHILE_INF( ret <= 0 );

          ON_RETRY_ERROR { eof = true; return -1; };

        } while (err != SCECdErNO);
        
        int full_sec_bytes = sectors * PS2_CD_SECTOR_SIZE;
        _data += full_sec_bytes;
        current_sector += sectors;
        // WE DON'T SYNC, SINCE WE WANT TO TRY TO ALLOW THIS TO BE ASYNC

      }

      if( extra_bytes > 0 )
      {
        // WELL, FINISH UP THE LAST BIT

        sceCdSync( 0 );
        err = sceCdGetError();
        while(err  != SCECdErNO)
        {
          
          RETRY_LOOP_ENTRY;
            retryCount++;
            ret = sceCdRead( last_current_sector, last_read_amt, last_read_loc, &g_sce_rmode );
          RETRY_LOOP_WHILE_INF( ret <= 0 );

          ON_RETRY_ERROR { eof = true; return -1; };
          sceCdSync( 0 );
          err = sceCdGetError();
        } while (err != SCECdErNO);

        // DO THE EXTRA BYTES
        retryCount = -1;
        
        do 
        {
          RETRY_LOOP_ENTRY;
          retryCount++;
            ret = sceCdRead( current_sector, 1, sector_buffer, &g_sce_rmode );
          RETRY_LOOP_WHILE_INF( ret <= 0 );

          ON_RETRY_ERROR { eof = true; return -1; };
          sceCdSync( 0 );
           //tateThreadReadyQueue( 0 );
          

          err = sceCdGetError();
          //sprintf(errortxt, "%d", err);
          //assert(err == SCECdErNO);
        } while (err != SCECdErNO);

        memcpy( _data, sector_buffer, extra_bytes );
        
        
      }
      
      curr_fp += actual_bytes;
      bytes_read += actual_bytes;

      if( bytes_read < bytes )
        eof = true;

      ret = bytes_read;

      if( !async )
      {
        int thisret = 0;
        sceCdSync( 0 );
          //RotateThreadReadyQueue( 0 );
        // If there is an error.. it would have to be in the 
        // section before if (extra_bytes)
        
        err = sceCdGetError();
        while(err  != SCECdErNO)
        {
          
          RETRY_LOOP_ENTRY;
            retryCount++;
            thisret = sceCdRead( last_current_sector, last_read_amt, last_read_loc, &g_sce_rmode );
          RETRY_LOOP_WHILE_INF( thisret <= 0 );

          ON_RETRY_ERROR { eof = true; return -1; };
          sceCdSync( 0);
          err = sceCdGetError();
        } while (err != SCECdErNO);

      }
    }
  }
  prev_async = async;
//  proftimer_file_read.stop();
  return ret;
}


int os_file::write(void * data, int bytes)
{
  // returns number of bytes written.
  assert(opened);
  assert((flags&FILE_WRITE) || (flags&FILE_APPEND));
  assert(data);
  assert( !( bytes < 0 ) );

  if( bytes == 0 ) return 0;

  int ret;

  RETRY_LOOP_ENTRY;
    ret = sceWrite(io.host, data, bytes);
  RETRY_LOOP_WHILE( ret < 0 );

  ON_RETRY_ERROR{ eof = true; return -1; };

  // check for end of file.
  eof=ret<bytes;

  curr_fp += ret;

  return ret;
}

int os_file::get_size()  // returns file size
{
  if (!opened) 
    return -1;

  int cur;
  int ret, ret2;

  if( from_cd )
  {
    return io.cd.size;
  }
  else
  {
    RETRY_LOOP_ENTRY;
      cur = sceLseek(io.host, 0, SCE_SEEK_CUR);
    RETRY_LOOP_WHILE( cur < 0 );

    ON_RETRY_ERROR{ eof = true; return -1; };

    RETRY_LOOP_ENTRY;
      ret = sceLseek(io.host, 0, SCE_SEEK_END);
    RETRY_LOOP_WHILE( ret < 0 );

    ON_RETRY_ERROR{ eof = true; return -1; };

    RETRY_LOOP_ENTRY;
      ret2 = sceLseek(io.host, cur, SCE_SEEK_SET);
    RETRY_LOOP_WHILE( ret2 < 0 );

    ON_RETRY_ERROR{ eof = true; return -1; };

    return ret;
  }
}


int os_file::set_fp( int pos, filepos_t base )
{
  int method = 0;
  int ret = -1;
  
  int size = get_size();

  switch( base )
  {
    case FP_BEGIN:
      curr_fp = pos;
      break;

    case FP_CURRENT:
      curr_fp += pos;
      break;

    case FP_END:
      curr_fp = size + pos;
      break;

    default:
      assert( 0 );
      break;
  }

  if( curr_fp > size ) curr_fp = size - 1;
  if( curr_fp < 0 ) curr_fp = 0;

  if( !from_cd )
  {
    switch ( base )
    {
      case FP_BEGIN:
        method = SCE_SEEK_SET;
        break;
      case FP_CURRENT:
        method = SCE_SEEK_CUR;
        break;
      case FP_END:
        method = SCE_SEEK_END;
        break;
    }
    RETRY_LOOP_ENTRY;
      ret = sceLseek(io.host, pos, method);
    RETRY_LOOP_WHILE( ret < 0 );

    ON_RETRY_ERROR{ eof = true; return -1; };
  }

  // could actually be true now, but chances are it will be false.
  eof = false;

  return ret;
}

unsigned int os_file::get_fp()
{
  return curr_fp;
}

bool os_file::file_exists(const stringx& _name)
{
//  proftimer_file_exists.start();
  os_file foo;
  int ret = false;

  foo.name = _name;
  foo.flags = FILE_READ;

  if (_name != "GAME.INI" && _name != "tricks.dat" && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_CD))
    ret = foo.try_to_open_from_cd();
  else if (_name == "GAME.INI" || _name == "tricks.dat")
    ret = foo.try_to_open_from_cd();
  if (ret == false)
    ret = foo.try_to_open_from_disk();

  if (ret)
  {
    foo.close();
    return true;
  }
  return false;

//  proftimer_file_exists.stop();
}


bool os_file::directory_exists(const stringx& name)
{
  assert("os_file::directory_exists not yet supported on PS2, it may never be.\n" == 0);
  return false;
}

// returns true if file1 is newer than file2.
bool os_file::is_file_newer(const stringx& file1, const stringx& file2)
{
  assert("os_file::is_file_newer is not yet supported on PS2, it may never be.\n" == 0);
  return false;
}

// for debugging via file on the host system
os_file host_system_file;

host_system_file_handle host_fopen( const char* fname, host_fopen_flags_t flags )
{
  // we only support one file at a time
  assert( ! host_system_file.is_open() );
//  return fopen( fname, flags );
  unsigned int snflags = 0;
  switch ( flags & HOST_RWA_MASK )
  {
    case HOST_READ:
      snflags |= os_file::FILE_READ;
      break;
    case HOST_WRITE:
      snflags |= os_file::FILE_WRITE;
      break;
    case HOST_APPEND:
      assert( 0 );
      break;
  }

  host_system_file.open( fname, snflags );
  return &host_system_file;
}

void host_fclose( host_system_file_handle fp )
{
  assert( fp->is_open() );
  assert( fp == &host_system_file );
  fp->close();
//  assert("host_fclose is not yet supported on PS2, it may never be.\n" == 0);
}

void host_fprintf( host_system_file_handle fp, const char* fmt, ... )
{
  assert( fp == &host_system_file );
  assert( fp->is_open() );
  // perform variable argument print
  char buf[1024];
  va_list args;
  va_start( args, fmt );
  vsprintf( buf, fmt, args );
  va_end( args );
  // send string to output file
  fp->write( buf, strlen(buf) );
}

int host_read( host_system_file_handle fp, void* buf, int len )
{
  assert( fp == &host_system_file );
  assert( fp->is_open() );

  return fp->read( buf, len );
}

int host_write( host_system_file_handle fp, void const * buf, int len )
{
  assert( fp == &host_system_file );
  assert( fp->is_open() );

  return fp->write( const_cast<void *>(buf), len );
}

int host_get_size( host_system_file_handle fp )
{
  assert( fp == &host_system_file );
  assert( fp->is_open() );
  
  return fp->get_size();
}

int host_fseek( host_system_file_handle fp, int offset, host_seek_mode_t mode )
{
  assert( fp == &host_system_file );
  assert( fp->is_open() );

  os_file::filepos_t os_mode;
  
  switch( mode )
  {
    case HOST_CUR:
      os_mode = os_file::FP_CURRENT;
      break;
    case HOST_BEGIN:
      os_mode = os_file::FP_BEGIN;
      break;
    case HOST_END:
      os_mode = os_file::FP_END;
      break;
    default:
      assert( "Uhh!? bad host_seek_mode" && 0 );
	  return -1;	// otherwise os_mode possibly uninitialized (dc 01/29/02)
  }

  return fp->set_fp( offset, os_mode );
}

bool load_iop_module(const char *module_name, const char *debug_path)
{
  int ret = -1;

  // try to grab the sio2man off of the CD first 
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_CD))
  {
    stringx cd_name(module_name);
    cd_name.to_upper();
    cd_name = stringx("cdrom0:\\") + cd_name;
    cd_name += ";1";
nglPrintf("Trying to open %s\n", cd_name.c_str());
    ret = sceSifLoadModule((char *)cd_name.c_str(), 0, NULL);
  }

  if (ret < 0)
  {
    stringx disk_name(nglHostPrefix);
    disk_name += module_name;
    ret = sceSifLoadModule((char *)disk_name.c_str(), 0, NULL);
  }

#if 0
  if (ret < 0 && os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
    return false;

  if (ret < 0)
  {
    stringx debug_name(debug_path);
    debug_name += module_name;
    ret = sceSifLoadModule((char *)debug_name.c_str(),0, NULL);
  }
#endif
  return (ret >= 0);
}



/*
 * sifdev.h reference stuff 
 *

// Flag for sceOpen()
#define SCE_RDONLY      0x0001
#define SCE_WRONLY      0x0002
#define SCE_RDWR        0x0003
#define SCE_NBLOCK      0x0010  // Non-Blocking I/O
#define SCE_APPEND      0x0100  // append (writes guaranteed at the end)
#define SCE_CREAT       0x0200  // open with file create
#define SCE_TRUNC       0x0400  // open with truncation
#define SCE_NOBUF       0x4000  // no device buffer and console interrupt
#define SCE_NOWAIT      0x8000  // asyncronous i/o

// SCE local usage
#define SCE_USING       0x10000000  // io descript usage bit
#define SCE_NOWBDC      0x20000000  // not write back d cashe

// Seek Code
#define SCE_SEEK_SET	0
#define SCE_SEEK_CUR	1
#define SCE_SEEK_END	2

// Ioctl Code 
#define SCE_FS_EXECUTING	0x1
#define SCE_PAD_ADDRESS		0x1

// Error codes
#define	SCE_ENXIO	6	// No such device or address
#define	SCE_EBADF	9	// Bad file number
#define	SCE_ENODEV	19	// No such device
#define	SCE_EINVAL	22	// Invalid argument
#define	SCE_EMFILE	24	// Too many open files
#define	SCE_EBINDMISS	0x10000
#define	SCE_ECALLMISS	0x10001
#define	SCE_ETYPEMISS	0x10002
#define	SCE_ELOADMISS	0x10003

extern int  sceOpen(char *filename, int flag);
extern int  sceClose(int fd);
extern int  sceRead(int fd, void *buf, int nbyte);
extern int  sceWrite(int fd, void *buf, int nbyte);
extern int  sceLseek(int fd, int offset, int whence);
extern int  sceIoctl(int fd, int req, void *);
extern int  sceFsReset(void);

#define SCE_STM_R	0x0001
#define SCE_STM_W	0x0002
#define SCE_STM_X	0x0004
#define SCE_STM_C	0x0008
#define SCE_STM_F	0x0010
#define SCE_STM_D	0x0020

extern int sceSifInitIopHeap(void);
extern void *sceSifAllocIopHeap(int);
extern int sceSifFreeIopHeap(void *);
extern int sceSifLoadIopHeap(char *, void *);

// ee load file routine
typedef struct {
	unsigned int epc;
	unsigned int gp;
	unsigned int sp;
	unsigned int dummy;  
} sceExecData;

#define SCE_SIF_TYPECHAR	0
#define SCE_SIF_TYPESHORT	1
#define SCE_SIF_TYPEINT		2

extern int sceSifLoadModule(char *filename, int args, char *argp);
extern int sceSifLoadElf(char *name, sceExecData *data);
extern int sceSifLoadElfPart(char *name, char *secname, sceExecData *data);
extern int sceSifLoadFileReset(void);

extern int sceSifRebootIop(char *img);
extern int sceSifSyncIop(void);

#define IOP_IMAGE_FILE "IOPRP16.IMG"
#define IOP_IMAGE_file "ioprp16.img"
*/