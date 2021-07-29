#ifndef _AI_LOCOMOTION_H_
#define _AI_LOCOMOTION_H_

#include "global.h"
#include "ostimer.h"
#include "path.h"
#include "ai_polypath.h"
#include "ai_constants.h"

class ai_interface;
class entity;

enum eLocomotionType
{
  LOCOMOTION_NONE,
  LOCOMOTION_DIRECT,
  LOCOMOTION_WALK,
  LOCOMOTION_WINGED,
  LOCOMOTION_HELI
};

class ai_locomotion
{
public:
  typedef enum
  {
    _FORWARD,
    _FORWARD_R,
    _RIGHT,
    _BACKWARD_R,
    _BACKWARD,
    _BACKWARD_L,
    _LEFT,
    _FORWARD_L,
    _WINGED_IDLE,
    _JOCKEY_ANIMS
  } eJockeyAnim;

protected:
  static const char* jockey_anims[_JOCKEY_ANIMS];

  friend class ai_interface;

  ai_interface *owner;

  path current_path;
  ai_path my_path;
  path_graph *current_path_graph;
  int patrol_id;
  bool xz_movement;

  virtual ai_locomotion *make_copy(ai_interface *own);
  void copy(ai_locomotion *b);

  rational_t patrol_radius;

  time_value_t has_been_stuck_for_how_long;
  vector3d previous_pos;
  vector3d local_dest;

  vector3d start_pos;
  vector3d last_pos;
  vector3d target_pos;
  rational_t tgtrange;
  bool running_speed;
  bool use_path;
  int path_tries;
  bool wait_until_reached;
  bool wait_for_facing;

  virtual bool set_path(const vector3d &dest, rational_t additional_weight_mod = 0.5f, bool force_path = false);
  virtual void set_goto_path(rational_t mod = 0.0f, bool force_path = false);

  bool in_service;

  virtual void going_into_service();
  virtual void going_out_of_service();


  void read_data(chunk_file& fs);
  virtual void handle_chunk(chunk_file& fs, stringx &label);

  eLocomotionType type;

  bool facing;
  bool playing_face_anim;
  vector3d face_dir;
  rational_t turn_speed;


  rational_t jockey_timer;
  rational_t jockey_stuck_timer;
  vector3d last_jockey_pos;
  vector3d jockey_pos;
  vector3d jockey_dir;
  rational_t jockey_speed;
  bool jockey;
  bool use_45_jockey;
  eJockeyAnim jockey_anim_a;
  eJockeyAnim jockey_anim_b;

  virtual void adjust_jockey_animation(const vector3d &dir, time_value_t t);


  void conditional_compute_sector(region_node *rgn, const vector3d &p1, const vector3d &p2);

  time_value_t repulsion_timer;
  vector3d repulsion_local_dest;
  bool process_repulsion(time_value_t delta_t);
  time_value_t repulsion_wait_timer;
  bool repulsion_wait;

public:
  ai_locomotion(ai_interface *own);
  virtual ~ai_locomotion();

  path *get_current_path()                          { return(&current_path); }
  path_graph *get_current_path_graph()              { return(current_path_graph); }
  void set_current_path_graph(path_graph *g)        { clear_path(); current_path_graph = g; }
  void clear_path();

  ai_interface *get_ai_interface() const    { return(owner); }
  entity *get_my_entity() const;

  rational_t get_xz_rotation_to_point( const vector3d &target_pos ) const;
  rational_t get_abs_xz_rotation_to_point( const vector3d &target_pos ) const;
  rational_t get_xz_rotation_to_dir( const vector3d &dir ) const;
  rational_t get_abs_xz_rotation_to_dir( const vector3d &dir ) const;
  bool xz_facing_point( const vector3d &target_pos, rational_t rads ) const;
  void apply_rotation( rational_t rot );

  bool crossed_point(const vector3d test_point, const vector3d &cur_pos, rational_t radius, bool force_xz);
  bool get_next_patrol_point(const vector3d &last_pos, const vector3d &cur_pos, vector3d &next);
  bool get_nearest_patrol_point(const vector3d &cur_pos, vector3d &patrol_pt);

  bool xz_movement_only()     { return(xz_movement); }
  int get_patrol_id()         { return(patrol_id); }

  rational_t get_patrol_radius()        { return(patrol_radius); }
  void set_patrol_radius(rational_t r)  { patrol_radius = r; }

  virtual bool frame_advance(time_value_t t);
  virtual bool process_movement(time_value_t t)          { return(true); }
  virtual bool set_destination(const vector3d &pos, rational_t radius = 2.0f, bool fast = true, bool path_find = true, bool force_finish = false);
  virtual bool set_facing(const vector3d &dir, rational_t mod = 1.0f);
  virtual bool set_facing_point(const vector3d &pt, rational_t mod = 1.0f);

  eLocomotionType get_type() const { return(type); }

  void jockey_to(const vector3d &pt);
  void jockey_dir_time(const vector3d &dir, time_value_t t);
  void stop_jockey();
  inline bool is_jockeying() const  { return(jockey); }

  bool using_animation() const;

  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};

extern const rational_t safe_atan2_ZVEC;

#endif
