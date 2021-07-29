/*---------------------------------------------------------------------------------------------------------
  NGL_GC.CPP - Gamecube implementation of NGL.

  Maintained by Michael Montague (mikem@treyarch.com)
---------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/mtx.h>
#include <dolphin/vi.h>
#include <dolphin/gx.h>
#include <dolphin/base/PPCArch.h>
#include <dolphin/mtx/mtx44ext.h>

#include "ngl_fileio.h"

#ifdef NGL_GC_PERFLIB
#include <dolphin/perf.h>
#endif // NGL_GC_PERFLIB

/*---------------------------------------------------------------------------------------------------------
  Specifics for dealing with the Arch Engine.
---------------------------------------------------------------------------------------------------------*/
#ifdef ARCH_ENGINE
#include "global.h"
#include "profiler.h"
#else
#define START_PROF_TIMER(x)
#define STOP_PROF_TIMER(x)
#define ADD_PROF_TIMER(x,y)
#endif

#undef BOOL

/*---------------------------------------------------------------------------------------------------------
  NGL standard includes.
---------------------------------------------------------------------------------------------------------*/
#include "ngl_version.h"
#include "ngl_gc.h"
#include "ngl_gc_internal.h"

// Standard resource includes.
#ifdef NGL_DEBUG
#include "ngl_spheremesh.inc"
#include "ngl_radiusmesh.inc"
#endif
#include "ngl_sysfont_fdf.inc"
#include "ngl_sysfont_gct.inc"

#include "ngl_instbank.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/*---------------------------------------------------------------------------------------------------------
  General useful functions.
---------------------------------------------------------------------------------------------------------*/
// A nice complement to sizeof.
#define elementsof(x) (sizeof(x)/sizeof(x[0]))

inline float minf( float a, float b ) { if ( a < b ) return a; return b; }
inline float maxf( float a, float b ) { if ( a > b ) return a; return b; }

inline int mini( int a, int b ) { if ( a < b ) return a; return b; }
inline int maxi( int a, int b ) { if ( a > b ) return a; return b; }

#define NGL_DEGTORAD(x) ((M_PI*(x))/180.0f)
#define NGL_RADTODEG(x) ((180.0f*(x))/M_PI)
#define NGL_REF_ASPECT_RATIO (4.0f/3.0f)

/*---------------------------------------------------------------------------------------------------------
  NGL internal constants.
---------------------------------------------------------------------------------------------------------*/

// Bins for fast sorting by material/Z.
// NGL_OPAQUE_BINS -must- be a power of 2 plus 1! (don't ask ;))
#define NGL_OPAQUE_BINS			129
#define NGL_TRANSLUCENT_BINS	128
#define NGL_TOTAL_BINS ( NGL_OPAQUE_BINS + NGL_TRANSLUCENT_BINS )

// Render list node types.
enum
{
  	NGLNODE_BIN          = 0,
  	NGLNODE_MESH_SECTION = 1,
  	NGLNODE_QUAD         = 2,
  	NGLNODE_STRING       = 3,
  	NGLNODE_CUSTOM       = 4,
};

// Clip planes.  Code relies on the order.
enum
{
  	NGLCLIP_NEAR   = 0,
  	NGLCLIP_FAR    = 1,
  	NGLCLIP_LEFT   = 2,
  	NGLCLIP_RIGHT  = 3,
  	NGLCLIP_TOP    = 4,
  	NGLCLIP_BOTTOM = 5,
  	NGLCLIP_MAX    = 6
};

// texture                                (1)
// texture + detail                       (3)
// texture +          environment         (5)
// texture + detail + environment         (7)
// texture +                        light (9)
// texture + detail +               light (11)
// texture +          environment + light (13)
// texture + detail + environment + light (15)

#define NGL_RMODE_TEXTURE     0x1
#define NGL_RMODE_DETAIL      0x2
#define NGL_RMODE_ENVIRONMENT 0x4
#define NGL_RMODE_LIGHT       0x8
#define NGL_RMODE_VARBIT1			0x10
#define NGL_RMODE_VARBIT2			0x20

#define NGL_RMODE_NOINDEX			NGL_RMODE_VARBIT1
#define NGL_RMODE_2_COL				NGL_RMODE_VARBIT2

#define NGL_RMODE_T    (NGL_RMODE_TEXTURE)
#define NGL_RMODE_TD   (NGL_RMODE_TEXTURE|NGL_RMODE_DETAIL)
#define NGL_RMODE_TE   (NGL_RMODE_TEXTURE|NGL_RMODE_ENVIRONMENT)
#define NGL_RMODE_TDE  (NGL_RMODE_TEXTURE|NGL_RMODE_DETAIL|NGL_RMODE_ENVIRONMENT)
#define NGL_RMODE_TL   (NGL_RMODE_TEXTURE|NGL_RMODE_LIGHT)
#define NGL_RMODE_TDL  (NGL_RMODE_TEXTURE|NGL_RMODE_DETAIL|NGL_RMODE_LIGHT)
#define NGL_RMODE_TEL  (NGL_RMODE_TEXTURE|NGL_RMODE_ENVIRONMENT|NGL_RMODE_LIGHT)
#define NGL_RMODE_TDEL (NGL_RMODE_TEXTURE|NGL_RMODE_DETAIL|NGL_RMODE_ENVIRONMENT|NGL_RMODE_LIGHT)

/*---------------------------------------------------------------------------------------------------------
  Internal structures.
---------------------------------------------------------------------------------------------------------*/
// This is an object shared by mesh material nodes in the render list, it's a single instance of a mesh in the scene.
struct nglMeshNode
{
  	nglMesh* Mesh;                 // Mesh resource.
  	int Clip;                      // 1 if the mesh intersects one or more of the guard bands.
  	float Dist;                    // Distance from the farthest point on the mesh to the camera.

  	nglMatrix LocalToWorld;        // Local to world matrix.
  	nglMatrix WorldToLocal;        // Inverse of the above, for lighting in local space.

  	nglMatrix LocalToScreen;

  	nglRenderParams Params;        // Custom render parameters.
};

struct nglMeshSectionNode
{
  	nglMeshNode* MeshNode;         // Mesh node structure shared by all the nodes.
  	nglMeshSection* Section;       // What section of the mesh to render.
  	u_int MaterialFlags;           // Copy of material flags, possibly with changes.
 	nglMeshSectionNode *NextSectionNode;
};

struct nglStringNode
{
  	char Data[256];
  	u_int FontIdx;
  	float X, Y, Z;
  	float XScale, YScale;
  	u_int Color;
};

struct nglListNode
{
  	nglListNode* Next;             // Next node in the render order.
  	u_int Type;                    // Node Type.

  	// Sorting parameters.
  	nglSortInfo SortInfo;

  	// Custom node data.
  	nglCustomNodeFn NodeFn;
  	void* NodeData;
};

// Made to look like a nglListNode.
struct nglListBin
{
  	nglListNode* Next;             // Next node in the render order.
  	u_int Type;
};

struct nglLightListNode
{
  	nglLightListNode* Next;        // Next light in scene.
  	nglLightListNode* LocalNext;   // Next light affecting the current object.
  	nglLightInfo Light;            // Copy of the light structure.

  	nglVector WorldPos;       // Position in world space.
  	nglVector WorldDir;       // Direction in world space.
};

struct nglProjectorLightListNode
{
  	nglProjectorLightListNode* Next;        // Next projector light in scene.
  	nglProjectorLightListNode* LocalNext;   // Next projector light affecting the current object.
  	nglProjectorLightInfo ProjectorLight;   // Copy of the projector light structure.
};

// Lighting structures.
struct nglLightNode
{
  	// Next light in scene for each lighting category the node is in.
  	nglLightNode* Next[NGL_NUM_LIGHTCATS];
  	nglLightNode* LocalNext[NGL_NUM_LIGHTCATS];

  	u_int Type;
  	void* Data;       // Copy of the light structure.

  	u_int LightCat;   // Lighting category flags.
};

struct nglLightContext
{
  	// Head node for the list of all lights in this context.
  	nglLightNode Head;

  	// Head node for the list of projector lights in this context.
  	nglLightNode ProjectorHead;

  	// Ambient color for the context.
  	nglVector Ambient;
};

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
  	u_int BlendMode;
  	float BlendModeConstant;

  	// Texture to project.
  	nglTexture* Tex;
};

struct nglDirProjectorLightInfo
{
  	// These matrices MUST be on a 16-bytes boundary !
  	nglMatrix WorldToUV;  // the matrix for going from world to uv space
  	nglMatrix UVToWorld;  // the matrix for going from uv to world space

  	// Blending operation (usually NGLBM_ADDITIVE).
  	u_int BlendMode;
  	float BlendModeConstant;

  	// Texture to project.
  	nglTexture* Tex;

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

enum ViewTypes
{
	NGL_VIEW_PERSPECTIVE,
	NGL_VIEW_ORTHOGRAPHIC,
	NGL_VIEW_MAX_TYPES,
};

class nglScene
{
public:
  	// Scene hierarchy info.
  	nglScene* Parent;
  	nglScene* NextSibling;
  	nglScene* FirstChild;
  	nglScene* LastChild;

  	nglTexture* RenderTarget;   // Usually nglBackBufferTex, or a texture for the scene to be rendered into.
  	bool Download;              // If true, the result of rendering is downloaded into a system memory texture.

  	// Global matrices.
  	nglMatrix ViewToWorld;    // Camera -> World

  	nglMatrix ViewToScreen;   // Camera -> Screen

  	nglMatrix WorldToView;    // World -> Camera
  	nglMatrix WorldToScreen;  // World -> Camera -> Screen

  	nglVector ClipPlanes[NGLCLIP_MAX];     // Planes for sphere rejection.

  	nglListBin RenderListBins[NGL_TOTAL_BINS];

  	// Viewport.
  	float ViewX1, ViewY1, ViewX2, ViewY2;

  	// Screen clear settings.
  	u_int ClearFlags;
  	float ClearZ;
  	u32 ClearColor;

  	// Fog settings.
	bool    FogEnabled;
  	int     FogType;
  	float   FogNear;
  	float   FogFar;
  	float   FogMin;
  	float   FogMax;
  	GXColor FogColor;

  	// Saved parameters to nglSetPerspectiveMatrix.
		#ifdef PROJECT_KELLYSLATER
		float CX;
		float CY;
  	float ZMin;
  	float ZMax;
		#endif

  	float HFOV;
  	float NearZ;
  	float FarZ;
	float Aspect;
	ViewTypes ViewType; // Perspective / Ortho / other.
	float QuadZCompParam[2]; // Used to compute the Z for the Quad.

	// params specific to ortho
	float Top;
	float Bottom;
	float Left;
	float Right;

  	bool ZWriteEnable;
  	bool ZTestEnable;

  float AnimTime;
  u_int IFLFrame;
};

#ifdef NGL_DEBUGGING
#define STAGE_ENABLE( x ) ( nglStage.x )
#else
#define STAGE_ENABLE( x ) 1
#endif

#ifdef NGL_DEBUGGING
#define DEBUG_ENABLE( x ) ( nglSyncDebug.x )
#else
#define DEBUG_ENABLE( x ) 0
#endif

nglStageStruct nglStage; // = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

nglDebugStruct nglDebug = { 0 };
static nglDebugStruct nglSyncDebug = { 0 };  // Updated synchronously once per frame.

nglPerfInfoStruct nglPerfInfo = { 0 };

#if defined(NGL_GC_PERFLIB)

#define nglPERFDLSize ( 64 * 1024 )
static int nglPERFDLRenderSize = 0;
static u32 nglPERFDL[nglPERFDLSize] __attribute__((aligned(32)));

enum PERFEvents
{
	NGL_DUMMY_EVENT,
  	NGL_NUM_PERF_EVENTS
};

struct nglPERFEventStruct
{
  	int id;
  	char *name;
  	int type;
  	GXColor color;
};

struct nglPERFEventStruct nglPERFEvents[NGL_NUM_PERF_EVENTS] =
{
	{ NGL_DUMMY_EVENT, "", 0, { 0x00, 0x00, 0x00, 0x00 } },
};

#define START_EVENT(x) PERFEventStart( (x) )
#define END_EVENT(x) PERFEventEnd( (x) )
#else //NGL_GC_PERFLIB
#define START_EVENT(x)
#define END_EVENT(x)

#endif //NGL_GC_PERFLIB

#define NGLGC_UNSCALE_UV     (1/NGLGC_SCALE_UV)
#define NGLGC_UNSCALE_NRM    (1/NGLGC_SCALE_NRM)
#define NGLGC_SCALE_WEIGHT   255.0f // We use 255.5f on the exporter side.
#define NGLGC_UNSCALE_WEIGHT (1/NGLGC_SCALE_WEIGHT)

#pragma pack(push,1)
// Vertex data stored in a section
struct nglVertPN
{
	float x;
	float y;
	float z;
	short nx;
	short ny;
	short nz;
	short Pad1;
};

struct nglVertP
{
	float x;
	float y;
	float z;
};

struct nglVertN
{
	short nx;
	short ny;
	short nz;
};

struct nglSkinVertPN
{
	float x;
	float y;
	float z;
	short nx;
	short ny;
	short nz;
	short bones;
	u_char index1;
	u_char index2;
	u_char index3;
	u_char index4;
	u_char weight1;
	u_char weight2;
	u_char weight3;
	u_char weight4;
};

struct nglSkinnedVertex
{
	u_int color;
	short u1;
	short v1;
	short u2;
	short v2;
};

struct nglSkinnedVertex2
{
	float x;
	float y;
	float z;
	short nx;
	short ny;
	short nz;
	short bones;
	u_int color;
	short u1;
	short v1;
	short u2;
	short v2;
	u_char index1;
	u_char index2;
	u_char index3;
	u_char index4;
	u_char weight1;
	u_char weight2;
	u_char weight3;
	u_char weight4;
};
#pragma pack(pop)

typedef struct _nglTevTriplet
{
	GXTevStageID stage;
	GXTexCoordID coord;
	GXTexMapID map;
} nglTevTriplet;

// Lighting.
nglLightContext* nglDefaultLightContext = 0;
nglLightContext* nglCurLightContext = 0;

/*---------------------------------------------------------------------------------------------------------
  Internal function prototypes.
---------------------------------------------------------------------------------------------------------*/

static void nglClearVRAM( u_char r, u_char g, u_char b, u_char a );

static nglMesh* nglProcessMeshFile( nglMeshFileHeader* Mesh );

static void nglRenderMeshCallback( void* Data );
static void nglRenderQuadCallback( void* Data );

static NGL_INLINE void nglGenEnvCoords( u_short PIdx, u_short NIdx=-1 );

static void nglSetupMeshLighting(nglMeshSection* Section, nglMesh* Mesh, nglMeshNode* MeshNode, nglRenderParams* Params );
static void nglSetupMeshProjLighting( nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params );
static NGL_INLINE bool nglSphereInFrustum(const nglVector Center, float Radius);

static void* nglListAlloc( u_int Bytes, u_int Alignment = 4 );

//----------------------------------------------------------------------------

void nglTransposeMatrix( nglMatrix& DstMat, nglMatrix SrcMat );

/*---------------------------------------------------------------------------------------------------------
  Global variables.
---------------------------------------------------------------------------------------------------------*/

// Current in-progress scratch mesh.
bool nglScratchEdit = false;
nglMesh* nglScratch = NULL;
nglMeshSection* nglScratchSection = NULL;
short* nglScratchStripPos = NULL;
u_int nglScratchVertIdx = 0;

static u8* nglListWork = NULL;
static u8* nglListWorkPos;
static u32 nglListWorkSize;

static void*      nglFifo = NULL;
static GXFifoObj* nglFifoObj = NULL;
static u32        nglFifoSize = (256*1024);

static nglLightListNode nglLightListHead;
static nglProjectorLightListNode nglProjectorLightListHead;

// Frame counter.
static int nglFrame = 0;

// Pathnames to append when loading meshes/textures.
static char nglMeshPath[256];
static char nglTexturePath[256];

// Calls outside nglListInit/nglListSend affect the global scene.
static nglScene nglDefaultScene;

// Currently active scene.
static nglScene* nglCurScene = &nglDefaultScene;

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
	u_int TexOfs[2];       // Offset in texture of topleft of glyph, in pixels.
	u_int GlyphSize[2];    // Pixels size of glyph.
	u_int GlyphOrigin[2];  // Pixels offset from cp to glyph origin.
	u_int CellWidth;       // Pixels to advance cp.
};

struct nglFontHeader
{
	u_int Version;
	u_int CellHeight;
	u_int Ascent;
	u_int FirstGlyph;
	u_int NumGlyphs;
};

struct nglFont
{
  	nglTexture*   Tex;
  	nglGlyphInfo* GlyphInfo;
  	nglFontHeader Header;
  	u_int	MapFlags;
  	u_int	BlendMode;
  	u_int	BlendModeConstant;
};

// System font.
nglFont *nglSysFont;

// Font storage.  Should fonts get an instance bank?
nglFont nglFonts[NGL_MAX_FONTS];

u32 nglVBlankCount = 0; // Updated every flip.

float nglGCAnimFramesPerSecond = 30.0f;
// Current animation frame for animated textures.  This global is sometimes used as a parameter.
static int nglTextureAnimFrame = 0;
float nglIFLSpeed = 60.0f;

// Debugging flag to optionally turn off and on material flags.
static u_int nglMaterialFlagsAnd = 0xFFFFFFFF;
static u_int nglMaterialFlagsOr = 0;

static char nglWork[512];

static nglSystemCallbacks nglCurSystemCallbacks = { 0 };

nglMesh* nglSphereMesh;
nglMesh* nglRadiusMesh;

// Skip list based instancing system used for meshes and textures.
static nglInstanceBank nglMeshBank;
static nglInstanceBank nglMeshFileBank;
static nglInstanceBank nglTextureBank;
static nglInstanceBank nglFontBank;

// NGL on GameCube objects
static GXRenderModeObj nglRenderModeObj;

static bool nglFirstFlip = true;
static bool nglCopyToXFB = false;

static void* nglFrameBuffer = NULL;
static int nglFrameBufferSize = 0;

nglTexture nglWhiteTex;
static nglMaterial nglWhiteMat;
static u32 nglWhite1x1Tex[16] __attribute__((aligned(32)));

static int* nglVertEnvMapTemp = NULL;
static int nglVertEnvMapSize = sizeof( nglVertP ) >> 2;
static nglVertP *nglEnvMapVertP;
static nglVertN *nglEnvMapVertN;
// hacky global vars for env. mapping
static nglMatrix nglMeshLocalToWorld;
static nglMatrix nglMeshWorldToLocal;
//static nglMatrix nglMeshLocalToView;

static u_int nglTVMode = NGLTV_NTSC;
static u_int nglDisplayWidth = 640;
static u_int nglDisplayHeight = 448;

static Mtx 		_nglMTXIdentity;
static Mtx 		_nglMTXIdentityInvZ;
static GXColor	_nglGXColorZero;
static GXColor	_nglGXColorOne;

static GXAttr nglTexAttrs[] = {
	GX_VA_TEX0,
	GX_VA_TEX1,
	GX_VA_TEX2,
	GX_VA_TEX3,
	GX_VA_TEX4,
	GX_VA_TEX5,
	GX_VA_TEX6,
	GX_VA_TEX7,
};

static GXTexCoordID nglTexCoordDst[] = {
	GX_TEXCOORD0,
	GX_TEXCOORD1,
	GX_TEXCOORD2,
	GX_TEXCOORD3,
	GX_TEXCOORD4,
	GX_TEXCOORD5,
	GX_TEXCOORD6,
	GX_TEXCOORD7,
};

static GXTexGenSrc nglTexCoordSrc[] = {
	GX_TG_TEX0,
	GX_TG_TEX1,
	GX_TG_TEX2,
	GX_TG_TEX3,
	GX_TG_TEX4,
	GX_TG_TEX5,
	GX_TG_TEX6,
	GX_TG_TEX7,
	GX_TG_TEXCOORD0,
	GX_TG_TEXCOORD1,
	GX_TG_TEXCOORD2,
	GX_TG_TEXCOORD3,
	GX_TG_TEXCOORD4,
	GX_TG_TEXCOORD5,
	GX_TG_TEXCOORD6,
};

enum nglGCTexMtxTypes {
	NGLTEXMTX_IDENTITY,
	NGLTEXMTX_DEFAULT_SCALE,
	NGLTEXMTX_DETAIL_SCALE,
	NGLTEXMTX_SCROLLEDMAP,
	NGLTEXMTX_ENVMAP,
	NGLTEXMTX_SHADMAP,
};

enum nglGCTexPtMtxTypes {
	NGLTEXMTX_PM_IDENTITY,
	NGLTEXMTX_PM_ENVMAP,
};

static GXTexMtx nglTexMtx[] = {
	GX_IDENTITY,
	GX_TEXMTX0, // Always used as the basic scaling matrix.
	GX_TEXMTX1, // Always used for the detail texture.
	GX_TEXMTX2, // Always used for ScrolledMap.
	GX_TEXMTX3, // Always used for EnvMap.
	GX_TEXMTX4,	// Always used for ShadowMap.
	GX_TEXMTX5,
	GX_TEXMTX6,
	GX_TEXMTX7,
	GX_TEXMTX8,
	GX_TEXMTX9,
};

static GXPTTexMtx nglTexPtMtx[] = {
	GX_PTIDENTITY,
	GX_PTTEXMTX0, // Always used for the EnvMap PostMatrix.
	GX_PTTEXMTX2,
	GX_PTTEXMTX3,
	GX_PTTEXMTX4,
	GX_PTTEXMTX5,
	GX_PTTEXMTX6,
	GX_PTTEXMTX7,
	GX_PTTEXMTX8,
	GX_PTTEXMTX9,
	GX_PTTEXMTX10,
	GX_PTTEXMTX11,
	GX_PTTEXMTX12,
	GX_PTTEXMTX13,
	GX_PTTEXMTX14,
	GX_PTTEXMTX15,
	GX_PTTEXMTX16,
	GX_PTTEXMTX17,
	GX_PTTEXMTX18,
	GX_PTTEXMTX19,
};

static nglVector* nglSkinWork = NULL;

static nglTevTriplet nglTevList[] = {
	{ GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0 },
	{ GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1 },
	{ GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP2 },
	{ GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP3 },
	{ GX_TEVSTAGE4, GX_TEXCOORD4, GX_TEXMAP4 },
	{ GX_TEVSTAGE5, GX_TEXCOORD5, GX_TEXMAP5 },
	{ GX_TEVSTAGE6, GX_TEXCOORD6, GX_TEXMAP6 },
	{ GX_TEVSTAGE7, GX_TEXCOORD7, GX_TEXMAP7 },
	{ GX_TEVSTAGE8, GX_TEXCOORD7, GX_TEXMAP7 },
	{ GX_TEVSTAGE9, GX_TEXCOORD7, GX_TEXMAP7 },
	{ GX_TEVSTAGE10, GX_TEXCOORD7, GX_TEXMAP7 },
};

static GXLightObj nglLights[8];
// GX lights are bits, so we need this to translate
static GXLightID nglGXLight[8] =
{
	GX_LIGHT0,
	GX_LIGHT1,
	GX_LIGHT2,
	GX_LIGHT3,
	GX_LIGHT4,
	GX_LIGHT5,
	GX_LIGHT6,
	GX_LIGHT7,
};

static GXFogType nglFogTypes[6] = {
	GX_FOG_NONE,
	GX_FOG_LIN,
	GX_FOG_EXP,
	GX_FOG_EXP2,
	GX_FOG_REVEXP,
	GX_FOG_REVEXP2,
};

#ifdef __MWERKS__
int stricmp( const char* s1, const char* s2 )
{

	while( *s1 && *s2 ) {
		char c1 = tolower( *s1 );
		char c2 = tolower( *s2 );

		if( c1 < c2 ) {
			return -1;
		} else if( c1 > c2 ) {
			return 1;
		} else {
			s1++;
			s2++;
		}

	}

	if( *s1 ) {
		return 1;
	} else if( *s2 ) {
		return -1;
	} else {
		return 0;
	}

}
#endif

//-----------------------------------------------------------------------------

NGL_INLINE long volatile nglGetCPUCycle( void )
{
	return OSGetTick( );
}

void nglSetSystemCallbacks( nglSystemCallbacks* Callbacks )
{
	memcpy(&nglCurSystemCallbacks, Callbacks, sizeof(nglSystemCallbacks));
}

//-----------------------------------------------------------------------------

#ifndef NGL_FINAL

void nglPrintf( const char* Format, ... )
{
  	static char Work[512];
  	va_list args;
  	va_start( args, Format );
  	vsprintf( Work, Format, args );
  	va_end( args );

  	if (nglCurSystemCallbacks.DebugPrint)
    	nglCurSystemCallbacks.DebugPrint( Work );
	else
		OSReport(Work);
}

void nglWarning( const char* Format, ... )
{
	// For now this is the same as nglPrintf.
	static char Work[512];
  	va_list args;
  	va_start( args, Format );
  	vsprintf( Work, Format, args );
  	va_end( args );

  	if (nglCurSystemCallbacks.DebugPrint)
    	nglCurSystemCallbacks.DebugPrint( Work );
	else
  		nglPrintf( TTY_RED "%s" TTY_BLACK, Work );
}

void nglFatal( const char* Format, ... )
{
  	static char Work[1024];
  	va_list args;
  	va_start( args, Format );
  	vsprintf( Work, Format, args );
  	va_end( args );

  	if (nglCurSystemCallbacks.CriticalError)
    	nglCurSystemCallbacks.CriticalError( Work );
	else
	{
    	OSReport( TTY_BLUE "NGL Fatal Error: " TTY_RED );
    	OSReport( Work );
    	OSReport( TTY_BLACK "\n" );
		asm( trap );
  	}
}

#endif // !NGL_FINAL

bool nglReadFileGC( const char* FileName, nglFileBuf* File, u_int Align )
{
	DVDFileInfo io;
	const char* PreMunge = FileName;
	char Munged[256];
	char* Munge = Munged;

	while( *PreMunge ) {

		if( *PreMunge == '\\' ) {
			*Munge = '/';
		} else {
			*Munge = *PreMunge;
		}

		Munge++;
		PreMunge++;
	}

	*Munge = '\0';

	if( DVDOpen( Munged, &io ) == false ) {
		return false;
	}

	File->Size = DVDGetLength( &io );
	File->Buf = (u_char*) nglMemAlloc( OSRoundUp32B( File->Size ), 32 );
	DVDRead( &io, File->Buf, OSRoundUp32B( File->Size ), 0 );
	DVDClose( &io );

	return true;
}

bool nglReadFile( const char* FileName, nglFileBuf* File, u_int Align )
{
  	if (nglCurSystemCallbacks.ReadFile)
    	return nglCurSystemCallbacks.ReadFile( FileName, File, Align );
   	else
  		return nglReadFileGC( FileName, File, Align );
}

void nglReleaseFileGC( nglFileBuf* File )
{
	nglMemFree( File->Buf );
	memset( File, 0, sizeof( nglFileBuf ) );
}

void nglReleaseFile( nglFileBuf* File )
{
  	if ( nglCurSystemCallbacks.ReleaseFile )
    	nglCurSystemCallbacks.ReleaseFile( File );
  	else
		nglReleaseFileGC( File );
}

void* nglMemAllocGC( u_int Size, u_int Align )
{
	return OSAlloc(Size);
}

void* nglMemAlloc( u_int Size, u_int Align )
{
  	if( nglCurSystemCallbacks.MemAlloc )
    	return nglCurSystemCallbacks.MemAlloc( Size, Align );
   	else
    	return nglMemAllocGC( Size, Align );
}

void nglMemFreeGC( void* Ptr )
{
	if (Ptr) OSFree(Ptr);
}

void nglMemFree( void* Ptr )
{
  	if( nglCurSystemCallbacks.MemFree )
    	nglCurSystemCallbacks.MemFree( Ptr );
   	else
    	nglMemFreeGC(Ptr);
}

void nglNormalize( nglVector& dst, const nglVector src )
{
	float length = sqrtf( src[0] * src[0] + src[1] * src[1] + src[2] * src[2] );
	if (length > 0.0f)
		length = 1.0f / length;
	dst[0] = src[0] * length;
	dst[1] = src[1] * length;
	dst[2] = src[2] * length;
}

static NGL_INLINE void nglSpecialNormalize( nglVector& dst, const nglVector src )
{
	float length = sqrtf( src[0] * src[0] + src[1] * src[1] + src[2] * src[2] );
	if (length > 0.0f)
		length = 1.0f / length;
	dst[0] = src[0] * length;
	dst[1] = src[1] * length;
	dst[2] = src[2] * length;
}

void nglCopyVector( nglVector& dst, const nglVector src )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void nglScaleVector( nglVector& dst, const nglVector src, float scale )
{
	dst[0] = src[0] * scale;
	dst[1] = src[1] * scale;
	dst[2] = src[2] * scale;
	dst[3] = src[3] * scale;
}

void nglScaleVectorXYZ( nglVector& dst, const nglVector src, float s )
{
#ifdef NGL_FAST_MATH
	VECScale( (VecPtr) src, (VecPtr) dst, s );
#else
	dst[0] = src[0] * s;
	dst[1] = src[1] * s;
	dst[2] = src[2] * s;
#endif
}

void nglSubVector( nglVector& dst, const nglVector lhs, const nglVector rhs )
{
	dst[0] = lhs[0] - rhs[0];
	dst[1] = lhs[1] - rhs[1];
	dst[2] = lhs[2] - rhs[2];
	dst[3] = lhs[3] - rhs[3];
}

void nglSubVectorXYZ( nglVector& dst, const nglVector lhs, const nglVector rhs )
{
#ifdef NGL_FAST_MATH
	VECSubtract( (VecPtr) lhs, (VecPtr) rhs, (VecPtr) dst );
#else
	dst[0] = lhs[0] - rhs[0];
	dst[1] = lhs[1] - rhs[1];
	dst[2] = lhs[2] - rhs[2];
#endif
}

void nglSetVector( nglVector& dst, float x, float y, float z, float w )
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

void nglDivVector( nglVector& dst, const nglVector src, float l )
{
	NGL_ASSERT( l != 0.0f, "" );

	dst[0] = src[0] / l;
	dst[1] = src[1] / l;
	dst[2] = src[2] / l;
	dst[3] = src[3] / l;
}

float nglInnerProduct( const nglVector lhs, const nglVector rhs )
{
	// Sony only calculates the xyz inner product
#ifdef NGL_FAST_MATH
	return VECDotProduct( (VecPtr) lhs, (VecPtr) rhs );
#else
	return ( lhs[0] * rhs[0] ) + ( lhs[1] * rhs[1] ) + ( lhs[2] * rhs[2] );
#endif
}

static NGL_INLINE void mtxToNgl( nglMatrix& dst, const Mtx src )
{
	dst[0][0] = src[0][0];
	dst[1][0] = src[0][1];
	dst[2][0] = src[0][2];
	dst[3][0] = src[0][3];

	dst[0][1] = src[1][0];
	dst[1][1] = src[1][1];
	dst[2][1] = src[1][2];
	dst[3][1] = src[1][3];

	dst[0][2] = src[2][0];
	dst[1][2] = src[2][1];
	dst[2][2] = src[2][2];
	dst[3][2] = src[2][3];

	dst[0][3] = 0.0f;
	dst[1][3] = 0.0f;
	dst[2][3] = 0.0f;
	dst[3][3] = 1.0f;
}

static NGL_INLINE void nglToVec( Vec* dst, const nglVector src )
{
	dst->x = src[0];
	dst->y = src[1];
	dst->z = src[2];
}

void nglIdentityMatrix( nglMatrix& matrix )
{
#ifdef NGL_FAST_MATH
	MTX44Identity( matrix );
#else
	static nglMatrix identity = nglMatrix( nglVector(1.0f, 0.0f, 0.0f, 0.0f),
																nglVector(0.0f, 1.0f, 0.0f, 0.0f),
																nglVector(0.0f, 0.0f, 1.0f, 0.0f),
																nglVector(0.0f, 0.0f, 0.0f, 1.0f) );
	memcpy( &matrix[0][0], &identity[0][0], sizeof( nglMatrix ) );
#endif
}

