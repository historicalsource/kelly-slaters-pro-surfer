// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef TARGET_XBOX	// These are still in NGL for other platforms
#include "osparticle.h"
#include "ksngl.h"	// Basic NGL includes
#include "ngl_xbox_internal.h"	// Secret NGL includes

/*---------------------------------------------------------------------------------------------------------

  Particle System API.
  
---------------------------------------------------------------------------------------------------------*/

// Particle constants
#define PARTICLE_MAX_VERTS     4000 * 4
#define PARTICLE_VB_COUNT      3

// Particle system vertex shader declaration.
const DWORD PARTICLE_VS_Decl[] = 
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),       // x,y,z
	D3DVSD_REG(1, D3DVSDT_D3DCOLOR),     // ARGB color
	D3DVSD_REG(2, D3DVSDT_FLOAT2),       // map UVs
	D3DVSD_REG(3, D3DVSDT_FLOAT2),       // lightmap UVs
	D3DVSD_END()
};

// Shaders for particle system.
static nglShader PARTICLE_PS;
static nglShader PARTICLE_VS;

// Shader registers
int32 PARTICLE_VSC_ViewToScreen;
int32 PARTICLE_VSC_MaxScreenSize;

// Particles
IDirect3DVertexBuffer8* PARTICLE_VertBuf[PARTICLE_VB_COUNT];
IDirect3DVertexBuffer8* PARTICLE_CurVertBuf;
uint16 PARTICLE_VertCount = 0;
uint8* PARTICLE_CurWork = 0;
uint16 PARTICLE_Max = 0; // Max number of particles per frame

#pragma pack(push,1)
// Vertex type for non-skin meshes.
struct ParticleVertex
{
	XGVECTOR3 Pos;            // Position in local space.
	uint32 Diffuse;           // ARGB packed color.
	XGVECTOR2 UV;             // Texture map UVs
	XGVECTOR2 Scale;          // Particle size.
};
#pragma pack(pop)

/*-----------------------------------------------------------------------------
Description: Lock the particles vertex buffer.
This function MUST NOT be called more than once a frame.
-----------------------------------------------------------------------------*/
void PARTICLE_LockVertexBuffer()
{
	static uint32 CurVB = 0;
	
	if (PARTICLE_VertBuf[CurVB]->IsBusy())
	{
		if (++CurVB >= PARTICLE_VB_COUNT)
			CurVB = 0;
	}
	
	// Lock the entire particles vertex buffer (supposed faster than only locking the used portion).
	PARTICLE_CurVertBuf = PARTICLE_VertBuf[CurVB];
	PARTICLE_CurVertBuf->Lock(0, 0, (BYTE**)&PARTICLE_CurWork, 0);
	
	PARTICLE_VertCount = 0;
}

/*-----------------------------------------------------------------------------
Description: Sets the maximum number of particles allowed per frame.
-----------------------------------------------------------------------------*/
void PARTICLE_SetMax(uint16 Num)
{
	PARTICLE_Max = Num;
}

