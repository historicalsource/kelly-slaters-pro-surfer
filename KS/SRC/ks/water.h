#ifndef WATER_H
#define WATER_H

/*
extern nglTextureAnim *WaterTexAnimDark;
extern nglTextureAnim *WaterTexAnimLight;
extern nglTextureAnim *WaterTexAnimHighlight;
*/

void WATER_Init(void);
void WATER_Cleanup(void);
void WATER_Create( const int heroIdx );
void WATER_ListAdd(void);
float WATER_Altitude(float x, float z);
void WATER_Normal(float x, float z, float &nx, float &ny, float &nz);
/*
void WATER_LoadTexAnims(void);

*/

extern bool WATER_GetDrawWave(void);
extern void WATER_SetDrawWave(bool onoff);
extern bool WATER_GetDrawFar(void);
extern void WATER_SetDrawFar(bool onoff);
extern bool WATER_GetDrawHorizon(void);
extern void WATER_SetDrawHorizon(bool onoff);
extern bool WATER_GetDrawNear(void);
extern void WATER_SetDrawNear(bool onoff);
extern bool WATER_GetDrawSeam(void);
extern void WATER_SetDrawSeam(bool onoff);

#endif /* #ifndef WATER_H */
