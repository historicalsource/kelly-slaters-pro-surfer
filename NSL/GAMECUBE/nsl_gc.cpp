#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#ifndef NSL_NO_ASSERT_H
#include <assert.h>
#endif

#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/ar.h>
#include <dolphin/arq.h>
#include <dolphin/ai.h>
#include <dolphin/ax.h>
#include <dolphin/axfx.h>

#include "common/nsl.h"
#include "gamecube/nsl_gc.h"
#include "gamecube/nsl_gc_stream.h"

#include <dolphin/mix.h>

/*-------------------------------------------------------------------------
  NSL system object, non-static to provide access from other nsl_gc_* modules
-------------------------------------------------------------------------*/

nslSystem nsl;

static struct nslSystemCallbackStruct nslSystemCallbacks;

// This needs to be global to work with the reverb call back system.
static AXFX_REVERBHI reverbHi;

/*-------------------------------------------------------------------------
  GameCube-specific code/data
-------------------------------------------------------------------------*/

typedef struct __nslTransferRecord 
{
	DVDFileInfo info;
	u32 readPos;
	ARQRequest request;
	u32 address;
	void (*callback)( struct __nslTransferRecord* );
} _nslTransferRecord;

static void _nslInitOnce( void );
static void _nslClearSystemData( void );
static int _nslParseSoundList( const char* level );
static int _nslLoadGSW( const char* level );
static bool _nslInitializedOnce = false;

// why do we have this when we already have nsl.initialized?
// because we can't be guaranteed that nsl.initialized is
// going to be initialized to 'false' because it isn't static
// and we need a gatekeeper for repeated calls to nslInit
static bool	_nslInitialized = false;
static unsigned char _nslAramNullBuffer[256] __attribute__((aligned(32)));
static unsigned char* _nslReadBuffer = NULL;
static _nslTransferRecord _nslGSWRecord;
static volatile bool _nslGSWLoaded;
static nslGCDVDIdleFunc _nslDVDIdleFunc;

#define NSL_READ_BUF_SIZE (64 * 1024)

/*-------------------------------------------------------------------------
  Public NSL interface
-------------------------------------------------------------------------*/
void nslSetRootDir( const char* root )
{
	_nslSafeStrncpy( nsl.root, root, sizeof( nsl.root ) );
}

void nslGCSetDVDIdleFunc( nslGCDVDIdleFunc f )
{
	_nslDVDIdleFunc = f;
}

static void *_nslAllocWrapper(unsigned long size)
{
	const int arbitary_alignment = 32;
	return nslSystemCallbacks.MemAlloc(size, arbitary_alignment);
}

bool nslInit( void )
{
	int i;
	
	if( _nslInitialized )
		return true;

	_nslInitOnce( );

	// initialize audio interface (DAC, etc.), let it use default stack
	AIInit( NULL );
	
	// initialize voice management, etc.
	AXInit( );

	// initialize MIX lib
	MIXInit( );
	AXRegisterCallback( MIXUpdateSettings );

	// there is no mono mode, so just keeping state realistic
	AXSetMode( AX_MODE_STEREO );

	AXSetCompressor( AX_COMPRESSOR_ON );

	// get the base address of ARAM (16kb allocated to OS at start, usually)
	nsl.aramBaseAddress = ARGetBaseAddress( );
	
	// get the total size of ARAM (NOA plans to release a memory upgrade)
	nsl.aramSize = ARGetSize( );
	
	// we'll put our null buffer right at the beginning
	nsl.aramNullAddress = nsl.aramBaseAddress;
	
	// where we may start loading data
	nsl.aramUsableAddress = nsl.aramNullAddress + sizeof( _nslAramNullBuffer );
	
	for( i=0; i<NSL_NUM_STREAMS; i++ )
	{
		// FIXME: This is a nasty usage of memory :)
		nsl.streams[i].audioBufferStart = nsl.aramUsableAddress;
		nsl.streams[i].audioBufferSize	= 32768 * 2;
		nsl.aramUsableAddress += nsl.streams[i].audioBufferSize;
	}
	
	// first sneak in the null buffer
	ARQRequest request;

	// no need for a callback, static memory
	ARQPostRequest( &request,	0,
								ARQ_TYPE_MRAM_TO_ARAM,
								ARQ_PRIORITY_HIGH,
								(u32) _nslAramNullBuffer,
								nsl.aramNullAddress,
								sizeof( _nslAramNullBuffer ),
								NULL );

	nsl.playQueue.set_alloc_funcs( nslMemAlloc, nslMemFree );

	_nslClearSystemData( );

	// Set one of the cores to apply reverb to sounds played through it.
	// These parameters were originally taken from example code.

	if (nslSystemCallbacks.MemAlloc && nslSystemCallbacks.MemFree)
	{
		AXFXSetHooks(&_nslAllocWrapper, nslSystemCallbacks.MemFree);
	}

	reverbHi.tempDisableFX      = FALSE;
	reverbHi.time               = 3.0f;
	reverbHi.preDelay           = 0.1f;
	reverbHi.damping            = 0.5f;
	reverbHi.coloration         = 0.5f;
	reverbHi.crosstalk          = 0.3f;
	reverbHi.mix                = 0.5f;

	AXFXReverbHiInit(&reverbHi);
	AXRegisterAuxACallback((void(*)(void*,void*))AXFXReverbHiCallback, &reverbHi);

	nsl.playQueue.init( 64 );
	nsl.initialized = true;
	nsl.on = false;
	
	_nslInitialized = true;

	return true;
}

