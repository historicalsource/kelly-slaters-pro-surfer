
// ksngl_wave_gc.cpp - included from ksngl.cpp

#include "ngl_gc_internal.h"
#if NGL_GC_PROFILE
#include "ngl_gc_profile_internal.h"
#endif
#include "ksngl.h"
#include "wavetex.h"

//----------------------------------------------------------------------------------------
//  @Particle system code (custom node).
//----------------------------------------------------------------------------------------
static float particle_tex_coords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
void nglRenderParticleCallback( void* Data )
{
  nglParticleSystem *ps = ( nglParticleSystem *) Data;
	const float inv_r = 1.0f / RAND_MAX;
	uint16 particle;
  static float const_alpha_scale = 1.0f;

  srand( ps->Seed );

	if( ps->Num == 0 )
	{
		//Should we assert?
		return;
	}

	#warning "This be broke -eo"
	#if 0
  if ( DEBUG_ENABLE( ShowPerfInfo ) )
  {
    NGL_PAUSE_MMCR0;
    NGL_PAUSE_MMCR1;
    NGL_RESET_MMCR0;
    NGL_RESET_MMCR1;
    NGL_START_MMCR0;
    NGL_START_MMCR1;
  }
	#endif

#ifdef MARTINS_VIEWPORT_FOO
	f32 vp_old[GX_VIEWPORT_SZ];
  f32 vp_new[GX_VIEWPORT_SZ];
  GXGetViewportv( vp_old );
  memcpy( vp_new, vp_old, sizeof( f32 ) * GX_VIEWPORT_SZ );
  vp_new[5] -= 0.05f;
  GXSetViewportv( vp_new );
#endif //MARTINS_VIEWPORT_FOO

  float tslice = (float) fabs( ps->Dura / ps->Num );

	// Set up GX
	GXSetNumChans( 1 );
	GXSetNumTexGens( 1 );
	GXSetNumTevStages( 1 );
	GXSetZMode( GX_TRUE, GX_LEQUAL, GX_FALSE );
  //GXSetZCompLoc( GX_FALSE );
  GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0 );
	//nglGCTempVCInFirstTEVStage = true;
	nglSetTevBlend( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0 );

	nglTexture *tex;
	if( ps->Tex == NULL )
		tex = &nglWhiteTex;
	else
		tex = ps->Tex;

	assert( tex->Type == 0 );
	if( ps->Scol != ps->Ecol )
	{
		GXLoadTlut( &tex->TlutObj, 0 );
	}
	if( tex->PaletteSize != 0 )
	{
		GXLoadTlut( &tex->TlutObj, 0 );
		GXInitTexObjTlut( &tex->TexObj, 0 );
	}
	GXLoadTexObj( &tex->TexObj, GX_TEXMAP0 );

	// Set vertex attributes
	GXClearVtxDesc();
	GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
	GXSetVtxDesc( GX_VA_CLR0, GX_DIRECT );
	GXSetVtxDesc( GX_VA_TEX0, GX_INDEX8 );

	// FIXME: Should this be in nglListInit? or does ngl need a per-frame custom
	// node configurator?
	GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
	GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 );
	GXSetArray( GX_VA_TEX0, particle_tex_coords, sizeof( float ) * 2 );
	GXSetCullMode( GX_CULL_NONE );
	GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
	GXSetChanCtrl( GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );
	GXSetChanCtrl( GX_ALPHA1, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );

	// Set identity view matrix (negative z)
	Mtx neg_z_ident;
	MTXIdentity( neg_z_ident );
	MTXRowCol( neg_z_ident, 2, 2 ) = -1.0f;
	GXLoadPosMtxImm( neg_z_ident, GX_PNMTX0 );

	nglMatrix ViewToScreen;
  nglTransposeMatrix( ViewToScreen, nglCurScene->ViewToScreen );

  //Begin rendering
	for( particle = 0; particle < ps->Num; particle++ )
	{
		nglVector pos, vel, view_pos;
		float r1, r2, r3;
		float size, life, age, t;
		uint8 color[4];

		// life
		r1 = inv_r * rand();
		life = ps->Rlife * r1 + ps->Life;
		age = (float) fmod( ps->Ctime + ( particle * tslice ), life );
		t = age / life;

		//color
		int shift = 0;
		for( int i = 0; i < 4; i++ )
		{
			int s = ( ps->Scol & ( 0xff << shift ) ) >> shift;
			int e = ( ps->Ecol & ( 0xff << shift ) ) >> shift;

			int a = ( s + ( (int) ( ( e - s ) * t ) ) );

			color[i] = a;

			shift += 8;
		}

		//velocity
		r1 = inv_r * rand();
		r2 = inv_r * rand();
		r3 = inv_r * rand();
		vel[0] = ps->Svel[0] + r1 * ps->Rvel1[0] + r2 * ps->Rvel2[0] + r2 * ps->Rvel3[0] + ps->Force[0] * age;
		vel[1] = ps->Svel[1] + r1 * ps->Rvel1[1] + r2 * ps->Rvel2[1] + r2 * ps->Rvel3[1] + ps->Force[1] * age;
		vel[2] = ps->Svel[2] + r1 * ps->Rvel1[2] + r2 * ps->Rvel2[2] + r2 * ps->Rvel3[2] + ps->Force[2] * age;
		vel[3] = 1.0f;

		//position
		r1 = inv_r * rand();
		r2 = inv_r * rand();
		pos[0] = ps->Spos[0] + r1 * ps->Rpos1[0] + r2 * ps->Rpos2[0] + vel[0] * age;
		pos[1] = ps->Spos[1] + r1 * ps->Rpos1[1] + r2 * ps->Rpos2[1] + vel[1] * age;
		pos[2] = ps->Spos[2] + r1 * ps->Rpos1[2] + r2 * ps->Rpos2[2] + vel[2] * age;
		pos[3] = 1.0f;

		nglApplyMatrix( view_pos, nglCurScene->WorldToView, pos );

		//view_pos[2] -= 1.0f;

    //size
		r1 = inv_r * rand();
		size = ( ps->Ssize + ( ps->Esize - ps->Ssize ) * t + ps->Rsize * r1 ) * 0.5f;

		size *= ps->Aspect;

/*    nglVector p1, p2, pin;
    nglApplyMatrix( p1, ViewToScreen, nglVector( view_pos[0] - size, view_pos[1] - size, view_pos[2], 1.0f ) );
    nglApplyMatrix( p2, ViewToScreen, nglVector( view_pos[0] + size, view_pos[1] + size, view_pos[2], 1.0f ) );
    float InvW = 1.0f / p1[3];
    nglScaleVector( p1, p1, InvW );
    nglScaleVector( p2, p2, InvW );

		float xscale, yscale;

		xscale = 1.0f / fabs( p1[0] - p2[0] );
		yscale = 1.0f / fabs( p1[1] - p2[0] );

		if( xscale < 1.0f || yscale < 1.0f )
		{
			if( xscale < yscale )
				size *= xscale;
			else
				size *= yscale;
		}
	*/
		// I'm using immediate rendering at the moment mainly so I don't have to
		// allocate memory from ngl to build up display lists.
		// Might need to be changed to display lists if the number of particles
		// increase
	  GXBegin( GX_QUADS, GX_VTXFMT7, 4 );
		GXPosition3f32( view_pos[0] - size, view_pos[1] - size, view_pos[2] );
		GXColor4u8( color[3], color[2], color[1], color[0] );
		GXTexCoord1x8( 0 );
		GXPosition3f32( view_pos[0] + size, view_pos[1] - size, view_pos[2] );
		GXColor4u8( color[3], color[2], color[1], color[0] );
		GXTexCoord1x8( 1 );
		GXPosition3f32( view_pos[0] + size, view_pos[1] + size, view_pos[2] );
		GXColor4u8( color[3], color[2], color[1], color[0] );
		GXTexCoord1x8( 2 );
		GXPosition3f32( view_pos[0] - size, view_pos[1] + size, view_pos[2] );
		GXColor4u8( color[3], color[2], color[1], color[0] );
		GXTexCoord1x8( 3 );
	  GXEnd();
	}

  //GXSetViewportv( vp_old );


#warning "This be broke"
#if 0
  if ( DEBUG_ENABLE( ShowPerfInfo ) )
  {
    NGL_PAUSE_MMCR0;
    NGL_PAUSE_MMCR1;

    nglPerfInfo.ParticleRenderCycles += NGL_GET_CYCLES();
    nglPerfInfo.TotalParticles += ps->Num;
  }
#endif
  GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0 );
}

void nglListAddParticle( nglParticleSystem* Particle )
{
  // Create the particle node.
  nglParticleSystem* ParticleNode = nglListNew( nglParticleSystem );

	if( !ParticleNode )
	{
#ifdef NGL_VERBOSE
		nglPrintf( "nglListAddParticle: could not allocate nglParticleSystem.\n" );
#endif
		return;
	}

  memcpy( ParticleNode, Particle, sizeof( nglParticleSystem ) );

  nglSortInfo Info;
  Info.Type = NGLSORT_TRANSLUCENT;

  nglVector Center;
  nglApplyMatrix( Center, nglCurScene->WorldToView, Particle->Spos );
  Info.Dist = Center[2];

  nglListAddNode( NGLNODE_PARTICLE, nglRenderParticleCallback, ParticleNode, &Info );
}




