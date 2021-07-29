#ifndef HWMATH_H
#define HWMATH_H


#if defined(TARGET_PC)
#include "hwospc\x86_math.h"
#elif defined(TARGET_XBOX)
#include "hwosxb\xb_math.h"
#elif defined(TARGET_MKS)
#include "hwosmks\sh4_math.h"
#elif defined(TARGET_PS2)
#include "hwosps2\ps2_math.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_math.h"
#elif defined(TARGET_NULL)
#include "hwosnull\null_math.h"
#endif

#include "usefulmath.h"

#endif // HWMATH_H
