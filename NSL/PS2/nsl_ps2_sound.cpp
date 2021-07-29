#include "common/nsl.h"
#include "ps2/nsl_ps2.h"
#include "ps2/gas.h"
#include "ps2/gas_utility.h"
#include <math.h>
#include "common/nl.h"

///////////////////////// nsl sound api functions /////////////////////////


/*-------------------------------------------------------------------------
  nsl Add Sound 
-------------------------------------------------------------------------*/
static nslSoundId _nslAddSound( nslSourceId soundSource, int streamOffset=0, int streamSamples=0 )
{
  if (!nsl.on) return NSL_INVALID_ID;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(soundSource);

  for (int i=0; i < NSL_NUM_SOUNDS; i++) 
  {
    if (!nsl.soundSlots[i].used) 
    {
      _nslClearSoundSlot(i, false);
      // Get the gasInstanceID
      nslSource *src;
      nslSound *snd;
      snd = &nsl.soundSlots[i];
      src = &nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(soundSource)];

      snd->myId += NSL_ID_INCREMENT;
      snd->inRange = true;
      if (src->length >= NSL_SMALL3D_SOUND_SIZE)
      {
        snd->gasInstanceId = nslPs2GasRpc(GAS_RPC_ADD_INSTANCE, "", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(soundSource)].gasSourceId, 0, streamOffset, streamSamples);
        if (snd->gasInstanceId == (nlUint32)GAS_INVALID_ID) 
        {
          snd->inRange = false;
          return (nslSoundId)NSL_INVALID_ID;
        }
        snd->gasInstanceId =  ~GAS_INSTANCE_STEREO_FLAG_BIT & nsl.soundSlots[i].gasInstanceId;
      }
      else
      {
        snd->inRange = false;
        snd->gasInstanceId = (nlUint32)GAS_INVALID_ID;
      }
      
      snd->myEmitter = NSL_GLOBAL_EMITTER_ID;
      
      // Setup associations
      snd->mySource = soundSource;
      snd->used = true;     
      nsl.soundSlotsUsedCount++;
    
      snd->angle = 0;
    
      snd->minDist = src->minDist;
      snd->maxDist = src->maxDist;
    
      snd->positionalVolume = 1.0f;
      //snd->rawVolume = src->rawVolume;
			snd->rawVolume = 1.0f;

      snd->pitch = src->pitch;
      snd->old_left = 0;
      snd->old_right = 0;

//      snd->balance = src->balance;

    
      snd->pauseCount = 0;
      snd->dampenCount = 0;
    
    

      snd->isPlaying = false;
      snd->isReallyPlaying = false;
      snd->isReady = false;
      snd->isQueuing = true;
      snd->looping = src->looping;
    
    
    
  
      // We do the pitch here since it is not 
      // automatically updated on the frame
      // we don't want to send tons of pitch calls
      // unnecessarily, so we only do it as need be
      // ie whenever its different to start, and 
      // whenever we change
      if ((snd->pitch != 1.0f) && (snd->gasInstanceId != (nlUint32)GAS_INVALID_ID))
      {
        nslPs2GasRpc(GAS_RPC_INSTANCE_PITCH, "", 
          snd->gasInstanceId,
          (unsigned char)(snd->pitch*GAS_COMMAND_PITCH_ONE),
          0, 0);
      }
      // Return
			
      return snd->myId;
    }
  }
	
  return NSL_INVALID_ID;
}

nslSoundId nslAddSound( nslSourceId soundSource )
{
	return _nslAddSound( soundSource, 0, 0 );
}

nslSoundId nslStreamAddSound( nslSourceId soundSource, int streamOffset, int streamSamples )
{
	return _nslAddSound( soundSource, streamOffset, streamSamples );
}

/*-------------------------------------------------------------------------
  nsl Play Sound 
-------------------------------------------------------------------------*/
void nslPlaySound( nslSoundId soundToPlay )
{

  /*
    The thing is, we are trying to limit how sounds tie up resources.  
    Sounds normally get resources on the PS2 when they are added.  
    If they aren't played for a while, this is a problem.  So we have 2 strategies.  
    The first is that looping 3d sounds aren't played until they are in range.  
    This is most of the checks about gasInstanceId != GAS_INVALID_ID.  
    The second strategy is that a small sound that is out of range when played, we ignore.  
    The threshold is NSL_SMALL3D_SOUND_SIZE.  The idea being that the sound will probably 
    have stopped by the time it gets to an audible place.  
  */
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToPlay)].mySource);
  

  ASSERT_SOUND_ID_VALID(soundToPlay);
  nslSound *snd;

  snd = &nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToPlay)];
  // We consider all sounds (incl. queued) to be playing once
  // they've been issued a play command
  snd->isPlaying = true; 
  snd->pauseCount= 0;
	
		

  // GAS should queue now
  
  if ( (!snd->isReallyReady) && 
       !(
          (nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].length < NSL_SMALL3D_SOUND_SIZE) &&
          (snd->gasInstanceId == (nlUint32)GAS_INVALID_ID) &&
          !(snd->looping)
        )
       
     ) 
  {
#ifdef DEBUG_OUTPUT
    nslPrintf("PUSHING %s for the first time\n", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].fileName);
#endif
    nsl.queuedSounds.push(soundToPlay);
    return;
  }
  
  
  // Play in standard, mono (as assumed for Global Emitters... 
  // We should look at balance, too
  if (snd->myEmitter == NSL_GLOBAL_EMITTER_ID) 
  {
    snd->positionalVolume = 1.0f;
    snd->angle = 0;
    
    float volumeModifier = 1.0f;
    switch (nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].type) {
      case NSL_SOURCETYPE_SFX:     volumeModifier = nsl.sfxVolume;     break;
      case NSL_SOURCETYPE_AMBIENT: volumeModifier = nsl.ambientVolume; break;
      case NSL_SOURCETYPE_MUSIC:   volumeModifier = nsl.musicVolume;   break;
      case NSL_SOURCETYPE_VOICE:   volumeModifier = nsl.voiceVolume;   break;
      case NSL_SOURCETYPE_MOVIE:   volumeModifier = nsl.movieVolume;   break;
      case NSL_SOURCETYPE_USER1:   volumeModifier = nsl.user1Volume;   break;
      case NSL_SOURCETYPE_USER2:   volumeModifier = nsl.user2Volume;   break;

      default: nslFatal("FAIL"); break;
    }

    // BALANCE - It may not be right,
    // but it works for us
    // Discussed with Andy.. 
    // Sound balance spreads sound to left and right
    // Master balance dampens the side it is NOT on.
    // ie src = -.4 master = .5  vol = .8
    // SRC:
    //   volL = vol * ((1 - src)/2)  
    //   volR = vol * ((1 + src)/2)
    // MASTER:
    //   volL = volL - (master * vol_left);


    // DO Sound levels  
    float left = nsl.masterVolume *
                 snd->rawVolume   *
                 volumeModifier;
