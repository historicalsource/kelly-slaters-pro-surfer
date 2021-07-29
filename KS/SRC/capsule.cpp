// capsule.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// The capsule and collision_capsule primitives


#include "global.h"

//!#include "character.h"
#include "capsule.h"
#include "debug_render.h"
#include "iri.h"
#include "renderflav.h"
#include "pmesh.h"



collision_capsule::collision_capsule(entity * _owner)
//  : collision_geometry()
{
  owner = _owner;
  compute_dimensions();
}


collision_capsule::~collision_capsule()
{
}


collision_geometry* collision_capsule::make_instance( entity* ent ) const
{
  return NEW collision_capsule( ent );
}


void collision_capsule::compute_dimensions()
{
  rel_cap.base = owner->get_abs_position()+(owner->get_radius()*0.125F)*YVEC;
  rel_cap.end = (owner->get_radius()*0.5F)*YVEC+rel_cap.base;
  rel_cap.radius = owner->get_radius()*0.25F;
  abs_cap = rel_cap;
}


void collision_capsule::xform(po const & the_po)
{
  collision_geometry::xform(the_po);
  assert(rel_cap.base.is_valid() && rel_cap.end.is_valid());
  abs_cap.base = the_po.slow_xform(rel_cap.base);
  abs_cap.end = the_po.slow_xform(rel_cap.end);
  abs_cap.radius = rel_cap.radius;
  lag_point = ZEROVEC;
}

void collision_capsule::split_xform(po const & po_1, po const & po_2, int second_po_start_idx)
{
  // Note:  po_2 is last frame's po.
  collision_geometry::xform(po_2);
  vector3d vel;
  owner->get_velocity(&vel);
  abs_cap.base = po_2.slow_xform(rel_cap.base);
  abs_cap.end = po_2.slow_xform(rel_cap.end);
  abs_cap.radius = rel_cap.radius;
  rational_t a = dot(abs_cap.end-abs_cap.base,vel);
  if (a>0)
  {
    abs_cap.end = po_1.slow_xform(rel_cap.end);
    lag_point = abs_cap.base;
  }
  else
  {
    abs_cap.base = po_1.slow_xform(rel_cap.base);
    lag_point = abs_cap.end;
  }
}

const vector3d& collision_capsule::get_split_xform_lag_point()
{
  return lag_point;
}


void collision_capsule::estimate_physical_properties(entity * rb, rational_t material_density)
{

  return;
  /*
  rational_t length = norm(rel_cap.base-rel_cap.end)*2*rel_cap.radius;

  // colume for a cylinder
  rational_t volume = PI*rel_cap.radius*rel_cap.radius*length;
  rational_t mass = material_density*volume
  rb->set_mass(mass);
  rb->set_I_vector(vector3d(I_x,I_y,I_z));
  rb->set_volume(volume);
  rb->set_radius(radius);
  rb->set_c_o_m(vector3d(0,0,0));
  */
}


void collision_capsule::get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const
{
  vector3d outval;
  rational_t a = dot(abs_cap.base,axis);
  rational_t b = dot(abs_cap.end,axis);
  vector3d normal_axis = axis;
  normal_axis.normalize();
  if (a<b)
  {
    outval = abs_cap.base-normal_axis*rel_cap.radius;
  }
  else if (b<a)
  {
    outval = abs_cap.end-normal_axis*rel_cap.radius;
  }
  else
  {
    outval = (abs_cap.end+abs_cap.base)/2 - normal_axis*rel_cap.radius;
  }
  *target=outval;
}


void collision_capsule::get_min_extent( vector3d* v ) const
{
  if ( abs_cap.base.x < abs_cap.end.x )
    v->x = abs_cap.base.x - abs_cap.radius;
  else
    v->x = abs_cap.end.x - abs_cap.radius;
  if ( abs_cap.base.y < abs_cap.end.y )
    v->y = abs_cap.base.y - abs_cap.radius;
  else
    v->y = abs_cap.end.y - abs_cap.radius;
  if ( abs_cap.base.z < abs_cap.end.z )
    v->z = abs_cap.base.z - abs_cap.radius;
  else
    v->z = abs_cap.end.z - abs_cap.radius;
}

void collision_capsule::get_max_extent( vector3d* v ) const
{
  if ( abs_cap.base.x > abs_cap.end.x )
    v->x = abs_cap.base.x + abs_cap.radius;
  else
    v->x = abs_cap.end.x + abs_cap.radius;
  if ( abs_cap.base.y > abs_cap.end.y )
    v->y = abs_cap.base.y + abs_cap.radius;
  else
    v->y = abs_cap.end.y + abs_cap.radius;
  if ( abs_cap.base.z > abs_cap.end.z )
    v->z = abs_cap.base.z + abs_cap.radius;
  else
    v->z = abs_cap.end.z + abs_cap.radius;
}


void collision_capsule::apply_radius_scale(rational_t rad_scale)
{
  abs_cap.radius = rel_cap.radius*rad_scale;
}


rational_t g_capsule_translucency = 0.5f;

void collision_capsule::render() const
{
#if defined (DEBUG)
  render_capsule(abs_cap.base,abs_cap.end,abs_cap.radius,color32(255, 255, 255, 255*g_capsule_translucency ));
#endif
}
