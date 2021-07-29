
#ifndef _ksdbmenu_h
#define _ksdbmenu_h

#include "menusys.h"
#include "menu.h"

class KSMainMenu : public Menu
{
public:
	KSMainMenu( Menu *p ) : Menu(p) {}
	KSMainMenu( Menu *p, int ents, pMenuEntry *ent ) : Menu(p,ents,ent) {}
	virtual ~KSMainMenu() { Close(); }

protected:
	virtual void OnOpen( Menu *cto, MenuSystem *c );
	virtual void OnClose( bool toparent );
};


extern MenuSystem *menus;
extern KSMainMenu *menu_main;
//extern Menu menu_main;
extern Menu *fakefrontend;
void KSDBMENU_InitMainMenu( void );
void KSDBMENU_KillMainMenu( void );

#endif

