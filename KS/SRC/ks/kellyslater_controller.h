#ifndef _KELLYSLATER_CONTROLLER_H_
#define _KELLYSLATER_CONTROLLER_H_

#include "global.h"

#include "controller.h"
// BIGCULL #include "spiderman_common.h"
// BIGCULL #include "spiderman_controller.h"
#include "physics.h"
#include "board.h"
#include "waveenum.h"
#include "trick_system.h"
#include "scoringmanager.h"
#include "specialmeter.h"
#include "player.h"
//#if defined(TARGET_XBOX)
#include "camera.h"
#include "inputmgr.h"
#include "game.h"
#include "wave.h"
#include "ks_camera.h"
#include "ik_object.h"
//#endif /* TARGET_XBOX JIV DEBUG */
#include "SoundScript.h"
class trail;

typedef enum
{
  NO_HANDS_IN_WATER,
  LEFT_HAND_IN_WATER,
  RIGHT_HAND_IN_WATER,
  BOTH_HANDS_IN_WATER
};

#define NUM_STAND_ANIMS 6
#define NUM_CARVE_ANIMS 18
#define NUM_TURN_ANIMS 18


#define GRUNT_PROB (.4f)

#define MAC(a) a,
enum
{
#include "kellyslater_states_mac.h"
STATE_NUMSTATES
};
#undef MAC

enum
{
	WAVE_SECTION_TUBE,
};

enum
{
	SUPER_STATE_NO_SUPERSTATE,
	SUPER_STATE_FLYBY,
	SUPER_STATE_NORMAL_SURF,
	SUPER_STATE_WIPEOUT,
	SUPER_STATE_LIE_ON_BOARD,
	SUPER_STATE_PREPARE_JUMP,
	SUPER_STATE_AIR,
	SUPER_STATE_IN_TUBE, 
	SUPER_STATE_CPU_CONTROLLED, 
};

enum
{
	WIP_PHYS_DISMOUNT,
	WIP_PHYS_DISMOUNT2,	//  this dismount phase replaces WIP_PHYS_DISMOUNT when the dismount should use regular phyics code
	WIP_PHYS_FREEFALL,
	WIP_PHYS_IMPACT,
	WIP_PHYS_TUMBLE,
	WIP_PHYS_SWIM
};

enum
{
	WIP_INT_LOW,
	WIP_INT_MED,
	WIP_INT_EXT
};

enum SurferAnimations
{
#define MAC(a, b) a,
#include "kellyslater_shared_anims_mac.h"
_SURFER_NUM_SHARED_ANIMS,
_SURFER_NUM_START_SLATER_ANIMS = _SURFER_NUM_SHARED_ANIMS,
DUMMY_NUM = _SURFER_NUM_START_SLATER_ANIMS - 1,
#include "slater_ind_anims_mac.h"
_SURFER_NUM_SLATER_ANIMS,
_SURFER_NUM_START_ROBB_ANIMS = _SURFER_NUM_SLATER_ANIMS,
DUMMY_NUM_SLATER = _SURFER_NUM_START_ROBB_ANIMS - 1,
#include "robb_ind_anims_mac.h"
_SURFER_NUM_ROBB_ANIMS,
_SURFER_NUM_START_MACHADO_ANIMS = _SURFER_NUM_ROBB_ANIMS,
DUMMY_NUM_ROBB = _SURFER_NUM_START_MACHADO_ANIMS - 1,
#include "machado_ind_anims_mac.h"
_SURFER_NUM_MACHADO_ANIMS,
_SURFER_NUM_START_IRONS_ANIMS = _SURFER_NUM_MACHADO_ANIMS,
DUMMY_NUM_MACHADO = _SURFER_NUM_START_IRONS_ANIMS - 1,
#include "irons_ind_anims_mac.h"
_SURFER_NUM_IRONS_ANIMS,
_SURFER_NUM_START_FRANKENREITER_ANIMS = _SURFER_NUM_IRONS_ANIMS,
DUMMY_NUM_IRONS = _SURFER_NUM_START_FRANKENREITER_ANIMS - 1,
#include "frankenreiter_ind_anims_mac.h"
_SURFER_NUM_FRANKENREITER_ANIMS,
_SURFER_NUM_START_FLETCHER_ANIMS = _SURFER_NUM_FRANKENREITER_ANIMS,
DUMMY_NUM_FRANKENREITER = _SURFER_NUM_START_FLETCHER_ANIMS - 1,
#include "fletcher_ind_anims_mac.h"
_SURFER_NUM_FLETCHER_ANIMS,
_SURFER_NUM_START_CURREN_ANIMS = _SURFER_NUM_FLETCHER_ANIMS,
DUMMY_NUM_FLETCHER = _SURFER_NUM_START_CURREN_ANIMS - 1,
#include "curren_ind_anims_mac.h"
_SURFER_NUM_CURREN_ANIMS,
_SURFER_NUM_START_CARROL_ANIMS = _SURFER_NUM_CURREN_ANIMS,
DUMMY_NUM_CURREN = _SURFER_NUM_START_CARROL_ANIMS - 1,
#include "carrol_ind_anims_mac.h"
_SURFER_NUM_CARROL_ANIMS,
_SURFER_NUM_START_ANDERSEN_ANIMS = _SURFER_NUM_CARROL_ANIMS,
DUMMY_NUM_CARROL = _SURFER_NUM_START_ANDERSEN_ANIMS - 1,
#include "andersen_ind_anims_mac.h"
_SURFER_NUM_ANDERSEN_ANIMS,
#undef MAC
  _SURFER_NUM_ANIMS = _SURFER_NUM_ANDERSEN_ANIMS
};

