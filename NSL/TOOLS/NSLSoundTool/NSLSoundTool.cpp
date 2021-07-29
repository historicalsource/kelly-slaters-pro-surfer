#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "NSLSoundTool.h"

opt_info_t opt;

float NSL_SOUND_TOOL_VERSION = 1.26f;
time_t TIMESTAMP_WHEN_EXPORTED = 0;

// in nsl_common.cpp
extern nslLanguageEnum nslLanguageStringToEnum( const char *stringToLookup );

// default options
void opt_info_t::clear()
{
  // strings
	safe_strcpy( input_wav_dir,     "C:\\sm\\data\\" );
	safe_strcpy( input_snd_dir,     "C:\\sm\\data\\sounds\\" );
	safe_strcpy( ps2_out_dir,       "C:\\sm\\data\\ps2sound\\" );
  safe_strcpy( ps2_spu_dir,       "spu" );
	safe_strcpy( xbox_out_dir,      "c:\\sm\\data\\xboxsound\\" );
	safe_strcpy( gc_out_dir,        "C:\\sm\\dvdroot\\gcsound\\" );
  safe_strcpy( default_info_file, "global.snd" );
	safe_strcpy( ps2_collection_dir, "ps2sound" );

  // flags
  for ( int i=0; i<NSL_LANGUAGE_Z; ++i )
  {
    export_language[i] = false;
  }
  
  export_ps2 = false;
  export_xbox = false;
  export_gc = false;
  export_soundids_h = false;

  dryrun = false;
  quiet = false;
  ultra_verbose = false;
  only_newer = false;
  emulation_export_only = false;
}


void parse_language( char *language_str )
{
  char lower_lang[PATH_MAX];
  strcpy( lower_lang, language_str );
  strupr( lower_lang );

  for ( int i=0; i<NSL_LANGUAGE_Z; ++i )
  { 
    if ( strcmp( lower_lang, nslLanguageStr[i] ) == 0 )
    {
      // a match, so we export this language
      opt.export_language[i] = true;
      break;
    }
  }
  if ( i == NSL_LANGUAGE_Z )
  {
		error( "couldn't find language '%s'\n", language_str );
  }
}


static int parse_args( int argc, char* argv[] )
{
  int j,i = 1;
  int in_options = 1;

  while( ( i < argc ) && in_options ) {

    if( argv[i][0] != '-' ) {
      in_options = 0;
    } else {

      switch( argv[i][1] ) {
      case 'D':
      case 'd':
        opt.dryrun = true;
        ++i;
        break;

      case 'T':
      case 't':
        ++i;
        parse_language( argv[i] );
        ++i;
        break;

      case 'F':
      case 'f':
        ++i;
        safe_strcpy( opt.default_info_file, argv[i] );
        ++i;
        break;

      case 'I':
      case 'i':
        ++i;
        safe_strcpy( opt.input_wav_dir, argv[i] );
        ++i;
        break;

      case 'S':
      case 's':
        ++i;
        safe_strcpy( opt.input_snd_dir, argv[i] );
        ++i;
        break;

      case 'N':
      case 'n':
				opt.only_newer = true;
        ++i;
        break;
      
      case 'O':
      case 'o':
        ++i;
        safe_strcpy( opt.ps2_out_dir, argv[i] );
				opt.export_ps2 = true;
        ++i;
        break;

      case 'P':
      case 'p':
        ++i;
        safe_strcpy( opt.ps2_spu_dir, argv[i] );
        ++i;
        break;

      case 'X':
      case 'x':
        ++i;
        safe_strcpy( opt.xbox_out_dir, argv[i] );
        opt.export_xbox = true;
        ++i;
        break;

			case 'G':
			case 'g':
				++i;
				safe_strcpy( opt.gc_out_dir, argv[i] );
				opt.export_gc = true;
				++i;
				break;

			case 'Q':
			case 'q':
				opt.quiet = true;
				++i;
				break;

      case 'U':
      case 'u':
        opt.ultra_verbose = true;
        ++i;
        break;

      case 'H':
      case 'h':
        ++i;
				safe_strcpy( opt.soundids_out_dir, argv[i] );
        opt.export_soundids_h = true;
        ++i;
        break;

      case 'C':
      case 'c':
        ++i;
				safe_strcpy( opt.ps2_collection_dir, argv[i] );
        ++i;
        break;

      default:
        printf( "parse_args: unrecognized option \'%s\'\n", argv[i] );
        ++i;
        break;
      }
    }
  }
  // enable english language choice, if no languages were specified
  for ( j=0; j<NSL_LANGUAGE_Z; ++j )
  { 
    if ( opt.export_language[j] == true )
      break;
  }
  opt.export_language[NSL_LANGUAGE_NONE] = true;
  if ( j == NSL_LANGUAGE_Z )
  {
    opt.export_language[NSL_LANGUAGE_ENGLISH] = true;
  }

	return i;
}

static void export_level( const char* level, nslLanguageEnum lang = NSL_LANGUAGE_Z );

