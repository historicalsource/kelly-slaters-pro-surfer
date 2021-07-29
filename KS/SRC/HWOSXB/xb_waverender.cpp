// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef TARGET_XBOX	// These are still in NGL for other platforms
#include "ngl_xbox_internal.h"
#undef NGL_DEBUG	// prevents nglListAddCustomMesh from compiling (dc 05/23/02)
#define PROJECT_KELLYSLATER	// It may be useful to record what code is KS-specific

#include "xb_waverender.h"

#include "waverendermenu.cpp"

// Stuff which needs to go in ngl_xbox_internal.h (dc 06/24/02)
extern int32 nglVSC2_Projector_LocalToScreen;
extern nglMatrix nglProjVertexTransform;  // Cache matrix for projected light. Avoid to calculate the vertex transform several times.
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
extern nglProjectorParamStruct nglProjectorParam[NGL_MAX_PROJECTORS];
struct nglProjectorRegStruct
{
	// Registers indices.
	int32 Local2UV;
	int32 Dir;
};
extern nglProjectorRegStruct nglVSC2_Projector[NGL_MAX_TS];
extern int32 nglVSC2_Projector_TexScale;
extern nglProjectorRegStruct nglVSC2_Projector[NGL_MAX_TS];
extern DWORD nglVSDirProjector[4];
extern char NGL_VSC_SHADOW_FADING[16];
extern void nglSetupRuntimeVSKey(void *NodeData);

// Environmental bump map vertex shader declaration.
const DWORD nglVS_Bump_Decl[] = 
{
  D3DVSD_STREAM(0),
  D3DVSD_REG(0, D3DVSDT_FLOAT3),       // x,y,z
  D3DVSD_REG(1, D3DVSDT_D3DCOLOR),     // ARGB color
  D3DVSD_REG(2, D3DVSDT_NORMPACKED3),  // normal (packed)
  D3DVSD_REG(3, D3DVSDT_FLOAT2),       // map UVs
#ifdef PACK_BASIS_VECTORS	// faster upload to shader, but slower CPU time, for packing
  D3DVSD_REG(4, D3DVSDT_NORMPACKED3),  // S = d<x,y,z> / ds --> object space direction of increasing s
  D3DVSD_REG(5, D3DVSDT_NORMPACKED3),  // T = d<x,y,z> / dt --> object space direction of increasing t
  D3DVSD_REG(6, D3DVSDT_NORMPACKED3),  // SxT
#else
  D3DVSD_REG(4, D3DVSDT_FLOAT3),       // S = d<x,y,z> / ds --> object space direction of increasing s
  D3DVSD_REG(5, D3DVSDT_FLOAT3),       // T = d<x,y,z> / dt --> object space direction of increasing t
  D3DVSD_REG(6, D3DVSDT_FLOAT3),       // SxT
#endif
  D3DVSD_REG(7, D3DVSDT_PBYTE1),       // Foam alpha
  D3DVSD_END()
};

// Vertex type for bump mapped water.
#pragma pack(push,1)	// allows types of size not a multiple of 4

struct NormPacked3 {
    int32 x : 11, y : 11, z : 10;
};

struct WAVERENDER_Vertex
{
  XGVECTOR3 Pos;            // Position in local space.
  uint32 Diffuse;           // ARGB packed color.
  NormPacked3 Norm;         // Packed normal.
  XGVECTOR2 UV;             // map UVs
#ifdef PACK_BASIS_VECTORS
  NormPacked3 S;
  NormPacked3 T;
  NormPacked3 SxT;
#else
  D3DXVECTOR3 S;
  D3DXVECTOR3 T;
  D3DXVECTOR3 SxT;
#endif
  BYTE FA;       // Foam alpha
};

#pragma pack(pop)

static const u_int WAVERENDER_NumSubmissions = 2;

static nglShader WAVERENDER_PS[WAVERENDER_NumSubmissions];
static bool WAVERENDER_PSValid = false;

static nglShader WAVERENDER_VS[WAVERENDER_NumSubmissions];
static bool WAVERENDER_VSValid = false;

// Store the final VS source code.
char WAVERENDER_VSSrc[4096];
// Store each VS fragment source code.
char WAVERENDER_VSBuf[4096];

int32 WAVERENDER_VSCBumpCutoff;
int32 WAVERENDER_VSCBumpFalloff;
int32 WAVERENDER_VSCBumpScale;
int32 WAVERENDER_VSCBasisTransform;
int32 WAVERENDER_VSCBumpEye;
#ifdef PROJECT_KELLYSLATER 
int32 WAVERENDER_VSCCameraDir;
int32 WAVERENDER_VSCWaterFarColor;
int32 WAVERENDER_VSCWaterNearColor;
int32 WAVERENDER_VSCWaterNearFarScale;
int32 WAVERENDER_VSCWaterNearFarOffset;
int32 WAVERENDER_VSCWaterNearFarMin;
int32 WAVERENDER_VSCWaterNearFarMax;
int32 WAVERENDER_VSCSunDir;
int32 WAVERENDER_VSCWaterSpecularColor;
int32 WAVERENDER_VSCWaterSpecularScale;
int32 WAVERENDER_VSCWaterSpecularOffset;
#endif

// Bumpmap settings
nglVector nglBumpScale(0, -30.f * 1.f/200.f, 30.f, 0.5f);	// y is cutoff, z is falloff, w is scale
static XGVECTOR4 nglWaterFarColor(.9f, .9f, .9f, 1);
static XGVECTOR4 nglWaterNearColor(0, .6f, .5f, 1);
static float nglAlphaFalloffScale = 1;
static float nglAlphaFalloffOffset = 0;
static float nglAlphaFalloffMin = 0;
static float nglAlphaFalloffMax = 1;
static float nglBumpFalloffScale = 1;
static float nglBumpFalloffOffset = 0;
static float nglBumpFalloffMin = 0;
static float nglBumpFalloffMax = 1;
static float nglRGBFalloffScale = 1;
static float nglRGBFalloffOffset = 0;
static float nglRGBFalloffMin = 0;
static float nglRGBFalloffMax = 1;
static XGVECTOR4 nglWaterSpecularColor(1, 1, 1, 1);
static XGVECTOR4 nglSunDir(0, 0, 1, 1);
static float nglSpecularFalloffScale = 1;
static float nglSpecularFalloffOffset = 0;

