/*************************************************************************
 * nl_ps2.cpp
 *-----------------------------------------------------------------------*
 * PS2 types and utility functions for next gen libraries
 *-----------------------------------------------------------------------*
 * Treyarch LLC Copyright 2001
 *************************************************************************/


#include <eekernel.h>
#include <libvu0.h>

#include "nl_ps2.h"


nlVector4d NL_ZERO_VECTOR4D = { 0.0f, 0.0f, 0.0f, 0.0f };
nlVector3d NL_ZERO_VECTOR3D = { 0.0f, 0.0f, 0.0f };



//==========================================================================
// sceVu0ApplyMatrix    Multiplies matrix by vector.

// Syntax
//  void sceVu0ApplyMatrix(sceVu0FVECTOR v0, sceVu0FMATRIX m0, sceVu0FVECTOR v1)
// Arguments
//  v1    Input: vector
//  m0    Input: matrix
//  v0    Output: vector
//
// Description
// Multiplies vector v0 by matrix m from the right, and applies the result to v1.

//
//    v0=m0*v1
// Return Value
//  None
//--------------------------------------------------------------------------

inline void sceVu0ApplyMatrix( sceVu0FVECTOR v0, sceVu0FMATRIX m0, sceVu0FVECTOR v1 )
{
  asm __volatile__ (
    "lqc2    vf4,0x0(%1)  \n "
    "lqc2    vf5,0x10(%1) \n "
    "lqc2    vf6,0x20(%1) \n "
    "lqc2    vf7,0x30(%1) \n "
    "lqc2    vf8,0x0(%2)  \n "
    "vmulax.xyzw	ACC,   vf4,vf8  \n "
    "vmadday.xyzw	ACC,   vf5,vf8  \n "
    "vmaddaz.xyzw	ACC,   vf6,vf8  \n "
    "vmaddw.xyzw	vf9,vf7,vf8     \n "
    "sqc2    vf9,0x0(%0)          \n "
    : : "r" (v0) , "r" (m0) ,"r" (v1)  );
}


//==========================================================================
// sceVu0UnitMatrix	Unit matrix 
// Syntax
//	void sceVu0UnitMatrix(sceVu0FMATRIX m0);
// Arguments
// 	m1		Input: matrix
// 	m0		Output: matrix
// Description
// Converts a given matrix to a unit matrix.
//-------------------------------------------------------------------------- 	
void nslSceVu0UnitMatrix(sceVu0FMATRIX m0)
{
	asm __volatile__("
	vsub.xyzw	vf4,vf0,vf0 #vf4.xyzw=0;
	vadd.w	vf4,vf4,vf0
	vmr32.xyzw	vf5,vf4
	vmr32.xyzw	vf6,vf5
	vmr32.xyzw	vf7,vf6
	sqc2    vf4,0x30(%0)
	sqc2    vf5,0x20(%0)
	sqc2    vf6,0x10(%0)
	sqc2    vf7,0x0(%0)
	": : "r" (m0));
}


//==========================================================================
//
// sceVu0MulMatrix    Finds the product of two matrices.

// Syntax
//  void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2)
// Arguments
//  m1,m2   Input: matrix
//  m0    Output: matrix
// Description
//  Multiplies matrix m2 by matrix m1 from the right, and applies the result to m0.

//
//    m0=m1*m2
// Return Value
//  None
//
//--------------------------------------------------------------------------
inline void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1,sceVu0FMATRIX m2)
{
  asm __volatile__ (
    "lqc2    vf1,0x0(%2)  \n"
    "lqc2    vf2,0x10(%2) \n"
    "lqc2    vf3,0x20(%2) \n"
    "lqc2    vf4,0x30(%2) \n"

    "lqc2    vf5,0x0(%1) \n"
    "lqc2    vf6,0x10(%1) \n"
    "lqc2    vf7,0x20(%1) \n"
    "lqc2    vf8,0x30(%1) \n"

    "vmulax.xyzw	ACC,   vf1,vf5 \n"
    "vmadday.xyzw	ACC,   vf2,vf5 \n"
    "vmaddaz.xyzw	ACC,   vf3,vf5 \n"
    "vmaddw.xyzw	vf9,   vf4,vf5 \n"

    "vmulax.xyzw	ACC,   vf1,vf6 \n"
    "vmadday.xyzw	ACC,   vf2,vf6 \n"
    "vmaddaz.xyzw	ACC,   vf3,vf6 \n"
    "vmaddw.xyzw	vf10,  vf4,vf6 \n"

    "vmulax.xyzw	ACC,   vf1,vf7 \n"
    "vmadday.xyzw	ACC,   vf2,vf7 \n"
    "vmaddaz.xyzw	ACC,   vf3,vf7 \n"
    "vmaddw.xyzw	vf11,  vf4,vf7 \n"

    "vmulax.xyzw	ACC,   vf1,vf8 \n"
    "vmadday.xyzw	ACC,   vf2,vf8 \n"
    "vmaddaz.xyzw	ACC,   vf3,vf8 \n"
    "vmaddw.xyzw	vf12,  vf4,vf8 \n"

    "sqc2    vf9,0x0(%0) \n"
    "sqc2    vf10,0x10(%0) \n"
    "sqc2    vf11,0x20(%0) \n"
    "sqc2    vf12,0x30(%0) \n"
    : : "r" (m0), "r" (m2), "r" (m1) );

/*
  asm __volatile__("
    lqc2    vf4,0x0(%2)
    lqc2    vf5,0x10(%2)
    lqc2    vf6,0x20(%2)
    lqc2    vf7,0x30(%2)
    li    $7,4
.loopMulMatrix%=:
  lqc2    vf8,0x0(%1)
    vmulax.xyzw ACC,   vf4,vf8
    vmadday.xyzw  ACC,   vf5,vf8
    vmaddaz.xyzw  ACC,   vf6,vf8
    vmaddw.xyzw vf9,vf7,vf8
    sqc2    vf9,0x0(%0)
    addi    $7,-1
    addi    %1,0x10
    addi    %0,0x10
    bne    $0,$7,.loopMulMatrix%=
    ": : "r" (m0), "r" (m2), "r" (m1) : "$7");
*/
}

