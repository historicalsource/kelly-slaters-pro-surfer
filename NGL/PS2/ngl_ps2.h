/*!
 * @mainpage N Graphics Library
 *
 * @section new New Features
 *
 * - Check out the new @ref meshedit
 *
 * @section modules NGL Modules
 *
 * - @ref meshedit
 *
 * @section maint Mainainers
 *
 * 		- Wade Brainerd <wade@treyarch.com>
 * 		- Michael Montague <mikem@treyarch.com>
 * 		- Andy Chien <chien@treyarch.com>
 *
 * @section copyright
 *
 * NGL &copy; 2001-2002 Treyarch Corp.
 */

#ifndef NGL_H
#define NGL_H

/*---------------------------------------------------------------------------------------------------------
  Defines and types for projects that conditionally compile NGL.  
---------------------------------------------------------------------------------------------------------*/
#define NGL               1
#define NGL_PS2           1
#define NGL_MESHFILE_EXT  ".ps2mesh"

// Takes 4 integers ranging from 0-255 and returns a packed color value.
#define NGL_RGBA32( r, g, b, a )  ((int)((r)+1)>>1) | ( ((int)((g)+1)>>1) << 8 ) | ( ((int)((b)+1)>>1) << 16 ) | ( ((int)((a)+1)>>1) << 24 )

// Takes 4 integers ranging from 0-255 and returns a packed color value for use with 32bit textures.
#define NGL_RGBA32_TEX( r, g, b, a )  ((int)(r)) | ( ((int)(g)) << 8 ) | ( ((int)(b)) << 16 ) | ( ((int)((a)+1)>>1) << 24 )

/*---------------------------------------------------------------------------------------------------------
  Feature support defines.  For more explicit version info, include ngl_version.h.
---------------------------------------------------------------------------------------------------------*/
#define NGL_HAS_VECTOR_CLASS 1
#define NGL_HAS_NEW_FONT_API 1
#define NGL_HAS_MORPH_API 1

/*---------------------------------------------------------------------------------------------------------
  Includes
---------------------------------------------------------------------------------------------------------*/
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#include <eetypes.h>
#include "tim2.h"
#include "ngl_fixedstr.h"

#if 0   // these currently conflict w/spiderman
/*---------------------------------------------------------------------------------------------------------
  General platform independent types.
---------------------------------------------------------------------------------------------------------*/
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned long   uint64;
typedef          char   int8;
typedef          short  int16;
typedef          int    int32;
typedef          long   int64;
#endif

/*---------------------------------------------------------------------------------------------------------
  Vector/Matrix API.

  These are designed to be as simple as possible to pass data into NGL, and are not for general game use.
  Therefore the constructors should be fairly robust (app defined constructors are supported) but they
  don't support any operators.
---------------------------------------------------------------------------------------------------------*/
class nglVector
{
public:
  float x, y, z, w;

  nglVector() {}

  nglVector( const nglVector& v )
  { x = v.x; y = v.y; z = v.z; w = v.w; }

  nglVector( float _x, float _y, float _z, float _w = 1.0f )
  { x = _x; y = _y; z = _z; w = _w; }

  nglVector( sceVu0FVECTOR v )
  { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }

  // allow projects to define specific constructors for nglVector
#ifndef NGL_VECTOR_CTORS
#define NGL_VECTOR_CTORS
#endif
  NGL_VECTOR_CTORS

  inline const float& operator[](int i) const
  { return (&x)[i]; }

  inline float& operator[](int i)
  { return (&x)[i]; }

  inline operator float*()
  { return &x; }

  inline operator const float*()
  { return &x; }
} __attribute__((aligned (16)));

class nglMatrix
{
public:
  nglVector x, y, z, w;

  nglMatrix() {}

  nglMatrix( const nglMatrix& m )
  { x = m.x; y = m.y; z = m.z; w = m.w; }

  nglMatrix( const nglVector& _x, const nglVector& _y, const nglVector& _z, const nglVector& _w )
  { x = _x; y = _y; z = _z; w = _w; }

  nglMatrix( sceVu0FMATRIX m )
  { *this = *(nglMatrix*)m; }

  // allow projects to define specific constructors for nglMatrix
#ifndef NGL_MATRIX_CTORS
#define NGL_MATRIX_CTORS
#endif
  NGL_MATRIX_CTORS

  inline const nglVector& operator[](int i) const
  { return (&x)[i]; }

  inline nglVector& operator[](int i)
  { return (&x)[i]; }

  inline operator float*()
  { return &x.x; }

  inline operator const float*()
  { return &x.x; }
} __attribute__((aligned (16)));

void nglIdentityMatrix( nglMatrix& matrix );
void nglMulMatrix( nglMatrix& dst, const nglMatrix& lhs, const nglMatrix& rhs );
void nglApplyMatrix( nglVector& dst, nglMatrix& lhs, nglVector& rhs );

/*---------------------------------------------------------------------------------------------------------
  System Callback API.

  Allows the game to override various system operations within NGL.  Defaults are provided.
---------------------------------------------------------------------------------------------------------*/
// File buffer for callback functions to fill.
struct nglFileBuf
{
  u_char* Buf;
  u_int Size;
  u_int UserData;     // Optional data for the callbacks' use.
};

// Structure of function pointers for NGL system callbacks.
struct nglSystemCallbackStruct
{
  // File operations.
  bool (*ReadFile)( const char* FileName, nglFileBuf* File, u_int Align );
  void (*ReleaseFile)( nglFileBuf* File );

  // Errors/debugging.
  void (*CriticalError)( const char* Text );
  void (*DebugPrint)( const char* Text );

  // Memory allocation.
  void* (*MemAlloc)( u_int Size, u_int Align );
  void (*MemFree)( void* Ptr );
};

/*---------------------------------------------------------------------------------------------------------
  nglSetSystemCallbacks

  Overrides default system operations.  Passing in a structure of function pointers will override the
  current system callbacks and cause NGL to use the new ones for all operations.

  NULL can passed in any of the pointers, which will cause the NGL default implementation to be used.
---------------------------------------------------------------------------------------------------------*/
void nglSetSystemCallbacks( nglSystemCallbackStruct* Callbacks );

/*---------------------------------------------------------------------------------------------------------
  System callback function handlers (defaults provided), in case the client wants to call them directly.
---------------------------------------------------------------------------------------------------------*/
#if defined(NGL_FINAL)
inline void nglPrintf( const char* Format, ... ) {}
inline void nglWarning( const char* Format, ... ) {}
inline void nglFatal( const char* Format, ... ) {}
#else

//! Print a formatted message to the TTY.
void nglPrintf( const char* Format, ... );

//! Print a formatted message, in bright red, to the TTY.
void nglWarning( const char* Format, ... );

//! Enter an infinite loop and print a message, in bright red, to the TTY.
void nglFatal( const char* Format, ... );

#endif

bool nglReadFile( const char* FileName, nglFileBuf* File, u_int Align = 1 );
void nglReleaseFile( nglFileBuf* File );
void* nglMemAlloc( u_int Size, u_int Align = 1 );
void nglMemFree( void* Ptr );