void nglMulMatrix( nglMatrix& dst, const nglMatrix lhs, const nglMatrix rhs )
{
	NGL_ASSERT( &lhs != &dst, "" );
	NGL_ASSERT( &rhs != &dst, "" );

	// dst could be one of our input matrices (and it was one of our bugs in Jan-2002).
	float a0, a1, a2, a3;
	float b00, b01, b02, b03;
	float b10, b11, b12, b13;
	float b20, b21, b22, b23;
	float b30, b31, b32, b33;

	b00 = rhs[0][0];
	b01 = rhs[0][1];
	b02 = rhs[0][2];
	b03 = rhs[0][3];

	b10 = rhs[1][0];
	b11 = rhs[1][1];
	b12 = rhs[1][2];
	b13 = rhs[1][3];

	b20 = rhs[2][0];
	b21 = rhs[2][1];
	b22 = rhs[2][2];
	b23 = rhs[2][3];

	b30 = rhs[3][0];
	b31 = rhs[3][1];
	b32 = rhs[3][2];
	b33 = rhs[3][3];

	a0 = lhs[0][0];
	a1 = lhs[0][1];
	a2 = lhs[0][2];
	a3 = lhs[0][3];

	dst[0][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	dst[0][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	dst[0][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	dst[0][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;

	a0 = lhs[1][0];
	a1 = lhs[1][1];
	a2 = lhs[1][2];
	a3 = lhs[1][3];

	dst[1][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	dst[1][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	dst[1][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	dst[1][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;

	a0 = lhs[2][0];
	a1 = lhs[2][1];
	a2 = lhs[2][2];
	a3 = lhs[2][3];

	dst[2][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	dst[2][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	dst[2][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	dst[2][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;

	a0 = lhs[3][0];
	a1 = lhs[3][1];
	a2 = lhs[3][2];
	a3 = lhs[3][3];

	dst[3][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	dst[3][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	dst[3][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	dst[3][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;
}

#if 0
// FIXME: this function actually doesn't work
asm void nglMulMatrixPS(register nglMatrix ab, register nglMatrix a, register nglMatrix b)
{
    nofralloc;
    psq_l       fp0 ,  0(a), 0, 0;          // a00,a01
    psq_l       fp2 ,  0(b), 0, 0;          // b00,b01
    ps_muls0    fp6 ,   fp2,  fp0;          // b00a00,b01a00
    psq_l       fp3 , 16(b), 0, 0;          // b10,b11
    psq_l       fp4 , 32(b), 0, 0;          // b20,b21
    ps_madds1   fp6 ,   fp3,  fp0,  fp6;    // b00a00+b10a01,b01a00+b11a01
    psq_l       fp1 ,  8(a), 0, 0;          // a02,a03
    psq_l       fp5 , 48(b), 0, 0;          // b30,b31

    // b00a00+b10a01+b20a02,b01a00+b11a01+b21a02
    ps_madds0   fp6 ,   fp4,  fp1,  fp6;
    psq_l       fp0 , 16(a), 0, 0;          // a10,a11

    // b00a00+b10a01+b20a02+b30a03,b01a00+b11a01+b21a02+b31a03
    ps_madds1   fp6 ,   fp5,  fp1,  fp6;
    psq_l       fp1 , 24(a), 0, 0;          // a12,a13
    ps_muls0    fp8 ,   fp2,  fp0;          // b00a10,b01a10
    ps_madds1   fp8 ,   fp3,  fp0,  fp8;    // b00a10+b10a11,b01a11+b11a11
    psq_l       fp0 , 32(a), 0, 0;          // a20,a21

    // b00a10+b10a11+b20a12,b01a11+b11a11+b21a12
    ps_madds0   fp8 ,   fp4,  fp1,  fp8;

    // b00a10+b10a11+b20a12+b30a13,b01a10+b11a11+b21a12+b31a13
    ps_madds1   fp8 ,   fp5,  fp1,  fp8;
    psq_l       fp1 , 40(a), 0, 0;          // a22,a23
    ps_muls0    fp10,   fp2,  fp0;          // b00a20,b01a20
    ps_madds1   fp10,   fp3,  fp0, fp10;    // b00a20+b10a21,b01a20+b11a21
    psq_l       fp0 , 48(a), 0, 0;          // a30,a31

    // b00a20+b10a21+b20a22,b01a20+b11a21+b21a22
    ps_madds0   fp10,   fp4,  fp1, fp10;

    // b00a20+b10a21+b20a22+b30a23,b01a20+b11a21+b21a22+b31a23
    ps_madds1   fp10,   fp5,  fp1, fp10;
    psq_l       fp1 , 56(a), 0, 0;          // a32,a33

    ps_muls0    fp12,   fp2,  fp0;          // b00a30,b01a30
    psq_l       fp2 ,  8(b), 0, 0;          // b02,b03
    ps_madds1   fp12,   fp3,  fp0, fp12;    // b00a30+b10a31,b01a30+b11a31
    psq_l       fp0 ,  0(a), 0, 0;          // a00,a01

    // b00a30+b10a31+b20a32,b01a30+b11a31+b21a32
    ps_madds0   fp12,   fp4,  fp1, fp12;
    psq_l       fp3 , 24(b), 0, 0;          // b12,b13

    // b00a30+b10a31+b20a32+b30a33,b01a30+b11a31+b21a32+b31a33
    ps_madds1   fp12,   fp5,  fp1, fp12;
    psq_l       fp1 ,  8(a), 0, 0;          // a02,a03

    ps_muls0    fp7 ,   fp2,  fp0;          // b02a00,b03a00
    psq_l       fp4 , 40(b), 0, 0;          // b22,b23
    ps_madds1   fp7 ,   fp3,  fp0, fp7;     // b02a00+b12a01,b03a00+b13a01
    psq_l       fp5 , 56(b), 0, 0;          // b32,b33

    // b02a00+b12a01+b22a02,b03a00+b13a01+b23a02
    ps_madds0   fp7 ,   fp4,  fp1, fp7;

    psq_l       fp0 , 16(a), 0, 0;          // a10,a11

    // b02a00+b12a01+b22a02+b32a03,b03a00+b13a01+b23a02+b33a03
    ps_madds1   fp7 ,   fp5,  fp1, fp7;
    psq_l       fp1 , 24(a), 0, 0;          // a12,a13

    ps_muls0    fp9 ,   fp2,  fp0;          // b02a10,b03a10
    psq_st      fp6 , 0(ab), 0, 0;          // ab00,ab01
    ps_madds1   fp9 ,   fp3,  fp0, fp9;     // b02a10+b12a11,b03a10+b13a11
    psq_l       fp0 , 32(a), 0, 0;          // a20,a21

    // b02a10+b12a11+b22a12,b03a10+b13a11+b23a12
    ps_madds0   fp9,    fp4,  fp1, fp9;
    psq_st      fp8 ,16(ab), 0, 0;          // ab10,ab11

    // b02a10+b12a11+b22a12+b32a13,b03a10+b13a11+b23a12+b33a13
    ps_madds1   fp9 ,   fp5,  fp1, fp9;
    psq_l       fp1 , 40(a), 0, 0;          // a22,a23
    ps_muls0    fp11,   fp2,  fp0;          // b02a20,b03a20
    psq_st      fp10,32(ab), 0, 0;          // ab20,ab21
    ps_madds1   fp11,   fp3,  fp0, fp11;    // b02a20+b12a21,b03a20+b13a21
    psq_l       fp0 , 48(a), 0, 0;          // a30,a31

    // b02a20+b12a21+b22a22,b03a20+b13a21+b23a22
    ps_madds0   fp11,   fp4,  fp1, fp11;
    psq_st      fp12,48(ab), 0, 0;          // ab30,ab31

    // b02a20+b12a21+b22a22+b32a23,b03a20+b13a21+b23a22+b33a23
    ps_madds1   fp11,   fp5,  fp1, fp11;

    psq_l       fp1,  56(a), 0, 0;          // a32,a33
    ps_muls0    fp13,   fp2,  fp0;          // b02a30,b03a30
    psq_st      fp7 , 8(ab), 0, 0;          // ab02,ab03
    ps_madds1   fp13,   fp3,  fp0, fp13;    // b02a30+b12a31,b03a30+b13a31
    psq_st      fp9 ,24(ab), 0, 0;          // ab12,ab13

    // b02a30+b12a31+b22a32,b03a30+b13a31+b23a32
    ps_madds0   fp13,   fp4,  fp1, fp13;
    psq_st      fp11,40(ab), 0, 0;          // ab22,ab23

    // b02a30+b12a31+b22a32+b32a33,b03a30+b13a31+b23a32+b33a33
    ps_madds1   fp13,   fp5,  fp1, fp13;

    psq_st      fp13,56(ab), 0, 0;          // ab32,ab33
    blr
}
#endif

void nglApplyMatrix(nglVector& dst, const nglMatrix m, const nglVector v )
{
	NGL_ASSERT( &dst != &v, "" );

	float v0 = v[0];
	float v1 = v[1];
	float v2 = v[2];
	float v3 = v[3];

	dst[0] = v0 * m[0][0] + v1 * m[1][0] + v2 * m[2][0] + v3 * m[3][0];
	dst[1] = v0 * m[0][1] + v1 * m[1][1] + v2 * m[2][1] + v3 * m[3][1];
	dst[2] = v0 * m[0][2] + v1 * m[1][2] + v2 * m[2][2] + v3 * m[3][2];
	dst[3] = v0 * m[0][3] + v1 * m[1][3] + v2 * m[2][3] + v3 * m[3][3];
}

static NGL_INLINE void nglSpecialApplyMatrix( nglVector& dst, const nglMatrix m, const nglVector v )
{
	NGL_ASSERT( &dst != &v, "" );

	float v0 = v[0];
	float v1 = v[1];
	float v2 = v[2];

	dst[0] = v0 * m[0][0] + v1 * m[1][0] + v2 * m[2][0] + m[3][0];
	dst[1] = v0 * m[0][1] + v1 * m[1][1] + v2 * m[2][1] + m[3][1];
	dst[2] = v0 * m[0][2] + v1 * m[1][2] + v2 * m[2][2] + m[3][2];
}

static NGL_INLINE void nglSpecialNormalTransformMatrix( nglVector& dst, const nglMatrix m, const nglVector v )
{
	NGL_ASSERT( &dst != &v, "" );

	float v0 = v[0];
	float v1 = v[1];
	float v2 = v[2];

	dst[0] = v0 * m[0][0] + v1 * m[1][0] + v2 * m[2][0];
	dst[1] = v0 * m[0][1] + v1 * m[1][1] + v2 * m[2][1];
	dst[2] = v0 * m[0][2] + v1 * m[1][2] + v2 * m[2][2];
	dst[3] = 1.0f;
}

static NGL_INLINE void nglSpecialNormalTransformMatrix2( nglVector& dst, const nglMatrix m, const float *v )
{
	NGL_ASSERT( (float *)&dst != v, "" );

	float v0 = v[0];
	float v1 = v[1];
	float v2 = v[2];

	dst[0] = v0 * m[0][0] + v1 * m[1][0] + v2 * m[2][0];
	dst[1] = v0 * m[0][1] + v1 * m[1][1] + v2 * m[2][1];
	dst[2] = v0 * m[0][2] + v1 * m[1][2] + v2 * m[2][2];
}

void nglInverseMatrix( nglMatrix& dst, const nglMatrix src )
{
	nglMatrix m;
#ifdef NGL_FAST_MATH
	MTX44Inverse( src, m );
#else
	nglMatrix b;

	nglCopyMatrix( b, src );
	nglIdentityMatrix( m );

   	for (int i=0; i<4; ++i)
   	{
     	float val = b[i][i];      // find pivot
     	float aval=fabs(val);
     	int ind = i;
     	int j;
     	for (j=i+1; j<4; ++j)
     	{
       		float t=b[j][i];
       		float at=fabs(t);
       		if (at > aval)
       		{
         		ind = j;
         		val = t;
         		aval=at;
       		}
     	}

     	if (!aval)   // singular
       		return;

     	if (ind != i)      // swap columns
     	{
       		for (j=0; j<4; ++j)
       		{
         		float val2 = m[i][j];
         		m[i][j] = m[ind][j];
         		m[ind][j] = val2;
         		val2 = b[i][j];
         		b[i][j] = b[ind][j];
         		b[ind][j] = val2;
       		}
     	}

     	if (val!=1.0f)
     	{
       		float ival=1.0f/val;
       		for (j=0; j<4; ++j)
       		{
         		b[i][j] *= ival;
         		m[i][j] *= ival;
       		}
     	}

     	for (j=0; j<4; ++j)    // eliminate column
     	{
       		if (j == i)
         		continue;
       		float n = b[j][i];
       		if (n)
       		{
         		for (int k=0; k<4; ++k)
         		{
          			b[j][k] -= b[i][k] * n;
          			m[j][k] -= m[i][k] * n;
         		}
       		}
     	}
   	}
#endif
	nglCopyMatrix( dst, m );
}

void nglTransposeMatrix( nglMatrix& DstMat, nglMatrix SrcMat )
{
	NGL_ASSERT( &DstMat != &SrcMat, "" );
#ifdef NGL_FAST_MATH
	MTX44Transpose( SrcMat, DstMat );
#else
	DstMat[0][0] = SrcMat[0][0];
	DstMat[0][1] = SrcMat[1][0];
	DstMat[0][2] = SrcMat[2][0];
	DstMat[0][3] = SrcMat[3][0];

	DstMat[1][0] = SrcMat[0][1];
	DstMat[1][1] = SrcMat[1][1];
	DstMat[1][2] = SrcMat[2][1];
	DstMat[1][3] = SrcMat[3][1];

	DstMat[2][0] = SrcMat[0][2];
	DstMat[2][1] = SrcMat[1][2];
	DstMat[2][2] = SrcMat[2][2];
	DstMat[2][3] = SrcMat[3][2];

	DstMat[3][0] = SrcMat[0][3];
	DstMat[3][1] = SrcMat[1][3];
	DstMat[3][2] = SrcMat[2][3];
	DstMat[3][3] = SrcMat[3][3];
#endif
}

void nglCameraMatrix( nglMatrix& dst, const nglVector pos, const nglVector up, const nglVector at )
{
	Vec p, u, a;
	Mtx m;

	nglToVec( &p, pos );
	nglToVec( &u, up );
	nglToVec( &a, at );
	MTXLookAt( m, &p, &u, &a );
	mtxToNgl( dst, m );
}

void nglTransMatrix( nglMatrix& dst, const nglMatrix src, const nglVector trans )
{
	dst[3][0] = src[3][0] + trans[0];
	dst[3][1] = src[3][1] + trans[1];
	dst[3][2] = src[3][2] + trans[2];
}

void nglMatrixRot( nglMatrix& Out, const nglMatrix In, const nglVector rot)
{
	nglCopyMatrix( Out, In );
	MTXRotRad( (float (*)[4])&Out[0][0], 'X', rot[0] );
	MTXRotRad( (float (*)[4])&Out[0][0], 'Y', rot[1] );
	MTXRotRad( (float (*)[4])&Out[0][0], 'Z', rot[2] );
}

void nglInitRenderModeObj( void )
{
	GXRenderModeObj* rmo = NULL;

	// Initialize GX subsystem.
	switch(VIGetTvFormat())
	{
	case VI_NTSC:
		rmo = &GXNtsc480IntDf;
    	nglTVMode = NGLTV_NTSC;
		break;
	case VI_PAL:
		rmo = &GXPal528IntDf;
    	nglTVMode = NGLTV_PAL;
		break;
	case VI_MPAL:
		rmo = &GXMpal480IntDf;
    	nglTVMode = NGLTV_MPAL;
		break;
	case VI_EURGB60:
		rmo = &GXEurgb60Hz480IntDf;
    	nglTVMode = NGLTV_EURBG60;
		break;
	default:
		NGL_ASSERT(FALSE, "Bad TV Mode");
		break;
	}

	GXAdjustForOverscan(rmo, &nglRenderModeObj, 0, 16);

	memcpy(&nglRenderModeObj, rmo, sizeof(nglRenderModeObj));

	nglFrameBufferSize = OSRoundUp32B(VIPadFrameBufferWidth(nglRenderModeObj.fbWidth) * nglRenderModeObj.xfbHeight * VI_DISPLAY_PIX_SZ);
}

GXRenderModeObj *nglGetRenderMode()
{
  	return &nglRenderModeObj;
}

//-----------------------------------------------------------------------------

void nglInitWhiteTexture( void )
{
	nglWhite1x1Tex[0]=0xFFFF0000; // AR is 255.
	nglWhite1x1Tex[8]=0xFFFF0000; // GB is 255.

	memset( &nglWhiteTex, 0, sizeof(nglTexture));

	nglWhiteTex.Type = NGLTEX_GCT;
	nglWhiteTex.Flags = 0;
	nglWhiteTex.Width = 1;
	nglWhiteTex.Height = 1;
	nglWhiteTex.Data = (u8*) &nglWhite1x1Tex[0];
	nglWhiteTex.ImageData = (u8*) &nglWhite1x1Tex[0];
	nglWhiteTex.PaletteData = NULL;
	nglWhiteTex.Mipmaps = 0;
	nglWhiteTex.TexelFormat = GX_TF_RGBA8;
	nglWhiteTex.FileName = "white";

	memset( &nglWhiteMat, 0, sizeof(nglMaterial));

	nglWhiteMat.Flags |= NGLMAT_TEXTURE_MAP;
	nglWhiteMat.Flags |= NGLMAT_BACKFACE_DEFAULT;
	nglWhiteMat.Flags |= NGLMAT_BILINEAR_FILTER;
//	nglWhiteMat.Flags |= NGLMAT_PERSPECTIVE_CORRECT;
	nglWhiteMat.MapName = "white";
	nglWhiteMat.Map = &nglWhiteTex;
	nglWhiteMat.MapBlendMode = NGLBM_OPAQUE;
	nglWhiteMat.MapBlendModeConstant = 1.0f;
	nglWhiteMat.Color = 0xFFFFFFFF;

	DCStoreRange(&nglWhiteTex, sizeof(nglTexture));
	DCStoreRange(&nglWhiteMat, sizeof(nglMaterial));
	DCStoreRange(nglWhite1x1Tex, sizeof(u32) * 16);

	GXInitTexObj( &nglWhiteTex.TexObj,
								nglWhiteTex.ImageData,
								nglWhiteTex.Width,
								nglWhiteTex.Height,
								GX_TF_RGBA8,
								GX_REPEAT,
								GX_REPEAT,
								GX_FALSE );

}

//-----------------------------------------------------------------------------
#if defined(NGL_GC_PERFLIB)
static void nglPERFFree(void *ptr)
{
  	nglMemFree( ptr );
}

static void *nglPERFMalloc(u32 size)
{
  	if (size > 0)
    	return nglMemAlloc( size, 32 );

	return NULL;
}

static void nglPERFStateRestore( void )
{
}
#endif //NGL_GC_PERFLIB

void _nglSetDefaultSceneParams()
{
	memset(nglCurScene, 0, sizeof(nglScene));

  nglSetRenderTarget( NULL );
  nglSetViewport( 0, 0, nglGetScreenWidth() - 1, nglGetScreenHeight() - 1 );

	nglSetAspectRatio(4.0f/3.0f);
  nglSetPerspectiveMatrix(60, 0.1f, 10000.0f);
	#ifdef PROJECT_KELLYSLATER
	nglSetPerspectiveCXCY( nglGetScreenWidth()/2.0f, nglGetScreenHeight()/2.0f );
	#endif

  nglSetWorldToViewMatrix(nglUnitMatrix);

  nglSetClearFlags( NGLCLEAR_Z );
  nglSetClearColor( 0, 0, 0, 0 );
  nglSetClearZ( 1.0f );

  nglSetZWriteEnable( true );
  nglSetZTestEnable( true );

  nglEnableFog(false);
  nglSetFogRange(0.0f, 10000.0f, 0.0f, 1.0f);
  nglSetFogColor(1.0f, 1.0f, 1.0f);

  nglSetAnimTime(0.0f);
}

//-----------------------------------------------------------------------------

void nglPostRetraceCallback( u32 count )
{

	if( nglCopyToXFB ) {

		if( nglFirstFlip ) {
			VISetBlack( FALSE );
			VIFlush( );
			nglFirstFlip = false;
		}

		GXCopyDisp( nglFrameBuffer, GX_FALSE );
		GXFlush( );
		nglCopyToXFB = false;
	}

}

void nglInit( void )
{
  	nglPrintf( TTY_BLACK "\n" );
  	nglPrintf( TTY_BLUE   "-----------------------------------------------------------------\n" TTY_BLACK );
  	nglPrintf( TTY_RED    "  Nyarlathotep's Graphics Laboratory " NGL_VERSION_STR "\n" );
  	nglPrintf( TTY_YELLOW "  Copyright (c) 2001, 2002 Treyarch Corporation\n" );
  	nglPrintf( TTY_BLACK "\n" );

  	OSInitFastCast();

	// clear stage to enable all
	memset(&nglStage, 1, sizeof(nglStage));

	// setup our internal identity matricies
	MTXIdentity(_nglMTXIdentity);
	MTXIdentity(_nglMTXIdentityInvZ);
	_nglMTXIdentityInvZ[2][2] = -1.0f;

	// setup our internal colors
	_nglGXColorZero.r = _nglGXColorZero.g = _nglGXColorZero.b = _nglGXColorZero.a = 0x00;
	_nglGXColorOne.r  = _nglGXColorOne.g  = _nglGXColorOne.b  = _nglGXColorOne.a  = 0xFF;

	NGL_ASSERT((nglFifoSize%32) == 0, "FIFO Size must be a multiple of 32");

	// Initialize the graphics system.
	nglFifo = nglMemAlloc(nglFifoSize, 32);
	nglFifoObj = GXInit(nglFifo, nglFifoSize);

	nglInitRenderModeObj( );

	GXSetMisc( GX_MT_XF_FLUSH, GX_XF_FLUSH_SAFE );
	GXSetVerifyLevel( GX_WARN_NONE );

	VISetPostRetraceCallback( nglPostRetraceCallback );

	VIConfigure( &nglRenderModeObj );
	VIFlush( );

	if( !nglFrameBuffer )
	{
		nglFrameBuffer = nglMemAlloc( nglFrameBufferSize, 32 );
	}

	// allocate our work buffers
 	if (!nglListWork)
 	{
		nglSetBufferSize(NGLBUF_LIST_WORK, 256*1024);
	}

	GXSetViewport(0.0f, 0.0f, nglRenderModeObj.fbWidth, nglRenderModeObj.xfbHeight, 0.0f, 1.0f);
	GXSetScissor( 0, 0, nglRenderModeObj.fbWidth, nglRenderModeObj.efbHeight );
	GXSetDispCopySrc( 0, 0, nglRenderModeObj.fbWidth, nglRenderModeObj.efbHeight );
	GXSetDispCopyDst( nglRenderModeObj.fbWidth, nglRenderModeObj.xfbHeight );
	float ratio = ( (float) nglRenderModeObj.xfbHeight ) / ( (float) nglRenderModeObj.efbHeight );
	GXSetDispCopyYScale( ratio );
	GXSetCopyFilter( nglRenderModeObj.aa, nglRenderModeObj.sample_pattern,
									 GX_TRUE, nglRenderModeObj.vfilter );

	// 08-Nov-2001 SAL: GX_PF_RGBA6_Z24 is the only format that supports Alpha in EFB.
	//  The only reason we need this is for the alpha requirement in some RenderToTextures.
	//  No reason as yet to look for a more complex solution that would use 8 bits for RGBs.
	GXSetPixelFmt( GX_PF_RGBA6_Z24, GX_ZC_LINEAR ); // Only format that supports Alpha in EFB.

	nglInstanceBank::SetAllocFunc( nglMemAlloc );
	nglInstanceBank::SetFreeFunc( nglMemFree );

  nglMeshPath[0] = '\0';
  nglTexturePath[0] = '\0';

	nglMeshBank.Init( );
	nglMeshFileBank.Init( );
	nglTextureBank.Init( );
	nglFontBank.Init( );

	nglSkinWork = NULL;
	memset( nglFonts, 0, sizeof(nglFonts) );

	// Initialize standard system resources.
	nglInitWhiteTexture();

  // Load up the debugging meshes.
#ifdef NGL_DEBUG
	nglLoadMeshFileInPlace("nglsphere", nglSphereMeshData);
	nglLoadMeshFileInPlace("nglradius", nglRadiusMeshData);
	nglSphereMesh = nglGetMesh("ngl_sphere");
	nglRadiusMesh = nglGetMesh("ngl_radius");
#endif

	// Get the system font texture from memory (embedded cpp file).
	nglTexture *FontTex = nglLoadTextureInPlace("nglSysFont", NGLTEX_GCT, nglSysFontGCT, sizeof(nglSysFontGCT));
	FontTex->Flags |= NGLGC_TEXFLAG_SYSTEM;
	// Get the system font FDF from memory (embedded cpp file).
	nglSysFont = nglLoadFontInPlace("nglSysFont", nglSysFontFDF);

	nglCurScene = &nglDefaultScene;
	_nglSetDefaultSceneParams();

#if defined(NGL_GC_PERFLIB)
	PERFInit( 60, 1, NGL_NUM_PERF_EVENTS, nglPERFMalloc, nglPERFFree, nglPERFStateRestore );
	for( int i = 0; i < ( NGL_NUM_PERF_EVENTS ) - 1; i++ )
	{
  	PERFSetEvent( (unsigned char)nglPERFEvents[i].id, nglPERFEvents[i].name, (PerfType) nglPERFEvents[i].type );
  	PERFSetEventColor( nglPERFEvents[i].id, nglPERFEvents[i].color );
	}
	PERFSetDrawBWBarKey( 1 );
	PERFSetDrawBWBar( 1 );
	PERFSetDrawCPUBar( 1 );
	PERFSetDrawXFBars( 1 );
	PERFSetDrawRASBar( 1 );
#endif

	nglListWorkPos = nglListWork;

	VISetNextFrameBuffer( nglFrameBuffer );
	VIFlush( );
}

void nglExit()
{
	// FIXME
}

void nglSetBufferSize(u_int BufID, u_int Size)
{
  	switch (BufID)
  	{
	case NGLBUF_GX_FIFO:
		NGL_ASSERT(nglFifo == NULL, "Fifo size cannot be changed after NGL Init");
		nglFifoSize = Size;
		break;
  	case NGLBUF_LIST_WORK:
    	if (nglListWork)
      		nglMemFree(nglListWork);
    	nglListWork = (u_char*)nglMemAlloc(Size, 32);
    	nglListWorkSize = Size;
    	break;
	default:
		nglFatal("Invalid Enum sent to nglSetBufferSize");
  	}
}

u_int nglGetVBlankCount()
{
	return nglVBlankCount;
}

void nglSetRenderTarget( nglTexture* Tex, bool Download )
{
	nglCurScene->RenderTarget = Tex;
	nglCurScene->Download = Download;
}

void nglSetViewport( float x1, float y1, float x2, float y2 )
{
	nglCurScene->ViewX1 = x1;
  	nglCurScene->ViewY1 = y1;
  	nglCurScene->ViewX2 = x2;
  	nglCurScene->ViewY2 = y2;
}

void nglSetClearFlags( u_int ClearFlags )
{
	nglCurScene->ClearFlags = ClearFlags;
}

void nglSetClearColor( float R, float G, float B, float A )
{
	u8 r, g, b, a;

	r = nglFTOI(R * 255.0f);
  	g = nglFTOI(G * 255.0f);
  	b = nglFTOI(B * 255.0f);
  	a = nglFTOI(A * 255.0f);

  	nglCurScene->ClearColor = NGL_RGBA32(r, g, b, a);
}

void nglSetClearZ( float Z )
{
	nglCurScene->ClearZ = Z;
}

void nglSetZWriteEnable( bool enable )
{
	nglCurScene->ZWriteEnable = enable;
}

void nglSetZTestEnable( bool enable )
{
	nglCurScene->ZTestEnable = enable;
}

static NGL_INLINE void SceneGXSetZMode(bool ZTestEnable, bool ZWriteEnable)
{
  	if (!nglCurScene->ZTestEnable) ZTestEnable = false;
  	if (!nglCurScene->ZWriteEnable) ZWriteEnable = false;
  	GXSetZMode( ((ZWriteEnable || ZTestEnable) ? GX_TRUE : GX_FALSE),
    	(ZTestEnable ? GX_LEQUAL : GX_ALWAYS),
        (ZWriteEnable ? GX_TRUE : GX_FALSE ) );
}

void nglEnableFog(bool bEnable)
{
	nglCurScene->FogEnabled = bEnable;
}

void nglSetFogColor( float R, float G, float B )
{
  	nglCurScene->FogColor.r = (u8) ( R * 255 );
  	nglCurScene->FogColor.g = (u8) ( G * 255 );
  	nglCurScene->FogColor.b = (u8) ( B * 255 );
	nglCurScene->FogType = NGL_GC_FOG_LIN;
}

void nglSetFogRange( float Near, float Far, float Min, float Max )
{
  	nglCurScene->FogNear = Near;
  	nglCurScene->FogFar = Far;
}

//-----------------------------------------------------------------------------

void nglGCSetFog( int FogType, float StartZ, float EndZ, float R, float G, float B )
{
	NGL_ASSERT( FogType < NGL_GC_MAX_FOG_TYPE, "" );
  	nglCurScene->FogNear = StartZ;
  	nglCurScene->FogFar = EndZ;
  	nglCurScene->FogColor.r = (u8) ( R * 255 );
  	nglCurScene->FogColor.g = (u8) ( G * 255 );
  	nglCurScene->FogColor.b = (u8) ( B * 255 );
}

/*===========================================================================*


    GP hang work-around auto-recovery system


 *===========================================================================*/

/*---------------------------------------------------------------------------*
    Automatic frame skipping to work around GP hang condition.
 *---------------------------------------------------------------------------*/
static BOOL GPHangWorkaround = FALSE;

// used to count missed frames by VI retrace callback handler
static vu32 FrameCount;
static u32  FrameMissThreshold; // number of frames to be considered a timeout

// tokens are used instead of GXDrawDone
#define DEMO_START_FRAME_TOKEN  0xFEEB
#define DEMO_END_FRAME_TOKEN    0xB00B

static void __NoHangRetraceCallback ( u32 count );
static void __NoHangDoneRender      ( void );

static void  __DEMODiagnoseHang     ( void );

void DEMOSetGPHangMetric( GXBool enable );

/*---------------------------------------------------------------------------*
    Name:           DEMOEnableGPHangWorkaround

    Description:    Sets up the DEMO library to skip past any GP hangs and
                    attempt to repair the graphics pipe whenever a timeout
                    of /timeoutFrames/ occurs.  This will serve as a
                    temporary work-around for any GP hangs that may occur.

    Arguments: timeoutFrames        The number of 60hz frames to wait in
                                    DEMODoneRender before aborting the
                                    graphics pipe.  Should be at least
                                    equal to your standard frame rate
                                    (e.g. 60hz games should use a value of 1,
                                    30hz games should use a value of 2)

    Returns:        None
 *---------------------------------------------------------------------------*/
void DEMOEnableGPHangWorkaround ( u32 timeoutFrames )
{
#ifdef EMU
    #pragma unused (timeoutFrames)
#else
    if (timeoutFrames)
    {
        GPHangWorkaround = TRUE;
        FrameMissThreshold = timeoutFrames;
        VISetPreRetraceCallback( __NoHangRetraceCallback );

        // Enable counters for post-hang diagnosis
        DEMOSetGPHangMetric( GX_TRUE );
    }
    else
    {
        GPHangWorkaround = FALSE;
        FrameMissThreshold = 0;
        DEMOSetGPHangMetric( GX_FALSE );
        VISetPreRetraceCallback( NULL );
    }
#endif
}

/*---------------------------------------------------------------------------*
    Name:           __NoHangRetraceCallback

    Description:    VI callback to count missed frames for GPHangWorkaround

                    Also checks for hangs during high watermark condition.

    Arguments:      Unused

    Returns:        None
 *---------------------------------------------------------------------------*/
static void __NoHangRetraceCallback ( u32 count )
{
    #pragma unused (count)
    static u32 ovFrameCount = 0;
    static u32 lastOvc = 0;
    u32        ovc;
    GXBool     overhi, junk;

    // Increment the frame counter used by __NoHangDoneRender.

    FrameCount++;

    // Check the high watermark status.
    // If we're still in the same high watermark state as before,
    // increment the counter and check for time-out.

    GXGetGPStatus(&overhi, &junk, &junk, &junk, &junk);
    ovc = GXGetOverflowCount();

    if (overhi && (ovc == lastOvc))
    {
        ovFrameCount++;
        if (ovFrameCount >= FrameMissThreshold)
        {
            // We timed out.  Report and diagnose the hang.
            OSReport("---------WARNING : HANG AT HIGH WATERMARK----------\n");

            __DEMODiagnoseHang();

            // We cannot easily recover from this situation.  Halt program.
            OSHalt("Halting program");
        }
    }
    else
    {
        lastOvc = ovc;
        ovFrameCount = 0;
    }
}

/*---------------------------------------------------------------------------*
    Name:           __NoHangDoneRender

    Description:    Called in lieu of the standard DEMODoneRender if
                    GPHangWorkaround == TRUE.

                    Uses a token to check for end of frame so that we can
                    also count missed frames at the same time.

    Arguments:      None

    Returns:        None
 *---------------------------------------------------------------------------*/
static void __NoHangDoneRender()
{
    BOOL abort = FALSE;
    GXCopyDisp(nglFrameBuffer, GX_TRUE);
    GXSetDrawSync( DEMO_END_FRAME_TOKEN );

    FrameCount = 0;

    while( (GXReadDrawSync() != DEMO_END_FRAME_TOKEN ) && !abort )
    {
        if (FrameCount >= FrameMissThreshold)
        {
            OSReport("---------WARNING : ABORTING FRAME----------\n");
            abort = TRUE;
            __DEMODiagnoseHang();
            // FIXME: fix this
//            DEMOReInit(rmode); // XXX RMODE?
            // re-enable counters for post-hang diagnosis
            DEMOSetGPHangMetric( GX_TRUE );
        }
    }

}

/*---------------------------------------------------------------------------*
    Name:           DEMOSetGPHangMetric

    Description:    Sets up the GP performance counters in such a way to
                    enable us to detect the cause of a GP hang.  Note that
                    this takes over the performance counters, and you cannot
                    use GXSetGPMetric or GXInitXFRasMetric while you have
                    DEMOSetGPHangMetric enabled.

    Arguments:      enable:  set to GX_TRUE to enable the counters.
                             set to GX_FALSE to disable the counters.

    Returns:        None
 *---------------------------------------------------------------------------*/
void DEMOSetGPHangMetric( GXBool enable )
{
#ifdef EMU
    #pragma unused (enable)
#else
    if (enable)
    {
        // Ensure other counters are off
        GXSetGPMetric( GX_PERF0_NONE, GX_PERF1_NONE );

        // Set up RAS Ready counter
        GXWGFifo.u8  = GX_LOAD_BP_REG;
        GXWGFifo.u32 = 0x2402c004; // ... 101 10000 00000 00100

        // Set up SU Ready counter
        GXWGFifo.u8  = GX_LOAD_BP_REG;
        GXWGFifo.u32 = 0x23000020; // ... 100 000

        // Set up XF TOP and BOT busy counters
        GXWGFifo.u8  = GX_LOAD_XF_REG;
        GXWGFifo.u16 = 0x0000;
        GXWGFifo.u16 = 0x1006;
        GXWGFifo.u32 = 0x00084400; // 10000 10001 00000 00000
    }
    else
    {
        // Disable RAS counters
        GXWGFifo.u8  = GX_LOAD_BP_REG;
        GXWGFifo.u32 = 0x24000000;

        // Disable SU counters
        GXWGFifo.u8  = GX_LOAD_BP_REG;
        GXWGFifo.u32 = 0x23000000;

        // Disable XF counters
        GXWGFifo.u8  = GX_LOAD_XF_REG;
        GXWGFifo.u16 = 0x0000;
        GXWGFifo.u16 = 0x1006;
        GXWGFifo.u32 = 0x00000000;
    }
#endif
}

/*---------------------------------------------------------------------------*
    Name:           __DEMODiagnoseHang

    Description:    Reads performance counters (which should have been set
                    up appropriately already) in order to determine why the
                    GP hung.  The counters must be set as follows:

                    GXSetGPHangMetric( GX_TRUE );

                    The above call actually sets up multiple counters, which
                    are read using a non-standard method.

    Arguments:      None

    Returns:        None
 *---------------------------------------------------------------------------*/
static void __DEMODiagnoseHang()
{
    u32 xfTop0, xfBot0, suRdy0, r0Rdy0;
    u32 xfTop1, xfBot1, suRdy1, r0Rdy1;
    u32 xfTopD, xfBotD, suRdyD, r0RdyD;
    GXBool readIdle, cmdIdle, junk;

    // Read the counters twice in order to see which are changing.
    // This method of reading the counters works in this particular case.
    // You should not use this method to read GPMetric counters.
    GXReadXfRasMetric( &xfBot0, &xfTop0, &r0Rdy0, &suRdy0 );
    GXReadXfRasMetric( &xfBot1, &xfTop1, &r0Rdy1, &suRdy1 );

    // XF Top & Bot counters indicate busy, others indicate ready.
    // Convert readings into indications of who is ready/idle.
    xfTopD = (xfTop1 - xfTop0) == 0;
    xfBotD = (xfBot1 - xfBot0) == 0;
    suRdyD = (suRdy1 - suRdy0) > 0;
    r0RdyD = (r0Rdy1 - r0Rdy0) > 0;

    // Get CP status
    GXGetGPStatus(&junk, &junk, &readIdle, &cmdIdle, &junk);

    OSReport("GP status %d%d%d%d%d%d --> ",
             readIdle, cmdIdle, xfTopD, xfBotD, suRdyD, r0RdyD);

    // Depending upon which counters are changing, diagnose the hang.
    // This may not be 100% conclusive, but it's what we've observed so far.
    if (!xfBotD && suRdyD)
    {
        OSReport("GP hang due to XF stall bug.\n");
    }
    else if (!xfTopD && xfBotD && suRdyD)
    {
        OSReport("GP hang due to unterminated primitive.\n");
    }
    else if (!cmdIdle && xfTopD && xfBotD && suRdyD)
    {
        OSReport("GP hang due to illegal instruction.\n");
    }
    else if (readIdle && cmdIdle && xfTopD && xfBotD && suRdyD && r0RdyD)
    {
        OSReport("GP appears to be not hung (waiting for input).\n");
    }
    else
    {
        OSReport("GP is in unknown state.\n");
    }
}

//-----------------------------------------------------------------------------

#ifdef NGL_GC_NOA_SCREEN_CAPTURE
#include "screenshot.c"

void* ScreenShotAlloc(u32 size) { return nglMemAlloc(size, 32); }
void ScreenShotFree(void *ptr) { nglMemFree(ptr); }
#endif

bool nglGCEnableScreenShotService = false;

// non threaded version
void nglFlip( void )
{
	static OSTick cycle, lastcycle, diffcycle;

	if( GPHangWorkaround )
	{
		__NoHangDoneRender();
		return;
	}

	if (nglDebug.ScreenShot)
	{
		nglScreenShot();
		nglDebug.ScreenShot = 0;
	}

	// once the frame ends stop save the render targets
	if (nglDebug.RenderTargetShot == 2)
		nglDebug.RenderTargetShot = 0;

#ifdef NGL_GC_NOA_SCREEN_CAPTURE
	if (nglGCEnableScreenShotService)
	  SCREENSHOTService( nglFrameBuffer, ScreenShotAlloc, ScreenShotFree );
#endif

#if NGL_PROFILING
	// get render timing
	nglPerfInfo.RenderCycles = OSGetTick() - nglPerfInfo.RenderCycles;
	nglPerfInfo.RenderMS = (float)nglPerfInfo.RenderCycles / NGL_CPU_CLOCKS_PER_MS;
#endif // NGL_PROFILING

#if NGL_PROFILING
	// Calculate current frame rate.
	cycle = OSGetTick();
	diffcycle = cycle - lastcycle;
	lastcycle = cycle;
	nglPerfInfo.FPS = (float)OS_TIMER_CLOCK / (float)diffcycle;
#endif // NGL_PROFILING

#if defined(NGL_GC_PERFLIB)
	static bool done_loop = false;

	if( done_loop == true )
	{
  	PERFEndFrame();
  	PERFStopAutoSampling();
	}
	else
	{
  	done_loop = true;
	}

	DCInvalidateRange(nglPERFDL, nglPERFDLSize);
	GXBeginDisplayList(nglPERFDL, nglPERFDLSize);
	PERFDumpScreen();
	nglPERFDLRenderSize = GXEndDisplayList();

	PERFStartAutoSampling( 1.0f );
	PERFStartFrame();
#endif

	GXDrawDone( );
	nglCopyToXFB = true;
	VIWaitForRetrace( );

	// update all the frame variables
	nglVBlankCount = VIGetRetraceCount();
	nglTextureAnimFrame = nglVBlankCount * (nglGCAnimFramesPerSecond / 60.0f);
	nglFrame++;
}

void nglNormalizePlane( nglVector& o, nglVector p )
{
  	float l = sqrtf( p[0] * p[0] + p[1] * p[1] + p[2] * p[2] );
	nglDivVector( o, p, l );
}

//-----------------------------------------------------------------------------

void nglSetAspectRatio(float a)
{
	nglCurScene->Aspect = a;
}

// takes and returns degrees
static inline float nglCalcVFov(float fovx, float aspect)
{
	float	a;
	float	x;
	float   width = aspect;
	float   height = 1.0f;

	if (fovx < 1.0f || fovx > 179.0f)
		fovx = 90.0f;	// error, set to 90

	x = width/tanf(fovx/360.0f*M_PI);

	a = atanf(height/x);

	a = a*360.0f/M_PI;

	return a;
}

//-----------------------------------------------------------------------------

void CPUMTXOrtho(nglMatrix& m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f )
{
  	memset( m, 0, sizeof(nglMatrix) );
  	m[0][0] = 2/(r-l);
  	m[0][3] = -(r+l)/(r-l);
  	m[1][1] = 2/(t-b);
  	m[1][3] = -(t+b)/(t-b);
  	m[2][2] = -1/(f-n);
  	m[2][3] = -f/(f-n);
  	m[3][3] = 1;
}


//-----------------------------------------------------------------------------

// FIXME: this should be calculated once (per scene?) at render time
void nglSetOrthoMatrix(float t, float b, float l, float r, float nearz, float farz)
{
	// Cache values.
	nglCurScene->Top = t;
	nglCurScene->Bottom = b;
	nglCurScene->Left = l;
	nglCurScene->Right = r;
  	nglCurScene->NearZ = nearz;
  	nglCurScene->FarZ = farz;
	nglCurScene->ViewType = NGL_VIEW_ORTHOGRAPHIC;

	int Height, Width;
  	if (nglCurScene->RenderTarget)
  	{
  		Height = nglCurScene->RenderTarget->Height;
  		Width = nglCurScene->RenderTarget->Width;
  	}
  	else
  	{
  		Height = nglRenderModeObj.xfbHeight;
  		Width = nglRenderModeObj.fbWidth;
  	}

	CPUMTXOrtho(nglCurScene->ViewToScreen, t, b, l, r, nearz, farz);

	// FIXME: make sure these are still valid
	nglCurScene->QuadZCompParam[0] = nglCurScene->ViewToScreen[2][2];
	nglCurScene->QuadZCompParam[1] = nglCurScene->ViewToScreen[2][3];
}

void nglGetProjectionParams(float* hfov, float* nearz, float* farz)
{
  	if ( hfov ) *hfov = nglCurScene->HFOV;
  	if ( nearz ) *nearz = nglCurScene->NearZ;
  	if ( farz ) *farz = nglCurScene->FarZ;
}

//-----------------------------------------------------------------------------

void CPUMTXPerspective(nglMatrix& m, f32 fovY, f32 aspect, f32 n, f32 f )
{
  	memset( m, 0, sizeof(nglMatrix) );
  	f32 TanHalfFovY =  tan(NGL_DEGTORAD(fovY/2));
  	m[0][0] = 1/(TanHalfFovY*aspect);
  	m[1][1] = 1/(TanHalfFovY);
  	m[2][2] = -n/(f-n);
  	m[2][3] = -(f*n)/(f-n);
  	m[3][2] = -1;
}

//-----------------------------------------------------------------------------

#ifdef PROJECT_KELLYSLATER
void nglSetPerspectiveCXCY( float cx, float cy )
{
  nglCurScene->CX=cx;
  nglCurScene->CY=cy;
}
#endif

void nglSetPerspectiveMatrix( float hfovDeg, float nearz, float farz)
{
	// On the gamecube we have to specify a vertical field
	// of view that corresponds to the specified horizontal
	// field of view.
	float vfov = nglCalcVFov(hfovDeg, nglCurScene->Aspect);

	CPUMTXPerspective(nglCurScene->ViewToScreen, vfov, nglCurScene->Aspect, nearz, farz );

  	// Cache values.
  	nglCurScene->HFOV = hfovDeg;
  	nglCurScene->NearZ = nearz;
  	nglCurScene->FarZ = farz;
	nglCurScene->ViewType = NGL_VIEW_PERSPECTIVE;
	nglCurScene->QuadZCompParam[0] = nglCurScene->ViewToScreen[2][2];
	nglCurScene->QuadZCompParam[1] = nglCurScene->ViewToScreen[2][3];

  	// Initialize the clip planes (these are for mesh sphere rejection only).
  	nglCurScene->ClipPlanes[NGLCLIP_NEAR][0] = 0.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_NEAR][1] = 0.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_NEAR][2] = 1.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_NEAR][3] = nearz;

  	nglCurScene->ClipPlanes[NGLCLIP_FAR][0] = 0.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_FAR][1] = 0.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_FAR][2] = -1.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_FAR][3] = -farz;

	float hfov = NGL_DEGTORAD(hfovDeg);
  	nglCurScene->ClipPlanes[NGLCLIP_LEFT][0] = cosf(hfov);  // lefT
  	nglCurScene->ClipPlanes[NGLCLIP_LEFT][1] = 0.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_LEFT][2] = sinf(hfov);
  	nglCurScene->ClipPlanes[NGLCLIP_LEFT][3] = 0.0f;

  	nglCurScene->ClipPlanes[NGLCLIP_RIGHT][0] = -cosf(hfov);  // righT
  	nglCurScene->ClipPlanes[NGLCLIP_RIGHT][1] = 0.0f;
  	nglCurScene->ClipPlanes[NGLCLIP_RIGHT][2] = sinf(hfov);
  	nglCurScene->ClipPlanes[NGLCLIP_RIGHT][3] = 0.0f;

	nglCurScene->ClipPlanes[NGLCLIP_TOP][0] = 0.0f;        // Top
  	nglCurScene->ClipPlanes[NGLCLIP_TOP][1] = cosf(hfov);
  	nglCurScene->ClipPlanes[NGLCLIP_TOP][2] = sinf(hfov);
  	nglCurScene->ClipPlanes[NGLCLIP_TOP][3] = 0.0f;

  	nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][0] = 0.0f;       // boTTom
  	nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][1] = -cosf(hfov);
  	nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][2] = sinf(hfov);
  	nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][3] = 0.0f;

  	nglMulMatrix( nglCurScene->WorldToScreen, nglCurScene->WorldToView, nglCurScene->ViewToScreen );
}

int nglGetScreenWidth()
{
	// FIXME: we probably shoudln't support this before nglInit
	if (nglRenderModeObj.fbWidth)
	{
		return nglRenderModeObj.fbWidth;
	}
	else
	{
		nglWarning("nglGetScreenWidth called before nglInit\n");
		return 640;
	}
}

int nglGetScreenHeight()
{
	// FIXME: we probably shoudln't support this before nglInit
	if( nglRenderModeObj.efbHeight )
	{
		return nglRenderModeObj.efbHeight;
	}
	else
	{
		nglWarning("nglGetScreenHeight called before nglInit\n");
		return 448;
	}
}
void nglSetWorldToViewMatrix( const nglMatrix WorldToView )
{
	nglCopyMatrix( nglCurScene->WorldToView, WorldToView );
  	nglMulMatrix( nglCurScene->WorldToScreen, nglCurScene->WorldToView, nglCurScene->ViewToScreen );
	nglInverseMatrix( nglCurScene->ViewToWorld, nglCurScene->WorldToView );
}

void nglGetMatrix( nglMatrix& Dest, u_int ID )
{
  	switch( ID )
	{
  	case NGLMTX_VIEW_TO_WORLD:
    	nglCopyMatrix( Dest, nglCurScene->ViewToWorld );
    	break;
  	case NGLMTX_VIEW_TO_SCREEN:
  	  	nglCopyMatrix( Dest, nglCurScene->ViewToScreen );
  	  	break;
  	case NGLMTX_WORLD_TO_VIEW:
  	  	nglCopyMatrix( Dest, nglCurScene->WorldToView );
  	  	break;
  	case NGLMTX_WORLD_TO_SCREEN:
  	  	nglCopyMatrix( Dest, nglCurScene->WorldToScreen );
  	  	break;
  	}
}

void nglSetMeshPath( const char* Path )
{
  	strncpy( nglMeshPath, Path, sizeof(nglMeshPath) );
  	nglMeshPath[sizeof(nglMeshPath) - 1] = 0;
}

void nglSetTexturePath( const char* Path )
{
  	strncpy( nglTexturePath, Path, sizeof(nglTexturePath) );
  	nglTexturePath[sizeof(nglTexturePath) - 1] = 0;
}

const char *nglGetMeshPath( void )
{
	return nglMeshPath;
}

const char *nglGetTexturePath( void )
{
	return nglTexturePath;
}

void nglSetAnimTime( float Time )
{
	nglTextureAnimFrame = Time;
	nglCurScene->AnimTime = Time;
}

#if NGL_DEBUGGING
u_char* nglGetDebugFlagPtr( const char* Flag )
{
  	// Stage flags.
  	if ( stricmp( Flag, "Clip" ) == 0 )
    	return &nglStage.Clip;
  	else if ( stricmp( Flag, "Skin" ) == 0 )
    	return &nglStage.Skin;
// 	else if ( stricmp( Flag, "XForm" ) == 0 )
//    	return &nglStage.XForm;
// 	else if ( stricmp( Flag, "Backface" ) == 0 )
//    	return &nglStage.Backface;
// 	else if ( stricmp( Flag, "Light" ) == 0 )
//    	return &nglStage.Light;
// 	else if ( stricmp( Flag, "PlaneClip" ) == 0 )
//    	return &nglStage.PlaneClip;
// 	else if ( stricmp( Flag, "DetailPass" ) == 0 )
//    	return &nglStage.DetailPass;
// 	else if ( stricmp( Flag, "EnvironmentPass" ) == 0 )
//    	return &nglStage.EnvironmentPass;
// 	else if ( stricmp( Flag, "LightPass" ) == 0 )
//    	return &nglStage.LightPass;
  	// Debug flags.
  	else if ( stricmp( Flag, "ShowPerfInfo" ) == 0 )
    	return &nglDebug.ShowPerfInfo;
// 	else if ( stricmp( Flag, "DisableQuads" ) == 0 )
//    	return &nglDebug.DisableQuads;
// 	else if ( stricmp( Flag, "DisableTextures" ) == 0 )
//    	return &nglDebug.DisableTextures;
// 	else if ( stricmp( Flag, "DisableMipmaps" ) == 0 )
//    	return &nglDebug.DisableMipmaps;
// 	else if ( stricmp( Flag, "DisableVSync" ) == 0 )
//    	return &nglDebug.DisableVSync;
// 	else if ( stricmp( Flag, "DisableScratch" ) == 0 )
//    	return &nglDebug.DisableScratch;
// 	else if ( stricmp( Flag, "DebugPrints" ) == 0 )
//   	return &nglDebug.DebugPrints;
// 	else if ( stricmp( Flag, "DumpFrameLog" ) == 0 )
//   	return &nglDebug.DumpFrameLog;
// 	else if ( stricmp( Flag, "DumpSceneFile" ) == 0 )
//    	return &nglDebug.DumpSceneFile;
  	else if ( stricmp( Flag, "DrawMeshSpheres" ) == 0 )
    	return &nglDebug.DrawMeshSpheres;
// 	else if ( stricmp( Flag, "DrawNormals" ) == 0 )
//    	return &nglDebug.DrawNormals;
// 	else if ( stricmp( Flag, "DrawAsPoints" ) == 0 )
//    	return &nglDebug.DrawAsPoints;
  	else if ( stricmp( Flag, "DrawAsLines" ) == 0 )
    	return &nglDebug.DrawAsLines;
// 	else if ( stricmp( Flag, "DrawToFrontBuffer" ) == 0 )
//    	return &nglDebug.DrawToFrontBuffer;
  	else
    	return NULL;
}
#endif

void nglSetDebugFlag(const char* Flag, bool Set)
{
#if NGL_DEBUGGING
  	u_char* Ptr = nglGetDebugFlagPtr(Flag);
  	if (Ptr)
    	*Ptr = Set;
#endif
}

bool nglGetDebugFlag(const char* Flag)
{
#if NGL_DEBUGGING
  	u_char* Ptr = nglGetDebugFlagPtr(Flag);
  	if (Ptr)
    	return *Ptr;
  	else
	{
		nglWarning("nglGetDebugFlag: %s not found\n", Flag);
    	return false;
	}
#else
	return false;
#endif
}

NGL_INLINE void nglApplyMatrixPlane( nglVector& Out, nglMatrix Mat, nglVector Plane )
{
  	nglVector Pt;
  	nglVector Norm;

  	// Get plane normal and a reference point.
  	nglCopyVector( Norm, Plane );
  	Norm[3] = 0.0f;
  	nglScaleVectorXYZ( Pt, Norm, Plane[3] );
  	Pt[3] = 1.0f;

  	// Transform point.
  	nglApplyMatrix( Pt, Mat, Pt );

  	// Reorient normal.
  	nglApplyMatrix( Norm, Mat, Norm );

  	// Reconstruct plane.
  	nglCopyVector( Out, Norm );
  	Out[3] = nglInnerProduct( Pt, Norm );
}

NGL_INLINE int nglSpheresIntersect( nglVector Center1, float Radius1, nglVector Center2, float Radius2 )
{
  	nglVector V;
  	float Dist, Range;

  	nglSubVector( V, Center1, Center2 );
  	Dist = V[0] * V[0] + V[1] * V[1] + V[2] * V[2];

  	Range = Radius1 + Radius2;
  	return Dist <= Range * Range;
}

//----------------------------------------------------------------------------------------
//  @Screenshot code.
//----------------------------------------------------------------------------------------

/*
**  TGA File Header
*/
typedef struct _TgaHeader
{
    unsigned char   IdLength;            /* Image ID Field Length      */
    unsigned char   CmapType;            /* Color Map Type             */
    unsigned char   ImageType;           /* Image Type                 */
    /*
    ** Color Map Specification
    */
    unsigned char	CmapIndex[2];        /* First Entry Index          */
    unsigned char   CmapLength[2];       /* Color Map Length           */
    unsigned char   CmapEntrySize;       /* Color Map Entry Size       */
    /*
    ** Image Specification
    */
    unsigned short   X_Origin;           /* X-origin of Image          */
    unsigned short   Y_Origin;           /* Y-origin of Image          */
    unsigned short   ImageWidth;         /* Image Width                */
    unsigned short   ImageHeight;        /* Image Height               */
    unsigned char   PixelDepth;          /* Pixel Depth                */
    unsigned char   ImagDesc;            /* Image Descriptor           */
} TGAHEADER;

// thanks to michael vance for this
void nglSaveEFBToTGA(const char* name)
{
	TGAHEADER header;

	memset(&header, 0, sizeof(header));

	s32 width = nglGetScreenWidth();
	s32 height = nglGetScreenHeight();

	header.ImageType = 2;
	header.ImageWidth = width;
	header.ImageHeight = height;
	header.PixelDepth = 32;

	header.ImageWidth = _lhbrx(&header.ImageWidth);
	header.ImageHeight = _lhbrx(&header.ImageHeight);

	// open the file for writing
	int f = GCopen(name, NGL_FIO_CREATE|NGL_FIO_WRITE);
	if (f < 0) return;
	// write the header
	GCwrite(f, &header, sizeof(header));

	// attempt to allocate from list work
	bool mem_alloc = false;
	u8* rgb = (u8*)nglListAlloc(width*4);
	mem_alloc = true;
	if (!rgb) rgb = (u8*)nglMemAlloc(width*height*4);
	if (!rgb) return;
	u8* p = rgb;

	// read the image data directly from the EFB
	for (int i = height - 1; i >= 0; --i)
	{
		for (int j = 0; j < width; ++j)
		{
			u32 color;

			GXPeekARGB( j, i, &color );

			u8* c = (u8*)&color;
			*p++ = c[3];
			*p++ = c[2];
			*p++ = c[1];
			*p++ = c[0];
		}

		GCwrite(f, rgb, (width*4));
		p = rgb;
	}

	// free the memory
	if (mem_alloc) nglMemFree(rgb);
	// close the file
	GCclose(f);
}

// Works only on 32bit and 16bit textures, and gives the contents from the previous frame, if locked
void nglSaveTexture( nglTexture* Tex, const char* FileName )
{
  	// Save the texture.
  	static u_int SaveCount = 1;

  	if (FileName)
    	sprintf( nglWork, "%s.tga", FileName );
	else
    	sprintf( nglWork, "savetex%d.tga", SaveCount++ );

	NGL_ASSERT(Tex == NULL, "Saving of textures not yet supported");

	nglSaveEFBToTGA(nglWork);

  	nglPrintf( "NGL: Saved texture %s to %s.\n", "EFB" /*Tex->FileName.c_str()*/, nglWork );
}

void nglScreenShot(const char *FileName)
{
  	static int ScreenCount = 0;
  	if (FileName)
  	{
	  	nglSaveTexture(NULL, FileName);
  	}
  	else
  	{
	  	static char Buf[64];
	  	sprintf(Buf, "ngl_gcn_screenshot%4.4d", ScreenCount++);
	  	nglSaveTexture(NULL, Buf);
  	}
}

/*---------------------------------------------------------------------------------------------------------
  @Mesh loading/management code.
---------------------------------------------------------------------------------------------------------*/
// Change all the internal pointers in a mesh structure to be based at a new location.  Useful for copying meshes around
// and for loading them from files (in which they're based at 0).
void nglRebaseMesh( u_int NewBase, u_int OldBase, nglMesh* Mesh )
{
	if ( Mesh->Flags & NGLMESH_PROCESSED )
    	return;
#define PTR_OFFSET( Ptr, Type ) { if ( Ptr ) Ptr = (Type)( (u_int)Ptr + ( (u_int)NewBase - (u_int)OldBase ) ); }

	PTR_OFFSET( Mesh->Bones, nglMatrix* );
    PTR_OFFSET( Mesh->LODs, nglMeshLODInfo* );

    PTR_OFFSET( Mesh->Sections, nglMeshSection* );

    for ( u_int i = 0; i < Mesh->NSections; i++ )
    {
      	nglMeshSection* Section = &Mesh->Sections[i];

      	PTR_OFFSET( Section->Material, nglMaterial* );
  	    PTR_OFFSET( Section->Bones, nglMatrix* );
      	PTR_OFFSET( Section->Indices, u_short* );
      	PTR_OFFSET( Section->Verts, void* );
      	PTR_OFFSET( Section->BoneIndexCounts, int* );
      	PTR_OFFSET( Section->VertMap, short* );
      	PTR_OFFSET( Section->VertPNs, void* );
	  	PTR_OFFSET( Section->VertNs, void*);
	  	PTR_OFFSET( Section->VertDs, void*);
    }
#undef PTR_OFFSET
}

// Set up hash value for the material, for sorting by texture.
void nglCalcMaterialHash( nglMaterial* Material )
{
	// FIMXE: we don't need this so we won't implement it for now
}

void nglBindMeshTextures( nglMesh* Mesh, bool Load )
{
  	for ( u_int i = 0; i < Mesh->NSections; i++ )
  	{
  	  	// Process the material data.
  	  	nglMeshSection* Section = &Mesh->Sections[i];
  	  	nglMaterial* Material = Section->Material;

  	  	// Automatically load textures used by the mesh.
  	  	if ( Material->Flags & NGLMAT_TEXTURE_MAP )
  	  	{
  	  	  	if ( Load )
  	  	  	  	Material->Map = nglLoadTexture( Material->MapName );
  	  	  	else
  	  	  	  	Material->Map = nglGetTexture( Material->MapName );
  	  	  	if ( !Material->Map ) Material->Map = &nglWhiteTex;
  	  	}

  	  	if ( Material->Flags & NGLMAT_LIGHT_MAP )
  	  	{
  	  	  	if ( Load )
  	  	  	  	Material->LightMap = nglLoadTexture( Material->LightMapName );
  	  	  	else
  	  	  	  	Material->LightMap = nglGetTexture( Material->LightMapName );
  	  	  	if ( !Material->LightMap ) Material->LightMap = &nglWhiteTex;
  	  	}

  	  	if ( Material->Flags & NGLMAT_DETAIL_MAP )
  	  	{
  	  	  	if ( Load )
  	  	  	  	Material->DetailMap = nglLoadTexture( Material->DetailMapName );
  	  	  	else
  	  	  	  	Material->DetailMap = nglGetTexture( Material->DetailMapName );
  	  	  	if ( !Material->DetailMap ) Material->DetailMap = &nglWhiteTex;
  	  	}

  	  	if ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
  	  	{
  	  	  	if ( Load )
  	  	  	  	Material->EnvironmentMap = nglLoadTexture( Material->EnvironmentMapName );
  	  	  	else
  	  	  	  	Material->EnvironmentMap = nglGetTexture( Material->EnvironmentMapName );
  	  	  	if ( !Material->EnvironmentMap ) Material->EnvironmentMap = &nglWhiteTex;
  	  	}

  	  	nglCalcMaterialHash( Material );
  	}
}

//-----------------------------------------------------------------------------

static void nglProcessMesh( nglMesh* Mesh, u_int Version )
{
	if (Mesh->Flags&NGLMESH_INACCURATE_XFORM)
	    Mesh->Flags &= ~NGLMESH_INACCURATE_XFORM;

  	if ((Mesh->Flags&NGLMESH_SKINNED) && (Mesh->Flags&NGLMESH_PERFECT_TRICLIP))
    	Mesh->Flags &= ~NGLMESH_PERFECT_TRICLIP;
    	Mesh->Flags |= NGLMESH_REJECT_TRICLIP;

	for (int i=0; i<Mesh->NSections; i++)
	{
		nglMeshSection* Section = &Mesh->Sections[i];
		nglMaterial* Material = Section->Material;

		// <<<< 12-Feb-2002 SAL: For now disable Material color on characters since lighting is applied only to VertexColors.
		if (Mesh->Flags & NGLMESH_SKINNED)
			Material->Flags &= ~NGLMAT_MATERIAL_COLOR;

		if (Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH)
      		Material->Flags |= NGLMAT_ALPHA;

		// not yet supported
		Material->Flags &= ~NGLMAT_ENV_SPECULAR;

		///////////////////////////////////////////////////////
		// convert ArchMat material flags into NGL map flags //
		///////////////////////////////////////////////////////

		// set the default map flag
		Material->MapFlags = NGLMAP_POINT_FILTER;
		// check bilinear
		if (Material->Flags & NGLMAT_BILINEAR_FILTER)
			Material->MapFlags = NGLMAP_BILINEAR_FILTER;
		// trilinear overrides if set
		if (Material->Flags & NGLMAT_TRILINEAR_FILTER)
			Material->MapFlags = NGLMAP_TRILINEAR_FILTER;
		// test the clamp modes
		if (Material->Flags & NGLMAT_CLAMP_U)
			Material->MapFlags |= NGLMAP_CLAMP_U;
		if (Material->Flags & NGLMAT_CLAMP_V)
			Material->MapFlags |= NGLMAP_CLAMP_V;
	}

	// Automatically attach textures used by the mesh.
  	nglBindMeshTextures( Mesh, true );

	Mesh->Flags |= NGLMESH_PROCESSED;

 	return;
}

//-----------------------------------------------------------------------------

nglMesh* nglGetMeshA( const char* Name, bool Warn /*= true*/ )
{
  	nglFixedString p( Name );
  	return nglGetMesh( p, Warn );
}

//-----------------------------------------------------------------------------

nglMesh* nglGetMesh( const nglFixedString& Name, bool Warn /*= true*/ )
{
  	nglInstanceBank::Instance* Inst;
  	if ( ( Inst = nglMeshBank.Search( Name ) ) )
    	return (nglMesh*)Inst->Value;
  	if ( Warn )
    	nglWarning( "NGL: Unable to find mesh %s.\n", Name.c_str() );
  	return NULL;
}

//-----------------------------------------------------------------------------

nglMesh* nglGetFirstMeshInFile( const nglFixedString& FileName )
{
  	nglInstanceBank::Instance* Inst;
  	Inst = nglMeshFileBank.Search( FileName );
  	if ( !Inst )
    	return NULL;

  	return ( (nglMeshFile*)Inst->Value )->FirstMesh;
}

//-----------------------------------------------------------------------------

nglMesh* nglGetNextMeshInFile( nglMesh* Mesh )
{
  	if ( !Mesh )
    	return NULL;

  	return Mesh->NextMesh;
}

//-----------------------------------------------------------------------------
#define NGL_CURRENT_MESH_VERSION	0x000A

bool nglLoadMeshFileInternal( const nglFixedString& FileName, nglMeshFile* MeshFile )
{
  	// Parse header.
  	nglMeshFileHeader* Header = (nglMeshFileHeader*)MeshFile->FileBuf.Buf;

	if (memcmp( &Header->Tag, "GCNM", 4 ))
	{
		nglWarning( "nglLoadMesh: Bad mesh tag in %s%s.\n", nglMeshPath, FileName );
		return false;
	}

	if (Header->Version != NGL_CURRENT_MESH_VERSION)
	{
		nglWarning( "nglLoadMesh: Bad version %d (expected %d) in %s%s.\n", Header->Version, NGL_CURRENT_MESH_VERSION, nglMeshPath, FileName );
		return false;
	}

  	if (!Header->NMeshes)
  	{
    	nglWarning( "NGL: Mesh file contains no meshes: %s%s.gcmesh.\n", nglMeshPath, FileName.c_str() );
    	return false;
  	}

  	// Read the meshes starting from the beginning of the file.
  	u_int* BufPos = (u_int*)( Header + 1 );
  	MeshFile->FirstMesh = (nglMesh*)BufPos;

  	nglMesh* Mesh;
  	nglMesh* LastMesh = NULL;
  	for ( u_int i = 0; i < Header->NMeshes; i++ )
  	{
    	Mesh = (nglMesh*)BufPos;

    	// Search for an existing mesh with this name and skip it if we've already got one.
    	nglInstanceBank::Instance* Inst;
    	if ( (Inst = nglMeshBank.Search(Mesh->Name)) )
    	{
      		nglMesh* ExistingMesh = (nglMesh*)Inst->Value;
      		nglWarning( "NGL: Skipping duplicate mesh %s found in %s%s.gcmesh.  Originally contained in %s%s.gcmesh.\n",
        		Mesh->Name.c_str(), nglMeshPath, FileName.c_str(), ExistingMesh->File->FilePath, ExistingMesh->File->FileName.c_str() );
      		BufPos += Mesh->DataSize / sizeof(u_int);
      		continue;
    	}

    	Mesh->File = MeshFile;

    	// Convert pointers from file offsets.
    	nglRebaseMesh( (u_int)Header, 0, Mesh );

    	// Load textures, process materials, etc.
    	nglProcessMesh( Mesh, Header->Version );

    	// Link it into the list of meshes for the mesh file.
    	if ( LastMesh )
      		LastMesh->NextMesh = Mesh;
    	LastMesh = Mesh;

    	nglMeshBank.Insert( Mesh->Name, Mesh );
    	BufPos += Mesh->DataSize / sizeof(u_int);
  	}
  	if ( LastMesh )
    	LastMesh->NextMesh = NULL;
  	else
    	MeshFile->FirstMesh = NULL;

  	nglPrintf( "nglLoadMeshFile: Loaded %s%s.\n", nglMeshPath, FileName.c_str() );
  	return true;
}

//-----------------------------------------------------------------------------

bool nglLoadMeshFileInPlace( const nglFixedString& FileName, void* Data )
{
  	nglInstanceBank::Instance* Inst;
  	if ( ( Inst = nglMeshFileBank.Search( FileName ) ) )
  	{
    	Inst->RefCount++;
    	return true;
  	}

  	nglMeshFile* MeshFile = (nglMeshFile*)nglMemAlloc( sizeof(nglMeshFile) );
  	strcpy( MeshFile->FilePath, "" );
  	MeshFile->FileName = FileName;
  	MeshFile->LoadedInPlace = true;
  	MeshFile->FileBuf.Buf = (u_char*)Data;
  	MeshFile->FileBuf.Size = 0;
  	MeshFile->FileBuf.UserData = 0;

  	bool Result = nglLoadMeshFileInternal( FileName, MeshFile );
  	if ( !Result )
  	{
    	nglMemFree( MeshFile );
    	return false;
  	}

  	nglMeshFileBank.Insert( FileName, MeshFile );
  	return true;
}

//-----------------------------------------------------------------------------

bool nglLoadMeshFile( const nglFixedString& FileName )
{
  	nglInstanceBank::Instance* Inst;
  	if ( ( Inst = nglMeshFileBank.Search( FileName ) ) )
  	{
    	Inst->RefCount++;
    	return true;
  	}

  	strcpy( nglWork, nglMeshPath );
  	strcat( nglWork, FileName.c_str() );
  	strcat( nglWork, ".gcmesh" );

  	nglMeshFile* MeshFile = (nglMeshFile*)nglMemAlloc( sizeof(nglMeshFile) );
  	strcpy( MeshFile->FilePath, nglMeshPath );
  	MeshFile->FileName = FileName;
  	MeshFile->LoadedInPlace = false;

  	if ( !nglReadFile( nglWork, &MeshFile->FileBuf, 128 ) )
  	{
    	nglWarning( "nglLoadMeshFile: Unable to open %s%s.\n", nglMeshPath, FileName.c_str() );
    	nglMemFree( MeshFile );
    	return false;
  	}

  	bool Result = nglLoadMeshFileInternal( FileName, MeshFile );
  	if ( !Result )
  	{
    	nglReleaseFile( &MeshFile->FileBuf );
    	nglMemFree( MeshFile );
    	return false;
  	}

  	nglMeshFileBank.Insert( FileName, MeshFile );
  	return true;
}

//-----------------------------------------------------------------------------

void nglReleaseMeshFile( const nglFixedString& FileName )
{
  	// Remove a reference from the instance bank, delete the mesh file only if the count hits 0.
  	nglInstanceBank::Instance* Inst = nglMeshFileBank.Search( FileName );
  	if ( !Inst )
    	return;
  	if ( --Inst->RefCount > 0 )
    	return;

  	nglMeshFile* MeshFile = (nglMeshFile*)Inst->Value;
  	nglMeshFileBank.Delete( MeshFile->FileName );

  	nglMesh* Mesh = MeshFile->FirstMesh;
  	while ( Mesh )
	{
  		for ( u_int i = 0; i < Mesh->NSections; i++ )
  		{
    		nglMaterial* Material = Mesh->Sections[i].Material;
    		if ( Material->Flags & NGLMAT_TEXTURE_MAP )
    			nglReleaseTexture( Material->Map );

    		if ( Material->Flags & NGLMAT_LIGHT_MAP )
    		  	nglReleaseTexture( Material->LightMap );

    		if ( Material->Flags & NGLMAT_DETAIL_MAP )
    		  	nglReleaseTexture( Material->DetailMap );

    		if ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
    		  	nglReleaseTexture( Material->EnvironmentMap );
  		}

    	nglMeshBank.Delete( Mesh->Name );
    	Mesh = Mesh->NextMesh;
  	}

  	if ( !MeshFile->LoadedInPlace )
    	nglReleaseFile( &MeshFile->FileBuf );

  	nglMemFree( MeshFile );
}

//-----------------------------------------------------------------------------

void nglReleaseAllMeshFiles()
{
  	nglInstanceBank::Instance* Inst = nglMeshFileBank.Head->Forward[0];
  	while ( Inst != nglMeshFileBank.NIL )
  	{
    	if ( ( (nglMeshFile*)Inst->Value )->FileName == nglFixedString( "nglsphere" ) || ( (nglMeshFile*)Inst->Value )->FileName == nglFixedString( "nglradius" ) )
      		Inst = Inst->Forward[0];
    	else
    	{
      		Inst->RefCount = 1;
      		nglReleaseMeshFile( ( (nglMeshFile*)Inst->Value )->FileName );
      		Inst = nglMeshFileBank.Head->Forward[0];
    	}
  	}
}

//-----------------------------------------------------------------------------

// Loads a mesh file, adding each mesh in it to the mesh bank (if it's not there already).
// Returns first mesh in the file.
nglMesh* nglLoadMesh( const char* FileName )
{
  	if ( !FileName )
    	return 0;

	// Search for an existing copy.
  	nglInstanceBank::Instance* Inst;
  	if ( ( Inst = nglMeshBank.Search( FileName ) ) )
  	{
    	Inst->RefCount++;
    	return (nglMesh*)Inst->Value;
  	}

  	strcpy( nglWork, nglMeshPath );
  	strcat( nglWork, FileName );
  	strcat( nglWork, ".gcmesh" );

  	nglFileBuf File;

  	if ( !nglReadFile( nglWork, &File, 128 ) )
  	{
    	nglPrintf( "nglLoadMesh: Unable to open %s%s.\n", nglMeshPath, FileName );
    	return nglSphereMesh;
  	}

	nglMeshFileHeader* Header = (nglMeshFileHeader*) File.Buf;

	if( memcmp( &Header->Tag, "GCNM", 4 ) )
	{
		nglReleaseFile( &File );
		nglPrintf( "nglLoadMesh: Bad mesh tag in %s%s.\n", nglMeshPath, FileName );
		return nglSphereMesh;
	}

	if ( Header->Version != NGL_CURRENT_MESH_VERSION )
	{
		nglReleaseFile( &File );
		nglPrintf( "nglLoadMesh: Bad version %d (expected %d) in %s%s.\n", Header->Version, NGL_CURRENT_MESH_VERSION, nglMeshPath, FileName );
		return nglSphereMesh;
	}

  	nglMesh* Mesh = nglProcessMeshFile( Header );

  	return Mesh;
}

int nglGetMaterialIdx( nglMesh* Mesh, u_int MaterialId )
{

  	for ( u_int i = 0; i < Mesh->NSections; i++ )
	{
		if ( Mesh->Sections[i].Material->MaterialId == MaterialId )
      		return i;
  	}

  	return -1;
}

// Add a reference to a mesh in the instance bank.
void nglAddMeshRef( nglMesh* Tex )
{
  	// Search for an existing copy.
  	nglInstanceBank::Instance* Inst;
  	if ( ( Inst = nglMeshBank.Search( Tex->Name ) ) )
    	Inst->RefCount++;
}

void nglReleaseMesh( nglMesh* Mesh )
{
  	// Remove a reference from the instance bank, delete the texture only if the count hits 0.
  	if (nglMeshBank.Delete( Mesh->Name ) != 0 )
    	return;

  	for( int i = 0; i < Mesh->NSections; i++ )
	{
    	nglMaterial* Material = Mesh->Sections[i].Material;

    	// Undo processing and Release/deref the textures.
    	if (Material->Flags & NGLMAT_TEXTURE_MAP)
      		nglReleaseTexture( Material->Map );

    	if (Material->Flags & NGLMAT_LIGHT_MAP)
      		nglReleaseTexture( Material->LightMap );

    	if (Material->Flags & NGLMAT_DETAIL_MAP)
      		nglReleaseTexture( Material->DetailMap );

    	if (Material->Flags & (NGLMAT_ENVIRONMENT_MAP | NGLMAT_ENVIRONMENT_MAP))
      		nglReleaseTexture( Material->EnvironmentMap );
  	}

  	if( Mesh->Flags & NGLMESH_BUFFER_OWNER )
	{
  		nglFileBuf File;
  		nglMeshFileHeader* Header = (nglMeshFileHeader*) ( (u_char*) Mesh - sizeof( nglMeshFileHeader ) );
  		File.Buf = (u_char*) Header;
  		File.Size = Header->Size;
  		File.UserData = 0;
    	nglReleaseFile( &File );
  	}
}

void nglReleaseAllMeshes()
{
  	nglInstanceBank::Instance* Inst = nglMeshBank.Head->Forward[0];
  	while ( Inst != nglMeshBank.NIL )
  	{
    	Inst->RefCount = 1;
    	nglReleaseMesh( (nglMesh*)Inst->Value );
    	Inst = nglMeshBank.Head->Forward[0];
  	}
}

// Returns the next node to be added to the DMA chain, seamlessly moves between lists.
NGL_INLINE nglListNode* nglGetNextNode( nglListNode* Node )
{
  	Node = Node->Next;

  	while ( Node && Node->Type == NGLNODE_BIN )
    	Node = Node->Next;

  	return Node;
}

// Test view frustum clipping.
// -1 = Rejected, 0 = Not Clipped, 1 = Guard band intersection.
NGL_INLINE int nglGetClipResult( nglVector Center, float Radius )
{
  	float Dist;

  	for ( int i = 0; i < NGLCLIP_MAX; i++ )
  	{
    	// If the entire sphere is behind any plane, reject.
    	Dist = nglInnerProduct( Center, nglCurScene->ClipPlanes[i] ) - nglCurScene->ClipPlanes[i][3];

    	if ( Dist + Radius < 0 )
      		return -1;
  	}

  	return 0;
}

inline void nglGetSortInfo( nglSortInfo* SortInfo, u_int BlendMode, nglTexture* Tex, float Dist )
{
  	if ( BlendMode == NGLBM_OPAQUE || BlendMode == NGLBM_PUNCHTHROUGH )
  	{
    	if ( Tex )
    	{
      		SortInfo->Type = NGLSORT_TEXTURED;
      		SortInfo->Hash = Tex->Hash;
    	}
    	else
      		SortInfo->Type = NGLSORT_UNTEXTURED;
  	}
  	else
  	{
    	SortInfo->Type = NGLSORT_TRANSLUCENT;
    	SortInfo->Dist = Dist;
  	}
}

/*---------------------------------------------------------------------------------------------------------
  Render list code.
---------------------------------------------------------------------------------------------------------*/
// nglTestNode can be used to narrow the scene down (via binary searching) to a single section of a single mesh.
// It's useful for tracking down weird rendering problems.  Once you've binary searched down to a single node,
// you can break in the debugger and see the node's data as it gets setup, transformed and rendered.
int nglTestNodeStart = 0;
int nglTestNodeEnd = 100000;
int nglNodeCount = 0;

// Allocates from the render list storage, which is completely cleared each frame.
void* nglListAlloc( u_int Bytes, u_int Alignment /*= 4*/ )
{
	void* rval = NULL;

  	if ( ( nglListWorkPos - nglListWork ) % Alignment )
    	nglListWorkPos = nglListWork + ( ( nglListWorkPos - nglListWork ) / Alignment + 1 ) * Alignment;

	// check for overflow
	if ((nglListWorkPos + Bytes) > (nglListWork + nglListWorkSize))
	{
    	nglPrintf( "Render list allocation overflow. Reserved=%d Requested=%d Free=%d.\n",
      		nglListWorkSize, Bytes, ((nglListWork+nglListWorkSize) - (nglListWorkPos)) );
	}
	else
	{
		rval = nglListWorkPos;
  		nglListWorkPos += Bytes;
	}

  	return rval;
}

#define nglListNew( Type ) (Type*)nglListAlloc( sizeof(Type) )

// Finds the appropriate bin and adds the node to the render list.
void nglListAddNode( u_int Type, nglCustomNodeFn NodeFn, void* Data, nglSortInfo* SortInfo )
{
  	nglListNode* NewNode = nglListNew( nglListNode );

	if( !NewNode )
	{
		nglWarning("nglListAddNode: Couldn't Allocate new Node!\n");
		return;
	}

  	NewNode->Type = Type;
  	NewNode->NodeFn = NodeFn;
  	NewNode->NodeData = Data;
  	NewNode->SortInfo = *SortInfo;

  	// Find the appropriate bin and insert the node.
  	if ( SortInfo->Type == NGLSORT_TRANSLUCENT )
  	{
    	float BinDist = 1.0f - ( minf( maxf( SortInfo->Dist, nglCurScene->NearZ ), nglCurScene->FarZ ) - nglCurScene->NearZ ) /
        	( nglCurScene->FarZ - nglCurScene->NearZ );
    	u_int BinIdx = nglFTOI( NGL_OPAQUE_BINS + ( NGL_TRANSLUCENT_BINS - 1 ) * BinDist );

    	// Place it in the correct sorted position within the list.
    	nglListNode* Node = (nglListNode*)&nglCurScene->RenderListBins[BinIdx];
    	nglListNode* NextBin = BinIdx < ( NGL_TOTAL_BINS - 1 ) ? (nglListNode*)&nglCurScene->RenderListBins[BinIdx + 1] : 0;
    	for ( ;; )
    	{
      		// Insert the node here if it's closer.
      		if ( Node->Next == NextBin || ( Node->Next->Type != NGLNODE_BIN && Node->Next->SortInfo.Dist < SortInfo->Dist ) )
      		{
        		NewNode->Next = Node->Next;
        		Node->Next = NewNode;
        		break;
      		}
      		Node = Node->Next;
    	}
  	}
  	else
  	{
    	// Untextured models go in bin 0.
    	u_int BinIdx = 0;
    	if ( SortInfo->Type == NGLSORT_TEXTURED )
      		BinIdx = 1 + SortInfo->Hash & ( NGL_OPAQUE_BINS - 2 );

    	nglListNode* Node = (nglListNode*)&nglCurScene->RenderListBins[BinIdx];
    	NewNode->Next = Node->Next;
    	Node->Next = NewNode;
  	}
}

void nglListAddCustomNode( nglCustomNodeFn CustomNodeFn, void* Data, nglSortInfo* SortInfo )
{
  	nglListAddNode( NGLNODE_CUSTOM, CustomNodeFn, Data, SortInfo );
}

int nglGCBadMesh;
void nglListAddMesh( nglMesh* Mesh, const nglMatrix LocalToWorld, nglRenderParams* Params )
{
	nglVector WorldCenter;  // World space center of the bounding sphere.
  	nglVector Center;       // View space center of the bounding sphere.
  	float Dist = 0.0f, Radius = 0.0f;
  	int Clip = 0;

  	// Check for NULL pointers, commonly encountered when file loads fail.
  	if ( !Mesh ) return;

	NGL_ASSERT(Mesh != nglScratch, "Call nglCloseMesh before adding this mesh");

	// FIXME: don't draw using FULL_MATRIX currently
	if( Params && ( Params->Flags & NGLP_FULL_MATRIX ) )
		return;


	// Add the debugging sphere.
  	if ( DEBUG_ENABLE( DrawMeshSpheres ) && Mesh != nglRadiusMesh && nglRadiusMesh )
  	{
    	nglRenderParams p;
    	if ( Params )
      		p = *Params;
    	else
      		p.Flags = 0;

    	if ( p.Flags & NGLP_SCALE )
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
  	}

#if 0 // doesn't work yet
#ifdef ARCH_ENGINE
  	if ( DEBUG_ENABLE( DumpSceneFile ) )
  	{
    	host_fprintf( nglSceneFile, "\n" );
    	host_fprintf( nglSceneFile, "MODEL %s\n", Mesh->Name.c_str() );

    	if ( Params && ( Params->Flags & NGLP_TINT ) )
    	  host_fprintf( nglSceneFile, "  TINT %f %f %f %f\n", Params->TintColor[0], Params->TintColor[1], Params->TintColor[2], Params->TintColor[3] );

    	if ( Params && ( Params->Flags & NGLP_FULL_MATRIX ) )
    	    host_fprintf( nglSceneFile, "  FULLMATRIX 1\n" );

    	host_fprintf( nglSceneFile, "  ROW1 %f %f %f %f\n", LocalToWorld[0][0], LocalToWorld[0][1], LocalToWorld[0][2], LocalToWorld[0][3] );
    	host_fprintf( nglSceneFile, "  ROW2 %f %f %f %f\n", LocalToWorld[1][0], LocalToWorld[1][1], LocalToWorld[1][2], LocalToWorld[1][3] );
    	host_fprintf( nglSceneFile, "  ROW3 %f %f %f %f\n", LocalToWorld[2][0], LocalToWorld[2][1], LocalToWorld[2][2], LocalToWorld[2][3] );
    	host_fprintf( nglSceneFile, "  ROW4 %f %f %f %f\n", LocalToWorld[3][0], LocalToWorld[3][1], LocalToWorld[3][2], LocalToWorld[3][3] );

    	if ( Params && ( Params->Flags & NGLP_BONES ) )
    	{
    	  host_fprintf( nglSceneFile, "  NBONES %d\n", Params->NBones );
    	  for ( u_int i = 0; i < Params->NBones; i++ )
    	    host_fprintf( nglSceneFile, "  BONE %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", i,
    	      Params->Bones[i][0][0], Params->Bones[i][0][1], Params->Bones[i][0][2], Params->Bones[i][0][3],
    	      Params->Bones[i][1][0], Params->Bones[i][1][1], Params->Bones[i][1][2], Params->Bones[i][1][3],
    	      Params->Bones[i][2][0], Params->Bones[i][2][1], Params->Bones[i][2][2], Params->Bones[i][2][3],
    	      Params->Bones[i][3][0], Params->Bones[i][3][1], Params->Bones[i][3][2], Params->Bones[i][3][3] );
    	}
    	host_fprintf( nglSceneFile, "ENDMODEL\n" );
  	}
#endif
#endif

  	if ( !( Params && ( Params->Flags & NGLP_FULL_MATRIX ) ) )
  	{
    	// Update the mesh radius for scale.
    	Radius = Mesh->SphereRadius;
    	if ( Params && ( Params->Flags & NGLP_SCALE ) )
    	{
    	 	float Scale = Params->Scale[0];
    	 	if ( Params->Scale[1] > Scale )
    	 	  	Scale = Params->Scale[1];
    	 	if ( Params->Scale[2] > Scale )
    	 	  	Scale = Params->Scale[2];
    	 	Radius *= Scale;
    	}

    	// Transform sphere center into camera space and find the distance to the closest point for clipping logic.
    	nglApplyMatrix( WorldCenter, LocalToWorld, Mesh->SphereCenter );
    	nglApplyMatrix( Center, nglCurScene->WorldToView, WorldCenter );

    	if ( Center[2] - Radius > nglCurScene->FarZ )
    	  	return;

		if( !( Params && ( Params->Flags & NGLP_NO_CULLING )))
		{
		  	// <<<< 07-Feb-2002 SAL: There is a bug in here. The lensflares were not surviving this clip test.
	    	Clip = nglGetClipResult( Center, Radius );
	    	if ( Clip == -1 )
		      	return;
	  	}

    	// If sphere rejection is on and the sphere touches the plane, reject the whole mesh.
    	if ( Clip && ( ( Mesh->Flags & NGLMESH_REJECT_SPHERE ) || DEBUG_ENABLE( ForceNoClip ) || !STAGE_ENABLE( Clip ) ) )
	    	return;

		// FIXME: LOD support.  Very slow implementation, but it's okay since we don't use any yet.;)
		if ( Mesh->Flags & NGLMESH_LOD )
		{
			float LODSwitchRange = 0;
			nglMesh *LODMesh = NULL;
			for ( u_int i = 0; i < Mesh->NLODs; i++ )
			{
				if ( Center[2] > Mesh->LODs[i].Range )
				{
					if( LODSwitchRange < Mesh->LODs[i].Range )
					{
						LODSwitchRange = Mesh->LODs[i].Range;
						LODMesh = nglGetMesh( Mesh->LODs[i].Name );
					}
				}
			}
			if( LODMesh )
				Mesh = LODMesh;
  		}
  	}

  	// Add a new mesh node to the mesh list.
  	nglMeshNode* MeshNode = nglListNew( nglMeshNode );

	if( !MeshNode )
	{
		nglPrintf( "nglListAddMesh: couldn't allocate nglMeshNode.\n" );
		return;
	}

  	MeshNode->Mesh = Mesh;
	nglCopyMatrix( MeshNode->LocalToWorld, LocalToWorld );
	nglInverseMatrix( MeshNode->WorldToLocal, LocalToWorld );

  	if ( Params )
  	{
    	MeshNode->Params = *Params;
    	if( Params->Flags & NGLP_BONES )
    	{
    		nglMatrix *BonesCopy = (nglMatrix *)nglListAlloc( Params->NBones * sizeof(nglMatrix), 16 );
    		if( !BonesCopy )
			{
				nglPrintf( "nglListAddMesh: couldn't allocate BonesCopy.\n" );
				return;
			}
			memcpy( BonesCopy, Params->Bones, Params->NBones * sizeof(nglMatrix) );
			MeshNode->Params.Bones = BonesCopy;
    	}
  	}
  	else
    	MeshNode->Params.Flags = 0;

  	if ( MeshNode->Params.Flags & NGLP_FULL_MATRIX )
    	nglCopyMatrix( MeshNode->LocalToScreen, LocalToWorld );
  	else
  	{
    	if ( MeshNode->Params.Flags & NGLP_SCALE )
    	{
      		nglScaleVectorXYZ( MeshNode->LocalToWorld[0], MeshNode->LocalToWorld[0], MeshNode->Params.Scale[0] );
      		nglScaleVectorXYZ( MeshNode->LocalToWorld[1], MeshNode->LocalToWorld[1], MeshNode->Params.Scale[1] );
      		nglScaleVectorXYZ( MeshNode->LocalToWorld[2], MeshNode->LocalToWorld[2], MeshNode->Params.Scale[2] );
    	}

    	// Calculate the local to screen matrix.  The normal drawer relies on standard matrices.
    	nglMulMatrix( MeshNode->LocalToScreen, nglCurScene->WorldToScreen, LocalToWorld );
  	}

  	MeshNode->Clip = Clip;

  	// Create a new node for each material.
  	Dist = Center[2] + Radius;
  	nglMeshSectionNode *PrevSectionNode = NULL, *FirstSectionNode = NULL;
  	nglSortInfo SortInfo;
  	bool TranslucentSectionPresent = false;
  	for ( u_int i = 0; i < Mesh->NSections; i++ )
  	{
    	// Skip materials that should be masked off.
    	if ( MeshNode->Params.Flags & NGLP_MATERIAL_MASK )
      		if ( ( 1 << i ) & MeshNode->Params.MaterialMask )
        		continue;

    	nglMeshSection* Section = &Mesh->Sections[i];
    	nglMaterial* Material = Section->Material;

    	nglMeshSectionNode* SectionNode = nglListNew( nglMeshSectionNode );

    	if( !SectionNode )
    	{
			nglPrintf( "nglListAddMesh: couldn't allocate nglMeshSectionNode.\n" );
			return;
		}

    	SectionNode->MeshNode = MeshNode;
    	SectionNode->Section = Section;
    	SectionNode->MaterialFlags = Material->Flags;
    	SectionNode->NextSectionNode = NULL;

    	if( Params && ( Params->Flags & NGLP_BONES ))
    	{
	    	if( PrevSectionNode )
	    	{
		    	PrevSectionNode->NextSectionNode = SectionNode;
		  	}
		  	if( !FirstSectionNode )
		  	{
		  		FirstSectionNode = SectionNode;
		  	}
		}
    	PrevSectionNode = SectionNode;

    	// If the detail map is out of range, turn it off.
    	if ( Material->DetailMap && Dist > Material->DetailMapRange )
      		SectionNode->MaterialFlags &= ~NGLMAT_DETAIL_MAP;

    	// If tinting is attempting to make the mesh translucent, make it sort.
    	if ( MeshNode->Params.Flags & NGLP_TINT )
    	{
    		if( Params->TintColor[3] != 1.0f )
	      		SectionNode->MaterialFlags |= NGLMAT_ALPHA;
		}

    	// Build the info structure and add the node.
    	if ( SectionNode->MaterialFlags & NGLMAT_ALPHA )
    	{
      		SortInfo.Type = NGLSORT_TRANSLUCENT;
      		TranslucentSectionPresent = true;

      		// Check the first material for NGLMAT_ALPHA_SORT_FIRST.
      		if ( Material->Flags & NGLMAT_ALPHA_SORT_FIRST )
      		  	SortInfo.Dist = Material->ForcedSortDistance;
      		else
      		  	SortInfo.Dist = Dist;
    	}
    	else
    	{
      		if ( SectionNode->MaterialFlags & NGL_TEXTURE_MASK )
      		{
      		  	SortInfo.Type = NGLSORT_TEXTURED;
      		  	SortInfo.Hash = 0;
      		  	if ( Material->Map )
      		  	  	SortInfo.Hash = Material->Map->Hash;
      		  	else
      		  	{
      		  	  	if ( Material->DetailMap ) SortInfo.Hash = Material->DetailMap->Hash;
      		  	  	if ( Material->EnvironmentMap ) SortInfo.Hash = Material->EnvironmentMap->Hash;
      		  	  	if ( Material->LightMap ) SortInfo.Hash = Material->LightMap->Hash;
      		  	}
      		}
      		else
      		  	SortInfo.Type = NGLSORT_UNTEXTURED;
    	}

    	if(!( Params && ( Params->Flags & NGLP_BONES )))
    	{
	    	nglListAddNode( NGLNODE_MESH_SECTION, nglRenderMeshCallback, SectionNode, &SortInfo );
	  	}
  	}
  	if( FirstSectionNode && ( Params && ( Params->Flags & NGLP_BONES )))
  	{
  		if( TranslucentSectionPresent )
  		{
  			SortInfo.Type = NGLSORT_TRANSLUCENT;
  		}

    	nglListAddNode( NGLNODE_MESH_SECTION, nglRenderMeshCallback, FirstSectionNode, &SortInfo );
  	}
}

void nglListAddLightOld( nglLightInfo* Light, nglVector Pos, nglVector Dir )
{
  	nglMatrix Work;
  	nglLightListNode* NewNode;

  	NewNode = nglListNew( nglLightListNode );

	if( !NewNode )
	{
		nglPrintf( "nglListAddLight: could not allocate nglLightListNode.\n" );
		return;
	}

  	NewNode->Next = nglLightListHead.Next;
  	NewNode->Light = *Light;
  	nglCopyVector( NewNode->WorldPos, Pos );
  	NewNode->WorldPos[3] = 1.0;
  	nglCopyVector( NewNode->WorldDir, Dir );
  	NewNode->WorldDir[3] = 0.0;
  	nglLightListHead.Next = NewNode;

  	if ( (Light->Type == NGLLIGHT_TRUEPOINT || Light->Type == NGLLIGHT_FAKEPOINT) && DEBUG_ENABLE(DrawLights))
  	{
    	nglRenderParams Params;

    	Params.Flags = NGLP_TINT | NGLP_SCALE;
    	nglCopyVector( Params.TintColor, Light->ColorMul );

    	if (nglSphereMesh)
    	{
      		nglIdentityMatrix(Work);
			nglTransMatrix(Work, Work, NewNode->WorldPos);
      		nglListAddMesh(nglSphereMesh, Work, &Params);
    	}

    	if (nglRadiusMesh && DEBUG_ENABLE(DrawLightNearRanges))
    	{
      		nglIdentityMatrix( Work );
			nglTransMatrix( Work, Work, NewNode->WorldPos );
      		Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = Light->DistFalloffStart;
      		nglListAddMesh( nglRadiusMesh, Work, &Params );
    	}

    	if (nglRadiusMesh && DEBUG_ENABLE(DrawLightFarRanges))
    	{
      		nglIdentityMatrix( Work );
			nglTransMatrix( Work, Work, NewNode->WorldPos );
      		Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = Light->DistFalloffEnd;
      		nglListAddMesh( nglRadiusMesh, Work, &Params );
    	}
  	}

#if 0
#ifdef ARCH_ENGINE
  	if ( DEBUG_ENABLE( DumpSceneFile ) )
  	{
    	host_fprintf( nglSceneFile, "\n" );
    	host_fprintf( nglSceneFile, "LIGHT\n" );

    	if ( Light->Type == NGLLIGHT_TRUEPOINT )
    	  	host_fprintf( nglSceneFile, "  TYPE POINT\n" );
    	else if ( Light->Type == NGLLIGHT_FAKEPOINT )
    	  	host_fprintf( nglSceneFile, "  TYPE FAKEPOINT\n" );
    	else if ( Light->Type == NGLLIGHT_DIRECTIONAL )
    	  	host_fprintf( nglSceneFile, "  TYPE DIRECTIONAL\n" );

    	for (u_int i=0; i<8; i++)
    	{
      		if ( Light->Flags & ( 1 << ( i + NGL_LIGHTCAT_SHIFT ) ) )
        		host_fprintf( nglSceneFile, "  LIGHTCAT %d\n", i );
    	}

    	if ( Light->Type == NGLLIGHT_TRUEPOINT || Light->Type == NGLLIGHT_FAKEPOINT )
      		host_fprintf( nglSceneFile, "  POS %f %f %f\n", Pos[0], Pos[1], Pos[2] );
    	else
      		host_fprintf( nglSceneFile, "  DIR %f %f %f\n", Dir[0], Dir[1], Dir[2] );

    	host_fprintf( nglSceneFile, "  RANGE %f %f\n", Light->DistFalloffStart, Light->DistFalloffEnd );

    	host_fprintf( nglSceneFile, "  ADDCOLOR %f %f %f %f\n", Light->ColorAdd[0], Light->ColorAdd[1], Light->ColorAdd[2], Light->ColorAdd[3] );
    	host_fprintf( nglSceneFile, "  MULCOLOR %f %f %f %f\n", Light->ColorMul[0], Light->ColorMul[1], Light->ColorMul[2], Light->ColorMul[3] );

    	host_fprintf( nglSceneFile, "ENDLIGHT\n" );
  	}
#endif
#endif
}

// Checks the sorting order of the translucent bins.
void nglCheckList()
{
  	nglListNode* Node = (nglListNode*) &nglCurScene->RenderListBins[NGL_OPAQUE_BINS];

  	while ( Node->Next )
	{

    	if( Node->Type != NGLNODE_BIN && Node->Next->Type != NGLNODE_BIN )
		{
      		if( Node->SortInfo.Dist < Node->Next->SortInfo.Dist )
        		nglPrintf( "NGL: Bad sorting order detected.\n" );

    	}

    	Node = Node->Next;
  	}
}

static GXAttr LightMapAttrCache;
static GXAttr DetailMapAttrCache;

short *VertMap;

int nglGCSkinMesh = false; // <<<< 19-Feb-2002 SAL: Using this to enable a slightly different EnvMap for the SkinnedMeshes.
nglDirProjectorLightInfo *TheProjLight; // For now we are just dealing with one projector light for Spidey Shadow.
nglMatrix ProjLightLocalToUV;

int nglGCSkipEnvMap = false; // <<<< 09-Dec-2001 SAL: Temp Fix till we advance a little more on SkinMeshes and EnvMapping.
int nglGCDoShadowMap = false; // <<<< 09-Jan-2002 SAL: Temp Fix till a better implementation is done.

#define NGLGC_BASE_STAGE_CHECK(stage) (stage==GX_TEVSTAGE0)

#define USE_NBT_FOR_ENVMAP 1 // 11-Dec-2001 SAL: Leave this option in till we
//  finalize the EnvMapping. (Set to 0 to easily test CPU computed TexCoords).

static int nglSetupVtxFmt(nglMeshNode* MeshNode, nglMesh* Mesh, nglMeshSection* Section, nglMaterial* Material, nglRenderParams* Params)
{
	int i = 0;
	int RMode = 0;
	GXAttr LightMapAttr = GX_VA_TEX7;
	GXAttr DetailMapAttr = GX_VA_TEX7; // Set to dummy for now.
	int EnvMapStage=-1;
	int InputTexCoord = 0;

	// clear the descriptions
	GXClearVtxDesc();

	// pos, color, and tex 0 are always indexed
	GXSetVtxDesc( GX_VA_POS, GX_INDEX16 );
#if USE_NBT_FOR_ENVMAP
	if(!( Material->Flags & NGLMAT_ENVIRONMENT_MAP && (!nglGCSkipEnvMap) ))
		GXSetVtxDesc( GX_VA_NRM, GX_INDEX16 );
#else
		GXSetVtxDesc( GX_VA_NRM, GX_INDEX16 );
#endif //#if USE_NBT_FOR_ENVMAP

	GXSetVtxDesc( GX_VA_CLR0, GX_INDEX16 );

	if (Material->Flags&NGLMAT_TEXTURE_MAP)
	{
		GXSetVtxAttrFmt( GX_VTXFMT0, nglTexAttrs[i], GX_TEX_ST, GX_S16, 9 );
		GXSetVtxDesc( nglTexAttrs[i], GX_INDEX16 );
		if ((Material && (Material->Flags&NGLMAT_UV_SCROLL)) || (Params->Flags&NGLP_TEXTURE_SCROLL))
		{
	    	float Scroll[2] = { 0, 0 };
	    	if (Params->Flags&NGLP_TEXTURE_SCROLL)
	    	{
	      		Scroll[0] += Params->ScrollU;
	      		Scroll[1] += Params->ScrollV;
	    	}
	    	if (Material->Flags&NGLMAT_UV_SCROLL)
	    	{
		      Scroll[0] += nglCurScene->AnimTime * Material->ScrollU;
		      Scroll[1] += nglCurScene->AnimTime * Material->ScrollV;
	    	}
	    	Scroll[0] -= (int(Scroll[0]));
	    	Scroll[1] -= (int(Scroll[1]));

			Mtx ScrollMat;
			MTXIdentity(ScrollMat);
			ScrollMat[0][3] = Scroll[0];
			ScrollMat[1][3] = Scroll[1];
			GXLoadTexMtxImm(ScrollMat, nglTexMtx[NGLTEXMTX_SCROLLEDMAP], GX_MTX2x4);
			GXSetTexCoordGen(nglTexCoordDst[i], GX_TG_MTX2x4, nglTexCoordSrc[i], nglTexMtx[NGLTEXMTX_SCROLLEDMAP]);
		}
		else
		{
			GXSetTexCoordGen(nglTexCoordDst[i], GX_TG_MTX2x4, nglTexCoordSrc[i], GX_IDENTITY);
		}
		++i;
		InputTexCoord++;
		RMode |= NGL_RMODE_TEXTURE;
	}

	// detail map tex coords are always provided in immediate mode
	if (Material->Flags & NGLMAT_DETAIL_MAP)
	{
		NGL_ASSERT( Material->Flags & NGLMAT_TEXTURE_MAP, "" ); // Not supporting detail map without base texture.
		DetailMapAttr = nglTexAttrs[i];
		nglMatrix ScaleMat;
    	nglIdentityMatrix( ScaleMat );
		GXSetVtxAttrFmt( GX_VTXFMT0, nglTexAttrs[i], GX_TEX_ST, GX_S16, 9 );
		GXSetVtxDesc( nglTexAttrs[i], GX_INDEX16 );
    	ScaleMat[0][0] = Material->DetailMapUScale;
    	ScaleMat[1][1] = Material->DetailMapVScale;
    	GXLoadTexMtxImm( (float (*)[4])&ScaleMat, nglTexMtx[NGLTEXMTX_DETAIL_SCALE], GX_MTX2x4 );
		GXSetTexCoordGen( nglTexCoordDst[i], GX_TG_MTX2x4, nglTexCoordSrc[InputTexCoord], nglTexMtx[NGLTEXMTX_DETAIL_SCALE] );
		++i;
		InputTexCoord++; // <<<< 17-Dec-2001 SAL: For the current implementation we should not be resending this data. Remove later.
		RMode |= NGL_RMODE_DETAIL;
	}

	// same with environment maps
	if( Material->Flags & (NGLMAT_ENVIRONMENT_MAP | NGLMAT_ENV_SPECULAR) )
	{
		if( nglGCSkinMesh || ( Material->Flags & NGLMAT_ENV_SPECULAR) )
		{
			GXSetTexCoordGen2( nglTexCoordDst[i], GX_TG_MTX3x4, GX_TG_NRM,
				nglTexMtx[NGLTEXMTX_ENVMAP], GX_ENABLE, nglTexPtMtx[NGLTEXMTX_PM_ENVMAP] ); // NGLTEXMTX_ENVMAP NGLTEXMTX_PM_IDENTITY
			nglMatrix EnvMat;
	    	nglIdentityMatrix( EnvMat );

	    	EnvMat[0][0] = 0.5f;
	    	EnvMat[0][3] = 0.5f;
	    	EnvMat[1][1] = -0.5f; // 24-Jan-2002 SAL: Flip the y to match our textures.
	    	EnvMat[1][3] = 0.5f;
	    	EnvMat[2][2] = 0;
	    	EnvMat[2][3] = 1;
	    	GXLoadTexMtxImm( (float (*)[4])&EnvMat, nglTexPtMtx[NGLTEXMTX_PM_ENVMAP], GX_MTX3x4 );
			EnvMapStage=i;
			++i;
			if(!( Material->Flags & NGLMAT_ENV_SPECULAR))
				RMode |= NGL_RMODE_ENVIRONMENT;
		}
		else if(!nglGCSkipEnvMap)
		{
#if USE_NBT_FOR_ENVMAP
			GXSetTexCoordGen2( nglTexCoordDst[i], GX_TG_MTX3x4, GX_TG_BINRM,
				GX_IDENTITY, GX_ENABLE, nglTexPtMtx[NGLTEXMTX_PM_ENVMAP] ); // NGLTEXMTX_ENVMAP NGLTEXMTX_PM_IDENTITY
	    	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, GX_F32, 0);
			nglMatrix EnvMat;
	    	nglIdentityMatrix( EnvMat );

	    	EnvMat[0][0] = 0.5f;
	    	EnvMat[0][3] = 0.5f;
	    	EnvMat[1][1] = -0.5f; // 24-Jan-2002 SAL: Flip the y to match our textures.
	    	EnvMat[1][3] = 0.5f;
	    	EnvMat[2][2] = 0;
	    	EnvMat[2][3] = 1;
	    	GXLoadTexMtxImm( (float (*)[4])&EnvMat, nglTexPtMtx[NGLTEXMTX_PM_ENVMAP], GX_MTX3x4 );
			GXSetVtxDesc( GX_VA_NBT, GX_DIRECT );
			EnvMapStage=i;
#else //#if USE_NBT_FOR_ENVMAP
			GXSetTexCoordGen( nglTexCoordDst[i], GX_TG_MTX2x4, nglTexCoordSrc[InputTexCoord], GX_IDENTITY);
			GXSetVtxAttrFmt( GX_VTXFMT0, nglTexAttrs[InputTexCoord], GX_TEX_ST, GX_F32, 0 );
			GXSetVtxDesc( nglTexAttrs[InputTexCoord], GX_DIRECT );
			InputTexCoord++;
#endif //else //#if USE_NBT_FOR_ENVMAP
			++i;
			RMode |= NGL_RMODE_ENVIRONMENT;
		}
	}

	// but sometimes there is an additional tex-coord
	if (Material->Flags & NGLMAT_LIGHT_MAP)
	{
		GXSetVtxAttrFmt( GX_VTXFMT0, nglTexAttrs[InputTexCoord], GX_TEX_ST, GX_S16, 9 );
		GXSetVtxDesc( nglTexAttrs[InputTexCoord], GX_INDEX16 );
		LightMapAttr = nglTexAttrs[InputTexCoord];
		GXSetTexCoordGen( nglTexCoordDst[i], GX_TG_MTX2x4, nglTexCoordSrc[InputTexCoord], GX_IDENTITY);
		InputTexCoord++;
		++i;
		RMode |= NGL_RMODE_LIGHT;
	}

	if (nglGCDoShadowMap)
	{
    	nglMulMatrix(ProjLightLocalToUV, MeshNode->LocalToWorld, TheProjLight->WorldToUV);
    	Mtx ShadowMapMtx;
		nglToMtx(ShadowMapMtx, ProjLightLocalToUV);
    	GXLoadTexMtxImm(ShadowMapMtx, nglTexMtx[NGLTEXMTX_SHADMAP], GX_MTX2x4);
		GXSetTexCoordGen(nglTexCoordDst[i], GX_TG_MTX2x4, GX_TG_POS, nglTexMtx[NGLTEXMTX_SHADMAP]);
		++i;
	}

	// our # of texcoords is equal to the number of stages
	GXSetNumTexGens( i );

	// setup vertex arrays
	if (Mesh->Flags & NGLMESH_SCRATCH)
	{
		nglVertex* Verts = (nglVertex*) Section->Verts;
		nglVertEnvMapTemp = (int*) Verts;
		nglVertEnvMapSize = sizeof( nglVertex ) >> 2;

		LightMapAttrCache = LightMapAttr;
		DetailMapAttrCache = DetailMapAttr;
	}
	else if (Mesh->Flags & NGLMESH_SKINNED)
	{
		nglSkinnedVertex* Verts = (nglSkinnedVertex*) Section->Verts;
		nglSkinVertPN* VertPNs = (nglSkinVertPN*) Section->VertPNs;
		VertMap = Section->VertMap;
		// <<<< 04-Dec-2001 SAL: EnvMap support has to be reimplemented for skinned characters.
		nglVertEnvMapTemp = (int*) VertPNs;
		nglVertEnvMapSize = sizeof( nglSkinVertPN ) >> 2;

		// set our vertex descriptor
//		GXSetVtxDesc( GX_VA_CLR1, GX_INDEX16 );

		GXSetArray( GX_VA_POS, nglSkinWork, sizeof( nglVector ) * 2 );
		GXSetArray( GX_VA_NRM, &nglSkinWork[1], sizeof( nglVector ) * 2 );
		GXSetArray( GX_VA_CLR0, &Verts->color, sizeof( nglSkinnedVertex ) );
		GXSetArray( GX_VA_TEX0, &Verts->u1, sizeof( nglSkinnedVertex ) );
		GXSetArray( LightMapAttr, &Verts->u2, sizeof( nglSkinnedVertex ) );
		GXSetArray( DetailMapAttr, &Verts->u1, sizeof( nglSkinnedVertex ) );
	}
	else
	{
		nglVertex2* VertDs = (nglVertex2*) Section->VertDs;
		nglVertP* VertPs = (nglVertP*) Section->VertPNs;
		nglVertN* VertNs = (nglVertN*) Section->VertNs;
		VertMap = NULL;
		nglEnvMapVertP = VertPs;
		nglEnvMapVertN = VertNs;

		// set our vertex descriptor
		GXSetVtxDesc( GX_VA_CLR1, GX_INDEX16 );

		GXSetArray( GX_VA_POS, &VertPs->x, sizeof( nglVertP ) );
		GXSetArray( GX_VA_NRM, &VertNs->nx, sizeof( nglVertN ) );
		GXSetArray( GX_VA_CLR0, &VertDs->color, sizeof( nglVertex2 ) );
		GXSetArray( GX_VA_CLR1, ((u8*)&VertDs->color)+2, sizeof( nglVertex2 ) );
		GXSetArray( GX_VA_TEX0, &VertDs->u1, sizeof( nglVertex2 ) );
		GXSetArray( LightMapAttr, &VertDs->u2, sizeof( nglVertex2 ) );
		GXSetArray( DetailMapAttr, &VertDs->u1, sizeof( nglVertex2 ) );
	}

	return RMode;
}

// FIXME: these are hacks right now, we should be using the lighing stages
bool nglUseTint = false;
nglVector nglTintColor;

GXTevColorArg nglGCTempSrcClr = GX_CC_TEXC;
GXTevAlphaArg nglGCTempSrcAlpha = GX_CA_TEXA;
GXTevColorArg nglGCTempSrcVCC = GX_CC_RASC;
GXTevAlphaArg nglGCTempSrcVCA = GX_CA_RASA;

// this is defined as OPAQUE by NGL
static void nglBlendModulate( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map )
{
	GXTevColorArg ColorIn;
	GXTevAlphaArg AlphaIn;

	if (NGLGC_BASE_STAGE_CHECK(stage))
	{
		ColorIn = GX_CC_RASC;
		AlphaIn = GX_CA_RASA;
	}
	else
	{
		ColorIn = GX_CC_CPREV;
		AlphaIn = GX_CA_APREV;
	}

	// activate this tev stage
	GXSetTevOrder( stage, coord, map, GX_COLOR0A0 );
	// cprev = (ZERO (ADD) ((1.0 - ColorIn)*ZERO + ColorIn*TexC) + TB_ZERO) * SCALE_1;
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, ColorIn, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	// aprev = (AlphaIn (ADD) ((1.0 - ZERO)*ZERO + ZERO*ZERO + TB_ZERO) * SCALE_1;
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, AlphaIn);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

//A special ks specific blend modulate mode. Scales the final output by 2, for
//improved lighting
static void nglBlendModulateKSpecial( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map )
{
	GXTevColorArg ColorIn;
	GXTevAlphaArg AlphaIn;

	if (NGLGC_BASE_STAGE_CHECK(stage))
	{
		ColorIn = GX_CC_RASC;
		AlphaIn = GX_CA_RASA;
	}
	else
	{
		ColorIn = GX_CC_CPREV;
		AlphaIn = GX_CA_APREV;
	}

	// activate this tev stage
	GXSetTevOrder( stage, coord, map, GX_COLOR0A0 );
	// cprev = (ZERO (ADD) ((1.0 - ColorIn)*ZERO + ColorIn*TexC)) * SCALE_2;
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, ColorIn, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);

	// aprev = (AlphaIn (ADD) ((1.0 - ZERO)*ZERO + ZERO*ZERO + TB_ZERO) * SCALE_1;
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, AlphaIn);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

static void nglBlendDecal(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, float constant = 1.0f)
{
	GXTevColorArg CArg,CArg2;
	GXTevAlphaArg AArg;
	int AlternateAlphaInputs = false;

	if (!NGLGC_BASE_STAGE_CHECK(stage))
	{
		CArg = GX_CC_CPREV;
		AArg = GX_CA_APREV;
		if (constant == 1.0f)
		{
			CArg2 = GX_CC_TEXA;
		}
		else
		{
			CArg2 = GX_CC_KONST;
			u_int RangeAdjustedConstant = (u_int)(255.0f * constant);
			GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, 0xFF };
			GXSetTevKColor(GX_KCOLOR0, color);
			GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
			GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
		}
	}
	else
	{
		CArg = GX_CC_ZERO;
		CArg2 = GX_CC_ONE;
		if (constant == 1.0f)
		{
			AArg = nglGCTempSrcAlpha;
		}
		else
		{
			AArg = GX_CA_KONST;
		}
		u_int RangeAdjustedConstant = (u_int)(255.0f * constant);
		GXColor color = { 0xFF, 0xFF, 0xFF, RangeAdjustedConstant };

		if (nglUseTint)
		{
			AArg = GX_CA_KONST;
			CArg2 = GX_CC_KONST;
		 	color.r = (u8)(nglTintColor[0] * 255.0f);
		 	color.g = (u8)(nglTintColor[1] * 255.0f);
		 	color.b = (u8)(nglTintColor[2] * 255.0f);
 			if (constant != 1.0f)
 			{
			 	color.a = (u8)(nglTintColor[3] * 255.0f);
			}
			else
			 	AlternateAlphaInputs = true;
		}

		GXSetTevKColor( GX_KCOLOR0, color );
		GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
		GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	}

	GXSetTevOrder(stage, coord, map, GX_COLOR_NULL);
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn(stage, CArg, nglGCTempSrcClr, CArg2, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	if (AlternateAlphaInputs)
		GXSetTevAlphaIn(stage, GX_CA_ZERO, AArg, nglGCTempSrcAlpha, GX_CA_ZERO);
	else
		GXSetTevAlphaIn(stage, AArg, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// INDENT
static void nglBlendDecalAddAlpha( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	// 21-Feb-2002 SAL: Special case used for the LightMap stage of tranlucent glasses.
	GXSetTevOrder( stage, coord, map, GX_COLOR_NULL );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn( stage,
									 GX_CC_CPREV,
									 nglGCTempSrcClr,
									 GX_CC_TEXA,
									 GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaIn( stage,
									 GX_CA_APREV,
									 GX_CA_ZERO,
									 GX_CA_ZERO,
									 nglGCTempSrcAlpha );
}

static void nglBlendAdditive(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, float constant = 1.0f)
{
	GXTevColorArg ColorIn;
	GXTevAlphaArg AlphaIn;

	NGL_ASSERT(constant == 1.0f, "constant alpha blend not yet supported");

	if (NGLGC_BASE_STAGE_CHECK(stage))
	{
		ColorIn = GX_CC_RASC;
		AlphaIn = GX_CA_RASA;
	}
	else
	{
		ColorIn = GX_CC_CPREV;
		AlphaIn = GX_CA_APREV;
	}

	// activate this tev stage
	GXSetTevOrder(stage, coord, map, GX_COLOR0A0);
	// cprev = (D_ZERO (ADD) ((1.0 - C_ColorIn)*A_ZERO + C_ColorIn*B_TexC) + TB_ZERO) * SCALE_1;
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, ColorIn, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	// aprev = (D_ZERO (ADD) ((1.0 - C_TEXA)*A_ZERO + C_TEXA*B_AlphaIn + TB_ZERO) * SCALE_1;
	GXSetTevAlphaIn(stage, GX_CA_ZERO, AlphaIn, GX_CA_TEXA, GX_CA_ZERO);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	// dst_pix_clr = src_pix_clr * src_alpha + dst_pix_clr * 1.0
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
}




static void nglBlendSubtract( GXTevStageID stage,
															GXTexCoordID coord,
															GXTexMapID map,
															float constant = 1.0f )
{
	GXTevColorArg CArg2=GX_CC_KONST;
	GXTevAlphaArg AArg,AArg2=GX_CA_KONST;

	GXColor color = { 0xFF, 0xFF, 0xFF, 0xFF };
	if( constant != 1.0f )
	{
		color.r = color.g = color.b = color.a = (u8)(constant * 255.0f);
	}

	GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ZERO, GX_BL_ONE, GX_LO_NOOP);
	AArg = GX_CA_KONST;

	GXSetTevKColor( GX_KCOLOR0, color );
	GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
	GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );


	GXSetTevOrder( stage, coord, map, GX_COLOR0A0 );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = 0 * ( 1 - Cf ) + Ct * Cf + 0
	//    = Ct * Cf
	GXSetTevColorIn( stage,
									 GX_CC_ZERO,
									 nglGCTempSrcClr,
									 CArg2,
									 GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// A' = 0 * ( 1 - Af ) + At * Af + 0
	//    = At * Af
	GXSetTevAlphaIn( stage,
									 GX_CA_ZERO,
									 AArg,
									 AArg2,
									 GX_CA_ZERO );
}

static void nglBlendReplace( GXTevStageID stage,
														 GXTexCoordID coord,
														 GXTexMapID map )
{
	GXTevColorArg CArg = GX_CC_ONE;
	GXTevAlphaArg AArg = GX_CA_KONST;

	if( NGLGC_BASE_STAGE_CHECK(stage) )
	{
	}
	else
	{
		nglBlendDecal(stage,coord,map); // Just treat these as a simple blend mode.
		return;
	}

	GXColor color = { 0xFF, 0xFF, 0xFF, 0xFF };
	GXSetTevKColor( GX_KCOLOR0, color );
	GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
	GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );

	GXSetTevOrder( stage, coord, map, GX_COLOR0A0 );
	// ooh, actual sub op
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = 0 * ( 1 - 0 ) + 0 * 0 + Ct
	//    = Ct
	GXSetTevColorIn( stage,
									 GX_CC_ZERO,
									 nglGCTempSrcClr,
									 CArg,
									 GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + At
	//    = At
	GXSetTevAlphaIn( stage,
									 GX_CA_ZERO,
									 nglGCTempSrcAlpha,
									 AArg,
									 GX_CA_ZERO );

}

static void nglBlendVertexColor ( GXTevStageID stage )
{
	GXTevColorArg CArg,CArg2=nglGCTempSrcVCC;
	GXTevAlphaArg AArg,AArg2=nglGCTempSrcVCA;
	if( !NGLGC_BASE_STAGE_CHECK(stage) ) {
		CArg = GX_CC_CPREV;
		AArg = GX_CA_APREV;
	}
	else
	{
		CArg = GX_CC_ONE;
		AArg = GX_CA_KONST;
		GXColor color = { 0xFF, 0xFF, 0xFF, 0xFF };
		GXSetTevKColor( GX_KCOLOR0, color );
		GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
		GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
	}

	GXSetTevOrder( stage, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = Ct * ( 1 - 0 ) + 0 * 0 + Cf
	//    = Ct + Cf
	GXSetTevColorIn( stage,
									GX_CC_ZERO,
									CArg,
									CArg2,
									GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaIn( stage,
									 GX_CA_ZERO,
									 AArg,
									 AArg2,
									 GX_CA_ZERO);
}

static void	nglBlendSignedAdd( GXTevStageID stage,
															GXTexCoordID coord,
															GXTexMapID map )
{
	GXTevColorArg CArg;
	GXTevAlphaArg AArg;

	CArg = GX_CC_CPREV;
	AArg = GX_CA_APREV;

	GXSetTevOrder( stage, coord, map, GX_COLOR_NULL );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_SUBHALF,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = Ct * ( 1 - 0 ) + 0 * 0 + Cf
	//    = Ct + Cf
	GXSetTevColorIn( stage,
									CArg,
									GX_CC_ZERO,
									GX_CC_ZERO,
									nglGCTempSrcClr );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaIn( stage,
									 AArg,
									 GX_CA_ZERO,
									 GX_CA_ZERO,
									 GX_CA_ZERO);
}

static void nglBlendProjLightStage1(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, float constant = 1.0f)
{
	GXSetTevOrder(stage, coord, map, GX_COLOR1A1);
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn(stage, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
}

static void nglBlendProjLightStage2( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	GXSetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_C0, GX_CC_A0, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaIn(stage, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

//-----------------------------------------------------------------------------

static void nglBlendClearAlpha( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map )
{
  SceneGXSetZMode(/*test*/ false, /*write*/ false);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
	GXSetTevOrder( stage, coord, map, GX_COLOR_NULL );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVREG1 );
	GXSetTevColorIn( stage,
									 GX_CC_ZERO,
									 GX_CC_ZERO,
									 GX_CC_ZERO,
									 GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVREG1 );
	GXSetTevAlphaIn( stage,
									 GX_CA_TEXA, // This should be the white Tex with Alpha = 1;
									 GX_CA_ZERO,
									 GX_CA_ZERO,
									 GX_CA_ZERO );

}

//-----------------------------------------------------------------------------
//
// @Blend -- new blending functionality, to be exposed for custom nodes
//
// GCN blend equation is:
//   out_reg = (d (op) ((1.0 - c)*a + c*b) + bias) * scale;
//
//-----------------------------------------------------------------------------

static void nglSetTevBlend(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map)
{
	GXTevColorArg TexColor;
	GXTevAlphaArg TexAlpha;

	if (stage == GX_TEVSTAGE0)
	{
		TexColor = GX_CC_TEXC;
		TexAlpha = GX_CA_TEXA;
	}
	else
	{
		NGL_ASSERT(false, "nglSetTevBlend unimplemented for stages greater than 0");
	}

	// load up the vertex color into RAS
	GXSetTevOrder(stage, coord, map, GX_COLOR0A0);
	// dst_pix_clr = src_pix_clr * src_alpha + dst_pix_clr * (1.0 - src_alpha)
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	// dst_clr = (0 + ((1.0 - TexColor)*0 + RSAC*TexColor) + 0) * 1.0;
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_RASC, TexColor, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	// dst_alpha = (0 + ((1.0 - TexAlpha)*0 + RASA*TexAlpha) + 0) * 1;
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_RASA, TexAlpha, GX_CA_ZERO);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
}


//-----------------------------------------------------------------------------

static void nglSetupBlendModeOLD(u_int mode, float constant, GXTevStageID stage, GXTexCoordID coord, GXTexMapID map)
{
	switch (mode)
	{
	case NGLBM_OPAQUE:
		// FIXME: will be diff't if using material color
		nglBlendModulate( stage, coord, map );
		break;
		case NGLBM_KSOPAQUE:
			nglBlendModulateKSpecial( stage, coord, map );
			break;
  	case NGLBM_PUNCHTHROUGH:
  		nglBlendReplace( stage, coord, map );
		break;
  	case NGLBM_MOTION_BLUR:
  		// Blend the same as NGLBM_BLEND.
  		NGL_ASSERT(NGLGC_BASE_STAGE_CHECK(stage), "");
  	case NGLBM_BLEND:
  		// actually a decal mode in Spiderman
		nglBlendDecal( stage, coord, map );
  		break;
  	case NGLBM_ADDITIVE:
		nglBlendAdditive( stage, coord, map );
  	break;
  	case NGLBM_SUBTRACTIVE:
		NGL_ASSERT(NGLGC_BASE_STAGE_CHECK(stage), ""); // Expecting this to be the base stage.
  		nglBlendSubtract( stage, coord, map );
		break;
  	case NGLBM_CONST_BLEND:
  		// really a blend mode
		nglBlendDecal( stage, coord, map, constant );
  		break;
  	case NGLBM_CONST_ADDITIVE:
		// FIXME
		nglBlendAdditive( stage, coord, map, constant );
		break;
  	case NGLBM_CONST_SUBTRACTIVE:
		NGL_ASSERT(NGLGC_BASE_STAGE_CHECK(stage), ""); // Expecting this to be the base stage.
		nglBlendSubtract( stage, coord, map, constant );
		break;
  	case NGLBM_ADDSIGNED:
		NGL_ASSERT(!NGLGC_BASE_STAGE_CHECK(stage), ""); // This is currently setup for detail maps following a base map.
		nglBlendSignedAdd( stage, coord, map );
		break;
  	case NGLBM_PROJ_SHADOW_S1:
		NGL_ASSERT(!NGLGC_BASE_STAGE_CHECK(stage), "");
		nglBlendProjLightStage1( stage, coord, map );
		break;
  	case NGLBM_PROJ_SHADOW_S2:
		NGL_ASSERT(!NGLGC_BASE_STAGE_CHECK(stage), "");
		nglBlendProjLightStage2( stage, coord, map );
		break;
	case NGLBM_GC_SPECIAL1:
		NGL_ASSERT(NGLGC_BASE_STAGE_CHECK(stage), ""); // Expecting this to be the base stage. // Clear Alpha.
		nglBlendClearAlpha( stage, coord, map );
		break;
  	case NGLBM_GC_SPECIAL2:
		NGL_ASSERT(!NGLGC_BASE_STAGE_CHECK(stage), ""); // This is currently setup to do a special Blend_AddAlpha for translucent glass's LightMap.
		nglBlendDecalAddAlpha( stage, coord, map );
		break;
	default:
		NGL_ASSERT(FALSE, "");
		break;
	}
}

static void nglSetupTevStage(u_int mode, float constant, GXTevStageID stage, GXTexCoordID coord, GXTexMapID map)
{
	switch (mode)
	{
  	case NGLBM_BLEND:
		nglSetTevBlend(stage, coord, map);
  		break;
	default:
		nglSetupBlendModeOLD(mode, constant, stage, coord, map);
	}
}

static void nglSetClampMode( u_int Flags, nglTexture* Texture )
{
	GXTexWrapMode WrapS = GX_REPEAT;
	GXTexWrapMode WrapT = GX_REPEAT;

	if (Flags & NGLMAP_CLAMP_U || (Texture->Flags & NGLGC_TEXFLAG_UCLAMP))
		WrapS = GX_CLAMP;

	if (Flags & NGLMAP_CLAMP_V || (Texture->Flags & NGLGC_TEXFLAG_VCLAMP))
		WrapT = GX_CLAMP;

	GXInitTexObjWrapMode( &Texture->TexObj, WrapS, WrapT );
}

static void nglSetFilterMode(u_int Flags, nglTexture* Texture)
{
	GXTexFilter mag = GX_NEAR;
	GXTexFilter min = GX_NEAR;
	GXAnisotropy aniso = GX_ANISO_1;
	float bias = 0.0f;

	if (!Texture->Mipmaps)
	{
		switch (Flags & NGLMAP_FILTER_BITS)
		{
		case NGLMAP_POINT_FILTER:
			mag = GX_NEAR;
			min = GX_NEAR;
			break;
		case NGLMAP_BILINEAR_FILTER:
		case NGLMAP_TRILINEAR_FILTER:
		case NGLMAP_ANISOTROPIC_FILTER:
			mag = GX_LINEAR;
			min = GX_LINEAR;
			break;
		}
	}
	else
	{
		switch (Flags & NGLMAP_FILTER_BITS)
		{
		case NGLMAP_POINT_FILTER:
			mag = GX_NEAR;
			min = GX_NEAR;
			break;
		case NGLMAP_BILINEAR_FILTER:
			mag = GX_LINEAR;
			min = GX_LIN_MIP_NEAR;
			break;
		case NGLMAP_ANISOTROPIC_FILTER:
			aniso = GX_ANISO_4;
		case NGLMAP_TRILINEAR_FILTER:
			mag = GX_LINEAR;
			min = GX_LIN_MIP_LIN;
			break;
		}
	}

	GXInitTexObjLOD(&Texture->TexObj, min, mag, 0.0f, (f32)Texture->Mipmaps,
		bias, GX_DISABLE, GX_ENABLE, aniso);
}

//-----------------------------------------------------------------------------


#define NGL_GC_TEST_TRANSLUCENTS 0
#if NGL_GC_TEST_TRANSLUCENTS
int TempTestTranlucentObjects=true; // Translucent Test.
#endif //#if NGL_GC_TEST_TRANSLUCENTS
int nglGCUseWhiteShadow = false;
static NGL_INLINE int nglSetupTevs( nglMaterial* Material, nglRenderParams* Params )
{
	// index into tev triplet list, # of stages
	int i = 0;
	int CoordInd = 0;
	int MapInd = 0;
	u_int Flags = Material->Flags;
	u_int MapFlags = Material->MapFlags;
	int FirstMode = -1;
	int VCInFirstTEVStage=false; // VertCol Blend in first TEV stage if all we have to do is VertCol and Base Tex.

	// default to no tint
	nglUseTint = false;

	if( nglGCSkipEnvMap && !nglGCSkinMesh )
	{
		Flags &= ~NGLMAT_ENVIRONMENT_MAP;
	}

	if (STAGE_ENABLE(LightAmb) && (Params->Flags&NGLP_TINT))
	{
		nglUseTint = true;
		nglTintColor[0] = Params->TintColor[0];
		nglTintColor[1] = Params->TintColor[1];
		nglTintColor[2] = Params->TintColor[2];
		nglTintColor[3] = Params->TintColor[3];
	}

	// default blend mode
	GXSetBlendMode( GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP );

	if ( Flags & NGLMAT_TEXTURE_MAP )
	{
		nglTexture* Map = Material->Map;
#if NGL_GC_TEST_TRANSLUCENTS
		if( TempTestTranlucentObjects && (nglVBlankCount & 0x0020) && Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH )
			Map = &nglWhiteTex;
#endif //#if NGL_GC_TEST_TRANSLUCENTS
		if( Params->Flags & NGLP_TEXTURE_FRAME )
		{
			if( Map->Frames && Map->NFrames )
			{
				Map = Map->Frames[ Params->TextureFrame % Map->NFrames ];
			}
		}
		else if( Map->NFrames ) {
			if( Map->Frames )
				Map = Map->Frames[ nglCurScene->IFLFrame % Map->NFrames ];
		}

		nglSetClampMode( MapFlags, Map );
		nglSetFilterMode( MapFlags, Map );
		if( Map->PaletteSize ) {
			GXLoadTlut( &Map->TlutObj, MapInd );
			GXInitTexObjTlut( &Map->TexObj, MapInd );
		}

		GXLoadTexObj( &Map->TexObj, nglTevList[MapInd].map );

		if( ( Material->Flags & NGLMAT_LIGHT ) && Material->MapBlendMode == NGLBM_OPAQUE )
		{
			nglSetupTevStage( NGLBM_KSOPAQUE,
										Material->MapBlendModeConstant,
										nglTevList[i].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
		}
		else
		{
			nglSetupTevStage( Material->MapBlendMode,
										Material->MapBlendModeConstant,
										nglTevList[i].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
		}
		GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_FALSE, GX_FALSE );

		if( FirstMode == -1 ) {
			FirstMode = Material->MapBlendMode;
		}

		++i;
		CoordInd++;
		MapInd++;
	}

	if (Flags&NGLMAT_DETAIL_MAP)
	{
		nglTexture* DetailMap = Material->DetailMap;

		nglSetClampMode(MapFlags, DetailMap);
		nglSetFilterMode(NGLMAP_BILINEAR_FILTER, DetailMap);

		NGL_ASSERT(!DetailMap->NFrames, "");

		if (DetailMap->PaletteSize)
		{
			GXLoadTlut(&DetailMap->TlutObj, MapInd);
			GXInitTexObjTlut(&DetailMap->TexObj, MapInd);
		}

		GXLoadTexObj(&DetailMap->TexObj, nglTevList[MapInd].map);

		{
			nglSetupTevStage( NGLBM_ADDSIGNED,
										Material->DetailMapBlendModeConstant,
										nglTevList[i].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
		}

		GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_FALSE, GX_FALSE );

		if( FirstMode == -1 ) {
			FirstMode = Material->DetailMapBlendMode;
		}

		++i;
		CoordInd++;
		MapInd++;
	}

	if( Flags & (NGLMAT_ENVIRONMENT_MAP | NGLMAT_ENV_SPECULAR) ) {
		nglTexture* EnvironmentMap = Material->EnvironmentMap;

		nglSetClampMode( MapFlags, EnvironmentMap );
		nglSetFilterMode( MapFlags, EnvironmentMap );

		NGL_ASSERT( !EnvironmentMap->NFrames, "" );

		if( EnvironmentMap->PaletteSize ) {
			GXLoadTlut( &EnvironmentMap->TlutObj, MapInd );
			GXInitTexObjTlut( &EnvironmentMap->TexObj, MapInd );
		}

		GXLoadTexObj( &EnvironmentMap->TexObj, nglTevList[MapInd].map );

		{
			nglSetupTevStage( Material->EnvironmentMapBlendMode,
										Material->EnvironmentMapBlendModeConstant,
										nglTevList[i].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
		}
		GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_FALSE, GX_FALSE );

		if( FirstMode == -1 ) {
			FirstMode = Material->EnvironmentMapBlendMode;
		}

		++i;
		CoordInd++;
		MapInd++;
	}

	if( Flags & NGLMAT_LIGHT_MAP ) {
		nglTexture* LightMap = Material->LightMap;

		nglSetClampMode(MapFlags, LightMap);
		nglSetFilterMode(NGLMAP_BILINEAR_FILTER, LightMap);

		NGL_ASSERT( !LightMap->NFrames, "" );

		if( LightMap->PaletteSize ) {
			GXLoadTlut( &LightMap->TlutObj, MapInd );
			GXInitTexObjTlut( &LightMap->TexObj, MapInd );
		}

		GXLoadTexObj( &LightMap->TexObj, nglTevList[MapInd].map );
		{
			// FIXME: need to fix the blend mode to accept parms from a previous stage
			nglSetupBlendModeOLD( Material->LightMapBlendMode,
										Material->LightMapBlendModeConstant,
										nglTevList[i].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
		}
		GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_FALSE, GX_FALSE );

		if( FirstMode == -1 ) {
			FirstMode = Material->LightMapBlendMode;
		}

		++i;
		CoordInd++;
		MapInd++;
	}

	nglGCTempSrcClr = GX_CC_TEXC;
	nglGCTempSrcAlpha = GX_CA_TEXA;

	// always 1 color channel
	if (nglGCDoShadowMap)
		GXSetNumChans( 2 );
	else
		GXSetNumChans( 1 );

	if (nglGCDoShadowMap)
	{
		nglSetClampMode( NGLMAP_CLAMP_U | NGLMAP_CLAMP_V, TheProjLight->Tex );
		nglSetFilterMode(NGLMAP_BILINEAR_FILTER, TheProjLight->Tex );
		// <<<< 23-Jan-2002 SAL: Must set filter and clamp modes here.
		if (nglGCUseWhiteShadow)
			GXLoadTexObj(&nglWhiteTex.TexObj, nglTevList[MapInd].map);
		else
			GXLoadTexObj(&TheProjLight->Tex->TexObj, nglTevList[MapInd].map);

		nglSetupTevStage( NGLBM_PROJ_SHADOW_S1, //TheProjLight->BlendMode,
									TheProjLight->BlendModeConstant,
									nglTevList[i].stage,
									nglTevList[CoordInd].coord,
									nglTevList[MapInd].map );
		nglSetupTevStage( NGLBM_PROJ_SHADOW_S2, //TheProjLight->BlendMode,
									TheProjLight->BlendModeConstant,
									nglTevList[i+1].stage,
									nglTevList[CoordInd].coord,
									nglTevList[MapInd].map );
		GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_FALSE, GX_FALSE );

		++i;
		++i;
		CoordInd++;
		MapInd++;
	}

	// We specify this outside of the above expression because
	// we need to bump the blend mode if the tinting was taken
	// care of in the lighting stage and we went through
	// nglBlendVertexColor above.
	if (STAGE_ENABLE(LightAmb) && (Params->Flags&NGLP_TINT))
	{

		if( Params->TintColor[3] != 1.0f ) {

			// FIXME: hack to promote the blend mode if tinting is being used
			if( Material->MapBlendMode == NGLBM_ADDITIVE ) {
				GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP );
			}
			else if( (!Params->TintColor[0]) && (!Params->TintColor[1]) && (!Params->TintColor[2]) )
			{
				// <<<< 14-Jan-2002 SAL: Special fix to get shadows to work right. Must consider using a separate blend type for shadows.
				GXSetBlendMode( GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP );
			}
			else
			{
				GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP );
			}

		}

	}

	GXSetNumTevStages( ( i > 0 ) ? i : 1 );

	// deal with alpha compare
	if( FirstMode == NGLBM_PUNCHTHROUGH ) {
		GXSetAlphaCompare( GX_GREATER, 128, GX_AOP_AND, GX_ALWAYS, 0 );
		GXSetZCompLoc( GX_FALSE );
	} else {
		GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0 );
		GXSetZCompLoc( GX_TRUE );
	}

	// setup z mode based on first layer
	if( ( FirstMode == NGLBM_OPAQUE ) || ( FirstMode == NGLBM_PUNCHTHROUGH ) )
	{
    SceneGXSetZMode(/*test*/ true, /*write*/ true);
	}
	else if( FirstMode == NGLBM_MOTION_BLUR)
	{
    SceneGXSetZMode(/*test*/ false, /*write*/ false);
	}
	else
	{
    SceneGXSetZMode(/*test*/ true, /*write*/ false);
	}

	nglGCTempSrcVCC = GX_CC_RASC;
	nglGCTempSrcVCA = GX_CA_RASA;

	return i;
}

//-----------------------------------------------------------------------------


static NGL_INLINE void nglGenEnvCoords( u_short PIdx, u_short NIdx )
{
	// <<<< 27-Sep-2001 SAL: This assumes that the position and normal info is in the same place for both
	// nglVertex and nglSkinVertex. If we don't implement this code in the GPU then revisit the issue.
	// Also we are assuming that size of nglVertex and nglSkinVertex will always be a multiple of 4.
	nglVertP* Vert = &nglEnvMapVertP[PIdx];
	nglVertN* VertN = &nglEnvMapVertN[NIdx];
	// Transform the point and normals to World coordinates (To implement).
	nglVector TVert,TNrm, TNrm2;

#ifdef NGL_FAST_MATH
	nglSpecialApplyMatrix( TVert, nglMeshLocalToWorld, (nglVector&) *Vert );
#else
	TVert[0] = Vert->x * nglMeshLocalToWorld[0][0] + Vert->y * nglMeshLocalToWorld[1][0] + Vert->z * nglMeshLocalToWorld[2][0] + nglMeshLocalToWorld[3][0];
	TVert[1] = Vert->x * nglMeshLocalToWorld[0][1] + Vert->y * nglMeshLocalToWorld[1][1] + Vert->z * nglMeshLocalToWorld[2][1] + nglMeshLocalToWorld[3][1];
	TVert[2] = Vert->x * nglMeshLocalToWorld[0][2] + Vert->y * nglMeshLocalToWorld[1][2] + Vert->z * nglMeshLocalToWorld[2][2] + nglMeshLocalToWorld[3][2];
#endif

	// Unpack the normals.
	TNrm2[0] = VertN->nx * NGLGC_UNSCALE_NRM;
	TNrm2[1] = VertN->ny * NGLGC_UNSCALE_NRM;
	TNrm2[2] = VertN->nz * NGLGC_UNSCALE_NRM;

#ifdef NGL_FAST_MATH
	nglSpecialNormalTransformMatrix2( TNrm, nglMeshLocalToWorld, TNrm2 );
#else
	TNrm[0] = TNrm2[0] * nglMeshLocalToWorld[0][0] + TNrm2[1] * nglMeshLocalToWorld[1][0] + TNrm2[2] * nglMeshLocalToWorld[2][0];
	TNrm[1] = TNrm2[0] * nglMeshLocalToWorld[0][1] + TNrm2[1] * nglMeshLocalToWorld[1][1] + TNrm2[2] * nglMeshLocalToWorld[2][1];
	TNrm[2] = TNrm2[0] * nglMeshLocalToWorld[0][2] + TNrm2[1] * nglMeshLocalToWorld[1][2] + TNrm2[2] * nglMeshLocalToWorld[2][2];
#endif

	nglVector Vec1,Refl;
	float DotProd;

#ifdef NGL_FAST_MATH
	nglSubVectorXYZ( Vec1, TVert, nglCurScene->ViewToWorld[3] );
#else
	Vec1[0]=TVert[0]-nglCurScene->ViewToWorld[3][0];
	Vec1[1]=TVert[1]-nglCurScene->ViewToWorld[3][1];
	Vec1[2]=TVert[2]-nglCurScene->ViewToWorld[3][2];
#endif

#ifdef NGL_FAST_MATH
	DotProd = nglInnerProduct( Vec1, TNrm );
#else
	DotProd=Vec1[0]*TNrm[0]+Vec1[1]*TNrm[1]+Vec1[2]*TNrm[2];
#endif
	Refl[0]=Vec1[0]-2*DotProd*TNrm[0];
	Refl[1]=Vec1[1]-2*DotProd*TNrm[1];
	Refl[2]=Vec1[2]-2*DotProd*TNrm[2];

// Test adding the length to Refl[2] (thus skipping a divide).
	Refl[2] += sqrtf( Refl[0] * Refl[0] + Refl[1] * Refl[1] + Refl[2] * Refl[2] );

#if USE_NBT_FOR_ENVMAP
	GXNormal3f32( TNrm2[0], TNrm2[1], TNrm2[2] );
	GXNormal3f32( Refl[0], Refl[1], Refl[2] );
	GXNormal3f32( 0.0f, 0.0f, 0.0f );
#else
	float m,FVal1,FVal2;

	FVal1=Refl[0]*Refl[0]+Refl[1]*Refl[1]+Refl[2]*Refl[2];
	m=2*sqrtf(FVal1);

	if(m<0.00005f)
		m=0.00005f;

	FVal1=Refl[0]/m + 0.5f;
	FVal2=Refl[1]/m + 0.5f;
	FVal2 = 1.0f - FVal2; // 24-Jan-2002 SAL: Flip the y to match our textures.

	GXTexCoord2f32( FVal1, FVal2 );
#endif //#if USE_NBT_FOR_ENVMAP
}

//-----------------------------------------------------------------------------

NGL_INLINE void nglSendT( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
		GXNormal1x16( VertMap[Idx] );
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendTD( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
		GXNormal1x16( VertMap[Idx] );
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendE( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
#if 0 //USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( VertMap[Idx] );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( Idx );
#if 0 //!USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendTE( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
#if 0 //USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( VertMap[Idx] );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
#if 0 //!USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendTDE( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
#if 0 //USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( VertMap[Idx] );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
		GXTexCoord1x16( Idx );
#if 0 //!USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
		GXNormal1x16( VertMap[Idx] );
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendTL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
		GXNormal1x16( VertMap[Idx] );
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendTDL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
		GXNormal1x16( VertMap[Idx] );
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
		GXTexCoord1x16( Idx );
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendEL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
#if 0 //USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( VertMap[Idx] );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( Idx );
#if 0 //!USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendTEL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
#if 0 //USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( VertMap[Idx] );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
#if 0 //!USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( Idx );
	}
}

NGL_INLINE void nglSendTDEL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		u_short Idx = Indices[j];
		GXPosition1x16( VertMap[Idx] );
#if 0 //USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( VertMap[Idx] );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( Idx );
		GXTexCoord1x16( Idx );
		GXTexCoord1x16( Idx );
#if 0 //!USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( VertMap[Idx] );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( Idx );
	}
}

//-----------------------------------------------------------------------------

NGL_INLINE void nglSendTNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
		GXNormal1x16( j );
		GXColor1x16( j );
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendTDNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
		GXNormal1x16( j );
		GXColor1x16( j );
		GXTexCoord1x16( j );
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendENoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( j );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( j );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}


NGL_INLINE void nglSendTENoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( j );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( j );
		GXTexCoord1x16( j );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendTDENoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( j );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( j );
		GXTexCoord1x16( j );
		GXTexCoord1x16( j );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendLNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
		GXNormal1x16( j );
		GXColor1x16( j );
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendTLNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
		GXNormal1x16( j );
		GXColor1x16( j );
		GXTexCoord1x16( j );
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendTDLNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
		GXNormal1x16( j );
		GXColor1x16( j );
		GXTexCoord1x16( j );
		GXTexCoord1x16( j );
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendELNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( j );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( j );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendTELNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( j );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( j );
		GXTexCoord1x16( j );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( j );
	}
}

NGL_INLINE void nglSendTDELNoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( j );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( j );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( j );
		GXTexCoord1x16( j );
		GXTexCoord1x16( j );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( j );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( j );
	}
}