//#define USE_STD_SCRATCH_MESH_CODE


static NGL_INLINE void nglWaveFastWriteVertexPNCAUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, u_int Alpha, float U, float V )
{
  	register nglVertex* const Vert = ((nglVertex*)nglScratchSection->Verts) + nglScratchVertIdx;
	float u,v;

	Vert->x = X;
	Vert->y = Y;
	Vert->z = Z;

	Vert->color = Color;
	Vert->Pad1  = Alpha;

	u = U * NGLGC_SCALE_UV;
	v = V * NGLGC_SCALE_UV;

	OSf32tos16(&u, &Vert->u1);
	OSf32tos16(&v, &Vert->v1);

	Vert->nx = (short)(NX * NGLGC_SCALE_NRM);
  	Vert->ny = (short)(NY * NGLGC_SCALE_NRM);
  	Vert->nz = (short)(NZ * NGLGC_SCALE_NRM);

  	nglScratchVertIdx++;
}

void nglWaveWriteVertexPNCAUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, u_int Alpha, float U, float V )
{
	nglWaveFastWriteVertexPNCAUV( X, Y, Z, NX, NY, NZ, Color, Alpha, U, V );
}

float WaveBumpNormalScale=0.8f;

#ifndef USE_STD_SCRATCH_MESH_CODE




static void nglRenderWaveCallback( void* Data );

static void nglWaveBlendDecal( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	GXTevColorArg CArg,CArg2;
	GXTevAlphaArg AArg;
	int AlternateAlphaInputs=false;

	if( !NGLGC_BASE_STAGE_CHECK(stage) )
	{
		CArg = GX_CC_CPREV;
		AArg = GX_CA_APREV;
		if( constant == 1.0f )
		{
			CArg2 = GX_CC_TEXA;
		}
		else
		{
			CArg2 = GX_CC_KONST ;
			u_int RangeAdjustedConstant = (u_int) ( 255.0f * constant );
			// <<<< 20-Feb-2002 SAL: Weird hardware bug? The Konst Alpha set here is affecting the Punchthrough test, though we don't select it anywhere. Test it later on.
			//GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant };
			GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, 0xFF };
			GXSetTevKColor( GX_KCOLOR0, color );
			GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
			GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
		}
	}
	else
	{
		CArg = GX_CC_ZERO;
		CArg2 = GX_CC_ONE;
		if( constant == 1.0f )
		{
			AArg = nglGCTempSrcAlpha;
		}
		else
		{
			AArg = GX_CA_KONST;
		}
		u_int RangeAdjustedConstant = (u_int) ( 255.0f * constant );
		GXColor color = { 0xFF, 0xFF, 0xFF, RangeAdjustedConstant };
		#ifdef BIGUGLYNGLGCMERGE
		if(0) //nglGCTempMC)
		{
			AArg = GX_CA_KONST;
			CArg2 = GX_CC_KONST;
		 	color.r = (u8) ( nglGCTempMaterialColor[0] * 255.0f );
		 	color.g = (u8) ( nglGCTempMaterialColor[1] * 255.0f );
		 	color.b = (u8) ( nglGCTempMaterialColor[2] * 255.0f );
 			if( constant != 1.0f )
 			{
			 	color.a = (u8) ( nglGCTempMaterialColor[3] * 255.0f );
			}
			else
			 	AlternateAlphaInputs=true;
		}
		#endif
		GXSetTevKColor( GX_KCOLOR0, color );
		GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
		GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	}

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
									 CArg,
									 nglGCTempSrcClr,
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
	if( AlternateAlphaInputs )
		GXSetTevAlphaIn( stage,
										 GX_CA_ZERO,
										 AArg,
										 nglGCTempSrcAlpha,
										 GX_CA_ZERO );
	else
		GXSetTevAlphaIn( stage,
										 AArg,
										 GX_CA_ZERO,
										 GX_CA_ZERO,
										 GX_CA_ZERO );
}

nglVertex *nglWaveEnvVerts;

static NGL_INLINE void nglGenEnvWaveCoords( u_short PIdx, u_short NIdx=-1 );

static NGL_INLINE void nglGenEnvWaveCoords( u_short PIdx, u_short NIdx )
{
#if NGL_GC_PROFILE
	nglGCPD.EnviroMapCoord.Start();
#endif //#if NGL_GC_PROFILE
	// <<<< 27-Sep-2001 SAL: This assumes that the position and normal info is in the same place for both
	// nglVertex and nglSkinVertex. If we don't implement this code in the GPU then revisit the issue.
	// Also we are assuming that size of nglVertex and nglSkinVertex will always be a multiple of 4.
	//nglVertP* Vert = &nglEnvMapVertP[PIdx];
	//nglVertN* VertN = &nglEnvMapVertN[NIdx];
	nglVertex *Vert=&nglWaveEnvVerts[PIdx];

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
	TNrm2[0] = Vert->nx * NGLGC_UNSCALE_NRM;
	TNrm2[1] = Vert->ny * NGLGC_UNSCALE_NRM;
	TNrm2[2] = Vert->nz * NGLGC_UNSCALE_NRM;

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

//	nglNormalize(Vec1,Vec1);

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
//	nglSpecialNormalize(Refl,Refl);

#if USE_NBT_FOR_ENVMAP
	GXNormal3f32( TNrm2[0], TNrm2[1], TNrm2[2] );
	GXNormal3f32( Refl[0], Refl[1], Refl[2] );
	GXNormal3f32( 0.0f, 0.0f, 0.0f );
#else
	float m,FVal1,FVal2;

//	FVal1=Refl[0]*Refl[0]+Refl[1]*Refl[1]+(Refl[2]+1)*(Refl[2]+1);
	FVal1=Refl[0]*Refl[0]+Refl[1]*Refl[1]+Refl[2]*Refl[2];
	m=2*sqrtf(FVal1);

	if(m<0.00005f)
		m=0.00005f;

	FVal1=Refl[0]/m + 0.5f;
	FVal2=Refl[1]/m + 0.5f;
	FVal2 = 1.0f - FVal2; // 24-Jan-2002 SAL: Flip the y to match our textures.

	GXTexCoord2f32( FVal1, FVal2 );
#endif //#if USE_NBT_FOR_ENVMAP
#if NGL_GC_PROFILE
	nglGCPD.EnviroMapCoord.Add();
#endif //#if NGL_GC_PROFILE
}

static NGL_INLINE void nglGenNBTCoords( u_short PIdx, u_short NIdx=-1 );

static NGL_INLINE void nglGenNBTCoords( u_short PIdx, u_short NIdx )
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

//	nglNormalize(Vec1,Vec1);

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
//	nglSpecialNormalize(Refl,Refl);

#if 1 //USE_NBT_FOR_ENVMAP
	GXNormal3f32( TNrm2[0], TNrm2[1], TNrm2[2] );
	GXNormal3f32( Refl[0], Refl[1], Refl[2] );
	GXNormal3f32( 0.0f, 0.0f, 0.0f );
#else
	float m,FVal1,FVal2;

//	FVal1=Refl[0]*Refl[0]+Refl[1]*Refl[1]+(Refl[2]+1)*(Refl[2]+1);
	FVal1=Refl[0]*Refl[0]+Refl[1]*Refl[1]+Refl[2]*Refl[2];
	m=2*sqrtf(FVal1);

	if(m<0.00005f)
		m=0.00005f;

	FVal1=Refl[0]/m + 0.5f;
	FVal2=Refl[1]/m + 0.5f;
	FVal2 = 1.0f - FVal2; // 24-Jan-2002 SAL: Flip the y to match our textures.

	GXTexCoord2f32( FVal1, FVal2 );
#endif //#if USE_NBT_FOR_ENVMAP
#if NGL_GC_PROFILE
	nglGCPD.EnviroMapCoord.Add();
#endif //#if NGL_GC_PROFILE
}

static NGL_INLINE void nglGenNBTCoord( nglVertex *nvert )
{
	// <<<< 27-Sep-2001 SAL: This assumes that the position and normal info is in the same place for both
	// nglVertex and nglSkinVertex. If we don't implement this code in the GPU then revisit the issue.
	// Also we are assuming that size of nglVertex and nglSkinVertex will always be a multiple of 4.
	nglVertP tvert;
	nglVertN tvern;
	tvert.x=nvert->x;
	tvert.y=nvert->y;
	tvert.z=nvert->z;
	tvern.nx=nvert->nx;
	tvern.ny=nvert->ny;
	tvern.nz=nvert->nz;

	nglVertP* Vert = &tvert; //nglEnvMapVertP[0];
	nglVertN* VertN = &tvern; //nglEnvMapVertN[0];
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

//	nglNormalize(Vec1,Vec1);

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
//	nglSpecialNormalize(Refl,Refl);

	GXNormal3f32( tvern.nx, tvern.ny, tvern.nz); // TNrm2[0], TNrm2[1], TNrm2[2] );
	GXNormal3f32( 0.0f, 1.0f, 0.0f ); //Refl[0], Refl[1], Refl[2] );
	GXNormal3f32( 0.0f, 0.0f, 1.0f );
}

