
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>

#include "NSLSoundTool.h"
#include "sndfile.h"
#include "imaadpcm.h"
#include "wavparse.h"


#define STRING_LENGTH 128
#define MAX_LINES 600

#define XBOX_SOUND_ALIGN 32

// Version 0.99 - still in development
// Anytime format changes this should change too
// so that NSL can detect any inconsistencies
// static const DWORD XBoxVersion = 0x009A;
static char lines[MAX_LINES][STRING_LENGTH];

// extern "C" { int process_line(char *line, GasSource *entry); }

// int clean_filename( char * filename );

static char * XBuf;
nslXBoxSndEntry BankHdr[NSL_LANGUAGE_Z][MAX_LINES];
static DWORD XBufNum[NSL_LANGUAGE_Z];
static DWORD XCurOfs[NSL_LANGUAGE_Z] = {0};
static DWORD XCurStreamOfs[NSL_LANGUAGE_Z] = {0};

const int XBOX_ADPCM_SAMPLES_PER_BLOCK = 64;

char xbox_spu_streams[NSL_LANGUAGE_Z][256];
char xbox_spu_header[NSL_LANGUAGE_Z][256];
char xbox_spu_collection[NSL_LANGUAGE_Z][256];

// FILE * fileStream;
FILE * fileHdr[NSL_LANGUAGE_Z];
FILE * fileBank[NSL_LANGUAGE_Z];
FILE * fileStreams[NSL_LANGUAGE_Z];

int XCachedNum[NSL_LANGUAGE_Z] = {0};
nslXBoxSndEntry XCached[MAX_LINES][NSL_LANGUAGE_Z];
static char XCachedName[MAX_LINES][NSL_LANGUAGE_Z][STRING_LENGTH];

void * NewXBuf( int size, nslLanguageEnum which_lang )
{
  void * p = malloc( size );
  XBuf = (char *)p;
  ++XBufNum[which_lang];
  return p;
}

void ReleaseXBuf()
{
  free( XBuf );
  XBuf = NULL;
  memset( &XCurOfs, 0, sizeof(XCurOfs) );
}

// ExportStreams - Exports the streams for ALL levels
// During exporting levels all streaming sounds are collected
// This function MUST be called at the end to export the streamed sounds
// outputHeader - the output XBox specific header file
// outputBank - the output XBox specific bank file
BOOL ExportStreams(const char * outputHeader, const char * outputBank)
{
  return FALSE;
}

