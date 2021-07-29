#ifndef _XBGLOBALS_H_
#define _XBGLOBALS_H_

#ifndef _XBOX
#error "This should only be included for xbox builds"
#endif /* _XBOX */

/* Let's get cute! */
//#define _INC_MALLOC
#include <xtl.h>
#include <xgraphics.h>

typedef unsigned __int64 uint64;

#define __PRETTY_FUNCTION__ "some_xbox_func"

/* FIXME: */
extern void nglPrintf(const char *format, ...);
extern void xbstub(const char *str, const char *f, int l);

#define STUB(str) xbstub(str, __FILE__, __LINE__)

// mismatched destructor: not exception safe
#define DONT_USE_SCRATCHPAD_FOR_STACK 1

typedef float rational_t;

extern void console_log(const char *format, ...);

/* aren't there in a header or something? */
struct nglFileBuf;

extern bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align );
extern void KSReleaseFile( nglFileBuf* File );

/*	Moved to ksngl.h for centralization.  (dc 05/31/02)
extern void ksNormalize( float &nx, float &ny, float &nz );
*/

#endif /* _XBGLOBALS_H_ */
