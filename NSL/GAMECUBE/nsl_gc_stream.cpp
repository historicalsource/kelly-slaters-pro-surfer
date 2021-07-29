#include "gamecube/nsl_gc.h"
#include "gamecube/nsl_gc_stream.h"
#include <dolphin/os.h>
#include <dolphin/mix.h>
#include <dolphin/ar.h>

#define GETHILO(id) (((u32)(id##Hi)<<16)|((u32)(id##Lo)))
#define OSReport	if(0)

///////////////////////////////////////////////////////////////////////////////////////////////////

void nslStreamChannel::Transfer(void) 
{
	int isNotReadingOrTransfering; 
	isNotReadingOrTransfering = (isTransfering == false) && (parentStream->isReading == false );

	// We must make sure that this channel is not in DMA transfer and DVD async reader is idle	
	NSL_ASSERT( isNotReadingOrTransfering );
	if( isNotReadingOrTransfering )
	{
		int chunk;
		
		isTransfering = true;
		chunk = parentStream->chunksReaded;

        // If this is the last chunk from the stream remove the LOOPING and ADJUST the correct ENDING POSITION
        // This way voice when ends will be auto-released by falling into AXVoiceDroppedCallback() function
        if( parentStream->sampleFormat == AX_PB_FORMAT_ADPCM )
		{
			// NOTE: I'm confused - whether should I put this thing here or not...
			// In AXSTREAM.C they have this code for each buffer loop (rerun)
			//
			// But in the same time they have AX_PB_TYPE_STREAM set.... 
			// the funny part is that this even works with AX_PB_TYPE_NORMAL set
			//
			// Well actually it doesn't matter (at least doesn't sound different to me)
			// if I add this code or not... also doesn't matter if I set AX_PB_TYPE_NORMAL 
			// or AX_PB_TYPE_STREAM
			//
			if( (chunk&1) == 0 )
			{
		        u8 *data;
		        AXPBADPCMLOOP loop;
		        data					= (u8 *)memoryBuffer[0];
		        loop.loop_pred_scale	= *(data + ((chunk==0) ? parentStream->adpcmHeaderSize : 0) );
		        loop.loop_yn1			= 0;
		        loop.loop_yn2			= 0;
		        AXSetVoiceAdpcmLoop( voice, &loop );
	        }
        }
        
		DCFlushRange( memoryBuffer[chunk&1], (u32) parentStream->chunkSize );
		ARQPostRequest( 
			&arqRequest,
			(u32) 0,
			(u32) ARQ_TYPE_MRAM_TO_ARAM,
			(u32) ARQ_PRIORITY_HIGH,
			(u32) memoryBuffer[chunk&1],
			(u32) audioBuffer[chunk&1],
			(u32) parentStream->chunkSize,
			ARQPostRequestCallbackBridge
		);
	}
}

void nslStreamChannel::ARQPostRequestCallbackBridge( u32 reqAdr )
{
	nslStreamChannel *streamChannel;
	streamChannel = (nslStreamChannel *) reqAdr;
	NSL_ASSERT( streamChannel );
	if( streamChannel )
		streamChannel->ARQPostRequestCallback();
}

void nslStreamChannel::ARQPostRequestCallback( )
{
	// Audio data was successfully transfered over the DMA and right now it's here
	// Switch to the next buffer
	isTransfering = false;
	if( parentStream->IsNotReadingOrTransfering() )
		parentStream->chunksTransfered++;
	OSReport("DMA Transfer finished: %p/%p\n", parentStream, this );
}

// Releasing the channel
void nslStreamChannel::AXVoiceDroppedCallbackBridge( void *voiceHandle )
{
	voiceHandle;
//	NSL_ASSERT(0);
}

void nslStreamChannel::AXVoiceDroppedCallback()
{
//	NSL_ASSERT(0);
}

