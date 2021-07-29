#ifndef CAPSULE_H
#define CAPSULE_H

#include "algebra.h"
#include "colgeom.h"
class entity;

class capsule
{
public:
	capsule() {}
	capsule( const vector3d& b, const vector3d& e, rational_t r) { base = b; end = e; radius = r; }

  vector3d base;
  vector3d end;
  rational_t radius;
};


class collision_capsule : public collision_geometry
{
public:
  collision_capsule() 
    //: rel_cap(vector3d(0.0F),vector3d(0.0F),0.0F)
    //, abs_cap(vector3d(0.0F),vector3d(0.0F),0.0F)
    //, lag_point(0.0F)
  {}
  collision_capsule(entity * _owner);
  virtual ~collision_capsule();
  virtual collision_geometry* make_instance( entity* _owner ) const;
  void compute_dimensions();

  inline const capsule & get_rel_capsule() const { return rel_cap; }
  inline const capsule & get_abs_capsule() const { return abs_cap; }
  inline void set_capsule(const capsule & c)
  {
    rel_cap = c;
    assert(c.base.is_valid() && c.end.is_valid());
  }
  virtual unsigned int get_type() const { return collision_geometry::CAPSULE; }

  virtual void xform(po const & the_po);
  virtual void split_xform(po const & po_1, po const & po2, int second_po_start_idx);

  // This is a hack to retain the "last frame's basepoint" for collision algorithms.
  const vector3d& get_split_xform_lag_point();

  // this function is for a cheat to use different radii for inter-character collisions
  virtual void apply_radius_scale(rational_t rad_scale);
  virtual void estimate_physical_properties(entity * rb, rational_t material_density=DENSITY_OF_WATER);
  virtual void get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const;
  virtual void get_min_extent( vector3d* v ) const;
  virtual void get_max_extent( vector3d* v ) const;
  virtual void render() const;

  inline const vector3d& get_base() const { return abs_cap.base; }
  inline const vector3d& get_end() const { return abs_cap.end; }
  virtual rational_t get_radius() const { return abs_cap.radius+(abs_cap.base-abs_cap.end).length(); }
  virtual rational_t get_core_radius() const { return abs_cap.radius; }

private:
  capsule rel_cap;
  capsule abs_cap;
  vector3d lag_point;
};

#endif