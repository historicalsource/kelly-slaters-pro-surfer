
// Toby Lael 10-12-2001

#ifndef INCLUDED_FRONTENDMENUS_H
#define INCLUDED_FRONTENDMENUS_H

#include "FEMenu.h"
#include "ksdbmenu.h"
#include "SoundScript.h"
#define SONGS_PER_SCREEN 6
class PauseMenuSystem;

/*
	README:

	From now on, there's only one MenuSystem in-game (PauseMenuSystem).
	Every menu in this system is now it's own class, including the Pause Menu
	When adding new menus, be sure to add them to the PauseMenuSystem,
	 and preferably create an enum listing the options in the menu (in order)
	 so that the menu functionality is clear.

*/

//************** Pause Menu ***************

class PauseMenuClass : public FEMenu
{
public:
	enum
	{
		Continue,
		Retry,
		Goals,
		Tip,
		Options,
		TrickBook,
		EndRun,
		ReturnToFE,
	};

public:
	PauseMenuSystem *	sys;
	FEMenuEntry *		goals;
	FEMenuEntry *		tip;
	FEMenuEntry *		options;
	FEMenuEntry *		trickbook;
	FEMenuEntry *		returnFE;

public:
	PauseMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnActivate();
	virtual void Select(int entry_index);
	virtual void OnButtonRelease(int c, int b);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnStart(int c);
};

//************** Time Attack Pause Menu ***************

//	Pops up when user hits START in the middle of Time Attack mode.
class TimeAttackPauseMenuClass : public FEMenu
{
public:
	enum
	{
		Continue,
		Options,
		TrickBook,
		EndRun,
		Restart,
		Quit,
		ReturnToFE,
	};
	PauseMenuSystem* sys;
	FEMenuEntry *options;
	FEMenuEntry *trickbook;

public:
	TimeAttackPauseMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnActivate(void);
	virtual void Select(int entry_index);
	virtual void OnButtonRelease(int c, int b);
	virtual void OnStart(int c);
};

//************** Tutorial Pause Menu ***************

//	Pops up when user hits START in the middle of Time Attack mode.
class TutorialPauseMenuClass : public FEMenu
{
public:
	enum
	{
		NextTip,
		Continue,
		Retry,
		Options,
		TrickBook,
		EndRun,
		ReturnToFE,
	};
	PauseMenuSystem* sys;
	FEMenuEntry *options;
	FEMenuEntry *trickbook;

public:
	TutorialPauseMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnActivate(void);
	virtual void Select(int entry_index);
	virtual void OnButtonRelease(int c, int b);
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void OnStart(int c);

};

//************** EndRun Menu ***************

//	Pops up when a normal level ends (time runs out).
class EndRunMenuClass : public FEMenu
{
public:
	enum
	{
		Retry,
		BeachGoals,
		Options,
		Replay, 
		Quit,
		ReturnToFE,
	};

private:
	FEMenuEntry *	replayEntry;
	FEMenuEntry *	goalsEntry;
	FEMenuEntry *	optionsEntry;

public:
	PauseMenuSystem* sys;
	FEMenuEntry *save;

public:
	EndRunMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnActivate();
	virtual void Select(int entry_index);
};

//************** SaveCareerPrompt Menu ***************
	
//	Pops up when a normal level ends (time runs out).
class SaveCareerPromptClass : public FEMenu
{
public:
	enum {
		Yes,
		No
	};
	enum 
	{
		START_SAVING_GLOBAL,
		SAVING_GLOBAL,
		START_SAVING_CAREER,
		SAVING_CAREER,
		START_SAVING_PHOTO,
		SAVING_PHOTO,
		SAVING_DONE,
		STATE_NONE
	};
	PauseMenuSystem* sys;
public:
	SaveCareerPromptClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	~SaveCareerPromptClass();
	void Select(int entry_index);
	virtual void Draw();
	virtual void OnActivate();
	void SetNextMenu(int m) { next_menu = m; }
	static void SaveProgressFunc(void *userdata, int progress);
	virtual void Update(time_value_t time_inc);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);
	int  GetState() {return myState;}
private:
	int myFrameTimer;
	int myState;
	int next_menu;
	FEMenuEntry *yes, *no;
	BoxText *message;

	void SetState(const int idx);
};



//************** HeatMid Menu ***************

//	Pops up when user hits START in the middle of a competition run.
class HeatMidMenuClass : public FEMenu
{
public:
	enum
	{
		Continue,
		Restart,
		Goals,
		Tip,
		Options,
		TrickBook,
		EndRun,
		EndComp,
	};

public:
	PauseMenuSystem* sys;
	FEMenuEntry *options;
	FEMenuEntry *trickbook;

public:
	HeatMidMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnActivate(void);
	virtual void Select(int entry_index);
	virtual void OnButtonRelease(int c, int b);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnStart(int c);
};


//************** HeatEnd Menu ***************

