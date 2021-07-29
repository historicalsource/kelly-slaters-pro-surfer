#include "global.h"

#ifndef TARGET_PS2
#error This file should only be included in builds where TARGET_PS2 is defined. (dc 06/06/02)
#endif

#include "ngl_ps2_internal.h"
#include "ksngl.h"
#include "ps2_waverender.h"	// For KSMAT_WAVETRANS (dc 06/06/02)

#undef  START_PROF_TIMER
#define START_PROF_TIMER( timer ) ((void)0)
#undef  STOP_PROF_TIMER
#define STOP_PROF_TIMER( timer ) ((void)0)

extern char nglWaveTransAddr[];
extern char nglWaveDarkAddr[];
extern char nglWaveHighlightAddr[];
extern char nglWaveFoamAddr[];

#define PROJECT_KELLYSLATER	// Allows us to distinguish what was copied from NGL from what we added.  (dc 06/01/02)

static float DarkAlphaScale = 225.0f; //96.0f; //1000
static float DarkAlphaOffset = 0.0f; //64.0f;  //-400
static float HighlightAlphaScale = 192.f;
static float HighlightAlphaOffset = -64.0f; //-160.f;
static float CoreHighlightAlphaScale = 192.f;
static float CoreHighlightAlphaOffset = -64.0f; //-160.f;
//static float HighlightSubFactor = 2.0f; //-160.f;
static float TransHighlightAlphaScale = 192.f;
static float TransHighlightAlphaOffset = -64.0f; //-160.f;
static float TransHighlightAlphaMin = 64.0f; //-160.f;
//static float TransparentSubFactor = 1.0f; //-160.f;

static nglVector SunPos(0, 1000, -3500, 1);

#include "waverendermenu.cpp"

void WAVERENDER_SetWaveDarkParams( float scale, float off )
{
	DarkAlphaScale = scale;
	DarkAlphaOffset = off;
}

void WAVERENDER_SetWaveHighlightParams( float scale, float off, float cscale, float coff, nglVector sun, float tscale, float toff, float tmin )
{
	HighlightAlphaScale = scale;
	HighlightAlphaOffset = off;
	CoreHighlightAlphaScale = cscale;
	CoreHighlightAlphaOffset = coff;
	TransHighlightAlphaScale = tscale;
	TransHighlightAlphaOffset = toff;
	TransHighlightAlphaMin = tmin;
	SunPos = sun;
}

void WAVERENDER_RenderSectionWave( u_int*& _Packet, void* Param );
void nglVif1AddWaveDark(u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, bool standalone );
void nglVif1AddWaveFoam(u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, bool standalone );
void nglVif1AddWaveHighlight(u_int*& _Packet, u_int& _CmdListDataPtr, int asCore, nglMaterial* Material, u_int MaterialFlags, bool standalone );
void nglVif1AddWaveTransparency(u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, bool standalone );

// Copied from NGL, because needed for compiling section renderer. (dc 06/01/02)
extern float nglMipBias;
extern u_int nglTexL;

// Kelly Slater Wave Custom Stuff.
#ifdef PROJECT_KELLYSLATER

void nglVif1AddWaveDark(u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, bool standalone )
{
	u_int* Packet = _Packet;
	u_int CmdListDataPtr = _CmdListDataPtr;
	
	if ( !STAGE_ENABLE( DetailPass ) )
		return;
	
	nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglWaveDarkAddr );
	float ScaleOffset[] = {DarkAlphaScale, DarkAlphaOffset};
    nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V2_32, 1, (void *) ScaleOffset, sizeof(ScaleOffset) );
	
	_Packet = Packet;
	_CmdListDataPtr = CmdListDataPtr;
	
}

void nglVif1AddWaveHighlight(u_int*& _Packet, u_int& _CmdListDataPtr, int asCore, nglMaterial* Material, u_int MaterialFlags, bool standalone )
{
	u_int* Packet = _Packet;
	u_int CmdListDataPtr = _CmdListDataPtr;
	
	if ( asCore == 0) // highlights
	{
		if ( !STAGE_ENABLE( LightPass ) )
			return;
		
		nglVif1AddCommandListProgram(  Packet, CmdListDataPtr, nglWaveHighlightAddr );
		nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, &SunPos, sizeof(nglVector) );
		float ScaleOffset[] = {HighlightAlphaScale, HighlightAlphaOffset};
		nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V2_32, 1, (void *) ScaleOffset, sizeof(ScaleOffset) );
	}
	else if ( asCore == 1) // core specularity
	{
		if ( !STAGE_ENABLE( EnvironmentPass ) )
			return;
		
		nglVif1AddCommandListProgram(  Packet, CmdListDataPtr, nglWaveHighlightAddr );
		nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, &SunPos, sizeof(nglVector) );
		float ScaleOffset[] = {CoreHighlightAlphaScale, CoreHighlightAlphaOffset};
		nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V2_32, 1, (void *) ScaleOffset, sizeof(ScaleOffset) );
	}
	else
	{
		nglFatal("Bad parameter\n");
	}
	
	_Packet = Packet;
	_CmdListDataPtr = CmdListDataPtr;
}