// Local function prototypes
static void WAVERENDER_RenderSectionIgnore(void *Data);
void WAVERENDER_RenderSectionWave0(void *Data);
void WAVERENDER_RenderSectionWave1(void *Data);

/*-----------------------------------------------------------------------------
Description: Begin the vertex shader source code.
-----------------------------------------------------------------------------*/
void ksvsf_begin()
{
	WAVERENDER_VSSrc[0] = 0;

/*
	sprintf(WAVERENDER_VSBuf, "xvs.1.1\n" "#pragma screenspace\n");	// Done in nglCreateVertexShader

	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
*/
}

/*-----------------------------------------------------------------------------
Description: End the vertex shader source code (in other words it calculates
             the screen space scale and offset).
-----------------------------------------------------------------------------*/
void ksvsf_end()
{
/*	// Done in nglCreateVertexShader
	sprintf(WAVERENDER_VSBuf,
			// Scale.
			";---- vsf_end\n" "mul oPos.xyz, r12, c[%d]\n"
			// Compute 1/w.
			"+ rcc r1.x, r12.w\n"
			// Scale by 1/w, add offset.
			"mad oPos.xyz, r12, r1.x, c[%d]\n",
			nglVSC_Scale, nglVSC_Offset);

	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);

#if NGLDEBUG
	if (nglDebug.ShowVSCode)
		nglPrintf(";----------------------------------\n%s----------------------------------\n", WAVERENDER_VSSrc);
#endif
*/
}

/*-----------------------------------------------------------------------------
Description: Transform a non skinned vertex.
-----------------------------------------------------------------------------*/
void ksvsf_transform()
{
	sprintf(WAVERENDER_VSBuf,
	";---- vsf_transform\n"
	"dp4 oPos.x, v0, c[%d]\n"
	"dp4 oPos.y, v0, c[%d]\n"
	"dp4 oPos.z, v0, c[%d]\n"
	"dp4 oPos.w, v0, c[%d]\n", 
	nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1,
	nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3);
	
	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}

/*-----------------------------------------------------------------------------
Description: Setup the map UVs.
-----------------------------------------------------------------------------*/
void ksvsf_map(uint32 Stage)
{
	sprintf(WAVERENDER_VSBuf, 
	";---- vsf_map\n" 
	"mov oT%d.xy, v3\n", 
	Stage);

	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}

/*-----------------------------------------------------------------------------
Description: Setup the bumpmap UVs.  Same as the diffuse map's UVs.
-----------------------------------------------------------------------------*/
void ksvsf_bumpmap(void)
{
	sprintf(WAVERENDER_VSBuf,
	
	"; This shader does the per-vertex dot3 work.\n"
	"; It transforms the light vector by the basis vectors\n"
	"; passed into the shader.\n"
	"; The basis vectors only need to change if the model's\n"
	"; shape changes.\n"
	"; The output vector is stored in the diffuse channel\n"
	"; (use the menu to look at the generated light vector)\n"
	
	"#define V_POSITION v0\n"
	"#define V_DIFFUSE v1\n"
	"#define V_NORMAL v2\n"
	"#define V_TEXTURE v3\n"
	"#define V_S v4\n"
	"#define V_T v5\n"
	"#define V_SxT v6        ;  as an alternative SxT may be computed on the fly\n"
	
	"#define WORLD_VERTEX	r0\n"
#ifdef WAVERENDER_COMPUTE_BASIS_VECTORS	// could be done here or in CPU (dc 06/21/02)
	"#define SxT             r1\n"
#else
	"#define SxT             V_SxT\n"
#endif
	"#define S               r2\n"
	"#define T               r3\n"
	"#define EYE_VECTOR      r4\n"
	"#define DROPOFF         r5\n"
	
#ifdef STANDALONE	// This will be done by ksvsf_transform
	"; Transform position to clip space and output it\n"
	"dp4 oPos.x, V_POSITION, c[%d]	; CV_WORLDVIEWPROJ_0\n"
	"dp4 oPos.y, V_POSITION, c[%d]	; CV_WORLDVIEWPROJ_1\n"
	"dp4 oPos.z, V_POSITION, c[%d]	; CV_WORLDVIEWPROJ_2\n"
	"dp4 oPos.w, V_POSITION, c[%d]	; CV_WORLDVIEWPROJ_3\n"
#endif
#ifdef STANDALONE	// This will be overwritten by ksvsf_nearfar
	"mov oD0, V_DIFFUSE	; Pass vertex color to pixel shader\n"	
#endif
	
#ifdef COMPUTE_BASIS_VECTORS	// could be done here or in CPU (dc 06/21/02)
	"; Normalize S and T\n"
	"dp3 S.w, V_S, V_S\n"
	"rsq S.w, S.w\n"
	"mul S, V_S, S.w\n"
	
	"dp3 T.w, V_T, V_T\n"
	"rsq T.w, T.w\n"
	"mul T, V_T, T.w\n"
	
	"; here we compute SxT (if V_SxT is available, then this is unnecessary, of course)\n"
	"mul SxT, S.zxyw, T.yzxw           ; 2 instruction cross product\n"
	"mad SxT, S.yzxw, T.zxyw, -SxT\n"
	
	"dp3 SxT.w, SxT, SxT                 ; and 3 instruction normalization --- S and T need not be orthogonal\n"
	"rsq SxT.w, SxT.w\n"
	"mul SxT.xyz, SxT, SxT.w\n"
	
	"; Scale the bumps\n"
	"mul S, S, c[%d].w	; CV_BUMPSCALE\n"
	"mul T, T, c[%d].w	; CV_BUMPSCALE\n"
#else
	"; Scale the bumps\n"
	"mul S, V_S, c[%d].w	; CV_BUMPSCALE\n"
	"mul T, V_T, c[%d].w	; CV_BUMPSCALE\n"
#endif
	
#ifdef LOCAL_TO_WORLD
	"; Calculate the eye vector in world space\n"
	"dp3 WORLD_VERTEX.x, V_POSITION, c[%d]	; CV_WORLD_0	; apply local-to-world transform to vertex\n"
	"dp3 WORLD_VERTEX.y, V_POSITION, c[%d]	; CV_WORLD_1\n"
	"dp3 WORLD_VERTEX.zw, V_POSITION, c[%d]	; CV_WORLD_2\n"
	"add EYE_VECTOR, -WORLD_VERTEX, c[%d]	; CV_EYE_WORLD	; vector from vertex to eye\n"
#else	// Local to world assumed to be the identity (but can have offset)
	"add EYE_VECTOR, -V_POSITION, c[%d]	; CV_EYE_WORLD	; vector from vertex to eye\n"
#endif

#ifdef PROJECT_KELLYSLATER
	"dp3 EYE_VECTOR.w, EYE_VECTOR, EYE_VECTOR\n"
	"rsq EYE_VECTOR.w, EYE_VECTOR.w\n"
	"mad DROPOFF.w, EYE_VECTOR.w, c[%d].z, c[%d].y\n"	// scale * w + offset
	"min DROPOFF.w, DROPOFF.w, c[%d].z\n"
	"max DROPOFF.w, DROPOFF.w, c[%d].x\n"
	"mul S, S, DROPOFF.w\n"
	"mul T, T, DROPOFF.w\n"
#endif
	
	"; Rotate the basis vectors by the world transform to put them into world space\n"
	"dp3 oT1.x, S,  c[%d]	; CV_BASISTRANSFORM_0	; world-to-local transform???\n"
	"dp3 oT1.y, T,  c[%d]	; CV_BASISTRANSFORM_0\n"
	"dp3 oT1.z, SxT, c[%d]	; CV_BASISTRANSFORM_0\n"
	
	"dp3 oT2.x, S,  c[%d]	; CV_BASISTRANSFORM_1\n"
	"dp3 oT2.y, T,  c[%d]	; CV_BASISTRANSFORM_1\n"
	"dp3 oT2.z, SxT, c[%d]	; CV_BASISTRANSFORM_1\n"
	
	"dp3 oT3.x, S,  c[%d]	; CV_BASISTRANSFORM_2\n"
	"dp3 oT3.y, T,  c[%d]	; CV_BASISTRANSFORM_2\n"
	"dp3 oT3.z, SxT, c[%d]	; CV_BASISTRANSFORM_2\n"
	
	"; store the eye vector in the texture coordinate w for\n"
	"; the pixel shader\n"
	"mov oT1.w, EYE_VECTOR.x\n"
	"mov oT2.w, EYE_VECTOR.y\n"
	"mov oT3.w, EYE_VECTOR.z\n"
	
	"mov oT0.xy, V_TEXTURE\n", 
	
#ifdef STANDALONE	// This will be done by ksvsf_transform
	nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1, nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3, 
#endif
	WAVERENDER_VSCBumpScale, WAVERENDER_VSCBumpScale, 
#ifdef LOCAL_TO_WORLD
	nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1, nglVSC_LocalToWorld + 2, 
#endif
	WAVERENDER_VSCBumpEye,  
#ifdef PROJECT_KELLYSLATER
	WAVERENDER_VSCBumpFalloff, 
	WAVERENDER_VSCBumpCutoff, 
	nglVSC_Util, 
	nglVSC_Util, 
#endif
	WAVERENDER_VSCBasisTransform + 0, WAVERENDER_VSCBasisTransform + 0, WAVERENDER_VSCBasisTransform + 0,
	WAVERENDER_VSCBasisTransform + 1, WAVERENDER_VSCBasisTransform + 1, WAVERENDER_VSCBasisTransform + 1,
	WAVERENDER_VSCBasisTransform + 2, WAVERENDER_VSCBasisTransform + 2, WAVERENDER_VSCBasisTransform + 2
	);
	
	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}

