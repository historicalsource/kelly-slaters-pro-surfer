// Extras Select Front End Screen

#ifndef __CHEATFRONTEND_H_
#define __CHEATFRONTEND_H_

#include "Cheat.h"
#include "FEMenu.h"
#include "HighScoreFrontEnd.h"

#define INITIAL_HIGHLIGHTED_NUMPAD 1

#define PHONE_TEXT_COLOR           color32(49, 67, 48, 255)
#define PHONE_TEXT_COLOR_HI        color32(73, 100, 72, 255)
//#define PHONE_TEXT_COLOR_HI        color32(126, 108, 84, 255)

#define PHONE_SCREEN_LEFT            377
#define PHONE_SCREEN_TOP             73
#define PHONE_SCREEN_RIGHT           (PHONE_SCREEN_LEFT + 125)
#define PHONE_SCREEN_BOTTOM          (PHONE_SCREEN_TOP  + 100)
#define PHONE_SCREEN_MIDDLE_X        ((PHONE_SCREEN_LEFT + PHONE_SCREEN_RIGHT) / 2)
#define PHONE_SCREEN_MIDDLE_Y        ((PHONE_SCREEN_TOP + PHONE_SCREEN_BOTTOM) / 2)
#define PHONE_TEXT_SIZE              0.666f // 12 pixels high
#define PHONE_TOGGLE_TEXT_SIZE       0.555f // 10 pixels high
//#define PHONE_TOGGLE_TEXT_SIZE       0.42f
#define MAX_CHEATS_PER_SCREEN        2  // the number of lines that'll fit in the phone display
#define PHONE_MAIN_MENU_VERT_SPACING 18 // spacing of the main cheat menu
#define PHONE_LIST_VERT_SPACING      32 // spacing of the toggle menu
#define PHONE_TOGLE_TOP_OFFSET		 ((PHONE_SCREEN_BOTTOM - PHONE_SCREEN_TOP - (PHONE_LIST_VERT_SPACING * MAX_CHEATS_PER_SCREEN)) / 2)
#define MYSTERY_KLUDGE_VALUE         1.0f // Kludge value for scaling the placement of the cursor (because Font::getWidth() doesn't seem to work properly)

// constants for the phone number display
#define NUM_PHONE_NUM_GAPS  2
#define PHONE_NUM_GAP_CHAR  ' '
#define MAX_PHONE_NUM_LEN   10
#define PHONE_CURSOR_PERIOD 0.25f
#define PHONE_CURSOR_INTENSITY 1.5f   // higher means that it will be on for a larger portion of the period

#define CHEAT_CODE_MESSAGE_DELAY    0.4f // The delay before putting up the success/failure message
#define CHEAT_CODE_MESSAGE_DURATION 1.0f // The duration of the success/failure message before returning to the main cheat menu


class CheatCodeMenu;
class EnterCheatMenu;

class CheatFrontEnd : public FEMultiMenu
{
private:
	enum
	{
		CHEAT_MENU_ENTER_CODE,
		CHEAT_MENU_TOGGLE_CHEAT,
	};

	GraphicalMenuSystem* sys;

	bool pq_indices_set; // true if SetPQIndices() has been called

	PanelQuad *arrow_up[2];   // off, on
	PanelQuad *arrow_down[2]; // off, on
	PanelQuad *cellphone;
	PanelQuad *numbers[10];
	PanelQuad *numbers_hi[10];

	CheatCodeMenu *code_menu;
	EnterCheatMenu *enter_code;
	FEMenuEntry *new_cheat_entry, *toggle_cheats_entry;
public:
	CheatFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~CheatFrontEnd();

	virtual void Update(time_value_t time_inc) { if(active) active->Update(time_inc); else FEMultiMenu::Update(time_inc); }
	virtual void Load();
	virtual void Draw();
	virtual void Select(int entry_index);
	virtual void Select();
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnRight(int c);
	virtual void OnLeft(int c);
	virtual void OnTriangle(int c);

	void TurnOnPhone(bool on);

private:
	void SetPQIndices();
};

///////////////////////////////////////////////////////////////////////////////////////
// CheatCodeMenu
//
// This is the menu listing all the currently unlocked cheat codes
///////////////////////////////////////////////////////////////////////////////////////

class CheatCodeMenu : public FEMultiMenu
{
private:
	FEMenuEntry *cheats[MAX_CHEATS_PER_SCREEN];        // one for every cheat we can display at a time
	int menu_entry_cheat_index[MAX_CHEATS_PER_SCREEN]; // -1 if there's no cheat open in that menu entry
	GraphicalMenuSystem* sys;
	int next_down, next_up;  // true if there are more entries listed in that direction

	TextString *toggle_state[MAX_CHEATS_PER_SCREEN];

	PanelQuad *arrow_up;  
	PanelQuad *arrow_down;

public:
	CheatCodeMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~CheatCodeMenu();

	virtual void Draw();
	virtual void Load();
	virtual void Select(int entry_index);
	virtual void Select();
	virtual void OnTriangle(int c) { parent->MakeActive(NULL); }
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);

private:
	void SetPQIndices();
	void ReOrderEntries(const int start);
	void ResetToggles();
	void ResetUpDownArrows();
};

///////////////////////////////////////////////////////////////////////////////////////
// EnterCheatMenu
//
// This is the menu for entering a new cheat code in the phone
///////////////////////////////////////////////////////////////////////////////////////

class EnterCheatMenu : public FEMultiMenu
{
private:
	GraphicalMenuSystem *sys;
	stringx             current_code;   // the current number being typed in
	MultiLineString     *code_display;  // the way the current code is displayed to the user
	TextString          *cursor;
	int                 current_button;
	PanelQuad           *numbers_hi[10];
	bool                pq_indices_set;

	static const int    phone_num_gap[NUM_PHONE_NUM_GAPS]; // the locations of the gaps/dashes in the phone number display

	bool closing;
	float closing_timer;
	int cheat_unlocked;  // > -1 if a cheat was just unlocked

public:
	EnterCheatMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~EnterCheatMenu();

	virtual void Update(time_value_t time_inc);
	virtual void Load();
	virtual void Select(int entry_index);
	virtual void Select();
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnRight(int c);
	virtual void OnLeft(int c);
	virtual void OnCross(int c) { Select(); }
	virtual void OnTriangle(int c);
	virtual void Draw();

private:
	void ChangeButton(int new_button);
	void SetPQIndices();
	void ExitMenu(float delay);  // delay is the amount of time to delay before putting up the "new cheat unlocked" message
};

#endif //__CHEATFRONTEND_H_