//-----------------------------------------------------------------------------

NGL_INLINE void nglSendT2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; j++ ) {
		GXPosition1x16( *(Indices++) );
		GXNormal1x16( *(Indices++) );
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendTD2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; j++ ) {
		GXPosition1x16( *(Indices++) );
		GXNormal1x16( *(Indices++) );
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendE2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( (Indices[-1]), (Indices[0]) );
		Indices++;
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( *(Indices++) );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices++) );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( *(Indices-3), *(Indices-2) );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendTE2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( (Indices[-1]), (Indices[0]) );
		Indices++;
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( *(Indices++) );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( *(Indices-3), *(Indices-2) );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendTDE2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( (Indices[-1]), (Indices[0]) );
		Indices++;
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( *(Indices++) );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( *(Indices-3), *(Indices-2) );
#endif //#if !USE_NBT_FOR_ENVMAP
	}
}

NGL_INLINE void nglSendL2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
		GXNormal1x16( *(Indices++) );
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendTL2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
		GXNormal1x16( *(Indices++) );
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendTDL2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
		GXNormal1x16( *(Indices++) );
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendEL2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( (Indices[-1]), (Indices[0]) );
		Indices++;
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( *(Indices++) );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( *(Indices-2), *(Indices-1) );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendTEL2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( (Indices[-1]), (Indices[0]) );
		Indices++;
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( *(Indices++) );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( *(Indices-2), *(Indices-1) );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( *(Indices++) );
	}
}

