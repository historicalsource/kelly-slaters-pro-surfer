#include "common/nsl.h"
#include "ps2/nsl_ps2.h"
#include <sifdev.h>
#include <libcdvd.h>
#include "ps2/gas.h"
#include "ps2/gas_utility.h"
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdio.h>
// This no longer is needed
//#include <sntty.h>


// the nsl system data struct
nslSystem nsl;
struct nslSystemCallbackStruct nslSystemCallbacks;
char *nslHostPrefix = "host0:";

#ifdef _DEBUGNSL_
bool NSLasserted = false;
#endif


///////////////////////// nsl system api functions ////////////////////////

/*-------------------------------------------------------------------------
  nsl Init
-------------------------------------------------------------------------*/

static char modDir[64]="";
static char gasName[64]="GAS.IRX";
void nslPreInitSetModDirPS2(const char *directory)
{
	strcpy(modDir,directory);
}

void nslPreInitSetGasNamePS2(const char *name)
{
	strcpy(gasName,name);
}

bool nslInit(  )
{
// unused?   int fileID, fileSize;
// unused?   char fname[256];
// unused?   int counter =0;
  if (nsl.initialized) return true;

  sceCdSync(0);
  // Clear everything out
  if (nsl.firstInit)
  {
	char filename[256];

	strcpy(filename,modDir); 
	strcat(filename,"LIBSD.IRX");
    if (!_nslLoadModule(filename)) 
		return false;
	if (!_nslLoadModule("SDRDRV.IRX")) 
		return false;
	strcpy(filename,modDir); 
	strcat(filename,gasName);
    if (!_nslLoadModule(filename)) 
		return false;
  
	nsl.firstInit = 0;
  }
  nsl.on = false;
  nsl.initialized = 1;
  // Setup RPC
  nslPs2GasRpcInit();


  // Look up the file size..
  nslPs2GasRpc(GAS_RPC_INIT, "", 0, (nsl.proview?1:0), (int)nsl.cdDvdMode, 0);
  fflush(stdout);
  _nslReleaseAllSources();
  _nslResetInternal( NSL_CLEAR_INIT );

  nsl.numBanks = 0;
  // override finalize sources option for GAS buffer temp loading (for spider-man)
  nsl.finalizeSourcesEnabled = true;
  nsl.rootDir[0] = '\0';

  nsl.speakerMode = NSL_SPEAKER_STEREO;
	nsl.outputMode = NSL_OUTPUT_BOTH;
  nsl.language = NSL_LANGUAGE_ENGLISH;

  nsl.sfxVolume     = 1.0f;
	nsl.ambientVolume = 1.0f;
	nsl.musicVolume   = 1.0f;
	nsl.voiceVolume   = 1.0f;
  nsl.masterVolume  = 1.0f;
  nsl.movieVolume   = 1.0f;
  nsl.user1Volume   = 1.0f;
  nsl.user2Volume   = 1.0f;

  return true;
}

bool nslIsOn()
{
  return nsl.on;
}

void nslToggleOnOff()
{
  nsl.on = !nsl.on;
}

void nslEnableFinalizeSourcesPS2( bool enable )
{
  nsl.finalizeSourcesEnabled = enable;
}


/*-------------------------------------------------------------------------
  nsl Set Root Dir
-------------------------------------------------------------------------*/
void nslSetRootDir( const char *rootDir )
{
  if (rootDir)
    strcpy(nsl.rootDir, rootDir);
  else
    strcpy(nsl.rootDir, "");
  if (nsl.rootDir[strlen(nsl.rootDir) - 1] == '\\')
    nsl.rootDir[strlen(nsl.rootDir) - 1] = '\0';

}

/*-------------------------------------------------------------------------
  nsl Reset
-------------------------------------------------------------------------*/
bool nslReset( const char *soundListfile, nslLanguageEnum language )
{
  char realSoundListFile[256];
  char toGas[256];
  

  ASSERT_NSL_INITIALIZED();
  if (!nsl.initialized) return false;
  //if (!nsl.on) return false;
  // only change the language if they explicitly specify it, else keep the old one
  if (language != NSL_LANGUAGE_Z)
  {
    nsl.language = language;
  }

  
  char *pos = strchr(soundListfile, '\\');
  
  if (pos == NULL)
    strcpy(realSoundListFile, "LEVELS\\");      
  else 
    strcpy(realSoundListFile, "" );
  
  strcat(realSoundListFile, soundListfile);
  
  if (stricmp(".SND", soundListfile + strlen(soundListfile) - 4) != 0)
    strcat(realSoundListFile, ".SND");

  strcpy(toGas, nsl.rootDir);
  strcat(toGas, "\\");
  strcat(toGas, nslLanguageStr[nsl.language]);
  strcat(toGas, "\\");
  strcat(toGas, realSoundListFile);
  

  
  fflush(stdout);
  nslReleaseAllSounds();
  _nslReleaseAllSources();
  _nslResetInternal( NSL_CLEAR_RESET );

  //strcat(fname, ";1");
// unused?   nslFileBuf fp;
/*
  while ((!nslReadFile(fname, &fp, 1)) && (counter < 5))  counter++;
  if (counter == 5) 
  {
    strcpy(fname, "host0:");
    strcat(fname, soundListfile);
    if (!nslReadFile(fname, &fp, 1))
    {
      nsl.on = false;
      return;
    }
  }
    
  // Here we parse the input file
  _nslLoadSourceData(fp);

  
  nslReleaseFile(&fp);
*/

  while(nslPopBank());
  nslPs2GasRpc(GAS_RPC_SET_MASTER_VOLUME, "", 0, 0, 0, 0 );

  //Load it up
  nslPs2GasRpc(GAS_RPC_SET_ROOT_DIR, nsl.rootDir, 0, 0, 0, 0);
  nslPs2GasRpc(GAS_RPC_RESET, "", 0,0, 0, 0);
  if (nslPs2GasRpc(GAS_RPC_LOAD_SND_LIST, toGas, 0, 0, 0, 0) == 0)
  {
		nsl.on = false;
    return false;
  }
  if ( nsl.finalizeSourcesEnabled )
    nslPs2GasRpc(GAS_RPC_FINALISE_SRCS, "", 0, 0, 0, 0);
  for (int i=0; i < NSL_NUM_SOUNDS; i++) 
    nsl.soundSlots[i].myId = i;
  for (int i=0; i < NSL_NUM_SOURCES; i++) 
    nsl.sourceSlots[i].myId = i;
  for (int i=0; i < NSL_NUM_EMITTERS; i++) 
    nsl.emitterSlots[i].myId = i;
	nslPs2GasRpc(GAS_RPC_SET_MASTER_VOLUME, "", SCE_VOLUME_MAX, 0, 0, 0 );
  nsl.numBanks++;
	nsl.on = true;
	return true;
}	


