/*---------------------------------------------------------------------------------------------------------
  NGL_XBOX_INTERNAL.H - Low level access to the NGL module for writing Custom Nodes and Extensions.

  Florent Gluck (florent@treyarch.com)
---------------------------------------------------------------------------------------------------------*/

#ifndef NGL_XBOX_INTERNAL_H
#define NGL_XBOX_INTERNAL_H

/*---------------------------------------------------------------------------------------------------------
  System headers.
---------------------------------------------------------------------------------------------------------*/
#include <xgraphics.h>
#include <xgmath.h>
#include <xbdm.h>
#include "ngl_Instbank.h"
#include "ngl_PixelShadersConst.h"
#include "ngl_dds.h"

/*---------------------------------------------------------------------------------------------------------
  NGL internal constants.
---------------------------------------------------------------------------------------------------------*/

#define NGL_SCREEN_WIDTH            640
#define NGL_SCREEN_HEIGHT           480

#define NGL_DEPTHBUFFER_FORMAT      D3DFMT_D24S8
#define NGL_Z_MAX                   (float)D3DZ_MAX_D24S8

#define NGL_DEPTHBUFFER_WIDTH       640
#define NGL_DEPTHBUFFER_HEIGHT      512

// For now, the XBox will not support more than 8 lights.
// This value can't be modified (!), it's deeply hardcoded into the NGL library.
#define NGL_MAX_LIGHTS              8

// Maximum number of projectors (must be a multiple of 4 !).
#define NGL_MAX_PROJECTORS          12

// These three enum must match the ones in the mesh converter (meshcvt).
enum
{
	NGL_XBMESH_VERSION = 0x305,
	NGL_VS_MAX_BONES = 46,		// Max bones supported by the NGL vertex shaders.
	NGL_MAX_BONESPERVERTEX = 4,	// Max bones per vertex.
};

// XBox hardware supports a max of 4 texture stages.
#define NGL_MAX_TS                  4

/*-----------------------------------------------------------------------------
Description: Usefull functions.
-----------------------------------------------------------------------------*/

// Assert implementation.
#if defined(_DEBUG) || defined(DEBUG)
extern void nglPrintf(const char *format, ...);
#define NGL_ASSERT(f, text) { if (!(f)) nglFatal("ASSERT in %s, line %d: %s\n", __FILE__, __LINE__, text); }
#else
#define NGL_ASSERT(f, text) {}
#endif

// Return the number of element in a static array.
#define elementsof(x)               (sizeof(x) / sizeof(x[0]))

// Matrix/vector copy shortcut.
#define nglMatrixCopy(dest, src)    memcpy(dest, src, sizeof(nglMatrix))
#define nglVectorCopy(dest, src)    memcpy(dest, src, sizeof(nglVector))

// Usefull to convert from RGBA (ps2) to ARGB (xbox).
#define RGBA2ARGB(c)                (((c >> 8) & 0xFFFFFF) | ((c & 0xFF) << 24))

#define NGL_PI                      3.14159265359f

static inline float nglMinF(float a, float b)
{
	if (a < b)
		return a;

	return b;
}
static inline float nglMaxF(float a, float b)
{
	if (a > b)
		return a;

	return b;
}

static inline int32 nglMinI(int32 a, int32 b)
{
	if (a < b)
		return a;

	return b;
}
static inline int32 nglMaxI(int32 a, int32 b)
{
	if (a > b)
		return a;

	return b;
}

// Convert from float to integer.
static inline int32 nglFTOI(float f)
{
	int32 n;

	__asm
	{
		fld dword ptr[f]
		fistp dword ptr[n]
	}

	return n;
}

static inline DWORD nglFloat2Dword(float f)
{
	return *((DWORD *) & f);
}

// Return true if x is a power of 2.
static inline bool nglPowerOf2(uint32 x)
{
	return x && !(x & (x - 1));
}

// Return the number of vblanks.
static inline uint32 nglGetVBlankCount()
{
	D3DFIELD_STATUS d3dStatus;
	nglDev->GetDisplayFieldStatus(&d3dStatus);
	return d3dStatus.VBlankCount;
}