void nslToggleOnOff( void )
{
	nsl.on = !nsl.on;
}

bool nslReset( const char* level, nslLanguageEnum lang )
{
	nslShutdown();
	nslInit();

	nsl.lang = lang;

	if( _nslParseSoundList( level ) ) 
	{
		nsl.initialized = 0;
		return false;
	}

	if( _nslLoadGSW( level ) ) 
	{
		nsl.initialized = 0;
		return false;
	}

	while( !_nslGSWLoaded )
	{

		if( _nslDVDIdleFunc ) {
			_nslDVDIdleFunc( );
		}

	}

	nsl.on = true;
	
	return true;
}

void nslShutdown( void )
{
	if( !_nslInitialized ) {
		return;
	}

	nslReleaseAllSounds( );

	nsl.playQueue.release( );

	nslMemFree( nsl.infos );
	nsl.infos = NULL;
	nsl.infoCount = 0;

	_nslClearSystemData( );

	nsl.initialized = false;
	nsl.on = false;

	AXFXReverbHiShutdown( &reverbHi );

	MIXQuit( );
	
	AXQuit( );
	AIReset( );

	_nslInitialized = false;
}

static void _nslAdvancePlayQueue( void )
{
	// examine queued sounds list
	int queued = (int) nsl.playQueue.size( );

#ifdef NSL_QUEUE_LIMIT
	if( queued > NSL_QUEUE_LIMIT )
	{
		queued = NSL_QUEUE_LIMIT;
	}
#endif

	for( int i = 0; i < queued; i++ ) 
	{
		nslSoundId sid = nsl.playQueue.pop( );
		nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( sid ) ];

#if (defined NSL_ENSURE_WEAK)
		if( !sound->used || !sound->isPlaying ) 
		{
			nslPrintf( "_nslAdvancePlayQueue: bogus sound (%d) on queue! (used)\n", sid );
			continue;
		}
#else
		NSL_ASSERT( sound->used );
		NSL_ASSERT( sound->isPlaying );
#endif

		nslSource* source = sound->source;

		if( !source ) 
		{
			nslPrintf( "_nslAdvancePlayQueue: bogus sound (%d) on queue! (source)\n", sid );
			continue;
		}

		if( !sound->vpb )
		{
			nslPrintf( "_nslAdvancePlayQueue: bogus sound (%d) on queue! (vpb)\n", sid );
			continue;
		}

		if( sound->frameDelayed == false )
		{
			sound->frameDelayed = true;
			nsl.playQueue.push( sid );
			continue;
		}

		// FIXME: more of a note, loaded is never 0, so
		// this code is somewhat stupid, but assuming
		// we ever do stream, this architecture may
		// be useful... or maybe it's just junk, who knows.
		if( source->loaded && sound->pauseCount <= 0 ) 
		{
			nslVerbosePlayPrintf( "_nslAdvancePlayQueue: starting '%s' (%d).\n", source->info->name, sound->id );
			
			AXSetVoiceState( sound->vpb, AX_PB_STATE_RUN );
			sound->isReallyPlaying = true;
		} 
		else 
		{
			if( !source->loaded )
				nslVerbosePrintf( "_nslAdvancePlayQueue: deferring '%s' because source isn't loaded.\n", source->info->name );

			// back on you go
			nsl.playQueue.push( sid );
		}
	}
}

