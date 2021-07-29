// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

/*
void nglSetCurrentCamera( int cam )
{
	currentcam=cam;
}

void WAVERENDER_SetWaveCameraPos( nglVector cam )
{
	CamPos[currentcam] = cam;
}
*/

float aspectratiox=2.0f;
float aspectratioy=3.0f;


#if NGL > 0x010700
void ksnglSetViewport( float x1, float y1, float x2, float y2 )
#else
void ksnglSetViewport( u_int x1, u_int y1, u_int x2, u_int y2 )
#endif
{
	#if 0 //NGL >= 0x010700
		// need to compensate for NGLs sudden failure to update aspect ratio in
		//   newer versions
		float xfactor=(1.0f+x2-x1); // aspectratiox; //* ((float)nglGetScreenHeight());
		float yfactor=(1.0f+y2-y1); // aspectratioy; //* ((float)nglGetScreenWidth());
		nglSetAspectRatio(xfactor/yfactor);
		float fov, nearz, farz;
		nglGetProjectionParams(&fov, &nearz, &farz);

		float nfov=90.0f*xfactor/nglGetScreenWidth();

		nglSetPerspectiveMatrix(nfov, nearz, farz);

	#endif
	nglSetViewport( x1, y1, x2, y2 );
}


void ksnglSetPerspectiveMatrix( float hfovDeg, float cx, float cy, float nearz, float farz)
{
	#if NGL >= 0x010700
		nglSetPerspectiveCXCY( cx, cy );
		nglSetPerspectiveMatrix(hfovDeg, nearz, farz);
	#else
		nglSetPerspectiveMatrix(hfovDeg, cx, cy, nearz, farz);
	#endif
}

void ksnglSetOrthoMatrix( float cx, float cy, float nearz, float farz, float zmin, float zmax )
{
	#if NGL >= 0x010700
		nglSetPerspectiveCXCY( cx, cy );
		nglSetOrthoMatrix(-1.0f, 1.0f, -1.0f, 1.0f, nearz, farz);
	#else
		nglSetOrthoMatrix( cx, cy, nearz, farz, zmin, zmax );
	#endif
}




#if 0
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
  int Type;

  // Global matrices.
  nglMatrix ViewToWorld;    // Camera -> World

  nglMatrix ViewToScreen;   // Camera -> Screen

  nglMatrix WorldToView;    // World -> Camera
  nglMatrix WorldToScreen;  // World -> Camera -> Screen

  nglVector ClipPlanes[NGLCLIP_MAX];     // Planes for sphere rejection.

  nglListBin RenderListBins[NGL_TOTAL_BINS];

	// Quads.
	int NumQuads;
	nglQuad* FirstQuad;
	nglQuad* LastQuad;

  // Viewport.
  float ViewX1, ViewY1, ViewX2, ViewY2;

  // Screen clear settings.
  u_int ClearFlags;
  float ClearZ;
  nglVector ClearColor;

  // Fog settings.
  float FogNear;
  float FogFar;
  float FogMin;
  float FogMax;
  int FogType;
  GXColor FogColor;

  // Saved parameters to nglSetPerspectiveMatrix.
  float HFOV;
  float CX;
  float CY;
  float NearZ;
  float FarZ;
	ViewTypes ViewType; // Perspective / Ortho / other.
	float QuadZCompParam[2]; // Used to compute the Z for the Quad.

  float ScrZ;
  float ZMin;
  float ZMax;

  bool ZWriteEnable;
  bool ZTestEnable;
};


#endif

extern nglScene *nglCurScene;

#ifdef TARGET_XBOX
#include "ngl_xbox_internal.h"	// nglScene is not publicly defined (dc 06/29/02)
#endif

