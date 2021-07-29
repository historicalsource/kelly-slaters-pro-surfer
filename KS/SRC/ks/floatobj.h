#ifndef _FLOATOBJ_H_
#define _FLOATOBJ_H_

#include "entity_anim.h"
#include "kellyslater_controller.h"
#include "file_finder.h"

class trail;

#include "wave.h"

typedef enum
{
  DOLPHIN_STATE_HIDDEN,
  DOLPHIN_STATE_PRESURFACE,
  DOLPHIN_STATE_SURFACE,
  DOLPHIN_STATE_SINK,
  DOLPHIN_STATE_FLIP,
  DOLPHIN_STATE_SWIM,
  DOLPHIN_STATE_JUMP
} DOLPHIN_STATES;

class text_parser;

class beach_object
{
public:
  beach_object (entity *ent, const stringx& path);
  virtual ~beach_object ();

  virtual void spawn () = 0;
  virtual void despawn () = 0;
  virtual bool update (float dt) = 0;
  virtual bool parse_params (char** argp, int argc);
  virtual void get_settings (const beach_object& obj);

  virtual void collide (entity *ent, const vector3d& dir);
  virtual void jumped_over (entity *ent);
  virtual void sprayed (entity *ent);

  virtual bool is_surfing_object() { return false; }

  beach_object *next;
  float spawn_time;
  int timer_type;	// -1 means relative to level.  n means relative to start of wave type n. (dc 02/11/02)
  int times_spawned;
  bool spawned;
  bool smashable;
  bool active;
	bool never_despawn;

  entity* get_entity () const
    { return my_entity; }
  bool is_physical () const
    { return physical; }
  bool is_active () const
    { return active; }
	int get_spawn_count () const
		{ return spawn_count; }

protected:
  bool physical;
  entity *my_entity;
  int spawn_count;
	stringx my_path;

  // don't really need to be here
  bool find_param (char** argp, int argc, const char *name) const;
  bool read_int_param (char** argp, int argc, const char *name, int *value) const;
  bool read_float_param (char** argp, int argc, const char *name, float *value) const;
  bool read_vector3d_param (char** argp, int argc, const char *name, vector3d *value) const;
  bool read_text_param (char** argp, int argc, const char *name, stringx& value) const;
};

class beach_event : public beach_object
{
public:
  beach_event (bool (*func) (float dt, void **func_data));
  virtual ~beach_event ();

  void spawn ();
  void despawn ();
  bool update (float dt);
  static beach_event* create_from_file (text_parser& parser, int timer_relative = -1);

protected:
  bool (*my_func) (float dt, void **func_data);
  void *my_func_data;
};

class beach_billboard : public beach_object
{
public:
  beach_billboard (entity *ent, const stringx& path);
  virtual ~beach_billboard ();

  void spawn ();
  void despawn ();
  bool update (float dt);
  static beach_billboard* create_from_file (text_parser& parser, int timer_relative = -1);
  virtual bool parse_params (char** argp, int argc);

protected:
  vector3d my_velocity;
  //vector3d my_initial_position;
  po my_initial_po;
  bool rotate;

  float my_age;
  float my_life;
};

class water_object : public beach_object
{
public:
  water_object (entity *ent, const stringx& path);
  virtual ~water_object ();

  virtual bool parse_params (char** argp, int argc);
  static water_object* create_from_file (text_parser& parser, int timer_relative = -1);
  virtual void get_settings (const water_object& obj);

  virtual void spawn ();
  virtual void despawn ();
  virtual bool update (float dt);

  virtual void collide (entity *ent, const vector3d& dir);
  virtual void jumped_over (entity *ent);
  virtual void sprayed (entity *ent);

  bool is_grindable () const
    { return grindable; }
	virtual int get_type () const
		{ return -1; }

protected:
  color32 ren_col;
	float my_max_alpha;

  WavePositionHint wave_hint;
  bool use_hint;
  po my_initial_po;
  bool grindable;
  float fade_distance;
  bool spawn_by_marker;
  float marker_offset;
  float marker_num;
  float current_x_mult;
};

//#define SLALOM_TEST

class floating_object : public water_object
{
public:
  floating_object (entity *ent, const stringx& path);
  virtual ~floating_object ();

  bool parse_params (char** argp, int argc);
  virtual void get_settings (const floating_object& obj);

  void spawn ();
  bool update (float dt);

  virtual void collide (entity *ent, const vector3d& dir);