static float detransparifactor=-1.0f;

void nglVif1AddWaveTransparency(u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, bool standalone )
{
	u_int* Packet = _Packet;
	u_int CmdListDataPtr = _CmdListDataPtr;
	
	nglVif1AddCommandListProgram(  Packet, CmdListDataPtr, nglWaveTransAddr );
	float ScaleOffset[] = {TransHighlightAlphaScale, TransHighlightAlphaMin, detransparifactor};
	nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, (void *) ScaleOffset, sizeof(ScaleOffset) );
	
	_Packet = Packet;
	_CmdListDataPtr = CmdListDataPtr;
}

void nglVif1AddWaveFoam(u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, bool standalone )
{
	u_int* Packet = _Packet;
	u_int CmdListDataPtr = _CmdListDataPtr;
	
	nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglWaveFoamAddr );
	
	_Packet = Packet;
	_CmdListDataPtr = CmdListDataPtr;
}

// Ouch, had to copy instead of including header!  (dc 06/01/02)
enum
{
	NGLPASS_DIFFUSE,
		NGLPASS_DETAIL,
		NGLPASS_ENVIRONMENT,
		NGLPASS_LIGHT,
#ifdef PROJECT_KELLYSLATER
		NGLPASS_FOAM,
#endif
};

extern nglTexture  *nglWhiteTex;

enum 
{
	WAVERENDER_GIFTAGTEXTURE = 0x00000001, 
		WAVERENDER_GIFTAGBLENDMODE = 0x00000002,
		WAVERENDER_GIFTAGOTHER = 0x00000004,
};

// Copied from NGL so we could remove some unnecessary GifTags and save command list space (dc 06/04/02)
void WAVERENDER_Vif1AddSetTexture( nglTexture* Tex, u_int MapFlags, float MipRatio, u_int Options )
{
	START_PROF_TIMER( proftimer_section_set_texture );
	
	if (!Tex) Tex = nglWhiteTex;
	
	if ( Tex->Type == NGLTEX_IFL || Tex->Type == NGLTEX_ATE )
		Tex = Tex->Frames[nglTextureAnimFrame % Tex->NFrames];
	
	if ( Tex->Type == NGLTEX_TGA )
	{
		if ( Options & WAVERENDER_GIFTAGTEXTURE ) 
		{
			nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );
			nglVif1AddGSReg( NGL_GS_TEX1_1, Tex->GsTex1 );
			nglVif1AddGSReg( NGL_GS_TEXA, Tex->GsTexA );
		}
	}
	else
		if ( Tex->Type == NGLTEX_TIM2 )
		{
			// load 4-bit textures specially
			if ( ( (sceGsTex0*)&Tex->GsTex0 )->PSM == SCE_GS_PSMT4 )
			{
				// load it as an 8 bit texture
				( (sceGsTex0*)&Tex->GsTex0 )->PSM = SCE_GS_PSMT8;
				( (sceGsTex0*)&Tex->GsTex0 )->CLD = 1;
				if ( Options & WAVERENDER_GIFTAGTEXTURE ) 
				{
					nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );
				}
				
				// then treat it as a 4 bit texture
				( (sceGsTex0*)&Tex->GsTex0 )->PSM = SCE_GS_PSMT4;
				( (sceGsTex0*)&Tex->GsTex0 )->CLD = 0;
				if ( Options & WAVERENDER_GIFTAGTEXTURE ) 
				{
					nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );
				}
			}
			else
			{
				if ( Options & WAVERENDER_GIFTAGTEXTURE ) 
				{
					nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );
				}
			}
			
