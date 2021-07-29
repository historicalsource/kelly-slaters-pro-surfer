#include "nsl_xbox_ext.h"
#include <math.h>


void nlVector3dAdd( nlVector3d &result, nlVector3d &v1, nlVector3d &v2 )
{
	result[0] = v1[0] + v2[0];
	result[1] = v1[1] + v2[1];
	result[2] = v1[2] + v2[2];
}

void nlVector3dSub( nlVector3d &result, nlVector3d &v1, nlVector3d &v2 )
{
	result[0] = v1[0] - v2[0];
	result[1] = v1[1] - v2[1];
	result[2] = v1[2] - v2[2];
}

float nlVector3dLength( nlVector3d &v )
{
	return( sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]+v[2]) );
}

float nlVector3dLength2( nlVector3d &v )
{
	return( v[0]*v[0]+v[1]*v[1]+v[2]+v[2] );
}

