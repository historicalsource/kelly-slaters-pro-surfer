#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <direct.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

extern "C"
{
#include "encvag.h"
}
#include <algorithm>
#include "sndfile.h"
#include "../../ps2/gas.h"
#include "../../ps2/gas/GasDefines.h"
#include "../../ps2/gas/GasSystem.h"
#include "../../common/nsl.h"
#include "NSLSoundTool.h"

// defined in nsl.cpp


struct ps2_system_info_t
{
  char level_name[PATH_MAX];
  char level_dir[PATH_MAX];
  vector<snd_info_t> cd_entries;    // files nominated for inclusion in stream.vbc
  vector<snd_info_t> level_entries[NSL_LANGUAGE_Z]; // files needed for this level
  FILE* cd_snd_file;
  FILE* cd_file;
  int   cd_offset;
  int   spu_mem_free[NSL_LANGUAGE_Z];
  FILE* spu_file[NSL_LANGUAGE_Z];
  FILE* snd_file[NSL_LANGUAGE_Z];
  int   spu_offset[NSL_LANGUAGE_Z];

  void clear() 
  {
    level_name[0] = '\0';
    level_dir[0] = '\0';
    cd_entries.clear();
//    level_entries.clear();
    if (cd_file != NULL)
      fclose(cd_file);
    if (cd_snd_file != NULL)
      fclose(cd_snd_file);
    cd_file = NULL;
    cd_snd_file = NULL;
    cd_offset = 0;
    for (int i=0; i<NSL_LANGUAGE_Z; ++i)
    {
      spu_mem_free[i] = 0;
      spu_file[i] = NULL;
      snd_file[i] = NULL;
      spu_offset[i] = 0;
      level_entries[i].clear();
    }
  }
};

ps2_system_info_t ps2_info;

#define STRING_LENGTH 256
#define MAX_LINES 600


#define BLK_SIZE 2048
#define SPU_BLK_SIZE 16
#define REAL_SPU_BLK_SIZE 56

#define REAL_BLK_SIZE ( ( BLK_SIZE / SPU_BLK_SIZE ) * REAL_SPU_BLK_SIZE )

#define AUDIO_IN_EXT ".wav"
#define AUDIO_OUT_EXT ".vbc"
#define AUDIO_INFO_EXT ".info"

int convert_entry( snd_info_t &entry, nslLanguageEnum which_lang );
bool write_preconverted_file( snd_info_t &entry, nslLanguageEnum lang );


#define g_spu_buffer_num  ((MAX_CD_STEREO_STREAMS * 2) + MAX_CD_MONO_STREAMS)
#define g_spu_heap_start  (SPU_MEMORY_TOP + (SPU_BUFFER_SIZE * g_spu_buffer_num))
#define g_spu_heap_end    (SPU_MEMORY_MAX - (2 * SPU_FX_WORK_AREA_SIZE))
//#define g_spu_heap_end    (SPU_MEMORY_MAX - (SPU_FX_WORK_AREA_SIZE))

static int g_spu_heap_curr = (g_spu_heap_start);

extern float NSL_SOUND_TOOL_VERSION;
extern time_t TIMESTAMP_WHEN_EXPORTED;

// IMPORTANT : if you update this file format, you MUST also increment 
//             the NSL Sound Tool version number, or old, invalid files
//             won't work
bool write_preconvert_info_file( char *file_name, snd_info_t &nfo, nslLanguageEnum lang )
{
  // open the file for writing
  FILE *outfile = fopen( file_name, "wb" );
  if (outfile)
  {

    long FILE_INFO_STRUCT_MAGIC_NUMBER = long(NSL_SOUND_TOOL_VERSION * 1000.0f);
    fwrite( &FILE_INFO_STRUCT_MAGIC_NUMBER, 1, sizeof(long), outfile );
    fwrite( &nfo.ps2.wave_modified_time[lang], 1, sizeof(long), outfile );
    fwrite( &nfo.ps2.padded_size[lang], 1, sizeof(int), outfile );
    fwrite( &nfo.ps2.real_size[lang], 1, sizeof(int), outfile );

    fwrite( &nfo.ps2.freq, 1, sizeof(int), outfile );
    fwrite( &nfo.ps2.stereo, 1, sizeof(bool), outfile );
    fwrite( &nfo.loop, 1, sizeof(bool), outfile );
    fwrite( &nfo.applyReverb, 1, sizeof(bool), outfile );

    fwrite( &nfo.name, PATH_MAX, sizeof(char), outfile );
    fwrite( &nfo.alias, PATH_MAX, sizeof(char), outfile );
    fwrite( &nfo.aliasID, 1, sizeof(int), outfile );
    fwrite( &nfo.volume_left, 1, sizeof(float), outfile );
    fwrite( &nfo.volume_right, 1, sizeof(float), outfile );
    fwrite( &nfo.pitch, 1, sizeof(float), outfile );
    fwrite( &nfo.type, 1, sizeof(nslSourceTypeEnum), outfile );

    if ( nfo.ps2.vag_buffer[lang] )
    {
      // now the vag info.
      fwrite( nfo.ps2.vag_buffer[lang], nfo.ps2.padded_size[lang], sizeof(char), outfile );
    }
    else
    {
      error("Eh?  that's strange, no VAG info to write out for file %s\n", file_name);
    }

    fclose( outfile );
    return true;
  }
  return false;
}

