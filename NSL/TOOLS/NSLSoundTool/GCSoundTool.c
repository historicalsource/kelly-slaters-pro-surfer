#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "sndfile.h"
#include "adpcm.h"

#include "GCSoundTool.h"

typedef struct _gc_snd_info_t {
	char name[PATH_MAX];
	int cd;
	int loop;

	int size;
	int offset;
	int frequency;
	int voll;
	int volr;
	int stereo;
	int group;
} gc_snd_info_t;

#define GC_NUM_STREAM_INFO 256

static FILE* stream_file = NULL;
static int stream_pos = 0;

static gc_snd_info_t* stream_snds = NULL;
static gc_snd_info_t* level_snds = NULL;

static int stream_alloced = 0;
static int level_alloced = 0;

static int stream_count = 0;
static int level_count = 0;

int pre_export_gc( void )
{
	char filename[256];

	assert( !stream_snds );
	stream_alloced = GC_NUM_STREAM_INFO;
	stream_snds = malloc( sizeof( gc_snd_info_t ) * stream_alloced );
	stream_count = 0;

	// open stream output file
	_snprintf( filename, sizeof( filename ), "%s\\stream.gsw", opt_gc_out_dir );

	if( ( stream_file = fopen( filename, "wb" ) ) == NULL ) {
		error( "couldn't open streaming audio data output file '%s'\n", filename );
		return -1;
	}

	stream_pos = 0;

	return 0;
}

static int find_duplicate_stream_entry( gc_snd_info_t* info )
{
	int i;

	for( i = 0; i < stream_count; i++ ) {
		gc_snd_info_t* gsit = &stream_snds[i];

		if( stricmp( info->name, gsit->name ) == 0 ) {
			// duplicate
			return i;
		}

	}

	return -1;
}

#define ROUND_UP_32(x) (((unsigned int)(x) + 32 - 1) & ~(32 - 1))

static int write_pcm_to_file( FILE* file, int* file_pos, gc_snd_info_t* info )
{
	char filename[PATH_MAX];
	SNDFILE* snd_file = NULL;
	SF_INFO snd_info;
	short* wav_data = NULL;
	char* adpcm_data = NULL;
	int num_samples = 0;
	int sample_size = 0;
	struct adpcm_state state = { -1, 0 };
	int j;
	int written = 0;
	int padding = 0;

	_snprintf( filename, sizeof( filename ), "%s\\%s.wav", opt_input_wav_dir, info->name );
	snd_file = sf_open_read( filename, &snd_info );

	if( snd_file == NULL ) {
		error( "couldn't open input audio data file '%s'\n", info->name );
		sf_perror( NULL );
		return -1;
	}

	if( snd_info.pcmbitwidth != 16 ) {
		error( "unsupported PCM bit width (%d) for '%s'\n", snd_info.pcmbitwidth, filename );
		sf_close( snd_file );
		return -1;
	}

	if( ( snd_info.format & SF_FORMAT_WAV ) == 0 ) {
		error( "major format must be MS WAV for '%s'\n", filename );
		sf_close( snd_file );
		return -1;
	}

	num_samples = snd_info.samples * snd_info.channels;
	sample_size = sizeof( short ) * num_samples;
	wav_data = malloc( sample_size );
	adpcm_data = malloc( ( sample_size / 4 ) + 1 );
	sf_readf_short( snd_file, wav_data, snd_info.samples );
	info->size = adpcm_coder( wav_data, adpcm_data, num_samples, &state );

	info->offset = (*file_pos);
	info->frequency = snd_info.samplerate;
	info->stereo = ( snd_info.channels > 1 ) ? 1 : 0;
	written = fwrite( adpcm_data, 1, info->size, file );

	if( written != info->size ) {
		error( "incomplete write %d/%d bytes for '%s'\n", written, info->size, filename );
	}

	// pad out to 32B
	padding = ROUND_UP_32( written ) - written;

	for( j = 0; j < padding; j++ ) {
		fputc( 0, file );
	}

	written += padding;
	// record our progress
	(*file_pos) += written;

	free( adpcm_data );
	free( wav_data );

	sf_close( snd_file );

#ifdef _DEBUG
	message( "wrote %d bytes (IMA-ADPCM) for '%s'\n", written, filename );
#endif

	return 0;
}

static int write_to_stream_gsw( gc_snd_info_t* info )
{
	int i;

	i = find_duplicate_stream_entry( info );

	if( i < 0 ) {
		return 0;
	}

	// need to write the goods.
	write_pcm_to_file( stream_file, &stream_pos, info );

	return 0;
}

