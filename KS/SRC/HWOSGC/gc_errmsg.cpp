#include "global.h"

#include <stdio.h>
#include "inputmgr.h"

#include "errorcontext.h"
#include "app.h"
#include "game_info.h"
#include "ngl.h"

#include <stdarg.h>

void os_unlock_static_heap( void );


#define gcerrprintf OSReport

void warning( const stringx& str )
{
	gcerrprintf( "%s\n", str.c_str( ) );
}

void error( const stringx& str )
{
	gcerrprintf( "%s\n", str.c_str( ) );
  assert( false );
}

void warning( const char* fmtp, ... )
{
  va_list vlist;
  va_start( vlist, fmtp );
  char fmtbuff[2048];

  vsprintf( fmtbuff, fmtp, vlist );

  warning( stringx( fmtbuff ) );
}

void error( const char* fmtp, ... )
{
  va_list vlist;
  va_start( vlist, fmtp );
  char fmtbuff[2048];

  vsprintf( fmtbuff, fmtp, vlist );

  error( stringx( fmtbuff ) );
}