// IMPORTANT : if you update this file format, you MUST also increment 
//             the NSL Sound Tool version number, or old, invalid files
//             won't work
bool read_preconvert_info_file( char *file_name, snd_info_t &nfo, nslLanguageEnum lang, bool read_vb_data )
{
  FILE *infile = fopen( file_name, "rb" );
  bool ret_val = false;
  if (infile)
  {
    long magic_number = 0;
    fread( &magic_number, 1, sizeof(long), infile );
    float FILE_INFO_STRUCT_MAGIC_NUMBER = long(NSL_SOUND_TOOL_VERSION * 1000.0f);
    if ( magic_number == FILE_INFO_STRUCT_MAGIC_NUMBER )
    {
      ret_val = true;
      // continue reading
      fread( &nfo.ps2.wave_modified_time[lang], 1, sizeof(long), infile );
      fread( &nfo.ps2.padded_size[lang], 1, sizeof(int), infile );
      fread( &nfo.ps2.real_size[lang], 1, sizeof(int), infile );

      fread( &nfo.ps2.freq, 1, sizeof(int), infile );
      fread( &nfo.ps2.stereo, 1, sizeof(bool), infile );
      fread( &nfo.loop, 1, sizeof(bool), infile );
      fread( &nfo.applyReverb, 1, sizeof(bool), infile );

      fread( &nfo.name, PATH_MAX, sizeof(char), infile );
      fread( &nfo.alias, PATH_MAX, sizeof(char), infile );
      fread( &nfo.aliasID, 1, sizeof(int), infile );
      fread( &nfo.volume_left, 1, sizeof(float), infile );
      fread( &nfo.volume_right, 1, sizeof(float), infile );
      fread( &nfo.pitch, 1, sizeof(float), infile );
      fread( &nfo.type, 1, sizeof(nslSourceTypeEnum), infile );

      if ( read_vb_data )
      {
        if ( nfo.ps2.vag_buffer[lang] == NULL )
        {
          nfo.ps2.vag_buffer[lang] = (char *)malloc( nfo.ps2.padded_size[lang] );
        }
        if ( nfo.ps2.vag_buffer[lang] != NULL )
        {
          // now the vag info.
          fread( nfo.ps2.vag_buffer[lang], nfo.ps2.padded_size[lang], sizeof(char), infile );
        }
      }
    }
    else
    {
      if (opt.ultra_verbose)
        message("%s: Incorrect NSL version, re-converting.\n", nfo.name);
    }
    fclose( infile );
  }
  return ret_val;
}


bool write_preconverted_file( snd_info_t &entry, nslLanguageEnum lang )
{
  bool wrote_out_file = false;
  int  result = 0;
  char found_file_name[PATH_MAX];
  char path_addendum[PATH_MAX];
  char base_path[PATH_MAX];
  char base_file_name[PATH_MAX];

  // find the right file
  path_addendum[0] = '\0';
  found_file_name[0] = '\0';
  sprintf( base_path, "%s%s", opt.input_wav_dir, entry.path );
  sprintf( base_file_name, "%s%s", entry.name, AUDIO_IN_EXT );

  result = _nslLocalizedPath( base_path, base_file_name, lang, NSL_PLATFORM_PS2, 
                              found_file_name, path_addendum );
  if (result != -1)
  {
    // check if the file is already there
    struct _stat buf;
    result = _stat( found_file_name, &buf );

    if( result != 0 )
      error( "Error getting system info (perhaps it's missing?) for %s\n", entry.name );
    else
    {
      /* Output some of the statistics: * /
      message( "File name     : %s\n", found_file_name );
      message( "File path     : %s\n", path_addendum );
      message( "File size     : %ld\n", buf.st_size );
      message( "Drive         : %c:\n", buf.st_dev + 'A' );
      message( "Time created  : %s", ctime( &buf.st_ctime ) );
      message( "Time accessed : %s", ctime( &buf.st_atime ) );
      message( "Time modified : %s", ctime( &buf.st_mtime ) ); // */
    }

    // build info file name

    sprintf( entry.ps2.info_file_name[lang], "%s%s%s%s", base_path, path_addendum, entry.name, AUDIO_INFO_EXT );

    if (opt.only_newer)
    {
      // now get the info from our previous conversion (if any)
      if ( read_preconvert_info_file( entry.ps2.info_file_name[lang], entry, lang, false ) )
      {
        double time_diff = difftime( buf.st_mtime, entry.ps2.wave_modified_time[lang] );
        if ( time_diff != 0.0 )
        {
          if ( time_diff < 0.0 )
          {
            error("This is strange, the file (%s)\n  was modified before (%s)\n  the time we processed it last (%s).\n",
                  found_file_name, ctime( &buf.st_mtime ), ctime( &entry.ps2.wave_modified_time[lang] ) );
            // even though it may not need it, reprocess it anyway, for paranoia's sake
          }

          // newer, we need to process it again
          wrote_out_file = true;
        }
      }
      else
      {
        // no info file, so we need to process it
        wrote_out_file = true;
      }
    }
    else
    {
      // if not only newer, then we need to reprocess it
      wrote_out_file = true;
    }

    // now reprocess it, if wrote_out_file is true
    if (wrote_out_file)
    {
      entry.ps2.wave_modified_time[lang] = buf.st_mtime;

      if (opt.ultra_verbose)
        message("%s: Is being processed\n", entry.name);
      // actually do the conversion.
      convert_entry(entry, lang);

      // write out the converted data and such.
      write_preconvert_info_file( entry.ps2.info_file_name[lang], entry, lang );
      entry.ps2.free_vag_buffer();
    }
  }

  return wrote_out_file;
}


