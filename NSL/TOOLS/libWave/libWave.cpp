#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "libWave.h"

#pragma comment(lib,"msacm32")
#pragma comment(lib,"winmm")

WAVEFORMATEX* WIN32DSHOW_WaveLoad( const char *filename, void **data, int *size );
WAVEFORMATEX* LIBSNDFILE_WaveLoad( const char *filename, void **_data, int *_size );

void* WAVE_MemoryRealloc( void *mem, int newSize )
{
	return realloc( mem, newSize );
}

void* WAVE_MemoryCalloc( int elCount, int elSize )
{
	return calloc( elCount, elSize );
}

void* WAVE_MemoryAlloc( int size )
{
	return malloc( size );
}

int	WAVE_MemoryFree( void *ptr )
{
	free( ptr );
	return 1;
}

int WIN32ACM_WaveConvert( WAVEFORMATEX *iwf, void *iwave, int isize, WAVEFORMATEX *owf, void **_owave, int *_osize )
{
	WAVE_ReportHeader( "WIN32ACM_WaveConvert" );

	ACMSTREAMHEADER strmhdr;
	HACMSTREAM sh;
	DWORD osize;
	void *owave;

	if( _owave ) *_owave = NULL;
	if( _osize ) *_osize = 0;

	if(acmStreamOpen( &sh, NULL, iwf, owf, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ))
		return 0;

	if(acmStreamSize( sh, isize, &osize, ACM_STREAMSIZEF_SOURCE ) || ((owave = WAVE_MemoryCalloc(1,osize + 65536)) == NULL ))
	{
		acmStreamClose( sh, 0 );
		return 0;
	}

	memset( &strmhdr, 0, sizeof( strmhdr));
	strmhdr.cbStruct        = sizeof(strmhdr);
	strmhdr.pbSrc           = (unsigned char*) iwave;
	strmhdr.cbSrcLength     = 
	strmhdr.cbSrcLengthUsed = isize;
	strmhdr.pbDst           = (unsigned char*) owave;
	strmhdr.cbDstLength     =
	strmhdr.cbDstLengthUsed = osize;

	if( acmStreamPrepareHeader(sh,&strmhdr,0) || acmStreamConvert(sh,&strmhdr,ACM_STREAMCONVERTF_END|ACM_STREAMCONVERTF_START) )
	{
		WAVE_MemoryFree(owave);
		acmStreamClose( sh, 0 );
		return 0;
	}

	osize = strmhdr.cbDstLengthUsed; // didn't necessarily need all that

	if( acmStreamUnprepareHeader(sh,&strmhdr,0) )
	{
		WAVE_MemoryFree(owave);
		acmStreamClose(sh,0);
		return 0;
	}

	if( _owave ) *_owave = owave;
	if( _osize ) *_osize = osize;
	return acmStreamClose(sh,0) == 0;
}

