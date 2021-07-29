#ifndef _AI_GOALS_H_
#define _AI_GOALS_H_

#include "global.h"
#include "ostimer.h"
#include "pstring.h"
#include "color.h"
#include "simple_classes.h"
#include "ai_constants.h"

class ai_interface;
class entity;
class ai_action;

#define AI_GOAL_MAKE_COPY_MAC(g) ai_goal* g##_ai_goal::make_copy(ai_interface *own) { g##_ai_goal *goal = NEW g##_ai_goal(own); return((ai_goal*)goal); }

class ai_goal
{
protected:
  friend class ai_interface;

  ai_interface *owner;
  rational_t priority;
  rational_t priority_modifier;
  bool in_service;
  list<ai_action *> actions;
  pstring type;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own) = 0;

  virtual void dump_actions();
  virtual void dump_action(ai_action *act);

  void read_data(chunk_file& fs);
  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  ai_goal(ai_interface *own);
  virtual ~ai_goal();

  virtual rational_t calculate_priority(time_value_t t) = 0;
  inline rational_t get_priority()                { return(priority*priority_modifier); }

  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual unsigned int add_action(ai_action *act);
  virtual bool running_action(unsigned int id);
  virtual void dump_action(unsigned int id);

  inline ai_interface *get_ai_interface() const   { return(owner); }
  entity *get_my_entity() const;

  inline pstring get_type() const                 { return(type); }

  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);

  virtual bool set_str(const pstring &att, const stringx &str);
  virtual bool get_str(const pstring &att, stringx &str);

protected:
  static int compare( const void* x1, const void* x2 )
  {
    rational_t diff = (*((ai_goal**)x1))->get_priority() - (*((ai_goal**)x2))->get_priority();
    if ( diff < 0.0f )
      return 1;
    if ( diff > 0.0f )
      return -1;
    return 0;
  }
};



class idle_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

public:
  idle_ai_goal(ai_interface *own);
  virtual ~idle_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};

class disable_ai_goal : public idle_ai_goal
{
protected:
  friend class ai_interface;

  virtual ai_goal* make_copy(ai_interface *own);

  virtual rational_t frame_advance(time_value_t t);

public:
  disable_ai_goal(ai_interface *own);
  virtual ~disable_ai_goal();

  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual rational_t calculate_priority(time_value_t t);
};


#if 0 // BIGCULL

class death_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  bool fade;
  rational_t fade_timer;
  rational_t fade_time;
  color32 ren_col;
  bool coll_active;

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  death_ai_goal(ai_interface *own);
  virtual ~death_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};


class wounded_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  bool					first_frame;
  unsigned int	knock_down;
  bool					synced;
  bool					combo;

public:
  wounded_ai_goal(ai_interface *own);
  virtual ~wounded_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};





class exploded_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  vector3d launch_dir;
  rational_t launch_force;

  bool running;
  bool hit_from_behind;
  bool landing;
	bool flying;
	bool recovered_from_fall;
  int num_bounces;

  rational_t grav_mod;
  bool grav_enabled;

  vector3d blast_pos;

  stringx launch_f_animation;
  stringx launch_b_animation;

public:
  exploded_ai_goal(ai_interface *own);
  virtual ~exploded_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};





class prone_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  simple_timer recover_timer;
  rational_t prone_time;
  rational_t prone_time_var;

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  prone_ai_goal(ai_interface *own);
  virtual ~prone_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};




class coward_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  rational_t min_fear_cower;
  rational_t min_fear_run;

  bool running;
  bool cowering;

  virtual void handle_chunk(chunk_file& fs, stringx &label);


public:
  coward_ai_goal(ai_interface *own);
  virtual ~coward_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};



class threaten_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  rational_t attack_time;
  rational_t attack_time_var;
  rational_t attack_timer;

  rational_t threaten_time;
  rational_t threaten_time_var;
  rational_t threaten_timer;

  entity *target;

  int threaten_team;

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  threaten_ai_goal(ai_interface *own);
  virtual ~threaten_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();

//  virtual bool set_str(const pstring &att, const stringx &str);
//  virtual bool get_str(const pstring &att, stringx &str);
  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};



class guard_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  vector3d guard_pos;
  vector3d guard_dir;
  bool first_entry;
  bool returning;
  bool changed;

public:
  guard_ai_goal(ai_interface *own);
  virtual ~guard_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};



class patrol_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  vector3d last_pos;
  vector3d patrol_pos;
  vector3d next_patrol;
  bool reset_patrol;
  bool running_speed;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

public:
  patrol_ai_goal(ai_interface *own);
  virtual ~patrol_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};

class follow_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  entity *leader;
  stringx leader_name;
  rational_t max_dist;

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  follow_ai_goal(ai_interface *own);
  virtual ~follow_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual bool set_str(const pstring &att, const stringx &str);
  virtual bool get_str(const pstring &att, stringx &str);
  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};


class slugger_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  rational_t reload;

  virtual ai_goal* make_copy(ai_interface *own);

public:
  slugger_ai_goal(ai_interface *own);
  virtual ~slugger_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};

class moving_shooter2_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  rational_t reload;
  rational_t move_timer;
  rational_t accuracy;

  virtual ai_goal* make_copy(ai_interface *own);

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  moving_shooter2_ai_goal(ai_interface *own);
  virtual ~moving_shooter2_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};


class camper_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  rational_t reload;
  rational_t accuracy;

  virtual ai_goal* make_copy(ai_interface *own);

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  camper_ai_goal(ai_interface *own);
  virtual ~camper_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual bool set_num(const pstring &att, rational_t val);
  virtual bool get_num(const pstring &att, rational_t &val);
};



class investigate_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);
  bool complete;
  vector3d target;
  bool search;
  bool did_search;

public:
  investigate_ai_goal(ai_interface *own);
  virtual ~investigate_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};



class switch_obj;
class alarm_pusher_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  static alarm_pusher_ai_goal *pushers[2];

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);
  switch_obj *alarm_button;
  bool go_hit_alarm;

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  alarm_pusher_ai_goal(ai_interface *own);
  virtual ~alarm_pusher_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};



class cover_test_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

public:
  cover_test_ai_goal(ai_interface *own);
  virtual ~cover_test_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};


class movement_test_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  rational_t timers[10];

public:
  movement_test_ai_goal(ai_interface *own);
  virtual ~movement_test_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};


class path_test_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  simple_timer timer;
  bool moving;

public:
  path_test_ai_goal(ai_interface *own);
  virtual ~path_test_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};

class gun_test_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  simple_timer test_timer1;
  simple_timer test_timer2;

  bool rounds;

public:
  gun_test_ai_goal(ai_interface *own);
  virtual ~gun_test_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};


class anim_test_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  rational_t timers[10];
  rational_t blenda;
  rational_t blendb;
  rational_t blenda_dir;
  rational_t blendb_dir;

public:
  anim_test_ai_goal(ai_interface *own);
  virtual ~anim_test_ai_goal();

  virtual rational_t calculate_priority(time_value_t t);
  virtual void going_into_service();
  virtual void going_out_of_service();
};

#endif // BIGCULL

#endif
