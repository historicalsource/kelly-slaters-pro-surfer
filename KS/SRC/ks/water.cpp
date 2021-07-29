#include "global.h"
#include "ngl.h"
#include "profiler.h"
#include "app.h"
#include "game.h"
#include "camera.h"
#include "oswaverender.h"	// For nglMeshFastWriteVertexPNCUVSTR
#include "water.h"
#include "wave.h"
#include "wavetex.h"		// For all WAVETEX_ calls
#if defined(TARGET_PS2)
#include "ngl_ps2_internal.h"	// For nglMeshFastWriteVertex functions only!!! (dc 12/14/01)
#endif // defined(TARGET_PS2)

#ifdef TARGET_XBOX
static inline void WATER_SubmitVertex(float x, float y, float z, float nx, float ny, float nz, u_int color,
	float u, float v)
{
	WAVERENDER_MeshFastWriteVertexPNCUVSTR(x, y, z, nx, ny, nz, color, u, v, 1, 0, 0, 0, 0, 1, 0, 1, 0);
}
#elif defined(TARGET_GC)
void nglWaveWriteVertexPNCAUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, u_int Alpha, float U, float V );
static inline void WATER_SubmitVertex( float x, float y, float z,
                                       float nx, float ny, float nz,
                                       u_int color,
                                       float u, float v )
{
	nglWaveWriteVertexPNCAUV(x, y, z, nx, ny, nz, color, 0, u, v);
}
#else
#define WATER_SubmitVertex nglMeshFastWriteVertexPNCUV
#endif

// Read-only copies of WAVE variables
static const float &WATER_ScaleU = WAVE_ScaleU;	// must match up with water
static const float &WATER_ScaleV = WAVE_ScaleV;
static const float &WATER_OffsetU = WAVE_OffsetU;
static const float &WATER_OffsetV = WAVE_OffsetV;

static const nglMatrix &WATER_LocalToWorld = WAVE_LocalToWorld;
//static const nglVector &WATER_LocalScale = WAVE_LocalScale;

//static const float &WATER_TexAnimFrame = WAVE_TexAnimFrame;
extern nglVector WAVE_SortCenter;
extern float WAVE_SortRadius;

static const u_int &WATER_AverageColor = WAVE_AverageColor;

// Local to this file
#if 1
#define WATER_MESHIDEALSTEPX 8
#define WATER_MESHIDEALSTEPZ 8
#define WATER_MESHIDEALSTEPMAXX 12	// need to leave room for ~ 4 more than ideal
#define WATER_MESHIDEALSTEPMAXZ 12
#else
#define WATER_MESHIDEALSTEPX 32
#define WATER_MESHIDEALSTEPZ 32
#define WATER_MESHIDEALSTEPMAXX 36	// need to leave room for ~ 4 more than ideal
#define WATER_MESHIDEALSTEPMAXZ 36
#endif
static float WATER_MeshGridX[WATER_MESHIDEALSTEPMAXX];
static float WATER_MeshGridZ[WATER_MESHIDEALSTEPMAXZ];
static float WATER_MeshGridU[WATER_MESHIDEALSTEPMAXX];
static float WATER_MeshGridV[WATER_MESHIDEALSTEPMAXZ];
static u_int WATER_MeshStepX;
static u_int WATER_MeshStepZ;

static float WATER_MeshMinX = -2;
static float WATER_MeshMaxX = 2;
#define WATER_MESH_WIDTH (WATER_MeshMaxX - WATER_MeshMinX)
static float WATER_MeshMinZ = -2;
static float WATER_MeshMaxZ = 2;
#define WATER_MESH_DEPTH (WATER_MeshMaxZ - WATER_MeshMinZ)

static float WATER_StandardSeamSizeX = 50.f;
static float WATER_StandardSeamSizeZ = 50.f;
static float WATER_SeamSizeX;
static float WATER_SeamSizeZ;
//static u_int WATER_SeamLoX;
//static u_int WATER_SeamHiX;
//static u_int WATER_SeamLoZ;
//static u_int WATER_SeamHiZ;

#if defined(TARGET_XBOX)
static float WATER_StandardMarginSizeX = 5.f;
static float WATER_StandardMarginSizeZ = 50.f;
#else
static float WATER_StandardMarginSizeX = 5.f;
static float WATER_StandardMarginSizeZ = 5.f;
#endif /* TARGET_XBOX JIV DEBUG */
static float WATER_MarginSizeX;
static float WATER_MarginSizeZ;

static float WATER_StandardInfinityX = 40000;
static float WATER_StandardInfinityZ = 40000;
static float WATER_InfinityX;	// These must remain with zmax, or the z-buffer values will wrap
static float WATER_InfinityZ;
static float WATER_StandardDepthCutoff = 500;	// smallest (furthest) acceptable depth value
static float WATER_DepthCutoff;

static nglMatrix WATER_LocalToScreen;
static void WATER_ScaleToWave(void);

/*
static const u_int NearMeshNumVert = (WATER_MESHIDEALSTEPX + 4) * (WATER_MESHIDEALSTEPZ + 4) * 4;	// max size
static u_int NearWaterMeshID;
// Slight overkill, need to compute the actual minimum value.
static const u_int SeamMeshNumVert = (2 * (WATER_MESHIDEALSTEPX + 4) + 2 * (WATER_MESHIDEALSTEPZ + 4)
									  + 2 * WAVE_MeshStepX + 2 * WAVE_MeshStepZ) * 3;	// max size
*/

// The stripping of the seam mesh really only helps CPU time.  The GPU time seems to be fill-rate bound.
#ifdef AGGRESSIVE_STRIPPING
// Better case: Smaller strip size in practice, but requires backfaceculling
static const u_int SeamStripSizeX = 3 * (WAVE_MeshStepX + WATER_MESHIDEALSTEPX + 2) / 2;
static const u_int SeamStripSizeZ = 3 * (WAVE_MeshStepZ + WATER_MESHIDEALSTEPZ + 2) / 2;
#else
// Best case without backface culling: 2 verts / tri
static const u_int SeamStripSizeX = 4 * ((WAVE_MeshStepX + WATER_MESHIDEALSTEPX) / 2)
	+ 3 * ((WAVE_MeshStepX + WATER_MESHIDEALSTEPX) % 2);	// 4 verts / quad + 3 verts / extra tri
static const u_int SeamStripSizeZ = 4 * ((WAVE_MeshStepZ + WATER_MESHIDEALSTEPZ) / 2)
	+ 3 * ((WAVE_MeshStepZ + WATER_MESHIDEALSTEPZ) % 2);	// 4 verts / quad + 3 verts / extra tri
#endif
static const u_int SeamMeshNumVert = 2 * SeamStripSizeX + 2 * SeamStripSizeZ;
static u_int SeamWaterMeshID;
static const u_int FarStripSizeX = 2 * WATER_MESHIDEALSTEPX + 2;
static const u_int FarStripSizeZ = 2 * WATER_MESHIDEALSTEPZ + 2;
static const u_int FarMeshNumVert = 2 * FarStripSizeX + 2 * FarStripSizeZ + 4 * 4;	// 4 strips + 4 quads
static u_int FarWaterMeshID;
static const u_int HorizonMeshNumVert = 8 * 4;
static u_int HorizonWaterMeshID;

/*
static void WATER_FillInNearMesh(void);
*/
static void WATER_FillInSeamMesh(void);
static void WATER_FillInFarMesh(void);
static void WATER_FillInHorizonMesh(void);
static void WATER_FindScreenExtents(void);

static void WATER_ComputeUniformGrid(void);
static void WATER_ComputeScaledGrid(void);

//static float WATER_TimeFrequency = .0005f;
//static float WATER_XFrequency = 0.5f;
//static float WATER_ZFrequency = 2.5f;
//static float WATER_MaxAmplitude = 0;	// .2f;	// amplitude of small waves

static float WATER_HorizonYBias = -.01f;	// -10.f;	// fix z-fighting by shifting lower

const static float &WATER_TexMinU = WAVE_TexMinU;	// tiling limit for a 128 x 128 texture
const static float &WATER_TexMaxU = WAVE_TexMaxU;
const static float &WATER_TexMinV = WAVE_TexMinV;
const static float &WATER_TexMaxV = WAVE_TexMaxV;

static float WATER_HorizonPixelsX = 30;
static float WATER_HorizonPixelsY = 30;

struct WaterDebugStruct
{
	u_int BackfaceCull : 1;
	u_int BilinearFilter : 1;
	u_int DrawNearMesh : 1;
/*
#ifndef TARGET_XBOX
	u_int DrawNearMeshDark : 1;
	u_int DrawNearMeshHigh : 1;
#endif
*/
	u_int DrawSeamMesh : 1;
#ifndef TARGET_XBOX
	u_int DrawSeamMeshDark : 1;
	u_int DrawSeamMeshHigh : 1;
#endif
	u_int DrawFarMesh : 1;
#ifndef TARGET_XBOX
	u_int DrawFarMeshDark : 1;
	u_int DrawFarMeshHigh : 1;
	u_int DrawHorizonMesh : 1;
#endif
	u_int DrawTexture : 1;
	u_int DrawTextureAnim : 1;
	u_int DrawWaterMesh : 1;
	u_int FadeFarMesh : 1;
	u_int FadeHorizonMesh : 1;
	u_int FreezeWater : 1;
	u_int RecomputeExtents : 1;
	u_int ScissorNearMesh : 1;
	u_int ScissorSeamMesh : 1;
	u_int ScissorFarMesh : 1;
	u_int ScissorHorizonMesh : 1;
	u_int ShowNearLines : 1;
	u_int ShowSeamLines : 1;
	u_int ShowFarLines : 1;
	u_int ShowHorizonLines : 1;
	u_int TranparentNear : 1;
	u_int UniformGrid : 1;
	u_int UseLights : 1;
}
WaterDebug =
{
	0, // BackfaceCull
	1, // BilinearFilter
	0, // DrawNearMesh
/*
#ifndef TARGET_XBOX
	1, // DrawNearMeshDark
	1, // DrawNearMeshHigh
#endif
*/
	1, // DrawSeamMesh
#ifndef TARGET_XBOX
	1, // DrawSeamMeshDark
	1, // DrawSeamMeshHigh
#endif
	1, // DrawFarMesh
#ifndef TARGET_XBOX
	1, // DrawFarMeshDark
	1, // DrawFarMeshHigh
#ifdef TARGET_PS2
	1, // DrawHorizonMesh
#else
	0, // DrawHorizonMesh
#endif
#endif
	1, // DrawTexture
	1, // DrawTextureAnim
	1, // DrawWaterMesh
	1, // FadeFarMesh
	0, // FadeHorizonMesh
	0, // FreezeWater
	1, // RecomputeExtents
	1, // ScissorNearMesh
	1, // ScissorSeamMesh
	1, // ScissorFarMesh
	1, // ScissorHorizonMesh
	0, // ShowNearLines
	0, // ShowSeamLines
	0, // ShowFarLines
	0, // ShowHorizonLines
	0, // TranparentNear
	1, // UniformGrid
	0, // UseLights
};

static bool WATER_InInit = false;	// hack for first call to WATER_FillInHorizonMesh