int pre_export_ps2( void )
{
  if( !opt.dryrun )
  {
    // clear must happen before we set the cd_file
    ps2_info.clear();

    char cd_collection[PATH_MAX];
    char snd_name[PATH_MAX];
    char pathy_path[PATH_MAX];
    if ((opt.ps2_out_dir[strlen(opt.ps2_out_dir) -1] != '\\') &&
        (opt.ps2_out_dir[strlen(opt.ps2_out_dir) -1] != '/'))
    {
      sprintf( pathy_path, "%s\\STREAM\\", opt.ps2_out_dir );
    }
    else
    {
      sprintf( pathy_path, "%sSTREAM\\", opt.ps2_out_dir );
    }
    sprintf( cd_collection, "%sSTREAM.VBC", pathy_path );
    sprintf( snd_name, "%sSTREAM.SND", pathy_path );

    mkdir( pathy_path );

    // bail out without writing if we're doing an emulation export
    if (!opt.emulation_export_only)
    {
      ps2_info.cd_file = fopen( cd_collection, "wb" );
      ps2_info.cd_snd_file = fopen( snd_name, "wb" );

      if( !ps2_info.cd_file )
      {
        fatal_error( "could not open cd output file: %s\n", cd_collection ); 
      } 
  
      if( ps2_info.cd_snd_file )
      {
        fprintf(ps2_info.cd_snd_file, "; This file is just for human reference.\n");
        fprintf(ps2_info.cd_snd_file, "; It details what was put in the streaming VBC file\n");
        fprintf(ps2_info.cd_snd_file, "; and where we put it.  It is not used in-game\n\n");
      } 
    }
    else
    {
      ps2_info.cd_file = NULL;
      ps2_info.cd_snd_file = NULL;
    }
  }

	return 0;
}

int post_export_ps2( void )
{
  // now create the streaming vbc file

  if( !opt.dryrun && !opt.emulation_export_only)
  {
    fclose( ps2_info.cd_file );
    fclose( ps2_info.cd_snd_file );
  }

	return 0;
}


int start_level_ps2( const char* level )
{
//  // clear out any previous level information
//  ps2_info.level_entries.clear();

  // set up our new level information
  safe_strcpy( ps2_info.level_name, (char *)level );
  clean_filename(ps2_info.level_name);
  int name_len = strlen(ps2_info.level_name);
  int last_slash = find_last_slash( ps2_info.level_name );
  if (last_slash >= 0)
  {
    safe_strcpy( ps2_info.level_dir, ps2_info.level_name, last_slash + 1 );
    safe_strcpy( ps2_info.level_name, (char *)(level + last_slash + 1), name_len - last_slash );
  }
  else
  {
    safe_strcpy( ps2_info.level_dir, "levels", PATH_MAX );
  }

  // reset our memory total
  for (int i=0; i<NSL_LANGUAGE_Z; ++i)
  {
    ps2_info.level_entries[i].clear();
    ps2_info.spu_mem_free[i] = g_spu_heap_end - g_spu_heap_start;
    ps2_info.spu_offset[i] = 0;
  }

  return 0;
}

struct size_wrapper
{
  int size;
  int index;
  size_wrapper() : size(0), index(0) {}
  size_wrapper( int _size, int _index )  
  { 
    size = _size;
    index = _index;
  }
  size_wrapper( const size_wrapper &other)
  {
    size = other.size;
    index = other.index;
  }
  bool operator == (const size_wrapper &other) const
  {
    return (size == other.size);
  }
  bool operator < (const size_wrapper &other) const
  {
    return (size < other.size);
  }
  bool operator > (const size_wrapper &other) const
  {
    return (size > other.size);
  }
};

int compare_size_wrappers( const void* x1, const void* x2 )
{
  return(((size_wrapper *)x1)->size - ((size_wrapper *)x2)->size);
}


