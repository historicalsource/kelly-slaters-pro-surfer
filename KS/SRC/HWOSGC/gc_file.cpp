#include "global.h"
#include "osdevopts.h"
//for gc_states
#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/pad.h>
#include <dolphin/vi.h>
#include "ngl_gc.h"
//for gc_states

char os_file::root_dir[MAX_DIR_LEN];
char os_file::pre_root_dir[MAX_DIR_LEN];
bool os_file::system_locked = false;

extern stringx locale_dir;

os_file::os_file( )
{
  flags = 0;
  opened = false;
  from_cd = false;
  host_file= INVALID_HOST_SYSTEM_FILE_HANDLE;
}

os_file::os_file( const stringx& _name, int _flags )
{
  flags = 0;
  opened = false;
  from_cd = false;
  host_file= INVALID_HOST_SYSTEM_FILE_HANDLE;
  open( _name, _flags );
}

os_file::~os_file( )
{

  if( is_open( ) ) {
    close( );
  }

}

static inline void check_system_locked( const stringx& name )
{

  if( os_file::is_system_locked( ) &&
  		!os_developer_options::inst()->is_flagged( os_developer_options::FLAG_MOVE_EDITOR ) )
	{
    warning( name + ": os_file system is locked; no file access allowed" );
  }

}

static os_file* files_used_log;
static const char* FILES_USED_LOG_NAME = "files_used_log_gc.txt";

bool os_file::try_to_open_from_cd( void )
{
	return try_to_open_from_disk( );
}

bool os_file::try_to_open_from_disk( void )
{
	if( flags == FILE_WRITE )
	{
		host_file= host_fopen(  name.c_str(), (host_fopen_flags_t)HOST_WRITE );
		opened=  host_file != INVALID_HOST_SYSTEM_FILE_HANDLE ;
		from_cd = false;
    return(opened);
	}

  from_cd = true;
  
	char nintendo_sucks[256];
	strcpy( nintendo_sucks, name.c_str( ) );
	int l = strlen( nintendo_sucks );

	if ( l < 1 )
	{
		opened = false;
		return false;
	}

	// fix-up
	for( int i = 0; i < l; i++ )
	{
		if( nintendo_sucks[i] == '\\' )
		{
			nintendo_sucks[i] = '/';
		}

	}

	s32 fast = -1;

	fast = DVDConvertPathToEntrynum( nintendo_sucks );

	if( fast < 0 || fast > 0x00100000)
	{
		opened = false;
		return false;
	}

	bool b = DVDFastOpen( fast, &io );

  if( !b )
	{
    opened = false;
  }
	else
	{
    opened = true;
    offset = 0;
  }

  return opened;
}

void create_stash_file_list_entry( stringx name )
{
  files_used_log->open( FILES_USED_LOG_NAME, os_file::FILE_APPEND );
  name.to_lower( );
  stringx sfl_name( name );

  if( files_used_log->is_open( ) ) {
    files_used_log->set_fp( 0, os_file::FP_END );
    files_used_log->write( (void*) sfl_name.c_str( ), sfl_name.length( ) );
    files_used_log->write( (void*) "\r\n", 2 );
    files_used_log->close( );
  }

}

void os_file::open( const stringx& _name, int _flags )
{
  bool ret = false;
  assert( !opened );

  check_system_locked( _name );

	#ifdef EVAN
	char damnopaquestringclass[64];
	strcpy(damnopaquestringclass,_name.c_str());
	#endif

  name = _name.c_str();

  flags = _flags;

  ret = try_to_open_from_cd( );
}

void os_file::set_root_dir( const stringx& dir )
{
  assert( dir.size( ) < MAX_DIR_LEN );
  strcpy( root_dir, dir.c_str( ) );
}

void os_file::set_pre_root_dir( const stringx& dir )
{
  assert( dir.size( ) < MAX_DIR_LEN );
  strcpy( pre_root_dir, dir.c_str( ) );
}

void os_file::close( void )
{
  assert( opened );
  if( !from_cd )
  {
  	host_fclose( host_file );
  	return;
  }

	DVDClose( &io );

  opened = false;
  offset = 0;
}

extern void system_idle( void );

static char extra[32] __attribute__((aligned (32)));

#define ALIGNED(x,n) (((x)%(n))==0)

