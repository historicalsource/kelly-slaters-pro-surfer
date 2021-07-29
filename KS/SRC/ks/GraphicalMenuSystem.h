// GraphicalMenuSystem

#ifndef GMENUSYSTEM_H
#define GMENUSYSTEM_H



#include "IGOFrontEnd.h"
#include "MainFrontEnd.h"
#include "SurferFrontEnd.h"
#include "BeachFrontEnd.h"
#include "SaveLoadFrontEnd.h"
#include "TrickBookFrontEnd.h"
#include "StatsFrontEnd.h"
#include "FrontEndMenus.h"
#include "BoardFrontEnd.h"
#include "ExtrasFrontEnd.h"
#include "MultiFrontEnd.h"
#include "HighScoreFrontEnd.h"
#include "PhotoFrontEnd.h"
#include "TutorialFrontEnd.h"
#include "CheatFrontEnd.h"
#include "AccompFrontEnd.h"
#ifdef TARGET_GC
#include "GCMCDetectFrontEnd.h"
#else
#include "MCDetectFrontEnd.h"
#endif
#include "PAL60FrontEnd.h"
#include "LogbookFrontEnd.h"
#include "ksnvl.h"

class TrickBookFrontEnd;
class StatsFrontEnd;
class BeachFrontEnd;
class MainFrontEnd;
class IGOFrontEnd;
class SurferFrontEnd;
class BoardFrontEnd;
class ExtrasFrontEnd;
class MultiFrontEnd;
class CreditsFrontEnd;
class AccompFrontEnd;
class LogbookFrontEnd;

class GraphicalMenuSystem;

enum {	FEDB_Surfer,
		FEDB_Board,
		FEDB_Room,
		FEDB_Overlays,
		FEDB_Language,
		FEDB_UserCam,
		FEDB_NormCam,
		FEDB_End };

class FEDebugMenu : public FEMenu
{
private:
	FEManager* man;

public:
	FEDebugMenu(FEMenuSystem* s, FEManager* m) { man = m; FEMenu::cons(s, 150, 150, color32(255, 255, 255, 255), color32(255, 255, 0, 255)); }
	
	void AddAll();
	virtual void OnCross(int c) { Select(highlighted->entry_num); }
	virtual void Select(int entry_index);
};


class LegalFrontEnd : public FEGraphicalMenu
{
private:
//	PanelQuad* bkg;
	BoxText* legal_babble;
	float timer;

public:
	LegalFrontEnd(FEManager* man, stringx p, stringx pf_name);
	~LegalFrontEnd();
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void Load();
	virtual void OnStart(int c) { Select(0); }
	virtual void OnCross(int c) { Select(0); }
	virtual void Select(int n);
	virtual void SetSystem(FEMenuSystem* s);
};


class TitleFrontEnd : public FEGraphicalMenu
{
  friend class DemoModeManager;

private:
	PanelQuad* bkg, *box[9];
	TextString* loading;
	int loading_draw_counter;
#ifdef TARGET_GC
	GCMCDetectFrontEnd *mc;
#else
	MCDetectFrontEnd *mc;
#endif
	
public:
	TitleFrontEnd(/*FEMenuSystem* s, */FEManager* man, stringx p, stringx pf_name);
	virtual ~TitleFrontEnd();

	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void Load();
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
#ifdef TARGET_XBOX
	virtual void OnTriangle(int c);
#endif
	virtual void OnStart(int c) { Select(0); }
	virtual void OnCross(int c) { Select(0); }
	virtual void Select(int n);
	virtual void SetSystem(FEMenuSystem* s);
};


class HelpbarFE : public FrontEnd
{
private:
	enum {
		ARROW_H,
		ARROW_V,
		ARROW_BOTH,
		CROSS,
		TRIANGLE,
		CIRCLE,
		SQUARE,
		BTN_NUM };

	PanelQuad* buttons[BTN_NUM];
	TextString* help_text[BTN_NUM];
	bool has_text[BTN_NUM];
	stringx default_text[BTN_NUM];
	bool disabled;
	static const int start_x = 423;
	static const int x_spacing = 118;

public:
	HelpbarFE(FEManager* man, stringx p, stringx pf_name);
	virtual ~HelpbarFE();
	