int convert_entry( snd_info_t &entry, nslLanguageEnum which_lang )
{
  SNDFILE *aiff;
  SF_INFO aiff_info;
  char src_file[PATH_MAX];
  char path_addendum[PATH_MAX];
  char base_path[PATH_MAX];
  char base_file_name[PATH_MAX];
  short *in_buffer;
  char *buffer_ptr, *vag_ptr;
  int vag_buffer_bytes;

  assert(entry.ps2.vag_buffer[which_lang] == NULL);

  // find the right file
  path_addendum[0] = '\0';
  sprintf( base_path, "%s%s", opt.input_wav_dir, entry.path );
  sprintf( base_file_name, "%s%s", entry.name, AUDIO_IN_EXT );
  bool used_lang = false;

  FILE *the_file = localized_fopen( base_path, base_file_name, "rb", which_lang, NSL_PLATFORM_PS2, 
                                    path_addendum, &used_lang );

  if (the_file != NULL)
  {
    fclose(the_file);
  }
  else if ( opt.export_language[which_lang] )
  {
    if (which_lang != NSL_LANGUAGE_NONE)
      error( "Could not open aiff file: %s\n", base_file_name );
    return 0;
  }

  // open up our input file
  sprintf( src_file, "%s%s%s%s%s", opt.input_wav_dir, entry.path, path_addendum, entry.name, AUDIO_IN_EXT );
  
  aiff = sf_open_read( src_file, &aiff_info );
  if( aiff == NULL )
  {
    error( "Could not open aiff file: %s\n", src_file );
    return 0;
  }

  if( aiff_info.pcmbitwidth != 16 )
  {
    error( "File %s does not have 16-bit samples\n", src_file );
    sf_close( aiff );
    return 0;
  }
  // set stereo settings
  bool new_stereo = (bool)( aiff_info.channels == 2 );
  // This seeks in terms of multichannel samples.  not what we want
  entry.ps2.real_size[which_lang] = sf_seek(aiff, 0, SEEK_END) - sf_seek(aiff, 0, SEEK_SET)/sizeof(void *);
  
  // We have to adjust for stereo (*aiff_info.channels), bitwidth (*aiff_info.pcmbitwidth/8)
  // and compression (/ 3.5)

  entry.ps2.real_size[which_lang] = entry.ps2.real_size[which_lang] * 
                                    aiff_info.channels * 
                                    (aiff_info.pcmbitwidth/8) / 3.5;

  sf_seek(aiff, 0, SEEK_SET);
  if ( entry.ps2.processed && entry.ps2.stereo != new_stereo )
    fatal_error("non-matching stereo settings for language %s of sound %s", 
                nslLanguageStr[which_lang], entry.name);
  entry.ps2.stereo = new_stereo;
  if( aiff_info.channels == 1 )
  {
    int file_samples = aiff_info.samples;
    int file_bytes;
    int buffer_bytes;
    int buffer_samples;

    if( entry.loop )
    {
      const int div = REAL_BLK_SIZE / 2;
      file_samples = ( file_samples / div ) * div;
    }
    
    file_bytes = file_samples * 2;
    
    buffer_bytes = ( ( file_bytes + REAL_BLK_SIZE - 1 ) / REAL_BLK_SIZE ) * REAL_BLK_SIZE;
    buffer_samples = buffer_bytes / 2;
    
    in_buffer = (short *)malloc( buffer_bytes );
    memset( in_buffer, 0, buffer_bytes );

    sf_read_short( aiff, in_buffer, file_samples );

    vag_buffer_bytes = ( buffer_bytes / REAL_SPU_BLK_SIZE ) * SPU_BLK_SIZE ;
    entry.ps2.vag_buffer[which_lang] = (char *)malloc( vag_buffer_bytes );
    
    entry.ps2.padded_size[which_lang] = vag_buffer_bytes;
    if (entry.ps2.processed)
    {
      if ( entry.ps2.freq != aiff_info.samplerate )
        fatal_error("non-matching sample rate (%d) for language %s of sound %s, previous rate (%d)", 
                    aiff_info.samplerate, nslLanguageStr[which_lang], entry.name, entry.ps2.freq);
    }
    entry.ps2.freq = aiff_info.samplerate;

    buffer_ptr = (char *)in_buffer;
    vag_ptr = (char *)entry.ps2.vag_buffer[which_lang];

    EncVagInit( 1 );
    do
    {
      EncVag( (short *)buffer_ptr, (short *)vag_ptr, 3 );
      buffer_ptr += REAL_SPU_BLK_SIZE;
      vag_ptr += SPU_BLK_SIZE;
    } while( vag_ptr != ( entry.ps2.vag_buffer[which_lang] + vag_buffer_bytes ) );
  
    sf_close( aiff );
    free( in_buffer );

    entry.ps2.processed = true;
  }
  else if( aiff_info.channels == 2 )
  {
    //short *buffer_c;
    short *left_wav_buffer=NULL,  *right_wav_buffer=NULL, *source_wav=NULL;
    short *left_enc_buffer=NULL,  *right_enc_buffer=NULL;
    short *curr_left_wav=NULL,    *curr_right_wav=NULL, *curr_source_wav=NULL;
    short *curr_left_enc=NULL,    *curr_right_enc=NULL, *curr_vag_enc=NULL;
    int file_samples = aiff_info.samples;
    int file_bytes;
    int buffer_bytes, enc_buffer_bytes;
    if( entry.loop )
    {
      const int div = REAL_BLK_SIZE / 2;
      file_samples = ( file_samples / div ) * div;
    }
    file_bytes = file_samples * 2;

    buffer_bytes = ( ( file_bytes + REAL_BLK_SIZE - 1 ) / REAL_BLK_SIZE ) * REAL_BLK_SIZE;

    //in_buffer = (short *)malloc( buffer_bytes * 2 );
    right_wav_buffer = (short *)malloc( buffer_bytes );
    left_wav_buffer = (short *)malloc( buffer_bytes );
    source_wav = (short *)malloc( buffer_bytes * 2 );    
    memset( source_wav, 0, buffer_bytes * 2 );

    sf_read_short( aiff, source_wav, file_samples * 2 );
    curr_source_wav = source_wav;
    curr_left_wav   = left_wav_buffer;
    curr_right_wav  = right_wav_buffer;
    while( ((char *)curr_source_wav) < ((char *)source_wav) + buffer_bytes*2 )
    {
      *curr_left_wav  = *curr_source_wav;
      curr_source_wav++;
      curr_left_wav++;
      *curr_right_wav = *curr_source_wav;
      curr_source_wav++;
      curr_right_wav++;
    }
    
    free(source_wav);


    enc_buffer_bytes = ( buffer_bytes / REAL_SPU_BLK_SIZE ) * SPU_BLK_SIZE;

    // Left channel first
    left_enc_buffer = (short *)malloc(enc_buffer_bytes);
    curr_left_enc = left_enc_buffer;
    curr_left_wav   = left_wav_buffer;

    EncVagInit( 1 );
    do
    {
      EncVag( (short *)curr_left_wav, (short *)curr_left_enc, 3 );
      curr_left_wav += REAL_SPU_BLK_SIZE/2;
      curr_left_enc += SPU_BLK_SIZE/2;
    } while( ((char *)curr_left_enc) != ( ((char *)left_enc_buffer) + enc_buffer_bytes ) );
    
    free(left_wav_buffer);

    // Right channel second
    right_enc_buffer = (short *)malloc(enc_buffer_bytes);
    curr_right_enc = right_enc_buffer;
    curr_right_wav  = right_wav_buffer;

    EncVagInit( 1 );
    do
    {
      EncVag( (short *)curr_right_wav, (short *)curr_right_enc, 3 );
      curr_right_wav += REAL_SPU_BLK_SIZE/2;
      curr_right_enc += SPU_BLK_SIZE/2;
    } while( ((char *)curr_right_enc) != ( ((char *)right_enc_buffer) + enc_buffer_bytes ) );
    
    free(right_wav_buffer);

    // Interleave the left and right encoded channels
    // in 2k blocks
    vag_buffer_bytes = ( buffer_bytes * 2 / REAL_SPU_BLK_SIZE ) * SPU_BLK_SIZE;
    assert ( vag_buffer_bytes == 2 * enc_buffer_bytes);

    entry.ps2.vag_buffer[which_lang] = (char *)malloc(vag_buffer_bytes);
    
    curr_right_enc = right_enc_buffer;
    curr_left_enc  = left_enc_buffer;
    curr_vag_enc   = (short *)entry.ps2.vag_buffer[which_lang];

    // Start interleaving 2k chunks
    while (((char *)curr_vag_enc) != entry.ps2.vag_buffer[which_lang] + vag_buffer_bytes)
    {
      memcpy(curr_vag_enc, curr_left_enc, BLK_SIZE);
      // Both of these pointers are shorts, so divide 
      // BLK_SIZE by 2
      curr_vag_enc  += BLK_SIZE/2;
      curr_left_enc += BLK_SIZE/2;

      memcpy(curr_vag_enc, curr_right_enc, BLK_SIZE);
      // Both of these pointers are shorts, so divide 
      // BLK_SIZE by 2
      curr_vag_enc  += BLK_SIZE/2;
      curr_right_enc += BLK_SIZE/2;
    }

    free(left_enc_buffer);
    free(right_enc_buffer);

    if (entry.ps2.processed)
    {
      if ( entry.ps2.freq != aiff_info.samplerate )
        fatal_error("non-matching sample rate (%d) for language %s of sound %s, previous rate (%d)", 
                    aiff_info.samplerate, nslLanguageStr[which_lang], entry.name, entry.ps2.freq);
    }
    entry.ps2.freq = aiff_info.samplerate;
    entry.ps2.padded_size[which_lang] = vag_buffer_bytes;

    sf_close( aiff );
    entry.ps2.processed = true;
  }
  if (entry.ps2.padded_size[which_lang] < entry.ps2.real_size[which_lang])
     entry.ps2.real_size[which_lang] = entry.ps2.padded_size[which_lang];
  return vag_buffer_bytes;
}

