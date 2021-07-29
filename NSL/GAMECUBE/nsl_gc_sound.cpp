#include "common/nsl.h"
#include "gamecube/nsl_gc.h"
#include "gamecube/nsl_gc_stream.h"

#include <dolphin/mix.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define ADPCM_ADDR(x) ((((x) / 14) * 16) + ((x) % 14) + 2)

static nlInt32 _nslFindUnusedSound( void );
static void _nslVoiceCallback( void* p );
static float _nslCalcDistAttenuation( nslSound* sound );
static void	_nslUpdatePitch( nslSound* sound, nslSource* source );
static void _nslPauseSound( nslSound* sound );
static void _nslUnpauseSound( nslSound* sound );
static const char* _nslGetSoundParamString( nslSoundParamEnum param );

int _nslGetStreamId( nslSound *sound, int *_isCommon )
{
	int sid, isCommon;
	
	sid		 = -1;
	isCommon = -1;

	if( sound && sound->source )
	{	
		if( sound->commonStreamId != -1 && sound->source->streamId == -1 )
		{
			isCommon = 1;
			sid = sound->commonStreamId;
		}
		else if( sound->commonStreamId == -1 && sound->source->streamId != -1 )
		{
			isCommon = 0;
			sid = sound->source->streamId;
		}
		else
		{
			NSL_ASSERT(0);
		}
	}
	
	if( _isCommon )
	   *_isCommon = isCommon;
	   
	return sid;
}

static nslSoundId _nslAddSound( nslSourceId id )
{
	NSL_ENSURE_ON_VAL( NSL_INVALID_ID );
	NSL_ENSURE_RID_VAL( id, NSL_INVALID_ID );

	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	
	nlInt32 slot = _nslFindUnusedSound( );
	
	if( slot < 0 ) 
	{
		nslVerbosePrintf( "nslAddSound: unsable to find unused sound slot for source '%s' (%d).\n", source->info->name, id );
		return NSL_INVALID_ID;
	}

	nslSound* sound = &nsl.sounds[ slot ];

	sound->id += NSL_ID_INCREMENT;
	
	// FIXME: cull based on size... or something
	sound->source = source;

	sound->isPlaying = false;
	sound->isReallyPlaying = false;
	sound->needVolumeUpdate = true;
	sound->frameDelayed = false;

	sound->rawVolume = source->volume;
	sound->angle = 0.0f;
	sound->pitch = source->pitch;
	sound->minDist = source->minDist;
	sound->maxDist = source->maxDist;

	sound->pauseCount = 0;
	sound->dampenCount = 0;

	sound->used = true;   
	sound->commonStreamId = -1;
	
	nsl.soundCount++;

	nslVerbosePrintf( "nslAddSound: adding sound '%s' (%d) (%d).\n", sound->source->info->name, sound->id, nsl.soundCount );

	return sound->id;
}

nslSoundId nslAddSound( nslSourceId srcId )
{
	nslSoundId sndId;
	sndId = _nslAddSound( srcId );
	if( sndId != NSL_INVALID_ID )
	{
		nslSource* source;
		nslSound* sound;
		
		sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID(sndId) ];
		source = sound->source;
		if( source->info->flags & NSL_SOURCE_INFO_STREAM )
		{
			// Must make sure that common stream was not already opened as not common (e.g. nslStreamLoadSource)
			NSL_ASSERT( source->streamId == -1 );
			if( source->streamId == -1 )
			{
				if( sound->commonStreamId == -1 )
				{
					nslStreamInfo info;
					info.sampleRate			= (int)source->info->frequency;
					info.sampleFormat		= NSL_STREAM_FORMAT_ADPCM;
					info.channelCount		= 1;
					info.chunkSize			= 32768;
					sound->commonStreamId	= nslStreamOpen(	"COMMON.STR",
																&info,
																source->info->flags & NSL_SOURCE_INFO_REVERB );
				}
//				NSL_ASSERT( sound->commonStreamId != -1 );
				if( sound->commonStreamId != -1 )
				{
					nslStreamLoad( sound->commonStreamId, (s32)source->info->offset, (s32)source->info->samples );
					return sndId;
				}
				else
				{
					nslPrintf( "can't open COMMON.STR\n" );
				}
			}
			_nslReleaseSound(NSL_GET_SLOT_FROM_ID( sndId ));
			return NSL_INVALID_ID;
		}
	}
	return sndId;
}