#define NUM_TUBE_DIFFICULTY_LEVELS 10

class BalanceMeter
{
public:
	BalanceMeter();
	void Init(int player, bool vertical_meter, float base_time, float stat_bonus, bool reset = true);
	void SetPlayerNum(const int playerIdx) { player_num = playerIdx; }
	void AdjustStability(float stability) {time_to_full_acc *= stability;}
	float Update(int direction, const float time_step);
	void End();


private:

	float current_balance;  //  This controls what the surfer's current balance is.
	float balance_acc;  //  The speed at which the balance will change.
	float total_balance_time;
	float time_to_full_acc;
	bool vert_meter;
	int player_num;
};


class BigWaveMeter
{
public:

	BigWaveMeter() { player_num = 0; }
	void Init(int player, float base_time, int stat_bonus, bool reset = true);
	void AdjustStability(float stability) {time_to_full_acc *= stability;}
	float Update(int direction, const float time_step);
	void Resize(float new_size);
	void End();


private:

	float current_balance;  //  This controls what the surfer's current balance is.
	float balance_acc;  //  The speed at which the balance will change.
	float total_balance_time;
	float time_to_full_acc;
	int player_num;
};


enum BoardAnimations
{
#define MAC(a, b) a,
#include "board_anims_mac.h"
#undef MAC
  _BOARD_NUM_ANIMS
};

//------------------------------------------------------------
//
//		STATS.H
//
//------------------------------------------------------------

// table names
enum
{
	TURN_RATINGS,
	WORST_TURNS,
	BEST_TURNS
};


enum
{
	TURN_HEADING_CARVE,
	TURN_HEADING_GRAB,
	TURN_HEADING_SLIDE,
	TURN_HEADING_HARD_CARVE,
	TURN_HEADING_HARD_GRAB,
	TURN_HEADING_HARD_SLIDE,
	TURN_HEADING_SNAP,
	NUM_TURN_HEADINGS
};

enum
{
	TURN_SUBHEADING_TURN_VEL,
	TURN_SUBHEADING_BANK_ACCEL,
	TURN_SUBHEADING_BANK_VEL,
	TURN_SUBHEADING_BANK_ANGLE,
	NUM_TURN_SUBHEADINGS
};




//------------------------------------------------------------
//
//		controller related stuff
//
//------------------------------------------------------------
#define CUTOFF0	0.25f
#define CUTOFF1	0.75f

#define PRESS_UP_DOWN_CAMERA_LAG_TIME 0.75f


class turn_data
{
public:
	float TurnVel;
	float BankAccel;
	float BankVel;
	float Bank;
};


void register_kellyslater_inputs();
void map_kellyslater_inputs();

enum{STATE_STACK_MAX_DEPTH=32};

enum
{
	SPIN_180,
	SPIN_360,
	SPIN_540,
	SPIN_720,
	SPIN_MANUAL,
	SPIN_FLOATER,
	SPIN_NUM
};