#if !NGL_DISABLE_MIPMAPS
			if ( Tex->MipMapTextures > 1 && MipRatio && !DEBUG_ENABLE( DisableMipmaps ) )
			{
				if ( Options & WAVERENDER_GIFTAGTEXTURE ) 
				{
					nglVif1AddGSReg( NGL_GS_MIPTBP1_1, Tex->GsMipTBP1 );
					nglVif1AddGSReg( NGL_GS_MIPTBP2_1, Tex->GsMipTBP2 );
				}				
				u_int MinFilter = 4, MagFilter = 1;  // Bilinear by default
				if ( MapFlags & NGLMAP_POINT_FILTER )
					MinFilter = 2, MagFilter = 0;
				else if ( MapFlags & NGLMAP_TRILINEAR_FILTER )
					MinFilter = 5;
				
				u_int FirstLod = Tex->MipMapFirstLod;
				int K = -FirstLod - (nglLog2( nglMipBias * nglCurScene->ScrZ * MipRatio * Tex->InvHypot ) << nglTexL);
				( (sceGsTex1*)&Tex->GsTex1 )->K = (K&255)<<4;
				( (sceGsTex1*)&Tex->GsTex1 )->MMAG = MagFilter;
				( (sceGsTex1*)&Tex->GsTex1 )->MMIN = MinFilter;
				( (sceGsTex1*)&Tex->GsTex1 )->MXL = Tex->MipMapTextures-1 - FirstLod;
				if ( Options & WAVERENDER_GIFTAGTEXTURE ) 
				{
					nglVif1AddGSReg( NGL_GS_TEX1_1, Tex->GsTex1 );
				}
			}
			else
#endif
			{
				u_int MinFilter = 1, MagFilter = 1;  // Bilinear by default
				if ( MapFlags & NGLMAP_POINT_FILTER )
					MinFilter = MagFilter = 0;
				if ( Options & WAVERENDER_GIFTAGOTHER ) 
				{
					Tex->GsTex1 = SCE_GS_SET_TEX1( 0, 0, MagFilter, MinFilter, 0, 0, 0 );
					nglVif1AddGSReg( NGL_GS_TEX1_1, Tex->GsTex1 );
				}
			}
			if ( Options & WAVERENDER_GIFTAGOTHER ) 
			{
				nglVif1AddGSReg( NGL_GS_TEXA, Tex->GsTexA );
			}
		}
		
		// If this is a feedback style effect from the render target to itself, flush the texture cache before
		// rendering such that all previously rendered objects show up in the feedback result.
		// (I moved this from the top to here, so that nglVif1AddTextFixup won't be broken when it occurs - Paul).
		if ( Tex == nglCurScene->RenderTarget )
			nglVif1AddRawGSReg( SCE_GS_TEXFLUSH, 0 );
		
		STOP_PROF_TIMER( proftimer_section_set_texture );
}

// Copied from NGL so we could remove some unnecessary GifTags and save command list space (dc 06/04/02)
void WAVERENDER_Vif1BakeMaterialPass( u_int*& Packet, u_int& CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, u_int Pass, u_int Options )
{
	START_PROF_TIMER( proftimer_section_setup_material );
	
	// Per-pass variables.
	u_int Mode, Fix;
	u_long Clamp;
	nglTexture* Map;
	float MipRatio;
	
	switch ( Pass )
	{
	case NGLPASS_DIFFUSE:
		Clamp = SCE_GS_SET_CLAMP( ( MaterialFlags & NGLMAT_CLAMP_U ) != 0, ( MaterialFlags & NGLMAT_CLAMP_V ) != 0, 0, 0, 0, 0 );
		Map = Material->Map; MipRatio = Material->MapMipRatio;
		Mode = Material->MapBlendMode; Fix = Material->MapBlendModeConstant;
		break;
	case NGLPASS_DETAIL:
		Clamp = SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 );
		Map = Material->DetailMap; MipRatio = Material->MapMipRatio;
		Mode = Material->DetailMapBlendMode; Fix = Material->DetailMapBlendModeConstant;
		break;
	case NGLPASS_ENVIRONMENT:
	/*
	#ifdef PROJECT_KELLYSLATER
	if ( MaterialFlags & NGLMAT_WAVETRANS )
	Clamp = SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 );
	else
	#endif
		*/
		Clamp = SCE_GS_SET_CLAMP( 0, MaterialFlags & NGLMAT_ENV_CYLINDER ? 0 : 1, 0, 0, 0, 0 );
		Map = Material->EnvironmentMap; MipRatio = Material->MapMipRatio;
		Mode = Material->EnvironmentMapBlendMode; Fix = Material->EnvironmentMapBlendModeConstant;
		break;
	case NGLPASS_LIGHT:
	/*
	#ifdef PROJECT_KELLYSLATER
	if ( MaterialFlags & NGLMAT_WAVETRANS )
	Clamp = SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 );
	else
	#endif
		*/
		Clamp = SCE_GS_SET_CLAMP( ( MaterialFlags & NGLMAT_LIGHT_CLAMP_U ) != 0, ( MaterialFlags & NGLMAT_LIGHT_CLAMP_V ) != 0, 0, 0, 0, 0 );
		Map = Material->LightMap; MipRatio = Material->LightMapMipRatio;
		Mode = Material->LightMapBlendMode; Fix = Material->LightMapBlendModeConstant;
		break;
#ifdef PROJECT_KELLYSLATER
	case NGLPASS_FOAM:
		Clamp = SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 );
		Map = (nglTexture *) Material->Pad0; MipRatio = Material->MapMipRatio;
		Mode = Material->Pad[0]; Fix = Material->Pad[1];
		break;
