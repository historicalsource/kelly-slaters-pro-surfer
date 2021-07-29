#ifndef NSL_XBOX_HEADER
#define NSL_XBOX_HEADER
//
// Sound Library - API
//
// Treyarch LLC, June 2001
//
// (designed by Wade Brainerd, Jamie Fristrom, Andy Chien, and Greg Taylor)
//
#include "../common/nsl.h"
#include <xtl.h>
// Keep assert.h out! - KS 3/15/02  
//#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <dsound.h>
#include "nsl_xbox_streams.h"
#include <xgmath.h>
// ======================================================================================
//
//
// System functions
//
//
//
//
// Defines, forward decolation and static variables
//
//
#define DISABLE_EMITTERS		0
#define DISABLE_SOUNDS			0
#define DISABLE_STREAMS			1       // This enables the NEW STREAMS!
#define DISABLE_NEW_STREAMS 0

#define ZERO_STRUCTURE( x ) (memset( &x, 0, sizeof( x ) ))
#define CHECK_ID( x ) { if (!_nslCheckSoundId(x)) return; }
#define CHECK_ID2( x, y ) { if (!_nslCheckSoundId(x)) return (y); }
#define RETURN_ON_INVALID_ID(x) { if (x == NSL_INVALID_ID) return; }
#define RETURN_WITH_VALUE_ON_INVALID_ID(x, y) { if (x == NSL_INVALID_ID) return (y); }
#define check_dsound_result( hr ) { if( FAILED(hr) ) nslGetDsErrorMsgXbox( __LINE__, hr ); }
#define DS_TRY( x ) check_dsound_result( x )
#define INC_WRAP( x, y ) { if ( ++x > y ) x = 0; }


#if defined(NSLDEBUG)
extern void _nslAssert(const char* exp_str, const char* file_name, int line);
#define internalAssert(exp) { if(!(exp)) _nslAssert(#exp, __FILE__, __LINE__); }
#else
#define internalAssert(exp) {}
#endif

#define NSL_NUM_SOUND_SOURCES		4096
#define NSL_NUM_EMITTERS				256
// Maximum number of sounds can be played simultaneously.
// This most likely is hardware dependent.
// The actual number of sounds is the define - 1 because the zeroth slot is invalid.
// Yes, we are wasting a slot but we wouldn't need to deal with one off problem every 
// where and it's only a few bytes of space wasted.
#define NSL_NUM_SOUNDS					256

#if !DISABLE_STREAMS
#define NSL_NUM_STREAM_SOUNDS		6
#define FILESTRM_PACKET_COUNT		3
// This value is hard-coded assuming an ADPCM frame of 36 samples and 16
// bit stereo (128 frames per packet) - from MS example
// Asen -- I don't think it will be ALWAYS stereo - bad assumption!
#define FILESTRM_PACKET_BYTES 2 * 2 * 36 * 128
#endif

// the way sound ID works is that higher bits is the unique ID given out by the system
// every time a new sound is requested while the lower 8 bits are used to determine which
// voice channel the sound is intended for.
#define NSL_ID_BITS			8
#define NSL_ID_MASK			0xFFFFFF00
#define NSL_SOUND_MASK	0x000000FF
#define NSL_CALC_ID( x, y ) (x<<NSL_ID_BITS)|y	// x = source id, y = sound slot
#define NSL_SLOT( x ) ( x&NSL_SOUND_MASK )

// ======================================================================================
//
//
// nslSource structure and functions 
//
//
#include "nsl_xbox_source.h"

// this struct is used by the sound file that were loaded up to the memory
typedef struct nslSource 
{
	nslSourceId							id;
	bool										isLoaded;
	bool										isLooped;

	XBOXADPCMWAVEFORMAT			format;
  DSBUFFERDESC						desc;
	void*										data;

	nslXBoxSndEntry					sndEntry;

	nslSoundStatusEnum			status;

	HANDLE file_handle;

} _nslSource;


// ======================================================================================
//
//
// nslSound structure and functions
//
// 

// This struct is internal to NSL.

#if !DISABLE_STREAMS
typedef unsigned int nslStreamId;
typedef struct nslStreamSound
{
	nslSoundId					soundId;
	
	bool								isUsed;
	bool								isInitialized;
	bool								startPlay;	
	bool								shouldDie;			// This tells the stream thread that this stream needs to die asap.
	bool								almostDone;
	bool								endStream;

	HANDLE							hFile;																	// Stream file handle
	IDirectSoundStream* pRenderFilter;                          // Render (DirectSoundStream) filter
	void*								pvSourceBuffer;                         // Source filter data buffer
	DWORD               adwPacketStatus[FILESTRM_PACKET_COUNT]; // Packet status array
	nlUlong							fileLength;															// Stream file length
	nlUlong							fileOffset;															// Current offset from beginning of the stream file
} _nslStreamSound;
#endif

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
typedef signed int nslNewStreamId;
typedef struct nslNewStreamSound
{
	nslSoundId					soundId;
	
	IDirectSoundStream* pRenderFilter;                          // Render (DirectSoundStream) filter
	void *              pvSourceBuffer[XBoxStreamPackets];      // Source filter data buffer
	DWORD               dwPacketStatus[XBoxStreamPackets];      // Packet status array
  bool                isPacketReady[XBoxStreamPackets];
  DWORD               dwCompletedSize[XBoxStreamPackets];     // Current progress
  DWORD               dwTransferred[XBoxStreamPackets];       // How much did we read?
  XMEDIAPACKET        xmp;
  
  int                 currentPacket;                          // Current playing packet
  int                 playingPacket;                          // Current playing packet
  int                 lastPacket;                             // Reached the end
  bool                isInitialized;
  bool                startPlay;

  bool                shouldDie;                              // This stream has to be stopped
  bool                almostDead;                             // Waiting for it to die (Angels deserve to die?!)
  
	nlUlong							fileOffset;															// Current offset from beginning of the stream file
	nlUlong							fileLength;															// Stream file length
} _nslStreamSound;
#endif