WAVEFORMATEX* WIN32MMIO_WaveLoadPCM( const char *_filename, void **_wave, int *_size )
{
	WAVE_ReportHeader( "WIN32MMIO_WaveLoadPCM(%s)", _filename );

	char			filename[256];
	HMMIO			hm;
	void*			wave;
	DWORD			size;
	WORD			extraBytes;
	MMCKINFO		ckRiff, ckFmt, ckData;
	WAVEFORMATEX	*pwfx;
	PCMWAVEFORMAT	pcmWF;

	pwfx = NULL;
	wave = 0;
	size = 0;

	strncpy( filename, _filename, sizeof(filename) );
	hm = mmioOpen( filename, NULL, MMIO_ALLOCBUF|MMIO_READ);
	if( hm )
	{
		ckFmt.ckid = mmioFOURCC('f','m','t',' ');
        ckData.ckid = mmioFOURCC('d', 'a', 't', 'a');
		extraBytes = 0;
		if(	!mmioDescend(hm,&ckRiff,NULL,0)
				&& ckRiff.ckid==FOURCC_RIFF
				&& ckRiff.fccType==mmioFOURCC('W','A','V','E') &&
			!mmioDescend(hm,&ckFmt,&ckRiff,MMIO_FINDCHUNK)
				&& ckFmt.cksize>=sizeof(pcmWF) &&
			mmioRead(hm,(char*)&pcmWF,sizeof(pcmWF))==sizeof(pcmWF)
				&&( pcmWF.wf.wFormatTag==WAVE_FORMAT_PCM 
				|| mmioRead(hm,(char*)&extraBytes,sizeof(extraBytes)==sizeof(extraBytes))) &&
			(pwfx = (WAVEFORMATEX*)WAVE_MemoryCalloc(1,sizeof(pwfx[0])+extraBytes)))
		{
			memcpy(pwfx,&pcmWF,sizeof(pcmWF));
			if(extraBytes)
			   mmioRead(hm,(char*)(pwfx + 1),extraBytes);
			pwfx->cbSize = extraBytes;
            if( mmioSeek(hm,ckRiff.dwDataOffset+sizeof(FOURCC),SEEK_SET)!=-1 &&	mmioDescend(hm,&ckData,&ckRiff,MMIO_FINDCHUNK)==0 )
			{
				if((wave=WAVE_MemoryCalloc(1,(size=ckData.cksize) +  65536)))
				{
					if( (DWORD)mmioRead(hm,(char*)wave,size) != size )
					{
						WAVE_ReportError( "mmioRead(%p,%p,%d) failed", hm, wave, size );
						WAVE_MemoryFree( pwfx );
						pwfx = 0;
						WAVE_MemoryFree( wave );
						size = 0;
						wave = NULL;
					}
				}
				else
				{
					WAVE_MemoryFree(pwfx);
					pwfx = 0;
				}
			}
			else
			{
				WAVE_ReportError( "mmioSeek(%p,%p,SEEK_SET) or mmioDescend() failed", hm, ckRiff.dwDataOffset+sizeof(FOURCC) );
				WAVE_MemoryFree(pwfx);
				pwfx = 0;
			}
		}	
		else
		{
			WAVE_ReportError( "failed" );
		}
		mmioClose( hm, 0 );
	}
	else
	{
		WAVE_ReportError( "mmioOpen(%s, NULL, MMIO_ALLOCBUF|MMIO_READ) failed!", filename );
	}

	if( _wave ) *_wave = wave;
	if( _size ) *_size = size;
	return pwfx;
}

int	WAVE_SourceInit( WAVESource *wave )
{
	if( wave )
	{
		memset( wave, 0, sizeof(*wave) );
		return 1;
	}
	return 0;
}

int WAVE_SourceFree( WAVESource *wave )
{
	if( wave )
	{
		WAVE_MemoryFree(wave->data);
		return WAVE_SourceInit( wave );
	}
	return 0;
}

