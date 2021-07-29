/*---------------------------------------------------------------------------------------------------------
  NGL_GC - GC implementation of NGL

  Maintained by Michael Montague (mikem@treyarch.com)
  (c)2001 Treyarch LLC.
---------------------------------------------------------------------------------------------------------*/
#ifndef NGL_H
#define NGL_H

/*---------------------------------------------------------------------------------------------------------
  Defines and types for projects that conditionally compile NGL.
---------------------------------------------------------------------------------------------------------*/

#include "ngl_version.h"

#define NGL_BIG_ENDIAN

#ifdef DEBUG
#define NGL_DEBUG
#endif

#ifdef NGL_DEBUG
#define NGL_INLINE
#else
#define NGL_INLINE inline
#endif

#define NGL_MESHFILE_EXT ".gcmesh"

// Takes 4 integers ranging from 0-255 and returns a packed color value.
#define NGL_RGBA32( r, g, b, a ) ((((u_int)(r)&0xFF)<<24)|(((u_int)(g)&0xFF)<<16)|(((u_int)(b)&0xFF)<<8)|((u_int)(a)&0xFF))

/*---------------------------------------------------------------------------------------------------------
  General platform independent types.
---------------------------------------------------------------------------------------------------------*/
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef unsigned long  u_long128;

/*---------------------------------------------------------------------------------------------------------
  Includes
---------------------------------------------------------------------------------------------------------*/
#include <dolphin/gx.h>

#include "ngl_fixedstr.h"

/*---------------------------------------------------------------------------------------------------------
  Version functions
---------------------------------------------------------------------------------------------------------*/
static NGL_INLINE u_int nglGetVersion()	{ return NGL; }

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
        nglVector( const nglVector& v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w ) {}
        nglVector( float _x, float _y, float _z, float _w = 1.0f ) : x( _x ), y( _y ), z( _z ), w( _w ) {}

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
}; // __attribute__((aligned (16)));

class nglMatrix
{
public:
  	nglVector x, y, z, w;

  	nglMatrix() {}
        nglMatrix( const nglMatrix& m ) : x ( m.x ), y( m.y ), z( m.z ), w( m.w ) {}
        nglMatrix( const nglVector& _x, const nglVector& _y, const nglVector& _z, const nglVector& _w ) :
        x( _x ), y( _y ), z( _z ), w( _w ) {}

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
}; // __attribute__((aligned (16)));

const nglMatrix nglUnitMatrix = nglMatrix(
	nglVector( 1.0f, 0.0f, 0.0f, 0.0f ),
	nglVector( 0.0f, 1.0f, 0.0f, 0.0f ),
	nglVector( 0.0f, 0.0f, 1.0f, 0.0f ),
	nglVector( 0.0f, 0.0f, 0.0f, 1.0f ) );

void nglNormalize( nglVector& dst, const nglVector src );
void nglCopyVector( nglVector& dst, const nglVector src );
void nglScaleVector( nglVector& dst, const nglVector src, float scale );
void nglSubVector( nglVector& dst, const nglVector lhs, const nglVector rhs );
void nglSetVector( nglVector& dst, float x, float y, float z, float w );
void nglIdentityMatrix( nglMatrix& matrix );
void nglMulMatrix( nglMatrix& dst, const nglMatrix lhs, const nglMatrix rhs );
void nglCameraMatrix( nglMatrix& dst, const nglVector pos, const nglVector up, const nglVector at );
void nglTransposeMatrix( nglMatrix& DstMat, nglMatrix SrcMat );
void nglMatrixRot(nglMatrix& Out, const nglMatrix In, const nglVector rot);
void nglInverseMatrix( nglMatrix& dst, const nglMatrix src );
void nglApplyMatrix( nglVector& dst, const nglMatrix m, const nglVector v );

/*---------------------------------------------------------------------------------------------------------
  System Callback API.

  Allows the game to override various system operations within NGL.  Defaults are provided.
---------------------------------------------------------------------------------------------------------*/
// File buffer for callback functions to fill.
struct nglFileBuf
{
  	u_char* Buf;
  	u_int Size;
  	u_int UserData;
};

// Structure of function pointers for NGL system callbacks.
struct nglSystemCallbacks
{
  	bool (*ReadFile)( const char* FileName, nglFileBuf* File, u_int Align );
  	void (*ReleaseFile)( nglFileBuf* File );
  	void (*CriticalError)( const char* Text );
  	void (*DebugPrint)( const char* Text );
  	void* (*MemAlloc)( u_int Size, u_int Align );
  	void (*MemFree)( void* Ptr );
};

/*---------------------------------------------------------------------------------------------------------
  nglSetSystemCallbacks

  Overrides default system operations.  Passing in a structure of function pointers will override the
  current system callbacks and cause NGL to use the new ones for all operations.

  NULL can passed in any of the pointers, which will cause the NGL default implementation to be used.
---------------------------------------------------------------------------------------------------------*/
void nglSetSystemCallbacks(nglSystemCallbacks* Callbacks );

