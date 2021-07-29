#ifndef PS2_WAVERENDER_H
#define PS2_WAVERENDER_H

#ifndef TARGET_PS2
#error This file should only be included in builds where TARGET_PS2 is defined. (dc 06/06/02)
#endif

enum {
	KSMAT_FOAM_MAP             = 0x00000001, 
	KSMAT_WAVETRANS            = 0x00000002,
};
void WAVERENDER_SetWaveDarkParams( float scale, float off );
void WAVERENDER_SetWaveHighlightParams( float scale, float off, float cscale, float coff, nglVector sun, float tscale, float toff, float tmin );
void WAVERENDER_RenderSectionWave( u_int*& _Packet, void* Param );

#endif	// #ifndef PS2_WAVERENDER_H