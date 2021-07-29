#ifndef USEFULMATH_H
#define USEFULMATH_H
////////////////////////////////////////////////////////////////////////////////
/*
  usefulmath.h

  useful math that doesn't really belong in any other file
*/
////////////////////////////////////////////////////////////////////////////////
#include "types.h"

inline float sqr(float x) // there is no speed reason for this, it's just for convenience
{
  return x*x;
}

inline int sgn(float x)
{
  return (x<0)?-1:((x>0)?1:0);
}

inline bool is_power_of_2(uint32 x) 
{
  return x && !(x & (x-1));
}

inline float range( float v, float mn, float mx )
{
  if ( v < mn ) return mn;
  if ( v > mx ) return mx;
  return v;
}

const angle_t SMALL_ANGLE = 0.00001f;
const rational_t SMALL_DIST  = 0.00001f;
const rational_t LARGE_DIST  = 10.0e+8f;

const angle_t PI = 3.1415927f;
const angle_t _2PI = PI*2.0f;

#if !defined(__GNUC__)
inline uint32 log2(uint32 v)
{
	uint32 l = 1;
	while (v >= 2)
  {
		v >>= 1;
		++l;
	}
	return l;
}

inline uint32 log10(uint32 v)
{
	uint32 l = 1;
	while (v >= 10) 
  {
		v /= 10;
		++l;
	}
	return l;
}
#endif // !__GNUC__

// Common intrinsic functions supported by some platforms
#ifdef TARGET_PS2
  #define __fsqrt(f) sqrtf(f)
  #define __fabs(f)  fabsf(f)
#else
  #define __fsqrt(f) sqrt(f)
  #define __fabs(f)  fabs(f)
#endif

#ifdef TARGET_PS2

  #include "hwosps2\ps2_math.h"

  inline rational_t safe_atan2(rational_t x, rational_t y)
  {
    rational_t res = 0.0f;

    if(x != 0.0f || y != 0.0f)
    {
      res = (x*x) + (y*y);
      res = __fsqrt(res);
      res = y / res;

      res = fast_acos_lookup_table[(int)((res+1.0f)*_FAST_ACOS_LOOKUP_MOD)];

      if(x < 0.0f)
        res = -res;
    }

    return(res);
  }

#else

  inline rational_t safe_atan2(rational_t x, rational_t y)
  {
    return (x==0 && y==0) ? 0.0f : (rational_t)atan2(x,y);
  }

#endif


#define EPSILONF 0.00005f

inline bool is_zero(rational_t v) { return (__fabs(v) < EPSILONF); }

inline rational_t upper_quadratic( const rational_t a, const rational_t b, const rational_t c )
{
  assert(a!=0);

  rational_t discriminant = b*b-4*a*c;

  assert (discriminant>0);

  return ( -b+(rational_t)__fsqrt(discriminant) ) / (2*a);
}

inline rational_t lower_quadratic( const rational_t a, const rational_t b, const rational_t c )
{
  assert(a!=0);

  rational_t discriminant = b*b-4*a*c;

  assert (discriminant>0);

  return ( -b-(rational_t)__fsqrt(discriminant) ) / (2*a);
}

// these don't specify the source units... I'd rather use macros that did. --Sean
// #define _TO_RADIANS(a)    ((a) * (PI / 180.0f))
// #define _TO_DEGREES(a)    ((a) * (180.0f / PI))

#define DEG_TO_RAD(a)    ((a) * (PI / 180.0f))
#define RAD_TO_DEG(a)    ((a) * (180.0f / PI))
#define DEG_TO_UNIT(a)   ((a) * (0.5f / 180.0f))
#define UNIT_TO_DEG(a)   ((a) * (180.0f / 0.5f))
#define UNIT_TO_RAD(a)   ((a) * (PI / 0.5f))
#define RAD_TO_UNIT(a)   ((a) * (0.5f / PI))


#endif  // USEFULMATH_H
