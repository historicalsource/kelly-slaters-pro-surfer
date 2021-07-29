#ifndef XB_WAVERENDER_H
#define XB_WAVERENDER_H

#ifdef TARGET_XBOX	// These are still in NGL for other platforms

void WAVERENDER_Init(void);
void WAVERENDER_Cleanup(void);
nglMesh *WAVERENDER_CreateScratchMesh(u_int NVerts, void (*FillInFunc)(void));
void WAVERENDER_SetSectionRenderer(nglMesh *Mesh);
void WAVERENDER_ScratchAddVertexPNCUVSTR(float X, float Y, float Z, float NX, float NY,
                                 float NZ, uint32 Color, float U, float V, 
								 float SX, float SY, float SZ, float TX, float TY, float TZ, 
								 float RX, float RY, float RZ, uint8 FA = 0
								 );

#define WAVERENDER_MeshFastWriteVertexPNCUVSTR WAVERENDER_ScratchAddVertexPNCUVSTR

#endif	// #ifdef TARGET_XBOX

#endif	// #ifndef XB_WAVERENDER_H