#ifndef COLLIDE_H
#define COLLIDE_H

#include "algebra.h"
#include "capsule.h"
#include "colgeom.h"
#include "algebra.h"
#include "meshrefs.h"
//!class collision_capsule;
class cface;
class cg_mesh;
class face;
class vr_pmesh;
class region;
class partition3;
class entity;

enum col_type
{
  ONE_HIT_PER_M2_POLY = 1,
  PP_REAR_CULL        = 2,
  PP_FULL_MESH        = 4,
  ONE_HIT_PER_MESH    = 8,
};

// List of external prototypes for collision functions

// taken from beam.cpp 4/10/00 (JDB)
bool collide_segment_geometry(const vector3d &p0, const vector3d &p1, const collision_geometry *cg, vector3d &impact_pos, vector3d &impact_normal, const po &my_po, bool rear_cull = true);

// added 4/10/00 to help with combat targeting (JDB)
bool collide_segment_entity(const vector3d &p0, const vector3d &p1, const entity *ent, vector3d* impact_pos, vector3d* impact_normal, rational_t default_radius = 1.0f, bool rear_cull = true);

bool collide_polygon_segment( const cface* f1, const cg_mesh* m1,
                              const vector3d& p1, const vector3d& p2,
                              vector3d& hit_loc );

bool collide_polygon_segment( face_ref f1, const vr_pmesh* m1,
                              const vector3d& p1, const vector3d& p2,
                              vector3d& hit_loc );

bool collide_polygon_polygon( const cface* f1, const cg_mesh* m1,
                              const cface* f2, const cg_mesh* m2,
                              vector3d& hit_loc );

// collide_segment_capsule and collide_segment_capsule_accurate_result are NOT the same thing!
// collide_segment_capsule is used for colliding guys with the world and colmeshes and such
// the hit_point is not necessarily the point where the segment hits the capsule.. it seems
// to be locked to the axis of the capsule instead.
// I should probably rename this to "collide_capsule_segment"  ;)
bool collide_segment_capsule( const vector3d& n1, const vector3d& n2,
                              const vector3d& base, const vector3d& end, const rational_t radius,
                              vector3d& hit_point );

inline bool collide_segment_capsule( const vector3d& n1, const vector3d& n2,
                              capsule const& cap,
                              vector3d& hit_point )
{
  return collide_segment_capsule(n1, n2, cap.base, cap.end, cap.radius, hit_point);
}

inline bool collide_segment_capsule( const vector3d& n1, const vector3d& n2,
                              collision_capsule const& cap,
                              vector3d& hit_point )
{
  return collide_segment_capsule(n1, n2, cap.get_base(), cap.get_end(), cap.get_core_radius(), hit_point);
}

// collide_segment_capsule and collide_segment_capsule_accurate_result are NOT the same thing!
// collide_segment_capsule_accurate_result is used for targeting and such where the
// accurate determination of hit_point is desired.  It does return the nearest point where
// the segment hits the capsule.
// I should probably rename this to "collide_segment_capsule"
bool collide_segment_capsule_accurate_result( const vector3d& n1, const vector3d& n2,
                              const vector3d& base, const vector3d& end, const rational_t radius,
                              vector3d& hit_point );

inline bool collide_segment_capsule_accurate_result( const vector3d& n1, const vector3d& n2,
                              capsule const& cap,
                              vector3d& hit_point )
{
  return collide_segment_capsule_accurate_result(n1, n2, cap.base, cap.end, cap.radius, hit_point);
}

inline bool collide_segment_capsule_accurate_result( const vector3d& n1, const vector3d& n2,
                              collision_capsule const& cap,
                              vector3d& hit_point )
{
  return collide_segment_capsule_accurate_result(n1, n2, cap.get_base(), cap.get_end(), cap.get_core_radius(), hit_point);
}

bool collide_segment_cylinder( const vector3d& o,
                               const vector3d& d,
                               const vector3d& center,
                               const vector3d& normal,
                               const rational_t radius,
                               const rational_t depth,
                               vector3d& hit_point );

inline bool collide_segment_cylinder( const vector3d& p1, const vector3d& p2,
                               const vector3d& c1, const vector3d& c2, const rational_t radius,
                               vector3d& hit_point )
{
  return collide_segment_cylinder( p1, p2-p1,
                              (c1+c2)*0.5f, (c2-c1).normalize(), radius, (c2-c1).length()*0.5f,
                              hit_point );
}



bool collide_segment_sphere( const vector3d& p0, const vector3d& p1,
                             const vector3d& s, rational_t r,
                             vector3d* hit_loc );

bool collide_sphere_polygon( const vector3d& p, rational_t r,
                             const cface* f, const cg_mesh* m,
                             vector3d& hit_loc );

bool collide_sphere_mesh( const vector3d& p, rational_t r, const vector3d& v,
                          const cg_mesh* m,
                          vector3d& hit_loc );

// ct is an amalgam of col_type's
bool collide_capsule_polygon( collision_capsule const& c,
                              cface const* f2, const cg_mesh* m2,
                              vectorvector* hit_list,
                              vectorvector* normal_list1,
                              vectorvector* normal_list2,
                              const vector3d& rel_vel );

