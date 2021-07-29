#include "global.h"		// included by all files
#include "ngl.h"	// For graphics engine
#include "kshooks.h"	// For KSWhatever calls
#include "entity.h"
#include "menu.h"
#include "game.h"
#include "beachdata.h"
#include "wds.h"
#include "app.h"
#include "profiler.h"
#include "FrontEndManager.h"			// For Get/Set IGO draws
#include "conglom.h"
#include "kellyslater_controller.h"
#include "spline.h"		// For spline calculations
#include "parser.h"		// For io functions
#include "wave.h"		// For WAVE_MESHWIDTH and WAVE_MESHDEPTH
#include "water.h"		// For WATER_Tile_X, WATER_Tile_Z, and WATER_Altitude
#include "wavetex.h"	// For all WAVETEX_ calls
#include "osdevopts.h"	// For data paths
#include "ksfx.h"
#include "timer.h"		// For TIMER_GetFrameSec and TIMER_GetTotalSec
#include "blur.h"		// For BLUR_TurnOn
#include "wavedata.h"
#include "wavesound.h"	// For wSound
#include "random.h"		// For random
#include "underwtr.h"		// For random

#if defined(TARGET_PS2)
#include "ngl_ps2_internal.h"	// For nglMeshFastWriteVertex functions only!!! (dc 12/14/01)
#endif // defined(TARGET_PS2)

#if defined(TARGET_XBOX)

#define ExitHandler() _ExitHandler(__FILE__, __LINE__)
#define _ExitHandler(f,l) { fprintf(stderr, "[%s:%d] exit\n", f, l ); exit(1); }

#endif /* TARGET_XBOX JIV DEBUG */

#if defined(TARGET_GC)

#define ExitHandler() _ExitHandler(__FILE__, __LINE__)
#define _ExitHandler(f,l) { fprintf(stderr, "[%s:%d] exit\n", f, l ); exit(1); }

#endif /* TARGET_XBOX JIV DEBUG */

#define NOWARNING_UNUSED(var) void *_##var = (void *) var;

#define WAVEDIE
#ifdef DEBUG
	#if !defined(TOBY) && !defined(TARGET_GC)
		#define WAVEERROR
		#define WAVEWARN
	#endif
#endif

/*
#ifdef DAVID
#define START_PROF_TIMER_DAVID(a) START_PROF_TIMER(a)
#define STOP_PROF_TIMER_DAVID(a) STOP_PROF_TIMER(a)
#else
*/
#define START_PROF_TIMER_DAVID(a)
#define STOP_PROF_TIMER_DAVID(a)
/*
#endif
*/

#ifdef WAVEDIE
#define wavedie(a) error a
#else
#define wavedie(a)
#endif

#ifdef WAVEERROR
#define waveerror(a) debug_print a
#else
#define waveerror(a)
#endif

//#define WAVEWARN

#ifdef WAVEWARN
#define wavewarn(a) debug_print a
#else
#define wavewarn(a)
#endif

//#ifdef EVAN
//#undef wavewarn
//#undef waveerror
//#define wavewarn(a) ((void)0)
//#define waveerror(a) ((void)0)
//#endif

// Exported
float WAVE_MeshMinX = -2;
float WAVE_MeshMaxX = 2;
float WAVE_MeshMinZ = -2;
float WAVE_MeshMaxZ = 2;

u_int WAVE_MeshStepX = 50;
u_int WAVE_MeshStepZ = 30;
float WAVE_MeshGridX[WAVE_MESHSTEPMAXX+1];
float WAVE_SliceGridX[WAVE_MESHSTEPMAXX+1];
float WAVE_MeshGridZ[WAVE_MESHSTEPMAXZ+1];
float WAVE_ControlGridZ[WAVE_MESHSTEPMAXZ+1];
//u_int WAVE_HintGridX[WAVE_MESHSTEPMAXX+1];
u_int WAVE_HintGridZ[WAVE_MESHSTEPMAXZ+1];

float WAVE_TexMinU = -16;	// tiling limit for a 128 x 128 texture
float WAVE_TexMaxU = 16;
float WAVE_TexMinV = -16;
float WAVE_TexMaxV = 16;

float WAVE_ScaleU;	// texture unit change per wave space change
float WAVE_ScaleV;
static float WAVE_BaseOffsetU = 0;	// texture units for (0,0) in wave space, excluding scroll
static float WAVE_BaseOffsetV = 0;	// set so as to center wave in uv space, avoiding PS2 artifacts
float WAVE_OffsetU;	// texture units for (0,0) in wave space, including scroll
float WAVE_OffsetV;	// WAVE_Offset* = WAVE_BaseOffset* + WAVE_Shift*
static float WAVE_ShiftU;	// texture units for scroll amount
static float WAVE_ShiftV;
float WAVE_ShiftX;	// world units for scroll amount
float WAVE_ShiftZ;
//float WAVE_ScaleSpatial = 1.f;	// better to use WAVE_LocalScale below

nglMatrix WAVE_LocalToWorld;
//static float WAVE_LocalScale = 1;
nglVector WAVE_LocalScale(1, 1, 1, 1);	// has conflict with triclipping, to be resolved soon (dc 05/04/01)

float WAVE_TexAnimFrame = 0.0f;
float WAVE_TexAnimSpeed = 1.0f;

float WAVE_EmitterStartCrashX;	// these come from wave exporter
float WAVE_EmitterStartLipX;
float WAVE_EmitterStartCrestX;
float WAVE_EmitterEndX;

WaveEmitter WAVE_Emitter[WAVE_MESHSTEPMAXX+1];

u_int WAVE_AverageColor = 0;

// Chosen to draw after all underwater items, but before all nearby objects.  (dc 06/10/02)
nglVector WAVE_SortCenter(0, -1.0f, 0, 0);
float WAVE_SortRadius = (float) 100000;

// Local to this file
template <class T> inline T sq(const T &t) { return t * t; }

#if defined(TARGET_PS2)
#define WAVE_TILELIMIT 2048
#elif defined(TARGET_GC)
	// anything above this causes stretching -EO
#define WAVE_TILELIMIT 16 * 1024
#elif defined(TARGET_XBOX)
#define WAVE_TILELIMIT 64 * 1024	// High quality range - 2^16; Acceptable quality range - 2^20; absolute range - 2^23
#endif

#define WAVEDATA_LOOKUP(a) (WaveDataArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].wd_type].##a)
#ifdef TARGET_XBOX
#define WAVE_TileU WAVEDATA_LOOKUP(tileu_xbox)
#define WAVE_TileV WAVEDATA_LOOKUP(tilev_xbox)
#define WAVE_ShiftSpeedPeakX WAVEDATA_LOOKUP(speedx_xbox)	// world units per second
#define WAVE_ShiftSpeedPeakZ WAVEDATA_LOOKUP(speedz_xbox)
#else
#define WAVE_TileU WAVEDATA_LOOKUP(tileu)
#define WAVE_TileV WAVEDATA_LOOKUP(tilev)
#define WAVE_ShiftSpeedPeakX WAVEDATA_LOOKUP(speedx)	// world units per second
#define WAVE_ShiftSpeedPeakZ WAVEDATA_LOOKUP(speedz)
#endif
#define WAVE_ShiftSpeedBoostU WAVEDATA_LOOKUP(boostu)
#define WAVE_ShiftSpeedBoostV WAVEDATA_LOOKUP(boostv)
static float WAVE_ShiftSpeedU;	// texture units per second
static float WAVE_ShiftSpeedV;
static float WAVE_ShiftSpeedX;	// world units per second
static float WAVE_ShiftSpeedZ;
static float WAVE_ShiftSpeedPeakU;	// texture units per second
static float WAVE_ShiftSpeedPeakV;
static bool WAVE_LeftBreaker = false;	// true = breaks to left as seen from beach

/*	Should be chosen to match well with WAVE_MeshStepZ and NGLMEM_MAX_BATCH_VERTS.
	A single profile should be composed of a whole number of strips, which means
	WAVE_MeshStepZ = K * (WAVE_StripSize / 2 - 1).  Also a VU1 batch should be composed
	of a whole number of strips, possibly plus a small unused area.  That means
	NGLMEM_MAX_BATCH_VERTS = C * WAVE_StripSize + epsilon.

	CHANGE:  Now NGL lets us make strips of arbitrary size, and automatically breaks them
	up at batch boundaries.  So we can use the maximum strip length, which is
	2 * WAVE_MeshStepZ + 2.  (dc 03/27/02)
*/
const static u_int WAVE_StripSize = 2 * WAVE_MeshStepZ + 2;
//const static u_int WAVE_MeshNumVert = WAVE_MeshStepX * WAVE_MeshStepZ * 4;
const static u_int WAVE_StripsPerProfile = (2 * WAVE_MeshStepZ) / (WAVE_StripSize - 2);
const static u_int WAVE_MeshNumVert = WAVE_MeshStepX * WAVE_StripsPerProfile * WAVE_StripSize;
const static u_int WAVE_ProfileNumVert = WAVE_MeshStepZ * 6;

static u_int WaveMeshID;

#ifdef DEBUG
#define WAVE_MAXDEBUGMESHES 32
static nglMesh *DebugMesh[WAVE_MAXDEBUGMESHES];
static nglRenderParams DebugRenderParams[WAVE_MAXDEBUGMESHES];
static nglMatrix DebugTransform[WAVE_MAXDEBUGMESHES];
static u_int WAVE_NumDebugMeshes;
#ifdef TARGET_PS2
#ifdef JWEBSTER
static float WAVE_DebugSphereScale = 30.f;
#else
static float WAVE_DebugSphereScale = 40.f;
#endif
#else
static float WAVE_DebugSphereScale = 10.f;
#endif
#endif

// Scratchpad stuff
#ifdef TARGET_PS2
/*	We can sometimes get substantial frame rate benefits by moving such items as WAVE_ProfileMetaCoeffs
	into the scratchpad.
*/
#define ScratchpadStart (0x70000000)
#define ScratchpadEnd (ScratchpadStart + 0x4000)	// 16 Kb

class WaveScratchBase {
public:
	WaveScratchBase() {}
	virtual ~WaveScratchBase() {}
	static void ResetSP(void) {sp = ScratchpadStart;}

protected:
	static u_int sp;
};
u_int WaveScratchBase::sp = ScratchpadStart;

template <class T> class WaveScratch : public WaveScratchBase {
public:
	WaveScratch() : data(*(T *) sp) {sp += sizeof(data); assert(sp <= ScratchpadEnd);}
	virtual ~WaveScratch() {sp -= sizeof(data); assert(sp >= ScratchpadStart);}

	T &GetData(void) {return data;}
	void SetData(const T &d) {memcpy(&data, &d, sizeof(data));}

private:
	T &data;
};

// Declare a variable locally which will reside in the scratchpad
#define DECLARE_SCRATCH(type, var) WaveScratch<type> var##scratch; type &var = var##scratch.GetData();

// Declare a variable at file scope which will sometimes reside in the scratchpad, but sometimes not.
// While the data is in the scratchpad, it should not be modified, as the changes will not propogate back.
#define PREPARE_SCRATCH(type, num, var) static type var##Stable[num]; static type *var = var##Stable;
#define USE_SCRATCH(type, num, var) WaveScratch<type [num]> var##scratch; var##scratch.SetData(var##Stable); \
	var = var##scratch.GetData();
#define UNUSE_SCRATCH(var) var = var##Stable;

#else	// #ifdef TARGET_PS2

class WaveScratchBase {
public:
	WaveScratchBase() {}
	virtual ~WaveScratchBase() {}
	static inline void ResetSP(void) {}
};	// dummy class

#define DECLARE_SCRATCH(type, var) type var;
#define PREPARE_SCRATCH(type, num, var) static type var##Stable[num]; static type *var = var##Stable;
#define USE_SCRATCH(type, num, var)
#define UNUSE_SCRATCH(var)

#endif	// #ifdef TARGET_PS2 #else

// Wave schedule data
#define WAVE_TYPEMAX 3
struct WaveScheduleTypeStruct {
	stringx name;
	float scale;
	float height;
};
static WaveScheduleTypeStruct WAVE_ScheduleType[WAVE_TYPEMAX];
static u_int WAVE_ScheduleTypeMax = 0;
struct WaveScheduleStruct {
	char id;			// A/B/C type for scoring purposes
	float duration;
	u_int type;			// index into WAVE_ScheduleType
	u_int wd_type;		// index into WaveDataArray
	u_int break_type;	// initial index into WAVE_BreakArray
};
#define WAVE_SCHEDULEMAX 32
static WaveScheduleStruct WAVE_ScheduleArray[WAVE_SCHEDULEMAX];
static u_int WAVE_ScheduleLength = 0;
static u_int WAVE_ScheduleIndex = 0;
static float WAVE_ScheduleProgress = 0;
static float WAVE_ScheduleTimeStart = 0;
static float WAVE_ScheduleTimeEnd = 0;
static float WAVE_MinDuration = 15;

#define WAVE_HEIGHT_FUDGEFACTOR_ARRAY_SIZE 8
static float WaveHeightFudgeFactorArray[WAVE_HEIGHT_FUDGEFACTOR_ARRAY_SIZE];

struct rgba {
	float r, g, b, a;	// ordering matches VU0 routine in WAVE_VertColor (dc 11/30/01)
#ifdef TARGET_XBOX
	float fa;	// foam alpha
#endif
}
#if !defined(TARGET_XBOX)
__attribute__((aligned(16)))
#endif // TARGET_XBOX
;
struct abgr {
	float a, b, g, r;	// GameCube color order
#ifdef TARGET_GC
	float fa;	// foam alpha
#endif
}
#if !defined(TARGET_XBOX)
__attribute__((aligned(16)))
#endif // TARGET_XBOX
;

#ifdef TARGET_GC
typedef abgr wavecolor;
#else
typedef rgba wavecolor;
#endif


// Data formats read in from .wave file
static u_int WAVE_NumProfileStandard = 15;
static u_int WAVE_NumControlStandard = 15;

struct WaveControlPoints {
	// color must come first for alignment purposes (dc 01/24/02)
	wavecolor color[WAVE_NUMPROFILEMAX][WAVE_NUMCONTROLMAX];	// opposite order from other arrays!!!
	float x[WAVE_NUMCONTROLMAX][WAVE_NUMPROFILEMAX];
	float y[WAVE_NUMCONTROLMAX][WAVE_NUMPROFILEMAX];
	float z[WAVE_NUMCONTROLMAX][WAVE_NUMPROFILEMAX];
	WaveRegionEnum region[WAVE_NUMCONTROLMAX][WAVE_NUMPROFILEMAX];
	WaveEmitterEnum emitter[WAVE_NUMCONTROLMAX][WAVE_NUMPROFILEMAX];
	u_int numcontrol, numprofile;
}
#if !defined(TARGET_XBOX)
__attribute__((aligned(16)))
#endif // TARGET_XBOX
;
static WaveControlPoints WAVE_ControlPointsArray[WAVE_TYPEMAX];
static WaveControlPoints *WAVE_ControlPoints = WAVE_ControlPointsArray;

#define WAVE_PARTITIONMAX 32
struct WavePartition {
	WavePartition(u_int N_in, float *guide_in, float *guidestep_in, float *weight_in)
		: N(N_in), guide(guide_in), guidestep(guidestep_in), weight(weight_in) {}

	u_int N;
	float *guide;
	float *guidestep;
	float *weight;
};

// Since WAVE_ControlPoints now changes at runtime (dc 01/23/02)
static float WAVE_BaseProfile[WAVE_NUMPROFILEMAX];
//static float (&WAVE_BaseProfile)[WAVE_NUMPROFILEMAX] = WAVE_ControlPoints->x[0];
static float WAVE_ProfileX[WAVE_NUMPROFILEMAX];
static float WAVE_ProfileStepX[WAVE_NUMPROFILEMAX-1] =
{	// weighting of the profiles for subdivision purposes; may need tailoring to beaches
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
static float WAVE_ProfileWeightX[WAVE_NUMPROFILEMAX-1] =
{
	1, 1, 1, 1, 1, 1, 1, 2, 2, 1.5f, 3, 2, 1, 1, 1,
};
static float WAVE_BaseProfileStep[WAVE_NUMPROFILEMAX-1];
static WavePartition WAVE_BaseProfilePartition(
	WAVE_NUMPROFILEMAX,
	WAVE_BaseProfile,
	WAVE_BaseProfileStep,
	WAVE_ProfileWeightX
);
static float WAVE_SubdivideBoostX = 2;
static float WAVE_SubdivideAbsoluteRangeX = 0.1f;
static float WAVE_SubdivideRangeX;
static float WAVE_BaseControl[WAVE_NUMCONTROLMAX];
static float WAVE_ControlZ[WAVE_NUMCONTROLMAX];
static float WAVE_ControlStepZ[WAVE_NUMCONTROLMAX-1];
static float WAVE_ControlWeightZ[WAVE_NUMCONTROLMAX-1] =
{
	1, 2, 2, 1, 2, 2, 1, 1, 2, 2, 4, 0.5f, 0.5f, 1, 1,
};
static WavePartition WAVE_BaseControlPartition(
	WAVE_NUMCONTROLMAX,
	WAVE_ControlZ,
	WAVE_ControlStepZ,
	WAVE_ControlWeightZ
);
static float WAVE_SubdivideBoostZ = 3;
static float WAVE_SubdivideAbsoluteRangeZ = 0.2f;
static float WAVE_SubdivideRangeZ;

/*	World units are

		[WAVE_MeshMinX,WAVE_MeshMaxX] X
		[WAVE_MeshMinZ,WAVE_MeshMaxZ]

	Control/Profile units are

		[WAVE_MeshMinX,WAVE_MeshMaxX] X
		[0,WAVE_ControlPoints->numprofile-1]
*/
#define WAVE_PROFILEMINX WAVE_ProfileX[0]
#define WAVE_PROFILEMAXX WAVE_ProfileX[WAVE_ControlPoints->numprofile-1]
#define WAVE_PROFILEWIDTH (WAVE_PROFILEMAXX - WAVE_PROFILEMINX)
#define WAVE_CONTROLMINZ WAVE_ControlZ[0]
#define WAVE_CONTROLMAXZ WAVE_ControlZ[WAVE_ControlPoints->numcontrol-1]
#define WAVE_CONTROLDEPTH (WAVE_CONTROLMAXZ - WAVE_CONTROLMINZ)

//static float WAVE_ScaleTime = 0.0001f;
static nglVector WAVE_LocalOffset(0, 0, 0, 0);
static nglVector WAVE_LocalRotation(0, 0, 0, 0);

#ifdef DEBUG
static float WAVE_MixMargin = 0.01f;	// at 0.01, basically just maps the edges of the mesh to 0
#endif

#ifdef DEBUG
#undef USEINFO2
#define USEINFO2(emitter, color) COLOR_##color,
static u_int WAVE_EmitterColor[WAVE_EM_MAX] = {
#include "waveemitter.txt"
};
#endif

static float WAVE_EmitterZ;
static float WAVE_EmitterCrestZ;
static int WAVE_EmitterHint;
static int WAVE_EmitterCrestHint;

#ifdef DEBUG
#undef USEINFO
#define USEINFO(name) #name,
static char *WAVE_StageName[WAVE_StageMax] = {
#include "wavestage.txt"
};
#endif

static WaveStageEnum WAVE_Stage;
static float WAVE_StageTimer;
static float WAVE_StageProgress;
static float WAVE_DurationBuilding = 5;	// time (in seconds)
static float WAVE_DurationSubsiding = 5;	// time (in seconds)
static float WAVE_StageDuration[WAVE_StageMax] = {0};	// Eventually, we may read this from the break map
static float WAVE_StageStart[WAVE_StageMax] = {0};

/*	Simple perturbations:  shift in z-direction and scale in y-direction.
*/
#define WAVE_DepthPerturbAmp WAVEDATA_LOOKUP(d_amp)
#define WAVE_DepthPerturbFreq WAVEDATA_LOOKUP(d_freq)
#define WAVE_HeightPerturbAmp WAVEDATA_LOOKUP(h_amp)
#define WAVE_HeightPerturbFreq WAVEDATA_LOOKUP(h_freq)

#ifdef DEBUG
static float WAVE_DrawnProfileX = 0;
static float WAVE_DrawnProfileXX = 0;
#endif

static float WAVE_MeshMinV;
static float WAVE_CrashV;
static float WAVE_CrestV;
static float WAVE_MeshMaxV;
static float WAVE_VTwistCrashV;
static float WAVE_VTwistCrestV;
static float WAVE_VTwistScale = 1.f;
static float WAVE_VTwistSlopeMinCrash;
static float WAVE_VTwistSlopeCrashCrest;
static float WAVE_VTwistSlopeCrestMax;

// For grind path
static float WAVE_GrindGridX[WAVE_NUMPROFILEMAX];
static SplineCoeffs<WAVE_NUMPROFILEMAX> WAVE_GrindCoeffs;
static u_int WAVE_NumGrindCoeffs;
static vector3d WAVE_GrindPath[WAVE_MESHSTEPMAXX+1];
#ifdef DEBUG
static const u_int &WAVE_GrindStepX = WAVE_MeshStepX;
#endif

// Colors
#include "watercolors.h"


#undef USEINFO2
#define USEINFO2(region, color) COLOR_##color,
static const u_int WAVE_RegionColor[WAVE_REGIONMAX] = {
#include "waveregion.txt"
};

/*
static u_int WAVE_VertexColor[4] = {
	COLOR_GRAY,
	COLOR_GRAY,
	COLOR_GRAY,
	COLOR_GRAY,
};
*/
static const u_int WAVE_FunkyVertexColor[4] = {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_RED,
};
//static u_int WAVE_WriteableVertexColor[4] = {0};

#ifdef DEBUG
static u_int WAVE_DrawnProfileColor[6] = {
	COLOR_YELLOW,
	COLOR_BLACK,
	COLOR_BLACK,
	COLOR_YELLOW,
	COLOR_YELLOW,
	COLOR_BLACK,
};
#endif

#if defined(TARGET_XBOX)
#ifdef DEBUG
struct WaveMarker {
	WaveMarker( float _x, float _z, const char *_name, u_int _color ) :x(_x), z(_z), name(_name), color(_color) {  }
	const float x, z;
	const char *name;
	const u_int color;
	vector3d pt;	// these non-const's come last
	float profilex, meshx, meshz;
};
#else	// #ifdef DEBUG
struct WaveMarker {
	WaveMarker( float _x, float _z ) :x(_x), z(_z) {  }
	const float x, z;
	vector3d pt;	// these non-const's come last
	float profilex, meshx, meshz;
};
#endif	// #ifdef DEBUG #else
#elif defined(TARGET_GC)
struct WaveMarker {
	float x, z;
#ifdef DEBUG
	char name[64];
	u_int color;
#endif
	vector3d pt;	// these non-const's come last
	float profilex, meshx, meshz;
};

#else
struct WaveMarker {
	const float x, z;
#ifdef DEBUG
	const char name[64];
	const u_int color;
#endif

	vector3d pt;	// these non-const's come last
	float profilex, meshx, meshz;
};
#endif /* TARGET_XBOX JIV DEBUG */

#undef USEINFO


#if defined(TARGET_XBOX)
#ifdef DEBUG
#define USEINFO(x, z, name, color) WaveMarker(x, z, #name, COLOR_##color),
#else
#define USEINFO(x, z, name, color) WaveMarker(x, z),
#endif
#else
#ifdef DEBUG
#define USEINFO(x, z, name, color) {x, z, #name, COLOR_##color},
#else
#define USEINFO(x, z, name, color) {x, z},
#endif
#endif /* TARGET_XBOX JIV DEBUG */

#ifdef TARGET_GC
static WaveMarker WAVE_Marker[WAVE_MarkerMax];
#else
static WaveMarker WAVE_Marker[WAVE_MarkerMax] = {
#include "wavemarker.txt"
};
#endif
static bool WAVE_MarkerInitComplete = false;

#undef USEINFO
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFO(item, type, lo, hi, step, init, menu) USEINFO##type(item)
#define USEINFOINT(item) int item;
#define USEINFOFLOAT(item) float item;
#ifndef TARGET_GC
// I hate metrowerks I hate metrowerks I hate metrowerks I hate metrowerks
// I hate metrowerks I hate metrowerks I hate metrowerks I hate metrowerks
// I hate metrowerks I hate metrowerks I hate metrowerks I hate metrowerks
static
#endif
struct WaveDebugStruct
{
#include "wavedebug.txt"
}
#undef USEINFO
#define USEINFO(item, type, lo, hi, step, init, menu) init,
WaveDebug =
{
#include "wavedebug.txt"
};

/*	We will perturb the wave by changing the profile spacing between sections
	by the following amounts.  These amounts are relative to the usual spacing between
	profiles, which we take to be 1.  So the array {0.0, 0.5, 1.0} indicates that a certain
	profile spacing is 1.5 instead of 1, and the adjacent profile spacing is also 1.5
	instead of 1.  The perturbation is gradually applied to all sections of the wave,
	after which point the wave will appear to have shifted completely over by one section
	width in the x direction.

	Sometime soon after that, the wave must gradually be brought back in frame by undoing
	the total shift while at the same time modifying the U scroll speed by a corresponding
	amount.  The forward perturbation should look like the wave is changing shape, while the
	reverse operation should be slower, and look like the camera is catching up.
*/
#undef USEINFO
#define USEINFO(item) #item,
static char *WAVE_PerturbStageName[] =
{
	USEINFO(None)
#include "wavebreakstage.txt"
};

static WavePerturbStageEnum WAVE_PerturbStage;

static WavePerturbTypeEnum WAVE_PerturbType = WAVE_PerturbTypeSurge;
static WavePerturbTypeEnum WAVE_DebugPerturbType = WAVE_PerturbTypeSurge;

#undef USEINFO
#define USEINFO(name, n, type) #name,
static char *WAVE_PerturbName[WAVE_PerturbTypeMax] = {
#include "wavebreak.txt"
};

struct WaveBreakStruct {
	WavePerturbTypeEnum type;
	float time;					// if 0, then uninitialized
	float prob[WAVE_PerturbTypeMax];	// probability of each type
};
#define WAVE_MAXBREAK 8
struct WaveBreakTypeStruct {
	stringx name;
	int numbreak;
	WaveBreakStruct breaklist[WAVE_MAXBREAK];
};
#define WAVE_MAXBREAKTYPE 8
static WaveBreakTypeStruct WAVE_BreakArray[WAVE_MAXBREAKTYPE];
static u_int WAVE_BreakTypeMax;
static WaveBreakStruct *WAVE_BreakNext;
static WaveBreakInfoStruct WAVE_BreakInfo;
static float WAVE_BreakWarningTime = 3;	// seconds before break when warning particles appear

#if defined(TARGET_XBOX)
// seems kosher, check Strousoup // JIV FIXME
struct wave_emitter_warning_t
{
	vector3d start, stop;
};

static wave_emitter_warning_t WAVE_EmitterWarningLine[WAVE_PerturbTypeMax];
#else
static struct {
	vector3d start, stop;
} WAVE_EmitterWarningLine[WAVE_PerturbTypeMax];
#endif /* TARGET_XBOX JIV DEBUG */

static bool WAVE_BreakWarn = false;
static float WAVE_BreakWarnStartX;
static float WAVE_BreakWarnStopX;
#ifdef DEBUG
static WavePerturbTypeEnum WAVE_DrawnWarningLineIndex = (WavePerturbTypeEnum) 0;
#endif

static u_int WAVE_PerturbSampleStepX = 200;	// resolution of subdivision computation

static float WAVE_PerturbProgress;	// fraction of completion of perturb stage

/*
// For perturbation type CURL
static float WAVE_PerturbCurlProfileX[] = {0, 8.1f, 8.4f, 10.8f, 11.8f, 14};	// assumes 15 profiles
static float WAVE_PerturbCurlProfileXX[] = {0, 8.1f, 8.4f, 8.4f, 11.8f, 14};
static float WAVE_PerturbCurlPulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbCurlProfileLo = 8.4f;	// assumes profiles consistent across all waves
static float WAVE_PerturbCurlProfileHi = 10.8f;
static float WAVE_PerturbCurlDuration[WAVE_PerturbStageMax] = {3, 1, 3, 0, 0, 1};
*/

// For perturbation type RUSH
static float WAVE_PerturbRushPulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbRushProfileLo = 5;	// assumes profiles consistent across all waves
static float WAVE_PerturbRushProfileHi = 10.3f;
static float WAVE_PerturbRushDuration[WAVE_PerturbStageMax] = {3, 10, 2.5f, 0, 0, 10,};

// For perturbation type BIGRUSH
static float WAVE_PerturbBigRushPulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbBigRushProfileLo = 4.5f;	// assumes profiles consistent across all waves
static float WAVE_PerturbBigRushProfileHi = 11;
static float WAVE_PerturbBigRushDuration[WAVE_PerturbStageMax] = {3, 12, 2.5f, 0, 0, 12,};

// For perturbation type SURGE
static float WAVE_PerturbSurgePulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbSurgeProfileLo = 5;	// assumes profiles consistent across all waves
static float WAVE_PerturbSurgeProfileHi = 10.3f;
static float WAVE_PerturbSurgeDuration[WAVE_PerturbStageMax] = {3, 10, 2.5f, 0, 0, 10,};

// For perturbation type BIGSURGE
static float WAVE_PerturbBigSurgePulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbBigSurgeProfileLo = 4.5f;	// assumes profiles consistent across all waves
static float WAVE_PerturbBigSurgeProfileHi = 11;
static float WAVE_PerturbBigSurgeDuration[WAVE_PerturbStageMax] = {3, 10, 2.5f, 0, 0, 12,};

// For perturbation type TONGUE
static float WAVE_PerturbTongueProfileX[] = {0, 7, 9.5f, 11, 12, 14};	// assumes 15 profiles
static float WAVE_PerturbTongueProfileXX[] = {0, 7, 9.5f, 5.5f, 11.5f, 14};
static float WAVE_PerturbTonguePulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbTongueProfileLo = 4.5f;	// assumes profiles consistent across all waves
static float WAVE_PerturbTongueProfileHi = 11;
static float WAVE_PerturbTongueDuration[WAVE_PerturbStageMax] = {2, 8, 1, 8, 1, 10};

// For perturbation type BIGTONGUE
static float WAVE_PerturbBigTongueProfileX[] = {0, 7, 10, 11.5f, 12, 14};	// assumes 15 profiles
static float WAVE_PerturbBigTongueProfileXX[] = {0, 7, 10, 5.5f, 10.5f, 14};
static float WAVE_PerturbBigTonguePulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbBigTongueProfileLo = 4.7f;	// assumes profiles consistent across all waves
static float WAVE_PerturbBigTongueProfileHi = 11.5f;
static float WAVE_PerturbBigTongueDuration[WAVE_PerturbStageMax] = {2, 8, 1, 8, 1, 12};

#ifdef DEBUG
// For perturbation type TEST
static float WAVE_PerturbTestProfileX[] = {0, 14, 14, 14, 14, 14};	// assumes 15 profiles
static float WAVE_PerturbTestProfileXX[] = {0, 14, 14, 14, 14, 14};
// These three aren't really used by the test perturb ...
static float WAVE_PerturbTestPulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbTestProfileLo = 4.5f;
static float WAVE_PerturbTestProfileHi = 11;
// ... just included so it compiles
static float WAVE_PerturbTestDuration[WAVE_PerturbStageMax] = {0};	// Test is static
#endif

// For stairstep effect
static float WAVE_PerturbStairstepPulse[] = {0.0f, 0.25f, 0.5f, 0.5f, 2.75f, 3.75f};
static float WAVE_PerturbStairstepProfileLo = 7.5;	// assumes profiles consistent across all waves
static float WAVE_PerturbStairstepProfileHi = 8.7;
static float WAVE_PerturbStairstepDuration[WAVE_PerturbStageMax] = {0};	// Stairstep is continual

// Generic perturb structures
class WaveBasePerturbClass {
public:
	WaveBasePerturbClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled)
		: duration(_duration), partition(WAVE_PARTITIONMAX, guide, guidestep, weight), enabled(_enabled) {}

	float (&duration)[WAVE_PerturbStageMax], start[WAVE_PerturbStageMax];
	WavePartition partition;

	bool Enabled(void) { return enabled; }

	virtual void Init(void) = 0;
	virtual float WorldToProfile(float worldx) = 0;

protected:
	const int &enabled;

private:
	float guide[WAVE_PARTITIONMAX];
	float guidestep[WAVE_PARTITIONMAX];
	float weight[WAVE_PARTITIONMAX];
};

template <int n> class WavePerturbClass : public WaveBasePerturbClass {
public:
	WavePerturbClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled)
		: WaveBasePerturbClass(_duration, _enabled) {}

	const static u_int num;

	virtual void Init(void) = 0;
	virtual float WorldToProfile(float worldx) = 0;
};

template <int n> const u_int WavePerturbClass<n>::num = n;

template <int n> class WavePulsePerturbClass : public WavePerturbClass<n> {
public:
	WavePulsePerturbClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePerturbClass<n>(_duration, _enabled), pulse(_pulse), profilelo(_profilelo), profilehi(_profilehi),
		offset(0) {}

	// input parameters
	const float (&pulse)[n];
	float profilelo, profilehi;

	// computed data
	float pulsex[n], pulsexx[n];
	SplineCoeffs<n> pulsecoeffs;
	float offset;

	virtual void Init(void);
	virtual float WorldToProfile(float worldx);
	virtual float WorldToPulse(float worldx);
};

template <int n> class WavePushPerturbClass : public WavePulsePerturbClass<n> {
public:
	WavePushPerturbClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePulsePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi) {}

	virtual float WorldToProfile(float worldx);
	virtual float WorldToPulse(float worldx);
};

template <int n> class WaveProfilePerturbClass : public WavePulsePerturbClass<n> {
public:
	WaveProfilePerturbClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi, float (&_profilex)[n], float (&_profilexx)[n])
	: WavePulsePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi), profilex(_profilex),
		profilexx(_profilexx), shift(0) {}

	// input parameters
	const float (&profilex)[n], (&profilexx)[n];

	// computed data
	float x[n], xx[n];
	SplineCoeffs<n> coeffs;
	float shift;

	virtual void Init(void);
	virtual float WorldToProfile(float worldx) = 0;
};

template <int n> class WavePerturbCurlClass : public WaveProfilePerturbClass<n> {
public:
	WavePerturbCurlClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi, float (&_profilex)[n], float (&_profilexx)[n])
	: WaveProfilePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi, _profilex, _profilexx) {}

	virtual float WorldToProfile(float worldx);
};

template <int n> class WavePerturbRushClass : public WavePushPerturbClass<n> {
public:
	WavePerturbRushClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePushPerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi) {}
};

template <int n> class WavePerturbBigRushClass : public WavePerturbRushClass<n> {
public:
	WavePerturbBigRushClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePerturbRushClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi) {}
};

template <int n> class WavePerturbSurgeClass : public WavePulsePerturbClass<n> {
public:
	WavePerturbSurgeClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePulsePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi) {}
};

template <int n> class WavePerturbBigSurgeClass : public WavePerturbSurgeClass<n> {
public:
	WavePerturbBigSurgeClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePerturbSurgeClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi) {}
};

template <int n> class WavePerturbTongueClass : public WaveProfilePerturbClass<n> {
public:
	WavePerturbTongueClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi, float (&_profilex)[n], float (&_profilexx)[n])
	: WaveProfilePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi, _profilex, _profilexx) {}

	virtual float WorldToProfile(float worldx);
};

template <int n> class WavePerturbBigTongueClass : public WavePerturbTongueClass<n> {
public:
	WavePerturbBigTongueClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi, float (&_profilex)[n], float (&_profilexx)[n])
	: WavePerturbTongueClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi, _profilex, _profilexx) {}
};

#undef USEINFO
#define USEINFO(name, n, type) USEINFO##type(name, n)
#define USEINFOProfile(name, n) static WavePerturb##name##Class<n> WAVE_Perturb##name( \
	WAVE_Perturb##name##Duration, \
	WaveDebug.Break##name, \
	WAVE_Perturb##name##Pulse, \
	WAVE_Perturb##name##ProfileLo, \
	WAVE_Perturb##name##ProfileHi, \
	WAVE_Perturb##name##ProfileX, \
	WAVE_Perturb##name##ProfileXX \
);
#define USEINFOPulse(name, n) static WavePerturb##name##Class<n> WAVE_Perturb##name( \
	WAVE_Perturb##name##Duration, \
	WaveDebug.Break##name, \
	WAVE_Perturb##name##Pulse, \
	WAVE_Perturb##name##ProfileLo, \
	WAVE_Perturb##name##ProfileHi \
);
#define USEINFOPush(name, n) static WavePerturb##name##Class<n> WAVE_Perturb##name( \
	WAVE_Perturb##name##Duration, \
	WaveDebug.Break##name, \
	WAVE_Perturb##name##Pulse, \
	WAVE_Perturb##name##ProfileLo, \
	WAVE_Perturb##name##ProfileHi \
);
#include "wavebreak.txt"

#undef USEINFO
#define USEINFO(name, n, type) &WAVE_Perturb##name,
static WaveBasePerturbClass *WAVE_PerturbArray[] = {
#include "wavebreak.txt"
};

static WaveBasePerturbClass *WAVE_Perturb = WAVE_PerturbArray[0];

#ifdef DEBUG
template <int n> class WavePerturbTestClass : public WaveProfilePerturbClass<n> {
public:
	WavePerturbTestClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi,	float (&_profilex)[n], float (&_profilexx)[n])
	: WaveProfilePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi, _profilex, _profilexx),
		numused(num) {}

	u_int numused;

	virtual void Init(void);
	virtual float WorldToProfile(float worldx);
};

static WavePerturbTestClass<6> WAVE_PerturbTest(WAVE_PerturbTestDuration,
	WaveDebug.BreakTest, WAVE_PerturbTestPulse, WAVE_PerturbTestProfileLo, WAVE_PerturbTestProfileHi,
	WAVE_PerturbTestProfileX, WAVE_PerturbTestProfileXX);
#endif

template <int n> class WavePerturbStairstepClass : public WavePulsePerturbClass<n> {
public:
	WavePerturbStairstepClass(float (&_duration)[WAVE_PerturbStageMax], const int &_enabled, float (&_pulse)[n],
		float _profilelo, float _profilehi)
	: WavePulsePerturbClass<n>(_duration, _enabled, _pulse, _profilelo, _profilehi) {}

	virtual float WorldToProfile(float worldx);
};

static WavePerturbStairstepClass<6> WAVE_PerturbStairstep(WAVE_PerturbStairstepDuration,
	WaveDebug.BreakStairstep, WAVE_PerturbStairstepPulse,
	WAVE_PerturbStairstepProfileLo, WAVE_PerturbStairstepProfileHi);

// wave debug menus
#include "wavemenu.cpp"

template <int n> struct SplineData2D {
	float y[n], z[n];
};

typedef SplineData2D<WAVE_NUMCONTROLMAX> WaveProfile;

static SplineCoeffs<WAVE_NUMPROFILEMAX> WAVE_ProfileToWorldCoeffs;	// transform from profile space to world space
static SplineCoeffs<WAVE_NUMCONTROLMAX> WAVE_ControlToWorldCoeffs;	// transform from control space to world space

typedef struct {
	SplineCoeffs<WAVE_NUMPROFILEMAX> y[WAVE_NUMCONTROLMAX], z[WAVE_NUMCONTROLMAX];
} WaveProfileMetaCoeffs;
PREPARE_SCRATCH(WaveProfileMetaCoeffs, 1, WAVE_ProfileMetaCoeffs);

struct WaveVertInfo {
	float x, y, z;
	float nx, ny, nz;
	u_int rgba;
	float u, v;
#ifdef TARGET_GC
	u_int fa;
#endif
#ifdef TARGET_XBOX
	float sx, sy, sz;
	float tx, ty, tz;
	float stx, sty, stz;
	char fa;
#endif
};
static WaveVertInfo WAVE_VertInfo[(WAVE_MESHSTEPMAXX+1) * (WAVE_MESHSTEPMAXZ+1)];
static u_int WAVE_VertIndex;
#ifdef DEBUG
static WaveVertInfo WAVE_ProfileVertInfo[WAVE_MESHSTEPMAXZ+1];
#endif

extern float WAVE_TexAnimFrame;

static void WAVE_Load(u_int type);
static void WAVE_LoadAll(void);
static void WAVE_MeshInit(void);
static void WAVE_OnNewWave(void);
static void WAVE_EmitterInit(void);
static void WAVE_SoundInit(void);
static void WAVE_MarkerInit(void);
static void WAVE_PerturbInit(void);
static void WAVE_PerturbNext(void);
static void WAVE_PerturbSkipBlanks(void);
static void WAVE_ResetBreaks(void);
static void WAVE_PerturbReset(float nexttime);
static void WAVE_ScheduleSync(void);
static void WAVE_StageReset(void);
static void WAVE_StageAdvance(void);
static void WAVE_ComputeProfileCoeffs(float x, float profilex, WaveProfileCoeffs &wpc, bool doperturb = true);
static void WAVE_ComputeProfileCoeffs(float x, WaveProfileCoeffs &wpc);
static void WAVE_ComputeProfileMetaCoeffs(void);
static float WAVE_ArtToProfile(float x);
static float WAVE_WorldToProfile(float x);
static inline void WAVE_Apply(const WaveProfileCoeffs &wpc, float z, u_int zhint, float &yy, float &zz);
static inline void WAVE_Apply(const WaveProfileCoeffs &wpc, float z, float &yy, float &zz);
static void WAVE_CopyPartition(const WavePartition *wpin, WavePartition *wpout);
static void WAVE_ComputeGrid(void);
static void WAVE_ComputeSlices(void);
static void WAVE_EmitterUpdate(void);
static void WAVE_SoundUpdate(void);
static void WAVE_ComputeShift(void);
static void WAVE_ComputeStage(void);
static void WAVE_ComputeVTwist(void);
static void WAVE_ComputeFoamAlphas( void );
#ifdef DEBUG
static void WAVE_TextureCoordVTwist(float x, float z, float &u, float &v);
static void WAVE_TextureCoordSimple(float x, float z, float &u, float &v);
static void (*WAVE_TextureCoord)(float x, float z, float &u, float &v);
#else
static inline void WAVE_TextureCoord(float x, float z, float &u, float &v);
#endif
static inline void WAVE_CheckForEmitter(const WaveProfileCoeffs &wpc, float xmesh, float xslice,
	WaveEmitter *we);
static void WAVE_ComputeMesh(void);
static void WAVE_FillInMesh(void);
static inline WaveRegionEnum WAVE_FindRegion(const WavePositionHint &hint);
static float WAVE_GetMarkerProfile(WaveMarkerEnum markernum);
#ifdef DEBUG
static void WAVE_FillInProfile(const u_int color[4]);
static void WAVE_DumpProfileValues(void);
static void WAVE_PrepareDebug(void);
static void WAVE_CreateDebug(void);
static void WAVE_ListAddDebug(void);
static void WAVE_DrawFloater(void);
static void WAVE_DrawLine(const vector3d &start, const vector3d &stop, u_int color = COLOR_RED, float scale = 0);
static void WAVE_DrawLines(const vector3d *pt, u_int numpt, u_int color = COLOR_RED, float scale = 0);
static void WAVE_DrawTubeThreshs(void);
static void WAVE_PrintInfoToScreen(void);
static void WAVE_FindNearestStressTest(void);
/*
static void WAVE_PrintMeshDebug(void);
*/
#ifdef TARGET_XBOX
static void WAVE_TestTexturing(void);
#endif
#endif

void WAVE_StaticInit( void )
{
	WAVEMENU_StaticInit();
}

// Independent Wave timer
float WAVE_TotalSec;
static inline float WAVE_GetFrameSec(void)
{
	return TIMER_GetFrameSec();
}

static inline float WAVE_GetTotalSec(void)
{
	return WAVE_TotalSec;
}

static inline float WAVE_GetRemainingLevelSec(void)
{
	return TIMER_IsInfiniteDuration() ? 0 : TIMER_GetRemainingLevelSec();
}

static inline float WAVE_GetRemainingScheduleSec(void)
{
	return WAVE_ScheduleTimeEnd - WAVE_GetTotalSec();
}

static void WAVE_ResetTimer(void)
{
	WAVE_TotalSec = 0;
}

KSWaterState::KSWaterState()
{
  TotalSec = 0.0f;
	//TexAnimFrame=0.0f;
	ScaleU=0.0f;
	ScaleV=0.0f;
	ShiftU=0.0f;
	ShiftV=0.0f;
	ShiftX=0.0f;
	ShiftZ=0.0f;
	ShiftSpeedU = 0.0f;
	ShiftSpeedV = 0.0f;
	Stage=0;
	PerturbStage = 0;
}

void KSWaterState::Save( void )
{
  for(int i=0; i<WAVE_StageMax; i++)
  {
    StageStart[i] = WAVE_StageStart[i];
    StageDuration[i] = WAVE_StageDuration[i];
  }
  TotalSec = WAVE_TotalSec;
	//TexAnimFrame=WAVE_TexAnimFrame;
	ScaleU=WAVE_ScaleU;
	ScaleV=WAVE_ScaleV;
	ShiftU=WAVE_ShiftU;
	ShiftV=WAVE_ShiftV;
	ShiftX=WAVE_ShiftX;
	ShiftZ=WAVE_ShiftZ;
	ShiftSpeedU = WAVE_ShiftSpeedU;
	ShiftSpeedV = WAVE_ShiftSpeedV;
	Stage=WAVE_Stage;
	PerturbStage = WAVE_PerturbStage;
	ScheduleTimeStart = WAVE_ScheduleTimeStart;
	ScheduleTimeEnd = WAVE_ScheduleTimeEnd;
}

void KSWaterState::WriteToDisk( os_file dataFile )
{
  for(int i=0; i<WAVE_StageMax; i++)
  {
	  dataFile.write((void *)&StageStart[i], sizeof(float));
	  dataFile.write((void *)&StageDuration[i], sizeof(float));
  }
	dataFile.write((void *)&TotalSec,       sizeof(float));
	//dataFile.write((void *)&TexAnimFrame,   sizeof(float));
	dataFile.write((void *)&ScaleU,         sizeof(float));
	dataFile.write((void *)&ScaleV,         sizeof(float));
	dataFile.write((void *)&ShiftU,         sizeof(float));
	dataFile.write((void *)&ShiftV,         sizeof(float));
	dataFile.write((void *)&ShiftX,         sizeof(float));
	dataFile.write((void *)&ShiftZ,         sizeof(float));
	dataFile.write((void *)&ShiftSpeedU,    sizeof(float));
	dataFile.write((void *)&ShiftSpeedV,    sizeof(float));
	dataFile.write((void *)&PerturbStage,   sizeof(int));
	dataFile.write((void *)&Stage,          sizeof(int));
	dataFile.write((void *)&ScheduleTimeStart, sizeof(int));
	dataFile.write((void *)&ScheduleTimeEnd, sizeof(int));
}


void KSWaterState::ReadFromDisk( os_file dataFile )
{
  for(int i=0; i<WAVE_StageMax; i++)
  {
	  dataFile.read((void *)&StageStart[i], sizeof(float),   0);
	  dataFile.read((void *)&StageDuration[i], sizeof(float),   0);
  }
	dataFile.read((void *)&TotalSec,       sizeof(float),   0);
	//dataFile.read((void *)&TexAnimFrame,   sizeof(float),   0);
	dataFile.read((void *)&ScaleU,         sizeof(float),   0);
	dataFile.read((void *)&ScaleV,         sizeof(float),   0);
	dataFile.read((void *)&ShiftU,         sizeof(float),   0);
	dataFile.read((void *)&ShiftV,         sizeof(float),   0);
	dataFile.read((void *)&ShiftX,         sizeof(float),   0);
	dataFile.read((void *)&ShiftZ,         sizeof(float),   0);
	dataFile.read((void *)&ShiftSpeedU,    sizeof(float),   0);
	dataFile.read((void *)&ShiftSpeedV,    sizeof(float),   0);
	dataFile.read((void *)&PerturbStage,   sizeof(int),     0);
	dataFile.read((void *)&Stage,          sizeof(int),     0);
	dataFile.read((void *)&ScheduleTimeStart, sizeof(int),     0);
	dataFile.read((void *)&ScheduleTimeEnd, sizeof(int),     0);
}


void KSWaterState::Restore( void )
{
  for(int i=0; i<WAVE_StageMax; i++)
  {
    WAVE_StageStart[i] = StageStart[i];
    WAVE_StageDuration[i] = StageDuration[i];
  }
	WAVE_TotalSec = TotalSec;
	//WAVE_TexAnimFrame=TexAnimFrame;
	WAVE_ScaleU=ScaleU;
	WAVE_ScaleV=ScaleV;
	WAVE_ShiftU=ShiftU;
	WAVE_ShiftV=ShiftV;
	WAVE_ShiftX=ShiftX;
	WAVE_ShiftZ=ShiftZ;
	WAVE_ShiftSpeedU = ShiftSpeedU;
	WAVE_ShiftSpeedV = ShiftSpeedV;
	WAVE_ScheduleTimeStart = ScheduleTimeStart;
	WAVE_ScheduleTimeEnd = ScheduleTimeEnd;
	WAVE_Stage=(WaveStageEnum) Stage;
#if defined(TARGET_XBOX)
	WAVE_PerturbStage = (WavePerturbStageEnum) int(PerturbStage);
#else
	WAVE_PerturbStage = (WavePerturbStageEnum) PerturbStage;
#endif /* TARGET_XBOX JIV DEBUG */
	WAVE_ComputeGrid();	// Find the grid of points which determine the mesh
	WAVE_ComputeShift();
	WAVE_ComputeStage();
	WAVE_ComputeVTwist();
	WAVE_ComputeSlices();	// Should come after the other computations
	WAVE_EmitterUpdate();
	WAVE_SoundUpdate();	// Should come after WAVE_ComputeSlices
	if ( particle_enable )
	{
	  ks_fx_update();
	}
	WAVE_GetBreakInfo(&WAVE_BreakInfo);
}

void KSWaterState::Pause( void )
{
}

void WAVE_Reset(void)
{
	WAVE_ShiftU = 0;
	WAVE_ShiftV = 0;
	WAVE_ShiftX = 0;
	WAVE_ShiftZ = 0;

	WAVE_ResetTimer();
	WAVE_ResetSchedule();
}

void WAVE_Init(void)
{
	#ifdef TARGET_GC
		int index=0;
		#undef USEINFO
		#ifdef DEBUG
			#define USEINFO(xx, zz, nn, cc) WAVE_Marker[index].x=xx;    \
			                                   WAVE_Marker[index].z=zz;    \
			                                   strcpy(WAVE_Marker[index].name, #nn ); \
			                                   WAVE_Marker[index].color=COLOR_##cc;    \
			                                   index++;
		#else
			#define USEINFO(xx, zz, nn, cc) WAVE_Marker[index].x=xx;    \
			                                   WAVE_Marker[index].z=zz;    \
			                                   index++;
		#endif
		#include "wavemarker.txt"

	#endif



#if defined(TARGET_PS2)
	assert(!( ((u_int) (WAVE_ControlPoints)) & (16 - 1) ));	// verify alignment
#endif

	int curbeach=g_game_ptr->get_beach_id();

	WAVE_LeftBreaker = BeachDataArray[curbeach].bdir;

	WAVETEX_LoadTextureAnims();
	WAVETEX_Init();
#ifdef TARGET_XBOX
	WAVERENDER_Init();
#endif
	WaveMeshID = WAVETEX_InitWaveMesh(WAVE_MeshNumVert, WAVETEX_MATWAVE, WAVE_FillInMesh);

	WAVE_TexMinU = - WAVE_TILELIMIT / (int) WAVETEX_Width();	// signed type
	WAVE_TexMaxU = WAVE_TILELIMIT / WAVETEX_Width();

	WAVE_TexMinV = - WAVE_TILELIMIT / (int) WAVETEX_Height();
	WAVE_TexMaxV = WAVE_TILELIMIT / WAVETEX_Height();

	WAVE_LoadAll();
	WAVE_Reset();
}

void KSWaterState::Reset( void )
{
  WAVE_Reset();
	 ks_fx_reset(); // or reset or whatever it's called
}

void WAVE_Cleanup( void )
{
	WAVETEX_FreeWaveMesh(WaveMeshID);
#ifdef TARGET_XBOX
	WAVERENDER_Cleanup();
#endif
}

//bool KSReadFile( char* FileName, nglFileBuf* File, u_int Align );
//void KSReleaseFile( nglFileBuf* File );
void UNDERWATER_Init(int beachid);

// Remove if we move all levels over to multiple waves (dc 01/23/02)
#define SUPPORT_SINGLE_WAVE

static void WAVE_Load(u_int type)
{
	// Here, we decide which of the waves to load. (dc 01/23/02)
	assert(type < countof(WAVE_ControlPointsArray));
	WAVE_ControlPoints = WAVE_ControlPointsArray + type;

	const stringx &name = WAVE_ScheduleType[type].name;
	const float &scale = WAVE_ScheduleType[type].scale;
	float maxheight = -1000.0f, minheight = 1000.0f;
	u_int i, j;

	nglFileBuf F;
	static const stringx extname(".wave");
	stringx pathname = name + extname;
#ifdef SUPPORT_SINGLE_WAVE
	stringx oldpathname = g_game_ptr->get_beach_name() + extname;
#endif
	char *s;
	int n;

	if (!KSReadFile(pathname.c_str(), &F, 1)
#ifdef SUPPORT_SINGLE_WAVE
		&& !KSReadFile(oldpathname.c_str(), &F, 1)
#endif
	)
	{
		wavewarn(("WAVE:\tCould not open wave file %s.\n", pathname.c_str()));
		wavewarn(("WAVE:\tAttempting to continue without this wave."));
		memset(WAVE_ControlPoints, 0, sizeof(*WAVE_ControlPoints));
		return;
	}

	s = (char *) F.Buf;

	#define WAVEEXP_CODE 0xadf654ad
	u_int code = 0, version = 0;
	n = sscanf(s, "%x, %d\n", &code, &version);
	if (version >= 1)	// prior to this, we didn't have a version number
	{
		assert(2 == n);
		assert(WAVEEXP_CODE == code);
		s = strchr(s, '\n') + 1;

		n = sscanf(s, "%d, %d\n", &WAVE_ControlPoints->numcontrol, &WAVE_ControlPoints->numprofile);
		assert(2 == n);
		s = strchr(s, '\n') + 1;
	}
	else
	{
		WAVE_ControlPoints->numprofile = 11;
		WAVE_ControlPoints->numcontrol = 16;
	}
	assert(WAVE_ControlPoints->numprofile <= WAVE_NUMPROFILEMAX);
	assert(WAVE_ControlPoints->numcontrol <= WAVE_NUMCONTROLMAX);
	assert(countof(WAVE_ProfileStepX) == WAVE_NUMPROFILEMAX-1);
	assert(countof(WAVE_ProfileWeightX) == WAVE_NUMPROFILEMAX-1);
	assert(countof(WAVE_ControlWeightZ) == WAVE_NUMCONTROLMAX-1);
	if (WAVE_ControlPoints->numprofile != WAVE_NumProfileStandard
		|| WAVE_ControlPoints->numcontrol != WAVE_NumControlStandard)
	{
		wavewarn(("WAVE:\tThe wave mesh %s does not have standard dimensions.\n", name.c_str()));
		wavewarn(("WAVE:\tStandard dimensions are %d X %d.  This mesh is %d X %d.",
			WAVE_NumProfileStandard, WAVE_NumControlStandard,
			WAVE_ControlPoints->numprofile, WAVE_ControlPoints->numcontrol
		));
		wavewarn(("WAVE:\tRendering should still work, but AI parameters will not be tuned correctly."));
	}

	for (j = 0; j < WAVE_ControlPoints->numprofile; ++j)
	{
		for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
		{
			int a = 0x80, r = 0x80, g = 0x80, b = 0x80;

			if ( version<2 )
			{
				n = sscanf(s, "%f, %f, %f, %d, %d\n",
					&WAVE_ControlPoints->x[i][j],
					&WAVE_ControlPoints->y[i][j],
					&WAVE_ControlPoints->z[i][j],
					(int *) &WAVE_ControlPoints->region[i][j],
					(int *) &WAVE_ControlPoints->emitter[i][j]
				);
				assert(5 == n);
			}
			else
			{
				n = sscanf(s, "%f, %f, %f, %d, %d, %d, %d, %d, %d\n",
					&WAVE_ControlPoints->x[i][j],
					&WAVE_ControlPoints->y[i][j],
					&WAVE_ControlPoints->z[i][j],
					(int *) &WAVE_ControlPoints->region[i][j],
					(int *) &WAVE_ControlPoints->emitter[i][j],
					&r,
					&g,
					&b,
					&a
				);
				assert(9 == n);
			}

			WAVE_ControlPoints->y[i][j] *= scale;	// scale wave in vertical direction

			// Keep track of lowest and highest points of wave.
			minheight = min(minheight, WAVE_ControlPoints->y[i][j]);
			maxheight = max(maxheight, WAVE_ControlPoints->y[i][j]);

			#ifdef TARGET_GC
			r-=128;
			g-=128;
			b-=128;
			r*=2;
			g*=2;
			b*=2;
			r+=128;
			g+=128;
			b+=128;
			if ( r>255 ) r=255;
			if ( g>255 ) g=255;
			if ( b>255 ) b=255;
			if (r<0) r=0;
			if (g<0) g=0;
			if (b<0) b=0;
			#endif

			WAVE_ControlPoints->color[j][i].a=a;
			WAVE_ControlPoints->color[j][i].r=r;
			WAVE_ControlPoints->color[j][i].g=g;
			WAVE_ControlPoints->color[j][i].b=b;

			assert(WAVE_ControlPoints->region[i][j] < WAVE_REGIONMAX);
			s = strchr(s, '\n') + 1;
		}
	}

	// Calculate the wave's height.
	WAVE_ScheduleType[type].height = maxheight-minheight;

	KSReleaseFile(&F);
}

static void WAVE_LoadAll(void)
{
	if (WAVE_ScheduleTypeMax == 0)
	{
		wavewarn(("WAVE:\tNo waves found in .beach file.  Adding default wave."));
		WAVE_AddToSchedule('B', "default", "default", "default", 1, 60);
	}
	for (u_int i = 0; i < WAVE_ScheduleTypeMax; ++i)
	{
		WAVE_Load(i);
	}
}

static void WAVE_TextureCoordInit(void)
{
	// We use the WaveDebug values so the tiling and scrolling can be tuned in real time (04/25/02)
	WAVE_ScaleU = (WAVE_LeftBreaker ? WaveDebug.TileU : -WaveDebug.TileU) / WAVE_MESHWIDTH;	// get the right tilt
	WAVE_ScaleV = WaveDebug.TileV / WAVE_MESHDEPTH;

	// Make the point given by WAVE_MarkerUVOrigin the origin of UV space (prior to any scrolling)
	// Requires WAVE_MarkerInit to have been called first.
	assert(WAVE_MarkerInitComplete);
	WAVE_BaseOffsetU = -WAVE_ScaleU * WAVE_Marker[WAVE_MarkerUVOrigin].meshx;
	WAVE_BaseOffsetV = -WAVE_ScaleV * WAVE_Marker[WAVE_MarkerUVOrigin].meshz;
#ifdef TARGET_PS2
	assert(WAVE_ScaleU * WAVE_MeshMinX + WAVE_BaseOffsetU >= WAVE_TexMinU + 1);
	assert(WAVE_ScaleU * WAVE_MeshMinX + WAVE_BaseOffsetU <= WAVE_TexMaxU - 1);
	assert(WAVE_ScaleU * WAVE_MeshMaxX + WAVE_BaseOffsetU >= WAVE_TexMinU + 1);
	assert(WAVE_ScaleU * WAVE_MeshMaxX + WAVE_BaseOffsetU <= WAVE_TexMaxU - 1);
	assert(WAVE_ScaleV * WAVE_MeshMinZ + WAVE_BaseOffsetV >= WAVE_TexMinV + 1);
	assert(WAVE_ScaleV * WAVE_MeshMinZ + WAVE_BaseOffsetV <= WAVE_TexMaxV - 1);
	assert(WAVE_ScaleV * WAVE_MeshMaxZ + WAVE_BaseOffsetV >= WAVE_TexMinV + 1);
	assert(WAVE_ScaleV * WAVE_MeshMaxZ + WAVE_BaseOffsetV <= WAVE_TexMaxV - 1);
#endif

	WAVE_ShiftSpeedPeakU = WaveDebug.SpeedX * WAVE_ScaleU;	// texture units per second
	WAVE_ShiftSpeedPeakV = WaveDebug.SpeedZ * WAVE_ScaleV;
}

static void WAVE_MeshInit(void)
{
	// Here, we decide which of the loaded waves is active. (dc 01/23/02)
	assert(WAVE_ScheduleArray[WAVE_ScheduleIndex].type < countof(WAVE_ControlPointsArray));
	WAVE_ControlPoints = WAVE_ControlPointsArray + WAVE_ScheduleArray[WAVE_ScheduleIndex].type;

	u_int i, j;

	WAVE_BaseProfilePartition.N = WAVE_ControlPoints->numprofile;
	WAVE_BaseControlPartition.N = WAVE_ControlPoints->numcontrol;

	for (j = 0; j < WAVE_ControlPoints->numprofile; ++j)
	{
		WAVE_BaseProfile[j] = WAVE_ControlPoints->x[0][j];
		if (WaveDebug.ConstantResolutionX)
		{
			WAVE_ProfileX[j] = WAVE_BaseProfile[j];
			if (j > 0) WAVE_ProfileStepX[j-1] = WAVE_ProfileX[j] - WAVE_ProfileX[j-1];
		}
		else
		{
			WAVE_ProfileX[j] = j ? WAVE_ProfileX[j-1] + WAVE_ProfileStepX[j-1] : 0;
		}

		if (j > 0) WAVE_BaseProfileStep[j-1] = WAVE_BaseProfile[j] - WAVE_BaseProfile[j-1];
	}

	for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
	{
		WAVE_BaseControl[i] = WAVE_ControlPoints->z[i][0];
		if (WaveDebug.ConstantResolutionZ)
		{
			WAVE_ControlZ[i] = WAVE_BaseControl[i];
		}
		else
		{
			WAVE_ControlZ[i] = i;
		}
		if (i > 0) WAVE_ControlStepZ[i-1] = WAVE_ControlZ[i] - WAVE_ControlZ[i-1];
	}

	SPLINE_ComputeCoeffs(WAVE_ProfileX, WAVE_BaseProfile, WAVE_ControlPoints->numprofile,
		WAVE_ProfileToWorldCoeffs.a,
		WAVE_ProfileToWorldCoeffs.b,
		WAVE_ProfileToWorldCoeffs.c,
		WAVE_ProfileToWorldCoeffs.d
	);
	SPLINE_ComputeCoeffs(WAVE_ControlZ, WAVE_BaseControl, WAVE_ControlPoints->numcontrol,
		WAVE_ControlToWorldCoeffs.a,
		WAVE_ControlToWorldCoeffs.b,
		WAVE_ControlToWorldCoeffs.c,
		WAVE_ControlToWorldCoeffs.d
	);

	WAVE_ComputeProfileMetaCoeffs();

/*
	WAVE_MeshMinX = WAVE_ScaleSpatial * WAVE_BaseProfile[0];
	WAVE_MeshMaxX = WAVE_ScaleSpatial * WAVE_BaseProfile[WAVE_ControlPoints->numprofile-1];
	WAVE_MeshMinZ = WAVE_ScaleSpatial * WAVE_BaseControl[0];
	WAVE_MeshMaxZ = WAVE_ScaleSpatial * WAVE_BaseControl[WAVE_ControlPoints->numcontrol-1];
*/
	WAVE_MeshMinX = WAVE_BaseProfile[0];
	WAVE_MeshMaxX = WAVE_BaseProfile[WAVE_ControlPoints->numprofile-1];
	WAVE_MeshMinZ = WAVE_BaseControl[0];
	WAVE_MeshMaxZ = WAVE_BaseControl[WAVE_ControlPoints->numcontrol-1];

	WaveDebug.SpeedX = WAVE_ShiftSpeedPeakX;
	WaveDebug.SpeedZ = WAVE_ShiftSpeedPeakZ;
	WaveDebug.TileU = WAVE_TileU;
	WaveDebug.TileV = WAVE_TileV;
	WaveDebug.BoostU = WAVE_ShiftSpeedBoostU;
	WaveDebug.BoostV = WAVE_ShiftSpeedBoostV;

	WAVE_MarkerInit();

	WAVE_TextureCoordInit();	// Must come after WAVE_MarkerInit (dc 06/26/02)

	WAVE_SubdivideRangeX = WAVE_MESHWIDTH * WAVE_SubdivideAbsoluteRangeX;
	WAVE_SubdivideRangeZ = WAVE_CONTROLDEPTH * WAVE_SubdivideAbsoluteRangeZ;

	// Otherwise we refer to the perturb data in WAVE_ComputeGrid before it's been initialized by
	// WAVE_PerturbInit.  (dc 06/30/02)
	assert(WAVE_PerturbStage == WAVE_PerturbStageNone);

	WAVE_ComputeGrid();	// Must come after setting WAVE_MeshMinX, WAVE_MeshMaxX

	WAVE_PerturbInit();	// Must come after call to WAVE_ComputeGrid (dc 09/19/01)

	// We now use the beach database rather than the wave export (dc 07/06/01)
	// We switched back to using the wave export (dc 08/04/01)
	WAVE_EmitterInit();	// must follow WAVE_ComputeGrid, WAVE_PerturbInit

	WAVE_SoundInit();

//	nglLoadTextureA("WtBx0000");
#ifndef TARGET_XBOX
	WAVE_AverageColor = WAVETEX_AverageColor(0, 0);
#endif

	int curbeach=g_game_ptr->get_beach_id();

//	BeachDataArray[curbeach].darkscale;

	UNDERWATER_Init(curbeach);
}

static void WAVE_OnNewWave(void)
{
	// Call all the OnNewWave functions.  (dc 01/25/02)
	kellyslater_controller *controller;
	controller = g_world_ptr->get_ks_controller(0);
	if (controller) controller->OnNewWave();
	controller = g_world_ptr->get_ks_controller(1);
	if (controller) controller->OnNewWave();

	if (g_beach_ptr) g_beach_ptr->OnNewWave();

	ks_fx_OnNewWave();
	for (u_int i = 0; i < WAVE_MESHSTEPMAXX+1; ++i)
	{
		WAVE_Emitter[i].type = WAVE_EM_NONE;	// else crash particles may come back before next WAVE_CREATE
	}
	WAVE_ComputeGrid();	// else WAVE_Emitters are restored before the flat, new wave is computed
	WAVE_ComputeSlices();

	wSound.OnNewWave();

	BLUR_TurnOn();
}

// Put the wave in WAVE_StageStable immediately, for the flyby.  (dc 07/08/02)
void WAVE_OnFlybyStart(void)
{
	assert(WAVE_Stage == WAVE_StageBuilding);
	WAVE_StageAdvance();
	assert(WAVE_Stage == WAVE_StageStable);
}

void WAVE_Unload(void)
{
	WAVETEX_UnloadTextureAnims();
	WATER_Cleanup();
}



extern bool particle_enable;

void WAVE_Tick(void)
{
	WAVE_TotalSec += WAVE_GetFrameSec();

	WAVE_TexAnimFrame = WAVETEX_FrameFix(0, WAVE_TexAnimFrame + WAVE_TexAnimSpeed );
	WAVE_ComputeGrid();	// Find the grid of points which determine the mesh
	WAVE_ComputeShift();
	WAVE_ComputeStage();
	WAVE_ComputeVTwist();
	WAVE_ComputeSlices();	// Should come after the other computations
	WAVE_EmitterUpdate();
	WAVE_SoundUpdate();	// Should come after WAVE_ComputeSlices
	UNDERWATER_ScrollBottom();
	if ( WaveDebug.AnimateFoam )
		WAVE_ComputeFoamAlphas();

	WAVE_GetBreakInfo(&WAVE_BreakInfo);
}

void WAVE_ReplayTick(void)
{
	WAVE_TotalSec += WAVE_GetFrameSec();

  if(ksreplay.Playspeed() >= 1.0f)
	  WAVE_TexAnimFrame = WAVETEX_FrameFix(0, WAVE_TexAnimFrame + WAVE_TexAnimSpeed );
	WAVE_ComputeGrid();	// Find the grid of points which determine the mesh
	WAVE_ComputeShift();
	WAVE_ComputeStage();
	WAVE_ComputeVTwist();
	WAVE_ComputeSlices();	// Should come after the other computations
	WAVE_EmitterUpdate();
	WAVE_SoundUpdate();	// Should come after WAVE_ComputeSlices
	UNDERWATER_ScrollBottom();
	if ( particle_enable )
	{
	  ks_fx_update();
	}

	WAVE_GetBreakInfo(&WAVE_BreakInfo);
}

void WAVE_Create( const int heroIdx )
{
	if (WaveDebug.ReloadWave)	// Only works from data, not stash, because stash has been closed.  (dc 06/11/02)
	{
		mem_lock_malloc(false);
		WAVE_LoadAll();
		WAVE_MeshInit();
		mem_lock_malloc(true);
		WaveDebug.ReloadWave = false;
	}

	// Move data into scratchpad for speed
	WaveScratchBase::ResetSP();

	USE_SCRATCH(WaveProfileMetaCoeffs, 1, WAVE_ProfileMetaCoeffs);

	// Must be advanced even if we are not drawing
	//  uhhhh... why is that?
	//WAVE_TexAnimFrame = WAVETEX_FrameFix(0, WAVE_TexAnimFrame + 1.0f);

	//nglVector Center = {0, 0, 0, 0};
	//float Radius = (float) 1e10;

	// Set up the local-to-world transform
	nglIdentityMatrix(WAVE_LocalToWorld);

	WAVE_LocalToWorld[0][0] = WAVE_LocalScale[0];
	WAVE_LocalToWorld[1][1] = WAVE_LocalScale[1];
	WAVE_LocalToWorld[2][2] = WAVE_LocalScale[2];

	KSNGL_RotateMatrix(WAVE_LocalToWorld, WAVE_LocalToWorld, WAVE_LocalRotation);
	KSNGL_TranslateMatrix(WAVE_LocalToWorld, WAVE_LocalToWorld, WAVE_LocalOffset);

#ifdef DEBUG
	// Find the spline interval for the emitter sites
	WAVE_EmitterHint = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, WAVE_EmitterZ);
	WAVE_EmitterCrestHint = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, WAVE_EmitterCrestZ);

	WAVE_TextureCoord = WaveDebug.VTwist ? WAVE_TextureCoordVTwist : WAVE_TextureCoordSimple;
#endif

#ifdef DEBUG
	WAVE_PrepareDebug();

	// Already done in WAVE_Tick, but need it here to see effect when game is paused,
	// or if we want to draw DEBUG meshes from inside.
	WAVE_ComputeGrid();	// Find the grid of points which determine the mesh
	WAVE_ComputeSlices();
#endif

	if (WaveDebug.DrawWaveMesh)
	//	(WaveDebug.DrawLightMesh || WaveDebug.DrawDarkMesh || WaveDebug.DrawHighMesh)
	//)
	{
		// These should come before WAVETEX_UseWaveMesh, otherwise the material won't be updated. (dc 06/19/02)
		WAVETEX_SetMatTextured(true, WAVETEX_MATWAVE );
//		WAVETEX_SetMatSort(USERSORT_WAVE);
		WAVETEX_SetMatBlended( WaveDebug.TranparentWave, WAVETEX_MATWAVE  );
		WAVETEX_SetMatZSorted( true, WAVETEX_MATWAVE  );
		WAVETEX_SetMatPlayerShadows( true, WAVETEX_MATWAVE  );
		WAVETEX_SetMatTranslucent( true, WAVETEX_MATWAVE  );
		WAVETEX_SetMatFoamy( true, WAVETEX_MATWAVE  );

		WAVETEX_UseWaveMesh(WaveMeshID, WAVETEX_MATWAVE);
		u_int MeshFlags = 0;
		MeshFlags |= NGLMESH_LIGHTCAT_8;	// for shadow projection (dc 06/24/02)
		if (WaveDebug.UseLights)
		{
			MeshFlags |= NGLMESH_LIGHTCAT_1;
		}
		if (WaveDebug.ScissorWaveMesh)
		{
			MeshFlags |= NGLMESH_PERFECT_TRICLIP;
		}
#if NGL > 0x010700
#warning "These debug flags won't work for a while ..."
#else
		nglSetMeshFlags(MeshFlags);
#endif
		WAVE_ComputeMesh();
		WAVE_FillInMesh();
		nglMeshSetSphere(WAVE_SortCenter, WAVE_SortRadius);	// no real need for this sphere, since we will never cull this mesh
		WAVETEX_FillWaveMesh( WaveMeshID
#ifndef TARGET_XBOX
			, WaveDebug.DrawLightMesh, WaveDebug.DrawDarkMesh, WaveDebug.DrawHighMesh, true, true
#endif
		);
		WAVETEX_CloseWaveMesh();
	}
#ifdef DEBUG
	WAVE_CreateDebug();
#endif

	// Can no longer count on our data in scratchpad being valid
	UNUSE_SCRATCH(WAVE_ProfileMetaCoeffs);
}

#ifdef DEBUG
static void WAVE_PrepareDebug(void)
{
	WAVE_NumDebugMeshes = 0;	// reset each frame

	if (WaveDebug.BreakTest)
	{
		WAVE_ComputeSlices();	// Want this to be done even when the debug menu is up
		#ifndef TARGET_GC
		WAVE_DrawnProfileX = WAVE_PerturbTest.x[WAVE_MenuPerturb->GetActiveEntry() / 2];
		WAVE_DrawnProfileXX = WAVE_PerturbTest.xx[WAVE_MenuPerturb->GetActiveEntry() / 2];
		#endif
		if (WAVE_LeftBreaker)
		{
			WAVE_DrawnProfileX = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_DrawnProfileX;
		}
	}
	else if (WaveDebug.DrawProfile)
	{
		WAVE_DrawnProfileX = WAVE_MeshGridX[WaveDebug.ProfileIndex];
		WAVE_DrawnProfileXX = WAVE_SliceGridX[WaveDebug.ProfileIndex];
	}
}

static void WAVE_CreateDebug(void)
{
	int current_wave = WAVE_GetIndex ();

	WAVE_TextureCoordInit();	// Allow tuning of texture scaling, scroll speed (dc 04/25/02)

	if (WaveDebug.BreakTest || WaveDebug.DrawProfile)
	{
		WAVE_FillInProfile(WAVE_DrawnProfileColor);
	}
	if (WaveDebug.DrawSoundEmitters)
	{
		for (u_int k = 0; k < WAVE_SE_MAX; ++k)
		{
			for (u_int i = 0; i < WAVE_SoundEmitter[k].numsegment; ++i)
			{
				WAVE_DrawLine(WAVE_SoundEmitter[k].segment[i].start,
					WAVE_SoundEmitter[k].segment[i].stop, WAVE_SoundEmitter[k].color);
			}
		}
	}
	if (WaveDebug.DrawTubeRealLine)
	{
		vector3d start(
			WAVE_MeshMinX,
			WaveDataArray[current_wave].tubecenstart_y,
			WaveDataArray[current_wave].tubecenstart_z
		);
		vector3d stop(
			WAVE_MeshMaxX,
			WaveDataArray[current_wave].tubecenstop_y,
			WaveDataArray[current_wave].tubecenstop_z
		);
		WAVE_DrawLine(start, stop, COLOR_RED, 5.0f);
	}
	if (WaveDebug.DrawTubeThreshs)
	{
		WAVE_DrawTubeThreshs();
	}
	if (WaveDebug.DrawClipLine)
	{
		vector3d start = *WAVE_GetMarker(WAVE_MarkerCurlStartTube);
		start.y += 1;
		vector3d stop = *WAVE_GetMarker(WAVE_MarkerCurlStartShoulder);
		stop.y += 1;
		WAVE_DrawLine(start, stop);
	}
	if (WaveDebug.DrawWarningLine)
	{
		WAVE_DrawLine(WAVE_EmitterWarningLine[WAVE_DrawnWarningLineIndex].start,
			WAVE_EmitterWarningLine[WAVE_DrawnWarningLineIndex].stop
		);
	}
	if (WaveDebug.DrawGrindPath)
	{
		WAVE_DrawLines(WAVE_GrindPath, WAVE_GrindStepX, COLOR_RED, 1);
	}
	if (WaveDebug.DrawMarkers)
	{
		for (u_int i = 0; i < WAVE_MarkerMax; ++i)
		{
			WAVE_DrawLines(&WAVE_Marker[i].pt, 1, WAVE_Marker[i].color);
		}
	}
	if (WaveDebug.DrawGuideBase)
	{
		u_int num = WAVE_BaseProfilePartition.N;
		vector3d pt[WAVE_PARTITIONMAX];
		for (u_int i = 0; i < num; ++i)
		{
			pt[i].x = WAVE_BaseProfilePartition.guide[i];
			if (WAVE_LeftBreaker) pt[i].x = WAVE_MeshMaxX + WAVE_MeshMinX - pt[i].x;
			pt[i].y = 1;
			pt[i].z = WAVE_Marker[WAVE_MarkerInFrontOfWave].pt.z;
		}
		WAVE_DrawLines(pt, num, COLOR_WHITE, 0.5f);
	}
	if (WaveDebug.DrawGuidePerturb)
	{
		u_int num = WAVE_Perturb->partition.N;
		vector3d pt[WAVE_PARTITIONMAX];
		for (u_int i = 0; i < num; ++i)
		{
			pt[i].x = WAVE_Perturb->partition.guide[i];
			if (WAVE_LeftBreaker) pt[i].x = WAVE_MeshMaxX + WAVE_MeshMinX - pt[i].x;
			pt[i].y = 1;
			pt[i].z = WAVE_Marker[WAVE_MarkerInFrontOfWave].pt.z;
		}
		WAVE_DrawLines(pt, num, COLOR_GRAY, 0.5f);
	}
	if (WaveDebug.DrawEmitters)
	{
		vector3d pt[WAVE_EM_MAX][WAVE_MESHSTEPMAXX];
		u_int num[WAVE_EM_MAX] = {0};
		for (u_int i = 0; i < WAVE_MeshStepX; ++i)
		{
			const WaveEmitterEnum &type = WAVE_Emitter[i].type;
			if (type)
			{
				pt[type][num[type]++] = vector3d(WAVE_Emitter[i].x, WAVE_Emitter[i].y, WAVE_Emitter[i].z);
			}
		}
		for (u_int type = 1; type < WAVE_EM_MAX; ++type)
		{
			WAVE_DrawLines(pt[type], num[type], WAVE_EmitterColor[type]);
		}
	}
	if (WaveDebug.DrawProfile || WaveDebug.DrawSoundEmitters || WaveDebug.DrawTubeRealLine || WaveDebug.DrawClipLine ||
		WaveDebug.DrawWarningLine || WaveDebug.DrawGrindPath || WaveDebug.DrawMarkers
	)
	{
		particle_enable = false;	// these make it hard to see
	}
	else
	{
		particle_enable = true;
	}
	if (WaveDebug.ShowRegions || WaveDebug.ShowGridLines || WaveDebug.DrawProfile)	// must come last because alters material
	{
		WaveDebug.TranparentWave = false;
	}
	if (WaveDebug.ShowRegions)
	{
		WaveDebug.UseLights = false;
	}
	if (WaveDebug.DumpProfileValues)
	{
		WAVE_DumpProfileValues();
		WaveDebug.DumpProfileValues = 0;
	}
#ifdef TARGET_XBOX
	if (WaveDebug.TestTexturing)
	{
		WAVE_TestTexturing();
	}
#endif
}
#endif

void WAVE_ListAdd(void)
{
	nglRenderParams rp;
	rp.Flags = 0;
	rp.Flags |= NGLP_SCALE;
#ifdef TARGET_XBOX
	if (!UNDERWATER_CameraUnderwater(WAVETEX_GetPlayer()))
	{
		rp.Flags |= NGLP_REVERSE_BACKFACECULL;
	}
#endif
	rp.Scale[0] = WAVE_LocalScale[0];
	rp.Scale[1] = WAVE_LocalScale[1];
	rp.Scale[2] = WAVE_LocalScale[2];

	if (WaveDebug.DrawWaveMesh)
	{
		WAVETEX_SubmitMesh(WaveMeshID, WAVE_LocalToWorld, rp
#ifndef TARGET_XBOX
			, WaveDebug.DrawWaveMesh, WaveDebug.DrawDarkMesh, WaveDebug.DrawHighMesh, WaveDebug.DrawHighMesh, true
#endif
		);
	}

#ifdef DEBUG
	WAVE_ListAddDebug();
#endif
}

#ifdef DEBUG
static void WAVE_ListAddDebug(void)
{
	for (u_int i = 0; i < WAVE_NumDebugMeshes; ++i)
	{
		nglListAddMesh( (nglMesh *) DebugMesh[i], DebugTransform[i], &DebugRenderParams[i]);
	}
	if (WaveDebug.PrintInfo)
	{
		WAVE_PrintInfoToScreen();
	}
}
#endif

static void WAVE_EmitterInitWarnings(void)
{
	WaveProfileCoeffs wpc;
	WaveEmitter we;
	float xmesh, xslice;
	bool started;

	WaveBasePerturbClass * const OldPerturb = WAVE_Perturb;
	WavePerturbTypeEnum OldPerturbType = WAVE_PerturbType;
	WavePerturbStageEnum OldPerturbStage = WAVE_PerturbStage;
	WaveStageEnum OldStage = WAVE_Stage;
	WaveBreakInfoStruct OldBreakInfo = WAVE_BreakInfo;

	WAVE_BreakInfo.onoff = false;
	WAVE_BreakInfo.time = 1000000;
	WAVE_EmitterUpdate();

  #if defined(TARGET_XBOX) || defined(TARGET_GC)
	for (WavePerturbTypeEnum type = (WavePerturbTypeEnum) 0;
       type < WAVE_PerturbTypeMax;
       type = WavePerturbTypeEnum(type+1))
#else
	for (WavePerturbTypeEnum type = (WavePerturbTypeEnum) 0; type < WAVE_PerturbTypeMax; ++ (int &) type)
#endif /* TARGET_XBOX JIV DEBUG */
	{
		// faked being in the middle of a break effect
		WAVE_Perturb = WAVE_PerturbArray[type];
		WAVE_PerturbType = type;
		WAVE_PerturbStage = WAVE_PerturbStageHold;
		WAVE_Stage = WAVE_StageStable;

		WAVE_ComputeSlices();

		vector3d start(WAVE_MeshMaxX, 0, 0), stop(WAVE_MeshMaxX, 0, 0);
		started = false;
		for (int xstep = WAVE_MeshStepX; xstep >= 0; --xstep)
		{
			if (WAVE_LeftBreaker)
			{
				xmesh = WAVE_MeshGridX[WAVE_MeshStepX-xstep];
				xslice = WAVE_SliceGridX[WAVE_MeshStepX-xstep];
			}
			else
			{
				xmesh = WAVE_MeshGridX[xstep];
				xslice = WAVE_SliceGridX[xstep];
			}

			WAVE_ComputeProfileCoeffs(xmesh, xslice, wpc);

			WAVE_CheckForEmitter(wpc, xmesh, xslice, &we);

			if (!started)
			{
				if (we.type != WAVE_EM_NONE && we.type != WAVE_EM_CREST)
				{
					start.x = we.x;
					start.y = we.y;
					start.z = we.z;
					started = true;
				}
			}
			else
			{
				if (we.type == WAVE_EM_NONE || we.type == WAVE_EM_CREST)
				{
					stop.x = we.x;
					stop.y = we.y;
					stop.z = we.z;
					break;
				}
			}
		}

		if (WAVE_LeftBreaker)
		{
			WAVE_EmitterWarningLine[type].start = start;
			WAVE_EmitterWarningLine[type].stop = stop;
		}
		else
		{
			WAVE_EmitterWarningLine[type].start = stop;
			WAVE_EmitterWarningLine[type].stop = start;
		}
	}

	// put stuff back the way we found it
	WAVE_Perturb = OldPerturb;
	WAVE_PerturbType = OldPerturbType;
	WAVE_PerturbStage = OldPerturbStage;
	WAVE_Stage = OldStage;
	WAVE_BreakInfo = OldBreakInfo;
	WAVE_EmitterUpdate();

	WAVE_ComputeSlices();
}


static bool WAVE_RegionIsFoamy( WaveRegionEnum wre )
{
	if ( wre==WAVE_REGIONTUBE )
		return true;
	if ( wre==WAVE_REGIONCEILING )
		return true;
	//if ( wre==WAVE_REGIONWASH )
	//	return true;
	return false;
}

static float foamfadecurvex=4.0f;
static float foamfadeoffx=1.0f;
static float foamstartx=9.0f;
static float foamstopx=11.0f;

static void WAVE_ComputeFoamAlphas( void )
{
	// Hack to override transparency values from art

	// These transparency values are used for the foam on the wave only
	//   they do not get used in the transparency of the wave itself.

	int curbeach=g_game_ptr->get_beach_id();

	int putfoamintube=BeachDataArray[curbeach].foamintube;
	#ifdef TARGET_GC
		// no lip foam on GC
		if ( putfoamintube==0 )
		{
			putfoamintube=3;
		}
		else if ( putfoamintube==2 )
		{
			putfoamintube=1;
		}
	#endif

	u_int foamxlo=(u_int) FTOI(WAVE_Marker[WAVE_MarkerTubeFoamStartX].x);
	u_int foamxhi=(u_int) FTOI(WAVE_Marker[WAVE_MarkerTubeFoamEndX].x);
	u_int foamzlo=(u_int) FTOI(WAVE_Marker[WAVE_MarkerTubeFoamStartX].z);
	u_int foamzhi=(u_int) FTOI(WAVE_Marker[WAVE_MarkerTubeFoamEndX].z);

	for (u_int i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
	{
		float a=0x80;

		if ( i>=(WAVE_EmitterZ) && i <= WAVE_EmitterCrestZ )
		{
			a=0x00;
		}
		else if ( i>=(WAVE_EmitterZ-2) && i <= WAVE_EmitterCrestZ )
		{
			a=(u_int)FTOI(128.0f * (1.0f- ((WAVE_EmitterZ-i) / (9.0f)) ));
		}
		else if ( i-WAVE_EmitterCrestZ >=0 && i-WAVE_EmitterCrestZ <= 1.0f*(WAVE_EmitterCrestZ-WAVE_EmitterZ) )
		{
			a=0x00;
		}
		else
		{
			a=0x00;
		}

		for (u_int j = 0; j < WAVE_ControlPoints->numprofile; ++j)
		{
			if ( putfoamintube==2 )
			{
				a=0x80;
				if ( !WAVE_RegionIsFoamy( WAVE_ControlPoints->region[i][j] ) )
				{
					if ( i>=(WAVE_EmitterZ) && i <= WAVE_EmitterCrestZ )
					{
						a=0x00;
					}
					else if ( i>=(WAVE_EmitterZ-2) && i <= WAVE_EmitterCrestZ )
					{
						a=(u_int)FTOI(128.0f * (1.0f- ((WAVE_EmitterZ-i) / (9.0f)) ));
					}
					else if ( i-WAVE_EmitterCrestZ >=0 && i-WAVE_EmitterCrestZ <= 1.0f*(WAVE_EmitterCrestZ-WAVE_EmitterZ) )
						a=0x00;
					else
						a=0x00;
					float xdiff=fabs(j-WAVE_EmitterStartLipX) / foamfadecurvex;
					xdiff *= xdiff;
					if ( xdiff>foamfadeoffx )
					{
						a=0;
					}
					else
					{
						a *= (foamfadeoffx-xdiff);
						a /= foamfadeoffx;
					}
				}
			}
			else if ( putfoamintube==1 )
			{
					// Foam is limited to specific wave regions
				bool foaminrect=( i >= foamzlo && i<=foamzhi &&
				                  j >= foamxlo && j<=foamxhi );

				if ( foaminrect && WAVE_RegionIsFoamy( WAVE_ControlPoints->region[i][j] ) )
				{
					a=0x80;
				}
				else
					a=0x00;
			}
			else if ( putfoamintube==0 )
			{
#ifdef TARGET_XBOX
				// no lip foam on Xbox (dc 05/7/02)
				a=0;
#else
				if ( j>=foamstartx && j<=foamstopx )
				{
					a=0x80;

					//if ( i>=(WAVE_EmitterZ) && i <= WAVE_EmitterCrestZ )
					if ( i>=(WAVE_EmitterCrestZ-1) && i <= WAVE_EmitterCrestZ )
					{
						a=0x20;
					}
					else if ( i>=(WAVE_EmitterZ-2) && i <= WAVE_EmitterCrestZ )
					{
						a=(u_int)FTOI(100.0f * (1.0f- ((WAVE_EmitterZ-i) / (11.0f)) ));
					}
					else if ( i-WAVE_EmitterCrestZ >=0 && i-WAVE_EmitterCrestZ <= 1.0f*(WAVE_EmitterCrestZ-WAVE_EmitterZ) )
					{
						a=0x00;
					}
					else
					{
						a=0x00;
					}
				}
				else
				{
					a=0;
				}
				#if 0
					// Foam is placed on the lip of the wave
				float xdiff=fabs(j-WAVE_EmitterStartLipX) / foamfadecurvex;
				xdiff *= xdiff;
				if ( xdiff>foamfadeoffx )
				{
					a=0;
				}
				else
				{
					a *= (foamfadeoffx-xdiff);
					a /= foamfadeoffx;
				}
				#endif
#endif
			}
			else if ( putfoamintube==3 )
			{
				// foam off
				a=0;
			}

#if defined(TARGET_GC)
			a *= 2; a = a<255 ? a : 255; //min(a,255);	// rescale for GC
			WAVE_ControlPoints->color[j][i].fa=a;	// store as foam alpha, and then replace
#endif
#if defined(TARGET_XBOX)
			a *= 2; a = min(a,255);	// rescale for Xbox
			WAVE_ControlPoints->color[j][i].fa=a;	// store as foam alpha, and then replace

			// PS2 really computes alpha later on the VU1.  We need a placeholder for now.
			static float OpaqueForXBox = 0xff;
			static float PrettyClearForXBox = 0xd8;

// Hack for supporting old and new water texturing.  Remove soon.  (dc 04/28/02)
extern const char *WAVETEX_EnvironmentMapName;
extern const char *WAVETEX_BumpMapName;
if (nglGetTexture(WAVETEX_EnvironmentMapName) && nglGetTexture(WAVETEX_BumpMapName))
{
	PrettyClearForXBox = 0x0;
}
else
{
	PrettyClearForXBox = 0xd8;
}
			if (i >= WAVE_Marker[WAVE_MarkerInFrontOfWave].z ||
				i == 0 ||
				j == 0 ||
				j == WAVE_ControlPoints->numprofile - 1
			)
			{
				a = OpaqueForXBox;
			}
			else
			{
				a = PrettyClearForXBox;
			}
#endif
#if defined(TARGET_GC)
			// PS2 really computes alpha later on the VU1.  We need a placeholder for now.
			static float OpaqueForXBox = 0xff;
			static float PrettyClearForXBox = 0xd8;

// Hack for supporting old and new water texturing.  Remove soon.  (dc 04/28/02)
	PrettyClearForXBox = 0xd8;
			if (i >= WAVE_Marker[WAVE_MarkerInFrontOfWave].z ||
				i == 0 ||
				j == 0 ||
				j == WAVE_ControlPoints->numprofile - 1
			)
			{
				a = OpaqueForXBox;
			}
			else
			{
				a = PrettyClearForXBox;
			}
#endif
			WAVE_ControlPoints->color[j][i].a=a;
		}
	}
}


/*	Read the emitter labels from the wave export and translate them to the smaller number of
	parameters used by the code.  Conditions are:

		- All points marked SPLASH or TRICKLE must be at a single z-value
		- All points marked CREST must be at a single z-value
		- All points marked SPLASH must be at a lower x-value than all points marked TRICKLE

	Modification:  We have relaxed the restriction on CREST points being at a single z-value,
	so that we can use the CREST points to build the grind path over the top of the wave.
*/
static void WAVE_EmitterInit(void)
{
	int curbeach=g_game_ptr->get_beach_id();
	u_int splashmin = INT_MAX, splashmax = 0;
	u_int tricklemin = INT_MAX, tricklemax = 0;
	bool bInitEmitterSplashZ = false;
	bool bInitEmitterTrickleZ = false;
	bool bInitEmitterCrestZ = false;
	float (&GrindX)[WAVE_NUMPROFILEMAX] = WAVE_GrindGridX, GrindZ[WAVE_NUMPROFILEMAX];
	SplineCoeffs<WAVE_NUMPROFILEMAX> &wgc = WAVE_GrindCoeffs;
	u_int &gj = WAVE_NumGrindCoeffs;

	gj = 0;
	for (u_int j = 0; j < WAVE_ControlPoints->numprofile; ++j)
	{
		for (u_int i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
		{
			switch (WAVE_ControlPoints->emitter[i][j])
			{
			case WAVE_EM_SPLASH:
				if (splashmin > j)
				{
					splashmin = j;
				}
				if (splashmax < j)
				{
					splashmax = j;
				}
				if (bInitEmitterSplashZ || bInitEmitterTrickleZ)
				{
					assert(WAVE_EmitterZ == WAVE_ControlZ[i]);
				}
				else
				{
					WAVE_EmitterZ = WAVE_ControlZ[i];
				}
				bInitEmitterSplashZ = true;
				break;
			case WAVE_EM_TRICKLE:
				if (tricklemin > j)
				{
					tricklemin = j;
				}
				if (tricklemax < j)
				{
					tricklemax = j;
				}
				if (bInitEmitterSplashZ || bInitEmitterTrickleZ)
				{
					assert(WAVE_EmitterZ == WAVE_ControlZ[i]);
				}
				else
				{
					WAVE_EmitterZ = WAVE_ControlZ[i];
				}
				bInitEmitterTrickleZ = true;
				break;
			case WAVE_EM_CREST:
				if (bInitEmitterSplashZ && !bInitEmitterCrestZ)
				{
					WAVE_EmitterCrestZ = WAVE_ControlZ[i];	// use the first crest point above a splash
					bInitEmitterCrestZ = true;
				}
				if (gj == 0 && j != 0)
				{
					GrindX[gj] = WAVE_MeshMinX;
					GrindZ[gj] = WAVE_ControlZ[i];
					++gj;
				}
				GrindX[gj] = WAVE_BaseProfile[j];	// x value in world space
				assert(gj == 0 || GrindX[gj] > GrindX[gj-1]);
				GrindZ[gj] = WAVE_ControlZ[i];	// z value in control space
				++gj;
				break;
			default:	// not an emitter
				break;
			}
		}
	}
	assert(bInitEmitterSplashZ && bInitEmitterTrickleZ && bInitEmitterCrestZ);
	if (GrindX[gj-1] != WAVE_MeshMaxX)
	{
		GrindX[gj] = WAVE_MeshMaxX;
		GrindZ[gj] = GrindZ[gj-1];
		++gj;
	}

/*	We get these from markers now.  See below.  (dc 01/24/02)
	WAVE_EmitterStartCrashX = (splashmin == 0) ?
		WAVE_BaseProfile[splashmin] :
		(WAVE_BaseProfile[splashmin-1] + WAVE_BaseProfile[splashmin-1]) / 2;
	assert(splashmax + 1 == tricklemin);
	WAVE_EmitterStartLipX = (WAVE_BaseProfile[splashmax] + WAVE_BaseProfile[tricklemin]) / 2;
	WAVE_EmitterStartCrestX = (tricklemax == WAVE_ControlPoints->numcontrol - 1) ?
		WAVE_BaseProfile[tricklemax] :
		(WAVE_BaseProfile[tricklemax] + WAVE_BaseProfile[tricklemax+1]) / 2;
*/

	// This has to be done after WAVE_MeshMinX / WAVE_MeshMaxX are set,
	// And it's better to do it after WAVE_MarkerInit.
	if (BeachDataArray[curbeach].bdir)
	{
		WAVE_EmitterStartCrashX = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_GetMarker (WAVE_MarkerEmitterStartCrashX)->x;
		WAVE_EmitterStartLipX = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_GetMarker (WAVE_MarkerEmitterStartLipX)->x;
		WAVE_EmitterStartCrestX = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_GetMarker (WAVE_MarkerEmitterStartCrestX)->x;
		WAVE_EmitterEndX = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_GetMarker (WAVE_MarkerEmitterEndX)->x;
	}
	else
	{
		WAVE_EmitterStartCrashX = WAVE_GetMarker (WAVE_MarkerEmitterStartCrashX)->x;
		WAVE_EmitterStartLipX = WAVE_GetMarker (WAVE_MarkerEmitterStartLipX)->x;
		WAVE_EmitterStartCrestX = WAVE_GetMarker (WAVE_MarkerEmitterStartCrestX)->x;
		WAVE_EmitterEndX = WAVE_GetMarker (WAVE_MarkerEmitterEndX)->x;
	}

	WAVE_ComputeFoamAlphas();

	// Find the spline interval for the emitter sites
	WAVE_EmitterHint = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, WAVE_EmitterZ);
	WAVE_EmitterCrestHint = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, WAVE_EmitterCrestZ);

	// Set up the spline for the grind path and get the location of the crest emitters
	SPLINE_ComputeCoeffs(GrindX, GrindZ, gj, wgc.a, wgc.b, wgc.c, wgc.d);

	WAVE_EmitterInitWarnings();
}

#define NTHBIT(a) (1<<(a))
#if defined(TARGET_XBOX) || defined(TARGET_GC)
WaveSoundEmitter WAVE_SoundEmitter[WAVE_SE_MAX];
#else
WaveSoundEmitter WAVE_SoundEmitter[WAVE_SE_MAX] =
{
	{WAVE_SE_TYPEREGION, NTHBIT(WAVE_REGIONFACE), COLOR_BLUE, },
	{WAVE_SE_TYPEEMITTER, NTHBIT(WAVE_EM_SPLASH), COLOR_GREEN, },
	{WAVE_SE_TYPEREGION, NTHBIT(WAVE_REGIONTUBE), COLOR_RED, },
};
#endif /* TARGET_XBOX JIV DEBUG */

static void WAVE_SoundInit(void)
{
	u_int minx, maxx;
	u_int minz, maxz;
	bool include;

#if defined(TARGET_XBOX) || defined(TARGET_GC)
  // weird aggregate warning.  Forgot my Stroustroup today JIV FIXME
  WAVE_SoundEmitter[0].type   = WAVE_SE_TYPEREGION;
  WAVE_SoundEmitter[0].idmask = NTHBIT(WAVE_REGIONFACE);


  WAVE_SoundEmitter[1].type = WAVE_SE_TYPEEMITTER;
  WAVE_SoundEmitter[1].idmask = NTHBIT(WAVE_EM_SPLASH);

  WAVE_SoundEmitter[2].type = WAVE_SE_TYPEREGION;
	WAVE_SoundEmitter[2].idmask = NTHBIT(WAVE_REGIONTUBE);
#endif /* TARGET_XBOX JIV DEBUG */

	assert(WAVE_REGIONMAX <= 32);	// for use with bitmask
	assert(WAVE_SE_TYPEMAX <= 32);

	for (int k = 0; k < WAVE_SE_MAX; ++k)
	{
		minx = minz = INT_MAX;
		maxx = maxz = 0;

		for (u_int j = 0; j < WAVE_ControlPoints->numprofile; ++j)
		{
			for (u_int i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
			{
				switch (WAVE_SoundEmitter[k].type)
				{
				case WAVE_SE_TYPEEMITTER:
					include = (NTHBIT(WAVE_ControlPoints->emitter[i][j]) & WAVE_SoundEmitter[k].idmask);
					break;
				case WAVE_SE_TYPEREGION:
					include = (NTHBIT(WAVE_ControlPoints->region[i][j]) & WAVE_SoundEmitter[k].idmask);
					break;
				default:
					assert(0);
					return;	// else "include" is uninitialized (dc 01/29/02)
				}

				if (include)
				{
					if (minx > j)
					{
						minx = j;
					}
					if (maxx < j)
					{
						maxx = j;
					}
					if (minz > i)
					{
						minz = i;
					}
					if (maxz < i)
					{
						maxz = i;
					}
				}
			}
		}

		WAVE_SoundEmitter[k].minx = (minx == 0) ?
			WAVE_BaseProfile[minx] :
			(WAVE_BaseProfile[minx-1] + WAVE_BaseProfile[minx]) / 2;
		WAVE_SoundEmitter[k].maxx = (maxx == WAVE_ControlPoints->numprofile - 1) ?
			WAVE_BaseProfile[maxx] :
			(WAVE_BaseProfile[maxx] + WAVE_BaseProfile[maxx+1]) / 2;
		WAVE_SoundEmitter[k].minz = (minz == 0) ?
			WAVE_ControlZ[minz] :
			(WAVE_ControlZ[minz-1] + WAVE_ControlZ[minz]) / 2;
		WAVE_SoundEmitter[k].maxz = (maxz == WAVE_ControlPoints->numprofile - 1) ?
			WAVE_ControlZ[maxz] :
			(WAVE_ControlZ[maxz] + WAVE_ControlZ[maxz+1]) / 2;

//		WAVE_SoundEmitter[k].z = (WAVE_SoundEmitter[k].minz + WAVE_SoundEmitter[k].maxz) / 2;
		WAVE_SoundEmitter[k].minhintz =
			SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, WAVE_SoundEmitter[k].minz);
		WAVE_SoundEmitter[k].maxhintz =
			SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, WAVE_SoundEmitter[k].maxz);
	}
}

static void WAVE_MarkerInit(void)
{
	for (u_int i = 0; i < WAVE_MarkerMax; ++i)
	{
		const float &x = WAVE_Marker[i].x, &z = WAVE_Marker[i].z;
		float &xx = WAVE_Marker[i].pt.x, &yy = WAVE_Marker[i].pt.y, &zz = WAVE_Marker[i].pt.z;
		WaveProfileCoeffs wpc;
		float &meshx = WAVE_Marker[i].meshx, &meshz = WAVE_Marker[i].meshz;
		float &profilex = WAVE_Marker[i].profilex;

		meshx = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(x));

		profilex = meshx;

		if (WAVE_LeftBreaker)
		{
			meshx = WAVE_MeshMinX + WAVE_MeshMaxX - meshx;
		}

		WAVE_ComputeProfileCoeffs(meshx, profilex, wpc, false);

		meshz = SPLINE_Evaluate(WAVE_ControlZ,
			WAVE_ControlToWorldCoeffs.a,
			WAVE_ControlToWorldCoeffs.b,
			WAVE_ControlToWorldCoeffs.c,
			WAVE_ControlToWorldCoeffs.d,
			WAVE_ControlPoints->numcontrol, z);

		xx = meshx;
		WAVE_Apply(wpc, z, yy, zz);
	}

	WAVE_MarkerInitComplete = true;
}

static void WAVE_EmitterUpdate(void)
{
	WAVE_BreakWarn =
		(WAVE_StageStable == WAVE_Stage) &&
		(
			(WAVE_BreakInfo.onoff && WAVE_PerturbStage <= WAVE_PerturbStageHold) ||
			(!WAVE_BreakInfo.onoff && WAVE_BreakInfo.time < WAVE_BreakWarningTime)
		);
	WAVE_BreakWarnStartX = WAVE_EmitterWarningLine[WAVE_BreakInfo.type].start.x;
	WAVE_BreakWarnStopX = WAVE_EmitterWarningLine[WAVE_BreakInfo.type].stop.x;
}

static float WAVE_SoundEmitterMargin = 0.02f;
static void WAVE_SoundUpdate(void)
{
	const float margin = WAVE_MESHWIDTH * WAVE_SoundEmitterMargin;


	for (int k = 0; k < WAVE_SE_MAX; ++k)
	{
		WaveSoundEmitter &wse = WAVE_SoundEmitter[k];
		bool inout = false;
		bool maybein = false, maybeout = false;
		u_int maybestartx = (u_int) -1, maybestopx = (u_int) -1;
		u_int numsegment = 0;
		const float &minx = wse.minx;
		const float &maxx = wse.maxx;
		const float mininnerx = wse.minx + margin / 2;
		const float maxinnerx = wse.maxx - margin / 2;
		const float minouterx = wse.minx - margin / 2;
		const float maxouterx = wse.maxx + margin / 2;
		float slicex;

		/*	The "- 1" below is for convenience, since we don't check the last
			slice in WAVE_ComputeMesh.
		*/
		for (u_int i = 0; i < WAVE_MeshStepX; ++i)
		{
			slicex = WAVE_SliceGridX[i];

			/*	Is the slice within the pre-computed emitter range?  If so, we
				wait until we get past a certain margin before we begin an emitter
				segment.  Otherwise, we sometimes get segments from small fluctuations
				in the wave.
			*/
			if (!inout)
			{
				if (slicex > minx && slicex <= maxx)
				{
					if (!maybein)
					{
						maybestartx = i;
						maybein = true;
					}
				}
				else
				{
					maybein = false;
				}
				if (slicex > mininnerx && slicex <= maxinnerx)
				{
					inout = true;
					assert(maybein);
					maybein = false;
					assert(maybestartx != (u_int) -1);	// otherwise uninitialized (dc 01/29/02)
					wse.startx[numsegment] = maybestartx;
				}
			}
			else
			{
				if (slicex <= minx || slicex > maxx)
				{
					if (!maybeout)
					{
						maybestopx = i;
						maybeout = true;
					}
				}
				else
				{
					maybeout = false;
				}
				if (slicex <= minouterx || slicex > maxouterx)
				{
					inout = false;
					assert(maybeout);
					maybeout = false;
					assert(maybestopx != (u_int) -1);	// otherwise uninitialized (dc 01/29/02)
					wse.stopx[numsegment] = maybestopx;
					++numsegment;
					assert(numsegment <= WAVE_SE_LINE_MAX);
				}
			}
		}
		if (inout)
		{
			inout = false;
			maybeout = false;
			wse.stopx[numsegment] = WAVE_MeshStepX;
			++numsegment;
			assert(numsegment <= WAVE_SE_LINE_MAX);
		}
		wse.numsegment = numsegment;
	}
}

/*	Precompute the subdivision partition for each break effect.  The object is to use the subdivision
	information for the static wave to figure out where geometry is needed in the perturbed wave.

	To get this information, we first simulate being in the "Hold" stage of the perturbation, which gives
	the most representative shape.  We then compute a wave grid for this shape, at a much higher resolution
	than we use for the rendering of the wave.  The increments of this grid are computed to cover equal
	subdivision weight in the base partition --- so steps are shorter where we requested more resolution in
	the original wave.

	We then measure how many consecutive grid steps of the perturbed wave map to the same profile interval
	of the original wave.  The union of these steps becomes an interval in our partition, and the number of
	steps becomes the weight of the interval.  The size of the derived partition may be somewhat greater than
	that of the base partition, if the perturbation uses some sections of the original wave more than once.
*/
static void WAVE_PerturbInitPartitions(void)
{
	WaveBasePerturbClass * const OldPerturb = WAVE_Perturb;
	WavePerturbTypeEnum OldPerturbType = WAVE_PerturbType;
	WavePerturbStageEnum OldPerturbStage = WAVE_PerturbStage;
	WaveStageEnum OldStage = WAVE_Stage;
	WaveBreakInfoStruct OldBreakInfo = WAVE_BreakInfo;
	u_int OldPerturbStairstep = WaveDebug.BreakStairstep;

	WAVE_BreakInfo.onoff = false;
	WAVE_BreakInfo.time = 1000000;

	WaveDebug.BreakStairstep = false;

	u_int i;
	u_int xstep, xcell, xincstep;
	float x, xmesh, xinc, xslice, xweight, xtotalweight;
	bool xincchanged;

	for (i = 0, xtotalweight = 0; i < WAVE_ControlPoints->numprofile - 1; xtotalweight += WAVE_ProfileWeightX[i++]) continue;

#if defined(TARGET_XBOX) || defined(TARGET_GC)
	for (WavePerturbTypeEnum type = (WavePerturbTypeEnum) 0;
       type < WAVE_PerturbTypeMax;
       type = WavePerturbTypeEnum(type+1))
#else
	for (WavePerturbTypeEnum type = (WavePerturbTypeEnum) 0; type < WAVE_PerturbTypeMax; ++ (int &) type)
#endif /* TARGET_XBOX JIV DEBUG */
	{
		// Fake being in the middle of a break effect
		WAVE_Perturb = WAVE_PerturbArray[type];
		WAVE_PerturbType = type;
		WavePartition &partition = WAVE_PerturbArray[WAVE_PerturbType]->partition;
		WAVE_PerturbStage = WAVE_PerturbStageHold;
		WAVE_Stage = WAVE_StageStable;

		u_int &partitionstep = partition.N;
		float *&guidearray = partition.guide;
		float *&guidesteparray = partition.guidestep;
		float *&weightarray = partition.weight;

		// Build the weighted partition for the break effect
		for (
			xstep = 0, xcell = 0, xmesh = WAVE_MeshMinX, xincchanged = false, xincstep = 0,
				partitionstep = 0, guidearray[0] = WAVE_MeshMinX;
			xmesh <= WAVE_MeshMaxX;
			++xstep, ++xincstep, xmesh += xinc
		)
		{
			x = WAVE_LeftBreaker ? WAVE_MeshMaxX + WAVE_MeshMinX - xmesh : xmesh;
			xslice = WAVE_WorldToProfile(x);
			if (xstep > 0 && !(xslice >= WAVE_BaseProfile[xcell] && xslice <= WAVE_BaseProfile[xcell+1]))
			{
				xincchanged = true;
			}
			xcell = SPLINE_BinarySearch(WAVE_BaseProfile, WAVE_ControlPoints->numprofile, xslice);
			xweight = WAVE_ProfileWeightX[xcell];
			xinc = (WAVE_BaseProfileStep[xcell] * xtotalweight) / (WAVE_PerturbSampleStepX * xweight);

			if (xincchanged)
			{
				++partitionstep;
				assert(partitionstep < WAVE_PARTITIONMAX);
				guidearray[partitionstep] = xmesh;
				guidesteparray[partitionstep - 1] = guidearray[partitionstep] - guidearray[partitionstep - 1];
				weightarray[partitionstep - 1] = (float) xincstep;

				xincchanged = false;
				xincstep = 0;
			}
		}
		if (guidearray[partitionstep] > WAVE_MeshMaxX)
		{
			guidearray[partitionstep] = WAVE_MeshMaxX;
			guidesteparray[partitionstep - 1] = guidearray[partitionstep] - guidearray[partitionstep - 1];
		}
		else
		{
			++partitionstep;
			assert(partitionstep < WAVE_PARTITIONMAX);
			guidearray[partitionstep] = WAVE_MeshMaxX;
			guidesteparray[partitionstep - 1] = guidearray[partitionstep] - guidearray[partitionstep - 1];
			weightarray[partitionstep - 1] = (float) xincstep;
		}
		++partitionstep;
	}

	// put stuff back the way we found it
	WAVE_Perturb = OldPerturb;
	WAVE_PerturbType = OldPerturbType;
	WAVE_PerturbStage = OldPerturbStage;
	WAVE_Stage = OldStage;
	WAVE_BreakInfo = OldBreakInfo;
	WaveDebug.BreakStairstep = OldPerturbStairstep;
}

template <int n> void WaveProfilePerturbClass<n>::Init(void)
{
	// Set up spline
	shift = 0;
	for (u_int i = 0; i < num; ++i)
	{
		x[i] = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(profilex[i]));
		xx[i] = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(profilexx[i]));
		shift = min(shift, xx[i] - x[i]);
	}

	SPLINE_ComputeCoeffs(x, xx, num, coeffs.a, coeffs.b, coeffs.c, coeffs.d);

	WavePulsePerturbClass<n>::Init();
}

template <int n> void WavePulsePerturbClass<n>::Init(void)
{
	// Set up surge spline
	float lo = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(profilelo));
	float hi = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(profilehi));
	float range = hi - lo;

	// scale to the size of the wave
	u_int jumpindex = 0;	// index at which pulsexx repeats
	for (u_int i = 0; i < num; ++i)
	{
		pulsexx[i] = range * pulse[i];
		pulsex[i] = pulsexx[i];
		if (i > 0 && pulsexx[i] == pulsexx[i-1])
		{
			jumpindex = i;
		}
	}

	// pinch part of the profile map closed
	assert(jumpindex != 0);
	pulsex[jumpindex] += range;
	offset = hi - pulsex[jumpindex];

	SPLINE_ComputeCoeffs(pulsex, pulsexx, num, pulsecoeffs.a, pulsecoeffs.b, pulsecoeffs.c, pulsecoeffs.d);
}

#ifdef DEBUG
template <int n> void WavePerturbTestClass<n>::Init(void)
{
	// Set up spline
	for (u_int i = 0; i < numused; ++i)
	{
		x[i] = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(profilex[i]));
		xx[i] = SPLINE_Evaluate(WAVE_ProfileX,
			WAVE_ProfileToWorldCoeffs.a,
			WAVE_ProfileToWorldCoeffs.b,
			WAVE_ProfileToWorldCoeffs.c,
			WAVE_ProfileToWorldCoeffs.d,
			WAVE_ControlPoints->numprofile, WAVE_ArtToProfile(profilexx[i]));
	}

	SPLINE_ComputeCoeffs(x, xx, numused, coeffs.a, coeffs.b, coeffs.c, coeffs.d);
}
#endif

static void WAVE_PerturbInit(void)
{
	for (WavePerturbTypeEnum type = (WavePerturbTypeEnum) 0;
		type < WAVE_PerturbTypeMax;
		type = (WavePerturbTypeEnum) (type+1))
	{
		WAVE_PerturbArray[type]->Init();
	}

	WAVE_PerturbStairstep.Init();

	// Must come after the rest of the perturb init
	WAVE_PerturbInitPartitions();
}

void WAVE_ClearBreakArray(void)
{
	for (int i = 0; i < WAVE_MAXBREAKTYPE; ++i)
	{
		WAVE_BreakArray[i].numbreak = 0;
		memset(WAVE_BreakArray[i].breaklist, 0, sizeof(WAVE_BreakArray[i].breaklist));
	}

	WAVE_BreakTypeMax = 0;
}

void WAVE_AddBreak(const stringx &breakstr, const stringx &typestr, float time, float prob)
{
	// Identify type
	WavePerturbTypeEnum type;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
	for (type = (WavePerturbTypeEnum) 0;
       type < WAVE_PerturbTypeMax && typestr != WAVE_PerturbName[type];
       type = WavePerturbTypeEnum(type+1)
#else
	for (type = (WavePerturbTypeEnum) 0;
		type < WAVE_PerturbTypeMax && typestr != WAVE_PerturbName[type];
		++ (int &) type
#endif /* TARGET_XBOX JIV DEBUG */
	) continue;
	if (WAVE_PerturbTypeMax <= type)	// unrecognized break type
	{
		wavewarn(("WAVE:\tIgnoring unrecognized wave type ""%s"" (may be obsolete).\n", typestr.c_str()));
		return;
	}

	u_int break_type = WAVE_BreakTypeMax;
	if (breakstr == "default")
	{
		break_type = WAVE_CheckBreakType(breakstr);
	}
	else
	{
		for (u_int i = 0; i < WAVE_BreakTypeMax; ++i)
		{
			if (breakstr == WAVE_BreakArray[i].name)
			{
				break_type = i;
				break;
			}
		}
	}
	assert (break_type < WAVE_BreakTypeMax);
	WaveBreakStruct *breaklist = WAVE_BreakArray[break_type].breaklist;
	int &numbreak = WAVE_BreakArray[break_type].numbreak;
	float probtotal = prob;

	int i;
	assert(numbreak == WAVE_MAXBREAK || 0 == breaklist[numbreak].time);
	for (i = 0; i < numbreak && time > breaklist[i].time; ++i)
	{
		// this space left blank intentionally
	}
	assert(i < WAVE_MAXBREAK);
	if (time > breaklist[i].time)
	{
		for (int j = numbreak; j > i; --j)
		{
			breaklist[j] = breaklist[j-1];
		}
		++numbreak;
	}
	else
	{
		for (int t = 0; t < WAVE_PerturbTypeMax; ++t)
		{
			probtotal += breaklist[i].prob[t];
		}
	}
	if (probtotal > 1.01) // account for roundoff
	{
		wavewarn(("WAVE:\tTotal probability exceeds 100%% for break."));
		wavewarn(("WAVE:\tTruncating probability from %2.2f to %2.2f.", prob, prob - (probtotal - 1)));
		wavewarn(("WAVE:\tWave name = %s, Break name = %s, time = %2.2f sec.",
			WAVE_BreakArray[break_type].name.c_str(), WAVE_PerturbName[type], time));
		prob -= (probtotal - 1);
	}
	breaklist[i].prob[type] += prob;
	breaklist[i].time = time;
//	breaklist[i].type = type;	// determined later, based on prob (dc 03/20/02)
}

static void WAVE_ResetBreaks(void)
{
	// Choose break types according to preset probabilities (dc 03/20/02)
	int i, j;
	for (i = 0; i < WAVE_BreakArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].break_type].numbreak; ++i)
	{
		WaveBreakStruct *b = WAVE_BreakArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].break_type].breaklist + i;
		float r = random_r(); // random number from 0 to 1
		float cumul = 0;
		for (j = 0; j < WAVE_PerturbTypeMax; ++j)
		{
			cumul += b->prob[j];
			if (cumul > r) break;
		}
		b->type = WavePerturbTypeEnum(j);	// Could be WAVE_PerturbTypeMax, meaning "none"
	}

	WAVE_BreakNext = WAVE_BreakArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].break_type].breaklist;
	WAVE_PerturbType = WAVE_BreakNext->type;
	WAVE_PerturbSkipBlanks();
	WAVE_PerturbReset(WAVE_BreakNext->time - WAVE_GetTotalSec());
}

static void WAVE_PerturbAdvance(void)
{
	if (WAVE_PerturbStageNone == WAVE_PerturbStage && WAVE_Stage != WAVE_StageStable)
	{
		WAVE_PerturbNext();	// Skip if the wave is not yet stable (dc 03/19/02)
		return;
	}

	WAVE_PerturbProgress = 0;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
	WAVE_PerturbStage = WavePerturbStageEnum((int(WAVE_PerturbStage)+1));
#else
	++ (int &) WAVE_PerturbStage;
#endif /* TARGET_XBOX JIV DEBUG */
	(int &) WAVE_PerturbStage %= WAVE_PerturbStageMax;

	if (WAVE_PerturbStageNone == WAVE_PerturbStage)
	{
		if (WaveDebug.BreakLoop)	// If we're creating the break from the debugger
		{
			WAVE_PerturbReset(1);	// override wait time
		}
		else
		{
			WAVE_PerturbNext();
		}
	}
}

static void WAVE_PerturbNext(void)
{
	++WAVE_BreakNext;

	assert (WAVE_BreakNext - WAVE_BreakArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].break_type].breaklist
		< WAVE_MAXBREAK);

	WAVE_PerturbType = WAVE_BreakNext->type;

	WAVE_PerturbSkipBlanks();

	WAVE_PerturbReset(WAVE_BreakNext->time - WAVE_GetTotalSec());
}

static void WAVE_PerturbSkipBlanks(void)
{
	while (WAVE_PerturbType == WAVE_PerturbTypeMax)	// WAVE_PerturbTypeMax signifies "none"
	{
		++WAVE_BreakNext;

		assert (WAVE_BreakNext - WAVE_BreakArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].break_type].breaklist
			< WAVE_MAXBREAK);

		WAVE_PerturbType = WAVE_BreakNext->type;
	}
}

static void WAVE_PerturbReset(float nexttime)
{
	assert(WAVE_PerturbType < WAVE_PerturbTypeMax);	// WAVE_PerturbTypeMax signifies "none"

	WAVE_Perturb = WAVE_PerturbArray[WAVE_PerturbType];

	// Won't work for infinite duration waves if nexttime == 0.
	// That case never happens currently. (dc 06/10/02)
	WAVE_Perturb->duration[WAVE_PerturbStageNone] = nexttime ? nexttime : WAVE_GetRemainingLevelSec() + 1;

	// Set Stage start times using current time + durations
	float prevstart = WAVE_GetTotalSec(), prevduration = 0;
	for (int i = 0; i < (int) WAVE_PerturbStageMax; ++i)
	{
		prevstart = WAVE_Perturb->start[i] = prevstart + prevduration;
		prevduration = WAVE_Perturb->duration[i];
	}

	WAVE_PerturbStage = (WavePerturbStageEnum) 0;
	WAVE_PerturbProgress = 0;
}

void WAVE_EndBreak(void)
{
	if (!WaveDebug.BreakLoop)	// Not if we're creating the break from the debugger
	{
		if (WAVE_PerturbStage != WAVE_PerturbStageNone)
		{
			WAVE_PerturbNext();
		}
	}
}

void WAVE_ClearSchedule(void)
{
	WAVE_ScheduleTypeMax = 0;
	memset(WAVE_ScheduleArray, 0, sizeof(WAVE_ScheduleArray));
	WAVE_ScheduleLength = WAVE_ScheduleIndex = 0;
}

static u_int WAVE_CheckScheduleType(const stringx &typestr, float scale)
{
	u_int type = WAVE_TYPEMAX;

	// Find the right wave type.  We have a separate wave type for each combination of typestr
	// and scale.  Different values of wdstr and duration don't require new wave types.
	for (u_int i = 0; i < WAVE_ScheduleTypeMax; ++i)
	{
		if (typestr == WAVE_ScheduleType[i].name && scale == WAVE_ScheduleType[i].scale)
		{
			type = i;
			break;
		}
	}
	if (type == WAVE_TYPEMAX)
	{
		if (WAVE_ScheduleTypeMax < WAVE_TYPEMAX)
		{
			type = WAVE_ScheduleTypeMax;
			WAVE_ScheduleType[WAVE_ScheduleTypeMax].name = typestr;
			WAVE_ScheduleType[WAVE_ScheduleTypeMax].scale = scale;
			++WAVE_ScheduleTypeMax;
		}
		else
		{
			type = 0;
			wavewarn(("WAVE:\tToo many wave types in .beach file."));
			wavewarn(("WAVE:\tAt most %d variations of wave + scale are allowed.", WAVE_TYPEMAX));
			wavewarn(("WAVE:\tReplacing wave %s at scale %2.2f by wave %s at scale %2.2f.",
				typestr.c_str(), scale, WAVE_ScheduleType[0].name.c_str(), WAVE_ScheduleType[0].scale));
		}
	}

	return type;
}

u_int WAVE_CheckBreakType(const stringx &breakstr)
{
	u_int break_type = WAVE_MAXBREAKTYPE;

	// Find the right break type.
	for (u_int i = 0; i < WAVE_BreakTypeMax; ++i)
	{
		if (breakstr == WAVE_BreakArray[i].name)
		{
			break_type = i;
			break;
		}
	}
	if (break_type == WAVE_MAXBREAKTYPE)
	{
		if (WAVE_BreakTypeMax < WAVE_MAXBREAKTYPE)
		{
			break_type = WAVE_BreakTypeMax;
			WAVE_BreakArray[WAVE_BreakTypeMax].name = breakstr;
			++WAVE_BreakTypeMax;
		}
		else
		{
			break_type = 0;
			wavewarn(("WAVE:\tToo many break types in .beach file."));
			wavewarn(("WAVE:\tAt most %d different .break files are allowed."));
			wavewarn(("WAVE:\tReplacing break type %s by break type %s for purposes of wave effects.",
				breakstr.c_str(), WAVE_BreakArray[0].name.c_str()));
		}
	}

	return break_type;
}

static u_int WAVE_CheckWaveDataType(const stringx &wdstr)
{
	u_int wd_type = WAVE_LAST;

	// Find the corrent entry for this wave from WaveDataArray.
	stringx wd = wdstr;
  stringx cwd;
	bool retried = false;
retry:
	wd.to_upper();
	for (u_int i = 0; i < WAVE_LAST; ++i)
	{
    cwd = WaveDataArray[i].name;
    cwd.to_upper();
		if (wd == cwd)
		{
			wd_type = i;
			break;
		}
	}
	if (wd_type == WAVE_LAST)
	{
		if (retried)
		{
			wavewarn(("WAVE:\tCouldn't find wave named %s either.  Using first wave as default.\n", wd.c_str()));
			wd_type = 0;
		}
		else
		{
			int curbeach=g_game_ptr->get_beach_id();
			wavewarn(("WAVE:\tCouldn't find wave named %s in WaveDataArray.  Trying default name %s instead.\n",
				wd.c_str(), BeachDataArray[curbeach].name));
			wd = BeachDataArray[curbeach].name;
			retried = true;
			goto retry;
		}
	}

	return wd_type;
}

void WAVE_AddToSchedule(char id, const stringx &typestr, const stringx &breakstr, const stringx &wdstr,
	float scale, float duration)
{
	// Add to schedule
	assert (duration > 0);
	WAVE_ScheduleArray[WAVE_ScheduleLength].id = id;
	WAVE_ScheduleArray[WAVE_ScheduleLength].duration = duration;
	WAVE_ScheduleArray[WAVE_ScheduleLength].type = WAVE_CheckScheduleType(typestr, scale);
	WAVE_ScheduleArray[WAVE_ScheduleLength].break_type = WAVE_CheckBreakType(breakstr);
	WAVE_ScheduleArray[WAVE_ScheduleLength].wd_type = WAVE_CheckWaveDataType(wdstr);
	++WAVE_ScheduleLength;
}

void WAVE_ResetSchedule(void)
{
	WAVE_ScheduleIndex = 0;
	WAVE_ScheduleSync();
}

static void WAVE_ScheduleAdvance(void)
{
	if (WAVE_GetRemainingLevelSec() < 0)
	{
		return;	// don't bother; the level is about to end (dc 01/26/02)
	}

	++WAVE_ScheduleIndex;

	if (WAVE_ScheduleIndex >= WAVE_ScheduleLength)
	{
		wavewarn(("WAVE\tFinished last wave before level ended.  Restarting first wave."));
		WAVE_ScheduleIndex = 0;
	}

	if (WAVE_Stage != (WaveStageEnum)(WAVE_StageMax - 1) || WAVE_StageProgress + WAVE_GetFrameSec() < .999f)
	{
		wavewarn(("WAVE:\tWave ending without having fully subsided."));
	}

	if (WAVE_PerturbStage != (WavePerturbStageEnum) 0
		&& (
			WAVE_PerturbStage != (WavePerturbStageEnum)(WAVE_PerturbStageMax - 1)
			|| WAVE_PerturbProgress + WAVE_GetFrameSec() < .999f
		)
	)
	{
		wavewarn(("WAVE:\tWave ending in the middle of a break."));
	}

	WAVE_ScheduleSync();

		// underwater hint was getting messed up somewhere around here
	UNDERWATER_CameraReset();
}

static void WAVE_ScheduleSync(void)
{
	// Avoid infinite recursion with WAVE_StageReset.  (dc 01/26/02)
	if (WAVE_ScheduleArray[WAVE_ScheduleIndex].duration < WAVE_MinDuration)
	{
		wavewarn(("WAVE:\tWave duration is shorter than minimum allowed."));
		wavewarn(("WAVE:\tRaising duration from %2.2f to %2.2f.",
			WAVE_ScheduleArray[WAVE_ScheduleIndex].duration, WAVE_MinDuration));
		WAVE_ScheduleArray[WAVE_ScheduleIndex].duration = WAVE_MinDuration;
	}

	WAVE_ScheduleProgress = 0;
	WAVE_ScheduleTimeStart = 0;	// relative to wave timer, which is about to reset
	WAVE_ScheduleTimeEnd = WAVE_ScheduleTimeStart + WAVE_ScheduleArray[WAVE_ScheduleIndex].duration;

	WAVE_ResetTimer();	// Must come before WAVE_ResetBreaks (dc 03/19/02)
	WAVE_StageReset();
	WAVE_ResetBreaks();	// Start break map again from time 0. (dc 01/25/02)

	// This call used to be at the beginning of the function, but there were two order-of-initialization
	// problems.  WAVE_StageReset and WAVE_ResetBreaks both need to happen before WAVE_ComputeGrid, which is
	// called from WAVE_MeshInit.  Otherwise, the values of WAVE_Stage and WAVE_PerturbStage are not
	// initialized for the new wave.  That shouldn't usually be a problem, but it might be if we are
	// entering a new level and have not called WAVE_PerturbInit yet.  In that case, we could be referring
	// to outdated perturb data from the previous level.  Anyhow, there have been some crashes related to
	// this, and I'm hoping moving this line will fix them.  (dc 06/30/02)
	WAVE_MeshInit();

	WAVE_OnNewWave();
}

static void WAVE_StageReset(void)
{
	float duration = WAVE_ScheduleTimeEnd - WAVE_GetTotalSec();
	assert(duration >= WAVE_MinDuration);

	WAVE_StageDuration[WAVE_StageBuilding] = WAVE_DurationBuilding;
	WAVE_StageDuration[WAVE_StageStable] = duration - WAVE_DurationBuilding - WAVE_DurationSubsiding;
	WAVE_StageDuration[WAVE_StageSubsiding] = WAVE_DurationSubsiding;

	if (WAVE_StageDuration[WAVE_StageStable] < 0)
	{
		wavewarn(("WAVE:\tDuration of next wave is too short.  Not enough time to build / subside normally."));
		WAVE_StageDuration[WAVE_StageBuilding] = duration / 2;
		WAVE_StageDuration[WAVE_StageStable] = 0;
		WAVE_StageDuration[WAVE_StageSubsiding] = duration / 2;
	}

	// Set Stage start times using current time + durations
	WAVE_StageStart[WAVE_StageBuilding] = WAVE_GetTotalSec();
	WAVE_StageStart[WAVE_StageStable] = WAVE_StageStart[WAVE_StageBuilding] + WAVE_StageDuration[WAVE_StageBuilding];
	WAVE_StageStart[WAVE_StageSubsiding] = WAVE_StageStart[WAVE_StageStable] + WAVE_StageDuration[WAVE_StageStable];

	WAVE_Stage = (WaveStageEnum) 0;
	WAVE_StageProgress = 0;

	// Not necessary, now that the schedule advances every time we bail.  (dc 03/22/02)
//	WAVE_EndBreak();	// Prevent break from carrying over to restarted wave (dc 03/19/02)
}

static void WAVE_StageAdvance(void)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
    WAVE_Stage = WaveStageEnum(int(WAVE_Stage) + 1 );
#else
	++ (int &) WAVE_Stage;
#endif /* TARGET_XBOX JIV DEBUG */
	(int &) WAVE_Stage %= WAVE_StageMax;
	WAVE_StageProgress = 0;
}

void WAVE_EndWave(bool advance)
{
  if(!ksreplay.IsPlaying())
    ksreplay.SetEndWave();

	if (WaveDebug.BreakLoop || WaveDebug.OneWave || WaveDebug.BreakFreeze)	// Keep with the same wave
	{
		return;
	}

	float duration = WAVE_ScheduleTimeEnd - WAVE_GetTotalSec();

	if (advance || duration < WAVE_MinDuration)	// goto next wave type
	{
		WAVE_ScheduleAdvance();
	}
	else			// restart same wave type
	{
		WAVE_StageReset();
		WAVE_OnNewWave();
	}
}

static void WAVE_ComputeProfileMetaCoeffs(void)
{
	for (u_int i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
	{
		SPLINE_ComputeCoeffs(WAVE_BaseProfile, WAVE_ControlPoints->y[i], WAVE_ControlPoints->numprofile,
			WAVE_ProfileMetaCoeffsStable->y[i].a,
			WAVE_ProfileMetaCoeffsStable->y[i].b,
			WAVE_ProfileMetaCoeffsStable->y[i].c,
			WAVE_ProfileMetaCoeffsStable->y[i].d
		);
		SPLINE_ComputeCoeffs(WAVE_BaseProfile, WAVE_ControlPoints->z[i], WAVE_ControlPoints->numprofile,
			WAVE_ProfileMetaCoeffsStable->z[i].a,
			WAVE_ProfileMetaCoeffsStable->z[i].b,
			WAVE_ProfileMetaCoeffsStable->z[i].c,
			WAVE_ProfileMetaCoeffsStable->z[i].d
		);

		if (WaveDebug.DumpProfileCoeffs)
		{
			nglPrintf("\n\n");
			for (u_int j = 0; j < WAVE_ControlPoints->numprofile; ++j)
			{
				nglPrintf("%f\n", WAVE_BaseProfile[j]);
			}
			nglPrintf("\n");
			for (u_int j = 0; j < WAVE_ControlPoints->numprofile; ++j)
			{
				nglPrintf("%.16f %.12f %f %f\n",
					WAVE_ProfileMetaCoeffsStable->y[i].a[j],
					WAVE_ProfileMetaCoeffsStable->y[i].b[j],
					WAVE_ProfileMetaCoeffsStable->y[i].c[j],
					WAVE_ProfileMetaCoeffsStable->y[i].d[j]
				);
			}
			nglPrintf("\n");
			for (u_int j = 0; j < WAVE_ControlPoints->numprofile; ++j)
			{
				nglPrintf("%.16f %.12f %f %f\n",
					WAVE_ProfileMetaCoeffsStable->z[i].a[j],
					WAVE_ProfileMetaCoeffsStable->z[i].b[j],
					WAVE_ProfileMetaCoeffsStable->z[i].c[j],
					WAVE_ProfileMetaCoeffsStable->z[i].d[j]
				);
			}
		}
	}
}

#ifdef DEBUG
static float WAVE_TestSlice(float x)
{
	if (WAVE_LeftBreaker)
	{
		x = WAVE_MeshMinX + WAVE_MeshMaxX - x;
	}

	x = WAVE_PerturbTest.WorldToProfile(x);
	if (x < WAVE_MeshMinX) x = WAVE_MeshMinX;
	else if (x > WAVE_MeshMaxX) x = WAVE_MeshMaxX;

	return x;
}
#endif

/*	Convert Art units to Profile units.  Art units step by 1 between each profile in the exported wave.
	Profile units step by an amount indicated in WAVE_ProfileStepX.
*/
static float WAVE_ArtToProfile(float x)
{
	assert (x >= 0 && x <= WAVE_ControlPoints->numprofile);

	int floorx = (int) x;
	float fracx = x - floorx;

	return (1 - fracx) * WAVE_ProfileX[floorx] + fracx * WAVE_ProfileX[floorx+1];
}
#ifndef countof
#define countof(a) (sizeof(a)/sizeof(*(a)))
#endif /* countof JIV DEBUG */

inline float WAVE_Sin(float a)
{
	static const float twopi = 6.283185308;
	static const float sintable[] = {
	#include "sin.txt"
	};
	int index = ((int) (a / twopi * countof(sintable))) % (int) countof(sintable);
	return (index >= 0) ? sintable[index] : -sintable[-index];
}

template <int n> float WavePerturbTongueClass<n>::WorldToProfile(float worldx)
{
	float profilex;

	float tonguex = SPLINE_Evaluate(x, coeffs.a, coeffs.b, coeffs.c, coeffs.d, num, worldx);

	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageDo:
		profilex = (1 - WAVE_PerturbProgress) * worldx + WAVE_PerturbProgress * tonguex;
		break;
	case WAVE_PerturbStageHold:
		profilex = tonguex;
		break;
	case WAVE_PerturbStageCollapse:
		profilex = (1 - WAVE_PerturbProgress) * tonguex +
			WAVE_PerturbProgress * WorldToPulse(worldx);
		break;
	case WAVE_PerturbStageWait:
		profilex = WorldToPulse(worldx);
		break;
	case WAVE_PerturbStageUndo:
		profilex = worldx + (WorldToPulse(worldx) - worldx) * sqr(1 - WAVE_PerturbProgress);	// fade out before tail of wave
		break;
	default:
		profilex = worldx;
		break;
	}

	return profilex;
}

template <int n> float WavePerturbCurlClass<n>::WorldToProfile(float worldx)
{
	float profilex;

	float curlx = SPLINE_Evaluate(x, coeffs.a, coeffs.b, coeffs.c, coeffs.d, num, worldx);

	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageDo:
		profilex = (1 - WAVE_PerturbProgress) * worldx + WAVE_PerturbProgress * curlx;
		break;
	case WAVE_PerturbStageHold:
	case WAVE_PerturbStageCollapse:
	case WAVE_PerturbStageWait:
		profilex = curlx;
		break;
	case WAVE_PerturbStageUndo:
		profilex = max(curlx, worldx + (1 - WAVE_PerturbProgress) * shift);
		break;
	default:
		profilex = worldx;
		break;
	}

	return profilex;
}

template <int n> float WavePulsePerturbClass<n>::WorldToPulse(float worldx)
{
	// Shift x to the domain of the function
	static const float &domain = pulsex[num-1];
	assert(pulsex[0] == 0);

	float progress;

	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageDo:
		progress = WAVE_PerturbProgress;
		break;
	case WAVE_PerturbStageUndo:
		progress = WAVE_PerturbProgress + 1;
		break;
	default:
		progress = 1;
		break;
	}

	float modx = worldx - offset + (progress - 1) * domain;
	modx = min(max(0.f,modx),domain);

	float profilex = worldx;
	profilex += SPLINE_Evaluate(pulsex, pulsecoeffs.a, pulsecoeffs.b, pulsecoeffs.c, pulsecoeffs.d, num, modx);
	profilex -= modx;

	return profilex;
}

template <int n> float WavePulsePerturbClass<n>::WorldToProfile(float worldx)
{
	float profilex;

	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageDo:
	case WAVE_PerturbStageHold:
	case WAVE_PerturbStageCollapse:
	case WAVE_PerturbStageWait:
		profilex = WorldToPulse(worldx);
		break;
	case WAVE_PerturbStageUndo:
		profilex = worldx + (WorldToPulse(worldx) - worldx) * sqr(1 - WAVE_PerturbProgress);	// fade out before tail of wave
		break;
	default:
		profilex = worldx;
		break;
	}

	return profilex;
}

template <int n> float WavePushPerturbClass<n>::WorldToPulse(float worldx)
{
	// Shift x to the domain of the function
	static const float &domain = pulsex[num-1];
	assert(pulsex[0] == 0);

	float progress;

	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageDo:
		progress = 2 - WAVE_PerturbProgress;
		break;
	case WAVE_PerturbStageUndo:
		progress = WAVE_PerturbProgress + 1;
		break;
	default:
		progress = 1;
		break;
	}

	float modx = worldx - offset + (progress - 1) * domain;
	modx = min(max(0.f,modx),domain);

	float profilex = worldx;
	profilex += SPLINE_Evaluate(pulsex, pulsecoeffs.a, pulsecoeffs.b, pulsecoeffs.c, pulsecoeffs.d, num, modx);
	profilex -= modx;

	return profilex;
}

template <int n> float WavePushPerturbClass<n>::WorldToProfile(float worldx)
{
	float profilex;

	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageDo:
		profilex = worldx + (WorldToPulse(worldx) - worldx) * sqr(WAVE_PerturbProgress);
		break;
	case WAVE_PerturbStageHold:
	case WAVE_PerturbStageCollapse:
	case WAVE_PerturbStageWait:
		profilex = WorldToPulse(worldx);
		break;
	case WAVE_PerturbStageUndo:
		profilex = worldx + (WorldToPulse(worldx) - worldx) * sqr(1 - WAVE_PerturbProgress);	// fade out before tail of wave
		break;
	default:
		profilex = worldx;
		break;
	}

	return profilex;
}

#ifdef DEBUG
template <int n> float WavePerturbTestClass<n>::WorldToProfile(float worldx)
{
	return SPLINE_Evaluate(x, coeffs.a, coeffs.b, coeffs.c, coeffs.d, numused, worldx);
}
#endif

template <int n> float WavePerturbStairstepClass<n>::WorldToProfile(float worldx)
{
	float profilex;

	// Shift x to the domain of the stairstep function
	static const float &domain = pulsex[num-1];
	assert(pulsex[0] == 0);
	// gives the right stairstep direction and ensures modx >= 0
	float modx = worldx + (WAVE_ShiftX >= 0 ? WAVE_ShiftX : -WAVE_ShiftX) + WAVE_MESHWIDTH;
	modx -= domain * (int) (modx / domain);

	float stairstepx = worldx;
	stairstepx += SPLINE_Evaluate(pulsex, pulsecoeffs.a, pulsecoeffs.b, pulsecoeffs.c, pulsecoeffs.d, num, modx);
	stairstepx -= modx;

	//	This effect fades out when other perturbations are in progress
	switch (WAVE_PerturbStage)
	{
	case WAVE_PerturbStageNone:
		profilex = stairstepx;
		break;
	case WAVE_PerturbStageDo:
		profilex = WAVE_PerturbProgress * worldx + (1 - WAVE_PerturbProgress) * stairstepx;
		break;
	case WAVE_PerturbStageUndo:
		profilex = (1 - WAVE_PerturbProgress) * worldx + WAVE_PerturbProgress * stairstepx;
		break;
	default:
		profilex = worldx;
		break;
	}

	return profilex;
}

static float WAVE_WorldToProfile(float x)
{
	if (WAVE_LeftBreaker)
	{
		x = WAVE_MeshMinX + WAVE_MeshMaxX - x;
	}

	if (WaveDebug.StaticWave)
	{
		return x;
	}

	if (WAVE_PerturbStage != WAVE_PerturbStageNone && WAVE_Perturb->Enabled())
	{
		x = WAVE_Perturb->WorldToProfile(x);
		if (x < WAVE_MeshMinX) x = WAVE_MeshMinX;
		else if (x > WAVE_MeshMaxX) x = WAVE_MeshMaxX;
	}

	if (WAVE_PerturbStairstep.Enabled())
	{
		x = WAVE_PerturbStairstep.WorldToProfile(x);
		if (x < WAVE_MeshMinX) x = WAVE_MeshMinX;
		else if (x > WAVE_MeshMaxX) x = WAVE_MeshMaxX;
	}

	float a = (x - WAVE_MeshMinX) / WAVE_MESHWIDTH;	// from 0 to 1 over width of wave, in reverse time

	switch (WAVE_Stage)	// these are linked to the subdivision methods in WAVE_ComputeGrid
	{
	case WAVE_StageBuilding:
/*
		// Old method
		a = 1 - (1 - a) * WAVE_StageProgress;
*/
		// New method
		a = min(1.f, a + 1 - WAVE_StageProgress);
		break;
	case WAVE_StageSubsiding:
/*
		// Old method
		a = a * (1 - WAVE_StageProgress);
*/
		// New method
		a = max(0.f, a - WAVE_StageProgress);
		break;
	default:
		break;
	}

	return WAVE_MESHWIDTH * a + WAVE_MeshMinX;
}

/*
static float WAVE_WorldToControl(float z)
{
	return WAVE_CONTROLDEPTH * ((z - WAVE_MeshMinZ) / WAVE_MESHDEPTH) + WAVE_CONTROLMINZ;
}

static float WAVE_ControlToWorld(float z)
{
	return WAVE_MESHDEPTH * ((z - WAVE_CONTROLMINZ) / WAVE_CONTROLDEPTH) + WAVE_MeshMinZ;
}

static float WAVE_ControlToWorldD(float dz)
{
	return dz * WAVE_MESHDEPTH / WAVE_CONTROLDEPTH;
}
*/

static float WAVE_HeightPerturb(float x)
{
	float heightperturb = (1 + WAVE_HeightPerturbAmp * WAVE_Sin(WAVE_HeightPerturbFreq * (x + WAVE_ShiftX)));

	// Keep the edge of the wave from stretching during build.
	if (WAVE_Stage == WAVE_StageBuilding)
	{
		float a = (x - WAVE_MeshMinX) / WAVE_MESHWIDTH;	// from 0 to 1 over width of wave, in reverse time

		if (!WAVE_LeftBreaker)
		{
			a = 1 - a;
		}

		if (a > WAVE_StageProgress)
		{
			heightperturb *= (1 - a) / (1 - WAVE_StageProgress);
		}
	}

	return heightperturb;
}

static float WAVE_DepthPerturb(float x)
{
	return WAVE_DepthPerturbAmp * WAVE_Sin(WAVE_DepthPerturbFreq * (x + WAVE_ShiftX));
}

static void WAVE_ComputeProfileCoeffs(float x, float profilex, WaveProfileCoeffs &wpc, bool doperturb)
{
	DECLARE_SCRATCH(WaveProfile, wp);
	const WaveProfileMetaCoeffs &wpmc = *WAVE_ProfileMetaCoeffs;
	u_int i;
	float heightperturb;
	float depthperturb;

	if (doperturb && WaveDebug.BreakSimple)
	{
		heightperturb = WAVE_HeightPerturb(x);
		depthperturb = WAVE_DepthPerturb(x);
	}
	else
	{
		heightperturb = 1;
		depthperturb = 0;
	}

#if defined(TARGET_XBOX)
  assert( profilex == profilex );
  assert( x == x );
#endif /* TARGET_XBOX JIV DEBUG */

START_PROF_TIMER_DAVID(proftimer_generic_12);
	u_int cellx = SPLINE_BinarySearch(WAVE_BaseProfile, WAVE_ControlPoints->numprofile, profilex);
STOP_PROF_TIMER_DAVID(proftimer_generic_12);

START_PROF_TIMER_DAVID(proftimer_generic_13);
	for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
	{
		wp.y[i] = SPLINE_Evaluate(WAVE_BaseProfile,
			wpmc.y[i].a,
			wpmc.y[i].b,
			wpmc.y[i].c,
			wpmc.y[i].d,
			WAVE_ControlPoints->numprofile,
			profilex,
			cellx
		) * heightperturb;

#if defined(TARGET_XBOX)
		assert( wp.y[i] == wp.y[i] );
#endif /* TARGET_XBOX JIV DEBUG */

		wp.z[i] = SPLINE_Evaluate(WAVE_BaseProfile,
			wpmc.z[i].a,
			wpmc.z[i].b,
			wpmc.z[i].c,
			wpmc.z[i].d,
			WAVE_ControlPoints->numprofile,
			profilex,
			cellx
		) + depthperturb;

#if defined(TARGET_XBOX)
		assert( wp.z[i] == wp.z[i] );
#endif /* TARGET_XBOX JIV DEBUG */
	}
STOP_PROF_TIMER_DAVID(proftimer_generic_13);

START_PROF_TIMER_DAVID(proftimer_generic_14);
	if (WaveDebug.DumpProfileControls)
	{
		for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
		{
			nglPrintf("%4.4f, %4.4f, %4.4f\n", profilex, wp.y[i], wp.z[i]);
		}
		nglPrintf("\n");

		WaveDebug.DumpProfileControls = 0;
	}
STOP_PROF_TIMER_DAVID(proftimer_generic_14);

START_PROF_TIMER_DAVID(proftimer_generic_15);
	SPLINE_ComputeCoeffs(WAVE_ControlZ, wp.y, WAVE_ControlPoints->numcontrol, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d);
	SPLINE_ComputeCoeffs(WAVE_ControlZ, wp.z, WAVE_ControlPoints->numcontrol, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d);
STOP_PROF_TIMER_DAVID(proftimer_generic_15);

	if (WaveDebug.DumpProfileCoeffs)
	{
		nglPrintf("%f\n\n", profilex);
		for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
		{
			nglPrintf("%f\n", WAVE_ControlZ[i]);
		}
		nglPrintf("\n");
		for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
		{
			nglPrintf("%.16f %.12f %f %f\n", wpc.y.a[i], wpc.y.b[i], wpc.y.c[i], wpc.y.d[i]);
		}
		nglPrintf("\n");
		for (i = 0; i < WAVE_ControlPoints->numcontrol; ++i)
		{
			nglPrintf("%.16f %.12f %f %f\n", wpc.z.a[i], wpc.z.b[i], wpc.z.c[i], wpc.z.d[i]);
		}

		WaveDebug.DumpProfileCoeffs = 0;
	}
}

static void WAVE_ComputeProfileCoeffs(float x, WaveProfileCoeffs &wpc)
{
	WAVE_ComputeProfileCoeffs(x, WAVE_WorldToProfile(x), wpc);
}

static inline void WAVE_Apply(const WaveProfileCoeffs &wpc, float z, u_int zhint, float &yy, float &zz)
{
	yy = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z, zhint);
//	yy *= WAVE_ScaleSpatial;
	zz = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z, zhint);
}

static inline void WAVE_Apply(const WaveProfileCoeffs &wpc, float z, float &yy, float &zz)
{
	u_int zhint = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, z);
	WAVE_Apply(wpc, z, zhint, yy, zz);
}

/*	Attempt at improving speed of normals.  May give an improvement of around 10% in WAVE_Normal.
	Some assembler problems with BOOTABLE.
#ifdef TARGET_PS2
static inline void WAVE_Vu0Normalize(nglVector vout, const nglVector vin)
{
	__asm__ volatile (
	"
		.set noreorder

		lqc2       vf04,0x0000(%1)
		VMUL.xyz   vf05,vf04,vf04
//		VADDz.x    vf05,vf05,vf05
		VADDz.y    vf05,vf05,vf05
		VRSQRT     Q,vf00w,vf05z
		VWAITQ
		VMULq.xyz  vf06,vf04,Q
		sqc2       vf06,0x0000(%0)

		.set reorder
	"
	:	"=r" (vout) 		// %0	// output
	:	"r" (vin)	 		// %1	// input
	:	"memory"					// modified
	);
}
#endif // #ifdef TARGET_PS2
*/

static inline void WAVE_Normal(const WaveProfileCoeffs &wpc, float z, u_int zhint, float &nx, float &ny, float &nz)
{
	float dyy_dz, dzz_dz;

	dyy_dz = SPLINE_EvaluateD(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z, zhint);
//	dyy_dz *= WAVE_ScaleSpatial;
	dzz_dz = SPLINE_EvaluateD(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z, zhint);
//	dzz_dz = WAVE_ControlToWorldD(dzz_dz);

	nx = 0;			// approximate
	ny = dzz_dz;	// exact
	nz = -dyy_dz;	// exact

	// !!! Requires that nx, ny, nz be consecutive on the stack and properly byte-aligned !!!
	KSNGL_Normalize( (float *) &nx );
}

static void WAVE_Normal(const WaveProfileCoeffs &wpc, float z, float &nx, float &ny, float &nz)
{
	u_int zhint = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, z);
	WAVE_Normal(wpc, z, zhint, nx, ny, nz);
}

static void WAVE_ComputeVTwist(void)
{
	WAVE_MeshMinV = WAVE_ScaleV * WAVE_MeshMinZ + WAVE_ShiftV;
	WAVE_CrashV = WAVE_ScaleV * WAVE_EmitterZ + WAVE_ShiftV;
	WAVE_CrestV = WAVE_ScaleV * WAVE_EmitterCrestZ + WAVE_ShiftV;
	WAVE_MeshMaxV = WAVE_ScaleV * WAVE_MeshMaxZ + WAVE_ShiftV;

	WAVE_VTwistCrashV = WAVE_CrashV + WAVE_VTwistScale * (WAVE_CrestV - WAVE_CrashV);
	WAVE_VTwistCrestV = WAVE_CrestV - WAVE_VTwistScale * (WAVE_CrestV - WAVE_CrashV);

	WAVE_VTwistSlopeMinCrash = (WAVE_VTwistCrashV - WAVE_MeshMinV) / (WAVE_CrashV - WAVE_MeshMinV);
	WAVE_VTwistSlopeCrashCrest = (WAVE_VTwistCrestV - WAVE_VTwistCrashV) / (WAVE_CrestV - WAVE_CrashV);
	WAVE_VTwistSlopeCrestMax = (WAVE_MeshMaxV - WAVE_VTwistCrestV) / (WAVE_MeshMaxV - WAVE_CrestV);
}

#ifdef DEBUG
static inline void WAVE_TextureCoordSimple(float x, float z, float &u, float &v)
{
	if (WaveDebug.ScrollWater)
	{
		u = WAVE_ScaleU * x + WAVE_OffsetU;
		v = WAVE_ScaleV * z + WAVE_OffsetV;
	}
	else
	{
		u = WAVE_ScaleU * x + WAVE_BaseOffsetU;
		v = WAVE_ScaleV * z + WAVE_BaseOffsetV;
	}
}

static void WAVE_TextureCoordVTwist(float x, float z, float &u, float &v)
{
	WAVE_TextureCoordSimple(x, z, u, v);

	if (v < WAVE_CrashV)
	{
		v = WAVE_VTwistSlopeMinCrash * (v - WAVE_MeshMinV) + WAVE_MeshMinV;
	}
	else if (v < WAVE_CrestV)
	{
		v = WAVE_VTwistSlopeCrashCrest * (v - WAVE_CrashV) + WAVE_VTwistCrashV;
	}
	else
	{
		v = WAVE_VTwistSlopeCrestMax * (v - WAVE_CrestV) + WAVE_VTwistCrestV;
	}
}
#else
static inline void WAVE_TextureCoord(float x, float z, float &u, float &v)
{
	u = WAVE_ScaleU * x + WAVE_OffsetU;
	v = WAVE_ScaleV * z + WAVE_OffsetV;
}
#endif

static void WAVE_ComputeShift(void)
{
	if (WaveDebug.StaticWave)
	{
		WAVE_ShiftSpeedU = WAVE_ShiftSpeedPeakU;
		WAVE_ShiftSpeedV = WAVE_ShiftSpeedPeakV;
	}
	else
	{
		switch (WAVE_Stage)
		{
		case WAVE_StageBuilding:
			// We boost current speed during building for rush effect (dc 06/11/02)
			// We use the WaveDebug values so the values can be tuned in real time
			WAVE_ShiftSpeedU = WAVE_StageProgress * WAVE_ShiftSpeedPeakU
				+ WaveDebug.BoostU * WAVE_StageProgress * (1 - WAVE_StageProgress) * WAVE_ShiftSpeedPeakU;
			WAVE_ShiftSpeedV = WAVE_StageProgress * WAVE_ShiftSpeedPeakV
				+ WaveDebug.BoostV * WAVE_StageProgress * (1 - WAVE_StageProgress) * WAVE_ShiftSpeedPeakV;
			break;
		case WAVE_StageStable:
			WAVE_ShiftSpeedU = WAVE_ShiftSpeedPeakU;
			WAVE_ShiftSpeedV = WAVE_ShiftSpeedPeakV;
			break;
		case WAVE_StageSubsiding:
			WAVE_ShiftSpeedU = (1 - WAVE_StageProgress) * WAVE_ShiftSpeedPeakU;
			WAVE_ShiftSpeedV = (1 - WAVE_StageProgress) * WAVE_ShiftSpeedPeakV;
			break;
		default:
			assert(0);
			break;
		}

		if (!WaveDebug.BreakFreeze)
		{
			switch (WAVE_PerturbStage)
			{
			case WAVE_PerturbStageUndo:
				WAVE_ShiftSpeedU = WAVE_ShiftSpeedPeakU + WAVE_ScaleU / WAVE_Perturb->duration[WAVE_PerturbStageUndo];
				break;
			default:
				break;
			}
		}
	}

	if (WAVE_LeftBreaker)
	{
		WAVE_ShiftSpeedU = -WAVE_ShiftSpeedU;
	}

	WAVE_ShiftU += WAVE_ShiftSpeedU * WAVE_GetFrameSec();
	WAVE_ShiftU -= (int) WAVE_ShiftU;
	if ( WAVE_ShiftU < 0.0f ) WAVE_ShiftU += 1.0f;
	WAVE_OffsetU = WAVE_BaseOffsetU + WAVE_ShiftU;
	WAVE_ShiftV += WAVE_ShiftSpeedV * WAVE_GetFrameSec();
	WAVE_ShiftV -= (int) WAVE_ShiftV;
	if ( WAVE_ShiftV < 0.0f ) WAVE_ShiftV += 1.0f;
	WAVE_OffsetV = WAVE_BaseOffsetV + WAVE_ShiftV;

	WAVE_ShiftSpeedX = WAVE_ShiftSpeedU / WAVE_ScaleU;
	WAVE_ShiftSpeedZ = WAVE_ShiftSpeedV / WAVE_ScaleV;
	WAVE_ShiftX += WAVE_ShiftSpeedX * WAVE_GetFrameSec();
	WAVE_ShiftZ += WAVE_ShiftSpeedZ * WAVE_GetFrameSec();
}

static void WAVE_ComputeStage(void)
{
	if (WaveDebug.BreakReload)
	{
		WAVE_PerturbInit();
	}
	if (WaveDebug.BreakFreeze)
	{
		return;
	}
	if (WaveDebug.RestartWave)
	{
		WAVE_EndWave(false);
		WaveDebug.RestartWave = false;
	}
	if (WaveDebug.EndWave)
	{
		WAVE_Stage = WAVE_StageStable;
		WAVE_PerturbStage = WAVE_PerturbStageNone;
		WAVE_PerturbProgress = 0;
		WaveDebug.EndWave = false;
	}

	WAVE_ScheduleProgress = (WAVE_GetTotalSec() - WAVE_ScheduleTimeStart)
		/ (WAVE_ScheduleTimeEnd - WAVE_ScheduleTimeStart);
	if (WAVE_ScheduleProgress >= 1.f)
	{
		WAVE_ScheduleAdvance();
	}

	WAVE_StageProgress = (WAVE_GetTotalSec() - WAVE_StageStart[WAVE_Stage])
		/ WAVE_StageDuration[WAVE_Stage];
	if (WAVE_StageProgress >= 1.f)
	{
		WAVE_StageAdvance();
	}

	float wppdwps=WAVE_Perturb->duration[WAVE_PerturbStage];
		// PLEASE don't divide by zero
	if ( wppdwps==0.0f )
		WAVE_PerturbProgress = 1.0f;
	else
		WAVE_PerturbProgress = (WAVE_GetTotalSec() - WAVE_Perturb->start[WAVE_PerturbStage])
			/ wppdwps;
	if (WAVE_PerturbProgress >= 1.f)
	{
		WAVE_PerturbAdvance();
	}

	if (
		WaveDebug.BreakLoop &&
		(WAVE_Perturb->duration[WAVE_PerturbStageNone] > 1 || WAVE_DebugPerturbType != WAVE_PerturbType)
	)
	{
		WAVE_PerturbType = WAVE_DebugPerturbType;
		WAVE_Perturb = WAVE_PerturbArray[WAVE_PerturbType];

		WAVE_PerturbReset(1);	// override wait time
	}
}

/*	The spacing of the mesh in the z-direction (front to back of the wave) is based on cues from
	the exported art.  Where the control points are closer together (near the crest of the wave),
	the mesh is finer.  The algorithm tries to break the "jump" between each successive pair of
	control points into an approximately constant number of steps:

	z (10)
	jumps	+--------------+--------+----+--+-+---+-------+-------------+----------------------+

	zz (25)	       zinc
	steps	*-----*-----*----*---*---*-*-***********--*--*----*-----*------*---------*---------*

	To compute a step increment in the above example, we try to make

		zinc = (10 - 1) / (25 - 1) * (z[i+1] - z[i])

	so that the step size is proportional to the jump size, by the same ratio as the number of jumps
	to the number of steps.  Sometimes a step will cross from one jump interval to the next.  In that
	case we compute the step in two pieces.  The first piece goes from the current position to the
	next jump boundary.  We compute the fraction this piece occupies of the total step (if we hadn't
	crossed a boundary).  If the first piece was only .4 of the ideal step, then the second piece should
	be .6 of the ideal step for the next interval.

	More formally, let

		zjumpmax = number of jumps
		zsteparea = 1 / number of steps
		zguide = array of jump boundaries
		zjump = index of current jump interval

	Then the ideal step increment starting from a point zbasecontrol in the interval
	[zguide[zjump], zguide[zjump+1]] is

		zinc = zjumpmax * (zguide[zjump+1] - zguide[zjump]) * zsteparea;

	If this increment would push us past a jump boundary (zbasecontrol + zinc > zguide[zjump+1]), then the
	fraction of the ideal increment which is covered by moving to the next jump boundary is

		frac = (zguide[zjump+1] - zbasecontrol) / (zjumpmax * (zguide[zjump+1] - zguide[zjump]) * zsteparea)

	Then the actual increment to advance is composed of these two pieces

		zinc = (zguide[zjump] - zbasecontrol) +
				zjumpmax * (zguide[zjump+1] - zguide[zjump]) * zsteparea * (1 - frac)

static void WAVE_ComputeGrid(void)
{
	u_int xstep, zstep, zjump;
	float xinc = ((float) WAVE_MESHWIDTH) / WAVE_MeshStepX;
	float zinc;
	float xbase = WAVE_MeshMinX, zbase;
	const u_int xstepmax = WAVE_MeshStepX;
	const u_int zjumpmax = WAVE_ControlPoints->numcontrol - 1;
	const u_int zstepmax = WAVE_MeshStepZ;
	const float zsteparea = 1.f / zstepmax;
	const float (&zguide)[WAVE_ControlPoints->numcontrol] = WAVE_ControlZ;	// just a renaming

	assert(zstepmax >= zjumpmax);

	for (xstep = 0; xstep < WAVE_MeshStepX; ++xstep, xbase += xinc)
	{
		WAVE_MeshGridX[xstep] = xbase;
	}

	WAVE_MeshGridX[xstepmax] = WAVE_MeshMaxX;

	for (zstep = 0, zjump = 0, zbase = WAVE_CONTROLMINZ; zstep < zstepmax; ++zstep, zbase += zinc)
	{
		WAVE_MeshGridZ[zstep] = WAVE_ControlToWorld(zbase);
		WAVE_HintGridZ[zstep] = zjump;

		// Increment depends what part of wave we're on (see comment block above)
		zinc = zjumpmax * (zguide[zjump+1] - zguide[zjump]) * zsteparea;
		if (zbase + zinc > zguide[zjump+1])
		{
			float surplus = zsteparea -
				(zguide[zjump+1] - zbase) / (zjumpmax * (zguide[zjump+1] - zguide[zjump]));
			++zjump;
			assert(zjump < zjumpmax);
			zinc = (zguide[zjump] - zbase)
				+ zjumpmax * (zguide[zjump+1] - zguide[zjump]) * surplus;
		}
	}

	WAVE_MeshGridZ[zstepmax] = WAVE_MeshMaxZ;
	WAVE_HintGridZ[zstepmax] = zjumpmax - 1;
}
*/

/*	This algorithm is a generalization of the old WAVE_ComputeGrid above.  The input is a WavePartition *wp,
	basically a collection of intervals with weights assigned.  The output is gridarray, a float [] of size
	stepmax + 1, which roughly follows the elements of wp->guide.  Here's an example:

	N = 15

	i	guide		guidestep	weight
	0	-33.3923	6.23959		1
	1	-27.1527	10.2072		1
	2	-16.9455	7.7763		1
	3	-9.16917	5.56026		1
	4	-3.60892	4.95232		1
	5	1.34341		1.47847		1
	6	2.82188		1.75987		2
	7	4.58174		3.17691		2
	8	7.75866		3.85585		1.5
	9	11.6145		7.41145		3
	10	19.026		16.0648		2
	11	35.0907		34.6732		1
	12	69.764		37.3057		1
	13	107.07		33.3844		1
	14	140.454

	The weight represents the share of gridarray should be assigned to the interval.  For instance, the
	interval [2.82188, 4.58174] between points 6 and 7 has weight 2.  Since the total weight is 19.5, that
	means 2 / 19.5 = 10.26% of the elements of the output array should fall in this interval.

	To compute the output array, we keep track of which interval we're in and what step size is dictated for
	that interval by its weight.  The only wrinkle is what happens when we change intervals.  In that case, we
	do a special computation to see what fraction of a step is still left to go after the interval boundary.
*/
static void WAVE_SubdivideGrid(const WavePartition *wp, float *gridarray, u_int stepmax)
{
	u_int step, jump;
	float inc;
	float base, basestep;
	float weight;
	float weighttotal;
	const float *weightptr = wp->weight;
	float guide;
	const float *guideptr = wp->guide;
	float guidestep;
	const float *guidestepptr = wp->guidestep;
	float *&gridptr = gridarray;
	const u_int &jumpmax = wp->N - 1;
	float surplus;

	// Sum up the weights
	for (jump = 0, weighttotal = 0; jump < jumpmax; ++jump, weighttotal += *weightptr++) continue;
	weightptr = wp->weight;

	// Compute the divisions
	for (
		step = 1, jump = 0, inc = 1, *gridptr++ = base = guide = *guideptr++, guidestep = 0,
			basestep = guidestep + inc;
		step < stepmax;
		++step, *gridptr++ = base, base += inc, basestep += inc
	)
	{
		while (basestep > guidestep)	// overshot next jump boundary
		{
			surplus = (basestep - guidestep) / inc;	// overshot by what fraction

			++jump;
			assert(jump <= jumpmax);
			weight = *weightptr++;
			guidestep = *guidestepptr++;
			assert(guidestep >= 0);

			inc = (guidestep * weighttotal) / (stepmax * weight);	// step size for next jump interval
			basestep = inc * surplus;	// proceed by remaining fraction of a step
			base = guide + basestep;

			guide = *guideptr++;
		}
	}

	*gridptr = wp->guide[jumpmax];
}

/*	This function is superseded by WAVE_RefinePositions.
static void WAVE_SubdivideByPosition(float *weight, const float *guide, u_int numguide, float boost, float pos,
	u_int cell = (u_int) -1)
{
//	The assert sometimes fails, because surfer goes outside of mesh every so often
if (pos < guide[0]) pos = guide[0];
if (pos > guide[numguide-1]) pos = guide[numguide-1];
	assert(pos >= guide[0] && pos <= guide[numguide-1]);
	if (cell == (u_int) -1) cell = SPLINE_BinarySearch(guide, numguide, pos);
	assert(pos >= guide[cell] && pos <= guide[cell+1]);
	float center = (guide[cell] + guide[cell+1]) / 2;
	float size = guide[cell+1] - guide[cell];
	u_int lo, hi;
	float frac;
	if (pos < center) {lo = cell - 1; hi = cell; frac = (center - pos) / size;}
	else {lo = cell; hi = cell + 1; frac = 1 - (pos - center) / size;}
	if (lo >= 0) weight[lo] *= 1 + boost * frac;
	if (hi < numguide - 1) weight[hi] *= 1 + boost * (1 - frac);
}
*/

/*	Takes as input two WavePartitions (see comments for WAVE_SubdivideGrid above).  Return the
	union of the two partitions, in increasing order, with weights determined by choice of mode:

	WAVE_RefineBlend:
	The weights of the intervals in the union vary continuously based on the parameter "blend".
	When blend = 0, the output partition is identical to wp0, in the sense that the weight of
	each interval is determined solely by the weights of wp0.  When blend = 1, the same holds for
	wp1.  For intermediate values of blend, the weights for blend = 0 and blend = 1 are linearly
	interpolated.  This mode is used to move seamlessly from one partition to another, as during
	wave perturbations.

	WAVE_RefineConcatenate:
	Apply wp1 followed by wp0, in the sense that each interval of wp1 retains its original weight,
	but is subdivided according to the weights of wp0.  This mode can be used to modify an existing,
	partition so that a fixed percentage of the total weight ends up in a certain range.

	WAVE_RefineModulate:
	Determine refinement weights using wp0 (same as Blend mode with blend = 0, then multiply by
	the weights of wp1.  This mode can be used to modify an existing partition by boosting the weight
	in certain areas.

	Example:

	wp0		guide	weight
			0		1
			2		2
			5

	wp1		guide	weight
			1		1
			4		2
			6

	wp		guide	Blend = 0	Blend = .5	Blend = 1	Concatenate	Modulate
			0		.5			.25			0			0			0
			1		.5			.42			.33			.43			.5
			2		1.33		1			.67			.57			1.33
			4		.67			.83			1			2			1.33
			5		0			.5			1			0			0
			6
*/
enum WaveRefineModeEnum {
	WAVE_RefineBlend,
	WAVE_RefineConcatenate,
	WAVE_RefineModulate,
	WAVE_RefineMax
};
void WAVE_RefinePartitions(const WavePartition *wp0, const WavePartition *wp1, WavePartition *wp,
	WaveRefineModeEnum mode = WAVE_RefineModulate, float blend = 0)
{
	u_int i, i0, i1;
	bool adv0, adv1;
	float curr, prev = (float) 1e10, curr0, curr1;
	float *currptr;
	const float *curr0ptr, *curr1ptr;
	float *currstepptr;
	const float *currstep0ptr, *currstep1ptr;
	float weight, weight0, weight1;
	float splitweight0, splitweight1;
	float cumulweight0, invcumulweight0;
	float *weightptr, *backweightptr;
	const float *weight0ptr, *weight1ptr;
	float step, invstep0, invstep1, diff;
	const static float toobig = 3.402823466e38;
	const u_int &numguide0 = wp0->N, &numguide1 = wp1->N;
	u_int &numguide = wp->N;
	u_int begin, end;
	assert(numguide >= numguide0 + numguide1);

	for (
		i0 = 0, curr0ptr = wp0->guide, curr0 = *curr0ptr++, currstep0ptr = wp0->guidestep,
			weight0ptr = wp0->weight, weight0 = 0, invstep0 = 0, cumulweight0 = 0,
		i1 = 0, curr1ptr = wp1->guide, curr1 = *curr1ptr++, currstep1ptr = wp1->guidestep,
			weight1ptr = wp1->weight, weight1 = 0, invstep1 = 0,
		i = 0, currptr = wp->guide, currstepptr = wp->guidestep, backweightptr = weightptr = wp->weight,
		numguide = numguide0 + numguide1,  	// less, if there are duplicates
		begin = 0, end = 0;

		i < numguide;

		++i, prev = curr
	)
	{
		diff = curr0 - curr1;
		if (diff < 0)
		{
			curr = curr0;
			adv0 = true;
			adv1 = false;
		}
		else if (diff == 0)
		{
			curr = curr0;
			adv0 = true;
			adv1 = true;
			--numguide;
		}
		else //if (diff > 0)
		{
			assert(curr0 > curr1);
			curr = curr1;
			adv0 = false;
			adv1 = true;
		}

		if (i > 0)
		{
			assert (prev != (float) 1e10);	// else uninitialized (dc 01/29/02)
			step = curr - prev;
			assert(step > 0);
			*currstepptr++ = step;
			splitweight0 = weight0 * step * invstep0;
			splitweight1 = weight1 * step * invstep1;
			switch (mode)
			{
			case WAVE_RefineBlend:	// move smoothly from wp0 to wp1 as blend moves from 0 to 1
				weight = (1 - blend) * splitweight0 + blend * splitweight1;
				break;
			case WAVE_RefineConcatenate:	// split intervals of wp1 according to weights of wp0
				cumulweight0 += splitweight0;
				// fall through to next case
			case WAVE_RefineModulate:	// use wp1 to boost/reduce weights of wp0 over certain ranges
				weight = splitweight0 * weight1;
				break;
			default:
				assert(0);
				return;	// else weight uninitialized (dc 01/29/02)
			}

			assert(weight >= 0);
			if (weight > 0)
			{
				if (end == 0) begin = i - 1;
				end = i;
			}
			*weightptr++ = weight;
		}

		*currptr++ = curr;

		if (adv0)
		{
			++i0;
			if (i0 < numguide0)
			{
				assert(curr0 < *curr0ptr);
				curr0 = *curr0ptr++;
				assert(curr0 > curr);
				assert(curr0 < toobig);
				weight0 = *weight0ptr++;
				assert(weight0 > 0);
				invstep0 = 1 / *currstep0ptr++;
			}
			else
			{
				assert(i0 == numguide0);
				curr0 = toobig;
				weight0 = 0;
				invstep0 = 0;
			}
		}

		if (adv1)
		{
			++i1;
			if (i1 < numguide1)
			{
				assert(curr1 < *curr1ptr);
				curr1 = *curr1ptr++;
				assert(curr1 > curr);
				assert(curr1 < toobig);
				weight1 = *weight1ptr++;
				assert(weight1 > 0);
				invstep1 = 1 / *currstep1ptr++;
			}
			else
			{
				assert(i1 == numguide1);
				curr1 = toobig;
				weight1 = 0;
				invstep1 = 0;
			}

			if (mode == WAVE_RefineConcatenate && i1 > 1)
			{
				for (
					invcumulweight0 = (cumulweight0 > 0) ? 1 / cumulweight0 : 1;
					backweightptr < weightptr;
					*backweightptr++ *= invcumulweight0
				) continue;
				cumulweight0 = 0;
			}
		}
	}

	numguide = end - begin + 1;
	wp->guide += begin;
	wp->guidestep += begin;
	wp->weight += begin;
}

/*	Perform a scaling and offset on the input partition.
*/
static void WAVE_TransformPartition(const WavePartition *wpin, WavePartition *wpout, float scale, float offset)
{
	const float *guideinptr = wpin->guide;
	float *guideoutptr = wpout->guide;
	const float *guidestepinptr = wpin->guidestep;
	float *guidestepoutptr = wpout->guidestep;
	const float *weightinptr = wpin->weight;
	float *weightoutptr = wpout->weight;
	const u_int &numguide = wpin->N;
	wpout->N = numguide;

	assert(scale > 0);
	for (u_int i = 0; i < numguide - 1; ++i)
	{
		*guideoutptr++ = scale * *guideinptr++ + offset;
		*guidestepoutptr++ = scale * *guidestepinptr++;
		*weightoutptr++ = *weightinptr++;
	}
	*guideoutptr++ = scale * *guideinptr++ + offset;
}

/*	Restrict a partition to a particular interval.  All guides and weights outside this interval
	are discarded.
*/
static void WAVE_RestrictPartition(WavePartition *wp, float min, float max)
{
	float *guideptr = wp->guide;
	u_int i;

	for (i = 0; i < wp->N && *guideptr < min ; ++i, ++guideptr) continue;
	if (i > 0 && *guideptr > min && *(guideptr - 1) < min)
	{
		--i;
		--guideptr;
		wp->guide[i] = min;
		float oldguidestep = wp->guidestep[i];
		wp->guidestep[i] = wp->guide[i+1] - wp->guide[i];
		assert(wp->guidestep[i] > 0);
		wp->weight[i] *= wp->guidestep[i] / oldguidestep;
	}
	wp->guide += i;
	wp->guidestep += i;
	wp->weight += i;
	wp->N -= i;
	for (i = 0; i < wp->N && *guideptr <= max ; ++i, ++guideptr) continue;
	if (i < wp->N && *guideptr > max && *(guideptr-1) < max)
	{
		wp->guide[i] = max;
		float oldguidestep = wp->guidestep[i-1];
		wp->guidestep[i-1] = wp->guide[i] - wp->guide[i-1];
		assert(wp->guidestep[i-1] > 0);
		wp->weight[i-1] *= wp->guidestep[i-1] / oldguidestep;
		++i;
	}
	wp->N = i;
}

static void WAVE_PrependPartition(const WavePartition *wpin, WavePartition *wpout, float guide, float weight)
{
	const float *guideinptr = wpin->guide;
	float *guideoutptr = wpout->guide;
	const float *guidestepinptr = wpin->guidestep;
	float *guidestepoutptr = wpout->guidestep;
	const float *weightinptr = wpin->weight;
	float *weightoutptr = wpout->weight;
	u_int i;
	const u_int &numguidein = wpin->N;
	u_int &numguideout = wpout->N;
	const u_int numweightin = numguidein - 1;

	numguideout = numguidein;

	if (guide < *guideinptr)
	{
		++numguideout;
		*guideoutptr++ = guide;
		*guidestepoutptr++ = *guideinptr - guide;
		*weightoutptr++ = weight;
	}
	*guideoutptr++ = *guideinptr++;
	for (
		i = 0;
		i < numweightin;
		++i, *guideoutptr++ = *guideinptr++, *guidestepoutptr++ = *guidestepinptr++,
			*weightoutptr++ = *weightinptr++
	) continue;

//#if defined(TARGET_XBOX)
//  return *weightoutptr;
//#endif /* TARGET_XBOX JIV DEBUG */

}

static void WAVE_AppendPartition(const WavePartition *wpin, WavePartition *wpout, float guide, float weight)
{
	const float *guideinptr = wpin->guide;
	float *guideoutptr = wpout->guide;
	const float *guidestepinptr = wpin->guidestep;
	float *guidestepoutptr = wpout->guidestep;
	const float *weightinptr = wpin->weight;
	float *weightoutptr = wpout->weight;
	u_int i;
	const u_int &numguidein = wpin->N;
	u_int &numguideout = wpout->N;
	const u_int numweightin = numguidein - 1;

	numguideout = numguidein;

	*guideoutptr++ = *guideinptr++;
	for (
		i = 0;
		i < numweightin;
		++i, *guideoutptr++ = *guideinptr++, *guidestepoutptr++ = *guidestepinptr++,
			*weightoutptr++ = *weightinptr++
	) continue;
	if (guide > *--guideinptr)
	{
		++numguideout;
		*guideoutptr++ = guide;
		*guidestepoutptr++ = guide - *guideinptr;
		*weightoutptr++ = weight;
	}
}

static void WAVE_PrependDeadweight(const WavePartition *wpin, WavePartition *wpout, float weight)
{
	float min = wpin->guide[0], max = wpin->guide[wpin->N - 1];
	float epsilon = 0.0001f * (max - min);

	++wpout->guide;
	++wpout->guidestep;
	++wpout->weight;
	WAVE_CopyPartition(wpin, wpout);
	WAVE_RestrictPartition(wpout, min + epsilon, max);
	--wpout->guide;
	--wpout->guidestep;
	--wpout->weight;
	wpout->guide[0] = min;
	wpout->guidestep[0] = (min + epsilon) - min;	// ~= epsilon, up to floating point accuracy
	wpout->weight[0] = weight;
	++wpout->N;
/*
	const float *guideinptr = wpin->guide;
	float *guideoutptr = wpout->guide;
	const float *guidestepinptr = wpin->guidestep;
	float *guidestepoutptr = wpout->guidestep;
	const float *weightinptr = wpin->weight;
	float *weightoutptr = wpout->weight;
	u_int i;
	const u_int &numguidein = wpin->N;
	u_int &numguideout = wpout->N;
	const u_int numweightin = numguidein - 1;
	assert(numweightin >= 1);
	float epsilon;

	numguideout = numguidein + 1;

	epsilon = min(*guidestepinptr / 2, 0.01f);
	*guideoutptr++ = *guideinptr;
	*guidestepoutptr++ = (*guideinptr + epsilon) - *guideinptr; // ~= epsilon;
	*weightoutptr++ = weight;
	*guideoutptr++ = *guideinptr++ + epsilon;
	*guidestepoutptr++ = *guideinptr - *(guideoutptr-1), ++guidestepinptr;
	*weightoutptr++ = *weightinptr++;
	*guideoutptr++ = *guideinptr++;
	for (
		i = 1;
		i < numweightin;
		++i, *guideoutptr++ = *guideinptr++, *guidestepoutptr++ = *guidestepinptr++,
			*weightoutptr++ = *weightinptr++
	);
*/
}

static void WAVE_AppendDeadweight(const WavePartition *wpin, WavePartition *wpout, float weight)
{
	float min = wpin->guide[0], max = wpin->guide[wpin->N - 1];
	float epsilon = 0.0001f * (max - min);

	WAVE_CopyPartition(wpin, wpout);
	WAVE_RestrictPartition(wpout, min, max - epsilon);
	wpout->guide[wpout->N] = max;
	wpout->guidestep[wpout->N - 1] = max - (max - epsilon);	// ~= epsilon, up to floating point accuracy
	wpout->weight[wpout->N - 1] = weight;
	++wpout->N;

/*
	const float *guideinptr = wpin->guide;
	float *guideoutptr = wpout->guide;
	const float *guidestepinptr = wpin->guidestep;
	float *guidestepoutptr = wpout->guidestep;
	const float *weightinptr = wpin->weight;
	float *weightoutptr = wpout->weight;
	u_int i;
	const u_int &numguidein = wpin->N;
	u_int &numguideout = wpout->N;
	const u_int numweightin = numguidein - 1;
	assert(numweightin >= 1);
	float epsilon = 0.01f;

	numguideout = numguidein + 1;

	*guideoutptr++ = *guideinptr++;
	for (
		i = 0;
		i < numweightin - 1;
		++i, *guideoutptr++ = *guideinptr++, *guidestepoutptr++ = *guidestepinptr++,
			*weightoutptr++ = *weightinptr++
	);
	epsilon = min(*guidestepinptr / 2, 0.01f);
	*guideoutptr++ = *guideinptr - epsilon;
	*guidestepoutptr++ = *(guideoutptr-1) - *(guideinptr-1), ++guidestepinptr;
	*weightoutptr++ = *weightinptr++;
	*guideoutptr++ = *guideinptr++;
	*guidestepoutptr++ = *(guideoutptr-1) - (*(guideoutptr-1) - epsilon); // ~= epsilon;
	*weightoutptr++ = weight;
*/
}

static float WAVE_TotalWeight(const WavePartition *wp)
{
	float total;
	u_int i;
	const float *weightptr = wp->weight;
	const u_int &numweight = wp->N - 1;

	for (i = 0, total = 0; i < numweight; ++i, total += *weightptr++) continue;

	return total;
}

/*	Allocate enough memory from "mem" to fit a partition of size N, and advance
	the value of "mem".
*/
static WavePartition *WAVE_AllocPartition(char *&mem, u_int N)
{
	WavePartition *wp = (WavePartition *) mem;
	mem += (sizeof(*wp) + sizeof(*mem) - 1) / sizeof(*mem);
	wp->N = N;
	wp->guide = (float *) mem;
	mem += N * (sizeof(*wp->guide) + sizeof(*mem) - 1) / sizeof(*mem);
	wp->guidestep = (float *) mem;
	mem += (N - 1) * (sizeof(*wp->guidestep) + sizeof(*mem) - 1) / sizeof(*mem);
	wp->weight = (float *) mem;
	mem += (N - 1) * (sizeof(*wp->weight) + sizeof(*mem) - 1) / sizeof(*mem);

	return wp;
}

static void WAVE_CopyPartition(const WavePartition *wpin, WavePartition *wpout)
{
	wpout->N = wpin->N;
	memcpy(wpout->guide, wpin->guide, wpin->N * sizeof(*wpin->guide));
	memcpy(wpout->guidestep, wpin->guidestep, wpin->N * sizeof(*wpin->guidestep));
	memcpy(wpout->weight, wpin->weight, wpin->N * sizeof(*wpin->weight));
}

static inline const WavePartition *WAVE_ValidatePartition(const WavePartition *wp)
{
#if defined(DEBUG) && !defined(TARGET_XBOX) && defined(DAVID)
	u_int i;
	const float *guideptr = wp->guide;
	const float *guidestepptr = wp->guidestep;
	const float *weightptr = wp->weight;
	const u_int &numweight = wp->N - 1;

	for (i = 0; i < numweight; ++i, ++guideptr, ++guidestepptr, ++weightptr)
	{
		assert(*guidestepptr > 0);
		assert(fabsf(*guidestepptr - (*(guideptr+1) - *guideptr)) < 0.001f * *guidestepptr);
		assert(*weightptr > 0);
	}
#endif

	return wp;
}

/*	Find the point (x, 0, z) in world space (for z given) which maps to a particular
	horizontal value p in screen space, if such a point exists.
static nglMatrix WAVE_LocalToScreen;
static float WAVE_ScreenWidth;
static float WAVE_ScreenHeight;
static inline bool WAVE_ScreenToX(float z, float p, float &x)
{
WAVE_ScreenWidth = nglGetScreenWidth();
WAVE_ScreenHeight = nglGetScreenHeight();

nglMatrix nglWorldToScreen;
nglGetMatrix (nglWorldToScreen, NGLMTX_WORLD_TO_SCREEN);
nglMulMatrix(WAVE_LocalToScreen, nglWorldToScreen, *(nglMatrix *) &WAVE_LocalToWorld);

	const nglMatrix &m = WAVE_LocalToScreen;

	p -= WAVE_ScreenWidth / 2;
	p += 2048;
	if (0 == m[0][0] - p * m[0][3]) return false;
	x = ((-m[2][0] + p * m[2][3]) * z + (-m[3][0] + p * m[3][3])) / (m[0][0] - p * m[0][3]);
	if ((m[0][2] * x + m[2][2] * z + m[3][2]) / (m[0][3] * x + m[2][3] * z + m[3][3]) < 0) return false;
	return true;
}
*/

/*	Newer version.  Variable spacing is determined by a spline map from an evenly-spaced grid to
	the grid of the input data in world space.  When we subdivide the evenly-spaced grid, we get
	a subdivision of the input grid which matches the base grid in local density.  Example:

	Evenly-spaced grid
	|---*---|---*---|---*---|---*---|---*---|---*---|---*---|---*---|---*---|---*---|---*---|---*---|

	World space grid
	|----------*----------|-------*-------|----*----|--*--|-*-|*|*|-*-|--*--|--*--|--*--|-----*-----|

	| --> Initial grid mark
	* --> Subdivision mark

	For technical reasons we use different conventions for X and Z.  In X, we compute the world space
	grid and then do all computations in world space.  That's because the mapping from local X to world X
	can be the identity --- the wave is never distorted in the X-direction.  In Z, we compute the world
	space grid but do most computations in "control space".  That's because, in most places, the wave is
	distorted in the Z-direction, so we have to do a conversion anyhow.

	WARNING:	A BIG DRAWBACK OF THIS METHOD IS THAT THE SPLINE CAN CHANGE DIRECTION, EVEN IF THE CONTROL
	POINTS ARE STRICTLY INCREASING.  AVOIDING THIS PROBLEM IS MOSTLY A MATTER OF TRIAL AND ERROR.  BECAUSE
	OF THIS EFFECT, I'M REVERTING TO SOMETHING MORE LIKE THE ORIGINAL METHOD.

	NOTE:  THIS FUNCTION ONLY NEEDS TO BE RUN IF THE GRID CHANGES.  WE USED TO KEEP THE GRID CONSTANT, SO
	WE DIDN'T NEED TO CALL THIS FUNCTION EVERY FRAME.  WITH DYNAMIC SUBDIVISION, WE NEED TO MAKE THIS CALL
	MOST OR ALL OF THE TIME.
*/

/*	New version, dynamic subdivision.  We now vary the underlying grid of the wave mesh each frame, based
	on a number of factors.  The recomputing of the grid has the goal of eliminating the appearance of
	polygonality in the mesh by concentrating more geometry where it is needed.  Here are the criteria for
	subdivision:

	BASELINE:
	The baseline subdivision of the wave comes from the Max export.  There are WAVE_ControlPoints->numprofile points in the
	x-direction and WAVE_ControlPoints->numcontrol points in the z-direction.  Each interval between profile or control
	points receives the same number of mesh points.  Generally the areas of the wave near the break (in the
	x-direction) and near the lip (in the z-direction) receive the most mesh points.

	This process is modified by WAVE_ProfileWeightX and WAVE_ControlWeightZ.  For instance, since
	WAVE_ControlWeightZ[0] == 1 and WAVE_ControlWeightZ[1] == 2, the interval between control points 0 and 1
	will receive only half as many mesh points as the interval between control points 1 and 2.  The weights
	are computed by hand, based on what looks best in the game.  For instance we boost the front of the wave
	at the expense of the back of the wave.

	Together, the intervals from the wave export and their corresponding weights form the "Base partitions".

	X-DIRECTION MODIFIERS:
	Building or subsiding wave:  When the wave is building or subsiding, the areas which require the most
	geometry are linearly translated from their default positions.  We model this by scaling and offsetting
	the base x-partition, and then restricting it to the limits of the mesh.

	Perturbed wave:  In WAVE_PerturbInitPartitions, we compute partitions for each wave perturbation by applying
	the perturbation to the base partition.  While the pertubation is at its holding stage, we use its partition
	in place of the base partition.  While the perturbation is beginning and ending, we blend between the base
	partition and the perturbed partition.

	Surfer's position:  We boost the amount of geometry near the surfer's position, under the assumption that
	this area of the mesh is likelier to be prominent on the screen.  Sometimes, this assumption backfires
	slightly, as when the break is in the background, in profile.

	Camera's position:  Ideally, we'd like to devote practically all of the geometry to just the portion of the
	mesh which is on screen.  Also, we'd like to subdivide the areas which are close to the camera more than
	the areas which are far from the camera.  These goals have proved tricky in practice, so the code is disabled
	for the time being.

	Z-DIRECTION MODIFIERS:
	Surfer's position:  Similar to the modifier in the x-direction.
*/
static void WAVE_ComputeGrid(void)
{
START_PROF_TIMER(proftimer_wave_compute_grid);
	u_int xstep, zstep, zcell;
	const u_int xstepmax = WAVE_MeshStepX, zstepmax = WAVE_MeshStepZ;
	const WavePartition *wp;
	char _mem[2048], *mem = _mem;	// can increase if needed
	char *memend = _mem + countof(_mem);

	// Subdivision based on x
//	mem = _mem;	// done already
	wp = &WAVE_BaseProfilePartition;

	if (!WaveDebug.StaticWave)
	{
		if (WaveDebug.SubdivideBuilding)
		{
			// Transform partition for building or subsiding wave
			WavePartition *TransformedPartition = WAVE_AllocPartition(mem, wp->N);
			assert(mem < memend);
			WavePartition *PendedPartition = WAVE_AllocPartition(mem, TransformedPartition->N + 1);
			assert(mem < memend);
			WavePartition *PendedPartition2 = WAVE_AllocPartition(mem, PendedPartition->N + 1);
			assert(mem < memend);

//			float scale;
			float prevtotal, currtotal;
			float flatweight, deadweight;
			static float flatfrac = 0.2f;

			switch (WAVE_Stage)
			{
			case WAVE_StageBuilding:
				if (WAVE_StageProgress < 1)
				{
					WAVE_TransformPartition(wp, TransformedPartition, 1, -WAVE_MESHWIDTH * (1 - WAVE_StageProgress));
					WAVE_ValidatePartition(TransformedPartition);
					prevtotal = WAVE_TotalWeight(TransformedPartition);
					WAVE_RestrictPartition(TransformedPartition, WAVE_MeshMinX, WAVE_MeshMaxX);
					wp = WAVE_ValidatePartition(TransformedPartition);
					currtotal = WAVE_TotalWeight(wp);
					flatweight = prevtotal * flatfrac * (1 - WAVE_StageProgress);
					assert(flatweight > 0);
					WAVE_AppendPartition(wp, PendedPartition, WAVE_MeshMaxX, flatweight);
					wp = WAVE_ValidatePartition(PendedPartition);
					deadweight = prevtotal - currtotal - flatweight;
					assert(deadweight >= 0);
					if (deadweight > 0) WAVE_PrependDeadweight(wp, PendedPartition2, deadweight);
					wp = WAVE_ValidatePartition(PendedPartition2);
				}
				break;
			case WAVE_StageSubsiding:
				if (WAVE_StageProgress > 0)
				{
					WAVE_TransformPartition(wp, TransformedPartition, 1, WAVE_MESHWIDTH * WAVE_StageProgress);
					WAVE_ValidatePartition(TransformedPartition);
					prevtotal = WAVE_TotalWeight(TransformedPartition);
					WAVE_RestrictPartition(TransformedPartition, WAVE_MeshMinX, WAVE_MeshMaxX);
					wp = WAVE_ValidatePartition(TransformedPartition);
					currtotal = WAVE_TotalWeight(wp);
					flatweight = prevtotal * flatfrac * WAVE_StageProgress;
					assert(flatweight > 0);
					WAVE_PrependPartition(wp, PendedPartition, WAVE_MeshMinX, flatweight);
					wp = WAVE_ValidatePartition(PendedPartition);
					deadweight = prevtotal - currtotal - flatweight;
					assert(deadweight >= 0);
					if (deadweight > 0) WAVE_AppendDeadweight(wp, PendedPartition2, deadweight);
					wp = WAVE_ValidatePartition(PendedPartition2);
				}
				break;
			default:
				break;
			}
		}

		// Use pre-calibrated weighting for current perturb shape
		if (WaveDebug.SubdividePerturb)
		{
			WavePartition *SubdividePerturbPartition = WAVE_AllocPartition(mem, wp->N + WAVE_Perturb->partition.N);
			assert(mem < memend);

			switch (WAVE_PerturbStage)
			{
			case WAVE_PerturbStageDo:
				WAVE_RefinePartitions(wp, &WAVE_Perturb->partition, SubdividePerturbPartition,
					WAVE_RefineBlend, WAVE_PerturbProgress);
				wp = WAVE_ValidatePartition(SubdividePerturbPartition);
				break;
			case WAVE_PerturbStageHold:
			case WAVE_PerturbStageCollapse:
			case WAVE_PerturbStageWait:
				wp = &WAVE_Perturb->partition;
				break;
			case WAVE_PerturbStageUndo:
				WAVE_RefinePartitions(wp, &WAVE_Perturb->partition, SubdividePerturbPartition,
					WAVE_RefineBlend, 1 - WAVE_PerturbProgress);
				wp = WAVE_ValidatePartition(SubdividePerturbPartition);
				break;
			default:
				break;
			}
		}
	}

	// Increase weight near surfer position
	if (WaveDebug.SubdivideX)
	{
		float boost = WAVE_SubdivideBoostX;

#ifndef TARGET_XBOX
		// This subdivision causes flickering during the building stage
		switch (WAVE_Stage)
		{
		case WAVE_StageBuilding:
			boost = 1;	// no boost
			break;
		case WAVE_StageStable:
			boost = 1 + (WAVE_SubdivideBoostX - 1) * min(WAVE_StageTimer, 1.f);
			break;
		default:
			break;
		}
#endif

		kellyslater_controller *controller;
		if ((controller = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())) != 0)	// add second player later (multiplayer fixme)
		{
			float x = controller->get_board_controller().rb->pos.x;
x = min(max(x, WAVE_MeshMinX + 0.1f), WAVE_MeshMaxX - 0.1f);	// surfer position bug (dc 11/21/01)
			if (WAVE_LeftBreaker)
			{
				x = WAVE_MeshMinX + WAVE_MeshMaxX - x;
			}

			WavePartition *SurferPartition = WAVE_AllocPartition(mem, 4);
			assert(mem < memend);
			SurferPartition->guide[0] = wp->guide[0];
			SurferPartition->guide[1] = max(wp->guide[0] + 0.1f, x - WAVE_SubdivideRangeX / 2);
			SurferPartition->guide[2] = min(wp->guide[wp->N-1] - 0.1f, x + WAVE_SubdivideRangeX / 2);
			SurferPartition->guide[3] = wp->guide[wp->N-1];

			SurferPartition->guidestep[0] = SurferPartition->guide[1] - SurferPartition->guide[0];
			SurferPartition->guidestep[1] = SurferPartition->guide[2] - SurferPartition->guide[1];
			SurferPartition->guidestep[2] = SurferPartition->guide[3] - SurferPartition->guide[2];

			SurferPartition->weight[0] = 1;
			SurferPartition->weight[1] = boost;
			SurferPartition->weight[2] = 1;

			WavePartition *SubdivideXPartition = WAVE_AllocPartition(mem, wp->N + SurferPartition->N);
			assert(mem < memend);

			WAVE_RefinePartitions(wp, SurferPartition, SubdivideXPartition);

			wp = WAVE_ValidatePartition(SubdivideXPartition);

/*	Try at incorporating camera position
			// trace along the wave until you get off the screen
			float xlo, xhi;
			if (!WAVE_ScreenToX(z, 0, xlo)) xlo = WAVE_MeshMinX;
			if (!WAVE_ScreenToX(z, WAVE_ScreenWidth, xhi)) xhi = WAVE_MeshMaxX;

			WavePartition *SurferPartition = WAVE_AllocPartition(mem, 5);
			assert(mem < memend);
			SurferPartition->guide[0] = wp->guide[0];
			SurferPartition->guide[1] = max(wp->guide[0], xlo);
			SurferPartition->guide[2] = x;
			SurferPartition->guide[3] = min(wp->guide[wp->N-1], xhi);
			SurferPartition->guide[4] = wp->guide[wp->N-1];

			SurferPartition->guidestep[0] = SurferPartition->guide[1] - SurferPartition->guide[0];
			SurferPartition->guidestep[1] = SurferPartition->guide[2] - SurferPartition->guide[1];
			SurferPartition->guidestep[2] = SurferPartition->guide[3] - SurferPartition->guide[2];
			SurferPartition->guidestep[3] = SurferPartition->guide[4] - SurferPartition->guide[3];

			SurferPartition->weight[0] = 1;
			SurferPartition->weight[1] = WAVE_SubdivideBoostX / 2;
			SurferPartition->weight[2] = WAVE_SubdivideBoostX / 2;
			SurferPartition->weight[3] = 1;

			WavePartition *SubdivideXPartition = WAVE_AllocPartition(mem, wp->N + SurferPartition->N);
			assert(mem < memend);

			WAVE_RefinePartitions(wp, SurferPartition, SubdivideXPartition, WAVE_RefineConcatenate);

			wp = WAVE_ValidatePartition(SubdivideXPartition);
*/
		}
	}

	WAVE_SubdivideGrid(wp, WAVE_MeshGridX, WAVE_MeshStepX);

	// For left breaker, flip grid, but still list in increasing order.
	if (WAVE_LeftBreaker)
	{
		for (xstep = 0; xstep <= xstepmax / 2; ++xstep)
		{
			float xtemp = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_MeshGridX[xstep];
			WAVE_MeshGridX[xstep] = WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_MeshGridX[xstepmax - xstep];
			WAVE_MeshGridX[xstepmax - xstep] = xtemp;
		}
	}

	// Subdivision based on z
	mem = _mem;
	wp = &WAVE_BaseControlPartition;

	if (WaveDebug.SubdivideZ)
	{
		kellyslater_controller *controller;
		if (((controller = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())) != 0) &&	// add second player later (multiplayer fixme)
			controller->get_board_controller().center_hint_valid)
		{
			const WavePositionHint &hint = controller->get_board_controller().wave_center_hint;
			const float &z = hint.z;
			assert(z >= WAVE_CONTROLMINZ && z <= WAVE_CONTROLMAXZ);

			WavePartition *SurferPartition = WAVE_AllocPartition(mem, 4);
			assert(mem < memend);
			SurferPartition->guide[0] = WAVE_CONTROLMINZ;
			SurferPartition->guide[1] = max(WAVE_CONTROLMINZ + 0.1f, z - WAVE_SubdivideRangeZ / 2);
			SurferPartition->guide[2] = min(WAVE_CONTROLMAXZ - 0.1f, z + WAVE_SubdivideRangeZ / 2);
			SurferPartition->guide[3] = WAVE_CONTROLMAXZ;

			SurferPartition->guidestep[0] = SurferPartition->guide[1] - SurferPartition->guide[0];
			SurferPartition->guidestep[1] = SurferPartition->guide[2] - SurferPartition->guide[1];
			SurferPartition->guidestep[2] = SurferPartition->guide[3] - SurferPartition->guide[2];

			SurferPartition->weight[0] = 1;
			SurferPartition->weight[1] = WAVE_SubdivideBoostZ;
			SurferPartition->weight[2] = 1;

			WavePartition *SubdivideZPartition = WAVE_AllocPartition(mem, wp->N + SurferPartition->N);
			assert(mem < memend);

			WAVE_RefinePartitions(wp, SurferPartition, SubdivideZPartition);

			wp = WAVE_ValidatePartition(SubdivideZPartition);
		}
	}

	WAVE_SubdivideGrid(wp, WAVE_ControlGridZ, WAVE_MeshStepZ);

	zcell = 0;
	for (zstep = 0; zstep <= zstepmax; ++zstep)
	{
		while (WAVE_ControlGridZ[zstep] > WAVE_ControlZ[zcell+1] && zcell+1 < WAVE_ControlPoints->numcontrol) ++zcell;
		WAVE_HintGridZ[zstep] = zcell;
		WAVE_MeshGridZ[zstep] = SPLINE_Evaluate(WAVE_ControlZ,
			WAVE_ControlToWorldCoeffs.a,
			WAVE_ControlToWorldCoeffs.b,
			WAVE_ControlToWorldCoeffs.c,
			WAVE_ControlToWorldCoeffs.d,
			WAVE_ControlPoints->numcontrol, WAVE_ControlGridZ[zstep], WAVE_HintGridZ[zstep]);
		assert(zstep == 0 || WAVE_MeshGridZ[zstep] > WAVE_MeshGridZ[zstep-1]);
	}
STOP_PROF_TIMER(proftimer_wave_compute_grid);
}

static void WAVE_ComputeSlices(void)
{
#ifdef DEBUG
	if (WaveDebug.BreakTest)
	{
		int numused;
		for (
			numused = WAVE_PerturbTest.num;
			fabsf(WAVE_PerturbTest.profilex[numused-1] - WAVE_PerturbTest.profilex[numused-2]) < 0.00001;
			--numused
		) continue;
		assert(numused > 0);
		WAVE_PerturbTest.numused = numused;

		WAVE_PerturbTest.Init();

		for (u_int i = 0; i <= WAVE_MeshStepX; ++i)
		{
			WAVE_SliceGridX[i] = WAVE_TestSlice(WAVE_MeshGridX[i]);
		}
	}
	else
#endif
	{
		// Lock edges to zero
		WAVE_SliceGridX[0] = WAVE_LeftBreaker ? WAVE_MeshMaxX : WAVE_MeshMinX;
		for (u_int i = 1; i < WAVE_MeshStepX; ++i)
		{
			WAVE_SliceGridX[i] = WAVE_WorldToProfile(WAVE_MeshGridX[i]);
		}
		WAVE_SliceGridX[WAVE_MeshStepX] = WAVE_LeftBreaker ? WAVE_MeshMinX : WAVE_MeshMaxX;
	}
}

#ifdef DEBUG
static void WAVE_DumpProfile(void)
{
	u_int i;
	WaveVertInfo *wvi = WAVE_ProfileVertInfo;

	for (i = 0; i < WAVE_MeshStepZ; ++i, wvi += 4)
	{
		debug_print("x = %4.4f, y = %4.4f, z = %4.4f\n", wvi->x, wvi->y, wvi->z);
	}
}

static void WAVE_DumpProfileValues(void)
{
	debug_print("\n");
	debug_print("__FUNCTION__");
	for (u_int xstep = 0; xstep < WAVE_MeshStepX; ++xstep)
	{
		float x = WAVE_MeshGridX[xstep];
		float profilex = WAVE_SliceGridX[xstep];
		debug_print("x = %4.4f, profilex = %4.4f\n", x, profilex);
	}
	debug_print("\n");
}

#ifdef TARGET_XBOX
#include "ngl_xbox_internal.h"	// For struct nglVertexBasic

//static
nglTexture *WAVE_CreateBumpMap(const char *HeightMapName)
{
	nglTexture *HeightMap = nglLoadTexture(HeightMapName);

	assert(HeightMap);

    D3DLOCKED_RECT lock;
	uint32 *SrcBits;
	if (HeightMap->Type == NGLTEX_TGA)
	{
		SrcBits = new uint32[HeightMap->Width * HeightMap->Height];
		HeightMap->DXTexture.Simple->LockRect(0, &lock, 0, 0);
		XGUnswizzleRect(lock.pBits, HeightMap->Width, HeightMap->Height, NULL, SrcBits, lock.Pitch, NULL, 4);
		HeightMap->DXTexture.Simple->UnlockRect(0);
	}
	else if (HeightMap->Type == NGLTEX_DDS)
	{
	}
	else
	{
		assertmsg(false, "Unsupported texture type!\n");
	}

/*
static uint32 SrcBits[] = {
#include "pSrcBits.txt"
};
*/

	uint32 BumpMapFormat = NGLTF_SWIZZLED;	// Not supported in main NGL: | NGLTF_V8U8;
	uint16 *DestBits = new uint16[HeightMap->Width * HeightMap->Height];

	nglLockTextureXB(HeightMap);

    for(int y = 0; y < HeightMap->Height; y++)
    {
		uint16 *DestRow = (uint16 *)((uint32)DestBits + HeightMap->Width * sizeof(uint16) * y);

        for(int x = 0; x < HeightMap->Width; x++)
        {
/*
extern uint32 KSNGL_GetTexel(nglTexture* Tex, int32 x, int32 y);
			uint32 SrcPixel00 = (int8) KSNGL_GetTexel(HeightMap, x, y);
			uint32 SrcPixel10 = (int8) KSNGL_GetTexel(HeightMap, (x + 1) % HeightMap->Width, y);	// texture must tile
			uint32 SrcPixelM0 = (int8) KSNGL_GetTexel(HeightMap, (x - 1) % HeightMap->Width, y);
			uint32 SrcPixel01 = (int8) KSNGL_GetTexel(HeightMap, x, min(y + 1, HeightMap->Height - 1));
			uint32 SrcPixel0M = (int8) KSNGL_GetTexel(HeightMap, x, max(y - 1, 0));
*/
#define GetTexel(x, y) SrcBits[y * HeightMap->Width + x]
uint32 SrcPixel00 = (int8) GetTexel(x, y);
uint32 SrcPixel10 = (int8) GetTexel((x + 1) % HeightMap->Width, y);
uint32 SrcPixelM0 = (int8) GetTexel((x - 1) % HeightMap->Width, y);
uint32 SrcPixel01 = (int8) GetTexel(x, min(y + 1, HeightMap->Height - 1));
uint32 SrcPixel0M = (int8) GetTexel(x, max(y - 1, 0));

			uint32 du = SrcPixelM0 - SrcPixel10;
			uint32 dv = SrcPixel0M - SrcPixel01;

			if((SrcPixel00 < SrcPixelM0) && (SrcPixel00 < SrcPixel10))  // If we are at valley
			{
				du = max(SrcPixelM0 - SrcPixel00, SrcPixel10 - SrcPixel00);  // Choose greater of 1st order diffs
			}

/*
static float TestFreqX = 30;
static float TestFreqY = 30;
du = (int8) (0x7f * sinf(TestFreqX * ((float)(x)) / HeightMap->Width));
dv = (int8) (0x7f * sinf(TestFreqY * ((float)(y)) / HeightMap->Height));
*/

			uint16 *DestPixel = (uint16 *)((uint32)DestRow + (x << 1));

			// The luminance bump value
			uint32 Luminance = (SrcPixel00 > 1) ? 63 : 127;

			switch (BumpMapFormat & 0xF00)
			{
/*	Not supported in main NGL
			case NGLTF_LVU655:
				*DestPixel = (uint16) (((du>>3) & 0x1f) << 0);
				*DestPixel |= (uint16) (((dv>>3) & 0x1f) << 5);
				*DestPixel |= (uint16) (((Luminance>>2) & 0x3f) << 10);
				break;
			case NGLTF_V8U8:
				*DestPixel = (uint16) (du << 0);
				*DestPixel |= (uint16) (dv << 8);
				break;
*/
			default:
				assert (false);
				break;
			}
        }
    }

	nglUnlockTextureXB(HeightMap);

	nglTexture *BumpMap = nglCreateTexture(BumpMapFormat, HeightMap->Width, HeightMap->Height);
//	nglLockTextureXB(BumpMap);

    BumpMap->DXTexture.Simple->LockRect(0, &lock, 0, 0);
    XGSwizzleRect(DestBits, lock.Pitch, NULL, lock.pBits, BumpMap->Width, BumpMap->Height,
                   NULL, XGBytesPerPixelFromFormat(D3DFMT_L6V5U5));
    BumpMap->DXTexture.Simple->UnlockRect(0);

//	nglUnlockTextureXB(BumpMap);

	delete [] (uint32 *) SrcBits;
	delete DestBits;

	nglReleaseTexture(HeightMap);

	return BumpMap;
}

static void WAVE_TestTexturing(void)
{
	static bool FirstTime = true;
	static nglTexture *TestBumpMapTex = NULL;
	if (FirstTime)
	{
		FirstTime = false;
		nglLoadTexture("envmap");
		nglLoadTexture("blank");
		TestBumpMapTex = WAVE_CreateBumpMap("wteb0000");
	}

	// Scale sphere
	static float TestScale = WAVE_DebugSphereScale;

	// Position sphere
	static nglVector TestSpherePos(20, 11, -10, 1);
/*
	const float &xcenter = TestSpherePos[0];
	const float &ycenter = TestSpherePos[1];
	const float &zcenter = TestSpherePos[2];
	const float &wcenter = TestSpherePos[3];
*/

/*	To display the sphere.  Needs reworking.
extern void WAVETEX_RedoMaterialForXbox( u_int scratchid );
	WAVETEX_RedoMaterialForXbox(ScratchID);
*/

	nglMaterial Material;
	memset(&Material, 0, sizeof(Material));
/*
static bool TestMap = true;
static bool TestAnim = false;
	Material.Flags |= NGLMAT_TEXTURE_MAP;
	if (TestMap)
	{
		if (TestAnim)
		{
			Material.Map = nglGetTexture("wtc");
		}
		else
		{
			Material.Map = nglGetTexture("wtez0000");
		}
	}
	else
	{
		Material.Map = nglGetTexture("blank");
	}

	Material.Flags &= ~NGLMAT_DETAIL_MAP;
	Material.DetailMap = NULL;

static bool TestEnvironmentMap = true;
	if (TestEnvironmentMap)
	{
		Material.Flags |= NGLMAT_ENVIRONMENT_MAP;
		Material.EnvironmentMap = nglGetTexture("envmap");
static uint32 TestEnvironmentMapBlendMode = NGLBM_CONST_ADDITIVE;
		Material.EnvironmentMapBlendMode = TestEnvironmentMapBlendMode;
static uint32 EnvironmentMapBlendModeConstant = 0x80;
		Material.EnvironmentMapBlendModeConstant = EnvironmentMapBlendModeConstant;
	}
	else
	{
		Material.Flags &= ~NGLMAT_ENVIRONMENT_MAP;
		Material.EnvironmentMap = NULL;
	}

	Material.Flags &= ~NGLMAT_LIGHT_MAP;
	Material.LightMap = NULL;

static bool TestBumpMap = true;
	if (TestBumpMap)
	{
		Material.Flags |= NGLMAT_BUMP_MAP;
		if (TestAnim)
		{
			Material.DetailMap = nglGetTexture("wtc");
		}
		else
		{
			Material.DetailMap = TestBumpMapTex;
		}
	}
	else
	{
		Material.Flags &= ~NGLMAT_BUMP_MAP;
		Material.DetailMap = NULL;
	}

static bool TestLight = false;
	if (TestLight)
	{
		Material.Flags |= NGLMAT_LIGHT;
	}
	else
	{
		Material.Flags &= ~NGLMAT_LIGHT;
	}
static bool TestAlpha = false;
	if (TestAlpha)
	{
		Material.Flags |= NGLMAT_ALPHA;
	}
	else
	{
		Material.Flags &= ~NGLMAT_ALPHA;
	}
static bool TestBackfaceCull = false;
	if (TestBackfaceCull)
	{
		Material.Flags |= NGLMAT_BACKFACE_CULL;
	}
	else
	{
		Material.Flags &= ~NGLMAT_BACKFACE_CULL;
	}
static bool TestBlend = false;
	if (TestBlend)
	{
		Material.MapBlendMode = NGLBM_BLEND;
	}
	else
	{
		Material.MapBlendMode = NGLBM_OPAQUE;
	}

	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//	KSNGL_ScratchSetMaterial(&Material);
*/

	// Establish constants used in sphere generation
	static DWORD dwNumSphereRings    = 15;
	static DWORD dwNumSphereSegments = 30;
	static DWORD dwNumSphereVertices = 2 * dwNumSphereRings * (dwNumSphereSegments+1);
	FLOAT fDeltaRingAngle = ( D3DX_PI / dwNumSphereRings );
	FLOAT fDeltaSegAngle  = ( 2.0f * D3DX_PI / dwNumSphereSegments );
	DWORD c = NGL_RGBA32(255, 255, 255, 255);

	static u_int TestSphereCount = dwNumSphereVertices;

/*	To trace the sphere out over time
TestSphereCount++;
if (TestSphereCount > dwNumSphereVertices) TestSphereCount = 3;
*/

	KSNGL_CreateScratchMesh(TestSphereCount, &Material, false,true);
	nglRenderParams &Params = DebugRenderParams[WAVE_NumDebugMeshes];
	Params.Flags = NGLP_NO_CULLING;
	nglMatrix &Transform = DebugTransform[WAVE_NumDebugMeshes];
	nglIdentityMatrix(Transform);
static float TestRotationSpeed = 1;
	nglVector Rotation(0, WAVE_GetTotalSec() * TestRotationSpeed, 0, 0);
	KSNGL_RotateMatrix(Transform, Transform, Rotation);
	KSNGL_TranslateMatrix(Transform, Transform, TestSpherePos);
	nglMesh *Mesh = NULL;	// Not supported in main NGL:	nglGetScratchMesh(ScratchID);

	nglMeshWriteStrip(TestSphereCount);

	// Generate the group of rings for the sphere
	u_int vertcount = 0;
	for( DWORD ring = 0; ring < dwNumSphereRings; ring++ )
	{
		FLOAT r0 = sinf( (ring+0) * fDeltaRingAngle );
		FLOAT r1 = sinf( (ring+1) * fDeltaRingAngle );
		FLOAT y0 = cosf( (ring+0) * fDeltaRingAngle );
		FLOAT y1 = cosf( (ring+1) * fDeltaRingAngle );

		// Generate the group of segments for the current ring
		for( DWORD seg = 0; seg < (dwNumSphereSegments+1); seg++ )
		{
			FLOAT x0 = sinf( seg * fDeltaSegAngle );
			FLOAT z0 = cosf( seg * fDeltaSegAngle );
			FLOAT x1 = sinf( seg * fDeltaSegAngle );
			FLOAT z1 = cosf( seg * fDeltaSegAngle );

			// Add two vertices to the strip which makes up the sphere
			// (using the transformed normal to generate texture coords)
			if (++vertcount > TestSphereCount) break;
			WAVERENDER_ScratchAddVertexPNCUVSTR(TestScale*r0*x0, TestScale*y0, TestScale*r0*z0, r0*x0, y0, r0*z0, c,
				-((FLOAT)seg)/dwNumSphereSegments, (ring+0)/(FLOAT)dwNumSphereRings,
				z0, 0, -x0, y0*x0, -r0, y0*z0, r0*x0, y0, r0*z0);
			if (++vertcount > TestSphereCount) break;
			WAVERENDER_ScratchAddVertexPNCUVSTR(TestScale*r1*x1, TestScale*y1, TestScale*r1*z1, r1*x1, y1, r1*z1, c,
				-((FLOAT)seg)/dwNumSphereSegments, (ring+1)/(FLOAT)dwNumSphereRings,
				z1, 0, -x1, y1*x1, -r1, y1*z1, r1*x1, y1, r1*z1);
		}
		if (vertcount > TestSphereCount) break;
	}

	if (WaveDebug.FakeCamera)
	{
		// Set the projection matrix --- this must come first for NGL Xbox!  (dc 04/14/02)
		static float StaticVFOV = NGL_PI/4;
		static float StaticCX = 640.f / 2;	// center of screen
		static float StaticCY = 480.f / 2;
		static float StaticNearZ = 0.1f;
		static float StaticFarZ = 25.1f;

		float hfov_in_radians = 2 * atanf(tanf(StaticVFOV / 2) * (float)nglGetScreenWidth() / (float)nglGetScreenHeight());
		float hfov = hfov_in_radians / (NGL_PI / 180.0f);

		ksnglSetPerspectiveMatrix(hfov, StaticCX, StaticCY, TestScale * StaticNearZ, TestScale * StaticFarZ);

		// Set the view matrix
		nglMatrix FakeWorldToView;
		static nglVector StaticEyePt(0.0f, 0.0f,-4.0f);
		static nglVector StaticLookatPt(0.0f, 0.0f, 0.0f);
		static nglVector UpVec(0.0f, 1.0f, 0.0f);
		nglVector EyePt(
			TestScale * StaticEyePt[0],	// + TestSpherePos[0],
			TestScale * StaticEyePt[1],	// + TestSpherePos[1],
			TestScale * StaticEyePt[2],	// + TestSpherePos[2],
			0
		);
		nglVector LookatPt(
			TestScale * StaticLookatPt[0],	// + TestSpherePos[0],
			TestScale * StaticLookatPt[1],	// + TestSpherePos[1],
			TestScale * StaticLookatPt[2],	// + TestSpherePos[2],
			0
		);
		EyePt[0] += TestSpherePos[0]; EyePt[1] += TestSpherePos[1]; EyePt[2] += TestSpherePos[2];
		LookatPt[0] += TestSpherePos[0]; LookatPt[1] += TestSpherePos[1]; LookatPt[2] += TestSpherePos[2];
//		nglSetCameraMatrix(FakeWorldToView, EyePt, LookatPt, UpVec);	// Not in main NGL
		nglSetWorldToViewMatrix(FakeWorldToView);
	}

	DebugMesh[WAVE_NumDebugMeshes] = nglCloseMesh();
	++WAVE_NumDebugMeshes;
}
#endif

#endif



#if 1
extern float WAVETEX_transcale;
extern int WAVETEX_transmin;
extern int WAVETEX_transmax;
extern int WAVETEX_transval;
//float transcale=8.0f;
//int transmin=64;
//int transmax=128;

inline u_int WAVE_Transparency( u_int rgba, float xx, float yy, float zz)
{
	#ifdef BUILD_DEBUG
	return (rgba & 0x00FFFFFF) | (WAVETEX_transval <<24);
	#else
	return rgba;
	#endif
}
#endif

#if 0
	// revived for reference purposes
inline u_int WAVE_Transparency( u_int rgba, float xx, float yy, float zz)
{
	if (!WaveDebug.TranparentWave) return rgba;

	float dx=xx-camx;
	float dz=zz-camz;
	float dsquared=(dx*dx)+(dz*dz);
	float d=sqrtf(dsquared);
	//nglPrintf("%8.3f  ",dsquared);
	//dsquared *= transcale;
	d *= WAVETEX_transcale;
	if ( yy > 1.0f )
		d += 128.0f; //*= (yy*yy);
	int ds;
	ds=(int) d; //squared;
	if (ds<WAVETEX_transmin) ds=WAVETEX_transmin;
	if (ds>WAVETEX_transmax) ds=WAVETEX_transmax;

	//return ds * 0x1000000;
	return (rgba & 0x00FFFFFF) | (ds <<24);
	//return (rgba & 0xFFFFFF00) | ds;
}
#endif


static inline void WAVE_CheckForEmitter(const WaveProfileCoeffs &wpc, float xmesh, float xslice, WaveEmitter *we)
{
	// figure out the locations of particle emitters

	// check for regular particles
	if (xslice > WAVE_EmitterStartCrashX && xslice <= WAVE_EmitterStartLipX)
	{
		we->type = WAVE_EM_SPLASH;
	}
	else if (xslice > WAVE_EmitterStartLipX && xslice < WAVE_EmitterStartCrestX)
	{
		we->type = WAVE_EM_TRICKLE;
		we->xfactor = 1 - (xslice - WAVE_EmitterStartLipX)/(WAVE_EmitterStartCrestX - WAVE_EmitterStartLipX);
	}
	else if (xslice > WAVE_EmitterStartCrestX && xslice < WAVE_EmitterEndX)
	{
		we->type = WAVE_EM_CREST;
		we->xfactor = 1 - (xslice - WAVE_EmitterStartCrestX)/(WAVE_EmitterEndX - WAVE_EmitterStartCrestX);
	}
	else
	{
		we->type = WAVE_EM_NONE;
	}

	// check for particles which warn of upcoming break
/*	Temporary fix.  This is screwing up particles.  (dc 03/29/02)
	if (WAVE_BreakWarn)
	{
		if (
			(xmesh > WAVE_BreakWarnStartX && xmesh < WAVE_BreakWarnStopX) &&
			(we->type != WAVE_EM_SPLASH)
		)
		{
			we->type = WAVE_EM_WARN;
		}
	}
*/

	if (we->type > WAVE_EM_NONE)
	{
		we->x = we->crestx = xmesh;
		WAVE_Apply(wpc, WAVE_EmitterZ, WAVE_EmitterHint,
			we->y, we->z);
		WAVE_Apply(wpc, WAVE_EmitterCrestZ, WAVE_EmitterCrestHint,
			we->cresty, we->crestz);
	}
	else
	{
		// Should be removed.  This data is being used by ksfx, but it shouldn't be.
		we->x = we->crestx = xmesh;	// crash_draw_intervals reads this even when emitter type is NONE! (dc 03/29/02)
		WAVE_Apply(wpc, WAVE_EmitterZ, WAVE_EmitterHint,
			we->y, we->z);
		WAVE_Apply(wpc, WAVE_EmitterCrestZ, WAVE_EmitterCrestHint,
			we->cresty, we->crestz);
	}
}

static inline void WAVE_CheckForSound(const WaveProfileCoeffs &wpc, u_int xstep, float xmesh)
{
	// figure out the locations of sound emitters
	for (int k = 0; k < WAVE_SE_MAX; ++k)
	{
		WaveSoundEmitter &wse = WAVE_SoundEmitter[k];
		const u_int &numsegment = wse.numsegment;
		assert(numsegment <= WAVE_SE_LINE_MAX);

		for (u_int i = 0; i < numsegment; ++i)
		{
			SoundLine &segment = wse.segment[i];
			if (xstep == wse.startx[i])
			{
				vector3d vmin, vmax;
				vmin.x = vmax.x = xmesh;
				WAVE_Apply(wpc, wse.minz, wse.minhintz, vmin.y, vmin.z);
				WAVE_Apply(wpc, wse.maxz, wse.maxhintz, vmax.y, vmax.z);
				segment.start = (vmin + vmax) / 2;
				if (i == 0)
				{
					wse.line.start = (vmin + vmax) / 2;
				}
			}
			if (xstep == wse.stopx[i])
			{
				vector3d vmin, vmax;
				vmin.x = vmax.x = xmesh;
				WAVE_Apply(wpc, wse.minz, wse.minhintz, vmin.y, vmin.z);
				WAVE_Apply(wpc, wse.maxz, wse.maxhintz, vmax.y, vmax.z);
				segment.stop = (vmin + vmax) / 2;
				if (i == numsegment - 1)
				{
					wse.line.stop = (vmin + vmax) / 2;
				}
			}
		}
	}
}



#if defined(TARGET_XBOX)
static inline void WAVE_FillInVert(WaveVertInfo *wvi, float x, float y, float z, float nx, float ny, float nz,
	u_int rgba, float u, float v, float sx, float sy, float sz, float tx, float ty, float tz, float stx, float sty,
	float stz, char fa)
#elif defined(TARGET_GC)
static inline void WAVE_FillInVert(WaveVertInfo *wvi, float x, float y, float z, float nx, float ny, float nz,
	u_int rgba, u_int foamalpha, float u, float v)
#else
static inline void WAVE_FillInVert(WaveVertInfo *wvi, float x, float y, float z, float nx, float ny, float nz,
	u_int rgba, float u, float v)
#endif
{
	wvi->x = x;
	wvi->y = y;
	wvi->z = z;
	wvi->nx = nx;
	wvi->ny = ny;
	wvi->nz = nz;
	wvi->rgba = rgba; //WAVE_Transparency(rgba,x,y,z) ;	// too expensive on CPU
	wvi->u = u;
	wvi->v = v;
#ifdef TARGET_GC
	wvi->fa = foamalpha; //WAVE_Transparency(rgba,x,y,z) ;	// too expensive on CPU
#endif
#if defined(TARGET_XBOX)
	wvi->sx = sx;
	wvi->sy = sy;
	wvi->sz = sz;
	wvi->tx = tx;
	wvi->ty = ty;
	wvi->tz = tz;
	wvi->stx = stx;
	wvi->sty = sty;
	wvi->stz = stz;
	wvi->fa = fa;
#endif
}

#ifdef TARGET_PS2

inline unsigned int WAVE_VertColor(const wavecolor *color00ptr, const wavecolor *color01ptr, const wavecolor *color10ptr,
	const wavecolor *color11ptr, const float *interp)
{
	register u_long128	ans;

	__asm__ volatile (	// for bootable, "ACC" must be capitalized
	"
		.set noreorder

		lqc2 vf01, 0x00(%1)
		lqc2 vf02, 0x00(%2)
		lqc2 vf03, 0x00(%3)
		lqc2 vf04, 0x00(%4)
		lqc2 vf05, 0x00(%5)
		vmulax ACC, vf01, vf05
		vmadday ACC, vf02, vf05
		vmaddaz ACC, vf03, vf05
		vmaddw vf06, vf04, vf05
		vftoi0 vf06, vf06
		qmfc2 %0, vf06
		ppacb %0, %0, %0
		ppacb %0, %0, %0

		.set reorder
	"
	:	"=&r" (ans) 		// %0	// output
	:	"r" (color00ptr), 	// %1	// input
		"r" (color01ptr), 	// %2
		"r" (color10ptr), 	// %3
		"r" (color11ptr), 	// %4
		"r" (interp) 		// %5
	:	"memory"					// modified
	);

	return ans;
/*

	//a=128; //WAVE_ControlPoints->a[zbase][xbase];
	a=COLORADDMACRO(WAVE_ControlPoints->a,xbase,zbase,x0z0,x0z1,x1z0,x1z1);
	r=COLORADDMACRO(WAVE_ControlPoints->r,xbase,zbase,x0z0,x0z1,x1z0,x1z1);
	g=COLORADDMACRO(WAVE_ControlPoints->g,xbase,zbase,x0z0,x0z1,x1z0,x1z1);
	b=COLORADDMACRO(WAVE_ControlPoints->b,xbase,zbase,x0z0,x0z1,x1z0,x1z1);

u_long128 check = ans;
u_int diffa = a - ((*(u_int *) &ans & 0xff000000) >> 24);
u_int diffb = b - ((*(u_int *) &ans & 0x00ff0000) >> 16);
u_int diffg = g - ((*(u_int *) &ans & 0x0000ff00) >> 8);
u_int diffr = r - ((*(u_int *) &ans & 0x000000ff) >> 0);
assert(diffa == 1 || diffa == 0);
assert(diffb == 1 || diffb == 0);
assert(diffg == 1 || diffg == 0);
assert(diffr == 1 || diffr == 0);

	return (a<<24)+(b<<16)+(g<<8)+(r<<0);
	//return WAVE_ControlPoints->abgr[zbase][xbase];
*/
}

#else	//	#ifdef TARGET_PS2

#define COLORADDMACRO(comp00,comp01,comp10,comp11,x0z0,x0z1,x1z0,x1z1) ((int) ( \
  ( comp00 * x0z0 ) + \
  ( comp01 * x0z1 ) + \
  ( comp10 * x1z0 ) + \
  ( comp11 * x1z1 ) ) )

#ifdef TARGET_GC


extern float WAVE_camx;
extern float WAVE_camy;
extern float WAVE_camz;
float nglInnerProduct( const nglVector lhs, const nglVector rhs );
u_int WAVETEX_GCNearColor( void );
u_int WAVETEX_GCFarColor( void );
float WAVETEX_GCNFFadeScale( void );
float WAVETEX_GCNFFadeOffset( void );

float WAVE_GCLightDarkCalc( float nx, float ny, float nz, float x, float y, float z, float scale, float offset )
{

	nglVector norm; norm[0]=nx; norm[1]=ny; norm[2]=nz;
	nglVector camv; camv[0]=WAVE_camx-x; camv[1]=WAVE_camy-y; camv[2]=WAVE_camz-z;
	nglVector camn;
	nglNormalize( camn, camv );
	float rv=nglInnerProduct( camn, norm );

	rv*=scale;
	rv+=offset;
	if ( rv < 0.0f ) rv=0.0f;
	if ( rv > 1.0f ) rv=1.0f;
	return rv;
}


inline unsigned int WAVE_VertColor(const wavecolor *color00ptr, const wavecolor *color01ptr, const wavecolor *color10ptr,
	const wavecolor *color11ptr, const float *interp,
	float lightdark, wavecolor vertcolornear, wavecolor vertcolorfar )
{
	float ta,tr,tg,tb;
	ta=1.0f; //((lightdark*vertcolornear.a)+((1.0f-lightdark)*vertcolorfar.a))/2.0f;
	//tr=2.0f * (((lightdark)*(vertcolornear.r))+((1.0f-lightdark)*(vertcolorfar.r)));
	//tg=2.0f * (((lightdark)*(vertcolornear.g))+((1.0f-lightdark)*(vertcolorfar.g)));
	//tb=2.0f * (((lightdark)*(vertcolornear.b))+((1.0f-lightdark)*(vertcolorfar.b)));
	tr=(((lightdark)*(vertcolornear.r))+((1.0f-lightdark)*(vertcolorfar.r)));
	tg=(((lightdark)*(vertcolornear.g))+((1.0f-lightdark)*(vertcolorfar.g)));
	tb=(((lightdark)*(vertcolornear.b))+((1.0f-lightdark)*(vertcolorfar.b)));

	int a,r,g,b;

	a=COLORADDMACRO(color00ptr->a,color01ptr->a,color10ptr->a,color11ptr->a,interp[0],interp[1],interp[2],interp[3]);
	r=COLORADDMACRO(color00ptr->r,color01ptr->r,color10ptr->r,color11ptr->r,interp[0],interp[1],interp[2],interp[3]);
	g=COLORADDMACRO(color00ptr->g,color01ptr->g,color10ptr->g,color11ptr->g,interp[0],interp[1],interp[2],interp[3]);
	b=COLORADDMACRO(color00ptr->b,color01ptr->b,color10ptr->b,color11ptr->b,interp[0],interp[1],interp[2],interp[3]);

	//a = (int) ( (float) a * ta );
	r = (int) ( (float) r * tr );
	g = (int) ( (float) g * tg );
	b = (int) ( (float) b * tb );

		// why does this work
	if ( r>255 ) r=255;
	if ( g>255 ) g=255;
	if ( b>255 ) b=255;
		// and this doesn't?
	//a&=0xFF;
	//r&=0xFF;
	//g&=0xFF;
	//b&=0xFF;

	if (r<0) r=0;
	if (g<0) g=0;
	if (b<0) b=0;

	assert(a<256);
	assert(r<256);
	assert(g<256);
	assert(b<256);
	assert(a>=0);
	assert(r>=0);
	assert(g>=0);
	assert(b>=0);

#if 0 // shadow testing
	r=128;
	g=128;
	b=128;
	a=255;
#endif

	return (r<<24)+(g<<16)+(b<<8)+(a);
}

#else


inline unsigned int WAVE_VertColor(const wavecolor *color00ptr, const wavecolor *color01ptr, const wavecolor *color10ptr,
	const wavecolor *color11ptr, const float *interp)
{
	int a,r,g,b;

	a=COLORADDMACRO(color00ptr->a,color01ptr->a,color10ptr->a,color11ptr->a,interp[0],interp[1],interp[2],interp[3]);
	r=COLORADDMACRO(color00ptr->r,color01ptr->r,color10ptr->r,color11ptr->r,interp[0],interp[1],interp[2],interp[3]);
	g=COLORADDMACRO(color00ptr->g,color01ptr->g,color10ptr->g,color11ptr->g,interp[0],interp[1],interp[2],interp[3]);
	b=COLORADDMACRO(color00ptr->b,color01ptr->b,color10ptr->b,color11ptr->b,interp[0],interp[1],interp[2],interp[3]);
	return (a<<24)+(b<<16)+(g<<8)+(r<<0);
}


#endif

#if defined( TARGET_XBOX ) || defined(TARGET_GC)
inline unsigned int WAVE_VertFoamAlpha(const wavecolor *color00ptr, const wavecolor *color01ptr, const wavecolor *color10ptr,
	const wavecolor *color11ptr, const float *interp)
{
	int fa;

	fa=COLORADDMACRO(color00ptr->fa,color01ptr->fa,color10ptr->fa,color11ptr->fa,interp[0],interp[1],interp[2],interp[3]);
	return fa;
}
#endif

#endif	//	#ifdef TARGET_PS2 #else


#ifdef DEBUG
//static bool WAVEDEB_nointerpcolors=false;
#endif

float WAVE_FoamUFactor=1.0f;
float WAVE_FoamVFactor=1.0f;
float WAVE_FoamVSpeed=2.0f;

bool OverallFoam=false;
int OverallFoamval=0x80;

void uinttowavecolor( u_int ucolor, wavecolor &wcolor )
{
	wcolor.r= 2.0f * (float)((ucolor >> 24) & 0xFF) / 255.0f;
	wcolor.g= 2.0f * (float)((ucolor >> 16) & 0xFF) / 255.0f;
	wcolor.b= 2.0f * (float)((ucolor >> 8 ) & 0xFF) / 255.0f;
	wcolor.a= (float)((ucolor >> 0 ) & 0xFF) ;
}


static void WAVE_ComputeMesh(void)
{
START_PROF_TIMER(proftimer_wave_compute_mesh);
START_PROF_TIMER_DAVID(proftimer_generic_0);
	u_int xprofilebase=0;    // x is between this and this+1 in the profile array
	float xprofilediff=0.0f; // how close is it to xprofilebase+1
	u_int zcontrolbase=0;    // z is between this and this+1 in the control array
	float zcontroldiff=0.0f; // how close is it to zcontrolbase+1
	wavecolor (*color0ptr)[WAVE_NUMPROFILEMAX], (*color1ptr)[WAVE_NUMPROFILEMAX], *color00ptr, *color10ptr;

	u_int xstep, zstep;
	float xmesh, zmesh;
	float zcontrol;
	u_int zhint;
#ifdef DEBUG
	u_int xhintrounded, zhintrounded;
#endif
	float xslice;
#ifdef TARGET_XBOX
	u_int vertcolor = COLOR_GRAY;
	char vertfoamalpha = 0;
#else
	u_int vertcolor = COLOR_WHITE;
#endif
#ifdef TARGET_GC
	char vertfoamalpha = 0;
	int curbeach=g_game_ptr->get_beach_id();

	wavecolor vertcolornear;
	wavecolor vertcolorfar;
	uinttowavecolor( WAVETEX_GCNearColor(), vertcolornear);
	uinttowavecolor( WAVETEX_GCFarColor() , vertcolorfar );
	float gcfadescale=  WAVETEX_GCNFFadeScale();
	float gcfadeoffset= WAVETEX_GCNFFadeOffset();


#endif
#ifdef TARGET_XBOX
	float sx, sy, sz;
	float tx, ty, tz;
	float stx, sty, stz;
#endif
	DECLARE_SCRATCH(WaveProfileCoeffs, wpc);
	WaveVertInfo *wvi = WAVE_VertInfo;
#ifdef DEBUG
	const bool showregions = WaveDebug.ShowRegions;
	const bool drawgrindpath = WaveDebug.DrawGrindPath;
	bool computeprofile = WaveDebug.DrawProfile || WaveDebug.BreakTest;
	bool justdidprofile = false;
	const float mixxlocutoff = WAVE_MeshMinX + WAVE_MixMargin * WAVE_MESHWIDTH;
	const float mixxhicutoff = WAVE_MeshMaxX - WAVE_MixMargin * WAVE_MESHWIDTH;
	const float mixzlocutoff = WAVE_MeshMinZ + WAVE_MixMargin * WAVE_MESHDEPTH;
	const float mixzhicutoff = WAVE_MeshMaxZ - WAVE_MixMargin * WAVE_MESHDEPTH;
#endif

	WAVE_VertIndex = 0;	// reset vertex count

	//WAVE_CameraChecks(g_game_ptr->get_active_player());

STOP_PROF_TIMER_DAVID(proftimer_generic_0);

	for (xstep = 0; xstep <= WAVE_MeshStepX; ++xstep)
	{
START_PROF_TIMER_DAVID(proftimer_generic_1);
#ifdef DEBUG
		if (justdidprofile)
		{
			xstep = 0;
			wvi = WAVE_VertInfo;
			justdidprofile = false;
		}
		if (computeprofile)
		{
			xmesh = WAVE_DrawnProfileX;
			xslice = WAVE_DrawnProfileXX;

			xstep = WAVE_MeshStepX / 2;	// avoid clamping
			wvi = WAVE_ProfileVertInfo;
			justdidprofile = true;
			computeprofile = false;
		}
		else
#endif
		{
			xmesh = WAVE_MeshGridX[xstep];
			xslice = WAVE_SliceGridX[xstep];
		}

		xprofilebase = SPLINE_BinarySearch(WAVE_BaseProfile, WAVE_ControlPoints->numprofile, xslice);
		xprofilediff = (xslice - WAVE_BaseProfile[xprofilebase]) /
			(WAVE_BaseProfile[xprofilebase+1] - WAVE_BaseProfile[xprofilebase]);
		assert(xprofilediff>=0.0f);
		assert(xprofilediff<=1.001f); // can't just be 1 because of occasional round off errors
		assert(xprofilebase < WAVE_ControlPoints->numprofile - 1 || (xprofilebase < WAVE_ControlPoints->numprofile - 1 && xprofilediff == 0));

		color0ptr = WAVE_ControlPoints->color + xprofilebase;
		color1ptr = color0ptr + 1;

STOP_PROF_TIMER_DAVID(proftimer_generic_1);

START_PROF_TIMER_DAVID(proftimer_generic_2);
		WAVE_ComputeProfileCoeffs(xmesh, xslice, wpc);
STOP_PROF_TIMER_DAVID(proftimer_generic_2);

START_PROF_TIMER_DAVID(proftimer_generic_3);
#ifdef DEBUG
		if ( showregions || WaveDebug.Shadows )
		{
			xhintrounded = SPLINE_BinarySearch(WAVE_BaseProfile, WAVE_ControlPoints->numprofile, xslice);
			if (xhintrounded <= WAVE_ControlPoints->numprofile &&
				xslice - WAVE_BaseProfile[xhintrounded] > WAVE_BaseProfile[xhintrounded + 1] - xslice)
			{
				++xhintrounded;
			}
		}
#endif

		// Join wave surface to water surface at the seam
		bool doclampx;
		if (WaveDebug.Clamp)
		{
			doclampx = (xstep == 0) || (xstep == WAVE_MeshStepX);
		}
		else
		{
			doclampx = false;
		}
#ifdef DEBUG
		float mixx;
		bool domixx = false;
		float mixnx = 0, mixn;
		bool domixnx = false;
		if (WaveDebug.MixX)
		{
			if (xmesh < mixxlocutoff)
			{
				domixx = true;
				mixx = (xmesh - WAVE_MeshMinX) / (mixxlocutoff - WAVE_MeshMinX);
				if (WaveDebug.MixN)
				{
					domixnx = true;
					mixnx = mixx;
				}
			}
			else if (xmesh > mixxhicutoff)
			{
				domixx = true;
				mixx = (WAVE_MeshMaxX - xmesh) / (WAVE_MeshMaxX - mixxhicutoff);
				if (WaveDebug.MixN)
				{
					domixnx = true;
					mixnx = mixx;
				}
			}
		}
#endif
STOP_PROF_TIMER_DAVID(proftimer_generic_3);

		for (zstep = 0; zstep <= WAVE_MeshStepZ; ++zstep)
		{
START_PROF_TIMER_DAVID(proftimer_generic_4);
			float x, z;
			float xx, yy, zz;
			nglVector n;
			float &nx = n[0], &ny = n[1], &nz = n[2];	// guarantees correct alignment
#ifdef DEBUG
			nglVector nwater;
			float &nwaterx = nwater[0], &nwatery = nwater[1], &nwaterz = nwater[2];	// guarantees correct alignment
#endif
			float u, v;

			zcontrol = WAVE_ControlGridZ[zstep];
			zhint = WAVE_HintGridZ[zstep];
			zmesh = WAVE_MeshGridZ[zstep];

			zcontrolbase=(int) zcontrol;
			zcontroldiff=zcontrol-((float) zcontrolbase);
			assert(zcontroldiff>=0.0f);
			assert(zcontroldiff<=1.0f);
			assert(zcontrolbase < WAVE_ControlPoints->numcontrol - 1 || (zcontrolbase == WAVE_ControlPoints->numcontrol - 1 && zcontroldiff == 0));
			color00ptr = *color0ptr + zcontrolbase;
			color10ptr = *color1ptr + zcontrolbase;

// I don't know why I have to do this.  Why doesn't the __attribute__ thingy work
// on the xbox build?  It works in other files....
#if defined(TARGET_XBOX)
			float waveinterp[4] = {
#else
			float waveinterp[4] __attribute__((aligned(16))) = {
#endif // TARGET_XBOX
				(1.0f-xprofilediff)*(1.0f-zcontroldiff),
				(1.0f-xprofilediff)*(     zcontroldiff),
				(     xprofilediff)*(1.0f-zcontroldiff),
				(     xprofilediff)*(     zcontroldiff),
			};

			// Join wave surface to water surface at the seam
			bool doclampz;
			if (WaveDebug.Clamp)
			{
				doclampz = (zstep == 0) || (zstep == WAVE_MeshStepZ);
			}
			else
			{
				doclampz = false;
			}
#ifdef DEBUG
			float mixz;
			bool domixz = false;
			float mixnz = 0;
			bool domixnz = false;
			if (WaveDebug.MixZ)
			{
				if (zmesh < mixzlocutoff)
				{
					domixz = true;
					mixz = (zmesh - WAVE_MeshMinZ) / (mixzlocutoff - WAVE_MeshMinZ);
					if (WaveDebug.MixN)
					{
						domixnz = true;
						mixnz = mixz;
					}
				}
				else if (zmesh > mixzhicutoff)
				{
					domixz = true;
					mixz = (WAVE_MeshMaxZ - zmesh) / (WAVE_MeshMaxZ - mixzhicutoff);
					if (WaveDebug.MixN)
					{
						domixnz = true;
						mixnz = mixz;
					}
				}
			}
#endif
STOP_PROF_TIMER_DAVID(proftimer_generic_4);

START_PROF_TIMER_DAVID(proftimer_generic_5);
			x = xmesh;
			z = zcontrol;
			xx = x;
			if (doclampx || doclampz)
			{
				yy = WATER_Altitude(x, zmesh);
				zz = zmesh;
				WATER_Normal(x, zmesh, nx, ny, nz);
			}
			else
			{
START_PROF_TIMER_DAVID(proftimer_generic_6);
				WAVE_Apply(wpc, z, zhint, yy, zz);
STOP_PROF_TIMER_DAVID(proftimer_generic_6);
START_PROF_TIMER_DAVID(proftimer_generic_7);
				WAVE_Normal(wpc, z, zhint, nx, ny, nz);
STOP_PROF_TIMER_DAVID(proftimer_generic_7);
#ifdef DEBUG
				if (domixx)
				{
					yy = mixx * yy + (1 - mixx) * WATER_Altitude(x, zmesh);
					zz = mixx * zz + (1 - mixx) * zmesh;
				}
				if (domixnx || domixnz)
				{
					mixn = max(mixnx, mixnz);
					WATER_Normal(x, zmesh, nwaterx, nwatery, nwaterz);
					nx = mixn * nx + (1 - mixn) * nwaterx;
					ny = mixn * ny + (1 - mixn) * nwatery;
					nz = mixn * nz + (1 - mixn) * nwaterz;
				}
#endif
			}
START_PROF_TIMER_DAVID(proftimer_generic_8);
			WAVE_TextureCoord(x, zmesh, u, v);
STOP_PROF_TIMER_DAVID(proftimer_generic_8);
STOP_PROF_TIMER_DAVID(proftimer_generic_5);
#ifdef DEBUG
			if ( WaveDebug.Shadows )
#endif
			{
START_PROF_TIMER_DAVID(proftimer_generic_9);
#ifdef TARGET_GC
				float lightdark=WAVE_GCLightDarkCalc(nx,ny,nz,xx,yy,zz,gcfadescale,gcfadeoffset);

				vertcolor = WAVE_VertColor(color00ptr, color00ptr + 1, color10ptr, color10ptr + 1, waveinterp,
				            lightdark, vertcolornear, vertcolorfar );
				vertfoamalpha = WAVE_VertFoamAlpha(color00ptr, color00ptr + 1, color10ptr, color10ptr + 1, waveinterp );
#else
				vertcolor = WAVE_VertColor(color00ptr, color00ptr + 1, color10ptr, color10ptr + 1, waveinterp );
#endif
#ifdef TARGET_XBOX
				vertfoamalpha = WAVE_VertFoamAlpha(color00ptr, color00ptr + 1, color10ptr, color10ptr + 1, waveinterp );
#endif
STOP_PROF_TIMER_DAVID(proftimer_generic_9);
			}

#ifdef DEBUG
			if (showregions)
			{
				zhintrounded = zhint;
				if (zhintrounded <= WAVE_ControlPoints->numcontrol
					&& zcontrol - WAVE_ControlZ[zhintrounded] > WAVE_ControlZ[zhintrounded + 1] - zcontrol)
				{
					++zhintrounded;
				}
				vertcolor =	WAVE_RegionColor[WAVE_ControlPoints->region[zhintrounded][xhintrounded]];
			}
#endif
#ifdef TARGET_XBOX
			// Compute S, T, SXT for environmental bump map.
			// Not really accurate, and could be done this way in the vertex shader.
			sx = 1; sy = 0, sz = 0;
			float recipnormt = 1 / sqrtf(sqr(ny) + sqr(nz));	// never zero
			tx = 0; ty = -nz * recipnormt; tz = ny * recipnormt;
			stx = nx; sty = ny; stz = nz;
#endif
START_PROF_TIMER_DAVID(proftimer_generic_10);
#if defined(TARGET_XBOX)
			WAVE_FillInVert(wvi++, xx, yy, zz, nx, ny, nz, vertcolor, u, v, sx, sy, sz, tx, ty, tz,
				stx, sty, stz, vertfoamalpha);
#elif defined(TARGET_GC)
			WAVE_FillInVert(wvi++, xx, yy, zz, nx, ny, nz, vertcolor, vertfoamalpha, u, v);
#else
			WAVE_FillInVert(wvi++, xx, yy, zz, nx, ny, nz, vertcolor, u, v);
#endif
STOP_PROF_TIMER_DAVID(proftimer_generic_10);
		}

START_PROF_TIMER_DAVID(proftimer_generic_11);
#ifdef DEBUG
		if (drawgrindpath)
		{
			const SplineCoeffs<WAVE_NUMPROFILEMAX> &wgc = WAVE_GrindCoeffs;
			float grindz;
			float &grindxx = WAVE_GrindPath[xstep].x;
			float &grindyy = WAVE_GrindPath[xstep].y;
			float &grindzz = WAVE_GrindPath[xstep].z;
			grindxx = xmesh;
			grindz = SPLINE_Evaluate(WAVE_GrindGridX, wgc.a, wgc.b, wgc.c, wgc.d, WAVE_NumGrindCoeffs, xslice);
			WAVE_Apply(wpc, grindz, grindyy, grindzz);
		}
#endif

		WAVE_CheckForEmitter(wpc, xmesh, xslice, &WAVE_Emitter[xstep]);
		WAVE_CheckForSound(wpc, xstep, xmesh);
STOP_PROF_TIMER_DAVID(proftimer_generic_11);
	}

#ifdef DEBUG
	if (WaveDebug.DrawFloater)
	{
		WAVE_DrawFloater();
	}

	if (WaveDebug.DumpProfileMesh)
	{
		WAVE_DumpProfile();
		WaveDebug.DumpProfileMesh = 0;
	}
/*
WAVE_FindNearestStressTest();
*/
#endif
STOP_PROF_TIMER(proftimer_wave_compute_mesh);
}

bool WAVETEX_MultiMesh( void );

#ifdef TARGET_GC
void nglWaveWriteVertexPNCAUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, u_int Alpha, float U, float V );
#endif


void WAVE_FillInMesh(void)
{
START_PROF_TIMER(proftimer_wave_build_mesh);
	u_int xstep, sstep;
	WaveVertInfo *wvi0 = WAVE_VertInfo;
	WaveVertInfo *wvi1 = wvi0 + WAVE_MeshStepZ + 1;
	u_int strip;
#ifdef DEBUG
	u_int vertcount = 0;

	assert ((WAVE_StripsPerProfile * (WAVE_StripSize - 2)) == (2 * WAVE_MeshStepZ));	// profile divided evenly into strips
#endif

	for (xstep = 0; xstep < WAVE_MeshStepX; ++xstep, ++wvi0, ++wvi1)
	{
		for (strip = 0; strip < WAVE_StripsPerProfile; ++strip, --wvi0, --wvi1)
		{
START_PROF_TIMER_DAVID(proftimer_generic_17);
			nglMeshWriteStrip(WAVE_StripSize);
STOP_PROF_TIMER_DAVID(proftimer_generic_17);
#ifdef DEBUG
			vertcount += WAVE_StripSize;

			if (WaveDebug.ShowGridLines)
			{
				for (sstep = 0; sstep < WAVE_StripSize; sstep += 2, ++wvi0, ++wvi1)
				{
#ifdef TARGET_XBOX
					WAVERENDER_MeshFastWriteVertexPNCUVSTR(wvi0->x, wvi0->y, wvi0->z, wvi0->nx, wvi0->ny, wvi0->nz,
						WAVE_FunkyVertexColor[sstep % 4], wvi0->u, wvi0->v, wvi0->sx, wvi0->sy, wvi0->sz,
						wvi0->tx, wvi0->ty, wvi0->tz, wvi0->stx, wvi0->sty, wvi0->stz, wvi0->fa);
					WAVERENDER_MeshFastWriteVertexPNCUVSTR(wvi1->x, wvi1->y, wvi1->z, wvi1->nx, wvi1->ny, wvi1->nz,
						WAVE_FunkyVertexColor[(sstep+1) % 4], wvi1->u, wvi1->v, wvi1->sx, wvi1->sy, wvi1->sz,
						wvi1->tx, wvi1->ty, wvi1->tz, wvi1->stx, wvi1->sty, wvi1->stz, wvi1->fa);
#else
					nglMeshFastWriteVertexPNCUV(wvi0->x, wvi0->y, wvi0->z, wvi0->nx, wvi0->ny, wvi0->nz,
						WAVE_FunkyVertexColor[sstep % 4], wvi0->u, wvi0->v);
					nglMeshFastWriteVertexPNCUV(wvi1->x, wvi1->y, wvi1->z, wvi1->nx, wvi1->ny, wvi1->nz,
						WAVE_FunkyVertexColor[(sstep+1) % 4], wvi1->u, wvi1->v);
#endif
				}
			}
			else
#endif
			{
				for (sstep = 0; sstep < WAVE_StripSize; sstep += 2, ++wvi0, ++wvi1)
				{
#ifdef TARGET_XBOX
					WAVERENDER_MeshFastWriteVertexPNCUVSTR(wvi0->x, wvi0->y, wvi0->z, wvi0->nx, wvi0->ny, wvi0->nz,
						wvi0->rgba, wvi0->u, wvi0->v, wvi0->sx, wvi0->sy, wvi0->sz, wvi0->tx, wvi0->ty, wvi0->tz,
						wvi0->stx, wvi0->sty, wvi0->stz, wvi0->fa);
					WAVERENDER_MeshFastWriteVertexPNCUVSTR(wvi1->x, wvi1->y, wvi1->z, wvi1->nx, wvi1->ny, wvi1->nz,
						wvi1->rgba, wvi1->u, wvi1->v, wvi1->sx, wvi1->sy, wvi1->sz, wvi1->tx, wvi1->ty, wvi1->tz,
						wvi1->stx, wvi1->sty, wvi1->stz, wvi1->fa);
#elif defined(TARGET_GC)
					nglWaveWriteVertexPNCAUV(wvi0->x, wvi0->y, wvi0->z, wvi0->nx, wvi0->ny, wvi0->nz,
						wvi0->rgba, wvi0->fa, wvi0->u, wvi0->v);
					nglWaveWriteVertexPNCAUV(wvi1->x, wvi1->y, wvi1->z, wvi1->nx, wvi1->ny, wvi1->nz,
						wvi1->rgba, wvi1->fa, wvi1->u, wvi1->v);
#else
					nglMeshFastWriteVertexPNCUV(wvi0->x, wvi0->y, wvi0->z, wvi0->nx, wvi0->ny, wvi0->nz,
						wvi0->rgba, wvi0->u, wvi0->v);
					nglMeshFastWriteVertexPNCUV(wvi1->x, wvi1->y, wvi1->z, wvi1->nx, wvi1->ny, wvi1->nz,
						wvi1->rgba, wvi1->u, wvi1->v);
#endif
				}
			}
		}
	}
STOP_PROF_TIMER(proftimer_wave_build_mesh);
}

#ifdef DEBUG
static void WAVE_FillInProfile(const u_int color[6])
{
	nglRenderParams &rp = DebugRenderParams[WAVE_NumDebugMeshes];
	rp.Flags = NGLP_ZBIAS | NGLP_NO_CULLING;
	rp.ZBias = 10000;

	nglMatrix &Transform = DebugTransform[WAVE_NumDebugMeshes];
	nglIdentityMatrix(Transform);

	nglMaterial Mat;
	memset( &Mat, 0, sizeof(Mat) );
	Mat.Flags = NGLMAT_BILINEAR_FILTER;
	Mat.Flags |= NGLMAT_ALPHA;
	Mat.MapBlendMode = NGLBM_BLEND;
	Mat.Map = NULL;
	// Let's always draw this on top, since it's what we're interested in.  (dc 06/11/02)
	Mat.ForcedSortDistance = 0;

	KSNGL_CreateScratchMesh(WAVE_ProfileNumVert, &Mat, false);

#if NGL > 0x010700
#warning "Don't do this"
#else
	nglSetMeshFlags(NGLMESH_PERFECT_TRICLIP);
#endif

	nglMeshCalcSphere();	// Necessary for triclipping to work (dc 06/26/02)

	u_int zstep;
	float x0, x1, x2;
	WaveVertInfo *wvi0 = WAVE_ProfileVertInfo;
	WaveVertInfo *wvi1 = wvi0 + 1;
	u_int vertcount = 0;

	for (zstep = 0; zstep < WAVE_MeshStepZ; ++zstep, ++wvi0, ++wvi1)
	{
		x0 = wvi0->x - WaveDebug.ProfileWidth / 2;
		x1 = wvi0->x;
		x2 = wvi0->x + WaveDebug.ProfileWidth / 2;

		nglMeshWriteStrip(6);
		vertcount += 6;

		nglMeshFastWriteVertexPCUV(
			x0, wvi0->y + WaveDebug.ProfileYBias, wvi0->z,
			color[0],
			wvi0->u, wvi0->v
		);
		nglMeshFastWriteVertexPCUV(
			x0, wvi1->y + WaveDebug.ProfileYBias, wvi1->z,
			color[1],
			wvi1->u, wvi1->v
		);
		nglMeshFastWriteVertexPCUV(
			x1, wvi0->y + WaveDebug.ProfileYBias, wvi0->z,
			color[2],
			wvi0->u, wvi0->v
		);
		nglMeshFastWriteVertexPCUV(
			x1, wvi1->y + WaveDebug.ProfileYBias, wvi1->z,
			color[3],
			wvi1->u, wvi1->v
		);
		nglMeshFastWriteVertexPCUV(
			x2, wvi0->y + WaveDebug.ProfileYBias, wvi0->z,
			color[0],
			wvi0->u, wvi0->v
		);
		nglMeshFastWriteVertexPCUV(
			x2, wvi1->y + WaveDebug.ProfileYBias, wvi1->z,
			color[1],
			wvi1->u, wvi1->v
		);
	}

	assert(vertcount <= WAVE_ProfileNumVert);
	// Fill in any remaining vectors in scratch mesh (should be fixed later)
	while (vertcount < WAVE_ProfileNumVert)
	{
		// Add a pair of tris
		nglMeshWriteStrip(6);
		vertcount += 6;
		nglMeshFastWriteVertexPCUV(0, 0, 0, 0, 0, 0);
		nglMeshFastWriteVertexPCUV(0, 0, 0, 0, 0, 0);
		nglMeshFastWriteVertexPCUV(0, 0, 0, 0, 0, 0);
		nglMeshFastWriteVertexPCUV(0, 0, 0, 0, 0, 0);
		nglMeshFastWriteVertexPCUV(0, 0, 0, 0, 0, 0);
		nglMeshFastWriteVertexPCUV(0, 0, 0, 0, 0, 0);
	}

	DebugMesh[WAVE_NumDebugMeshes] = nglCloseMesh();
	++WAVE_NumDebugMeshes;
}

static void WAVE_DrawQuad(const nglVector &world, const u_int &color)
{
	const float margin = 20;
	const float a = ((float) ((color & 0xff000000) >> 24)) / 256;
	const float b = ((float) ((color & 0x00ff0000) >> 16)) / 256;
	const float g = ((float) ((color & 0x0000ff00) >> 8)) / 256;
	const float r = ((float) ((color & 0x000000ff) >> 0)) / 256;
  nglMatrix nglWorldToScreen;
  nglGetMatrix (nglWorldToScreen, NGLMTX_WORLD_TO_SCREEN);

	nglVector screen;
	nglApplyMatrix(screen, nglWorldToScreen, *(nglVector *) &world);	// Sony prototype doesn't use const

	float left = screen[0];
	float top = screen[1];
	float depth = screen[2];
	float w0 = screen[3];
	left /= w0;
	top /= w0;
	depth /= w0;
	left -= 2048 - nglGetScreenWidth() / 2;
// We used to need this line because of a bug in NGL (dc 07/10/01)
//	left /= ((float) nglGetScreenWidth()) / 640;
	top -= 2048 - nglGetScreenHeight() / 2;
	float right = left;
	float bottom = top;
	left -= margin;
	top -= margin;
	right += margin;
	bottom += margin;

	nglQuad q;
	nglInitQuad(&q);
	nglSetQuadRect(&q, left, top, right, bottom);
	nglSetQuadColor( &q, NGL_RGBA32( (u_int) (r * 255), (u_int) (g * 255), (u_int) (b * 255), (u_int) (a * 255) ) );
	nglSetQuadZ(&q, 1 /* depth */);	// z-units aren't the same as for meshes?
	nglListAddQuad(&q);
}

static void WAVE_DrawFloater(void)
{
	static vector3d position1(0, 0, 0), position2(0, 0, 0);
	static WavePositionHint hint1 = {0}, hint2 = {0};
	static WaveVelocityHint vhint1 = {0}, vhint2 = {0};
	static bool continuous = false;
	static WaveRegionEnum region1 = WAVE_REGIONFRONT, region2 = WAVE_REGIONFRONT;

	if (position1.x < WAVE_MeshMinX) {position1.x = WAVE_MeshMaxX; continuous = false;}
	else if (position1.x > WAVE_MeshMaxX) {position1.x = WAVE_MeshMinX; continuous = false;}
	if (position1.z < WAVE_MeshMinZ) {position1.z = WAVE_MeshMaxZ; continuous = false;}
	else if (position1.z > WAVE_MeshMaxZ) {position1.z = WAVE_MeshMinZ; continuous = false;}

	if (position2.x < WAVE_MeshMinX) {position1.x = WAVE_MeshMaxX; continuous = false;}
	else if (position2.x > WAVE_MeshMaxX) {position1.x = WAVE_MeshMinX; continuous = false;}
	if (position2.z < WAVE_MeshMinZ) {position1.z = WAVE_MeshMaxZ; continuous = false;}
	else if (position2.z > WAVE_MeshMaxZ) {position1.z = WAVE_MeshMinZ; continuous = false;}

	if (continuous)
	{
#if defined(TARGET_XBOX)
		WaveFloaterArgs wfa1(
			(WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_VELOCITYGIVEN | WAVE_REGIONSOUGHT),
			&hint1,
			vhint1,
			&position1,
			NULL,
			&region1);
		WaveFloaterArgs wfa2(
			(WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_VELOCITYGIVEN | WAVE_REGIONSOUGHT),
			&hint2,
			vhint2,
			&position2,
			NULL,
			&region2);
#else
		WaveFloaterArgs wfa1 = {
			(WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_VELOCITYGIVEN | WAVE_REGIONSOUGHT),
			&hint1,
			vhint1,
			&position1,
			NULL,
			&region1,
		};
		WaveFloaterArgs wfa2 = {
			(WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_VELOCITYGIVEN | WAVE_REGIONSOUGHT),
			&hint2,
			vhint2,
			&position2,
			NULL,
			&region2,
		};
#endif /* TARGET_XBOX JIV DEBUG */

		WAVE_TrackFloater(wfa1);
		WAVE_TrackFloater(wfa2);
	}
	else
	{
#if defined(TARGET_XBOX)
		WaveNearestArgs wna1(
			(WaveQueryFlags) (
				WAVE_YGIVEN |
				WAVE_HINTSOUGHT
			),
			position1,
			&position1,
			NULL,
			NULL,
			NULL,
			NULLREF(WavePositionHint),
			&hint1,
			NULLREF(WaveTolerance));
#else
		WaveNearestArgs wna1 = {
			(WaveQueryFlags) (
				WAVE_YGIVEN |
				WAVE_HINTSOUGHT
			),
			position1,
			&position1,
			NULL,
			NULL,
			NULL,
			NULLREF(WavePositionHint),
			&hint1,
			NULLREF(WaveTolerance),
		};
#endif /* TARGET_XBOX JIV DEBUG */
		WAVE_FindNearest(wna1);

		WAVE_GetVHint(&hint1, &hint2, 10, &vhint2);

		position2 = position1;
		hint2 = hint1;

		continuous = true;
	}

	nglVector world1(position1.x, position1.y, position1.z, 1);
	nglVector world2(position2.x, position2.y, position2.z, 1);

	WAVE_DrawQuad(world1, WAVE_RegionColor[region1]);
	WAVE_DrawQuad(world2, WAVE_RegionColor[region2]);
}

static void WAVE_DrawTubeThreshs()
{
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex ();
	extern nglMesh *nglSphereMesh;
	nglRenderParams Params;
	nglMatrix Work;
	nglVector v1, v2;

	static nglVector WAVE_TubeSphereColor(1, 0, 0, 1);
	static float WAVE_TubeSphereScale = 2;

	Params.Flags = NGLP_TINT | NGLP_SCALE;
	Params.TintColor = WAVE_TubeSphereColor;

	Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = WAVE_TubeSphereScale;

	nglIdentityMatrix(Work);

	int main_tube = 0;
	bool left = BeachDataArray[current_beach].bdir;
	if (WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment > 1 && left)
		main_tube = 1;
	if (left)
		v1[0] = (WaveDataArray[current_wave].firsttubethresh * (WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].start.x)) + WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].start.x;
	else
		v1[0] = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].stop.x - (WaveDataArray[current_wave].firsttubethresh * (WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].start.x));
	v1[1] = WaveDataArray[current_wave].tubecenstart_y;
	v1[2] = WaveDataArray[current_wave].tubecenstart_z;
	v1[3] = 1;

	KSNGL_TranslateMatrix( Work, Work, v1 );

	nglListAddMesh(nglSphereMesh, Work, &Params);

	Params.Flags = NGLP_TINT | NGLP_SCALE;
	Params.TintColor = WAVE_TubeSphereColor;
	Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = WAVE_TubeSphereScale;
	nglIdentityMatrix(Work);

	if (left)
		v2[0] = (WaveDataArray[current_wave].secondtubethresh * (WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].start.x)) + WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].start.x;
	else
		v2[0] = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].stop.x - (WaveDataArray[current_wave].secondtubethresh * (WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[main_tube].start.x));
	v2[1] = WaveDataArray[current_wave].tubecenstart_y;
	v2[2] = WaveDataArray[current_wave].tubecenstart_z;
	v2[3] = 1;
	KSNGL_TranslateMatrix( Work, Work, v2);
	nglListAddMesh(nglSphereMesh, Work, &Params);

	nglMaterial mat;
	memset(&mat, 0, sizeof (mat));
	mat.MapBlendMode = NGLBM_OPAQUE;
	// Let's always draw this on top, since it's what we're interested in.  (dc 06/11/02)
	mat.ForcedSortDistance = 0;

	nglRenderParams &rp = DebugRenderParams[WAVE_NumDebugMeshes];
	rp.Flags = NGLP_NO_CULLING;
	nglMatrix &Transform = DebugTransform[WAVE_NumDebugMeshes];
	nglIdentityMatrix(Transform);
	KSNGL_CreateScratchMesh(8, &mat, false);

#if NGL > 0x010700
#warning "pass me to that function above"
#else
	nglSetMeshFlags(NGLMESH_PERFECT_TRICLIP);
#endif

	u_int color = ((int) (WAVE_TubeSphereColor[3] * 255) << 24) +
		((int) (WAVE_TubeSphereColor[2] * 255) << 16) +
		((int) (WAVE_TubeSphereColor[1] * 255) << 8) +
		((int) (WAVE_TubeSphereColor[0] * 255) << 0);

	nglMeshWriteStrip(4);
	nglMeshFastWriteVertexPC(v1[0], v1[1] + 20, v1[2] + 15, color);
	nglMeshFastWriteVertexPC(v1[0], v1[1] + 20, v1[2] - 15, color);
	nglMeshFastWriteVertexPC(v1[0], v1[1] - 100, v1[2] + 15, color);
	nglMeshFastWriteVertexPC(v1[0], v1[1] - 100, v1[2] - 15, color);

	nglMeshWriteStrip(4);
	nglMeshFastWriteVertexPC(v2[0], v2[1] + 20, v2[2] + 15, color);
	nglMeshFastWriteVertexPC(v2[0], v2[1] + 20, v2[2] - 15, color);
	nglMeshFastWriteVertexPC(v2[0], v2[1] - 100, v2[2] + 15, color);
	nglMeshFastWriteVertexPC(v2[0], v2[1] - 100, v2[2] - 15, color);

	nglMeshCalcSphere();
	DebugMesh[WAVE_NumDebugMeshes] = nglCloseMesh();
	++WAVE_NumDebugMeshes;
}

#if defined(TARGET_XBOX) || defined(TARGET_GC)
static void WAVE_DrawLines(const vector3d *pt, u_int numpt, u_int color, float scale)
#else
static void WAVE_DrawLines(const vector3d *pt, u_int numpt, u_int color = COLOR_RED, float scale = 0)
#endif /* TARGET_XBOX JIV DEBUG */
{
	if (scale == 0) scale = WAVE_DebugSphereScale;

	extern nglMesh *nglSphereMesh;
	nglRenderParams Params;
	nglVector SphereColor(
		((float) ((color & 0x000000ff) >> 0)) / 255,
		((float) ((color & 0x0000ff00) >> 8)) / 255,
		((float) ((color & 0x00ff0000) >> 16)) / 255,
		((float) ((color & 0xff000000) >> 24)) / 128
	);

	Params.Flags = NGLP_TINT | NGLP_SCALE;

#if defined(TARGET_XBOX) || defined(TARGET_GC)
	memcpy( Params.TintColor, SphereColor, sizeof Params.TintColor );
#else
	Params.TintColor = SphereColor;
#endif /* TARGET_XBOX JIV DEBUG */
	Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = scale;

	if (numpt > 1)
	{
		nglRenderParams &rp = DebugRenderParams[WAVE_NumDebugMeshes];
		rp.Flags = NGLP_NO_CULLING;

		nglMatrix &Transform = DebugTransform[WAVE_NumDebugMeshes];
		nglIdentityMatrix(Transform);

		nglMaterial mat;
		memset(&mat, 0, sizeof (mat));
		mat.MapBlendMode = NGLBM_OPAQUE;
		// Let's always draw this on top, since it's what we're interested in.  (dc 06/11/02)
		mat.ForcedSortDistance = 0;

		KSNGL_CreateScratchMesh(8 * (numpt - 1), &mat, false);

#if NGL > 0x010700
#warning "do or do not.  there is no try."
#else
		nglSetMeshFlags(NGLMESH_PERFECT_TRICLIP);
#endif
	}

	for (u_int i = 0; i < numpt; ++i)
	{
		const vector3d &start = pt[i], &stop = pt[i+1];
		nglVector v1(start.x, start.y, start.z, 1), v2(stop.x, stop.y, stop.z, 1);
		nglMatrix Work;

		nglIdentityMatrix( Work );
		KSNGL_TranslateMatrix(Work, Work, v1);
		nglListAddMesh(nglSphereMesh, Work, &Params);

		if (i < numpt - 1)
		{
			nglMeshWriteStrip(4);
			nglMeshFastWriteVertexPC(v1[0], v1[1] - scale / 8, v1[2], color);
			nglMeshFastWriteVertexPC(v1[0], v1[1] + scale / 8, v1[2], color);
			nglMeshFastWriteVertexPC(v2[0], v2[1] - scale / 8, v2[2], color);
			nglMeshFastWriteVertexPC(v2[0], v2[1] + scale / 8, v2[2], color);

			nglMeshWriteStrip(4);
			nglMeshFastWriteVertexPC(v1[0], v1[1], v1[2] - scale / 8, color);
			nglMeshFastWriteVertexPC(v1[0], v1[1], v1[2] + scale / 8, color);
			nglMeshFastWriteVertexPC(v2[0], v2[1], v2[2] - scale / 8, color);
			nglMeshFastWriteVertexPC(v2[0], v2[1], v2[2] + scale / 8, color);
		}
	}

	if (numpt > 1)
	{
		nglMeshCalcSphere();
		DebugMesh[WAVE_NumDebugMeshes] = nglCloseMesh();
		++WAVE_NumDebugMeshes;
		assert(WAVE_NumDebugMeshes < WAVE_MAXDEBUGMESHES);
	}
}

#if defined(TARGET_XBOX) || defined(TARGET_GC)
static void WAVE_DrawLine(const vector3d &start, const vector3d &stop, u_int color, float scale)
#else
static void WAVE_DrawLine(const vector3d &start, const vector3d &stop, u_int color = COLOR_RED, float scale = 0)
#endif /* TARGET_XBOX JIV DEBUG */
{
	const vector3d pt[2] = {start, stop};
	WAVE_DrawLines(pt, 2, color, scale);
}

static void WAVE_PrintInfoToScreen(void)
{
#ifndef TARGET_GC
	static float WAVE_PrintInfoQuadZ = 0.2f;
	static float WAVE_PrintInfoQuadRight = nglGetScreenWidth() - 20.f;
	static float WAVE_PrintInfoQuadLeft = WAVE_PrintInfoQuadRight - 260.f;
//	static float WAVE_PrintInfoQuadTop = 200.f;
	static float WAVE_PrintInfoQuadBottom = nglGetScreenHeight() - 100.f;
	static u_int WAVE_PrintInfoQuadColor = ALPHA_HALFTONE;

	static float WAVE_PrintInfoTextTabx0 = 10;
	static float WAVE_PrintInfoTextTabx1 = 170;
	static float WAVE_PrintInfoTextTaby0 = 10;
	static float WAVE_PrintInfoTextCR = 12;
	static u_int WAVE_PrintInfoTextColor = COLOR_YELLOW;
//	static u_int WAVE_PrintInfoTextFont = 0;	// standard debug font
	static float WAVE_PrintInfoTextZ = 0.1f;

	const float &z = WAVE_PrintInfoQuadZ;
	const float &left = WAVE_PrintInfoQuadLeft;
	const float &right = WAVE_PrintInfoQuadRight;
//	const float &top = WAVE_PrintInfoQuadTop;
	const float &bottom = WAVE_PrintInfoQuadBottom;
	const u_int &color = WAVE_PrintInfoQuadColor;

	const float &tabx0 = WAVE_PrintInfoTextTabx0;
	const float &tabx1 = WAVE_PrintInfoTextTabx1;
	const float &taby0 = WAVE_PrintInfoTextTaby0;
	const float &cr = WAVE_PrintInfoTextCR;
	const u_int &tcolor = WAVE_PrintInfoTextColor;
	const float &fontz = WAVE_PrintInfoTextZ;

	float wavetimeleft = WAVE_ScheduleTimeEnd - WAVE_GetTotalSec();
	float stagetimeleft = WAVE_StageDuration[WAVE_Stage] -
		(WAVE_GetTotalSec() - WAVE_StageStart[WAVE_Stage]);
	float breaktimeleft;
	if (WAVE_PerturbStage == WAVE_PerturbStageNone)
	{
		breaktimeleft = WAVE_Perturb->start[WavePerturbStageEnum(1)] - WAVE_GetTotalSec();
	}
	else
	{
		breaktimeleft = -(WAVE_GetTotalSec() - WAVE_Perturb->start[WAVE_PerturbStage]);
		for (int stage = WAVE_PerturbStage; stage < WAVE_PerturbStageMax; ++stage)
		{
			breaktimeleft += WAVE_Perturb->duration[stage];
		}
	}
	bool nextbreakvalid = (breaktimeleft >= 0 && breaktimeleft <= WAVE_GetRemainingScheduleSec());

/*
extern float nglIFLSpeed;
#ifdef TARGET_PS2
	nglTexture *wavetex = nglGetTexture("wtcz");
	int wavetexframe = FTOI(TIMER_GetTotalSec() * nglIFLSpeed) % wavetex->Size;
#else
	nglTexture *wavetex = nglGetTexture("wtc");
	int wavetexframe = FTOI(TIMER_GetTotalSec() * nglIFLSpeed) % wavetex->NFrames;
#endif
*/

	struct infoline {
		const char *text;
		const enum {
			TYPE_CHAR,
			TYPE_FLOAT,
			TYPE_INT,
			TYPE_STRING,
			TYPE_STRINGA,
			TYPE_STRINGX,
		} type;
		const void *data;
	};
#define VOIDPTR(a) ((void *) &(a))
	const infoline WAVE_PrintInfo[] =
	{
		{
			"Wave: index",
			infoline::TYPE_INT,
			VOIDPTR(WAVE_ScheduleIndex)
		},
		{
			"Wave: scoring",
			infoline::TYPE_CHAR,
			VOIDPTR(WAVE_ScheduleArray[WAVE_ScheduleIndex].id)
		},
		{
			"Wave: geom",
			infoline::TYPE_STRINGX,
			VOIDPTR(WAVE_ScheduleType[WAVE_ScheduleArray[WAVE_ScheduleIndex].type].name)
		},
		{
			"Wave: breakmap",
			infoline::TYPE_STRINGX,
			VOIDPTR(WAVE_BreakArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].break_type].name)
		},
		{
			"Wave: data",
			infoline::TYPE_STRINGA,
			VOIDPTR(WaveDataArray[WAVE_ScheduleArray[WAVE_ScheduleIndex].wd_type].name)
		},
		{
			"Wave: scale",
			infoline::TYPE_FLOAT,
			VOIDPTR(WAVE_ScheduleType[WAVE_ScheduleArray[WAVE_ScheduleIndex].type].scale)
		},
		{
			"Wave: duration",
			infoline::TYPE_FLOAT,
			VOIDPTR(WAVE_ScheduleArray[WAVE_ScheduleIndex].duration)
		},
		{
			"Wave: time left",
			infoline::TYPE_FLOAT,
			VOIDPTR(wavetimeleft)
		},
		{
			"Stage: name",
			infoline::TYPE_STRING,
			VOIDPTR(WAVE_StageName[WAVE_Stage])
		},
		{
			"Stage: time left",
			infoline::TYPE_FLOAT,
			VOIDPTR(stagetimeleft)
		},
		{
			"Break: name",
			infoline::TYPE_STRING,
			WAVE_PerturbStage == WAVE_PerturbStageNone ?
				VOIDPTR(WAVE_PerturbStageName[WAVE_PerturbStage]) : // fake way of getting "none"
				VOIDPTR(WAVE_PerturbName[WAVE_PerturbType])
		},
		{
			"Break: phase",
			infoline::TYPE_STRING,
			VOIDPTR(WAVE_PerturbStageName[WAVE_PerturbStage])
		},
		{
			WAVE_PerturbStage == WAVE_PerturbStageNone ? "Break: time next" : "Break: time left",
			nextbreakvalid ? infoline::TYPE_FLOAT : infoline::TYPE_STRINGA,
			nextbreakvalid ? VOIDPTR(breaktimeleft) : VOIDPTR("n/a")
		},
/*
		{
			"Texture frame",
			infoline::TYPE_INT,
			VOIDPTR(wavetexframe)
		},
*/
	};

	const float top = bottom - (2 * taby0 + countof(WAVE_PrintInfo) * cr);

	nglQuad quad;
	nglInitQuad(&quad);
	nglSetQuadZ(&quad, z);
	nglSetQuadRect(&quad, left, top, right, bottom);
	nglSetQuadColor(&quad, color);
	nglListAddQuad(&quad);

	u_int i;
	float y;
	for (i = 0, y = top + taby0; i < countof(WAVE_PrintInfo); ++i, y += cr)
	{
		infoline line = WAVE_PrintInfo[i];
		nglListAddString(nglSysFont, left + tabx0, y, fontz, tcolor, line.text );
		switch (line.type)
		{
		case infoline::TYPE_CHAR:
			nglListAddString(nglSysFont, left + tabx1, y, fontz, tcolor, "%c", *(const char *) line.data);
			break;
		case infoline::TYPE_FLOAT:
			nglListAddString(nglSysFont, left + tabx1, y, fontz, tcolor, "%2.2f", *(const float *) line.data);
			break;
		case infoline::TYPE_INT:
			nglListAddString(nglSysFont, left + tabx1, y, fontz, tcolor, "%d", *(const int *) line.data);
			break;
		case infoline::TYPE_STRING:
			nglListAddString(nglSysFont, left + tabx1, y, fontz, tcolor, *(const char * const *)line.data);
			break;
		case infoline::TYPE_STRINGA:
			nglListAddString(nglSysFont, left + tabx1, y, fontz, tcolor, (const char *) line.data);
			break;
		case infoline::TYPE_STRINGX:
			nglListAddString(nglSysFont, left + tabx1, y, fontz, tcolor, ((const stringx *) line.data)->c_str());
			break;
		default:
			assert(0);
			break;
		}
	}
#endif
}

NOWARNING_UNUSED(WAVE_FindNearestStressTest)
static void WAVE_FindNearestStressTest(void)
{
  // Should not make calls to rand() or srand() - rbroner
	//srand((int) WAVE_GetTotalSec());

	float x = ((float) (random_r(WAVE_MESHWIDTH * 100) + ((int) (WAVE_MeshMinX * 100)))) / 100;
//	float y;
	float z = ((float) (random_r(WAVE_MESHDEPTH * 100) + ((int) (WAVE_MeshMinZ * 100)))) / 100;
	WaveProfileCoeffs wpc;

	WAVE_ComputeProfileCoeffs(x, wpc);

	vector3d position_in, position_out;

	position_in.x = x;
	position_in.y = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z);
//	position_in.y *= WAVE_ScaleSpatial;
	position_in.z = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z);

	WavePositionHint hintin = {
		x,
		z + (WAVE_CONTROLDEPTH / 100) * random_r(-1.0f, 1.0f),
		0,	// currently not used by WAVE_FindNearest
	};

	WaveTolerance tolerance;


#if defined(TARGET_XBOX)
	WaveNearestArgs wna(
		(WaveQueryFlags) (WAVE_YGIVEN | WAVE_HINTGIVEN | WAVE_NEARMATCH),
		position_in,
		&position_out,
		NULL,
		NULL,
		NULL,
		hintin,
		NULL,
		tolerance );
#else
	WaveNearestArgs wna = {
		(WaveQueryFlags) (WAVE_YGIVEN | WAVE_HINTGIVEN | WAVE_NEARMATCH),
		position_in,
		&position_out,
		NULL,
		NULL,
		NULL,
		hintin,
		NULL,
		tolerance,
	};
#endif /* TARGET_XBOX JIV DEBUG */
	WAVE_FindNearest(wna);
	if (fabsf(position_out.z-position_in.z) > 0.001)
	{
#ifndef EVAN
		debug_print("WAVE_FindNearest failed! Trying to get (%.6f,%.6f,%.6f), but got (%.6f,%.6f,%.6f).\n",
			position_in.x, position_in.y, position_in.z, position_in.x, position_in.y, position_in.z);
#endif
	}
}

/*	How far is a given point from the corresponding point on the mesh?
*/
void WAVE_VerifyHint(const WavePositionHint &hint, const vector3d &position)
{
	const float &x = hint.x;
	const float &z = hint.z;
	float profilex = WAVE_WorldToProfile(x);
	WaveProfileCoeffs wpc;
	WAVE_ComputeProfileCoeffs(x, profilex, wpc);

	for (u_int i = 0; i < WAVE_ControlPoints->numcontrol - 1; ++i)
	{
		assert(wpc.y.a[i] == hint.wpc.y.a[i]);
		assert(wpc.y.b[i] == hint.wpc.y.b[i]);
		assert(wpc.y.c[i] == hint.wpc.y.c[i]);
		assert(wpc.y.d[i] == hint.wpc.y.d[i]);
		assert(wpc.z.a[i] == hint.wpc.z.a[i]);
		assert(wpc.z.b[i] == hint.wpc.z.b[i]);
		assert(wpc.z.c[i] == hint.wpc.z.c[i]);
		assert(wpc.z.d[i] == hint.wpc.z.d[i]);
	}

	float yy = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z);
	float zz = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z);

	assert(x == position.x);
	assert(yy == position.y);
	assert(zz == position.z);
}

float WAVE_MeshError(const WavePositionHint &hint, const vector3d &position)
{
	WAVE_VerifyHint(hint, position);

	const float &x = hint.x, &z = hint.z;
	const u_int &xgrid = hint.xgrid, &zgrid = hint.zgrid;
	float xfrac = (x - WAVE_MeshGridX[xgrid]) / (WAVE_MeshGridX[xgrid+1] - WAVE_MeshGridX[xgrid]);
	float zfrac = (z - WAVE_ControlGridZ[zgrid]) / (WAVE_ControlGridZ[zgrid+1] - WAVE_ControlGridZ[zgrid]);
	float frac00 = (1 - xfrac) * (1 - zfrac);
	float frac01 = (1 - xfrac) * zfrac;
	float frac10 = xfrac * (1 - zfrac);
	float frac11 = xfrac * zfrac;

	WaveVertInfo *wvi00 = WAVE_VertInfo + xgrid * (WAVE_MeshStepZ + 1) + zgrid;
	WaveVertInfo *wvi10 = wvi00 + WAVE_MeshStepZ + 1;
	WaveVertInfo *wvi01 = wvi00 + 1;
	WaveVertInfo *wvi11 = wvi10 + 1;

	float xx = x;
	float yy = frac00 * wvi00->y + frac01 * wvi01->y + frac10 * wvi10->y + frac11 * wvi11->y;
	float zz = frac00 * wvi00->z + frac01 * wvi01->z + frac10 * wvi10->z + frac11 * wvi11->z;

	float dist = sqrtf(sqr(xx - position.x) + sqr(yy - position.y) + sqr(zz - position.z));

	static float WAVE_MeshErrorCutoff = 0.5f;
	if (dist > WAVE_MeshErrorCutoff)
	{
		debug_print("WAVE:\tDistance from wave to mesh = %f\n", dist);
	}

	return dist;
}
#endif

static void WAVE_BuildHint(WavePositionHint *hint, float x0, float profilex0, float z0,
	const WaveProfileCoeffs &wpc)
{
	hint->x = x0;
	hint->z = z0;
	hint->xprofile = profilex0;
	hint->zcell = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, z0);
	// This is only needed for WAVE_FindRegion for now
	hint->xcell = SPLINE_BinarySearch(WAVE_BaseProfile, WAVE_ControlPoints->numprofile, profilex0);
	// These are only needed for WAVE_TrackFloater for now
	hint->xgrid = SPLINE_BinarySearch(WAVE_MeshGridX, WAVE_MeshStepX + 1, x0);
	hint->zgrid = SPLINE_BinarySearch(WAVE_ControlGridZ, WAVE_MeshStepZ + 1, z0);
	if (&hint->wpc != &wpc)
	{
		hint->wpc = wpc;
	}
}

/*
static void WAVE_BuildHint(WavePositionHint *hint, float x0, float z0,
	const WaveProfileCoeffs &wpc)
{
	WAVE_BuildHint(hint, x0, WAVE_WorldToProfile(x0), z0, wpc);
}
*/

static void WAVE_BuildHint(WavePositionHint *hint, float x0, float z0)
{
	float profilex0 = WAVE_WorldToProfile(x0);
	DECLARE_SCRATCH(WaveProfileCoeffs, wpc);
	WAVE_ComputeProfileCoeffs(x0, profilex0, wpc);

	WAVE_BuildHint(hint, x0, profilex0, z0, wpc);
}

static inline WaveRegionEnum WAVE_FindRegion(const WavePositionHint &hint)
{
//	const float &x0 = hint.x;
	const float &z0 = hint.z;
	const float &profilex0 = hint.xprofile;
	u_int ix = hint.xcell;
	u_int iz = hint.zcell;

	// the x portion here is redundant with work done in WAVE_ComputeProfileCoeffs
	if (ix <= WAVE_ControlPoints->numprofile && profilex0 - WAVE_BaseProfile[ix] > WAVE_BaseProfile[ix + 1] - profilex0) ++ix;

//	u_int iz = SPLINE_BinarySearch(WAVE_ControlZ, WAVE_ControlPoints->numcontrol, z0);
	if (iz <= WAVE_ControlPoints->numcontrol && z0 - WAVE_ControlZ[iz] > WAVE_ControlZ[iz + 1] - z0) ++iz;

	WaveRegionEnum region = WAVE_ControlPoints->region[iz][ix];

	return region;
}

/*	WAVE_FindNearest()

	-----------------------------------------ALGORITHM-------------------------------------------

	Find the nearest point on the wave to the input point (roughly).  In other words, suppose we are
	given (xx,yy,zz), not necessarily on the wave, but probably close to it.  We want to find some
	point (x0,y0,z0) with f(x0,y0,z0) = (xx0,yy0,zz0) as close as possible to (xx,yy,zz), where f is
	the mapping of the flat water plane onto the wave shape.  The algorithm is a second-order iterative
	minimization of the distance function.

	First, we assume x0 = xx, since f preserves the x-coordinate.  (We implicitly make the same
	assumption again when we set xxnormal = 0).  This way, we only need to compute one profile and
	one set of spline coefficients.  The assumption doesn't necessarily give the closest point, but
	it's good enough for our purposes.  If (xx,yy,zz) is actually on the wave, then this value of x0
	is correct.

	Next, given a starting guess for z0, find the actual values of yy0 and zz0.  Then let F be the
	squared-distance to the target point, and compute F, and its derivatives dF, and ddF at z0.
	Imagine constructing a quadratic in z which matches the value, and first two derivatives, of F at z0.
	Then compute the value which minimizes this quadratic, and use it as the next guess for z0.

	This process is guaranteed to approach a value of z0 for which F is a local minimum, so it gives a
	point on the wave which is locally nearest to the input point.  We may miss the global minimum
	though.  For instance if the input point is in back of the wave, we may accidently find the closest
	point on the front of the wave instead.  Such instances can be avoided by making a smarter first
	guess for z.

	-----------------------------------------PROBLEMS--------------------------------------------

                                            _____@________
                                           /              \
                                          /                \
                                         /                  +
                                        /      _____         \
                                       /      /     \         \
                                      /      /       2         \
                                     /      /         \         \
                                    |      /           \         \                        ^
                                    |     /            |          \                       |
                                    |    /             |           \                      |
                                    |   |              |            \                     y   z -->
     ________________               |   |              |             \________________________
                     \              |   |              /
                      \             *   |             /
                       \____________3\__|_______1____/
                                      \ |
                                       \|

	--- Trying to get point @ by passing in x and z.  End up with point 1 instead, which has the
		same z value, but a different y value.  Would be fixed by passing in y as well, and setting the
		ygiven flag (see below).
	--- Trying to get point + by passing in x, y and z.  End up with point 2 instead, because it is a
		local minimum for distance to the target.  Would be probably be fixed by setting the nearmatch
		flag (see below).
	--- Trying to get point * by passing in x, y, and z.  End up with point 3 instead, because it has
		nearly the same location.  The position is correct, but the normal is wrong.  Would probably be
		fixed using a hint (see below).

	-----------------------------------------OPTIONS---------------------------------------------

	WAVE_YGIVEN:  If set, try to match a point in 3D space.  Otherwise, try to get above some point in
	the xz-plane, regardless of height.

	WAVE_NORMALSOUGHT:  If set, return approximate normal vector in xxnormal, yynormal, zznormal.

	WAVE_HINTGIVEN:  If set, the value of hintin is used to make the initial guess of z0.  Generally,
	the hint is a value returned from a previous call to this function, and you are trying to stay near
	the same position on the wave.  This option also helps you follow a path across places where the
	wave is self-intersecting.

	WAVE_HINTSOUGHT:  If set, return a hint in hintout.  This hint can be used in subsequent calls to
	keep the returned point near the same position.

	WAVE_NEARMATCH:  If set, we expect the input point to be very nearly on the wave.  If the answer we
	get is deemed insufficiently close, we switch to a first order method.  This method is less likely to
	jump over the best solution, but it is slower.

	WAVE_CURRENTSOUGHT:  If set, the argument called "current" is set to a vector giving the direction
	and magnitude of the current at the given point.  This vector is identical (so far) to the scroll
	speed of the water texture.

	WAVE_REGIONSOUGHT:  If set, the argument called "region" is filled in with the appropriate value of
	WaveRegionEnum for the given point.

	WAVE_VELOCITYGIVEN:  Not used by this function.

	WAVE_LIMITSGIVEN:  If set, the marker values frontlimit and backlimit are used to restrict the domain
	of the wave being searched.  Only z values (in grid space) between the front and back markers are
	considered.

	NOTE:  You can pass the same variables for xx, yy, zz and xxout, yyout, zzout.
*/
float WAVE_ToleranceDefaultD = 1.f, WAVE_ToleranceDefaultZ = 2.f;

bool WAVE_FindNearest(const WaveNearestArgs &wna)
{
START_PROF_TIMER(proftimer_wave_collide);
	bool success = true;
	const vector3d &position_in = wna.position_in;
	vector3d *position_out = wna.position_out;
	const float &xxin = position_in.x, &yyin = position_in.y, &zzin = position_in.z;
	float xx = xxin, yy = yyin, zz = zzin;
	float &xxout = position_out->x, &yyout = position_out->y, &zzout = position_out->z;
	const WaveQueryFlags &flags = wna.flags;
	const bool ygiven = (flags & WAVE_YGIVEN);
	const bool normalsought = (flags & WAVE_NORMALSOUGHT);
	const bool hintgiven = (flags & WAVE_HINTGIVEN);
	const bool hintsought = (flags & WAVE_HINTSOUGHT);
	const bool nearmatch = (flags & WAVE_NEARMATCH) || !ygiven;	// we can match x and z exactly
	const bool currentsought = (flags & WAVE_CURRENTSOUGHT);
	const bool regionsought = (flags & WAVE_REGIONSOUGHT);
	const bool limitsgiven = (flags & WAVE_LIMITSGIVEN);	// only match within a certain z0 range
	assert (!(flags & WAVE_VELOCITYGIVEN));	// not supported
	const WavePositionHint &hintin = wna.hintin;
	WavePositionHint localhint;
	WavePositionHint &hintout = hintsought ? *wna.hintout : localhint;

	float x0 = xx;
	float profilex0 = WAVE_WorldToProfile(x0);
	DECLARE_SCRATCH(WaveProfileCoeffs, wpc);
	WAVE_ComputeProfileCoeffs(x0, profilex0, wpc);

	float zfront, zback;
	if (limitsgiven)
	{
		// Restrict search to a z-range in grid space. (dc 05/01/02)
		assert(wna.frontlimit != WAVE_MarkerInvalid && wna.backlimit != WAVE_MarkerInvalid);
		zfront = WAVE_Marker[wna.frontlimit].z;
		zback = WAVE_Marker[wna.backlimit].z;
		assert(zfront <= zback);
	}
	else
	{
		zfront = WAVE_CONTROLMINZ;
		zback = WAVE_CONTROLMAXZ;
	}

	float yy0 = 0, yydelta = 0, dyy0 = 0, ddyy0 = 0;
	float zz0, zzdelta, dzz0, ddzz0;
	float F0, dF0 = (float) 1e10, ddF0;
	float zdelta = (float) 1e10;

	// we will stop the iteration when the adjustment is this small (in control units)
	static const float zdeltathresh = 0.0002f;
	// as far from the wave surface as "nearmatch" will allow (sqrd, in world units)
	const float Fthresh = ygiven ? (nearmatch ? sq(wna.tolerance.dthresh) : 0) : 0.0001f;
	// as far as we expect the guy to move in one increment (in world units)
	const float &z0thresh = sq(wna.tolerance.zthresh);
	float z0errorinit = -1e10, z0error = z0errorinit;	// force error if uninitialized

//	yy /= WAVE_ScaleSpatial;
	float z0;
	float dzz0prev;
	float xxprev, yyprev = 1e10, zzprev = 1e10;
	if (hintgiven)
	{
		z0 = hintin.z;
		const WaveProfileCoeffs &wpcprev = hintin.wpc;
		dzz0prev = SPLINE_EvaluateD(WAVE_ControlZ, wpcprev.z.a, wpcprev.z.b, wpcprev.z.c, wpcprev.z.d, WAVE_ControlPoints->numcontrol, z0);
		xxprev = hintin.x;
		yyprev = ygiven
			? SPLINE_Evaluate(WAVE_ControlZ, wpcprev.y.a, wpcprev.y.b, wpcprev.y.c, wpcprev.y.d,
				WAVE_ControlPoints->numcontrol, z0)
			: yyin;
		zzprev = SPLINE_Evaluate(WAVE_ControlZ, wpcprev.z.a, wpcprev.z.b, wpcprev.z.c, wpcprev.z.d,
			WAVE_ControlPoints->numcontrol, z0);

#ifdef DEBUG
		// Check that the input point matches the hint
		float distsq = sq(xxin - xxprev) + sq(yyin - yyprev) + sq(zzin - zzprev);
		if ((distsq > z0thresh) || (nearmatch && ygiven && distsq > Fthresh))
		{
			waveerror(("%s:%d\tHint does not match input position --- caller error.", __FILE__, __LINE__));
		}
#endif
	}
	else
	{
		z0 = zfront;
		dzz0prev = 0;	// not used in this case
	}
	float z0init = z0; // initial guess

	u_int numtry = 0;
	const static u_int basemaxtry = 25;
	static u_int maxtry = basemaxtry;
	bool done = false, checkdone = false, firstpass = true;

	// Need enough steps to traverse the mesh and then subdivide down to zdeltathresh
	const static float maxcontroldepth = 14;
	assert(maxcontroldepth >= WAVE_CONTROLDEPTH);
	const static float stepsizeinit = maxcontroldepth / basemaxtry;
	const static float log2_e = 1.4426950408889634073599246810019f;
	const static float log2_stepsizeinit = logf(stepsizeinit) * log2_e;
	const static float log2_zdeltathresh = logf(zdeltathresh) * log2_e;
	const u_int stepmaxtry = basemaxtry + (int) ceilf(log2_stepsizeinit - log2_zdeltathresh);

	float stepsize = stepsizeinit;
	bool smallsteps = false;
	float oldsign = 0, sign = 0, signswitched = false;
	bool hitfront = (z0init == zfront), hitback = (z0init == zback), cycled = false;

	do
	{
		if (ygiven)
		{
			yy0 = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z0);
			yydelta = yy0 - yy;
		}
//		else	// value alread set on initialization
//		{
//			yydelta = 0;
//		}

		zz0 = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z0);
		zzdelta = zz0 - zz;

		F0 = yydelta * yydelta + zzdelta * zzdelta;

		dzz0 = SPLINE_EvaluateD(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z0);

		if (firstpass && hintgiven)
		{
			assert(yyprev != 1e10 && zzprev != 1e10);
			float yyshift = ygiven ? yy0 - yyprev : 0;
			float zzshift = zz0 - zzprev;
			z0error = z0errorinit = sq(yyshift) + sq(zzshift);
			firstpass = false;
		}

		if (checkdone)	// the estimates have stabilized, but are we close enough
		{
			checkdone = false;
			if (
				F0 < Fthresh &&
				(!hintgiven || !ygiven ||
					sq(fabsf(z0 - z0init) * (fabsf(dzz0) + fabsf(dzz0prev)) / 2) < z0thresh + z0error
				)
			)
			{
				break;	// the derived point is close to the target point
			}
			if ((!hintgiven || !ygiven) && (!hitfront || !hitback))
			{
				z0 = hitfront ? (hitback = true, zback) : (hitfront = true, zfront);
				z0error = 0;
			}
			else if (!smallsteps)
			{
				smallsteps = true;	// try first order approach to avoid jumping past solution
				z0 = z0init;
				hitfront = (z0init == zfront);
				hitback = (z0init == zback);
				maxtry = stepmaxtry;
				z0error = z0errorinit;
			}
			else	// our initial guess led us to an undesired local minimum
			{
				waveerror(("%s:\tCouldn't find near match, bailing ...", __FUNCTION__));
				success = false;
				break;
			}

			// If we got here, reset and try again.
			numtry = 0;
			cycled = false;
			if (smallsteps)
			{
				sign = 0;
				stepsize = stepsizeinit;
				signswitched = false;
			}
			continue;
		}

		if (ygiven)
		{
			dyy0 = SPLINE_EvaluateD(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z0);
			ddyy0 = SPLINE_EvaluateDD(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z0);
		}
//		else	// value alread set on initialization
//		{
//			dyy0 = 0;
//			ddyy0 = 0;
//		}

		// moved earlier in loop
		// dzz0 = SPLINE_EvaluateD(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z0);
		ddzz0 = SPLINE_EvaluateDD(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, z0);

		dF0 = 2 * (yydelta * dyy0 + zzdelta * dzz0);
		ddF0 = 2 * (yydelta * ddyy0 + dyy0 * dyy0 + zzdelta * ddzz0 + dzz0 * dzz0);

		if (smallsteps)
		{
			oldsign = sign;
			sign = (dF0 > 0) ? 1  : -1;
			if (oldsign * sign < 0) signswitched = true; // sign == 0 first time through
			if (signswitched) stepsize /= 2;
			zdelta = -sign * stepsize;
		}
		else
		{
			if (ddF0 == 0)
			{
				zdelta = 0.1;	// arbitrary guess
			}
			else
			{
				zdelta = - dF0 / ddF0;
			}
		}
		z0 += zdelta;
		if (z0 < zfront)
		{
			z0 = zfront;
			if (hitfront)
			{
				cycled = true;
			}
			hitfront = true;
		}
		else if (z0 > zback)
		{
			z0 = zback;
			if (hitback)
			{
				cycled = true;
			}
			hitback = true;
		}

		if (numtry++ == maxtry || cycled)	// if cycled then we're caught in a loop (dc 05/01/02)
		{
			if ((!hintgiven || !ygiven) && (!hitfront || !hitback))
			{
				z0 = hitfront ? (hitback = true, zback) : (hitfront = true, zfront);
				z0error = 0;
			}
			else if (!smallsteps)
			{
				smallsteps = true;	// try first order approach to avoid jumping past solution
				z0 = z0init;
				hitfront = (z0init == zfront);
				hitback = (z0init == zback);
				maxtry = stepmaxtry;
				z0error = z0errorinit;
			}
			else
			{
				waveerror(("%s:\tCouldn't find match in %d steps, bailing ...", __FUNCTION__, maxtry));
				success = false;
				done = true;	// sanity check to avoid infinite loop
			}

			// If we got here, reset and try again.
			numtry = 0;
			cycled = false;
			if (smallsteps)
			{
				sign = 0;
				stepsize = stepsizeinit;
				signswitched = false;
			}
			continue;
		}
		else if (fabsf(zdelta) < zdeltathresh)
		{
			if (nearmatch)
			{
				checkdone = true;
			}
			else
			{
				done = true;	// the estimates are no longer changing much
			}
		}
	} while (!done);

	assert(z0 >= zfront && z0 <= zback);

	if (hintsought)
	{
		WAVE_BuildHint(&hintout, x0, profilex0, z0, wpc);
	}

	xxout = xx;
    if (!ygiven)
    {
		yy0 = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z0);
    }
	yyout = yy0; // * WAVE_ScaleSpatial;
	zzout = zz0;

	if (normalsought)
	{
		float &xxnormal = wna.normal->x, &yynormal = wna.normal->y, &zznormal = wna.normal->z;
		nglVector n;

		WAVE_Normal(wpc, z0, n[0], n[1], n[2]);	// this call involves some redundant work
		xxnormal = n[0]; yynormal = n[1]; zznormal = n[2];	// overcome alignment problem
	}

	if (currentsought)
	{
	  	float &xxcurrent = wna.current->x, &yycurrent = wna.current->y, &zzcurrent = wna.current->z;

		xxcurrent = - WAVE_ShiftSpeedX;
		if (!ygiven)
		{
			dyy0 = SPLINE_EvaluateD(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, z0);
		}
		float dzz_dz = SPLINE_EvaluateD(WAVE_ControlZ,
			WAVE_ControlToWorldCoeffs.a,
			WAVE_ControlToWorldCoeffs.b,
			WAVE_ControlToWorldCoeffs.c,
			WAVE_ControlToWorldCoeffs.d,
			WAVE_ControlPoints->numcontrol, z0
		);
		float speed = - WAVE_ShiftSpeedZ / dzz_dz;
		yycurrent = speed * dyy0;
		zzcurrent = speed * dzz0;
	}

	if (regionsought)
	{
		WaveRegionEnum &region = *wna.region;

		region = WAVE_FindRegion(hintout);

		// hack to cause wipeouts from collision with wave
 		if (profilex0 > WAVE_GetMarkerProfile(WAVE_MarkerLipMark0) &&
			profilex0 < WAVE_GetMarkerProfile(WAVE_MarkerLipMark6))
		{

			float emitteryy = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol, WAVE_EmitterZ);
			float emitterzz = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol, WAVE_EmitterZ);
			if (fabsf(yyout - emitteryy) < 2 && fabsf(zzout - emitterzz) < 1)
			{
				if (profilex0 > WAVE_GetMarkerProfile(WAVE_MarkerLipCrash))
					region = (WaveRegionEnum) (WAVE_REGIONMAX + 1);
				else
					region = WAVE_REGIONMAX;
			}
		}
	}

#ifdef DEBUG
	if (WaveDebug.DumpMeshError && hintsought && nearmatch)
	{
		WAVE_MeshError(hintout, *position_out);
	}
#endif
STOP_PROF_TIMER(proftimer_wave_collide);

	return success;
}

/*	Find the overall speed of the wave, as opposed to the surface speed at
	some particular point.
*/
void WAVE_GlobalCurrent(vector3d *current)
{
	current->x = - WAVE_ShiftSpeedX;
	current->y = 0;
	current->z = - WAVE_ShiftSpeedZ;
}

/*	Given an object whose position was known last frame, and which is under the
	influence of the current, find its position this frame.

	The position information is maintained in the hint, which gets updated by this function.
	Orignally, the hint should come from a call to WAVE_FindNearest.

	The vhint specifies a velocity in local space, other than the current.  It should come
	from a call to WAVE_GetVHint, or should be NULL.

	The output vector is posnew.  For now, we assume the object is in the area of the wave
	mesh, and it's the user's resposibility to determine when the object leaves this area.

void WAVE_TrackFloaterOld(WaveFloaterArgs &wfa)
{
	const WaveQueryFlags &flags = wfa.flags;
	bool normalsought = (flags & WAVE_NORMALSOUGHT);
	bool regionsought = (flags & WAVE_REGIONSOUGHT);
	bool velocitygiven = (flags & WAVE_VELOCITYGIVEN);
	assert (flags & WAVE_HINTGIVEN);	// required
	assert (flags & WAVE_HINTSOUGHT);	// required
	assert (!(flags & WAVE_YGIVEN));	// not supported
	assert (!(flags & WAVE_NEARMATCH));	// not supported
	assert (!(flags & WAVE_CURRENTSOUGHT));	// not supported
	float &x = wfa.hint->x, &z = wfa.hint->z;
	const float zero = 0;
	const float &vx = velocitygiven ? wfa.vhint.vx : zero;
	const float &vz = velocitygiven ? wfa.vhint.vz : zero;
	u_int &zcell = wfa.hint->zcell;
	float &xx = wfa.position_out->x, &yy = wfa.position_out->y, &zz = wfa.position_out->z;

	float dzz_dz = SPLINE_EvaluateD(WAVE_ControlZ,
		WAVE_ControlToWorldCoeffs.a,
		WAVE_ControlToWorldCoeffs.b,
		WAVE_ControlToWorldCoeffs.c,
		WAVE_ControlToWorldCoeffs.d,
		WAVE_ControlPoints->numcontrol, z, zcell
	);

	// update position according to current (which might be off by a frame)
	x += (vx - WAVE_ShiftSpeedX) * WAVE_GetFrameSec();
	z += (vz - WAVE_ShiftSpeedZ / dzz_dz) * WAVE_GetFrameSec();

	WaveProfileCoeffs wpc;
	WAVE_ComputeProfileCoeffs(x, wpc);

	// update the precomputed z-interval, which usually won't change between calls
	while (z < WAVE_ControlZ[zcell] && zcell >= 0) --zcell;
	while (z > WAVE_ControlZ[zcell+1] && (zcell+1) < (WAVE_ControlPoints->numcontrol-1)) ++zcell;

	xx = x;
	WAVE_Apply(wpc, z, zcell, yy, zz);

	if (normalsought)
	{
		float &nxx = wfa.normal->x, &nyy = wfa.normal->y, &nzz = wfa.normal->z;
		nglVector n;

		WAVE_Normal(wpc, zz, n[0], n[1], n[2]);
		nxx = n[0]; nyy = n[1]; nzz = n[2];	// overcome alignment problem
	}

	if (regionsought)
	{
		const WavePositionHint &hint = *wfa.hint;
		WaveRegionEnum &region = *wfa.region;

		region = WAVE_FindRegion(hint);
	}
}
*/

void WAVE_TrackFloater(WaveFloaterArgs &wfa)
{
	const WaveQueryFlags &flags = wfa.flags;
	bool normalsought = (flags & WAVE_NORMALSOUGHT);
	bool regionsought = (flags & WAVE_REGIONSOUGHT);
	bool velocitygiven = (flags & WAVE_VELOCITYGIVEN);
	assert (flags & WAVE_HINTGIVEN);	// required
	assert (flags & WAVE_HINTSOUGHT);	// required
	assert (!(flags & WAVE_YGIVEN));	// not supported
	assert (!(flags & WAVE_NEARMATCH));	// not supported
	assert (!(flags & WAVE_CURRENTSOUGHT));	// not supported
	assert (!(flags & WAVE_LIMITSGIVEN));	// not supported
	float &x = wfa.hint->x, &z = wfa.hint->z;
	const float zero = 0;
	const float &vx = velocitygiven ? wfa.vhint.vx : zero;
	const float &vz = velocitygiven ? wfa.vhint.vz : zero;
	u_int &zcell = wfa.hint->zcell;
	u_int &xgrid = wfa.hint->xgrid, &zgrid = wfa.hint->zgrid;
	float &xx = wfa.position_out->x, &yy = wfa.position_out->y, &zz = wfa.position_out->z;

	float dzz_dz = SPLINE_EvaluateD(WAVE_ControlZ,
		WAVE_ControlToWorldCoeffs.a,
		WAVE_ControlToWorldCoeffs.b,
		WAVE_ControlToWorldCoeffs.c,
		WAVE_ControlToWorldCoeffs.d,
		WAVE_ControlPoints->numcontrol, z, zcell
	);

#if defined(TARGET_XBOX)
  if(dzz_dz == 0.0f)
  {
    // JIV FIXME div by zero.  So bad.
    dzz_dz = 0.000001f;
  }
#endif /* TARGET_XBOX JIV DEBUG */

	// update position according to current (which might be off by a frame)
	x += (vx - WAVE_ShiftSpeedX) * WAVE_GetFrameSec();
	z += ((vz - WAVE_ShiftSpeedZ) / dzz_dz) * WAVE_GetFrameSec();

	// update the precomputed z-interval, which usually won't change between calls
	while (z < WAVE_ControlZ[zcell] && zcell > 0) --zcell;
	while (z > WAVE_ControlZ[zcell+1] && (zcell+1) < (WAVE_ControlPoints->numcontrol-1)) ++zcell;

	// update the precomputed interval, which usually won't change between calls
	while (x < WAVE_MeshGridX[xgrid] && xgrid > 0) --xgrid;
	while (x > WAVE_MeshGridX[xgrid+1] && (xgrid+1) < (WAVE_MeshStepX-1)) ++xgrid;
	while (z < WAVE_ControlGridZ[zgrid] && zgrid > 0) --zgrid;
	while (z > WAVE_ControlGridZ[zgrid+1] && (zgrid+1) < (WAVE_MeshStepZ-1)) ++zgrid;

	float xfrac = (x - WAVE_MeshGridX[xgrid]) / (WAVE_MeshGridX[xgrid+1] - WAVE_MeshGridX[xgrid]);
	float zfrac = (z - WAVE_ControlGridZ[zgrid]) / (WAVE_ControlGridZ[zgrid+1] - WAVE_ControlGridZ[zgrid]);
	float frac00 = (1 - xfrac) * (1 - zfrac);
	float frac01 = (1 - xfrac) * zfrac;
	float frac10 = xfrac * (1 - zfrac);
	float frac11 = xfrac * zfrac;

	WaveVertInfo *wvi00 = WAVE_VertInfo + xgrid * (WAVE_MeshStepZ + 1) + zgrid;
	WaveVertInfo *wvi10 = wvi00 + WAVE_MeshStepZ + 1;
	WaveVertInfo *wvi01 = wvi00 + 1;
	WaveVertInfo *wvi11 = wvi10 + 1;

	xx = x;
	yy = frac00 * wvi00->y + frac01 * wvi01->y + frac10 * wvi10->y + frac11 * wvi11->y;
//	yy = yy * WAVE_ScaleSpatial;
	zz = frac00 * wvi00->z + frac01 * wvi01->z + frac10 * wvi10->z + frac11 * wvi11->z;

	if (normalsought)
	{
		float &nxx = wfa.normal->x, &nyy = wfa.normal->y, &nzz = wfa.normal->z;
		nglVector n;

		n[0] = frac00 * wvi00->nx + frac01 * wvi01->nx + frac10 * wvi10->nx + frac11 * wvi11->nx;
		n[1] = frac00 * wvi00->ny + frac01 * wvi01->ny + frac10 * wvi10->ny + frac11 * wvi11->ny;
		n[2] = frac00 * wvi00->nz + frac01 * wvi01->nz + frac10 * wvi10->nz + frac11 * wvi11->nz;
		KSNGL_Normalize( (float *) &n );
		nxx = n[0]; nyy = n[1]; nzz = n[2];	// overcome alignment problem
	}

	if (regionsought)
	{
		const WavePositionHint &hint = *wfa.hint;
		WaveRegionEnum &region = *wfa.region;
		const float &profilex0 = hint.xprofile;
		u_int &xcell = wfa.hint->xcell;

		while (profilex0 < WAVE_BaseProfile[xcell] && xcell >= 0) --xcell;
		while (profilex0 > WAVE_BaseProfile[xcell+1] && (xcell+1) < (WAVE_ControlPoints->numprofile-1)) ++xcell;

		region = WAVE_FindRegion(hint);
	}
}

void WAVE_GetVHint(const WavePositionHint *hintfrom, const WavePositionHint *hintto, float seconds,
	WaveVelocityHint *vhint)
{
	const float &z = hintfrom->z;
	const u_int &zcell = hintfrom->zcell;

	// Account for the stretching of the control-to-world transform
	float dzz_dz = SPLINE_EvaluateD(WAVE_ControlZ,
		WAVE_ControlToWorldCoeffs.a,
		WAVE_ControlToWorldCoeffs.b,
		WAVE_ControlToWorldCoeffs.c,
		WAVE_ControlToWorldCoeffs.d,
		WAVE_ControlPoints->numcontrol, z, zcell
	);

	vhint->vx = (hintto->x - hintfrom->x) / seconds;
	vhint->vz = (hintto->z - hintfrom->z) * dzz_dz / seconds;
}

/*	This function should only be used sporadically, not every frame.  To do --- try a spline along the crest.
*/
void WAVE_GetGrindDirection(const WavePositionHint *hint, const vector3d &posfrom, vector3d *unitdir)
{
	const static float epsilon = WAVE_LeftBreaker ? 0.1f : -0.1f;
	const float &xto = posfrom.x + epsilon;
	const float &zto = hint->z;
	const u_int &zcell = hint->zcell;
	vector3d posto;
	nglVector u;

	DECLARE_SCRATCH(WaveProfileCoeffs, wpc);
	WAVE_ComputeProfileCoeffs(xto, wpc);

	posto.x = xto;
	posto.y = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol,
		zto, zcell);
	posto.z = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol,
		zto, zcell);

	u[0] = posto.x - posfrom.x;
	u[1] = posto.y - posfrom.y;
	u[2] = posto.z - posfrom.z;
	KSNGL_Normalize( (float *) u );

	unitdir->x = u[0];
	unitdir->y = u[1];
	unitdir->z = u[2];
}

/*	Snap to wave crest.  Should only be used once per grind.
*/
void WAVE_GetGrindPosition(const WavePositionHint *hintin, WavePositionHint *hintout,
	const vector3d &posin, vector3d *posout)
{
	const float &x0 = posin.x, &profilex0 = hintin->xprofile;
	const SplineCoeffs<WAVE_NUMPROFILEMAX> &wgc = WAVE_GrindCoeffs;

	float grindz = SPLINE_Evaluate(WAVE_GrindGridX, wgc.a, wgc.b, wgc.c, wgc.d, WAVE_NumGrindCoeffs, profilex0);
	WAVE_BuildHint(hintout, x0, profilex0, grindz, hintin->wpc);

	const WaveProfileCoeffs wpc = hintin->wpc;

	posout->x = x0;
	posout->y = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol,
		grindz, hintout->zcell);
	posout->z = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol,
		grindz, hintout->zcell);

#ifdef DEBUG
	if (WaveDebug.DumpMeshError)
	{
		WAVE_MeshError(*hintout, *posout);
	}
#endif
}

/*	This function should only be used sporadically, not every frame.  If we need it every frame,
	we should avoid repeating the calculation of the WaveProfileCoeffs from WAVE_FindNearest.
*/
void WAVE_GetLipPosition(const vector3d &refpos, vector3d *lippos)
{
	const float &refx = refpos.x;
	float &xx = lippos->x, &yy = lippos->y, &zz = lippos->z;

	DECLARE_SCRATCH(WaveProfileCoeffs, wpc);
	WAVE_ComputeProfileCoeffs(refx, wpc);

	xx = refx;
	yy = SPLINE_Evaluate(WAVE_ControlZ, wpc.y.a, wpc.y.b, wpc.y.c, wpc.y.d, WAVE_ControlPoints->numcontrol,
		WAVE_EmitterZ, WAVE_EmitterHint);
//	yy = yy * WAVE_ScaleSpatial;
	zz = SPLINE_Evaluate(WAVE_ControlZ, wpc.z.a, wpc.z.b, wpc.z.c, wpc.z.d, WAVE_ControlPoints->numcontrol,
		WAVE_EmitterZ, WAVE_EmitterHint);
}

void WAVE_GetBreakInfo(WaveBreakInfoStruct *wbi)
{
	wbi->stage = WAVE_PerturbStage;
	wbi->onoff = (WAVE_PerturbStage != WAVE_PerturbStageNone);
	wbi->type = WAVE_BreakNext->type;
	if (wbi->onoff)
	{
		WavePerturbStageEnum stage = (WavePerturbStageEnum)(((int)(WAVE_PerturbStageMax)) - 1);
		wbi->time = WAVE_Perturb->start[stage] + WAVE_Perturb->duration[stage] - WAVE_GetTotalSec();
	}
	else
	{
		assert(WAVE_PerturbStage == WAVE_PerturbStageNone);
		WavePerturbStageEnum stage = (WavePerturbStageEnum) (((int)(WAVE_PerturbStageNone)) + 1);
		wbi->time = WAVE_Perturb->start[stage] - WAVE_GetTotalSec();
	}
	if (WAVE_Perturb->duration[WAVE_PerturbStage] == 0.0f)
		wbi->stageprogress = 1.0f;
	else
		wbi->stageprogress = (WAVE_GetTotalSec() - WAVE_Perturb->start[WAVE_PerturbStage])
			/ WAVE_Perturb->duration[WAVE_PerturbStage];
}

const vector3d *WAVE_GetMarker(WaveMarkerEnum markernum)
{
	assert(WAVE_MarkerInitComplete);
	return &WAVE_Marker[markernum].pt;
}

void WAVE_HintFromMarker(WaveMarkerEnum markernum, WavePositionHint *hint)
{
  WAVE_BuildHint(hint, WAVE_Marker[markernum].x, WAVE_Marker[markernum].z);
}

static float WAVE_GetMarkerProfile(WaveMarkerEnum markernum)
{
	assert(WAVE_MarkerInitComplete);
	return (WAVE_LeftBreaker ?
		WAVE_MeshMinX + WAVE_MeshMaxX - WAVE_Marker[markernum].pt.x :
		WAVE_Marker[markernum].pt.x
	);
}

int WAVE_GetIndex(void)
{
	int retval = WAVE_ScheduleArray[WAVE_ScheduleIndex].wd_type;
	assert(retval >= 0 && retval < WAVE_LAST);
	return retval;
}

int WAVE_GetScheduleIndex(void)
{
	return WAVE_ScheduleIndex;
}

int WAVE_GetNextScheduleIndex(void)
{
	return (WAVE_ScheduleIndex + 1) % WAVE_ScheduleLength;
}

char WAVE_GetScoringType(void)
{
	return WAVE_ScheduleArray[WAVE_ScheduleIndex].id;
}

float WAVE_GetScheduleSec(void)
{
	return WAVE_GetTotalSec() - WAVE_ScheduleTimeStart;
}

float WAVE_GetScheduleRemainingSec(void)
{
	return WAVE_ScheduleTimeEnd - WAVE_GetTotalSec();
}

WaveStageEnum WAVE_GetStage()
{
	return WAVE_Stage;
}

float WAVE_GetStageRemainingSec(void)
{
	return WAVE_StageStart[WAVE_Stage] + WAVE_StageDuration[WAVE_Stage] - WAVE_GetTotalSec();
}

// ============================================================================

bool WAVE_CheckCollision (CollideCallStruct & cs, bool hint_valid, bool y_valid, bool near_match, bool limitgiven, WaveMarkerEnum frontlimit, WaveMarkerEnum backlimit)
{
	bool retval;
	if ((cs.position.x < WAVE_MeshMaxX) &&
		(cs.position.x > WAVE_MeshMinX) &&
		(cs.position.z < WAVE_MeshMaxZ) &&
		(cs.position.z > WAVE_MeshMinZ))
	{
		//    WavePositionHint hintout;

#if defined(TARGET_XBOX)
		WaveNearestArgs args(
			(WaveQueryFlags) (near_match ? WAVE_NEARMATCH : 0),
			cs.position,
			&cs.position,
			cs.normal,
			cs.current,
			cs.region,
			*cs.hint,
			cs.hint,
			cs.tolerance );
#else
		WaveNearestArgs args = {
			(WaveQueryFlags) (near_match ? WAVE_NEARMATCH : 0),
				cs.position,
				&cs.position,
				cs.normal,
				cs.current,
				cs.region,
				*cs.hint,
				cs.hint,
				cs.tolerance,
		};

#endif /* TARGET_XBOX JIV DEBUG */

		if (cs.hint)
		{
			args.flags = (WaveQueryFlags)(args.flags | WAVE_HINTSOUGHT);

			if (hint_valid)
			{
				args.flags = (WaveQueryFlags)(args.flags | WAVE_HINTGIVEN);
			}
		}

		if (y_valid)
			args.flags = (WaveQueryFlags)(args.flags | WAVE_YGIVEN);

		if (limitgiven)
		{
			args.flags = (WaveQueryFlags)(args.flags | WAVE_LIMITSGIVEN);
			args.frontlimit = frontlimit;
			args.backlimit = backlimit;
		}

		if (cs.normal)
			args.flags = (WaveQueryFlags)(args.flags | WAVE_NORMALSOUGHT);

		if (cs.current)
			args.flags = (WaveQueryFlags)(args.flags | WAVE_CURRENTSOUGHT);

		if (cs.region)
			args.flags = (WaveQueryFlags)(args.flags | WAVE_REGIONSOUGHT);

		retval = WAVE_FindNearest(args);
	}
	else
	{
		cs.position.y = WATER_Altitude (cs.position.x, cs.position.z);

		if (cs.normal)
			WATER_Normal (cs.position.x, cs.position.z, cs.normal->x, cs.normal->y, cs.normal->z);

		if (cs.current)
			WAVE_GlobalCurrent (cs.current);

		if (cs.region)
			*cs.region = WAVE_REGIONFRONT; // not really but enough for a wipeout

		if (cs.hint)
			wavewarn(("WAVE:\tHint requested for position outside of wave mesh!\n"));

		return false;	// avoid assert for now (dc 07/14/02)
	}

	// Temporary check for bug (dc 03/02/02)
	assert(cs.hint == NULL || (cs.hint->z >= WAVE_CONTROLMINZ && cs.hint->z <= WAVE_CONTROLMAXZ));

	return retval;
}

void WAVE_GetCollisionBox (vector3d & boundsMin, vector3d & boundsMax, float marginx, float marginz)
{
	// keep the player close to the wave
	boundsMin.x = WAVE_MeshMinX + marginx;
	boundsMin.y = -20;
	boundsMin.z = WAVE_MeshMinZ + marginz;

	boundsMax.x = WAVE_MeshMaxX - marginx;
	boundsMax.y = 20;
	boundsMax.z = WAVE_MeshMaxZ - marginz;
}


bool WAVE_IsStatic(void)
{
	return WaveDebug.StaticWave;
}

bool WAVE_GetDraw(void)
{
	return WaveDebug.DrawWaveMesh;
}

void WAVE_SetDraw(bool onoff)
{
	WaveDebug.DrawWaveMesh = onoff;
}

float WAVE_GetHeight(void)
{
	return WAVE_ScheduleType[WAVE_ScheduleArray[WAVE_ScheduleIndex].type].height;
}

float WAVE_GetNextHeight(void)
{
	u_int index = (WAVE_ScheduleIndex + 1) % WAVE_ScheduleLength;
	return WAVE_ScheduleType[WAVE_ScheduleArray[index].type].height;
}

void WAVE_AddHeightFudge(int index, float scale)
{
	assert(index < WAVE_HEIGHT_FUDGEFACTOR_ARRAY_SIZE);
	assert(index >= 0);
	WaveHeightFudgeFactorArray[index] = scale;
}

float WAVE_GetHeightFudgeFactor(int index)
{
	assert(index < WAVE_HEIGHT_FUDGEFACTOR_ARRAY_SIZE);
	assert(index >= 0);
	return WaveHeightFudgeFactorArray[index];
}
