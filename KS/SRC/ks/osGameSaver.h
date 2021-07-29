#ifndef __OS_GAME_SAVER_H_
#define __OS_GAME_SAVER_H_

#if defined(TARGET_XBOX)
#include "hwosxb/xb_GameSaver.h"
#elif defined(TARGET_GC)
#include "hwosgc/gc_GameSaver.h"
#elif defined(TARGET_PS2)
#include "hwosps2/ps2_GameSaver.h"
#endif


#endif // __OS_GAME_SAVER_H_