typedef struct nslSound
{
  // reference to the data in case we lose the buffer and need to copy the data again.  
  nslSoundId		id;
	nslEmitterId	eId;	// the emitter that contains this sound
	nslSourceId		srcId;	// the source id for this sound

#if !DISABLE_STREAMS
	nslStreamId		streamSlot;	// if this sound is streamed the Slot is set to the correct index, otherwise NSL_INVALID_ID
#endif

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
  nslNewStreamId newStreamSlot;
#endif

	// DirectSound buffer.
  LPDIRECTSOUNDBUFFER			pDSBuffer;

	bool									isUsed;						// when this sound slot is being used
	bool									isQueued;					// when a sound is in queue, it is in use so don't try to use it!
	bool									isPlaying;				// true if the sound is currently playing
	bool									isReady;					// true if the sound is ready to be played - mainly for streams
	bool									isPauseGuarded;		// true if the sound shouldn't be affected by pause
	bool									isDampenGuarded;	// true if the sound shouldn't be dampened
	bool									isDead;						// true if the sound has been stopped.  because of hardware limitation, the buffer may not be ready immediately
	
	nlint32								isPaused;
	nlint32								dampenCount;
	nslSoundParam					params;

	nslSound*							next;							// points to the next sound in the emitter's soundlist
} _nslSound;

// ======================================================================================
//
//
// nslEmitter structure and functions
//
//
//
// NOTE: Zero is an invalid nslEmitterId
//
typedef struct nslEmitter
{
	nslEmitterId			id;

	bool							isDead;
	bool							isUsed;
	bool							isAutoRelease;
	
	nlVector3d				pos;
	nslSound*					soundList;						// the list of sounds belonged to this emitter
	nslSound*					lastSound;						// points to the last element in the sound list
} _nslEmitter;


// ======================================================================================
//
//
// System Structure
//
//

typedef struct nslSystem
{
	// hardware
  LPDIRECTSOUND						pDS;

	// callback functions
	char										rootDir[NSL_MAX_STR_LENGTH];
  char                    bankFileName[NSL_MAX_STR_LENGTH];
	nslSystemCallbackStruct	callbacks;
	
	// resources
#if !DISABLE_STREAMS
	HANDLE								hWorkderThread;  // worker thread for streams
	nslStreamSound				streams[NSL_NUM_STREAM_SOUNDS];
#endif

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
  HANDLE                hStreamFile;
  nslNewStreamSound     soundStreams[MaxXBoxStreams];
  CXBoxStreams          streamer;       // this class takes care of streaming
#endif

	nlMatrix4x4						listenerPo;
	nlVector3d						listenerPos;
	nslEmitter						emitters[NSL_NUM_EMITTERS];
	nslSound							sounds[NSL_NUM_SOUNDS];
	nslSource							sources[NSL_NUM_SOUND_SOURCES];

	nslFileBuf						soundBank;
	
	// states/flags
	bool									initialized;
	bool									loadedSndFile;
	bool                  killThread;
	nslSpeakerModeEnum		speakerMode;
	nslOutputModeEnum			outputMode;
  nslLanguageEnum       language;
	
	float									masterVolume;
	float									gameVolume[NSL_SOURCETYPE_Z];
	float									dampenValue;

	nlUlong								id;
	
	nlUint								numStreamSounds;
	nlint32								numSources;
	nlint32								numSounds;
	nlint32								numEmitters;
	nlint32								num3DSounds;
#if defined (NSLDEBUG)
	nlUlong								soundDataMemoryUsage;
#endif
} _nslSystem;

extern nslSystem sndSystem;



// ======================================================================================
//
//
// NSL Internal Functions 
//
//
void	_str2upper( char *str );
inline float _toDecibel(float f);

inline bool _nslCheckSoundId( nslSoundId id );
int		_nslFindNewSoundSlot();
void	_nslClearSoundSlot( nslSoundId id );
int		_nslFindNewStreamSlot();

#if !DISABLE_STREAMS
void	_nslClearStreamSlot( nslStreamId id );
bool	_nslFindFreePacket( nslStreamId id, DWORD* pdwPacketIndex );
HRESULT _nslProcessSource( nslStreamId id, DWORD packetIndex );
HRESULT _nslProcessRenderer( nslStreamId id, DWORD packetIndex );
bool	_nslProcessStream( nslStreamId id );
DWORD WINAPI _nslStreamThreadProc( LPVOID lpParameter );
#endif

#ifdef NSL_LOAD_SOURCE_BY_NAME
void	_nslProcessSndEntry( const char *soundSourceName );
#endif
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
void	_nslProcessSndEntry( nlUint16 aliasID );
#endif

void	_nslReleaseAllSources();

void	_nslAddSoundToEmitter(nslEmitterId id, nslSoundId sId);
void	_nslRemoveSoundFromEmitter(nslEmitterId id, nslSoundId sId);
void	_nslClearEmitter(nslEmitterId id);
int		_nslFindNewEmitterSlot();
void	_nslReleaseAllEmitters();


HANDLE nslOpenStream( const char* FileName );
bool _set3dVolume( nslSound *snd, _nslStreamSound *strm );
void _setNon3dVolume( nslSound *snd, _nslStreamSound *strm );
float _setSoundAngle( nslSound *snd );


#endif
