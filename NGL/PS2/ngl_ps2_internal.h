/*---------------------------------------------------------------------------------------------------------
  NGL_PS2_INTERNAL.H - Low level access to the NGL module for writing Custom Nodes and Extensions.

  Maintained by Wade Brainerd (wade@treyarch.com), Mike Montague (mikem@treyarch.com),
  Andy Chien (chien@treyarch.com).
---------------------------------------------------------------------------------------------------------*/
#ifndef NGL_PS2_INTERNAL_H
#define NGL_PS2_INTERNAL_H

/*---------------------------------------------------------------------------------------------------------
  System headers.
---------------------------------------------------------------------------------------------------------*/
#include <malloc.h>
#include <libdma.h>
#include <libvu0.h>
#include <libsn.h>
#include <libpkt.h>
#include <libgraph.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <math.h>
#include <libpc.h>
#include <ctype.h>
#include <stdarg.h>

// Just in case this wasn't included already, we don't want compiler errors.
#include "ngl_ps2.h"

/*---------------------------------------------------------------------------------------------------------
  Internal defines
  
  Define these on the command line using -DNGL_XXXX or -DNGL_XXXX=0, to configure NGL for your project.
---------------------------------------------------------------------------------------------------------*/
// Defining NGL_FINAL removes all print statements, NGL_ASSERT calls, and lots of debugging and profiling
// code.  If you want to leave the profiling or debugging code in, you can control those manually too.
#ifdef NGL_FINAL
#undef NGL_PROFILING
#undef NGL_DEBUGGING
#define NGL_PROFILING 0
#define NGL_DEBUGGING 0
#endif

// Use this define to make NGL PS2 emulate 640x480 resolution no matter what the actual resolution is.
// Not implemented yet!!
#ifndef NGL_EMULATE_480_LINES
#define NGL_EMULATE_480_LINES 0
#endif

// The PS2 renders 448 lines per frame, versus the XBox and GameCube's 480.  However, their aspect ratio 
// is still the same because the XBox and GC just cut 16 lines from the top and bottom of the screen 
// (480-32=448) as opposed to expanding pixels.
//
// Therefore, the most correct solution for the PS2 is to just shift all screen coordinates for interface
// elements (quads and strings) up by 16 pixels, and tell the application for positioning purposes that
// they're rendering in a x480 resolution.
#ifndef NGL_EMULATE_480_LINES
#define NGL_EMULATE_480_LINES 0
#endif

// NGL_USE_SN_TUNER sets up NGL to be compatible with the SN Systems performance tuner.  This includes
// not using any scePcXXXX calls and not resetting the CPU cycle counter.  Note that your project
// should also not make use of sceSifRebootIOP.
#ifndef NGL_USE_SN_TUNER
#define NGL_USE_SN_TUNER 0
#endif

// NGL_ASYNC_LIST_SEND enables writing the next dma chain while the previous one renders, which usually
// gains a few extra milliseconds for rendering at the cost of double-size DMA buffers.
#ifndef NGL_ASYNC_LIST_SEND
#define NGL_ASYNC_LIST_SEND 1
#endif

// This disables ALL mipmap handling code.
#ifndef NGL_DISABLE_MIPMAPS
#define NGL_DISABLE_MIPMAPS 0
#endif

// Backwards compatibility define, use this to indicate that filtering flags in materials are corrupt
// and that the given value should be used for everything.
//#define NGL_FORCE_FILTER NGLMAP_TRILINEAR_FILTER

// Write DMA chains using Scratchpad burst transfers (only takes place inside nglListSend).
#ifndef NGL_USE_SCRATCHPAD
#define NGL_USE_SCRATCHPAD 1
#endif

// Controls removing nglPrintf statements, NGL_ASSERT statements and debug code.
#ifndef NGL_DEBUGGING
#define NGL_DEBUGGING 1
#endif

// Controls whether or not NGL times it's internal functions or not.  Some things like FPS are calculated
// regardless of this flag.
#ifndef NGL_PROFILING
#define NGL_PROFILING 1
#endif

// Global effects of the profiling and debugging flags.
#if NGL_DEBUGGING == 0
#define STAGE_ENABLE( x ) 1
#define DEBUG_ENABLE( x ) 0
#else
#define STAGE_ENABLE( x ) ( nglStage.x )
#define DEBUG_ENABLE( x ) ( nglSyncDebug.x )
#endif

#if NGL_PROFILING == 0
#undef START_PROF_TIMER
#undef STOP_PROF_TIMER
#define START_PROF_TIMER(x)
#define STOP_PROF_TIMER(x)
#endif