static void _nslAdvanceState( void )
{
	int i;
	for( i = 0; i < NSL_NUM_SOUNDS; i++ ) 
	{
		nslSound* sound = &nsl.sounds[i];
		if( !sound->used )
			continue;

		nslSource* source = sound->source;
		NSL_ASSERT( source );
		NSL_ASSERT( source->info );

		if( source && source->info )
		{
			if( source->info->flags & NSL_SOURCE_INFO_STREAM )
			{
				int sid;
				
				if(		 source->streamId != -1 && sound->commonStreamId == -1 ) sid = source->streamId;
				else if( source->streamId == -1 && sound->commonStreamId != -1 ) sid = sound->commonStreamId;
				else
				{
					NSL_ASSERT( source->streamId == -1 || sound->commonStreamId == -1 );
					sid = -1;
				}
				
				if( sid != - 1 )
				{
					int state;
					state = nslStreamStatus( sid );
					switch( state )
					{
						case NSL_STREAM_STATE_CLOSED:
							NSL_ASSERT(0);
							// fallback to STATE_IDLE
							break;
							
						case NSL_STREAM_STATE_IDLE:
							nslStopSound( sound->id );
							break;
					}
				}
			}
			else if( sound->isReallyPlaying && sound->pauseCount <= 0 ) 
			{
				AXVPB* vpb = sound->vpb;

				// let's see if this works
				if( vpb->pb.state == AX_PB_STATE_STOP ) 
				{
					nslVerbosePrintf( "_nslAdvanceState: stopping (done) '%s' (%d).\n", source->info->name, sound->id ); 
					nslStopSound( sound->id );
				} 
			}
		}
	}
}

static void _nslAdvanceEmitters( void )
{
	int i;
	for( i = 0; i < NSL_NUM_EMITTERS; i++ ) 
	{
		nslEmitter* emitter = &nsl.emitters[i];
		if( !emitter->used )
			continue;

		if( (emitter->autoRelease) && (emitter->numSounds == 0) ) 
		{
			nslVerbosePrintf( "_nslAdvanceEmitters: auto-releasing emitter %d.\n", emitter->id );
			nslReleaseEmitter( emitter->id );
		}
	}
}

void _nslReallyUpdateVolume( nslSound* sound )
{
	if( sound->used && sound->needVolumeUpdate )
	{
		nslSource* source = sound->source;

		if( source->info->flags & NSL_SOURCE_INFO_STREAM )
		{
			int id;
			f32 l, r;
			
			_nslCalcTwoChannel( sound, source, l, r );
			id = _nslGetStreamId( sound );
			if( id != -1 )
				nslStreamSetVolume( id, l );
		} 
		else 
		{
			if( !sound->vpb )
				return;

			if( sound->emitter == NULL )
				_nslUpdateVolumeGlobal( sound, source );
			else
				_nslUpdateVolume3D( sound, source );  
		}
		sound->needVolumeUpdate = false;
	}
}

static void _nslAdvanceMixing( void )
{
	int i;
	for( i = 0; i < NSL_NUM_SOUNDS; i++ )
	{
		nslSound* sound = &nsl.sounds[i];
		_nslReallyUpdateVolume( sound );
	}
}

void nslFrameAdvance( float delta )
{
	NSL_ENSURE_ON( );

	_nslAdvanceMixing( );
#ifndef NSL_NO_PLAY_QUEUE
	_nslAdvancePlayQueue( );
#endif
	_nslAdvanceState( );
	_nslAdvanceEmitters( );
	
	for( int i = 0; i < NSL_NUM_STREAMS; i++ ) {
		nslStreamUpdate( i );
	}

}

void nslSetMasterVolume( float volume ) 
{
	NSL_ENSURE_ON( );

	nsl.masterVolume = volume;
	
	_nslUpdateVolumeAll( );
}

float nslGetMasterVolume( void )
{
	if( _nslInitializedOnce == false )
		return 0.0f;
	return nsl.masterVolume;
}

