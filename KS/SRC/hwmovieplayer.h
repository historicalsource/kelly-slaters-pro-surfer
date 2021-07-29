#ifndef _HWMOVIEPLAYER_H_
#define _HWMOVIEPLAYER_H_

#ifdef TARGET_PC
	#include "hwospc/pc_movieplayer.h"
#elif defined ( TARGET_MKS )
	#include "hwosmks/mks_movieplayer.h"
#elif defined(TARGET_PS2)
	#include "hwosps2/ps2_movieplayer.h"
#elif defined(TARGET_NULL)
	#include "hwosnull/null_movieplayer.h"
#elif defined(TARGET_XBOX)
	#include "hwosxb/xb_movieplayer.h"
#elif defined(TARGET_GC)
	#include "hwosgc\gc_movieplayer.h"
#endif

#endif