int WAVE_SourceLoad( WAVESource *source, const char *filename )
{
//	WAVE_ReportHeader( "WAVE_SourceLoad(%s)", filename );

	if( source && filename )
	{
		WAVEFORMATEX *iwf;
		void *data;
		int size;

		iwf = NULL;

		if( !iwf ) iwf = LIBSNDFILE_WaveLoad( filename, &data, &size );
		if( !iwf ) iwf = WIN32MMIO_WaveLoadPCM( filename, &data, &size );
//		if( !iwf ) iwf = WIN32DSHOW_WaveLoad( filename, &data, &size );

		if( iwf )
		{
			// Check requirments how user wanted WAVE file to be loaded (source->chan, source->bits, source->freq)
			// And if needed to perform some conversion on the fly
			// Also if the loaded source file is not pure PCM it will be converted to such
			if( (source->chan && source->chan != iwf->nChannels)       ||
				(source->bits && source->bits != iwf->wBitsPerSample)  ||
				(source->freq && source->freq != (int)iwf->nSamplesPerSec)  ||
				(iwf->wFormatTag != WAVE_FORMAT_PCM) ||	(iwf->cbSize != 0)   ||
				(iwf->nBlockAlign != iwf->wBitsPerSample * iwf->nChannels/8) ||
				(iwf->nAvgBytesPerSec != iwf->nSamplesPerSec * iwf->nBlockAlign)
			){
				WAVEFORMATEX owf;
				void *odata;
				int osize;

				memset(&owf,0,sizeof(owf));
				owf.wFormatTag      = WAVE_FORMAT_PCM;
				owf.nChannels       = source->chan ? source->chan : iwf->nChannels;
				owf.wBitsPerSample  = source->bits ? source->bits : iwf->wBitsPerSample;
				owf.nSamplesPerSec  = source->freq ? source->freq : iwf->nSamplesPerSec;
				owf.nBlockAlign     = owf.wBitsPerSample*owf.nChannels/8;
				owf.nAvgBytesPerSec = owf.nSamplesPerSec*owf.nBlockAlign;
				if( WIN32ACM_WaveConvert( iwf, data, size, &owf, &odata, &osize ) )
				{
					WAVE_MemoryFree( data );
					data = odata;
					source->chan = owf.nChannels;
					source->bits = owf.wBitsPerSample;
					source->freq = owf.nSamplesPerSec;
				}
				else
				{
					WAVE_MemoryFree( data );
					WAVE_MemoryFree( iwf );
					return 0;
				}
			}
			else
			{
				source->chan = iwf->nChannels;
				source->bits = iwf->wBitsPerSample;
				source->freq = iwf->nSamplesPerSec;
			}
			source->data = data;
			source->size = size;
			return 1;
		}
	}
	return 0;
}

int WAVE_SourceGetSampleSize( const WAVESource *source )
{
	return source ? source->chan * source->bits / 8 : 0;
}

int WAVE_SourceGetSampleCount( const WAVESource *source )
{
	return source ? source->size / WAVE_SourceGetSampleSize( source ) : 0;
}

// source data fields cannot be default (e.g.0) values
// target data can contain default(e.g.0) values, for such their values will be copyied from the source ones
int WAVE_SourceConvert( const WAVESource* source, WAVESource *target )
{
	WAVE_ReportHeader( "WAVE_SourceConvert" );

	// Use Win32 API ACM to convert the sample
	WAVEFORMATEX iwf, owf;
	void *odata; // output data buffer
	int   osize; // output data size

	// target data pointer and length must be empty values
	if( target->data != 0 || target->size != 0 )
		return 0;

	// we must have valid data pointer and data size must be non-negative number
	if( source->data == 0 || source->size <= 0 )
		return 0;

	// wave source must be only mono or stereo
	if( source->chan != 1 && source->chan != 2 )
		return 0;

	// wave source must be 8 or 16 bits
	if( source->bits != 8 && source->bits != 16 )
		return 0;

	// wave source frequency must be non-negative
	if( source->freq <= 0 )
		return 0;

	memset( &iwf, 0, sizeof(iwf) );
	memset( &owf, 0, sizeof(owf) );

	iwf.wFormatTag      = WAVE_FORMAT_PCM;
	iwf.nChannels       = source->chan;
	iwf.wBitsPerSample  = source->bits;
	iwf.nSamplesPerSec  = source->freq;
	iwf.nBlockAlign     = iwf.wBitsPerSample*iwf.nChannels/8;
	iwf.nAvgBytesPerSec = iwf.nSamplesPerSec*iwf.nBlockAlign;

	owf.wFormatTag      = WAVE_FORMAT_PCM;
	owf.nChannels       = target->chan ? target->chan : source->chan;
	owf.wBitsPerSample  = target->bits ? target->bits : source->bits;
	owf.nSamplesPerSec  = target->freq ? target->freq : source->freq;
	owf.nBlockAlign     = owf.wBitsPerSample*owf.nChannels/8;
	owf.nAvgBytesPerSec = owf.nSamplesPerSec*owf.nBlockAlign;

	if( WIN32ACM_WaveConvert( &iwf, source->data, source->size, &owf, &odata, &osize ) )
	{
		// Only if we succeed then touche target data
		target->data = odata;
		target->size = osize;
		target->chan = target->chan ? target->chan : source->chan;
		target->bits = target->bits ? target->bits : source->bits;
		target->freq = target->freq ? target->freq : source->freq;
		return 1;
	}

	return 0;
}