/*---------------------------------------------------------------------------------------------------------
  Default system callback functions.
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

void nglFatal( const char* Format, ... );
bool nglReadFile( const char* FileName, nglFileBuf* File, u_int Align = 1 );
void nglReleaseFile( nglFileBuf* File );
void* nglMemAlloc( u_int Size, u_int Align = 0 );
void nglMemFree( void* Ptr );

/*---------------------------------------------------------------------------------------------------------
  General Rendering API
---------------------------------------------------------------------------------------------------------*/
void nglInit();
void nglExit();

// Buffer size control.  All of these except NGLBUF_LIST_WORK and NGLBUF_GX_FIFO are double buffered, so the
// size passed here refers to the size of a single buffer (IE twice the passed amount of memory is allocated).
// Can be called before or after nglInit.  (Except NGLBUF_GX_FIFO)
enum
{
	NGLBUF_GX_FIFO,				// Buffer for FIFO that stores GX commands
  	NGLBUF_LIST_WORK,           // Buffer for temporary render list objects such as scenes, mesh nodes, and lights.
};
void nglSetBufferSize(u_int BufID, u_int Size);

void nglFlip();

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

u_int nglGetVBlankCount();

/*---------------------------------------------------------------------------------------------------------
  Render List API

  The Render List is the core of NGL's rendering API.  Use these functions to construct a render list,
  which is passed to the hardware via nglListSend.
---------------------------------------------------------------------------------------------------------*/
// Initialize the scene list, and the enters main scene.
void nglListInit();

// Sends the list of scenes to the hardware.
void nglListSend( bool Flip = false );

// Scenes
// A global default scene is automatically created and selected by nglListInit().
// Whenever a new scene is created with nglListBeginScene(), it takes its
//  initial parameter settings from the default scene.
// Many ngl api calls implicitly reference the currently selected scene.
// These include calls to set scene parameters or to add objects to be rendered in the scene.
class nglScene;
// nglListBeginScene creates a child of the currently selected scene and returns it.
// The returned nglScene is only valid between nglListInit() and nglListSend(),
// after which it is implicitly deleted.
nglScene* nglListBeginScene();
// nglListEndScene selects the parent of the current scene to be the new current scene.
void nglListEndScene();
// nglListSelectScene allows the specified scene to be selected as the current scene.
// The previous current scene is returned.
nglScene* nglListSelectScene(nglScene *scene);


// Specify a texture to render the scene to.
struct nglTexture;
void nglSetRenderTarget( nglTexture* Tex, bool Dummy = false );

// Set up for orthogonal rendering.
void nglSetOrthoMatrix(float t, float b, float l, float r, float nearz, float farz);
static NGL_INLINE void nglSetOrthoMatrix(float nearz, float farz)
	{ nglSetOrthoMatrix(-1.0f, 1.0f, -1.0f, 1.0f, nearz, farz); }



// Set up for perspective rendering.  hfov is horizontal fov in degrees (example 90.0f).
void nglSetPerspectiveMatrix(float hfov, float nearz, float farz);
#ifdef PROJECT_KELLYSLATER
void nglSetPerspectiveCXCY( float cx, float cy );
#endif

void nglGetProjectionParams(float* hfov, float* nearz, float* farz);

// Scene camera related functions.
void nglSetWorldToViewMatrix( const nglMatrix WorldToView );

// Test a sphere to see if it's in the view frustum.
// -1 = Rejected, 0 = Not Clipped, 1 = Guard band intersection.
int nglGetClipResult( nglVector Center, float Radius );

// Clear flags.
enum
{
	NGLCLEAR_NONE 	 = 0x0000,
  	NGLCLEAR_COLOR 	 = 0x0001,
  	NGLCLEAR_Z 		 = 0x0002,
	NGLCLEAR_STENCIL = 0x0000,
};

// Set screen/texture clear parameters for the scene.
void nglSetClearFlags( u_int ClearFlags );
void nglSetClearColor( float R, float G, float B, float A );
void nglSetClearZ( float Z );   // Z is in hardware space (0.0-nearest, 1.0-farthest).

// Enable/disable z-buffer writes for the current scene.  Enabled by default.
void nglSetZWriteEnable(bool Enable);

// Enable/disable z-buffer testing for the current scene.  Enabled by default.
void nglSetZTestEnable( bool Enable );

// Screen-space view rectangle for the scene.  Defaults are 0, 0, ScreenWidth - 1, ScreenHeight - 1.
void nglSetViewport( float x1, float y1, float x2, float y2 );

// Fog parameters for the scene.
void nglEnableFog(bool bEnable);
void nglSetFogColor( float R, float G, float B );
void nglSetFogRange( float Near, float Far, float Min, float Max );

// Returns one of the current matrices in the model->view->projection chain.
enum
{
  	NGLMTX_VIEW_TO_WORLD,
  	NGLMTX_VIEW_TO_SCREEN,
  	NGLMTX_WORLD_TO_VIEW,
  	NGLMTX_WORLD_TO_SCREEN,
};

void nglGetMatrix( nglMatrix& Dest, u_int ID );
void nglProjectPoint(nglVector& Out, nglVector In);

