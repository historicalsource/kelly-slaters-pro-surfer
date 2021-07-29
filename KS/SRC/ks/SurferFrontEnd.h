// Surfer Select Front End Screen

#ifndef SURFERFRONTEND_H
#define SURFERFRONTEND_H


#include "FEMenu.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "surferdata.h"	// For SURFER_LAST
#include "osgamesaver.h"

class GraphicalMenuSystem;
class SurferBioFrontEnd;
class CareerMenu;
class TutorialMenu;

class SurferFrontEnd : public FEMultiMenu
{
public:
	enum {	ACT_SURFER = 1,
			ACT_CAREER = 2,
			ACT_TUTORIAL = 3,
	};

private:
	enum {	OnSurfer,
			BioHigh,
			TrickHigh,
			PersHigh,
			ScrapHigh,
			ContHigh,
			HCapHigh,
	};

	// display state
	enum {	DISP_SELECT,	// Surfer Select
			DISP_HANDICAP,	// Surfer Select w/Handicap
			DISP_STATS,		// Surfer Stats
	};
	
	int disp_state;
	int state;

	enum {  Pickable,
			Viewable,
			Hidden };
	int availability[SURFER_LAST];
	bool personality_unlocked[SURFER_LAST];
	GraphicalMenuSystem* sys;
	
	bool first_time_through;
	FEMenuEntry* current_surfer;  // the surfer highlighted now
	FEMenuEntry* ks;
	FEMenuEntry* Bio;
	FEMenuEntry* Trick;
	FEMenuEntry* Personality;
	FEMenuEntry* Continue;
	FEMenuEntry* ScrapBook;
	FEMenuEntry* Handicap;
	static const int MAX_WAVES = 8;
	TextString* players[2];
	TextString* surfer_select;
	TextString* firstname;
	TextString* gauge_labels[4];
	PanelQuad* gauges[4][3];
	PanelQuad* red_gauges[4];
	PanelQuad* horiz_arrows[2][2];
	PanelQuad* ss_lines[3];
	PanelQuad* ss_box[9];
	PanelQuad* hcap_gauge, *hcap_slider, *hcap_color[3];

	// 1 for all characters, 2 for bio & trick
	entity* ents[3];
	FEManager* manager;
	bool wait_for_camera;
	int hcap;
	static const int hcap_max = MAX_HANDICAP;
	static const int hcap_slider_x = 460;
	static const int hcap_slider_y = 267;

	static const int arrow_timer = 3;
	int arrow_counter;
	int arrow_num;

	SurferBioFrontEnd* bio_menu;
	bool in_tb_or_bio;
	float progressval;

	int most_recent_controller;

public:
	int current_surfer_index;
	static bool personality_up;
	
public:
	SurferFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf);
	virtual ~SurferFrontEnd();
	virtual void Load();
	virtual void Select(int entry_index);
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnAnyButtonPress(int c, int b);
	virtual void OnButtonRelease(int c, int b) { if(active) active->OnButtonRelease(c, b); }
private:
	void Pick(int index);
	void SetPQIndices();

	void OnHighlightHero(int old_index=-1);
	void MaskWave(int wave);
	void SetPersonality(bool up);
	void ToSurfer(bool to);
	void Up();
	void Down();
	void checkHigh(bool set_state);
	void SetState(int s);
	void SaveCareer();
	bool BioAvailable();
	void SetDisplay(int d_state);
	void AdjustHandicap(bool left);
	void AdjustHandicapAbso(int absolute);

public:
	static stringx getName(int index);
	static stringx getAbbr(int index);
	static bool getPersonalityUp() {return personality_up;};
	void TurnPQ(bool on);
};


// **********************************************

class SurferBioFrontEnd : public FEMultiMenu
{
private:
	static const int BIO_VIS_LINES = 11; // 10;
	PreformatText* bios[SURFER_LAST];
	GraphicalMenuSystem* sys;
	SurferFrontEnd* bio_parent;

	TextString* firstname;
	TextString* lastname;
	BoxText* intro;

	PanelQuad* images[SURFER_LAST];
	PanelQuad* scroll_marker;
	bool wait_for_camera;
	bool up_pressed, down_pressed;

	static const int count_max = 3;
	int counter;

	float scroll_marker_x;
	float scroll_marker_y_t;
	float scroll_marker_y_b;

public:
	SurferBioFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf, SurferFrontEnd* sfe);
	virtual ~SurferBioFrontEnd();
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void Select(int entry_index) {}

	// overridden to avoid using parent's panel
	virtual PanelQuad* GetPointer(const char* sn) { return FrontEnd::GetPointer(sn); }
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnUp(int c) { up_pressed = true; counter = count_max; }
	virtual void OnDown(int c) { down_pressed = true; counter = count_max; }
	virtual void OnLeft(int c) {}
	virtual void OnRight(int c) {}
	virtual void OnCross(int c) {}
	virtual void OnTriangle(int c);
	virtual void OnButtonRelease(int c, int b);

private:
	void SetPQIndices();
	void UpdateScrollbar();
};

#endif