#define DIRECT_NBT


//#define BASEWATEROFF


bool dothebump=false;

uint8 waverenderr=128;
uint8 waverenderg=128;
uint8 waverenderb=128;
uint8 waverendera=128;

#if 0
static void nglSetupWaveLighting( nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params )
{
	int NumLights=0;
	u32 LightsMask=GX_LIGHT_NULL;
	u32 ProjLightsMask=GX_LIGHT_NULL;
  // Determine what lighting context (list of lights) the mesh uses.
  nglCurLightContext = (nglLightContext*)nglDefaultLightContext;
  if ( Params->Flags & NGLP_LIGHT_CONTEXT )
    nglCurLightContext = (nglLightContext*)Params->LightContext;
  // There's a bug here involving adding the light to the local lights more than once if it matches on
  // more than one lighting category.
	if( nglGCDoShadowMap )
	{
		// Use the first light for the alpha computation for the ProjectorLight.
    nglDirProjectorLightInfo* Light = TheProjLight;
		GXLightObj *TheLightObj=&nglLights[NumLights];
		ProjLightsMask = ProjLightsMask | nglGXLight[NumLights];
		GXColor Color;
		Color.r = 255;
		Color.g = 255;
		Color.b = 255;
		Color.a = 190; // Dim the shadows just a bit.
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
//		if(TempMatTest==1)
//		GXInitLightPos( TheLightObj, Light->Zaxis[0] * ScaleInfinity,
//			Light->Zaxis[1] * ScaleInfinity, Light->Zaxis[2] * ScaleInfinity );
		GXInitLightDir( TheLightObj, 1.0f, 0.0f, 0.0f ); // Not useful, but didn't want to leave it unitialized.
		GXInitLightAttn( TheLightObj, 1.0f, 0, 0, 1.0f, 0, 0 );
//		if(!(TempShadowLight==1))
//		{
//		GXLoadLightObjImm( TheLightObj, nglLightIDs[NumLights] );
//		NumLights++;
//		}

		if(TempShadowLight)
		{
			TheLightObj=&nglLights[NumLights];
			ProjLightsMask = ProjLightsMask | nglLightIDs[NumLights];
			nglVector LightViewPos,LightWorldPos;
		  //nglApplyMatrix(LightPos, nglCurScene->WorldToView, Light->Pos);
		  //nglMatrix ViewToWorld;
		  //nglTransposeMatrix( ViewToWorld, nglCurScene->WorldToView );
		  nglCopyVector( LightWorldPos, Light->Pos );
		  LightWorldPos[3] = 1.0f;
			nglApplyMatrix(LightViewPos, nglCurScene->WorldToView, LightWorldPos);
			if(fabs(LightViewPos[3])<0.00001f)
				LightViewPos[3]=0.00001f;
			LightViewPos[0]/=LightViewPos[3];
			LightViewPos[1]/=LightViewPos[3];
			LightViewPos[2]/=LightViewPos[3];
			LightViewPos[2]=-LightViewPos[2];

			Color.a = 255; // No reason to dim the cutoff light.
			GXInitLightColor( TheLightObj, Color );
			GXInitLightPos( TheLightObj, LightViewPos[0], LightViewPos[1], LightViewPos[2] );
			//GXInitLightPos( TheLightObj, Light->Pos[0], Light->Pos[1], Light->Pos[2] );
			GXInitLightAttn( TheLightObj, ConstA[0], ConstA[1], ConstA[2], ConstK[0], ConstK[1], ConstK[2] );
			GXInitLightDir( TheLightObj, Light->Zaxis[0], Light->Zaxis[1], Light->Zaxis[2] ); // Not useful, but didn't want to leave it unitialized.
			GXLoadLightObjImm( TheLightObj, nglLightIDs[NumLights] );
			NumLights++;
		}
	}

	#if 0
  if((Section->Material->Flags & NGLMAT_LIGHT) && ( Mesh->Flags & NGLMESH_SKINNED )
  	&& (!( Params->Flags & NGLP_NO_LIGHTING)))
  {

	  for (u_int i = 0; i < NGL_NUM_LIGHTCATS; i++)
	  {
	    if (!(Mesh->Flags & (1 << (NGL_LIGHTCAT_SHIFT + i))))
	      continue;
			nglLightNode* LightNode = nglCurLightContext->Head.Next[i]; // LocalNext is not used for the GC.
	    while((LightNode != &nglCurLightContext->Head) && (NumLights < 8))
		  {
	      if((LightNode->Type == NGLLIGHT_FAKEPOINT) || (LightNode->Type == NGLLIGHT_TRUEPOINT))
		    {
	        nglPointLightInfo* Light = (nglPointLightInfo*)LightNode->Data;
		      nglVector LightPos;

		      // Transform light to local space.
		      nglApplyMatrix( LightPos, nglMeshWorldToLocal, Light->Pos );

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
					LightsMask = LightsMask | nglLightIDs[NumLights];
					GXColor Color;
					Color.r = (u8) ( TheColor[0] * 255.0f );
					Color.g = (u8) ( TheColor[1] * 255.0f );
					Color.b = (u8) ( TheColor[2] * 255.0f );
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
					GXLoadLightObjImm( TheLightObj, nglLightIDs[NumLights] );
					NumLights++;
				}
				else if ( LightNode->Type == NGLLIGHT_DIRECTIONAL )
				{
	        nglDirLightInfo* Light = (nglDirLightInfo*)LightNode->Data;
					GXLightObj *TheLightObj=&nglLights[NumLights];
					LightsMask = LightsMask | nglLightIDs[NumLights];
					GXColor Color;
					Color.r = (u8) ( Light->Color[0] * 255.0f );
					Color.g = (u8) ( Light->Color[1] * 255.0f );
					Color.b = (u8) ( Light->Color[2] * 255.0f );
					Color.a = (u8) ( Light->Color[3] * 255.0f );
					GXInitLightColor( TheLightObj, Color );
					// GXInitLightAttn( TheLightObj, 1.0f, 0, 0, 1.0f, 0, 0 );
					float ScaleInfinity = -1024 *1024;
					GXInitLightPos( TheLightObj, Light->Dir[0] * ScaleInfinity,
						Light->Dir[1] * ScaleInfinity, Light->Dir[2] * ScaleInfinity );
					GXLoadLightObjImm( TheLightObj, nglLightIDs[NumLights] );
					NumLights++;
		  	}
	      LightNode = LightNode->Next[i];
			}
	  }
	}

	// Setup the tinting as a light.
	if( Params->Flags & NGLP_TINT )
	{
		nglVector TintColor;
		nglCopyVector( TintColor, Params->TintColor );
		GXColor Color;
		Color.r = (u8) ( TintColor[0] * 255.0f );
		Color.g = (u8) ( TintColor[1] * 255.0f );
		Color.b = (u8) ( TintColor[2] * 255.0f );
		Color.a = (u8) ( TintColor[3] * 255.0f );
		GXSetChanAmbColor( GX_COLOR0A0, Color );
		GXSetChanCtrl( GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, LightsMask,
			GX_DF_CLAMP, GX_AF_NONE );
		#if 0
		if( nglGCUseVCFix )
		{
			Color.r = (u8) ( TintColor[2] * 255.0f );
			Color.g = (u8) ( TintColor[3] * 255.0f );
			Color.b = (u8) ( TintColor[0] * 255.0f );
			Color.a = (u8) ( TintColor[1] * 255.0f );
			GXSetChanAmbColor( GX_COLOR1, Color );
			GXSetChanCtrl( GX_COLOR1, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, LightsMask,
				GX_DF_CLAMP, GX_AF_NONE );
		}
		#endif
	}
	else
	#endif
	{
		if( LightsMask != GX_LIGHT_NULL )
		{
			// No ambient color.
			GXColor Color;
			Color.r = Color.g =Color.b = Color.a = 0;
			GXSetChanAmbColor( GX_COLOR0A0, Color );
			GXSetChanCtrl( GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, LightsMask,
				GX_DF_CLAMP, GX_AF_NONE );
			#if 0
			if( nglGCUseVCFix )
			{
				GXSetChanAmbColor( GX_COLOR1A1, Color );
				GXSetChanCtrl( GX_COLOR1, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, LightsMask,
					GX_DF_CLAMP, GX_AF_NONE );
			}
			#endif
		}
		else
		{
			nglMatrix IdentMat;
			nglIdentityMatrix( IdentMat );
//			GXLoadNrmMtxImm( IdentMat, GX_PNMTX0 );

			// No lighting or tinting.
			GXSetChanCtrl( GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
				GX_DF_NONE, GX_AF_NONE );
			#if 0
			if( nglGCUseVCFix )
			{
				GXSetChanCtrl( GX_COLOR1, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
					GX_DF_NONE, GX_AF_NONE );
			}
			#endif
		}
	}
	#if 0
	if( nglGCDoShadowMap )
	{
		// No ambient color.
		GXColor Color;
		Color.r = Color.g =Color.b = Color.a = 0;
		GXSetChanAmbColor( GX_ALPHA1, Color );
		Color.r = Color.g =Color.b = Color.a = 255;
		GXSetChanMatColor( GX_ALPHA1, Color );
		if( TempShadowLight2==2 )
			GXSetChanCtrl( GX_ALPHA1, GX_TRUE, GX_SRC_REG, GX_SRC_REG, ProjLightsMask,
				GX_DF_NONE, GX_AF_NONE );
		else if( TempShadowLight2==1 )
			GXSetChanCtrl( GX_ALPHA1, GX_TRUE, GX_SRC_REG, GX_SRC_REG, ProjLightsMask,
				GX_DF_SIGN, GX_AF_SPOT );
		else
			GXSetChanCtrl( GX_ALPHA1, GX_TRUE, GX_SRC_REG, GX_SRC_REG, ProjLightsMask,
				GX_DF_CLAMP, GX_AF_SPOT );
	}
	else
	{
		GXSetChanCtrl( GX_ALPHA1, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
			GX_DF_NONE, GX_AF_NONE );
	}
	#endif
}
#endif

