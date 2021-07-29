#define	WIN32_LEAN_AND_MEAN 1
#define	WIN32_EXTRA_LEAN 1
#include <windows.h>
#include <stdlib.h>
#include "libWave.h"

#define	LINK(x)	link(&x,#x);
static	HMODULE	dspToolDLL;
static	char*	dspToolDLLName = "dsptool.dll";
static	int	 (*getBytesForAdpcmInfo) (void);
static	int	 (*getNibbleAddress) (int samples);
static	int	 (*getBytesForPcmBuffer) (int samples);
static	int	 (*getBytesForAdpcmBuffer)(int samples);
static	int	 (*getBytesForPcmSamples) (int samples);
static	int	 (*getBytesForAdpcmSamples)(int samples);
static	int	 (*getSampleForAdpcmNibble) (int nibble);
static	void (*getLoopContext)(char *src, WAVE_GAMECUBE_ADPCMINFO *cxt, int samples);
static	void (*encode)(short *src, char  *dst, WAVE_GAMECUBE_ADPCMINFO *cxt, int samples);
static	void (*decode)(char  *src, short *dst, WAVE_GAMECUBE_ADPCMINFO *cxt, int samples);
static	int  link( void* func, char *name )
{	
	*(void**)func = (void *)GetProcAddress( dspToolDLL, name ); 
	if( *(void**)func == 0 )
	{
		WAVE_ReportError( "missing function `%s'", name );
		return 0;
	}
	return 1;
}
static int init( void )
{
	if( dspToolDLL )
		return 1;

	dspToolDLL = LoadLibrary( dspToolDLLName );
	if( dspToolDLL )
	{
		TCHAR name[256];
		int r;

		GetModuleFileName(dspToolDLL, name, sizeof(name));
		WAVE_ReportInternal( "\nLoading %s", name );

		r|=LINK(getBytesForAdpcmBuffer);
		r&=LINK(getBytesForAdpcmSamples);
		r&=LINK(getBytesForPcmBuffer);
		r&=LINK(getBytesForPcmSamples);
		r&=LINK(getSampleForAdpcmNibble);
		r&=LINK(getBytesForAdpcmInfo);
		r&=LINK(getNibbleAddress);
		r&=LINK(encode);
		r&=LINK(decode);
		r&=LINK(getLoopContext);

		if( r == 0 )
			WAVE_ReportFatal( "one or more missing DLL functions" );

		return r;
	}

	WAVE_ReportFatal( "failed to load DLL" );
	return 0;
}
#undef LINK

static int getBytesForAdpcmBuffer2( int samples )
{
	// dspToolD.dll formula is
	//
	// ((samples/14) + (samples%14 != 0)) * 8
	//
	// dspTool.dll formula seems to be as dspToolD.dll but optimized
	//
	return ((samples + 13) / 14) * 8;
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

static void swap_adpcm( WAVE_GAMECUBE_ADPCMINFO* adpcm )
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

static void encode2(short *src, char  *dst, WAVE_GAMECUBE_ADPCMINFO *cxt, int samples, int headerSize)
{
	encode( src, dst + headerSize, cxt, samples );
	memcpy( dst, cxt, sizeof(*cxt));
	swap_adpcm( (WAVE_GAMECUBE_ADPCMINFO *) dst );
}

int WAVE_EncodeGAMECUBE_ADPCM( const WAVESource *_source, WAVETarget *target )
{
	WAVE_ReportHeader( 
		"WAVE_EncodeGAMECUBE_ADPCM:%s:%d:%d:%dHz:%d:%s->%dKhz:%s", 
			dspToolDLLName,
			_source->size, 
				WAVE_SourceGetSampleCount(_source),
				_source->freq, 
				_source->bits, 
				_source->chan == 1 ? "mono" : (_source->chan == 2 ? "stereo" : "unknown"), 
			 target->freq,
				 target->chan == 1 ? "mono" : ( target->chan == 2 ? "stereo" : "unknown")
		 );

	WAVESource source;
	int result = 0;

	if( WAVE_SourceIsValid(_source) && init() && WAVE_SourceInit(&source) )
	{
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
			int headerSize;

			dataInterleavedStereoChunkSize = target->dataInterleavedStereoChunkSize;

			doInterleave = (source.chan == 2) && (dataInterleavedStereoChunkSize > 0);

			sampleSize  = WAVE_SourceGetSampleSize(  &source );
			sampleCount = WAVE_SourceGetSampleCount( &source );

			headerSize  = WAVE_GAMECUBE_ADPCMINFO_SIZE;
			adpcmBytes  = headerSize;
			adpcmBytes += getBytesForAdpcmBuffer( sampleCount * source.chan );
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
					WAVE_GAMECUBE_ADPCMINFO adpcmInfoL = {0};
					WAVE_GAMECUBE_ADPCMINFO adpcmInfoR = {0};
					short *L, *R;

					if( WAVE_StereoSplit( &source, (void **)&L, (void **)&R ) )
					{
						short *LEnc, *REnc;

						LEnc = (short *)WAVE_MemoryCalloc(1, adpcmBytes/2);
						REnc = (short *)WAVE_MemoryCalloc(1, adpcmBytes/2);
						if( LEnc && REnc )
						{
							encode2( L, (char*)LEnc, &adpcmInfoL, sampleCount, headerSize );
							encode2( R, (char*)REnc, &adpcmInfoR, sampleCount, headerSize );
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
					WAVE_GAMECUBE_ADPCMINFO adpcmInfoM = {0};
					encode2( (short *)source.data, (char *)adpcmBuffer, &adpcmInfoM, sampleCount, headerSize );
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
