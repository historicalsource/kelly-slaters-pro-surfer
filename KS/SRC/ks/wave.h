#ifndef WAVE_H
#define WAVE_H

#if defined(TARGET_PS2)
#include "ngl_ps2.h"
#elif defined(TARGET_XBOX)
#include "ngl.h"
#elif defined(TARGET_GC)
#include "ngl.h"
#endif /* foo */

#include "replay.h"
#include "underwtr.h"

extern float WAVE_TotalSec;

extern float WAVE_MeshMinX;
extern float WAVE_MeshMaxX;
#define WAVE_MESHWIDTH (WAVE_MeshMaxX - WAVE_MeshMinX)
extern float WAVE_MeshMinZ;
extern float WAVE_MeshMaxZ;
#define WAVE_MESHDEPTH (WAVE_MeshMaxZ - WAVE_MeshMinZ)

extern u_int WAVE_MeshStepX;
extern u_int WAVE_MeshStepZ;
#define WAVE_MESHSTEPMAXX 50
#define WAVE_MESHSTEPMAXZ 30
extern float WAVE_MeshGridX[WAVE_MESHSTEPMAXX+1];
extern float WAVE_MeshGridZ[WAVE_MESHSTEPMAXZ+1];

extern float WAVE_TexMinU;	// tiling limit for a 128 x 128 texture
extern float WAVE_TexMaxU;
extern float WAVE_TexMinV;
extern float WAVE_TexMaxV;

//extern float WAVE_ScaleSpatial;
extern float WAVE_ScaleU;
extern float WAVE_ScaleV;
extern float WAVE_OffsetU;
extern float WAVE_OffsetV;
extern float WAVE_ShiftX;
extern float WAVE_ShiftZ;

extern nglMatrix WAVE_LocalToWorld;
extern nglVector WAVE_LocalScale;

extern float WAVE_TexAnimFrame;

#ifdef PROJECT_KELLYSLATER	// not needed for independent wave drawing app
#include "waveenum.h"

// for the particle code
extern float WAVE_EmitterMinX;
extern float WAVE_EmitterMidX;
extern float WAVE_EmitterMaxX;
struct WaveEmitter {
	WaveEmitterEnum type;			// should we have spray particles on this wave profile, and what type
	float x, y, z;					// location of the emitter
	float crestx, cresty, crestz;	// used for strength measure (currently ~ max height of wave profile)
	float mod;
	float xfactor;
};
extern WaveEmitter WAVE_Emitter[WAVE_MESHSTEPMAXX+1];

// for the sound code
enum SoundEmitterEnum
{
	WAVE_SE_FACE,
	WAVE_SE_SPLASH,
	WAVE_SE_TUBE,
	WAVE_SE_MAX
};
enum SoundEmitterType
{
	WAVE_SE_TYPEREGION,
	WAVE_SE_TYPEEMITTER,
	WAVE_SE_TYPEMAX
};

#undef USEINFO
#define USEINFO(name) WAVE_Stage##name,
enum WaveStageEnum {
#include "wavestage.txt"
	WAVE_StageMax
};

struct SoundLine
{
	vector3d start, stop;
};
#define WAVE_SE_LINE_MAX 2
struct WaveSoundEmitter
{
	// private, internal use
	SoundEmitterType type;
	u_int idmask;
	u_int color;

	float minx, maxx;
	float minz, maxz;
	u_int startx[WAVE_SE_LINE_MAX], stopx[WAVE_SE_LINE_MAX];
//	float z;
	u_int minhintz, maxhintz;

	// the emitters
	SoundLine line;
	SoundLine segment[WAVE_SE_LINE_MAX];
	u_int numsegment;
};
extern WaveSoundEmitter WAVE_SoundEmitter[WAVE_SE_MAX];

extern u_int WAVE_AverageColor;

extern float WAVE_SortRadius;

// declarations for WAVE_FindNearest
enum WaveQueryFlags {
	WAVE_YGIVEN = 0x0001,
	WAVE_NORMALSOUGHT = 0x0002,
	WAVE_HINTGIVEN = 0x0004,
	WAVE_HINTSOUGHT = 0x0008,
	WAVE_NEARMATCH = 0x0010,
	WAVE_CURRENTSOUGHT = 0x0020,
	WAVE_REGIONSOUGHT = 0x0040,
	WAVE_VELOCITYGIVEN = 0x0080,
	WAVE_LIMITSGIVEN = 0x0100,
};

#define WAVE_NUMPROFILEMAX 16	// These can be increased, but at some memory cost
#define WAVE_NUMCONTROLMAX 16
template <int n> struct SplineCoeffs {
	float a[n], b[n], c[n], d[n];
};
typedef struct {
	SplineCoeffs<WAVE_NUMCONTROLMAX> y, z;
} WaveProfileCoeffs;