/*---------------------------------------------------------------------------------------------------------
  Custom Node API
---------------------------------------------------------------------------------------------------------*/
// Custom node callback function.
typedef void (*nglCustomNodeFn)( void* Data );

enum
{
  	NGLSORT_TRANSLUCENT,
  	NGLSORT_TEXTURED,
  	NGLSORT_UNTEXTURED
};

struct nglTexture;
struct nglSortInfo
{
  	u_int Type;
  	float Dist;           // Sorting distance for NGLCN_ALPHA.
  	u_int Hash;           // Texture map's hash value for NGLCN_TEXTURED.
};

void nglListAddCustomNode( nglCustomNodeFn Fn, void* Data, nglSortInfo* Info );

/*---------------------------------------------------------------------------------------------------------
  Texture API
---------------------------------------------------------------------------------------------------------*/

enum TextureTypes
{
	NGLTEX_GCT,     // Gamecube swizzled and/or compressed texture
  	NGLTEX_IFL,     // IFL files aren't actual textures, but arrays of texture pointers in an animated sequence.
  	NGLTEX_ATE,     // ATE files aren't actual textures, but arrays of texture pointers in an animated sequence.
};

enum TextureFlags
{
	NGLGC_TEXFLAG_NULL = 0x0000,
	NGLGC_TEXFLAG_UCLAMP = 0x0001,	// Force U clamping.
	NGLGC_TEXFLAG_VCLAMP = 0x0002,	// Force V clamping.
	NGLGC_TEXFLAG_SPECIAL_BLEND1 = 0x0004, // Just for testing for now.
	NGLGC_TEXFLAG_SPECIAL_BLEND2 = 0x0008,
	NGLGC_TEXFLAG_SPECIAL_BLEND3 = 0x0010,
	NGLGC_TEXFLAG_SPECIAL_BLEND4 = 0x0010,
	NGLGC_TEXFLAG_SYSTEM = 0x8000,
};


// Texture resource structure.  These are stored in the texture instance bank.
struct nglTexture
{
	// Generic format data
  	u_short Type;                 // Texture type (see above enum).
  	u_short Flags;							// Special Flags for texture processing.
  	u_int Width, Height;        // Texture dimensions.
  	unsigned char* Data;        // "Base" texture data (headers, etc.)
	unsigned char* ImageData;   // Beginning of texture image data (followed by mip-maps)
  	u_int PaletteSize;          // Number of entries in palette
	unsigned char* PaletteData; // Palette data (NULL if no palette)
	u_int Mipmaps;              // Number of mipmaps

	// GCT format data:
	u_int TexelFormat;          // Format of texel data
	u_int PaletteFormat;        // Format of LUT data

  	// IFL format data:
  	u_int NFrames;              // If this member is greater than 1, this is an animated texture.
  	nglTexture** Frames;        // Array of texture pointers for the individual frames (which are also stored in the instance bank).

  	// Bookkeeping, non cache-critical.
  	u_int Hash;                 // Name hash for sorting into render bins.
  	nglFixedString FileName;    // Original filename.

  	// GCN specific data
  	GXTexObj TexObj;
  	GXTlutObj TlutObj;
};

nglTexture* nglLoadTexture( const nglFixedString& FileName );
nglTexture* nglLoadTextureA( const char* FileName );

nglTexture* nglGetTexture( const nglFixedString& FileName );
nglTexture* nglGetTextureA( const char* FileName );

// these are suitable replacements until we implement the real thing
static NGL_INLINE nglTexture* nglGetFrontBufferTex() { return NULL; }
static NGL_INLINE nglTexture* nglGetBackBufferTex()  { return NULL; }

// Add a reference to a texture in the instance bank.
void nglAddTextureRef( nglTexture* Tex );

// Load a texture and lock immediately.   Locked textures have space allocated in VRAM and
// can be used for Quads.  Unlocked textures are streamed as needed by the mesh renderer.
nglTexture* nglLoadTextureLock( const char* FileName );
nglTexture* nglLoadTextureFromMem( const nglFixedString& FileName, int Type, nglFileBuf &File );
nglTexture* nglLoadTextureInPlace( const nglFixedString& FileName, u_int Type, void* Data, u_int Size );

void nglReleaseTexture( nglTexture* Tex );
void nglReleaseAllTextures();

// Forces a texture to stay VRAM between frames.  Designed for load-time only.
void nglLockTexture( nglTexture* Tex );
void nglUnlockTexture( nglTexture* Tex );

void nglUnlockAllTextures();
void nglRelockAllTextures();

// Texture creation API.
enum
{
  	// Enumerated format values.
  	NGLTF_32BIT         = 0x0001,
  	NGLTF_16BIT         = 0x0002,

  	// Flags to be OR'd into above values.
  	NGLTF_TEMP          = 0x1000,   // Texture structure is allocated in nglListWork, lost at the end of the frame.
  	NGLTF_RENDER_TARGET = 0x2000,   // Specify this flag if you intend to use this texture as a render target.

	// xbox compatibility functions
	NGLTF_SWIZZLED      = 0x0000,
	NGLTF_LINEAR        = 0x0000,
};

