#ifndef _PS2_SOUND_TOOL_H
#define _PS2_SOUND_TOOL_H

// for any NSL specific defines
#include "common/nsl.h"

struct ps2_info_t 
{
  int   freq;
  bool  stereo;
  bool  processed;

  long  wave_modified_time[NSL_LANGUAGE_Z];
  int   padded_size[NSL_LANGUAGE_Z];
  int   real_size[NSL_LANGUAGE_Z];
  int   cd_offset[NSL_LANGUAGE_Z];
  char* vag_buffer[NSL_LANGUAGE_Z];
  char info_file_name[NSL_LANGUAGE_Z][PATH_MAX];

  ps2_info_t()
  {
    clear(false);
  }
  ~ps2_info_t()
  {
    for (int i=0; i<NSL_LANGUAGE_Z; ++i)
    {
      if (vag_buffer[i])
        free(vag_buffer[i]);
      vag_buffer[i] = NULL;
    }
  }

  void clear (bool free_buffer = true)
  {
    freq = 0;
    stereo = false;
    processed = false;

    int i;
    for ( i=0; i<NSL_LANGUAGE_Z; ++i )
    {
      cd_offset[i] = -1;
      padded_size[i] = 0;
      real_size[i] = 0;
      info_file_name[i][0] = '\0';
      wave_modified_time[i] = 0;
    }

    for ( i=0; i<NSL_LANGUAGE_Z; ++i )
    {
      if (free_buffer && vag_buffer[i])
        free(vag_buffer[i]);
      vag_buffer[i] = NULL;
    }
  }
  void copy ( const ps2_info_t &other )
  {
    freq = other.freq;
    stereo = other.stereo;
    processed = other.processed;

    int i;
    for ( i=0; i<NSL_LANGUAGE_Z; ++i )
    {
      padded_size[i] = other.padded_size[i];
      real_size[i] = other.real_size[i];
      cd_offset[i] = other.cd_offset[i];
      strcpy( info_file_name[i], other.info_file_name[i] );
      wave_modified_time[i] = other.wave_modified_time[i];
    }

    // clear out our old vag_buffer
    for ( i=0; i<NSL_LANGUAGE_Z; ++i )
    {
      if (vag_buffer[i])
        free(vag_buffer[i]);
      vag_buffer[i] = NULL;
    }

    // create a new one and copy the other guy's data over
    for ( i=0; i<NSL_LANGUAGE_Z; ++i )
    {
      if (other.vag_buffer[i])
      {
        vag_buffer[i] = (char *)malloc(other.padded_size[i]);
        memcpy(vag_buffer[i], other.vag_buffer[i], other.padded_size[i]);
      }
    }
  }
  void free_vag_buffer()
  {
    for ( int i=0; i<NSL_LANGUAGE_Z; ++i )
    {
      if ( vag_buffer[i] )
        free( vag_buffer[i] );
      vag_buffer[i] = NULL;
    }
  }
};

struct snd_info_t;

// called before any export occurs
int pre_export_ps2( void );
// called after all per-snd exporting is finished
int post_export_ps2( void );

// called before beginning to export per-file data for a level
int start_level_ps2( const char* level );
// called for each file listed in an input SND file
int export_file_ps2( snd_info_t* info );
// called after all files from an input SND have been exported
int end_level_ps2( const char* level );

#endif
