// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX)
#include <stdlib.h>
#include <stdio.h>
#include "ngl.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "VOEngine.h"
#include "ksnsl.h"
#include "random.h"
#include "kshooks.h"	// For KSWhatever calls
#include "careerdata.h"
#include "beachdata.h"

VOEngine VoiceOvers;
//bool KSReadFile( char* FileName, nglFileBuf* File, u_int Align );
//===============================
//
//  R A N D O M V O
//
//===============================

void RandomVO::init()
{
  // Setup a RandomVO
  probability = .5;
  valid = true;
  for (int i=0; i < MAX_RANDOM_SOURCES; i++) {
    sources[i] = sourcesUsed[i] = sourcesUnused[i] = NSL_INVALID_ID;
  }
  numUsedSources=0;
  numUnusedSources=0;
  totalSources=0;
  lastSource = NSL_INVALID_ID;
}

//Play a vo
int RandomVO::play()
{
  int index = 0;
  if (!valid) return NSL_INVALID_ID;
  if (totalSources == 0) return NSL_INVALID_ID;

  // Have we run out?
  // Then move them all back
  if (numUnusedSources == 0) {
    for (int i=0; i < totalSources; i++)
    {
      sourcesUnused[i] = sources[i];
      sourcesUsed[i] = NSL_INVALID_ID;
    }
    numUnusedSources = totalSources;
    numUsedSources = 0;
  }
  // Check random factor to see if we should
  // play
  float val = random();
  if (val > probability)
    return NSL_INVALID_ID;

  // Random number between 0 and unused - 1
  int src = random(numUnusedSources);

  for (int i=0; i < totalSources; i++)
  {
    if (sources[i] == sourcesUnused[src])
      index = i;
  }
  lastSource = sourcesUnused[src];
  if (lastSource != NSL_INVALID_ID)
  {
    //  Create that sound
    thisSound = nslAddSound(lastSource);
    if (thisSound != NSL_INVALID_ID)
    {
      // add it to the used list
      sourcesUsed[numUsedSources++] = src;
      // Move the last one to the spot that we removed
      sourcesUnused[src] = sourcesUnused[--numUnusedSources];
      // Play it.. it will get done soon enough
      if (thisSound != NSL_INVALID_ID)
        nslPlaySound(thisSound);
    }
    else
    {
      return NSL_INVALID_ID;
    }
  }
  else
  {
    return NSL_INVALID_ID;
  }
  return thisSound;
}

//Play a vo
int RandomVO::play3d(float x, float y, float z, float min, float max)
{
  int index = 0;
  if (!valid) return NSL_INVALID_ID;
  if (totalSources == 0) return NSL_INVALID_ID;

  // Have we run out?
  // Then move them all back
  if (numUnusedSources == 0) {
    for (int i=0; i < totalSources; i++)
    {
      sourcesUnused[i] = sources[i];
      sourcesUsed[i] = NSL_INVALID_ID;
    }
    numUnusedSources = totalSources;
    numUsedSources = 0;
  }
  // Check random factor to see if we should
  // play
  float val = random();
  if (val > probability)
    return NSL_INVALID_ID;

  // Random number between 0 and unused - 1
  int src = random(numUnusedSources);

  for (int i=0; i < totalSources; i++)
  {
    if (sources[i] == sourcesUnused[src])
      index = i;
  }
  lastSource = sourcesUnused[src];
  //  Create that sound
	if (lastSource == NSL_INVALID_ID)
		return NSL_INVALID_ID;
  thisSound = nslAddSound(lastSource);
	if (thisSound == NSL_INVALID_ID)
		return NSL_INVALID_ID;
  // add it to the used list
  sourcesUsed[numUsedSources++] = src;
  // Move the last one to the spot that we removed
  sourcesUnused[src] = sourcesUnused[--numUnusedSources];
  // Play it.. it will get done soon enough
  nlVector3d pos;
  pos[0] = x; pos[1] = y; pos[2] = z;

  nslEmitterId e = nslCreateEmitter(pos);
  nslSetSoundEmitter(e, thisSound);
  nslSetSoundRange(thisSound, min, max);
  nslSetEmitterAutoRelease(e);
  nslPlaySound(thisSound);

  return thisSound;
}


