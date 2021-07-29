// NVL include file redirector.

#ifdef TARGET_PS2
#include "nvl_ps2.h"
#include "nvlstream_ps2.h"
#elif defined(_XBOX)
#include "nvl_xbox.h"
#elif defined(TARGET_GC)
#include "nvl_gc.h"
#include "nvlstream_gc.h"
#else
#error Need to specify platform for NVL.h.
#endif
