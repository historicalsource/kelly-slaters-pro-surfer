#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <list>
#include <algorithm>

#include <windows.h>

#include "sndfile.h"

#include "NSLSoundTool.h"

#include "../libWave/libWave.h"

#pragma pack(push,1)

typedef struct _gc_snd_header_t {
	char tag[4];
	unsigned int version;
	char gsw_name[32];
	unsigned int num_entries;

	unsigned char pad[20];
} gc_snd_header_t; // sizeof( gc_snd_header_t ) == 64

#define GC_SND_VERSION 0x00030003 

typedef struct {
	// start context
	unsigned short coef[16];
	unsigned short gain;
	unsigned short pred_scale;
	unsigned short yn1;
	unsigned short yn2;

	// loop context
	unsigned short loop_pred_scale;
	unsigned short loop_yn1;
	unsigned short loop_yn2;

	unsigned short pad0;
} ADPCMINFO; // sizeof( ADPCMINFO ) == 48

typedef struct _gc_snd_info_t {
	unsigned int alias;
	char name[180];

	unsigned int flags;
	unsigned int offset;
	unsigned int samples;
	unsigned int frequency;
	unsigned int type;
	float volume;

	ADPCMINFO adpcm;
} gc_snd_info_t; // sizeof( gc_snd_info_t ) == 128

#pragma pack(pop) // pack(push,1)

enum {
	GC_SND_FLAG_STREAM		  = 0x0001,
	GC_SND_FLAG_STEREO_STREAM = 0x0002,
	GC_SND_FLAG_LOOPING		  = 0x0004,
	GC_SND_FLAG_REVERB		  =	0x0008
};

class gc_snd_t
{
public:
	gc_snd_info_t info;
	bool langs[NSL_LANGUAGE_Z];

	gc_snd_t( )
	{
		memset( &info, 0, sizeof( info ) );
		memset( langs, 0, sizeof( langs ) );
	}
};

static list<gc_snd_t*> level_snds;
static list<gc_snd_t*> stream_snds;

static char stream_filename[256] = "";
static FILE *stream_file = 0;

static HINSTANCE h_dll = NULL;

typedef unsigned int (*func1_t)( unsigned int );
typedef void (*func2_t)( short*, unsigned char*, ADPCMINFO*, unsigned int );
typedef void (*func3_t)( unsigned char*, ADPCMINFO*, unsigned int );

static func1_t getBytesForAdpcmBuffer;
static func2_t encode;
static func3_t getLoopContext;

static gc_snd_t* find_duplicate_info( const list<gc_snd_t*>& haystack, const char* needle )
{

	for( list<gc_snd_t*>::const_iterator i = haystack.begin( );
			 i != haystack.end( ); i++ ) {
		gc_snd_t* gst = *i;
		gc_snd_info_t* info = &gst->info;

		if( stricmp( needle, info->name ) == 0 ) {
			return gst;
		}

	}

	return NULL;
}

static gc_snd_t* find_duplicate_level_entry( const char* name )
{
	return find_duplicate_info( level_snds, name );
}

#define ROUND_UP_32(x) (((unsigned int)(x) + 32 - 1) & ~(32 - 1))
#define ROUND_UP_8(x) (((unsigned int)(x) + 8 - 1) & ~(8 - 1))

static int write_pcm_to_file( gc_snd_info_t* info,
														  SNDFILE* snd_file,
															SF_INFO* snd_info,
															FILE* file,
															int* file_pos )
{
	short* wav_data = NULL;
	int bytes_needed = 0;
	unsigned char* adpcm_data = NULL;
	int written = 0;

	if( snd_info->channels != 1 ) {
		error( "only mono sounds in the level GSW (impossible!) for '%s'\n", info->name );
		return -1;
	}

	if( snd_info->pcmbitwidth != 16 ) {
		error( "unsupported PCM bit width (%d) for '%s'\n", snd_info->pcmbitwidth, info->name );
		return -1;
	}

	if( ( snd_info->format & SF_FORMAT_WAV ) == 0 ) {
		error( "major format must be MS WAV for '%s'\n", info->name );
		return -1;
	}

	wav_data = (short*) malloc( sizeof( short ) * snd_info->samples );
	// at least large enough
	bytes_needed = getBytesForAdpcmBuffer( snd_info->samples );
	adpcm_data = (unsigned char*) malloc( bytes_needed );
	sf_readf_short( snd_file, wav_data, snd_info->samples );
	encode( wav_data, adpcm_data, &info->adpcm, snd_info->samples );

	if( info->flags & GC_SND_FLAG_LOOPING ) {
		getLoopContext( adpcm_data, &info->adpcm, 0 );
	}

	info->offset = (*file_pos);
	info->samples = snd_info->samples;
	info->frequency = snd_info->samplerate;
	written = fwrite( adpcm_data, 1, bytes_needed, file );

	if( written != bytes_needed ) {
		error( "incomplete write %d/%d bytes for '%s'\n", written, bytes_needed, info->name );
	}

	// record our progress
	(*file_pos) += written;

	free( adpcm_data );
	free( wav_data );

#ifdef _DEBUG
	message( "wrote %d bytes for '%s'\n", written, info->name );
#endif

	return 0;
}