static void nglSetupWaveLighting(nglMeshNode* MeshNode, nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params)
{
	int NumLights = 0;
	u32 LightsMask = GX_LIGHT_NULL;
	u32 ProjLightsMask = GX_LIGHT_NULL;
	u32 ParamFlags = Params->Flags;

	// Determine what lighting context (list of lights) the mesh uses.
  	nglCurLightContext = (nglLightContext*)nglDefaultLightContext;
  	if (ParamFlags & NGLP_LIGHT_CONTEXT)
    	nglCurLightContext = (nglLightContext*)Params->LightContext;

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

  	if ((Section->Material->Flags & NGLMAT_LIGHT) && (!(ParamFlags&NGLP_NO_LIGHTING)))
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
					Color.r = (u8) ( TheColor[0] * 255.0f );
					Color.g = (u8) ( TheColor[1] * 255.0f );
					Color.b = (u8) ( TheColor[2] * 255.0f );
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
					GXColor Color;
					Color.r = (u8)(Light->Color[0] * 255.0f);
					Color.g = (u8)(Light->Color[1] * 255.0f);
					Color.b = (u8)(Light->Color[2] * 255.0f);
					Color.a = 0xFF;
					// setup the light for use on the gamecube
					GXLightObj *TheLightObj = &nglLights[NumLights];
					GXInitLightColor( TheLightObj, Color );
					GXInitLightPos( TheLightObj, Light->Dir[0] * INF,
						Light->Dir[1] * INF, Light->Dir[2] * INF);
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
			}
			else
			{
				// if we don't have tint, add it as the ambient
				ParamFlags |= NGLP_TINT;
				Params->TintColor.x = nglCurLightContext->Ambient.x;
				Params->TintColor.y = nglCurLightContext->Ambient.y;
				Params->TintColor.z = nglCurLightContext->Ambient.z;
			}
		}
	}

	// Setup the tinting as a light.
	if (ParamFlags&NGLP_TINT)
	{
		GXColor Color;
		Color.r = (u8)(Params->TintColor.x * 255.0f);
		Color.g = (u8)(Params->TintColor.y * 255.0f);
		Color.b = (u8)(Params->TintColor.z * 255.0f);
		Color.a = 0xFF;
		GXSetChanAmbColor( GX_COLOR0A0, Color );
		GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, LightsMask, GX_DF_CLAMP, GX_AF_NONE);
	}
	else
	{
		// No tinting.
		GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	}

#if 0
	if (nglGCDoShadowMap)
	{
		// No ambient color.
		GXColor Color;
		Color.r = Color.g =Color.b = Color.a = 0;
		GXSetChanAmbColor(GX_COLOR1A1, Color );
		Color.r = Color.g =Color.b = Color.a = 255;
		GXSetChanMatColor(GX_COLOR1A1, Color );
		GXSetChanCtrl(GX_COLOR1A1, GX_TRUE, GX_SRC_REG, GX_SRC_REG, ProjLightsMask, GX_DF_CLAMP, GX_AF_SPOT);
	}
	else
	{
		GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	}
#endif

}

//-----------------------------------------------------------------------------

#if 0
static void nglSetupWaveProjLighting( nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params )
{
	int NumProjLights=0;
  // Determine what lighting context (list of lights) the mesh uses.
  nglCurLightContext = (nglLightContext*)nglDefaultLightContext;
  if ( Params->Flags & NGLP_LIGHT_CONTEXT )
    nglCurLightContext = (nglLightContext*)Params->LightContext;
  // There's a bug here involving adding the light to the local lights more than once if it matches on
  // more than one lighting category.
  nglVector WorldPos;
  nglApplyMatrix( WorldPos, nglMeshLocalToWorld, Mesh->SphereCenter );

  if ( (!(Mesh->Flags & NGLMESH_SKINNED))
      /* && (!( Mesh->Flags & NGLMESH_SCRATCH_MESH )) */ )
  {

	  for (u_int i = 0; i < NGL_NUM_LIGHTCATS; i++)
	  {
	    if (!(Mesh->Flags & (1 << (NGL_LIGHTCAT_SHIFT + i))))
	      continue;
			nglLightNode* LightNode = nglCurLightContext->ProjectorHead.Next[i]; // LocalNext is not used for the GC.
	    while((LightNode != &nglCurLightContext->ProjectorHead) && (NumProjLights < 1))
		  {
				if ( LightNode->Type == NGLLIGHT_PROJECTED_DIRECTIONAL )
				{
	        nglDirProjectorLightInfo* Light = (nglDirProjectorLightInfo*)LightNode->Data;
	        if ( nglIsSphereVisible( &Light->Frustum, WorldPos, Mesh->SphereRadius ) )
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
#endif

//-----------------------------------------------------------------------------

static void nglSetupWaveProjLighting(nglMeshNode* MeshNode, nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params)
{
	int NumProjLights = 0;

  	// Determine what lighting context (list of lights) the mesh uses.
  	nglCurLightContext = (nglLightContext*)nglDefaultLightContext;
  	if ( Params->Flags & NGLP_LIGHT_CONTEXT )
    	nglCurLightContext = (nglLightContext*)Params->LightContext;

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



void BumpSetTevOp(GXTevStageID id, GXTevMode mode)
{
    GXTevColorArg carg = GX_CC_RASC;
    GXTevAlphaArg aarg = GX_CA_RASA;

    if (id != GX_TEVSTAGE0) {
        carg = GX_CC_CPREV;
        aarg = GX_CA_APREV;
    }

    switch (mode) {
      case GX_MODULATE:
        GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_TEXC, carg, GX_CC_ZERO);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_TEXA, aarg, GX_CA_ZERO);
        break;
      case GX_DECAL:
        GXSetTevColorIn(id, carg, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, aarg);
        break;
      case GX_BLEND:
        GXSetTevColorIn(id, carg, GX_CC_ONE, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_TEXA, aarg, GX_CA_ZERO);
        break;
      case GX_REPLACE:
        GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        break;
      case GX_PASSCLR:
        GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, carg);
        GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, aarg);
        break;
      default:
        ASSERTMSG(0, "BumpSetTevOp: Invalid Tev Mode");
        break;
    }

    GXSetTevColorOp(id, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(id, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
}


static void nglBlendVertexAlpha ( GXTevStageID stage )
{
  GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA); //APREV);
  GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
  GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

}


static void nglBumpBlendAlphaPass( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA); //APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
}


static void nglBumpBlendDecal( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	//GXColor tcolor = { 255, 255, 255, 255 };
	//GXSetTevColor( GX_TEVREG2, tcolor );

	GXTevColorArg CArg,CArg2;
	GXTevAlphaArg AArg;
	int AlternateAlphaInputs=false;

	CArg = GX_CC_CPREV;
	//CArg = GX_CC_APREV; //TEXC;
	AArg = GX_CA_APREV;
	if( constant == 1.0f )
	{
		CArg2 = GX_CC_TEXA;
	}
	else
	{
		CArg2 = GX_CC_KONST ;
		u_int RangeAdjustedConstant = (u_int) ( 255.0f * constant );
		// <<<< 20-Feb-2002 SAL: Weird hardware bug? The Konst Alpha set here is affecting the Punchthrough test, though we don't select it anywhere. Test it later on.
		//GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant };
		GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, 0xFF };
		GXSetTevKColor( GX_KCOLOR0, color );
		GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
		GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
	}
	#ifdef BASEWATEROFF
	CArg = GX_CC_TEXC;
	AArg = GX_CA_TEXA;
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	#endif


	//GXSetTevOrder( stage, coord, map, GX_COLOR1A1 );
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
									 CArg,
									 nglGCTempSrcClr,
									 CArg2,
									 GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
}


static void nglBumpBlendAdditive( GXTevStageID stage,
															GXTexCoordID coord,
															GXTexMapID map,
															float constant = 1.0f )
{
	//GXColor tcolor = { 255, 255, 255, 255 };
	//GXSetTevColor( GX_TEVREG2, tcolor );
	GXTevColorArg CArg,CArg2;
	GXTevAlphaArg AArg,AArg2;
	int AlternateAlphaInputs=false; //true;

	CArg = GX_CC_CPREV;
	//CArg = GX_CC_RASC;
	AArg = GX_CA_APREV;
	CArg2 = GX_CC_TEXA;
	AArg2 = nglGCTempSrcAlpha;
	if( constant != 1.0f )
	{
		CArg2 = GX_CC_KONST;
		u_int RangeAdjustedConstant = (u_int) ( 255.0f * constant );
		GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant };
		GXSetTevKColor( GX_KCOLOR0, color );
		GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
		GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
	}
	#ifdef BASEWATEROFF
	CArg = GX_CC_TEXC;
	AArg = GX_CA_TEXA;
	AArg2 = GX_CA_TEXA;
	#endif

	//GXSetTevOrder( stage, coord, map, GX_COLOR1A1 );
	GXSetTevOrder( stage, coord, map, GX_COLOR_NULL );
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
									nglGCTempSrcClr,
									CArg2,
									CArg );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
}

