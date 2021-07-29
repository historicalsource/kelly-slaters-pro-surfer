#ifndef __KSPS_BOARD_H_
#define __KSPS_BOARD_H_

#include "wave.h"
#include "physics.h"
//class SimpleForce;
class PhysicsObjectClass;
typedef class PhysicsObjectClass *PhysicsObjectClassPtr;

// New KellySlater Stuff PTA 4/22/01
#include "entity.h"
//#include "floatobj.h" -- please use forward references
class kellyslater_controller;
class water_object;

#define BOARD_THICKNESS	0.1f

#define M_RAD 		 0.0174532925199432957692369076848861f
#define BOARD_PITCH_UP	(3.00f * M_RAD)	// <- in degrees

enum TurnIndicesEnum
{
	REGULAR,
	HARD,
	GRAB,
	HARD_GRAB,
	CROUCH,
	HARD_CROUCH,
	GRAB_CROUCH,
	HARD_GRAB_CROUCH,
	REGULAR_TAILSLIDE,
	HARD_TAILSLIDE,
	GRAB_TAILSLIDE,
	HARD_GRAB_TAILSLIDE,
	CROUCH_TAILSLIDE,
	HARD_CROUCH_TAILSLIDE,
	GRAB_CROUCH_TAILSLIDE,
	HARD_GRAB_CROUCH_TAILSLIDE,

	MAX_TURN_TYPES,
};

//\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/

#include "physics.h"

typedef enum {
	REGULAR_TURN,
	HARD_REGULAR_TURN,
	GRAB_TURN,
	HARD_GRAB_TURN,
	TAILSLIDE_TURN,
	GRABSLIDE_TURN,
	HARD_TAILSLIDE_TURN,
	SNAP_TURN,
	HARD_SNAP_TURN,
	TAILCHUCK_TURN,
	REBOUND_TURN,
	LAYBACK_SLIDE_TURN,
	GOUGE_TURN,
	REVERT_CUTBACK_TURN,
	CROUCH_TURN,
	CARVE_TURN,
	TRIM_TURN,
	HARD_TRIM_TURN,
	TRICK_SPIN,
	HARD_TRICK_SPIN,
	LIE_TURN,

	NUM_TURN_TYPES
} TurnTypeEnum;

typedef struct TurnPropertiesStruct
{
	float maxLeanAngle; // how much the board leans into the turn
	float maxTurnAngle; // how much the board turns
} TurnPropertiesType;

typedef enum {
	BOARD_NONE,
	BOARD_GRIND,
	BOARD_FLOAT,
	BOARD_FORWARD,
	BOARD_TURN_RIGHT,
	BOARD_TURN_LEFT,
	BOARD_ANIM_CONTROL,
	BOARD_JUMP,
	BOARD_IN_AIR	//  this will only be set during some wipeouts and some exit moves
} BoardStateEnum;

class SurfBoardObjectClass { //: public entity {
friend class KSReplay;
public:	
	static const int LANDING_FLAG_PERFECT;
	static const int LANDING_FLAG_SLOPPY;
	static const int LANDING_FLAG_FAKEY;
	static const int LANDING_FLAG_JUNK;

	static const float CLOCKTIME_WIPEOUT;

public:
	SurfBoardObjectClass();

	virtual ~SurfBoardObjectClass();

	virtual int32 Terminate();
	virtual int32 Load(char *name);
	virtual int32 Update(float dt);
	virtual int32 Init();
	virtual void OnNewWave();

	static void InitConstants();

  void set_board(entity * board)
  {
    my_board = board;
  }

  void set_board_model(entity * board)
  {
    my_board_model = board;
  }

  kellyslater_controller *get_ks_controller()  {return ksctrl;}
  void set_ks_controller(kellyslater_controller * _ksctrl)  {ksctrl = _ksctrl;}

	void MoveForward(float degree = 1.0f)
	{
		forward.SetForceScalar(props_forwardForce * degree);
	}

	float	props_forwardForce;	// the initial forward force to apply

	//  useful for tubes, this changes player velocity, but only along the X-axis.
	void MoveForwardOnX(float degree = 1.0f);

	vector3d GetForwardDir(void) { return my_board->get_abs_po().non_affine_slow_xform (ZVEC); }
	vector3d GetUpDir(void) { return my_board->get_abs_po().non_affine_slow_xform (YVEC); }
	vector3d GetRightDir(void) { return my_board->get_abs_po().non_affine_slow_xform (XVEC); }
	vector3d GetVelocity(void) { return (rb->linMom/rb->mass); }

	void Turn(BoardStateEnum st, float degree, float dt, float leanEaseIn = 4.5f);
	void StopOnWave() {this->rb->linMom.x = 0;}
	float GetLeanPercentage(void);