static int PackWaveFile( char *wavename, int *_dataSize, int *_frequency )
{
//	WAVE_ReportHeader( "PackWaveFile(%s)", wavename );

	WAVESource waveSource;
	WAVETarget waveTarget;
	int dataSize = 0;
	int dataSamples = 0;
	int frequency = 0;

	printf("packing stream %s: %s: ", stream_filename, wavename);

	if( WAVE_SourceInit(&waveSource) && WAVE_TargetInit(&waveTarget) && WAVE_SourceLoad(&waveSource, wavename) )
	{
		WAVE_TargetSetMono( &waveTarget );
		//WAVE_TargetSetFrequency( &waveTarget, freq );
		WAVE_TargetSetDataFormat( &waveTarget, WAVE_TARGET_GAMECUBE_ADPCM );
		WAVE_TargetSetDataAlignment( &waveTarget, 2048 );
		//WAVE_TargetSetDataInterleavedStereoChunkSize( &waveTarget, 32768 );
		printf( "source: %d bytes, %dHz/%dbits/%s ", 
			waveSource.size, 
			waveSource.freq, 
			waveSource.bits, 
			waveSource.chan == 2 ? "stereo" : "mono"	); 
		if( WAVE_Encode( &waveSource, &waveTarget ) )
		{
			frequency = WAVE_TargetGetFrequency( &waveTarget );
			dataSize = WAVE_TargetGetDataSize( &waveTarget );
			dataSamples = WAVE_TargetGetDataSamples( &waveTarget );
			printf( "target: %d bytes, %d samples, %dHz/%dbits/%s ", 
				waveTarget.dataSize, waveTarget.dataSamples, waveTarget.freq, waveTarget.bits, 
				waveTarget.chan == 2 ? "stereo" : "mono"	
			); 
			if( fwrite( WAVE_TargetGetData(	&waveTarget ), 1, dataSize, stream_file ) != (size_t) dataSize )
				WAVE_ReportError( "Can't write %d bytes to file", dataSize );
			WAVE_TargetFree( &waveTarget );
		}
		WAVE_SourceFree( &waveSource );
	}

	printf("\n");

	if( _dataSize )	 *_dataSize  = dataSize;
	if( _frequency ) *_frequency = frequency;
	return dataSamples;
}

static int write_level_gsw( const char* level, nslLanguageEnum language )
{
	char filename[PATH_MAX];
	FILE* file = NULL;
	int file_pos = 0;

	_snprintf( filename, sizeof( filename ), "%s\\%s\\%s.gsw", opt.gc_out_dir, nslLanguageStr[language], level );
	ensure_dir( filename );

	if( ( file = fopen( filename, "wb" ) ) == NULL ) {
		error( "couldn't open audio data output file '%s'\n", filename );
		return -1;
	}

	for( list<gc_snd_t*>::iterator i = level_snds.begin( );	 i != level_snds.end( ); i++ ) 
	{
		gc_snd_t* snd = *i;
		gc_snd_info_t* info = &snd->info;
		char path[PATH_MAX];
		char* caret = NULL;
		SNDFILE* snd_file = NULL;
		SF_INFO snd_info;

		if( snd->langs[language] == false ) {
#ifdef _DEBUG
			message( "deferring '%s', not set for this language\n", info->name );
#endif
			continue;
		}

		strcpy( path, opt.input_wav_dir );
		strcat( path, info->name );
		strcat( path, ".wav" );
		caret = strrchr( path, '\\' );
		assert( caret );
		*caret = '\0';
		++caret;

		if( _nslLocalizedPath( path, caret, language, NSL_PLATFORM_GAMECUBE, filename, NULL ) < 0 ) {
			error( "couldn't find input audio data file '%s'\n", info->name );
			continue;
		}

		if( info->flags & GC_SND_FLAG_STREAM )
		{
			int dataSize, frequency, samples;

			memset( &info->adpcm, 0, sizeof( info->adpcm ));
			info->offset = ftell( stream_file );
			samples = PackWaveFile( filename, &dataSize, &frequency );
			info->frequency = frequency;
			info->samples = samples;
		}
		else
		{
			snd_file = sf_open_read( filename, &snd_info );

			if( snd_file == NULL ) {
				error( "couldn't open input audio data file '%s'\n", info->name );
				sf_perror( NULL );
				continue;
			}

			write_pcm_to_file( info, snd_file, &snd_info, file, &file_pos );
			sf_close( snd_file );
		}
	}

	fclose( file );

	return 0;
}