void WATER_Init(void)
{
	WATER_InInit = true;

	WAVE_Init();

#ifdef TARGET_XBOX
	// Needed for calls to WATER_FillIn****Mesh (dc 05/27/02)
	WATER_ScaleToWave();

	if (WaterDebug.UniformGrid)
	{
		WATER_ComputeUniformGrid();	// must be done even if not drawing near mesh
	}
	else
	{
		WATER_ComputeScaledGrid();	// must be done even if not drawing near mesh
	}

#endif

	SeamWaterMeshID = WAVETEX_InitWaveMesh(SeamMeshNumVert, WAVETEX_MATSEAM, WATER_FillInSeamMesh);
/*
	NearWaterMeshID = WAVETEX_InitWaveMesh(NearMeshNumVert, WAVETEX_MATNEAR, WATER_FillInNearMesh);
*/
	FarWaterMeshID = WAVETEX_InitWaveMesh(FarMeshNumVert, WAVETEX_MATFAR, WATER_FillInFarMesh);
	HorizonWaterMeshID = WAVETEX_InitWaveMesh(HorizonMeshNumVert, WAVETEX_MATHORIZON, WATER_FillInHorizonMesh);

	WATER_InInit = false;
}

void WATER_Cleanup(void)
{
	WAVE_Cleanup();

	WAVETEX_FreeWaveMesh(SeamWaterMeshID);
/*
	WAVETEX_FreeWaveMesh(NearWaterMeshID);
*/
	WAVETEX_FreeWaveMesh(FarWaterMeshID);
	WAVETEX_FreeWaveMesh(HorizonWaterMeshID);
//	nglUnlockAllScratchMeshes();
}



void WATER_Create( const int heroIdx )
{
	WAVETEX_Update();	// Must come before FillInMesh; switches between mesh buffers.  (dc 06/14/02)
						// Must be called exactly once per frame, damn it.

	if (WaterDebug.FreezeWater)
	{
		return;
	}

	UNDERWATER_CameraSelect(heroIdx);

	START_PROF_TIMER( proftimer_wave );
	WAVE_Create(heroIdx);	// must come before any water draw calls
	STOP_PROF_TIMER( proftimer_wave );

	WAVETEX_PrepareMaterials();
	//nglVector Center = {0, 0, 0, 0};
	//float Radius = 1e10;

	WATER_ScaleToWave();

	if (WaterDebug.RecomputeExtents)
	{
		WATER_FindScreenExtents();	// find an xz rect which covers the screen (or as much as practical)
	}

	if (WaterDebug.UniformGrid)
	{
		WATER_ComputeUniformGrid();	// must be done even if not drawing near mesh
	}
	else
	{
		WATER_ComputeScaledGrid();	// must be done even if not drawing near mesh
	}

	u_int MeshFlags = 0;
	if (WaterDebug.UseLights)
	{
		MeshFlags |= NGLMESH_LIGHTCAT_1;
	}
	if (WaterDebug.ScissorNearMesh)
	{
		MeshFlags |= NGLMESH_PERFECT_TRICLIP;
	}

//	WAVETEX_SetMatSort(USERSORT_NEAR);
	WAVETEX_SetMatBlended( WaterDebug.TranparentNear, WAVETEX_MATNEAR );
	WAVETEX_SetMatZSorted( false, WAVETEX_MATNEAR  );
	WAVETEX_SetMatPlayerShadows( false, WAVETEX_MATNEAR  );
	WAVETEX_SetMatFoamy( false, WAVETEX_MATNEAR  );

/*
	if (WaterDebug.DrawWaterMesh && (WaterDebug.DrawNearMesh
#ifndef TARGET_XBOX
		|| WaterDebug.DrawNearMeshDark || WaterDebug.DrawNearMeshHigh
#endif
	))	// must occur after WAVE_Create()
	{
		WAVETEX_SetMatTextured(true);	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatBlended(true);	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_UseWaveMesh(NearWaterMeshID, WAVETEX_MATNEAR);
		u_int MeshFlags = 0;
		if (WaterDebug.UseLights)
			MeshFlags |= NGLMESH_LIGHTCAT_1;
		if (WaterDebug.ScissorNearMesh)
			MeshFlags |= NGLMESH_PERFECT_TRICLIP;
		nglSetMeshFlags(MeshFlags);
		WATER_FillInNearMesh();
		nglMeshSetSphere(WAVE_SortCenter, WAVE_SortRadius);	// no real need for this sphere, since we will never cull this mesh
		WAVETEX_FillWaveMesh( NearWaterMeshID
#ifndef TARGET_XBOX
			, WaterDebug.DrawNearMesh, WaterDebug.DrawNearMeshDark, WaterDebug.DrawNearMeshHigh, WaterDebug.DrawNearMeshHigh, true
#endif
		);
	}
*/

	WAVETEX_SetMatBlended( false, WAVETEX_MATSEAM );

	START_PROF_TIMER( proftimer_water_seam );
	if (WaterDebug.DrawWaterMesh && (WaterDebug.DrawSeamMesh
#ifndef TARGET_XBOX
		|| WaterDebug.DrawSeamMeshDark || WaterDebug.DrawSeamMeshHigh
#endif
	))	// must occur after WAVE_Create() and WATER_FillInNearMesh()
	{
		// Draw the seam between the water and the wave
		WAVETEX_SetMatTranslucent(true, WAVETEX_MATSEAM);
		WAVETEX_SetMatTextured(true, WAVETEX_MATSEAM);	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatBlended(true, WAVETEX_MATSEAM);	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatZSorted( false, WAVETEX_MATSEAM  );
		WAVETEX_SetMatPlayerShadows( false, WAVETEX_MATSEAM  );
		WAVETEX_UseWaveMesh(SeamWaterMeshID, WAVETEX_MATSEAM );
		WAVETEX_SetMatFoamy( false, WAVETEX_MATSEAM  );
		u_int MeshFlags = 0;
		if (WaterDebug.UseLights)
			MeshFlags |= NGLMESH_LIGHTCAT_1;
		if (WaterDebug.ScissorSeamMesh)
			MeshFlags |= NGLMESH_PERFECT_TRICLIP;
#if NGL > 0x010700
#warning "These debug flags won't work for a while ..."
#else
		nglSetMeshFlags(MeshFlags);
#endif
		WATER_FillInSeamMesh();
		nglMeshSetSphere(WAVE_SortCenter, WAVE_SortRadius);	// no real need for this sphere, since we will never cull this mesh
		WAVETEX_FillWaveMesh( SeamWaterMeshID
#ifndef TARGET_XBOX
			, WaterDebug.DrawSeamMesh, WaterDebug.DrawSeamMeshDark, WaterDebug.DrawSeamMeshHigh, WaterDebug.DrawSeamMeshHigh, true
#endif
		);
		WAVETEX_CloseWaveMesh();
	}
	STOP_PROF_TIMER( proftimer_water_seam );


	START_PROF_TIMER( proftimer_water_far );
	if (WaterDebug.DrawWaterMesh && (WaterDebug.DrawFarMesh
#ifndef TARGET_XBOX
		|| WaterDebug.DrawFarMeshHigh || WaterDebug.DrawFarMeshDark
#endif
	))	// must occur after WAVE_Create() and WATER_FillInNearMesh()
	{
		// Draw the water surface out to the horizon.
		WAVETEX_SetMatTranslucent(false, WAVETEX_MATFAR );
		WAVETEX_SetMatTextured(true, WAVETEX_MATFAR );	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatBlended(WaterDebug.FadeFarMesh, WAVETEX_MATFAR );	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatFoamy( false, WAVETEX_MATFAR );
//		WAVETEX_SetMatSort(USERSORT_FAR );
		WAVETEX_UseWaveMesh(FarWaterMeshID, WAVETEX_MATFAR );
		u_int MeshFlags = 0;
		if (WaterDebug.UseLights)
			MeshFlags |= NGLMESH_LIGHTCAT_1;
		if (WaterDebug.ScissorFarMesh)
			MeshFlags |= NGLMESH_PERFECT_TRICLIP;
//		WAVETEX_SetMatSort(USERSORT_FAR );
		WAVETEX_SetMatZSorted( false, WAVETEX_MATSEAM  );
		WAVETEX_SetMatPlayerShadows( false, WAVETEX_MATSEAM  );
		WAVETEX_SetMatBlended(WaterDebug.FadeFarMesh, WAVETEX_MATFAR );
#if NGL > 0x010700
#warning "These debug flags won't work for a while ..."
#else
		nglSetMeshFlags(MeshFlags);
#endif
		WATER_FillInFarMesh();
		nglMeshSetSphere(WAVE_SortCenter, WAVE_SortRadius);	// no real need for this sphere, since we will never cull this mesh
		WAVETEX_FillWaveMesh( FarWaterMeshID
#ifndef TARGET_XBOX
			, WaterDebug.DrawFarMesh, WaterDebug.DrawFarMeshDark, WaterDebug.DrawFarMeshHigh, WaterDebug.DrawFarMeshHigh, true
#endif
		);
		WAVETEX_CloseWaveMesh();
	}
	STOP_PROF_TIMER( proftimer_water_far );

//	WAVETEX_SetMatSort(USERSORT_HORZ);

#ifndef TARGET_XBOX
	START_PROF_TIMER( proftimer_water_horizon );
	if ( WaterDebug.DrawWaterMesh
		&& (WaterDebug.DrawHorizonMesh)) //|| WaterDebug.DrawHorizonMeshDark || WaterDebug.DrawHorizonMeshHigh))	// must come last because it alters the material
	{
		// Draw the water surface out to the horizon.
		WAVETEX_SetMatTranslucent(false, WAVETEX_MATHORIZON );
		WAVETEX_SetMatTextured(false, WAVETEX_MATHORIZON);	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatBlended(WaterDebug.FadeHorizonMesh, WAVETEX_MATHORIZON);	// must come before WAVETEX_UseWaveMesh (dc 06/19/02)
		WAVETEX_SetMatFoamy( false, WAVETEX_MATHORIZON );
		WAVETEX_UseWaveMesh(HorizonWaterMeshID, WAVETEX_MATHORIZON);
		u_int MeshFlags = 0;
		if (WaterDebug.UseLights)
			MeshFlags |= NGLMESH_LIGHTCAT_1;
		if (WaterDebug.ScissorHorizonMesh)
			MeshFlags |= NGLMESH_PERFECT_TRICLIP;
		WAVETEX_SetMatBlended(WaterDebug.FadeHorizonMesh, WAVETEX_MATHORIZON);
		WAVETEX_SetMatTextured(FALSE, WAVETEX_MATHORIZON);
#if NGL > 0x010700
#warning "These debug flags won't work for a while ..."
#else
		nglSetMeshFlags(MeshFlags);
#endif
		WATER_FillInHorizonMesh();
		nglMeshSetSphere(WAVE_SortCenter, WAVE_SortRadius);	// no real need for this sphere, since we will never cull this mesh
		WAVETEX_FillWaveMesh( HorizonWaterMeshID
			, WaterDebug.DrawHorizonMesh, FALSE, FALSE, FALSE, false
		);
		WAVETEX_CloseWaveMesh();
	}
	STOP_PROF_TIMER( proftimer_water_horizon );
#endif

/*
#define NUM_SPLINE_POINTS 10
	float x[NUM_SPLINE_POINTS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, };
	float y[NUM_SPLINE_POINTS] = {2, 9, 7, 4, 8, 0, 1, 6, 3, 5, };
float a[NUM_SPLINE_POINTS], b[NUM_SPLINE_POINTS], c[NUM_SPLINE_POINTS], d[NUM_SPLINE_POINTS];
int n = NUM_SPLINE_POINTS;
SPLINE_ComputeCoeffs(x, y, n, a, b, c, d);
for (int i = 0; i < NUM_SPLINE_POINTS - 1; ++i)
{
float yval = SPLINE_Evaluate(x, a, b, c, d, NUM_SPLINE_POINTS, x[i+1]);
if (y[i+1] - yval > 0.0001f || y[i+1] - yval < -0.0001f)
{
	debug_print("Doh!\n");
}
}
*/
}


