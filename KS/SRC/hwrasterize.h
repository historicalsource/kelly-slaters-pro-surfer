#ifndef HWRASTERIZE_H
#define HWRASTERIZE_H
/*-------------------------------------------------------------------------------------------------------
	HWRASTERIZE.H - Include file redirect to appropriate rasterization module based on target platform.
-------------------------------------------------------------------------------------------------------*/

#if defined(TARGET_PC)
#include "hwospc/d3d_rasterize.h"
#elif defined(TARGET_MKS)
#include "hwosmks/set5_rasterize.h"
#elif defined(TARGET_PS2)
#include "hwosps2/ps2_rasterize.h"
#elif defined(TARGET_NULL)
#include "hwosnull/null_rasterize.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_rasterize.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_rasterize.h"
#else
#error no target defined
#endif
#endif