static unsigned int swap_uint( unsigned int ui )
{
	unsigned int a = ( ui & 0xFF000000 ) >> 24;
	unsigned int b = ( ui & 0x00FF0000 ) >> 8;
	unsigned int c = ( ui & 0x0000FF00 ) << 8;
	unsigned int d = ( ui & 0x000000FF ) << 24;

	return ( a | b | c | d );
}

static unsigned short swap_ushort( unsigned short us )
{
	unsigned short a = ( us & 0xFF00 ) >> 8;
	unsigned short b = ( us & 0x00FF ) << 8;

	return ( a | b );
}

static float swap_float( float f )
{
	unsigned int i = *( (unsigned int*) &f );
	unsigned int a = ( i & 0x000000FF ) << 24;
	unsigned int b = ( i & 0x0000FF00 ) << 8;
	unsigned int c = ( i & 0x00FF0000 ) >> 8;
	unsigned int d = ( i & 0xFF000000 ) >> 24;

	unsigned int r = ( a | b | c | d );

	f = *( (float*) &r );

	return f;
}

static void swap_adpcm( ADPCMINFO* adpcm )
{
	int i;

	for( i = 0; i < 16; i++ ) {
		adpcm->coef[i] = swap_ushort( adpcm->coef[i] );
	}

	adpcm->gain = swap_ushort( adpcm->gain );
	adpcm->pred_scale = swap_ushort( adpcm->pred_scale );
	adpcm->yn1 = swap_ushort( adpcm->yn1 );
	adpcm->yn2 = swap_ushort( adpcm->yn2 );

	adpcm->loop_pred_scale = swap_ushort( adpcm->loop_pred_scale );
	adpcm->loop_yn1 = swap_ushort( adpcm->loop_yn1 );
	adpcm->loop_yn2 = swap_ushort( adpcm->loop_yn2 );
}

static int write_level_snd( const char* level, nslLanguageEnum language )
{
	char filename[PATH_MAX];
	FILE* file = NULL;
	gc_snd_header_t header;
	int written = 0;
	int num_entries = 0;

	assert( sizeof( header ) == 64 );

	_snprintf( filename, sizeof( filename ), "%s\\%s\\%s.snd", opt.gc_out_dir, nslLanguageStr[language], level );
	ensure_dir( filename );

	if( ( file = fopen( filename, "wb" ) ) == NULL ) 
	{
		error( "couldn't open snd output file '%s'\n", filename );
		return -1;
	}

#ifdef _DEBUG
	message( "writing '%s'\n", filename );
#endif

	// make room for header
	memset( &header, 0, sizeof( header ) );
	written = fwrite( &header, 1, sizeof( header ), file );

	if( written != sizeof( header ) ) 
	{
		error( "incomplete write of snd file header\n" );
		return -1;
	}

	// tweak the goddamn names and write
	for( list<gc_snd_t*>::iterator i = level_snds.begin( ); i != level_snds.end( ); i++ ) 
	{
		gc_snd_t* snd = *i;
		gc_snd_info_t* src = &snd->info;
		gc_snd_info_t dst;
		char* caret = NULL;

		if( snd->langs[language] == false ) {
#ifdef _DEBUG
			message( "deferring '%s', not set for this language\n", src->name );
#endif
			continue;
		}

		++num_entries;

		memcpy( &dst, src, sizeof( dst ) );
		caret = get_basename( src->name );
		strncpy( dst.name, caret, sizeof( dst.name ) );
		_strupr( dst.name );

		// byte-swap
		dst.alias = swap_uint( dst.alias );
		dst.flags = swap_uint( dst.flags );
		dst.offset = swap_uint( dst.offset );
		dst.samples = swap_uint( dst.samples );
		dst.frequency = swap_uint( dst.frequency );
		dst.type = swap_uint( dst.type );
		dst.volume = swap_float( dst.volume );

		swap_adpcm( &dst.adpcm );

#ifdef _DEBUG
		message( "writing snd info for '%s' ('%s')\n", dst.name, src->name );
#endif
		written = fwrite( &dst, 1, sizeof( dst ), file );

		if( written != sizeof( dst ) ) {
			error( "incomplete write of snd file info\n" );
		}

	}

	fseek( file, 0, SEEK_SET );
	strcpy( header.tag, "SOND" );
	header.version = GC_SND_VERSION;
	header.version = swap_uint( header.version );
	_snprintf( header.gsw_name, sizeof( header.gsw_name ), "%s.gsw", level );
	header.num_entries = swap_uint( num_entries );
	memset( header.pad, 0, sizeof( header.pad ) );

	written = fwrite( &header, 1, sizeof( header ), file );

	if( written != sizeof( header ) ) {
		error( "incomplete write of snd file header\n" );
		return -1;
	}

	fclose( file );

	return 0;
}