void write_snd_line( snd_info_t &entry, FILE *list_out, int offset, 
                      int padded_size, int real_size, nslLanguageEnum lang = NSL_LANGUAGE_Z )
{
  if( padded_size == 0 ) return;
  char base_name[PATH_MAX];
  strcpy(base_name, entry.name);
  strupr(base_name);
  int voll = (int)(entry.volume_left * 100.0f);
  int volr = (int)(entry.volume_right * 100.0f);
  int pitch = (int)(entry.pitch * 1000.0f);

  if (lang != NSL_LANGUAGE_Z)
    fprintf( list_out, "%-9s ", nslLanguageStr[lang]);

 
#ifdef NSL_LOAD_SOURCE_BY_NAME
  fprintf( list_out, 
#ifdef NSL1
          "%-14s size %8d offset %8d freq %5d volume %3d ", 
#else
          "%-14s realsize %8d size %8d offset %8d freq %5d volume %3d ", 
#endif
          base_name,
#ifndef NSL1
          real_size,
#endif
          padded_size, 
          offset,
          entry.ps2.freq,
          voll
          );
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  fprintf( list_out, 
#ifdef NSL1
          "%d size %8d offset %8d freq %5d volume %3d ", 
#else
          "%d realsize %8d size %8d offset %8d freq %5d volume %3d ", 
#endif
          entry.aliasID,
#ifndef NSL1
          real_size,
#endif
          padded_size, 
          offset,
          entry.ps2.freq,
          voll
          );
#endif

  if (entry.spu && !entry.cd)
  {
    fprintf( list_out, "spu ");
  }
  else 
  {
    assert(entry.cd && !entry.spu);
    fprintf( list_out, " cd ");
  }

  if (entry.ps2.stereo)
  {
    fprintf( list_out, "stereo ");
  }
  else 
  {
    fprintf( list_out, " mono  ");
  }
  switch (entry.type)
  {
    case NSL_SOURCETYPE_SFX:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_SFX] ); 
      break;
    case NSL_SOURCETYPE_AMBIENT:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_AMBIENT] ); 
      break;
    case NSL_SOURCETYPE_VOICE:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_VOICE] ); 
      break;
    case NSL_SOURCETYPE_MUSIC:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_MUSIC] ); 
      break;
    case NSL_SOURCETYPE_MOVIE:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_MOVIE] ); 
      break;
    case NSL_SOURCETYPE_USER1:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_USER1] ); 
      break;
    case NSL_SOURCETYPE_USER2:    
      fprintf( list_out, " " ); 
      fprintf( list_out, nslSourceTypesStr[NSL_SOURCETYPE_USER2] ); 
      break;
    default:
      fprintf( list_out, " sfx " );
      break;

  }

  if (entry.loop)
  {
    fprintf( list_out, " loop ");
  }