nslSoundId nslStreamAddSound( nslSourceId srcId, int offset, int samples )
{
	nslSoundId sndId;
	sndId = _nslAddSound( srcId );
	if( sndId != NSL_INVALID_ID )
	{
		nslSource* source;
		nslSound* sound;
		
		sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID(sndId) ];
		source = sound->source;
		
		NSL_ASSERT( source->info->flags & NSL_SOURCE_INFO_STREAM );
		if( source->info->flags & NSL_SOURCE_INFO_STREAM )
		{
			NSL_ASSERT( source->streamId != -1 );
			NSL_ASSERT( sound->commonStreamId == -1 );
			if( source->streamId != -1 && sound->commonStreamId == -1 )
			{
				nslStreamLoad( source->streamId, offset, samples );
				return sndId;
			}
			_nslReleaseSound( NSL_GET_SLOT_FROM_ID( sndId ) );
			return NSL_INVALID_ID;
		}
	}
	return sndId;
}

void nslPlaySound( nslSoundId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	
	if( sound->isPlaying ) 
		return;	// in a perfect world, this is an error

	sound->isPlaying = true; 
	sound->pauseCount = 0;

	nslSource* source = sound->source;

	if( source->info->flags & NSL_SOURCE_INFO_STREAM )
	{
		int sid;
		sid = _nslGetStreamId( sound );
		if( sid!=-1 && nslStreamPlay(sid) )
		{
			nslVerbosePlayPrintf( "nslPlaySound: playing streamed file '%s' (%d).\n", source->info->name, id );
			_nslUpdateVolume( sound );
		}
		else
		{
			sound->isPlaying = false;
		}
		return;
	}

	if( source->info->offset == (nlUint32) -1 ) 
	{
		nslVerboseDebugPrintf( "nslPlaySound: source info has invalid offset '%s' (%d).\n", source->info->name, id );
		return;	// invalid
	}

	s32 priority = 0;

	switch( source->info->type ) 
	{
		case NSL_SOURCETYPE_AMBIENT:
			priority = 16;
			break;
		case NSL_SOURCETYPE_VOICE:
			priority = 17;
			break;
		case NSL_SOURCETYPE_MOVIE:
			priority = 18;
			break;
		case NSL_SOURCETYPE_MUSIC:
			priority = 19;
			break;
		case NSL_SOURCETYPE_SFX:
		default:
			priority = 15;
			break;
	}

	sound->vpb = AXAcquireVoice( (u32)priority, _nslVoiceCallback, (u32) sound );
#ifdef DEBUG
	sound->dropped = false;
#endif

	if( !sound->vpb ) 
	{
		nslVerboseDebugPrintf( "nslPlaySound: could not acquire voice for '%s'.\n", source->info->name );
		sound->isPlaying = false;
		return;
	}

	if (source->info->flags & NSL_SOURCE_INFO_REVERB)
	{
		// Have the volume sent to AuxA at maximum of 0 because AuxA is set up for reverb.

		MIXInitChannel( sound->vpb, 0, -960, 0, -960, 64, 127, 0 );
	}
	else
	{
		// Send no sound to AuxA or AuxB. -904 is the lowest volume so I'm told.

		MIXInitChannel( sound->vpb, 0, -960, -960, -960, 64, 127, 0 );
	}

	nslVerbosePlayPrintf( "nslPlaySound: playing sound '%s' (%d) (0x%x) (%d).\n", sound->source->info->name, sound->id, sound->vpb, nsl.soundCount );
#ifdef NSL_NO_PLAY_QUEUE
	_nslReallyUpdateVolume( sound );
#else
	_nslUpdateVolume( sound );
#endif
	_nslUpdatePitch( sound, source );

	// address of our sample data in ARAM
	AXPBADDR addr;
	memset( &addr, 0, sizeof( AXPBADDR ) );
	addr.format = AX_PB_FORMAT_ADPCM;
	u32 base = ( nsl.aramUsableAddress + source->info->offset ) * 2;
	u32 begin = base + ADPCM_ADDR( 0 );
	addr.currentAddressHi = AR_ADDR_HI( begin );
	addr.currentAddressLo = AR_ADDR_LO( begin );
	u32 end = base + ADPCM_ADDR( source->info->samples );
	addr.endAddressHi = AR_ADDR_HI( end );
	addr.endAddressLo = AR_ADDR_LO( end );
	
	if( source->info->flags & NSL_SOURCE_INFO_LOOPING ) 
	{
		addr.loopFlag = AXPBADDR_LOOP_ON;
		addr.loopAddressHi = AR_ADDR_HI( begin );
		addr.loopAddressLo = AR_ADDR_LO( begin );		
	}	
	else 
	{
		addr.loopFlag = AXPBADDR_LOOP_OFF;
		u32 null = nsl.aramNullAddress * 2;
		null += ADPCM_ADDR( 0 );
		addr.loopAddressHi = AR_ADDR_HI( null );
		addr.loopAddressLo = AR_ADDR_LO( null );
	}

	AXSetVoiceAddr( sound->vpb, &addr );
	AXSetVoiceAdpcm( sound->vpb, &source->info->adpcm );
	AXSetVoiceAdpcmLoop( sound->vpb, &source->info->loop );

#ifdef NSL_NO_PLAY_QUEUE
	AXSetVoiceState( sound->vpb, AX_PB_STATE_RUN );
	sound->isReallyPlaying = true;
#else
	nsl.playQueue.push( id );
#endif
}

