#include "global.h"	// For all files
#include "menu.h"	// For menu structures
#include "entity.h"
#include "wave.h"
#include "wds.h"

MenuEntryIntEdit *	player1Score = NULL;
MenuEntryIntEdit *	player2Score = NULL;
MenuEntryFunction *	cheat_goal0 = NULL;
MenuEntryFunction *	cheat_goal1 = NULL;
MenuEntryFunction *	cheat_goal2 = NULL;
MenuEntryFunction *	cheat_goal3 = NULL;
MenuEntryFunction *	cheat_goal4 = NULL;

class MenuCheat : public Menu
{
public:
	MenuCheat(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuCheat() {};

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnClose( bool toparent );
};

MenuCheat *MENCHEAT_Menu=NULL; //(NULL, 0, NULL);
Submenu *menu_cheat = NULL; //Submenu("Entities", &MENUDRAW_MenuEntities);

MENUENTRYBUTTONFUNCTION(CompleteGoal0Button)
{
	if (g_beach_ptr)
		g_beach_ptr->complete_goal(0);

	return true;
}

MENUENTRYBUTTONFUNCTION(CompleteGoal1Button)
{
	if (g_beach_ptr)
		g_beach_ptr->complete_goal(1);

	return true;
}

MENUENTRYBUTTONFUNCTION(CompleteGoal2Button)
{
	if (g_beach_ptr)
		g_beach_ptr->complete_goal(2);

	return true;
}

MENUENTRYBUTTONFUNCTION(CompleteGoal3Button)
{
	if (g_beach_ptr)
		g_beach_ptr->complete_goal(3);

	return true;
}

MENUENTRYBUTTONFUNCTION(CompleteGoal4Button)
{
	if (g_beach_ptr)
		g_beach_ptr->complete_goal(4);

	return true;
}

void MENUCHEAT_StaticInit( void )
{
	
	player1Score = NULL;
	player2Score = NULL;
	cheat_goal0 = NULL;
	cheat_goal1 = NULL;
	cheat_goal2 = NULL;
	cheat_goal3 = NULL;
	cheat_goal4 = NULL;
	MENCHEAT_Menu = NEW MenuCheat(NULL, 0, NULL);
	menu_cheat = NEW Submenu("Cheat Menu", MENCHEAT_Menu);
}

void MenuCheat::OnClose( bool toparent )
{
	mem_lock_malloc(false);

	if (player1Score)
		delete player1Score;
	player1Score=NULL;

	if (player2Score)
		delete player2Score;
	player2Score=NULL;

	if (cheat_goal0)
		delete cheat_goal0;
	cheat_goal0 = NULL;
	
	if (cheat_goal1)
		delete cheat_goal1;
	cheat_goal1 = NULL;
	
	if (cheat_goal2)
		delete cheat_goal2;
	cheat_goal2 = NULL;
	
	if (cheat_goal3)
		delete cheat_goal3;
	cheat_goal3 = NULL;
	
	if (cheat_goal4)
		delete cheat_goal4;
	cheat_goal4 = NULL;

	ClearMenu();

	mem_lock_malloc(true);
	Menu::OnClose(toparent);
}


void MenuCheat::OnOpen(Menu *cto, MenuSystem *c)
{

	mem_lock_malloc(false);

	ClearMenu();

	if (g_world_ptr)
	{
		player1Score = NEW MenuEntryIntEdit("Player 1s Score", &g_world_ptr->get_ks_controller(0)->get_my_scoreManager().score,
																			0, 999999999, 5000);
		AddEntry(player1Score);
		if (g_game_ptr->get_num_players() == 2)
		{
			player2Score = NEW MenuEntryIntEdit("Player 2s Score", &g_world_ptr->get_ks_controller(1)->get_my_scoreManager().score,
																			0, 999999999, 5000);
			AddEntry(player2Score);
		}
	}

	cheat_goal0 = NEW MenuEntryFunction("Complete Goal 0", CompleteGoal0Button);
	AddEntry(cheat_goal0);
	cheat_goal1 = NEW MenuEntryFunction("Complete Goal 1", CompleteGoal1Button);
	AddEntry(cheat_goal1);
	cheat_goal2 = NEW MenuEntryFunction("Complete Goal 2", CompleteGoal2Button);
	AddEntry(cheat_goal2);
	cheat_goal3 = NEW MenuEntryFunction("Complete Goal 3", CompleteGoal3Button);
	AddEntry(cheat_goal3);
	cheat_goal4 = NEW MenuEntryFunction("Complete Goal 4", CompleteGoal4Button);
	AddEntry(cheat_goal4);
	
	mem_lock_malloc(true);
	c->Refresh();
	Menu::OnOpen(cto, c);
}