int RandomVO::play(int i)
{
  if (( i < 0 ) || ( i >= totalSources )) return NSL_INVALID_ID;
  if (!valid) return NSL_INVALID_ID;
  if (totalSources == 0) return NSL_INVALID_ID;

  // Check random factor to see if we should
  // play
  float val = random();
  if (val > probability)
    return -1;

  // Random number between 0 and unused - 1

  //  Create that sound
	if (sources[i] == NSL_INVALID_ID)
		return NSL_INVALID_ID;
  thisSound = nslAddSound(sources[i]);
	if (thisSound == NSL_INVALID_ID)
		return NSL_INVALID_ID;

  nslPlaySound(thisSound);
  return i;
}

int RandomVO::getLastSourcePlayed()
{
  return lastSource;
}

// Pretty simple
void RandomVO::stop()
{
  if (isPlaying())
    nslStopSound(thisSound);
}

bool RandomVO::isPlaying()
{
  return (nslGetSoundStatus(thisSound) != NSL_SOUNDSTATUS_INVALID);
}

void RandomVO::setProbability(float p)
{
  probability=p;
}
void RandomVO::shutdown()
{
  totalSources = 0;
  numUnusedSources = 0;
  numUsedSources = 0;

  for (int i=0; i < MAX_RANDOM_SOURCES; i++)
  {
    sources[i] = sourcesUsed[i] = sourcesUnused[i] = NSL_INVALID_ID;
  }
}
// Add a source
bool RandomVO::addSource(nslSourceId s)
{
  if (!valid) init();
  if (totalSources < MAX_RANDOM_SOURCES)
  {
    sources[totalSources++] = s;
    sourcesUnused[numUnusedSources++] = s;
    return true;
  }
  else
  {
    return false;
  }
}

//===============================
//
//  V O E N G I N E
//
//===============================