int	WAVE_StereoSplit( const WAVESource *wave, void **left, void **right )
{
	WAVE_ReportHeader( "WAVE_StereoSplit" );
	if( (wave->chan == 2) && (wave->bits==8 || wave->bits==16) && wave->data && wave->size > 0 )
	{
		void  *L,	*R;
		char  *L8,  *R8;
		short *L16, *R16;
		int	  sampleCount;
		int	  sampleSize;

		sampleSize  = WAVE_SourceGetSampleSize(  wave );
		sampleCount = WAVE_SourceGetSampleCount( wave );

		L = WAVE_MemoryCalloc(sampleCount, sampleSize/2); L16 = (short*)L; L8 = (char*)L;
		R = WAVE_MemoryCalloc(sampleCount, sampleSize/2); R16 = (short*)R; R8 = (char*)R;
		if( L && R )
		{
			int   i;
			char  *S8  = (char *)wave->data;
			short *S16 = (short*)wave->data;

			if( wave->bits == 8 )
			{
				for( i=0; i<sampleCount; i++ )
				{
					L8[i] = S8[i*2+0];
					R8[i] = S8[i*2+1];
				}
			}
			else if( wave->bits == 16 )
			{
				for( i=0; i<sampleCount; i++ )
				{
					L16[i] = S16[i*2+0];
					R16[i] = S16[i*2+1];
				}
			}

			*left  = L;
			*right = R;

			return 1;
		}
		WAVE_MemoryFree( L );
		WAVE_MemoryFree( R );
	}
	return 0;
}

int	WAVE_StereoInterleave( int size, const void *L, const void *R, int blockSize, void *D )
{
	if( size > 0 && L && R && blockSize > 0 && D )
	{
		int n_blocks;
		int lastSize;

		n_blocks = size / blockSize;
		lastSize = size % blockSize;
		while( n_blocks-- )
		{
			memcpy( D, L, blockSize ); D = (char *)D + blockSize;
			memcpy( D, R, blockSize ); D = (char *)D + blockSize;
		}
		if( lastSize )
		{
			memcpy( D, L, lastSize ); D = (char *)D + blockSize;
			memcpy( D, R, lastSize ); D = (char *)D + blockSize;
		}

		return 1;
	}
	return 0;
}

int	WAVE_StereoInterleaveGetSize( int nonInterleavedStereoSize, int interleavedStereoBlockSize ) // calculates the size of interleaved stereo wave data
{
	int		monoTrackSize = (nonInterleavedStereoSize + 1) / 2;
	int		monoTrackAlignedSize = (monoTrackSize + interleavedStereoBlockSize - 1) / interleavedStereoBlockSize;
	return	monoTrackAlignedSize * 2 * interleavedStereoBlockSize; // 2 tracks - left&right
}

static struct  {
		int					targetFormat;
		char*				formatName;
		WAVE_ENCODE_FUNC	func;
} encoders[] = {
#define	ENC(x) { WAVE_TARGET_##x, #x, WAVE_Encode##x }
		ENC(GAMECUBE_ADPCM),
		ENC(XBOX_IMAADPCM),
		ENC(PS2_VAG),
#undef	ENC
};

int	WAVE_Encode( const WAVESource *source, WAVETarget *target )
{
	int i;
	for( i=0; i<sizeof(encoders)/sizeof(encoders[0]); i++ )
		if( encoders[i].targetFormat == target->dataFormat )
			return encoders[i].func( source, target );
	return 0;
}

int WAVE_TargetInit( WAVETarget* target )
{
	if( target )
	{
		memset( target, 0, sizeof(*target) );
		target->dataAlignment = 1;
	}
	return 1;
}

int WAVE_TargetFree( WAVETarget* target )
{
	if( target )
	{
		WAVE_MemoryFree( target->data );
		WAVE_MemoryFree( target->userData1 );
		WAVE_MemoryFree( target->userData2 );
		return WAVE_TargetInit( target );
	}
	return 1;
}