#ifdef PROJECT_KELLYSLATER
#define ALPHA_FALLOFF
/*-----------------------------------------------------------------------------
Description: Modify vertex color / alpha based on angle of camera vector to 
vertex normal.
-----------------------------------------------------------------------------*/
#define PERVERTEX_EYE
void ksvsf_water_nearfar()
{
	sprintf(WAVERENDER_VSBuf,
	
#ifdef STANDALONE	// so they will be the same as in ksvsf_bump
	"#define V_POSITION v0\n"
	"#define V_DIFFUSE v1\n"
	"#define V_NORMAL v2\n"
	"#define V_TEXTURE v3\n"
	
	"#define WORLD_VERTEX	r0\n"
	"#define EYE_VECTOR   r4\n"
#endif
	"#define FALLOFF_COLOR      r1\n"
	"#define TEMP   r5\n"
	"#define NEARFAR   r6\n"
	
#ifdef STANDALONE	// done already in ksvsf_bump
	//Calculate the eye vector in world space
	// For optimization, we should re-use the calculation from nglvsf_bump
	"dp3 WORLD_VERTEX.x, V_POSITION, c[%d]	; CV_WORLD_0	; apply local-to-world transform to vertex\n"
	"dp3 WORLD_VERTEX.y, V_POSITION, c[%d]	; CV_WORLD_1\n"
	"dp3 WORLD_VERTEX.zw, V_POSITION, c[%d]	; CV_WORLD_2\n"
	"add EYE_VECTOR, -WORLD_VERTEX, c[%d]	; CV_EYE_WORLD	; vector from vertex to eye\n"

	// Normalize
	"dp3 EYE_VECTOR.w, EYE_VECTOR, EYE_VECTOR\n"
	"rsq EYE_VECTOR.w, EYE_VECTOR.w\n"
#endif
	"mul EYE_VECTOR, EYE_VECTOR, EYE_VECTOR.w\n"
	
	// Compute cos of angle between camera vector and vertex normal
	"dp3 EYE_VECTOR.w, EYE_VECTOR, V_NORMAL\n"	// This assumes normal vector is in world space!!!
	
	// Compute near / far factor
	// Factor for Alpha falloff in x, Bump falloff in y, RGB falloff in z
	"mov TEMP, c[%d]\n"
	"mad NEARFAR, EYE_VECTOR.w, TEMP, c[%d]\n"	// scale * w + offset
	"max NEARFAR, NEARFAR, c[%d]\n"	// >= 0, or other min value
	"min NEARFAR, NEARFAR, c[%d]\n"	// <= 1, or other max value
	
	// Compute near / far water falloff and store result in vertex color
	"mad FALLOFF_COLOR.xyz, c[%d].xyz, -NEARFAR.z, c[%d].xyz\n"		// (1-falloff) * far color
	"mad FALLOFF_COLOR.xyz, c[%d].xyz, NEARFAR.z, FALLOFF_COLOR.xyz\n"	// + falloff * near color
	"mul oD0.xyz, FALLOFF_COLOR.xyz, V_DIFFUSE.xyz\n"
	
	// cap at 1
//	"min oD0.xyz, oD0.xyz, c[%d].z\n"	// hardware will cap oD0 automatically

#ifdef ALPHA_FALLOFF
	"mad oD0.w, -V_DIFFUSE.w, NEARFAR.x, NEARFAR.x\n"	// a = (1 - a_diffuse) * a_nearfar
#endif
	"mov oD1.w, NEARFAR.y\n",	// store near/far coefficient in specular alpha
	
#ifdef STANDALONE	// done already in ksvsf_bump
	nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1, nglVSC_LocalToWorld + 2, 
	nglVSC_CameraPos, 
#endif
	WAVERENDER_VSCWaterNearFarScale, 
	WAVERENDER_VSCWaterNearFarOffset,
	WAVERENDER_VSCWaterNearFarMin, 
	WAVERENDER_VSCWaterNearFarMax, 
	WAVERENDER_VSCWaterFarColor, WAVERENDER_VSCWaterFarColor, 
	WAVERENDER_VSCWaterNearColor, 
#ifdef ALPHA_FALLOFF
	nglVSC_Util, 
	nglVSC_Util, 
#endif
	nglVSC_Util
	);

	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}

