// High Score Front End Screen

#ifndef HIGHSCOREFRONTEND_H
#define HIGHSCOREFRONTEND_H


#include "FEMenu.h"
#include "entity_maker.h"
#include "entityflags.h"


class GraphicalMenuSystem;
class NameEntryMenu;

class HighScoreFrontEnd : public FEMultiMenu
{
public:
	static const int NUM_ROWS = 10;

private:
	GraphicalMenuSystem* sys;

	static const int NUM_COLUMNS = 5;
	TextString* column_labels[NUM_COLUMNS];
	TextString* info[NUM_ROWS][NUM_COLUMNS];

	PanelQuad* arrows[2][2];	// left, right; on, off
	PanelQuad* line;

	// to get the arrows to blink
	static const int arrow_timer = 3;
	int arrow_counter;
	int arrow_num;

	color32 labelColor;
	bool in_game;

	float flash_timer;

	// only for in-game
	int next_menu;
	FEMenuEntry* all_beaches[BEACH_LAST*2];
	NameEntryMenu* nem;
	PanelQuad* fade;
	PanelQuad* lines[19];

	int first_line;		// new high score line
	int second_line;	// new high score line, is beach high score only if icon high score as well

public:
	HighScoreFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name, bool i_g);
	virtual ~HighScoreFrontEnd();
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void Select(int entry_index);
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
	virtual void OnStart(int c);

	// only for in-game
	void SetNextMenu(int m) { next_menu = m; }
	void TurnHighScore(bool on);
	void SetString(stringx str);
	static bool IsNewHighScore(const int score, const int icons);

private:
	void SetPQIndices();
	void OnSwitchBeach();
};


class NameEntryMenu : public FEMultiMenu
{
private:
	PanelQuad* box[9];
	PanelQuad* line;
	PanelQuad* name_box[3];
	PanelQuad* keys[41][2];	// a-z, 0-9, back, space, clear(x3)

	bool in_game;

	TextString* high_score, *enter_name;
	TextString* name;
	int name_count;
	bool name_entered;
	bool canceled;
	FEMenuEntry* entry[41];

public:
	NameEntryMenu(FEMenuSystem* s, FEManager* man, const bool in_g);
	virtual ~NameEntryMenu();
	virtual void Select(int entry_index);
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnTriangle(int c) {}
	virtual void OnCross(int c) { Select(highlighted->entry_num); }
	virtual void Update(time_value_t time_inc);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnStart(int c);
	bool NameEntered() { return name_entered; }
	bool WasCanceled() { return canceled; }
	void ResetCanceled() { canceled = false; }
	stringx GetName() { return name->getText(); }
	void ClearName() { name->getText().copy(""); name_entered = false; }

	void SetPQIndices();
	void TurnNameEntry(bool on);
};


#endif