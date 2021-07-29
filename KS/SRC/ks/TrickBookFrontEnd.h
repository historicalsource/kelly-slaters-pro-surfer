// Trick Book Front End Screen

#ifndef TRICKFRONTEND_H
#define TRICKFRONTEND_H

#include "FEMenu.h"
#include "trick_system.h"

class GraphicalMenuSystem;

class TrickBookFrontEnd : public FEMultiMenu
{
private:
	GraphicalMenuSystem* sys;

	// ignore TRICKBOOKTYPE_NOTYPE
	static const int NUM_TYPES = TRICKBOOKTYPE_NUMTYPES-1;
	SurferTrick** tricks[NUM_TYPES];
	TextString* buttons;
	TextString* trickbook;

	FEMenu* State2[NUM_TYPES];

	PanelQuad* arrows[2][2];
	PanelQuad* bkg;
	static const int arrow_timer = 3;
	int arrow_counter;
	int arrow_num;

	bool wait_for_camera;

public:
	TrickBookFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~TrickBookFrontEnd();
	virtual void Init();
	virtual void Load();
	virtual void Select(int entry_index);
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void DrawTop();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c) { FEGraphicalMenu::OnCross(c); }
	virtual void OnActivate();
private:
	void SetPQIndices();
	void ChangeButtonText(SurferTrick* st);

public:
	static bool TrickOK(int t);
};

#endif