/*-----------------------------------------------------------------------------
Description: Render the particle system.
-----------------------------------------------------------------------------*/
void PARTICLE_RenderSystem(void* Data)
{
	ParticleSystem *ps = (ParticleSystem *)Data;
	const float inv_r = 1.0f / RAND_MAX;
	uint16 particles_added = 0, i;
	
	Random Rand(ps->Seed);
	
	// Create a small sine table to speed up the calculations.
	const int sin_num = 32;
	static float sin_table[sin_num];
	static bool sin_init = false;
	
	if (!sin_init)
	{
		for (int i = 0; i < sin_num; i++)
			sin_table[i] = (float)sin((float)i/sin_num * 2 * 3.1415f);
		sin_init = true;
	}
	
// move the sqrt out of here
float MaxSize = (float)sqrt (ps->MaxSize / ps->Aspect) * 0.5f;

	for (i = 0; i < ps->Num; i++)
	{
		XGVECTOR3 pos, vel;
		float r1, r2, r3;
		float size, life, age, t;
		uint32 color;
		
		if ((PARTICLE_VertCount + particles_added) == PARTICLE_Max)
			break;
		
		float tslice = (float) fabs (ps->Dura / ps->Num);
		
		// Life
		r1 = inv_r * Rand.rand();
		life = ps->Rlife * r1 + ps->Life;
		age = (float) fmod (ps->Ctime + (i * tslice), life);
		t = age / life;
		
		// Color
		color = 0;
		r1 = inv_r * Rand.rand();
		
		for (int32 i = 0; i < 4; i++)
		{
			int shift = i * 8;
			int s = (ps->Scol & (0xff << shift)) >> shift;
			int e = (ps->Ecol & (0xff << shift)) >> shift;
			int r = ((ps->Rcol & (0xff << shift)) >> shift) * r1;

			s += r;
			if (s > 0xff)
				s = 0xff;

			e += r;
			if (e > 0xff)
				e = 0xff;

			int a = (s + ((int) ((e - s) * t)));

			color += a << shift;
		}

		// If the particle is too transparent it will only waste fill rate
		const int particle_min_alpha = 0x10;
		if (((color & 0xff000000) >> 24) < particle_min_alpha)
			continue;

		// Velocity
		r1 = inv_r * Rand.rand();
		r2 = inv_r * Rand.rand();
		r3 = inv_r * Rand.rand();
		vel.x = ps->Svel[0] + r1 * ps->Rvel1[0] + r2 * ps->Rvel2[0] + r3 * ps->Rvel3[0] + ps->Force[0] * age;
		vel.y = ps->Svel[1] + r1 * ps->Rvel1[1] + r2 * ps->Rvel2[1] + r3 * ps->Rvel3[1] + ps->Force[1] * age;
		vel.z = ps->Svel[2] + r1 * ps->Rvel1[2] + r2 * ps->Rvel2[2] + r3 * ps->Rvel3[2] + ps->Force[2] * age;
		
		// Position
		r1 = inv_r * Rand.rand();
		r2 = inv_r * Rand.rand();
		pos.x = ps->Spos[0] + r1 * ps->Rpos1[0] + r2 * ps->Rpos2[0] + vel.x * age;
		pos.y = ps->Spos[1] + r1 * ps->Rpos1[1] + r2 * ps->Rpos2[1] + vel.y * age;
		pos.z = ps->Spos[2] + r1 * ps->Rpos1[2] + r2 * ps->Rpos2[2] + vel.z * age;
		
		// Size
		r1 = inv_r * Rand.rand();
		size = (ps->Ssize + (ps->Esize - ps->Ssize) * t + ps->Rsize * r1) * 0.5f;
		
		// UV Rotation
		//		int rot = (int) ((2.0f * 3.1415f * inv_r * Rand.rand() + age * ps->RotVel) / (2.0f * 3.1415f) * sin_num);
		int rot = Rand.rand() % sin_num;
		
		ParticleVertex* Vert = ((ParticleVertex*)PARTICLE_CurWork) + (PARTICLE_VertCount + particles_added) * 4;
		
		Vert[0].Pos = pos;
		Vert[0].Diffuse = color;
		Vert[0].UV = XGVECTOR2(0, 0);
		Vert[0].Scale = XGVECTOR2(-size * ps->Aspect, -size);
		
		Vert[1].Pos = pos;
		Vert[1].Diffuse = color;
		Vert[1].UV = XGVECTOR2(1, 0);
		Vert[1].Scale = XGVECTOR2(size * ps->Aspect, -size);
		
		Vert[2].Pos = pos;
		Vert[2].Diffuse = color;
		Vert[2].UV = XGVECTOR2(1, 1);
		Vert[2].Scale = XGVECTOR2(size * ps->Aspect, size);
		
		Vert[3].Pos = pos;
		Vert[3].Diffuse = color;
		Vert[3].UV = XGVECTOR2(0, 1);
		Vert[3].Scale = XGVECTOR2(-size * ps->Aspect, size);
		
		// Rotate the UVs
		for (int k = 0; k < 4; k++)
		{
			float x, y;
			
			Vert[k].UV[0] -= 0.5f;
			Vert[k].UV[1] -= 0.5f;
			
			// x' = x cos - y sin
			// y' = y cos + x sin
			x = Vert[k].UV[0] * sin_table[(rot+sin_num/4)%sin_num] - Vert[k].UV[1] * sin_table[rot];
			y = Vert[k].UV[1] * sin_table[(rot+sin_num/4)%sin_num] + Vert[k].UV[0] * sin_table[rot];
			
			Vert[k].UV[0] = x + 0.5f;
			Vert[k].UV[1] = y + 0.5f;
		}

		particles_added++;
	}
	
	if (particles_added)
	{
		// Pass the matrices to the vertex shader.
		XGMATRIX LocalToView, ViewToScreen;

		XGMatrixTranspose(&LocalToView, (XGMATRIX*)&nglCurScene->WorldToView);
		nglDev->SetVertexShaderConstant(nglVSC_LocalToScreen, &LocalToView, 4);

		XGMatrixTranspose(&ViewToScreen, (XGMATRIX*)&nglCurScene->ViewToScreen);
		nglDev->SetVertexShaderConstant(PARTICLE_VSC_ViewToScreen, &ViewToScreen, 4);

		XGVECTOR4 MaxScreenSize (MaxSize, -MaxSize, 0, 0);
		nglDev->SetVertexShaderConstant(PARTICLE_VSC_MaxScreenSize, &MaxScreenSize, 1);

		// Set shaders
		nglSetVertexShader(PARTICLE_VS.Handle);
		nglSetPixelShader(PARTICLE_PS.Handle);

		// Set texture
		nglBindTexture(ps->Tex, ps->MaterialFlags, 0);
		nglSetBlendingMode(ps->BlendMode, 0x20);
//		nglSetMapAddressMode(NGLMAT_CLAMP_U | NGLMAT_CLAMP_V, 0);
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		nglDev->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		nglDev->SetTextureStageState(0, D3DTSS_BORDERCOLOR, D3DCOLOR_RGBA(0,0,0,0));
		nglSetCullingMode(0, 0);

		// Render particles
		nglBindVertexBuffer(PARTICLE_CurVertBuf, sizeof (ParticleVertex));
		nglDev->DrawPrimitive(D3DPT_QUADLIST, PARTICLE_VertCount * 4, particles_added);
		
		PARTICLE_VertCount = PARTICLE_VertCount + particles_added;
	}
}

