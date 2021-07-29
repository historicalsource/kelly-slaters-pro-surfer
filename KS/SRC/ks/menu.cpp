// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "menu.h"

#if _XBOX
// weird
#ifndef NEW
#define NEW new
#endif /* NEW */

//#include <assert.h>	// May override our own assert.  (dc 06/14/02)
#include <stdlib.h>
#include <stdio.h>
#endif /* TARGET_XBOX JIV DEBUG */

MenuColor entrytitle     = { 255,  65,  65, 255 };
MenuColor entryactive    = { 255, 255,   0, 255 };
MenuColor entryenabled   = { 192, 192, 192, 255 };
MenuColor entrydisabled  = {  64,  64,  64, 255 };

//===================================================================
// Menu

Menu::Menu( Menu *p )
{
	parent=p;
	activeentry=-1;
	entries=0;
	entry=NULL;
	isopen=false;
	closeto=NULL;
	control=NULL;
}

Menu::Menu( Menu *p, int ents, pMenuEntry *ent )
{
	parent=p;
	entries=0;
	entry=NULL;
	activeentry=-1;
	isopen=false;
	closeto=NULL;
	control=NULL;
	AddEntries(ents,ent);
	//entries=ents;
	//entry=ent;
}

bool Menu::Resize( int n )
{
	if ( entries!=n )
	{
		pMenuEntry *nentry=NULL;
		if ( n )
		{
			nentry=NEW pMenuEntry[n];
			if ( nentry==NULL ) return FALSE;
			for ( int i=0; i<n; i++ )
			{
				nentry[i]=NULL;
			}
			int cpy=min(n,entries);
			if ( cpy )
			{
				for ( int i=0; i<cpy; i++ )
				{
					nentry[i]=entry[i];
				}
			}
		}
		if (entry)
		{
			delete[] entry;
		}
		entry=nentry;
		entries=n;
		if (activeentry >= entries)
		{
			activeentry = -1;
		}
	}
	return TRUE;
}


void Menu::Open( Menu *cto, MenuSystem *c )
{
	if ( !isopen )
	{
		control=c;
		control->Opening(this);
		if (cto) 
		{
			assert(activeentry==-1);
		}
		OnOpen(cto,c);
		isopen=true;
		if ( activeentry<0 )
			FindActivateEntry(1);
		if ( activeentry<0 )
		{
				// no entries enabled
			assert(0);
			Close();
		}
	}
}

void Menu::Close( bool toparent )
{
	if ( isopen )
	{
		isopen=false;
		control->Closing(this);
		OnClose(toparent);
		if (toparent) 
		{
			ActivateEntry(-1);
			assert(activeentry==-1);
		}
	}
}

void Menu::CloseAll( void )
{
	if ( isopen )
	{
		Menu *o=closeto;
		Close();
		if ( o )
			o->CloseAll();
	}
}

void Menu::ClearMenu( void )
{
	Resize(0);
}

void Menu::AddEntry( pMenuEntry ent )
{
	Resize(entries+1);
	entry[entries-1]=ent;
}

void Menu::AddEntries( int ents, pMenuEntry *ent )
{
	for ( int i=0; i<ents; i++ )
		AddEntry(ent[i]);
}

void Menu::DelEntry( pMenuEntry ent )
{
	// The commented line is superfluous.  (dc 06/18/02)
//	if ( entries )
		for ( int i=0; i<entries; i++ )
		{
			if ( entry[i]==ent )
			{
				// The commented lines are superfluous.  (dc 06/18/02)
//				if ( i<entries-1 )
//				{
					for ( int j=i; j<entries-1; j++ )
					{
						// We're shifting, not swapping!  The commented lines are incorrect.  (dc 06/18/02)
//						pMenuEntry te=entry[i];
//						entry[i]=entry[i+1];
//						entry[i+1]=te;

						entry[j]=entry[j+1];
					}
				}
				Resize(entries-1);
				return;
//			}
		}
}

unsigned int Menu::GetElementFlags( int i )
{
	if (i>=0 && i<entries && entry[i])
		return entry[i]->GetState();
	return 0;
}

