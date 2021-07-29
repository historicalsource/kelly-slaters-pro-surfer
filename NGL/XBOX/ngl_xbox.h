/*---------------------------------------------------------------------------------------------------------
  NGL_XBOX - XBOX interface of the NGL graphics library.

  Florent Gluck (florent@treyarch.com)
---------------------------------------------------------------------------------------------------------*/

#ifndef NGL_XBOX_H
#define NGL_XBOX_H

/*---------------------------------------------------------------------------------------------------------
  Defines and types for projects that conditionally compile NGL.
---------------------------------------------------------------------------------------------------------*/
#define NGL       1
#define NGL_XBOX  1

#define NGL_MESHFILE_EXT            ".xbmesh"

#ifndef NGL_DEBUG
#define NGL_DEBUG                   _DEBUG
#endif

// Takes 4 integers ranging from 0-255 and returns a correct packed color value for the XBox arch.
#define NGL_RGBA32(r, g, b, a)      (((((uint32)(a))<<24) | (((uint32)(r))<<16) | (((uint32)(g))<<8) | ((uint32)(b))))

/*---------------------------------------------------------------------------------------------------------
  Includes
---------------------------------------------------------------------------------------------------------*/
#include <xtl.h>
#include <xgmath.h>
#include "ngl_types.h"
#include "ngl_fixedStr.h"

/*---------------------------------------------------------------------------------------------------------
  Version API.

  Functions that return the version number and the version build date.

---------------------------------------------------------------------------------------------------------*/

char *nglGetVersion();
char *nglGetVersionDate();

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

	nglVector(const nglVector& v)
	{ x = v.x; y = v.y; z = v.z; w = v.w; }

	nglVector(float _x, float _y, float _z, float _w = 1.0f)
	{ x = _x; y = _y; z = _z; w = _w; }

	nglVector(XGVECTOR3 v)
	{ x = v[0]; y = v[1]; z = v[2]; w = 1.0f; }

	nglVector(XGVECTOR4 v)
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
};

__declspec(align(16)) class nglMatrix
{
public:
	nglVector x, y, z, w;

	nglMatrix() {}

	nglMatrix(const nglMatrix& m)
	{ x = m.x; y = m.y; z = m.z; w = m.w; }

	nglMatrix(const nglVector& _x, const nglVector& _y, const nglVector& _z, const nglVector& _w)
	{ x = _x; y = _y; z = _z; w = _w; }

	nglMatrix(XGMATRIX& m)
	{ *this = *(nglMatrix*)&m; }

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
};

extern nglMatrix nglMatrixID;

// Set the matrix to the identity.
inline static void nglIdentityMatrix(nglMatrix &mat)
{
	memcpy(mat, nglMatrixID, sizeof(nglMatrix));
}
inline static void nglMatrixIdentity(nglMatrix &mat)
{
	memcpy(mat, nglMatrixID, sizeof(nglMatrix));
}

// Multiply two matrices (src2 * src1).
inline static void nglMatrixMul(nglMatrix &dest, const nglMatrix &src1, const nglMatrix &src2)
{
	XGMatrixMultiply((XGMATRIX *)&dest, (XGMATRIX *)&src2, (XGMATRIX *)&src1);
}

inline static void nglMulMatrix(nglMatrix &dest, const nglMatrix &src1, const nglMatrix &src2)
{
	XGMatrixMultiply((XGMATRIX *)&dest, (XGMATRIX *)&src2, (XGMATRIX *)&src1);
}

// Tranform a matrix by a vector.
inline static void nglApplyMatrix(nglVector &dest, const nglMatrix &msrc, const nglVector &vsrc)
{
	XGVec4Transform((XGVECTOR4 *)&dest, (XGVECTOR4 *)&vsrc, (XGMATRIX *)&msrc);
}

// Translate a matrix using the specified x,y,z positions.
void nglMatrixTrans(nglMatrix &Out, const nglMatrix &In, const nglVector &trans);

// Rotate a matrix using the specified x,y,z angles.
void nglMatrixRot(nglMatrix &Out, const nglMatrix &In, const nglVector &rot);

/*---------------------------------------------------------------------------------------------------------
  System Callback API.

  Allows the game to override various system operations within NGL.  Defaults are provided.

---------------------------------------------------------------------------------------------------------*/

// File buffer for callback functions to fill.
struct nglFileBuf
{
	uint8 *Buf;
	uint32 Size;
	uint32 UserData;  // Optional data for the callbacks' use.
};

// Structure of function pointers for NGL system callbacks.
struct nglSystemCallbackStruct
{
	// File operations.
	bool(*ReadFile) (const char *FileName, nglFileBuf *File, uint32 Align);
	void (*ReleaseFile) (nglFileBuf *File);

	// Errors/debugging.
	void (*CriticalError) (const char *Text);
	void (*DebugPrint) (const char *Text);

	// Memory allocation.
	void *(*MemAlloc) (uint32 Size, uint32 Align);
	void (*MemFree) (void *Ptr);
};

/*---------------------------------------------------------------------------------------------------------
  Callbacks API.

  Overrides default system operations.  Passing in a structure of function pointers will override the
  current system callbacks and cause NGL to use the new ones for all operations.
  NULL can passed in any of the pointers, which will cause the NGL default implementation to be used.
---------------------------------------------------------------------------------------------------------*/

void nglSetSystemCallbacks(nglSystemCallbackStruct *Callbacks);

/*---------------------------------------------------------------------------------------------------------
  System callback function handlers (defaults provided), in case the client wants to call them directly.



---------------------------------------------------------------------------------------------------------*/

void nglPrintf(const char *Format, ...);
void nglWarning(const char *Format, ...);
void nglError(const char *Format, ...);
void nglLog(const char *Format, ...);
void nglInfo(const char *Format, ...);
void nglFatal(const char *Format, ...);

bool nglReadFile(char *FileName, nglFileBuf *File, uint32 Align = 0);
void nglReleaseFile(nglFileBuf *File);

void *nglMemAlloc(uint32 Size, uint32 Align = 0, bool WriteCombined = 0);
void nglMemFree(void *Ptr);

/*---------------------------------------------------------------------------------------------------------
  XBox specific API.


---------------------------------------------------------------------------------------------------------*/

// These memory functions are XBox specific.
void nglDisplayMemoryStatusXB();
uint32 nglGetFreePhysicalMemoryXB();

bool nglFileExistsXB(const char *FileName);

// Read a "vshader keys file" and generate (compile + create) the corresponding vertex shaders. Each vertex
// shader can be associated to a key which completely describes the vertex shader (textures passes, lights,
// etc.). By saving those keys in a file, it allows the client to load this file and generate the associated
// vshaders at initialization time.
bool nglReadVShaderFileXB(const char *FileName);

