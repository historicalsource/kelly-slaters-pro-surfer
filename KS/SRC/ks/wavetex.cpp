#include "global.h"		// Must come first

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "ngl.h"	// For graphics engine
#include "light.h"
#include "conglom.h"
#include "inputmgr.h"
#include "kellyslater_controller.h"
#include "wds.h"
#else
// let's get cute
#include "ngl_ps2.h"	// For graphics engine
#endif /* TARGET_XBOX JIV DEBUG */

#include "spline.h"		// For spline calculations
#include "parser.h"		// For io functions
#include "wave.h"		// For WAVE_MESHWIDTH and WAVE_MESHDEPTH
#include "water.h"		// For WATER_Tile_X, WATER_Tile_Z, and WATER_Altitude
#include "wavetex.h"		// For assorted WAVETEX_ calls
#include "beachdata.h"		// For beach specific settings
#include "game.h"		// For beach specific settings
#include "ksngl.h"		// For beach specific settings - how did you know I used copy & paste?
#include "projconst.h"
#include "geomgr.h"
#include "oswaverender.h"	// For WAVERENDER_Init(), WAVERENDER_CreateScratchMesh, etc.

#ifdef PROJECT_KELLYSLATER	// exclude from wave standalone app
#include "osdevopts.h"
#endif

#ifdef TARGET_GC
#include "gc_ifl.h"
#endif

#ifdef TARGET_PS2
#define ENABLE_FOAM_PASS
#endif

#undef USEINFO
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFO(item, type, lo, hi, step, init, menu) USEINFO##type(item)
#define USEINFOINT(item) int item;
#define USEINFOFLOAT(item) float item;
#ifndef TARGET_GC
	// I hate Metrowerks I hate Metrowerks I hate Metrowerks I hate Metrowerks
static
#endif
struct WavetexDebugStruct
{
#include "wavetexdebug.txt"
}
#undef USEINFO
#define USEINFO(item, type, lo, hi, step, init, menu) init,
WavetexDebug =
{
#include "wavetexdebug.txt"
};

#define GC_MONOTEXTURE "Wtez"

#if defined(TARGET_PS2)
#define WAVETEX_NUMBUFFER 2	// Needed to prevent overwriting active DMA chain
#elif defined (TARGET_XBOX)
#define WAVETEX_NUMBUFFER 3	// Needed to avoid stall waiting for buffer lock
#else
#define WAVETEX_NUMBUFFER 2	// Do we need any more?
#endif
struct WaveMeshHandle
{
	nglMesh *baseid[WAVETEX_NUMBUFFER];	// double-buffered (dc 06/14/02)
};
static u_int WAVETEX_BufferIndex = 0;	// For static scratch mesh double-buffering (dc 06/14/02)

#ifdef TARGET_XBOX
static const char *WAVETEX_DiffuseMapName = "wtc";
//static
const char *WAVETEX_EnvironmentMapName = "CubeEnv";	//"OpenOceanCube";	//"CubeLobby";	//"cube_map_nvidia";
//static
const char *WAVETEX_BumpMapName = "wtcnormal";	//"bump_map_nvidia_Normal";	//"FlatNormal";
static const char *WAVETEX_SpecularMapName = "wtcspecular";
static const char *WAVETEX_FoamMapName = "FomE";
#endif

float WAVETEX_DarkAlphaScale = 225.0f;
float WAVETEX_DarkAlphaOffset = 0.0f;
float WAVETEX_DarkHighAlphaScale = 225.0f;
float WAVETEX_DarkHighAlphaOffset = 0.0f;
float WAVETEX_DarkUnderAlphaScale = 225.0f;
float WAVETEX_DarkUnderAlphaOffset = 0.0f;
float WAVETEX_HighlightAlphaScale = 192.f;
float WAVETEX_HighlightAlphaOffset = -64.0f;
float WAVETEX_CoreHighlightAlphaScale = 320.f;
float WAVETEX_CoreHighlightAlphaOffset = -256.0f;
float WAVETEX_TransHighlightAlphaScale = 4096.f;
float WAVETEX_TransHighlightAlphaOffset = 0.0f;

float WAVETEX_transcale=8.0f;
int WAVETEX_transmin=64;
int WAVETEX_transmax=128;
int WAVETEX_transval=128;
int WAVETEX_alltrans=80;


#ifdef TARGET_GC
extern float WaveBumpNormalScale;
#endif



static int wavetex_currentmat;
//static nglMaterial WaveTexFMat;
static nglMaterial WaveTexLMat[2][WAVETEX_MAT_MAX];


#if 1 //ndef TARGET_GC
const int shadowtexw=128;
const int shadowtexh=128;
#else
const int shadowtexw=256;
const int shadowtexh=256;
#endif



nglVector WAVETEX_SunPos(0, 1000, -3500, 1);
nglVector WAVETEX_SunPosNorm(0, 1000, -3500, 1);

extern float WAVE_camx;
extern float WAVE_camy;
extern float WAVE_camz;
//nglVector WAVETEX_campos(0, 0, 0, 1);
float lo_dark_limit=0.0f;
float hi_dark_limit=110.0f;


//void WAVERENDER_SetWaveCameraPos( nglVector cam );


bool nglReadTextureFile( char* FileName, nglFileBuf* File, u_int Align );

#ifndef TARGET_GC
static nglTexture *WaveTexAnimLight=NULL;
static nglTexture *WaveTexAnimDark=NULL;
static nglTexture *WaveTexAnimHighlight=NULL;
static nglTexture *WaveTexAnimSpotlight=NULL;
static nglTexture *WaveTexAnimFoam=NULL;
#endif
static nglFileBuf WaveAnimFile;

//static nglFileBuf FoamAnimFile;


static nglTexture *ShadowSourceTexture=NULL;
static nglTexture *ShadowTargetTexture=NULL;
static nglTexture *ShadowBlurryTexture=NULL;
//static nglTexture *ShadowReflecTexture=NULL;
static nglTexture *ShadowTestTexture=NULL;
static nglTexture *FoamTexture=NULL;

#ifdef TARGET_GC	// Toby added these include guards to stop warnings in PS2 compile.
static gc_ifl WaveTexAnimLight;
static gc_ifl BumpTexture2;
static gc_ifl WaveTexAnimFoam;
static nglTexture *BumpTexture=NULL;
static nglTexture *EnvTexture=NULL;
nglTexture *DiffusePaletteTexture=NULL;
nglTexture *ElevationPaletteTexture=NULL;
#endif


extern float WAVE_TexAnimFrame;

//
// Temporary fix for wave texturing problems caused by multiplayer
//

static bool WAVETEX_CameraUnderwater( void )
{
	int p=WAVETEX_GetPlayer();
	if (g_game_ptr->is_splitscreen()) return false;
	return UNDERWATER_CameraUnderwater(p);
}


static void WAVETEX_WriteMaterialParms( void )
{
	//WAVETEX_SetCameraHeight( WAVE_camy );
	WAVETEX_SetCameraPos( /*
WAVETEX_campos[0],WAVETEX_campos[1],WAVETEX_campos[2]*/
 );
	WAVETEX_SunPosNorm = WAVETEX_SunPos;
	KSNGL_Normalize( WAVETEX_SunPosNorm );

#ifdef TARGET_PS2
	//WAVERENDER_SetWaveDarkParams( WAVETEX_DarkAlphaScale, WAVETEX_DarkAlphaOffset );
	WAVERENDER_SetWaveHighlightParams( WAVETEX_HighlightAlphaScale,	WAVETEX_HighlightAlphaOffset,
	   WAVETEX_CoreHighlightAlphaScale,	WAVETEX_CoreHighlightAlphaOffset,
	   WAVETEX_SunPosNorm,
	   WAVETEX_transcale,	WAVETEX_TransHighlightAlphaOffset,
	   WAVETEX_CameraUnderwater() ? 128 :WAVETEX_transmin	// no translucency underwater (dc 06/28/02)
	);
#endif
}

static int wavetex_playerid=0;

void WAVETEX_SetPlayer( int player )
{
	wavetex_playerid=player;
	//wavetex_currentmat=player;
	//nglSetCurrentCamera( player );
}

int WAVETEX_GetPlayer( void )
{
	return wavetex_playerid;
}


void WAVETEX_SetCameraPos( )
{
	const float &y = WAVE_camy;
/*
	WAVETEX_campos[0]=x;
	WAVETEX_campos[1]=y;
	WAVETEX_campos[2]=z;
*/
//	WAVERENDER_SetWaveCameraPos( WAVETEX_campos );
#ifdef TARGET_PS2
	#if 1
	if ( WAVETEX_CameraUnderwater() )
	{
		WAVERENDER_SetWaveDarkParams( WAVETEX_DarkUnderAlphaScale/128.0f, WAVETEX_DarkUnderAlphaOffset/128.0f );
	}
	else if ( y<=lo_dark_limit )
	{
	#endif
		WAVERENDER_SetWaveDarkParams( WAVETEX_DarkAlphaScale/128.0f, WAVETEX_DarkAlphaOffset/128.0f );
	#if 1
	}
	else if ( y>=hi_dark_limit )
	{
		WAVERENDER_SetWaveDarkParams( WAVETEX_DarkHighAlphaScale/128.0f, WAVETEX_DarkHighAlphaOffset/128.0f );
	}
	else
	{
		float hi=(y-lo_dark_limit)/(hi_dark_limit-lo_dark_limit);
		float lo=1.0f-hi;
		WAVERENDER_SetWaveDarkParams( (lo*WAVETEX_DarkAlphaScale + hi*WAVETEX_DarkHighAlphaScale)/128.0f,
		                      (lo*WAVETEX_DarkAlphaOffset + hi*WAVETEX_DarkHighAlphaOffset)/128.0f);
	}
	#endif
#endif
	#ifdef TARGET_GC
		WaveBumpNormalScale= WavetexDebug.GCBumpScale;
	#endif
}

#ifdef TARGET_GC
	void FixupATEForBogusEndianMachines( char *atefile );
#endif

static bool newshadowbuf=true;