int WAVE_TargetSetDataFormat( WAVETarget *target, int targetFmt )
{
	if( target )
	{
		target->dataFormat = targetFmt;
		return 1;
	}
	return 0;
}

int WAVE_TargetSetDataAlignment( WAVETarget *target, int alignment )
{
	if( target )
	{
		target->dataAlignment = alignment;
		return 1;
	}
	return 0;
}

int	WAVE_TargetSetDataInterleavedStereoChunkSize( WAVETarget *target, int interleavedStereoBlockSize )
{
	if( target )
	{
		target->dataInterleavedStereoChunkSize = interleavedStereoBlockSize;
		return 1;
	}
	return 0;
}

int	WAVE_TargetDump( const WAVETarget *target, const char *targetName )
{
	if( target )
	{
		int i;
		printf("Dumping contents of: %s\n", targetName ? targetName : "<none>");
		printf("\tbits:       %10d\n", target->bits );
		printf("\tchannels:   %10d\n", target->chan );
		printf("\tfrequency:  %10d\n", target->freq );
		printf("\tdataFormat: %10d ", target->dataFormat );
		for( i=0; i<sizeof(encoders)/sizeof(encoders[0]); i++)
			if( encoders[i].targetFormat == target->dataFormat )
				printf("%s", encoders[i].formatName);
		printf("\n");
		printf("\tinterleave: %10d\n", target->dataInterleavedStereoChunkSize);
		printf("Encoder output:\n");
		printf("\tdata:                  %p\n", target->data);
		printf("\tdataSamples:         %10d\n", target->dataSamples);
		printf("\tdataSize:            %10d\n", target->dataSize);
		printf("\tuserData1:             %p\n", target->userData1);
		printf("\tuserData2:             %p\n", target->userData2);
		printf("\tuserData1Size:       %10d\n", target->userData1Size);
		printf("\tuserData2Size:       %10d\n", target->userData2Size);
		printf("\n");
	}
	return 0;
}

int WAVE_SourceIsValid( const WAVESource *source )
{
	WAVE_ReportHeader( "WAVE_SourceIsValid" );

	if( source )
	{
		if( source->chan < 1 || source->chan > 2 )
			WAVE_ReportWarning( "chan=%d, should be 1 or 2 (mono/stereo)", source->chan );

		if( source->bits != 8 && source->bits != 16 )
			WAVE_ReportWarning( "bits=%d, should be 8 or 16", source->bits );

		if( source->freq <= 0 || source->freq >= 50000 )
			WAVE_ReportWarning( "freq=%d" );

		if( source->data == NULL )
			WAVE_ReportWarning( "data=NULL" );

		if( source->size <= 0 )
			WAVE_ReportWarning( "size=%d", source->size );

		return 1;
	}
	return 0;
}

void* WAVE_TargetGetData( WAVETarget *target )
{
	if( target )
	{
		return target->data;
	}
	return NULL;
}

int	WAVE_TargetGetDataSize( WAVETarget *target )
{
	if( target )
	{
		return target->dataSize;
	}
	return NULL;
}

int	WAVE_TargetGetDataSamples( WAVETarget *target )
{
	if( target )
	{
		return target->dataSamples;
	}
	return NULL;
}

int	WAVE_TargetSetMono(	WAVETarget *target )
{
	if( target )
	{
		target->chan = 1;
		return 1;
	}
	return 0;
}

int	WAVE_TargetSetFrequency( WAVETarget *target, int freq )
{
	WAVE_ReportHeader( "WAVE_TargetSetFrequency" );
	if( target )
	{
		if( freq > 0 )
		{
			target->freq = freq;
			return 1;
		}
		WAVE_ReportError( "invalid frequency %d\n", freq );
	}
	return 0;
}

int	WAVE_TargetGetFrequency( WAVETarget *target )
{
	WAVE_ReportHeader( "WAVE_TargetGetFrequency" );
	if( target )
	{
		return target->freq;
	}
	return 0;
}
