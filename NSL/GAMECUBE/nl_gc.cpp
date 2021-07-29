/*************************************************************************
 * nl_gc.cpp
 *-----------------------------------------------------------------------*
 * Gamecube types and utility functions for next gen libraries
 *-----------------------------------------------------------------------*
 * Treyarch LLC Copyright 2001
 *************************************************************************/

#include <string.h>
#include <math.h>

#ifndef NSL_NO_ASSERT_H
#include <assert.h>
#endif

#include "nl_gc.h"

nlVector4d NL_ZERO_VECTOR4D = { 0.0f, 0.0f, 0.0f, 0.0f };
nlVector3d NL_ZERO_VECTOR3D = { 0.0f, 0.0f, 0.0f };

void nlIdentityMatrix( nlMatrix4x4 matrix )
{
	matrix[0][0] = 1.0f;
	matrix[0][1] = 0.0f;
	matrix[0][2] = 0.0f;
	matrix[0][3] = 0.0f;

	matrix[1][0] = 0.0f;
	matrix[1][1] = 1.0f;
	matrix[1][2] = 0.0f;
	matrix[1][3] = 0.0f;

	matrix[2][0] = 0.0f;
	matrix[2][1] = 0.0f;
	matrix[2][2] = 1.0f;
	matrix[2][3] = 0.0f;

	matrix[3][0] = 0.0f;
	matrix[3][1] = 0.0f;
	matrix[3][2] = 0.0f;
	matrix[3][3] = 1.0f;
}

void nlMatrixMul( nlMatrix4x4 m, nlMatrix4x4 a, nlMatrix4x4 b )
{
	float a0, a1, a2, a3;

	float b00, b01, b02, b03;
	float b10, b11, b12, b13;
	float b20, b21, b22, b23;
	float b30, b31, b32, b33;

	// compute (a x b) -> m
	a0 = a[0][0];
	a1 = a[0][1];
	a2 = a[0][2];
	a3 = a[0][3];

	b00 = b[0][0];
	b10 = b[1][0];
	b20 = b[2][0];
	b30 = b[3][0];

	b01 = b[0][1];
	b11 = b[1][1];
	b21 = b[2][1];
	b31 = b[3][1];

	b02 = b[0][2];
	b12 = b[1][2];
	b22 = b[2][2];
	b32 = b[3][2];

	b03 = b[0][3];
	b13 = b[1][3];
	b23 = b[2][3];
	b33 = b[3][3];

	m[0][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	m[0][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	m[0][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	m[0][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;

	a0 = a[1][0];
	a1 = a[1][1];
	a2 = a[1][2];
	a3 = a[1][3];

	m[1][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	m[1][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	m[1][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	m[1][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;

	a0 = a[2][0];
	a1 = a[2][1];
	a2 = a[2][2];
	a3 = a[2][3];

	m[2][0] = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
	m[2][1] = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
	m[2][2] = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
	m[2][3] = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;
}

float nlDotProduct4d( nlVector4d v0, nlVector4d v1 )
{
  return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2] + v0[3] * v1[3];
}

float nlDotProduct3d( nlVector3d v0, nlVector3d v1 )
{
  return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

void nlCrossProduct4d( nlVector4d dst, nlVector4d lhs, nlVector4d rhs )
{
	assert( 0 );
}

void nlCrossProduct3d( nlVector3d v, nlVector3d a, nlVector3d b )
{
	float a0, a1, a2;
	float b0, b1, b2;

	a0 = a[0];
	a1 = a[1];
	a2 = a[2];
	
	b0 = b[0];
	b1 = b[1];
	b2 = b[2];	

  v[0] = a1 * b2 - a2 * b1;
  v[1] = a2 * b0 - a0 * b2;
  v[2] = a0 * b1 - a1 * b0;
}

void nlTransformVector( nlVector4d dst, nlMatrix4x4 m, nlVector4d v ) 
{
	float v0, v1, v2, v3;
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;

	v0 = v[0];
	v1 = v[1];
	v2 = v[2];
	v3 = v[3];

	m00 = m[0][0];
	m10 = m[1][0];
	m20 = m[2][0];
	m30 = m[3][0];

	m01 = m[0][1];
	m11 = m[1][1];
	m21 = m[2][1];
	m31 = m[3][1];

	m02 = m[0][2];
	m12 = m[1][2];
	m22 = m[2][2];
	m32 = m[3][2];

	m03 = m[0][3];
	m13 = m[1][3];
	m23 = m[2][3];
	m33 = m[3][3];

	dst[0] = v0 * m00 + v0 * m10 + v0 * m20 + v0 * m30;
	dst[1] = v1 * m01 + v1 * m11 + v1 * m21 + v1 * m31;
	dst[2] = v2 * m02 + v2 * m12 + v2 * m22 + v2 * m32;
	dst[3] = v3 * m03 + v3 * m13 + v3 * m23 + v3 * m33;
}

float _nlVectorLength( nlVector3d v )
{
	return sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
}

void nlNormalize( nlVector3d dst, nlVector3d src )
{
	float length = _nlVectorLength( src );
	assert( length != 0.0f );
	length = 1.0f / length;
	dst[0] = src[0] * length;
	dst[1] = src[1] * length;
	dst[2] = src[2] * length;
}