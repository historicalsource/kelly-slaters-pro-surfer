// NSL include file redirector.

#include "common/nsl.h"
#ifdef TARGET_PS2
#include "ps2/nsl_ps2.h"
#elif defined(_XBOX)
#include "xbox/nsl_xbox.h"
#elif defined(TARGET_GC)
// No.
#else
#error "You must define a platform for NSL"
#endif