bool WAVETEX_LoadTextureAnims( bool resetparms )
{
	if ( resetparms )
	{
		int curbeach=g_game_ptr->get_beach_id();

		// Divided by 128 to compensate for change in VU1 code (dc 08/30/01)
		WAVETEX_DarkAlphaScale           = BeachDataArray[curbeach].darkscale; // / 128;
		WAVETEX_DarkAlphaOffset          = BeachDataArray[curbeach].darkoff; // / 128  ;
		WAVETEX_DarkHighAlphaScale           = BeachDataArray[curbeach].darkhiscale; // / 128;
		WAVETEX_DarkHighAlphaOffset          = BeachDataArray[curbeach].darkhioff; // / 128  ;
		WAVETEX_DarkUnderAlphaScale           = BeachDataArray[curbeach].darkunderscale; // / 128;
		WAVETEX_DarkUnderAlphaOffset          = BeachDataArray[curbeach].darkunderoff; // / 128  ;
		WAVETEX_HighlightAlphaScale      = BeachDataArray[curbeach].highscale      ;
		WAVETEX_HighlightAlphaOffset     = BeachDataArray[curbeach].highoff        ;
		WAVETEX_CoreHighlightAlphaScale  = BeachDataArray[curbeach].corescale      ;
		WAVETEX_CoreHighlightAlphaOffset = BeachDataArray[curbeach].coreoff        ;

		WAVETEX_SunPos[0] = BeachDataArray[curbeach].sunx     ;
		WAVETEX_SunPos[1] = BeachDataArray[curbeach].suny     ;
		WAVETEX_SunPos[2] = BeachDataArray[curbeach].sunz     ;

		WAVETEX_transcale=BeachDataArray[curbeach].transcale;
		WAVETEX_transmin =BeachDataArray[curbeach].transmin;
		WAVETEX_transmax=128;

		#ifdef TARGET_GC
		WavetexDebug.GCBumpScale=BeachDataArray[curbeach].gcbumpscale;
		WaveBumpNormalScale=WavetexDebug.GCBumpScale;
		WavetexDebug.GCColorNearA        = ((BeachDataArray[curbeach].gccolornear >> 24) & 0xFF) ;
		WavetexDebug.GCColorNearR        = ((BeachDataArray[curbeach].gccolornear >> 16) & 0xFF) ;
		WavetexDebug.GCColorNearG        = ((BeachDataArray[curbeach].gccolornear >> 8 ) & 0xFF) ;
		WavetexDebug.GCColorNearB        = ((BeachDataArray[curbeach].gccolornear >> 0 ) & 0xFF) ;
		WavetexDebug.GCColorFarA         = ((BeachDataArray[curbeach].gccolorfar  >> 24) & 0xFF) ;
		WavetexDebug.GCColorFarR         = ((BeachDataArray[curbeach].gccolorfar  >> 16) & 0xFF) ;
		WavetexDebug.GCColorFarG         = ((BeachDataArray[curbeach].gccolorfar  >> 8 ) & 0xFF) ;
		WavetexDebug.GCColorFarB         = ((BeachDataArray[curbeach].gccolorfar  >> 0 ) & 0xFF) ;
		WavetexDebug.GCFalloffScale  = BeachDataArray[curbeach].gcrgbfadescale;
		WavetexDebug.GCFalloffOffset = BeachDataArray[curbeach].gcrgbfadeoffset;
		#endif
	}

#ifdef PROJECT_KELLYSLATER	// exclude from wave standalone app
  stringx texture_path = "levels\\"
			+ g_game_ptr->get_beach_name() +
	  "\\" +
	  os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) +
	  "\\FomE\\" ;
  nglSetTexturePath( (char *)texture_path.c_str() );
#endif

	char thisisapainintheass[256];

	#if defined(TARGET_XBOX)
	  // JIV FIXME
	  sprintf(thisisapainintheass, "stubstripes.ifl");
	//    sprintf(thisisapainintheass, "\\xbox_textures\\WtC.ate");
	#else
		strcpy(thisisapainintheass,nglGetTexturePath());
		strcat(thisisapainintheass,"fom.ate");
	#endif /* TARGET_XBOX JIV DEBUG */
	bool rv;

#ifdef TARGET_GC
	WaveTexAnimFoam.init( "fomE" );
#else
	WaveTexAnimFoam  = nglLoadTexture(nglFixedString("fomE"));
#endif

#ifdef PROJECT_KELLYSLATER	// exclude from wave standalone app
  texture_path = "levels\\"
			+ g_game_ptr->get_beach_name() +
	  "\\" +
	  os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) +
	  "\\";
  nglSetTexturePath( (char *)texture_path.c_str() );
#endif

  WaveAnimFile.Size=0; WaveAnimFile.Buf=NULL;


	#if defined(TARGET_XBOX)
	  // JIV FIXME
	  sprintf(thisisapainintheass, "stubstripes.ifl");
	//    sprintf(thisisapainintheass, "\\xbox_textures\\WtC.ate");
	#else
		strcpy(thisisapainintheass,nglGetTexturePath());
		strcat(thisisapainintheass,"WtC" PLATFORM_ANIMTEX_EXT);
		//strcat(thisisapainintheass,"WtC.ate"); // PLATFORM_ANIMTEX_EXT);
	#endif /* TARGET_XBOX JIV DEBUG */

	#if defined(TARGET_GC)
	#if 0
	DiffusePaletteTexture  =nglLoadTexture(nglFixedString("GC_Diffuse_Palette"));
	ElevationPaletteTexture=nglLoadTexture(nglFixedString("GC_Elevation_Palette"));
	#endif
	EnvTexture=nglLoadTexture(nglFixedString("EnvMapAdd"));
	WaveTexAnimLight.init( "WtCx" );
	BumpTexture2.init( "Wodz" );
	#else
	rv=nglReadFile( thisisapainintheass, &WaveAnimFile, 128 );
	if (rv)
	{
#ifdef TARGET_PS2
		WaveTexAnimSpotlight = nglLoadTextureInPlace(nglFixedString("WtCz"),NGLTEX_ATE,WaveAnimFile.Buf, WaveAnimFile.Size);
		WaveTexAnimHighlight = nglLoadTextureInPlace(nglFixedString("WtCo"),NGLTEX_ATE,WaveAnimFile.Buf, WaveAnimFile.Size);
		WaveTexAnimDark = nglLoadTextureInPlace(nglFixedString("WtCx"),NGLTEX_ATE,WaveAnimFile.Buf, WaveAnimFile.Size);
		WaveTexAnimLight = nglLoadTextureInPlace(nglFixedString("WtCy"),NGLTEX_ATE,WaveAnimFile.Buf, WaveAnimFile.Size);
		if (NULL==WaveTexAnimFoam)
			WaveTexAnimFoam  = nglLoadTextureInPlace(nglFixedString("WtCf"),NGLTEX_ATE,WaveAnimFile.Buf, WaveAnimFile.Size);
		//WavetexDebug.DrawTexture=true;
#endif //TARGET_PS2
	}
	else
	{
		//WavetexDebug.DrawTexture=false;
		//WavetexDebug.Transparency=false;
	}
	#endif

	static int shadow_texture_flags;
	#ifdef TARGET_XBOX
		shadow_texture_flags=NGLTF_32BIT|NGLTF_LINEAR;
	#else
		shadow_texture_flags=NGLTF_32BIT;
		//shadow_texture_flags=NGLTF_32BIT|NGLTF_VRAM_ONLY;
	#endif

	ShadowTestTexture=NULL;
	ShadowSourceTexture=nglCreateTexture(shadow_texture_flags,shadowtexw,shadowtexh);
#ifdef TARGET_PS2
	nglLockTexturePS2(ShadowSourceTexture);
#endif
	//ShadowTargetTexture=nglCreateTexture(shadow_texture_flags,shadowtexw,shadowtexh);
#ifdef TARGET_PS2
	//nglLockTexturePS2(ShadowTargetTexture);
#endif
	//ShadowBlurryTexture=nglCreateTexture(shadow_texture_flags,shadowtexw,shadowtexh);
#ifdef TARGET_PS2
	//nglLockTexturePS2(ShadowBlurryTexture);
#endif

	newshadowbuf=true;
	WAVETEX_ClearShadows();

	for ( int i=WAVETEX_MATWAVE; i<WAVETEX_MAT_MAX; i++ )
	{
		WAVETEX_InitMaterial( WaveTexLMat[0][i], WAVETEX_TEXLITE, WAVETEX_PASSFULL, i );
		WAVETEX_InitMaterial( WaveTexLMat[1][i], WAVETEX_TEXLITE, WAVETEX_PASSFULL, i );
	}
	//WAVETEX_InitMaterial( WaveTexFMat, WAVETEX_TEXFOAM, WAVETEX_PASSFOAM );
	return TRUE;
}

#ifdef TARGET_GC

u_int WAVETEX_GCNearColor( void )
{
	return
		(WavetexDebug.GCColorNearR << 24) +
		(WavetexDebug.GCColorNearG << 16) +
		(WavetexDebug.GCColorNearB <<  8) +
		(WavetexDebug.GCColorNearA <<  0) ;
}
u_int WAVETEX_GCFarColor( void )
{
	return
		(WavetexDebug.GCColorFarR  << 24) +
		(WavetexDebug.GCColorFarG  << 16) +
		(WavetexDebug.GCColorFarB  <<  8) +
		(WavetexDebug.GCColorFarA  <<  0) ;
}
float WAVETEX_GCNFFadeScale( void )
{
	return WavetexDebug.GCFalloffScale  ;
}
float WAVETEX_GCNFFadeOffset( void )
{
	return WavetexDebug.GCFalloffOffset ;
}




#endif




void WAVETEX_CheckClearShadows( void )
{
	if (newshadowbuf)
	{
		newshadowbuf=false;
		WAVETEX_ClearShadows();
	}
}


void nglReleaseFile( nglFileBuf * );

bool WAVETEX_UnloadTextureAnims( void )
{
	if (ShadowSourceTexture)
	{
#ifdef TARGET_PS2
		nglUnlockTexturePS2(ShadowSourceTexture);
#endif
		nglDestroyTexture(ShadowSourceTexture);
	}
	if (ShadowTargetTexture)
	{
#ifdef TARGET_PS2
		nglUnlockTexturePS2(ShadowTargetTexture);
#endif
		nglDestroyTexture(ShadowTargetTexture);
	}
	if (ShadowBlurryTexture)
	{
#ifdef TARGET_PS2
		nglUnlockTexturePS2(ShadowBlurryTexture);
#endif
		nglDestroyTexture(ShadowBlurryTexture);
	}
#ifdef TARGET_GC
	WaveTexAnimLight.deinit();
  BumpTexture2.deinit();
	WaveTexAnimFoam.deinit();
	if( BumpTexture )
    nglReleaseTexture( BumpTexture );

  if( EnvTexture )
    nglReleaseTexture( EnvTexture );
#else
	if( WaveTexAnimLight )
		nglReleaseTexture( WaveTexAnimLight );
	if (WaveTexAnimSpotlight)
		nglReleaseTexture(WaveTexAnimSpotlight);
	if (WaveTexAnimHighlight)
		nglReleaseTexture(WaveTexAnimHighlight);
	if (WaveTexAnimDark)
		nglReleaseTexture(WaveTexAnimDark);
  if (WaveTexAnimFoam)
		nglReleaseTexture(WaveTexAnimFoam);
	if (FoamTexture)
		nglReleaseTexture(FoamTexture);
#endif
	nglReleaseFile( &WaveAnimFile );
	//nglReleaseFile( &FoamAnimFile );
	ShadowSourceTexture=NULL;
	ShadowTargetTexture=NULL;
	ShadowBlurryTexture=NULL;
	FoamTexture=NULL;
	#ifdef TARGET_GC
	if (EnvTexture)
		nglReleaseTexture(EnvTexture);
	EnvTexture=NULL;
	#endif
	return TRUE;
}

bool WAVETEX_ReloadTextureAnims( void )
{
		// this is used solely for debugging purposes
	bool memlocked=mem_malloc_locked();
	mem_lock_malloc(false);

	WAVETEX_UnloadTextureAnims();
	WAVETEX_LoadTextureAnims(false);

	mem_lock_malloc(memlocked);
	return TRUE;
}