//	Pops up at the end of a run if it's not the end of the competition
class HeatEndMenuClass : public FEMenu
{
public:
	enum
	{
		NextRun,
		Goals,
		//Tip,
		Options,
		Replay,
		EndComp
	};

private:
	PauseMenuSystem *	sys;
	FEMenuEntry *		optionsEntry;

public:
	HeatEndMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	void Select(int entry_index);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnActivate(void);
};


//************** CompEnd Menu ***************

//	Pops up at the end of the competition
class CompEndMenuClass : public FEMenu
{
public:
	enum
	{
		Restart,
		Goals,
		Options,
		Replay,
		Quit,
		ReturnToFE,
	};

public:
	PauseMenuSystem *	sys;
	//FEMenuEntry *		save;
	FEMenuEntry *		optionsEntry;

public:
	CompEndMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnActivate(void);
	virtual void Select(int entry_index);
};

//************** Options Menu ****************

class OptionsMenuClass : public FEMenu
{
public:
	enum
	{
		Camera, 
		Sound, 
		Playlist, 
		Rumble, 
		DisplayOn,
	};

private:
	PauseMenuSystem *	sys;
	FEMenuEntry *		cameraEntry;
	FEMenuEntry *		rumbleEntry;
	FEMenuEntry *		displayEntry;
	bool				rumble;
	float				rumbleTimer;
private:
	void ToggleRumble();

public:
	OptionsMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnActivate();
	virtual void Select(int entry_index);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);
	virtual void Update(time_value_t time_inc);
	void RumbleOn(bool on, int controller);

};

//************** Sound Menu ***************

class SoundMenuClass : public FEMenu
{
private:
	PauseMenuSystem* sys;
	int soundType;  // 0 = stereo, 1 = prologic, 2 = mono
	int sounds[5];    // all between 0-MAX_VOL
	bool muted;      // if muted, then disable sound levels
	FEMenuEntry* sound_mute;
	FEMenuEntry* sound_type;
	FEMenuEntry* sound_levels[5];
	
	// cannot simply change this number; the text depends on it being 10
	static const int MAX_VOL = 10;

private:
	void Change(int index, bool up);

public:
	SoundMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void OnActivate(void);
	virtual void Select(int entry_index);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void OnTriangle(int c);
};

// **************** Camera Menu ******************

class CameraMenuClass : public FEMenu
{
public:
	enum { Follow, Beach, FPS };
	PauseMenuSystem* sys;

private:	
	FEMenuEntry *menu_item[3];
	bool multiplayer;
	
public:
	CameraMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual ~CameraMenuClass();
	void Draw();
	void Init();
	virtual void OnActivate(void);
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void OnTriangle(int c);
	virtual void Select(int entry_index);
	virtual void HighlightDefault(void);
};

// **************** Trick Type Menu ******************

class TrickTypeMenuClass : public FEMenu
{
public:
	PauseMenuSystem* sys;
private:
	// ignore TRICKBOOKTYPE_NOTYPE
	static const int NUM_TYPES = TRICKBOOKTYPE_NUMTYPES-1;
	static const int MAX_ENTRIES = 50;	// per subtype
	FEMenu* sub_menus[NUM_TYPES];
	FEMenuEntry* entry[NUM_TYPES];
	int trick_list_index[NUM_TYPES][MAX_ENTRIES];
public:
	TrickTypeMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual ~TrickTypeMenuClass();
	virtual void Init();
	virtual void Select(int entry_index);
	virtual void HighlightDefault(void);
	virtual void OnActivate();
	virtual void Update(time_value_t time_inc);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
};

// **************** Trick Menu ******************

class TrickMenuClass : public FEMenu
{
public:
	TrickMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);

	virtual void Select(int entry_num);
	virtual void OnActivate(void);
	virtual void OnCross(int c);
};

//************** Playlist Menu ***************

class PlaylistMenuClass : public FEMenu
{
private:
	PauseMenuSystem* sys;
	stringx* song_names;
	FEMenuEntry* songName[SONGS_PER_SCREEN];
	TextString* upArrow, *downArrow;
	TextString* helpText2;
	TextString* lineNumbers[SONGS_PER_SCREEN];
	TextString* onOff[SONGS_PER_SCREEN];
	TextString* playing, *currentSong, *currentArtist;
	int offset, pos, active, numSongs;
	color32 col, colh;
	bool tweaked;
public:
	PlaylistMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	~PlaylistMenuClass();
	
	void AddSongsToList();
	virtual void Draw();
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void OnSquare(int c);
	virtual void OnL1(int c);
	virtual void OnR1(int c);
	virtual void Select(int entry_index);
	virtual void Update(float time_inc);
	virtual void OnActivate();
	virtual void OnTriangle(int c);
};


//************** Replay Menu ***************

