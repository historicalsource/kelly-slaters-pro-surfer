#ifndef GC_MATH_H
#define GC_MATH_H

#include <math.h>
#include "osassert.h"

typedef float rational_t;
typedef float angle_t;
typedef float fp;

//
// Warm that cache.
//

inline void prefetch( void* )
{
	// perhaps some other time?
}

static inline int FTOI( float f )  // doesn't round according to C standard
{
	return (int) f;
	#if 0
	int n;

  f += 3 << 22;
  n = ( ( *( (int*) &f ) ) & 0x007fffff ) - 0x00400000;

  return n;
	#endif
}
//
// Length/distance routines.
//

inline fp fast_length2( fp x, fp y, fp z )
{
  return ( x * x ) + ( y * y ) + ( z * z );
}

inline fp fast_length( fp x, fp y, fp z )
{
  return sqrtf( ( x * x ) + ( y * y ) + ( z * z ) );
}

inline fp fast_recip_length( fp x, fp y, fp z )
{
  return 1.0f / sqrtf( ( x * x ) + ( y * y ) + ( z * z ) );
}

inline fp fast_distance( fp x1, fp y1, fp z1, fp v2[3] )
{
  fp dx = x1 - v2[0];
  fp dy = y1 - v2[1];
  fp dz = z1 - v2[2];

  return sqrtf( ( dx * dx ) + ( dy * dy ) + ( dz * dz ) );
}

inline fp fast_recip_sqrt( fp x )
{
  return 1.0f / sqrtf( x );
}

//
// Trig excitement.
//

inline rational_t fast_asin( rational_t a )
{
	return asinf( a );
}

inline rational_t fast_acos(rational_t a)
{
	return acosf( a );
}

inline void fast_sin_cos_approx( fp rads, fp* sinx, fp* cosx )
{

	if( rads < 0.0001f && rads > -0.0001f ) {
		*sinx = 0.0f;
		*cosx = 1.0f;
	} else {
		*sinx = sinf( rads );
		*cosx = cosf( rads );
	}

}

inline fp fast_sin( fp rads )
{
	return sinf( rads );
}

inline fp fast_cos( fp rads )
{
	return cosf( rads );
}

inline fp fast_sin_lookup( fp rads )
{
	return sinf( rads );
}

inline fp fast_cos_lookup(fp rads)
{
  return cosf( rads );
}

#endif