class spin_controller
{
public:
  spin_controller();
  void frame_advance(float dt);
  vector3d SetSpinType(int type, float t = 0.0f, vector3d final_vec = ZEROVEC, float time_mult = 1.0f);
  int GetSpinType(void) { return spin_type; }
  void SetMyBoard(SurfBoardObjectClass *board) { my_board_controller = board; }
  void Reset(void) { activated = false; num_spins = 0; }
  void Pause(void) { activated = false; }
  void Continue(void) { activated = true; }
  bool IsActivated(void) { return activated; }
  void set_my_ks_controller(kellyslater_controller *ksc)  {my_ks_controller=ksc;}

#if defined(TARGET_XBOX)
  bool is_valid( void ) const;
#endif /* TARGET_XBOX JIV DEBUG */

private:
  int num_spins;
  float time;
  float depress_timer; //  used for canceling spins during 180's or highers
  int spin_type;
  float scale_factor;
  float spin_time[SPIN_NUM];
  bool activated;
  float spin_dir;
  vector3d spin_axis;
  SurfBoardObjectClass *my_board_controller;
  kellyslater_controller *my_ks_controller;
};

class auto_camera;
class big_wave_camera;
class duckdive_camera;
class wipeout_camera_2;
class photo_camera;

class FloatingText;

class kellyslater_controller : public entity_controller
{
public:
	enum { MAX_JUMP_ENT = 3 };
	static const float CLOCKTIME_DUCK_DIVE;

#if defined(TARGET_XBOX)
  bool is_valid( void ) const;
#endif /* TARGET_XBOX JIV DEBUG */

  BalanceMeter tube_meter;
  //BalanceMeter float_meter;
  //BigWaveMeter big_wave_meter;

  void start_secondary_cam(camera *cur_cam);
  void end_secondary_cam();
  bool get_ik_valid() { return IK_enabled; };
  void set_ik_valid(bool valid) { IK_enabled = valid; };
  trail *my_trail;
rational_t get_floor_offset(void);
protected:
  friend class surfer_ai_goal;
  friend class KSReplay;
  friend class IGOIconManager;

private:
  // initialization stuff
  void SetTurnStat(int table,int heading, float turnVel, float bankAccel, float bankVel, float Bank);
  void CalcTurnStats(turn_data * td, int heading);
  void SetupCameras(void);

  void SetWorldLimits();
  void SetMyRatings();
  void CalculateStats();

  void CalcTransform(vector3d & rot, vector3d & pos);

  void DoWipeoutPhysics(float dt);
  void DoWipImpactSoundStuff(void);

  void SetTrickRegion(const TRICKREGION r);

	// The surfer state
	int state;
	int last_state;
	int super_state;
	int last_super_state;
	int dummy_state;
	vector<int> state_stack;

	int initial_face_trick_region;

	int stand_anim_nums[NUM_STAND_ANIMS];
	int carve_anim_nums[NUM_CARVE_ANIMS];
	int turn_anim_nums[NUM_TURN_ANIMS];

  TRICKREGION	trickRegion;		// the surfer's current trick region
  TRICKREGION	prevTrickRegion;	// surfer's trick region in the last frame

  // turn variables
  turn_data carve;
  turn_data hardCarve;
  turn_data grab;
  turn_data hardGrab;
  turn_data slide;
  turn_data hardSlide;

  float tube_start_time;  //  When this reaches a certain threshold, switch to tube controls.

  int take_off_type;
  bool takeoff_wobble;

  vector3d last_tube_pos;
  float last_tube_dist;
  bool out_of_tube;
  bool last_was_main_tube;
  bool floater_gap;

  // Physical Info
  vector3d			pos;
  vector3d			rot;

  vector3d			rotVel;		// current orientation angle velocities
  vector3d			rotAccel;	// current orientation angle accelerations
  vector3d			rotDest;	// destination orientation angle
  vector3d			rotDestVel;	// destination orientation angle velocities

  float			wavePitch;
  float			waveBank;
  float			waveRegion;

  float			speed;

  // do I ride with the wrong feet ?
  bool goofy;

  float hold_time;  //  this is time to hold a grab trick in the air when player releases buttons
					//  usually determined by time to air path apex
  float hold_timer;  //  time the trick has been held

  float trick_timer;
  float complete_time;
  bool first_trick;

  float junk_time;
  float junk_timer;
  bool junk_combo_pressed;

  float t_spin_hold;

  bool switch_cam_pressed_already;  //  Only allow changing cameras exactly when the button is first pressed.