static int add_to_stream_list( gc_snd_info_t* info )
{
	int i;

	if( stream_count >= stream_alloced ) {
		error( "overflowed stream list, please increase GC_NUM_STREAM_INFO\n" );
		return -1;
	}

	i = find_duplicate_stream_entry( info );

	// ie, we couldn't find a duplicate
	if( i < 0 ) {
		memcpy( &stream_snds[stream_count++], info, sizeof( gc_snd_info_t ) );
		assert( stream_count <= stream_alloced );
	}

	return 0;
}

int post_export_gc( void )
{
	fclose( stream_file );
	stream_file = NULL;

	free( stream_snds );
	stream_snds = NULL;

	return 0;
}

int start_level_gc( const char* level )
{
	char filename[256];
	FILE* file = NULL;
	int num_snds = 0;

	_snprintf( filename, sizeof( filename ), "%s\\%s.snd", opt_input_snd_dir, level );

	if( ( file = fopen( filename, "r" ) ) == NULL ) {
		error( "couldn't open input snd '%s'\n", filename );
		return -1;
	}

	// extra because I don't trust count_lines, no other reason
	num_snds = count_lines( file ) + 1;
	fclose( file );

	assert( !level_snds );
	level_alloced = num_snds;
	level_snds = malloc( sizeof( gc_snd_info_t ) * level_alloced );
	level_count = 0;

	return 0;
}

int export_file_gc( snd_info_t* info )
{
	gc_snd_info_t* gc_snd = NULL;

	if( level_count >= level_alloced ) {
		return -1;
	}

	gc_snd = &level_snds[level_count++];
	assert( level_count <= level_alloced );

	strncpy( gc_snd->name, info->name, PATH_MAX );
	gc_snd->cd = info->cd;
	gc_snd->loop = info->loop;

	gc_snd->size = -1;
	gc_snd->offset = -1;
	gc_snd->frequency = -1;
	gc_snd->voll = 100;
	gc_snd->volr = 100;
	gc_snd->stereo = 0;
	gc_snd->group = 0;

	if( info->cd ) {
		add_to_stream_list( gc_snd );
	}

	return 0;
}

static int write_level_gsw( const char* level )
{
	char filename[PATH_MAX];
	FILE* file = NULL;
	int file_pos = 0;
	int i;

	_snprintf( filename, sizeof( filename ), "%s\\%s.gsw", opt_gc_out_dir, level );

	if( ( file = fopen( filename, "wb" ) ) == NULL ) {
		error( "couldn't open audio data output file '%s'\n", filename );
		return -1;
	}

	for( i = 0; i < level_count; i++ ) {
		gc_snd_info_t* info = &level_snds[i];

		// we don't write cd data to the level.gsw,
		// we write it to the stream.gsw
		if( info->cd ) {
			write_to_stream_gsw( info );
		} else {
			write_pcm_to_file( file, &file_pos, info );
		}

	}

	fclose( file );

	return 0;
}

const char* snd_groups[] = {
	"sfx",
	"ambient",
	"music",
	"voice",
	"movie",
};

static int write_level_snd( const char* level )
{
	char filename[PATH_MAX];
	FILE* file = NULL;
	int i;

	_snprintf( filename, sizeof( filename ), "%s\\%s.snd", opt_gc_out_dir, level );

	if( ( file = fopen( filename, "w" ) ) == NULL ) {
		error( "couldn't open snd output file '%s'\n", filename );
		return -1;
	}

	fprintf( file, "; DO NOT EDIT. Generated by NSLSoundTool (oh yeah!).\n" );

	for( i = 0; i < level_count; i++ ) {
		gc_snd_info_t* snd = &level_snds[i];
		const char* group = snd_groups[snd->group];
		char munged[PATH_MAX];
		char* caret = NULL;

		caret = get_basename( snd->name );
		strncpy( munged, caret, PATH_MAX );
		caret = strchr( munged, '.' );

		if( caret ) {
			*caret = '\0';
		}

		caret = munged;

		while( *caret ) {
			*caret = toupper( *caret );
			++caret;
		}

		fprintf( file, "%s size %d offset %d freq %d voll %d volr %d %s",
						 munged, snd->size, snd->offset, snd->frequency,
						 snd->voll, snd->volr, group );

		if( snd->cd ) {
			fputs( " cd", file );
		}

		if( snd->loop ) {
			fputs( " loop", file );
		}

		if( snd->stereo ) {
			fputs( " stereo", file );
		} else {
			fputs( " mono", file );
		}

		fputc( '\n', file );
	}

	fprintf( file, "COLLECTION gcsound\\%s.gsw\n", level );

	fclose( file );

	return 0;
}

int end_level_gc( const char* level )
{
	write_level_gsw( level );
	write_level_snd( level );

	free( level_snds );
	level_snds = NULL;

	return 0;
}