/*-------------------------------------------------------------------------
  nsl Reset
-------------------------------------------------------------------------*/
void nslReset( void )
{
  char realSoundListFile[256];
  char toGas[256];
  

  ASSERT_NSL_INITIALIZED();
  if (!nsl.initialized) return;
  
  // only change the language if they explicitly specify it, else keep the old one

  
  fflush(stdout);
  nslReleaseAllSounds();
  _nslReleaseAllSources();
  _nslResetInternal( NSL_CLEAR_RESET );

  //strcat(fname, ";1");
// unused?   nslFileBuf fp;
/*
  while ((!nslReadFile(fname, &fp, 1)) && (counter < 5))  counter++;
  if (counter == 5) 
  {
    strcpy(fname, "host0:");
    strcat(fname, soundListfile);
    if (!nslReadFile(fname, &fp, 1))
    {
      nsl.on = false;
      return;
    }
  }
    
  // Here we parse the input file
  _nslLoadSourceData(fp);

  
  nslReleaseFile(&fp);
*/

  while(nslPopBank());
  nslPs2GasRpc(GAS_RPC_SET_MASTER_VOLUME, "", SCE_VOLUME_MAX, 0, 0, 0 );

  //Load it up
  nslPs2GasRpc(GAS_RPC_SET_ROOT_DIR, nsl.rootDir, 0, 0, 0, 0);
  nslPs2GasRpc(GAS_RPC_RESET, "", 0,0, 0, 0);
	nsl.on =false;
}

/*-------------------------------------------------------------------------
  nsl Push Bank
-------------------------------------------------------------------------*/
bool nslPushBank( const char *soundListfile, nslLanguageEnum language = NSL_LANGUAGE_Z )
{

  char realSoundListFile[256];
  char toGas[256];
  bool retVal = false;

  ASSERT_NSL_INITIALIZED();
  //if (!nsl.on) return false;
  
  // only change the language if they explicitly specify it, else keep the old one
  if (language != NSL_LANGUAGE_Z)
  {
    nsl.language = language;
  }

  
  char *pos = strchr(soundListfile, '\\');
  
  if (pos == NULL)
    strcpy(realSoundListFile, "LEVELS\\");      
  else 
    strcpy(realSoundListFile, "" );
  
  strcat(realSoundListFile, soundListfile);
  
  if (stricmp(".SND", soundListfile + strlen(soundListfile) - 4) != 0)
    strcat(realSoundListFile, ".SND");

  strcpy(toGas, nsl.rootDir);
  strcat(toGas, "\\");
  strcat(toGas, nslLanguageStr[nsl.language]);
  strcat(toGas, "\\");
  strcat(toGas, realSoundListFile);

  retVal = nslPs2GasRpc(GAS_RPC_PUSH_BANK, toGas, 0,0,0,0);
  
  if (retVal)
    nsl.numBanks++;
  else nslError("Could not load second bank");
	
	if (nsl.numBanks > 0) nsl.on = true;
	else nsl.on = false;
  return retVal;
}

/*-------------------------------------------------------------------------
  nsl Pop Bank
-------------------------------------------------------------------------*/
bool nslPopBank( )
{
  if (nsl.numBanks == 0)
    return false;
  int src;
  for (int i=0; i < NSL_NUM_SOUNDS; i++)
  {
    src = NSL_GET_SOURCE_SLOT_FROM_ID(nsl.soundSlots[i].mySource);
    if (nsl.sourceSlots[src].used && nsl.soundSlots[i].used && (nsl.sourceSlots[src].bank == nsl.numBanks - 1))
    {
      nslStopSound(nsl.soundSlots[i].myId); 
    }
  }

  for (int i=0; i < NSL_NUM_SOURCES; i++)
  {
    if (nsl.sourceSlots[i].used && nsl.sourceSlots[i].bank == nsl.numBanks - 1)
    {
      _nslClearSourceSlot(i, false); 
    }
  }

	
  

  bool retVal = nslPs2GasRpc(GAS_RPC_POP_BANK, 0, 0,0,0,0);
	
	nsl.numBanks--;
	if (nsl.numBanks == 0 )
		nsl.on = false;
	else
		nsl.on = true;


  return retVal;
}
/*-------------------------------------------------------------------------
  nsl Shutdown
-------------------------------------------------------------------------*/
void nslShutdown()
{
  //if (!nsl.on) return;
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  nslReleaseAllSounds();
  _nslClearSystemData( NSL_CLEAR_FREE );
  nslPs2GasRpc(GAS_RPC_SHUTDOWN, "", 0, 0, 0, 0);
	nsl.on = false;
	nsl.initialized = false;
}

extern GasStatus g_RPC_status;
int _nslAssembleCommandListAndCheck3dSounds(nslSound *theSound, void *commandData);
void _nslCheckAndMaybePlay3dSound(nslSound *theSound);
int _nslCheckAutoReleaseEmitters(nslEmitter *daEmmiter, void *userData);

