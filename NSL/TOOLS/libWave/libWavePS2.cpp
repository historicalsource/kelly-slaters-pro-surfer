#define	WIN32_LEAN_AND_MEAN 1
#define	WIN32_EXTRA_LEAN 1
#include <windows.h>
#include <stdlib.h>
#include "libWave.h"

#define	LINK(x)	link(&x,#x);
static	HMODULE	EncVagDLL;
static  char*   EncVagDLLName="encvag.dll";
static	int	 (pascal *EncVagInit)( short conversion_mode );
static	int	 (pascal *EncVag)( short x[], short y[], short block_attribute );
static  int	 (pascal *EncVagFin)( short y[] );
static	int  link( void* func, char *name )
{	
	*(void**)func = (void *)GetProcAddress( EncVagDLL, name ); 
	if( *(void**)func == 0 )
	{
		WAVE_ReportError( "missing DLL function `%s'", name );
		return 0;
	}
	return 1;
}
static int init( void )
{
	int r=0;

	if( EncVagDLL )
		return 1;

	if( EncVagDLL == NULL )
	{
		EncVagDLL = LoadLibrary( EncVagDLLName );
		if( EncVagDLL )
		{
			TCHAR name[256];
			GetModuleFileName( EncVagDLL, name, sizeof(name));
			WAVE_ReportInternal( "%s loaded", name );
			r|=LINK(EncVagInit);
			r&=LINK(EncVag);
			r&=LINK(EncVagFin);

			if( r == 0 )
				WAVE_ReportFatal( "one or more missing DLL functions" );
		}
		else
		{
			WAVE_ReportError( "failed to load dll" );
		}
	}
	return r;
}
#undef LINK

/* conversion_mode */
#define ENC_VAG_MODE_NORMAL	1
#define ENC_VAG_MODE_HIGH	2
#define ENC_VAG_MODE_LOW	3
#define ENC_VAG_MODE_4BIT	4

/* block_attribute */
#define ENC_VAG_1_SHOT		0
#define ENC_VAG_1_SHOT_END	1
#define ENC_VAG_LOOP_START	2
#define ENC_VAG_LOOP_BODY	3
#define ENC_VAG_LOOP_END	4

static void encode( short *src, short *dst, int numBlocks )
{
	int n_blocks = numBlocks;
	try
	{
		EncVagInit( ENC_VAG_MODE_NORMAL );
		while( n_blocks-- )
		{
			EncVag( src, dst, ENC_VAG_LOOP_BODY );
			src += 28;
			dst += 8;
		}
	}
	catch( ... )
	{
		WAVE_ReportError( "\n%s exception: block:%d/%d src=%p dst=%p %s\n", 
			EncVagDLLName, numBlocks - n_blocks - 1, n_blocks, src, dst );
	}
}

int WAVE_EncodePS2_VAG( const WAVESource *_source, WAVETarget *target )
{
	WAVE_ReportHeader( 
		"WAVE_EncodePS2_VAG:%s:%d:%d:%dHz:%d:%s->%dKhz:%s", 
			EncVagDLLName,
			_source->size, 
				WAVE_SourceGetSampleCount(_source),
				_source->freq, 
				_source->bits, 
				_source->chan == 1 ? "mono" : (_source->chan == 2 ? "stereo" : "unknown"), 
			 target->freq,
				 target->chan == 1 ? "mono" : ( target->chan == 2 ? "stereo" : "unknown") 
	);

	WAVESource source;
	int result=0;

	if( WAVE_SourceIsValid(_source) && init() && WAVE_SourceInit(&source) )
	{
		memset(&source, 0, sizeof(source));
		source.bits = 16; // IMAADPCM requires 16 bits
		source.freq = target->freq;
		source.chan = target->chan;
		if( WAVE_SourceConvert( _source, &source ) )
		{
		  void* adpcmBuffer;
			int adpcmBytes;
			int sampleCount;
			int sampleSize;
			int doInterleave;
			int dataInterleavedStereoChunkSize;
			int adpcmBlocks;

			dataInterleavedStereoChunkSize = target->dataInterleavedStereoChunkSize;

			doInterleave = (source.chan == 2) && (dataInterleavedStereoChunkSize > 0);

			sampleSize  = WAVE_SourceGetSampleSize(  &source );
			sampleCount = WAVE_SourceGetSampleCount( &source );

			adpcmBlocks = (sampleCount * sampleSize  + 55) / 56;
			adpcmBytes  = adpcmBlocks * 16;
			adpcmBytes *= source.chan;
			adpcmBytes  = (adpcmBytes + target->dataAlignment - 1) / target->dataAlignment;
			adpcmBytes *= target->dataAlignment;

			if( doInterleave )
				adpcmBytes  = WAVE_StereoInterleaveGetSize( adpcmBytes, target->dataInterleavedStereoChunkSize );

			adpcmBuffer = WAVE_MemoryCalloc( 1, adpcmBytes );
			if( adpcmBuffer )
			{
				int isOk = 0;

				if( doInterleave )
				{
					short *L, *R;

					if( WAVE_StereoSplit( &source, (void **)&L, (void **)&R ) )
					{
						short *LEnc, *REnc;

						LEnc = (short *)WAVE_MemoryCalloc(1, adpcmBytes/2);
						REnc = (short *)WAVE_MemoryCalloc(1, adpcmBytes/2);
						if( LEnc && REnc )
						{
							encode( (short *)L, (short *)LEnc, adpcmBlocks/2 );
							encode( (short *)R, (short *)REnc, adpcmBlocks/2 );
							isOk = WAVE_StereoInterleave( adpcmBytes/2, LEnc, REnc, target->dataInterleavedStereoChunkSize, adpcmBuffer );
						}
						WAVE_MemoryFree( REnc );
						WAVE_MemoryFree( LEnc );
						WAVE_MemoryFree( R );
						WAVE_MemoryFree( L );
					}
				}
				else
				{
					encode( (short *)source.data, (short *)adpcmBuffer, adpcmBlocks );
					isOk = 1;
				}

				if( isOk )
				{
					target->bits		= source.bits;
					target->chan		= source.chan;
					target->freq		= source.freq;
					target->data		= adpcmBuffer;
					target->dataSize	= adpcmBytes;
					target->dataSamples	= sampleCount;
					result = 1;
				}
				else
				{
					WAVE_MemoryFree( adpcmBuffer );
				}
			}
			WAVE_SourceFree( &source );
		}
	}
	return result;
}
