#if defined(NGL_GC_PROFILE)
//-----------------------------------------------------------------------------
// NGL_GC PROFILE CODE
//-----------------------------------------------------------------------------
#include "ngl_gc.h"
#include "ngl_gc_internal.h"
#include "ngl_gc_profile_internal.h"

nglGCProfile nglGCProfiles[30]; // Some General purpose Profile variables.

//-----------------------------------------------------------------------------

// Constructor.
nglGCProfile::nglGCProfile()
{
	Reset();
}

//-----------------------------------------------------------------------------

// Destructor.
nglGCProfile::~nglGCProfile()
{
}

//-----------------------------------------------------------------------------

// Set Start Val.
void nglGCProfile::Start()
{
	StartVal=OSGetTime();
}

//-----------------------------------------------------------------------------

// Set End Val.
void nglGCProfile::End()
{
	EndVal=OSGetTime();
	Val=EndVal-StartVal;
}

//-----------------------------------------------------------------------------

// Set End Val and add to total count.
void nglGCProfile::Add()
{
	EndVal=OSGetTime();
  Val+=EndVal-StartVal;
 	Count++;
}

//-----------------------------------------------------------------------------

// Return count.
u_int nglGCProfile::GetCount()
{
	return(Count);
}

//-----------------------------------------------------------------------------

// Return Val.
OSTime nglGCProfile::GetVal()
{
	return(Val);
}

//-----------------------------------------------------------------------------

// Return Val in MicroSecs.
float nglGCProfile::MicroSecs()
{
	return(OSTicksToMicroseconds((float)Val));
}

//-----------------------------------------------------------------------------

// Return Val in MilSecs.
float nglGCProfile::MilSecs()
{
	return(OSTicksToMilliseconds((float)Val));
}

//-----------------------------------------------------------------------------

// Reset the profile information.
void nglGCProfile::Reset()
{
	StartVal = EndVal = OSGetTime();
	Count=Val=0;
}

//-----------------------------------------------------------------------------
// NGL_GC DEBUG DATA DISPLAY
//-----------------------------------------------------------------------------

nglGCProfileInfo nglGCPD; // Short for nglGCProfileData.
nglGCRenderTest nglGCRT;