int LoadAndConvertXBoxFile( snd_info_t* info, nslLanguageEnum which_lang )
{
  SNDFILE *aiff;
  SF_INFO aiff_info;
  char src_file[256];
  short *in_buffer;
  CImaAdpcmCodec codec;

/*
	char tmp_file[256];
	FILE *tmp_fp;
  strcpy( tmp_file, opt.input_wav_dir );
  strcat( tmp_file, info->path );
	strcat( tmp_file, "xbox\\" );
  strcat( tmp_file, info->name );
  strcat( tmp_file, ".wav" );
	
	if(tmp_fp = fopen(tmp_file, "r"))
	{
		strcpy( src_file, tmp_file );
		fclose(tmp_fp);
	}
	else
	{
		strcpy( src_file, opt.input_wav_dir );
		strcat( src_file, info->path );
		strcat( src_file, info->name );
		strcat( src_file, ".wav" );			
	}
*/
  
  if ( fileBank[which_lang] == NULL )
  {
    fatal_error( "Stream file wasn't open!!!\n" );
    return 0;
  }
  
  char file_path[256];
  char file_name[256];
  char path_addendum[256];

  strcpy( file_path, opt.input_wav_dir );
  strcat( file_path, info->path );
  strcpy( file_name, info->name );
  strcat( file_name, ".wav" );
  path_addendum[0] = 0;
  FILE * ftest = localized_fopen( file_path, file_name, "rb", which_lang, NSL_PLATFORM_XBOX, path_addendum, NULL, NULL );
  if (ftest)
  {
    fclose(ftest);
  }
  else
  {
    error( "Could not open file %s (specific language: %s)!\n", file_name, nslLanguageStr[ which_lang ] );
    return 0;
  }

  strcpy( src_file, file_path );
  strcat( src_file, path_addendum );
  strcat( src_file, file_name );
  aiff = sf_open_read( src_file, &aiff_info );

  if( aiff == NULL )
  {
    printf( "Could not open aiff file: %s\n", src_file );
    return 0;
  }

  if( aiff_info.pcmbitwidth != 16 )
  {
    sf_close( aiff );
    fatal_error( "%s in language %s must be a 16-bit sample!\n", file_name, nslLanguageStr[ which_lang ] );
  }
  
//  entry.flag.stereo = ( aiff_info.channels == 2 ? 1 : 0 );

  int file_samples = aiff_info.samples;
  int buffer_bytes;
  int sample_bytes;

  if( aiff_info.channels == 1 )
  {
    sample_bytes = buffer_bytes = file_samples * 2;
  }
  else
  {
    buffer_bytes = file_samples * 4;
  }

  in_buffer = (short*)NewXBuf( buffer_bytes, which_lang );
  memset( in_buffer, 0, buffer_bytes );

  // Set the header for that file
	nslXBoxSndEntry &theHeader = BankHdr[which_lang][XBufNum[which_lang]-1];

#ifdef NSL_LOAD_SOURCE_BY_NAME
	char *leanName;
	leanName = strrchr(info->name, '\\');
	if(leanName)
		sprintf(theHeader.fileName, ++leanName);
	else
		sprintf(theHeader.fileName, info->name);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  theHeader.aliasID = info->aliasID;
#endif

  sf_read_short( aiff, in_buffer, aiff_info.channels * file_samples );
  
  IMAADPCMWAVEFORMAT adpcmWaveFmt;
  codec.CreateImaAdpcmFormat( aiff_info.channels, aiff_info.samplerate, XBOX_ADPCM_SAMPLES_PER_BLOCK, &adpcmWaveFmt );

  codec.Initialize( &adpcmWaveFmt, TRUE );
  memcpy( &theHeader.format, &adpcmWaveFmt.wfx, sizeof(theHeader.format) );

  int adpcmBytes;
  int adpcmBlocks;
  char * adpcmBuffer;

  // Calculate number of ADPCM blocks and length of ADPCM data
  adpcmBlocks = aiff_info.samples / XBOX_ADPCM_SAMPLES_PER_BLOCK;   // Align to 64 samples
  adpcmBytes = adpcmBlocks * theHeader.format.nBlockAlign;      // Find the required buffer size
	
  adpcmBuffer = (char *)malloc( adpcmBytes );
	memset(adpcmBuffer, 0, adpcmBytes);
  codec.Convert( in_buffer, adpcmBuffer, adpcmBlocks );
  free( in_buffer );
  in_buffer = (short *)adpcmBuffer;
  XBuf = adpcmBuffer;

	// Fill in the header info
  if ( aiff_info.channels == 1 )
  {
    theHeader.channels = 1;
  }
  else
  {
    theHeader.channels = 2;
	}
	
  theHeader.flags = 0;

  if (info->applyReverb)
  {
	  theHeader.flags |= XBOX_SND_FLAGS_REVERB;
  }
	
  if ( info->loop )
  {
    theHeader.flags |= XBOX_SND_FLAGS_LOOPED;
  }
	
//  if ( !info->cd )
//  {
    theHeader.offset = XCurOfs[which_lang];
    XCurOfs[which_lang] += adpcmBytes;
//  }
//  else
//  {
//    theHeader.flags |= XBOX_SND_FLAGS_STREAM;
//    theHeader.offset = XCurStreamOfs[which_lang];
//    XCurStreamOfs[which_lang] += adpcmBytes;
//  }

  theHeader.size = adpcmBytes;

	theHeader.samples = aiff_info.samples;

	theHeader.type = info->type;
	theHeader.params.volume = info->volume_left;
	assert(info->volume_left == info->volume_right); // we no longer support seperate L and R volumes
	theHeader.params.pitch = info->pitch;
	theHeader.params.minDist = NSL_DEFAULT_MIN_SOUND_DIST;
	theHeader.params.maxDist = NSL_DEFAULT_MAX_SOUND_DIST;

  sf_close( aiff );

  return 1;
}