void VOEngine::init()
{
	int i=0, j=0;
	myTimer = 0.0f;
	played = false;
	for (i=0; i < LEVEL_LAST; i++)
	{
		beachChallVO[i][0] = beachChallVO[i][0] = NSL_INVALID_ID;
		beachChallVO[i][1] = beachChallVO[i][1] = NSL_INVALID_ID;
		whichVisit[i] = 0;
	}
	currentSound = NSL_INVALID_ID;
	char beachname[32];
	// Load up all the sounds!
	for (i=0; i < LEVEL_LAST; i++)
	{
		whichVisit[CareerDataArray[i].beach]++;
		int appendNum = whichVisit[CareerDataArray[i].beach];
		strcpy(beachname, BeachDataArray[CareerDataArray[i].beach].name);
		
		j = 0;
		{
			// SKILL CHALLENGES
			/*if (CareerDataArray[i].goal[j] == GOAL_BUOY)
			{
				
				// Try beach specific
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_BUOY0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_BUOY0") + stringx(appendNum)).c_str());
				// Load for any beach
				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("KS_ANY_SKILLBUOY01");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource("OTHER_ANY_SKILLBUOY01");
			}
			else*/ if (CareerDataArray[i].goal[j] == GOAL_SKILL_FACE || CareerDataArray[i].goal[j] == GOAL_SKILL_FACE_SCORE)
			{
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_FACE0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_FACE0") + stringx(appendNum)).c_str());

				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("KS_ANY_SKILLFACE01");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource("OTHER_ANY_SKILLFACE01");

			}
			else if (CareerDataArray[i].goal[j] == GOAL_SKILL_TUBE || CareerDataArray[i].goal[j] == GOAL_SKILL_TUBE_SCORE)
			{
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_TUBE0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_TUBE0") + stringx(appendNum)).c_str());

				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("KS_ANY_SKILLTUBE01");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource("OTHER_ANY_SKILLTUBE01");
			}
			else if (CareerDataArray[i].goal[j] == GOAL_SKILL_AIR  || CareerDataArray[i].goal[j] == GOAL_SKILL_AIR_SCORE)
			{

				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_AIR0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_AIR0") + stringx(appendNum)).c_str());

				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("KS_ANY_SKILLAIR01");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource("OTHER_ANY_SKILLAIR01");	
			}
			else if (CareerDataArray[i].goal[j] == GOAL_SKILL_360_SPIN || CareerDataArray[i].goal[j] == GOAL_SKILL_360_SPIN_SCORE ||
				CareerDataArray[i].goal[j] == GOAL_SKILL_540_SPIN || CareerDataArray[i].goal[j] == GOAL_SKILL_540_SPIN_SCORE)
			{
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_SPIN0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_SPIN0") + stringx(appendNum)).c_str());

				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource(("KS_ANY_SPIN0" + stringx(appendNum)).c_str());
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource(("OTHER_ANY_SPIN0" + stringx(appendNum)).c_str());	
			}
			// GHOST ICON
			else if (CareerDataArray[i].goal[j] == GOAL_ICON_3D || CareerDataArray[i].goal[j] == GOAL_ICON_TETRIS )
			{
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_GHOST0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_GHOST0") + stringx(appendNum)).c_str());

				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("KS_ANY_GHOST01");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource("OTHER_ANY_GHOST01");	
			}
			// NEW TRICK
			else if (CareerDataArray[i].goal[j] == GOAL_LEARN_NEW_TRICK)
			{
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname) + stringx("_NEWTRICK0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname) + stringx("_NEWTRICK0") + stringx(appendNum)).c_str());

				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("KS_ANY_GHOST01");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][1] = nslLoadSource("OTHER_ANY_GHOST01");	
			}
			// Competitions
			else if (CareerDataArray[i].goal[j] == GOAL_COMPETITION_1 ||
			         CareerDataArray[i].goal[j] == GOAL_COMPETITION_2 ||
					 CareerDataArray[i].goal[j] == GOAL_COMPETITION_3)
			{
				beachChallVO[i][0] = nslLoadSource((stringx("SPONSOR_") +stringx(beachname) + stringx("_COMP0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = beachChallVO[i][0];
			}
			else if ((CareerDataArray[i].goal[j] == GOAL_PHOTO_1) || 
			         (CareerDataArray[i].goal[j] == GOAL_PHOTO_2) ||
			         (CareerDataArray[i].goal[j] == GOAL_PHOTO_3))
			{
				beachChallVO[i][0] = nslLoadSource((stringx("SPONSOR_") +stringx(beachname) + stringx("_PHOTO0") + stringx(appendNum)).c_str());
				beachChallVO[i][1] = beachChallVO[i][0];
			}
			else if (CareerDataArray[i].goal[j] == GOAL_TUTORIAL_1 || 
			         CareerDataArray[i].goal[j] == GOAL_TUTORIAL_2 || 
			         CareerDataArray[i].goal[j] == GOAL_TUTORIAL_3) 
			{
				beachChallVO[i][0] = nslLoadSource((stringx("KS_") + stringx(beachname)).c_str());
				beachChallVO[i][1] = nslLoadSource((stringx("OTHER_") + stringx(beachname)).c_str());

			}

			// Fill in w/ placeholder sound
			if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_PLAY_PLACEHOLDER))
			{
				if (beachChallVO[i][0] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("PLACEHOLDER");
				if (beachChallVO[i][1] == NSL_INVALID_ID)
					beachChallVO[i][0] = nslLoadSource("PLACEHOLDER");
			}


		}
	}

  on = true;
}


void VOEngine::frameAdvance(float timeInc)
{
	if (!on) return;

}
void VOEngine::playVO()
{
	stopVO();
	int othersurfer = SURFER_LAST;
	// ARG!  Bad hack to deal with bad vo data
	if (CareerDataArray[currentLevel].beach == BEACH_TRESTLES)
		othersurfer=SURFER_NATHAN_FLETCHER;

	// Use other surfer
	if (currentSurfer == SURFER_KELLY_SLATER || currentSurfer == othersurfer)
	{
		if (beachChallVO[currentLevel][1] != NSL_INVALID_ID)
			currentSound = nslAddSound(beachChallVO[currentLevel][1]);
		else
			currentSound = NSL_INVALID_ID;
		
		if (currentSound != NSL_INVALID_ID) nslPlaySound(currentSound);
	}
	else // Use KS
	{
		if (beachChallVO[currentLevel][0] != NSL_INVALID_ID)
			currentSound = nslAddSound(beachChallVO[currentLevel][0]);
		else
			currentSound = NSL_INVALID_ID;
		if (currentSound != NSL_INVALID_ID) nslPlaySound(currentSound);
	}

}
void VOEngine::stopVO()
{
	if (nslGetSoundStatus(currentSound) != NSL_INVALID_ID)
		nslStopSound(currentSound);
}
void VOEngine::setCurrentLevel(int level)  
{ 
	if (nslGetSoundStatus(currentSound) != NSL_INVALID_ID)
		nslStopSound(currentSound);
	myTimer = 0;
	currentLevel  = level; 
};

bool VOEngine::isPlaying()
{
  if (!on) return false;
  playing = (nslGetSoundStatus(currentSound) != NSL_SOUNDSTATUS_INVALID);
  return (playing);
}




//===============================
// Play section
//===============================
/*
nslSoundId VOEngine::doinBigTrick()
{
  if (!on) return NSL_INVALID_ID;
  if (paused) return NSL_INVALID_ID;

  bigSpeaker = random(2);
  if (numBigTricksKS == 0) return currentSound;
  if (numBigTricksDF == 0) return currentSound;
  if (currentBigTrickKS == -1) return NSL_INVALID_ID;
  if (currentBigTrickDF == -1) return NSL_INVALID_ID;
  if (bigSpeaker)
  {
    currentBigTrickKS = random(numBigTricksKS);
    currentSound = nslAddSound(KSfGroup[currentBigTrickKS]);
    nslPlaySound(currentSound);
    return currentSound;
  }
  else
  {
    currentBigTrickDF = random(numBigTricksDF);
    currentSound = nslAddSound(DFfGroup[currentBigTrickDF]);
    nslPlaySound(currentSound);
    return currentSound;
  }
}

nslSoundId VOEngine::didBigTrick()
{
  if (!on) return NSL_INVALID_ID;
  if (paused) return NSL_INVALID_ID;  

  if (bigSpeaker < 0)
    return currentSound;
  if (currentBigTrickKS == -1) return NSL_INVALID_ID;
  if (currentBigTrickDF == -1) return NSL_INVALID_ID;
  if (bigSpeaker)
  {
    bigSpeaker = -1;
    currentSound = nslAddSound(KSfGroupA[currentBigTrickKS]);
    nslPlaySound(currentSound);
    return currentSound;
  }
  else
  {
    bigSpeaker = -1;
    currentSound = nslAddSound(DFfGroupA[currentBigTrickDF]);
    nslPlaySound(currentSound);
    return currentSound;
  }
}

nslSoundId VOEngine::failedBigTrick()
{
  if (!on) return NSL_INVALID_ID;
  if (paused) return NSL_INVALID_ID;

  if (bigSpeaker < 0)
    return currentSound;
  if (currentBigTrickKS == -1) return NSL_INVALID_ID;
  if (currentBigTrickDF == -1) return NSL_INVALID_ID;
  if (bigSpeaker)
  {
    bigSpeaker = -1;
    currentSound = nslAddSound(KSfGroupB[currentBigTrickKS]);
    nslPlaySound(currentSound);
    return currentSound;
  }
  else
  {
    bigSpeaker = -1;
    currentSound = nslAddSound(DFfGroupB[currentBigTrickDF]);
    nslPlaySound(currentSound);
    return currentSound;
  }

}




void VOEngine::registerTrick(int t)
{
  if (!on) return;
  if (paused) return;

  if ((t == lastTrick) && (lastTrick == trickBeforeThat))
  {
    if (!isPlaying())
    {
      speaker = random(2);
      if (speaker)
      {
        currentSound = KStypeB.play();
      }
      else
      {
        currentSound = DFtypeB.play();
      }
    }
       
  }
  trickBeforeThat = lastTrick;
  lastTrick = t;  

}
nslSoundId VOEngine::landedTrick(int numTricksInChain, int points)
{
  if (!on) return NSL_INVALID_ID;
  if (paused) return NSL_INVALID_ID;
  
  if (points > coolPointLevel)
  {
    landedTricks++;
    int speaker = random(2);
    if (speaker)
      currentSound = KStypeD.play();
    else
      currentSound = DFtypeD.play();
  }
  
  return currentSound;
	return NSL_INVALID_ID;
}


nslSoundId VOEngine::wipeOut()
{
	
  if (!on) return NSL_INVALID_ID; 
  if (paused) return NSL_INVALID_ID;

  if (isPlaying()) return currentSound;

  if (bigSpeaker >= 0)
  {
    return failedBigTrick();
  }
  else
  {
    speaker = random(2);
    if (speaker) 
      currentSound = KStypeA.play();
    else
      currentSound = DFtypeA.play();
    wipeouts++;
    return currentSound;  
  }
	return NSL_INVALID_ID;
}


*/