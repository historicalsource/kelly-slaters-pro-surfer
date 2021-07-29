#ifndef NGL_GC_PROFILE_H
#define NGL_GC_PROFILE_H
#if !defined(NGL_GC_PROFILE)
#error Must not include the file if NGL_GC_PROFILE is not defined.
#endif

class nglGCProfile
{
public:
	nglGCProfile();
	~nglGCProfile();
	void Start();
	void End();
	void Add();
	void Reset();
	OSTime GetVal();
	u_int GetCount();
	float MicroSecs();
	float MilSecs();
protected:
	OSTime StartVal,EndVal,Val;
	u_int Count;
};

extern nglGCProfile nglGCProfiles[30];

//-----------------------------------------------------------------------------

#endif //#ifndef NGL_GC_PROFILE_H