NGL_INLINE void nglSendTDEL2COL( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j ) {
		GXPosition1x16( *(Indices++) );
#if USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( (Indices[-1]), (Indices[0]) );
		Indices++;
#else //#if USE_NBT_FOR_ENVMAP
		GXNormal1x16( *(Indices++) );
#endif //else //#if USE_NBT_FOR_ENVMAP
		GXColor1x16( *(Indices) );
		GXColor1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
		GXTexCoord1x16( *(Indices) );
#if !USE_NBT_FOR_ENVMAP
		nglGenEnvCoords( *(Indices-2), *(Indices-1) );
#endif //#if !USE_NBT_FOR_ENVMAP
		GXTexCoord1x16( *(Indices++) );
	}
}

//-----------------------------------------------------------------------------

typedef void (*nglSendFunc)( u_int NIndices, u_short* Indices );

static nglSendFunc nglSendFuncs[] = {
	NULL,						// 0
	&nglSendT,
	NULL,
	&nglSendTD,
	&nglSendE,
	&nglSendTE,					// 5
	NULL,
	&nglSendTDE,
	&nglSendL,
	&nglSendTL,
	NULL,                      // 10
	&nglSendTDL,
	&nglSendEL,
	&nglSendTEL,
	NULL,
	&nglSendTDEL,              // 15
	NULL,
	&nglSendTNoIndex,
	NULL,
	&nglSendTDNoIndex,
	&nglSendENoIndex,          // 20
	&nglSendTENoIndex,
	NULL,
	&nglSendTDENoIndex,
	&nglSendLNoIndex,
	&nglSendTLNoIndex,         // 25
	NULL,
	&nglSendTDLNoIndex,
	&nglSendELNoIndex,
	&nglSendTELNoIndex,
	NULL,                      // 30
	&nglSendTDELNoIndex,
	NULL,
	&nglSendT2COL,
	NULL,
	&nglSendTD2COL,            // 35
	&nglSendE2COL,
	&nglSendTE2COL,
	NULL,
	&nglSendTDE2COL,
	&nglSendL2COL,
	&nglSendTL2COL,
	NULL,
	&nglSendTDL2COL,
	&nglSendEL2COL,
	&nglSendTEL2COL,
	NULL,
	&nglSendTDEL2COL,
};