//               * (1-snd->balance)/2;

    float right =nsl.masterVolume *
                 snd->rawVolume *
                 volumeModifier;
//                 * (1+snd->balance)/2;
    if (nsl.speakerMode == NSL_SPEAKER_MONO)
      right = left;

    // Dampen via master balance.. 
/*
    if (nsl.balance > 0) 
      left = left - (nsl.balance * left);
    else 
      right = right + (nsl.balance * right);
*/
// unused?    if (left > 0)
// unused?      int testval = 0;


    // If this was a small sound that wasn't added.. (can this happen??)
    if ((nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].length < NSL_SMALL3D_SOUND_SIZE) &&
        (snd->gasInstanceId == (nlUint32)GAS_INVALID_ID) &&
        !(snd->looping))
    {
      // Add the instance
      snd->gasInstanceId = nslPs2GasRpc(GAS_RPC_ADD_INSTANCE, "", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].gasSourceId, 0, 0, 0);
      // If it failed, free it
      if (snd->gasInstanceId == (nlUint32)GAS_INVALID_ID)
      {
        nslStopSound(snd->myId);
        return;
      }

      // Else play it for real
      nslPlaySound(snd->myId);
    }
    else
    {
      // Play it normally
      snd->isReallyPlaying = true;
      nslPs2GasRpc(GAS_RPC_PLAY_INSTANCE, NULL, 
        snd->gasInstanceId, 
        (int)(left*SCE_VOLUME_MAX), 
        (int)(right*SCE_VOLUME_MAX), 0);
    }
#ifdef DEBUG_OUTPUT
    nslPrintf("PLAYING %s (NO 3d)\n", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].fileName);
#endif
  } 
  else  
  {  
    // We have a real emitter.
    // Check to see if it is currently in range
    snd->inRange = _set3dVolume(soundToPlay);
    
    // If its looping
		if (snd->looping)
		{
      // And in range
			if (snd->inRange) 
			{
        // Play it
				snd->isReallyPlaying = true;
				nslPs2GasRpc(GAS_RPC_PLAY_INSTANCE, NULL, 
					snd->gasInstanceId, 
					(int)(snd->left), 
					(int)(snd->right), 0);
#ifdef DEBUG_OUTPUT
    nslPrintf("PLAYING %s (3d looping in Range) volume %d/%d\n", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].fileName, snd->left, snd->right);
#endif
      }
      else // NOT IN RANGE, BUT LOOPING
      {
        // Stop it for now
        if (snd->gasInstanceId != (nlUint32)GAS_INVALID_ID)
          nslPs2GasRpc(GAS_RPC_STOP_INSTANCE, NULL, snd->gasInstanceId, 0, 0, 0);
        
        snd->isReallyPlaying = false;
        snd->gasInstanceId = (nlUint32)GAS_INVALID_ID;
#ifdef DEBUG_OUTPUT
    nslPrintf("NOT PLAYING %s (3d looping out of range)\n", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].fileName);
#endif
      }
		}
		else // NOT LOOPING
		{
      // This sound is in range
      if (snd->inRange)
      {
        // Is it an unadded small sound?
        if ((nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].length < NSL_SMALL3D_SOUND_SIZE) &&
            (snd->gasInstanceId == (nlUint32)GAS_INVALID_ID))
        {
          // Add it
          snd->gasInstanceId = nslPs2GasRpc(GAS_RPC_ADD_INSTANCE, "", 
            nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].gasSourceId, 
            0, 0, 0);
          // Play it if its ok
          if (snd->gasInstanceId != (nlUint32)GAS_INVALID_ID)
            nslPlaySound(snd->myId);
          else 
          {
#ifdef DEBUG_OUTPUT
            nslPrintf("NOT PLAYING %s (3d non looping  small sound in range (couldn't get instance)\n", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].fileName);
#endif
            // Release it
            nslStopSound(snd->myId);
          }
        }
        else // VALID ID AND/OR BIG SOUND
        {
          if (snd->gasInstanceId != (nlUint32)GAS_INVALID_ID)
          {
            snd->isReallyPlaying = true;
		        nslPs2GasRpc(GAS_RPC_PLAY_INSTANCE, NULL, 
			        snd->gasInstanceId, 
			        (int)(snd->left), 
			        (int)(snd->right), 0);			
          }
          else
          {
            nslFatal("Somehow we had an impossible case (NOT LOOPING, INVALID ID AND IN RANGE)\n");
          }
        }
      }
      else  // OUT OF RANGE
      {
        if (nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].length >= NSL_SMALL3D_SOUND_SIZE)
        {
          if (snd->gasInstanceId != (nlUint32)GAS_INVALID_ID)
          {
            snd->isReallyPlaying = true;
						snd->inRange = true;
	  	      nslPs2GasRpc(GAS_RPC_PLAY_INSTANCE, NULL, 
		  	      snd->gasInstanceId, 
			        (int)(snd->left), 
			        (int)(snd->right), 0);			
          }
          else
          {
            nslFatal("INVALID ID FOR BIG SOUND OUT OF RANGE");
          }
        }
        else // SMALL SOUND, OUT RANGE
        {
          nslStopSound(snd->myId);
        }
      }
    } // END IF IN RANGE
  }
}