/***
 * Public interface.
 ***/

int pre_export_gc( void )
{
	assert( h_dll == NULL );

	h_dll = LoadLibrary( "dsptool.dll" );

	if( !h_dll ) {
		error( "couldn't load dsptool.dll!\n" );
		return -1;
	}

	getBytesForAdpcmBuffer = (func1_t) GetProcAddress( h_dll, "getBytesForAdpcmBuffer" );
	encode = (func2_t) GetProcAddress( h_dll, "encode" );
	getLoopContext = (func3_t) GetProcAddress( h_dll, "getLoopContext" );

	sprintf( stream_filename, "%s/%s", opt.gc_out_dir, "COMMON.STR" );
	stream_file = fopen( stream_filename, "wb" );
	if( !stream_file ) 
	{
		error( "can't create stream_file %s\n", stream_filename );
		return -1;
	}

	return 0;
}

static void mr_delete( void* p )
{
	delete p;
}

int post_export_gc( void )
{
	FreeLibrary( h_dll );
	h_dll = NULL;

	return 0;
}

int start_level_gc( const char* level )
{
	char filename[256];
	FILE* file = NULL;
	int num_snds = 0;

	_snprintf( filename, sizeof( filename ), "%s\\%s.snd", opt.input_snd_dir, level );

	if( ( file = fopen( filename, "r" ) ) == NULL ) {
		error( "couldn't open input snd '%s'\n", filename );
		return -1;
	}

	// extra because I don't trust count_lines, no other reason
	num_snds = count_lines( file ) + 1;
	fclose( file );

	return 0;
}

static void or_languages( bool* dst, const bool* src )
{

	for( int i = 0; i < NSL_LANGUAGE_Z; i++ ) {

		if( src[i] ) {
			dst[i] = true;
		}

	}

}

int export_file_gc( snd_info_t* info )
{
	char fullname[PATH_MAX];
	gc_snd_t* dup = NULL;

	_snprintf( fullname, sizeof( fullname ), "%s\\%s", info->path, info->name );

	// no duplicates
	if( ( dup = find_duplicate_level_entry( fullname ) ) ) {
		or_languages( dup->langs, info->languages );
#ifdef _DEBUG
		message( "discarding duplicate level entry '%s'\n", fullname );
#endif
		return 0;
	}

	gc_snd_t* gc_snd = new gc_snd_t;
	gc_snd_info_t* gc_info = &gc_snd->info;
	memcpy( &gc_snd->langs, info->languages, sizeof( gc_snd->langs ) );

	strncpy( gc_info->name, fullname, sizeof( gc_info->name ) );
	gc_info->flags = 0;

	if( info->cd ) {
		gc_info->flags |= GC_SND_FLAG_STREAM;
	}

	if( info->loop ) {
		gc_info->flags |= GC_SND_FLAG_LOOPING;
	}

	if(info->applyReverb)
	{
		gc_info->flags |= GC_SND_FLAG_REVERB;
	}

	gc_info->alias = info->aliasID;
	gc_info->offset = -1;
	gc_info->samples = -1;
	gc_info->frequency = -1;
	gc_info->type = info->type;
	// FIXME: der Hack!
	gc_info->volume = ( info->volume_left + info->volume_right ) / 2.0f;

	memset( &gc_info->adpcm, 0, sizeof( gc_info->adpcm ) );

	level_snds.push_back( gc_snd );

	return 0;
}

int end_level_gc( const char* level )
{

	for( int i = 1; i < NSL_LANGUAGE_Z; ++i ) {

		if( opt.export_language[i] ) {
			nslLanguageEnum language = (nslLanguageEnum) i;
			write_level_gsw( level, language );
			write_level_snd( level, language );
		}

	}

	for_each( level_snds.begin( ), level_snds.end( ), mr_delete );
	level_snds.clear( );

	if( stream_file )
	{
		fclose( stream_file );
		stream_file = NULL;
		strcpy( stream_filename, "" );
	}

	return 0;
}