static void nglFoamBlendDecal( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	GXTevColorArg CArg,CArg2;
	GXTevAlphaArg AArg;
	int AlternateAlphaInputs=false;

	if( !NGLGC_BASE_STAGE_CHECK(stage) ) {
		CArg = GX_CC_CPREV;
		AArg = GX_CA_APREV;
		if( constant == 1.0f )
		{
			CArg2 = GX_CC_TEXA;
		}
		else
		{
			CArg2 = GX_CC_KONST ;
			u_int RangeAdjustedConstant = (u_int) ( 255.0f * constant );
			// <<<< 20-Feb-2002 SAL: Weird hardware bug? The Konst Alpha set here is affecting the Punchthrough test, though we don't select it anywhere. Test it later on.
			//GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant };
			GXColor color = { RangeAdjustedConstant, RangeAdjustedConstant, RangeAdjustedConstant, 0xFF };
			GXSetTevKColor( GX_KCOLOR0, color );
			GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
			GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
		}
	}
	else
	{
		CArg = GX_CC_ZERO;
		CArg2 = GX_CC_ONE;
		if( constant == 1.0f )
		{
			AArg = nglGCTempSrcAlpha;
		}
		else
		{
			AArg = GX_CA_KONST;
		}
		u_int RangeAdjustedConstant = (u_int) ( 255.0f * constant );
		GXColor color = { 0xFF, 0xFF, 0xFF, RangeAdjustedConstant };
		#if 0
		if(nglGCTempMC)
		{
			AArg = GX_CA_KONST;
			CArg2 = GX_CC_KONST;
		 	color.r = (u8) ( nglGCTempMaterialColor[0] * 255.0f );
		 	color.g = (u8) ( nglGCTempMaterialColor[1] * 255.0f );
		 	color.b = (u8) ( nglGCTempMaterialColor[2] * 255.0f );
 			if( constant != 1.0f )
 			{
			 	color.a = (u8) ( nglGCTempMaterialColor[3] * 255.0f );
			}
			else
			 	AlternateAlphaInputs=true;
		}
		#endif
		GXSetTevKColor( GX_KCOLOR0, color );
		GXSetTevKColorSel( stage, GX_TEV_KCSEL_K0  );
		GXSetTevKAlphaSel( stage, GX_TEV_KASEL_K0_A );
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	}

	GXSetTevOrder( stage, coord, map, GX_COLOR1A1 ); //_NULL );
	#if 0
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn( stage,
									 CArg,
									 nglGCTempSrcClr,
									 CArg2,
									 GX_CC_ZERO );
	#else
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn( stage,
									 GX_CC_CPREV, //CArg,
									 GX_CC_C0, //TEXC, //nglGCTempSrcClr,
									 GX_CC_TEXA, //CArg2,
									 GX_CC_ZERO );
	#endif
	#if 0
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	if( AlternateAlphaInputs )
		GXSetTevAlphaIn( stage,
										 GX_CA_ZERO,
										 AArg,
										 nglGCTempSrcAlpha,
										 GX_CA_ZERO );
	else
		GXSetTevAlphaIn( stage,
										 AArg,
										 GX_CA_ZERO,
										 GX_CA_ZERO,
										 GX_CA_ZERO );
	#else
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
	#endif
}

static void nglFoamBlendDecalVC( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	GXTevColorArg CArg,CArg2;
	CArg2=nglGCTempSrcVCC;
	GXTevAlphaArg AArg,AArg2=nglGCTempSrcVCA;

	if( !NGLGC_BASE_STAGE_CHECK(stage) )
	{
		CArg = GX_CC_CPREV;
		CArg2 = GX_CC_TEXA;
		AArg = GX_CA_APREV;
	}
	else
	{
		CArg = GX_CC_ZERO;
		CArg2 = GX_CC_ONE;
		AArg = nglGCTempSrcAlpha;
	}

	//AArg = nglGCTempSrcAlpha;
//	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);

	GXSetTevOrder( stage, coord, map, GX_COLOR1A1 );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVPREV );
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn( stage,
									 GX_CC_ZERO, //CArg,
									 GX_CC_TEXA, //nglGCTempSrcClr,
									 GX_CC_RASA, //CArg2,
									 GX_CC_CPREV );
	#if 1
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
									 GX_CA_APREV );
	#else
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
	#endif
}

static void nglBlendWaveProjLightStage1( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	GXSetTevOrder( stage, coord, map, GX_ALPHA0 );
	GXSetTevColorOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVREG0 );
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorIn( stage,
									 nglGCTempSrcClr,
									 GX_CC_ZERO,
									 GX_CC_ZERO,
									 GX_CC_ZERO );
	GXSetTevAlphaOp( stage,
									 GX_TEV_ADD,
									 GX_TB_ZERO,
									 GX_CS_SCALE_1,
									 GX_TRUE,
									 GX_TEVREG0 );
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaIn( stage,
									 GX_CA_ZERO,
									 nglGCTempSrcAlpha,
									 GX_CA_RASA,
									 GX_CA_ZERO );
}

static void nglBlendWaveProjLightStage2( GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f )
{
	//GXSetTevOrder( stage, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR_NULL );
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
									 GX_CC_TEXC, //0,
									 GX_CC_TEXA, //A0,
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
									 GX_CA_ZERO );
}


static void nglBlendWaveDecalVC(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map , float constant = 1.0f)
{
	GXTevColorArg CArg2 = nglGCTempSrcVCC;
	GXTevAlphaArg AArg,AArg2 = nglGCTempSrcVCA;

	NGL_ASSERT(NGLGC_BASE_STAGE_CHECK(stage), "");

	if (constant == 1.0f)
	{
		AArg = nglGCTempSrcAlpha;
	}
	else
	{
		AArg = GX_CA_KONST;
		u_int RangeAdjustedConstant = (u_int)(255.0f * constant);
		GXColor color = { 0xFF, 0xFF, 0xFF, RangeAdjustedConstant };
		GXSetTevKColor(GX_KCOLOR0, color);
		GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
		GXSetTevKAlphaSel(stage, GX_TEV_KASEL_K0_A);
	}

	// dst_pix_clr = src_pix_clr * src_alpha + dst_pix_clr * 1.0 - (src_alpha)
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	// associate texture, coords and color register with this stage
	GXSetTevOrder(stage, coord, map, GX_COLOR0A0);
	// set color and alpha inputs
	GXSetTevColorIn(stage, GX_CC_ZERO, nglGCTempSrcClr, CArg2, GX_CC_ZERO);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, AArg, AArg2, GX_CA_ZERO);
	// C' = Cf * ( 1 - At ) + Ct * At + 0
	//    = Cf * ( 1 - At ) + Ct * At
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	// A' = 0 * ( 1 - 0 ) + 0 * 0 + Af
	//    = Af
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}




NGL_INLINE void nglSendWaveTENoIndex( u_int NIndices, u_short* Indices )
{
	for( int j = 0; j < NIndices; ++j )
	{
		GXPosition1x16( j );
		nglGenEnvWaveCoords( j );
		//GXNormal1x16( j );
		nglVertex *Vert=&nglWaveEnvVerts[j];
		GXColor1x16( j );
		GXColor4u8( 128, 128, 128, Vert->Pad1 );
		GXTexCoord1x16( j );
	}
}

bool filterbumptex=true;



f32 indMtx[2][3] = { { 0.5, 0.5, 0.5 } , { 0.5, -0.5, 0.5 } };

//#define BUMPCRAP
extern nglTexture *DiffusePaletteTexture;
extern nglTexture *ElevationPaletteTexture;

