#include "ps2_graphics.h"

void matrix4x4_to_sceVu0FMATRIX(const matrix4x4 &our_mtx, sceVu0FMATRIX &sce_mtx)
{
  for (int row=0; row<4; ++row)
    for (int col=0; col<4; ++col)
      sce_mtx[row][col] = our_mtx[row][col];
}

void sceVu0FMATRIX_to_matrix4x4(const sceVu0FMATRIX &sce_mtx, matrix4x4 &our_mtx)
{
  for (int row=0; row<4; ++row)
    for (int col=0; col<4; ++col)
      our_mtx[row][col] = sce_mtx[row][col];
}