#endif
	default:
		NGL_ASSERT(false, "Invalid pass number.");
		Clamp = SCE_GS_SET_CLAMP( ( MaterialFlags & NGLMAT_CLAMP_U ) != 0, ( MaterialFlags & NGLMAT_CLAMP_V ) != 0, 0, 0, 0, 0 );
		Map = Material->Map; MipRatio = Material->MapMipRatio;
		Mode = Material->MapBlendMode; Fix = Material->MapBlendModeConstant;
		break;
	}
	
	nglVif1StartMaterialKick();
	
	// Blend mode comes first, because when NGL detects Alpha TINT on a mesh it has to replace the ALPHA and TEST registers
	// with those from NGLBM_BLEND.  Call it a hack, it is.
	// BlendMode does change sometimes between passes. (dc 06/06/02)
	if ( Options & WAVERENDER_GIFTAGBLENDMODE )
	{
		nglVif1AddSetBlendMode( Mode, Fix, ( MaterialFlags & NGLMAT_FORCE_Z_WRITE ) ? true : false );
	}
	
	// Second comes the texture map for the pass.  Each time the material is rendered, we replace these values with the actual
	// streamed location of the texture and its mipmaps, so here we just have to leave space for 5 GS registers.
	u_int MapFlags = 0;
#ifdef NGL_FORCE_FILTER
	MapFlags |= NGL_FORCE_FILTER		// For projects with bunk data.
#else
		if ( MaterialFlags & NGLMAT_BILINEAR_FILTER )
			MapFlags |= NGLMAP_BILINEAR_FILTER;
		else
			if ( MaterialFlags & NGLMAT_TRILINEAR_FILTER )
				MapFlags |= NGLMAP_TRILINEAR_FILTER;
			else
				MapFlags |= NGLMAP_POINT_FILTER;
#endif
			WAVERENDER_Vif1AddSetTexture( Map, MapFlags, MipRatio, Options );
			
			// Last comes everything else.
			if ( Options & WAVERENDER_GIFTAGOTHER ) 
			{
				nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRIM( 0, 1, 1, 0, 1, ( MaterialFlags & NGLMAT_ANTIALIAS ) != 0, 0, 0, 0 ) );
			}
			if ( Options & WAVERENDER_GIFTAGOTHER ) 
			{
				nglVif1AddGSReg( NGL_GS_CLAMP_1, Clamp );
			}
			nglVif1EndMaterialKick( Packet, CmdListDataPtr );
			
			STOP_PROF_TIMER( proftimer_section_setup_material );
}