void nslSetVolume( nslSourceTypeEnum type, float volume )
{
	NSL_ENSURE_ON( );

	switch( type ) 
	{
		case NSL_SOURCETYPE_SFX:
			nsl.sfxVolume = volume;
			break;
			
		case NSL_SOURCETYPE_MUSIC:
			nsl.musicVolume = volume;
			break;
			
		case NSL_SOURCETYPE_AMBIENT:
			nsl.ambientVolume = volume;
			break;
			
		case NSL_SOURCETYPE_VOICE:
			nsl.voiceVolume = volume;
			break;
			
		case NSL_SOURCETYPE_MOVIE:
			nsl.movieVolume = volume;
			break;
			
		default:
			nslFatal( "nslSetVolume: invalid type.\n" );
			break;
	}

	_nslUpdateVolumeOfType( type );
}

float nslGetVolume( nslSourceTypeEnum type )
{
	// NSL doesn't need to be initialized for this
	// query to be valid, just initialized once.
	if( _nslInitializedOnce == false )
		return 0.0f;

	float value = 0.0f;

	switch( type ) 
	{
		case NSL_SOURCETYPE_SFX:
			value = nsl.sfxVolume;
			break;
			
		case NSL_SOURCETYPE_AMBIENT:
			value = nsl.ambientVolume;
			break;
			
		case NSL_SOURCETYPE_MUSIC:
			value = nsl.musicVolume;
			break;
			
		case NSL_SOURCETYPE_VOICE:
			value = nsl.voiceVolume;
			break;
			
		case NSL_SOURCETYPE_MOVIE:
			value = nsl.movieVolume;
			break;
			
		default:
			nslFatal( "nslGetVolume: invalid type.\n" );
			break;
	}

	return value;
}

void nslSetListenerPo( const nlMatrix4x4& po )
{
	NSL_ENSURE_ON( );

	nsl.listenerPo[0][0] = po[0][0];
	nsl.listenerPo[0][1] = po[0][1];
	nsl.listenerPo[0][2] = po[0][2];
	nsl.listenerPo[0][3] = po[0][3];

	nsl.listenerPo[1][0] = po[1][0];
	nsl.listenerPo[1][1] = po[1][1];
	nsl.listenerPo[1][2] = po[1][2];
	nsl.listenerPo[1][3] = po[1][3];

	nsl.listenerPo[2][0] = po[2][0];
	nsl.listenerPo[2][1] = po[2][1];
	nsl.listenerPo[2][2] = po[2][2];
	nsl.listenerPo[2][3] = po[2][3];

	nsl.listenerPo[3][0] = po[3][0];
	nsl.listenerPo[3][1] = po[3][1];
	nsl.listenerPo[3][2] = po[3][2];
	nsl.listenerPo[3][3] = po[3][3];
	
	_nslUpdateVolumeAll( );
}

void nslGetListenerPo( nlMatrix4x4* dest )
{
	NSL_ENSURE_ON( );

	nlMatrix4x4& po = (*dest);
	
	po[0][0] = nsl.listenerPo[0][0];
	po[0][1] = nsl.listenerPo[0][1];
	po[0][2] = nsl.listenerPo[0][2];
	po[0][3] = nsl.listenerPo[0][3];

	po[1][0] = nsl.listenerPo[1][0];
	po[1][1] = nsl.listenerPo[1][1];
	po[1][2] = nsl.listenerPo[1][2];
	po[1][3] = nsl.listenerPo[1][3];

	po[2][0] = nsl.listenerPo[2][0];
	po[2][1] = nsl.listenerPo[2][1];
	po[2][2] = nsl.listenerPo[2][2];
	po[2][3] = nsl.listenerPo[2][3];

	po[2][0] = nsl.listenerPo[3][0];
	po[2][1] = nsl.listenerPo[3][1];
	po[2][2] = nsl.listenerPo[3][2];
	po[2][3] = nsl.listenerPo[3][3];
}

void nslSetListenerPosition( const nlVector3d& position )
{
	NSL_ENSURE_ON( );

	nsl.listenerPo[0][3] = position[0];
	nsl.listenerPo[1][3] = position[1];
	nsl.listenerPo[2][3] = position[2];
	
	_nslUpdateVolumeAll( );
}