nglTexture* nglCreateTexture( u_int Format, u_int Width, u_int Height );
void nglDestroyTexture( nglTexture* Tex );

/*---------------------------------------------------------------------------------------------------------
  Mesh API
---------------------------------------------------------------------------------------------------------*/
// Material flags.
enum
{
  	// Translucency.
  	NGLMAT_ALPHA               = 0x00000001,   // Material is translucent, vertex alpha is used.
  	NGLMAT_ALPHA_SORT_FIRST    = 0x00000002,   // Causes non-additive alpha polys to go in the additive list, saves sort time.

  	NGLMAT_ALPHA_FALLOFF       = 0x00000004,
  	NGLMAT_ALPHA_FALLOFF_OUT   = 0x00000008,

  	// Material passes.
  	NGLMAT_TEXTURE_MAP         = 0x00000010,
  	NGLMAT_LIGHT_MAP           = 0x00000020,
  	NGLMAT_DETAIL_MAP          = 0x00000040,
  	NGLMAT_ENVIRONMENT_MAP     = 0x00000080,

  	NGLMAT_BUMP_MAP            = 0x00000100,   // Replaces the detail map with a bump map.

  	// Clipping stuff.
  	NGLMAT_BACKFACE_DEFAULT    = 0x00000400,
  	NGLMAT_BACKFACE_CULL       = 0x00000800,   // Clip polygons that are facing away from the camera.

  	// Filtering.
  	NGLMAT_BILINEAR_FILTER     = 0x00001000,
  	NGLMAT_TRILINEAR_FILTER    = 0x00002000,
  	NGLMAT_ANTIALIAS           = 0x00004000,
//  	NGLMAT_PERSPECTIVE_CORRECT = 0x00008000,

  	NGLMAT_CLAMP_U             = 0x00010000,
  	NGLMAT_CLAMP_V             = 0x00020000,

  	NGLMAT_FOG                 = 0x00040000,

  	NGLMAT_LIGHT               = 0x00100000,   // Self illuminated if this flag is off.
  	NGLMAT_UV_SCROLL           = 0x00200000,   // Automatic UV scrolling.

	NGLMAT_ENV_SPECULAR        = 0x00400000,   // Replaces the environment map with a specular highlight pass.
  	NGLMAT_ENV_CYLINDER        = 0x00800000,   // Specifies cylindrical environment mapping.

  	// New clamping for diffuse2 layer.
  	NGLMAT_LIGHT_CLAMP_U       = 0x01000000,
  	NGLMAT_LIGHT_CLAMP_V       = 0x02000000,

#ifdef PROJECT_KELLYSLATER
  	NGLMAT_WAVETRANS           = 0x08000000,
#endif

  	NGLMAT_MATERIAL_COLOR      = 0x10000000,   // Set to force the first pass to use the material color instead of the vertex color.
};

#define NGL_LIGHTCAT_MASK      0xFF000000
#define NGL_TEXTURE_MASK       ( NGLMAT_TEXTURE_MAP | NGLMAT_LIGHT_MAP | NGLMAT_DETAIL_MAP | NGLMAT_ENVIRONMENT_MAP )

// Material blend modes.
enum
{
  	NGLBM_OPAQUE,                       // No translucency is performed, alpha is ignored.
  	NGLBM_PUNCHTHROUGH,                 // Similar to opaque, except if alpha is below a threshold the pixel is skipped.

  	NGLBM_BLEND,                        // Blends the texel with the background, modulated by Alpha.
  	NGLBM_ADDITIVE,                     // Adds the texel to the background, modulated by Alpha.
  	NGLBM_SUBTRACTIVE,                  // Subtracts the texel from the background, modulated by Alpha.

  	NGLBM_CONST_BLEND,                  // Blends the texel with the background, modulated by BlendModeConstant.
  	NGLBM_CONST_ADDITIVE,               // Adds the texel to the background, modulated by BlendModeConstant.
  	NGLBM_CONST_SUBTRACTIVE,            // Subtracts the texel from the background, modulated by BlendModeConstant.
  	NGLBM_ADDSIGNED,					// Used for detail textures. Standard implementation.

	NGLBM_PROJ_SHADOW_S1,				// Two stages for rendering one shadow map.
	NGLBM_PROJ_SHADOW_S2,

	NGLBM_MOTION_BLUR, 					// For rendering the previous FrameBuffer into the current one.
	NGLBM_KSOPAQUE,
  	NGLBM_MAX_BLEND_MODES,
	NGLBM_GC_LENSFLARE = 0x0100,
	NGLBM_GC_SPECIAL1 = 0x010000,
	NGLBM_GC_SPECIAL2,
	NGLBM_GC_SPECIAL3,
	NGLBM_GC_SPECIAL4,
	NGLBM_GC_SPECIAL5,
};

#pragma pack(push,1)
// Mesh material structure.
struct nglMaterial
{
  	u_int Flags;
  	u_int MapFlags;
  	u_int Pad2;
  	u_int Pad3;

	nglFixedString MapName;
	nglTexture* Map;
  	u_int MapBlendMode;
  	float MapBlendModeConstant;

