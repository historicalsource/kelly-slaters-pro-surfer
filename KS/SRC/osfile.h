#ifndef OSFILE_H
#define OSFILE_H
/*-------------------------------------------------------------------------------------------------------
  FILE.H - Include file redirect for file module.
-------------------------------------------------------------------------------------------------------*/

#if defined(TARGET_PC)
#include "hwospc/w32_file.h"
#elif defined(TARGET_MKS)
#include "hwosmks/sy_file.h"
#elif defined(TARGET_PS2)
#include "hwosps2/ps2_file.h"
#elif defined(TARGET_NULL)
#include "hwosnull/null_file.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_file.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_file.h"
#endif

#endif
