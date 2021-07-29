#ifndef HULL_H
#define HULL_H
////////////////////////////////////////////////////////////////////////////////
/*
  a convex hull defined by at least four outward-facing planes

  it is up to the user to correctly specify the planes.  
  points inside all of the planes are considered inside the hull.
  this can be used to represent a box or a view frustum.
  the view frustum can be cheaply represented by only 5 planes (exclude the near plane)
*/
////////////////////////////////////////////////////////////////////////////////

#include "plane.h"
#include "algebra.h"
class sphere;

class hull // represents a convex hull formed by a set of planes
{
public:
  hull() { faces.reserve(6); }
  void resize(int size) { faces.resize(size); }
  void add_face(const plane& p);
  int get_num_faces() const { return faces.size(); }
  const plane& get_face(int i) const { return faces[i]; }
  // This tests against those faces added earlier first.  
  // Add the most important faces first.
  bool contains(const vector3d& p) const;
  // checks if sphere is totally within the hull.
  bool contains(const sphere& s) const;
  // checks if sphere is at least partially within the hull.  Not very accurate near corners (but that's ok)
  bool includes(const sphere& s) const;
  // Clip the given polygon (expressed as an ordered convex list of points)
  // and return the result as a list of points (convex but not necessarily
  // ordered) filled into the given result parameter.
  typedef vector<vector3d> poly_t;
  void clip(const poly_t& poly, poly_t& result) const;
private:
  typedef vector<plane> plane_list_t;
  plane_list_t faces;
  // bounding sphere that encompasses the entire hull 
  //sphere bound;
};

#endif // HULL_H