  // Animation Info
  //AnimStructPtr	anims[MAX_ANIM_NUM];
  int				lastAnim;
  float     lastTime;

  // These have been moved from CItem.
  //short	mFrame;
  short	mAnim;
  short mLastAnim;
  char  mAnimDir;
  //bool  mAnimFinished;
  bool  ignore_tweening;
  short bAnim;  //   current board animation

  bool end_level;

#ifdef _DEBUG
  int curr_anim_enum;
#endif

  // Stats
  float turnRatings[NUM_TURN_HEADINGS][NUM_TURN_SUBHEADINGS];
  float worstTurns[NUM_TURN_HEADINGS][NUM_TURN_SUBHEADINGS];
  float bestTurns[NUM_TURN_HEADINGS][NUM_TURN_SUBHEADINGS];

  SurfBoardObjectClass my_board_controller;
  entity * my_board;
  entity * my_board_model;
  entity * my_board_member;
  entity * my_rotate_object;
  entity * my_parent_node;

  TrickManager		my_trickManager;
  ScoringManager	my_scoreManager;
  CarveManager		my_carveManager;
  SpecialMeter		specialMeter;

  int my_player_num;        // which player I am (0 thru MAX_PLAYERS-1)
  int surfer_num;			//  this is used as a reference into the attributes array of g_surfer_attribs

private:
  // Ex userfunc functions
  void RotationVel(int angleId, float vel);
  void RotationTo(int angleId, float angle, float vel, float accel);
  float GetRotation(int angleId);
  //void SetAccel(float a);
  void debug_mode_play_anim(void);

  int GetTurnCycle(float degree);

  WaveRegionEnum WaveRegion();
  bool IsInsideWave();

  void process_controls(float dt);
  void update(float dt);
  void downdate(float dt);

  // Pseudo states
  void TurnDegree();
  void TurnType();
  void Spin(float t);

  int GetAnalogTurn(void);

  void SuperStateFlyby(float dt);
  void SuperStateLieOnBoard(float dt);
  void SuperStatePrepareJump(float dt);
  void SuperStateNormalSurf(float dt);
  void SuperStateAir(float dt);
  void SuperStateWipeout(float dt);
  void SuperStateInTube(float dt);
  void SuperStateCPUControlled(float dt);

  void StateAirTrick(float dt);
  void Freefall(float dt);

  void InitializeSurfer(stringx hero_name);

  int StandAnim(void);
  void DetermineTakeoffType(bool weak = false);
  void DetermineTubeGaps(void);

  bool CheckForRoof(void);
  void SetSurferAttribs(void);

  bool DoFaceTrick();

  // Internal state variables
private:
	SSEventId floaterEvent, stallEvent, carveEvent,spinEvent,rideEvent;
  //   IK variables
  bool IK_enabled;
  ik_object *my_ik_object;
  //   end IK variables

  int turnType;
  int lastTurnType;
  int changeTurnType;
  int turn_index;  //  animation index for analog turns
  float stick, degree;
  int last_region;
  int current_region;

  bool bSpecialTrick;

  float	rideTimer;		// length of time surfer has not wiped out.

  /*
  void SetupTurn();
  void SetupRegularTurn();
  void SetupGrabTurn();
  void SetupTailslideTurn();
  void SetupSnapTurn();
  */

  // add a little spring to the legs
  float last_position;
  float last_offset;

  // for the wipeouts
  vector3d velocity;
  WavePositionHint wave_hint;
  bool wave_hint_valid;

  int currentTrick;
  int completedTrick;
  int newTrick;
  int airIKtrick;
  bool trick_complete;
  bool manual;
  bool current_trick_type;
  bool trick_queued;
  bool bDoingTrick;

  vector3d wave_norm;
  vector3d wave_norm_lf;

  bool exit_state;

  float press_up_time;  //  A timer to make sure that the camera doesn't immediatel swing around when pressing up then down.

  spin_controller spin_ctrl;

  //  surfer physics state(s) for wipeouts
  vector3d surfer_velocity;
  vector3d wip_position;
  vector3d water_current;
  vector3d water_normal;
  WaveRegionEnum water_region;
  int wip_phys_state;
  int wipeout_type;
  float low_wip_y_height;
  int get_on_board_anim;
  int b_get_on_board_anim;

  int num_wipeouts;  // this is purely for info for returning to frontend
  bool no_buttons_pressed;  //  used for same thing
  