  void set_dy (float max, float speed)
    { max_dy = max; speed_dy = speed; }
  void set_angle (float max, float speed)
    { max_angle = max; speed_angle = speed; }

#ifdef SLALOM_TEST
  typedef enum { SLALOM_NOTTESTED, SLALOM_MISSED, SLALOM_OK };
  floating_object *my_other_buoy;
  int slalom;
#endif

private:
  // variables to control the way the object moves up and down
  float desired_dy;
  float current_dy;
  float max_dy;
  float speed_dy;

  // variables to control the tilting
  float desired_angle;
  float current_angle;
  float max_angle;
  float speed_angle;

  float water_interaction;
};

// ============================================================================
// generic_anim stuff for surfing objects

class generic_anim
{
public:
  generic_anim (const stringx& path, const stringx& name) { my_base_name = name; dummy = false; cur_state = cur_anim = 0; left_down = right_down = false; };
  virtual ~generic_anim () { };

  virtual void update (bool collide, bool jump, bool spray, float *alpha) = 0;
  virtual void spawn () = 0;
	virtual void switch_anims () = 0;
	void set_dummy ()
		{ dummy = true; }

#ifdef DEBUG
	virtual void draw_debug_labels() { };
#endif

protected:
  stringx my_base_name;
  int cur_state, cur_anim;
	bool dummy;
	bool left_down, right_down;
};

class generic_anim_misc : public generic_anim
{
public:
  generic_anim_misc (entity *entity, const stringx& path, const stringx& name);
  generic_anim_misc (entity **entities, const stringx& path, const stringx& name, const char **prefixes, int count);

	void construct (entity **entities, const stringx& path, const stringx& name, const char **prefixes, int count);
  virtual ~generic_anim_misc ()
	{
		delete[] items_prefixes;
		delete[] my_entities;

#ifdef DEBUG
		for (int i = 0; i < items_count; i++)
			delete anim_labels[i];
		free (anim_labels);
#endif
	};

  void update (bool collide, bool jump, bool spray, float *alpha);
  void spawn ()
	  { generic_anim_state = GA_SPAWN; };
	void switch_anims ();

#ifdef DEBUG
	virtual void draw_debug_labels();
#endif

private:
  static const char* generic_anim_names[];
  int generic_anims[5];
  int generic_anim_state;
	int items_count;
	stringx* items_prefixes;
  entity **my_entities;

#ifdef DEBUG
	FloatingText **anim_labels;
#endif

  typedef enum { GA_IDLE, GA_COLLIDE, GA_AFTER_COLLIDE, GA_JUMP, GA_SPRAY, GA_SPAWN };
};

class generic_anim_animal : public generic_anim
{
public:
  generic_anim_animal (entity *entity, const stringx& path, const stringx& name);
  virtual ~generic_anim_animal ()
	{
#ifdef DEBUG
		delete anim_label;
#endif
	};

  void update (bool collide, bool jump, bool spray, float *alpha);
  void spawn ()
	  { generic_anim_state = AA_SPAWN; };

	void switch_anims ();

#ifdef DEBUG
	virtual void draw_debug_labels();
#endif

	bool is_diving () const
		{ return generic_anim_state == AA_DIVE; }

private:
  static const char* generic_anim_names[];
  int generic_anims[2];
  int generic_anim_state;
  entity *my_entity;

#ifdef DEBUG
	FloatingText *anim_label;
#endif

  typedef enum { AA_IDLE, AA_DIVE, AA_SPAWN };
};

class generic_anim_ice : public generic_anim
{
public:
  generic_anim_ice (entity *entity, const stringx& path, const stringx& name);
  virtual ~generic_anim_ice ()
	{
#ifdef DEBUG
		delete anim_label;
#endif
	};

  void update (bool collide, bool jump, bool spray, float *alpha);
  void spawn ()
	  { generic_anim_state = IA_SPAWN; };

	void switch_anims ();

#ifdef DEBUG
	virtual void draw_debug_labels();
#endif

private:
  static const char* generic_anim_names[];
  int generic_anims[3];
  int generic_anim_state;
  entity *my_entity;

#ifdef DEBUG
	FloatingText *anim_label;
#endif

  typedef enum { IA_IDLE, IA_COLLIDE, IA_COLLIDEI, IA_SPAWN };
};

class surfing_object : public water_object
{
public:
  surfing_object (entity *ent, const stringx& path, const stringx& name);
  virtual ~surfing_object ();

