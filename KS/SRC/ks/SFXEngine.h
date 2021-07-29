#ifndef SFXENGINEH
#define SFXENGINEH
#include "VOEngine.h"
#include "entity.h"
#define THUNDER_DELAY                  .25
#define MAX_TUBE_SFX_DIST               5
#define SFX_VOLUME_ADJUSTMENT_FACTOR   .8
class SFXEngine 
{
private:
  RandomVO thunderSounds;
  RandomVO hitSurferSounds;
  RandomVO hitPier;
  RandomVO didGoal;
  nslSoundId dolphinGood;
  nslSoundId dolphinBad;
  RandomVO whaleSounds;
  RandomVO buoySounds;
  RandomVO randomSounds;
float randomTime;
  bool paused;
  float thunderTime;
  float volumeMod;
public:
  void pause() {paused = true;}
  void unpause() {paused = false;}
  void thunder();
  void playRain();
  void gotChallenge();
  void trickFailed();
  void trickSuceeded();
  void collided(entity *surfer, entity *hit);
	void sprayed(entity *spray);
  void init();
  void tick(float delta_t);
  void shutdown();
  void paddle(bool, float x, float y, float z, float volume);
};


extern SFXEngine sfx;
#endif