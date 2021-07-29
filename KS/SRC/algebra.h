// Include redirect for platform specific Algebra library.
#ifndef ALGEBRA_H

#if (defined TARGET_PS2)
	#include "hwosps2\ps2_algebra.h"
#elif (defined TARGET_GC)
	#include "hwosgc/gc_algebra.h"
#elif (defined TARGET_XBOX)
	#include "hwosxb/xb_algebra.h"
#elif (defined TARGET_PC) && (defined USE_NGL)
	#include "hwosnglpc/algebra.h"
  #include "hwosnglpc/pc_algebra.h"
#else
	// pc version is (currently) generic
	#include "hwospc\pc_algebra.h"
#endif

#endif