void Menu::GetElementText( int i, char *text, int len )
{
	if (i>=0 && i<entries && entry[i])
		entry[i]->MenuText(text,len);
	else
		strcpy(text,"");
}

MenuColor Menu::GetElementColor( int i )
{
	if (i>=0 && i<entries && entry[i])
		return entry[i]->GetColor();
	else
		return entrydisabled;
}

void Menu::ActivateEntry( int e )
{
	if ( activeentry>=0 && entry[activeentry] )
	{
		entry[activeentry]->Deactivate();
		activeentry=-1;
	}
	if ( e>=0  && entry[e] )
	{
		entry[e]->Activate();
		if (entry[e]->IsActive())
			activeentry=e;
	}
}

void Menu::FindActivateEntry( int dir )
{
	if ( entries )
	{
		int a=activeentry;
		for ( int i=0; i<entries; i++ )
		{
			a+=dir;
			if ( a<0 ) a=entries-1;
			if ( a>=entries ) a=0;
			if ( entry[a] && entry[a]->IsEnabled() )
			{
				ActivateEntry(a);
				return;
			}
		}
	}
}

void Menu::ButtonPress( int buttoninfo )
{
	assert(isopen);
	//MenuCommand mc=buttoninfo; //MENU_TranslateButton( buttoninfo );
	OnButtonPress(buttoninfo);
	control->Refresh();
}

void Menu::ButtonRelease( int buttonid )
{
	assert(isopen);
	//MenuCommand mc=buttoninfo; // MENU_TranslateButton( buttonid );
	OnButtonRelease(buttonid);
}



void Menu::OnTick( float dtime )
{
	for ( int i=0; i<entries; i++ )
		if ( entry[i] )
			entry[i]->OnTick(dtime);
}

void Menu::OnOpen( Menu *cto, MenuSystem *c )
{
	if ( cto )
		closeto=cto;
	for ( int i=0; i<entries; i++ )
		if ( entry[i] )
			entry[i]->OnMenuOpen(this,c);
}

void Menu::OnClose( bool toparent )
{
	for ( int i=0; i<entries; i++ )
		if ( entry[i] )
			entry[i]->OnMenuClose();
	if ( toparent )
	{
		Menu *o=closeto;
		closeto=NULL;
		if ( o )
			o->Open(NULL,control);
	}
}

void Menu::OnButtonPress( int buttonid )
{
	assert(isopen);
	switch ( buttonid )
	{
		case MENUCMD_UP:
			FindActivateEntry(-1);
			break;
		case MENUCMD_DOWN:
			FindActivateEntry(1);
			break;
/* BETH
		case MENUCMD_START:
			CloseAll();
			break;
*/
		case MENUCMD_TRIANGLE:
			Close();
			break;
		default:
			if ( activeentry>=0 && entry[activeentry])
				entry[activeentry]->OnButtonPress(buttonid);
	}
}

void Menu::OnButtonRelease( int buttonid )
{
	assert(isopen);
	switch ( buttonid )
	{
		case MENUCMD_UP:
		case MENUCMD_DOWN:
		case MENUCMD_START:
		case MENUCMD_TRIANGLE:
			break;
		default:
			if ( activeentry>=0 && entry[activeentry])
				entry[activeentry]->OnButtonRelease(buttonid);
	}
}

//===================================================================
// MenuEntry

void MenuEntry::Show( void )
{
	if (!IsVisible())
	{
		SetFlag(MENTRY_VISIBLE,true);
		OnShow();
	}
}

void MenuEntry::Hide( void )
{
	if ( IsEnabled() )
		Disable();
	if (IsVisible())
	{
		OnHide();
		SetFlag(MENTRY_VISIBLE,false);
	}
}

void MenuEntry::Enable( void )
{
	assert(IsVisible());
	if ( IsVisible() && !IsEnabled())
	{
		SetFlag(MENTRY_ENABLED,true);
		OnEnable();
	}
}

void MenuEntry::Disable( void )
{
	if ( IsActive() )
		Deactivate();
	if (IsEnabled())
	{
		OnDisable();
		SetFlag(MENTRY_ENABLED,false);
	}
}

