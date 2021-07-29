/*---------------------------------------------------------------------------*
  Project:  Dolphin XFB Font Library
  File:     Xfont.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

Change History:
	
	09/24/01	Greg McBride
				changed to initial ASM version of putstring
				
	09/25/01	Greg McBride
				removed old code
				optimized putString function, unrolling loop
				and various other optimizing
				
    
 $NoKeywords: $

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <dolphin.h>
#include "xfont.h"


static u32 m_fbSize;
static u8 *m_xfb;
static u32 XfbWidth;
static u16 cc;
static u16 cc2;

typedef struct
{
    u32         Y;
    u32         Cb;
    u32         Cr;
    
} XFColor_s;

static XFColor_s XFcolor[] = {
    { 180, 128, 128 },
    { 162,  44, 142 },
    { 131, 156,  44 },
    { 112,  72,  58 },
    {  84, 184, 198 },
    {  65, 100, 212 },
    {  35, 212, 114 },
    {  16, 128, 128 },
};

// this font table stores ASCII values
// from 0x20 up to 0x7f
// 0x7f is a "pointing hand" instead of delete char
extern u32	XFontBitmap[];

void XFONTputString(register s16 *x, register s16 *y, register char *strptr, register u32 *fontptr );

/*---------------------------------------------------------------------------*
    Name:           XFONTInit
    
    Description:    This function saves some information about any
    				external frame buffers that the application is using.
    
    Arguments:      xfbW		width of the application-created xfb
    
    				fbSize		32byte padded size of allocated frame buffer
    
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTInit(u32 xfbW, u32 fbSize )
{
    XfbWidth = xfbW;
    
    m_fbSize = fbSize;
}

/*---------------------------------------------------------------------------*
    Name:           XFONTSetFrameBuffer
    
    Description:    This function sets a pointer to the next external
                    video frame buffer to render into.
    
    Arguments:      xfb			pointer to an external frame buffer
    							that the application has created
    
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTSetFrameBuffer(void* xfb)
{
	m_xfb = (u8*)xfb;
}

/*---------------------------------------------------------------------------*
    Name:           XFONTPrintf
    
    Description:    This function draws a character string, at the 
    				character column and row (16 by 6 pixel characters)
    				given by x and y. 
    
    Arguments:      x			character column to start at
    
    				y			line number to display this string on
    				
    				fmt			text to display, including variables
    							
    				
    				...			any variables that are needed by the
    							format string
    
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTPrintf( s16 x, s16 y, char* fmt, ... )
{
    va_list  vlist;
    char     buf[256];

    va_start( vlist, fmt );
    vsprintf( buf, fmt, vlist );
    va_end( vlist );

	XFONTputString(&x, &y, buf, XFontBitmap );

    return;
}

/*---------------------------------------------------------------------------*
    Name:           XFONTSetFgColor
    
    Description:    This function sets the color that will be used when
    				drawing text.
    
    Arguments:      fg			color code for text drawing
    							colors range from 0 - 7
    							
    
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTSetFgColor(u32 fg)
{
	u16 colorVal, colorVal2;

    colorVal = (u16)((XFcolor[fg].Y << 8)
        + (XFcolor[fg].Cb));

	colorVal2 =(u16)((XFcolor[fg].Y << 8)
        + XFcolor[fg].Cr);
	
	cc = colorVal;
	
    cc2 = colorVal2;
}

/*---------------------------------------------------------------------------*
    Name:           XFONTPaintRow
    
    Description:    This function will paint a horizontal bar on a text
    				row at a givel row number with a given color. Good
    				for making titles stand out against a game screen. 
    
    Arguments:      code		color code for bar
    							colors range from 0 - 7
    							
    				y			starting character row to paint
    				
    				h			number of character rows to paint
    
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTPaintRow( u32 code, u32 y, u32 h )
{
    u32         cVal;
    u8*         ptr;
    u8* start;
    u8* end;
    u32 inc;
    
    cVal = (XFcolor[code].Y << 24)
        + (XFcolor[code].Cb << 16)
        + (XFcolor[code].Y << 8)
        + XFcolor[code].Cr;

	start = m_xfb + XfbWidth*y*32;
	end = start + XfbWidth*h*32;
	inc = VI_DISPLAY_PIX_SZ * 2;



    for (ptr = start; ptr < end; ptr += inc)
        *(u32*)ptr = cVal;
}

/*---------------------------------------------------------------------------*
    Name:           XFONTClearScreen
    
    Description:    This function will fill the screen with a given color. 
    
    Arguments:      code		color code for filling screen
    							colors range from 0 - 7
    							
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTClearScreen(u32 code)
{
    u32         cVal;
    u8*         ptr;
    
    cVal = (XFcolor[code].Y << 24)
        + (XFcolor[code].Cb << 16)
        + (XFcolor[code].Y << 8)
        + XFcolor[code].Cr;

    for (ptr = m_xfb; ptr < m_xfb + m_fbSize; ptr += VI_DISPLAY_PIX_SZ * 2)
        *(u32*)ptr = cVal;
}

/*---------------------------------------------------------------------------*
    Name:           XFONTShadowRow
    
    Description:    This function will alter the color of the existing
    				screen pixels. Good for darkening  a background 
    				image before drawing text on top. 
    
    Arguments:      code		color code for filling screen
    							colors range from 0 - 7
    							
    				y			start character row to fill in
    				
    				h			number of character rows to fill
    							
    Returns:        None
 *---------------------------------------------------------------------------*/
