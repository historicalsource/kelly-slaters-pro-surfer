#include "nsl_xbox.h"

//
//
// slEmitter structure and functions
//
//
void _nslAddSoundToEmitter(nslEmitterId id, nslSoundId sId)
{
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(sId)];
	internalAssert(theSound->next == NULL);  // this sound was added some where else?  remove it first;
	nslEmitter* theEmitter = &sndSystem.emitters[id];

	if(!theEmitter->soundList) // first element on the soundList
		theEmitter->soundList = theSound;
	else
		theEmitter->lastSound->next = theSound;

	theEmitter->lastSound = theSound;
	theSound->eId = id;
	theSound->next = NULL;
}

void _nslRemoveSoundFromEmitter(nslEmitterId id, nslSoundId sId)
{
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(sId)];
	nslEmitter* theEmitter = &sndSystem.emitters[id];
	nslSound* curr = theEmitter->soundList;
	nslSound* prev = NULL;
	while (curr)
	{
		if (curr == theSound)
		{
			if (!prev)  // removing the first or the only element of the list
			{
				if(theEmitter->soundList == theEmitter->lastSound) // there was only one element
					theEmitter->lastSound = NULL;
				theEmitter->soundList = theSound->next;
			}
			else	// the element is in the middle or the last element of the list
			{
				prev->next = theSound->next;
				if(theEmitter->lastSound == theSound)	// removing the last element of the list
					theEmitter->lastSound = prev;
			}
			break;
		}
		else
		{
			prev = curr;
			curr = curr->next;
			internalAssert(curr); // this sound was not in the soundList at all!!!
		}
	}
	theSound->next = NULL;
	theSound->eId = NSL_INVALID_ID;
}

void _nslClearEmitter(nslEmitterId id)
{
	nslEmitter* theEmitter = &sndSystem.emitters[id];
	internalAssert( theEmitter->soundList == NULL );
	memset(theEmitter, 0, sizeof(nslEmitter));

	--sndSystem.numEmitters;
}

int _nslFindNewEmitterSlot()
{
/*
  int freeemitters = 0;

	for (int i=0; i<NSL_NUM_EMITTERS; ++i)
	{
		if (i != NSL_GLOBAL_EMITTER_ID)
		{
			if (!sndSystem.emitters[i].isUsed && !sndSystem.emitters[i].isDead)
      {
        ++freeemitters;
      }
		}
	}
  nslPrintf( "Free emitters: %d\n", freeemitters );
*/

	for (int i=0; i<NSL_NUM_EMITTERS; ++i)
	{
		if (i != NSL_GLOBAL_EMITTER_ID)
		{
			if (!sndSystem.emitters[i].isUsed && !sndSystem.emitters[i].isDead)
				return i;
		}
	}

	return NSL_INVALID_ID;
}

nslEmitterId nslCreateLineEmitter( const nlVector3d &startPosition, const nlVector3d &endPosition )
{
	return (nslCreateEmitter(startPosition));
}

nslEmitterId nslCreateEmitter( const nlVector3d &position )
{
#if DISABLE_EMITTERS
	return NSL_INVALID_ID;
#else
	nslEmitterId id = (unsigned int)_nslFindNewEmitterSlot();
	if (id == NSL_INVALID_ID)
	{
		nslPrintf("NSL WARNING: Out of emitter slots");
		return NSL_INVALID_ID;
	}

	nslEmitter &theEmitter = sndSystem.emitters[id];
	theEmitter.id = id;
	theEmitter.isUsed = true;
	theEmitter.isAutoRelease = false;
	theEmitter.isDead = false;
	memcpy(&theEmitter.pos, position, sizeof(nlVector3d));
	internalAssert(theEmitter.soundList == NULL);

	++sndSystem.numEmitters;

	return id;
#endif
}


// this only marks the emitter as dead, the emitter is not freed until all its sounds are dead
void nslReleaseEmitter( nslEmitterId emitterToRelease )
{
#if !DISABLE_EMITTERS
	nslSound *theSound, *nextSound;
	
	RETURN_ON_INVALID_ID(emitterToRelease);
	if (!sndSystem.emitters[emitterToRelease].isUsed)
		return;
	
	nslEmitter &theEmitter = sndSystem.emitters[emitterToRelease];
	theSound = theEmitter.soundList;
	while (theSound)
	{
		// Preserve this pointer, since it get's changed bt nslSetSoundEmitter
		nextSound = theSound->next;
		// Change it so that it will stop
		nslSetSoundEmitter(NSL_GLOBAL_EMITTER_ID, theSound->id);
		// Stop it
		nslStopSound(theSound->id);
		// Move on
		theSound = nextSound;
	}
	// mark this emitter dead, but it shouldn't be killed until all its sound stop playing
	theEmitter.isUsed = false;
	theEmitter.isDead = true;
#endif
}

