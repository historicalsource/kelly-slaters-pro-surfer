#ifndef BOUNDBOX_H
#define BOUNDBOX_H

#include "algebra.h"

class bounding_box
{
public:
  vector3d vmin;
  vector3d vmax;

  bounding_box() 
    : vmin(FLT_MAX,FLT_MAX,FLT_MAX), vmax(-FLT_MAX,-FLT_MAX,-FLT_MAX) 
  {}

  bounding_box(const vector3d& _min,const vector3d& _max) 
    : vmin(_min), vmax(_max)
  {}

  bounding_box(const vector3d& _ctr,rational_t radius)  // from sphere
    : vmin(_ctr), vmax(_ctr)
  {
    vector3d sz(radius,radius,radius);
    vmin -= sz;
    vmax += sz;
  }

  vector3d size() const
  {
    return vmax-vmin;
  }

  vector3d center() const
  {
    return (vmax+vmin)*0.5F;
  }

  float radius() const // radius needed to include box corners (measured from center)
  {
    return (vmax-vmin).length()*0.5F;
  }

  float xz_radius() const
  {
    vector3d vmax_tmp = vmax;
    vmax_tmp.y = 0;
    vector3d vmin_tmp = vmin;
    vmin_tmp.y = 0;
    return (vmax_tmp-vmin_tmp).length()*0.5F;
  }

  // expand box so that it includes the point
  void accumulate(const vector3d& p) 
  {
    if (vmin.x>p.x) vmin.x=p.x;
    if (vmin.y>p.y) vmin.y=p.y;
    if (vmin.z>p.z) vmin.z=p.z;
    if (vmax.x<p.x) vmax.x=p.x;
    if (vmax.y<p.y) vmax.y=p.y;
    if (vmax.z<p.z) vmax.z=p.z;
  }
  
  // box touches other box?
  bool intersect(const bounding_box& b) const
  {
    return ( b.vmax.x > vmin.x &&
             b.vmax.y > vmin.y &&
             b.vmax.z > vmin.z &&
             b.vmin.x < vmax.x &&
             b.vmin.y < vmax.y &&
             b.vmin.z < vmax.z );
  }

  // point within box?
  bool intersect(const vector3d& p) const
  {
    return ( p.x >= vmin.x &&
             p.y >= vmin.y &&
             p.z >= vmin.z &&
             p.x <= vmax.x &&
             p.y <= vmax.y &&
             p.z <= vmax.z );
  }

  // point within XZ bounds?
  bool xz_intersect( const vector3d& p ) const
  {
    return ( p.x>vmin.x && p.x<vmax.x && p.z>vmin.z && p.z<vmax.z );
  }

  // sphere within box (only approximate near corners)
  bool intersect( const vector3d& p, rational_t r ) const
  {
    return ( p.x+r > vmin.x &&
             p.y+r > vmin.y &&
             p.z+r > vmin.z &&
             p.x-r < vmax.x &&
             p.y-r < vmax.y &&
             p.z-r < vmax.z );
  }
};


#endif  // BOUNDBOX_H