	void GetVelocity(vector3d &v)
	{
		//this->rb->GetVelocity(v);
		v = this->rb->linMom;
        v /= this->rb->mass;
	}

	float GetSpeed(void)
	{
		vector3d velocity;

		GetVelocity(velocity);

		return velocity.length();
	}

	void SetWaveRegion(WaveRegionEnum reg) { current_region = reg; }
	int GetWaveRegion(void) { return current_region; }
	int GetLastWaveRegion(void) { return last_region; }

	float CalculatePathPeakTime(void);

	WaveRegionEnum GetRegion(void)
	{
		return current_region;
	}

	void SetBoneController(void * bone) //BoneStructPtr bone)
	{
    debug_print("STUBBED OUT: SetBoneController");
		//this->bone = bone;
	}

	void SetBoardState(BoardStateEnum t_state) { this->state = t_state; }
	BoardStateEnum GetBoardState(void) { return (this->state); }
	int GetLanding(void) const { return landingType; }

	void TrickSpin(float angle);
	void Jump(float degree = 1.0f);

	void BoardFloatWave(const float time_step);
	void BoardInAir(const float time_step);
	void SetWipeoutPhysics(void);
	bool GoneOverLip(float dt);

	bool InAir () const
    { return inAirFlag; }
  bool WipedOut () const
    { return wiped_out; }

	float GetMaxRotation() { return this->maxTurnAngle;}
	void SetMaxRotation(float r) { this->maxTurnAngle = r; }
	float GetMaxLean() { return this->maxLeanAngle; }
	void SetMaxLean(float l) { this->maxLeanAngle = l; }

	float GetTotalAirTime(void) { return air_time; }
	float GetAirTime(void) { return air_timer; }

	vector3d GetTakeoffDir(void) { return take_off_dir; }

	vector3d GetLipVec(void) { return lip_vec; }
	vector3d GetLipNormal(void) { return lip_normal; }

	void InitSnap(void) { snap_vec = -GetForwardDir(); }

	bool IsGrindingObject(void) { return grind_object != NULL; }

	void ResetFloatSpeed(bool max = true);
	void IncrementFloatSpeed(void);

	void AddVelocity(vector3d vel)
	{
		rb->linMom += rb->mass*vel;
	};

	void SetVelocity(vector3d vel)
	{
		rb->linMom = rb->mass*vel;
	};

	void SetControllerForce(vector3d force) { controller_force = force; }
	void AddControllerForce(vector3d force) { controller_force += force; }

	vector3d GetFloatPos(void) { return (float_pos); }

	bool CollideWithLip(void);
	vector3d GetWaveCurrent(void) { return current; }
	vector3d GetWaveNormal(void) { return normal; }

	bool InLaunchRegion(void)
	{
		return ((current_region == WAVE_REGIONLIP)
			|| (current_region == WAVE_REGIONLIP2)
			|| (current_region == WAVE_REGIONCHIN)
			|| (current_region == WAVE_REGIONFACE)
			|| (current_region == WAVE_REGIONBACK));
	};

	bool InTubeRegion(void)
	{
		return current_region == WAVE_REGIONTUBE;// ||
			//current_region == WAVE_REGIONWASH;
	};

	bool InGrindRegion(void);

	bool InSnapRegion(void)
	{
		return ((current_region == WAVE_REGIONLIP)
			|| (current_region == WAVE_REGIONLIP2)
			|| (current_region == WAVE_REGIONCHIN)
			|| (current_region == WAVE_REGIONCURL));
	};

	bool InSnapCone(void)
	{
		vector3d dir = GetForwardDir();
		dir -= YVEC*dot(dir,YVEC);
		dir.normalize();
		float val = dot(dir, lip_normal);
		if (val > 0.342f) //  within 140 degree cone
			return true;

		return false;
	}

	bool InLaunchCone(void)
	{
		vector3d for_dir = GetForwardDir();
		vector3d dir = for_dir;
		dir.z = 0.0f;
		dir.normalize();
		float val = dot(dir,YVEC);
		if (val > 0.174f) //  within 160 degree cone
			return true;

		if ((current_region == WAVE_REGIONBACK) || (current_region == WAVE_REGIONLIP)
				|| (current_region == WAVE_REGIONLIP2))
		{
			dir = for_dir;
			dir.y = 0.0f;
			dir.normalize();
			val = dot(dir,lip_normal);
			if (val > 0.174f) //  within 160 degree cone
				return true;
		}

		return false;
	}

	bool DoingFaceTurn(void);

	void SetTurnType(TurnTypeEnum turn)
	{
		if (turn != TurnType)
		{
			//ground_turn = 0.0f;
			//abs_ground_turn = 0.0f;
		}
		TurnType = turn;
		this->maxLeanAngle = this->turnProps[turn].maxLeanAngle;
		this->maxTurnAngle = this->turnProps[turn].maxTurnAngle;
	};