//==========================================================================
//
// sceVu0InnerProduct   Finds the inner product of two vectors.
// Syntax
//  float sceVu0InnerProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
// Arguments
//  v0,v1   Input: vector
// Description
//  Finds the inner product of vectors v0 and v1.
// Return Value
//  Inner product
//
//--------------------------------------------------------------------------
inline float sceVu0InnerProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
{
  register float ret;

  asm __volatile__("
    lqc2    vf4,0x0(%1)
    lqc2    vf5,0x0(%2)
    vmul.xyz vf5,vf4,vf5
    vaddy.x vf5,vf5,vf5
    vaddz.x vf5,vf5,vf5
    qmfc2   $2 ,vf5
    mtc1    $2,%0
    ": "=f" (ret) : "r" (v0), "r" (v1) : "$2" );
    return ret;
}

//==========================================================================
// 
// sceVu0OuterProduct		Finds the outer product of two vectors.
// Syntax
// 	void sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
// Arguments
// 	v1,v2		Input: vector
// 	v0		Output: vector
// Description
// 	Finds the outer product of vectors v1 and v2, and applies the result to v0.
//-------------------------------------------------------------------------- 	
inline void sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
	asm __volatile__
	("
	lqc2    vf4,0x0(%1)
	lqc2    vf5,0x0(%2)
	vopmula.xyz	ACC,vf4,vf5
	vopmsub.xyz	vf6,vf5,vf4
	vsub.w vf6,vf6,vf6		#vf6.xyz=0;
	sqc2    vf6,0x0(%0)
	": : "r" (v0) , "r" (v1) ,"r" (v2));
}


/*-------------------------------------------------------------------------
  nlIdentityMatrix
-------------------------------------------------------------------------*/
void nlIdentityMatrix( nlMatrix4x4 matrix )
{
  nslSceVu0UnitMatrix( matrix );
}


/*-------------------------------------------------------------------------
  nlMatrix4x4Mul
-------------------------------------------------------------------------*/
void nlMatrixMul( nlMatrix4x4 dst, nlMatrix4x4 lhs, nlMatrix4x4 rhs )
{
  sceVu0MulMatrix( dst, lhs, rhs );
}


/*-------------------------------------------------------------------------
  nlDotProduct 
-------------------------------------------------------------------------*/
float nlDotProduct4d( nlVector4d v0, nlVector4d v1 )
{
  return sceVu0InnerProduct( v0, v1 );
}

/*-------------------------------------------------------------------------
  nlDotProduct 
-------------------------------------------------------------------------*/
float nlDotProduct3d( nlVector3d v0, nlVector3d v1 )
{
  return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}


/*-------------------------------------------------------------------------
  nlCrossProduct
-------------------------------------------------------------------------*/
void nlCrossProduct4d( nlVector4d dst, nlVector4d lhs, nlVector4d rhs )
{
  sceVu0OuterProduct( dst, lhs, rhs );
}

/*-------------------------------------------------------------------------
  nlCrossProduct
-------------------------------------------------------------------------*/
void nlCrossProduct3d( nlVector3d dst, nlVector3d lhs, nlVector3d rhs )
{
  dst[0] =    lhs[1] * rhs[2] - lhs[2] * rhs[1];
  dst[1] =    lhs[2] * rhs[0] - lhs[0] * rhs[2];
  dst[2] =    lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

/*-------------------------------------------------------------------------
  nlTransformVector
-------------------------------------------------------------------------*/
void nlTransformVector(nlVector4d dst, nlMatrix4x4 m, nlVector4d in) 
{
  sceVu0ApplyMatrix(dst, m, in);
}

void nlAddVect3d( nlVector3d & result, const nlVector3d & n1, const nlVector3d & n2 )
{
  result[0] = n1[0] + n2[0];
  result[1] = n1[1] + n2[1];
  result[2] = n1[2] + n2[2];
}

void nlSubVect3d( nlVector3d & result, const nlVector3d & n1, const nlVector3d & n2 )
{
  result[0] = n1[0] - n2[0];
  result[1] = n1[1] - n2[1];
  result[2] = n1[2] - n2[2];
}

void nlScaleVect3d( nlVector3d & result, const nlVector3d & vec, float fact )
{
  result[0] = vec[0] * fact;
  result[1] = vec[1] * fact;
  result[2] = vec[2] * fact;
}

// stolen and ported from the arch-engine collide.cpp function, included in comments below for reference
float nlDistancePointSegment( const nlVector3d & p,
                              const nlVector3d & n1, const nlVector3d & n2,
                              nlVector3d & hit_point )
{
  nlVector3d n;
  nlSubVect3d( n, n2, n1 );

  float n_len = nlDotProduct3d(n, n);
  if (n_len > 0.0f)
    n_len = sqrtf(n_len);

  // cover the zero length segment case
  if (n_len > 0.0f)
  {
    nlScaleVect3d( n, n, 1.0f / n_len );
  }

  nlVector3d c1;
  nlSubVect3d( c1, p, n1 );

  float ofs = nlDotProduct3d( c1, n );
  if (ofs < 0.0f) 
    ofs = 0.0f;
  else if (ofs > n_len) 
    ofs = n_len;

  nlScaleVect3d( hit_point, n, ofs );
  nlAddVect3d( hit_point, n1, hit_point );

  nlVector3d temp_vec;
  nlSubVect3d( temp_vec, hit_point, p );
  float ret_val = nlDotProduct3d( temp_vec, temp_vec );
  if (ret_val > 0.0f)
    ret_val = sqrtf(ret_val);

  return ret_val;
}
/*
rational_t dist_point_segment( const vector3d& p,
                               const vector3d& n1, const vector3d& n2,
                               vector3d& hit_point )
{
  vector3d n = n2-n1;
  rational_t n_len = n.length();
  // cover the zero length segment case
  if (n_len > 0.0f)
    n /= n_len;

  vector3d c1 = p - n1;

  rational_t ofs = dot(c1,n);
  if (ofs<0.0f) ofs = 0.0f;
  else if (ofs>n_len) ofs = n_len;

  hit_point = n1+ofs*n;

  return (hit_point-p).length();
}
*/