  int wip_anim_dis;
  int wip_anim_free;
  int wip_anim_imp;
  int wip_anim_imp2;
  int wip_anim_trans;
  int wip_anim_swim;
  int b_wip_anim_dis;
  int b_wip_anim_free;
  int b_wip_anim_imp;

  int goofy_int;
  bool dry;
  bool left_hand_dry;
  bool right_hand_dry;

  bool perform_snap;
  vector3d current;

  int float_quadrant;
  int float_trick;
  int float_anim;
  int float_board_anim;
  float float_time;
  vector3d grind_vector;
  vector3d grind_right;
  float float_balance;

  float grind_jump_timer;
  float slide_dismount_timer;
  float float_region_timer;

  float duckdive_timer;
  float tween_timer;
  float tween_time;

  vector3d force_dir;	//  to be used to add a force in a determined direction

  int tube_trick;
  int tube_anim;
  float tube_balance;
  int last_balance_anim;
  bool last_need_extra_balance;
  int last_direction;
  int tube_board_anim;
  int last_tube_trick;
  float tube_trick_time_so_far;
  float tube_ride_time;
  int just_started;	//  This is set as true until the user has touched the controls.
  bool left_stick_pressed;
  bool oscilate_up;
  bool passed_first_tube_thresh;
  bool passed_second_tube_thresh;
  bool passed_tube_difficulty_level[NUM_TUBE_DIFFICULTY_LEVELS];
  float out_of_tube_time;
  float current_trick_time;
  float_filter depth_meter_filter;

  bool possible_exit_jump;
  int stand_num;

  entity *jumped_entities[MAX_JUMP_ENT];
  int num_jumped__entities;

  device_id_t joystick_num;          // which joystick this controller is attached to

  float attribs[4];

  bool shakeoff;
  bool trick_added;

	bool flyby_first;
	bool flyby_key_down;

  game_camera *player_cam;

  // Moved here from wds.h
  // Now each player has his own cameras.
  beach_camera* beach_cam_ptr;
  auto_camera* auto_cam_ptr;
  big_wave_camera* big_wave_cam_ptr;
  debug_camera* ksdebug_cam_ptr;
  look_back_camera* look_back_cam_ptr;
  old_shoulder_camera* old_shoulder_cam_ptr;
  shoulder_camera* shoulder_cam_ptr;
  stationary_camera* stationary_cam_ptr;
  flyby_camera* flyby_cam_ptr;
  wipeout_camera* wipeout_cam_ptr;
  wipeout_camera_2* wipeout_cam2_ptr;
  follow_camera* follow_cam_ptr;
  follow_close_camera* follow_close_cam_ptr;
  buoy_camera* buoy_cam_ptr;
  duckdive_camera* duckdive_cam_ptr;
  fps_camera* fps_cam_ptr;
  photo_camera* photo_cam_ptr;

  bool did_celebration;	
  bool done_scoring;

public:
  void SetConglomTexture(entity* c, int text);

  entity *get_my_board_model(){return my_board_model;}
  bool isDry() {return dry;};
  float Lip_Distance();
  kellyslater_controller(int hero_num, entity *ent);
  virtual ~kellyslater_controller();

  virtual void frame_advance(time_value_t t);

  void Anim(int animid, float blend_time, bool loop = true, float time=0.0f, bool reverse = false );
  void BoardAnim(int animid, float blend_time, bool loop = true, float time=0.0f );
  float GetAnimPercentage();
  float GetAnimTime();
  float GetAnimDuration();
  float GetBoardAnimDuration();
  float GetBoardAnimTime();
  bool AnimComplete();
  bool BoardAnimComplete();
  bool AnimLooped();
  bool AnimBlended();
  int GetAnim(void) { return mAnim; }
  entity * GetBoardModel(void) { return my_board_model; }
  entity * GetBoardMember(void) { return my_board_member; }
  void SetPlayerCamera(game_camera *cam);
  void AlignSurferWithBoard(float dt);
  void OrientToWave(bool upVec, float dt, int flag);

  bool EndLevel(void) { return end_level; }
  void SetEndLevel(bool end) { end_level=end; }
  game_camera* GetPlayerCam(void) { return player_cam; }
  void UpdateHand(void)
  {
	if (goofy)
	{
		get_owner()->UpdateHandedness();
		my_board_model->UpdateHandedness();
	}
  }

