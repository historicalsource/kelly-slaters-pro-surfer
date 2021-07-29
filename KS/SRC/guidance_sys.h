#ifndef _GUIDANCE_SYS_H_
#define _GUIDANCE_SYS_H_

#include "global.h"
#include "ostimer.h"
#include "random.h"

class physical_interface;

class guidance_system
{
protected:
  physical_interface *owner;

  int flags;

public:
  typedef enum
  {
    _GUIDANCE_GENERIC,
    _GUIDANCE_ROCKET,
    _GUIDANCE_UNKNOWN
  } eGuidanceSysType;

  guidance_system(physical_interface *_owner)
  {
    owner = _owner;
    flags = 0;

    assert(owner);
  }

  virtual ~guidance_system()
  {
    owner = NULL;
  }

  void set_flag(unsigned int flag, bool val = true) { if(val) flags |= flag; else flags &= ~flag; }
  bool is_flagged(unsigned int flag) const          { return((flags & flag) != 0); }

  virtual eGuidanceSysType get_type() { return(_GUIDANCE_GENERIC); }

  virtual void frame_advance(time_value_t t) {};

  virtual void launch(const vector3d &dir, rational_t force);
};


class rocket_guidance_sys : public guidance_system
{
protected:
  vector3d target_pos;
  entity *target;

  rational_t launch_force;
  
  rational_t wobble_timer;
  rational_t guidance_delay;
  rational_t accel_delay;

  rational_t guided_accuracy;
  rational_t turn_factor;
  rational_t accel_factor;

  rational_t full_wobble_timer;
  rational_t full_guidance_delay;
  rational_t full_accel_delay;

  rational_t wobble_timer_var;
  rational_t guidance_delay_var;
  rational_t accel_delay_var;

  void wobble();

public:
  enum
  {
    _ROCKET_GUIDED        = 0x00000001,
    _ROCKET_POINT_GUIDED  = 0x00000002,

    _ROCKET_FLAG_MASK     = 0xffffffff
  };

  rocket_guidance_sys(physical_interface *_owner);
  ~rocket_guidance_sys();

  virtual eGuidanceSysType get_type()                 { return(_GUIDANCE_ROCKET); }

  bool is_guided() const                              { return(is_flagged(_ROCKET_GUIDED)); }
  void set_guided(bool val = true)                    { set_flag(_ROCKET_GUIDED, val); }

  bool is_point_guided() const                        { return(is_flagged(_ROCKET_POINT_GUIDED)); }
  void set_point_guided(bool val = true)              { set_flag(_ROCKET_POINT_GUIDED, val); }

  void set_target(entity *ent);
  void set_target_pos(const vector3d &pos)            { target_pos = pos; }

  rational_t get_guided_accuracy() const              { return(guided_accuracy); }
  rational_t get_turn_factor() const                  { return(turn_factor); }
  rational_t get_accel_factor() const                 { return(accel_factor); }

  void set_guided_accuracy(rational_t f)              { guided_accuracy = f; }
  void set_turn_factor(rational_t f)                  { turn_factor = f; }
  void set_accel_factor(rational_t f)                 { accel_factor = f; }
                                                      
  rational_t get_wobble_timer() const                 { return(full_wobble_timer); }
  rational_t get_guidance_delay() const               { return(full_guidance_delay); }
  rational_t get_accel_delay() const                  { return(full_accel_delay); }
                                                      
  void set_wobble_timer(rational_t t)                 { full_wobble_timer = t; }
  void set_guidance_delay(rational_t t)               { full_guidance_delay = t; }
  void set_accel_delay(rational_t t)                  { full_accel_delay = t; }
                                                      
  rational_t get_wobble_timer_var() const             { return(wobble_timer_var); }
  rational_t get_guidance_delay_var() const           { return(guidance_delay_var); }
  rational_t get_accel_delay_var() const              { return(accel_delay_var); }
                                                      
  void set_wobble_timer_var(rational_t v)             { wobble_timer_var = v; }
  void set_guidance_delay_var(rational_t v)           { guidance_delay_var = v; }
  void set_accel_delay_var(rational_t v)              { accel_delay_var = v; }
                                                      
  rational_t get_wobble_timer_with_var() const        { return(full_wobble_timer + (PLUS_MINUS_ONE*wobble_timer_var)); }
  rational_t get_guidance_delay_with_var() const      { return(full_guidance_delay + (PLUS_MINUS_ONE*guidance_delay_var)); }
  rational_t get_accel_delay_with_var() const         { return(full_accel_delay + (PLUS_MINUS_ONE*accel_delay_var)); }

  void copy(rocket_guidance_sys *sys);

  virtual void launch(const vector3d &dir, rational_t force);
  virtual void frame_advance(time_value_t t);
};

#endif