#if NGL_DEBUG
extern bool nglCheckD3DError(HRESULT status, const char *filename,
							 int32 linenum);
#define NGL_D3DTRY(exp) nglCheckD3DError(exp, __FILE__, __LINE__)
#else
#define NGL_D3DTRY(exp) exp
#endif

/*---------------------------------------------------------------------------------------------------------
  Pixel shaders stuff.
---------------------------------------------------------------------------------------------------------*/

// Pixel shaders types.
// Pixel shader ending with _MC suffix are used when NGLMAT_MATERIAL_COLOR is set.
enum
{
	// 1st pass
	NGL_PS_NOTEX,				// Pixel shader used for untextured polys.

	NGL_PS_T,					// map (opaque) | enviro (opaque).
	NGL_PS_T_MC,				// map (opaque) | enviro (opaque).
	NGL_PS_E,
	NGL_PS_E_MC,

	NGL_PS_TD,					// map (opaque) + detail (addsign) .
	NGL_PS_TD_MC,				// map (opaque) + detail (addsign) .
	NGL_PS_TE,					// map (opaque) + enviro (blend const).
	NGL_PS_TE_MC,				// map (opaque) + enviro (blend const).
	NGL_PS_TL,					// map (opaque) | enviro (opaque) + light (blend).
	NGL_PS_TL_MC,				// map (opaque) | enviro (opaque) + light (blend).
	NGL_PS_TL2,					// map (opaque) + light (additive).
	NGL_PS_TL2_MC,				// map (opaque) + light (additive).
	NGL_PS_EL,
	NGL_PS_EL_WIN,				// Special pixel shader to render framed-windows (made for Spiderman).

	NGL_PS_TDE,					// map (opaque) + detail (addsign) + enviro (blend const).
	NGL_PS_TDE_MC,				// map (opaque) + detail (addsign) + enviro (blend const).
	NGL_PS_TDL,					// map (opaque) + detail (addsign) + light (blend).
	NGL_PS_TDL_MC,				// map (opaque) + detail (addsign) + light (blend).
	NGL_PS_TEL,					// map (opaque) + enviro (blend const) + light (blend).
	NGL_PS_TEL_MC,				// map (opaque) + enviro (blend const) + light (blend).
	NGL_PS_TEL2,				// map (opaque) + enviro (additive) + light (blend).
	NGL_PS_TEL2_MC,				// map (opaque) + enviro (additive) + light (blend).
	NGL_PS_TEL3,				// map (opaque) + enviro (blend const) + light (additive).
	NGL_PS_TEL3_MC,				// map (opaque) + enviro (blend const) + light (additive).

	NGL_PS_TDEL,				// map (opaque) + detail (addsign) + enviro (blend const) + light (blend).
	NGL_PS_TDEL_MC,				// map (opaque) + detail (addsign) + enviro (blend const) + light (blend).

	// 2nd pass
	NGL_PS_1PROJ,				// 1 projector.
	NGL_PS_2PROJ,				// 2 projectors.
	NGL_PS_3PROJ,				// 3 projectors.
	NGL_PS_4PROJ,				// 4 projectors.

	NGL_PS_MAX,					// Used to retrieve the #elements (#pixel shaders).

	// Used to rerieve the PS for the first pass. Required by nglTSFunc().
	NGL_PS_MAX_PASS1 = NGL_PS_1PROJ
};

// Pixel shaders used by NGL.
extern DWORD nglPixelShaderHandle[NGL_PS_MAX];

/*---------------------------------------------------------------------------------------------------------
  Vertex shaders stuff.
---------------------------------------------------------------------------------------------------------*/

// Vertex formats used by NGL.
#pragma pack(push,1)

// Vertex type for non-skin meshes.
struct nglVertexBasic
{
	XGVECTOR3 Pos;              // Position in local space.
	uint32 Diffuse;             // ARGB packed color.
	struct                      // Packed normal.
	{
		int32 x:11, y:11, z:10;
	}
	Norm;
	XGVECTOR2 UV[2];            // map UVs, lightmap UVs.
};

