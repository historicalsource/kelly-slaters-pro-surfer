// ===================================================================== *
// MENU.H
//	 Generalized menu classes
// ===================================================================== */

//
// This is primarily the logic for element switching and general logic
// All display related and input interface code is in menusys.h/cpp
//

#ifndef _menu_h
#define _menu_h

/* ===================================================================== *
   Includes
 * ===================================================================== */

#include "menusys.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* ===================================================================== *
   Defined classes and forward class declarations
 * ===================================================================== */

class Menu;
class MenuEntry;
class MenuEntryLabel;
class MenuEntryTitle;
class MenuEntryFunction;
class Submenu;
class MenuEntryIntEdit;
class MenuEntryFloatEdit;
class MenuEntryFunctionFloatEdit;
class MenuEntryListEdit;
class MenuEntryStringxListEdit;
class MenuEntryEnumEdit;

typedef MenuEntry *pMenuEntry;

typedef bool MenuEntryButtonFunction( MenuEntry *entry, int buttonid );
#define MENUENTRYBUTTONFUNCTION(name) bool name( MenuEntry *entry, int buttonid )
typedef MenuEntryButtonFunction *pMenuEntryButtonFunction;

/* ===================================================================== *
   Constants and Enumerated types
 * ===================================================================== */

//-------------------------------------------------------------------
// MenuEntryFlags
//   Basic state information for menu entries

enum MenuEntryFlags
{
	MENTRY_VISIBLE      = 1 << 0,     // whether this entry is hidden/visible
	MENTRY_ENABLED      = 1 << 1,     // whether this entry is selectable
	MENTRY_ACTIVE       = 1 << 2,     // whether this entry is selected
};

// Entries must be visible to be enabled
// Entries must be enabled to be activated


/* ===================================================================== *
   Classes
 * ===================================================================== */

//-------------------------------------------------------------------
// Menu
//   This is a list of menu entries

class Menu
{
	Menu       *parent;
	int         entries;
	pMenuEntry *entry;
	int         activeentry;
	bool        isopen;
	Menu       *closeto;
	MenuSystem *control;


	bool Resize( int n );

	public:
	Menu( Menu *p ); // { parent=p; activeentry=-1; entries=0; entry=NULL; isopen=false; closeto=NULL; control=NULL; }
	Menu( Menu *p, int ents, pMenuEntry *ent ); // { parent=p; entries=ents; entry=ent; activeentry=-1; isopen=false; closeto=NULL; control=NULL; }
	virtual ~Menu() { Close(); Resize(0); }

	void ClearMenu( void );
	void AddEntry( pMenuEntry ent );
	void AddEntries( int ents, pMenuEntry *ent );
	void DelEntry( pMenuEntry ent );

	bool IsOpen( void ) { return isopen; }

	void Open( Menu *cto, MenuSystem *c );
	void Close( bool toparent=TRUE );
	virtual void CloseAll( void );

	protected:
	void ActivateEntry( int e );
	void FindActivateEntry( int dir );
	pMenuEntry GetEntry( int e ) { if (e >= 0 && e < entries) return entry[e]; else return NULL; }

	public:
	void ButtonPress( int buttoninfo );
	void ButtonRelease( int buttoninfo );

	int NumEntries( void ) { return entries; }
	int GetActiveEntry( void ) { return activeentry; }
	unsigned int GetElementFlags( int i );
	void GetElementText( int i, char *text, int len );
	MenuColor GetElementColor( int i );

	//
	// Functionality that can be changed
	//

	virtual void OnTick( float dtime );
	virtual void OnButtonPress( int buttonid );
	virtual void OnButtonRelease( int buttonid );

	protected:
	virtual void OnOpen( Menu *cto, MenuSystem *c );
	virtual void OnClose( bool toparent );
};

//-------------------------------------------------------------------
// MenuEntry
//   This is the base class for all menu entries. It controls the
//   internal logical constraints required by the Menu class

class MenuEntry
{
	friend class Menu;

	unsigned int flags;

	public:

	MenuEntry() { flags=MENTRY_VISIBLE | MENTRY_ENABLED; }
	virtual ~MenuEntry() { }

	protected:
	bool GetFlag( unsigned int flag ) { return (flags & flag)!=0; }
	void SetFlag( unsigned int flag, bool onOff ) { if (onOff) flags |= flag; else flags &= ~flag; }

	public:
	bool IsVisible( void ) { return GetFlag(MENTRY_VISIBLE); }
	bool IsEnabled( void ) { return IsVisible() && GetFlag(MENTRY_ENABLED); }
	bool IsActive( void ) { return GetFlag(MENTRY_ACTIVE); }

	unsigned int GetState( void ) { return flags; }
	virtual MenuColor GetColor( void );