int LoadAndConvertStream( snd_info_t* info, nslLanguageEnum which_lang )
{
  char src_file[256];
  char dest_file[256];

  SNDFILE *aiff;
  SF_INFO aiff_info;
  short *in_buffer;

  CImaAdpcmCodec codec;
  CWaveFile wfDest;               // Destination file
  //BYTE * pbSampleData = NULL;     // Source data
  BYTE * pbEncodedData = NULL;    // Compressed data
  IMAADPCMWAVEFORMAT wfxEncode;   // Destination format
  // BOOL bHadWaveSample;            // TRUE if source had a wave sample chunk
  // WAVESAMPLE ws;                  // WaveSample chunk
  // WAVESAMPLE_LOOP wsl;            // WaveSample loop

/*
  strcpy( src_file, opt.input_wav_dir );
  strcat( src_file, info->path );
  strcat( src_file, info->name );
  strcat( src_file, ".wav" );
*/
  if ( fileStreams[which_lang] == NULL )
  {
    fatal_error( "Bank file wasn't open!!!\n" );
    return 0;
  }
  
  char file_path[256];
  char file_name[256];
  char path_addendum[256];

  strcpy( file_path, opt.input_wav_dir );
  strcat( file_path, info->path );
  strcpy( file_name, info->name );
  strcat( file_name, ".wav" );
  path_addendum[0] = 0;
  FILE * ftest = localized_fopen( file_path, file_name, "rb", which_lang, NSL_PLATFORM_XBOX, path_addendum, NULL, NULL );
  if (ftest)
  {
    fclose(ftest);
  }
  else
  {
    error( "Could not open file %s (specific language: %s)!\n", file_name, nslLanguageStr[ which_lang ] );
    return 0;
  }

  strcpy( src_file, file_path );
  strcat( src_file, path_addendum );
  strcat( src_file, file_name );

  strcpy( dest_file, opt.xbox_out_dir );
  if ( which_lang != NSL_LANGUAGE_NONE )
  {
    strcat( dest_file, "\\" );
    strcat( dest_file, nslLanguageStr[ which_lang ] );
    strcat( dest_file, "\\" );
  }

#ifdef NSL_LOAD_SOURCE_BY_NAME
  strcat( dest_file, info->name );
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  char tmp[12];
  sprintf( tmp, "%d", info->aliasID );
  strcat( dest_file, tmp );
#endif

  strcat( dest_file, ".xss" );
  
  // Set the header for that file
  XBuf = NULL;

	nslXBoxSndEntry &theHeader = BankHdr[which_lang][XBufNum[which_lang] - 1];

#ifdef NSL_LOAD_SOURCE_BY_NAME
	char *leanName;
	leanName = strrchr(info->name, '\\');
	if(leanName)
		sprintf(theHeader.fileName, ++leanName);
	else
		sprintf(theHeader.fileName, info->name);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  theHeader.aliasID = info->aliasID;
#endif

  theHeader.flags |= XBOX_SND_FLAGS_STREAM;

  if (info->applyReverb)
  {
	  theHeader.flags |= XBOX_SND_FLAGS_REVERB;
  }
	
  if ( info->loop )
  {
    theHeader.flags |= XBOX_SND_FLAGS_LOOPED;
  }
	 
  aiff = sf_open_read( src_file, &aiff_info );

  if( aiff == NULL )
  {
    printf( "Could not open aiff file: %s\n", src_file );
    return 0;
  }

  if( aiff_info.pcmbitwidth != 16 )
  {
    printf( "Must be a 16-bit sample\n" );
    sf_close( aiff );
    return 0;
  }

  int file_samples = aiff_info.samples;
  int buffer_bytes;

  if( aiff_info.channels == 1 )
  {
    buffer_bytes = file_samples * 2;
  }
  else
  {
    buffer_bytes = file_samples * 4;
  }

  in_buffer = (short*)malloc( buffer_bytes );
  memset( in_buffer, 0, buffer_bytes );

  sf_read_short( aiff, in_buffer, aiff_info.channels * file_samples );
  
  // Create an APDCM format structure based off the source format
  codec.CreateImaAdpcmFormat( aiff_info.channels, aiff_info.samplerate, XBOX_ADPCM_SAMPLES_PER_BLOCK, &wfxEncode );
  memcpy( &theHeader.format, &wfxEncode.wfx, sizeof(theHeader.format) );

  // Calculate number of ADPCM blocks and length of ADPCM data
  DWORD dwDstBlocks = aiff_info.samples / XBOX_ADPCM_SAMPLES_PER_BLOCK;
  DWORD dwDestLength = dwDstBlocks * wfxEncode.wfx.nBlockAlign;

  // Allocate a buffer for encoded data
  pbEncodedData = new BYTE[ dwDestLength ];

  // Initialize the codec
  if( FALSE == codec.Initialize( &wfxEncode, TRUE ) )
  {
    printf( "Couldn't initialize codec.\n" );
    free( in_buffer );
    return 0;
  }

  // Convert the data
  if( FALSE == codec.Convert( in_buffer, pbEncodedData, dwDstBlocks ) )
  {
    printf( "Codec failed.\n" );
    free( in_buffer );
    return 0;
  }

/*
  // Write the output file with the encoded data
  if( FAILED( wfDest.Create( dest_file, 
                             (WAVEFORMATEX *)&wfxEncode, 
                             sizeof( IMAADPCMWAVEFORMAT ), 
                             pbEncodedData, 
                             dwDestLength,
                             NULL,
                             NULL ) ) )
  {
    printf( "Couldn't open output file %s\n", dest_file );
    free( in_buffer );
    return 0;
  }
*/

  if ( fileStreams[which_lang] )
  {
    fwrite( pbEncodedData, dwDestLength, 1, fileStreams[which_lang] );
  }

  // Clean up
  delete[] pbEncodedData;

  free( in_buffer );

	// Fill in the header info
  theHeader.flags |= XBOX_SND_FLAGS_STREAM;
	
  if ( info->loop )
  {
    theHeader.flags |= XBOX_SND_FLAGS_LOOPED;
  }
	
  if ( aiff_info.channels == 1 )
  {
    theHeader.channels = 1;
  }
  else
  {
    theHeader.channels = 2;
	}

	theHeader.samples = aiff_info.samples;

	theHeader.type = info->type;
	theHeader.params.volume = info->volume_left;
	assert(info->volume_left == info->volume_right); // we no longer support seperate L and R volumes
	theHeader.params.pitch = info->pitch;
	theHeader.params.minDist = NSL_DEFAULT_MIN_SOUND_DIST;
	theHeader.params.maxDist = NSL_DEFAULT_MAX_SOUND_DIST;
  theHeader.offset = XCurStreamOfs[which_lang];
  XCurStreamOfs[which_lang] += dwDestLength;
  theHeader.size = dwDestLength;
	
  if ( (XCurStreamOfs[which_lang] % 4096) != 0 )
  {
    char buf[4096];
    int size;
    size = 4096 - (XCurStreamOfs[which_lang] % 4096);
    fwrite( buf, size, 1, fileStreams[which_lang] );
    XCurStreamOfs[which_lang] += size;
  }

  return 1;
}