#if NGL_DEBUGGING == 0
#define NGL_ASSERT(x,s) ( (void)0 )
#else
#define NGL_ASSERT(x,s) \
  if ( !(x) )             \
  {                       \
    nglWarning( TTY_BLACK "Assertion failed in %s(%d):\n" TTY_RED "\"%s\" - %s\n", __FILE__, __LINE__, #x, s );  \
    asm( "break 1" );     \
  }
#endif

// Handy crash tracing mechansim.
#if 0
#undef START_PROF_TIMER
#undef STOP_PROF_TIMER
#define START_PROF_TIMER(x) nglPrintf( "enter " #x "\n" );
#define STOP_PROF_TIMER(x) nglPrintf( "leave " #x "\n" );
#endif

// Have to use different casts under GCC 3.0.x.
#if __GNUC__ >= 3
#define SCE_MATRIX(x) ((float(*)[4])(float*)&(x))
#define SCE_VECTOR(x) ((float*)(x))
#else
#define SCE_MATRIX(x) ((sceVu0FMATRIX)&(x))
#define SCE_VECTOR(x) ((sceVu0FVECTOR)(x))
#endif

/*---------------------------------------------------------------------------------------------------------
  Generally useful functions.
---------------------------------------------------------------------------------------------------------*/
// A nice complement to sizeof.
#define elementsof(x) (sizeof(x)/sizeof(x[0]))

// The smallest multiple of b at least as large as a, if b is a power of 2
#define NGL_ALIGN(a, b) (((u_int)(a)+((b)-1))&(~((b)-1)))

#define nglPrefetch(addr,offset) \
  asm volatile("pref 0," #offset "(addr)" : : "r addr" (addr))

#define nglLoadQW(dest,addr,offset) \
  asm volatile("lq dest," #offset "(addr)" : "=r dest" (dest) : "r addr" (addr))

#define nglSetLowQW(result,val) \
  asm volatile("paddw result, val, $0" : "=r result" (result) : "r val" (val))

#define nglStoreQW(val,addr,offset) \
  asm volatile("sq val," #offset "(addr)" : : "r val" (val), "r addr" (addr))

// This is actually the same as casting to int, but works with u_ints too.
inline int nglFTOI( float input )  
{
  register float output;
  __asm__ volatile ("cvt.w.s %0, %1" : "=f" (output) : "f" (input) );
  return (int&)output;
}

// Next power of 2, returns same if already a power of 2.
inline u_int nglNextPow2( u_int v )
{
  v -= 1;
  v |= v >> 16;
  v |= v >> 8;
  v |= v >> 4;
  v |= v >> 2;
  v |= v >> 1;
  return v + 1;
}

// Integer log2 of a float
inline int nglLog2(float x)
{
	u_int ix = (u_int&)x;
	u_int ex = (ix >> 23) & 0xFF;
	return int(ex) - 127;
}

inline long volatile nglGetCPUCycle()
{
  register unsigned long result;
  __asm__ volatile ("mfc0 %0,$9" : "=r" (result));
  return result;
}

/*---------------------------------------------------------------------------------------------------------
  Internal headers.
---------------------------------------------------------------------------------------------------------*/
#include "ngl_instbank.h"
#include "ngl_dma.h"
#include "tim2.h"

#include "matrix.h"
typedef mat_44 matrix_t;
typedef vec_xyzw plane_t;

/*---------------------------------------------------------------------------------------------------------
  Math stuff.
---------------------------------------------------------------------------------------------------------*/
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

enum
{
  NGLFRUSTUM_TOP,
  NGLFRUSTUM_BOTTOM,
  NGLFRUSTUM_LEFT,
  NGLFRUSTUM_RIGHT,
  NGLFRUSTUM_NEAR,
  NGLFRUSTUM_FAR
};

typedef nglVector nglPlane;
struct nglFrustum
{
  // these planes are in world space:
  nglPlane Planes[6];//left, right, back, top, bottom, front;

#ifdef DEBUG
  nglVector wp[8];
#endif
};

// Generalized frustum support.
void nglBuildFrustum(nglFrustum* frustum, nglMatrix& m);
bool nglIsSphereVisible( nglFrustum* Frustum, const nglVector& Center, float Radius );

inline void nglNormalizePlane( nglVector& o, nglVector& p )
{
  float l = sqrtf(p[0]*p[0]+p[1]*p[1]+p[2]*p[2]);
  sceVu0DivVector( o, p, l );
}

inline void nglApplyMatrixPlane( nglVector& Out, nglMatrix& Mat, nglVector& Plane )
{
  nglVector Pt;
  nglVector Norm;

  // Get plane normal and a reference point.
  sceVu0CopyVector( Norm, Plane );
  Norm[3] = 0.0f;
  sceVu0ScaleVectorXYZ( Pt, Norm, Plane[3] );
  Pt[3] = 1.0f;

  // Transform point.
  sceVu0ApplyMatrix( Pt, SCE_MATRIX(Mat), Pt );

  // Reorient normal.
  sceVu0ApplyMatrix( Norm, SCE_MATRIX(Mat), Norm );

  // Reconstruct plane.
  sceVu0CopyVector( Out, Norm );
  Out[3] = sceVu0InnerProduct( Pt, Norm );
}

inline int nglSpheresIntersect( nglVector& Center1, float Radius1, nglVector& Center2, float Radius2 )
{
  nglVector V;
  float DistSqr, Range;

  sceVu0SubVector( V, Center1, Center2 );
  DistSqr = V[0] * V[0] + V[1] * V[1] + V[2] * V[2];

  Range = Radius1 + Radius2;
  return DistSqr <= Range * Range;
}

inline u_int nglVectorToColor( const nglVector& Color )
{
  return NGL_RGBA32( nglFTOI( Color[0] * 255.0f ), nglFTOI( Color[1] * 255.0f ), nglFTOI( Color[2] * 255.0f ), nglFTOI( Color[3] * 255.0f ) );
}

inline void nglColorToVector( nglVector& Dest, u_int Color )
{
  Dest[0] = ( ( Color >> 0  ) & 0xFF ) / 128.0f;
  Dest[1] = ( ( Color >> 8  ) & 0xFF ) / 128.0f;
  Dest[2] = ( ( Color >> 16 ) & 0xFF ) / 128.0f;
  Dest[3] = ( ( Color >> 24 ) & 0xFF ) / 128.0f;
}

// Convert a 'hardware' Z coordinate (0=near 1=far) to GS Z coordinates (NGL_Z_MAX=near 0=far).
u_int nglHardwareZToGS( float Z );

// Converts a view-space Z coordinate into GS Z coordinates.
u_int nglViewZToGS( float Z );

/*---------------------------------------------------------------------------------------------------------
  Render list stuff.
---------------------------------------------------------------------------------------------------------*/
// Render list node types.
enum
{
  NGLNODE_BIN = 0,
  NGLNODE_MESH_SECTION = 1,
  NGLNODE_QUAD = 2,
  NGLNODE_STRING = 3,
  NGLNODE_PARTICLE = 4,
  NGLNODE_SCENE = 5,
  NGLNODE_CUSTOM = 6,
};

struct nglListNode
{
  nglListNode* Next;             // Next node in the render order.
  u_int Type;                    // Node Type.

  // Sorting parameters.
  nglSortInfo SortInfo;

  // Type-specific node data.
  nglCustomNodeFn NodeFn;
  void* NodeData;
};

// Adds a new node to the render list, after sorting it correctly.  This will cause the node to be rendered
// by nglListSend calling NodeFn with the Data pointer while building the Vif1 DMA packet.
void nglListAddNode( u_int Type, nglCustomNodeFn NodeFn, void* Data, nglSortInfo* SortInfo, u_char **Buf = NULL );

// Check a view-space sphere against the view frustum and the guard bands, returns clipping/rejection info.
// -1 = Rejected, 0 = Not Clipped, 1 = Guard band intersection.
//int nglGetClipResult( nglVector Center, float Radius );

// Check a view-space sphere against the guard bands, returns clipping info.
// 0 = Not Clipped, 1 = Guard band intersection.
int nglGetClipResultGBOnly( nglVector& Center, float Radius );

// Check a view-space sphere against the view frustum, returns clipping info.
// -1 = Rejected, 0 = Not Clipped.
int nglGetClipResultViewOnly( nglVector& Center, float Radius );

// Fills out a SortInfo structure for use with nglListAddNode.
void nglGetSortInfo( nglSortInfo* SortInfo, u_int BlendMode, nglTexture* Tex, float Dist );

// Allocates from the render list storage space (which is completely cleared each call to nglListInit).
// Alignment is a bit value, ie 0 for no alignment, 4 for 16 byte alignment.
void* nglListAlloc( u_int Bytes, u_int Alignment = 4 );
#define nglListNew( Type ) (Type*)nglListAlloc( sizeof(Type) )

/*---------------------------------------------------------------------------------------------------------
  Debugging structs/flags
---------------------------------------------------------------------------------------------------------*/
struct nglStageStruct
{
  u_char FSAA;
  u_char DMASend;

  u_char Clip;
  u_char BatchDMA;
  u_char Skin;
  u_char XForm;
  u_char Backface;
  u_char Light;
  u_char Kick;
  u_char PlaneClip;
  u_char CommandList;

  u_char DiffusePass;
  u_char DetailPass;
  u_char EnvironmentPass;
  u_char LightPass;

  u_char ProjectedTexturePass;
  u_char FogPass;
};
extern nglStageStruct nglStage;

struct nglDebugStruct
{
  // Show performance info (FPS/etc)
  u_char ShowPerfInfo;
  u_char ScreenShot;

  u_char DisableQuads;
  u_char DisableMeshes;
  u_char DisableTextures;
  u_char DisableMipmaps;
  u_char DisableVSync;
  u_char DisableScratch;
  u_char DisableMipOpt;

  // Enable printf's throughout the system.
  u_char DebugPrints;
  u_char DumpFrameLog;
  u_char DumpSceneFile;
  u_char DumpTextures;

  // Debugging geometry.
  u_char DrawLightSpheres;
  u_char DrawMeshSpheres;
  u_char DrawNormals;

  // Draw points only (for VU speed testing).
  u_char DrawAsPoints;
  u_char DrawAsLines;

  u_char VUClearMem;         // Set to clear VU1 memory at the start of each batch.
  u_char VUClearRegs;        // Set to zero VU1 registers at the start of each program.
  u_char VUDebugBreak;       // Trigger a breakpoint on VU1.
  u_char VUParticleDebugBreak;    // Trigger a breakpoint on VU1 in the particle code.

  u_char DMARepeatMode;

  u_char DrawToFrontBuffer;
  u_char SyncRender;
};
extern nglDebugStruct nglDebug;
extern nglDebugStruct nglSyncDebug;  // Updated synchronously once per frame.

struct nglPerfInfoStruct
{
  // Curremt frame rate.
  float FPS;

  // Cycle counters.
  u_int RenderStart;
  u_int RenderFinish;

  // Millisecond counters.
  float TotalMS;                      // Total millseconds since nglInit.
  float RenderMS;                     // Milliseconds taken by the Async Render pipe (DMA/VIF/VU/GS).
  float FrameMS;                      // Milliseconds between calls to nglFlip.
  float ListSendMS;                   // Milliseconds (EE time) taken by nglListSend.
  u_int ParticleCount;                // Total particles.

  u_int ListSendCycles;               // Cycles taken by nglListSend this frame.
  u_int ListSendMisses;               // Cache misses in nglListSend this frame.

  float TotalSeconds;                 // Total seconds since nglInit.  Used by the cheezy automatic animation code.

  // Misc counters.
  u_int TotalPolys;                   // Triangles rendered this frame.
  u_int TotalVerts;                   // Vertices rendered this frame.

  u_int TextureDataStreamed;          // Bytes of texture data streamed to the GS this frame.

  u_int BakedTextureDMASize;
  u_int BakedMaterialDMASize;

  u_int ListWorkBytesUsed;            // Bytes of nglListWork used this frame.
  u_int Vif1WorkBytesUsed;            // Bytes of nglVif1PacketWork used this frame.
  u_int GifWorkBytesUsed;             // Bytes of nglVif1PacketWork used this frame.
  u_int ScratchMeshBytesUsed;         // Bytes of nglScratchMeshWorkTemp used this frame.

  u_int MaxListWorkBytesUsed;         // Highest recorded values of the above.
  u_int MaxVif1WorkBytesUsed;         // 
  u_int MaxGifWorkBytesUsed;          // 
  u_int MaxScratchMeshBytesUsed;      // 
};
extern nglPerfInfoStruct nglPerfInfo;

/*---------------------------------------------------------------------------------------------------------
  Debug rendering stuff
---------------------------------------------------------------------------------------------------------*/
// Draws a standard sphere mesh with specified scale and color.  If Solid is true, a sold sphere is rendered,
// otherwise just the outline is rendered (all three axes, not just a circle).
void nglDrawDebugSphere( const nglVector& Pos, float Scale, u_int Color, bool Solid );

/*---------------------------------------------------------------------------------------------------------
  Scene/node stuff.
---------------------------------------------------------------------------------------------------------*/
// Bins for fast sorting by material or Z.
// NGL_OPAQUE_BINS -must- be a power of 2 plus 1! (don't ask ;))
#define NGL_OPAQUE_BINS         129
#define NGL_TRANSLUCENT_BINS    0
#define NGL_TOTAL_BINS ( NGL_OPAQUE_BINS + NGL_TRANSLUCENT_BINS )

// Made to look like a nglListNode.
struct nglListBin
{
  nglListNode* Next;             // Next node in the render order.
  u_int Type;
};

struct nglScene
{
  // Scene hierarchy info.
  nglScene* Parent;
  nglScene* NextSibling;
  nglScene* FirstChild;
  nglScene* LastChild;

  nglTexture* RenderTarget;   // Usually nglBackBufferTex, otherwise a texture for the scene to be rendered into.
  bool Download;              // If true, the result of rendering is downloaded into a system memory texture.

  // Array of textures required to be present when a streamed RenderTarget is allocated.
  // Used to ensure a streamed RenderTarget will not be flushed from video ram
  // while needed, due to references to any of these textures.
  // Note: Do not include duplicates or the RenderTarget itself.
  // Defaults to empty list.
  nglTexture** RequiredTextures;
  u_int NRequiredTextures;

  // Global matrices.
  nglMatrix ViewToWorld;    // Camera -> World

  nglMatrix ViewToScreen;   // Camera -> Screen
  nglMatrix ViewToClip;     // Camera -> Clip
  nglMatrix ClipToScreen;   // Clip -> Screen

  nglMatrix WorldToView;    // World -> Camera
  nglMatrix WorldToScreen;  // World -> Camera -> Screen
  nglMatrix WorldToClip;    // World -> Camera -> Clip

  nglMatrix ViewToScreenNoFTOI;   // Camera -> Screen w/big number multiplied in.
  nglMatrix WorldToScreenNoFTOI;  // World -> Camera -> Screen  w/big number multiplied in.

  point_t ViewPos;  // Position of the camera in world space.
  vector_t ViewDir;  // Direction of the camera in world space.

  nglVector ClipPlanes[NGLCLIP_MAX];     // Planes for sphere rejection.
  nglVector GBClipPlanes[NGLCLIP_MAX];   // Planes for guard band clipping.

  nglVector WorldClipPlanes[NGLCLIP_MAX];     // Planes for sphere rejection in world space.
  nglVector WorldGBClipPlanes[NGLCLIP_MAX];   // Planes for guard band clipping in world space.

  nglListNode* OpaqueRenderList;
  nglListNode* TransRenderList;
  u_int OpaqueListCount, TransListCount;

  // Viewport.
  u_int ViewX1, ViewY1, ViewX2, ViewY2;

  // Screen clear settings.
  u_int ClearFlags;
  float ClearZ;
  sceVu0IVECTOR ClearColor;

  u_int FBWriteMask;

  // Z Write & Test enable settings
  bool ZWriteEnable;
  //HACK ALERT - For now, only applies to Quads and Strings.
  //  This is because material baking of meshes would freeze ztest enable to whatever
  //  ZTestEnable was set to for the current scene when the material was baked.
  //  For now Quads and Strings are sufficient, as this feature is initially intended to facilitate
  //  rendering of scenes with quads and strings in front of a previously rendered 3d scenes.
  bool ZTestEnable;

  // Fog settings.
  float FogNear, FogFar;
  float FogMin, FogMax;
  nglVector FogColor;

  // Projection parameters.
  float HFOV;
  float CX;
  float CY;
  float NearZ;
  float FarZ;
  float InvFarZMinusNearZ;
  float ZMin;
  float ZMax;
  bool UsingOrtho;

  int RenderW;
  int RenderH;

  float AnimTime;
  u_int IFLFrame;

  float ScrZ;

  // Arbitrary cached stuff (for the rendering pipeline).
  struct
  {
    bool WriteFBA;
  } Cache;
};

// Calls outside nglListInit/nglListSend affect the global scene.
extern nglScene nglDefaultScene;

// Currently active scene.
extern nglScene* nglCurScene;

/*---------------------------------------------------------------------------------------------------------
  PS2MESH file stuff.
---------------------------------------------------------------------------------------------------------*/
#define NGL_MESHFILE_VERSION  0x607
#define NGL_MAX_PATH_LENGTH   256

// Mesh File structure, these are contained in nglMeshFileBank.
struct nglMeshFile
{
  nglFixedString FileName;
  char FilePath[NGL_MAX_PATH_LENGTH];

  nglFileBuf FileBuf;
  bool LoadedInPlace;

  nglMesh* FirstMesh;
};
extern nglInstanceBank nglMeshFileBank;

struct nglMaterialInfo
{
  // Biggest hash of all the textures.
  u_int Hash;

  // Location of the GS registers for each pass in VU mem.
  u_int DiffuseKickAddr;
  u_int DetailKickAddr;
  u_int EnvironmentKickAddr;
  u_int LightKickAddr;
  u_int CmdListEndAddr;
  u_int* BakedDMA;
};

// Batch Info structure.  The nglMeshSection structure has an array of these, one for each batch.  They can be used
// (and are used by the Mesh Editing API) to access the vertex data of a mesh.
struct nglMeshBatchInfo
{
  // Basic statistical/processing information.
  u_int NVerts;

  // Vertex data pointers.
  u_int* StripData;
  float* PosData;
  float* UVData;
  u_int* ColorData;
  float* NormData;
  float* LightUVData;
  char* BoneCountData;
  unsigned short* BoneIdxData;
  float* BoneWeightData;
};

// Instance bank for meshes.  Use these to insert and delete resources.
extern nglInstanceBank nglMeshBank;

// Current animation frame for animated textures.  Warning: This global is sometimes used as a parameter.
extern int nglTextureAnimFrame;

// Debugging flags to optionally turn off and on material flags.
extern u_int nglMaterialFlagsAnd;
extern u_int nglMaterialFlagsOr;

// Change all the internal pointers in a mesh structure to be based at a new location.
// Useful for copying meshes around and for loading them from files (in which they're based at 0).
void nglRebaseMesh( u_int NewBase, u_int OldBase, nglMesh* Mesh, bool Scratch = false );

/*---------------------------------------------------------------------------------------------------------
  Lighting stuff.
---------------------------------------------------------------------------------------------------------*/
typedef void (*nglCustomLightFn)( u_int*& Packet, void* NodeData );

struct nglLightNode
{
  // Next light in scene for each lighting category the node is in.
  nglLightNode* Next[NGL_NUM_LIGHTCATS];
  nglLightNode* LocalNext[NGL_NUM_LIGHTCATS];

  u_int LightCat;   // Lighting category flags.
  u_int Type;

  nglCustomLightFn NodeFn;  // Call back function apply the light.
  void* NodeData;   // Copy of the light structure.
};

struct nglLightContext
{
  // Head node for the list of all lights in this context.
  nglLightNode VertexHead;

  // Head node for the list of projector lights in this context.
  nglLightNode TextureHead;

  // Empty boolean's for both local l)ight lists.
  bool LocalVertexListEmpty;
  bool LocalTextureListEmpty;

  // Ambient color for the context.
  nglVector Ambient;
};

extern nglLightContext* nglGlobalLightContext;
extern nglLightContext* nglCurLightContext;

/*---------------------------------------------------------------------------------------------------------
  Command list stuff.
---------------------------------------------------------------------------------------------------------*/
inline void nglVif1AddCommandListProgram( u_int*& Packet, u_int& DataPtr, char* Addr );
inline void nglVif1AddCommandListData( u_int*& Packet, u_int& DataPtr, int Mode, int QWC, void* Data, int DataSize );
inline void nglVif1AddCommandListEnd( u_int*& Packet, u_int& DataPtr );

/*---------------------------------------------------------------------------------------------------------
  GS related stuff.
---------------------------------------------------------------------------------------------------------*/
// Cache of the current state of all GS registers used by NGL.
enum nglGSRegType
{
  NGL_GS_FOG,
  NGL_GS_PRMODE,
  NGL_GS_TEX0_1,
  NGL_GS_TEX1_1,
  NGL_GS_TEXCLUT,
  NGL_GS_MIPTBP1_1,
  NGL_GS_MIPTBP2_1,
  NGL_GS_CLAMP_1,
  NGL_GS_TEXA,
  NGL_GS_ALPHA_1,
  NGL_GS_TEST_1,
  NGL_GS_PABE,
  NGL_GS_RGBAQ,
  NGL_GS_NUM_REGS
};

inline void nglVif1ResetGSRegisterCache();

// This version takes NGL GS register indices and uses the register cache to see if an update is really neccessary.
inline void nglVif1AddGSReg( nglGSRegType Reg, u_long Val );

// This version takes Sony GS register indices and doesn't use the register cache.
inline void nglVif1AddRawGSReg( u_int Reg, u_long Val );

// Sets the GS ALPHA and TEST registers based on a BlendMode ID and Constant.
// ForceZ forces Z writes.
// (ForceZ is currently not implemented correctly for punchthrough modes...
//  it disables the punchthrough test, making it appear always true)
inline void nglVif1AddSetBlendMode( int Mode, int Fix, bool ForceZ );

// Sets the GS ALPHA and TEST registers based on a BlendMode ID and Constant.
// Also disables Z testing if nglCurScene->ZTestEnable is false.
inline void nglVif1AddSetBlendModeCheckZTest( int Mode, int Fix = 0, bool ForceZ = false );

// This adds the appropriate GS registers to set up the GS for a texture.
void nglVif1AddSetTexture( nglTexture* Tex, u_int Flags, float MipRatio );

// These functions prepare an A/D packet for embedding in Path2 of a VIF1 DMA chain (using the DIRECT opcode).
inline void nglVif1StartDirectGifAD();
inline void nglVif1EndDirectGifAD( u_int*& Packet );

// These functions create a material kick command list out of GS register adds.
inline void nglVif1StartMaterialKick();
inline void nglVif1EndMaterialKick( u_int*& Packet, u_int& DataPtr );

// Instance bank for textures.  Use these to add or delete resources.
extern nglInstanceBank nglTextureBank;

// Global textures for referencing the front and back buffers w/o calling nglGetTexture.
extern nglTexture nglFrontBufferTex;
extern nglTexture nglBackBufferTex;

// Builds DMA packets to upload texture data.
// References PSM and Data fields of GsImage[] and GsClut.
// Also references Width, Height, MipMapTextures, and ClutColors.
// Fills in DMA, TBW, and Offset fields of GsImage[] and GsClut.  Sets GsSize.
void nglBakeTexture( nglTexture* Tex);

/*---------------------------------------------------------------------------------------------------------
  VIF1 Quad API.
---------------------------------------------------------------------------------------------------------*/
// Draws a fast rectangle using 32pixel wide vertical strips for page buffer coherency.
void nglVif1AddFastRect( u_int*& Packet, nglVector& Rect, u_int Z, sceVu0IVECTOR Color );

void nglVif1StartQuads( u_int*& Packet );
void nglVif1EndQuads( u_int*& Packet );

void nglVif1AddQuadMaterial( nglQuad* Quad );
void nglVif1AddQuadVerts( nglQuad* Quad );

/*---------------------------------------------------------------------------------------------------------
  DMA functions.
---------------------------------------------------------------------------------------------------------*/
// Opens a new DMA CNT tag.  An empty space is left in Packet and saved in nglDmaTagPtr, to be filled in later by nglDmaEndTag.
// Packet must be aligned to a 16 byte boundary.
inline void nglDmaStartTag( u_int*& Packet );

// Closes a DMA tag.  Calculates the Tag QWC from the difference between the current Packet and nglDmaTagPtr.
// Valid types are SCE_DMA_ID_CNT, SCE_DMA_ID_END, and SCE_DMA_ID_CALL (set Addr to the call address).
inline void nglDmaEndTag( u_int*& Packet, u_int Type, void* Addr = 0 );

// Adds a new DMA REF tag to Packet.  This function can break up transfers that are too large (>512k) into multiple transfers.
inline void nglDmaAddRef( u_int*& Packet, void* Addr, u_int QWC );

// Adds a new DMA CALL tag to Packet.
inline void nglDmaAddCall( u_int*& Packet, void* Addr );

/*---------------------------------------------------------------------------------------------------------
  VIF functions.
---------------------------------------------------------------------------------------------------------*/
inline void nglVif1InitPacket( u_int*& Packet );
inline void nglVif1ClosePacket( u_int*& Packet );

// Flush the Scratchpad buffer.  If NGL_USE_SCRATCHPAD is defined, all Vif1 packet operations occur on the scratchpad.
// Call this function before writing more than 16k of data to DMA the data on the scratchpad into main memory.
void nglVif1FlushSPAD(u_int*& ScratchPacket, bool Force = false );

// Stall until the Vif1 DMA is complete, with a built-in timeout.
void nglVif1SafeWait();

// Unpack an arbitrary amount of data to a location in VU memory.  If the DataSize is not a multiple of 4 bytes,
// extra data may be copied (but will be ignored by the VIF).
inline void nglVif1AddUnpack( u_int*& Packet, u_int Offset, u_int Mode, u_int QWC, void* Data, int DataSize );

// Adds a 16 byte alignment maintaining UNPACK opcode (no data is copied).
inline void nglVif1AddUnpackAligned( u_int*& Packet, u_int Addr, u_int QWC, u_int Mode );

// Add a call to some VU code to the VIF chain.
#define NGL_PROG_ADDR( x, base ) ( ( (u_int)x - (u_int)base ) / 8 )
inline void nglVif1AddCallProgram( u_int*& Packet, u_int Addr );

// Various types of flushes (aka stalls on the VU, GS, GIF, etc).
inline void nglVif1AddFlush( u_int*& Packet );
inline void nglVif1AddFlushE( u_int*& Packet );
inline void nglVif1AddFlushA( u_int*& Packet );

// Optimized code to unpack a matrix into VU memory.  Requires Packet to be 16 byte aligned.
inline void nglVif1UnpackMatrix( u_int*& Packet, u_int Addr, const nglMatrix* Matrix );

// unpack the bones into VU1 memory
void nglVif1UnpackBones( u_int*& Packet, nglMesh* Mesh, u_int Flags, nglMatrix* Bones, nglMatrix* WorldToLocal = NULL );

/*---------------------------------------------------------------------------------------------------------
  Texture streaming related functions.
---------------------------------------------------------------------------------------------------------*/
// Returns false if the texture was just too damn large.
// Does not modify contents of supplied Textures array
bool nglVif1AddTextureStreaming( u_int*& Packet, nglTexture** Textures, u_int NTextures );

inline bool nglVif1AddSingleTextureStreaming( u_int*& Packet, nglTexture *Texture );

extern u_int nglLockBufferSize;

/*---------------------------------------------------------------------------------------------------------
  VIF1 Interrupt Handler related functions.
---------------------------------------------------------------------------------------------------------*/
void* nglVif1IntAlloc( u_int Size, u_int Align );
void nglVif1IntAddCallback( u_int*& Packet, nglCustomNodeFn CallbackFn, void* Data );

/*---------------------------------------------------------------------------------------------------------
  Primitive VU code caching.
---------------------------------------------------------------------------------------------------------*/
enum
{
  NGL_VUCODE_NONE,
  NGL_VUCODE_MESH,
  NGL_VUCODE_PART,
};
extern u_int nglLastVUCodeDma;

/*---------------------------------------------------------------------------------------------------------
  NGL internal constants.
---------------------------------------------------------------------------------------------------------*/
// String constants for TTY color control codes.
#define TTY_BLACK   "\x1b\x1e"
#define TTY_RED     "\x1b\x1f"
#define TTY_GREEN   "\x1b\x20"
#define TTY_YELLOW  "\x1b\x21"
#define TTY_BLUE    "\x1b\x22"
#define TTY_MAGENTA "\x1b\x23"
#define TTY_CYAN    "\x1b\x24"
#define TTY_LTGREY  "\x1b\x25\x25"

#define TTY_BG_BLACK    "\x1b\x28"
#define TTY_BG_RED      "\x1b\x29"
#define TTY_BG_GREEN    "\x1b\x2A"
#define TTY_BG_DKYELLOW "\x1b\x2B"
#define TTY_BG_BLUE     "\x1b\x2C"
#define TTY_BG_MAGENTA  "\x1b\x2D"
#define TTY_BG_CYAN     "\x1b\x2E"
#define TTY_BG_BKGRND   "\x1b\x2F"

// DEPRICATED -- use nglDisplayWidth and nglDisplayHeight instead
//#define NGL_SCREEN_WIDTH      512
//#define NGL_SCREEN_HEIGHT     448
extern u_int nglDisplayWidth;
extern u_int nglDisplayHeight;

//#define NGL_FRAME_FORMAT     SCE_GS_PSMCT16S
//#define NGL_FRAME_PIXELSIZE  2
#define NGL_FRAME_FORMAT      SCE_GS_PSMCT32
#define NGL_FRAME_PIXELSIZE   4
//#define NGL_FRAME_FORMAT      SCE_GS_PSMCT24
//#define NGL_FRAME_PIXELSIZE   4

// 32 bit z buffer
//#define NGL_Z_FORMAT          SCE_GS_PSMZ32
//#define NGL_Z_PIXELSIZE       4
//#define NGL_Z_MAX             4294967295.0f

// 24 bit z buffer
#define NGL_Z_FORMAT          SCE_GS_PSMZ24
#define NGL_Z_PIXELSIZE       4
#define NGL_Z_MAX             16777215.0f

// 16 bit z buffer
//#define NGL_Z_FORMAT          SCE_GS_PSMZ16S
//#define NGL_Z_PIXELSIZE       2
//#define NGL_Z_MAX             65535.0f

#define NGL_MAX_BONES         64

// VRAM addresses are in blocks (256bytes)
#define NGL_VRAM_END            16384

#define NGL_VRAM_FRAME_SIZE     ( ( nglDisplayWidth * nglDisplayHeight * NGL_FRAME_PIXELSIZE ) / 256 ) // 3584
#define NGL_VRAM_FRAME1         NGL_VRAM_FRAME_SIZE // 3584
#define NGL_VRAM_FRAME2         ( NGL_VRAM_FRAME_SIZE * 2 ) //7168
#define NGL_VRAM_ZBUFFER        0

#define NGL_VRAM_LOCKED_START   ( NGL_VRAM_FRAME_SIZE * 3 ) // 10752
#define NGL_VRAM_LOCKED_END     ( NGL_VRAM_LOCKED_START + nglLockBufferSize )

#define NGL_VRAM_STREAM_START   NGL_VRAM_LOCKED_END
#define NGL_VRAM_STREAM_END     NGL_VRAM_END
#define NGL_VRAM_STREAM_SIZE	  ( NGL_VRAM_STREAM_END - NGL_VRAM_LOCKED_END  )

#define NGL_MAX_MATERIAL_TEXTURES 12  // maximum number of textures that can be applied to a material - includes projectors.

#define NGL_CPU_CLOCKS_PER_MS 294912.0f

// Things that really should be in the PS2 include files.
#define SCE_VIF1_USN ( 1 << 14 )
#define SCE_VIF1_FLG ( 1 << 15 )

#define SCE_VIF1_MASK_ENABLE 0x10

#define SCE_VIF1_S_32   0x00
#define SCE_VIF1_S_16   0x01
#define SCE_VIF1_S_8    0x02

#define SCE_VIF1_V2_32  0x04
#define SCE_VIF1_V2_16  0x05
#define SCE_VIF1_V2_8   0x06

#define SCE_VIF1_V3_32  0x08
#define SCE_VIF1_V3_16  0x09
#define SCE_VIF1_V3_8   0x0a

#define SCE_VIF1_V4_32  0x0c
#define SCE_VIF1_V4_16  0x0d
#define SCE_VIF1_V4_8   0x0e
#define SCE_VIF1_V4_5   0x0f

/*---------------------------------------------------------------------------------------------------------
  VU1 related constants.
---------------------------------------------------------------------------------------------------------*/
#include "ngl_vudefs.h"

/*---------------------------------------------------------------------------------------------------------
  VU Microcode addresses for ngl_vu1.dsm.
---------------------------------------------------------------------------------------------------------*/
extern char nglLoadMicrocode[];
extern char nglLoadParticleMicrocode[];

extern char nglBaseAddr[];
extern char nglEndAddr[];

extern char nglNodeSetupAddr[];                             // Does setup per node, matrix multiplies etc.
extern char nglLoadIdentityAddr[];                          // Loads an identitiy matrix into NGLMEM_LOCAL_TO_WORLD.
extern char nglNoCommandListAddr[];                         // Does nothing in place of running the command list.
extern char nglCommandListAddr[];                           // Runs a list of other programs.  This is the backbone of the VU renderer.
extern char nglCommandListCallAddr[];                       // Calls another command list, saving the current command list position for later.
extern char nglKickAddr[];                                  // Kicks transformed verts to the GS.
extern char nglMaterialKickAddr[];                          // Kicks data from the command list.
extern char nglDummyKickAddr[];                             // Executes a dummy kick to stall on the GS.
extern char nglLoadMatrixAddr[];							// Loads a Matrix as the Local to World Matrix
extern char nglKickAndLoadMatrixAddr[];						// Kicks the verts and Loads a Matrix as the Local to World Matrix

extern char nglBackfaceCullScreenAddr[];                    // Cull polygons facing away from the camera (uses cross product).
extern char nglFogAddr[];                                   // Apply fog to transformed verts..
extern char nglTransformAddr[];                             // Transform.
extern char nglTransformClipAddr[];                         // Transform w/rejection clipping.
extern char nglTransformFrustumClipAddr[];                  // Transform w/rejection clipping.  Stores some info for the frustum clipper.
extern char nglSkinAddr[];                                  // Weighted mesh stuff applied to source vertices.
extern char nglFrustumClipAddr[];                           // Sets clip bits and kicks scissored triangles.

extern char nglDetailMapAddr[];                             // Generate detail ST coordinates.
extern char nglSpecularAddr[];                              // Generate specular highlight coordinates.
extern char nglProjectedTextureAddr[];                      // Generate projected ST coordinates.
extern char nglPointProjectedTextureAddr[];                 // Generate projected ST coordinates from a point source.
extern char nglEnvironmentMapSphereAddr[];                  // Generate spherical environment map ST coordinates.
extern char nglEnvironmentMapCylinderAddr[];                // Generate cylindrical environment map ST coordinates.
extern char nglLightMapAddr[];                              // Generate light map ST coordinates.
extern char nglWriteQAddr[];                                // Set default (0,0,1) STQ coordinates.

extern char nglTextureScrollAddr[];                         // Scroll texture UV coordinates.

extern char nglDirLightAddr[];                              // Applies a directional light w/no falloff.
extern char nglPointLightAddr[];                            // Applies a point light with true falloff.
extern char nglTintAddr[];                                  // Applies a color.
extern char nglAlphaFalloffAddr[];                          // Applies a directional light from the camera that affects alpha.
extern char nglMatColorAddr[];                              // Stores a constant color in all vertex colors.

extern char nglSkinOneBone[];                               // Used for jump table access.
extern char nglSkinTwoBones[];                              // ...
extern char nglSkinThreeBones[];                            // ...
extern char nglSkinFourBones[];                             // ...

extern char nglZBiasAddr[];                                 // Adds a fixed value to Z coordinates, without affecting drawing.

extern char nglParticleBaseAddr[];
extern char nglParticleEndAddr[];
extern char nglDebugParticleBreakAddr[];                    // Debug version of the particle system generator program.
extern char nglParticleAddr[];                              // Particle system generator program.

/*---------------------------------------------------------------------------------------------------------
  Math function inline implementation.
---------------------------------------------------------------------------------------------------------*/
inline u_int nglHardwareZToGS( float Z )
{
  return nglFTOI( ( 1.0f - Z ) * NGL_Z_MAX ) + 1;
}

inline u_int nglViewZToGS( float Z )
{
  float _Z = Z;
  if ( _Z < nglCurScene->NearZ )
    _Z = nglCurScene->NearZ;
  if ( _Z > nglCurScene->FarZ )
    _Z = nglCurScene->FarZ;
  nglVector V( 0.0f, 0.0f, _Z, 1.0f );
  sceVu0ApplyMatrix( V, SCE_MATRIX(nglCurScene->ViewToScreen), V );
  V[2] /= V[3];
  return nglFTOI( V[2] * 16.0f );
//  return nglFTOI( ( nglCurScene->ZMin * NGL_Z_MAX ) + ( ( nglCurScene->ZMax - nglCurScene->ZMin ) * NGL_Z_MAX ) / ( nglCurScene->NearZ - nglCurScene->FarZ ) * ( _Z - nglCurScene->FarZ ) );
}

inline u_int nglScreenXToGS( float X )
{
  return (int)( ( X + 2048 - ( nglCurScene->RenderTarget->Width / 2 ) ) * 16 );
}

extern u_int nglTVMode;

inline u_int nglScreenYToGS( float Y )
{
  // See the comment by NGL_EMULATE_480_LINES
#if NGL_EMULATE_480_LINES
	float YOfs = nglTVMode == NGLTV_PAL ? 0.0f : -16.0f;
#else
	float YOfs = 0.0f;
#endif
  return (int)( ( Y + 2048 + YOfs - ( nglCurScene->RenderTarget->Height / 2 ) ) * 16 );
}

/*---------------------------------------------------------------------------------------------------------
  Render list function inline implementation.
---------------------------------------------------------------------------------------------------------*/
inline int nglGetClipResult( nglVector& Center, float Radius )
{
  u_int Ret = 0;

  for ( int i = 0; i < NGLCLIP_MAX; i++ )
  {
    // If the entire sphere is behind any plane, reject.
    float Dist = sceVu0InnerProduct( Center, nglCurScene->WorldClipPlanes[i] ) - nglCurScene->WorldClipPlanes[i][3];
    if ( Dist + Radius < 0 )
      return -1;

    Dist = sceVu0InnerProduct( Center, nglCurScene->WorldGBClipPlanes[i] ) - nglCurScene->WorldGBClipPlanes[i][3];
    if ( Dist < Radius )
      Ret = 1;
  }

  return Ret;
}

inline int nglGetClipResultGBOnly( nglVector& Center, float Radius )
{
  for ( int i = 0; i < NGLCLIP_MAX; i++ )
  {
    float Dist = sceVu0InnerProduct( Center, nglCurScene->WorldGBClipPlanes[i] ) - nglCurScene->WorldGBClipPlanes[i][3];
    if ( Dist < Radius )
      return 1;
  }

  return 0;
}

inline int nglGetClipResultViewOnly( nglVector& Center, float Radius )
{
  for ( int i = 0; i < NGLCLIP_MAX; i++ )
  {
    // If the entire sphere is behind any plane, reject.
    float Dist = sceVu0InnerProduct( Center, nglCurScene->WorldClipPlanes[i] ) - nglCurScene->WorldClipPlanes[i][3];
    if ( Dist + Radius < 0 )
      return -1;
  }

  return 0;
}

inline void nglGetSortInfo( nglSortInfo* SortInfo, u_int BlendMode, nglTexture* Tex, float Dist )
{
  if ( BlendMode == NGLBM_OPAQUE || BlendMode == NGLBM_PUNCHTHROUGH )
  {
    SortInfo->Type = NGLSORT_TEXTURED;
    if ( Tex )
      SortInfo->Hash = Tex->Hash;
    else
      SortInfo->Hash = 0;
  }
  else
  {
    SortInfo->Type = NGLSORT_TRANSLUCENT;
    SortInfo->Dist = Dist;
  }
}

/*---------------------------------------------------------------------------------------------------------
  DMA functions inline implementation.
---------------------------------------------------------------------------------------------------------*/
extern u_long* nglDmaTagPtr;

inline void nglDmaStartTag( u_int*& Packet )
{
  nglDmaTagPtr = (u_long*)Packet;
  Packet += sizeof(u_long128) / sizeof(u_int);
}

inline void nglDmaEndTag( u_int*& Packet, u_int Type, void* Addr = 0 )
{
  while ( (u_int)Packet & 15 ) *(Packet++) = SCE_VIF1_SET_NOP( 0 );
  nglDmaTagPtr[0] = SCE_DMA_SET_TAG( ( (u_int)Packet - (u_int)nglDmaTagPtr ) / 16 - 1, 0, Type, 0, Addr, 0 );
  nglDmaTagPtr[1] = 0;
}

inline void nglDmaAddRef( u_int*& Packet, void* Addr, u_int QWC )
{
#ifdef NGL_SAFE_DMA
  if ( (u_int)Addr + QWC * 16 >= 32 * 1024 * 1024 )
	  nglWarning( "DMA REF addr > 32Mb (addr=%x qwc=%x)\n", Addr, QWC );
  else
#endif
  {
    NGL_ASSERT( QWC < NGL_MAX_DMA_SIZE, "Tried to DMA over 512k.  Use nglDmaAddLargeRef instead." );
    register u_long128 Tag;
    nglSetLowQW( Tag, SCE_DMA_SET_TAG( QWC, 0, SCE_DMA_ID_REF, 0, Addr, 0 ) );
    nglStoreQW( Tag, Packet, 0 );
    Packet += 4;
  }
}

// Slower version that can handle chunks >512k.
inline void nglDmaAddLargeRef( u_int*& Packet, void* Addr, u_int QWC )
{
#ifdef NGL_SAFE_DMA
  if ( (u_int)Addr + QWC * 16 >= 32 * 1024 * 1024 )
	  nglWarning( "DMA REF addr > 32Mb (addr=%x qwc=%x)\n", Addr, QWC );
  else
#endif
  {
    register u_int QWCLeft = QWC;
    register u_int* Pos = (u_int*)Addr;
    while ( QWCLeft )
    {
      u_int Size = QWCLeft >= NGL_MAX_DMA_SIZE ? NGL_MAX_DMA_SIZE - 1 : QWCLeft;
      u_long128 Tag;
      nglSetLowQW( Tag, SCE_DMA_SET_TAG( Size, 0, SCE_DMA_ID_REF, 0, Pos, 0 ) );
      nglStoreQW( Tag, Packet, 0 );
      Packet += 4;
      Pos += Size * 4;
      QWCLeft -= Size;
    }
  }
}

inline void nglDmaAddCall( u_int*& Packet, void* Addr )
{
#ifdef NGL_SAFE_DMA
  if ( (u_int)Addr + QWC * 16 >= 32 * 1024 * 1024 )
	  nglWarning( "DMA CALL addr > 32Mb (addr=%x qwc=%x)\n", Addr, QWC );
  else
#endif
  {
    register u_long128 Tag;
    nglSetLowQW( Tag, SCE_DMA_SET_TAG( 0, 0, SCE_DMA_ID_CALL, 0, Addr, 0 ) );
    nglStoreQW( Tag, Packet, 0 );
    Packet += 4;
  }
}

/*---------------------------------------------------------------------------------------------------------
  VIF functions inline implementation.
---------------------------------------------------------------------------------------------------------*/
extern u_int* nglVif1Packet;
extern u_char* nglVif1PacketWork;

inline void nglVif1InitPacket( u_int*& Packet )
{
  // This can help narrow down weird bugs.
//  memset( nglVif1PacketWork, 0, sizeof(nglVif1PacketWork) );
  nglVif1Packet = (u_int*)nglVif1PacketWork;
#if NGL_USE_SCRATCHPAD
  Packet = (u_int*)NGL_SCRATCHPAD_MEM;
#else
  Packet = nglVif1Packet;
#endif
}

inline void nglVif1ClosePacket( u_int*& Packet )
{
  nglVif1FlushSPAD( Packet, true );
#if NGL_USE_SCRATCHPAD
  Packet = nglVif1Packet;
#endif
}

inline void nglVif1AddUnpack( u_int*& Packet, u_int Offset, u_int Mode, u_int QWC, void* Data, int DataSize )
{
  u_int* DataWords = (u_int*)Data;
  *(Packet++) = SCE_VIF1_SET_UNPACK( Offset, QWC, Mode, 0 ) | SCE_VIF1_USN;
  do {
    *(Packet++) = *(DataWords++);
    DataSize -= 4;
  } while ( DataSize > 0 );
}

inline void nglVif1AddCallProgram( u_int*& Packet, u_int Addr )
{
  *(Packet++) = SCE_VIF1_SET_MSCAL( Addr, 0 );
}

inline void nglVif1AddFlush( u_int*& Packet )
{
  *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
}

inline void nglVif1AddFlushE( u_int*& Packet )
{
  *(Packet++) = SCE_VIF1_SET_FLUSHE( 0 );
}

inline void nglVif1AddFlushA( u_int*& Packet )
{
  *(Packet++) = SCE_VIF1_SET_FLUSHA( 0 );
}

// Adds a 16 byte alignment maintaining UNPACK opcode (no data is copied).
inline void nglVif1AddUnpackAligned( u_int*& Packet, u_int Addr, u_int QWC, u_int Mode )
{
  NGL_ASSERT( ( (u_int)Packet & 15 ) == 0, "16 byte packet alignment required." );
  register u_long128 UnpackQW; // NOP NOP NOP UNPACK
  register u_long Opcode = (u_long)SCE_VIF1_SET_UNPACK( Addr, QWC, Mode, 0 ) << 32;
  asm ( "pextlw UnpackQW, Opcode, $0\n" : "=r UnpackQW" (UnpackQW) : "r Opcode" (Opcode) );
  nglStoreQW( UnpackQW, Packet, 0 );
  Packet += 4;
}

inline void nglVif1UnpackMatrix( u_int*& Packet, u_int Addr, const nglMatrix* Matrix )
{
  nglVif1AddUnpackAligned( Packet, Addr, 4, SCE_VIF1_V4_32 );
  nglStoreQW( ( (u_long128*)Matrix )[0], Packet, 0 );
  nglStoreQW( ( (u_long128*)Matrix )[1], Packet, 16 );
  nglStoreQW( ( (u_long128*)Matrix )[2], Packet, 32 );
  nglStoreQW( ( (u_long128*)Matrix )[3], Packet, 48 );
  Packet += sizeof(nglMatrix) / sizeof(u_int);
}

/*---------------------------------------------------------------------------------------------------------
  Command list functions inline implementation.
---------------------------------------------------------------------------------------------------------*/
inline void nglVif1AddCommandListProgram( u_int*& Packet, u_int& DataPtr, char* Addr )
{
  Packet[0] = SCE_VIF1_SET_UNPACK( DataPtr++, 1, SCE_VIF1_S_32, 0 );
  Packet[1] = NGL_PROG_ADDR( Addr, nglBaseAddr );
  Packet += 2;
}

inline void nglVif1AddCommandListData( u_int*& Packet, u_int& DataPtr, int Mode, int QWC, void* Data, int DataSize )
{
  u_int* DataWords = (u_int*)Data;
  *(Packet++) = SCE_VIF1_SET_UNPACK( DataPtr, QWC, Mode, 0 ) | SCE_VIF1_USN;
  do {
    *(Packet++) = *(DataWords++);
    DataSize -= 4;
  } while ( DataSize );
  DataPtr += QWC;
}

inline void nglVif1AddCommandListEnd( u_int*& Packet, u_int& DataPtr )
{
  *(Packet++) = SCE_VIF1_SET_UNPACK( DataPtr++, 1, SCE_VIF1_S_32, 0 );
  *(Packet++) = 0;  // 0 signals the end of the command list.
}

#if 0
/*---------------------------------------------------------------------------------------------------------
  Command list functions inline implementation.
---------------------------------------------------------------------------------------------------------*/
struct nglCmdListInfoStruct
{
  u_int CmdCount;
  u_int DataPtr;
  u_char ListOfs[NGLCMD_NUM_LISTS * 2];
  u_int Pad;  // To make sure CmdList ends up on a 4 byte boundary.
  u_short CmdList[128];
};

extern nglCmdListInfoStruct nglCmdListInfoData;
nglCmdListInfoStruct* const nglCmdListInfo = &nglCmdListInfoData;

extern bool nglFatalDMAError;

inline void nglVif1InitCommandLists()
{
  nglCmdListInfo->CmdCount = 0;
  nglCmdListInfo->DataPtr = NGLMEM_COMMAND_LIST_START;
}

inline void nglVif1StartCommandList( u_int ListIdx )
{
  // Mark the beginning of the command list in both the command list program array and VU memory (for the data).
  nglCmdListInfo->ListOfs[ListIdx * 2 + 0] = nglCmdListInfo->CmdCount;
  nglCmdListInfo->ListOfs[ListIdx * 2 + 1] = nglCmdListInfo->DataPtr;
}

inline void nglVif1EndCommandList()
{
  nglCmdListInfo->CmdList[nglCmdListInfo->CmdCount++] = 0;
}

inline void nglVif1EmptyCommandList( u_int ListIdx )
{
  nglCmdListInfo->ListOfs[ListIdx * 2 + 0] = 0xFF;
}

inline void nglVif1AddCommandListProgram( char* Addr )
{
  nglCmdListInfo->CmdList[nglCmdListInfo->CmdCount++] = NGL_PROG_ADDR( Addr, nglBaseAddr );
}

inline void nglVif1ExecCommandLists( u_int*& _Packet )
{
  // Check the command list position to see if we overflowed it.  If so, don't render the mesh this frame.
#if NGL_DEBUGGING
  if ( nglCmdListInfo->DataPtr >= NGLMEM_COMMAND_LIST_START + NGLMEM_COMMAND_LIST_SIZE )
  {
    nglWarning( "NGL: Command list overflowed (%d/%d).\n", nglCmdListInfo->DataPtr - NGLMEM_COMMAND_LIST_START, NGLMEM_COMMAND_LIST_SIZE );
    nglFatalDMAError = true;
    return;
  }
#endif

  // Write the VU memory addresses for each lists' programs and data, so the VU code knows where to find them.
  // Program address goes into the X component and Data address into the Y component.
  register u_int* Packet = _Packet;
  *(Packet++) = SCE_VIF1_SET_UNPACK( NGLMEM_COMMAND_LIST_ADDRS, NGLCMD_NUM_LISTS, SCE_VIF1_V2_8, 0 ) | SCE_VIF1_USN;

  register u_short* ShortPtr = (u_short*)Packet;
  register u_short* ListOfs = (u_short*)nglCmdListInfo->ListOfs;
  const u_char DataPtr = nglCmdListInfo->DataPtr;
  u_int i;
  for ( i = 0; i < NGLCMD_NUM_LISTS; i++ )
  {
//    assert( ( *ListOfs & 0xFF ) == 0xFF || ( *ListOfs & 0xFF ) + DataPtr <= 0xFF );
    if ( ( *ListOfs & 0xFF ) != 0xFF )
      *(ShortPtr++) = *ListOfs + DataPtr;
    else
      // Empty command list, signal the VU code with 0.
      *(ShortPtr++) = 0;
    ListOfs++;
  }
  if ( NGLCMD_NUM_LISTS & 1 )
    ShortPtr++;
  Packet = (u_int*)ShortPtr;

  // Write the command list program array.
  register u_int CmdCount = nglCmdListInfo->CmdCount;
  *(Packet++) = SCE_VIF1_SET_UNPACK( DataPtr, CmdCount, SCE_VIF1_S_16, 0 ) | SCE_VIF1_USN;

  register int Count = CmdCount;
  register u_int* CmdList = (u_int*)nglCmdListInfo->CmdList;
  do {
    *(Packet++) = *(CmdList++);
    Count -= 2;
  } while ( Count > 0 );

  // Start the chain.  Each MSCNT after nglNodeSetupAddr calls nglCommandListAddr.
  if ( STAGE_ENABLE( CommandList ) )
    *(Packet++) = SCE_VIF1_SET_MSCAL( NGL_PROG_ADDR( nglNodeSetupAddr, nglBaseAddr ), 0 );
  else
    *(Packet++) = SCE_VIF1_SET_MSCAL( NGL_PROG_ADDR( nglNoCommandListAddr, nglBaseAddr ), 0 );

  _Packet = Packet;
}

inline void nglVif1AddCommandListData( u_int*& _Packet, int Mode, int QWC, void* Data, int DataSize )
{
  register u_int* Packet = _Packet;
  register u_int* DataWords = (u_int*)Data;
  *(Packet++) = SCE_VIF1_SET_UNPACK( nglCmdListInfo->DataPtr, QWC, Mode, 0 ) | SCE_VIF1_USN;
  do {
    *(Packet++) = *(DataWords++);
    DataSize -= 4;
  } while ( DataSize > 0 );
  nglCmdListInfo->DataPtr += QWC;
  _Packet = Packet;
}

#endif
/*---------------------------------------------------------------------------------------------------------
  Texture streaming functions inline implementation
---------------------------------------------------------------------------------------------------------*/
inline void nglTexStreamPos::align(u_int size)
{
	if (addr+size > NGL_VRAM_STREAM_END)
	{
		addr = NGL_VRAM_STREAM_START;
		++cycle;
	}
}
inline void nglTexStreamPos::align2k(u_int size)
{
	addr = (addr+31)&~31;
	if (addr+size > NGL_VRAM_STREAM_END)
	{
		addr = NGL_VRAM_STREAM_START;
		addr = (addr+31)&~31;
		++cycle;
	}
}
inline void nglTexStreamPos::alloc(u_int size)
{
	addr += size;
	if (addr >= NGL_VRAM_STREAM_END)
	{
		addr = NGL_VRAM_STREAM_START;
		++cycle;
	}
}

inline bool operator==(const nglTexStreamPos &a, const nglTexStreamPos &b)
{
	return a.cycle == b.cycle && a.addr == b.addr;
}

inline bool operator>(const nglTexStreamPos &a, const nglTexStreamPos &b)
{
	return a.cycle > b.cycle ||
		  (a.cycle == b.cycle && a.addr > b.addr);
}

inline bool operator>=(const nglTexStreamPos &a, const nglTexStreamPos &b)
{
	return a.cycle > b.cycle ||
		  (a.cycle == b.cycle && a.addr >= b.addr);
}

inline bool operator<(const nglTexStreamPos &a, const nglTexStreamPos &b)
{
	return a.cycle < b.cycle ||
		  (a.cycle == b.cycle && a.addr < b.addr);
}

inline bool operator<=(const nglTexStreamPos &a, const nglTexStreamPos &b)
{
	return a.cycle < b.cycle ||
		  (a.cycle == b.cycle && a.addr <= b.addr);
}

extern nglTexStreamPos nglTexStreamPrevStartRef;	// Oldest referenced texture in previous texture block
extern nglTexStreamPos nglTexStreamCurStartRef;	// Oldest referenced texture in current texture block

inline bool nglVif1AddSingleTextureStreaming( u_int*& Packet, nglTexture* Tex )
{
  return nglVif1AddTextureStreaming( Packet, &Tex, 1 );
}

inline bool nglVif1CheckTextureStreaming( nglTexture** Textures, int NTextures )
{
  for ( int i = 0; i < NTextures; i++ )
    if ( Textures[i]->TexStreamPos < nglTexStreamCurStartRef );
      return false;
  return true;
}

inline bool nglVif1CheckSingleTextureStreaming( nglTexture* Tex )
{
  return Tex->TexStreamPos >= nglTexStreamCurStartRef;
}

/*---------------------------------------------------------------------------------------------------------
  GS related functions inline implementation.
---------------------------------------------------------------------------------------------------------*/
extern u_int nglGifRegCount;
extern u_char nglGifRegIdx[128];
extern u_long nglGifRegVal[128];

extern u_int nglGSRegIdx[NGL_GS_NUM_REGS];

inline void nglVif1ResetGSRegisterCache()
{
}

inline void nglVif1AddGSReg( nglGSRegType Reg, u_long Val )
{
  nglGifRegIdx[nglGifRegCount] = nglGSRegIdx[Reg];
  nglGifRegVal[nglGifRegCount] = Val;
  nglGifRegCount++;
}

inline void nglVif1AddRawGSReg( u_int Reg, u_long Val )
{
  nglGifRegIdx[nglGifRegCount] = Reg;
  nglGifRegVal[nglGifRegCount] = Val;
  nglGifRegCount++;
}

extern const u_long nglGSBlendModes[NGLBM_MAX_BLEND_MODES][2];

inline void nglVif1AddSetBlendMode( int Mode, int Fix = 0, bool ForceZ = false)
{
  nglVif1AddGSReg( NGL_GS_ALPHA_1, nglGSBlendModes[Mode][0] | (u_long)Fix << 32 );
  nglVif1AddGSReg( NGL_GS_TEST_1, ForceZ ? nglGSBlendModes[NGLBM_OPAQUE][1] : nglGSBlendModes[Mode][1] );
}

inline void nglVif1AddSetBlendModeCheckZTest( int Mode, int Fix = 0, bool ForceZ = false)
{
  nglVif1AddGSReg( NGL_GS_ALPHA_1, nglGSBlendModes[Mode][0] | (u_long)Fix << 32 );
  if (nglCurScene->ZTestEnable)
    nglVif1AddGSReg( NGL_GS_TEST_1,  ForceZ ? nglGSBlendModes[NGLBM_OPAQUE][1] : nglGSBlendModes[Mode][1] );
  else
    nglVif1AddGSReg( NGL_GS_TEST_1, (ForceZ ? nglGSBlendModes[NGLBM_OPAQUE][1] : nglGSBlendModes[Mode][1]) & ~GS_TEST_ZTST_M | (1<<17) );
}

inline void nglVif1StartMaterialKick()
{
  nglGifRegCount = 1;
}

inline void nglVif1EndMaterialKick( u_int*& Packet, u_int& DataPtr )
{
  if ( nglGifRegCount == 1 )
    return;

  nglVif1AddCommandListProgram( Packet, DataPtr, nglMaterialKickAddr );

  // Unpack the register count plus the indices as S_8, and then values as V2_32.
  nglGifRegIdx[0] = nglGifRegCount - 1;
  nglVif1AddUnpack( Packet, DataPtr, SCE_VIF1_S_8, nglGifRegCount, nglGifRegIdx, nglGifRegCount );
  nglVif1AddUnpack( Packet, DataPtr + 1, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, nglGifRegCount - 1, &nglGifRegVal[1], ( nglGifRegCount - 1 ) * sizeof(u_long) );
  DataPtr += nglGifRegCount;
}

inline void nglVif1StartDirectGifAD()
{
  nglGifRegCount = 0;
}

inline void nglVif1EndDirectGifAD( u_int*& Packet )
{
  // Set the DIRECT VIFCode and the GIFTag with the correct number of A/D entries.
  while ( (u_int)( Packet + 1 ) & 15 ) *(Packet++) = SCE_VIF1_SET_NOP( 0 );
  *(Packet++) = SCE_VIF1_SET_DIRECT( nglGifRegCount + 1, 0 );

  u_long* TagPtr = (u_long*)Packet;
  TagPtr[0] = SCE_GIF_SET_TAG( nglGifRegCount, 1, 0, 0, SCE_GIF_PACKED, 1 );
  TagPtr[1] = 0xE;
  TagPtr += 2;

  // Write out the GS registers.
  for ( u_int i = 0; i < nglGifRegCount; i++ )
  {
    TagPtr[0] = nglGifRegVal[i];
    TagPtr[1] = nglGifRegIdx[i];
    TagPtr += 2;
  }
  Packet = (u_int*)TagPtr;
}

inline u_int* nglVif1StartDirect( u_int*& Packet )
{
  NGL_ASSERT( ( (u_int)Packet & 15 ) == 0, "16 byte packet alignment required." );
  u_int* TagPtr = Packet;
  Packet += 4;
  return TagPtr;
}

inline void nglVif1EndDirect( u_int*& Packet, u_int* TagPtr )
{
  Packet = (u_int*)NGL_ALIGN( Packet, 16 );
  TagPtr[0] = SCE_VIF1_SET_NOP( 0 );
  TagPtr[1] = SCE_VIF1_SET_NOP( 0 );
  TagPtr[2] = SCE_VIF1_SET_NOP( 0 );
  u_int QWC = ( (u_int)Packet - ( (u_int)TagPtr + 16 ) ) / 16;
  NGL_ASSERT( QWC, "Empty DIRECT tag detected.\n" );
  TagPtr[3] = SCE_VIF1_SET_DIRECT( QWC & 0xFFFF, 0 );
}

inline u_long* nglGifStartTag( u_int*& Packet )
{
  NGL_ASSERT( ( (u_int)Packet & 15 ) == 0, "16 byte packet alignment required." );
  u_long* TagPtr = (u_long*)Packet;
  Packet += 4;
  return TagPtr;
}

inline u_int nglGifCalcNLoop( u_int* Packet, u_long* TagPtr, u_int Mode, u_int NRegs )
{
  // PACKED uses 16 byte regs, REGLIST uses 8 byte regs.  Don't ask me, ask Sony ;)
  const u_int RegSize = Mode == SCE_GIF_PACKED ? 16 : 8;
  u_int Size = (u_int)Packet - ( (u_int)TagPtr + 16 );
  NGL_ASSERT( Size % ( RegSize * NRegs ) == 0, "Bad GS register count (based on GIFTag)." );
  return Size / ( RegSize * NRegs );
}

inline void nglGifEndTag( u_int*& Packet, u_long* TagPtr, u_long Tag, u_long Regs )
{
  // No assert is here like in nglVif1EndDirect - 0 length GIFTags are perfectly ok.
  NGL_ASSERT( ( (u_int)Tag & ( 0xFFFFFFFF << 16 ) ) == 0, "Invalid bit(s) in GIFTag." );
  TagPtr[0] = Tag;
  TagPtr[1] = Regs;
  Packet = (u_int*)NGL_ALIGN( Packet, 16 );
}

/*---------------------------------------------------------------------------------------------------------
  Font structures.
---------------------------------------------------------------------------------------------------------*/
#define NGLFONT_VERSION       0x102

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
	// Font info.
  nglTexture* Tex;
  nglGlyphInfo* GlyphInfo;
  nglFontHeader Header;

	// Rendering properties.
  u_int	MapFlags;
  u_int	BlendMode;
  u_int	BlendModeConstant;

	// System flag to protect from nglReleaseAllFonts.
	bool System;
};

/*---------------------------------------------------------------------------------------------------------
  Edit mesh functions inline implementation.  Optimized for cycle count. (dc 12/14/01)
---------------------------------------------------------------------------------------------------------*/
// Current in-progress scratch mesh.
extern nglMesh* nglScratch;
extern nglMeshBatchInfo nglScratchBatch;
extern u_int nglScratchBatchIdx;
extern u_int nglScratchVertIdx;
extern u_int nglScratchStripVertIdx;

// Necessary scratch mesh functions
void nglMeshNextBatch();

inline void nglMeshFastWriteVertexPC( float X, float Y, float Z, u_int Color )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPCUV( float X, float Y, float Z, u_int Color, float U, float V )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPCUVB( float X, float Y, float Z, u_int Color, float U, float V,
							  int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;

  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;

  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
  BoneWeightData[0] = w1;
  BoneWeightData[1] = w2;
  BoneWeightData[2] = w3;
  BoneWeightData[3] = w4;

  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
  BoneIdxData[0] = b1*4 + BonePos;
  BoneIdxData[1] = b2*4 + BonePos;
  BoneIdxData[2] = b3*4 + BonePos;
  BoneIdxData[3] = b4*4 + BonePos;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPCUV2( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2 )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
  LightUVData[0] = U2;
  LightUVData[1] = V2;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPCUV2B( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2,
							   int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
  LightUVData[0] = U2;
  LightUVData[1] = V2;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;

  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;

  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
  BoneWeightData[0] = w1;
  BoneWeightData[1] = w2;
  BoneWeightData[2] = w3;
  BoneWeightData[3] = w4;

  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
  BoneIdxData[0] = b1*4 + BonePos;
  BoneIdxData[1] = b2*4 + BonePos;
  BoneIdxData[2] = b3*4 + BonePos;
  BoneIdxData[3] = b4*4 + BonePos;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPUV( float X, float Y, float Z, float U, float V )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPN( float X, float Y, float Z, float NX, float NY, float NZ )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPNC( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPNCUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, float U, float V )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
  ColorData[0] = Color;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPNUV( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPNUVB( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V,
							  int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;

  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;

  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
  BoneWeightData[0] = w1;
  BoneWeightData[1] = w2;
  BoneWeightData[2] = w3;
  BoneWeightData[3] = w4;

  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
  BoneIdxData[0] = b1*4 + BonePos;
  BoneIdxData[1] = b2*4 + BonePos;
  BoneIdxData[2] = b3*4 + BonePos;
  BoneIdxData[3] = b4*4 + BonePos;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPNUV2( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2 )
{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
  LightUVData[0] = U2;
  LightUVData[1] = V2;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

inline void nglMeshFastWriteVertexPNUV2B( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2,
							   int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)

{
  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
    nglMeshNextBatch();

  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);

  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);

  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);

  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;

  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
  PosData[0] = X;
  PosData[1] = Y;
  PosData[2] = Z;

  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
  NormData[0] = NX;
  NormData[1] = NY;
  NormData[2] = NZ;

  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
  UVData[0] = U;
  UVData[1] = V;

  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
  LightUVData[0] = U2;
  LightUVData[1] = V2;

  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;

  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;

  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
  BoneWeightData[0] = w1;
  BoneWeightData[1] = w2;
  BoneWeightData[2] = w3;
  BoneWeightData[3] = w4;

  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
  BoneIdxData[0] = b1*4 + BonePos;
  BoneIdxData[1] = b2*4 + BonePos;
  BoneIdxData[2] = b3*4 + BonePos;
  BoneIdxData[3] = b4*4 + BonePos;

  nglScratchStripVertIdx++;
  nglScratchVertIdx++;
}

/*---------------------------------------------------------------------------------------------------------
  @Mesh rendering code.
---------------------------------------------------------------------------------------------------------*/
// This is an object shared by mesh material nodes in the render list, it's a single instance of a mesh in the scene.
struct nglMeshNode
{
  nglMesh* Mesh;                 // Mesh resource.
  int Clip;                      // 1 if the mesh intersects one or more of the guard bands.
  float Dist;                    // Distance from the farthest point on the mesh to the camera.

  nglMatrix LocalToWorld;      // Local to world matrix.
  nglMatrix WorldToLocal;      // Inverse of the above, for lighting in local space.

  nglRenderParams Params;        // Custom render parameters.
};

struct nglMeshSectionNode
{
  nglMeshNode* MeshNode;         // Mesh node structure shared by all the nodes.
  nglMeshSection* Section;       // What section of the mesh to render.
  u_int MaterialFlags;           // Copy of material flags, possibly with changes.
};

// Sets nglCurLightContext and builds a list of lights affecting the current mesh into the context's Local Lights List.
// Also fills Textures with the set of textures that will need streaming as a result of projector lights, and increments NTextures.
void nglDetermineLocalLights( nglLightContext* Context, nglMesh* Mesh, nglMatrix& LocalToWorld, nglTexture** Textures, u_int& NTextures );

void nglVif1AddVertexLightSetup( u_int*& _Packet, u_int& _CmdListDataPtr, nglMeshSection* Section, nglMesh* Mesh, nglMeshNode* MeshNode );
void nglVif1AddTextureLightSetup( u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, nglMesh* Mesh, nglMeshNode* MeshNode, u_int MaterialFlags );

inline void nglVif1AddTextureFixup( u_int*& _Packet, nglTexture* Tex, u_int GSRegAddr, float MipRatio );

#endif	// #ifndef NGL_PS2_INTERNAL_H