nglTexture *WAVETEX_GetShadowTex( void )
{
	if ( WavetexDebug.ShadowBlur && ShadowBlurryTexture)
		return ShadowBlurryTexture;
	if ( WavetexDebug.ShadowDistort && ShadowTargetTexture)
		return ShadowTargetTexture;
	return ShadowSourceTexture;
}

bool bigfunkyhack=true;
bool tweaked=false;

float WAVETEX_shadowalpha=1.00f;
float WAVETEX_camerahowfar=30.0f;
float WAVETEX_cameraangle=0.0f;
bool sunoverhead=true;
bool boardshadow=true;

void WAVETEX_SetShadowScale( float s )
{
	WAVETEX_camerahowfar=15.0f * s / ((float)shadowtexw/64.0f);
}


nglVector WAVETEX_ProjectionSun(0, 200, 0, 1);

nglVector &WAVETEX_GetSunPos(void)
{
	if ( WavetexDebug.UseTextureSun )
		return WAVETEX_SunPos;
	return WAVETEX_ProjectionSun;
}

matrix4x4 WAVETEX_suntoent;
matrix4x4 WAVETEX_suntobrd;
matrix4x4 WAVETEX_suntolit;

int WavetexDebug_ShadowPass=1;
#ifdef TARGET_GC
bool updateshadowsun=false;
#else
bool updateshadowsun=true;
#endif
#if defined(TARGET_XBOX) || defined(TARGET_GC)
#ifdef TARGET_GC
static float WAVETEX_ShadowCX = 5;	// big enough to cover the surfer and his board from any angle
#else
static float WAVETEX_ShadowCX = 3;	// big enough to cover the surfer and his board from any angle
#endif
static const float &WAVETEX_ShadowCY = WAVETEX_ShadowCX;	// maintain correct aspect ratio
static float WAVETEX_ShadowNearZ = 0;	// z range needs to cover min / max distance of surfer from the sun
static float WAVETEX_ShadowFarZ = 100000;
float WAVETEX_renderscale = 1;
float WAVETEX_lightscale = WAVETEX_ShadowCX;
#else
// increasing this makes the shadow smaller and fuzzier but less likely
// to go off the edge
float WAVETEX_renderscale=0.40f;
float WAVETEX_lightscale=2.00f / WAVETEX_renderscale;
#endif
nglVector WAVETEX_projScale;


void WAVETEX_UpdateSunCamPos( void );

const float minvisiblealpha=1.0f/128.0f;

bool WAVETEX_ProjectThisLight( light_source *lp )
{
	#ifdef EVAN
		//return FALSE;
	#endif

	int current_state = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_current_state();
	int super_state = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_super_state();

	if (super_state == SUPER_STATE_WIPEOUT || current_state == STATE_DUCKDIVE ||
		current_state == STATE_SWIMTOLIE || current_state == STATE_FLYBY )
		return false;

	if (g_game_ptr->is_splitscreen())
		return false;

	if (WAVETEX_shadowalpha < minvisiblealpha )
		return false;
  if ( (!WavetexDebug.ShadowLights) || lp->get_properties().get_flavor()==LIGHT_FLAVOR_POINT)
		return false;
	if ( updateshadowsun )
		WAVETEX_UpdateSunCamPos();
	return WavetexDebug_ShadowPass;
}

int wavetex_blend = NGLBM_BLEND;

nglVector SunColor(0.5, 0.5, 0.5, 0.5);

void WAVETEX_ProjectLight( light_source *lp )
{
	if ( WAVETEX_ProjectThisLight(lp) )
	{
	#if 1 //ndef TARGET_GC
		WAVETEX_projScale[0] = WAVETEX_lightscale;
		WAVETEX_projScale[1] = WAVETEX_lightscale;
		WAVETEX_projScale[2] = WAVETEX_lightscale;
		WAVETEX_projScale[3] = 1.0f;
		nglTexture *texttex = WAVETEX_GetShadowTex();
		nglListAddDirProjectorLight( NGLMESH_LIGHTCAT_8, *(nglMatrix*) &WAVETEX_suntolit, WAVETEX_projScale,
			wavetex_blend, 0, texttex );
	#endif /* TARGET_XBOX JIV DEBUG */
	}

}



float shadowfov=20;
bool setwtov=true;
bool setpmat=false;
bool setomat=true;
bool shadowwtf2=false;
bool shadowwtf=true;
bool presetfov=true;
bool presetsize=true;
bool presetsize32=true;
int perspsw=shadowtexw/2;
int perspsh=shadowtexh/2;
#ifdef TARGET_GC
vector3d WAVETEX_ProjectShadowOff = vector3d(0, 10000,0);
#else
vector3d WAVETEX_ProjectShadowOff = vector3d(0,-1000,0);
#endif
vector3d WAVETEX_up=vector3d(0,0,-1);


#if 0
#define get_obj_pos get_abs_position
#else
#define get_obj_pos get_handed_abs_position
#endif

void WAVETEX_UpdateSunCamPos( void )
{
	//nglVector &SunPos=WAVETEX_GetSunPos();
	entity* ent;
	vector3d look_at;
	vector3d from;

	ent= ((conglomerate *)g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->GetBoardModel())->get_member("BOARD");
	look_at=ent->get_obj_pos();
	from=ent->get_obj_pos(); //vector3d(WAVETEX_SunPos[0],WAVETEX_SunPos[1],WAVETEX_SunPos[2]);
	from += WAVETEX_ProjectShadowOff;


	ent = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_owner();
	vector3d look_at2;
	look_at2=ent->get_obj_pos();
	look_at=(look_at + look_at2 ) / 2.0f;
	from=ent->get_obj_pos(); //vector3d(WAVETEX_SunPos[0],WAVETEX_SunPos[1],WAVETEX_SunPos[2]);
	from += WAVETEX_ProjectShadowOff;
	WAVETEX_suntoent[3][0]=from[0];
	WAVETEX_suntoent[3][1]=from[1];
	WAVETEX_suntoent[3][2]=from[2];
	WAVETEX_suntoent[3][3]=1.0f;
	geometry_manager::inst()->set_look_at( &WAVETEX_suntoent, from, look_at, WAVETEX_up );

	ent= ((conglomerate *)g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->GetBoardModel())->get_member("BOARD");
	look_at=ent->get_obj_pos();
	from=ent->get_obj_pos(); //vector3d(WAVETEX_SunPos[0],WAVETEX_SunPos[1],WAVETEX_SunPos[2]);
	from += WAVETEX_ProjectShadowOff;
	WAVETEX_suntobrd[3][0]=from[0];
	WAVETEX_suntobrd[3][1]=from[1];
	WAVETEX_suntobrd[3][2]=from[2];
	WAVETEX_suntobrd[3][3]=1.0f;
	geometry_manager::inst()->set_look_at( &WAVETEX_suntobrd, from, look_at, WAVETEX_up );

	if (presetfov)
	{
  	shadowfov=proj_field_of_view_in_degrees();
	}
	if ( presetsize )
	{
		if ( presetsize32 )
		{
			perspsw=shadowtexw/2;
			perspsh=shadowtexh/2;
		}
		else
		{
			perspsw=nglGetScreenWidth()/2;
			perspsh=nglGetScreenHeight()/2;
		}
	}

	memcpy(&WAVETEX_suntolit,&WAVETEX_suntobrd,sizeof(WAVETEX_suntobrd));
	if ( shadowwtf )
	{
		WAVETEX_suntolit[3][0]=from[0];
		WAVETEX_suntolit[3][1]=from[1];
		WAVETEX_suntolit[3][2]=from[2];
	}

}

const float blurfactor=0.05;
//float BlurPassXLo[4]={0.0f, blurfactor/2.0f, blurfactor, blurfactor/2.0f};
//float BlurPassXHi[4]={shadowtexw-0.0f, shadowtexw-blurfactor/2.0f, shadowtexw-blurfactor, shadowtexw-blurfactor/2.0f};
//float BlurPassYLo[4]={0.0f, blurfactor/2.0f, blurfactor, blurfactor/2.0f};
//float BlurPassYHi[4]={shadowtexh-0.0f, shadowtexh-blurfactor/2.0f, shadowtexh-blurfactor, shadowtexh-blurfactor/2.0f};
float BlurPassXLo[4]={1, 3, 5, 3};
float BlurPassXHi[4]={58, 60, 62, 60};
float BlurPassYLo[4]={3, 5, 3, 1};
float BlurPassYHi[4]={60, 62, 60, 58};

#ifdef THE_COMPILER_DOES_NOT_CRASH_ANYMORE
#include "shadow.cpp"
#endif

float clear_r=0.5f;
float clear_g=0.5f;
float clear_b=0.5f;
float clear_a=1.0f;

#ifdef TARGET_XBOX
		bool wavetex_fullalphashadow=true;
		float wavetex_fullalphavalue=1.f;
		// 0 = dark shadow, 1 = player reflection
		float shadow_reflective_value=0.f;
#else
	#ifndef TARGET_GC
		bool wavetex_fullalphashadow=false;
		float wavetex_fullalphavalue=0.5f;
		// 0 = dark shadow, 1 = player reflection
		float shadow_reflective_value=0.0f;
	#else
		#if 0 //def EVAN
			bool wavetex_fullalphashadow=true;
			float wavetex_fullalphavalue=1.00f;
			// 0 = dark shadow, 1 = player reflection
			float shadow_reflective_value=0.25f;
		#else
			bool wavetex_fullalphashadow=false;
			float wavetex_fullalphavalue=0.50f;
			// 0 = dark shadow, 1 = player reflection
			float shadow_reflective_value=0.00f;
		#endif
	#endif
#endif

bool wavetex_saveshadowtex=false;
bool g_draw_wave_quad=false;

void llc_flush_to_current_scene( void );

void WAVETEX_ClearShadows( void )
{
	if ( ShadowSourceTexture )
	{
	  nglListBeginScene();
		nglSetClearColor( 0.0, 0.0, 0.0, 0.0 );
	  nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z  | NGLCLEAR_STENCIL );
		nglSetRenderTarget(ShadowSourceTexture,true);
		nglSetViewport( 0,0,shadowtexw-1,shadowtexh-1 );
	  nglListEndScene();
	}
}

void WAVETEX_RenderShadows( camera *cam )
{
#ifndef TARGET_GC
  if ( g_draw_wave_quad )
  {
    nglQuad q;
    nglInitQuad( &q );
    nglSetQuadZ( &q, 0.1f );
    nglSetQuadColor(&q, NGL_RGBA32 (255, 255, 255, 255));
		nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAT_TEXTURE_MAP |  NGLMAP_BILINEAR_FILTER);
    nglSetQuadTex( &q, WaveTexAnimLight );  // this comes out right every time
    nglSetQuadPos( &q, 20, nglGetScreenHeight( ) - 150 );
    nglListAddQuad( &q );
  }

	if ( wavetex_saveshadowtex )
	{
		bool memlocked=mem_malloc_locked();
		mem_lock_malloc(false);
		nglSaveTexture(ShadowSourceTexture,"shadowtex");
		mem_lock_malloc(memlocked);

		wavetex_saveshadowtex=false;
	}