void MenuEntry::Activate( void )
{
	assert(IsEnabled());
	if ( IsEnabled() && !IsActive() )
	{
		SetFlag(MENTRY_ACTIVE,true);
		OnActivate();
	}
}

void MenuEntry::Deactivate( void )
{
	if (IsActive())
	{
		OnDeactivate();
		SetFlag(MENTRY_ACTIVE,false);
	}
}

void MenuEntry::OnMenuOpen( Menu *m, MenuSystem *c  )
{
	Show();
	Enable();
}

void MenuEntry::OnMenuClose( void )
{
	Deactivate();
}

MenuColor MenuEntry::GetColor( void )
{
	if ( IsActive() )
	{
		return entryactive;
	}
	else if ( IsEnabled() )
	{
		return entryenabled;
	}
	else
	{
		return entrydisabled;
	}
}



//===================================================================
// MenuEntryText

int MenuEntryLabel::MenuText( char *text, int len )
{
	if ( len && label )
	{
		strncpy(text,label,len);
		text[len-1]=0;
		return strlen(text);
	}
	else
		return MenuEntry::MenuText(text,len);
}

//===================================================================
// MenuEntryTitle

MenuColor MenuEntryTitle::GetColor( void )
{
	return entrytitle;
}

//===================================================================
// Submenu

void Submenu::OnButtonPress( int buttonid )
{
	if ( buttonid==MENUCMD_CROSS && parent && menuopen )
	{
		Menu *p=parent;
		MenuSystem *s=system;
		parent->Close(FALSE);
		menuopen->Open(p,s);
	}
}

//===================================================================
// MenuEntryIntEdit


MenuEntryIntEdit::MenuEntryIntEdit( const char *text, int *t, int l, int h )
	: MenuEntryLabel(text)
{
	tint=t;
	lo=l;
	hi=h;
	format=NULL;
	delta=1;
	FixValue();
}

MenuEntryIntEdit::MenuEntryIntEdit( const char *text, int *t, int l, int h, char *f )
	: MenuEntryLabel(text)
{
	tint=t;
	lo=l;
	hi=h;
	format=f;
	delta=1;
	FixValue();
}

MenuEntryIntEdit::MenuEntryIntEdit( const char *text, int *t, int l, int h, int d )
	: MenuEntryLabel(text)
{
	tint=t;
	lo=l;
	hi=h;
	format=NULL;
	delta=d;
	FixValue();
}

MenuEntryIntEdit::MenuEntryIntEdit( const char *text, int *t, int l, int h, int d, char *f )
	: MenuEntryLabel(text)
{
	tint=t;
	lo=l;
	hi=h;
	format=f;
	delta=d;
	FixValue();
}

void MenuEntryIntEdit::OnButtonPress( int buttonid )
{
	if ( buttonid==MENUCMD_LEFT )
		DecValue();
	else if ( buttonid==MENUCMD_RIGHT )
		IncValue();
	else if ( buttonid==MENUCMD_R1 )
		IncValueByTen();
	else if ( buttonid==MENUCMD_L1 )
		DecValueByTen();
	else if ( buttonid==MENUCMD_R2 )
		IncValueByFifty();
	else if ( buttonid==MENUCMD_L2 )
		DecValueByFifty();
	else
		MenuEntryLabel::OnButtonPress(buttonid);
}

int MenuEntryIntEdit::MenuText( char *text, int len )
{
	if ( len && label )
	{
		int lused=MenuEntryLabel::MenuText(text,len);
		char tval[32];
		if ( format )
		{
			sprintf(tval,format,GetValue());
		}
		else
		{
			sprintf(tval," : %d",GetValue());
		}

		strncpy(text+lused,tval,len-lused);
		text[len-1]=0;
		return strlen(text);
	}
	else
		return MenuEntry::MenuText(text,len);
}

void MenuEntryIntEdit::FixValue( void )
{
	int v=GetValue();
	SetValue( v<lo ? lo : (v>hi ? hi : v));
}