#define NGL_MAX_BONES 64
static nglMatrix nglBoneWork[NGL_MAX_BONES];
Mtx StorePosMtx1;
nglMatrix StorePosMtx2;

static void nglSendVerts( nglMesh* Mesh, nglMeshSection* Section, int RMode, u_int MeshFlags, nglRenderParams* Params )
{
	u_int NIndices = 0;

	if (MeshFlags & NGLMESH_SCRATCH)
	{
		NIndices = Section->NVerts;
		RMode |= NGL_RMODE_NOINDEX;
	}
	else if (MeshFlags & NGLMESH_SKINNED)
	{
		NIndices = Section->NIndices / 3; // Position, Normal, and Data each count towards NIndices set by the MeshCvt.
	}
	else
	{
		NIndices = Section->NIndices / 3; // Position, Normal, and Data each count towards NIndices set by the MeshCvt.
		RMode |= NGL_RMODE_2_COL;
	}

	if( Mesh->NBones )
	{
		nglMatrix temp_world_to_local;
		nglInverseMatrix( temp_world_to_local, nglMeshLocalToWorld );
		NIndices = Section->BoneIndexCounts[0];
		// Reset the matrix, etc.
		GXSetArray( GX_VA_POS, nglSkinWork, sizeof( nglVector ) * 2 );
		GXSetArray( GX_VA_NRM, &nglSkinWork[1], sizeof( nglVector ) * 2 );
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0 );
		GXLoadPosMtxImm( StorePosMtx1, GX_PNMTX0 );
		GXLoadNrmMtxImm( (float (*)[4])&temp_world_to_local[0][0], GX_PNMTX0 );
		if( RMode & NGL_RMODE_ENVIRONMENT )
		{
			Mtx wmzp;
			nglMatrix Work2;
			nglMulMatrix( Work2, nglMeshLocalToWorld, nglCurScene->WorldToView );
			nglToMtx( wmzp, Work2 );
			wmzp[0][3] = wmzp[1][3] = 0;
			wmzp[2][3] = 1;
	    	GXLoadTexMtxImm( (float (*)[4])&wmzp, nglTexMtx[NGLTEXMTX_ENVMAP], GX_MTX3x4 );
		}
	}

	u_short* Indices = Section->Indices;
 	GXInvalidateVtxCache();

	bool UseDisplayList = false;
	if ( NIndices )
	{
		if (MeshFlags & NGLMESH_SCRATCH)
		{
			nglVertex* Verts = (nglVertex*)Section->Verts;
			short* NVerts = Section->VertMap;

			for (u_int strip=0;strip<Section->NTris;strip++)
			{
				GXSetArray( GX_VA_POS, &Verts->x, sizeof( nglVertex ) );
				GXSetArray( GX_VA_NRM, &Verts->nx, sizeof( nglVertex ) );
				GXSetArray( GX_VA_CLR0, &Verts->color, sizeof( nglVertex ) );
				GXSetArray( GX_VA_TEX0, &Verts->u1, sizeof( nglVertex ) );
				GXSetArray( LightMapAttrCache, &Verts->u2, sizeof( nglVertex ) );
				GXSetArray( DetailMapAttrCache, &Verts->u1, sizeof( nglVertex ) );

#if NGL_PROFILING
				nglPerfInfo.TotalVerts += *NVerts;
  				nglPerfInfo.TotalPolys += *NVerts - 2;
#endif // NGL_PROFILING

				if (DEBUG_ENABLE(DrawAsLines))
					GXBegin( GX_LINESTRIP, GX_VTXFMT0, *NVerts );
				else
					GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, *NVerts );
					nglSendFuncs[ RMode ]( *NVerts, NULL );
				GXEnd();

				Verts += *NVerts;
				NVerts++;
			}
		}
		else
		{

			if(!UseDisplayList)
			{
#if NGL_PROFILING
				nglPerfInfo.TotalVerts += NIndices;
  				nglPerfInfo.TotalPolys += NIndices - 2;
#endif // NGL_PROFILING

				if (DEBUG_ENABLE(DrawAsLines))
					GXBegin( GX_LINESTRIP, GX_VTXFMT0, NIndices );
				else
  					GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, NIndices );
			}

			NGL_ASSERT( RMode >= 0 && RMode <= ( NGL_RMODE_TDEL | NGL_RMODE_2_COL ), "" );
			NGL_ASSERT( nglSendFuncs[RMode], "" );

			if (!UseDisplayList)
			{
				nglSendFuncs[ RMode ]( NIndices, Indices );
				GXEnd();
			}
			else
			{
#if NGL_PROFILING
				nglPerfInfo.TotalVerts += Section->NIndices;
  				nglPerfInfo.TotalPolys += Section->NIndices - 2;
#endif // NGL_PROFILING

				GXCallDisplayList( (void *)Indices, Section->NIndices );
			}
		}
	}

	if( Mesh->NBones )
	{
		// Switch Format and draw the other bones.
		nglSkinnedVertex* Verts = (nglSkinnedVertex*) Section->Verts;
		nglSkinVertPN* VertPNs = (nglSkinVertPN*) Section->VertPNs;
		// <<<< 04-Dec-2001 SAL: EnvMap support has to be reimplemented for skinned characters.
		nglVertEnvMapTemp = (int*) VertPNs;
		nglVertEnvMapSize = sizeof( nglSkinnedVertex ) >> 2;
		GXSetArray( GX_VA_POS, &VertPNs->x, sizeof( nglSkinVertPN ) );
		GXSetArray( GX_VA_NRM, &VertPNs->nx, sizeof( nglSkinVertPN ) );
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14 );
	 	GXInvalidateVtxCache( );

		for (int i=0; i<Mesh->NBones; i++)
		{
			Indices += NIndices;
			NIndices = Section->BoneIndexCounts[i+1];

			if (NIndices)
			{
				nglMatrix Work;
				// get our world to view matrix (camera) multiplied
				// against the local to world matrix of this mesh
				nglMulMatrix( Work, nglBoneWork[i], StorePosMtx2 );
				Mtx wmzp;
				nglToMtx( wmzp, Work );
				GXLoadPosMtxImm( wmzp, GX_PNMTX0 );
				// Load the matrix for normal transformation.
				nglMulMatrix( Work, nglBoneWork[i], nglMeshLocalToWorld );
				nglToMtx( wmzp, Work );
				GXLoadNrmMtxImm( wmzp, GX_PNMTX0 );
				if (RMode & NGL_RMODE_ENVIRONMENT)
				{
					nglMatrix Work2;
					nglMulMatrix( Work2, Work, nglCurScene->WorldToView );
					nglToMtx( wmzp, Work2 );
					wmzp[0][3] = wmzp[1][3] = 0;
					wmzp[2][3] = 1;
			    	GXLoadTexMtxImm( (float (*)[4])&wmzp, nglTexMtx[NGLTEXMTX_ENVMAP], GX_MTX3x4 );
				}
#if NGL_PROFILING
				nglPerfInfo.TotalVerts += NIndices;
  				nglPerfInfo.TotalPolys += NIndices - 2;
#endif // NGL_PROFILING

				if (DEBUG_ENABLE(DrawAsLines))
					GXBegin( GX_LINESTRIP, GX_VTXFMT0, NIndices );
				else
					GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, NIndices );
					nglSendFuncs[ RMode ]( NIndices, Indices );
				GXEnd( );
			}
		}
	}
}

static void nglSetupBones( const nglMeshNode* MeshNode )
{
	const nglMesh* Mesh = MeshNode->Mesh;
	const nglRenderParams* Params = &MeshNode->Params;

	NGL_ASSERT( Mesh->Flags & NGLMESH_SKINNED, "" );
	NGL_ASSERT( Params->NBones < NGL_MAX_BONES, "" );
	NGL_ASSERT( Mesh->NBones == Params->NBones, "" );

	for( int i = 0; i < Params->NBones; i++ ) {

		if( Params->Flags & NGLP_BONES_LOCAL ) {
			NGL_ASSERT( 0, "" ); // 25-Oct-2001 SAL: Haven't tested this. If this is used then it needs to be tested.
			nglCopyMatrix( nglBoneWork[i], Params->Bones[i] );
		} else {
			nglMatrix Work,Work2;

			nglMatrix& Mf = Mesh->Bones[i];
			nglMatrix& Pf = Params->Bones[i];

			nglInverseMatrix( Work, Mesh->Bones[i] );
			nglMulMatrix( Work2, Params->Bones[i], MeshNode->WorldToLocal );
			nglMulMatrix( nglBoneWork[i], Work, Work2 );
		}

	}

}

static void nglSkinVerts( nglMeshSection* Section )
{
	int LoopStart=0, LoopEnd;
	LoopEnd = Section->BoneIndexCounts[Section->NBones+1];
	int i = 0;
	nglVector *SkinWorkPos = &nglSkinWork[0];
	nglSkinVertPN* Vert = &( ( (nglSkinVertPN*) Section->VertPNs )[0] );

	for( i=0; i<LoopEnd; i++, Vert++ )
	{
		int Index = Vert->index1;
		NGL_ASSERT( Index < NGL_MAX_BONES, "" );

		nglMatrix& M = nglBoneWork[ Index ];

		nglVector *SrcPos = (nglVector *)&Vert->x;
		nglSpecialApplyMatrix( *(SkinWorkPos++), M, *SrcPos );
		short *SrcNrm = (short *)&Vert->nx;
		float VertNrm[3] = { NGLGC_UNSCALE_NRM * (*SrcNrm), NGLGC_UNSCALE_NRM * (*(SrcNrm+1)),
			NGLGC_UNSCALE_NRM * (*(SrcNrm+2)), };
		nglSpecialNormalTransformMatrix2( *(SkinWorkPos++), M, VertNrm );
	}

	LoopStart = LoopEnd;
	LoopEnd += Section->BoneIndexCounts[Section->NBones+2];

	for( i=LoopStart; i<LoopEnd; i++, Vert++ )
	{
		int Index = Vert->index1;
		NGL_ASSERT( Index < NGL_MAX_BONES, "" );
		nglMatrix& M = nglBoneWork[ Index ];
		Index = Vert->index2;
		NGL_ASSERT( Index < NGL_MAX_BONES, "" );
		nglMatrix& M2 = nglBoneWork[ Index ];

		nglVector *SrcPos = (nglVector *)&Vert->x;
		nglVector Vd1, Vd2;
		nglSpecialApplyMatrix( Vd1, M, *SrcPos );
		nglSpecialApplyMatrix( Vd2, M2, *SrcPos );
		float Weight1 = Vert->weight1 * NGLGC_UNSCALE_WEIGHT, Weight2 = Vert->weight2 * NGLGC_UNSCALE_WEIGHT;
		(*SkinWorkPos)[0] = Weight1 * Vd1[0] + Weight2 * Vd2[0];
		(*SkinWorkPos)[1] = Weight1 * Vd1[1] + Weight2 * Vd2[1];
		(*(SkinWorkPos++))[2] = Weight1 * Vd1[2] + Weight2 * Vd2[2];
		short *SrcNrm = (short *)&Vert->nx;
		float VertNrm[3] = { NGLGC_UNSCALE_NRM * (*SrcNrm), NGLGC_UNSCALE_NRM * (*(SrcNrm+1)),
			NGLGC_UNSCALE_NRM * (*(SrcNrm+2)), };
		nglSpecialNormalTransformMatrix2( Vd1, M, VertNrm );
		nglSpecialNormalTransformMatrix2( Vd2, M2, VertNrm );
		(*SkinWorkPos)[0] = Weight1 * Vd1[0] + Weight2 * Vd2[0];
		(*SkinWorkPos)[1] = Weight1 * Vd1[1] + Weight2 * Vd2[1];
		(*(SkinWorkPos++))[2] = Weight1 * Vd1[2] + Weight2 * Vd2[2];

	}

	LoopStart = LoopEnd;
	LoopEnd += Section->BoneIndexCounts[Section->NBones+3];
	NGL_ASSERT( LoopEnd == Section->NCPUVerts, "" );

	for( i = LoopStart; i < LoopEnd; ++i )
	{
		nglSkinVertPN* Vert = &( ( (nglSkinVertPN*) Section->VertPNs )[i] );
		u_char* Indices = &Vert->index1;
		float Weights[4] = { Vert->weight1 * NGLGC_UNSCALE_WEIGHT, Vert->weight2 * NGLGC_UNSCALE_WEIGHT,
			Vert->weight3 * NGLGC_UNSCALE_WEIGHT, Vert->weight4 * NGLGC_UNSCALE_WEIGHT };

		nglVector Vs( Vert->x, Vert->y, Vert->z, 1.0f );
		nglVector Vns( Vert->nx, Vert->ny, Vert->nz, 1.0f );
		nglVector& Va = nglSkinWork[ i * 2 ];
		nglVector& Vna = nglSkinWork[ i * 2 + 1 ];

		Va[0] = 0.0f;
		Va[1] = 0.0f;
		Va[2] = 0.0f;
		Va[3] = 1.0f;

		Vna[0] = 0.0f;
		Vna[1] = 0.0f;
		Vna[2] = 0.0f;
		Vna[3] = 1.0f;

		NGL_ASSERT( Vert->bones <= 4, "" );
		// index{1,2,3,4} weight{1,2,3,4}
		for( int j = 0; j < Vert->bones; j++ ) {
			NGL_ASSERT( Indices[j] < NGL_MAX_BONES, "" );

			nglMatrix& M = nglBoneWork[ Indices[j] ];
			nglVector Vd,Vn;

			nglApplyMatrix( Vd, M, Vs );
			nglSpecialNormalTransformMatrix( Vn, M, Vns );

			Vd[0] *= Weights[j];
			Vd[1] *= Weights[j];
			Vd[2] *= Weights[j];

			Vn[0] *= Weights[j];
			Vn[1] *= Weights[j];
			Vn[2] *= Weights[j];

			Va[0] += Vd[0];
			Va[1] += Vd[1];
			Va[2] += Vd[2];

			Vna[0] += Vn[0];
			Vna[1] += Vn[1];
			Vna[2] += Vn[2];
		}
	}

	// Because our writes to nglSkinWork are still in the
	// cache we need to force a flush and then invalidate
	// the vertex cache.
	DCStoreRangeNoSync( nglSkinWork, Section->NCPUVerts * sizeof( nglVector ) * 2 );
	GXInvalidateVtxCache();
}