  bool IsGoofy(void) { return goofy; }
  bool IsDoingTrick(void) { return bDoingTrick || bSpecialTrick; }
  bool IsDoingSpecialTrick(void) { return bSpecialTrick; }
  void SetDoingSpecialTrick(bool set) { bSpecialTrick = set; }

  void SetNewTrick(const int trickIdx);
  void SetCompletedTrick(void);
  void SetCompletedTrick(int trick);
  void SetCurrentTrick(void);
  void ResetTricks(void);
  void ClearTricks(void);
  void SetFloatTrick(int trick, int anim, int banim)
  {
	  float_trick = trick;
	  float_anim = anim;
	  float_board_anim = banim;
  }

  bool IsFloaterLanding(void);
  bool IsPlayingFloatAnim(void)
  {
	  return ((mAnim <= (float_anim + 5)) && (mAnim >= (float_anim -5)));
  }

  bool StartFlyby();

  bool Z_Within_Tube();
  void Tube_Align(float z_offset = 0.0f, float spin_rate = 5);
  void StartTube();
  void EndTube();
  void SetTubeTrick(int trick, int anim, int banim);
  int IsTubeHandInWater();
  bool IsAIPlayer();
  void SetFloatPO(float angle);
  void StartGrind(const vector3d direction);
  void StartFloat(const vector3d direction);
  vector3d GetGrindVector(void) { return grind_vector; }
  vector3d GetGrindRight(void) { return grind_right; }
  float GetFloatBalance(void) { return float_balance; }
  bool CheckGrindPathEnd(void);

  void check_celebration(void);
  void StartCelebration(void); 
  void StartDisappointment(void);
  bool IsCelebrationDone(void) { return did_celebration; }
  bool IsScoringDone(void) { return done_scoring; }

  bool IsUpDownThreshDone() {return press_up_time > PRESS_UP_DOWN_CAMERA_LAG_TIME;}

  bool TrickComplete(void) { return trick_complete; }
  int GetCompletedTrick(void) { return completedTrick; }
  int GetCurrentTrick(void);
  TRICKREGION GetTrickRegion(void) const { return trickRegion; }
  float Tube_Distance();
  float Closest_Tube_Distance(int *main_tube = NULL, vector3d *closest_point = NULL, float x_pos = 1000000);

  float GetRideTime(void) const { return rideTimer; }

  void reset_state ();

  void Reset();

  void set_state (int new_state)
  {
    last_state = state;
    state = new_state;
  }

  void set_super_state(int new_state)
  {
	  last_super_state = super_state;
	  super_state = new_state;
  }
  int get_last_super_state()
  {
    return last_super_state;
  }
  void set_wave_hint (WavePositionHint hint)
    { wave_hint = hint; }

  int get_current_state () const
    { return state; }
  int get_last_state () const
    { return last_state; }
  int get_super_state () const
	{ return super_state; }

  SurfBoardObjectClass & get_board_controller(void) { return my_board_controller; }
  ScoringManager & get_my_scoreManager(void) { return my_scoreManager; }
  CarveManager & get_my_carveManager(void) { return my_carveManager; }
  SpecialMeter * get_special_meter(void) { return &specialMeter; }

  int get_player_num(void) { return my_player_num; }
  void set_player_num(const int n);

  void SetShakeOff(bool shake) { shakeoff = shake; }

  // can you say 'hack'? I knew you could...
  void JustToKeepUpdatePrivate(float dt) { downdate(dt); }

  void ResetPhysics( void ) { my_board_controller.ResetPhysics(); Reset(); }
  virtual void OnNewWave( void ) { my_board_controller.OnNewWave(); Reset(); }

  const po& get_owner_po( void ) { return get_owner()->get_rel_po(); }
  void set_owner_po( const po& p ) { get_owner()->set_rel_po(p); }

  entity * GetBoard(void) { return my_board; }
  int GetCurrentAnim( void ) { return mAnim; }
  int GetCurrentBoardAnim( void ) { return bAnim; }
  float GetCurrentFrame( void ) { return GetAnimTime(); }
  void SetAnimAndFrame( int a, int b, float time, float blenda = 0, float blendb = 0, bool KSLoop = false, bool BLoop = false, bool KSAnimCall = false, bool BAnimCall = false);
  void SetJumpedEntity(entity *ent);

  device_id_t get_joystick_num()        {return joystick_num;}
  void set_joystick_num(device_id_t jn) {joystick_num=jn;}