/*-------------------------------------------------------------------------
  nsl Play Sound 3D
-------------------------------------------------------------------------*/
void nslPlaySound3D( nslSoundId soundToPlay, const nlVector3d &pos )
{
  if (!nsl.on) return; 
  ASSERT_NSL_INITIALIZED();
  // a convenience function
  ASSERT_SOUND_ID_VALID(soundToPlay);
  nslEmitterId emitId = nslCreateEmitter( pos );
  nslSetEmitterAutoRelease( emitId );
  nslSetSoundEmitter( emitId, soundToPlay );
  nslPlaySound( soundToPlay );
}


/*-------------------------------------------------------------------------
  nsl Stop Sound 
-------------------------------------------------------------------------*/
void nslStopSound( nslSoundId soundToStop )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();

  if ( nslGetSoundStatus( soundToStop ) != NSL_SOUNDSTATUS_INVALID )
  {
    if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToStop)].gasInstanceId != (nlUint32)GAS_INVALID_ID)
      nslPs2GasRpc(GAS_RPC_STOP_INSTANCE, "", nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToStop)].gasInstanceId, 0, 0, 0);
    nsl.queuedSounds.find(soundToStop, true);
// not needed?   ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToStop)].mySource);

  
    // Fix up emitters
    if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToStop)].myEmitter != NSL_GLOBAL_EMITTER_ID)
      nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToStop)].myEmitter)].emittedSounds.find(soundToStop, true);

    // clear the slots
    _nslClearSoundSlot(NSL_GET_SLOT_FROM_ID(soundToStop), false);
    nsl.soundSlotsUsedCount--;
    // Update the ID
    nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToStop)].myId += NSL_ID_INCREMENT;
  }
}


/*-------------------------------------------------------------------------
  nsl Release All Sounds
-------------------------------------------------------------------------*/
void nslReleaseAllSounds()
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  
  for (int i=0; i < NSL_NUM_SOUNDS; i++) 
  {
    if (nsl.soundSlots[i].used) 
    {
      nslStopSound(nsl.soundSlots[i].myId);
    }
  }
}


/*-------------------------------------------------------------------------
  nsl Add Sound With Offset
-------------------------------------------------------------------------*/
nslSoundId nslAddSoundWithOffset( nslSourceId soundSource, float seconds)
{
  unsigned int milliseconds = nslFTOI(seconds) * 1000;
  if (!nsl.on) return NSL_INVALID_ID;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(soundSource);
  if (seconds > nslGetSourceLength(soundSource))
  {
    // Should we fatal here?
    nslFatal("SETTING AN OFFSET PAST THE END OF THE SOUND!");
    return NSL_INVALID_ID;
  }
  for (int i=0; i < NSL_NUM_SOUNDS; i++) 
  {
    if (!nsl.soundSlots[i].used) 
    {
      _nslClearSoundSlot(i, false);
      // Get the gasInstanceID
      nslSource *src;
      nslSound *snd;
      snd = &nsl.soundSlots[i];
      src = &nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(soundSource)];

      snd->myId += NSL_ID_INCREMENT;
      snd->inRange = true;
      if (src->length >= NSL_SMALL3D_SOUND_SIZE)
      {
        snd->gasInstanceId = nslPs2GasRpc(GAS_RPC_ADD_INSTANCE_WITH_OFFSET, "", nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(soundSource)].gasSourceId, milliseconds, 0, 0);
        if (snd->gasInstanceId == (nlUint32)GAS_INVALID_ID) 
        {
          snd->inRange = false;
          return (nslSoundId)NSL_INVALID_ID;
        }
        snd->gasInstanceId =  ~GAS_INSTANCE_STEREO_FLAG_BIT & nsl.soundSlots[i].gasInstanceId;
      }
      else
      {
        snd->inRange = false;
        snd->gasInstanceId = (nlUint32)GAS_INVALID_ID;
      }
      
      snd->myEmitter = NSL_GLOBAL_EMITTER_ID;
      
      // Setup associations
      snd->mySource = soundSource;
      snd->used = true;     
      nsl.soundSlotsUsedCount++;
    
      snd->angle = 0;
    
      snd->minDist = src->minDist;
      snd->maxDist = src->maxDist;
    
      snd->positionalVolume = 1.0f;
      snd->rawVolume = src->rawVolume;

      snd->pitch = src->pitch;
      snd->old_left = 0;
      snd->old_right = 0;

//      snd->balance = src->balance;

    
      snd->pauseCount = 0;
      snd->dampenCount = 0;
    
    

      snd->isPlaying = false;
      snd->isReallyPlaying = false;
      snd->isReady = false;
      snd->isQueuing = true;
      snd->looping = src->looping;
    
    
    
  
      // We do the pitch here since it is not 
      // automatically updated on the frame
      // we don't want to send tons of pitch calls
      // unnecessarily, so we only do it as need be
      // ie whenever its different to start, and 
      // whenever we change
      if ((snd->pitch != 1.0f) && (snd->gasInstanceId != (nlUint32)GAS_INVALID_ID))
      {
        nslPs2GasRpc(GAS_RPC_INSTANCE_PITCH, "", 
          snd->gasInstanceId,
          (unsigned char)(snd->pitch*GAS_COMMAND_PITCH_ONE),
          0, 0);
      }
      // Return
			
      return snd->myId;
    }
  }
	
  return NSL_INVALID_ID;
}


/*-------------------------------------------------------------------------
  nsl Get Sound Position
-------------------------------------------------------------------------*/

float nslGetSoundPlaybackPosition ( nslSoundId whichSound )
{
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].gasInstanceId == GAS_INVALID_ID)
  {
    return nsl.sourceSlots[ NSL_GET_SOURCE_SLOT_FROM_ID( nsl.soundSlots[ NSL_GET_SLOT_FROM_ID(whichSound) ].mySource)].length;
  }

  return (float)nslPs2GasRpc(GAS_RPC_GET_SOUND_POSITION, "", nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].gasInstanceId, 0, 0, 0)/1000.0f;
}
/*-------------------------------------------------------------------------
  nsl Pause Sound 
-------------------------------------------------------------------------*/
void nslPauseSound( nslSoundId whichSound )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();

  // The new, command list style pause/unpause
  ASSERT_SOUND_ID_VALID(whichSound);
  nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount++;
}