void ksvsf_water_specular()
{
	sprintf(WAVERENDER_VSBuf,
	
	"#define V_POSITION v0\n"
	"#define V_DIFFUSE v1\n"
	"#define V_NORMAL v2\n"
	"#define V_TEXTURE v3\n"
	
	"#define WORLD_VERTEX	r0\n"
	"#define REFLECTION_VECTOR   r3\n"
	"#define EYE_VECTOR   r4\n"
	"#define TEMP   r5\n"
	"#define SPECULAR   r6\n"
	
#ifdef PERVERTEX_EYE
	//Calculate the eye vector in world space
	// For optimization, we should re-use the calculation from nglvsf_bump
	"dp3 WORLD_VERTEX.x, V_POSITION, c[%d]	; CV_WORLD_0	; apply local-to-world transform to vertex\n"
	"dp3 WORLD_VERTEX.y, V_POSITION, c[%d]	; CV_WORLD_1\n"
	"dp3 WORLD_VERTEX.zw, V_POSITION, c[%d]	; CV_WORLD_2\n"
	"add EYE_VECTOR, -WORLD_VERTEX, c[%d]	; CV_EYE_WORLD	; vector from vertex to eye\n"
	
	// Normalize
	"dp3 EYE_VECTOR.w, EYE_VECTOR, EYE_VECTOR\n"
	"rsq EYE_VECTOR.w, EYE_VECTOR.w\n"
	"mul EYE_VECTOR, EYE_VECTOR, -EYE_VECTOR.w\n"
#else
	// Could combine this with next instruction
	"mov EYE_VECTOR, -c[%d]\n"
#endif
	
	// Calculate reflection vector: E - 2 * (E dot N) * N 
	"dp3 REFLECTION_VECTOR.w, EYE_VECTOR, V_NORMAL\n"	// This assumes normal is given in world space!!!
	"add REFLECTION_VECTOR.w, REFLECTION_VECTOR.w, REFLECTION_VECTOR.w\n"
	"mad REFLECTION_VECTOR.xyz, V_NORMAL, -REFLECTION_VECTOR.w, EYE_VECTOR\n"
	
	// Compute cos of angle between reflection vector and sun vector
	"dp3 REFLECTION_VECTOR.w, REFLECTION_VECTOR, c[%d]\n"
	
	// Compute factor for Color/Alpha falloff in x
	"mov TEMP, c[%d]\n"
	"mad SPECULAR, REFLECTION_VECTOR.w, TEMP, c[%d]\n"	// scale * w + offset
	"max SPECULAR, SPECULAR, c[%d].x\n"	// >= 0
	"min SPECULAR, SPECULAR, c[%d].z\n"	// <= 1
	
	// Compute vertex color falloff, and store result in vertex color oD1 (oD0 used for foam pass).
	"mul oD1, c[%d], SPECULAR.x\n", 	// falloff * specular color
	
#ifdef PERVERTEX_EYE
	nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1, nglVSC_LocalToWorld + 2, 
	nglVSC_CameraPos, 
#else
	WAVERENDER_VSCCameraDir,
#endif
	WAVERENDER_VSCSunDir, 
	WAVERENDER_VSCWaterSpecularScale, 
	WAVERENDER_VSCWaterSpecularOffset,
	nglVSC_Util, 
	nglVSC_Util, 
	WAVERENDER_VSCWaterSpecularColor
	);
	
	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}

void ksvsf_water_foam()
{
	sprintf(WAVERENDER_VSBuf,
	
	"#define V_DIFFUSE v1\n"
	"#define V_FOAM v7\n"
	
	// Replace regular alpha by foam alpha.
	"mov oD1.xyz, c[%d].z\n"
	"mov oD1.w, V_FOAM.x\n", 
	nglVSC_Util 
	);
	
	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}

void ksvsf_1projector_dir()
{
	sprintf(WAVERENDER_VSBuf, ";---- vsf_projector_dir\n"
#ifdef STANDALONE
		// Transform the vertex position.
		"dp4 oPos.x, v0, c[%d]\n"
		"dp4 oPos.y, v0, c[%d]\n"
		"dp4 oPos.z, v0, c[%d]\n" 
		"dp4 oPos.w, v0, c[%d]\n"
#endif
		// Transform the UV coordinates.
		"dp4 r0.x, v0, c[%d]\n"
		"dp4 r0.y, v0, c[%d]\n" 
		"mul oT0.xy, r0.xy, c[%d].xx\n"
//#if 0
		// Normalize normal vector in r2.
		"dp3 r2.w, v2, v2\n" "rsq r2.w, r2.w\n" "mul r2, v2, r2.w\n"
		// Calculate dot(-LightDir, N). Clamping to [0,1].
		"dp3 r0.x, c[%d], -r2\n"
		"max r0.x, r0.x, %s\n"
		"mul oD0.a, r0.x, %s\n"
//#endif
		, 
#ifdef STANDALONE
		nglVSC2_Projector_LocalToScreen + 0,
		nglVSC2_Projector_LocalToScreen + 1,
		nglVSC2_Projector_LocalToScreen + 2,
		nglVSC2_Projector_LocalToScreen + 3,
#endif
		nglVSC2_Projector[0].Local2UV + 0,
		nglVSC2_Projector[0].Local2UV + 1, 
		nglVSC2_Projector_TexScale
//#if 0
		, 
		nglVSC2_Projector[0].Dir, 
		NGL_VSC_ZERO, 
		NGL_VSC_SHADOW_FADING
//#endif
	);
	
	strcat(WAVERENDER_VSSrc, WAVERENDER_VSBuf);
}
#endif