	nglFixedString LightMapName;
	nglTexture* LightMap;
  	u_int LightMapBlendMode;
  	float LightMapBlendModeConstant;

	nglFixedString DetailMapName;
	nglTexture* DetailMap;
  	u_int DetailMapBlendMode;
  	float DetailMapBlendModeConstant;
  	float DetailMapUScale;
  	float DetailMapVScale;
  	float DetailMapRange;
  	float DetailMapAlphaClamp;

	nglFixedString EnvironmentMapName;
	nglTexture* EnvironmentMap;
  	u_int EnvironmentMapBlendMode;
  	float EnvironmentMapBlendModeConstant;

  	u_int Color;
	u_int MaterialId;
  	float AlphaFalloff;
  	float ForcedSortDistance;
  	float ScrollU, ScrollV;
};
#pragma pack(pop)

enum
{
  	NGLP_TINT                 = 0x00000001,   // Multiply vertex colors by a tint color, before lighting.
  	NGLP_BONES                = 0x00000002,   // Means that bone positions are specified in the render parameters.
  	NGLP_BONES_WORLD          = 0x00000004,   // Bone positions are in world space instead of local space.
  	NGLP_BONES_LOCAL          = 0x00000008,   // Bone positions are relative to the defaults.
  	NGLP_FULL_MATRIX          = 0x00000010,   // LocalToWorld is the complete to screen space matrix and no clipping will be performed.
  	NGLP_SCALE                = 0x00000040,   // A non-uniform scale is to be applied and the Scale member is valid.
  	NGLP_TEXTURE_FRAME        = 0x00000080,   // TextureFrame member is valid and should be used to override the default.
  	NGLP_TEXTURE_SCROLL       = 0x00000100,   // ScrollU and ScrollV members are valid and should be used to scroll the texture.
  	NGLP_MATERIAL_MASK        = 0x00000200,   // Mask off materials by index.
  	NGLP_ZBIAS                = 0x00000400,   // Offset Z coordinates in the ZBuffer (doesn't affect display).
  	NGLP_NO_CULLING           = 0x00000800,   // Turns off view frustum culling, assumes that the game code has already done it.
  	NGLP_LIGHT_CONTEXT        = 0x00001000,   // LightContext member is valid and specifies the lighting context that the mesh should use.
  	NGLP_NO_LIGHTING          = 0x00002000,   // Disable lighting for this instance.
  	NGLP_REVERSE_BACKFACECULL = 0x00004000,   //

  	NGLP_NO_VCFIX			  = 0x00008000,    // No perspective correct vertex coloring workaround for the hardware color interpolation bug.
  	NGLP_KSPECIAL_OPAQUE    = 0x00010000,    // Place Holder.
  	NGLP_SPECIAL_02		      = 0x00020000,    // Place Holder.
  	NGLP_SPECIAL_03		      = 0x00040000,    // Place Holder.
  	NGLP_SPECIAL_04		      = 0x00080000,    // Place Holder.
};

struct nglLightContext;

struct nglRenderParams
{
  	u_int Flags;                    	// Flags that specify what members are valid.
  	u_int TextureFrame;             	// Frame value for animated textures.
  	float ScrollU, ScrollV;         	// Texture scrolling amount.
  	nglVector TintColor;            	// Color scale to be multiplied by vertex colors - ranges from 0.0 to 1.0.
  	nglVector Scale;                	// 3D non-uniform scale factor.  Do not apply this to the matrix!
  	u_int MaterialMask;                 // 32bit mask to selectively disable materials by ID.
  	float ZBias;						// Determines a scale for Z buffer
  	nglLightContext *LightContext;					// ID of the lighting context to apply to this mesh.

  	// Bone transforms for skinned meshes.
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

  	NGLMESH_LOD                = 0x00000100,

  	NGLMESH_TEMP               = 0x00001000,   // Created in temporary memory, lost after the next call to nglListSend.
  	NGLMESH_STATIC             = 0x00002000,   // Copied to allocated memory, must be destroyed

  	// Set by code, tool should never set.
  	NGLMESH_PROCESSED          = 0x00100000,
  	NGLMESH_BUFFER_OWNER       = 0x00200000,
  	NGLMESH_SCRATCH            = 0x00400000,

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

// Mesh resource structure.  These are stored in the mesh instance bank.
#pragma pack(push,1)
struct nglMeshFileHeader
{
  	char  Tag[4];
  	u_int Version;
  	u_int NMeshes;
  	u_int Size;
};

struct nglMesh;

struct nglMeshLODInfo
{
  	nglMesh* Mesh;
  	float Range;
  	nglFixedString Name;
};

struct nglMeshSection
{
	nglMaterial* Material;

	float SphereRadius;
	nglVector SphereCenter;

	u_int NBones;
	nglMatrix* Bones;

	u_int NIndices;
	u_short* Indices;