/*---------------------------------------------------------------------------------------------------------
  nglSetProViewPS2 - PS2 Specific

  Call this to tell the default NGL file reader to use ProView for file loading and TTY output.
---------------------------------------------------------------------------------------------------------*/
extern const char* nglHostPrefix;
extern bool nglUsingProView;
void nglSetProViewPS2( bool UsingProView );

/*---------------------------------------------------------------------------------------------------------
  General API
---------------------------------------------------------------------------------------------------------*/
void nglInit();
void nglExit();

// This function should be called BEFORE nglInit or nglResetDisplay in order to take effect.
// Default value is NTSC, so we have to call this func only to activate PAL code config.
enum
{
  NGLTV_PAL,
  NGLTV_NTSC,
};
void nglSetTVMode( u_int TVMode );
u_int nglGetTVMode();

void nglSetAspectRatio(float a);

// Resets the graphics system to NGL's preferred mode.  Useful for integrating NGL with other display libraries,
// such as movie players.
void nglResetDisplay();

// Buffer size control.  All of these except NGLBUF_LIST_WORK are double buffered, so the size passed here
// refers to the size of a single buffer (IE twice the passed amount of memory is allocated).  Can be called
// before or after nglInit.
enum
{
  NGLBUF_VIF1_PACKET_WORK,      // Buffer for the DMA packet containing rendering commands.
  NGLBUF_GIF_PACKET_WORK,       // Buffer for the DMA packets containing texture upload commands.
  NGLBUF_SCRATCH_MESH_WORK,     // Buffer for creating temporary meshes.
  NGLBUF_LIST_WORK,             // Buffer for temporary render list objects such as scenes, mesh nodes, and lights.
};
void nglSetBufferSize( u_int BufID, u_int Size );

// Flips the front/back buffers.  Typically you want to do this by passing true to nglListSend.
void nglFlip();

// Lock the frame rate to a given FPS (default is 60fps).
// The parameter must divide evenly into 60, ie 60, 30, 20, 15, 10, etc.
void nglSetFrameLock( float FPS );

// Sets the speed at which IFLs play, in frames per second (default is 30fps).
// Values should divide evenly into 60, ie 60, 30, 20, 15, 10, etc.
void nglSetIFLSpeed( float FPS );

// Get/set debugging flags externally.  See nglDebugStruct in ngl_ps2.cpp for a list.
void nglSetDebugFlag( const char* Flag, bool Set );
bool nglGetDebugFlag( const char* Flag );

// Returns the screen width and height in pixels.
int nglGetScreenWidth();
int nglGetScreenHeight();

void nglSetMeshPath( const char* Path );
void nglSetTexturePath( const char* Path );
const char *nglGetMeshPath( void );
const char *nglGetTexturePath( void );

// Take a screenshot of the next frame.
void nglScreenShot(const char *FileName = NULL);

/*---------------------------------------------------------------------------------------------------------
  Custom Node API
---------------------------------------------------------------------------------------------------------*/
// Custom node callback function.
typedef void (*nglCustomNodeFn)( u_int*& Packet, void* Data );

enum
{
  NGLSORT_TRANSLUCENT,
  NGLSORT_TEXTURED,
};

struct nglSortInfo
{
  u_int Type;
  union {
    float Dist;           // Sorting distance for NGLSORT_TRANSLUCENT.
    u_int Hash;           // Texture map's hash value for NGLSORT_TEXTURED.
  };
};

void nglListAddCustomNode( nglCustomNodeFn Fn, void* Data, nglSortInfo* SortInfo );

/*---------------------------------------------------------------------------------------------------------
  Scene API

  A global default scene is automatically created and selected by nglListInit().  Whenever a new scene is
  created with nglListBeginScene(), it takes its initial parameter settings from the parent scene.

  Many NGL API calls implicitly reference the currently selected scene.   These include calls to set scene
  parameters or to add objects to be rendered in the scene.
---------------------------------------------------------------------------------------------------------*/
class nglScene;

// Initializes the scene list, and the enters main scene.  Call this once per frame, before any other
// scene functions.
void nglListInit();

// Send the list of scenes to the hardware asynchronously.  If Flip is true, it will perform a flip operation
// at the last possible moment before starting the next frame rendering for maximum parallelism.  Call this
// after all your scenes have been constructed, when you want to render everything.
void nglListSend( bool Flip = false );

//
// Creates a child of the currently selected scene and selects it, returning a handle that can be used
// to later select the scene.
//
// Child scenes are normally rendered first before their parent scene, and can be used to set up special
// offscreen textures, or multiple scenes can be chained together at the same level to force a given rendering
// order.  For example, you can create two child scenes of the main scene, one to render the game and one
// to render the interface (and even set the interface scene to clear the Z buffer).
//
// (PS2 Only)
// The SortInfo parameter allows you control when the scene is actually rendered, beyond the normal child
// parent hierarchy.  By passing SortInfo, a scene can be rendered in the middle of a parent scene as
// opposed to at the beginning.
nglScene* nglListBeginScene( nglSortInfo *SortInfo = 0 );

// nglListEndScene leaves the current scene and returns to the parent scene.
void nglListEndScene();

//
// nglListSelectScene allows you to jump to a particular scene directly, and add more nodes to it or change
// scene parameters.  This can be useful for simultaneously generating the main game scene and a scene
// containing a reflection.
//
// The currently active scene is returned, so you can return to it later.
nglScene* nglListSelectScene(nglScene *scene);

// Specify a texture to render the scene to.  If NULL is passed, the back buffer of the screen is assumed.
struct nglTexture;
void nglSetRenderTarget( nglTexture* Tex, bool Download = false );

// Specify an array of textures required to be present when a streamed RenderTarget is allocated.
// Used to ensure a streamed RenderTarget will not be flushed from video ram
// while needed, due to references to any of these textures.
// Note: Do not include duplicates or the RenderTarget itself.
// Defaults to empty array if not called.
void nglSetRequiredTexturesPS2( nglTexture** TexTbl, u_int NTex);

// Set up for orthogonal rendering.
//
// Set up for orthogonal (parallel projection) rendering.   The view ranges from -1 to 1 in X and Y axes, where
// (0,0) translates to the center of projection.
//   (cx,cy) is the center of projection in screen coordinates.
//   (nearz..farz) is the input range that vertices and quads will use.
//   (zmin..zmax) PS2 only - can be used to limit output to a given range of the Z buffer.
void nglSetOrthoMatrix( float cx, float cy, float nearz, float farz, float zmin = 0.0f, float zmax = 1.0f );

//
// Set up for perspective rendering.
//   hfov is the horizontal field of view in degrees (example 90.0f).
//   (cx,cy) is the center of projection in screen coordinates.
//   (nearz..farz) is the input range that vertices and quads will use.
//   (zmin..zmax) PS2 only - can be used to limit output to a given range of the Z buffer.
//   (renderwidth,renderheight) - WTB TODO: need to find out what these do ;)
void nglSetPerspectiveMatrix(
  float hfov, float cx, float cy, float nearz, float farz,
  float zmin = 0.0f, float zmax = 1.0f, int renderwidth=0, int renderheight=0 );

