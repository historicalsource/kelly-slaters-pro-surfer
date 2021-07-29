////////////////////////////////////////////////////////////////////////////////
/*
  hinge.cpp

  Also home of 'hinge', a simple degree of freedom between limbs.

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "hinge.h"
#include "actuator.h"
#include "po.h"
//!#include "limb.h"
//!#include "actor.h"

////////////////////////////////////////////////////////////////////////////////
//    hinge
////////////////////////////////////////////////////////////////////////////////
hinge::hinge(vector3d _axis, rational_t _min, rational_t _max, limb * _my_limb)
  {
  axis = _axis;
  min = _min;
  max = _max;
  my_limb = _my_limb;
  a = NULL;
  vel = 0; //1.0f;
  val = (min+max)/2;
  acceleration_factor = 0;
  shadow_val = val;
  rest = val;
  tension = 1.0f;
  pegged = false;
  active = true;
  bounded = false;
  lifetime = last_second_val = this_second_val = 0;
  }


hinge::~hinge()
  {
  delete a;
  }


hinge* hinge::make_instance() const
  {
  hinge* h = NEW hinge( axis, min, max , my_limb);
  h->set_rest( rest );
  h->set_tension( tension );
  h->set_bounded( bounded );
  if ( a )
    h->add_actuator( a->get_k(), a->get_drag(), a->get_strike_scale() );
  return h;
  }


void hinge::advance(time_value_t t)
  {
  rational_t new_lifetime = lifetime + t;
  if ( ((int) new_lifetime)>((int) lifetime) )
    {
    last_second_val = this_second_val;
    this_second_val = val;
    }
  lifetime = new_lifetime;

  pegged = false;
  if (a)
    a->frame_advance(t);
  rational_t val_inc = (vel-acceleration_factor/2)*t*(1-my_limb->get_my_actor()->get_paralysis_factor());
  shadow_val = val+val_inc/2;
  val += val_inc;
  acceleration_factor = 0;

  bool changed = true;

  if (bounded)
    {
    if (val<min)
      {
      val=min;
      vel = 0;
      pegged = true;
      }
    else if (val>max)
      {
      val = max;
      vel = 0;
      pegged = true;
      }
    }
  // force transfer >>>>

/*
  while (changed)
    {
    changed = false;
    if (val<min)
      {
      val=min-(val-min);
      vel=-vel;
      changed = true;
      }
    else if (val>max)
      {
      val=max-(val-max);
      vel=-vel;
      changed = true;
      }
    }
*/
  }


po const & hinge::build_po(po * ret, bool shadow)
{
  if(axis.x == 1.0f)
    ret->set_rotate_x(shadow?shadow_val:val);
  else if(axis.y == 1.0f)
    ret->set_rotate_y(shadow?shadow_val:val);
  else if(axis.z == 1.0f)
    ret->set_rotate_z(shadow?shadow_val:val);
  else
    ret->set_rot(axis,(shadow?shadow_val:val));

  return *ret;
}


vector3d hinge::build_position()
{
  vector3d result;
  result = axis*val;
  return result;
}


void hinge::set_val(rational_t v)
{
  val = v;
}


void hinge::set_vel(rational_t v)
  {
  vel = v;
  acceleration_factor = 0;
  }

void hinge::adjust_val(rational_t v)
  {
  val += v;
  }


void hinge::adjust_vel(rational_t v, bool impulse)
  {
  vel += v;
  if (!impulse)
    acceleration_factor += v;
  }

void hinge::add_actuator(rational_t k, rational_t drag, rational_t strike_scale)
  {
  a = NEW hinge_actuator(k, drag, strike_scale, this);
  }