void WAVERENDER_RenderSectionWave( u_int*& _Packet, void* Param )
{
	register nglMeshSectionNode* Node = (nglMeshSectionNode*)Param;
	register nglMeshNode* MeshNode = Node->MeshNode;
	register nglMesh* Mesh = MeshNode->Mesh;
	register nglMeshSection* Section = Node->Section;
	register nglMaterial* Material = Section->Material;
	register u_int MaterialFlags = Material->Flags;
	register nglMaterialInfo* Info = Material->Info;
	register u_int* Packet = _Packet;
	
	START_PROF_TIMER( proftimer_send_mesh_sections );
	
#ifdef INTERNAL_NGL
	if ( DEBUG_ENABLE( DumpFrameLog ) & 2 )
		nglPrintf( "LOG: Adding section %s(%d) of %s (%d).\n",
		MaterialFlags & NGLMAT_TEXTURE_MAP ? Material->Map->FileName.c_str() : "",
		Section - Mesh->Sections,
		Mesh->Name.c_str(),
		nglNodeCount );
#endif
	
	if ( nglLastVUCodeDma != NGL_VUCODE_MESH )
	{
		nglDmaStartTag( Packet );
		nglDmaEndTag( Packet, SCE_DMA_ID_CALL, nglLoadMicrocode );
		nglLastVUCodeDma = NGL_VUCODE_MESH;
	}
	
	nglDmaStartTag( Packet );
	
	// Batch DMA can reset the STCYCL register..
	Packet[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 );
	Packet[1] = SCE_VIF1_SET_STMASK( 0 );
	Packet[2] = 0xF0F0F0F0; // 11-11-00-00 (WZYX)
	Packet += 3;
	
	// Build the command lists.
	static u_char CmdListOfs[NGLCMD_NUM_LISTS];
	u_int CmdListDataPtr = NGLMEM_COMMAND_LIST_START;
	
#ifdef UNUSED
	// Skinning pass setup.
	if ( Mesh->Flags & NGLMESH_SKINNED )
	{
		CmdListOfs[NGLCMD_SKIN] = CmdListDataPtr;
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglSkinAddr );
		nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
	}
	else
		CmdListOfs[NGLCMD_SKIN] = 0;
#endif
	
	// Diffuse pass/basic transform and lighting.
	CmdListOfs[NGLCMD_DIFFUSE] = CmdListDataPtr;
	
#ifdef UNUSED
	nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglTextureScrollAddr );
	
	if ( Material->Flags & NGLMAT_MATERIAL_COLOR )
	{
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglMatColorAddr );
		nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_8, 1, &Material->Color, sizeof(u_int) );
	}
	
	nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglTintAddr );
	
	// Call lighting command list.
	if ( ( Material->Flags & NGLMAT_LIGHT ) && ( Mesh->Flags & NGL_LIGHTCAT_MASK ) )
	{
		u_int Addr = NGLCMD_VERTEX_LIGHT;
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglCommandListCallAddr );
		nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &Addr, sizeof(u_int) );
	}
	
	if ( !( Material->Flags & NGLMAT_TEXTURE_MAP ) )
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglWriteQAddr );
	
	nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglBackfaceCullScreenAddr );
#endif
	
	if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglTransformFrustumClipAddr );
	else
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglTransformClipAddr );
	
#ifdef PROJECT_KELLYSLATER
	if ( ( Material->UserFlags & KSMAT_WAVETRANS ) && ( WaverenderDebug.DrawPassTrans ) )
	{
		nglVif1AddWaveTransparency( Packet, CmdListDataPtr, Material, Material->Flags, false  );
	}
#endif
	
	nglTexture* Textures[NGL_MAX_MATERIAL_TEXTURES];
	u_int NTextures = 0;
	
	// Determine what lighting context (list of lights) the mesh uses.
	if ( MaterialFlags & NGLMAT_LIGHT )
	{
		nglLightContext* Context;
		if ( MeshNode->Params.Flags & NGLP_LIGHT_CONTEXT )
			Context = MeshNode->Params.LightContext;
		else
			Context = NULL;
		nglDetermineLocalLights( Context, Mesh, MeshNode->LocalToWorld, Textures, NTextures );
	}
	
#ifdef PROJECT_KELLYSLATER
	if ( Material->UserFlags & KSMAT_FOAM_MAP )
		assert (Material->Pad0);
#endif
	
	// Stream the textures.
	if ( ( MaterialFlags & NGLMAT_TEXTURE_MAP ) && ( WaverenderDebug.DrawPassLight ) )
		Textures[NTextures++] = Material->Map;
	if ( ( MaterialFlags & NGLMAT_DETAIL_MAP ) && ( WaverenderDebug.DrawPassDark ) )
		Textures[NTextures++] = Material->DetailMap;
	if ( ( MaterialFlags & NGLMAT_ENVIRONMENT_MAP ) && ( WaverenderDebug.DrawPassHighSpot ) )
		Textures[NTextures++] = Material->EnvironmentMap;
	if ( ( MaterialFlags & NGLMAT_LIGHT_MAP ) && ( WaverenderDebug.DrawPassHighCore ) )
		Textures[NTextures++] = Material->LightMap;
#ifdef PROJECT_KELLYSLATER
	if ( ( Material->UserFlags & KSMAT_FOAM_MAP ) && ( WaverenderDebug.DrawPassFoam ) )
		Textures[NTextures++] = (nglTexture *) Material->Pad0;