nglMatrix nglGCmz; // Globalize to prevent frequent recomputation.
float nglGCCameraPosInWorld[3];

static void nglRenderMesh(nglMeshNode* MeshNode, nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params)
{
	nglMaterial* Material = Section->Material;

	// FIXME: make this write a var and pass it to GXSetCullMode so we can see it in the debugger
	// set the backface culling based on material flags
	if ((Material->Flags&NGLMAT_BACKFACE_CULL) || (Material->Flags&NGLMAT_BACKFACE_DEFAULT))
	  GXSetCullMode( ( Params->Flags & NGLP_REVERSE_BACKFACECULL ) ? GX_CULL_FRONT : GX_CULL_BACK );
	else
		GXSetCullMode(GX_CULL_NONE);

	if (Mesh->Flags&NGLMESH_SKINNED)
	{
		if (!STAGE_ENABLE(Skin))
			return;

		// Use the normals stored as nglVectors.
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

		// this is here rather than below because nglSetupVtxFmt
		// uses nglSkinWork for setting up the shared vertex array
		nglSkinWork = (nglVector*)nglListAlloc(Section->NVerts * sizeof(nglVector) * 2);

		if (!nglSkinWork)
		{
			nglPrintf("nglRenderMesh: couldn't allocate nglSkinWork.\n");
			return;
		}

		nglSkinVerts( Section );

		nglGCSkipEnvMap = true;
		nglGCSkinMesh = true;
	}
	else
	{
		// Reset to the packed format of the vertices.
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14 );
	}

	if (Mesh->Flags & NGLMESH_SCRATCH)
		nglGCSkipEnvMap = true;

	nglSetupMeshProjLighting( Mesh, Section, Params );

	int RMode = nglSetupVtxFmt(MeshNode, Mesh, Section, Material, Params);

	if (!RMode)
	{
		// FIXME: non-textured things are geborked
		if (Section->Material)
		{
			Material->Map=&nglWhiteTex;
			Material->Flags|=NGLMAT_TEXTURE_MAP;
			RMode = nglSetupVtxFmt(MeshNode, Mesh, Section, Material, Params);
		}

/*
		if (!RMode)
		{
			Section->Material = &nglWhiteMat;
			Material = &nglWhiteMat;
			RMode = nglSetupVtxFmt( Mesh, Section, Material, Params );
		}
*/

		if (!RMode)
		{
			nglPrintf( "RMode is totally bogus still.\n" );
			nglGCSkipEnvMap = false;
			nglGCDoShadowMap = false;
			nglGCSkinMesh = false;
			return;
		}
	}

	nglSetupMeshLighting( Section, Mesh, MeshNode, Params );
	nglSetupTevs( Material, Params );

	if (Mesh->Flags & NGLMESH_SKINNED)
		PPCSync();

	nglSendVerts( Mesh, Section, RMode, Mesh->Flags, Params );

	nglGCDoShadowMap = false;
	nglGCSkinMesh = false;
	nglGCSkipEnvMap = false;
}

//-----------------------------------------------------------------------------
void nglRenderMeshCallback( void* Data )
{
	nglMeshSectionNode* Node = (nglMeshSectionNode*) Data;

	nglMeshNode* MeshNode = Node->MeshNode;
	nglMesh* Mesh = MeshNode->Mesh;
	nglMeshSection* Section = Node->Section;
	nglRenderParams* Params = &MeshNode->Params;

	if (Mesh->Flags & NGLMESH_SCRATCH)
		if (!STAGE_ENABLE(Scratch))
			return;

	// needed for lighting
	nglCopyMatrix(nglMeshWorldToLocal, MeshNode->WorldToLocal);

	nglMatrix z;
	nglIdentityMatrix(z);
	z[2][2] = -1.0f;

	if (Params->Flags & NGLP_FULL_MATRIX)
	{
		// FIXME: setup an ortho projection of some sort

		nglMatrix sz;
		nglMulMatrix(sz, MeshNode->LocalToScreen, z);
		Mtx szp;
		nglToMtx(szp, sz);
		GXLoadPosMtxImm(szp, GX_PNMTX0);
	}
	else
	{
		if (Mesh->Flags & NGLMESH_SKINNED)
		{
			nglSetupBones( MeshNode );
		}

		// get our world to view matrix (camera) multiplied
		// against the local to world matrix of this mesh
		nglMatrix wmz;
		nglMulMatrix( wmz, MeshNode->LocalToWorld, nglGCmz );
		Mtx wmzp;
		nglToMtx( wmzp, wmz );

		// load the position and normal matricies (normal for lighting)
		GXLoadPosMtxImm(wmzp, GX_PNMTX0);
		GXLoadNrmMtxImm(wmzp, GX_PNMTX0);

		if (Mesh->NBones)
		{
			memcpy( &StorePosMtx1, &wmzp, sizeof(Mtx) );
			memcpy( &StorePosMtx2[0][0], &wmz[0][0], sizeof(nglMatrix) );
		}
	}

	// hack for env. mapping
	// nglMeshWorldToLocal is used in lighting.
	// FIXME: get rid of this and pass the MeshNode correctly
	nglCopyMatrix( nglMeshLocalToWorld, MeshNode->LocalToWorld );
	while( Node )
	{
		nglRenderMesh(MeshNode, Mesh, Section, Params);
		Node = Node->NextSectionNode;
		if( Node )
			Section = Node->Section;
	}
}

// this function clears the EFB with a quad, we project to GX_MAX_Z and
// then write GX_MAX_Z-1 because the hardware always clips at GX_MAX_Z
// FIXME: need to implement ClearZ parameter
static void _nglClearImmediate(u32 Color, const u_int ClearFlags)
{
	// check the z clear
	if (ClearFlags&NGLCLEAR_Z)
		GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_ENABLE);
	else
		GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_DISABLE);
	// check the color clear
	if (ClearFlags&NGLCLEAR_COLOR)
		GXSetColorUpdate(GX_TRUE);
	else
		GXSetColorUpdate(GX_FALSE);

	GXSetZCompLoc(GX_FALSE);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

	// turn off clipping and culling
	GXSetCullMode(GX_CULL_NONE);
	GXSetClipMode(GX_CLIP_DISABLE);
	// set an ortho matrix to the screen
	Mtx44 QuadMtx;
	MTXOrtho(QuadMtx, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, GX_MAX_Z24);
	GXSetProjection(QuadMtx, GX_ORTHOGRAPHIC);
	// reset our camera matrix to the identity
	GXLoadPosMtxImm(_nglMTXIdentity, GX_PNMTX0);
	GXSetCurrentMtx(GX_PNMTX0);
	// set our lighting to generate color0 using the vertex color
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	// Setup number of Tev stages etc.
	GXSetNumTexGens(0); // no texture
	GXSetNumTevStages(1);
	// set the blend
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	// pass the color straight through
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	// pass the alpha straight through
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
	// Setup the vertex format.
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	// render the quad
	GXBegin(GX_QUADS, GX_VTXFMT1, 4);
		// top left
		GXPosition3f32(0.0f, 0.0f, -(GX_MAX_Z24-1));
		GXColor1u32(Color);
		// top right
		GXPosition3f32(1.0f, 0.0f, -(GX_MAX_Z24-1));
		GXColor1u32(Color);
		// bottom right
		GXPosition3f32(1.0f, 1.0f, -(GX_MAX_Z24-1));
		GXColor1u32(Color);
		// bottom left
		GXPosition3f32(0.0f, 1.0f, -(GX_MAX_Z24-1));
		GXColor1u32(Color);
	GXEnd();
	// reset values necessary for drawing
	GXSetCullMode(GX_CULL_BACK);
	GXSetClipMode(GX_CLIP_ENABLE);
	GXSetColorUpdate(GX_TRUE);
}

f32 DefaultProjectionForScene[GX_PROJECTION_SZ];
f32 DefaultQuadProjectionForScene[GX_PROJECTION_SZ];

static void nglRenderScene(nglScene* Scene)
{
	nglScene* Child = Scene->FirstChild;

	while(Child)
	{
		nglRenderScene(Child);
		Child = Child->NextSibling;
	}

	nglCurScene = Scene;

  Scene->IFLFrame = (u_int) ( Scene->AnimTime * nglIFLSpeed );

	#if 0 //def PROJECT_KELLYSLATER
	float SceneViewX1 = nglCurScene->ViewX1;
	float SceneViewY1 = nglCurScene->ViewY1;
	if (((int) SceneViewX1	) & 1 ) SceneViewX1 -= 1.0f;
	if (((int) SceneViewY1	) & 1 ) SceneViewY1 -= 1.0f;
	float SceneWidth = (nglCurScene->ViewX2 - SceneViewX1);
	float	SceneHeight = (nglCurScene->ViewY2 - SceneViewY1);
	if (((int) SceneWidth	) & 1 ) SceneWidth +=1.0f;
	if (((int) SceneHeight	) & 1 ) SceneHeight +=1.0f;
	float SceneViewX2 = nglCurScene->ViewX2;
	float SceneViewY2 = nglCurScene->ViewY2;
	#else
	float SceneViewX1 = nglCurScene->ViewX1;
	float SceneViewY1 = nglCurScene->ViewY1;
	float SceneViewX2 = nglCurScene->ViewX2;
	float SceneViewY2 = nglCurScene->ViewY2;
	// the viewport is always sent as a rectangle, so we need to nudge it to width and height
	float SceneWidth  = (nglCurScene->ViewX2 - SceneViewX1) + 1.0f;
	float SceneHeight = (nglCurScene->ViewY2 - SceneViewY1) + 1.0f;
	#endif

	#if 1 //ndef PROJECT_KELLYSLATER
	// adjust the scene parameters if it's a render target
	if (nglCurScene->RenderTarget)
	{
  		nglTexture* Target = nglCurScene->RenderTarget;

		// check and fixup the width
		if (SceneWidth > Target->Width)
		{
			float WidthDiff = SceneWidth - Target->Width;
			SceneWidth -= WidthDiff;
			SceneViewX2 -= WidthDiff;
		}
		// check and fixup the height
		if (SceneHeight > Target->Height)
		{
			float HeightDiff = SceneHeight - Target->Height;
			SceneHeight -= HeightDiff;
			SceneViewY2 -= HeightDiff;
		}
	}
	#endif
/*
	else
	{
		nglPrintf("  Scene Width, Height: %f, %f\n", SceneWidth, SceneHeight);
	}
*/

	if ((SceneWidth == 0.0f) || (SceneHeight == 0.0f))
	{
		nglWarning( "We shouldn't be using a viewport which has zero dimensions (or too close to it)" );
		return;
	}

	// Set the Z range.
	if( nglRenderModeObj.field_rendering )
		GXSetViewportJitter(SceneViewX1, SceneViewY1, SceneWidth, SceneHeight, 0.0f, 1.0f, VIGetNextField());
	else
		GXSetViewport(SceneViewX1, SceneViewY1, SceneWidth, SceneHeight, 0.0f, 1.0f);

	GXSetScissor(SceneViewX1, SceneViewY1, SceneWidth, SceneHeight);

	// clear the scene if necessary
	if (nglCurScene->ClearFlags & (NGLCLEAR_COLOR|NGLCLEAR_Z))
	{
		_nglClearImmediate(nglCurScene->ClearColor, nglCurScene->ClearFlags);
	}
	// Set the projection matrix.
	// Have to use perspective transformation for the Z component of the QuadProjection Matrix.

	if (nglCurScene->ViewType == NGL_VIEW_PERSPECTIVE)
	{
		#ifndef PROJECT_KELLYSLATER
		// FIXME: all this is semi magic numbers
		Mtx44 QuadMtx;
		MTXOrtho(QuadMtx,SceneViewY1, SceneViewY2, SceneViewX1, SceneViewX2, 0.0f, 1.0000001f);
		GXSetProjection(QuadMtx, GX_ORTHOGRAPHIC);
		GXGetProjectionv(DefaultQuadProjectionForScene);
		float vfov = nglCalcVFov(nglCurScene->HFOV, nglCurScene->Aspect);
		Mtx44 p;
		MTXPerspective(p, vfov, nglCurScene->Aspect, nglCurScene->NearZ, nglCurScene->FarZ);
		GXSetProjection(p, GX_PERSPECTIVE);
		#else
		Mtx44 QuadMtx;
		MTXOrtho(QuadMtx,SceneViewY1,nglCurScene->ViewY2,
			SceneViewX1, nglCurScene->ViewX2,
			//nglCurScene->NearZ, nglCurScene->FarZ );
			0.0f, 1.0000001f );
		GXSetProjection(QuadMtx,GX_ORTHOGRAPHIC);
		GXGetProjectionv(DefaultQuadProjectionForScene);
#ifdef PROJECT_SPIDERMAN
		float hfovDeg=Scene->HFOV;
#else
		float hfovDeg=2.0f*NGL_RADTODEG(atan(tan(NGL_DEGTORAD(Scene->HFOV/2.0f))*(nglCurScene->Aspect/NGL_REF_ASPECT_RATIO)));
#endif
		float vfov = 2.0f*NGL_RADTODEG(atan(tan(NGL_DEGTORAD(hfovDeg/2.0f))/nglCurScene->Aspect));
		Mtx44 p;
		float ViewPortARAdjust = ( nglRenderModeObj.efbHeight * SceneWidth ) / ( SceneHeight * nglRenderModeObj.fbWidth );
		MTXPerspective( p, vfov,
			nglCurScene->Aspect * ViewPortARAdjust,
			nglCurScene->NearZ, nglCurScene->FarZ );
		GXSetProjection( p, GX_PERSPECTIVE );
		#endif
	}
	else
	{
		#if 1 //ndef PROJECT_KELLYSLATER
		SceneViewX1 = nglCurScene->ViewX1;
		SceneViewY1 = nglCurScene->ViewY1;
		SceneViewX2 = nglCurScene->ViewX2;
		SceneViewY2 = nglCurScene->ViewY2;
		// the viewport is always sent as a rectangle, so we need to nudge it to width and height
		SceneWidth  = (nglCurScene->ViewX2 - SceneViewX1) + 1.0f;
		SceneHeight = (nglCurScene->ViewY2 - SceneViewY1) + 1.0f;
		if ( nglCurScene->RenderTarget)
		{
		  nglTexture* Target = nglCurScene->RenderTarget;

			// check and fixup the width
			if (SceneWidth > Target->Width)
			{
				float WidthDiff = SceneWidth - Target->Width;
				SceneWidth -= WidthDiff;
				SceneViewX2 -= WidthDiff;
			}
			// check and fixup the height
			if (SceneHeight > Target->Height)
			{
				float HeightDiff = SceneHeight - Target->Height;
				SceneHeight -= HeightDiff;
				SceneViewY2 -= HeightDiff;
			}
		}
		// FIXME: all this is semi magic numbers
		Mtx44 QuadMtx;
		MTXOrtho(QuadMtx, SceneViewY1, SceneViewY2, SceneViewX1, SceneViewX2, nglCurScene->NearZ, nglCurScene->FarZ);
		GXSetProjection(QuadMtx,GX_ORTHOGRAPHIC);
		GXGetProjectionv(DefaultQuadProjectionForScene);

		// Note that top and bottom are reversed to match the PS2.
		Mtx44 p;
		MTXOrtho(p, nglCurScene->Top, nglCurScene->Bottom, nglCurScene->Left, nglCurScene->Right, nglCurScene->NearZ, nglCurScene->FarZ);
		GXSetProjection(p, GX_ORTHOGRAPHIC);
		#else
		Mtx44 QuadMtx;
		MTXOrtho(QuadMtx,SceneViewY1,nglCurScene->ViewY2,
			SceneViewX1, nglCurScene->ViewX2,
			nglCurScene->NearZ, nglCurScene->FarZ );
		GXSetProjection(QuadMtx,GX_ORTHOGRAPHIC);
		GXGetProjectionv(DefaultQuadProjectionForScene);
	  Mtx44 p;
		float cx, cy;
		int Height, Width;
	  if( nglCurScene->RenderTarget )
	  {
	  	Height = nglCurScene->RenderTarget->Height;
	  	Width = nglCurScene->RenderTarget->Width;
	  }
	  else
	  {
	  	Height = SceneHeight;
	  	Width = SceneWidth;
	  }
	  // <<<< 09-Apr-2002 SAL: Fix this later to match on all platforms.
		cx = (nglCurScene->CX - SceneViewX1) - Width / 2;
		cx /= Width / 2;
		cy = (nglCurScene->CY - SceneViewY1) - Height / 2;
		cy /= Height / 2;
	  if( nglCurScene->RenderTarget )
	  {
	  	// <<<< 08-Jan-2002 SAL: Don't know why this should be so, but doing it this way for now.
	  	// Assuming that ViewX1 ... Y2 are set.
	  	//float ViewWidth = nglCurScene->ViewX2 - SceneViewX1;
	  	//float ViewHeight = nglCurScene->ViewY2 - SceneViewY1;
	  	float HeightAdjust = SceneHeight / Height;
	  	float WidthAdjust = SceneWidth / Width;
	  	float NearZAdjust = -1000; // <<<< 10-Jan-2002 SAL: Temp fix for some bug with the NearZ setting.
			MTXOrtho( p, cy+HeightAdjust, cy-HeightAdjust, cx-WidthAdjust, cx+WidthAdjust, nglCurScene->NearZ + NearZAdjust, nglCurScene->FarZ ); // Note that top and bottom are reversed to match the PS2.
		}
		else
		{
			MTXOrtho( p, cy+1.0f, cy-1.0f, cx-1.0f, cx+1.0f, nglCurScene->NearZ, nglCurScene->FarZ ); // Note that top and bottom are reversed to match the PS2.
		}
		GXSetProjection( p, GX_ORTHOGRAPHIC );
		#endif
	}
	GXGetProjectionv(DefaultProjectionForScene);

 	// Set the fog.
	if (nglCurScene->FogEnabled)
  		GXSetFog(nglFogTypes[nglCurScene->FogType],	nglCurScene->FogNear, nglCurScene->FogFar, nglCurScene->NearZ, nglCurScene->FarZ, nglCurScene->FogColor);
	else
		GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, _nglGXColorZero);

	// Compute some global matrices.
	nglMatrix z;
	nglIdentityMatrix( z );
	z[2][2] = -1.0f;
	nglMulMatrix( nglGCmz, nglCurScene->WorldToView, z );
	nglGCCameraPosInWorld[0] = nglCurScene->ViewToWorld[3][0];
	nglGCCameraPosInWorld[1] = nglCurScene->ViewToWorld[3][1];
	nglGCCameraPosInWorld[2] = nglCurScene->ViewToWorld[3][2];

  	SceneGXSetZMode(/*test*/ true, /*write*/ true);

  	nglListNode* Node = nglGetNextNode( (nglListNode*) &Scene->RenderListBins[0] );

  	// Render the meshes.
  	while (Node)
	{
		if ((nglNodeCount >= nglTestNodeStart) && (nglNodeCount <= nglTestNodeEnd))
			Node->NodeFn( Node->NodeData );
		nglNodeCount++;
		Node = nglGetNextNode( Node );
  	}

  	if (nglCurScene->RenderTarget)
  	{
		if (nglDebug.RenderTargetShot)
		{
			nglDebug.RenderTargetShot = 2;
			nglScreenShot();
		}

  		nglTexture* TargetTex = nglCurScene->RenderTarget;
		GXSetTexCopySrc(0, 0, TargetTex->Width, TargetTex->Height);
		GXSetTexCopyDst(TargetTex->Width, TargetTex->Height, (GXTexFmt)TargetTex->TexelFormat, GX_FALSE);

		GXSetZMode(GX_FALSE, GX_NEVER, GX_ENABLE);
		GXPixModeSync();
		GXCopyTex (TargetTex->ImageData, GX_FALSE);
		GXPixModeSync();
		GXInvalidateTexAll(); // <<<< 30-Jan-2002 SAL: Invalidating the tex region is all that is necessary.
	}
}

//-----------------------------------------------------------------------------
// @Send - Build and send the render DMA chain.
// this is the non threaded version
void nglListSend( bool Flip )
{

	// make sure the cache is stored before we render
	PPCSync();

	nglNodeCount = 0;

#if NGL_PROFILING
  	nglPerfInfo.TotalPolys = 0;
  	nglPerfInfo.TotalVerts = 0;

  	nglPerfInfo.ListSendCycles = OSGetTick();
  	nglPerfInfo.RenderCycles = OSGetTick();
#endif // NGL_PROFILING

	nglRenderScene(&nglDefaultScene);

#if NGL_PROFILING
  	nglPerfInfo.ListSendCycles = OSGetTick() - nglPerfInfo.ListSendCycles;
  	nglPerfInfo.ListSendMS = (float)nglPerfInfo.ListSendCycles / NGL_CPU_CLOCKS_PER_MS;
#endif // NGL_PROFILING

	// render the perflib graph if appropriate
#if defined(NGL_GC_PERFLIB)
  	if (DEBUG_ENABLE(ShowPerfGraph))
  	{
    	if (nglPERFDLRenderSize>0)
    	{
      		PERFPreDraw();
      		GXCallDisplayList(nglPERFDL, nglPERFDLRenderSize);
      		PERFPostDraw();
    	}
  	}
#endif

	if (Flip)
	{
		nglFlip();
	}

#if 0
#ifdef ARCH_ENGINE
  	if ( DEBUG_ENABLE( DumpSceneFile ) )
  	{
    	host_fprintf( nglSceneFile, "\n" );
    	host_fprintf( nglSceneFile, "ENDSCENE\n" );
    	host_fclose( nglSceneFile );
  	}
#endif
#endif
}

nglScene* nglListBeginScene()
{
  	nglScene* Scene;
  	if ( nglCurScene )
  	{
    	// Add the new scene at the end of the current scene's child list.
    	Scene = nglListNew( nglScene );

		if( !Scene )
		{
			nglPrintf( "nglListBeginScene: could not allocate nglScene.\n" );
			return (nglScene *)0;
		}

    	if ( nglCurScene->LastChild )
      		nglCurScene->LastChild->NextSibling = Scene;
    	else
      		nglCurScene->FirstChild = Scene;
    	nglCurScene->LastChild = Scene;

    	// Set up the scene based on nglDefaultScene (whose parameters can be set outside the render loop).
    	memcpy( Scene, &nglDefaultScene, sizeof(nglScene) );
  	}
  	else
    	Scene = &nglDefaultScene;

	// Initialize some parameters.
  	Scene->NextSibling = NULL;
  	Scene->FirstChild = NULL;
  	Scene->LastChild = NULL;

  	// Initialize the new scene's hierarchy linkage.
  	Scene->Parent = nglCurScene;

  	// Initialize the new scene's bin structure.
  	for ( u_int i = 0; i < NGL_TOTAL_BINS - 1; i++ )
  	{
    	Scene->RenderListBins[i].Next = (nglListNode*)&Scene->RenderListBins[i + 1];
    	Scene->RenderListBins[i].Type = NGLNODE_BIN;
  	}
  	Scene->RenderListBins[NGL_TOTAL_BINS - 1].Next = 0;
  	Scene->RenderListBins[NGL_TOTAL_BINS - 1].Type = NGLNODE_BIN;

  	nglCurScene = Scene;

  	return Scene;
}

void nglListEndScene()
{
  	if (nglCurScene == &nglDefaultScene)
  	{
		nglPrintf( "nglListEndScene: list scene underflow.\n" );
  		return;
  	}

  	nglCurScene = nglCurScene->Parent;
}

nglScene* nglListSelectScene(nglScene *scene)
{
  	nglScene *prev = nglCurScene;
  	nglCurScene = scene;
  	return prev;
}

void nglListInit()
{
#if NGL_PROFILING
  	// Check to see if we added too much to the render list.
  	nglPerfInfo.ListWorkBytesUsed = nglListWorkPos - nglListWork;
  	if ( nglPerfInfo.ListWorkBytesUsed > nglPerfInfo.MaxListWorkBytesUsed )
    	nglPerfInfo.MaxListWorkBytesUsed = nglPerfInfo.ListWorkBytesUsed;
#endif // NGL_PROFILING

	GXSetCurrentGXThread();

	nglListWorkPos = nglListWork;

  	// Reset the light lists.
  	nglDefaultLightContext = nglCreateLightContext();

	nglLightListHead.Next = &nglLightListHead;
	nglProjectorLightListHead.Next = &nglProjectorLightListHead;

  nglDefaultScene.AnimTime = nglVBlankCount * ( 1.0f / 60.0f );

	// unimplemented
// 	if ( nglSyncDebug.DumpFrameLog )
//    	nglDebug.DumpFrameLog = 0;

// 	if ( nglSyncDebug.DumpSceneFile )
//    	nglDebug.DumpSceneFile = 0;

  	nglSyncDebug = nglDebug;

  	// Set up the initial scene.
  	nglCurScene = NULL;
  	nglListBeginScene();

#if NGL_PROFILING
	// Show performance info from the previous render list.
  	if (DEBUG_ENABLE(ShowPerfInfo))
  	{
    	nglQuad q;
    	nglInitQuad( &q );
    	nglSetQuadRect( &q, 350, 20, 640, 160 );
		nglSetQuadColor(&q, NGL_RGBA32(0x00, 0x00, 0x00, 0xE0));
    	nglSetQuadZ( &q, 0.0f );
    	nglListAddQuad( &q );

    	nglListAddString(nglSysFont, 360, 30, 0,
      		"\1[802020FF]\2[1.1]NGL %s\2[1]\1[FFFFFFFF]\n"
			"%.2f FPS\n"
      		"%.2fms RENDER\n"
      		"%.2fms FIFO BUILD\n"
      		"%d VERTS\n"
      		"%d POLYS\n"
    	  	"%d NODES\n"
    	  	"LIST  %07d %07d\n",
			NGL_VERSION_STR,
			nglPerfInfo.FPS,
      		nglPerfInfo.RenderMS,
      		nglPerfInfo.ListSendMS,
      		nglPerfInfo.TotalVerts,
      		nglPerfInfo.TotalPolys,
			nglNodeCount,
    	  	nglPerfInfo.ListWorkBytesUsed, nglPerfInfo.MaxListWorkBytesUsed);
   	}
#endif // NGL_PROFILING

	// Normal geometary data path
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGB, GX_RGBA8, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 9 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_S16, 9 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX2, GX_TEX_ST, GX_S16, 9 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX3, GX_TEX_ST, GX_S16, 9 );
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, GX_F32, 0 );

	// Vertex format 1 is used for nglQuads.
	GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0 ); // <<<< Probably not used, but sent anyways?!?! Check later.
	GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 );

	GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GXSetTexCoordGen( GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
	GXSetTexCoordGen( GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY);
	GXSetTexCoordGen( GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY);

	nglMatrix ScaleMat = nglUnitMatrix;
	ScaleMat[0][0] = NGLGC_UNSCALE_UV;
	ScaleMat[1][1] = NGLGC_UNSCALE_UV;
	GXLoadTexMtxImm( (float (*)[4])&ScaleMat, nglTexMtx[NGLTEXMTX_DEFAULT_SCALE], GX_MTX2x4 );

#if 0
#ifdef ARCH_ENGINE
  	// Start the scene dump if requested.
  	if ( DEBUG_ENABLE( DumpSceneFile ) )
  	{
    	nglSceneFile = host_fopen( "scenedump.scene", HOST_WRITE );
    	host_fprintf( nglSceneFile, "/* */\n" );
    	host_fprintf( nglSceneFile, "/* Midnight scene file dump. */\n" );
    	host_fprintf( nglSceneFile, "/* */\n" );
    	host_fprintf( nglSceneFile, "\n" );
    	host_fprintf( nglSceneFile, "CAMERA\n" );
    	host_fprintf( nglSceneFile, "  ROW1 %f %f %f %f\n", nglCurScene->WorldToView[0][0], nglCurScene->WorldToView[0][1], nglCurScene->WorldToView[0][2], nglCurScene->WorldToView[0][3] );
    	host_fprintf( nglSceneFile, "  ROW2 %f %f %f %f\n", nglCurScene->WorldToView[1][0], nglCurScene->WorldToView[1][1], nglCurScene->WorldToView[1][2], nglCurScene->WorldToView[1][3] );
    	host_fprintf( nglSceneFile, "  ROW3 %f %f %f %f\n", nglCurScene->WorldToView[2][0], nglCurScene->WorldToView[2][1], nglCurScene->WorldToView[2][2], nglCurScene->WorldToView[2][3] );
    	host_fprintf( nglSceneFile, "  ROW4 %f %f %f %f\n", nglCurScene->WorldToView[3][0], nglCurScene->WorldToView[3][1], nglCurScene->WorldToView[3][2], nglCurScene->WorldToView[3][3] );
    	host_fprintf( nglSceneFile, "ENDCAMERA\n" );
    	host_fprintf( nglSceneFile, "\n" );
  	}
#endif
#endif
}

//----------------------------------------------------------------------------------------
//  @Texture manager code.
//----------------------------------------------------------------------------------------
nglTexture* nglLoadTextureA( const char* FileName )
{
  	nglFixedString p( FileName );
  	return nglLoadTexture( p );
}

bool nglLoadTextureIFL( nglTexture* Tex, unsigned char *Buf, u_int BufSize );
bool nglLoadTextureGCT( nglTexture* Tex, unsigned char* Buf, u_int BufSize );
//bool nglLoadTextureATE( nglTexture* Tex, unsigned char *Buf, const nglFixedString &FileName );

static inline bool IsPow2(u_int x) { return (x & (x-1))==0; }

static void nglProcessLoadedTexture(nglTexture *Tex)
{
	if ( Tex->Type==NGLTEX_GCT )
	{
		bool Width_IsPow2 = IsPow2(Tex->Width);
		bool Height_IsPow2 = IsPow2(Tex->Height);
		if (Tex->Mipmaps && (!Width_IsPow2 || !Height_IsPow2))
		{
			nglPrintf( "Mipmapped texture with non power of 2 dimensions\n" );
			Tex->Mipmaps = 0;
		}
		if (!Width_IsPow2) Tex->Flags |= NGLGC_TEXFLAG_UCLAMP;
		if (!Height_IsPow2) Tex->Flags |= NGLGC_TEXFLAG_VCLAMP;

		GXTexWrapMode WrapS = (Tex->Flags & NGLGC_TEXFLAG_UCLAMP) ? GX_CLAMP: GX_REPEAT;
		GXTexWrapMode WrapT = (Tex->Flags & NGLGC_TEXFLAG_VCLAMP) ? GX_CLAMP: GX_REPEAT;

		// Build hash for bin sorting.
		Tex->Hash = ( (u_int*)&Tex->FileName )[0];
		Tex->Hash += ( (u_int*)&Tex->FileName )[1];
		Tex->Hash += ( (u_int*)&Tex->FileName )[2];
		Tex->Hash += ( (u_int*)&Tex->FileName )[3];
		Tex->Hash += ( (u_int*)&Tex->FileName )[4];
		Tex->Hash += ( (u_int*)&Tex->FileName )[5];
		Tex->Hash += ( (u_int*)&Tex->FileName )[6];
		Tex->Hash += ( (u_int*)&Tex->FileName )[7];

		Tex->Hash = ~0 - Tex->Hash;

		if( Tex->PaletteSize )
		{
			GXInitTlutObj( &Tex->TlutObj,
										 Tex->PaletteData,
										 (GXTlutFmt) Tex->PaletteFormat,
										 Tex->PaletteSize );
			GXInitTexObjCI( &Tex->TexObj,
											Tex->ImageData,
											Tex->Width,
											Tex->Height,
											(GXCITexFmt) Tex->TexelFormat,
											WrapS,
											WrapT,
											Tex->Mipmaps,
											0 );
		}
		else
		{
			// Setup GC specific fun.
			GXInitTexObj( &Tex->TexObj,
										Tex->ImageData,
										Tex->Width,
										Tex->Height,
										(GXTexFmt) Tex->TexelFormat,
										WrapS,
										WrapT,
										Tex->Mipmaps );
		}

		nglTextureBank.Insert( Tex->FileName, Tex );
	}
	else if ( Tex->Type==NGLTEX_IFL )
	{
		nglTextureBank.Insert( Tex->FileName, Tex );
	}
	else if ( Tex->Type==NGLTEX_ATE )
	{
		if ( Tex->NFrames )
			for ( int i=0; i<Tex->NFrames; i++ )
			{
				nglProcessLoadedTexture(Tex->Frames[i]);
			}
	}
}

nglTexture* nglLoadTextureInPlace( const nglFixedString& FileName, u_int Type, void* Data, u_int Size )
{
  bool Result = false;

  nglTexture* Tex = (nglTexture*)nglMemAlloc( sizeof(nglTexture) );
  memset( Tex, 0, sizeof(nglTexture) );

	if ( Type==NGLTEX_GCT )
	{
		Result = nglLoadTextureGCT( Tex, (unsigned char *) Data, Size );
	}
  else if ( Type==NGLTEX_IFL )
  {
    Result = nglLoadTextureIFL( Tex, (unsigned char *) Data, Size );
  }
/*
  else if ( Type==NGLTEX_ATE )
  {
    Result = nglLoadTextureATE( Tex, (unsigned char *) Data, FileName  );
  }
*/

  if ( !Result )
  {
    nglPrintf( "nglLoadTextureFromMem: Unable to open %s%s.\n", nglTexturePath, FileName.c_str() );
    nglMemFree( Tex );
    return 0;
  }

  Tex->FileName = FileName;

  nglProcessLoadedTexture(Tex);

  return Tex;
}

nglTexture* nglLoadTextureFromMem( const nglFixedString& FileName, int Type, nglFileBuf &File )
{
	return nglLoadTextureInPlace(FileName,Type,File.Buf,File.Size);
}

nglTexture* nglLoadTexture( const nglFixedString& FileName )
{
  bool Result = false;

  // Search for an existing copy.
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( FileName ) ) )
  {
    Inst->RefCount++;
    return (nglTexture*)Inst->Value;
  }

  nglTexture* Tex = (nglTexture*)nglMemAlloc( sizeof(nglTexture) );

  if( !Tex )
  {
  	return NULL;
  }

  memset( Tex, 0, sizeof(nglTexture) );

  static char NameExt[1024];
  nglFileBuf File;

  strcpy( nglWork, nglTexturePath );
  strcat( nglWork, FileName.c_str() );

	strcpy( NameExt, nglWork );
	strcat( NameExt, ".gct" );
	if( nglReadFile( NameExt, &File, 128 ) )
	{
		Result = nglLoadTextureGCT( Tex, File.Buf, File.Size );

		if( !Result ) {
			nglReleaseFile( &File );
		}

	}
	else
	{
		strcpy( NameExt, nglWork );
		strcat( NameExt, ".ifl" );
		if ( nglReadFile( NameExt, &File, 128 ) )
		{
		  Result = nglLoadTextureIFL( Tex, File.Buf, File.Size );
	      nglReleaseFile( &File );
		}
	}

  if ( !Result )
  {
    nglPrintf( "nglLoadTexture: Unable to open %s%s.\n", nglTexturePath, FileName.c_str() );
    nglMemFree( Tex );
    return 0;
  }

  Tex->FileName = FileName;

  nglProcessLoadedTexture(Tex);

  return Tex;
}

// Add a reference to a texture in the instance bank.
void nglAddTextureRef( nglTexture* Tex )
{
  // Search for an existing copy.
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( Tex->FileName ) ) )
    Inst->RefCount++;
}

nglTexture* nglGetTextureA( const char* FileName )
{
  nglFixedString p( FileName );
  return nglGetTexture( p );
}

nglTexture* nglGetTexture( const nglFixedString& FileName )
{
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( FileName ) ) )
    return (nglTexture*)Inst->Value;
  return 0;
}

void nglReleaseAllTextures()
{
  nglUnlockAllTextures();

  nglInstanceBank::Instance* Inst = nglTextureBank.Head->Forward[0];
  while ( Inst != nglTextureBank.NIL )
  {
		if ( ( (nglTexture*)Inst->Value )->Flags & NGLGC_TEXFLAG_SYSTEM )
		  Inst = Inst->Forward[0];
		else
		{
		  Inst->RefCount = 1;
		  nglReleaseTexture( (nglTexture*)Inst->Value );
		  Inst = nglTextureBank.Head->Forward[0];
		}

  }

  GXInvalidateTexAll( );
}

void nglReleaseTexture( nglTexture* Tex )
{
	if( Tex->Flags & NGLGC_TEXFLAG_SYSTEM )
		return;

	if(Tex->FileName == nglFixedString("nglGCCust"))
	{
		// These textures will have to be destroyed by nglDestroyTexture.
		return;
	}
  else if ( nglTextureBank.Delete( Tex->FileName ) != 0 )
    return; // Remove a reference from the instance bank, delete the texture only if the count hits 0.

  if ( Tex->Type == NGLTEX_GCT )
  {
  	nglFileBuf File;
  	File.Buf = (u_char*) Tex->Data;
  	nglReleaseFile( &File );
  }
  else
  if ( Tex->Type == NGLTEX_IFL )
  {
    for ( u_int i = 0; i < Tex->NFrames; i++ )
      nglReleaseTexture( Tex->Frames[i] );
    nglMemFree( Tex->Frames );
  }
  else if ( Tex->Type==NGLTEX_ATE )
  {
    for ( u_int i = 0; i < Tex->NFrames; i++ )
      nglReleaseTexture( Tex->Frames[i] );
    nglMemFree( Tex->Frames );
  }
	nglMemFree(Tex);
}

void nglDestroyTexture( nglTexture* Tex )
{
	if(Tex)
	{
		nglMemFree(Tex->Data);
		nglMemFree(Tex);
		Tex=NULL;
	}
}

nglTexture* nglLoadTextureLock( const char* FileName )
{
  return nglLoadTextureA( FileName );
}

void nglRelockAllTextures( void )
{
	// empty on GC
}

void nglUnlockTexture( nglTexture* Tex )
{
	// empty on GC
}

void nglUnlockAllTextures( void )
{
	// empty on GC
}

//----------------------------------------------------------------------------------------
//  @Quad (2d interface) code.
//----------------------------------------------------------------------------------------

void nglListAddQuad(nglQuad* Quad)
{
	NGL_ASSERT( nglCurScene, "" );
	nglQuad* NewQuad = nglListNew( nglQuad );

	if (!NewQuad)
	{
		nglPrintf( "nglListAddQuad: could not allocate new quad.\n" );
    	return;
  	}

  	memcpy(NewQuad, Quad, sizeof(nglQuad));

	// Add quad as a node.
	nglSortInfo Info;
	Info.Type = NGLSORT_TRANSLUCENT;
	Info.Dist = NewQuad->Z;

	nglListAddNode(NGLNODE_QUAD, nglRenderQuadCallback, NewQuad, &Info);

	// Dont sync because we won't render this for a bit yet.
  	DCStoreRangeNoSync(NewQuad, sizeof(nglQuad));
}

//-----------------------------------------------------------------------------

