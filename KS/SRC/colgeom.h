#ifndef COLGEOM_H
#define COLGEOM_H


#include "algebra.h"
#include "physics.h"
//#include "physent.h"
#include "entity.h"
#include "cface.h"
//#include "collide.h"

class po;
class capsule;
class entity;
class rigid_body;
class region;

//typedef vector<vector3d,malloc_alloc> vectorvector;
typedef vector<vector3d> vectorvector;


// The following is workspace for the collision routines.
// They record all the collision points between a pair of physical_entities
// p1 and p2 for the current frame.
// The vectors start out empty, and are cleared after each collision
// so they are always empty when collision_computations begin.
extern vectorvector hit_list;
extern vectorvector normal_list1;
extern vectorvector normal_list2;

void COLGEOM_stl_prealloc( void );

class collision_geometry
{
public:
  collision_geometry();
  collision_geometry(const collision_geometry &b);
  virtual ~collision_geometry();

  virtual collision_geometry* make_instance( entity* _owner ) const = 0;

  virtual void xform(po const & the_po);

  virtual void apply_radius_scale(rational_t rad_scale) {}

  // Split xform is for xforming a mesh with two different po's.  The first xforms the verts of index
  // 0 thru second_po_start_idx-1, the second xforms the verts second_po_start_idx thru the end of the list.
  virtual void split_xform(po const & po_1, po const & po2, int second_po_start_idx)
  { assert(0); }
  virtual void split_xform(po const & po_1, po const & po2, po const & po3, int second_po_start_idx, int third_po_start_idx)
  { assert(0); }
  virtual rational_t get_radius() const { assert(owner); return owner->get_radius(); }
  // Whereas radius is the distance of the furthest point from the center, core_radius is distance of the
  // furthest point of the core of the colgeom.  This currently has meaning only for capsules, whose core is
  // defined to be the line segment from cap.base to cap.end.
  virtual rational_t get_core_radius() const { return get_radius(); }
  virtual const vector3d& get_abs_position() const { assert(owner); return owner->get_abs_position(); }

  static bool collides(collision_geometry* g1, collision_geometry* g2,
                       vectorvector* hit_list, vectorvector* normal_list1, vectorvector * normal_list2,
                       unsigned int ct, const vector3d & rel_vel, cface * hitFace = NULL);
  static bool collides_with_region(collision_geometry* g1, region* t,
                                vectorvector* hit_list, vector <vector3d>* normal_list1, vector <vector3d>* normal_list2,
                                unsigned int ct,const vector3d& rel_vel);

  // The core of a colgeom is defined above (for get_core radius).  Currently only differs from "center" for capsules.
  static rational_t distance_between_cores(const collision_geometry *g1, const collision_geometry* g2, vector3d * diff);

  virtual void estimate_physical_properties(entity * rb, rational_t material_density=DENSITY_OF_WATER)=0;

  virtual void get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const=0;

  virtual void get_min_extent( vector3d* v ) const { *v = vector3d(0,0,0); };
  virtual void get_max_extent( vector3d* v ) const { *v = vector3d(0,0,0); };
  enum
  {
    NONE,
    CAPSULE,
    MESH
  };
  virtual const vector3d& get_pivot() const { return ZEROVEC; }

  inline bool is_valid() const { return valid; }
  inline void validate() { valid = true; }
  inline void invalidate() { valid = false; }
  virtual unsigned int get_type() const=0;

  virtual bool is_pivot_valid() const { return false; }
  virtual bool is_entity_collision() const { return true; }
  virtual bool is_camera_collision() const { return false; }

  inline void set_owner(entity * e) { owner = e; }
protected:
  entity * owner;
  bool valid;
};

class cg_none : public collision_geometry
{
public:
  cg_none();
  virtual ~cg_none();
  virtual void xform(po const & the_po);
  virtual void get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const
  {
    *target=ZEROVEC;
  }
  virtual unsigned int get_type() const;
};

#endif