#endif
	nglVif1AddTextureStreaming( Packet, Textures, NTextures );
	
	u_int Options = 0xffffffff;
	int LastBlendMode = 0xffffffff;
	
	// Support models with no diffuse texture.
	if ( ( MaterialFlags & NGLMAT_TEXTURE_MAP ) && ( WaverenderDebug.DrawPassLight ) )
	{
#ifdef PROJECT_KELLYSLATER
		Info->DiffuseKickAddr = CmdListDataPtr;
		if (LastBlendMode == Material->MapBlendMode) Options &= ~WAVERENDER_GIFTAGBLENDMODE;
		else Options |= WAVERENDER_GIFTAGBLENDMODE;
		WAVERENDER_Vif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_DIFFUSE, Options );
		Options &= ~WAVERENDER_GIFTAGOTHER;
		LastBlendMode = Material->MapBlendMode;
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
		if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
			nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
#endif
	}
	nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
	
	// Detail map pass.
	if ( ( MaterialFlags & NGLMAT_DETAIL_MAP ) && ( WaverenderDebug.DrawPassDark ) )
	{
#ifdef PROJECT_KELLYSLATER
		CmdListOfs[NGLCMD_DETAIL] = CmdListDataPtr;
		nglVif1AddWaveDark( Packet, CmdListDataPtr, Material, Material->Flags, true  );
		Info->DetailKickAddr = CmdListDataPtr;
		if (LastBlendMode == Material->DetailMapBlendMode) Options &= ~WAVERENDER_GIFTAGBLENDMODE;
		else Options |= WAVERENDER_GIFTAGBLENDMODE;
		WAVERENDER_Vif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_DETAIL, Options );
		Options &= ~WAVERENDER_GIFTAGOTHER;
		LastBlendMode = Material->DetailMapBlendMode;
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
		if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
			nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
		nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
#endif
	}
	else
		CmdListOfs[NGLCMD_DETAIL] = 0;
	
	// Environment map pass.
	if ( ( MaterialFlags & NGLMAT_ENVIRONMENT_MAP ) && ( WaverenderDebug.DrawPassHighSpot ) )
	{
#ifdef PROJECT_KELLYSLATER
		CmdListOfs[NGLCMD_ENVIRONMENT] = CmdListDataPtr;
		nglVif1AddWaveHighlight( Packet, CmdListDataPtr, 0, Material, Material->Flags, true  );
		Info->EnvironmentKickAddr = CmdListDataPtr;
		if (LastBlendMode == Material->EnvironmentMapBlendMode) Options &= ~WAVERENDER_GIFTAGBLENDMODE;
		else Options |= WAVERENDER_GIFTAGBLENDMODE;
		WAVERENDER_Vif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_ENVIRONMENT, Options );
		Options &= ~WAVERENDER_GIFTAGOTHER;
		LastBlendMode = Material->EnvironmentMapBlendMode;
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
		if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
			nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
		nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
#endif
	}
	else
		CmdListOfs[NGLCMD_ENVIRONMENT] = 0;
	
	// Light map pass.
	if ( ( MaterialFlags & NGLMAT_LIGHT_MAP ) && ( WaverenderDebug.DrawPassHighCore ) )
	{
#ifdef PROJECT_KELLYSLATER
		CmdListOfs[NGLCMD_LIGHT] = CmdListDataPtr;
		nglVif1AddWaveHighlight( Packet, CmdListDataPtr, 1, Material, Material->Flags, true );
		Info->LightKickAddr = CmdListDataPtr;
		if (LastBlendMode == Material->LightMapBlendMode) Options &= ~WAVERENDER_GIFTAGBLENDMODE;
		else Options |= WAVERENDER_GIFTAGBLENDMODE;
		WAVERENDER_Vif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_LIGHT, Options );
		Options &= ~WAVERENDER_GIFTAGOTHER;
		LastBlendMode = Material->LightMapBlendMode;
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
		if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
			nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
		nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
#endif
	}
	else
		CmdListOfs[NGLCMD_LIGHT] = 0;
	
