#ifndef NL_HEADER
#define NL_HEADER

#ifdef NL_PS2
#include "../ps2/nl_ps2.h"
#elif defined (NL_GC)
#include "../gamecube/nl_gc.h"
#elif defined (NL_PC)
#include "../pc/nl_pc.h"
#elif defined (NL_XBOX)
#include "../xbox/nl_xbox.h"
#elif defined (NSL_SOUND_TOOL)
// use the pc definitions for the tool, since it's a pc tool.
#include "../pc/nl_pc.h"
#endif

#endif