	virtual void Show( void );
	virtual void Hide( void );
	virtual void Enable( void );
	virtual void Disable( void );
	virtual void Activate( void );
	virtual void Deactivate( void );

	public:
	virtual void OnTick( float dtime ) {}
	virtual void OnMenuOpen( Menu *m, MenuSystem *c );
	virtual void OnMenuClose( void );
	virtual void OnButtonPress( int buttonid ) {}
	virtual void OnButtonRelease( int buttonid ) {}

	protected:
	virtual void OnHide( void ) {}
	virtual void OnShow( void ) {}
	virtual void OnEnable( void ) {}
	virtual void OnDisable( void ) {}
	virtual void OnActivate( void ) {}
	virtual void OnDeactivate( void ) {}

	public:
	virtual int MenuText( char *text, int len ) { if (len>0) *text=0; return 0; }
};

//-------------------------------------------------------------------
// MenuEntryLabel
//   A menu entry that manifests as a simple text label without any
//   actual functionality

class MenuEntryLabel : public MenuEntry
{
	protected:
	const char      *label;

	public:
	MenuEntryLabel( const char *t ) { label=t; }
	virtual ~MenuEntryLabel() {}

	virtual int MenuText( char *text, int len );
};

//-------------------------------------------------------------------
// MenuEntryTitle
//   A menu entry that never gets enabled and is shown in a different
//   color than most entries. It is useful for menu titles, although
//   it is subject to the same constraints as any other entry (so it
//   is subject to being scrolled off a menu that is too short)

class MenuEntryTitle : public MenuEntryLabel
{
	public:
	MenuEntryTitle( char *t ) : MenuEntryLabel( t ) { Disable(); }
	virtual ~MenuEntryTitle() {}

		// This is never intended to be enabled
	virtual void Enable( void ) { MenuEntryLabel::Disable(); }
	virtual void Disable( void ) { MenuEntryLabel::Disable(); }
	virtual MenuColor GetColor( void );

};

//-------------------------------------------------------------------
// MenuEntryFunction
//   This is a typical menu entry. It's behavior is determined by a
//   user supplied callback function

class MenuEntryFunction : public MenuEntryLabel
{
	pMenuEntryButtonFunction fn;

	public:
	MenuEntryFunction( char *t, pMenuEntryButtonFunction f )
		: MenuEntryLabel( t )
	{ fn=f; }
	virtual ~MenuEntryFunction( ) {}

	virtual void OnButtonPress( int buttonid ) { if (fn) (*fn)(this,buttonid); }
	//virtual void OnButtonPress( int buttonid ) { MenuEntryLabel::OnButtonPress(buttonid); if (fn) (*fn)(this,buttonid); }
	//virtual void OnTick( float dtime ) { MenuEntryLabel::OnTick(dtime); if (fn) (*fn)(this,MENUCMD_NONE); }
	//virtual void OnButtonRelease( int buttonid ) { MenuEntryLabel::OnButtonRelease(buttonid); if (fn) (*fn)(this,-buttonid); }
};


class MenuEntryFunctionFloatEdit : public MenuEntryLabel
{
	float  *tfloat;
	protected:
	float  lo, hi, step;
	char *format;
	pMenuEntryButtonFunction fn;

	public:
	MenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction fn, float l, float h, float s );
	MenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction fn, float l, float h, float s, char *f );
	virtual ~MenuEntryFunctionFloatEdit() {}

	virtual void OnMenuOpen( Menu *m, MenuSystem *c ) { MenuEntryLabel::OnMenuOpen(m,c); FixValue(); }
	virtual int MenuText( char *text, int len );
	virtual void OnButtonPress( int buttonid );

	void FixValue( void );
//	virtual void DecValue( void );
//	virtual void IncValue( void );

	virtual void SetValue( float n ) { *tfloat = n; }
	virtual float GetValue( void ) { return tfloat ? *tfloat : 0; }
};


//-------------------------------------------------------------------
// Submenu
//   This is a menu entry which when selected, causes a child menu to
//   be brought up.

class Submenu : public MenuEntryLabel
{
	Menu       *menuopen;
	Menu       *parent;
	MenuSystem *system;

	public:
	Submenu( char *text, Menu *p ) : MenuEntryLabel(text) { menuopen=p; parent=NULL; system=NULL; }
	virtual ~Submenu() {}

	virtual void OnButtonPress( int buttonid );

	virtual void OnMenuOpen( Menu *m, MenuSystem *c ) { MenuEntryLabel::OnMenuOpen(m,c); parent=m; system=c; }
	virtual void OnMenuClose( void ) { parent=NULL; system=NULL; }
	Menu *GetMenu(void) { return menuopen; }

};

//-------------------------------------------------------------------
// MenuEntryIntEdit
//   This entry type allows a user supplied int value to be edited.
//   While the minimum and maximum values are user defined, the step
//   size is 1.