void nglGetProjectionParams( float* hfov, float* cx, float* cy, float* nearz, float* farz );

// Sets the camera matrix for the scene.
void nglSetWorldToViewMatrix( const nglMatrix& WorldToView );

// Test a sphere to see if it's in the view frustum.
// -1 = Not visible, 0 = Completely visible, 1 = Partially visible.
int nglGetClipResult( nglVector& Center, float Radius );

// Clear flags.
enum
{
  NGLCLEAR_COLOR = 0x1,
  NGLCLEAR_Z     = 0x2,
};

// Screen/render target clear parameters for the scene.
void nglSetClearFlags( u_int ClearFlags );
void nglSetClearColor( float R, float G, float B, float A );
void nglSetClearZ( float Z );   // Z is in hardware space (0.0-nearest, 1.0-farthest).

// Framebuffer write mask.  On PS2 you can control the masking down to individual bits, but these masks
// are provided for convenience and compatibility.
enum
{
  NGL_FBWRITE_A    = 0xFF000000,
  NGL_FBWRITE_B    = 0x00FF0000,
  NGL_FBWRITE_G    = 0x0000FF00,
  NGL_FBWRITE_R    = 0x000000FF,
  NGL_FBWRITE_RGB  = 0x00FFFFFF,
  NGL_FBWRITE_RGBA = 0xFFFFFFFF,
};

// Set a write mask when rendering the geometry to the framebuffer.  All channels are written by default.
void nglSetFBWriteMask( u_int WriteMask );

// Enables/Disables writes to Z buffer for this scene.  Enabled by default.
void nglSetZWriteEnable( bool Enable );

// Enables/Disables Z buffer testing for this scene.  Enabled by default.
// HACK ALERT- Currently only disables z testing for Quads and Strings.
void nglSetZTestEnable( bool Enable );

// Sets the clipping rectangle for the scene, in coordinates of pixels.  Defaults are 0, 0, ScreenWidth - 1, ScreenHeight - 1.
void nglSetViewport( u_int x1, u_int y1, u_int x2, u_int y2 );

// Fog parameters for the scene.
void nglSetFogColor( float R, float G, float B );
void nglSetFogRange( float Near, float Far, float Min, float Max );

// Animation frame to use for texture scrolling and IFL animation.  This is controllable per scene to allow frontend
// animation to continue while game animation stops.
// To get smooth IFL animation, multiples of 1/60th second are required.  If not called, NGL's internal VBlank-based timer is used.
void nglSetAnimTime( float Time );

// Returns one of the current matrices in the model->view->projection chain.
enum
{
  NGLMTX_VIEW_TO_WORLD,
  NGLMTX_VIEW_TO_SCREEN,
  NGLMTX_WORLD_TO_VIEW,
  NGLMTX_WORLD_TO_SCREEN,
};
void nglGetMatrix( nglMatrix& Dest, u_int ID );

// Projects a point from World space (X,Y,Z) to Screen space (0..nglGetScreenWidth()-1,0..nglGetScreenHeight()-1).
// Useful for placing 2D interface elements on 3D points in the scene.
void nglProjectPoint( nglVector& Out, nglVector& In );

/*---------------------------------------------------------------------------------------------------------
  Texture API
---------------------------------------------------------------------------------------------------------*/
struct TIM2_PICTUREHEADER;

enum
{
  NGLTEX_TGA,     // General purpose format.  Currently supports 32bit, 24bit and 16bit.
  NGLTEX_TIM2,    // Versatile PS2 specific format.  Handles palettes, many formats, etc.
  NGLTEX_IFL,     // IFL files aren't actual textures, but arrays of texture pointers in an animated sequence.
  NGLTEX_ATE,     // Animated texture format that stores all the frames with a shared header/palette.
};

class nglTexStreamPos {
public:
	u_int addr, cycle;
	nglTexStreamPos() {}
	nglTexStreamPos(u_int a, u_int c) : addr(a), cycle(c) {}
	void align(u_int size);
	void align2k(u_int size);
	void alloc(u_int size);
};

// Texture resource structure.  These are stored in the texture instance bank.
// TODO: Collapse all texture types into a single set of member variables (lose 'ph' var).
struct nglTexGsChunk
{
  u_char PSM;                        // PSM for this block (can be different between palette/image data).
  u_char TBW;                        // TBW value for BITBLTBUF.
  u_short Offset;                    // Offset from texture's GsBaseTBP.
  u_short Size;                      // Total size of the texture in GS blocks up to this mip level.
  u_int* DMA;                        // Pointer to pregenerated uploading DMA.  Can be NULL if not present.
  void* Data;						             // Physical pixel data.
};  //  ^^^^ - Use this for procedurally modifying textures.

struct nglTexture
{
  nglTexStreamPos TexStreamPos;		    // Most recent location of texture in VRAM stream

  u_short Width, Height;              // Texture dimensions.
  u_int Hash;                         // Name hash for sorting into render bins.

  u_char Type;                        // Texture type (see above enum).
  u_char TW, TH;                      // 2^TW=Width, 2^TH=Height.

  struct
  {
    u_int LoadedInPlace : 1;          // 1 if the texture was loaded via nglLoadTextureInPlace
    u_int Locked : 1;                 // 1 if the texture is locked in VRAM, as opposed to streamed on demand.
    u_int VRAMOnly : 1;               // 1 if the texture has no system memory representation.
    u_int System : 1;                 // 1 if the texture was loaded by the system and shouldn't ever be released,
    u_int RenderTarget : 1;           // 1 if the texture is being used as a render target
  } Flags;

  // TIM2 format data:
  TIM2_PICTUREHEADER *ph;

  // TGA format data:
  u_int Format;                       // GS texture format.
  u_int* Data;                        // Pixel data address in RAM.

  // IFL/ATE format data:
  u_int NFrames;                      // If this member is greater than 1, this is an animated texture.
  nglTexture** Frames;                // Array of texture pointers for the individual frames (which are also stored in the instance bank).

  // Bookkeeping, non cache-critical.
  u_int SrcDataSize;                  // System memory used by the texture.
  nglFileBuf FileBuf;                 // The FileBuf structure this texture was loaded with.
  nglFixedString FileName;            // Original filename.

  // GS info, for texture streaming.
  u_int GsBaseTBP;                    // Most recently uploaded base TBP.
  u_int GsSize;                       // Total size of the texture in GS blocks (same as gsImage[0].Size)

  u_int* DMAMem;                      // Pointer to the baked texture DMA for all chunks (Clut and Mipmaps).

  u_int ClutColors;                   // Total colors in the palette, 0 if no palette.
  nglTexGsChunk GsClut;

  u_int MipMapTextures;               // (1 indicates no mipmappping, but 0 is allowed to mean this also for convenience)
  nglTexGsChunk GsImage[7];           // Only GsImage's 0 through max(0,MipMapTextures-1) are valid.