void nslSetListenerOrientation( const nlVector3d& _fwd, const nlVector3d& _up )
{
	NSL_ENSURE_ON( );

	nlVector3d left, up, fwd;

	fwd[0] = _fwd[0];
	fwd[1] = _fwd[1];
	fwd[2] = _fwd[2];

	up[0] = _up[0];
	up[1] = _up[1];
	up[2] = _up[2];

	nlCrossProduct3d( left, up, fwd );

	float length = sqrtf( left[0] * left[0] + left[1] * left[1] + left[2] * left[2] );
	NSL_ASSERT( length != 0.0f );
	float recipLength = 1.0f / length;

	left[0] *= recipLength;
	left[1] *= recipLength;
	left[2] *= recipLength;

	nsl.listenerPo[0][0] = left[0];
	nsl.listenerPo[0][1] = left[1];
	nsl.listenerPo[0][2] = left[2]; 

	nsl.listenerPo[1][0] = up[0];
	nsl.listenerPo[1][1] = up[1];
	nsl.listenerPo[1][2] = up[2]; 

	nsl.listenerPo[2][0] = fwd[0];
	nsl.listenerPo[2][1] = fwd[1];
	nsl.listenerPo[2][2] = fwd[2]; 
	
	_nslUpdateVolumeAll( );
}

void nslSetSpeakerMode( nslSpeakerModeEnum mode )
{
	NSL_ENSURE_ON( );

	nsl.speakerMode = mode;

	if( mode == NSL_SPEAKER_MONO ) 
	{
		OSSetSoundMode( OS_SOUND_MODE_MONO );
	}
	else 
	{
		OSSetSoundMode( OS_SOUND_MODE_STEREO );
	}

	u32 ax_mode = AX_MODE_STEREO;

	if( mode == NSL_SPEAKER_PROLOGIC )
	{
		ax_mode = AX_MODE_DPL2;
	}
	else if( mode == NSL_SPEAKER_DOLBY_51 )
	{
		ax_mode = AX_MODE_SURROUND;
	}

	AXSetMode( ax_mode );
}

nslSpeakerModeEnum nslGetSpeakerMode( void )
{
	if( _nslInitializedOnce == false )
		return ( OSGetSoundMode( ) == OS_SOUND_MODE_MONO ) ? NSL_SPEAKER_MONO : NSL_SPEAKER_STEREO;
	return nsl.speakerMode;
}

void nslSetOutputMode( nslOutputModeEnum mode )
{
	NSL_ENSURE_ON( );

	// ignore, no idea what this is
	nsl.outputMode = mode;
}

void nslSetSystemCallbacks( nslSystemCallbackStruct* callbacks )
{
	memcpy( &nslSystemCallbacks, callbacks, sizeof( nslSystemCallbackStruct ) );
}

void nslPrintf( const char* Format, ... )
{
	char Work[1024];
	va_list args;
	va_start( args, Format );
	vsprintf( Work, Format, args );
	va_end( args );

	if( nslSystemCallbacks.DebugPrint ) 
		nslSystemCallbacks.DebugPrint( Work );
	else
		OSReport( Work );
}

void nslFatal( const char* Format, ... )
{
	char Work[1024];
	va_list args;
	va_start( args, Format );
	vsprintf( Work, Format, args );
	va_end( args );

	if( nslSystemCallbacks.CriticalError )
		nslSystemCallbacks.CriticalError( Work );
	else
		OSHalt( Work );
}

bool nslReadFile( const char* FileName, nslFileBuf* File, nlUint32 Align )
{
	char rooted[256];

	if( nsl.root[0] != '\0' )
		snprintf( rooted, sizeof( rooted ), "%s/%s", nsl.root, FileName );
	else
		_nslSafeStrncpy( rooted, FileName, sizeof( rooted ) );

	if( nslSystemCallbacks.ReadFile ) 
	{
		return nslSystemCallbacks.ReadFile( FileName, File, Align );
	} 
	else 
	{
		char* caret = rooted;

		while( *caret ) 
		{
			if( *caret == '\\' )
				*caret = '/';
			++caret;
		}

		s32 entry = DVDConvertPathToEntrynum( rooted );

		if( entry < 0 )
			return false;

		DVDFileInfo info;
		BOOL b = DVDFastOpen( entry, &info );

		if( !b )
			return false; // technically impossible

		File->Size = DVDGetLength( &info );
		File->Buf = (nlUchar*) nslMemAlloc( OSRoundUp32B( File->Size ) );

		DVDReadAsync( &info, File->Buf, (long)OSRoundUp32B( File->Size ), 0, NULL );
		s32 status = 0;

		do {
			status = DVDGetFileInfoStatus( &info );

			switch( status ) {
			case DVD_STATE_COVER_OPEN:
			case DVD_STATE_NO_DISK:
			case DVD_STATE_WRONG_DISK:
			case DVD_STATE_RETRY:
			case DVD_STATE_FATAL_ERROR:

				if( _nslDVDIdleFunc ) {
					_nslDVDIdleFunc( );
				}

				break;
			}

		} while( status != DVD_STATE_END );
		
		DVDClose( &info );
		
	  return true;
	}

}

