/***************************************************************
 *
 *  VOEngine & RandomVO
 *  These two classes are used to play voices
 *  The VOEngine is designed to play only one 
 *  sound at once, but play them randomly, 
 *  and for it to be not too repetitious
 *  
 *  The RandomVO allows for several sounds
 *  of the same type to be cycled through in 
 *  random order.. although all will be played 
 *  before one is repeated.
 *
 *  VOEngine is just a collection of different
 *  RandomVO's with some helpful procs
 *
 **************************************************************/

#ifndef VOENGINE
#define VOENGINE
#include "ksnsl.h"
#include "tricks.h"
#include "math.h"
#include "careerdata.h"
#include "beachdata.h"

#define MAX_RANDOM_SOURCES 50
#define MAX_POINT_LEVELS 5
#define POINTS_TO_INDEX(pts) (int)(log(pts))


#define NUM_BAIL10_TAGS 3
#define VO_WAIT_THRESHOLD 1.0f
#define NUM_DIFF_SURFERS 2

// This class plays a random source 
class RandomVO
{
private:
  float             probability;
  nslSoundId        thisSound;
  int               totalSources;
  int               numUsedSources;
  int               numUnusedSources;
  bool              valid;
  int               lastSource;
  int               probablity;
  nslEmitterId      pos;              
  nslSourceId sources[MAX_RANDOM_SOURCES];
  nslSourceId sourcesUsed[MAX_RANDOM_SOURCES];
  nslSourceId sourcesUnused[MAX_RANDOM_SOURCES];
public:
  void shutdown();
  nslSoundId getSoundId(void) { return thisSound;};
  void setProbability(float p);
  int getLastSourcePlayed();
  bool addSource(nslSourceId s);
  bool isPlaying();
  void stop();
  int  play();
  int   play3d(float x, float y, float z, float minDist, float maxDist);
  int  play(int index);
  void init();
};


class VOEngine 
{
public:
  // Initialize
  void init();
	void setCurrentLevel(int level);
	void stopVO();
	void playVO();

	void setCurrentSurfer(int index) { currentSurfer = index; };
	void pause() {};
	void unpause() {};
  // Get the state.. shouldn't be too necessary
  bool isPlaying();
	nslSoundId getCurrentSound() { return currentSound; }
  void frameAdvance(float timeInc);
	void resetTimer() { myTimer = 0;};
private:
  bool on, played;
	float myTimer;
	int whichVisit[BEACH_LAST];
	int currentSurfer, currentLevel;
	nslSourceId beachChallVO[LEVEL_LAST][NUM_DIFF_SURFERS];
  // OLD VO STUFF
  bool playing;
	nslSoundId currentSound;
};


extern VOEngine VoiceOvers;

#endif