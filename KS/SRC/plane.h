#ifndef PLANE_H
#define PLANE_H
////////////////////////////////////////////////////////////////////////////////

// plane.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

// a plane in 3d with sidedness information,
// defined as a normal vector and the distance from 
// the origin to the point on the plane nearest the origin

// this is abrash's definition and is nice computationally compared
// to the point/normal definition because instead of subtracting from
// point and then dot producting with normal you dot product from normal
// and only have one subtract.

////////////////////////////////////////////////////////////////////////////////

#include "algebra.h"

class plane
{
public:
  typedef rational_t fp;
  plane() {}
  plane( const vector3d& _normal, fp d );
  plane( fp a, fp b, fp c, fp d );
  plane( const vector3d& _point_on_surface, const vector3d& _normal );
  plane( const vector3d& _point_on_surface1,
         const vector3d& _point_on_surface2,
         const vector3d& _point_on_surface3 );
  
  const vector3d& get_unit_normal() const { return unit_normal; }
  fp get_distance_from_origin() const { return odistance; }
  
  // returns signed distance from point to plane
  // note:  this returns negative number if behind plane
  fp distance_above(const vector3d& pt) const
  {
    return dot(unit_normal, pt) - odistance;
  }

  // returns point on plane closest to given point
  vector3d closest_point(const vector3d& pt) const
  {
    fp dd = distance_above(pt);
    return pt - unit_normal*dd;
  }

public:
  fp odistance;  // distance from origin
  vector3d unit_normal;  // perpendicular to plane
};


#endif