void nslPlaySound3D( nslSoundId id, const nlVector3d& pos )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslEmitterId eid = nslCreateEmitter( pos );
	assert( eid != NSL_INVALID_ID );

	if( eid != NSL_INVALID_ID ) 
	{
		nslSetEmitterAutoRelease( eid );
		nslSetSoundEmitter( eid, id );
		nslPlaySound( id );
	}
}

void nslStopSound( nslSoundId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];

	if( !sound->used ) 
	{
		assert( !sound->vpb ); // in a perfect world, this is an error
		return;
	}

	if( !sound->isPlaying ) 
	{
		assert( !sound->vpb ); // should also be fatal
		return;
	}

	nslSource* source = sound->source;
	nslVerbosePlayPrintf( "nslStopSound: stopping sound '%s' (%d) (%d).\n", source->info->name, sound->id, nsl.soundCount - 1 );

	if( source->info->flags & NSL_SOURCE_INFO_STREAM )
	{
		int sid, isCommon;
		sid = _nslGetStreamId( sound, &isCommon );
		if( sid != -1 )
		{
			nslStreamStop( sid );
			if( isCommon )
			{
				nslStreamClose( sid );
				sound->commonStreamId = -1;
			}
		}
	}
	else 
	{
		assert( sound->vpb );

		// get around a nasty SM crash bug...
		if( sound->vpb )
			AXSetVoiceState( sound->vpb, AX_PB_STATE_STOP );
	}

	// release all resources, reset
	_nslReleaseSound( NSL_GET_SLOT_FROM_ID( id ) );
}

void nslReleaseAllSounds( void )
{
	NSL_ENSURE_ON( );

	int i;
  
	for( i = 0; i < NSL_NUM_SOUNDS; i++ )
	{
  		nslSound* sound = &nsl.sounds[i];
		if( sound->used )
			nslStopSound( sound->id );
		_nslReleaseSound( (u32)i );
	}

	for( i = 0; i < NSL_NUM_EMITTERS; i++ )
	{
		nslEmitter* emitter = &nsl.emitters[i];
		if( emitter->used )
			memset( emitter->sounds, 0, sizeof( emitter->sounds ) );
	}
}

void nslPauseSound( nslSoundId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	_nslPauseSound( sound );
}

void nslUnpauseSound( nslSoundId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	_nslUnpauseSound( sound );
}

void nslPauseGuardSound( nslSoundId id ) 
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];

	if( sound->pauseCount > 0 )
		nslFatal( "nslPauseGuardSound: guarding a paused sound is illegal.\n" );

	if( sound->pauseCount <= 0 )
		sound->pauseCount = -1;
}

void nslPauseAllSounds( void )
{
	NSL_ENSURE_ON( );
	
	for( int i = 0; i < NSL_NUM_SOUNDS; i++ )
	{
		nslSound* sound = &nsl.sounds[i];
		if( sound->used )
			_nslPauseSound( sound );
	}

}

