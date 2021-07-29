#ifndef _AI_ACTIONS_H_
#define _AI_ACTIONS_H_

#include "global.h"
#include "ostimer.h"
#include "ai_constants.h"
#include "simple_classes.h"
// BIGCULL #include "damage_interface.h"

class ai_interface;
class entity;
class ai_goal;  

class ai_action
{
public:
  enum
  {
    IN_SERVICE      = 0x00000001
  };

  ai_action(ai_goal *own);
  virtual ~ai_action() { }

  // returns  TRUE = POP_ME
  //         bsptr = PUSH_THIS
  virtual bool frame_advance( time_value_t delta_t ) = 0;

  virtual void going_out_of_service();
  virtual void going_into_service();

  inline bool is_flagged(int flag) const          { return((flags & flag) != 0); }
  inline void set_flag(int flag, bool on = true)  { if(on) flags |= flag; else flags &= ~flag; }

  inline bool is_in_service()                     { return(is_flagged(IN_SERVICE)); }

  inline ai_goal *get_ai_goal() const             { return(owner); }
  ai_interface *get_ai_interface() const;
  entity *get_my_entity() const;
  inline unsigned int get_id() const              { return(id); }

protected:
  ai_goal *owner;
  int flags;
  unsigned int id;

private:
  static unsigned int action_id_counter;
};


class anim_ai_action : public ai_action
{
public:
  anim_ai_action(ai_goal *own);

  void setup(const stringx &play_anim, int slot, const stringx &sound = empty_string, bool loop = false, bool rev = false, bool _non_cosmetic = false, bool _tween = true);

  bool is_non_cosmetic() const      { return(non_cosmetic); }
  bool is_looping() const           { return(looping); }
  bool is_reverse() const           { return(reverse); }
  bool is_tween() const             { return(tween); }

  void set_non_cosmetic(bool t)     { non_cosmetic = t; }
  void set_looping(bool t)          { looping = t; }
  void set_reverse(bool t)          { reverse = t; }
  void set_tween(bool t)            { tween = t; }
  void force_safety_checks(bool t)  { safety_checks = t; }

  virtual bool frame_advance( time_value_t delta_t );
  virtual void going_out_of_service();
  virtual void going_into_service();

  int get_anim_damage_value() const       { return(anim_damage_value); }
  rational_t get_anim_recover() const     { return(anim_recover); }
  rational_t get_anim_recover_var() const { return(anim_recover_var); }
  int get_anim_flags() const              { return(anim_flags); }

protected:
  stringx anim_to_find;
  stringx sound_grp;
  int anim_slot;
  bool looping;
  bool reverse;
  bool non_cosmetic;
  bool tween;

  int anim_damage_value;
  rational_t anim_recover;
  rational_t anim_recover_var;
  int anim_flags;

  vector3d safe_pos;
  bool safety_checks;
};

#if 0 // BIGCULL 
class attack_ai_action : public anim_ai_action
{
public:
  attack_ai_action(ai_goal *own);

  void setup(const stringx &play_anim, const stringx &sound, int slot, eDamageType type = DAMAGE_MELEE, rational_t dmg_mod = 1.0f, rational_t range = 0.0f);

  virtual bool frame_advance( time_value_t delta_t );
  virtual void going_out_of_service();
  virtual void going_into_service();

protected:
  eDamageType attack_type;
  rational_t ext_range;
  rational_t damage_mod;
};


class search_ai_action : public ai_action
{
public:
  search_ai_action(ai_goal *own);

  void setup(rational_t time);

  virtual bool frame_advance( time_value_t delta_t );
  virtual void going_out_of_service();
  virtual void going_into_service();

protected:
  rational_t duration;
};


class jump_ai_action : public ai_action
{
public:
  jump_ai_action(ai_goal *own);

  void setup_rel(rational_t _force);
  void setup_abs(const vector3d &_dir, rational_t _force);
  void setup_targ(const vector3d &target_pos, rational_t xzvel = 10.0f);

  virtual bool frame_advance( time_value_t delta_t );
  virtual void going_out_of_service();
  virtual void going_into_service();

protected:
  typedef enum
  {
    _JUMP_REL,
    _JUMP_ABS,
    _JUMP_TARG
  } eJumpType;

  int state;
  rational_t force;
  vector3d dir;
  rational_t xzvel;
  eJumpType type;
  rational_t grav_mod;
  bool allow_wounded;
};


class dodge_ai_action : public anim_ai_action
{
public:
  dodge_ai_action(ai_goal *own);

  typedef enum
  {
    _DODGE_L,
    _DODGE_R,
    _DODGE_F,
    _DODGE_B,
    _DODGE_LR,
    _DODGE_FB,
    _DODGE_LRB,
    _DODGE_LRF,
    _DODGE
  } eDodgeDir;

  void setup(eDodgeDir dir = _DODGE, bool _allow_damage = true, bool _allow_wounded = false);

  virtual void going_out_of_service();
  virtual void going_into_service();

protected:
  unsigned int sig_id;
  bool allow_damage;
  bool allow_wounded;
  bool was_allow_wounded;
};





class item;
class use_item_ai_action : public ai_action
{
public:
  use_item_ai_action(ai_goal *own);
  virtual void going_out_of_service();
  virtual void going_into_service();

  virtual bool frame_advance( time_value_t delta_t );

  bool used_item;

protected:
  item *get_usable_item();
};


#endif // BIGCULL 

#endif