void nglInitQuad(nglQuad* Quad)
{
  	memset(Quad, 0, sizeof(nglQuad));

  	Quad->Verts[0].Color = NGL_RGBA32(255, 255, 255, 255);
  	Quad->Verts[1].Color = NGL_RGBA32(255, 255, 255, 255);
  	Quad->Verts[2].Color = NGL_RGBA32(255, 255, 255, 255);
  	Quad->Verts[3].Color = NGL_RGBA32(255, 255, 255, 255);

  	Quad->Verts[0].U = 0.0f;
  	Quad->Verts[1].U = 1.0f;
  	Quad->Verts[2].U = 0.0f;
  	Quad->Verts[3].U = 1.0f;

  	Quad->Verts[0].V = 0.0f;
  	Quad->Verts[1].V = 0.0f;
  	Quad->Verts[2].V = 1.0f;
  	Quad->Verts[3].V = 1.0f;

  	Quad->MapFlags = NGLMAP_BILINEAR_FILTER | NGLMAP_CLAMP_U | NGLMAP_CLAMP_V;
  	Quad->BlendMode = NGLBM_BLEND;

	// FIXME: might need to move this to render-time
//  	Quad->Z = nglCurScene->NearZ + 0.000001f; // Allow for small transformation errors.
//  	if (Quad->Z > nglCurScene->FarZ)
//  		Quad->Z = nglCurScene->FarZ;
}

//-----------------------------------------------------------------------------

void nglSetQuadMapFlags( nglQuad* Quad, u_int MapFlags )
{
  	Quad->MapFlags = MapFlags;
}

void nglSetQuadTex( nglQuad* Quad, nglTexture* Tex )
{
  	Quad->Tex = Tex;
}

void nglSetQuadBlend( nglQuad* Quad, u_int Blend, u_int Constant )
{
  	Quad->BlendMode = Blend;
	Quad->BlendModeConstant = ((float)Constant) / 100.0f;
}

//-----------------------------------------------------------------------------

void nglSetQuadUV( nglQuad* Quad, float u1, float v1, float u2, float v2 )
{
  	Quad->Verts[0].U = u1;
  	Quad->Verts[0].V = v1;
  	Quad->Verts[1].U = u2;
  	Quad->Verts[1].V = v1;
  	Quad->Verts[2].U = u1;
  	Quad->Verts[2].V = v2;
  	Quad->Verts[3].U = u2;
  	Quad->Verts[3].V = v2;
}

//-----------------------------------------------------------------------------

void nglSetQuadColor( nglQuad* Quad, u_int Color )
{
  	Quad->Verts[0].Color = Color;
  	Quad->Verts[1].Color = Color;
  	Quad->Verts[2].Color = Color;
  	Quad->Verts[3].Color = Color;
}

//-----------------------------------------------------------------------------

void nglSetQuadRect( nglQuad* Quad, float x1, float y1, float x2, float y2 )
{
  	Quad->Verts[0].X = x1;
  	Quad->Verts[0].Y = y1;
  	Quad->Verts[1].X = x2;
  	Quad->Verts[1].Y = y1;
  	Quad->Verts[2].X = x1;
  	Quad->Verts[2].Y = y2;
  	Quad->Verts[3].X = x2;
  	Quad->Verts[3].Y = y2;
}

//-----------------------------------------------------------------------------

void nglSetQuadPos( nglQuad* Quad, float x, float y )
{
  float w, h;

  if (Quad->Tex)
  {
    	w = Quad->Tex->Width;
    	h = Quad->Tex->Height;
  }
  else
    	w = h = 50;

  nglSetQuadRect(Quad, x, y, x + w, y + h);
}

//-----------------------------------------------------------------------------

void nglSetQuadZ( nglQuad* Quad, float z )
{
  	Quad->Z = z;
}

//-----------------------------------------------------------------------------

void nglRotateQuad( nglQuad* Quad, float cx, float cy, float theta )
{
  	for ( int i = 0; i < 4; i++ )
  	{
    	nglQuadVertex* Vert = &Quad->Verts[i];
    	float x = Vert->X - cx;
    	float y = Vert->Y - cy;
    	Vert->X = x * cosf( theta ) - y * sinf( theta ) + cx;
    	Vert->Y = y * cosf( theta ) + x * sinf( theta ) + cy;
  	}
}

//-----------------------------------------------------------------------------

void nglScaleQuad( nglQuad* Quad, float cx, float cy, float sx, float sy )
{
  	for ( int i = 0; i < 4; i++ )
  	{
    	nglQuadVertex* Vert = &Quad->Verts[i];
    	float x = Vert->X - cx;
    	float y = Vert->Y - cy;
    	Vert->X = x * sx + cx;
    	Vert->Y = y * sy + cy;
  	}
}

//-----------------------------------------------------------------------------

// New vertex API.  Verts go in this order: Top Left, Top Right, Bottom Left, Bottom Right.
void nglSetQuadVPos( nglQuad* Quad, int VertIdx, float x, float y )
{
  	Quad->Verts[VertIdx].X = x;
  	Quad->Verts[VertIdx].Y = y;
}

//-----------------------------------------------------------------------------

void nglSetQuadVUV( nglQuad* Quad, int VertIdx, float u, float v )
{
  	Quad->Verts[VertIdx].U = u;
  	Quad->Verts[VertIdx].V = v;
}

//-----------------------------------------------------------------------------

void nglSetQuadVColor( nglQuad* Quad, int VertIdx, u_int Color )
{
  	Quad->Verts[VertIdx].Color = Color;
}

//-----------------------------------------------------------------------------

static void nglRenderQuadCallback( void* Data )
{
	nglQuad* Quad = (nglQuad*)Data;

	if (!Quad)
	{
		nglWarning("nglSendQuad: Skipping NULL Quad\n");
		return;
	}

	// setup the texture to use
	nglTexture* Tex = Quad->Tex;
	if (Tex)
	{
		// set the filter mode based on map flags
		nglSetFilterMode(Quad->MapFlags, Quad->Tex);
		nglSetClampMode(Quad->MapFlags, Quad->Tex);

		if (Tex->PaletteSize)
		{
			GXLoadTlut(&Tex->TlutObj, 0);
			GXInitTexObjTlut(&Tex->TexObj, 0);
		}

		GXLoadTexObj(&Tex->TexObj, GX_TEXMAP0);
	}
	else
	{
		nglSetFilterMode(Quad->MapFlags, &nglWhiteTex);
		nglSetClampMode(Quad->MapFlags, &nglWhiteTex);
		GXLoadTexObj(&nglWhiteTex.TexObj, GX_TEXMAP0);
	}

	GXSetCullMode(GX_CULL_NONE);

	// reset our camera matrix to the identity
	GXLoadPosMtxImm(_nglMTXIdentityInvZ, GX_PNMTX0);
	// Setup the orthographic projection.
	GXSetProjectionv(DefaultQuadProjectionForScene);
	// set our lighting to generate color0 using the vertex color
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	// Setup number of Tev stages etc.
	GXSetNumTexGens(1);
	GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	// set the number of tev stages
	GXSetNumTevStages(1);
	// set the blend modes
	nglSetupTevStage(Quad->BlendMode, Quad->BlendModeConstant, GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);
	GXSetTexCoordCylWrap(GX_TEXCOORD0, GX_FALSE, GX_FALSE);

	float QuadZ = Quad->Z;

	// this code is required for kelly slater and probably others ...
	// seems like a big hack though
	if (nglCurScene->ViewType == NGL_VIEW_PERSPECTIVE)
	{
		if (QuadZ != 0.0f)
		{
			// Due to rounding errors it could happen that something right on the NearZ plane may get clipped off.
			float NewZ = 1.0f + ((-QuadZ) * nglCurScene->QuadZCompParam[0] + nglCurScene->QuadZCompParam[1] ) / QuadZ;
			QuadZ = NewZ;
		}
	}

	// Setup the vertex format.
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	// render the quad
	GXBegin(GX_QUADS, GX_VTXFMT1, 4);
		// Vert 0 -- top left
		GXPosition3f32(Quad->Verts[0].X, Quad->Verts[0].Y, QuadZ);
		GXColor1u32(Quad->Verts[0].Color);
		GXTexCoord2f32(Quad->Verts[0].U, Quad->Verts[0].V);
		// Vert 1 -- top right
		GXPosition3f32(Quad->Verts[1].X, Quad->Verts[1].Y, QuadZ);
		GXColor1u32(Quad->Verts[1].Color);
		GXTexCoord2f32(Quad->Verts[1].U, Quad->Verts[1].V);
		// Vert 3 -- bottom right
		GXPosition3f32(Quad->Verts[3].X, Quad->Verts[3].Y, QuadZ);
		GXColor1u32(Quad->Verts[3].Color);
		GXTexCoord2f32(Quad->Verts[3].U, Quad->Verts[3].V);
		// Vert 2 -- bottom left
		GXPosition3f32(Quad->Verts[2].X, Quad->Verts[2].Y, QuadZ);
		GXColor1u32(Quad->Verts[2].Color);
		GXTexCoord2f32(Quad->Verts[2].U, Quad->Verts[2].V);
	GXEnd();

	GXSetCullMode(GX_CULL_BACK);

	GXSetProjectionv( DefaultProjectionForScene );
}

//----------------------------------------------------------------------------------------
//  @Scratch mesh code.
//----------------------------------------------------------------------------------------

void _nglAllocateMeshSection(nglMeshSection* Section, u_int verts, u_int strips)
{
  	// Set up the section with defaults.
  	Section->NVerts = verts;
	Section->NTris = strips;

	Section->Verts = nglListAlloc(sizeof(nglVertex) * verts);
	Section->VertMap = (short*)nglListAlloc(sizeof(short) * strips);

	// temporary clear these
	memset(Section->Verts, 0, (sizeof(nglVertex)*verts));
	memset(Section->VertMap, 0, (sizeof(short)*strips));
}

#define ALIGN(a, b) (((a)+((b)-1))&(~((b)-1)))

void nglCreateMesh(u_int MeshFlags, u_int NSections, u_int NBones/* = 0*/, nglMatrix* Bones/* = 0*/ )
{
  	// Set up the mesh.
  	u_int i;
  	nglMesh* Mesh = (nglMesh*)nglListAlloc(sizeof(nglMesh));
  	memset( Mesh, 0, sizeof(nglMesh) );
  	Mesh->Flags = MeshFlags | NGLMESH_SCRATCH;

  	Mesh->NSections = NSections;
  	Mesh->Sections = (nglMeshSection*)nglListAlloc(NSections * sizeof(nglMeshSection));
  	memset( Mesh->Sections, 0, NSections * sizeof(nglMeshSection) );

  	for ( i = 0; i < NSections; i++ )
	{
    	Mesh->Sections[i].Material = (nglMaterial*)nglListAlloc(sizeof(nglMaterial));
    	memset( Mesh->Sections[i].Material, 0, sizeof(nglMaterial) );
	}

  	Mesh->NBones = NBones;
	if (NBones)
	{
  		Mesh->Bones = (nglMatrix*)nglListAlloc(NBones * sizeof(nglMatrix), 16);
    	for (i=0; i<NBones; ++i)
		{
       		// unimplemented
			NGL_ASSERT(false, "");
#if 0
      		Mesh->Bones[i] = Bones[i];
        	*(transform_t*)Mesh->Bones[i] = ( *(transform_t*)Mesh->Bones[i] ).orthonormal_inverse();
#endif
    	}
	}

  	// FIXME: See if the create operation overflowed the scratch mesh buffer.

  	nglScratch = Mesh;
  	nglScratchSection = Mesh->Sections - 1;  // hack to prepare for 1st addsection call.
	nglScratchEdit = false;

  	nglVector Center( 0, 0, 0, 1 );
  	nglMeshSetSphere( Center, 1.0e32f );
}

nglMesh* nglCloseMesh()
{
	nglMesh* rval;

	if (nglScratchEdit)
    	rval = nglScratch;
	else
	{
  		nglScratch->DataSize = (u_int)nglListWorkPos - (u_int)nglScratch;
  		if ( nglScratch->Flags & NGLMESH_TEMP )
    		rval = nglScratch;
  		else
  		{
    		// Allocate space to copy the mesh.
    		nglMesh* Mesh = (nglMesh*)nglMemAlloc(nglScratch->DataSize, 128);
    		memcpy(Mesh, nglScratch, nglScratch->DataSize);
    		// Rebase all the pointers within the mesh to point to the new memory.
    		nglRebaseMesh((u_int)Mesh, (u_int)nglScratch, Mesh);
			// return our mesh
    		rval = Mesh;
  		}
	}

	// flush the mesh
	DCStoreRangeNoSync(rval, rval->DataSize);

  	nglScratchEdit = false;
  	nglScratch = NULL;
  	nglScratchSection = NULL;
	nglScratchStripPos = NULL;
  	nglScratchVertIdx = 0;

	return rval;
}

void nglDestroyMesh( nglMesh* Mesh )
{
  	nglMemFree( Mesh );
}

void nglEditMesh( nglMesh* Mesh )
{
	NGL_ASSERT(nglScratch == NULL, "Previously Edited Mesh Not Closed");

	nglScratchEdit = true;
  	nglScratch = (nglMesh*)Mesh;
  	nglScratchSection = nglScratch->Sections;
  	nglScratchStripPos = nglScratchSection->VertMap;
  	nglScratchVertIdx = 0;
}

void nglMeshSetSection( u_int Idx )
{
  	nglScratchSection = &nglScratch->Sections[Idx];
  	nglScratchStripPos = nglScratchSection->VertMap;
  	nglScratchVertIdx = 0;
}

void nglMeshSetVertex( u_int VertIdx )
{
  	// Unimplemented.
  	NGL_ASSERT( false, "unimplemented" );
}

void nglMeshGetVertex( nglScratchVertex* Vertex )
{
  	// Unimplemented.
  	NGL_ASSERT( false, "unimplemented" );
}

void nglMeshAddSection( nglMaterial* Material, u_int NVerts, u_int Strips )
{
  	nglScratchSection++;
  	if ( (u_int)( nglScratchSection - nglScratch->Sections ) >= nglScratch->NSections )
    	nglFatal( "Added too many sections to a scratch mesh.\n" );

  	if (Material)
  	{
    	// Copy the material and correct some common material mistakes.
    	memcpy( nglScratchSection->Material, Material, sizeof(nglMaterial) );

    	if (Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH)
      		nglScratchSection->Material->Flags |= NGLMAT_ALPHA;

    	if ( !(Material->Flags&NGL_TEXTURE_MASK) )
    	{
      		nglScratchSection->Material->Flags |= NGLMAT_TEXTURE_MAP;
      		nglScratchSection->Material->Map = &nglWhiteTex;
    	}

		///////////////////////////////////////////////////////
		// convert ArchMat material flags into NGL map flags //
		///////////////////////////////////////////////////////

		// set the default map flag
		Material->MapFlags = NGLMAP_POINT_FILTER;
		// check bilinear
		if (Material->Flags & NGLMAT_BILINEAR_FILTER)
			Material->MapFlags = NGLMAP_BILINEAR_FILTER;
		// trilinear overrides if set
		if (Material->Flags & NGLMAT_TRILINEAR_FILTER)
			Material->MapFlags = NGLMAP_TRILINEAR_FILTER;
		// test the clamp modes
		if (Material->Flags & NGLMAT_CLAMP_U)
			Material->MapFlags |= NGLMAP_CLAMP_U;
		if (Material->Flags & NGLMAT_CLAMP_V)
			Material->MapFlags |= NGLMAP_CLAMP_V;
  	}
  	else
  	{
    	nglScratchSection->Material->Flags = NGLMAT_TEXTURE_MAP;
    	nglScratchSection->Material->Map = &nglWhiteTex;
  	}

  	// find memory for the scratch mesh
  	_nglAllocateMeshSection(nglScratchSection, NVerts, Strips);

  	nglScratchStripPos = nglScratchSection->VertMap;
  	nglScratchVertIdx = 0;
}

// Copy a vertex from one batch to another.  Vert indices are batch relative.
void nglMeshCopyVertex( nglMeshBatchInfo* DestBatch, int DestIdx, nglMeshBatchInfo* SrcBatch, int SrcIdx )
{
  	// Position, normal, UV, color.
  	DestBatch->PosData[DestIdx * 3 + 0] = SrcBatch->PosData[SrcIdx * 3 + 0];
  	DestBatch->PosData[DestIdx * 3 + 1] = SrcBatch->PosData[SrcIdx * 3 + 1];
  	DestBatch->PosData[DestIdx * 3 + 2] = SrcBatch->PosData[SrcIdx * 3 + 2];

  	if ( DestBatch->NormData )
	{
    	DestBatch->NormData[DestIdx * 3 + 0] = SrcBatch->NormData[SrcIdx * 3 + 0];
    	DestBatch->NormData[DestIdx * 3 + 1] = SrcBatch->NormData[SrcIdx * 3 + 1];
    	DestBatch->NormData[DestIdx * 3 + 2] = SrcBatch->NormData[SrcIdx * 3 + 2];
  	}

  	if ( DestBatch->UVData )
  	{
    	DestBatch->UVData[DestIdx * 2 + 0] = SrcBatch->UVData[SrcIdx * 2 + 0];
    	DestBatch->UVData[DestIdx * 2 + 1] = SrcBatch->UVData[SrcIdx * 2 + 1];
	}

  	if ( DestBatch->ColorData )
    	DestBatch->ColorData[DestIdx] = SrcBatch->ColorData[SrcIdx];

  	if ( DestBatch->LightUVData )
	{
    	DestBatch->LightUVData[DestIdx * 2 + 0] = SrcBatch->LightUVData[SrcIdx * 2 + 0];
    	DestBatch->LightUVData[DestIdx * 2 + 1] = SrcBatch->LightUVData[SrcIdx * 2 + 1];
  	}
  	if ( DestBatch->BoneCountData )
  	{
    	DestBatch->BoneCountData[DestIdx] = SrcBatch->BoneCountData[SrcIdx];
    	DestBatch->BoneIdxData[DestIdx * 4 + 0] = SrcBatch->BoneIdxData[SrcIdx * 4 + 0];
    	DestBatch->BoneIdxData[DestIdx * 4 + 1] = SrcBatch->BoneIdxData[SrcIdx * 4 + 1];
    	DestBatch->BoneIdxData[DestIdx * 4 + 2] = SrcBatch->BoneIdxData[SrcIdx * 4 + 2];
    	DestBatch->BoneIdxData[DestIdx * 4 + 3] = SrcBatch->BoneIdxData[SrcIdx * 4 + 3];
    	DestBatch->BoneWeightData[DestIdx * 4 + 0] = SrcBatch->BoneWeightData[SrcIdx * 4 + 0];
    	DestBatch->BoneWeightData[DestIdx * 4 + 1] = SrcBatch->BoneWeightData[SrcIdx * 4 + 1];
    	DestBatch->BoneWeightData[DestIdx * 4 + 2] = SrcBatch->BoneWeightData[SrcIdx * 4 + 2];
    	DestBatch->BoneWeightData[DestIdx * 4 + 3] = SrcBatch->BoneWeightData[SrcIdx * 4 + 3];
  	}
}

void nglMeshWritePrimitive( u_int Length )
{
	// set the length of the strip
  	*nglScratchStripPos = Length;
  	nglScratchStripPos++;
}

void nglMeshWriteVertexPC( float X, float Y, float Z, u_int Color )
{
	nglMeshFastWriteVertexPC( X, Y, Z, Color );
}

void nglMeshWriteVertexPCUV( float X, float Y, float Z, u_int Color, float U, float V )
{
	nglMeshFastWriteVertexPCUV( X, Y, Z, Color, U, V );
}

void nglMeshWriteVertexPCUVB( float X, float Y, float Z, u_int Color, float U, float V,
                int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPCUVB( X, Y, Z, Color, U, V, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshWriteVertexPCUV2( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2 )
{
	nglMeshFastWriteVertexPCUV2( X, Y, Z, Color, U, V, U2, V2 );
}

void nglMeshWriteVertexPCUV2B( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2,
                 int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPCUV2B( X, Y, Z, Color, U, V, U2, V2, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshWriteVertexPUV( float X, float Y, float Z, float U, float V )
{
	nglMeshFastWriteVertexPUV( X, Y, Z, U, V );
}

void nglMeshWriteVertexPN( float X, float Y, float Z, float NX, float NY, float NZ )
{
	nglMeshFastWriteVertexPN( X, Y, Z, NX, NY, NZ );
}

void nglMeshWriteVertexPNC( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color )
{
	nglMeshFastWriteVertexPNC( X, Y, Z, NX, NY, NZ, Color );
}

void nglMeshWriteVertexPNCUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, float U, float V )
{
	nglMeshFastWriteVertexPNCUV( X, Y, Z, NX, NY, NZ, Color, U, V );
}

void nglMeshWriteVertexPNUV( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V )
{
	nglMeshFastWriteVertexPNUV( X, Y, Z, NX, NY, NZ, U, V );
}

void nglMeshWriteVertexPNUVB( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V,
                int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPNUVB( X, Y, Z, NX, NY, NZ, U, V, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshWriteVertexPNUV2( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2 )
{
	nglMeshFastWriteVertexPNUV2( X, Y, Z, NX, NY, NZ, U, V, U2, V2 );
}

void nglMeshWriteVertexPNUV2B( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2,
							   int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPNUV2B( X, Y, Z, NX, NY, NZ, U, V, U2, V2, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshSetSphere( nglVector& Center, float Radius )
{
  	nglCopyVector( nglScratch->SphereCenter, Center );
  	nglScratch->SphereRadius = Radius;

  	for ( u_int s = 0; s < nglScratch->NSections; ++s)
  	{
   		nglCopyVector( nglScratch->Sections[s].SphereCenter, Center );
   		nglScratch->Sections[s].SphereRadius = Radius;
  	}
}

void nglMeshCalcSphere( u_int NVerts /*ignored*/ )
{
	float MinX = 1.0e32f, MaxX = -1.0e32f;
	float MinY = 1.0e32f, MaxY = -1.0e32f;
  	float MinZ = 1.0e32f, MaxZ = -1.0e32f;

  	u_int s, i;

  	// Calculate the center.
  	for ( s = 0; s < nglScratch->NSections; ++s)
  	{
   		float SectionMinX = 1.0e32f, SectionMaxX = -1.0e32f;
   		float SectionMinY = 1.0e32f, SectionMaxY = -1.0e32f;
   		float SectionMinZ = 1.0e32f, SectionMaxZ = -1.0e32f;

		nglMeshSection* Section = &nglScratch->Sections[s];
		nglVertex* Verts = (nglVertex*) Section->Verts;
		short* NVerts = Section->VertMap;

		for (u_int strip=0;strip<Section->NTris;strip++)
		{
    		for (i=0; i<*NVerts; i++)
    		{
      			float X, Y, Z;
      			X = Verts->x;
      			if ( X < SectionMinX ) SectionMinX = X;
      			if ( X > SectionMaxX ) SectionMaxX = X;

      			Y = Verts->y;
      			if ( Y < SectionMinY ) SectionMinY = Y;
      			if ( Y > SectionMaxY ) SectionMaxY = Y;

      			Z = Verts->z;
      			if ( Z < SectionMinZ ) SectionMinZ = Z;
      			if ( Z > SectionMaxZ ) SectionMaxZ = Z;

      			Verts++;
    		}

    		NVerts++;
   		}

   		nglVector Center;
   		float Radius;

   		Center[0] = SectionMinX + ( SectionMaxX - SectionMinX ) * 0.5f;
   		Center[1] = SectionMinY + ( SectionMaxY - SectionMinY ) * 0.5f;
   		Center[2] = SectionMinZ + ( SectionMaxZ - SectionMinZ ) * 0.5f;
  		Center[3] = 1.0f;

   		// Calculate the radius from an arbitrary point on the box.
   		float DistX, DistY, DistZ;
   		DistX = SectionMinX - Center[0];
   		DistY = SectionMinY - Center[1];
   		DistZ = SectionMinZ - Center[2];
   		Radius = sqrtf( DistX * DistX + DistY * DistY + DistZ * DistZ );

   		nglCopyVector( nglScratch->Sections[s].SphereCenter, Center );
   		nglScratch->Sections[s].SphereRadius = Radius;

   		if ( SectionMinX < MinX ) MinX = SectionMinX;
   		if ( SectionMaxX > MaxX ) MaxX = SectionMaxX;

		if ( SectionMinY < MinY ) MinY = SectionMinY;
		if ( SectionMaxY > MaxY ) MaxY = SectionMaxY;

		if ( SectionMinZ < MinZ ) MinZ = SectionMinZ;
		if ( SectionMaxZ > MaxZ ) MaxZ = SectionMaxZ;
	}

  	nglVector Center;
  	float Radius;

  	Center[0] = MinX + ( MaxX - MinX ) * 0.5f;
  	Center[1] = MinY + ( MaxY - MinY ) * 0.5f;
  	Center[2] = MinZ + ( MaxZ - MinZ ) * 0.5f;
  	Center[3] = 1.0f;

  	// Calculate the radius from an arbitrary point on the box.
  	float DistX, DistY, DistZ;
  	DistX = MinX - Center[0];
  	DistY = MinY - Center[1];
  	DistZ = MinZ - Center[2];
  	Radius = sqrtf( DistX * DistX + DistY * DistY + DistZ * DistZ );

  	// Fill out the appropriate mesh entries.
  	nglCopyVector( nglScratch->SphereCenter, Center );
  	nglScratch->SphereRadius = Radius;
}

void nglScratchSetMaterial( nglMaterial* Material )
{
  // Copy the material and correct some common material mistakes.
  memcpy( nglScratchSection->Material, Material, sizeof(nglMaterial) );

  if ( Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH )
    nglScratchSection->Material->Flags |= NGLMAT_ALPHA;

  if ( !( Material->Flags & NGL_TEXTURE_MASK ) || !Material->Map )
  {
    nglScratchSection->Material->Flags |= NGLMAT_TEXTURE_MAP;
    nglScratchSection->Material->Map = &nglWhiteTex;
  }
}

//-----------------------------------------------------------------------------

void nglClearVRAM( u_char r, u_char g, u_char b, u_char a )
{
	GXColor color;

	color.r = 0xAA; //r;
	color.g = 0xBB; //g;
	color.b = 0xCC; //b;
	color.a = 0xDD; //a;

	nglFlip( );
}

//----------------------------------------------------------------------------------------
//  @GCT file code.
//----------------------------------------------------------------------------------------
#pragma pack(push,1)
struct gctHeaderV2
{
	char Tag[4];
	u32 Version;
	u32 Width;
	u32 Height;
	u8 Compressed;
	u8 Pad1;
	u8 Pad2;
	u8 Pad3;
	u32 Pad4;
	u32 Pad5;
	u32 Pad6;
};
#pragma pack(pop)

#define GCT_VERSION_V2 0x0002
#define GCT_VERSION_V3 0x0003
#define GCT_VERSION    0x0003

static bool nglLoadTextureGCTv2( nglTexture* Tex, unsigned char* Buf )
{
	gctHeaderV2* Header;

	Header = (gctHeaderV2*) Buf;

	Tex->Type = NGLTEX_GCT;

	Tex->Width = Header->Width;
	Tex->Height = Header->Height;

	Tex->Data = Buf;
	Tex->ImageData = Tex->Data + sizeof( gctHeaderV2 );
	Tex->PaletteSize = 0;
	Tex->PaletteData = NULL;
	Tex->Mipmaps = 0;
	Tex->TexelFormat = ( Header->Compressed ) ? GX_TF_CMPR : GX_TF_RGBA8;

	Tex->NFrames = 0;
	Tex->Frames = NULL;

	return true;
}

#pragma pack(push,1)
struct gctHeaderV3
{
	u8 Tag[4];
	u32 Version;
	u16 HeaderSize;
	u16 UserDataSize;
	u32 ImageSize;
	u16 Width;
	u16 Height;
	u8 TexelFormat;
	u8 LUTFormat;
	u8 Mipmaps;
	u8 Pad[9];
};
#pragma pack(pop)

NGL_INLINE static int nglGCTlutToSize( int Tlut )
{
	int Size = 0;

	switch( Tlut ) {
	case GX_TF_C4:
		Size = 16;
		break;
	case GX_TF_C8:
		Size = 256;
		break;
	case GX_TF_C14X2:
		Size = 1024;
		break;
	}

	return Size;
}

static bool nglLoadTextureGCTv3( nglTexture* Tex, unsigned char* Buf )
{
	gctHeaderV3* Header;

	Header = (gctHeaderV3*) Buf;

	Tex->Type = NGLTEX_GCT;

	Tex->Width = Header->Width;
	Tex->Height = Header->Height;

	Tex->Data = Buf;
	Tex->ImageData = Tex->Data + Header->HeaderSize;
	Tex->TexelFormat = Header->TexelFormat;
	Tex->PaletteSize = nglGCTlutToSize( Tex->TexelFormat );
	Tex->PaletteFormat = Header->LUTFormat;

	if( Tex->PaletteSize ) {
		Tex->PaletteData = Tex->Data + Header->HeaderSize + Header->ImageSize;
	} else {
		Tex->PaletteData = NULL;
	}

	Tex->Mipmaps = Header->Mipmaps;

	Tex->NFrames = 0;
	Tex->Frames = NULL;

	return true;
}

bool nglLoadTextureGCT( nglTexture* Tex, unsigned char* Buf, u_int BufSize )
{

	if( memcmp( Buf, "GCNT", 4 ) ) {
		return false;
	}

	u32 version = *(u32*)( Buf + 4 );

	//Ensure texture is in physical ram for gx!
    DCStoreRangeNoSync(Buf, BufSize);

	if( version == GCT_VERSION_V2 ) {
		return nglLoadTextureGCTv2( Tex, Buf );
	} else if( version == GCT_VERSION_V3 ) {
		return nglLoadTextureGCTv3( Tex, Buf );
	} else {
		return false;
	}

}

bool nglLoadTextureSplitGCT( nglTexture* Tex, unsigned char* header, unsigned char *palette, unsigned char *image )
{
	gctHeaderV3* Header;

	Header = (gctHeaderV3*) header;

	Tex->Type = NGLTEX_GCT;

	Tex->Width = Header->Width;
	Tex->Height = Header->Height;

	Tex->Data = header;
	Tex->ImageData = image;
	Tex->TexelFormat = Header->TexelFormat;
	Tex->PaletteSize = nglGCTlutToSize( Tex->TexelFormat );
	Tex->PaletteFormat = Header->LUTFormat;

	if( Tex->PaletteSize ) {
		Tex->PaletteData = palette;
	} else {
		Tex->PaletteData = NULL;
	}

	Tex->Mipmaps = Header->Mipmaps;

	Tex->NFrames = 0;
	Tex->Frames = NULL;

	return true;
}

//----------------------------------------------------------------------------------------
//  @IFL file code.
//----------------------------------------------------------------------------------------
static bool nglInLoadTextureIFL = false;

bool nglLoadTextureIFL( nglTexture* Tex, unsigned char *Buf, u_int BufSize )
{
  // Temporary buffer for building the texture array.
  static char FileName[256];
  static nglTexture* Textures[256];
  int NTextures = 0;

  if ( nglInLoadTextureIFL )
  {
    nglPrintf( "NGL: Recursive IFL detected.\n" );
    return false;
  }
  nglInLoadTextureIFL = true;

// handy parser macro
#define endofbuf( a ) ( (a) - (char*)Buf >= (int)BufSize )

  char* Pos = (char*)Buf;
  while ( !endofbuf( Pos ) )
  {
    // parse out the filename (no extension).
    char* Line = Pos;
    while ( !endofbuf( Line ) && !isspace( *Line ) ) Line++;
    u_int Count = mini( Line - Pos, 255 );
    if ( !Count ) break;
    strncpy( FileName, Pos, Count );
    FileName[Count] = '\0';
    char* Ext = strchr( FileName, '.' );
    if ( Ext ) *Ext = '\0';

    // attempt to load the texture.
    Textures[NTextures] = nglLoadTextureA( FileName );
    if ( Textures[NTextures] )
      NTextures++;

    // skip any whitespace
    Pos = Line;
    while ( !endofbuf( Pos ) && isspace( *Pos ) ) Pos++;
  }

#undef endofbuf

  nglInLoadTextureIFL = false;

  if ( NTextures )
  {
    Tex->Type = NGLTEX_IFL;
    Tex->NFrames = NTextures;

    Tex->Frames = (nglTexture**)nglMemAlloc( sizeof(nglTexture*) * NTextures );
    memcpy( Tex->Frames, Textures, sizeof(nglTexture*) * NTextures );

    // Steal our 'width and height' from the first frame.
    Tex->Width = Tex->Frames[0]->Width;
    Tex->Height = Tex->Frames[0]->Height;
    return true;
  }

  return false;
}

void nglLockTexture( nglTexture* Tex )
{
	// empty on GCN
}

#if 0
//----------------------------------------------------------------------------------------
//  @ATE file code.  ATE files are Kelly-Slater specific animated textures.
//----------------------------------------------------------------------------------------
#include "ngl_ate.h"

bool nglLoadTextureATE( nglTexture* Tex, unsigned char *Buf, const nglFixedString &FileName )
{
  atestring ps = atestring( FileName.c_str() );

  if ( !ATEHeaderValid((char *)Buf) )
    return false;

  Tex->Type = NGLTEX_ATE;
  Tex->NFrames = ATETextureCount( (char *)Buf, ps );
  if ( Tex->NFrames <= 0 )
  {
    return false;
  }

  Tex->Frames = (nglTexture**)nglMemAlloc( sizeof(nglTexture*) * Tex->NFrames );
  for ( u_int i=0; i<Tex->NFrames; i++ )
  {
    nglFixedString nam( ATETextureName( (char *)Buf, ps, i ).c_str() );
    #ifdef DEBUG
      char tname[256];
      strcpy(tname,nam.c_str());
    #endif
    u_char *hed=(u_char *) ATETextureHeader( (char *)Buf,ps, i );
    u_char *img=(u_char *) ATETextureImage( (char *)Buf,ps, i );
    u_char *pal=(u_char *) ATETexturePalette( (char *)Buf,ps, i );
    Tex->Frames[i] = (nglTexture*) nglMemAlloc( sizeof(nglTexture));
    memset( Tex->Frames[i], 0, sizeof(nglTexture) );
    nglLoadTextureSplitGCT(Tex->Frames[i],hed,pal,img);
    Tex->Frames[i]->FileName = nam;
  	//Tex->Frames[i]->LoadedInPlace=true;
  }

  Tex->Data=Buf;

  return true;
}
#endif

//-----------------------------------------------------------------------------
// Description: Create the default lighting context.
//-----------------------------------------------------------------------------
nglLightContext *nglCreateLightContext()
{
  nglLightContext* Context = nglListNew(nglLightContext);

	if( !Context )
	{
#ifdef NGL_VERBOSE
		nglPrintf( "nglCreateLightContext: could not allocate nglLightContext.\n" );
#endif
		return 0;
	}

  // Initialize the circular linked list.
  for (int i = 0; i < NGL_NUM_LIGHTCATS; i++)
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

//-----------------------------------------------------------------------------
// Description: // Switching and retrieving the current lighting context.
//-----------------------------------------------------------------------------

void nglSetLightContext( nglLightContext* Context )
{
  nglCurLightContext = Context;
}

nglLightContext *nglGetLightContext()
{
  return nglCurLightContext;
}

//-----------------------------------------------------------------------------
// Description: Set the ambient light.
//-----------------------------------------------------------------------------
void nglSetAmbientLight(float R, float G, float B)
{
  	nglCurLightContext->Ambient[0] = R;
  	nglCurLightContext->Ambient[1] = G;
  	nglCurLightContext->Ambient[2] = B;
}

//-----------------------------------------------------------------------------
// Description: Add a new light node.
//-----------------------------------------------------------------------------
void nglListAddLightNode(u_int Type, void* Data, u_int LightCat)
{
  nglLightNode* NewNode = nglListNew(nglLightNode);

	if( !NewNode )
	{
#ifdef NGL_VERBOSE
		nglPrintf( "nglListAddLightNode: could not allocate nglLightNode.\n" );
#endif
		return;
	}

  NewNode->Type = Type;
  NewNode->Data = Data;
  NewNode->LightCat = LightCat;

  for (int i = 0; i < NGL_NUM_LIGHTCATS; i++)
    if (LightCat & (1 << (NGL_LIGHTCAT_SHIFT + i)))
    {
      NewNode->Next[i] = nglCurLightContext->Head.Next[i];
      nglCurLightContext->Head.Next[i] = NewNode;
    }
}

//-----------------------------------------------------------------------------
// Description: Add a directional light to the light list.
//-----------------------------------------------------------------------------
void nglListAddDirLight(u_int LightCat, nglVector Dir, nglVector Color)
{
	nglDirLightInfo* Light = nglListNew(nglDirLightInfo);

	if (!Light)
	{
		nglPrintf("nglListAddDirLight: could not allocate nglDirLightInfo.\n");
		return;
	}

  	nglCopyVector(Light->Dir, Dir);
  	Light->Dir[3] = 0.0f;
  	nglCopyVector(Light->Color, Color);

  	nglListAddLightNode(NGLLIGHT_DIRECTIONAL, Light, LightCat);
}

//-----------------------------------------------------------------------------
// Description: Add a fake point light to the light list.
//-----------------------------------------------------------------------------
void nglListAddFakePointLight(u_int LightCat, nglVector Pos, float Near, float Far, nglVector Color)
{
  // Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
  nglVector ViewPos;
  nglApplyMatrix(ViewPos, nglCurScene->WorldToView, Pos);
  if (!nglSphereInFrustum(ViewPos, Far))
    return;

  nglPointLightInfo* Light = nglListNew(nglPointLightInfo);

	if( !Light )
	{
#ifdef NGL_VERBOSE
		nglPrintf( "nglListAddFakePointLight: could not allocate nglPointLightInfo.\n" );
#endif
		return;
	}

  nglCopyVector(Light->Pos, Pos);
  Light->Pos[3] = 1.0f;
  Light->Near = Near;
  Light->Far = Far;
#ifdef PROJECT_SPIDERMAN
	Color[3]=0; // Spidey code is misusing Alpha component for lights that are not to affect the translucency of the lit object.
#endif
  nglCopyVector(Light->Color, Color);

  nglListAddLightNode(NGLLIGHT_FAKEPOINT, Light, LightCat);

}

//-----------------------------------------------------------------------------
// Description: Add a true point light to the light list.
//-----------------------------------------------------------------------------
void nglListAddPointLight(u_int LightCat, nglVector Pos, float Near, float Far, nglVector Color)
{
  // Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
  nglVector ViewPos;
  nglApplyMatrix(ViewPos, nglCurScene->WorldToView, Pos);
  if (!nglSphereInFrustum(ViewPos, Far))
    return;

  nglPointLightInfo* Light = nglListNew(nglPointLightInfo);

	if( !Light )
	{
#ifdef NGL_VERBOSE
		nglPrintf( "nglListAddPointLight: could not allocate nglPointLightInfo.\n" );
#endif
		return;
	}

  nglCopyVector(Light->Pos, Pos);
  Light->Pos[3] = 1.0f;
  Light->Near = Near;
  Light->Far = Far;
#ifdef PROJECT_SPIDERMAN
	Color[3]=0; // Spidey code is misusing Alpha component for lights that are not to affect the translucency of the lit object.
#endif
  nglCopyVector(Light->Color, Color);

  nglListAddLightNode(NGLLIGHT_TRUEPOINT, Light, LightCat);
}

//-----------------------------------------------------------------------------

// v1 minus v2
static inline void nglVectSubtract(nglVector& dest, const nglVector& v1, const nglVector& v2)
{
  dest[0] = v1[0] - v2[0];
  dest[1] = v1[1] - v2[1];
  dest[2] = v1[2] - v2[2];
}

//-----------------------------------------------------------------------------

// v1 dot v2
static inline float nglVectDot(const nglVector& v1, const nglVector& v2)
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

//-----------------------------------------------------------------------------

// v1 cross v2 for a left handed system
static inline void nglVectCross(nglVector& dest, const nglVector& v1, const nglVector& v2)
{
  dest[0] = v1[2]*v2[1] - v1[1]*v2[2];
  dest[1] = v1[0]*v2[2] - v1[2]*v2[0];
  dest[2] = v1[1]*v2[0] - v1[0]*v2[1];
}

//-----------------------------------------------------------------------------

static inline void nglVectNormalize(nglVector& v)
{
  float length;

  length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if(length < 0.0000001f)
	{
		v[0] = v[1] = 0;
		v[2] = 1.0f;
		length = 1.0f;
	}
  length = 1.0f / length;

  v[0] *= length;
  v[1] *= length;
  v[2] *= length;
}

//-----------------------------------------------------------------------------

static inline float nglVectLength(const nglVector& v)
{
  	return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

//-----------------------------------------------------------------------------

// Computes the 6 planes of the view frustum given a
// projection matrix of any kind (orthographic or perspective)
// and also can incorporate a modelview type transformation
// to compute planes in object space.
void nglBuildFrustum(nglFrustum* frustum, nglMatrix& m)
{
  nglVector lp[8] = // local points
  {
    nglVector( 0.0f, 0.0f, 0.0f, 1.0f ), // 0 - (0,0,0)
    nglVector( 0.0f, 1.0f, 0.0f, 1.0f ), // 1 - (0,1,0)
    nglVector( 1.0f, 1.0f, 0.0f, 1.0f ), // 2 - (1,1,0)
    nglVector( 1.0f, 0.0f, 0.0f, 1.0f ), // 3 - (1,0,0)
    nglVector( 1.0f, 0.0f, 1.0f, 1.0f ), // 4 - (1,0,1)
    nglVector( 1.0f, 1.0f, 1.0f, 1.0f ), // 5 - (1,1,1)
    nglVector( 0.0f, 0.0f, 1.0f, 1.0f ), // 6 - (0,0,1)
    nglVector( 0.0f, 1.0f, 1.0f, 1.0f )  // 7 - (0,1,1)
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
  nglVector wp[8];  // world points
  nglVector v1, v2; // vectors used for computing normals

  for(int i = 0; i < 8; i++)
     nglApplyMatrix(wp[i], m, lp[i]);

  // compute near plane normal
  nglVectSubtract(v1, wp[1], wp[0]);
  nglVectSubtract(v2, wp[3], wp[0]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_NEAR], v1, v2);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_NEAR]);
  frustum->Planes[NGLFRUSTUM_NEAR][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_NEAR], wp[0]);

  // compute far plane normal
  nglVectSubtract(v1, wp[4], wp[6]);
  nglVectSubtract(v2, wp[7], wp[6]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_FAR], v1, v2);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_FAR]);
  frustum->Planes[NGLFRUSTUM_FAR][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_FAR], wp[6]);

  // compute left plane normal
  nglVectSubtract(v1, wp[6], wp[0]);
  nglVectSubtract(v2, wp[1], wp[0]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_LEFT], v1, v2);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_LEFT]);
  frustum->Planes[NGLFRUSTUM_LEFT][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_LEFT], wp[0]);

  // compute right plane normal
  nglVectSubtract(v1, wp[2], wp[3]);
  nglVectSubtract(v2, wp[4], wp[3]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_RIGHT], v1, v2);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_RIGHT]);
  frustum->Planes[NGLFRUSTUM_RIGHT][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_RIGHT], wp[3]);

  // compute top plane normal
  nglVectSubtract(v1, wp[7], wp[1]);
  nglVectSubtract(v2, wp[2], wp[1]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_TOP], v1, v2);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_TOP]);
  frustum->Planes[NGLFRUSTUM_TOP][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_TOP], wp[1]);

  // compute bottom plane normal
  nglVectSubtract(v1, wp[3], wp[0]);
  nglVectSubtract(v2, wp[6], wp[0]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_BOTTOM], v1, v2);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_BOTTOM]);
  frustum->Planes[NGLFRUSTUM_BOTTOM][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_BOTTOM], wp[0]);
}