void nslUnpauseAllSounds( void )
{
	NSL_ENSURE_ON( );

	for( int i = 0; i < NSL_NUM_SOUNDS; i++ ) 
	{
		nslSound* sound = &nsl.sounds[i];
		if( sound->used )
			_nslUnpauseSound( sound );
	}
}

void nslUndampenAllSounds( void )
{
	NSL_ENSURE_ON( );

	for( int i = 0; i < NSL_NUM_SOUNDS; i++ )
	{
		nslSound* sound = &nsl.sounds[i];
		if( sound->used ) 
		{
			sound->dampenCount = 0;
			_nslUpdateVolume( sound );
		}
	}
}

void nslDampenGuardSound( nslSoundId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = & nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	if( sound->dampenCount <= 0 )
		sound->dampenCount = -1;
}

void nslDampenAllSounds( float level )
{
	NSL_ENSURE_ON( );
  
	nsl.dampenLevel = level;
	for( int i = 0; i < NSL_NUM_SOUNDS; i++ )
	{
		nslSound* sound = &nsl.sounds[i];
		if( sound->used ) 
		{
			if( sound->dampenCount < 1 ) 
			{
				sound->dampenCount++;
				_nslUpdateVolume( sound );
			}
		}
	}
}

nslSoundStatusEnum nslGetSoundStatus( nslSoundId id ) 
{
	NSL_ENSURE_ON_VAL( NSL_SOUNDSTATUS_INVALID );
	// We handle the invalid sid case specifically below.

	if( id == NSL_INVALID_ID )
		return NSL_SOUNDSTATUS_INVALID; // FIXME: should be illegal?

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];

	if( !sound->used )
		return NSL_SOUNDSTATUS_INVALID;	// FIXME: should be illegal?

	if( id != sound->id )
		return NSL_SOUNDSTATUS_INVALID;

	if( sound->pauseCount > 0 )
		return NSL_SOUNDSTATUS_PAUSED;

	nslSource* source = sound->source;

	if( source->info->flags & NSL_SOURCE_INFO_STREAM )
	{
		int sid;;
		
		sid = _nslGetStreamId( sound );
		if( sid != -1 )
		{
			switch( nslStreamStatus( sid ) )
			{
				case NSL_STREAM_STATE_CLOSED:		return NSL_SOUNDSTATUS_INVALID;
				case NSL_STREAM_STATE_IDLE:			return NSL_SOUNDSTATUS_INVALID;
				case NSL_STREAM_STATE_LOADING:		return NSL_SOUNDSTATUS_QUEUING;
				case NSL_STREAM_STATE_LOADED:		return NSL_SOUNDSTATUS_READY;
				case NSL_STREAM_STATE_PLAYING:		return NSL_SOUNDSTATUS_PLAYING;
				case NSL_STREAM_STATE_PAUSED:		return NSL_SOUNDSTATUS_PAUSED;
				case NSL_STREAM_STATE_INTERRUPTED:
					return NSL_SOUNDSTATUS_QUEUING;
				
				default:
					NSL_ASSERT(0);
					return NSL_SOUNDSTATUS_INVALID;
			}
		}
		return NSL_SOUNDSTATUS_INVALID;
	}
		
	// regular
	if( sound->isPlaying )
		return NSL_SOUNDSTATUS_PLAYING;

	if( source->loaded )
		return NSL_SOUNDSTATUS_READY;
		
	return NSL_SOUNDSTATUS_QUEUING;
}

bool nslIsSoundPlaying( nslSoundId id )
{
	return nslGetSoundStatus( id ) == NSL_SOUNDSTATUS_PLAYING;
}

bool nslIsSoundReady( nslSoundId id )
{
	return nslGetSoundStatus( id ) == NSL_SOUNDSTATUS_READY;
}

