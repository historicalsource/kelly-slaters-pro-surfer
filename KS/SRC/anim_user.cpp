// anim_user.cpp
#include "global.h"

#include "linear_anim.h"
#include "algebra.h"
#include "quatcomp.h"

#include <limits.h>

///////////////////////////////////////////////////////////////////////////////
// This file provides required function definitions for animation template
// classes used by this project.
///////////////////////////////////////////////////////////////////////////////


rational_t linear_key<rational_t>::interpolate( const linear_key<rational_t>& b, rational_t r ) const
{
  return get_value() * (1.0f - r) + b.get_value() * r;
}


vector3d linear_key<vector3d>::interpolate( const linear_key<vector3d>& b, rational_t r ) const
{
  return get_value() * (1.0f - r) + b.get_value() * r;
}


quaternion linear_key<quaternion>::interpolate( const linear_key<quaternion>& b, rational_t r ) const
{
  return slerp( get_value(), b.get_value(), r );
}

extern void qc_to_quat( const quatcomp& src, quaternion* dest );

/*
void quat_to_qc( const quaternion& src, quatcomp* dest )
{
  assert( src.a < 1.0 );
  assert( src.b < 1.0 );
  assert( src.c < 1.0 );
  assert( src.d < 1.0 );
  dest->a = src.a * SHRT_MAX;
  dest->b = src.b * SHRT_MAX;
  dest->c = src.c * SHRT_MAX;
  dest->d = src.d * SHRT_MAX;
}
quatcomp linear_key<quatcomp>::interpolate( const linear_key<quatcomp>& , rational_t ) const
{
  assert(0);
  // we get a linker error if this bad function is deleted

	quatcomp blah;
	return blah;
}
*/