void nslReleaseFile( nslFileBuf* File )
{
	if( nslSystemCallbacks.ReleaseFile )
	{
		nslSystemCallbacks.ReleaseFile( File );
	}	
	else
	{
		nslMemFree( File->Buf );
		memset( File, 0, sizeof( nslFileBuf ) );
	}
}

void* nslMemAlloc( nlUint32 Size, nlUint32 Align )
{
	if( nslSystemCallbacks.MemAlloc )
		return nslSystemCallbacks.MemAlloc( Size, Align );
	else
		return OSAlloc( OSRoundUp32B( Size ) );
}

void nslMemFree( void* Ptr )
{
	if( nslSystemCallbacks.MemFree )
		nslSystemCallbacks.MemFree( Ptr );
	else if( Ptr )
		OSFree( Ptr );
}

static void _nslInitOnce( void )
{
	if( _nslInitializedOnce )
		return;

	// this state should persist between resets, in my opinion
	nsl.speakerMode = ( OSGetSoundMode( ) == OS_SOUND_MODE_MONO ) ? NSL_SPEAKER_MONO : NSL_SPEAKER_STEREO;
	nsl.outputMode = NSL_OUTPUT_BOTH;
	nsl.sfxVolume = 1.0f;
	nsl.ambientVolume = 1.0f;
	nsl.musicVolume = 1.0f;
	nsl.voiceVolume = 1.0f;
	nsl.movieVolume = 1.0f;
	nsl.masterVolume = 1.0f;
	nsl.dampenLevel = 0.0f;
	nsl.root[0] = '\0';

	_nslInitializedOnce = true;
}

static void _nslResetAllIds( void )
{
	int i = 0;

	for( i = 0; i < NSL_NUM_SOURCES;  i++ ) {
		nsl.sources[i].id  = i;
	}

	for( i = 0; i < NSL_NUM_SOUNDS;   i++ ) {
		nsl.sounds[i].id   = i;
	}

	for( i = 0; i < NSL_NUM_EMITTERS; i++ ) {
		nsl.emitters[i].id = i;
	}

}

static void _nslClearSystemData( void )
{
	nlIdentityMatrix( nsl.listenerPo );

	_nslReleaseAllSources( );
	_nslReleaseAllSounds( );
	_nslReleaseAllEmitters( );
	_nslResetAllIds( );

	nsl.playQueue.clear( );
	nsl.stereoSound = NULL;
	nsl.lang = NSL_LANGUAGE_ENGLISH;

	_nslGSWLoaded = false;
}

nslSourceInfo* _nslGetSourceInfo( nslMoniker moniker )
{
	NSL_CHECK_MONIKER( moniker );
	NSL_ASSERT( nsl.infos );
	
	int i;
	
	for( i=0; i < nsl.infoCount; i++ )
	{
		nslSourceInfo* info = &nsl.infos[i];
		#ifdef NSL_LOAD_SOURCE_BY_NAME
			if( _nslStrCaseCmp( moniker, info->name ) == 0 )
		#endif
		#ifdef NSL_LOAD_SOURCE_BY_ALIAS
			if( moniker == info->alias )
		#endif
			{
				return info;
			}
	}
	return NULL;
}

typedef struct _nslSNDHeader {
	char tag[4];
	nlUint32 version;
	char gswName[32];
	nlUint32 numEntries;
	nlUint8 pad[20];
} nslSNDHeader;

#define GC_SND_VERSION 0x00030003

