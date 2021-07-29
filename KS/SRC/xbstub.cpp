// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "xbglobals.h"

#include <stdio.h>

static bool print_stubs = true;

void xbstub(const char *str, const char *f, int l)
{
  if(print_stubs)
    nglPrintf( "STUB [%s:%d] %s\n", f,l, str );
}

/*	Moved to ksngl.h for centralization.  (dc 05/31/02)
void ksNormalize( float &nx, float &ny, float &nz )
{
  float nlength = float(sqrt(nx * nx + ny * ny + nz * nz));

  if(nlength == 0.0f)
    return;

  nx /= nlength;
  ny /= nlength;
  nz /= nlength;

  assert( nx == nx );
  assert( ny == ny );
  assert( nz == nz );
}
*/

u_int nglTakeScreen = 0;
