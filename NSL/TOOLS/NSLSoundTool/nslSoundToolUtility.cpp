// simple and rarely changed utility/wrapper functions go here

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <direct.h>
#include <errno.h>

#include "NSLSoundTool.h"

// note - localized_path moved to nsl_common.cpp

// wow, check out all those parameters!
// file_path -     path to file (include any root dirs)
// file_name -     assumes correct extension in file_name, no path info
// file_flags -    standard fopen flag string
// path_addendum - if non-NULL, the resulting path extensions used where the file was found are
//                 stored here (assumes at least PATH_MAX length)
// used_lang -     if non-NULL, it will be set if we used the language subdir, cleared if not
// used_platform - if non-NULL, it will be set if we used the platform subdir, cleared if not
FILE * localized_fopen( char *file_path, char *file_name, char *file_flags,
												enum _nslLanguageEnum lang, enum _nslPlatformEnum platform,
                        char *path_addendum, bool *used_lang, bool *used_platform )
{
  char check_name[PATH_MAX];
  FILE *ret_val = NULL;

  // check the localized platform directory first (blah/english/ps2/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, nslLanguageStr[lang]);
  strcat(check_name, "\\");
  strcat(check_name, nslPlatformStr[platform]);
  strcat(check_name, "\\");
  strcat(check_name, file_name);
//  message("Looking for file %s\n", check_name);
  ret_val = fopen(check_name, file_flags);

  if (ret_val != NULL)
  {
    if (path_addendum)
    {
      strcpy(path_addendum, nslLanguageStr[lang]);
      strcat(path_addendum, "\\");
      strcat(path_addendum, nslPlatformStr[platform]);
      strcat(path_addendum, "\\");
    }
    if (used_lang)
      *used_lang = true;
    if (used_platform)
      *used_platform = true;
    return ret_val;
  }
  
  // now try the platform dir (blah/ps2/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, nslPlatformStr[platform]);
  strcat(check_name, "\\");
  strcat(check_name, file_name);
//  message("Looking for file %s\n", check_name);
  ret_val = fopen(check_name, file_flags);

  if (ret_val != NULL)
  {
    if (path_addendum)
    {
      strcpy(path_addendum, nslPlatformStr[platform]);
      strcat(path_addendum, "\\");
    }
    if (used_lang)
      *used_lang = false;
    if (used_platform)
      *used_platform = true;
    return ret_val;
  }

  // check the localized directory (blah/english/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, nslLanguageStr[lang]);
  strcat(check_name, "\\");
  strcat(check_name, file_name);
//  message("\nLooking for file %s\n", check_name);
  ret_val = fopen(check_name, file_flags);

  if (ret_val != NULL)
  {
    if (path_addendum)
    {
      strcpy(path_addendum, nslLanguageStr[lang]);
      strcat(path_addendum, "\\");
    }
    if (used_lang)
      *used_lang = true;
    if (used_platform)
      *used_platform = false;
    return ret_val;
  }

  // now try the root dir (blah/file)
  check_name[0] = '\0';
  if (file_path != NULL)
  {
    strcpy(check_name, file_path);
  }
  strcat(check_name, file_name);
//  message("Looking for file %s\n", check_name);
  ret_val = fopen(check_name, file_flags);

  if (ret_val != NULL)
  {
    if (used_lang)
      *used_lang = false;
    if (used_platform)
      *used_platform = false;
    return ret_val;
  }

  return ret_val;
}


void pre_export( void )
{

	if( opt.export_ps2 ) {
		pre_export_ps2( );
	}

	if( opt.export_xbox ) {
		pre_export_xbox( );
	}

	if( opt.export_gc ) {
		pre_export_gc( );
	}

}

void post_export( void )
{

	if( opt.export_ps2 ) {
		post_export_ps2( );
	}

	if( opt.export_xbox ) {
		post_export_xbox( );
	}

	if( opt.export_gc ) {
		post_export_gc( );
	}

  if ( opt.export_soundids_h )
  {
    export_alias_ids( opt.soundids_out_dir );
  }
}

