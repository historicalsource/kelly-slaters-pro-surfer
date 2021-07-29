// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef _XBOX
#include "hwrasterize.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "menusys.h"
#include "menu.h"


MenuColor menubackground = { 0, 0, 0, 192 };
MenuColor menuborder     = { 255, 255, 255, 192 };

//const int menuborderwidth = 2;
//const int entry0offx = 10;
//const int entry0offy = 10;
const int menuborderwidth = 1;
const int entry0offx = 1;
const int entry0offy = 1;


MenuRender::MenuRender( MenuRect &mlim )
{
	memcpy(&limit, &mlim, sizeof(MenuRect));
	memcpy(&inuse, &mlim, sizeof(MenuRect));
	for ( int i=0; i<maxmenulines; i++ )
	{
		strcpy(text[i],"");
		state[i]=0;
	}
	active=0;
	topentry=0;
	//MENU_InitMenus();
}


void MenuRender::Draw( void )
{
	if ( active )
	{
		MENU_DrawStart();
		DrawFrame(inuse.x0, inuse.y0, inuse.x1, inuse.y1);
		int x=inuse.x0+(entry0offx * MENU_BorderSpaceFactor());
		int y=inuse.y0+(entry0offy * MENU_BorderSpaceFactor())+(menuborderwidth * MENU_BorderSizeFactor());
		for ( int i=0; i<active; i++ )
		{
			if ( state[i] & MENTRY_VISIBLE )
			{
				#if 1
					MENU_DrawTextAt( x,y,text[i],GetElementColor(topentry+i) );
				#else
				if ( state[i] & MENTRY_ACTIVE )
				{
					MENU_DrawTextAt( x,y,text[i],entryactive );
				}
				else if ( state[i] & MENTRY_ENABLED )
				{
					MENU_DrawTextAt( x,y,text[i],entryenabled );
				}
				else
				{
					MENU_DrawTextAt( x,y,text[i],entrydisabled );
				}
				#endif
				y+=MENU_TextHeight(text[i])+MENU_TextSeparation();
			}
		}
		MENU_DrawEnd();
	}
}

void MenuRender::Clear( void )
{
	MENU_ClearRect(limit.x0,limit.y0,limit.x1,limit.y1);
}


void MenuRender::DrawFrame( int x0, int y0, int x1, int y1 )
{
	MENU_DrawRect(x0,y0,x1,y1,menubackground);
	MENU_DrawRect(x0,y0,x0+(menuborderwidth * MENU_BorderSizeFactor()),y1,menuborder);
	MENU_DrawRect(x0,y0,x1,y0+(menuborderwidth * MENU_BorderSizeFactor()),menuborder);
	MENU_DrawRect(x1-(menuborderwidth * MENU_BorderSizeFactor()),y0,x1,y1,menuborder);
	MENU_DrawRect(x0,y1-(menuborderwidth * MENU_BorderSizeFactor()),x1,y1,menuborder);
}

void MenuRender::OpenMenu( Menu *m )
{
	topentry=0;
	active=0;
	memcpy(&inuse,&limit,sizeof(inuse));
	for ( int i=0; i<maxmenulines; i++ )
	{
		strcpy(text[i],"");
		state[i]=0;
	}
}

void MenuRender::CloseMenu( void )
{
	for ( int i=0; i<maxmenulines; i++ )
	{
		strcpy(text[i],"");
		state[i]=0;
	}
	active=0;
}

void MenuRender::Refresh( void )
{
	// adjust the top of scrolling menu entries for scrolling off the top
	int aent=0;
	for ( int j=0; j<TotalEntries(); j++ )
	{
		if ( GetElementState(j) & MENTRY_ACTIVE )
		{
			aent=j;
			break;
		}
	}
	if ( aent < topentry )
	{
		topentry=aent;
	}

	// adjust the top of scrolling menu entries for scrolling off the bottom
	bool checked=false;
	while ( !checked )
	{
		checked=true;
		int acount=0;
		int y=inuse.y0+2*(entry0offy * MENU_BorderSpaceFactor())+2*(menuborderwidth * MENU_BorderSizeFactor());
		for ( int k=topentry; k<TotalEntries(); k++ )
		{
			char ttext[maxmenulinelength];
			GetElementText(k,ttext,maxmenulinelength);
			int state=GetElementState(k);
			if ( state & MENTRY_ACTIVE )
			{
				if ( acount+1 >= maxmenulines || y + MENU_TextHeight(ttext)+(active>0?MENU_TextSeparation():0) > limit.y1)
				{
					topentry++;
					checked=false;
					assert(topentry<TotalEntries());
				}
			}
			if ( state & MENTRY_VISIBLE )
			{
				acount++;
				y+=MENU_TextHeight(ttext)+(active>0?MENU_TextSeparation():0);
			}
		}
	}
	// update the states and strings for all the active entries
	active=0;
	inuse.y1=inuse.y0+2*(entry0offy * MENU_BorderSpaceFactor())+2*(menuborderwidth * MENU_BorderSizeFactor());
	for ( int i=topentry; i<TotalEntries(); i++ )
	{
		state[active]=GetElementState(i);
		if ( state[active] & MENTRY_VISIBLE )
		{
			GetElementText(i,text[active],maxmenulinelength);
			if ( inuse.y1 + MENU_TextHeight(text[i])+(active>0?MENU_TextSeparation():0) > limit.y1 )
				break;
			if ( active+1 >= maxmenulines )
				break;
			active++;
			inuse.y1+=MENU_TextHeight(text[i])+(active>0?MENU_TextSeparation():0);
		}
/*
		else
		{
			int d=0;
		}
*/
	}
}