#ifndef NSL1
  if (entry.applyReverb)
  {
    fprintf( list_out, " reverb ");
  }
#endif
  
  fprintf( list_out, "\n");
}

bool write_spu_entry( snd_info_t &entry, nslLanguageEnum lang, bool is_forced=false )
{
  // here we should just copy over the preconverted data into the spu image, if it fits
  read_preconvert_info_file( entry.ps2.info_file_name[lang], entry, lang, !opt.emulation_export_only );

  if ( !opt.emulation_export_only && entry.ps2.vag_buffer[lang] == NULL )
  {
    error("Unable to find %s language version of sound %s to export\n", nslLanguageStr[lang], entry.name);
    return false;
  }

  if (  entry.cd == true 
     || entry.ps2.padded_size[lang] >= ps2_info.spu_mem_free[lang]
     || entry.ps2.stereo == true 
     )
  {
    entry.cd = true;
	if(is_forced)
	    error("--out of spu ram; ignoring %s\n", entry.name);
    // bail out without writing if we're doing an emulation export
    if (opt.emulation_export_only)
      return true;
  }
  else
  {
    ps2_info.spu_mem_free[lang] -= entry.ps2.padded_size[lang];

    // write out this sound wave
    int offset = ps2_info.spu_offset[lang];
    ps2_info.spu_offset[lang] += entry.ps2.padded_size[lang];

    // bail out without writing if we're doing an emulation export
    if (opt.emulation_export_only)
      return true;

    if (entry.loop) 
    {
      _AdpcmSetMarkLOOP(entry.ps2.vag_buffer[lang], entry.ps2.padded_size[lang]);
    }
    else 
    {
      _AdpcmSetMarkSTOP(entry.ps2.vag_buffer[lang], entry.ps2.padded_size[lang]);
    }

    fwrite( entry.ps2.vag_buffer[lang], 1, entry.ps2.padded_size[lang], ps2_info.spu_file[lang] );

    // now write this line to the SND file
    entry.spu = true;
    assert( entry.cd == false);
    write_snd_line( entry, ps2_info.snd_file[lang], offset, entry.ps2.padded_size[lang], entry.ps2.real_size[lang] );

    if (opt.ultra_verbose)
      message( "%s: Written to spu, spu ram remaining %d\n", entry.name, ps2_info.spu_mem_free[lang] );
  }
  // now that we've written it, dump the VAG data from memory.
  free( entry.ps2.vag_buffer[lang] );
  entry.ps2.vag_buffer[lang] = NULL;
  entry.ps2.padded_size[lang] = 0;

  return true;
}

