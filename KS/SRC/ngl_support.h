#ifndef NGL_SUPPORT_H
#define NGL_SUPPORT_H

#ifdef NGL

#include "algebra.h"

inline const nglMatrix &native_to_ngl(const po &po_in)
{
	return *( (nglMatrix *) &po_in );
}

inline nglMatrix &native_to_ngl(po &po_in)
{
	return *( (nglMatrix *) &po_in );
}

inline const nglMatrix &native_to_ngl(const matrix4x4 &mat_in)
{
	return *( (nglMatrix *) &mat_in );
}

inline nglMatrix &native_to_ngl(matrix4x4 &mat_in)
{
	return *( (nglMatrix *) &mat_in );
}

#endif // NGL

#endif // NGL_SUPPORT_H