#ifdef PROJECT_KELLYSLATER
	u_int FoamKickAddr;  // used to be part of nglMaterialInfo, but we can make it local now.  (dc 06/05/02)
	if ( ( Material->UserFlags & KSMAT_FOAM_MAP ) && ( WaverenderDebug.DrawPassFoam ) )
	{
		CmdListOfs[NGLCMD_PROJECTEDTEXTURES] = CmdListDataPtr;
		nglVif1AddWaveFoam( Packet, CmdListDataPtr, Material, Material->Flags, true );
		FoamKickAddr = CmdListDataPtr;
		if (LastBlendMode == (int) Material->Pad[0]) Options &= ~WAVERENDER_GIFTAGBLENDMODE;
		else Options |= WAVERENDER_GIFTAGBLENDMODE;
		WAVERENDER_Vif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_FOAM, Options );
		Options &= ~WAVERENDER_GIFTAGOTHER;
		LastBlendMode = Material->Pad[0];
		nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
		if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
			nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
		nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
	}
	else
#endif
		CmdListOfs[NGLCMD_PROJECTEDTEXTURES] = 0;
	
#ifdef UNUSED	
#if 0
	if ( Material->Flags & NGLMAT_FOG )
	{
		CmdListOfs[NGLCMD_FOG] = CmdListDataPtr;
		nglVif1AddFogSetup( Packet, CmdListDataPtr );
		nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
	}
	else
#endif
#endif
		CmdListOfs[NGLCMD_FOG] = 0;
	
	
	Info->CmdListEndAddr = CmdListDataPtr;
	
	// Unpack the array of command list pointers.
	nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS, SCE_VIF1_S_8, NGLCMD_NUM_LISTS, CmdListOfs, NGLCMD_NUM_LISTS );
	
	// Align to 16 byte boundary.
	while ( ( (u_int)Packet ) & 15 ) *(Packet++) = SCE_VIF1_SET_NOP( 0 );
	
#ifdef UNUSED	// Unpack the bones and the matrix.
	if ( MeshNode->Params.Flags & NGLP_BONES_MASK )
		nglVif1UnpackBones( Packet, Mesh, MeshNode->Params.Flags, MeshNode->Params.Bones, (nglMatrix*)&MeshNode->WorldToLocal );
#endif
	
	// Set up the local -> world matrix in VU mem.
	if ( MeshNode->Params.Flags & NGLP_IDENTITY_MATRIX )
		nglVif1AddCallProgram( Packet, NGL_PROG_ADDR( nglLoadIdentityAddr, nglBaseAddr ) );
	else
		nglVif1UnpackMatrix( Packet, NGLMEM_LOCAL_TO_WORLD, (nglMatrix*)&MeshNode->LocalToWorld );
	
	// Build the list of lights local to this mesh.  Also adds texture entries to the array for any projector lights.
	if ( MeshNode->Params.Flags & NGLP_TEXTURE_FRAME )
		nglTextureAnimFrame = MeshNode->Params.TextureFrame;
	else
		nglTextureAnimFrame = nglCurScene->IFLFrame;
	
	// Lighting (build vertex and texture light command lists).
	if ( MaterialFlags & NGLMAT_LIGHT )
	{
		u_int CmdListDataPtr = Info->CmdListEndAddr;
#ifdef UNUSED
		if ( !nglCurLightContext->LocalVertexListEmpty && !( MeshNode->Params.Flags & NGLP_NO_LIGHTING ) )
		{
			nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_VERTEX_LIGHT, SCE_VIF1_S_32, 1, &CmdListDataPtr, sizeof(u_int) );
			nglVif1AddVertexLightSetup( Packet, CmdListDataPtr, Section, Mesh, MeshNode );
			nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
		}
		else
		{
			u_int Val = 0;
			nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_VERTEX_LIGHT, SCE_VIF1_S_32, 1, &Val, sizeof(u_int) );
		}
#endif
		
		if ( !nglCurLightContext->LocalTextureListEmpty )
		{
#ifdef PROJECT_KELLYSLATER
			nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_FOG, SCE_VIF1_S_32, 1, &CmdListDataPtr, sizeof(u_int) );
#else
			nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_PROJECTEDTEXTURES, SCE_VIF1_S_32, 1, &CmdListDataPtr, sizeof(u_int) );
#endif
			nglVif1AddTextureLightSetup( Packet, CmdListDataPtr, Material, Mesh, MeshNode, MaterialFlags );
			nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
		}
		else
		{
			u_int Val = 0;
#ifdef PROJECT_KELLYSLATER
			nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_FOG, SCE_VIF1_S_32, 1, &Val, sizeof(u_int) );
#else
			nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_PROJECTEDTEXTURES, SCE_VIF1_S_32, 1, &Val, sizeof(u_int) );