int pre_export_xbox( void )
{
  memset( &XCurStreamOfs, 0, sizeof(XCurStreamOfs) );

	int i;

  for ( i = NSL_LANGUAGE_NONE; i < NSL_LANGUAGE_Z; ++i )
  {
    if ( opt.export_language[i] )
    {
      if (i == NSL_LANGUAGE_NONE)
      {
        continue;
      }
      else
      {
	      sprintf( xbox_spu_streams[i], "%s\\%s\\%s", opt.xbox_out_dir, nslLanguageStr[i], STREAM_FILENAME );
      }

      fileStreams[i] = fopen( xbox_spu_streams[i], "wb" );
    }
    else
    {
      fileStreams[i] = NULL;
    }
  }

//	sprintf( outputStream, "%s\\%s%s", opt.xbox_out_dir, "stream", NSL_XBOX_SND_BANK_EXT );

//  fileStream = fopen( outputStream, "wb" );

	return 0;
}

int post_export_xbox( void )
{
  // Export one by one the streamed sounds
  // (the whole file could get pretty big so we can't afford to keep it in RAM)

  int i;

  for ( i = NSL_LANGUAGE_NONE; i < NSL_LANGUAGE_Z; ++i )
  {
    if (i == NSL_LANGUAGE_NONE)
      continue;

    if ( opt.export_language[i] )
    {
      fclose( fileStreams[i] );
		}
  }   
//  fclose( fileStream );

	return 0;
}

int start_level_xbox( const char* level )
{
  int i;

  for ( i = NSL_LANGUAGE_NONE; i < NSL_LANGUAGE_Z; ++i )
  {
    if ( opt.export_language[i] )
    {
      if (i == NSL_LANGUAGE_NONE)
      {
        fileBank[i] = NULL;
        continue;

	      // sprintf( xbox_spu_header[i], "%s\\%s%s", opt.xbox_out_dir, level, NSL_XBOX_SND_HEADER_EXT );
	      // sprintf( xbox_spu_collection[i], "%s\\%s%s", opt.xbox_out_dir, level, NSL_XBOX_SND_BANK_EXT );
      }
      else
      {
	      sprintf( xbox_spu_header[i], "%s\\%s\\%s%s", opt.xbox_out_dir, nslLanguageStr[i], level, NSL_XBOX_SND_HEADER_EXT );
	      sprintf( xbox_spu_collection[i], "%s\\%s\\%s%s", opt.xbox_out_dir, nslLanguageStr[i], level, NSL_XBOX_SND_BANK_EXT );
	      sprintf( xbox_spu_streams[i], "%s\\%s\\%s%s", opt.xbox_out_dir, nslLanguageStr[i], level, NSL_XBOX_STREAM_SND_EXT );
      }

      ensure_dir( xbox_spu_collection[i] );
      fileBank[i] = fopen( xbox_spu_collection[i], "wb" );
      // fileStreams[i] = fopen( xbox_spu_streams[i], "wb" );
    }
    else
    {
      fileBank[i] = NULL;
      fileStreams[i] = NULL;
    }
  }

  XBuf = NULL;
  memset(&XBufNum, 0, sizeof(XBufNum));
  ReleaseXBuf();

	return 0;
}

