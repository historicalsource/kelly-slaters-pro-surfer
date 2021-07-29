/*-------------------------------------------------------------------------------------------------------
XB_FILE.CPP 
-------------------------------------------------------------------------------------------------------*/
// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef ARCH_ENGINE
#include "osdevopts.h"
#endif
//#pragma hdrstop

#include "xb_file.h"
#include "profiler.h"

#undef free
#undef malloc
#include "ngl.h"

#define INVALID_FP 0
#define PATH_MAX 1024 // ha ha ha

/*-------------------------------------------------------------------------------------------------------
os_file implementation 
-------------------------------------------------------------------------------------------------------*/
char os_file::root_dir[MAX_DIR_LEN];
char os_file::pre_root_dir[MAX_DIR_LEN];
bool os_file::system_locked = false;

extern stringx locale_dir;

static bool xb_file_warn_about_everything = false;

/*** constructors ***/
os_file::os_file()
{
	flags=0;
	opened=false;
	from_cd = false;
	io = INVALID_FP;
}

os_file::os_file(const stringx & _name, int _flags)
{
	flags=0;
	opened=false;
	from_cd = false;
	io = INVALID_FP;
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

os_file* files_used_log;
const char* FILES_USED_LOG_NAME = "files_used_log_xb.txt";

/*bool os_file::try_to_open_from_cd()
{
char filename[256];
char cd_filename[256];

  opened = false;
  
	filespec spec(name);
	
	  #if 0
	  if (spec.name.length() > 8 || spec.ext.length() > 4)
	  {
	  //    if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
	  //      nglPrintf("File %s is not in 8.3 format\n", name.c_str());
	  return false;
	  }
	  #endif
	  
		// check the filename is within reasonable limits with cdrom0:\ and ;1 added
		assert(name.length() < 245);
		
		  strcpy(filename, name.c_str());
		  strupr(filename);
		  if (name != "GAME.INI" && os_developer_options::inst()->is_flagged(os_developer_options::FLAG_CHATTY_LOAD))
		  nglPrintf("CD Search Filename: '%s'\n", filename);
		  
			strcpy(cd_filename, "C:\\");
			strcat(cd_filename, filename);
			
			  io = fopen(cd_filename, "rb");
			  if (io == 0)
			  {
			  return false;
			  }
			  else    
			  {
			  opened = true;
			  from_cd = true;
			  }
			  return true;
			  }
*/
bool os_file::try_to_open_from_disk(char drive_letter)
{
	// read from host disk
	
	// look in the locale directory (if specified) for the file first, then check the real place.
	/*
	if ( locale_dir.length() )
	{
    filespec spec(name);
    if ( os_file::file_exists( locale_dir + "\\" + spec.name + spec.ext ) )
	name = locale_dir + "\\" + spec.name + spec.ext;
	}
	*/
	
	//  proftimer_file_open.start();
	// only add the root_dir if there isn't already a drive letter.
	//  if (name[1]!=':' && name[0]!='\\' 
	//      && !(name[0]=='h' && name[1]=='o' && name[2]=='s' && name[3]=='t' && name[4]=='0' && name[5]==':'))
	//    name=root_dir+name;
	
	// until we implement a DVD/CD reader, we'll get all of our data from the host
	//    if (!(name[0]=='h' && name[1]=='o' && name[2]=='s' && name[3]=='t' && name[4]=='0' && name[5]==':'))
	//      name=nglHostPrefix+name;
	
	//  nglPrintf("opening file %s on the host disk\n", name.c_str());
	const char *mode = 0;
	
	// convert the flags to Sony's fileio flags 
	switch (flags)
	{
    case FILE_READ:   mode="rb";              break;
    case FILE_WRITE:  mode="wb+";  break;
    case FILE_APPEND: mode="ab+";  break;
    case FILE_MODIFY: mode="rb+";                break;
	}
	assert(mode);
	
#ifdef EVAN
	char damnopaquestringclass[256];
	strcpy(damnopaquestringclass,name.c_str());
#endif
	// attempt to open the file
	
	// full path
	const char *cname = name.c_str();
	char fullpath[PATH_MAX];
	
	//    assert(strcmp("\\WtC.ifl", cname));
	
	if(cname[1] == ':')
	{
		strcpy( fullpath, cname );
	}
	else if(cname[0] == '\\')
	{
		sprintf( fullpath, "%c:%s", drive_letter, cname );
	}
	else
	{
		sprintf( fullpath, "%c:\\%s", drive_letter, cname );
	}
	
	
	io = fopen(fullpath, mode );
	
	// check for errors
	if (io == 0)
	{
		if(xb_file_warn_about_everything)
			nglPrintf( "xb_file: Could not open %s", _strerror(fullpath) );

		opened = false;
	}
	else    
	{
		//      nglPrintf( "xb_file: OPENED %s\n", fullpath );
		
		opened = true;
		from_cd = false;
	}
	return opened;
}

void create_stash_file_list_entry(stringx name);

/*** open ***/
void os_file::open(const stringx & _name, int _flags)
{
	bool ret = false;
	assert(!opened);
	
	check_system_locked( _name );
	
	//int ate_kludge = _name.find(".ate");
	//assert( ate_kludge == -1 ); // we don't load ates
	
	name = _name;
	flags = _flags;
	
	if (_flags & FILE_READ && _name != "GAME.INI")
		ret = try_to_open_from_disk('Z'); // if we're reading, first try to open from the cache
	
	if(ret)
		debug_print("Opened %s from HD cache\n", name.c_str());
	else 
		ret = try_to_open_from_disk('D'); // if it hasn't been opened yet, try to get it from the game drive
	
	if (ret == false)
	{
		//    nglPrintf("Error.  Could not open file %s on either the cd or the host disk.\n", name.c_str());
		opened = false;
	}
#ifndef NDEBUG
	else if (_flags & FILE_READ && _name != "FONT8X12.tm2" && _name != "GAME.INI" && 
		!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
	{
		create_stash_file_list_entry(name);
	}
#endif
}

void create_stash_file_list_entry(stringx name)
{
#if 0
	files_used_log->open(FILES_USED_LOG_NAME, os_file::FILE_APPEND);
	name.to_lower();
	stringx sfl_name(name);
	//  sfl_name = name.substr(npos, name.length());
	if (files_used_log->is_open())
	{
		files_used_log->set_fp(0, os_file::FP_END);
		files_used_log->write((void *)sfl_name.c_str(), sfl_name.length());
		files_used_log->write((void *)"\r\n", 2);
		files_used_log->close();
	}
#endif
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
	STUB( "os_file::check_for_errors" );
	
	assert(0);
	
	return true;
}

// note: these functions are heavily protected since they are used by classes with complicated logic.

/*** close ***/
void os_file::close()
{
	assert(io!=INVALID_FP);
	assert(opened);
	
	if(io)
		fclose(io);
	
	io = INVALID_FP;
	opened=false;
	from_cd=false;
}


/*** read ***/
int os_file::read(void * data, int bytes, bool async)
{
	if(async)
		STUB( "os_file::read async not handled" );
	
	// returns number of bytes read.
	assert(opened);
	assert(flags&FILE_READ);
	assert(io!=INVALID_FP);
	assert(data);
	assert(bytes>0);
	
	if(at_eof())
	{
		return -1;
	}
	
	int retval = 0;
	
	char *dataptr = (char *) data;
	
	do
	{
		int count = fread(dataptr, 1, bytes, io);
		
		if(count < 0)
		{
			if (!at_eof()) 
			{
				disk_read_error();
			}

			return -1;
		}
		
		if(count == 0)
		{
			if (!at_eof()) 
			{
				disk_read_error();
			}

			return retval;
		}
		
		retval += count;
		dataptr += count;
		
	} while( retval < bytes );
	
	return retval;
}


int os_file::write(void * data, int bytes)
{
	// returns number of bytes written.
	assert(opened);
	assert((flags&FILE_WRITE) || (flags&FILE_APPEND));
	assert(io!=INVALID_FP);
	assert(data);
	assert(bytes>0);
	
	int ret = fwrite(data, 1, bytes, io);
	
	return ret;
}

int os_file::get_size()  // returns file size
{
	if (!opened) 
		return -1;
	
	int cur = fseek(io, 0, SEEK_CUR);
	fseek(io, 0, SEEK_END);
	
	int ret = ftell(io);
	
	fseek(io, cur, SEEK_SET);
	
	return ret;
}


void os_file::set_fp( int pos, filepos_t base )
{
	int method = 0;
	switch ( base )
	{
    case FP_BEGIN:
		method = SEEK_SET;
		break;
    case FP_CURRENT:
		method = SEEK_CUR;
		break;
    case FP_END:
		method = SEEK_END;
		break;
	}
	
	fseek(io, pos, method);
}

bool os_file::at_eof() const
{
	// fixes some crashes with missing files
	if (!io)
		return true;

	return feof(io);
}


unsigned int os_file::get_fp()
{
	return ftell(io);
}

bool os_file::file_exists(const stringx& _name)
{
	//  proftimer_file_exists.start();
	os_file foo;
	int ret = false;
	
	foo.name = _name;
	foo.flags = FILE_READ;
	
	if (_name != "GAME.INI")
		ret = foo.try_to_open_from_disk('Z');

	if(ret)
		debug_print("Found %s in HD cache\n", _name.c_str());
	else 
		ret = foo.try_to_open_from_disk('D');
	
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
	stringx dir_name = name;
	DWORD result;
	
	// First remove trailing slashes, since GetFileAttributes might need
	// to have the bare directory name (?)
	dir_name.remove_trailing("\\/");
	
	// See if the directory exists
	result = GetFileAttributes(dir_name.c_str());
	if((result != -1) && (result & FILE_ATTRIBUTE_DIRECTORY))
		return true;
	else 
		return false;
}

// returns true if file1 is newer than file2.
bool os_file::is_file_newer(const stringx& file1, const stringx& file2)
{
	assert("os_file::is_file_newer is not yet supported on XB, it may never be.\n" == 0);
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
	/*KILLME  switch ( flags & HOST_TB_MASK )
	{
    case HOST_TEXT:
	break;
    case HOST_BINARY:
	break;
}*/
	host_system_file.open( fname, snflags );
	return &host_system_file;
}

void host_fclose( host_system_file_handle fp )
{
	assert( fp == &host_system_file );
	fp->close();
	//  assert("host_fclose is not yet supported on XB, it may never be.\n" == 0);
}

void host_fprintf( host_system_file_handle fp, const char* fmt, ... )
{
	assert( fp == &host_system_file );
	// perform variable argument print
	char buf[1024];
	va_list args;
	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );
	// send string to output file
	fp->write( buf, strlen(buf) );
}

int host_read( host_system_file_handle fp, char* buf, int len )
{
	assert("host_read is not yet supported on XB, it may never be.\n" == 0);
	return 0;
}

bool load_iop_module(const char *module_name, const char *debug_path)
{
	STUB( module_name );
	
	return true;
}
