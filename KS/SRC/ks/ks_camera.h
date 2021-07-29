#ifndef KS_CAMERA_H
#define KS_CAMERA_H

#define OVERHEAD_CAM_THRESHHOLD 3
#define TUBE_TRANSITION_LENGTH 12

#include "FEAnim.h"

class vector_filter
{
public:
	void Filter_Vector(vector3d &filtered_vec, time_value_t time_step, float filter_speed_r, float filter_speed_i = 1000000);
	void Init_Filter(vector3d start_vec);
	vector3d GetCurrentState(void) { return vector3d(xwPos[0], ywPos[0], zwPos[0]); }
	vector3d GetCurrentError(void) { return vector3d(xerr[0], yerr[0], zerr[0]); }
	
private:
	rational_t xwPos[2];
	rational_t ywPos[2];
	rational_t zwPos[2];
	rational_t xerr[2];
	rational_t yerr[2];
	rational_t zerr[2];
};

class float_filter
{
public:
	void Filter_Float(float &filtered_float, time_value_t time_step, float filter_speed_r, float filter_speed_i = 1000000);
	void Init_Filter(float start_float);
	
private:
	rational_t Pos[2];
	rational_t err[2];
};

//  this camera is different than the previous one because it can update the yaw and pitch by using the PS2 controller
class debug_camera : public game_camera
{
public:

	debug_camera( const entity_id& _id, entity* _target_entity = NULL );
	virtual ~debug_camera() { /* destroy_link_ifc(); */ }
	virtual void sync( camera& b );
    virtual void frame_advance( time_value_t t );

private:

	rational_t yaw;  //  angle of camera vector from surfers x axis projected onto world xz plane
	rational_t pitch;    //  angle of camera vector from world xz plane
	rational_t magn;   //  magnitude of camera vector
};

class look_back_camera : public game_camera
{
private:
	vector3d object_pos;
	vector_filter target_filter;
	bool first_time;
	float_filter look_back_height_filter;
	float_filter offset_filter;

public:
	look_back_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~look_back_camera () { /* destroy_link_ifc(); */ }
	virtual void sync (camera& b);
	bool FindNearestObject(void);
	virtual void frame_advance (time_value_t t);
	virtual void init();
};

class replay_camera : public game_camera
{
public:
  enum ReplayCam        {RC_DEFAULT,        // Side/beach cam.
                         RC_TUBE_DEFAULT,   // View from front of surfer, looking back into the tube
                         RC_AIR_YOYO,       // Stay directly above surfer at a constant height (just above the max height of the jump)
                         RC_AIR_BIRD,       // Flyby type cam, that swoops in close at the height of the jump
                         RC_AIR_CIRCLE,     // Circle around the surfer while in the air
                         RC_AIR_TAKEOFF,    // Stationary cam at the point of takeoff
                         RC_AIR_LANDING,    // Stationary cam at the point of landing
                         RC_NONE};
  enum ReplayCamRegion  {RCR_AIR, RCR_FACE, RCR_TUBE, RCR_WIPEOUT, RCR_MISC, RCR_NONE};

private:
  ReplayCam           rc;                   // Current camera
  ReplayCamRegion     rcr;                  // Region of the wave the main surfer is in

  float               regionChangeTime;     // Time until surfer will change regions
  int                 regionChangeFrame;    // Replay frame when the surfer will change regions
  float               lastCamChange;        // Time of last camera change
  bool                regionChange;         // Did the region change last call to updateCamRegion()
  ReplayCamRegion     lastRegion;           // Next region the surfer will be in
  ReplayCamRegion     nextRegion;           // Next region the surfer will be in

  entity              *ent;                 // Entity that's targeted

  vector3d            position;             // Camera position
  vector3d            target;               // Camera target location
  vector3d            velocity;             // Camera velocity

  vector3d            defaultCam;           // Loction of default cam from its target
  vector3d            tubeCam;              // Loction of default tube cam from its target
  vector3d            yoyoCam;              // Distance from height of jump for the yoyo cam
  vector3d            birdCam;              // Distance from close up of bird cam
  vector3d            birdCamVel;           // Velocity of "bird"
  float               circleCamStartDist;   // Starting distance from player
  float               circleCamCloseDist;   // Distance from close up of circle cam
  vector3d            takeoffCamPos;        // Distance from takeoff point
  vector3d            takeoffCamDir;        // Camera direction
  vector3d            landingCamPos;        // Distance from landing point
  vector3d            landingCamDir;        // Camera direction

  bool                waveDir;              // Direction of the wave

  vector3d            jumpStartPos;         // Starting position of the current jump
  vector3d            jumpEndPos;           // Ending position of the current jump
  vector3d            jumpMaxHeight;        // Position at the peak of the current jump

  float               holdCamTimer;         // Do not change from the current cam while this is > 0
  ReplayCam           nextCam;              // Change to this cam when holdCamTimer reaches 0;

  float               keepTubeThreshhold;   // If tube is left and re-entered in less than this threshhold, treat it as never having left the tube
  float               keepFaceThreshhold;   // If tube is left and re-entered in less than this threshhold, treat it as never having left the tube