  // GS register cache.  These are first updated at load time, then modified when the texture is placed in the streaming buffer,
  // and then finally copied and modified again when they're applied to a material.
  u_long GsTexA;
  u_long GsTex0;

  // These are only set for Mipmapped textures.
  u_long GsTex1;
  u_long GsMipTBP1;
  u_long GsMipTBP2;
  float InvHypot;                      // 1/Sqrt(Width^2+Height^2), for mipmapping calculation.

  // These are for tracking first MipMap lod actually needed (only used if MipMapTextures>1)
  u_int MipMapInitCount;               // Last nglListInitCount for which MipMapFirstLod was valid
  u_int MipMapFirstLod;                // 0 for all lods, MipMapTextures-1 for only lowest lod
};

nglTexture* nglLoadTexture( const nglFixedString& FileName );
nglTexture* nglLoadTextureA( const char* FileName );

nglTexture* nglGetTexture( const nglFixedString& FileName );
nglTexture* nglGetTextureA( const char* FileName );

nglTexture* nglGetFrontBufferTex();
nglTexture* nglGetBackBufferTex();

nglTexture* nglLoadTextureInPlace( const nglFixedString& FileName, u_int Type, void* Data, u_int Size );

// Add a reference to a texture in the instance bank.
void nglAddTextureRef( nglTexture* Tex );

void nglReleaseTexture( nglTexture* Tex );
void nglReleaseAllTextures();

// Forces a texture to stay VRAM between frames.  Designed for load-time only.
void nglLockTexturePS2( nglTexture* Tex );
void nglUnlockTexturePS2( nglTexture* Tex );

void nglUnlockAllTexturesPS2();
void nglRelockAllTexturesPS2();

// Sets the size of the locked buffer, can only be called once per frame (any time before locking textures).
void nglSetLockBufferSizePS2( u_int Bytes );

// Texture creation API.
enum
{
  // Enumerated format values.
  NGLTF_32BIT = 0x1,
  NGLTF_16BIT = 0x2,

  // Flags to be OR'd into above values.
  NGLTF_TEMP          = 0x1000,   // Texture structure is allocated in nglListWork, lost at the end of the frame.
  NGLTF_RENDER_TARGET = 0x2000,   // Specify this flag if you intend to use this texture as a render target.

  NGLTF_VRAM_ONLY     = 0x8000    // PS2 specific flag, causes the texture to reside only in video memory
};                                //  (if a palette is present in texture, it is still uploaded).

nglTexture* nglCreateTexture( u_int Format, u_int Width, u_int Height );
void nglDestroyTexture( nglTexture* Tex );

// Saves a TGA file containing the texture to the host.  If no filename is given, it uses savetexXX.tga.  This
// is useful for debugging render to texture operations, especially involving Alpha as you can load the file
// in Photoshop and example the contents.
void nglSaveTexture( nglTexture* Tex, const char* FileName );

/*---------------------------------------------------------------------------------------------------------
  Mesh API
---------------------------------------------------------------------------------------------------------*/
// Material flags.
enum
{
  // Translucency.
  NGLMAT_ALPHA                = 0x00000001,   // Material is translucent and will be sorted.
  NGLMAT_ALPHA_SORT_FIRST     = 0x00000002,   // Causes polys to always sort at a particular distance from the camera.

  NGLMAT_ALPHA_FALLOFF        = 0x00000004,
  NGLMAT_ALPHA_FALLOFF_OUT    = 0x00000008,

  // Material passes.
  NGLMAT_TEXTURE_MAP          = 0x00000010,
  NGLMAT_LIGHT_MAP            = 0x00000020,
  NGLMAT_DETAIL_MAP           = 0x00000040,
  NGLMAT_ENVIRONMENT_MAP      = 0x00000080,

  NGLMAT_BUMP_MAP             = 0x00000100,   // Replaces the detail map with a bump map.

  // Clipping stuff.
  NGLMAT_BACKFACE_DEFAULT     = 0x00000400,
  NGLMAT_BACKFACE_CULL        = 0x00000800,   // Clip polygons that are facing away from the camera.

  // General rendering.
  NGLMAT_BILINEAR_FILTER      = 0x00001000,
  NGLMAT_TRILINEAR_FILTER     = 0x00002000,
  NGLMAT_ANTIALIAS            = 0x00004000,
  NGLMAT_PERSPECTIVE_CORRECT  = 0x00008000,

  NGLMAT_CLAMP_U              = 0x00010000,
  NGLMAT_CLAMP_V              = 0x00020000,

  NGLMAT_FOG                  = 0x00040000,
  NGLMAT_FORCE_Z_WRITE        = 0x00080000,

  NGLMAT_LIGHT                = 0x00100000,   // Self illuminated if this flag is off.
  NGLMAT_UV_SCROLL            = 0x00200000,   // Automatic UV scrolling.

  NGLMAT_ENV_CYLINDER         = 0x00400000,   // Specifies cylindrical environment mapping.
  NGLMAT_ENV_SPECULAR         = 0x00800000,   // Replaces the environment map with a specular highlight pass.

  // New clamping for light map.
  NGLMAT_LIGHT_CLAMP_U        = 0x01000000,
  NGLMAT_LIGHT_CLAMP_V        = 0x02000000,

  NGLMAT_MATERIAL_COLOR       = 0x10000000,   // Set to force the first pass to use the material color instead of the vertex color.
  NGLMAT_FORCE_BLEND          = 0x20000000,   // Set to force all passes to use NGLBM_BLEND instead of their normal blend mode.
};

#define NGL_TEXTURE_MASK       ( NGLMAT_TEXTURE_MAP | NGLMAT_LIGHT_MAP | NGLMAT_DETAIL_MAP | NGLMAT_ENVIRONMENT_MAP )

// Material blend modes.
enum
{
  NGLBM_OPAQUE,                       // No translucency is performed, alpha is ignored.
  NGLBM_PUNCHTHROUGH,                 // Similar to opaque, except if alpha is below a threshold the pixel is skipped.

  NGLBM_BLEND,                        // Blends the texel with the background, modulated by Alpha.
  NGLBM_ADDITIVE,                     // Adds the texel to the background, modulated by Source Alpha.
  NGLBM_SUBTRACTIVE,                  // Subtracts the texel from the background, modulated by Source Alpha.

  NGLBM_CONST_BLEND,                  // Blends the texel with the background, modulated by BlendModeConstant.
  NGLBM_CONST_ADDITIVE,               // Adds the texel to the background, modulated by BlendModeConstant.
  NGLBM_CONST_SUBTRACTIVE,            // Subtracts the texel from the background, modulated by BlendModeConstant.

  NGLBM_PUNCHTHROUGH_NO_ZTEST,        // Only draws if alpha nonzero.  Performs no zbuffer testing.
  NGLBM_DESTALPHA_ADDITIVE,           // Adds the texel to the background, modulated by Destination Alpha

  NGLBM_MAX_BLEND_MODES
};

// Mesh material structure with a fixed set of passes.
struct nglMaterialInfo;
struct nglMaterial
{
  // Material flags - see NGLMAT_XXXX.
  u_int Flags;