/*-------------------------------------------------------------------------
  nsl Unpause Sound 
-------------------------------------------------------------------------*/
void nslUnpauseSound( nslSoundId whichSound )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOUND_ID_VALID(whichSound);
  
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount > 0) 
    nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount--;

}

/*-------------------------------------------------------------------------
  nsl Pause Guard Sound 
-------------------------------------------------------------------------*/
void nslPauseGuardSound( nslSoundId soundToGuard ) 
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOUND_ID_VALID(soundToGuard);
  int pc = nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToGuard)].pauseCount;
  if (!(pc <= 0))
    nslFatal("Trying to pause guard a paused sound!");
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToGuard)].pauseCount <= 0)
    nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToGuard)].pauseCount = -1;
}

  
/*-------------------------------------------------------------------------
  nsl Pause All Sounds
-------------------------------------------------------------------------*/
void nslPauseAllSounds()
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  for (int i=0; i < NSL_NUM_SOUNDS; i++)
  {
    if (nsl.soundSlots[i].used) 
    {
      nsl.soundSlots[i].pauseCount++; 
    }
  }
}


/*-------------------------------------------------------------------------
  nsl Unpause All Sounds
-------------------------------------------------------------------------*/
void nslUnpauseAllSounds()
{ 
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();

  for (int i=0; i < NSL_NUM_SOUNDS; i++)
    if (nsl.soundSlots[i].used) 
      if (nsl.soundSlots[i].pauseCount > 0)
        nsl.soundSlots[i].pauseCount--;
  
   
}



/*-------------------------------------------------------------------------
  nsl Undampen All Sounds
-------------------------------------------------------------------------*/
void nslUndampenAllSounds()
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  for (int i=0; i < NSL_NUM_SOUNDS; i++)
    if (nsl.soundSlots[i].used)
      nsl.soundSlots[i].dampenCount = 0;
}


/*-------------------------------------------------------------------------
  nsl Dampen Guard Sound 
-------------------------------------------------------------------------*/
void nslDampenGuardSound( nslSoundId soundToGuard )
{
  if (!nsl.on) return;
// not needed?   ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToGuard)].mySource);

  ASSERT_SOUND_ID_VALID(soundToGuard);
  
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToGuard)].dampenCount <= 0)
    nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToGuard)].dampenCount = -1;
}


/*-------------------------------------------------------------------------
  nsl Dampen All Sounds
-------------------------------------------------------------------------*/
void nslDampenAllSounds( float dampenLevel )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  
  nsl.dampenLevel = dampenLevel;

  for (int i=0; i < NSL_NUM_SOUNDS; i++)
    if (nsl.soundSlots[i].used)
      if (nsl.soundSlots[i].dampenCount < 1)
        nsl.soundSlots[i].dampenCount++;


}


nslSoundStatusEnum nslGetSoundStatus( nslSoundId whichSound) 
{

  if (!nsl.on) return NSL_SOUNDSTATUS_INVALID;
  ASSERT_NSL_INITIALIZED();

  // Check for error cases
// unused?   nlUint32 gasId = nsl.soundSlots[whichSound&NSL_SLOT_MASK].gasInstanceId;
// unused?   nlUint32 negativeOne = (nlUint32)-1;
  nslSoundId checkSlot = (whichSound & NSL_SLOT_MASK);
  nslSoundId upCheckId = (whichSound & NSL_ID_MASK);

  // ERROR CASES
  if (whichSound == NSL_INVALID_ID)
    return NSL_SOUNDSTATUS_INVALID;
  if (!(nsl.soundSlots[checkSlot].used))
    return NSL_SOUNDSTATUS_INVALID;
  if (!(checkSlot < NSL_NUM_SOUNDS))
    return NSL_SOUNDSTATUS_INVALID;
  if (!(whichSound == nsl.soundSlots[checkSlot].myId))
    return NSL_SOUNDSTATUS_INVALID;
  if (!(upCheckId > 0))
    return NSL_SOUNDSTATUS_INVALID;

  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount > 0)
    return NSL_SOUNDSTATUS_PAUSED;

  // PLAY CASE
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].used && 
      nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].isPlaying && 
      (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount < 1))
    return NSL_SOUNDSTATUS_PLAYING;


  // READY CASE
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].isReady)
    return NSL_SOUNDSTATUS_READY;


  // QUEUING CASE (THE DEFAULT)
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].isQueuing)
    return NSL_SOUNDSTATUS_QUEUING;

  return NSL_SOUNDSTATUS_INVALID;
}

/*-------------------------------------------------------------------------
  nsl Is Sound Playing
-------------------------------------------------------------------------*/
bool nslIsSoundPlaying( nslSoundId whichSound )
{
  return (nslGetSoundStatus( whichSound ) == NSL_SOUNDSTATUS_PLAYING);
}

/*-------------------------------------------------------------------------
  nsl Is Sound Ready
-------------------------------------------------------------------------*/
bool nslIsSoundReady( nslSoundId whichSound )
{
  return (nslGetSoundStatus( whichSound ) == NSL_SOUNDSTATUS_READY);
}

