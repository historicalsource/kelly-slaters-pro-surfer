#include "common/nsl.h"
#include "ps2/nsl_ps2.h"

//////////////////////// nsl emitter api functions ////////////////////////


/*-------------------------------------------------------------------------
  nsl Create Emitter
-------------------------------------------------------------------------*/
nslEmitterId nslCreateEmitter( const nlVector3d &position )
{
  if (!nsl.on) return 1;
  ASSERT_NSL_INITIALIZED();
  for (int i=0; i < NSL_NUM_EMITTERS; i++) 
  {
    if (!nsl.emitterSlots[i].used) 
    {
      _nslClearEmitterSlot(i, false, NSL_CLEAR_RESET);
      nsl.emitterSlots[i].used = true;
      nsl.emitterSlots[i].autoRelease = false;
      nsl.emitterSlots[i].isALineEmitter = false;
      nsl.emitterSlotsUsedCount++;
      memcpy(nsl.emitterSlots[i].position, position, sizeof(nlVector3d));
      /*nsl.emitterSlots[i].position[0] = position[0];
      nsl.emitterSlots[i].position[1] = position[1];
      nsl.emitterSlots[i].position[2] = position[2];*/
      nsl.emitterSlots[i].myId += NSL_ID_INCREMENT;
      return nsl.emitterSlots[i].myId;
    }
  }
  return NSL_INVALID_ID;
}


/*-------------------------------------------------------------------------
  nsl Create Line Emitter
-------------------------------------------------------------------------*/
nslEmitterId nslCreateLineEmitter( const nlVector3d &startPos, const nlVector3d &endPos )
{
  if (!nsl.on) return 1;
  ASSERT_NSL_INITIALIZED();
  for (int i=0; i < NSL_NUM_EMITTERS; i++) 
  {
    if (!nsl.emitterSlots[i].used) 
    {
      _nslClearEmitterSlot(i, false, NSL_CLEAR_RESET);
      nsl.emitterSlots[i].used = true;
      nsl.emitterSlots[i].autoRelease = false;
      nsl.emitterSlots[i].isALineEmitter = true;
      nsl.emitterSlotsUsedCount++;
      memcpy(nsl.emitterSlots[i].startPosition, startPos, sizeof(nlVector3d));
      memcpy(nsl.emitterSlots[i].endPosition, endPos, sizeof(nlVector3d));
      memcpy(nsl.emitterSlots[i].position, startPos, sizeof(nlVector3d));
      nsl.emitterSlots[i].myId += NSL_ID_INCREMENT;
      return nsl.emitterSlots[i].myId;
    }
  }
  return NSL_INVALID_ID;
}


/*-------------------------------------------------------------------------
  nsl Release Emitter
-------------------------------------------------------------------------*/
void nslReleaseEmitter( nslEmitterId emitterToRelease )
{

  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_EMITTER_ID_VALID(emitterToRelease);
  nsl.emitterSlotsUsedCount--;
  
  while (nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToRelease)].emittedSounds.size())
  {
    nslSoundId mySound = nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToRelease)].emittedSounds.pop();
    if ( nslGetSoundStatus( mySound ) != NSL_SOUNDSTATUS_INVALID )
      nslStopSound(mySound);  
  }

  _nslClearEmitterSlot(NSL_GET_SLOT_FROM_ID(emitterToRelease), false, NSL_CLEAR_RESET);
}


/*-------------------------------------------------------------------------
  nsl Set Emitter Position
-------------------------------------------------------------------------*/
void nslSetEmitterPosition( nslEmitterId emitterToSet, const nlVector3d &newPosition )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_EMITTER_ID_VALID(emitterToSet);
  memcpy(nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToSet)].position, newPosition, sizeof(nlVector3d));
  /*nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToSet)].position[0] = newPosition[0];
  nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToSet)].position[1] = newPosition[1];
  nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToSet)].position[2] = newPosition[2];*/

}


/*-------------------------------------------------------------------------
  nsl Set Emitter Auto Release
-------------------------------------------------------------------------*/
void nslSetEmitterAutoRelease( nslEmitterId emitterToSet )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_EMITTER_ID_VALID(emitterToSet);
  nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(emitterToSet)].autoRelease = true;
}


/*-------------------------------------------------------------------------
  nsl Release All Emitters
-------------------------------------------------------------------------*/
void _nslReleaseAllEmitters()
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  for (int i=0; i < NSL_NUM_SOUNDS; i++) 
  {
    if ((nsl.soundSlots[i].myEmitter != NSL_GLOBAL_EMITTER_ID) &&  (nsl.soundSlots[i].used == true)) 
    {
      nslStopSound(nsl.soundSlots[i].myId);
    }
  }
  for (int i=0; i < NSL_NUM_EMITTERS; i++) 
  {
    _nslClearEmitterSlot(i, false, NSL_CLEAR_RESET);
  }
}


/*-------------------------------------------------------------------------
  nsl Set Sound Emitter
-------------------------------------------------------------------------*/
// all instances start with the default 'global' emitter
// this re-assigns an instance to a different one
void nslSetSoundEmitter( nslEmitterId soundEmitter, nslSoundId soundInstance )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_EMITTER_ID_VALID(soundEmitter);
  ASSERT_SOUND_ID_VALID(soundInstance);
  
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundInstance)].myEmitter != NSL_GLOBAL_EMITTER_ID) 
  {
    nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundInstance)].myEmitter)].emittedSounds.find(soundInstance, true);
  }
  if (soundEmitter != NSL_GLOBAL_EMITTER_ID)
    nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(soundEmitter)].emittedSounds.push(soundInstance);

  nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundInstance)].myEmitter = soundEmitter;

}

/////////////////////////// internal functions ////////////////////////////

// none yet.