// Write the vertex shader keys that have been used until now. In other words, generates a file which contains
// all the vertex shaders generated/used until now. If the specified file already exists, its keys will be
// added to the current database before beeing written to the file (aka keys are appended to the already
// existing file without duplicates).
bool nglWriteVShaderFileXB(const char *FileName);

// Set the camera matrix.
void nglSetCameraMatrixXB(nglMatrix &WorldToView, const nglVector &Pos, const nglVector &YDir, const nglVector &ZDir);

/*---------------------------------------------------------------------------------------------------------
  Debugging API.



---------------------------------------------------------------------------------------------------------*/

// Use to set various debugging/stage flags externally.
void nglSetDebugFlag(const char *Flag, bool Set);

/*---------------------------------------------------------------------------------------------------------
  General API.

  

---------------------------------------------------------------------------------------------------------*/

// Initialize the NGL library.
void nglInit();

// Empty for now...
void nglExit();

// Flips the front/back buffers. Typically you want to do this by passing true to nglListSend.
void nglFlip();

// Lock the frame rate to a given FPS (default is 60 FPS).
// On the XBox, only the following parameters are supported:
// * 60 : lock the framerate @ 60 FPS;
// * 30 : lock the framerate @ 30 FPS;
// *  0 : unlimited framerate (usefull for profiling)
void nglSetFrameLock(float FPS);

// Sets the speed at which IFLs play, in frames per second (default is 30fps).
// Values should divide evenly into 60, ie 60, 30, 20, 15, 10, etc.
void nglSetIFLSpeed(float FPS);

// Get/set debugging flags externally. See nglDebugStruct in ngl_xbox.cpp for a list.
void nglSetDebugFlag(const char *Flag, bool Set);
bool nglGetDebugFlag(const char *Flag);

// Returns the screen width and height in pixels.
int32 nglGetScreenWidth();
int32 nglGetScreenHeight();

void nglSetMeshPath(const char *Path);
void nglSetTexturePath(const char *Path);

const char *nglGetMeshPath();
const char *nglGetTexturePath();

// Take a screenshot of the current back buffer (BMP file).
void nglScreenShot(const char *FileName = NULL);

/*---------------------------------------------------------------------------------------------------------
  Custom Node API

  See ngl_xbox_internal.h for more infos and look at the CUSTOM RENDER API.

---------------------------------------------------------------------------------------------------------*/

// Custom node callback function.
typedef void (*nglCustomNodeFn) (void *Data);

// Allow D3D device to be accessible by the client.
extern LPDIRECT3DDEVICE8 nglDev;

enum
{
	NGLSORT_OPAQUE,
	NGLSORT_TRANSLUCENT,
};

struct nglSortInfo
{
	uint32 Type;
	union
	{
		float Dist;   // Sorting distance for tanslucent sections.
		uint32 Hash;  // Hash value for opaque sections.
	};
};

void nglListAddCustomNode(nglCustomNodeFn Fn, void *Data, nglSortInfo *SortInfo);

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
void nglListSend(bool Flip = false);

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
nglScene *nglListBeginScene();

// nglListEndScene leaves the current scene and returns to the parent scene.
void nglListEndScene();

// nglListSelectScene allows you to jump to a particular scene directly, and add more nodes to it or change
// scene parameters.  This can be useful for simultaneously generating the main game scene and a scene
// containing a reflection.
//
// The currently active scene is returned, so you can return to it later.
nglScene *nglListSelectScene(nglScene *scene);

// Specify a texture to render the scene to.  If NULL is passed, the back buffer of the screen is assumed.
struct nglTexture;
void nglSetRenderTarget(nglTexture *Tex, bool Download = false);

// Set up for orthogonal rendering.
//
// Set up for orthogonal (parallel projection) rendering.   The view ranges from -1 to 1 in X and Y axes, where
// (0,0) translates to the center of projection.
//   (cx,cy) is the center of projection in screen coordinates.
//   (nearz..farz) is the input range that vertices and quads will use.
//   (zmin..zmax) can be used to limit output to a given range of the Z buffer.
void nglSetOrthoMatrix(float cx, float cy, float nearz, float farz, float zmin = 0.0f, float zmax = 1.0f);

// Set up for perspective rendering.
//   hfov is the horizontal field of view in degrees (example 90.0f).
//   (cx,cy) is the center of projection in screen coordinates.
//   (nearz..farz) is the input range that vertices and quads will use.
//   (zmin..zmax) can be used to limit output to a given range of the Z buffer.
//   (renderwidth,renderheight) - WTB TODO: need to find out what these do ;)
void nglSetPerspectiveMatrix(float hfov, float cx, float cy, float nearz, float farz, float zmin = 0.0f, float zmax = 1.0f);

void nglGetProjectionParams(float *hfov, float *cx, float *cy, float *nearz, float *farz);

// Sets the camera matrix for the scene.
void nglSetWorldToViewMatrix(const nglMatrix &WorldToView);

// Clear flags (stencil is XBox specific).
// On XBox it's A LOT faster to clear specifying the 3 flags at the same time !
enum
{
	NGLCLEAR_COLOR   = D3DCLEAR_TARGET,
	NGLCLEAR_RGBA    = D3DCLEAR_TARGET,
	NGLCLEAR_RGB     = D3DCLEAR_TARGET_R | D3DCLEAR_TARGET_G | D3DCLEAR_TARGET_B,
	NGLCLEAR_R       = D3DCLEAR_TARGET_R,
	NGLCLEAR_G       = D3DCLEAR_TARGET_G,
	NGLCLEAR_B       = D3DCLEAR_TARGET_B,
	NGLCLEAR_A       = D3DCLEAR_TARGET_A,

	NGLCLEAR_Z       = D3DCLEAR_ZBUFFER,
	NGLCLEAR_STENCIL = D3DCLEAR_STENCIL
};

// Screen/texture clear parameters for the scene.
void nglSetClearFlags(uint32 ClearFlags);
void nglSetClearColor(float R, float G, float B, float A);
void nglSetClearZ(float Z);                 // Z is in hardware space (0.0-nearest, 1.0-farthest).
void nglSetClearStencil(uint32 Stencil);    // XBox specific.

// Framebuffer write mask (XBox specific).
enum
{
	NGL_FBWRITE_RGBA = D3DCOLORWRITEENABLE_ALL,
	NGL_FBWRITE_RGB  = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE,
	NGL_FBWRITE_R    = D3DCOLORWRITEENABLE_RED,
	NGL_FBWRITE_G    = D3DCOLORWRITEENABLE_GREEN,
	NGL_FBWRITE_B    = D3DCOLORWRITEENABLE_BLUE,
	NGL_FBWRITE_A    = D3DCOLORWRITEENABLE_ALPHA
};