void nslSetEmitterPosition( nslEmitterId emitterToSet, const nlVector3d &newPosition )
{
#if !DISABLE_EMITTERS
	RETURN_ON_INVALID_ID(emitterToSet);
	memcpy(&sndSystem.emitters[emitterToSet].pos, newPosition, sizeof(nlVector3d));
#endif
}

void nslSetEmitterAutoRelease( nslEmitterId emitterToSet )
{
#if !DISABLE_EMITTERS
	RETURN_ON_INVALID_ID(emitterToSet);
	sndSystem.emitters[emitterToSet].isAutoRelease = true;
#endif
}

void _nslReleaseAllEmitters()
{
#if !DISABLE_EMITTERS
	for(int i=0; i<NSL_NUM_EMITTERS; ++i)
	{
		if(i != NSL_GLOBAL_EMITTER_ID)
		{
			if (!sndSystem.emitters[i].isUsed && !sndSystem.emitters[i].isDead)
				continue;
			
			nslReleaseEmitter(sndSystem.emitters[i].id);
		}
	}
#endif
}


// all instances start with the default 'global' emitter
// this re-assigns an instance to a different one
void nslSetSoundEmitter( nslEmitterId soundEmitter, nslSoundId soundInstance )
{
#if !DISABLE_EMITTERS
	RETURN_ON_INVALID_ID(soundInstance);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundInstance)];
	nslSource* theSource = &sndSystem.sources[theSound->srcId];
	if( !(theSource->sndEntry.channels == 1) )
	{
#ifdef NSL_LOAD_SOURCE_BY_NAME
		nslFatal("NSL ERROR: 3D Sounds must be mono, %s is not.", theSource->sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
		nslFatal("NSL ERROR: 3D Sounds must be mono, %d is not.", theSource->sndEntry.aliasID);
#endif

		return;
	}
	if (sndSystem.num3DSounds+1 > 64)
	{
#ifdef NSL_LOAD_SOURCE_BY_NAME
		nslPrintf("NSL ERROR: Trying to play too many 3D sounds at once.  %s has been dropped.", theSource->sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
		nslPrintf("NSL ERROR: Trying to play too many 3D sounds at once.  %d has been dropped.", theSource->sndEntry.aliasID);
#endif

		nslStopSound(soundInstance);
		return;
	}
	internalAssert(theSound->isUsed);
	if (!theSound->isUsed)
		return;

	if (theSound->eId == soundEmitter)
		return;
	
	nslEmitter *theEmitter;
	// remove the sound from its current emitter
	theEmitter = &sndSystem.emitters[theSound->eId];
	internalAssert(theEmitter->isUsed);
	if (!theEmitter->isUsed)	// dead emitter?
		return;		
	_nslRemoveSoundFromEmitter(theEmitter->id, theSound->id);
	
	// add the sound to the new emitter
	theEmitter = &sndSystem.emitters[soundEmitter];
	internalAssert(theEmitter->isUsed);
	if (!theEmitter->isUsed)	// dead emitter?
		return;
	_nslAddSoundToEmitter(theEmitter->id, theSound->id);

	// Stream sound doesn't get started until its own thread gets started.
#if !DISABLE_STREAMS
	if (!theSound->streamSlot)
#elif !DISABLE_NEW_STREAMS
	if (theSound->newStreamSlot < 0)
#endif
	{
		// This sucks.  We have to use a new DSound obj.
		DSBUFFERDESC desc;
		memcpy( &desc, &theSource->desc, sizeof(DSBUFFERDESC) );
		//desc.dwFlags |= DSBCAPS_CTRL3D;
		DS_TRY( theSound->pDSBuffer->Release() );
		DS_TRY( sndSystem.pDS->CreateSoundBuffer( &desc , &theSound->pDSBuffer, NULL ) );
		DS_TRY( theSound->pDSBuffer->SetBufferData( theSource->data, theSource->sndEntry.size ) );
		DS_TRY( theSound->pDSBuffer->SetLoopRegion( 0, theSource->sndEntry.size ) );
		DS_TRY( theSound->pDSBuffer->SetCurrentPosition( 0 ) );
		//DS_TRY( theSound->pDSBuffer->SetMaxDistance( 1.0f, DS3D_IMMEDIATE ) );
	}
	// set the sound's emitter id to current emitter
	theSound->eId = soundEmitter;

	++sndSystem.num3DSounds;
#endif
}
