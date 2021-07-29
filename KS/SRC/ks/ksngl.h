// Stuff which used to be in NGL but got removed + Stuff that we wish was in NGL but isn't
#ifndef _KSNGL_H
#define _KSNGL_H

#ifndef TARGET_XBOX
#define NGLCLEAR_STENCIL 0
#endif

#ifdef TARGET_PS2
#define NGLP_REVERSE_BACKFACECULL 0
#endif

#ifdef TARGET_PS2
#elif defined (TARGET_GC)
void nglListAddScratchWave( u_int ID, const nglMatrix LocalToWorld, nglRenderParams* Params );
#endif
//void nglSetCurrentCamera( int cam );

#if NGL > 0x010700
void ksnglSetViewport( float x1, float y1, float x2, float y2 );
#else
void ksnglSetViewport( u_int x1, u_int y1, u_int x2, u_int y2 );
#endif

void ksnglSetPerspectiveMatrix( float hfovDeg, float cx, float cy, float nearz, float farz);
void ksnglSetOrthoMatrix( float cx, float cy, float nearz, float farz, float zmin = 0.0f, float zmax = 1.0f );

#ifdef TARGET_GC
static inline void nglSetMeshFlags( nglMesh *Mesh, u_int flags ) { Mesh->Flags = flags | NGLMESH_SCRATCH ; }
#endif


void ksnglSceneParamsFromParent( void );



inline void KSNGL_TransposeMatrix(nglMatrix &DstMat, const nglMatrix &SrcMat)
{
#ifdef TARGET_PS2
	sceVu0TransposeMatrix(*(sceVu0FMATRIX *) &DstMat, *(sceVu0FMATRIX *) &SrcMat);	// cast away const
#elif defined (TARGET_XBOX)
	XGMatrixTranspose((XGMATRIX *) &DstMat, (XGMATRIX *) &SrcMat);
#else	// TARGET_GC
	assert( &DstMat != &SrcMat );
	#ifdef NGL_FAST_MATH
	MTX44Transpose( SrcMat, DstMat );
	#else
	DstMat[0][0] = SrcMat[0][0];
	DstMat[0][1] = SrcMat[1][0];
	DstMat[0][2] = SrcMat[2][0];
	DstMat[0][3] = SrcMat[3][0];

	DstMat[1][0] = SrcMat[0][1];
	DstMat[1][1] = SrcMat[1][1];
	DstMat[1][2] = SrcMat[2][1];
	DstMat[1][3] = SrcMat[3][1];

	DstMat[2][0] = SrcMat[0][2];
	DstMat[2][1] = SrcMat[1][2];
	DstMat[2][2] = SrcMat[2][2];
	DstMat[2][3] = SrcMat[3][2];

	DstMat[3][0] = SrcMat[0][3];
	DstMat[3][1] = SrcMat[1][3];
	DstMat[3][2] = SrcMat[2][3];
	DstMat[3][3] = SrcMat[3][3];
	#endif
#endif
}

inline void KSNGL_RotateMatrix(nglMatrix &Dest, const nglMatrix &Src, const nglVector &Rot)
{
#ifdef TARGET_PS2
	sceVu0RotMatrix(*(sceVu0FMATRIX *) &Dest, *(sceVu0FMATRIX *) &Src, *(sceVu0FVECTOR *) &Rot);	// cast away const
#else
	nglMatrixRot(Dest, Src, Rot);
#endif
}

inline void KSNGL_TranslateMatrix(nglMatrix &Dest, const nglMatrix &Src, const nglVector &Offset)
{
#ifdef TARGET_PS2
	sceVu0TransMatrix(*(sceVu0FMATRIX *) &Dest, *(sceVu0FMATRIX *) &Src, *(sceVu0FVECTOR *) &Offset);	// cast away const
#elif defined (TARGET_XBOX)
	nglMatrixTrans(Dest, Src, Offset);
#else	// TARGET_GC
	Dest[3][0] = Src[3][0] + Offset[0];
	Dest[3][1] = Src[3][1] + Offset[1];
	Dest[3][2] = Src[3][2] + Offset[2];
	Dest[3][3] = Src[3][3] + Offset[3];
#endif
}

