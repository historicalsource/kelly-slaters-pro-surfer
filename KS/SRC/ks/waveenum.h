#ifndef WAVEENUM_H
#define WAVEENUM_H

#undef USEINFO2
#define USEINFO2(region, color) WAVE_REGION##region, 
enum WaveRegionEnum
{
#include "waveregion.txt"
	WAVE_REGIONMAX
};

#undef USEINFO2
#define USEINFO2(emitter, color) WAVE_EM_##emitter, 
enum WaveEmitterEnum {
#include "waveemitter.txt"
	WAVE_EM_MAX
};

#undef USEINFO
#define USEINFO(x, z, name, color) WAVE_Marker##name, 
enum WaveMarkerEnum {
	WAVE_MarkerInvalid = -1, 
#include "wavemarker.txt"
	WAVE_MarkerMax
};

#endif