void MenuEntryIntEdit::DecValue( void )
{
	SetValue( GetValue()-delta );
	FixValue();
}

void MenuEntryIntEdit::IncValue( void )
{
	SetValue( GetValue()+delta );
	FixValue();
}

void MenuEntryIntEdit::DecValueByTen( void )
{
	SetValue( GetValue()-(10*delta) );
	FixValue();
}

void MenuEntryIntEdit::IncValueByTen( void )
{
	SetValue( GetValue()+(10*delta) );
	FixValue();
}
void MenuEntryIntEdit::DecValueByFifty( void )
{
	SetValue( GetValue()-(50*delta) );
	FixValue();
}

void MenuEntryIntEdit::IncValueByFifty( void )
{
	SetValue( GetValue()+(50*delta) );
	FixValue();
}

//===================================================================
// MenuEntryFloatEdit


MenuEntryFloatEdit::MenuEntryFloatEdit( char *text, float *t, float l, float h, float s )
	: MenuEntryLabel(text)
{
	tfloat=t;
	lo=l;
	hi=h;
  step=s;
	format=NULL;
	FixValue();
}

MenuEntryFloatEdit::MenuEntryFloatEdit( char *text, float *t, float l, float h, float s, char *f )
	: MenuEntryLabel(text)
{
	tfloat=t;
	lo=l;
	hi=h;
  step=s;
	format=f;
	FixValue();
}

void MenuEntryFloatEdit::OnButtonPress( int buttonid )
{
  switch (buttonid)
  {
  case MENUCMD_LEFT:
	SetValue (GetValue () - step);
	FixValue ();
    break;
  case MENUCMD_RIGHT:
	SetValue (GetValue () + step);
	FixValue ();
    break;
  case MENUCMD_L1:
	SetValue (GetValue () - step * 10);
	FixValue ();
    break;
  case MENUCMD_R1:
	SetValue (GetValue () + step * 10);
	FixValue ();
    break;
  case MENUCMD_L2:
	SetValue (GetValue () - step * 100);
	FixValue ();
    break;
  case MENUCMD_R2:
	SetValue (GetValue () + step * 100);
	FixValue ();
    break;
  default:
    MenuEntryLabel::OnButtonPress(buttonid);
  }
}

int MenuEntryFloatEdit::MenuText( char *text, int len )
{
	if ( len && label )
	{
		int lused=MenuEntryLabel::MenuText(text,len);
		char tval[32];
		if ( format )
		{
			sprintf(tval,format,GetValue());
		}
		else
		{
			sprintf(tval," : %0.2f",GetValue());
		}

		strncpy(text+lused,tval,len-lused);
		text[len-1]=0;
		return strlen(text);
	}
	else
		return MenuEntry::MenuText(text,len);
}

void MenuEntryFloatEdit::FixValue( void )
{
	float v=GetValue();
	SetValue( v<lo ? lo : (v>hi ? hi : v));
}


//===================================================================
// MenuEntryFunctionFloatEdit


MenuEntryFunctionFloatEdit::MenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction func, float l, float h, float s )
	: MenuEntryLabel(text)
{
	tfloat=t;
	fn = func;
	lo=l;
	hi=h;
  step=s;
	format=NULL;
	FixValue();
}

MenuEntryFunctionFloatEdit::MenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction func, float l, float h, float s, char *f )
	: MenuEntryLabel(text)
{
	tfloat=t;
	fn = func;
	lo=l;
	hi=h;
  step=s;
	format=f;
	FixValue();
}

void MenuEntryFunctionFloatEdit::OnButtonPress( int buttonid )
{
  switch (buttonid)
  {
  case MENUCMD_LEFT:
	SetValue (GetValue () - step);
	FixValue ();
    break;
  case MENUCMD_RIGHT:
	SetValue (GetValue () + step);
	FixValue ();
    break;
  case MENUCMD_L1:
	SetValue (GetValue () - step * 10);
	FixValue ();
    break;
  case MENUCMD_R1:
	SetValue (GetValue () + step * 10);
	FixValue ();
    break;
  case MENUCMD_L2:
	SetValue (GetValue () - step * 100);
	FixValue ();
    break;
  case MENUCMD_R2:
	SetValue (GetValue () + step * 100);
	FixValue ();
    break;
	case MENUCMD_CROSS:
	if (fn) (*fn)(this,buttonid);
	break;
  default:
    MenuEntryLabel::OnButtonPress(buttonid);
  }
}