inline void KSNGL_Normalize( float *n )
{
#ifdef TARGET_PS2
#ifdef DEBUG	// This is a time-critical functions (dc 05/31/02)
	assert (!(((int) n) & 0xf));	// must be 16-byte aligned for PS2 version (dc 05/31/02)
#endif
	sceVu0Normalize(*(sceVu0FVECTOR *) n, *(sceVu0FVECTOR *) n);
#else	// TARGET_GC or TARGET_XBOX
	((vector3d *) n)->normalize();
#endif
}

#ifdef TARGET_PS2
u_int KSNGL_GetTexel( nglTexture* Tex, int x, int y );
void KSNGL_ReInit();
#endif

#ifdef TARGET_XBOX
#define nglMeshFastWriteVertexPCUV  nglMeshWriteVertexPCUV
#define nglMeshFastWriteVertexPNCUV nglMeshWriteVertexPNCUV
#define nglMeshFastWriteVertexPC    nglMeshWriteVertexPC
#define nglMeshFastWriteVertexPNC   nglMeshWriteVertexPNC
#endif

#ifndef NGLFONT_TOKEN_COLOR
#define NGLFONT_TOKEN_COLOR    "\1"
#endif

#ifndef NGLFONT_TOKEN_SCALE
#define NGLFONT_TOKEN_SCALE    "\2"
#endif

#ifndef NGLFONT_TOKEN_SCALEXY
#define NGLFONT_TOKEN_SCALEXY  "\3"
#endif

// Used to be in NGL; now just convenient wrappers.  (dc 05/30/02)
void KSNGL_CreateScratchMesh( int NVerts, nglMaterial *Material, bool Locked = 0, bool HasShadow=false );
void KSNGL_ScratchSetMaterial( nglMaterial* Material );

/*	Replaced by new API. (dc 05/30/02)
void KSNGL_SetFont( int idx );
void KSNGL_SetFontColor( u_int c );
void KSNGL_SetFontZ( float z );
void KSNGL_SetFontScale( float xscale, float yscale );
*/

#if NGL > 0x010700

enum
{
	NGLNODE_PARTICLE = 6
};

/*---------------------------------------------------------------------------------------------------------
  Particle System API
---------------------------------------------------------------------------------------------------------*/
// Particle system parameters.
struct nglParticleSystem
{
  u_int MaterialFlags;
  u_int BlendMode;

  u_int	Num;			              // Number of Particles
  u_int	Scol;			              // Starting Color
  u_int	Rcol;			              // Random Starting Color
  u_int	Ecol;			              // Ending Color
  float	Life;			              // Particle Lifetime
  float	Rlife;		              // Random Particle Lifetime
  float	Dura;			              // Duration for all particles to fire
  float	Ctime;		              // Current Time
  float	Ssize;                  // Starting Size
  float	Rsize;		              // Random Starting Size Modifier
  float	Esize;		              // Ending Size Modifier
  float MaxSize;                // Maximum Size in Screen Space
  float Aspect;                 // Aspect Ratio

  u_int	Seed;			              // Random Seed
  nglVector Spos;               // Starting Position
  nglVector Rpos1;              // Random Starting Line 1 Modifier
  nglVector Rpos2;              // Random Starting Line 2 Modifier
  nglVector Svel;	              // Starting Velocity
  nglVector Rvel1;	            // Random Starting Velocity Line 1 Modifier
  nglVector Rvel2;	            // Random Starting Velocity Line 2 Modifier
  nglVector Rvel3;	            // Random Starting Velocity Line 3 Modifier
  nglVector Force;	            // Constant force ( acceleration )

  nglTexture* Tex;              // Texture map.
};

void nglListAddParticle( nglParticleSystem* Particle );
#endif //NGL > 0x010700

#endif //_KSNGL_H
