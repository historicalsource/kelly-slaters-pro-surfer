#include "ps2_math.h"


#define _DISABLE_FAST_SIN_COS 0

bool internal_fast_sin_cos_approx = false;

void fast_sin_cos_approx( fp rads, fp* sinx, fp* cosx )
{
#if _DISABLE_FAST_SIN_COS
  *sinx = sinf(rads);
  *cosx = cosf(rads);
#else

  static sceVu0IVECTOR _S5432 = { 0x362e9c14, 0xb94fb21f, 0x3c08873e, 0xbe2aaaa4 };
  static sceVu0FVECTOR res;

  static float inv_PI = (float)(1.0f / PI);
  static float my_PI = (float)(PI);

  rational_t muller = 1.0f;
  rational_t orig_rads = rads;

  if(rads >= my_PI || rads <= -my_PI)
  {
    int num = (int)(rads*inv_PI);
    if(num & 0x1)
      muller = -1.0f;

    rads -= (((float)num)*my_PI);
  }

	asm __volatile__(

  "mtc1	$0,$f0 \n"
	"c.olt.s %1,$f0 \n"
	"li.s    $f0,1.57079637050628662109e0	 \n"
	"bc1f    _fcsa_01 \n"
	"add.s   %1,$f0,%1 \n"
	"li 	$7,1 \n"
	"j	_fcsa_02 \n"
  "_fcsa_01: \n"
	"sub.s   %1,$f0,%1 \n"
	"move	$7,$0 \n"
  "_fcsa_02: \n"

	"mfc1    $8,%1 \n"
	"qmtc2    $8,vf6 \n"
	"move	$6,$31 \n"
  
  
	"lqc2	vf05,0x0(%2) \n"
//	"lqc2	vf06,0x0(%0) \n"
	"vmr32.w vf06,vf06 \n"
	"vaddx.x vf04,vf00,vf06 \n"
	"vmul.x vf06,vf06,vf06 \n"
	"vmulx.yzw vf04,vf04,vf00 \n"
	"vmulw.xyzw vf08,vf05,vf06 \n"
	"vsub.xyzw vf05,vf00,vf00 \n"
	"vmulx.xyzw vf08,vf08,vf06 \n"
	"vmulx.xyz vf08,vf08,vf06 \n"
	"vaddw.x vf04,vf04,vf08 \n"
	"vmulx.xy vf08,vf08,vf06 \n"
	"vaddz.x vf04,vf04,vf08 \n"
	"vmulx.x vf08,vf08,vf06 \n"
	"vaddy.x vf04,vf04,vf08 \n"
	"vaddx.x vf04,vf04,vf08 \n"

	"vaddx.xy vf04,vf05,vf04 \n"

	"vmul.x vf07,vf04,vf04 \n"
	"vsubx.w vf07,vf00,vf07 \n"

	"vsqrt Q,vf07w \n"
	"vwaitq \n"

	"vaddq.x vf07,vf00,Q \n"

	"bne	$7,$0,_fcsa_01x \n"
	"vaddx.x vf04,vf05,vf07 \n"
	"b	_fcsa_02x \n"
"_fcsa_01x:	 \n"
	"vsubx.x vf04,vf05,vf07 \n"
"_fcsa_02x:	 \n"

  "sqc2 vf04, 0x0(%0)  \n"

	: 
  : "r" (&res[0]), "f" (rads), "r" (&_S5432[0])
  :"$6","$7","$8","$f0");

  *sinx = res[0]*muller;
  *cosx = res[1]*muller;

  // This fixes the accuracy problem with sin when rads < 0.05f (quaternions)
//  #pragma todo("Make this also handle values close to PI/-PI")
  if(orig_rads < _FAST_SIN_LOOKUP_THRESHOLD && orig_rads > -_FAST_SIN_LOOKUP_THRESHOLD)
  {
    *sinx = fast_sin_lookup_table[(int)(((fabsf(orig_rads) * _FAST_SIN_LOOKUP_THRESHOLD_INV) * _FAST_SIN_LOOKUP_SIZE_F) + 0.5f)];
    if(orig_rads < 0.0f)
      *sinx *= -1.0f;
  }

#endif
}

#define STATIC_TRIG_TABLES	// Saves .66 seconds of load time (dc 11/07/01)

u_int _fast_acos_lookup_table[_FAST_ACOS_LOOKUP_SIZE+1] = {
#include "acostable.txt"
};
#ifdef STATIC_TRIG_TABLES
rational_t *fast_acos_lookup_table = (rational_t *) _fast_acos_lookup_table;
#else
rational_t fast_acos_lookup_table[_FAST_ACOS_LOOKUP_SIZE+1];
#endif

void init_fast_acos_table()
{
#ifndef STATIC_TRIG_TABLES
	static bool table_initted = false;

  if(!table_initted)
  {
    table_initted = true;
    for(int i=0; i<=_FAST_ACOS_LOOKUP_SIZE; ++i)
      fast_acos_lookup_table[i] = acos(lookup_to_cos(i));
  }
#endif
}

u_int _fast_sin_lookup_table[_FAST_SIN_LOOKUP_SIZE+1] = {
#include "sintable.txt"
};
#ifdef STATIC_TRIG_TABLES
rational_t *fast_sin_lookup_table = (rational_t *) _fast_sin_lookup_table;
#else
rational_t fast_sin_lookup_table[_FAST_SIN_LOOKUP_SIZE+1];
#endif

void init_fast_sin_table()
{
#ifndef STATIC_TRIG_TABLES
  static bool table_initted = false;

  if(!table_initted)
  {
    table_initted = true;
    for(int i=0; i<=_FAST_SIN_LOOKUP_SIZE; ++i)
      fast_sin_lookup_table[i] = sin(_FAST_SIN_LOOKUP_THRESHOLD*((float)i / (float)_FAST_SIN_LOOKUP_SIZE));

/*
    for(int i=0; i<=_FAST_SIN_LOOKUP_SIZE; ++i)
      fast_sin_lookup_table[i] = sin(_FAST_SIN_PI_DIV_2*((float)i / (float)_FAST_SIN_LOOKUP_SIZE));
*/
  }
#endif
}
