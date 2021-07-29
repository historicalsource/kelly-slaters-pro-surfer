#ifndef NGL_GC_PROFILE_INTERNAL_H
#define NGL_GC_PROFILE_INTERNAL_H
#if !defined(NGL_GC_PROFILE)
#error Must not include the file if NGL_GC_PROFILE is not defined.
#endif
#include "ngl_gc_profile.h"

struct nglGCProfileInfo
{
	int DataScreenNum;
	u_int Color;
	int TotalVertCount;
	int SkinVertCount;
	int NonSkinVertCount;
	int QuadVertCount;
	int TotalTriCount;
	int SkinTriCount;
	int NonSkinTriCount;
	int QuadTriCount;
	int TotalMeshCount;
	int SkinMeshCount;
	int NonSkinMeshCount;
	nglGCProfile GameLoop, NonSkinRend, SkinRend,
		ProfDataRend, ShadowRend, QuadRend,
		MeshAdd, QuadAdd, ListSend, GXRend,
		CPUWait, GXWait, VSyncWait, ListSendWait,
		EnviroMapCoord, SkinCPU, SkinBoneMats,
		SkinDispList, NonSkinDispList, ListPreparation;
};

#define NGL_GC_USE_BIG_BUFFER 0
struct nglGCRenderTest
{
	int SkipBone;
	int SkipScratch;
	int SkipCustom; // Also skips ScratchMeshes.
	int SkipWorld;
	int SkipQuad;
	int DrawAsLines;
	int Test[5];
	int TestVal[5];
	float TestValf[5];
};

extern nglGCProfileInfo nglGCPD;
extern nglGCRenderTest nglGCRT;

void nglGCDebugScreen();
void nglGCProfileDataInit();

//-----------------------------------------------------------------------------

#endif //#ifndef NGL_GC_PROFILE_INTERNAL_H