void nglGCProfileDataInit()
{
	nglGCPD.DataScreenNum = 0; // Initialize the ScreenNum.
	nglGCPD.Color = 0xEEEEEEFF;
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen1();
static void nglGCDebugScreen2();
static void nglGCDebugScreen3();
static void nglGCDebugScreen4();
static void nglGCDebugScreen5();
static void nglGCDebugScreen6();
extern nglTexture *nglGCDebugScrTex;
extern void nglGetFontScale( float *ScaleX, float *ScaleY );
//extern int nglGCForceOpaqueString;

void nglGCDebugScreen()
{
	float OrigScaleX, OrigScaleY;
	
	nglGCPD.TotalVertCount = nglGCPD.SkinVertCount + nglGCPD.NonSkinVertCount + nglGCPD.QuadVertCount;
	nglGCPD.TotalTriCount = nglGCPD.SkinTriCount + nglGCPD.NonSkinTriCount + nglGCPD.QuadTriCount;
	nglGCPD.TotalMeshCount = nglGCPD.SkinMeshCount + nglGCPD.NonSkinMeshCount;
	nglGCPD.GameLoop.End();
	nglGCPD.GameLoop.Start();
	nglGCPD.ProfDataRend.Start();
	nglListBeginScene();
	extern void nglGCSetSceneLayer( int Type );
	nglGCSetSceneLayer( NGL_SCENETYPE_DEBUG_LAYER ); // <<<< 11-Apr-2002 SAL: This will require that this file be included in ngl_gc.cpp (the way it is done currently).
	nglSetClearFlags( NGLCLEAR_Z );
	nglSetPerspectiveMatrix( 90, 320, 240, 0.2, 10000, 0.0f, 0.0f ); // Only reason to have this is to set zmin and zmax to '0'.
	switch(nglGCPD.DataScreenNum)
	{
	case 1:
		nglGCDebugScreen1(); // Profile 1.
		break;
	case 2:
		nglGCDebugScreen2(); // Total Counts.
		break;
	case 3:
		nglGCDebugScreen3();
		break;
	case 4:
		nglGCDebugScreen4();
		break;
	case 5:
		nglGCDebugScreen5();
		break;
	case 6:
		nglGCDebugScreen6();
		break;
	case 7: // FrameRate activated.
		break;
	default:
		break;
	}
	nglListEndScene();
	nglGCPD.ProfDataRend.End();
	nglGCPD.TotalVertCount = nglGCPD.SkinVertCount =
		nglGCPD.NonSkinVertCount = nglGCPD.QuadVertCount =
		nglGCPD.TotalTriCount = nglGCPD.SkinTriCount =
		nglGCPD.NonSkinTriCount = nglGCPD.QuadTriCount =
		nglGCPD.TotalMeshCount = nglGCPD.SkinMeshCount =
		nglGCPD.NonSkinMeshCount = 0;
	nglGCPD.MeshAdd.Reset();
	nglGCPD.QuadAdd.Reset();
	nglGCPD.NonSkinRend.Reset();
	nglGCPD.SkinRend.Reset();
	nglGCPD.QuadRend.Reset();
	nglGCPD.EnviroMapCoord.Reset();
	nglGCPD.SkinDispList.Reset();
	nglGCPD.NonSkinDispList.Reset();
	nglGCPD.SkinCPU.Reset();
	nglGCPD.SkinBoneMats.Reset();
	for( int i = 0; i < 12; i++)
	{
		nglGCProfiles[i].Reset();
	}
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen1()
{
	int XPos=20,YPos=50,YInc=14,XInc=10;
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "Debug Screen 1" );
	YPos+=YInc;
	YPos+=YInc;
	// GameLoop.
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "> %6.3f ms GameLoop", nglGCPD.GameLoop.MilSecs() );
	YPos+=YInc;
	{
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms ListPreparation", nglGCPD.ListPreparation.MilSecs() );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms ListSend", nglGCPD.ListSend.MilSecs() );
		YPos+=YInc;
		{
			nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>> %6.3f ms SkinRend", nglGCPD.SkinRend.MilSecs() );
			YPos+=YInc;
			{
				nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>>> %6.3f ms SkinCPU", nglGCPD.SkinCPU.MilSecs() );
				YPos+=YInc;
				nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>>> %6.3f ms SkinBoneMats", nglGCPD.SkinBoneMats.MilSecs() );
				YPos+=YInc;
				nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>>> %6.3f ms SkinDispList", nglGCPD.SkinDispList.MilSecs() );
				YPos+=YInc;
			}
			nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>> %6.3f ms NonSkinRend", nglGCPD.NonSkinRend.MilSecs() );
			YPos+=YInc;
			{
				nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>>> %6.3f ms NonSkinDispList", nglGCPD.NonSkinDispList.MilSecs() );
				YPos+=YInc;
				YPos+=YInc;
				// EnvMapCoord CPU computation time.
				nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>>> %6.3f ms EnviroMapCoord", nglGCPD.EnviroMapCoord.MilSecs() );
				YPos+=YInc;
			}
		}
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms GXWait", nglGCPD.GXWait.MilSecs() );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms CPUWait", nglGCPD.CPUWait.MilSecs() );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms LSendWait", nglGCPD.ListSendWait.MilSecs() );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">>> %6.3f ms VSyncWait", nglGCPD.VSyncWait.MilSecs() );
		YPos+=YInc;
	}
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen2()
{
	int XPos=20,YPos=50,YInc=14,XInc=10;
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "Debug Screen 2" );
	YPos+=YInc;
	YPos+=YInc;
	// Vert Counts.
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "> %d TotalVerts", nglGCPD.TotalVertCount );
	YPos+=YInc;
	{
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %d SkinVerts", nglGCPD.SkinVertCount );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %d NonSkinVerts", nglGCPD.NonSkinVertCount );
		YPos+=YInc;
	}
	// Tri Counts.
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "> %d TotalTris", nglGCPD.TotalTriCount );
	YPos+=YInc;
	{
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %d SkinTris", nglGCPD.SkinTriCount );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %d NonSkinTris", nglGCPD.NonSkinTriCount );
		YPos+=YInc;
	}
	// Mesh Counts.
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "> %d TotalMeshs", nglGCPD.TotalMeshCount );
	YPos+=YInc;
	{
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %d SkinMeshs", nglGCPD.SkinMeshCount );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %d NonSkinMeshs", nglGCPD.NonSkinMeshCount );
		YPos+=YInc;
	}
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen3()
{
	int XPos=20,YPos=50,YInc=14,XInc=10;
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "Debug Screen 3" );
	YPos+=YInc;
	YPos+=YInc;
	for( int i = 0; i < 12; i++)
	{
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "> %6.3f ms Profile %d; C %d", nglGCProfiles[i].MilSecs(),
			i, nglGCProfiles[i].GetCount() );
		YPos+=YInc;
	}
	// GameLoop.
	nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, "> %6.3f ms GameLoop", nglGCPD.GameLoop.MilSecs() );
	YPos+=YInc;
	{
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms ListPreparation", nglGCPD.ListPreparation.MilSecs() );
		YPos+=YInc;
		nglListAddString( nglSysFont, XPos, YPos, 0, nglGCPD.Color, ">> %6.3f ms ListSend", nglGCPD.ListSend.MilSecs() );
		YPos+=YInc;
	}
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen4()
{
	if(!nglGCDebugScrTex)
		return;
	nglQuad q;
	nglInitQuad( &q ); // Will also set Z to be nearly at near plane.
	nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex( &q, nglGCDebugScrTex );
	nglSetQuadRect( &q, 350, 10, 630, 172 );
	nglSetQuadColor( &q, 0xFFFFFFFF );
	//nglSetQuadZ( &q, 1.0f );
	nglSetQuadBlend( &q, NGLBM_BLEND, 0 );
	nglListAddQuad( &q );
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen5()
{
	// Test the font system.
	float x = 50.0f;
	float y = 150.0f;
	const char Text[] = "Hello \2[1.5]\1[0000ffff]Mon \2[5.0]\1[ff0000ff]Biquet \2[1]\1[ffffffff]pipo\n\1[00ff00ff]Comment ca va ?\n\1[ffff00ff]Esta bien...";
	nglListAddString(nglSysFont, x, y, 0.0f, 0xFFFF00FF, Text);
}

//-----------------------------------------------------------------------------

static void nglGCDebugScreen6()
{
}

//-----------------------------------------------------------------------------

#endif //#if defined(NGL_GC_PROFILE)