// Vertex type for skin meshes.
struct nglVertexSkin
{
	XGVECTOR3 Pos;              // Position in local space.
	uint32 Diffuse;             // ARGB packed color.
	struct                      // Packed normal.
	{
		int32 x:11, y:11, z:10;
	}
	Norm;
	XGVECTOR2 UV[2];            // Texture coordinates (diffuse, lightmap).
	uint8 BoneIdx[NGL_MAX_BONESPERVERTEX];     // Bone indices.
	uint8 BoneWeight[NGL_MAX_BONESPERVERTEX];  // Bone weights [0,255] then converted to [0,1] when passed through the VS.
};

typedef nglVertexBasic nglVertexGeneric;

// Vertex type for quads (screen space).
struct nglVertexQuad
{
	XGVECTOR3 Pos;              // Position in screen space.
	uint32 Color;               // ARGB packed color.
	XGVECTOR2 UV;
};

#pragma pack(pop)

// Vertex shader vertex types declaration (must match the different nglVertexXXX types).
// These declarations must match the registers defined above.
extern const DWORD nglVS_Quads_Decl[];
extern const DWORD nglVS_NonSkin_Decl[];
extern const DWORD nglVS_Skin_Decl[];

// Light types.
enum
{
	NGL_DIR_LIGHT = 1,
	NGL_PNT_LIGHT = 2,
};

struct nglLightStruct
{
	uint32 Type;
	int32 Dir;
	int32 Col;
	int32 Near;
	int32 Far;
};

// Vertex shader registers go from -96 to 95.
enum
{
	NGL_VS_FIRST_REG = -96,  // First register to be used by the nglVSC_ variables.
	NGL_VS_LAST_REG = 95,
};

// First register which is free for client assignment.
extern int32 NGL_VS_FIRST_USER_REG;

// Variables used to store the different VS register values required for section rendering.
// Initialized in the nglInitVertexShadersConst() function.
extern int32 nglVSC_Util;
extern int32 nglVSC_Util2;
extern int32 nglVSC_Scale;
extern int32 nglVSC_Offset;
extern int32 nglVSC_FogParams;
extern int32 nglVSC_UVParams;
extern int32 nglVSC_MaterialColor;
extern int32 nglVSC_Bones;
extern nglLightStruct nglVSC_Light[NGL_MAX_LIGHTS];

/*---------------------------------------------------------------------------------------------------------
  Render list stuff.
---------------------------------------------------------------------------------------------------------*/

// Render list node types.
enum
{
	NGLNODE_BIN,
	NGLNODE_MESH_SECTION,
	NGLNODE_QUAD,
	NGLNODE_CUSTOM,
};

struct nglListNode
{
	nglListNode *Next;          // Next node in the render order.
	uint32 Type;                // Node Type.

	// Sorting parameters.
	nglSortInfo SortInfo;

	// Type-specific node data.
	nglCustomNodeFn NodeFn;
	void *NodeData;
};

// Adds a new node to the render list, after sorting it correctly. This will cause the node to be rendered
// by nglListSend calling the function NodeFn with the Data pointer.
void nglListAddNode(uint32 Type, nglCustomNodeFn NodeFn, void *Data, nglSortInfo *SortInfo);

// Fills out a SortInfo structure for use with nglListAddNode.
// VShaderHandle is a handle to the vertex shader to use to render this node.
void nglGetSortInfo(nglSortInfo *SortInfo, uint32 BlendMode, DWORD VShaderHandle, float Dist);

// Allocates from the render list storage space (which is completely cleared each call to nglListInit).
void *nglListAlloc(uint32 Bytes, uint32 Alignment = 16);

#define nglListNew(Type) (Type*)nglListAlloc(sizeof(Type))

/*---------------------------------------------------------------------------------------------------------
  Scene/node stuff.
---------------------------------------------------------------------------------------------------------*/

// This is an object shared by mesh material nodes in the render list, it's a single instance of a mesh in the scene.
struct nglMeshNode
{
	// These matrices MUST be on a 16-bytes boundary !
	nglMatrix LocalToWorld;     // Local to world matrix.
	nglMatrix WorldToLocal;     // Inverse of the above, for lighting in local space.
	nglMatrix LocalToScreen;

