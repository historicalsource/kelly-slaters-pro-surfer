/*****************************************************************
 * testCases.cpp
 * This file contains functions that 
 * test the functionality of nsl.
 * 
 * I added a functionality which 
 * sets the flag "NSLasserted" when the procedure would have
 * asserted.  In this way we can check to make sure it DOES
 * assert without killing the program.
 *
 * In the ps2 implementation 
 * they assert.  Similar behavior
 * should be used on other
 * platforms
 * 
 *  -Kevin Schmidt 
 ****************************************************************/

#include "../common/nsl.h"
#include "assert.h"

nslSoundId longStreamingSound;
nslSoundId shortSPUSound;
nslSourceId s;
nslSourceId s2;




//=======================================
// TESTING nslPlaySound 
//=======================================


// nslTestPlayInvalidId
// This procedure should assert!
// Use only when you can deal with it!

void nslTestPlayInvalidId( void ) 
{
  nslSoundId invalid;
  nslPlaySound(invalid);
}


// nslTestPlayValidIdAfterStreaming
// This should simply play the sounds ok.
/*
void nslTestPlayValidIdAfterStreaming( void ) 
{
  s   = nslGetSource("CITYLOW", false);
  s2  = nslGetSource("EXPL_CEI", false);
  
  shortSPUSound       = nslAddSound(s2);
  longStreamingSound  = nslAddSound(s);
  
  nslPlaySound(longStreamingSound);
  nslPlaySound(shortSPUSound);

  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING )
  {
    nslFrameAdvance(.1); 
  };

  nslStopSound(shortSPUSound);
  nslStopSound(longStreamingSound);

  

}

// nslTestPlayValidId
// Should simply play

void nslTestPlayValidId( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) 
  {
    nslFrameAdvance(.1); 
  };

  nslStopSound(shortSPUSound);
  


}

// nslTestPlayAfterRunOut
// Should play, stop, assert

void nslTestPlayAfterRunOut( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) 
  {
    nslFrameAdvance(.1); 
  };
  
  // Invalid.. assert
  nslPlaySound(shortSPUSound);
}

// nslTestPlayAfterStop
// Should assert
void nslTestPlayAfterStop( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  
  // Invalid.. assert
  nslPlaySound(shortSPUSound);  


}



//=======================================
// TESTING nslIsSoundPlaying
// NONE OF THESE SHOULD ASSERT
//=======================================

// nslTestPlayInvalidId

void nslTestIsPlayingInvalidId( void ) 
{
  nslSoundId invalid;
  int ret = nslGetSoundStatus(invalid) == NSL_SOUNDSTATUS_PLAYING;
  assert (ret == 0);
}


// nslTestIsPlayingValidIdAfterStreaming


void nslTestIsPlayingValidIdAfterStreaming( void ) 
{
  s   = nslGetSource("EXPL_CEI", false);  
  longStreamingSound  = nslAddSound(s);

  nslPlaySound(longStreamingSound);
  int ret = nslGetSoundStatus(longStreamingSound) == NSL_SOUNDSTATUS_PLAYING;

  nslStopSound(longStreamingSound);

  

  assert(ret != 0);

}

// nslTestIsPlayingValidId

void nslTestIsPlayingValidId( void ) 
{

  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  int ret =  nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;

  nslStopSound(shortSPUSound);
  

  assert(ret != 0);
 
}


// nslTestIsPlayingAfterRunOut


void nslTestIsPlayingAfterRunOut( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) 
  {
    nslFrameAdvance(.1); 
  };
  
  
  int ret = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;
  assert(ret == 0);

}

// nslTestIsPlayingAfterStop
void nslTestIsPlayingAfterStop( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  
  int ret = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;

  assert(ret == 0);
}

//=============================================
// TESTING nslIsSoundReady
//=============================================

// nslTestIsSoundReadyInvalidId

void nslTestIsSoundReadyInvalidId( void ) 
{
  nslSoundId invalid;
  nslGetSoundStatus(invalid);
}


// nslTestIsSoundReadyValidIdAfterLoaded
// Shouldn't assert

void nslTestIsSoundReadyValidIdAfterLoaded( void ) 
{
  s   = nslLoadSource("CITYLOW");  
  longStreamingSound  = nslAddSound(s);
  int ret = (nslGetSoundStatus(longStreamingSound) == NSL_SOUNDSTATUS_READY);

  nslPlaySound(longStreamingSound);
  nslStopSound(longStreamingSound);

  
} 

// nslTestIsSoundReadyValidId

void nslTestIsSoundReadyValidId( void ) 
{

  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  int ret = (nslGetSoundStatus(longStreamingSound) == NSL_SOUNDSTATUS_READY);
  nslPlaySound(shortSPUSound);
  

  nslStopSound(shortSPUSound);
  

}


// nslTestIsSoundReadyAfterRunOut
// Should assert

void nslTestIsSoundReadyAfterRunOut( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) {
    nslFrameAdvance(.1); 
  };
  
  nslGetSoundStatus(longStreamingSound);
  

}

// nslTestIsSoundReadyAfterStop
void nslTestIsSoundReadyAfterStop( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  
  // ASSERTS
  nslGetSoundStatus(longStreamingSound);
  
}


void nslTestIsSoundReadyWhilePaused ( void )
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  
  // ASSERTS
  nslGetSoundStatus(shortSPUSound);
  nslStopSound(shortSPUSound);
  

}




//=============================================
// TESTING nslPauseSound
//=============================================

void nslPauseSoundInvalidId( void )
{
  nslSoundId snd;
  nslPauseSound(snd);
}

void nslPauseSoundValidIdPlaying( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  nslUnpauseSound(shortSPUSound);
  nslStopSound(shortSPUSound);
  
}

void nslPauseSoundWhilePaused( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  nslStopSound(shortSPUSound);
  
}


void nslPauseSoundAfterRunOut( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) 
  {
    nslFrameAdvance(.1); 
  };
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  
}

void nslPauseSoundAfterStop( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  
}

void nslUnpauseSoundInvalidId( void  ) 
{
  nslSoundId snd;
  nslUnpauseSound(snd);
}

void nslUnpauseSoundAfterRunOut( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) 
  {
    nslFrameAdvance(.1); 
  };
  nslUnpauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  
}

void nslUnpauseSoundAfterStop( void ) 
{
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  nslUnpauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  
}

void nslPauseUnpauseMultiple( void ) 
{
  bool isPlaying;
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  isPlaying = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;
  assert(!isPlaying);
  nslUnpauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  isPlaying = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;
  assert(!isPlaying);
  nslUnpauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  isPlaying = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;
  assert(!isPlaying);
  nslUnpauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  isPlaying = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;
  assert(isPlaying);
  
}

void nslPauseGuardSoundInvalidId() 
{
  nslSoundId snd;
  nslPauseGuardSound(snd);
}

void nslPauseGuardSoundValidIdAfterStart() 
{
  bool isPlaying;
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslPauseGuardSound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslFrameAdvance(.1);
  isPlaying = nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING;
  assert(isPlaying);
  
}

void nslPauseGuardSoundPaused() 
{
  bool isPlaying;
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslPauseSound(shortSPUSound);
  nslPauseGuardSound(shortSPUSound);
  
}
void nslPauseGuardSoundValidIdAfterStop() 
{
  bool isPlaying;
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  nslPauseGuardSound(shortSPUSound);
  
}
void nslPauseGuardSoundValidIdAfterRunOut() 
{
  bool isPlaying;
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound  = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while(nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING) 
  {
    nslFrameAdvance(.1); 
  };
  nslPauseGuardSound(shortSPUSound);
  
}


void nslPauseAllSoundsNoSounds() 
{
  nslPauseAllSounds();
}


void nslPauseAllSoundsValid() 
{
  nslSoundId snd1, snd2;
  bool isPaused1, isPaused2;
  s  = nslGetSource("EXPL_CEI", false);
  snd1 = nslAddSound(s);
  snd2 = nslAddSound(s);
  nslPlaySound(snd1);
  nslPlaySound(snd2);
  nslPauseAllSounds();
  isPaused1 = !(nslGetSoundStatus(snd1) == NSL_SOUNDSTATUS_PLAYING);
  isPaused2 = !(nslGetSoundStatus(snd2) == NSL_SOUNDSTATUS_PLAYING);
  assert(isPaused1 && isPaused2);
  
}

void nslUnpauseAllSoundsPaused() {
  nslSoundId snd1, snd2;
  bool isPlaying1, isPlaying2;
  s  = nslGetSource("EXPL_CEI", false);
  snd1 = nslAddSound(s);
  snd2 = nslAddSound(s);
  nslPlaySound(snd1);
  nslPlaySound(snd2);
  nslPauseAllSounds();
  nslFrameAdvance(.1);
  nslPauseAllSounds();
  nslFrameAdvance(.1);
  nslUnpauseAllSounds();
  nslFrameAdvance(.1);
  nslUnpauseAllSounds();
  isPlaying1 = (nslGetSoundStatus(snd1) == NSL_SOUNDSTATUS_PLAYING);
  isPlaying2 = (nslGetSoundStatus(snd2) == NSL_SOUNDSTATUS_PLAYING);
  assert(isPlaying1 && isPlaying2);
  
}

void nslUnpauseAllSoundsNoSounds() 
{
  nslUnpauseAllSounds();
}

void nslStopSoundInvalidId() {
  nslSoundId invalidId;
  nslStopSound(invalidId);
} 


void nslStopSoundAfterRunOut() {
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  while (nslGetSoundStatus(shortSPUSound) == NSL_SOUNDSTATUS_PLAYING)
  {
    nslFrameAdvance(.1);
  }
  nslStopSound(shortSPUSound);
} 

void nslStopSoundAfterStop() {
  s  = nslGetSource("EXPL_CEI", false);
  shortSPUSound = nslAddSound(s);
  nslPlaySound(shortSPUSound);
  nslStopSound(shortSPUSound);
  nslStopSound(shortSPUSound);
} 
*/