void dbut( int buttonid )
{
	switch ( buttonid )
	{
		case MENUCMD_NONE    : debug_print("Button %s pressed\n","MENUCMD_NONE    "); break;
		case MENUCMD_SELECT  : debug_print("Button %s pressed\n","MENUCMD_SELECT  "); break;
		case MENUCMD_START   : debug_print("Button %s pressed\n","MENUCMD_START   "); break;
		case MENUCMD_UP      : debug_print("Button %s pressed\n","MENUCMD_UP      "); break;
		case MENUCMD_DOWN    : debug_print("Button %s pressed\n","MENUCMD_DOWN    "); break;
		case MENUCMD_LEFT    : debug_print("Button %s pressed\n","MENUCMD_LEFT    "); break;
		case MENUCMD_RIGHT   : debug_print("Button %s pressed\n","MENUCMD_RIGHT   "); break;
		case MENUCMD_CROSS   : debug_print("Button %s pressed\n","MENUCMD_CROSS   "); break;
		case MENUCMD_TRIANGLE: debug_print("Button %s pressed\n","MENUCMD_TRIANGLE"); break;
		case MENUCMD_SQUARE  : debug_print("Button %s pressed\n","MENUCMD_SQUARE  "); break;
		case MENUCMD_CIRCLE  : debug_print("Button %s pressed\n","MENUCMD_CIRCLE  "); break;
		case MENUCMD_L1      : debug_print("Button %s pressed\n","MENUCMD_L1      "); break;
		case MENUCMD_R1      : debug_print("Button %s pressed\n","MENUCMD_R1      "); break;
		case MENUCMD_L2      : debug_print("Button %s pressed\n","MENUCMD_L2      "); break;
		case MENUCMD_R2      : debug_print("Button %s pressed\n","MENUCMD_R2      "); break;
	}
}

void MenuInput::CheckButtonStates( void )
{
	for ( int i=0; i<MAXMENUCMD; i++ )
	{
		int state=MENU_GetButtonState(i);
		if ( state!=0 && bstate[i]==0 )
		{
			//dbut(i);
			ButtonPress(i);
			bstate[i]=1;
		}
		else if ( state==0 && bstate[i]!=0 )
		{
			ButtonRelease(i);
			bstate[i]=0;
		}
	}
}

void MenuInput::InitButtonStates( void )
{
	for ( int i=0; i<MAXMENUCMD; i++ )
	{
		int state=MENU_GetButtonState(i);
		bstate[i]=state;
	}
}

void MenuSystem::OpenMenu( Menu *m )
{
	MenuRender::OpenMenu(m);
	m->Open(NULL,this);
	Refresh();
	InitButtonStates();
}

void MenuSystem::CloseMenu( void )
{
	if ( curmenu )
		curmenu->Close();
	MenuRender::CloseMenu();
}

void MenuSystem::CloseAllMenus( void )
{
	if ( curmenu )
		curmenu->CloseAll();
}


void MenuSystem::ButtonPress( int buttoninfo )
{
	if ( curmenu )
		curmenu->ButtonPress(buttoninfo);
}

void MenuSystem::ButtonRelease( int buttoninfo )
{
	if ( curmenu )
		curmenu->ButtonRelease(buttoninfo);
}

void MenuSystem::Tick( float dtime )
{
	if ( curmenu )
		curmenu->OnTick(dtime);
	CheckButtonStates();
}



void MenuSystem::Opening( Menu *menu )
{
	assert(curmenu==NULL);
	curmenu=menu;
	Clear();
}

void MenuSystem::Closing( Menu *menu )
{
	Clear();
	assert(curmenu==menu);
	curmenu=NULL;
}



int MenuSystem::TotalEntries( void )
{
	if ( curmenu )
		return curmenu->NumEntries();
	return 0;
}

MENU_UINT32 MenuSystem::GetElementState( int i )
{
	if ( curmenu )
		return curmenu->GetElementFlags(i);
	return 0;
}

void MenuSystem::GetElementText( int i, char *t, int len )
{
	#if 0
	char tmp[maxmenulinelength];
	if ( curmenu )
		curmenu->GetElementText(i,tmp,maxmenulinelength);
	else
		strcpy(tmp,"");
	memset(t,' ',len);
	int tlen=strlen(tmp);
	if ( len<tlen ) tlen=len;
	if ( len < Width() ) tlen=Width();

	memcpy(t,tmp,tlen);
	t[tlen]=0;
	#else
	if ( curmenu )
		curmenu->GetElementText(i,t,len);
	else
		strcpy(t,"");
	#endif
}

MenuColor MenuSystem::GetElementColor( int i )
{
	if ( curmenu )
		return curmenu->GetElementColor(i);
	else
		return menubackground;
}






