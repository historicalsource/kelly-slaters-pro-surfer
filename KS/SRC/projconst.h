#ifndef PROJCONST_H
#define PROJCONST_H
////////////////////////////////////////////////////////////////////////////////
/*
  projconst.h

  projection transformation constants
*/
////////////////////////////////////////////////////////////////////////////////
#include "hwmath.h"

const rational_t PROJ_FIELD_OF_VIEW = 1.570795f;
const rational_t PROJ_ASPECT        = 3.0f/4.0f;
const rational_t PROJ_NEAR_PLANE_D  = 0.2f;
//const rational_t PROJ_W_OVER_Z      = 0.707106312093557605359224433054721f;   // sin(FOV/2) from projction xformation
//const rational_t PROJ_Z_OVER_W      = 1.41421450055969727768933161636527f;  // reciprocal of above
const rational_t PROJ_NEAR_PLANE_W  = 0.1414212624187115210718448866108f;    // she loves you ja ja ja
const rational_t PROJ_NEAR_PLANE_RHW = 7.07107250279848638844665808183355f;
extern rational_t PROJ_FAR_PLANE_D;
extern rational_t PROJ_FAR_PLANE_RHW;
extern rational_t PROJ_ZOOM;
extern rational_t PROJ_RECIP_ZOOM;   // 1.0f / PROJ_ZOOM, used for particles and other things

// This stuff was found all over the place so I refactored it here.
// We can precompute these per frame later and save some work!
#define PROJ_FOV_ZOOM_HALF  (PROJ_FIELD_OF_VIEW*PROJ_ZOOM*0.5F)
#define PROJ_COS_FOV  ((float)fast_cos(PROJ_FOV_ZOOM_HALF))
#define PROJ_SIN_FOV  ((float)fast_sin(PROJ_FOV_ZOOM_HALF))

extern float field_of_view_fudge;

inline void proj_update_recip_zoom( void ) 
{
  PROJ_RECIP_ZOOM = 1.0f / PROJ_ZOOM; 
}

inline rational_t proj_field_of_view_in_degrees( void )
{
  return (PROJ_FIELD_OF_VIEW * field_of_view_fudge * PROJ_ZOOM * 180.0f / PI);
}

#endif