class MenuEntryIntEdit : public MenuEntryLabel
{
	int  *tint;
	protected:
	int  lo, hi;
	int  delta;
	char *format;

	public:
	MenuEntryIntEdit( const char *text, int *t, int l, int h );
	MenuEntryIntEdit( const char *text, int *t, int l, int h, char *f );
	MenuEntryIntEdit( const char *text, int *t, int l, int h, int d );
	MenuEntryIntEdit( const char *text, int *t, int l, int h, int d, char *f );
	virtual ~MenuEntryIntEdit() {}

	virtual void OnMenuOpen( Menu *m, MenuSystem *c ) { MenuEntryLabel::OnMenuOpen(m,c); FixValue(); }
	virtual int MenuText( char *text, int len );
	virtual void OnButtonPress( int buttonid );

	void FixValue( void );
	virtual void DecValue( void );
	virtual void IncValue( void );
	virtual void DecValueByTen( void );
	virtual void IncValueByTen( void );
	virtual void DecValueByFifty( void );
	virtual void IncValueByFifty( void );

	virtual void SetValue( int n ) { if (tint && n >= lo && n <= hi) *tint=n; }
	virtual int GetValue( void ) { return tint ? *tint : 0; }

};

//-------------------------------------------------------------------
// MenuEntryFloatEdit
//   This entry type allows a user supplied float value to be edited.
//   The minimum and maximum values and the step size are user defined.

class MenuEntryFloatEdit : public MenuEntryLabel
{
	float  *tfloat;
	protected:
	float  lo, hi, step;
	char *format;

	public:
	MenuEntryFloatEdit( char *text, float *t, float l, float h, float s );
	MenuEntryFloatEdit( char *text, float *t, float l, float h, float s, char *f );
	virtual ~MenuEntryFloatEdit() {}

	virtual void OnMenuOpen( Menu *m, MenuSystem *c ) { MenuEntryLabel::OnMenuOpen(m,c); FixValue(); }
	virtual int MenuText( char *text, int len );
	virtual void OnButtonPress( int buttonid );

	void FixValue( void );
//	virtual void DecValue( void );
//	virtual void IncValue( void );

	virtual void SetValue( float n ) { *tfloat = n; }
	virtual float GetValue( void ) { return tfloat ? *tfloat : 0; }
};

//-------------------------------------------------------------------
// MenuEntryListEdit
//   This entry type is similar to the int editing entry type, but
//   rather than display the actual number, the entry-ith item in
//   a user supplied list of strings is shown.

class MenuEntryListEdit : public MenuEntryIntEdit
{
	char **label;

	public:
	MenuEntryListEdit( char *text, int *t, int n, char **l );
	MenuEntryListEdit( char *text, int *t, int n, char **l, char *f );
	virtual ~MenuEntryListEdit() {}

	virtual int MenuText( char *text, int len );

};

class MenuEntryStringxListEdit : public MenuEntryIntEdit
{
	stringx *label;

	public:
	MenuEntryStringxListEdit( char *text, int *t, int n, stringx *l );
	MenuEntryStringxListEdit( char *text, int *t, int n, stringx *l, char *f );
	virtual ~MenuEntryStringxListEdit() {}

	virtual int MenuText( char *text, int len );

};

//-------------------------------------------------------------------
// MenuEntryEnumEdit
//   This entry type is similar to the list editing entry type, but
//   the values associated with each text label are arbitrarily set by
//   the user.

class MenuEntryEnumEdit : public MenuEntryListEdit
{
	int *vals;

	public:
	MenuEntryEnumEdit( char *text, int *t, int n, char **l, int *v );
	MenuEntryEnumEdit( char *text, int *t, int n, char **l, int *v, char *f );
	virtual ~MenuEntryEnumEdit() {}

	virtual void SetValue( int n );
	virtual int GetValue( void );

};

//-------------------------------------------------------------------
// MenuEntryIntCallbackEdit
//   Similar to the MenuEntryIntEdit, but it also has a callback function

class MenuEntryIntCallbackEdit : public MenuEntryIntEdit
{
	pMenuEntryButtonFunction fn;

	public:
	MenuEntryIntCallbackEdit( char *text, int *t, int l, int h, pMenuEntryButtonFunction cf )
		: MenuEntryIntEdit(text,t,l,h)
		{ fn=cf; }
	MenuEntryIntCallbackEdit( char *text, int *t, int l, int h, char *f, pMenuEntryButtonFunction cf )
		: MenuEntryIntEdit(text,t,l,h,f)
		{ fn=cf; }
	virtual ~MenuEntryIntCallbackEdit() {}

	virtual void OnButtonPress( int bid ) { MenuEntryIntEdit::OnButtonPress(bid); if (fn) (*fn)(this,bid); }

};







#endif

