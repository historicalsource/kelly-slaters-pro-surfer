#include <dolphin/types.h>

void XFONTInit(u32 xfbW, u32 fbSize );

void XFONTSetFrameBuffer(void* xfb);

void XFONTPrintf( s16 x, s16 y, char* fmt, ... );
void XFONTSetFgColor(u32 fg);
void XFONTClearScreen(u32 bg);
void XFONTShadowRow(  u32 code, u32 y, u32 h );
void XFONTPaintRow( u32 code, u32 y, u32 h );

enum
{
	XFONT_COLOR_GRAY = 0,
	XFONT_COLOR_YELLOW,
	XFONT_COLOR_BLUE,	
	XFONT_COLOR_GREEN,	
	XFONT_COLOR_PURPLE,	
	XFONT_COLOR_RED,	
	XFONT_COLOR_DKBLUE,	
	XFONT_COLOR_BLACK	
};
