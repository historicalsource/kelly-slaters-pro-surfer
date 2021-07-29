// DEBUG.CPP - Debugging module implementation.
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// None of the functions in this module should ever get referenced in the release version.

#include "global.h"

#include "debug.h"
#include "hwrasterize.h"

#if defined(TARGET_MKS)
extern "C" {
//#include "usrsnasm.h"
};
#endif

#ifndef BUILD_FINAL

#include <stdarg.h>

void KSDebugPrintf( const char* Format, ... )	//--- can't use nglPrintf from here, because it causes an infinite loop.
{
  static char Work[512];
  va_list args;
  va_start( args, Format );
  vsprintf( Work, Format, args );
  va_end( args );

  KSDebugPrint(Work);
}

void debug_print_str(const char* str)
{
#if defined(TARGET_PC)
  OutputDebugString(str);
  OutputDebugString("\n");
#elif defined(TARGET_PS2) || defined(TARGET_GC) || defined(TARGET_XBOX)
  KSDebugPrintf("%s\n", str);
#endif
}

void debug_print(const stringx& str)
{
  debug_print_str( str.c_str() );
}

void debug_print(const char* fmtp, ... )
{
  va_list vlist;
  va_start(vlist, fmtp);
  char fmtbuff[2048];
  vsprintf(fmtbuff, fmtp, vlist);
  debug_print_str(fmtbuff);
}

// Print on screen.
void debug_print(const vector2di& pos, const stringx& str)
{
  hw_rasta::inst()->print(str.c_str(), pos);
}


//-----------------------------------------------------------------------------
// Name: debug_out()
// Desc: Outputs a message to the debug stream
//-----------------------------------------------------------------------------
void debug_out( char* strFile, int dwLine, const char* strMsg )
{
  debug_print(stringx(strFile)+"("+itos(dwLine)+"): "+strMsg);
}

#endif