void nslSetSoundParam( nslSoundId id, nslSoundParamEnum param, float value )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	nslSource* source = sound->source;
	assert( source );

	nslVerbosePrintf( "nslSetSoundParam: setting %s for '%s' (%d) to %f.\n", _nslGetSoundParamString( param ), source->info->name, id, value );

	switch( param ) 
	{
		case NSL_SOUNDPARAM_VOLUME:
			sound->rawVolume = value;
			_nslUpdateVolume( sound );
			break;
			
		case NSL_SOUNDPARAM_PITCH:
			sound->pitch = value;
			_nslUpdatePitch( sound, source );
			break;
			
		case NSL_SOUNDPARAM_MINDIST:
			sound->minDist = value;
			_nslUpdateVolume( sound );
			break;
			
		case NSL_SOUNDPARAM_MAXDIST:
			sound->maxDist = value;
			_nslUpdateVolume( sound );
			break;
			
		default:
			nslFatal( "nslSetSoundParam: invalid param.\n" );
			break;
	}
}

float	nslGetSoundParam( nslSoundId id, nslSoundParamEnum param )
{
	NSL_ENSURE_ON_VAL( 0.0f );
	NSL_ENSURE_SID_VAL( id, 0.0f );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	float value = 0.0f;

	switch( param ) 
	{
		case NSL_SOUNDPARAM_VOLUME: 
			value = sound->rawVolume;
			break;
			
		case NSL_SOUNDPARAM_PITCH:
			value = sound->pitch;
			break;
			
		case NSL_SOUNDPARAM_MINDIST:
			value = sound->minDist;
			break;
			
		case NSL_SOUNDPARAM_MAXDIST:
			value = sound->maxDist;
			break;
			
		default:
			nslFatal( "nslGetSoundParam: invalid param.\n" );
			break;
	}
	return value;
}

void nslSetSoundRange( nslSoundId id, float minDist, float maxDist )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];

	nslVerbosePrintf( "nslSetSoundRange: setting range to (%f,%f) for '%s' (%d).\n", minDist, maxDist, sound->source->info->name, id );

	sound->minDist = minDist;
	sound->maxDist = maxDist;
	_nslUpdateVolume( sound );
}

void nslSetSoundPosition( nslSoundId id, const nlVector3d& pos )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_SID( id );

	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( id ) ];
	// FIXME: print something for this? eh, i'm lazy.

	if( sound->emitter == NULL ) 
	{
		nslEmitterId emitter = nslCreateEmitter( pos );
		assert( emitter != NSL_INVALID_ID );
		if( emitter != NSL_INVALID_ID ) 
		{
			nslSetSoundEmitter( emitter, id );
			_nslUpdateVolume( sound );
		}
	} 
	else 
	{
		nslSetEmitterPosition( sound->emitter->id, pos );
	}
}

static nlInt32 _nslFindUnusedSound( void )
{
	for( int i = 0; i < NSL_NUM_SOUNDS; i++ )
	{
		nslSound* sound = &nsl.sounds[i];
		if( !sound->used )
			return i;
	}
	return -1;
}

void _nslReleaseSound( nlUint32 slot )
{
	assert( slot < NSL_NUM_SOUNDS );

	nslSound* sound = &nsl.sounds[slot];

	// remove us from the queue if we're there
	nsl.playQueue.erase( sound->id );

	if( sound->emitter != NULL )
	{
		_nslEmitterRemoveSound( sound->emitter, sound ); // remove us from our emitter's sound list
		sound->emitter = NULL;
	}

	// sound->id is left untouched in order to provide
	// unique ids to the application (+= NSL_INCREMENT_ID)

	// release sound->vpb first to catch any possible errors
	// with full sound info intact
	if( sound->vpb ) 
	{
		nslVerbosePrintf( "_nslReleaseSound: freeing AXVPB 0x%x (%d).\n", sound->vpb, sound->id );
		MIXReleaseChannel( sound->vpb );
		AXFreeVoice( sound->vpb );
		sound->vpb = NULL;
	}

	sound->source = NULL;

	sound->isPlaying = false;
	sound->isReallyPlaying = false;
	sound->frameDelayed = false;

	sound->pauseCount = 0;
	sound->dampenCount = 0;

	sound->rawVolume = 1.0f;
	sound->angle = 0.0f;
	sound->pitch = 1.0f;
	sound->minDist = NSL_DEFAULT_MIN_SOUND_DIST;
	sound->maxDist = NSL_DEFAULT_MAX_SOUND_DIST;

	if( sound->used )
		--nsl.soundCount;
	
	sound->used = false;
}