	int GetTurnType(void) { return TurnType; }
	float GetTurnAmount(void) { return ground_turn; }
	float GetAbsTurnAmount(void) { return abs_ground_turn; }
	void ResetTurnAmounts(void) { abs_ground_turn = ground_turn = 0.0f; }

  bool CanBustThroughTube(vector3d position);
  void ResetPhysics ();
  void ResetTimers();
  void DoWipeOut (int wip_type, bool keep_cam = false);
  void KlugeWipeout(int tumble_anim, int swim_anim);
  void CalcLandingWipeout(void);
  int CalculateCeilingWipeout(void);
  void SetWipeoutDone(void);

  void CalculateAirTurn(float amnt);
  void CorrectLipError(vector3d &pos, float dt);
  bool TongueEffectOn(void) { return (wave_info.onoff && ((wave_info.type == WAVE_PerturbTypeBigTongue)
														|| (wave_info.type == WAVE_PerturbTypeTongue))); }

  float get_wipeout_time() {return wipeout_time;}

	PhysicsObjectClassPtr rb; // rigidbody

	// forces //
	SimpleForce forward;  // move forward force
	vector3d controller_force;


	float x_extent, y_extent, z_extent;

	TurnPropertiesType	turnProps[NUM_TURN_TYPES];

	float maxLeanAngle; // how much the board leans into the turn
	float maxTurnAngle; // how much the board turns
	float maxJumpVelocity;
	bool inAirFlag;
  bool lip_jump;
  bool launch_jump;
  bool float_jump;
  bool grind_ollie;
  bool grind_jump;
  bool roof_jump;  //  used to slide off face without crashing
  bool exit_jump;

	vector3d forwardDir;	// body space forward direction

	bool in_water_wall;

  WaveBreakInfoStruct wave_info;

  WavePositionHint wave_front_hint;
  WavePositionHint wave_center_hint;
  WavePositionHint wave_dummy_hint;
  WavePositionHint wave_tube_hint;
  bool front_hint_valid;
  bool center_hint_valid;
  bool tube_hint_valid;
  vector3d prev_center_pos; // used to store previous collide position for the center hint

	// properties //
	BoardStateEnum state;
	float turnDegree;

	float leanEaseIn;  // how much to ease into a lean
	float curLeanAngle; // current lean angle

  entity * my_board; // Controller link PTA 4/23/01
  entity * my_board_model;
  kellyslater_controller * ksctrl;
  orientation last_orientation;
  vector3d current;
  vector3d normal;
  float air_timer;
  float air_time;

  bool wiped_out;
  int Downdate(float time_inc);
protected:
  // wipe out stuff
  bool CheckForWipeOut (CollideCallStruct& collide, float dt);
  bool CheckForLandingWipeOut (CollideCallStruct& colide, float dt);
  bool CheckForGrindLandingWipeOut(CollideCallStruct& colide, float dt);

  void KeepSurferOnWave(vector3d &position, vector3d &forces);

  // grinding
  bool do_grinding_stuff (float dt);
  bool check_entity_grind (entity *ent);
  void check_for_grinding ();

  water_object* grind_object; // beach object that surfer is currently grinding on
  entity* grind_entity;       // collision entity that surfer is currently grinding on
  float grind_percent;        // amount that the surfer has grinded along grind_vector
  vector3d grind_vector;      // direction
  bool grind_forward;
  int grind_idx;
  float grind_speed;

  static const float LANDING_ANGLE_PERFECT;
  static const float LANDING_ANGLE_REGULAR;
  static const float LANDING_ANGLE_SLOPPY;
  static const float LANDING_ANGLE_JUNK;

  int	landingType;	// bitfield of LANDING_FLAG_*

  int wipeout_type;
  float backwards_time; // how long the surfer is moving backwards
  float wipeout_time;   // how long to wait until the player is reset
  float max_wipeout_time;
  float standing_time;

  WaveRegionEnum current_region;
  WaveRegionEnum last_region;

  float air_turn;       // amount of turning in the air
  float abs_ground_turn;
  float ground_turn;
  vector3d take_off_dir;// direction when the surfer jumped

  int TurnType;

  float omega;

  float last_f; //  to filter roll with repect to absolute when not turning
  vector3d snap_vec;
  bool init_grind;

  float float_speed;
  vector3d float_pos;

  vector3d lip_vec; //  along lip away from break
  vector3d lip_normal; //  perpindicular to YVEC and lip_vec, away from the beach


  // player/board attributes
//  float stability_factor;
//  float speed_factor;
//  float carving_factor;
//  float trick_factor;
};

// FIXME: move these values inside the board class once they don't need to be changed by the debug menu anymore
extern float stability_factor;
extern float speed_factor;
extern float carving_factor;
extern float trick_factor;

#endif//__KSPS_BOARD_H_