// Set a write mask when rendering the geometry to the framebuffer.  All channels are written by default.
void nglSetFBWriteMask(uint32 WriteMask);

// Enables/Disables writes to Z buffer for this scene.  Enabled by default.
void nglSetZWriteEnable(bool Enable);

// Enables/Disables Z buffer testing for this scene.  Enabled by default.
void nglSetZTestEnable(bool Enable);

// Screen-space view rectangle for the scene.  Defaults are 0, 0, ScreenWidth - 1, ScreenHeight - 1.
void nglSetViewport(uint32 x1, uint32 y1, uint32 x2, uint32 y2);

// Fog parameters for the scene.
void nglEnableFog(bool Enable);
void nglSetFogColor(float R, float G, float B);
void nglSetFogRange(float Near, float Far, float Min, float Max);

// Animation frame to use for texture scrolling and IFL animation.  This is controllable per scene to allow frontend
// animation to continue while game animation stops.
// To get smooth IFL animation, multiples of 1/60th second are required.
// If not called, NGL's internal VBlank-based timer is used.
void nglSetAnimTime(float Time);

// Returns one of the current matrices in the model->view->projection chain.
enum
{
	NGLMTX_VIEW_TO_WORLD,
	NGLMTX_VIEW_TO_SCREEN,
	NGLMTX_WORLD_TO_VIEW,
	NGLMTX_WORLD_TO_SCREEN,
};

void nglGetMatrix(nglMatrix &Dest, uint32 ID);

// Projects a point from World space (X,Y,Z) to Screen space (0..nglGetScreenWidth()-1,0..nglGetScreenHeight()-1).
// Useful for placing 2D interface elements on 3D points in the scene.
void nglProjectPoint(nglVector &Out, nglVector &In);

/*---------------------------------------------------------------------------------------------------------
  Texture API.



---------------------------------------------------------------------------------------------------------*/

// Texture types.
enum
{
	NGLTEX_TGA,      // General purpose format.  Currently supports 32bit, 24bit and 16bit.
	NGLTEX_DDS,      // XBOX simple texture.
	NGLTEX_DDSCUBE,  // XBOX cube texture (6 faces).
	NGLTEX_XPR,			 // XBOX xpr texture
	NGLTEX_XPRCUBE,	 // XBOX xpr cube texture
	NGLTEX_IFL,      // IFL files aren't actual textures, but arrays of texture pointers in an animated sequence.
};

// Texture resource structure.  These are stored in the texture instance bank.
struct nglTexture
{
	uint32 Type;           // Texture type, see NGLTEX_xxx enum.
	uint32 Static;         // The texture is static and must NEVER be released
	// Furthermore never release the associated D3D texture and surface.

	uint32 Width, Height;  // Texture dimensions.
	uint32 MipmapLevels;   // Number of mipmap levels.
	uint32 *Data;          // Pixel data address in RAM (only valid when locked!).
	uint32 DataPitch;      // Pitch of the data (only valid when locked!).

	// Textures.
	union
	{
		IDirect3DTexture8 *Simple;    // D3D "simple" texture.
		IDirect3DCubeTexture8 *Cube;  // D3D cube texture for cubic mapping (6 surfaces).
	}
	DXTexture;

	// File Pointer (When loading an XPR file, the file pointer is not released until the texture is destroyed
	nglFileBuf *File;

	// Surface.
	IDirect3DSurface8 *DXSurface;  // D3D surface (aka texture of level 0).

	// IFL format data:
	uint32 NFrames;           // If this member is greater than 1, this is an animated texture.
	nglTexture **Frames;      // Array of texture pointers for the individual frames
	// (which are also stored in the instance bank).
	uint32 Format;            // See XBox NGLTF_xxx flags.

	nglFixedString FileName;  // Original filename.
};

nglTexture *nglLoadTexture(const nglFixedString &FileName, bool ForceCubeMap = false);
nglTexture *nglLoadTextureA(const char *FileName);

// Return a pointer to an already loaded texture.
// Note: if you want to retrieve the front/back buffer textures uses the functions below.
nglTexture *nglGetTexture(const nglFixedString &FileName);
nglTexture *nglGetTextureA(const char *FileName);

// Return the front/back buffer textures.
// Client must not retrieve them directly without passing by a function !
nglTexture *nglGetFrontBufferTex();
nglTexture *nglGetBackBufferTex();

nglTexture *nglLoadTextureInPlace(const nglFixedString &FileName, uint32 Type, void *Data, uint32 Size);

// Add a reference to a texture in the instance bank.
void nglAddTextureRef(nglTexture *Tex);

void nglReleaseTexture(nglTexture *Tex);
void nglReleaseAllTextures();

// These functions are XBox specific, they lock a texture.
// This is the only way to access individual pixels of the texture.
bool nglLockTextureXB(nglTexture *Tex);
bool nglUnlockTextureXB(nglTexture *Tex);

// Texture creation API.
enum
{
	// These 3 flags are ignored on the XBox and they are provided to be compatible with the PS2 version.
	NGLTF_16BIT     = 0x0000,
	NGLTF_TEMP      = 0x0000,
	NGLTF_VRAM_ONLY = 0x0000,

	NGLTF_32BIT     = 0x0100,

	// These flags are XBox specific.
	NGLTF_SWIZZLED  = 0x0004,
	NGLTF_LINEAR    = 0x0008,

	// XBox specific texture formats.
	// YUY2 is required by the NVL decoder.
	NGLTF_ARGB8888  = NGLTF_32BIT,
	NGLTF_ARGB4444  = 0x0200,
	NGLTF_ARGB1555  = 0x0300,
	NGLTF_RGB888    = 0x0400,
	NGLTF_RGB565    = 0x0500,
	NGLTF_RGB655    = 0x0600,
	NGLTF_YUY2      = 0x0700,    // This format requires NGLTF_LINEAR.
};

// Create a texture.
// The client must specify if he wants to create a linear or swizzled texture.
// In case of linear texture don't forget to multiply UV by width and height.
//
// Linear/swizzled textures have the following differences:
// - Rendering TO a linear texture is faster than rendering to a swizzled.
// - Rendering FROM a swizzled texture is faster than rendering from a linear.
// - Swizzled format must have width and height to be a power of 2.
// - Swizzled render target must have the viewport size equal to the texture size.
// - Swizzled textures use coordinates that range [0,1].
// - Linear textures use UV range [0, width or height], so it REQUIRES UV scaling.
// - The hardware requires CLAMP for linear textures.
// - Linear format does not support mipmaps (mipmap level must be 1).
// - Linear format does not support cube maps nor volume.
// - Generally speaking, it's faster to use linear textures for shadows rendering.
// IMPORTANT: Because of the current NGL implementation, try to render to linear textures,
//            because clearing the zbuffer when rendering to a swizzled texture is sloooooow.
nglTexture *nglCreateTexture(uint32 Format, uint32 Width, uint32 Height);

