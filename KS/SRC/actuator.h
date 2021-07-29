#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "generator.h"
#include "hwmath.h"
class hinge;

const rational_t DEFAULT_ACTUATOR_STRIKE_SCALE = 3.5f;

class hinge_actuator: public force_generator
{
public:
  virtual ~hinge_actuator() {}

  hinge_actuator(rational_t _k, rational_t _drag, rational_t _strike_scale, hinge * _h);

  virtual void frame_advance(time_value_t time_inc);

  void fcs_adjust_target(rational_t t);
  void fcs_set_target(rational_t t);

  void fcs_set_strike(bool t)
  {
    strike = t;
  }
  bool fcs_get_strike() { return strike; }

  rational_t get_target() const { return target; }

  void set_scale(rational_t s) { scale = s; }

  rational_t get_k() const { return k; }
  rational_t get_drag() const { return drag; }
  rational_t get_strike_scale() const { return strike_scale; } 
  void set_strike_scale(rational_t ss) { strike_scale=ss; } 

  bool get_strike() { return strike; }

private:
  rational_t target, k, drag, strike_scale;
  rational_t scale;
  hinge * h;
  bool strike;
  int strike_diff_sign;
};

#endif