/*-------------------------------------------------------------------------
  nsl Is Sound Ready
-------------------------------------------------------------------------*/
/* Deprecated
bool nslIsSoundReady( nslSoundId whichSound )
{
  if (!nsl.on) return true;;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOUND_ID_VALID(whichSound);
  
  nslSoundId checkSlot = (whichSound & NSL_SLOT_MASK);
  nslSoundId upCheckId = (whichSound & NSL_ID_MASK);

  if ((checkSlot > NSL_NUM_SOUNDS) || 
      (whichSound != nsl.soundSlots[checkSlot].myId) ||
      (upCheckId <= 0)) return 0;

  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].isReady)
    return NSL_SOUNDSTATUS_READY;
  
}
*/
/*-------------------------------------------------------------------------
  nsl Is Sound Playing
-------------------------------------------------------------------------*/
/* Deprecated
bool nslIsSoundPlaying( nslSoundId whichSound )
{
  if (!nsl.on) return true;
  ASSERT_NSL_INITIALIZED();
  // If its in the queue, 
  if (fifo_queue_find(&nsl.queuedSounds, whichSound, false)) 
    return ((nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount < 1) &&
            nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].used);

  nslSoundId checkSlot = (whichSound & NSL_SLOT_MASK);
  nslSoundId upCheckId = (whichSound & NSL_ID_MASK);
  if ((checkSlot > NSL_NUM_SOUNDS) || 
      (whichSound != nsl.soundSlots[checkSlot].myId) ||
      (upCheckId <= 0)) return 0;

  return nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].used && 
    nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].isPlaying && 
    (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pauseCount < 1) ;
}
*/
/*-------------------------------------------------------------------------
  nsl Set Sound Param
-------------------------------------------------------------------------*/
void nslSetSoundParam( nslSoundId soundToSet, nslSoundParamEnum whichParam, 
                       float newVal )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].mySource);
  ASSERT_SOUND_ID_VALID(soundToSet);

  switch (whichParam) {
    case NSL_SOUNDPARAM_VOLUME: 
      if (newVal > 1.0f)
        newVal = 1.0f;
      nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].rawVolume=newVal; break;
    case NSL_SOUNDPARAM_PITCH:
      nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].pitch=newVal; break;
    case NSL_SOUNDPARAM_MINDIST:
      nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].minDist=newVal; break;
    case NSL_SOUNDPARAM_MAXDIST:
      nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].maxDist=newVal; break;
    default: nslFatal(false && "Unhandled sourcetype"); break;
  }
}


/*-------------------------------------------------------------------------
  nsl Get Sound Param
-------------------------------------------------------------------------*/
float	nslGetSoundParam( nslSoundId whichSound, nslSoundParamEnum whichParam )
{

  if (!nsl.on) return 1.0f;
  ASSERT_NSL_INITIALIZED();
// not needed?   ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].mySource);
  ASSERT_SOUND_ID_VALID(whichSound);
  switch (whichParam) {
    case NSL_SOUNDPARAM_VOLUME: 
      return nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].rawVolume; break;
    case NSL_SOUNDPARAM_PITCH:
      return nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].pitch; break;
    case NSL_SOUNDPARAM_MINDIST:
      return nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].minDist; break;
    case NSL_SOUNDPARAM_MAXDIST:
      return nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].maxDist; break;
    default: nslFatal(false && "Unhandled sourcetype"); break;
  }
  return 0.0f;
}


/*-------------------------------------------------------------------------
  nsl Set Sound Range
-------------------------------------------------------------------------*/
void nslSetSoundRange( nslSoundId soundToSet, float minDist, float maxDist)
{
  if (!nsl.on) return;
  // if a sound uses the global emitter, do we use these?
  ASSERT_NSL_INITIALIZED();
// not needed?    ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].mySource);

  ASSERT_SOUND_ID_VALID(soundToSet);
  nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].minDist = minDist;
  nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].maxDist = maxDist;

}

/*-------------------------------------------------------------------------
  nsl Set Sound Position
-------------------------------------------------------------------------*/
void nslSetSoundPosition( nslSoundId soundToSet, const nlVector3d &pos )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
// not needed?   ASSERT_SOURCE_ID_VALID(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].mySource);
  ASSERT_SOUND_ID_VALID(soundToSet);
  if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].myEmitter == NSL_GLOBAL_EMITTER_ID) 
  {
    nslEmitterId newEmitter = nslCreateEmitter(pos);
    nslPrintf( "Woah, setting position on a sound without an emitter?  New emitter (0x%x), sound (0x%x)\n",
               newEmitter, soundToSet );
    nslSetEmitterAutoRelease(newEmitter);
    nslSetSoundEmitter( newEmitter, soundToSet );
  }
  nslSetEmitterPosition(nsl.soundSlots[NSL_GET_SLOT_FROM_ID(soundToSet)].myEmitter, pos);
}




/////////////////////////// internal functions ////////////////////////////

// none yet.




