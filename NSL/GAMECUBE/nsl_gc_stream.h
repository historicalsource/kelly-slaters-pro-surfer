#ifndef	NSL_GC_STREAM_HEADER
#define	NSL_GC_STREAM_HEADER

#include <dolphin/dvd.h>
#include <dolphin/arq.h>
#include <dolphin/ax.h>

#include "common/nsl.h"
#include "gamecube/nsl_gc.h"

enum nslStreamStateEnum
{
	NSL_STREAM_STATE_CLOSED,	// set right after nslStreamClose() is being called
	NSL_STREAM_STATE_IDLE,		// set right after nslStreamOpen is being called
								// set		 after nslStreamStop() finishes stopping (should not take long time)
								// set		 after nslStreamPlay() finishes playing music (should take long time)
	NSL_STREAM_STATE_LOADING,	// set right after nslStreamLoad() is being called
	NSL_STREAM_STATE_LOADED,	// set		 after nslStreamLoad() finishes
	NSL_STREAM_STATE_PLAYING,	// set		 after nslStreamPlay() call succededs and starts playing
	NSL_STREAM_STATE_PAUSED,
	NSL_STREAM_STATE_INTERRUPTED,
	
	NSL_STREAM_NUM_STATES
};

struct nslStream;

struct nslStreamChannel
{
	friend struct nslStream;
	
private:
	ARQRequest	 arqRequest;
	AXVPB*		 voice;
	nslStream*	 parentStream;
	char*		 memoryBuffer[2];	// Double-buffered memory buffer
	char*		 audioBuffer[2];	// Double-buffered audio buffer
	u32			 sampleStart;		// this is the same as audioBuffer[0] but with sample addressing
	volatile int isTransfering;		// Is DMA memory->audio transfer currently active? (ARQPostRequest)
	
	static	void AXVoiceDroppedCallbackBridge( void *voiceHandle );
			void AXVoiceDroppedCallback();
	static	void ARQPostRequestCallbackBridge( u32 reqAdr );
			void ARQPostRequestCallback();
			void Transfer();
			void ProcessHeader();
};

struct nslStream 
{
	friend struct nslStreamChannel;
	
private:
	enum {					  				///////////////////////////////////////////////////////////////////////////////////////
		adpcmHeaderSize = 64				// AXPBADPCM headerSize (acutally AXPBADPCM is less than 64 but we may need extra data)
	};
	
	// Stream format info					////////////////////////////////////////////////////////////
	DVDFileInfo file;						// This must be the first member of 'nslStream'
	int sampleRate;							// 11025Khz, 22050Khz, 44100Khz
	int sampleFormat;						// AX_PB_FORMAT_ADPCM, AX_PB_FORMAT_PCM16, AX_PB_FORMAT_PCM8
	int channelCount;						// 1-mono, 2-stereo, 3-stereoSurround?
	int chunkSize;							// 2048, 4096, 8192, 32768
	
	// Memory&Audio bufs   				    ////////////////////////////////////////////////////////////////////
	void* memoryBuffers;					// size = chunkSize * channelCount * 2, must be aligned to 32
	void* audioBuffers;						// size = chunkSize * channelCount * 2, must be aligned to 32
	
	// Precalculated stuff 					////////////////////////////////////////////////////////////////////
	int chunksTotal;						// How many chunks are in the stream
	u32 sampleHalfSize;						// sample addressing - 
	u32	sampleLastChunkSize;				// sample addressing - 
	
	// Stream channels (stereo)	
	nslStreamChannel channels[2];

	// Variable stuff						/////////////////////////////////////////////////////////////
	volatile nslStreamStateEnum state;		// NSL_STREAM_STATE_xxxx
	volatile nslStreamStateEnum beforePauseState;
	volatile int isReading;					// Is there any DVDReadAsync active?
	volatile int fileOffset;				// Current file position from where we read the audio chunks
	volatile int chunksReaded;				// How many chunks have been readed
	volatile int chunksTransfered;			// How many chunks have been readed & transfered through dma
	volatile int chunksPlayed;				// How many chunks have been played
	volatile int oldPlayingBuffer;
	volatile int oldReadingBuffer;
	
	// nsl stuff
	float oldVolume;
	float volume;
	bool playRequested;

public:
	/// Functions							/////////////////////////////////////////////////////////////////////
	int Open( const char *, const nslStreamInfo *info, char *userAudioBuf, bool reverb );	// File from which samples will be readed - all must be in one format
	int Load( int offset, int samples );	// From where sample starts to starts, and how many samples it got 
	int Play();
	int Stop();
	int Close();
	int Pause();
	int Rewind();
	int Update();
	int Unpause();
	nslStreamStateEnum GetState()
	{
		return state;
	}
	void  SetVolume( float volume );
	float GetVolume( void );
	
private:
	void UpdateVolume();
	int	IsReading();
	int	IsTransfering();
	int	IsReadingOrTransfering();
	int	IsNotReadingOrTransfering();
	int Read();
	
	static	void DVDReadAsyncCallbackBridge( s32 result, DVDFileInfo* fileInfo );
			void DVDReadAsyncCallback();
};

int					nslStreamOpen(		const char *filename, const nslStreamInfo *streamInfo, bool reverb );
int					nslStreamLoad(		int streamId, int offset, int samples );
int					nslStreamPlay(		int streamId );
int					nslStreamStop(		int streamId );
int					nslStreamClose(		int streamId );
int					nslStreamPause(		int streamId );
int					nslStreamRewind(	int streamId );
int					nslStreamUnpause(	int streamId );
int					nslStreamUpdate(	int streamId );
nslStreamStateEnum	nslStreamStatus(	int streamId );
void				nslStreamSetVolume(	int streamId, float volume );
float				nslStreamGetVolume(	int streamId );

#endif