static void WAVERENDER_CompilePS(void)
{
	nglCreatePixelShader(&WAVERENDER_PS[0],
		"tex t0\n"
		"texm3x3pad t1, t0_bx2\n"
		"texm3x3pad t2, t0_bx2\n"
		"texm3x3vspec t3, t0_bx2\n"
		"xmma discard.rgb, discard.rgb, r0.rgb, t3.rgb, 1-v1.a, v0.rgb, 1-zero\n"
		"+mov r0.a, 1-v0.a\n"
		"xfc 1-zero, r0, zero, zero, zero, zero, r0.a"
	);
	
/*	For specular in additive mode
	nglCreatePixelShader(&WAVERENDER_PS[1],
		"tex t0\n"  // foam map
		"tex t1\n"  // specular map

		// modulate by vertex colors
		"mul t0, t0, v0\n"
		"mul t1, t1, v1\n"

		// perform the alpha blending here, then set alpha to 1
		"xmma discard.rgb, discard.rgb, r0.rgb, t0.a, t0.rgb, t1.a, t1.rgb\n"
		"+mov r0.a, 1-zero.a\n"
	);
*/
	
	nglCreatePixelShader(&WAVERENDER_PS[1],
		"tex t0\n"  // shadow map
		"tex t1\n"  // foam map
		"mul r0, t0, v0.a\n"	// modulate by vertex color
		"lrp r0, v1.a, t1, r0\n"
		"xfc 1-zero, r0, zero, zero, zero, zero, r0.a"
	);

	WAVERENDER_PSValid = true;
}

static void WAVERENDER_CompileVS(void)
{
	ksvsf_begin();
	ksvsf_transform();
	ksvsf_bumpmap();
	ksvsf_water_nearfar();
	ksvsf_end();

	nglCreateVertexShader(nglVS_Bump_Decl, &WAVERENDER_VS[0], true, WAVERENDER_VSSrc);

	ksvsf_begin();
	ksvsf_transform();
	ksvsf_map(1);
	ksvsf_water_foam();
	ksvsf_1projector_dir();
	ksvsf_end();

	nglCreateVertexShader(nglVS_Bump_Decl, &WAVERENDER_VS[1], true, WAVERENDER_VSSrc);
	
	WAVERENDER_VSValid = true;
}

static void WAVERENDER_CompileShaders(void)
{
	if (!WAVERENDER_PSValid)
	{
		WAVERENDER_CompilePS();
	}

	if (!WAVERENDER_VSValid)
	{
		WAVERENDER_CompileVS();
	}
}

void WAVERENDER_Init()
{
	int32 reg = NGL_VS_FIRST_USER_REG;	// Call must come after nglInitShadersConst()

	// Initialize Vertex Shader register assignments
	WAVERENDER_VSCBumpScale = WAVERENDER_VSCBumpFalloff = WAVERENDER_VSCBumpCutoff = reg; reg++;	// share a register
	WAVERENDER_VSCBasisTransform = reg; reg += 4;
	WAVERENDER_VSCBumpEye = reg; reg++;
#ifdef PROJECT_KELLYSLATER	// Some of these can be shared
	WAVERENDER_VSCCameraDir = reg; reg++;
	WAVERENDER_VSCWaterNearFarScale = reg; reg++;
	WAVERENDER_VSCWaterNearFarOffset = reg; reg++;
	WAVERENDER_VSCWaterNearFarMin = reg; reg++;
	WAVERENDER_VSCWaterNearFarMax = reg; reg++;
	WAVERENDER_VSCWaterFarColor = reg; reg++;
	WAVERENDER_VSCWaterNearColor = reg; reg++;
	WAVERENDER_VSCWaterSpecularScale = reg; reg++;
	WAVERENDER_VSCWaterSpecularOffset = reg; reg++;
	WAVERENDER_VSCWaterSpecularColor = reg; reg++;
#endif

	if (reg > NGL_VS_LAST_REG)
		nglFatal("Too many vshader registers used %d ! (see nglInitShadersConst() and nglVSC_xxx variables)\n", reg);

	WAVERENDER_CompileShaders();	// Must be done before memory gets locked (dc 06/17/02)
}

void WAVERENDER_Cleanup(void)
{
	for (u_int i = 0; i < WAVERENDER_NumSubmissions; ++i)
	{
		nglReleaseShader(&WAVERENDER_VS[i]);
		nglReleaseShader(&WAVERENDER_PS[i]);
	}
	WAVERENDER_VSValid = false;
	WAVERENDER_PSValid = false;
}

void WAVERENDER_SetSectionRenderer(nglMesh *Mesh)
{
	if (WaverenderDebug.DrawSubmission0) 
	{
		nglSetSectionRenderer(&Mesh->Sections[0], WAVERENDER_RenderSectionWave0, &WAVERENDER_VS[0]);
	}
	else 
	{
		nglSetSectionRenderer(&Mesh->Sections[0], WAVERENDER_RenderSectionIgnore, &WAVERENDER_VS[0]);
	}

	if (WaverenderDebug.DrawSubmission1) 
	{
		nglSetSectionRenderer(&Mesh->Sections[1], WAVERENDER_RenderSectionWave1, &WAVERENDER_VS[1]);
	}
	else 
	{
		nglSetSectionRenderer(&Mesh->Sections[1], WAVERENDER_RenderSectionIgnore, &WAVERENDER_VS[1]);
	}
}

nglMesh *WAVERENDER_CreateScratchMesh(u_int NVerts, void (*FillInFunc)(void)) 
{
	nglMaterial Material;
	memset(&Material, 0, sizeof(nglMaterial));
	Material.MapBlendMode = NGLBM_OPAQUE;
	Material.Flags = NGLMAT_BILINEAR_FILTER | NGLMAT_PERSPECTIVE_CORRECT | NGLMAT_LIGHT;

	nglCreateMesh(2, 0, 0, sizeof(WAVERENDER_Vertex));
//	nglSetMeshFlags(NGLMESH_TEMP);	// Without this line, this function should only be called once per mesh.
	nglMeshAddSection(&Material, NVerts);

	assert(FillInFunc);
	FillInFunc();

	// Add a second section for the second submission
	nglMeshAddSection(&Material, 0);

	nglMesh *Mesh = nglCloseMesh();

	// Make Section 1 a partial copy of Section 0.  We will change the Material later.  (dc 05/28/02)
	Mesh->Sections[1].VB = Mesh->Sections[0].VB;	// already equal
	Mesh->Sections[1].PrimInfo = Mesh->Sections[0].PrimInfo;
	Mesh->Sections[1].PrimCount = Mesh->Sections[0].PrimCount;
	Mesh->Sections[1].PrimType = Mesh->Sections[0].PrimType;	// already equal

	return Mesh;
}