void nglDestroyTexture(nglTexture *Tex);

// Saves a BMP file containing the texture.  If no filename is given, it uses savetexXX.bmp.
void nglSaveTexture(nglTexture *Tex, const char *FileName);

/*---------------------------------------------------------------------------------------------------------
  Mesh API.



---------------------------------------------------------------------------------------------------------*/

// Material flags.
enum
{
	// Translucency.
	NGLMAT_ALPHA               = 0x00000001,  // Material is translucent, vertex alpha is used.
	NGLMAT_ALPHA_SORT_FIRST    = 0x00000002,  // Causes non-additive alpha polys to go in the
	// additive list, saves sort time.
	NGLMAT_ALPHA_FALLOFF       = 0x00000004,
	NGLMAT_ALPHA_FALLOFF_OUT   = 0x00000008,

	// Material passes.
	NGLMAT_TEXTURE_MAP         = 0x00000010,
	NGLMAT_LIGHT_MAP           = 0x00000020,
	NGLMAT_DETAIL_MAP          = 0x00000040,
	NGLMAT_ENVIRONMENT_MAP     = 0x00000080,
	NGLMAT_BUMP_MAP            = 0x00000100,  // Replaces the detail map with a bump map.

	// Clipping stuff.
	NGLMAT_BACKFACE_DEFAULT    = 0x00000400,  // If one or two of these flags is set,
	NGLMAT_BACKFACE_CULL       = 0x00000800,  // clip polygons that are facing away from the camera.

	// Filtering mode.
	NGLMAT_BILINEAR_FILTER     = 0x00001000,
	NGLMAT_TRILINEAR_FILTER    = 0x00002000,

	// These params are not used on the XBox.
	NGLMAT_ANTIALIAS           = 0x00004000,
	NGLMAT_PERSPECTIVE_CORRECT = 0x00008000,

	// Clamping for diffuse1 (aka map).
	NGLMAT_CLAMP_U             = 0x00010000,
	NGLMAT_CLAMP_V             = 0x00020000,

	NGLMAT_FOG                 = 0x00040000,
	NGLMAT_FORCE_Z_WRITE       = 0x00080000,

	// Lighting.
	NGLMAT_LIGHT               = 0x00100000,  // Self illuminated if this flag is off.

	NGLMAT_UV_SCROLL           = 0x00200000,  // Automatic UV scrolling.

	NGLMAT_ENV_CYLINDER        = 0x00400000,  // Specifies cylindrical environment mapping.
	NGLMAT_ENV_SPECULAR        = 0x00800000,  // Replaces the environment map with a specular highlight pass.

	// Clamping for diffuse2 (aka lightmap).
	NGLMAT_LIGHT_CLAMP_U       = 0x01000000,
	NGLMAT_LIGHT_CLAMP_V       = 0x02000000,
	
#ifdef PROJECT_KELLYSLATER
	NGLMAT_WATER_NEARFAR       = 0x04000000,
	NGLMAT_WATER_SPECULAR      = 0x08000000,
	NGLMAT_WATER_FOAM          = 0x20000000,
#endif
	
	NGLMAT_MATERIAL_COLOR      = 0x10000000,  // Set to force the first pass to use the material color instead of the vertex color.
};

#define NGL_TEXTURE_MASK       ( NGLMAT_TEXTURE_MAP | NGLMAT_LIGHT_MAP | NGLMAT_DETAIL_MAP | NGLMAT_ENVIRONMENT_MAP )

// Material blend modes.
// WARNING: order is important !
enum
{
	NGLBM_OPAQUE,               // No translucency is performed, alpha is ignored.
	NGLBM_PUNCHTHROUGH,         // Similar to opaque, except if alpha is below a threshold the pixel is skipped.

	NGLBM_BLEND,                // Blends the texel with the background, modulated by Alpha.
	NGLBM_ADDITIVE,             // Adds the texel to the background, modulated by Alpha.
	NGLBM_SUBTRACTIVE,          // Subtracts the texel from the background, modulated by Alpha.

	NGLBM_CONST_BLEND,          // Blends the texel with the background, modulated by BlendModeConstant.
	NGLBM_CONST_ADDITIVE,       // Adds the texel to the background, modulated by BlendModeConstant.
	NGLBM_CONST_SUBTRACTIVE,    // Subtracts the texel from the background, modulated by BlendModeConstant.

	NGLBM_OPAQUE_NOZWRITE,        // Like opaque, but with z-writes disabled.
	NGLBM_PUNCHTHROUGH_NOZWRITE,  // Like punchthrough, but with z-writes disabled.

	NGLBM_INVALID               // This special blending mode is used to reset the blending mode caching system.
};

// Mesh material structure with a fixed set of passes.
#pragma pack(push,1)

struct nglMaterial
{
	// Material flags - see NGLMAT_XXXX.
	uint32 Flags;

	// Texture pointers.
	nglTexture *Map;
	nglTexture *LightMap;
	nglTexture *DetailMap;
	nglTexture *EnvironmentMap;

	uint32 Pad0;  // nglFixedString must be on a 8 Bytes boundary.

	nglFixedString MapName;
	uint32 MapBlendMode;
	uint32 MapBlendModeConstant;

	nglFixedString LightMapName;
	uint32 LightMapBlendMode;
	uint32 LightMapBlendModeConstant;

	nglFixedString DetailMapName;
	uint32 DetailMapBlendMode;
	uint32 DetailMapBlendModeConstant;

	nglFixedString EnvironmentMapName;
	uint32 EnvironmentMapBlendMode;
	uint32 EnvironmentMapBlendModeConstant;

	float DetailMapUScale;  // U and V are scaled independently.
	float DetailMapVScale;
	float DetailMapRange;
	float DetailMapAlphaClamp;

	// Misc parameters.
	uint32 Color;
	uint32 MaterialID;
	float AlphaFalloff;
	float ForcedSortDistance;
	float ScrollU, ScrollV;
};

#pragma pack(pop)

enum
{
	NGLP_TINT                 = 0x00000001,  // Multiply vertex colors by a tint color, before lighting.

	// The following flags specify that bone positions are present in the render parameters
	// (using NBones and Bones) in various coordinate systems.  At most one may be specified.
	NGLP_BONES_WORLD          = 0x00000002,  // Bone positions are in bone local (as defined by reference) to world space.
	NGLP_BONES_LOCAL          = 0x00000004,  // Bone positions are in bone local (as defined by reference) to local space.
	NGLP_BONES_RELATIVE       = 0x00000008,  // Bone positions are in reference mesh space to local space.
	// (Formerly this was NGLP_BONES_LOCAL!)