int write_cd_entry( snd_info_t &entry, nslLanguageEnum lang )
{
  // here we should verify that it hasn't already been exported to the stream VBC for this
  // language, and if it hasn't copy over the preconverted data into the stream VBC
  int offset = -1;

  // bail out without writing if we're doing an emulation export
  if (opt.emulation_export_only)
    return 0;

  vector<snd_info_t>::iterator it = find( ps2_info.cd_entries.begin(), ps2_info.cd_entries.end(), entry );
  snd_info_t * use_this = NULL;
  if ( it == ps2_info.cd_entries.end() )
  {
    // let's add it
    ps2_info.cd_entries.push_back(entry);
    use_this = &ps2_info.cd_entries.back();
    for (int i=0; i<NSL_LANGUAGE_Z; ++i)
      use_this->languages[i] = false;
  }
  else
  {
    use_this = &(*it);
  }

  int  result = 0;
  char found_file_name[PATH_MAX];
  char path_addendum[PATH_MAX];
  char base_path[PATH_MAX];
  char base_file_name[PATH_MAX];

  // find the right file
  path_addendum[0] = '\0';
  found_file_name[0] = '\0';
  bool used_lang= false;

  sprintf( base_path, "%s%s", opt.input_wav_dir, use_this->path );
  sprintf( base_file_name, "%s%s", use_this->name, AUDIO_INFO_EXT );

  result = _nslLocalizedPath( base_path, base_file_name, lang, NSL_PLATFORM_PS2, 
                              found_file_name, path_addendum, &used_lang );

  bool write_out_new_one = false;
  nslLanguageEnum virtual_lang = lang;
  if ( used_lang == false )
  {
    virtual_lang = NSL_LANGUAGE_NONE;
    if ( use_this->languages[NSL_LANGUAGE_NONE] == false )
    {
      write_out_new_one = true;
    }
  }
  else if ( use_this->languages[lang] == false )
  {
    write_out_new_one = true;
  }

  if (write_out_new_one)
  {
    // look for the file on disk
    read_preconvert_info_file( found_file_name, *use_this, virtual_lang, true );

    if ( use_this->ps2.vag_buffer[virtual_lang] == NULL )
    {
      error( "Sound %s is missing preconverted data.  what's the deal there?\n", use_this->name );
      return 0;
    }
  
    // write out this sound wave
    use_this->ps2.cd_offset[virtual_lang] = ps2_info.cd_offset;
    ps2_info.cd_offset += use_this->ps2.padded_size[virtual_lang];
    fwrite( use_this->ps2.vag_buffer[virtual_lang], 1, use_this->ps2.padded_size[virtual_lang], ps2_info.cd_file );

    // now that we've written it, dump the VAG data from memory.
    free( use_this->ps2.vag_buffer[virtual_lang] );
    use_this->ps2.vag_buffer[virtual_lang] = NULL;

    if ( use_this->ps2.freq > 24000 && use_this->ps2.stereo != true )
    {
      error( "Sound %s, is mono cd streaming (size %d bytes)\n\tbut has a sampling rate of %d, which is over our 24khz limit.\n\n",
             use_this->name, use_this->ps2.padded_size[virtual_lang], use_this->ps2.freq );
    }
    if ( use_this->ps2.freq == 32000 )
    {
      fatal_error( "Sound %s has an unsupported sampling rate of %d.\n",
                   use_this->name, use_this->ps2.freq );
    }

    if ( ps2_info.cd_snd_file )
    {
      use_this->cd = true;
      assert( use_this->spu == false);
      write_snd_line( *use_this, ps2_info.cd_snd_file, use_this->ps2.cd_offset[virtual_lang], 
        use_this->ps2.padded_size[virtual_lang], use_this->ps2.real_size[virtual_lang], virtual_lang );
    }
    if (opt.ultra_verbose)
      message( "%s: Written to cd, for language (%s)\n", entry.name, nslLanguageStr[lang] );

    use_this->languages[virtual_lang] = true;
  }

  int size = -1;
  int realSize = -1;

  offset = use_this->ps2.cd_offset[virtual_lang];
  size = use_this->ps2.padded_size[virtual_lang];
  realSize = use_this->ps2.real_size[virtual_lang];
  if (offset < 0)
  {
    fatal_error("Unable to find %s language (or general) version of sound %s to export", nslLanguageStr[lang], use_this->name);
  }

  // now write this line to the SND file
  use_this->cd = true;
  assert( use_this->spu == false);
  write_snd_line( *use_this, ps2_info.snd_file[lang], offset, size, realSize  );

  return offset;
}



