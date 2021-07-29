#ifndef __RUMBLEMANAGER
#define __RUMBLEMANAGER

#include "global.h"
#include "singleton.h"
#include "joystick.h"

#define MIN_TUBE_DIST .1f
#define MAX_TUBE_DIST -1.3f


#ifdef TARGET_PS2
#include "hwosps2/ps2_input.h"
extern ps2_joypad_device *g_pad;
#elif TARGET_XBOX
#include "hwosxb/xb_input.h"
extern xb_joypad_device *g_pad;
#elif TARGET_GC
#include "hwosgc/gc_input.h"
extern gc_joypad_device *g_pad;
#endif

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "waveenum.h"
#endif /* TARGET_XBOX JIV DEBUG */

#define RUMBLE_PADS 4

class rumbleManager
{
public:
  
  typedef enum {
    LANDING = 0,
    IN_AIR,
    WIPING_OUT,
    UNDERWATER,
    GRINDING_OBJECT,
    FLOATER,
    IN_WASH,
    LIE_ON_BOARD_POCKET,
    LIE_ON_BOARD_FACE,
    LIE_ON_BOARD_CHIN,
    STANDING_NEAR_TUBE,
    RUMBLE_NONE,
    RUMBLE_STATE_END
  } RumbleState;

  static const char regionNames[RUMBLE_STATE_END][30]; 
    


  RumbleState currentRumbleState[2];
  RumbleState lastRumbleState[2];
  bool drawState;

  float rumbleLevels[RUMBLE_STATE_END];
  float rumbleVarPeriods[RUMBLE_STATE_END];
  float rumbleVarAmplitudes[RUMBLE_STATE_END];
  float rumbleFreqs[RUMBLE_STATE_END];

  rumbleManager();
  void toggleDrawState()
  {
    drawState = !drawState;
  };
  void drawCurrentState();
  void shutdown();
  void init();
  bool isOn(int controller);
  void turnOn(bool on_p, int controller);
  ~rumbleManager();
  void tick(float delta_t);
  void pause();
  void unpause();
  void writeLevels();
private:
  float variancePeriod;
  float varianceAmplitude;

  float currentStateTime[2];
  bool on[RUMBLE_PADS];
  bool paused;
  float rumbleLevel, rumbleFreq;
  int ks_state[2], ks_laststate[2];
  WaveRegionEnum currentRegion[2], lastRegion[2];
};

extern rumbleManager rumbleMan;
#endif
