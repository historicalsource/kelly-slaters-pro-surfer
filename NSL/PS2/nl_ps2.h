/*************************************************************************
 * nl_ps2.h
 *-----------------------------------------------------------------------*
 * PS2 types and utility functions common to next gen libraries
 *-----------------------------------------------------------------------*
 * Treyarch LLC Copyright 2001
 *************************************************************************/

#ifndef NL_PS2_HEADER
#define NL_PS2_HEADER
/*-------------------------------------------------------------------------
  General platform independent types.
-------------------------------------------------------------------------*/
typedef unsigned char   nlUchar;
typedef unsigned char   nlUint8;
typedef unsigned short  nlUint16;
typedef unsigned int    nlUint32;
typedef unsigned long   nlUint64;
typedef signed   char   nlInt8;
typedef          short  nlInt16;
typedef          int    nlInt32;
typedef          long   nlInt64;

/*-------------------------------------------------------------------------
  Vector/Matrix API.

  These are designed to be as simple as possible to pass data into a next 
  gen library and are not for general game use.
-------------------------------------------------------------------------*/

typedef float nlMatrix4x4[4][4] __attribute__((aligned (16)));
typedef float nlVector4d[4] __attribute__((aligned (16)));
typedef float nlVector3d[3];


#define TO_NLVECTOR3D(x) (*( (nlVector3d *) &(x) ) )

void  nlIdentityMatrix( nlMatrix4x4 matrix );
void  nlMatrixMul( nlMatrix4x4 dst, nlMatrix4x4 lhs, nlMatrix4x4 rhs );

float nlDotProduct4d( nlVector4d v0, nlVector4d v1 );
float nlDotProduct3d( nlVector3d v0, nlVector3d v1 );

void  nlCrossProduct4d( nlVector4d dst, nlVector4d lhs, nlVector4d rhs );
void  nlCrossProduct3d( nlVector3d dst, nlVector3d lhs, nlVector3d rhs );

void  nlTransformVector( nlVector4d dst, nlMatrix4x4 m, nlVector4d in );

void nlAddVect3d( nlVector3d & result, const nlVector3d & n1, const nlVector3d & n2 );
void nlSubVect3d( nlVector3d & result, const nlVector3d & n1, const nlVector3d & n2 );
void nlScaleVect3d( nlVector3d & result, const nlVector3d & vec, float fact );

float nlDistancePointSegment( const nlVector3d & p,
                              const nlVector3d & n1, const nlVector3d & n2,
                              nlVector3d & hit_point );

#endif