float NSL_RIGHT_PAN_RATIO_TABLE[360] = {
	0.5000000000f, 0.5087261796f, 0.5174497366f, 0.5261679888f, 0.5348782539f, 0.5435778499f, 
	0.5522642136f, 0.5609346628f, 0.5695865750f, 0.5782172084f, 0.5868241191f, 0.5954045057f, 
	0.6039558649f, 0.6124755144f, 0.6209609509f, 0.6294095516f, 0.6378186941f, 0.6461858749f, 
	0.6545085311f, 0.6627840996f, 0.6710100770f, 0.6791839600f, 0.6873033047f, 0.6953655481f, 
	0.7033683062f, 0.7113091350f, 0.7191855907f, 0.7269952297f, 0.7347357869f, 0.7424048185f, 
	0.7500000000f, 0.7575190663f, 0.7649596334f, 0.7723194957f, 0.7795964479f, 0.7867882252f, 
	0.7938926220f, 0.8009075522f, 0.8078307509f, 0.8146601915f, 0.8213938475f, 0.8280295134f, 
	0.8345652819f, 0.8409991860f, 0.8473291993f, 0.8535534143f, 0.8596699238f, 0.8656768799f, 
	0.8715724349f, 0.8773548007f, 0.8830222487f, 0.8885729909f, 0.8940054178f, 0.8993177414f, 
	0.9045085311f, 0.9095760584f, 0.9145187736f, 0.9193353057f, 0.9240240455f, 0.9285836816f, 
	0.9330127239f, 0.9373098612f, 0.9414738417f, 0.9455032349f, 0.9493970275f, 0.9531539083f, 
	0.9567727447f, 0.9602524638f, 0.9635919333f, 0.9667902589f, 0.9698463082f, 0.9727593064f, 
	0.9755282402f, 0.9781523943f, 0.9806308746f, 0.9829629064f, 0.9851478338f, 0.9871850610f, 
	0.9890738130f, 0.9908136129f, 0.9924038649f, 0.9938441515f, 0.9951340556f, 0.9962731004f, 
	0.9972609282f, 0.9980973601f, 0.9987820387f, 0.9993147850f, 0.9996954203f, 0.9999238253f, 
	1.0000000000f, 0.9944444299f, 0.9888888597f, 0.9833333492f, 0.9777777791f, 0.9722222090f, 
	0.9666666389f, 0.9611111283f, 0.9555555582f, 0.9499999881f, 0.9444444180f, 0.9388889074f, 
	0.9333333373f, 0.9277777672f, 0.9222221971f, 0.9166666865f, 0.9111111164f, 0.9055555463f, 
	0.8999999762f, 0.8944444656f, 0.8888888955f, 0.8833333254f, 0.8777777553f, 0.8722222447f, 
	0.8666666746f, 0.8611111045f, 0.8555555344f, 0.8500000238f, 0.8444444537f, 0.8388888836f, 
	0.8333333135f, 0.8277778029f, 0.8222222328f, 0.8166666627f, 0.8111110926f, 0.8055555820f, 
	0.8000000119f, 0.7944444418f, 0.7888888717f, 0.7833333611f, 0.7777777910f, 0.7722222209f, 
	0.7666666508f, 0.7611111403f, 0.7555555701f, 0.7500000000f, 0.7444444299f, 0.7388888597f, 
	0.7333333492f, 0.7277777791f, 0.7222222090f, 0.7166666389f, 0.7111111283f, 0.7055555582f, 
	0.6999999881f, 0.6944444180f, 0.6888889074f, 0.6833333373f, 0.6777777672f, 0.6722221971f, 
	0.6666666865f, 0.6611111164f, 0.6555555463f, 0.6499999762f, 0.6444444656f, 0.6388888955f, 
	0.6333333254f, 0.6277777553f, 0.6222222447f, 0.6166666746f, 0.6111111045f, 0.6055555344f, 
	0.6000000238f, 0.5944444537f, 0.5888888836f, 0.5833333135f, 0.5777778029f, 0.5722222328f, 
	0.5666666627f, 0.5611110926f, 0.5555555820f, 0.5500000119f, 0.5444444418f, 0.5388888717f, 
	0.5333333611f, 0.5277777910f, 0.5222222209f, 0.5166666508f, 0.5111111403f, 0.5055555701f, 
	0.5000000000f, 0.4944444299f, 0.4888888896f, 0.4833333194f, 0.4777777791f, 0.4722222090f, 
	0.4666666687f, 0.4611110985f, 0.4555555582f, 0.4499999881f, 0.4444444478f, 0.4388888776f, 
	0.4333333373f, 0.4277777672f, 0.4222222269f, 0.4166666567f, 0.4111111164f, 0.4055555463f, 
	0.4000000060f, 0.3944444358f, 0.3888888955f, 0.3833333254f, 0.3777777851f, 0.3722222149f, 
	0.3666666746f, 0.3611111045f, 0.3555555642f, 0.3499999940f, 0.3444444537f, 0.3388888836f, 
	0.3333333433f, 0.3277777731f, 0.3222222328f, 0.3166666627f, 0.3111111224f, 0.3055555522f, 
	0.3000000119f, 0.2944444418f, 0.2888889015f, 0.2833333313f, 0.2777777910f, 0.2722222209f, 
	0.2666666806f, 0.2611111104f, 0.2555555701f, 0.2500000000f, 0.2444444448f, 0.2388888896f, 
	0.2333333343f, 0.2277777791f, 0.2222222239f, 0.2166666687f, 0.2111111134f, 0.2055555582f, 
	0.2000000030f, 0.1944444478f, 0.1888888925f, 0.1833333373f, 0.1777777821f, 0.1722222269f, 
	0.1666666716f, 0.1611111164f, 0.1555555612f, 0.1500000060f, 0.1444444507f, 0.1388888955f, 
	0.1333333403f, 0.1277777851f, 0.1222222224f, 0.1166666672f, 0.1111111119f, 0.1055555567f, 
	0.1000000015f, 0.0944444463f, 0.0888888910f, 0.0833333358f, 0.0777777806f, 0.0722222254f, 
	0.0666666701f, 0.0611111112f, 0.0555555560f, 0.0500000007f, 0.0444444455f, 0.0388888903f, 
	0.0333333351f, 0.0277777780f, 0.0222222228f, 0.0166666675f, 0.0111111114f, 0.0055555557f, 
	0.0000000000f, 0.0000761517f, 0.0003045916f, 0.0006852376f, 0.0012179781f, 0.0019026507f, 
	0.0027390718f, 0.0037269408f, 0.0048659779f, 0.0061558355f, 0.0075961216f, 0.0091864420f, 
	0.0109262262f, 0.0128149856f, 0.0148521438f, 0.0170370825f, 0.0193691980f, 0.0218476560f, 
	0.0244717635f, 0.0272407196f, 0.0301536806f, 0.0332098454f, 0.0364081152f, 0.0397475958f, 
	0.0432272777f, 0.0468460917f, 0.0506030433f, 0.0544967838f, 0.0585262291f, 0.0626901463f, 
	0.0669872761f, 0.0714164227f, 0.0759759992f, 0.0806647390f, 0.0854812115f, 0.0904240832f, 
	0.0954915807f, 0.1006822959f, 0.1059946418f, 0.1114270091f, 0.1169778928f, 0.1226452887f, 
	0.1284276396f, 0.1343231648f, 0.1403300911f, 0.1464467198f, 0.1526709050f, 0.1590008736f, 
	0.1654347032f, 0.1719704568f, 0.1786063164f, 0.1853398830f, 0.1921693087f, 0.1990924925f, 
	0.2061075270f, 0.2132119089f, 0.2204036266f, 0.2276805192f, 0.2350403666f, 0.2424811274f, 
	0.2500001192f, 0.2575952709f, 0.2652642429f, 0.2730047405f, 0.2808145881f, 0.2886909842f, 
	0.2966317534f, 0.3046344519f, 0.3126966953f, 0.3208161891f, 0.3289900422f, 0.3372159898f, 
	0.3454915285f, 0.3538141251f, 0.3621814847f, 0.3705905974f, 0.3790391088f, 0.3875244856f, 
	0.3960441351f, 0.4045956433f, 0.4131760001f, 0.4217828214f, 0.4304134548f, 0.4390655160f, 
	0.4477359056f, 0.4564222097f, 0.4651218057f, 0.4738320112f, 0.4825504422f, 0.4912739396f };