void WATER_ListAdd(void)
{
#ifdef TARGET_PS2
	assert (WATER_InfinityX * WATER_InfinityX + WATER_InfinityZ * WATER_InfinityZ
		< nglCurScene->FarZ * nglCurScene->FarZ);
#endif

	nglRenderParams rp;
	rp.Flags = NGLP_SCALE;
	rp.Scale[0] = WAVE_LocalScale[0];
	rp.Scale[1] = WAVE_LocalScale[1];
	rp.Scale[2] = WAVE_LocalScale[2];

	WAVE_ListAdd();
/*
	if (WaterDebug.DrawWaterMesh && WaterDebug.DrawNearMesh)
	{
		WAVETEX_SubmitMesh(NearWaterMeshID, WATER_LocalToWorld, rp
#ifndef TARGET_XBOX
			, WaterDebug.DrawNearMesh, WaterDebug.DrawNearMeshDark, WaterDebug.DrawNearMeshHigh, WaterDebug.DrawNearMeshHigh, true
#endif
		);
	}
*/
	if (WaterDebug.DrawWaterMesh && WaterDebug.DrawSeamMesh)
	{
		WAVETEX_SubmitMesh(SeamWaterMeshID, WATER_LocalToWorld, rp
#ifndef TARGET_XBOX
			, WaterDebug.DrawSeamMesh, WaterDebug.DrawSeamMeshDark, WaterDebug.DrawSeamMeshHigh, WaterDebug.DrawSeamMeshHigh, true
#endif
		);
	}
#ifndef TARGET_XBOX
	if (WaterDebug.DrawWaterMesh && WaterDebug.DrawHorizonMesh)
	{
		WAVETEX_SubmitMesh(HorizonWaterMeshID, WATER_LocalToWorld, rp
			, false, false, false, false, false
		);
	}
#endif
	if (WaterDebug.DrawWaterMesh && WaterDebug.DrawFarMesh)
	{
		WAVETEX_SubmitMesh(FarWaterMeshID, WATER_LocalToWorld, rp
#ifndef TARGET_XBOX
			, WaterDebug.DrawFarMesh, WaterDebug.DrawFarMeshDark, WaterDebug.DrawFarMeshHigh, WaterDebug.DrawFarMeshHigh, true
#endif
		);
	}
}



static void WATER_ScaleToWave(void)
{
  nglMatrix nglWorldToScreen;
  nglGetMatrix (nglWorldToScreen, NGLMTX_WORLD_TO_SCREEN);
/*
	WATER_SeamSizeX = WATER_StandardSeamSizeX * WAVE_ScaleSpatial;
	WATER_SeamSizeZ = WATER_StandardSeamSizeZ * WAVE_ScaleSpatial;

	WATER_MarginSizeX = WATER_StandardMarginSizeX * WAVE_ScaleSpatial;
	WATER_MarginSizeZ = WATER_StandardMarginSizeZ * WAVE_ScaleSpatial;

	WATER_InfinityX = WATER_StandardInfinityX * WAVE_ScaleSpatial;
	WATER_InfinityZ = WATER_StandardInfinityZ * WAVE_ScaleSpatial;
	WATER_DepthCutoff = WATER_StandardDepthCutoff / WAVE_ScaleSpatial;
*/
	WATER_SeamSizeX = WATER_StandardSeamSizeX;
	WATER_SeamSizeZ = WATER_StandardSeamSizeZ;

	WATER_MarginSizeX = WATER_StandardMarginSizeX;
	WATER_MarginSizeZ = WATER_StandardMarginSizeZ;

	WATER_InfinityX = WATER_StandardInfinityX;
	WATER_InfinityZ = WATER_StandardInfinityZ;
	WATER_DepthCutoff = WATER_StandardDepthCutoff;

	// cast away const because the Sony prototype doesn't use it
	nglMulMatrix(WATER_LocalToScreen, nglWorldToScreen, *(nglMatrix *) &WATER_LocalToWorld);
}

#ifndef countof
#define countof(a) (sizeof(a)/sizeof(*(a)))
#endif

inline float WATER_Sin(float a)
{
	static const float twopi = 6.283185308;
	static const float sintable[] = {
	#include "sin.txt"
	};
	int index = ((int) (a / twopi * countof(sintable))) % (int) countof(sintable);
	return (index >= 0) ? sintable[index] : -sintable[-index];
}

inline float WATER_Cos(float a)
{
  return WATER_Sin (a + 1.570796327f); // cos (a) = sin (a + pi/2)
}

inline float WATER_AltitudeInternal(float x, float z)
{
/*
	if (
		x < WATER_MeshMinX + WATER_MarginSizeX ||
		x > WATER_MeshMaxX - WATER_MarginSizeX ||
		z < WATER_MeshMinZ + WATER_MarginSizeZ ||
		z > WATER_MeshMaxZ - WATER_MarginSizeZ
	)
	{
*/
		return 0;
/*
	}
	else
	{
		float altitude;

		altitude = WATER_MaxAmplitude * WATER_Sin(x * WATER_XFrequency)
			* WATER_Sin((z - TIMER_GetTotalSec() * WATER_TimeFrequency) * WATER_ZFrequency);

		return altitude;
	}
*/
}

float WATER_Altitude(float x, float z)
{
	return WATER_AltitudeInternal(x, z);
}

inline void WATER_NormalInternal(float x, float z, float &nx, float &ny, float &nz)
{
/*
	if (x < WATER_MeshMinX + WATER_MarginSizeX ||
		x > WATER_MeshMaxX - WATER_MarginSizeX ||
		z < WATER_MeshMinZ + WATER_MarginSizeZ ||
		z > WATER_MeshMaxZ - WATER_MarginSizeZ)
  {
*/
    nx = 0;
    ny = 1;
    nz = 0;
/*
  }
  else
  {
    float dx, dz;

    dx = WATER_MaxAmplitude * WATER_Cos(x * WATER_XFrequency) * WATER_XFrequency
			* WATER_Sin((z - TIMER_GetTotalSec() * WATER_TimeFrequency) * WATER_ZFrequency);
    dz = WATER_MaxAmplitude * WATER_Sin(x * WATER_XFrequency)
			* WATER_Cos((z - TIMER_GetTotalSec() * WATER_TimeFrequency) * WATER_ZFrequency)
            * WATER_ZFrequency;

    // normalize
    nglVector n(-dx, 1, -dz, 1);
    sceVu0Normalize (n, n);
    nx = n[0];
    ny = n[1];
    nz = n[2];
  }
*/
}

void WATER_Normal(float x, float z, float &nx, float &ny, float &nz)
{
	WATER_NormalInternal(x, z, nx, ny, nz);
}

static inline void WATER_TextureCoordU(float x, float &u)
{
	u = WATER_ScaleU * x + WATER_OffsetU;
}

static inline void WATER_TextureCoordV(float z, float &v)
{
	v = WATER_ScaleV * z + WATER_OffsetV;
}

static inline void WATER_TextureCoord(float x, float z, float &u, float &v)
{
	u = WATER_ScaleU * x + WATER_OffsetU;
	v = WATER_ScaleV * z + WATER_OffsetV;
}

static inline void WATER_InverseTextureCoordX(float u, float &x)
{
	x = (u - WATER_OffsetU) / WATER_ScaleU;
}

static inline void WATER_InverseTextureCoordZ(float v, float &z)
{
	z = (v - WATER_OffsetV) / WATER_ScaleV;
}

static inline void WATER_InverseTextureCoord(float u, float v, float &x, float &z)
{
	x = (u - WATER_OffsetU) / WATER_ScaleU;
	z = (v - WATER_OffsetV) / WATER_ScaleV;
}

#include "watercolors.h"

/*
static const u_int WATER_VertexColor[4] = {
	COLOR_GRAY,
	COLOR_GRAY,
	COLOR_GRAY,
	COLOR_GRAY,
};
*/
static const u_int &WATER_HorizonColor = WATER_AverageColor;	// needs to be close to the average color of the water texture
static const u_int WATER_FunkyVertexColor[4] = {
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_RED,
	COLOR_BLACK,
};

/*	Fill in the xz - lattice of points which serve as the basis for the mesh.  Spacing is
	uniform in world units.
*/
static void WATER_ComputeUniformGrid(void)
{
	u_int xstep, zstep;
	float xbase, zbase;
	const float xcutlo = WAVE_MeshMinX - WATER_SeamSizeX;
	const float xcuthi = WAVE_MeshMaxX + WATER_SeamSizeX;
	const float zcutlo = WAVE_MeshMinZ - WATER_SeamSizeZ;
	const float zcuthi = WAVE_MeshMaxZ + WATER_SeamSizeZ;

	if (WATER_MeshMinX > xcutlo)
	{
		WATER_MeshMinX = xcutlo;
	}
	if (WATER_MeshMaxX < xcuthi)
	{
		WATER_MeshMaxX = xcuthi;
	}
	if (WATER_MeshMinZ > zcutlo)
	{
		WATER_MeshMinZ = zcutlo;
	}
	if (WATER_MeshMaxZ < zcuthi)
	{
		WATER_MeshMaxZ = zcuthi;
	}

	float xinc = ((float) WATER_MESH_WIDTH) / WATER_MESHIDEALSTEPX;
	float zinc = ((float) WATER_MESH_DEPTH) / WATER_MESHIDEALSTEPZ;

	for (xstep = 0, xbase = WATER_MeshMinX; xstep <= WATER_MESHIDEALSTEPX; ++xstep, xbase += xinc)
	{
		WATER_MeshGridX[xstep] = xbase;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
	}
	WATER_MeshStepX = WATER_MESHIDEALSTEPX;

	for (zstep = 0, zbase = WATER_MeshMinZ; zstep <= WATER_MESHIDEALSTEPZ; ++zstep, zbase += zinc)
	{
		WATER_MeshGridZ[zstep] = zbase;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
	}
	WATER_MeshStepZ = WATER_MESHIDEALSTEPZ;
}

static inline void WATER_XZToScreen(float x, float z, float &p, float &q)
{
	const nglMatrix &m = WATER_LocalToScreen;

	p = m[0][0] * x + m[2][0] * z + m[3][0];	// indexing on m is col x row
	q = m[0][1] * x + m[2][1] * z + m[3][1];

	float invdenom = 1 / (m[0][3] * x + m[2][3] * z + m[3][3]);

	p *= invdenom;
	q *= invdenom;
}

static inline void WATER_ScreenToXZ(float p, float q, float &x, float &z)
{
	const nglMatrix &m = WATER_LocalToScreen;
	float mm00, mm01, mm10, mm11, b0, b1, invdetmm;

	mm00 = m[0][0] - p * m[0][3];	// indexing on m is col x row
	mm01 = m[2][0] - p * m[2][3];
	mm10 = m[0][1] - q * m[0][3];
	mm11 = m[2][1] - q * m[2][3];

	b0 = - ( m[3][0] - p * m[3][3] );
	b1 = - ( m[3][1] - q * m[3][3] );

	invdetmm = 1 / (mm00 * mm11 - mm01 * mm10);

	x = invdetmm * (+ mm11 * b0 - mm01 * b1);
	z = invdetmm * (- mm10 * b0 + mm00 * b1);
}

