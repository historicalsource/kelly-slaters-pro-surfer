#ifndef CONTROLLER_H
#define CONTROLLER_H


#include "po.h"
#include "hwmath.h"
#include "ostimer.h"
#include "project.h"
#include "entity.h"
//!#include "ladder.h"
//#include "crate.h"
#include "item.h"

class theta_and_psi_mcs;
class dolly_and_strafe_mcs;
class character_head_fcs;
//!class character;
class player_controller;
class entity_anim_tree;
class crawl_marker;

class controller
{
public:
  controller()
  {
    active = true;
    deactivate = false;
  }
  virtual ~controller() {}

  bool is_active() const { return active; }
  void set_active( bool yorn )
  {
    if ( is_active() )
    {
      if ( !yorn )
        kill();
    }
    else if ( yorn )
      resurrect();
  }


  virtual void frame_advance(time_value_t time_inc)=0;

  virtual void kill() { active = false; }
  virtual void resurrect() { active = true; }

  virtual void set_combat_mode( bool torf ) {}
  virtual bool get_combat_mode() const { return false; }

  virtual bool is_controller() const            { return(true); }
  virtual bool is_mouselook_controller() const  { return(false); }
  virtual bool is_entity_controller() const     { return(false); }
  virtual bool is_character_controller() const  { return(false); }
  virtual bool is_player_controller() const     { return(false); }

  virtual bool is_joystick_usercam_controller() const  { return(false); }
//  virtual bool is_a_brain() const               { return(false); }
//  virtual bool is_a_turret_brain() const        { return(false); }

protected:
  void copy_instance_data( const controller& b ) { active = b.active; deactivate = b.deactivate; }

  bool active;
  bool deactivate;
};

class mouselook_controller : public controller
{
public:
  mouselook_controller(dolly_and_strafe_mcs* _move_cs,theta_and_psi_mcs* _angle_mcs);

  virtual void frame_advance(time_value_t time_inc);

  virtual bool is_mouselook_controller() const  { return(true); }

private:
  dolly_and_strafe_mcs* move_cs;
  theta_and_psi_mcs* angle_mcs;
};


class joypad_usercam_controller : public controller
{
public:
  joypad_usercam_controller(dolly_and_strafe_mcs* _move_cs,theta_and_psi_mcs* _angle_mcs);

  virtual void frame_advance(time_value_t time_inc);

  virtual bool is_joystick_usercam_controller() const  { return(true); }

private:
  dolly_and_strafe_mcs* move_cs;
  theta_and_psi_mcs* angle_mcs;
};


/*
#if defined(TARGET_PC)
  class edit_controller : public controller
  {
  public:
    edit_controller(dolly_and_strafe_mcs* _move_cs,theta_and_psi_mcs* _angle_mcs);

    virtual void frame_advance(time_value_t time_inc);

  private:
    theta_and_psi_mcs* angle_mcs;
    dolly_and_strafe_mcs* move_cs;
  };
#endif
*/

class entity_controller : public controller
{
public:

  typedef enum
  {
    INVALID = -1,
    NONE = 0,
    WALKING,
    RUNNING,
    STANDING,
    TURNING,
    ATTACKING,
    USING_ITEM,
    DRAW_ITEM,
    HOLSTER_ITEM,
    FRANTIC,
    SCARED,
    WOUNDED,
    DYING,
    JUMPING_FORWARD,
    CRAWLING,
    ROLLING,
    MAX_STATES
  } eControllerState;

  entity_controller( entity* ent );
  virtual ~entity_controller();

  virtual entity_controller* make_instance( entity *ent ) const;


  virtual player_controller * as_player_controller();

  virtual void frame_advance(time_value_t time_inc) {};

  inline entity *get_owner() const            { return(owner); }

  void set_state(eControllerState s)          { state = s; }
  virtual eControllerState get_state() const  { return(state); }

  virtual bool is_entity_controller() const     { return(true); }

  virtual void set_blocking( bool torf ) {}
  virtual bool is_blocking() const { return false;}
  virtual bool is_in_cautious_mode() const { return false; }
  virtual bool is_in_running_mode()  const { return true; }  // characters run by default