// if a sound is less than this, then it 
#define NEGATIVE_THRESHHOLD 0

/********************************************************************************
 * bool _set3dVolume(nslSoundId whichSound)
 * sets the left and right values of the sound indicated by 
 * which sound.  
 * it returns true if the sound should really be played
 * false otherwise
 ********************************************************************************/

bool _set3dVolume(nslSoundId whichSound)
{

  int back;
  short new_left, new_right;
  unsigned short vol = 0;
  float angle = 0;
// unused?   float old_angle = nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)].angle;
  float left_table, right_table;
  float volumeModifier = 1.0f;
  float xzDistanceSq   = 10000.0f;

  // Cache the sound pointer
  nslSound *snd;
  snd = &nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)];
  
  // If its not really playing.. then return false
  if (!_setSoundPosVolume(whichSound,  &xzDistanceSq)) 
  {
    // Out of range.. 
    snd->positionalVolume = 0;
    snd->left = 0;
    snd->right = 0;
    return false;
  }
  _setSoundAngle(whichSound);
  float checkValue = snd->minDist * 0.5f;

  // if we get REALLY close to the microphone (50% of min distance), 
  // start drifting towards center, rather than being on either side.
  if (xzDistanceSq < checkValue && checkValue > 0.0f)
  {
    float closenessValue = xzDistanceSq / checkValue;


	// new new new!  Exciting hackey hack to keep sounds close to the 
	// microphone to make things happy and nice.
	if (snd->angle <= M_PI) // right side
	{
      
      snd->angle = ((snd->angle) * (closenessValue));                 
	}
	else					// left side
	{
      
      snd->angle = ((M_PI * 2.0f) * (1.0f - (closenessValue))) + 
                   ((snd->angle) * (closenessValue));
      if (snd->angle >= M_PI * 1.9f)
        snd->angle = 0.0f;
	}

  }

  switch (nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(snd->mySource)].type) 
  {
    case NSL_SOURCETYPE_SFX:     volumeModifier = nsl.sfxVolume;     break;
    case NSL_SOURCETYPE_AMBIENT: volumeModifier = nsl.ambientVolume; break;
    case NSL_SOURCETYPE_MUSIC:   volumeModifier = nsl.musicVolume;   break;
    case NSL_SOURCETYPE_VOICE:   volumeModifier = nsl.voiceVolume;   break;
    case NSL_SOURCETYPE_MOVIE:   volumeModifier = nsl.movieVolume;   break;
    case NSL_SOURCETYPE_USER1:   volumeModifier = nsl.user1Volume;   break;
    case NSL_SOURCETYPE_USER2:   volumeModifier = nsl.user2Volume;   break;
    default: nslFatal("DOH2"); break;
  }
  
  int damp = (int)(snd->dampenCount > 0?nsl.dampenLevel:1);

  // We ignore balance for these.. 
  // let 3d figure it out

  vol = (unsigned short)(SCE_VOLUME_MAX*
    nsl.masterVolume* damp *
    snd->positionalVolume*
    snd->rawVolume*
    volumeModifier);
  if (nsl.speakerMode != NSL_SPEAKER_MONO) 
  {
    angle = 180.0f*snd->angle/M_PI;
  
    if (angle == (int)angle) {
      left_table = (1.0f - NSL_RIGHT_PAN_RATIO_TABLE[(int)angle]);
      right_table = NSL_RIGHT_PAN_RATIO_TABLE[(int)angle];
    } else {
      int iangle = (int)angle, other_angle;
      float  delta; 
      delta = angle - iangle;
      other_angle = iangle + 1;
    
      // Interpolate
      left_table =  delta*((1.0f - NSL_RIGHT_PAN_RATIO_TABLE[other_angle]) - 
                          (1.0f - NSL_RIGHT_PAN_RATIO_TABLE[iangle])) +
                    (1 - NSL_RIGHT_PAN_RATIO_TABLE[iangle]);

      right_table = delta*(NSL_RIGHT_PAN_RATIO_TABLE[other_angle] - NSL_RIGHT_PAN_RATIO_TABLE[iangle]) +
                    NSL_RIGHT_PAN_RATIO_TABLE[iangle];


    }

    // Do a little damping for rear sounds
    if ((angle > 80) && (angle < 100)) 
    {
      vol = (unsigned short)((float)vol * (-1.0f * ((angle - 80.0f)/20.0f)*0.1f + 1.0f));
    } 
    else if ((angle >= 100) && (angle <=260))
    {
      vol = (unsigned short)((float)vol * 0.9f);
    }
    else if ((angle >260) && (angle <280))
    {
      vol = (unsigned short)((float)vol * (((angle - 260.0f)/20.0f)*0.1f + 0.9f));
    }
    
  
    if (!(vol <= SCE_VOLUME_MAX))
      nslFatal("Bad sound volume: %d\n", vol);
    if (!(angle >= 0))
      nslFatal("Bad angle! (<0)\n");

    new_left = (short)(vol * left_table);
    new_right = (short)(vol * right_table);
    
    if ((nsl.speakerMode == NSL_SPEAKER_PROLOGIC) ||  (nsl.speakerMode == NSL_SPEAKER_DOLBY_51))
    {
      while (angle > 360) 
        angle-=360;

      // Is it behind us?
      if (angle>90 && angle<270) 
        back = 1;
      else 
        back = 0;

      if(back)
      {
        if((snd->old_left <= 0 && snd->old_right <= 0) || 
           (snd->old_left > 0 && snd->old_right > 0))
        {
          int which_sign = snd->old_left <= 0 ? -1 : 1;
          // left and right are currently in phase, reverse the smallest...
          if(abs(new_left) <= abs(new_right))
          {
            new_left = -which_sign * new_left;
            new_right = which_sign * new_right;
          }
          else
          {
            new_left = which_sign * new_left;
            new_right = -which_sign * new_right;
          }
        }
        else
        {
          // channels are already out of phase, so match...
          if (snd->old_right <= 0)
          {
            new_right  = -new_right;
          }
          else // if (snd->old_left <= 0)
          {
            new_left = -new_left;
          }
        }
      }
      else
	    {
		    if (snd->old_left <= 0)
		    {
			    if ((abs(snd->old_left) > abs(snd->old_right)) || (snd->old_right <= 0))
			    {
				    new_left  = -new_left;
				    new_right = -new_right;
			    }
		    }
		    else if (snd->old_right <= 0) 
		    {
			    if ((abs(snd->old_left) < abs(snd->old_right)) || (snd->old_left <= 0))
			    {
				    new_left  = -new_left;
				    new_right = -new_right;				
			    }
		    }
	    }
    }
 

    snd->old_right  = new_right;
    snd->old_left   = new_left;


    // Set the sounds 
    snd->left       = new_left;
    snd->right      = new_right;
  } 
  else 
  {
    snd->left = snd->right = vol;
  }


  return true;
}