void nglSetBumpScale(float BumpScale) 
{
	nglBumpScale.w = BumpScale;
}

void nglSetWaterNearFarColors(XGVECTOR4 FarColor, XGVECTOR4 NearColor)
{
	nglWaterFarColor = FarColor;
	nglWaterNearColor = NearColor;
}

void nglSetWaterAlphaFalloff(float Scale, float Offset, float Min, float Max)
{
	nglAlphaFalloffScale = Scale;
	nglAlphaFalloffOffset = Offset;
	nglAlphaFalloffMin = Min;
	nglAlphaFalloffMax = Max;
}

void nglSetWaterBumpFalloff(float Scale, float Offset, float Min, float Max)
{
	nglBumpFalloffScale = Scale;
	nglBumpFalloffOffset = Offset;
	nglBumpFalloffMin = Min;
	nglBumpFalloffMax = Max;
}

void nglSetWaterRGBFalloff(float Scale, float Offset, float Min, float Max)
{
	nglRGBFalloffScale = Scale;
	nglRGBFalloffOffset = Offset;
	nglRGBFalloffMin = Min;
	nglRGBFalloffMax = Max;
}

void nglSetWaterSpecularColor(XGVECTOR4 SpecularColor)
{
	nglWaterSpecularColor = SpecularColor;
}

void nglSetWaterSpecularFalloff(float Scale, float Offset)
{
	nglSpecularFalloffScale = Scale;
	nglSpecularFalloffOffset = Offset;
}

void nglSetSunDir(XGVECTOR4 SunDir)
{
	nglSunDir = SunDir;
}

void WAVERENDER_SetVertexShader(nglMeshSectionNode *Node, uint32 MaterialFlags)
{
	nglSectionSetVertexShader(Node, MaterialFlags);

#ifdef PROJECT_KELLYSLATER
	nglMeshNode *MeshNode = Node->MeshNode;
	nglMeshSection *Section = Node->Section;
	nglMaterial *Material = Section->Material;

	if (MaterialFlags & NGLMAT_BUMP_MAP) 
	{
		// Probably redundant with nglVSC_LocalToWorld
		XGMATRIX m;
		XGMatrixTranspose(&m, (XGMATRIX*)&MeshNode->LocalToWorld);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCBasisTransform, &m, 4);
		
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCBumpScale, nglBumpScale, 1);
		
		// Probably redundant with nglVSC_CameraPos
		nglVector ViewWorldPos(
			nglCurScene->ViewToWorld[3][0],
			nglCurScene->ViewToWorld[3][1],
			nglCurScene->ViewToWorld[3][2],
			1.0f
		);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCBumpEye, ViewWorldPos, 1);
	}
	
	if (MaterialFlags & NGLMAT_WATER_NEARFAR)
	{
		// Get the view position vector into world space.
		nglVector WorldViewPos(
			nglCurScene->ViewToWorld[3][0],
			nglCurScene->ViewToWorld[3][1],
			nglCurScene->ViewToWorld[3][2],
			1.0f
		);
		nglDev->SetVertexShaderConstant(nglVSC_CameraPos, &WorldViewPos, 1);
		
		// Pass the local to world matrix to the vertex shader.
		XGMATRIX m;
		XGMatrixTranspose(&m, (XGMATRIX*)&MeshNode->LocalToWorld);
		nglDev->SetVertexShaderConstant(nglVSC_LocalToWorld, &m, 4);
		
		// World space vector pointing directly into the camera
		nglVector ViewWorldDir(
			-nglCurScene->ViewToWorld[2][0],
			-nglCurScene->ViewToWorld[2][1],
			-nglCurScene->ViewToWorld[2][2],
			1.0f
		);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCCameraDir, ViewWorldDir, 1);
		
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterFarColor, &nglWaterFarColor, 1);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterNearColor, &nglWaterNearColor, 1);
		
		XGVECTOR4 WaterScale(
			nglAlphaFalloffScale, 
			nglBumpFalloffScale, 
			nglRGBFalloffScale, 
			0
		);
		XGVECTOR4 WaterOffset(
			nglAlphaFalloffScale * (nglAlphaFalloffOffset - 0.5f) + 0.5f, 
			nglBumpFalloffScale * (nglBumpFalloffOffset - 0.5f) + 0.5f, 
			nglRGBFalloffScale * (nglRGBFalloffOffset - 0.5f) + 0.5f, 
			0
		);
		XGVECTOR4 WaterMin(
			nglAlphaFalloffMin, 
			nglBumpFalloffMin, 
			nglRGBFalloffMin, 
			0
		);
		XGVECTOR4 WaterMax(
			nglAlphaFalloffMax, 
			nglBumpFalloffMax, 
			nglRGBFalloffMax, 
			0
		);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterNearFarScale, &WaterScale, 1);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterNearFarOffset, &WaterOffset, 1);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterNearFarMin, &WaterMin, 1);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterNearFarMax, &WaterMax, 1);
	}
	
	if (MaterialFlags & NGLMAT_WATER_SPECULAR)
	{
		// Get the view position vector into world space.
		nglVector WorldViewPos(
			nglCurScene->ViewToWorld[3][0],
			nglCurScene->ViewToWorld[3][1],
			nglCurScene->ViewToWorld[3][2],
			1.0f
		);
		nglDev->SetVertexShaderConstant(nglVSC_CameraPos, &WorldViewPos, 1);
		
		// Pass the local to world matrix to the vertex shader.
		XGMATRIX m;
		XGMatrixTranspose(&m, (XGMATRIX*)&MeshNode->LocalToWorld);
		nglDev->SetVertexShaderConstant(nglVSC_LocalToWorld, &m, 4);
		
		// World space vector pointing directly into the camera
		nglVector ViewWorldDir(
			-nglCurScene->ViewToWorld[2][0],
			-nglCurScene->ViewToWorld[2][1],
			-nglCurScene->ViewToWorld[2][2],
			1.0f
		);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCCameraDir, ViewWorldDir, 1);
		
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCSunDir, &nglSunDir, 1);
		
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterSpecularColor, &nglWaterSpecularColor, 1);
		
		XGVECTOR4 WaterScale(
			nglSpecularFalloffScale, 
			0, 
			0, 
			0
		);
		XGVECTOR4 WaterOffset(
			nglSpecularFalloffScale * (nglSpecularFalloffOffset - 0.5f) + 0.5f, 
			0, 
			0, 
			0
		);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterSpecularScale, &WaterScale, 1);
		nglDev->SetVertexShaderConstant(WAVERENDER_VSCWaterSpecularOffset, &WaterOffset, 1);
	}