class ReplayMenuClass : public FEMenu
{
public:
	enum ReplayType { ReplayPlay, ReplaySlow, ReplayFF };
	PauseMenuSystem* sys;

private:
	virtual void ReplayStart();
	virtual void ReplayEnd();
	virtual void ReplayChange(ReplayType option);
	SSEventId replayEvent;

public:
	ReplayMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual void Select(int entry_index) { ReplayStart(); }
	virtual void OnActivate();
	virtual void Update(time_value_t time_inc);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnL2(int c);
	virtual void OnCross(int c);
	virtual void OnCircle(int c);
	virtual void OnStart(int c);
	virtual void OnTriangle(int c);
};


//************** Goals Menu ****************

class GoalsMenuClass : public FEMenu
{
protected:
	TextString *		title;
	BoxText	*			names[5];		// goal title
	TextString *		status[5];		// done or not done
	PauseMenuSystem *	sys;

	PanelQuad* tip_box[9];
	BoxText* tip;
	bool show_tip;

	bool wasMenuBGOn;

public:
	// Creators.
	GoalsMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	~GoalsMenuClass();

	// Modifiers.
	virtual void Load(PanelFile* panel);
	virtual void Draw(void);
	virtual void OnActivate();
	virtual void OnUnactivate(FEMenu * m);
	virtual void OnCross(int c);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);

private:
	void SetAsTip();	// change display to tip-style
	void SetNoTip();	// change display to no-tip-style
};

//************** Tips Menu ****************

class TipMenuClass : public FEMenu
{
public:
	BoxText* tip;
	stringx* tip_list;
	int tip_list_size;
	PauseMenuSystem* sys;
	static const int max_line_size = 256; 

public:
	TipMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual ~TipMenuClass();

	virtual void Init();
	virtual void Load();
	virtual void Draw();
	virtual void Select(int entry_index);
	virtual void OnActivate();
	virtual void OnTriangle(int c);
	virtual stringx NextTip();	// returns a random tip
private:
	virtual void SetTip();
};

//************** Quit Confirm Menu *******************

class QuitConfirmMenuClass : public FEMenu
{	
private:
	// enum { RESTART, RETRY, ENDRUN, ENDLEVEL, RESTARTCOMP };
	enum { YES, NO };

	PauseMenuSystem *	sys;
	BoxText *					question;
	FEMenuEntry *			yesEntry;
	FEMenuEntry *			noEntry;
	
public:
	QuitConfirmMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha);
	virtual ~QuitConfirmMenuClass() { delete question; }

	virtual void Draw();
	virtual void Select(int entry_index);
	virtual void OnActivate(void);
};

//************** Lost Controller Menu ****************

class LostControllerMenuClass : public FEMenu
{
public:
	PauseMenuSystem* sys;

public:
	LostControllerMenuClass(FEMenuSystem* m, int x, int y, color32 c);

	virtual void Init();
	virtual void OnActivate();
	virtual void Update();
};

// ************** Pause Menu SYSTEM *****************

// All in-game menus use this menu system
class PauseMenuSystem : public FEMenuSystem
{
public:
	enum {	PauseMenu,
			TimeAttackPauseMenu,
			TutorialPauseMenu,
			EndRunMenu,
			HeatMidMenu,
			HeatEndMenu,
			CompEndMenu,
			OptionsMenu,
			GoalsMenu,
			LostControllerMenu,
			SoundMenu,
			CameraMenu,
			TrickTypeMenu,
			PlaylistMenu,
			ReplayMenu,
			MapMenu,
			HighScoreMenu,
			PhotoMenu,
			TutorialMenu,
			SaveLoadMenu,
			SaveConfirmMenu,
			TipMenu,
			QuitConfirmMenu,
			NumMenus
	};
	bool draw;
	bool replay_mode;
	bool end_level;
	int pause_controller;
	int pause_player;
	BeachFrontEnd* map;
	TextString* player;
	SSEventId popupEvent;
	SSEventId navigationEvent;

public:
	PauseMenuSystem(FEManager* man, Font* f);
	virtual ~PauseMenuSystem();
	virtual void InitAll();
	virtual void Load();
	virtual void startDraw(int menu_num = -1, const bool pauseGame = true);
	virtual void endDraw(const bool unpause = true);
	virtual void Update(time_value_t time_inc);
	virtual void UpdateInScene();
	virtual void Draw();
	virtual void OnButtonPress(int b, int c);
	virtual void MakeActive(int m, int sm=1);
	virtual void Select(int menu_index, int entry_index);

	bool SetDisconnect(bool b);
	bool GetDisconnect(){ return controller_disconnected; };
	void Retry();		// from EndRun
	void EndLevel();	// from EndRun
	void EndRun();		// from PauseMenu
	void Restart();		// from TimeAttackMenu
	void RestartComp();	// from HeatMidMenu
	void EndComp();		// from HeadMidMenu
	void PrepareToEndLevel() { end_level = true; }
	void ActivateAndExit();
	bool IsResumable(FEMenu * m) const;

private:
	int old_device_flags;
	bool controller_disconnected;
};


extern bool IsDebugMenuDisplayed(void);

#endif	INCLUDED_FRONTENDMENUS_H