void _nslReleaseAllSounds( void )
{
	for( int i = 0; i < NSL_NUM_SOUNDS; i++ ) 
	{
		// changed from used to isPlaying...
		if( nsl.sounds[i].isPlaying )
			nslStopSound( nsl.sounds[i].id );
			
		// FIXME: hm, we'll call ReleaseSound twice here for any
		// playing sound...
		_nslReleaseSound( i );
	}
	nsl.soundCount = 0;
}

static void _nslVoiceCallback( void* p )
{
	AXVPB* vpb = (AXVPB*) p;
	nslSound* sound = (nslSound*) vpb->userContext;

#ifdef DEBUG
	sound->dropped = true;
#endif
	// zero out the vpb so it doesn't try to release it
	sound->vpb = NULL;

	// FIXME: requires synchronization primitive!
	_nslReleaseSound( NSL_GET_SLOT_FROM_ID( sound->id ) );
}

static float _nslCalcDistAttenuation( nslSound* sound )
{
	nslEmitter* emitter = sound->emitter;
	assert( emitter );

	float dx = emitter->position[0] - nsl.listenerPo[0][3];
	float dy = emitter->position[1] - nsl.listenerPo[1][3];
	float dz = emitter->position[2] - nsl.listenerPo[2][3];
	float dist2 = ( dx * dx ) + ( dy * dy ) + ( dz * dz );

	assert( sound->minDist >= 0.0f );
	assert( sound->maxDist >= 0.0f );

	float min2 = sound->minDist * sound->minDist;
	float max2 = sound->maxDist * sound->maxDist;
	float volume;

	if(      dist2 > max2 )	volume = 0.0f;
	else if( dist2 < min2 )	volume = 1.0f;
	else                    volume = ( max2 - dist2 ) / ( max2 - min2 );

	return volume;
}

static void _nslUpdatePitch( nslSound* sound, nslSource* source )
{
	assert( sound );

	if( !sound->vpb ) 
		return;	// updating the pitch before playing, s'cool

	assert( source );
	assert( source->info );

	f32 fp = sound->pitch * ( (f32) source->info->frequency / (f32) AX_IN_SAMPLES_PER_SEC );

	if( sound->vpb->pb.state == AX_PB_STATE_RUN ) 
	{
		AXSetVoiceSrcRatio( sound->vpb, fp );
		return;
	}

	u32 up = (u32) ( 0x00010000 * fp );

	AXPBSRC src;
	
	src.ratioHi = AR_ADDR_HI( up );
	src.ratioLo = AR_ADDR_LO( up );
	src.currentAddressFrac  = 0;
	src.last_samples[0] = 0;
	src.last_samples[1] = 0;
	src.last_samples[2] = 0;
	src.last_samples[3] = 0;
	
	AXSetVoiceSrc( sound->vpb, &src );
	AXSetVoiceSrcType( sound->vpb, AX_SRC_TYPE_4TAP_12K );
}

void _nslCalcTwoChannel( nslSound* sound, nslSource* source, f32& left, f32& right )
{
	float volumeModifier = 1.0f;

	switch( source->info->type ) 
	{
		case NSL_SOURCETYPE_SFX:
			volumeModifier = nsl.sfxVolume;
			break;
		case NSL_SOURCETYPE_AMBIENT:
			volumeModifier = nsl.ambientVolume;
			break;
		case NSL_SOURCETYPE_MUSIC:
			volumeModifier = nsl.musicVolume;
			break;
		case NSL_SOURCETYPE_VOICE:
			volumeModifier = nsl.voiceVolume;
			break;
		case NSL_SOURCETYPE_MOVIE:
			volumeModifier = nsl.movieVolume;
			break;
		default:
			nslFatal( "_nslCalcTwoChannel: illegal source info type.\n" );
			break;
	}

	if( sound->dampenCount > 0 )
		volumeModifier *= nsl.dampenLevel;

	left  = nsl.masterVolume * sound->rawVolume * volumeModifier;
	right = nsl.masterVolume * sound->rawVolume * volumeModifier;
	// These are always the same, so there's no point in checking
	// if the speaker mode is mono or not. Makes the function
	// somewhat pointless, yes...
}

