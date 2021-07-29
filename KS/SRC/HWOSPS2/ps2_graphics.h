#ifndef PS2_GRAPHICS_H
#define PS2_GRAPHICS_H
/*-------------------------------------------------------------------------------------------------------

  PS2 implementation of the of the new style hwos graphics stuff (replacing pmesh, rasterize & more)

-------------------------------------------------------------------------------------------------------*/
#ifndef PRE_WADE_GFX
#include <libvu0.h>

#include "algebra.h"
#ifdef PROJECT_KELLYSLATER
#include "ngl_ps2.h"
#else
//#include "krender.h"
#endif

void matrix4x4_to_sceVu0FMATRIX(const matrix4x4 &our_mtx, sceVu0FMATRIX &sce_mtx);
void sceVu0FMATRIX_to_matrix4x4(const sceVu0FMATRIX &sce_mtx, matrix4x4 &our_mtx);

inline void set_cursor( int val ) 
{ 
  // null on ps2
}

#endif

#endif