	nglMesh *Mesh;              // Mesh resource.
	nglRenderParams Params;     // Custom render parameters.
};

struct nglLightConfigStruct
{
	// Using 8-bits fields to save a bit of memory...
	uint8 Type[NGL_MAX_LIGHTS + NGL_MAX_PROJECTORS];  // Size must be a multiple of 4.
	uint8 DirLightCount;   // Number of directional lights affecting the section.
	uint8 PntLightCount;   // Number of point lights affecting the section.
	uint8 ProjLightCount;  // Number of projected lights affecting the section.
	void *LightPtr[NGL_MAX_LIGHTS + NGL_MAX_PROJECTORS];
};

struct nglMeshSectionNode
{
	nglMeshNode *MeshNode;      // Mesh node structure shared by all the nodes.
	nglMeshSection *Section;    // What section of the mesh to render.
	uint32 MaterialFlags;       // Copy of material flags, possibly with changes.

	uint32 FullVSKey;           // Vertex shader key that stores the vshader to use for this section.
	uint32 VSHandle;            // Vertex shader handle, used for sorting.

	nglLightConfigStruct *LightConfig;
};

// Made to look like a nglListNode.
struct nglListBin
{
	nglListNode *Next;          // Next node in the render order.
	uint32 Type;
};

class nglScene
{
  public:
	// These matrices MUST be on a 16-bytes boundary !
	nglMatrix ViewToWorld;      // Camera -> World
	nglMatrix ViewToScreen;     // Camera -> Screen
	nglMatrix WorldToView;      // World -> Camera
	nglMatrix WorldToScreen;    // World -> Camera -> Screen

	// Scene hierarchy info.
	nglScene *Parent;
	nglScene *NextSibling;
	nglScene *FirstChild;
	nglScene *LastChild;

	nglTexture *RenderTarget;   // Usually nglBackBufferTex, or a texture for the scene
	// to be rendered into.

	nglListNode *OpaqueRenderList;
	nglListNode *TransRenderList;
	uint32 OpaqueListCount, TransListCount;

	// Viewport.
	uint32 ViewX1, ViewY1, ViewX2, ViewY2;

	// Screen clear settings.
	uint32 ClearFlags;
	float ClearZ;
	uint32 ClearStencil;  // XBox specific flag.
	uint32 ClearColor;    // Packed ARGB color.

	// Fog settings.
	bool Fog;  // Enable fog on a per-scene-basis (override NGLMAT_FOG).
	float FogNear;
	float FogFar;
	float FogMin;
	float FogMax;
	uint32 FogColor;  // Packed ARGB color.

	// Saved parameters to nglSetPerspectiveMatrix.
	float HFOV;
	float CX;
	float CY;
	float NearZ;
	float FarZ;
	float ZMin;  // ZMin, ZMax range is [0,1].
	float ZMax;
	bool UsingOrtho;

	float AnimTime;
	uint32 IFLFrame;

	float ScrZ;

	// XBox specific. Write mask when rendering the geometry to the framebuffer.
	uint32 FBWriteMask;

	bool ZWriteEnable;
	bool ZTestEnable;

	uint32 Pad[2];
};

// Calls outside nglListInit/nglListSend affect the global scene.
extern nglScene nglDefaultScene;

// Currently active scene.
extern nglScene *nglCurScene;

// Depth surface used for z-buffer/stencil depth comparisons.
extern LPDIRECT3DSURFACE8 nglDepthBufferSurface;

/*---------------------------------------------------------------------------------------------------------
  XBMESH file stuff.
---------------------------------------------------------------------------------------------------------*/

// Pathnames to append when loading meshes/textures.
#define NGL_MAX_PATH_LENGTH         256

// Mesh File structure, these are contained in nglMeshFileBank.
struct nglMeshFile
{
	nglFixedString FileName;
	char FilePath[NGL_MAX_PATH_LENGTH];

	nglFileBuf FileBuf;
	bool LoadedInPlace;

	nglMesh *FirstMesh;
};