static int _nslParseSoundList( const char* level )
{
	char filename[256];
	nslFileBuf file;

	snprintf( filename, sizeof( filename ), "%s/%s.snd", nslLanguageStr[nsl.lang], level );

	if( nslReadFile( filename, &file, 1 ) == false )
	{
		nslPrintf( "Couldn't open sound list '%s'.\n", filename );
		return -1;
	}

	unsigned char* buf = file.Buf;
	nslSNDHeader* header = (nslSNDHeader*) buf;
	
	if( strcmp( header->tag, "SOND" ) != 0 ) 
	{
		nslFatal( "Corrupt SND file '%s'.\n", filename );
		return -1;
	}

	if( header->version != GC_SND_VERSION ) 
	{
		nslFatal( "Unexpected version '0x%x' in SND file '%s'.\n", header->version, filename );
		return -1;
	}

	nsl.infoCount = header->numEntries;
	nsl.infos = (nslSourceInfo*) nslMemAlloc( nsl.infoCount * sizeof( nslSourceInfo ), 32 );
	buf += sizeof( nslSNDHeader );
	memcpy( nsl.infos, buf, nsl.infoCount * sizeof( nslSourceInfo ) );
	
	nslReleaseFile( &file );

	return 0;
}

static void _nslChainDVDCB( s32 length, DVDFileInfo* fileInfo );

static void _nslChainARQCB( u32 raw )
{
	ARQRequest* request = (ARQRequest*) raw;
	_nslTransferRecord* record = (_nslTransferRecord*) request->owner;
	s32 remaining = (s32)(DVDGetLength( &record->info ) - record->readPos);

	if( remaining <= 0 ) 
	{
		if( record->callback )
			record->callback( record );
		return;
	}

	remaining = (s32)OSRoundUp32B( remaining );
	s32 length = ( remaining > NSL_READ_BUF_SIZE ) ? NSL_READ_BUF_SIZE : remaining;
	DVDReadAsync( &record->info, _nslReadBuffer, length, (s32)record->readPos, _nslChainDVDCB );
}

static void _nslChainDVDCB( s32 length, DVDFileInfo* fileInfo )
{
	_nslTransferRecord* record = (_nslTransferRecord*) DVDGetUserData( (DVDCommandBlock*) fileInfo );
	ARQPostRequest( &record->request, 
					(u32) record,
					ARQ_TYPE_MRAM_TO_ARAM,
					ARQ_PRIORITY_HIGH,
					(u32) _nslReadBuffer,
					record->address + record->readPos,
					(u32) length,
					_nslChainARQCB );
	record->readPos += length;
}

static void _nslGSWLoadedCB( _nslTransferRecord* record )
{
	_nslGSWLoaded = true;
	nslMemFree( _nslReadBuffer );
	_nslReadBuffer = 0;
	DVDClose( &record->info );
}

static int _nslLoadGSW( const char* level )
{
	char filename[256];
	char* caret = filename;

	if( nsl.root[0] != '\0' )
		snprintf( filename, sizeof( filename ), "%s/%s/%s.gsw", nsl.root, nslLanguageStr[nsl.lang], level );
	else
		snprintf( filename, sizeof( filename ), "%s/%s.gsw", nslLanguageStr[nsl.lang], level );

	while( *caret ) 
	{
		if( *caret == '\\' )
			*caret = '/';
		++caret;
	}

	// we need to go around the usual read junk
	s32 entry = DVDConvertPathToEntrynum( filename );
	if( entry < 0 ) 
	{
		nslFatal( "couldn't find level audio data file '%s'\n", filename );
		return -1;
	}

	_nslTransferRecord* record = &_nslGSWRecord;
	record->readPos = 0;
	record->address = nsl.aramUsableAddress;
	record->callback = _nslGSWLoadedCB;
	DVDFileInfo* fileInfo = &record->info;

	if( DVDFastOpen( entry, fileInfo ) == FALSE ) 
	{
		nslFatal( "unable to open level audio data file '%s'\n", filename );
		return -1;
	}

	_nslReadBuffer = (unsigned char*) nslMemAlloc( NSL_READ_BUF_SIZE, 32 );
	DVDSetUserData( (DVDCommandBlock*) fileInfo, record );
	u32 length = DVDGetLength( fileInfo );
	length = OSRoundUp32B( length );
	length = ( length > NSL_READ_BUF_SIZE ) ? NSL_READ_BUF_SIZE : length;
	DVDReadAsync( fileInfo, _nslReadBuffer, (s32)length, 0, _nslChainDVDCB );

	return 0;
}
