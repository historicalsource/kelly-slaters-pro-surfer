// MainFrontEnd.h

#ifndef MAINFRONTEND_H
#define MAINFRONTEND_H

#include "FEMenu.h"
#include "GraphicalMenuSystem.h"

#if defined(TARGET_PS2)
#define BETH_MOVIE
#endif /* TARGET_PS2 */

#ifdef BETH_MOVIE
#include "hwosps2/ps2_m2vplayer.h"
#endif

class OptionsMenu;
class MultiplayerMenu;
class FreesurfMenu;
class CareerMenu;
class MultiSubMenu;
class GraphicalMenuSystem;



class MainFrontEnd : public FEMultiMenu
{
public:
	enum {	MainFreeEntry, 
			MainCareerEntry, 
			MainMultiEntry, 
			MainExtrasEntry,
			MainOpEntry,
			MainEnd };
private:
	FEManager* manager;
	GraphicalMenuSystem* sys;
	static const int num_entries = MainEnd;

	FEMenuEntry* entry_list[num_entries];
	entity* ents[num_entries];

	static const int num_boxes = 9;
	static const int num_circles = 1;
	static const int num_lines = 3;
	PanelQuad* boxes[MainEnd][num_boxes];
	PanelQuad* circles[MainEnd][num_circles];
	PanelQuad* lines[MainEnd][num_lines];

public:
	OptionsMenu* Options;
	MultiplayerMenu* Multi;
	FreesurfMenu* Freesurf;
	CareerMenu* career_menu;
	MultiSubMenu* multi_sub;

public:
	MainFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~MainFrontEnd();
	virtual void Init();
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void UpdateInScene();
	virtual void Draw();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnCross(int c);
	virtual void OnAnyButtonPress(int c, int b); 
	virtual void OnTriangle(int c);
	virtual void OnActivate();
	virtual void MakeActive(FEMenu* a);
	virtual void Select() { Select(highlighted->entry_num); }
	virtual void Select(int entry_index);
	void FormatBIO(Font *fon, int maxWidth, char *in, int inSize, char *out, int outSize, float scale);
	int returnToHighlighted;

	void ResizeCareerBox(int h);
private:
	void setOpPQ(bool on);
	void SetPQIndices();
	void UpdateHighlight();
};

// **********************************************

class CareerMenu : public FEMenu
{
public:
	enum { ContinueEntry, NewEntry, LoadEntry, SaveEntry, OKEntry, CancelEntry, NumEntries };
	FEMenuEntry* entry[NumEntries];
	BoxText* warning;
	bool warning_up;
	static const int spacing = 17;
	GraphicalMenuSystem* sys;
	int warning_bottom;
	int regular_bottom;

public:
	CareerMenu(FEMenuSystem* s, int x, int y);
	virtual ~CareerMenu() { delete warning; }
	virtual void Select(int entry_index);
	virtual void OnActivate();
	virtual void Draw();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnCross(int c) { Select(highlighted->entry_num); }
	virtual void OnTriangle(int c);
	void SetBoxBottom(int b) { regular_bottom = b; }

private:
	void StartWarning();
	void EndWarning();
};

/******************************************************/


class OptionsMenu : public FEGraphicalMenu
{
public:
	enum {	OpScoreEntry,
			OpRumble1Entry,
			OpRumble2Entry,
			OpRumble3Entry,
			OpRumble4Entry,
			OpStereoEntry,
			OpMasterEntry,
			OpAmbientEntry,
			OpSFXEntry,
			OpVoiceEntry,
			OpMusicEntry,
			OpEnd };

private:
	static const int MAX_VOLUME = 10;   // volume is between 0-10
	static const int NUM_VOLUME = 5;
	static const int first_vol = OpMasterEntry;
	static const int num_switches = OpStereoEntry;

	PanelQuad* levels[NUM_VOLUME][MAX_VOLUME];
	PanelQuad* lights[OpEnd];
	PanelQuad* back;
	
	float rumbleTimer[4];

	int stereo_op;					// 0 = mono, 1 = stereo, 2 = prologic
	TextString* stereo_text[3];		// 3 settings, indexed by stereo_op
	TextString* switch_text[num_switches][2];	// for score, rum1 - rum4, with on/off
	TextString* options_text;
	TextString* slevels_text;
	
	int volumes[NUM_VOLUME];

	FEMenuEntry* options_list[OpEnd];
	bool switches[num_switches];

public:
	OptionsMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
	{ cons(s, man, p, pf_name); }
	virtual ~OptionsMenu();

	virtual void Load();
	virtual void Draw();
	virtual void Update(time_value_t time_inc);
	virtual void Select() {}
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
	virtual void setPQMain(bool on, bool from_main=false);

	// overrides so it doesn't look in parent's panel
	virtual PanelQuad* GetPointer(const char* s) { return panel.GetPointer(s); }
protected:
	virtual void cons(FEMenuSystem* s, FEManager* man, stringx path, stringx pf_name);
private:
	void RumbleOn(bool on, int controller);
	void ChangeSwitch(bool up);
	void ChangeVolume(bool up);
	void SetPQIndices();
	void MaskVolume(int index);

};

/******************************************************/

class MultiplayerMenu : public FEMenu
{
public:
	enum
	{
		MultiPushEntry,
		MultiHeadToHeadEntry,
		MultiTimeAttackEntry,
		MultiEnd
	};
	GraphicalMenuSystem* sys;
	FEMenuEntry* entry[MultiEnd];
	static const int spacing = 17;

public:
	MultiplayerMenu(FEMenuSystem* s, int x, int y);
	virtual void OnActivate();
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void Update(time_value_t time_inc);
	virtual void Select(int entry_index);
	virtual void OnCross(int c) { Select(highlighted->entry_num); }
	virtual void OnTriangle(int c);

private:
	void UpdateDisable();
};

/******************************************************/

class MultiSubMenu : public FEMenu
{
public:
	enum
	{
		SubEasyEntry,
		SubMediumEntry,
		SubHardEntry,
		SubEnd
	};
	GraphicalMenuSystem* sys;
	FEMenuEntry* entry[SubEnd];
	TextString* difficulty;

	static const int spacing = 17;
	int push[SubEnd];
	int time[SubEnd];

public:
	MultiSubMenu(FEMenuSystem* s, int x, int y);
	virtual ~MultiSubMenu() { delete difficulty; }
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void Select(int entry_index);
	virtual void OnCross(int c) { Select(highlighted->entry_num); }
	virtual void OnTriangle(int c);
};

/******************************************************/

class FreesurfMenu : public FEMenu
{
public:
	enum
	{
		FreeRegularEntry,
		FreeHighScoreEntry,
		FreeIconEntry,
		FreeEnd
	};
	FEMenuEntry* entry[FreeEnd];
	GraphicalMenuSystem* sys;
	static const int spacing = 17;

public:
	FreesurfMenu(FEMenuSystem* s, int x, int y);
	virtual void OnActivate();
	virtual void Select(int entry_index);
	virtual void OnCross(int c) { Select(highlighted->entry_num); }
	virtual void OnTriangle(int c);
};


#endif