#endif
	// Draw shadow texture

	if ( ShadowSourceTexture && WavetexDebug_ShadowPass )
	{
		#if 1
		WAVETEX_shadowalpha= wavetex_fullalphavalue;

		float tdist=g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->Closest_Tube_Distance() - 1;
		if ( tdist<0.0f)
			WAVETEX_shadowalpha+=(tdist*2.0f*wavetex_fullalphavalue);

		if (wavetex_fullalphashadow)
			WAVETEX_shadowalpha=wavetex_fullalphavalue;

		if (WAVETEX_shadowalpha < minvisiblealpha )
			return;
		#endif
		if ( !updateshadowsun )
			WAVETEX_UpdateSunCamPos();

	  nglListBeginScene();
		nglSetClearColor( 0.0, 0.0, 0.0, 0.0 );
	  nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z  | NGLCLEAR_STENCIL );
		nglSetRenderTarget(ShadowSourceTexture,true);
		nglSetViewport( 0,0,shadowtexw-1,shadowtexh-1 );
	  nglListEndScene();

		if ( WavetexDebug.ShadowPass )
		{
			nglListBeginScene();
			if ( WavetexDebug.ShadowTrans )
				nglSetClearColor( 0.0, 0.0, 0.0, 0.0 );
			else
				nglSetClearColor( clear_r, clear_g, clear_b, clear_a );
			nglSetClearZ( 1 );
			nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_STENCIL | NGLCLEAR_Z );
			nglSetRenderTarget(ShadowSourceTexture,true); //false);
			nglSetViewport( 1,1,shadowtexw-2,shadowtexh-2 );
#ifdef TARGET_XBOX
			ksnglSetOrthoMatrix(WAVETEX_ShadowCX, WAVETEX_ShadowCY, WAVETEX_ShadowNearZ, WAVETEX_ShadowFarZ);
#else
#ifdef TARGET_GC // 0 //NGL > 0x010700
			nglSetOrthoMatrix(-(WAVETEX_ShadowCY)/2.0f, (WAVETEX_ShadowCY)/2.0f, -(WAVETEX_ShadowCX/2.0f), (WAVETEX_ShadowCX/2.0f), WAVETEX_ShadowNearZ, WAVETEX_ShadowFarZ);
			//nglSetOrthoMatrix(-shadowtexw/2,-shadowtexh/2,shadowtexw/2,shadowtexh/2,0.2f,65536.0f);
			//ksnglSetOrthoMatrix(shadowtexw / 2, shadowtexh / 2, 0.2f, 65536.0f);
#else
			ksnglSetOrthoMatrix(shadowtexw / 2, shadowtexh / 2, 0.2f, 65536.0f);
#endif
#endif

			entity* ent;
			nglSetWorldToViewMatrix( *(nglMatrix*) &WAVETEX_suntoent );
			render_flavor_t flavor=RENDER_OPAQUE_PORTION | RENDER_TRANSLUCENT_PORTION;
			if (WavetexDebug.ShadowLoRes)
			{
				flavor |= RENDER_SHADOW_MODEL;
				//flavor |= RENDER_LORES_MODEL;
			}

			ent = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_owner();
			ent->rendershadow(cam, 1.0f, flavor, WAVETEX_shadowalpha, WAVETEX_renderscale);

			if ( boardshadow )
			{
				#ifndef TARGET_GC

				nglListEndScene();
				nglListBeginScene();

				#endif
				nglSetRenderTarget(ShadowSourceTexture,true); //false);
#ifdef TARGET_XBOX
				nglSetOrthoMatrix(WAVETEX_ShadowCX, WAVETEX_ShadowCY, WAVETEX_ShadowNearZ, WAVETEX_ShadowFarZ);
#else
#ifdef TARGET_GC // 0 //NGL > 0x010700
				nglSetOrthoMatrix(-(WAVETEX_ShadowCY)/2.0f, (WAVETEX_ShadowCY)/2.0f, -(WAVETEX_ShadowCX/2.0f), (WAVETEX_ShadowCX/2.0f), WAVETEX_ShadowNearZ, WAVETEX_ShadowFarZ);
				//ksnglSetOrthoMatrix(shadowtexw , shadowtexh , 0.2f, 65536.0f);
				//nglSetOrthoMatrix(-shadowtexw/2,-shadowtexh/2,shadowtexw/2,shadowtexh/2,0.2f,65536.0f);
				//ksnglSetOrthoMatrix(shadowtexw / 2, shadowtexh / 2, 0.2f, 65536.0f);
#else
				ksnglSetOrthoMatrix(shadowtexw / 2, shadowtexh / 2, 0.2f, 65536.0f);
#endif
#endif
		  	nglSetWorldToViewMatrix( *(nglMatrix*) &WAVETEX_suntobrd );
				nglSetViewport( 1,1,shadowtexw-2,shadowtexh-2 );
				ent= ((conglomerate *)g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->GetBoardModel())->get_member("BOARD");	// WAVETEX_RenderShadows(): Currently only draws player0's shadow (multiplayer fixme?)
				ent->rendershadow(cam, 1.0f, flavor, WAVETEX_shadowalpha, WAVETEX_renderscale);
			}
			nglListEndScene();
		}
	}

  if ( WavetexDebug.DrawShadowQuad )
  {
    nglQuad q;
    nglInitQuad( &q );
    nglSetQuadZ( &q, 0.0f );
    nglSetQuadColor(&q, NGL_RGBA32 (255, 255, 255, 255));
		nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
    nglSetQuadTex( &q, ShadowSourceTexture );  // this comes out right every time
    nglSetQuadPos( &q, 20, nglGetScreenHeight( ) - 150 );
		//nglSetQuadBlend(&q, NGLBM_OPAQUE, 0);
		nglSetQuadBlend(&q, NGLBM_BLEND, 0);
    nglListAddQuad( &q );
  }



#ifdef THE_COMPILER_DOES_NOT_CRASH_ANYMORE
	// Distort it to look like it's on water

	if ( ShadowSourceTexture && ShadowTargetTexture && WavetexDebug.ShadowDistort )
	{
		nglListBeginScene();
		nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_STENCIL );
		if ( WavetexDebug.ShadowTrans )
			nglSetClearColor( 0.0, 0.0, 0.0, 0.0 );
		else
			nglSetClearColor( clear_r, clear_g, clear_b, clear_a );
		nglSetRenderTarget(ShadowTargetTexture,true); //false);
		nglSetOrthoMatrix(shadowtexw / 2, shadowtexh / 2, 0.2f, 65536.0f);
		nglMatrix Identity;
		nglIdentityMatrix(Identity);
		nglSetWorldToViewMatrix(Identity);


		int twiddleframe;
		twiddleframe = ((int)(WAVE_TexAnimFrame/4.0f))%25;
		if ( WavetexDebug.Autowiggle )
		{
			static int wiggleframe=0;
			twiddleframe = ((int)(wiggleframe++ / 4))%25;
		}


		if ( twiddleframe>12 )
		{
			twiddleframe=24-twiddleframe;
		}
		assert(twiddleframe>=0 && twiddleframe<=12);

		nglQuad tquad;
		nglInitQuad(&tquad);
    nglSetQuadColor( &tquad, NGL_RGBA32( 128,128,128,256 ) );
    nglSetQuadZ( &tquad, 0.3f );
		tquad.BlendMode=NGLBM_OPAQUE; //BLEND;
		tquad.Tex=ShadowSourceTexture;

		UVArray &aframe=uvanims[twiddleframe%25];

		for ( int y=0; y<16; y++ )
		{
			UVRow &arow=aframe[y];
			UVRow &arow1=aframe[y+1];
			for ( int x=0; x<16; x++ )
			{
				UVPair &apair00 = arow[x];
				UVPair &apair01 = arow[x+1];
				UVPair &apair10 = arow1[x];
				UVPair &apair11 = arow1[x+1];

		    nglSetQuadRect( &tquad, ((float) x) * (shadowtexw/16.0f), ((float) y) * (shadowtexh/16.0f), ((float) x+1.0f) * (shadowtexw/16.0f), ((float) y+1.0f) * (shadowtexh/16.0f) );
		    //nglSetQuadUV( &tquad, u0,v0,u1,v1 );
		    nglSetQuadVUV( &tquad, 0, apair00[0], apair00[1] );
		    nglSetQuadVUV( &tquad, 1, apair01[0], apair01[1] );
		    nglSetQuadVUV( &tquad, 2, apair10[0], apair10[1] );
		    nglSetQuadVUV( &tquad, 3, apair11[0], apair11[1] );
				//tquad.MaterialFlags=0;
				nglListAddQuad( &tquad );

			}
		}

		nglListEndScene();
	}
#endif

	// Blur it

	if ( ShadowBlurryTexture && ShadowTargetTexture && WavetexDebug.ShadowBlur )
	{
	  nglListBeginScene();
		nglSetClearColor( 0.0, 0.0, 0.0, 0.0 );
	  nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL );
		nglSetRenderTarget(ShadowBlurryTexture,true);
		nglSetViewport( 0,0,shadowtexw-1,shadowtexh-1 );
	  nglListEndScene();

		nglTexture *BlurSource=ShadowSourceTexture;
		if ( ShadowSourceTexture && ShadowTargetTexture && WavetexDebug.ShadowDistort )
			BlurSource=ShadowTargetTexture;

		nglListBeginScene();
		if ( WavetexDebug.ShadowTrans )
			nglSetClearColor( 0.0, 0.0, 0.0, 0.0 );
		else
			nglSetClearColor( clear_r, clear_g, clear_b, clear_a );
		nglSetRenderTarget(ShadowBlurryTexture,true); //false);
#if 0 //NGL > 0x010700
		nglSetOrthoMatrix(0.2f, 65536.0f);
#else
		ksnglSetOrthoMatrix(shadowtexw / 2, shadowtexh / 2, 0.2f, 65536.0f);
#endif
		nglMatrix Identity;
		nglIdentityMatrix(Identity);
		nglSetWorldToViewMatrix(Identity);

		nglQuad tquad;
		nglInitQuad(&tquad);

		nglSetQuadColor( &tquad, NGL_RGBA32( 128,128,128,64) );
		nglSetQuadUV( &tquad, 0.0f, 0.0f, 1.0f, 1.0f );
	  nglSetQuadZ( &tquad, 0.3f );
		tquad.BlendMode=NGLBM_BLEND;
		tquad.Tex=BlurSource;
		nglSetQuadRect( &tquad, BlurPassXLo[0], BlurPassYLo[0], BlurPassXHi[0], BlurPassYHi[0] );
		nglListAddQuad( &tquad );
		nglSetQuadRect( &tquad, BlurPassXLo[1], BlurPassYLo[1], BlurPassXHi[1], BlurPassYHi[1] );
		nglListAddQuad( &tquad );
		nglSetQuadRect( &tquad, BlurPassXLo[2], BlurPassYLo[2], BlurPassXHi[2], BlurPassYHi[2] );
		nglListAddQuad( &tquad );
		nglSetQuadRect( &tquad, BlurPassXLo[3], BlurPassYLo[3], BlurPassXHi[3], BlurPassYHi[3] );
		nglListAddQuad( &tquad );

		nglListEndScene();
	}
}

