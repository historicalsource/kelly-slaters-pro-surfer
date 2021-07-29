// xb_errmsg.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

// error message handling that's operating-system dependent

#include "global.h"

#include <stdio.h>
//#include "xb_errmsg.h"
#include "errorcontext.h"
#include "app.h"
#include "game_info.h"
#include "ksngl.h"	// For NGLFONT_TOKEN_ constants

#include <stdarg.h>

#undef free
#undef malloc
#include "ngl.h"

//-----------------------------------------------------------------------------
// Debug printing support
//-----------------------------------------------------------------------------

void os_unlock_static_heap();

void warning(const stringx& str)
{
	nglPrintf("%s\n", str.c_str());
}

void error(const stringx& str)
{
	nglPrintf("%s\n", str.c_str());
	assert(false);
#if defined(BUILD_BOOTABLE) || 0
	app::bomb();
#endif
}

void warning(const char* fmtp, ...) 
{
	va_list vlist;
	va_start(vlist, fmtp);
	char fmtbuff[2048];
	vsprintf(fmtbuff, fmtp, vlist);
	warning(stringx(fmtbuff));
}

void error(const char* fmtp, ...) 
{
	va_list vlist;
	va_start(vlist, fmtp);
	char fmtbuff[2048];
	vsprintf(fmtbuff, fmtp, vlist);
	assertmsg(false, fmtbuff);
}

// Specific to Xbox (dc 07/11/02)
void disk_read_error(void)
{
// Last minute hack to put up an error message before any disc loads have occurred.  
// We can't use the globaltext array, since it may not have been loaded yet.  
// So this is the one example of raw screen text in the code.  (dc 07/10/02)
static stringx disk_damaged_error[] = {
	// These versions are uppercase, because that's how they appeared in a posting from MS.
	"THERE'S A PROBLEM WITH THE DISC\nYOU'RE USING. IT MAY BE DIRTY OR\nDAMAGED.", 
	"", 
	"LE DISQUE UTILISE PRESENTE UNE\nANOMALIE. IL EST PEUT-ETRE SALE\nOU ENDOMMAGE.", 
	// This version needs to be lowercase, because we don't have uppercase letters with umlauts in our fonts.
	"Bei der benutzten DVD ist ein Problem\naufgetreten. Möglicherweise ist sie\nverschmutzt oder beschädigt.", 
};
assert(LANGUAGE_ENGLISH == 0);
assert(LANGUAGE_PIG_LATIN == 1);
assert(LANGUAGE_FRENCH == 2);
assert(LANGUAGE_GERMAN == 3);
assert(LANGUAGE_LAST == 4);

	// Xbox TRC 1.7-6-9 (dc 07/10/02)
	TextString::MakeReplacements(disk_damaged_error[ksGlobalTextLanguage]);
	official_error(disk_damaged_error[ksGlobalTextLanguage].c_str());
}