static void nglRenderWave( nglMeshNode *MeshNode, nglMesh* Mesh, nglMeshSection* Section, nglRenderParams* Params )
{
	int BumpMapInd;
	int EnvMapTexStage;

	//nglGCTempVCInFirstTEVStage=false; //true;
	nglMaterial* Material = Section->Material;

	Material->Flags |= NGLMAT_BILINEAR_FILTER;
  //My hacks
	Material->Flags &= ~NGLMAT_ALPHA;
	Material->MapBlendMode = 0;

	bool isWaveMesh=false;
	if ( Material->Flags & NGLMAT_ENV_SPECULAR )
	{
		isWaveMesh=true;
			// subverting the meaning of this flag to indicate the main wave mesh
		Material->Flags &= ~NGLMAT_ENV_SPECULAR ;
	}

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

	if( ( Material->Flags & NGLMAT_BACKFACE_CULL ) ||
			( Material->Flags & NGLMAT_BACKFACE_DEFAULT ) )
	{
	  //GXSetCullMode( ( Params->Flags & NGLP_REVERSE_BACKFACECULL ) ? GX_CULL_FRONT : GX_CULL_BACK );
	  GXSetCullMode( GX_CULL_FRONT );
	}
	else
	{
		GXSetCullMode( GX_CULL_NONE );
	}
		// is this the main mesh
 	//if ( isWaveMesh )
	//	GXSetCullMode( GX_CULL_FRONT );

	{
		// Reset to the packed format of the vertices.
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14 );
		//nglGCUseVCFix = true;
	}

	nglGCSkipEnvMap = true;
	//nglGCUseVCFix = false;
	//nglGCSkipVCFix = true;
	nglGCSkipEnvMap = false;

	if ( isWaveMesh )
		nglSetupWaveProjLighting( MeshNode, Mesh, Section, Params );

	GXSetNumChans( 2 );
	GXColor tcolor = { 128, 128, 128, 255 };
	GXSetChanAmbColor( GX_COLOR1, tcolor );
	GXSetChanMatColor( GX_COLOR1, tcolor );
	GXSetChanCtrl( GX_COLOR1A1, GX_TRUE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
		GX_DF_CLAMP, GX_AF_NONE );
	//GXSetChanAmbColor( GX_COLOR0, tcolor );
	//GXSetChanMatColor( GX_COLOR0, tcolor );
	//GXSetChanCtrl( GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
	//	GX_DF_CLAMP, GX_AF_NONE );
	//int RMode = 0; //nglSetupWaveVtxFmt( Mesh, Section, Material, Params );
	//static int nglSetupWaveVtxFmt( nglMesh* Mesh, nglMeshSection* Section, nglMaterial* Material, nglRenderParams* Params )
	{
		u_int Flags = Material->Flags;
		int Stage = 0;
		//int RMode = 0;
		//GXAttr LightMapAttr = GX_VA_TEX7;
		//GXAttr DetailMapAttr = GX_VA_TEX7; // Set to dummy for now.
		int EnvMapStage = -1;
		int InputTexCoord = 0;
		int TexCoordDst = 0;

		// clear the descriptions
		GXClearVtxDesc( );

		// pos, color, and tex 0 are always indexed
		GXSetVtxDesc( GX_VA_POS, GX_INDEX16 );
		GXSetVtxDesc( GX_VA_NBT, GX_DIRECT );
		//GXSetVtxDesc( GX_VA_NRM, GX_INDEX16 );
		GXSetVtxDesc( GX_VA_CLR0, GX_INDEX16 );
		GXSetVtxDesc( GX_VA_CLR1, GX_DIRECT );
		GXSetVtxDesc( GX_VA_TEX0, GX_INDEX16 );
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 9 );
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, GX_F32, 0);
		//GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_XYZ, GX_F32, 0);
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGBA8, 0 );
		GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0 );


		GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
		Stage++;
		TexCoordDst++;
		InputTexCoord++;
		//RMode |= NGL_RMODE_TEXTURE;

		if ( Flags & NGLMAT_BUMP_MAP )
		{

			// same with environment maps
			//GXSetTexCoordGen( nglTexCoordDst[TexCoordDst], GX_TG_MTX2x4, GX_TG_BINRM, nglTexMtx[NGLTEXMTX_IDENTITY] );
			GXSetTexCoordGen2( GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_BINRM,
				GX_IDENTITY, GX_ENABLE, GX_PTTEXMTX0 );
			nglMatrix EnvMat;
			nglIdentityMatrix( EnvMat );
			EnvMat[0][0] = 0.5f;
			EnvMat[0][3] = 0.5f;
			EnvMat[1][1] = -0.5f; // 24-Jan-2002 SAL: Flip the y to match our textures.
			EnvMat[1][3] = 0.5f;
			EnvMat[2][2] = 0;
			EnvMat[2][3] = 1;
			GXLoadTexMtxImm( (float (*)[4])&EnvMat, GX_PTTEXMTX0, GX_MTX3x4 );
			EnvMapStage=Stage;

			Stage++;
			TexCoordDst++;

			//RMode |= NGL_RMODE_ENVIRONMENT;
		}

		GXSetTexCoordGen( GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
		Stage++;
		TexCoordDst++;


		if (nglGCDoShadowMap)
		{
    	nglMulMatrix(ProjLightLocalToUV, MeshNode->LocalToWorld, TheProjLight->WorldToUV);
    	Mtx ShadowMapMtx, TempMtx;
			nglToMtx(ShadowMapMtx, ProjLightLocalToUV);
			//MTXTranspose(ShadowMapMtx, TempMtx);
    	GXLoadTexMtxImm(ShadowMapMtx, nglTexMtx[NGLTEXMTX_SHADMAP], GX_MTX2x4);
			GXSetTexCoordGen(nglTexCoordDst[TexCoordDst], GX_TG_MTX2x4, GX_TG_POS, nglTexMtx[NGLTEXMTX_SHADMAP]);
			TexCoordDst++;
			Stage++;
		}


		// our # of texcoords is equal to the number of stages
		GXSetNumTexGens( Stage );

		nglVertex* Verts = (nglVertex*) Section->Verts;
		nglVertEnvMapTemp = (int*) Verts;
		nglVertEnvMapSize = sizeof( nglVertex ) >> 2;

		//LightMapAttrCache = LightMapAttr;
		//DetailMapAttrCache = DetailMapAttr;
		nglVertex2* VertDs = (nglVertex2*) Section->VertDs;
		nglVertP* VertPs = (nglVertP*) Section->VertPNs;
		nglVertN* VertNs = (nglVertN*) Section->VertNs;
		VertMap = NULL;
		nglEnvMapVertP = VertPs;
		nglEnvMapVertN = VertNs;
	}

	nglSetupWaveLighting( MeshNode, Mesh, Section, Params );

#ifdef BASEWATEROFF
	GXTevStageID BaseStage =GX_TEVSTAGE0;
	GXTevStageID BumpStage1=GX_TEVSTAGE0;
	GXTevStageID BumpStage2=GX_TEVSTAGE1;
#else
	GXTevStageID BaseStage =GX_TEVSTAGE0;
	GXTevStageID BumpStage1=GX_TEVSTAGE1;
	GXTevStageID BumpStage2=GX_TEVSTAGE2;