  virtual bool is_moving() const { return( ((state == WALKING)||(state==RUNNING)||(state==CRAWLING)) ? true : false ); };

protected:
  void copy_instance_data( const entity_controller& b );

  entity *owner;

  eControllerState state;
};
/*!
class character_controller : public entity_controller
{
public:
  character_controller( character* chr,
                        character_head_fcs* _head_fcs );

  virtual ~character_controller();
  virtual void frame_advance(time_value_t time_inc)=0;

  void set_target_speed_pct(rational_t v);
  void set_target_h_speed_pct(rational_t v);
  void set_d_theta_pct(rational_t v);
  void set_burst(vector3d dir, time_value_t time);
  void set_block_type(int b);
  void set_jump_flag(bool torf);
  void set_crouch_flag(bool torf);
  void set_front_crouch_flag(bool torf);
  void set_rear_crouch_flag(bool torf);
  void set_flip(bool torf);

  void set_neck_target_extend(rational_t t);
  void set_neck_target_theta(rational_t t);
  void set_neck_target_psi(rational_t t);
  void set_head_target_psi(rational_t t);
  void set_head_target_phi(rational_t t);
  void set_jaw_target_psi(rational_t t);


  virtual int get_rank()
  {
    return 100000;    // A LARGE NUMBER
  }
  virtual void set_rank(int r)
  {
    //assert(0);
  }

  void set_recording( bool v );

  virtual bool is_character_controller() const  { return(true); }
  
protected:
  character_head_fcs * head_fcs;
  bool recording;
};

!*/
enum input_t
{
  HERO_NONE                 = 0x00000001,
  HERO_FIRE                 = 0x00000002,
  HERO_THROW = HERO_FIRE,
  HERO_PUNCH = HERO_FIRE,
  HERO_KICK                 = 0x00000004,
  HERO_JUMP_KICK            = 0x00000008,
  HERO_DRAW                 = 0x00000010,
  HERO_SHEATHE = HERO_DRAW,
  HERO_INV_SCROLL           = 0x00000020,
  HERO_LOOK_AROUND          = 0x00000040,
  HERO_ACTIVATE             = 0x00000080,
  HERO_JUMP                 = 0x00000100,
  HERO_FLY                  = 0x00000200,
  HERO_JUMP_FORW            = 0x00000400,
  HERO_JUMP_BACKW           = 0x00000800,
  HERO_DODGE_L              = 0x00001000,
  HERO_DODGE_R              = 0x00002000,

  HERO_ZOOM_IN              = 0x00004000,
  HERO_ZOOM_OUT             = 0x00008000,
  HERO_ZOOM_SLOW            = 0x00010000,

  HERO_TURBO                = 0x00020000,
  HERO_STEALTH              = 0x00040000,
};


enum hero_state_t
{
  STATE_NONE,
  STATE_PAUSE,
  STATE_SCROLL_INV,
  STATE_LOOK_AROUND,
  STATE_CLIMB,
  STATE_CLIMB_LEDGE,
  STATE_CLIMB_OFF_LEDGE,
  STATE_JUMP_TO_LEDGE,
  STATE_FALLING,
  STATE_DODGE_LEFT,
  STATE_DODGE_RIGHT,
  STATE_BROAD_JUMP,
  STATE_CLIMB_1M_LEDGE,
  STATE_GRAB_CRATE,
  STATE_PUSHING_CRATE,
  STATE_STACKING_CRATE,
  STATE_UNSTACKING_CRATE,
  STATE_PULLING_CRATE,
  STATE_RELEASING_CRATE,
  STATE_SP_PUNCH,
  STATE_SP_BASH_PUNCH,
  STATE_SP_KICK,
  STATE_SP_JUMP_KICK,

  STATE_DYING,
  STATE_GETTING_DAMAGE,
  STATE_GETTING_HIT,
  STATE_GETTING_SHOT,
  STATE_GETTING_EXPLOSIVE,
  STATE_STUN,