/*-------------------------------------------------------------------------
  nsl Frame Advance
-------------------------------------------------------------------------*/
void nslFrameAdvance( float timeElapsed )
{ 
  if (!nsl.initialized) return;
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();

  // Things we need to do... 
  // *Update volumes for changed listener positions &
  //  Set volumes on sounds based on changes in the
  //  sfx/music/ambient/voice volumes   - CHECK
  // *Set pitches ? 
  // *auto release emitters

  GasCommandEntry commandList[GAS_MAX_INSTANCES];  
  for (int i=0; i < GAS_MAX_INSTANCES; i++)
  {
    commandList[i].set_volume = 0;
    commandList[i].set_paused = 0;
    commandList[i].set_pitch  = 0;
    commandList[i].instance_id = (unsigned int)-1;
    commandList[i].pitch          = 0x5AFE;
    commandList[i].volume_left    = 0x5AFE;
    commandList[i].volume_right   = 0x5AFE;
  }

  _nslSoundForEach(_nslAssembleCommandListAndCheck3dSounds, commandList);


  // Send off all the volume and pause adjustments
  nslPs2GasRpc(GAS_RPC_COMMAND_LIST, (char *)commandList, 0, 0, 0, 0);

  // Now we check the queued sounds to see if they 
  // are ready to be played
  int size = nsl.queuedSounds.size();
  for (int k=0; k < size; k++) 
  {
    // Grab the first queued sound
    nslSoundId s = nsl.queuedSounds.pop();
    // Is it still valid
    if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(s)].used) 
    {
      // is it ready
      if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(s)].isReallyReady)
      {
        // Is it paused
        if (nsl.soundSlots[NSL_GET_SLOT_FROM_ID(s)].pauseCount < 1)
          nslPlaySound(s);     
        else // paused, put it back
        {
#ifdef DEBUG_OUTPUT
    nslSound *snd = &nsl.soundSlots[NSL_GET_SLOT_FROM_ID(s)];
    nslPrintf("PUSHING %s back 'cause it's paused\n", nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(snd->mySource)].fileName);
#endif
           nsl.queuedSounds.push(s);
        }
      }  
      else // not ready, put it back
      {
#ifdef DEBUG_OUTPUT
    nslSound *snd = &nsl.soundSlots[NSL_GET_SLOT_FROM_ID(s)];
    nslPrintf("PUSHING %s for the nth time (not ready)\n", nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(snd->mySource)].fileName);
#endif
        nsl.queuedSounds.push(s);
      }
    }
  }

  _nslEmitterForEach(_nslCheckAutoReleaseEmitters, NULL);

  // DONE!
}


int _nslCheckAutoReleaseEmitters(nslEmitter *daEmmiter, void *userData)
{
  
  // Now we check to see if:
  // 1.  An Emitter should be auto-released
  // 2.  Any sounds in the emitter should 
  //      be thrown out of the list
  if (daEmmiter->used) 
  {
    // Check auto release
    int size = daEmmiter->emittedSounds.size();
#ifdef DEBUG
    // check emitters for consistency
    short it;
    if (daEmmiter->emittedSounds.iterator_reset(&it) > 0)
    {
      nslSoundId *currSound = NULL;
      do
      {
        currSound = daEmmiter->emittedSounds.iterate(&it);
        if (currSound != NULL)
        {
          ASSERT_SOUND_ID_VALID((*currSound));
        }
      } while (currSound != NULL);
    }
#endif

    if ( daEmmiter->autoRelease && size == 0 )
    {
      nslReleaseEmitter(daEmmiter->myId);
    }
  }
  return 0;
}


int _nslDebugCheckSoundSlotNotInAnyEmitters(nslEmitter *daEmmiter, void *userData)
{
  if (daEmmiter->used) 
  {
    if (userData == NULL)
    {
      nslFatal("This is a dumbass programmer at the helm.\n");
      return 0;
    }
    nslSoundId checkId = *(nslSoundId *)userData;

    // check emitters for consistency
    short it;
    if (daEmmiter->emittedSounds.iterator_reset(&it) > 0)
    {
      nslSoundId *currSound = NULL;
      do
      {
        currSound = daEmmiter->emittedSounds.iterate(&it);
        if (currSound != NULL)
        {
          if (NSL_GET_SLOT_FROM_ID((*currSound)) == NSL_GET_SLOT_FROM_ID(checkId))
          {
            // bad
            nslFatal("Houston, we have a problem!  Slot in use\n");
          }
        }
      } while (currSound != NULL);
    }
  }
  return 0;
}