#endif
	//nglSetupWaveTevs( Material, Params );

	{
		// index into tev triplet list, # of stages
		int Stage = 0;
		int CoordInd = 0;
		int MapInd = 0;
		u_int Flags = Material->MapFlags;
		int FirstMode = -1;
		int VCInFirstTEVStage=false; // VertCol Blend in first TEV stage if all we have to do is VertCol and Base Tex.


		// default blend mode
		GXSetBlendMode( GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP );

		// ------------ Set up textures -----------------

		//#define MULTIPALETTEWATER

		nglTexture* Map = Material->Map;
		nglSetClampMode( Flags, Map );
		nglSetFilterMode( Flags, Map );
		if( Map->PaletteSize )
		{
			#ifndef MULTIPALETTEWATER
			GXLoadTlut( &Map->TlutObj, GX_TEXMAP0 );
			GXInitTexObjTlut( &Map->TexObj, GX_TEXMAP0 );
			#else
			assert( DiffusePaletteTexture->PaletteSize );
			GXLoadTlut( &DiffusePaletteTexture->TlutObj, GX_TEXMAP0 );
			GXInitTexObjTlut( &DiffusePaletteTexture->TexObj, GX_TEXMAP0 );
			#endif
		}
		#ifdef MULTIPALETTEWATER
		else
		{
			assert(0);
		}
		#endif
		GXLoadTexObj( &Map->TexObj, GX_TEXMAP0 );

		if ( Material->Flags & NGLMAT_BUMP_MAP )
		{
			nglTexture* EnvironmentMap = Material->EnvironmentMap;
			nglSetClampMode( Flags, EnvironmentMap );
			nglSetFilterMode( Flags, EnvironmentMap );
			if( EnvironmentMap->PaletteSize )
			{
				GXLoadTlut( &EnvironmentMap->TlutObj, GX_TEXMAP1 );
				GXInitTexObjTlut( &EnvironmentMap->TexObj, GX_TEXMAP1 );
			}
			GXLoadTexObj( &EnvironmentMap->TexObj, GX_TEXMAP1 );

			nglTexture* BumpMap = WAVETEX_GetTexture( WAVETEX_TEXBUMP2 );
			nglSetClampMode( Flags, BumpMap );
			nglSetFilterMode( Flags, BumpMap );
			if( BumpMap->PaletteSize )
			{
				#ifndef MULTIPALETTEWATER
				GXLoadTlut( &BumpMap->TlutObj, GX_TEXMAP7 );
				GXInitTexObjTlut( &BumpMap->TexObj, GX_TEXMAP7 );
				#else
				assert( ElevationPaletteTexture->PaletteSize );
				GXLoadTlut( &ElevationPaletteTexture->TlutObj, GX_TEXMAP7 );
				GXInitTexObjTlut( &ElevationPaletteTexture->TexObj, GX_TEXMAP7 );
				#endif
			}
			#ifdef MULTIPALETTEWATER
			else
			{
				assert(0);
			}
			#endif
			#ifndef MULTIPALETTEWATER
			GXLoadTexObj( &BumpMap->TexObj, GX_TEXMAP7 );
			#else
			GXLoadTexObj( &Map->TexObj, GX_TEXMAP7 );
			#endif

			nglTexture* BumpMap2 = WAVETEX_GetTexture( WAVETEX_TEXBUMP2 );
			nglSetClampMode( Flags, BumpMap2 );
			nglSetFilterMode( Flags, BumpMap2 );
			if( BumpMap2->PaletteSize )
			{
				#ifndef MULTIPALETTEWATER
				GXLoadTlut( &BumpMap2->TlutObj, GX_TEXMAP6 );
				GXInitTexObjTlut( &BumpMap2->TexObj, GX_TEXMAP6 );
				#else
				GXLoadTlut( &ElevationPaletteTexture->TlutObj, GX_TEXMAP6 );
				GXInitTexObjTlut( &ElevationPaletteTexture->TexObj, GX_TEXMAP6 );
				#endif
			}
			#ifdef MULTIPALETTEWATER
			else
			{
				assert(0);
			}
			#endif

			#ifndef MULTIPALETTEWATER
			GXLoadTexObj( &BumpMap2->TexObj, GX_TEXMAP6 );
			#else
			GXLoadTexObj( &Map->TexObj, GX_TEXMAP6 );
			#endif
		}
 		if ( isWaveMesh )
		{
			nglTexture* FoamMap = WAVETEX_GetTexture( WAVETEX_TEXFOAM );
			nglSetClampMode( Flags, FoamMap );
			nglSetFilterMode( Flags, FoamMap );
			if( FoamMap->PaletteSize )
			{
				GXLoadTlut( &FoamMap->TlutObj, GX_TEXMAP5 );
				GXInitTexObjTlut( &FoamMap->TexObj, GX_TEXMAP5 );
			}
			GXLoadTexObj( &FoamMap->TexObj, GX_TEXMAP5 );
		}


#ifndef BASEWATEROFF
		#if 1
		nglBlendWaveDecalVC(
									BaseStage,
									GX_TEXCOORD0,
									GX_TEXMAP0 );
		#else
		nglBlendMode( Material->MapBlendMode,
									Material->MapBlendModeConstant,
									BaseStage,
									GX_TEXCOORD0,
									GX_TEXMAP0 );
		#endif
		GXSetTexCoordCylWrap( GX_TEXCOORD0, GX_FALSE, GX_FALSE );

		FirstMode = Material->MapBlendMode;


		Stage++;
		CoordInd++;
		MapInd++;
#endif

		if ( Material->Flags & NGLMAT_BUMP_MAP )
		{
			GXSetTexCoordCylWrap( GX_TEXCOORD1, GX_FALSE, GX_FALSE );
			//GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_TRUE, GX_TRUE );

			EnvMapTexStage=GX_TEVSTAGE1;

			Stage++;
			CoordInd++;
			MapInd++;

			BumpMapInd=MapInd;

			BumpMapInd=MapInd;
			MapInd++;


			// Heer thar be bump mapping

			GXSetNumIndStages(1);
	    Mtx normalMtx, bumpnormalMtx;
	    Mtx normalTexMtx, tempMtx;
	    if (!MTXInverse((float (*)[4])&nglCurScene->WorldToView, tempMtx)) {ASSERTMSG(0,"Singular matrix!\n");}
	    MTXTranspose(tempMtx, normalMtx);
			// matrix for transforming normals from object space to eye space,
			// scale by WaveBumpNormalScale/2, offset by 0.5,0.5.
			// the negation in Y is needed since the T axis is opposite of the Y axis
			MTXScale(tempMtx, WaveBumpNormalScale/2, -WaveBumpNormalScale/2, WaveBumpNormalScale/2);
			MTXConcat(tempMtx, normalMtx, normalTexMtx);
			MTXTrans(tempMtx, 0.5f, 0.5f, 0.0f);
			MTXConcat(tempMtx, normalTexMtx, normalTexMtx);

			// matrix for transforming bump offsets
			// take normal mtx, then scale by WaveBumpNormalScale/2
			// the negation in [1][1] is needed since the T axis is opposite of the Y axis
			MTXScale(tempMtx, WaveBumpNormalScale/2, WaveBumpNormalScale/2, WaveBumpNormalScale/2);
			MTXConcat(tempMtx, normalMtx, bumpnormalMtx);
			indMtx[0][0] =  bumpnormalMtx[0][0];
			indMtx[0][1] =  bumpnormalMtx[0][1];
			indMtx[0][2] =  bumpnormalMtx[0][2];
			indMtx[1][0] =  bumpnormalMtx[1][0];
			indMtx[1][1] =  -bumpnormalMtx[1][1];
			indMtx[1][2] =  bumpnormalMtx[1][2];
			// set indirect matrix and scale
			GXSetIndTexMtx(GX_ITM_0, indMtx, 0);

			// Indirect Stage 0 -- Sample normal perturbation map
			GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP7);
			GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

			// Stage 0 -- Save material texture
			//
			// TEVPREV = TEXC/TEXA
			//
			GXSetTevDirect(BumpStage1);
			//GXSetTevOrder(BumpStage1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR1A1 ); //_NULL);
			//BumpSetTevOp(GX_TEVSTAGE2, GX_REPLACE);
			//nglBlendReplace(
			//nglBlendDecalAddAlpha(
			nglBumpBlendDecal(
								BumpStage1,
								GX_TEXCOORD1,
								GX_TEXMAP1, 0.0f );
			//BumpSetTevOp(GX_TEVSTAGE2, GX_BLEND);
			// Set up the indirect bump calculation for Stage 1
			//
			GXSetTevIndBumpXYZ(BumpStage2, GX_INDTEXSTAGE0, GX_ITM_0);

			// Stage 1 -- Add source normal in Bump. Index lightmap with result of
			//            perturbation. Apply diffuse and specular components.
			//
			// TEVPREVC = PREVC * TEXC + TEXA
			// TEVPREVA = PREVA
			//
			//GXSetTevOrder(BumpStage2, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR1A1 ); //_NULL);
			nglBumpBlendAdditive(
								BumpStage2,
								GX_TEXCOORD1,
								GX_TEXMAP1 );
			Stage++;
		}

		// back to our regularly scheduled code

		nglGCTempSrcClr = GX_CC_TEXC;
		nglGCTempSrcAlpha = GX_CA_TEXA;
		// always 1 color channel