  STATE_HITTING,

//  STATE_BLOCKING,

  STATE_LAUNCH_UP_GRAB,
  STATE_LAUNCH_F_GRAB,

  INTERNAL_STATES_END,
  ADVENTURE_MODE_START = INTERNAL_STATES_END-1,

  ADVENTURE_IDLE,
  ADVENTURE_GO_FORWARD,
  ADVENTURE_GO_BACKWARD,
  ADVENTURE_GO_FL,
  ADVENTURE_GO_FR,
  ADVENTURE_GO_BL,
  ADVENTURE_GO_BR,
  ADVENTURE_ROTATE_LEFT,
  ADVENTURE_ROTATE_RIGHT,
  ADVENTURE_PUNCH,
  ADVENTURE_BASH_PUNCH,
  ADVENTURE_KICK,
  ADVENTURE_JUMP_KICK,
  ADVENTURE_JUMP,
  ADVENTURE_JUMP_FORWARD,
  ADVENTURE_JUMP_BACKWARD,
  SILENT_KILL,

  ADVENTURE_MODE_END,
  CRAWL_MODE_START = ADVENTURE_MODE_END-1,

  CRAWL_IDLE,
  CRAWL_FORWARD,
  CRAWL_BACKWARD,
  CRAWL_FL,
  CRAWL_FR,
  CRAWL_BL,
  CRAWL_BR,
  CRAWL_ROTATE_LEFT,
  CRAWL_ROTATE_RIGHT,
  CRAWL_IDLE0,
  CRAWL_IDLE1,
  CRAWL_IDLE2,
  CRAWL_IDLE3,
  CRAWL_IDLE4,
  CRAWL_IDLE5,
  CRAWL_IDLE6,

  CRAWL_MODE_END,
};


inline bool is_internal_state( hero_state_t s )   { return s<INTERNAL_STATES_END; }
inline bool is_adventure_mode( hero_state_t s )   { return s>ADVENTURE_MODE_START && s<ADVENTURE_MODE_END; }
inline bool is_crawl_mode( hero_state_t s )       { return s>CRAWL_MODE_START && s<CRAWL_MODE_END; }
inline bool is_immovable_mode( hero_state_t s )   { return s==ADVENTURE_IDLE||s==ADVENTURE_ROTATE_LEFT||s==ADVENTURE_ROTATE_RIGHT||s==CRAWL_IDLE||(s>=CRAWL_IDLE0&&s<=CRAWL_IDLE6);}
inline bool is_jumping_forward( hero_state_t s )  { return s==STATE_BROAD_JUMP||s==ADVENTURE_JUMP_FORWARD;}
inline bool is_running_forward( hero_state_t s )  { return s==ADVENTURE_GO_FORWARD||s==ADVENTURE_GO_FL||s==ADVENTURE_GO_FR;}
inline bool is_jumping( hero_state_t s )          { return s==STATE_DODGE_LEFT||s==STATE_DODGE_RIGHT||s==STATE_BROAD_JUMP||(s>=ADVENTURE_JUMP&&s<=ADVENTURE_JUMP_BACKWARD);}
inline bool is_attacking( hero_state_t s )        { return (s>=STATE_SP_PUNCH&&s<=STATE_SP_JUMP_KICK)||(s>=ADVENTURE_PUNCH&&s<=ADVENTURE_JUMP_KICK);}
//inline bool is_getting_hit( hero_state_t s )      { return s>=STATE_DYING&&s<=STATE_GETTING_EXPLOSIVE;}
inline bool is_getting_hit( hero_state_t s )      { return s!=STATE_GETTING_SHOT&&s>=STATE_DYING&&s<=STATE_GETTING_EXPLOSIVE;}
inline bool is_hitting( hero_state_t s )          { return s==ADVENTURE_PUNCH||s==ADVENTURE_BASH_PUNCH||s==ADVENTURE_KICK||s==ADVENTURE_JUMP_KICK||s==STATE_SP_PUNCH||s==STATE_SP_BASH_PUNCH||s==STATE_SP_KICK||s==STATE_SP_JUMP_KICK||s==STATE_HITTING;}
inline bool is_climbing( hero_state_t s )         { return (s==STATE_CLIMB||s==STATE_CLIMB_LEDGE||s==STATE_JUMP_TO_LEDGE||s==STATE_CLIMB_1M_LEDGE||s==STATE_CLIMB_OFF_LEDGE);}
inline bool is_crate_mode( hero_state_t s )       { return (s==STATE_GRAB_CRATE||s==STATE_PUSHING_CRATE||s==STATE_STACKING_CRATE||s==STATE_PULLING_CRATE||s==STATE_RELEASING_CRATE);}
//inline bool is_blocking_mode( hero_state_t s )    { return s==STATE_BLOCKING;}
inline bool draw_disabled_state( hero_state_t s ) { return (s>=CRAWL_IDLE&&s<=CRAWL_IDLE6)||(s>=STATE_CLIMB&&s<=STATE_JUMP_TO_LEDGE)||s==STATE_LAUNCH_UP_GRAB||s==STATE_LAUNCH_F_GRAB;}


