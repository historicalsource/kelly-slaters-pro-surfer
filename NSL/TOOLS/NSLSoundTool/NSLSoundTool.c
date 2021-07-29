#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "PS2SoundTool.h"
#include "XBoxSoundTool.h"
#include "GCSoundTool.h"

int opt_dryrun = 0;

char opt_input_wav_dir[256];
char opt_input_snd_dir[256];

char opt_level_dir[256];

char opt_ps2_out_dir[256];
char opt_ps2_spu_dir[256];

char opt_xbox_out_dir[256];

char opt_gc_out_dir[256];

int opt_export_ps2 = 0;
int opt_export_xbox = 0;
int opt_export_gc = 0;

int opt_quiet = 0;

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

static int parse_args( int argc, char* argv[] )
{
  int i = 1;
  int in_options = 1;

  while( ( i < argc ) && in_options ) {

    if( argv[i][0] != '-' ) {
      in_options = 0;
    } else {

      switch( argv[i][1] ) {
      case 'D':
      case 'd':
        opt_dryrun = 1;
        ++i;
        break;

      case 'I':
      case 'i':
        ++i;
        strcpy( opt_input_wav_dir, argv[i] );
        ++i;
        break;

      case 'S':
      case 's':
        ++i;
        strcpy( opt_input_snd_dir, argv[i] );
        ++i;
        break;

      case 'O':
      case 'o':
        ++i;
        strcpy( opt_ps2_out_dir, argv[i] );
				opt_export_ps2 = 1;
        ++i;
        break;

      case 'P':
      case 'p':
        ++i;
        strcpy( opt_ps2_spu_dir, argv[i] );
        ++i;
        break;

      case 'L':
      case 'l':
        ++i;
        strcpy( opt_level_dir, argv[i] );
        ++i;
        break;

      case 'X':
      case 'x':
        ++i;
        strcpy( opt_xbox_out_dir, argv[i] );
        opt_export_xbox = 1;
        ++i;
        break;

			case 'G':
			case 'g':
				++i;
				strcpy( opt_gc_out_dir, argv[i] );
				opt_export_gc = 1;
				++i;
				break;

			case 'Q':
			case 'q':
				opt_quiet = 1;
				++i;
				break;

      default:
        printf( "parse_args: unrecognized option \'%s\'\n", argv[i] );
        ++i;
        break;
      }

    }

  }

	return i;
}

static void pre_export( void )
{

	if( opt_export_ps2 ) {
		pre_export_ps2( );
	}

	if( opt_export_xbox ) {
		pre_export_xbox( );
	}

	if( opt_export_gc ) {
		pre_export_gc( );
	}

}

static void post_export( void )
{

	if( opt_export_ps2 ) {
		post_export_ps2( );
	}

	if( opt_export_xbox ) {
		post_export_xbox( );
	}

	if( opt_export_gc ) {
		post_export_gc( );
	}

}

static void start_level( const char* filename )
{

	if( opt_export_ps2 ) {
		start_level_ps2( filename );
	}

	if( opt_export_xbox ) {
		start_level_xbox( filename );
	}

	if( opt_export_gc ) {
		start_level_gc( filename );
	}

}

static void end_level( const char* filename )
{

	if( opt_export_ps2 ) {
		end_level_ps2( filename );
	}

	if( opt_export_xbox ) {
		end_level_xbox( filename );
	}

	if( opt_export_gc ) {
		end_level_gc( filename );
	}

}

static void export_file( snd_info_t* info )
{
		
	if( opt_export_ps2 ) {
		export_file_ps2( info );
	}

  if( opt_export_xbox ) {
		export_file_xbox( info );
	}

	if( opt_export_gc ) {
		export_file_gc( info );
	}

}

