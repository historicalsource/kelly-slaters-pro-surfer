/*---------------------------------------------------------------------------------------------------------
  NGL_GCN_INTERNAL.H - Low level access to the NGL module for writing Custom Nodes and Extensions.

  Maintained by Michael Montague (mikem@treyarch.com)
---------------------------------------------------------------------------------------------------------*/
#ifndef NGL_GCN_INTERNAL_H
#define NGL_GCN_INTERNAL_H

/*---------------------------------------------------------------------------------------------------------
  System headers.
---------------------------------------------------------------------------------------------------------*/

#include <dolphin/mtx.h>

// Just in case this wasn't included already, we don't want compiler errors.
#include "ngl_gc.h"

/*---------------------------------------------------------------------------------------------------------
  Internal defines

  Define these on the command line using -DNGL_XXXX or -DNGL_XXXX=0, to configure NGL for your project.
---------------------------------------------------------------------------------------------------------*/
// Defining NGL_FINAL removes all print statements, NGL_ASSERT calls, and lots of debugging and profiling
// code.  If you want to leave the profiling or debugging code in, you can control those manually too.
#ifdef NGL_FINAL
#undef NGL_PROFILING
#undef NGL_DEBUGGING
#define NGL_PROFILING 0
#define NGL_DEBUGGING 0
#endif

// Controls removing nglPrintf statements, NGL_ASSERT statements and debug code.
#ifndef NGL_DEBUGGING
#define NGL_DEBUGGING 1
#endif

// Controls whether or not NGL times it's internal functions or not.  Some things like FPS are calculated
// regardless of this flag.
#ifndef NGL_PROFILING
#define NGL_PROFILING 1
#endif

// Global effects of the profiling and debugging flags.
#if NGL_DEBUGGING == 0
#define STAGE_ENABLE( x ) 1
#define DEBUG_ENABLE( x ) 0
#else
#define STAGE_ENABLE( x ) ( nglStage.x )
#define DEBUG_ENABLE( x ) ( nglSyncDebug.x )
#endif

#if NGL_PROFILING == 0
#undef START_PROF_TIMER
#undef STOP_PROF_TIMER
#define START_PROF_TIMER(x)
#define STOP_PROF_TIMER(x)
#endif