	// Mask of all flags that indicate bone positions are present in render parameters
	// (Only used for testing, not setting)
	NGLP_BONES_MASK =        (NGLP_BONES_WORLD | NGLP_BONES_LOCAL | NGLP_BONES_RELATIVE),

	NGLP_BONES                = 0x00000000,  // This flag is obsolete and ignored

	NGLP_FULL_MATRIX          = 0x00000010,  // LocalToWorld is the complete to screen space matrix and no clipping will be performed.
	NGLP_SCALE                = 0x00000040,  // A non-uniform scale is to be applied and the Scale member is valid.
	NGLP_TEXTURE_FRAME        = 0x00000080,  // TextureFrame member is valid and should be used to override the default.
	NGLP_TEXTURE_SCROLL       = 0x00000100,  // ScrollU and ScrollV members are valid and should be used to scroll the texture.
	NGLP_MATERIAL_MASK        = 0x00000200,  // Mask off materials by index.
	NGLP_ZBIAS                = 0x00000400,  // Offset Z coordinates in the ZBuffer (doesn't affect display).
	NGLP_NO_CULLING           = 0x00000800,  // Turns off view frustum culling, assumes that the game code has already done it.
	NGLP_LIGHT_CONTEXT        = 0x00001000,  // LightContext member is valid and specifies the lighting context that the mesh should use.
	NGLP_NO_LIGHTING          = 0x00002000,  // Disable lighting for this instance.

	NGLP_REVERSE_BACKFACECULL = 0x00004000,	 // Reverse the back face culling mode.
};

struct nglLightContext;

struct nglRenderParams
{
	uint32 Flags;                   // Flags that specify what members are valid.
	uint32 TextureFrame;            // Frame value for animated textures.
	float ScrollU, ScrollV;         // Texture scrolling amount.
	nglVector TintColor;            // Color scale to be multiplied by vertex colors - ranges from 0.0 to 1.0.
	nglVector Scale;                // 3D non-uniform scale factor.  Do not apply this to the matrix !
	uint32 MaterialMask;            // 32bit mask to selectively disable materials by ID.
	float ZBias;                    // Determines a scale for Z buffer.
	nglLightContext* LightContext;  // Lighting context to apply to this mesh.

	// Bone transforms for skinned meshes.
	uint32 NBones;
	nglMatrix *Bones;
};

// Mesh flags.
enum
{
	// Clipping stuff.
	NGLMESH_PERFECT_TRICLIP = 0x00000001,
	NGLMESH_REJECT_TRICLIP  = 0x00000002,
	NGLMESH_REJECT_SPHERE   = 0x00000004,

	// Transform stuff.
	NGLMESH_SKINNED         = 0x00000020,

	NGLMESH_LOD             = 0x00000100,

	NGLMESH_TEMP            = 0x00001000,  // Created in temporary memory, lost after the next call to nglListSend.
	NGLMESH_DOUBLE_BUFFER   = 0x00002000,  // NGL will double buffer the scratch mesh (to prevent modifying it while it's rendering).

	// Set by code, tool should never set.
	NGLMESH_PROCESSED       = 0x00100000,
	NGLMESH_BUFFER_OWNER    = 0x00200000,
	NGLMESH_SCRATCH_MESH    = 0x00400000,

	// Lighting categories.
	NGLMESH_LIGHTCAT_1      = 0x01000000,
	NGLMESH_LIGHTCAT_2      = 0x02000000,
	NGLMESH_LIGHTCAT_3      = 0x04000000,
	NGLMESH_LIGHTCAT_4      = 0x08000000,
	NGLMESH_LIGHTCAT_5      = 0x10000000,
	NGLMESH_LIGHTCAT_6      = 0x20000000,
	NGLMESH_LIGHTCAT_7      = 0x40000000,
	NGLMESH_LIGHTCAT_8      = 0x80000000,
};

#define NGL_LIGHTCAT_MASK     0xFF000000

// NGL primitive types.
enum
{
    NGLPRIM_PNTLIST   = D3DPT_POINTLIST,      // Collection of isolated points.
    NGLPRIM_LINELIST  = D3DPT_LINELIST,       // List of isolated straight line segments. Count must be >= 2 and even !
    NGLPRIM_LINESTRIP = D3DPT_LINESTRIP,      // Single polyline. Count must be >= 2 !
    NGLPRIM_TRILIST   = D3DPT_TRIANGLELIST,   // Sequence of isolated triangles. Each group of three vertices defines a separate triangle. 
    NGLPRIM_TRISTRIP  = D3DPT_TRIANGLESTRIP,  // Triangle strip (first one must be clock-wise).
	NGLPRIM_TRIFAN    = D3DPT_TRIANGLEFAN,    // Triangle fan.
    NGLPRIM_QUADLIST  = D3DPT_QUADLIST,       // Sequence of isolated quads (4 vertices / quad !).
};

#pragma pack(push,1)

struct nglVertexBuffer
{
	void *VertexBufferData;     // Vertex buffer data (MUST be aligned to a 4KB boundary).
	IDirect3DVertexBuffer8 VertexBuffer;
	uint32 VertexCount;         // Unused. To be removed.

	void *IndexBufferData;      // Index buffer data.
	IDirect3DIndexBuffer8 IndexBuffer;	// Unused. To be removed.
	uint32 IndexCount;          // Unused. To be removed.
};

struct nglMeshFileHeader
{
	uint32 Tag;                 // 'XBXM'
	uint32 Version;
	uint32 NMeshes;

	uint32 NSkinVertexBuffers;
	uint32 NVertexBuffers;

	nglVertexBuffer *SkinVertexBuffers;  // Shared vertex buffer for skinned meshes.
	nglVertexBuffer *VertexBuffers;      // Shared vertex buffer for non-skinned meshes.

	uint32 Pad[1];
};

struct nglMeshLODInfo
{
	float Range;
	nglFixedString Name;
};

struct nglMeshSection
{
	nglVertexBuffer *VB;        // Pointer to the vertex and index buffer.

	union
	{
		uint32 IndexOffset;     // First index for this section in the mesh's index buffer.
		void*  PrimInfo;        // Pointer to the primitive info (scratch mesh only).
	};

	union
	{
		uint32 IndexCount;      // # of indices for this section.
		uint32 PrimCount;       // # primitive sets for this section (scratch mesh only).
	};

	nglMaterial *Material;

	float SphereRadius;
	nglVector SphereCenter;

	uint32 NBones;              // Number of bones/section.
	int16 *BonesIdx;            // Section bones (as indices to Mesh->Bones).

	uint32 PartialVSKey;        // Vertex shader key that stores the vshader to use for this section.
	uint32 VSInitHandle;        // Vertex shader handle, used for sorting.

	IDirect3DPushBuffer8 *PB;   // Pushbuffer for the section (0 if no PB rendering).