int _nslAssembleCommandListAndCheck3dSounds(nslSound *theSound, void *commandData)
{
  // This procedure assembles the command list and checks 3d sounds
  // to see if they have entered/left 
  GasCommandEntry *currentCommandListEntry = NULL;

  if ((theSound->used)  && (theSound->isPlaying))
  {
    

    if (theSound->myEmitter == NSL_GLOBAL_EMITTER_ID) 
    {
      float typeVol = 1;

      // Don't touch bad id's
      if (theSound->gasInstanceId != (nlUint32)GAS_INVALID_ID) 
      {
         currentCommandListEntry = &(((GasCommandEntry *)commandData)[theSound->gasInstanceId&GAS_INSTANCE_ID_MASK]);
        // Activate this one
        currentCommandListEntry->instance_id = theSound->gasInstanceId;
   
        // Find out what kind of sound this is
        switch (nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].type) 
        {
          case NSL_SOURCETYPE_SFX:     typeVol = nsl.sfxVolume;     break;
          case NSL_SOURCETYPE_AMBIENT: typeVol = nsl.ambientVolume; break;
          case NSL_SOURCETYPE_MUSIC:   typeVol = nsl.musicVolume;   break;
          case NSL_SOURCETYPE_VOICE:   typeVol = nsl.voiceVolume;   break;
          case NSL_SOURCETYPE_MOVIE:   typeVol = nsl.movieVolume;   break;
          case NSL_SOURCETYPE_USER1:   typeVol = nsl.user1Volume;   break;
          case NSL_SOURCETYPE_USER2:   typeVol = nsl.user2Volume;   break;
          default: nslFatal("DOH"); break;
        }

        // Yes, we're setting the volume
        float damp = (theSound->dampenCount > 0?nsl.dampenLevel:1);
        // Calc left and right
        float left = nsl.masterVolume*
          theSound->rawVolume*
					nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(theSound->mySource)].rawVolume*
          typeVol * damp;
//         * (1-theSound->balance)/2;

        float right = nsl.masterVolume*
          theSound->rawVolume*
					nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(theSound->mySource)].rawVolume*
          typeVol * damp; 
//        * (1+theSound->balance)/2;


        // Dampen via master balance.. 
/* balance is deprecated.
        if (nsl.balance > 0) 
          left = left - (nsl.balance *left);
        else 
          right = right + (nsl.balance*right);
*/
        if (nsl.speakerMode == NSL_SPEAKER_MONO)
          right = left;

        //Set it
        currentCommandListEntry->set_volume = 1;

        currentCommandListEntry->volume_left = 
          (unsigned short)(SCE_VOLUME_MAX *     // Base volume
          left);
        currentCommandListEntry->volume_right =   // Same as above
          (unsigned short)(SCE_VOLUME_MAX *
          right);
      } // end of   if (nsl.soundSlots[i].gasInstanceId != (nlUint32)GAS_INVALID_ID)
    } // end of       if (nsl.soundSlots[i].myEmitter == NSL_GLOBAL_EMITTER_ID) 
    else   // Not the GLOBAL_EMITTER_ID, ie a positional sound
    {
      _nslCheckAndMaybePlay3dSound(theSound);
      if (theSound->gasInstanceId != (nlUint32)GAS_INVALID_ID)
      {
        currentCommandListEntry = &(((GasCommandEntry *)commandData)[theSound->gasInstanceId&GAS_INSTANCE_ID_MASK]);
        // Activate this one
				
        currentCommandListEntry->instance_id = theSound->gasInstanceId;

        currentCommandListEntry->set_volume = 1;
        currentCommandListEntry->volume_left = theSound->left;
        currentCommandListEntry->volume_right =   theSound->right;
      }
    }

    if ((currentCommandListEntry == NULL) && (theSound->gasInstanceId != (unsigned int)GAS_INVALID_ID))
    {
      // INsanity checks
      if (theSound->looping && !theSound->inRange && theSound->gasInstanceId != (unsigned int)GAS_INVALID_ID)
      {
        nslFatal("Woah momma, what the hell is up with that?");
      }

      // INsanity checks
      if ( !theSound->looping && nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].length > NSL_SMALL3D_SOUND_SIZE )
      {
        if ( !theSound->inRange || theSound->gasInstanceId == (unsigned int)GAS_INVALID_ID )
        {
          nslFatal("Woah pappa, what the hell is up with that?");
        }
      }

      // INsanity checks
      if (!theSound->looping && nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].length <= NSL_SMALL3D_SOUND_SIZE)
      {
        if (theSound->inRange || theSound->gasInstanceId != (unsigned int)GAS_INVALID_ID)
        {
          nslFatal("Woah baby, what the hell is up with that?");
        }
      }

      nslFatal("Woah cousin, what the hell is up with that?");
    }
    
    // Set the pause stuff
    if (theSound->gasInstanceId != (nlUint32)GAS_INVALID_ID) 
    {
      currentCommandListEntry->set_paused = 0;
      
      if ((theSound->pauseCount > 0))
      {
        currentCommandListEntry->set_paused = 1;
        currentCommandListEntry->paused = 1;
      } 
      else 
      {
        currentCommandListEntry->set_paused = 1;
        currentCommandListEntry->paused = 0;
      }

      currentCommandListEntry->set_pitch = 1;
      currentCommandListEntry->pitch     = (short int)(theSound->pitch*GAS_COMMAND_PITCH_ONE);

/*      printf("%s (%d): NSL pitch %g gaspitch 0x%x\n", nslGetSourceName(theSound->mySource),
        theSound->gasInstanceId, 
        theSound->pitch, currentCommandListEntry->pitch );
        */
    }
  }
  return 0;
}

/*-------------------------------------------------------------------------
  nsl Set Master Volume
-------------------------------------------------------------------------*/
void nslSetMasterVolume(float newVolume) 
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  nsl.masterVolume = newVolume;
}

/*-------------------------------------------------------------------------
  nsl Get Master Volume
-------------------------------------------------------------------------*/
float nslGetMasterVolume( ) 
{
  if (!nsl.initialized) return 1;
  ASSERT_NSL_INITIALIZED();
  return nsl.masterVolume;
}


/*-------------------------------------------------------------------------
  nsl Set Volume
-------------------------------------------------------------------------*/
void nslSetVolume( nslSourceTypeEnum typeOfSound, float newVolume )
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  switch(typeOfSound) 
  {
    case NSL_SOURCETYPE_SFX:      nsl.sfxVolume = newVolume;     break;
    case NSL_SOURCETYPE_MUSIC:    nsl.musicVolume = newVolume;   break;
    case NSL_SOURCETYPE_AMBIENT:  nsl.ambientVolume = newVolume; break;
    case NSL_SOURCETYPE_VOICE:    nsl.voiceVolume = newVolume;   break;
    case NSL_SOURCETYPE_MOVIE:    nsl.movieVolume = newVolume;   break;
    case NSL_SOURCETYPE_USER1:    nsl.user1Volume = newVolume;   break;
    case NSL_SOURCETYPE_USER2:    nsl.user2Volume = newVolume;   break;

    default: nslFatal(false && "Unhandled sourcetype"); break;
  };
}


/*-------------------------------------------------------------------------
  nsl Get Volume
-------------------------------------------------------------------------*/
float nslGetVolume( nslSourceTypeEnum typeOfSound )
{
  if (!nsl.initialized) return 1;
  ASSERT_NSL_INITIALIZED();
  switch(typeOfSound) 
  {
    case NSL_SOURCETYPE_SFX:      return nsl.sfxVolume;     break;
    case NSL_SOURCETYPE_MUSIC:    return nsl.musicVolume;   break;
    case NSL_SOURCETYPE_AMBIENT:  return nsl.ambientVolume; break;
    case NSL_SOURCETYPE_VOICE:    return nsl.voiceVolume;   break;
    case NSL_SOURCETYPE_MOVIE:    return nsl.movieVolume;   break;
    case NSL_SOURCETYPE_USER1:    return nsl.user1Volume;   break;
    case NSL_SOURCETYPE_USER2:    return nsl.user2Volume;   break;
    default: nslFatal(false && "Unhandled sourcetype"); break;
  };

  // this code should never get reached.
  return 0.0f;
}


/*-------------------------------------------------------------------------
  nsl Set Master Balance
-------------------------------------------------------------------------*/
// balance is between -1.0 (left) to 1.0 (right) where 0.0 is the center
void nslSetMasterBalance( float newBalance)
{ 
/*  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  nsl.balance = newBalance;
  */
  //deprecated
}


/*-------------------------------------------------------------------------
  nsl Set Listener Po
-------------------------------------------------------------------------*/
void nslSetListenerPo( const nlMatrix4x4 &positionAndOrientation )
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  // Can this be a memcpy?
  memcpy(&nsl.listenerPo, &positionAndOrientation, sizeof(nlMatrix4x4));
  /*
  for (int i=0; i<4; i++)
    for( int j=0; j<4; j++ )  
      nsl.listenerPo[i][j] = positionAndOrientation[i][j];
      */
}