//		if( nglGCUseVCFix || nglGCDoShadowMap )
//			GXSetNumChans( 2 );
//		else
//			GXSetNumChans( 1 );

			// is this the main mesh
 		if ( isWaveMesh )
		{
			nglFoamBlendDecalVC(
								(GXTevStageID) Stage,
								GX_TEXCOORD2,
								GX_TEXMAP5 );
			Stage++;
			CoordInd++;
		}

		if( isWaveMesh && nglGCDoShadowMap )
		{
			nglSetClampMode( NGLMAP_CLAMP_U | NGLMAP_CLAMP_V, TheProjLight->Tex );
			nglSetFilterMode( NGLMAP_BILINEAR_FILTER, TheProjLight->Tex );
			// <<<< 23-Jan-2002 SAL: Must set filter and clamp modes here.
			if( nglGCUseWhiteShadow )
			GXLoadTexObj( &nglWhiteTex.TexObj, nglTevList[MapInd].map );
			else
			GXLoadTexObj( &TheProjLight->Tex->TexObj, nglTevList[MapInd].map );

#if 1
			nglBlendDecal(nglTevList[Stage].stage, nglTevList[CoordInd].coord, nglTevList[MapInd].map );
			Stage++;
#else
			nglBlendWaveProjLightStage1( nglTevList[Stage].stage, nglTevList[CoordInd].coord, nglTevList[MapInd].map );
			Stage++;
			nglBlendWaveProjLightStage2( nglTevList[Stage].stage, nglTevList[CoordInd].coord, nglTevList[MapInd].map );
			Stage++;
			#if 0
			nglBlendMode( NGLBM_PROJ_SHADOW_S1, //TheProjLight->BlendMode,
										TheProjLight->BlendModeConstant,
										nglTevList[Stage].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
			nglBlendMode( NGLBM_PROJ_SHADOW_S2, //TheProjLight->BlendMode,
										TheProjLight->BlendModeConstant,
										nglTevList[Stage+1].stage,
										nglTevList[CoordInd].coord,
										nglTevList[MapInd].map );
			#endif
#endif
			GXSetTexCoordCylWrap( nglTevList[CoordInd].coord, GX_FALSE, GX_FALSE );

			CoordInd++;
			MapInd++;
		}
		if( 1 ) //nglGCUseVCFix )
		{
			CoordInd++;
			CoordInd++;
		}

		GXSetNumTevStages( ( Stage > 0 ) ? Stage : 1 );

		// deal with alpha compare
		if( FirstMode == NGLBM_PUNCHTHROUGH )
		{
			GXSetAlphaCompare( GX_GREATER, 128, GX_AOP_AND, GX_ALWAYS, 0 );
			GXSetZCompLoc( GX_FALSE );
		}
		else
		{
			GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0 );
			GXSetZCompLoc( GX_TRUE );
		}

		// setup z mode based on first layer
		if( ( FirstMode == NGLBM_OPAQUE ) || ( FirstMode == NGLBM_PUNCHTHROUGH ) )
		{
			GXSetZMode( GX_TRUE, GX_LEQUAL, GX_TRUE );
		}
		else if( FirstMode == NGLBM_MOTION_BLUR)
		{
			GXSetZMode( GX_FALSE, GX_ALWAYS, GX_FALSE );
		}
		else
		{
			GXSetZMode( GX_TRUE, GX_LEQUAL, GX_FALSE );
		}

		nglGCTempSrcVCC = GX_CC_RASC;
		nglGCTempSrcVCA = GX_CA_RASA;
		//nglGCTempMC=false;
		//nglGCTempVCInFirstTEVStage=false;
	}

	nglGCSkipEnvMap = false;


	//nglSendWaveVerts( Mesh, Section, RMode, Mesh->Flags, Params );
	//static void nglSendWaveVerts( nglMesh* Mesh, nglMeshSection* Section, int RMode, u_int MeshFlags, nglRenderParams* Params )
	{
		u_int MeshFlags=Mesh->Flags;
		u_int NIndices = 0;

		NIndices = Section->NVerts;

		u_short* Indices = Section->Indices;
	 	GXInvalidateVtxCache( );

		bool UseDisplayList = false;
		if( NIndices )
		{

			if ( 1 ) //MeshFlags & NGLMESH_SCRATCH_MESH)
			{
				nglVertex* Verts = (nglVertex*) Section->Verts;
				short* NVerts = Section->VertMap;

				for (u_int strip=0;strip<Section->NTris;strip++)
				{
					nglWaveEnvVerts=Verts;
					GXSetArray( GX_VA_POS, &Verts->x, sizeof( nglVertex ) );
					GXSetArray( GX_VA_NRM, &Verts->nx, sizeof( nglVertex ) );
					GXSetArray( GX_VA_CLR0, &Verts->color, sizeof( nglVertex ) );
					GXSetArray( GX_VA_CLR1, &Verts->color, sizeof( nglVertex ) );
					GXSetArray( GX_VA_TEX0, &Verts->u1, sizeof( nglVertex ) );
					GXSetArray( GX_VA_TEX1, &Verts->u1, sizeof( nglVertex ) );
					GXSetArray( GX_VA_TEX2, &Verts->u1, sizeof( nglVertex ) );

					if ( *NVerts )
					{
						GXBegin( GX_TRIANGLESTRIP, GX_VTXFMT0, *NVerts );
						nglSendWaveTENoIndex( *NVerts, NULL );
						GXEnd();
					}

					Verts += *NVerts;
					NVerts++;
				}
			}
		}
	}

	nglGCDoShadowMap = false;
	//nglGCUseVCFix = false;
	nglGCSkinMesh = false;
	//nglGCSkipVCFix = false;
	nglGCSkipEnvMap = true;

	if( Mesh->Flags & NGLMESH_SKINNED ) {
#if NGL_GC_PROFILE
	nglGCPD.SkinDispList.Add();
#endif //#if NGL_GC_PROFILE
	}
	else
	{
#if NGL_GC_PROFILE
	nglGCPD.NonSkinDispList.Add();
#endif //#if NGL_GC_PROFILE
	}

	GXSetTevDirect(BumpStage2);
}




static void nglRenderWaveCallback( void* Data )
{
	nglMeshSectionNode* Node = (nglMeshSectionNode*) Data;

	nglMeshNode* MeshNode = Node->MeshNode;
	nglMesh* Mesh = MeshNode->Mesh;
	nglMeshSection* Section = Node->Section;
	nglRenderParams* Params = &MeshNode->Params;


	nglMatrix z;
	nglIdentityMatrix( z );
	z[2][2] = -1.0f;

	{

		// get our world to view matrix (camera) multiplied
		// against the local to world matrix of this mesh
		nglMatrix mz;
		nglMulMatrix( mz, nglCurScene->WorldToView, z );
		nglMatrix wmz;
		nglMulMatrix( wmz, MeshNode->LocalToWorld, mz );
		Mtx wmzp;
		nglToMtx( wmzp, wmz );
		GXLoadPosMtxImm( wmzp, GX_PNMTX0 );
		// Load the matrix for normal transformation.
		GXLoadNrmMtxImm( (float (*)[4])&MeshNode->WorldToLocal, GX_PNMTX0 );
		if( Mesh->NBones )
		{
			memcpy( &StorePosMtx1, &wmzp, sizeof(Mtx) );
			memcpy( &StorePosMtx2, &wmz, sizeof(nglMatrix) );
		}
	}

	// hack for env. mapping
	nglCopyMatrix( nglMeshLocalToWorld, MeshNode->LocalToWorld );
	// nglMeshWorldToLocal is used in lighting.
	nglCopyMatrix( nglMeshWorldToLocal, MeshNode->WorldToLocal );
	//nglMulMatrix( nglMeshLocalToView, nglMeshLocalToWorld, nglCurScene->WorldToView );
	while( Node )
	{


		nglRenderWave( MeshNode, Mesh, Section, Params );


		Node = Node->NextSectionNode;
		if( Node )
			Section = Node->Section;
	}
}




void nglListAddWave( nglMesh* Mesh, const nglMatrix LocalToWorld, nglRenderParams* Params )
{
  nglVector WorldCenter;  // World space center of the bounding sphere.
  nglVector Center;       // View space center of the bounding sphere.
  float Dist = 0.0f, Radius = 0.0f;
  int Clip = 0;

  // Check for NULL pointers, commonly encountered when file loads fail.
  if ( !Mesh )
    return;

//  if ( !( Params && ( Params->Flags & NGLP_FULL_MATRIX ) ) )	// Obsolete flag.  Was ignored previously.  (dc 05/30/02)
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
    {
      return;
    }

		if( !( Params && ( Params->Flags & NGLP_NO_CULLING )))
		{
		  // <<<< 07-Feb-2002 SAL: There is a bug in here. The lensflares were not surviving this clip test.
	    Clip = nglGetClipResult( Center, Radius );
	    if ( Clip == -1 )
	    {
	      return;
	    }
	  }

    // If sphere rejection is on and the sphere touches the plane, reject the whole mesh.
    if ( Clip && ( ( Mesh->Flags & NGLMESH_REJECT_SPHERE ) || DEBUG_ENABLE( ForceNoClip ) || !STAGE_ENABLE( Clip ) ) )
    {
      return;
    }

  }

  // Add a new mesh node to the mesh list.
  nglMeshNode* MeshNode = nglListNew( nglMeshNode );

	if( !MeshNode )
	{
		return;
	}

  MeshNode->Mesh = Mesh;
	nglCopyMatrix( MeshNode->LocalToWorld, LocalToWorld );
	nglInverseMatrix( MeshNode->WorldToLocal, LocalToWorld );

  if ( Params )
    MeshNode->Params = *Params;
  else
    MeshNode->Params.Flags = 0;

/*	// Obsolete flag.  Was ignored previously.  (dc 05/30/02)
  if ( MeshNode->Params.Flags & NGLP_FULL_MATRIX )
    nglCopyMatrix( MeshNode->LocalToScreen, LocalToWorld );
  else
*/
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
    //if ( Material->DetailMap && Dist > Material->DetailMapRange )
    //  SectionNode->MaterialFlags &= ~NGLMAT_DETAIL_MAP;

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
	    nglListAddNode( NGLNODE_MESH_SECTION, nglRenderWaveCallback, SectionNode, &SortInfo );
	  }
  }
}
#endif

void nglListAddScratchWave( u_int ID, const nglMatrix LocalToWorld, nglRenderParams* Params )
{
	#ifdef USE_STD_SCRATCH_MESH_CODE
		nglListAddMesh( (nglMesh *) ID, LocalToWorld, Params );
	#else


		nglMesh* Mesh = (nglMesh*)ID;
		DCStoreRangeNoSync(Mesh, Mesh->DataSize);
	 	nglListAddWave( Mesh, LocalToWorld, Params );
	#endif
}