void ksnglSceneParamsFromParent( void )
{
	nglScene *childscene=nglCurScene;
	nglScene *parentscene=childscene->Parent;

	nglSetRenderTarget( parentscene->RenderTarget );
	nglSetViewport( parentscene->ViewX1, parentscene->ViewY1, parentscene->ViewX2, parentscene->ViewY2 );

//	nglSetAspectRatio(4.0f/3.0f);
	#if NGL >= 0x010700
		nglSetPerspectiveMatrix(parentscene->HFOV, parentscene->NearZ, parentscene->FarZ);
	#else
		nglSetPerspectiveMatrix(parentscene->HFOV, parentscene->CX, parentscene->CY, parentscene->NearZ, parentscene->FarZ);
	#endif
	nglSetWorldToViewMatrix(parentscene->WorldToView);

	nglSetClearFlags( parentscene->ClearFlags );
//	nglSetClearColor( 0, 0, 0, 0 );
	nglSetClearZ( parentscene->ClearZ );

#if 0
	//	nglSetFBWriteMask( NGL_FBWRITE_RGBA );
	nglSetZWriteEnable( true );
	nglSetZTestEnable( true );

	nglEnableFog(false);
	nglSetFogRange(0.0f, 10000.0f, 0.0f, 1.0f);
	nglSetFogColor(1.0f, 1.0f, 1.0f);

	nglSetAnimTime(0.0f);
#endif
}


#ifdef TARGET_PS2
void KSNGL_ReInit()
{
	extern int nglVBlankInterrupt(int p);
	// Install the VBlank handler.
	if ( AddIntcHandler( INTC_VBLANK_S, nglVBlankInterrupt, 0 ) < 0 )
		nglFatal( "nglInit: Error adding VBlank Interrupt Handler!\n" );
	EnableIntc( INTC_VBLANK_S );
}

extern void* nglTim2GetClut( nglTexture *Tex );
extern void* nglTim2GetImage( nglTexture *Tex, int mipmap );
extern int nglTim2GetMipMapPictureSize(TIM2_PICTUREHEADER *ph, int mipmap, int *pWidth, int *pHeight);
inline u_int nglTim2GetClutTexel( nglTexture *Tex, int index )
{
	TIM2_PICTUREHEADER *ph=Tex->ph;
	unsigned char *pClut;

	pClut = (unsigned char*)nglTim2GetClut(Tex);

	if(pClut==NULL) {
		// no assumed level texture data
		return(0);
	}

	switch(ph->ClutType) {
    case TIM2_RGB16:
		return((pClut[index*2 + 1]<<8) | pClut[index*2]);

    case TIM2_RGB24:
		return((pClut[index*3 + 2]<<16) | (pClut[index*3 + 1]<<8) | pClut[index*3]);

    case TIM2_RGB32:
		return((pClut[index*4 + 3]<<24) | (pClut[index*4 + 2]<<16) | (pClut[index*4 + 1]<<8) | pClut[index*4]);
	}

	// illegal pixel format
	return(0);
}

inline u_int KSNGL_Tim2GetTexel( nglTexture *Tex, int mipmap, int x, int y )
{
	TIM2_PICTUREHEADER *ph=Tex->ph;
	unsigned char *pImage;
	int t;
	int w, h;

	pImage = (unsigned char*)nglTim2GetImage(Tex, mipmap);
	if(pImage==NULL) {
		// no assumed level texture data
		return(0);
	}
	nglTim2GetMipMapPictureSize(ph, mipmap, &w, &h);
	if(x>w || y>h) {
		// illegal texel cooridnates
		return(0);
	}

	int index=0;

	t = y*w + x;
	switch(ph->ImageType) {
    case TIM2_RGB16:
		return((pImage[t*2 + 1]<<8) | pImage[t*2]);

    case TIM2_RGB24:
		return((pImage[t*3 + 2]<<16) | (pImage[t*3 + 1]<<8) | pImage[t*3]);

    case TIM2_RGB32:
		return((pImage[t*4 + 3]<<24) | (pImage[t*4 + 2]<<16) | (pImage[t*4 + 1]<<8) | pImage[t*4]);

    case TIM2_IDTEX4:
		if(x & 1) {
			index=(pImage[t/2]>>4);
		} else {
			index=(pImage[t/2] & 0x0F);
		}
		return nglTim2GetClutTexel( Tex, index );
    case TIM2_IDTEX8:
		index=(pImage[t]);
		return nglTim2GetClutTexel( Tex, index );
	}

	// illegal pixel format
	return(0);
}