void nslGetListenerPo(nlMatrix4x4 *dest)
{
  memcpy(dest, &nsl.listenerPo, sizeof(nlMatrix4x4));
}
/*-------------------------------------------------------------------------
  nsl Set Listener Position
-------------------------------------------------------------------------*/
void nslSetListenerPosition( const nlVector3d &newPosition )
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  nsl.listenerPo[0][3] = newPosition[0];
  nsl.listenerPo[1][3] = newPosition[1];
  nsl.listenerPo[2][3] = newPosition[2];
}


/*-------------------------------------------------------------------------
  nsl Set Listener Orientation
-------------------------------------------------------------------------*/
void nslSetListenerOrientation( const nlVector3d &facingVector, const nlVector3d &upVector )
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  // Assumes facingVector and UpVector are normalized
 
  // Cross product doesn't use const's
  nlVector3d left, up, fwd;
  memcpy(fwd, facingVector, sizeof(facingVector));
  memcpy(up, upVector, sizeof(upVector));

  
  nlCrossProduct3d( left, up, fwd );
  float leftmag = sqrtf(left[0]*left[0] + left[1]*left[1] + left[2]*left[2]);

  left[0]/=leftmag;
  left[1]/=leftmag;
  left[2]/=leftmag;

  nsl.listenerPo[0][0] = left[0];         nsl.listenerPo[0][1] = left[1];     nsl.listenerPo[0][2] = left[2]; 
  nsl.listenerPo[1][0] = upVector[0];     nsl.listenerPo[1][1] = upVector[1];     nsl.listenerPo[1][2] = upVector[2]; 
  nsl.listenerPo[2][0] = fwd[0];          nsl.listenerPo[2][1] = fwd[1];    nsl.listenerPo[2][2] = fwd[2]; 
  

}
/*-------------------------------------------------------------------------
  nsl PS2 Pre Init Proview Mode
-------------------------------------------------------------------------*/

void nslPreInitProviewModePS2(bool on)
{
  nsl.proview = on;
}

void nslPreInitCdDvdModePS2(nslPs2CdDvdMode mode)
{
  nsl.cdDvdMode = mode;
}

void nslSetHostStreamingPS2(bool on)
{
  if (!nsl.initialized) return;
  nslPs2GasRpc(GAS_RPC_SET_HOST_STREAM, "", (on?1:0), 0, 0, 0);
}


/*-------------------------------------------------------------------------
  nsl Set Speaker Mode
-------------------------------------------------------------------------*/
void nslSetSpeakerMode( nslSpeakerModeEnum newMode )
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  if (newMode != nsl.speakerMode)
  {
    nsl.speakerMode = newMode;
    if (nsl.speakerMode != NSL_SPEAKER_MONO)
      nslPs2GasRpc(GAS_RPC_SET_STEREO, "", 1, 0, 0, 0);
    else 
      nslPs2GasRpc(GAS_RPC_SET_STEREO, "", 0, 0, 0, 0);
  }
}

/*-------------------------------------------------------------------------
  nsl Set Speaker Mode
-------------------------------------------------------------------------*/
nslSpeakerModeEnum nslGetSpeakerMode(  )
{ 
  return nsl.speakerMode;
}

/*-------------------------------------------------------------------------
  nsl Set Output Mode
-------------------------------------------------------------------------*/
void nslSetOutputMode( nslOutputModeEnum newMode )
{
  if (!nsl.initialized) return;
  ASSERT_NSL_INITIALIZED();
  nsl.outputMode = newMode;
}


void nslSetSystemCallbacks( nslSystemCallbackStruct* Callbacks )
{
  memcpy(&nslSystemCallbacks, Callbacks, sizeof(nslSystemCallbackStruct));  
}






/////////////////////////// internal functions ////////////////////////////


/*-------------------------------------------------------------------------
  _nsl Emitter For Each
-------------------------------------------------------------------------*/
void _nslEmitterForEach( _nslEmitterCallback cb, void *userData  )
{
  unsigned int i=0, active=0; 
  while ((i < NSL_NUM_EMITTERS))
  {
    if (nsl.emitterSlots[i].used)
    {
      active++;
      cb(&nsl.emitterSlots[i], userData);
    }

    i++;
  }
}

/*-------------------------------------------------------------------------
  _nsl Sound For Each
-------------------------------------------------------------------------*/
void _nslSoundForEach( _nslSoundCallback cb, void *userData )
{
  unsigned int i=0, active=0; 
  while ((i < NSL_NUM_SOUNDS))
  {
    if (nsl.soundSlots[i].used)
    {
      active++;
      cb(&nsl.soundSlots[i], userData);
    }
    i++;
  }
}


/*-------------------------------------------------------------------------
  _nsl Source For Each
-------------------------------------------------------------------------*/
void _nslSourceForEach( _nslSourceCallback cb, void *userData )
{
  unsigned int i=0, active=0; 
  while ((i < NSL_NUM_SOURCES) && (active < nsl.sourceSlotsUsedCount))
  {
    if (nsl.sourceSlots[i].used)
    {
      active++;
      cb(&nsl.sourceSlots[i], userData);
    }
    i++;
  }
}




/*-------------------------------------------------------------------------
  _nsl Reset Internal
-------------------------------------------------------------------------*/
void _nslResetInternal( _nslClearBehaviour clearStyle )
{
  _nslClearSystemData( clearStyle );
}


/*-------------------------------------------------------------------------
  _nsl Clear System Data
-------------------------------------------------------------------------*/
void _nslClearSystemData( _nslClearBehaviour clearStyle )
{
  int i;

	nlIdentityMatrix( nsl.listenerPo );
  switch (clearStyle) 
  {
    case NSL_CLEAR_INIT: 
      nsl.queuedSounds.init(NSL_MAX_QUEUED_SOUNDS); break;
    case NSL_CLEAR_RESET:
      nsl.queuedSounds.clear(); break;
    case NSL_CLEAR_FREE:
      nsl.queuedSounds.free(); break;
  }

  // Create a fifo_queue for sounds that weren't ready
  nsl.queuedSounds.clear();
//	nsl.balance = 0.0f;

  // Clear out the slots
  nsl.sourceSlotsUsedCount = 0;
  nsl.soundSlotsUsedCount = 0;
  nsl.emitterSlotsUsedCount = 0;

  for (i=0; i<NSL_NUM_SOURCES; ++i)
    _nslClearSourceSlot(i, true);
  for (i=0; i<NSL_NUM_SOUNDS; ++i)
    _nslClearSoundSlot(i, true);
  for (i=0; i<NSL_NUM_EMITTERS; ++i)
    _nslClearEmitterSlot(i, true, clearStyle);
}