/*	Fill in the xz - lattice of points which serve as the basis for the mesh.  Spacing is
	uniform in screen units, so that subdivision is finer near the camera.
*/
static void WATER_ComputeScaledGrid(void)
{
	u_int xstep, zstep;
	float xbase, zbase;
	const float xcutlo = WAVE_MeshMinX - WATER_SeamSizeX;
	const float xcuthi = WAVE_MeshMaxX + WATER_SeamSizeX;
	const float zcutlo = WAVE_MeshMinZ - WATER_SeamSizeZ;
	const float zcuthi = WAVE_MeshMaxZ + WATER_SeamSizeZ;

	if (WATER_MeshMinX > xcutlo)
	{
		WATER_MeshMinX = xcutlo;
	}
	if (WATER_MeshMaxX < xcuthi)
	{
		WATER_MeshMaxX = xcuthi;
	}
	if (WATER_MeshMinZ > zcutlo)
	{
		WATER_MeshMinZ = zcutlo;
	}
	if (WATER_MeshMaxZ < zcuthi)
	{
		WATER_MeshMaxZ = zcuthi;
	}

//	float xinc = ((float) WATER_MESH_WIDTH) / WATER_MESHIDEALSTEPX;
//	float zinc = ((float) WATER_MESH_DEPTH) / WATER_MESHIDEALSTEPZ;

	float xnext, znext;
	const float xlo = WATER_MeshMinX;
	const float xhi = WATER_MeshMaxX;
	const float xmid = (WATER_MeshMinX + WATER_MeshMaxX) / 2;
	const float zlo = WATER_MeshMinZ;
	const float zhi = WATER_MeshMaxZ;
	const float zmid = (WATER_MeshMinZ + WATER_MeshMaxZ) / 2;

	float plo, qlo;
	float phi, qhi;
	float pnext, qnext;
	float p, q;
	float pinc, qinc;

	// find the extents of the xz block in screen space.
	WATER_XZToScreen(xlo, zmid, plo, qlo);
	WATER_XZToScreen(xhi, zmid, phi, qhi);

	pinc = (phi - plo) / WATER_MESHIDEALSTEPX;
	qinc = (qhi - qlo) / WATER_MESHIDEALSTEPX;

	for (
		xstep = 0, p = plo, q = qlo, xbase = WATER_MeshMinX;
		xstep <= WATER_MESHIDEALSTEPX;
		++xstep, p = pnext, q = qnext, xbase = xnext
	)
	{
		WATER_MeshGridX[xstep] = xbase;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);

		// Find next x
		pnext = p + pinc;
		qnext = q + qinc;
		WATER_ScreenToXZ(pnext, qnext, xnext, znext);
	}
	WATER_MeshStepX = WATER_MESHIDEALSTEPX;

	// find the extents of the xz block in screen space.
	WATER_XZToScreen(xmid, zlo, plo, qlo);
	WATER_XZToScreen(xmid, zhi, phi, qhi);

	pinc = (phi - plo) / WATER_MESHIDEALSTEPZ;
	qinc = (qhi - qlo) / WATER_MESHIDEALSTEPZ;

	for (
		zstep = 0, p = plo, q = qlo, zbase = WATER_MeshMinZ;
		zstep <= WATER_MESHIDEALSTEPZ;
		++zstep, p = pnext, q = qnext, zbase = znext
	)
	{
		WATER_MeshGridZ[zstep] = zbase;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);

		// Find next z
		pnext = p + pinc;
		qnext = q + qinc;
		WATER_ScreenToXZ(pnext, qnext, xnext, znext);
	}
	WATER_MeshStepZ = WATER_MESHIDEALSTEPZ;

}

#ifdef OLD_NEARMESH
/*	Fill in the xz - lattice of points which serve as the basis for the mesh.  Spacing is
	uniform in world units.
*/
static void WATER_ComputeUniformGrid(void)
{
	u_int xstep, zstep;
	float xbase, zbase;
	float xinc, zinc;
	float xincdef = ((float) WATER_MESH_WIDTH) / WATER_MESHIDEALSTEPX;
	float zincdef = ((float) WATER_MESH_DEPTH) / WATER_MESHIDEALSTEPZ;
	const float xcutlo = WAVE_MeshMinX - WATER_SeamSizeX;
	const float xcuthi = WAVE_MeshMaxX + WATER_SeamSizeX;
	const float zcutlo = WAVE_MeshMinZ - WATER_SeamSizeZ;
	const float zcuthi = WAVE_MeshMaxZ + WATER_SeamSizeZ;

	xstep = 0;
	if (WATER_MeshMinX > xcutlo)
	{
		WATER_SeamLoX = 0;
		WATER_MeshGridX[xstep] = xcutlo;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
		++xstep;
	}
	for (xbase = WATER_MeshMinX; xbase < WATER_MeshMaxX; ++xstep, xbase += xinc)
	{
		WATER_MeshGridX[xstep] = xbase;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);

		// Align to the edges of the wave
		xinc = xincdef;
		if (xbase < xcutlo && xbase + xinc > xcutlo)
		{
			WATER_SeamLoX = xstep + 1;
			xinc = xcutlo - xbase;
		}
		else if (xbase < xcuthi && xbase + xinc > xcuthi)
		{
			WATER_SeamHiX = xstep + 1;
			xinc = xcuthi - xbase;
		}
	}
	WATER_MeshGridX[xstep] = WATER_MeshMaxX;
	WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
	++xstep;
	if (WATER_MeshMaxX < xcuthi)
	{
		WATER_SeamHiX = xstep;
		WATER_MeshGridX[xstep] = xcuthi;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
		++xstep;
	}
	WATER_MeshStepX = xstep - 1;

	zstep = 0;
	if (WATER_MeshMinZ > zcutlo)
	{
		WATER_SeamLoZ = 0;
		WATER_MeshGridZ[zstep] = zcutlo;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
		++zstep;
	}
	for (zbase = WATER_MeshMinZ; zbase < WATER_MeshMaxZ; ++zstep, zbase += zinc)
	{
		WATER_MeshGridZ[zstep] = zbase;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);

		// Align to the edges of the wave
		zinc = zincdef;
		if (zbase < zcutlo && zbase + zinc > zcutlo)
		{
			WATER_SeamLoZ = zstep + 1;
			zinc = zcutlo - zbase;
		}
		else if (zbase < zcuthi && zbase + zinc > zcuthi)
		{
			WATER_SeamHiZ = zstep + 1;
			zinc = zcuthi - zbase;
		}
	}
	WATER_MeshGridZ[zstep] = WATER_MeshMaxZ;
	WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
	++zstep;
	if (WATER_MeshMaxZ < zcuthi)
	{
		WATER_SeamHiZ = zstep;
		WATER_MeshGridZ[zstep] = zcuthi;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
		++zstep;
	}
	WATER_MeshStepZ = zstep - 1;
}

static inline void WATER_XZToScreen(float x, float z, float &p, float &q)
{
	const nglMatrix &m = WATER_LocalToScreen;

	p = m[0][0] * x + m[2][0] * z + m[3][0];	// indexing on m is col x row
	q = m[0][1] * x + m[2][1] * z + m[3][1];

	float invdenom = 1 / (m[0][3] * x + m[2][3] * z + m[3][3]);

	p *= invdenom;
	q *= invdenom;
}

static inline void WATER_ScreenToXZ(float p, float q, float &x, float &z)
{
	const nglMatrix &m = WATER_LocalToScreen;
	float mm00, mm01, mm10, mm11, b0, b1, invdetmm;

	mm00 = m[0][0] - p * m[0][3];	// indexing on m is col x row
	mm01 = m[2][0] - p * m[2][3];
	mm10 = m[0][1] - q * m[0][3];
	mm11 = m[2][1] - q * m[2][3];

	b0 = - ( m[3][0] - p * m[3][3] );
	b1 = - ( m[3][1] - q * m[3][3] );

	invdetmm = 1 / (mm00 * mm11 - mm01 * mm10);

	x = invdetmm * (+ mm11 * b0 - mm01 * b1);
	z = invdetmm * (- mm10 * b0 + mm00 * b1);
}

/*	Fill in the xz - lattice of points which serve as the basis for the mesh.  Spacing is
	uniform in screen units, so that subdivision is finer near the camera.
*/
static void WATER_ComputeScaledGrid(void)
{
	u_int xstep, zstep;
	float xbase, zbase;
	float xnext, znext;
	const float xlo = WATER_MeshMinX;
	const float xhi = WATER_MeshMaxX;
	const float xmid = (WATER_MeshMinX + WATER_MeshMaxX) / 2;
	const float zlo = WATER_MeshMinZ;
	const float zhi = WATER_MeshMaxZ;
	const float zmid = (WATER_MeshMinZ + WATER_MeshMaxZ) / 2;
	const float xcutlo = WAVE_MeshMinX - WATER_SeamSizeX;
	const float xcuthi = WAVE_MeshMaxX + WATER_SeamSizeX;
	const float zcutlo = WAVE_MeshMinZ - WATER_SeamSizeZ;
	const float zcuthi = WAVE_MeshMaxZ + WATER_SeamSizeZ;

	float plo, qlo;
	float phi, qhi;
	float pnext, qnext;
	float p, q;
	float pincdef, qincdef;

	// find the extents of the xz block in screen space.
	WATER_XZToScreen(xlo, zmid, plo, qlo);
	WATER_XZToScreen(xhi, zmid, phi, qhi);

	pincdef = (phi - plo) / WATER_MESHIDEALSTEPX;
	qincdef = (qhi - qlo) / WATER_MESHIDEALSTEPX;

	xstep = 0;
	if (WATER_MeshMinX > xcutlo)
	{
		WATER_SeamLoX = 0;
		WATER_MeshGridX[xstep] = xcutlo;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
		++xstep;
	}
	for (
		p = plo, q = qlo, xbase = WATER_MeshMinX;
		xbase < WATER_MeshMaxX;
		++xstep, p = pnext, q = qnext, xbase = xnext
	)
	{
		WATER_MeshGridX[xstep] = xbase;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);

		// Find next proposed x
		pnext = p + pincdef;
		qnext = q + qincdef;
		WATER_ScreenToXZ(pnext, qnext, xnext, znext);

		// Align to the edges of the wave
		if (xbase < xcutlo && xnext > xcutlo)
		{
			WATER_SeamLoX = xstep + 1;
			xnext = xcutlo;
			WATER_XZToScreen(xnext, zmid, pnext, qnext);
		}
		else if (xbase < xcuthi && xnext > xcuthi)
		{
			WATER_SeamHiX = xstep + 1;
			xnext = xcuthi;
			WATER_XZToScreen(xnext, zmid, pnext, qnext);
		}
	}
	WATER_MeshGridX[xstep] = WATER_MeshMaxX;
	WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
	++xstep;
	if (WATER_MeshMaxX < xcuthi)
	{
		WATER_SeamHiX = xstep;
		WATER_MeshGridX[xstep] = xcuthi;
		WATER_TextureCoordU(WATER_MeshGridX[xstep], WATER_MeshGridU[xstep]);
		++xstep;
	}
	WATER_MeshStepX = xstep - 1;

	// find the extents of the xz block in screen space.
	WATER_XZToScreen(xmid, zlo, plo, qlo);
	WATER_XZToScreen(xmid, zhi, phi, qhi);

	pincdef = (phi - plo) / WATER_MESHIDEALSTEPZ;
	qincdef = (qhi - qlo) / WATER_MESHIDEALSTEPZ;

	zstep = 0;
	if (WATER_MeshMinZ > zcutlo)
	{
		WATER_SeamLoZ = 0;
		WATER_MeshGridZ[zstep] = zcutlo;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
		++zstep;
	}
	for (
		p = plo, q = qlo, zbase = WATER_MeshMinZ;
		zbase < WATER_MeshMaxZ;
		++zstep, p = pnext, q = qnext, zbase = znext
	)
	{
		WATER_MeshGridZ[zstep] = zbase;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);

		// Find next proposed z
		pnext = p + pincdef;
		qnext = q + qincdef;
		WATER_ScreenToXZ(pnext, qnext, xnext, znext);

		// Align to the edges of the wave
		if (zbase < zcutlo && znext > zcutlo)
		{
			WATER_SeamLoZ = zstep + 1;
			znext = zcutlo;
			WATER_XZToScreen(xmid, znext, pnext, qnext);
		}
		else if (zbase < zcuthi && znext > zcuthi)
		{
			WATER_SeamHiZ = zstep + 1;
			znext = zcuthi;
			WATER_XZToScreen(xmid, znext, pnext, qnext);
		}
	}
	WATER_MeshGridZ[zstep] = WATER_MeshMaxZ;
	WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
	++zstep;
	if (WATER_MeshMaxZ < zcuthi)
	{
		WATER_SeamHiZ = zstep;
		WATER_MeshGridZ[zstep] = zcuthi;
		WATER_TextureCoordV(WATER_MeshGridZ[zstep], WATER_MeshGridV[zstep]);
		++zstep;
	}
	WATER_MeshStepZ = zstep - 1;
}
#endif

