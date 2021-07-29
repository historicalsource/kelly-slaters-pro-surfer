#ifndef GC_ERRMSG_H
#define GC_ERRMSG_H

#include "debug.h"

// Bring up a dialog box to warn the user.
void warning(const stringx& str);

// Error message that halts execution
void error(const stringx& str);

void warning(const char* fmtp, ...);
void error(const char* fmtp, ...);

// output in a dialog box (compatibility)
inline void os_output_string(const char* str) { warning(stringx(str)); }

#endif
