// Include redirect for target-specific allocation functions.
#ifndef OSALLOC_H
#define OSALLOC_H

#if defined(TARGET_PC)
#include "hwospc\w32_alloc.h"
#elif defined(TARGET_MKS)
#include "hwosmks\sy_alloc.h"
#elif defined(TARGET_PS2)
#include "hwosps2\ps2_alloc.h"
#elif defined(TARGET_NULL)
#include "hwosnull\null_alloc.h"
#elif defined(TARGET_XBOX)
#include "hwosxb\xb_alloc.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_alloc.h"
#endif
#endif