static const char* next_token( const char* src, char* dst, size_t n )
{
	unsigned int si = 0;
	unsigned int di = 0;

	// catch special case of an empty string
	if( ( src == NULL ) || ( src[0] == '\0' ) ) {
		return NULL;
	}

	while( 1 ) {

		if( di >= n ) {
			return &src[si];
		}

		if( src[si] == '\0' ) {
			dst[di] = '\0';
			return &src[si];
		}
		
		if( src[si] == ' ' || src[si] == '\n' ) {
			dst[di] = '\0';
			return &src[++si];
		}

		dst[di] = src[si];
		++si;
		++di;
	}

	// impossible state
	return NULL;
}

static void build_info( const char* line, snd_info_t* info )
{
	char token[256];
	const char* caret = next_token( line, token, sizeof( token ) );

	strncpy( info->name, token, sizeof( info->name ) );

	info->loop = 0;
	info->cd = 0;

	while( ( caret = next_token( caret, token, sizeof( token ) ) ) ) {

		if( stricmp( token, "loop" ) == 0 ) {
			info->loop = 1;
		} else if( stricmp( token, "cd" ) == 0 ) {
			info->cd = 1;
		} else {

			if( token[0] ) {
				message( "build_info: unrecognized token '%s'\n", token );
			}

		}

	}

}

static void export_level( const char* level )
{
	char filename[PATH_MAX];
	FILE* file = NULL;
	char line[1024];

	_snprintf( filename, sizeof( filename ), "%s\\%s.snd", opt_input_snd_dir, level );

	// open file
	if( ( file = fopen( filename, "r" ) ) == NULL ) {
		error( "couldn't open snd input file '%s'\n", filename );
		return;
	}

	while( !feof( file ) ) {
		snd_info_t info;

		// parse line
		fgets( line, sizeof( line ), file );

		if( line[0] == '\0' ) {
			continue;
		}

		if( line[0] == ';' ) {
			continue;
		}

		// export line
		build_info( line, &info );
		export_file( &info );
	}

	// close file
	fclose( file );
}

void message( const char* fmt, ... )
{
	char buf[256];
	va_list args;

	if( opt_quiet ) {
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

int main( int argc, char* argv[] )
{
  int num_lines = 0;
	int curr_file = 0;
	int found = 0;
 
	strcpy( opt_input_wav_dir, "C:\\sm\\data\\" );
	strcpy( opt_input_snd_dir, "C:\\sm\\data\\sounds\\" );

	strcpy( opt_level_dir, "levels" );

	strcpy( opt_ps2_out_dir, "C:\\sm\\data\\ps2sound\\" );
  strcpy( opt_ps2_spu_dir, "spu" );

	strcpy( opt_xbox_out_dir, "X:\\sm\\data\\xboxsound\\" );

	strcpy( opt_gc_out_dir, "C:\\sm\\dvdroot\\gcsound\\" );

 	if( argc < 2 ) {
  	printf( "Usage: %s [OPTION]... [SND]...\n", get_basename( argv[0] ) );
		printf( " -D          Dry run, do not write output\n" );
		printf( " -Q          Be quiet\n" );
		printf( " -I <dir>    Input WAV file directory\n" );
		printf( " -S <dir>    Input SND file directory\n");
		printf( " -L <dir>    Output SND file directory name (e.g., \"levels\")\n" );
		printf( " -O <dir>    Output PS2 audio directory\n" );
		printf( " -P <dir>    Output PS2 SPU directory name (e.g., \"spu\")\n" );
		printf( " -X <dir>    Output Xbox audio directory\n" );
		printf( " -G <dir>    Output Gamecube audio directory\n" );

    return 0;
  }

 	curr_file = parse_args( argc, argv );

	pre_export( );

  while( curr_file < argc ) {
		char snd_file[PATH_MAX];

		strncpy( snd_file, argv[curr_file], sizeof( snd_file ) );

		start_level( snd_file );
		export_level( snd_file );
		end_level( snd_file );

		++curr_file;
	}

	post_export( );

  return 0;
}

