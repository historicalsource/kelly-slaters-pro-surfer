/*---------------------------------------------------------------------------------------------------------
  NGL_PS2.CPP - Playstation2 implementation of NGL.

  Maintained by Wade Brainerd (wade@treyarch.com), Mike Montague (mikem@treyarch.com), 
  Andy Chien (chien@treyarch.com).
---------------------------------------------------------------------------------------------------------*/

#include <algorithm>
#include <utility>
using namespace std;

/*---------------------------------------------------------------------------------------------------------
  Specifics for dealing with the Arch Engine.
---------------------------------------------------------------------------------------------------------*/
#ifdef ARCH_ENGINE
#include "global.h"
#include "profiler.h"
#else
#define START_PROF_TIMER(x)
#define STOP_PROF_TIMER(x)
#define ADD_PROF_TIMER(x,y)
#endif 

#undef BOOL

/*---------------------------------------------------------------------------------------------------------
  NGL standard includes.
---------------------------------------------------------------------------------------------------------*/
#include "ngl_ps2.h"
#include "ngl_ps2_internal.h"
#include "ngl_version.h"

// Standard resource includes.
#if NGL_DEBUGGING
#include "ngl_spheremesh.inc"
#include "ngl_radiusmesh.inc"
#endif
#include "ngl_sysfont_fdf.inc"
#include "ngl_sysfont_tm2.inc"

/*---------------------------------------------------------------------------------------------------------
  Internal structures and data.
---------------------------------------------------------------------------------------------------------*/
// VIF1 interrupt/Path3 texture streaming stuff.
enum
{
  NGLINT_FINISH,         // Record the current CPU cycle for speed timing.
  NGLINT_LOADTEXTURE,    // Load a texture block via Path3.
};

#define NGL_INT_MAX_ENTRIES     256

struct nglVif1IntEntry
{
  u_int Type;

  // Texture loading stuff.
  u_int* GifDMA;                      // DMA chain for the interrupt handler to send to the GIF.
  u_int DataSize;                     // Total size of the texture entry.
  u_int NTextures;
};

int nglNVif1IntEntries = 0;
nglVif1IntEntry* nglVif1IntArray = 0;

nglVif1IntEntry* _nglVif1IntArray;
int _nglNVif1IntEntries = 0;
int _nglVif1IntEntryIdx = 0;

// Mostly recent texture upload interrupt entry.
nglVif1IntEntry* nglVif1IntCurTexEntry = 0;

nglVif1IntEntry nglVif1IntArrayA[NGL_INT_MAX_ENTRIES];
nglVif1IntEntry nglVif1IntArrayB[NGL_INT_MAX_ENTRIES];

nglStageStruct nglStage;

nglDebugStruct nglDebug;
nglDebugStruct nglSyncDebug;  // Updated synchronously once per frame.

nglPerfInfoStruct nglPerfInfo;

bool nglUsingProView = false;
const char* nglHostPrefix = "host0:";

// Scratch mesh globals.
u_int nglScratchMeshWorkSize = 0;
u_char* nglScratchMeshWorkA = 0;
u_char* nglScratchMeshWorkB = 0;
u_int* nglScratchMeshPos = 0;
u_char* nglScratchMeshWork = 0;

// Current in-progress scratch mesh.
nglMesh* nglScratch = 0;
nglMeshSection* nglScratchSection = 0;
nglMeshBatchInfo nglScratchBatch;
u_int nglScratchBatchIdx = 0;
u_int nglScratchVertIdx = 0;
u_int nglScratchStripVertIdx = 0;

// Render list workspace.
u_int nglListWorkSize = 0;
u_char* nglListWorkEnd = 0;
u_char* nglListWork = 0;
u_char* nglListWorkPos = 0;

// Vif1 packet workspace.
u_int nglVif1PacketWorkSize = 0;
u_char* nglVif1PacketWorkA = 0;
#if NGL_ASYNC_LIST_SEND
u_char* nglVif1PacketWorkB = 0;
#endif
u_char* nglVif1PacketWork = 0;
u_int* nglVif1Packet = 0;

u_long* nglDmaTagPtr = NULL;

// If this becomes true, we've detected a really bad situation and just skip the frame.
bool nglFatalDMAError = false;

bool nglDisableTim2Conversion = true;

u_int nglLastVUCodeDma = 0;

// Gif packet workspace.
u_char* nglGifPacketWorkA = 0;
#if NGL_ASYNC_LIST_SEND
u_char* nglGifPacketWorkB = 0;
#endif
u_int nglGifPacketWorkSize = 0;
u_int* nglGifPacketNext;
u_char* nglGifPacketWork;

sceGifPacket nglGifPkt __attribute__((aligned(16)));

u_short nglCommandList[128];
u_short nglCommandListCount;

// Cache of the current state of all GS registers.
u_int nglGSRegIdx[NGL_GS_NUM_REGS] __attribute__((section("sbss"))) =
{
  SCE_GS_FOG,
  SCE_GS_PRMODE,
  SCE_GS_TEX0_1,
  SCE_GS_TEX1_1,
  SCE_GS_TEXCLUT,
  SCE_GS_MIPTBP1_1,
  SCE_GS_MIPTBP2_1,
  SCE_GS_CLAMP_1,
  SCE_GS_TEXA,
  SCE_GS_ALPHA_1,
  SCE_GS_TEST_1,
  SCE_GS_PABE,
  SCE_GS_RGBAQ,
};

// Compressed representation of the GS registers to be set for a material.
u_int nglGifRegCount;
u_char nglGifRegIdx[128];
u_long nglGifRegVal[128];

//u_int* nglVif1DirectPtr = 0;          // Used by StartGifAD/EndGifAD.

// Frame counter.
int nglFrame = 0;

float nglIFLSpeed = 30.0f;

volatile bool nglDMAFinished = true;
u_int nglDMATimeoutCounter = 0;
bool nglStopOnDmaTimeout = false;
bool nglCompleteFrame = false;

u_int nglTVMode = NGLTV_NTSC;
u_int nglDisplayWidth = 512;
u_int nglDisplayHeight = 448;

// System font.
nglFont* nglSysFont = NULL;

// Pathnames to prepend when loading meshes/textures.  For reference, 256 characters is:
// ................................................................................................................................................................................................................................................................ + filename.
char nglMeshPath[NGL_MAX_PATH_LENGTH];
char nglTexturePath[NGL_MAX_PATH_LENGTH];

//float nglGuardBandEdge = 150.0f;
//float nglGuardBandEdge = 256.0f;      // exact screen clipping
float nglGuardBandEdge = 1024.0f;
nglVector nglScreenMaxValue( 4095,4095,16777215,1 );
nglVector nglScreenMinValue( 0,0,0,0 );

// Calls outside nglListInit/nglListSend affect the global scene.
nglScene nglDefaultScene;

bool nglInScene = false;

// Currently active scene.
nglScene* nglCurScene = &nglDefaultScene;

// Texture management globals.
u_int nglLockedTextureSize = 0;      // Locked texture buffer.  Starts off empty.

u_int nglLockBufferSize = ( 256 * 1024 ) / 256;

nglTexStreamPos nglTexStreamPrevStartRef;	// Oldest referenced texture in previous texture block
nglTexStreamPos nglTexStreamCurStartRef;	// Oldest referenced texture in current texture block
											// Current allocation point in looping texture streaming buffer
nglTexStreamPos nglTexStreamAlloc(NGL_VRAM_STREAM_START,2);

nglTexture nglFrontBufferTex;
nglTexture nglBackBufferTex;

// Current animation frame for animated textures.  Warning: This global is sometimes used as a parameter.
int nglTextureAnimFrame = 0;

// Debugging flag to optionally turn off and on material flags.
u_int nglMaterialFlagsAnd = 0xFFFFFFFF;
u_int nglMaterialFlagsOr = 0;

char nglWork[512];

nglSystemCallbackStruct nglSystemCallbacks = { 0 };

// Global meshes for displaying debugging info.
nglMesh* nglSphereMesh;
nglMesh* nglRadiusMesh;

// Memory mapped registers.
tVIF1_STAT* nglVif1Stat = (tVIF1_STAT*)VIF1_STAT;
tVIF1_ERR* nglVif1Err = (tVIF1_ERR*)VIF1_ERR;
tGIF_STAT* nglGifStat = (tGIF_STAT*)GIF_STAT;

// Global VBlank counter (updated by the interrupt handler).
volatile u_int nglVBlankCount = 0;
u_int nglLastVBlank = 0;

// Frame locking support - the minimum number of VBlanks that must pass before
u_int nglFrameLock;

// Skip list based instancing system for meshes and textures.
nglInstanceBank nglMeshBank;
nglInstanceBank nglMeshFileBank;
nglInstanceBank nglTextureBank;
nglInstanceBank nglFontBank;

// variable to hold aspect ratio, until we create a better api
static float nglAspectRatio = 4.0f / 3.0f;

/*---------------------------------------------------------------------------------------------------------
  @Internal function prototypes.
---------------------------------------------------------------------------------------------------------*/
int nglVif1Interrupt( int ca );
void nglClearVRAM( u_char r, u_char g, u_char b, u_char a );
void nglVif1IntCreateTextureBlock();
void nglVif1IntCloseTextureBlock();
u_int nglVif1IntTextureUploadNextAvail(void);
bool nglVif1IntMaybeAddTextureUpload(u_int*& Packet, nglTexture **Textures, u_int NTextures, bool full, bool allow_adds=true);
void nglVif1IntAddFinish( u_int*& Packet );
void nglVif1AddTextureStreamSetup( u_int*& Packet );
void nglVif1AddTextureStreamEnd( u_int*& Packet );
void nglVif1SetupScene( u_int*& Packet, nglScene* Scene, bool ClearEnable );
void nglVif1RenderScene( u_int*& Packet, nglScene* Scene );

// TIM2 related functions.
inline int nglTim2CalcBufWidth( int psm, int w );
inline int nglTim2CalcBufSize(int psm, int w, int h);
inline int nglTim2CheckFileHeader( void* pTim2 );
inline TIM2_PICTUREHEADER *nglTim2GetPictureHeader(void *pTim2, int imgno);
void* nglTim2GetClut( nglTexture *Tex );
void* nglTim2GetImage( nglTexture *Tex, int mipmap );
int nglTim2GetMipMapPictureSize(TIM2_PICTUREHEADER *ph, int mipmap, int *pWidth, int *pHeight);

int nglTim2Convert4to32(int width, int height, u_char *p_input, u_char *p_output);
int nglTim2Convert8to32(int width, int height, u_char *p_input, u_char *p_output);
int nglTim2Convert16to32(int width, int height, u_char *image_in, u_char *image_out);

typedef int (*nglTim2Convert)(int width, int height, u_char *p_input, u_char *p_output);

// Scene Dump API.  This should eventually contain functions for most of the per-frame NGL API.
void nglSceneDumpStart();
void nglSceneDumpMesh( nglMesh* Mesh, const nglMatrix& LocalToWorld, const nglRenderParams* Params );
void nglSceneDumpDirLight( u_int LightCat, const nglVector& Dir, const nglVector& Color );
void nglSceneDumpPointLight( u_int Type, u_int LightCat, const nglVector& Pos, float Near, float Far, const nglVector& Color );
void nglSceneDumpEnd();

void nglDumpTextures();

/*---------------------------------------------------------------------------------------------------------
  @Callback default implementations.
---------------------------------------------------------------------------------------------------------*/
void nglSetSystemCallbacks( nglSystemCallbackStruct* Callbacks )
{
  nglSystemCallbacks = *Callbacks;
  nglInstanceBank::SetAllocFunc(nglSystemCallbacks.MemAlloc);
  nglInstanceBank::SetFreeFunc(nglSystemCallbacks.MemFree);
}

#if !defined(NGL_FINAL)
void nglWarning( const char* Format, ... )
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  nglPrintf( TTY_RED "%s" TTY_BLACK, Work );
}

void nglPrintf( const char* Format, ... )
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  if ( nglSystemCallbacks.DebugPrint )
    nglSystemCallbacks.DebugPrint( Work );
  else
    scePrintf( Work );
}

void nglFatal( const char* Format, ... )
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  if ( nglSystemCallbacks.CriticalError )
    nglSystemCallbacks.CriticalError( Work );
  else
  {
    scePrintf( TTY_BLUE "NGL Fatal Error: " TTY_RED );
    scePrintf( Work );
    scePrintf( TTY_BLACK "\n" );
    asm( "break 1" );
  }
}
#endif

bool nglReadFile( const char* FileName, nglFileBuf* File, u_int Align )
{
  if ( nglSystemCallbacks.ReadFile )
    return nglSystemCallbacks.ReadFile( FileName, File, Align );
  else
  {
    static char Work[256];

    if ( strncmp( FileName, nglHostPrefix, strlen( nglHostPrefix ) ) == 0 )
      strcpy( Work, "" );
    else
      strcpy( Work, nglHostPrefix );
    strcat( Work, FileName );

    int fd = sceOpen( Work, SCE_RDONLY );
    if ( fd < 0 )
      return false;

    File->Size = sceLseek( fd, 0, SCE_SEEK_END );
    sceLseek( fd, 0, SCE_SEEK_SET );

    // sceRead only reads in 32-byte chunks, rounding down, so we need to read in the extra on our own.
    File->Buf = (u_char*)nglMemAlloc( ( File->Size + 31 ) & ~31, 128 );
    sceRead( fd, File->Buf, ( File->Size + 31 ) & ~31 );
    sceClose( fd );

    return true;
  }
}

void nglReleaseFile( nglFileBuf* File )
{
  if ( nglSystemCallbacks.ReleaseFile )
    nglSystemCallbacks.ReleaseFile( File );
  else
  {
    nglMemFree( File->Buf );
    memset( File, 0, sizeof(nglFileBuf) );
  }
}

void* nglMemAlloc( u_int Size, u_int Align )
{
  if ( nglSystemCallbacks.MemAlloc )
    return nglSystemCallbacks.MemAlloc( Size, Align );
  else
    return memalign( Align, Size );
}

void nglMemFree( void* Ptr )
{
  if ( nglSystemCallbacks.MemFree )
    nglSystemCallbacks.MemFree( Ptr );
  else
    free( Ptr );
}

/*---------------------------------------------------------------------------------------------------------
  @System code.
---------------------------------------------------------------------------------------------------------*/
int nglGetScreenWidth()
{
  return nglDisplayWidth;
}

int nglGetScreenHeight()
{
#if NGL_EMULATE_480_LINES
  return 480;
#else
  return nglDisplayHeight;
#endif
}

void nglSetMeshPath( const char* Path )
{
  strncpy( nglMeshPath, Path, sizeof(nglMeshPath) );
  nglMeshPath[sizeof(nglMeshPath) - 1] = 0;
}

void nglSetTexturePath( const char* Path )
{
  strncpy( nglTexturePath, Path, sizeof(nglTexturePath) );
  nglTexturePath[sizeof(nglTexturePath) - 1] = 0;
}

const char *nglGetMeshPath( void )
{
	return nglMeshPath;
}

const char *nglGetTexturePath( void )
{
	return nglTexturePath;
}


// Call this before nglInit if you're using ProView for file transfers.
void nglSetProViewPS2( bool UsingProView )
{
  nglUsingProView = UsingProView;
}

void nglSetTVMode( u_int TVMode )
{
  nglTVMode = TVMode;

	switch (nglTVMode)
	{
	case NGLTV_NTSC:
	{
		nglDisplayWidth = 512;
		nglDisplayHeight = 448;
		break;
	}
	case NGLTV_PAL:
	{
		nglDisplayWidth = 512;
		nglDisplayHeight = 512;
		break;
	}
	}
	// make sure the front/back buffers have updated values
  	nglFrontBufferTex.Width = nglDisplayWidth;
  	nglFrontBufferTex.Height = nglDisplayHeight;
  	nglBackBufferTex.Width = nglDisplayWidth;
  	nglBackBufferTex.Height = nglDisplayHeight;
	// maybe this should be a client call?
	nglSetViewport( 0, 0, nglDisplayWidth - 1, nglDisplayHeight - 1 );
}

u_int nglGetTVMode()
{
	return nglTVMode;
}

void nglSetAspectRatio(float a)
{
	nglAspectRatio = a;
}

void nglIdentityMatrix( nglMatrix& matrix )
{
  sceVu0UnitMatrix( SCE_MATRIX(matrix) );
}

void nglMulMatrix( nglMatrix& dst, const nglMatrix& lhs, const nglMatrix& rhs )
{
  sceVu0MulMatrix( SCE_MATRIX(dst), SCE_MATRIX(lhs), SCE_MATRIX(rhs) );
}

void nglApplyMatrix( nglVector& dst, nglMatrix& lhs, nglVector& rhs )
{
  sceVu0ApplyMatrix( dst, SCE_MATRIX(lhs), rhs );
}

#if NGL_DEBUGGING
u_char* nglGetDebugFlagPtr( const char* Flag )
{
  // Stage flags.
  if ( stricmp( Flag, "FSAA" ) == 0 )
    return &nglStage.FSAA;
  else
  if ( stricmp( Flag, "DMASend" ) == 0 )
    return &nglStage.DMASend;
  else
  if ( stricmp( Flag, "Clip" ) == 0 )
    return &nglStage.Clip;
  else
  if ( stricmp( Flag, "BatchDMA" ) == 0 )
    return &nglStage.BatchDMA;
  else
  if ( stricmp( Flag, "Skin" ) == 0 )
    return &nglStage.Skin;
  else
  if ( stricmp( Flag, "XForm" ) == 0 )
    return &nglStage.XForm;
  else
  if ( stricmp( Flag, "Backface" ) == 0 )
    return &nglStage.Backface;
  else
  if ( stricmp( Flag, "Light" ) == 0 )
    return &nglStage.Light;
  else
  if ( stricmp( Flag, "Kick" ) == 0 )
    return &nglStage.Kick;
  else
  if ( stricmp( Flag, "PlaneClip" ) == 0 )
    return &nglStage.PlaneClip;
  else
  if ( stricmp( Flag, "CommandList" ) == 0 )
    return &nglStage.CommandList;
  else
  if ( stricmp( Flag, "DiffusePass" ) == 0 )
    return &nglStage.DiffusePass;
  else
  if ( stricmp( Flag, "DetailPass" ) == 0 )
    return &nglStage.DetailPass;
  else
  if ( stricmp( Flag, "EnvironmentPass" ) == 0 )
    return &nglStage.EnvironmentPass;
  else
  if ( stricmp( Flag, "LightPass" ) == 0 )
    return &nglStage.LightPass;
  else
  if ( stricmp( Flag, "ProjectedTexturePass" ) == 0 )
    return &nglStage.ProjectedTexturePass;
  else
  if ( stricmp( Flag, "FogPass" ) == 0 )
    return &nglStage.FogPass;
  else
  // Debug flags.
  if ( stricmp( Flag, "ShowPerfInfo" ) == 0 )
    return &nglDebug.ShowPerfInfo;
  else
  if ( stricmp( Flag, "ScreenShot" ) == 0 )
    return &nglDebug.ScreenShot;
  else
  if ( stricmp( Flag, "DisableQuads" ) == 0 )
    return &nglDebug.DisableQuads;
  else
  if ( stricmp( Flag, "DisableTextures" ) == 0 )
    return &nglDebug.DisableTextures;
  else
  if ( stricmp( Flag, "DisableMipmaps" ) == 0 )
    return &nglDebug.DisableMipmaps;
  else
  if ( stricmp( Flag, "DisableVSync" ) == 0 )
    return &nglDebug.DisableVSync;
  else
  if ( stricmp( Flag, "DisableScratch" ) == 0 )
    return &nglDebug.DisableScratch;
  else
  if ( stricmp( Flag, "DisableMipOpt" ) == 0 )
    return &nglDebug.DisableMipOpt;
  else
  if ( stricmp( Flag, "DebugPrints" ) == 0 )
    return &nglDebug.DebugPrints;
  else
  if ( stricmp( Flag, "DumpFrameLog" ) == 0 )
    return &nglDebug.DumpFrameLog;
  else
  if ( stricmp( Flag, "DumpSceneFile" ) == 0 )
    return &nglDebug.DumpSceneFile;
  else
  if ( stricmp( Flag, "DumpTextures" ) == 0 )
    return &nglDebug.DumpTextures;
  else
  if ( stricmp( Flag, "DrawLightSpheres" ) == 0 )
    return &nglDebug.DrawLightSpheres;
  else
  if ( stricmp( Flag, "DrawMeshSpheres" ) == 0 )
    return &nglDebug.DrawMeshSpheres;
  else
  if ( stricmp( Flag, "DrawNormals" ) == 0 )
    return &nglDebug.DrawNormals;
  else
  if ( stricmp( Flag, "DrawAsPoints" ) == 0 )
    return &nglDebug.DrawAsPoints;
  else
  if ( stricmp( Flag, "DrawAsLines" ) == 0 )
    return &nglDebug.DrawAsLines;
  else
  if ( stricmp( Flag, "DMARepeatMode" ) == 0 )
    return &nglDebug.DMARepeatMode;
  else
  if ( stricmp( Flag, "DrawToFrontBuffer" ) == 0 )
    return &nglDebug.DrawToFrontBuffer;
  else
  if ( stricmp( Flag, "SyncRender" ) == 0 )
    return &nglDebug.SyncRender;
  else
    return NULL;
}
#endif

void nglSetDebugFlag( const char* Flag, bool Set )
{
#if NGL_DEBUGGING
  u_char* Ptr = nglGetDebugFlagPtr( Flag );
  if ( Ptr )
    *Ptr = Set;
#endif
}

bool nglGetDebugFlag( const char* Flag )
{
#if NGL_DEBUGGING
  u_char* Ptr = nglGetDebugFlagPtr( Flag );
  if ( Ptr )
    return *Ptr;
  else
#endif
    return false;
}

struct nglDisplayEnv
{
  tGS_PMODE pmode;
  tGS_SMODE2 smode2;
  tGS_DISPFB2 dispfb;
  tGS_DISPLAY2 display;
  tGS_BGCOLOR bgcolor;

  // extra registers
  tGS_DISPFB1 dispfb1;
  tGS_DISPLAY1 display1;
  tGS_DISPLAY1 pad;
} __attribute__ ((aligned(64)));

void nglSetDisplayEnv( int frame, int fsaa )
{
  nglDisplayEnv disp;
  sceGsSetDefDispEnv((sceGsDispEnv*)&disp, NGL_FRAME_FORMAT, nglDisplayWidth, nglDisplayHeight, 0, 0 ); // dx,dy = 0

  disp.dispfb.FBP = frame == 0 ? ( NGL_VRAM_FRAME1 / 32 ) : ( NGL_VRAM_FRAME2 / 32 );

  // set up 2 circuit antialiasing
  *(u_long*)&disp.dispfb1 = *(u_long*)&disp.dispfb;
  *(u_long*)&disp.display1 = *(u_long*)&disp.display;

  disp.dispfb.DBY = 1; // dispfb2 has DBY field of 1
  disp.display.DH = disp.display1.DH - 1;
  disp.display.DX = disp.display.DX + 2; // display2 has magh/2
  disp.pmode.ALP = 0x80;

  // toggle FSAA
  disp.pmode.EN1 = fsaa;
  disp.pmode.EN2 = 1;

  // write registers to GS.
  DPUT_GS_PMODE( *(u_long*)&disp.pmode );
  DPUT_GS_SMODE2( *(u_long*)&disp.smode2 );
  DPUT_GS_DISPFB2( *(u_long*)&disp.dispfb );
  DPUT_GS_DISPLAY2( *(u_long*)&disp.display );
  DPUT_GS_BGCOLOR( *(u_long*)&disp.bgcolor );

  DPUT_GS_DISPLAY1( *(u_long*)&disp.display1 );
  DPUT_GS_DISPFB1( *(u_long*)&disp.dispfb1 );
}

// Props to George Bain (SCEE) for this function.
void nglClearVRAM( u_char r, u_char g, u_char b, u_char a )
{
  // set base address of GIF packet
  sceGifPkInit(&nglGifPkt, (u_long128*)nglGifPacketWork );
  sceGifPkReset(&nglGifPkt);

  // set GIF tag
  sceGifPkAddGsData(&nglGifPkt, SCE_GIF_SET_TAG(9,1,NULL,NULL,SCE_GIF_PACKED,1));
  sceGifPkAddGsData(&nglGifPkt, 0xEL);

  // enable PRIM register
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_PRMODECONT(1));
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_PRMODECONT);

  // frame buffer settting
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_FRAME_1(0, 1024/64, SCE_GS_PSMCT32, 0));
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_FRAME_1);

  // offset value ( PRIM coord -> WIN coord )
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_XYOFFSET_1(0,0));
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_XYOFFSET_1);

  // scissor settings ( WIN coordinates x0,x1,y0,y1 )
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_SCISSOR_1(0,1024,0,1024));
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SCISSOR_1);

  // pixel test control
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_TEST_1(0,0,0,0,0,0,0,0));
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_TEST_1);

  // set sprite primitive
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,0,0,0,0,0) );
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_PRIM);

  // set RGBA of sprite
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_RGBAQ(r, g, b, a, 0x3f800000));
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_RGBAQ);

  // set upper-left position of sprite
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_XYZ(0,0,0) );
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_XYZF2);

  // set lower-right position of sprite
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_SET_XYZ( 1024<<4, 1024<<4, 0) );
  sceGifPkAddGsData(&nglGifPkt, SCE_GS_XYZF2);

  // get packet size in quad words
  sceGifPkTerminate(&nglGifPkt);

  // DMA send (Normal Mode) to GS via path 3
  FlushCache(WRITEBACK_DCACHE);

  _nglDmaStartGifNormal(((u_int)nglGifPkt.pBase&NGL_DMA_MEM), sceGifPkSize(&nglGifPkt));
  _nglDmaSetWaitGif();
  _nglDmaWait();
}

//
// GS ALPHA parameters per blend mode:
//
// OPAQUE             ( Sc - 0  ) * Fx + 0   - Fx is forced to 128.
// PUNCHTHROUGH       ( Sc - Dc ) * Sa + Dc

// BLEND              ( Sc - Dc ) * Sa + Dc
// ADDITIVE           ( Sc - 0  ) * Sa + Dc
// SUBTRACTIVE        ( 0  - Sc ) * Sa + Dc

// CONST_BLEND        ( Sc - Dc ) * Fx + Dc
// CONST_ADDITIVE     ( Sc - 0  ) * Fx + Dc
// CONST_SUBTRACTIVE  ( 0  - Sc ) * Fx + Dc

// PUNCHTHROUGH_NO_ZTEST ( Sc - Dc ) * Sa + Dc
// DESTALPHA_ADDITIVE ( Sc - 0  ) * Da + Dc


const u_long nglGSBlendModes[NGLBM_MAX_BLEND_MODES][2] =
{
  { SCE_GS_SET_ALPHA( 0, 2, 2, 2, 0x80 ), SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 2 ) },
  { SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ), SCE_GS_SET_TEST( 1, 5, 0x40, 0, 0, 0, 1, 2 ) },

  { SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },
  { SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },
  { SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },

  { SCE_GS_SET_ALPHA( 0, 1, 2, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },
  { SCE_GS_SET_ALPHA( 0, 2, 2, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },
  { SCE_GS_SET_ALPHA( 2, 0, 2, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },

  { SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ), SCE_GS_SET_TEST( 1, 6, 0, 0, 0, 0, 1, 1 ) },
  { SCE_GS_SET_ALPHA( 0, 2, 1, 1, 0 ), SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 2 ) },
};

#ifndef PROJECT_KELLYSLATER
const 
#endif
u_int nglTexL = 0;
u_int nglMinFilter = 4;

// Calculate GS memory buffer width from pixel format and texture size.
int nglTim2CalcBufWidth( int psm, int w )
{
  if ( psm == SCE_GS_PSMT8 || psm == SCE_GS_PSMT4 )
  {
    w = ( w + 63 ) / 64;
    if ( w & 1 ) w++;
    return w;
  }
  else
    return ( w + 63 ) / 64;
}

void nglCalcTextureGSRegs( nglTexture* Tex )
{
  Tex->TW = nglLog2( nglNextPow2( Tex->Width ) );
  Tex->TH = nglLog2( nglNextPow2( Tex->Height ) );

  if ( Tex->Type == NGLTEX_TGA )
  {
    // Set the value of alpha for 24-bit textures to be 0x80 (1.0f).  This also affects the
    // value of on/off alpha for 16-bit textures.
    if ( Tex->Format == SCE_GS_PSMCT24 )
      Tex->GsTexA = SCE_GS_SET_TEXA( 0x80, 0, 0 );
    else if ( Tex->Format == SCE_GS_PSMCT16 )
      Tex->GsTexA = SCE_GS_SET_TEXA( 0x0, 0, 0x80 );
    else
      Tex->GsTexA = 0;

    Tex->GsTex0 = SCE_GS_SET_TEX0(
      0, ( Tex->Width + 63 ) / 64, Tex->Format,
      Tex->TW, Tex->TH, 1, 0, 0, 0, 0, 0, 0 );

    Tex->GsTex1 = SCE_GS_SET_TEX1( 0, 0, 1, 1, 0, 0, 0 );
  }
  else
  if ( Tex->Type == NGLTEX_TIM2 )
  {
    // calculate per-texture GS registers from the picture header.
    TIM2_PICTUREHEADER* ph = Tex->ph;
    int psm = psmtbl[ph->ImageType - 1];
    int cpsm = 0;
    if ( Tex->ClutColors )
    {
      // Get the GS pixel format.
      if ( ( ph->ClutType & 0x3F ) == TIM2_RGB16 )
        cpsm = SCE_GS_PSMCT16;
      else if ( ( ph->ClutType & 0x3F ) == TIM2_RGB24 )
        cpsm = SCE_GS_PSMCT24;
      else
        cpsm = SCE_GS_PSMCT32;
    }

    // calculate TEXA register.
    u_int tcc = 1;
    if ( cpsm == SCE_GS_PSMCT24 || psm == SCE_GS_PSMCT24 )
    {
      Tex->GsTexA = SCE_GS_SET_TEXA( 0x80, 0, 0x80 );
      tcc = 0;
    }
    else
    if ( cpsm == SCE_GS_PSMCT16 || psm == SCE_GS_PSMCT16 )
    {
      Tex->GsTexA = SCE_GS_SET_TEXA( 0, 0, 0x80 );
      tcc = 1;
    }
    else
      Tex->GsTexA = 0;

    // calculate TEX0 register.
    int tbw = nglTim2CalcBufWidth( psm, Tex->Width );
    Tex->GsTex0 = SCE_GS_SET_TEX0( 0, tbw, psm, Tex->TW, Tex->TH, tcc, 0, 0, cpsm, 0, 0, 1 );

    if ( ph->MipMapTextures > 1 )
    {
      sceGsTex1* GsTex1 = (sceGsTex1*)&Tex->GsTex1;
      GsTex1->LCM = 0;
      GsTex1->MXL = ph->MipMapTextures - 1;

      GsTex1->L = nglTexL;

      GsTex1->MMAG = 1;
      GsTex1->MMIN = nglMinFilter;
    }
    else
      Tex->GsTex1 = SCE_GS_SET_TEX1( 0, 0, 1, 1, 0, 0, 0 );

    Tex->InvHypot = 1.0f / sqrtf( Tex->Width * Tex->Width + Tex->Height * Tex->Height );
  }
}

void nglInitFrameBufferTextures()
{
  memset(&nglFrontBufferTex,0,sizeof(nglTexture));

  nglFrontBufferTex.Type = NGLTEX_TGA;
  nglFrontBufferTex.Flags.Locked = 1;
  nglFrontBufferTex.Flags.VRAMOnly = 1;
  nglFrontBufferTex.Flags.System = 1;
  nglFrontBufferTex.Flags.RenderTarget = 1;

  nglFrontBufferTex.Width = nglDisplayWidth;
  nglFrontBufferTex.Height = nglDisplayHeight;

  nglFrontBufferTex.GsSize = nglFrontBufferTex.Width * nglFrontBufferTex.Height * NGL_FRAME_PIXELSIZE / 256;

  nglFrontBufferTex.GsBaseTBP = NGL_VRAM_FRAME1;
  nglFrontBufferTex.Format = NGL_FRAME_FORMAT;
  nglFrontBufferTex.FileName = "NGLFRONTBUF";

  nglCalcTextureGSRegs( &nglFrontBufferTex );
  ( (sceGsTex0*)&nglFrontBufferTex.GsTex0 )->TBP0 = nglFrontBufferTex.GsBaseTBP;

  memcpy( &nglBackBufferTex, &nglFrontBufferTex, sizeof(nglTexture) );
  nglBackBufferTex.GsBaseTBP = NGL_VRAM_FRAME2;
  nglBackBufferTex.FileName = "NGLBACKBUF";

  nglCalcTextureGSRegs( &nglBackBufferTex );
  ( (sceGsTex0*)&nglBackBufferTex.GsTex0 )->TBP0 = nglBackBufferTex.GsBaseTBP;

  nglTextureBank.Insert( nglFrontBufferTex.FileName, &nglFrontBufferTex );
  nglTextureBank.Insert( nglBackBufferTex.FileName, &nglBackBufferTex );
}

nglTexture* nglGetFrontBufferTex()
{
	return &nglFrontBufferTex;
}

nglTexture* nglGetBackBufferTex()
{
	return &nglBackBufferTex;
}

#define NGL_SPECULAR_TEXSIZE 64
nglTexture* nglSpecularTex = 0;

void nglInitSpecularTexture()
{
  nglSpecularTex = nglCreateTexture( NGLTF_32BIT, NGL_SPECULAR_TEXSIZE, NGL_SPECULAR_TEXSIZE );
  nglSpecularTex->FileName = nglFixedString( "nglspecular" );
  nglSpecularTex->Flags.System = 1;

  u_int* Data = (u_int*)nglSpecularTex->Data;
  for ( int j = 0; j < NGL_SPECULAR_TEXSIZE; j++ )
    for ( int i = 0; i < NGL_SPECULAR_TEXSIZE; i++ )
    {
      Data[j * NGL_SPECULAR_TEXSIZE + i] = NGL_RGBA32_TEX( 0xFF, 0xFF, 0xFF, 0xFF );
    }

  nglTextureBank.Insert( nglSpecularTex->FileName, nglSpecularTex );
}

#define NGL_WHITE_TEXSIZE 32
nglTexture* nglWhiteTex = 0;

void nglInitWhiteTexture()
{
  nglWhiteTex = nglCreateTexture( NGLTF_32BIT, NGL_WHITE_TEXSIZE, NGL_WHITE_TEXSIZE );
  nglWhiteTex->FileName = nglFixedString( "nglwhite" );
  nglWhiteTex->Flags.System = 1;

  u_int* Data = (u_int*)nglWhiteTex->Data;
  for ( int j = 0; j < NGL_WHITE_TEXSIZE; j++ )
    for ( int i = 0; i < NGL_WHITE_TEXSIZE; i++ )
      Data[j * NGL_WHITE_TEXSIZE + i] = NGL_RGBA32_TEX( 0xFF, 0xFF, 0xFF, 0xFF );

  nglTextureBank.Insert( nglWhiteTex->FileName, nglWhiteTex );
}

#define NGL_PHONG_TEXSIZE 64
nglTexture* nglPhongTex = 0;

void nglInitPhongTexture()
{
  nglPhongTex = nglCreateTexture( NGLTF_32BIT, NGL_PHONG_TEXSIZE, NGL_PHONG_TEXSIZE );
  nglPhongTex->FileName = nglFixedString( "nglphong" );
  nglPhongTex->Flags.System = 1;

  u_int* Data = (u_int*)nglPhongTex->Data;
  for ( float j = 0; j < NGL_PHONG_TEXSIZE; j++ )
    for ( float i = 0; i < NGL_PHONG_TEXSIZE; i++ )
    {
      float x = ( NGL_PHONG_TEXSIZE / 2.0f - ( i + 0.5f ) ) / ( NGL_PHONG_TEXSIZE / 2.0f - 0.5f );
      float y = ( NGL_PHONG_TEXSIZE / 2.0f - ( j + 0.5f ) ) / ( NGL_PHONG_TEXSIZE / 2.0f - 0.5f );
//      float Color = 1.0f - min( sqrtf( x*x + y*y ), 1.0f );
//      float Color = 1.0f - min( 1.0f / ( x*x + y*y ), 1.0f );
      int Color = int( ( 1.0f - min( powf( x*x + y*y, 0.33333f ), 1.0f ) ) * 255.0f );
      int Ofs = int( j * NGL_PHONG_TEXSIZE + i );
      Data[Ofs] = NGL_RGBA32_TEX( 255, 255, 255, Color );
    }

  nglTextureBank.Insert( nglPhongTex->FileName, nglPhongTex );
}

// @VBlank handler.
int nglVBlankInterrupt(int p)
{
  // Increment the global VBlank counter.
  nglVBlankCount++;

  ExitHandler();
  return 0;
}

void _nglSetDisplay()
{
  nglVif1SafeWait();
  sceGsSyncV( 0 );
  sceGsResetPath();
  sceGsResetGraph( 0, SCE_GS_INTERLACE, nglTVMode == NGLTV_NTSC ? SCE_GS_NTSC : SCE_GS_PAL, SCE_GS_FIELD );
  nglClearVRAM( 0, 0, 0, 0x80 );
  nglFlip();
}

// @Init
void nglInit()
{
  // In case the user set some debug flags before starting the program.
  memset( &nglDebug, 0, sizeof(nglDebugStruct) );
  memset( &nglStage, 1, sizeof(nglStageStruct) );
  memset( &nglPerfInfo, 0, sizeof(nglPerfInfoStruct) );

  nglSyncDebug = nglDebug;

  nglPrintf( TTY_BLACK "\n" );
  nglPrintf( TTY_BLUE   "-----------------------------------------------------------------\n" TTY_BLACK );
  nglPrintf( TTY_RED    "  Nyarlathotep's Graphics Laboratory " NGL_VERSION "\n" );
  nglPrintf( TTY_YELLOW "  (c) 2002 Treyarch LLC\n" );
  nglPrintf( TTY_BLACK "\n" );

  FlushCache(0);

  nglPrintf( TTY_BLUE "Initializing DMA hardware...\n" );

  // mdm 11.19.2001: call our new dma init function
  _nglDmaInit();

  // mdm 11.19.2001: not necessary once you replace the VU0 sce library
  sceVpu0Reset();

  // Install the Vif1 interrupt handler.
  if ( AddIntcHandler( INTC_VIF1, nglVif1Interrupt, -1 ) < 0 )
    nglFatal( "nglInit: Error adding Vif1 Interrupt Handler!\n" );
  EnableIntc( INTC_VIF1 );

  // Install the VBlank handler.
  if ( AddIntcHandler( INTC_VBLANK_S, nglVBlankInterrupt, 0 ) < 0 )
    nglFatal( "nglInit: Error adding VBlank Interrupt Handler!\n" );
  EnableIntc( INTC_VBLANK_S );

  nglFrameLock = 1;

  // Set GIF Path3 transfers to IMT (intermittent) mode
  DPUT_GIF_MODE( GIF_MODE_IMT_M );
  sync_l();

  // Set TIMER modes to count HBlanks & enable Count Up (whatever that means).
  *T0_MODE = T_MODE_CLKS_M | T_MODE_CUE_M;
  *T1_MODE = T_MODE_CLKS_M | T_MODE_CUE_M;

  // Make default allocations for various NGL buffers (if not already done by the app).
  nglPrintf( TTY_BLUE "Allocating DMA packet buffers...\n" );
  if ( !nglVif1PacketWorkA )
    nglSetBufferSize( NGLBUF_VIF1_PACKET_WORK, 512 * 1024 );
  if ( !nglGifPacketWorkA )
    nglSetBufferSize( NGLBUF_GIF_PACKET_WORK, 256 * 1024 );
  if ( !nglScratchMeshWorkA )
    nglSetBufferSize( NGLBUF_SCRATCH_MESH_WORK, 1024 * 1024 );
  if ( !nglListWork )
    nglSetBufferSize( NGLBUF_LIST_WORK, 384 * 1024 );

  // Load the VU1 microcode.
  nglPrintf( TTY_BLUE "Loading VU1 microcode...\n" );

  // Check to make sure we didn't overrun our VU code space.
  if ( (u_int)nglEndAddr - (u_int)nglBaseAddr >= 16 * 1024 )
    nglFatal( "VU code overflow, please remove some instructions.\n" );

  nglLastVUCodeDma = NGL_VUCODE_MESH;
  nglVif1PacketWork = nglVif1PacketWorkA;
  u_int* Packet;
  nglVif1InitPacket( Packet );
  nglDmaStartTag( Packet );
  nglDmaEndTag( Packet, SCE_DMA_ID_CALL, &nglLoadMicrocode );
  nglDmaStartTag( Packet );
  nglDmaEndTag( Packet, SCE_DMA_ID_END );
  nglVif1ClosePacket( Packet );
  FlushCache( 0 );
  _nglDmaStartVif1SourceChain(((u_int)nglVif1PacketWork&NGL_DMA_MEM), 0);

  nglGifPacketWork = nglGifPacketWorkA;

  nglPrintf( TTY_BLUE "Initializing system structures...\n" );

  nglLockedTextureSize = 0;

  nglMeshPath[0] = '\0';
  nglTexturePath[0] = '\0';

  nglMeshBank.Init();
  nglMeshFileBank.Init();
  nglTextureBank.Init();
  nglFontBank.Init();

  nglPerfInfo.TotalMS = 0;
  nglPerfInfo.TotalSeconds = 0;

  // Reset graphics data path
  nglPrintf( TTY_BLUE "Initializing GS hardware...\n" );
  _nglSetDisplay();

  // Initialize standard system resources.
  nglInitFrameBufferTextures();
  nglInitPhongTexture();
  nglInitWhiteTexture();

  // Load up the debugging meshes.
#if NGL_DEBUGGING
  nglLoadMeshFileInPlace( "nglsphere", nglSphereMeshData );
  nglLoadMeshFileInPlace( "nglradius", nglRadiusMeshData );
  nglSphereMesh = nglGetMesh( "ngl_sphere" );
  nglRadiusMesh = nglGetMesh( "ngl_radius" );
#endif

  // Get the system font FDF from memory (embedded cpp file).
  nglTexture* FontTex = nglLoadTextureInPlace( "nglsysfont", NGLTEX_TIM2, nglSysFontTM2, sizeof(nglSysFontFDF) );
  FontTex->Flags.System = 1;
  nglSysFont = nglLoadFontInPlace( "nglsysfont", nglSysFontFDF );
	nglSysFont->System = true;

  // Set up the default scene.
  memset( &nglDefaultScene, 0, sizeof(nglScene) );
  nglCurScene = &nglDefaultScene;
  nglCurScene->RenderTarget = &nglBackBufferTex;
  nglSetFogRange( 0, 0, 0, 0 );
  nglSetClearFlags( NGLCLEAR_Z );
  nglSetClearColor( 0, 0, 0, 0 );
  nglSetClearZ( 1.0f );
  nglSetFBWriteMask( NGL_FBWRITE_RGBA );
  nglSetZWriteEnable( true );
  nglSetZTestEnable( true );
  nglSetViewport( 0, 0, nglDisplayWidth - 1, nglDisplayHeight - 1 );
  nglSetPerspectiveMatrix( 60, nglDisplayWidth / 2, nglDisplayHeight / 2, 0.0f, 10000.0f, 0.0f, 1.0f );

  nglPrintf( TTY_BLUE "NGL initialization complete.\n" TTY_BLACK );
}

// nglExit is a NOP on console platforms.
void nglExit()
{
}

void nglResetDisplay()
{
	_nglSetDisplay();
}

void nglSetFrameLock( float FPS )
{
  nglFrameLock = nglFTOI( 60.0f / FPS );
}

void nglSetIFLSpeed( float FPS )
{
  nglIFLSpeed = FPS;
}

void nglSetBufferSize( u_int BufID, u_int Size )
{
  if ( nglInScene )
    nglFatal( "nglSetBufferSize called from within a scene.\n" );
  nglVif1SafeWait();

  switch ( BufID )
  {
  case NGLBUF_VIF1_PACKET_WORK:
    if ( nglVif1PacketWorkA )
    {
      nglMemFree( nglVif1PacketWorkA );
#if NGL_ASYNC_LIST_SEND
      nglMemFree( nglVif1PacketWorkB );
#endif
    }
    nglVif1PacketWorkA = (u_char*)nglMemAlloc( Size, 128 );
#if NGL_ASYNC_LIST_SEND
    nglVif1PacketWorkB = (u_char*)nglMemAlloc( Size, 128 );
#endif
    nglVif1PacketWorkSize = Size;
    break;
  case NGLBUF_GIF_PACKET_WORK:
    if ( nglGifPacketWorkA )
    {
      nglMemFree( nglGifPacketWorkA );
#if NGL_ASYNC_LIST_SEND
      nglMemFree( nglGifPacketWorkB );
#endif
    }
    nglGifPacketWorkA = (u_char*)nglMemAlloc( Size, 128 );
#if NGL_ASYNC_LIST_SEND
    nglGifPacketWorkB = (u_char*)nglMemAlloc( Size, 128 );
#endif
    nglGifPacketWorkSize = Size;
    break;
  case NGLBUF_SCRATCH_MESH_WORK:
    if ( nglScratchMeshWorkA )
    {
      nglMemFree( nglScratchMeshWorkA );
      nglMemFree( nglScratchMeshWorkB );
    }
    nglScratchMeshWorkA = (u_char*)nglMemAlloc( Size, 128 );
    nglScratchMeshWorkB = (u_char*)nglMemAlloc( Size, 128 );
    nglScratchMeshWorkSize = Size;
    nglScratchMeshWork = nglScratchMeshWorkA;
	  nglScratchMeshPos = (u_int*)nglScratchMeshWork;
    break;
  case NGLBUF_LIST_WORK:
    if ( nglListWork )
      nglMemFree( nglListWork );
    nglListWork = (u_char*)nglMemAlloc( Size, 128 );
    nglListWorkSize = Size;
    nglListWorkEnd = (u_char*)(nglListWork + nglListWorkSize);
    break;
  }
}

void nglVif1SafeWait()
{
  // If the DMA doesn't complete in 100ms then cancel it.
  register u_int Start = nglGetCPUCycle();

  while ( !nglDMAFinished )
  {
    u_int Time = nglGetCPUCycle();
    if ( !DEBUG_ENABLE( VUDebugBreak ) && Time - Start >= NGL_CPU_CLOCKS_PER_MS * 100.0f )
    {
      // all these pretty variables are for deciding if we should wait to restart or not
      nglCompleteFrame = !nglStopOnDmaTimeout;
      while (!nglCompleteFrame)
			{
#if !NGL_USE_SN_TUNER
        scePcStart( 0, 0, 0 );
#endif
			}
      // print message
      nglWarning( "NGL: Vif1 DMA Timeout, resetting VIF1/GIF.\n" );
#ifndef NGL_FINAL
//      nglDMATimeoutCounter = 120;
#endif
      // stops dma on channel one ...
      *D1_CHCR = 1 | (1<<2) | (0<<4) | (0<<6) | (0<<7) | (0<<8);
      *D2_CHCR = 1 | (1<<2) | (0<<4) | (0<<6) | (0<<7) | (0<<8);
      // resets the vif1
      *VIF1_FBRST = 1 | (1<<3);
      // resets the gif
      *GIF_CTRL = 1;
      // jump out of the loop
      break;
    }
  }
  if ( _nglVif1IntEntryIdx != _nglNVif1IntEntries )
    nglWarning( "NGL: Not enough interrupts received (%d/%d).\n", _nglVif1IntEntryIdx, _nglNVif1IntEntries );
}

//NOTE:  Do not use floating point in nglGifSafeWait!
//  It is called by nglVif1Interrupt at interrupt level, and the floating point
//  registers are not saved and restored, so must not be modified!
const u_int nglGifTimeOut = (u_int)(NGL_CPU_CLOCKS_PER_MS * 100.0f);
void nglGifSafeWait()
{
  // If the DMA doesn't complete in 100ms then cancel it.
  register u_int Start = nglGetCPUCycle();

  while ( (*D2_CHCR & 0x0100) || (*GIF_STAT & 0x0c00) )
  {
    u_int Time = nglGetCPUCycle();
    if ( Time - Start >= nglGifTimeOut )
    {
      // all these pretty variables are for deciding if we should wait to restart or not
      nglCompleteFrame = !nglStopOnDmaTimeout;
      while (!nglCompleteFrame)
			{
#if !NGL_USE_SN_TUNER
        scePcStart( 0, 0, 0 );
#endif
			}
      // print message
      nglWarning( "NGL: GIF DMA Timeout, resetting VIF1/GIF.\n" );
#ifndef NGL_FINAL
//      nglDMATimeoutCounter = 120;
#endif
      // stops dma on channel one and two...
      *D1_CHCR = 1 | (1<<2) | (0<<4) | (0<<6) | (0<<7) | (0<<8);
      *D2_CHCR = 1 | (1<<2) | (0<<4) | (0<<6) | (0<<7) | (0<<8);
      // resets the vif1
      *VIF1_FBRST = 1 | (1<<3);
      // resets the gif
      *GIF_CTRL = 1;
      // jump out of the loop
      break;
    }
  }
//#endif // NGL_FINAL
}

// @Flip function
void nglFlip()
{
  if ( DEBUG_ENABLE( DebugPrints ) )
    nglPrintf( "nglFlip enter\n" );

  STOP_PROF_TIMER( proftimer_render );

  // Wait for any in-progress render to finish
  nglVif1SafeWait();

  if ( nglDebug.ScreenShot )
  {
    nglScreenShot();
    nglDebug.ScreenShot = 0;
  }

  nglFrame++;

  // Wait for the vertical retrace before flipping.
  if ( nglFrameLock != 0 && !DEBUG_ENABLE( DisableVSync ) )
  {
    u_int Start = nglVBlankCount;
#if NGL_USE_SN_TUNER
		sceGsSyncV( 0 );
#endif
    while ( nglVBlankCount == Start || nglVBlankCount - nglLastVBlank < nglFrameLock ) {}
    nglLastVBlank = nglVBlankCount;
  }

  // Report to our masters
  nglPerfInfo.RenderMS = (float)( nglPerfInfo.RenderFinish - nglPerfInfo.RenderStart ) / NGL_CPU_CLOCKS_PER_MS;
  ADD_PROF_TIMER( proftimer_gfx_pipe, (float)( nglPerfInfo.RenderFinish - nglPerfInfo.RenderStart ) );

  // Swap the frame buffer texture pointers.
  nglFrontBufferTex.GsBaseTBP = (nglFrame&0x1) ? NGL_VRAM_FRAME2 : NGL_VRAM_FRAME1;
  nglBackBufferTex.GsBaseTBP  = (nglFrame&0x1) ? NGL_VRAM_FRAME1 : NGL_VRAM_FRAME2;
  ( (sceGsTex0*)&nglFrontBufferTex.GsTex0 )->TBP0 = nglFrontBufferTex.GsBaseTBP;
  ( (sceGsTex0*)&nglBackBufferTex.GsTex0 )->TBP0 = nglBackBufferTex.GsBaseTBP;

  // Update the frame buffers.
#if NGL_ASYNC_LIST_SEND
  nglSetDisplayEnv( DEBUG_ENABLE( DrawToFrontBuffer ) ? (nglFrame & 0x1) : !(nglFrame & 0x1), STAGE_ENABLE( FSAA ) );
#else
  nglSetDisplayEnv( DEBUG_ENABLE( DrawToFrontBuffer ) ? !(nglFrame & 0x1) : (nglFrame & 0x1), STAGE_ENABLE( FSAA ) );
#endif

  // Calculate current frame rate.
  static u_int cycle, lastcycle, diffcycle;
  cycle = nglGetCPUCycle();
  diffcycle = cycle - lastcycle;
  lastcycle = cycle;
  nglPerfInfo.FrameMS = diffcycle / NGL_CPU_CLOCKS_PER_MS;
  nglPerfInfo.TotalMS += nglPerfInfo.FrameMS;
  nglPerfInfo.FPS = ( NGL_CPU_CLOCKS_PER_MS * 1000.0f / diffcycle );
  nglPerfInfo.TotalSeconds = nglPerfInfo.TotalMS / 1000.0f;

  START_PROF_TIMER( proftimer_render );

  if ( DEBUG_ENABLE( DebugPrints ) )
    nglPrintf( "nglFlip leave\n" );
}

/*---------------------------------------------------------------------------------------------------------
  Render list code.
---------------------------------------------------------------------------------------------------------*/
// nglTestNode can be used to narrow the scene down (via binary searching) to a single section of a single mesh.
// It's useful for tracking down weird rendering problems.  Once you've binary searched down to a single node,
// you can break in the debugger and see the node's data as it gets setup, transformed and rendered.
int nglTestNodeStart = 0;
int nglTestNodeEnd = 100000;
int nglNodeCount = 0;

inline bool nglEvalTestNode()
{
  nglNodeCount++;
  if ( nglNodeCount < nglTestNodeStart || nglNodeCount > nglTestNodeEnd )
    return false;

  return true;
}

// Allocates from the render list storage, which is completely cleared each frame.
// Alignment is a bit value, ie 0 for no alignment
void* nglListAlloc( u_int Bytes, u_int Alignment )
{
  register u_int Shifted = 1 << Alignment;
  register u_int WorkPos = (u_int)nglListWorkPos;
  if ( WorkPos & ( Shifted - 1 ) )
    WorkPos = ( ( WorkPos >> Alignment ) + 1 ) * Shifted;

#if NGL_DEBUGGING
  if ( WorkPos + Bytes > (u_int)nglListWorkEnd )
  {
    nglWarning( "Render list allocation overflow. Reserved=%d Requested=%d Free=%d.\n",
      nglListWorkSize, Bytes, nglListWorkSize - ( WorkPos - (u_int)nglListWork ) );
    return NULL;
  }
#endif

  void* Ret = (void*)WorkPos;
  nglListWorkPos = (u_char*)( WorkPos + Bytes );
  return Ret;
}

// Finds the appropriate bin and adds the node to the render list.
void nglListAddNode( u_int Type, nglCustomNodeFn NodeFn, void* Data, nglSortInfo* SortInfo, u_char** Buf )
{
  nglListNode* NewNode;
  if ( Buf )
  {
    NewNode = (nglListNode*)*Buf;
    *Buf += sizeof(nglListNode);
  }
  else
  {
    NewNode = nglListNew( nglListNode );
    if ( !NewNode )
      return;
  }

  NewNode->Type = Type;
  NewNode->NodeFn = NodeFn;
  NewNode->NodeData = Data;
  NewNode->SortInfo = *SortInfo;

  // Find the appropriate bin and insert the node.
  if ( SortInfo->Type == NGLSORT_TRANSLUCENT )
  {
    NewNode->Next = (nglListNode*)nglCurScene->TransRenderList;
    nglCurScene->TransRenderList = NewNode;
    ++nglCurScene->TransListCount;
  }
  else
  {
    NewNode->Next = (nglListNode*)nglCurScene->OpaqueRenderList;
    nglCurScene->OpaqueRenderList = NewNode;
    ++nglCurScene->OpaqueListCount;
  }
}

void nglListAddCustomNode( nglCustomNodeFn CustomNodeFn, void* Data, nglSortInfo* SortInfo)
{
  nglListAddNode( NGLNODE_CUSTOM, CustomNodeFn, Data, SortInfo );
}

int nglCrashFrame = -1;

u_int nglListInitCount = 1;

void nglListInit()
{
  ++nglListInitCount;

  // Reset the list allocator.
  nglListWorkPos = nglListWork;

  // Double buffer the scratch mesh work.
  if (nglScratchMeshWork == nglScratchMeshWorkA )
    nglScratchMeshWork = nglScratchMeshWorkB;
  else
    nglScratchMeshWork = nglScratchMeshWorkA;
  nglScratchMeshPos = (u_int*)( (u_int)nglScratchMeshWork );
  nglScratch = 0;

  // Reset the light lists.
  nglGlobalLightContext = nglCreateLightContext();

  if ( nglSyncDebug.DumpFrameLog )
    nglDebug.DumpFrameLog = 0;

  if ( nglSyncDebug.DumpSceneFile )
    nglDebug.DumpSceneFile = 0;

  if ( nglSyncDebug.DumpTextures )
    nglDebug.DumpTextures = 0;

  nglSyncDebug = nglDebug;

  // Set up the initial scene.
  nglCurScene = NULL;
  nglListBeginScene();

  // Init default animation time.
  nglDefaultScene.AnimTime = nglVBlankCount * ( 1.0f / 60.0f );

  // Show performance info from the previous render list.
  if ( DEBUG_ENABLE( ShowPerfInfo ) )
  {
    nglQuad q;
    nglInitQuad( &q );
    nglSetQuadRect( &q, nglGetScreenWidth() - 230, 10, nglGetScreenWidth() - 10, 222 );
    nglSetQuadColor( &q, NGL_RGBA32( 0, 0, 0, 192 ) );
    nglSetQuadZ( &q, 0.3f );
    nglListAddQuad( &q );

    nglListAddString( nglSysFont, nglGetScreenWidth() - 220, 20, 0,
			"\1[802020FF]\2[1.1]NGL " NGL_VERSION "\2[1]\1[FFFFFFFF]\n"
			"%.2f FPS\n"
      "%.2fms ASYNC RENDER\n"
      "%.2fms DMA BUILD\n"
      "%d VERTS\n"
      "%d POLYS\n"
      "%d TEX UPLOAD\n"
      "%d NODES\n"
      "LIST  %07d %07d\n"
      "VIF1  %07d %07d\n"
      "GIF   %07d %07d\n"
      "SMESH %07d %07d\n",
      nglPerfInfo.FPS,
      nglPerfInfo.RenderMS,
      nglPerfInfo.ListSendMS,
      nglPerfInfo.TotalVerts,
      nglPerfInfo.TotalPolys,
      nglPerfInfo.TextureDataStreamed,
      nglNodeCount,
      nglPerfInfo.ListWorkBytesUsed, nglPerfInfo.MaxListWorkBytesUsed,
      nglPerfInfo.Vif1WorkBytesUsed, nglPerfInfo.MaxVif1WorkBytesUsed,
      nglPerfInfo.GifWorkBytesUsed, nglPerfInfo.MaxGifWorkBytesUsed,
      nglPerfInfo.ScratchMeshBytesUsed, nglPerfInfo.MaxScratchMeshBytesUsed );
  }

  // If we suffered a DMA timeout, print a message to that effect.
  if ( nglDMATimeoutCounter > 0 )
  {
    u_int Alpha = nglDMATimeoutCounter > 30 ? 0xFF : nglDMATimeoutCounter * 0xFF / 30;

    const char* Text = "[NGL] Vif1 DMA Timeout";
    u_int Width, Height;
    nglGetStringDimensions( nglSysFont, &Width, &Height, Text );

    int Left = nglGetScreenWidth()/2 - (Width+10)/2;

    nglQuad q;
    nglInitQuad( &q );
    nglSetQuadRect( &q, Left, 15, Left + Width + 10, 20 + Height + 5);
    nglSetQuadColor( &q, NGL_RGBA32( 0, 0, 0, Alpha ) );
    nglSetQuadZ( &q, 0.3f );
    nglListAddQuad( &q );

    nglListAddString( nglSysFont, Left + 5, 20, 0, Text );

    nglDMATimeoutCounter--;
  }

  if ( DEBUG_ENABLE( DumpFrameLog ) )
    nglPrintf( "LOG: ============================= Frame log start ===========================\n" );

  nglSceneDumpStart();

  if ( nglSyncDebug.DumpTextures )
    nglDumpTextures();

  nglInScene = true;
}

// @Send - Build and send the render DMA chain.
void nglListSend( bool Flip )
{
  nglInScene = false;

  nglSceneDumpEnd();

  // Check to see if we added too much to the render list.
  nglPerfInfo.ListWorkBytesUsed = nglListWorkPos - nglListWork;
  if ( nglPerfInfo.ListWorkBytesUsed >= nglListWorkSize )
    nglFatal( "List workspace overflow (%d/%d).\n", nglPerfInfo.ListWorkBytesUsed, nglListWorkSize );

#ifndef NGL_ASYNC_LIST_SEND
  if ( Flip )
  {
    // Stall on the previous render and swap the frame buffers.
    STOP_PROF_TIMER( proftimer_render_sendlist );
    STOP_PROF_TIMER( proftimer_render );
    START_PROF_TIMER( proftimer_frame_flip );
    nglFlip();
    STOP_PROF_TIMER( proftimer_frame_flip );
    START_PROF_TIMER( proftimer_render );
    START_PROF_TIMER( proftimer_render_sendlist );
  }
  else
  {
    // Wait for the previous render to finish
    nglVif1SafeWait();
  }
#endif

  START_PROF_TIMER( proftimer_render_sendlist );

  nglPerfInfo.ListSendCycles = nglGetCPUCycle();
#if !NGL_USE_SN_TUNER
  nglPerfInfo.ListSendMisses = scePcGetCounter1();
#endif

  if ( nglCurScene != &nglDefaultScene )
    nglFatal( "nglListSend called while one or more scenes were still active (need to call nglListEndScene).\n" );

  if ( DEBUG_ENABLE( DebugPrints ) )
    nglPrintf( "nglListSend enter\n" );

  nglPerfInfo.TotalPolys = 0;
  nglPerfInfo.TotalVerts = 0;
  nglPerfInfo.TextureDataStreamed = 0;
  nglPerfInfo.ParticleCount = 0;
  nglNodeCount = 0;

  nglFatalDMAError = false;

  // Swap interrupt command list and packet work double buffers.
#if NGL_ASYNC_LIST_SEND
  if ( nglVif1PacketWork == nglVif1PacketWorkA )
    nglVif1PacketWork = nglVif1PacketWorkB;
  else
    nglVif1PacketWork = nglVif1PacketWorkA;

  if ( nglGifPacketWork == nglGifPacketWorkA )
    nglGifPacketWork = nglGifPacketWorkB;
  else
    nglGifPacketWork = nglGifPacketWorkA;
#endif

  nglGifPacketNext = (u_int*)nglGifPacketWork;

  if ( nglVif1IntArray == nglVif1IntArrayA )
    nglVif1IntArray = nglVif1IntArrayB;
  else
    nglVif1IntArray = nglVif1IntArrayA;

  u_int* Packet;
  nglVif1InitPacket( Packet );

  nglDmaStartTag( Packet );

  nglVif1AddTextureStreamSetup( Packet );
  nglDmaEndTag( Packet, SCE_DMA_ID_CNT );

  nglVif1RenderScene( Packet, &nglDefaultScene );

  nglDmaStartTag( Packet );
  nglVif1AddTextureStreamEnd( Packet );

  nglVif1IntAddFinish( Packet );

  nglDmaEndTag( Packet, SCE_DMA_ID_END );
  nglVif1ClosePacket( Packet );

  // Check the Vif and Gif workspaces for overflows.
  nglPerfInfo.Vif1WorkBytesUsed = ( (u_int)Packet & NGL_DMA_MEM ) - (u_int)nglVif1PacketWork;
  if ( nglPerfInfo.Vif1WorkBytesUsed > nglVif1PacketWorkSize )
    nglFatal( "Vif1 packet workspace overflow (%d/%d).\n", nglPerfInfo.Vif1WorkBytesUsed, nglVif1PacketWorkSize );

  nglPerfInfo.GifWorkBytesUsed = ( (u_int)nglGifPacketNext & NGL_DMA_MEM ) - (u_int)nglGifPacketWork;
  if ( nglPerfInfo.GifWorkBytesUsed > nglGifPacketWorkSize )
    nglFatal( "Gif packet workspace overflow (%d/%d).\n", nglPerfInfo.GifWorkBytesUsed, nglGifPacketWorkSize );

#if NGL_PROFILING
  // Check for new maximum values.
  if ( nglPerfInfo.ListWorkBytesUsed > nglPerfInfo.MaxListWorkBytesUsed )
    nglPerfInfo.MaxListWorkBytesUsed = nglPerfInfo.ListWorkBytesUsed;
  if ( nglPerfInfo.Vif1WorkBytesUsed > nglPerfInfo.MaxVif1WorkBytesUsed )
    nglPerfInfo.MaxVif1WorkBytesUsed = nglPerfInfo.Vif1WorkBytesUsed;
  if ( nglPerfInfo.GifWorkBytesUsed > nglPerfInfo.MaxGifWorkBytesUsed )
    nglPerfInfo.MaxGifWorkBytesUsed = nglPerfInfo.GifWorkBytesUsed;
  if ( nglPerfInfo.ScratchMeshBytesUsed > nglPerfInfo.MaxScratchMeshBytesUsed )
    nglPerfInfo.MaxScratchMeshBytesUsed = nglPerfInfo.ScratchMeshBytesUsed;

  nglPerfInfo.ListSendCycles = nglGetCPUCycle() - nglPerfInfo.ListSendCycles;
#if !NGL_USE_SN_TUNER
  nglPerfInfo.ListSendMisses = scePcGetCounter1() - nglPerfInfo.ListSendMisses;
#endif

  nglPerfInfo.ListSendMS = nglPerfInfo.ListSendCycles / NGL_CPU_CLOCKS_PER_MS;
#endif

  // If we detected a problem building the DMA chain, just don't render this one instead of sending a DMA that we
  // know is going to crash.
  if ( !nglFatalDMAError )
  {
#if NGL_ASYNC_LIST_SEND
    if ( Flip )
    {
      // Stall on the previous render and swap the frame buffers.
      STOP_PROF_TIMER( proftimer_render_sendlist );
      STOP_PROF_TIMER( proftimer_render );
      START_PROF_TIMER( proftimer_frame_flip );
      nglFlip();
      STOP_PROF_TIMER( proftimer_frame_flip );
      START_PROF_TIMER( proftimer_render );
      START_PROF_TIMER( proftimer_render_sendlist );
    }
    else
    {
      // Wait for the previous render to finish
      nglVif1SafeWait();
    }
#endif

    // Store the current interrupt work buffer info into the buffered variables used by the interrupt handler.
    _nglVif1IntArray = nglVif1IntArray;
    _nglNVif1IntEntries = nglNVif1IntEntries;
    _nglVif1IntEntryIdx = 0;

    // Reset the interrupt timer.
    nglPerfInfo.RenderStart = nglGetCPUCycle();
    *T1_COUNT = 0;

    nglDMAFinished = false;

    // Flush the main CPU cache.  DO NOT REMOVE THIS CALL ;)
    FlushCache( 0 );

    // Send the data.
    if ( STAGE_ENABLE( DMASend ) )
       _nglDmaStartVif1SourceChain( nglVif1PacketWork, 0 );

    // Turn this one on to test just the speed of the VIF/VU/GS w/o any EE.
    // Check nglPerfInfo.FrameMS and nglPerfInfo.FPS for the timings.
    while ( DEBUG_ENABLE( DMARepeatMode ) )
    {
      nglFlip();
      _nglVif1IntEntryIdx = 0;
      _nglDmaStartVif1SourceChain( nglVif1PacketWork, 0 );
    }
  }

#ifdef ARCH_ENGINE
  SET_PROF_COUNT( profcounter_node_rend, nglNodeCount );
  SET_PROF_COUNT( profcounter_tri_rend, nglPerfInfo.TotalPolys );
#ifdef PROJECT_SPIDERMAN
  SET_PROF_COUNT( profcounter_texture_stream, nglPerfInfo.TextureDataStreamed );
#endif
#endif

  if ( DEBUG_ENABLE( DebugPrints ) )
    nglPrintf( "nglListSend leave\n" );

  if ( DEBUG_ENABLE( SyncRender ) )
    nglVif1SafeWait();

  STOP_PROF_TIMER( proftimer_render_sendlist );
}

/*---------------------------------------------------------------------------------------------------------
  @Scene management code.
---------------------------------------------------------------------------------------------------------*/
void nglSetRenderTarget( nglTexture* Tex, bool Download )
{
  nglCurScene->RenderTarget = Tex;
  nglCurScene->Download = Download;
  Tex->Flags.RenderTarget = 1;
}

void nglSetRequiredTexturesPS2( nglTexture** TexTbl, u_int NTex)
{
  nglTexture** RequiredTextures = (nglTexture **)nglListAlloc(NTex * sizeof(nglTexture *));
  if (RequiredTextures)
  {
    memcpy(RequiredTextures, TexTbl, NTex * sizeof(nglTexture *));
    nglCurScene->RequiredTextures = RequiredTextures;
    nglCurScene->NRequiredTextures = NTex;
  }
}

void nglSetViewport( u_int x1, u_int y1, u_int x2, u_int y2 )
{
  nglCurScene->ViewX1 = x1;
  nglCurScene->ViewY1 = y1;
  nglCurScene->ViewX2 = x2;
  nglCurScene->ViewY2 = y2;
}

void nglSetClearFlags( u_int ClearFlags )
{
  nglCurScene->ClearFlags = ClearFlags;
}

void nglSetClearColor( float R, float G, float B, float A )
{
  nglCurScene->ClearColor[0] = nglFTOI( R * 255.0f );
  nglCurScene->ClearColor[1] = nglFTOI( G * 255.0f );
  nglCurScene->ClearColor[2] = nglFTOI( B * 255.0f );
  nglCurScene->ClearColor[3] = nglFTOI( A * 255.0f );
}

void nglSetClearZ( float Z )
{
  nglCurScene->ClearZ = Z;
}

void nglSetFBWriteMask( u_int WriteMask )
{
  nglCurScene->FBWriteMask = WriteMask;
}

void nglSetZWriteEnable( bool enable )
{
  nglCurScene->ZWriteEnable = enable;
}

void nglSetZTestEnable( bool enable )
{
  nglCurScene->ZTestEnable = enable;
}

void nglSetFogColor( float R, float G, float B )
{
  nglCurScene->FogColor[0] = nglFTOI( R * 255.0f );
  nglCurScene->FogColor[1] = nglFTOI( G * 255.0f );
  nglCurScene->FogColor[2] = nglFTOI( B * 255.0f );
}

void nglSetFogRange( float Near, float Far, float Min, float Max )
{
  nglCurScene->FogNear = Near;
  nglCurScene->FogFar = Far;
  nglCurScene->FogMin = Min;
  nglCurScene->FogMax = Max;
}

void nglSetAnimTime( float Time )
{
  nglCurScene->AnimTime = Time;
}

void nglGetProjectionParams( float* hfov, float* cx, float* cy, float* nearz, float* farz )
{
  if ( hfov ) *hfov = nglCurScene->HFOV;
  if ( cx ) *cx = nglCurScene->CX;
  if ( cy ) *cy = nglCurScene->CY;
  if ( nearz ) *nearz = nglCurScene->NearZ;
  if ( farz ) *farz = nglCurScene->FarZ;
}

void nglCalculateMatrices( nglScene *scene, int RenderWidth = 0, int RenderHeight = 0 )
{
  if ( RenderWidth == 0 )
    RenderWidth = scene->RenderTarget->Width;

  if ( RenderHeight == 0 )
    RenderHeight = scene->RenderTarget->Height;

  if ( scene->UsingOrtho )
  {
  /*
  // screen aspect ratio.
  float ax = renderwidth / 2 * 1.0f;
  float ay = renderheight / 2 * -1.0f; //-0.47f;

    // constants based on the Z buffer bit depth.
    float zmin = scene->ZMin;
    float zmax = scene->ZMax * NGL_Z_MAX;

      // translate the screen center into GS range.
      float cx = scene->CX + 2047.0f - renderwidth / 2;
      float cy = scene->CX + 2047.0f - renderheight / 2;

        float nearz = scene->NearZ;
        float farz = scene->FarZ;

          float az, cz;
          cz = (zmin * farz - zmax * nearz) / (farz - nearz);
          az = farz * nearz * (zmax - zmin) / (farz - nearz);

            sceVu0UnitMatrix( scene->ViewToScreen );
            scene->ViewToScreen[0][0] = ax;  scene->ViewToScreen[1][1] = ay;  scene->ViewToScreen[2][2] = 1.0f;
            scene->ViewToScreen[3][0] = cx;  scene->ViewToScreen[3][1] = cy;  scene->ViewToScreen[3][2] = cz;
    */
  }
  else
  {
    float ax = 1.0f;
    float ay = - (nglAspectRatio * ((float) RenderHeight) / ((float) RenderWidth));

    // constants based on the Z buffer bit depth.
    float zmin = 1.0f;
    float zmax = NGL_Z_MAX;

    // calculate the distance from the screen to the eye point.
    float scrz = ((float)(RenderWidth/2)) / tanf( ((scene->HFOV * 3.14159f) / 180.0f) / 2.0f );
    scene->ScrZ = scrz;

    // weird srini variable use that I don't understand.
    float cxOrig = scene->CX;
    float cyOrig = scene->CY;

    // translate the screen center into GS range.
    float cx = scene->CX + 2048.0f - ((float)RenderWidth / 2);
    float cy = scene->CY + 2048.0f - ((float)RenderHeight / 2);

    float nearz = scene->NearZ;
    float farz = scene->FarZ;

    float gsx, gsy;
    gsx = nearz*cx/scrz;
    gsy = nearz*cy/scrz;

    float az, cz;
    cz = (zmin * farz - zmax * nearz) * scene->InvFarZMinusNearZ;
    az = farz * nearz * (zmax - zmin) * scene->InvFarZMinusNearZ;

    //                  | scrz    0  0 0 |
    // nglViewToScreen = |    0 scrz  0 0 |
    //                  |    0    0  0 1 |
    //                  |    0    0  1 0 |
    sceVu0UnitMatrix(SCE_MATRIX(scene->ViewToScreen));
    scene->ViewToScreen[0][0] = scrz;
    scene->ViewToScreen[1][1] = scrz;
    scene->ViewToScreen[2][2] = 0.0f;
    scene->ViewToScreen[3][3] = 0.0f;
    scene->ViewToScreen[3][2] = 1.0f;
    scene->ViewToScreen[2][3] = 1.0f;

    //                  | ax  0  0  0 |
    // nglViewToScreen = |  0 ay  0  0 | * mt
    //                  |  0  0 az  0 |
    //                  | cx cy cz  1 |
    nglMatrix mt;
    sceVu0UnitMatrix(SCE_MATRIX(mt));
    mt[0][0] = ax;  mt[1][1] = ay;  mt[2][2] = az;
    mt[3][0] = cx;  mt[3][1] = cy;  mt[3][2] = cz;

    sceVu0MulMatrix(SCE_MATRIX(scene->ViewToScreen), SCE_MATRIX(mt), SCE_MATRIX(scene->ViewToScreen) );

    scene->ViewToScreen[2][2] /= 16.0f;    // Neccessary for fogging.
    scene->ViewToScreen[3][2] /= 16.0f;

    // Create the no FTOI version.
    /*
    sceVu0CopyMatrix( scene->ViewToScreenNoFTOI, scene->ViewToScreen );
    scene->ViewToScreenNoFTOI[2][0] += (float)0x80000;
    scene->ViewToScreenNoFTOI[2][1] += (float)0x80000;
    scene->ViewToScreenNoFTOI[2][2] += (float)0x80000;
    */

    // Reduce the guard band in clipping to minimize hardware interpolation errors
    // Translate the screen center into GS range.
    float cxPrev=cx,cyPrev=cy;
    cx = cxOrig + nglGuardBandEdge - ((float)RenderWidth / 2);
    cy = cyOrig + nglGuardBandEdge - ((float)RenderHeight / 2);
    gsx = nearz*cx/scrz;
    gsy = nearz*cy/scrz;

    //                |2n/2gsx    0        0           0      |
    // nglViewToClip = |   0    2n/2gsy     0           0      |
    //                |   0       0   (f+n)/(f-n)      1      |
    //                |   0       0   -2f*n/(f-n)      0      |
    sceVu0UnitMatrix(SCE_MATRIX(scene->ViewToClip));
    scene->ViewToClip[0][0] = 2.0f * nearz / (gsx-(-gsx));
    scene->ViewToClip[1][1] = 2.0f * nearz / (gsy-(-gsy));
    scene->ViewToClip[2][2] = (farz + nearz) * scene->InvFarZMinusNearZ;
    scene->ViewToClip[3][2] = -2.0f*(farz * nearz) * scene->InvFarZMinusNearZ;
    scene->ViewToClip[2][3] = 1.0f;
    scene->ViewToClip[3][3] = 0.0f;

    cx=cxPrev; cy=cyPrev;
    sceVu0UnitMatrix(SCE_MATRIX(scene->ClipToScreen));
    scene->ClipToScreen[0][0] = scrz*ax*gsx/nearz;
    scene->ClipToScreen[1][1] = scrz*ay*gsy/nearz;
    scene->ClipToScreen[2][2] = (-zmax+zmin)/2;
    scene->ClipToScreen[3][2] = (zmax+zmin)/2;
    scene->ClipToScreen[3][0] = cx;
    scene->ClipToScreen[3][1] = cy;
    //scene->ClipToScreen[3][2] = cz;
    scene->ClipToScreen[3][3] = 1.0f;

    scene->ClipToScreen[2][2] /= 16.0f;  // Neccessary for fogging.
    scene->ClipToScreen[3][2] /= 16.0f;

    sceVu0MulMatrix( SCE_MATRIX(scene->WorldToScreen), SCE_MATRIX(scene->ViewToScreen), SCE_MATRIX(scene->WorldToView) );
    sceVu0MulMatrix( SCE_MATRIX(scene->WorldToScreenNoFTOI), SCE_MATRIX(scene->ViewToScreenNoFTOI), SCE_MATRIX(scene->WorldToView) );
    sceVu0MulMatrix( SCE_MATRIX(scene->WorldToClip), SCE_MATRIX(scene->ViewToClip), SCE_MATRIX(scene->WorldToView) );
    sceVu0InversMatrix( SCE_MATRIX(scene->ViewToWorld), SCE_MATRIX(scene->WorldToView) );
    for ( int i = 0; i < NGLCLIP_MAX; i++ )
    {
      nglApplyMatrixPlane( scene->WorldClipPlanes[i], scene->ViewToWorld, scene->ClipPlanes[i] );
      nglApplyMatrixPlane( scene->WorldGBClipPlanes[i], scene->ViewToWorld, scene->GBClipPlanes[i] );
    }

    scene->ViewPos = *(transform_t*)&scene->ViewToWorld * point_t( 0, 0, 0 );
    scene->ViewDir = *(transform_t*)&scene->ViewToWorld * vector_t( 0, 0, 1 );
  }
}

// Set up the nglViewToScreen and nglViewToClip matrices.
void nglSetPerspectiveMatrix( float hfov, float cx, float cy, float nearz, float farz,
               float zmin, float zmax, int renderwidth, int renderheight )
{
  // Hack to allow us to turn off clipping for ortho transform, since clipping doesn't
  // yet work for this case.  Remove when fixed (dc 09/30/01)
  nglCurScene->UsingOrtho = false;

  if ( renderwidth == 0 )
    renderwidth = nglCurScene->RenderTarget->Width;

  if ( renderwidth == 0 )
    renderwidth = nglCurScene->RenderTarget->Height;

  // Cache values.
  nglCurScene->HFOV = hfov;
  nglCurScene->CX = cx;
  nglCurScene->CY = cy;
  nglCurScene->NearZ = nearz;
  nglCurScene->FarZ = farz;
  nglCurScene->InvFarZMinusNearZ = 1.0f / (nglCurScene->FarZ - nglCurScene->NearZ);
  nglCurScene->ZMin = zmin;
  nglCurScene->ZMax = zmax;
  nglCurScene->RenderW = renderwidth;
  nglCurScene->RenderH = renderheight;

  // calculate the distance from the screen to the eye point.
  hfov = ( nglCurScene->HFOV * 3.14159f/180.0f ) / 2.0f;
  float scrz = ( renderwidth / 2 ) / tanf( hfov );

  // Initialize the clip planes (these are for mesh sphere rejection only).
  nglCurScene->ClipPlanes[NGLCLIP_NEAR][0] = 0.0f;
  nglCurScene->ClipPlanes[NGLCLIP_NEAR][1] = 0.0f;
  nglCurScene->ClipPlanes[NGLCLIP_NEAR][2] = 1.0f;
  nglCurScene->ClipPlanes[NGLCLIP_NEAR][3] = nearz;

  nglCurScene->ClipPlanes[NGLCLIP_FAR][0] = 0.0f;
  nglCurScene->ClipPlanes[NGLCLIP_FAR][1] = 0.0f;
  nglCurScene->ClipPlanes[NGLCLIP_FAR][2] = -1.0f;
  nglCurScene->ClipPlanes[NGLCLIP_FAR][3] = -farz;

  nglCurScene->ClipPlanes[NGLCLIP_LEFT][0] = cosf(hfov);  // lefT
  nglCurScene->ClipPlanes[NGLCLIP_LEFT][1] = 0.0f;
  nglCurScene->ClipPlanes[NGLCLIP_LEFT][2] = sinf(hfov);
  nglCurScene->ClipPlanes[NGLCLIP_LEFT][3] = 0.0f;

  nglCurScene->ClipPlanes[NGLCLIP_RIGHT][0] = -cosf(hfov);  // righT
  nglCurScene->ClipPlanes[NGLCLIP_RIGHT][1] = 0.0f;
  nglCurScene->ClipPlanes[NGLCLIP_RIGHT][2] = sinf(hfov);
  nglCurScene->ClipPlanes[NGLCLIP_RIGHT][3] = 0.0f;

  nglCurScene->ClipPlanes[NGLCLIP_TOP][0] = 0.0f;        // Top
  nglCurScene->ClipPlanes[NGLCLIP_TOP][1] = cosf(hfov);
  nglCurScene->ClipPlanes[NGLCLIP_TOP][2] = sinf(hfov);
  nglCurScene->ClipPlanes[NGLCLIP_TOP][3] = 0.0f;

  nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][0] = 0.0f;       // boTTom
  nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][1] = -cosf(hfov);
  nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][2] = sinf(hfov);
  nglCurScene->ClipPlanes[NGLCLIP_BOTTOM][3] = 0.0f;

  // Initialize the outer planes (these are used to test what needs to be scissored).
  float gfov = atanf( (nglGuardBandEdge/2) / scrz );

  nglCurScene->GBClipPlanes[NGLCLIP_NEAR][0] = 0.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_NEAR][1] = 0.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_NEAR][2] = 1.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_NEAR][3] = nearz;

  nglCurScene->GBClipPlanes[NGLCLIP_FAR][0] = 0.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_FAR][1] = 0.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_FAR][2] = -1.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_FAR][3] = -farz;

  nglCurScene->GBClipPlanes[NGLCLIP_LEFT][0] = cosf(gfov);  // left
  nglCurScene->GBClipPlanes[NGLCLIP_LEFT][1] = 0.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_LEFT][2] = sinf(gfov);
  nglCurScene->GBClipPlanes[NGLCLIP_LEFT][3] = 0.0f;

  nglCurScene->GBClipPlanes[NGLCLIP_RIGHT][0] = -cosf(gfov);  // right
  nglCurScene->GBClipPlanes[NGLCLIP_RIGHT][1] = 0.0f;
  nglCurScene->GBClipPlanes[NGLCLIP_RIGHT][2] = sinf(gfov);
  nglCurScene->GBClipPlanes[NGLCLIP_RIGHT][3] = 0.0f;

  nglCurScene->GBClipPlanes[NGLCLIP_TOP][0] = 0.0f;        // top
  nglCurScene->GBClipPlanes[NGLCLIP_TOP][1] = cosf(gfov);
  nglCurScene->GBClipPlanes[NGLCLIP_TOP][2] = sinf(gfov);
  nglCurScene->GBClipPlanes[NGLCLIP_TOP][3] = 0.0f;

  nglCurScene->GBClipPlanes[NGLCLIP_BOTTOM][0] = 0.0f;       // bottom
  nglCurScene->GBClipPlanes[NGLCLIP_BOTTOM][1] = -cosf(gfov);
  nglCurScene->GBClipPlanes[NGLCLIP_BOTTOM][2] = sinf(gfov);
  nglCurScene->GBClipPlanes[NGLCLIP_BOTTOM][3] = 0.0f;

  nglCalculateMatrices(nglCurScene);
}

void nglSetOrthoMatrix( float cx, float cy, float nearz, float farz, float zmin, float zmax )
{
  // Hack to allow us to turn off clipping for ortho transform, since clipping doesn't
  // yet work for this case.  Remove when fixed (dc 09/30/01)
  nglCurScene->UsingOrtho = true;

  // Cache values.
  nglCurScene->CX = cx;
  nglCurScene->CY = cy;
  nglCurScene->NearZ = nearz;
  nglCurScene->FarZ = farz;
  nglCurScene->InvFarZMinusNearZ = 1.0f / (nglCurScene->FarZ - nglCurScene->NearZ);

//	int RenderWidth = nglCurScene->RenderTarget->Width;
//	int RenderHeight = nglCurScene->RenderTarget->Height;

  // screen aspect ratio.
  float ax = nglCurScene->RenderTarget->Width / 2 * 1.0f;
  float ay = nglCurScene->RenderTarget->Height / 2 * -1.0f; //-0.47f;
//	float ax = 1.0f;
//	float ay = - (nglAspectRatio * ((float)(RenderWidth / RenderHeight)));

  // constants based on the Z buffer bit depth.
  zmin = 1.0f;
  zmax = NGL_Z_MAX;

  // translate the screen center into GS range.
  cx = cx + 2047.0f - nglCurScene->RenderTarget->Width / 2;
  cy = cy + 2047.0f - nglCurScene->RenderTarget->Height / 2;

  float az, cz;
  cz = (zmin * farz - zmax * nearz) * nglCurScene->InvFarZMinusNearZ;
  az = farz * nearz * (zmax - zmin) * nglCurScene->InvFarZMinusNearZ;

  sceVu0UnitMatrix( SCE_MATRIX(nglCurScene->ViewToScreen) );
  nglCurScene->ViewToScreen[0][0] = ax;  nglCurScene->ViewToScreen[1][1] = ay;  nglCurScene->ViewToScreen[2][2] = az;
  nglCurScene->ViewToScreen[3][0] = cx;  nglCurScene->ViewToScreen[3][1] = cy;  nglCurScene->ViewToScreen[3][2] = cz;

  // Clipping not supported in ortho yet.
}

/*
void nglSetOrthoMatrix( float cx, float cy, float nearz, float farz, float zmin, float zmax )
{
  // Hack to allow us to turn off clipping for ortho transform, since clipping doesn't
  // yet work for this case.  Remove when fixed (dc 09/30/01)
  nglCurScene->UsingOrtho = true;

  // Cache values.
  nglCurScene->CX = cx;
  nglCurScene->CY = cy;
  nglCurScene->NearZ = nearz;
  nglCurScene->FarZ = farz;
  nglCurScene->ZMin = zmin;
  nglCurScene->ZMax = zmax;

  // Clipping not supported in ortho yet.
  nglCalculateMatrices(nglCurScene);
}
*/

void nglSetWorldToViewMatrix( const nglMatrix& WorldToView )
{
  sceVu0CopyMatrix( SCE_MATRIX(nglCurScene->WorldToView), SCE_MATRIX(WorldToView) );
  sceVu0InversMatrix( SCE_MATRIX(nglCurScene->ViewToWorld), SCE_MATRIX(nglCurScene->WorldToView) );
  sceVu0MulMatrix( SCE_MATRIX(nglCurScene->WorldToScreen), SCE_MATRIX(nglCurScene->ViewToScreen), SCE_MATRIX(nglCurScene->WorldToView) );
  sceVu0MulMatrix( SCE_MATRIX(nglCurScene->WorldToScreenNoFTOI), SCE_MATRIX(nglCurScene->ViewToScreenNoFTOI), SCE_MATRIX(nglCurScene->WorldToView) );
  sceVu0MulMatrix( SCE_MATRIX(nglCurScene->WorldToClip), SCE_MATRIX(nglCurScene->ViewToClip), SCE_MATRIX(nglCurScene->WorldToView) );
  for ( int i = 0; i < NGLCLIP_MAX; i++ )
  {
    nglApplyMatrixPlane( nglCurScene->WorldClipPlanes[i], nglCurScene->ViewToWorld, nglCurScene->ClipPlanes[i] );
    nglApplyMatrixPlane( nglCurScene->WorldGBClipPlanes[i], nglCurScene->ViewToWorld, nglCurScene->GBClipPlanes[i] );
  }

  nglCurScene->ViewPos = *(transform_t*)&nglCurScene->ViewToWorld * point_t( 0, 0, 0 );
  nglCurScene->ViewDir = *(transform_t*)&nglCurScene->ViewToWorld * vector_t( 0, 0, 1 );
}

void nglGetMatrix( nglMatrix& Dest, u_int ID )
{
  switch ( ID )
  {
  case NGLMTX_VIEW_TO_WORLD:
    Dest = nglCurScene->ViewToWorld;
    break;
  case NGLMTX_VIEW_TO_SCREEN:
    Dest = nglCurScene->ViewToScreen;
    break;
  case NGLMTX_WORLD_TO_VIEW:
    Dest = nglCurScene->WorldToView;
    break;
  case NGLMTX_WORLD_TO_SCREEN:
    Dest = nglCurScene->WorldToScreen;
    break;
  }
}

void nglProjectPoint( nglVector& Out, nglVector& In )
{
  In[3] = 1.0f;
  sceVu0ApplyMatrix( Out, SCE_MATRIX(nglCurScene->WorldToScreen), In );
  float InvW = 1.0f / Out[3];
  Out[0] = ( Out[0] * InvW ) - ( 2048 - nglGetScreenWidth() / 2 );
  Out[1] = ( Out[1] * InvW ) - ( 2048 - nglGetScreenHeight() / 2 );
  Out[2] = ( Out[2] * InvW );
}

void nglVif1RenderSceneNode( u_int*& Packet, void* Param )
{
  nglScene* Scene = (nglScene*)Param;
  nglVif1RenderScene(Packet, Scene);
  //if Scene and Scene->Parent have different setup then...
  //(for now, assume they are different)
  nglVif1SetupScene(Packet, Scene->Parent, /*ClearEnable*/ false);
}

nglScene *nglListBeginScene(nglSortInfo *SortInfo)
{
  nglScene* Scene;
  if ( !nglCurScene )
	Scene = &nglDefaultScene;
  else
  {
    // Add the new scene at the end of the current scene's child list.
    Scene = nglListNew( nglScene );
    if ( !Scene )
      return (nglScene *)0;
	if (!SortInfo)
	{
	  if ( nglCurScene->LastChild )
		nglCurScene->LastChild->NextSibling = Scene;
	  else
		nglCurScene->FirstChild = Scene;
	  nglCurScene->LastChild = Scene;
	}

    // Set up the scene based on nglDefaultScene (whose parameters can be set outside the render loop).
    memcpy( Scene, &nglDefaultScene, sizeof(nglScene) );
  }

  // Initialize the new scene's hierarchy linkage.
  Scene->Parent = nglCurScene;
  Scene->NextSibling = NULL;
  Scene->FirstChild = NULL;
  Scene->LastChild = NULL;

  // Initialize the new scene's bin structure.
  Scene->OpaqueRenderList = NULL;
  Scene->TransRenderList = NULL;
  Scene->OpaqueListCount = 0;
  Scene->TransListCount = 0;

  if (SortInfo)
    nglListAddNode( NGLNODE_SCENE, nglVif1RenderSceneNode, (void *)Scene, SortInfo );

  nglCurScene = Scene;

  return Scene;
}

void nglListEndScene()
{
  if ( nglCurScene == &nglDefaultScene )
    nglFatal( "Scene stack underflow (too many nglListEndScene calls!).\n" );
  nglCurScene = nglCurScene->Parent;
}

nglScene* nglListSelectScene(nglScene *scene)
{
  nglScene *prev = nglCurScene;
  nglCurScene = scene;
  return prev;
}

/*---------------------------------------------------------------------------------------------------------
  @Scene rendering code
---------------------------------------------------------------------------------------------------------*/

#define NGL_SPR_NODE_THRESHOLD ( 4 * 1024 )
#define NGL_SPR_BUF_SIZE ( 12 * 1024 )

void nglVif1FlushSPAD( u_int*& Packet, bool Force )
{
#if NGL_USE_SCRATCHPAD
  NGL_ASSERT( (u_int)Packet - NGL_SCRATCHPAD_MEM < NGL_SPR_BUF_SIZE, "Node exceeded Vif1 packet size limit (%d/%d)." );

  if ( (u_int)Packet == NGL_SCRATCHPAD_MEM )
    return;

  if ( Force || (u_int)Packet - NGL_SCRATCHPAD_MEM >= NGL_SPR_NODE_THRESHOLD )
  {
    u_int QWC = ( (u_int)Packet - NGL_SCRATCHPAD_MEM ) / 16;
    _nglDmaStartFromSPRNormal( nglVif1Packet, NGL_SCRATCHPAD_MEM, QWC );
    while ( *D8_CHCR & D_CHCR_STR_M );
    nglVif1Packet += QWC * 4;
    Packet = (u_int*)NGL_SCRATCHPAD_MEM;
  }
#endif
}

// Draws a rectangle on screen using 32-pixel wide vertical strips (optimizes GS performance).
void nglVif1AddFastRect( u_int*& Packet, nglScene *Scene, nglVector& Rect, u_int Z, sceVu0IVECTOR Color )
{
  nglVif1AddRawGSReg( SCE_GS_PRIM, SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
  nglVif1AddRawGSReg( SCE_GS_PRMODE, SCE_GS_SET_PRMODE( 0, 0, 0, 0, 0, 0, 0, 0 ) );
  nglVif1AddRawGSReg( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( Color[0], Color[1], Color[2], Color[3], 0x3f800000 ) );

  nglTexture* Target = Scene->RenderTarget;
  int x1 = nglFTOI( Rect[0] + 2048 - ( Target->Width / 2 ) );
  int y1 = nglFTOI( Rect[1] + 2048 - ( Target->Height / 2 ) );
  int x2 = nglFTOI( Rect[2] + 2048 - ( Target->Width / 2 ) );
  int y2 = nglFTOI( Rect[3] + 2048 - ( Target->Height / 2 ) );

  // Left sprite - align to a 32 pixel boundary.
  if ( x1 & 31 )
  {
    nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( x1 << 4, y1 << 4, Z ) );
    nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( ( ( x1 + 31 ) & ~31 ) << 4, ( y2 + 1 ) << 4, Z ) );
    x1 = ( x1 + 31 ) & ~31;
  }

  // Middle - write 32 pixel wide vertical strips.
  while ( x1 + 31 < x2 )
  {
    nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( x1 << 4, y1 << 4, Z ) );
    nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( ( x1 + 32 ) << 4, ( y2 + 1 ) << 4, Z ) );
    x1 += 32;
  }

  // Right edge - write the final strip to complete the rectangle.
  nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( x1 << 4, y1 << 4, Z ) );
  nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( ( x2 + 1 ) << 4, ( y2 + 1 ) << 4, Z ) );
}

// Set up the initial rendering environment for a scene.
void nglVif1AddSetDrawEnv( u_int*& Packet, nglScene *Scene, bool ClearEnable )
{
  nglTexture* Target = Scene->RenderTarget;

  // If it's a streamed target, ensure it has been allocated a position in vram
  if ( !Target->Flags.Locked )
  {
    // Note: nglVif1AddTextureStreaming implicitly references nglCurScene->RenderTarget,
    //   so we must set nglCurScene to Scene temporarily when calling it!
    nglScene *nglCurScene_save = nglCurScene;
    nglCurScene = Scene;
    nglVif1AddTextureStreaming( Packet, Scene->RequiredTextures, Scene->NRequiredTextures);
    nglCurScene = nglCurScene_save;
  }

  nglVif1StartDirectGifAD();

  // Flush the frame buffer (containing the previous scene) to VRAM, in case we need to texture from it.
  nglVif1AddRawGSReg( SCE_GS_TEXFLUSH,   0 );

  // Frame buffer/Z buffer address and size.
  if ( Target->Format == SCE_GS_PSMCT16 )
  {
    nglVif1AddRawGSReg( SCE_GS_FRAME_1,    SCE_GS_SET_FRAME( Target->GsBaseTBP / 32, ( Target->Width + 63 ) / 64, SCE_GS_PSMCT16S, ~Scene->FBWriteMask ) );
    nglVif1AddRawGSReg( SCE_GS_FBA_1,      SCE_GS_SET_FBA( 1 ) );          // Alpha correction enabled.
  }
  else
  {
    nglVif1AddRawGSReg( SCE_GS_FRAME_1,    SCE_GS_SET_FRAME( Target->GsBaseTBP / 32, ( Target->Width + 63 ) / 64, Target->Format, ~Scene->FBWriteMask ) );
    nglVif1AddRawGSReg( SCE_GS_FBA_1,      SCE_GS_SET_FBA( 0 ) );          // Alpha correction disabled.
  }
  if (Scene->ZWriteEnable)
    nglVif1AddRawGSReg( SCE_GS_ZBUF_1,     SCE_GS_SET_ZBUF( NGL_VRAM_ZBUFFER, NGL_Z_FORMAT, 0 ) );
  else
    nglVif1AddRawGSReg( SCE_GS_ZBUF_1,     SCE_GS_SET_ZBUF( NGL_VRAM_ZBUFFER, NGL_Z_FORMAT, 1 ) );

  Scene->Cache.WriteFBA = true;

  // Offset from primitive coordinates to window coordinates.  We always render to 'screen coordinates' even if we're rendering to
  // a texture.  The GS handles the conversion.  Right? :)
  nglVif1AddRawGSReg( SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( ( 2048 - Target->Width / 2 ) * 16, ( 2048 - Target->Height / 2 ) * 16 ) );

  // Screen viewport setup.
  nglVif1AddRawGSReg( SCE_GS_SCISSOR_1,  SCE_GS_SET_SCISSOR( Scene->ViewX1, Scene->ViewX2, Scene->ViewY1, Scene->ViewY2 ) );

  // Misc stuff.
  nglVif1AddRawGSReg( SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT( 0 ) );   // Enable the PRMODE register.
  nglVif1AddRawGSReg( SCE_GS_COLCLAMP,   SCE_GS_SET_COLCLAMP( 1 ) );     // Clamp colors at 255 (as opposed to wrapping!).
  nglVif1AddRawGSReg( SCE_GS_DTHE,       SCE_GS_SET_DTHE( 0 ) );         // Dithering disabled (should we turn it on for 16-bit modes?)

  // Screen clear.
  if ( Scene->ClearFlags && ClearEnable)
  {
    // Clear everything.
    if ( Scene->ClearFlags == ( NGLCLEAR_COLOR | NGLCLEAR_Z ) )
      nglVif1AddRawGSReg( SCE_GS_TEST_1, SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ) );
    else  // Clear just color or just Z.
      nglVif1AddRawGSReg( SCE_GS_TEST_1, SCE_GS_SET_TEST( 1, 0, 0, Scene->ClearFlags, 0, 0, 1, 1 ) );

    nglVector Rect( Scene->ViewX1, Scene->ViewY1, Scene->ViewX2, Scene->ViewY2 );
    nglVif1AddFastRect( Packet, Scene, Rect, nglHardwareZToGS( Scene->ClearZ ), Scene->ClearColor );
  }

  nglVif1EndDirectGifAD( Packet );

  nglVif1ResetGSRegisterCache();
}

void nglVif1AddGiftagSetup( u_int*& _Packet )
{
  u_int* Packet = _Packet;

  u_int TriFanPrim = SCE_GS_PRIM_TRIFAN;
  u_int Prim = SCE_GS_PRIM_TRISTRIP;

  if ( DEBUG_ENABLE( DrawAsLines ) )
  {
    TriFanPrim = SCE_GS_PRIM_LINESTRIP;
    Prim = SCE_GS_PRIM_LINESTRIP;
  }
  else
  if ( DEBUG_ENABLE( DrawAsPoints ) )
  {
    TriFanPrim = SCE_GS_PRIM_POINT;
    Prim = SCE_GS_PRIM_POINT;
  }

  // Set the GIFTags in VU memory.
  Packet[0] = SCE_VIF1_SET_UNPACK( NGLMEM_STRIP_GIFTAG, 3, SCE_VIF1_V4_32, 0 );

  u_long Tag;
  Tag = SCE_GIF_SET_TAG( 0, 1, 1, Prim, SCE_GIF_PACKED, 3 );    // standard giftag
  Packet[1] = Tag & 0xFFFFFFFF;
  Packet[2] = Tag >> 32;
  Packet[3] = 0x412;
  Packet[4] = 0;
  Packet += 5;

  Tag = SCE_GIF_SET_TAG( 0, 1, 1, TriFanPrim, SCE_GIF_PACKED, 3 );  // trifan giftag, for the scissor function
  Packet[0] = Tag & 0xFFFFFFFF;
  Packet[1] = Tag >> 32;
  Packet[2] = 0x412;
  Packet[3] = 0;
  Packet += 4;

  Tag = SCE_GIF_SET_TAG( 0, 1, 0, 0, SCE_GIF_PACKED, 1 );       // dummy giftag.
  Packet[0] = Tag & 0xFFFFFFFF;
  Packet[1] = Tag >> 32;
  Packet[2] = 0;
  Packet[3] = 0;
  Packet += 4;

  _Packet = Packet;
}

class nglOpaqueCompare
{
public:
  bool operator()( const pair<nglListNode*,u_int>& NodeA, const pair<nglListNode*,u_int>& NodeB )
  {
    // Sort by ascending hash
    return NodeA.second < NodeB.second;
  }
};

class nglTransCompare
{
public:
  bool operator()( const pair<nglListNode*,u_int>& NodeA, const pair<nglListNode*,u_int>& NodeB )
  {
    // Sort by descending dist, preserving submission order for equal dists
    if ( (float&)NodeA.second > (float&)NodeB.second ) return true;
    else if ( (float&)NodeA.second < (float&)NodeB.second ) return false;
    else
    // Preserve submission order for nodes at the same distance
    // (the pointer value can be used for this, because nodes are allocated sequentially each frame in memory).
    return (u_int)NodeA.first < (u_int)NodeB.first;
  }
};

void nglOpenRenderList( pair<nglListNode*,u_int>* NodeTbl, nglListNode* List, u_int Size )
{
  pair<nglListNode*,u_int>* p = NodeTbl;
  while ( List )
  {
    p->first = List;
    p->second = List->SortInfo.Hash;     // technically this is both hash and dist.
    p++;
    List = List->Next;
  }
}

void nglCloseRenderList( pair<nglListNode*,u_int>* NodeTbl, nglListNode *(&ListHead), u_int Size )
{
  pair<nglListNode*,u_int>* p = NodeTbl + ( Size - 1 );
  nglListNode* Prev = NULL;
  for ( ; Size; --p, --Size )
  {
    p->first->Next = Prev;
    Prev = p->first;
  }
  ListHead = Prev;
}

static nglListNode* nglPrevNode = 0;

void nglVif1SetupScene( u_int*& Packet, nglScene* Scene, bool ClearEnable )
{
  nglDmaStartTag( Packet );

  // Reset some VIF1 registers.
  Packet[0] = SCE_VIF1_SET_BASE( 0, 0 );
  Packet[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
  Packet[2] = SCE_VIF1_SET_STCYCL( 1, 1, 0 );
  Packet[3] = SCE_VIF1_SET_ITOP( 0, 0 );
  Packet += 4;

  // Calculate fog coefficients.
  nglVector FogVal( Scene->FogNear, 1.0f / ( Scene->FogFar - Scene->FogNear ), Scene->FogMin * 255.0f, ( Scene->FogMax - Scene->FogMin ) * 255.0f );
  nglVif1AddUnpack( Packet, NGLMEM_FOG_VAL, SCE_VIF1_V4_32, 1, &FogVal, sizeof(float) * 4 );
  nglVif1AddUnpack( Packet, NGLMEM_FOG_COLOR, SCE_VIF1_V3_32, 1, &Scene->FogColor, sizeof(float) * 3 );

  // Set up scene constants in VU memory.
  nglVif1AddUnpack( Packet, NGLMEM_WORLD_TO_VIEW, SCE_VIF1_V4_32, 4, Scene->WorldToView, sizeof(nglMatrix) );
  nglVif1AddUnpack( Packet, NGLMEM_WORLD_TO_SCREEN, SCE_VIF1_V4_32, 4, Scene->WorldToScreen, sizeof(nglMatrix) );
  nglVif1AddUnpack( Packet, NGLMEM_WORLD_TO_CLIP, SCE_VIF1_V4_32, 4, Scene->WorldToClip, sizeof(nglMatrix) );
  nglVif1AddUnpack( Packet, NGLMEM_CLIP_TO_SCREEN, SCE_VIF1_V4_32, 4, Scene->ClipToScreen, sizeof(nglMatrix) );
  nglVif1AddUnpack( Packet, NGLMEM_VIEW_TO_WORLD, SCE_VIF1_V4_32, 4, Scene->ViewToWorld, sizeof(nglMatrix) );

  nglVif1AddGiftagSetup( Packet );

  nglVif1AddUnpack( Packet, NGLMEM_SCREEN_MIN, SCE_VIF1_V4_32, 1, nglScreenMinValue, sizeof(nglVector) );
  nglVif1AddUnpack( Packet, NGLMEM_SCREEN_MAX, SCE_VIF1_V4_32, 1, nglScreenMaxValue, sizeof(nglVector) );

  // Set up the global drawing environment for the scene (viewport, framebuffer, fog, etc).
  nglVif1AddSetDrawEnv( Packet, Scene, ClearEnable );

  nglDmaEndTag( Packet, SCE_DMA_ID_CNT );

  nglVif1FlushSPAD( Packet, true );
}

void nglVif1RenderScene( u_int*& Packet, nglScene* Scene )
{
  // First recursively render the children to take care of dependencies.
  nglScene* Child = Scene->FirstChild;
  while ( Child )
  {
    nglVif1RenderScene( Packet, Child );
    Child = Child->NextSibling;
  }

  // Calculate the current IFL frame.
  Scene->IFLFrame = nglFTOI( Scene->AnimTime * nglIFLSpeed );

  // Calculate matrices and clip planes in all necessary spaces.
  nglCalculateMatrices(Scene);

  nglVif1SetupScene( Packet, Scene, /*ClearEnable*/ true );

  START_PROF_TIMER( proftimer_send_sort );
  pair<nglListNode*,u_int>* NodeTbl = (pair<nglListNode*,u_int>*)NGL_SCRATCHPAD_MEM + 8;
  nglOpenRenderList( NodeTbl, Scene->OpaqueRenderList, Scene->OpaqueListCount );
  sort( NodeTbl, NodeTbl + Scene->OpaqueListCount, nglOpaqueCompare() );
  nglCloseRenderList( NodeTbl, Scene->OpaqueRenderList, Scene->OpaqueListCount );
  STOP_PROF_TIMER( proftimer_send_sort );

  // Set nglCurScene to currently rendering scene, so that the node callback can use it.
  // Note: nglVif1AddTextureStreaming is used by many node callback functions, and implicitly
  //  references nglCurScene->RenderTarget.
  nglScene *nglCurScene_save = nglCurScene;
  nglCurScene = Scene;

  // Render the nodes in the scene.
  START_PROF_TIMER( proftimer_send_addnodes );

  nglListNode* Node = Scene->OpaqueRenderList;
  while ( Node )
  {
    if ( !nglEvalTestNode() )
    {
      nglPrevNode = Node;
      Node = Node->Next;
      continue;
    }

    // Execute the callback function that adds this node to the DMA chain.
    Node->NodeFn( Packet, Node->NodeData );

    nglPrevNode = Node;
    Node = Node->Next;
  }
  STOP_PROF_TIMER( proftimer_send_addnodes );

  START_PROF_TIMER( proftimer_send_sort );
  nglVif1FlushSPAD( Packet, true );
  nglOpenRenderList( NodeTbl, Scene->TransRenderList, Scene->TransListCount );
  sort( NodeTbl, NodeTbl + Scene->TransListCount, nglTransCompare() );
  nglCloseRenderList( NodeTbl, Scene->TransRenderList, Scene->TransListCount );
  STOP_PROF_TIMER( proftimer_send_sort );

  START_PROF_TIMER( proftimer_send_addnodes );
  Node = (nglListNode*)Scene->TransRenderList;
  while ( Node )
  {
    if ( !nglEvalTestNode() )
    {
      nglPrevNode = Node;
      Node = Node->Next;
      continue;
    }

    // Execute the callback function that adds this node to the DMA chain.
    Node->NodeFn( Packet, Node->NodeData );

    nglPrevNode = Node;
    Node = Node->Next;
  }
  STOP_PROF_TIMER( proftimer_send_addnodes );

  // Restore nglCurScene to its original value
  nglCurScene = nglCurScene_save;
}

/*---------------------------------------------------------------------------------------------------------
  @Lighting
---------------------------------------------------------------------------------------------------------*/
enum
{
  // Standard lights.
  NGLLIGHT_FAKEPOINT,        // Point light faked by a directional light.
  NGLLIGHT_TRUEPOINT,        // Accurate per-vertex point light.
  NGLLIGHT_DIRECTIONAL,      // General directional light.

  // Projected texture lights.
  NGLLIGHT_PROJECTED_DIRECTIONAL,
  NGLLIGHT_PROJECTED_SPOT,
  NGLLIGHT_PROJECTED_POINT,
};

nglLightContext* nglGlobalLightContext = 0;
nglLightContext* nglCurLightContext = 0;

nglLightContext* nglCreateLightContext()
{
  nglLightContext* Context = nglListNew( nglLightContext );
  if ( !Context )
    return 0;

  // Initialize the circular linked list.
  for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
  {
    Context->VertexHead.Next[i] = &Context->VertexHead;
    Context->TextureHead.Next[i] = &Context->TextureHead;
  }

  Context->Ambient[0] = 1.0f;
  Context->Ambient[1] = 1.0f;
  Context->Ambient[2] = 1.0f;
  Context->Ambient[3] = 1.0f;

  nglCurLightContext = Context;

  return Context;
}

// Switching and retrieving the current lighting context.
void nglSetLightContext( nglLightContext* Context )
{
  nglCurLightContext = Context;
}

nglLightContext* nglGetLightContext()
{
  return nglCurLightContext;
}

void nglSetAmbientLight( float R, float G, float B, float A )
{
  nglCurLightContext->Ambient[0] = R;
  nglCurLightContext->Ambient[1] = G;
  nglCurLightContext->Ambient[2] = B;
  nglCurLightContext->Ambient[3] = A;
}

void nglListAddVertexLightNode( u_int Type, void* NodeData, u_int LightCat )
{
  nglLightNode* NewNode = nglListNew( nglLightNode );
  if ( !NewNode )
    return;

  NewNode->Type = Type;
  NewNode->NodeData = NodeData;
  NewNode->LightCat = LightCat;

  for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
    if ( LightCat & ( 1 << ( NGL_LIGHTCAT_SHIFT + i ) ) )
    {
      NewNode->Next[i] = nglCurLightContext->VertexHead.Next[i];
      nglCurLightContext->VertexHead.Next[i] = NewNode;
    }
}

// Directional lights.
struct nglDirLightInfo
{
  nglVector Dir;
  nglVector Color;
};

void nglListAddDirLight( u_int LightCat, const nglVector& Dir, const nglVector& Color )
{
  nglDirLightInfo* Light = nglListNew( nglDirLightInfo );
  if ( !Light )
    return;

  sceVu0CopyVector( Light->Dir, (nglVector)Dir );
  Light->Dir[3] = 0.0f;
  sceVu0CopyVector( Light->Color, (nglVector)Color );

  nglListAddVertexLightNode( NGLLIGHT_DIRECTIONAL, Light, LightCat );

  // No debug rendering at present for directional lights.

  if ( DEBUG_ENABLE( DumpSceneFile ) )
    nglSceneDumpDirLight( LightCat, Dir, Color );
}

struct nglPointLightInfo
{
  // Position in world space.
  nglVector Pos;
  nglVector Color;

  // Ranges at which falloff starts and finishes.
  float Near;
  float Far;
};

void nglListAddFakePointLight( u_int LightCat, const nglVector& Pos, float Near, float Far, const nglVector& Color )
{
  // Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
  if ( nglGetClipResultViewOnly( (nglVector&)Pos, Far ) == -1 )
    return;

  nglPointLightInfo* Light = nglListNew( nglPointLightInfo );
  if ( !Light )
    return;

  sceVu0CopyVector( Light->Pos, (nglVector)Pos );
  Light->Pos[3] = 1.0f;
  Light->Near = Near;
  Light->Far = Far;
  sceVu0CopyVector( Light->Color, (nglVector)Color );

  nglListAddVertexLightNode( NGLLIGHT_FAKEPOINT, Light, LightCat );

  if ( DEBUG_ENABLE( DrawLightSpheres ) )
  {
    u_int col = nglVectorToColor( Light->Color );
    nglDrawDebugSphere( Light->Pos, 0.25f, col, true );
    nglDrawDebugSphere( Light->Pos, Light->Near, col, false );
    nglDrawDebugSphere( Light->Pos, Light->Far, col, false );
  }

  if ( DEBUG_ENABLE( DumpSceneFile ) )
    nglSceneDumpPointLight( NGLLIGHT_FAKEPOINT, LightCat, Pos, Near, Far, Color );
}

void nglListAddPointLight( u_int LightCat, const nglVector& Pos, float Near, float Far, const nglVector& Color )
{
  // Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
  if ( nglGetClipResultViewOnly( (nglVector&)Pos, Far ) == -1 )
    return;

  nglPointLightInfo* Light = nglListNew( nglPointLightInfo );
  if ( !Light )
    return;

  sceVu0CopyVector( Light->Pos, (nglVector)Pos );
  Light->Pos[3] = 1.0f;
  Light->Near = Near;
  Light->Far = Far;
  sceVu0CopyVector( Light->Color, (nglVector)Color );

  nglListAddVertexLightNode( NGLLIGHT_TRUEPOINT, Light, LightCat );

  if ( DEBUG_ENABLE( DrawLightSpheres ) )
  {
    u_int col = nglVectorToColor( Light->Color );
    nglDrawDebugSphere( Light->Pos, 0.25f, col, true );
    nglDrawDebugSphere( Light->Pos, Light->Near, col, false );
    nglDrawDebugSphere( Light->Pos, Light->Far, col, false );
  }

  if ( DEBUG_ENABLE( DumpSceneFile ) )
    nglSceneDumpPointLight( NGLLIGHT_TRUEPOINT, LightCat, (nglVector)Pos, Near, Far, (nglVector)Color );
}

// v1 minus v2
static inline void nglVectSubtract(nglVector& dest, const nglVector& v1, const nglVector& v2)
{
  dest[0] = v1[0] - v2[0];
  dest[1] = v1[1] - v2[1];
  dest[2] = v1[2] - v2[2];
}

// v1 dot v2
static inline float nglVectDot(const nglVector& v1, const nglVector& v2)
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

// v1 cross v2 for a left handed system
static inline void nglVectCross(nglVector& dest, const nglVector& v1, const nglVector& v2)
{
  dest[0] = v1[2]*v2[1] - v1[1]*v2[2];
  dest[1] = v1[0]*v2[2] - v1[2]*v2[0];
  dest[2] = v1[1]*v2[0] - v1[0]*v2[1];
}

static inline void nglVectNormalize(nglVector& v)
{
  float length;

  length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

  length = 1.0f / length;

  v[0] *= length;
  v[1] *= length;
  v[2] *= length;
}

static inline float nglVectLength(const nglVector& v)
{
  return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

// Computes the 6 planes of the view frustum given a
// projection matrix of any kind (orthographic or perspective)
// and also can incorporate a modelview type transformation
// to compute planes in object space.
void nglBuildFrustum(nglFrustum* frustum, nglMatrix& m)
{
  nglVector lp[8] = // local points
  {
    nglVector( 0.0f, 0.0f, 0.0f, 1.0f ), // 0 - (0,0,0)
    nglVector( 0.0f, 1.0f, 0.0f, 1.0f ), // 1 - (0,1,0)
    nglVector( 1.0f, 1.0f, 0.0f, 1.0f ), // 2 - (1,1,0)
    nglVector( 1.0f, 0.0f, 0.0f, 1.0f ), // 3 - (1,0,0)
    nglVector( 1.0f, 0.0f, 1.0f, 1.0f ), // 4 - (1,0,1)
    nglVector( 1.0f, 1.0f, 1.0f, 1.0f ), // 5 - (1,1,1)
    nglVector( 0.0f, 0.0f, 1.0f, 1.0f ), // 6 - (0,0,1)
    nglVector( 0.0f, 1.0f, 1.0f, 1.0f )  // 7 - (0,1,1)
  };
/*
         y
         |
        1|
        /\
       / |\
      / r|l\
     / a | e\
    / e /0\ f\7
  2|\n /top\/t|
   | \/    /\ |
   |r/\bot/ r\|
  3|/i \5/ a /6
   /\ g | f /  \
  /  \ h|  /    \
 x    \t| /      z
       \|/
        4
*/
  nglVector wp[8];  // world points
  nglVector v1, v2; // vectors used for computing normals

  for(int i = 0; i < 8; i++)
     sceVu0ApplyMatrix(wp[i], SCE_MATRIX(m), lp[i]);

  // compute near plane normal
  nglVectSubtract(v1, wp[1], wp[0]);
  nglVectSubtract(v2, wp[3], wp[0]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_NEAR], v2, v1);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_NEAR]);
  frustum->Planes[NGLFRUSTUM_NEAR][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_NEAR], wp[0]);

  // compute far plane normal
  nglVectSubtract(v1, wp[4], wp[6]);
  nglVectSubtract(v2, wp[7], wp[6]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_FAR], v2, v1);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_FAR]);
  frustum->Planes[NGLFRUSTUM_FAR][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_FAR], wp[6]);

  // compute left plane normal
  nglVectSubtract(v1, wp[6], wp[0]);
  nglVectSubtract(v2, wp[1], wp[0]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_LEFT], v2, v1);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_LEFT]);
  frustum->Planes[NGLFRUSTUM_LEFT][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_LEFT], wp[0]);

  // compute right plane normal
  nglVectSubtract(v1, wp[2], wp[3]);
  nglVectSubtract(v2, wp[4], wp[3]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_RIGHT], v2, v1);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_RIGHT]);
  frustum->Planes[NGLFRUSTUM_RIGHT][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_RIGHT], wp[3]);

  // compute top plane normal
  nglVectSubtract(v1, wp[7], wp[1]);
  nglVectSubtract(v2, wp[2], wp[1]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_TOP], v2, v1);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_TOP]);
  frustum->Planes[NGLFRUSTUM_TOP][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_TOP], wp[1]);

  // compute bottom plane normal
  nglVectSubtract(v1, wp[3], wp[0]);
  nglVectSubtract(v2, wp[6], wp[0]);
  nglVectCross(frustum->Planes[NGLFRUSTUM_BOTTOM], v2, v1);
  nglVectNormalize(frustum->Planes[NGLFRUSTUM_BOTTOM]);
  frustum->Planes[NGLFRUSTUM_BOTTOM][3] = -nglVectDot(frustum->Planes[NGLFRUSTUM_BOTTOM], wp[0]);
}

// this function assumes that the x, y, and z componentes of plane are normalized
float nglDistanceToPlane(const nglVector& plane, const nglVector& point)
{
  //dot(unit_normal, pt) + d;
  float Dist = (plane[0]*point[0] + plane[1]*point[1] + plane[2]*point[2]) + plane[3];
  return Dist;
}

// takes a point and radius in world space
bool nglIsSphereVisible( nglFrustum* Frustum, const nglVector& Center, float Radius )
{
  // This is a good order to cull by, as it culls the majority of invisible
  // objects in the first couple tries.  Relies on short-circuit evaluation!
  Radius = -Radius;

  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_LEFT], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_RIGHT], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_NEAR], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_TOP], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_BOTTOM], Center) < Radius )
    return false;
  if ( nglDistanceToPlane(Frustum->Planes[NGLFRUSTUM_FAR], Center) < Radius )
    return false;

  return true;
}

void nglListAddTextureLightNode( u_int Type, void* NodeData, u_int LightCat )
{
  nglLightNode* NewNode = nglListNew( nglLightNode );
  if ( !NewNode )
    return;

  NewNode->Type = Type;
  NewNode->NodeData = NodeData;
  NewNode->LightCat = LightCat;

  for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
    if ( LightCat & ( 1 << ( NGL_LIGHTCAT_SHIFT + i ) ) )
    {
      NewNode->Next[i] = nglCurLightContext->TextureHead.Next[i];
      nglCurLightContext->TextureHead.Next[i] = NewNode;
    }
}

struct nglPointProjectorLightInfo
{
  // Position in world space.
  nglVector Pos;

  // Color of the light.
  nglVector Color;

  // Range at which falloff finishes.
  float Range;

  // Blending operation (usually NGLBM_ADDITIVE).
  u_int BlendMode;
  u_int BlendModeConstant;

  // Texture to project (usually nglPhongTex).
  nglTexture* Tex;
};

void nglListAddPointProjectorLight( u_int LightCat, const nglVector& Pos, const nglVector& Color, float Range, u_int BlendMode, u_int BlendModeConstant, nglTexture* Tex )
{
  // Cull vs view frustum planes - if the light is outside the view frustum we can ignore it.
  if ( nglGetClipResultViewOnly( (nglVector&)Pos, Range ) == -1 )
    return;

  return;

  nglPointProjectorLightInfo* Light = nglListNew( nglPointProjectorLightInfo );
  if ( !Light )
    return;

  Light->BlendMode = BlendMode;
  Light->BlendModeConstant = BlendModeConstant;
  if ( Tex )
    Light->Tex = Tex;
  else
    Light->Tex = nglPhongTex;

  sceVu0CopyVector( Light->Pos, (nglVector)Pos );
  Light->Pos[3] = 1.0f;
  sceVu0CopyVector( Light->Color, (nglVector)Color );
  Light->Range = Range;

  nglListAddTextureLightNode( NGLLIGHT_PROJECTED_POINT, Light, LightCat );

  if ( DEBUG_ENABLE( DrawLightSpheres ) )
  {
    u_int col = nglVectorToColor( Light->Color );
    nglDrawDebugSphere( Light->Pos, 0.25f, col, true );
    nglDrawDebugSphere( Light->Pos, Light->Range, col, false );
  }

  // No scene dump support yet.
}

struct nglDirProjectorLightInfo
{
  // Blending operation (usually NGLBM_ADDITIVE).
  u_int BlendMode;
  u_int BlendModeConstant;

  // Texture to project (usually nglPhongTex).
  nglTexture* Tex;

#ifdef PROJECT_SPIDERMAN
  float FadeScale;
  nglVector PosCullOffset;
#endif

  // For scaling in the U,V directions.  Z component is used as a far plane for the frustum.
  nglVector Scale;

  // Calculated parameters.
  nglVector Pos;
  nglVector Xaxis;
  nglVector Yaxis;
  nglVector Zaxis;

  // the matrix for going from world to uv space
  nglMatrix WorldToUV;

  // the matrix for going from uv to world space
  nglMatrix UVToWorld;

  // the frustum object for this light/projector
  nglFrustum Frustum;
};

#ifdef PROJECT_SPIDERMAN
void nglListAddDirProjectorLight( u_int LightCat, const nglMatrix& PO, const nglVector& Scale, float FadeScale, const nglVector& PosCullOffset, u_int BlendMode, u_int BlendModeConstant, nglTexture* Tex )
#else
void nglListAddDirProjectorLight( u_int LightCat, const nglMatrix& PO, const nglVector& Scale, u_int BlendMode, u_int BlendModeConstant, nglTexture* Tex )
#endif
{
  nglDirProjectorLightInfo* Light = nglListNew( nglDirProjectorLightInfo );
  if ( !Light )
    return;

  Light->BlendMode = BlendMode;
  Light->BlendModeConstant = BlendModeConstant;
  if ( Tex )
    Light->Tex = Tex;
  else
    Light->Tex = nglPhongTex;

#ifdef PROJECT_SPIDERMAN
  Light->FadeScale = FadeScale;

  Light->PosCullOffset[0] = PosCullOffset[0];
  Light->PosCullOffset[1] = PosCullOffset[1];
  Light->PosCullOffset[2] = PosCullOffset[2];
  Light->PosCullOffset[3] = PosCullOffset[3];
#endif

  Light->Scale[0] = Scale[0];
  Light->Scale[1] = Scale[1];
  Light->Scale[2] = Scale[2];
  Light->Scale[3] = 1.0f;

  Light->Pos[0] = PO[3][0];
  Light->Pos[1] = PO[3][1];
  Light->Pos[2] = PO[3][2];
  Light->Pos[3] = 0.0f;

  Light->Xaxis[0] = PO[0][0];
  Light->Xaxis[1] = PO[0][1];
  Light->Xaxis[2] = PO[0][2];
  Light->Xaxis[3] = 0.0f;

  Light->Yaxis[0] = PO[1][0];
  Light->Yaxis[1] = PO[1][1];
  Light->Yaxis[2] = PO[1][2];
  Light->Yaxis[3] = 0.0f;

  Light->Zaxis[0] = PO[2][0];
  Light->Zaxis[1] = PO[2][1];
  Light->Zaxis[2] = PO[2][2];
  Light->Zaxis[3] = 0.0f;

  nglMatrix PositionMtx
  (
    nglVector(   1.f,        0.f,      0.f,    0.f), // X
    nglVector(   0.f,        1.f,      0.f,    0.f), // Y
    nglVector(   0.f,        0.f,      1.f,    0.f), // Z
    nglVector(-Light->Pos[0], -Light->Pos[1], -Light->Pos[2], 1.f) // T
  );

  nglMatrix OrientationMtx
  (
    nglVector(Light->Xaxis[0], Light->Yaxis[0], Light->Zaxis[0], 0.f),
    nglVector(Light->Xaxis[1], Light->Yaxis[1], Light->Zaxis[1], 0.f),
    nglVector(Light->Xaxis[2], Light->Yaxis[2], Light->Zaxis[2], 0.f),
    nglVector(   0.f,              0.f,           0.f,         1.f)
  );

  nglMatrix ScaleMtx
  (
    nglVector(1.0f/Scale[0], 0.f,            0.f,            0.f),
    nglVector(0.f,           1.0f/Scale[1],  0.f,            0.f),
    nglVector(0.f,           0.f,            1.0f/Scale[2],  0.f),
    nglVector(0.f,           0.f,            0.f,            1.f)
  );

  nglMatrix UVTranslationMtx
  (
    nglVector(1.f,  0.f,  0.f, 0.f),
    nglVector(0.f,  1.f,  0.f, 0.f),
    nglVector(0.f,  0.f,  1.f, 0.f),
    nglVector(0.5f, 0.5f, 0.f, 1.f)
  );

  nglMatrix InversePositionMtx
  (
    nglVector(   1.f,      0.f,      0.f,   0.f), // X
    nglVector(   0.f,      1.f,      0.f,   0.f), // Y
    nglVector(   0.f,      0.f,      1.f,   0.f), // Z
    nglVector(Light->Pos[0], Light->Pos[1], Light->Pos[2], 1.f)  // T
  );

  nglMatrix InverseOrientationMtx
  (
    nglVector(Light->Xaxis[0], Light->Xaxis[1], Light->Xaxis[2], 0.f), // X
    nglVector(Light->Yaxis[0], Light->Yaxis[1], Light->Yaxis[2], 0.f), // Y
    nglVector(Light->Zaxis[0], Light->Zaxis[1], Light->Zaxis[2], 0.f), // Z
    nglVector(    0.f,             0.f,           0.f,       1.f)  // T
  );          //  X                 Y                Z


  nglMatrix InverseScaleMtx
  (
    nglVector(Scale.x, 0.f,       0.f,       0.f),
    nglVector(0.f,       Scale.y, 0.f,       0.f),
    nglVector(0.f,       0.f,      Scale.z,  0.f),
    nglVector(0.f,       0.f,      0.f,       1.f)
  );

  nglMatrix InverseUVTranslationMtx
  (
    nglVector( 1.f,  0.f,  0.f, 0.f),
    nglVector( 0.f,  1.f,  0.f, 0.f),
    nglVector( 0.f,  0.f,  1.f, 0.f),
    nglVector(-0.5f,-0.5f, 0.f, 1.f)
  );

  // compute this light's worl to UV space matrix
  sceVu0MulMatrix( SCE_MATRIX(Light->WorldToUV), SCE_MATRIX(OrientationMtx), SCE_MATRIX(PositionMtx) );
  sceVu0MulMatrix( SCE_MATRIX(Light->WorldToUV), SCE_MATRIX(ScaleMtx), SCE_MATRIX(Light->WorldToUV) );
  // << do perspective here if/when necessary >>
  sceVu0MulMatrix( SCE_MATRIX(Light->WorldToUV), SCE_MATRIX(UVTranslationMtx), SCE_MATRIX(Light->WorldToUV) );

  // compute the inverse matrix as well
  sceVu0MulMatrix( SCE_MATRIX(Light->UVToWorld), SCE_MATRIX(InverseScaleMtx), SCE_MATRIX(InverseUVTranslationMtx) );
  // << do inverse perspective here if/when necessary >>
  sceVu0MulMatrix( SCE_MATRIX(Light->UVToWorld), SCE_MATRIX(InverseOrientationMtx), SCE_MATRIX(Light->UVToWorld) );
  sceVu0MulMatrix( SCE_MATRIX(Light->UVToWorld), SCE_MATRIX(InversePositionMtx), SCE_MATRIX(Light->UVToWorld) );

  // build the frustum for this light
  nglBuildFrustum( &Light->Frustum, Light->UVToWorld );

  nglListAddTextureLightNode( NGLLIGHT_PROJECTED_DIRECTIONAL, Light, LightCat );

  // No debug rendering support yet.

  // No scene dump support yet.
}

// Spot lights are faked by dir lights at the moment.
void nglListAddSpotProjectorLight( u_int LightCat, const nglMatrix& PO, const nglVector& Scale, float FOV, u_int BlendMode, u_int BlendModeConstant, nglTexture* Tex )
{
#ifdef PROJECT_SPIDERMAN
  nglVector PosCullOffset = { 0.0f, 0.0f, 0.0f, 0.0f };
  nglListAddDirProjectorLight( LightCat, PO, Scale, 1.0f, PosCullOffset, BlendMode, BlendModeConstant, Tex );
#else
  nglListAddDirProjectorLight( LightCat, PO, Scale, BlendMode, BlendModeConstant, Tex );
#endif
}

/*---------------------------------------------------------------------------------------------------------
  @Texture streaming code.
---------------------------------------------------------------------------------------------------------*/
nglTexture* nglLastTexture = NULL;

float nglMipBias = 3.0f;

// Set the appropriate GS registers for a texture.  All of the fields are either filled out at load time or by
// texture streaming.
void nglVif1AddSetTexture( nglTexture* Tex, u_int MapFlags, float MipRatio )
{
  START_PROF_TIMER( proftimer_section_set_texture );

  if (!Tex) Tex = nglWhiteTex;

  if ( Tex->Type == NGLTEX_IFL || Tex->Type == NGLTEX_ATE )
    Tex = Tex->Frames[nglTextureAnimFrame % Tex->NFrames];

  if ( Tex->Type == NGLTEX_TGA )
  {
    nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );
    nglVif1AddGSReg( NGL_GS_TEX1_1, Tex->GsTex1 );
    nglVif1AddGSReg( NGL_GS_TEXA, Tex->GsTexA );
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
      nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );

      // then treat it as a 4 bit texture
      ( (sceGsTex0*)&Tex->GsTex0 )->PSM = SCE_GS_PSMT4;
      ( (sceGsTex0*)&Tex->GsTex0 )->CLD = 0;
      nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );
    }
    else
      nglVif1AddGSReg( NGL_GS_TEX0_1, Tex->GsTex0 );

#if !NGL_DISABLE_MIPMAPS
    if ( Tex->MipMapTextures > 1 && MipRatio && !DEBUG_ENABLE( DisableMipmaps ) )
    {
      nglVif1AddGSReg( NGL_GS_MIPTBP1_1, Tex->GsMipTBP1 );
      nglVif1AddGSReg( NGL_GS_MIPTBP2_1, Tex->GsMipTBP2 );

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
      nglVif1AddGSReg( NGL_GS_TEX1_1, Tex->GsTex1 );
    }
    else
#endif
		{
			u_int MinFilter = 1, MagFilter = 1;  // Bilinear by default
			if ( MapFlags & NGLMAP_POINT_FILTER )
				MinFilter = MagFilter = 0;
      nglVif1AddGSReg( NGL_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, MagFilter, MinFilter, 0, 0, 0 ) );
		}
    nglVif1AddGSReg( NGL_GS_TEXA, Tex->GsTexA );
  }

  // If this is a feedback style effect from the render target to itself, flush the texture cache before
  // rendering such that all previously rendered objects show up in the feedback result.
  // (I moved this from the top to here, so that nglVif1AddTextFixup won't be broken when it occurs - Paul).
  if ( Tex == nglCurScene->RenderTarget )
    nglVif1AddRawGSReg( SCE_GS_TEXFLUSH, 0 );

  STOP_PROF_TIMER( proftimer_section_set_texture );
}

u_int* nglLastIntAddr = NULL;

void nglVif1AddTextureStreamSetup( u_int*& Packet )
{
  nglNVif1IntEntries = 0;
  nglVif1IntCurTexEntry = NULL;

  // This keeps it from using textures from previous frame
  nglTexStreamAlloc.addr = NGL_VRAM_STREAM_START;
  nglTexStreamAlloc.cycle += 2;
  if (nglTexStreamAlloc.cycle >= 0xFFFF0000)
    nglFatal( "nglTexStreamAlloc.cycle got too close to wrapping (%d).\n", nglTexStreamAlloc.cycle );

  nglTexStreamPrevStartRef = nglTexStreamAlloc;
  nglTexStreamCurStartRef = nglTexStreamAlloc;

  // Set the interrupt bit for the first batch of textures.
  nglLastIntAddr = NULL;
  *(Packet++) = SCE_VIF1_SET_NOP( 1 );
  *(Packet++) = SCE_VIF1_SET_FLUSHA( 0 );
  nglLastIntAddr = Packet;
  *(Packet++) = SCE_VIF1_SET_NOP( 1 );

  nglVif1IntCreateTextureBlock();
}

void nglVif1AddTextureStreamEnd( u_int*& Packet )
{
  nglVif1IntCloseTextureBlock();

  // Add a dummy entry for the last texture block, if we ever got to the point of creating one.
  if ( nglLastIntAddr )
  {
    nglVif1IntEntry* Entry;
    Entry = &nglVif1IntArray[nglNVif1IntEntries++];
    if ( nglNVif1IntEntries >= NGL_INT_MAX_ENTRIES )
      return;

    Entry->Type = NGLINT_LOADTEXTURE;
    Entry->NTextures = 0;
    Entry->DataSize = 0;
  }
}

bool nglVif1AddTextureStreaming( u_int*& Packet, nglTexture** Textures, u_int NTextures )
{
  START_PROF_TIMER( proftimer_send_texturestream );

  static nglTexture* NewTextures[32+1];

  // Check for IFLs, remove locked textures, and determine if any actual work needs to be done.
  u_int i, j, k;
  for ( i = 0, j = 0, k = 0; i < NTextures; ++i )
  {
    nglTexture* Tex = Textures[i];
    if ( Tex->Type == NGLTEX_IFL || Tex->Type == NGLTEX_ATE )
      Tex = Tex->Frames[nglTextureAnimFrame % Tex->NFrames];
    if (!Tex->Flags.Locked)
    {
		  NewTextures[j++] = Tex;
      if ( Tex->TexStreamPos >= nglTexStreamCurStartRef ) k++;
    }
  }
  nglTexture *RenderTarget = nglCurScene->RenderTarget;
  if (!RenderTarget->Flags.Locked)
  {
    NewTextures[j++] = RenderTarget;
    if (RenderTarget->TexStreamPos >= nglTexStreamCurStartRef)
      k++;
  }
  if ( j == k ) // all textures are present in referencable memory.
    return true;
  NTextures = j;

  bool result, more_space;
  if (nglVif1IntCurTexEntry->NTextures && nglVif1IntTextureUploadNextAvail() < NGL_VRAM_STREAM_SIZE/3)
  {
	  result = nglVif1IntMaybeAddTextureUpload(Packet, NewTextures, NTextures, true, false /*don't allow adds*/);
	  more_space = true;	// Reattempt
  }
  else
  {
	  result = nglVif1IntMaybeAddTextureUpload(Packet, NewTextures, NTextures, true );
	  if (!result)
		  result = nglVif1IntMaybeAddTextureUpload(Packet, NewTextures, NTextures, false );
	  more_space = false;	// Only reattempt if more space
  }
  while (!result)
  {
    *(Packet++) = SCE_VIF1_SET_FLUSHA ( 0 );

    nglLastIntAddr = Packet;
    *(Packet++) = SCE_VIF1_SET_NOP( 1 );

	  more_space = more_space || (nglTexStreamCurStartRef > nglTexStreamPrevStartRef);

    nglVif1IntCloseTextureBlock();
    nglVif1IntCreateTextureBlock();

    if (more_space)
		  result = nglVif1IntMaybeAddTextureUpload(Packet, NewTextures, NTextures, false );
	  more_space = true; // Reattempt
	  if (nglTexStreamPrevStartRef==nglTexStreamAlloc) break;
  }

#if NGL_DEBUGGING
  if ( !result )
  {
    // Disable texture mapping on the node if its texture is too big for the streaming buffer.
    nglWarning( "NGL: Can't fit all the textures in the streaming buffer. Buffer size is %d.\n", NGL_VRAM_STREAM_SIZE * 256 );
    for ( i = 0; i < NTextures; i++ )
      nglPrintf( "NGL: Texture %d is %s, size is %d.\n", i, NewTextures[i]->FileName.c_str(), NewTextures[i]->GsSize * 256 );
  }
#endif

  STOP_PROF_TIMER( proftimer_send_texturestream );
  return result;
}

//----------------------------------------------------------------------------------------
//  @Texture loading/management code.
//----------------------------------------------------------------------------------------
nglTexture* nglLoadTextureA( const char* FileName )
{
  nglFixedString p( FileName );
  return nglLoadTexture( p );
}

bool nglLoadTextureATE( nglTexture* Tex, unsigned char *Buf, const nglFixedString &FileName );
bool nglLoadTextureIFL( nglTexture* Tex, unsigned char *Buf, u_int BufSize );
bool nglLoadTextureTGA( nglTexture* Tex, unsigned char *hdr );
bool nglLoadTextureTM2( nglTexture* Tex, unsigned char *hdr );
bool nglLoadTextureSplitTM2( nglTexture* Tex, unsigned char *hdr, unsigned char *img, unsigned char *pal );

nglTexture* nglLoadTextureInPlace( const nglFixedString& FileName, u_int Type, void* Data, u_int Size )
{
  bool Result = false;

  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( FileName ) ) )
  {
    Inst->RefCount++;
    return (nglTexture*)Inst->Value;
  }

  nglTexture* Tex = (nglTexture*)nglMemAlloc( sizeof(nglTexture) );
  memset( Tex, 0, sizeof(nglTexture) );

  switch ( Type )
  {
    case NGLTEX_TIM2:  Result = nglLoadTextureTM2( Tex, (u_char*)Data ); break;
    case NGLTEX_TGA :  Result = nglLoadTextureTGA( Tex, (u_char*)Data ); break;
    case NGLTEX_IFL :  Result = nglLoadTextureIFL( Tex, (u_char*)Data, Size ); break;
    case NGLTEX_ATE :  Result = nglLoadTextureATE( Tex, (u_char*)Data, FileName  ); break;
  }

  if ( !Result )
  {
    nglWarning( "nglLoadTextureInPlace: Unable to parse %s.\n", FileName.c_str() );
    nglMemFree( Tex );
    return 0;
  }

  Tex->FileName = FileName;
  Tex->Flags.Locked = 0;
  Tex->TexStreamPos.addr = 0;
  Tex->TexStreamPos.cycle = 0;
  Tex->Flags.LoadedInPlace = true;
  Tex->Hash = rand();

  Tex->FileBuf.Buf = (u_char*)Data;
  Tex->FileBuf.Size = Size;

  nglTextureBank.Insert( Tex->FileName, Tex );

  return Tex;
}

nglTexture* nglLoadTexture( const nglFixedString& FileName )
{
  bool Result = false;

  // Search for an existing copy.
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( FileName ) ) )
  {
    Inst->RefCount++;
    return (nglTexture*)Inst->Value;
  }

  nglTexture* Tex = (nglTexture*)nglMemAlloc( sizeof(nglTexture) );
  memset( Tex, 0, sizeof(nglTexture) );

  Tex->FileName = FileName;
  Tex->Flags.Locked = 0;
  Tex->TexStreamPos.addr = 0;
  Tex->TexStreamPos.cycle = 0;
  Tex->Flags.LoadedInPlace = false;
  Tex->Hash = rand();

  static char NameExt[1024];
  nglFileBuf File;

  strcpy( nglWork, nglTexturePath );
  strcat( nglWork, FileName.c_str() );

  strcpy( NameExt, nglWork );
  strcat( NameExt, ".tm2" );
  if ( nglReadFile( NameExt, &File, 128 ) )
  {
    Result = nglLoadTextureTM2( Tex, File.Buf );
    if ( !Result )
      nglReleaseFile( &File );
  }
  else
  {
    strcpy( NameExt, nglWork );
    strcat( NameExt, ".ifl" );
    if ( nglReadFile( NameExt, &File, 128 ) )
    {
      Result = nglLoadTextureIFL( Tex, File.Buf, File.Size );
      if ( !Result )
        nglReleaseFile( &File );
    }
    else
    {
      strcpy( NameExt, nglWork );
      strcat( NameExt, ".tga" );
      if ( nglReadFile( NameExt, &File, 128 ) )
      {
        Result = nglLoadTextureTGA( Tex, File.Buf );
        nglReleaseFile( &File );
      }
      else
      {
        strcpy( NameExt, nglWork );
        strcat( NameExt, ".ate" );
        if ( nglReadFile( NameExt, &File, 128 ) )
        {
          nglFixedString alltex=nglFixedString("");
          Result = nglLoadTextureATE( Tex, File.Buf, alltex ); //FileName );
          if ( !Result )
            nglReleaseFile( &File );
        }
      }
    }
  }

  Tex->FileBuf = File;

  if ( !Result )
  {
    nglWarning( "nglLoadTexture: Unable to open %s%s.\n", nglTexturePath, FileName.c_str() );
    nglMemFree( Tex );
    return 0;
  }

  nglTextureBank.Insert( Tex->FileName, Tex );

  return Tex;
}

// Add a reference to a texture in the instance bank.
void nglAddTextureRef( nglTexture* Tex )
{
  // Search for an existing copy.
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( Tex->FileName ) ) )
    Inst->RefCount++;
}

nglTexture* nglGetTextureA( const char* FileName )
{
  nglFixedString p( FileName );
  return nglGetTexture( p );
}

nglTexture* nglGetTexture( const nglFixedString& FileName )
{
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglTextureBank.Search( FileName ) ) )
    return (nglTexture*)Inst->Value;
  return 0;
}

void nglReleaseAllTextures()
{
  nglUnlockAllTexturesPS2();

  nglInstanceBank::Instance* Inst = nglTextureBank.Head->Forward[0];
  while ( Inst != nglTextureBank.NIL )
  {
    if ( ( (nglTexture*)Inst->Value )->Flags.System )
      Inst = Inst->Forward[0];
    else
    {
      Inst->RefCount = 1;
      nglReleaseTexture( (nglTexture*)Inst->Value );
      Inst = nglTextureBank.Head->Forward[0];
    }
  }
}

void nglReleaseTexture( nglTexture* Tex )
{
  // Avoid releasing NGL's statically allocated resources.
  if ( Tex->Flags.System )
    return;

  // Remove a reference from the instance bank, delete the texture only if the count hits 0.
  if ( nglTextureBank.Delete( Tex->FileName ) )
    return;

  nglDestroyTexture(Tex);
}

void nglDestroyTexture( nglTexture* Tex )
{
  // First wait for the DMA to complete, in case this texture's Baked DMA is in use.
  nglVif1SafeWait();

  if ( Tex->Flags.Locked )
    nglUnlockTexturePS2( Tex );

  // For animated textures, release all the frames.
  if ( Tex->Type == NGLTEX_IFL )
  {
    for ( u_int i = 0; i < Tex->NFrames; i++ )
      nglReleaseTexture( Tex->Frames[i] );
    nglMemFree( Tex->Frames );
  }

  if ( Tex->Type == NGLTEX_ATE )
  {
    for ( u_int i = 0; i < Tex->NFrames; i++ )
    {
      if ( Tex->Frames[i] )
      {
        nglReleaseTexture( Tex->Frames[i] );
		// Done from within nglReleaseTexture (dc 06/06/02)
//        nglMemFree( Tex->Frames[i] );
      }
    }
    nglMemFree( Tex->Frames );
		#if 0
    if ( Tex->Data )
    {
      nglFileBuf File;
      File.Buf = (u_char*)Tex->Data;
      nglReleaseFile( &File );
    }
		#endif
  }

  // For others, release the data if it wasn't loaded in place.
  if ( !Tex->Flags.LoadedInPlace )
  {
    if (( Tex->Type == NGLTEX_TIM2 ) || ( Tex->Type == NGLTEX_IFL ) || ( Tex->Type == NGLTEX_ATE ))
      nglReleaseFile( &Tex->FileBuf );
    else
    if ( Tex->Type == NGLTEX_TGA )
      nglMemFree( Tex->Data );
  }

  // Release baked texture DMA.
  if (Tex->DMAMem)
    nglMemFree( Tex->DMAMem );

  nglMemFree( Tex );
}

// This function is slow, since it reuploads *all locked textures*.
void nglRelockAllTexturesPS2()
{
  nglVif1SafeWait();
  nglLockedTextureSize = 0;
  nglInstanceBank::Instance* Inst = nglTextureBank.Head->Forward[0];
  while ( Inst != nglTextureBank.NIL )
  {
    nglTexture* Tex = (nglTexture*)Inst->Value;
    if ( !Tex->Flags.System && Tex->Flags.Locked )
    {
      Tex->Flags.Locked = 0;
      nglLockTexturePS2( Tex );
    }
    Inst = Inst->Forward[0];
  }
}

// This function is slow, since it reuploads *all locked textures*.
void nglUnlockTexturePS2( nglTexture* Tex )
{
  Tex->Flags.Locked = false;
  nglRelockAllTexturesPS2();
}

void nglUnlockAllTexturesPS2()
{
  nglLockedTextureSize = 0;
  nglInstanceBank::Instance* Inst = nglTextureBank.Head->Forward[0];
  while ( Inst != nglTextureBank.NIL )
  {
    nglTexture* Tex = (nglTexture*)Inst->Value;
    if ( !Tex->Flags.System )
      Tex->Flags.Locked = false;
    Inst = Inst->Forward[0];
  }
}

nglTexture* nglCreateTexture( u_int Format, u_int Width, u_int Height )
{
  nglTexture* Tex;
  if ( Format & NGLTF_TEMP )
    Tex = nglListNew( nglTexture );
  else
    Tex = (nglTexture*)nglMemAlloc( sizeof(nglTexture) );
  if ( !Tex )
    return NULL;

  memset( Tex, 0, sizeof(nglTexture) );

  Tex->Type = NGLTEX_TGA;

  u_int PixelSize = 0;
  if ( ( Format & 0xFF ) == NGLTF_32BIT )
  {
    Tex->Format = SCE_GS_PSMCT32;
    PixelSize = 4;
  }
  else
  if ( ( Format & 0xFF ) == NGLTF_16BIT )
  {
    Tex->Format = SCE_GS_PSMCT16;
    PixelSize = 2;
  }
  else
  {
    nglWarning( "NGL: Tried to create a texture without a valid format.\n" );
    return NULL;
  }

  Tex->Width = Width;
  Tex->Height = Height;

  if ( Format & NGLTF_VRAM_ONLY )
    Tex->Flags.VRAMOnly = 1;
  else
  if ( !( Format & NGLTF_TEMP ) )
  {
    Tex->Data = (u_int*)nglMemAlloc( Tex->Width * Tex->Height * PixelSize );
    if ( !Tex->Data )
    {
      nglMemFree( Tex );
      return NULL;
    }
  }

  if ( Format & NGLTF_RENDER_TARGET )
    Tex->Flags.RenderTarget = 1;

  // Calculate GS storage requirement.
  Tex->GsSize = ( ( Tex->Width * Tex->Height * PixelSize / 256 ) + 31 ) & ~31;
  Tex->SrcDataSize = Tex->Width * Tex->Height * PixelSize;

  nglCalcTextureGSRegs( Tex );

  return Tex;
}

//----------------------------------------------------------------------------------------
//  @IFL file code.
//----------------------------------------------------------------------------------------
bool nglInLoadTextureIFL = false;

bool nglLoadTextureIFL( nglTexture* Tex, unsigned char *Buf, u_int BufSize )
{
  // Temporary buffer for building the texture array.
  static char FileName[256];
  static nglTexture* Textures[512];
  int NTextures = 0;

  if ( nglInLoadTextureIFL )
  {
    nglWarning( "NGL: Recursive IFL detected.\n" );
    return false;
  }
  nglInLoadTextureIFL = true;

// handy parser macro
#define endofbuf( a ) ( (a) - (char*)Buf >= (int)BufSize )

  char* Pos = (char*)Buf;
  while ( !endofbuf( Pos ) )
  {
    // parse out the filename (no extension).
    char* Line = Pos;
    while ( !endofbuf( Line ) && !isspace( *Line ) ) Line++;
    u_int Count = min( Line - Pos, 255 );
    if ( !Count ) break;
    strncpy( FileName, Pos, Count );
    FileName[Count] = '\0';
    char* Ext = strchr( FileName, '.' );
    if ( Ext ) *Ext = '\0';

    if ( (u_int)NTextures >= elementsof(Textures) )
    {
      nglWarning( "Exceeded max number of textures in an IFL file (%d).\n", elementsof(Textures) );
      break;
    }

    // attempt to load the texture.
    Textures[NTextures] = nglLoadTextureA( FileName );
    if ( Textures[NTextures] )
      NTextures++;

    // skip any whitespace
    Pos = Line;
    while ( !endofbuf( Pos ) && isspace( *Pos ) ) Pos++;
  }

#undef endofbuf

  nglInLoadTextureIFL = false;

  if ( NTextures )
  {
    Tex->Type = NGLTEX_IFL;
    Tex->NFrames = NTextures;

    Tex->Frames = (nglTexture**)nglMemAlloc( sizeof(nglTexture*) * NTextures );
    memcpy( Tex->Frames, Textures, sizeof(nglTexture*) * NTextures );

    // Steal our 'width and height' from the first frame.
    Tex->Width = Tex->Frames[0]->Width;
    Tex->Height = Tex->Frames[0]->Height;
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------------------
//  @ATE file code.  ATE files are Kelly-Slater specific animated textures.
//----------------------------------------------------------------------------------------
#include "ngl_ate.h"

bool nglLoadTextureATE( nglTexture* Tex, unsigned char *Buf, const nglFixedString &FileName )
{
  atestring ps = atestring( FileName.c_str() );

  if ( !ATEHeaderValid((char *)Buf) )
    return false;

  u_int Temp = nglDisableTim2Conversion;
  nglDisableTim2Conversion = 1;

  Tex->Type = NGLTEX_ATE;
  Tex->NFrames = ATETextureCount( (char *)Buf, ps );
  if ( Tex->NFrames <= 0 )
  {
    nglDisableTim2Conversion = Temp;
    return false;
  }

  Tex->Frames = (nglTexture**)nglMemAlloc( sizeof(nglTexture*) * Tex->NFrames );
  for ( u_int i=0; i<Tex->NFrames; i++ )
  {
    nglFixedString nam( ATETextureName( (char *)Buf, ps, i ).c_str() );
    u_char *hed=(u_char *) ATETextureHeader( (char *)Buf,ps, i );
    u_char *img=(u_char *) ATETextureImage( (char *)Buf,ps, i );
    u_char *pal=(u_char *) ATETexturePalette( (char *)Buf,ps, i );
    Tex->Frames[i] = (nglTexture*) nglMemAlloc( sizeof(nglTexture));
    memset( Tex->Frames[i], 0, sizeof(nglTexture) );
    nglLoadTextureSplitTM2(Tex->Frames[i],hed,img,pal);
    Tex->Frames[i]->FileName = nam;
  	Tex->Frames[i]->Flags.LoadedInPlace=true;

	// Otherwise memory allocated for this texture's DMA won't get freed.  (dc 06/06/02)
	nglTextureBank.Insert( Tex->Frames[i]->FileName, Tex->Frames[i] );
  }

  Tex->Data=(unsigned int *)Buf;

  nglDisableTim2Conversion = Temp;
  return true;
}

//----------------------------------------------------------------------------------------
//  @TGA file code.
//----------------------------------------------------------------------------------------
typedef struct _TgaHeader
{
  u_char   IdLength;            /* Image ID Field Length      */
  u_char   CmapType;            /* Color Map Type             */
  u_char   ImageType;           /* Image Type                 */
                                /* ** Color Map Specification */
  u_char   CmapIndex[2];        /* First Entry Index          */
  u_char   CmapLength[2];       /* Color Map Length           */
  u_char   CmapEntrySize;       /* Color Map Entry Size       */
                                /* ** Image Specification     */
  u_short  X_Origin;            /* X-origin of Image          */
  u_short  Y_Origin;            /* Y-origin of Image          */
  u_short  ImageWidth;          /* Image Width                */
  u_short  ImageHeight;         /* Image Height               */
  u_char   PixelDepth;          /* Pixel Depth                */
  u_char   ImagDesc;            /* Image Descriptor           */
} TGAHEADER;

bool nglLoadTextureTGA( nglTexture* Tex, unsigned char *Buf )
{
  TGAHEADER *hdr = (TGAHEADER *)Buf;
  u_int i, j;

  Tex->Width = hdr->ImageWidth;
  Tex->Height = hdr->ImageHeight;

  if ( hdr->CmapType )
  {
    nglWarning( "Palettized TGAs not supported: %s\n", nglWork );
    return false;
  }

  if ( hdr->ImageType != 2 )
  {
    nglWarning( "Invalid TGA Image Type: %d (%s)\n", hdr->ImageType, nglWork );
    return false;
  }

  if ( hdr->PixelDepth == 16 )
  {
    Tex->Format = SCE_GS_PSMCT16;
    Tex->GsSize = Tex->Width * Tex->Height * 2;
    Tex->Data = (u_int*)nglMemAlloc( Tex->GsSize, 128 );

    u_char* SrcData = Buf + sizeof(TGAHEADER);
    u_char* DestData = (u_char*)Tex->Data;

    for ( i = 0; i < Tex->Height; i++ )
    {
      memcpy(
        &DestData[i * Tex->Width * 2],
        &SrcData[( Tex->Height - i - 1 ) * Tex->Width * 2],
        Tex->Width * 2 );
    }
  }
  else
  if ( hdr->PixelDepth == 24 )
  {
    Tex->Format = SCE_GS_PSMCT32;
    Tex->GsSize = Tex->Width * Tex->Height * 4;
    Tex->Data = (u_int*)nglMemAlloc( Tex->GsSize, 128 );

    u_char* SrcData = Buf + sizeof(TGAHEADER);
    u_char* DestData = (u_char*)Tex->Data;
    for ( j = 0; j < Tex->Height; j++ )
      for ( i = 0; i < Tex->Width; i++ )
      {
        int SrcOfs = ( i + ( ( Tex->Height - j - 1 ) * Tex->Width ) ) * 3;
        int DestOfs = ( i + ( j * Tex->Width ) ) * 4;
        DestData[DestOfs + 0] = SrcData[SrcOfs + 2];
        DestData[DestOfs + 1] = SrcData[SrcOfs + 1];
        DestData[DestOfs + 2] = SrcData[SrcOfs + 0];
        DestData[DestOfs + 3] = 0x80;
      }
  }
  else
  if ( hdr->PixelDepth == 32 )
  {
    Tex->Format = SCE_GS_PSMCT32;
    Tex->GsSize = Tex->Width * Tex->Height * 4;
    Tex->Data = (u_int*)nglMemAlloc( Tex->GsSize, 128 );

    u_char* SrcData = Buf + sizeof(TGAHEADER);
    u_char* DestData = (u_char*)Tex->Data;
    for ( j = 0; j < Tex->Height; j++ )
      for ( i = 0; i < Tex->Width; i++ )
      {
        int SrcOfs = ( i + ( ( Tex->Height - j - 1 ) * Tex->Width ) ) * 4;
        int DestOfs = ( i + ( j * Tex->Width ) ) * 4;
        DestData[DestOfs + 0] = SrcData[SrcOfs + 2];
        DestData[DestOfs + 1] = SrcData[SrcOfs + 1];
        DestData[DestOfs + 2] = SrcData[SrcOfs + 0];
        if ( SrcData[SrcOfs + 3] == 0xff )  // special case to make 255 become 128.
          DestData[DestOfs + 3] = 0x80;
        else
          DestData[DestOfs + 3] = SrcData[SrcOfs + 3] / 2;
      }
  }
  else
  if ( hdr->ImageType != 2 )
  {
    nglWarning( "Invalid TGA pixel depth.\n" );
    return false;
  }

  Tex->SrcDataSize = Tex->GsSize;

  // Convert DataSize to GS local memory units and align to an 8k page boundary.
  Tex->GsSize /= 256;
  while ( Tex->GsSize & 31 ) Tex->GsSize++;

  Tex->Type = NGLTEX_TGA;
  Tex->GsBaseTBP = 0;

  nglCalcTextureGSRegs( Tex );

  return true;
}

//----------------------------------------------------------------------------------------
//  @TIM2 code.
//----------------------------------------------------------------------------------------

inline int nglTim2CheckFileHeader( void* pTim2 )
{
  TIM2_FILEHEADER* pFileHdr = (TIM2_FILEHEADER*)pTim2;
  int i;

  // Check TIM2 signature.
  if ( pFileHdr->FileId[0]=='T' || pFileHdr->FileId[1]=='I' || pFileHdr->FileId[2]=='M' || pFileHdr->FileId[3]=='2' )
    i = 1;
  else if ( pFileHdr->FileId[0]=='C' || pFileHdr->FileId[1]=='L' || pFileHdr->FileId[2]=='T' || pFileHdr->FileId[3]=='2' )
    i = 2;
  else
  {
    nglWarning( "nglTim2CheckFileHeader: TIM2 is broken %02X,%02X,%02X,%02X\n",
      pFileHdr->FileId[0], pFileHdr->FileId[1], pFileHdr->FileId[2], pFileHdr->FileId[3] );
    return 0;
  }

  // Check TIM2 file format version, format ID.
  if( !( pFileHdr->FormatVersion==0x03 || ( pFileHdr->FormatVersion == 0x04 && ( pFileHdr->FormatId == 0x00 || pFileHdr->FormatId == 0x01 ) ) ) )
  {
    nglWarning( "nglTim2CheckFileHeaer: TIM2 is broken (2)\n" );
    return 0;
  }
  return i;
}

// get imgno-th picture header
// arguments:
// pTim2    pointer to the head of TIM2 data
//  imgno   picture header number to be retrieved
// return value:
//          pointer to the picture header
inline TIM2_PICTUREHEADER *nglTim2GetPictureHeader(void *pTim2, int imgno)
{
  TIM2_FILEHEADER *pFileHdr = (TIM2_FILEHEADER *)pTim2;
  TIM2_PICTUREHEADER *pPictHdr;
  int i;

  // check picture number
  if(imgno>=pFileHdr->Pictures) {
    printf("Tim2GetPictureHeader: Illegal image no.(%d)\n", imgno);
    return(NULL);
  }

  if(pFileHdr->FormatId==0x00) {
    // 16 byte alignment if format ID is 0x00
    pPictHdr = (TIM2_PICTUREHEADER *)((char *)pTim2 + sizeof(TIM2_FILEHEADER));
  } else {
    // 128 byte alignment if format ID is 0x01
    pPictHdr = (TIM2_PICTUREHEADER *)((char *)pTim2 + 0x80);
  }

  // skip to the assumed picture
  for(i=0; i<imgno; i++) {
    pPictHdr = (TIM2_PICTUREHEADER *)((char *)pPictHdr + pPictHdr->TotalSize);
  }
  return(pPictHdr);
}



static u_char CT32SizeTbl[4][8] = {
  {  1,  2,  5,  6, 17, 18, 21, 22 },
  {  3,  4,  7,  8, 19, 20, 23, 24 },
  {  9, 10, 13, 14, 25, 26, 29, 30 },
  { 11, 12, 15, 16, 27, 28, 31, 32 }
};

static u_char CT16SizeTbl[8][4] = {
  {  1,  3,  9, 11 },
  {  2,  4, 10, 12 },
  {  5,  7, 13, 15 },
  {  6,  8, 14, 16 },
  { 17, 19, 25, 27 },
  { 18, 20, 26, 28 },
  { 21, 23, 29, 31 },
  { 22, 24, 30, 32 }
};

#define MT8SizeTbl CT32SizeTbl
#define MT4SizeTbl CT16SizeTbl

int nglTim2CalcBufSize(int psm, int w, int h)
{
  int size = 0;
  int row;

  switch(psm)
  {
  case SCE_GS_PSMCT32:
  case SCE_GS_PSMCT24:
  case SCE_GS_PSMT8H:
  case SCE_GS_PSMT4HL:
  case SCE_GS_PSMT4HH:
    row = ((w+63)>>6)<<5;
    size = CT32SizeTbl[((h-1)>>3)&3][((w-1)>>3)&7] + (row-32) + ((h-1)>>5)*row;
    break;

  case SCE_GS_PSMCT16:
    row = ((w+63)>>6)<<5;
    size = CT16SizeTbl[((h-1)>>3)&7][((w-1)>>4)&3] + (row-32) + ((h-1)>>6)*row;
    break;

  case SCE_GS_PSMT8:
    row = ((w+127)>>7)<<5;
    size = MT8SizeTbl[((h-1)>>4)&3][((w-1)>>4)&7] + (row-32) + ((h-1)>>6)*row;
    break;

  case SCE_GS_PSMT4:
    row = ((w+127)>>7)<<5;
    size = MT4SizeTbl[((h-1)>>4)&7][((w-1)>>5)&3] + (row-32) + ((h-1)>>7)*row;
    break;
  }

  return size;
}

bool nglLoadTextureSplitTM2( nglTexture* Tex, unsigned char *Buf, unsigned char *Img, unsigned char *Pal )
{
  // Parse the picture header from the TM2 file and build the texture.
  // Get picture header and save the buffer pointer so we can free it when Unload time comes.
  TIM2_PICTUREHEADER* ph = (TIM2_PICTUREHEADER *) Buf; //nglTim2GetPictureHeader( Buf, 0 );
  Tex->Type = NGLTEX_TIM2;
  Tex->ph = ph;
  Tex->Data = (u_int*)Buf;
  Tex->Width = ph->ImageWidth;
  Tex->Height = ph->ImageHeight;
  Tex->SrcDataSize = ph->TotalSize;

#if defined (PROJECT_KELLYSLATER) && NGL_DEBUGGING
  if (Tex->ph->ImageType == TIM2_RGB32)
    nglPrintf ("WARNING: Loading a 32-bit texture: %s.tm2\n", nglWork);
#endif

  // Convert 4bit, 8bit and 16bit textures such that they can be uploaded as PSMCT32.
  // Involves swizzling the pixels within each mip level.
  if ( !nglDisableTim2Conversion )
  {
    nglTim2Convert converter;

    if ( TIM2_IDTEX4 == ph->ImageType )
      converter = nglTim2Convert4to32;
    else if ( TIM2_IDTEX8 == ph->ImageType )
      converter = nglTim2Convert8to32;
    else if ( TIM2_RGB16 == ph->ImageType )
      converter = nglTim2Convert16to32;
    else
      converter = NULL;

    if (NULL != converter)
    {
      u_char *input, *output;
      int width, height, size;

      size = nglTim2GetMipMapPictureSize(ph,0,&width,&height);
      if ( size < 1024 )  // increase size in case it's too small for the palette
        size = 1024;
      input = (u_char*)nglMemAlloc(size);

      //Convert image - all pis
      for (u_int i = 0; i < ph->MipMapTextures; i++)
      {
        size = nglTim2GetMipMapPictureSize(ph,i,&width,&height);
        output = (u_char*)nglTim2GetImage(Tex,i);
        memcpy(input,output,size);
        converter(width,height,input,output);
      }

      //Convert CLUT if it is in PSMCT16
      if ( TIM2_RGB16 == ph->ClutType)
      {
        output = (u_char*)nglTim2GetClut(Tex);
        memcpy(input,output,ph->ClutSize);
        nglTim2Convert16to32(16,ph->ClutColors/16,input,output);
      }
      nglMemFree(input);
    }
  }

  // Lookup the real pixel format.
  u_int psm = psmtbl[ph->ImageType - 1];
  Tex->GsImage[0].PSM = psm;

//  u_long128 *pImage = (u_long128 *)((char*)ph + ph->HeaderSize);
  Tex->GsImage[0].Data = Img;

  // Setup Mipmaps if they exist.
#if !NGL_DISABLE_MIPMAPS
  Tex->MipMapTextures = ph->MipMapTextures;
  NGL_ASSERT( Tex->MipMapTextures <= 1, "Mipmaps not supported in ATE format (dc 05/31/02)." );
#else
  Tex->MipMapTextures = 0;
#endif

  // Setup the palette if neccessary.
  Tex->ClutColors = ph->ClutColors;
  if ( Tex->ClutColors )
  {
    if ( ( ph->ClutType & 0x3F ) == TIM2_RGB16 )
      Tex->GsClut.PSM = SCE_GS_PSMCT16;
    else if ( ( ph->ClutType & 0x3F ) == TIM2_RGB24 )
      Tex->GsClut.PSM = SCE_GS_PSMCT24;
    else
      Tex->GsClut.PSM = SCE_GS_PSMCT32;

    // Calculate the address of the CLUT data.
    Tex->GsClut.Data = Pal;	//(u_long128*)( (char*)ph + ph->HeaderSize + ph->ImageSize );

    if ( ph->ImageType == TIM2_IDTEX4 && !( ph->ClutType & 0x40 ) )
      nglWarning( "NGL: 4bit texture with 32bit palette needs to be re-exported: %s.\n", Tex->FileName.c_str() );
  }

  // Pre-build GS registers so we can set them efficiently when we reference the texture later.
  nglCalcTextureGSRegs( Tex );

  // Pre-build DMA packets for uploading this texture to video memory at runtime.
  nglBakeTexture( Tex );

  return true;
}

bool nglLoadTextureTM2( nglTexture* Tex, unsigned char *Buf )
{
  if ( !nglTim2CheckFileHeader( Buf ) )
  {
    nglWarning( "Invalid TIM2 file: %s.\n", nglWork );
    return false;
  }

  // Parse the picture header from the TM2 file and build the texture.
  TIM2_PICTUREHEADER* ph = nglTim2GetPictureHeader( Buf, 0 );
  Tex->Type = NGLTEX_TIM2;
  Tex->ph = ph;
  Tex->Data = (u_int*)Buf;
  Tex->Width = ph->ImageWidth;
  Tex->Height = ph->ImageHeight;
  Tex->SrcDataSize = ph->TotalSize;

#if defined (PROJECT_KELLYSLATER) && NGL_DEBUGGING
  if (Tex->ph->ImageType == TIM2_RGB32)
    nglPrintf ("WARNING: Loading a 32-bit texture: %s.tm2\n", nglWork);
#endif

  // Convert 4bit, 8bit and 16bit textures such that they can be uploaded as PSMCT32.
  // Involves swizzling the pixels within each mip level.
  if ( !nglDisableTim2Conversion )
  {
    nglTim2Convert converter;

    if ( TIM2_IDTEX4 == ph->ImageType )
      converter = nglTim2Convert4to32;
    else if ( TIM2_IDTEX8 == ph->ImageType )
      converter = nglTim2Convert8to32;
    else if ( TIM2_RGB16 == ph->ImageType )
      converter = nglTim2Convert16to32;
    else
      converter = NULL;

    if (NULL != converter)
    {
      u_char *input, *output;
      int width, height, size;

      size = nglTim2GetMipMapPictureSize(ph,0,&width,&height);
      if ( size < 1024 )  // increase size in case it's too small for the palette
        size = 1024;
      input = (u_char*)nglMemAlloc(size);

      //Convert image - all pis
      for (u_int i = 0; i < ph->MipMapTextures; i++)
      {
        size = nglTim2GetMipMapPictureSize(ph,i,&width,&height);
        output = (u_char*)nglTim2GetImage(Tex,i);
        memcpy(input,output,size);
        converter(width,height,input,output);
      }

      //Convert CLUT if it is in PSMCT16
      if ( TIM2_RGB16 == ph->ClutType)
      {
        output = (u_char*)nglTim2GetClut(Tex);
        memcpy(input,output,ph->ClutSize);
        nglTim2Convert16to32(16,ph->ClutColors/16,input,output);
      }
      nglMemFree(input);
    }
  }

  // Lookup the real pixel format.
  u_int psm = psmtbl[ph->ImageType - 1];
  Tex->GsImage[0].PSM = psm;

  u_long128 *pImage = (u_long128 *)((char*)ph + ph->HeaderSize);
  Tex->GsImage[0].Data = pImage;

  // Setup Mipmaps if they exist.
#if !NGL_DISABLE_MIPMAPS
  Tex->MipMapTextures = ph->MipMapTextures;
  NGL_ASSERT( Tex->MipMapTextures <= 7, "Too many mipmaps in texture (7 max including base)." );
  if ( Tex->MipMapTextures > 1 && !DEBUG_ENABLE( DisableMipmaps ) )
  {
    // MIPMAP header is placed just after the picture header.
    TIM2_MIPMAPHEADER* pm;
    pm = (TIM2_MIPMAPHEADER*)( ph + 1 );

    Tex->GsMipTBP1 = 0;
    Tex->GsMipTBP2 = 0;

    int w = Tex->Width;
    int h = Tex->Height;

    // Transfer image data of each Mipmap level.
    for ( u_int i = 1; i < Tex->MipMapTextures; i++ )
    {
      w>>=1;
      h>>=1;
      // Bilinear mipmaps should not go below 8x8 (see 3.4.11 in GS User's Manual)
      if (w<8 || h<8)
      {
        Tex->MipMapTextures = i;
        break;
      }
      // Advance pImage to the next mipmap.
      pImage = (u_long128*)( (char*)pImage + pm->MMImageSize[i - 1] );
      Tex->GsImage[i].PSM = psm;
	    Tex->GsImage[i].Data = pImage;
    }
  }
  else 
    Tex->MipMapTextures = 0;
#else
  Tex->MipMapTextures = 0;
#endif

  // Setup the palette if neccessary.
  Tex->ClutColors = ph->ClutColors;
  if ( Tex->ClutColors )
  {
    if ( ( ph->ClutType & 0x3F ) == TIM2_RGB16 )
      Tex->GsClut.PSM = SCE_GS_PSMCT16;
    else if ( ( ph->ClutType & 0x3F ) == TIM2_RGB24 )
      Tex->GsClut.PSM = SCE_GS_PSMCT24;
    else
      Tex->GsClut.PSM = SCE_GS_PSMCT32;

    // Calculate the address of the CLUT data.
    Tex->GsClut.Data = (u_long128*)( (char*)ph + ph->HeaderSize + ph->ImageSize );

    if ( ph->ImageType == TIM2_IDTEX4 && !( ph->ClutType & 0x40 ) )
      nglWarning( "NGL: 4bit texture with 32bit palette needs to be re-exported: %s.\n", Tex->FileName.c_str() );
  }

  // Pre-build GS registers so we can set them efficiently when we reference the texture later.
  nglCalcTextureGSRegs( Tex );

  // Pre-build DMA packets for uploading this texture to video memory at runtime.
  nglBakeTexture( Tex );

  return true;
}

// Transfers data to a specific location (similar to the SCE function).
void nglGifPkRefLoadImage( sceGifPacket *pPacket, u_short bp, u_char psm, u_short bw, u_long128 *image, u_int size, u_int w, u_int h )
{
  // Note that the usual DMA functions aren't used as they rely on a global (and a tag is probably in progress when
  // this function is called).
  u_long* Packet = (u_long*)pPacket->pCurrent;

  *(Packet++) = SCE_DMA_SET_TAG( 5, 0, SCE_DMA_ID_CNT, 0, 0, 0 );
  *(Packet++) = 0;

  *(Packet++) = SCE_GIF_SET_TAG( 3, 0, 0, 0, 0, 1 );
  *(Packet++) = 0xE;

  *(Packet++) = SCE_GS_SET_BITBLTBUF( 0, 0, 0, bp, bw, psm );
  *(Packet++) = SCE_GS_BITBLTBUF;

  *(Packet++) = SCE_GS_SET_TRXREG( w, h );
  *(Packet++) = SCE_GS_TRXREG;

  *(Packet++) = SCE_GS_SET_TRXDIR( 0 );
  *(Packet++) = SCE_GS_TRXDIR;

  *(Packet++) = SCE_GIF_SET_TAG( size, 0, 0, 0, SCE_GIF_IMAGE, 0 );
  *(Packet++) = 0;

  *(Packet++) = SCE_DMA_SET_TAG( size, 0, SCE_DMA_ID_REF, 0, image, 0 );
  *(Packet++) = 0;

  pPacket->pCurrent = (u_int*)Packet;
}

void nglBakeTextureChunk( u_long*& Packet, int psm, int tbw, int w, int h, u_long128 *image )
{
  int line_size;
  switch ( psm )
  {
    case SCE_GS_PSMZ32:
    case SCE_GS_PSMCT32:
      line_size = w * 4;
      break;

    case SCE_GS_PSMZ24:
    case SCE_GS_PSMCT24:
      line_size = w * 3;
      break;

    case SCE_GS_PSMZ16:
    case SCE_GS_PSMZ16S:
    case SCE_GS_PSMCT16:
    case SCE_GS_PSMCT16S:
      line_size = w * 2;

      if ( !nglDisableTim2Conversion )
      {
        psm = SCE_GS_PSMCT32;
        w >>= 1;
        tbw = nglTim2CalcBufWidth(psm,w);
      }
      break;

    case SCE_GS_PSMT8H:
    case SCE_GS_PSMT8:
      line_size = w;

      if ( !nglDisableTim2Conversion )
      {
        psm = SCE_GS_PSMCT32;
        w >>= 1;
        tbw = nglTim2CalcBufWidth(psm,w);
      }
      break;

    case SCE_GS_PSMT4HL:
    case SCE_GS_PSMT4HH:
    case SCE_GS_PSMT4:
      line_size = w / 2;  // this is okay since 4bit textures must have >2 even widths.

      if ( !nglDisableTim2Conversion )
      {
        psm = SCE_GS_PSMCT32;
        w >>= 1;
        tbw = nglTim2CalcBufWidth(psm,w);
      }
      break;

    default:
      return;
  }

  // To avoid exceeding the max DMA transfer limit of 512KB, split the transfer into sets of 'lines'.
  int lines = 32764 * 16 / line_size;
  for( int y = 0; y < h; y += lines )
  {
    u_long128* p = (u_long128*)( (char*)image + line_size * y );
    if( y + lines > h )
      lines = h - y;

    int qwc = ( lines * line_size + 15 ) / 16;

    // Note that the usual DMA functions aren't used as they rely on a global (and a tag is probably in progress when
    // this function is called).
    *(Packet++) = SCE_DMA_SET_TAG( 5, 0, SCE_DMA_ID_CNT, 0, 0, 0 );
    *(Packet++) = 0;

    *(Packet++) = SCE_GIF_SET_TAG( 3, 0, 0, 0, 0, 1 );
    *(Packet++) = 0xE;

    *(Packet++) = SCE_GS_SET_TRXREG( w, lines );
    *(Packet++) = SCE_GS_TRXREG;

    *(Packet++) = SCE_GS_SET_TRXPOS( 0, 0, 0, y, 0 );
    *(Packet++) = SCE_GS_TRXPOS;

    *(Packet++) = SCE_GS_SET_TRXDIR( 0 );
    *(Packet++) = SCE_GS_TRXDIR;

    *(Packet++) = SCE_GIF_SET_TAG( qwc, 0, 0, 0, SCE_GIF_IMAGE, 0 );
    *(Packet++) = 0;

    *(Packet++) = SCE_DMA_SET_TAG( qwc, 0, SCE_DMA_ID_REF, 0, p, 0 );
    *(Packet++) = 0;
  }

  *(Packet++) = SCE_DMA_SET_TAG( 0, 0, SCE_DMA_ID_RET, 0, 0, 0 );
  *(Packet++) = 0;
}

void nglBakeTexture( nglTexture* Tex )
{
  u_long128 *pImage;

  u_long* Packet = (u_long*)nglGifPacketWork;
  u_int Offset = 0;

  u_int psm = Tex->GsImage[0].PSM;
  int width = Tex->Width;
  int height = Tex->Height;

  Tex->MipMapFirstLod = 0;
  Tex->MipMapInitCount = 0;

  // Texture load image is ordered as follows:
  //  Clut
  //  Level n     (n = Tex->MipMapTextures-1)
  //  Level n-1
  //  ...
  //  Level 0      (full sized image)
  // This allows the allocation and loading of larger mip levels to be skipped
  // by stopping at a level before level 0.

  // Load the palette if neccessary (comes after the texture data for page cache speed).
  if ( Tex->ClutColors )
  {
    Tex->GsClut.TBW = 1;
    Tex->GsClut.Offset = Offset;
    Tex->GsClut.DMA = (u_int*)Packet;

    pImage = (u_long128 *)Tex->GsClut.Data;
    if (pImage)
    {
      nglBakeTextureChunk( Packet, Tex->GsClut.PSM, 1, 16, Tex->ClutColors / 16, pImage );
      Offset += ( ( 4 * Tex->ClutColors ) + 255 ) / 256;
    }
  }

  // Upload Mipmaps if they exist.
#if !NGL_DISABLE_MIPMAPS
  if ( Tex->MipMapTextures > 1 && !DEBUG_ENABLE( DisableMipmaps ) )
  {
    // Transfer image data of each Mipmap level.
    for ( u_int i=Tex->MipMapTextures-1; i>=1; --i )
    {
      // Texture size of the higher level will be half of the origianl
      int w = width >> i;
      int h = height >> i;

      psm = Tex->GsImage[i].PSM;

      Tex->GsImage[i].TBW = nglTim2CalcBufWidth( psm, w );
      Tex->GsImage[i].Offset = Offset;
      Tex->GsImage[i].DMA = (u_int*)Packet;

      pImage = (u_long128*)Tex->GsImage[i].Data;
      if (pImage)
      {
        nglBakeTextureChunk( Packet, psm, Tex->GsImage[i].TBW, w, h, pImage );
        Offset += nglTim2CalcBufSize( psm, w, h );
      }
      Tex->GsImage[i].Size = Offset;
    }
  }
  else
#endif
    Tex->MipMapTextures = 0;

  // Upload level 0 texture if it exists
  pImage = (u_long128*)Tex->GsImage[0].Data;
  if (pImage)
  {
    // Page align within upload in case used as render target
    // Note that the entire upload will also be page aligned in that case.
    Offset = (Offset+31)&~31;
    // NOTE: We should really have a flag to indicate that texture
    //  is legal as a render target, and only perform the above alignment
    //  if it is set.  SetRenderTarget would then reject textures which
    //  hadn't been thus set.  SetRenderTarget could still set the current
    //  render target bit in the texture to indicate whether it is actually
    //  being used as a render target (but this is of questionable utility,
    //  since hopefully the render target legal bit would only be set for
    //  textures which actually were going to be used as render targets).
  }
  Tex->GsImage[0].TBW = nglTim2CalcBufWidth( psm, width );
  Tex->GsImage[0].Offset = Offset;
  Tex->GsImage[0].DMA = (u_int*)Packet;

  if (pImage)
  {
    nglBakeTextureChunk( Packet, psm, Tex->GsImage[0].TBW, width, height, pImage );
    Offset += nglTim2CalcBufSize( psm, width, height );
  }

  Tex->GsImage[0].Size = Offset;

  Tex->GsSize = Tex->GsImage[0].Size;

  // Allocate memory for DMA and copy and relocate it there from working area

  u_int Size = (u_int)Packet - (u_int)nglGifPacketWork;
  u_char* BakedDMA = (u_char*)nglMemAlloc( Size );
  memcpy( BakedDMA, nglGifPacketWork, Size );

  nglPerfInfo.BakedTextureDMASize += Size;

  Tex->DMAMem = (u_int *)BakedDMA;

#define PTR_OFFSET( Ptr ) { if ( Ptr ) Ptr = (u_int*)( (u_int)Ptr + ( (u_int)BakedDMA - (u_int)nglGifPacketWork ) ); }
  PTR_OFFSET( Tex->GsImage[0].DMA );
  if ( Tex->ClutColors )
    PTR_OFFSET( Tex->GsClut.DMA );
  for ( u_int i = 1; i < Tex->MipMapTextures; i++ )
    PTR_OFFSET( Tex->GsImage[i].DMA );
#undef PTR_OFFSET
}

inline void nglUploadTextureChunk( u_long*& Packet, nglTexGsChunk* Chunk, u_int BaseTBP )
{
  // Note that the usual DMA functions aren't used as they rely on a global (and a tag is probably in progress when
  // this function is called).
  *(Packet++) = SCE_DMA_SET_TAG( 2, 0, SCE_DMA_ID_CNT, 0, 0, 0 );
  *(Packet++) = 0;

  *(Packet++) = SCE_GIF_SET_TAG( 1, 0, 0, 0, 0, 1 );
  *(Packet++) = 0xE;

  *(Packet++) = SCE_GS_SET_BITBLTBUF( 0, 0, 0, BaseTBP + Chunk->Offset, Chunk->TBW, Chunk->PSM );
  *(Packet++) = SCE_GS_BITBLTBUF;

  *(Packet++) = SCE_DMA_SET_TAG( 0, 0, SCE_DMA_ID_CALL, 0, Chunk->DMA, 0 );
  *(Packet++) = 0;
}

void nglUploadTexture( u_long*& Packet, nglTexture* Tex, u_int tbp )
{
  u_int VRAMOnly = Tex->Flags.VRAMOnly;

  Tex->GsBaseTBP = tbp;

  if ( Tex->ClutColors )
  {
    // Upload palette if present, even if VRAMOnly is set.
		nglUploadTextureChunk( Packet, &Tex->GsClut, tbp );
	  ( (sceGsTex0*)&Tex->GsTex0 )->CBP = tbp + Tex->GsClut.Offset;
  }

  // NOTE:  Currently, VRAMOnly for TM2 can allocate space for a texture image
  //  but doesn't then set its address.  This is correct if the texture
  //  is designed to upload a palette but reference the frame buffer or zbuffer,
  //  but is incorrect if it is also allocating space for a render target image.
  //  Additional flags are needed to signal this case (and changes will also
  //  be needed in nglBakeTexture to reserve space in gsImage[].Offset/Size
  //  without generating dma packet to upload texture).
  //  If a palette is not needed, but a VRAMOnly streamed texture is needed,
  //  this can currently be done with a TGA texture, which *does* set its address.
  //  (Obviously all this needs to be cleaned up with a better interface.)
  if (!VRAMOnly)
  {
    u_int FirstLod = Tex->MipMapFirstLod;
    nglTexGsChunk* Chunk = &Tex->GsImage[FirstLod];

	  nglUploadTextureChunk( Packet, Chunk, tbp );
	  ( (sceGsTex0*)&Tex->GsTex0 )->TBP0 = tbp + Chunk->Offset;
    ( (sceGsTex0*)&Tex->GsTex0 )->TBW = Chunk->TBW;

    ( (sceGsTex0*)&Tex->GsTex0 )->TW = Tex->TW - FirstLod;
    ( (sceGsTex0*)&Tex->GsTex0 )->TH = Tex->TH - FirstLod;

	  ( (sceGsTex1*)&Tex->GsTex1 )->MMIN = nglMinFilter;
    ( (sceGsTex1*)&Tex->GsTex1 )->MXL = Tex->MipMapTextures-1 - FirstLod;

    Tex->GsMipTBP1 = 0;
    Tex->GsMipTBP2 = 0;
    u_int lim = Tex->MipMapTextures - FirstLod;
    for ( u_int i = 1; i < lim; i++ )
    {
      ++Chunk;

      if( i < 4 )
      {
        // Level 1, 2 and 3 go in GsMiptbp1.
        Tex->GsMipTBP1 |= ((u_long)( tbp + Chunk->Offset ))<<((i-1)*20);
        Tex->GsMipTBP1 |= ((u_long)Chunk->TBW)<<((i-1)*20 + 14);
      }
      else
      {
        // Level 4, 5 and 6 go in GsMiptbp2.
        Tex->GsMipTBP2 |= ((u_long)( tbp + Chunk->Offset ))<<((i-4)*20);
        Tex->GsMipTBP2 |= ((u_long)Chunk->TBW)<<((i-4)*20 + 14);
      }

		  nglUploadTextureChunk( Packet, Chunk, tbp );
    }
  }
}

void nglSetLockBufferSizePS2( u_int Bytes )
{
  nglLockBufferSize = Bytes / 256;
}

// Forces a texture to stay VRAM between frames.
void nglLockTexturePS2( nglTexture* Tex )
{
  if ( Tex == &nglFrontBufferTex || Tex == &nglBackBufferTex )
    return;

  // Attempt to lock all frames of the IFL.
  if ( Tex->Type == NGLTEX_IFL || Tex->Type == NGLTEX_ATE )
  {
    for ( u_int i = 0; i < Tex->NFrames; i++ )
      nglLockTexturePS2( Tex->Frames[i] );
    return;
  }

  if ( Tex->Flags.Locked )
    return;

  if ( Tex->Flags.VRAMOnly )
  {
    // Increment and align to 2k boundary.
    nglLockedTextureSize += Tex->GsSize;
	  nglLockedTextureSize = (nglLockedTextureSize+31)&~31;
    Tex->Flags.Locked = 1;
    return;
  }

  // If the texture would break the region of video memory allocated for locked textures, stop.
  // The code should handle NULL Addr pointers and just turn off texture mapping.
  if ( NGL_VRAM_LOCKED_START + nglLockedTextureSize + Tex->GsSize > NGL_VRAM_LOCKED_END )
  {
    nglWarning( "Failed to lock texture: %s(%d).  %d of %d bytes used.\n",
      Tex->FileName.c_str(), Tex->GsSize * 256,
      nglLockedTextureSize * 256,
      ( NGL_VRAM_LOCKED_END - NGL_VRAM_LOCKED_START ) * 256 );
    Tex->GsBaseTBP = 0;
    return;
  }
  else
  {
    nglPrintf( "Locked texture: %s(%d).  %d of %d bytes used.\n",
      Tex->FileName.c_str(), Tex->GsSize * 256,
      ( nglLockedTextureSize + Tex->GsSize ) * 256,
      ( NGL_VRAM_LOCKED_END - NGL_VRAM_LOCKED_START ) * 256 );
  }

  if ( Tex->Type == NGLTEX_TGA )
  {
    // Load the image to VRAM.
    Tex->GsBaseTBP = NGL_VRAM_LOCKED_START + nglLockedTextureSize;
    ( (sceGsTex0*)&Tex->GsTex0 )->TBP0 = Tex->GsBaseTBP;

    u_int TexDataQWC = Tex->SrcDataSize / 16;

    sceGifPkInit( &nglGifPkt, (u_long128*)( (u_int)nglGifPacketWork ) );
    sceGifPkReset( &nglGifPkt );

    // Add a LoadImage for uploading the texture data.
    nglGifPkRefLoadImage( &nglGifPkt,
      Tex->GsBaseTBP, Tex->Format, ( Tex->Width + 63 ) / 64,
      (u_long128*)Tex->Data, TexDataQWC,
      Tex->Width, Tex->Height );

    // End DMA transfer
    sceGifPkEnd( &nglGifPkt, 0, 0, 0 );

    // GIF tag for texture settings
    sceGifPkAddGsData( &nglGifPkt, SCE_GIF_SET_TAG( 1, 1, NULL, NULL, SCE_GIF_PACKED, 1 ) );
    sceGifPkAddGsData( &nglGifPkt, 0xEL );

    // Flush the texture cache
    sceGifPkAddGsData( &nglGifPkt, 0 );
    sceGifPkAddGsData( &nglGifPkt, SCE_GS_TEXFLUSH );

    // Get packet size in quad words
    sceGifPkTerminate( &nglGifPkt );

	FlushCache(0);

    // Start a path 3 chain mode transfer
//    *D2_QWC = 0;
//    *D2_TADR = (u_int)nglGifPkt.pBase & NGL_DMA_MEM;
//    *D2_CHCR = 0 | (1<<2) | (0<<4) | (0<<6) | (0<<7) | (1<<8);
	_nglDmaStartGifSourceChain( (u_int)nglGifPkt.pBase & NGL_DMA_MEM, 0);

    // Wait for completion.
    while((*D2_CHCR & 0x0100) || (*GIF_STAT & 0x0c00));
  }
	else if ( Tex->Type == NGLTEX_TIM2 )
	{
	    sceGifPkInit( &nglGifPkt, (u_long128*)( (u_int)nglGifPacketWork ) );
    	sceGifPkReset( &nglGifPkt );
		// Add DMAs for the palette, main texture and each Mipmap level.
		// (Only uploads if Tex->VRAMOnly is not set)
		u_long* GifPacket = (u_long*)nglGifPkt.pCurrent;
		nglUploadTexture( GifPacket, Tex, NGL_VRAM_LOCKED_START + nglLockedTextureSize );
		nglGifPkt.pCurrent = (u_int*)GifPacket;

    	// End DMA transfer
    	sceGifPkEnd( &nglGifPkt, 0, 0, 0 );

    	// GIF tag for texture settings
    	sceGifPkAddGsData( &nglGifPkt, SCE_GIF_SET_TAG( 1, 1, NULL, NULL, SCE_GIF_PACKED, 1 ) );
    	sceGifPkAddGsData( &nglGifPkt, 0xEL );

    	// Flush the texture cache
    	sceGifPkAddGsData( &nglGifPkt, 0 );
    	sceGifPkAddGsData( &nglGifPkt, SCE_GS_TEXFLUSH );

    	// Get packet size in quad words
    	sceGifPkTerminate( &nglGifPkt );

		FlushCache(0);

    	// Start a path 3 chain mode transfer
		_nglDmaStartGifSourceChain( (u_int)nglGifPkt.pBase & NGL_DMA_MEM, 0);

    	// Wait for completion.
    	while((*D2_CHCR & 0x0100) || (*GIF_STAT & 0x0c00));
	}
  else
    nglFatal( "Unsupported texture type for locking.\n" );

  // Increment and align to 2k boundary.
  nglLockedTextureSize += Tex->GsSize;
  nglLockedTextureSize = (nglLockedTextureSize+31)&~31;

  Tex->Flags.Locked = 1;
}

//----------------------------------------------------------------------------------------
//  @Interrupt handler/more texture streaming code.
//----------------------------------------------------------------------------------------

u_int nglVif1IntTextureUploadNextAvail(void)
{
	return  (nglTexStreamCurStartRef.cycle+1 - nglTexStreamAlloc.cycle) * NGL_VRAM_STREAM_SIZE +
		    (nglTexStreamCurStartRef.addr - nglTexStreamAlloc.addr);
}

bool nglVif1IntMaybeAddTextureUpload(u_int*& Packet, nglTexture **Textures, u_int NTextures, bool full, bool allow_adds)
{
	START_PROF_TIMER( proftimer_texstream_addtexture );

	nglTexStreamPos Alloc;
	nglTexStreamPos StartRef = nglTexStreamCurStartRef;
	nglTexStreamPos Refable;
	if (full)
	{
		Refable = nglTexStreamAlloc;
		--Refable.cycle;
	}
	else if (StartRef < nglTexStreamPrevStartRef)
		Refable = StartRef;
	else
		Refable = nglTexStreamPrevStartRef;

	u_int i;

	Alloc = nglTexStreamAlloc;
	--Alloc.cycle; // First have alloc represent start of legal refs
	for ( i = 0; i < NTextures; ++i )
	{
		nglTexture *Tex = Textures[i];
		if (Tex->TexStreamPos >= Refable)
		{
			if (Tex->TexStreamPos < StartRef)
				StartRef = Tex->TexStreamPos;
		}
		else
		{
			if (!allow_adds)
			{
				STOP_PROF_TIMER( proftimer_texstream_addtexture );
				return false;
			}
			if (Tex->Flags.RenderTarget)
				//Force to 2K boundary
				Alloc.align2k(Tex->GsSize);
			else
				Alloc.align(Tex->GsSize);
			Alloc.alloc(Tex->GsSize);
			if (Alloc > StartRef || Alloc > nglTexStreamPrevStartRef)
			{
				STOP_PROF_TIMER( proftimer_texstream_addtexture );
				return false;
			}
			if (Alloc > Refable)
				Refable = Alloc;
		}
	}

	Alloc = nglTexStreamAlloc;
	for ( i = 0; i < NTextures; ++i )
	{
		nglTexture *Tex = Textures[i];
		if (Tex->TexStreamPos < Refable)
		{
			if (Tex->Flags.RenderTarget)
				//Force to 2K boundary
				Alloc.align2k(Tex->GsSize);
			else
				Alloc.align(Tex->GsSize);

			Tex->TexStreamPos = Alloc;

			if ( DEBUG_ENABLE( DumpFrameLog ) & 1 )
        if (Tex->MipMapFirstLod)
				  nglPrintf( "LOG: Uploading %dx%d %s (lod>=%d) to %d; size: %d (%d bytes) (lod0=%d bytes).\n",
                    Tex->Width, Tex->Height, Tex->FileName.c_str(), Tex->MipMapFirstLod, Alloc.addr, Tex->GsSize,
                    /*Tex->SrcDataSize*/ Tex->GsSize*256, Tex->GsImage[0].Size*256);
        else
				  nglPrintf( "LOG: Uploading %dx%d %s to %d; size: %d (%d bytes).\n",
                    Tex->Width, Tex->Height, Tex->FileName.c_str(), Alloc.addr, Tex->GsSize,
                    /*Tex->SrcDataSize*/ Tex->GsSize*256);

			nglPerfInfo.TextureDataStreamed += /*Tex->SrcDataSize*/ Tex->GsSize*256;

			START_PROF_TIMER( proftimer_texstream_dmabuild );
			if ( Tex->Type == NGLTEX_TGA )
			{
				Tex->GsBaseTBP = Alloc.addr;
				( (sceGsTex0*)&Tex->GsTex0 )->TBP0 = Tex->GsBaseTBP;
				if (!Tex->Flags.VRAMOnly)
				{
					u_int TexDataQWC = ( Tex->SrcDataSize + 15 ) / 16;
					nglGifPkRefLoadImage( &nglGifPkt,
						Tex->GsBaseTBP, Tex->Format, ( Tex->Width + 63 ) / 64,
						(u_long128*)Tex->Data, TexDataQWC,
						Tex->Width, Tex->Height );
				}
			}
			else if ( Tex->Type == NGLTEX_TIM2 )
			{
				// Add DMAs for the palette, main texture and each Mipmap level.
				// (Only uploads if Tex->VRAMOnly is not set)
				u_long* GifPacket = (u_long*)nglGifPkt.pCurrent;
				nglUploadTexture( GifPacket, Tex, Alloc.addr );
				nglGifPkt.pCurrent = (u_int*)GifPacket;
			}
      if (Tex==nglCurScene->RenderTarget)
      {
        //If the current render target has just been uploaded/allocated,
        // ensure that the frame buffer pointer is pointing to it.
        // This would only happen for a streamed render target, if during
        // rendering to it, the target buffer got flushed and reloaded,
        // otherwise loading the frame buffer pointer is handled
        // in nglVif1AddSetDrawEnv whenever the current scene is changed.
        // Note that having the frame buffer flushed and then reloaded in the
        // middle of rendering to it is probably not desirable ;-)
        // The following code is also invoked the first time a streamed
        // render target is allocated, in which case it is redundantly
        // handled again by nglVif1AddSetDrawEnv.
        nglVif1StartDirectGifAD();
        if ( Tex->Format == SCE_GS_PSMCT16 )
        {
          nglVif1AddRawGSReg( SCE_GS_FRAME_1,    SCE_GS_SET_FRAME( Tex->GsBaseTBP / 32, ( Tex->Width + 63 ) / 64, SCE_GS_PSMCT16S, ~nglCurScene->FBWriteMask ) );
          nglVif1AddRawGSReg( SCE_GS_FBA_1,      SCE_GS_SET_FBA( 1 ) );          // Alpha correction enabled.
        }
        else
        {
          nglVif1AddRawGSReg( SCE_GS_FRAME_1,    SCE_GS_SET_FRAME( Tex->GsBaseTBP / 32, ( Tex->Width + 63 ) / 64, Tex->Format, ~nglCurScene->FBWriteMask ) );
          nglVif1AddRawGSReg( SCE_GS_FBA_1,      SCE_GS_SET_FBA( 0 ) );          // Alpha correction disabled.
        }
        nglVif1EndDirectGifAD( Packet );
      }

			STOP_PROF_TIMER( proftimer_texstream_dmabuild );

			Alloc.alloc(Tex->GsSize);

			++nglVif1IntCurTexEntry->NTextures;
			nglVif1IntCurTexEntry->DataSize += Tex->GsSize;
		}
	}
	nglTexStreamCurStartRef = StartRef;
	nglTexStreamAlloc = Alloc;

	STOP_PROF_TIMER( proftimer_texstream_addtexture );
	return true;
}

void nglVif1IntCloseTextureBlock()
{
  if (nglTexStreamCurStartRef==nglTexStreamAlloc)
  {
	  nglTexStreamAlloc.cycle += 2;
	  nglTexStreamAlloc.addr = NGL_VRAM_STREAM_START;
	  nglTexStreamPrevStartRef = nglTexStreamAlloc;
	  nglTexStreamCurStartRef = nglTexStreamAlloc;
  }
  else
  {
	  nglTexStreamPrevStartRef = nglTexStreamCurStartRef;
	  nglTexStreamCurStartRef = nglTexStreamAlloc;
  }

  // Flush the texture cache
  sceGifPkCnt( &nglGifPkt, 0, 0, 0 );
  sceGifPkAddGsData( &nglGifPkt, SCE_GIF_SET_TAG( 1, 1, NULL, NULL, SCE_GIF_PACKED, 1 ) );
  sceGifPkAddGsData( &nglGifPkt, 0xEL );
  sceGifPkAddGsData( &nglGifPkt, 0 );
  sceGifPkAddGsData( &nglGifPkt, SCE_GS_TEXFLUSH );

  // End DMA transfer
  sceGifPkEnd( &nglGifPkt, 0, 0, 0 );
  sceGifPkTerminate( &nglGifPkt );

  if ( ( (u_int)nglGifPkt.pCurrent & NGL_DMA_MEM ) - (u_int)nglGifPacketWork >= nglGifPacketWorkSize )
    nglFatal( "Gif packet workspace overflow (%d/%d).\n",( (u_int)nglGifPkt.pCurrent & NGL_DMA_MEM ) - (u_int)nglGifPacketWork, nglGifPacketWorkSize );

  // Align and close off the GIF packet, updating the global GIF packet workspace pointer..
  nglGifPacketNext = nglGifPkt.pCurrent;
  while ( (u_int)nglGifPacketNext & 15 ) *(nglGifPacketNext++) = 0;

  if ( ( DEBUG_ENABLE( DumpFrameLog ) & 1 ) && nglVif1IntCurTexEntry )
    nglPrintf( "LOG: End of block.  NTex: %d, Size: %d (%d bytes).\n", nglVif1IntCurTexEntry->NTextures, nglVif1IntCurTexEntry->DataSize, nglVif1IntCurTexEntry->DataSize*256 );
}

void nglVif1IntCreateTextureBlock()
{
  if ( DEBUG_ENABLE( DumpFrameLog ) & 1 )
    nglPrintf( "LOG: vvvvvvvvvvvvvvvvvvvvvvv Starting new texture block vvvvvvvvvvvvvvvvvvvvvvv\n" );

  // Create a new interrupt list entry.
  nglVif1IntEntry* Entry;
  Entry = &nglVif1IntArray[nglNVif1IntEntries++];
  if ( nglNVif1IntEntries >= NGL_INT_MAX_ENTRIES )
    return;

  Entry->Type = NGLINT_LOADTEXTURE;
  Entry->NTextures = 0;
  Entry->DataSize = 0;

  nglVif1IntCurTexEntry = Entry;

  // Append the DMA chain for the block into the GIF packet workspace.
  Entry->GifDMA = nglGifPacketNext;
  sceGifPkInit( &nglGifPkt, (u_long128*)( NGL_UNCACHED_ACCEL_MEM | (u_int)Entry->GifDMA ) );
  sceGifPkReset( &nglGifPkt );
}

void nglVif1IntAddFinish( u_int*& Packet )
{
  nglVif1IntEntry* Entry = &nglVif1IntArray[nglNVif1IntEntries++];
  if ( nglNVif1IntEntries >= NGL_INT_MAX_ENTRIES )
    return;

  Entry->Type = NGLINT_FINISH;

  *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
  *(Packet++) = SCE_VIF1_SET_NOP( 1 );
}

//NOTE:  Do not use floating point in nglVif1Interrupt or any of the routines it calls!
//  The floating point registers are not saved and restored, so must not be modified!
int nglVif1Interrupt( int ca )
{
  START_PROF_TIMER( proftimer_send_interrupt );

  // This shouldn't be neccessary, if FLUSHA in the VIF chain takes care of it (but it doesn't).
  START_PROF_TIMER( proftimer_send_interrupt_dmawait );
  nglGifSafeWait();
  STOP_PROF_TIMER( proftimer_send_interrupt_dmawait );

  register int cnt;
  if ( DEBUG_ENABLE( DebugPrints ) )
  {
    cnt = *T1_COUNT;
    nglPrintf( "enter interrupt %d at %d cycles.\n", _nglVif1IntEntryIdx, cnt );
  }

  if ( ca != INTC_VIF1 )
  {
    nglWarning( "nglVif1Interrupt: Bad interrupt cause.\n" );
    goto InterruptExit;
  }

  if ( _nglVif1IntEntryIdx >= _nglNVif1IntEntries )
  {
    nglWarning( "nglVif1Interrupt: Extra interrupt received (%d/%d).\n", _nglVif1IntEntryIdx + 1, _nglNVif1IntEntries );
    goto InterruptExit;
  }

  nglVif1IntEntry* Entry;
  Entry = &_nglVif1IntArray[_nglVif1IntEntryIdx++];
  if ( Entry->Type == NGLINT_FINISH )
  {
    nglPerfInfo.RenderFinish = nglGetCPUCycle();
    nglDMAFinished = true;
  }
  else
  if ( Entry->Type == NGLINT_LOADTEXTURE )
  {
    // Start a path 3 (dma channel 2) chain mode transfer
    if ( Entry->NTextures )
      _nglDmaStartGifSourceChain( Entry->GifDMA, 0 );
  }

InterruptExit:
  if ( DEBUG_ENABLE( DebugPrints ) )
  {
    cnt = *T1_COUNT;
    nglPrintf( "leave interrupt %d at %d cycles.\n", _nglVif1IntEntryIdx - 1, cnt );
  }

  // Restart the VIF.
  DPUT_VIF1_FBRST( VIF1_FBRST_STC_M );
  asm __volatile__(" sync.l ");

  STOP_PROF_TIMER( proftimer_send_interrupt );

  // to insure all store instructions are complete
  ExitHandler();
  return 0;
}

void* nglTim2GetClut( nglTexture *Tex )
{
  TIM2_PICTUREHEADER *ph=Tex->ph;

  // calculate the top address of the image data
  void *pClut = (void *)((char *)ph + ph->HeaderSize);

  return(pClut);
}

void* nglTim2GetImage( nglTexture *Tex, int mipmap )
{
  TIM2_PICTUREHEADER *ph=Tex->ph;

  if(mipmap>=ph->MipMapTextures) {
    // no MIPMAP texture of the assumed level
    return(NULL);
  }

  void* pImage = Tex->GsImage[mipmap].Data;

  return(pImage);
}

int nglTim2GetMipMapPictureSize(TIM2_PICTUREHEADER *ph, int mipmap, int *pWidth, int *pHeight)
{
  int w, h, n;
  w = ph->ImageWidth>>mipmap;
  h = ph->ImageHeight>>mipmap;
  if(pWidth) {
    *pWidth  = w;
  }
  if(pHeight) {
    *pHeight = h;
  }

  n = w * h;
  switch(ph->ImageType) {
    case TIM2_RGB16:  n *= 2;   break;
    case TIM2_RGB24:  n *= 3;   break;
    case TIM2_RGB32:  n *= 4;   break;
    case TIM2_IDTEX4: n /= 2;   break;
    case TIM2_IDTEX8:       break;
  }

  // each MIPMAP texture is aligned to 16 byte boundary, regardless of
  // the setting by FormatId
  n = (n + 15) & ~15;
  return(n);
}

//Tim2 texture Conversion from PSMT4,PSMT8,PSMCT16 -> PSMCT32
#define PSMCT32_BLOCK_WIDTH 8
#define PSMCT32_BLOCK_HEIGHT 8
#define PSMCT32_PAGE_WIDTH  64
#define PSMCT32_PAGE_HEIGHT 32

//Convert texture from PSMT4 -> PSMCT32
int BlockPSMT4toPSMCT32(u_char *p_input, u_char *p_output)
{

  static int lut[] = {
    // even column
    0, 68, 8,  76, 16, 84, 24, 92,
    1, 69, 9,  77, 17, 85, 25, 93,
    2, 70, 10, 78, 18, 86, 26, 94,
    3, 71, 11, 79, 19, 87, 27, 95,
    4, 64, 12, 72, 20, 80, 28, 88,
    5, 65, 13, 73, 21, 81, 29, 89,
    6, 66, 14, 74, 22, 82, 30, 90,
    7, 67, 15, 75, 23, 83, 31, 91,

    32, 100, 40, 108, 48, 116, 56, 124,
    33, 101, 41, 109, 49, 117, 57, 125,
    34, 102, 42, 110, 50, 118, 58, 126,
    35, 103, 43, 111, 51, 119, 59, 127,
    36, 96,  44, 104, 52, 112, 60, 120,
    37, 97,  45, 105, 53, 113, 61, 121,
    38, 98,  46, 106, 54, 114, 62, 122,
    39, 99,  47, 107, 55, 115, 63, 123,


    // odd column
    4, 64, 12, 72, 20, 80, 28, 88,
    5, 65, 13, 73, 21, 81, 29, 89,
    6, 66, 14, 74, 22, 82, 30, 90,
    7, 67, 15, 75, 23, 83, 31, 91,
    0, 68, 8,  76, 16, 84, 24, 92,
    1, 69, 9,  77, 17, 85, 25, 93,
    2, 70, 10, 78, 18, 86, 26, 94,
    3, 71, 11, 79, 19, 87, 27, 95,

    36, 96,  44, 104, 52, 112, 60, 120,
    37, 97,  45, 105, 53, 113, 61, 121,
    38, 98,  46, 106, 54, 114, 62, 122,
    39, 99,  47, 107, 55, 115, 63, 123,
    32, 100, 40, 108, 48, 116, 56, 124,
    33, 101, 41, 109, 49, 117, 57, 125,
    34, 102, 42, 110, 50, 118, 58, 126,
    35, 103, 43, 111, 51, 119, 59, 127
  };

  unsigned int i, j, k, i0, i1, i2;
  unsigned int index0, index1;
  unsigned char c_in, c_out, *pIn;

  pIn = p_input;

  // for first step, we only think for a single block. (4bits, 32x16)
  index1 = 0;

  for(k = 0; k < 4; k++)
  {
    index0 = (k % 2) * 128;

    for(i = 0; i < 16; i++)
    {

      for(j = 0; j < 4; j++)
      {

        c_out = 0x00;

        // lower 4bit.
        i0 = lut[index0++];
        i1 = i0 / 2;
        i2 = (i0 & 0x1) * 4;
        c_in = (pIn[i1] & (0x0f << i2)) >> i2;
        c_out = c_out | c_in;

        // uppper 4bit
        i0 = lut[index0++];
        i1 = i0 / 2;
        i2 = (i0 & 0x1) * 4;
        c_in = (pIn[i1] & (0x0f << i2)) >> i2;
        c_out = c_out | ((c_in << 4) & 0xf0);

        p_output[index1++] = c_out;
      }
    }
    pIn += 64;
  }

  return 0;
}

#define PSMT4_BLOCK_WIDTH 32
#define PSMT4_BLOCK_HEIGHT 16

int PagePSMT4toPSMCT32(int width, int height, u_char *p_input, u_char *p_output)
{

  static u_int block_table4[] = {
    0,  2,  8, 10,
    1,  3,  9, 11,
    4,  6, 12, 14,
    5,  7, 13, 15,
    16, 18, 24, 26,
    17, 19, 25, 27,
    20, 22, 28, 30,
    21, 23, 29, 31
  };

  static u_int block_table32[] = {
    0,  1,  4,  5, 16, 17, 20, 21,
    2,  3,  6,  7, 18, 19, 22, 23,
    8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31
  };

  u_int *index32_h, *index32_v, in_block_nb;

  u_char input_block[16 * 16], output_block[16 * 16];
  u_char *pi0, *pi1, *po0, *po1;
  int index0, index1, i, j, k;
  int n_width, n_height, input_page_line_size;
  int output_page_line_size;

  // --- create table for output 32bit buffer ---
  index32_h = (u_int*) nglMemAlloc(8 * 4 * sizeof(u_int));
  index32_v = (u_int*) nglMemAlloc(8 * 4 * sizeof(u_int));
  index0 = 0;
  for(i = 0; i < 4; i++)
  {
    for(j = 0; j < 8; j++)
    {
      index1 = block_table32[index0];
      index32_h[index1] = j;
      index32_v[index1] = i;
      index0++;
    }
  }

  n_width = width / 32;
  n_height = height / 16;

  memset(input_block, 0, 16 *16);
  memset(output_block, 0, 16 * 16);

  input_page_line_size = 128 / 2;    // PSMT4 page width (byte)
  output_page_line_size = 256;       // PSMCT32 page width (byte)

  // now assume copying from page top.
  for(i = 0; i < n_height; i++)
  {

    for(j = 0; j < n_width; j++)
    {

      pi0 = input_block;
      pi1 = p_input + 16 * i * input_page_line_size + j * 16;

      in_block_nb = block_table4[i * n_width + j];

      for(k = 0; k < PSMT4_BLOCK_HEIGHT; k++)
      {
        memcpy(pi0, pi1, PSMT4_BLOCK_WIDTH / 2); // copy full 1 line of 1 block.
        pi0 += PSMT4_BLOCK_WIDTH / 2;
        pi1 += input_page_line_size;
      }

      BlockPSMT4toPSMCT32(input_block, output_block);

      po0 = output_block;
      po1 = p_output + 8 * index32_v[in_block_nb] * output_page_line_size + index32_h[in_block_nb] * 32;
      for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++)
      {
        memcpy(po1, po0, PSMCT32_BLOCK_WIDTH * 4);
        po0 += PSMCT32_BLOCK_WIDTH * 4;
        po1 += output_page_line_size;
      }

    }
  }

  nglMemFree(index32_h);
  nglMemFree(index32_v);

  return 0;
}

#define PSMT4_PAGE_WIDTH    128
#define PSMT4_PAGE_HEIGHT   128

int nglTim2Convert4to32(int width, int height, u_char *p_input, u_char *p_output)
{
  int i, j, k;
  int n_page_h, n_page_w, n_page4_width_byte, n_page32_width_byte;
  u_char *pi0, *pi1, *po0, *po1;
  int n_input_width_byte, n_output_width_byte, n_input_height, n_output_height;
  u_char input_page[PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT];
  u_char output_page[PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT];

  // ----- check width -----
  for(i = 0; i < 11; i++)
  {
    if( width == (0x400 >> i) )
      break;
  }
  if(i == 11)
  {
    nglFatal("TM2 width is not power of 2.\n");
    return -1;
  }

//  nglPrintf("input_page: %d\n", PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT);
//  nglPrintf("output_page: %d\n", PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

  memset(input_page, 0, PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT);
  memset(output_page, 0, PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

  // ----- check height -----
  for(i = 0; i < 11; i++)
  {
    if(height == (0x400 >> i))
      break;
  }
  if(i == 11)
  {
    nglFatal("TM2 height is not power of 2.\n");
    return -1;
  }

  n_page_w = (width - 1) / PSMT4_PAGE_WIDTH + 1;
  n_page_h = (height - 1) / PSMT4_PAGE_HEIGHT + 1;

  n_page4_width_byte = PSMT4_PAGE_WIDTH / 2;
  n_page32_width_byte = PSMCT32_PAGE_WIDTH * 4;

//  nglPrintf("n_page_w : %d\n", n_page_w );
//  nglPrintf("n_page_h : %d\n", n_page_h );

//  nglPrintf("n_page4_width_byte : %d\n", n_page4_width_byte );
//  nglPrintf("n_page32_width_byte : %d\n", n_page32_width_byte );


  // --- set in/out buffer size (for image smaller than one page) ---
  if(n_page_w == 1)
  {
    n_input_width_byte = width / 2;
    n_output_height = width / 4;
  }
  else
  {
    n_input_width_byte = n_page4_width_byte;
    n_output_height = PSMCT32_PAGE_HEIGHT;
  }

  if(n_page_h == 1)
  {
    n_input_height = height;
    n_output_width_byte = height * 2;
  }
  else
  {
    n_input_height = PSMT4_PAGE_HEIGHT;
    n_output_width_byte = n_page32_width_byte;
  }


  for(i = 0; i < n_page_h; i++)
  {
    for(j = 0; j < n_page_w; j++)
    {
      pi0 = p_input + (n_input_width_byte * PSMT4_PAGE_HEIGHT) * n_page_w * i
        + n_input_width_byte * j;
      pi1 = input_page;

      for(k = 0; k < n_input_height; k++)
      {
        memcpy(pi1, pi0, n_input_width_byte);
        pi0 += n_input_width_byte * n_page_w;
        pi1 += n_page4_width_byte;
      }

      PagePSMT4toPSMCT32(PSMT4_PAGE_WIDTH, PSMT4_PAGE_HEIGHT, input_page, output_page);

      po0 = p_output + (n_output_width_byte * PSMCT32_PAGE_HEIGHT) * n_page_w * i
        + n_output_width_byte * j;
      po1 = output_page;
      for(k = 0; k < n_output_height; k++)
      {
        memcpy(po0, po1, n_output_width_byte);
        po0 += n_output_width_byte * n_page_w;
        po1 += n_page32_width_byte;
      }
    }
  }

  return 0;
}


//Convert texture from PSMT8 -> PSMCT32

int nglBlockPSMT8toPSMCT32(u_char *p_input, u_char *p_output)
{

  static int lut[] = {
    // even column
    0, 36, 8,  44,
    1, 37, 9,  45,
    2, 38, 10, 46,
    3, 39, 11, 47,
    4, 32, 12, 40,
    5, 33, 13, 41,
    6, 34, 14, 42,
    7, 35, 15, 43,

    16, 52, 24, 60,
    17, 53, 25, 61,
    18, 54, 26, 62,
    19, 55, 27, 63,
    20, 48, 28, 56,
    21, 49, 29, 57,
    22, 50, 30, 58,
    23, 51, 31, 59,

    // odd column
    4, 32, 12, 40,
    5, 33, 13, 41,
    6, 34, 14, 42,
    7, 35, 15, 43,
    0, 36, 8,  44,
    1, 37, 9,  45,
    2, 38, 10, 46,
    3, 39, 11, 47,

    20, 48, 28, 56,
    21, 49, 29, 57,
    22, 50, 30, 58,
    23, 51, 31, 59,
    16, 52, 24, 60,
    17, 53, 25, 61,
    18, 54, 26, 62,
    19, 55, 27, 63
  };

  unsigned int i, j, k, i0;
  unsigned int index0, index1;
  unsigned char *pIn;

  pIn = p_input;

  // for first step, we only think for a single block. (4bits, 32x16)
  index1 = 0;

  for(k = 0; k < 4; k++)
  {

    index0 = (k % 2) * 64;

    for(i = 0; i < 16; i++)
    {
      for(j = 0; j < 4; j++)
      {
        i0 = lut[index0++];
        p_output[index1++] = pIn[i0];
      }
    }
    pIn += 64;
  }

  return 0;
}

#define PSMT8_BLOCK_WIDTH  16
#define PSMT8_BLOCK_HEIGHT 16

int nglPagePSMT8toPSMCT32(int width, int height, u_char *p_input, u_char *p_output)
{

  static u_int block_table8[] = {
    0,  1,  4,  5, 16, 17, 20, 21,
    2,  3,  6,  7, 18, 19, 22, 23,
    8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31
  };

  static u_int block_table32[] = {
    0,  1,  4,  5, 16, 17, 20, 21,
    2,  3,  6,  7, 18, 19, 22, 23,
    8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31
  };

  u_int *index32_h, *index32_v, in_block_nb;

  u_char input_block[16 * 16], output_block[16 * 16];
  u_char *pi0, *pi1, *po0, *po1;
  int index0, index1, i, j, k;
  int n_width, n_height, input_page_line_size;
  int output_page_line_size;


  // --- create table for output 32bit buffer ---
  index32_h = (u_int*) nglMemAlloc(8 * 4 * sizeof(u_int));
  index32_v = (u_int*) nglMemAlloc(8 * 4 * sizeof(u_int));
  index0 = 0;
  for(i = 0; i < 4; i++)
  {
    for(j = 0; j < 8; j++)
    {
      index1 = block_table32[index0];
      index32_h[index1] = j;
      index32_v[index1] = i;
      index0++;
    }
  }


  // how many blocks we should calc (width/height)
  n_width = width / PSMT8_BLOCK_WIDTH;
  n_height = height / PSMT8_BLOCK_HEIGHT;

  memset(input_block, 0, 16 *16);
  memset(output_block, 0, 16 * 16);

  input_page_line_size  = 128;    // PSMT8 page width (byte)
  output_page_line_size = 256;    // PSMCT32 page width (byte)

  // now assume copying from page top.
  for(i = 0; i < n_height; i++)
  {

    for(j = 0; j < n_width; j++)
    {

      pi0 = input_block;
      pi1 = p_input + PSMT8_BLOCK_HEIGHT * i * input_page_line_size + j * PSMT8_BLOCK_WIDTH; // byte

      in_block_nb = block_table8[i * n_width + j];

      for(k = 0; k < PSMT8_BLOCK_HEIGHT; k++)
      {
        memcpy(pi0, pi1, PSMT8_BLOCK_WIDTH); // copy full 1 line of 1 block.
        pi0 += PSMT8_BLOCK_WIDTH;
        pi1 += input_page_line_size;
      }

      nglBlockPSMT8toPSMCT32(input_block, output_block);

      po0 = output_block;
      po1 = p_output + 8 * index32_v[in_block_nb] * output_page_line_size + index32_h[in_block_nb] * 32;
      for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++)
      {
        memcpy(po1, po0, PSMCT32_BLOCK_WIDTH * 4);
        po0 += PSMCT32_BLOCK_WIDTH * 4;
        po1 += output_page_line_size;
      }

    }
  }

  nglMemFree(index32_h);
  nglMemFree(index32_v);

  return 0;
}

#define PSMT8_PAGE_WIDTH    128
#define PSMT8_PAGE_HEIGHT   64

int nglTim2Convert8to32(int width, int height, u_char *p_input, u_char *p_output)
{
  int i, j, k;
  int n_page_h, n_page_w, n_page8_width_byte, n_page32_width_byte;
  int n_input_width_byte, n_output_width_byte, n_input_height, n_output_height;
  u_char *pi0, *pi1, *po0, *po1;
  u_char input_page[PSMT8_PAGE_WIDTH * PSMT8_PAGE_HEIGHT];
  u_char output_page[PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT];

  // ----- check width -----
  for(i = 0; i < 11; i++)
  {
    if(width == (0x400 >> i))
      break;
  }
  if(i == 11)
  {
    nglFatal("TM2 width is not power of 2.\n");
    return -1;
  }

  // ----- check height -----
  for(i = 0; i < 11; i++)
  {
    if(height == (0x400 >> i))
      break;
  }
  if(i == 11)
  {
    nglFatal("TM2 height is not power of 2.\n");
    return -1;
  }

  memset(input_page, 0, PSMT8_PAGE_WIDTH * PSMT8_PAGE_HEIGHT);
  memset(output_page, 0, PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

  n_page_w = (width - 1) / PSMT8_PAGE_WIDTH + 1;
  n_page_h = (height - 1) / PSMT8_PAGE_HEIGHT + 1;

  n_page8_width_byte = PSMT8_PAGE_WIDTH;
  n_page32_width_byte = PSMCT32_PAGE_WIDTH * 4;

  // --- set in/out buffer size (for image smaller than one page) ---
  if(n_page_w == 1)
  {
    n_input_width_byte = width;
    n_output_width_byte = width * 2;
  }
  else
  {
    n_input_width_byte = n_page8_width_byte;
    n_output_width_byte = n_page32_width_byte;
  }

  if(n_page_h == 1)
  {
    n_input_height = height;
    n_output_height = height / 2;
  }
  else
  {
    n_input_height = PSMT8_PAGE_HEIGHT;
    n_output_height = PSMCT32_PAGE_HEIGHT;
  }

  // --- conversion ---
  for(i = 0; i < n_page_h; i++)
  {
    for(j = 0; j < n_page_w; j++)
    {
      pi0 = p_input + (n_input_width_byte * PSMT8_PAGE_HEIGHT) * n_page_w * i
        + n_input_width_byte * j;
      pi1 = input_page;

      for(k = 0; k < n_input_height; k++)
      {
        memcpy(pi1, pi0, n_input_width_byte);
        pi0 += n_input_width_byte * n_page_w;
        pi1 += n_page8_width_byte;
      }

      // --- convert a page ---
      nglPagePSMT8toPSMCT32(PSMT8_PAGE_WIDTH, PSMT8_PAGE_HEIGHT, input_page, output_page);

      po0 = p_output + (n_output_width_byte * n_output_height) * n_page_w * i
        + n_output_width_byte * j;
      po1 = output_page;
      for(k = 0; k < n_output_height; k++)
      {
        memcpy(po0, po1, n_output_width_byte);
        po0 += n_output_width_byte * n_page_w;
        po1 += n_page32_width_byte;
      }
    }
  }

  return 0;
}

//Convert texture from PSMCT16 -> PSMCT32

#define COLUMN_SIZE_16  64 // in bytes

int nglBlockPSMCT16toPSMCT32(u_char *block_in, u_char *block_out)
{
  int lut[]={
     0,  8,
     1,  9,
     2, 10,
     3, 11,
     4, 12,
     5, 13,
     6, 14,
     7, 15,
    16, 24,
    17, 25,
    18, 26,
    19, 27,
    20, 28,
    21, 29,
    22, 30,
    23, 31,
  };
  int i, j;
  u_char column_in[COLUMN_SIZE_16], column_out[COLUMN_SIZE_16];

  for (i=0; i<4; i++)
  {
    memcpy(column_in, block_in+COLUMN_SIZE_16*i, COLUMN_SIZE_16);

    for (j=0; j<16; j++)
    {
      memcpy(column_out+4*j+0, column_in+lut[j*2+0]*2, 2);
      memcpy(column_out+4*j+2, column_in+lut[j*2+1]*2, 2);
    }

    memcpy(block_out+COLUMN_SIZE_16*i, column_out, COLUMN_SIZE_16);
  }

  return 0;
}

#define PAGE_WIDTH      64 // in pixels
#define PAGE_HEIGHT_16  64 // in lines
#define PAGE_HEIGHT_32  32 // in lines
#define PAGE_SIZE     8192 // in bytes

#define BLOCK_WIDTH_16  16 // in pixels
#define BLOCK_HEIGHT_16  8 // in pixels
#define BLOCK_SIZE_16  256 // in bytes

int nglPagePSMCT16toPSMCT32(int width, int height, u_char *p_input, u_char *p_output)
{

  static u_int block_table16[] = {
     0,  2,  8, 10,
     1,  3,  9, 11,
     4,  6, 12, 14,
     5,  7, 13, 15,
    16, 18, 24, 26,
    17, 19, 25, 27,
    20, 22, 28, 30,
    21, 23, 29, 31
  };

  static u_int block_table32[] = {
    0,  1,  4,  5, 16, 17, 20, 21,
    2,  3,  6,  7, 18, 19, 22, 23,
    8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31
  };

  u_int *index32_h, *index32_v, in_block_nb;

  u_char input_block[16 * 16], output_block[16 * 16];
  u_char *pi0, *pi1, *po0, *po1;
  int index0, index1, i, j, k;
  int n_width, n_height, input_page_line_size;
  int output_page_line_size;



  // --- create table for output 32bit buffer ---
  index32_h = (u_int*) nglMemAlloc(8 * 4 * sizeof(u_int));
  index32_v = (u_int*) nglMemAlloc(8 * 4 * sizeof(u_int));
  index0 = 0;
  for(i = 0; i < 4; i++)
  {
    for(j = 0; j < 8; j++)
    {
      index1 = block_table32[index0];
      index32_h[index1] = j;
      index32_v[index1] = i;
      index0++;
    }
  }


  // how many blocks we should calc (width/height)
  n_width = width / BLOCK_WIDTH_16;
  n_height = height / BLOCK_HEIGHT_16;

  memset(input_block, 0xFF, 16 *16);
  memset(output_block, 0xFF, 16 * 16);

  input_page_line_size  = 128;    // PSMCT16 page width (byte)
  output_page_line_size = 256;    // PSMCT32 page width (byte)

  // now assume copying from page top.
  for(i = 0; i < n_height; i++)
  {

    for(j = 0; j < n_width; j++)
    {

      pi0 = input_block;
      pi1 = p_input + BLOCK_HEIGHT_16 * i * input_page_line_size + j * BLOCK_WIDTH_16*2; // byte

      in_block_nb = block_table16[i * n_width + j];

      for(k = 0; k < BLOCK_HEIGHT_16; k++) {
        memcpy(pi0, pi1, BLOCK_WIDTH_16*2); // copy full 1 line of 1 block.
        pi0 += BLOCK_WIDTH_16*2;
        pi1 += input_page_line_size;
      }

      nglBlockPSMCT16toPSMCT32(input_block, output_block);

      po0 = output_block;
      po1 = p_output + 8 * index32_v[in_block_nb] * output_page_line_size + index32_h[in_block_nb] * 32;
      for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++)
      {
        memcpy(po1, po0, PSMCT32_BLOCK_WIDTH * 4);
        po0 += PSMCT32_BLOCK_WIDTH * 4;
        po1 += output_page_line_size;
      }

    }
  }

  nglMemFree(index32_h);
  nglMemFree(index32_v);

  return 0;
}

int nglTim2Convert16to32(int width, int height, u_char *image_in, u_char *image_out)
{
  int number_pages_w, nx;
  int number_pages_h, ny;
  int l, x, y, w;

  u_char page_in[PAGE_SIZE], page_out[PAGE_SIZE];

  // get the number of pages
  number_pages_w=width/PAGE_WIDTH;
  number_pages_h=height/PAGE_HEIGHT_16;

  // for each page
  for (ny=0; ny<number_pages_h; ny++)
  {
    for (nx=0; nx<number_pages_w; nx++)
    {
      // get page data
      for (l=0; l<PAGE_HEIGHT_16; l++)
      {
        x=nx*PAGE_WIDTH*2;
        y=ny*PAGE_HEIGHT_16;
        w=width*2;
        memcpy(page_in+l*PAGE_WIDTH*2, image_in+x+(y+l)*w, PAGE_WIDTH*2);
      }

      // convert page from PSMCT16 to PSMCT32
    //  Page16to32(page_in, page_out);
      nglPagePSMCT16toPSMCT32(PAGE_WIDTH, PAGE_HEIGHT_16, page_in, page_out);

      // write converted page to output buffer
      for (l=0; l<PAGE_HEIGHT_16/2; l++)
      {
        x=(nx%(number_pages_w/2))*PAGE_WIDTH*4;
        y=(ny*2+(nx>=number_pages_w/2))*PAGE_HEIGHT_32;
        w=width*2;
        memcpy(image_out+x+(y+l)*w, page_out+l*PAGE_WIDTH*4, PAGE_WIDTH*4);
      }
    }
  }

  return 0;
}

//----------------------------------------------------------------------------------------
//  @Screenshot code.
//----------------------------------------------------------------------------------------
void nglDownloadTexture( nglTexture *Dest, nglTexture *Source )
{
  sceGsStoreImage gs_simage;
  sceGsSetDefStoreImage( &gs_simage, Source->GsBaseTBP, Dest->Width / 64, SCE_GS_PSMCT32, 0, 0, Dest->Width, Dest->Height );
  FlushCache(0);
  sceGsExecStoreImage( &gs_simage, (u_long128 *)Dest->Data );
  sceGsSyncPath(0, 0);
}

// Thank leo for the bmp work that i stole from him.
bool nglExportTextureBMP( nglTexture* Tex, const char *Name )
{
#ifndef NGL_FINAL
  int fd;
  u_char *row_buf;

  u_int byteswritten = 0;
  u_short bits = 24;
  u_int cmap = 0;
  u_int bfSize = 54 + Tex->Width*Tex->Height*3;

  u_long pixoff = 54 + cmap*4;
  u_short res = 0;

  fd = sceOpen((char*)Name, SCE_WRONLY | SCE_TRUNC | SCE_CREAT);

  char m1 ='B', m2 ='M';
  sceWrite(fd, &m1, 1);       byteswritten++; // B
  sceWrite(fd, &m2, 1);       byteswritten++; // M
  sceWrite(fd, &bfSize, 4);   byteswritten+=4;// bfSize
  sceWrite(fd, &res, 2);      byteswritten+=2;// bfReserved1
  sceWrite(fd, &res, 2);      byteswritten+=2;// bfReserved2
  sceWrite(fd, &pixoff, 4);   byteswritten+=4;// bfOffBits

  u_int biSize = 40, compress = 0, size = 0;
  u_int width = Tex->Width, height = Tex->Height, pixels = 0;
  u_short planes = 1;
  sceWrite(fd, &biSize, 4);   byteswritten+=4;// biSize
  sceWrite(fd, &width, 4);    byteswritten+=4;// biWidth
  sceWrite(fd, &height, 4);   byteswritten+=4;// biHeight
  sceWrite(fd, &planes, 2);   byteswritten+=2;// biPlanes
  sceWrite(fd, &bits, 2);     byteswritten+=2;// biBitCount
  sceWrite(fd, &compress, 4); byteswritten+=4;// biCompression
  sceWrite(fd, &size, 4);     byteswritten+=4;// biSizeImage
  sceWrite(fd, &pixels, 4);   byteswritten+=4;// biXPelsPerMeter
  sceWrite(fd, &pixels, 4);   byteswritten+=4;// biYPelsPerMeter
  sceWrite(fd, &cmap, 4);     byteswritten+=4;// biClrUsed
  sceWrite(fd, &cmap, 4);     byteswritten+=4;// biClrImportant

  u_int widthDW = (((Tex->Width*24) + 31) / 32 * 4);
  u_int row_size = Tex->Width * 3;
  u_int row;

  row_buf = (u_char*)nglMemAlloc( row_size );

  for ( row = 0; row < Tex->Height; row++ )
  {
    u_char *buf = (u_char *)(Tex->Data + (Tex->Height-row-1)*Tex->Width);

    // write a row
    for (u_int col = 0; col < Tex->Width; col++)
    {
      row_buf[col*3]     = buf[col*4 + 2];
      row_buf[col*3 + 1] = buf[col*4 + 1];
      row_buf[col*3 + 2] = buf[col*4 + 0];
    }
    sceWrite(fd, row_buf, row_size);

    byteswritten += row_size;

    for (u_int count = row_size; count < widthDW; count++)
    {
      u_char dummy = '\0';

      sceWrite(fd, &dummy, 1);
      byteswritten++;
    }
  }

  nglMemFree(row_buf);

  sceClose(fd);
#endif

  return true;
}

void nglExportTextureTGA( nglTexture* Tex, const char* FileName )
{
#ifndef NGL_FINAL
  int fd = sceOpen( (char*)FileName, SCE_WRONLY | SCE_TRUNC | SCE_CREAT );

  TGAHEADER hdr;
  memset( &hdr, 0, sizeof(TGAHEADER) );

  hdr.ImageType = 2;
  hdr.ImageWidth = Tex->Width;
  hdr.ImageHeight = Tex->Height;

  // 24bit textures export as 32bit TGAs.
  if ( Tex->Format == SCE_GS_PSMCT32 || Tex->Format == SCE_GS_PSMCT24 )
    hdr.PixelDepth = 32;
  else
  if ( Tex->Format == SCE_GS_PSMCT16 )
    hdr.PixelDepth = 16;

  sceWrite( fd, &hdr, sizeof(TGAHEADER) );

  u_int PixelSize = hdr.PixelDepth / 8;
  u_char* RowBuf = (u_char*)nglMemAlloc( Tex->Width * PixelSize );

  for ( u_int j = 0; j < Tex->Height; j++ )
  {
    u_char *Buf = (u_char*)Tex->Data + ( Tex->Height - j - 1 ) * Tex->Width * PixelSize;

    // write a row
    if ( Tex->Format == SCE_GS_PSMCT32 || Tex->Format == SCE_GS_PSMCT24 )
    {
      for ( u_int i = 0; i < Tex->Width; i++ )
      {
        RowBuf[i * 4 + 0] = Buf[i * 4 + 2];
        RowBuf[i * 4 + 1] = Buf[i * 4 + 1];
        RowBuf[i * 4 + 2] = Buf[i * 4 + 0];
        RowBuf[i * 4 + 3] = Buf[i * 4 + 3];
      }
    }
    else
    if ( Tex->Format == SCE_GS_PSMCT16 )
      memcpy( RowBuf, Buf, Tex->Width * PixelSize );

    sceWrite( fd, RowBuf, Tex->Width * PixelSize );
  }

  nglMemFree( RowBuf );
  sceClose( fd );
#endif
}

// Works only on 32bit and 16bit textures, and gives the contents from the previous frame, if locked
void nglSaveTexture( nglTexture* Tex, const char* FileName )
{
  // Wait for any render in progress to finish
  nglVif1SafeWait();

  // Save the texture.
  nglTexture Dest;
  static u_int SaveCount = 0;

  Dest.Format = Tex->Format;
  Dest.SrcDataSize = Tex->Width * Tex->Height * 4;
  Dest.Data = (u_int*)nglMemAlloc( Dest.SrcDataSize );
  Dest.Height = Tex->Height;
  Dest.Width = Tex->Width;

  if ( FileName )
    sprintf( nglWork, "host:%s.tga", FileName );
  else
    sprintf( nglWork, "host:savetex%d.tga", SaveCount++ );

  nglDownloadTexture( &Dest, Tex );
  nglExportTextureTGA( &Dest, nglWork );

  nglMemFree( Dest.Data );
  nglPrintf( "NGL: Saved texture %s to %s.\n", Tex->FileName.c_str(), nglWork );
}

void nglScreenShot(const char *FileName)
{
  static int ScreenCount = 0;
  if (FileName) 
  {
	  nglSaveTexture(&nglBackBufferTex, FileName);
  }
  else 
  {
	  static char Buf[64];
	  sprintf(Buf, "screenshot%4.4d", ScreenCount++);
	  nglSaveTexture(&nglBackBufferTex, Buf);
  }
}

/*---------------------------------------------------------------------------------------------------------
  @Mesh loading/management code.
---------------------------------------------------------------------------------------------------------*/
// Change all the internal pointers in a mesh structure to be based at a new location.  Useful for copying meshes around
// and for loading them from files (in which they're based at 0).
void nglRebaseMesh( u_int NewBase, u_int OldBase, nglMesh* Mesh, bool Scratch /*= false*/ )
{
#define PTR_OFFSET( Ptr, Type ) { if ( Ptr ) Ptr = (Type)( (u_int)Ptr + ( (u_int)NewBase - (u_int)OldBase ) ); }

  PTR_OFFSET( Mesh->Bones, nglMatrix* );
  PTR_OFFSET( Mesh->LODs, nglMeshLODInfo* );
  PTR_OFFSET( Mesh->Sections, nglMeshSection* );

  for ( u_int i = 0; i < Mesh->NSections; i++ )
  {
    nglMeshSection* Section = &Mesh->Sections[i];

    PTR_OFFSET( Section->BatchInfo, nglMeshBatchInfo* );
    PTR_OFFSET( Section->BatchDMA, u_int* );

    // All meshes are exported with their batch DMA at a multiple of 128 bytes to increase DMA speed.  If this
    // assert hits, either a) the exporter is broken or b) nglMemAlloc isn't respecting the alignment parameter.
    NGL_ASSERT( ( (u_int)Section->BatchDMA & 127 ) == 0, "Invalid mesh image, or nglMemAlloc alignment is broken." );

    PTR_OFFSET( Section->Material, nglMaterial* );
    if ( Scratch )
    {
      PTR_OFFSET( Section->Material->Info, nglMaterialInfo* );
      if ( Section->Material->Info )
        PTR_OFFSET( Section->Material->Info->BakedDMA, u_int* )
    }

    for ( u_int j = 0; j < Section->NBatches; j++ )
    {
      nglMeshBatchInfo* Batch = &Section->BatchInfo[j];
      PTR_OFFSET( Batch->StripData, u_int* );
      PTR_OFFSET( Batch->PosData, float* );
      PTR_OFFSET( Batch->NormData, float* );
      PTR_OFFSET( Batch->ColorData, u_int* );
      PTR_OFFSET( Batch->UVData, float* );
      PTR_OFFSET( Batch->LightUVData, float* );
      PTR_OFFSET( Batch->BoneCountData, char* );
      PTR_OFFSET( Batch->BoneIdxData, unsigned short* );
      PTR_OFFSET( Batch->BoneWeightData, float* );
    }
  }
#undef PTR_OFFSET
}

// Set up hash value for the material, for sorting by texture.
void nglCalcMaterialHash( nglMaterial* Material )
{
  nglMaterialInfo* Info = Material->Info;
  if ( !Info )
    return;

  u_int Size = 0;
  if ( Material->Map )
  {
    Info->Hash = Material->Map->Hash;
    Size = Material->Map->GsSize;
  }
  if ( Material->DetailMap && Material->DetailMap->GsSize > Size )
  {
    Info->Hash = Material->DetailMap->Hash;
    Size = Material->DetailMap->GsSize;
  }
  if ( Material->EnvironmentMap && Material->EnvironmentMap->GsSize > Size )
  {
    Info->Hash = Material->EnvironmentMap->Hash;
    Size = Material->EnvironmentMap->GsSize;
  }
  if ( Material->LightMap && Material->LightMap->GsSize > Size )
  {
    Info->Hash = Material->LightMap->Hash;
    Size = Material->LightMap->GsSize;
  }
}

void nglBindMeshTextures( nglMesh* Mesh, bool Load )
{
  for ( u_int i = 0; i < Mesh->NSections; i++ )
  {
    // Process the material data.
    nglMeshSection* Section = &Mesh->Sections[i];
    nglMaterial* Material = Section->Material;

    // Automatically load textures used by the mesh.
    if ( Material->Flags & NGLMAT_TEXTURE_MAP )
    {
      if ( Load )
        Material->Map = nglLoadTexture( Material->MapName );
      else
        Material->Map = nglGetTexture( Material->MapName );
      if ( !Material->Map ) Material->Map = nglWhiteTex;
    }

    if ( Material->Flags & NGLMAT_LIGHT_MAP )
    {
      if ( Load )
        Material->LightMap = nglLoadTexture( Material->LightMapName );
      else
        Material->LightMap = nglGetTexture( Material->LightMapName );
      if ( !Material->LightMap ) Material->LightMap = nglWhiteTex;
    }

    if ( Material->Flags & NGLMAT_DETAIL_MAP )
    {
      if ( Load )
        Material->DetailMap = nglLoadTexture( Material->DetailMapName );
      else
        Material->DetailMap = nglGetTexture( Material->DetailMapName );
      if ( !Material->DetailMap ) Material->DetailMap = nglWhiteTex;
    }

    if ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
    {
      if ( Load )
        Material->EnvironmentMap = nglLoadTexture( Material->EnvironmentMapName );
      else
        Material->EnvironmentMap = nglGetTexture( Material->EnvironmentMapName );
      if ( !Material->EnvironmentMap ) Material->EnvironmentMap = nglWhiteTex;
    }

    nglCalcMaterialHash( Material );
  }
}

// Note that this function can now be called from nglCreateMeshCopy.  So be careful what you do in here.
void nglProcessMesh( nglMesh* Mesh, u_int Version )
{
  u_int i;

  // Pre-invert bone matrices.
  for ( i = 0; i < Mesh->NBones; i++ )
    sceVu0InversMatrix( SCE_MATRIX( Mesh->Bones[i] ), SCE_MATRIX( Mesh->Bones[i] ) );

  for ( i = 0; i < Mesh->NSections; i++ )
  {
    // Process the material data.
    nglMeshSection* Section = &Mesh->Sections[i];
    nglMaterial* Material = Section->Material;

    Material->Flags &= ~NGLMAT_DETAIL_MAP;

#ifdef PROJECT_SPIDERMAN
    if ( Material->MapBlendMode != NGLBM_OPAQUE )
      Material->Flags &= ~NGLMAT_LIGHT;
    else
      Material->Flags |= NGLMAT_LIGHT;

    // hack cause backface culling is slow.
    if ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
      Material->Flags &= ~NGLMAT_BACKFACE_CULL;
#endif

    // Automatically attach textures used by the mesh.
		if (i==0)
			nglBindMeshTextures( Mesh, true );
		else
			nglBindMeshTextures( Mesh, false );
    // Flat shaded meshes (or meshes whose textures could not be loaded) automatically receive nglWhiteTex.
    if ( !( Material->Flags & NGL_TEXTURE_MASK ) )
    {
      Material->Flags |= NGLMAT_TEXTURE_MAP;
      Material->Map = nglWhiteTex;
    }

    Material->Info = (nglMaterialInfo*)nglMemAlloc( sizeof(nglMaterialInfo), 8 );
    void nglBakeMaterial( nglMesh* Mesh, nglMeshSection* Section, bool Scratch );
    nglBakeMaterial( Mesh, Section, false );
  }


  Mesh->Flags |= NGLMESH_PROCESSED;
}



nglMesh* nglGetMeshA( const char* Name, bool Warn /*= true*/ )
{
  nglFixedString p( Name );
  return nglGetMesh( p, Warn );
}

nglMesh* nglGetMesh( const nglFixedString& Name, bool Warn /*= true*/ )
{
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglMeshBank.Search( Name ) ) )
    return (nglMesh*)Inst->Value;
  if ( Warn )
    nglWarning( "NGL: Unable to find mesh %s.\n", Name.c_str() );
  return NULL;
}

nglMesh* nglGetFirstMeshInFile( const nglFixedString& FileName )
{
  nglInstanceBank::Instance* Inst;
  Inst = nglMeshFileBank.Search( FileName );
  if ( !Inst )
    return NULL;

  return ( (nglMeshFile*)Inst->Value )->FirstMesh;
}

nglMesh* nglGetNextMeshInFile( nglMesh* Mesh )
{
  if ( !Mesh )
    return NULL;

  return Mesh->NextMesh;
}

bool nglLoadMeshFileInternal( const nglFixedString& FileName, nglMeshFile* MeshFile )
{
  // Parse header.
  nglMeshFileHeader* Header = (nglMeshFileHeader*)MeshFile->FileBuf.Buf;
  if ( strncmp( (char*)( &Header->Tag ), "PS2M", 4 ) )
  {
    nglWarning( "NGL: Corrupted mesh file: %s%s.ps2mesh.\n", nglMeshPath, FileName.c_str() );
    return false;
  }

  if ( Header->Version != NGL_MESHFILE_VERSION && Header->Version != 0x606 )
  {
    nglWarning( "NGL: Unsupported mesh file version: %s%s.ps2mesh (version %x, current version is %x).\n", nglMeshPath, FileName.c_str(), Header->Version, NGL_MESHFILE_VERSION );
    return false;
  }

  if ( !Header->NMeshes )
  {
    nglWarning( "NGL: Mesh file contains no meshes: %s%s.ps2mesh.\n", nglMeshPath, FileName.c_str() );
    return false;
  }

  // Read the meshes starting from the beginning of the file.
  u_int* BufPos = (u_int*)( Header + 1 );
  MeshFile->FirstMesh = (nglMesh*)BufPos;

  nglMesh* Mesh;
  nglMesh* LastMesh = NULL;
  for ( u_int i = 0; i < Header->NMeshes; i++ )
  {
    Mesh = (nglMesh*)BufPos;

    // Search for an existing mesh with this name and skip it if we've already got one.
    nglInstanceBank::Instance* Inst;
    if ( ( Inst = nglMeshBank.Search( Mesh->Name ) ) )
    {
      nglMesh* ExistingMesh = (nglMesh*)Inst->Value;
      nglWarning( "NGL: Skipping duplicate mesh %s found in %s%s.ps2mesh.  Originally contained in %s%s.ps2mesh.\n",
        Mesh->Name.c_str(), nglMeshPath, FileName.c_str(), ExistingMesh->File->FilePath, ExistingMesh->File->FileName.c_str() );
      BufPos += Mesh->DataSize / sizeof(u_int);
      continue;
    }

    Mesh->File = MeshFile;

    // Convert pointers from file offsets.
    if ( !( Mesh->Flags & NGLMESH_PROCESSED ) )
      nglRebaseMesh( (u_int)Header, 0, Mesh );

    // Load textures, process materials, etc.
    nglProcessMesh( Mesh, Header->Version );

    // Link it into the list of meshes for the mesh file.
    if ( LastMesh )
      LastMesh->NextMesh = Mesh;
    LastMesh = Mesh;

    nglMeshBank.Insert( Mesh->Name, Mesh );
    BufPos += Mesh->DataSize / sizeof(u_int);
  }
  if ( LastMesh )
    LastMesh->NextMesh = NULL;
  else
    MeshFile->FirstMesh = NULL;

  nglPrintf( "nglLoadMeshFile: Loaded %s%s.\n", nglMeshPath, FileName.c_str() );
  return true;
}

bool nglLoadMeshFileInPlace( const nglFixedString& FileName, void* Data )
{
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglMeshFileBank.Search( FileName ) ) )
  {
    Inst->RefCount++;
    return true;
  }

  nglMeshFile* MeshFile = (nglMeshFile*)nglMemAlloc( sizeof(nglMeshFile) );
  strcpy( MeshFile->FilePath, "" );
  MeshFile->FileName = FileName;
  MeshFile->LoadedInPlace = true;
  MeshFile->FileBuf.Buf = (u_char*)Data;
  MeshFile->FileBuf.Size = 0;
  MeshFile->FileBuf.UserData = 0;

  bool Result = nglLoadMeshFileInternal( FileName, MeshFile );
  if ( !Result )
  {
    nglMemFree( MeshFile );
    return false;
  }

  nglMeshFileBank.Insert( FileName, MeshFile );
  return true;
}

bool nglLoadMeshFile( const nglFixedString& FileName )
{
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglMeshFileBank.Search( FileName ) ) )
  {
    Inst->RefCount++;
    return true;
  }

  strcpy( nglWork, nglMeshPath );
  strcat( nglWork, FileName.c_str() );
  strcat( nglWork, ".ps2mesh" );

  nglMeshFile* MeshFile = (nglMeshFile*)nglMemAlloc( sizeof(nglMeshFile) );
  strcpy( MeshFile->FilePath, nglMeshPath );
  MeshFile->FileName = FileName;
  MeshFile->LoadedInPlace = false;

  if ( !nglReadFile( nglWork, &MeshFile->FileBuf, 128 ) )
  {
    nglWarning( "nglLoadMeshFile: Unable to open %s%s.\n", nglMeshPath, FileName.c_str() );
    nglMemFree( MeshFile );
    return false;
  }

  bool Result = nglLoadMeshFileInternal( FileName, MeshFile );
  if ( !Result )
  {
    nglReleaseFile( &MeshFile->FileBuf );
    nglMemFree( MeshFile );
    return false;
  }

  nglMeshFileBank.Insert( FileName, MeshFile );
  return true;
}

void nglReleaseMeshFile( const nglFixedString& FileName )
{
  // Make sure they don't try and destroy it while it's being rendered from.
  nglVif1SafeWait();

  // Remove a reference from the instance bank, delete the mesh file only if the count hits 0.
  nglInstanceBank::Instance* Inst = nglMeshFileBank.Search( FileName );
  if ( !Inst )
    return;
  if ( --Inst->RefCount > 0 )
    return;

  nglMeshFile* MeshFile = (nglMeshFile*)Inst->Value;
  nglMeshFileBank.Delete( MeshFile->FileName );

  nglMesh* Mesh = MeshFile->FirstMesh;
  while ( Mesh )
  {
    for ( u_int i = 0; i < Mesh->NSections; i++ )
    {
      nglMaterial* Material = Mesh->Sections[i].Material;
      if ( Material->Flags & NGLMAT_TEXTURE_MAP )
        nglReleaseTexture( Material->Map );

      if ( Material->Flags & NGLMAT_LIGHT_MAP )
        nglReleaseTexture( Material->LightMap );

      if ( Material->Flags & NGLMAT_DETAIL_MAP )
        nglReleaseTexture( Material->DetailMap );

      if ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
        nglReleaseTexture( Material->EnvironmentMap );

      nglMemFree( Material->Info->BakedDMA );
      nglMemFree( Material->Info );
	  Material->Info = NULL;
    }

    nglMeshBank.Delete( Mesh->Name );
    Mesh = Mesh->NextMesh;
  }

  if ( !MeshFile->LoadedInPlace )
    nglReleaseFile( &MeshFile->FileBuf );

  nglMemFree( MeshFile );
}

void nglReleaseAllMeshFiles()
{
  nglInstanceBank::Instance* Inst = nglMeshFileBank.Head->Forward[0];
  while ( Inst != nglMeshFileBank.NIL )
  {
    if ( ( (nglMeshFile*)Inst->Value )->FileName == nglFixedString( "nglsphere" ) || ( (nglMeshFile*)Inst->Value )->FileName == nglFixedString( "nglradius" ) )
      Inst = Inst->Forward[0];
    else
    {
      Inst->RefCount = 1;
      nglReleaseMeshFile( ( (nglMeshFile*)Inst->Value )->FileName );
      Inst = nglMeshFileBank.Head->Forward[0];
    }
  }
}

int nglGetMaterialIdx( nglMesh* Mesh, u_int MaterialID )
{
  for ( u_int i = 0; i < Mesh->NSections; i++ )
    if ( Mesh->Sections[i].Material->MaterialID == MaterialID )
      return i;

  return -1;
}

enum
{
  NGLPASS_DIFFUSE,
  NGLPASS_DETAIL,
  NGLPASS_ENVIRONMENT,
  NGLPASS_LIGHT,
};

void nglVif1BakeMaterialPass( u_int*& Packet, u_int& CmdListDataPtr, nglMaterial* Material, u_int MaterialFlags, u_int Pass )
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
    Clamp = SCE_GS_SET_CLAMP( 0, MaterialFlags & NGLMAT_ENV_CYLINDER ? 0 : 1, 0, 0, 0, 0 );
    Map = Material->EnvironmentMap; MipRatio = Material->MapMipRatio;
    Mode = Material->EnvironmentMapBlendMode; Fix = Material->EnvironmentMapBlendModeConstant;
    break;
  case NGLPASS_LIGHT:
    Clamp = SCE_GS_SET_CLAMP( ( MaterialFlags & NGLMAT_LIGHT_CLAMP_U ) != 0, ( MaterialFlags & NGLMAT_LIGHT_CLAMP_V ) != 0, 0, 0, 0, 0 );
    Map = Material->LightMap; MipRatio = Material->LightMapMipRatio;
    Mode = Material->LightMapBlendMode; Fix = Material->LightMapBlendModeConstant;
    break;
  default:
    NGL_ASSERT(false, "Invalid pass number.");
    break;
  }

  nglVif1StartMaterialKick();

  // Blend mode comes first, because when NGL detects Alpha TINT on a mesh it has to replace the ALPHA and TEST registers
  // with those from NGLBM_BLEND.  Call it a hack, it is.
  nglVif1AddSetBlendMode( Mode, Fix, ( MaterialFlags & NGLMAT_FORCE_Z_WRITE ) ? true : false );

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
  nglVif1AddSetTexture( Map, MapFlags, MipRatio );

  // Last comes everything else.
  nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRIM( 0, 1, 1, 0, 1, ( MaterialFlags & NGLMAT_ANTIALIAS ) != 0, 0, 0, 0 ) );
  nglVif1AddGSReg( NGL_GS_CLAMP_1, Clamp );
  nglVif1EndMaterialKick( Packet, CmdListDataPtr );

  STOP_PROF_TIMER( proftimer_section_setup_material );
}

void nglBakeMaterial( nglMesh* Mesh, nglMeshSection* Section, bool Scratch )
{
  nglMaterial* Material = Section->Material;
  nglMaterialInfo* Info = Material->Info;

  memset( Info, 0, sizeof(nglMaterialInfo) );
  nglCalcMaterialHash( Material );

  u_int* Packet, *Start = 0;
  if ( Scratch )
  {
    nglScratchMeshPos = (u_int*)NGL_ALIGN( (u_int)nglScratchMeshPos, 128 );
    Start = Packet = (u_int*)nglScratchMeshPos;
  }
  else
    Packet = (u_int*)nglVif1PacketWork;

  nglDmaStartTag( Packet );

  // Batch DMA can reset the STCYCL register..
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 );
  Packet[1] = SCE_VIF1_SET_STMASK( 0 );
  Packet[2] = 0xF0F0F0F0; // 11-11-00-00 (WZYX)
  Packet += 3;

  // Build the command lists.
  static u_char CmdListOfs[NGLCMD_NUM_LISTS];
  u_int CmdListDataPtr = NGLMEM_COMMAND_LIST_START;

  // Skinning pass setup.
  if ( Mesh->Flags & NGLMESH_SKINNED )
  {
    CmdListOfs[NGLCMD_SKIN] = CmdListDataPtr;
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglSkinAddr );
    nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
  }
  else
    CmdListOfs[NGLCMD_SKIN] = 0;

  // Diffuse pass/basic transform and lighting.
  CmdListOfs[NGLCMD_DIFFUSE] = CmdListDataPtr;

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

  if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP )
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglTransformFrustumClipAddr );
  else
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglTransformClipAddr );

  // Support models with no diffuse texture.
  if ( Material->Flags & NGLMAT_TEXTURE_MAP )
  {
    Info->DiffuseKickAddr = CmdListDataPtr;
    nglVif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_DIFFUSE );
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
    if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP )
      nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
  }
  nglVif1AddCommandListEnd( Packet, CmdListDataPtr );

  // Detail map pass.
  if ( Material->Flags & NGLMAT_DETAIL_MAP )
  {
    CmdListOfs[NGLCMD_DETAIL] = CmdListDataPtr;
    Info->DetailKickAddr = CmdListDataPtr;
//    nglVif1AddDetailPassSetup( Packet, CmdListDataPtr, Material, Material->Flags );
    nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
  }
  else
    CmdListOfs[NGLCMD_DETAIL] = 0;

  // Environment map pass.
  if ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
  {
    CmdListOfs[NGLCMD_ENVIRONMENT] = CmdListDataPtr;
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglEnvironmentMapCylinderAddr );
    Info->EnvironmentKickAddr = CmdListDataPtr;
    nglVif1BakeMaterialPass( Packet, CmdListDataPtr, Material, Material->Flags, NGLPASS_ENVIRONMENT );
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
    if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP )
      nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
    nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
  }
  else
    CmdListOfs[NGLCMD_ENVIRONMENT] = 0;

  // Light map pass.
  if ( Material->Flags & NGLMAT_LIGHT_MAP )
  {
    CmdListOfs[NGLCMD_LIGHT] = CmdListDataPtr;
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglLightMapAddr );
    Info->LightKickAddr = CmdListDataPtr;
    nglVif1BakeMaterialPass( Packet, CmdListDataPtr, Section->Material, Material->Flags, NGLPASS_LIGHT );
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );
    if ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP )
      nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );
    nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
  }
  else
    CmdListOfs[NGLCMD_LIGHT] = 0;

  CmdListOfs[NGLCMD_PROJECTEDTEXTURES] = 0;

#if 0
  if ( Material->Flags & NGLMAT_FOG )
  {
    CmdListOfs[NGLCMD_FOG] = CmdListDataPtr;
    nglVif1AddFogSetup( Packet, CmdListDataPtr );
    nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
  }
  else
#endif
    CmdListOfs[NGLCMD_FOG] = 0;

  Info->CmdListEndAddr = CmdListDataPtr;

  // Unpack the array of command list pointers.
  nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS, SCE_VIF1_S_8, NGLCMD_NUM_LISTS, CmdListOfs, NGLCMD_NUM_LISTS );

  nglDmaEndTag( Packet, SCE_DMA_ID_RET );

  if ( Scratch )
  {
    Info->BakedDMA = Start;
    nglScratchMeshPos = Packet;
  }
  else
  {
    Info->BakedDMA = (u_int*)nglMemAlloc( (u_int)Packet - (u_int)nglVif1PacketWork, 128 );
    memcpy( Info->BakedDMA, nglVif1PacketWork, (u_int)Packet - (u_int)nglVif1PacketWork );
  }
}

// Requires and leaves Packet 16 byte aligned.
void nglVif1UnpackBones( u_int*& Packet, nglMesh* Mesh, u_int Flags, nglMatrix* Bones, nglMatrix* WorldToLocal /*= NULL*/ )
{
  if ( !STAGE_ENABLE( Skin ) )
    return;
  if ( Mesh->NBones > NGL_MAX_BONES )
    return;

  // Verify that exactly one flag is set.
  NGL_ASSERT( ( Flags & NGLP_BONES_MASK ) == NGLP_BONES_RELATIVE ||
              ( Flags & NGLP_BONES_MASK ) == NGLP_BONES_LOCAL ||
              ( Flags & NGLP_BONES_MASK ) == NGLP_BONES_WORLD,
              "Invalid bone flags in render params (only one bit can be set)." );

  nglVif1AddUnpackAligned( Packet, NGLMEM_END - Mesh->NBones * 4, Mesh->NBones == 64 ? 0 : Mesh->NBones * 4, SCE_VIF1_V4_32 );

  matrix_t* OutputBones = (matrix_t*)Packet;
  Packet += Mesh->NBones * sizeof(matrix_t) / sizeof(u_int);

  matrix_t* ParamBones = (matrix_t*)Bones;
  if ( Flags & NGLP_BONES_RELATIVE )
    memcpy( OutputBones, ParamBones, sizeof(matrix_t) * Mesh->NBones );
  else
  if ( Flags & NGLP_BONES_LOCAL )
  {
    matrix_t* MeshBones = (matrix_t*)Mesh->Bones;
    for ( u_int i = 0; i < Mesh->NBones; i++ )
      OutputBones[i] = ParamBones[i] * MeshBones[i];
  }
  else /*NGLP_BONES_WORLD*/
  {
    NGL_ASSERT( WorldToLocal != NULL, "NGLP_BONES_WORLD requires a WorldToLocal matrix." ); 
    matrix_t* MeshBones = (matrix_t*)Mesh->Bones;
    matrix_t WorldToLocalMat = *(matrix_t*)WorldToLocal;
    for ( u_int i = 0; i < Mesh->NBones; i++ )
      OutputBones[i] = WorldToLocalMat * ParamBones[i] * MeshBones[i];
  }
}

#if 0
void nglVif1AddAlphaFalloffSetup( u_int*& Packet, u_int& CmdListDataPtr, nglMeshNode* MeshNode, nglMaterial* Material )
{
  // Transform the View vector into local space.
  vector_t LocalViewDir = MeshNode->WorldToLocal * -nglCurScene->ViewDir;
  *(vec_xyzw*)&LocalViewDir = vec_w( Material->AlphaFalloff );

  nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglAlphaFalloffAddr );
  nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, &LocalViewDir, sizeof(nglVector) );
}

void nglVif1AddDetailPassSetup( u_int*& _Packet, u_int& _CmdListDataPtr, nglMeshNode* MeshNode, nglMaterial* Material, u_int MaterialFlags )
{
  u_int* Packet = _Packet;
  u_int CmdListDataPtr = _CmdListDataPtr;

  if ( !STAGE_ENABLE( DetailPass ) )
    return;

  nglVif1AddSetMaterial( Packet, CmdListDataPtr, Material, MaterialFlags, NGLPASS_DETAIL );

  // Calculate range in Z buffer space, and then scalar to take that into an appropriate alpha value.
  sceVu0FVECTOR Range = { 0, 0, Material->DetailMapRange, 1 };
  sceVu0ApplyMatrix( Range, nglCurScene->ViewToScreen, Range );
  float ZScale = 128.0f / ( nglCurScene->ZMax - Range[2] );

  // Run the detail map program and kick.
  nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDetailMapAddr );
  nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &ZScale, sizeof(float) );
  nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &Range[2], sizeof(float) );
  nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V2_32, 1, &Material->DetailMapUScale, sizeof(float) * 2 );

  if ( STAGE_ENABLE( Kick ) )
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );

  // Kick extra verts for plane clipping if neccessary.
  if ( MeshNode->Clip && ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP ) && STAGE_ENABLE( PlaneClip ) )
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );

  _Packet = Packet;
  _CmdListDataPtr = CmdListDataPtr;
}

void nglVif1AddFogSetup( u_int*& _Packet, u_int& _CmdListDataPtr, nglMeshNode* MeshNode )
{
  u_int* Packet = _Packet;
  u_int CmdListDataPtr = _CmdListDataPtr;

  point_t ViewPos( 0, 0, 0 );
  ViewPos = *(transform_t*)nglCurScene->ViewToWorld * ViewPos;

  nglMesh* Mesh = MeshNode->Mesh;
  point_t WorldPos = *(point_t*)Mesh->SphereCenter;
  WorldPos = MeshNode->LocalToWorld * WorldPos;

  // If the scene's fog min value is zero (no fog) and the mesh is entirely within the near range of the fog, skip this pass.
  float DistSqr = WorldPos.distance_sqr_from( ViewPos );
  if ( nglCurScene->FogMin == 0.0f && DistSqr + Mesh->SphereRadius < nglCurScene->FogNear * nglCurScene->FogNear )
    return;

  nglVif1StartMaterialKick();
  nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRIM( 0, 1, 0, 0, 1, 0, 0, 0, 0 ) );
  nglVif1AddSetBlendMode( NGLBM_BLEND, 0 );
  nglVif1EndMaterialKick( Packet, CmdListDataPtr );

  // If the mesh is past the far range of the fog, use a fast function.
  if ( DistSqr - Mesh->SphereRadius > nglCurScene->FogFar * nglCurScene->FogFar )
  {
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglMatColorAddr );
    u_int Color = ( nglFTOI( nglCurScene->FogColor[0] ) ) | ( nglFTOI( nglCurScene->FogColor[1] ) << 8 ) | ( nglFTOI( nglCurScene->FogColor[2] ) << 16 ) | ( nglFTOI( nglCurScene->FogMax * 255.0f ) << 24 );
    nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_8, 1, &Color, sizeof(u_int) );
  }
  else
  {
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFogAddr );

    // Transform the View Direction vector into local space.
    point_t LocalViewPos = MeshNode->WorldToLocal * nglCurScene->ViewPos;
    nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, &LocalViewPos, sizeof(float) * 3 );
  }

  if ( STAGE_ENABLE( Kick ) )
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );

  // Kick extra verts for plane clipping if neccessary.
  if ( MeshNode->Clip && ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP ) && STAGE_ENABLE( PlaneClip ) )
    nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );

  _Packet = Packet;
  _CmdListDataPtr = CmdListDataPtr;
}
#endif

void nglVif1AddTextureLightSetup( u_int*& _Packet, u_int& _CmdListDataPtr, nglMaterial* Material, nglMesh* Mesh, nglMeshNode* MeshNode, u_int MaterialFlags )
{
  u_int* Packet = _Packet;
  u_int CmdListDataPtr = _CmdListDataPtr;

  if ( !STAGE_ENABLE( ProjectedTexturePass ) )
    return;

  nglVector WorldPos;
  sceVu0ApplyMatrix( WorldPos, SCE_MATRIX(MeshNode->LocalToWorld), Mesh->SphereCenter );

  for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
  {
    // If the mesh matches this category, run through the context looking for lights that affect it.
    if ( !( Mesh->Flags & ( 1 << ( NGL_LIGHTCAT_SHIFT + i ) ) ) )
      continue;

    nglLightNode* LightNode = nglCurLightContext->TextureHead.LocalNext[i];
    while ( LightNode != &nglCurLightContext->TextureHead )
    {
      if ( LightNode->Type == NGLLIGHT_PROJECTED_DIRECTIONAL )
      {
        nglDirProjectorLightInfo* Light = (nglDirProjectorLightInfo*)LightNode->NodeData;
        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglProjectedTextureAddr );

        nglMatrix LocalToUV;
        sceVu0MulMatrix( SCE_MATRIX(LocalToUV), SCE_MATRIX(Light->WorldToUV), SCE_MATRIX(MeshNode->LocalToWorld) );

        nglVector LocalLightDir; // = { 0, -1, 0, 0 };
        sceVu0ApplyMatrix( LocalLightDir, SCE_MATRIX(MeshNode->WorldToLocal), Light->Zaxis );

#ifdef PROJECT_SPIDERMAN
        LocalLightDir[3] = Light->FadeScale;
        nglVector LocalPos, WorldPos;
        WorldPos[0] = Light->Pos[0] + Light->PosCullOffset[0];
        WorldPos[1] = Light->Pos[1] + Light->PosCullOffset[1];
        WorldPos[2] = Light->Pos[2] + Light->PosCullOffset[2];
        WorldPos[3] = 1.0f;
        sceVu0ApplyMatrix( LocalPos, SCE_MATRIX(MeshNode->WorldToLocal), WorldPos );
#endif

        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, LocalLightDir, sizeof(nglVector) );
#ifdef PROJECT_SPIDERMAN
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, LocalPos, sizeof(nglVector) );
#endif
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 4, LocalToUV, sizeof(nglMatrix) );

        nglVif1StartMaterialKick();
        nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRMODE( 1, 1, 0, 1, 0, 0, 0, 0 ) );
        nglVif1AddSetBlendMode( Light->BlendMode, Light->BlendModeConstant, false );
        nglVif1AddSetTexture( Light->Tex, 0, 0.0f );
        nglVif1AddGSReg( NGL_GS_CLAMP_1, SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0 ) );
        nglVif1EndMaterialKick( Packet, CmdListDataPtr );

        if ( STAGE_ENABLE( Kick ) )
          nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );

        // Kick extra verts for plane clipping if neccessary.
        if ( MeshNode->Clip && ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP ) && STAGE_ENABLE( PlaneClip ) )
          nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );

        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDummyKickAddr );
        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDummyKickAddr );
      }
      else
      if ( LightNode->Type == NGLLIGHT_PROJECTED_POINT )
      {
        nglPointProjectorLightInfo* Light = (nglPointProjectorLightInfo*)LightNode->NodeData;

        nglVector LocalPos;
        sceVu0ApplyMatrix( LocalPos, SCE_MATRIX(MeshNode->WorldToLocal), Light->Pos );
        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglPointProjectedTextureAddr );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, LocalPos, sizeof(nglVector) );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Light->Color, sizeof(nglVector) );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &Light->Range, sizeof(float) );

        nglVif1StartMaterialKick();
        nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRMODE( 1, 1, 0, 1, 0, 0, 0, 0 ) );
        nglVif1AddSetBlendMode( Light->BlendMode, Light->BlendModeConstant, (MaterialFlags & NGLMAT_FORCE_Z_WRITE) ? true : false );
        nglVif1AddSetTexture( Light->Tex, 0, 0.0f );
        nglVif1AddGSReg( NGL_GS_CLAMP_1, SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 ) );
        nglVif1EndMaterialKick( Packet, CmdListDataPtr );

        if ( STAGE_ENABLE( Kick ) )
          nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglKickAddr );

        // Kick extra verts for plane clipping if neccessary.
        if ( MeshNode->Clip && ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP ) && STAGE_ENABLE( PlaneClip ) )
          nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglFrustumClipAddr );

        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDummyKickAddr );
        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDummyKickAddr );
      }

      LightNode = LightNode->LocalNext[i];
    }
  }

  _Packet = Packet;
  _CmdListDataPtr = CmdListDataPtr;
}

// These are purposefully not inlined.
void nglBreakAssembly();

inline void nglVif1AddTextureFixup( u_int*& _Packet, nglTexture* Tex, u_int GSRegAddr, float MipRatio )
{
//  START_PROF_TIMER( proftimer_section_texture_fixup );
  register u_int* Packet = _Packet;
  register int RegCnt = 0;

  if ( Tex->Type == NGLTEX_IFL || Tex->Type == NGLTEX_ATE )
    Tex = Tex->Frames[nglTextureAnimFrame % Tex->NFrames];

  if ( (u_int)Packet & 1 ) *(Packet++) = SCE_VIF1_SET_NOP( 0 );
  Packet[0] = SCE_VIF1_SET_NOP( 0 );
//  Packet[1] = SCE_VIF1_SET_UNPACK( ... ); to be filled in afterwards

  // Load 4-bit textures specially.
  register u_long* RegPtr = (u_long*)( Packet + 2 );
  register u_long GsTex0 = Tex->GsTex0;
  if ( ( (sceGsTex0*)&Tex->GsTex0 )->PSM == SCE_GS_PSMT4 )
  {
    // load it as an 8 bit texture
    GsTex0 &= ~( GS_TEX0_PSM_M | GS_TEX0_CLD_M );
    GsTex0 |= ( SCE_GS_PSMT8 << GS_TEX0_PSM_O ) | ( 1L << GS_TEX0_CLD_O );
    *(RegPtr++) = GsTex0;
    RegCnt++;

    // then treat it as a 4 bit texture
    GsTex0 &= ~( GS_TEX0_PSM_M | GS_TEX0_CLD_M );
    GsTex0 |= ( SCE_GS_PSMT4 << GS_TEX0_PSM_O );
  }

  *(RegPtr++) = GsTex0;
  RegCnt++;

#if !NGL_DISABLE_MIPMAPS
  if ( Tex->MipMapTextures > 1 && MipRatio && !DEBUG_ENABLE( DisableMipmaps ) )
  {
    RegPtr[0] = Tex->GsMipTBP1;
    RegPtr[1] = Tex->GsMipTBP2;
    RegPtr += 2;
    RegCnt += 2;

    int K = -Tex->MipMapFirstLod - (nglLog2( nglMipBias * nglCurScene->ScrZ * MipRatio * Tex->InvHypot ) << nglTexL);
    ( (sceGsTex1*)&Tex->GsTex1 )->K = (K&255)<<4;
    *RegPtr++ = Tex->GsTex1;
  }
  else
#endif
	{
		// Add only the magnification/minifaction options (no mipmap settings).
		u_long GsTex1 = Tex->GsTex1;
		GsTex1 &= ( GS_TEX1_MMAG_M | GS_TEX1_MMIN_M );
    *RegPtr++ = GsTex1;
	}
  ++RegCnt;

  // No need to change TEXA.  Write the UNPACK register.
  Packet[1] = SCE_VIF1_SET_UNPACK( GSRegAddr + 4, RegCnt, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 0 );
  Packet = (u_int*)RegPtr;

  _Packet = Packet;
//  STOP_PROF_TIMER( proftimer_section_texture_fixup );
}

void nglVif1RenderBakedMeshSection( u_int*& _Packet, void* Param )
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

  if ( DEBUG_ENABLE( DumpFrameLog ) & 2 )
    nglPrintf( "LOG: Adding section %s(%d) of %s (%d).\n",
      MaterialFlags & NGLMAT_TEXTURE_MAP ? Material->Map->FileName.c_str() : "",
      Section - Mesh->Sections,
      Mesh->Name.c_str(),
      nglNodeCount );

  if ( nglLastVUCodeDma != NGL_VUCODE_MESH )
  {
    nglDmaStartTag( Packet );
    nglDmaEndTag( Packet, SCE_DMA_ID_CALL, nglLoadMicrocode );
    nglLastVUCodeDma = NGL_VUCODE_MESH;
  }

  nglDmaAddCall( Packet, Info->BakedDMA );
  nglDmaStartTag( Packet );

  // Unpack the bones and the matrix.
  if ( MeshNode->Params.Flags & NGLP_BONES_MASK )
    nglVif1UnpackBones( Packet, Mesh, MeshNode->Params.Flags, MeshNode->Params.Bones, (nglMatrix*)&MeshNode->WorldToLocal );

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

  // Stream the textures.
  if ( MaterialFlags & NGLMAT_TEXTURE_MAP )
    Textures[NTextures++] = Material->Map;
  if ( MaterialFlags & NGLMAT_DETAIL_MAP )
    Textures[NTextures++] = Material->DetailMap;
  if ( MaterialFlags & NGLMAT_ENVIRONMENT_MAP )
    Textures[NTextures++] = Material->EnvironmentMap;
  if ( MaterialFlags & NGLMAT_LIGHT_MAP )
    Textures[NTextures++] = Material->LightMap;
  nglVif1AddTextureStreaming( Packet, Textures, NTextures );

  // Fixup the GS registers in VU mem to match the textures' actual loaded addresses.
  if ( MaterialFlags & NGLMAT_TEXTURE_MAP )
    nglVif1AddTextureFixup( Packet, Material->Map, Info->DiffuseKickAddr, Material->MapMipRatio );
  if ( MaterialFlags & NGLMAT_DETAIL_MAP )
    nglVif1AddTextureFixup( Packet, Material->DetailMap, Info->DetailKickAddr, Material->MapMipRatio );
  if ( MaterialFlags & NGLMAT_ENVIRONMENT_MAP )
    nglVif1AddTextureFixup( Packet, Material->EnvironmentMap, Info->EnvironmentKickAddr, Material->MapMipRatio );
  if ( MaterialFlags & NGLMAT_LIGHT_MAP )
    nglVif1AddTextureFixup( Packet, Material->LightMap, Info->LightKickAddr, Material->LightMapMipRatio );

  // Lighting (build vertex and texture light command lists).
  if ( MaterialFlags & NGLMAT_LIGHT )
  {
    u_int CmdListDataPtr = Info->CmdListEndAddr;
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

    if ( !nglCurLightContext->LocalTextureListEmpty )
    {
      nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_PROJECTEDTEXTURES, SCE_VIF1_S_32, 1, &CmdListDataPtr, sizeof(u_int) );
      nglVif1AddTextureLightSetup( Packet, CmdListDataPtr, Material, Mesh, MeshNode, MaterialFlags );
      nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
    }
    else
    {
      u_int Val = 0;
      nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_PROJECTEDTEXTURES, SCE_VIF1_S_32, 1, &Val, sizeof(u_int) );
    }
  }

  // Set the diffuse pass flags.
  register u_int Flags = 0;
  if ( MeshNode->Clip )
  {
    Flags |= NGLVU_CLIP;
    if ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP )
      Flags |= NGLVU_CLIP_PERFECT;
  }
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
      }
    }
  }

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

void nglVif1RenderSimpleBakedMeshSection( u_int*& _Packet, void* Param )
{
  nglMeshSectionNode* Node = (nglMeshSectionNode*)Param;
  register nglMeshNode* MeshNode = Node->MeshNode;
  register nglMesh* Mesh = MeshNode->Mesh;
  register nglMeshSection* Section = Node->Section;
  register nglMaterial* Material = Section->Material;
  register u_int MaterialFlags = Material->Flags;
  register nglMaterialInfo* Info = Material->Info;
  register u_int* Packet = _Packet;

  START_PROF_TIMER( proftimer_send_simple_sections );

  if ( DEBUG_ENABLE( DumpFrameLog ) & 2 )
    nglPrintf( "LOG: Adding section %s(%d) of %s (%d).\n",
      MaterialFlags & NGLMAT_TEXTURE_MAP ? Material->Map->FileName.c_str() : "",
      Section - Mesh->Sections,
      Mesh->Name.c_str(),
      nglNodeCount );

  if ( nglLastVUCodeDma != NGL_VUCODE_MESH )
  {
    nglDmaStartTag( Packet );
    nglDmaEndTag( Packet, SCE_DMA_ID_CALL, nglLoadMicrocode );
    nglLastVUCodeDma = NGL_VUCODE_MESH;
  }

  nglDmaAddCall( Packet, Info->BakedDMA );
  nglDmaStartTag( Packet );

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

  // Stream the textures.
  Textures[NTextures++] = Material->Map;
  nglVif1AddTextureStreaming( Packet, Textures, NTextures );

  // Fixup the GS registers in VU mem to match the textures actual loaded address.
  nglVif1AddTextureFixup( Packet, Material->Map, Info->DiffuseKickAddr, Material->MapMipRatio );

  // Vertex lighting.
  if ( MaterialFlags & NGLMAT_LIGHT )
  {
    u_int CmdListDataPtr = Info->CmdListEndAddr;
    if ( !nglCurLightContext->LocalVertexListEmpty )
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

    if ( !nglCurLightContext->LocalTextureListEmpty )
    {
      nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_PROJECTEDTEXTURES, SCE_VIF1_S_32, 1, &CmdListDataPtr, sizeof(u_int) );
      nglVif1AddTextureLightSetup( Packet, CmdListDataPtr, Material, Mesh, MeshNode, MaterialFlags );
      nglVif1AddCommandListEnd( Packet, CmdListDataPtr );
    }
    else
    {
      u_int Val = 0;
      nglVif1AddUnpack( Packet, NGLMEM_COMMAND_LIST_ADDRS + NGLCMD_PROJECTEDTEXTURES, SCE_VIF1_S_32, 1, &Val, sizeof(u_int) );
    }
  }

  // Set the diffuse pass flags.
  register u_int Flags = 0;
  if ( MeshNode->Clip )
  {
    Flags |= NGLVU_CLIP;
    if ( MeshNode->Mesh->Flags & NGLMESH_PERFECT_TRICLIP )
      Flags |= NGLVU_CLIP_PERFECT;
  }
  if ( MaterialFlags & NGLMAT_BACKFACE_CULL )
    Flags |= NGLVU_BACKFACE;

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
  STOP_PROF_TIMER( proftimer_send_simple_sections );
}

// Builds a list of lights affecting the current mesh into the Context's Local Lights List.  If Context is NULL, uses nglGlobalLightContext.
// Also fills Textures with the set of textures that will need streaming as a result of projector lights, and increments NTextures for each one.
void nglDetermineLocalLights( nglLightContext* Context, nglMesh* Mesh, nglMatrix& LocalToWorld, nglTexture** Textures, u_int& NTextures )
{
  START_PROF_TIMER( proftimer_section_lighting );

  if ( Context )
    nglCurLightContext = Context;
  else
    nglCurLightContext = nglGlobalLightContext;

  nglCurLightContext->LocalVertexListEmpty = true;
  nglCurLightContext->LocalTextureListEmpty = true;

  // Find the mesh's center in world space.
  nglVector WorldPos;
  sceVu0ApplyMatrix( WorldPos, SCE_MATRIX( LocalToWorld ), Mesh->SphereCenter );

  // I think there's a bug here involving adding the light to the local lights more than once if it matches on
  // more than one lighting category.
  for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
  {
    // Empty the context's local lights list.
    nglCurLightContext->VertexHead.LocalNext[i] = &nglCurLightContext->VertexHead;
    nglCurLightContext->TextureHead.LocalNext[i] = &nglCurLightContext->TextureHead;

    // If the mesh matches this category, run through the context looking for lights that affect it.
    if ( !( Mesh->Flags & ( 1 << ( NGL_LIGHTCAT_SHIFT + i ) ) ) )
      continue;

    // Check against the list of normal lights.
    nglLightNode* LightNode = nglCurLightContext->VertexHead.Next[i];
    while ( LightNode != &nglCurLightContext->VertexHead )
    {
      bool Success = false;
      if ( LightNode->Type == NGLLIGHT_TRUEPOINT || LightNode->Type == NGLLIGHT_FAKEPOINT )
      {
        nglPointLightInfo* Light = (nglPointLightInfo*)LightNode->NodeData;
        if ( nglSpheresIntersect( Light->Pos, Light->Far, WorldPos, 0.0f ) )
          Success = true;
      }
      else
      if ( LightNode->Type == NGLLIGHT_DIRECTIONAL )
        Success = true;

      // If the light affects the mesh, add it to the list.
      if ( Success )
      {
        LightNode->LocalNext[i] = nglCurLightContext->VertexHead.LocalNext[i];
        nglCurLightContext->VertexHead.LocalNext[i] = LightNode;
        nglCurLightContext->LocalVertexListEmpty = false;
      }

      LightNode = LightNode->Next[i];
    }

    // Check against the list of projector lights.
    LightNode = nglCurLightContext->TextureHead.Next[i];
    while ( LightNode != &nglCurLightContext->TextureHead )
    {
      bool Success = false;
      if ( LightNode->Type == NGLLIGHT_PROJECTED_POINT )
      {
        nglPointProjectorLightInfo* Light = (nglPointProjectorLightInfo*)LightNode->NodeData;
        if ( nglSpheresIntersect( Light->Pos, Light->Range, WorldPos, Mesh->SphereRadius ) )
        {
          Textures[NTextures++] = Light->Tex;
          Success = true;
        }
      }
      else
      if ( LightNode->Type == NGLLIGHT_PROJECTED_DIRECTIONAL )
      {
        nglDirProjectorLightInfo* Light = (nglDirProjectorLightInfo*)LightNode->NodeData;
        if ( nglIsSphereVisible( &Light->Frustum, WorldPos, Mesh->SphereRadius ) )
        {
          Textures[NTextures++] = Light->Tex;
          Success = true;
        }
      }

      // If the light affects the mesh, add it to the list.
      if ( Success )
      {
        LightNode->LocalNext[i] = nglCurLightContext->TextureHead.LocalNext[i];
        nglCurLightContext->TextureHead.LocalNext[i] = LightNode;
        nglCurLightContext->LocalTextureListEmpty = false;
      }

      // Break if we've run out of texture slots.
      if ( NTextures == NGL_MAX_MATERIAL_TEXTURES )
        break;

      LightNode = LightNode->Next[i];
    }
  }
  STOP_PROF_TIMER( proftimer_section_lighting );
}

void nglVif1AddVertexLightSetup( u_int*& _Packet, u_int& _CmdListDataPtr, nglMeshSection* Section, nglMesh* Mesh, nglMeshNode* MeshNode )
{
  START_PROF_TIMER( proftimer_section_lighting );
  u_int* Packet = _Packet;
  u_int CmdListDataPtr = _CmdListDataPtr;

  if ( DEBUG_ENABLE( DumpFrameLog ) & 16 )
    nglPrintf( "NGL: Lighting for %s material %d(%s) LightCat %x:\n",
    Mesh->Name.c_str(), Section - Mesh->Sections, Section->Material->MapName.c_str(), Mesh->Flags >> NGL_LIGHTCAT_SHIFT );

  for ( int i = 0; i < NGL_NUM_LIGHTCATS; i++ )
  {
    // If the mesh matches this category, run through the context looking for lights that affect it.
    if ( !( Mesh->Flags & ( 1 << ( NGL_LIGHTCAT_SHIFT + i ) ) ) )
      continue;

    nglLightNode* LightNode = nglCurLightContext->VertexHead.LocalNext[i];
    while ( LightNode != &nglCurLightContext->VertexHead )
    {
      if ( LightNode->Type == NGLLIGHT_TRUEPOINT )
      {
        nglPointLightInfo* Light = (nglPointLightInfo*)LightNode->NodeData;

        if ( DEBUG_ENABLE( DumpFrameLog ) & 16 )
          nglPrintf( "TruePoint (%x) at (%.2f,%.2f,%.2f) Color (%.2f,%.2f,%.2f,%.2f) Range %.2f->%.2f.\n",
            LightNode->LightCat >> NGL_LIGHTCAT_SHIFT,
            Light->Pos[0], Light->Pos[1], Light->Pos[2],
            Light->Color[0], Light->Color[1], Light->Color[2], Light->Color[3],
            Light->Near, Light->Far );

        // Transform light to local space.
        nglVector LightPos;
        sceVu0CopyVector( LightPos, Light->Pos );
        LightPos[3] = 1.0f;
        sceVu0ApplyMatrix( LightPos, SCE_MATRIX(MeshNode->WorldToLocal), LightPos );

        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglPointLightAddr );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, LightPos, sizeof(float) * 3 );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &Light->Near, sizeof(float) );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_S_32, 1, &Light->Far, sizeof(float) );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Light->Color, sizeof(nglVector) );
      }
      else
      if ( LightNode->Type == NGLLIGHT_FAKEPOINT )
      {
        nglPointLightInfo* Light = (nglPointLightInfo*)LightNode->NodeData;

        // Transform light to local space.
        nglVector LightPos;
        sceVu0CopyVector( LightPos, Light->Pos );
        LightPos[3] = 1.0f;
        sceVu0ApplyMatrix( LightPos, SCE_MATRIX(MeshNode->WorldToLocal), LightPos );

        nglVector V;
        float Range, Scale;

        // This is a duplication of the math that happens per-vertex in nglPointLightAddr.
        sceVu0SubVector( V, Mesh->SphereCenter, LightPos );
        Range = sqrtf( V[0] * V[0] + V[1] * V[1] + V[2] * V[2] );
        sceVu0ScaleVector( V, V, 1.0f / Range );
        V[3] = 0.0f;

        if ( Light->Far <= Light->Near )
          Scale = 1.0f;
        else
        {
          Scale = 1.0f - ( Range - Light->Near ) / ( Light->Far - Light->Near );
          if ( Scale < 0.0f ) Scale = 0.0f;
          if ( Scale > 1.0f ) Scale = 1.0f;
        }

        if ( DEBUG_ENABLE( DumpFrameLog ) & 16 )
          nglPrintf( "FakePoint (%x) at (%.2f,%.2f,%.2f) Color (%.2f,%.2f,%.2f,%.2f) Range %.2f->%.2f. Scale =%.2f.\n",
            LightNode->LightCat >> NGL_LIGHTCAT_SHIFT,
            Light->Pos[0], Light->Pos[1], Light->Pos[2],
            Light->Color[0], Light->Color[1], Light->Color[2], Light->Color[3],
            Light->Near, Light->Far, Scale );

        nglVector Color;
        sceVu0ScaleVector( Color, Light->Color, Scale );

        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDirLightAddr );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, V, sizeof(float) * 3 );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Color, sizeof(nglVector) );

        // Add specular highlights.
        if ( Section->Material->Flags & NGLMAT_ENV_SPECULAR )
        {
          nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglSpecularAddr );

          // Transform the View Direction vector into local space.
          vector_t LocalViewDir = (transform_t&)MeshNode->WorldToLocal * nglCurScene->ViewDir;
          vector_t HalfVector = ( -LocalViewDir + -*(vector_t*)&V ).normalize();
          nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, &HalfVector, sizeof(float) * 3 );

          float Intensity = sqrtf( Color[0] * Color[0] + Color[1] * Color[1] + Color[2] * Color[2] );
          nglVector Specular( Section->Material->SpecularIntensity * Intensity, 0, 0, 0 );
          nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Specular, sizeof(nglVector) );
        }
      }
      else
      if ( LightNode->Type == NGLLIGHT_DIRECTIONAL )
      {
        nglDirLightInfo* Light = (nglDirLightInfo*)LightNode->NodeData;

        if ( DEBUG_ENABLE( DumpFrameLog ) & 16 )
          nglPrintf( "Directional (%x) (%.2f,%.2f,%.2f) Color (%.2f,%.2f,%.2f,%.2f).\n",
            LightNode->LightCat >> NGL_LIGHTCAT_SHIFT,
            Light->Dir[0], Light->Dir[1], Light->Dir[2],
            Light->Color[0], Light->Color[1], Light->Color[2], Light->Color[3] );

        nglVector LocalDir;
        sceVu0ApplyMatrix( LocalDir, SCE_MATRIX(MeshNode->WorldToLocal), Light->Dir );

        nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglDirLightAddr );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, LocalDir, sizeof(float) * 3 );
        nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Light->Color, sizeof(nglVector) );

        // Add specular highlights.
        if ( Section->Material->Flags & NGLMAT_ENV_SPECULAR )
        {
          nglVif1AddCommandListProgram( Packet, CmdListDataPtr, nglSpecularAddr );

          // Transform the View Direction vector into local space.
          vector_t LocalViewDir = (transform_t&)MeshNode->WorldToLocal * nglCurScene->ViewDir;
          vector_t HalfVector = ( -LocalViewDir + -*(vector_t*)&LocalDir ).normalize();
          nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V3_32, 1, &HalfVector, sizeof(float) * 3 );

          float Intensity = sqrtf( Light->Color[0] * Light->Color[0] + Light->Color[1] * Light->Color[1] + Light->Color[2] * Light->Color[2] );
          nglVector Specular(Section->Material->SpecularIntensity * Intensity * 255.0f, 0, 0, 0 );
          nglVif1AddCommandListData( Packet, CmdListDataPtr, SCE_VIF1_V4_32, 1, Specular, sizeof(nglVector) );
        }
      }

      LightNode = LightNode->LocalNext[i];
    }
  }

  _Packet = Packet;
  _CmdListDataPtr = CmdListDataPtr;
  STOP_PROF_TIMER( proftimer_section_lighting );
}

// This function exists to add an arbitrary function call to optimized ASM code, it's useful for splitting up sections
// that you want to analyze seperately.
void nglBreakAssembly()
{
}

nglCustomNodeFn nglGetMeshSectionFunction( u_int MaterialFlags, u_int ParamFlags )
{
  if (    !( MaterialFlags & ( NGLMAT_DETAIL_MAP | NGLMAT_ENVIRONMENT_MAP | NGLMAT_LIGHT_MAP | NGLMAT_ALPHA_FALLOFF | NGLMAT_UV_SCROLL | NGLMAT_MATERIAL_COLOR ) )
      &&  !( ParamFlags & ( NGLP_TINT | NGLP_BONES_MASK | NGLP_TEXTURE_FRAME | NGLP_TEXTURE_SCROLL | NGLP_ZBIAS | NGLP_NO_LIGHTING ) ) )
    return nglVif1RenderSimpleBakedMeshSection;
  else
    return nglVif1RenderBakedMeshSection;
}
                    
inline void nglDetermineMipLevel(nglTexture *Tex, float MipRatio, float w)
{
  //Commented out lines are being handled by caller

  //if (!Tex || !MipRatio) return;
  //if (Tex->MipMapTextures < 2) return;
  int Lod;
  if (Tex->MipMapInitCount != nglListInitCount)
  {
    Tex->MipMapInitCount = nglListInitCount;
    Lod = Tex->MipMapTextures-1;
    Tex->MipMapFirstLod = Lod;
    Tex->GsSize = Tex->GsImage[Lod].Size;
  }
  //else if (Tex->MipMapFirstLod==0) return;
  else
    Lod = Tex->MipMapFirstLod;
  int FirstLod;
  if (w>0.0f)
  {
    FirstLod = ( nglLog2(w) - nglLog2( nglMipBias * nglCurScene->ScrZ * MipRatio * Tex->InvHypot ) ) << nglTexL;
    if (FirstLod < 0) FirstLod = 0;
    if (FirstLod >= Lod) return;
  }
  else FirstLod = 0;
  Tex->MipMapFirstLod = FirstLod;
  Tex->GsSize = Tex->GsImage[FirstLod].Size;
}

void nglSetSectionRenderer( nglMeshSection* Section, nglCustomNodeFn RenderFn )
{
	Section->RenderFn = RenderFn;
}

void nglListAddMesh( nglMesh* Mesh, const nglMatrix& LocalToWorld, nglRenderParams* Params )
{
  // Check for NULL pointers, commonly encountered when file loads fail.
  if ( !Mesh )
    return;

//  if ( !nglEvalTestNode() )
//    return;

  if ( DEBUG_ENABLE( DisableScratch ) && ( Mesh->Flags & NGLMESH_SCRATCH_MESH ) )
    return;

  START_PROF_TIMER( proftimer_render_add_mesh );

  if ( DEBUG_ENABLE( DumpFrameLog ) & 4 )
    nglPrintf( "nglListAddMesh: Added mesh %s at (%f,%f,%f).\n",
      Mesh->Name.c_str(),
      LocalToWorld[3][0], LocalToWorld[3][1], LocalToWorld[3][2] );

  if ( DEBUG_ENABLE( DumpSceneFile ) )
    nglSceneDumpMesh( Mesh, LocalToWorld, Params );

  START_PROF_TIMER( proftimer_addmesh_culling );

  register u_int ParamFlags = Params ? Params->Flags : 0;
  register u_int MeshFlags = Mesh->Flags;

  // Update the mesh radius for scale.
  register float Radius = Mesh->SphereRadius;

  matrix_t ScaledLocalToWorld;

  float ScaleMag = 1.0f;
  if ( ParamFlags & NGLP_SCALE )
  {
    ScaleMag = Params->Scale[0];
    if ( Params->Scale[1] > ScaleMag )
      ScaleMag = Params->Scale[1];
    if ( Params->Scale[2] > ScaleMag )
      ScaleMag = Params->Scale[2];
    Radius *= ScaleMag;

    vector_t Scale = *(vector_t*)&Params->Scale;
    ScaledLocalToWorld.set_col0( *(vec_xyzw*)&LocalToWorld[0] * vec_x( Scale ) );
    ScaledLocalToWorld.set_col1( *(vec_xyzw*)&LocalToWorld[1] * vec_y( Scale )  );
    ScaledLocalToWorld.set_col2( *(vec_xyzw*)&LocalToWorld[2] * vec_z( Scale )  );
    ScaledLocalToWorld.set_col3( *(vec_xyzw*)&LocalToWorld[3] );
  }
  else
  {
//    ScaledLocalToWorld = *(transform_t*)&LocalToWorld;
    ScaledLocalToWorld.set_col0( ( *(vec_xyzw*)&LocalToWorld[0] ).normalize() );
    ScaledLocalToWorld.set_col1( ( *(vec_xyzw*)&LocalToWorld[1] ).normalize() );
    ScaledLocalToWorld.set_col2( ( *(vec_xyzw*)&LocalToWorld[2] ).normalize() );
    ScaledLocalToWorld.set_col3( *(vec_xyzw*)&LocalToWorld[3] );
  }

  // Transform sphere center into camera space and find the distance to the closest point for clipping logic.
  vec_xyzw Center = ScaledLocalToWorld * *(vec_xyzw*)&Mesh->SphereCenter;      // View space center of the bounding sphere.

  // LOD support.  Very slow implementation, but it's okay since we don't use any yet.;)
  if ( MeshFlags & NGLMESH_LOD )
  {
    for ( u_int i = 0; i < Mesh->NLODs; i++ )
      if ( ((nglVector*)&Center)->z > Mesh->LODs[i].Range )
        Mesh = nglGetMesh( Mesh->LODs[i].Name );
  }

  // Hack to allow us to turn off clipping for ortho transform, since clipping doesn't
  // yet work for this case.  Remove when fixed (dc 09/30/01)
  int Clip = 0;
  if ( nglCurScene->UsingOrtho )
    Clip = 0;
  else
  {
    if ( Params && ( Params->Flags & NGLP_NO_CULLING ) )
      Clip = nglGetClipResultGBOnly( *(nglVector*)&Center, Radius );
    else
      Clip = nglGetClipResult( *(nglVector*)&Center, Radius );
  }

  if ( Clip == -1 )
  {
    if ( DEBUG_ENABLE( DumpFrameLog ) & 4 )
      nglPrintf( "nglListAddMesh: Mesh rejected by clipping.\n" );
    STOP_PROF_TIMER( proftimer_addmesh_culling );
    STOP_PROF_TIMER( proftimer_render_add_mesh );
    return;
  }

  // If sphere rejection is on and the sphere touches the plane, reject the whole mesh.
  if ( Clip && ( MeshFlags & NGLMESH_REJECT_SPHERE ) )
  {
    if ( DEBUG_ENABLE( DumpFrameLog ) & 4 )
      nglPrintf( "nglListAddMesh: Mesh rejected by NGLMESH_REJECT_SPHERE." );
    STOP_PROF_TIMER( proftimer_addmesh_culling );
    STOP_PROF_TIMER( proftimer_render_add_mesh );
    return;
  }

  // Draw a debugging sphere.  Turns red if the mesh is clipped.
  if ( DEBUG_ENABLE( DrawMeshSpheres ) && Mesh != nglRadiusMesh && nglRadiusMesh )
    nglDrawDebugSphere( *(nglVector*)&Center, Radius, Clip ? NGL_RGBA32( 0xFF, 0, 0, 0xFF ) : NGL_RGBA32( 0xFF, 0xFF, 0xFF, 0xFF ), false );

  STOP_PROF_TIMER( proftimer_addmesh_culling );
  START_PROF_TIMER( proftimer_addmesh_meshnode );

  // Attempt to add a new mesh node to the mesh list.  Allocates all the sub-structures simultaneously.
  u_char* Buf = (u_char*)nglListAlloc( sizeof(nglMeshNode) + ( sizeof(nglMeshSectionNode) + sizeof(nglListNode) ) * Mesh->NSections );
  if ( !Buf )
  {
    STOP_PROF_TIMER( proftimer_addmesh_meshnode );
    STOP_PROF_TIMER( proftimer_render_add_mesh );
    return;
  }

  nglMeshNode* MeshNode = (nglMeshNode*)Buf;
  Buf += sizeof(nglMeshNode);
  MeshNode->Mesh = Mesh;
  *(matrix_t*)&MeshNode->LocalToWorld = ScaledLocalToWorld;
  *(matrix_t*)&MeshNode->WorldToLocal = *(matrix_t*)&LocalToWorld;
  sceVu0InversMatrix( SCE_MATRIX( MeshNode->WorldToLocal ), SCE_MATRIX( MeshNode->WorldToLocal ) );

  if ( Params )
  {
	  if ( ( ParamFlags & NGLP_BONES_MASK ) && !Params->Bones )
	  {
		  Params->Bones = (nglMatrix*)nglListAlloc(sizeof(nglMatrix) * Params->NBones);
		  if (!Params->Bones) return;
	  }
    MeshNode->Params = *Params;
  }
  MeshNode->Params.Flags = ParamFlags;
  MeshNode->Clip = Clip;

  STOP_PROF_TIMER( proftimer_addmesh_meshnode );
  START_PROF_TIMER( proftimer_addmesh_listnode );

  // Create a new node for each material.
  for ( u_int i = 0; i < Mesh->NSections; i++ )
  {
    // Skip materials that should be masked off.
    if ( MeshNode->Params.Flags & NGLP_MATERIAL_MASK )
      if ( ( 1 << i ) & MeshNode->Params.MaterialMask )
        continue;

    nglMeshSectionNode* SectionNode = (nglMeshSectionNode*)Buf;
    Buf += sizeof(nglMeshSectionNode);
    SectionNode->MeshNode = MeshNode;

    nglMeshSection* Section = &Mesh->Sections[i];
    SectionNode->Section = Section;

    nglMaterial* Material = Section->Material;
    SectionNode->MaterialFlags = Material->Flags;

    if ( ParamFlags & NGLP_TINT )
    {
      if ( MeshNode->Params.TintColor[3] != 1.0f )
        SectionNode->MaterialFlags |= NGLMAT_ALPHA;
    }

#if 0
    // If the detail map is out of range, turn it off.
    if ( Material->DetailMap && NearDist > Material->DetailMapRange )
      SectionNode->MaterialFlags &= ~NGLMAT_DETAIL_MAP;
#endif

    bool DetermineMap;
    bool DetermineLightMap;
    if (DEBUG_ENABLE( DisableMipOpt ))
    {
      if (Material->Map && Material->Map->MipMapTextures>1)
      {
        Material->Map->MipMapFirstLod = 0;
        Material->Map->GsSize = Material->Map->GsImage[0].Size;
      }
      if (Material->LightMap && Material->LightMap->MipMapTextures>1)
      {
        Material->LightMap->MipMapFirstLod = 0;
        Material->LightMap->GsSize = Material->LightMap->GsImage[0].Size;
      }
      DetermineMap = false;
      DetermineLightMap = false;
    }
    else
    {
      DetermineMap = Material->Map && Material->MapMipRatio && Material->Map->MipMapTextures>1 &&
                     (Material->Map->MipMapInitCount!=nglListInitCount || Material->Map->MipMapFirstLod);
      DetermineLightMap = Material->LightMap && Material->LightMapMipRatio && Material->LightMap->MipMapTextures>1 &&
                     (Material->LightMap->MipMapInitCount!=nglListInitCount || Material->LightMap->MipMapFirstLod);
    }

    vec_xyzw SectionCenter;
    float SectionRadius;

    if ( DetermineMap || DetermineLightMap || SectionNode->MaterialFlags & NGLMAT_ALPHA )
    {
      // SectionCenter is view space center of the bounding sphere, SectionRadius is its radius.
	    SectionCenter = ScaledLocalToWorld * *(vec_xyzw*)&Section->SphereCenter;
	    SectionCenter = *(matrix_t*)&nglCurScene->WorldToView * SectionCenter;
	    SectionRadius = Section->SphereRadius * ScaleMag;
    }

    // Build the info structure and add the node.
    nglSortInfo SortInfo;
    if ( SectionNode->MaterialFlags & NGLMAT_ALPHA )
    {
	    float Dist = SectionRadius + (float)vec_z( SectionCenter );
      //float NearDist = -SectionRadius + (float)vec_z( SectionCenter );

      SortInfo.Type = NGLSORT_TRANSLUCENT;

      // If the sort distance is overridden by the material, use it.
      if ( Material->Flags & NGLMAT_ALPHA_SORT_FIRST )
        SortInfo.Dist = Material->ForcedSortDistance;
      else
        SortInfo.Dist = Dist;
    }
    else
    {
      SortInfo.Type = NGLSORT_TEXTURED;
      SortInfo.Hash = Material->Info->Hash;
    }

    if (DetermineMap || DetermineLightMap)
    {
      // Change SectionCenter to be near point in screen space for calculation of NearW
      SectionCenter -= vec_z(SectionRadius);
      SectionCenter = *(matrix_t*)&nglCurScene->ViewToScreen * SectionCenter;
      float NearW = (float)vec_w(SectionCenter);
      if (DetermineMap)
        nglDetermineMipLevel(Material->Map, Material->MapMipRatio, NearW);
      if (DetermineLightMap)
        nglDetermineMipLevel(Material->LightMap, Material->LightMapMipRatio, NearW);
    }

    nglCustomNodeFn NodeFn;
		if ( Section->RenderFn )
			NodeFn = Section->RenderFn;
		else
			NodeFn = nglGetMeshSectionFunction( SectionNode->MaterialFlags, ParamFlags );
    nglListAddNode( NGLNODE_MESH_SECTION, NodeFn, SectionNode, &SortInfo, &Buf );
  }
  STOP_PROF_TIMER( proftimer_addmesh_listnode );
  STOP_PROF_TIMER( proftimer_render_add_mesh );
}

//----------------------------------------------------------------------------------------
//  @Quad (2d interface) code.
//----------------------------------------------------------------------------------------
void nglVif1StartQuads( u_int*& Packet )
{
  if ( DEBUG_ENABLE( DisableQuads ) )
    return;

  START_PROF_TIMER( proftimer_send_quads );

  nglVif1FlushSPAD( Packet, true );
  nglDmaStartTag( Packet );

//  nglVif1ResetGSRegisterCache();
}

void nglVif1EndQuads( u_int*& Packet )
{
  if ( DEBUG_ENABLE( DisableQuads ) )
    return;

  nglDmaEndTag( Packet, SCE_DMA_ID_CNT );
  nglVif1FlushSPAD( Packet, true );

  STOP_PROF_TIMER( proftimer_send_quads );
}

void nglVif1AddQuadMaterial( nglQuad* Quad )
{
  if ( Quad->Tex )
    nglVif1AddSetTexture( Quad->Tex, Quad->MapFlags, 0.0f );
  nglVif1AddSetBlendModeCheckZTest( Quad->BlendMode, Quad->BlendModeConstant );
  nglVif1AddGSReg( NGL_GS_CLAMP_1, 
	  SCE_GS_SET_CLAMP( ( Quad->MapFlags & NGLMAP_CLAMP_U ) != 0, ( Quad->MapFlags & NGLMAP_CLAMP_V ) != 0, 0, 0, 0, 0 ) );
  nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRMODE( 1, Quad->Tex != 0, 0, Quad->BlendMode != NGLBM_OPAQUE, 0, 0, 0, 0 ) );
}

void nglVif1AddQuadVerts( nglQuad* Quad )
{
  u_int z = nglViewZToGS( Quad->Z );

  nglVif1AddRawGSReg( SCE_GS_PRIM, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 0, 0, 0, 0, 0, 0, 0, 0 ) );
  for ( int j = 0; j < 4; j++ )
  {
    nglQuadVertex* Vert = &Quad->Verts[j];
    nglVif1AddRawGSReg( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( Vert->Color, 0, 0, 0, 0x3f800000 ) );  // Q == 1.0f.
    nglVif1AddRawGSReg( SCE_GS_ST, SCE_GS_SET_ST( *(u_int*)&Vert->U, *(u_int*)&Vert->V ) );
    u_int x = nglScreenXToGS( Vert->X );
    u_int y = nglScreenYToGS( Vert->Y );
    nglVif1AddRawGSReg( SCE_GS_XYZ2, SCE_GS_SET_XYZ( x, y, z ) );
  }
}

void nglVif1RenderQuad( u_int*& Packet, nglQuad* Quad )
{
  if ( DEBUG_ENABLE( DisableQuads ) )
    return;

  // Guard band culling.
  for ( int j = 0; j < 4; j++ )
  {
    nglQuadVertex* Vert = &Quad->Verts[j];
    int x = nglFTOI( ( Vert->X + 2048 - ( nglCurScene->RenderTarget->Width / 2 ) ) * 16 );
    int y = nglFTOI( ( Vert->Y + 2048 - ( nglCurScene->RenderTarget->Height / 2 ) ) * 16 );
    if ( x < 0 || x > 4095 * 16 || y < 0 || y > 4095 * 16 )
      return;
  }

  if ( Quad->Tex )
    nglVif1AddSingleTextureStreaming( Packet, Quad->Tex );

  nglVif1StartDirectGifAD();

  nglVif1AddQuadMaterial( Quad );
  nglVif1AddQuadVerts( Quad );

  nglVif1EndDirectGifAD( Packet );
}

void nglVif1RenderSingleQuad( u_int*& Packet, void* Param )
{
  nglQuad* Quad = (nglQuad*)Param;
  nglVif1StartQuads( Packet );
  nglVif1RenderQuad( Packet, Quad );
  nglVif1EndQuads( Packet );
  nglVif1FlushSPAD( Packet );
}

void nglListAddQuad( nglQuad* Quad )
{
  nglQuad* Entry = nglListNew( nglQuad );
  if ( !Entry )
    return;
  memcpy( Entry, Quad, sizeof(nglQuad) );

  nglSortInfo SortInfo;
  nglGetSortInfo( &SortInfo, Entry->BlendMode, Entry->Tex, Entry->Z );
  nglListAddNode( NGLNODE_QUAD, nglVif1RenderSingleQuad, Entry, &SortInfo );
}

void nglInitQuad( nglQuad* Quad )
{
  memset( Quad, 0, sizeof(nglQuad) );

  Quad->Verts[0].Color = NGL_RGBA32( 0xFF, 0xFF, 0xFF, 0xFF );
  Quad->Verts[1].Color = NGL_RGBA32( 0xFF, 0xFF, 0xFF, 0xFF );
  Quad->Verts[2].Color = NGL_RGBA32( 0xFF, 0xFF, 0xFF, 0xFF );
  Quad->Verts[3].Color = NGL_RGBA32( 0xFF, 0xFF, 0xFF, 0xFF );

  Quad->Verts[0].U = 0.0f;
  Quad->Verts[1].U = 1.0f;
  Quad->Verts[2].U = 0.0f;
  Quad->Verts[3].U = 1.0f;

  Quad->Verts[0].V = 0.0f;
  Quad->Verts[1].V = 0.0f;
  Quad->Verts[2].V = 1.0f;
  Quad->Verts[3].V = 1.0f;

  Quad->MapFlags = NGLMAP_BILINEAR_FILTER | NGLMAT_CLAMP_U | NGLMAT_CLAMP_V;
  Quad->BlendMode = NGLBM_BLEND;
}

void nglSetQuadTex( nglQuad* Quad, nglTexture* Tex )
{
  Quad->Tex = Tex;
}

void nglSetQuadMapFlags( nglQuad* Quad, u_int MapFlags )
{
  Quad->MapFlags = MapFlags;
}

void nglSetQuadBlend( nglQuad* Quad, u_int Blend, u_int Constant )
{
  Quad->BlendMode = Blend;
  Quad->BlendModeConstant = Constant;
}

// Simplified API.
void nglSetQuadUV( nglQuad* Quad, float u1, float v1, float u2, float v2 )
{
  Quad->Verts[0].U = u1;
  Quad->Verts[0].V = v1;
  Quad->Verts[1].U = u2;
  Quad->Verts[1].V = v1;
  Quad->Verts[2].U = u1;
  Quad->Verts[2].V = v2;
  Quad->Verts[3].U = u2;
  Quad->Verts[3].V = v2;
}

void nglSetQuadColor( nglQuad* Quad, u_int c )
{
  Quad->Verts[0].Color = c;
  Quad->Verts[1].Color = c;
  Quad->Verts[2].Color = c;
  Quad->Verts[3].Color = c;
}

void nglSetQuadPos( nglQuad* Quad, float x, float y )
{
  float w, h;
  if ( Quad->Tex )
  {
    w = Quad->Tex->Width;
    h = Quad->Tex->Height;
  }
  else
    w = h = 50;

  nglSetQuadRect( Quad, x, y, x + w, y + h );
}

void nglSetQuadRect( nglQuad* Quad, float x1, float y1, float x2, float y2 )
{
  Quad->Verts[0].X = x1;
  Quad->Verts[0].Y = y1;
  Quad->Verts[1].X = x2;
  Quad->Verts[1].Y = y1;
  Quad->Verts[2].X = x1;
  Quad->Verts[2].Y = y2;
  Quad->Verts[3].X = x2;
  Quad->Verts[3].Y = y2;
}

void nglSetQuadZ( nglQuad* Quad, float z )
{
  Quad->Z = z;
}

// New vertex API.  Verts go in this order: Top Left, Top Right, Bottom Left, Bottom Right.
void nglSetQuadVPos( nglQuad* Quad, int VertIdx, float x, float y )
{
  Quad->Verts[VertIdx].X = x;
  Quad->Verts[VertIdx].Y = y;
}

void nglSetQuadVUV( nglQuad* Quad, int VertIdx, float u, float v )
{
  Quad->Verts[VertIdx].U = u;
  Quad->Verts[VertIdx].V = v;
}

void nglSetQuadVColor( nglQuad* Quad, int VertIdx, u_int Color )
{
  Quad->Verts[VertIdx].Color = Color;
}

void nglRotateQuad( nglQuad* Quad, float cx, float cy, float theta )
{
  for ( int i = 0; i < 4; i++ )
  {
    nglQuadVertex* Vert = &Quad->Verts[i];
    float x = Vert->X - cx;
    float y = Vert->Y - cy;
    Vert->X = x * cosf( theta ) - y * sinf( theta ) + cx;
    Vert->Y = y * cosf( theta ) + x * sinf( theta ) + cy;
  }
}

void nglScaleQuad( nglQuad* Quad, float cx, float cy, float sx, float sy )
{
  for ( int i = 0; i < 4; i++ )
  {
    nglQuadVertex* Vert = &Quad->Verts[i];
    float x = Vert->X - cx;
    float y = Vert->Y - cy;
    Vert->X = x * sx + cx;
    Vert->Y = y * sy + cy;
  }
}

/*---------------------------------------------------------------------------------------------------------
  @Font API.

  Mostly taken from Sean Palmer's font system.
---------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Description: Return the integer value following the specified token.
             For example, if data contains: "posx 320 posy 200 color 128",
             the call nglGetTokenUINT(data, "posy", 10) will returns 200.
-----------------------------------------------------------------------------*/
u_int nglGetTokenUINT(char*& Data, const char* Token, u_int Base)
{
  u_int TokenLength = strlen(Token);

  for (;;)
  {
    u_int CharCount = 0;

    // Locate the beginning of the token.
    for (u_int i = 0; i < TokenLength; i++)
    {
      if (Data[i] != Token[i])
        break;
      else
        CharCount++;
    }

    // Token found. Read the string and convert it to an integer value.
    if (CharCount == TokenLength)
    {
      // Skip the token we just recognized.
      Data += TokenLength;
      // String to integer conversion.
      char *StopString;
      u_int Val = strtol(Data, &StopString, Base);
      Data = StopString;
      return Val;
    }
    else
    {
      Data++;
    }
  }
}

/*-----------------------------------------------------------------------------
Description: Return the integer value pointed by data.
-----------------------------------------------------------------------------*/
u_int nglGetUINT(char*& Data, u_int Base)
{
  // Move to the first digit.
  while (*Data < '0' || *Data > '9') Data++;

  // String to integer conversion.
  char *StopString;
  u_int Val = strtol(Data, &StopString, Base);
  Data = StopString;
  return Val;
}

/*-----------------------------------------------------------------------------
Description: Parse the FDF file and initialize the font header and the font's
             glyphs.
-----------------------------------------------------------------------------*/
void nglParseFDF(char* Data, nglFont* Font)
{
  Font->Header.Version = nglGetTokenUINT(Data, "version", 16);
  NGL_ASSERT(Font->Header.Version == NGLFONT_VERSION, "Unsupported font version !");

  Font->Header.CellHeight = nglGetTokenUINT(Data, "cellheight", 10);
  NGL_ASSERT(Font->Header.CellHeight, "");

  Font->Header.Ascent = nglGetTokenUINT(Data, "ascent", 10);
  NGL_ASSERT(Font->Header.Ascent, "");

  Font->Header.FirstGlyph = nglGetTokenUINT(Data, "firstglyph", 10);
  NGL_ASSERT(Font->Header.FirstGlyph, "");

  Font->Header.NumGlyphs = nglGetTokenUINT(Data, "numglyphs", 10);
  NGL_ASSERT(Font->Header.NumGlyphs > 0, "");

  Font->GlyphInfo = (nglGlyphInfo*)nglMemAlloc(sizeof(nglGlyphInfo) * Font->Header.NumGlyphs);

  for (u_int i = 0; i < Font->Header.NumGlyphs; i++)
  {
    u_int gidx = nglGetUINT(Data, 10);
    NGL_ASSERT(gidx == i + Font->Header.FirstGlyph, "Character out of sequence in FDF file.");

    nglGlyphInfo& gi = Font->GlyphInfo[gidx - Font->Header.FirstGlyph];
    gi.TexOfs[0] = nglGetTokenUINT(Data, "x", 10);
    gi.TexOfs[1] = nglGetTokenUINT(Data, "y", 10);
    gi.CellWidth = nglGetTokenUINT(Data, "w", 10);
    NGL_ASSERT(gi.CellWidth, "");
    gi.GlyphOrigin[0] = nglGetTokenUINT(Data, "gx", 10);
    gi.GlyphOrigin[1] = nglGetTokenUINT(Data, "gy", 10);
    gi.GlyphSize[0] = nglGetTokenUINT(Data, "gw", 10);
    gi.GlyphSize[1] = nglGetTokenUINT(Data, "gh", 10);
  }
}

nglFont* nglGetFont( const nglFixedString& FileName )
{
  nglInstanceBank::Instance* Inst;
  if ( ( Inst = nglFontBank.Search( FileName ) ) )
    return (nglFont*)Inst->Value;
  return 0;
}

/*-----------------------------------------------------------------------------
Description: Load a font (from a .fdf file) and return a pointer to it.
             The texture font must be loaded before calling this function.
-----------------------------------------------------------------------------*/
nglFont* nglLoadFont(const nglFixedString& FontName)
{
  // Search for an existing copy.
  nglInstanceBank::Instance* Inst;
  if ((Inst = nglFontBank.Search(FontName)))
  {
    Inst->RefCount++;
    return (nglFont*)Inst->Value;
  }

  // The font's texture must be loaded before calling this function !
  // Get the font's texture. If it fails, the function returns.
  nglFont* Font = (nglFont*)nglMemAlloc(sizeof(nglFont));
  memset(Font, 0, sizeof(nglFont));

  Font->Tex = nglGetTexture(FontName);

  if (!Font->Tex)
  {
    nglFatal("nglLoadFont(): failed to get the texture %s !\n", FontName.c_str());
    return NULL;
  }

  Font->MapFlags = NGLMAP_BILINEAR_FILTER;
  Font->BlendMode = NGLBM_BLEND;

  // Load the font's description file (fdf). If it fails, the function returns.
  // Look into the texture path.
  char FileName[NGL_MAX_PATH_LENGTH];
  strcpy(FileName, nglTexturePath);
  strcat(FileName, FontName.c_str());
  strcat(FileName, ".fdf");

  nglFileBuf FileBuf;
  if (!nglReadFile(FileName, &FileBuf, 0))
    return NULL;

  // Parse the fdf file.
  char* Data = (char*)FileBuf.Buf;

  nglParseFDF(Data, Font);

  // Release the fdf file.
  nglReleaseFile(&FileBuf);

  nglFontBank.Insert(Font->Tex->FileName, Font);

  return Font;
}

/*-----------------------------------------------------------------------------
Description: Load a font from memory (must point to a fdf file) and return a
             pointer to the loaded font.
             The texture font must be loaded before calling this function.
-----------------------------------------------------------------------------*/
nglFont* nglLoadFontInPlace(const nglFixedString& FontName, void* FDFdata)
{
  // Search for an existing copy.
  nglInstanceBank::Instance* Inst;
  if ((Inst = nglFontBank.Search(FontName)))
  {
    Inst->RefCount++;
    return (nglFont*)Inst->Value;
  }

  // The font's texture must be loaded before calling this function !
  // Get the font's texture. If it fails, the function returns.
  nglFont* Font = (nglFont*)nglMemAlloc(sizeof(nglFont));
  memset(Font, 0, sizeof(nglFont));

  Font->Tex = nglGetTexture(FontName);

  if (!Font->Tex)
  {
    nglFatal("nglLoadFont(): failed to get the texture %s !\n", FontName.c_str());
    return NULL;
  }

  Font->MapFlags = NGLMAP_BILINEAR_FILTER;
  Font->BlendMode = NGLBM_BLEND;

  // Parse the fdf file.
  nglParseFDF((char*)FDFdata, Font);

  nglFontBank.Insert(Font->Tex->FileName, Font);

  return Font;
}

/*-----------------------------------------------------------------------------
Description: Release a font from memory.
-----------------------------------------------------------------------------*/
void nglReleaseFont(nglFont* Font)
{
  NGL_ASSERT(Font, "Trying to release a NULL font !");

	// Remove a reference from the instance bank, delete the font only if the count hits 0.
  if (nglFontBank.Delete(Font->Tex->FileName))
		return;

  // Release the glyph infos and the font.
  nglMemFree(Font->GlyphInfo);
  nglMemFree(Font);
  Font = NULL;
}

/*-----------------------------------------------------------------------------
Description: Release all the fonts from memory.
-----------------------------------------------------------------------------*/
void nglReleaseAllFonts()
{
  nglInstanceBank::Instance* Inst = nglFontBank.Head->Forward[0];

  while ( Inst != nglFontBank.NIL )
  {
    if ( ( (nglFont*)Inst->Value )->System )
      Inst = Inst->Forward[0];
    else
    {
      Inst->RefCount = 1;
      nglReleaseFont( (nglFont*)Inst->Value );
      Inst = nglFontBank.Head->Forward[0];
    }
  }
}

/*-----------------------------------------------------------------------------
Description: Setup the font's mesures.
-----------------------------------------------------------------------------*/
void nglSetFontMeasures(nglFont* Font, float* aoffs, float* asize, float* auvpos,
                        float* auvsize,  float scalex, float scaley, const nglGlyphInfo& ginfo)
{
  float InvWidth  = 1.0f / (float)Font->Tex->Width;
  float InvHeight = 1.0f / (float)Font->Tex->Height;
  aoffs[0]   = int(ginfo.GlyphOrigin[0]) * scalex;
  aoffs[1]   = int(ginfo.GlyphOrigin[1]) * scaley;
  asize[0]   = u_int(ginfo.GlyphSize[0]) * scalex;
  asize[1]   = u_int(ginfo.GlyphSize[1]) * scaley;
  auvpos[0]  = u_int(ginfo.TexOfs[0]) * InvWidth;
  auvpos[1]  = u_int(ginfo.TexOfs[1]) * InvHeight;
  auvsize[0] = u_int(ginfo.GlyphSize[0]) * InvWidth;
  auvsize[1] = u_int(ginfo.GlyphSize[1]) * InvHeight;
}

struct nglStringNode
{
  char* Text;
  nglFont* Font;
  float X, Y, Z;
  u_int Color;
};

void nglRenderSingleCharacter( u_long*& RegPtr, float* pos, float* size, float* uvpos, float* uvsize )
{
  float x_ratio = 1.0f, y_ratio = 1.0f;

  float left    = (pos[0]) * x_ratio,
        top     = (pos[1]) * y_ratio,
        right   = (pos[0] + size[0]) * x_ratio,
        bottom  = (pos[1] + size[1]) * y_ratio;

  float uvright = uvpos[0] + uvsize[0];
  float uvbottom = uvpos[1] + uvsize[1];

  u_int Z = nglViewZToGS( pos[2] );
  RegPtr[0] = SCE_GS_SET_ST( (u_int&)uvpos[0], (u_int&)uvpos[1] );
  RegPtr[1] = SCE_GS_SET_XYZ( nglScreenXToGS( left ), nglScreenYToGS( top ), Z );
  RegPtr[2] = SCE_GS_SET_ST( (u_int&)uvright, (u_int&)uvbottom );
  RegPtr[3] = SCE_GS_SET_XYZ( nglScreenXToGS( right ), nglScreenYToGS( bottom ), Z );
  RegPtr += 4;
}

#define NGLFONT_TOKEN_COLOR    '\1'
#define NGLFONT_TOKEN_SCALE    '\2'
#define NGLFONT_TOKEN_SCALEXY  '\3'

void nglVif1RenderString( u_int*& Packet, void* Param )
{
  nglStringNode* Node = (nglStringNode*)Param;

  nglFont* Font = Node->Font;

  float curpos[3] = { Node->X, Node->Y, Node->Z };
  float offs[2];
  float size[2];
  float uvpos[2];
  float uvsize[2];

  const char* TextPtr = Node->Text;
  char  c;

  u_int Color = Node->Color;

  float ScaleX = 1.0f;
  float ScaleY = 1.0f;
  float CurMaxScaleY = ScaleY;

  nglDmaStartTag( Packet );

  nglVif1AddTextureStreaming( Packet, &Font->Tex, 1 );

  nglVif1StartDirectGifAD();
  nglVif1AddSetTexture( Font->Tex, Font->MapFlags, 0.0f );
  nglVif1AddSetBlendModeCheckZTest( NGLBM_BLEND, 0 );
  nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRMODE( 0, 1, 0, 1, 0, 0, 0, 0 ) );
  nglVif1AddRawGSReg( SCE_GS_PRIM, SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
  nglVif1AddRawGSReg( SCE_GS_RGBAQ, Color | ( 0x3f800000L << 32 ) );  // Q = 1.0f
  nglVif1EndDirectGifAD( Packet );

  u_int* DirectTag = nglVif1StartDirect( Packet );
  u_long* GifTag = nglGifStartTag( Packet );

  u_long* RegPtr = (u_long*)Packet;
  while ((c = *TextPtr))
  {
    // Check for color token: \1[RRGGBBAA] or \1[0xRRGGBBAA]
    if (c == NGLFONT_TOKEN_COLOR)
    {
      TextPtr += 2;
      // String to integer (hexa) conversion.
      char *StopString;
      Color = strtoul(TextPtr, &StopString, 16);
      Color = NGL_RGBA32( ( Color >> 24 ) & 0xFF, ( Color >> 16 ) & 0xFF, ( Color >> 8 ) & 0xFF, Color & 0xFF );
      TextPtr = StopString + 1;
      c = *TextPtr;

      // stop writing chars, write the RGBAQ register.
      Packet = (u_int*)RegPtr;
      u_int NLoop = nglGifCalcNLoop( Packet, GifTag, SCE_GIF_REGLIST, 4 );
      if ( NLoop )
        nglGifEndTag( Packet, GifTag, SCE_GIF_SET_TAG( NLoop, 0, 0, 0, SCE_GIF_REGLIST, 4 ), 0x5252 );
      else
        Packet = (u_int*)GifTag;

      GifTag = nglGifStartTag( Packet );
      *(u_long*)Packet = Color | ( 0x3f800000L << 32 ); // Q = 1.0f
      Packet += 2;
      nglGifEndTag( Packet, GifTag, SCE_GIF_SET_TAG( 1, 0, 0, 0, SCE_GIF_REGLIST, 1 ), 0x1 );

      GifTag = nglGifStartTag( Packet );
      RegPtr = (u_long*)Packet;

	  continue;
    }
    // Check for scale token: \2[SCALE]
    else if (c == NGLFONT_TOKEN_SCALE)
    {
      TextPtr += 2;
      // String to float conversion.
      char *StopString;
      float OldScaleY = ScaleY;
      ScaleX = ScaleY = (float)strtod(TextPtr, &StopString);
      TextPtr = StopString + 1;
      c = *TextPtr;

      // Store the current max scale (needed to go to the next line).
      if (ScaleY > OldScaleY)
        CurMaxScaleY = ScaleY;
      else
        CurMaxScaleY = OldScaleY;

	  continue;
    }
		// Check for independent scale token: \3[SCALEX,SCALEY]
		else if (c == NGLFONT_TOKEN_SCALEXY)
		{
			TextPtr += 2;
			// String to float conversion (SCALEX).
			char *StopString;
			ScaleX = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;
			
			// String to float conversion (SCALEY).
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;
			
			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;
			
			continue;
		}

    if (c == '\n')
    {
      curpos[0] = Node->X;
      curpos[1] += Font->Header.CellHeight * CurMaxScaleY;
      CurMaxScaleY = ScaleY;
    }
    else
    {
      const nglGlyphInfo& ginfo = Font->GlyphInfo[(unsigned char)c - Font->Header.FirstGlyph];

      if (c != ' ')
      {
        nglSetFontMeasures( Font, offs, size, uvpos, uvsize, ScaleX, ScaleY, ginfo );
        float gpos[3] = { curpos[0] + offs[0], curpos[1] + offs[1], curpos[2] };
        nglRenderSingleCharacter( RegPtr, gpos, size, uvpos, uvsize );
      }

      curpos[0] += ginfo.CellWidth * ScaleX;
    }

    TextPtr++;
  }

  Packet = (u_int*)RegPtr;
  u_int NLoop = nglGifCalcNLoop( Packet, GifTag, SCE_GIF_REGLIST, 4 );
  if ( NLoop )
    nglGifEndTag( Packet, GifTag, SCE_GIF_SET_TAG( NLoop, 1, 0, 0, SCE_GIF_REGLIST, 4 ), 0x5252 );
  else
    Packet = (u_int*)GifTag;
  nglVif1EndDirect( Packet, DirectTag );

  nglDmaEndTag( Packet, SCE_DMA_ID_CNT );
  nglVif1FlushSPAD( Packet );
}

void nglSetFontMapFlags(nglFont *Font, u_int MapFlags)
{
	Font->MapFlags = MapFlags;
}

u_int nglGetFontMapFlags(nglFont *Font)
{
	return Font->MapFlags;
}

// Set the font's blend mode.
void nglSetFontBlend(nglFont* Font, u_int BlendMode, u_int Constant)
{
	Font->BlendMode = BlendMode;
	Font->BlendModeConstant = Constant;
}

void nglListAddString( nglFont* Font, float x, float y, double z, const char* Text, ... )
{
  va_list argptr;
  va_start( argptr, Text );
  vsprintf( nglWork, Text, argptr );
  va_end( argptr );

  nglListAddString( Font, x, y, z, NGL_RGBA32( 0xFF, 0xFF, 0xFF, 0xFF ), nglWork );
}

void nglListAddString( nglFont* Font, float x, float y, double z, u_int Color, const char* Text, ... )
{
  if (Text == NULL || *Text == '\0' || Font == NULL )
    return;

  nglStringNode* StringNode = nglListNew( nglStringNode );
  if ( !StringNode )
    return;

  va_list argptr;
  va_start( argptr, Text );
  vsprintf( nglWork, Text, argptr );
  va_end( argptr );

  u_int Len = strlen( nglWork );
  StringNode->Text = (char*)nglListAlloc( Len + 1 );
  memcpy( StringNode->Text, nglWork, Len + 1 );

  StringNode->Font = Font;
  StringNode->Color = Color;
  StringNode->X = x;
  StringNode->Y = y;
  StringNode->Z = z;

  nglSortInfo SortInfo;
  SortInfo.Type = NGLSORT_TRANSLUCENT;
  SortInfo.Dist = z;
  nglListAddNode( NGLNODE_STRING, nglVif1RenderString, StringNode, &SortInfo );
}

/*-----------------------------------------------------------------------------
Description: Get the dimension, in pixels, of a string (which can have scale
             tokens and cariage returns as well).
-----------------------------------------------------------------------------*/
void nglGetStringDimensions(nglFont *Font, u_int *outWidth, u_int *outHeight, const char *Text, ...)
{
	const char *TextPtr = Text;
	char c;
	char prev_c = 0;

	float ScaleX = 1.0f;
	float ScaleY = 1.0f;
	float CurMaxScaleY = ScaleY;

	float Width  = 0.0f;
	float MaxWidth = 0.0f;
	float Height = 0.0f;
	float MaxHeight = 0.0f;
	float TotalHeight = 0.0f;

	while ((c = *TextPtr))
	{
		// Check for color token: \1[RRGGBBAA] or \1[0xRRGGBBAA]
		if (c == NGLFONT_TOKEN_COLOR)
		{
			TextPtr += 2;
			// String to integer (hexa) conversion.
			char *StopString;
			strtoul(TextPtr, &StopString, 16);
			TextPtr = StopString + 1;
			continue;
		}
		// Check for scale token: \2[SCALE]
		else if (c == NGLFONT_TOKEN_SCALE)
		{
			TextPtr += 2;
			// String to float conversion.
			char *StopString;
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			ScaleX = ScaleY;
			TextPtr = StopString + 1;

			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;

			continue;
		}
		// Check for independent scale token: \3[SCALEX,SCALEY]
		else if (c == NGLFONT_TOKEN_SCALEXY)
		{
			TextPtr += 2;
			// String to float conversion (SCALEX).
			char *StopString;
			ScaleX = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;

			// String to float conversion (SCALEY).
			float OldScaleY = ScaleY;
			ScaleY = (float) strtod(TextPtr, &StopString);
			TextPtr = StopString + 1;

			// Store the current max scale (needed to go to the next line).
			if (ScaleY > OldScaleY)
				CurMaxScaleY = ScaleY;
			else
				CurMaxScaleY = OldScaleY;

			continue;
		}

		if (c != '\n')
		{
			if (prev_c == '\n')
			{
				TotalHeight += MaxHeight;
				CurMaxScaleY = ScaleY;
				Height = Font->Header.CellHeight * ScaleY;
				MaxHeight = Height;
				Width = 0;
			}

			const nglGlyphInfo &ginfo = Font->GlyphInfo[(char)c - Font->Header.FirstGlyph];

			Width += ginfo.CellWidth * ScaleX;
			if (Width > MaxWidth)
				MaxWidth = Width;

			Height = Font->Header.CellHeight * CurMaxScaleY;
			if (Height > MaxHeight)
				MaxHeight = Height;
		}

		prev_c = c;

		TextPtr++;
	}

	TotalHeight += MaxHeight;

	*outHeight = nglFTOI(TotalHeight);
	*outWidth = nglFTOI(MaxWidth);
}

//----------------------------------------------------------------------------------------
//  Mesh @Morphing code.
//----------------------------------------------------------------------------------------
nglMesh* nglCreateMeshCopy( nglMesh* SrcMesh )
{
  // This is messier than it should be, since ps2mesh's aren't 128 byte aligned WITHIN the file.
  nglMesh* NewMesh = (nglMesh*)nglMemAlloc( SrcMesh->DataSize + 128, 128 );
  u_int SrcAlignment = ( (u_int)SrcMesh ) & 127;
  NewMesh = (nglMesh*)( ( (u_int)NewMesh & ~127 ) | SrcAlignment );

  memcpy( NewMesh, SrcMesh, SrcMesh->DataSize );

  nglRebaseMesh( (u_int)NewMesh, (u_int)SrcMesh, NewMesh );
  nglProcessMesh( NewMesh, NGL_MESHFILE_VERSION );

  return NewMesh;
}

void nglMorphMesh( nglMesh* DestMesh, nglMesh* SrcMesh, nglMorphTarget* Morph, float Weight, u_int ComponentMask, u_long SectionMask )
{
  FlushCache( WRITEBACK_DCACHE );
  nglVif1SafeWait();

  SectionMask |= Morph->SectionMask;
  ComponentMask &= Morph->ComponentMask;

  NGL_ASSERT( SrcMesh->NSections == DestMesh->NSections == Morph->NSections, "nglMorphMesh: Section count not equal." );
  nglMeshSection* SrcSection = SrcMesh->Sections;
  nglMeshSection* DestSection = DestMesh->Sections;
  nglMorphTargetSection* MorphSection = Morph->Sections;

  for ( u_int i = 0; i < SrcMesh->NSections; i++ )
  {
    if ( SectionMask & ( 1 << i ) )
      continue;

    NGL_ASSERT( SrcMesh->NSections == DestMesh->NSections, "nglMorphMesh: Batch count not equal." );
    nglMeshBatchInfo* SrcBatch = SrcSection->BatchInfo;
    nglMeshBatchInfo* DestBatch = DestSection->BatchInfo;
    nglMeshBatchInfo* MorphBatch = MorphSection->BatchInfo;

    for ( u_int j = 0; j < SrcSection->NBatches; j++ )
    {
      // Morph XYZ's.
      if ( ( ComponentMask & NGLMC_XYZ ) && MorphBatch->PosData )
      {
        u_int QWC = ( SrcBatch->NVerts * 3 * sizeof(float) + 15 ) / 16 + 1;
        _nglDmaStartToSPRNormal( 0, (u_int)SrcBatch->PosData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );
        _nglDmaStartToSPRNormal( QWC * 16, (u_int)MorphBatch->PosData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );

        register float* SrcData = (float*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->PosData & 0xF ) );
        register float* MorphData = (float*)( NGL_SCRATCHPAD_MEM + QWC * 16 + ( (u_int)MorphBatch->PosData & 0xF ) );
        register float _Weight = Weight;
        register int Count = SrcBatch->NVerts * 3;

        for ( ; Count > 0; --Count )
          *SrcData = *(SrcData++) + *(MorphData++) * _Weight;

        memcpy( (char*)( (u_int)DestBatch->PosData & ~0xF ), (char*)NGL_SCRATCHPAD_MEM, QWC * 16 );
//        _nglDmaStartFromSPRNormal( (u_int)DestBatch->PosData & ~0xF, 0, QWC );
//        while ( *D8_CHCR & D_CHCR_STR_M );
      }

      // Morph Normals.
      if ( ( ComponentMask & NGLMC_NORMAL ) && MorphBatch->NormData )
      {
        u_int QWC = ( SrcBatch->NVerts * 3 * sizeof(float) + 15 ) / 16 + 1;
        _nglDmaStartToSPRNormal( 0, (u_int)SrcBatch->NormData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );
        _nglDmaStartToSPRNormal( QWC * 16, (u_int)MorphBatch->NormData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );

        register float* SrcData = (float*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->NormData & 0xF ) );
        register float* MorphData = (float*)( NGL_SCRATCHPAD_MEM + QWC * 16 + ( (u_int)MorphBatch->NormData & 0xF ) );
        register float _Weight = Weight;
        register int Count = SrcBatch->NVerts * 3;
        for ( ; Count > 0; --Count )
          *SrcData = *(SrcData++) + *(MorphData++) * _Weight;

        memcpy( DestBatch->NormData, (float*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->NormData & 0xF ) ), SrcBatch->NVerts * 3 * sizeof(float) );
//        _nglDmaStartFromSPRNormal( (u_int)DestBatch->NormData & ~0xF, 0, QWC );
//        while ( *D8_CHCR & D_CHCR_STR_M );
      }

      // Morph UVs.
      if ( ( ComponentMask & NGLMC_UV ) && MorphBatch->UVData )
      {
        u_int QWC = ( SrcBatch->NVerts * 2 * sizeof(float) + 15 ) / 16 + 1;
        _nglDmaStartToSPRNormal( 0, (u_int)SrcBatch->UVData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );
        _nglDmaStartToSPRNormal( QWC * 16, (u_int)MorphBatch->UVData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );

        register float* SrcData = (float*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->UVData & 0xF ) );
        register float* MorphData = (float*)( NGL_SCRATCHPAD_MEM + QWC * 16 + ( (u_int)MorphBatch->UVData & 0xF ) );
        register float _Weight = Weight;
        register int Count = SrcBatch->NVerts * 2;
        for ( ; Count > 0; --Count )
          *SrcData = *(SrcData++) + *(MorphData++) * _Weight;

        memcpy( DestBatch->UVData, (float*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->UVData & 0xF ) ), SrcBatch->NVerts * 2 * sizeof(float) );
//        _nglDmaStartFromSPRNormal( (u_int)DestBatch->UVData & ~0xF, 0, QWC );
//        while ( *D8_CHCR & D_CHCR_STR_M );
      }

      // Morph Colors.
      if ( ( ComponentMask & NGLMC_COLOR ) && MorphBatch->ColorData )
      {
        u_int QWC = ( SrcBatch->NVerts * sizeof(u_int) + 15 ) / 16 + 1;
        _nglDmaStartToSPRNormal( 0, (u_int)SrcBatch->ColorData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );
        _nglDmaStartToSPRNormal( QWC * 16, (u_int)MorphBatch->ColorData & ~0xF, QWC );
        while ( *D9_CHCR & D_CHCR_STR_M );

        register char* SrcData = (char*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->ColorData & 0xF ) );
        register char* MorphData = (char*)( NGL_SCRATCHPAD_MEM + QWC * 16 + ( (u_int)MorphBatch->ColorData & 0xF ) );
        register int _Weight = (int)( Weight * 255.0f );
        register int Count = SrcBatch->NVerts * 4;
        for ( ; Count > 0; --Count )
          *SrcData = *(SrcData++) + ( ( (int)*(MorphData++) * _Weight ) >> 8 );

        memcpy( DestBatch->ColorData, (float*)( NGL_SCRATCHPAD_MEM + ( (u_int)SrcBatch->ColorData & 0xF ) ), SrcBatch->NVerts * sizeof(u_int) );
//        _nglDmaStartFromSPRNormal( (u_int)DestBatch->ColorData & ~0xF, 0, QWC );
//        while ( *D8_CHCR & D_CHCR_STR_M );
      }

      SrcBatch++;
      DestBatch++;
      MorphBatch++;
    }
    SrcSection++;
    DestSection++;
    MorphSection++;
  }
}

// Change all the internal pointers in a mesh structure to be based at a new location.  Useful for copying meshes around
// and for loading them from files (in which they're based at 0).
void nglRebaseMorphTarget( u_int NewBase, u_int OldBase, nglMorphTarget* Morph )
{
#define PTR_OFFSET( Ptr, Type ) { if ( Ptr ) Ptr = (Type)( (u_int)Ptr + ( (u_int)NewBase - (u_int)OldBase ) ); }

  PTR_OFFSET( Morph->Sections, nglMorphTargetSection* );

  for ( u_int i = 0; i < Morph->NSections; i++ )
  {
    nglMorphTargetSection* Section = &Morph->Sections[i];

    PTR_OFFSET( Section->BatchInfo, nglMeshBatchInfo* );
    for ( u_int j = 0; j < Section->NBatches; j++ )
    {
      nglMeshBatchInfo* Batch = &Section->BatchInfo[j];
      PTR_OFFSET( Batch->StripData, u_int* );
      PTR_OFFSET( Batch->PosData, float* );
      PTR_OFFSET( Batch->NormData, float* );
      PTR_OFFSET( Batch->ColorData, u_int* );
      PTR_OFFSET( Batch->UVData, float* );
      PTR_OFFSET( Batch->LightUVData, float* );
      PTR_OFFSET( Batch->BoneCountData, char* );
      PTR_OFFSET( Batch->BoneIdxData, unsigned short* );
      PTR_OFFSET( Batch->BoneWeightData, float* );
    }
  }

#undef PTR_OFFSET
}

nglMorphTarget* nglCreateMorphTarget( nglMesh* SrcMesh, nglMesh* DestMesh, u_int ComponentMask, u_long SectionMask )
{
  // Allocate the morph target and sections.
  nglMorphTarget* Morph = (nglMorphTarget*)nglScratchMeshPos;
  nglScratchMeshPos += sizeof(nglMorphTarget) / sizeof(u_int);

  Morph->ComponentMask = ComponentMask;
  Morph->SectionMask = SectionMask;

  NGL_ASSERT( SrcMesh->NSections = DestMesh->NSections, "Morph target creation - section count mismatch." );
  Morph->NSections = SrcMesh->NSections;
  Morph->Sections = (nglMorphTargetSection*)nglScratchMeshPos;
  nglScratchMeshPos += ( sizeof(nglMorphTargetSection) * Morph->NSections ) / sizeof(u_int);
  memset( Morph->Sections, 0, sizeof(nglMorphTargetSection) * Morph->NSections );

  for ( u_int i = 0; i < Morph->NSections; i++ )
  {
    if ( SectionMask & ( 1 << i ) )
      continue;

    nglMeshSection* SrcSection = &SrcMesh->Sections[i];
    nglMeshSection* DestSection = &DestMesh->Sections[i];
    NGL_ASSERT( SrcSection->NBatches == DestSection->NBatches, "Morph target creation - batch count mismatch." );

    nglMorphTargetSection* Section = &Morph->Sections[i];
    Section->NBatches = SrcSection->NBatches;

    Section->BatchInfo = (nglMeshBatchInfo*)nglScratchMeshPos;
    nglScratchMeshPos += ( sizeof(nglMeshBatchInfo) * Section->NBatches ) / sizeof(u_int);
    memset( Section->BatchInfo, 0, sizeof(nglMeshBatchInfo) * Section->NBatches );

    for ( u_int j = 0; j < SrcSection->NBatches; j++ )
    {
      nglMeshBatchInfo* SrcBatch = &SrcSection->BatchInfo[j];
      nglMeshBatchInfo* DestBatch = &DestSection->BatchInfo[j];
      NGL_ASSERT( SrcBatch->NVerts == DestBatch->NVerts, "Morph target creation - vertex count mismatch." );

      nglMeshBatchInfo* Batch = &Section->BatchInfo[j];
      memset( Batch, 0, sizeof(nglMeshBatchInfo) );

      Batch->NVerts = SrcBatch->NVerts;

      // Morph XYZ's.
      if ( ComponentMask & NGLMC_XYZ )
      {
        if ( memcmp( SrcBatch->PosData, DestBatch->PosData, Batch->NVerts * 12 ) != 0 )
        {
          Batch->PosData = (float*)nglScratchMeshPos;
          nglScratchMeshPos += Batch->NVerts * 3;
          for ( u_int k = 0; k < Batch->NVerts * 3; k++ )
            Batch->PosData[k] = DestBatch->PosData[k] - SrcBatch->PosData[k];
        }
      }

      // Morph Normals.
      if ( ComponentMask & NGLMC_NORMAL )
      {
        if ( memcmp( SrcBatch->NormData, DestBatch->NormData, Batch->NVerts * 12 ) != 0 )
        {
          Batch->NormData = (float*)nglScratchMeshPos;
          nglScratchMeshPos += Batch->NVerts * 3;
          for ( u_int k = 0; k < Batch->NVerts * 3; k++ )
            Batch->NormData[k] = DestBatch->NormData[k] - SrcBatch->NormData[k];
        }
      }

      // Morph UVs.
      if ( ComponentMask & NGLMC_UV )
      {
        if ( memcmp( SrcBatch->UVData, DestBatch->UVData, Batch->NVerts * 8 ) != 0 )
        {
          Batch->UVData = (float*)nglScratchMeshPos;
          nglScratchMeshPos += Batch->NVerts * 2;
          for ( u_int k = 0; k < Batch->NVerts * 2; k++ )
            Batch->UVData[k] = DestBatch->UVData[k] - SrcBatch->UVData[k];
        }
      }

      // Morph Colors (using signed bytes).
      if ( ComponentMask & NGLMC_COLOR )
      {
        if ( memcmp( SrcBatch->ColorData, DestBatch->ColorData, Batch->NVerts * 4 ) != 0 )
        {
          Batch->ColorData = (u_int*)nglScratchMeshPos;
          nglScratchMeshPos += Batch->NVerts;
          char* SrcColor = (char*)SrcBatch->ColorData;
          char* DestColor = (char*)DestBatch->ColorData;
          char* Color = (char*)Batch->ColorData;
          for ( u_int k = 0; k < Batch->NVerts * 4; k++ )
            Color[k] = DestColor[k] - SrcColor[k];
        }
      }
    }
  }

  Morph->Size = (u_int)nglScratchMeshPos - (u_int)nglScratchMeshWork;

  nglMorphTarget* NewMorph = (nglMorphTarget*)nglMemAlloc( Morph->Size );
  memcpy( NewMorph, Morph, Morph->Size );

  nglRebaseMorphTarget( (u_int)NewMorph, (u_int)Morph, NewMorph );

  nglScratchMeshPos = (u_int*)Morph;
  return NewMorph;
}

void nglDestroyMorphTarget( nglMorphTarget* Morph )
{
  nglMemFree( Morph );
}

//----------------------------------------------------------------------------------------
//  @Particle system code (custom node).
//----------------------------------------------------------------------------------------
u_int debug_draw_points = 0;

void nglVif1RenderParticleSystem( u_int*& _Packet, void* Data )
{
  nglParticleSystem* Particle = (nglParticleSystem*)Data;
  nglTexture* Tex = Particle->Tex;
  u_int AddTrans = 0;

  if (Tex == NULL)
    Tex = nglWhiteTex;

  float fbuf[30];
  u_int buf[4];

  u_int* Packet = _Packet;

  // Upload the particle system VU code.
  if ( nglLastVUCodeDma != NGL_VUCODE_PART )
  {
    AddTrans = 1;
    nglLastVUCodeDma = NGL_VUCODE_PART;
    nglDmaStartTag( Packet );
    nglDmaEndTag( Packet, SCE_DMA_ID_CALL, nglLoadParticleMicrocode );
  }

  nglDmaStartTag( Packet );

  nglVif1AddSingleTextureStreaming( Packet, Tex );

  Packet[0] = SCE_VIF1_SET_BASE( 0, 0 );
  Packet[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
  Packet[2] = SCE_VIF1_SET_STCYCL( 1, 1, 0 );
  Packet[3] = SCE_VIF1_SET_ITOP( 0, 0 );
  Packet += 4;

  u_long Tag = SCE_GIF_SET_TAG( 2, 1, 0, 0, SCE_GIF_PACKED, 3 );
  Packet[0] = SCE_VIF1_SET_UNPACK( 50, 1, SCE_VIF1_V4_32, 0 );
  Packet[1] = Tag & 0xFFFFFFFF;
  Packet[2] = Tag >> 32;
  Packet[3] = 0x513;
  Packet[4] = 0;
  Packet += 5;

  fbuf[0] = 0.0f;
  nglVif1AddUnpack( Packet, 51, SCE_VIF1_S_32, 1, &fbuf[0], sizeof(u_int));
  nglVif1AddUnpack( Packet, 58, SCE_VIF1_S_32, 1, &fbuf[0], sizeof(u_int));

  fbuf[0] = Tex->Height;
  nglVif1AddUnpack( Packet, 54, SCE_VIF1_S_32, 1, &fbuf[0], sizeof(u_int));
  nglVif1AddUnpack( Packet, 61, SCE_VIF1_S_32, 1, &fbuf[0], sizeof(u_int));

  // Set the offset value
  int offaddr = NGLMEM_PART_DATA;

  // S_32
  buf[0] = Particle->Num;
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_S_32, 1, buf, sizeof(u_int));
  offaddr +=1;

  // V4_8
  buf[0] = Particle->Scol;
  buf[1] = Particle->Rcol;
  buf[2] = Particle->Ecol;
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_V4_8, 3, buf, sizeof(u_int)*3);
  offaddr +=3;

  // S_32
  fbuf[0] = Particle->Ssize;
  fbuf[1] = Particle->Rsize;
  fbuf[2] = Particle->Esize;
  fbuf[3] = Particle->Life;
  fbuf[4] = Particle->Rlife;
  fbuf[5] = Particle->Dura;
  fbuf[6] = Particle->Aspect;
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_S_32, 7, fbuf, sizeof(u_int)*7);
  offaddr += 7;

  buf[0] = Particle->Seed;
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_S_32, 1, buf, sizeof(u_int));
  offaddr +=1;

  fbuf[0] = Particle->Ctime;
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_S_32, 1, fbuf, sizeof(u_int));
  offaddr +=1;

  // V3_32
  float *j = Particle->Spos;
  for(int i=0;i<24;)
  {
    fbuf[i++]= *(j++);
    fbuf[i++]= *(j++);
    fbuf[i++]= *(j++);
    j++;
  }
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_V3_32, 8, fbuf, sizeof(u_int)*3*8);
  offaddr +=8;

  fbuf[0] = Particle->MaxSize;
  nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_S_32, 1, fbuf, sizeof(u_int));
  offaddr +=1;

  // Only redownload the transforms if we need to, reduce vif and i/o
  if (AddTrans)
  {
    // Add World -> View Transform
    nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_V4_32, 4, nglCurScene->WorldToView, sizeof(nglMatrix));
    offaddr +=4;

    // Add View  -> Screen Transform
    nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_V4_32, 4, nglCurScene->ViewToScreen, sizeof(nglMatrix));
    offaddr +=4;

    // Add View  -> Clip Transform
    nglVif1AddUnpack( Packet, offaddr, SCE_VIF1_V4_32, 4, nglCurScene->ViewToClip, sizeof(nglMatrix));
    offaddr +=4;
  }

  int ab = 1;  // alpha blending?
  int uv = 1;  // use uv coords?

  nglVif1StartDirectGifAD();

  if (Particle->BlendMode == NGLBM_PUNCHTHROUGH)
  {
//    Particle->MaterialFlags &= ~NGLMAT_ALPHA;
    ab = 0;
  }

  if (!debug_draw_points)
    nglVif1AddRawGSReg( SCE_GS_PRIM, SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, ab, 0, uv, 0, 0) );
  else
    nglVif1AddRawGSReg( SCE_GS_PRIM, SCE_GS_SET_PRIM( SCE_GS_PRIM_POINT, 0, 1, 0, ab, 0, uv, 0, 0) );

  nglVif1AddGSReg( NGL_GS_PRMODE, SCE_GS_SET_PRMODE( 0, 1, 0, ab, 0, uv, 0, 0 ) );
  nglVif1AddSetTexture( Tex, Particle->MaterialFlags, 0.0f );
  nglVif1AddSetBlendMode( Particle->BlendMode, 0x20, (Particle->MaterialFlags & NGLMAT_FORCE_Z_WRITE) ? true : false );
//  nglVif1AddGSReg( SCE_GS_TEST_1, SCE_GS_SET_TEST( 1, 2, 0x80, 1, 0, 0, 0, 0 ) );
  nglVif1EndDirectGifAD( Packet );

  if ( DEBUG_ENABLE( VUParticleDebugBreak ) )
    nglVif1AddCallProgram( Packet, NGL_PROG_ADDR( nglDebugParticleBreakAddr, nglParticleBaseAddr ) );
  else
    nglVif1AddCallProgram( Packet, NGL_PROG_ADDR( nglParticleAddr, nglParticleBaseAddr ) );
  nglVif1AddFlush( Packet );

  nglDmaEndTag( Packet, SCE_DMA_ID_CNT );
  nglVif1FlushSPAD( Packet );

  _Packet = Packet;
}

void nglListAddParticle( nglParticleSystem* Particle )
{
  // Create the particle node.
  nglParticleSystem* ParticleNode = nglListNew( nglParticleSystem );
  if ( !ParticleNode )
    return;

  memcpy( ParticleNode, Particle, sizeof(nglParticleSystem) );

  nglSortInfo Info;
  Info.Type = NGLSORT_TRANSLUCENT;

  nglVector Center;
  sceVu0ApplyMatrix(Center, SCE_MATRIX(nglCurScene->WorldToView), Particle->Spos );

  nglPerfInfo.ParticleCount += Particle->Num;
  Info.Dist = Center[2];
  nglListAddNode( NGLNODE_PARTICLE, nglVif1RenderParticleSystem, ParticleNode, &Info );
}

//----------------------------------------------------------------------------------------
//  @Scratch mesh code.
//----------------------------------------------------------------------------------------
// DMA chain building code for scratch meshes.  Keep in sync with ps2mesh.cpp in meshcvt.
void nglVif1AddCommandListExec( u_int*& Packet, u_int VertBase, u_int Pass )
{
  Packet[0] = SCE_VIF1_SET_FLUSHE( 0 );
  Packet[1] = SCE_VIF1_SET_UNPACK( NGLMEM_COMMAND_LIST_INDEX, 1, SCE_VIF1_S_32, 0 );
  Packet[2] = Pass;
  Packet[3] = SCE_VIF1_SET_ITOP( VertBase, 0 );
  Packet[4] = SCE_VIF1_SET_MSCNT( 0 );
  Packet += 5;

//  *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
}

void nglVif1AddBatchSetup( u_int*& Packet, u_int VertBase, int NVerts )
{
  // Unpack the vertex count for this batch over the GIFtag left in memory by NGL.
  Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_GIFTAG, 1, SCE_VIF1_S_32 | SCE_VIF1_MASK_ENABLE, 0 );
  Packet[1] = NVerts | 0x8000;
  Packet += 2;
}

void nglVif1AddSkinPass( u_int*& Packet, u_int VertBase, u_int SrcBufSize, nglMeshSection* Section, nglMeshBatchInfo* Batch )
{
  // Unpack the SrcXYZs into the source buffer.
  Batch->PosData = (float*)Packet + 4;
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 2, 0 );
  Packet[1] = SCE_VIF1_SET_STMASK( 0 );
  Packet[2] = 0x40404040; // 01-00-00-00 (WZYX)
  Packet[3] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + SrcBufSize, Batch->NVerts, SCE_VIF1_V3_32 | SCE_VIF1_MASK_ENABLE, 0 );
  Packet += 4 + Batch->NVerts * 3;

  // Unpack the normal data (if present).
  nglMaterial* Material = Section->Material;
  if (    ( Material->Flags & NGLMAT_LIGHT )
      ||  ( Material->Flags & NGLMAT_DETAIL_MAP )
      ||  ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
      ||  ( Material->Flags & NGLMAT_ALPHA_FALLOFF ) )
  {
    Batch->NormData = (float*)Packet + 1;
    Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + SrcBufSize + 1, Batch->NVerts, SCE_VIF1_V3_32 | SCE_VIF1_MASK_ENABLE, 0 );
    Packet += 1 + Batch->NVerts * 3;
  }

  // Unpack the bone counts into where the UVs normally go.
  Batch->BoneCountData = (char*)( Packet + 2 );
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 );
  Packet[1] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START, Batch->NVerts, SCE_VIF1_S_8, 0 );
  Packet += 2 + ( Batch->NVerts + 3 ) / 4;

// Fix up the bone count data to be jump table offsets instead of bone counts.
//    for ( u_int k = 0; k < Batch->NVerts * 4; k++ )
//      Batch->BoneCountData[k] = nglProgAddrs[NGLPROG_SKIN_ONE_BONE + Batch->BoneCountData[k] - 1];

  // Unpack the bone indices into where the Colors normally go.
  Batch->BoneIdxData = (u_short*)( Packet + 1 );
  Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + 1, Batch->NVerts, SCE_VIF1_V4_16, 0 );
  Packet += 1 + Batch->NVerts * 2;

  // Unpack the weights into where the dest positions normally go.
  Batch->BoneWeightData = (float*)Packet + 1;
  Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + 2, Batch->NVerts, SCE_VIF1_V4_32, 0 );
  Packet += 1 + Batch->NVerts * 4;

  nglVif1AddCommandListExec( Packet, VertBase, NGLCMD_SKIN );
}

void nglVif1AddSkinDiffusePass( u_int*& Packet, u_int VertBase, nglMeshBatchInfo* Batch, nglMaterial* Material )
{
  // Unpack the strip flags onto where the destination positions will go.
  Batch->StripData = (u_int*)Packet + 2;
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 );
  Packet[1] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + 2, Batch->NVerts, SCE_VIF1_S_32, 0 );
  Packet += 2 + Batch->NVerts;

//  for ( u_int i = 0; i < Batch->NVerts; i++ )
//    if ( Batch->StripData[i] == 0x8000 )
//      Batch->StripData[i] |= 1;

  // Unpack the UV data.
  if ( Material->Flags & NGLMAT_TEXTURE_MAP )
  {
    Batch->UVData = (float*)Packet + 3;
    Packet[0] = SCE_VIF1_SET_STMASK( 0 );
    Packet[1] = 0x50505050; // 01-01-00-00 (WZYX)
    Packet[2] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START, Batch->NVerts, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 0 );
    Packet += 3 + Batch->NVerts * 2;
  }

  // Unpack the color data.
  if ( !( Material->Flags & NGLMAT_MATERIAL_COLOR ) )
  {
    Batch->ColorData = (u_int*)Packet + 1;
    Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + 1, Batch->NVerts, SCE_VIF1_V4_8, 0 ) | SCE_VIF1_USN;
    Packet += 1 + Batch->NVerts;
  }

  nglVif1AddCommandListExec( Packet, VertBase, NGLCMD_DIFFUSE );
}

void nglVif1AddDiffusePass( u_int*& Packet, u_int VertBase, u_int SrcBufSize, nglMeshSection* Section, nglMeshBatchInfo* Batch )
{
  // Unpack the strip flags onto where the destination positions will go.
  Batch->StripData = (u_int*)Packet + 2;
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 );
  Packet[1] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + 2, Batch->NVerts, SCE_VIF1_S_32, 0 );
  Packet += 2 + Batch->NVerts;

  //  for ( u_int i = 0; i < Batch->NVerts; i++ )
  //    if ( Batch->StripData[i] == 0x8000 )
  //      Batch->StripData[i] |= 1;

  // Unpack the UV data.
  if ( Section->Material->Flags & NGLMAT_TEXTURE_MAP )
  {
    Batch->UVData = (float*)Packet + 3;
    Packet[0] = SCE_VIF1_SET_STMASK( 0 );
    Packet[1] = 0x50505050; // 01-01-00-00 (WZYX)
    Packet[2] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START, Batch->NVerts, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 0 );
    Packet += 3 + Batch->NVerts * 2;
  }

  // Unpack the color data.
  if ( ( Section->Material->Flags & NGLMAT_MATERIAL_COLOR ) == 0 )
  {
    Batch->ColorData = (u_int*)Packet + 1;
    Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + 1, Batch->NVerts, SCE_VIF1_V4_8, 0 ) | SCE_VIF1_USN;
    Packet += 1 + Batch->NVerts;
  }

  // Unpack the SrcXYZs into the source buffer.
  Batch->PosData = (float*)Packet + 4;
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 2, 0 );
  Packet[1] = SCE_VIF1_SET_STMASK( 0 );
  Packet[2] = 0x40404040; // 01-00-00-00 (WZYX)
  Packet[3] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + SrcBufSize, Batch->NVerts, SCE_VIF1_V3_32 | SCE_VIF1_MASK_ENABLE, 0 );
  Packet += 4 + Batch->NVerts * 3;

  // Unpack the normal data (if present).
  nglMaterial* Material = Section->Material;
  if (  ( Material->Flags & NGLMAT_LIGHT )
      ||  ( Material->Flags & NGLMAT_DETAIL_MAP )
    ||  ( Material->Flags & NGLMAT_ENVIRONMENT_MAP )
    ||  ( Material->Flags & NGLMAT_ALPHA_FALLOFF ) )
  {
    Batch->NormData = (float*)Packet + 1;
    Packet[0] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START + SrcBufSize + 1, Batch->NVerts, SCE_VIF1_V3_32 | SCE_VIF1_MASK_ENABLE, 0 );
    Packet += 1 + Batch->NVerts * 3;
  }

  nglVif1AddCommandListExec( Packet, VertBase, NGLCMD_DIFFUSE );
}

void nglVif1AddLightPass( u_int*& Packet, u_int VertBase, nglMeshBatchInfo* Batch )
{
  // Upload the lightmap UVs.
  Batch->LightUVData = (float*)Packet + 4;
  Packet[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 );
  Packet[1] = SCE_VIF1_SET_STMASK( 0 );
  Packet[2] = 0x70707070; // 01-11-00-00 (WZYX)
  Packet[3] = SCE_VIF1_SET_UNPACK( VertBase + NGLMEM_VERT_START, Batch->NVerts, SCE_VIF1_V2_32 | SCE_VIF1_MASK_ENABLE, 0 );
  Packet += 4 + Batch->NVerts * 2;

  nglVif1AddCommandListExec( Packet, VertBase, NGLCMD_LIGHT );
}


// Figure out the maximum batch size for nglCreateMeshDMA after reserving space for the bones and clip buffer.
void nglComputeBatchSize(u_int flags, u_int nBones, u_int nVerts, u_int * MaxVerts, u_int * nBatches, u_int * BufSize)
{
  u_int BufferEnd = NGLMEM_END;
  if ( flags & NGLMESH_PERFECT_TRICLIP )
    BufferEnd = NGLMEM_CLIP_BUF;
  BufferEnd -= nBones * 4;
  *MaxVerts = ( ( BufferEnd - NGLMEM_BUFFER0 - 3 ) / 3 ) / 5;
  *BufSize = ( BufferEnd - NGLMEM_BUFFER0 ) / 3;
  *MaxVerts -= 4;  // Account for maximum of 3 vert VU code overrun.
  *MaxVerts -= 2;  // Account for the 2 starting verts (continuation of the previous batch present in all but the first batch).

  // For small meshes, don't bother trying to triple buffer anything.
  if ( nVerts < *MaxVerts / 2 )
    *nBatches = 1;
  else
  if ( nVerts < *MaxVerts )
    *nBatches = 2;
  else
  {
    // Find the largest multiple of 3 batches that fits the mesh.
    int n = nVerts / ( *MaxVerts * 3 ) + 1;
    *nBatches = (int)n * 3;
  }
}

// Estimates the number of bytes that will be required by MeshDMAMemory.
// This is guaranteed to be greater than or
// equal to the actual number of bytes requred.  It is needed to make the Scratch mesh allocator gracefully fail when
// it is going to overflow the scratch mesh buffer.
/*
u_int nglEstimateMeshMemory(u_int nBones, u_int nVerts)
{
  u_int nBatches, dummy1, dummy2, estimate;

  nglComputeBatchSize(NGLMESH_PERFECT_TRICLIP, nBones, nVerts, &dummy1, &nBatches, &dummy2);

  estimate = sizeof(nglScratchMesh)*4 + nBatches * sizeof(nglMeshBatchInfo) + 128 + 24 + 128*((int)nBatches+1) + 15*nVerts*4;

  return estimate;
}
*/

// Creates the DMA chain for a mesh section and fills out the rest of the section structure.
void nglCreateMeshDMA( u_int*& Packet, nglMesh* Mesh, nglMeshSection* Section )
{
  if ( ( Mesh->Flags & NGLMESH_SKINNED ) && ( Mesh->Flags & NGLMESH_PERFECT_TRICLIP ) )
    Mesh->Flags &= ~NGLMESH_PERFECT_TRICLIP;

  // Figure out the maximum batch size after reserving space for the bones and clip buffer.
  u_int MaxVerts;
  nglComputeBatchSize(Mesh->Flags, Mesh->NBones, Section->NVerts, &MaxVerts, &(Section->NBatches), &(Section->BatchBufSize));

  // Allocate the batch info structures.
  Section->BatchInfo = (nglMeshBatchInfo*)Packet;
  Packet += Section->NBatches * sizeof(nglMeshBatchInfo) / sizeof(u_int);
  memset( Section->BatchInfo, 0, Section->NBatches * sizeof(nglMeshBatchInfo) );

  nglMeshBatchInfo* Batch = Section->BatchInfo;
  u_int BatchVerts =  ( Section->NVerts / Section->NBatches ) + 1;
  NGL_ASSERT( BatchVerts <= MaxVerts, "Internal batch sizing error." );

  // Align to 128 byte boundary for DMA speed.
  while ( ( (u_int)Packet ) & 127 ) *(Packet++) = SCE_VIF1_SET_NOP( 0 );

  Section->BatchDMA = Packet;

  // Flush rendering and set the initial MASK register.
  Packet[0] = SCE_VIF1_SET_FLUSH( 0 );
  Packet[1] = SCE_VIF1_SET_STROW( 0 );
  Packet[2] = 0;
  Packet[3] = 0;
  Packet[4] = 0x3f800000; // == 1.0f
  Packet[5] = 0;
  Packet += 6;

  // Unpack the SRCBUF offset.
  int SrcBufOfs = ( MaxVerts + 6 ) * 3;
  Packet[0] = SCE_VIF1_SET_UNPACK( NGLMEM_SRCBUF, 1, SCE_VIF1_S_32, 0 );
  Packet[1] = NGLMEM_VERT_START + SrcBufOfs;
  Packet += 2;

  nglMaterial* Material = Section->Material;
  u_int BatchesLeft = Section->NBatches;
  u_int VertsLeft = Section->NVerts;
  u_int BufferAddr[3] = { NGLMEM_BUFFER0, NGLMEM_BUFFER0 + Section->BatchBufSize, NGLMEM_BUFFER0 + Section->BatchBufSize * 2 };

//  nglPrintf( "Creating BatchDMA.  NVerts=%d MaxVerts=%d NBatches=%d\n", Section->NVerts, MaxVerts, BatchesLeft );

  while ( BatchesLeft )
  {
    u_int num = BatchesLeft > 3 ? 3 : BatchesLeft;
    u_int i;

    // Determine the number of verts going into each batch.
    for ( i = 0; i < num; i++ )
    {
      nglMeshBatchInfo* CurBatch = Batch + i;
      CurBatch->NVerts = VertsLeft > BatchVerts ? BatchVerts : VertsLeft;
      VertsLeft -= CurBatch->NVerts;

      // All batches but the first one have two verts implicitly added to continue the strip from the previous batch.
      if ( i > 0 || BatchesLeft < Section->NBatches )
        CurBatch->NVerts += 2;

//      nglPrintf( "Added batch w/%d verts.\n", CurBatch->NVerts );
    }

    // Set GIFtags in VU memory.  Could we replace this with a single upload and VU code?
    Packet[0] = SCE_VIF1_SET_STMASK( 0 );
    Packet[1] = 0xFCFCFCFC; // 00-00-00-11 (WZYX)
    Packet += 2;
    for ( i = 0; i < num; i++ )
      nglVif1AddBatchSetup( Packet, BufferAddr[i], ( Batch + i )->NVerts );
    if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

	// Process skinning if neccessary.
    if ( Mesh->Flags & NGLMESH_SKINNED )
    {
      for ( i = 0; i < num; i++ )
        nglVif1AddSkinPass( Packet, BufferAddr[i], SrcBufOfs, Section, Batch + i );
      if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

      // Add the skinned diffuse pass.
      for ( i = 0; i < num; i++ )
        nglVif1AddSkinDiffusePass( Packet, BufferAddr[i], Batch + i, Material );
      if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
    }
    else
    {
      // Add the diffuse pass.
      for ( i = 0; i < num; i++ )
        nglVif1AddDiffusePass( Packet, BufferAddr[i], SrcBufOfs, Section, Batch + i );
      if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
    }

    // Call the detail map command list.
    for ( i = 0; i < num; i++ )
      nglVif1AddCommandListExec( Packet, BufferAddr[i], NGLCMD_DETAIL );
    if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

    // Call the environment map command list.
    for ( i = 0; i < num; i++ )
      nglVif1AddCommandListExec( Packet, BufferAddr[i], NGLCMD_ENVIRONMENT );
    if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

    // Add the light map if present.
    if ( Material->Flags & NGLMAT_LIGHT_MAP )
    {
      for ( i = 0; i < num; i++ )
      {
#ifdef PROJECT_KELLYSLATER
        nglVif1AddCommandListExec( Packet, BufferAddr[i], NGLCMD_LIGHT );
#else
        nglVif1AddLightPass( Packet, BufferAddr[i], Batch + i );
#endif
      }
      if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
    }

    // Call the command list for projected textures.
    for ( i = 0; i < num; i++ )
      nglVif1AddCommandListExec( Packet, BufferAddr[i], NGLCMD_PROJECTEDTEXTURES );
    if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

    // Call the command list for the fog pass.
    for ( i = 0; i < num; i++ )
      nglVif1AddCommandListExec( Packet, BufferAddr[i], NGLCMD_FOG );
    if ( num < 3 ) *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

    *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
    nglVif1AddCommandListExec( Packet, 0, NGLCMD_VERTEX_LIGHT );
    *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

    Batch += num;
    BatchesLeft -= num;
  }

  *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

  // Align to 16 byte boundary and finish.
  while ( ( (u_int)Packet ) & 15 ) *(Packet++) = SCE_VIF1_SET_NOP( 0 );
  Section->BatchQWC = ( Packet - Section->BatchDMA ) / 4;
}

void nglCreateMesh( u_int NSections, u_int NBones, nglMatrix* Bones )
{
  while ( ( (u_int)nglScratchMeshPos ) & 127 ) *(nglScratchMeshPos++) = SCE_VIF1_SET_NOP( 0 );

  // Set up the mesh.
  u_int i;
  u_int* BufPos = nglScratchMeshPos;
  nglMesh* Mesh = (nglMesh*)BufPos;
  BufPos += sizeof(nglMesh) / sizeof(u_int);
  memset( Mesh, 0, sizeof(nglMesh) );
  Mesh->Flags = NGLMESH_PERFECT_TRICLIP | NGLMESH_SCRATCH_MESH;

  Mesh->NSections = NSections;
  Mesh->Sections = (nglMeshSection*)BufPos;
  BufPos += NSections * sizeof(nglMeshSection) / sizeof(u_int);
  memset( Mesh->Sections, 0, NSections * sizeof(nglMeshSection) );

  for ( i = 0; i < NSections; i++ )
  {
    Mesh->Sections[i].Material = (nglMaterial*)BufPos;
    BufPos += sizeof(nglMaterial) / sizeof(u_int);
    memset( Mesh->Sections[i].Material, 0, sizeof(nglMaterial) );

    Mesh->Sections[i].Material->Info = (nglMaterialInfo*)BufPos;
    BufPos += sizeof(nglMaterialInfo) / sizeof(u_int);
    memset( Mesh->Sections[i].Material->Info, 0, sizeof(nglMaterialInfo) );
  }

  Mesh->NBones = NBones;
  BufPos = (u_int*)NGL_ALIGN( (u_int)BufPos, 16 );
  Mesh->Bones = (nglMatrix*)BufPos;
  BufPos += NBones * sizeof(nglMatrix) / sizeof(u_int);
  if (Bones)
    for (i=0; i<NBones; ++i)
    {
//      Mesh->Bones[i] = Bones[i];
//        *(transform_t*)&Mesh->Bones[i] = ( *(transform_t*)&Mesh->Bones[i] ).orthonormal_inverse();
      sceVu0InversMatrix( SCE_MATRIX( Mesh->Bones[i] ), SCE_MATRIX( Bones[i] ) );
    }

  nglScratchMeshPos = BufPos;

  // See if the create operation overflowed the scratch mesh buffer.
  nglPerfInfo.ScratchMeshBytesUsed = ( (u_int)nglScratchMeshPos & NGL_DMA_MEM ) - (u_int)nglScratchMeshWork;
  if ( nglPerfInfo.ScratchMeshBytesUsed > nglScratchMeshWorkSize )
    nglFatal( "Scratch mesh workspace overflow (%d/%d).\n", nglPerfInfo.ScratchMeshBytesUsed, nglScratchMeshWorkSize );

  nglScratch = Mesh;
  nglScratchSection = Mesh->Sections - 1;  // hack to prepare for 1st addsection call.
  nglScratchVertIdx = 0;
  nglScratchStripVertIdx = 0;

  nglVector Center( 0, 0, 0, 1 );
  nglMeshSetSphere( Center, 1.0e32f );
}

nglMesh* nglCloseMesh()
{
  nglScratch->DataSize = (u_int)nglScratchMeshPos - (u_int)nglScratch;
  if ( nglScratch->Flags & NGLMESH_TEMP )
  {
    nglScratchMeshPos = (u_int*)NGL_ALIGN( (u_int)nglScratchMeshPos, 128 );
    return nglScratch;
  }
  else
  {
    nglScratchMeshPos = (u_int*)nglScratch;

    // Allocate space to copy the mesh.
    nglMesh* Mesh = (nglMesh*)nglMemAlloc( nglScratch->DataSize, 128 );
    memcpy( Mesh, nglScratch, nglScratch->DataSize );

    // Rebase all the pointers within the mesh to point to the new memory.
    nglRebaseMesh( (u_int)Mesh, (u_int)nglScratch, Mesh, true );

    return Mesh;
  }
}

void nglDestroyMesh( nglMesh* Mesh )
{
  // Make sure they don't try and destroy it while it's being rendered from.
  nglVif1SafeWait();

  nglMemFree( Mesh );
}

void nglEditMesh( nglMesh* Mesh )
{
  nglScratch = (nglMesh*)Mesh;
  nglScratchSection = nglScratch->Sections;
  memcpy( &nglScratchBatch, nglScratch->Sections->BatchInfo, sizeof(nglMeshBatchInfo) );
  nglScratchBatchIdx = 0;
  nglScratchVertIdx = 0;
  nglScratchStripVertIdx = 0;
}

void nglMeshSetSection( u_int Idx )
{
  nglScratchSection = &nglScratch->Sections[Idx];
}

void nglMeshSetVertex( u_int VertIdx )
{
  // Unimplemented.
  NGL_ASSERT( false, "Function unimplemented." );
}

void nglMeshGetVertex( nglScratchVertex* Vertex )
{
  // Unimplemented.
  NGL_ASSERT( false, "Function unimplemented." );
}

void nglMeshAddSection( nglMaterial* Material, u_int NVerts, u_int NStrips )
{
  nglScratchSection++;
  if ( (u_int)( nglScratchSection - nglScratch->Sections ) >= nglScratch->NSections )
    nglFatal( "Added too many sections to a scratch mesh.\n" );

  Material->Info = nglScratchSection->Material->Info;

  // Copy the material and correct some common material mistakes.
  memcpy( nglScratchSection->Material, Material, sizeof(nglMaterial) );

  if ( Material->MapBlendMode != NGLBM_OPAQUE && Material->MapBlendMode != NGLBM_PUNCHTHROUGH &&
       Material->MapBlendMode != NGLBM_PUNCHTHROUGH_NO_ZTEST)
    nglScratchSection->Material->Flags |= NGLMAT_ALPHA;

  if ( !( Material->Flags & NGL_TEXTURE_MASK ) )
  {
    nglScratchSection->Material->Flags |= NGLMAT_TEXTURE_MAP;
    nglScratchSection->Material->Map = nglWhiteTex;
  }

  nglBakeMaterial( nglScratch, nglScratchSection, true );

  // Set up the section with defaults.
  nglScratchSection->NVerts = NVerts;
  nglScratchSection->NStrips = 0;

  nglCreateMeshDMA( nglScratchMeshPos, nglScratch, nglScratchSection );

  memcpy( &nglScratchBatch, nglScratch->Sections->BatchInfo, sizeof(nglMeshBatchInfo) );
  nglScratchBatchIdx = 0;

  // See if the operation overflowed the scratch mesh buffer.
  nglPerfInfo.ScratchMeshBytesUsed = ( (u_int)nglScratchMeshPos & NGL_DMA_MEM ) - (u_int)nglScratchMeshWork;
  if ( nglPerfInfo.ScratchMeshBytesUsed > nglScratchMeshWorkSize )
    nglFatal( "Scratch mesh workspace overflow (%d/%d).\n", nglPerfInfo.ScratchMeshBytesUsed, nglScratchMeshWorkSize );
}

// Copy a vertex from one batch to another.  Vert indices are batch relative.
void nglMeshCopyVertex( nglMeshBatchInfo* DestBatch, int DestIdx, nglMeshBatchInfo* SrcBatch, int SrcIdx )
{
  // Position, normal, UV, color.
  DestBatch->PosData[DestIdx * 3 + 0] = SrcBatch->PosData[SrcIdx * 3 + 0];
  DestBatch->PosData[DestIdx * 3 + 1] = SrcBatch->PosData[SrcIdx * 3 + 1];
  DestBatch->PosData[DestIdx * 3 + 2] = SrcBatch->PosData[SrcIdx * 3 + 2];

  if ( DestBatch->NormData )
  {
    DestBatch->NormData[DestIdx * 3 + 0] = SrcBatch->NormData[SrcIdx * 3 + 0];
    DestBatch->NormData[DestIdx * 3 + 1] = SrcBatch->NormData[SrcIdx * 3 + 1];
    DestBatch->NormData[DestIdx * 3 + 2] = SrcBatch->NormData[SrcIdx * 3 + 2];
  }

  if ( DestBatch->UVData )
  {
    DestBatch->UVData[DestIdx * 2 + 0] = SrcBatch->UVData[SrcIdx * 2 + 0];
    DestBatch->UVData[DestIdx * 2 + 1] = SrcBatch->UVData[SrcIdx * 2 + 1];
  }

  if ( DestBatch->ColorData )
    DestBatch->ColorData[DestIdx] = SrcBatch->ColorData[SrcIdx];

  if ( DestBatch->LightUVData )
  {
    DestBatch->LightUVData[DestIdx * 2 + 0] = SrcBatch->LightUVData[SrcIdx * 2 + 0];
    DestBatch->LightUVData[DestIdx * 2 + 1] = SrcBatch->LightUVData[SrcIdx * 2 + 1];
  }
  if ( DestBatch->BoneCountData )
  {
    DestBatch->BoneCountData[DestIdx] = SrcBatch->BoneCountData[SrcIdx];
    DestBatch->BoneIdxData[DestIdx * 4 + 0] = SrcBatch->BoneIdxData[SrcIdx * 4 + 0];
    DestBatch->BoneIdxData[DestIdx * 4 + 1] = SrcBatch->BoneIdxData[SrcIdx * 4 + 1];
    DestBatch->BoneIdxData[DestIdx * 4 + 2] = SrcBatch->BoneIdxData[SrcIdx * 4 + 2];
    DestBatch->BoneIdxData[DestIdx * 4 + 3] = SrcBatch->BoneIdxData[SrcIdx * 4 + 3];
    DestBatch->BoneWeightData[DestIdx * 4 + 0] = SrcBatch->BoneWeightData[SrcIdx * 4 + 0];
    DestBatch->BoneWeightData[DestIdx * 4 + 1] = SrcBatch->BoneWeightData[SrcIdx * 4 + 1];
    DestBatch->BoneWeightData[DestIdx * 4 + 2] = SrcBatch->BoneWeightData[SrcIdx * 4 + 2];
    DestBatch->BoneWeightData[DestIdx * 4 + 3] = SrcBatch->BoneWeightData[SrcIdx * 4 + 3];
  }

}

void nglMeshNextBatch()
{
#if NGL_DEBUGGING
  if ( nglScratchBatchIdx + 1 >= nglScratchSection->NBatches )
    nglFatal( "Added too many vertices to a scratch mesh (%d).\n", nglScratchSection->NVerts );
#endif

  // Copy the last two vertices from the previous batch.
  nglMeshBatchInfo* SrcBatch = &nglScratchSection->BatchInfo[nglScratchBatchIdx];
  nglMeshBatchInfo* DestBatch = SrcBatch + 1;
  nglMeshCopyVertex( DestBatch, 0, SrcBatch, SrcBatch->NVerts - 2 );
  nglMeshCopyVertex( DestBatch, 1, SrcBatch, SrcBatch->NVerts - 1 );
  DestBatch->StripData[0] = 0x8000;
  DestBatch->StripData[1] = 0x8000;

  nglScratchBatchIdx++;
  nglScratchVertIdx = 2;

  memcpy( &nglScratchBatch, DestBatch, sizeof(nglMeshBatchInfo) );
}

void nglMeshWriteStrip( u_int Length )
{
//  nglPrintf( "Added strip. Length=%d VertIdx=%d NVerts=%d\n", Length, nglScratchVertIdx, nglScratchBatch.NVerts );
  nglScratchStripVertIdx = 0;
}

void nglMeshWriteVertexPC( float X, float Y, float Z, u_int Color )
{
	nglMeshFastWriteVertexPC( X, Y, Z, Color );
}

void nglMeshWriteVertexPCUV( float X, float Y, float Z, u_int Color, float U, float V )
{
	nglMeshFastWriteVertexPCUV( X, Y, Z, Color, U, V );
}

void nglMeshWriteVertexPCUVB( float X, float Y, float Z, u_int Color, float U, float V,
                int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPCUVB( X, Y, Z, Color, U, V, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshWriteVertexPCUV2( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2 )
{
	nglMeshFastWriteVertexPCUV2( X, Y, Z, Color, U, V, U2, V2 );
}

void nglMeshWriteVertexPCUV2B( float X, float Y, float Z, u_int Color, float U, float V, float U2, float V2,
                 int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPCUV2B( X, Y, Z, Color, U, V, U2, V2, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshWriteVertexPUV( float X, float Y, float Z, float U, float V )
{
	nglMeshFastWriteVertexPUV( X, Y, Z, U, V );
}

void nglMeshWriteVertexPN( float X, float Y, float Z, float NX, float NY, float NZ )
{
	nglMeshFastWriteVertexPN( X, Y, Z, NX, NY, NZ );
}

void nglMeshWriteVertexPNC( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color )
{
	nglMeshFastWriteVertexPNC( X, Y, Z, NX, NY, NZ, Color );
}

void nglMeshWriteVertexPNCUV( float X, float Y, float Z, float NX, float NY, float NZ, u_int Color, float U, float V )
{
	nglMeshFastWriteVertexPNCUV( X, Y, Z, NX, NY, NZ, Color, U, V );
}

void nglMeshWriteVertexPNUV( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V )
{
	nglMeshFastWriteVertexPNUV( X, Y, Z, NX, NY, NZ, U, V );
}

void nglMeshWriteVertexPNUVB( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V,
                int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPNUVB( X, Y, Z, NX, NY, NZ, U, V, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshWriteVertexPNUV2( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2 )
{
	nglMeshFastWriteVertexPNUV2( X, Y, Z, NX, NY, NZ, U, V, U2, V2 );
}

void nglMeshWriteVertexPNUV2B( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float U2, float V2,
							   int bones, float w1, float w2, float w3, float w4, int b1, int b2, int b3, int b4)
{
	nglMeshFastWriteVertexPNUV2B( X, Y, Z, NX, NY, NZ, U, V, U2, V2, bones, w1, w2, w3, w4, b1, b2, b3, b4);
}

void nglMeshSetSphere( nglVector& Center, float Radius )
{
  sceVu0CopyVector( nglScratch->SphereCenter, Center );
  nglScratch->SphereRadius = Radius;

  for ( u_int s = 0; s < nglScratch->NSections; ++s)
  {
   sceVu0CopyVector( nglScratch->Sections[s].SphereCenter, Center );
   nglScratch->Sections[s].SphereRadius = Radius;
  }
}

void nglMeshCalcSphere( u_int NVerts /*ignored*/ )
{
  float MinX = 1.0e32f, MaxX = -1.0e32f;
  float MinY = 1.0e32f, MaxY = -1.0e32f;
  float MinZ = 1.0e32f, MaxZ = -1.0e32f;

  u_int s, i, j;

  // Calculate the center.
  for ( s = 0; s < nglScratch->NSections; ++s)
  {
   float SectionMinX = 1.0e32f, SectionMaxX = -1.0e32f;
   float SectionMinY = 1.0e32f, SectionMaxY = -1.0e32f;
   float SectionMinZ = 1.0e32f, SectionMaxZ = -1.0e32f;

   for ( j = 0; j < nglScratch->Sections[s].NBatches; j++ )
   {
    nglMeshBatchInfo* Info = &nglScratch->Sections[s].BatchInfo[j];
    int Idx = 0;
    for ( i = 0; i < Info->NVerts; i++ )
    {
      float X, Y, Z;
      X = Info->PosData[Idx++];
      if ( X < SectionMinX ) SectionMinX = X;
      if ( X > SectionMaxX ) SectionMaxX = X;

      Y = Info->PosData[Idx++];
      if ( Y < SectionMinY ) SectionMinY = Y;
      if ( Y > SectionMaxY ) SectionMaxY = Y;

      Z = Info->PosData[Idx++];
      if ( Z < SectionMinZ ) SectionMinZ = Z;
      if ( Z > SectionMaxZ ) SectionMaxZ = Z;
    }
   }

   nglVector Center;
   float Radius;

   Center[0] = SectionMinX + ( SectionMaxX - SectionMinX ) * 0.5f;
   Center[1] = SectionMinY + ( SectionMaxY - SectionMinY ) * 0.5f;
   Center[2] = SectionMinZ + ( SectionMaxZ - SectionMinZ ) * 0.5f;
   Center[3] = 1.0f;

   // Calculate the radius from an arbitrary point on the box.
   float DistX, DistY, DistZ;
   DistX = SectionMinX - Center[0];
   DistY = SectionMinY - Center[1];
   DistZ = SectionMinZ - Center[2];
   Radius = sqrtf( DistX * DistX + DistY * DistY + DistZ * DistZ );

   sceVu0CopyVector( nglScratch->Sections[s].SphereCenter, Center );
   nglScratch->Sections[s].SphereRadius = Radius;

   if ( SectionMinX < MinX ) MinX = SectionMinX;
   if ( SectionMaxX > MaxX ) MaxX = SectionMaxX;

   if ( SectionMinY < MinY ) MinY = SectionMinY;
   if ( SectionMaxY > MaxY ) MaxY = SectionMaxY;

   if ( SectionMinZ < MinZ ) MinZ = SectionMinZ;
   if ( SectionMaxZ > MaxZ ) MaxZ = SectionMaxZ;
  }

  nglVector Center;
  float Radius;

  Center[0] = MinX + ( MaxX - MinX ) * 0.5f;
  Center[1] = MinY + ( MaxY - MinY ) * 0.5f;
  Center[2] = MinZ + ( MaxZ - MinZ ) * 0.5f;
  Center[3] = 1.0f;

  // Calculate the radius from an arbitrary point on the box.
  float DistX, DistY, DistZ;
  DistX = MinX - Center[0];
  DistY = MinY - Center[1];
  DistZ = MinZ - Center[2];
  Radius = sqrtf( DistX * DistX + DistY * DistY + DistZ * DistZ );

  // Fill out the appropriate mesh entries.
  sceVu0CopyVector( nglScratch->SphereCenter, Center );
  nglScratch->SphereRadius = Radius;
}

void nglSetMeshFlags( u_int Flags )
{
  nglScratch->Flags = Flags | NGLMESH_SCRATCH_MESH | (nglScratch->Flags & NGLMESH_TEMP);
}

u_int nglGetMeshFlags()
{
  return nglScratch->Flags;
}

/*---------------------------------------------------------------------------------------------------------
  Scene Dump API.  This should be able to capture the entire NGL List API.
---------------------------------------------------------------------------------------------------------*/
void nglHostPrintf( u_int File, const char* Format, ... )
{
  static char Work[2048];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  if ( File >= 0 )
    sceWrite( File, Work, strlen( Work ) );
}

u_int nglHostOpen( const char* FileName )
{
  static char Work[256];
  strcpy( Work, "host0:" );
  strcat( Work, FileName );
  return sceOpen( Work, SCE_WRONLY | SCE_TRUNC | SCE_CREAT );
}

void nglHostClose( u_int File )
{
  sceClose( File );
}

u_int nglSceneDumpFile = 0;

void nglSceneDumpStart()
{
  // Start the scene dump if requested.
  if ( DEBUG_ENABLE( DumpSceneFile ) )
  {
    nglSceneDumpFile = nglHostOpen( "scenedump.scene" );
    nglHostPrintf( nglSceneDumpFile, "//\n" );
    nglHostPrintf( nglSceneDumpFile, "// Midnight scene file dump.\n" );
    nglHostPrintf( nglSceneDumpFile, "//\n" );
    nglHostPrintf( nglSceneDumpFile, "\n" );
    nglHostPrintf( nglSceneDumpFile, "CAMERA\n" );
    nglHostPrintf( nglSceneDumpFile, "  ROW1 %f %f %f %f\n", nglCurScene->WorldToView[0][0], nglCurScene->WorldToView[0][1], nglCurScene->WorldToView[0][2], nglCurScene->WorldToView[0][3] );
    nglHostPrintf( nglSceneDumpFile, "  ROW2 %f %f %f %f\n", nglCurScene->WorldToView[1][0], nglCurScene->WorldToView[1][1], nglCurScene->WorldToView[1][2], nglCurScene->WorldToView[1][3] );
    nglHostPrintf( nglSceneDumpFile, "  ROW3 %f %f %f %f\n", nglCurScene->WorldToView[2][0], nglCurScene->WorldToView[2][1], nglCurScene->WorldToView[2][2], nglCurScene->WorldToView[2][3] );
    nglHostPrintf( nglSceneDumpFile, "  ROW4 %f %f %f %f\n", nglCurScene->WorldToView[3][0], nglCurScene->WorldToView[3][1], nglCurScene->WorldToView[3][2], nglCurScene->WorldToView[3][3] );
    nglHostPrintf( nglSceneDumpFile, "ENDCAMERA\n" );
    nglHostPrintf( nglSceneDumpFile, "\n" );
  }
}

void nglSceneDumpEnd()
{
  if ( DEBUG_ENABLE( DumpSceneFile ) )
  {
    nglHostPrintf( nglSceneDumpFile, "\n" );
    nglHostPrintf( nglSceneDumpFile, "ENDSCENE\n" );
    nglHostClose( nglSceneDumpFile );
  }
}

// Scene dump code.
void nglSceneDumpMesh( nglMesh* Mesh, const nglMatrix& LocalToWorld, const nglRenderParams* Params )
{
  nglHostPrintf( nglSceneDumpFile, "\n" );
  nglHostPrintf( nglSceneDumpFile, "MODEL %s\n", Mesh->Name.c_str() );

  if ( Params && ( Params->Flags & NGLP_TINT ) )
    nglHostPrintf( nglSceneDumpFile, "  TINT %f %f %f %f\n", Params->TintColor[0], Params->TintColor[1], Params->TintColor[2], Params->TintColor[3] );

  nglHostPrintf( nglSceneDumpFile, "  ROW1 %f %f %f %f\n", LocalToWorld[0][0], LocalToWorld[0][1], LocalToWorld[0][2], LocalToWorld[0][3] );
  nglHostPrintf( nglSceneDumpFile, "  ROW2 %f %f %f %f\n", LocalToWorld[1][0], LocalToWorld[1][1], LocalToWorld[1][2], LocalToWorld[1][3] );
  nglHostPrintf( nglSceneDumpFile, "  ROW3 %f %f %f %f\n", LocalToWorld[2][0], LocalToWorld[2][1], LocalToWorld[2][2], LocalToWorld[2][3] );
  nglHostPrintf( nglSceneDumpFile, "  ROW4 %f %f %f %f\n", LocalToWorld[3][0], LocalToWorld[3][1], LocalToWorld[3][2], LocalToWorld[3][3] );

  if ( Params && ( Params->Flags & NGLP_BONES_MASK ) )
  {
    nglHostPrintf( nglSceneDumpFile, "  NBONES %d\n", Params->NBones );
    for ( u_int i = 0; i < Params->NBones; i++ )
      nglHostPrintf( nglSceneDumpFile, "  BONE %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", i,
        Params->Bones[i][0][0], Params->Bones[i][0][1], Params->Bones[i][0][2], Params->Bones[i][0][3],
        Params->Bones[i][1][0], Params->Bones[i][1][1], Params->Bones[i][1][2], Params->Bones[i][1][3],
        Params->Bones[i][2][0], Params->Bones[i][2][1], Params->Bones[i][2][2], Params->Bones[i][2][3],
        Params->Bones[i][3][0], Params->Bones[i][3][1], Params->Bones[i][3][2], Params->Bones[i][3][3] );
  }
  nglHostPrintf( nglSceneDumpFile, "ENDMODEL\n" );
}

void nglSceneDumpDirLight( u_int LightCat, const nglVector& Dir, const nglVector& Color )
{
  nglHostPrintf( nglSceneDumpFile, "\n" );
  nglHostPrintf( nglSceneDumpFile, "LIGHT\n" );
  nglHostPrintf( nglSceneDumpFile, "  TYPE DIRECTIONAL\n" );
  for ( u_int i = 0; i < 8; i++ )
    if ( LightCat & ( 1 << ( i + NGL_LIGHTCAT_SHIFT ) ) )
      nglHostPrintf( nglSceneDumpFile, "  LIGHTCAT %d\n", i );
  nglHostPrintf( nglSceneDumpFile, "  DIR %f %f %f\n", Dir[0], Dir[1], Dir[2] );
  nglHostPrintf( nglSceneDumpFile, "  COLOR %f %f %f %f\n", Color[0], Color[1], Color[2], Color[3] );

  nglHostPrintf( nglSceneDumpFile, "ENDLIGHT\n" );
}

void nglSceneDumpPointLight( u_int Type, u_int LightCat, const nglVector& Pos, float Near, float Far, const nglVector& Color )
{
  nglHostPrintf( nglSceneDumpFile, "\n" );
  nglHostPrintf( nglSceneDumpFile, "LIGHT\n" );
  if ( Type == NGLLIGHT_TRUEPOINT )
    nglHostPrintf( nglSceneDumpFile, "  TYPE POINT\n" );
  else
    nglHostPrintf( nglSceneDumpFile, "  TYPE FAKEPOINT\n" );
  for ( u_int i = 0; i < 8; i++ )
    if ( LightCat & ( 1 << ( i + NGL_LIGHTCAT_SHIFT ) ) )
      nglHostPrintf( nglSceneDumpFile, "  LIGHTCAT %d\n", i );
  nglHostPrintf( nglSceneDumpFile, "  POS %f %f %f\n", Pos[0], Pos[1], Pos[2] );
  nglHostPrintf( nglSceneDumpFile, "  RANGE %f %f\n", Near, Far );
  nglHostPrintf( nglSceneDumpFile, "  COLOR %f %f %f %f\n", Color[0], Color[1], Color[2], Color[3] );

  nglHostPrintf( nglSceneDumpFile, "ENDLIGHT\n" );
}

void nglDumpTextures()
{
  u_int file = nglHostOpen( "ngl_texdump.txt" );

  int TotalCount = 0;
  int TotalSize = 0;

  nglInstanceBank::Instance* Inst = nglTextureBank.Head->Forward[0];
  while ( Inst != nglTextureBank.NIL )
  {
    nglTexture* Tex = (nglTexture*)Inst->Value;
    int Size = 0;
    if ( Tex->Type == NGLTEX_TIM2 || Tex->Type == NGLTEX_TGA )
      Size = Tex->SrcDataSize;
    else if ( Tex->Type == NGLTEX_IFL )
    {
      // Size stays 0 for IFLs, as we can't easily track duplicate frames.
//      for ( u_int i = 0; i < Tex->NFrames; i++ )
//        Size += Tex->Frames[i]->ph->TotalSize;
    }
    nglHostPrintf( file, "%s (%d): ", Tex->FileName.c_str(), Size );

    TotalCount++;
    TotalSize += Size;

    nglInstanceBank::Instance* MeshInst = nglMeshBank.Head->Forward[0];
    while ( MeshInst != nglMeshBank.NIL )
    {
      nglMesh* Mesh = (nglMesh*)MeshInst->Value;
      bool Found = false;
      for ( u_int i = 0; i < Mesh->NSections; i++ )
      {
        nglMaterial* Material = Mesh->Sections[i].Material;
        if ( Material->Map == Tex || Material->DetailMap == Tex || Material->LightMap == Tex || Material->EnvironmentMap == Tex )
          Found = true;
      }
      if ( Found )
        nglHostPrintf( file, "%s, ", Mesh->Name.c_str() );
      MeshInst = MeshInst->Forward[0];
    }
    nglHostPrintf( file, "\n" );
    Inst = Inst->Forward[0];
  }

  nglHostPrintf( file, "Total Textures: %d   Total Size: %d\n", TotalCount, TotalSize );
  nglHostClose( file );
}

/*---------------------------------------------------------------------------------------------------------
  Debug rendering API.
---------------------------------------------------------------------------------------------------------*/
void nglDrawDebugSphere( const nglVector& Pos, float Scale, u_int Color, bool Solid )
{
#if NGL_DEBUGGING
  nglMesh* Mesh = Solid ? nglSphereMesh : nglRadiusMesh;
  if ( !Mesh )
    return;

  nglRenderParams Params;

  Params.Flags = NGLP_TINT | NGLP_SCALE;
  nglColorToVector( Params.TintColor, Color );

  Params.Scale[0] = Scale / Mesh->SphereRadius; // divide is there in case the radius isn't exactly 1.0.
  Params.Scale[1] = Scale / Mesh->SphereRadius;
  Params.Scale[2] = Scale / Mesh->SphereRadius;
  Params.Scale[3] = 1.0f;

  nglMatrix Work;
  sceVu0UnitMatrix( SCE_MATRIX(Work) );
  sceVu0TransMatrix( SCE_MATRIX(Work), SCE_MATRIX(Work), (nglVector)Pos );
  nglListAddMesh( Mesh, Work, &Params );
#endif
}