extern float WAVE_camx;
extern float WAVE_camz;
//extern float transcale;
extern float WAVETEX_transcale;
extern int WAVETEX_transmin;
extern int WAVETEX_transmax;
extern int WAVETEX_transval;


#ifdef TARGET_GC

float ldfactor=0.5f;


extern float WAVE_camx;
extern float WAVE_camy;
extern float WAVE_camz;
float nglInnerProduct( const nglVector lhs, const nglVector rhs );
u_int WAVETEX_GCNearColor( void );
u_int WAVETEX_GCFarColor( void );
float WAVETEX_GCNFFadeScale( void );
float WAVETEX_GCNFFadeOffset( void );

float WATER_GCLightDarkCalc( float x, float y, float z, float scale, float offset )
{

	nglVector norm; norm[0]=0.0f; norm[1]=1.0f; norm[2]=0.0f;
	nglVector camv; camv[0]=WAVE_camx-x; camv[1]=WAVE_camy-y; camv[2]=WAVE_camz-z;
	nglVector camn;
	nglNormalize( camn, camv );
	float rv=nglInnerProduct( camn, norm );

	rv*=scale;
	rv+=offset;
	if ( rv < 0.0f ) rv=0.0f;
	if ( rv > 1.0f ) rv=1.0f;
	return rv;
	//return ldfactor;
}

inline u_int WATER_Transparency( u_int vertcolorfar, u_int vertcolornear, float xx, float yy, float zz, float scale, float offset)
{
	float lightdark=WATER_GCLightDarkCalc( xx, yy, zz, scale,offset );
	float ta,tr,tg,tb;
	ta=1.0f; //((lightdark*vertcolornear.a)+((1.0f-lightdark)*vertcolorfar.a))/2.0f;
	tr=2.0f * (((lightdark)*(((vertcolornear>>24)&0xFF)/255.0f))+((1.0f-lightdark)*(((vertcolorfar>>24)&0xFF)/255.0f)))/1.0f;
	tg=2.0f * (((lightdark)*(((vertcolornear>>16)&0xFF)/255.0f))+((1.0f-lightdark)*(((vertcolorfar>>16)&0xFF)/255.0f)))/1.0f;
	tb=2.0f * (((lightdark)*(((vertcolornear>> 8)&0xFF)/255.0f))+((1.0f-lightdark)*(((vertcolorfar>> 8)&0xFF)/255.0f)))/1.0f;
	//return (rgba & 0x00FFFFFF) | (WAVETEX_transval <<24);
	int a,r,g,b;
	a=r=g=b=128;

	a = (int) ( (float) a * ta );
	r = (int) ( (float) r * tr );
	g = (int) ( (float) g * tg );
	b = (int) ( (float) b * tb );
	a&=0xFF;
	r&=0xFF;
	g&=0xFF;
	b&=0xFF;

	assert(a<256);
	assert(r<256);
	assert(g<256);
	assert(b<256);
	assert(a>=0);
	assert(r>=0);
	assert(g>=0);
	assert(b>=0);

	return ((r&0xFF)<<24)+((g&0xFF)<<16)+((b&0xFF)<<8)+((0xFF)<<0);
	return vertcolorfar;
}

#else

inline u_int WATER_Transparency( u_int rgba, float xx, float yy, float zz)
{
	#ifdef BUILD_DEBUG
	return (rgba & 0x00FFFFFF) | (WAVETEX_transval <<24);
	#else
	return rgba;
	#endif
}

#endif




/*
static void WATER_FillInNearMesh(void)
{
	u_int xstep, zstep;
	float xbase, zbase;
	float xnext, znext;
	float ubase, vbase;
	float unext, vnext;
	const u_int * const vertcolor = WaterDebug.ShowNearLines ? WATER_FunkyVertexColor : WATER_VertexColor;
	const float xcutlo = WAVE_MeshMinX - WATER_SeamSizeX;
	const float xcuthi = WAVE_MeshMaxX + WATER_SeamSizeX;
	const float zcutlo = WAVE_MeshMinZ - WATER_SeamSizeZ;
	const float zcuthi = WAVE_MeshMaxZ + WATER_SeamSizeZ;
	u_int vertcount = 0;

#ifdef TARGET_GC
	u_int vertcolornear=WAVETEX_GCNearColor();
	u_int vertcolorfar= WAVETEX_GCFarColor();
	float gcfadescale=  WAVETEX_GCNFFadeScale();
	float gcfadeoffset= WAVETEX_GCNFFadeOffset();
#endif

	float x, y, z;
	float nx, ny, nz;
	float u, v;

	for (xstep = 0; xstep < WATER_MeshStepX; ++xstep)
	{
		xbase = WATER_MeshGridX[xstep];
		xnext = WATER_MeshGridX[xstep+1];
		ubase = WATER_MeshGridU[xstep];
		unext = WATER_MeshGridU[xstep+1];

		for (zstep = 0; zstep < WATER_MeshStepZ; ++zstep)
		{
			zbase = WATER_MeshGridZ[zstep];
			znext = WATER_MeshGridZ[zstep+1];
			vbase = WATER_MeshGridV[zstep];
			vnext = WATER_MeshGridV[zstep+1];

			// Skip portions covered by the wave
			if (xbase >= xcutlo && xbase < xcuthi &&
				zbase >= zcutlo && zbase < zcuthi
			)
				continue;

			// Add a pair of tris
			nglMeshWriteStrip(4);
			vertcount += 4;

			x = xbase;
			z = zbase;
			y = WATER_AltitudeInternal(x, z);
			u = ubase;
			v = vbase;
			WATER_NormalInternal(x, z, nx, ny, nz);
			#ifdef TARGET_GC
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
			#else
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolor[0],x,y,z), u, v);
			#endif

			x = xbase;
			z = znext;
			y = WATER_AltitudeInternal(x, z);
			u = ubase;
			v = vnext;
			WATER_NormalInternal(x, z, nx, ny, nz);
			#ifdef TARGET_GC
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
			#else
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolor[1],x,y,z), u, v);
			#endif

			x = xnext;
			z = zbase;
			y = WATER_AltitudeInternal(x, z);
			u = unext;
			v = vbase;
			WATER_NormalInternal(x, z, nx, ny, nz);
			#ifdef TARGET_GC
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
			#else
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolor[2],x,y,z), u, v);
			#endif

			x = xnext;
			z = znext;
			y = WATER_AltitudeInternal(x, z);
			u = unext;
			v = vnext;
			WATER_NormalInternal(x, z, nx, ny, nz);
			#ifdef TARGET_GC
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
			#else
				WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolor[3],x,y,z), u, v);
			#endif
		}
	}

	assert(vertcount <= NearMeshNumVert);
	// Fill in any remaining vectors in scratch mesh (should be fixed later)
	while (vertcount < NearMeshNumVert)
	{
		// Add a pair of tris
		nglMeshWriteStrip(4);
		vertcount += 4;
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
}
*/

#ifdef TARGET_GC
static inline void WATER_SeamSubmitVertex(const float &x, const float &z, float &y, float &nx, float &ny, float &nz,
	float &u, float &v, u_int &vertcolornear, u_int &vertcolorfar, u_int &vertcount,
	float gcfadescale, float gcfadeoffset )
#else
static inline void WATER_SeamSubmitVertex(const float &x, const float &z, float &y, float &nx, float &ny, float &nz,
	float &u, float &v, u_int &vertcolor, u_int &vertcount)
#endif
{
#ifdef NONFLAT_WATER
	y = WATER_AltitudeInternal(x, z);
	WATER_NormalInternal(x, z, nx, ny, nz);
#endif
	WATER_TextureCoord(x, z, u, v);
#ifdef DEBUG
#ifndef TARGET_GC
	if (WaterDebug.ShowSeamLines)
	{
		vertcolor = WATER_FunkyVertexColor[vertcount % countof(WATER_FunkyVertexColor)];
	}
#endif
#endif
#ifdef TARGET_GC
	WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
#else
	WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
#endif
	++vertcount;
}