	virtual void Update(time_value_t time_inc) {}
	virtual void LoadPanel(bool floating=false);
	virtual void ReloadPanel() { FrontEnd::ReloadPanel(); }
	virtual void Draw();

	// for these add functions, give no argument to use default text
	// best to use arrow_both, unless they do different things or one isn't used
	void AddArrowBoth(stringx text=stringx(""))	{ ChangeBtn(ARROW_BOTH, true, text);  ChangeBtn(ARROW_H, false, ""); ChangeBtn(ARROW_H, false, "");}
	void AddArrowH(stringx text=stringx(""))	{ ChangeBtn(ARROW_H, true, text); ChangeBtn(ARROW_BOTH, false, ""); }
	void AddArrowV(stringx text=stringx(""))	{ ChangeBtn(ARROW_V, true, text); ChangeBtn(ARROW_BOTH, false, ""); }
	void AddCircle(stringx text=stringx(""))	{ ChangeBtn(CIRCLE, true, text); }
	void AddSquare(stringx text=stringx(""))	{ ChangeBtn(SQUARE, true, text); }
	void RemoveTriangle()						{ ChangeBtn(TRIANGLE, false, ""); }
	void RemoveX()								{ ChangeBtn(CROSS, false, ""); }
	void RemoveArrowBoth()						{ ChangeBtn(ARROW_BOTH, false, ""); }
	void SetXText(stringx text)					{ ChangeBtn(CROSS, true, text); }
	void SetTriangleText(stringx text)			{ ChangeBtn(TRIANGLE, true, text); }
	void DisableHelpbar()						{ disabled = true; }

	// resets to defaults: triangle, x, and arrow_both are on with
	// default text; others are off
	// this should always be called when activating a new menu, 
	// before the other functions are called)
	void Reset();

	// *needs* to be called when helpbar is changed; call after
	// all reset, add & remove functions
	void Reformat();

private:
	void SetPQIndices();
	void ChangeBtn(int index, bool add, stringx text);
};


class GraphicalMenuSystem : public FEMenuSystem
{
private:
	bool exiting;
	bool play_fe_movie_on_exit;

public:
	enum {	
			MainMenu, 
			ExtrasMenu,
			CreditsMenu,
			HighScoreMenu,
			SurferMenu, 
			BeachMenu, 
			SaveLoadMenu, 
			TrickBookMenu,
			BoardMenu,
			AccompMenu,
			LogbookMenu,
			TitleMenu,/*keep me 3rd to last*/
			Legal,		/*keep me second to last*/
			OptionsMenu /*keep me last*/ };
	
	bool is_loaded[OptionsMenu+1];
	bool multiplayer;
	bool multi_1;		// first player selection (for SurferSelect & BoardSelect)
	int beach;
	bool usercam;
	bool prepare_to_make_active;
	bool skip_flyin;
	
	int first_screen;		// changes depending on if coming from in-game
	int first_screen_sub;

	nglTexture *back_movie_tex;
	nvlMovieSource *back_movie_source;
	nvlMovie *back_movie;
	void *back_movie_buffer;
	
	// another movie (not the same one)
	nglTexture *another_movie_tex;
	nvlMovieSource *another_movie_source;
	nvlMovie *another_movie;

	bool fedbm_up;
	FEDebugMenu* fedbm;
	bool fedb_draw_overlays;

	GraphicalMenuSystem(FEManager* man);
	virtual ~GraphicalMenuSystem();
	virtual void UpdateInScene();
	virtual void Update(time_value_t time_inc);
	virtual void DrawMovie();
	virtual void DrawTop();
	virtual void Draw();
	virtual void LoadAll();
	virtual void Reload() {}
	void Select(int menu_index, int entry_index) { menus[menu_index]->Select(entry_index); }
	void killMovie();
	void restartMovie();
	void PrepareToExit( bool exit_to_movie=false ) { exiting = true; play_fe_movie_on_exit = exit_to_movie; }
	void Exit();

	virtual void OnButtonPress(int button, int controller);
	
private:
	bool soundStarted;
	bool LoadedAll;
	// cheat codes
	bool get_one_button_down (control_id_t& btn) const;
	void check_for_cheats (float time_inc);
	
	float cheat_timer;
	bool cheat_release;
	int cheat_index;
	
#define MAX_CHEAT_SIZE 16
	control_id_t cheat_buffer[MAX_CHEAT_SIZE];
};





#endif
