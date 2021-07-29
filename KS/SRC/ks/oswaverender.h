#ifndef OSWAVERENDER_H
#define OSWAVERENDER_H
/*-------------------------------------------------------------------------------------------------------
  WAVERENDER.H - Include file redirect for wave render module.
-------------------------------------------------------------------------------------------------------*/

#if defined(TARGET_PC)
//#include "hwospc/w32_waverender.h"
#elif defined(TARGET_MKS)
//#include "hwosmks/sy_waverender.h"
#elif defined(TARGET_PS2)
#include "hwosps2/ps2_waverender.h"
#elif defined(TARGET_NULL)
//#include "hwosnull/null_waverender.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_waverender.h"
#elif defined(TARGET_GC)
//#include "hwosgc\gc_waverender.h"
#endif

#endif
