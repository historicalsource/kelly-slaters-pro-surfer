#include "libWave.h"
#include "libWaveXBOX_IMAADPCM.h"

int WAVE_EncodeXBOX_IMAADPCM( const WAVESource *_source, WAVETarget *target )
{
	WAVE_ReportHeader( 
		"WAVE_EncodeXBOX_IMAADPCM:%d:%d:%dHz:%d:%s->%dKhz:%s", 
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

	if( WAVE_SourceIsValid( _source ) && WAVE_SourceInit( &source ))
	{
		source.bits = 16; // IMAADPCM requires 16 bits
		source.freq = target->freq;
		source.chan = target->chan;
		if( WAVE_SourceConvert( _source, &source ) )
		{
			IMAADPCMWAVEFORMAT*	adpcmInfo;

			adpcmInfo = (IMAADPCMWAVEFORMAT *)WAVE_MemoryCalloc(1,sizeof(*adpcmInfo));
			if( adpcmInfo )
			{
				CImaAdpcmCodec codec;

			  void*	adpcmBuffer;
				int	adpcmBytes;
				int	adpcmBlocks;
				int	sampleSize;
				int	sampleCount;

				codec.CreateImaAdpcmFormat( source.chan, source.freq, 64, adpcmInfo );
				codec.Initialize( adpcmInfo, TRUE );

				sampleSize  = WAVE_SourceGetSampleSize(  &source );
				sampleCount = WAVE_SourceGetSampleCount( &source );

				adpcmBlocks = (sampleCount + adpcmInfo->wSamplesPerBlock - 1) / adpcmInfo->wSamplesPerBlock;
				adpcmBytes  = adpcmBlocks * adpcmInfo->wfx.nBlockAlign;

				adpcmBytes  = (adpcmBytes + target->dataAlignment - 1) / target->dataAlignment;
				adpcmBytes *= target->dataAlignment;

				adpcmBuffer = WAVE_MemoryCalloc( 1, adpcmBytes );
				if( adpcmBuffer )
				{
					if( codec.Convert( source.data, adpcmBuffer, adpcmBlocks ) && WAVE_TargetInit(target) )
					{
						target->dataFormat		= WAVE_TARGET_XBOX_IMAADPCM;
						target->bits			= source.bits;
						target->chan			= source.chan;
						target->freq			= source.freq;
						target->data			= adpcmBuffer;
						target->dataSize		= adpcmBytes;
						target->dataSamples		= sampleCount;
						target->userData1		= adpcmInfo;
						target->userData1Size	= sizeof(*adpcmInfo);
						result					= 1;
					}
					else
					{
						WAVE_MemoryFree( adpcmBuffer );
						WAVE_MemoryFree( adpcmInfo );
					}
				}
				else
				{
					WAVE_MemoryFree( adpcmInfo );
				}
			}
			WAVE_SourceFree( &source );
		}
	}
	return result;
}
