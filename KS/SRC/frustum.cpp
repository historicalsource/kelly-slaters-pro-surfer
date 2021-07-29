// frustum.cpp

#include "global.h"

#include "frustum.h"

// Actually this may be better as a special case of convex_hull (in hull.h).  
// Or, maybe just use convex_hull instead!

bool frustum::is_sphere_visible(const vector3d& center, rational_t radius) const // sphere in world space
{
  #ifdef DEBUG
  extern bool g_clip_to_view_frustrum;
  if (!g_clip_to_view_frustrum) return true;
  #endif

  // This is a good order to cull by, as it culls the majority of invisible
  // objects in the first couple tries.  Relies on short-circuit evaluation!
  radius = -radius; // don't want to do this 6 times below!
  return left  .distance_above(center) > radius &&
         right .distance_above(center) > radius &&
         back  .distance_above(center) > radius &&
         top   .distance_above(center) > radius &&
         bottom.distance_above(center) > radius;// &&
         //front .distance_above(center) > radius;
  // We can get away with not testing the front clip plane because
  // the left,right,top,bottom planes take care of it pretty good.
  // The worst that can happen is that it might say something is visible
  // when actually it's slightly behind the front clip plane (clipped 
  // by less than a foot or so,) right near the eye pos.
  // If we use parallel projections, we would need to test the front plane.
}



// This routine might seem a little like magic to most people.  It works.
// Should I change this into a ctor?

void frustum::extract_planes_from_matrix(const matrix4x4& m) 
{
  // Credit:  Gil Gribb  (ggribb@mail.ravensoft.com)
  left  =plane(m.x.x - m.x.w, m.y.x - m.y.w, m.z.x - m.z.w, m.w.x - m.w.w);
  right =plane(m.x.x + m.x.w, m.y.x + m.y.w, m.z.x + m.z.w, m.w.x + m.w.w);
  top   =plane(m.x.y - m.x.w, m.y.y - m.y.w, m.z.y - m.z.w, m.w.y - m.w.w);
  bottom=plane(m.x.y + m.x.w, m.y.y + m.y.w, m.z.y + m.z.w, m.w.y + m.w.w);
  front =plane(m.x.z - m.x.w, m.y.z - m.y.w, m.z.z - m.z.w, m.w.z - m.w.w);
  back  =plane(m.x.z + m.x.w, m.y.z + m.y.w, m.z.z + m.z.w, m.w.z + m.w.w);
}