int MenuEntryFunctionFloatEdit::MenuText( char *text, int len )
{
	if ( len && label )
	{
		int lused=MenuEntryLabel::MenuText(text,len);
		char tval[32];
		if ( format )
		{
			sprintf(tval,format,GetValue());
		}
		else
		{
			sprintf(tval," : %0.2f",GetValue());
		}

		strncpy(text+lused,tval,len-lused);
		text[len-1]=0;
		return strlen(text);
	}
	else
		return MenuEntry::MenuText(text,len);
}

void MenuEntryFunctionFloatEdit::FixValue( void )
{
	float v=GetValue();
	SetValue( v<lo ? lo : (v>hi ? hi : v));
}


//===================================================================
// MenuEntryListEdit


MenuEntryListEdit::MenuEntryListEdit( char *text, int *t, int n, char **l )
	: MenuEntryIntEdit(text,t,0,n-1)
{
	label=l;
}

MenuEntryListEdit::MenuEntryListEdit( char *text, int *t, int n, char **l, char *f )
	: MenuEntryIntEdit(text,t,0,n-1,f)
{
	label=l;
}

int MenuEntryListEdit::MenuText( char *text, int len )
{
	if ( len && label )
	{
		int lused=MenuEntryLabel::MenuText(text,len);
		char tval[256];
/*
		char *estr=label[GetValue()];
*/
		if ( format )
		{
			sprintf(tval,format,label[GetValue()]);
		}
		else
		{
			sprintf(tval," : %s",label[GetValue()]);
		}

		strncpy(text+lused,tval,len-lused);
		text[len-1]=0;
		return strlen(text);
	}
	else
		return MenuEntry::MenuText(text,len);
}


//===================================================================
// MenuEntrySTringxListEdit


MenuEntryStringxListEdit::MenuEntryStringxListEdit( char *text, int *t, int n, stringx *l )
	: MenuEntryIntEdit(text,t,0,n-1)
{
	label=l;
}

MenuEntryStringxListEdit::MenuEntryStringxListEdit( char *text, int *t, int n, stringx *l, char *f )
	: MenuEntryIntEdit(text,t,0,n-1,f)
{
	label=l;
}

int MenuEntryStringxListEdit::MenuText( char *text, int len )
{
	if ( len && label )
	{
		int lused=MenuEntryLabel::MenuText(text,len);
		char tval[256];
/*
		char *estr=label[GetValue()];
*/
		if ( format )
		{
			sprintf(tval,format,label[GetValue()].c_str());
		}
		else
		{
			sprintf(tval," : %s",label[GetValue()].c_str());
		}

		strncpy(text+lused,tval,len-lused);
		text[len-1]=0;
		return strlen(text);
	}
	else
		return MenuEntry::MenuText(text,len);
}

//===================================================================
// MenuEntryEnumEdit


MenuEntryEnumEdit::MenuEntryEnumEdit( char *text, int *t, int n, char **l, int *v )
	: MenuEntryListEdit(text,t,n,l)
{
	vals=v;
}

MenuEntryEnumEdit::MenuEntryEnumEdit( char *text, int *t, int n, char **l, int *v, char *f )
	: MenuEntryListEdit(text,t,n,l,f)
{
	vals=v;
}


void MenuEntryEnumEdit::SetValue( int n )
{
	if ( n>=0 && n<=hi )
	{
		MenuEntryIntEdit::SetValue(vals[n]);
	}
}

int MenuEntryEnumEdit::GetValue( void )
{
	int mv=MenuEntryIntEdit::GetValue();
	for ( int i=0; i<=hi; i++ )
	{
		if ( mv==vals[i] )
			return i;
	}
	return 0;
}