#ifdef TARGET_GC
int whattex=-1;
#endif

nglTexture *WAVETEX_GetTextureAnim( int textype )
{
#ifdef TARGET_GC
	return NULL;
#endif

#ifdef TARGET_XBOX
	switch (textype)
	{
	case WAVETEX_TEXFOAM:
		return nglGetTexture(WAVETEX_FoamMapName);
	case WAVETEX_TEXDARK:
	case WAVETEX_TEXHIGH:
	case WAVETEX_TEXSPOT:
	case WAVETEX_TEXLITE:
		return nglGetTexture(WAVETEX_BumpMapName);
	default:
		assertmsg(0, "Bad water texture type.");
		return NULL;
	}
#endif

#ifdef TARGET_PS2
	switch (textype)
	{
	case WAVETEX_TEXDARK:
		return WaveTexAnimDark;
		break;
	case WAVETEX_TEXHIGH:
		return WaveTexAnimHighlight;
		break;
	case WAVETEX_TEXSPOT:
		return WaveTexAnimSpotlight;
		break;
	case WAVETEX_TEXFOAM:
		return WaveTexAnimFoam;
		break;
	case WAVETEX_TEXLITE:
	default:
		return WaveTexAnimLight;
		break;
	}
#endif
}

nglTexture *WAVETEX_GetTexture( int textype, int frame )
{
	if ( frame<0 )
	{
		frame=(int) WAVE_TexAnimFrame;
	}

#ifdef TARGET_GC
  if( textype == 0 )
    return WaveTexAnimLight.get_frame( frame );
  else if( textype == WAVETEX_TEXBUMP2 )
    return BumpTexture2.get_frame( frame );
	else if( textype==WAVETEX_TEXBUMP )
		return BumpTexture;
	else if( textype==WAVETEX_TEXENVM )
		return EnvTexture;
	else if( textype==WAVETEX_TEXFOAM )
		return WaveTexAnimFoam.get_frame( frame );
	else
		return NULL;
#else


#ifdef TARGET_PS2
	if ( textype==WAVETEX_TEXHIGH || textype==WAVETEX_TEXSPOT )
	{
			// detect half speed specular animations
		if ( WaveTexAnimDark && WaveTexAnimHighlight &&
		     WaveTexAnimDark->NFrames > WaveTexAnimHighlight->NFrames )
		{
			frame = frame / 2;
		}
	}
#endif

	nglTexture *anim=WAVETEX_GetTextureAnim(textype);
	if ( anim==NULL && textype==WAVETEX_TEXFOAM )
	{
		return FoamTexture;
	}

	if ( anim==NULL )
	{
		return NULL;
	}


	if ( anim && frame < (int) anim->NFrames)
		return anim->Frames[frame];
	else if ( anim && anim->Type == NGLTEX_IFL )
	{
		if ( anim->NFrames )
		{
			assert(anim->Frames[frame%anim->NFrames]);
			return anim->Frames[frame%anim->NFrames];
		}
	}

#endif //TARGET_GC
	return NULL;
}

#ifndef TARGET_XBOX

u_int WAVETEX_AverageColor( int textype, unsigned int frame )
{
#ifdef TARGET_GC
	return 0x80808080;
#else
	u_int i, j;
	const u_int istep = 16, jstep = 16;
	u_int t;
	float avgr = 0, avgg = 0, avgb = 0, avga = 0, avgcount = 0;

		nglTexture *tex = WAVETEX_GetTexture(textype, frame);

		if ( tex==NULL ) return 0x80808080;

		#if defined(TARGET_XBOX)
		  // JIV FIXME
		  bool locked = nglLockTextureXB(tex);
		  assert(locked);
		#endif /* TARGET_XBOX JIV DEBUG */

		if (tex)
		for (i = 0; i < tex->Width; i += istep)
		{
			for (j = 0; j < tex->Height; j += jstep)
			{
				t = KSNGL_GetTexel(tex, i, j);
				avgr += t & 0x000000ff;
				avgg += (t & 0x0000ff00) >> 8;
				avgb += (t & 0x00ff0000) >> 16;
				avga += (t & 0xff000000) >> 24;
				++avgcount;
			}
		}

		#if defined(TARGET_XBOX)
		  nglUnlockTextureXB(tex);
		#endif /* TARGET_XBOX JIV DEBUG */

		avgr /= avgcount;
		avgg /= avgcount;
		avgb /= avgcount;
		avga /= avgcount;

		assert(avga >= 0 && avga < 256);
		assert(avgb >= 0 && avgb < 256);
		assert(avgg >= 0 && avgg < 256);
		assert(avgr >= 0 && avgr < 256);

#ifdef TARGET_PS2
		avgr /= 2;	// vertex colors should range from 0 to 128 on PS2 (dc 06/19/02)
		avgg /= 2;
		avgb /= 2;
#endif

		return (((u_int) FTOI(avga)) << 24) + (((u_int) FTOI(avgb)) << 16) + (((u_int) FTOI(avgg)) << 8) + ((u_int) FTOI(avgr));
#endif
}
#endif

u_int WAVETEX_Width( int textype, int frame )
{
	#ifdef TARGET_GC
	return 128;
	#endif
  if( frame < 0 ) frame = (int) WAVE_TexAnimFrame / 2;
	nglTexture *tex = WAVETEX_GetTexture(textype, frame);
	if ( tex ) return tex->Width;
	return 1;
}

u_int WAVETEX_Height( int textype, int frame )
{
	#ifdef TARGET_GC
	return 128;
	#endif
  if( frame < 0 ) frame = (int) WAVE_TexAnimFrame / 2;
	nglTexture *tex = WAVETEX_GetTexture(textype, frame);
	if ( tex ) return tex->Height;
	return 1;
}

float WAVETEX_FrameFix( int textype, float frame )
{
#ifdef TARGET_GC
  int asize;
	if( textype == 0 )
    asize = WaveTexAnimLight.num_frames * 2;
  else if( textype == WAVETEX_TEXBUMP2 )
    asize = BumpTexture2.num_frames * 2;
	else
		return 0.0f;

	while ( frame>=asize )
	  frame -= asize;
	return frame;
#else

  nglTexture *anim=WAVETEX_GetTextureAnim(textype);
	if ( anim==NULL )
	{
		return 0.0f;
	}
  int asize = 0;

	// this should work for either type of animated texture on any platform (EO 3/3/2)

#ifndef TARGET_XBOX
	if ( anim->Type == NGLTEX_ATE )
	{
		asize=anim->NFrames;
	}
	else
#endif
	if ( anim->Type == NGLTEX_IFL )
	{
		asize=anim->NFrames;
	}
	if ( asize <= 0)
		return 0.0f;
	asize*=2;
	while ( frame>=asize )
	{
		frame -= asize;
	}
	return frame;
#endif //TARGET_GC
}

//static int envblendval=32;

