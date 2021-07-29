
#ifndef INCLUDED_COORDS_H
#define INCLUDED_COORDS_H

#include "global.h"
#include "ngl.h"

#ifdef TARGET_XBOX
extern int32 nglGetScreenWidthTV();
extern int32 nglGetScreenHeightTV();
extern int32 nglGetScreenXOffsetTV();
extern int32 nglGetScreenYOffsetTV();
#else
#define nglGetScreenWidthTV nglGetScreenWidth
#define nglGetScreenHeightTV nglGetScreenHeight
inline int32 nglGetScreenXOffsetTV() {return 0;}
inline int32 nglGetScreenYOffsetTV() {return 0;}
#endif

// Converts from 640x480 coordinates to hardware-specific coordinates.
template <class T> void adjustSizes(T &x, T &y)
{
	x = (T) (x*(nglGetScreenWidthTV()/640.f));
	y = (T) (y*(nglGetScreenHeightTV()/480.f));
}

// Converts from hardware-specific coordinates to 640x480 coordinates.
template <class T> void unadjustSizes(T &x, T &y)
{
	x = (T) (x*(640.f/nglGetScreenWidthTV()));
	y = (T) (y*(480.f/nglGetScreenHeightTV()));
}

// Converts from 640x480 coordinates to hardware-specific coordinates.
template <class T> void adjustCoords(T &x, T &y)
{
	x = (T) (x*(nglGetScreenWidthTV()/640.f)+nglGetScreenXOffsetTV());
	y = (T) (y*(nglGetScreenHeightTV()/480.f)+nglGetScreenYOffsetTV());
}

// Converts from hardware-specific coordinates to 640x480 coordinates.
template <class T> void unadjustCoords(T &x, T &y)
{
	x = (T) ((x-nglGetScreenXOffsetTV())*640.f/nglGetScreenWidthTV());
	y = (T) ((y-nglGetScreenYOffsetTV())*480.f/nglGetScreenHeightTV());
}

#endif INCLUDED_COORDS_H