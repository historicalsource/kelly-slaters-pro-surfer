// portal.h
// Copyright (C) 2000 Treyarch LLC    ALL RIGHTS RESERVED

#ifndef PORTAL_H
#define PORTAL_H

#include "algebra.h"
#include "sphere.h"
#include "stringx.h"
#include "graph.h"
#include "pmesh.h"
#include "region_graph.h"

//class portal;
//class region;
//typedef graph<stringx,region*,portal*> region_graph;
//typedef region_graph::node region_node;

class portal : public vr_pmesh
{
// Constructors
public:
  portal();
  portal( region_node* _front, region_node* _back );
  virtual ~portal() {}

  region_node* get_front() const         { return front; }
  region_node* get_back() const          { return back; }
  const vector3d& get_effective_center() const { return bound.get_center(); }
  rational_t get_effective_radius() const      { return bound.get_radius(); }
  const sphere& get_bound_sphere() const       { return bound; }
  const vector3d& get_cylinder_normal() const  { return normal; }
  // note that this is half the effective thickness of the cylinder
  rational_t get_cylinder_depth() const        { return cylinder_depth; }
  // list of normals parallels pmesh faces
  vector3d get_normal( bool front ) const      { return front ? normal : -normal; }
  float get_non_planar_fudge_factor() const    { return nonplanarfudgefactor; }
  bool touches_sphere( const sphere& s ) const;
  bool touches_segment( const vector3d& p1, const vector3d& p2 ) const;
  bool is_active() const                       { return !inactive; }
  void set_active(bool a)                      { inactive = !a; }
protected:
  virtual void compute_info();
  sphere bound;
  rational_t cylinder_depth;
  vector3d normal;    // overall normal of portal (and cylinder)
  rational_t nonplanarfudgefactor; // kludge to dot product with portal normal
  region_node* front;
  region_node* back;
  bool inactive;
};
typedef list<portal*> portal_list;

class plane;

// not a good place for this, but it'll work for now. --Sean
void ClipTriToPlane(const plane& clipto,       // the plane to clip to, in same coordsys as verts
                    vector3d const* verts[4],  // pointers to the verts, in order (3 in / 3 or 4 out)
                    vector3d buf[2]);          // extra space that may be needed for the NEW vertices
void ClipQuadToPlane(const plane& clipto,      // the plane to clip to, in same coordsys as verts
                    vector3d const* verts[5],  // pointers to the verts, in order (4 in / 3, 4, or 5 out)
                    vector3d buf[2]);          // extra space that may be needed for the NEW vertices


#endif // PORTAL_H
