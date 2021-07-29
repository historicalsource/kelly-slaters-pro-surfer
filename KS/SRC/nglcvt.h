#if defined(TARGET_XBOX)
#include "ngl.h"
#elif defined(TARGET_GC)
#include "ngl.h"
#else
#include "ngl_ps2.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "frustum.h"

void plane2nglPlane( const plane &ip, nglPlane &op );
void frustum2nglFrustum( const frustum &ifr, nglFrustum &ofr );