struct input_state
{
  bool        d; // DELTA, IE: THIS STATE HAS CHANGED
  bool        s; // STATE, IE: WHETHER THIS CURRENTLY PRESSED
  rational_t  t; //  TIME, HOW LONG THIS STATE HAS BEEN MAINTAINED
};


enum cntrl_t
{
  CNTRL_NONE = 0,
  CNTRL_TRANSITION,
  CNTRL_IDLE,
  CNTRL_RUN,
  CNTRL_CRAWL,
  CNTRL_ROTATE,
  CNTRL_SCROLL_INV,
  CNTRL_FRSTP,
  CNTRL_HIT,
  CNTRL_JUMP,
  CNTRL_FLY,
  CNTRL_FALL,
  CNTRL_CLIMB_LEDGE,
  CNTRL_JUMP_TO_LEDGE,
  CNTRL_CLIMB_1M_LEDGE,
  CNTRL_WAIT_ANIM_END,
  CNTRL_HANDLE_CRATE,
  CNTRL_GET_HIT,
  CNTRL_BLOCKING
};


enum
{
  ATTACK_PUNCH,
  ATTACK_BASH_PUNCH,
  ATTACK_KICK,
  ATTACK_JUMP_KICK
};


class item_widget;


//---------------------------------------------------------------
class combo_attack_info
{
public:
  combo_attack_info()
  { t0=0.0f;t1=0.0f;t2=0.0f;btn=0;newstate=empty_string;waiting_to_transit=0; }
  combo_attack_info( time_value_t tf, time_value_t tt, time_value_t ts, int butt, stringx& newst )
  { t0=tf;t1=tt;t2=ts;btn=butt;newstate=newst;waiting_to_transit=0; }

  void copy( const combo_attack_info &cai )
  { t0=cai.t0;t1=cai.t1;t2=cai.t2;btn=cai.btn;newstate=cai.newstate;waiting_to_transit=cai.waiting_to_transit; }
  void copy( const combo_attack_info *cai )
  { t0=cai->t0;t1=cai->t1;t2=cai->t2;btn=cai->btn;newstate=cai->newstate;waiting_to_transit=cai->waiting_to_transit; }
  
  combo_attack_info( const combo_attack_info &cai )
  { copy(cai); }

  combo_attack_info& operator=(const combo_attack_info &b) 
  {
    copy(b);
    return *this;
  }

  time_value_t t0;
  time_value_t t1;
  time_value_t t2;
  int btn;
  int waiting_to_transit;
  stringx newstate;
};
//---------------------------------------------------------------


struct secondary_anim
{
  int state;
  int slot;
  game_clock_t start_time;
  time_value_t ofs_time;
  time_value_t elapsed() const 
  {
    return start_time.elapsed() + ofs_time; 
  }
  unsigned short anim_flags;
};

