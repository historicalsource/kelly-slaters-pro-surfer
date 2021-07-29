// Extras Select Front End Screen

#ifndef EXTRASFRONTEND_H
#define EXTRASFRONTEND_H

#include "FEMenu.h"
#include "cheatfrontend.h"

class GraphicalMenuSystem;

class ExtrasFrontEnd : public FEMultiMenu
{
public:
	enum {	ExtrasBeachesEntry,
			ExtrasSurfersEntry,
			ExtrasTrailersEntry,
			ExtrasHighScoreEntry,
			ExtrasScrapbookEntry,
			ExtrasCheatsEntry,
			ExtrasCreditsEntry,
			ExtrasWebsitesEntry,
			ExtrasLogbookEntry,
#ifdef TARGET_PS2
			ExtrasTHDemoEntry,
#endif
			ExtrasDemoEntryYes,
			ExtrasDemoEntryNo,		
			ExtrasEndEntry };

private:
	int cur_index;
	GraphicalMenuSystem* sys;

	FEMenuEntry* entry[ExtrasEndEntry];
	FEMenu* surfers;
	FEMenu* beaches;
	FEMenu* trailers;
	TextString* extras;
	BoxText* prompt;
	BoxText* websites;
	CheatFrontEnd* cheat_menu;

	PanelQuad* videos[ExtrasEndEntry];
	PanelQuad* website_box[9];
	PanelQuad* arrows[2][2];	// left, right; on, off
	bool saveCareerPrompt;
	// to get the arrows to blink
	static const int arrow_timer = 3;
	int arrow_counter;
	int arrow_num;

public:
	ExtrasFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	~ExtrasFrontEnd();
	virtual void Load();
	virtual void Draw();
	virtual void Select(int entry_index);
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnRight(int c);
	virtual void OnLeft(int c);
	virtual void OnTriangle(int c);
	virtual void Update(time_value_t time_inc) { if(active) active->Update(time_inc); else FEMultiMenu::Update(time_inc); }

private:
	void SetPQIndices();
	void UpdateState();
	void UpdateHelpbar();
	void PlayMovie(int sbt);	// sbt = 0 for surfer, 1 for beach, 2 for trailer
};


// ***** credits screen ***************************

class CreditsFrontEnd : public FEMultiMenu
{
private:
	PreformatText* credits;
	GraphicalMenuSystem* sys;
	static const int num_lines = 300;
	float first_line_y;
	float x_all;
	bool up_pressed;
	bool down_pressed;

public:
	CreditsFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	~CreditsFrontEnd() { delete credits; }
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void Select(int entry_index) {}
	virtual void OnActivate();
	virtual void OnUp(int c) { up_pressed = true; }
	virtual void OnDown(int c) { down_pressed = true; }
	virtual void OnRight(int c) {}
	virtual void OnLeft(int c) {}
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnStart(int c);
	virtual void OnButtonRelease(int c, int b);
};


#endif