	u_int NVerts;
	void* Verts;
	int* BoneIndexCounts;	//  Due to a split in the section for optimizing SkinnedMeshes.
	u_int NCPUVerts;	//  The verts that have to be transformed in the CPU (for skinning).
	u_int NTris;
	short *VertMap; // Verts mapped to VertPNs.
	u_int NVertPNs; // The 'position and normal' info of the verts is stored separately
	void* VertPNs; //  from the rest of the vertex and can be common for more than one vertex.
	u_int NVertNs;// VertNs is the array of normals.
	void* VertNs;
	u_int NVertDs; // VertNs is the array of colors and UVs.
	void* VertDs;
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

	u_int	NBones;
	nglMatrix* Bones;

	u_int	NSections;
	nglMeshSection* Sections;

  	u_int Pad;
};
#pragma pack(pop)

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


nglMesh* nglLoadMesh( const char* Name );
void nglReleaseMesh( nglMesh* Mesh );
void nglReleaseAllMeshes();

// Finds the index of a material by ID, suitable for use with NGLP_MATERIAL_MASK.  Returns -1 if the ID can't be found.
int nglGetMaterialIdx( nglMesh* Mesh, u_int MaterialID );

// Add a reference to a mesh in the instance bank.
void nglAddMeshRef( nglMesh* Tex );

void nglListAddMesh( nglMesh* Mesh, const nglMatrix LocalToWorld, nglRenderParams* Params = 0 );

/*---------------------------------------------------------------------------------------------------------
  Lighting API

  Use these structs, flags and enums to create a nglLightInfo struct which can be passed to nglListAddLight.
---------------------------------------------------------------------------------------------------------*/
// Light flags.
enum
{
  	// Lighting categories.
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

enum
{
  	// Standard lights.
  	NGLLIGHT_FAKEPOINT,        // Point light faked by a directional light.
  	NGLLIGHT_TRUEPOINT,        // True vertex accurate point light.
  	NGLLIGHT_DIRECTIONAL,      // General directional light.

  	// Projected texture lights.
  	NGLLIGHT_PROJECTED_DIRECTIONAL,
  	NGLLIGHT_PROJECTED_SPOT,
  	NGLLIGHT_PROJECTED_POINT,
};

// Functions to add various types of lights to the scene.
void nglListAddDirLight(u_int LightCat, nglVector Dir, nglVector Color);
void nglListAddFakePointLight(u_int LightCat, nglVector Pos, float Near, float Far, nglVector Color);
void nglListAddPointLight(u_int LightCat, nglVector Pos, float Near, float Far, nglVector Color);

// Creates a new lighting context and returns the ID.  All lights added to the render list after this call
// will go  into the newly created context, until a new call to nglCreateLightContext is made.
nglLightContext *nglCreateLightContext();

// Switching and retrieving the current lighting context.
void nglSetLightContext( nglLightContext *Context );
nglLightContext *nglGetLightContext();

// Sets the ambient color for the current context.
void nglSetAmbientLight(float R, float G, float B);

struct nglLightInfo
{
  	int Type;
  	u_int Flags;

  	// Color parameters.
  	nglVector ColorAdd;
  	nglVector ColorMul;

  	// Falloff parameters - must not change order or size.
  	float DistFalloffStart;
  	float DistFalloffEnd;
};

//Projector Light
typedef nglVector nglPlane;

enum
{
  	NGLFRUSTUM_TOP,
  	NGLFRUSTUM_BOTTOM,
  	NGLFRUSTUM_LEFT,
  	NGLFRUSTUM_RIGHT,
  	NGLFRUSTUM_NEAR,
  	NGLFRUSTUM_FAR
};

struct nglFrustum
{
  	// these planes are in world space:
  	nglPlane Planes[6];//left, right, back, top, bottom, front;
};

struct nglProjectorLightInfo
{
  	int Type;
  	u_int Flags;

  	nglTexture *Texture;

  	float Fov;

  	nglVector Pos;

  	nglVector Xaxis;
  	nglVector Yaxis;
  	nglVector Zaxis;

  	// for scaling in the U,V directions...and the z component can be used for falloff computations
  	nglVector Scale;

  	// the matrix for going from world to uv space
  	nglMatrix WorldToUV;

  	// the matrix for going from uv to world space
  	nglMatrix UVToWorld;