void XFONTShadowRow(  u32 code, u32 y, u32 h )
{
    u32         cVal;
    u8*         ptr;
    u8* start;
    u8* end;
    u32 topY;
    u32 botY;
    
    cVal = (XFcolor[code].Y << 24)
        + (XFcolor[code].Cb << 16)
        + (XFcolor[code].Y << 8)
        + XFcolor[code].Cr;

	start = m_xfb + XfbWidth*y*32;
	end = start + XfbWidth*h*32;

    for (ptr = start; ptr < end; ptr += VI_DISPLAY_PIX_SZ * 2)
    {
	    cVal = *(u32*)ptr;
	    
	    topY = (cVal ) & 0x0f00;
	
		botY = cVal & 0x000f;
		
	    cVal &= 0xf0f0;
	    
	    cVal |= topY;
	    cVal |= botY;
	    
        *(u32*)ptr = cVal;
    }
}

/*---------------------------------------------------------------------------*
    Name:           XFONTputString
    
    Description:    This function will print a character string to the screen. 
    
    Arguments:      x			character column to start drawing at.
    
    				y			character row.
    				
    				string		array of chars to draw.
    							
    Returns:        None
*---------------------------------------------------------------------------*/
void XFONTputString(register s16 *x, register s16 *y, register char *strptr, register u32 *fontptr )
{
	__asm
	{
		lwz		r8, strptr
		lhz		r5, 0(x)
		addi	r8, r8, -1		
		lhz		r6, 0(y)
		slwi	r5, r5, 5		// change from char coords to pixel coords, and times 2 bytes per pixel

// This is a global object and must be referenced via far addressing
// via lis and addi. 
        lis     r7, XfbWidth@ha
        addi    r7, r7, XfbWidth@l
        lwz     r9, 0(r7)
        
        
		slwi	r6, r6, 5		// change from char coords to pixel coords, and times 2 bytes per pixel
		mullw	r9, r6, r9
		
		
        lis     r7, m_xfb@ha
        addi    r7, r7, m_xfb@l
        lwz     r10, 0(r7)
   		
		
		add		r9, r5, r9		// y*linewidth + x
		
		
		add		r9, r9, r10	// r10 is start of pixels to draw

        lis     r7, cc@ha
        addi    r7, r7, cc@l
        lhz     r6, 0(r7)

		
		slwi	r7, r6, 16
		or		r7, r6, r7		// r7 is color bytes
		
__loop3:
		lbzu	r12, 1(r8) 		// get offset to font data for this char
		lwz		r6, fontptr
		
		cmpwi	r12, 0
		beq		__done
		
		subi	r12, r12, 32	// font table starts at ASCII 32
		li		r16, 8			// draw 8 lines
		slwi	r12, r12, 5		// 32 bytes per char in the font table
		add		r10, r12, r6	// r10 is start of char's font data
		li		r15, 1
		subi	r10, r10, 4		// preincrement later, so start before actual data



__loop2:		
		lwzu	r14, 4(r10)

		cmpwi	r14, 0			// check for negative (top bit set)
		li		r12, 8			// draw 8 lines
		
		bge		__nodraw1		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw1:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw2		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw2:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw3		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw3:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw4		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw4:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw5		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw5:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw6		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw6:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw7		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw7:	
		slwi.	r14, r14, 4		// next dot in font data
		addi	r9, r9, 4		// next "dot"	
		
		bge		__nodraw8		// draw the pixel or not?
		
		stw		r7, 0(r9)		// write 2 pixels to the screen
		stw		r7, 1280(r9)	// draw line below this as well

__nodraw8:	

		subf.	r16, r15, r16	// loop
		addi	r9, r9, 4		// next "dot"	
		addi	r9, r9, 2528	// start of next pair of lines
		
		bne		__loop2
		
		
		// next char if there is one
		
		addi	r9, r9, -20448		// next character start position
		
		b		__loop3
		
__done:
			
	}
}