void nslStreamChannel::ProcessHeader()
{
	switch( parentStream->sampleFormat )
	{
		// Gamecube ADPCM stream format begins with 64 bytes header.
		// First 40 bytes are the AXPBAPDCM header, 
		// The other 24 are just empty for now
		case AX_PB_FORMAT_ADPCM:
			{
				AXPBADPCM *adpcm;
				u8 *data;
				data				= (u8 *)memoryBuffer[0];
				adpcm				= (AXPBADPCM *)data;
				adpcm->gain			= 0;
				adpcm->pred_scale	= *(data + nslStream::adpcmHeaderSize);
				adpcm->yn1			= 0;
				adpcm->yn2			= 0;
				AXSetVoiceAdpcm( voice, adpcm );
			}
			break;
			
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void nslStream::DVDReadAsyncCallbackBridge( s32 result, DVDFileInfo* fileInfo )
{
	nslStream *stream;
	
	stream = (nslStream *)fileInfo;
	NSL_ASSERT( stream );
 
	if( stream && result >= 0 )
	{
		NSL_ASSERT( result == stream->chunkSize * stream->channelCount );
		stream->DVDReadAsyncCallback();
	}
}

void nslStream::DVDReadAsyncCallback()
{
	int i;

	isReading = false;
	
	// Data was successfully transfered off the DVD to memoryBuffer's
	// Now it's time to transfer each channel through DMA using ARQPostRequest()
	for( i=0; i<channelCount; i++ )
		channels[i].Transfer();

	// First chunk (chunksReaded==0) contains header for the sound data
	// for PCM8 and PCM16 this header will be probably of size 0
	// but for ADPCM it's size is 64(adpcmHeaderSize) and it starts as regular AXPBADPCM structure
	// this structure must be setup with AXSetVoiceAdpcm
	if( chunksReaded == 0 )
	{		
		for( i=0; i<channelCount; i++ )
			channels[i].ProcessHeader();
	}
	
	chunksReaded ++;
	fileOffset += chunkSize * channelCount;
}

// Read should not be called for the last chunk (e.g. when chunksReaded == chunksTotal
int nslStream::Read()
{
	// Read all channels at once
	
	int	chunksEof;
	
	chunksEof = (unsigned)chunksReaded >= (unsigned)chunksTotal;
	
	NSL_ASSERT( chunksEof == false );
	if( chunksEof == false )
	{
		int isReadingOrTransfering;
		
		isReadingOrTransfering = IsReadingOrTransfering();
		
		NSL_ASSERT( isReadingOrTransfering == false );
		if( isReadingOrTransfering == false )
		{
			isReading = DVDReadAsync( &file, channels[0].memoryBuffer[ chunksReaded&1 ], chunkSize * channelCount, fileOffset, DVDReadAsyncCallbackBridge );
			NSL_ASSERT( isReading );
			return isReading;
		}
	}
	return 0;
}

int nslStream::Open( const char *filename, const nslStreamInfo *info, char *userAudioBuffer, bool reverb )
{
	NSL_ASSERT(info);
	if( info && (state == NSL_STREAM_STATE_CLOSED) )
	{
		int	i, shift;
		BOOL openOk;
		
		NSL_ASSERT(info->sampleRate>=4800 && info->sampleRate<=48000);
		NSL_ASSERT(info->sampleFormat==NSL_STREAM_FORMAT_ADPCM||info->sampleFormat==NSL_STREAM_FORMAT_PCM16||info->sampleFormat==NSL_STREAM_FORMAT_PCM8);
		NSL_ASSERT(info->channelCount==1 || info->channelCount==2);
		NSL_ASSERT(info->chunkSize>0 && !(info->chunkSize&(info->chunkSize-1))); // check if it's power of two
		
		sampleRate		= info->sampleRate;
		channelCount	= info->channelCount;
		chunkSize		= info->chunkSize;
		
		char dvdFilename[256];
		strcpy( dvdFilename, filename );
		
		openOk = DVDOpen( dvdFilename, &file );
		NSL_ASSERT( openOk );
		if( openOk == false )
			return 0;

		// Acquire voices for each channel
		for( i=0; i<channelCount; i++ )
		{
			channels[i].voice = AXAcquireVoice( AX_PRIORITY_NODROP, channels[i].AXVoiceDroppedCallbackBridge, (u32) &channels[i] );
			NSL_ASSERT( channels[i].voice );
			if( channels[i].voice == NULL )
			{
				int j;
				DVDClose( &file );
				for( j=0; j<i; j++ )
					AXFreeVoice( channels[j].voice );
				return 0;
			}
		}
		
		switch( info->sampleFormat )
		{
			case NSL_STREAM_FORMAT_ADPCM:
				sampleFormat = AX_PB_FORMAT_ADPCM;
				shift = 2;
				break;
				
			case NSL_STREAM_FORMAT_PCM8:
				sampleFormat = AX_PB_FORMAT_PCM8;
				shift = 1;
				break;
				
			case NSL_STREAM_FORMAT_PCM16:
				sampleFormat = AX_PB_FORMAT_PCM16;
				shift = 0;
				break;
		}
		
		sampleHalfSize	= ((u32)chunkSize << shift) >> 1;
		memoryBuffers	= nslMemAlloc( (unsigned)( chunkSize * channelCount * 2 ), 32 );
		audioBuffers	= userAudioBuffer;
		for( i=0; i<channelCount; i++ )
		{
			channels[i].parentStream		= this;
			channels[i].memoryBuffer[0]		= (char*)((u32)memoryBuffers + chunkSize * ( channelCount * i + 0 ));
			channels[i].memoryBuffer[1]		= (char*)((u32)memoryBuffers + chunkSize * ( channelCount * i + 1 ));
			channels[i].audioBuffer[0]		= (char*)((u32) audioBuffers + chunkSize * ( channelCount * 0 + i ));
			channels[i].audioBuffer[1]		= (char*)((u32) audioBuffers + chunkSize * ( channelCount * 1 + i ));
			channels[i].sampleStart			= ((u32)channels[i].audioBuffer[0] << shift) >> 1;
			channels[i].isTransfering		= 0;
		}
		
		beforePauseState = NSL_STREAM_STATE_CLOSED;

		// Voice setup
		for( i=0; i<channelCount; i++ )
		{
			AXPBSRC src;
			u32	ratio;
		
			switch( sampleFormat )
			{
				case AX_PB_FORMAT_ADPCM:
					AXSetVoiceType( channels[i].voice, AX_PB_TYPE_STREAM );
					break;
											
				case AX_PB_FORMAT_PCM8:
				case AX_PB_FORMAT_PCM16:
					AXSetVoiceType( channels[i].voice, AX_PB_TYPE_NORMAL );
					break;
			}
			
			ratio = 256 * (u32)sampleRate / 125;
			
			src.ratioHi				= (u16)( ratio >> 16 );
			src.ratioLo				= (u16)( ratio & 0xFFFF );
			src.currentAddressFrac	= 0;
			src.last_samples[0]		= 0;
			src.last_samples[1]		= 0;
			src.last_samples[2]		= 0;
			src.last_samples[3]		= 0;

			if( channelCount == 1 )
			{
				if (reverb)
				{
					// Have the volume sent to AuxA at maximum of 0 because AuxA is set up for reverb.

					MIXInitChannel( channels[i].voice, 0, -960, 0, -960, 64, 127, 0 );
				}
				else
				{
					// Send no sound to AuxA or AuxB. -904 is the lowest volume so I'm told.

					MIXInitChannel(	channels[i].voice, 0, -960, -960, -960, 64, 127, 0 );
				}
			}
			else if( channelCount == 2 )
			{
				if (reverb)
				{
					// Have the volume sent to AuxA at maximum of 0 because AuxA is set up for reverb.

					MIXInitChannel(	channels[i].voice, 0, -960, 0, -960, i * 127, 127, 0 );
				}
				else
				{
					// Send no sound to AuxA or AuxB. -904 is the lowest volume so I'm told.

					MIXInitChannel(	channels[i].voice, 0, -960, -960, -960, i * 127, 127, 0 );
				}
			}

			AXSetVoiceSrcType(	channels[i].voice, AX_SRC_TYPE_4TAP_16K );
			AXSetVoiceSrc(		channels[i].voice, &src );
		}
		
		state = NSL_STREAM_STATE_IDLE;
		return 1;
	}
	return 0;
}

int nslStream::Close( void )
{
	if( state == NSL_STREAM_STATE_IDLE )
	{
		int i;
		BOOL closeOk;
		
		Stop();

		for( i=0; i<channelCount; i++ )
		{
			MIXReleaseChannel( channels[i].voice );
			AXFreeVoice( channels[i].voice );
			channels[i].voice = NULL;
		}
					
		state = NSL_STREAM_STATE_CLOSED;
		
		nslMemFree( memoryBuffers );
		memoryBuffers = 0;

		playRequested = false;

		closeOk = DVDClose( &file );
		NSL_ASSERT( closeOk );
		return closeOk;
	}
	else
	{
		NSL_ASSERT( 0 );
		nslPrintf( "nslStream::Close(): unclosed source, possible resource leak.\n" );
	}
	return 0;
}

int nslStream::Load( int offset, int samples )
{
	// Only if we are idle then we can start new stream sample
	if( state == NSL_STREAM_STATE_IDLE )
	{
		int i, bytesInStream, bytesInLastChunk, shift;
		
		state = NSL_STREAM_STATE_LOADING;
		switch( sampleFormat )
		{
			case AX_PB_FORMAT_ADPCM:
				bytesInStream = ((samples + 13)/14) * 8 + adpcmHeaderSize;
				shift = 2;
				break;
				
			case AX_PB_FORMAT_PCM8:
				bytesInStream = samples;
				shift = 1;
				break;
				
			case AX_PB_FORMAT_PCM16:
				bytesInStream = samples * 2;
				shift = 0;
				break;
		}
		
		oldPlayingBuffer	= -1;
		oldReadingBuffer	= -1;
		chunksPlayed		= -1;
		chunksReaded		= 0;
		chunksTransfered	= 0;
		fileOffset			= offset;
		chunksTotal			= (bytesInStream + chunkSize - 1) / chunkSize;
		bytesInLastChunk	= bytesInStream % chunkSize; 
		sampleLastChunkSize = ((u32)bytesInLastChunk << shift) >> 1;
		
		for( i=0; i<channelCount; i++ )
			channels[i].isTransfering = 0;
		
		for( i=0; i<channelCount; i++ )
		{
			AXPBADDR addr;
			u32 startA, endA, loopA;
			
			startA	= channels[i].sampleStart;
			loopA	= channels[i].sampleStart;
			endA	= channels[i].sampleStart + sampleHalfSize*2 - 1;
			
			if( sampleFormat == AX_PB_FORMAT_ADPCM )
			{
				startA += adpcmHeaderSize*2 + 2;
				loopA  += 2;
			}

			addr.format				= (u16) sampleFormat;
			addr.loopFlag			= AXPBADDR_LOOP_ON;
			addr.currentAddressHi	= (u16)( startA >> 16 );
			addr.currentAddressLo	= (u16)( startA & 0xFFFF );
			addr.loopAddressHi		= (u16)( loopA >> 16 );
			addr.loopAddressLo		= (u16)( loopA & 0xFFFF );
			addr.endAddressHi		= (u16)( endA >> 16 );
			addr.endAddressLo		= (u16)( endA & 0xFFFF );
			AXSetVoiceAddr(	channels[i].voice, &addr );
		}
		Read();
		return 1;
	}
	return 0;
}

int nslStream::Play( void )
{
	// We can start play only if first portion was loaded
	if( state == NSL_STREAM_STATE_LOADED )
	{
		state = NSL_STREAM_STATE_PLAYING;

		UpdateVolume( );
			
		for( int i = 0; i < channelCount; i++ )
		{
			AXSetVoiceState( channels[i].voice, AX_PB_STATE_RUN );
		}
			
		oldVolume = volume;
		return 1;
	}
	else if( state == NSL_STREAM_STATE_LOADING )
	{
		playRequested = true;
		return 1;
	}
	return 0;
}

int nslStream::Pause( void )
{
	// On closed state - we just shut our eyes and continue
	NSL_ASSERT( (unsigned) state < NSL_STREAM_NUM_STATES );
	if( state == NSL_STREAM_STATE_CLOSED ||	state == NSL_STREAM_STATE_PAUSED )
	{
		return 0;
	}

	// For all other states we are like this:
	if( state == NSL_STREAM_STATE_PLAYING )
	{
		int i;
		for( i=0; i<channelCount; i++ )
			AXSetVoiceState( channels[i].voice, AX_PB_STATE_STOP );
	}
	
	beforePauseState = state;
	state = NSL_STREAM_STATE_PAUSED;
	return 1;
}

int nslStream::Unpause( void )
{
	if(	state == NSL_STREAM_STATE_PAUSED )
	{
		int i;
		state = beforePauseState;
		if( state == NSL_STREAM_STATE_PLAYING )
			for( i=0; i<channelCount; i++ )
				AXSetVoiceState( channels[i].voice, AX_PB_STATE_RUN );
		return 1;
	}
	return 0;
}

int nslStream::Stop( void )
{
	int i;

	switch( state )
	{
		case NSL_STREAM_STATE_LOADING:
		case NSL_STREAM_STATE_LOADED:
		case NSL_STREAM_STATE_PLAYING:
		case NSL_STREAM_STATE_PAUSED:
		case NSL_STREAM_STATE_INTERRUPTED:
			for( i=0; i<channelCount; i++ )
				AXSetVoiceState( channels[i].voice, AX_PB_STATE_STOP );
			if( IsReading() )
			{
				// this needs to be blocking, duh
				DVDCancel( &file.cb );
				isReading = false;
			}
			state = NSL_STREAM_STATE_IDLE;
			return 1;
	}
	return 0;
}

int nslStream::Rewind( void )
{
	//TODO!
	return 1;
}

void nslStream::UpdateVolume( void )
{

	for( int i = 0; i < channelCount; ++i )
	{
		int atten = _nslCalcDecibelAtten( volume );
		nslVerboseStreamPrintf( "nslStream::UpdateVolume( ): setting atten to %d.\n", atten );
		MIXSetInput( channels[i].voice, atten );
	}

}

int nslStream::Update( void )
{
	NSL_ASSERT( (unsigned)state < NSL_STREAM_NUM_STATES );
	switch( state )
	{
		case NSL_STREAM_STATE_PLAYING:
			if( volume != oldVolume )
			{
				UpdateVolume( );
				oldVolume = volume;
			}
			{
				u32	curr;
				int playingBuffer;
				int	bufferGotChanged;
				
				curr = GETHILO( channels[0].voice->pb.addr.currentAddress ) - channels[0].sampleStart;
				NSL_ASSERT( curr < sampleHalfSize * 2 );
				
				playingBuffer = ( curr >= sampleHalfSize );
				if( playingBuffer )
					curr -= sampleHalfSize;
					
				bufferGotChanged = ( oldPlayingBuffer != playingBuffer );
				if( bufferGotChanged )
				{
					oldPlayingBuffer = playingBuffer;
					chunksPlayed ++;
				}
				
				if( chunksPlayed + 1 >= chunksTotal )
				{
					if( curr >= sampleLastChunkSize )
					{
						Stop();
						return 1;
					}
				}
				else if( chunksPlayed >= chunksTransfered )
				{

					// Disc cover could have been opened, etc.
					for( int i = 0; i < channelCount; ++i ) {
						AXSetVoiceState( channels[i].voice, AX_PB_STATE_STOP );
					}

					state = NSL_STREAM_STATE_INTERRUPTED;
					return 1;
				}
				else if( IsNotReadingOrTransfering() )
				{
					if( chunksReaded < chunksTotal && oldReadingBuffer != playingBuffer )
					{
						oldReadingBuffer = playingBuffer;
						Read();
						return 1;
					}
				}
			}
			return 0;
			
		case NSL_STREAM_STATE_LOADING:
			if( IsNotReadingOrTransfering() )
			{
				state = NSL_STREAM_STATE_LOADED;
				
				if( playRequested )
				{
					playRequested = false;
					Play( );
				}
			}	
			return 0;

		case NSL_STREAM_STATE_CLOSED:
		case NSL_STREAM_STATE_IDLE:
		case NSL_STREAM_STATE_PAUSED:
		case NSL_STREAM_STATE_LOADED:
			return 0;
			
		case NSL_STREAM_STATE_INTERRUPTED:

			if( chunksTransfered > chunksPlayed ) {

				for( int i = 0; i < channelCount; ++i ) {
					AXSetVoiceState( channels[i].voice, AX_PB_STATE_RUN );
				}

			}

			state = NSL_STREAM_STATE_PLAYING;
			return 0;
	}
	return 0;
}

int nslStream::IsReading()
{
	return isReading;
}

int nslStream::IsTransfering()
{
	int i, isTransfering;
	isTransfering = false;
	for( i=0; i<channelCount; i++ )
		if( channels[i].isTransfering )
			isTransfering = true;
	return isTransfering;
}

int nslStream::IsReadingOrTransfering()
{
	return IsReading() || IsTransfering();
}
int nslStream::IsNotReadingOrTransfering()
{
	return !IsReadingOrTransfering();
}

float nslStream::GetVolume( void )
{
	return volume;
}

void nslStream::SetVolume( float newVolume )
{
	if( newVolume < 0.0f )
		newVolume = 0.0f;
	if( newVolume > 1.0f )
		newVolume = 1.0f;
	volume = newVolume;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//enum {
//	NSL_NUM_STREAMS = 16
//};

static nslStream nslStreams[ NSL_NUM_STREAMS ];

int nslStreamOpen( const char *_filename, const nslStreamInfo *streamInfo, bool reverb )
{
	int i;
	char filename[256], *c;
	strcpy( filename, nsl.root );
	strcat( filename, "\\" );
	strcat( filename, _filename );
	for( c=filename; *c; c++ )
		if( *c == '\\' )
		    *c = '/';
	nslVerboseStreamPrintf("nslStreamOpen(%s -> %s)\n",_filename,filename);
	for( i=0; i<NSL_NUM_STREAMS; i++ )
	{
		if( nslStreams[i].Open(	filename,
								streamInfo,
								(char *)nsl.streams[i].audioBufferStart,
								reverb))
			return i;
	}
	return -1;
}

int nslStreamLoad( int streamId, int offset, int samples )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Load( offset, samples );
	}
	return 0;
}

int nslStreamPlay( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Play();
	}
	return 0;
}

int nslStreamStop( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Stop();
	}
	return 0;
}

int nslStreamClose( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Close();
	}
	return 0;
}

int nslStreamPause(	int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Pause();
	}
	return 0;
}

int nslStreamRewind( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Rewind();
	}
	return 0;
}

int nslStreamUnpause( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Unpause();
	}
	return 0;
}

int nslStreamUpdate( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].Update();
	}
	return 0;
}

nslStreamStateEnum nslStreamStatus( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].GetState();
	}
	return NSL_STREAM_STATE_CLOSED;
}

void nslStreamSetVolume( int streamId, float volume )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			nslStreams[streamId].SetVolume(volume);
	}
}

float nslStreamGetVolume( int streamId )
{
	if( streamId != -1 )
	{
		NSL_ASSERT( (unsigned)streamId < NSL_NUM_STREAMS );
		if( (unsigned)streamId < NSL_NUM_STREAMS )
			return nslStreams[streamId].GetVolume();
	}
	return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////