int os_file::read( void* data, int bytes, bool async )
{
  assert( opened );
  assert( data != NULL );
  assert( bytes > 0 );
  
  if( !from_cd )
  {
    return host_read( host_file, data, bytes );
  }
  
  int ret = 0;
  int limit = OSRoundUp32B( DVDGetLength( &io ) );
  
  if( ( offset + bytes ) > limit ) {
  	bytes = ( limit - offset );
  }
  
  // We need to pre-read to the extra structure if
  // 'data' isn't 32 bytes aligned. We are guaranteed
  // that it will be 4-byte aligned -mjd
  
  int align_adj = 32 - ( (u32) data & 0x1f );
 	s32 status = 0;
  
  if( align_adj != 32 )
  {
    if( align_adj > bytes )
        align_adj = bytes;
    
    if( !ALIGNED( align_adj, 4 ) )
    {
        assert( 0 );
        return -1;
    }
    
   	DVDReadAsync( &io, extra, 32, offset, NULL );

		do {
			status = DVDGetFileInfoStatus( &io );

			switch( status ) {
			case DVD_STATE_COVER_OPEN:
			case DVD_STATE_NO_DISK:
			case DVD_STATE_WRONG_DISK:
			case DVD_STATE_RETRY:
			case DVD_STATE_FATAL_ERROR:
				system_idle( );
				break;
			}

		} while( status != DVD_STATE_END );
    
    memcpy( data, extra, align_adj );
    
    ret = align_adj;
    offset += align_adj;
    bytes -= align_adj;
    data = (void *)( (char *)data + align_adj );
    
  }
  
  int neu_bytes = OSRoundDown32B( bytes );

	// if we have to read more then 32 bytes, read it in
	//
	// this protects us against attempting to read 0 bytes
	// from the file when we round down.
	if( neu_bytes > 0 ) {

		if( !ALIGNED( offset, 4 ) ) {
			assert( 0 );
			return -1;
		}

	  DVDReadAsync( &io, data, neu_bytes, offset, NULL );

		do {
			status = DVDGetFileInfoStatus( &io );

			switch( status ) {
			case DVD_STATE_COVER_OPEN:
			case DVD_STATE_NO_DISK:
			case DVD_STATE_WRONG_DISK:
			case DVD_STATE_RETRY:
			case DVD_STATE_FATAL_ERROR:
				system_idle( );
				break;
			}

		} while( status != DVD_STATE_END );

		ret += neu_bytes;
		offset += neu_bytes;
	}

  int remaining_bytes = bytes - neu_bytes;

	if( remaining_bytes <= 0 ) {
		return ret;
	}

	// second pass to pick up the leftovers
	if( !ALIGNED( offset, 4 ) ) {
		assert( 0 );
		return -1;
	}
	
	DVDReadAsync( &io, extra, 32, offset, NULL );

	do {
		status = DVDGetFileInfoStatus( &io );

		switch( status ) {
		case DVD_STATE_COVER_OPEN:
		case DVD_STATE_NO_DISK:
		case DVD_STATE_WRONG_DISK:
		case DVD_STATE_RETRY:
		case DVD_STATE_FATAL_ERROR:
			system_idle( );
			break;
		}

	} while( status != DVD_STATE_END );

	unsigned char* p = (unsigned char*) data + neu_bytes;
	memcpy( p, extra, remaining_bytes );
	ret += remaining_bytes;
	offset += remaining_bytes;

	return ret;
}

int os_file::write( void* data, int bytes )
{
  // returns number of bytes written.
  assert( opened );
  assert( data );
  assert( bytes > 0 );
	assert( !from_cd );
  
  return host_write( host_file, data, bytes);
}

int os_file::get_size( void )
{

  if( !opened ) {
    return -1;
  }

	if( !from_cd )
		return  host_get_size( host_file );

	int ret = DVDGetLength( &io );

  return ret;
}

void os_file::set_fp( int pos, filepos_t base )
{
  if( !from_cd )
  {
    host_fseek( host_file, pos, (host_seek_mode_t )base );
    return;
  }
  
	switch( base ) {
	case FP_BEGIN:
		offset = pos;
		break;
	case FP_CURRENT:
		error( "os_file::set_fp( ..., FP_CURRENT ) is unsupported on this platform" );
		break;
	case FP_END:
		offset = DVDGetLength( &io ) - pos;
		break;
	default:
		assert( 0 );
		break;
	}

}

unsigned int os_file::get_fp( void )
{
  if( !from_cd )
    return ftell( host_file );
  
  return offset;
}

bool os_file::file_exists( const stringx& _name )
{
  os_file foo;
  int ret = false;

  foo.name = _name;

    // fix-up
	for( int i = 0; i < foo.name.size( ); i++ ) {

		if( foo.name[i] == '\\' ) {
			foo.name[i] = '/';
		}

	}

  foo.flags = FILE_READ;

  ret = foo.try_to_open_from_cd( );

  if( ret )
  {
    foo.close( );
    return true;
  }
  
  return false;
}

bool os_file::directory_exists( const stringx& name )
{
  return false;
}

// returns true if file1 is newer than file2.
bool os_file::is_file_newer( const stringx& file1, const stringx& file2 )
{
  return false;
}

bool os_file::at_eof( void )
{
	if ( !is_open() ) return true;
	int limit = DVDGetLength( &io );

	return ( offset >= limit );
}

host_system_file_handle host_fopen( const char* fname, host_fopen_flags_t flags )
{
  char mode[4];
  int idx = 0;

	memset( mode, 0, sizeof( mode ) );
  
  switch( flags&0x3 )
  {
    case HOST_READ:
      mode[idx++] = 'r';
      break;

    case HOST_WRITE:
      mode[idx++] = 'w';
      break;

    case HOST_APPEND:
      mode[idx++] = 'a';
      break;

    case HOST_RWA_MASK:
      mode[idx++] = 'w';
      mode[idx++] = '+';
      break;
  }

	mode[idx++] = 'b';
	mode[idx] = '\0';

  return fopen( fname, mode );
}

void host_fclose( host_system_file_handle fp )
{
  assert( fp != NULL );

  fclose( fp );
}

void host_fprintf( host_system_file_handle fp, const char* fmt, ... )
{
	// empty?
}

int host_read( host_system_file_handle fp, void* _buf, int len )
{
  assert( fp != NULL );

  return fread( _buf, 1, len, fp );
}

int host_write( host_system_file_handle fp, const void* _buf, int len )
{
  assert( fp != NULL );

  return fwrite( _buf, 1, len, fp );
}

int host_fseek( host_system_file_handle fp, int offset, host_seek_mode_t _mode )
{
  assert( fp != NULL );
  return fseek( fp, offset, _mode );
}

int host_get_size( host_system_file_handle fp )
{
  int offset = ftell( fp );
  int size;
  fseek( fp, 0, SEEK_END );
  
  size = ftell( fp );
  fseek( fp, SEEK_SET, offset );

  return size;
}