/*-------------------------------------------------------------------------
  _nsl Clear Source Slot
-------------------------------------------------------------------------*/
void _nslClearSourceSlot( nlUint32 whichSlot, bool resetId )
{
  if ( whichSlot >= NSL_NUM_SOURCES)
    nslFatal("Make sure the slot is a legal one.");
  nslSource *mySlot = &nsl.sourceSlots[whichSlot];
  if (resetId)
    mySlot->myId = whichSlot;
  mySlot->bank = -1;
  mySlot->rawVolume = 1.0;
  mySlot->looping = false;
  mySlot->maxDist= -1;
  mySlot->minDist= -1;
  mySlot->pitch = 1.0;
//  mySlot->balance = 0.0;
  mySlot->used = false;

	mySlot->type = NSL_SOURCETYPE_SFX;
#ifdef NSL_LOAD_SOURCE_BY_NAME
  strcpy(mySlot->fileName,"unused");
#elif defined(NSL_LOAD_SOURCE_BY_ALIAS)
  mySlot->aliasID = NSL_INVALID_ID;
#endif
  mySlot->gasSourceId = (nlUint32)GAS_INVALID_ID;
}


/*-------------------------------------------------------------------------
  _nsl Clear Sound Slot
-------------------------------------------------------------------------*/
void _nslClearSoundSlot( nlUint32 whichSlot, bool resetId )
{
#ifdef DEBUG
  _nslEmitterForEach(_nslDebugCheckSoundSlotNotInAnyEmitters, &whichSlot);
#endif

  if (whichSlot >= NSL_NUM_SOUNDS )
    nslFatal("Make sure the slot is a legal one.");
  
  nslSound *mySlot = &nsl.soundSlots[whichSlot];

  if (resetId)
    mySlot->myId = whichSlot;

  mySlot->used = false;
  
  mySlot->mySource = NSL_INVALID_ID;
  mySlot->myEmitter = NSL_GLOBAL_EMITTER_ID;

  mySlot->pauseCount = 1;  // all sounds start off 'paused'
  mySlot->dampenCount = 0; // all sounds start off 'undampened'

  mySlot->rawVolume = 1.0f;
  mySlot->positionalVolume = 1.0f; 
  
  mySlot->left =  (short)(0.5f * SCE_VOLUME_MAX); 
  mySlot->right =  (short)(0.5f * SCE_VOLUME_MAX);
  
  mySlot->angle = 0;
//  mySlot->balance = 0;
  mySlot->isPlaying = false;
  mySlot->isReallyPlaying = false;
  mySlot->isReady = false;
  mySlot->isReallyReady = false;
  mySlot->looping = false;
  mySlot->isQueuing = false;
  mySlot->inRange = false;
  mySlot->minDist = -1;
  mySlot->maxDist = -1;
  mySlot->pitch = 1;

  mySlot->gasInstanceId = (nlUint32)GAS_INVALID_ID;
}

void _nslDebugSoundData()
{
  unsigned int i=0;
  for (i =0; i < nsl.soundSlotsUsedCount; i++)
  {
    if (nsl.soundSlots[i].used)
    {
#ifdef NSL_LOAD_SOURCE_BY_ALIAS

      nslPrintf("ID:%d\tGID:%d\tRP:%d\t%d\n", 
                      nsl.soundSlots[i].myId, 
                      nsl.soundSlots[i].gasInstanceId, 
                      nsl.soundSlots[i].isReallyPlaying, 
                      nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(nsl.soundSlots[i].mySource)].aliasID);
#endif
#ifdef NSL_LOAD_SOURCE_BY_NAME
      nslPrintf("ID:%d\tGID:%d\tRP:%d\t%s\n", 
                      nsl.soundSlots[i].myId, 
                      nsl.soundSlots[i].gasInstanceId, 
                      nsl.soundSlots[i].isReallyPlaying, 
                      nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(nsl.soundSlots[i].mySource)].fileName);
#endif
    }
  }

}
/*-------------------------------------------------------------------------
  _nsl Clear Emitter Slot
-------------------------------------------------------------------------*/
void _nslClearEmitterSlot( nlUint32 whichSlot, bool resetId, _nslClearBehaviour 
                           clearStyle )
{
  if ( whichSlot >= NSL_NUM_EMITTERS )
    nslFatal("Make sure the slot is a legal one.");
  nslEmitter *mySlot = &nsl.emitterSlots[whichSlot];

  if (resetId)
    mySlot->myId = whichSlot;

  mySlot->used = false;
  mySlot->autoRelease = false;
  mySlot->isALineEmitter = false;

  mySlot->startPosition[0] = 0.0f;
  mySlot->startPosition[1] = 0.0f;
  mySlot->startPosition[2] = 0.0f;

  mySlot->endPosition[0] = 0.0f;
  mySlot->endPosition[1] = 0.0f;
  mySlot->endPosition[2] = 0.0f;

  mySlot->position[0] = 0.0f;
  mySlot->position[1] = 0.0f;
  mySlot->position[2] = 0.0f;
  switch (clearStyle)
  {
    case NSL_CLEAR_INIT:  mySlot->emittedSounds.init(NSL_NUM_SOUNDS_PER_EMITTERS); break;
    case NSL_CLEAR_RESET: mySlot->emittedSounds.clear();                break;
    case NSL_CLEAR_FREE:  mySlot->emittedSounds.free();                 break;
  }
}