void WAVETEX_InitMaterial( nglMaterial &Mat, int textype, int passtype, int mattype )
{
	if ( passtype==WAVETEX_PASSFOAM )
	{
		assert(0);
		memset( &Mat, 0, sizeof(Mat) );
		Mat.Flags = 0;
		if (WavetexDebug.UseLights)
			Mat.Flags |= NGLMAT_LIGHT;
		Mat.Map = FoamTexture;
		Mat.DetailMap =
		Mat.EnvironmentMap =
		Mat.LightMap = NULL;
		if (WavetexDebug.BackfaceCull)
			Mat.Flags |= NGLMAT_BACKFACE_CULL;
		if (WavetexDebug.BilinearFilter)
			Mat.Flags |= NGLMAT_BILINEAR_FILTER;
		if (WavetexDebug.TrilinearFilter)
			Mat.Flags |= NGLMAT_TRILINEAR_FILTER;
		if ( WavetexDebug.DrawLightPass && Mat.Map )
			Mat.Flags |= NGLMAT_TEXTURE_MAP;
		Mat.MapBlendMode = NGLBM_ADDITIVE;
		Mat.Flags |= NGLMAT_ALPHA;
		Mat.Flags |= NGLMAT_LIGHT;
		//Mat.Flags |= NGLMAT_ALPHA_SORT_FIRST;
		return;
	}
	else if ( passtype==WAVETEX_PASSFULL )
	{
		memset( &Mat, 0, sizeof(Mat) );
		Mat.Flags = 0;
		//if (WavetexDebug.UseLights)
			Mat.Flags |= NGLMAT_LIGHT;
		if (WavetexDebug.DrawTexture)
		{
			int aframe=(int) WAVE_TexAnimFrame / 2;
			if (!WavetexDebug.DrawTextureAnim)
				aframe=0;

			if (WavetexDebug.MultiTexture)
			{
				#ifndef TARGET_GC
		 		if ( WAVETEX_CameraUnderwater() )
				{
					Mat.Map = WAVETEX_GetTexture( WAVETEX_TEXDARK, aframe );
					Mat.DetailMap = WAVETEX_GetTexture( WAVETEX_TEXLITE, aframe );
				}
				else
				#endif
				{
					Mat.Map = WAVETEX_GetTexture( WAVETEX_TEXLITE, aframe );
					Mat.DetailMap = WAVETEX_GetTexture( WAVETEX_TEXDARK, aframe );
				}
				Mat.EnvironmentMap = WAVETEX_GetTexture( WAVETEX_TEXSPOT, aframe );
				Mat.LightMap = WAVETEX_GetTexture( WAVETEX_TEXHIGH, aframe );
#ifdef ENABLE_FOAM_PASS
				Mat.Pad0 = (u_int) WAVETEX_GetTexture( WAVETEX_TEXFOAM, aframe );
#endif
			}
			else
			{
				Mat.Map =
				Mat.DetailMap =
				Mat.EnvironmentMap =
				Mat.LightMap = WAVETEX_GetTexture( WAVETEX_TEXLITE, aframe );
#ifdef ENABLE_FOAM_PASS
				Mat.Pad0 = (u_int) WAVETEX_GetTexture( WAVETEX_TEXLITE, aframe );
#endif
			}
		}
		else
		{
			Mat.Map = NULL;
			Mat.EnvironmentMap = NULL;
			Mat.DetailMap = NULL;
			Mat.LightMap = NULL;
#ifdef ENABLE_FOAM_PASS
			Mat.Pad0 = (u_int) NULL;
#endif
		}
#if defined(TARGET_PS2)
		// Bilinear filter setting for .tim2
		if (Mat.Map && Mat.Map->Type == NGLTEX_TIM2)
		{
			((sceGsTex1 &) Mat.Map->ph->GsTex1).MMAG = WavetexDebug.BilinearFilter;
			((sceGsTex1 &) Mat.Map->ph->GsTex1).MMIN = WavetexDebug.BilinearFilter;
		}
		else if (Mat.Map && Mat.Map->Type == NGLTEX_ATE)
		{
			//((sceGsTex1 &) Mat.Map->ph->GsTex1).MMAG = WavetexDebug.BilinearFilter;
			//((sceGsTex1 &) Mat.Map->ph->GsTex1).MMIN = WavetexDebug.BilinearFilter;
		}
#endif /* TARGET_XBOX JIV DEBUG */
		if (WavetexDebug.BackfaceCull)
			Mat.Flags |= NGLMAT_BACKFACE_CULL;
		// Bilinear filter setting for .tga
		if (WavetexDebug.BilinearFilter)
			Mat.Flags |= NGLMAT_BILINEAR_FILTER;
		if (WavetexDebug.TrilinearFilter)
			Mat.Flags |= NGLMAT_TRILINEAR_FILTER;

		Mat.DetailMapBlendMode = NGLBM_BLEND;
		Mat.EnvironmentMapBlendMode = NGLBM_ADDITIVE;
		Mat.LightMapBlendMode = NGLBM_ADDITIVE;
#ifdef ENABLE_FOAM_PASS
		Mat.Pad[0] = NGLBM_BLEND;
//		if ( WavetexDebug.FoamPass ) //&& Mat.Pad0 )
//			Mat.UserFlags |= KSMAT_FOAM_MAP;
#endif
		if ( WavetexDebug.DrawLightPass && Mat.Map )
			Mat.Flags |= NGLMAT_TEXTURE_MAP;
		#ifdef TARGET_GC
			//Mat.Flags |= NGLMAT_LIGHT;
			if ( WavetexDebug.BumpPass )
			{
				Mat.Flags |= NGLMAT_BUMP_MAP;
				Mat.Flags |= NGLMAT_ENVIRONMENT_MAP;
			}
			Mat.EnvironmentMap = WAVETEX_GetTexture( WAVETEX_TEXENVM );
		#else
		if ( WavetexDebug.DrawDarkPass && Mat.DetailMap )
			Mat.Flags |= NGLMAT_DETAIL_MAP;

#ifndef TARGET_XBOX
		if (!WavetexDebug.AllTranslucent)
			Mat.UserFlags |= KSMAT_WAVETRANS;
#endif

		if ( !WAVETEX_CameraUnderwater() )
		{
#ifdef TARGET_PS2
			if ( WavetexDebug.ForceZ )
				Mat.Flags |= NGLMAT_FORCE_Z_WRITE ;
#endif
			if (WavetexDebug.TransSortFirst)
				Mat.Flags |= NGLMAT_ALPHA_SORT_FIRST;
			if (WavetexDebug.Transparency)
				Mat.MapBlendMode = NGLBM_BLEND;
			else
				Mat.MapBlendMode = NGLBM_OPAQUE;
			Mat.Flags |= NGLMAT_ALPHA;
			if ( WavetexDebug.DrawSpotPass && Mat.EnvironmentMap && !WavetexDebug.AllTranslucent )
				Mat.Flags |= NGLMAT_ENVIRONMENT_MAP;
			if ( WavetexDebug.DrawHighPass && Mat.LightMap && !WavetexDebug.AllTranslucent )
				Mat.Flags |= NGLMAT_LIGHT_MAP;
		}
 		else
		{
			if (!WavetexDebug.AllTranslucent)
 				Mat.MapBlendMode = NGLBM_OPAQUE;
			else
				Mat.MapBlendMode = NGLBM_BLEND;
		}
		#endif

		return;
	}
	if ( mattype==WAVETEX_MATWAVE )
	{
		#ifdef TARGET_GC
			// subverting the meaning of this flag to indicate the main wave mesh
			Mat.Flags |= NGLMAT_ENV_SPECULAR ;
			if ( !WAVETEX_CameraUnderwater() )
			{
				Mat.Flags |= NGLMAT_BACKFACE_CULL;
			}
		#endif
	}
	else if ( mattype==WAVETEX_MATNEAR )
	{
		Mat.MapBlendMode = NGLBM_OPAQUE;
		Mat.Flags &= ~NGLMAT_ALPHA;
		#ifdef TARGET_PS2
			Mat.Flags &= ~NGLMAT_FORCE_Z_WRITE ;
			Mat.UserFlags &= ~KSMAT_FOAM_MAP;
			Mat.UserFlags &= ~KSMAT_WAVETRANS;
			Mat.Pad0 = (u_int) NULL;
			Mat.UserFlags &= ~KSMAT_WAVETRANS;
		#endif
	}
	else if ( mattype==WAVETEX_MATSEAM )
	{
		Mat.MapBlendMode = NGLBM_BLEND;
		Mat.Flags |= NGLMAT_ALPHA;
		#ifdef TARGET_PS2
			Mat.Flags &= ~NGLMAT_FORCE_Z_WRITE ;
			Mat.UserFlags &= ~KSMAT_FOAM_MAP;
			Mat.UserFlags &= ~KSMAT_WAVETRANS;
			//Mat.UserFlags |= KSMAT_WAVETRANS;
			Mat.Pad0 = (u_int) NULL;
		#endif
	}
	else if ( mattype==WAVETEX_MATFAR )
	{
		Mat.MapBlendMode = NGLBM_OPAQUE;
		Mat.Flags &= ~NGLMAT_ALPHA;
		#ifdef TARGET_PS2
			Mat.Flags &= ~NGLMAT_FORCE_Z_WRITE ;
			Mat.UserFlags &= ~KSMAT_FOAM_MAP;
			Mat.UserFlags &= ~KSMAT_WAVETRANS;
			Mat.Pad0 = (u_int) NULL;
		#endif
	}
	else if ( mattype==WAVETEX_MATHORIZON )
	{
		Mat.MapBlendMode = NGLBM_OPAQUE;
		Mat.Flags &= ~NGLMAT_ALPHA;
		#ifdef TARGET_PS2
			Mat.Flags &= ~NGLMAT_FORCE_Z_WRITE ;
			Mat.UserFlags &= ~KSMAT_FOAM_MAP;
			Mat.UserFlags &= ~KSMAT_WAVETRANS;
			Mat.Pad0 = (u_int) NULL;
		#endif
		Mat.Flags &= ~(NGLMAT_TEXTURE_MAP | NGLMAT_DETAIL_MAP |
			NGLMAT_ENVIRONMENT_MAP | NGLMAT_LIGHT_MAP);
		Mat.Map = NULL;
		Mat.DetailMap = NULL;
		Mat.EnvironmentMap = NULL;
		Mat.LightMap = NULL;
	}
}


void WAVETEX_PrepareMaterials( void )
{
	if (WavetexDebug.AllTranslucent)
	{
		WAVETEX_transval=WAVETEX_alltrans;
	}
	else
	{
		WAVETEX_transval=128;
	}

	WAVETEX_WriteMaterialParms();
	for ( int i=WAVETEX_MATWAVE; i<WAVETEX_MAT_MAX; i++ )
	{
		WAVETEX_InitMaterial( WaveTexLMat[wavetex_currentmat][i], WAVETEX_TEXLITE, WAVETEX_PASSFULL, i );
	}
}

void WAVETEX_Update(void)
{
	WAVETEX_PrepareMaterials( );
	#ifndef TARGET_GC
	WAVETEX_BufferIndex = (WAVETEX_BufferIndex + 1) % WAVETEX_NUMBUFFER;
	#else
	WAVETEX_BufferIndex = wavetex_playerid;
	#endif
}

void WAVETEX_SetMatSort( u_int matsort, int matid )
{
	SetMatSort(WaveTexLMat[wavetex_currentmat][matid],matsort);
}

void WAVETEX_SetMatZSorted( bool onOff, int matid )
{
#ifdef TARGET_PS2
	WaveTexLMat[wavetex_currentmat][matid].Flags |= NGLMAT_FORCE_Z_WRITE ;
#endif
}


static bool force_all_textured=false;

void WAVETEX_SetMatTextured( bool onOff, int matid )
{
#ifdef TARGET_PS2
	// Heuristic check whether we can skip this step.
	// Can't use NGLMAT_TEXTURE_MAP since that can get set for other reasons.  (dc 06/19/02)
	bool CurrentlyOnOff = WaveTexLMat[wavetex_currentmat][matid].Flags & NGLMAT_DETAIL_MAP;

	if (CurrentlyOnOff == onOff) return;

	if ( onOff || force_all_textured )
	{
		// Shouldn't ever happen, because we only turn off texturing on the horizon mesh,
		// and that happens last.  If this does happen, we need to call WAVETEX_InitMaterial
		// again, and then make sure this call doesn't undo any previous changes to the material.
		// (dc 06/19/02)

		// In practice, this assertion will happen if the level's ATE file
		//   is missing -EO 6/2x/02
		assert(false);
	}
	else
	{
		WaveTexLMat[wavetex_currentmat][matid].Flags &= ~(NGLMAT_TEXTURE_MAP | NGLMAT_DETAIL_MAP |
			NGLMAT_ENVIRONMENT_MAP | NGLMAT_LIGHT_MAP);
		WaveTexLMat[wavetex_currentmat][matid].UserFlags &= ~KSMAT_FOAM_MAP;
		WaveTexLMat[wavetex_currentmat][matid].Map = NULL;
		WaveTexLMat[wavetex_currentmat][matid].DetailMap = NULL;
		WaveTexLMat[wavetex_currentmat][matid].EnvironmentMap = NULL;
		WaveTexLMat[wavetex_currentmat][matid].LightMap = NULL;
		WaveTexLMat[wavetex_currentmat][matid].Pad0 = (u_int) NULL;
	}
#endif
}

void WAVETEX_SetMatBlended( bool onOff, int matid )
{
	if (onOff)
	{
		WaveTexLMat[wavetex_currentmat][matid].MapBlendMode = NGLBM_BLEND;
		WaveTexLMat[wavetex_currentmat][matid].Flags |= NGLMAT_ALPHA;
	}
	else
	{
		WaveTexLMat[wavetex_currentmat][matid].MapBlendMode = NGLBM_OPAQUE;
		WaveTexLMat[wavetex_currentmat][matid].Flags &= ~NGLMAT_ALPHA;
	}
}

void WAVETEX_SetMatPlayerShadows( bool onOff, int matid )
{
}

void WAVETEX_SetMatTranslucent( bool onOff, int matid )
{
#ifdef TARGET_PS2
	if ( onOff )
	{
		WaveTexLMat[wavetex_currentmat][matid].UserFlags |= KSMAT_WAVETRANS;
	}
	else
	{
		WaveTexLMat[wavetex_currentmat][matid].UserFlags &= ~KSMAT_WAVETRANS;
	}
#endif
}