/*	Upon entry, the arrays

		WAVE_MeshGridX, WAVE_MeshGridZ
		WATER_MeshGridX, WATER_MeshGridZ

	are filled in with the grid points for the wave mesh and the water mesh.  The water mesh
	is guaranteed to contain points which are spaced out by

		WATER_SeamSizeX, WATER_SeamSizeZ

	from the corners of the wave mesh.

	+-----+--+-----+-----+-----+-----+-----+-+-----+---+
    |                                                  |
    +                   water mesh                     +
    |                                                  |
    |                                                  |
    +        +-----+-----+-----+-----+-----+-+         +
    +        + \          seam             / +         +
    |        |  *-----*---*----*----*-----*  |         |
    |        |s |                         | s|         |
    +        +e *                         * e+         +
    |        |a *        wave mesh        * a|         |
    |        |m |                         | m|         |
    +        +  *                         *  +         +
    |        |  *-----*---*----*----*-----*  |         |
    |        | /          seam             \ |         |
    +        +-----+-----+-----+-----+-----+-+         +
    |                                                  |
    +                                                  +
    |                                                  |
    |                                                  |
	+-----+--+-----+-----+-----+-----+-----+-+-----+---+

	We have four seams to fill, each bounded on the two long sides by irregularly spaced vertices.
	We walk along the two sides, building triangles, at each step advancing whichever side is furthest
	behind.
*/
static void WATER_FillInSeamMesh(void)
{
	u_int inner, outer;
	const float xcutlo = min(WATER_MeshMinX,WAVE_MeshMinX - WATER_SeamSizeX);//WAVE_MeshMinX - WATER_SeamSizeX;
	const float xcuthi = max(WATER_MeshMaxX,WAVE_MeshMaxX + WATER_SeamSizeX);//WAVE_MeshMaxX + WATER_SeamSizeX;
	const float zcutlo = min(WATER_MeshMinZ,WAVE_MeshMinZ - WATER_SeamSizeZ);//WAVE_MeshMinZ - WATER_SeamSizeZ;
	const float zcuthi = max(WATER_MeshMaxZ,WAVE_MeshMaxZ + WATER_SeamSizeZ);//WAVE_MeshMaxZ + WATER_SeamSizeZ;
	const u_int &xseamlo = 0;//WATER_SeamLoX;
	const u_int &xseamhi = WATER_MeshStepX;//WATER_SeamHiX;
	const u_int &zseamlo = 0;//WATER_SeamLoZ;
	const u_int &zseamhi = WATER_MeshStepZ;//WATER_SeamHiZ;
	u_int vertcolor = COLOR_GRAY;
	u_int vertcount = 0;
	bool doinner;
	bool previnner = false, prevprevinner;
	bool submitinner, submitouter;

#ifdef TARGET_GC
	u_int vertcolornear=WAVETEX_GCNearColor();
	u_int vertcolorfar= WAVETEX_GCFarColor();
	float gcfadescale=  WAVETEX_GCNFFadeScale();
	float gcfadeoffset= WAVETEX_GCNFFadeOffset();
#endif

	float x = 0, y = 0, z = 0;	// no need to recompute y, n for flat water.
	float nx = 0, ny = 1, nz = 0;
	float u, v;

	for (int i = 0; i < 2; ++i)
	{
		// First step will be along outer side, so we start strip with outer vertex
		assert(WAVE_MeshGridX[0] > WATER_MeshGridX[xseamlo]);

		// Add a strip
		nglMeshWriteStrip(SeamStripSizeX);
		vertcount = 0;

		for (inner = 0, outer = xseamlo; inner < WAVE_MeshStepX || outer < xseamhi; doinner ? ++inner : ++outer)
		{
			// Advance either the inner or outer point
			if (inner == WAVE_MeshStepX)
			{
				doinner = false;
			}
			else if (outer == xseamhi)
			{
				doinner = true;
			}
			else
			{
				doinner = WAVE_MeshGridX[inner] < WATER_MeshGridX[outer];
			}

			// Decide how many points need to be added to the strip
			if (inner == 0 && outer == xseamlo)
			{
				submitinner = submitouter = true;
			}
			else
			{
#ifdef AGGRESSIVE_STRIPPING
				submitinner = !previnner && !prevprevinner;
				submitouter = previnner && prevprevinner;
#else
				submitinner = !previnner && !doinner;
				submitouter = previnner && doinner;
#endif
			}

			float zin = (i == 0) ? WAVE_MeshMinZ : WAVE_MeshMaxZ;
			float zout = (i == 0) ? zcutlo : zcuthi;

			// outer point
			if (submitouter)
			{
				x = WATER_MeshGridX[outer];
				z = zout;
				#ifdef TARGET_GC
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolornear, vertcolorfar, vertcount,gcfadescale,gcfadeoffset);
				#else
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolor, vertcount);
				#endif
				prevprevinner = previnner; previnner = false;
			}

			// inner point
			if (submitinner)
			{
				x = WAVE_MeshGridX[inner];
				z = zin;
				#ifdef TARGET_GC
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolornear, vertcolorfar, vertcount,gcfadescale,gcfadeoffset);
				#else
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolor, vertcount);
				#endif
				prevprevinner = previnner; previnner = true;
			}

			// advance point
			if (doinner)
			{
				x = WAVE_MeshGridX[inner+1];
				z = zin;
			}
			else
			{
				x = WATER_MeshGridX[outer+1];
				z = zout;
			}
			#ifdef TARGET_GC
			WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolornear, vertcolorfar, vertcount,gcfadescale,gcfadeoffset);
			#else
			WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolor, vertcount);
			#endif
			prevprevinner = previnner; previnner = doinner;
		}

		assert(vertcount <= SeamStripSizeX);
		// Fill in any remaining vectors in strip with copies of the previous vertex
		while (vertcount < SeamStripSizeX)
		{
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
			++vertcount;
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		// First step will be along outer side, so we start strip with outer vertex
		assert(WAVE_MeshGridZ[0] > WATER_MeshGridZ[zseamlo]);

		// Add a strip
		nglMeshWriteStrip(SeamStripSizeZ);
		vertcount = 0;

		for (inner = 0, outer = zseamlo; inner < WAVE_MeshStepZ || outer < zseamhi; doinner ? ++inner : ++outer)
		{
			// Advance either the inner or outer point
			if (inner == WAVE_MeshStepZ)
			{
				doinner = false;
			}
			else if (outer == zseamhi)
			{
				doinner = true;
			}
			else
			{
				doinner = WAVE_MeshGridZ[inner] < WATER_MeshGridZ[outer];
			}

			// Decide how many points need to be added to the strip
			if (inner == 0 && outer == zseamlo)
			{
				submitinner = submitouter = true;
			}
			else
			{
#ifdef AGGRESSIVE_STRIPPING
				submitinner = !previnner && !prevprevinner;
				submitouter = previnner && prevprevinner;
#else
				submitinner = !previnner && !doinner;
				submitouter = previnner && doinner;
#endif
			}

			float xin = (i == 0) ? WAVE_MeshMinX : WAVE_MeshMaxX;
			float xout = (i == 0) ? xcutlo : xcuthi;

			// outer point
			if (submitouter)
			{
				x = xout;
				z = WATER_MeshGridZ[outer];
				#ifdef TARGET_GC
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolornear, vertcolorfar, vertcount,gcfadescale,gcfadeoffset);
				#else
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolor, vertcount);
				#endif
				prevprevinner = previnner; previnner = false;
			}

			// inner point
			if (submitinner)
			{
				x = xin;
				z = WAVE_MeshGridZ[inner];
				#ifdef TARGET_GC
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolornear, vertcolorfar, vertcount,gcfadescale,gcfadeoffset);
				#else
				WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolor, vertcount);
				#endif
				prevprevinner = previnner; previnner = true;
			}

			// advance point
			if (doinner)
			{
				x = xin;
				z = WAVE_MeshGridZ[inner+1];
			}
			else
			{
				x = xout;
				z = WATER_MeshGridZ[outer+1];
			}
			#ifdef TARGET_GC
			WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolornear, vertcolorfar, vertcount,gcfadescale,gcfadeoffset);
			#else
			WATER_SeamSubmitVertex(x, z, y, nx, ny, nz, u, v, vertcolor, vertcount);
			#endif
			prevprevinner = previnner; previnner = doinner;
		}

		assert(vertcount <= SeamStripSizeZ);
		// Fill in any remaining vectors in strip with copies of the previous vertex
		while (vertcount < SeamStripSizeZ)
		{
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
			++vertcount;
		}
	}
}

static void WATER_FillInFarMesh(void)
{
#ifdef TARGET_GC
	u_int vertcolornear=WAVETEX_GCNearColor();
	u_int vertcolorfar= WAVETEX_GCFarColor();
	float gcfadescale=  WAVETEX_GCNFFadeScale();
	float gcfadeoffset= WAVETEX_GCNFFadeOffset();
#endif

	float X[4] = {
		-WATER_InfinityX,
		min(WAVE_MeshMinX - WATER_SeamSizeX, WATER_MeshMinX),
		max(WAVE_MeshMaxX + WATER_SeamSizeX, WATER_MeshMaxX),
		WATER_InfinityX,
	};
	float Z[4] = {
		-WATER_InfinityZ,
		min(WAVE_MeshMinZ - WATER_SeamSizeZ, WATER_MeshMinZ),
		max(WAVE_MeshMaxZ + WATER_SeamSizeZ, WATER_MeshMaxZ),
		WATER_InfinityZ,
	};

	// Go to a lot of trouble to get around the tiling limits.  (dc 07/08/02)
	float Ulo[4];
	float Vlo[4];
	float Uhi[4];
	float Vhi[4];

	const float &umin = WATER_TexMinU, &umax = WATER_TexMaxU;
	const float &vmin = WATER_TexMinV, &vmax = WATER_TexMaxV;
/*	assert(
		-WATER_InfinityX < WATER_MeshMinX &&
		WATER_InfinityX > WATER_MeshMaxX &&
		-WATER_InfinityZ < WATER_MeshMinZ &&
		WATER_InfinityZ > WATER_MeshMaxZ &&
		-WATER_InfinityX < WAVE_MeshMinX &&
		WATER_InfinityX > WAVE_MeshMaxX &&
		-WATER_InfinityZ < WAVE_MeshMinZ &&
		WATER_InfinityZ > WAVE_MeshMaxZ
	);*/

	WATER_TextureCoord(X[1], Z[1], Ulo[1], Vlo[1]);
	WATER_TextureCoord(X[2], Z[2], Uhi[2], Vhi[2]);

	float usign = (WATER_ScaleU > 0) ? 1 : -1;	// to handle flipped tiling (dc 06/18/02)

	Uhi[1] = Ulo[1] - ((int) Ulo[1]) + usign * (umax - 1);
	Ulo[0] = Uhi[1] - usign * ((umax - umin) + 2);
	Ulo[2] = Uhi[2] - ((int) Uhi[2]) + usign * (umin + 1);
	Uhi[3] = Ulo[2] + usign * ((umax - umin) - 2);

	X[0] = X[1] - (Uhi[1] - Ulo[0]) / WATER_ScaleU;
	X[3] = X[2] + (Uhi[3] - Ulo[2]) / WATER_ScaleU;

	float vsign = (WATER_ScaleV > 0) ? 1 : -1;	// to handle flipped tiling (dc 06/18/02)

	Vhi[1] = Vlo[1] - ((int) Vlo[1]) + vsign * (vmax - 1);
	Vlo[0] = Vhi[1] - vsign * ((vmax - vmin) + 2);
	Vlo[2] = Vhi[2] - ((int) Vhi[2]) + vsign * (vmin + 1);
	Vhi[3] = Vlo[2] + vsign * ((vmax - vmin) - 2);

	Z[0] = Z[1] - (Vhi[1] - Vlo[0]) / WATER_ScaleV;
	Z[3] = Z[2] + (Vhi[3] - Vlo[2]) / WATER_ScaleV;

	const float y = 0;
	const float nx = 0, ny = 1, nz = 0;
	const u_int colorin = COLOR_GRAY;
#ifdef TARGET_PS2
	const u_int colorout = ALPHA_TRANSPARENT | (~ALPHA_OPAQUE & COLOR_GRAY);
#else
	const u_int &colorout = colorin;
#endif
	u_int vertcolor;
	float x, z;
	float u, v;
	float xin, zin, xout, zout;
	float uin, vin, uout, vout;
	u_int vertcount = 0;
	u_int xstep, zstep;

	// low Z side
	nglMeshWriteStrip(FarStripSizeX);
	zin = Z[1];
	vin = Vhi[1];
	zout = Z[0];
	vout = Vlo[0];

	for (xstep = 0; xstep <= WATER_MeshStepX; ++xstep)
	{
		vertcount += 2;

		x = WATER_MeshGridX[xstep];
		z = zin;
		u = WATER_MeshGridU[xstep];
		v = vin;
		vertcolor = colorin;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif

		x = WATER_MeshGridX[xstep];
		z = zout;
		u = WATER_MeshGridU[xstep];
		v = vout;
		vertcolor = colorout;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif
	}

	// high Z side
	nglMeshWriteStrip(FarStripSizeZ);
	zin = Z[2];
	vin = Vlo[2];
	zout = Z[3];
	vout = Vhi[3];

	for (xstep = 0; xstep <= WATER_MeshStepX; ++xstep)
	{
		vertcount += 2;

		x = WATER_MeshGridX[xstep];
		z = zin;
		u = WATER_MeshGridU[xstep];
		v = vin;
		vertcolor = colorin;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif

		x = WATER_MeshGridX[xstep];
		z = zout;
		u = WATER_MeshGridU[xstep];
		v = vout;
		vertcolor = colorout;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	 	#endif
	}

	// low X side
	nglMeshWriteStrip(FarStripSizeX);
	xin = X[1];
	uin = Uhi[1];
	xout = X[0];
	uout = Ulo[0];

	for (zstep = 0; zstep <= WATER_MeshStepZ; ++zstep)
	{
		vertcount += 2;

		x = xin;
		z = WATER_MeshGridZ[zstep];
		u = uin;
		v = WATER_MeshGridV[zstep];
		vertcolor = colorin;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif

		x = xout;
		z = WATER_MeshGridZ[zstep];
		u = uout;
		v = WATER_MeshGridV[zstep];
		vertcolor = colorout;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif
	}

	// high X side
	nglMeshWriteStrip(FarStripSizeZ);
	xin = X[2];
	uin = Ulo[2];
	xout = X[3];
	uout = Uhi[3];

	for (zstep = 0; zstep <= WATER_MeshStepZ; ++zstep)
	{
		vertcount += 2;

		x = xin;
		z = WATER_MeshGridZ[zstep];
		u = uin;
		v = WATER_MeshGridV[zstep];
		vertcolor = colorin;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif

		x = xout;
		z = WATER_MeshGridZ[zstep];
		u = uout;
		v = WATER_MeshGridV[zstep];
		vertcolor = colorout;
		#ifdef TARGET_GC
			WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
		#else
			WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
		#endif
	}

	// low X low Z corner
	nglMeshWriteStrip(4);
	xin = X[1];
	uin = Uhi[1];
	xout = X[0];
	uout = Ulo[0];
	zin = Z[1];
	vin = Vhi[1];
	zout = Z[0];
	vout = Vlo[0];

	vertcount += 4;

	x = xin;
	z = zin;
	u = uin;
	v = vin;
	vertcolor = colorin;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xin;
	z = zout;
	u = uin;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zin;
	u = uout;
	v = vin;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zout;
	u = uout;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	// low X high Z corner
	nglMeshWriteStrip(4);
	xin = X[1];
	uin = Uhi[1];
	xout = X[0];
	uout = Ulo[0];
	zin = Z[2];
	vin = Vlo[2];
	zout = Z[3];
	vout = Vhi[3];

	vertcount += 4;

	x = xin;
	z = zin;
	u = uin;
	v = vin;
	vertcolor = colorin;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xin;
	z = zout;
	u = uin;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zin;
	u = uout;
	v = vin;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zout;
	u = uout;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	// high X low Z corner
	nglMeshWriteStrip(4);
	xin = X[2];
	uin = Ulo[2];
	xout = X[3];
	uout = Uhi[3];
	zin = Z[1];
	vin = Vhi[1];
	zout = Z[0];
	vout = Vlo[0];

	vertcount += 4;

	x = xin;
	z = zin;
	u = uin;
	v = vin;
	vertcolor = colorin;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xin;
	z = zout;
	u = uin;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zin;
	u = uout;
	v = vin;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zout;
	u = uout;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	// low X high Z corner
	nglMeshWriteStrip(4);
	xin = X[2];
	uin = Ulo[2];
	xout = X[3];
	uout = Uhi[3];
	zin = Z[2];
	vin = Vlo[2];
	zout = Z[3];
	vout = Vhi[3];

	vertcount += 4;

	x = xin;
	z = zin;
	u = uin;
	v = vin;
	vertcolor = colorin;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xin;
	z = zout;
	u = uin;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zin;
	u = uout;
	v = vin;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	x = xout;
	z = zout;
	u = uout;
	v = vout;
	vertcolor = colorout;
	#ifdef TARGET_GC
		WATER_SubmitVertex(x, y, z, nx, ny, nz, WATER_Transparency(vertcolorfar,vertcolornear,x,y,z,gcfadescale,gcfadeoffset), u, v);
	#else
		WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor, u, v);
	#endif

	assert(vertcount == FarMeshNumVert);
}