  void pickCam();
  void updateCam(float time_delta);
  void updateCamRegion(float time_delta);

public:
  replay_camera (const entity_id& _id, entity* _target_entity = NULL);
  virtual ~replay_camera () { /* destroy_link_ifc(); */ }
  virtual void reset ();
  virtual void sync (camera& b);
  virtual void frame_advance (time_value_t t);
  //camera *get_last_camera() { return last_cam; };
  //void set_last_camera(camera *cam) { last_cam = cam; };
  //void toggle_debug_cam() {debugCam = !debugCam;}
  //void toggle_player();
};

class old_shoulder_camera : public game_camera
{
public:
  old_shoulder_camera (const entity_id& _id, entity* _target_entity = NULL);
  virtual ~old_shoulder_camera ();
  virtual void frame_advance (time_value_t t);

private:
  float turn_angle;
  bool lip_jump;
  vector3d last_desired_pos;
  vector3d last_target;
};

class shoulder_camera : public game_camera
{
public:
	shoulder_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~shoulder_camera ();
	virtual void frame_advance (time_value_t time_step);
	virtual void init();
	
private:
	bool lip_jump;
	vector3d last_desired_pos;
	vector3d last_target;
	bool first_time;
	float camera_angle_change;
	vector_filter shoulder_cam_filter;
	float_filter dist_mult_filter;
	float_filter tube_dist_filter;
	vector3d surfer_cam;
	vector3d delta_cam;
	bool use_delta_cam;
	float last_tube_difference;
};

class fps_camera : public game_camera
{
public:
	fps_camera (const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl);
  virtual ~fps_camera () { /* destroy_link_ifc(); */ }
  virtual void sync (camera& b);
	virtual void init ();
  virtual void frame_advance (time_value_t t);

};
class stationary_camera : public game_camera
{
public:
	typedef list <vector3d> POINT_LIST;
	typedef POINT_LIST::const_iterator POINT_ID;

  stationary_camera (const entity_id& _id, entity* _target_entity = NULL);
  virtual ~stationary_camera () { /* destroy_link_ifc(); */ }
  virtual void sync (camera& b);
  virtual void frame_advance (time_value_t t);
  void add_point (const vector3d & pt);

protected:
  POINT_ID current_point;
  POINT_LIST camera_points;
  float change_timer;
};

/*class flyby_camera : public game_camera
{
public:
  typedef struct
  {
    vector3d pt;
    float time;
  } PATH_POINT;
	typedef list <PATH_POINT> POINT_LIST;
	typedef POINT_LIST::const_iterator POINT_ID;

  flyby_camera (const entity_id& _id, entity* _target_entity = NULL);
  virtual ~flyby_camera () { /* destroy_link_ifc(); *//* }
  virtual void sync (camera& b);
  virtual void frame_advance (time_value_t t);
  void add_point (const vector3d & pt, float time);

protected:
  POINT_ID current_point;
  POINT_LIST camera_points;
  float change_timer;
};*/


class auto_camera : public game_camera
{
public:
    auto_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl);
    virtual ~auto_camera() { /* destroy_link_ifc(); */ }
    virtual void sync( camera& b );
    virtual void frame_advance( time_value_t t );
	void Reset(void);
	virtual void init(void) { Reset(); }
	vector3d GetStartPosition(void) { return GetPosDifference(0.0f); }
	virtual void OnNewWave(void) { Reset(); }

protected:

	enum
	{
		BEACH_CAM1,
		BEACH_CAM2
	};

	enum
	{
		BREAK_CAM_OFF,
		BREAK_TONGUE_NORMAL,
		BREAK_TONGUE_COLLAPSE,
		BREAK_SURGE,
		BREAK_RUSH
	};

	virtual int GetCameraType(void) { return BEACH_CAM1; }
	virtual vector3d GetPosDifference(float t);
	void DoBreakCameraWork(void);
	bool InLaunchState(void);
	void FindNearestObject(void);

	void Norm(vector3d &vec, float &length)
	{
		vector3d copy = vec;
		vec.normalize();
		length = dot(vec, copy);
	};

	int break_cam_state;
	WaveBreakInfoStruct wave_info;
	vector_filter low_pass_filter;
	vector_filter facing_offset_filter;
	bool object_near;
	vector3d object_pos;
	vector3d delta_vec;
	float delta_float;
	vector3d actual_vec;
	float actual_float;
	float cutoff_frequency;
	bool init_filter;
};

class beach_camera : public auto_camera
{
public:
	beach_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl);
private:
	virtual int GetCameraType(void) { return BEACH_CAM2; }
	virtual vector3d GetPosDifference(float t);
};


class big_wave_camera : public game_camera
{
public:
	big_wave_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl);
	virtual ~big_wave_camera() { /* destroy_link_ifc(); */ }
    virtual void frame_advance( time_value_t t );
	void Reset(void);
	virtual void init(void) { Reset(); }
private:

	vector_filter low_pass_filter;
	float cutoff_frequency;
};