void WAVETEX_SetMatFoamy( bool onOff, int matid )
{
#ifdef TARGET_GC
	if ( onOff & !g_game_ptr->is_splitscreen())
	{
		// subverting the meaning of this flag to indicate the main wave mesh
		WaveTexLMat[wavetex_currentmat][matid].Flags |= NGLMAT_ENV_SPECULAR ;
		if ( !WAVETEX_CameraUnderwater() )
		{
			WaveTexLMat[wavetex_currentmat][matid].Flags |= NGLMAT_BACKFACE_CULL;
		}
	}
	else
	{
		// subverting the meaning of this flag to indicate the main wave mesh
		WaveTexLMat[wavetex_currentmat][matid].Flags &= ~NGLMAT_ENV_SPECULAR ;
	}
#endif
#ifdef ENABLE_FOAM_PASS
	if ( onOff )
	{
		WaveTexLMat[wavetex_currentmat][matid].UserFlags |= KSMAT_FOAM_MAP;
	}
	else
	{
		#ifndef TARGET_XBOX
			WaveTexLMat[wavetex_currentmat][matid].UserFlags &= ~KSMAT_FOAM_MAP;
		#endif // DAJ debug
	}
#endif
}




const u_int MAXWAVEMESHES=6;

WaveMeshHandle wavemeshes[MAXWAVEMESHES];
u_int numwavemeshes = 0;

// These should be the same as in wave.cpp, but it doesn't matter, since they're going away soon (dc 08/27/01)
//static nglVector Center(0, 0, 0, 0);
//static nglVector FoamCenter(0, 1, 0, 0);
//static float Radius = 3500;	// Chosen to draw after all underwater items, but before all nearby objects.  (dc 06/10/02)

void WAVETEX_FreeWaveMesh( u_int meshid )
{
	for (u_int i = 0; i < WAVETEX_NUMBUFFER; ++i)
	{
		// need some kind of ngl call to clean up the scratchmesh
		// instead call nglUnlockAllScratchMeshes somewhere
		//nglUnlockScratchMesh( wavemeshes[meshid].baseid );
		nglDestroyMesh( wavemeshes[meshid].baseid[i] );
	}

	// this is a cheesy hack, but as long as all the meshes
	//   get cleaned up at once it'll work
	numwavemeshes--;
}

u_int WAVETEX_InitWaveMesh( u_int nvert, int mattype, void (*fillinfunc)(void) )
{
	u_int rv=numwavemeshes++;
	assert(numwavemeshes<=MAXWAVEMESHES);
	WaveMeshHandle *wmh=&wavemeshes[rv];
	//for ( int i=WAVETEX_MATWAVE; i<WAVETEX_MAT_MAX; i++ )
	//{
		WAVETEX_InitMaterial( WaveTexLMat[wavetex_currentmat][mattype], WAVETEX_TEXLITE, WAVETEX_PASSFULL, mattype );
		WaveTexLMat[wavetex_currentmat][mattype].Flags |= NGLMAT_LIGHT;
	//}
	u_int MeshFlags = 0;
	MeshFlags |= NGLMESH_LIGHTCAT_8;	// for shadow projection (dc 06/24/02)
	if (WavetexDebug.UseLights)
	{
		MeshFlags |= NGLMESH_LIGHTCAT_1;
	}
	if (WavetexDebug.ScissorWaveMesh)
	{
		MeshFlags |= NGLMESH_PERFECT_TRICLIP;
	}

 	//wmh->baseid = -1;
 	//wmh->foamid = -1;

//	if ( passes & WAVETEX_MAINPASS )
	for (u_int i = 0; i < WAVETEX_NUMBUFFER; ++i)
	{
#if defined(TARGET_XBOX)
		wmh->baseid[i] = WAVERENDER_CreateScratchMesh( nvert, fillinfunc );
#else
  		KSNGL_CreateScratchMesh( nvert, &WaveTexLMat[wavetex_currentmat][mattype], true );
#if NGL > 0x010700
  		wmh->baseid[i] = nglCloseMesh();
#else
extern nglMesh *nglScratch;
  		wmh->baseid[i] = nglScratch;
#endif
#endif
//		KSNGL_ScratchSetMaterial( &WaveTexLMat[wavetex_currentmat] );

#if NGL > 0x010700
#warning "NGLMERGE: this doesn't work anymore."
#else
		nglSetMeshFlags( MeshFlags );
#endif
	}

//	nglMeshSetSphere(Center, Radius);	// no real need for this sphere, since we will never cull this mesh
  return rv;
}

u_int lastmeshid=0;

void WAVETEX_UseWaveMesh( u_int rv, int mattype, u_int submesh )
{
	lastmeshid=rv;
	WaveMeshHandle *wmh=&wavemeshes[rv];

	//nglEditMesh(wmh->baseid[WAVETEX_BufferIndex]);
	{
		nglEditMesh(wmh->baseid[WAVETEX_BufferIndex]);
//		nglMeshSetSphere(Center, Radius);	// no real need for this sphere, since we will never cull this mesh
//#ifndef TARGET_PS2	// PS2 caches too much for us to change materials on the fly.  (dc 06/03/02)
		KSNGL_ScratchSetMaterial(&WaveTexLMat[wavetex_currentmat][mattype]);
//#endif
	}
}

void WAVETEX_CloseWaveMesh( void )
{
#if NGL > 0x010700
	nglCloseMesh();
#endif
}




bool WAVETEX_MultiMesh( void )
{
	return WavetexDebug.FoamPass && (FoamTexture!=NULL);
}

void WAVETEX_UseWaveSubMesh( u_int rv )
{
	WaveMeshHandle *wmh=&wavemeshes[lastmeshid];
	{
		nglEditMesh(wmh->baseid[WAVETEX_BufferIndex]);
	}
}

void WAVETEX_FillWaveMesh( u_int wavemeshid
#ifndef TARGET_XBOX
	, bool uselite, bool usedark, bool usehigh, bool usespot, bool useenv
#endif
)
{
}

#ifdef TARGET_XBOX
static void WAVETEX_SubmitNGLParameters(void)
{
extern void nglSetBumpScale(float BumpScale);
extern void nglSetWaterNearFarColors(XGVECTOR4 FarColor, XGVECTOR4 NearColor);
extern void nglSetWaterAlphaFalloff(float Scale, float Offset, float Min, float Max);
extern void nglSetWaterBumpFalloff(float Scale, float Offset, float Min, float Max);
extern void nglSetWaterRGBFalloff(float Scale, float Offset, float Min, float Max);
extern void nglSetWaterSpecularColor(XGVECTOR4 SpecularColor);
extern void nglSetSunDir(XGVECTOR4 SunDir);
extern void nglSetWaterSpecularFalloff(float Scale, float Offset);
	nglSetBumpScale(WavetexDebug.BumpScale);

/*
	The vertex colors from the .wave file are combined multiplicatively with the near/far color
	to produce the final vertex color.  The vertex shader code is in ngl_VertexShaders.h
	in the shader fragment called nglvsf_water_nearfar().

	We intentionally don't multiply the input vertex colors by 2, so the midrange values of these
	colors are around 0.5 .  To compensate, we multiply the near/far colors by 2 --- these are vertex
	shader constants, so they are not limited to the range [0,1].  The calculation results are the
	same as on PS2, except that the near/far colors, scale and offset are set independently for the
	two platforms

	We took this approach so that the input vertex colors could either brighten or darken the output
	color, like on PS2.  Some of the input vertex colors are > 0.5, so If we multiplied them by 2,
	they would be out of range.  The vertex shader would have capped them out at 1, so they could no
	longer have brightened the output, only darkened it.
*/
	nglSetWaterNearFarColors(
		XGVECTOR4(
			2 * WavetexDebug.ColorFarR,
			2 * WavetexDebug.ColorFarG,
			2 * WavetexDebug.ColorFarB,
			WavetexDebug.ColorFarA
		),
		XGVECTOR4(
			2 * WavetexDebug.ColorNearR,
			2 * WavetexDebug.ColorNearG,
			2 * WavetexDebug.ColorNearB,
			WavetexDebug.ColorNearA
		)
	);

	static float AlphaHeightAttenuateBegin = 10, AlphaHeightAttenuateEnd = 20;
	float AlphaHeightAttenuate = min(1, max(0, AlphaHeightAttenuateEnd - WAVE_camy) /
		(AlphaHeightAttenuateEnd - AlphaHeightAttenuateBegin));

	nglSetWaterAlphaFalloff(WavetexDebug.AlphaFalloffScale, WavetexDebug.AlphaFalloffOffset,
		WavetexDebug.AlphaFalloffMin, WavetexDebug.AlphaFalloffMax * AlphaHeightAttenuate);
	nglSetWaterBumpFalloff(WavetexDebug.BumpFalloffScale, WavetexDebug.BumpFalloffOffset,
		WavetexDebug.BumpFalloffMin, WavetexDebug.BumpFalloffMax);
	nglSetWaterRGBFalloff(WavetexDebug.RGBFalloffScale, WavetexDebug.RGBFalloffOffset,
		WavetexDebug.RGBFalloffMin, WavetexDebug.RGBFalloffMax);

	nglSetWaterSpecularColor(
		XGVECTOR4(
			2 * WavetexDebug.ColorSpecularR,
			2 * WavetexDebug.ColorSpecularG,
			2 * WavetexDebug.ColorSpecularB,
			WavetexDebug.ColorSpecularA
		)
	);

	// !!!! Assumes SunDirectionX/Y/Z ordered correctly and of correct type !!!!
	vector3d unit_sun(WavetexDebug.SunDirectionX, WavetexDebug.SunDirectionY, WavetexDebug.SunDirectionZ);
	unit_sun.normalize();
	nglSetSunDir(
		XGVECTOR4(
			unit_sun.x,
			unit_sun.y,
			unit_sun.z,
			1
		)
	);

	nglSetWaterSpecularFalloff(WavetexDebug.SpecularFalloffScale, WavetexDebug.SpecularFalloffOffset);
}
#endif