#if 0
static void WATER_OldFillInFarMesh(void)
{
	float y = 0;
	float nx = 0, ny = 1, nz = 0;
	float X[4] = {
		-WATER_InfinityX,
		min(WAVE_MeshMinX - WATER_SeamSizeX, WATER_MeshMinX),
		max(WAVE_MeshMaxX + WATER_SeamSizeX, WATER_MeshMaxX),
		WATER_InfinityX,
	};
	float Z[4] = {
		-WATER_InfinityZ,
		min(WAVE_MeshMinZ - WATER_SeamSizeZ, WATER_MeshMinZ),
		max(WAVE_MeshMaxZ + WATER_SeamSizeZ, WATER_MeshMaxZ),
		WATER_InfinityZ,
	};
	const float &umin = WATER_TexMinU, &umax = WATER_TexMaxU;
	const float &vmin = WATER_TexMinV, &vmax = WATER_TexMaxV;
	float Ulo[4];
	float Vlo[4];
	float Uhi[4];
	float Vhi[4];
	float x, z;
	float u, v;
	const u_int * const vertcolor = WaterDebug.ShowFarLines ? WATER_FunkyVertexColor : WATER_VertexColor;
	u_int vertcount = 0;

	assert(
		-WATER_InfinityX < WATER_MeshMinX &&
		WATER_InfinityX > WATER_MeshMaxX &&
		-WATER_InfinityZ < WATER_MeshMinZ &&
		WATER_InfinityZ > WATER_MeshMaxZ &&
		-WATER_InfinityX < WAVE_MeshMinX &&
		WATER_InfinityX > WAVE_MeshMaxX &&
		-WATER_InfinityZ < WAVE_MeshMinZ &&
		WATER_InfinityZ > WAVE_MeshMaxZ
	);

	WATER_TextureCoord(X[1], Z[1], Ulo[1], Vlo[1]);
	WATER_TextureCoord(X[2], Z[2], Uhi[2], Vhi[2]);

	Uhi[1] = Ulo[1] - ((int) Ulo[1]) + umax - 1;
	Vhi[1] = Vlo[1] - ((int) Vlo[1]) + vmax - 1;
	Ulo[0] = Uhi[1] - (umax - umin) + 2;
	Vlo[0] = Vhi[1] - (vmax - vmin) + 2;
	Ulo[2] = Uhi[2] - ((int) Uhi[2]) + umin + 1;
	Vlo[2] = Vhi[2] - ((int) Vhi[2]) + vmin + 1;
	Uhi[3] = Ulo[2] + (umax - umin) - 2;
	Vhi[3] = Vlo[2] + (vmax - vmin) - 2;

	X[0] = X[1] - (Uhi[1] - Ulo[0]) / WATER_ScaleU;
	Z[0] = Z[1] - (Vhi[1] - Vlo[0]) / WATER_ScaleV;
	X[3] = X[2] + (Uhi[3] - Ulo[2]) / WATER_ScaleU;
	Z[3] = Z[2] + (Vhi[3] - Vlo[2]) / WATER_ScaleV;

	const float * const gridarrayx[3] = {
		X,
		WATER_MeshGridX,
		X + 2,
	};
	const float * const gridarrayulo[3] = {
		Ulo,
		WATER_MeshGridU,
		Ulo + 2,
	};
	const float * const gridarrayuhi[3] = {
		Uhi,
		WATER_MeshGridU,
		Uhi + 2,
	};
	const u_int numarrayx[3] = {
		1,
		WATER_MeshStepX,
		1,
	};
	const float * const gridarrayz[3] = {
		Z,
		WATER_MeshGridZ,
		Z + 2,
	};
	const float * const gridarrayvlo[3] = {
		Vlo,
		WATER_MeshGridV,
		Vlo + 2,
	};
	const float * const gridarrayvhi[3] = {
		Vhi,
		WATER_MeshGridV,
		Vhi + 2,
	};
	const u_int numarrayz[3] = {
		1,
		WATER_MeshStepZ,
		1,
	};

	for (int i = 0; i < 3; ++i)
	{
		const float * const gridx = gridarrayx[i];
		const float * const gridulo = gridarrayulo[i];
		const float * const griduhi = gridarrayuhi[i];
		const u_int numx = numarrayx[i];
		for (int j = 0; j < 3; ++j)
		{
			if (i == 1 && j == 1) continue;	// this is the near area

			const float * const gridz = gridarrayz[j];
			const float * const gridvlo = gridarrayvlo[j];
			const float * const gridvhi = gridarrayvhi[j];
			const u_int numz = numarrayz[j];

			for (u_int stepx = 0; stepx < numx; ++stepx)
			{
				for (u_int stepz = 0; stepz < numz; ++stepz)
				{
					// Add a pair of tris
					nglMeshWriteStrip(4);
					vertcount += 4;

					x = gridx[stepx];
					z = gridz[stepz];
					u = gridulo[stepx];
					v = gridvlo[stepz];
					WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor[0], u, v);

					x = gridx[stepx];
					z = gridz[stepz+1];
					u = gridulo[stepx];
					v = gridvhi[stepz+1];
					WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor[1], u, v);

					x = gridx[stepx+1];
					z = gridz[stepz];
					u = griduhi[stepx+1];
					v = gridvlo[stepz];
					WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor[2], u, v);

					x = gridx[stepx+1];
					z = gridz[stepz+1];
					u = griduhi[stepx+1];
					v = gridvhi[stepz+1];
					WATER_SubmitVertex(x, y, z, nx, ny, nz, vertcolor[3], u, v);
				}
			}
		}
	}

	assert(vertcount <= FarMeshNumVert);
	// Fill in any remaining vectors in scratch mesh (should be fixed later)
	while (vertcount < FarMeshNumVert)
	{
		// Add a pair of tris
		nglMeshWriteStrip(4);
		vertcount += 4;
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
		WATER_SubmitVertex(0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
}
#endif

static void WATER_FillInHorizonMesh(void)
{
	float y;
	if (WATER_InInit)
	{
		y = 0;
	}
	else
	{
		const vector3d &campos = app::inst()->get_game()->get_current_view_camera()->get_abs_position();
		y = WATER_HorizonYBias * campos.y;
	}
	float nx = 0, ny = 1, nz = 0;
	float X[4] = {
		-WATER_InfinityX,
		min(WAVE_MeshMinX - WATER_SeamSizeX, WATER_MeshMinX),
		max(WAVE_MeshMaxX + WATER_SeamSizeX, WATER_MeshMaxX),
		WATER_InfinityX,
	};
	float Z[4] = {
		-WATER_InfinityZ,
		min(WAVE_MeshMinZ - WATER_SeamSizeZ, WATER_MeshMinZ),
		max(WAVE_MeshMaxZ + WATER_SeamSizeZ, WATER_MeshMaxZ),
		WATER_InfinityZ,
	};
	float x, z;

	assert(
		-WATER_InfinityX < WATER_MeshMinX &&
		WATER_InfinityX > WATER_MeshMaxX &&
		-WATER_InfinityZ < WATER_MeshMinZ &&
		WATER_InfinityZ > WATER_MeshMaxZ &&
		-WATER_InfinityX < WAVE_MeshMinX &&
		WATER_InfinityX > WAVE_MeshMaxX &&
		-WATER_InfinityZ < WAVE_MeshMinZ &&
		WATER_InfinityZ > WAVE_MeshMaxZ
	);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (i == 1 && j == 1) continue;	// this is the near area

			// Add a pair of tris
			nglMeshWriteStrip(4);

			x = X[i];
			z = Z[j];
			nglMeshFastWriteVertexPNC(x, y, z, nx, ny, nz, WATER_HorizonColor);

			x = X[i];
			z = Z[j+1];
			nglMeshFastWriteVertexPNC(x, y, z, nx, ny, nz, WATER_HorizonColor);

			x = X[i+1];
			z = Z[j];
			nglMeshFastWriteVertexPNC(x, y, z, nx, ny, nz, WATER_HorizonColor);

			x = X[i+1];
			z = Z[j+1];
			nglMeshFastWriteVertexPNC(x, y, z, nx, ny, nz, WATER_HorizonColor);
		}
	}
}

/*
nglTextureAnim *WaterTexAnimDark;
nglTextureAnim *WaterTexAnimLight;
nglTextureAnim *WaterTexAnimHighlight;

void WATER_LoadTexAnims(void)
{
	WaterTexAnimDark = nglLoadTextureAnim(pstring("WtBx"));
	WaterTexAnimLight = nglLoadTextureAnim(pstring("WtBy"));
	WaterTexAnimHighlight = nglLoadTextureAnim(pstring("WtBz"));
}
*/

