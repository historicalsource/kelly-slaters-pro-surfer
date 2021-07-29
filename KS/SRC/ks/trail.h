#ifndef _TRAIL_H_
#define _TRAIL_H_

#ifdef TARGET_XBOX
#include "HWOSXB\xb_algebra.h"
#endif // TARGET_XBOX

#include "wave.h"

class kellyslater_controller;

#define MAX_SPRAY_PTS 25
#define MAX_TRAIL_NODES 30

typedef struct
{
  WavePositionHint hint;
  WaveVelocityHint vhint;
  vector3d pos;
  vector3d norm;
  WaveRegionEnum region;
  u_int vvalid;
} surface_point_t;

typedef struct
{
  u_int valid;
  float age;
  float uv;

  surface_point_t pnt1;
  surface_point_t pnt2;
  surface_point_t pnt3;
} trail_node_t;

typedef struct
{
#if defined(TARGET_XBOX)
  vector3d __declspec(align(16)) current;	// aligned so can treat as nglVectors
  vector3d __declspec(align(16)) start;
#else
  vector3d current __attribute__((aligned(16)));	// aligned so can treat as nglVectors
  vector3d start __attribute__((aligned(16)));
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d *trailpt;
	trail_node_t *trail_node;
  vector3d vel;
  float seed;
  float age;
  float life;
  int valid;
} spray_control_t;

typedef struct
{
  spray_control_t control_points_a[MAX_SPRAY_PTS];
  spray_control_t control_points_b[MAX_SPRAY_PTS];
  int num_control_points_a;
  int num_control_points_b;

  float last_apow;
  float last_bpow;
  vector3d lastpointa;
  vector3d lastpointb;
  float extra_splash_power[2];
  vector3d last_vec;

  float floater_interval;
  float airdrops_interval;
  float left_hand_interval;
  float right_hand_interval;
} spray_params_t;

class trail
{
public:
  trail (bool spray);
  virtual ~trail ();

  void draw ();
  void update (float dt);
  void add_point (vector3d *pointa, vector3d *center, vector3d *pointb, float vela, float velb, WavePositionHint *hint, bool dummy);

  bool is_valid () const
    { return valid; }

  void initialize (float samplerate, float life, bool extra, kellyslater_controller *owner);
  void destroy ();
  void reset ();

  void create_big_landing_splash ();
  void create_chophop_splash (vector3d& pos);
	bool spray_object (entity *ent) const;
	void create_face_trick_splash (bool left);

protected:
  void update_surface_point (surface_point_t *sp);
  void create_surface_point (surface_point_t *newone, vector3d *pos, WavePositionHint *hint, 
	  const WaveTolerance *tolerance);
  void comp_trail_orig (vector3d *pointa, vector3d *cent, vector3d *pointb);
  float comp_part_bspray (vector3d *Vec, vector3d *Pos, float dt, float scale);
  bool spraypt_add (spray_control_t *SC, spray_control_t *ListPtr, int *ListCount);
  void spraypt_pos (spray_control_t *SC);
  void spraypt_draw(spray_control_t *SprayControlPts, u_int fxindex, float vary, int scp_index, int web);
  void spraypt_update (spray_control_t *SprayControlPts, u_int max);
  bool hand_underwater (entity *hand, vector3d& pos) const;

  void board_update (float dt);
  void board_update_air (float dt);
	bool spray_object_internal (spray_control_t *SprayControlPts, int scp_index, entity *ent) const;

  // trail variables
  bool valid;
  u_int my_index;
  float my_life;
  float my_sample_rate;
  float my_last_sample;
  u_int my_total_nodes;
  trail_node_t my_trail_data[MAX_TRAIL_NODES];
  bool my_extra;
  float my_lastmag1;
  float my_lastmag2;

  kellyslater_controller* my_owner;
  spray_params_t *my_spray;
};

#define MAX_TRAIL_GENERATORS 4
extern trail* g_trails[MAX_TRAIL_GENERATORS];

#endif // _TRAIL_H_