void _nslUpdateVolumeGlobal( nslSound* sound, nslSource* source )
{
	f32 left = 1.0f;
	f32 right = 1.0f;
	
	_nslCalcTwoChannel( sound, source, left, right );
	
	if( source->info->flags & NSL_SOURCE_INFO_STREAM )
	{
		int id = _nslGetStreamId(sound);
		if( id != -1 )
			nslStreamSetVolume( id, left );
	}
	else
	{
		// left == right here always (yeah, stupid, I know)
		int atten = _nslCalcDecibelAtten( left );
		MIXSetInput( sound->vpb, atten );
		MIXSetPan( sound->vpb, 64 );
		MIXSetSPan( sound->vpb, 127 );
	}
}

void _nslUpdateVolume( nslSound* sound )
{
	sound->needVolumeUpdate = true;
}

float _nlVectorLength( nlVector3d v );

static float _nslCalcSoundAngle( nslSound* sound )
{
	nlVector4d fwd, at, up;

	fwd[0] = 0.0f;
	fwd[1] = 0.0f;
	fwd[2] = -1.0f;
	fwd[3] = 0.0f;

	up[0] = 0.0f;
	up[1] = 1.0f;
	up[2] = 0.0f;
	up[3] = 0.0f;

	nslEmitter* emitter = sound->emitter;
	at[0] = nsl.listenerPo[0][3] - emitter->position[0];
	at[1] = nsl.listenerPo[1][3] - emitter->position[1];
	at[2] = nsl.listenerPo[2][3] - emitter->position[2];
	at[3] = 0.0f;

	float length = _nlVectorLength( at );

	if( length < 0.001f )
		return 0.0f;

	float recip = 1.0f / length;
	at[0] *= recip;
	at[1] *= recip;
	at[2] *= recip;
	at[3]  = 0.0f;

	nlTransformVector( fwd, nsl.listenerPo, fwd );

	length = _nlVectorLength( fwd );

	if( length < 0.001f )
		return 0.0f;

	recip = 1.0f / length;
	fwd[0] *= recip;
	fwd[1] *= recip;
	fwd[2] *= recip;
	fwd[3]  = 0.0f;

	float cosTheta = nlDotProduct3d( at, fwd );
	float angle;

	if( cosTheta >= 0.999f ) 
	{
		angle = 0.0f;
	} 
	else if( cosTheta <= -0.999f ) 
	{
		angle = M_PI;
	} 
	else 
	{
		nlVector4d newUp;

		nlCrossProduct3d( newUp, at, fwd );
		nlTransformVector( up, nsl.listenerPo, up );

		length = _nlVectorLength( up );

		if( length < 0.001f )
			return 0.0f;

		recip = 1.0f / length;
		up[0] *= recip;
		up[1] *= recip;
		up[2] *= recip;
		up[3]  = 0.0f;

		float cosPhi = nlDotProduct3d( up, newUp );
		angle = acosf( cosTheta );

		if( cosPhi >= 0.0f )
			angle = -angle;

		// FIXME: extremely hokey
		while( angle < 0.0f )
			angle += ( M_PI * 2.0f );
	}
	return angle;
}

void _nslUpdateVolume3D( nslSound* sound, nslSource* source )
{
  float distAtten = _nslCalcDistAttenuation( sound );

	if( distAtten <= 0.01f ) 
	{
		// FIXME: is this all we should be doing?
		MIXSetInput( sound->vpb, -960 );
		
		nslVerbosePrintf( "_nslUpdateVolume3D: attenuation causing cut-off, muting '%s' (%d).\n", source->info->name, sound->id );
		return;
	}

	sound->angle = _nslCalcSoundAngle( sound );

	// balance is ignored for 3D sounds
	float left = 1.0f;
	float right = 1.0f;
	float volumeModifier = 1.0f;

	switch( source->info->type ) 
	{
		case NSL_SOURCETYPE_SFX:
			volumeModifier = nsl.sfxVolume;
			break;
		case NSL_SOURCETYPE_AMBIENT:
			volumeModifier = nsl.ambientVolume;
			break;
		case NSL_SOURCETYPE_MUSIC:
			volumeModifier = nsl.musicVolume;
			break;
		case NSL_SOURCETYPE_VOICE:
			volumeModifier = nsl.voiceVolume;
			break;
		case NSL_SOURCETYPE_MOVIE:
			volumeModifier = nsl.movieVolume;
			break;
		default:
			nslFatal( "_nslPlaySoundGlobal: illegal source info type.\n" );
			break;
	}

	volumeModifier = nsl.masterVolume * volumeModifier * sound->rawVolume * distAtten;

	if( sound->dampenCount > 0 )
		volumeModifier *= nsl.dampenLevel;

	int atten = _nslCalcDecibelAtten( volumeModifier );

	if( nsl.speakerMode == NSL_SPEAKER_MONO ) 
	{
		MIXSetInput( sound->vpb, atten );
		MIXSetPan( sound->vpb, 64 );
		MIXSetSPan( sound->vpb, 127 );
		return;
	}

	float x = cosf( sound->angle + ( M_PI / 2.0f ) );
	float y = sinf( sound->angle + ( M_PI / 2.0f ) );

	int pan = 64 + ( (int) ( x * 64.0f ) );
	int span = 64 + ( (int) ( y * 64.0f ) );

	MIXSetInput( sound->vpb, atten );
	MIXSetPan( sound->vpb, pan );
	MIXSetSPan( sound->vpb, span );
}

