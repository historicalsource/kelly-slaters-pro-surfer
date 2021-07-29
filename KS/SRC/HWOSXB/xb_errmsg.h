#ifndef XB_ERRMSG_H
#define XB_ERRMSG_H
/*-------------------------------------------------------------------------------------------------------
  
  oserrmsg.h

  error messages and debug messages.

  These are supposed to be for programmer eyes only, although an incorrect
  installation may sometimes produce an error dialog that users see.

  os_output prints out a dialog,
  and error prints out a dialog and halts execution.
-------------------------------------------------------------------------------------------------------*/
//#include "errorcontext.h"
#include "debug.h"


// Bring up a dialog box to warn the user.
void warning(const stringx& str);

// Error message that halts execution
void error(const stringx& str);

void warning(const char* fmtp, ...);
void error(const char* fmtp, ...);
void disk_read_error(void);

// output in a dialog box (compatibility)
inline void os_output_string(const char* str) { warning(stringx(str)); }

#endif