int end_level_ps2( const char* filename )
{
  int lang;

  // ok now all of our level entries for each language have complete lists of files to export
  // so the first thing we want to do is make sure they are all up to date
  for ( lang=NSL_LANGUAGE_NONE+1; lang<NSL_LANGUAGE_Z; ++lang )
  {
    if ( opt.export_language[lang] == false )
      continue;
    // now loop thru all the sounds for this level and possibly reconvert them
    vector<snd_info_t>::iterator it = ps2_info.level_entries[lang].begin();
    vector<snd_info_t>::iterator it_end = ps2_info.level_entries[lang].end();
    while ( it != it_end )
    {
      write_preconverted_file( (*it), (nslLanguageEnum)lang );
      ++it;
    }

    // now we write out a new SPU image (if allowed)
    char snd_filename[PATH_MAX];
    char spu_collection[PATH_MAX];
    char snd_path[PATH_MAX];
    char spu_path[PATH_MAX];

    // create the output snd/spu files
    sprintf( snd_path, "%s\\", opt.ps2_out_dir );
    mkdir( snd_path );
    strcat( snd_path, nslLanguageStr[lang] );
    strcat( snd_path, "\\" );
    mkdir( snd_path );
    strcat( snd_path, ps2_info.level_dir );
    strcat( snd_path, "\\" );
    mkdir( snd_path );
    sprintf( snd_path, "%s\\%s\\%s\\", opt.ps2_out_dir, nslLanguageStr[lang], ps2_info.level_dir );
    sprintf( spu_path, "%s\\%s\\%s\\%s\\", opt.ps2_out_dir, nslLanguageStr[lang], ps2_info.level_dir, opt.ps2_spu_dir );
    sprintf( snd_filename, "%s.snd", ps2_info.level_name );
    sprintf( spu_collection, "%s.vbc", ps2_info.level_name );

    if( opt.dryrun )
    {
      ps2_info.snd_file[lang] = stdout;
    }
    else
    {
      mkdir( snd_path );
      mkdir( spu_path );

      if( !opt.emulation_export_only )
      {
        ps2_info.snd_file[lang] = localized_fopen( snd_path, snd_filename, "w", (nslLanguageEnum)lang, NSL_PLATFORM_PS2 );
        ps2_info.spu_file[lang] = localized_fopen( spu_path, spu_collection, "wb", (nslLanguageEnum)lang, NSL_PLATFORM_PS2 );

        if( ps2_info.snd_file[lang] == NULL )
        {
          printf( "Error opening file %s\\%s\n", spu_path, spu_collection );
          exit(-1);
        }
        fprintf( ps2_info.snd_file[lang], ";WARNING WARNING!!\n;DO NOT EDIT THIS FILE!\n;THIS FILE IS GENERATED BY ADPCMTOOL\n" );

        if( ps2_info.spu_file[lang] == NULL )
        {
          printf( "Error opening file %s\\%s\n", spu_path, spu_collection );
          exit(-1);
        }
      }
    }
  }

  // next we need to calculate which files can fit in spu ram
  for ( lang=NSL_LANGUAGE_NONE+1; lang<NSL_LANGUAGE_Z; ++lang )
  {
    if ( opt.export_language[lang] == false )
      continue;

    vector<size_wrapper> sorted_entries;
    vector<size_wrapper> forced_spu_entries;

    // now loop thru all the sounds for this level and possibly reconvert them
    vector<snd_info_t>::iterator it = ps2_info.level_entries[lang].begin();
    vector<snd_info_t>::iterator it_end = ps2_info.level_entries[lang].end();
    int i = 0;
    while ( it != it_end )
    {
      size_wrapper sw;
      sw.size = (*it).ps2.padded_size[lang];
      sw.index = i;

      if (sw.size > 0)
      {
        if ((*it).spu)
          forced_spu_entries.push_back(sw);
        else
          sorted_entries.push_back(sw);
      }
      else
      {
        error("Zero-length sound file, skipping export of %s.\n", (*it).name);
      }

      ++i;
      ++it;
    }

    // sort them by size
    int size = sorted_entries.size();
    if (size > 1)
    {
      qsort( &(*sorted_entries.begin()), size, sizeof(size_wrapper), compare_size_wrappers );
    }

    // add all forced spu files first
    vector<size_wrapper>::iterator f_it;
    vector<size_wrapper>::iterator f_it_end = forced_spu_entries.end();

    for ( f_it = forced_spu_entries.begin(); f_it != f_it_end; ++f_it )
    {
      write_spu_entry( ps2_info.level_entries[lang][(*f_it).index], (nslLanguageEnum)lang, true );
    }

    // now pack as many in as we can
    vector<size_wrapper>::iterator s_it;
    vector<size_wrapper>::iterator s_it_end = sorted_entries.end();

    for ( s_it = sorted_entries.begin(); s_it != s_it_end; ++s_it )
    {
      if ( ps2_info.level_entries[lang][(*s_it).index].cd == false )
      {
        write_spu_entry( ps2_info.level_entries[lang][(*s_it).index], (nslLanguageEnum)lang );
      }
    }

    // write out forced CD entries last, reset s_it 
    // this is due to some kludgy code MJD wrote in GAS that I don't feel like touching.
    for ( s_it = sorted_entries.begin(); s_it != s_it_end; ++s_it )
    {
      if ( ps2_info.level_entries[lang][(*s_it).index].cd )
      {
        write_cd_entry( ps2_info.level_entries[lang][(*s_it).index], (nslLanguageEnum)lang );
      }
    }

    message( "\nLEVEL %22s\n  LANGUAGE: %16s\n  SPU Mem Used: %12d\n  SPU Mem Free: %12d\n",
            ps2_info.level_name,
            nslLanguageStr[lang], 
            (g_spu_heap_end - g_spu_heap_start) - ps2_info.spu_mem_free[lang],
            ps2_info.spu_mem_free[lang] );

    fprintf( ps2_info.snd_file[lang], "COLLECTION %s\\%s\\%s\\%s\\%s.VBC\n", // %ld\n", 
             opt.ps2_collection_dir, nslLanguageStr[lang], ps2_info.level_dir, opt.ps2_spu_dir, ps2_info.level_name /*, TIMESTAMP_WHEN_EXPORTED*/ );
    fclose(ps2_info.snd_file[lang]);
    ps2_info.snd_file[lang] = NULL;

    fclose(ps2_info.spu_file[lang]);
    ps2_info.spu_file[lang] = NULL;

    // next we optionally write out a report for the PC to use in emulating the ps2
  }
	return 0;
}

int export_file_ps2( snd_info_t* info )
{
  int lang;
  for ( lang=NSL_LANGUAGE_NONE+1; lang<NSL_LANGUAGE_Z; ++lang )
  {
    // if the language is being exported, add this file to this level for this language
    if ( opt.export_language[lang] && info->languages[lang] )
    {
      // check if it's already there
      vector<snd_info_t>::iterator it = find( ps2_info.level_entries[lang].begin(), ps2_info.level_entries[lang].end(), *info );
      if ( it == ps2_info.level_entries[lang].end())
      {
        ps2_info.level_entries[lang].push_back( *info );
      }
    }
  }
	return 0;
}
