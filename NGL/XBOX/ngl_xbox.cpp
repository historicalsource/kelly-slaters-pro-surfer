/*---------------------------------------------------------------------------------------------------------
  NGL_XBOX - XBOX implementation of the NGL graphics library.

  Florent Gluck (florent@treyarch.com)
---------------------------------------------------------------------------------------------------------*/

// Todo:
// - Use the S.Palmer's math library.
// - Per pixel diffuse + specular lighting: directional, point.
// - Per pixel diffuse + specular directional bump mapping (dot3).
// - Finish the implementation of debug features/flags (scene dump).
// - Implement palettized textures support/rendering.
// - Support all NGLBM_CONST_BLEND/ADDITIVE/SUBTRACTIVE modes.
// - void nglExit().
// - Support all NGLP_ parameters (especially bones).
// - VS: true point light.
// - VS: alpha falloff.
// - VS: point projector light.
// - VS: spot projector light.
//
// Fixes:
// - Cubic enviro map PS: divide by the "w" component to be perspective correct (PROJECT3D).
//
// Optimizations:
// - In meshcvt: use XGCompileDrawIndexedVertices() to create a DrawIndexedVertices PB and save it to disk,
//   then register it at loading time.
// - Use several z-buffer for each potential render target (z-buffer's dimension must match render target's one).
// - Use GPU texture format instead of DDS files (allow to directly register them).
// - Optimize VS multi-piece code.
// - Sort by rendertaget, then by vshader.
//
// Improvements:
// - Function to select the way nodes are sorted (by vshaders or textures - whatever).
// - String rendering (strips).
// - VS registers allocation by the user.
// - Flexible multipass rendering.
// - Implement a real hash function to save memory used by the VS database.

/*---------------------------------------------------------------------------------------------------------
  Includes.
---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stl.h>
#include "ngl_xbox.h"
#include "ngl_xbox_internal.h"
#include "ngl_version.h"

// System font.
#include "ngl_SysFontFDF.inc"
#include "ngl_SysFontTEX.inc"

// Debug meshes.
#if NGL_DEBUG
#include "ngl_DataSphere.inc"
#include "ngl_DataRadius.inc"
#endif

// Set to 1 to create the D3D device as pure: faster, but set it to 0 if you want to use xbvscapture/xbpscapture.
#if NGL_DEBUG && defined(PROJECT_KELLYSLATER)
#define D3DCOMPILE_PUREDEVICE    0
#else
#define D3DCOMPILE_PUREDEVICE    1
#endif

/*---------------------------------------------------------------------------------------------------------
  Global defines/constants (most of them are XBox specific).
---------------------------------------------------------------------------------------------------------*/

// Set to 1 if you want to use spherical environment mapping instead of cubic mapping (lot better !).
#define NGL_USE_SPHERICAL_ENVMAP    0

// Set to 1 to use pushbuffer optimization instead of DrawPrimitiveUP() call for quads rendering.
// WARNING: this feature is available for december or newer SDK !
#define NGL_QUADS_USE_PUSHBUFFER    1

// Set to 1 to use pushbuffer rendering optimizations.
// Note: only DrawIndexedVertices() is "pushbuffered" and only when the #indices is >= NGL_PUSHBUFFER_IDXLIMIT.
#define NGL_USE_PUSHBUFFER          0
#define NGL_PUSHBUFFER_IDXLIMIT     1000

// Flag present to be compatible with Spiderman.
#define NGL_FORCE_CREATE_VB         0

// Set to 1 to have profiling informations (the only way to have profiling infos for release build).
#define NGL_PROFILING               0

// The push-buffer is divided into logical units of size NGL_GLOBAL_PUSHBUFFER_SIZE.
// NGL_GLOBAL_KICKOFF_SIZE indicates the number of units that are passed to the GPU for processing.
// The pushbuffer size has the following restrictions:
// - Must be at least 64 KB.
// - Must be a multiple of KickOffSize.
// - PushBufferSize / KickOffSize must be at least four.
#define NGL_GLOBAL_PUSHBUFFER_SIZE  (1024 * 512)

// A smaller kickoff size will increase the granularity of what is pushed to the GPU and will reduce
// the amount of time that a resource is locked, but will result in more overhead.
#define NGL_GLOBAL_KICKOFF_SIZE     (1024 * 32)

// Scratch mesh constants.
#define NGL_MAX_SCRATCHMESHES       512
#define NGL_SCRATCH_MESH_MAX_VERTS  32768
#define NGL_SCRATCH_MESH_WORK_SIZE  (NGL_SCRATCH_MESH_MAX_VERTS * sizeof(nglVertexGeneric))
#define NGL_SCRATCH_MAX_PRIMS       6000
// Scratch mesh system is using triple buffering VB (about 1.1 MB per VB).
#define NGL_SCRATCH_VB_COUNT        3

// Storage for elements of the render list.
#define NGL_LIST_WORK_SIZE          (1024 * 1024)

// Alpha multiplication coefficient to make shadows more translucent.
#ifdef PROJECT_KELLYSLATER
#define NGL_SHADOW_FADING            0.25f
#else
#define NGL_SHADOW_FADING            0.75f
#endif

// If FINAL config is used, some switches are forced.
#ifdef NGL_FINAL
#undef NGL_PROFILING
#define NGL_PROFILING               0
#endif

// Is this really required ?
#undef ARCH_ENGINE

#pragma warning(disable:4701)  // Disable warning "variable may be used without having been initialized".
#pragma warning(disable:4706)  // Disable warning "assignment within conditional expression".
#pragma warning(disable:4127)  // Disable warning "conditional expression is constant".

/*---------------------------------------------------------------------------------------------------------
  Pixel shaders types / constants / variables.
---------------------------------------------------------------------------------------------------------*/

// Pixel shader microcodes.
#include "PixelShaders/ps_notex.h"

#include "PixelShaders/ps_t.h"
#include "PixelShaders/ps_t_mc.h"

#include "PixelShaders/ps_td.h"
#include "PixelShaders/ps_td_mc.h"
#include "PixelShaders/ps_te.h"
#include "PixelShaders/ps_te_mc.h"
#include "PixelShaders/ps_tl.h"
#include "PixelShaders/ps_tl_mc.h"
#include "PixelShaders/ps_tl2.h"
#include "PixelShaders/ps_tl2_mc.h"
#include "PixelShaders/ps_el_win.h"  // Special pixel shader used in Spiderman to render framed-windows.

#include "PixelShaders/ps_tde.h"
#include "PixelShaders/ps_tde_mc.h"
#include "PixelShaders/ps_tdl.h"
#include "PixelShaders/ps_tdl_mc.h"
#include "PixelShaders/ps_tel.h"
#include "PixelShaders/ps_tel_mc.h"
#include "PixelShaders/ps_tel2.h"
#include "PixelShaders/ps_tel2_mc.h"
#include "PixelShaders/ps_tel3.h"
#include "PixelShaders/ps_tel3_mc.h"

#include "PixelShaders/ps_tdel.h"
#include "PixelShaders/ps_tdel_mc.h"

#include "PixelShaders/ps_1proj.h"
#include "PixelShaders/ps_2proj.h"
#include "PixelShaders/ps_3proj.h"
#include "PixelShaders/ps_4proj.h"

// Pixel shaders' opcodes.
// WARNING: Order MUST matches the NGL_PS_xxx enum (defined in ngl_xbox_internal.h) !
DWORD *nglPixelShaderOpcode[NGL_PS_MAX] =
{
	// 1st pass
	ngl_ps_notex_opcode,

	ngl_ps_t_opcode,
	ngl_ps_t_mc_opcode,
	ngl_ps_t_opcode,     // = ngl_ps_e
	ngl_ps_t_mc_opcode,  // = ngl_ps_e

	ngl_ps_td_opcode,
	ngl_ps_td_mc_opcode,
	ngl_ps_te_opcode,
	ngl_ps_te_mc_opcode,
	ngl_ps_tl_opcode,
	ngl_ps_tl_mc_opcode,
	ngl_ps_tl2_opcode,
	ngl_ps_tl2_mc_opcode,
	ngl_ps_tl_opcode,      // = ngl_ps_el
	ngl_ps_el_win_opcode,

	ngl_ps_tde_opcode,
	ngl_ps_tde_mc_opcode,
	ngl_ps_tdl_opcode,
	ngl_ps_tdl_mc_opcode,
	ngl_ps_tel_opcode,
	ngl_ps_tel_mc_opcode,
	ngl_ps_tel2_opcode,
	ngl_ps_tel2_mc_opcode,
	ngl_ps_tel3_opcode,
	ngl_ps_tel3_mc_opcode,

	ngl_ps_tdel_opcode,
	ngl_ps_tdel_mc_opcode,

	// 2nd pass
	ngl_ps_1proj_opcode,
	ngl_ps_2proj_opcode,
	ngl_ps_3proj_opcode,
	ngl_ps_4proj_opcode,
};

// Pixel shaders' handles.
DWORD nglPixelShaderHandle[NGL_PS_MAX];

/*---------------------------------------------------------------------------------------------------------
  Vertex shaders types / constants / variables.
---------------------------------------------------------------------------------------------------------*/

// Vertex shader vertex types declaration (must match the different nglVertexXXX types).
// These declarations must match the registers defined above.
const DWORD nglVS_Quads_Decl[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    // screenspace x,y,z,rhw (rhw is expanded to 1.0)
	D3DVSD_REG(1, D3DVSDT_D3DCOLOR),  // ARGB color
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    // map UVs
	D3DVSD_END()
};

const DWORD nglVS_NonSkin_Decl[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),       // x,y,z
	D3DVSD_REG(1, D3DVSDT_D3DCOLOR),     // ARGB color
	D3DVSD_REG(2, D3DVSDT_NORMPACKED3),  // normal (packed)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),       // map UVs
	D3DVSD_REG(4, D3DVSDT_FLOAT2),       // lightmap UVs
	D3DVSD_END()
};

const DWORD nglVS_Skin_Decl[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),       // x,y,z
	D3DVSD_REG(1, D3DVSDT_D3DCOLOR),     // ARGB color
	D3DVSD_REG(2, D3DVSDT_NORMPACKED3),  // normal (packed)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),       // map UVs
	D3DVSD_REG(4, D3DVSDT_FLOAT2),       // lightmap UVs
	D3DVSD_REG(5, D3DVSDT_PBYTE4),       // bone idx
	D3DVSD_REG(6, D3DVSDT_PBYTE4),       // blend weights
	D3DVSD_END(),
};

// Each vertex shader (VS) is compiled (@runtime) using several vertex shader fragments (VSF)
// "glued" together. The final vshaders are stored in a database. When a section is beeing
// rendered, we search the corresponding VS in the DB (according to the vshader key). If it
// is not found, we compile it (at runtime) and store it in the DB.
// There is a way to avoid this runtime compilation process by compiling all the vertex shader
// at any time. See nglReadVShaderFile() and nglWriteVShaderFile() for more infos.

// 32-bits key used to identify a vertex shader.
// bit   0-5: Reserved (6-bits) for texture stages combinations (see NGL_TS enum).
//            These bits are set in the nglProcessMesh() function at loading time.
// bit     6: Noskin/skin
#define NGL_VSFLAG_SKIN               (uint32)0x0000000000000040
// bit     7: Nofog/fog
#define NGL_VSFLAG_FOG                (uint32)0x0000000000000080
// bit     8: Tint/notint
#define NGL_VSFLAG_TINT               (uint32)0x0000000000000100
// bit     9: Stage 0: noscrolluv/scrolluv
#define NGL_VSFLAG_SCROLLUV           (uint32)0x0000000000000200
// bit 10-13: Reserved (4-bits) to indicate the number of directional lights used.
//            These bits are set in the nglSetupLightConfig() routine.
#define NGL_VSFLAG_DIRLIGHT_OFS       (uint32)10
// bit 14-17: Reserved (4-bits) to indicate the number of point lights used.
//            These bits are set in the nglSetupLightConfig() routine.
#define NGL_VSFLAG_PNTLIGHT_OFS       (uint32)14

// bit   xxx: Alpha falloff enabled/disabled
//#define NGL_VSFLAG_ALPHAFALLOFF       (uint32)0x0000000000000400

// Point light not implemented.
#define NGL_VSFLAG_LASTUSED           (uint32)13

// This mask is used to only keep the TS bits of the VS key.
#define NGL_VSFLAG_TS_MASK            (uint32)0x000000000000003F

// Projector light types.
enum
{
	NGL_DIR_PROJECTOR = 1,
	NGL_PNT_PROJECTOR = 2,
	NGL_SPOT_PROJECTOR = 3,
};

struct nglProjectorParamStruct
{
	uint32 Type;
	uint32 BM;
	uint32 BMConst;
	nglTexture *Tex;

	// Projectors values.
	nglVector Local2UV[2];
	nglVector Dir;
	float TexScale;
};

nglProjectorParamStruct nglProjectorParam[NGL_MAX_PROJECTORS];

nglMatrix nglProjVertexTransform;  // Cache matrix for projected light. Avoid to calculate the vertex transform several times.
nglVector nglProjTexScale;

struct nglProjectorRegStruct
{
	// Registers indices.
	int32 Local2UV;
	int32 Dir;
};

int32 nglVSC2_Projector_LocalToScreen;
int32 nglVSC2_Projector_TexScale;
nglProjectorRegStruct nglVSC2_Projector[NGL_MAX_TS];

// Vertex shaders database stuff.
// The vshader database is *never* accessed directly, but always via the hash table pointers.
enum
{
	// Hash table. Contains pointers to the vertex shaders database entries.
	// Its size depends on the number of bits used by the NGLVS_FLAG_xxx flags (NGL_VSFLAG_LASTUSED)
	NGL_VS_HASH_TABLE_SIZE = 1 << (NGL_VSFLAG_LASTUSED + 1),

	// Vertex shader database. Number of potentially vertex shaders that can be created.
	NGL_VS_DB_SIZE = 1024,

	// Average size in bytes of the vertex shader opcode.
	NGL_BYTES_PER_VSHADER = 1024,

	NGL_VS_OPCODE_POOL = NGL_VS_DB_SIZE * NGL_BYTES_PER_VSHADER,
};

struct nglVSDBEntry
{
	DWORD Handle;  // Handle to the vshader.
	uint32 Key;    // VShader key (required by the nglWriteVShaderFile() function).
};

// VShaders hash table and database.
nglVSDBEntry *nglVSHashTable[NGL_VS_HASH_TABLE_SIZE];
nglVSDBEntry nglVSDB[NGL_VS_DB_SIZE];
uint32 nglVSDBPtr = 0;

// VShaders opcodes memory pool.
uint8 nglVSOpcodePool[NGL_VS_OPCODE_POOL];
uint32 nglVSOpcodePoolPtr = 0;

// First register that can be used when rendering using multi-passes.
// Warning: before nglInitShadersConst() this variable is uninitialized !
int32 NGL_VS_FIRST_FREE_REG;

// First register which is free for client assignment.
int32 NGL_VS_FIRST_USER_REG = NGL_VS_LAST_REG;

// Variables used to store the different VS register values required for section rendering.
// Initialized in the nglInitShadersConst() function.
int32 nglVSC_Util;
int32 nglVSC_Util2;
int32 nglVSC_Scale;
int32 nglVSC_Offset;
int32 nglVSC_LocalToScreen;
int32 nglVSC_FogParams;
int32 nglVSC_UVParams;
int32 nglVSC_CameraPos;
int32 nglVSC_LocalToWorld;
int32 nglVSC_Tint;
int32 nglVSC_MaterialColor;
int32 nglVSC_Bones;    // First register to be used for bones.
nglLightStruct nglVSC_Light[NGL_MAX_LIGHTS];

// Directional projector lights vertex shaders.
DWORD nglVSDirProjector[4];
LPXGBUFFER nglVSDirProjectorOpcode[4];
DWORD nglVSDirProjectorSkin;
LPXGBUFFER nglVSDirProjectorSkinOpcode;

// Hardcoded vshaders.
nglShader nglVSQuad;      // Quads vshader.

/*---------------------------------------------------------------------------------------------------------
  Internal structures.
---------------------------------------------------------------------------------------------------------*/
struct nglScratchMesh
{
	// Mesh structure.
	nglMesh Mesh;
	nglMeshSection Section;
	nglMaterial Material;
};

// Lighting structures.
struct nglDirLightInfo
{
	nglVector Dir;
	nglVector Color;
};

struct nglPointLightInfo
{
	// Position in world space.
	nglVector Pos;
	nglVector Color;

	// Ranges at which falloff starts and finishes.
	float Near;
	float Far;
};

struct nglPointProjectorLightInfo
{
	// Position in world space.
	nglVector Pos;

	// Color of the light.
	nglVector Color;

	// Range at which falloff finishes.
	float Range;

	// Blending operation (usually NGLBM_ADDITIVE).
	uint32 BlendMode;
	uint32 BlendModeConstant;

	// Texture to project (usually nglPhongTex).
	nglTexture *Tex;
};

struct nglDirProjectorLightInfo
{
	// These matrices MUST be on a 16-bytes boundary !
	nglMatrix WorldToUV;  // the matrix for going from world to uv space
	nglMatrix UVToWorld;  // the matrix for going from uv to world space

	// Blending operation (usually NGLBM_ADDITIVE).
	uint32 BlendMode;
	uint32 BlendModeConstant;

	// Texture to project (usually nglPhongTex).
	nglTexture *Tex;

	// For scaling in the U,V directions.  Z component is used as a far plane for the frustum.
	nglVector Scale;

	// Calculated parameters.
	nglVector Pos;
	nglVector Xaxis;
	nglVector Yaxis;
	nglVector Zaxis;

	// the frustum object for this light/projector
	nglFrustum Frustum;
};

// Stage flags.
struct nglStageStruct
{
	uint8 BackFace;
	uint8 SphereReject;
	uint8 NonSkin;
	uint8 Skin;
	uint8 Light;
	uint8 Projector;
	uint8 Quads;
	uint8 Scratch;
	uint8 Tint;
	uint8 PassNOTEX;
	uint8 PassT;
	uint8 PassE;
	uint8 PassTL;
	uint8 PassTD;
	uint8 PassTDL;
	uint8 PassTE;
	uint8 PassEL;
	uint8 PassTDE;
	uint8 PassTEL;
	uint8 PassTDEL;
	uint8 Fog;
	uint8 MatAlpha;
	uint8 MatLight;
};

// Debugging flags.
struct nglDebugStruct
{
	// Show performance info (FPS, etc.)
	uint8 ShowPerfInfo;

	// Allow to take a screenshot from the debugger.
	uint8 ScreenShot;

	// Display vertex shader source code (built on-the-fly).
	uint8 ShowVSCode;

	// Enable printf's throughout the system.
	uint8 InfoPrints;
	uint8 DumpFrameLog;
	uint8 DumpSceneFile;  // Not implemented yet.

	// Debugging geometry.
	uint8 DrawLightSpheres;
	uint8 DrawMeshSpheres;

	// Change the drawing method.
	uint8 DrawAsPoints;
	uint8 DrawAsLines;

	// If this flag is set, the scene is rendered step-by-step for each node
	// thus we can see the render of each node separatly.
	uint8 DispMat;
};

struct nglGPUCounterStruct
{
	HANDLE Handle;
	uint32 Type;
	DM_COUNTDATA Data;
	float Value;
};

enum
{
	NGL_TIMING_BUFDEPTH = 8,
};

struct nglPerfInfoStruct
{
	// GPU counters (see nglGPUCounterStruct).
	// WARNING: CPU performance is affected when this flag is set !
	bool EnableGPUCounters;     // Set it to tell NGL to enable GPU counters for profiling infos.
	bool IsGPUCountersEnabled;  // State variable: indicates if the GPU counters are enable/disable.

	nglGPUCounterStruct GPU;
	nglGPUCounterStruct GPU_BackEnd;
	nglGPUCounterStruct GPU_FrontEnd;
	nglGPUCounterStruct CPU_DPC;
	nglGPUCounterStruct CPU_Total;
	nglGPUCounterStruct FPS;

	// Cycle counters.
	LARGE_INTEGER RenderStart;
	LARGE_INTEGER RenderFinish;
	LARGE_INTEGER ListSendStart;
	LARGE_INTEGER ListSendFinish;

	// Millisecond counters.
	float RenderMSTab[NGL_TIMING_BUFDEPTH];
	float RenderMS;  // Complete (from nglListInit()) render time in millisec.

	float ListSendMSTab[NGL_TIMING_BUFDEPTH];
	float ListSendMS;  // nglListSend() render time in millisec.

	// Vertices and polygons counter. Count the number of verts/polys for the current frame.
	// Warning: quads are not taking in account, only standard/skin and scratch meshes !
	uint32 TotalVerts;
	uint32 TotalPolys;
};

/*---------------------------------------------------------------------------------------------------------
  Global variables.
---------------------------------------------------------------------------------------------------------*/

// System font.
nglFont *nglSysFont;

// Global meshes/textures path.
char nglMeshPath[NGL_MAX_PATH_LENGTH];
char nglTexturePath[NGL_MAX_PATH_LENGTH];

// Scratch mesh globals.
struct nglPrimitiveInfo
{
	uint32 VertIndex;
	uint32 VertCount;
};

__declspec(align(16))
IDirect3DVertexBuffer8 *nglScratchVertBufArray[NGL_SCRATCH_VB_COUNT];  // Vertex buffer used by temporary scratch meshes.
IDirect3DVertexBuffer8 *nglScratchVertBuf;                             // Current vertex buffer used by scratch meshes.
uint32 nglScratchTotalVertCount = 0;                                   // #vertices in the scratch mesh VB.
nglPrimitiveInfo nglScratchPrimInfo[NGL_SCRATCH_MAX_PRIMS];            // Stores the primitive info.
uint32 nglScratchPrimInfoPtr = 0;

// Current in-progress scratch mesh.
bool nglScratchIsEdited = false;
nglMesh *nglScratch = 0;
nglMeshSection *nglScratchSection = 0;
uint32 nglScratchVertCount = 0;
uint8 *nglScratchVertBufPtr = 0;
uint8 *nglScratchTempVertBufPtr = 0;

// Skip list based instancing system used for meshes, textures and fonts.
nglInstanceBank nglMeshBank;
nglInstanceBank nglMeshFileBank;
nglInstanceBank nglTextureBank;
nglInstanceBank nglFontBank;

// Callbacks.
nglSystemCallbackStruct nglSystemCallbacks = { 0 };

// Debugging/Profiling.
#if (NGL_DEBUG || NGL_PROFILING)
nglPerfInfoStruct nglPerfInfo;
uint32 nglNodesCount;
uint32 nglTimingIdx = 0;
#endif

#if NGL_DEBUG
nglMesh *nglSphereMesh;
nglMesh *nglRadiusMesh;
nglMesh *nglSphereDataAligned;
nglMesh *nglRadiusDataAligned;
nglDebugStruct nglDebug = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
nglStageStruct nglStage = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
uint32 nglSectionNumber;
#endif  // NGL_DEBUG

// Timings.
LARGE_INTEGER nglTicksPerSec;

// Debugging flag to optionally turn off and on material flags.
uint32 nglMaterialFlagsAnd = 0xFFFFFFFF;
uint32 nglMaterialFlagsOr = 0x00000000;

// Lighting.
nglLightContext *nglDefaultLightContext = 0;
nglLightContext *nglCurLightContext = 0;

// Scene rendering.
LPDIRECT3DSURFACE8 nglFakeSurface;
LPDIRECT3DSURFACE8 nglDepthBufferSurface;
nglTexture nglFrontBufferTex;
nglTexture nglBackBufferTex;
nglScene nglDefaultScene;  // Calls outside nglListInit/nglListSend affect the global scene.
nglScene *nglCurScene = &nglDefaultScene;  // Currently active scene.
uint8 *nglListWorkPos;
__declspec(align(16))
uint8 nglListWork[NGL_LIST_WORK_SIZE];

// Frame counter.
uint32 nglFrame = 0;            // Frame counter for animated textures and scrolled textures.
int32 nglTextureAnimFrame = 0;  // Current animation frame for animated textures.  This global is sometimes used as a parameter.
float nglIFLSpeed = 30.0f;

// XBMESH versions support.
enum
{
	NGL_SUPPORTED_XBMESH_VERSIONS = 1,
};
uint32 nglSupportedXBMESH[NGL_SUPPORTED_XBMESH_VERSIONS] =
	{ NGL_XBMESH_VERSION };

// Global NGL present parameters.
D3DPRESENT_PARAMETERS nglPresentParams;

// Cache variables on a per-section basis (initially set to invalid values).
int32 nglPrevBM = NGLBM_INVALID;
int32 nglPrevBackFaceCull = -1;
uint32 nglPrevVS = 0;
uint32 nglPrevPS = 0;

// Cache variables on a per-scene basis.
uint32 nglPrevFBWriteMask = 0;

float nglAspectRatio;

/*---------------------------------------------------------------------------------------------------------
  Internal function prototypes.

---------------------------------------------------------------------------------------------------------*/
void nglInitShaders();
void nglInitShadersConst();
DWORD nglGetVertexShaderFromDB(uint32 key);
bool nglLoadTextureIFL(nglTexture *Tex, uint8 *Data, uint32 Size);
bool nglLoadTextureTGA(nglTexture *Tex, uint8 *Data, uint32 Size);
bool nglLoadTextureDDS(nglTexture *Tex, uint8 *Data, bool ForceCubeMap);
bool nglLoadTextureXPR(nglTexture *Tex, uint8 *Data);
void nglLockScratchMeshVertexBuffer();
inline bool nglSphereInFrustum(const nglVector &Center, float Radius);
inline nglListNode *nglGetNextNode(nglListNode *Node);
void nglSetupLightConfig(nglMeshSectionNode *Node);
void nglRenderSingleQuad(void *Data);
void nglRenderSection(void *Data);

/*---------------------------------------------------------------------------------------------------------
  Version API.

  Functions that return the version number and the version build date.

---------------------------------------------------------------------------------------------------------*/

char *nglGetVersion()
{
	return (char *)NGL_VERSION;
}

char *nglGetVersionDate()
{
	return (char *)NGL_VERSION_DATE;
}

/*---------------------------------------------------------------------------------------------------------
  Vector/Matrix/math API.



---------------------------------------------------------------------------------------------------------*/

nglMatrix nglMatrixID(
	nglVector(1.0f, 0.0f, 0.0f, 0.0f),
	nglVector(0.0f, 1.0f, 0.0f, 0.0f),
	nglVector(0.0f, 0.0f, 1.0f, 0.0f),
	nglVector(0.0f, 0.0f, 0.0f, 1.0f)
);

nglVector nglVectorOne(1.0f, 1.0f, 1.0f, 1.0f);

/*-----------------------------------------------------------------------------
Description: Rotate a matrix using the specified x,y,z angles.
-----------------------------------------------------------------------------*/
void nglMatrixRot(nglMatrix &Out, const nglMatrix &In, const nglVector &rot)
{
	XGMATRIX RotX, RotY, RotZ, tmp;
	XGMatrixRotationX(&RotX, rot[0]);
	XGMatrixRotationY(&RotY, rot[1]);
	XGMatrixRotationZ(&RotZ, rot[2]);
	XGMatrixMultiply(&tmp, (XGMATRIX *)&In, &RotX);
	XGMatrixMultiply(&tmp, &tmp, &RotY);
	XGMatrixMultiply((XGMATRIX *)&Out, &tmp, &RotZ);
}

/*-----------------------------------------------------------------------------
Description: Translate a matrix using the specified x,y,z positions.
-----------------------------------------------------------------------------*/
void nglMatrixTrans(nglMatrix &Out, const nglMatrix &In, const nglVector &trans)
{
	XGMATRIX Trans;
	XGMatrixTranslation(&Trans, trans[0], trans[1], trans[2]);
	XGMatrixMultiply((XGMATRIX *)&Out, (XGMATRIX *)&In, &Trans);
}

/*-----------------------------------------------------------------------------
Description: Scale a vector.
-----------------------------------------------------------------------------*/
inline void nglVectorScale3(nglVector &Out, const nglVector &In, float coeff)
{
	XGVec3Scale((XGVECTOR3 *)&Out, (XGVECTOR3 *)&In, coeff);
}

/*-----------------------------------------------------------------------------
Description: Scale a vector.
-----------------------------------------------------------------------------*/
inline void nglVectorScale4(nglVector &Out, const nglVector &In, float coeff)
{
	XGVec4Scale((XGVECTOR4 *)&Out, (XGVECTOR4 *)&In, coeff);
}

/*-----------------------------------------------------------------------------
Description: Subtract 2 vectors.
-----------------------------------------------------------------------------*/
inline void nglVectorSubtract3(nglVector &dest, const nglVector &v1, const nglVector &v2)
{
	XGVec3Subtract((XGVECTOR3 *)&dest, (XGVECTOR3 *)&v1, (XGVECTOR3 *)&v2);
}

/*-----------------------------------------------------------------------------
Description: Subtract 2 vectors.
-----------------------------------------------------------------------------*/
inline void nglVectorSubtract4(nglVector &dest, const nglVector &v1, const nglVector &v2)
{
	XGVec4Subtract((XGVECTOR4 *)&dest, (XGVECTOR4 *)&v1, (XGVECTOR4 *)&v2);
}

/*-----------------------------------------------------------------------------
Description: v1 dot v2.
-----------------------------------------------------------------------------*/
inline float nglVectorDot3(const nglVector &v1, const nglVector &v2)
{
	return XGVec3Dot((XGVECTOR3 *)&v1, (XGVECTOR3 *)&v2);
}

/*-----------------------------------------------------------------------------
Description: v1 dot v2.
-----------------------------------------------------------------------------*/
inline float nglVectorDot4(const nglVector &v1, const nglVector &v2)
{
	return XGVec4Dot((XGVECTOR4 *)&v1, (XGVECTOR4 *)&v2);
}

/*-----------------------------------------------------------------------------
Description: Cross product in 3 dimensions.
-----------------------------------------------------------------------------*/
inline void nglVectorCross3(nglVector &dest, const nglVector &v1, const nglVector &v2)
{
	XGVec3Cross((XGVECTOR3 *)&dest, (XGVECTOR3 *)&v1, (XGVECTOR3 *)&v2);
}

/*-----------------------------------------------------------------------------
Description: Normalize a vector to [0.0, 1.0].
-----------------------------------------------------------------------------*/
inline void nglVectorNormalize3(nglVector &v)
{
	XGVec3Normalize((XGVECTOR3 *)&v, (XGVECTOR3 *)&v);
}

/*-----------------------------------------------------------------------------
Description: Normalize a vector to [0.0, 1.0].
-----------------------------------------------------------------------------*/
inline void nglVectorNormalize4(nglVector &v)
{
	XGVec4Normalize((XGVECTOR4 *)&v, (XGVECTOR4 *)&v);
}

/*-----------------------------------------------------------------------------
Description: Set the camera matrix.
-----------------------------------------------------------------------------*/
void nglSetCameraMatrixXB(nglMatrix &WorldToView, const nglVector &Pos, const nglVector &YDir, const nglVector &ZDir)
{
	XGVECTOR3 pos(Pos[0], Pos[1], Pos[2]);
	XGVECTOR3 ydir(YDir[0], YDir[1], YDir[2]);
	XGVECTOR3 zdir(ZDir[0], ZDir[1], ZDir[2]);

	XGMatrixLookAtLH((XGMATRIX *)&WorldToView, &pos, &ydir, &zdir);
}

/*-----------------------------------------------------------------------------
Description: Apply a matrix to a plane's normal vector.
-----------------------------------------------------------------------------*/
inline void nglApplyMatrixPlane(nglVector &Out, nglMatrix &Mat, nglVector &Plane)
{
	nglVector Pt;
	nglVector Norm;

	// Get plane normal and a reference point.
	nglVectorCopy(&Norm, &Plane);
	Norm[3] = 0.0f;
	Pt[0] = Norm[0] * Plane[3];
	Pt[1] = Norm[1] * Plane[3];
	Pt[2] = Norm[2] * Plane[3];
	Pt[3] = 1.0f;

	// Transform point.
	nglApplyMatrix(Pt, Mat, Pt);

	// Reorient normal.
	nglApplyMatrix(Norm, Mat, Norm);

	// Reconstruct plane.
	nglVectorCopy(&Out, &Norm);
	Out[3] = Pt[0] * Norm[0] + Pt[1] * Norm[1] + Pt[2] * Norm[2] + Pt[3] * Norm[3];
}

/*-----------------------------------------------------------------------------
Description: Return true if 2 spheres intersect.
-----------------------------------------------------------------------------*/
inline int32 nglSpheresIntersect(nglVector &Center1, float Radius1, nglVector &Center2, float Radius2)
{
	nglVector V;
	float DistSqr, Range;

	nglVectorSubtract4(V, Center1, Center2);
	DistSqr = V[0] * V[0] + V[1] * V[1] + V[2] * V[2];

	Range = Radius1 + Radius2;
	return DistSqr <= Range * Range;
}

/*---------------------------------------------------------------------------------------------------------
  Callbacks API.

  NULL can passed in any of the pointers, which will cause the NGL default implementation to be used.

---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Setup a system callback (read file, alloc/dealloc memory, etc.).
-----------------------------------------------------------------------------*/
void nglSetSystemCallbacks(nglSystemCallbackStruct *Callbacks)
{
	nglSystemCallbacks = *Callbacks;
	nglInstanceBank::SetAllocFunc(nglSystemCallbacks.MemAlloc);
	nglInstanceBank::SetFreeFunc(nglSystemCallbacks.MemFree);
}

/*---------------------------------------------------------------------------------------------------------
  Miscellaneous routines API.

  This API implements memory status/allocation and I/O routines.

---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: This functions prints formatted text to the debugger TTY window,
             using the system callbacks if they are specified (if not it uses
             the defaults).
             A callback could be specified to use a user specified function.
-----------------------------------------------------------------------------*/
#ifdef NGL_FINAL

void nglPrintf(const char *Format, ...)  {}
void nglWarning(const char *Format, ...) {}
void nglError(const char *Format, ...)   {}
void nglInfo(const char *Format, ...)    {}
void nglLog(const char *Format, ...)     {}

#else

#define NGL_PRINTF_CORE(x) \
    char buf[4096];\
    va_list args;\
    va_start(args, Format);\
    vsprintf((char*)buf, Format, args);\
    va_end(args);\
    if (nglSystemCallbacks.DebugPrint)\
    {\
      nglSystemCallbacks.DebugPrint(buf);\
    }\
    else\
    {\
      x\
      OutputDebugString(buf);\
    }

void nglPrintf(const char *Format, ...)  { NGL_PRINTF_CORE(;) }
void nglWarning(const char *Format, ...) { NGL_PRINTF_CORE(OutputDebugString("NGL WARN:  ");) }
void nglError(const char *Format, ...)   { NGL_PRINTF_CORE(OutputDebugString("NGL ERROR: ");) }
void nglInfo(const char *Format, ...)    { NGL_PRINTF_CORE(OutputDebugString("NGL INFO:  ");) }
void nglLog(const char *Format, ...)     { NGL_PRINTF_CORE(OutputDebugString("NGL LOG:   ");) }

#endif

/*-----------------------------------------------------------------------------
Description: Print an error message and stop the execution.
             A callback could be specified to use a user specified function.
-----------------------------------------------------------------------------*/
#ifdef NGL_FINAL
void nglFatal(const char *Format, ...)
{
	char buf[4096];
	va_list args;
	va_start(args, Format);
	vsprintf(buf, Format, args);
	va_end(args);

	if (nglSystemCallbacks.CriticalError)
		nglSystemCallbacks.CriticalError(buf);
	else
		Sleep(INFINITE);
}
#else
void nglFatal(const char *Format, ...)
{
	char buf[4096];
	va_list args;
	va_start(args, Format);
	vsprintf(buf, Format, args);
	va_end(args);

	if (nglSystemCallbacks.CriticalError)
	{
		nglSystemCallbacks.CriticalError(buf);
	}
	else
	{
		nglPrintf("NGL FATAL: ");
		nglPrintf(buf);
		Sleep(INFINITE);
	}
}
#endif

/*-----------------------------------------------------------------------------
Description: Return the amount of free physical memory in KB.
-----------------------------------------------------------------------------*/
uint32 nglGetFreePhysicalMemoryXB()
{
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);
	return (stat.dwAvailPhys >> 10);
}

/*-----------------------------------------------------------------------------
Description: Return true if the specified file exists.
-----------------------------------------------------------------------------*/
bool nglFileExists(const char *FileName)
{
	HANDLE f = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
						  OPEN_EXISTING,
						  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
						  NULL);

	if (f == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	CloseHandle(f);

	return true;
}

/*-----------------------------------------------------------------------------
Description: Read a file into memory. An alignement could be specified
             (by default, alignement = 0).
             A callback could be specified to use a user specified function.
-----------------------------------------------------------------------------*/
bool nglReadFile(char *FileName, nglFileBuf *File, uint32 Align)
{
	if (nglSystemCallbacks.ReadFile)
		return nglSystemCallbacks.ReadFile(FileName, File, Align);
	else
	{
		// Open file for read.
		HANDLE f = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL |
                              FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		if (f == INVALID_HANDLE_VALUE)
			return false;

		// Get file size.
		File->Size = GetFileSize(f, NULL);

		// Read file.
		File->Buf = (uint8 *) nglMemAlloc(File->Size, Align);
		uint32 bytesRead;

		// Read check.
		if (!ReadFile(f, File->Buf, File->Size, (DWORD *) &bytesRead, NULL))
		{
			nglError("Can't read \"%s\" !\n", FileName);
			nglMemFree(File->Buf);
			return false;
		}

		if (bytesRead != File->Size)
		{
			nglError("Can't read \"%s\" !\n", FileName);
			nglMemFree(File->Buf);
			return false;
		}

		CloseHandle(f);

		return true;
	}
}

/*-----------------------------------------------------------------------------
Description: Release the memory previously allocated by nglReadFile().
             A callback could be specified to use a user specified function.
-----------------------------------------------------------------------------*/
void nglReleaseFile(nglFileBuf *File)
{
	if (nglSystemCallbacks.ReleaseFile)
		nglSystemCallbacks.ReleaseFile(File);
	else
		nglMemFree(File->Buf);
}

/*-----------------------------------------------------------------------------
Description: Print memory information (free/used phys. mem, virtual, etc.).
-----------------------------------------------------------------------------*/
void nglDisplayMemoryStatusXB()
{
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);
	const uint32 MB = (1024 * 1024);
	nglInfo("%4d total MB of virtual memory.\n", stat.dwTotalVirtual / MB);
	nglInfo("%4d  free MB of virtual memory.\n", stat.dwAvailVirtual / MB);
	nglInfo("%4d total MB of physical memory.\n", stat.dwTotalPhys / MB);
	nglInfo("%4d  free MB of physical memory.\n", stat.dwAvailPhys / MB);
	nglInfo("%4d total MB of paging file.\n", stat.dwTotalPageFile / MB);
	nglInfo("%4d  free MB of paging file.\n", stat.dwAvailPageFile / MB);
	nglInfo("%4d  percent of memory is in use.\n", stat.dwMemoryLoad);
}

/*-----------------------------------------------------------------------------
Description: Allocate a chunk of memory.
             This function must only be used by the NGL library.
             If an alignment is specified, the memory is allocated using the
             XPhysicalAlloc() function, which implies:
             * Memory is 4KB aligned;
             * Memory has a 4KB page granularity.
             * Memory is guaranteed to be contiguous.
             These 3 points are required when allocating memory for a vertex
             buffer (and textures as well, except the alignment).
             If no alignment is specified (or when Align = 0), the memory is
             allocated using malloc(), so it is not aligned or contiguous
             (usefull to allocate structures or memory beeing used by the cpu).
-----------------------------------------------------------------------------*/
void *nglMemAllocInternal(uint32 Size, uint32 Align = 0)
{
#ifdef PROJECT_KELLYSLATER
	if (nglSystemCallbacks.MemAlloc)
	{
		return nglSystemCallbacks.MemAlloc(Size, Align);
	}
	else
#endif
	{
		if (Align)
		{
			void *MemPtr = (void *) (XPhysicalAlloc(Size, MAXULONG_PTR, Align, PAGE_READWRITE));
			NGL_ASSERT(MemPtr != NULL, "Cannot allocate physical memory: XPhysicalAlloc() failed !");
			return MemPtr;
		}
		else
		{
			void *MemPtr = (void *) malloc(Size);
			NGL_ASSERT(MemPtr != NULL, "Cannot allocate memory: memalloc() failed !");
			return MemPtr;
		}
	}
}

/*-----------------------------------------------------------------------------
Description: Release a chunk of memory.
             This function must only be used by the NGL library.
-----------------------------------------------------------------------------*/
void nglMemFreeInternal(void *Ptr, uint32 Align = 0)
{
#ifdef PROJECT_KELLYSLATER
	if (nglSystemCallbacks.MemFree)
	{
		nglSystemCallbacks.MemFree(Ptr);
	}
	else
#endif
	{
		if (Align)
		{
			XPhysicalFree(Ptr);
			Ptr = 0;
		}
		else
		{
			free(Ptr);
			Ptr = 0;
		}
	}
}

/*-----------------------------------------------------------------------------
Description: Allocate a chunk of memory.
             The memory is allocated using the XPhysicalAlloc() function, which
             implies the following:
             * Memory is 4KB aligned;
             * Memory has a 4KB page granularity.
             * Memory is guaranteed to be contiguous.
             These 3 points are required when allocating memory for a vertex
             buffer (and textures as well, except the alignment).
			 If WriteCombined is true, the memory is allocated as write-combined
			 memory (required for pushbuffer). Normally you shouldn't specify
			 this flag.
             A callback could be specified to use a user specified function.
-----------------------------------------------------------------------------*/
void *nglMemAlloc(uint32 Size, uint32 Align, bool WriteCombined)
{
	if (nglSystemCallbacks.MemAlloc)
		return nglSystemCallbacks.MemAlloc(Size, Align);
	else
	{
		uint32 Flags = PAGE_READWRITE;
		if (WriteCombined)
			Flags |= PAGE_WRITECOMBINE;
		void *MemPtr = (void *) (XPhysicalAlloc(Size, MAXULONG_PTR, Align, Flags));
		NGL_ASSERT(MemPtr != NULL, "Cannot allocate physical memory: XPhysicalAlloc() failed !");
		return MemPtr;
	}
}

/*-----------------------------------------------------------------------------
Description: Release a chunk of memory.
             A callback could be specified to use a user specified function.
-----------------------------------------------------------------------------*/
void nglMemFree(void *Ptr)
{
	if (nglSystemCallbacks.MemFree)
		nglSystemCallbacks.MemFree(Ptr);
	else
	{
		XPhysicalFree(Ptr);
		Ptr = 0;
	}
}

/*-----------------------------------------------------------------------------
Description: Take a screenshot of the current back buffer.
-----------------------------------------------------------------------------*/
void nglScreenShot(const char *FileName)
{
  static uint32 ScreenCount = 0;
  if (FileName) 
  {
	  nglSaveTexture(&nglBackBufferTex, FileName);
  }
  else 
  {
	  static char Buf[64];
	  sprintf(Buf, "screenshot%4.4d", ScreenCount++);
	  nglSaveTexture(&nglBackBufferTex, Buf);
  }
}

/*---------------------------------------------------------------------------------------------------------
  Debugging API.


  
---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Return a pointer to the specified flag.
-----------------------------------------------------------------------------*/
#if NGL_DEBUG
uint8 *nglGetDebugFlagPtr(const char *Flag)
{
	// Stage flags.
	if (stricmp(Flag, "BackFace") == 0)
		return &nglStage.BackFace;
	else if (stricmp(Flag, "SphereReject") == 0)
		return &nglStage.SphereReject;
	else if (stricmp(Flag, "NonSkin") == 0)
		return &nglStage.NonSkin;
	else if (stricmp(Flag, "Skin") == 0)
		return &nglStage.Skin;
	else if (stricmp(Flag, "Light") == 0)
		return &nglStage.Light;
	else if (stricmp(Flag, "Projector") == 0)
		return &nglStage.Projector;
	else if (stricmp(Flag, "Quads") == 0)
		return &nglStage.Quads;
	else if (stricmp(Flag, "Scratch") == 0)
		return &nglStage.Scratch;
	else if (stricmp(Flag, "Tint") == 0)
		return &nglStage.Tint;
	else if (stricmp(Flag, "PassNOTEX") == 0)
		return &nglStage.PassNOTEX;
	else if (stricmp(Flag, "PassT") == 0)
		return &nglStage.PassT;
	else if (stricmp(Flag, "PassE") == 0)
		return &nglStage.PassE;
	else if (stricmp(Flag, "PassTL") == 0)
		return &nglStage.PassTL;
	else if (stricmp(Flag, "PassTD") == 0)
		return &nglStage.PassTD;
	else if (stricmp(Flag, "PassTDL") == 0)
		return &nglStage.PassTDL;
	else if (stricmp(Flag, "PassTE") == 0)
		return &nglStage.PassTE;
	else if (stricmp(Flag, "PassEL") == 0)
		return &nglStage.PassEL;
	else if (stricmp(Flag, "PassTDE") == 0)
		return &nglStage.PassTDE;
	else if (stricmp(Flag, "PassTEL") == 0)
		return &nglStage.PassTEL;
	else if (stricmp(Flag, "PassTDEL") == 0)
		return &nglStage.PassTDEL;
	else if (stricmp(Flag, "Fog") == 0)
		return &nglStage.Fog;
	else if (stricmp(Flag, "MatAlpha") == 0)
		return &nglStage.MatAlpha;
	else if (stricmp(Flag, "MatLight") == 0)
		return &nglStage.MatLight;
	else
	// Debug flags.
	if (stricmp(Flag, "ShowPerfInfo") == 0)
		return &nglDebug.ShowPerfInfo;
	else if (stricmp(Flag, "ScreenShot") == 0)
		return &nglDebug.ScreenShot;
	else if (stricmp(Flag, "ShowVSCode") == 0)
		return &nglDebug.ShowVSCode;
	else if (stricmp(Flag, "InfoPrints") == 0)
		return &nglDebug.InfoPrints;
	else if (stricmp(Flag, "DumpFrameLog") == 0)
		return &nglDebug.DumpFrameLog;
	else
	/*
	   TODO:
	   if (stricmp(Flag, "DumpSceneFile") == 0)
	   return &nglDebug.DumpSceneFile;
	   else
	 */
	if (stricmp(Flag, "DrawLightSpheres") == 0)
		return &nglDebug.DrawLightSpheres;
	else if (stricmp(Flag, "DrawMeshSpheres") == 0)
		return &nglDebug.DrawMeshSpheres;
	else if (stricmp(Flag, "DrawAsPoints") == 0)
		return &nglDebug.DrawAsPoints;
	else if (stricmp(Flag, "DrawAsLines") == 0)
		return &nglDebug.DrawAsLines;
	else if (stricmp(Flag, "DispMat") == 0)
		return &nglDebug.DispMat;
	else
		return NULL;
}
#endif

/*-----------------------------------------------------------------------------
Description: Enable/disable the specified flag.
-----------------------------------------------------------------------------*/
void nglSetDebugFlag(const char *Flag, bool Set)
{
#if NGL_DEBUG
	uint8 *Ptr = nglGetDebugFlagPtr(Flag);
	if (Ptr)
		*Ptr = Set;
#endif
}

/*-----------------------------------------------------------------------------
Description: Retrieve the state of the specified flag.
-----------------------------------------------------------------------------*/
bool nglGetDebugFlag(const char *Flag)
{
#if NGL_DEBUG
	uint8 *Ptr = nglGetDebugFlagPtr(Flag);
	if (Ptr)
		return (*Ptr) != 0;
	else
#endif
		return false;
}

/*-----------------------------------------------------------------------------
Description: Print the D3D error of the status value passed as argument.
-----------------------------------------------------------------------------*/
bool nglCheckD3DError(HRESULT status, const char *filename, int32 linenum)
{
	if (status == D3D_OK)
		return false;

	char msg[512];

	switch (status)
	{
		case D3DERR_WRONGTEXTUREFORMAT:
			sprintf(msg, "Wrong texture format");
			break;
		case D3DERR_UNSUPPORTEDCOLOROPERATION:
			sprintf(msg, "Unsupported color operation");
			break;
		case D3DERR_UNSUPPORTEDCOLORARG:
			sprintf(msg, "Unsupported color arg");
			break;
		case D3DERR_UNSUPPORTEDALPHAOPERATION:
			sprintf(msg, "Unsupported alpha operation");
			break;
		case D3DERR_UNSUPPORTEDALPHAARG:
			sprintf(msg, "Unsupported alpha arg");
			break;
		case D3DERR_TOOMANYOPERATIONS:
			sprintf(msg, "Too many operations");
			break;
		case D3DERR_CONFLICTINGTEXTUREFILTER:
			sprintf(msg, "Conflicting texture filter");
			break;
		case D3DERR_UNSUPPORTEDFACTORVALUE:
			sprintf(msg, "Unsupported factor value");
			break;
		case D3DERR_CONFLICTINGRENDERSTATE:
			sprintf(msg, "Conflicting render state");
			break;
		case D3DERR_UNSUPPORTEDTEXTUREFILTER:
			sprintf(msg, "Unsupported texture filter");
			break;
		case D3DERR_CONFLICTINGTEXTUREPALETTE:
			sprintf(msg, "Conflicting texture palette");
			break;
		case D3DERR_DRIVERINTERNALERROR:
			sprintf(msg, "Driver internal error");
			break;
		case D3DERR_NOTFOUND:
			sprintf(msg, "Not found");
			break;
		case D3DERR_MOREDATA:
			sprintf(msg, "More data");
			break;
		case D3DERR_DEVICELOST:
			sprintf(msg, "Device lost");
			break;
		case D3DERR_DEVICENOTRESET:
			sprintf(msg, "Device not reset");
			break;
		case D3DERR_NOTAVAILABLE:
			sprintf(msg, "Not available");
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			sprintf(msg, "Out of video memory");
			break;
		case D3DERR_INVALIDDEVICE:
			sprintf(msg, "Invalid device");
			break;
		case D3DERR_INVALIDCALL:
			sprintf(msg, "Invalid call");
			break;
		case E_FAIL:
			sprintf(msg,
					"An undetermined error occurred inside the Direct3D subsystem.");
			break;
		case E_INVALIDARG:
			sprintf(msg,
					"An invalid parameter was passed to the returning function");
			break;
		case E_OUTOFMEMORY:
			sprintf(msg,
					"Direct3D could not allocate sufficient memory to complete the call.");
			break;

		default:
		{
			sprintf(msg, "UNKNOWN ERROR IN D3D: %08x", status);
			break;
		}
	}

	nglFatal("Direct3D failure on line %d of file %s: %s\n", linenum,
			 filename, msg);

	return true;
}

/*---------------------------------------------------------------------------------------------------------
  General rendering API.


  
---------------------------------------------------------------------------------------------------------*/

IDirect3D8 *nglD3D = NULL;
IDirect3DDevice8 *nglDev = NULL;

/*-----------------------------------------------------------------------------
Description: Create and initialize the front/back buffer textures.
-----------------------------------------------------------------------------*/
void nglInitFrameBufferTexture()
{
	memset(&nglFrontBufferTex, 0, sizeof(nglTexture));
	memset(&nglBackBufferTex, 0, sizeof(nglTexture));

	nglFrontBufferTex.Type = NGLTEX_DDS;
	nglFrontBufferTex.Static = 1;
	nglFrontBufferTex.FileName = "NGLFRONTBUF";

	nglBackBufferTex.Type = NGLTEX_DDS;
	nglBackBufferTex.Static = 1;
	nglBackBufferTex.FileName = "NGLBACKBUF";

	// Create the shared depth buffer (zbuffer + stencil) surface.
	// The size is 640x512 thus it means that we can't have a linear texture > 640x512 and
	// a swizzled one > 512x512 (should be sufficient though).
	NGL_D3DTRY(nglDev->CreateDepthStencilSurface(NGL_DEPTHBUFFER_WIDTH,
               NGL_DEPTHBUFFER_HEIGHT, D3DFMT_LIN_D24S8, 0, &nglDepthBufferSurface));

	NGL_D3DTRY(nglDev->CreateImageSurface(NGL_DEPTHBUFFER_WIDTH,
               NGL_DEPTHBUFFER_HEIGHT, D3DFMT_LIN_A8R8G8B8, &nglFakeSurface));

	// Set frontbuffer texture params.
	nglFrontBufferTex.Width = nglGetScreenWidth();
	nglFrontBufferTex.Height = nglGetScreenHeight();
	nglFrontBufferTex.Format = NGLTF_LINEAR;
	nglTextureBank.Insert(nglFrontBufferTex.FileName, &nglFrontBufferTex);

	// Set backbuffer texture params.
	nglBackBufferTex.Width = nglGetScreenWidth();
	nglBackBufferTex.Height = nglGetScreenHeight();
	nglBackBufferTex.Format = NGLTF_LINEAR;
	nglTextureBank.Insert(nglBackBufferTex.FileName, &nglBackBufferTex);

	// Retrieve front/back surfaces (to increment their ref count by 1).
	NGL_D3DTRY(nglDev->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &nglFrontBufferTex.DXSurface));

	NGL_D3DTRY(nglDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &nglBackBufferTex.DXSurface));

	// In this case, DX textures/surfaces are similar, set the texture to point to the surface.
	nglFrontBufferTex.DXTexture.Simple = (IDirect3DTexture8 *) nglFrontBufferTex.DXSurface;
	nglBackBufferTex.DXTexture.Simple = (IDirect3DTexture8 *) nglBackBufferTex.DXSurface;
}

/*-----------------------------------------------------------------------------
Description: Initialize the GPU counters used for profiling infos.
-----------------------------------------------------------------------------*/
#if (NGL_DEBUG || NGL_PROFILING)

uint32 nglType2Query(uint32 type)
{
	uint32 query = type & DMCOUNT_COUNTTYPE;

	if (query == DMCOUNT_EVENT)
		query |= (type & DMCOUNT_COUNTSUBTYPE) | DMCOUNT_PERSEC;

	return query;
}

void nglInitGPUCounters()
{
	if (DmEnableGPUCounter(true) != XBDM_NOERR)
		nglFatal("DmEnableGPUCounter() failed !");

	// Walk through the available GPU counters.
	PDM_WALK_COUNTERS CurCounter = NULL;
	DM_COUNTINFO CounterInfo;

	for (;;)
	{
		int32 Status = DmWalkPerformanceCounters(&CurCounter, &CounterInfo);

		if (Status == XBDM_ENDOFLIST)
			break;

		if (Status != XBDM_NOERR)
			nglFatal("DmWalkPerformanceCounters() failed !");
		//nglInfo("%s\n", CounterInfo.Name);  // Print the counter names.

		// Retrieve handles and types for counters of interest.
		if (stricmp(CounterInfo.Name, "% GPU Backend") == 0)
		{
			DmOpenPerformanceCounter(CounterInfo.Name,
									 &nglPerfInfo.GPU_BackEnd.Handle);
			nglPerfInfo.GPU_BackEnd.Type = nglType2Query(CounterInfo.Type);
		}
		else if (stricmp(CounterInfo.Name, "% CPU:DPC") == 0)
		{
			DmOpenPerformanceCounter(CounterInfo.Name,
									 &nglPerfInfo.CPU_DPC.Handle);
			nglPerfInfo.CPU_DPC.Type = nglType2Query(CounterInfo.Type);
		}
		else if (stricmp(CounterInfo.Name, "% GPU Frontend") == 0)
		{
			DmOpenPerformanceCounter(CounterInfo.Name,
									 &nglPerfInfo.GPU_FrontEnd.Handle);
			nglPerfInfo.GPU_FrontEnd.Type = nglType2Query(CounterInfo.Type);
		}
		else if (stricmp(CounterInfo.Name, "% GPU") == 0)
		{
			DmOpenPerformanceCounter(CounterInfo.Name,
									 &nglPerfInfo.GPU.Handle);
			nglPerfInfo.GPU.Type = nglType2Query(CounterInfo.Type);
		}
		else if (stricmp(CounterInfo.Name, "% CPU:total") == 0)
		{
			DmOpenPerformanceCounter(CounterInfo.Name,
									 &nglPerfInfo.CPU_Total.Handle);
			nglPerfInfo.CPU_Total.Type = nglType2Query(CounterInfo.Type);
		}
		else if (stricmp(CounterInfo.Name, "Frames") == 0)
		{
			DmOpenPerformanceCounter(CounterInfo.Name,
									 &nglPerfInfo.FPS.Handle);
			nglPerfInfo.FPS.Type = nglType2Query(CounterInfo.Type);
		}
	}

	// Close the counters walk.
	if (DmCloseCounters(CurCounter) != XBDM_NOERR)
		nglFatal("DmCloseCounters() failed !");

	// Disable GPU performance counters.
	if (DmEnableGPUCounter(false) != XBDM_NOERR)
		nglFatal("DmEnableGPUCounter() failed !");
}

#endif

/*-----------------------------------------------------------------------------
Description: Reset the list allocator.
-----------------------------------------------------------------------------*/
inline void nglListWorkReset()
{
	nglListWorkPos = nglListWork;
}

/*-----------------------------------------------------------------------------
Description: Binds a vertex buffer to a device data stream.
-----------------------------------------------------------------------------*/
void nglBindVertexBuffer(IDirect3DVertexBuffer8 *VertexBuffer, uint32 VertexSize)
{
	static IDirect3DVertexBuffer8 *PrevVertexBuffer = 0;

	// Cache the last bind.
	if (VertexBuffer == PrevVertexBuffer)
		return;

	NGL_D3DTRY(nglDev->SetStreamSource(0, VertexBuffer, VertexSize));
	PrevVertexBuffer = VertexBuffer;
}

/*-----------------------------------------------------------------------------
Description: NGL initialization routine.
-----------------------------------------------------------------------------*/
void nglInit()
{
	uint32 i;

	nglPrintf("------------------------------------------------------------------------\n");
	nglPrintf("NGL version %s built on %s\n\n", nglGetVersion(), nglGetVersionDate());

#if (NGL_DEBUG || NGL_PROFILING)
	memset(&nglPerfInfo, 0, sizeof(nglPerfInfoStruct));
#endif

	nglMeshPath[0] = 0;
	nglTexturePath[0] = 0;

	nglMeshBank.Init();
	nglMeshFileBank.Init();
	nglTextureBank.Init();
	nglFontBank.Init();

	// Test if we need to switch to "widescreen".
	if (XGetVideoFlags() & XC_VIDEO_FLAGS_WIDESCREEN)
		nglAspectRatio = 16.0f / 9.0f;
	else
		nglAspectRatio = 4.0f / 3.0f;

	// XBox graphic hardware initialization.
	nglD3D = Direct3DCreate8(D3D_SDK_VERSION);
	if (!nglD3D)
		nglFatal("Direct3DCreate8 failed !\n");

	// Setup the main pushbuffer memory sizes.
	if (nglD3D->SetPushBufferSize(NGL_GLOBAL_PUSHBUFFER_SIZE, NGL_GLOBAL_KICKOFF_SIZE) != S_OK)
		nglFatal("SetPushBufferSize failed (check out NGL_GLOBAL_PUSHBUFFER_SIZE and NGL_GLOBAL_KICKOFF_SIZE) !\n");

	ZeroMemory(&nglPresentParams, sizeof(nglPresentParams));

	nglPresentParams.BackBufferWidth = nglGetScreenWidth();
	nglPresentParams.BackBufferHeight = nglGetScreenHeight();
	nglPresentParams.BackBufferFormat = D3DFMT_LIN_A8R8G8B8;
	nglPresentParams.BackBufferCount = 2;

	// Set the FSAA to use.
	nglPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	nglPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	nglPresentParams.hDeviceWindow = NULL;
	nglPresentParams.Windowed = false;
	nglPresentParams.EnableAutoDepthStencil = true;
	nglPresentParams.AutoDepthStencilFormat = NGL_DEPTHBUFFER_FORMAT;
	//nglPresentParams.Flags = D3DPRESENTFLAG_10X11PIXELASPECTRATIO;
	nglPresentParams.Flags = 0;
	nglPresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	nglPresentParams.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;  // Set to 60 FPS by default.

	HRESULT res;
#if D3DCOMPILE_PUREDEVICE
	if (FAILED(res = nglD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
                                          D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                          &nglPresentParams, &nglDev)))
#else
	if (FAILED(res = nglD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
                                          D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                          &nglPresentParams, &nglDev)))
#endif
	{
		NGL_D3DTRY(res);
	}

	NGL_D3DTRY(nglDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0));
	NGL_D3DTRY(nglDev->Present(NULL, NULL, NULL, NULL));

	nglBindVertexBuffer(0, 0);

	nglDev->SetRenderState(D3DRS_LIGHTING, false);
	nglDev->SetRenderState(D3DRS_COLORVERTEX, false);
	nglDev->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);
	// If we want specular lighting, just enable this renderstate
	// and set the specular color to the oD1 VS register.
	// For now, specular color is used to process the NGL_MATERIAL_COLOR flag and shadows (to have 4/pass instead of 2).
	nglDev->SetRenderState(D3DRS_SPECULARENABLE, true);
	nglDev->SetRenderState(D3DRS_NORMALIZENORMALS, false);
	nglDev->SetRenderState(D3DRS_LOCALVIEWER, false);

	nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	nglDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	nglDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	nglDev->SetRenderState(D3DRS_ZWRITEENABLE, true);
	nglDev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	nglDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	nglDev->SetRenderState(D3DRS_DITHERENABLE, true);
	nglDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	nglDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);

	// Fog parameters.
	nglDev->SetRenderState(D3DRS_FOGENABLE, true);
	nglDev->SetRenderState(D3DRS_RANGEFOGENABLE, false);
	// Note: by setting D3DFOG_NONE above, we are telling the hardware we want
	// complete control over the fog value. In other words, the vertex shader
	// will set oFog.x in a range from 0.0 (max fog) to 1.0 (no fog), which will
	// be the final fog value as seen by the pixel shader.
	// We could still set the fog table mode to linear, exp, or exp2, in which
	// case we'd need our vertex shader to set oFog.x in view space coordinates.
	// The hardware will take that value and perform the fog calculations (and
	// table lookup) on it.
	nglDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	// These renderstates are useless because calculations are done by the vertex shader.
	nglDev->SetRenderState(D3DRS_FOGSTART, 0);
	nglDev->SetRenderState(D3DRS_FOGEND, 0);
	nglDev->SetRenderState(D3DRS_FOGDENSITY, 0);  // Only for exponential modes.

	for (i = 0; i < NGL_MAX_TS; i++)
	{
		nglDev->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
		nglDev->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		nglDev->SetTextureStageState(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		nglDev->SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		nglDev->SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	}

	nglInitShaders();  // Vertex/pixel shaders init.

	// Scratch mesh system init: create VBs.
	for (i = 0; i < NGL_SCRATCH_VB_COUNT; i++)
	{
		NGL_D3DTRY(nglDev->CreateVertexBuffer(NGL_SCRATCH_MESH_WORK_SIZE, D3DUSAGE_WRITEONLY, 0, 0,&nglScratchVertBufArray[i]));
	}
	nglLockScratchMeshVertexBuffer();

	// Set up the default scene (fog disabled by default).
	nglInitFrameBufferTexture();
	memset(&nglDefaultScene, 0, sizeof(nglDefaultScene));
	nglCurScene->FogColor = 0xFFFFFFFF;
	nglCurScene->FogNear = 1.0f;
	nglCurScene->FogFar = 1000.0f;
	nglCurScene->FogMin = 0.0f;
	nglCurScene->FogMax = 1.0f;
	nglCurScene->RenderTarget = nglGetBackBufferTex();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	nglSetFBWriteMask(NGL_FBWRITE_RGBA);
	nglSetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	nglSetClearZ(1.0f);
	nglSetClearStencil(0);
	nglSetZWriteEnable(true);
	nglSetZTestEnable(true);
	nglSetViewport(0, 0, nglGetScreenWidth() - 1, nglGetScreenHeight() - 1);
	nglSetPerspectiveMatrix(60.0f, NGL_SCREEN_WIDTH / 2.0f, NGL_SCREEN_HEIGHT / 2.0f, 0.0f, 10000.0f, 0.0f, 1.0f);

	// Get the system font texture from memory (embedded cpp file).
	nglTexture *FontTex = nglLoadTextureInPlace("nglSysFont", NGLTEX_DDS, nglSysFontTEX, sizeof(nglSysFontTEX));
	FontTex->Static = 1;
	// Get the system font FDF from memory (embedded cpp file).
	nglSysFont = nglLoadFontInPlace("nglSysFont", nglSysFontFDF);

	// Get the frequency of the timer (used for profiling).
	QueryPerformanceFrequency(&nglTicksPerSec);

	// Initialize the GPU counters stuff.
#if (NGL_DEBUG || NGL_PROFILING)
	nglInitGPUCounters();
#endif

#if NGL_DEBUG
	// Load the debug meshes from memory.
	// As static arrays can't be aligned to a 4KB boundary, we have to allocate the memory
	// to a 4KB boundary and copy the data there (crap !).
	nglMesh *nglSphereDataAligned = (nglMesh *) nglMemAllocInternal(sizeof(nglSphereMeshData), 4096);
	nglMesh *nglRadiusDataAligned = (nglMesh *) nglMemAllocInternal(sizeof(nglRadiusMeshData), 4096);
	memcpy(nglSphereDataAligned, nglSphereMeshData, sizeof(nglSphereMeshData));
	memcpy(nglRadiusDataAligned, nglRadiusMeshData, sizeof(nglRadiusMeshData));
	nglLoadMeshFileInPlace("nglsphere", nglSphereDataAligned);
	nglLoadMeshFileInPlace("nglradius", nglRadiusDataAligned);
	nglSphereMesh = nglGetMesh("ngl_sphere");
	nglRadiusMesh = nglGetMesh("ngl_radius");
#endif

	// Clear the color/z/stencil buffers and flip (do it for each backbuffer + front buffer - to
	// remove the garbage that could be displayed when calling nglFlip(true) for the first time).
	for (i = 0; i < nglPresentParams.BackBufferCount + 1; i++)
	{
		NGL_D3DTRY(nglDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0));
		NGL_D3DTRY(nglDev->Present(NULL, NULL, NULL, NULL));
	}

	// Required by static scratch meshes.
	nglListWorkReset();
}


/*-----------------------------------------------------------------------------
Description: NGL exit function, release every NGL object from the memory.
-----------------------------------------------------------------------------*/
void nglExit()
{
	// Free the memory used to store the debug meshes.
	// (was required because static array can't be 4KB aligned).
#if NGL_DEBUG
	nglMemFreeInternal(nglSphereDataAligned, 4096);
	nglMemFreeInternal(nglRadiusDataAligned, 4096);
#endif
}

/*-----------------------------------------------------------------------------
Description: Flip the displayed pages.
-----------------------------------------------------------------------------*/
void nglFlip()
{
#if NGL_DEBUG
	// Take a screenshot if the corresponding debug flag is specified.
	if (nglDebug.ScreenShot)
	{
		nglScreenShot();
		nglDebug.ScreenShot = 0;
	}
#endif

	nglFrame++;

	NGL_D3DTRY(nglDev->Present(NULL, NULL, NULL, NULL));
}

/*-----------------------------------------------------------------------------
Description: Lock the frame rate to a given FPS (default is 60 FPS).
NOTES:       On the XBox, only the following parameters are supported:
             * 60 : lock the framerate @ 60 FPS;
             * 30 : lock the framerate @ 30 FPS;
             *  0 : unlimited framerate (usefull for profiling)
-----------------------------------------------------------------------------*/
void nglSetFrameLock(float FPS)
{
	UINT fps;
	switch (int32(FPS))
	{
		case 60:
			fps = D3DPRESENT_INTERVAL_ONE;
			break;
		case 30:
			fps = D3DPRESENT_INTERVAL_TWO;
			break;
		case 0:
			fps = D3DPRESENT_INTERVAL_IMMEDIATE;
			break;
		default:
			fps = D3DPRESENT_INTERVAL_ONE;
	}
	nglPresentParams.FullScreen_PresentationInterval = fps;
	NGL_D3DTRY(nglDev->Reset(&nglPresentParams));
}

/*-----------------------------------------------------------------------------
Description: Sets the speed at which IFLs play, in fps (default is 30fps).
             Values should divide evenly into 60, ie 60, 30, 20, 15, 10, etc.
-----------------------------------------------------------------------------*/
void nglSetIFLSpeed(float FPS)
{
	nglIFLSpeed = FPS;
}

/*-----------------------------------------------------------------------------
Description: Return the screen width.
-----------------------------------------------------------------------------*/
int32 nglGetScreenWidth()
{
	return NGL_SCREEN_WIDTH;
}

/*-----------------------------------------------------------------------------
Description: Return the screen height.
-----------------------------------------------------------------------------*/
int32 nglGetScreenHeight()
{
	return NGL_SCREEN_HEIGHT;
}

#ifdef PROJECT_KELLYSLATER
int nglScreenWidthTV = 580;
int nglScreenHeightTV = 448;
int nglScreenXOffsetTV = (NGL_SCREEN_WIDTH - nglScreenWidthTV) / 2;
int nglScreenYOffsetTV = (NGL_SCREEN_HEIGHT - nglScreenHeightTV) / 2;

/*-----------------------------------------------------------------------------
Description: Return the width of the part of the screen buffer which gets
mapped to the TV screen.
-----------------------------------------------------------------------------*/
int32 nglGetScreenWidthTV()
{
  return nglScreenWidthTV;
}

/*-----------------------------------------------------------------------------
Description: Return the height of the part of the screen buffer which gets
mapped to the TV screen.
-----------------------------------------------------------------------------*/
int32 nglGetScreenHeightTV()
{
  return nglScreenHeightTV;
}

/*-----------------------------------------------------------------------------
Description: Return the width of the part of the screen buffer which gets
mapped to the TV screen.
-----------------------------------------------------------------------------*/
int32 nglGetScreenXOffsetTV()
{
  return nglScreenXOffsetTV;
}

/*-----------------------------------------------------------------------------
Description: Return the height of the part of the screen buffer which gets
mapped to the TV screen.
-----------------------------------------------------------------------------*/
int32 nglGetScreenYOffsetTV()
{
  return nglScreenYOffsetTV;
}
#endif

/*-----------------------------------------------------------------------------
Description: Set the meshes path.
-----------------------------------------------------------------------------*/
void nglSetMeshPath(const char *Path)
{
	if (nglSystemCallbacks.ReadFile)
	{
		strcpy(nglMeshPath, Path);
	}
	else
	{
		strcpy(nglMeshPath, "D:\\");
		strcat(nglMeshPath, Path);
	}
}

/*-----------------------------------------------------------------------------
Description: Return the meshes path.
-----------------------------------------------------------------------------*/
const char *nglGetMeshPath()
{
	if (nglSystemCallbacks.ReadFile)
	{
		return (const char *) nglMeshPath;
	}
	else
	{
		return (const char *) nglMeshPath[3];  // [3] to skip "D:\"
	}
}

/*-----------------------------------------------------------------------------
Description: Set the textures path.
-----------------------------------------------------------------------------*/
void nglSetTexturePath(const char *Path)
{
	if (nglSystemCallbacks.ReadFile)
	{
		strcpy(nglTexturePath, Path);
	}
	else
	{
		strcpy(nglTexturePath, "D:\\");
		strcat(nglTexturePath, Path);
	}
}

/*-----------------------------------------------------------------------------
Description: Return the textures path.
-----------------------------------------------------------------------------*/
const char *nglGetTexturePath()
{
	if (nglSystemCallbacks.ReadFile)
	{
		return (const char *) nglTexturePath[3];
	}
	else
	{
		return (const char *) nglTexturePath[3];  // [3] to skip "D:\"
	}
}

/*---------------------------------------------------------------------------------------------------------
  Render List API

  The Render List is the core of NGL's rendering API.  Use these functions to construct a render list,
  which is passed to the hardware via nglListSend.
---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Fills out a SortInfo structure for use with nglListAddNode.
-----------------------------------------------------------------------------*/
void nglGetSortInfo(nglSortInfo *SortInfo, uint32 BlendMode, DWORD VShaderHandle, float Dist)
{
	if (BlendMode == NGLBM_OPAQUE || BlendMode == NGLBM_PUNCHTHROUGH)
	{
		SortInfo->Type = NGLSORT_OPAQUE;
		// Opaque sections are sorted by vertex shaders.
		SortInfo->Hash = VShaderHandle;
	}
	else
	{
		SortInfo->Type = NGLSORT_TRANSLUCENT;
		SortInfo->Dist = Dist;
	}
}

/*-----------------------------------------------------------------------------
Description: Allocates from the render list storage space (which is completely
             cleared each call to nglListInit).
-----------------------------------------------------------------------------*/
void *nglListAlloc(uint32 Bytes, uint32 Alignment)
{
	if ((nglListWorkPos - nglListWork) % Alignment)
		nglListWorkPos = nglListWork + ((nglListWorkPos - nglListWork) / Alignment + 1) * Alignment;

	if (nglListWorkPos + Bytes > nglListWork + NGL_LIST_WORK_SIZE)
	{
		nglError("Render list allocation overflow. Reserved = %d Requested = %d Free = %d.\n",
                 NGL_LIST_WORK_SIZE, Bytes,
                 NGL_LIST_WORK_SIZE - (nglListWorkPos - nglListWork));
		return NULL;
	}

	void *Ret = nglListWorkPos;
	nglListWorkPos += Bytes;

	return Ret;
}

/*-----------------------------------------------------------------------------
Description: Begin a sub-scene.
-----------------------------------------------------------------------------*/
nglScene *nglListBeginScene()
{
	nglScene *Scene;

	if (nglCurScene)
	{
		// Add the new scene at the end of the current scene's child list.
		Scene = nglListNew(nglScene);

		if (!Scene)
			return (nglScene *) 0;

		if (nglCurScene->LastChild)
			nglCurScene->LastChild->NextSibling = Scene;
		else
			nglCurScene->FirstChild = Scene;

		nglCurScene->LastChild = Scene;

		// Set up the scene based on nglDefaultScene (whose parameters can be set outside the render loop).
		memcpy(Scene, &nglDefaultScene, sizeof(nglScene));
	}
	else
	{
		Scene = &nglDefaultScene;
	}

	// Initialize the new scene's hierarchy linkage.
	Scene->Parent = nglCurScene;
	Scene->NextSibling = NULL;
	Scene->FirstChild = NULL;
	Scene->LastChild = NULL;

	// Initialize the new scene's bin structure.
	Scene->OpaqueRenderList = NULL;
	Scene->TransRenderList = NULL;
	Scene->OpaqueListCount = 0;
	Scene->TransListCount = 0;

	nglCurScene = Scene;

	return Scene;
}

/*-----------------------------------------------------------------------------
Description: Terminate a sub-scene.
-----------------------------------------------------------------------------*/
void nglListEndScene()
{
	if (nglCurScene == &nglDefaultScene)
		nglFatal("Scene stack underflow (too many nglListEndScene calls!).\n");

	nglCurScene = nglCurScene->Parent;
}

/*-----------------------------------------------------------------------------
Description: Select a sub-scene.
-----------------------------------------------------------------------------*/
nglScene *nglListSelectScene(nglScene *scene)
{
	nglScene *prev = nglCurScene;
	nglCurScene = scene;

	return prev;
}

/*-----------------------------------------------------------------------------
Description: Begins the scene list, and the main scene (initialize the render
             lists: quads, meshes, light, etc.).
-----------------------------------------------------------------------------*/
void nglListInit()
{
	nglListWorkReset();

	// Reset the light lists.
	nglDefaultLightContext = nglCreateLightContext();

	// Set up the initial scene.
	nglCurScene = NULL;
	nglListBeginScene();

	// Init default animation time.
	nglDefaultScene.AnimTime = (float) nglGetVBlankCount() * (1.0f / 60.0f);

	// Show performance info from the previous render list.
#if (NGL_DEBUG && !NGL_PROFILING)
	if (nglDebug.ShowPerfInfo)
#endif

#if (NGL_DEBUG || NGL_PROFILING)
	{
		nglQuad q;
		nglInitQuad(&q);
		nglSetQuadR(&q, 470.0f, 0.0f, 639.0f, 205.0f);
		nglSetQuadC(&q, 0.0f, 0.1f, 0.15f, 0.85f);
		nglSetQuadZ(&q, 0.0f);
		nglListAddQuad(&q);
		nglListAddString(nglSysFont, 480.0f, 25.0f, 0.0f,
                         "\1[A0A0A0FF]%.2f FPS\n"
                         "%.2f %%GPU TOTAL\n"
                         "%.2f %%GPU PS\n"
                         "%.2f %%GPU VS\n"
                         "%.2f %%CPU DPC\n"
                         "%.2f %%CPU TOTAL\n"
                         "%.2fms LISTSEND\n%2.2fms RENDER\n%d VERTS\n%d POLYS\n%d MESH NODES",
                         nglPerfInfo.FPS.Value, nglPerfInfo.GPU.Value,
                         nglPerfInfo.GPU_BackEnd.Value,
                         nglPerfInfo.GPU_FrontEnd.Value,
                         nglPerfInfo.CPU_DPC.Value,
                         nglPerfInfo.CPU_Total.Value, nglPerfInfo.ListSendMS,
                         nglPerfInfo.RenderMS, nglPerfInfo.TotalVerts,
                         nglPerfInfo.TotalPolys, nglNodesCount);
	}

	nglPerfInfo.TotalPolys = 0;
	nglPerfInfo.TotalVerts = 0;

	QueryPerformanceCounter(&nglPerfInfo.RenderStart);
#endif // (NGL_DEBUG || NGL_PROFILING)

#if NGL_DEBUG
	if (nglDebug.DumpFrameLog)
		nglLog("============================= Frame log start ===========================\n");
#endif
}

/*-----------------------------------------------------------------------------
Description: 
-----------------------------------------------------------------------------*/
class nglOpaqueCompare
{
public:
	bool operator() (nglListNode *NodeA, nglListNode *NodeB)
	{
		// Sort by ascending hash.
		return NodeA->SortInfo.Hash < NodeB->SortInfo.Hash;
	}
};

class nglTransCompare
{
public:
	bool operator() (nglListNode *NodeA, nglListNode *NodeB)
	{
		// Sort by descending dist, preserving submission order for equal dists.
		if (NodeA->SortInfo.Dist > NodeB->SortInfo.Dist)
			return true;
		else if (NodeA->SortInfo.Dist < NodeB->SortInfo.Dist)
			return false;
		else
			// Preserve submission order for nodes at the same distance.
			// (the pointer value can be used for this, because nodes are allocated sequentially each frame in memory).
			return (uint32) NodeA < (uint32) NodeB;
	}
};

nglListNode **nglOpenRenderList(nglListNode *List, uint32 ListCount)
{
	nglListNode **result = (nglListNode **) nglListAlloc(sizeof(nglListNode *) * ListCount);

	if (result)
	{
		nglListNode **p = result;

		while (List)
		{
			*p++ = List;
			List = List->Next;
		}
	}

	return result;
}

void nglCloseRenderList(nglListNode ** NodeTable, nglListNode *(&ListHead), uint32 ListCount)
{
	nglListNode **p;
	nglListNode *prev = (nglListNode *) NULL;

	for (p = NodeTable + (ListCount - 1); ListCount; p--, ListCount--)
	{
		(*p)->Next = prev;
		prev = *p;
	}

	ListHead = prev;
	nglListWorkPos = (uint8 *) NodeTable;
}

/*-----------------------------------------------------------------------------
Description: Render the nodes lists (opaque and translucent).
-----------------------------------------------------------------------------*/
void nglRenderNodes()
{
	// Set the view port.
	// If the render target is a swizzled texture, the viewport must be the entire texture size !
	// Scene->Zmin/Zmax range is [0,1].
	D3DVIEWPORT8 vp = {
		(DWORD) nglCurScene->ViewX1, (DWORD) nglCurScene->ViewY1,
		(DWORD) nglCurScene->ViewX2 - (DWORD) nglCurScene->ViewX1 + 1,
		(DWORD) nglCurScene->ViewY2 - (DWORD) nglCurScene->ViewY1 + 1,
		nglCurScene->ZMin, nglCurScene->ZMax
	};

	// All this crap is to clear the render target.
	if (nglCurScene->RenderTarget->Format & NGLTF_LINEAR)
	{
		// Set the render target.
		NGL_D3DTRY(nglDev->SetRenderTarget(nglCurScene->RenderTarget->DXSurface, nglDepthBufferSurface));

		NGL_D3DTRY(nglDev->SetViewport(&vp));

		// Clear the renger target (color buffer, zbuffer, stencil buffer).
		if (nglCurScene->ClearFlags)
			NGL_D3DTRY(nglDev->Clear(0, NULL, nglCurScene->ClearFlags,
                                     nglCurScene->ClearColor, nglCurScene->ClearZ,
                                     nglCurScene->ClearStencil));
	}
	// This crappy stuff is required when rendering to a swizzled texture ! :(
	// Otherwise the zbuffer will not be cleared correctly.
	// There are some other ways to do it (a bit faster) but it will require a lot more memory.
	else
	{
		// Clear the shared depth buffer (zbuffer, stencil) according to the scene flags.
		NGL_D3DTRY(nglDev->SetRenderTarget(nglFakeSurface, nglDepthBufferSurface));

		NGL_D3DTRY(nglDev->SetViewport(&vp));

		if (nglCurScene->ClearFlags && nglCurScene->ClearFlags != NGLCLEAR_COLOR)
			NGL_D3DTRY(nglDev->Clear(0, NULL, nglCurScene->ClearFlags,
                                     nglCurScene->ClearColor, nglCurScene->ClearZ,
                                     nglCurScene->ClearStencil));
		// Set the render target.
		NGL_D3DTRY(nglDev->SetRenderTarget(nglCurScene->RenderTarget->DXSurface, nglDepthBufferSurface));
		// Clear the color buffer.
		if (nglCurScene->ClearFlags & NGLCLEAR_COLOR)
			NGL_D3DTRY(nglDev->Clear(0, NULL, NGLCLEAR_COLOR, nglCurScene->ClearColor, 0.0f, 0));
	}

#if NGL_DEBUG
	// Change the mesh's fill mode.
	if (nglDebug.DrawAsPoints)
		nglDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
	else if (nglDebug.DrawAsLines)
		nglDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		nglDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
#endif

	// Set the write mask when rendering the geometry to the framebuffer.
	if (nglCurScene->FBWriteMask != nglPrevFBWriteMask)
	{
		nglDev->SetRenderState(D3DRS_COLORWRITEENABLE, nglCurScene->FBWriteMask);
		nglPrevFBWriteMask = nglCurScene->FBWriteMask;
	}

	// Render the opaque nodes.
	nglListNode *Node = nglCurScene->OpaqueRenderList;

	while (Node)
	{
		// Execute the callback function that render this node.
		Node->NodeFn(Node->NodeData);
#if (NGL_DEBUG || NGL_PROFILING)
		if (Node->Type == NGLNODE_MESH_SECTION)
			nglNodesCount++;
#endif
		Node = Node->Next;
	}

	// Render the translucent nodes.
	Node = (nglListNode *) nglCurScene->TransRenderList;

	while (Node)
	{
		// Execute the callback function that render this node.
		Node->NodeFn(Node->NodeData);
#if (NGL_DEBUG || NGL_PROFILING)
		if (Node->Type == NGLNODE_MESH_SECTION)
			nglNodesCount++;
#endif
		Node = Node->Next;
	}

#if NGL_DEBUG
	// Quads fill mode is set to solid otherwise when using the point/wireframe modes
	// fonts are not displayed correctly.
	nglDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
#endif
}

/*-----------------------------------------------------------------------------
Description: Set the dynamic bits of the vertex shader key (typically NGLP_xxx
             parameters and lighting configuration).
-----------------------------------------------------------------------------*/
void nglSetupRuntimeVSKey(void *NodeData)
{
	nglMeshSectionNode *SectionNode = (nglMeshSectionNode *) NodeData;
	nglMeshNode *MeshNode = SectionNode->MeshNode;
	nglMeshSection *Section = SectionNode->Section;
	nglMesh *Mesh = MeshNode->Mesh;

	// Initialize the full vshader key from the partial one.
	SectionNode->FullVSKey = Section->PartialVSKey;

	// Tint ?
	if (MeshNode->Params.Flags & NGLP_TINT)
		// Set the tint bit.
		SectionNode->FullVSKey |= NGL_VSFLAG_TINT;
	else
		SectionNode->FullVSKey &= ~NGL_VSFLAG_TINT;

#if NGL_DEBUG
	if (!nglStage.Tint)
		SectionNode->FullVSKey &= ~NGL_VSFLAG_TINT;
#endif

	// Fog ?
	if ((Section->Material->Flags & NGLMAT_FOG) && nglCurScene->Fog)
		SectionNode->FullVSKey |= NGL_VSFLAG_FOG;
	else
		SectionNode->FullVSKey &= ~NGL_VSFLAG_FOG;

#if NGL_DEBUG
	if (!nglStage.Fog)
		SectionNode->FullVSKey &= ~NGL_VSFLAG_FOG;
#endif

	// Reset the light config for the section.
	SectionNode->LightConfig = 0;

	// Clear the lighting bits (2x 4-bits).
	SectionNode->FullVSKey &= ~(((uint32) 0xF << NGL_VSFLAG_DIRLIGHT_OFS) | ((uint32) 0xF << NGL_VSFLAG_PNTLIGHT_OFS));

	// Setup the lighting bits.
	// This function modify the follwing members:
	// - SectionNode->FullVSKey
	// - SectionNode->LightConfig
	if (Mesh->Flags & NGL_LIGHTCAT_MASK
        && (!(MeshNode->Params.Flags & NGLP_NO_LIGHTING))
        && Section->Material->Flags & NGLMAT_LIGHT)
	{
		nglSetupLightConfig(SectionNode);
	}
}

/*-----------------------------------------------------------------------------
Description: Render the scenes tree.
-----------------------------------------------------------------------------*/
void nglRenderScene(nglScene *Scene)
{
	// First recursively render the children to take care of dependencies.
	nglScene *Child = Scene->FirstChild;

	while (Child)
	{
		nglRenderScene(Child);
		Child = Child->NextSibling;
	}

	// Render the current scene.
	nglCurScene = Scene;


#ifdef PROJECT_KELLYSLATER
	if (nglCurScene->RenderTarget != &nglBackBufferTex) 
	{
		XGVECTOR4 Scale(
			((float) (nglCurScene->ViewX2 - nglCurScene->ViewX1 + 1)) * 0.5f, 
			(-(float) (nglCurScene->ViewY2 - nglCurScene->ViewY1 + 1)) * 0.5f, 
			16777215.0f, 
			1.0f
		);
		XGVECTOR4 Offset(
			((float) (nglCurScene->ViewX2 - nglCurScene->ViewX1 + 1)) * 0.5f, 
			((float) (nglCurScene->ViewY2 - nglCurScene->ViewY1 + 1)) * 0.5f, 
			0.0f, 
			0.0f
		);
		nglDev->SetVertexShaderConstant(nglVSC_Scale, Scale, 1);
		nglDev->SetVertexShaderConstant(nglVSC_Offset, Offset, 1);
	}
#endif

	// Configure for z testing
	nglDev->SetRenderState(D3DRS_ZENABLE, (Scene->ZWriteEnable||Scene->ZTestEnable)?D3DZB_TRUE:D3DZB_FALSE);
	nglDev->SetRenderState(D3DRS_ZFUNC, Scene->ZTestEnable?D3DCMP_LESSEQUAL:D3DCMP_ALWAYS);

	// Calculate the current IFL frame.
	nglCurScene->IFLFrame = nglFTOI(nglCurScene->AnimTime * nglIFLSpeed);

	// At this point, the lights list has been built and the NGLP_xxx parameters
	// are known, so we can we can setup the vshader key for each section.
	// Once we have the vertex shader key, we can get the vertex shader handle
	// required for sorting by vertex shaders (it's a lot better to sort by vshader
	// handles than by vshader keys because it allows the client to sort by handle
	// as well. VSKey are NGL internal specific.
	nglListNode *Node = nglCurScene->OpaqueRenderList;

	while (Node)
	{
		// Sections are sorted by vertex shader handles.
		if (Node->Type == NGLNODE_MESH_SECTION)
		{
			nglMeshSectionNode *SectionNode = (nglMeshSectionNode *) Node->NodeData;
			nglMeshSection *Section = SectionNode->Section;

			// Use the default NGL vshader key system sorting for sections that don't use customize renderer.
			if (Section->RenderSectionFunc == nglRenderSection)
			{
				// Setup the dynamic bits of the vshader key.
				nglSetupRuntimeVSKey(Node->NodeData);

				// Get the vshader handle from its key.
				SectionNode->VSHandle = nglGetVertexShaderFromDB(SectionNode->FullVSKey);
				Node->SortInfo.Hash = SectionNode->VSHandle;
			}
		}
		Node = Node->Next;
	}

	Node = nglCurScene->TransRenderList;

	while (Node)
	{
		// Sections are sorted by vertex shader handles.
		if (Node->Type == NGLNODE_MESH_SECTION)
		{
			nglMeshSectionNode *SectionNode = (nglMeshSectionNode *) Node->NodeData;
			nglMeshSection *Section = SectionNode->Section;

			// Use the default NGL vshader key system sorting for sections that don't use customize renderer.
			if (Section->RenderSectionFunc == nglRenderSection)
			{
				// Setup the dynamic bits of the vshader key.
				nglSetupRuntimeVSKey(Node->NodeData);

				// Get the vshader handle from its key.
				SectionNode->VSHandle = nglGetVertexShaderFromDB(SectionNode->FullVSKey);

				// This messes up the translucent sort order (dc 06/10/02)
//				Node->SortInfo.Hash = Section->VSHandle;
			}
		}
		Node = Node->Next;
	}

	// Sort the opaque list.
	nglListNode **NodeTable;
	NodeTable = nglOpenRenderList(nglCurScene->OpaqueRenderList, nglCurScene->OpaqueListCount);

	if (!NodeTable)
		return;

	sort(NodeTable, NodeTable + nglCurScene->OpaqueListCount, nglOpaqueCompare());

	nglCloseRenderList(NodeTable, nglCurScene->OpaqueRenderList, nglCurScene->OpaqueListCount);

	// Sort the translucent list.
	NodeTable = nglOpenRenderList(nglCurScene->TransRenderList, nglCurScene->TransListCount);

	if (!NodeTable)
		return;

	sort(NodeTable, NodeTable + nglCurScene->TransListCount, nglTransCompare());

	nglCloseRenderList(NodeTable, nglCurScene->TransRenderList, nglCurScene->TransListCount);

	// Render the nodes.
	nglRenderNodes();

#ifdef PROJECT_KELLYSLATER
	if (nglCurScene->RenderTarget != &nglBackBufferTex) 
	{
		nglDev->SetVertexShaderConstant(nglVSC_Scale,  XGVECTOR4(nglGetScreenWidthTV() * 0.5f, -nglGetScreenHeightTV() * 0.5f, 16777215.0f, 1.0f), 1);
		nglDev->SetVertexShaderConstant(nglVSC_Offset, XGVECTOR4(nglGetScreenWidth() * 0.5f,  nglGetScreenHeight() * 0.5f, 0.0f, 0.0f), 1);
	}
#endif
}

/*-----------------------------------------------------------------------------
Description: "Reset" the XBox GPU counters.
-----------------------------------------------------------------------------*/
void nglResetGPUCountersXB()
{
	DmEnableGPUCounter(false);
	DmEnableGPUCounter(true);
}

/*-----------------------------------------------------------------------------
Description: Render the skinned/non-skinned/scratch meshes and quads.
-----------------------------------------------------------------------------*/
void nglListSend(bool Flip)
{
	NGL_ASSERT(nglCurScene == &nglDefaultScene,
               "nglListSend called while one or more scenes were still active (need to call nglListEndScene).\n");

#ifndef NGL_ASYNC_LIST_SEND
	// Stall on the previous render and swap the frame buffers.
	if (Flip)
		nglFlip();
#endif

#if (NGL_DEBUG || NGL_PROFILING)
	QueryPerformanceCounter(&nglPerfInfo.ListSendStart);
	nglPerfInfo.EnableGPUCounters = 1;
#endif

#if (NGL_DEBUG || NGL_PROFILING)
	nglNodesCount = 0;
#endif

#if NGL_DEBUG
	nglSectionNumber = 0;
#endif

	__asm wbinvd;	// Magic words for flushing the CPU cache (dc 06/17/02)

	// Render the scene tree.
	nglRenderScene(&nglDefaultScene);

#if (NGL_DEBUG || NGL_PROFILING)
	// Timings/profiling calculations.

	// Enable GPU performance counters so we can read them to have profiling infos.
	// WARNING: it decreases the CPU performance !
	if (nglPerfInfo.EnableGPUCounters && (!nglPerfInfo.IsGPUCountersEnabled))
	{
		if (DmEnableGPUCounter(true) != XBDM_NOERR)
			nglWarning("DmEnableGPUCounter() failed !");
		else
			nglPerfInfo.IsGPUCountersEnabled = true;
	}

	// Disable GPU performance counters.
	if ((!nglPerfInfo.EnableGPUCounters) && nglPerfInfo.IsGPUCountersEnabled)
	{
		nglPerfInfo.IsGPUCountersEnabled = false;
		DmEnableGPUCounter(false);
		nglPerfInfo.GPU.Value = 0.0f;
		nglPerfInfo.GPU_BackEnd.Value = 0.0f;
		nglPerfInfo.GPU_FrontEnd.Value = 0.0f;
		nglPerfInfo.CPU_DPC.Value = 0.0f;
		nglPerfInfo.CPU_Total.Value = 0.0f;
		nglPerfInfo.FPS.Value = 0.0f;
	}

	// Retrieve GPU performance counters.
	if (nglPerfInfo.IsGPUCountersEnabled)
	{
#define PERCENT(x) (float)x.Data.CountValue.LowPart / (float)x.Data.RateValue.LowPart * 100.0f

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.GPU.Handle, nglPerfInfo.GPU.Type, &nglPerfInfo.GPU.Data) != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.GPU.Value = PERCENT(nglPerfInfo.GPU);

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.GPU.Handle, nglPerfInfo.GPU.Type, &nglPerfInfo.GPU.Data) != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.GPU.Value = PERCENT(nglPerfInfo.GPU);

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.GPU_BackEnd.Handle, nglPerfInfo.GPU_BackEnd.Type,
                                            &nglPerfInfo.GPU_BackEnd.Data) != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.GPU_BackEnd.Value = PERCENT(nglPerfInfo.GPU_BackEnd);

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.GPU_FrontEnd.Handle, nglPerfInfo.GPU_FrontEnd.Type,
                                            &nglPerfInfo.GPU_FrontEnd.Data) != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.GPU_FrontEnd.Value = PERCENT(nglPerfInfo.GPU_FrontEnd);

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.CPU_DPC.Handle, nglPerfInfo.CPU_DPC.Type,
                                            &nglPerfInfo.CPU_DPC.Data) != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.CPU_DPC.Value = PERCENT(nglPerfInfo.CPU_DPC);

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.CPU_Total.Handle, nglPerfInfo.CPU_Total.Type,
                                            &nglPerfInfo.CPU_Total.Data) != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.CPU_Total.Value = PERCENT(nglPerfInfo.CPU_Total);

		if (DmQueryPerformanceCounterHandle(nglPerfInfo.FPS.Handle, nglPerfInfo.FPS.Type, &nglPerfInfo.FPS.Data)  != XBDM_NOERR)
			nglResetGPUCountersXB();
		nglPerfInfo.FPS.Value = (float)nglPerfInfo.FPS.Data.CountValue.QuadPart / (float)nglPerfInfo.FPS.Data.RateValue.QuadPart;

#undef PERCENT
	}

	// ListSend/Rendering time calculations.
	QueryPerformanceCounter(&nglPerfInfo.ListSendFinish);
	nglPerfInfo.RenderFinish = nglPerfInfo.ListSendFinish;

	// Rendering time [ms] from nglListInit() to the end of nglListSend().
	nglPerfInfo.RenderMSTab[nglTimingIdx] = (float) ((((double)nglPerfInfo.RenderFinish.QuadPart -
	                                        (double)nglPerfInfo.RenderStart.QuadPart) / (double)nglTicksPerSec.QuadPart) * 1000.0);

	// Rendering time [ms] for nglListSend to complete.
	nglPerfInfo.ListSendMSTab[nglTimingIdx] = (float) ((((double)nglPerfInfo.ListSendFinish.QuadPart -
	                                          (double)nglPerfInfo.ListSendStart.QuadPart) / (double)nglTicksPerSec.QuadPart) * 1000.0);

	nglTimingIdx = (nglTimingIdx + 1) % NGL_TIMING_BUFDEPTH;

	if (nglTimingIdx % NGL_TIMING_BUFDEPTH == 0)
	{
		nglPerfInfo.RenderMS = 0.0f;

		for (int32 i = 0; i < NGL_TIMING_BUFDEPTH; i++)
			nglPerfInfo.RenderMS += nglPerfInfo.RenderMSTab[i];

		nglPerfInfo.RenderMS /= (float) NGL_TIMING_BUFDEPTH;

		nglPerfInfo.ListSendMS = 0.0f;

		for (int32 i = 0; i < NGL_TIMING_BUFDEPTH; i++)
			nglPerfInfo.ListSendMS += nglPerfInfo.ListSendMSTab[i];

		nglPerfInfo.ListSendMS /= (float) NGL_TIMING_BUFDEPTH;
	}
#endif // (NGL_DEBUG || NGL_PROFILING)

	// Required by static scratch meshes.
	//nglListWorkReset();	// Florent removed this to allow us to repeatedly call nglListSend (dc 06/24/02)

	// Lock the scratch meshes VB.
	nglLockScratchMeshVertexBuffer();
}

/*-----------------------------------------------------------------------------
Description: Adds a new node to the render list, after sorting it correctly.
             This will cause the node to be rendered by nglListSend calling
             the function NodeFn with the Data pointer.
-----------------------------------------------------------------------------*/
void nglListAddNode(uint32 Type, nglCustomNodeFn NodeFn, void *Data, nglSortInfo *SortInfo)
{
	nglListNode *NewNode = nglListNew(nglListNode);

	if (!NewNode)
		return;

	NewNode->Type = Type;
	NewNode->NodeFn = NodeFn;
	NewNode->NodeData = Data;
	memcpy(&NewNode->SortInfo, SortInfo, sizeof(nglSortInfo));

	// Insert the node in the right list (translucent/opaque).
	if (SortInfo->Type == NGLSORT_TRANSLUCENT)
	{
		NewNode->Next = (nglListNode *) nglCurScene->TransRenderList;
		nglCurScene->TransRenderList = NewNode;
		nglCurScene->TransListCount++;
	}
	else
	{
		NewNode->Next = (nglListNode *) nglCurScene->OpaqueRenderList;
		nglCurScene->OpaqueRenderList = NewNode;
		nglCurScene->OpaqueListCount++;
	}
}

/*-----------------------------------------------------------------------------
Description: Returns the next node to be added, seamlessly moves between lists.
-----------------------------------------------------------------------------*/
inline nglListNode *nglGetNextNode(nglListNode *Node)
{
	Node = Node->Next;

	while (Node && Node->Type == NGLNODE_BIN)
		Node = Node->Next;

	return Node;
}

/*-----------------------------------------------------------------------------
Description: Set the material blending mode (only if it has changed).
-----------------------------------------------------------------------------*/
void nglSetBlendingMode(int32 BlendMode, int32 BlendModeConst)
{
	if (!nglCurScene->ZWriteEnable)
	{
		if (BlendMode == NGLBM_OPAQUE)
			BlendMode = NGLBM_OPAQUE_NOZWRITE;
		else if (BlendMode == NGLBM_PUNCHTHROUGH)
			BlendMode = NGLBM_PUNCHTHROUGH_NOZWRITE;
	}

	// Only change the blending mode if it has changed.
	if (BlendMode == nglPrevBM)
		return;

	// Material blending. Only supports the same framebuffer blending mode for all the
	// 4 stages. That's a big difference compared to the PS2 architecture (blending
	// with the FB at each pass).
	switch (BlendMode)
	{
		// Blends the texel with the background, modulated by Alpha.
		case NGLBM_BLEND:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			nglDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// No translucency is performed, alpha is ignored.
		case NGLBM_OPAQUE:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, true);
			break;

		// No translucency, but if alpha is below a given threshold the pixel is skipped.
		case NGLBM_PUNCHTHROUGH:
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, true);
			nglDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			// Only fragments with alpha > 128 will be accepted.
			nglDev->SetRenderState(D3DRS_ALPHAREF, (DWORD) 0x80);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, true);
			break;

		// Blends the texel with the background, modulated by BlendModeConstant.
		case NGLBM_CONST_BLEND:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			nglDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_CONSTANTALPHA);
			nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTALPHA);
			// BlendModeConstant's range is [0,255].
			nglDev->SetRenderState(D3DRS_BLENDCOLOR, BlendModeConst << 24);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// Adds the texel to the background, modulated by Alpha.
		case NGLBM_ADDITIVE:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			nglDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// No translucency is performed, alpha is ignored.
		case NGLBM_OPAQUE_NOZWRITE:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// No translucency, but if alpha is below a given threshold the pixel is skipped.
		case NGLBM_PUNCHTHROUGH_NOZWRITE:
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, true);
			nglDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			// Only fragments with alpha > 128 will be accepted.
			nglDev->SetRenderState(D3DRS_ALPHAREF, (DWORD) 0x80);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// Adds the texel to the background, modulated by BlendModeConstant.
		case NGLBM_CONST_ADDITIVE:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			nglDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_CONSTANTALPHA);
			nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			// BlendModeConstant's range is [0,255].
			nglDev->SetRenderState(D3DRS_BLENDCOLOR, BlendModeConst << 24);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// Subtracts the texel from the background, modulated by Alpha.
		case NGLBM_SUBTRACTIVE:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			nglDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		// Subtracts the texel from the background, modulated by BlendModeConstant.
		case NGLBM_CONST_SUBTRACTIVE:
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			nglDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			// BlendModeConstant's range is [0,255].
			nglDev->SetRenderState(D3DRS_BLENDCOLOR, BlendModeConst << 24);
			nglDev->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;

		default:
			nglFatal("Unsupported blending mode !");
	}

	nglPrevBM = BlendMode;
}

/*-----------------------------------------------------------------------------
Description: Set the culling mode (only if it has changed).
-----------------------------------------------------------------------------*/
void nglSetCullingMode(uint32 MaterialFlags, uint32 ParamFlags)
{
	// If true, use CCW mode.
	int32 CurBackFaceCull = MaterialFlags & (NGLMAT_BACKFACE_CULL | NGLMAT_BACKFACE_DEFAULT) |
	                        (ParamFlags & NGLP_REVERSE_BACKFACECULL);

	// Only change the backface culling if it has changed.
	if (CurBackFaceCull == nglPrevBackFaceCull)
		return;

	nglPrevBackFaceCull = CurBackFaceCull;

	CurBackFaceCull &= ~NGLP_REVERSE_BACKFACECULL;

	if (ParamFlags & NGLP_REVERSE_BACKFACECULL)
		nglDev->SetRenderState(D3DRS_CULLMODE, CurBackFaceCull ? D3DCULL_CW : D3DCULL_NONE);
	else
		nglDev->SetRenderState(D3DRS_CULLMODE, CurBackFaceCull ? D3DCULL_CCW : D3DCULL_NONE);
}

/*-----------------------------------------------------------------------------
Description: Set the map texture clamping/wrapping mode.
-----------------------------------------------------------------------------*/
void nglSetMapAddressMode(uint32 MaterialFlags, uint32 stage)
{
	if (MaterialFlags & NGLMAT_CLAMP_U)
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	else
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);

	if (MaterialFlags & NGLMAT_CLAMP_V)
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	else
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
}

/*-----------------------------------------------------------------------------
Description: Set the lightmap texture clamping/wrapping mode.
-----------------------------------------------------------------------------*/
void nglSetLightmapAddressMode(uint32 MaterialFlags, uint32 stage)
{
	if (MaterialFlags & NGLMAT_LIGHT_CLAMP_U)
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	else
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);

	if (MaterialFlags & NGLMAT_LIGHT_CLAMP_V)
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	else
		nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
}

/*-----------------------------------------------------------------------------
Description: Set the enviro map texture mode.
-----------------------------------------------------------------------------*/
inline void nglSetEnviromapAddressMode(uint32 stage)
{
	nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
}

/*-----------------------------------------------------------------------------
Description: Set the detail map texture mode.
-----------------------------------------------------------------------------*/
inline void nglSetDetailmapAddressMode(uint32 stage)
{
	nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	nglDev->SetTextureStageState(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
}

/*-----------------------------------------------------------------------------
Description: Set the current pixel shader.
-----------------------------------------------------------------------------*/
void nglSetPixelShader(uint32 PS)
{
	// Cache the last pixel shader.
	if (PS == nglPrevPS)
		return;

	NGL_D3DTRY(nglDev->SetPixelShader(PS));
	nglPrevPS = PS;
}

/*-----------------------------------------------------------------------------
Description: Set the current vertex shader.
-----------------------------------------------------------------------------*/
void nglSetVertexShader(uint32 VS)
{
	// Cache the last vertex shader.
	if (VS == nglPrevVS)
		return;

	NGL_D3DTRY(nglDev->SetVertexShader(VS));
	nglPrevVS = VS;
}

/*-----------------------------------------------------------------------------
Description: Setup the normal lights/projector light vertex shaders.
-----------------------------------------------------------------------------*/
void nglSetupLightsVS(nglMeshSectionNode *SectionNode)
{
	nglMeshNode *MeshNode = SectionNode->MeshNode;
	nglMeshSection *Section = SectionNode->Section;
	nglLightConfigStruct *LightCfg = SectionNode->LightConfig;
	u_int TotalLightCount = LightCfg->DirLightCount + LightCfg->PntLightCount + LightCfg->ProjLightCount;
	u_int NormalLightCount = 0;
	u_int ProjLightCount = 0;

	for (uint32 i = 0; i < TotalLightCount; i++)
	{
		if (LightCfg->Type[i] == NGLLIGHT_DIRECTIONAL)
		{
			// The section has directional light.
			// Info: Light->Dir[3] is 0.
			nglDirLightInfo *Light = (nglDirLightInfo *) LightCfg->LightPtr[i];

			nglVector LocalDir;
			nglApplyMatrix(LocalDir, MeshNode->WorldToLocal, Light->Dir);

			LocalDir[3] = 0.0f;	// Just to be sure.
			nglVectorNormalize3(LocalDir);

			nglLightStruct *l = &nglVSC_Light[NormalLightCount++];
			l->Type = NGL_DIR_LIGHT;
			nglDev->SetVertexShaderConstant(l->Dir, &LocalDir, 1);
			nglDev->SetVertexShaderConstant(l->Col, &Light->Color, 1);
		}
		else if (LightCfg->Type[i] == NGLLIGHT_FAKEPOINT)
		{
			// The section has fake point light.
			nglPointLightInfo *Light = (nglPointLightInfo *) LightCfg->LightPtr[i];

			// Transform light to local space.
			nglVector LightPos;
			nglVectorCopy(&LightPos, &Light->Pos);
			LightPos[3] = 1.0f;
			nglApplyMatrix(LightPos, MeshNode->WorldToLocal, LightPos);

			nglVector V;
			float Range, Scale;
			nglVectorSubtract4(V, Section->SphereCenter, LightPos);
			Range = (float) sqrt(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]);
			nglVectorScale4(V, V, 1.0f / Range);
			V[3] = 0.0f;

			if (Light->Far <= Light->Near)
				Scale = 1.0f;
			else
			{
				Scale = 1.0f - (Range - Light->Near) / (Light->Far - Light->Near);
				if (Scale < 0.0f)
					Scale = 0.0f;
				if (Scale > 1.0f)
					Scale = 1.0f;
			}

			nglVector Color;
			nglVectorScale4(Color, Light->Color, Scale);

			nglLightStruct *l = &nglVSC_Light[NormalLightCount++];
			l->Type = NGL_DIR_LIGHT;
			nglDev->SetVertexShaderConstant(l->Dir, &V, 1);
			nglDev->SetVertexShaderConstant(l->Col, &Color, 1);
		}

		/*
		   // true point light is not implemented yet.
		   else if (LightCfg->Type[i] == NGLLIGHT_true_POINTLIGHT)
		   {
		   // The section has true point light.
		   nglPointLightInfo* Light = (nglPointLightInfo*)LightCfg->LightPtr[i];

		   // Transform light to local space.
		   nglVector LightPos;
		   nglVectorCopy(LightPos, Light->Pos);
		   LightPos[3] = 1.0f;
		   nglApplyMatrix(LightPos, MeshNode->WorldToLocal, LightPos);

		   nglLightStruct* l = &nglVSC_Light[NormalLightCount++];
		   l->Type = NGL_PNT_LIGHT;
		   nglDev->SetVertexShaderConstant(l->Dir , &LightPos, 1);
		   nglDev->SetVertexShaderConstant(l->Col , &Light->Color, 1);
		   nglDev->SetVertexShaderConstant(l->Near , &Light->Near, 1);
		   nglDev->SetVertexShaderConstant(l->Far , &Light->Far, 1);
		   }
		 */

		else if (LightCfg->Type[i] == NGLLIGHT_PROJECTED_DIRECTIONAL)
		{
			// The section has projected directional light.
			nglDirProjectorLightInfo *Light = (nglDirProjectorLightInfo *) LightCfg->LightPtr[i];

			// Point to the current projector's data.
			nglProjectorParamStruct *Proj = &nglProjectorParam[ProjLightCount++];

			// Save the blending mode/const and texture.
			Proj->BM = Light->BlendMode;
			Proj->BMConst = Light->BlendModeConstant;
			Proj->Tex = Light->Tex;

			nglMatrix LocalToUV;
			nglMatrixMul(LocalToUV, Light->WorldToUV, MeshNode->LocalToWorld);
			// Store the 2 first rows (x,y) of the LocalToUV matrix (transposed).
			for (int32 v = 0; v < 4; v++)
			{
				Proj->Local2UV[0][v] = LocalToUV[v][0];
				Proj->Local2UV[1][v] = LocalToUV[v][1];
			}

			// Set the texture scaling factor.
			// Warning: linear textures must be square ! (allow to only use 1 mul in the VS).
			if (Proj->Tex->Format & NGLTF_LINEAR)
				Proj->TexScale = (float) Proj->Tex->Width;
			else
				Proj->TexScale = 1.0f;	// Swizzle textures don't need to be scaled.

			// Store the LocalLightDir vector.
			nglVector LocalLightDir;
			nglApplyMatrix(LocalLightDir, MeshNode->WorldToLocal,
						   Light->Zaxis);
			memcpy(Proj->Dir, LocalLightDir, sizeof(nglVector));
			Proj->Type = NGL_DIR_PROJECTOR;	// Used to quickly know the projector type.
		}

		/*
		   // Point projected light is not implemented yet.
		   else if (LightCfg->Type[i] == NGLLIGHT_PROJECTED_POINT)
		   {
		   // The section has projected point light.
		   nglPointProjectorLightInfo* Light = (nglPointProjectorLightInfo*)LightNode->Data;

		   nglVector LocalPos;
		   nglApplyMatrix(LocalPos, MeshNode->WorldToLocal, Light->Pos);

		   //nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, LocalPos, sizeof(nglVector) );
		   //nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Light->Color, sizeof(nglVector) );
		   //nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &Light->Range, sizeof(float) );
		   }
		 */

	}

}

/*-----------------------------------------------------------------------------
Description: 
-----------------------------------------------------------------------------*/
void nglSetupLightConfig(nglMeshSectionNode *SectionNode)
{
	// Bit of the VSKey used for normal lighting:
	// bit 11-26: Reserved (16-bits) to indicate the light configuration.
	//            01 (binary) encodes a point light, 10 encodes a directional light and 00 encodes no light.
	//            2-bits/light with 8 lights max => 16-bits are required (not optimal but convenient).

	// Bit of the VSKey used for projector lights:
	// bit 27-42: Reserved (16-bits) to indicate the projector light configuration.
	//            01 (binary) encodes a point proj, 10 encodes a directional proj, 11 encodes a spot proj
	//            and 00 encodes no projector light.
	//            2-bits/projector light with 8 proj max => 16-bits are required (not optimal but convenient).

	// Projector light VS are generated in nglInit().

	nglMeshNode *MeshNode = SectionNode->MeshNode;
	nglMeshSection *Section = SectionNode->Section;
	nglMesh *Mesh = MeshNode->Mesh;

	// Allocate the light config struct from the render list memory.
	SectionNode->LightConfig = (nglLightConfigStruct *) nglListAlloc(sizeof(nglLightConfigStruct));
	nglLightConfigStruct *LightCfg = SectionNode->LightConfig;
	uint32 LightCount = 0;		// Total number of lights affecting the section (including projected lights).

	LightCfg->DirLightCount = 0;
	LightCfg->PntLightCount = 0;
	LightCfg->ProjLightCount = 0;

	// Initialize the current light context.
	nglCurLightContext = (nglLightContext *) nglDefaultLightContext;
	if (MeshNode->Params.Flags & NGLP_LIGHT_CONTEXT)
		nglCurLightContext = (nglLightContext *) MeshNode->Params.LightContext;

	// Find the mesh's center in world space.
	nglVector WorldPos;
	nglApplyMatrix(WorldPos, MeshNode->LocalToWorld, Section->SphereCenter);

	// For each potential light.
	for (int32 i = 0; i < NGL_MAX_LIGHTS; i++)
	{
		// If the mesh matches this category, run through the context looking for lights that affect it.
		if (!(Mesh->Flags & (1 << (NGL_LIGHTCAT_SHIFT + i))))
			continue;

		// Check against the list of "normal" lights.

#ifdef PROJECT_SPIDERMAN
		// For some reasons, there is a lighting bug in subway_ax where some walls are lit weirdly ... (?)
		// By allowing lighting only for skin-meshes "fix" the bug (crap).
		if (Mesh->NBones)
#endif

#if NGL_DEBUG
			if (nglStage.Light)
#endif
			{
				nglLightNode *LightNode = nglCurLightContext->Head.Next[i];

				while (LightNode != &nglCurLightContext->Head)
				{
					// Check if the section has a directional light.
					if (LightNode->Type == NGLLIGHT_DIRECTIONAL)
					{
						nglDirLightInfo *Light = (nglDirLightInfo *) LightNode->Data;

#if NGL_DEBUG
						if (nglDebug.DumpFrameLog & 16)
							nglLog("Directional (%x) (%.2f,%.2f,%.2f) Color (%.2f,%.2f,%.2f,%.2f).\n",
     							   LightNode->LightCat >> NGL_LIGHTCAT_SHIFT,
							       Light->Dir[0], Light->Dir[1], Light->Dir[2],
							       Light->Color[0], Light->Color[1],
							       Light->Color[2], Light->Color[3]);
#endif

						LightCfg->Type[LightCount] = (uint8) LightNode->Type;
						LightCfg->LightPtr[LightCount] = Light;
						LightCfg->DirLightCount++;
						LightCount++;

						if (LightCfg->DirLightCount + LightCfg->PntLightCount >= NGL_MAX_LIGHTS)
							break;	// Clamp the number of lights/section to NGL_MAX_LIGHTS.
					}

					// Check if the section has a fake or true point light.
					else
					{
						nglPointLightInfo *Light = (nglPointLightInfo *) LightNode->Data;

						if (nglSpheresIntersect(Light->Pos, Light->Far, WorldPos, Section->SphereRadius))
						{
							if (LightNode->Type == NGLLIGHT_FAKEPOINT)
							{
#if NGL_DEBUG
								if (nglDebug.DumpFrameLog & 16)
									nglLog("FakePoint (%x) at (%.2f,%.2f,%.2f) Color (%.2f,%.2f,%.2f,%.2f) Range %.2f->%.2f.\n",
									       LightNode->
									       LightCat >> NGL_LIGHTCAT_SHIFT,
									       Light->Pos[0], Light->Pos[1],
									       Light->Pos[2], Light->Color[0],
									       Light->Color[1], Light->Color[2],
									       Light->Color[3], Light->Near,
									       Light->Far);
#endif

								LightCfg->DirLightCount++;
							}
							else
							{
#if NGL_DEBUG
								if (nglDebug.DumpFrameLog & 16)
									nglLog("truePoint (%x) at (%.2f,%.2f,%.2f) Color (%.2f,%.2f,%.2f,%.2f) Range %.2f->%.2f.\n",
									       LightNode->
									       LightCat >> NGL_LIGHTCAT_SHIFT,
									       Light->Pos[0], Light->Pos[1],
									       Light->Pos[2], Light->Color[0],
									       Light->Color[1], Light->Color[2],
									       Light->Color[3], Light->Near,
									       Light->Far);
#endif

								LightCfg->PntLightCount++;
							}

							LightCfg->Type[LightCount] = (uint8) LightNode->Type;
							LightCfg->LightPtr[LightCount] = Light;
							LightCount++;

							if (LightCfg->DirLightCount + LightCfg->PntLightCount >= NGL_MAX_LIGHTS)
								break;	// Clamp the number of lights/section to NGL_MAX_LIGHTS.
						}
					}

					LightNode = LightNode->Next[i];
				}
			}

		// Check against the list of projector lights.

		// For now, skin meshes can't have shadows casted on them.
		if (!Mesh->NBones)
#if NGL_DEBUG
			if (nglStage.Projector)
#endif
			{
				nglLightNode *LightNode = nglCurLightContext->ProjectorHead.Next[i];

				while (LightNode != &nglCurLightContext->ProjectorHead)
				{
					// Check if the section has projected directional light.
					if (LightNode->Type == NGLLIGHT_PROJECTED_DIRECTIONAL)
					{
						nglDirProjectorLightInfo *Light = (nglDirProjectorLightInfo *) LightNode->Data;

						if (nglIsSphereVisible(&Light->Frustum, WorldPos, Section->SphereRadius))
						{
							LightCfg->Type[LightCount] = (uint8) LightNode->Type;
							LightCfg->LightPtr[LightCount] = Light;
							LightCfg->ProjLightCount++;
							LightCount++;

							if (LightCfg->ProjLightCount >=	NGL_MAX_PROJECTORS)
								break;	// Clamp the number of projectors/section to NGL_MAX_PROJECTORS.
						}
					}

					// TODO:
					// For now, point projector light is not supported.
#if 0
					else if (LightNode->Type == NGLLIGHT_PROJECTED_POINT)
					{
						nglPointProjectorLightInfo *Light = (nglPointProjectorLightInfo *) LightNode->Data;

						if (nglSpheresIntersect(Light->Pos, Light->Range, WorldPos, Section->SphereRadius))
						{

						}
					}
#endif

					LightNode = LightNode->Next[i];
				}
			}

	}

	// If there is at least one light, update the vshader key.
	if (LightCount)
	{
		// Set the VSKey's lighting bits.
		SectionNode->FullVSKey |= (LightCfg->DirLightCount << NGL_VSFLAG_DIRLIGHT_OFS) |
			                      (LightCfg->PntLightCount << NGL_VSFLAG_PNTLIGHT_OFS);
	}
	else
	{
		SectionNode->LightConfig = 0;
	}

}

/*-----------------------------------------------------------------------------
Description: Setup the right vertex shader for the current section.
-----------------------------------------------------------------------------*/
void nglSectionSetVertexShader(nglMeshSectionNode *SectionNode, uint32 MaterialFlags)
{
	nglMeshNode *MeshNode = SectionNode->MeshNode;
	nglMeshSection *Section = SectionNode->Section;
	nglMaterial *Material = Section->Material;

	bool UpdateUV = false;

	// Pass the local to screen matrix to the vertex shader.
	XGMATRIX LocalToScreen;
	XGMatrixTranspose(&LocalToScreen, (XGMATRIX *)&MeshNode->LocalToScreen);
	nglDev->SetVertexShaderConstant(nglVSC_LocalToScreen, &LocalToScreen, 4);
	// Cache matrix for projector light. Avoid to calculate the vertex transform several times.
	memcpy(nglProjVertexTransform, LocalToScreen, sizeof(nglMatrix));

	// Fog ?
	if (SectionNode->FullVSKey & NGL_VSFLAG_FOG)
	{
		// nglFogMin & nglFogMax range is [0,1].
		// Parameters for the vertex shader:
		// c[C_FOGPARAMS].x = nglFogMin
		// c[C_FOGPARAMS].y = nglFogNearZ
		// c[C_FOGPARAMS].z = 1 / ((nglFogFarZ - nglFogNearZ) * (nglFogMax - nglFogMin))
		nglVector FogParams;
		FogParams[0] = nglCurScene->FogMin;
		FogParams[1] = nglCurScene->FogNear;
		FogParams[2] = 1.0f / ((nglCurScene->FogFar - nglCurScene->FogNear) *
		               (nglCurScene->FogMax - nglCurScene->FogMin));
		nglDev->SetRenderState(D3DRS_FOGCOLOR, nglCurScene->FogColor);
		nglDev->SetVertexShaderConstant(nglVSC_FogParams, &FogParams, 1);
	}

	// This variable is used to store the scroll_UV values and scale_UV values (detail map).
	nglVector UVparam(0.0f, 0.0f, 0.0f, 0.0f);

	// Scrolled textures ?
	// Params.ScrollU/V is the scrolling amount.
	// Material.ScrollU/V is the scroll rate per scond.
	if ((MeshNode->Params.Flags & NGLP_TEXTURE_SCROLL) || (MaterialFlags & NGLMAT_UV_SCROLL))
	{
		UpdateUV = true;
		if (MeshNode->Params.Flags & NGLP_TEXTURE_SCROLL)
		{
			UVparam[0] += MeshNode->Params.ScrollU;
			UVparam[1] += MeshNode->Params.ScrollV;
		}
		if (MaterialFlags & NGLMAT_UV_SCROLL)
		{
			UVparam[0] += nglCurScene->AnimTime * Material->ScrollU;
			UVparam[1] += nglCurScene->AnimTime * Material->ScrollV;
		}
	}

	// Detailmap ?
	// Set the detail map UV scaling value.
	if (MaterialFlags & NGLMAT_DETAIL_MAP)
	{
		UpdateUV = true;
		UVparam[2] = Material->DetailMapUScale;
		UVparam[3] = Material->DetailMapVScale;
	}

	// If UV need to be updated, sent the UV parameters to the vertex shader.
	if (UpdateUV)
	{
		nglDev->SetVertexShaderConstant(nglVSC_UVParams, &UVparam, 1);
	}

	// Enviromap ?
	if (MaterialFlags & NGLMAT_ENVIRONMENT_MAP)
	{
		// Get the view position vector into world space.
		nglVector WorldViewPos(
			nglCurScene->ViewToWorld[3][0],
			nglCurScene->ViewToWorld[3][1],
			nglCurScene->ViewToWorld[3][2],
			1.0f);
		nglDev->SetVertexShaderConstant(nglVSC_CameraPos, &WorldViewPos, 1);

		// Pass the local to world matrix to the vertex shader.
		XGMATRIX m;
		XGMatrixTranspose(&m, (XGMATRIX *)&MeshNode->LocalToWorld);
		nglDev->SetVertexShaderConstant(nglVSC_LocalToWorld, &m, 4);
	}
	
	// Tint ?
	// Multiply vertex colors by a tint color, before lighting.
	// Tint color should be a RGBA nglVector. Range is not bound to 1.0 for each component.
	if (SectionNode->FullVSKey & NGL_VSFLAG_TINT)
	{
		nglDev->SetVertexShaderConstant(nglVSC_Tint, &MeshNode->Params.TintColor, 1);
	}
	
	// Setup the normal lights/projector light vertex shaders.
	// The LightConfig pointer is only valid if the section is lit or has projected light(s).
	if (SectionNode->LightConfig)
		nglSetupLightsVS(SectionNode);

	// Falloff ? (not implemented yet)
	/*
	   if (MaterialFlags & NGLMAT_ALPHA_FALLOFF)
	   {
	   // TODO: alpha falloff support.
	   // Transform the View vector into local space.
	   nglVector LocalViewDir = { 0, 0, -1, 0 };
	   nglApplyMatrix(LocalViewDir, nglCurScene->ViewToWorld, LocalViewDir);
	   nglApplyMatrix(LocalViewDir, MeshNode->WorldToLocal, LocalViewDir);
	   LocalViewDir[3] = Material->AlphaFalloff;

	   //nglVif1AddCommandListProgram(Packet, CmdListDataPtr, nglAlphaFalloffAddr);
	   //nglVif1AddCommandListData(Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, LocalViewDir, sizeof(nglVector));
	   }
	 */

	// Material color ?
	if (MaterialFlags & NGLMAT_MATERIAL_COLOR)
	{
		nglVector v;  // Color format is RGBA.
		v[0] = (float) ((Material->Color >> 16) & 0xff) / 255.0f;
		v[1] = (float) ((Material->Color >> 8) & 0xff) / 255.0f;
		v[2] = (float) ((Material->Color >> 0) & 0xff) / 255.0f;
		v[3] = (float) ((Material->Color >> 24) & 0xff) / 255.0f;
		nglDev->SetVertexShaderConstant(nglVSC_MaterialColor, &v, 1);
	}
	else
	{
		nglDev->SetVertexShaderConstant(nglVSC_MaterialColor, &nglVectorOne, 1);
	}

	nglSetVertexShader(SectionNode->VSHandle);
}

/*-----------------------------------------------------------------------------
Description: Texture stages stuff.
-----------------------------------------------------------------------------*/

// Possible texture stage combinations (coded on 6-bits, 64 possibilities - reserved for futur features).
// Required at the vertex shader level.
// NOTE: if you add a new stage combo, don't forget to update the nglSetShaderConfig() function.
enum
{
	// 0 stage (aka no texture).
	NGL_TS_NOTEX,

	// 1 stage.
	NGL_TS_T,
	NGL_TS_E,

	// 2 stages.
	NGL_TS_TD,
	NGL_TS_TE,
	NGL_TS_TL,
	NGL_TS_EL,

	// 3 stages.
	NGL_TS_TDE,
	NGL_TS_TDL,
	NGL_TS_TEL,

	// 4 stages.
	NGL_TS_TDEL,

	NGL_TS_MAX,                  // Used to retrieve the #elements.

	NGL_TS_INVALID = NGL_TS_MAX  // Invalid combination, reserved for VS database initialization.
};

/*-----------------------------------------------------------------------------
Description: Setup the texture stages configuration (used by the vshader, see
             NGL_TS_xxx) and the pixel shader index (see NGL_PS_xxx) according
             to the material properties.
			 The NGL_TS_xxx value is stored in Section->VSKey (6 first bits).
			 The NGL_PS_xxx value is stored in Section->PSIdx.
-----------------------------------------------------------------------------*/
void nglSetShaderConfig(nglMeshSection *Section)
{
	nglMaterial *Material = Section->Material;
	uint32 TexCombo = Material->Flags & (NGLMAT_TEXTURE_MAP | NGLMAT_LIGHT_MAP | NGLMAT_DETAIL_MAP |
		NGLMAT_ENVIRONMENT_MAP | NGLMAT_BUMP_MAP);
	uint32 TSCfg = 0;	// initialize with something
	uint32 PSIdx = 0;	// initialize with something

	if (TexCombo)
	{
		switch (TexCombo)
		{
			// ---==[ 1 stage combinations: T, E ]==---

			// T, T_MC
			case (NGLMAT_TEXTURE_MAP):
				TSCfg = NGL_TS_T;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_T_MC;
				else
					PSIdx = NGL_PS_T;
				break;

			// E, E_MC
			case (NGLMAT_ENVIRONMENT_MAP):
				Material->MapBlendMode = Material->EnvironmentMapBlendMode;
				Material->MapBlendModeConstant = Material->EnvironmentMapBlendModeConstant;
				TSCfg = NGL_TS_E;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_E_MC;
				else
					PSIdx = NGL_PS_E;
				break;

			// ---==[ 2 stages combinations: TD, TE, TL, EL ]==---

			// TD, TD_MC
			case (NGLMAT_TEXTURE_MAP | NGLMAT_DETAIL_MAP):
				TSCfg = NGL_TS_TD;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_TD_MC;
				else
					PSIdx = NGL_PS_TD;
				break;

			// TE, TE_MC
			case (NGLMAT_TEXTURE_MAP | NGLMAT_ENVIRONMENT_MAP):
				TSCfg = NGL_TS_TE;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_TE_MC;
				else
					PSIdx = NGL_PS_TE;
				break;

			// TL, TL_MC, TL2, TL2_MC
			case (NGLMAT_TEXTURE_MAP | NGLMAT_LIGHT_MAP):
				TSCfg = NGL_TS_TL;
				if (Material->LightMapBlendMode == NGLBM_ADDITIVE)
				{
					if (Material->Flags & NGLMAT_MATERIAL_COLOR)
						PSIdx = NGL_PS_TL2_MC;
					else
						PSIdx = NGL_PS_TL2;
				}
				else
				{
					if (Material->Flags & NGLMAT_MATERIAL_COLOR)
						PSIdx = NGL_PS_TL_MC;
					else
						PSIdx = NGL_PS_TL;

				}
				break;

			// EL, EL_WIN
			case (NGLMAT_ENVIRONMENT_MAP | NGLMAT_LIGHT_MAP):
				TSCfg = NGL_TS_EL;
				Material->MapBlendMode = Material->EnvironmentMapBlendMode;
				Material->MapBlendModeConstant = Material->EnvironmentMapBlendModeConstant;

				// Hack to have framed-windows rendered properly in spiderman.
				if (Material->LightMapBlendMode == NGLBM_SUBTRACTIVE)
				{
					Material->MapBlendMode = NGLBM_BLEND;
					PSIdx = NGL_PS_EL_WIN;
				}
				else
				{
					PSIdx = NGL_PS_EL;
				}
				break;

			// ---==[ 3 stages combinations: TDE, TDL, TEL ]==---

			// TDE, TDE_MC
			case (NGLMAT_TEXTURE_MAP | NGLMAT_DETAIL_MAP | NGLMAT_ENVIRONMENT_MAP):
				TSCfg = NGL_TS_TDE;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_TDE_MC;
				else
					PSIdx = NGL_PS_TDE;
				break;

			// TDL, TDL_MC
			case (NGLMAT_TEXTURE_MAP | NGLMAT_DETAIL_MAP | NGLMAT_LIGHT_MAP):
				TSCfg = NGL_TS_TDL;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_TDL_MC;
				else
					PSIdx = NGL_PS_TDL;
				break;

			// TEL, TEL_MC, TEL2, TEL2_MC, TEL3, TEL3_MC,
			case (NGLMAT_TEXTURE_MAP | NGLMAT_ENVIRONMENT_MAP | NGLMAT_LIGHT_MAP):
				TSCfg = NGL_TS_TEL;
				// TEL2: Assuming: diffuse1 (opaque) + enviro (additive) + diffuse2 (blend).
				if (Material->EnvironmentMapBlendMode == NGLBM_ADDITIVE)
				{
					if (Material->Flags & NGLMAT_MATERIAL_COLOR)
						PSIdx = NGL_PS_TEL2_MC;
					else
						PSIdx = NGL_PS_TEL2;
				}
				// TEL3: Assuming: diffuse1 (opaque) + enviro (blend const) + diffuse2 (additive).
				else if (Material->LightMapBlendMode == NGLBM_ADDITIVE)
				{
					if (Material->Flags & NGLMAT_MATERIAL_COLOR)
						PSIdx = NGL_PS_TEL3_MC;
					else
						PSIdx = NGL_PS_TEL3;
				}
				// TEL: Assuming: diffuse1 (opaque) + enviro (blend const) + diffuse2 (blend).
				else
				{
					if (Material->Flags & NGLMAT_MATERIAL_COLOR)
						PSIdx = NGL_PS_TEL_MC;
					else
						PSIdx = NGL_PS_TEL;
				}
				break;

			// ---==[ 4 stages combinations: TDEL ]==---

			// TDEL, TDEL_MC
			case (NGLMAT_TEXTURE_MAP | NGLMAT_DETAIL_MAP | NGLMAT_ENVIRONMENT_MAP | NGLMAT_LIGHT_MAP):
				TSCfg = NGL_TS_TDEL;
				if (Material->Flags & NGLMAT_MATERIAL_COLOR)
					PSIdx = NGL_PS_TDEL_MC;
				else
					PSIdx = NGL_PS_TDEL;
				break;

			default:
#ifndef PROJECT_KELLYSLATER
				// Need to find a way of not triggering this warning for custom-rendered sections. (dc 05/23/02
				nglError("Unsupported texture stages combination (%d) ! (see nglGetTSConfig)\n", TexCombo);
#endif
				break;
		}
	}
	else
	{
		// ---==[ 0 stage combination: NOTEX ]==---

		// No texture.
		// NOTEX
		TSCfg = NGL_TS_NOTEX;
		PSIdx = NGL_PS_NOTEX;
	}

	// Clear the current texture stage config (TSCfg is stored on 6 bits).
	Section->PartialVSKey &= ~((1 << 6) - 1);

	// Update the section's shaders.
	Section->PartialVSKey |= TSCfg;
	Section->PSIdx = PSIdx;
}

/*-----------------------------------------------------------------------------
Description: Texture stages configuration.
-----------------------------------------------------------------------------*/
void ngl_ts_notex(nglMaterial *Material, uint32 *stage)
{
	nglBindTexture(0, Material->Flags, 0);
	*stage = 1;
}

void ngl_ts_t(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglBindTexture(Material->Map, Material->Flags, 0);
	*stage = 1;
}

void ngl_ts_e(nglMaterial *Material, uint32 *stage)
{
	nglSetEnviromapAddressMode(0);
	nglBindTexture(Material->EnvironmentMap, Material->Flags, 0);
	*stage = 1;
}

void ngl_ts_td(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetDetailmapAddressMode(1);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->DetailMap, Material->Flags, 1);
	*stage = 2;
}

void ngl_ts_te(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetEnviromapAddressMode(1);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->EnvironmentMap, Material->Flags, 1);
	*stage = 2;
}

void ngl_ts_tl(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetLightmapAddressMode(Material->Flags, 1);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->LightMap, Material->Flags, 1);
	*stage = 2;
}

void ngl_ts_el(nglMaterial *Material, uint32 *stage)
{
	nglSetEnviromapAddressMode(0);
	nglSetLightmapAddressMode(Material->Flags, 1);
	nglBindTexture(Material->EnvironmentMap, Material->Flags, 0);
	nglBindTexture(Material->LightMap, Material->Flags, 1);
	*stage = 2;
}

void ngl_ts_tde(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetDetailmapAddressMode(1);
	nglSetEnviromapAddressMode(2);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->DetailMap, Material->Flags, 1);
	nglBindTexture(Material->EnvironmentMap, Material->Flags, 2);
	*stage = 3;
}

void ngl_ts_tdl(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetDetailmapAddressMode(1);
	nglSetLightmapAddressMode(Material->Flags, 2);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->DetailMap, Material->Flags, 1);
	nglBindTexture(Material->LightMap, Material->Flags, 2);
	*stage = 3;
}

void ngl_ts_tel(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetEnviromapAddressMode(1);
	nglSetLightmapAddressMode(Material->Flags, 2);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->EnvironmentMap, Material->Flags, 1);
	nglBindTexture(Material->LightMap, Material->Flags, 2);
	*stage = 3;
}

void ngl_ts_tdel(nglMaterial *Material, uint32 *stage)
{
	nglSetMapAddressMode(Material->Flags, 0);
	nglSetDetailmapAddressMode(1);
	nglSetEnviromapAddressMode(2);
	nglSetLightmapAddressMode(Material->Flags, 3);
	nglBindTexture(Material->Map, Material->Flags, 0);
	nglBindTexture(Material->DetailMap, Material->Flags, 1);
	nglBindTexture(Material->EnvironmentMap, Material->Flags, 2);
	nglBindTexture(Material->LightMap, Material->Flags, 3);
	*stage = 4;
}

// WARNING: Order MUST matches the NGL_PS_xxx enum (defined in ngl_xbox_internal.h) !
// Function that associates a pixel shader to its texture stage configuration.
// It does the following matching: f(x) = y with x in NGL_PS_xxx and y in NGL_TS_xxx.
void (*nglTSFunc[NGL_PS_MAX_PASS1]) (nglMaterial *Material, uint32 *stage) =
{
	// 0 stage.
	ngl_ts_notex,

	// 1 stage.
	ngl_ts_t,
	ngl_ts_t,
	ngl_ts_e,
	ngl_ts_e,

	// 2 stages.
	ngl_ts_td,
	ngl_ts_td,
	ngl_ts_te,
	ngl_ts_te,
	ngl_ts_tl,
	ngl_ts_tl,
	ngl_ts_tl,
	ngl_ts_tl,
	ngl_ts_el,
	ngl_ts_el,

	// 3 stages.
	ngl_ts_tde,
	ngl_ts_tde,
	ngl_ts_tdl,
	ngl_ts_tdl,
	ngl_ts_tel,
	ngl_ts_tel,
	ngl_ts_tel,
	ngl_ts_tel,
	ngl_ts_tel,
	ngl_ts_tel,

	// 4 stages.
	ngl_ts_tdel,
	ngl_ts_tdel,
};

/*-----------------------------------------------------------------------------
Description: Setup the stream source, setup the right pixel shader and apply
             the textures stages (up to 4) to the current section.
-----------------------------------------------------------------------------*/
void nglSectionSetPixelShader(nglMeshSectionNode *Node, uint32 MaterialFlags)
{
	nglMeshNode *MeshNode = Node->MeshNode;
	nglMeshSection *Section = Node->Section;
	nglMaterial *Material = Section->Material;

	// Animated texture support.
	if (MeshNode->Params.Flags & NGLP_TEXTURE_FRAME)
		nglTextureAnimFrame = MeshNode->Params.TextureFrame;
	else
		nglTextureAnimFrame = nglCurScene->IFLFrame;

	// Set the texture stages config and the pixel shader.
	uint32 stage;
	nglTSFunc[Section->PSIdx](Material, &stage);
	nglSetPixelShader(nglPixelShaderHandle[Section->PSIdx]);

	// Pass the enviromap blend mode constant to the pshader.
	if (Material->Flags & NGLMAT_ENVIRONMENT_MAP)
	{
		static nglVector v(0.0f, 0.0f, 0.0f, 0.0f);
		v[3] = (float) Material->EnvironmentMapBlendModeConstant / 255.0f;
		nglDev->SetPixelShaderConstant(PSC_ENVIROMAP_BM_CONST, &v, 1);
	}

	// Disable unused texture stages.
	if (stage < NGL_MAX_TS)
	{
		nglDev->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		nglDev->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
		nglDev->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
}

/*-----------------------------------------------------------------------------
Description: Draw the current section (standard/scratch mesh).
-----------------------------------------------------------------------------*/
void nglDrawSection(nglMeshSectionNode *Node)
{
	nglMesh *Mesh = Node->MeshNode->Mesh;
	nglMeshSection *Section = Node->Section;

	nglVertexBuffer *VB = Node->Section->VB;

	// Scratch mesh rendering (temporary and static).
	if (Mesh->Flags & NGLMESH_SCRATCH_MESH)
	{
		nglPrimitiveInfo* PrimInfo = (nglPrimitiveInfo*)Section->PrimInfo;

		for (uint32 i = 0; i < Section->PrimCount; i++, PrimInfo++)
		{
			NGL_D3DTRY(nglDev->DrawVertices((D3DPRIMITIVETYPE)Section->PrimType, PrimInfo->VertIndex, PrimInfo->VertCount));
		}
	}

	// "Standard" mesh rendering.
	else
	{
		// Passing the bones to the vshader when rendering a skinned mesh.
		if (Mesh->NBones)
		{
			NGL_ASSERT(Node->MeshNode->Params.Bones, "Can't render skin section: no bones in NGL params !");

			int32 reg = nglVSC_Bones;

			for (uint32 k = 0; k < Section->NBones; k++)
			{
				uint16 BoneIdx = Section->BonesIdx[k];
				XGMATRIX BM;
				XGMatrixMultiply(&BM, (XGMATRIX *)&(Mesh->Bones[BoneIdx]), (XGMATRIX *)&Node->MeshNode->Params.Bones[BoneIdx]);
				XGMatrixMultiply(&BM, &BM, (XGMATRIX *)&Node->MeshNode->WorldToLocal);
				XGMatrixTranspose(&BM, &BM);
				nglDev->SetVertexShaderConstant(reg, &BM, 3);
				reg += 3;		// += 3 because bones are 3x3 matrices.
			}
		}

#if NGL_USE_PUSHBUFFER
		if (Section->PB)
		{
			// Run the pushbuffer that renders the section (only when there is enough vertices to render).
			NGL_D3DTRY(nglDev->RunPushBuffer(Section->PB, NULL));
		}
		else
		{
			uint16 *Idx = (uint16 *) ((uint32) VB->IndexBufferData + ((uint32) Section->IndexOffset << 1));
			NGL_D3DTRY(nglDev->DrawIndexedVertices((D3DPRIMITIVETYPE)Section->PrimType, Section->IndexCount, Idx));
		}
#else
		uint16 *Idx = (uint16 *) ((uint32) VB->IndexBufferData + ((uint32) Section->IndexOffset << 1));
		NGL_D3DTRY(nglDev->DrawIndexedVertices((D3DPRIMITIVETYPE)Section->PrimType, Section->IndexCount, Idx));
#endif

	}

#if (NGL_DEBUG || NGL_PROFILING)
	nglPerfInfo.TotalVerts += Section->IndexCount;
	nglPerfInfo.TotalPolys += Section->IndexCount - 2;
#endif
}

/*-----------------------------------------------------------------------------
Description: Render a section (a subpart of a skinned or non-skinned mesh).
-----------------------------------------------------------------------------*/
void nglRenderSection(void *Data)
{
	nglMeshSectionNode *SectionNode = (nglMeshSectionNode *) Data;
	nglMesh *Mesh = SectionNode->MeshNode->Mesh;
	nglMeshSection *Section = SectionNode->Section;
	nglMaterial *Material = Section->Material;

	uint32 MaterialFlags = SectionNode->MaterialFlags;
	MaterialFlags &= nglMaterialFlagsAnd;
	MaterialFlags |= nglMaterialFlagsOr;

#if NGL_DEBUG
	if (!nglStage.MatAlpha && Material->Flags & NGLMAT_ALPHA)
		return;
	if (!nglStage.MatLight && Material->Flags & NGLMAT_LIGHT)
		return;
	// Skip this section if it matches the specified texture stage combination.
	uint32 TS = (uint32) (SectionNode->FullVSKey & NGL_VSFLAG_TS_MASK);
	if (!nglStage.PassNOTEX && (TS == NGL_TS_NOTEX)) return;
	if (!nglStage.PassT && (TS == NGL_TS_T))         return;
	if (!nglStage.PassE && (TS == NGL_TS_E))         return;
	if (!nglStage.PassTD && (TS == NGL_TS_TD))       return;
	if (!nglStage.PassTE && (TS == NGL_TS_TE))       return;
	if (!nglStage.PassTL && (TS == NGL_TS_TL))       return;
	if (!nglStage.PassEL && (TS == NGL_TS_EL))       return;
	if (!nglStage.PassTDE && (TS == NGL_TS_TDE))     return;
	if (!nglStage.PassTDL && (TS == NGL_TS_TDL))     return;
	if (!nglStage.PassTEL && (TS == NGL_TS_TEL))     return;
	if (!nglStage.PassTDEL && (TS == NGL_TS_TDEL))   return;

	if (nglDebug.DispMat
		&& (nglCurScene->RenderTarget->Format & NGLTF_LINEAR))
	{
		NGL_D3DTRY(nglDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xA0A0A0A0, 1.0f, 0));
	}

	if (nglDebug.DumpFrameLog & 2)
		nglLog("Adding section %s(%d) of %s (%d).\n",
			   MaterialFlags & NGLMAT_TEXTURE_MAP ? Material->Map->FileName.
			   c_str() : "", Section - Mesh->Sections, Mesh->Name.c_str(),
			   nglNodesCount);
#endif

#if NGL_DEBUG
	if (nglStage.BackFace)
		nglSetCullingMode(MaterialFlags, SectionNode->MeshNode->Params.Flags);
	else
		nglDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
#else
	nglSetCullingMode(MaterialFlags, SectionNode->MeshNode->Params.Flags);
#endif

	// If tint makes the object translucent (if it was opaque), force the blending of the mesh with the framebuffer.
	// NOTE: MaterialFlags override the original Material->Flags value, BUT Material->Flags remains unmodified.
	// A more readable way of writting this line would be:
	// if ((SectionNode->MeshNode->Params.Flags & NGLP_TINT) && (SectionNode->MeshNode->Params.TintColor[3] < 1.0f) &&
	//     (Material->MapBlendMode == NGLBM_OPAQUE || Material->MapBlendMode == NGLBM_PUNCHTHROUGH))
	if ((MaterialFlags & NGLMAT_ALPHA) && (!(Material->Flags & NGLMAT_ALPHA)))
		nglSetBlendingMode(NGLBM_BLEND, 0);
	else
		// Set the blending mode with the framebuffer according to the diffuse1 map's parameters.
		nglSetBlendingMode(Material->MapBlendMode, Material->MapBlendModeConstant);

	// Set the stream source.
	if (Mesh->Flags & NGLMESH_TEMP)
		nglBindVertexBuffer(nglScratchVertBuf, Mesh->VertexSize);
	else
#if !NGL_FORCE_CREATE_VB
		nglBindVertexBuffer((D3DVertexBuffer *) &SectionNode->Section->VB->VertexBuffer, Mesh->VertexSize);
#else
		nglBindVertexBuffer((D3DVertexBuffer *) SectionNode->Section->VB->VertexBufferData, Mesh->VertexSize);
#endif

	// Setup the vertex shader.
	nglSectionSetVertexShader(SectionNode, MaterialFlags);

	// Setup the pixel shader and apply the textures stages.
	nglSectionSetPixelShader(SectionNode, MaterialFlags);

	// Render the section. Could be a scratch or standard mesh.
	nglDrawSection(SectionNode);

	// Projected lights rendering.
	if (SectionNode->LightConfig)
	{
		uint32 ProjCount = ((nglLightConfigStruct *) SectionNode->LightConfig)->ProjLightCount;

		if (ProjCount)
		{
			// Setup an alpha-test for the shadows to improve the fillrate.
			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, true);
			nglDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			nglDev->SetRenderState(D3DRS_ALPHAREF, 0);  // Only non-black fragments are accepted.

			nglSetBlendingMode(NGLBM_BLEND, 0);

			// Pass the transformation matrix to the VS; this matrix is cached (Computed in nglSectionSetVertexShader).
			nglDev->SetVertexShaderConstant(nglVSC2_Projector_LocalToScreen, &nglProjVertexTransform, 4);

			// Use multi-texture shadows instead of multi-pass shadows.
			// Faster but more buggy.

			// Render the projector lights passes.
			for (uint32 pass = 0; pass < (ProjCount + NGL_MAX_TS - 1) / NGL_MAX_TS; pass++)
			{
				// Enumerate the texture stages used per pass (yeah, I could calculate these values on-the-fly).
				static uint32 TexStages[NGL_MAX_PROJECTORS / NGL_MAX_TS][NGL_MAX_PROJECTORS] = {
					{1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4},  // 1st pass.
					{0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4},  // 2nd pass.
					{0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4}   // 3rd pass.
				};

				uint32 i;

				// Number of texture stages used for the current pass.
				uint32 TexStagesUsed = TexStages[pass][ProjCount - 1];
				float TexScale[NGL_MAX_TS];

				// Pass the projector light parameters to the VS.
				for (i = 0; i < TexStagesUsed; i++)
				{
					uint32 reg = (pass * NGL_MAX_TS) + i;
					nglProjectorParamStruct *Proj = &nglProjectorParam[reg];
					nglProjectorRegStruct *ProjReg = &nglVSC2_Projector[i];

					NGL_ASSERT(Proj->Tex, "Trying to render a projected light that has no texture !");

					// Set the texture stages.
					// Assuming that projectors doesn't use animated textures.
					nglDev->SetTexture(i, (IDirect3DTexture8 *) Proj->Tex->DXTexture.Simple);
					nglDev->SetTextureStageState(i, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
					nglDev->SetTextureStageState(i, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
					nglDev->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
					nglDev->SetTextureStageState(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);	// Bilinear filtering.
					nglDev->SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
					nglDev->SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTEXF_NONE);	// No mipmap filtering.

					// Set the VS registers.
					// First 2 rows of the LocalToUV matrix for UV tranform.
					nglDev->SetVertexShaderConstant(ProjReg->Local2UV, Proj->Local2UV, 2);
					// ProjLightDir vector.
					nglDev->SetVertexShaderConstant(ProjReg->Dir, Proj->Dir, 1);

					TexScale[i] = Proj->TexScale;
				}

				// Pass the texture scaling factor (TexHeight if the texture is linear, 1 otherwise).
				nglDev->SetVertexShaderConstant(nglVSC2_Projector_TexScale, TexScale, 1);

				if (TexStagesUsed < NGL_MAX_TS)
				{
					nglDev->SetTextureStageState(TexStagesUsed, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
					nglDev->SetTextureStageState(TexStagesUsed, D3DTSS_COLOROP, D3DTOP_DISABLE);
					nglDev->SetTextureStageState(TexStagesUsed, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
				}

				// Set the vertex shader.
				nglSetVertexShader(nglVSDirProjector[TexStagesUsed - 1]);

				// Set the pixel shader.
				nglSetPixelShader(nglPixelShaderHandle[NGL_PS_1PROJ + TexStagesUsed - 1]);

				nglDrawSection(SectionNode);
			}

			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglFlushBlendingMode();
		}
	}

#if NGL_DEBUG
	if (nglDebug.DispMat && (nglCurScene->RenderTarget->Format & NGLTF_LINEAR))
	{
		NGL_D3DTRY(nglDev->Present(NULL, NULL, NULL, NULL));

		if (nglDebug.InfoPrints)
			nglInfo("Section %d (mesh)\n", nglSectionNumber++);
	}
#endif
}

/*-----------------------------------------------------------------------------
Description: Set up an orthogonal ViewToScreen matrix.
-----------------------------------------------------------------------------*/
void nglSetOrthoMatrix(float cx, float cy, float nearz, float farz,
					   float zmin, float zmax)
{
	nglCurScene->UsingOrtho = true;

	// Cache values.
	nglCurScene->CX = cx;
	nglCurScene->CY = cy;
	nglCurScene->FarZ = farz;
	nglCurScene->NearZ = nearz;
	nglCurScene->ZMin = zmin;
	nglCurScene->ZMax = zmax;

#ifdef PROJECT_KELLYSLATER
	XGMatrixOrthoOffCenterLH((XGMATRIX *)&nglCurScene->ViewToScreen, -cx/2, cx/2, -cy/2, cy/2, nearz, farz);
#else
	// When rendering shadows, a clipping with the nearz plane occurs (?).
	// By pushing the nearz a bit fix the problem (dirty).  --Flo
	float NearZAdjust = -500.0f;

	float FBWidth = (float) nglGetScreenWidth();
	float FBHeight = (float) nglGetScreenHeight();
	float TexWidth = (float) nglCurScene->RenderTarget->Width;
	float TexHeight = (float) nglCurScene->RenderTarget->Height;
	/*
	   // Use this if you want to scale (could be usefull for shadows).
	   static float Scale = 1.0f;
	   float XhalfRange = FBWidth / TexWidth / Scale;
	   float YhalfRange = FBHeight / TexHeight / Scale;
	   float Xofs = (FBWidth - TexWidth) / TexWidth / Scale;
	   float Yofs = (FBHeight - TexHeight) / TexHeight / Scale;
	 */
	float XhalfRange = FBWidth / TexWidth;
	float YhalfRange = FBHeight / TexHeight;
	float Xofs = (FBWidth - TexWidth) / TexWidth;
	float Yofs = (FBHeight - TexHeight) / TexHeight;
	XGMatrixOrthoOffCenterLH((XGMATRIX *)&nglCurScene->ViewToScreen,
							 -XhalfRange + Xofs, XhalfRange + Xofs,
							 -YhalfRange - Yofs, YhalfRange - Yofs,
							 nglCurScene->NearZ + NearZAdjust, nglCurScene->FarZ);
#endif
}

/*-----------------------------------------------------------------------------
Description: Set up for perspective rendering.  hfov is horizontal fov
             in degrees (ie 90.0).
-----------------------------------------------------------------------------*/
void nglSetPerspectiveMatrix(float hfov_deg, float cx, float cy, float nearz,
							 float farz, float zmin, float zmax)
{
	nglCurScene->UsingOrtho = false;

	// TODO: modify it to implement center of projection (CX, CY)...
	nglCurScene->CX = cx;
	nglCurScene->CY = cy;
	nglCurScene->FarZ = farz;
	nglCurScene->NearZ = nearz;
	nglCurScene->HFOV = hfov_deg;

	nglCurScene->ZMin = zmin;
	nglCurScene->ZMax = zmax;

	// Calculate the vertical FOV from the horizontal one.
	float hfov = XGToRadian(hfov_deg);
	float vfov = 2.0f * (float) atan(tan(hfov * 0.5f) / nglAspectRatio);

	float aspect = (float) nglGetScreenWidth() / (float) nglGetScreenHeight();

#ifdef PROJECT_KELLYSLATER
	// Right now we need to have at least CX working for multiplayer.
	// This should be removed when the function is finished and working like the PS2 version.
	float left, right, top, bottom;

	top = -nearz * (float) tan(vfov * 0.5f);
	bottom = -top;
	left = top * aspect;
	right = bottom * aspect;

	float sx = right - left;
	left = -cx / (float) nglGetScreenWidth() * sx;
	right = left + sx;

	XGMatrixPerspectiveOffCenterLH((XGMATRIX *)&nglCurScene->ViewToScreen, left, right, top, bottom, nearz, farz);
#else
	XGMatrixPerspectiveFovLH((XGMATRIX *)&nglCurScene->ViewToScreen, vfov, aspect, nearz, farz);
#endif

	// Compute the nglCurScene->WorldToScreen matrix.
	nglMatrixMul(nglCurScene->WorldToScreen, nglCurScene->ViewToScreen, nglCurScene->WorldToView);
}

/*-----------------------------------------------------------------------------
Description: Get the rendering parameters.
-----------------------------------------------------------------------------*/
void nglGetProjectionParams(float *hfov, float *cx, float *cy, float *nearz,
							float *farz)
{
	if (hfov)
		*hfov = nglCurScene->HFOV;
	if (cx)
		*cx = nglCurScene->CX;
	if (cy)
		*cy = nglCurScene->CY;
	if (nearz)
		*nearz = nglCurScene->NearZ;
	if (farz)
		*farz = nglCurScene->FarZ;
}

/*-----------------------------------------------------------------------------
Description: Apply the world to view transformation (FFD).
-----------------------------------------------------------------------------*/
void nglSetWorldToViewMatrix(const nglMatrix &WorldToView)
{
	// Store the nglCurScene->WorldToView matrix.
	nglMatrixCopy(&nglCurScene->WorldToView, &WorldToView);

	// Compute the nglCurScene->ViewToWorld matrix.
	XGMatrixInverse((XGMATRIX *)&nglCurScene->ViewToWorld, NULL, (XGMATRIX *)&nglCurScene->WorldToView);

	// Compute the nglCurScene->WorldToScreen matrix.
	nglMatrixMul(nglCurScene->WorldToScreen, nglCurScene->ViewToScreen, nglCurScene->WorldToView);
}

/*-----------------------------------------------------------------------------
Description: Set the color used to clear the screen.
-----------------------------------------------------------------------------*/
void nglSetClearColor(float R, float G, float B, float A)
{
	nglCurScene->ClearColor = D3DCOLOR_ARGB(nglFTOI(A * 255.0f),
											nglFTOI(R * 255.0f),
											nglFTOI(G * 255.0f),
											nglFTOI(B * 255.0f));
}

/*-----------------------------------------------------------------------------
Description: Enable/disable the scene's fog.
-----------------------------------------------------------------------------*/
void nglEnableFog(bool Enable)
{
	nglCurScene->Fog = Enable;
}

/*-----------------------------------------------------------------------------
Description: Set the fog color.
-----------------------------------------------------------------------------*/
void nglSetFogColor(float R, float G, float B)
{
	nglCurScene->FogColor = D3DCOLOR_ARGB(0xFF,
										  nglFTOI(R * 255.0f),
										  nglFTOI(G * 255.0f),
										  nglFTOI(B * 255.0f));
}

/*-----------------------------------------------------------------------------
Description: Set the fog range.
             Min and Max are in the range [0,1].
-----------------------------------------------------------------------------*/
void nglSetFogRange(float Near, float Far, float Min, float Max)
{
	nglCurScene->FogNear = Near;
	nglCurScene->FogFar = Far;
	nglCurScene->FogMin = Min;
	nglCurScene->FogMax = Max;
}

/*-----------------------------------------------------------------------------
Description: Set the render target clearing flags.
-----------------------------------------------------------------------------*/
void nglSetClearFlags(uint32 ClearFlags)
{
	nglCurScene->ClearFlags = ClearFlags;
}

/*-----------------------------------------------------------------------------
Description: The the Z value used to clear the render target's z-buffer.
-----------------------------------------------------------------------------*/
void nglSetClearZ(float Z)
{
	nglCurScene->ClearZ = Z;
}

/*-----------------------------------------------------------------------------
Description: The the stencil value used to clear the render target's stencil
             buffer.
-----------------------------------------------------------------------------*/
void nglSetClearStencil(uint32 Stencil)
{
	nglCurScene->ClearStencil = Stencil;
}

/*-----------------------------------------------------------------------------
Description: Set a write mask when rendering to the framebuffer.
             See NGL_FBWRITE_xxx flags in ngl_xbox.h.
-----------------------------------------------------------------------------*/
void nglSetFBWriteMask(uint32 WriteMask)
{
	nglCurScene->FBWriteMask = WriteMask;
}

/*-----------------------------------------------------------------------------
Description: Enable/disable z-buffer writes for the current scene.
-----------------------------------------------------------------------------*/
void nglSetZWriteEnable(bool Enable)
{
	nglCurScene->ZWriteEnable = Enable;
}

/*-----------------------------------------------------------------------------
Description: Enable/disable z-buffer testing for the current scene.
-----------------------------------------------------------------------------*/
void nglSetZTestEnable(bool Enable)
{
	nglCurScene->ZTestEnable = Enable;
}

/*-----------------------------------------------------------------------------
Description: Set the render target of the current scene.
-----------------------------------------------------------------------------*/
void nglSetRenderTarget(nglTexture *Tex, bool Download)
{
	NGL_ASSERT(Tex, "nglSetRenderTarget() needs a valid texture to render to !");

	nglCurScene->RenderTarget = Tex;
}

/*-----------------------------------------------------------------------------
Description: Set the viewport of the current scene.
-----------------------------------------------------------------------------*/
void nglSetViewport(uint32 x1, uint32 y1, uint32 x2, uint32 y2)
{
	nglCurScene->ViewX1 = x1;
	nglCurScene->ViewY1 = y1;
	nglCurScene->ViewX2 = x2;
	nglCurScene->ViewY2 = y2;
}

/*-----------------------------------------------------------------------------
Description: Projects a point from World space (X,Y,Z) to Screen space
             (0..nglGetScreenWidth()-1, 0..nglGetScreenHeight()-1).
-----------------------------------------------------------------------------*/
void nglProjectPoint(nglVector &Out, nglVector &In)
{
  In[3] = 1.0f;
  nglApplyMatrix(Out, nglCurScene->WorldToScreen, In);
  float InvW = 1.0f / Out[3];
#ifdef PROJECT_KELLYSLATER
  Out[0] = (Out[0] * InvW * nglGetScreenWidthTV() + nglGetScreenWidth()) * 0.5f;
  Out[1] = (nglGetScreenHeight() - Out[1] * InvW * nglGetScreenHeightTV()) * 0.5f;
#else
  Out[0] = (Out[0] * InvW + 1.0f) * 0.5f * nglGetScreenWidth();
  Out[1] = (1.0f - Out[1] * InvW) * 0.5f * nglGetScreenHeight();
#endif
}

/*-----------------------------------------------------------------------------
Description: Animation frame to use for texture scrolling and IFL animation.
             This is controllable per scene to allow frontend animation to
             continue while game animation stops. To get smooth IFL animation,
             multiples of 1/60th second are required.
             If not called, NGL's internal VBlank-based timer is used.
-----------------------------------------------------------------------------*/
void nglSetAnimTime(float Time)
{
	nglCurScene->AnimTime = Time;
}

/*-----------------------------------------------------------------------------
Description: Returns one of the current matrices in the
             model->view->projection chain.
-----------------------------------------------------------------------------*/
void nglGetMatrix(nglMatrix &Dest, uint32 ID)
{
	switch (ID)
	{
		case NGLMTX_VIEW_TO_WORLD:
			nglMatrixCopy(&Dest, &nglCurScene->ViewToWorld);
			break;
		case NGLMTX_VIEW_TO_SCREEN:
			nglMatrixCopy(&Dest, &nglCurScene->ViewToScreen);
			break;
		case NGLMTX_WORLD_TO_VIEW:
			nglMatrixCopy(&Dest, &nglCurScene->WorldToView);
			break;
		case NGLMTX_WORLD_TO_SCREEN:
			nglMatrixCopy(&Dest, &nglCurScene->WorldToScreen);
			break;
	}
}

/*---------------------------------------------------------------------------------------------------------
  Custom Node API

  The user has the possibility to create its own vertex/pixel shaders directly in ASM.
  Furthermore the default section/quad rendering function can be overriden.

---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Create a "user" pixel shader from the specified source code.
-----------------------------------------------------------------------------*/
void nglCreatePixelShader(nglShader *PS, const char *SrcCode, ...)
{
	char nglWork[4096];
	char FinalSrcCode[4096];
	char PreCode[] = "xps.1.1\n";

	va_list argptr;
	va_start(argptr, SrcCode);
	vsprintf(nglWork, SrcCode, argptr);
	va_end(argptr);

	NGL_ASSERT(SrcCode, "");
	NGL_ASSERT(strlen(SrcCode) - strlen(PreCode) < 4096,
			   "Too long source code !");

	// Add the ps version directive.
	strcpy(FinalSrcCode, PreCode);

	// Add src code.
	strcat(FinalSrcCode, nglWork);

	// Compile the pshader.
	if (XGAssembleShader("", FinalSrcCode, strlen(FinalSrcCode), 0, NULL, &PS->Opcode,
		NULL, NULL, NULL, NULL, NULL) != S_OK)
		nglFatal("Cannot compile pixel shader !");

	// Create it.
	D3DPIXELSHADERDEF *psdf = (D3DPIXELSHADERDEF *) PS->Opcode->pData;
	nglDev->CreatePixelShader(psdf, &PS->Handle);

	if (!PS->Handle)
		nglFatal("Cannot create pixel shader !");
}

/*-----------------------------------------------------------------------------
Description: Create a "user" vertex shader from the specified source code.
-----------------------------------------------------------------------------*/
void nglCreateVertexShader(const DWORD *Decl, nglShader *VS,
						   bool WTransform, const char *SrcCode, ...)
{
	char nglWork[4096];
	char FinalSrcCode[4096];
	char PreCode[] = "xvs.1.1\n#pragma screenspace\n";
	char PostCode[128];

	if (WTransform)
	{
		sprintf(PostCode,
				"mul oPos.xyz, r12, c[%d]\n"
				"+rcc r1.x, r12.w\n"
				"mad oPos.xyz, r12, r1.x, c[%d]\n",
				nglVSC_Scale, nglVSC_Offset);
	}
	else
	{
		PostCode[0] = 0;
	}
	NGL_ASSERT(strlen(PostCode) + 1 < sizeof(PostCode), "String overflow in nglCreateVertexShader.");

	va_list argptr;
	va_start(argptr, SrcCode);
	vsprintf(nglWork, SrcCode, argptr);
	va_end(argptr);

	NGL_ASSERT(SrcCode, "");
	NGL_ASSERT(strlen(nglWork) + strlen(PreCode) + strlen(PostCode) + 1 < 4096, "Too long source code !");

	// Add vs version and screenspace directives.
	strcpy(FinalSrcCode, PreCode);

	// Add src code.
	strcat(FinalSrcCode, nglWork);

	// Add the scaling/offseting at the end of the vshader code.
	strcat(FinalSrcCode, PostCode);

	// Compile the vshader.
	if (XGAssembleShader("", FinalSrcCode, strlen(FinalSrcCode), 
#if defined(PROJECT_KELLYSLATER) &&	defined(NGL_DEBUG)
		SASM_DONOTOPTIMIZE, 
#else
		0, 
#endif
		NULL, &VS->Opcode, NULL, NULL, NULL, NULL, NULL) != S_OK
	)
		nglFatal("Cannot compile vertex shader !");

	// Create it.
	NGL_D3DTRY(nglDev->CreateVertexShader(Decl, (DWORD *) (VS->Opcode->pData), &VS->Handle, 0));

	if (!VS->Handle)
		nglFatal("Cannot create vertex shader !");
}

/*-----------------------------------------------------------------------------
Description: Release a "user" shader.
-----------------------------------------------------------------------------*/
void nglReleaseShader(nglShader *shader)
{
	shader->Opcode->Release();
	shader->Handle = 0;
}

/*-----------------------------------------------------------------------------
Description: Override the default section rendering code.
             If RenderSectionFunc == NULL, then the default NGL renderer will
             be used.
-----------------------------------------------------------------------------*/
void nglSetSectionRenderer(nglMeshSection *Section, void (*RenderSectionFunc) (void *Data), nglShader *VS)
{
	if (RenderSectionFunc)
	{
		Section->RenderSectionFunc = RenderSectionFunc;
		Section->VSInitHandle = VS->Handle;
	}
	else
	{
		Section->RenderSectionFunc = nglRenderSection;
		Section->VSInitHandle = 0;
	}
}

/*-----------------------------------------------------------------------------
Description: Override the default quad rendering code.
             If RenderQuadFunc == NULL, then the default NGL renderer will
             be used.
-----------------------------------------------------------------------------*/
void nglSetQuadRenderer(nglQuad *Quad, void (*RenderQuadFunc) (void *Data), nglShader *VS)
{
	if (RenderQuadFunc)
	{
		Quad->RenderQuadFunc = RenderQuadFunc;
		Quad->VSHandle = VS->Handle;
	}
	else
	{
		Quad->RenderQuadFunc = nglRenderSingleQuad;
		Quad->VSHandle = nglVSQuad.Handle;
	}
}

/*-----------------------------------------------------------------------------
Description: Flush the blending mode cache system.
-----------------------------------------------------------------------------*/
void nglFlushBlendingMode()
{
	nglPrevBM = NGLBM_INVALID;
}

/*-----------------------------------------------------------------------------
Description: Flush the backface culling cache system.
-----------------------------------------------------------------------------*/
void nglFlushBackFaceCulling()
{
	nglPrevBackFaceCull = -1;
}

/*-----------------------------------------------------------------------------
Description: Flush the vertex shader cache system.
-----------------------------------------------------------------------------*/
void nglFlushVertexShader()
{
	nglPrevVS = 0;
}

/*-----------------------------------------------------------------------------
Description: Flush the pixel shader cache system.
-----------------------------------------------------------------------------*/
void nglFlushPixelShader()
{
	nglPrevVS = 0;
}

/*-----------------------------------------------------------------------------
Description: Add a custom node to the render list.
-----------------------------------------------------------------------------*/
void nglListAddCustomNode(nglCustomNodeFn CustomNodeFn, void *Data, nglSortInfo *SortInfo)
{
	nglListAddNode(NGLNODE_CUSTOM, CustomNodeFn, Data, SortInfo);
}

/*---------------------------------------------------------------------------------------------------------
  Texture API.



---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Load a texture from a file (XPR, DDS, TGA, IFL).
             If the texture has been previously loaded, its reference count
             will be increased by 1 unit and a pointer to the previously
             loaded texture will be returned.
-----------------------------------------------------------------------------*/
nglTexture *nglLoadTexture(const nglFixedString &FileName, bool ForceCubeMap)
{
	if (!FileName.c_str()[0])
	{
		nglWarning("Empty filename passed to nglLoadTexture() !\n");
		return 0;
	}

	bool Result = false;

	// Search for an existing copy.
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglTextureBank.Search(FileName)))
	{
		Inst->RefCount++;
		return (nglTexture *) Inst->Value;
	}

	nglTexture *Tex = (nglTexture *) nglMemAllocInternal(sizeof(nglTexture));
	memset(Tex, 0, sizeof(nglTexture));

	Tex->FileName = FileName;
	Tex->File = NULL;

	nglFileBuf File;
	bool LoadedXPR = false;

	char nglWork[NGL_MAX_PATH_LENGTH];
	strcpy(nglWork, nglTexturePath);
	strcat(nglWork, FileName.c_str());

	char FullFileName[NGL_MAX_PATH_LENGTH];
	strcpy(FullFileName, nglWork);
	strcat(FullFileName, ".xpr");
	if (nglReadFile(FullFileName, &File, D3DTEXTURE_ALIGNMENT))
	{
		Result = nglLoadTextureXPR(Tex, File.Buf);
		Tex->File = &File;
		LoadedXPR = true;
	}
	else
	{
		strcpy(FullFileName, nglWork);
		strcat(FullFileName, ".dds");
		if (nglReadFile(FullFileName, &File))
		{
			Result = nglLoadTextureDDS(Tex, File.Buf, ForceCubeMap);
			nglReleaseFile(&File);
		}
		else
		{
			strcpy(FullFileName, nglWork);
			strcat(FullFileName, ".tga");
			if (nglReadFile(FullFileName, &File))
			{
				Result = nglLoadTextureTGA(Tex, File.Buf, File.Size);
				nglReleaseFile(&File);
			}
			else
			{
				strcpy(FullFileName, nglWork);
				strcat(FullFileName, ".ifl");
				if (nglReadFile(FullFileName, &File))
				{
					Result = nglLoadTextureIFL(Tex, File.Buf, File.Size);
					nglReleaseFile(&File);
				}
			}
		}
	}

	// No apropriate format found or loading error.
	if (!Result)
	{
		nglError("nglLoadTexture(): Cannot load %s%s !\n", nglTexturePath, FileName.c_str());
		if (LoadedXPR)
			nglReleaseFile(&File);
		nglMemFreeInternal(Tex);
		return NULL;
	}

	Tex->Format = NGLTF_SWIZZLED;

	nglTextureBank.Insert(Tex->FileName, Tex);

	return Tex;
}

/*-----------------------------------------------------------------------------
Description: Load a texture from a file (DDS, TGA, IFL).
             If the texture has been previously loaded, its reference count
             will be increased by 1 unit and a pointer to the previously
             loaded texture will be returned.
-----------------------------------------------------------------------------*/
nglTexture *nglLoadTextureA(const char *FileName)
{
	if (!FileName[0])
	{
		nglWarning("Empty filename passed to nglLoadTextureA() !\n");
		return 0;
	}

	nglFixedString s(FileName);
	return nglLoadTexture(s);
}

/*-----------------------------------------------------------------------------
Description: Return a pointer to the front buffer texture.
-----------------------------------------------------------------------------*/
nglTexture *nglGetFrontBufferTex()
{
	// Decrement its ref count by 1, because it gets incremented by the GetBackBuffer() call.
	nglFrontBufferTex.DXSurface->Release();
	NGL_D3DTRY(nglDev->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &nglFrontBufferTex.DXSurface));

	return &nglFrontBufferTex;
}

/*-----------------------------------------------------------------------------
Description: Return a pointer to the current back buffer texture.
-----------------------------------------------------------------------------*/
nglTexture *nglGetBackBufferTex()
{
	// Decrement its ref count by 1, because it gets incremented by the GetBackBuffer() call.
	nglBackBufferTex.DXSurface->Release();
	NGL_D3DTRY(nglDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &nglBackBufferTex.DXSurface));

	return &nglBackBufferTex;
}

/*-----------------------------------------------------------------------------
Description: Return a pointer to the specified texture.
-----------------------------------------------------------------------------*/
nglTexture *nglGetTexture(const nglFixedString &FileName)
{
	if (!FileName.c_str()[0])
	{
		nglWarning("Empty filename passed to nglGetTexture() !\n");
		return 0;
	}

	nglInstanceBank::Instance *Inst;

	if ((Inst = nglTextureBank.Search(FileName)))
		return (nglTexture *) Inst->Value;

	return NULL;
}

/*-----------------------------------------------------------------------------
Description: Return a pointer to the specified texture.
-----------------------------------------------------------------------------*/
nglTexture *nglGetTextureA(const char *FileName)
{
	nglFixedString s(FileName);
	return nglGetTexture(s);
}

/*-----------------------------------------------------------------------------
Description: Add a reference to a texture.
-----------------------------------------------------------------------------*/
void nglAddTextureRef(nglTexture *Tex)
{
	// Search for an existing copy.
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglTextureBank.Search(Tex->FileName)))
		Inst->RefCount++;
}

/*-----------------------------------------------------------------------------
Description: Load a texture from a memory address (DDS, TGA, IFL).
             If the texture has been previously loaded, its reference count
             will be increased by 1 unit and a pointer to the previously
             loaded texture will be returned.
-----------------------------------------------------------------------------*/
nglTexture *nglLoadTextureInPlace(const nglFixedString &FileName, uint32 Type, void *Data, uint32 Size)
{
	if (!FileName.c_str()[0])
	{
		nglWarning("Empty filename passed to nglLoadTextureInPlace() !\n");
		return 0;
	}

	bool Result = false;

	nglInstanceBank::Instance *Inst;
	if ((Inst = nglTextureBank.Search(FileName)))
	{
		Inst->RefCount++;
		return (nglTexture *) Inst->Value;
	}

	nglTexture *Tex = (nglTexture *) nglMemAllocInternal(sizeof(nglTexture));
	memset(Tex, 0, sizeof(nglTexture));

	Tex->FileName = FileName;

	switch (Type)
	{
		case NGLTEX_DDSCUBE:
			Result = nglLoadTextureDDS(Tex, (uint8 *) Data, false);
			break;
		case NGLTEX_XPRCUBE:
		case NGLTEX_XPR:
			Result = nglLoadTextureXPR(Tex, (uint8 *) Data);
			break;
		case NGLTEX_DDS:
			Result = nglLoadTextureDDS(Tex, (uint8 *) Data, false);
			break;
		case NGLTEX_TGA:
			Result = nglLoadTextureTGA(Tex, (uint8 *) Data, Size);
			break;
		case NGLTEX_IFL:
			Result = nglLoadTextureIFL(Tex, (uint8 *) Data, Size);
			break;
	}

	if (!Result)
	{
		nglError("nglLoadTextureInPlace(): Unable to open %s%s !\n", nglTexturePath, FileName.c_str());
		nglMemFreeInternal(Tex);
		return NULL;
	}

	Tex->Format = NGLTF_SWIZZLED;

	nglTextureBank.Insert(Tex->FileName, Tex);

	return Tex;
}

/*-----------------------------------------------------------------------------
Description: Release a texture from the memory.
             The texture is only released when its ref counter reaches 0,
             otherwise its counter is decreased by 1 unit.
-----------------------------------------------------------------------------*/
void nglReleaseTexture(nglTexture *Tex)
{
	NGL_ASSERT(Tex, "Trying to release a NULL texture !");

	// Avoid releasing NGL's statically allocated resources.
	if (Tex->Static)
		return;

	// Remove a reference from the instance bank, delete the texture only if the count hits 0.
	if (nglTextureBank.Delete(Tex->FileName))
		return;

	nglDestroyTexture(Tex);
}

/*-----------------------------------------------------------------------------
Description: Destroy a texture (deallocate it).
             This should only be used for textures from nglCreateTexture or
             internally from the nglReleaseTexture call.
-----------------------------------------------------------------------------*/
void nglDestroyTexture(nglTexture *Tex)
{
	NGL_ASSERT(Tex, "Trying to destroy a NULL texture !");

	// For animated textures, release all the frames.
	if (Tex->Type == NGLTEX_IFL)
	{
		for (uint32 i = 0; i < Tex->NFrames; i++)
			nglReleaseTexture(Tex->Frames[i]);
		nglMemFreeInternal(Tex->Frames);
	}
	else
	{
		// Force the texture resource to be unused.
		nglDev->SetTexture(0, NULL);
		nglDev->SetTexture(1, NULL);
		nglDev->SetTexture(2, NULL);
		nglDev->SetTexture(3, NULL);

		// Simple and Cube textures have the same address.
		Tex->DXTexture.Simple->Release();
		Tex->DXTexture.Simple = NULL;

		if (Tex->Type == NGLTEX_XPR || Tex->Type == NGLTEX_XPRCUBE)
		{
			nglReleaseFile(Tex->File);
		}

		if (Tex->DXSurface)
		{
			Tex->DXSurface->Release();
			Tex->DXSurface = NULL;
		}
	}

	nglMemFreeInternal(Tex);
}

/*-----------------------------------------------------------------------------
Description: Saves a BMP file containing the texture.
             If no filename is given, it uses savetexXX.bmp.
-----------------------------------------------------------------------------*/
void nglSaveTexture(nglTexture *Tex, const char *FileName)
{
	NGL_ASSERT(Tex, "Cannot save an NULL texture !");
	NGL_ASSERT(Tex->DXSurface, "Cannot save the texture, it has no surface ! (cubic texture ?)");

	if (!FileName[0])
	{
		nglWarning("Empty filename passed to nglSaveTexture(). Save Failed !\n");
		return;
	}

	char s[1024];
	static uint32 SaveCount = 0;

	if (FileName)
		sprintf(s, "D:\\%s.bmp", FileName);
	else
		sprintf(s, "D:\\savetex%4.4d.bmp", SaveCount++);

	if (XGWriteSurfaceToFile(Tex->DXSurface, s) == S_OK)
	{
#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglInfo("Saved texture %s to %s.\n", Tex->FileName.c_str(), s);
#endif
	}
	else
	{
		nglError("nglSaveTexture() failed, can't save texture %s to %s. .\n", Tex->FileName.c_str(), s);
	}
}

/*-----------------------------------------------------------------------------
Description: Force all of the textures to be released.
-----------------------------------------------------------------------------*/
void nglReleaseAllTextures()
{
	nglInstanceBank::Instance *Inst = nglTextureBank.Head->Forward[0];

	while (Inst != nglTextureBank.NIL)
	{
		if (((nglTexture *) Inst->Value)->Static)
			Inst = Inst->Forward[0];
		else
		{
			Inst->RefCount = 1;
			nglReleaseTexture((nglTexture *) Inst->Value);
			Inst = nglTextureBank.Head->Forward[0];
		}
	}
}

/*-----------------------------------------------------------------------------
Description: Lock a texture for read/write purpose. Always lock the first
             mipmap (level 0).
NOTES:       * Only lock the mipmap of level 0 (biggest);
             * The cubemap textures cannot be locked;
             * For swizzled texture, use nglGetTexel() to read a texel.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Here is an example of code which writes to a LINEAR texture:

  int32 TexWidth  = 320;
  int32 TexHeight = 240;
  nglTexture* T = nglCreateTexture(NGLTF_LINEAR | NGLTF_ARGB8888,
                                   TexWidth, TexHeight);
  nglLockTextureXB(T);
  for (int32 j = 0; j < TexHeight; j++)
  {
    uint8* Row = (uint8*)((uint32)T->Data + T->DataPitch * j);
    for (int32 i = 0; i < TexWidth; i++)
    {
      uint32* pix = (uint32*)((uint32)Row + (i << 2));
      *pix = 0xffff0000;  // format is: ARGB
    }
  }
  nglUnlockTextureXB(T);
-----------------------------------------------------------------------------*/

Swizzler nglSwizzler;

bool nglLockTextureXB(nglTexture *Tex)
{
	NGL_ASSERT(Tex, "Cannot lock a NULL texture !");

	NGL_ASSERT(Tex->Type != NGLTEX_DDSCUBE, "Cube texture cannot be locked (for now) !");

	D3DLOCKED_RECT Format;
	if (Tex->DXTexture.Simple->LockRect(0, &Format, NULL, 0) == D3D_OK)
	{
		Tex->Data = (uint32 *) Format.pBits;
		Tex->DataPitch = Format.Pitch;

		if (Tex->Format & NGLTF_SWIZZLED)
		{
			// Usually textures on XBox are 'swizzled' aka pixels are reordered according
			// cache memory optimizations. Thus we cannot directly read them using the classic
			// way. A swizzler allow to directly read or write pixels to the surface.
			nglSwizzler.Init(Tex->Width, Tex->Height, 0);
		}
		return true;
	}

	return false;
}

/*-----------------------------------------------------------------------------
Description: This function does nothing. Just present for compatibility reason.
             The XBox hardware doesn't need to unlock the texture.
-----------------------------------------------------------------------------*/
bool nglUnlockTextureXB(nglTexture *Tex)
{
	return true;
}

/*-----------------------------------------------------------------------------
Description: Get a texel from a texture. On the XBox, the texture must be
             locked before reading from it.
             Thus before and after calling this function (could be several
             times) you should call nglLockTextureXB() and nglUnlockTextureXB().
-----------------------------------------------------------------------------*/
uint32 nglGetTexel(nglTexture *Tex, int32 x, int32 y)
{
	NGL_ASSERT(Tex, "Tex is NULL !");

	// Accessing the swizzled texels... Very slow !
	nglSwizzler.SetU(0);
	nglSwizzler.SetV(0);
	SWIZNUM swx = nglSwizzler.SwizzleU(x);
	SWIZNUM swy = nglSwizzler.SwizzleV(y);
	nglSwizzler.AddU(swx);
	nglSwizzler.AddV(swy);
	return ((uint32 *) Tex->Data)[nglSwizzler.Get2D()];
}

/*-----------------------------------------------------------------------------
Description: Bind a texture to the specified stage.
-----------------------------------------------------------------------------*/
void nglBindTexture(nglTexture *Tex, uint32 MaterialFlags, uint32 stage)
{
	if (!Tex)
	{
		nglDev->SetTexture(stage, NULL);
		return;
	}

	nglTexture *T = Tex;
	if (Tex->Type == NGLTEX_IFL)	// Animated textures support (IFL).
		T = Tex->Frames[nglTextureAnimFrame % Tex->NFrames];

	// Simple and Cube textures have the same address.
	nglDev->SetTexture(stage, (IDirect3DTexture8 *) T->DXTexture.Simple);

#ifdef PROJECT_NHL2K
	// This is provided to keep NHL working...
	if (MaterialFlags & NGLMAT_TRILINEAR_FILTER)
	{
		// Anisotropic filtering.
		nglDev->SetTextureStageState(stage, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);
		nglDev->SetTextureStageState(stage, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);
	}
	else
	{
		// trilinear filtering.
		nglDev->SetTextureStageState(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		nglDev->SetTextureStageState(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	}
#else
	// Set the filtering type.
	if (MaterialFlags & NGLMAT_BILINEAR_FILTER)
	{
		// Bilinear filtering.
		nglDev->SetTextureStageState(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		nglDev->SetTextureStageState(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	}
	else
	{
		// Point filtering.
		nglDev->SetTextureStageState(stage, D3DTSS_MINFILTER, D3DTEXF_POINT);
		nglDev->SetTextureStageState(stage, D3DTSS_MAGFILTER, D3DTEXF_POINT);
	}
#endif

	// Use the best mipmap filtering.
	nglDev->SetTextureStageState(stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
}

/*-----------------------------------------------------------------------------
Description: Load a IFL texture file.
-----------------------------------------------------------------------------*/
bool nglLoadTextureIFL(nglTexture *Tex, uint8 *Data, uint32 Size)
{
	NGL_ASSERT(Tex, "Cannot load a NULL texture !");

	static bool InLoadTextureIFL = false;

	// Temporary buffer for building the texture array.
	char FileName[NGL_MAX_PATH_LENGTH];
	static nglTexture *Textures[256];
	int32 NTextures = 0;

	if (InLoadTextureIFL)
	{
		nglWarning("Recursive IFL detected !\n");
		return false;
	}
	InLoadTextureIFL = true;

// handy parser macro
#define ENDOFBUF(a) ((a) - (char*)Data >= (int32)Size)

	char *Pos = (char *) Data;
	while (!ENDOFBUF(Pos))
	{
		// parse out the filename (no extension).
		char *Line = Pos;
		while (!ENDOFBUF(Line) && !isspace(*Line))
			Line++;
		uint32 Count = nglMinI(Line - Pos, 255);
		if (!Count)
			break;
		strncpy(FileName, Pos, Count);
		FileName[Count] = 0;
		char *Ext = strchr(FileName, '.');
		if (Ext)
			*Ext = 0;

		// attempt to load the texture.
		Textures[NTextures] = nglLoadTextureA(FileName);
		if (Textures[NTextures])
			NTextures++;

		// skip any whitespace
		Pos = Line;
		while (!ENDOFBUF(Pos) && isspace(*Pos))
			Pos++;
	}

#undef ENDOFBUF

	InLoadTextureIFL = false;

	if (NTextures)
	{
		Tex->Type = NGLTEX_IFL;
		Tex->NFrames = NTextures;

		Tex->Frames = (nglTexture **) nglMemAllocInternal(sizeof(nglTexture *) * NTextures);
		memcpy(Tex->Frames, Textures, sizeof(nglTexture *) * NTextures);

		// Steal our 'width and height' from the first frame.
		Tex->Width = Tex->Frames[0]->Width;
		Tex->Height = Tex->Frames[0]->Height;
		return true;
	}

	return false;
}

/*-----------------------------------------------------------------------------
Description: Load a TGA texture file.
-----------------------------------------------------------------------------*/
bool nglLoadTextureTGA(nglTexture *Tex, uint8 *Data, uint32 Size)
{
	NGL_ASSERT(Tex, "Cannot load a NULL texture !");

	NGL_D3DTRY(D3DXCreateTextureFromFileInMemory(nglDev, Data, Size, &Tex->DXTexture.Simple));

	Tex->Type = NGLTEX_TGA;
	Tex->MipmapLevels = 0;		// TGA format doesn't support mipmap.
	D3DSURFACE_DESC info;
	Tex->DXTexture.Simple->GetLevelDesc(0, &info);
	Tex->Width = info.Width;
	Tex->Height = info.Height;
	NGL_D3DTRY(Tex->DXTexture.Simple->GetSurfaceLevel(0, &Tex->DXSurface));

#if NGL_DEBUG
	// Check if the texture's dimensions are a power of 2.
	if (!(nglPowerOf2(Tex->Width) && nglPowerOf2(Tex->Height)))
		nglError("Trying to load a non-power of 2 texture !\n");
#endif

	return true;
}

/*-----------------------------------------------------------------------------
Description: Load a DDS texture file.
-----------------------------------------------------------------------------*/
bool nglLoadTextureDDS(nglTexture *Tex, uint8 *Data, bool ForceCubeMap)
{
	NGL_ASSERT(Tex, "Cannot load a NULL texture !");

	// Check for DDS magic number.
	if (*(uint32 *) Data != MAKEFOURCC('D', 'D', 'S', ' '))
		return false;

	// Check if it's a cube map or not.
	DDS_HEADER *hdr = (DDS_HEADER *) (Data + 4);

	uint32 CubeMap = hdr->dwCubemapFlags & DDS_CUBEMAP_ALLFACES;
	if (CubeMap && (hdr->dwCubemapFlags != DDS_CUBEMAP_ALLFACES))
		return false;			// Invalid cubemap file (DX8): not all the 6 faces are present !

	D3DSURFACE_DESC info;

	// Cubemap DDS.
	if (CubeMap)
	{
		if (!nglLoadDDS
			((IDirect3DBaseTexture8 **) &Tex->DXTexture.Cube, Data))
		{
			nglError("nglLoadDDS(): can't load texture \"%s\" (unsupported format ?) !\n", Tex->FileName.c_str());
			return false;
		}
		Tex->Type = NGLTEX_DDSCUBE;
		Tex->MipmapLevels = Tex->DXTexture.Cube->GetLevelCount();
		// Don't retrieve the surfaces when using cubic textures.
		return true;
	}

	// "Simple" texture DDS.
	else
	{
		// Force the texture to be cubic. We copy the basic map into the 6 faces of the cubic texture.
		// FIXME: TO BE REMOVED !
		// This is just a temporary solution until the artists finish the cubic enviro map generation.
		// This code will be removed pretty soon.
		if (ForceCubeMap)
		{
			IDirect3DTexture8 *SimpleTexture;
			if (!nglLoadDDS((IDirect3DBaseTexture8 **) &SimpleTexture, Data))
			{
				nglError("nglLoadDDS(): can't load texture \"%s\" (unsupported format ?) !\n", Tex->FileName.c_str());
				return false;
			}
			// Get width and height from mipmap level 0.
			SimpleTexture->GetLevelDesc(0, &info);
			Tex->Width = info.Width;
			Tex->Height = info.Height;
			Tex->Type = NGLTEX_DDSCUBE;
			Tex->MipmapLevels = 1;
			// Create the cubic texture (1 mipmap level only).
			NGL_D3DTRY(nglDev->CreateCubeTexture(Tex->Width, 1, 0, info.Format, 0, &Tex->DXTexture.Cube));
			D3DLOCKED_RECT RectCube, Rect;
			D3DCUBEMAP_FACES CubeFaces[6] = {
				D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
				D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
				D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z
			};
			NGL_D3DTRY(SimpleTexture->LockRect(0, &Rect, NULL, 0));	// Lock the "simple" texture.
			// Lock each cube face and copy the simple (basic) texture into the 6 faces.
			for (int32 i = 0; i < 6; i++)
			{
				NGL_D3DTRY(Tex->DXTexture.Cube->LockRect(CubeFaces[i], 0, &RectCube, NULL, 0));
				memcpy(RectCube.pBits, Rect.pBits, info.Size);
			}
			SimpleTexture->Release();
		}

		else
		{
			if (!nglLoadDDS((IDirect3DBaseTexture8 **) &Tex->DXTexture.Simple, Data))
			{
				nglError("nglLoadDDS(): can't load texture \"%s\" (unsupported format ?) !\n", Tex->FileName.c_str());
				return false;
			}
			Tex->Type = NGLTEX_DDS;
			Tex->MipmapLevels = Tex->DXTexture.Simple->GetLevelCount();
			// Get width and height from mipmap level 0 !
			Tex->DXTexture.Simple->GetLevelDesc(0, &info);
			Tex->Width = info.Width;
			Tex->Height = info.Height;
			NGL_D3DTRY(Tex->DXTexture.Simple->GetSurfaceLevel(0, &Tex->DXSurface));
		}
	}

#if NGL_DEBUG
	// Check if the texture's dimensions are a power of 2.
	if (!(nglPowerOf2(Tex->Width) && nglPowerOf2(Tex->Height)))
		nglError("Trying to load a non-power of 2 texture !\n");
#endif

	return true;
}


/*-----------------------------------------------------------------------------
Description: Load a XPR texture file.
-----------------------------------------------------------------------------*/
bool nglLoadTextureXPR(nglTexture *Tex, uint8 *Data)
{
	NGL_ASSERT(Tex, "Cannot load a NULL texture !");

	XPR_HEADER *XprHdr = (XPR_HEADER *) Data;

	// Check for XPR magic number.
	if (XprHdr->dwMagic != XPR_MAGIC_VALUE)
		return false;

	DWORD *TexHdr = (DWORD *)(Data + sizeof(XPR_HEADER));

	uint32 CubeMap = (*(TexHdr+3) & D3DFORMAT_CUBEMAP);

	// Cubemap XPR.
	if (CubeMap)
	{
		Tex->DXTexture.Cube = (D3DCubeTexture *)TexHdr;
		Tex->DXTexture.Cube->Register(Data + XprHdr->dwHeaderSize);
		Tex->Type = NGLTEX_XPRCUBE;
		Tex->MipmapLevels = Tex->DXTexture.Simple->GetLevelCount();		
		
		return true;
	}
	// "Simple" texture XPR.
	else
	{
		D3DSURFACE_DESC info;
		
		Tex->DXTexture.Simple = (D3DTexture *)TexHdr;
		Tex->DXTexture.Simple->Register(Data + XprHdr->dwHeaderSize);
		Tex->Type = NGLTEX_XPR;
		Tex->MipmapLevels = Tex->DXTexture.Simple->GetLevelCount();		

		// Get width and height from mipmap level 0 !
		Tex->DXTexture.Simple->GetLevelDesc(0, &info);
		Tex->Width = info.Width;
		Tex->Height = info.Height;
	}

#if NGL_DEBUG
	// Check if the texture's dimensions are a power of 2.
	if (!(nglPowerOf2(Tex->Width) && nglPowerOf2(Tex->Height)))
		nglError("Trying to load a non-power of 2 texture !\n");
#endif

	return true;
}

/*-----------------------------------------------------------------------------
Description: Create a texture.
             The client must specify if he wants to create a linear or swizzled
             texture.
             Linear/swizzled textures have the following differences:
             - Rendering TO a linear texture is faster than rendering to a
               swizzled.
             - Rendering FROM a swizzled texture is faster than rendering from
               a linear.
             - Swizzled format must have width and height to be a power of 2.
             - Swizzled render target must have the viewport size equal to the
               texture size.
             - Swizzled textures use coordinates that range [0,1].
             - Linear textures use UV range [0, width or height], so it
               REQUIRES UV scaling.
             - The hardware requires CLAMP for linear textures.
             - Linear format does not support mipmaps (mipmap level must be 1).
             - Linear format does not support cube maps nor volume.
             - Generally speaking, it's faster to use linear textures for
               shadows rendering.
IMPORTANT:   Because of the current NGL implementation, try to render to linear
             textures, because clearing the zbuffer when rendering to a
             swizzled texture is sloooooow.
-----------------------------------------------------------------------------*/
nglTexture *nglCreateTexture(uint32 Format, uint32 Width, uint32 Height)
{
	NGL_ASSERT(Width <= NGL_DEPTHBUFFER_WIDTH, "nglCreateTexture(): too big width !");
	NGL_ASSERT(Height <= NGL_DEPTHBUFFER_HEIGHT, "nglCreateTexture(): too big height !");

	nglTexture *Tex = (nglTexture *) nglMemAllocInternal(sizeof(nglTexture));
	memset(Tex, 0, sizeof(nglTexture));

	nglFixedString s("CREATED TEXTURE");
	Tex->FileName = s;

	Tex->Type = NGLTEX_TGA;

	Tex->Width = Width;
	Tex->Height = Height;
	Tex->Format = Format;

	// Create a D3D 32-bits ARGB texture.
	IDirect3DTexture8 *D3DTex = NULL;

	// The user has to specify if the texture to be created is swizzled or linear.
	NGL_ASSERT(Format & NGLTF_SWIZZLED || Format & NGLTF_LINEAR,
			   "nglCreateTexture: you must specify if the texture to be created is swizzled or linear !");

	// Swizzled texture.
	if (Format & NGLTF_SWIZZLED)
	{
		Tex->Format |= NGLTF_SWIZZLED;

		// Check if the texture's dimensions are a power of 2.
		NGL_ASSERT(nglPowerOf2(Width) && nglPowerOf2(Height),
				   "nglCreateTexture: trying to create a non-power of 2 SWIZZLED texture !");

		D3DFORMAT format = D3DFMT_UNKNOWN;

		switch (Format & 0xF00)
		{
			case NGLTF_ARGB8888:
				format = D3DFMT_A8R8G8B8;
				break;
			case NGLTF_ARGB4444:
				format = D3DFMT_A4R4G4B4;
				break;
			case NGLTF_ARGB1555:
				format = D3DFMT_A1R5G5B5;
				break;
			case NGLTF_RGB888:
				format = D3DFMT_X8R8G8B8;
				break;
			case NGLTF_RGB565:
				format = D3DFMT_R5G6B5;
				break;
			case NGLTF_RGB655:
				format = D3DFMT_R6G5B5;
				break;
			case NGLTF_YUY2:
				NGL_ASSERT(false, "nglCreateTexture: NGLTF_YUY2 must be linear (specify NGLTF_LINEAR instead of NGLTF_SWIZZLED) !");
				break;
			default:
				NGL_ASSERT(false, "nglCreateTexture: you must specify a depth format !");
		}

		// Create a swizzled texture.
		NGL_D3DTRY(nglDev->CreateTexture(Width, Height, 1, 0, format, 0, &D3DTex));
	}

	// Linear texture.
	else if (Format & NGLTF_LINEAR)
	{
		Tex->Format |= NGLTF_LINEAR;

		D3DFORMAT format = D3DFMT_UNKNOWN;

		switch (Format & 0xF00)
		{
			case NGLTF_ARGB8888:
				format = D3DFMT_LIN_A8R8G8B8;
				break;
			case NGLTF_ARGB4444:
				format = D3DFMT_LIN_A4R4G4B4;
				break;
			case NGLTF_ARGB1555:
				format = D3DFMT_LIN_A1R5G5B5;
				break;
			case NGLTF_RGB888:
				format = D3DFMT_LIN_X8R8G8B8;
				break;
			case NGLTF_RGB565:
				format = D3DFMT_LIN_R5G6B5;
				break;
			case NGLTF_RGB655:
				format = D3DFMT_LIN_R6G5B5;
				break;
			case NGLTF_YUY2:
				format = D3DFMT_YUY2;
				break;
			default:
				NGL_ASSERT(false, "nglCreateTexture: you must specify a depth format !");
		}

		NGL_D3DTRY(nglDev-> CreateTexture(Width, Height, 1, 0, format, 0, &D3DTex));
	}

	Tex->DXTexture.Simple = D3DTex;

	// Initialize the D3D surface using the level 0 of the D3D texture.
	NGL_D3DTRY(Tex->DXTexture.Simple->GetSurfaceLevel(0, &Tex->DXSurface));

	return Tex;
}


/*---------------------------------------------------------------------------------------------------------
  Mesh API.



---------------------------------------------------------------------------------------------------------*/

#define PTR_OFFSET( Ptr, Type ) { if ( Ptr ) Ptr = (Type)( (uint32)Ptr + (uint32)Base ); }

/*-----------------------------------------------------------------------------
Description: Rebase the vertex buffer pointers.
-----------------------------------------------------------------------------*/
void nglRebaseHeader(uint32 Base, nglMeshFileHeader *pHeader)
{
	PTR_OFFSET(pHeader->SkinVertexBuffers, nglVertexBuffer *);
	PTR_OFFSET(pHeader->VertexBuffers, nglVertexBuffer *);
}

/*-----------------------------------------------------------------------------
Description: Rebase the vertex/index buffer data (vertices and indices).
-----------------------------------------------------------------------------*/
void nglRebaseVertexBuffer(uint32 Base, nglVertexBuffer *pVertexBuffer)
{
	PTR_OFFSET(pVertexBuffer->IndexBufferData, uint16 *);
	PTR_OFFSET(pVertexBuffer->VertexBufferData, void *);
}

#undef PTR_OFFSET

#define PTR_OFFSET( Ptr, Type ) { if ( Ptr ) Ptr = (Type)( (uint32)Ptr - (uint32)Base ); }

/*-----------------------------------------------------------------------------
Description: Rebase the vertex buffer pointers.
-----------------------------------------------------------------------------*/
void nglUnRebaseHeader(uint32 Base, nglMeshFileHeader *pHeader)
{
	PTR_OFFSET(pHeader->SkinVertexBuffers, nglVertexBuffer *);
	PTR_OFFSET(pHeader->VertexBuffers, nglVertexBuffer *);
}

/*-----------------------------------------------------------------------------
Description: Rebase the vertex/index buffer data (vertices and indices).
-----------------------------------------------------------------------------*/
void nglUnRebaseVertexBuffer(uint32 Base, nglVertexBuffer *pVertexBuffer)
{
	PTR_OFFSET(pVertexBuffer->IndexBufferData, uint16 *);
	PTR_OFFSET(pVertexBuffer->VertexBufferData, void *);
}

#undef PTR_OFFSET

/*-----------------------------------------------------------------------------
Description: Change all the internal pointers in a mesh structure to be based
             at a new location.  Useful for copying meshes around and for
             loading them from files (in which they're based at 0).
-----------------------------------------------------------------------------*/
void nglRebaseMesh(uint32 NewBase, uint32 OldBase, nglMesh *pMesh)
{
	NGL_ASSERT(pMesh, "Cannot rebase a NULL mesh !");

	if (pMesh->Flags & NGLMESH_PROCESSED)
		return;

#define PTR_OFFSET(Ptr, Type) { if (Ptr) Ptr = (Type)((uint32)Ptr + ((uint32)NewBase - (uint32)OldBase)); }

	PTR_OFFSET(pMesh->LODs, nglMeshLODInfo*);
	PTR_OFFSET(pMesh->Bones, nglMatrix*);
	PTR_OFFSET(pMesh->Sections, nglMeshSection*);

	for (uint32 i = 0; i < pMesh->NSections; i++)
	{
		nglMeshSection *pSection = &pMesh->Sections[i];

		PTR_OFFSET(pSection->VB, nglVertexBuffer*);
		PTR_OFFSET(pSection->Material, nglMaterial*);
		PTR_OFFSET(pSection->BonesIdx, int16*);
	}

#undef PTR_OFFSET
}

/*-----------------------------------------------------------------------------
Description: Change all the internal pointers in a mesh structure to be based
             at a new location.  Useful for copying meshes around and for
             loading them from files (in which they're based at 0).
-----------------------------------------------------------------------------*/
void nglUnRebaseMesh(uint32 NewBase, uint32 OldBase, nglMesh* pMesh)
{
	NGL_ASSERT(pMesh, "Cannot rebase a NULL mesh !");

	if (pMesh->Flags & NGLMESH_PROCESSED)
		return;

	#define PTR_OFFSET(Ptr, Type) { if (Ptr) Ptr = (Type)((uint32)Ptr +	((uint32)NewBase - (uint32)OldBase)); }

	PTR_OFFSET(pMesh->LODs, nglMeshLODInfo*);
	PTR_OFFSET(pMesh->Bones, nglMatrix*);

	for (uint32 i = 0; i < pMesh->NSections; i++)
	{
		nglMeshSection* pSection = &pMesh->Sections[i];

		if (pSection->Material->MapName)
			pSection->Material->Flags |= NGLMAT_TEXTURE_MAP;

		if (pSection->Material->LightMapName)
			pSection->Material->Flags |= NGLMAT_LIGHT_MAP;

		if (pSection->Material->DetailMapName)
			pSection->Material->Flags |= NGLMAT_DETAIL_MAP;

		if (pSection->Material->EnvironmentMapName)
			pSection->Material->Flags |= NGLMAT_ENVIRONMENT_MAP;

		PTR_OFFSET(pSection->VB, nglVertexBuffer*);
		PTR_OFFSET(pSection->Material, nglMaterial*);
		PTR_OFFSET(pSection->BonesIdx, int16*);
	}

	PTR_OFFSET( pMesh->Sections, nglMeshSection* );

	#undef PTR_OFFSET
}

/*-----------------------------------------------------------------------------
Description: Retrieve a mesh from a loaded file.  If the mesh can't be found it
             returns NULL and prints out a warning (which can be disabled by
             passing Warn = false).
-----------------------------------------------------------------------------*/
nglMesh *nglGetMeshA(const char *Name, bool Warn)
{
	nglFixedString p(Name);
	return nglGetMesh(p, Warn);
}

/*-----------------------------------------------------------------------------
Description: Retrieve a mesh from a loaded file.  If the mesh can't be found it
             returns NULL and prints out a warning (which can be disabled by
             passing Warn = false).
-----------------------------------------------------------------------------*/
nglMesh *nglGetMesh(const nglFixedString &Name, bool Warn)
{
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglMeshBank.Search(Name)))
		return (nglMesh *) Inst->Value;
	if (Warn)
		nglError("nglGetMesh(): Unable to find mesh %s !\n", Name.c_str());

	return NULL;
}

/*-----------------------------------------------------------------------------
Description: Return the first mesh of a "mesh collection" file.
-----------------------------------------------------------------------------*/
nglMesh *nglGetFirstMeshInFile(const nglFixedString &FileName)
{
	nglInstanceBank::Instance *Inst;
	Inst = nglMeshFileBank.Search(FileName);
	if (!Inst)
		return NULL;

	return ((nglMeshFile *) Inst->Value)->FirstMesh;
}

/*-----------------------------------------------------------------------------
Description: Return the next mesh of the "mesh collection".
-----------------------------------------------------------------------------*/
nglMesh *nglGetNextMeshInFile(nglMesh *Mesh)
{
	if (!Mesh)
		return NULL;

	return Mesh->NextMesh;
}

/*-----------------------------------------------------------------------------
Description: Register the vertex and indice buffers used by all the meshes of an
             xbmesh file. There are 2 types of them: for the non-skin meshes and
             for the skin ones. Each category can have several vertex buffers.
-----------------------------------------------------------------------------*/
bool nglRegisterMeshFileVBs(nglMeshFileHeader *Header)
{
	// If we have some non-skin meshes, register their vertex buffers.
	if (Header->NVertexBuffers)
	{
#if !NGL_FORCE_CREATE_VB
		for (uint32 i = 0; i < Header->NVertexBuffers; i++)
		{
			// Register the VertexBuffer.
			// Header->Vertex.VertexBufferData MUST be on a 4KB boundary !
			NGL_ASSERT(((uint32) Header->VertexBuffers[i].VertexBufferData & 4095) == 0,
					   "Mesh VertexBufferData isn't on a 4KB boundary !");
			Header->VertexBuffers[i].VertexBuffer.Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER;	// Initial ref count to 1.
			Header->VertexBuffers[i].VertexBuffer.Register(Header->VertexBuffers[i].VertexBufferData);
		}
#else
		LPDIRECT3DVERTEXBUFFER8 *VertBufs;

		// Allocate the array of VBs.
		VertBufs = (LPDIRECT3DVERTEXBUFFER8 *)nglMemAllocInternal(sizeof(LPDIRECT3DVERTEXBUFFER8) * Header->NVertexBuffers);

		// Create the vertex buffers.
		for (uint32 i = 0; i < Header->NVertexBuffers; i++)
		{
			uint32 sz =	Header->VertexBuffers[i].VertexCount * sizeof(nglVertexBasic);
			NGL_D3DTRY(nglDev->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY, 0, 0, &VertBufs[i]));

			// Fill in the vertex buffer.
			nglVertexBasic *DestVertices;

			if (VertBufs[i]->Lock(0, sz, (BYTE **) &DestVertices, 0) !=	D3D_OK)
				nglFatal("VertBufs[i]->Lock() failed !\n");

			nglVertexBasic *SrcVertices = (nglVertexBasic *) Header->VertexBuffers[i].VertexBufferData;
			memcpy(DestVertices, SrcVertices, sz);
			Header->VertexBuffers[i].VertexBufferData = (void *) VertBufs[i];
		}
#endif
	}

	// If we have some skin meshes, register their vertex buffers.
	if (Header->NSkinVertexBuffers)
	{
#if !NGL_FORCE_CREATE_VB
		for (uint32 i = 0; i < Header->NSkinVertexBuffers; i++)
		{
			// Register the VertexBuffer.
			// Header->Vertex.VertexBufferData MUST be on a 4KB boundary !
			NGL_ASSERT(((uint32) Header->SkinVertexBuffers[i].VertexBufferData & 4095) == 0,
					   "Mesh VertexBufferData isn't on a 4KB boundary !");
			Header->SkinVertexBuffers[i].VertexBuffer.Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER;	// Initial ref count to 1.
			Header->SkinVertexBuffers[i].VertexBuffer.Register(Header->SkinVertexBuffers[i].VertexBufferData);
		}
#else
		LPDIRECT3DVERTEXBUFFER8 *SkinVertBufs;

		// Allocate the array of VBs.
		SkinVertBufs = (LPDIRECT3DVERTEXBUFFER8 *)nglMemAllocInternal(sizeof(LPDIRECT3DVERTEXBUFFER8) * Header->NSkinVertexBuffers);

		// Create the vertex buffers.
		for (uint32 i = 0; i < Header->NSkinVertexBuffers; i++)
		{
			uint32 sz = Header->SkinVertexBuffers[i].VertexCount * sizeof(nglVertexSkin);
			NGL_D3DTRY(nglDev->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY, 0, 0, &SkinVertBufs[i]));

			// Fill in the vertex buffer.
			nglVertexSkin *DestVertices;

			if (SkinVertBufs[i]->Lock(0, sz, (BYTE **) &DestVertices, 0) !=	D3D_OK)
				nglFatal("VertBufs[i]->Lock() failed !\n");

			nglVertexSkin *SrcVertices = (nglVertexSkin *) Header->SkinVertexBuffers[i].VertexBufferData;
			memcpy(DestVertices, SrcVertices, sz);
			Header->SkinVertexBuffers[i].VertexBufferData = (void *) SkinVertBufs[i];
		}
#endif
	}

	return true;
}

/*-----------------------------------------------------------------------------
Description: Post processing of the mesh after loading: load maps, set sections,
             VB pointers, vertex shader keys, etc.
-----------------------------------------------------------------------------*/
void nglProcessMesh(nglMeshFileHeader *Header, nglMesh *Mesh)
{
	NGL_ASSERT(Header, "Header is NULL !");
	NGL_ASSERT(Mesh, "Mesh is NULL !");

	uint32 i;

	// If mesh already processed, return.
	if (Mesh->Flags & NGLMESH_PROCESSED)
		return;

	Mesh->Flags |= NGLMESH_PROCESSED;

	// Inverse the bones matrices.
	for (i = 0; i < Mesh->NBones; i++)
	{
		XGMatrixInverse((XGMATRIX *)&(Mesh->Bones[i]), NULL, (XGMATRIX *)&(Mesh->Bones[i]));
	}

	// Process sections and materials.
	for (i = 0; i < Mesh->NSections; i++)
	{
		nglMeshSection *Section = &Mesh->Sections[i];
		nglMaterial *Material = Section->Material;

		// Mesh files only contains triangle strips (at least for now...).
		Section->PrimType = NGLPRIM_TRISTRIP;

		// By default, no pushbuffer.
		Section->PB = 0;

		// NGL internal section rendering is set as default.
		Section->RenderSectionFunc = nglRenderSection;

		// Automatically load textures used by the mesh.

		// Diffuse1 map (aka texture map).
		if (Material->Flags & NGLMAT_TEXTURE_MAP)
		{
			Material->Map = nglLoadTexture(Material->MapName);
			if (!Material->Map)
				Material->Flags &= ~NGLMAT_TEXTURE_MAP;
		}
		else
			Material->Map = 0;

		// Diffuse2 map (aka lightmap).
		if (Material->Flags & NGLMAT_LIGHT_MAP)
		{
			Material->LightMap = nglLoadTexture(Material->LightMapName);
			if (!Material->LightMap)
				Material->Flags &= ~NGLMAT_LIGHT_MAP;
		}
		else
			Material->LightMap = 0;

		// Detail map.
		if (Material->Flags & NGLMAT_DETAIL_MAP)
		{
			Material->DetailMap = nglLoadTexture(Material->DetailMapName);
			if (!Material->DetailMap)
				Material->Flags &= ~NGLMAT_DETAIL_MAP;
		}
		else
			Material->DetailMap = 0;

		// Enviro map.
		if (Material->Flags & NGLMAT_ENVIRONMENT_MAP)
		{
#if NGL_USE_SPHERICAL_ENVMAP
			// Spiderman's artists think that spherical map gives better (!!!) result than cubic map...
			Material->EnvironmentMap = nglLoadTexture(Material->EnvironmentMapName);
#else
			// If the texture isn't a cubic texture we create a cubic texture where each face is
			// a copy of the basic texture.
			Material->EnvironmentMap = nglLoadTexture(Material->EnvironmentMapName, true);
#endif
			if (!Material->EnvironmentMap)
				Material->Flags &= ~NGLMAT_ENVIRONMENT_MAP;

		}
		else
			Material->EnvironmentMap = 0;

		// Calculate the vertex shader key per section.
		Section->PartialVSKey = (uint32) 0;

		// Skinned ?
		if (Mesh->NBones)
			Section->PartialVSKey |= NGL_VSFLAG_SKIN;

		// UV scroll ?
		if (Material->Flags & NGLMAT_UV_SCROLL)
			Section->PartialVSKey |= NGL_VSFLAG_SCROLLUV;


#ifdef PROJECT_SPIDERMAN
		// Material color ?
		// Some skin meshes have the material color flag set and it should not be the case.
		// I just remove it by hand in case the artists forgot to remove this flag for skin models.
		if ((Material->Flags & NGLMAT_MATERIAL_COLOR) && Mesh->NBones)
			Material->Flags &= ~NGLMAT_MATERIAL_COLOR;
#endif

		// Alpha falloff ?
		/*
		   // Not implemented.
		   if (Material->Flags & NGLMAT_ALPHA_FALLOFF)
		   Section->VSKey |= NGL_VSFLAG_ALPHAFALLOFF;
		 */

		// Texture stages configuration.
		nglSetShaderConfig(Section);

		// Force the material to have the alpha flag set properly.
		if (Material->MapBlendMode != NGLBM_OPAQUE
			&& Material->MapBlendMode != NGLBM_PUNCHTHROUGH)
			Material->Flags |= NGLMAT_ALPHA;
		else
			Material->Flags &= ~NGLMAT_ALPHA;

#ifdef PROJECT_SPIDERMAN
		// Force non-translucent (opaque and punchthrough) materials to be lit and
		// translucent ones to have their light flag removed.
		// This is a hack to have correct shadows in spiderman (this flag should be set by the artists).
		if (Material->MapBlendMode != NGLBM_OPAQUE
			&& Material->MapBlendMode != NGLBM_PUNCHTHROUGH)
			Material->Flags &= ~NGLMAT_LIGHT;
		else
			Material->Flags |= NGLMAT_LIGHT;
#endif

#if NGL_USE_PUSHBUFFER
		bool CpuCopy = Section->IndexCount > NGL_PUSHBUFFER_IDXLIMIT ? false : true;

		if (CpuCopy)
		{
			Section->PB = NULL;
		}
		else
		{
			// Setup the section's pushbuffer.
			DWORD PBSize;
			uint16 *Idx = (uint16 *) ((uint32) Section->VB->IndexBufferData + ((uint32) Section->IndexOffset << 1));

			// Calculate the size needed by the pushbuffer and allocate the contiguous memory for it.
			void *PBData = NULL;
			XGCompileDrawIndexedVertices(PBData, &PBSize, (D3DPRIMITIVETYPE)Section->PrimType, Section->IndexCount, Idx);
			PBData = (void *) nglMemAlloc(PBSize, D3DPUSHBUFFER_ALIGNMENT, true);

			// Fill the pushbuffer.
			XGCompileDrawIndexedVertices(PBData, &PBSize, (D3DPRIMITIVETYPE)Section->PrimType, Section->IndexCount, Idx);

			// Register the section's pushbuffer with the previously initialized data.
			uint32 sz = sizeof(IDirect3DPushBuffer8);
			Section->PB = (IDirect3DPushBuffer8 *) nglMemAllocInternal(sz);
			ZeroMemory(Section->PB, sz);
			Section->PB->Common = 1 | D3DCOMMON_TYPE_PUSHBUFFER;
			Section->PB->Size = PBSize;
			Section->PB->AllocationSize = PBSize;
			Section->PB->Register(PBData);
		}
#endif

	}  // end for each section.
}

/*-----------------------------------------------------------------------------
Description: Internal function to load a "mesh collection".
             Called by nglLoadMeshFile() and nglLoadMeshFileInPlace().
-----------------------------------------------------------------------------*/
bool nglLoadMeshFileInternal(const nglFixedString &FileName, nglMeshFile *MeshFile)
{
	NGL_ASSERT(MeshFile, "MeshFile is NULL !");

	// Parse the header.
	nglMeshFileHeader *Header = (nglMeshFileHeader *) MeshFile->FileBuf.Buf;
	if (strncmp((char *) (&Header->Tag), "XBXM", 4))
	{
		nglError("Corrupted mesh file: %s%s%s.\n", nglMeshPath, FileName.c_str(), NGL_MESHFILE_EXT);
		return false;
	}

	// Check the XBMESH version.
	uint32 i;
	for (i = 0; i < NGL_SUPPORTED_XBMESH_VERSIONS; i++)
	{
		if (Header->Version != nglSupportedXBMESH[i])
		{
			nglError("Unsupported mesh file version %s%s%s, mesh is 0x%x, this NGL version only supports ",
				 nglMeshPath, FileName.c_str(), NGL_MESHFILE_EXT,
				 Header->Version);

			for (int32 j = 0; j < 1; j++)
				nglPrintf("0x%x ", nglSupportedXBMESH[j]);

			nglPrintf("!\n");
			return false;
		}
	}

	if (!Header->NMeshes)
	{
		nglWarning("Mesh file contains no meshes: %s%s%s.\n", nglMeshPath, FileName.c_str(), NGL_MESHFILE_EXT);
		return false;
	}

	// Rebase the 2 header's vertex buffer pointers (skin/unskin).
	nglRebaseHeader((uint32) Header, Header);

	// Rebase the vertex/index data.
	for (i = 0; i < Header->NVertexBuffers; i++)
		nglRebaseVertexBuffer((uint32) Header, &Header->VertexBuffers[i]);

	for (i = 0; i < Header->NSkinVertexBuffers; i++)
		nglRebaseVertexBuffer((uint32) Header, &Header->SkinVertexBuffers[i]);

	// Register the vertex/index buffers.
	if (!nglRegisterMeshFileVBs(Header))
	{
		nglError("Cannot register vertex/index buffers !\n");
		return false;
	}

	// Read the meshes starting from the beginning of the file.
	uint8 *BufPos = (uint8 *) (Header + 1);	// Skip the file header.
	MeshFile->FirstMesh = (nglMesh *) BufPos;

	nglMesh *Mesh;
	nglMesh *LastMesh = NULL;
	for (uint32 i = 0; i < Header->NMeshes; i++)
	{
		Mesh = (nglMesh *) BufPos;

		// Search for an existing mesh with this name and skip it if we've already got one.
		nglInstanceBank::Instance *Inst;
		if ((Inst = nglMeshBank.Search(Mesh->Name)))
		{
			nglMesh *ExistingMesh = (nglMesh *) Inst->Value;
			nglWarning("Skipping duplicate mesh %s found in %s%s%s.  Originally contained in %s%s%s.\n",
				 Mesh->Name.c_str(), nglMeshPath, FileName.c_str(),
				 NGL_MESHFILE_EXT, ExistingMesh->File->FilePath,
				 ExistingMesh->File->FileName.c_str(), NGL_MESHFILE_EXT);
			BufPos += Mesh->DataSize;
			continue;
		}

		Mesh->File = MeshFile;

		// Convert pointers from file offsets.
		nglRebaseMesh((uint32) Header, 0, Mesh);

		// Load textures, process materials, etc.
		nglProcessMesh(Header, Mesh);

		// Link it into the list of meshes for the mesh file.
		if (LastMesh)
			LastMesh->NextMesh = Mesh;

		LastMesh = Mesh;

		nglMeshBank.Insert(Mesh->Name, Mesh);

		BufPos += Mesh->DataSize;
	}

	if (LastMesh)
		LastMesh->NextMesh = NULL;
	else
		MeshFile->FirstMesh = NULL;

#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglInfo("nglLoadMeshFile: Loaded %s%s.\n", nglMeshPath, FileName.c_str());
#endif

	return true;
}

/*-----------------------------------------------------------------------------
Description: Load a "mesh collection" from a memory address.
-----------------------------------------------------------------------------*/
bool nglLoadMeshFileInPlace(const nglFixedString &FileName, void *Data)
{
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglMeshFileBank.Search(FileName)))
	{
		Inst->RefCount++;
		return true;
	}

	nglMeshFile *MeshFile = (nglMeshFile *) nglMemAllocInternal(sizeof(nglMeshFile));
	strcpy(MeshFile->FilePath, "");
	MeshFile->FileName = FileName;
	MeshFile->LoadedInPlace = true;
	MeshFile->FileBuf.Buf = (uint8 *) Data;
	MeshFile->FileBuf.Size = 0;
	MeshFile->FileBuf.UserData = 0;

	bool Result = nglLoadMeshFileInternal(FileName, MeshFile);
	if (!Result)
	{
		nglMemFreeInternal(MeshFile);
		return false;
	}

	nglMeshFileBank.Insert(FileName, MeshFile);

	return true;
}

/*-----------------------------------------------------------------------------
Description: Load a "mesh collection" file into memory.
-----------------------------------------------------------------------------*/
bool nglLoadMeshFile(const nglFixedString &FileName)
{
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglMeshFileBank.Search(FileName)))
	{
		Inst->RefCount++;
		return true;
	}

	char nglWork[NGL_MAX_PATH_LENGTH];
	strcpy(nglWork, nglMeshPath);
	strcat(nglWork, FileName.c_str());
	strcat(nglWork, NGL_MESHFILE_EXT);

	nglMeshFile *MeshFile =	(nglMeshFile *) nglMemAllocInternal(sizeof(nglMeshFile));
	strcpy(MeshFile->FilePath, nglMeshPath);
	MeshFile->FileName = FileName;
	MeshFile->LoadedInPlace = false;

	// Vertex buffer MUST be on a 4KB boundary.
	if (!nglReadFile(nglWork, &MeshFile->FileBuf, 4096))
	{
		nglError("nglLoadMeshFile: Unable to open %s%s.\n", nglMeshPath, FileName.c_str());
		nglMemFreeInternal(MeshFile);
		return false;
	}

	bool Result = nglLoadMeshFileInternal(FileName, MeshFile);
	if (!Result)
	{
		nglReleaseFile(&MeshFile->FileBuf);
		nglMemFreeInternal(MeshFile);
		return false;
	}

	nglMeshFileBank.Insert(FileName, MeshFile);

	return true;
}

/*-----------------------------------------------------------------------------
Description: Release a mesh from the memory.
             The mesh is freed only when its ref counter reaches 0,
             otherwise its counter is decreased by 1.
-----------------------------------------------------------------------------*/
void nglReleaseMeshFile(const nglFixedString &FileName)
{
	// Remove a reference from the instance bank, delete the mesh file only if the count hits 0.
	nglInstanceBank::Instance *Inst = nglMeshFileBank.Search(FileName);
	if (!Inst)
		return;
	if (--Inst->RefCount > 0)
		return;

	nglMeshFile *MeshFile = (nglMeshFile *) Inst->Value;
	nglMeshFileBank.Delete(MeshFile->FileName);

	nglMesh *Mesh = MeshFile->FirstMesh;
	while (Mesh)
	{
		for (uint32 i = 0; i < Mesh->NSections; i++)
		{
			// Release used textures.
			nglMaterial *Material = Mesh->Sections[i].Material;
			if (Material->Flags & NGLMAT_TEXTURE_MAP)
				nglReleaseTexture(Material->Map);

			if (Material->Flags & NGLMAT_LIGHT_MAP)
				nglReleaseTexture(Material->LightMap);

			if (Material->Flags & NGLMAT_DETAIL_MAP)
				nglReleaseTexture(Material->DetailMap);

			if (Material->Flags & NGLMAT_ENVIRONMENT_MAP)
				nglReleaseTexture(Material->EnvironmentMap);

#if NGL_USE_PUSHBUFFER
			// Release the pushbuffer (if it was used) and its relative fixups.
			nglMeshSection *Section = &Mesh->Sections[i];
			if (Section->PB)
			{
				nglMemFree((void *) Section->PB->Data);
				nglMemFreeInternal(Section->PB);
			}
#endif
		}

		// Inverse the bones matrices.
		for (i = 0; i < Mesh->NBones; i++)
		{
			XGMatrixInverse((XGMATRIX *)&(Mesh->Bones[i]), NULL, (XGMATRIX *)&(Mesh->Bones[i]));
		}
		Mesh->Flags &= ~NGLMESH_PROCESSED;
		nglUnRebaseMesh(0, (uint32) MeshFile->FileBuf.Buf, Mesh);

		nglMeshBank.Delete(Mesh->Name);
		Mesh = Mesh->NextMesh;
	}

	if (!MeshFile->LoadedInPlace)
	{
		nglMeshFileHeader *Header = (nglMeshFileHeader *) MeshFile->FileBuf.Buf;
		uint32 i;

		// Unbind the current VB to be sure the one we are releasing isn't used by the GPU.
		nglBindVertexBuffer(0, 0);

#if NGL_FORCE_CREATE_VB
		// Release the non-skin VBs.
		for (i = 0; i < Header->NVertexBuffers; i++)
		{
			((IDirect3DVertexBuffer8 *) Header->VertexBuffers[i].
			 VertexBufferData)->Release();
		}

		// Release the skin VBs.
		for (i = 0; i < Header->NSkinVertexBuffers; i++)
		{
			((IDirect3DVertexBuffer8 *) Header->SkinVertexBuffers[i].
			 VertexBufferData)->Release();
		}
#else
		// If the resource is still used, wait until it could be freed up.
		for (i = 0; i < Header->NVertexBuffers; i++)
		{
			Header->VertexBuffers[i].VertexBuffer.BlockUntilNotBusy();

			Header->VertexBuffers[i].VertexBuffer.Common = 0;
			Header->VertexBuffers[i].VertexBuffer.Data = 0;
			((IDirect3DResource8 *) &Header->VertexBuffers[i].VertexBuffer)->
				Lock = 0;
		}

		// Same for the skin VBs.
		for (i = 0; i < Header->NSkinVertexBuffers; i++)
		{
			Header->SkinVertexBuffers[i].VertexBuffer.BlockUntilNotBusy();

			Header->VertexBuffers[i].VertexBuffer.Common = 0;
			Header->VertexBuffers[i].VertexBuffer.Data = 0;
			((IDirect3DResource8 *) &Header->VertexBuffers[i].VertexBuffer)->
				Lock = 0;
		}
#endif

		// Undo the rebase of the vertex/index data.
		for (i = 0; i < Header->NVertexBuffers; i++)
			nglUnRebaseVertexBuffer((uint32) Header, &Header->VertexBuffers[i]);

		for (i = 0; i < Header->NSkinVertexBuffers; i++)
			nglUnRebaseVertexBuffer((uint32) Header, &Header->SkinVertexBuffers[i]);

		// Undo the rebase of the 2 header's vertex buffer pointers (skin/unskin).
		nglUnRebaseHeader((uint32) Header, Header);

		nglReleaseFile(&MeshFile->FileBuf);
	}

	nglMemFreeInternal(MeshFile);
}

/*-----------------------------------------------------------------------------
Description: Force all of the meshes to be released.
-----------------------------------------------------------------------------*/
void nglReleaseAllMeshFiles()
{
	nglInstanceBank::Instance *Inst = nglMeshFileBank.Head->Forward[0];
	while (Inst != nglMeshFileBank.NIL)
	{
		if (((nglMeshFile *) Inst->Value)->FileName == nglFixedString("nglsphere")
			|| ((nglMeshFile *) Inst->Value)->FileName == nglFixedString("nglradius"))
		{
			Inst = Inst->Forward[0];
		}
		else
		{
			Inst->RefCount = 1;
			nglReleaseMeshFile(((nglMeshFile *) Inst->Value)->FileName);
			Inst = nglMeshFileBank.Head->Forward[0];
		}
	}
}

/*-----------------------------------------------------------------------------
Description: Add a mesh node to the render list.
-----------------------------------------------------------------------------*/
void nglListAddMesh(nglMesh *Mesh, const nglMatrix &_LocalToWorld, const nglRenderParams *Params)
{
	nglVector WorldCenter;		// World space center of the bounding sphere.
	nglVector Center;			// View space center of the bounding sphere.

	// Check for NULL pointers, commonly encountered when file loads fail.
	if (!Mesh)
		return;

	// Required because LocalToWorld must be 16-byte aligned.
	nglMatrix LocalToWorld;
	nglMatrixCopy(&LocalToWorld, &_LocalToWorld);

	float Radius = Mesh->SphereRadius;

#if NGL_DEBUG
	// Skip skinned meshes if this stage is disabled.
	if (Mesh->NBones && !nglStage.Skin)
		return;

	// Skip regular meshes if this stage is disabled.
	if (!nglStage.NonSkin && !Mesh->NBones && !(Mesh->Flags & NGLMESH_SCRATCH_MESH))
		return;

	// Skip scratch meshes (temporary and static) if this stage is disabled.
	if (!nglStage.Scratch && (Mesh->Flags & NGLMESH_SCRATCH_MESH))
		return;

	if (nglDebug.DumpFrameLog & 4)
		nglLog("nglListAddMesh: Added mesh %s at (%f,%f,%f).\n",
			   Mesh->Name.c_str(),
			   LocalToWorld[3][0], LocalToWorld[3][1], LocalToWorld[3][2]);
#endif

	// Add the debugging sphere.
#if NGL_DEBUG
	if (nglDebug.DrawMeshSpheres && (Mesh != nglRadiusMesh)	&& (Mesh != nglSphereMesh))
	{
		nglRenderParams p;
		if (Params)
			p = *Params;
		else
		{
			p.Scale[0] = p.Scale[1] = p.Scale[2] = 0;
			p.Flags = 0;
		}

		if (p.Flags & NGLP_SCALE)
		{
			p.Scale[0] *= Mesh->SphereRadius;
			p.Scale[1] *= Mesh->SphereRadius;
			p.Scale[2] *= Mesh->SphereRadius;
		}
		else
		{
			p.Scale[0] = Mesh->SphereRadius;
			p.Scale[1] = Mesh->SphereRadius;
			p.Scale[2] = Mesh->SphereRadius;
		}
		p.Flags = NGLP_SCALE;

		nglListAddMesh(nglSphereMesh, LocalToWorld, &p);
	}
#endif // NGL_DEBUG

	if (!(Params && (Params->Flags & NGLP_FULL_MATRIX)))
	{
		// Update the mesh radius for scale.
		if (Params && (Params->Flags & NGLP_SCALE))
		{
			float Scale = Params->Scale[0];
			if (Params->Scale[1] > Scale)
				Scale = Params->Scale[1];
			if (Params->Scale[2] > Scale)
				Scale = Params->Scale[2];
			Radius *= Scale;
		}

		nglApplyMatrix(WorldCenter, LocalToWorld, Mesh->SphereCenter);
		nglApplyMatrix(Center, nglCurScene->WorldToView, WorldCenter);

		// Hack to allow us to turn off clipping for ortho transform
		if (!nglCurScene->UsingOrtho)
		{
			// If the mesh is outside of one of the 6 frustum clipping planes, reject it.
#if NGL_DEBUG
			if (nglStage.SphereReject)
#endif

				if (Params && !(Params->Flags & NGLP_NO_CULLING))
				{
					if ((!nglSphereInFrustum(Center, Radius)) && (Mesh->Flags & NGLMESH_REJECT_SPHERE))
					{
#if NGL_DEBUG
						if (nglDebug.DumpFrameLog & 4)
							nglLog("nglListAddMesh: Mesh rejected by NGLMESH_REJECT_SPHERE.");
#endif

						return;
					}
				}
		}

		// FIXME: LOD support. Very slow implementation, but it's okay since we don't use any yet.;)
		if (Mesh->Flags & NGLMESH_LOD)
		{
			float LODSwitchRange = 0;
			nglMesh *LODMesh = NULL;
			for (uint32 i = 0; i < Mesh->NLODs; i++)
			{
				if (Center[2] > Mesh->LODs[i].Range)
				{
					if (LODSwitchRange < Mesh->LODs[i].Range)
					{
						LODSwitchRange = Mesh->LODs[i].Range;
						LODMesh = nglGetMesh(Mesh->LODs[i].Name);
					}
				}
			}
			if (LODMesh)
				Mesh = LODMesh;
		}
	}

	// Add a new mesh node to the mesh list.
	nglMeshNode *MeshNode = nglListNew(nglMeshNode);
	MeshNode->Mesh = Mesh;
	nglMatrixCopy(&MeshNode->LocalToWorld, &LocalToWorld);
	XGMatrixInverse((XGMATRIX *)&MeshNode->WorldToLocal, NULL, (XGMATRIX *)&LocalToWorld);

	if (Params)
		MeshNode->Params = *Params;
	else
		MeshNode->Params.Flags = 0;

	if (MeshNode->Params.Flags & NGLP_FULL_MATRIX)
		nglMatrixCopy(&MeshNode->LocalToScreen, &LocalToWorld);
	else
	{
		if (MeshNode->Params.Flags & NGLP_SCALE)
		{
			nglVectorScale3(MeshNode->LocalToWorld[0], MeshNode->LocalToWorld[0], MeshNode->Params.Scale[0]);
			nglVectorScale3(MeshNode->LocalToWorld[1], MeshNode->LocalToWorld[1], MeshNode->Params.Scale[1]);
			nglVectorScale3(MeshNode->LocalToWorld[2], MeshNode->LocalToWorld[2], MeshNode->Params.Scale[2]);
		}

		// Calculate the local to screen matrix.
		nglMatrixMul(MeshNode->LocalToScreen, nglCurScene->WorldToScreen, MeshNode->LocalToWorld);
	}

	// Create a new node for each material.
	for (uint32 i = 0; i < Mesh->NSections; i++)
	{
		// Skip materials that should be masked off.
		if (MeshNode->Params.Flags & NGLP_MATERIAL_MASK)
			if ((1 << i) & MeshNode->Params.MaterialMask)
				continue;

		nglMeshSection *Section = &Mesh->Sections[i];
		nglMaterial *Material = Section->Material;

		nglMeshSectionNode *SectionNode = nglListNew(nglMeshSectionNode);
		SectionNode->MeshNode = MeshNode;
		SectionNode->Section = Section;
		SectionNode->MaterialFlags = Material->Flags;
		SectionNode->VSHandle = Section->VSInitHandle;

		// If tinting is attempting to make the mesh translucent, make it sort.
		if ((MeshNode->Params.Flags & NGLP_TINT) && (Params->TintColor[3] < 1.0f))
			SectionNode->MaterialFlags |= (NGLMAT_ALPHA | NGLMAT_BACKFACE_CULL);

		// Build the info structure and add the node.
		nglSortInfo SortInfo;
		if (SectionNode->MaterialFlags & NGLMAT_ALPHA)
		{
			SortInfo.Type = NGLSORT_TRANSLUCENT;

			// Check the first material for NGLMAT_ALPHA_SORT_FIRST.
			if (Material->Flags & NGLMAT_ALPHA_SORT_FIRST)
				SortInfo.Dist = Material->ForcedSortDistance;
			else
				SortInfo.Dist = Center[2] + Radius;
		}
		else
		{
			SortInfo.Type = NGLSORT_OPAQUE;
			// Sections are sorted by vertex shader handles.
			// As we don't know yet the runtime parameters that affect the section (lights, NGLP_xxx, etc.),
			// we don't build the SortInfo.Hash here.
			// It is done in the nglRenderScene() function just before sorting the nodes.

			// This section has a customized renderer, so don't use the default NGL vertex shader key system.
			if (Section->RenderSectionFunc != nglRenderSection)
			{
				SortInfo.Hash = SectionNode->VSHandle;
			}
		}

		nglListAddNode(NGLNODE_MESH_SECTION, Section->RenderSectionFunc, SectionNode, &SortInfo);
	}
}

/*-----------------------------------------------------------------------------
Description: Find the index of a material by ID, suitable for use with
             NGLP_MATERIAL_MASK. Returns -1 if the ID can't be found.
-----------------------------------------------------------------------------*/
int32 nglGetMaterialIdx(nglMesh *Mesh, uint32 MaterialID)
{
	for (uint32 i = 0; i < Mesh->NSections; i++)
		if (Mesh->Sections[i].Material->MaterialID == MaterialID)
			return i;

	return -1;
}

/*-----------------------------------------------------------------------------
Description: Frustum clipping: test a sphere rejection according to
             the scene frustum (6 planes).
             Return true if the sphere is in the viewing frustum.
-----------------------------------------------------------------------------*/
inline bool nglSphereInFrustum(const nglVector &Center, float Radius)
{
	// NearZ plane clipping.
	if (Center[2] + Radius < nglCurScene->NearZ)
		return false;

	// FarZ plane clipping.
	if (Center[2] - Radius > nglCurScene->FarZ)
		return false;

	// Left plane clipping.
	float hfov_rad = NGL_PI * nglCurScene->HFOV / 180.0f;
	float cf = cosf(hfov_rad);
	float sf = sinf(hfov_rad) * Center[2] + Radius;

	if (Center[0] * -cf + sf < 0.0f)
		return false;

	// Right plane clipping.
	if (Center[0] * cf + sf < 0.0f)
		return false;

	// Up plane clipping.
	if (Center[1] * cf + sf < 0.0f)
		return false;

	// Bottom plane clipping.
	if (Center[1] * -cf + sf < 0.0f)
		return false;

	return true;
}

/*---------------------------------------------------------------------------------------------------------
 Lighting API.



---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Draw the debug sphere/radius (solid/non-solid) using the specified
             position, scale and color.
-----------------------------------------------------------------------------*/
#if NGL_DEBUG
void nglDrawDebugSphere(const nglVector &Pos, float Scale, nglVector &Color, bool Solid)
{
	nglMesh *Mesh = Solid ? nglSphereMesh : nglRadiusMesh;
	if (!Mesh)
		return;

	nglRenderParams Params;

	Params.Flags = NGLP_TINT | NGLP_SCALE;
	nglVectorCopy(&Params.TintColor, &Color);

	Params.Scale[0] = Scale / Mesh->SphereRadius;	// Divide is there in case the radius isn't exactly 1.0.
	Params.Scale[1] = Scale / Mesh->SphereRadius;
	Params.Scale[2] = Scale / Mesh->SphereRadius;
	Params.Scale[3] = 1.0f;

	nglMatrix Work;
	nglMatrixIdentity(Work);
	nglMatrixTrans(Work, Work, Pos);

	nglListAddMesh(Mesh, Work, &Params);
}
#endif

/*-----------------------------------------------------------------------------
Description: Create the default lighting context.
-----------------------------------------------------------------------------*/
nglLightContext* nglCreateLightContext()
{
	nglLightContext *Context = nglListNew(nglLightContext);
	if (!Context)
		return NULL;

	// Initialize the circular linked list.
	for (int32 i = 0; i < NGL_MAX_LIGHTS; i++)
	{
		Context->Head.Next[i] = &Context->Head;
		Context->ProjectorHead.Next[i] = &Context->ProjectorHead;
	}

	Context->Ambient[0] = 1.0f;
	Context->Ambient[1] = 1.0f;
	Context->Ambient[2] = 1.0f;
	Context->Ambient[3] = 1.0f;

	nglCurLightContext = Context;

	return Context;
}

/*-----------------------------------------------------------------------------
Description: Switch the current lighting context.
-----------------------------------------------------------------------------*/
void nglSetLightContext(nglLightContext *Context)
{
  nglCurLightContext = Context;
}

/*-----------------------------------------------------------------------------
Description: Retrieve the current lighting context.
-----------------------------------------------------------------------------*/
nglLightContext* nglGetLightContext()
{
  return nglCurLightContext;
}

/*-----------------------------------------------------------------------------
Description: Set the ambiant light.
-----------------------------------------------------------------------------*/
void nglSetAmbientLight(float R, float G, float B, float A)
{
	nglCurLightContext->Ambient[0] = R;
	nglCurLightContext->Ambient[1] = G;
	nglCurLightContext->Ambient[2] = B;
	nglCurLightContext->Ambient[3] = A;
}

/*-----------------------------------------------------------------------------
Description: Add a new light node.
-----------------------------------------------------------------------------*/
void nglListAddLightNode(uint32 Type, void *Data, uint32 LightCat)
{
	nglLightNode *NewNode = nglListNew(nglLightNode);
	NewNode->Type = Type;
	NewNode->Data = Data;
	NewNode->LightCat = LightCat;

	for (int32 i = 0; i < NGL_MAX_LIGHTS; i++)
		if (LightCat & (1 << (NGL_LIGHTCAT_SHIFT + i)))
		{
			NewNode->Next[i] = nglCurLightContext->Head.Next[i];
			nglCurLightContext->Head.Next[i] = NewNode;
		}
}

/*-----------------------------------------------------------------------------
Description: Add a directional light to the light list.
-----------------------------------------------------------------------------*/
void nglListAddDirLight(uint32 LightCat, nglVector &Dir, nglVector &Color)
{
#ifdef PROJECT_SPIDERMAN
	Color[3] = 0.0f;
#endif

	nglDirLightInfo *Light = nglListNew(nglDirLightInfo);
	nglVectorCopy(&Light->Dir, &Dir);
	Light->Dir[3] = 0.0f;
	nglVectorCopy(&Light->Color, &Color);

	nglListAddLightNode(NGLLIGHT_DIRECTIONAL, Light, LightCat);

	// No debug rendering at present for directional lights.
}

/*-----------------------------------------------------------------------------
Description: Add a fake point light to the light list.
-----------------------------------------------------------------------------*/
void nglListAddFakePointLight(uint32 LightCat, nglVector &Pos, float Near, float Far, nglVector &Color)
{
#ifdef PROJECT_SPIDERMAN
	Color[3] = 0.0f;
#endif

	// Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
	nglVector ViewPos;
	nglApplyMatrix(ViewPos, nglCurScene->WorldToView, Pos);
	if (!nglSphereInFrustum(ViewPos, Far))
		return;

	nglPointLightInfo *Light = nglListNew(nglPointLightInfo);
	nglVectorCopy(&Light->Pos, &Pos);
	Light->Pos[3] = 1.0f;
	Light->Near = Near;
	Light->Far = Far;
	nglVectorCopy(&Light->Color, &Color);

	nglListAddLightNode(NGLLIGHT_FAKEPOINT, Light, LightCat);

#if NGL_DEBUG
	if (nglDebug.DrawLightSpheres)
	{
		nglDrawDebugSphere(Light->Pos, 1.0f, Light->Color, true);
		nglDrawDebugSphere(Light->Pos, Light->Near, Light->Color, false);
		nglDrawDebugSphere(Light->Pos, Light->Far, Light->Color, false);
	}
#endif
}

/*-----------------------------------------------------------------------------
Description: Add a true point light to the light list.
-----------------------------------------------------------------------------*/
void nglListAddPointLight(uint32 LightCat, nglVector &Pos, float Near, float Far, nglVector &Color)
{
#ifdef PROJECT_SPIDERMAN
	Color[3] = 0.0f;
#endif

	// Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
	nglVector ViewPos;
	nglApplyMatrix(ViewPos, nglCurScene->WorldToView, Pos);
	if (!nglSphereInFrustum(ViewPos, Far))
		return;

	nglPointLightInfo *Light = nglListNew(nglPointLightInfo);
	nglVectorCopy(&Light->Pos, &Pos);
	Light->Pos[3] = 1.0f;
	Light->Near = Near;
	Light->Far = Far;
	nglVectorCopy(&Light->Color, &Color);

	nglListAddLightNode(NGLLIGHT_TRUEPOINT, Light, LightCat);

#if NGL_DEBUG
	if (nglDebug.DrawLightSpheres)
	{
		nglDrawDebugSphere(Light->Pos, 1.0f, Light->Color, true);
		nglDrawDebugSphere(Light->Pos, Light->Near, Light->Color, false);
		nglDrawDebugSphere(Light->Pos, Light->Far, Light->Color, false);
	}
#endif
}

/*-----------------------------------------------------------------------------
Description: Legacy interface implemented for compatibility reason.
-----------------------------------------------------------------------------*/
void nglListAddLight(nglLightInfo *Light, nglVector &Pos, nglVector &Dir)
{
	if (Light->Type == NGLLIGHT_DIRECTIONAL)
		nglListAddDirLight(Light->Flags, Dir, Light->ColorAdd);
	else if (Light->Type == NGLLIGHT_FAKEPOINT)
		nglListAddFakePointLight(Light->Flags, Pos, Light->DistFalloffStart, Light->DistFalloffEnd, Light->ColorAdd);
	else if (Light->Type == NGLLIGHT_TRUEPOINT)
		nglListAddPointLight(Light->Flags, Pos, Light->DistFalloffStart, Light->DistFalloffEnd, Light->ColorAdd);
}

/*---------------------------------------------------------------------------------------------------------
 Projector API.

 Taken from PS2 version 148.

---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Computes the 6 planes of the view frustum given a projection
             matrix of any kind (orthographic or perspective) and also can
             incorporate a modelview type transformation to compute planes in
             object space.
-----------------------------------------------------------------------------*/
void nglBuildFrustum(nglFrustum *frustum, nglMatrix &m)
{
	nglVector lp[8] = {		                // local points
		nglVector(0.0f, 0.0f, 0.0f, 1.0f),	// 0 - (0,0,0)
		nglVector(0.0f, 1.0f, 0.0f, 1.0f),	// 1 - (0,1,0)
		nglVector(1.0f, 1.0f, 0.0f, 1.0f),	// 2 - (1,1,0)
		nglVector(1.0f, 0.0f, 0.0f, 1.0f),	// 3 - (1,0,0)
		nglVector(1.0f, 0.0f, 1.0f, 1.0f),	// 4 - (1,0,1)
		nglVector(1.0f, 1.0f, 1.0f, 1.0f),	// 5 - (1,1,1)
		nglVector(0.0f, 0.0f, 1.0f, 1.0f),	// 6 - (0,0,1)
		nglVector(0.0f, 1.0f, 1.0f, 1.0f)	// 7 - (0,1,1)
	};

/*
         y
         |
        1|
        /\
       / |\
      / r|l\
     / a | e\
    / e /0\ f\7
  2|\n /top\/t|
   | \/    /\ |
   |r/\bot/ r\|
  3|/i \5/ a /6
   /\ g | f /  \
  /  \ h|  /    \
 x    \t| /      z
       \|/
        4
*/

	nglVector wp[8];			// world points
	nglVector v1, v2;			// vectors used for computing normals

	NGL_ASSERT(frustum, "Frustum is NULL !");

	for (int32 i = 0; i < 8; i++)
		nglApplyMatrix(wp[i], m, lp[i]);

	// compute near plane normal
	nglVectorSubtract3(v1, wp[1], wp[0]);
	nglVectorSubtract3(v2, wp[3], wp[0]);
	nglVectorCross3(frustum->Planes[NGLFRUSTUM_NEAR], v1, v2);
	nglVectorNormalize3(frustum->Planes[NGLFRUSTUM_NEAR]);
	frustum->Planes[NGLFRUSTUM_NEAR][3] = -nglVectorDot3(frustum->Planes[NGLFRUSTUM_NEAR], wp[0]);

	// compute far plane normal
	nglVectorSubtract3(v1, wp[4], wp[6]);
	nglVectorSubtract3(v2, wp[7], wp[6]);
	nglVectorCross3(frustum->Planes[NGLFRUSTUM_FAR], v1, v2);
	nglVectorNormalize3(frustum->Planes[NGLFRUSTUM_FAR]);
	frustum->Planes[NGLFRUSTUM_FAR][3] = -nglVectorDot3(frustum->Planes[NGLFRUSTUM_FAR], wp[6]);

	// compute left plane normal
	nglVectorSubtract3(v1, wp[6], wp[0]);
	nglVectorSubtract3(v2, wp[1], wp[0]);
	nglVectorCross3(frustum->Planes[NGLFRUSTUM_LEFT], v1, v2);
	nglVectorNormalize3(frustum->Planes[NGLFRUSTUM_LEFT]);
	frustum->Planes[NGLFRUSTUM_LEFT][3] = -nglVectorDot3(frustum->Planes[NGLFRUSTUM_LEFT], wp[0]);

	// compute right plane normal
	nglVectorSubtract3(v1, wp[2], wp[3]);
	nglVectorSubtract3(v2, wp[4], wp[3]);
	nglVectorCross3(frustum->Planes[NGLFRUSTUM_RIGHT], v1, v2);
	nglVectorNormalize3(frustum->Planes[NGLFRUSTUM_RIGHT]);
	frustum->Planes[NGLFRUSTUM_RIGHT][3] = -nglVectorDot3(frustum->Planes[NGLFRUSTUM_RIGHT], wp[3]);

	// compute top plane normal
	nglVectorSubtract3(v1, wp[7], wp[1]);
	nglVectorSubtract3(v2, wp[2], wp[1]);
	nglVectorCross3(frustum->Planes[NGLFRUSTUM_TOP], v1, v2);
	nglVectorNormalize3(frustum->Planes[NGLFRUSTUM_TOP]);
	frustum->Planes[NGLFRUSTUM_TOP][3] = -nglVectorDot3(frustum->Planes[NGLFRUSTUM_TOP], wp[1]);

	// compute bottom plane normal
	nglVectorSubtract3(v1, wp[3], wp[0]);
	nglVectorSubtract3(v2, wp[6], wp[0]);
	nglVectorCross3(frustum->Planes[NGLFRUSTUM_BOTTOM], v1, v2);
	nglVectorNormalize3(frustum->Planes[NGLFRUSTUM_BOTTOM]);
	frustum->Planes[NGLFRUSTUM_BOTTOM][3] = -nglVectorDot3(frustum->Planes[NGLFRUSTUM_BOTTOM], wp[0]);
}

/*-----------------------------------------------------------------------------
Description: Return the distance between a plane and a point.
             Assumes that the x, y, and z componentes of plane are normalized.
-----------------------------------------------------------------------------*/
float nglDistanceToPlane(const nglVector &plane, const nglVector &point)
{
	return (plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2]) + plane[3];
}

/*-----------------------------------------------------------------------------
Description: Return true if the sphere is visible in the frustum.
             Takes a point and radius in world space.
-----------------------------------------------------------------------------*/
bool nglIsSphereVisible(nglFrustum *Frustum, const nglVector &Center, float Radius)
{
	NGL_ASSERT(Frustum, "Frustum is NULL !");

	// This is a good order to cull by, as it culls the majority of invisible
	// objects in the first couple tries.  Relies on short-circuit evaluation!
	Radius = -Radius;

	if (nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_LEFT], Center) < Radius)
		return false;
	if (nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_RIGHT], Center) <	Radius)
		return false;
	if (nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_NEAR], Center) < Radius)
		return false;
	if (nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_TOP], Center) < Radius)
		return false;
	if (nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_BOTTOM], Center) < Radius)
		return false;
	if (nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_FAR], Center) < Radius)
		return false;

	return true;
}

/*-----------------------------------------------------------------------------
Description: Add a new projector light node.
-----------------------------------------------------------------------------*/
void nglListAddProjectorLightNode(uint32 Type, void *Data, uint32 LightCat)
{
	nglLightNode *NewNode = nglListNew(nglLightNode);

	if (!NewNode)
		return;

	NewNode->Type = Type;
	NewNode->Data = Data;
	NewNode->LightCat = LightCat;

	for (int32 i = 0; i < NGL_MAX_LIGHTS; i++)
	{
		if (LightCat & (1 << (NGL_LIGHTCAT_SHIFT + i)))
		{
			NewNode->Next[i] = nglCurLightContext->ProjectorHead.Next[i];
			nglCurLightContext->ProjectorHead.Next[i] = NewNode;
		}
	}
}

/*-----------------------------------------------------------------------------
Description: Add a point projector light to the list.

             -NOT IMPLEMENTED-
-----------------------------------------------------------------------------*/
void nglListAddPointProjectorLight(uint32 LightCat, nglVector &Pos, nglVector &Color, float Range,
								   uint32 BlendMode, uint32 BlendModeConstant, nglTexture *Tex)
{
	NGL_ASSERT(Tex, "nglListAddPointProjectorLight() called with a NULL texture !");

	// not implemented yet (vertex shader side).
	return;

#if 0
	// Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
	nglVector ViewPos;
	nglApplyMatrix(ViewPos, nglCurScene->WorldToView, Pos);
	if (!nglSphereInFrustum(ViewPos, Range))
		return;

	nglPointProjectorLightInfo *Light =	nglListNew(nglPointProjectorLightInfo);

	Light->BlendMode = BlendMode;
	Light->BlendModeConstant = BlendModeConstant;

	if (Tex)
		Light->Tex = Tex;
	/*
	   TODO:
	   else
	   Light->Tex = nglPhongTex;
	 */

	nglVectorCopy(&Light->Pos, &Pos);
	Light->Pos[3] = 1.0f;
	nglVectorCopy(&Light->Color, &Color);
	Light->Range = Range;

	nglListAddProjectorLightNode(NGLLIGHT_PROJECTED_POINT, Light, LightCat);

#if NGL_DEBUG
	if (nglDebug.DrawLightSpheres)
	{
		nglDrawDebugSphere(Light->Pos, 1.0f, Light->Color, true);
		nglDrawDebugSphere(Light->Pos, Light->Range, Light->Color, false);
	}
#endif

#endif
}

/*-----------------------------------------------------------------------------
Description: Add a directional projector light to the list.
             On the XBox, it's faster to project to a linear texture (specify
             the NGLTF_LINEAR flag when calling nglCreateTexture).
             The scaling of the UV coordinates is done automatically in the
             vertex shader. Note that you MUST use square linear textures.
-----------------------------------------------------------------------------*/
void nglListAddDirProjectorLight(uint32 LightCat, nglMatrix &PO, nglVector &Scale, uint32 BlendMode,
								 uint32 BlendModeConstant, nglTexture *Tex)
{
	nglDirProjectorLightInfo *Light = nglListNew(nglDirProjectorLightInfo);

	Light->BlendMode = BlendMode;
	Light->BlendModeConstant = BlendModeConstant;

	NGL_ASSERT(Tex, "nglListAddDirProjectorLight() called with a NULL texture !");

	if (Tex)
		Light->Tex = Tex;
	else
		return;
	/*
	   // TODO:
	   else
	   Light->Tex = nglPhongTex;
	 */

	Light->Scale[0] = Scale[0];
	Light->Scale[1] = Scale[1];
	Light->Scale[2] = Scale[2];
	Light->Scale[3] = 1.0f;

	Light->Pos[0] = PO[3][0];
	Light->Pos[1] = PO[3][1];
	Light->Pos[2] = PO[3][2];
	Light->Pos[3] = 0.0f;

	Light->Xaxis[0] = PO[0][0];
	Light->Xaxis[1] = PO[0][1];
	Light->Xaxis[2] = PO[0][2];
	Light->Xaxis[3] = 0.0f;

	Light->Yaxis[0] = PO[1][0];
	Light->Yaxis[1] = PO[1][1];
	Light->Yaxis[2] = PO[1][2];
	Light->Yaxis[3] = 0.0f;

	Light->Zaxis[0] = PO[2][0];
	Light->Zaxis[1] = PO[2][1];
	Light->Zaxis[2] = PO[2][2];
	Light->Zaxis[3] = 0.0f;

	nglMatrix PositionMtx(
		nglVector(1.f, 0.f, 0.f, 0.f),  // X
		nglVector(0.f, 1.f, 0.f, 0.f),  // Y
		nglVector(0.f, 0.f, 1.f, 0.f),  // Z
		nglVector(-Light->Pos[0], -Light->Pos[1], -Light->Pos[2], 1.f)  // T
	);

	nglMatrix OrientationMtx(
		nglVector(Light->Xaxis[0], Light->Yaxis[0], Light->Zaxis[0], 0.f),
		nglVector(Light->Xaxis[1], Light->Yaxis[1], Light->Zaxis[1], 0.f),
		nglVector(Light->Xaxis[2], Light->Yaxis[2], Light->Zaxis[2], 0.f),
		nglVector(0.f, 0.f, 0.f, 1.f)
	);

	nglMatrix ScaleMtx(
		nglVector(1.0f / Scale[0], 0.f, 0.f, 0.f),
		nglVector(0.f, 1.0f / Scale[1], 0.f, 0.f),
		nglVector(0.f, 0.f, 1.0f / Scale[2], 0.f),
		nglVector(0.f, 0.f, 0.f, 1.f)
	);

	nglMatrix UVTranslationMtx(
		nglVector(1.f, 0.f, 0.f, 0.f),
		nglVector(0.f, 1.f, 0.f, 0.f),
		nglVector(0.f, 0.f, 1.f, 0.f),
		nglVector(0.5f, 0.5f, 0.f, 1.f)
	);

	nglMatrix InversePositionMtx(
		nglVector(1.f, 0.f, 0.f, 0.f),  // X
		nglVector(0.f, 1.f, 0.f, 0.f),  // Y
		nglVector(0.f, 0.f, 1.f, 0.f),  // Z
		nglVector(Light->Pos[0], Light->Pos[1], Light->Pos[2], 1.f)  // T
	);

	nglMatrix InverseOrientationMtx(
		nglVector(Light->Xaxis[0], Light->Xaxis[1], Light->Xaxis[2], 0.f),  // X
		nglVector(Light->Yaxis[0], Light->Yaxis[1], Light->Yaxis[2], 0.f),  // Y
		nglVector(Light->Zaxis[0], Light->Zaxis[1], Light->Zaxis[2], 0.f),  // Z
		nglVector(0.f, 0.f, 0.f, 1.f)  // T
		//     X                 Y                Z
	);

	nglMatrix InverseScaleMtx(
		nglVector(Scale[0], 0.f, 0.f, 0.f),
		nglVector(0.f, Scale[1], 0.f, 0.f),
		nglVector(0.f, 0.f, Scale[2], 0.f),
		nglVector(0.f, 0.f, 0.f, 1.f)
	);

	nglMatrix InverseUVTranslationMtx(
		nglVector(1.f, 0.f, 0.f, 0.f),
		nglVector(0.f, 1.f, 0.f, 0.f),
		nglVector(0.f, 0.f, 1.f, 0.f),
		nglVector(-0.5f, -0.5f, 0.f, 1.f)
	);

	// Compute this light's world to UV space matrix.
	nglMatrixMul(Light->WorldToUV, OrientationMtx, PositionMtx);
	nglMatrixMul(Light->WorldToUV, ScaleMtx, Light->WorldToUV);
	// << do perspective here if/when necessary >>
	nglMatrixMul(Light->WorldToUV, UVTranslationMtx, Light->WorldToUV);

	// Compute the inverse matrix as well.
	nglMatrixMul(Light->UVToWorld, InverseScaleMtx, InverseUVTranslationMtx);
	// << do inverse perspective here if/when necessary >>
	nglMatrixMul(Light->UVToWorld, InverseOrientationMtx, Light->UVToWorld);
	nglMatrixMul(Light->UVToWorld, InversePositionMtx, Light->UVToWorld);

	// Build the frustum for this light.
	nglBuildFrustum(&Light->Frustum, Light->UVToWorld);

	nglListAddProjectorLightNode(NGLLIGHT_PROJECTED_DIRECTIONAL, Light, LightCat);
}

/*-----------------------------------------------------------------------------
Description: Add a spot projector light to the list.
-----------------------------------------------------------------------------*/
// Spot lights are faked by dir lights at the moment.
void nglListAddSpotProjectorLight(uint32 LightCat, nglMatrix &PO, nglVector &Scale, float FOV,
								  uint32 BlendMode, uint32 BlendModeConstant, nglTexture *Tex)
{
	// TODO...
}

/*---------------------------------------------------------------------------------------------------------
  Scratch Mesh API

  Set of functions to modify mesh resources, and to dynamically create new meshes.  Note that only one
  mesh may be edited at a time.
---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 Description: Create a scratch mesh that will exactly have NSections.
-----------------------------------------------------------------------------*/
void nglCreateMesh(uint32 NSections, uint32 NBones, nglMatrix *Bones, uint32 VertexSize)
{
	// Set up the mesh.
	uint32 i;
	nglMesh *Mesh = (nglMesh *) nglListAlloc(sizeof(nglMesh));

	memset(Mesh, 0, sizeof(nglMesh));
	Mesh->Flags = NGLMESH_PERFECT_TRICLIP | NGLMESH_SCRATCH_MESH;

	// Set the vertex size.
	Mesh->VertexSize = VertexSize ? VertexSize : sizeof(nglVertexGeneric);

	// Allocate sections.
	Mesh->NSections = NSections;
	Mesh->Sections = (nglMeshSection *) nglListAlloc(NSections * sizeof(nglMeshSection));
	memset(Mesh->Sections, 0, NSections * sizeof(nglMeshSection));

	// Allocate materials.
	for (i = 0; i < NSections; i++)
	{
		Mesh->Sections[i].Material = (nglMaterial *) nglListAlloc(sizeof(nglMaterial));
		memset(Mesh->Sections[i].Material, 0, sizeof(nglMaterial));
	}

	NGL_ASSERT(NBones == 0, "nglCreateMesh() doesn't support bones yet !");

	// Allocate mesh bones (not supported yet).
	/*
	Mesh->NBones = NBones;

	if (Bones)
	{
		Mesh->Bones = (nglMatrix *) nglListAlloc(NBones * sizeof(nglMatrix), 16);

		for (i = 0; i < NBones; ++i)
		{
			nglMatrixCopy(&Mesh->Bones[i], &Bones[i]);
			// Inverse the bones matrices.
			XGMatrixInverse((XGMATRIX *)&(Mesh->Bones[i]), NULL, (XGMATRIX *)&(Mesh->Bones[i]));
		}

		// Setup the bones remapping per section.
		for (uint32 j = 0; j < NSections; j++)
		{
			Mesh->Sections[j].NBones = NBones;
			for (uint32 k = 0; k < NBones; k++)
			{
				Mesh->Sections[j].BonesIdx[k] = (uint16) k;
			}
		}
	}
	*/

	nglScratchTempVertBufPtr = nglScratchVertBufPtr;

	nglScratch = Mesh;
	nglScratchSection = Mesh->Sections - 1;	 // Hack to prepare for 1st nglMeshAddSection call.
	nglScratchVertCount = 0;

	nglVector Center(0.0f, 0.0f, 0.0f, 1.0f);
	nglMeshSetSphere(Center, 1.0e32f);

	nglScratchIsEdited = false;
}

/*-----------------------------------------------------------------------------
Description: Close the current scratch mesh. Return a pointer to the newly
             created scratch mesh.
-----------------------------------------------------------------------------*/
nglMesh *nglCloseMesh()
{
	// Mesh is temporary - only valid for 1 frame.
	if (nglScratch->Flags & NGLMESH_TEMP)
	{
		nglScratchVertBufPtr = nglScratchTempVertBufPtr;
		return nglScratch;
	}
	// Mesh is static - remains in memory until nglDestroyMesh() is called.
	else
	{
		uint32 i;

		// Allocate memory for the mesh and copy it from the scratch memory.
		nglMesh *Mesh = (nglMesh *) nglMemAlloc(sizeof(nglMesh));
		memcpy(Mesh, nglScratch, sizeof(nglMesh));

		// Do the same for the sections and materials.
		nglMeshSection *Sections = (nglMeshSection *) nglMemAlloc(sizeof(nglMeshSection) * Mesh->NSections);
		nglMaterial *Materials = (nglMaterial *) nglMemAlloc(sizeof(nglMaterial) * Mesh->NSections);
		// Normally a memcpy(Section, nglScratch->Sections, sizeof(nglMeshSection) * Mesh->NSections);
		// should be okay but I'm not sure the nglScratch->Sections[i] are contiguous in memory...
		nglMeshSection *SectionsPtr = Sections;
		nglMaterial *MaterialsPtr = Materials;

		uint32 TotalPrimitiveCount = 0;

		for (i = 0; i < Mesh->NSections; i++)
		{
			memcpy(SectionsPtr, &nglScratch->Sections[i], sizeof(nglMeshSection));
			memcpy(MaterialsPtr, &nglScratch->Sections[i].Material[0], sizeof(nglMaterial));

			// Calculate the total number of primitives that have been used.
			TotalPrimitiveCount += SectionsPtr->PrimCount;

			SectionsPtr->Material = MaterialsPtr;
			++SectionsPtr;
			++MaterialsPtr;
		}

		// Setup the mesh sections.
		Mesh->Sections = Sections;

		// Allocate the nglVertexBuffer (basically: VB + IB) struct used by the mesh.
		nglVertexBuffer *VB;
		VB = (nglVertexBuffer *) nglMemAlloc(sizeof(nglVertexBuffer), 0);
		memset(VB, 0, sizeof(nglVertexBuffer));

		// Allocate memory for the vertex data (must use XPhysicalAlloc function).
		uint32 sz = nglScratchVertCount * Mesh->VertexSize;
		VB->VertexBufferData = (uint8 *) nglMemAlloc(sz, D3DVERTEXBUFFER_ALIGNMENT);
		NGL_ASSERT(((uint32) VB->VertexBufferData % D3DVERTEXBUFFER_ALIGNMENT) == 0,
			"nglMemAlloc() must call XPhysicalAlloc to allocate a contiguous-aligned memory chunk !");

		// Copy vertices from the global temp scratch mesh VB.
		nglVertexGeneric *SrcVertices = (nglVertexGeneric *) (nglScratchTempVertBufPtr - sz);
		memcpy(VB->VertexBufferData, SrcVertices, sz);

		// Remove the static scratch mesh's vertices from the global pool.
		nglScratchTotalVertCount -= nglScratchVertCount;

		// Remove the static scratch mesh's primitive infos from the global pool.
		nglScratchPrimInfoPtr -= TotalPrimitiveCount;

		// Allocate memory and store the primitive infos (store them in the index buffer data).
		sz = TotalPrimitiveCount * sizeof(nglPrimitiveInfo);
		VB->IndexBufferData = (void*) nglMemAlloc(sz);
		memcpy(VB->IndexBufferData, &nglScratchPrimInfo[nglScratchPrimInfoPtr], sz);

		for (i = 0; i < Mesh->NSections; i++)
		{
			uint32 RelativeAddr = (uint32)(&nglScratchPrimInfo[nglScratchPrimInfoPtr]) - (uint32)(&nglScratchPrimInfo[0]);
			Mesh->Sections[i].PrimInfo = (nglPrimitiveInfo*)((uint32)VB->IndexBufferData + RelativeAddr);
		}

		// Register the section's vertex data.
		VB->VertexBuffer.Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER;	// Initial ref count to 1.
		VB->VertexBuffer.Register(VB->VertexBufferData);

		// Setup the section's VB pointers.
		for (i = 0; i < Mesh->NSections; i++)
		{
			Mesh->Sections[i].VB = VB;
		}

		return Mesh;
	}
}

/*-----------------------------------------------------------------------------
Description: Release a static scratch mesh.
-----------------------------------------------------------------------------*/
void nglDestroyMesh(nglMesh *Mesh)
{
	nglVertexBuffer *VB = Mesh->Sections[0].VB;

	for (uint32 i = 0; i < Mesh->NSections; i++)
	{
		nglMemFree(&Mesh->Sections[i].Material[0]);	// Release the current material.
		nglMemFree(&Mesh->Sections[i]);	// Release the current section.
	}

	// Release the primitive infos.
	nglMemFree(VB->IndexBufferData);

	// Release the vertex data.
	VB->VertexBuffer.BlockUntilNotBusy();
	VB->VertexBuffer.Common = 0;
	VB->VertexBuffer.Data = 0;
	((IDirect3DResource8 *) &VB->VertexBuffer)->Lock = 0;
	nglMemFree(VB->VertexBufferData);

	// Free the vertex buffer.
	nglMemFree(VB);

	nglMemFree(Mesh);
}

/*-----------------------------------------------------------------------------
Description: Set a scratch mesh to be the currently edited one.
-----------------------------------------------------------------------------*/
void nglEditMesh(nglMesh *Mesh)
{
	nglScratchIsEdited = true;

	nglScratch = (nglMesh *) Mesh;
	nglScratchSection = nglScratch->Sections;

	// Lock the static scratch meshes' vertex buffer.
	if (!(nglScratch->Flags & NGLMESH_TEMP)) 
		nglScratch->Sections[0].VB->VertexBuffer.Lock(0, 0, (BYTE **) &nglScratchTempVertBufPtr, 0);
}

/*-----------------------------------------------------------------------------
Description: Points the current scratch section to the specified index.
-----------------------------------------------------------------------------*/
void nglMeshSetSection(uint32 Idx)
{
	nglScratchSection = &nglScratch->Sections[Idx];
}

/*-----------------------------------------------------------------------------
Description: Not implemented yet.
-----------------------------------------------------------------------------*/
void nglMeshSetVertex(uint32 VertIdx)
{
	// Unimplemented.
	NGL_ASSERT(false, "Function unimplemented yet.");
}

/*-----------------------------------------------------------------------------
Description: Not implemented yet.
-----------------------------------------------------------------------------*/
void nglMeshGetVertex(nglScratchVertex *Vertex)
{
	// Unimplemented.
	NGL_ASSERT(false, "Function unimplemented yet.");
}

/*-----------------------------------------------------------------------------
Description: Add a material to the current scratch mesh (note that this
             functions auto-increments the section pointer).
			 It is possible to specify the type of primitive the section is
			 made of (by default triangle strips - see NGLPRIM_xxx).
			 NVerts and NStrips are ignored on the XBox.
-----------------------------------------------------------------------------*/
void nglMeshAddSection(nglMaterial *Material, uint32 NVerts, uint32 NStrips, uint32 PrimType)
{
	nglScratchSection++;

	if ((uint32) (nglScratchSection - nglScratch->Sections) >= nglScratch->NSections)
		nglFatal("Added too many sections to a scratch mesh.\n");

	if (nglScratchPrimInfoPtr >= NGL_SCRATCH_MAX_PRIMS)
		nglFatal("Primitives (or strips) overflows - Increase NGL_SCRATCH_MAX_PRIMS !\n");

	// Copy the material and correct some common material mistakes.
	memcpy(nglScratchSection->Material, Material, sizeof(nglMaterial));

	if (Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH)
		nglScratchSection->Material->Flags |= NGLMAT_ALPHA;

	// Setup the primitive type.
	nglScratchSection->PrimType = PrimType;

	// Set up the section parameters.
	nglSetShaderConfig(nglScratchSection);

	nglScratchSection->PrimCount = 0;
	nglScratchSection->PrimInfo = &nglScratchPrimInfo[nglScratchPrimInfoPtr];
	nglScratchSection->BonesIdx = 0;
	nglScratchSection->NBones = 0;
	nglScratchSection->PB = 0;

	nglScratchSection->RenderSectionFunc = nglRenderSection;
	nglScratchSection->VSInitHandle = 0;
}

/*-----------------------------------------------------------------------------
Description: Add a primitive of an arbitrary length to the current scratch
             mesh. It is a extended version of nglMeshWriteStrip which works
			 with any kind of supported primitive (see NGLPRIM_xxx).
-----------------------------------------------------------------------------*/
void nglMeshWritePrimitive(uint32 Length)
{
	if (nglScratchIsEdited)
		return;

	if (nglScratchPrimInfoPtr >= NGL_SCRATCH_MAX_PRIMS)
		nglFatal("Primitives (or strips) overflows - Increase NGL_SCRATCH_MAX_PRIMS !\n");

	nglScratchPrimInfo[nglScratchPrimInfoPtr].VertCount = Length;
	nglScratchPrimInfo[nglScratchPrimInfoPtr].VertIndex = nglScratchTotalVertCount;

	nglScratchTotalVertCount += Length;
	nglScratchSection->PrimCount++;
	nglScratchPrimInfoPtr++;

	nglScratchVertCount += Length;
}

/*-----------------------------------------------------------------------------
Description: Add a triangle strip of an arbitrary length to the current
             scratch mesh.
-----------------------------------------------------------------------------*/
void nglMeshWriteStrip(uint32 Length)
{
	nglMeshWritePrimitive(Length);
}

/*-----------------------------------------------------------------------------
Description: Functions to add a vertex to the current scratch mesh.
-----------------------------------------------------------------------------*/
inline void nglScratchAddVertexNorm(nglVertexGeneric *Vertex, float NX, float NY, float NZ)
{
	Vertex->Norm.x = nglFTOI(NX * ((1 << 10) - 1));
	Vertex->Norm.y = nglFTOI(NY * ((1 << 10) - 1));
	Vertex->Norm.z = nglFTOI(NZ * ((1 << 9) - 1));
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPC(float X, float Y, float Z, uint32 Color)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	Vert->Diffuse = Color;
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPCUV(float X, float Y, float Z, uint32 Color, float U, float V)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	Vert->Diffuse = Color;
	Vert->UV[0] = XGVECTOR2(U, V);
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPCUVB(float X, float Y, float Z, uint32 Color, float U, float V, int32 bones,
							 float w1, float w2, float w3, float w4, int32 b1, int32 b2, int32 b3, int32 b4)
{
	NGL_ASSERT(false, "Function not supported !\n");
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPCUV2(float X, float Y, float Z, uint32 Color, float U, float V, float U2, float V2)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	Vert->Diffuse = Color;
	Vert->UV[0] = XGVECTOR2(U, V);
	Vert->UV[1] = XGVECTOR2(U2, V2);
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPCUV2B(float X, float Y, float Z, uint32 Color, float U, float V, float U2, float V2, int32 bones,
							  float w1, float w2, float w3, float w4, int32 b1, int32 b2, int32 b3, int32 b4)
{
	NGL_ASSERT(false, "Function not supported !\n");
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPUV(float X, float Y, float Z, float U, float V)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr );
	Vert->Pos = XGVECTOR3(X, Y, Z);
	Vert->UV[0] = XGVECTOR2(U, V);
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPN(float X, float Y, float Z, float NX, float NY, float NZ)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	nglScratchAddVertexNorm(Vert, NX, NY, NZ);
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPNC(float X, float Y, float Z, float NX, float NY, float NZ, uint32 Color)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	nglScratchAddVertexNorm(Vert, NX, NY, NZ);
	Vert->Diffuse = Color;
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPNCUV(float X, float Y, float Z, float NX, float NY, float NZ, uint32 Color, float U, float V)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	nglScratchAddVertexNorm(Vert, NX, NY, NZ);
	Vert->Diffuse = Color;
	Vert->UV[0] = XGVECTOR2(U, V);
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPNUV(float X, float Y, float Z, float NX, float NY, float NZ, float U, float V)
{
	nglVertexGeneric *Vert = (nglVertexGeneric *) (nglScratchTempVertBufPtr);
	Vert->Pos = XGVECTOR3(X, Y, Z);
	nglScratchAddVertexNorm(Vert, NX, NY, NZ);
	Vert->UV[0] = XGVECTOR2(U, V);
	nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPNUVB(float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, int32 bones,
							 float w1, float w2, float w3, float w4, int32 b1, int32 b2, int32 b3, int32 b4)
{
	NGL_ASSERT(false, "Function not supported !\n");
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPNUV2(float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2)
{
  nglVertexGeneric* Vert = (nglVertexGeneric*) (nglScratchTempVertBufPtr);
  Vert->Pos = XGVECTOR3(X, Y, Z);
  nglScratchAddVertexNorm(Vert, NX, NY, NZ);
  Vert->UV[0] = XGVECTOR2(U, V);
  Vert->UV[1] = XGVECTOR2(U2, V2);
  nglScratchTempVertBufPtr += sizeof(nglVertexGeneric);
}

/*---------------------------------------------------------------------------*/
void nglMeshWriteVertexPNUV2B(float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2,
							  int32 bones, float w1, float w2, float w3, float w4, int32 b1, int32 b2, int32 b3, int32 b4)
{
	NGL_ASSERT(false, "Function not supported !\n");
}

/*-----------------------------------------------------------------------------
Description: Set the scratch mesh's bounding sphere.
-----------------------------------------------------------------------------*/
void nglMeshSetSphere(nglVector &Center, float Radius)
{
	nglVectorCopy(&nglScratch->SphereCenter, &Center);
	nglScratch->SphereRadius = Radius;

	for (uint32 s = 0; s < nglScratch->NSections; s++)
	{
		nglVectorCopy(&nglScratch->Sections[s].SphereCenter, &Center);
		nglScratch->Sections[s].SphereRadius = Radius;
	}
}

/*-----------------------------------------------------------------------------
Description: Not implemented yet.
-----------------------------------------------------------------------------*/
void nglMeshCalcSphere(uint32 NVerts /*ignored */ )
{
	/*
	   float MinX = 1.0e32f, MaxX = -1.0e32f;
	   float MinY = 1.0e32f, MaxY = -1.0e32f;
	   float MinZ = 1.0e32f, MaxZ = -1.0e32f;

	   uint32 s, i, j;

	   // Calculate the center.
	   for (s = 0; s < nglScratch->NSections; ++s)
	   {
	   float SectionMinX = 1.0e32f, SectionMaxX = -1.0e32f;
	   float SectionMinY = 1.0e32f, SectionMaxY = -1.0e32f;
	   float SectionMinZ = 1.0e32f, SectionMaxZ = -1.0e32f;

	   for (j = 0; j < nglScratch->Sections[s].NBatches; j++)
	   {
	   nglMeshBatchInfo* Info = &nglScratch->Sections[s].BatchInfo[j];
	   int32 Idx = 0;
	   for (i = 0; i < Info->NVerts; i++)
	   {
	   float X, Y, Z;
	   X = Info->PosData[Idx++];
	   if (X < SectionMinX) SectionMinX = X;
	   if (X > SectionMaxX) SectionMaxX = X;

	   Y = Info->PosData[Idx++];
	   if (Y < SectionMinY) SectionMinY = Y;
	   if (Y > SectionMaxY) SectionMaxY = Y;

	   Z = Info->PosData[Idx++];
	   if (Z < SectionMinZ) SectionMinZ = Z;
	   if (Z > SectionMaxZ) SectionMaxZ = Z;
	   }
	   }

	   nglVector Center;
	   float Radius;

	   Center[0] = SectionMinX + (SectionMaxX - SectionMinX) * 0.5f;
	   Center[1] = SectionMinY + (SectionMaxY - SectionMinY) * 0.5f;
	   Center[2] = SectionMinZ + (SectionMaxZ - SectionMinZ) * 0.5f;
	   Center[3] = 1.0f;

	   // Calculate the radius from an arbitrary point on the box.
	   float DistX, DistY, DistZ;
	   DistX = SectionMinX - Center[0];
	   DistY = SectionMinY - Center[1];
	   DistZ = SectionMinZ - Center[2];
	   Radius = sqrtf(DistX * DistX + DistY * DistY + DistZ * DistZ);

	   sceVu0CopyVector(nglScratch->Sections[s].SphereCenter, Center);
	   nglScratch->Sections[s].SphereRadius = Radius;

	   if (SectionMinX < MinX) MinX = SectionMinX;
	   if (SectionMaxX > MaxX) MaxX = SectionMaxX;

	   if (SectionMinY < MinY) MinY = SectionMinY;
	   if (SectionMaxY > MaxY) MaxY = SectionMaxY;

	   if (SectionMinZ < MinZ) MinZ = SectionMinZ;
	   if (SectionMaxZ > MaxZ) MaxZ = SectionMaxZ;
	   }

	   nglVector Center;
	   float Radius;

	   Center[0] = MinX + (MaxX - MinX) * 0.5f;
	   Center[1] = MinY + (MaxY - MinY) * 0.5f;
	   Center[2] = MinZ + (MaxZ - MinZ) * 0.5f;
	   Center[3] = 1.0f;

	   // Calculate the radius from an arbitrary point on the box.
	   float DistX, DistY, DistZ;
	   DistX = MinX - Center[0];
	   DistY = MinY - Center[1];
	   DistZ = MinZ - Center[2];
	   Radius = sqrtf(DistX * DistX + DistY * DistY + DistZ * DistZ);

	   // Fill out the appropriate mesh entries.
	   sceVu0CopyVector(nglScratch->SphereCenter, Center);
	   nglScratch->SphereRadius = Radius;
	 */
}

/*-----------------------------------------------------------------------------
Description: Must only be used with scratch meshes !
-----------------------------------------------------------------------------*/
void nglSetMeshFlags(uint32 Flags)
{
	nglScratch->Flags = Flags | NGLMESH_SCRATCH_MESH | (nglScratch->Flags & NGLMESH_TEMP);
}

/*-----------------------------------------------------------------------------
Description: Return the flags of the current scratch mesh.
-----------------------------------------------------------------------------*/
uint32 nglGetMeshFlags()
{
	return nglScratch->Flags;
}

/*-----------------------------------------------------------------------------
Description: Legacy API.
-----------------------------------------------------------------------------*/
uint32 nglCreateScratchMesh(int32 NVerts, bool Locked)
{
	nglMaterial Material;
	memset(&Material, 0, sizeof(nglMaterial));
	Material.MapBlendMode = NGLBM_OPAQUE;
	Material.Flags = NGLMAT_BILINEAR_FILTER | NGLMAT_PERSPECTIVE_CORRECT | NGLMAT_LIGHT;

	nglCreateMesh(1);
	nglSetMeshFlags(NGLMESH_TEMP);
	nglMeshAddSection(&Material, NVerts);

	return (uint32) nglScratch;
}

/*-----------------------------------------------------------------------------
Description: Legacy API.
-----------------------------------------------------------------------------*/
void nglListAddScratchMesh(uint32 ID, const nglMatrix &LocalToWorld, nglRenderParams *Params)
{
	nglScratchVertBufPtr = nglScratchTempVertBufPtr;
	nglListAddMesh((nglMesh *) ID, LocalToWorld, Params);
}

/*-----------------------------------------------------------------------------
Description: Legacy API.
-----------------------------------------------------------------------------*/
void nglScratchSetMaterial(nglMaterial *Material)
{
	// Copy the material and correct some common material mistakes.
	memcpy(nglScratchSection->Material, Material, sizeof(nglMaterial));

	if (Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH)
		nglScratchSection->Material->Flags |= NGLMAT_ALPHA;

	nglSetShaderConfig(nglScratchSection);
}

/*-----------------------------------------------------------------------------
Description: Lock the scratch meshes vertex buffer.
             This function MUST NOT be called more than once a frame.
-----------------------------------------------------------------------------*/
void nglLockScratchMeshVertexBuffer()
{
	static uint32 CurVB = 0;

	if (nglScratchVertBufArray[CurVB]->IsBusy())
	{
		if (++CurVB >= NGL_SCRATCH_VB_COUNT)
			CurVB = 0;
	}

	// Lock the entire scratch meshes' vertex buffer (supposed faster than only locking the used portion).
	nglScratchVertBufArray[CurVB]->Lock(0, 0, (BYTE **) &nglScratchVertBufPtr, 0);
	nglScratchVertBuf = nglScratchVertBufArray[CurVB];

	nglScratch = 0;
	nglScratchTotalVertCount = 0;
	nglScratchPrimInfoPtr = 0;
}

/*---------------------------------------------------------------------------------------------------------
  Quad API.

  Quads are 2D interface graphics that can be drawn on top of the scene.  Construct nglQuad structures
  using the following functions and pass them to nglListAddQuad.
---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Set a quad to default values.
-----------------------------------------------------------------------------*/
void nglInitQuad(nglQuad *Quad)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	memset(Quad, 0, sizeof(nglQuad));

	Quad->RenderQuadFunc = nglRenderSingleQuad;
	Quad->VSHandle = nglVSQuad.Handle;

	Quad->Verts[0].Color = NGL_RGBA32(0xFF, 0xFF, 0xFF, 0xFF);
	Quad->Verts[1].Color = NGL_RGBA32(0xFF, 0xFF, 0xFF, 0xFF);
	Quad->Verts[2].Color = NGL_RGBA32(0xFF, 0xFF, 0xFF, 0xFF);
	Quad->Verts[3].Color = NGL_RGBA32(0xFF, 0xFF, 0xFF, 0xFF);

	Quad->Verts[0].U = 0.0f;
	Quad->Verts[1].U = 1.0f;
	Quad->Verts[2].U = 0.0f;
	Quad->Verts[3].U = 1.0f;

	Quad->Verts[0].V = 0.0f;
	Quad->Verts[1].V = 0.0f;
	Quad->Verts[2].V = 1.0f;
	Quad->Verts[3].V = 1.0f;

	Quad->MapFlags = NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER | NGLMAP_MIPMAP_LINEAR;
	Quad->BlendMode = NGLBM_BLEND;
}

/*-----------------------------------------------------------------------------
Description: Set quad's texture.
-----------------------------------------------------------------------------*/
void nglSetQuadTex(nglQuad *Quad, nglTexture *Tex)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Tex = Tex;
}

/*-----------------------------------------------------------------------------
Description: Set the quad's map flags. See NGLMAP_xxx for more details.
-----------------------------------------------------------------------------*/
void nglSetQuadMapFlags(nglQuad *Quad, uint32 MapFlags)
{
	Quad->MapFlags = MapFlags;
}

/*-----------------------------------------------------------------------------
Description: Return the quad's map flags. See NGLMAP_xxx for more details.
-----------------------------------------------------------------------------*/
uint32 nglGetQuadMapFlags(nglQuad *Quad)
{
	return Quad->MapFlags;
}

/*-----------------------------------------------------------------------------
Description: Set the quad's blend mode.
-----------------------------------------------------------------------------*/
void nglSetQuadBlend(nglQuad* Quad, uint32 Blend, uint32 Constant)
{
	Quad->BlendMode = Blend;
	Quad->BlendModeConstant = Constant;
}

/*-----------------------------------------------------------------------------
Description: Set quad's UV coordinates.
-----------------------------------------------------------------------------*/
void nglSetQuadUV(nglQuad *Quad, float u1, float v1, float u2, float v2)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Verts[0].U = u1;
	Quad->Verts[0].V = v1;
	Quad->Verts[1].U = u2;
	Quad->Verts[1].V = v1;
	Quad->Verts[2].U = u1;
	Quad->Verts[2].V = v2;
	Quad->Verts[3].U = u2;
	Quad->Verts[3].V = v2;
}

/*-----------------------------------------------------------------------------
Description: Set quad's color (packed integer ARGB).
-----------------------------------------------------------------------------*/
void nglSetQuadColor(nglQuad *Quad, uint32 c)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Verts[0].Color = c;
	Quad->Verts[1].Color = c;
	Quad->Verts[2].Color = c;
	Quad->Verts[3].Color = c;
}

/*-----------------------------------------------------------------------------
Description: Set quad's position.
-----------------------------------------------------------------------------*/
void nglSetQuadPos(nglQuad *Quad, float x, float y)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	float w, h;

	if (Quad->Tex)
	{
		w = (float) Quad->Tex->Width;
		h = (float) Quad->Tex->Height;
	}
	else
		w = h = 50;

	nglSetQuadR(Quad, x, y, x + w, y + h);
}

/*-----------------------------------------------------------------------------
Description: Set quad's rectangle (position + size).
-----------------------------------------------------------------------------*/
void nglSetQuadRect(nglQuad *Quad, float x1, float y1, float x2, float y2)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Verts[0].X = x1;
	Quad->Verts[0].Y = y1;
	Quad->Verts[1].X = x2;
	Quad->Verts[1].Y = y1;
	Quad->Verts[2].X = x1;
	Quad->Verts[2].Y = y2;
	Quad->Verts[3].X = x2;
	Quad->Verts[3].Y = y2;
}

/*-----------------------------------------------------------------------------
Description: Set the quad's Z value.
             The valid range is 0.0 (back) to 1.0 (front).
-----------------------------------------------------------------------------*/
void nglSetQuadZ(nglQuad *Quad, float z)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Z = z;
}

/*-----------------------------------------------------------------------------
Description: Set the vertex position at the specified index (0..3).
NOTE:        Verts go in this order: Top Left, Top Right, Bottom Left,
             Bottom Right.
-----------------------------------------------------------------------------*/
void nglSetQuadVPos(nglQuad *Quad, int32 VertIdx, float x, float y)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Verts[VertIdx].X = x;
	Quad->Verts[VertIdx].Y = y;
}

/*-----------------------------------------------------------------------------
Description: Set the vertex UVs at the specified index (0..3).
-----------------------------------------------------------------------------*/
void nglSetQuadVUV(nglQuad *Quad, int32 VertIdx, float u, float v)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Verts[VertIdx].U = u;
	Quad->Verts[VertIdx].V = v;
}

/*-----------------------------------------------------------------------------
Description: Set the vertex color at the specified index (0..3).
-----------------------------------------------------------------------------*/
void nglSetQuadVColor(nglQuad *Quad, int32 VertIdx, uint32 Color)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	Quad->Verts[VertIdx].Color = Color;
}

/*-----------------------------------------------------------------------------
Description: Rotate the quad counterclockwise around (cx,cy) by theta radians.
-----------------------------------------------------------------------------*/
void nglRotateQuad(nglQuad *Quad, float cx, float cy, float theta)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	for (int32 i = 0; i < 4; i++)
	{
		nglQuadVertex *Vert = &Quad->Verts[i];
		float x = Vert->X - cx;
		float y = Vert->Y - cy;
		Vert->X = x * cosf(theta) - y * sinf(theta) + cx;
		Vert->Y = y * cosf(theta) + x * sinf(theta) + cy;
	}
}

/*-----------------------------------------------------------------------------
Description: Squale a quad using the specified x and y scaling values.
-----------------------------------------------------------------------------*/
void nglScaleQuad(nglQuad *Quad, float cx, float cy, float sx, float sy)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

	for (int32 i = 0; i < 4; i++)
	{
		nglQuadVertex *Vert = &Quad->Verts[i];
		float x = Vert->X - cx;
		float y = Vert->Y - cy;
		Vert->X = x * sx + cx;
		Vert->Y = y * sy + cy;
	}
}

/*-----------------------------------------------------------------------------
Description: Render a quad's node.
-----------------------------------------------------------------------------*/
void nglRenderSingleQuad(void *Data)
{
	NGL_ASSERT(Data, "Data is NULL !");

	nglQuad *Quad = (nglQuad *) Data;

#if NGL_DEBUG
	if (nglDebug.DispMat && (nglCurScene->RenderTarget->Format & NGLTF_LINEAR))
	{
		NGL_D3DTRY(nglDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xA0A0A0A0, 1.0f, 0));
	}
#endif

	// Disable backface culling.
	if (nglPrevBackFaceCull)
	{
		nglDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		nglPrevBackFaceCull = 0;
	}

	// Only enable first texture stage.
	nglDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	nglDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Set the texture address mode.
	if (Quad->MapFlags & NGLMAP_CLAMP_U)
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	else
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);

	if (Quad->MapFlags & NGLMAP_CLAMP_V)
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	else
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		
	// Set the vertex shader.
	nglSetVertexShader(nglVSQuad.Handle);

	// Set the blending mode with the framebuffer.
	nglSetBlendingMode(Quad->BlendMode, Quad->BlendModeConstant);

#ifdef PROJECT_KELLYSLATER
	for (u_int i = 0; i < 4; ++i)
	{
		Quad->Verts[i].Color = NGL_RGBA32(
			(Quad->Verts[i].Color & 0x00ff0000) >> 17, 
			(Quad->Verts[i].Color & 0x0000ff00) >> 9, 
			(Quad->Verts[i].Color & 0x000000ff) >> 1, 
			(Quad->Verts[i].Color & 0xff000000) >> 24
		);
	}
#endif

	// Set the texture and the pixel shader.
	if (!Quad->Tex)
	{
		nglDev->SetTexture(0, NULL);

		// Set the pixel shader.
		nglSetPixelShader(nglPixelShaderHandle[NGL_PS_NOTEX]);
	}
	else
	{
		nglTexture *T = Quad->Tex;
		// Support of animated textures (IFL).
		if (Quad->Tex->Type == NGLTEX_IFL)
			T = Quad->Tex->Frames[nglTextureAnimFrame % Quad->Tex->NFrames];

		// Simple and Cube textures have the same address.
		nglDev->SetTexture(0, (IDirect3DTexture8 *) T->DXTexture.Simple);

		// Set the texture filtering type.
		if (Quad->MapFlags & NGLMAP_BILINEAR_FILTER)
		{
			nglDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			nglDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		}
		else
		{
			nglDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
			nglDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
		}

		// Set the mipmap filtering type.
		if (Quad->MapFlags & NGLMAP_MIPMAP_LINEAR)
		{
			nglDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		}
		else if (Quad->MapFlags & NGLMAP_MIPMAP_POINT)
		{
			nglDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		}
		else
		{
			nglDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
		}

		// UVs scaling (by texture size) is required for linear textures.
		if (Quad->Tex->Format & NGLTF_LINEAR)
		{
			uint32 sx = Quad->Tex->Width;
			uint32 sy = Quad->Tex->Height;

			Quad->Verts[1].U *= sx;
			Quad->Verts[2].V *= sy;
			Quad->Verts[3].U *= sx;
			Quad->Verts[3].V *= sy;
		}

		// Set the pixel shader.
		nglSetPixelShader(nglPixelShaderHandle[NGL_PS_T]);
	}

	// Clamp the QuadZ value to [NearZ,FarZ] range.
	float Z = Quad->Z;

	if (Z < nglCurScene->NearZ)
		Z = nglCurScene->NearZ;

	if (Z > nglCurScene->FarZ)
		Z = nglCurScene->FarZ;

	float QuadZ = ((nglCurScene->ZMin * NGL_Z_MAX) + ((nglCurScene->ZMax - nglCurScene->ZMin) * NGL_Z_MAX) *
	               (Z - nglCurScene->FarZ)) / (nglCurScene->NearZ - nglCurScene->FarZ);
	QuadZ = (float) NGL_Z_MAX - QuadZ;

	D3DCOLOR Color = Quad->Verts[0].Color;

#if NGL_QUADS_USE_PUSHBUFFER
	// Use pushbuffer optimization instead of DrawPrimitiveUP() call.
	const uint32 NVerts = 4;
	uint32 PushBufferSize = (sizeof(nglVertexQuad) * NVerts) / sizeof(DWORD);

	// Gain direct access to the pushbuffer. Note the "+5", which is
	// to reserve overhead for the encoding parameters.
	DWORD *PB;
	nglDev->BeginPush(PushBufferSize + 5, &PB);

	// Push the macro that start things off.
	PB[0] = D3DPUSH_ENCODE(D3DPUSH_SET_BEGIN_END, 1);

	// Specify the primitive type of the vertices that follow.
	PB[1] = D3DPT_QUADLIST;

	// Specify that an array of vertices comes next. Note that a max
	// of 2047 dwords can be specified in a INLINE_ARRAY section.
	// For handling more vertex data than that, simply split the data
	// into multiple INLINE_ARRAY sections.
	PB[2] =
		D3DPUSH_ENCODE(D3DPUSH_NOINCREMENT_FLAG | D3DPUSH_INLINE_ARRAY, PushBufferSize);
	PB += 3;

	// Write the position, color, UVs.
	// Vertex 0.
	*((XGVECTOR3 *) PB) =
		XGVECTOR3(Quad->Verts[0].X, Quad->Verts[0].Y, QuadZ);
	PB += 3;
	*((D3DCOLOR *) PB) = Color;
	PB++;
	*((XGVECTOR2 *) PB) = XGVECTOR2(Quad->Verts[0].U, Quad->Verts[0].V);
	PB += 2;

	// Vertex 1.
	*((XGVECTOR3 *) PB) =
		XGVECTOR3(Quad->Verts[1].X, Quad->Verts[1].Y, QuadZ);
	PB += 3;
	*((D3DCOLOR *) PB) = Color;
	PB++;
	*((XGVECTOR2 *) PB) = XGVECTOR2(Quad->Verts[1].U, Quad->Verts[1].V);
	PB += 2;

	// Vertex 2.
	*((XGVECTOR3 *) PB) =
		XGVECTOR3(Quad->Verts[3].X, Quad->Verts[3].Y, QuadZ);
	PB += 3;
	*((D3DCOLOR *) PB) = Color;
	PB++;
	*((XGVECTOR2 *) PB) = XGVECTOR2(Quad->Verts[3].U, Quad->Verts[3].V);
	PB += 2;

	// Vertex 3.
	*((XGVECTOR3 *) PB) =
		XGVECTOR3(Quad->Verts[2].X, Quad->Verts[2].Y, QuadZ);
	PB += 3;
	*((D3DCOLOR *) PB) = Color;
	PB++;
	*((XGVECTOR2 *) PB) = XGVECTOR2(Quad->Verts[2].U, Quad->Verts[2].V);
	PB += 2;

	// Push the macros that finish things off.
	PB[0] = D3DPUSH_ENCODE(D3DPUSH_SET_BEGIN_END, 1);
	PB[1] = 0;
	PB += 2;

	nglDev->EndPush(PB);
#else
	// DrawPrimitiveUP() version (in case if pushbuffer optimization is buggy).
	nglVertexQuad q[4];

	q[0].Color = q[1].Color = q[2].Color = q[3].Color = Color;
	q[0].Pos.z = q[1].Pos.z = q[2].Pos.z = q[3].Pos.z = QuadZ;

	// Between XBox and PS2, vertex 3 and 4 have to be swapped. 
	q[0].Pos.x = Quad->Verts[0].X;
	q[0].Pos.y = Quad->Verts[0].Y;
	q[1].Pos.x = Quad->Verts[1].X;
	q[1].Pos.y = Quad->Verts[1].Y;
	q[2].Pos.x = Quad->Verts[3].X;
	q[2].Pos.y = Quad->Verts[3].Y;
	q[3].Pos.x = Quad->Verts[2].X;
	q[3].Pos.y = Quad->Verts[2].Y;

	q[0].UV.x = Quad->Verts[0].U;
	q[0].UV.y = Quad->Verts[0].V;
	q[1].UV.x = Quad->Verts[1].U;
	q[1].UV.y = Quad->Verts[1].V;
	q[2].UV.x = Quad->Verts[3].U;
	q[2].UV.y = Quad->Verts[3].V;
	q[3].UV.x = Quad->Verts[2].U;
	q[3].UV.y = Quad->Verts[2].V;

	NGL_D3DTRY(nglDev->DrawPrimitiveUP(D3DPT_QUADLIST, 1, &q, sizeof(nglVertexQuad)));
#endif // NGL_QUADS_USE_PUSHBUFFER

#if NGL_DEBUG
	if (nglDebug.DispMat && (nglCurScene->RenderTarget->Format & NGLTF_LINEAR))
	{
		NGL_D3DTRY(nglDev->Present(NULL, NULL, NULL, NULL));

		if (nglDebug.InfoPrints)
			nglInfo("Section %d (quad)\n", nglSectionNumber++);
	}
#endif
}

/*-----------------------------------------------------------------------------
Description: Add a quad to the render list.
-----------------------------------------------------------------------------*/
void nglListAddQuad(nglQuad *Quad)
{
	NGL_ASSERT(Quad, "Quad is NULL !");

#if NGL_DEBUG
	if (!nglStage.Quads)
		return;
#endif

	nglQuad *Entry = nglListNew(nglQuad);
	memcpy(Entry, Quad, sizeof(nglQuad));

	// Add the quad to the render list.
	nglSortInfo SortInfo;
	nglGetSortInfo(&SortInfo, Entry->BlendMode, Quad->VSHandle, Entry->Z);
	nglListAddNode(NGLNODE_QUAD, Quad->RenderQuadFunc, Entry, &SortInfo);
}

/*---------------------------------------------------------------------------------------------------------
  Font API.

  Mostly taken from Sean Palmer's font system.
---------------------------------------------------------------------------------------------------------*/

#define NGLFONT_VERSION       0x102

#define NGLFONT_TOKEN_COLOR    '\1'
#define NGLFONT_TOKEN_SCALE    '\2'
#define NGLFONT_TOKEN_SCALEXY  '\3'

struct nglGlyphInfo
{
	uint32 TexOfs[2];       // Offset in texture of topleft of glyph, in pixels.
	uint32 GlyphSize[2];    // Pixels size of glyph.
	int32 GlyphOrigin[2];   // Pixels offset from cp to glyph origin.
	uint32 CellWidth;       // Pixels to advance cp.
};

struct nglFontHeader
{
	uint32 Version;
	uint32 CellHeight;
	uint32 Ascent;
	uint32 FirstGlyph;
	uint32 NumGlyphs;
};

struct nglFont
{
	nglTexture *Tex;
	nglGlyphInfo *GlyphInfo;
	nglFontHeader Header;

	// Map flags. See NGLMAP_xxx for more details.
	uint32 MapFlags;

	uint32 BlendMode;
	uint32 BlendModeConstant;
};

/*-----------------------------------------------------------------------------
Description: Return the integer value following the specified token.
             For example, if data contains: "posx 320 posy 200 color 128",
             the call nglGetTokenUINT(data, "posy", 10) will returns 200.
-----------------------------------------------------------------------------*/
uint32 nglGetTokenUINT(char *&Data, char *Token, uint32 Base)
{
	uint32 TokenLength = strlen(Token);

	for (;;)
	{
		uint32 CharCount = 0;

		// Locate the beginning of the token.
		for (uint32 i = 0; i < TokenLength; i++)
		{
			if (Data[i] != Token[i])
				break;
			else
				CharCount++;
		}

		// Token found. Read the string and convert it to an integer value.
		if (CharCount == TokenLength)
		{
			// Skip the token we just recognized.
			Data += TokenLength;
			// String to integer conversion.
			char *StopString;
			uint32 Val = strtol(Data, &StopString, Base);
			Data = StopString;
			return Val;
		}
		else
		{
			Data++;
		}
	}
}

/*-----------------------------------------------------------------------------
Description: Return the integer value pointed by data.
-----------------------------------------------------------------------------*/
uint32 nglGetUINT(char *&Data, uint32 Base)
{
	// Move to the first digit.
	while (*Data < '0' || *Data > '9')
		Data++;

	// String to integer conversion.
	char *StopString;
	uint32 Val = strtol(Data, &StopString, Base);
	Data = StopString;
	return Val;
}

/*-----------------------------------------------------------------------------
Description: Parse the FDF file and initialize the font header and the font's
             glyphs.
-----------------------------------------------------------------------------*/
void nglParseFDF(char *Data, nglFont *Font)
{
	Font->Header.Version = nglGetTokenUINT(Data, "version", 16);
	NGL_ASSERT(Font->Header.Version == NGLFONT_VERSION, "Unsupported font version !");

	Font->Header.CellHeight = nglGetTokenUINT(Data, "cellheight", 10);
	NGL_ASSERT(Font->Header.CellHeight, "");

	Font->Header.Ascent = nglGetTokenUINT(Data, "ascent", 10);
	NGL_ASSERT(Font->Header.Ascent, "");

	Font->Header.FirstGlyph = nglGetTokenUINT(Data, "firstglyph", 10);
	NGL_ASSERT(Font->Header.FirstGlyph, "");

	Font->Header.NumGlyphs = nglGetTokenUINT(Data, "numglyphs", 10);
	NGL_ASSERT(Font->Header.NumGlyphs > 0, "");

	Font->GlyphInfo =
		(nglGlyphInfo *) nglMemAlloc(sizeof(nglGlyphInfo) *
									 Font->Header.NumGlyphs);

	for (uint32 i = 0; i < Font->Header.NumGlyphs; i++)
	{
		nglGlyphInfo &gi = Font->GlyphInfo[i];
		uint32 gidx = nglGetUINT(Data, 10);
		NGL_ASSERT(gidx == i + Font->Header.FirstGlyph, "");
		gi.TexOfs[0] = nglGetTokenUINT(Data, "x", 10);
		gi.TexOfs[1] = nglGetTokenUINT(Data, "y", 10);
		gi.CellWidth = nglGetTokenUINT(Data, "w", 10);
		NGL_ASSERT(gi.CellWidth, "");
		gi.GlyphOrigin[0] = nglGetTokenUINT(Data, "gx", 10);
		gi.GlyphOrigin[1] = nglGetTokenUINT(Data, "gy", 10);
		gi.GlyphSize[0] = nglGetTokenUINT(Data, "gw", 10);
		gi.GlyphSize[1] = nglGetTokenUINT(Data, "gh", 10);
	}
}

/*-----------------------------------------------------------------------------
Description: Initialize a font with default values.
-----------------------------------------------------------------------------*/
void nglInitFont(nglFont *Font)
{
	memset(Font, 0, sizeof(nglFont));

	Font->MapFlags = NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER | NGLMAP_MIPMAP_LINEAR;
	Font->BlendMode = NGLBM_BLEND;
}

/*-----------------------------------------------------------------------------
Description: Load a font (from a .fdf file) and return a pointer to it.
             The texture font must be loaded before calling this function.
-----------------------------------------------------------------------------*/
nglFont *nglLoadFont(const nglFixedString &FontName)
{
	// Search for an existing copy.
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglFontBank.Search(FontName)))
	{
		Inst->RefCount++;
		return (nglFont *) Inst->Value;
	}

	nglFont *Font = (nglFont *) nglMemAlloc(sizeof(nglFont));
	nglInitFont(Font);

	// Get the font's texture. If it fails, the function returns.
	Font->Tex = nglGetTexture(FontName);
	if (!Font->Tex)
	{
		nglError("nglLoadFont(): failed to get the texture %s !\n", FontName.c_str());
		return NULL;
	}

	// Load the font's description file (fdf). If it fails, the function returns.
	// Look into the texture path.
	char FileName[NGL_MAX_PATH_LENGTH];
	strcpy(FileName, nglTexturePath);
	strcat(FileName, FontName.c_str());
	strcat(FileName, ".fdf");

	nglFileBuf FileBuf;
	if (!nglReadFile(FileName, &FileBuf, 0))
		return NULL;

	// Parse the fdf file.
	char *Data = (char *) FileBuf.Buf;

	nglParseFDF(Data, Font);

	// Release the fdf file.
	nglReleaseFile(&FileBuf);

	nglFontBank.Insert(Font->Tex->FileName, Font);

	return Font;
}

/*-----------------------------------------------------------------------------
Description: Load a font from memory (must point to a fdf file) and return a
             pointer to the loaded font.
             The texture font must be loaded before calling this function.
-----------------------------------------------------------------------------*/
nglFont *nglLoadFontInPlace(const nglFixedString &FontName, void *FDFdata)
{
	// Search for an existing copy.
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglFontBank.Search(FontName)))
	{
		Inst->RefCount++;
		return (nglFont *) Inst->Value;
	}

	nglFont *Font = (nglFont *) nglMemAlloc(sizeof(nglFont));
	nglInitFont(Font);

	// Get the font's texture. If it fails, the function returns.
	Font->Tex = nglGetTexture(FontName);
	if (!Font->Tex)
	{
		nglError("nglLoadFont(): failed to get the texture %s !\n", FontName.c_str());
		return NULL;
	}

	// Parse the fdf file.
	nglParseFDF((char *) FDFdata, Font);

	nglFontBank.Insert(Font->Tex->FileName, Font);

	return Font;
}

/*-----------------------------------------------------------------------------
Description: Release a font from memory.
-----------------------------------------------------------------------------*/
void nglReleaseFont(nglFont *Font)
{
	NGL_ASSERT(Font, "Trying to release a NULL font !");

	// Remove a reference from the instance bank, delete the font only if the count hits 0.
	if (nglFontBank.Delete(Font->Tex->FileName))
		return;

	// Release the glyph infos and the font.
	nglMemFree(Font->GlyphInfo);
	nglMemFree(Font);
	Font = NULL;
}

/*-----------------------------------------------------------------------------
Description: Release all the fonts from memory.
-----------------------------------------------------------------------------*/
void nglReleaseAllFonts()
{
	nglInstanceBank::Instance *Inst = nglFontBank.Head->Forward[0];

	while (Inst != nglFontBank.NIL)
	{
		Inst->RefCount = 1;
		nglReleaseFont((nglFont *) Inst->Value);
		Inst = nglFontBank.Head->Forward[0];
	}
}

// Set the font's blend mode.
void nglSetFontBlendMode(nglFont* Font, u_int BlendMode, u_int Constant)
{
	Font->BlendMode = BlendMode;
	Font->BlendModeConstant = Constant;
}

/*-----------------------------------------------------------------------------
Description: Setup the font's mesures.
-----------------------------------------------------------------------------*/
void nglSetFontMeasures(nglFont *Font, float *aoffs, float *asize, float *auvpos,
						float *auvsize, float scalex, float scaley, const nglGlyphInfo &ginfo)
{
	float InvW = 1.0f / (float)Font->Tex->Width;
	float InvH = 1.0f / (float)Font->Tex->Height;

	aoffs[0] = (float)ginfo.GlyphOrigin[0] * scalex;
	aoffs[1] = (float)ginfo.GlyphOrigin[1] * scaley;
	asize[0] = (float)(ginfo.GlyphSize[0]) * scalex;
	asize[1] = (float)(ginfo.GlyphSize[1]) * scaley;

	auvpos[0] = (float)(ginfo.TexOfs[0] - 1) * InvW;
	auvpos[1] = (float)(ginfo.TexOfs[1] - 1) * InvH;
	auvsize[0] = (float)(ginfo.GlyphSize[0]) * InvW;
	auvsize[1] = (float)(ginfo.GlyphSize[1]) * InvH;
}

/*-----------------------------------------------------------------------------
Description: Set the font's map flags. See NGLMAP_xxx for more details.
-----------------------------------------------------------------------------*/
void nglSetFontMapFlags(nglFont *Font, uint32 MapFlags)
{
	Font->MapFlags = MapFlags;
}

/*-----------------------------------------------------------------------------
Description: Return the font's map flags. See NGLMAP_xxx for more details.
-----------------------------------------------------------------------------*/
uint32 nglGetFontMapFlags(nglFont *Font)
{
	return Font->MapFlags;
}

/*-----------------------------------------------------------------------------
Description: Set the font's blend mode.
-----------------------------------------------------------------------------*/
void nglSetFontBlend(nglFont* Font, uint32 Blend, uint32 Constant)
{
	Font->BlendMode = Blend;
	Font->BlendModeConstant = Constant;
}

/*-----------------------------------------------------------------------------
Description: Render a single font using the Quad API.
-----------------------------------------------------------------------------*/
void nglRenderSingleCharacter(nglFont *Font, uint32 Color, float *Pos, float *Size, float *UVPos, float *UVSize)
{
	nglQuad q;
	nglInitQuad(&q);
	nglSetQuadZ(&q, Pos[2]);

	float x_ratio = 1.0f, y_ratio = 1.0f;

	float left = (Pos[0]) * x_ratio,
	      top = (Pos[1]) * y_ratio,
	      right = (Pos[0] + Size[0]) * x_ratio,
	      bottom = (Pos[1] + Size[1]) * y_ratio;

	nglSetQuadRect(&q, left, top, right, bottom);
	nglSetQuadUV(&q, UVPos[0], UVPos[1], UVPos[0] + UVSize[0], UVPos[1] + UVSize[1]);
	nglSetQuadColor(&q, Color);
	nglSetQuadBlend(&q, Font->BlendMode, Font->BlendModeConstant);
	nglSetQuadMapFlags(&q, Font->MapFlags);
	nglSetQuadTex(&q, Font->Tex ? Font->Tex : NULL);
	nglListAddQuad(&q);
}

/*-----------------------------------------------------------------------------
Description: Add a string to the nodes' list to be rendered.
             The string may contains some formatting informations:
             - Font color is set by the token (hexa): \1[RRGGBBAA]
             - Font scale is set by the token: \2[SCALE]
			 - Font independent scale is set by the token: \3[SCALEX,SCALEY]
             The default color is the Color argument and the default scale is 1.
             For example:
             "Hi from \1[ff0000ff]NGL \1[ffffffff] :-)\n\n\2[2.0]Bye!");
             Writes "NGL" in red and "Bye" with a scale of 2.0.
-----------------------------------------------------------------------------*/
void nglListAddStringFixed(nglFont *Font, float x, float y, float z, uint32 Color, const char *Text)
{
	float curpos[3] = { x, y, z };
	float offs[2];
	float size[2];
	float uvpos[2];
	float uvsize[2];

	const char *TextPtr = Text;
	char c;

	float ScaleX = 1.0f;
	float ScaleY = 1.0f;
	float CurMaxScaleY = ScaleY;

	while (c = *TextPtr)
	{
		// Check for color token: \1[RRGGBBAA] or \1[0xRRGGBBAA]
		if (c == NGLFONT_TOKEN_COLOR)
		{
			TextPtr += 2;
			// String to integer (hexa) conversion.
			char *StopString;
			uint32 ColorRGBA = strtoul(TextPtr, &StopString, 16);
			Color = RGBA2ARGB(ColorRGBA);
			TextPtr = StopString + 1;
			continue;
		}
		// Check for scale token: \2[SCALE]
		else if (c == NGLFONT_TOKEN_SCALE)
		{
			TextPtr += 2;
			// String to float conversion.
			char *StopString;
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			ScaleX = ScaleY;
			TextPtr = StopString + 1;
			
			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;
			
			continue;
		}
		// Check for independent scale token: \3[SCALEX,SCALEY]
		else if (c == NGLFONT_TOKEN_SCALEXY)
		{
			TextPtr += 2;
			// String to float conversion (SCALEX).
			char *StopString;
			ScaleX = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;
			
			// String to float conversion (SCALEY).
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;
			
			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;
			
			continue;
		}

		if (c == '\n')
		{
			curpos[0] = x;
			curpos[1] += Font->Header.CellHeight * CurMaxScaleY;
			CurMaxScaleY = ScaleY;
		}
		else
		{
			const nglGlyphInfo &ginfo = Font->GlyphInfo[(uint8) c - Font->Header.FirstGlyph];

			if (c != ' ')
			{
				nglSetFontMeasures(Font, offs, size, uvpos, uvsize, ScaleX, ScaleY, ginfo);
				float gpos[3] =	{ curpos[0] + offs[0], curpos[1] + offs[1], curpos[2] };
				nglRenderSingleCharacter(Font, Color, gpos, size, uvpos, uvsize);
			}

			curpos[0] += ginfo.CellWidth * ScaleX;
		}

		TextPtr++;
	}
}

/*-----------------------------------------------------------------------------
Description: Add a string to the nodes' list to be rendered.
-----------------------------------------------------------------------------*/
void nglListAddString(nglFont *Font, float x, float y, float z, const char *Text, ...)
{
	if (Text == NULL || *Text == 0)
		return;

	char nglWork[1024];

	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);

	nglListAddStringFixed(Font, x, y, z, NGL_RGBA32(0xFF, 0xFF, 0xFF, 0xFF), nglWork);
}

/*-----------------------------------------------------------------------------
Description: nglListAddString variant where the color is specified as an input
             parameter.
-----------------------------------------------------------------------------*/
void nglListAddString(nglFont *Font, float x, float y, float z, uint32 Color, const char *Text, ...)
{
	if (Text == NULL || *Text == 0)
		return;

	char nglWork[1024];

	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);

	nglListAddStringFixed(Font, x, y, z, Color, nglWork);
}

/*-----------------------------------------------------------------------------
Description: Get the dimension, in pixels, of a string (which can have scale
             tokens and cariage returns as well).
-----------------------------------------------------------------------------*/
void nglGetStringDimensions(nglFont *Font, uint32 *Width, uint32 *Height, const char *Text, ...)
{
	if (Text == NULL || *Text == 0)
		return;

	char nglWork[1024];
	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);

	const char *TextPtr = &nglWork[0];
	char c;

	float ScaleX = 1.0f;
	float ScaleY = 1.0f;
	float CurMaxScaleY = ScaleY;
	float CurMaxWidth = 0.0f;
	float fWidth = 0.0f;
	float fHeight = Font->Header.CellHeight * ScaleY;

	while (c = *TextPtr)
	{
		// Check for color token: \1[RRGGBBAA] or \1[0xRRGGBBAA]
		if (c == NGLFONT_TOKEN_COLOR)
		{
			TextPtr += 2;
			// String to integer (hexa) conversion.
			char *StopString;
			strtoul(TextPtr, &StopString, 16);
			TextPtr = StopString + 1;
			continue;
		}
		// Check for scale token: \2[SCALE]
		else if (c == NGLFONT_TOKEN_SCALE)
		{
			TextPtr += 2;
			// String to float conversion.
			char *StopString;
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			ScaleX = ScaleY;
			TextPtr = StopString + 1;
			
			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;
			
			continue;
		}
		// Check for independent scale token: \3[SCALEX,SCALEY]
		else if (c == NGLFONT_TOKEN_SCALEXY)
		{
			TextPtr += 2;
			// String to float conversion (SCALEX).
			char *StopString;
			ScaleX = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;
			
			// String to float conversion (SCALEY).
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;
			
			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;
			
			continue;
		}

		if (c == '\n')
		{
			// Store the current max width.
			if (fWidth > CurMaxWidth)
				CurMaxWidth = fWidth;
			fWidth = 0.0f;
			fHeight += Font->Header.CellHeight * CurMaxScaleY;
			CurMaxScaleY = ScaleY;
		}
		else
		{
			const nglGlyphInfo &ginfo = Font->GlyphInfo[(uint8) c - Font->Header.FirstGlyph];
			fWidth += ginfo.CellWidth * ScaleX;
		}

		TextPtr++;
	}

	const nglGlyphInfo &ginfo =	Font->GlyphInfo[' ' - Font->Header.FirstGlyph];

	if (fWidth > CurMaxWidth)
		CurMaxWidth = fWidth;

	*Width = nglFTOI(CurMaxWidth + ginfo.CellWidth * ScaleX);
	*Height = nglFTOI(fHeight);
}

// LEGACY INTERFACE - to be removed!
nglFont* nglLegacyFont = NULL;
uint32 nglLegacyFontColor = NGL_RGBA32(0xFF, 0xFF, 0xFF, 0xFF);
float nglLegacyFontZ = 0.0f;

void nglSetFont(nglFont* Font)  { nglLegacyFont = Font; }
void nglSetFontColor(uint32 c)  { nglLegacyFontColor = c; }
void nglSetFontZ(float z)       { nglLegacyFontZ = z; }
void nglSetFontScale(float xscale, float yscale) {}
void nglListAddString(float x, float y, const char* Text, ...)
{
	char nglWork[1024];
	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);
	nglListAddString(nglLegacyFont ? nglLegacyFont : nglSysFont, x, y, nglLegacyFontZ, nglLegacyFontColor, nglWork);
}

void nglGetStringDimensions(int* Width, int* Height, char* Text, ...)
{
	char nglWork[1024];
	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);
	nglGetStringDimensions(nglLegacyFont ? nglLegacyFont : nglSysFont, (uint32*)Width, (uint32*)Height, nglWork);
}
// END LEGACY INTERFACE

/*---------------------------------------------------------------------------------------------------------
  Vertex/Pixel Shaders API.


---------------------------------------------------------------------------------------------------------*/

#include "ngl_VertexShaders.h"

/*-----------------------------------------------------------------------------
Description: Clear the memory used by all the vertex shaders that have been
             compiled/created.
-----------------------------------------------------------------------------*/
void nglClearVShaderDB()
{
	nglError("Vertex shaders database overflow ! Increase NGL_VS_DB_SIZE.\n");

	// Release the memory used by the vertex shaders.
	for (uint32 i = 0; i < NGL_VS_DB_SIZE; i++)
	{
		nglVSDBEntry *Entry = &nglVSDB[i];
		if (Entry->Handle)
			nglDev->DeleteVertexShader(Entry->Handle);
	}
	// Clear the vshaders hash table and vs database.
	memset(nglVSHashTable, 0, sizeof(nglVSHashTable));
	memset(nglVSDB, 0, sizeof(nglVSDB));
	nglVSDBPtr = 0;

	// Clear the vs opcodes as well.
	memset(nglVSOpcodePool, 0, sizeof(nglVSOpcodePool));
	nglVSOpcodePoolPtr = 0;
}

/*-----------------------------------------------------------------------------
Description: Compile and create a vertex shader.
             Unfortunately, there is now way to tell XGAssembleShader() not to
             dynamically creates a memory buffer which is used to store the VS
             opcode. So, when UseOpcodePool = true, vshader opcodes are copied
             into a static array of memory (nglVSOpcodePool) and the memory
             previously allocated by XGAssembleShader() is freed.
             When UseOpcodePool = false, the caller must provide a buffer which
             is used to store the vshader opcode.
-----------------------------------------------------------------------------*/
bool nglCompileVertexShader(const DWORD *decl, DWORD &handle,
							bool UseOpcodePool = true, LPXGBUFFER *Opcode =
							NULL)
{
	if (UseOpcodePool)
	{
		LPXGBUFFER OpcodeBuf;

    if (XGAssembleShader("", nglVSSrc, strlen(nglVSSrc), 
#if defined(PROJECT_KELLYSLATER) &&	defined(NGL_DEBUG)
		SASM_DONOTOPTIMIZE, 
#else
		0, 
#endif
		NULL, &OpcodeBuf, NULL, NULL, NULL, NULL, NULL) != S_OK
	)
		nglFatal("Cannot compile vertex shader !");
	
		if (nglVSOpcodePoolPtr + OpcodeBuf->size >= NGL_VS_OPCODE_POOL)
		{
			nglError("Memory pool for vshader opcodes overflow ! Increase NGL_BYTES_PER_VSHADER.\n");
			return false;
		}

		memcpy(&nglVSOpcodePool[nglVSOpcodePoolPtr], OpcodeBuf->pData, OpcodeBuf->size);

		NGL_D3DTRY(nglDev->CreateVertexShader(decl, (DWORD *) & nglVSOpcodePool[nglVSOpcodePoolPtr], &handle, 0));

		nglVSOpcodePoolPtr += OpcodeBuf->size;

		OpcodeBuf->Release();

		if (!handle)
			nglFatal("Cannot create vertex shader !");
	}
	else
	{
		if (XGAssembleShader("", nglVSSrc, strlen(nglVSSrc), 0, NULL, Opcode, NULL, NULL, NULL, NULL, NULL) != S_OK)
			nglFatal("Cannot compile vertex shader !");

		NGL_D3DTRY(nglDev->CreateVertexShader(decl, (DWORD *) (*Opcode)->pData, &handle, 0));

		if (!handle)
			nglFatal("Cannot create vertex shader !");
	}

	return true;
}

/*-----------------------------------------------------------------------------
Description: Initialize the shaders constants.
-----------------------------------------------------------------------------*/
void nglInitShadersConst()
{
	// Reserve the vertex shaders registers to store the required parameters (bones, local to screen matrix, etc.).
	int32 reg = NGL_VS_FIRST_REG;

	// These 4 registers are only initialized once (here), so they can't be modified !
	nglVSC_Util = reg++;		// [ 0.0, 0.5, 1.0, 2.0 ]
	nglVSC_Util2 = reg++;		// [ 255.0001 * 3.0, nglVSC_Bones * 3, NGL_SHADOW_FADING, 0.0f ]
	nglVSC_Scale = reg++;		// [ 640.0, -480.0, 16777215.0, 1.0 ]
	nglVSC_Offset = reg++;		// [ 0.5, 0.5, 0.0, 0.0 ]
	NGL_VS_FIRST_FREE_REG = reg;

	nglVSC_LocalToScreen = reg;
	reg += 4;
	nglVSC_FogParams = reg++;
	nglVSC_UVParams = reg++;
	nglVSC_CameraPos = reg++;
	nglVSC_LocalToWorld = reg;
	reg += 4;
	nglVSC_Tint = reg++;
	nglVSC_MaterialColor = reg++;
	nglVSC_Bones = reg;
	reg += (NGL_VS_MAX_BONES * 3);	// Bones are using NGL_VS_MAX_BONES * 3 registers.

	for (uint32 i = 0; i < NGL_MAX_LIGHTS; i++)
	{
		memset(&nglVSC_Light[i], 0, sizeof(nglVSC_Light[0]));

		nglVSC_Light[i].Dir = reg++;
		nglVSC_Light[i].Col = reg++;

		// Point light not implemented.
		//nglVSC_Light[i].Near = reg++;
		//nglVSC_Light[i].Far = reg++;
	}

	if (reg > NGL_VS_LAST_REG)
		nglFatal("Too many vshader registers used %d ! (see nglInitShadersConst() and nglVSC_xxx variables)\n", reg);

	// Reserve the registers for the 2nd pass and next (used by projectors).
	reg = NGL_VS_FIRST_FREE_REG;

	nglVSC2_Projector_LocalToScreen = reg;
	reg += 4;
	nglVSC2_Projector_TexScale = reg++;

	for (i = 0; i < NGL_MAX_TS; i++)
	{
		nglVSC2_Projector[i].Local2UV = reg;
		reg += 2;
		nglVSC2_Projector[i].Dir = reg++;
	}

	NGL_VS_FIRST_USER_REG = reg; 

	if (reg > NGL_VS_LAST_REG)
		nglFatal("Too many vshader registers used %d ! (see nglInitShadersConst() and nglVSC_xxx variables)\n", reg);

	// Setup the constant registers' values.
	nglDev->SetVertexShaderConstant(nglVSC_Util, XGVECTOR4(0.0f, 0.5f, 1.0f, 2.0f), 1);
	sprintf(NGL_VSC_ZERO, "c[%d].x", nglVSC_Util);
	sprintf(NGL_VSC_HALF, "c[%d].y", nglVSC_Util);
	sprintf(NGL_VSC_ONE, "c[%d].z", nglVSC_Util);
	sprintf(NGL_VSC_TWO, "c[%d].w", nglVSC_Util);

	nglDev->SetVertexShaderConstant(nglVSC_Util2, XGVECTOR4(255.0001f * 3.0f, (float) nglVSC_Bones, NGL_SHADOW_FADING, 0.0f), 1);
	sprintf(NGL_VSC_BONES_SCALE, "c[%d].x", nglVSC_Util2);
	sprintf(NGL_VSC_BONES_OFS, "c[%d].y", nglVSC_Util2);
	sprintf(NGL_VSC_SHADOW_FADING, "c[%d].z", nglVSC_Util2);

	// These parameters are used to calculate the screen space scale and offset.
	// They depend upon the resolution and the super/multi sample mode as defined below:

	//                                                           Scale          Offset 
	//                                                          X     Y       X         Y 
	// D3DMULTISAMPLE_NONE                                     1.0 | 1.0 | 0.53125 | 0.53125 
	// D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR             1.0 | 1.0 | 0.03125 | 0.03125 
	// D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX           1.0 | 1.0 | 0.03125 | 0.03125 
	// D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_HORIZONTAL_LINEAR  2.0 | 1.0 | 0.53125 | 0.53125 
	// D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_VERTICAL_LINEAR    1.0 | 2.0 | 0.53125 | 0.53125 
	// D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR             1.0 | 1.0 | 0.03125 | 0.03125 
	// D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN           1.0 | 1.0 | 0.03125 | 0.03125 
	// D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_LINEAR             2.0 | 2.0 | 0.53125 | 0.53125 
	// D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_GAUSSIAN           2.0 | 2.0 | 0.53125 | 0.53125 
	// D3DMULTISAMPLE_9_SAMPLES_MULTISAMPLE_GAUSSIAN           1.5 | 1.5 | 0.03125 | 0.03125 
	// D3DMULTISAMPLE_9_SAMPLES_SUPERSAMPLE_GAUSSIAN           3.0 | 3.0 | 0.53125 | 0.53125 

  // MMmmhh... Currently setting it to 0.53125 0.53125 doesn't give the right scale/offset and
  // I noticed that 0.5 0.5 gives the correct result (?)....
#ifdef PROJECT_KELLYSLATER
  nglDev->SetVertexShaderConstant(nglVSC_Scale,  XGVECTOR4(nglGetScreenWidthTV() * 0.5f, -nglGetScreenHeightTV() * 0.5f, 16777215.0f, 1.0f), 1);
#else
  nglDev->SetVertexShaderConstant(nglVSC_Scale,  XGVECTOR4(nglGetScreenWidth() * 0.5f, -nglGetScreenHeight() * 0.5f, 16777215.0f, 1.0f), 1);
#endif
  nglDev->SetVertexShaderConstant(nglVSC_Offset, XGVECTOR4(nglGetScreenWidth() * 0.5f,  nglGetScreenHeight() * 0.5f, 0.0f, 0.0f), 1);
}

/*-----------------------------------------------------------------------------
Description: Create the static vertex shaders.
-----------------------------------------------------------------------------*/
void nglCreateStaticVShaders()
{
	// Compile and create the quads vertex shader.
	nglCreateVertexShader(nglVS_Quads_Decl, &nglVSQuad, false,
						  ";---- vsf_quad\n"
						  "mov oPos, v0\n"
						  "mov oD0,  v1\n"
						  "mov oT0,  v3\n" "mov oFog, %s\n",
						  NGL_VSC_ONE);

	// Compile and create the projector light vertex shaders for non skinned transform.
	void (*ProjectorDirVShader[4]) () =
	{
		nglvsf_1projector_dir,
		nglvsf_2projector_dir,
		nglvsf_3projector_dir, nglvsf_4projector_dir
	};

	for (uint32 i = 0; i < 4; i++)
	{
		nglvsf_begin();
		ProjectorDirVShader[i] ();
		nglvsf_nofog();
		nglvsf_end();

		nglCompileVertexShader(nglVS_NonSkin_Decl, nglVSDirProjector[i], false, &nglVSDirProjectorOpcode[i]);
	}

	// Compile and create the projector light vertex shader for skinned transform.
	nglvsf_begin();
	nglvsf_projector_dir_skin();
	nglvsf_nofog();
	nglvsf_end();

	nglCompileVertexShader(nglVS_Skin_Decl, nglVSDirProjectorSkin);
}

/*-----------------------------------------------------------------------------
Description: Initialize the vertex/pixel shaders.
-----------------------------------------------------------------------------*/
void nglInitShaders()
{
	// Clear the vshaders hash table and database.
	memset(nglVSHashTable, 0, sizeof(nglVSHashTable));
	memset(nglVSDB, 0, sizeof(nglVSDB));

	// Switch to 192 constant mode (192 constants are available for VS instead of 96).
	// Registers from -96 to 95 are available (instead of 0 to 95).
	// This is required for the vertex shaders code.
	// Furthermore, tell D3D not to reserve registers -37 and -38 for screen space transformation.
	NGL_D3DTRY(nglDev->SetShaderConstantMode(D3DSCM_192CONSTANTS | D3DSCM_NORESERVEDCONSTANTS));

	// Setup the vertex shaders global constants.
	nglInitShadersConst();

	// Compile and create the static vertex shaders.
	nglCreateStaticVShaders();

	// Create the pixel shaders.
	uint32 i;
	for (i = 0; i < NGL_PS_MAX; i++)
	{
		D3DPIXELSHADERDEF *psdf = (D3DPIXELSHADERDEF *) nglPixelShaderOpcode[i];
		NGL_D3DTRY(nglDev->CreatePixelShader(psdf, &nglPixelShaderHandle[i]));
		if (!nglPixelShaderHandle[i])
			nglFatal("CreatePixelShader() failed !");
	}
}

/*-----------------------------------------------------------------------------
Description: Vertex shader source code for texture stages combinations.
-----------------------------------------------------------------------------*/
void ngl_vs_notex(uint32 key)
{
	// Empty, nothing to generate :)
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("NOTEX/");
#endif
}

void ngl_vs_t(uint32 key)
{
	// Scroll the map ?
	if (key & NGL_VSFLAG_SCROLLUV)
	{
		nglvsf_map_scrolluv(0);
#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglPrintf("T scroll/");
#endif
	}
	else
	{
		nglvsf_map(0);
#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglPrintf("T/");
#endif
	}
}

void ngl_vs_e(uint32 key)
{
	nglvsf_enviromap(0);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("E/");
#endif
}

void ngl_vs_td(uint32 key)
{
	nglvsf_map(0);
	nglvsf_detailmap(1);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TD/");
#endif
}

void ngl_vs_te(uint32 key)
{
	nglvsf_map(0);
	nglvsf_enviromap(1);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TE/");
#endif
}

void ngl_vs_tl(uint32 key)
{
	nglvsf_map(0);
	nglvsf_lightmap(1);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TL/");
#endif
}

void ngl_vs_el(uint32 key)
{
	nglvsf_enviromap(0);
	nglvsf_lightmap(1);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("EL/");
#endif
}

void ngl_vs_tde(uint32 key)
{
	nglvsf_map(0);
	nglvsf_detailmap(1);
	nglvsf_enviromap(2);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TDE/");
#endif
}

void ngl_vs_tdl(uint32 key)
{
	nglvsf_map(0);
	nglvsf_detailmap(1);
	nglvsf_lightmap(2);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TDL/");
#endif
}

void ngl_vs_tel(uint32 key)
{
	nglvsf_map(0);
	nglvsf_enviromap(1);
	nglvsf_lightmap(2);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TEL/");
#endif
}

void ngl_vs_tdel(uint32 key)
{
	nglvsf_map(0);
	nglvsf_detailmap(1);
	nglvsf_enviromap(2);
	nglvsf_lightmap(3);
#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglPrintf("TDEL/");
#endif
}

// WARNING: Order MUST matches the NGL_TS_xxx enum !
void (*nglCreateVSFunc[NGL_TS_MAX]) (uint32 key) =
{
	// 0 stage.
	ngl_vs_notex,
	// 1 stage.
	ngl_vs_t,
	ngl_vs_e,
	// 2 stages.
	ngl_vs_td,
	ngl_vs_te,
	ngl_vs_tl,
	ngl_vs_el,
	// 3 stages.
	ngl_vs_tde,
	ngl_vs_tdl,
	ngl_vs_tel,
	// 4 stage.
	ngl_vs_tdel
};

/*-----------------------------------------------------------------------------
Description: Read a "vshader keys file" and generate (compile + create) the
             corresponding vertex shaders.
             Each vertex shader can be associated to a key which completely
             describes the vertex shader (textures passes, lights, etc.).
             By saving those keys in a file, it allows the client to load this
             file and generate the associated vshaders at initialization time.
             The file format (binary) is very simple: each key is stored on
             4-bytes and they are located consecutively in the file:
             offset 0: key 0 (4-bytes)
             offset 4: key 1 (4-bytes)
             offset 8: key 2 (4-bytes)
             ...
-----------------------------------------------------------------------------*/
bool nglReadVShaderFileXB(const char *FileName)
{
	// Open the file for reading operation.
	HANDLE f = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
						  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
						  NULL);

	if (f == INVALID_HANDLE_VALUE)
	{
		nglError("Can't open (read) vshader file \"%s\" !\n", FileName);
		return false;
	}

	// Get file size.
	uint32 Size;
	Size = GetFileSize(f, NULL);

	// Allocate memory and read the file.
	uint32 *VSKeys;
	VSKeys = (uint32 *) nglMemAllocInternal(Size);
	uint32 bytesRead;

	// Read check.
	if (!ReadFile(f, VSKeys, Size, (DWORD *) &bytesRead, NULL))
	{
		CloseHandle(f);
		nglError("Can't read \"%s\" !\n", FileName);
		nglMemFreeInternal(VSKeys);
		return false;
	}

	if (bytesRead != Size)
	{
		CloseHandle(f);
		nglError("Can't read \"%s\" !\n", FileName);
		nglMemFreeInternal(VSKeys);
		return false;
	}

	// Parse the keys and generate the corresponding vshaders.
	uint32 KeysCount = Size / sizeof(uint32);
	for (uint32 i = 0; i < KeysCount; i++)
	{
		// If the key isn't found in the DB, then the corresponding vshader is created and inserted.
		nglGetVertexShaderFromDB(VSKeys[i]);
	}

	nglInfo("%d vertex shader keys read from %s.\n", KeysCount, FileName);

	// Free the memory allocated for the file.
	nglMemFreeInternal(VSKeys);

	CloseHandle(f);

	return true;
}

/*-----------------------------------------------------------------------------
Description: Write the vertex shader keys that have been used until now.
             In other words, generates a file which contains all the vertex
             shaders generated/used until now.
             If the specified file already exists, its keys will be added to
             the current database before beeing written to the file (aka keys
             are appended to the already existing file without duplicates).
-----------------------------------------------------------------------------*/
bool nglWriteVShaderFileXB(const char *FileName)
{
	// If the vshader key file already exists, read it.
	HANDLE f = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
						  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (f != INVALID_HANDLE_VALUE)
	{
		CloseHandle(f);
		nglReadVShaderFileXB(FileName);
	}

	// Open the file for writing operation.
	// If the file already exist, it is overwritten.
	f = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
				   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (f == INVALID_HANDLE_VALUE)
	{
		nglError("Can't open (write) vshader file \"%s\" !\n", FileName);
		return false;
	}

	uint32 bytesWritten;
	for (uint32 i = 0; i < nglVSDBPtr; i++)
	{
		if (!WriteFile(f, &nglVSDB[i].Key, sizeof(uint32), (DWORD *) &bytesWritten, NULL))
		{
			CloseHandle(f);
			nglError("Can't write to \"%s\" !\n", FileName);
			return false;
		}
		if (bytesWritten != sizeof(uint32))
		{
			CloseHandle(f);
			nglError("Can't write \"%s\" !\n", FileName);
			return false;
		}
	}

	CloseHandle(f);

	return true;
}

/*-----------------------------------------------------------------------------
Description: Get a vertex shader (specified by its key) from the database.
             If it is not found, it is compiled, created and inserted into the
             database.
-----------------------------------------------------------------------------*/
DWORD nglGetVertexShaderFromDB(uint32 key)
{
	NGL_ASSERT(key <= NGL_VS_HASH_TABLE_SIZE, "VS key out of range ! (shouldn't happen, vskey buggy ?)");

	// If the vshader is found in the database, return its handle.
	if (nglVSHashTable[key])
		return nglVSHashTable[key]->Handle;

	// We generate the source code, compile it and insert it into the DB.

	DWORD *VSDecl;				// VS declaration (skin/unskin).

	// Build the vertex shader source code.
	nglvsf_begin();

#if NGL_DEBUG
	if (nglDebug.InfoPrints)
		nglInfo("VS %d, type: ", nglVSDBPtr);
#endif

	// Skinned mesh ?
	if (key & NGL_VSFLAG_SKIN)
	{
		nglvsf_skin_transform();
		VSDecl = (DWORD *) nglVS_Skin_Decl;

#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglPrintf("skin/");
#endif
	}
	else
	{
		nglvsf_transform();
		VSDecl = (DWORD *) nglVS_NonSkin_Decl;
	}

	// Texture stages combinations.
	nglCreateVSFunc[key & NGL_VSFLAG_TS_MASK] (key);

	// Fog ?
	if (key & NGL_VSFLAG_FOG)
	{
		nglvsf_fog();
#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglPrintf("fog/");
#endif
	}
	else
	{
		nglvsf_nofog();
	}

	// Alpha falloff ?
	/*
	   // Not implemented.
	   if (key & NGL_VSFLAG_ALPHAFALLOFF)
	   nglvsf_alphafalloff();
	 */

	// Light ?
	// To construct a normal light, the following VSF are required:
	//   1. nglvsf_light_diffuse_notint / nglvsf_light_diffuse_tint
	//   2. nglvsf_point_light / nglvsf_direction_light
	//   * Only use the following VSF if more than 1 light is used:
	//     2.1 nglvsf_point_light_next / nglvsf_direction_light_next
	//   3. nglvsf_light_last

	// LightCount = directional light count + point light count.
	uint32 LightCount =
		((key >> NGL_VSFLAG_DIRLIGHT_OFS) & (uint32) 0xF) +
		((key >> NGL_VSFLAG_PNTLIGHT_OFS) & (uint32) 0xF);

	if (LightCount)
	{
#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglPrintf("%d lights: ", LightCount);
#endif

		// Tint ?
		if (key & NGL_VSFLAG_TINT)
			nglvsf_light_begin(true);
		else
			nglvsf_light_begin(false);

		for (uint32 i = 0; i < LightCount; i++)
		{
			nglvsf_light_add_directional();
#if NGL_DEBUG
			if (nglDebug.InfoPrints)
				nglPrintf("D");
#endif

			/*
			// This code will not work with point/directional light combination.
			// Anyway, point light is not implemented yet.
			if (nglVSC_Light[i].Type == NGL_DIR_LIGHT)
			{
				nglvsf_light_add_directional();
				#if NGL_DEBUG
				if (nglDebug.InfoPrints)
					nglPrintf("D");
				#endif
			}
			else
			{
				nglvsf_light_add_point();
				#if NGL_DEBUG
				if (nglDebug.InfoPrints)
					nglPrintf("P");
				#endif
			}
			*/
		}

		nglvsf_light_end();

#if NGL_DEBUG
		if (nglDebug.InfoPrints)
			nglPrintf("/");
#endif
	}
	// No light.
	else
	{
		// Tint ?
		if (key & NGL_VSFLAG_TINT)
		{
			nglvsf_color(true);
		}
		else
		{
			nglvsf_color(false);
		}
	}

#if NGL_DEBUG
	if (key & NGL_VSFLAG_TINT)
	{
		if (nglDebug.InfoPrints)
			nglPrintf("tint/");
	}
#endif

#if NGL_DEBUG
	if (nglDebug.InfoPrints)
	{
		nglPrintf(" Key = 0x%8.8x\n", key);
	}
#endif

	nglvsf_end();

  InsertVShader:

	// If the DB overflows, clear it.
	if (nglVSDBPtr >= NGL_VS_DB_SIZE)
	{
		nglClearVShaderDB();
	}

	// Assemble the vertex shader and insert it into the DB.
	// Should be called once per vertex shader key.
	nglVSDBEntry *Entry = &nglVSDB[nglVSDBPtr];
	Entry->Key = key;

	// If vshader create fails, it means that the opcode memory has overflowed.
	if (!nglCompileVertexShader(VSDecl, Entry->Handle))
	{
		nglClearVShaderDB();
		goto InsertVShader;
	}

	nglVSHashTable[key] = Entry;

	nglVSDBPtr++;

	return Entry->Handle;
}