#ifdef NGL_DEBUG
#define NGL_ASSERT(x,s) 	\
	if ( !(x) )             \
  	{	                    \
    	nglWarning( TTY_BLACK "Assertion failed in %s(%d):\n" TTY_RED "\"%s\" - %s\n", __FILE__, __LINE__, #x, s );  \
    	asm( trap );     	\
  	}
#else
#define NGL_ASSERT(x,s) ( (void)0 )
#endif

/*---------------------------------------------------------------------------------------------------------
  NGL internal constants.
---------------------------------------------------------------------------------------------------------*/

#ifdef NGL_GC_SN_DEBUGGER
	// String constants for TTY color control codes.
	#define TTY_BLACK   "\x1b\x1e"
	#define TTY_RED     "\x1b\x1f"
	#define TTY_GREEN   "\x1b\x20"
	#define TTY_YELLOW  "\x1b\x21"
	#define TTY_BLUE    "\x1b\x22"
	#define TTY_MAGENTA "\x1b\x23"
	#define TTY_CYAN    "\x1b\x24"
	#define TTY_LTGREY  "\x1b\x25\x25"

	#define TTY_BG_BLACK    "\x1b\x28"
	#define TTY_BG_RED      "\x1b\x29"
	#define TTY_BG_GREEN    "\x1b\x2A"
	#define TTY_BG_DKYELLOW "\x1b\x2B"
	#define TTY_BG_BLUE     "\x1b\x2C"
	#define TTY_BG_MAGENTA  "\x1b\x2D"
	#define TTY_BG_CYAN     "\x1b\x2E"
	#define TTY_BG_BKGRND   "\x1b\x2F"
#else // METROWERKS
	// String constants for TTY color control codes.
	#define TTY_BLACK   	""
	#define TTY_RED     	""
	#define TTY_GREEN   	""
	#define TTY_YELLOW  	""
	#define TTY_BLUE    	""
	#define TTY_MAGENTA 	""
	#define TTY_CYAN    	""
	#define TTY_LTGREY  	""

	#define TTY_BG_BLACK    ""
	#define TTY_BG_RED      ""
	#define TTY_BG_GREEN    ""
	#define TTY_BG_DKYELLOW ""
	#define TTY_BG_BLUE     ""
	#define TTY_BG_MAGENTA  ""
	#define TTY_BG_CYAN     ""
	#define TTY_BG_BKGRND   ""
#endif

/*---------------------------------------------------------------------------------------------------------
  Debugging structs/flags
---------------------------------------------------------------------------------------------------------*/

// Global stage flags.
struct nglStageStruct
{
  	u_char BackfaceNI;
  	u_char PlaneClipNI;
  	u_char Clip;

  	u_char XFormNI;
	u_char Skin;
  	u_char Light;
	u_char LightAmb;	// implies tint too

  	u_char EnvironmentPassNI;
  	u_char DetailPassNI;
  	u_char LightPassNI;

  	u_char Scratch;
  	u_char QuadsNI;
};
extern nglStageStruct nglStage;

// Various debugging flags.
struct nglDebugStruct
{
  	// Show performance info (FPS/etc)
  	u_char ShowPerfInfo;
  	u_char ShowPerfGraph;
  	u_char ScreenShot;
	u_char RenderTargetShot;

  	u_char DisableQuadsNI;
  	u_char DisableTexturesNI;
  	u_char DisableMipmapsNI;
  	u_char DisableVSyncNI;
  	u_char DisableScratchNI;

  	// Enable printf's throughout the system.
  	u_char DebugPrintsNI;
  	u_char DumpFrameLogNI;
  	u_char DumpSceneFileNI;

  	// Debugging geometry.
  	u_char DrawLights;
  	u_char DrawLightNearRanges;
  	u_char DrawLightFarRanges;
  	u_char DrawMeshSpheres;
  	u_char DrawNormalsNI;

  	// Clipping.
  	u_char ForceNoClip;
  	u_char ForceClipNI;

  	// Draw points only (for VU speed testing).
  	u_char DrawAsPointsNI;
  	u_char DrawAsLines;

  	u_char DrawToFrontBufferNI;

  	u_char TestQuadZNI;
  	u_char TestQuadZValueNI;

  	// scratch mesh api
  	u_char NoScratchMeshNI;
};
extern nglDebugStruct nglDebug;
extern nglDebugStruct nglSyncDebug;  // Updated synchronously once per frame.

struct nglPerfInfoStruct
{
  	// Curremt frame rate.
  	float FPS;

  	// Millisecond counters.
  	float RenderMS;                     // Milliseconds taken by the GX pipeline
  	float ListSendMS;                   // Milliseconds taken by nglListSend.  (CPU time)

  	// Cycle counters.
  	OSTick RenderCycles;				// Cycles taken from the start and end of rendering (includes ListSendCycles)
  	OSTick ListSendCycles;              // Cycles taken by nglListSend this frame.

  	// Misc counters.
  	u_int TotalPolys;                   // Triangles rendered this frame.
  	u_int TotalVerts;                   // Vertices rendered this frame.

  	u_int TextureDataStreamed;          // Bytes of texture data streamed to the GS this frame.

  	u_int ListWorkBytesUsed;            // Bytes of nglListWork used this frame.
	u_int MaxListWorkBytesUsed;			// Maximum number of ListWork used
};
extern nglPerfInfoStruct nglPerfInfo;

#define NGLGC_SCALE_UV 512.0f
#define NGLGC_SCALE_NRM 16384.0f

#define NGL_CPU_CLOCKS_PER_MS	(((float)OS_TIMER_CLOCK / 1000.0f))

extern nglMeshSection* nglScratchSection;

static NGL_INLINE int nglFTOI( float f )  // doesn't round according to C standard
{
	int n;

  f += 3 << 22;
  n = ( ( *( (int*) &f ) ) & 0x007fffff ) - 0x00400000;

  return n;
}

static NGL_INLINE void nglToMtx44( Mtx44 dst, const nglMatrix src )
{
	dst[0][0] = src[0][0];
	dst[0][1] = src[1][0];
	dst[0][2] = src[2][0];
	dst[0][3] = src[3][0];

	dst[1][0] = src[0][1];
	dst[1][1] = src[1][1];
	dst[1][2] = src[2][1];
	dst[1][3] = src[3][1];

	dst[2][0] = src[0][2];
	dst[2][1] = src[1][2];
	dst[2][2] = src[2][2];
	dst[2][3] = src[3][2];

	dst[3][0] = src[0][3];
	dst[3][1] = src[1][3];
	dst[3][2] = src[2][3];
	dst[3][3] = src[3][3];
}

static NGL_INLINE void nglToMtx(register Mtx dst, const register nglMatrix& src)
{
	register f32 pair1, pair2, pair3, pair4;
	register f32 temp1, temp2;

//	NGL_ASSERT((void*)src == (void*)dst, "Src/Dst are the same!");

	asm
	{
		psq_l		pair1, 0(src),  0, 0		// src[0][0], src[0][1]
		psq_l		pair2, 16(src), 0, 0		// src[1][0], src[1][1]
		psq_l		pair3, 32(src), 0, 0		// src[2][0], src[2][1]
		ps_merge00	temp1, pair1, pair2;		// temp1 = src[0][0], src[1][0]
		psq_l		pair4, 48(src), 0, 0		// src[3][0], src[3][1]
		ps_merge11  temp2, pair1, pair2;		// temp2 = src[0][1], src[1][1]
		psq_st		temp1, 0(dst),  0, 0		// write to dest[0][0]. dest[0][1]
		psq_st		temp2, 16(dst), 0, 0		// write to dest[1][0], dest[1][1]
		ps_merge00  temp1, pair3, pair4;		// temp1 = src[2][0], src[2][0]
		ps_merge11  temp2, pair3, pair4;		// temp2 = src[3][1], src[3][1]
		psq_l		pair1, 8(src),  0, 0		// src[0][2], src[0][3]
		psq_st		temp1, 8(dst),  0, 0		// write to dest[0][2], dest[0][3]
		psq_st		temp2, 24(dst), 0, 0		// write to dest[1][2], dest[1][3]
		psq_l		pair2, 24(src), 0, 0		// src[1][2], src[1][3]
		psq_l		pair3, 40(src), 0, 0		// src[2][2], src[2][3]
		ps_merge00	temp1, pair1, pair2			// temp1 = src[0][2], src[1][2]
		psq_l		pair4, 56(src), 0, 0		// src[3][2], src[3][3]
		psq_st		temp1, 32(dst), 0, 0		// write to dest[2][0], dest[2][1]
		ps_merge00 	temp2, pair3, pair4			// temp3 = src[2][2], src[3][2]
		psq_st		temp2, 40(dst), 0, 0		// write to dest[2][2], dest[2][3]
	}
}

static NGL_INLINE void nglCopyMatrix(register nglMatrix& dst, const register nglMatrix& src)
{
	register f32 row01, row23;

	asm
	{
		psq_l		row01,  0(src), 0, 0		// read row 0
		psq_l		row23,  8(src), 0, 0
		psq_st		row01,  0(dst), 0, 0		// write row 0
		psq_st		row23,  8(dst), 0, 0
		psq_l		row01, 16(src), 0, 0		// read row 1
		psq_l		row23, 24(src), 0, 0
		psq_st		row01, 16(dst), 0, 0		// write row 1
		psq_st		row23, 24(dst), 0, 0
		psq_l		row01, 32(src), 0, 0		// read row 2
		psq_l		row23, 40(src), 0, 0
		psq_st		row01, 32(dst), 0, 0		// write row 2
		psq_st		row23, 40(dst), 0, 0
		psq_l		row01, 48(src), 0, 0		// read row 3
		psq_l		row23, 56(src), 0, 0
		psq_st		row01, 48(dst), 0, 0		// write row 3
		psq_st		row23, 56(dst), 0, 0
	}
}

// load half-word byte reversed (16bit)
static NGL_INLINE short _lhbrx(register short* val)
{
	register short rval;

	asm
	{
		lhbrx rval, 0, val;
	}

	return rval;
}

// load half-word byte reversed (16bit)
static NGL_INLINE unsigned short _lhbrx(register unsigned short* val)
{
	register unsigned short rval;

	asm
	{
		lhbrx rval, 0, val;
	}

	return rval;
}

struct nglVertex2
{
	u_int color;
	short u1;
	short v1;
	short u2;
	short v2;
};

struct nglVertex
{
	float x;
	float y;
	float z;
	short nx;
	short ny;
	short nz;
	short Pad1;
	u_int color;
	short u1;
	short v1;
	short u2;
	short v2;
};

/*---------------------------------------------------------------------------------------------------------
  GCNMESH file stuff.
---------------------------------------------------------------------------------------------------------*/
#define NGL_MESHFILE_VERSION  0x607
#define NGL_MAX_PATH_LENGTH   256

// Mesh File structure, these are contained in nglMeshFileBank.
struct nglMeshFile
{
  nglFixedString FileName;
  char FilePath[NGL_MAX_PATH_LENGTH];

  nglFileBuf FileBuf;
  bool LoadedInPlace;

  nglMesh* FirstMesh;
};
//extern nglInstanceBank nglMeshFileBank;

// Batch Info structure.  The nglMeshSection structure has an array of these, one for each batch.  They can be used
// (and are used by the Mesh Editing API) to access the vertex data of a mesh.
struct nglMeshBatchInfo
{
  // Basic statistical/processing information.
  u_int NVerts;

  // Vertex data pointers.
  u_int* StripData;
  float* PosData;
  float* UVData;
  u_int* ColorData;
  float* NormData;
  float* LightUVData;
  char* BoneCountData;
  unsigned short* BoneIdxData;
  float* BoneWeightData;
};

/*---------------------------------------------------------------------------------------------------------
  Edit mesh functions inline implementation.  Optimized for cycle count. (dc 12/14/01)
---------------------------------------------------------------------------------------------------------*/
// Current in-progress scratch mesh.
extern nglMesh* nglScratch;
extern nglMeshBatchInfo nglScratchBatch;
extern u_int nglScratchBatchIdx;
extern u_int nglScratchVertIdx;
extern u_int nglScratchStripVertIdx;

// Necessary scratch mesh functions
void nglMeshNextBatch();

static NGL_INLINE void nglMeshFastWriteVertexPC( float X, float Y, float Z, u_int Color )
{
  	register nglVertex* const Vert = ((nglVertex*)nglScratchSection->Verts) + nglScratchVertIdx;

	Vert->x = X;
	Vert->y = Y;
	Vert->z = Z;

	Vert->color = Color;

  	nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPCUV( float X, float Y, float Z, u_int Color, float U, float V )
{
  	register nglVertex* const Vert = ((nglVertex*)nglScratchSection->Verts) + nglScratchVertIdx;
	float u, v;

	Vert->x = X;
	Vert->y = Y;
	Vert->z = Z;

	Vert->color = Color;

	u = U * NGLGC_SCALE_UV;
	v = V * NGLGC_SCALE_UV;

	OSf32tos16(&u, &Vert->u1);
	OSf32tos16(&v, &Vert->v1);

	nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPCUVB( float X, float Y, float Z, u_int Color, float U, float V,
							  int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
//  ColorData[0] = Color;
//
//  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;
//
//  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;
//
//  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
//  BoneWeightData[0] = w1;
//  BoneWeightData[1] = w2;
//  BoneWeightData[2] = w3;
//  BoneWeightData[3] = w4;
//
//  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
//  BoneIdxData[0] = b1*4 + BonePos;
//  BoneIdxData[1] = b2*4 + BonePos;
//  BoneIdxData[2] = b3*4 + BonePos;
//  BoneIdxData[3] = b4*4 + BonePos;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPCUV2( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2 )
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
//  LightUVData[0] = U2;
//  LightUVData[1] = V2;
//
//  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
//  ColorData[0] = Color;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPCUV2B( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2,
							   int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
//  LightUVData[0] = U2;
//  LightUVData[1] = V2;
//
//  register u_int * const ColorData = (u_int *)(((u_int)nglScratchBatch.ColorData)+idx1i);
//  ColorData[0] = Color;
//
//  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;
//
//  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;
//
//  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
//  BoneWeightData[0] = w1;
//  BoneWeightData[1] = w2;
//  BoneWeightData[2] = w3;
//  BoneWeightData[3] = w4;
//
//  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
//  BoneIdxData[0] = b1*4 + BonePos;
//  BoneIdxData[1] = b2*4 + BonePos;
//  BoneIdxData[2] = b3*4 + BonePos;
//  BoneIdxData[3] = b4*4 + BonePos;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPUV( float X, float Y, float Z, float U, float V )
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPN( float X, float Y, float Z, float NX, float NY, float NZ )
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
//  NormData[0] = NX;
//  NormData[1] = NY;
//  NormData[2] = NZ;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPNC( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color )
{
  	register nglVertex* const Vert = ((nglVertex*)nglScratchSection->Verts) + nglScratchVertIdx;
	float nx, ny, nz;

	Vert->x = X;
	Vert->y = Y;
	Vert->z = Z;

	Vert->color = Color;

  	nx = NX * NGLGC_SCALE_NRM;
  	ny = NY * NGLGC_SCALE_NRM;
  	nz = NZ * NGLGC_SCALE_NRM;

	OSf32tos16(&nx, &Vert->nx);
	OSf32tos16(&ny, &Vert->ny);
	OSf32tos16(&nz, &Vert->nz);

  	nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPNCUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, float U, float V )
{
  	register nglVertex* const Vert = ((nglVertex*)nglScratchSection->Verts) + nglScratchVertIdx;
	float u,v;

	Vert->x = X;
	Vert->y = Y;
	Vert->z = Z;

	Vert->color = Color;

	u = U * NGLGC_SCALE_UV;
	v = V * NGLGC_SCALE_UV;

	OSf32tos16(&u, &Vert->u1);
	OSf32tos16(&v, &Vert->v1);

	Vert->nx = (short)(NX * NGLGC_SCALE_NRM);
  	Vert->ny = (short)(NY * NGLGC_SCALE_NRM);
  	Vert->nz = (short)(NZ * NGLGC_SCALE_NRM);

  	nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPNUV( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V )
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
//  NormData[0] = NX;
//  NormData[1] = NY;
//  NormData[2] = NZ;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPNUVB( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V,
							  int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
//  NormData[0] = NX;
//  NormData[1] = NY;
//  NormData[2] = NZ;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;
//
//  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;
//
//  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
//  BoneWeightData[0] = w1;
//  BoneWeightData[1] = w2;
//  BoneWeightData[2] = w3;
//  BoneWeightData[3] = w4;
//
//  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
//  BoneIdxData[0] = b1*4 + BonePos;
//  BoneIdxData[1] = b2*4 + BonePos;
//  BoneIdxData[2] = b3*4 + BonePos;
//  BoneIdxData[3] = b4*4 + BonePos;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPNUV2( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2 )
{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
//  NormData[0] = NX;
//  NormData[1] = NY;
//  NormData[2] = NZ;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
//  LightUVData[0] = U2;
//  LightUVData[1] = V2;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

static NGL_INLINE void nglMeshFastWriteVertexPNUV2B( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2,
							   int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)

{
   // unimplemented
   NGL_ASSERT(false, "unimplemented");
//  if ( nglScratchVertIdx >= nglScratchBatch.NVerts )
//    nglMeshNextBatch();
//
//  register const u_int idx1i = nglScratchVertIdx * 1 * sizeof(u_int);
//
//  register const u_int idx4s = nglScratchVertIdx * 4 * sizeof(unsigned short);
//
//  register const u_int idx2f = nglScratchVertIdx * 2 * sizeof(float);
//  register const u_int idx3f = nglScratchVertIdx * 3 * sizeof(float);
//  register const u_int idx4f = nglScratchVertIdx * 4 * sizeof(float);
//
//  register u_int * const StripData = (u_int *)(((u_int)nglScratchBatch.StripData)+idx1i);
//  StripData[0] = nglScratchStripVertIdx >> 1 ? 0 : 0x8000;
//
//  register float * const PosData = (float *)(((u_int)nglScratchBatch.PosData)+idx3f);
//  PosData[0] = X;
//  PosData[1] = Y;
//  PosData[2] = Z;
//
//  register float * const NormData = (float *)(((u_int)nglScratchBatch.NormData)+idx3f);
//  NormData[0] = NX;
//  NormData[1] = NY;
//  NormData[2] = NZ;
//
//  register float * const UVData = (float *)(((u_int)nglScratchBatch.UVData)+idx2f);
//  UVData[0] = U;
//  UVData[1] = V;
//
//  register float * const LightUVData = (float *)(((u_int)nglScratchBatch.LightUVData)+idx2f);
//  LightUVData[0] = U2;
//  LightUVData[1] = V2;
//
//  register const u_int BonePos = NGLMEM_END - nglScratch->NBones * 4;
//
//  nglScratchBatch.BoneCountData[nglScratchVertIdx] = bones;
//
//  register float * const BoneWeightData = (float *)(((u_int)nglScratchBatch.BoneWeightData)+idx4f);
//  BoneWeightData[0] = w1;
//  BoneWeightData[1] = w2;
//  BoneWeightData[2] = w3;
//  BoneWeightData[3] = w4;
//
//  register unsigned short * const BoneIdxData = (unsigned short *)(((u_int)nglScratchBatch.BoneIdxData)+idx4s);
//  BoneIdxData[0] = b1*4 + BonePos;
//  BoneIdxData[1] = b2*4 + BonePos;
//  BoneIdxData[2] = b3*4 + BonePos;
//  BoneIdxData[3] = b4*4 + BonePos;
//
//  nglScratchStripVertIdx++;
//  nglScratchVertIdx++;
}

#endif	// #ifndef NGL_GCN_INTERNAL_H