	void (*RenderSectionFunc)(void *Data);  // To override the default NGL renderer.

	uint32 PSIdx;               // Pixel shader index. See NGL_PS_xxx enum.

	uint32 PrimType;            // Primitive type. See NGLPRIM__xxx enum (scratch mesh only).

	uint32 Reserved[4];         // Reserved for future extensions.
};

// Mesh resource structure.
struct nglMeshFile;

struct nglMesh
{
	uint32 Flags;               // NGL Mesh flags.

	nglMeshFile *File;
	nglMesh *NextMesh;
	uint32 DataSize;

	nglFixedString Name;        // Mesh filename (no extension or path).

	nglVector SphereCenter;     // Center of mesh
	float SphereRadius;         // The radius of the bounding sphere with respect to the mesh's center.

	uint32 NLODs;
	nglMeshLODInfo *LODs;

	uint32 NBones;              // Number of bones for the mesh.
	nglMatrix *Bones;           // Pointer to bones matrices.

	uint32 NSections;
	nglMeshSection *Sections;

	uint32 VertexSize;          // Vertex size.
};

#pragma pack(pop)

// New mesh loading functions.
bool nglLoadMeshFile(const nglFixedString &FileName);
bool nglLoadMeshFileInPlace(const nglFixedString &FileName, void *Buf);

void nglReleaseMeshFile(const nglFixedString &FileName);
void nglReleaseAllMeshFiles();

// Retrieve a mesh from a loaded file.  If the mesh can't be found it returns NULL and prints out a warning
// message (which can be disabled by passing Warn = false).
nglMesh *nglGetMeshA(const char *Name, bool Warn = true);
nglMesh *nglGetMesh(const nglFixedString &Name, bool Warn = true);

// Functions to iterate through all the meshes in a file.  Call nglGetFirstMeshInFile with the file name, then call
// nglGetNextMeshInFile repeatedly with the most recent return value to iterate.
nglMesh *nglGetFirstMeshInFile(const nglFixedString &FileName);
nglMesh *nglGetNextMeshInFile(nglMesh *Mesh);

// Add a mesh to the scene.
void nglListAddMesh(nglMesh *Mesh, const nglMatrix &LocalToWorld, const nglRenderParams *Params = 0);

// Find the index of a material by ID, suitable for use with NGLP_MATERIAL_MASK.
// Returns -1 if the ID can't be found.
int32 nglGetMaterialIdx(nglMesh *Mesh, uint32 MaterialID);

/*---------------------------------------------------------------------------------------------------------
 Lighting API

 Use these structs, flags and enums to create a nglLightInfo struct which can be passed to nglListAddLight.

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

#define NGL_LIGHTCAT_SHIFT      24

// Functions to add various types of lights to the scene.
void nglListAddDirLight(uint32 LightCat, nglVector &Dir, nglVector &Color);
void nglListAddFakePointLight(uint32 LightCat, nglVector &Pos, float Near, float Far, nglVector &Color);
void nglListAddPointLight(uint32 LightCat, nglVector &Pos, float Near, float Far, nglVector &Color);

// Creates a new lighting context and returns the ID.  All lights added to the render list after this call
// will go into the newly created context, until a new call to nglCreateLightContext is made. Apply a lighting context
// to a mesh using NGLP_LIGHT_CONTEXT.
nglLightContext* nglCreateLightContext();

// Switching and retrieving the current lighting context.
void nglSetLightContext(nglLightContext* Context);
nglLightContext* nglGetLightContext();

// Sets the ambient color for the current context.
void nglSetAmbientLight(float R, float G, float B, float A);

// LEGACY INTERFACE - to be removed!  Use the type-specific interfaces above.
enum
{
	// Standard lights.
	NGLLIGHT_FAKEPOINT,         // Point light faked by a directional light.
	NGLLIGHT_TRUEPOINT,         // True vertex accurate point light.
	NGLLIGHT_DIRECTIONAL,       // General directional light.

	// Projected texture lights.
	NGLLIGHT_PROJECTED_DIRECTIONAL,
	NGLLIGHT_PROJECTED_SPOT,
	NGLLIGHT_PROJECTED_POINT,
};

struct nglLightInfo
{
	int32 Type;
	uint32 Flags;

	// Color parameters in RGBA format.
	nglVector ColorAdd;

	// Falloff parameters - must not change order or size.
	float DistFalloffStart;
	float DistFalloffEnd;
};

void nglListAddLight(nglLightInfo *Light, nglVector &Pos, nglVector &Dir);
// END LEGACY INTERFACE

/*---------------------------------------------------------------------------------------------------------
  Projector Light API

  Projector lights project a texture onto the scene.

  On the XBox, it's faster to project to a linear texture (specify the NGLTF_LINEAR flag when calling
  nglCreateTexture).
  The scaling of the UV coordinates is done automatically in the vertex shader.
  Note that you MUST use square linear textures.
---------------------------------------------------------------------------------------------------------*/

void nglListAddDirProjectorLight(uint32 LightCat, nglMatrix &PO,
                                 nglVector &Scale, uint32 BlendMode = NGLBM_ADDITIVE,
                                 uint32 BlendModeConstant = 0, nglTexture *Tex = 0);

void nglListAddSpotProjectorLight(uint32 LightCat, nglMatrix &PO,
                                  nglVector &Scale, float FOV,
                                  uint32 BlendMode = NGLBM_ADDITIVE,
                                  uint32 BlendModeConstant = 0, nglTexture *Tex = 0);

void nglListAddPointProjectorLight(uint32 LightCat, nglVector &Pos,
                                   nglVector &Color, float Range,
                                   uint32 BlendMode = NGLBM_ADDITIVE,
                                   uint32 BlendModeConstant = 0, nglTexture *Tex = 0);

/*---------------------------------------------------------------------------------------------------------
  Scratch Mesh API

  Set of functions to modify mesh resources, and to dynamically create new meshes.  Note that only one
  mesh may be edited at a time.
---------------------------------------------------------------------------------------------------------*/

// Create a new mesh with a set number of sections.
// Optionally NBones and Bones can be specified for skinned meshes.
// If NBones is >0 but Bones is not specified, the reference pose will be left uninitialized
// (Bones also does not need to be specified if NGLP_BONES_LOCAL is used).
// The new mesh is set up for editing automatically.
// To get a pointer to the mesh, finish building it and call nglCloseMesh.
// XBox specific: if you want to use another vertex format than the default one (nglVertexGeneric),
//                you have to specify the vertex size.
void nglCreateMesh(uint32 NSections, uint32 NBones = 0, nglMatrix *Bones = 0, uint32 VertexSize = 0);

// Call this when you're done creating the mesh to get the finalized pointer to it.  DON'T call this when editing a mesh.
nglMesh *nglCloseMesh();