#endif
		}
	}
	
	assert( CmdListDataPtr < NGLMEM_COMMAND_LIST_END );	// Command list overflow check.  (dc 06/04/02)
	
	// Set the diffuse pass flags.
	register u_int Flags = 0;
	if ( MeshNode->Clip )
	{
		Flags |= NGLVU_CLIP;
		if ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP && STAGE_ENABLE( PlaneClip ) )
			Flags |= NGLVU_CLIP_PERFECT;
	}
#ifdef UNUSED
	if ( MaterialFlags & NGLMAT_BACKFACE_CULL )
		Flags |= NGLVU_BACKFACE;
	
	if ( ( MeshNode->Params.Flags & NGLP_TEXTURE_SCROLL ) || ( MaterialFlags & NGLMAT_UV_SCROLL ) )
	{
		Flags |= NGLVU_TEXTURE_SCROLL;
		float Scroll[2] = { 0.0f, 0.0f };
		if ( MeshNode->Params.Flags & NGLP_TEXTURE_SCROLL )
		{
			Scroll[0] += MeshNode->Params.ScrollU;
			Scroll[1] += MeshNode->Params.ScrollV;
		}
		if ( MaterialFlags & NGLMAT_UV_SCROLL )
		{
			Scroll[0] += nglCurScene->AnimTime * Material->ScrollU;
			Scroll[1] += nglCurScene->AnimTime * Material->ScrollV;
		}
		Scroll[0] -= floorf( Scroll[0] );
		Scroll[1] -= floorf( Scroll[1] );
		nglVif1AddUnpack( Packet, NGLMEM_TEXTURE_SCROLL, SCE_VIF1_V2_32, 1, &Scroll, sizeof(float) * 2 );
	}
	
	if ( MeshNode->Params.Flags & NGLP_TINT )
	{
		Flags |= NGLVU_TINT;
		nglVif1AddUnpack( Packet, NGLMEM_TINT_COLOR, SCE_VIF1_V4_32, 1, MeshNode->Params.TintColor, sizeof(nglVector) );
		// If tinting the alpha of a material that is opaque or punchthrough, temporarily switch all the layers to 'blend'.
		if ( MeshNode->Params.TintColor[3] != 1.0f )
		{
			if ( Material->MapBlendMode != NGLBM_ADDITIVE && Material->MapBlendMode != NGLBM_BLEND )
			{
				Flags |= NGLVU_BACKFACE;
				if ( Info->DiffuseKickAddr )
					nglVif1AddUnpack( Packet, Info->DiffuseKickAddr + 2, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 2, (void*)nglGSBlendModes[NGLBM_BLEND], sizeof(u_long) * 2 );
				if ( Info->DetailKickAddr )
					nglVif1AddUnpack( Packet, Info->DetailKickAddr + 2, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 2, (void*)nglGSBlendModes[NGLBM_BLEND], sizeof(u_long) * 2 );
				if ( Info->EnvironmentKickAddr )
					nglVif1AddUnpack( Packet, Info->EnvironmentKickAddr + 2, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 2, (void*)nglGSBlendModes[NGLBM_BLEND], sizeof(u_long) * 2 );
				if ( Info->LightKickAddr )
					nglVif1AddUnpack( Packet, Info->LightKickAddr + 2, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 2, (void*)nglGSBlendModes[NGLBM_BLEND], sizeof(u_long) * 2 );
#ifdef PROJECT_KELLYSLATER
				if ( FoamKickAddr )
					nglVif1AddUnpack( Packet, FoamKickAddr + 2, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 2, (void*)nglGSBlendModes[NGLBM_BLEND], sizeof(u_long) * 2 );
#endif
			}
		}
	}
#endif
	
	nglVif1AddUnpack( Packet, NGLMEM_FLAGS, SCE_VIF1_S_32, 1, &Flags, sizeof(u_int) );
	
	// Start the chain.  Each MSCNT after nglNodeSetupAddr calls nglCommandListAddr.
	nglVif1AddCallProgram( Packet, NGL_PROG_ADDR( nglNodeSetupAddr, nglBaseAddr ) );
	
	nglDmaEndTag( Packet, SCE_DMA_ID_CNT );
	nglDmaAddLargeRef( Packet, Section->BatchDMA, Section->BatchQWC );
	
	nglVif1FlushSPAD( Packet );
	
#if NGL_DEBUGGING
	nglPerfInfo.TotalPolys += Section->NVerts - Section->NStrips * 2;
	nglPerfInfo.TotalVerts += Section->NVerts;
#endif
	
	_Packet = Packet;
	STOP_PROF_TIMER( proftimer_send_mesh_sections );
}