void WAVETEX_Init( void )
{
#ifdef TARGET_XBOX
	nglLoadTexture(WAVETEX_EnvironmentMapName);
	nglLoadTexture(WAVETEX_BumpMapName);
	nglLoadTexture(WAVETEX_SpecularMapName);
	nglLoadTexture(WAVETEX_FoamMapName);
	if (nglGetTexture(WAVETEX_EnvironmentMapName) && nglGetTexture(WAVETEX_BumpMapName))
	{
		nglSetIFLSpeed(15);
	}
	else
	{
		nglLoadTexture(WAVETEX_DiffuseMapName);	// not enough memory to load them both. (dc 04/20/02)
	}

	int current_beach = g_game_ptr->get_beach_id();
	WavetexDebug.BumpScale = BeachDataArray[current_beach].bumpscale;
	WavetexDebug.ColorFarA = ((BeachDataArray[current_beach].colorfar & 0xff000000) >> 24) / 255.f;
	WavetexDebug.ColorFarR = ((BeachDataArray[current_beach].colorfar & 0x00ff0000) >> 16) / 255.f;
	WavetexDebug.ColorFarG = ((BeachDataArray[current_beach].colorfar & 0x0000ff00) >> 8) / 255.f;
	WavetexDebug.ColorFarB = ((BeachDataArray[current_beach].colorfar & 0x000000ff) >> 0) / 255.f;
	WavetexDebug.ColorNearA = ((BeachDataArray[current_beach].colornear & 0xff000000) >> 24) / 255.f;
	WavetexDebug.ColorNearR = ((BeachDataArray[current_beach].colornear & 0x00ff0000) >> 16) / 255.f;
	WavetexDebug.ColorNearG = ((BeachDataArray[current_beach].colornear & 0x0000ff00) >> 8) / 255.f;
	WavetexDebug.ColorNearB = ((BeachDataArray[current_beach].colornear & 0x000000ff) >> 0) / 255.f;
	WavetexDebug.AlphaFalloffScale = BeachDataArray[current_beach].alphafadescale;
	WavetexDebug.AlphaFalloffOffset = BeachDataArray[current_beach].alphafadeoffset;
	WavetexDebug.AlphaFalloffMin = BeachDataArray[current_beach].alphafademin;
	WavetexDebug.AlphaFalloffMax = BeachDataArray[current_beach].alphafademax;
	WavetexDebug.BumpFalloffScale = BeachDataArray[current_beach].bumpfadescale;
	WavetexDebug.BumpFalloffOffset = BeachDataArray[current_beach].bumpfadeoffset;
	WavetexDebug.BumpFalloffMin = BeachDataArray[current_beach].bumpfademin;
	WavetexDebug.BumpFalloffMax = BeachDataArray[current_beach].bumpfademax;
	WavetexDebug.RGBFalloffScale = BeachDataArray[current_beach].rgbfadescale;
	WavetexDebug.RGBFalloffOffset = BeachDataArray[current_beach].rgbfadeoffset;
	WavetexDebug.RGBFalloffMin = BeachDataArray[current_beach].rgbfademin;
	WavetexDebug.RGBFalloffMax = BeachDataArray[current_beach].rgbfademax;
	WavetexDebug.ColorSpecularA = ((BeachDataArray[current_beach].colorspecular & 0xff000000) >> 24) / 255.f;
	WavetexDebug.ColorSpecularR = ((BeachDataArray[current_beach].colorspecular & 0x00ff0000) >> 16) / 255.f;
	WavetexDebug.ColorSpecularG = ((BeachDataArray[current_beach].colorspecular & 0x0000ff00) >> 8) / 255.f;
	WavetexDebug.ColorSpecularB = ((BeachDataArray[current_beach].colorspecular & 0x000000ff) >> 0) / 255.f;
	WavetexDebug.SunDirectionX = BeachDataArray[current_beach].sunx_xbox;
	WavetexDebug.SunDirectionY = BeachDataArray[current_beach].suny_xbox;
	WavetexDebug.SunDirectionZ = BeachDataArray[current_beach].sunz_xbox;
	WavetexDebug.SpecularFalloffScale = BeachDataArray[current_beach].specularfadescale;
	WavetexDebug.SpecularFalloffOffset = BeachDataArray[current_beach].specularfadeoffset;

	WAVETEX_SubmitNGLParameters();
#endif
	for ( int i=WAVETEX_MATWAVE; i<WAVETEX_MAT_MAX; i++ )
	{
		WAVETEX_InitMaterial( WaveTexLMat[0][i], WAVETEX_TEXLITE, WAVETEX_PASSFULL, i );
		WAVETEX_InitMaterial( WaveTexLMat[1][i], WAVETEX_TEXLITE, WAVETEX_PASSFULL, i );
	}
}

#ifdef TARGET_XBOX
/*
void WAVETEX_CopyMesh(u_int toid, u_int fromid)
{
	nglMesh *frommesh = (nglMesh *)fromid;
	nglMesh *tomesh = (nglMesh *)toid;
	assert(frommesh->NSections == tomesh->NSections);
	for (int i = 0; i < frommesh->NSections; ++i)
	{
		nglMeshSection *fromsection = &frommesh->Sections[i];
		nglMeshSection *tosection = &tomesh->Sections[i];

		tosection->IndexCount = fromsection->IndexCount;
		tosection->IndexOffset = fromsection->IndexOffset;
		tosection->SphereCenter[0] = fromsection->SphereCenter[0];
		tosection->SphereCenter[1] = fromsection->SphereCenter[1];
		tosection->SphereCenter[2] = fromsection->SphereCenter[2];
		tosection->SphereCenter[3] = fromsection->SphereCenter[3];
		tosection->SphereRadius = fromsection->SphereRadius;
		tosection->VB = fromsection->VB;
	}
}
*/

inline void WAVETEX_SubmitMesh(u_int rv, const nglMatrix &LocalToWorld, nglRenderParams &rp)
{
#ifdef DEBUG
	WAVETEX_SubmitNGLParameters();

	static int OldDrawNearFar = WavetexDebug.DrawNearFar;
	static int OldDrawSpecularPass = WavetexDebug.DrawSpecularPass;
	static int OldDrawFoamPass = WavetexDebug.DrawFoamPass;
	if (WavetexDebug.DrawNearFar != OldDrawNearFar || WavetexDebug.DrawSpecularPass != OldDrawSpecularPass)
	{
extern void nglClearVShaderDB();
		nglClearVShaderDB();	// force vertex shader to be recompiled
		OldDrawNearFar = WavetexDebug.DrawNearFar;
		OldDrawSpecularPass = WavetexDebug.DrawSpecularPass;
		OldDrawFoamPass = WavetexDebug.DrawFoamPass;
	}
#endif

	const WaveMeshHandle *wmh = &wavemeshes[rv];
	nglMesh *Mesh = wmh->baseid[WAVETEX_BufferIndex];
	nglEditMesh(Mesh);

	nglMaterial Material;

	// Submission 0
	memset(&Material, 0, sizeof(Material));

	if (WavetexDebug.DrawTexture)
	{
		if (WavetexDebug.DrawTextureAnim)
		{
			Material.Map = nglGetTexture(WAVETEX_DiffuseMapName);
		}
		else
		{
			Material.Map = NULL;
		}
	}
	else
	{
		Material.Map = NULL;
	}
	if (Material.Map)
	{
		Material.Flags |= NGLMAT_TEXTURE_MAP;
	}

	Material.Flags &= ~NGLMAT_DETAIL_MAP;
	Material.DetailMap = NULL;

	Material.EnvironmentMap = nglGetTexture(WAVETEX_EnvironmentMapName);
	if (Material.EnvironmentMap)
	{
		Material.Flags |= NGLMAT_ENVIRONMENT_MAP;
	}
	else
	{
		WavetexDebug.DrawBumpMap = false;	// can't have one without the other (dc 05/30/02)
	}
	Material.EnvironmentMapBlendMode = NGLBM_CONST_ADDITIVE;
	Material.EnvironmentMapBlendModeConstant = 0x80;

	Material.Flags &= ~NGLMAT_LIGHT_MAP;
	Material.LightMap = NULL;

	if (WavetexDebug.DrawBumpMap)
	{
		if (WavetexDebug.DrawTextureAnim)
		{
			Material.DetailMap = nglGetTexture(WAVETEX_BumpMapName);
		}
		else
		{
			Material.DetailMap = nglGetTexture(WAVETEX_BumpMapName)->Frames[0];
		}
		if (Material.DetailMap)
		{
			Material.Flags |= NGLMAT_BUMP_MAP;
		}
	}

	Material.Flags &= ~NGLMAT_LIGHT;
	Material.Flags |= NGLMAT_ALPHA;
	Material.Flags &= ~NGLMAT_BACKFACE_CULL;

	if (WavetexDebug.BilinearFilter)
	{
		Material.Flags |= NGLMAT_BILINEAR_FILTER;
	}
	else if (WavetexDebug.TrilinearFilter)
	{
		Material.Flags |= NGLMAT_TRILINEAR_FILTER;
	}

	if (WavetexDebug.BackfaceCull)
	{
		if (rv == 0)	// The wave mesh
		{
			#ifdef TARGET_GC
			Material.Flags |= NGLMAT_REVERSE_BACKFACECULL;
			#else
			// Other meshes are flat anyhow, and they aren't stripped consistently for backface culling.
			Material.Flags |= NGLMAT_BACKFACE_CULL;
			#endif
		}
	}

	if (WavetexDebug.DrawNearFar)
	{
		Material.Flags |= NGLMAT_WATER_NEARFAR;
	}

	Material.MapBlendMode = NGLBM_BLEND;

	nglMeshSetSection(0);
	KSNGL_ScratchSetMaterial(&Material);

	if (rv == 0)	// The wave mesh
	{
		// Submission 1
		memset(&Material, 0, sizeof(Material));

		if (WavetexDebug.DrawFoamPass)
		{
			Material.Map = nglGetTexture(WAVETEX_FoamMapName);
		}
		else
		{
			Material.Map = NULL;
		}
		if (Material.Map)
		{
			Material.Flags |= NGLMAT_TEXTURE_MAP;
		}

		if (WavetexDebug.DrawSpecularPass)
		{
			Material.EnvironmentMap = nglGetTexture(WAVETEX_SpecularMapName);
			// But we're not setting the NGLMAT_ENVIRONMENT_MAP flag
		}

		if (WavetexDebug.BilinearFilter)
		{
			Material.Flags |= NGLMAT_BILINEAR_FILTER;
		}
		else if (WavetexDebug.TrilinearFilter)
		{
			Material.Flags |= NGLMAT_TRILINEAR_FILTER;
		}

		if (WavetexDebug.BackfaceCull)
		{
			Material.Flags |= NGLMAT_BACKFACE_CULL;
		}

		if (WavetexDebug.DrawFoamPass)
		{
			Material.Flags |= NGLMAT_WATER_FOAM;
		}

		if (WavetexDebug.DrawSpecularPass)
		{
			Material.Flags |= NGLMAT_WATER_SPECULAR;
		}

		if (WavetexDebug.ShadowPass)
		{
			Material.Flags |= NGLMAT_LIGHT;	// Must be on in order for shadow projector light to work (dc 06/24/02)
		}

		Material.Flags |= NGLMAT_ALPHA;

		Material.MapBlendMode = NGLBM_BLEND;

		nglMeshSetSection(1);
		KSNGL_ScratchSetMaterial(&Material);
	}

	// Mesh submission
	WAVERENDER_SetSectionRenderer(Mesh);	// Must come after materials have been set (dc 05/28/01)
	nglListAddMesh(Mesh, LocalToWorld, &rp);
}
#else
void WAVETEX_SubmitMesh( u_int rv, const nglMatrix &LocalToWorld, nglRenderParams &rp
	, bool uselite, bool usedark, bool usehigh, bool usespot, bool useenv
)
{
	WaveMeshHandle *wmh=&wavemeshes[rv];
//	if ( wmh->baseid[WAVETEX_BufferIndex] != -1 )
	{
#if defined(TARGET_GC)
		nglListAddScratchWave( (u_int) wmh->baseid[WAVETEX_BufferIndex], LocalToWorld, &rp );
#else
		nglSetSectionRenderer(&wmh->baseid[WAVETEX_BufferIndex]->Sections[0], WAVERENDER_RenderSectionWave);
		nglListAddMesh( wmh->baseid[WAVETEX_BufferIndex], LocalToWorld, &rp);
#endif
	}
}
#endif

// wavetex debug menus
#include "wavetexmenu.cpp"

void WAVETEX_StaticInit( void )
{
	WAVETEXMENU_StaticInit();
}