/*-----------------------------------------------------------------------------
Description: Add a particle system to the render list.
-----------------------------------------------------------------------------*/
void PARTICLE_ListAdd(ParticleSystem* Particle)
{
	// Create the particle node.
	ParticleSystem* ParticleNode = nglListNew(ParticleSystem);
	if (!ParticleNode)
		return;
	
	memcpy(ParticleNode, Particle, sizeof(ParticleSystem));
	
	nglSortInfo Info;
	Info.Type = NGLSORT_TRANSLUCENT;
	
	nglVector Center;
	nglApplyMatrix(Center, nglCurScene->WorldToView, Particle->Spos);
	
	Info.Dist = Center[2];
	nglListAddCustomNode(PARTICLE_RenderSystem, ParticleNode, &Info);
}

/*-----------------------------------------------------------------------------
Description: Initialize the particle system (create the VBs, create the
vertex shader, etc.).
-----------------------------------------------------------------------------*/
void PARTICLE_Init()
{
	int32 reg = NGL_VS_FIRST_USER_REG;	// Call must come after nglInitShadersConst()

	// Initialize Vertex Shader register assignments
	PARTICLE_VSC_ViewToScreen = reg; reg += 4;
	PARTICLE_VSC_MaxScreenSize = reg; reg++;

	for (int32 i = 0; i < PARTICLE_VB_COUNT; i++)
		NGL_D3DTRY (nglDev->CreateVertexBuffer (PARTICLE_MAX_VERTS * sizeof (ParticleVertex),
		D3DUSAGE_WRITEONLY, (D3DFVF_XYZ|D3DFVF_DIFFUSE), D3DPOOL_DEFAULT, &PARTICLE_VertBuf[i]));
	
	PARTICLE_LockVertexBuffer();
	
	// Store the final VS source code.
	char PARTICLE_VSSrc[4096];

	// Build the vertex shader source code.
	PARTICLE_VSSrc[0] = 0;
	
	sprintf(PARTICLE_VSSrc,
	
	// begin
//	"xvs.1.1\n"	// Done in nglCreateVertexShader (dc 05/30/02)
//	"#pragma screenspace\n"
	
	// transform from world to view
	"dp4 r3.x, v0, c[%d]\n"
	"dp4 r3.y, v0, c[%d]\n"
	"dp4 r3.z, v0, c[%d]\n"
	"dp4 r3.w, v0, c[%d]\n"
	
	// transform without size
	"dp4 r4.x, r3, c[%d]\n"
	"dp4 r4.y, r3, c[%d]\n"
	
	// add size
	"add r3.xy, r3, v3\n"
	
	// transform from view to screen
	"dp4 oPos.x, r3, c[%d]\n"
	"dp4 oPos.y, r3, c[%d]\n"
	"dp4 oPos.z, r3, c[%d]\n"
	"dp4 oPos.w, r3, c[%d]\n"
	
	// map
	";---- vsf_map\n"
	"mov oT0.xy, v2\n"
	
	// no fog
	";---- vsf_nofog\n"
	"mov oFog, %s\n"
	
	// color
	";---- vsf_color_notint\n"
	"mov oD0, v1\n"
	
	// Scale.
	";---- vsf_end\n"
	"mul oPos.xyz, r12, c[%d]\n"
	// Compute 1/w.
	"+ rcc r1.x, r12.w\n"
	// Scale by 1/w, add offset.
	"mad oPos.xyz, r12, r1.x, c[%d]\n"
	
	// fill rate limit
	"mul r4.xy, r4, c[%d]\n"
	"+ rcc r1.x, r12.w\n"
	"mad r4.xy, r4, r1.x, c[%d]\n"
	"sub r3.xy, r12, r4\n"
	"min r3.x, r3.x, c[%d].x\n"
	"max r3.x, r3.x, c[%d].y\n"
	"min r3.y, r3.y, c[%d].x\n"
	"max r3.y, r3.y, c[%d].y\n"
	"add oPos.xy, r4, r3\n",
	
	nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1, nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3,
	PARTICLE_VSC_ViewToScreen + 0, PARTICLE_VSC_ViewToScreen + 1, 
	PARTICLE_VSC_ViewToScreen + 0, PARTICLE_VSC_ViewToScreen + 1, PARTICLE_VSC_ViewToScreen + 2, PARTICLE_VSC_ViewToScreen + 3,
	NGL_VSC_ONE,
	nglVSC_Scale,
	nglVSC_Offset,
	nglVSC_Scale,
	nglVSC_Offset,
	PARTICLE_VSC_MaxScreenSize, PARTICLE_VSC_MaxScreenSize, PARTICLE_VSC_MaxScreenSize, PARTICLE_VSC_MaxScreenSize
	);
	
	nglCreateVertexShader(PARTICLE_VS_Decl, &PARTICLE_VS, false, PARTICLE_VSSrc);

	nglCreatePixelShader(&PARTICLE_PS,
		"tex t0\n"
		"mul r0, t0, v0\n"	// modulate by vertex color
		"xfc 1-zero, r0, zero, zero, zero, zero, r0.a"
	);
}

void PARTICLE_Cleanup()
{
	for (int32 i = 0; i < PARTICLE_VB_COUNT; i++)
		PARTICLE_VertBuf[i]->Release();
	
	nglReleaseShader(&PARTICLE_VS);
	nglReleaseShader(&PARTICLE_PS);
}

#endif	// #ifdef TARGET_XBOX