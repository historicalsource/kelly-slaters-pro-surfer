
#include "global.h"
#include "nglcvt.h"


void plane2nglPlane( const plane &ip, nglPlane &op )
{
	op[0]=ip.unit_normal[0];
	op[1]=ip.unit_normal[1];
	op[2]=ip.unit_normal[2];
	op[3]=ip.odistance;
}

void frustum2nglFrustum( const frustum &ifr, nglFrustum &ofr )
{
	plane2nglPlane(ifr.top,   ofr.Planes[NGLFRUSTUM_TOP]);
	plane2nglPlane(ifr.bottom,ofr.Planes[NGLFRUSTUM_BOTTOM]);
	plane2nglPlane(ifr.left,  ofr.Planes[NGLFRUSTUM_LEFT]);
	plane2nglPlane(ifr.right, ofr.Planes[NGLFRUSTUM_RIGHT]);
	plane2nglPlane(ifr.front, ofr.Planes[NGLFRUSTUM_NEAR]);
	plane2nglPlane(ifr.back,  ofr.Planes[NGLFRUSTUM_FAR]);
}