#endif	// #ifdef PROJECT_KELLYSLATER
}

void WAVERENDER_SetPixelShader(u_int Submission, nglMeshSectionNode *Node, uint32 MaterialFlags)
{
	nglSetPixelShader(WAVERENDER_PS[Submission].Handle);

#ifdef PROJECT_KELLYSLATER
	nglMeshNode *MeshNode = Node->MeshNode;
	nglMeshSection *Section = Node->Section;
	nglMaterial *Material = Section->Material;

	// Animated texture support.
	if (MeshNode->Params.Flags & NGLP_TEXTURE_FRAME)
		nglTextureAnimFrame = MeshNode->Params.TextureFrame;
	else
		nglTextureAnimFrame = nglCurScene->IFLFrame;

	switch (Submission) 
	{
	case 0:
		nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
 
		// Set the texture stages config.
		nglBindTexture(Material->DetailMap, MaterialFlags, 0);
		nglBindTexture(NULL, MaterialFlags, 1);	// slots 1 and 2 ignored, used for bump map calculation
		nglBindTexture(NULL, MaterialFlags, 2);
		nglBindTexture(Material->EnvironmentMap, MaterialFlags, 3);

		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		nglDev->SetTextureStageState(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);	// avoids seams across cube faces
		nglDev->SetTextureStageState(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		break;

	case 1:
		// Stage 0 is the shadow map
		nglBindTexture(Material->Map, MaterialFlags, 1);
		nglBindTexture(Material->EnvironmentMap, MaterialFlags, 2);
		nglBindTexture(NULL, MaterialFlags, 3);

		nglDev->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		nglDev->SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		nglDev->SetTextureStageState(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		nglDev->SetTextureStageState(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		break;

	default:
		assertmsg(0, "Invalid submission number.\n");
		break;
	}

	// Pass the blend mode constants to the pshader.
/*
	if ((Material->Flags & NGLMAT_BUMP_MAP) && (Material->Flags & NGLMAT_WATER_NEARFAR))
	{
		// Set the pixel shader constants.
		nglVector v0 = { 0.0f, 0.0f, 0.0f, 0.0f };
static float nglBumpVertPSConst = .5f;
		v0[3] = nglBumpVertPSConst;
		nglDev->SetPixelShaderConstant(PSC_BUMPMAP_VERT_BM_CONST, &v0, 1);
		nglVector v1 = { 0.0f, 0.0f, 0.0f, 0.0f };
static float nglBumpEnvPSConst = .5f;
		v1[3] = nglBumpEnvPSConst;
		nglDev->SetPixelShaderConstant(PSC_BUMPMAP_ENV_BM_CONST, &v1, 1);
	}
*/
#endif	// #ifdef PROJECT_KELLYSLATER
}

/*-----------------------------------------------------------------------------
Description: Don't render the section.
-----------------------------------------------------------------------------*/
void WAVERENDER_RenderSectionIgnore(void *Data)
{
}

/*-----------------------------------------------------------------------------
Description: Render a section of a water mesh.
-----------------------------------------------------------------------------*/
void WAVERENDER_RenderSectionWave(u_int Submission, void *Data)
{
	nglMeshSectionNode *Node = (nglMeshSectionNode *) Data;
	nglMesh *Mesh = Node->MeshNode->Mesh;
	nglMeshSection *Section = Node->Section;
	nglMaterial *Material = Section->Material;

#ifdef PROJECT_KELLYSLATER
// Since we don't get the automatic call in nglRenderScene. (dc 06/24/02)
	nglSetupRuntimeVSKey(Data);
#endif

	uint32 MaterialFlags = Node->MaterialFlags;
	MaterialFlags &= nglMaterialFlagsAnd;
	MaterialFlags |= nglMaterialFlagsOr;

#if NGL_DEBUG
	if (!nglStage.MatAlpha && Material->Flags & NGLMAT_ALPHA)
		return;
	if (!nglStage.MatLight && Material->Flags & NGLMAT_LIGHT)
		return;
	// Skip this section if it matches the specified texture stage combination.
	uint32 TS = (uint32) (Section->VSKey & NGL_VSFLAG_TS_MASK);
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
		nglSetCullingMode(MaterialFlags, Node->MeshNode->Params.Flags);
	else
		nglDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
#else
	nglSetCullingMode(MaterialFlags, Node->MeshNode->Params.Flags);
#endif

	// If tint makes the object translucent (if it was opaque), force the blending of the mesh with the framebuffer.
	// NOTE: MaterialFlags override the original Material->Flags value, BUT Material->Flags remains unmodified.
	// A more readable way of writting this line would be:
	// if ((Node->MeshNode->Params.Flags & NGLP_TINT) && (Node->MeshNode->Params.TintColor[3] < 1.0f) &&
	//     (Material->MapBlendMode == NGLBM_OPAQUE || Material->MapBlendMode == NGLBM_PUNCHTHROUGH))
	if ((MaterialFlags & NGLMAT_ALPHA) && (!(Material->Flags & NGLMAT_ALPHA)))
		nglSetBlendingMode(NGLBM_BLEND, 0);
	else
		// Set the blending mode with the framebuffer according to the diffuse1 map's parameters.
		nglSetBlendingMode(Material->MapBlendMode, Material->MapBlendModeConstant);

#ifdef PROJECT_KELLYSLATER
	nglDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);	// Write to z-buffer even though translucent
	nglFlushBlendingMode();  // Flush the blending mode cache.
#endif
   
extern IDirect3DVertexBuffer8 *nglScratchVertBuf;	// Needed for rendering custom scratch mesh
	// Set the stream source.
	if (Mesh->Flags & NGLMESH_TEMP)
		nglBindVertexBuffer(nglScratchVertBuf, Mesh->VertexSize);
	else
#if !NGL_FORCE_CREATE_VB
		nglBindVertexBuffer((D3DVertexBuffer *) &Node->Section->VB->VertexBuffer, Mesh->VertexSize);
#else
		nglBindVertexBuffer((D3DVertexBuffer *) Node->Section->VB->VertexBufferData, Mesh->VertexSize);
#endif

#ifdef PROJECT_KELLYSLATER
	// Needed by NGL even though we override it later on. (dc 06/24/02)
	nglSectionSetVertexShader(Node, MaterialFlags);
#endif

	// Projected lights rendering.
	if (Node->LightConfig)
	{
		uint32 ProjCount = ((nglLightConfigStruct *) Node->LightConfig)->ProjLightCount;

#ifdef PROJECT_KELLYSLATER
		assert (ProjCount == 1);	// one projector light in texture slot 0
#endif

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

#ifndef PROJECT_KELLYSLATER	// we combine the shaders and the draw with our own passes (dc 06/24/02)
				// Set the vertex shader.
				nglSetVertexShader(nglVSDirProjector[TexStagesUsed - 1]);

				// Set the pixel shader.
				nglSetPixelShader(nglPixelShaderHandle[NGL_PS_1PROJ + TexStagesUsed - 1]);

				nglDrawSection(Node);
#endif
			}

			nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			nglFlushBlendingMode();
		}
	}

	// Setup the vertex shader and vertex shader constants
	WAVERENDER_SetVertexShader(Node, MaterialFlags);

	// Setup the pixel shader and pixel shader constants, and apply the texture stages.
	WAVERENDER_SetPixelShader(Submission, Node, MaterialFlags);

	// Render the section. Could be a scratch or standard mesh.
	nglDrawSection(Node);

#if NGL_DEBUG
	if (nglDebug.DispMat && (nglCurScene->RenderTarget->Format & NGLTF_LINEAR))
	{
		NGL_D3DTRY(nglDev->Present(NULL, NULL, NULL, NULL));

		if (nglDebug.InfoPrints)
			nglInfo("Section %d (mesh)\n", nglSectionNumber++);
	}
#endif
}

