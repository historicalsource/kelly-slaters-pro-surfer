#ifndef _UNDERWTR_H
#define _UNDERWTR_H

void UNDERWATER_Init(int curbeach);
void UNDERWATER_CameraReset( void );
bool UNDERWATER_CameraUnderwater( const int playerIdx );
float UNDERWATER_CameraOverWaterDist( const int playerIdx );
void UNDERWATER_CameraSelect(const int playerIdx);
void UNDERWATER_CameraChecks(const int playerIdx);
void UNDERWATER_ScrollBottom( void );
void UNDERWATER_EntitiesTrackCamera(void);




#endif	//_UNDERWTR_H
