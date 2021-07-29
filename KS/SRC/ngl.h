// NGL include file redirector.

#ifdef TARGET_PS2
#include "ngl_ps2.h"
#include "ksngl.h"
#elif defined(_XBOX)
#include "ngl_xbox.h"
#elif defined(TARGET_GC)
#include "hwosgc\ngl_gc.h"
#else
#error Need to specify platform for NGL.h.
#endif