  // Texture pointers.
  nglTexture* Map;
  nglTexture* LightMap;
  nglTexture* DetailMap;
  nglTexture* EnvironmentMap;

  u_int Pad0;  // nglFixedString must be on a 8byte boundary.
  nglFixedString MapName;
  int MapBlendMode;
  int MapBlendModeConstant;

  nglFixedString LightMapName;
  int LightMapBlendMode;
  int LightMapBlendModeConstant;

  nglFixedString DetailMapName;
  int DetailMapBlendMode;
  int DetailMapBlendModeConstant;

  float DetailMapUScale;
  float DetailMapVScale;
  float DetailMapRange;
  float DetailMapAlphaClamp;

  nglFixedString EnvironmentMapName;
  int EnvironmentMapBlendMode;
  int EnvironmentMapBlendModeConstant;

  u_int SpecularColor;
  float SpecularPower;
  float SpecularIntensity;

  // Misc parameters.
  u_int Color;
  u_int MaterialID;
  float AlphaFalloff;
  float ForcedSortDistance;
  float ScrollU, ScrollV;

  // Calculated members:
  nglMaterialInfo* Info;
  float MapMipRatio;
  float LightMapMipRatio;

	// Member for use by custom material renderers, so they don't have to hijack our flags.
	u_int UserFlags;
  u_int Pad[2];
};

enum
{
  NGLP_TINT             = 0x00000001,   // Multiply vertex colors by a tint color, before lighting.

  // The following flags specify that bone positions are present in the render parameters
  //  (using NBones and Bones) in various coordinate systems.  At most one may be specified.
  NGLP_BONES_WORLD      = 0x00000002,   // Bone positions are in bone local (as defined by reference) to world space.
  NGLP_BONES_LOCAL      = 0x00000004,   // Bone positions are in bone local (as defined by reference) to local space.
  NGLP_BONES_RELATIVE   = 0x00000008,   // Bone positions are in reference mesh space to local space. (Formerly this was NGLP_BONES_LOCAL!)

  // Mask of all flags that indicate bone positions are present in render parameters
  // (Only used for testing, not setting)
  NGLP_BONES_MASK		= (NGLP_BONES_WORLD|NGLP_BONES_LOCAL|NGLP_BONES_RELATIVE),

  NGLP_BONES            = 0x00000000,   // This flag is obsolete and ignored

  NGLP_IDENTITY_MATRIX  = 0x00000010,   // Specifies that the LocalToWorld matrix is Identity.  A valid identity matrix must still be passed, for now.
  NGLP_SCALE            = 0x00000040,   // A non-uniform scale is to be applied and the Scale member is valid.
  NGLP_TEXTURE_FRAME    = 0x00000080,   // TextureFrame member is valid and should be used to override the default.
  NGLP_TEXTURE_SCROLL   = 0x00000100,   // ScrollU and ScrollV members are valid and should be used to scroll the texture.
  NGLP_MATERIAL_MASK    = 0x00000200,   // Mask off materials by index.
  NGLP_ZBIAS            = 0x00000400,   // Offset Z coordinates in the ZBuffer (doesn't affect display).
  NGLP_NO_CULLING       = 0x00000800,   // Turns off view frustum culling, assumes that the game code has already done it.
  NGLP_LIGHT_CONTEXT    = 0x00001000,   // LightContext member is valid and specifies the lighting context that the mesh should use.
  NGLP_NO_LIGHTING      = 0x00002000,   // Disable lighting for this instance.

  // PS2 specific flags.
  NGLP_WRITE_FB_ALPHA   = 0x01000000,   // Enables writing the resulting alpha to the frame buffer.
};

class nglLightContext;
struct nglRenderParams
{
  u_int Flags;                      // Flags that specify what members are valid.
  u_int TextureFrame;               // Frame value for animated textures.
  float ScrollU, ScrollV;           // Texture scrolling amount.
  nglVector TintColor;              // Color scale to be multiplied by vertex colors - RGBA, ranges from 0.0 to 1.0.
  nglVector Scale;                  // 3D non-uniform scale factor.  Do not apply scale to the LocalToWorld matrix!
  u_int MaterialMask;               // 32bit mask to selectively disable materials by ID.
  float ZBias;                      // Determines a scale for Z buffer
  nglLightContext* LightContext;    // Lighting context to apply to this mesh.

  // Bone transforms for skinned meshes (used if Flags & NGLP_BONES_MASK is nonzero).
  // If Bones is specified as null, space for NBones matrices is allocated and returned by nglListAddMesh
  // in Bones (or left null if the mesh is culled).  After calling nglListAddMesh with Bones=null,
  // if the result in Bones is non-null, the caller is then responsible for filling in the array of matrices.
  // The memory itself is temporary and will automatically be freed after rendering.
  u_int NBones;
  nglMatrix* Bones;
};

// Mesh flags
enum
{
  // Clipping stuff.
  NGLMESH_PERFECT_TRICLIP    = 0x00000001,
  NGLMESH_REJECT_TRICLIP     = 0x00000002,
  NGLMESH_REJECT_SPHERE      = 0x00000004,

  // Transform stuff.
  NGLMESH_INACCURATE_XFORM   = 0x00000010,
  NGLMESH_SKINNED            = 0x00000020,

  NGLMESH_LOD                = 0x00000100,   // Obsolete.

  NGLMESH_TEMP               = 0x00001000,   // Created in temporary memory, lost after the next call to nglListSend.
  NGLMESH_DOUBLE_BUFFER      = 0x00002000,   // NGL will double buffer the scratch mesh (to prevent modifying it while it's rendering).

  // Set by code, tool should never set.
  NGLMESH_PROCESSED          = 0x00100000,
  NGLMESH_BUFFER_OWNER       = 0x00200000,
  NGLMESH_SCRATCH_MESH       = 0x00400000,

  // Lighting categories.
  NGLMESH_LIGHTCAT_1         = 0x01000000,
  NGLMESH_LIGHTCAT_2         = 0x02000000,
  NGLMESH_LIGHTCAT_3         = 0x04000000,
  NGLMESH_LIGHTCAT_4         = 0x08000000,
  NGLMESH_LIGHTCAT_5         = 0x10000000,
  NGLMESH_LIGHTCAT_6         = 0x20000000,
  NGLMESH_LIGHTCAT_7         = 0x40000000,
  NGLMESH_LIGHTCAT_8         = 0x80000000,
};

#define NGL_LIGHTCAT_MASK      0xFF000000

struct nglMeshFileHeader
{
  u_int Tag;
  u_int Version;
  u_int NMeshes;
  u_int Pad;
};

struct nglMesh;
struct nglMeshLODInfo
{
  nglMesh* Mesh;
  float Range;
  u_int Pad[2];

  nglFixedString Name;
};

struct nglMeshBatchInfo;
struct nglMeshBatchHeader;

struct nglMeshSection
{
  nglMaterial* Material;

  // Vert/strip totals.  NTris = NVerts - NStrips * 2;
  u_int NVerts;
  u_int NStrips;