// Instance banks for meshes.  Use these to insert and delete resources.
extern nglInstanceBank nglMeshFileBank;
extern nglInstanceBank nglMeshBank;

// Current animation frame for animated textures.  Warning: This global is sometimes used as a parameter.
extern int32 nglTextureAnimFrame;

// Debugging flag to optionally turn off and on material flags.
extern uint32 nglMaterialFlagsAnd;
extern uint32 nglMaterialFlagsOr;

/*---------------------------------------------------------------------------------------------------------
  Lighting stuff.
---------------------------------------------------------------------------------------------------------*/

struct nglLightNode
{
	nglLightNode *Next[NGL_MAX_LIGHTS];  // Next light in scene.
	uint32 Type;
	void *Data;                          // Copy of the light structure.
	uint32 LightCat;                     // Lighting category flags.
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

extern nglLightContext *nglDefaultLightContext;
extern nglLightContext *nglCurLightContext;

/*---------------------------------------------------------------------------------------------------------
  Math stuff.
---------------------------------------------------------------------------------------------------------*/
// Clip planes. Code relies on the order.
enum
{
	NGLCLIP_NEAR = 0,
	NGLCLIP_FAR = 1,
	NGLCLIP_LEFT = 2,
	NGLCLIP_RIGHT = 3,
	NGLCLIP_TOP = 4,
	NGLCLIP_BOTTOM = 5,
	NGLCLIP_MAX = 6
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
	nglPlane Planes[6];  // These planes are in world space: left, right, back, top, bottom, front;
};

// Generalized frustum support.
void nglBuildFrustum(nglFrustum *frustum, nglMatrix & m);
bool nglIsSphereVisible(nglFrustum *Frustum, const nglVector & Center, float Radius);

// XBox specific function (mainly used in midnight).
void nglSetCameraMatrix(nglMatrix &WorldToView, const nglVector Pos, const nglVector YDir, const nglVector ZDir);

/*---------------------------------------------------------------------------------------------------------
  CUSTOM RENDER API

  Usefull for custom nodes or when overriding the default NGL rendering section function.
---------------------------------------------------------------------------------------------------------*/

// Here is a small example that shows how to use the API:

/*
nglShader myVS, myPS;

void myRenderSectionFunc(void* Data)
{
  nglMeshSectionNode* Node = (nglMeshSectionNode*)Data;
  nglMesh* Mesh = Node->MeshNode->Mesh;
  nglMeshSection* Section = Node->Section;
  nglMaterial* Material = Section->Material;

  // Turn off backface culling.
  nglDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  nglFlushBackFaceCulling();

  // Set the blending mode with the framebuffer.
  nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  nglDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  nglFlushBlendingMode();

  // Set the stream source.
  nglBindVertexBuffer((D3DVertexBuffer*)&Node->Section->VB->VertexBuffer, Mesh->VertexSize);

  // Setup the vertex shader.
  // Pass the local to screen matrix to the vertex shader.
  XGMATRIX LocalToScreen;
  XGMatrixTranspose(&LocalToScreen, (XGMATRIX*)Node->MeshNode->LocalToScreen);
  nglDev->SetVertexShaderConstant(nglVSC_LocalToScreen, &LocalToScreen, 4);
  nglSetVertexShader(Section->VSHandle);

  // Setup the pixel shader.
  nglBindTexture(Material->Map, Material->Flags, 0);
  nglSetPixelShader(myPS.Handle);

  // Render the section.
  nglVertexBuffer* VB = Node->Section->VB;
  uint16* Idx = (uint16*)((uint32)VB->IndexBufferData + ((uint32)Section->IndexOffset << 1));
  NGL_D3DTRY(nglDev->DrawIndexedVertices(D3DPT_TRIANGLESTRIP, Section->IndexCount, Idx));
}

void Init()
{
  ...

  // Create the vertex shader.
  nglCreateVertexShader(nglVS_NonSkin_Decl, &myVS, true,
    "dp4 oPos.x, v0, c[%d]\n"
    "dp4 oPos.y, v0, c[%d]\n"
    "dp4 oPos.z, v0, c[%d]\n"
    "dp4 oPos.w, v0, c[%d]\n"
    "mov oT0.xy, v3\n"
    "mov oFog, %s\n"
    "mov oD0, v1\n"
    "mov oD1, v1\n",
    nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1, nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3,
    NGL_VSC_ONE
    );

  // Create the pixel shader.
  nglCreatePixelShader(&myPS,
    "def c1,0,1,0,1\n"
    "tex t0\n"
    "mul r0, t0, c1\n"
    "xfc fog.a, r0, fog.rgb, zero, zero, zero, r0.a\n"
    );

  // Override the section's renderer by our custom one.
  nglSetSectionRenderer(&Entity->Mesh->Sections[0], myRenderSectionFunc, &myVS);

  // Write this if you want to restore the NGL internal renderer.
  //nglSetSectionRenderer(&Entity->Mesh->Sections[0], NULL, NULL);

  ...
*/

// Shader generic type (vertex or pixel shader).
struct nglShader
{
	DWORD Handle;
	LPXGBUFFER Opcode;
};

// Set the PS to use (cached).
extern void nglSetPixelShader(uint32 PSHandle);

// Set the VS to use (cached).
extern void nglSetVertexShader(uint32 VSHandle);

// Usefull when the user creates/compiles its own vertex shader.
extern char NGL_VSC_ZERO[16];
extern char NGL_VSC_HALF[16];
extern char NGL_VSC_ONE[16];
extern char NGL_VSC_TWO[16];

// Reserved register indices. You have to use them when passing any of these matrix/vectors,
// otherwise NGL vshader code will be broken.
extern int32 nglVSC_LocalToScreen;
extern int32 nglVSC_CameraPos;
extern int32 nglVSC_LocalToWorld;
extern int32 nglVSC_Tint;

// Cache flushing functions.
void nglFlushBlendingMode();
void nglFlushBackFaceCulling();
void nglFlushVertexShader();
void nglFlushPixelShader();

// Internal NGL functions used for section rendering (could be usefull for the user).
void nglSetCullingMode(uint32 MaterialFlags, uint32 ParamFlags);
void nglSetBlendingMode(int32 BlendMode, int32 BlendModeConst);
void nglBindVertexBuffer(IDirect3DVertexBuffer8 *VertexBuffer, uint32 VertexSize);
void nglSectionSetVertexShader(nglMeshSectionNode *Node, uint32 MaterialFlags);
void nglSectionSetPixelShader(nglMeshSectionNode *Node, uint32 MaterialFlags);
void nglDrawSection(nglMeshSectionNode *Node);
void nglBindTexture(nglTexture *tex, uint32 MaterialFlags, uint32 stage);
DWORD nglGetVertexShaderFromDB(uint32 key);

// Pixel shader infos:
//   v0 = diffuse color
//   v1 = specular color
//   r0 = final output color
//   r1 = temporary register
//   Final default combiner to use: xfc fog.a, r0, fog.rgb, zero, zero, zero, r0.a

void nglCreatePixelShader(nglShader *PS, const char *SrcCode, ...);

// Vertex shader infos:
// Input registers:
//   v0 = Position
//   v1 = Color (RGBA)
//   v2 = Normal
//   v3 = UV set 0 (map)
//   v4 = UV set 1 (light map)
//   v5 = Bones indices
//   v6 = Bones weights
// Output registers:
//   oPos = final position in screen space
//   oD0  = diffuse color (r,g,b,a)
//   oD1  = specular color
//   oFog = fog amount (0 = full, 1 = no fog)
//   oT0  = 1st UVs
//   oT1  = 2nd UVs
//   oT2  = 3rd UVs
//   oT3  = 4th UVs

void nglCreateVertexShader(const DWORD *Decl, nglShader *VS, bool WTransform, const char *SrcCode, ...);

void nglReleaseShader(nglShader *shader);

void nglSetSectionRenderer(nglMeshSection *Section, void (*RenderSectionFunc) (void *Data), nglShader * VS);
void nglSetQuadRenderer(nglQuad *Quad, void (*RenderQuadFunc) (void *Data), nglShader *VS);

#endif
