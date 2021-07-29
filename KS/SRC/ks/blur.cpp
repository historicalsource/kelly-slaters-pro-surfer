// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "blur.h"
#include "game.h"	// For g_game_ptr
#include "timer.h"	// For TIMER_GetTotalSec()
#include "ngl.h"	// For nglListBeginScene()
#include "ksngl.h"

static bool BLUR_Active = false;
static float BLUR_StartTime;
static float BLUR_Duration = 1.f;	// in seconds
static float BLUR_Alpha = 1.f;

#ifndef TARGET_XBOX
u_int RoundUpToPowerOf2(u_int a)
{
	static const u_int check[5] = {
		0xffff0000,
		0xff00ff00,
		0xf0f0f0f0,
		0xcccccccc,
		0xaaaaaaaa,
	};
	register u_int b;

	--a;
	if ((b = (check[0] & a)) != 0) a = b;
	if ((b = (check[1] & a)) != 0) a = b;
	if ((b = (check[2] & a)) != 0) a = b;
	if ((b = (check[3] & a)) != 0) a = b;
	if ((b = (check[4] & a)) != 0) a = b;
	a <<= 1;

	return a;
}
#endif

void BLUR_Init(void)
{
	BLUR_TurnOff();
}

void BLUR_Draw(void)
{

	// These magic numbers get the PS2 to be blur free for the IGO's
	static float MAGICx1=-.55,MAGICy1=-.55, MAGICx2=.55, MAGICy2=.5;
	// no access to front buffer on GC
	if (BLUR_Active && !g_game_ptr->is_paused())
	{
		float progress = (TIMER_GetTotalSec() - BLUR_StartTime) / BLUR_Duration;
		if (progress >= 1)
		{
			BLUR_Active = false;
		}
#if defined(TARGET_PS2)
		float alpha = BLUR_Alpha * (1 - progress / 2) * 128;
#else
		float alpha = BLUR_Alpha * (1 - progress / 2) * 255;
#endif

#ifdef TARGET_GC
#if (NGL > 0x010700)
		#warning "need reimplementation of motion blur that doesn't suck -- mdm"
#else
    nglGCMotionBlur( NGL_RGBA32( 255, 255, 255, alpha ) );
#endif
#else
		nglListBeginScene();
		nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
		float zmax = 1;
		nglTexture *Tex = nglGetFrontBufferTex();
		nglQuad q;
		nglInitQuad(&q);
		nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
		nglSetQuadTex(&q, Tex);
		nglSetQuadColor(&q, NGL_RGBA32(255, 255, 255, 255));
#ifdef TARGET_PS2
		nglSetQuadRect(&q, MAGICx1 , MAGICy1, RoundUpToPowerOf2(nglGetScreenWidth())-MAGICx2, RoundUpToPowerOf2(nglGetScreenHeight())-MAGICy2);
#else
		nglSetQuadRect(&q, 0, 0, nglGetScreenWidth(), nglGetScreenHeight());
#endif
		nglSetQuadZ(&q, zmax);
		nglSetQuadUV(&q, 0, 0, 1, 1);
		nglSetQuadBlend(&q, NGLBM_CONST_BLEND, FTOI(alpha));
		nglListAddQuad(&q);
		nglListEndScene();
#endif	//TARGET_GC
	}
}

void BLUR_TurnOn(void)
{
	BLUR_Active = true;
	BLUR_StartTime = TIMER_GetTotalSec();
}

void BLUR_TurnOff(void)
{
	BLUR_Active = false;
}
