// ps2_errmsg.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// error message handling that's operating-system dependent

#include "global.h"

#include <stdio.h>
//#include "ps2_errmsg.h"
#include "errorcontext.h"
#include "app.h"
#include "game_info.h"
#include "hwrasterize.h"
#include "inputmgr.h"

#include <stdarg.h>

//-----------------------------------------------------------------------------
// Debug printing support
//-----------------------------------------------------------------------------

void os_unlock_static_heap();

void onscreenerror(const stringx& str)
{
	low_level_console_print("%s\n", str.c_str());
	low_level_console_flush();
}

void warning(const stringx& str)
{
	nglPrintf("%s\n", str.c_str());
}

void error(const stringx& str)
{
	nglPrintf("%s\n", str.c_str());
	assertmsg(false, str.c_str());
	#if defined(BUILD_BOOTABLE) || 0
	app::bomb();
	#endif
}

void onscreenerror(const char* fmtp, ...)
{
	va_list vlist;
	va_start(vlist, fmtp);
	char fmtbuff[2048];
	vsprintf(fmtbuff, fmtp, vlist);
	onscreenerror(stringx(fmtbuff));
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
	error(stringx(fmtbuff));
}


#ifdef DEBUG
#define MAX_STACK_DEPTH 10
#define DEPTH_IGNORE 0
#define OPCODE_MASK 0xffff0000
#define IMMEDIATE_MASK 0x0000ffff
#define ADDIUSP 0x27bd0000
#define SQRA    0x7fbf0000
#define SQFP    0x7fbe0000

unsigned int g_main_framepointer;

#define SEARCH_BACKWARD(pc,opcode)\
    do{\
        unsigned int *addr = (unsigned int *)pc;\
        while( (*addr&OPCODE_MASK) != opcode )\
            addr--;\
        pc = (unsigned int)addr;\
    }while(0);

#define SEARCH_FORWARD(pc,opcode)\
    do{\
        unsigned int *addr = (unsigned int *)pc;\
        while( (*addr&OPCODE_MASK) != opcode && addr < ((unsigned int *)pc) + 10)\
            addr++;\
        if( addr == ((unsigned int *)pc + 10) )\
          pc = 0;\
        else\
          pc = (unsigned int)addr;\
    }while(0);

#define PEEK_OFFSET(addr) (short)((*(unsigned int *)addr)&IMMEDIATE_MASK)
            
void get_callstack(unsigned int fp, unsigned int *addrs)
{
    unsigned int search_addr;
    int i;
    char *fp_addr = (char *)fp;
    unsigned int *temp_32;
    
    __asm__ volatile ("move %0,$31" : "=r"(search_addr) : : "%0");

    for( i = MAX_STACK_DEPTH + DEPTH_IGNORE - 1; i >= 0; --i )
    {
        short ra_offset,fp_offset;
        unsigned int sqra_addr,sqfp_addr;
        
        if( (unsigned int)fp_addr == g_main_framepointer )
        {
          addrs[i] = 0x0;
          continue;
        }
        SEARCH_BACKWARD(search_addr,ADDIUSP);
        
        sqra_addr = search_addr;
        sqfp_addr = search_addr;
        SEARCH_FORWARD(sqra_addr,SQRA);
        SEARCH_FORWARD(sqfp_addr,SQFP);
        // eck, if either of these are zero we are probably back in crt0 stuff
        if( !sqra_addr || !sqra_addr )
        {
          fp_addr = (char *)g_main_framepointer;
          addrs[i] = 0x0;
          continue;
        }
        ra_offset = PEEK_OFFSET(sqra_addr);
        fp_offset = PEEK_OFFSET(sqfp_addr);

        temp_32 = (unsigned int *)(fp_addr + ra_offset);
        search_addr = *temp_32;
        if( i < MAX_STACK_DEPTH ) addrs[i] = (*temp_32) - 8;
        temp_32 = (unsigned int *)(fp_addr + fp_offset);
        fp_addr = (char *) *temp_32;
    }
}

#define GET_BACKTRACE(addrs)\
do{\
  int fp;\
  __asm__ volatile ("move %0,$fp" : "=r"(fp) : : "%0");\
  get_callstack(fp, (unsigned int *)addrs );\
} while(0);

const char *get_backtrace(const char *string) 
{
	static char newstring[2048];	// need to protect against overflow. (dc 06/10/02)
	strcpy(newstring, string);
	int addrs[30];
	int counter = 0;
	memset(addrs, -1, sizeof(int)*30);
	GET_BACKTRACE(addrs);
	strcat(newstring, "\nCALLSTACK:\n");
	while((addrs[counter] != -1) && (counter < 30))
	{
		char tempstr[10];
		sprintf(tempstr, "0x%x\n", addrs[counter]);

		strcat(newstring, tempstr);
		counter++;
	}

	return newstring;
}

#endif	// #ifdef DEBUG

