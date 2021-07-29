// files for ps2 link speedup
#include "global.h"

#include "trail.cpp"
#include "ksfx.cpp"
#include "underwtr.cpp"
#include "water.cpp"
#include "wave.cpp"
#include "wavedata.cpp"
#include "wavetex.cpp"

#ifndef TARGET_GC
// This file should remain the last file included. It does strange things.
#include "ksngl.cpp"
#endif
