#ifndef XB_GRAPHICS_H
#define XB_GRAPHICS_H
/*-------------------------------------------------------------------------------------------------------

  XB implementation of the of the new style hwos graphics stuff (replacing pmesh, rasterize & more)

-------------------------------------------------------------------------------------------------------*/
#ifndef PRE_WADE_GFX

#include "global.h"
#include "algebra.h"
#ifdef PROJECT_KELLYSLATER
//#undef free
//#undef malloc
#include "ngl.h"
#else
//#include "krender.h"
#endif

inline void set_cursor( int val ) 
{ 
  // null on xb
}

#endif

#endif
