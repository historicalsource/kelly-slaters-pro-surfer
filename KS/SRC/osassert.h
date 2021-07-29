// This does NOT have #ifdef OSASSERT_H encapsulation because it needs to be included
// multiple times during the compile, to reaffirm our assert after STL bashes it

// The measure in the above comment doesn't help, since we include this header from within
// global.h, which does guard against multiple inclusion.  Instead, I'm trying to make sure
// that this file only gets included after the STL stuff. (dc 07/11/01)

#ifndef OSASSERT_H
#define OSASSERT_H

/*-------------------------------------------------------------------------------------------------------
OSASSERT.H - Include file redirect to standard assertion definition.
-------------------------------------------------------------------------------------------------------*/
#undef assert

#if defined(TARGET_MKS)
// dreamcast is weird
#elif defined(TARGET_PC)

// fancy PC assert
#include "SimpleAssert.h"
#define assert sAssert
#define assert_msg sAssertM
#else
#if !defined(BUILD_FINAL)
extern bool _assert(const char* exp_str, const char* file_name, int line);

#define assertmsg(exp, desc)							\
{														\
	if (!(exp))											\
	{													\
		static bool enabled=true;							\
		if (enabled)										\
		{													\
			enabled = _assert(desc, __FILE__, __LINE__);	\
		}													\
	}													\
}
#define assert(exp) assertmsg(exp, #exp)
#define verify(a) assert(a)
#else
#define assertmsg(exp, desc) ((void)0)
#define assert(exp) ((void)0)
#define verify(a) if (a) {}
#endif
#endif // defined(TARGET_MKS) else

void official_error(const char* fmtp, ...);

#endif	/* OSASSERT_H */