bool handle_include_file( const char *line )
{
  bool ret_val = false;
  nslLanguageEnum languageId = NSL_LANGUAGE_Z;
  if (line[0] == '#' && strncmp(line, "#include <", 10) == 0)
  {
    char *curr_pos = (char *)line + 10;
    char include_me[PATH_MAX];
    int i=0;

    ret_val = true;

    while ( (*curr_pos) != '>' )
    {
      include_me[i] = *curr_pos;
      ++curr_pos;
      ++i;
    }
    include_me[i] = '\0';

    // check for language specifier ( must match language name in nsl_common.cpp )
    char language_specifier[PATH_MAX];
    i = 0;
    language_specifier[0] = '\0';
    while ( (*curr_pos) != '\0' && (*curr_pos) != '\r' && (*curr_pos) != '\n' ) 
    {
      if (!(isspace(*curr_pos) || (*curr_pos) == '>'))
      {
        language_specifier[i] = toupper(*curr_pos);
        ++i;
      }
      ++curr_pos;
    }
    language_specifier[i] = '\0';

    if (language_specifier[0] != '\0')
    {
      languageId = nslLanguageStringToEnum( language_specifier );
    }
    export_level( include_me, languageId );
  }
  return ret_val;
}


static void export_level( const char* level, nslLanguageEnum lang )
{
	char filename[PATH_MAX];
	FILE* file = NULL;
	char line[1024];
  int counter = 0;

	_snprintf( filename, sizeof( filename ), "%s\\%s.snd", opt.input_snd_dir, get_basename((char *)level) );

	// open file
	if( ( file = fopen( filename, "r" ) ) == NULL ) {
		error( "couldn't open snd input file '%s'\n", filename );
		return;
	}

	while( !feof( file ) ) {
		snd_info_t info;

		// parse line
		char *val = fgets( line, sizeof( line ), file );

    // check for include directives
		if( val == NULL || line[0] == '\0' || ( line[0] == ';' ) || ( line[0] == '\r' ) || ( line[0] == '\n' ) )
			continue;

    if (handle_include_file( line ))
      continue;

		// export line
		build_info( line, &info );

    // set which languages to export for this info struct
    if (lang == NSL_LANGUAGE_Z)
    {
      // export for all languages
      for (int j=0; j<NSL_LANGUAGE_Z; ++j)
      {
        info.languages[j] = opt.export_language[j];
      }
    }
    else
    {
      for (int j=0; j<NSL_LANGUAGE_Z; ++j)
      {
        info.languages[j] = false;
      }
      // export for just this language
      info.languages[lang] = true;
    }

		export_file( &info );
	}
	// close file
	fclose( file );
}



int main( int argc, char* argv[] )
{
  int num_lines = 0;
	int curr_file = 0;
	int found = 0;
  int lang_no = 0;

  time( &TIMESTAMP_WHEN_EXPORTED );

  opt.clear();

 	if( argc < 2 ) {
    printf( "NSL Sound Tool -- ver %g\n\n", NSL_SOUND_TOOL_VERSION ); 
  	printf( "Usage: %s [OPTION]... [SND]...\n", get_basename( argv[0] ) );
		printf( " -D          Dry run, do not write output\n" );
		printf( " -Q          Be quiet\n" );
		printf( " -U          Ultra verbose mode (print out all files as processed)\n" );
    printf( " -T <lang>   Build sounds for the given language (see nsl.cpp for list)\n" );
		printf( " -F <file>   Global SND file for sound info defaults\n" );
		printf( " -I <dir>    Input WAV file directory\n" );
		printf( " -S <dir>    Input SND file directory\n");
		printf( " -L <dir>    Output SND file directory name (e.g., \"levels\") [NOT SUPPORTED]\n" );
		printf( " -O <dir>    Output PS2 audio directory\n" );
		printf( " -P <dir>    Output PS2 SPU directory name (e.g., \"spu\")\n" );
    printf( " -C <dir>    Local PS2 collections path (ps2sound by default)\n" );
		printf( " -X <dir>    Output Xbox audio directory\n" );
		printf( " -G <dir>    Output Gamecube audio directory\n" );
    printf( " -H <dir>    Output SoundIDs.h into this directory\n" );
    printf( " -N <dir>    Only re-export changed files (only newer) PS2 only currently.\n" );
    return 0;
  }

 	curr_file = parse_args( argc, argv );

  message( "NSL Sound Tool -- ver %g\n\n", NSL_SOUND_TOOL_VERSION ); 

	pre_export( );
  load_info_defaults( opt.default_info_file );


  // process all of the levels (banks)
  while( curr_file < argc ) {
		char snd_file[PATH_MAX];

		strncpy( snd_file, argv[curr_file], sizeof( snd_file ) );
    message( "\nProcessing %s\n", snd_file);
		start_level( snd_file );
		export_level( snd_file );
		end_level( snd_file );

		++curr_file;
	}

	post_export( );

  return 0;
}



