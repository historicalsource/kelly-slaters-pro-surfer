#ifndef GCGLOBALS_H
#define GCGLOBALS_H

namespace std{}

using namespace std;

struct nglFileBuf;

#include "osassert.h"
#include "kshooks.h"
//extern bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align );
//extern void KSReleaseFile( nglFileBuf* File );

/*	Moved to ksngl.h for centralization.  (dc 05/31/02)
extern void ksNormalize( float &nx, float &ny, float &nz );
*/

extern void gcstub(const char *str, const char *f, int l);
#define STUB(str) gcstub(str, __FILE__, __LINE__)

#endif