class wipeout_camera_2 : public game_camera
{
public:
	wipeout_camera_2 (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~wipeout_camera_2 ( /* destroy_link_ifc(); */ );
	virtual void frame_advance (time_value_t time_step);
	void init(po initial_po, camera *prev_cam);

private:
	vector_filter camera_filter;
	float_filter distance_filter;
	float lowest_y;
	camera *previous_camera;
	vector3d start_point;
	vector3d rel_vec;

	WavePositionHint wave_hint;
	bool wave_hint_valid;
	vector3d water_normal;
	WaveRegionEnum water_region;
};

class wipeout_camera : public game_camera
{
public:
	wipeout_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~wipeout_camera ();
	virtual void frame_advance (time_value_t time_step);
	void rotate_cam(vector3d *camera_pos, vector3d center_pos, float rotation_val);
	void init(po initial_po, camera *prev_cam);
	camera *get_prev_cam() {return previous_camera;}
	void set_collision_obj(entity *collision_obj) {collision_object = collision_obj; current_rotation = 0.0f;}

private:
	vector_filter camera_filter;
	vector_filter target_filter;
	float_filter distance_filter;
	camera *previous_camera;  //  Keep track of which camera was around before the wipeout.
	entity *collision_object;
	float current_rotation;
	int cam_type;
	bool just_started;
	bool was_swimming;
	po starter_po;
	float lowest_z;
};


class flyby_camera : public game_camera
{
public:
	flyby_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~flyby_camera ();
	virtual void frame_advance (time_value_t time_step);
	bool load();
	void start();
	bool is_finished();

private:
	PanelAnimFile *animation;
	bool is_playing;
	float current_anim_time;
	PanelAnim *camera;
	PanelAnim *target;
};



class follow_camera : public game_camera
{
public:
	follow_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~follow_camera () {}
	virtual void frame_advance (time_value_t time_step);
	virtual void init();

private:
	bool first_time;
	vector_filter cam_pos_filter;
	float jump_time_elapsed;
	vector3d last_cam_vec;
	vector3d in_air_cam;

	float_filter distance_filter;
};


class follow_close_camera : public game_camera
{
public:
	follow_close_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~follow_close_camera () {}
	virtual void frame_advance (time_value_t time_step);
	virtual void init();

private:
	bool first_time;
	vector_filter cam_pos_filter;
	float jump_time_elapsed;
	vector3d last_cam_vec;
	vector3d in_air_cam;

	float_filter distance_filter;
	float_filter above_head_filter;
	vector_filter facing_offset_filter;
};


class buoy_camera : public game_camera
{
public:
	buoy_camera (const entity_id& _id, entity* _target_entity = NULL);
	virtual ~buoy_camera () {}
	virtual void frame_advance (time_value_t time_step);
	virtual void init();

private:
	bool first_time;
	vector_filter cam_pos_filter;
	float jump_time_elapsed;
	vector3d last_cam_vec;
	vector3d in_air_cam;
	float default_distance_mult;

	float_filter distance_filter;
};


float interpolate_float(float first_float, float second_float, float difference);
void interpolate_vector(vector3d *new_vector, vector3d first_vector, vector3d second_vector, float difference,  int curve = 0);
float Get_Tube_Cam_Vector(kellyslater_controller *ksctrl, vector3d *tube_vector, entity *board_pos, float lag_distance);

//////////////////////////////////    Duck Diving Camera  //////////////////////////////////////

class duckdive_camera : public game_camera
{
public:
	duckdive_camera (const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl );
	virtual ~duckdive_camera() { /* destroy_link_ifc(); */ }
	virtual void frame_advance( time_value_t t );
	void Reset(void);
	void SetExitTransition(void) { trans_2_norm_cam = true; }
	void SetReset(void);

	bool do_reset;

private:

	void Norm(vector3d &vec, float &length)
	{
		vector3d copy = vec;
		vec.normalize();
		length = dot(vec, copy);
	};

	vector_filter low_pass_filter;
	vector3d delta_vec;
	float delta_float;
	vector3d actual_vec;
	float actual_float;
	float cutoff_frequency;

	bool trans_2_norm_cam;
	bool start_trans;
	game_camera *orig_cam;

};

//////////////////////////////////    Photo Camera  //////////////////////////////////////

class photo_camera : public game_camera
{
private:
	static float DIST_NORMAL;	// camera distance when surfer is surfing normally
	static float DIST_AIR;		// camera distance when surfer is in the air
	static float DIST_TUBE;		// camera distance when surfer is in the tube

private:
	bool			first_time;
	vector_filter	cam_pos_filter;
	float			jump_time_elapsed;
	vector3d		last_cam_vec;
	vector3d		in_air_cam;
	float_filter	distance_filter;

public:
	// Creators.
	photo_camera(const entity_id & _id, entity * _target_entity, kellyslater_controller * _ksctrl);

	// Modifiers.
	virtual void frame_advance(time_value_t dt);
	virtual void init();
};

#endif  // KS_CAMERA_H
