#ifndef PS2_MATH_H
#define PS2_MATH_H

// The place for CPU-specific niftiness

#include <math.h>
#include <libvu0.h>

typedef float rational_t;  
typedef float angle_t;
typedef float fp;


inline void prefetch(void*) {}


inline fp fast_length2( fp x, fp y, fp z )
{
  return x*x+y*y+z*z;
}

inline fp fast_length( fp x, fp y, fp z )
{
  return (rational_t)sqrtf(x*x+y*y+z*z);
}

inline fp fast_recip_length( fp x, fp y, fp z)
{
  return 1.0f/(rational_t)sqrtf(x*x+y*y+z*z);
}

inline fp fast_distance( fp x1, fp y1, fp z1, fp v2[3] )
{
  fp dx, dy, dz;
  dx = x1 - v2[0];
  dy = y1 - v2[1];
  dz = z1 - v2[2];
  return (rational_t)sqrtf(dx*dx+dy*dy+dz*dz);
}


inline fp fast_recip_sqrt(fp x) 
{
  return 1.0F/(rational_t)sqrtf(x);
}

#define _FAST_ACOS_LOOKUP_SIZE        (4096*4)

#define _FAST_ACOS_LOOKUP_SIZE_DIV2   (_FAST_ACOS_LOOKUP_SIZE / 2)
#define _FAST_ACOS_LOOKUP_MOD         ((float)_FAST_ACOS_LOOKUP_SIZE_DIV2) 
#define _FAST_ACOS_LOOKUP_MOD_INV     (1.0f / _FAST_ACOS_LOOKUP_MOD)  
#define cos_to_lookup(a)              ((int)(((a)+1)*_FAST_ACOS_LOOKUP_MOD))
#define lookup_to_cos(a)              (((float)((a)-_FAST_ACOS_LOOKUP_SIZE_DIV2))*_FAST_ACOS_LOOKUP_MOD_INV)

extern rational_t *fast_acos_lookup_table;

inline rational_t fast_acos(rational_t a)
{
  assert(a >= -1.0f && a <= 1.0f);
  return(fast_acos_lookup_table[(int)((a+1.0f)*_FAST_ACOS_LOOKUP_MOD)]);
}



///////////////////////////////////////////////////////////
#define _FAST_SIN_LOOKUP_SIZE             (4096*4)
#define _FAST_SIN_LOOKUP_SIZE_F           (4096.0f*4.0f)
#define _FAST_SIN_LOOKUP_THRESHOLD        (0.05f)
#define _FAST_SIN_LOOKUP_THRESHOLD_INV    (1.0f / _FAST_SIN_LOOKUP_THRESHOLD)
#define _FAST_SIN_PI                      (3.1415926535897932384626433832795f)
#define _FAST_SIN_PI_DIV_2                (_FAST_SIN_PI*0.5f)
#define _FAST_SIN_INV_PI_DIV_2            (1.0f/_FAST_SIN_PI_DIV_2)

extern rational_t *fast_sin_lookup_table;

void fast_sin_cos_approx( fp rads, fp* sinx, fp* cosx );

inline fp fast_sin( fp rads )
{
  fp sinx, cosx;
  fast_sin_cos_approx( rads, &sinx, &cosx );
  return sinx;
/*
  if(rads > _FAST_SIN_LOOKUP_THRESHOLD || rads < -_FAST_SIN_LOOKUP_THRESHOLD)
  {
    fp sinx, cosx;
    fast_sin_cos_approx( rads, &sinx, &cosx );
    return sinx;
  }
  else
  {
    if(rads > 0.0f)
      return(fast_sin_lookup_table[(int)(((rads * _FAST_SIN_LOOKUP_THRESHOLD_INV) * (float)(_FAST_SIN_LOOKUP_SIZE)) + 0.5f)]);
    else
      return(fast_sin_lookup_table[(int)(((fabsf(rads) * _FAST_SIN_LOOKUP_THRESHOLD_INV) * (float)(_FAST_SIN_LOOKUP_SIZE)) + 0.5f)] * -1.0f);
  }
*/
}

// float to int conversion
inline int FTOI( float input )  // doesn't round according to C standard
{
  register float output;
  __asm__ volatile ("cvt.w.s %0, %1" : "=f" (output) : "f" (input) );
  return (int&)output;
//  f += 3 << 22;
//  int n=((*(int*)&f)&0x007fffff) - 0x00400000;
//  return n;
}


inline fp fast_cos( fp rads )
{
  fp sinx, cosx;
  fast_sin_cos_approx( rads, &sinx, &cosx );
  return cosx;
}


// lookup table goes from 0 -> PI/2 (highest accuracy possible)
inline fp fast_sin_lookup(fp rads)
{
  rational_t muller = (rads < 0.0f) ? -1.0f : 1.0f;

  rads = fabsf(rads);

  if(rads > _FAST_SIN_PI_DIV_2)
  {
    int num = (int)(rads * _FAST_SIN_INV_PI_DIV_2);
    rads -= ((rational_t)num * _FAST_SIN_PI_DIV_2);

    if(num & 0x1)
      rads = _FAST_SIN_PI_DIV_2 - rads;

    if(num & 0x2)
      muller *= -1.0f;
  }

  return(fast_sin_lookup_table[(int)(((rads * _FAST_SIN_INV_PI_DIV_2) * _FAST_SIN_LOOKUP_SIZE_F) + 0.5f)]*muller);
}

inline fp fast_cos_lookup(fp rads)
{
  return(fast_sin(_FAST_SIN_PI_DIV_2 - rads));
}

/*
inline void fast_sin_cos_approx( fp rads, fp* sinx, fp* cosx )
{
  *sinx = fast_sin(rads);
  *cosx = fast_cos(rads);
}
*/

#endif 