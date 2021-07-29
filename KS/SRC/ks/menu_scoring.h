
#ifndef INCLUDED_MENU_SCORING_H
#define INCLUDED_MENU_SCORING_H

#include "menu.h"
#include "trickdata.h"
#include "player.h"

class TrickMenu : public Menu 
{
public:
	TrickMenu(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~TrickMenu() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
};

#endif INCLUDED_MENU_SCORING_H