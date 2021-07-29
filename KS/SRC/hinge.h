#ifndef HINGE_H
#define HINGE_H

#include "algebra.h"
#include "ostimer.h"
class po;
class vector3d;
class rigid_body;
class hinge_actuator;
class limb;

class hinge
{
public:
  hinge(vector3d _axis, rational_t _min, rational_t _max, limb * _my_limb);
  ~hinge();

  hinge* make_instance() const;

  void advance(time_value_t t);
  void add_actuator(rational_t k, rational_t drag, rational_t strike_scale);

  po const & build_po(po * result, bool shadow = false);
  vector3d build_position();

  void set_val(rational_t v);
  void set_vel(rational_t v);

  void adjust_val(rational_t v);
  void adjust_vel(rational_t v, bool impulse=false);

  hinge_actuator * get_actuator() const { return a; }

  // flag set each frame to report whether the actuator hit one of its limits
  bool get_pegged() const { return pegged; }

  // flags whether or not to use min and max limits
  void set_bounded(bool p) {bounded = p;}
  bool get_bounded() const {return bounded;}

  rational_t get_min() const { return min; }
  rational_t get_max() const { return max; }
  rational_t get_val() const { return val; }
  rational_t get_shadow_val() const { return shadow_val; }
  rational_t get_vel() const { return vel; }

  // IK parameters.  Used by leg, probably other things
  rational_t get_rest() const { return rest; }
  void set_rest(rational_t r) { rest=r; }
  rational_t get_tension() const { return tension; }
  void set_tension(rational_t t) { tension=t; }

  bool is_active() const { return active; }
  void set_active(bool a) { active=a; }

  const vector3d& get_axis() const { return axis; }

  rational_t get_lifetime() const { return lifetime; }
  rational_t get_last_second_val() const { return last_second_val; }
  rational_t get_this_second_val() const { return this_second_val; }

private:
  vector3d axis;
  rational_t min,max,rest,tension;

  rational_t val,vel;
  rational_t acceleration_factor;
  hinge_actuator * a;
  bool pegged;
  bool active;
  bool bounded;
  rational_t shadow_val;
  rational_t lifetime, this_second_val, last_second_val;

  limb * my_limb;

  friend class hinge_actuator;
};

#endif