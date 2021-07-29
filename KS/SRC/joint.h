#ifndef JOINT_H
#define JOINT_H

#include "global.h"
#include "ostimer.h"

class entity;
class vector3d;

//!#include "rigid.h"


class joint
  {
  public:
  	joint() {}
    joint(entity * const _a, /*! rigid_body * const _b, !*/ rational_t _min,rational_t _max,
          const vector3d & _axis_a, const vector3d & _axis_b,
          const vector3d & _loc_a, const vector3d & _loc_b,int _dim, rational_t _friction);
    virtual ~joint() {}

    virtual void frame_advance(time_value_t time_inc);

    bool should_kill(){return kill_me;}

    entity * get_a(){return a;}
//!    rigid_body * get_b(){return b;}
  protected:
    entity * a;
//!    rigid_body * b;
    rational_t min,max;
    vector3d axis_a, axis_b;
    vector3d loc_a, loc_b;
    int dim;
    rational_t friction;
    bool kill_me;
  };

/*
class angular_joint: public joint
  {
  public:
  };
*/

class linear_joint: public joint
  {
  public:
    linear_joint(entity * const _a, /*! rigid_body * const _b,!*/ rational_t _min,rational_t _max,
          const vector3d & _axis_a, const vector3d & _axis_b,
          const vector3d & _loc_a, const vector3d & _loc_b,int _dim, rational_t _friction, bool one_sided);
    virtual ~linear_joint() {}

    virtual void frame_advance(time_value_t time_inc);
  protected:
    bool one_sided;
  };

#endif