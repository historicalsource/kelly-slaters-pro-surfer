#ifndef _WAVESOUNDH_
#define _WAVESOUNDH_
#include "ksnsl.h"
#include "waveenum.h"

#define MAX_REGION_SOUNDS 8

#if defined(TARGET_XBOX)
#include "wave.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "ngl.h"
#include "VOEngine.h"



class WaveSound 
{
private:
 
  // The new, NAMED variables!  Ooooh!  Aaah!
  nslSoundId    AddInSounds[MAX_REGION_SOUNDS * 3];
  float         TargetSoundVolumes[MAX_REGION_SOUNDS * 3];

  nslSoundId    DropOutSounds[MAX_REGION_SOUNDS * 3];
  
  nslSoundId    ambtrk;
  nslSourceId   ambsrc;
  
  nslSoundId    lip1Snd;
  nslSoundId    crash1Snd;
  nslSoundId    waveCrashSnd;
  nslSoundId    waterLapSnd;
  
  nslSoundId    underwaterSnd1;
	nslSoundId    underwaterSnd2;
	nslSoundId		crowd;
  //nslSoundId    bailSnd[2];

  int           numTubeSounds;
  nslSoundId    tubeSnd[2][MAX_REGION_SOUNDS/2];
  nslSoundId    muffleSound;

  int           numFaceSounds;
  nslSoundId    faceSnd[2][MAX_REGION_SOUNDS/2];

  int           numFoamSounds;
  nslSoundId    foamSnd[2][MAX_REGION_SOUNDS/2];
  int           lastTubePieces;
  float         faceModifier;
  float         tubeModifier;
  float         foamModifier;
  float         lapModifier;
  float         underWaterModifier;
  float         crashModifier;
  float         muffleVolume;
  float         ambVol;
	float					dampenval;

  nslEmitterId  foamEmitters[2][MAX_REGION_SOUNDS/2];
  nslEmitterId  tubeEmitters[2][MAX_REGION_SOUNDS/2];
  nslEmitterId  faceEmitters[2][MAX_REGION_SOUNDS/2];

  nslSourceId     foamSrc[2];
  nslSourceId     tubeSrc[2];
  nslSoundId      muffleSrc;
  nslSourceId     faceSrc[2];

  nslSourceId     lip1Src;
  nslSourceId     crash1Src;
  nslSourceId     underwaterSrc;
  nslSourceId     waveCrashSrc;
  nslSourceId     waterLapSrc;
  
  nlVector3d      tubePos[2][MAX_REGION_SOUNDS/2];
  nlVector3d      facePos[2][MAX_REGION_SOUNDS/2];
  nlVector3d      bailPos[2][MAX_REGION_SOUNDS/2];
  nlVector3d      foamPos[2][MAX_REGION_SOUNDS/2];

  nlVector3d      lip1Pos;
  nlVector3d      crash1Pos;
  nlVector3d      waveCrashPos;
  

	SSEventId				underwaterEventFront;
	SSEventId				underwaterEventRear;

  // The old unnamed variables BOOO!
  nslSoundId    waveSounds[10];
  nslSourceId   waveSources[10];

  // Some still used
  nslEmitterId  waveEmitters[10];


  
  // Some state vars
  bool underwater;
  bool initialized;
  bool is_paused;
  bool crashed;
  bool lastCrashState;
  float delta;
  int rampCount;
  void SetFaceEmitterPositions();
  void SetTubeEmitterPositions();
  void SetFoamEmitterPositions();
  //void findBreakPoint(void);                          // Finds the crash point
  void rampUp( float totalTime, float lastDelta );      // Ramp sounds up in volume
  void rampDown( float totalTime, float lastDelta );    // Ramp sounds down in volume
  /*nslSoundId AddInWaveSound( nslSourceId src, float target );
  void DropOutWaveSound( nslSoundId snd );
  void ManageRampingSounds( float dt );
  */
  void setSoundVolumes(void);
  // These next three procedures calculate the position(s) of the sound(s) for
  // different regions.  
  //void positionEmitterOnLine(nglVector out, nglVector surferPos, nglVector endpt1,nglVector endpt2);
  //void foamSoundPosition(nglVector out1, nglVector surferPos, nglVector endpt1, nglVector endpt2);
  //void faceTubeSoundPositions(nglVector out1, nglVector out2, nglVector surferPos, nglVector surferForward , nglVector endpt1, nglVector endpt2);

public:  
	nslEmitterId	behindTheCamera;
  int perspective;
  static const int CAMERA_PERSPECTIVE=0;
  static const int SURFER_PERSPECTIVE=1;
  static const int HYBRID_PERSPECTIVE=2;

  kellyslater_controller* wet;
  void wentUnderWater(bool under);
  void shutdown();
  void init();
  void OnNewWave();
  void pause(); 
  void unpause();
	void dampen(float val) { dampenval = val; };
  void undampen() { dampenval = 1.0f; }
	bool isdampened() {  return dampenval != 1.0f; }
	void mute();
  void unmute();
#ifdef DEBUG	// On XBox nglSphereMesh exists only in DEBUG
  void drawSoundPoints();
#endif
  void frameAdvance(float deltaTime);
  bool showSpheres;
  int   waveDir;

  // Parameters for waves
  // Volumes
  float faceVolume;
  float underWaterVolume;
  float tubeVolume;
  float foamVolume;
  float crashVolume;
  float waveCrashVolume;

  // Ranges
  float foamMin;
  float foamMax;
  float waterLapVolume;
  float faceMin;
  float faceMax;
  float waveCrashMin;
  float waveCrashMax;
  float tubeMin;
  float tubeMax;
};


extern WaveSound wSound;

#endif