struct WavePositionHint {
	float x, z;
	float xprofile;
	u_int xcell, zcell;
	u_int xgrid, zgrid;
	WaveProfileCoeffs wpc;
};

struct WaveVelocityHint {
	float vx, vz;
};

extern float WAVE_ToleranceDefaultD, WAVE_ToleranceDefaultZ;
struct WaveTolerance {
	float dthresh;	// Farthest we could possibly be (in world units) from the wave surface
	float zthresh;	// Farthest we could possibly be (in world units) from the position given by the hint

	WaveTolerance(void) {dthresh = WAVE_ToleranceDefaultD; zthresh = WAVE_ToleranceDefaultZ;}
	WaveTolerance(float d, float z) {dthresh = d; zthresh = z;}
};

#define NULLREF(a) ((a &)*(int *)NULL)
struct WaveNearestArgs {
#if defined(TARGET_XBOX)
  WaveNearestArgs( 	WaveQueryFlags nflags,
                    const vector3d &nposition_in,
                    vector3d *nposition_out,
                    vector3d *nnormal,
                    vector3d *ncurrent,
                    WaveRegionEnum *nregion,
                    const WavePositionHint &nhintin,
                    WavePositionHint *nhintout,
                    const WaveTolerance &ntolerance,
					WaveMarkerEnum nfrontlimit = WAVE_MarkerInvalid,
					WaveMarkerEnum nbacklimit = WAVE_MarkerInvalid) :
    flags(nflags),
    position_in(nposition_in),
    position_out(nposition_out),
    normal(nnormal),
    current(ncurrent),
    region(nregion),
    hintin(nhintin),
    hintout(nhintout),
    tolerance(ntolerance),
	frontlimit(nfrontlimit),
	backlimit(nbacklimit)
    {
    }
#endif /* TARGET_XBOX JIV DEBUG */

	WaveQueryFlags flags;
	const vector3d &position_in;
	vector3d *position_out;
	vector3d *normal;
	vector3d *current;
	WaveRegionEnum *region;
	const WavePositionHint &hintin;
	WavePositionHint *hintout;
	const WaveTolerance &tolerance;
	WaveMarkerEnum frontlimit, backlimit;
};

struct WaveFloaterArgs {
#if defined(TARGET_XBOX)
  WaveFloaterArgs( WaveQueryFlags nflags,
                   WavePositionHint *nhint,
                   const WaveVelocityHint &nvhint,
                   vector3d *nposition_out,
                   vector3d *nnormal,
                   WaveRegionEnum *nregion )
    : flags(nflags), hint(nhint), vhint(nvhint), position_out(nposition_out),
    normal(nnormal), region(nregion)
  {
  }
#endif /* TARGET_XBOX JIV DEBUG */
	WaveQueryFlags flags;
	WavePositionHint *hint;
	const WaveVelocityHint &vhint;
	vector3d *position_out;
	vector3d *normal;
	WaveRegionEnum *region;
};

void WAVE_Init(void);
void WAVE_Tick(void);
void WAVE_ReplayTick(void);
void WAVE_Create( const int heroIdx );
void WAVE_ListAdd(void);
void WAVE_Unload(void);
void WAVE_Cleanup( void );
u_int WAVE_CheckBreakType(const stringx &breakstr);
void WAVE_ClearBreakArray(void);
void WAVE_AddBreak(const stringx &breakstr, const stringx &typestr, float time, float prob = 1);
void WAVE_ClearSchedule(void);
void WAVE_AddToSchedule(char id, const stringx &typestr, const stringx &breakstr, const stringx &wdstr,
	float scale, float duration);
void WAVE_ResetSchedule(void);
void WAVE_EndBreak(void);
void WAVE_EndWave(bool advance = true);
void WAVE_OnFlybyStart(void);	// Set the wave to its mature state.  (dc 07/08/02)
bool WAVE_FindNearest(const WaveNearestArgs &wna);
void WAVE_GlobalCurrent(vector3d *current);
void WAVE_TrackFloater(WaveFloaterArgs &wfa);
void WAVE_GetVHint(const WavePositionHint *hintfrom, const WavePositionHint *hintto, float seconds,
	WaveVelocityHint *vhint);
void WAVE_GetGrindDirection(const WavePositionHint *hint, const vector3d &posfrom, vector3d *unitdir);
void WAVE_GetGrindPosition(const WavePositionHint *hintin, WavePositionHint *hintout,
	const vector3d &posin, vector3d *posout);