bool XIsCached( nslLanguageEnum which_lang, char * filename, nslXBoxSndEntry * entry )
{
  int i;

  for ( i = 0; i < XCachedNum[which_lang]; i++ )
  {
    if ( stricmp( filename, XCachedName[i][which_lang] ) == 0 )
    {
      memcpy( entry, &XCached[i][which_lang], sizeof(nslXBoxSndEntry) );
      return true;
    }
  }

  return false;
}

void XAddToCache( nslLanguageEnum which_lang, char * filename, nslXBoxSndEntry * entry )
{
  assert( XCachedNum[which_lang] < MAX_LINES );
  strcpy( XCachedName[XCachedNum[which_lang]][which_lang], filename );
  memcpy( &XCached[XCachedNum[which_lang]][which_lang], entry, sizeof(nslXBoxSndEntry) );
  XCachedNum[which_lang]++;
}

int export_file_xbox_lang( snd_info_t* info, nslLanguageEnum which_lang )
{
  char filename[255];

  strcpy( filename, info->path );
  strcat( filename, info->name );

	// I'm forcing VOICE and SFX to be not streamed for now until we have a better streamming system on Xbox. - AC
  if ( info->cd /*&& !(info->type == NSL_SOURCETYPE_VOICE) && !(info->type == NSL_SOURCETYPE_SFX)*/ )
  {
    // A stream
    XBufNum[which_lang]++;
		
    if ( !XIsCached( which_lang, filename, &BankHdr[which_lang][XBufNum[which_lang] - 1] ) )
    {
      LoadAndConvertStream( info, which_lang );
			
      // Add it and export it if not
      XAddToCache( which_lang, filename, &BankHdr[which_lang][XBufNum[which_lang] - 1] );
    }		
  }
  else
  {
    // Not a stream
    LoadAndConvertXBoxFile( info, which_lang );
		
    // Export into bank
    if ( fileBank[which_lang] != NULL )
    {
			if(XBuf)
				fwrite( XBuf, 1, BankHdr[which_lang][XBufNum[which_lang] - 1].size, fileBank[ which_lang ] );
			else
				error( "Did not write %s to the bank...\n", info->name );
    }
		
    free( XBuf );	
  }

  XBuf = NULL;

	return 0;
}

int export_file_xbox( snd_info_t* info )
{
	message( "exporting '%s'\n", info->name );

  for ( int which_lang = NSL_LANGUAGE_NONE; which_lang < NSL_LANGUAGE_Z; ++which_lang )
  {
    if (which_lang == NSL_LANGUAGE_NONE)
      continue;

    if ( info->languages[ which_lang ] )
    {
      export_file_xbox_lang( info, (nslLanguageEnum) which_lang );
    }
  }
  
  return 0;
}

int end_level_xbox( const char* level )
{
	message( "finalizing for '%s'...\n", level );

  int i;

  for ( i = NSL_LANGUAGE_NONE; i < NSL_LANGUAGE_Z; ++i )
  {
    if (i == NSL_LANGUAGE_NONE)
      continue;

    if ( opt.export_language[i] )
    {
      // Export header
      fileHdr[i] = fopen( xbox_spu_header[i], "wb" );
	    nslXBoxSndHeader bankHeader;
	    bankHeader.version = nslXBoxSndVersion;
	    bankHeader.reserved = 0;
	    bankHeader.entryCnt = XBufNum[i];

      if ( fileHdr[i] != NULL )
      {
        fwrite( &bankHeader, sizeof(nslXBoxSndHeader), 1, fileHdr[i] );
        fwrite( BankHdr[i], sizeof(nslXBoxSndEntry), XBufNum[i], fileHdr[i] );
        fclose( fileHdr[i] );
      }

      if ( fileBank[i] )
      {
        fclose( fileBank[i] );
      }
    }
  }   

	return 0;
}
