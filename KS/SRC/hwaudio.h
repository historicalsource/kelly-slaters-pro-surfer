#ifndef HWAUDIO_H
#define HWAUDIO_H
/*-------------------------------------------------------------------------------------------------------
  HWAUDIO.H - Include file redirect for the audio module.
-------------------------------------------------------------------------------------------------------*/

#if defined(TARGET_PC)
#include "hwospc\ds_audio.h"
#elif defined(TARGET_MKS)
#include "hwosmks\am_audio.h"
#elif defined(TARGET_PS2)
#include "hwosps2\ps2_audio.h"
#elif defined(TARGET_NULL)
#include "hwosnull\null_audio.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_audio.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_audio.h"
#endif /* _XBOX */
#endif