void _nslCheckAndMaybePlay3dSound(nslSound *theSound)
{
  // Ok.. this portion of code is designed to shut down 3d looping 
  // sounds when their volume is 0.  This way we can save gas 
  // resources.  First check to see if its looping
  int start = 0;
  // I have had some issues with sounds not ramping
  // and then (sometimes) turning off abruptly.  Not sure
  // why.  Its seems to be gone now.  

  // -- GT - I think this issue was due to an error in the min/max ranges 
  // for emitted 3D sounds being initialized to bogus values.  It's fixed now.
  if (theSound->pauseCount > 0)
	{
		_set3dVolume(theSound->myId);
    return;
	}
  if (theSound->looping) 
  {
    if (theSound->inRange) // if it was ReallyPlaying(tm)
    {
      theSound->inRange = _set3dVolume(theSound->myId);
      // and now it isn't now
      if (!theSound->inRange) 
      {
        // In order to make sure that the nslPs2GasRpc call
        // doesn't change the state of the nsl Sound instance
        // we have to set its id to GAS_INVALID_ID before
        // we call nslPs2GasRpc
        // So we copy the stoping id and set the nsl var 
        nlUint32 gasInst2Stop = theSound->gasInstanceId;
        theSound->gasInstanceId = (nlUint32)GAS_INVALID_ID; 
        if (gasInst2Stop != (nlUint32)GAS_INVALID_ID)
          nslPs2GasRpc(GAS_RPC_STOP_INSTANCE, "", gasInst2Stop, 0, 0, 0);
        nsl.queuedSounds.find(theSound->myId, true);
        theSound->isReallyReady = false;
      }
    }
    else  // it wasn't in range
    {
      theSound->inRange = _set3dVolume(theSound->myId);
      // but now it is
      if (theSound->inRange) 
      {
        // Play it 
        if (theSound->gasInstanceId == (nlUint32)GAS_INVALID_ID)
        {
          start++;
          theSound->gasInstanceId =  ~GAS_INSTANCE_STEREO_FLAG_BIT & nslPs2GasRpc(GAS_RPC_ADD_INSTANCE, "", nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].gasSourceId, 0, 0, 0);
          theSound->isReallyReady = false;
          if (theSound->gasInstanceId != (nlUint32)GAS_INVALID_ID)
          {
            /*
            nslPs2GasRpc(GAS_RPC_PLAY_INSTANCE, "", 
              nsl.soundSlots[i].gasInstanceId, 
              nsl.soundSlots[i].left, 
              nsl.soundSlots[i].right, 0);*/
            nsl.queuedSounds.find(theSound->myId, true);
            nslPlaySound(theSound->myId);
          }
        }
        else
        {
          if (nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].length >= NSL_SMALL3D_SOUND_SIZE)
          {
            nsl.queuedSounds.find(theSound->myId, true);
            nslPlaySound(theSound->myId);
          }
          else
          {
#ifdef NSL_LOAD_SOURCE_BY_NAME
            nslPrintf("WTF!!! We had a sound (%s) out of range that had a gas ID\n", nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].fileName);
#endif
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
            nslPrintf("WTF!!! We had a sound (%d) out of range that had a gas ID\n", nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].aliasID);
#endif          
          }

        }
      }
    } 

  }//end of if (looping)
  else
  {
    theSound->inRange = _set3dVolume(theSound->myId);
    if ((!theSound->inRange) &&
        (nsl.sourceSlots[NSL_GET_SLOT_FROM_ID(theSound->mySource)].length <= NSL_SMALL3D_SOUND_SIZE))
    {
      nslStopSound(theSound->myId);
    }
  }
  
}

/*--------------------------------------------------------------------------
  Callback stuff
--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------
  nslPrintf
--------------------------------------------------------------------------*/

void nslPrintf( const char* Format, ... )
{
  static char Work[1024];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  // Do we have a user spec'ed proc?
  if ( nslSystemCallbacks.DebugPrint )
    nslSystemCallbacks.DebugPrint( Work );
  else
  {
    // Print & bail
    printf( Work );
    fflush(stdout);
  }
}


/*--------------------------------------------------------------------------
  nslFatal
--------------------------------------------------------------------------*/

void nslFatal( const char* Format, ... )
{
  static char Work[1024];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  // Do we have a user spec'ed proc?
  if ( nslSystemCallbacks.CriticalError )
    nslSystemCallbacks.CriticalError( Work );
  else
  {
    // Print & bail
    printf( Work );
    fflush(stdout);
//		asm("break");  please implement something we can turn off. -wtb
//    for( ;; );
  }
}
void nslError( const char* Format, ... )
{
  static char Work[1024];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  // Do we have a user spec'ed proc?
  if ( nslSystemCallbacks.Error )
    nslSystemCallbacks.Error( Work );
  else
  {
    // Print & bail
    printf( Work );
    fflush(stdout);
//		asm("break");  please implement something we can turn off. -wtb
//    for( ;; );
  }
}

void nslWarning( const char* Format, ... )
{
  static char Work[1024];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  // Do we have a user spec'ed proc?
  if ( nslSystemCallbacks.Error )
    nslSystemCallbacks.Error( Work );
  else
  {
    // Print & bail
    printf( Work );
    fflush(stdout);
//		asm("break");  please implement something we can turn off. -wtb
//    for( ;; );
  }
}

int _nslDoReadNow( const char* FileName, nslFileBuf* File, u_int Align )
{
  if ( nslSystemCallbacks.ReadFile )
  {
    return nslSystemCallbacks.ReadFile( FileName, File, Align );
  }
  else
  {

    int fd = sceOpen( FileName, SCE_RDONLY );
    if ( fd < 0 )
      return false;
    // How big is this sucker?
    File->Size = sceLseek( fd, 0, SCE_SEEK_END );
    sceLseek( fd, 0, SCE_SEEK_SET );

    // ps2 only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
    File->Buf = (u_char*)nslMemAlloc( ( File->Size + 31 ) & ~31, 128 );
    sceRead( fd, File->Buf, ( File->Size + 31 ) & ~31 );
    sceClose( fd );
  }
  return true;
}

/*--------------------------------------------------------------------------
  nslReadFile
--------------------------------------------------------------------------*/