  	// the frustum object for this light/projector
  	nglFrustum Frustum;
};

void nglListAddProjectorLight( nglProjectorLightInfo* Light, nglMatrix po );

/*---------------------------------------------------------------------------------------------------------
  Projector Light API

  Projector lights project a texture onto the scene.
---------------------------------------------------------------------------------------------------------*/
void nglListAddDirProjectorLight(
  u_int LightCat, nglMatrix PO, nglVector Scale,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );

void nglListAddSpotProjectorLight(
  u_int LightCat, nglMatrix PO, nglVector Scale, float FOV,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );

void nglListAddPointProjectorLight(
  u_int LightCat, nglVector Pos, nglVector Color, float Range,
  u_int BlendMode = NGLBM_ADDITIVE, u_int BlendModeConstant = 0,
  nglTexture* Tex = 0 );

/*---------------------------------------------------------------------------------------------------------
  Mesh Editing API

  Set of functions to modify mesh resources, and to dynamically create new meshes.  Note that only one
  mesh may be edited at a time.
---------------------------------------------------------------------------------------------------------*/
// Create a new mesh with a set number of sections.
// Optionally NBones and Bones can be specified for skinned meshes.
// If NBones is >0 but Bones is not specified, the reference pose will be left uninitialized
// (Bones also does not need to be specified if NGLP_BONES_LOCAL is used).
// The new mesh is set up for editing automatically.
// To get a pointer to the mesh, finish building it and call nglCloseMesh.
void nglCreateMesh( u_int MeshFlags, u_int NSections, u_int NBones = 0, nglMatrix *Bones = 0 );

// Call this when you're done creating the mesh to get the finalized pointer to it.  DON'T call this when editing a mesh.
nglMesh* nglCloseMesh();

// Destroy a non-temporary mesh.
void nglDestroyMesh( nglMesh* Mesh );

// Call this to use the mesh editing API to edit a mesh.
// Note that any changes will affect the actual data, and thus all instances of the mesh in the scene.
void nglEditMesh( nglMesh* Mesh );

// Calculate a bounding sphere for the current mesh.  Will only take into account the first NVerts vertices.
void nglMeshCalcSphere( u_int NVerts = 0 );

// Calculate a bounding sphere for the current mesh.
void nglMeshSetSphere( nglVector& Center, float Radius );

// Add a new section to the mesh.  Must be called as many times as NSections was passed to nglCreateMesh.
void nglMeshAddSection( nglMaterial* Material, u_int NVerts, u_int Strips );

// Jump to a particular section of the current mesh.
void nglMeshSetSection( u_int Idx );

// Jump to a particular vertex in the mesh.
void nglMeshSetVertex( u_int VertIdx );

// Retrieves the contents of the current vertex into a structure.
struct nglScratchVertex
{
  	float X, Y, Z;
  	float NX, NY, NZ;
  	u_int Color;
  	float U, V;
};
void nglMeshReadVertex( nglScratchVertex* Vertex );

// Starts a new strip with the given length.  Call this before adding vertices.
void nglMeshWritePrimitive( u_int Length );
static inline void nglMeshWriteStrip( u_int Length ) { nglMeshWritePrimitive( Length ); }

// These functions fill in the current section.  For newly created meshes, you must make as many WriteVertex calls as
// the number you passed in NVerts to nglMeshAddSection.  If you're working with a mesh resource, you can just fill in
// the parts you want changed (for example, using WriteVertexPN to leave Color and UV unchanged).
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

/*---------------------------------------------------------------------------------------------------------
  Quad API

  Quads are 2D interface graphics that can be drawn on top of the scene.  Construct nglQuad structures
  using the following functions and pass them to nglListAddQuad.
---------------------------------------------------------------------------------------------------------*/

// Map flags.
enum
{
  	// Filtering mode.
  	NGLMAP_POINT_FILTER       = 0x00000001,
  	NGLMAP_BILINEAR_FILTER    = 0x00000002,
  	NGLMAP_TRILINEAR_FILTER   = 0x00000004,
  	NGLMAP_ANISOTROPIC_FILTER = 0x00000008,

	// All bits related to filtering
	NGLMAP_FILTER_BITS        = NGLMAP_POINT_FILTER|NGLMAP_BILINEAR_FILTER|NGLMAP_TRILINEAR_FILTER|NGLMAP_ANISOTROPIC_FILTER,

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