bool collide_mesh_polygon( const cg_mesh* m1,
                           const cface* f2, const cg_mesh* m2,
                           vectorvector* hit_list,
                           vectorvector* normal_list1,
                           vectorvector* normal_list2,
                           unsigned int ct,
                           const vector3d& rel_vel );

bool collide_mesh_mesh( const cg_mesh* m1, const cg_mesh* m2,
                        vectorvector* hit_list1,
                        vectorvector* normal_list1,
                        vectorvector* normal_list2,
                        unsigned int ct,
                        const vector3d& rel_vel,
						cface * hitFace = NULL);

bool collide_mesh_region( const cg_mesh* m1, const region* t,
                         vectorvector* hit_list1,
                         vectorvector* normal_list1,
                         vectorvector* normal_list2,
                         unsigned int ct,
                         const vector3d& rel_vel );

bool collide_segment_region( const vector3d& p1, const vector3d& p2,
                             const region* t,
                             vector3d& hit_loc, vector3d& hit_normal,
                             unsigned int ct,
                             const vector3d& rel_vel );

bool collide_segment_region_with_poly_data( const vector3d& p1, const vector3d& p2,
                                            const region* t,
                                            vector3d& hit_loc, vector3d& hit_normal,
                                            unsigned int ct,
                                            const vector3d& rel_vel,
                                            const vr_pmesh** pM = 0,
                                            face_ref* pF = 0 );

bool collide_segment_mesh( const vector3d& p1, const vector3d& p2,
                           const cg_mesh* m2,
                           vector3d& hit_loc, vector3d& hit_normal,
                           unsigned int ct,
                           const vector3d& rel_vel );

bool collide_capsule_region( const collision_capsule* c1, const region* t,
                             vectorvector* hit_list,
                             vectorvector* normal_list1,
                             vectorvector* normal_list2,
                             unsigned int ct,
                             const vector3d& rel_vel );

bool collide_capsule_mesh( const collision_capsule* c1, const cg_mesh* m2,
                           vectorvector* hit_list,
                           vectorvector* normal_list1,
                           vectorvector* normal_list2,
                           unsigned int ct,
                           const vector3d& rel_vel );

bool collide_capsule_full_mesh( const collision_capsule* c1, const cg_mesh* m2,
                           vectorvector* hit_list,
                           vectorvector* normal_list1,
                           vectorvector* normal_list2,
                           unsigned int ct,
                           const vector3d& rel_vel );

bool collide_capsule_capsule( const capsule& cap1, const capsule& cap2,
                              vectorvector* hit_list,
                              vectorvector* normal_list1,
                              vectorvector* normal_list2,
                              rational_t& distance,
                              unsigned int ct,
                              const vector3d& rel_vel,
                              vector3d* core_diff = NULL );


rational_t dist_point_segment( const vector3d& p,
                               const vector3d& n1, const vector3d& n2,
                               vector3d& hit_point );

rational_t dist_point_polygon( const vector3d& p,
                               const vector3d& v0, const vector3d& v1, const vector3d& v2 );


bool collide_sphere_partition3( const vector3d& pos, rational_t r,
                                const partition3& h1,
                                vector3d& hit_point );

bool collide_sphere_two_partition3s( const vector3d& pos, rational_t r,
                                     const partition3& h1,
                                     const partition3& h2,
                                     vector3d& hit_point );

bool collide_sphere_three_partition3s( const vector3d& pos, rational_t r,
                                       const partition3& h1,
                                       const partition3& h2,
                                       const partition3& h3,
                                       vector3d& hit_point );

enum find_int_flags_t
{
  FI_COLLIDE_WORLD                  = 0x0001,
  FI_COLLIDE_ENTITY                 = 0x0002,
  FI_COLLIDE_BEAMABLE               = 0x0004,
  FI_COLLIDE_SCANNABLE              = 0x0008,
  FI_COLLIDE_CAMERA                 = 0x0010,
  FI_COLLIDE_AI_LOS                 = 0x0020,
  FI_COLLIDE_ENTITIES_MASK          = FI_COLLIDE_ENTITY|FI_COLLIDE_BEAMABLE|FI_COLLIDE_SCANNABLE|FI_COLLIDE_CAMERA|FI_COLLIDE_AI_LOS,

  // Hints
  FI_COLLIDE_ENTITY_NO_REAR_CULL    = 0x0040, // not included in FI_COLLIDE_HINTS.
  FI_COLLIDE_ENTITY_NO_CAPSULES     = 0x0080,
  FI_COLLIDE_ENTITY_VISIBLE_ONLY    = 0x0100,
  FI_COLLIDE_HINTS                  = FI_COLLIDE_ENTITY_NO_CAPSULES|FI_COLLIDE_ENTITY_VISIBLE_ONLY
};

bool find_intersection( const vector3d& p0,
                        const vector3d& p1,
                        region_node* start_region,
                        unsigned int flags,
                        vector3d* hit_loc,
                        vector3d* hit_normal,
                        region_node** hit_region = NULL,
                        entity** hit_entity = NULL );

bool in_world( const vector3d& p,
               rational_t r,
               const vector3d& v,
               region_node* start_region,
               vector3d& hit_loc, bool camera = false );


#endif
