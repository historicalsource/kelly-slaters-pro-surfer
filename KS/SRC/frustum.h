#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "algebra.h"
#include "plane.h"

#if 0
enum brd_clip_t
{
  BRD_CLIP_TRIANGLE,
  BRD_CLIP_RECTANGLE,
  BRD_CLIP_PENTAGON
};
#endif

// yes, the spelling is correct.  There is only 1 'r' in frustum.  The plural of frustum is "frusta".

class frustum  // entirely in world coordinates
{
public:
  #if 0
  // The following is all legacy crap that Jamie or Chunk understands, and I don't.  
  // I think it's way too complex.  --Sean
  brd_clip_t clip_type;
  vector3d p0, p1, p2, p3;
  vector3d left_n, right_n;
  vector3d xbasis, zbasis;
  plane top, right, bottom, left, back;
  #else
  // this is all you should need to define a frustum:
  // these planes are in world space:
  plane left, right, back, top, bottom, front; 
  // the camera matrix (camera to world) is useful for making adjustments to the frustum,
  // because it gives the orientation of the planes in world space.  
  
  // bounding sphere that encompasses the entire frustum in world space
  //vector3d bound_center;
  //rational_t bound_radius;

  frustum() {}
  //frustum(const matrix4x4& projection_matrix); // this kicks ass!
  //frustum(const po& camera_to_world, float fov_x, float fov_y); 

  bool is_sphere_visible(const vector3d& center, rational_t radius) const; // sphere in world space
  #endif

  // Computes the 6 planes of the view frustum given a
  // projection matrix of any kind (orthographic or perspective)
  // and also can incorporate a modelview type transformation
  // to compute planes in object space.  
  void extract_planes_from_matrix(const matrix4x4& m);

};

#endif // FRUSTUM_H