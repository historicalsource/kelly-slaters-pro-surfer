/*

  actuator.cpp
  
    Home of hinge_actuators and probably other actuator forms, force generators based on the damped
    oscillator.

*/
#include "global.h"


#include "actuator.h"
#include "hinge.h"
#include "hwmath.h"
#include "oserrmsg.h"
#include "stringx.h"

rational_t const MAX_FORCE_ANGLE = PI/2;

bool NEW_STRIKE_METHOD = true;
rational_t instability = 0.0f; // UNFINISHED <<<<

//////////////////////////////////////////////////////////////////////////////// 
//  hinge actuator
//////////////////////////////////////////////////////////////////////////////// 
hinge_actuator::hinge_actuator(rational_t _k, rational_t _drag, rational_t _strike_scale, hinge * _h)
{
  k = _k;
  drag = _drag;
  strike_scale = _strike_scale;
  h = _h;
  target = 0; //(h->min+h->max)/2;
  strike = false;
  strike_diff_sign = 0;
  scale = 1.0f;
}


void hinge_actuator::frame_advance(time_value_t t)
{
  rational_t vel = h->vel;
  rational_t diff = h->val-target;
  if (diff>MAX_FORCE_ANGLE) diff=MAX_FORCE_ANGLE;
  else if (diff<-MAX_FORCE_ANGLE) diff = -MAX_FORCE_ANGLE;

	assert( target<=10000 && target >= -10000 );
//  if (target>10000) assert(0);
//  else if (target<-10000) assert(0);

  int diff_sign = (diff>0)?1:-1;
  rational_t drag_scale = 1.0f;
  rational_t eff_scale = scale * ((strike)?1.15f:1.0f);

  if (strike)
    if (strike_diff_sign == 0)
    {
      strike_diff_sign = diff_sign;
    }
    else
    {
      if (strike_diff_sign == diff_sign && !h->get_pegged())
      {
        rational_t base_diff;
        if (!NEW_STRIKE_METHOD)
        {
          base_diff = diff;
          diff *= 1000;
          if (diff*diff_sign>MAX_FORCE_ANGLE)
            diff = MAX_FORCE_ANGLE*diff_sign;
          else
            if (diff==0)
              strike = false;
        }
        else
        {
          //PTA SUPER IMPORTANT CHANGE!!!! 3/19/99  New VSIM strike fix experiment!!!!
          base_diff = diff;
          diff *= strike_scale;
          if (diff==0)
            strike = false;
        }
        if (base_diff!=0)
        {
          rational_t force_scale = diff/base_diff; // 3 unless it pegs
          drag_scale = __fsqrt(__fsqrt(force_scale));
        }
      }
      else
      {
        strike = 0;
        strike_diff_sign = 0;
      }
    }

  rational_t eff_vel = vel;
  if (instability)
  {
    if (eff_vel>0)
    {
      eff_vel-=instability;
      if (eff_vel<0) eff_vel = 0;
    }
    else
    {
      eff_vel+=instability;
      if (eff_vel>0) eff_vel = 0;
    }
  }
  rational_t accel = -k*eff_scale*eff_scale*diff-drag*eff_scale*eff_vel*drag_scale;
  h->vel += accel*t;
  h->acceleration_factor += accel*t;

  target = 0;
}


void hinge_actuator::fcs_adjust_target(rational_t t)
{
  target += t;
  assert(t>-10000 && t<100000);
  //if (target>h->max) target = h->max;
  //else if (target<h->min) target = h->min;
}


void hinge_actuator::fcs_set_target(rational_t t)
{
  target = t;
  assert(t>-10000 && t<100000);
  //if (target>h->max) target = h->max;
  //else if (target<h->min) target = h->min;
}
