#ifndef _ksreplay_h
#define _ksreplay_h

#include "po.h"
#include "wave.h"
#include "timer.h"
#include "floatobj.h"

//#define USE_POS

class KSEntityPO
{
  friend class KSReplay;
  friend class replay_camera;

private:

#ifdef USE_POS
  po KSPo;
  po KSBPo;
#else
  vector3d KSPos;       // Surfer position
  vector3d KSBPos;      // Board position
  quaternion KSRot;     // Surfer rotation
  quaternion KSBRot;    // Board rotation
  vector3d   BoardCurrent;
  vector3d   BoardMomentum;
#endif

public:
	KSEntityPO();
	void Save( int player );
	void Restore( int player );

  friend void interpolate_entity_po(const KSEntityPO &po1, const KSEntityPO &po2, KSEntityPO &res, float interp);
};


class KSEntityState
{
  friend class KSReplay;
  friend class KSEntityPO;
  friend class replay_camera;

  friend void interpolate_entity_po(const KSEntityPO &po1, const KSEntityPO &po2, KSEntityPO &res, float interp);
private:

  uint32 KSState          : 7;
  uint32 KSSuperState     : 4;
  uint32 KSAnim           : 10;
  uint32 KSBAnim          : 8;

  uint32 KSBlend          : 7;
  uint32 KSBBlend         : 7;
  uint32 KSCurTrick       : 7;


  uint32 KSAnimCall       : 1;
  uint32 KSBAnimCall      : 1;
  uint32 KSLoop           : 1;
  uint32 KSBLoop          : 1;
  uint32 KSWipedOut       : 1;
  uint32 KSInAir          : 1;
  uint32 KSInTube         : 1;
  uint32 KSDry            : 1;
  uint32 KSIKValid        : 1;
  uint32 KSWipeoutSplash  : 1;
  uint32 EndWave          : 1;

  uint32 padding          : 3;



public:
	KSEntityState();
	void Reset();
	void Save( int player );
	void Restore( int player );

  void SetKSAnimInfo(float blend, bool loop, float frame);
  void SetKSBAnimInfo(float blend, bool loop, float frame);
};


class KSReplayFrame
{
  friend class KSReplay;
  friend class replay_camera;

  float wave_shiftx;
  float levelTime;  
  float totalTime;  

public:
	KSReplayFrame();
  void Save();
};


class KSReplay
{
  friend class KSEntityState;
  friend class KSEntityPO;
  friend class DemoModeManager;
  friend class replay_camera;

  friend void interpolate_entity_po(const KSEntityPO &po1, const KSEntityPO &po2, KSEntityPO &res, float interp);

public:
  enum KSReplayStatus {
	  REPLAY_IGNORE,
	  REPLAY_RECORD,
	  REPLAY_PLAYBACK,
	  REPLAY_PAUSED
  };

  int bch, sfr, brd;

private:
  uint32 seed;                        // Random number seed, set at the start of the replay

	KSReplayStatus    status;           // Current status

  float             playtime;         // Current time of replay
  float             lastPlaytime;     // Playtime of last restored frame
	bool              slomo;            // Is replaying in slo-mo
	bool              fastforward;      // Is replaying in fast forward

  bool              prepareSlomo;     // Switch to slomo next playframe
  bool              prepareNormal;    // Switch to normal speed next playframe

  int               slomospeed;       // When in slomo, playspeed = 1/slomospeed;
  int               ffspeed;          // When in fast forward, playspeed = ffspeed;

	unsigned int      playframe;        // Current frame of replay
	int               lastPlayframe;    // Previous restored frame of replay
  int               interpFrame;      // Ranges from 0 to interpTicks-1

  unsigned int      mainPOFrame;      // Current frame in the mainEntityPO array
  unsigned int      aiPOFrame;        // Current frame in the aiEntityPO array

  int               frameTicks;       // Number of intervals of 1/60th of a sec between current frame and next
  int               interpTicks;      // Number of actual intervals between this frame and next

  bool              aiSurfer;         // True if there's an AI surfer
	unsigned int      numFrames;        // Total number of saved frames
	unsigned int      numMainPOFrames;  // Total number of position/orientation frames for the main surfer
	unsigned int      numAIPOFrames;    // Total number of position/orientation frames for the ai surfer
	KSReplayFrame     *frame;           // [maxframes];
  KSEntityState     *mainEntityState; // [maxframes]
  KSEntityState     *aiEntityState;   // [maxframes]
  KSEntityPO        *mainEntityPO;    // [maxframes / MAINENTITY_UPDATEFRAMES]
  KSEntityPO        *aiEntityPO;      // [maxframes / AIENTITY_UPDATEFRAMES]
	unsigned int      maxframes;        // Should be at least 10800 for a 3 minute replay. Amount varies with available memory free. Set in Init().

  bool              noDraw;           // Specifies to restore the frame, but not to render it to screen

  KSWaterState      initWaterState;   // Initial water state


  // Stores info about collisions between entities and beach_objects
  struct Collision
  {
    beach_object *obj;
    entity       *ent;
    vector3d      dir;
    float         timeStamp;
  } *collisions;                      // [MAXCOLLISIONS]

  int current_collision;
  int num_collisions;

  bool              firstFrame;

public:
	KSReplay();
	virtual ~KSReplay();

	void Init( bool aiSurfer );
	void Clear( uint32 seed );
	void Term();

  void LoadFile( char *fname );
  void SaveFile( char *fname );
  void SaveAnimIndex();
  void PeekFile(char *fname, int *board, int *surfer, int *beach, unsigned int *numFrames, unsigned int *mainPOFrames, unsigned int *aiPOFrames );

  void Tick( bool running, time_value_t time_inc  );
  bool IsPlaying();
  int NumSurfers();

  void SetKSAnimInfo(kellyslater_controller *kscont, float a, bool loop, float time);
  void SetKSBAnimInfo(kellyslater_controller *kscont, float a, bool loop, float time);
  void SetCollisionInfo(beach_object *obj, entity *ent, const vector3d &dir);
  void SetWipeoutSplash(int player);
  void SetEndWave();

  void ReplayFXUpdate(time_value_t time_inc);

	void Record();
	void Play();
	void Resume();
	void Restart();
	void Stop();
  void Pause(bool paused);
  void SpeedSlow();
  void SpeedNormal();
  void SpeedFast();

  float Playspeed();

  void SetStatus(KSReplayStatus s) {status = s;}
  KSReplayStatus GetStatus() {return status;}
	bool Done();

  bool NoDraw();

  int MainPOFrames();
  int AIPOFrames();
  int SloMoFrames();
  int FastForwardFrames();

  void MainEntityPos(vector3d &pos, int frame=-1);
  replay_camera::ReplayCamRegion MainEntityRegion(int frame=-1);

protected:
	void SaveFrame();
	void RestoreNextFrame(time_value_t time_inc);
};


extern KSReplay ksreplay;



#endif