  // Exact bounding sphere.
  float SphereRadius;
  nglVector SphereCenter;

  u_int NBones;
  nglMatrix* Bones;

  u_int NBatches;
  nglMeshBatchInfo* BatchInfo;
  u_int* BatchDMA;
  u_int BatchQWC;
  u_int BatchBufSize;

	// Custom render function for the section, or NULL for the default.
  nglCustomNodeFn RenderFn;
};

// Mesh resource structure.  These are stored in the mesh instance bank.
struct nglMeshFile;
struct nglMesh
{
  u_int Flags;

  nglMeshFile* File;
  nglMesh* NextMesh;
  u_int DataSize;

  nglFixedString Name;

  nglVector SphereCenter;
  float SphereRadius;

  u_int NLODs;
  nglMeshLODInfo* LODs;

  u_int NBones;
  nglMatrix* Bones;

  u_int NSections;
  nglMeshSection* Sections;

  u_int Pad;
};

// Mesh loading functions.
bool nglLoadMeshFile( const nglFixedString& FileName );
bool nglLoadMeshFileInPlace( const nglFixedString& FileName, void* Buf );

void nglReleaseMeshFile( const nglFixedString& FileName );
void nglReleaseAllMeshFiles();

// Retrieve a mesh from a loaded file.  If the mesh can't be found it prints out a warning (which can be disabled by passing Warn=false).
nglMesh* nglGetMeshA( const char* Name, bool Warn = true );
nglMesh* nglGetMesh( const nglFixedString& Name, bool Warn = true );

// Functions to iterate through all the meshes in a file.  Call nglGetFirstMeshInFile with the file name, then call
// nglGetNextMeshInFile repeatedly with the most recent return value to iterate.
nglMesh* nglGetFirstMeshInFile( const nglFixedString& FileName );
nglMesh* nglGetNextMeshInFile( nglMesh* Mesh );

// Add a mesh to the scene.
void nglListAddMesh( nglMesh* Mesh, const nglMatrix& LocalToWorld, nglRenderParams* Params = 0 );

// Find the index of a material by ID, suitable for use with NGLP_MATERIAL_MASK.  Returns -1 if the ID can't be found.
int nglGetMaterialIdx( nglMesh* Mesh, u_int MaterialID );

// Call this function to update the texture pointers inside a mesh.  Use this if you've reloaded some textures and want to update the mesh
// to match, without reloading the mesh too.  Passing true for the Load parameter makes the function call nglLoadTexture on textures
// that aren't in the instance bank.
void nglBindMeshTextures( nglMesh* Mesh, bool Load = false );

// This function can be used to override the renderer used for a given section, and is an alternative to using custom nodes.
// The given function is called each time the section is rendered.  For a template function to base your function off,
// see nglVif1RenderBakedMeshSection.
void nglSetSectionRenderer( nglMeshSection* Section, nglCustomNodeFn RenderFn );

/*---------------------------------------------------------------------------------------------------------
  Lighting API

  Use these flags, enums and functions add lights to the scene.
---------------------------------------------------------------------------------------------------------*/
// Light flags.
enum
{
  // Lighting categories.  If a mesh and a light match one or more lighting categories, the light can affect the mesh.
  // For example - NGLLIGHT_LIGHTCAT_1 is assigned to all character meshes, and all lights designed to affect characters.
  NGLLIGHT_LIGHTCAT_1 = 0x01000000,
  NGLLIGHT_LIGHTCAT_2 = 0x02000000,
  NGLLIGHT_LIGHTCAT_3 = 0x04000000,
  NGLLIGHT_LIGHTCAT_4 = 0x08000000,
  NGLLIGHT_LIGHTCAT_5 = 0x10000000,
  NGLLIGHT_LIGHTCAT_6 = 0x20000000,
  NGLLIGHT_LIGHTCAT_7 = 0x40000000,
  NGLLIGHT_LIGHTCAT_8 = 0x80000000,
};

#define NGL_NUM_LIGHTCATS 8
#define NGL_LIGHTCAT_SHIFT 24

// Functions to add various types of lights to the scene.
void nglListAddDirLight( u_int LightCat, const nglVector& Dir, const nglVector& Color );
void nglListAddFakePointLight( u_int LightCat, const nglVector& Pos, float Near, float Far, const nglVector& Color );
void nglListAddPointLight( u_int LightCat, const nglVector& Pos, float Near, float Far, const nglVector& Color );

// Creates a new lighting context and returns the ID.  All lights added to the render list after this call will go
// into the newly created context, until a new call to nglCreateLightContext is made.  Apply a lighting context
// to a mesh using NGLP_LIGHT_CONTEXT.
nglLightContext* nglCreateLightContext();

// Switching and retrieving the current lighting context.
void nglSetLightContext( nglLightContext* Context );
nglLightContext* nglGetLightContext();

// Sets the ambient color for the current context.
void nglSetAmbientLight( float R, float G, float B, float A );

/*---------------------------------------------------------------------------------------------------------
  Projector Light API

  Projector lights project a texture onto the scene.
---------------------------------------------------------------------------------------------------------*/
#ifdef PROJECT_SPIDERMAN
void nglListAddDirProjectorLight(
  u_int LightCat, const nglMatrix& PO, const nglVector& Scale, float FadeScale, const nglVector& PosCullOffset,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );
#else
void nglListAddDirProjectorLight(
  u_int LightCat, const nglMatrix& PO, const nglVector& Scale,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );
#endif

void nglListAddSpotProjectorLight(
  u_int LightCat, const nglMatrix& PO, const nglVector& Scale, float FOV,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );

void nglListAddPointProjectorLight(
  u_int LightCat, const nglVector& Pos, const nglVector& Color, float Range,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );

/*---------------------------------------------------------------------------------------------------------
  Particle System API
---------------------------------------------------------------------------------------------------------*/
// Particle system parameters.
struct nglParticleSystem
{
  u_int MaterialFlags;
  u_int BlendMode;

  u_int Num;                    // Number of Particles
  u_int Scol;                   // Starting Color
  u_int Rcol;                   // Random Starting Color
  u_int Ecol;                   // Ending Color
  float Life;                   // Particle Lifetime
  float Rlife;                  // Random Particle Lifetime
  float Dura;                   // Duration for all particles to fire
  float Ctime;                  // Current Time
  float Ssize;                  // Starting Size
  float Rsize;                  // Random Starting Size Modifier
  float Esize;                  // Ending Size Modifier
  float MaxSize;                // Maximum Size in Screen Space
  float Aspect;                 // Aspect Ratio.

  u_int Seed;                   // Random Seed
  nglVector Spos;               // Starting Position
  nglVector Rpos1;              // Random Starting Line 1 Modifier
  nglVector Rpos2;              // Random Starting Line 2 Modifier
  nglVector Svel;               // Starting Velocity
  nglVector Rvel1;              // Random Starting Velocity Line 1 Modifier
  nglVector Rvel2;              // Random Starting Velocity Line 2 Modifier
  nglVector Rvel3;              // Random Starting Velocity Line 3 Modifier
  nglVector Force;              // Constant force ( acceleration )

