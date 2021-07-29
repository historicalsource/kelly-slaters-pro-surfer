#include "gamecube/nsl_gc.h"

// clears out data structure
static void _nslClearEmitter( nlUint32 slot )
{
	assert( slot < NSL_NUM_EMITTERS );

	nslEmitter* emitter = &nsl.emitters[slot];

	// leave emitter to produce unique ids
	emitter->position[0] = 0.0f;
	emitter->position[1] = 0.0f;
	emitter->position[2] = 0.0f;

	// zero out sounds list
	memset( emitter->sounds, 0, sizeof( emitter->sounds ) );
	
	emitter->numSounds = 0;
	emitter->autoRelease = false;
	emitter->used = false;
}

nslEmitterId nslCreateEmitter( const nlVector3d& position )
{
	NSL_ENSURE_ON_VAL( NSL_INVALID_ID );

	for( int i = 0; i < NSL_NUM_EMITTERS; i++ ) 
	{
		nslEmitter* emitter = &nsl.emitters[i];

		if( emitter->used )
	    	continue;

		_nslClearEmitter( i );

		emitter->id += NSL_ID_INCREMENT;

		emitter->position[0] = position[0];
		emitter->position[1] = position[1];
		emitter->position[2] = position[2];

		emitter->used = true;

		++nsl.emitterCount;

		return emitter->id;
	}

	return NSL_INVALID_ID;
}

nslEmitterId nslCreateLineEmitter( const nlVector3d& start, const nlVector3d& end )
{
	nlVector3d midpoint;

	midpoint[0] = start[0] + ( ( end[0] - start[0] ) / 2 );
	midpoint[1] = start[1] + ( ( end[1] - start[1] ) / 2 );
	midpoint[2] = start[2] + ( ( end[2] - start[2] ) / 2 );

	return nslCreateEmitter( midpoint );
}

void nslReleaseEmitter( nslEmitterId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_EID( id );

	nslEmitter* emitter = &nsl.emitters[ NSL_GET_SLOT_FROM_ID( id ) ];

	for( int i = 0; i < emitter->numSounds; ++i ) 
	{
		nslSound* sound = emitter->sounds[i];

		assert( sound );

		if( nslGetSoundStatus( sound->id ) != NSL_SOUNDSTATUS_INVALID )
      		nslStopSound( sound->id );
	}

	if( emitter->used )
		--nsl.emitterCount;
	  
	_nslClearEmitter( NSL_GET_SLOT_FROM_ID( id ) );
}

void nslSetEmitterPosition( nslEmitterId id, const nlVector3d& position )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_EID( id );

	nslEmitter* emitter = &nsl.emitters[ NSL_GET_SLOT_FROM_ID( id ) ];

	emitter->position[0] = position[0];
	emitter->position[1] = position[1];
	emitter->position[2] = position[2];

	for( int i = 0; i < emitter->numSounds; ++i ) 
	{
		nslSound* sound = emitter->sounds[i];
		assert( sound );
		
		nslSource* source = sound->source;
		assert( source );

		_nslUpdateVolume( sound );
	}
}

void nslSetEmitterAutoRelease( nslEmitterId id )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_EID( id );

	nslEmitter* emitter = &nsl.emitters[ NSL_GET_SLOT_FROM_ID( id ) ];

	emitter->autoRelease = true;
}

void _nslEmitterRemoveSound( nslEmitter* emitter, nslSound* sound )
{
	if( emitter->numSounds <= 0 )
		return;

	// so we always have at least one sound
	for( int i = 0; i < emitter->numSounds; ++i ) 
	{
		if( emitter->sounds[i] == sound ) 
		{
			// dec count, swap last into, zero out the new empty slot
			--emitter->numSounds;
			emitter->sounds[i] = emitter->sounds[emitter->numSounds];
			emitter->sounds[emitter->numSounds] = NULL;

			return;
		}
	}
}

void _nslEmitterAddSound( nslEmitter* emitter, nslSound* sound )
{
	if( emitter->numSounds >= NSL_NUM_EMITTER_SOUNDS ) 
		return;

	emitter->sounds[emitter->numSounds] = sound;
	++emitter->numSounds;
}

// all instances start with the default 'global' emitter
// this re-assigns an instance to a different one
void nslSetSoundEmitter( nslEmitterId eid, nslSoundId sid )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_EID( eid );
	NSL_ENSURE_SID( sid );

	nslEmitter* emitter = &nsl.emitters[ NSL_GET_SLOT_FROM_ID( eid ) ];
	nslSound* sound = &nsl.sounds[ NSL_GET_SLOT_FROM_ID( sid ) ];

	if( sound->emitter != NULL ) 
	{
  		// remove from old emitter
	  	_nslEmitterRemoveSound( sound->emitter, sound );
	}

	_nslEmitterAddSound( emitter, sound );
	sound->emitter = emitter;
}

void _nslReleaseAllEmitters( void )
{
	for( int i = 0; i < NSL_NUM_EMITTERS; i++ )
		_nslClearEmitter( i );

	nsl.emitterCount = 0;
}
