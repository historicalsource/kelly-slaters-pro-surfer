#ifndef GEOMGR_H
#define GEOMGR_H
////////////////////////////////////////////////////////////////////////////////

// geomgr.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// Geometry pipe.
// The geometry pipe is implicitly attached to the rasterizer and the vertex
// buffers.  When either the rasterizer or the vertex buffers make transformations,
// they are using the transformation matrices set up in the geometry pipe.

// Note:  when the rasterizer display changes, the current setup of the geometry
// pipe is *lost*.  Currently, the geometry pipe is setup in game::init_geometry_pipe()

////////////////////////////////////////////////////////////////////////////////

#include "singleton.h"
#include "algebra.h"
#include "hull.h"
#include "projconst.h"
#include "hwrasterize.h"

//  forward class declarations
class matrix4x4;
class sphere;

// maybe these should go into projconst.h?
#define INTERFACE_MIN_Z   0.0f
#define INTERFACE_MAX_Z   0.0495f // let's leave a little gap in z buf
#define VREPWIDGET_MIN_Z  0.0f
#define VREPWIDGET_MAX_Z  0.0495f // let's leave a little gap in z buf
#define WORLD_MIN_Z       0.05f
#define WORLD_MAX_Z       1.0f

class geometry_manager : public singleton
{
public:
  DECLARE_SINGLETON(geometry_manager)

  // the geometry manager always uses the current d3d device
  geometry_manager();
  ~geometry_manager();

  void restore();  // restores geometry manager after a mode change

  void rebuild_view_frame();  // recomputes viewing frustum, etc.  used by restore, but available to others.

  // the different types of transformations
  enum xform_t
  {
    XFORM_LOCAL_TO_WORLD,           // local to world (changed every time you render a NEW model)
    XFORM_WORLD_TO_VIEW,            // world to view for the current camera (for visibility/LOD)
    XFORM_VIEW_TO_WORLD,            // inverse of the above (for view space rendering)
    XFORM_VIEW_TO_PROJECTION,       // view to pre-clip projection  (changed for camera zoom?)
    XFORM_PROJECTION_TO_SCREEN,     // converts post-clip space to actual pixels (viewport xform)
    XFORM_EFFECTIVE_WORLD_TO_VIEW,  // world to view for the current camera or the scene analyzer (for drawing)
    XFORM_VIEW_TO_SCREEN,           // combines XFORM_VIEW_TO_PROJECTION with XFORM_PROJECTION_TO_SCREEN
    XFORM_WORLD_TO_PROJECTION,      // combines XFORM_EFFECTIVE_WORLD_TO_VIEW with XFORM_VIEW_TO_PROJECTION
    XFORM_WORLD_TO_SCREEN,          // combines XFORM_EFFECTIVE_WORLD_TO_VIEW with XFORM_VIEW_TO_PROJECTION with XFORM_PROJECTION_TO_SCREEN
    NUM_XFORMS
  };

  // set up a 4x4 look-at matrix
  void set_look_at( matrix4x4* dest,
                    const vector3d& from,
                    const vector3d& look_at,
                    const vector3d& up );

  // set view
  void set_view( const vector3d& from,
                 const vector3d& look_at,
                 const vector3d& up );

  // Scene analyzer interface - this allows you to concatenate an extra world to screen matrix that
  // is hidden from all the visibility determination functions.  It basically allows you tell the
  // renderer that it is drawing from a location, and then look at what it decides to keep or cull
  // from another location.
  void enable_scene_analyzer( bool enable ) { scene_analyzer_enabled = enable; }
  bool is_scene_analyzer_enabled() { return scene_analyzer_enabled; }
  void set_scene_analyzer( const vector3d& from,
                           const vector3d& look_at,
                           const vector3d& up );

  // check if any portion of sphere is within viewing frustum
  // if this function is slow, it could be rewritten to do a batch of spheres
  bool within_view( const sphere& s ) const;

  // transform a sphere radius into a screen extent
  // this returns radius in the projection coordinate system, which is a cuboid
  // with the near plane having z=0, the far plane having z=1, the left plane having
  // x = -4/3, the right plane having x = 4/3, top being y=1, and bottom being y=-1
  rational_t radius_world_to_screen( const sphere& s ) const;

  // crude test for a point being in visual range
  inline bool crude_clipped( const vector3d & v ) const
  {
    vector3d v1;

    matrix4x4 const& m1 = xforms[XFORM_LOCAL_TO_WORLD];
    v1 = v+vector3d(m1[3][0],m1[3][1],m1[3][2])-camera_pos;
    return ( v1.length2() > PROJ_FAR_PLANE_D*PROJ_FAR_PLANE_D || dot(camera_dir,v1)<-50);      // use view space to clip
//      vector3d view_v = xform3d_1(xforms[XFORM_WORLD_TO_VIEW], v); //*xforms[XFORM_LOCAL_TO_WORLD]
//      return (view_v.z>PROJ_FAR_PLANE_D || view_v.length2()>(PROJ_FAR_PLANE_D*PROJ_FAR_PLANE_D));
  }

  void set_clip_distance(rational_t clip_dist);

  const vector3d& get_camera_pos() const { return camera_pos; } // for current view (could be the scene analyzer cam)
  const vector3d& get_camera_dir() const { return camera_dir; }

  // set a single stage of the transforms that all future geometry calculations will use
  void set_xform( xform_t xform, const matrix4x4& matrix );
  const matrix4x4& get_world_to_screen() const { return xforms[XFORM_WORLD_TO_PROJECTION]; }
  const matrix4x4& get_viewport_xform() const { return xforms[XFORM_PROJECTION_TO_SCREEN]; }

  // set local-to-world transform
  void set_local_to_world( const matrix4x4& _x ) { set_xform( XFORM_LOCAL_TO_WORLD, _x ); }

  void set_cop(float x, float y, float min_z=0.0f, float max_z=1.0f);
  void get_cop(float* x, float* y, float *min_z=0, float *max_z=0) const
  {
    if (x) *x = cop_x;
    if (y) *y = cop_y;
    if (min_z) *min_z = cop_min_z;
    if (max_z) *max_z = cop_max_z;
  }

  float get_min_z() const
  {
    return cop_min_z;
  }

  float get_max_z() const
  {
    return cop_min_z;
  }

  float get_scale_z() const
  {
    return cop_scale_z;
  }

  float fix_z_coord(float z) const
  {
    return cop_scale_z * z + cop_min_z;
  }

  // I have no idea what "vp" stands for.  You might think "viewport" but you'd be wrong
  // They should only be equal if you have square pixels.  (640x480, 800x600, etc)
  float get_vp_width() { return vp_width; }
  float get_vp_height() { return vp_height; }

  // these are allocated in order to obtain 8-byte alignment on Dreamcast
  matrix4x4* xforms; // I don't like this... they should be named individually, not an array --Sean

  vector3d camera_pos; // for current view
  vector3d camera_up;
  vector3d camera_dir;

  matrix4x4 scene_analyzer;
  bool scene_analyzer_enabled;

  float cop_x, cop_y, cop_min_z, cop_max_z, cop_scale_z;

  float vp_width, vp_height;

  hull view_frustum;  // in view space
};


#endif // GEOMGR_H
