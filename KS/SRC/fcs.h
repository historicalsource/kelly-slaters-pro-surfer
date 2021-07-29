#ifndef FCS_H
#define FCS_H

#include "algebra.h"
#include "mobject.h"
#include "hwmath.h"
#include "ostimer.h"

class launcher;
//!class actor;
class hinge_actuator;
//!class character;

class force_control_system : public motion_object
{
public:
  force_control_system();
  virtual ~force_control_system() {}

  virtual void frame_advance(time_value_t time_inc)
  { locked = false; }

  virtual void reset_targets() 
  { assert(false); }  // Still only a pattern, but this is kind of a reminder that fcs's should always have one of these.
      

  // "locked" prevents the targets from being changed
  void set_locked(bool _l)
  { locked = _l; }

  bool is_locked() const
  { return locked; }

  // "reset locked" prevents targets from automatically being reset (via reset_targets)
  void set_reset_locked(bool _l)
  { reset_locked = _l; }

  bool is_reset_locked() const
  { return reset_locked; }

protected:
  bool locked, reset_locked;
};


#endif  // FCS_H
