#ifndef SPHERE_H
#define SPHERE_H
////////////////////////////////////////////////////////////////////////////////

// sphere.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// It's a sphere.  Arguably the most simple and useful of the basic bounding volumes.

////////////////////////////////////////////////////////////////////////////////

#include "algebra.h"
#include "chunkfile.h"

class sphere
{
public:
  sphere() : center(0,0,0), radius(0) {}
  sphere(const vector3d& _origin,rational_t _r) : center(_origin), radius(_r) {}

  // these names might help people more.
  const vector3d& get_center() const { return center; }
  rational_t get_radius() const    { return radius; }
  void set_center(const vector3d& o) { center=o; }
  void set_radius(rational_t r) { radius=r; }
  // for compatibility
  const vector3d& get_origin() const { return center; }
  rational_t get_r() const    { return radius; }
  void set_origin(const vector3d& o) { center=o; }
  void set_r(rational_t r) { radius=r; }

  inline bool contains(const vector3d& p) const
  {
    return (center-p).length2()<=sqr(radius);
  }

  inline bool intersects(const sphere& v) const
  {
    return (center-v.center).length2()<=sqr(radius+v.radius);
  }

  inline sphere& merge(const sphere& s) // combine two spheres together into a NEW bigger sphere
  {
    if (!radius)
    {
      *this = s;
    }
    else
    {
      vector3d v=(s.center-center).normalize();
      center-=v*radius;
      vector3d n=(center+s.center+v*s.radius)*0.5f;
      radius=(n-center).length();
      center=n;
    }
    return *this;
  }
  inline sphere& merge(const vector3d& p)  // make this sphere encompass the original and this NEW point
  {
    if (!radius)
    {
      center = p;
      radius = 1e-7f;
    }
    else
    {
      vector3d v=p-center;
      rational_t nr=v.length2();
      if (nr>radius*radius)
      {
        nr=(rational_t)__fsqrt(nr);
        rational_t dr=(nr+radius)*0.5f;
        center=p-v*(dr/nr);
        radius=dr;
      }
    }
    return *this;
  }
private:
  vector3d center;
  rational_t radius;

  friend inline void serial_out( chunk_file& io, const sphere& v );
  friend inline void serial_in( chunk_file& io, sphere* v );
};

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out( chunk_file& io, const sphere& v )
{
  serial_out(io,v.center);
  serial_out(io,v.radius);
}
#endif

#if !defined(NO_SERIAL_IN)
inline void serial_in( chunk_file& io, sphere* v )
{
  serial_in( io, &v->center );
  serial_in( io, &v->radius );
}
#endif

#endif // SPHERE_H