  void spawn ();
  void despawn ();
  bool update (float dt);
  bool parse_params (char** argp, int argc);

  virtual void collide (entity *ent, const vector3d& dir);
  virtual void jumped_over (entity *ent);
  virtual void sprayed (entity *ent);

  virtual bool is_surfing_object() { return true; }

  int get_state () const { return my_state; };
  typedef enum { BOOGIEBOARDER, MISC_SURFER1, MISC_SURFER2, KAYAKER, FATBASTARD, SWIMMER, CAMERAMAN, DOLPHIN, GREATWHITE, SEAL, SEAGULL, OUTRIGGER, HUMPBACK, WINDSURFER, HAMMERHEAD, MANTARAY, FISHERMAN, TURTLE, JETSKIER, SNORKELER, DINGY, ICEPATCH, KELP, HELICOPTER, DUMMY } SURFING_OBJECT_TYPES;
	virtual int get_type () const
		{ return my_type; }

#ifdef DEBUG
	void draw_debug_labels();
#endif

protected:
  void set_state (int new_state) { my_state = new_state; }; // this should only be used by the replay system
  void wave_check_collision (vector3d& position, vector3d& normal, vector3d *current, WaveRegionEnum *region);

#ifdef DEBUG
	FloatingText *anim_label;
#endif

private:
	SSEventId mySound;
  // Animation
  generic_anim* my_anim_handler;

  // AI
  bool dummy_ai (vector3d& position, vector3d& normal, float dt);
  bool floating_ai (vector3d& position, vector3d& normal, float dt);

  bool boogie_ai (vector3d& position, vector3d& normal, float dt);
  bool surfer1_ai (vector3d& position, vector3d& normal, float dt);
  bool surfer2_ai (vector3d& position, vector3d& normal, float dt);
  bool kayaker_ai (vector3d& position, vector3d& normal, float dt);
  bool fatbastard_ai (vector3d& position, vector3d& normal, float dt);
  bool swimmer_ai (vector3d& position, vector3d& normal, float dt);
  bool cameraman_ai (vector3d& position, vector3d& normal, float dt);
  bool dolphin_ai (vector3d& position, vector3d& normal, float dt);
  bool greatwhite_ai (vector3d& position, vector3d& normal, float dt);
  bool seal_ai (vector3d& position, vector3d& normal, float dt);
  bool seagull_ai (vector3d& position, vector3d& normal, float dt);
  bool outrigger_ai (vector3d& position, vector3d& normal, float dt);
  bool humpback_ai (vector3d& position, vector3d& normal, float dt);
  bool windsurfer_ai (vector3d& position, vector3d& normal, float dt);
  bool hammerhead_ai (vector3d& position, vector3d& normal, float dt);
  bool mantaray_ai (vector3d& position, vector3d& normal, float dt);
  bool fisherman_ai (vector3d& position, vector3d& normal, float dt);
  bool turtle_ai (vector3d& position, vector3d& normal, float dt);
  bool jetskier_ai (vector3d& position, vector3d& normal, float dt);
  bool snorkeler_ai (vector3d& position, vector3d& normal, float dt);
	bool dingy_ai (vector3d& position, vector3d& normal, float dt);
	bool icepatch_ai (vector3d& position, vector3d& normal, float dt);
	bool kelp_ai (vector3d& position, vector3d& normal, float dt);
	bool helicopter_ai (vector3d& position, vector3d& normal, float dt);

  bool (surfing_object::*ai_func) (vector3d& position, vector3d& normal, float dt);

  int my_type;
  float timer;
  float timer2;
  float turn_amount;
  float turn_rate;
  float lean_amount;
	float my_idle_delay;
  float tilt_amount;
  vector3d velocity;
  trail *my_trail;
  entity *my_board_entity;
	entity *my_third_entity; // this just keeps getting better and better
  stringx my_base_name;

  // state
  int my_state;
  int my_previous_state;

  // animation info
  stringx *my_name_anims;
  int my_num_anims;
  stringx *my_board_name_anims;
  int my_board_num_anims;

  vector3d offset;
  float extra_turn, total_extra_turn;

  friend class beach;
  friend class KSEntityState;
  friend class KSReplay;
};

class static_object : public water_object
{
public:
  static_object (entity *ent, const stringx& path);
  virtual ~static_object ();

  bool update (float dt);

  virtual bool parse_params (char** argp, int argc);

private:
  bool dont_move;
};

#endif // _FLOATOBJ_H_