// Calculates the positional volume based on sound ranges
bool _setSoundPosVolume( nslSoundId whichSound, float *xzDistanceSq ) 
{
  // Calc the distance
  // No sqaure roots here...
  // calc volume based on SQ

  nslEmitter *e;
  nslSound *snd;

  snd = &nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)];
  e = &nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(snd->myEmitter)];

  if (e->isALineEmitter)
  {
    nlVector3d listenerPos;
    listenerPos[0] = nsl.listenerPo[0][3];
    listenerPos[1] = nsl.listenerPo[1][3];
    listenerPos[2] = nsl.listenerPo[2][3];
    nlDistancePointSegment( listenerPos, e->startPosition, e->endPosition, e->position );
  }

  float xDist = e->position[0] - nsl.listenerPo[0][3];
  float yDist = e->position[1] - nsl.listenerPo[1][3];
  float zDist = e->position[2] - nsl.listenerPo[2][3];
  float distanceSQ = xDist*xDist + zDist*zDist;
  if (xzDistanceSq)
    *xzDistanceSq = distanceSQ;
  distanceSQ += yDist*yDist;
  // Copy values for ease
  if ( snd->minDist < 0.0f)
    nslFatal ("These should never go negative" );
  if ( snd->maxDist < 0.0f)
    nslFatal ("These should never go negative" );
/*
  if ( snd->minDist < 0.0f || snd->maxDist < 0.0f)
  {
    nslFatal("FOOBARED");
  }
*/
  float minDistSQ = snd->minDist * snd->minDist;
  float maxDistSQ = snd->maxDist * snd->maxDist;

  // Check to see if the sound is in range or not.  
  if (distanceSQ > maxDistSQ)
  {
    snd->positionalVolume = 0.0f;
    return false;
  } 
  else if (distanceSQ < minDistSQ) 
  {
    snd->positionalVolume = 1.0f;
    return true;
  } 
  else 
  {
    snd->positionalVolume = (maxDistSQ - distanceSQ)/(maxDistSQ-minDistSQ);
    return true;
  }
}



// PROBABLY SLOW!!!!
void _setSoundAngle( nslSoundId whichSound ) 
{
  // We need to calculate the angle
  nlVector3d forward,toSound, up, newUp;
  
  nslSound *snd = &nsl.soundSlots[NSL_GET_SLOT_FROM_ID(whichSound)];

  // Setup a few Vectors
  forward[0] = forward[1] = 0; forward[2] = -1;
  up[0] = up[2] = 0; up[1] = 1; 

  toSound[0] = nsl.listenerPo[0][3] - nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(snd->myEmitter)].position[0];
  toSound[1] = nsl.listenerPo[1][3] - nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(snd->myEmitter)].position[1];
  toSound[2] = nsl.listenerPo[2][3] - nsl.emitterSlots[NSL_GET_SLOT_FROM_ID(snd->myEmitter)].position[2];



  // Normalize toSound
  // A SQUARE ROOT!
  float length = sqrtf(toSound[0]*toSound[0] + toSound[2]*toSound[2]);

  // Nothin too small...
  if (length < .0001) 
  {
	  // bail out early if the toSound is directly up.
	  snd->angle = 0.0;
	  return;
  }
  toSound[0] /= length; 
  toSound[1] = 0.0f;
  toSound[2] /= length; 

  // Calc the new forward
  nlTransformVector(forward, nsl.listenerPo, forward);

  // Normalize Forward
  // A SQUARE ROOT!
  length = sqrtf(forward[0]*forward[0] + forward[2]*forward[2]);
  forward[0]/=length; 
  forward[1] = 0.0f;
  forward[2]/=length; 

  // Get the cos(theta)
  float cosTheta = nlDotProduct3d(toSound, forward);

  if (cosTheta >= .999) {
    snd->angle = 0;
  } 
  else if (cosTheta <= -.999) 
  {
    snd->angle = M_PI;
  } 
  else 
  {
    // We find the angle.. 
    // Not my code, really..

    nlCrossProduct3d(newUp, toSound, forward);

    // A SQUARE ROOT!
    length = sqrtf(newUp[0]*newUp[0] + newUp[1]*newUp[1] + newUp[2]*newUp[2]);
    newUp[0]/=length; newUp[1]/=length; newUp[2]/=length;


    nlTransformVector(up, nsl.listenerPo, up);

    // A SQUARE ROOT!
    length = sqrtf(up[0]*up[0] + up[1]*up[1] + up[2]*up[2]);
    up[0]/=length; up[1]/=length; up[2]/=length;

    float cosPhi = nlDotProduct3d( up, newUp );
    snd->angle = acosf(cosTheta);
    if (cosPhi >= 0.0) snd->angle = -snd->angle;

    while (snd->angle < 0.0f) 
      snd->angle += (M_PI*2.0f);
  }


}