//-----------------------------------------------------------------------------

// this function assumes that the x, y, and z componentes of plane are normalized
float nglDistanceToPlane(const nglVector& plane, const nglVector& point)
{
  //dot(unit_normal, pt) + d;
  float Dist = (plane[0]*point[0] + plane[1]*point[1] + plane[2]*point[2]) + plane[3];
  return Dist;
}

//-----------------------------------------------------------------------------

// takes a point and radius in world space
bool nglIsSphereVisible( nglFrustum* Frustum, const nglVector& Center, float Radius )
{
  // This is a good order to cull by, as it culls the majority of invisible
  // objects in the first couple tries.  Relies on short-circuit evaluation!
  Radius = -Radius;

  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_LEFT], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_RIGHT], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_NEAR], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_TOP], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_BOTTOM], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_FAR], Center) < Radius )
    return false;

  return true;
}

void nglListAddProjectorLightNode( u_int Type, void* Data, u_int LightCat )
{
  	nglLightNode* NewNode = nglListNew( nglLightNode );
  	if ( !NewNode )
    	return;

  	NewNode->Type = Type;
  	NewNode->Data = Data;
  	NewNode->LightCat = LightCat;

  	for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
	{
    	if ( LightCat & ( 1 << ( NGL_LIGHTCAT_SHIFT + i ) ) )
    	{
      		NewNode->Next[i] = nglCurLightContext->ProjectorHead.Next[i];
      		nglCurLightContext->ProjectorHead.Next[i] = NewNode;
    	}
	}
}

//-----------------------------------------------------------------------------

void nglListAddPointProjectorLight( u_int LightCat, nglVector Pos, nglVector Color, float Range, u_int BlendMode, u_int BlendModeConstant, nglTexture* Tex )
{
	// 07-Jan-2002 SAL: Not supported as yet.
  return;
}

//-----------------------------------------------------------------------------

void nglListAddDirProjectorLight(u_int LightCat, nglMatrix PO, nglVector Scale,
                                 u_int BlendMode, u_int BlendModeConstant, nglTexture* Tex)
{
  	nglDirProjectorLightInfo* Light = nglListNew(nglDirProjectorLightInfo);

  	if (!Light)
	{
		nglWarning("Unable to allocate nglDirPRojectorLight\n");
		return;
	}

  	Light->BlendMode = BlendMode;
  	Light->BlendModeConstant = BlendModeConstant;

   	Light->Tex = Tex;

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

  	Light->Pos[0] = PO[3][0];
  	Light->Pos[1] = PO[3][1];
  	Light->Pos[2] = PO[3][2];
  	Light->Pos[3] = 0.0f;

  	nglMatrix PositionMtx(
  		nglVector(   1.f,        0.f,      0.f,    0.f), // X
    	nglVector(   0.f,        1.f,      0.f,    0.f), // Y
    	nglVector(   0.f,        0.f,      1.f,    0.f), // Z
    	nglVector(-Light->Pos[0], -Light->Pos[1], -Light->Pos[2], 1.f)  // T
  	);

  	nglMatrix OrientationMtx(
    	nglVector(Light->Xaxis[0], Light->Yaxis[0], Light->Zaxis[0], 0.f),
    	nglVector(Light->Xaxis[1], Light->Yaxis[1], Light->Zaxis[1], 0.f),
    	nglVector(Light->Xaxis[2], Light->Yaxis[2], Light->Zaxis[2], 0.f),
    	nglVector(   0.f,              0.f,           0.f,         1.f)
  	);

  	nglMatrix ScaleMtx(
	    nglVector(1.0f/Scale[0], 0.f,            0.f,            0.f),
    	nglVector(0.f,           1.0f/Scale[1],  0.f,            0.f),
    	nglVector(0.f,           0.f,            1.0f/Scale[2],  0.f),
    	nglVector(0.f,           0.f,            0.f,            1.f)
  	);

  	nglMatrix UVTranslationMtx(
    	nglVector( 1.f,  0.f,  0.f, 0.f),
    	nglVector( 0.f,  1.f,  0.f, 0.f),
    	nglVector( 0.f,  0.f,  1.f, 0.f),
    	nglVector( 0.5f, 0.5f, 0.f, 1.f)
  	);

  	nglMatrix InversePositionMtx(
	    nglVector(    1.f,      0.f,      0.f,   0.f), // X
	    nglVector(    0.f,      1.f,      0.f,   0.f), // Y
	    nglVector(    0.f,      0.f,      1.f,   0.f), // Z
	    nglVector( Light->Pos[0], Light->Pos[1], Light->Pos[2], 1.f)  // T
  	);

  	nglMatrix InverseOrientationMtx(
    	nglVector(Light->Xaxis[0], Light->Xaxis[1], Light->Xaxis[2], 0.f), // X
    	nglVector(Light->Yaxis[0], Light->Yaxis[1], Light->Yaxis[2], 0.f), // Y
    	nglVector(Light->Zaxis[0], Light->Zaxis[1], Light->Zaxis[2], 0.f), // Z
    	nglVector(    0.f,             0.f,           0.f,       1.f)  // T
    	//   X                 Y                Z
  	);

  	nglMatrix InverseScaleMtx(
    	nglVector(Scale[0], 0.f,       0.f,       0.f),
    	nglVector(0.f,       Scale[1], 0.f,       0.f),
    	nglVector(0.f,       0.f,      Scale[2],  0.f),
    	nglVector(0.f,       0.f,      0.f,       1.f)
  	);

  	nglMatrix InverseUVTranslationMtx(
    	nglVector( 1.f,  0.f,  0.f, 0.f),
    	nglVector( 0.f,  1.f,  0.f, 0.f),
    	nglVector( 0.f,  0.f,  1.f, 0.f),
    	nglVector(-0.5f,-0.5f, 0.f, 1.f)
  	);

	nglMatrix Temp;

  	// compute this light's world to UV space matrix
  	nglMulMatrix( Light->WorldToUV, PositionMtx, OrientationMtx );
  	nglMulMatrix( Temp, Light->WorldToUV, ScaleMtx );
  	nglCopyMatrix( Light->WorldToUV, Temp );
  	// << do perspective here if/when necessary >>
  	nglMulMatrix( Temp, Light->WorldToUV, UVTranslationMtx );
  	nglCopyMatrix( Light->WorldToUV, Temp );

  	// compute the inverse matrix as well
  	nglMulMatrix( Light->UVToWorld, InverseUVTranslationMtx, InverseScaleMtx );
  	// << do inverse perspective here if/when necessary >>
  	nglMulMatrix( Temp, Light->UVToWorld, InverseOrientationMtx );
  	nglCopyMatrix( Light->UVToWorld, Temp );
  	nglMulMatrix( Temp, Light->UVToWorld, InversePositionMtx );
  	nglCopyMatrix( Light->UVToWorld, Temp );

  	// build the frustum for this light
  	nglBuildFrustum( &Light->Frustum, Light->UVToWorld );

  	nglListAddProjectorLightNode( NGLLIGHT_PROJECTED_DIRECTIONAL, Light, LightCat );
}

//-----------------------------------------------------------------------------
// Description: Frustum clipping: test a sphere rejection according to
//             the scene frustum (6 planes).
//             Return true if the sphere is in the view frustum.
//-----------------------------------------------------------------------------
static NGL_INLINE bool nglSphereInFrustum(const nglVector Center, float Radius)
{
  	// NearZ plane clipping.
  	if (Center[2] + Radius < nglCurScene->NearZ)
    	return false;

  	// FarZ plane clipping.
  	if (Center[2] - Radius > nglCurScene->FarZ)
    	return false;

  	// Left plane clipping.
  	float cf = cosf(NGL_DEGTORAD(nglCurScene->HFOV));
  	float sf = sinf(NGL_DEGTORAD(nglCurScene->HFOV)) * Center[2] + Radius;

  	if (Center[0] * -cf + sf < 0.0f)
    	return false;

  	// Right plane clipping.
  	if (Center[0] *  cf + sf < 0.0f)
    	return false;

  	// Up plane clipping.
  	if (Center[1] *  cf + sf < 0.0f)
    	return false;

  	// Bottom plane clipping.
  	if (Center[1] *  -cf + sf < 0.0f)
    	return false;

  	return true;
}

float ConstA[3]={ 1.0f, 0.0f, 0.0f}, ConstK[3]={1.0f,0.0f,0.0f};

static void nglSetupMeshLighting(nglMeshSection* Section, nglMesh* Mesh, nglMeshNode* MeshNode, nglRenderParams* Params)
{
	int NumLights = 0;
	u32 LightsMask = GX_LIGHT_NULL;
	u32 ProjLightsMask = GX_LIGHT_NULL;
	u32 ParamFlags = Params->Flags;

	// Determine what lighting context (list of lights) the mesh uses.
  	nglCurLightContext = nglDefaultLightContext;
  if (ParamFlags & NGLP_LIGHT_CONTEXT)
   	nglCurLightContext = Params->LightContext;

  	// There's a bug here involving adding the light to the local lights more than once if it matches on
  	// more than one lighting category.
	if (nglGCDoShadowMap)
	{
		// Use the first light for the alpha computation for the ProjectorLight.
    	nglDirProjectorLightInfo* Light = TheProjLight;
		GXLightObj *TheLightObj=&nglLights[NumLights];
		ProjLightsMask = ProjLightsMask | nglGXLight[NumLights];
		GXColor Color;
		Color.r = 255;
		Color.g = 255;
		Color.b = 255;
		Color.a = 255;
		GXInitLightColor( TheLightObj, Color );
		float ScaleInfinity = -1024 *1024;
		nglVector LightViewDir,LightWorldDir;
	  	nglMatrix ViewToWorld;
	  	nglTransposeMatrix( ViewToWorld, nglCurScene->WorldToView );
	  	nglCopyVector( LightWorldDir, Light->Zaxis );
	  	LightWorldDir[3] = 1.0f;
  		LightWorldDir[2]=-LightWorldDir[2];
		nglApplyMatrix(LightViewDir, ViewToWorld, LightWorldDir);
		GXInitLightPos( TheLightObj, LightViewDir[0] * ScaleInfinity,
			LightViewDir[1] * ScaleInfinity, LightViewDir[2] * ScaleInfinity );
		GXInitLightDir( TheLightObj, 1.0f, 0.0f, 0.0f ); // Not useful, but didn't want to leave it unitialized.
		GXInitLightAttn( TheLightObj, 1.0f, 0, 0, 1.0f, 0, 0 );
		GXLoadLightObjImm(TheLightObj, nglGXLight[NumLights]);
		NumLights++;
	}

  if (STAGE_ENABLE(Light) && (Section->Material->Flags&NGLMAT_LIGHT) && !(ParamFlags&NGLP_NO_LIGHTING))
  {
		for (u_int i = 0; i < NGL_NUM_LIGHTCATS; i++)
	  {
	   	if (!(Mesh->Flags & (1 << (NGL_LIGHTCAT_SHIFT + i))))
	  		continue;

			nglLightNode* LightNode = nglCurLightContext->Head.Next[i]; // LocalNext is not used for the GC.
	    while ((LightNode != &nglCurLightContext->Head) && (NumLights < 8))
		  {
	     	if ((LightNode->Type == NGLLIGHT_FAKEPOINT) || (LightNode->Type == NGLLIGHT_TRUEPOINT))
		   	{
	       	nglPointLightInfo* Light = (nglPointLightInfo*)LightNode->Data;
		     	nglVector LightPos;

		     	// Transform light to local space.
		     	nglApplyMatrix( LightPos, MeshNode->WorldToLocal, Light->Pos );

					if ( !nglSpheresIntersect( LightPos, Light->Far, Section->SphereCenter, Section->SphereRadius ) )
			   	{
			   		LightNode = LightNode->Next[i];
			   		continue;
			   	}

					nglVector V;
					float Range, Scale;

					nglSubVector( V, LightPos, Mesh->SphereCenter );
					Range = sqrtf( V[0] * V[0] + V[1] * V[1] + V[2] * V[2] );
					nglScaleVector( V, V, 1.0f / Range );
					V[3] = 0.0f;

					if ( Light->Far <= Light->Near )
						Scale = 1.0f;
					else
					{
						Scale = 1.0f - ( Range - Light->Near ) / ( Light->Far - Light->Near );
						if ( Scale <= 0.0f )
						{
				      		LightNode = LightNode->Next[i];
							continue; // Not adding this light.
						}
						if ( Scale > 1.0f )
							Scale = 1.0f;
					}
					nglVector TheColor;
					nglScaleVector( TheColor, Light->Color, Scale );

					GXLightObj *TheLightObj=&nglLights[NumLights];
					LightsMask = LightsMask | nglGXLight[NumLights];
					GXColor Color;
					Color.r = (u8) ( TheColor[0] * 127.0f );
					Color.g = (u8) ( TheColor[1] * 127.0f );
					Color.b = (u8) ( TheColor[2] * 127.0f );
					Color.a = (u8) ( TheColor[3] * 255.0f );
					GXInitLightColor( TheLightObj, Color );
					// GXInitLightAttn( TheLightObj, 1.0f, 0, 0, 1.0f, 0, 0 );
					float ScaleInfinity = 1024 *1024;
					// Compute LightDir in world coordinates.
					// <<<< 07-Jan-2002 SAL: Ignoring the sphere center for now.
					V[0] = Light->Pos[0] - nglMeshLocalToWorld[3][0];
					V[1] = Light->Pos[1] - nglMeshLocalToWorld[3][1];
					V[2] = Light->Pos[2] - nglMeshLocalToWorld[3][2];
					GXInitLightPos( TheLightObj, V[0] * ScaleInfinity,
						V[1] * ScaleInfinity, V[2] * ScaleInfinity );
					GXLoadLightObjImm(TheLightObj, nglGXLight[NumLights]);
					NumLights++;
				}
				else if (LightNode->Type == NGLLIGHT_DIRECTIONAL)
				{
					#define INF	-(1024.0f*1024.0f)
					// mark this light as used
					LightsMask |= nglGXLight[NumLights];
					// grab the light object for this node
	        		nglDirLightInfo* Light = (nglDirLightInfo*)LightNode->Data;
					// set the color of the light
					nglVector LightColor( Light->Color );

					GXColor Color;
					Color.r = (u8)( LightColor[0] * 127.0f );
					Color.g = (u8)( LightColor[1] * 127.0f );
					Color.b = (u8)( LightColor[2] * 127.0f );
					Color.a = (u8)(Light->Color[3] * 255.0f);
					// setup the light for use on the gamecube
					GXLightObj *TheLightObj = &nglLights[NumLights];
					GXInitLightColor(TheLightObj, Color);
					GXInitLightPos(TheLightObj, Light->Dir[0]*INF, Light->Dir[1]*INF, Light->Dir[2]*INF);
					GXLoadLightObjImm(TheLightObj, nglGXLight[NumLights]);
					// increase the count for the next light
					NumLights++;
					#undef INF
		  	}

	      LightNode = LightNode->Next[i];
			}
	  }
		// see if we need to apply ambient
		if ((nglCurLightContext->Ambient.x + nglCurLightContext->Ambient.y + nglCurLightContext->Ambient.z) != 3.0f)
		{
			// apply ambient to the tint
			if (ParamFlags&NGLP_TINT)
			{
				// if we already have a tint, scale it by the ambient color
				Params->TintColor.x *= nglCurLightContext->Ambient.x;
				Params->TintColor.y *= nglCurLightContext->Ambient.y;
				Params->TintColor.z *= nglCurLightContext->Ambient.z;
				Params->TintColor.w *= nglCurLightContext->Ambient.w;
			}
			else
			{
				// if we don't have tint, add it as the ambient
				ParamFlags |= NGLP_TINT;
				Params->TintColor.x = nglCurLightContext->Ambient.x;
				Params->TintColor.y = nglCurLightContext->Ambient.y;
				Params->TintColor.z = nglCurLightContext->Ambient.z;
				Params->TintColor.w = nglCurLightContext->Ambient.w;
			}
		}
	}

	GXColor Color;
	if( Section->Material->Flags&NGLMAT_LIGHT )
	{
		if( ParamFlags&NGLP_TINT )
		{
			Color.r = (u8)( Params->TintColor[0] * 127.0f );
			Color.g = (u8)( Params->TintColor[1] * 127.0f );
			Color.b = (u8)( Params->TintColor[2] * 127.0f );
			Color.a = (u8)( Params->TintColor[3] * 255.0f );
		}
		else
		{
			Color.r = 127;
			Color.g = 127;
			Color.b = 127;
			Color.a = 255;
		}
	}
	else
	{
		if( ParamFlags&NGLP_TINT )
		{
			Color.r = (u8)( Params->TintColor[0] * 255.0f );
			Color.g = (u8)( Params->TintColor[1] * 255.0f );
			Color.b = (u8)( Params->TintColor[2] * 255.0f );
			Color.a = (u8)( Params->TintColor[3] * 255.0f );
		}
		else
		{
			Color.r = 255;
			Color.g = 255;
			Color.b = 255;
			Color.a = 255;
		}
	}

	GXSetChanAmbColor(GX_COLOR0A0, Color);
	GXSetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, LightsMask, GX_DF_CLAMP, GX_AF_NONE);

	// FIXME: we should be able to do this without the use of the other color channel
	if (nglGCDoShadowMap)
	{
		// No ambient color.
		GXSetChanAmbColor(GX_COLOR1A1, _nglGXColorOne);
		GXSetChanMatColor(GX_COLOR1A1, _nglGXColorOne);
		GXSetChanCtrl(GX_COLOR1A1, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, ProjLightsMask, GX_DF_CLAMP, GX_AF_SPOT);
	}
	else
	{
		GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	}

}

//-----------------------------------------------------------------------------

static void nglSetupMeshProjLighting(nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params)
{
	int NumProjLights = 0;

  	// Determine what lighting context (list of lights) the mesh uses.
  	nglCurLightContext = nglDefaultLightContext;
  	if ( Params->Flags & NGLP_LIGHT_CONTEXT )
    	nglCurLightContext = Params->LightContext;

	// There's a bug here involving adding the light to the local lights more than once if it matches on
  	// more than one lighting category.
  	nglVector WorldPos;
  	nglApplyMatrix(WorldPos, nglMeshLocalToWorld, Mesh->SphereCenter);

	// don't apply this effect to skinned meshes
  	if (!(Mesh->Flags& NGLMESH_SKINNED))
  	{
	  	for (u_int i=0; i<NGL_NUM_LIGHTCATS; i++)
	  	{
			// skip catagories that we haven't set
	    	if (!(Mesh->Flags&(1<<(NGL_LIGHTCAT_SHIFT+i))))
	      		continue;
			// get the light node for this projector
			nglLightNode* LightNode = nglCurLightContext->ProjectorHead.Next[i]; // LocalNext is not used for the GC.
	    	while ((LightNode != &nglCurLightContext->ProjectorHead) && (NumProjLights < 1))
		  	{
				if (LightNode->Type == NGLLIGHT_PROJECTED_DIRECTIONAL)
				{
	        		nglDirProjectorLightInfo* Light = (nglDirProjectorLightInfo*)LightNode->Data;
	        		if (nglIsSphereVisible(&Light->Frustum, WorldPos, Mesh->SphereRadius))
	        		{
	        			NumProjLights++;
	        			TheProjLight = Light;
	        			nglGCDoShadowMap = true;
	        		}
				}
				LightNode = LightNode->Next[i];
			}
	  	}
	}
}

//-----------------------------------------------------------------------------

nglTexture* nglCreateTexture(u_int Format, u_int Width, u_int Height)
{
  	NGL_ASSERT((Format&NGLTF_32BIT)||(Format&NGLTF_16BIT), "nglCreateTexture: Invalid texture format" );

	// setup the texture parameters
	u32 TexFormat = (Format&NGLTF_32BIT) ? GX_TF_RGBA8 : GX_TF_RGB565;
  	u32 ImageSize = GXGetTexBufferSize(Width, Height, TexFormat, GX_DISABLE, 0);
	// allocate memory for the texture
  	nglTexture*  Tex = (nglTexture*)nglMemAlloc(sizeof(nglTexture));
  	gctHeaderV3* Hdr = (gctHeaderV3*)nglMemAlloc(sizeof(gctHeaderV3)+ImageSize, 32);
	// clear out the memory
  	memset(Tex, 0, sizeof(nglTexture));
  	memset(Hdr, 0, sizeof(gctHeaderV3));
	// setup the header
  	Hdr->Tag[0] = 'G';
  	Hdr->Tag[1] = 'C';
  	Hdr->Tag[2] = 'N';
  	Hdr->Tag[3] = 'T';
  	Hdr->Version = GCT_VERSION;
  	Hdr->HeaderSize = (u32)sizeof(gctHeaderV3);
  	Hdr->ImageSize = ImageSize;
  	Hdr->Width = (u16)Width;
  	Hdr->Height = (u16)Height;
  	Hdr->TexelFormat = TexFormat;
	// setup the ngl texture
  	Tex->Type = NGLTEX_GCT;
  	Tex->Flags = 0;
  	Tex->Width = Hdr->Width;
  	Tex->Height = Hdr->Height;
  	Tex->Data = (unsigned char*)Hdr;
  	Tex->ImageData = (unsigned char*)Tex->Data + Hdr->HeaderSize;
  	Tex->TexelFormat = (u_int)Hdr->TexelFormat;
  	Tex->FileName = nglFixedString("[created]");
	// initialize the gcn texture object
  	GXInitTexObj(&Tex->TexObj, Tex->ImageData, (u16)Tex->Width, (u16)Tex->Height,
		(GXTexFmt)Tex->TexelFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);

  	return Tex;
}

//-----------------------------------------------------------------------------

void nglSetTVMode( u_int TVMode )
{
  	GXRenderModeObj* rmo = NULL;
  	nglTVMode = TVMode;

	switch (nglTVMode)
	{
	case NGLTV_NTSC:
		nglDisplayWidth = 640;
		nglDisplayHeight = 448;
		rmo = &GXNtsc480IntDf;
		break;
	case NGLTV_PAL:
		nglDisplayWidth = 640;
		nglDisplayHeight = 512;
		rmo = &GXPal528IntDf;
		break;
	case NGLTV_MPAL:
		nglDisplayWidth = 640;
		nglDisplayHeight = 448;
		rmo = &GXMpal480IntDf;
		break;
	case NGLTV_EURBG60:
		nglDisplayWidth = 640;
		nglDisplayHeight = 512;
		rmo = &GXEurgb60Hz480IntDf;
		break;
	default:
		NGL_ASSERT(FALSE, "Unknown NGLTV_MODE");
		break;
	}

	memcpy(&nglRenderModeObj, rmo, sizeof(nglRenderModeObj) );

}

//-----------------------------------------------------------------------------

u_int nglGetTVMode( void )
{
	return nglTVMode;
}

//-----------------------------------------------------------------------------

void nglResetDisplay()
{
	VIConfigure( &nglRenderModeObj );
	VIFlush( );

	nglFlip();
	nglFlip();
}

//-----------------------------------------------------------------------------

void nglProjectPoint(nglVector& Out, nglVector In)
{
  	In[3] = 1.0f;
	nglVector InVec;
	InVec[0]=In[0]; InVec[1]=In[1];
	InVec[2]=-In[2];
  	InVec[3]=In[3];
  	nglVector OutVec;
  	nglApplyMatrix(InVec, nglCurScene->WorldToView, In);
  	InVec[2] = -InVec[2];
  	nglMatrix ViewToScreen;
	nglTransposeMatrix( ViewToScreen, nglCurScene->ViewToScreen );
	nglApplyMatrix(OutVec, ViewToScreen, InVec);

  	if(!OutVec[3])
  		OutVec[3]=1;
  	float InvW = 1.0f / OutVec[3];
  	Out[0] = (OutVec[0] * InvW + 1) * (nglGetScreenWidth() / 2);
  	Out[1] = (-OutVec[1] * InvW + 1) * (nglGetScreenHeight() / 2);
  	Out[2] = OutVec[2] * InvW; // <<<< 22-Feb-2002 SAL: Spidey doesn't use this, so deal with this parameter later.
}

/*---------------------------------------------------------------------------------------------------------

  @Font API.

  Mostly taken from Sean Palmer's font system.
---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Return the integer value following the specified token.
             For example, if data contains: "posx 320 posy 200 color 128",
             the call nglGetTokenUINT(data, "posy", 10) will returns 200.
-----------------------------------------------------------------------------*/
u_int nglGetTokenUINT(char *&Data, char *Token, u_int Base)
{
	u_int TokenLength = strlen(Token);

	for (;;)
	{
		u_int CharCount = 0;

		// Locate the beginning of the token.
		for (u_int i = 0; i < TokenLength; i++)
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
			u_int Val = strtol(Data, &StopString, Base);
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
u_int nglGetUINT(char *&Data, u_int Base)
{
	// Move to the first digit.
	while (*Data < '0' || *Data > '9')
		Data++;

	// String to integer conversion.
	char *StopString;
	u_int Val = strtol(Data, &StopString, Base);
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
	NGL_ASSERT(Font->Header.Version == NGLFONT_VERSION,
			   "Unsupported font version !");

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

	for (u_int i = 0; i < Font->Header.NumGlyphs; i++)
	{
		nglGlyphInfo &gi = Font->GlyphInfo[i];
		u_int gidx = nglGetUINT(Data, 10);
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

	// The font's texture must be loaded before calling this function !
	// Get the font's texture. If it fails, the function returns.
	nglFont *Font = (nglFont *) nglMemAlloc(sizeof(nglFont));
	memset(Font, 0, sizeof(nglFont));

	Font->Tex = nglGetTexture(FontName);

	if (!Font->Tex)
	{
		nglFatal("nglLoadFont(): failed to get the texture %s !\n",
				 FontName.c_str());
		return NULL;
	}

	Font->MapFlags = NGLMAP_BILINEAR_FILTER;
  	Font->BlendMode = NGLBM_BLEND;

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
	nglPrintf("Loading font %s\n", FontName);

	// Search for an existing copy.
	nglInstanceBank::Instance *Inst;
	if ((Inst = nglFontBank.Search(FontName)))
	{
		Inst->RefCount++;
		return (nglFont *) Inst->Value;
	}

	// The font's texture must be loaded before calling this function !
	// Get the font's texture. If it fails, the function returns.
	nglFont *Font = (nglFont *) nglMemAlloc(sizeof(nglFont));
	memset(Font, 0, sizeof(nglFont));

	Font->Tex = nglGetTexture(FontName);

	if (!Font->Tex)
	{
		nglFatal("nglLoadFont(): failed to get the texture %s !\n",
				 FontName.c_str());
		return NULL;
	}

	Font->MapFlags = NGLMAP_BILINEAR_FILTER;
  	Font->BlendMode = NGLBM_BLEND;

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

/*-----------------------------------------------------------------------------
Description: Setup the font's mesures.
-----------------------------------------------------------------------------*/
void nglSetFontMeasures(nglFont *Font, float *aoffs, float *asize, float *auvpos,
						float *auvsize, float scalex, float scaley, const nglGlyphInfo &ginfo)
{
	float InvWidth = 1.0f / (float) Font->Tex->Width;
	float InvHeight = 1.0f / (float) Font->Tex->Height;
	aoffs[0] = int(ginfo.GlyphOrigin[0]) * scalex;
	aoffs[1] = int(ginfo.GlyphOrigin[1]) * scaley;
	asize[0] = u_int(ginfo.GlyphSize[0]) * scalex;
	asize[1] = u_int(ginfo.GlyphSize[1]) * scaley;
	auvpos[0] = u_int(ginfo.TexOfs[0]) * InvWidth;
	auvpos[1] = u_int(ginfo.TexOfs[1]) * InvHeight;
	auvsize[0] = u_int(ginfo.GlyphSize[0]) * InvWidth;
	auvsize[1] = u_int(ginfo.GlyphSize[1]) * InvHeight;
}

/*-----------------------------------------------------------------------------
Description: Render a single font using the Quad API.
-----------------------------------------------------------------------------*/
void nglRenderSingleFont(nglFont* Font, u_int Color, float* Pos,
						 float* Size, float* UVPos, float* UVSize)
{
	float x_ratio = 1.0f, y_ratio = 1.0f;
	float left = (Pos[0]) * x_ratio;
	float top = (Pos[1]) * y_ratio;
	float right = (Pos[0] + Size[0]) * x_ratio;
	float bottom = (Pos[1] + Size[1]) * y_ratio;

	nglQuad q;
	nglInitQuad(&q);
	nglSetQuadZ(&q, Pos[2]);
	nglSetQuadRect(&q, left, top, right, bottom);
	nglSetQuadUV(&q, UVPos[0], UVPos[1], UVPos[0] + UVSize[0], UVPos[1] + UVSize[1]);
	nglSetQuadColor(&q, Color);
	nglSetQuadBlend(&q, Font->BlendMode, Font->BlendModeConstant);
	nglSetQuadTex(&q, Font->Tex);
	nglSetQuadMapFlags(&q, Font->MapFlags);
	nglListAddQuad(&q);
}

void nglSetFontMapFlags(nglFont *Font, u_int MapFlags)
{
	Font->MapFlags = MapFlags;
}

u_int nglGetFontMapFlags(nglFont *Font)
{
	return Font->MapFlags;
}

// Set the font's blend mode.
void nglSetFontBlendMode(nglFont* Font, u_int BlendMode, u_int Constant /*= 0*/)
{
	Font->BlendMode = BlendMode;
	Font->BlendModeConstant = Constant;
}

/*-----------------------------------------------------------------------------
Description: Add a string to the nodes' list to be rendered.
             The string may contains some formatting informations:
             - The current font color is set by the token (hexa): \1[RRGGBBAA]
             - The current font scale is set by the token: \2[SCALE]
             The default color is the Color argument and the default scale is 1.
             For example:
             "Hi from \1[ff0000ff]NGL \1[ffffffff] :-)\n\n\2[2.0]Bye!");
             Writes "NGL" in red and "Bye" with a scale of 2.0.
-----------------------------------------------------------------------------*/

void _nglListAddString(nglFont *Font, float x, float y, float z, u_int Color, const char *Text);

void nglListAddString(nglFont *Font, float x, float y, float z, const char *Text, ...)
{
	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);

	_nglListAddString(Font, x, y, z, 0xFFFFFFFF, nglWork);
}

void nglListAddString(nglFont *Font, float x, float y, float z, u_int Color, const char *Text, ...)
{
	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);

	_nglListAddString(Font, x, y, z, Color, nglWork);
}

void _nglListAddString(nglFont *Font, float x, float y, float z, u_int Color, const char *Text)
{
	if (Text == NULL || *Text == 0)
		return;

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

	while ( *TextPtr )
	{
		c = *TextPtr;
		// Check for color token: \1[RRGGBBAA] or \1[0xRRGGBBAA]
		if (c == NGLFONT_TOKEN_COLOR)
		{
			TextPtr += 2;
			// String to integer (hexa) conversion.
			char *StopString;
			u_int ColorRGBA = strtoul(TextPtr, &StopString, 16);
			Color = ColorRGBA;
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
			const nglGlyphInfo &ginfo = Font->GlyphInfo[(u_char) c - Font->Header.FirstGlyph];

			if (c != ' ')
			{
				nglSetFontMeasures(Font, offs, size, uvpos, uvsize, ScaleX, ScaleY, ginfo);
				float gpos[3] =	{ curpos[0] + offs[0], curpos[1] + offs[1], curpos[2] };
				nglRenderSingleFont(Font, Color, gpos, size, uvpos, uvsize);
			}

			curpos[0] += ginfo.CellWidth * ScaleX;
		}

		TextPtr++;
	}
}

/*-----------------------------------------------------------------------------
Description: Get the dimension, in pixels, of a string (which can have scale
             tokens and cariage returns as well).
-----------------------------------------------------------------------------*/
void _nglGetStringDimensions(nglFont* Font, u_int* outWidth, u_int* outHeight, const char* Text, ...)
{
	const char* TextPtr = Text;
    char c;
    char prev_c = 0;

    float ScaleX = 1.0f;
    float ScaleY = 1.0f;
    float CurMaxScaleY = ScaleY;

    float Width  = 0.0f;
    float MaxWidth = 0.0f;
    float Height = 0.0f;
    float MaxHeight = 0.0f;
    float TotalHeight = 0.0f;

    while ( ( c = *TextPtr ) )
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

        if (c != '\n')
        {
            if (prev_c == '\n')
            {
                TotalHeight += MaxHeight;
                CurMaxScaleY = ScaleY;
                Height = Font->Header.CellHeight * ScaleY;
                MaxHeight = Height;
                Width = 0;
            }

            const nglGlyphInfo &ginfo = Font->GlyphInfo[(unsigned char) c - Font->Header.FirstGlyph];

            Width += ginfo.CellWidth * ScaleX;
            if (Width > MaxWidth)
                MaxWidth = Width;

            Height = Font->Header.CellHeight * CurMaxScaleY;
            if (Height > MaxHeight)
                MaxHeight = Height;
        }

        prev_c = c;

        TextPtr++;
    }

    TotalHeight += MaxHeight;

	if (outHeight)
    	*outHeight = nglFTOI(TotalHeight);
	if (outWidth)
    	*outWidth = nglFTOI(MaxWidth);
}

void nglGetStringDimensions(nglFont* Font, u_int* outWidth, u_int* outHeight, const char* Text, ...)
{
	va_list argptr;
	va_start(argptr, Text);
	vsprintf(nglWork, Text, argptr);
	va_end(argptr);

	_nglGetStringDimensions(Font, outWidth, outHeight, nglWork);
}

void nglGCGetFrameBuffers( void** fb1, void** fb2, void** fb )
{

	if( fb ) {
		*fb = nglFrameBuffer;
	}

}

void nglGCFlipXFB( void )
{
	// empty
}

/*

  dummy functions needed to link with libsn.a

 */

#ifdef __LIBSN__
extern "C" {

int	__TRK_write_console(unsigned long handle, unsigned char* buffer, size_t* count, void(*idle_proc)(void) )
{
	return 0;
}

int	__read_console(unsigned long handle, unsigned char* buffer, size_t* count, void(*idle_proc)(void) )
{
	return 0;
}

#ifndef PROJECT_KELLYSLATER
enum
{
	__no_io_error
};

typedef struct
{
	unsigned int	open_mode		: 2;
	unsigned int	io_mode			: 3;
	unsigned int	buffer_mode		: 2;
	unsigned int	file_kind		: 3;	/*- mm 980708 -*/

	unsigned int	binary_io		: 1;
} __file_modes;

typedef unsigned long	__file_handle;
typedef void (* __idle_proc)  (void);
#endif

int	__open_file(const char * name, __file_modes mode, __file_handle * handle)
{
	return(__no_io_error);
}

int __read_file(__file_handle handle, unsigned char * buffer, size_t * count, __idle_proc idle_proc)
{
	return(__no_io_error);
}

int __write_file(__file_handle handle, unsigned char * buffer, size_t * count, __idle_proc idle_proc)
{
	return(__no_io_error);
}

int __position_file(__file_handle handle, fpos_t * position, int mode, __idle_proc idle_proc)
{
	return(__no_io_error);
}

int __close_file(__file_handle handle)
{
	return(__no_io_error);
}

};
#endif

//-----------------------------------------------------------------------------
