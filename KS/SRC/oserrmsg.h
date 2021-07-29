#ifndef OSERRMSG_H
#define OSERRMSG_H
/*-------------------------------------------------------------------------------------------------------
  OSERRMSG.H - Include redirect for error message routines.
-------------------------------------------------------------------------------------------------------*/

#if defined(TARGET_PC)
#include "hwospc\w32_errmsg.h"
#elif defined(TARGET_MKS)
#include "hwosmks\mks_errmsg.h"
#elif defined(TARGET_PS2)
#include "hwosps2\ps2_errmsg.h"
#elif defined(TARGET_NULL)
#include "hwosnull\null_errmsg.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_errmsg.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_errmsg.h"
#else
#error no target defined
#endif
#endif