u_int KSNGL_GetTexel( nglTexture* Tex, int x, int y )
{
	if ( Tex->Type == NGLTEX_TGA )
	{
		if ( Tex->Format == SCE_GS_PSMCT32 )
			return Tex->Data[y * Tex->Width + x];
		else
			if ( Tex->Format == SCE_GS_PSMCT16 )
				return ( (u_short*)Tex->Data )[y * Tex->Width + x];
	}
	else
		if ( Tex->Type == NGLTEX_TIM2 )
			return KSNGL_Tim2GetTexel( Tex, 0, x, y );

		return 0;
}

#endif

// platform specific custom node code for the waves

#ifdef TARGET_GC
#include "ksngl_wave_gc.cpp"
#endif  // TARGET_PS2

// Used to be in NGL; now just convenient wrappers.  (dc 05/30/02)
void KSNGL_CreateScratchMesh( int NVerts, nglMaterial *Material, bool Locked, bool HasShadow )
{
	u_int flags=( (Locked ? 0 : NGLMESH_TEMP) | NGLMESH_PERFECT_TRICLIP );

	#ifdef TARGET_GC
		if ( 1 ) //HasShadow )
			flags |= NGLMESH_LIGHTCAT_8;
	#endif


#if NGL > 0x010700
	nglCreateMesh(flags, 1 );
#else
	nglCreateMesh(1);
	nglSetMeshFlags( flags );
#endif
	// FIXME: hardcoded values waste memory and cause random crashes ;) -- mdm
	nglMeshAddSection( Material, NVerts, 255 );
	if (Locked)
	{
		// Special case for water meshes, which are created before we're ready to fill them in.  (dc 06/11/02)
		// We should try to get rid of this.
		nglMesh* Mesh = nglCloseMesh();
		nglEditMesh( Mesh );
	}
}

/* WARNING:

	This function should only be used for custom nodes, not for plain old scratch meshes.
	It shouldn't be used at all if you can help it.

	Lots of stuff involving the material is cached, especially on PS2, so you can't count on
	sensible results from switching the material after mesh creation.  (dc 06/06/02)
*/
void KSNGL_ScratchSetMaterial( nglMaterial* Material )
{
#ifdef TARGET_PS2
//	assert(false);	// asking for trouble
#endif

	extern nglMeshSection *nglScratchSection;	// not publicly declared on PS2 (dc 05/31/02)
#ifdef TARGET_PS2
	// This member of nglMaterial must be kept filled with some address.
	// The data it contains is probably going to be incorrect though.  (dc 05/31/02)
	nglMaterialInfo *OldInfo = nglScratchSection->Material->Info;
#endif

	// Copy the material and correct some common material mistakes.
	memcpy( nglScratchSection->Material, Material, sizeof(nglMaterial) );

	if ( Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH )
		nglScratchSection->Material->Flags |= NGLMAT_ALPHA;

#ifdef TARGET_PS2
	if ( !( Material->Flags & NGL_TEXTURE_MASK ) || !Material->Map )
	{
		nglScratchSection->Material->Flags |= NGLMAT_TEXTURE_MAP;
		extern nglTexture *nglWhiteTex;	// not publicly declared on PS2 (dc 05/31/02)
		nglScratchSection->Material->Map = nglWhiteTex;
	}
#endif

#ifdef TARGET_PS2
	nglScratchSection->Material->Info = OldInfo;

/*
	// Recompute the data for the Material->Info.  (dc 05/31/02)
	if (Material->UserFlags & KSMAT_WAVETRANS)
	{
		WAVERENDER_BakeMaterial( nglScratch, nglScratchSection, true );
	}
	else
	{
extern void nglBakeMaterial( nglMesh* Mesh, nglMeshSection* Section, bool Scratch );
		nglBakeMaterial( nglScratch, nglScratchSection, true );
	}

	// Fill in the DMA in place, without changing the batchinfo pointers.
	// This is a blind write to existing memory, which may overflow.  (dc 06/01/02)
void nglCreateMeshDMA( u_int*& Packet, nglMesh* Mesh, nglMeshSection* Section );
	u_int *dummy = (u_int*) nglScratchSection->BatchInfo;
	u_int OldQWC = nglScratchSection->BatchQWC;
	nglCreateMeshDMA( dummy, nglScratch, nglScratchSection );
	assert (nglScratchSection->BatchQWC < 2 * OldQWC);
*/
#endif
}