static void _nslPauseSound( nslSound* sound )
{
	if( sound->pauseCount < 0 ) 
	{
		nslVerbosePausePrintf( "_nslPauseSound: ignoring, pause-guarded sound: '%s' (%d).\n", sound->source->info->name, sound->id );
		// pause guard
		return;
	}

	++sound->pauseCount;

	if( sound->pauseCount == 1 ) 
	{
		nslSource* source = sound->source;
		
		if( source->info->flags & NSL_SOURCE_INFO_STREAM )
		{
			int sid = _nslGetStreamId( sound );
			if( sid != -1 )
				nslStreamPause( sid );
		} 
		else 
		{
			assert( sound->vpb );
			nslVerbosePrintf( "_nslPauseSound: setting voice state to stop for '%s' (%d).\n", sound->source->info->name, sound->id );
			AXSetVoiceState( sound->vpb, AX_PB_STATE_STOP );
		}
	}
}

static void _nslUnpauseSound( nslSound* sound )
{
	if( sound->pauseCount < 0 ) 
	{
		nslVerbosePausePrintf( "_nslUnpauseSound: ignoring, pause-guarded sound: '%s' (%d).\n", sound->source->info->name, sound->id );
		// pause guard
		return;
	}

	if( sound->pauseCount == 0 )
	{
		// FIXME: should be an error.
		nslVerbosePausePrintf( "_nslUnpauseSound: ignoring, sound not paused: '%s' (%d).\n", sound->source->info->name, sound->id );
		return;
	}

	--sound->pauseCount;

	if( sound->pauseCount == 0 ) 
	{
		nslSource* source = sound->source;
		
		if( source->info->flags & NSL_SOURCE_INFO_STREAM ) 
		{
			int sid = _nslGetStreamId( sound );
			if( sid != -1 )
				nslStreamUnpause( sid );
		} 
		else 
		{
			assert( sound->vpb );
			AXSetVoiceState( sound->vpb, AX_PB_STATE_RUN );
			nslVerbosePausePrintf( "_nslUnpauseSound: setting voice state to run for '%s' (%d).\n", sound->source->info->name, sound->id );
		}
	}
}

void _nslUpdateVolumeOfType( nslSourceTypeEnum type )
{
	for( int i = 0; i < NSL_NUM_SOUNDS; ++i ) 
	{
		nslSound* sound = &nsl.sounds[i];
		if( sound->used ) 
		{
			if( sound->source->info->type == type )
				_nslUpdateVolume( sound );		
		}
	}
}

void _nslUpdateVolumeAll( void )
{
	for( int i = 0; i < NSL_NUM_SOUNDS; ++i ) 
	{
		nslSound* sound = &nsl.sounds[i];
		if( sound->used ) 
		{
			_nslUpdateVolume( sound );
		}
	}
}

static const char* _nslGetSoundParamString( nslSoundParamEnum param )
{
	const char* s = "INVALID";
	const char* ps[] = { "volume", "pitch", "min dist", "max dist" };

	if( param >= 0 && param < NSL_SOUNDPARAM_Z )
		s = ps[param];
	
	return s;
}
