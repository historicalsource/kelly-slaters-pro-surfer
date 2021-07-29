/*

  joint.cpp
     Home of various forms of joints.
*/


#include "global.h"

//!#include "character.h"
#include "joint.h"
#include "entity.h"
#include "physical_interface.h"
//!#include "actor.h"
//!#include "limb.h"
#include "osalloc.h"

//!#include "character.h"
#include "colgeom.h"
#include "pmesh.h"
//#include "colcap.h"
#include "collide.h"
#include "colmesh.h"
#include "face.h"
#include "oserrmsg.h"
#include "hwmath.h"
//!#include "rigid.h"
#include "profiler.h"
#include "terrain.h"
//#include "fxman.h"
#include "capsule.h"
#include "vsplit.h"
#include "osdevopts.h"

#include "physical_interface.h"


////////////////////////////////////////////////////////////////
// joint
////////////////////////////////////////////////////////////////

joint::joint(entity * const _a, /*!rigid_body * const _b,!*/ rational_t _min,rational_t _max,
      const vector3d & _axis_a, const vector3d & _axis_b,
      const vector3d & _loc_a, const vector3d & _loc_b,int _dim, rational_t _friction)
  {
  a = _a;
//!  b = _b;
  min = _min;
  max = _max;
  axis_a = _axis_a;
  axis_b = _axis_b;
  loc_a = _loc_a;
  loc_b = _loc_b;
  dim = _dim;
  friction = _friction;

  kill_me = false;
  }


void joint::frame_advance(time_value_t time_inc)
  {
  }



////////////////////////////////////////////////////////////////
// linear_joint
////////////////////////////////////////////////////////////////


linear_joint::linear_joint(entity * const _a, /*!rigid_body * const _b, !*/rational_t _min,rational_t _max,
          const vector3d & _axis_a, const vector3d & _axis_b,
          const vector3d & _loc_a, const vector3d & _loc_b,int _dim,rational_t _friction, bool _one_sided) :
joint(_a,/*! _b, !*/_min,_max,_axis_a,_axis_b,_loc_a,_loc_b,_dim, _friction)
{
  one_sided = _one_sided;
  assert(axis_a.x!=0 || axis_a.y!=0 || axis_a.z!=0);
}


//entity * g_a;

void linear_joint::frame_advance(time_value_t time_inc)
{
  kill_me = false;
  // only case implemented so far is 2 dimensional joint between entity and the world.
  if (dim==2/*! && b == NULL !*/)
  {
    // clip entity's velocity and position to itself.
    vector3d vel;
    a->get_velocity(&vel);
    a->update_abs_po_reverse();
    vector3d posn = a->get_abs_position();
//    vector3d oldposn = a->get_abs_position();
//    rational_t old_yvel = vel.y;

    vector3d deepest = a->get_updated_closest_point_along_dir(axis_b);
    rational_t depth = dot(loc_b-deepest,axis_b); //rad-dot(posn-loc_b,axis_b);
    //g_a = a;

    if (depth>0)
      posn += depth*axis_b;
    rational_t bad_vel_mag = -dot(vel,axis_b);
    if (!one_sided || bad_vel_mag>=0)
    {
      vector3d vel_diff = axis_b*bad_vel_mag;
      vel += vel_diff;
      rational_t normal_accel_mag = vel_diff.length();
      vector3d friction_dir = -vel.normalize();
      vector3d friction_accel = (normal_accel_mag+.1f)*friction*friction_dir;

      // trying this to prevent people from force-dragging themselves down steep cliffs to avoid a fall.
      //friction_accel.y = 0;  // experiment #1
      friction_accel *= __fabs(dot(axis_b,YVEC));  // experiment #2
      if (friction_accel.y>0)
        friction_accel.y = 0;

      vector3d new_vel = vel+friction_accel;
      if (dot(new_vel,vel)<=0) new_vel = ZEROVEC;
      vel = new_vel;
      // a->get_mass(); // wtf?
    }
    else if (one_sided)
      kill_me = true;
    
//    assert(norm(posn-oldposn)<2.0f);

    if(a->has_parent())
    {
      po the_po = a->get_abs_po();
      the_po.set_position(posn);

      fast_po_mul(the_po, the_po, a->link_ifc()->get_parent()->get_abs_po().inverse());
      a->set_rel_po(the_po);
    }
    else
      a->set_rel_position(posn);

    a->update_colgeom();

    if(a->has_physical_ifc())
    {
      a->physical_ifc()->set_velocity_with_impact(vel);
    }
  }

  joint::frame_advance(time_inc);
}