void WAVE_GetLipPosition(const vector3d &refpos, vector3d *lippos);
const vector3d *WAVE_GetMarker(WaveMarkerEnum markernum);
void WAVE_HintFromMarker(WaveMarkerEnum markernum, WavePositionHint *hint);
void WAVE_AddHeightFudge(int index, float scale);
float WAVE_GetHeightFudgeFactor(int index);
int WAVE_GetIndex(void);	// Which wave # (from the enum in wavedata.h) are we on?
int WAVE_GetScheduleIndex(void);	// Which wave # (from the .beach file) are we on?
int WAVE_GetNextScheduleIndex(void);	// Which wave # (from the .beach file) is next?
char WAVE_GetScoringType(void);	// What's the current wave type A/B/C, for scoring purposes?
float WAVE_GetScheduleSec(void);	// How long since the current wave started?
float WAVE_GetScheduleRemainingSec(void);	// How long until the current wave ends?
WaveStageEnum WAVE_GetStage();	// Currently, WAVE_StageBuilding, WAVE_StageStable, WAVE_StageSubsiding
float WAVE_GetStageRemainingSec(void);	// How long until the current wave stage ends?
//void UNDERWTR_CameraReset( void );
//void UNDERWATER_EntitiesTrackCamera(void);
//void WAVE_CameraSelect(const int playerIdx);

#undef USEINFO
#define USEINFO(name, n, type) WAVE_PerturbType##name,
enum WavePerturbTypeEnum {
#include "wavebreak.txt"
	WAVE_PerturbTypeMax
};
#undef USEINFO
#define USEINFO(item) WAVE_PerturbStage##item,
enum WavePerturbStageEnum {
	WAVE_PerturbStageNone,
#include "wavebreakstage.txt"
	WAVE_PerturbStageMax
};
struct WaveBreakInfoStruct {
	bool onoff;					// Is a break currently happening?
	WavePerturbTypeEnum type;	// What type of break?
	float time;					// How long until current break ends or next break begins?
	WavePerturbStageEnum stage;	// What stage of the break?
	float stageprogress;		// Percentage completion of current stage (from 0 to 1)
};
void WAVE_GetBreakInfo(WaveBreakInfoStruct *wbi);

//bool WAVE_CameraUnderwater( const int playerIdx );
//float WAVE_CameraOverWaterDist( const int playerIdx );



//#define NO_HITS			1
//#define HIT_FLOOR		2
//#define HIT_CEILING		4				// for x.z there is a relevant altitude for ceiling
//#define HIT_ROOF		8				// for x.z there is a relevant altitude for roof

class CollideCallStruct
{
public:
CollideCallStruct (const vector3d& pos, WavePositionHint *h)
    { position = pos; normal = current = NULL; region = NULL; hint = h;}
  CollideCallStruct (const vector3d& pos, vector3d *n, vector3d *c, WaveRegionEnum *r, WavePositionHint *h)
    { position = pos; normal = n; current = c; region = r; hint = h;}

  vector3d position;      // uses XZ for input
  vector3d *normal;       // normal at that point
  vector3d *current;      // water current
  WaveRegionEnum *region; // section of the wave
  WavePositionHint *hint;  // hint
  WaveTolerance tolerance;

private:
//	unsigned char region;					// returns the region and collision bits for the FLOOR
//	int quadIndex;							// index into mesh list
//	int indexX, indexY;
};

bool WAVE_CheckCollision (CollideCallStruct & cs, bool hint_valid, bool y_valid, bool near_match, bool limitgiven = false, WaveMarkerEnum frontlimit = WAVE_MarkerInvalid, WaveMarkerEnum backlimit = WAVE_MarkerInvalid);
void WAVE_GetCollisionBox (vector3d & boundsMin, vector3d & boundsMax, float marginx, float marginy);

extern bool WAVE_IsStatic(void);
//extern void WAVE_CameraChecks(const int playerIdx);

#endif	/* #ifdef PROJECT_KELLYSLATER */

bool WAVE_GetDraw(void);
void WAVE_SetDraw(bool onoff);
float WAVE_GetHeight(void);
float WAVE_GetNextHeight(void);

class KSWaterState
{
  friend class KSReplay;

	//float TexAnimFrame;
  float StageStart[WAVE_StageMax];
  float StageDuration[WAVE_StageMax];
	float ScaleU;
	float ScaleV;
	float ShiftU;
	float ShiftV;
	float ShiftX;
	float ShiftZ;
	float ShiftSpeedU;
	float ShiftSpeedV;
	int   Stage;
	int   PerturbStage;
  float ScheduleTimeStart;
  float ScheduleTimeEnd;
	float TotalSec;

	public:
	KSWaterState();
	//virtual ~KSWaterState() {}
  void WriteToDisk( os_file dataFile );
  void ReadFromDisk( os_file dataFile );
	void Save( void );
	void Restore( void );
	void Reset( void );
	void Pause( void );


};

#endif
