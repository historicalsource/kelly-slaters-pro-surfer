// AISurferController.cpp
// This is a class that acts as a controller to control an 
// AI player.
// All AI should be done in the poll procedure, and be used 
// to set the controls for the next frame

#include "inputmgr.h"
#include "kellyslater_controller.h"

class AISurferController :  public input_device 
{
public:
  AISurferController();
  virtual ~AISurferController() {};

  
  // None of these are used
  virtual void vibrate( float intensity ) { };
  virtual void vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc ) { };
  virtual void stop_vibration() {};

  virtual bool is_vibrator_present() const { return false;};
  bool is_inserted() { return true; }
  bool is_port_open() const { return 1; }
  virtual bool is_connected() const { return 1; };
  void record_demo_start(const stringx &filename) { assert(0);};;
  void record_demo_stop() { assert(0);};
  void playback_demo_start(const stringx &filename) { assert(0);};
  void playback_demo_stop() { assert(0);};


  // All the real stuff
  virtual void poll();
  virtual stringx get_name() const { return stringx("ai_player_ctlr");};
  virtual stringx get_name( int axis ) const { return stringx("ai_player_ctlr_axis"); };
  virtual device_id_t get_id() const { return device_id; }; 

  virtual int get_axis_count() const;
  virtual axis_id_t get_axis_id(int axis) const;

  virtual rational_t get_axis_state( axis_id_t axis, int control_axis ) const;
  virtual rational_t get_axis_old_state( axis_id_t axis, int control_axis ) const;
  virtual rational_t get_axis_delta( axis_id_t axis, int control_axis ) const;

private:


  typedef enum {
    STATE_STANDING,
    STATE_CHASE,
    STATE_DO_TRICK,
    STATE_AVOID_SURFER,
    STATE_BOUNCE_AROUND_WAVE,
    STATE_LAST_STATE
  } AI_SURFER_STATE;

  typedef enum {
    NO_SUBSTATE,
    TRICK_APPROACH,
    TRICK_LAUNCH,
    TRICK_DOING_TRICK,
    TRICK_LANDING,
    TRICK_DONE,
    STATE_LAST_SUBSTATE
  } AI_SURFER_SUBSTATE;
  // A couple of vectors:
  //   target - direction to target
  //   toTube - the vector from the mouth of the tube to the interior
  //   right  - the right vector for the ksctl
  //   velocity - our velocity vector
  //   dir    - where we are going
  vector3d target, toTube, right, velocity, dir;
  // The direction we should paddle
  vector3d paddleDirVec;
  AI_SURFER_STATE myState;
  AI_SURFER_SUBSTATE mySubstate;
  
  // offdh - how far OFF our Direction  is from our Heading (a dot product)
  // diffAngle - angle between dir and heading
  // paddleAngle - how far off the desired paddle vector we are
  // lastToTargetDistance - distance to target
  
  float offdh, diffAngle, paddleAngle, toTargetDistance, lastToTargetDistance, relativeVelocityToTarget, lastRelativeVelocityToTarget, toTubeDist;
  
  // Velocity magnitudes in the direction of the target
  float last_toward_tube_velocity, this_toward_tube_velocity;
  // Where we are going (target - ourPos)
  vector3d heading;
  // Our aceleration values
  float acel, oldAcel;
  // Our corresponding kellyslater_controller 
  kellyslater_controller *ksctl;
  bool hitRegionPocket;
  
  // Buttons
  float X, Y, XRight, YRight;
  bool ButtonX, ButtonO, ButtonSq, ButtonTr;
  bool ButtonL1, ButtonL2, ButtonL3, ButtonR1,ButtonR2,ButtonR3;
  bool ButtonSelect,ButtonStart;

  // The olf values of the buttons
  float oldX, oldY, oldXRight, oldYRight;
  bool oldButtonX, oldButtonO, oldButtonSq, oldButtonTr;
  bool oldButtonL1, oldButtonL2, oldButtonL3, oldButtonR1, oldButtonR2, oldButtonR3;
  bool oldButtonSelect,oldButtonStart;

  // To check for collisions be between the player and the AI
  void checkCollisions();

  // calcs the sign for the X value to turn down the wave
  int  downWaveSign();

  //rational_t get_axis_state( axis_id_t axis, unsigned char *rdata ) const;  
  // Sets up some vars for the frame
  void       setupStateVars();
  // Stand up on the board
  bool       doStandUp();
  // Start chasing
  void       moveToTarget();
  void       clearButtons();
  bool       doTrick();
  int        curTrick;
  void       pressButton(int which);
  void       releaseButton(int which);
  void       pressDir(int which, float howmuch);
  bool       buttonStatus(int which);
};