/*	Takes a point [p,q] in screen space and finds a point [x,0,z], in the xz-plane of world space,
	which maps to [p,q].
*/
static inline float WATER_ScreenToWorld(float p, float q, int i, int j, int k, float &x, float &z)
{
	const nglMatrix &m = WATER_LocalToScreen;
	float mm00, mm01, mm10, mm11, b0, b1, detmm, invdetmm;
	float ww, rr, rrr;

	mm00 = m[0][i] - p * m[0][3];	// indexing on m is col x row
	mm01 = m[2][i] - p * m[2][3];
	mm10 = m[0][j] - q * m[0][3];
	mm11 = m[2][j] - q * m[2][3];

	b0 = - ( m[3][i] - p * m[3][3] );
	b1 = - ( m[3][j] - q * m[3][3] );

	detmm = mm00 * mm11 - mm01 * mm10;
	if (detmm) invdetmm = 1 / detmm; else return -1;

	x = invdetmm * (+ mm11 * b0 - mm01 * b1);
	z = invdetmm * (- mm10 * b0 + mm00 * b1);

	rr = m[0][k] * x + m[2][k] * z + m[3][k];
	ww = m[0][3] * x + m[2][3] * z + m[3][3];

	rrr = ww!=0.0f ? rr / ww : 1000000.0f;

	return rrr;
}

static inline void WATER_UpdateMinMax(float x, float z, float &minx, float &maxx, float &minz, float &maxz)
{
	if (x < minx) minx = x;
	if (x > maxx) maxx = x;

	if (z < minz) minz = z;
	if (z > maxz) maxz = z;
}

#define SCREENCONVERT_X(a) ((a) + 2048 - nglGetScreenWidth() / 2)
#define SCREENCONVERT_Y(a) ((a) + 2048 - nglGetScreenHeight() / 2)

/*	Find the smallest rectangle in the horizontal xz-plane of the water which "covers" the screen.

	SINPLEST CASE:

	All four screen corners are covered by water.  Then just invert the world to
	screen transformation to find xz coordinates for each corner, and compute the bounding box for
	these four points.

	HARDER CASE:

	One or more screen corners is not covered by water.  Typically the horizon runs
	across the center of the screen, and the top two corners are covered by sky, rather than water.

	We detect this situation by checking the z-buffer value (not the same as the z-coordinate in
	world units).  If it is negative, the point is not covered by water.  If the z-buffer value is
	small but positive, the point is covered by water, but the water is at a great distance.  Recall
	the z-buffer values are inverted, so the larger the value, the closer to the camera the point is.

	If a corner falls under either case, we look for the horizon point along both screen edges leading
	to the corner.  By that, I mean the point which lies on the edge and has a particular z-buffer value
	(given by WATER_DepthCutoff).  We use these horizon points in the bounding box calculation in place
	of the missing corners.

	SPECIAL CASES:

	None of the corner points is covered by water and none of the horizon points exist.  This case is
	the inverse of the simple case --- the camera is pointed away from the water.  We should not bother
	drawing the near mesh at all (but that response is not yet implemented).

	None of the corner points is covered by water and the horizon points exist, but are very close
	together.  We are viewing the water almost edge on.  We may need to adjust the extents to avoid having
	the water clip out.
*/
static void WATER_FindScreenExtents(void)
{
	float p[2] = {SCREENCONVERT_X(0), SCREENCONVERT_X(nglGetScreenWidth())};
	float q[2] = {SCREENCONVERT_Y(0), SCREENCONVERT_Y(nglGetScreenHeight())};
	float pp, qq;
	float x, z;
	float maxx = -1e10, minx = 1e10, maxz = -1e10, minz = 1e10;
	float u, v;
	const float &minu = WATER_TexMinU, &maxu = WATER_TexMaxU;
	const float &minv = WATER_TexMinV, &maxv = WATER_TexMaxV;

	float signx;
	float signy;

	for (int i = 0; i < 4; ++i)
	{
		pp = p[i % 2];
		qq = q[i / 2];
		signx = (i % 2) * (-2) + 1;	// +1 if we're on the left, -1 if we're on the right
		signy = (i / 2) * (-2) + 1;	// +1 if we're on the top, -1 if we're on the bottom

		if (WATER_ScreenToWorld(pp, qq, 0, 1, 2, x, z) > 0)
		{
			WATER_UpdateMinMax(x, z, minx, maxx, minz, maxz);
		}
		else
		{
			// find the horizon point along the left/right edge of the screen
			float yyy = WATER_ScreenToWorld(pp, 0, 0, 2, 1, x, z) + WATER_HorizonPixelsY * signy;
			if (yyy >= q[0] && yyy <= q[1])
			{
WATER_ScreenToWorld(pp, yyy, 0, 1, 2, x, z);
//				verify(WATER_ScreenToWorld(pp, yyy, 0, 1, 2, x, z) > 0);
				WATER_UpdateMinMax(x, z, minx, maxx, minz, maxz);
			}
			// find the horizon point along the top/bottom edge of the screen
			float xxx = WATER_ScreenToWorld(qq, 0, 1, 2, 0, x, z) + WATER_HorizonPixelsX * signx;
			if (xxx >= p[0] && xxx <= p[1])
			{
				//verify(WATER_ScreenToWorld(xxx, qq, 0, 1, 2, x, z) > 0);
				WATER_UpdateMinMax(x, z, minx, maxx, minz, maxz);
			}
		}

// xx[i] = x;
// zz[i] = z;
	}

	// failure means camera is pointing away from water
	assert(minx > -1e10f && maxx < 1e10f && minz > -1e10f && maxz < 1e10f);

	WATER_TextureCoord(minx, minz, u, v);
	if (u < minu)
	{
		WATER_InverseTextureCoordX(minu, minx);
		minx += WATER_SeamSizeX;
	}
	else if (u > maxu)
	{
		WATER_InverseTextureCoordX(maxu, minx);
		minx += WATER_SeamSizeX;
	}
	if (v < minv)
	{
		WATER_InverseTextureCoordZ(minv, minz);
		minz += WATER_SeamSizeZ;
	}
	else if (v > maxv)
	{
		WATER_InverseTextureCoordZ(maxv, minz);
		minz += WATER_SeamSizeZ;
	}
	WATER_TextureCoord(maxx, maxz, u, v);
	if (u < minu)
	{
		WATER_InverseTextureCoordX(minu, maxx);
		maxx -= WATER_SeamSizeX;
	}
	else if (u > maxu)
	{
		WATER_InverseTextureCoordX(maxu, maxx);
		maxx -= WATER_SeamSizeX;
	}
	if (v < minv)
	{
		WATER_InverseTextureCoordZ(minv, maxz);
		maxz -= WATER_SeamSizeZ;
	}
	else if (v > maxv)
	{
		WATER_InverseTextureCoordZ(maxv, maxz);
		maxz -= WATER_SeamSizeZ;
	}

	WATER_UpdateMinMax(WAVE_MeshMinX, WAVE_MeshMinZ, minx, maxx, minz, maxz);
	WATER_UpdateMinMax(WAVE_MeshMaxX, WAVE_MeshMaxZ, minx, maxx, minz, maxz);

	WATER_MeshMinX = minx;
	WATER_MeshMaxX = maxx;
	WATER_MeshMinZ = minz;
	WATER_MeshMaxZ = maxz;

	if (WATER_MeshMinX <= -WATER_InfinityX)
	{
		WATER_MeshMinX = -WATER_InfinityX + 1;
	}
	if (WATER_MeshMaxX >= WATER_InfinityX)
	{
		WATER_MeshMaxX = WATER_InfinityX - 1;
	}
	if (WATER_MeshMinZ <= -WATER_InfinityZ)
	{
		WATER_MeshMinZ = -WATER_InfinityZ + 1;
	}
	if (WATER_MeshMaxZ >= WATER_InfinityZ)
	{
		WATER_MeshMaxZ = WATER_InfinityZ - 1;
	}

/*
nglQuad qScreen;
nglInitQuad(&qScreen);
nglSetQuadRect(&qScreen, 100, 100, 500, 400);
nglSetQuadColor( &q, NGL_RGBA32( (u_int) (0 * 255), (u_int) (.5f * 255), (u_int) (0 * 255), (u_int) (.75f * 255) ) );
nglSetQuadZ(&qScreen, 1.0 );
nglListAddQuad(&qScreen);
*/

/*
float left = m[0][0] * xx[0] + m[2][0] * zz[0] + m[3][0];
float top = m[0][1] * xx[0] + m[2][1] * zz[0] + m[3][1];
float w0 = m[0][3] * xx[0] + m[2][3] * zz[0] + m[3][3];
left /= w0;
top /= w0;
left -= 2048 - NGL_SCREEN_WIDTH / 2;
top -= 2048 - NGL_SCREEN_DEPTH / 2;

float right = m[0][0] * xx[3] + m[2][0] * zz[3] + m[3][0];
float bottom = m[0][1] * xx[3] + m[2][1] * zz[3] + m[3][1];
float w3 = m[0][3] * xx[3] + m[2][3] * zz[3] + m[3][3];
right /= w3;
bottom /= w3;
right -= 2048 - NGL_SCREEN_WIDTH / 2;
bottom -= 2048 - NGL_SCREEN_DEPTH / 2;

nglQuad qWater;
nglInitQuad(&qWater);
nglSetQuadRect(&qWater, left, top, right, bottom);
nglSetQuadColor( &q, NGL_RGBA32( (u_int) (.5f * 255), (u_int) (0 * 255), (u_int) (0 * 255), (u_int) (.75f * 255) ) );
nglSetQuadZ(&qWater, 1.0 );
nglListAddQuad(&qWater);
*/

/*
nglMaterial Mat;
memset( &Mat, 0, sizeof(Mat) );
Mat.Flags = NGLMAT_LIGHT;

u_int TestMeshID = KSNGL_CreateScratchMesh(4, &Mat, false);

// Add a pair of tris
nglMeshWriteStrip(4);

nglMeshFastWriteVertexPNC(xx[0], 0, zz[0], 0, 1, 0, 0x7fffffff);
nglMeshFastWriteVertexPNC(xx[1], 0, zz[1], 0, 1, 0, 0x7fffffff);
nglMeshFastWriteVertexPNC(xx[2], 0, zz[2], 0, 1, 0, 0x7fffffff);
nglMeshFastWriteVertexPNC(xx[3], 0, zz[3], 0, 1, 0, 0x7fffffff);

//nglVector Center = {0, 0, 0, 0};
//float Radius = 1e10;
nglMeshSetSphere(WAVE_SortCenter, WAVE_SortRadius);	// no real need for this sphere, since we will never cull this mesh
*/
}

bool WATER_GetDrawFar(void)
{
	return WaterDebug.DrawFarMesh;
}

void WATER_SetDrawFar(bool onoff)
{
	WaterDebug.DrawFarMesh = onoff;
}

#ifndef TARGET_XBOX
bool WATER_GetDrawHorizon(void)
{
	return WaterDebug.DrawHorizonMesh;
}

void WATER_SetDrawHorizon(bool onoff)
{
	WaterDebug.DrawHorizonMesh = onoff;
}
#endif

bool WATER_GetDrawNear(void)
{
	return WaterDebug.DrawNearMesh;
}

void WATER_SetDrawNear(bool onoff)
{
	WaterDebug.DrawNearMesh = onoff;
}

bool WATER_GetDrawSeam(void)
{
	return WaterDebug.DrawSeamMesh;
}

void WATER_SetDrawSeam(bool onoff)
{
	WaterDebug.DrawSeamMesh = onoff;
}

bool WATER_GetDrawWave(void)
{
	return WAVE_GetDraw();
}

void WATER_SetDrawWave(bool onoff)
{
	WAVE_SetDraw(onoff);
}