bool nslReadFile( const char* FileName, nslFileBuf* File, u_int Align )
{
  
 // User spec'ed?
  static char Work[256];
  extern char* nslHostPrefix;
  Work[0] = '\0';
  if ( strncmp( FileName, nslHostPrefix, strlen(nslHostPrefix) ) == 0 )
  {
    strcpy( Work, "" );
    if ((nsl.rootDir) && (strstr(FileName, nsl.rootDir) == NULL))
    {
      strcat(Work, nsl.rootDir);
      strcat(Work, "\\");
    }
    strcat( Work, FileName );
    return _nslDoReadNow(Work, File, Align);
  }
  else if ( strncmp( FileName, "cdrom0:", strlen("cdrom0:") ) == 0 )
  {
    strcpy( Work, "cdrom0:" );

    if ((nsl.rootDir) && (strstr(FileName, nsl.rootDir) == NULL))
    {
      strcat(Work, nsl.rootDir);
      strcat(Work, "\\");
    }
    
    strcat( Work, FileName + strlen("cdrom0:"));
    if ((FileName[strlen(FileName) - 1] != '1') || ((FileName[strlen(FileName) - 2] != ';')))
      strcat(Work, ";1");

    return _nslDoReadNow(Work, File, Align);
  }
  else // try both, cd first
  {
    strcpy( Work, "cdrom0:" );
    if ((nsl.rootDir) && (strstr(FileName, nsl.rootDir) == NULL))
    {
      strcat( Work, nsl.rootDir );
      strcat( Work, "\\" );
    }
    strcat( Work, FileName ); 
    if ( _nslDoReadNow(Work, File, Align) )
      return true;
    else
    {
      strcpy( Work, "host0:" );
      if ((nsl.rootDir) && (strstr(FileName, nsl.rootDir) == NULL))
      {
        strcat( Work, nsl.rootDir );
        strcat( Work, "\\" );
      }
      strcat( Work, FileName ); 
      return _nslDoReadNow(Work, File, Align);
    }
  }



}

/*--------------------------------------------------------------------------
  nslReleaseFile
--------------------------------------------------------------------------*/

void nslReleaseFile( nslFileBuf* File )
{
  if ( nslSystemCallbacks.ReleaseFile )
    nslSystemCallbacks.ReleaseFile( File );
  else
  {
    nslMemFree( File->Buf );
    memset( File, 0, sizeof(nslFileBuf) );
  }
}

/*--------------------------------------------------------------------------
  nslMemAlloc
--------------------------------------------------------------------------*/

void* nslMemAlloc( u_int Size, u_int Align )
{
  if ( nslSystemCallbacks.MemAlloc )
    return nslSystemCallbacks.MemAlloc( Size, Align );
  else
    return memalign( Align, Size );
}


/*--------------------------------------------------------------------------
  nslMemFree
--------------------------------------------------------------------------*/

void nslMemFree( void* Ptr )
{
  if ( nslSystemCallbacks.MemFree )
    nslSystemCallbacks.MemFree( Ptr );
  else
    free( Ptr );
}



/*--------------------------------------------------------------------------
  _nslLoadModule
--------------------------------------------------------------------------*/

bool _nslLoadModule(char *mod) 
{
  int counter=0;
  char cd[256];

  // Try the CD
  strcpy(cd, "cdrom0:\\");
  strcat(cd, mod);
  strcat(cd, ";1");
  while ((sceSifLoadModule(cd, 0, NULL) < 0) && (counter < 5))
    counter++;
  

  
  if (counter < 5) return true;
  else  // Try the host
  {
    counter = 0;
    strcpy(cd, "host0:");
    strcat(cd, mod);

    while ((sceSifLoadModule(cd, 0, NULL) < 0) && (counter < 5))
      counter++;
    
    if (counter < 5) return true;
  }

  // Shit, bail
  return false;
}



/*--------------------------------------------------------------------------
  getWord
--------------------------------------------------------------------------*/

int _getWord(char linebuf[256], char word[20], int start_pos)
{
  int new_pos = start_pos;
  int total = 0;

  while ((linebuf[new_pos] != 13) && (linebuf[new_pos] != 10) && (start_pos < 256) && (total < 20) && (linebuf[new_pos] != ' ') && (linebuf[new_pos] != '\0'))
  {
    word[total] = linebuf[new_pos];
    total++;
    new_pos++;
  }
  word[total] = '\0';
  
  if (linebuf[new_pos] == '\0')
    return -1;
  while ((linebuf[new_pos] == 13) || (linebuf[new_pos] == 10) ||  (linebuf[new_pos] == ' '))
  {
    new_pos++;
  }

  return new_pos;
 
}


/*--------------------------------------------------------------------------
  _nslLoadSourceData

  We need to track the data regarding the 
  files we load in.. 
--------------------------------------------------------------------------*/
/*
void _nslLoadSourceData(nslFileBuf file)
{
  unsigned int lines = 0;
  char linebuf[256];
// unused?   char linebuf2[256];
  unsigned int pos = 0;
  int line_pos = 0;
  int numEntries = 0;
// unused?   int start_pos = 0;

  // Quick line count
  for (unsigned int i=0; i < file.Size; i++)
    if (file.Buf[i] == 13)
      lines++;

  // Examine each line
  for (unsigned int i=0; i < lines; i++)
  { 
    int length = 0;
    line_pos = 0;

    // Copy just this line into a buffer
    while ((line_pos < 255) && (pos < file.Size) && (file.Buf[pos] != 13))
    { 
      linebuf[line_pos] = file.Buf[pos];
      line_pos++;
      pos++;
    }
  linebuf[line_pos] = '\0';
    // Get rid of the last litte LF/CR
    pos+=2;
    length = line_pos;
    line_pos = 0;
    

    // Make sure its not a comment
    if (!(linebuf[0] == ';'))
    {
      char word[20];
      //A real line
      // Grab the name
      _getWord(linebuf, word, line_pos);
      strcpy(nsl.sourceInfo[numEntries].name, word);
      
      // Init it
      nsl.sourceInfo[numEntries].looping = false;
      nsl.sourceInfo[numEntries].sourceType = NSL_SOURCETYPE_SFX;

      // Now we look for tags
      while((line_pos < 255) && (line_pos != -1))
      {
        // Grab a word off the line
        line_pos = _getWord(linebuf, word, line_pos);

        // Is it a sound type flag or loop flag?
        if (strcmp(word, "loop") == 0) {
          nsl.sourceInfo[numEntries].looping = true;
        }
        else if (strcmp(word, "sfx") == 0) 
        {
          nsl.sourceInfo[numEntries].sourceType = NSL_SOURCETYPE_SFX;
        } 
        else if (strcmp(word, "ambient") == 0)
        {
          nsl.sourceInfo[numEntries].sourceType = NSL_SOURCETYPE_AMBIENT;
        } 
        else if (strcmp(word, "voice") == 0)
        {
          nsl.sourceInfo[numEntries].sourceType = NSL_SOURCETYPE_VOICE;
        } 
        else if (strcmp(word, "music") == 0) 
        {
          nsl.sourceInfo[numEntries].sourceType = NSL_SOURCETYPE_MUSIC;
        } 
      }
      numEntries++;
    }
  } 
}

*/