  nglTexture* Tex;              // Texture map.
};

//! Adds a particle to the NGL particle system.
//! @todo To be removed.
void nglListAddParticle( nglParticleSystem* Particle );

/*! @defgroup meshedit Mesh Editing API
 * Set of functions to modify mesh resources, and to dynamically create new meshes.  Note that only one
 * mesh may be edited at a time.
 *  @{
 */

//! Create a new mesh with a set number of sections.
/*!
 * Optionally NBones and Bones can be specified for skinned meshes.
 * If NBones is >0 but Bones is not specified, the reference pose will be left uninitialized
 * (Bones also does not need to be specified if NGLP_BONES_LOCAL is used).
 * The new mesh is set up for editing automatically.
 * To get a pointer to the mesh, finish building it and call nglCloseMesh.
 * @param NSections Number of Sections in the new mesh
 * @param NBones Number of Bones inthe new mesh, defaults to zero
 * @param Bones Pointer to the Bones Matricies, defaults to NULL
 */
void nglCreateMesh( u_int NSections, u_int NBones = 0, nglMatrix* Bones = NULL );
//! Call this when you're done creating the mesh to get the finalized pointer to it.
/*!
 * @return Final Pointer to the Mesh
 * @note DON'T call this when editing a mesh.
 */
nglMesh* nglCloseMesh();
//! Destroy a non-temporary mesh.
/*! @param Mesh Pointer to the mesh to be destoryed */
void nglDestroyMesh( nglMesh* Mesh );
//! Sets the flags for the mesh (FIXDOC: which enum?  we need to ref it!)
/*! @param Flags A value representing the falgs to set on the mesh */
void nglSetMeshFlags( u_int Flags );
//! Retrieves the current mesh flags.
/*! @note Clients should always get the flags and OR in the changes before calling SetMeshFlags! */
u_int nglGetMeshFlags();
//! Call this to use the mesh editing API to edit a mesh.
/*! @param Mesh A pointer to the mesh to be edited
 *  @note Any changes will affect the actual data, and thus all instances of the mesh in the scene. */
void nglEditMesh( nglMesh* Mesh );
//! Calculate a bounding sphere for the current mesh.
/*! Will only take into account the first NVerts vertices.
 *  @param NVerts Number of verts to take into account, defaults to all verts */
void nglMeshCalcSphere( u_int NVerts = 0 );
//! Calculate a bounding sphere for the current mesh.
/*! @param Center The center position of the bounding sphere
 *  @param Radius The radius of the bounding sphere */
void nglMeshSetSphere( nglVector& Center, float Radius );
//! Add a new section to the mesh.
/*! Must be called as many times as NSections was passed to nglCreateMesh.
 *  @param Material Point to the material associated with the new section
 *  @param NVerts Number of verts to be added to the new section
 *  @param NStrips Number of strips to be added to the new section */
void nglMeshAddSection( nglMaterial* Material, u_int NVerts, u_int NStrips = 0 );
//! Jump to a particular section of the current mesh.
/*! @param Idx Zero based index to the current section */
void nglMeshSetSection( u_int Idx );
//! Jump to a particular vertex in the mesh.
/*! @param VertIdx Zero based index to the current vertex */
void nglMeshSetVertex( u_int VertIdx );
//! Scratch Vertex Structure for use with nglMeshReadVertex
struct nglScratchVertex
{
  float X, Y, Z;		//!< Position
  float NX, NY, NZ;		//!< Normals
  u_int Color;			//!< Vertex Color
  float U, V;			//!< Texture Coords
};
//! Retrieves the contents of the current vertex into a structure.
/*! @param Vertex Pointer to be filled in by function call */
void nglMeshReadVertex( nglScratchVertex* Vertex );
//! Starts a new strip with the given length.  Call this before adding vertices.
/*! @param Length Number of verticies in the new strip */
void nglMeshWriteStrip( u_int Length );

/*! @defgroup meshwrite Mesh Write Functions
 *  @ingroup meshedit
 *  These functions fill in the current section.  For newly created meshes, you must make as many WriteVertex calls as
 *  the number you passed in NVerts to nglMeshAddSection.  If you're working with a mesh resource, you can just fill in
 *  the parts you want changed (for example, using WriteVertexPN to leave Color and UV unchanged).
 *  @{
 */
void nglMeshWriteVertexPC( float X, float Y, float Z, u_int Color );
void nglMeshWriteVertexPCUV( float X, float Y, float Z, u_int Color, float U, float V );
void nglMeshWriteVertexPCUVB( float X, float Y, float Z, u_int Color, float U, float V,
                int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4);
void nglMeshWriteVertexPCUV2( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2 );
void nglMeshWriteVertexPCUV2B( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2,
                 int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4);
void nglMeshWriteVertexPUV( float X, float Y, float Z, float U, float V );
void nglMeshWriteVertexPN( float X, float Y, float Z, float NX, float NY, float NZ );
void nglMeshWriteVertexPNC( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color );
void nglMeshWriteVertexPNCUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, float U, float V );
void nglMeshWriteVertexPNUV( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V );
void nglMeshWriteVertexPNUVB( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V,
                int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4);
void nglMeshWriteVertexPNUV2( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2 );
void nglMeshWriteVertexPNUV2B( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2,
                 int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4);
/*! @} */ // end of meshwrite

/*! @} */ // end of meshedit

/*---------------------------------------------------------------------------------------------------------
  Mesh Blending API

  Function to destructively blend a mesh by a set of morph targets.
---------------------------------------------------------------------------------------------------------*/

// Makes a deep copy of a mesh, including all vertex data etc.
nglMesh* nglCreateMeshCopy( nglMesh* SrcMesh );

//
// Blends a morph target with a base mesh by a given amount.  To apply more than one MorphTarget, call
// repeatedly but pass DestMesh in as both SrcMesh and DestMesh on subsequent calls.
//
// ComponentMask specifies what parts of the vertex format (XYZ, Normals, Color, Bone Data, etc) is morphed.
// SectionMask is a 64bit value with a bit for each section.  If the bit corresponding to a section is set,
// the section is skipped by the morphing.
//
// All the components currently supported by the morphing.
enum
{
  NGLMC_XYZ     = 0x1,
  NGLMC_NORMAL  = 0x2,
  NGLMC_UV      = 0x3,
  NGLMC_COLOR   = 0x4,
  NGLMC_ALL     = 0xFFFFFFFF
};

struct nglMorphTargetSection
{
  u_int NBatches;
  nglMeshBatchInfo* BatchInfo;
};

struct nglMorphTarget
{
  u_int Size;

  u_int ComponentMask;
  u_long SectionMask;

  u_int NSections;
  nglMorphTargetSection* Sections;
};

void nglMorphMesh( nglMesh* DestMesh, nglMesh* SrcMesh, nglMorphTarget* Morph, float Weight, u_int ComponentMask = NGLMC_ALL, u_long SectionMask = 0 );

