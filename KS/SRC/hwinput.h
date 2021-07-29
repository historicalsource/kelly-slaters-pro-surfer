// Input include file redirector.
#if defined(TARGET_PC)
#include "di_input.h"
#elif defined(TARGET_XBOX)
#include "hwosxbox/xbox_input.h"
#elif defined(TARGET_PS2)
#include "hwosps2/ps2_input.h"
#elif defined(TARGET_GC)
#include "hwosgc/gc_input.h"
#else
#error "Need to specify platform for hwinput.h."
#endif

