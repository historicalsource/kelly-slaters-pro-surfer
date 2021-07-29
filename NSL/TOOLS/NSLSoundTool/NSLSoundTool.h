#ifndef _NSL_SOUND_TOOL_H
#define _NSL_SOUND_TOOL_H

#include <stdio.h>


// for any NSL specific defines
#include "common/nsl.h"
#include "snd_parser.h"

//
// Export option information structure. 
//
struct opt_info_t 
{
  // input directories
  char input_wav_dir[PATH_MAX];
  char input_snd_dir[PATH_MAX];
  char default_info_file[PATH_MAX];

  // output directories
  char ps2_out_dir[PATH_MAX];
  char ps2_spu_dir[PATH_MAX];
  char ps2_collection_dir[PATH_MAX];

  char xbox_out_dir[PATH_MAX];

  char gc_out_dir[PATH_MAX];

  char soundids_out_dir[PATH_MAX];

  // export settings
  bool export_language[NSL_LANGUAGE_Z];
  bool export_ps2;
  bool export_xbox;
  bool export_gc;
  bool export_soundids_h;

  // flags
  bool dryrun;
  bool quiet;
  bool ultra_verbose;
  bool only_newer;
  bool emulation_export_only;
  opt_info_t()
  {
    clear();
  }

  void clear();
};

// the global instance of the opt_info structure
extern opt_info_t opt;


/*
 * Very simple services provided to other modules.
 *
 * code in nslSoundToolUtility.cpp
 */

// export stage functions
void  pre_export( void );
void  post_export( void );
void  start_level( const char* filename );
void  end_level( const char* filename );
void  export_file( snd_info_t* info );

// useful messages
void  message( const char* fmt, ... );
void  error( const char* fmt, ... );
void  fatal_error( const char* fmt, ... );

// truly utility functions
char* safe_strcpy( char *dest, const char *src, int length = PATH_MAX);
char* get_basename( char* name );
int   count_lines( FILE* file );
int localized_path( char* file_path,
										char* file_name,
										_nslLanguageEnum lang,
										_nslPlatformEnum platform,
										char* path_addendum );
FILE * localized_fopen( char *file_path, char *file_name, char *file_flags,
											  nslLanguageEnum lang, nslPlatformEnum platform,
                        char *path_addendum = NULL, bool *used_lang = NULL, bool *used_platform = NULL );
int clean_filename( char * filename );
int find_last_slash( const char* filename );
int ensure_dir( const char* filename );

#endif
