#ifndef COLCAP_H
#define COLCAP_H

#include "colgeom.h"

// use collision_capsule from capsule.h instead!

/*
class cg_capsule : public collision_geometry
{
public:
  cg_capsule( entity* _owner );
  virtual collision_geometry* make_instance( entity* owner ) const;
  virtual ~cg_capsule();
  virtual void xform(po & the_po);

  capsule * get_capsule() const { return cap; }

  virtual void get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const;

  virtual void get_min_extent( vector3d* v ) const;
  virtual void get_max_extent( vector3d* v ) const;

protected:
  virtual unsigned int get_type() const;
private:
  capsule * cap;
  capsule * raw_cap;
};
*/

#endif