//
// 'Subtracts' two meshes to create a new morph target from Src to Dest.  The two meshes must be
// identical in terms of vertex count, vertex order, etc.
//
// Certain meshcvt options can introduce differences into otherwise identical meshes, so be careful
// about things like subdivision and UV normalization.
nglMorphTarget* nglCreateMorphTarget( nglMesh* SrcMesh, nglMesh* DestMesh, u_int ComponentMask = NGLMC_ALL, u_long SectionMask = 0 );

void nglDestroyMorphTarget( nglMorphTarget* Morph );

/*---------------------------------------------------------------------------------------------------------
  Quad API

  Quads are 2D interface graphics that can be drawn in the scene.  Construct nglQuad structures
  using the following functions and pass them to nglListAddQuad.
---------------------------------------------------------------------------------------------------------*/
// Map flags.
enum
{
	// Filtering mode.  
	NGLMAP_POINT_FILTER				= 0x00000001,
	NGLMAP_BILINEAR_FILTER    = 0x00000002,
	NGLMAP_TRILINEAR_FILTER   = 0x00000004,
	NGLMAP_ANISOTROPIC_FILTER = NGLMAP_TRILINEAR_FILTER,	  // Provided for compatibility.

	// Clamping mode. Wraps if not set.
	NGLMAP_CLAMP_U            = 0x00000010,
	NGLMAP_CLAMP_V            = 0x00000020,
};

struct nglQuadVertex
{
  float X, Y;
  float U, V;
  u_int Color;
};

struct nglQuad
{
  // Vertex information.
  nglQuadVertex Verts[4];
  float Z;

  u_int MapFlags;
  u_int BlendMode;
  u_int BlendModeConstant;
  nglTexture* Tex;
};

void nglInitQuad( nglQuad* Quad );  // Sets UV 0-1, Z=0, empty rect, white color, NGLBM_BLEND, no texture.

void nglSetQuadZ( nglQuad* Quad, float z );                                   // Set Z in view space units.

// Simplified API.
void nglSetQuadRect( nglQuad* Quad, float x1, float y1, float x2, float y2 ); // Set rectangle position.
void nglSetQuadPos( nglQuad* Quad, float x, float y );                        // Set position.  Gets W/H from the texture (must already be set).

void nglSetQuadUV( nglQuad* Quad, float u1, float v1, float u2, float v2 );   // Set rectangle UVs: Top Left, Bottom Right.

void nglSetQuadColor( nglQuad* Quad, u_int c );                               // Set color - use the NGL_RGBA32 macro.

void nglSetQuadMapFlags( nglQuad *Quad, u_int MapFlags );					  // Set Map flags for clamping, filtering, etc.
void nglSetQuadBlend( nglQuad* Quad, u_int Blend, u_int Constant = 0 );       // Set blend mode.
void nglSetQuadTex( nglQuad* Quad, nglTexture* Tex );                         // Set texture.

// Vertex API.  Verts go in this order: Top Left, Top Right, Bottom Left, Bottom Right.
void nglSetQuadVPos( nglQuad* Quad, int VertIdx, float x, float y );          // Set vertex position.
void nglSetQuadVUV( nglQuad* Quad, int VertIdx, float u, float v );           // Set vertex UV.
void nglSetQuadVColor( nglQuad* Quad, int VertIdx, u_int Color );             // Set vertex Color.

// Rotate quad counterclockwise around (cx,cy) in screen coordinates by theta radians.
void nglRotateQuad( nglQuad* Quad, float cx, float cy, float theta );

// Scale quad around (cx,cy) in screen coordinates by scale (sx,sy).
void nglScaleQuad( nglQuad* Quad, float cx, float cy, float sx, float sy );

void nglListAddQuad( nglQuad* Quad );

/*---------------------------------------------------------------------------------------------------------
  Font API.

  Fonts are created using MakeFont, a found in ngl\working\tools\bin.  You select a TrueType font and
  pick some effects, and then export a .FDF file and a .TGA file.

  First, convert the .TGA file to the appropriate platform specific format, then load it using
  nglLoadTexture.  Then, load the .FDF file using nglLoadFont.  The texture must be loaded first.

  For debugging output, you can just use nglSysFont.
---------------------------------------------------------------------------------------------------------*/
struct nglFont;

// NGL System font.
extern nglFont* nglSysFont;

// Load a font (from a .fdf file) and return a pointer to it.
// The texture font must be loaded before calling this function.
nglFont* nglLoadFont(const nglFixedString& FontName);

// Load a font from memory (must point to a fdf file) and return a pointer to the loaded font.
// The texture font must be loaded before calling this function.
nglFont* nglLoadFontInPlace(const nglFixedString& FontName, void* FDFData);

nglFont* nglGetFont( const nglFixedString& FileName );

void nglReleaseFont(nglFont* Font);
void nglReleaseAllFonts();

// Set the font's blend mode.
void nglSetFontBlend(nglFont* Font, u_int BlendMode, u_int Constant = 0);

// Set the font's map flags. See NGLMAP_xxx for more details.
void nglSetFontMapFlags(nglFont* Font, u_int MapFlags);

// Return the font's map flags. See NGLMAP_xxx for more details.
u_int nglGetFontMapFlags(nglFont* Font);

// Add a string to the nodes' list to be rendered. The string may contains some formatting informations:
// - The current font color is set by the token (hexa): \1[RRGGBBAA]
// - The current font scale is set by the token: \2[SCALE]
// - Font independent scale is set by the token: \3[SCALEX,SCALEY]
// The default color is white and the default scale is 1.0. For example:
// "Hi from \1[ff0000ff]NGL \1[ffffffff] :-)\n\n\2[2.0]Bye!");
// Writes "NGL" in red and "Bye" with a scale of 2.0.
//

// ---------------------------------------------------------------------------------------------
// The "z" parameter has been changed to "double" on this function, to compensate for a compiler bug.  
// The bug is described in the following post from SN:
//
// Just a quick note to warn users about a compiler problem. As far as I can
// see the problem exits in the current SN 295, Sony 296 and Sony GCC3
// toolchains;
// 
// If you declare a function that takes an odd number of single precision float
// arguments *and* takes var args, the compiler generates incorrect stacks
// offsets and a save of the FP regs clobbers the top of the stack.
// 
// e.g.
// 
// bool MyFunc(int x, int y, float f, const char* format, ...);
// 
// Not stunningly common I would think, but as in all stack clobber situations,
// it could produce difficult to track down effects.
// 
// The bug has been fixed and will appear in the next release of the SN 2.95.3
// compiler and the upcoming SN 3.0.3 compiler.
// 
// 
// Phil Camp
// Build Tools Team
// 
// ---------------------------------------------------------------------------------------------
void nglListAddString(nglFont* Font, float x, float y, double z, const char* Text, ...);

// Version with an initial color (to avoid sprintf madness).
void nglListAddString( nglFont* Font, float x, float y, double z, u_int Color, const char* Text, ... );

// Get the dimension, in pixels, of a string (which can have scale tokens and cariage returns as well).
void nglGetStringDimensions(nglFont* Font, u_int* Width, u_int* Height, const char* Text, ...);

#endif
