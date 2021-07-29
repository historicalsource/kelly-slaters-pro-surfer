// ===================================================================== *
// MENUSYS.H
//	 Generalized menu system interface
// ===================================================================== */

#ifndef _menusys_h
#define _menusys_h

/* ===================================================================== *
   Includes
 * ===================================================================== */

#include <stddef.h>
#include <stdio.h>
//#include <assert.h>
#include <string.h>

/* ===================================================================== *
   Constants and Enumerated types
 * ===================================================================== */

//-------------------------------------------------------------------
// MenuCommand
//   These are the controls a menu can potentially act upon. While they
//   are named like PSX buttons, the mapping of these values to actual
//   buttons is system dependent.

enum MenuCommand
{
	MENUCMD_NONE  = 0,
	MENUCMD_SELECT   ,
	MENUCMD_START    ,
	MENUCMD_UP       ,
	MENUCMD_DOWN     ,
	MENUCMD_LEFT     ,
	MENUCMD_RIGHT    ,
	MENUCMD_CROSS    ,
	MENUCMD_TRIANGLE ,
	MENUCMD_SQUARE   ,
	MENUCMD_CIRCLE   ,
	MENUCMD_L1       ,
	MENUCMD_R1       ,
	MENUCMD_L2       ,
	MENUCMD_R2       ,
	MAXMENUCMD       ,
};

// max lines in a menu
const int maxmenulines=16;

// max length of a line of text
const int maxmenulinelength=64;

/* ===================================================================== *
   Defined classes and forward class declarations
 * ===================================================================== */

class Menu;
typedef unsigned int   MENU_UINT32;
typedef unsigned short MENU_UINT16;
typedef unsigned char  MENU_UINT8;
typedef char MenuLine[maxmenulinelength];

struct MenuColor
{
	MENU_UINT8 r,g,b,a;
};

struct MenuRect
{
	MENU_UINT16 x0,y0,x1,y1;
};


/* ===================================================================== *
   External function calls
 * ===================================================================== */

//
// This is all the system dependent functionality required by the menu
//   system.
//

//
// General initialization and cleanup
//

	// Initialize any system dependent menu stuff
extern void MENU_InitMenus( void );
	// Clean up any system dependent menu stuff
extern void MENU_TermMenus( void );

//
// Display routines
//

	// Start the drawing process
extern void MENU_DrawStart( void );

	// Complete the drawing process
extern void MENU_DrawEnd( void );

	// Determine border sizing ratio
extern int MENU_BorderSizeFactor( void );
extern int MENU_BorderSpaceFactor( void );

	// Draw a rectangle at the given coords of the given color
extern void MENU_DrawRect( int x0, int y0, int x1, int y1, const MenuColor &color );

	// Erase a rectangle at the given coords. This is only expected to
	//   erase drawn menu stuff. Clearing the background is optional.
extern void MENU_ClearRect( int x0, int y0, int x1, int y1 );

	// Draw text at the given coordinates
extern void MENU_DrawTextAt( int x, int y, const char *text, const MenuColor &color );

	// Determine the height of text
extern int MENU_TextHeight( const char *text );

	// Determine the width of text
extern int MENU_TextWidth( const char *text );

	// Determine how much separation is required between two lines of text
extern int MENU_TextSeparation( void );

//
// Input routines
//

	// Return 1 if the specified button is pressed, or 0 if it is not
extern int MENU_GetButtonState( int buttoninfo );


/* ===================================================================== *
   Classes
 * ===================================================================== */

class MenuRender
{

	MenuRect    limit;
	MenuRect    inuse;
	MenuLine    text[maxmenulines];
	MENU_UINT32 state[maxmenulines];
	int         active;
	int         topentry;

	public:
	MenuRender( MenuRect &mlim );
	virtual ~MenuRender() { MENU_TermMenus(); }

	void Draw( void );
	protected:
	void DrawFrame( int x0, int y0, int x1, int y1 );
	void Clear( void );

	public:
	virtual void OpenMenu( Menu *m );
	virtual void CloseMenu( void );

	void Refresh( void );

	int Width( void ) { return limit.x1-limit.x0; }

	protected:
		// this is the information the renderer requires from the menu
	virtual int TotalEntries( void )=0;
	virtual MENU_UINT32 GetElementState( int i )=0;
	virtual void GetElementText( int i, char *t, int len )=0;
	virtual MenuColor GetElementColor( int i )=0;

};

class MenuInput
{
	unsigned char bstate[MAXMENUCMD];

	public:
	MenuInput() { memset(bstate,0,MAXMENUCMD); }
	virtual ~MenuInput() {}

	void CheckButtonStates( void );
	void InitButtonStates( void );


	protected:
	virtual void ButtonPress( int buttoninfo )=0;
	virtual void ButtonRelease( int buttoninfo )=0;
};


class MenuSystem : public MenuRender, public MenuInput
{
	Menu *curmenu;

	public:
		MenuSystem( MenuRect &mlim ) : MenuRender(mlim) { curmenu=NULL; }
	virtual ~MenuSystem() {}

	virtual void OpenMenu( Menu *m );
	virtual void CloseMenu( void );
	void CloseAllMenus( void );

	void Opening( Menu *menu );
	void Closing( Menu *menu );

	virtual void ButtonPress( int buttoninfo );
	virtual void ButtonRelease( int buttoninfo );

	void Tick( float dtime );

	bool IsActive( void ) { return curmenu!=NULL; }

	protected:
		// this is the information the renderer requires from the menu
	virtual int TotalEntries( void );
	virtual MENU_UINT32 GetElementState( int i );
	virtual void GetElementText( int i, char *t, int len );
	virtual MenuColor GetElementColor( int i );
};









#endif