// Destroy a non-temporary mesh.
void nglDestroyMesh(nglMesh *Mesh);

void nglSetMeshFlags(uint32 Flags);
uint32 nglGetMeshFlags();

// Call this to use the mesh editing API to edit a mesh.
// Note that any changes will affect the actual data, and thus all instances of the mesh in the scene.
void nglEditMesh(nglMesh *Mesh);

// Calculate a bounding sphere for the current mesh.  Will only take into account the first NVerts vertices.
void nglMeshCalcSphere(uint32 NVerts = 0);

// Calculate a bounding sphere for the current mesh.
void nglMeshSetSphere(nglVector &Center, float Radius);

// Add a new section to the mesh.  Must be called as many times as NSections was passed to nglCreateMesh.
// It is possible to specify the type of primitive the section is made of (by default triangle strips - see NGLPRIM_xxx).
// NVerts and NStrips are ignored on the XBox.
void nglMeshAddSection(nglMaterial *Material, uint32 NVerts, uint32 NStrips = 0, uint32 PrimType = NGLPRIM_TRISTRIP);

// Jump to a particular section of the current mesh.
void nglMeshSetSection(uint32 Idx);

// Jump to a particular vertex in the mesh.
void nglMeshSetVertex(uint32 VertIdx);

// Retrieves the contents of the current vertex into a structure.
struct nglScratchVertex
{
	float X, Y, Z;
	float NX, NY, NZ;
	uint32 Color;
	float U, V;
};
void nglMeshReadVertex(nglScratchVertex *Vertex);

// Add a primitive of an arbitrary length to the current scratch mesh.
// It is a extended version of nglMeshWriteStrip which works with any kind of supported primitive (see NGLPRIM_xxx).
// Call this before adding vertices.
void nglMeshWritePrimitive(uint32 VertexCount);

// These functions fill in the current section.  For newly created meshes, you must make as many WriteVertex calls as
// the number you passed in NVerts to nglMeshAddSection.  If you're working with a mesh resource, you can just fill in
// the parts you want changed (for example, using WriteVertexPN to leave Color and UV unchanged).
void nglMeshWriteVertexPC(float X, float Y, float Z, uint32 Color);
void nglMeshWriteVertexPCUV(float X, float Y, float Z, uint32 Color, float U, float V);
void nglMeshWriteVertexPCUVB(float X, float Y, float Z, uint32 Color, float U,
                             float V, int32 bones, float w1, float w2,
                             float w3, float w4, int32 b1, int32 b2, int32 b3, int32 b4);
void nglMeshWriteVertexPCUV2(float X, float Y, float Z, uint32 Color, float U,
                             float V, float U2, float V2);
void nglMeshWriteVertexPCUV2B(float X, float Y, float Z, uint32 Color,
                              float U, float V, float U2, float V2,
                              int32 bones, float w1, float w2, float w3,
                              float w4, int32 b1, int32 b2, int32 b3, int32 b4);
void nglMeshWriteVertexPUV(float X, float Y, float Z, float U, float V);
void nglMeshWriteVertexPN(float X, float Y, float Z, float NX, float NY, float NZ);
void nglMeshWriteVertexPNC(float X, float Y, float Z, float NX, float NY, float NZ, uint32 Color);
void nglMeshWriteVertexPNCUV(float X, float Y, float Z, float NX, float NY,
                             float NZ, uint32 Color, float U, float V);
void nglMeshWriteVertexPNUV(float X, float Y, float Z, float NX, float NY,
                            float NZ, float U, float V);
void nglMeshWriteVertexPNUVB(float X, float Y, float Z, float NX, float NY,
                             float NZ, float U, float V, int32 bones,
                             float w1, float w2, float w3, float w4, int32 b1,
                             int32 b2, int32 b3, int32 b4);
void nglMeshWriteVertexPNUV2(float X, float Y, float Z, float NX, float NY,
                             float NZ, float U, float V, float U2, float V2);
void nglMeshWriteVertexPNUV2B(float X, float Y, float Z, float NX, float NY,
                              float NZ, float U, float V, float U2, float V2,
                              int32 bones, float w1, float w2, float w3,
                              float w4, int32 b1, int32 b2, int32 b3, int32 b4);

// LEGACY INTERFACE - to be removed. Use nglMeshWritePrimitive insead !
void nglMeshWriteStrip(uint32 Length);
// END LEGACY INTERFACE

// LEGACY INTERFACE - to be removed!  See implementations of these functions in ngl_xbox.cpp to learn how to use the new API.
#define NGL_SCRATCH_MESH_INVALID  0xFFFFFFFF

uint32 nglCreateScratchMesh(int32 NVerts, bool Locked = false);
void nglScratchSetMaterial(nglMaterial *Mat);

inline void nglScratchAddStrip(uint32 Length)
{
	nglMeshWritePrimitive(Length);
}
inline void nglScratchAddVertexPC(float X, float Y, float Z, uint32 Color)
{
	nglMeshWriteVertexPC(X, Y, Z, Color);
}
inline void nglScratchAddVertexPCUV(float X, float Y, float Z, uint32 Color, float U, float V)
{
	nglMeshWriteVertexPCUV(X, Y, Z, Color, U, V);
}
inline void nglScratchAddVertexPUV(float X, float Y, float Z, float U, float V)
{
	nglMeshWriteVertexPUV(X, Y, Z, U, V);
}
inline void nglScratchAddVertexPN(float X, float Y, float Z, float NX, float NY, float NZ)
{
	nglMeshWriteVertexPN(X, Y, Z, NX, NY, NZ);
}
inline void nglScratchAddVertexPNC(float X, float Y, float Z, float NX, float NY, float NZ, uint32 Color)
{
	nglMeshWriteVertexPNC(X, Y, Z, NX, NY, NZ, Color);
}
inline void nglScratchAddVertexPNCUV(float X, float Y, float Z, float NX, float NY, float NZ, uint32 Color, float U, float V)
{
	nglMeshWriteVertexPNCUV(X, Y, Z, NX, NY, NZ, Color, U, V);
}

inline void nglScratchSetSphere(nglVector &Center, float Radius)
{
	nglMeshSetSphere(Center, Radius);
}
inline void nglScratchCalcSphere()
{
	nglMeshCalcSphere();
}

void nglListAddScratchMesh(uint32 ID, const nglMatrix &LocalToWorld, nglRenderParams *Params = 0);

#define LOG_SCRATCH_MESH_ID(c)
// END LEGACY INTERFACE

/*---------------------------------------------------------------------------------------------------------
  Quad API.

  Quads are 2D interface graphics that can be drawn on top of the scene.  Construct nglQuad structures
  using the following functions and pass them to nglListAddQuad.
---------------------------------------------------------------------------------------------------------*/