/*!
class player_controller : public character_controller
{
public:
  player_controller( character* chr,
                     character_head_fcs* _head_fcs );

  virtual ~player_controller();
  virtual void frame_advance(time_value_t time_inc);
  void frame_advance1(time_value_t time_inc);
  virtual player_controller * as_player_controller() { return this; }
  character * get_targeted_opponent();

  bool is_item_wheel_up() const { return item_wheel_up; }

  void print_debug_info() const;

  hero_state_t get_hero_state() const { return hero_state; }
  bool is_in_crawl_mode() const { return is_crawl_mode(hero_state); }
  virtual bool is_in_running_mode()  const;

  virtual bool is_in_inventory();

  virtual void kill();
  virtual void resurrect();

  void clear_c_index() { c_index = CNTRL_NONE; }

  virtual bool is_player_controller() const     { return(true); }

private:
  void get_default_name( stringx& st );
  rational_t attack_time;
  int cur_attack_move;

  // for measuring how long the stick has been in a single position
  rational_t old_stick, steady_stick_time;

  // for slerping into crawl
  rational_t target_crawl_theta, cur_crawl_theta;
  vector3d target_crawl_pos;
  bool reached_crawl_pos;
  bool reached_crawl_angle;

  // Auto-aiming
  bool auto_aim_angle_finished, auto_aim_angle_locked;
  bool auto_aim_range_finished, auto_aim_range_locked;
  rational_t auto_aim_initial_angle;
  rational_t auto_aim_initial_range;
  vector3d auto_aim_locked_target;
  character * auto_aim_locked_enemy;

  // Head movement
  rational_t head_psi;
  rational_t head_theta;

  vector2d mousedelta;
  angle_t mousedir;
  rational_t mouseturn;
  rational_t mouserun;

  // move editing functions
  bool sweet_spot_found;
  po move_start_po;

  // on the fly swing adjustment
  rational_t get_attack_target_phi_adj();

  // mouselook
  bool item_wheel_up;
  rational_t wheel_counter;

  // item usage
  rational_t item_timer;
  bool using_item_timer;
  bool throwing_grenade;

  list<secondary_anim *> sec_anim_stack;

  bool finish_drawn_item();
  bool finish_holster_item();

private:
  cntrl_t             c_index, pause_sav_cntrl;
  input_state         buttons[7];
  input_t             last_res;
  hero_state_t         hero_state, hero_next_state, hero_saved_state;
  bool                camera_is_captured;
  int                 camera_move;
  po                  cam_sav_po, cam_fp_po;
  rational_t          start_time, cam_pitch, cam_roll;
  rational_t          last_v, last_h;
  bool                proc_jump;
  vector3d            last_pos, climbing_start_pos, out_normal, hit_react_face;
  int                 cur_anim_idx;
  int                 type_of_the_ledge;
  rational_t          curr_state_time, jamming_time;
  bool                do_not_jump, jamming;
  int                 crate_type;
  crate               *my_crate;
  bool                frozen;
  bool                aiming_mcs_was_active, getting_shot, exhursion_playing;
  bool                blocking;
  bool                melee_engaged;
  bool                saved_physics_flag;
  rational_t          stun_timer;
  vector3d            fall_pos;
  vector3d            crate_climb_start_pos, crate_handling_pos, crate_handling_facing;

  inline void         activate_nano();
  inline void         deactivate_nano();

  item_widget *item_wheel;
  item *next_inv_item_ptr;

  enum
  {
    ITEM_SHEATHED = 0,
    ITEM_SHEATHING = 1,
    ITEM_DRAWING = 2,
    ITEM_DRAWN = 3
  };
  char item_state, req_item_state;
  bool forcing_draw;  // used in forcing drawn state

  void init( bool from_scratch = true );
  void reset_inputs();
  rational_t stick_level( rational_t val );
  input_t get_input( rational_t *v, rational_t *h, rational_t delta_t = 0.0f );
  rational_t get_turn_amount();
  rational_t get_run_amount();
  cntrl_t get_control_index( rational_t delta_t );
  hero_state_t get_stick_dir( cntrl_t caller );
  void fix_position( rational_t delta_t );
  void punch_fire();
  void activate_handle();
  void rotate_chr( rational_t arg );
  int get_ledge( bool _1m = false, hero_state_t state_to_use = STATE_NONE );
  vector3d get_chr_offset_pos( const vector3d& offset, bool ignore_anim = false, bool use_prev_pos = false );
  void build_regions( vector<region_node*> *regs, region_node *r, const vector3d& o, const vector3d& d );
//  void adjust_collision_capsule();
  int get_crate( bool forse_ledge = false );
  hero_state_t add_attack_move( int attk_type );
  bool allow_autofire() const;
  void react_to_hit();
  hero_state_t get_state_anim_idx_facing( int *anim_idx, bool *hit_in_back, vector3d *throw_vect = NULL );
  void melee_adjust_direction( time_value_t t );
  void melee_check_engage();

  void process_item_state( time_value_t time_inc, bool anim_done = false );
  void sheathe();

  void process_sec_anim_stack( bool pop = true );
  bool pop_sec_anim_stack(int state = -1, bool pop = true, bool process = true);

  void play_SecAnim( int state,
                     time_value_t start_time,
                     unsigned short anim_flags,
                     int _slot = ANIM_SECONDARY_A );
//  void play_SecAnim( const stringx& filename,
//                     time_value_t start_time,
//                     unsigned short anim_flags );
  void kill_SecAnim();

public:
  friend cntrl_t hero_transition( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_idle( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_run( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_crawl( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_rotate( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t scroll_inventory( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_first_pers( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_hit( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_jump( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_climb_ledge( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_jump_to_ledge( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_fly( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_fall( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_climb_1m_ledge( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_wait_anim_end( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_handle_crate( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_getting_hit( rational_t delta_t, class player_controller *p_this );
  friend cntrl_t hero_blocking( rational_t delta_t, class player_controller *p_this );

  void force_sheathe_item();

  void set_frozen( bool torf, bool reimburse = true );
  bool is_frozen() const { return frozen; }

  virtual void set_blocking( bool torf ) { blocking = torf;}
  virtual bool is_blocking() const { return blocking;}

  int get_current_animation_index() const { return cur_anim_idx;}
    
  virtual void reset_anim();

  virtual eControllerState get_state() const;

  void sheathe_check();
  void draw_sheathe();
  void force_draw();
  bool item_wheel_is_visible();
  void show_item_wheel();
  void hide_item_wheel();
  void add_perm_item();
  void remove_perm_item();
  void add_inv_item();
  bool next_inv_item();
  bool prev_inv_item();
  void use_inv_item();
  void apply_inv_item_effects();
  void item_activation_effect();
  void item_deactivation_effect();
  void set_current_item_color(rational_t r = 1.0f, rational_t g = 1.0f, rational_t b = 1.0f, rational_t a = 1.0f);

  void change_to_item(item *itm, bool force = false);

  void set_fall_pos(const vector3d &vec)    { fall_pos = vec; }
  vector3d get_fall_pos() const             { return(fall_pos); }

  bool item_is_sheathed() { return ( item_state == ITEM_SHEATHED ); }

  void begin_sniper_mode();
  void end_sniper_mode();

  bool is_melee_engaged() const { return melee_engaged;}

public:
  // SUPPORT FOR HTH ATTACK MODES AND CHAINING
  void read_combo_anim_stuff( const stringx& combo_filename );
  map<stringx,stringx> sub_states; // < state_name, anim_name >
  map<stringx,int> sub_attacktypes; // < state_name, [0..1] >
  map<stringx,int> sub_attackdamage; // < state_name, [0..1] >
  map<stringx,vector<combo_attack_info*> > combo_map;
  stringx combo_state_name;
  bool combo_is_new;
  // "Ph'nglui mglw'nafh Cthulhu R'lyeh wagn'nagl fhtagn." 

  signed char inv_switch;
  bool reset_sniper;
  bool sniper_lock_triggers;
};
!*/

//!entity *compute_action_entity( character* chr );
//!bool is_crawling(character *chr);
//!crawl_marker *compute_crawl_marker( character* chr, rational_t dir = 0.0f );
//!ladder *compute_ladder_entity( character* chr );


#endif
