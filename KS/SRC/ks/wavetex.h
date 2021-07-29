
#ifndef _wavetex_h
#define _wavetex_h

#include "ksngl.h"

enum {
	WAVETEX_PASSBASE = 0,
	WAVETEX_PASSDARK    ,
	WAVETEX_PASSHIGH    ,
	WAVETEX_PASSSPOT    ,
	WAVETEX_PASSENVM    ,
	WAVETEX_PASSFULL    ,
	WAVETEX_PASSFOAM    ,
};

enum {
	WAVETEX_TEXLITE  = 0,
	WAVETEX_TEXDARK     ,
	WAVETEX_TEXHIGH     ,
	WAVETEX_TEXSPOT     ,
	WAVETEX_TEXENVM     ,
	WAVETEX_TEXFOAM     ,
	WAVETEX_TEXBUMP     ,
	WAVETEX_TEXBUMP2    ,
};

enum {
	WAVETEX_MATWAVE     ,
	WAVETEX_MATSEAM     ,
	WAVETEX_MATNEAR     ,
	WAVETEX_MATFAR      ,
	WAVETEX_MATHORIZON  ,
	WAVETEX_MAT_MAX     ,
};


#define WAVETRANSPARENCY

// for debug menus (dc 01/11/07)
extern float WAVETEX_DarkAlphaScale;
extern float WAVETEX_DarkAlphaOffset;
extern float WAVETEX_DarkHighAlphaScale;
extern float WAVETEX_DarkHighAlphaOffset;
extern float WAVETEX_DarkUnderAlphaScale;
extern float WAVETEX_DarkUnderAlphaOffset;
extern float WAVETEX_HighlightAlphaScale;
extern float WAVETEX_HighlightAlphaOffset;
extern float WAVETEX_CoreHighlightAlphaScale;
extern float WAVETEX_CoreHighlightAlphaOffset;
extern nglVector WAVETEX_SunPos;
extern float WAVETEX_transcale;
extern int WAVETEX_transmin;
extern int WavetexDebug_ShadowPass;

void WAVETEX_Init( void );
bool WAVETEX_LoadTextureAnims( bool resetparms=true );
bool WAVETEX_UnloadTextureAnims( void );
bool WAVETEX_ReloadTextureAnims( void );
nglTexture *WAVETEX_GetTexture( int textype, int frame=-1 );
#ifndef TARGET_XBOX
u_int WAVETEX_AverageColor( int textype, unsigned int frame );
#endif
u_int WAVETEX_Width( int textype=0, int frame=-1 );
u_int WAVETEX_Height( int textype=0, int frame=-1 );
float WAVETEX_FrameFix( int textype, float frame );
void WAVETEX_SetCameraPos( );
void WAVETEX_SetShadowScale( float s );

void WAVETEX_InitMaterial( nglMaterial &Mat, int textype, int passtype, int mattype );
void WAVETEX_PrepareMaterials( void );
void WAVETEX_Update( void );	// All once-per-rendered-frame tasks (dc 06/14/02)

void WAVETEX_SetMatSort( u_int matsort, int matid );
void WAVETEX_SetMatTextured( bool onOff, int matid );
void WAVETEX_SetMatBlended( bool onOff, int matid );
void WAVETEX_SetMatZSorted( bool onOff, int matid );
void WAVETEX_SetMatPlayerShadows( bool onOff, int matid );
void WAVETEX_SetMatTranslucent( bool onOff, int matid );
void WAVETEX_SetMatFoamy( bool onOff, int matid );

enum WavePasses {
	WAVETEX_MAINPASS = 1<<0,
	WAVETEX_FOAMPASS = 1<<1,
};


u_int WAVETEX_InitWaveMesh( u_int nvert, int mattype, void (*fillinfunc)(void) = NULL );
void WAVETEX_FreeWaveMesh( u_int meshid );
void WAVETEX_UseWaveMesh( u_int rv, int mattype, u_int submesh=0 );
void WAVETEX_UseWaveSubMesh( u_int rv );
void WAVETEX_FillWaveMesh( u_int wavemeshid
#ifndef TARGET_XBOX
	, bool uselite, bool usedark, bool usehigh, bool usespot, bool useenv
#endif
);
void WAVETEX_SubmitMesh( u_int rv, const nglMatrix &LocalToWorld, nglRenderParams &rp
#ifndef TARGET_XBOX
	, bool uselite, bool usedark, bool usehigh, bool usespot, bool useenv
#endif
);
void WAVETEX_CloseWaveMesh( void );

void WAVETEX_SetPlayer( int player );
int WAVETEX_GetPlayer( void );
void WAVETEX_ClearShadows( void );
void WAVETEX_CheckClearShadows( void );


#define WAVETEX_MESHMAN


inline void SetMatSort( nglMaterial &Mat, int usort )
{
}


#endif