// Map flags.
enum
{
	// Filtering mode. If not set, point filtering is used.
	NGLMAP_BILINEAR_FILTER    = 0x00000001,

	// Mipmap filtering. Anisotropic is not supported on PS2 but provided for compiling convenience.
	NGLMAP_MIPMAP_POINT       = 0x00000010,
	NGLMAP_MIPMAP_LINEAR      = 0x00000020,
	NGLMAP_MIPMAP_ANISOTROPIC = NGLMAP_MIPMAP_LINEAR,

	// Clamping mode. Wraps if not set.
	NGLMAP_CLAMP_U            = 0x00000100,
	NGLMAP_CLAMP_V            = 0x00000200,
};

struct nglQuadVertex
{
	float X, Y;
	float U, V;
	uint32 Color;
};

struct nglQuad
{
	nglQuadVertex Verts[4];               // Vertex information.
	float Z;

	uint32 MapFlags;                      // Map flags. See NGLMAP_xxx for more details.

	uint32 BlendMode;                     // Blend mode.
	uint32 BlendModeConstant;

	nglTexture *Tex;

	uint32 VSHandle;                      // Vertex shader handle, used for sorting.
	void (*RenderQuadFunc) (void *Data);  // To override the default NGL renderer.
};

void nglInitQuad(nglQuad *Quad);           // Sets UV 0-1, zero rect, white color, NGLBM_BLEND, no texture.
void nglSetQuadZ(nglQuad *Quad, float z);  // Set Z (camera space).  Higher is farther.

// Set the quad's map flags. See NGLMAP_xxx for more details.
void nglSetQuadMapFlags(nglQuad *Quad, uint32 MapFlags);
// Return the quad's map flags. See NGLMAP_xxx for more details.
uint32 nglGetQuadMapFlags(nglQuad *Quad);
// Set the quad's blending mode.
void nglSetQuadBlend(nglQuad* Quad, uint32 Blend, uint32 Constant = 0);

// Simplified API.
// Set rectangle position.
void nglSetQuadRect(nglQuad *Quad, float x1, float y1, float x2, float y2);
// Set position.  Gets W/H from the texture (must already be set).
void nglSetQuadPos(nglQuad *Quad, float x, float y);
// Set rectangle UVs: Top Left, Bottom Right.
void nglSetQuadUV(nglQuad *Quad, float u1, float v1, float u2, float v2);
// Set color - use the NGL_RGBA32 macro.
void nglSetQuadColor(nglQuad *Quad, uint32 c);
// Set texture.
void nglSetQuadTex(nglQuad *Quad, nglTexture *Tex);

// Vertex API.  Verts go in this order: Top Left, Top Right, Bottom Left, Bottom Right.
// Set vertex position.
void nglSetQuadVPos(nglQuad *Quad, int32 VertIdx, float x, float y);
// Set vertex UV.
void nglSetQuadVUV(nglQuad *Quad, int32 VertIdx, float u, float v);
// Set vertex Color.
void nglSetQuadVColor(nglQuad *Quad, int32 VertIdx, uint32 Color);

// Rotate quad counterclockwise around (cx,cy) by theta radians.
void nglRotateQuad(nglQuad *Quad, float cx, float cy, float theta);

// Scale quad around (cx,cy) in screen coordinates by scale (sx,sy).
void nglScaleQuad(nglQuad *Quad, float cx, float cy, float sx, float sy);

void nglListAddQuad(nglQuad *Quad);

// LEGACY INTERFACE - to be removed!
inline void nglSetQuadB(nglQuad *Quad, uint32 Blend, uint32 Constant)
{
	nglSetQuadB(Quad, Blend, Constant);
}
inline void nglSetQuadR(nglQuad *Quad, float x1, float y1, float x2, float y2)
{
	nglSetQuadRect(Quad, x1, y1, x2, y2);
}
inline void nglSetQuadP(nglQuad *Quad, float x, float y)
{
	nglSetQuadPos(Quad, x, y);
}
inline void nglSetQuadC(nglQuad *Quad, float r, float g, float b, float a)
{
	nglSetQuadColor(Quad, NGL_RGBA32((r * 255.0f), (g * 255.0f), (b * 255.0f), (a * 255.0f)));
}
inline void nglSetQuadT(nglQuad *Quad, nglTexture *Tex)
{
	nglSetQuadTex(Quad, Tex);
}
inline void nglSetQuadC32(nglQuad *Quad, uint32 c)
{
	nglSetQuadColor(Quad, c);
}
inline void nglSetQuadVP(nglQuad *Quad, int32 VertIdx, float x, float y)
{
	nglSetQuadVPos(Quad, VertIdx, x, y);
}
inline void nglSetQuadVC32(nglQuad *Quad, int32 VertIdx, uint32 Color)
{
	nglSetQuadVC32(Quad, VertIdx, Color);
}

// END LEGACY INTERFACE

/*---------------------------------------------------------------------------------------------------------
  Font API.



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
void nglSetFontMapFlags(nglFont *Font, uint32 MapFlags);
// Return the font's map flags. See NGLMAP_xxx for more details.
uint32 nglGetFontMapFlags(nglFont *Font);
// Set the font's blend mode.
void nglSetFontBlend(nglFont* Font, uint32 BlendMode, uint32 Constant = 0);

// Add a string to the nodes' list to be rendered. The string may contains some formatting informations:
// - Font color is set by the token (hexa): \1[RRGGBBAA]
// - Font scale is set by the token: \2[SCALE]
// - Font independent scale is set by the token: \3[SCALEX,SCALEY]
// The default color is the Color argument and the default scale is 1.
// "Hi from \1[ff0000ff]NGL \1[ffffffff] :-)\n\n\2[2.0]Bye!");
// Writes "NGL" in red and "Bye" with a scale of 2.0.
void nglListAddString(nglFont *Font, float x, float y, float z, const char *Text, ...);

// nglListAddString variant where the color is specified as an input parameter.
void nglListAddString(nglFont *Font, float x, float y, float z, uint32 Color, const char *Text, ...);

// Get the dimension, in pixels, of a string (which can have scale tokens and cariage returns as well).
void nglGetStringDimensions(nglFont *Font, uint32 *Width, uint32 *Height, const char *Text, ...);

// LEGACY INTERFACE - to be removed!
void nglSetFont(nglFont* Font);   // pass nglSysFont here if you're just trying to get up and running.
void nglSetFontColor(uint32 c);
void nglSetFontZ(float z);
void nglSetFontScale(float xscale, float yscale);
void nglListAddString(float x, float y, const char* Text, ...);
void nglGetStringDimensions(int32* Width, int32* Height, char* Text, ...);
// END LEGACY INTERFACE

/*-------------------------------------------------------------------------------------------------------*/

#endif