void WAVERENDER_RenderSectionWave0(void *Data)
{
	WAVERENDER_RenderSectionWave(0, Data);
}

void WAVERENDER_RenderSectionWave1(void *Data)
{
	WAVERENDER_RenderSectionWave(1, Data);
}

void WAVERENDER_ScratchAddVertexPNCUVSTR(float X, float Y, float Z, float NX, float NY,
                                 float NZ, uint32 Color, float U, float V, 
								 float SX, float SY, float SZ, float TX, float TY, float TZ, 
								 float RX, float RY, float RZ, uint8 FA
								 )
{
extern uint8 *nglScratchTempVertBufPtr;
	WAVERENDER_Vertex *Vert = (WAVERENDER_Vertex *) (nglScratchTempVertBufPtr);

	Vert->Pos = XGVECTOR3(X, Y, Z);
	Vert->Norm.x = nglFTOI(NX * ((1 << 10) - 1));
	Vert->Norm.y = nglFTOI(NY * ((1 << 10) - 1));
	Vert->Norm.z = nglFTOI(NZ * ((1 << 9) - 1));
	Vert->Diffuse = Color;
	Vert->UV = XGVECTOR2(U, V);
#ifdef PACK_BASIS_VECTORS
	Vert->S.x = nglFTOI(SX * ((1 << 10) - 1));
	Vert->S.y = nglFTOI(SY * ((1 << 10) - 1));
	Vert->S.z = nglFTOI(SZ * ((1 << 9) - 1));
	Vert->T.x = nglFTOI(TX * ((1 << 10) - 1));
	Vert->T.y = nglFTOI(TY * ((1 << 10) - 1));
	Vert->T.z = nglFTOI(TZ * ((1 << 9) - 1));
	Vert->SxT.x = nglFTOI(RX * ((1 << 10) - 1));
	Vert->SxT.y = nglFTOI(RY * ((1 << 10) - 1));
	Vert->SxT.z = nglFTOI(RZ * ((1 << 9) - 1));
#else
	Vert->S = D3DXVECTOR3(SX, SY, SZ);	// It would make sense to pack these vectors like Normal. (dc 04/14/02)
	Vert->T = D3DXVECTOR3(TX, TY, TZ);
	Vert->SxT = D3DXVECTOR3(RX, RY, RZ);
#endif
	Vert->FA = FA;

	nglScratchTempVertBufPtr += sizeof(*Vert);
}


#ifdef PROJECT_KELLYSLATER 
void nglSetGammaRamp(const char *FileName)
{
	D3DGAMMARAMP GammaRamp;

	//	To load Gamma from a Photoshop .raw file (dc 06/26/02)
	nglFileBuf FileBuf;
	nglReadFile((char *) FileName, &FileBuf);	// Cast away const because prototypes don't match!
	if (FileBuf.Size != sizeof(GammaRamp.red) + sizeof(GammaRamp.green) + sizeof(GammaRamp.blue))
	{
	nglWarning("Gamma file %s not found or has wrong size.", FileName);
	return;
	}
	uint8 *GammaPtr = FileBuf.Buf;
		
/*
	uint8 Gamma[3 * 256];	// Gamma used by PS2 (dc 06/26/02)
	const uint8 *GammaPtr = Gamma;
	for (u_int i = 0; i < 255; ++i)
	{
		Gamma[0 * 256 + i] = 
		Gamma[1 * 256 + i] = 
		Gamma[2 * 256 + i] = (uint8) min(max((0.925f * (float) i) + 19.25f, 0), 255);
	}
*/
	
	memcpy(GammaRamp.red, GammaPtr, sizeof(GammaRamp.red));
	GammaPtr += sizeof(GammaRamp.red) / sizeof (*GammaPtr);
	memcpy(GammaRamp.green, GammaPtr, sizeof(GammaRamp.green));
	GammaPtr += sizeof(GammaRamp.green) / sizeof (*GammaPtr);
	memcpy(GammaRamp.blue, GammaPtr, sizeof(GammaRamp.blue));
	nglDev->SetGammaRamp(0, &GammaRamp);
}
#endif

#endif	// #ifdef TARGET_XBOX