  	// Subset of material info supported by quads.
  	u_int MapFlags;
  	u_int BlendMode;
  	float BlendModeConstant;
  	nglTexture* Tex;
};

void nglInitQuad( nglQuad* Quad );  // Sets UV 0-1, zero rect, white color, NGLMB_ALPHA, no texture.

void nglSetQuadZ( nglQuad* Quad, float z );                                 // Set Z.  Higher is farther.

// Simplified API.
void nglSetQuadRect( nglQuad* Quad, float x1, float y1, float x2, float y2 ); // Set rectangle position.
void nglSetQuadPos( nglQuad* Quad, float x, float y );                        // Set position.  Gets W/H from the texture (must already be set).
void nglSetQuadUV( nglQuad* Quad, float u1, float v1, float u2, float v2 );   // Set rectangle UVs: Top Left, Bottom Right.
void nglSetQuadColor( nglQuad* Quad, u_int c );                               // Set color - use the NGL_RGBA32 macro.
void nglSetQuadMapFlags( nglQuad *Quad, u_int MapFlags );					  // Set Map flags for clamping, filtering, etc.
void nglSetQuadBlend( nglQuad* Quad, u_int Blend, u_int Constant = 0);        // Set blend mode.
void nglSetQuadTex( nglQuad* Quad, nglTexture* Tex );                         // Set texture.

// Vertex API.  Verts go in this order: Top Left, Top Right, Bottom Left, Bottom Right.
void nglSetQuadVPos( nglQuad* Quad, int VertIdx, float x, float y );          // Set vertex position.
void nglSetQuadVUV( nglQuad* Quad, int VertIdx, float u, float v );           // Set vertex UV.
void nglSetQuadVColor( nglQuad* Quad, int VertIdx, u_int Color );             // Set vertex Color.

// Rotate quad counterclockwise around (cx,cy) by theta radians.
void nglRotateQuad( nglQuad* Quad, float cx, float cy, float theta );

// Scale quad around (cx,cy) in screen coordinates by scale (sx,sy).
void nglScaleQuad( nglQuad* Quad, float cx, float cy, float sx, float sy );

void nglListAddQuad( nglQuad* Quad );

/*---------------------------------------------------------------------------------------------------------
  Font API
---------------------------------------------------------------------------------------------------------*/

// NGL System font.
struct nglFont;
extern nglFont *nglSysFont;

// Load a font (from a .fdf file) and return a pointer to it.
// The texture font must be loaded before calling this function.
nglFont *nglLoadFont(const nglFixedString &FontName);

// Load a font from memory (must point to a fdf file) and return a pointer to the loaded font.
// The texture font must be loaded before calling this function.
nglFont *nglLoadFontInPlace(const nglFixedString &FontName, void *FDFdata);

void nglReleaseFont(nglFont *Font);
void nglReleaseAllFonts();

// Set the font's blend mode.
void nglSetFontBlendMode(nglFont* Font, u_int BlendMode, u_int Constant = 0);

// Set the font's map flags. See NGLMAP_xxx for more details.
void nglSetFontMapFlags(nglFont* Font, u_int MapFlags);

// Return the font's map flags. See NGLMAP_xxx for more details.
u_int nglGetFontMapFlags(nglFont* Font);

// Add a string to the nodes' list to be rendered. The string may contains some formatting informations:
// - The current font color is set by the token (hexa): \1[RRGGBBAA]
// - The current font scale is set by the token: \2[SCALE]
// The default color is the Color argument and the default scale is 1.
// "Hi from \1[ff0000ff]NGL \1[ffffffff] :-)\n\n\2[2.0]Bye!");
// Writes "NGL" in red and "Bye" with a scale of 2.0.
void nglListAddString(nglFont *Font, float x, float y, float z, const char *Text, ...);
void nglListAddString(nglFont *Font, float x, float y, float z, u_int Color, const char *Text, ...);

// Get the dimension, in pixels, of a string (which can have scale tokens and cariage returns as well).
void nglGetStringDimensions(nglFont* Font, u_int* outWidth, u_int* outHeight, const char* Text, ...);

// Max font index.
#define NGL_MAX_FONTS 8

void nglLoadFont( const nglFixedString& FileName, u_int Idx );
void nglLoadFontA( const char* FileName, u_int Idx );
void nglReleaseFont( u_int Idx );

// Returns the width and height in pixels given the current font rendering settings.
void nglGetStringDimensions( int* Width, int* Height, char* Text, ... );

//-----------------------------------------------------------------------------
// TVMode API
//-----------------------------------------------------------------------------
enum
{
	NGLTV_UNKNOWN,
  	NGLTV_NTSC,
  	NGLTV_PAL,
  	NGLTV_MPAL,
  	NGLTV_EURBG60,
};
void nglSetTVMode( u_int TVMode );
u_int nglGetTVMode();
// Resets the graphics system to NGL's preferred mode.  Useful for integrating NGL with other display libraries,
// such as movie players.  Can also be used to toggle PAL/NTSC.
void nglResetDisplay();

void nglSetAspectRatio(float a);

//-----------------------------------------------------------------------------
//  GCN Extensions API
//-----------------------------------------------------------------------------

extern bool nglGCEnableScreenShotService; // Enable this variable to be able to use the ScreenCapture tool.

enum nglGCFogTypes
{
	NGL_GC_FOG_NONE,
	NGL_GC_FOG_LIN,
	NGL_GC_FOG_EXP,
	NGL_GC_FOG_EXP2,
	NGL_GC_FOG_REVEXP,
	NGL_GC_FOG_REVEXP2,
	NGL_GC_MAX_FOG_TYPE,
};

void nglGCGetFrameBuffers( void** fb1, void** fb2, void** fb );
void nglGCFlipXFB( void );

void nglGCSetFog( int FogType, float StartZ, float EndZ, float R, float G, float B );

//-----------------------------------------------------------------------------
void nglMeshWriteVertexPCUV( float X, float Y, float Z, u_int Color, float U, float V );
void nglMeshSetSphere( nglVector& Center, float Radius );

// Animation frame to use for texture scrolling and IFL animation.  This is controllable per scene to allow frontend
// animation to continue while game animation stops.
// To get smooth IFL animation, multiples of 1/60th second are required.
// If not called, NGL's internal VBlank-based timer is used.
void nglSetAnimTime( float Time );

#if defined(PROJECT_KELLYSLATER)

#define nglMatrixIdentity(m) nglIdentityMatrix(m)
#define nglMatrixTrans(d,s,v) nglTransposeMatrix(d,s)
#define nglMatrixMul(d,l,r)   nglMulMatrix(d,l,r)
void nglMatrixRot(nglMatrix& Out, const nglMatrix In, const nglVector rot);

typedef nglMatrix _nglMatrix;

#endif // PROJECT_KELLYSLATER JIV DEBUG


#endif //#ifndef NGL_H