  // CtrlEvnt() and CtrlDelta() serve as substitutes to inputmgr::get_control_state(), get_control_delta()
  float CtrlEvent(int control);
  bool  CtrlDelta(int control);
  float GetStick(int control);

  float GetAttribs(int attr);

  void ResetJumpedEntities(void)
  {
    for (int w = 0; w < MAX_JUMP_ENT; w++)
      jumped_entities[w] = NULL;
    num_jumped__entities = 0;
  }

  void SetWaveNorm(vector3d vec) { wave_norm_lf = wave_norm; wave_norm = vec; }
  void InitWipeout(int wip_type);
  void InitKlugeWipeout(int tumble_anim, int swim_anim);
  int GetWipeoutPhysState(void) { return wip_phys_state; }
  int GetWipeoutIntensity(void);
  bool IsDoingSomething(void);

  int GetNumJumpedEntities(void) const { return num_jumped__entities; }
  entity * GetJumpedEntity(const int idx) { return jumped_entities[idx]; }
  
  wipeout_camera* get_wipeout_cam_ptr() { return wipeout_cam_ptr; }
  wipeout_camera_2* get_wipeout_cam2_ptr() { return wipeout_cam2_ptr; }
  flyby_camera* get_flyby_cam_ptr() { return flyby_cam_ptr; }
  debug_camera* get_debug_cam_ptr() { return ksdebug_cam_ptr; }
  photo_camera * get_photo_cam_ptr(void) { return photo_cam_ptr; }
  fps_camera * get_fps_cam_ptr(void) { return fps_cam_ptr; }
  beach_camera * get_beach_cam_ptr(void) { return beach_cam_ptr; }
  follow_close_camera * get_follow_close_cam_ptr(void) { return follow_close_cam_ptr; }

  void PerformIK(void);

#ifdef DEBUG
	void draw_debug_labels();
	FloatingText *anim_label;
	FloatingText *state_label;
#endif

};


//------------------------------------------------------------
//
//		SURFER.H
//
//------------------------------------------------------------

// turn angles
enum
{
	TURN=1,
	BANK=2,
	PITCH=3
};

// bit numbers for game events
enum
{
	ANIM_END_EVENT,
	SECTION_CHANGE_EVENT,
	RECALC_STATS_EVENT,
};

enum
{
	RIGHT_RAIL,
	LEFT_RAIL
};


enum
{
  PHYSICS_CONTROL,
  ANIMATION_CONTROL,
};


enum
{
  PART_CENTER = 0,
  PART_RIGHT0 = 1,
  PART_RIGHT1 = 2,
  PART_RIGHT2 = 3,
  PART_LEFT0 = -1,
  PART_LEFT1 = -2,
  PART_LEFT2 = -3
};

#define BLEND_TIME	0.3f

#define TRICK_T	90


//------------------------------------------------------------
//
//		TURNSPEC.H
//
//------------------------------------------------------------

//////////////////////////// in degrees
#define REG_TURN_VEL			5.00
#define REG_BANK_ACCEL			0.20
#define REG_BANK_VEL			1.00
#define REG_BANK				20.00

#define HARD_REG_TURN_VEL		10.00
#define HARD_REG_BANK_ACCEL		0.40
#define HARD_REG_BANK_VEL		2.00
#define HARD_REG_BANK			40.00

#define GRAB_TURN_VEL			10.00
#define GRAB_BANK_ACCEL			0.40
#define GRAB_BANK_VEL			2.00
#define GRAB_BANK				20.00

#define HARD_GRAB_TURN_VEL		20.00
#define HARD_GRAB_BANK_ACCEL	0.80
#define HARD_GRAB_BANK_VEL		4.00
#define HARD_GRAB_BANK			40.00

#define SLIDE_TURN_VEL			10.00
#define SLIDE_BANK_ACCEL		0.05
#define SLIDE_BANK_VEL			0.50
#define SLIDE_BANK				5.00

#define HARD_SLIDE_TURN_VEL		20.00
#define HARD_SLIDE_BANK_ACCEL	0.10
#define HARD_SLIDE_BANK_VEL		1.00
#define HARD_SLIDE_BANK			10.00

#define SNAP_TURN_VEL			10.00
#define SNAP_BANK_ACCEL			0.0781
#define SNAP_BANK_VEL			0.3125
#define SNAP_BANK				5.00
////////////////////////////

#endif