void start_level( const char* filename )
{
	if( opt.export_ps2 ) {
		start_level_ps2( filename );
	}
	if( opt.export_xbox ) {
		start_level_xbox( filename );
	}
	if( opt.export_gc ) {
		start_level_gc( filename );
	}

}

void end_level( const char* filename )
{
	if( opt.export_ps2 ) {
		end_level_ps2( filename );
	}
	if( opt.export_xbox ) {
		end_level_xbox( filename );
	}
	if( opt.export_gc ) {
		end_level_gc( filename );
	}

}

void export_file( snd_info_t* info )
{
	if( opt.export_ps2 ) {
		export_file_ps2( info );
	}

  if( opt.export_xbox ) {
		export_file_xbox( info );
	}
	if( opt.export_gc ) {
		export_file_gc( info );
	}

}

void message( const char* fmt, ... )
{
	char buf[256];
	va_list args;

	if( opt.quiet ) {
		return;
	}

	va_start( args, fmt );
	_vsnprintf( buf, sizeof( buf ), fmt, args );
	fputs( buf, stdout );
#ifdef _DEBUG
	OutputDebugString( buf );
#endif
	va_end( args );
}

void error( const char* fmt, ... )
{
	char buf[256];
	va_list args;

	va_start( args, fmt );
	_vsnprintf( buf, sizeof( buf ), fmt, args );
	fputs( buf, stdout );
#ifdef _DEBUG
	OutputDebugString( buf );
#endif
	va_end( args );
}

void fatal_error( const char* fmt, ... )
{
	char buf[256];
	va_list args;

	va_start( args, fmt );
	_vsnprintf( buf, sizeof( buf ), fmt, args );
	fputs( buf, stdout );
#ifdef _DEBUG
	OutputDebugString( buf );
#endif
	va_end( args );

	exit( -1 );
}

char * safe_strcpy( char *dest, const char *src, int length )
{
  char * ret = strncpy( dest, src, length );
  dest[length-1] = '\0';
  return ret;
}


char* get_basename( char* path )
{
  char* ret = strrchr( path, '\\' );

  if( ret ) {
    return ret + 1;
  } else {
    return path;
	}

}

int count_lines( FILE* file )
{
	int c = -1;
	int num_lines = 0;

	while( !feof( file ) ) {
		c = fgetc( file );

		if( c == '\n' ) {
			++num_lines;
		}

	}

	return num_lines;
}


// cleans up the full_filename field in a GasSource struct
// removes extension, converts to uppercase and converts / to \\
// should this go into the gas module?

int clean_filename( char * filename )
{
  int pos, len;
  char *str;

  len = strlen( filename );

  pos = len;
  // Strip an extension, if any
  while( pos > len - 6 && pos >= 0 )
  {
    if( filename[pos] == '.' ){
      filename[pos] = '\0';
      break;
    }
    pos--;
  }

  // convert to upper case and convert slashes
  str = filename;
  while( *str != '\0' )
  {
    *str = toupper(*str);
    if( *str == '/' ) *str = '\\';
    str++;
  }
  return 1;
}

// if this returns negative, no slash, if zero or above, that's the pos in the string that
// has the last slash (with array indexers)
int find_last_slash( const char* filename )
{
  int pos, len;

  len = strlen( filename );

  pos = len - 1;
  // Strip an extension, if any
  while( pos >= 0 )
  {
    if( filename[pos] == '\\' )
    {
      break;
    }
    pos--;
  }
  return pos;
}

// This function expects a full path, ie c:\foo\bar will create a
// directory c:\foo, but not c:\foo\bar.
int ensure_dir( const char* dirname )
{
	char local[PATH_MAX];
	char* caret = local;

	safe_strcpy( local, dirname, sizeof( local ) );

	while( ( caret = strchr( caret, '\\' ) ) ) {
		*caret = '\0';
		int m = _mkdir( local );

		if( m ) {

			if( errno != EEXIST ) {
				return m;
			}

		}

		*caret = '\\';
		++caret;
	}

	return 0;
}
