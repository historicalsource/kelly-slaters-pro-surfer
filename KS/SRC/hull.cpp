#include "global.h"

#include "hull.h"
#include "sphere.h"


void hull::add_face(const plane& p)
{
  faces.push_back(p);
}

bool hull::contains(const vector3d& p) const
{
  for (plane_list_t::const_iterator fi = faces.begin(); fi != faces.end(); ++fi)
  {
    if( (*fi).distance_above(p) < 0 )
      return false;
  }
  return true;
}

bool hull::contains(const sphere& s) const
{
  for (plane_list_t::const_iterator fi = faces.begin(); fi != faces.end(); ++fi)
  {
    if ((*fi).distance_above(s.get_origin()) < s.get_r())
      return false;
  }
  return true;
}

bool hull::includes(const sphere& s) const
{
  for (plane_list_t::const_iterator fi = faces.begin(); fi != faces.end(); ++fi)
  {
    if ((*fi).distance_above(s.get_origin()) < -s.get_r())
      return false;
  }
  return true;
}

// Clip the given polygon (expressed as an ordered convex list of points)
// and return the result (also an ordered convex list of points).
void hull::clip(const poly_t& poly, poly_t& result) const
{
  const poly_t* src=&poly; // copy from poly first time, then from result thereafter
  poly_t tpoly;
  for (plane_list_t::const_iterator fi = faces.begin(); fi != faces.end(); )
  {
    // prepare for clip to next plane
    tpoly = *src;
    src=&result;
    result.resize(0);
    // build portion of polygon on back side of partition
    rational_t org_dist = (*fi).distance_above(tpoly[0]);
    if (org_dist <= 0)
    {
      // if first vertex is on or behind plane, add to list
      result.push_back(tpoly[0]);
    }
    rational_t prev_dist = org_dist;
    int i,tpz=tpoly.size();
    for (i=1; i<tpz; ++i)
    {
      rational_t dist = (*fi).distance_above(tpoly[i]);
      if ((dist<0 && prev_dist>0) || (dist>0 && prev_dist<0))
      {
        // if previous vertex was on opposite side of partition,
        // add point of intersection as NEW vertex
        vector3d dvec = tpoly[i] - tpoly[i-1];
        rational_t dratio = prev_dist / (prev_dist - dist);
        result.push_back((dvec*dratio)+tpoly[i-1]);
      }
      if (dist <= 0)
      {
        // if current vertex is on or behind partition, add to list
        result.push_back(tpoly[i]);
      }
      // remember distance value for next iteration
      prev_dist = dist;
    }
    // check segment between last vertex and first
    if ((org_dist<0 && prev_dist>0) || (org_dist>0 && prev_dist<0))
    {
      // if previous vertex was on opposite side of partition,
      // add point of intersection as NEW vertex
      vector3d dvec = tpoly[0] - tpoly[i-1];
      rational_t dratio = prev_dist / (prev_dist - org_dist);
      result.push_back((dvec*dratio)+tpoly[i-1]);
    }
    if (result.empty())  // totally clipped already
      return;
    // check result
    assert(result.size()>2);
    ++fi;
  }
}
