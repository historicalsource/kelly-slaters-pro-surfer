// Beach Select Front End Screen

#ifndef SAVELOADFRONTEND_H
#define SAVELOADFRONTEND_H

#include "FEMenu.h"
#include "osGameSaver.h"


class GraphicalMenuSystem;

enum SaveErrors
{
	SE_NO_MEMORY_CARDS,		// no memory cards inserted
	SE_UNFORMAT,			// unformatted memory card
	SE_NOT_ENOUGH_SPACE, // Not enough room
	SE_SAVE_ERROR,			// error during saving
	SE_LOAD_ERROR,			// error during loading
	SE_FORM_ERROR,			// error during formatting
	SE_DELETE_ERROR,
	SE_UNIQUE_FILENAME,
	SE_NO_FILENAME,
	SE_GC_CORRUPT,
	SE_GC_REGION
};

#ifdef TARGET_XBOX
#define NUMCARDS (NUM_MEMORY_PORTS*NUM_MEMORY_SLOTS + 1)
#define LOWEST_CARD -1
#define HIGHEST_CARD NUMCARDS-1
#else
#define NUMCARDS (NUM_MEMORY_PORTS*NUM_MEMORY_SLOTS)
#define LOWEST_CARD 0
#define HIGHEST_CARD NUMCARDS
#endif


class NamesMenu;
class KeyboardMenu;
class DialogMenu;


struct MemCard
{
	int free;
	bool exists;
	bool changed;
	bool available;
	bool ask_format;
	bool saved_games;
	int status;
};

//******************************************************

class SaveLoadFrontEnd : public FEMultiMenu
{
public:
	enum disp_states
	{
		DSTATE_LSD,			// choosing load/save/delete (default, nothing on)
		DSTATE_LOAD,		// selecting a game to load
		DSTATE_LOAD_PICK,	// dialog box confirm in load
		DSTATE_LOAD_GLOBAL, // Loading the globaldata
		DSTATE_LOADING_GLOBAL,
		DSTATE_LOADING,		// loading with progress bar
		DSTATE_LOAD_DONE,	// loading done
		DSTATE_SAVE,		// selecting a game to save
		DSTATE_SAVE_GLOBAL,		// start save global
		DSTATE_SAVE_NAME,	// entering name of save (may be skipped)
		DSTATE_SAVE_PICK,	// dialog box confirm in save
		DSTATE_SAVING,		// saving, with progress bar
		DSTATE_SAVING_GLOBAL,		// start save global
		DSTATE_SAVE_PHOTO,
		DSTATE_SAVING_PHOTO,
		DSTATE_SAVE_DONE,	// saving done
		DSTATE_DELETE,		// selecting a game to delete
		DSTATE_DELETE_PICK,	// dialog box confirm in delete
		DSTATE_DELETING,	// deleting; no progress bar
		DSTATE_DEL_DONE,	// deleting done
		DSTATE_FORMATTING,	// formatting; no progress bar
		DSTATE_FORM_DONE,	// formatting done
	};
	enum type
	{
		SL_SAVE,
		SL_LOAD,
		SL_DELETE,
	};

	enum activation_menu
	{
		ACT_LSD = 1,
		ACT_SAVE = 2,
		ACT_LOAD = 3,
		ACT_DELETE = 4,
		ACT_SAVE_NAME = 5,
	};
	static const int num_each = 15;	// entries per memory card
	int numGameData[NUMCARDS], numGameConfig[NUMCARDS];
	saveInfo SavedData[NUMCARDS][num_each*2];
	saveInfo savedGames[NUMCARDS][num_each];
	saveInfo savedConfigs[NUMCARDS][num_each];
	char desc[100];
  bool setCfg;
	int cur_type;
	float progressval;

	int back_menu;
	int back_sub_menu;
	int forward_menu;
	int forward_sub_menu;
	bool in_frontend;
	int post_format_state;
private:
	int fileType;
	int numGames;
	int dstate;
	GraphicalMenuSystem* sys;
	static const int lsd_num = 3;

	bool error_thrown;
	int current_error;
	int next_state;
	bool overwrite;
	FEMenuEntry* entry[lsd_num];
	MultiLineString *sftd;  // Select file to delete (isn't it obvious??)
public:
	NamesMenu* NameMenu;	// Name lists on memcards
	KeyboardMenu* KeyMenu;	// keyboard menu, for PickName
	DialogMenu* dialogMenu;

public:
	SaveLoadFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq, bool in_fe=true);
	virtual ~SaveLoadFrontEnd();
	virtual void Init();
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void Select();
	virtual void OnActivate();
	virtual void OnActivate(int submenu);
	virtual void OnUnactivate(FEMenu* m);
	virtual void OnLeft(int		c);
	virtual void OnRight(int c);
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	void getFileListing(int active, int adjusted);
	bool ExistsAndHasSpace(int card);
	bool GetInFrontEnd() {return in_frontend; }  // Who wants to guess what this does?
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	static void SetSaveProgress(void *userData, int val);
	static void SetGlobalSaveProgess(void *userData, int val);
	static void SetLoadProgressGame(void *userData, int val);
	static void SetLoadProgressGlobal(void *userData, int val);
	void StartError(int id);
	void EndError(int id);
	int getActiveCard();
	// the activate bool is just to prevent OnActivate infinite loops
	void SetDState(int s, bool activate = true, bool end_error=false);
	int  GetDState() {return dstate; };
	void SetOverwrite(const char* filename=NULL);
	bool Busy() { return (dstate == DSTATE_SAVING || dstate == DSTATE_LOADING || dstate == DSTATE_DELETING || dstate == DSTATE_FORMATTING); }
	void DialogYesOKPressed();
	void DialogNoPressed();
	void CancelDialog();
	bool SavedGamesExist();
	bool DialogActive();
	
	// returns something like "memory card (for PS2) in slot 1"
	// Useful for not enough space errors
	static stringx MakeStringCardInSlot(int active_card);

	// returns something like "MEMORY CARD slot 1"
	// Useful for NamesMenu headers
	static stringx MakeStringSlot(int active_card);

	// returns something like "memory card"
	// Useful for most memory card references
	static stringx MakeStringMemCard(int active_card);
private:
	void SetPQIndices();
	void TurnOffAll();
	void LoadFile(int card, int filenum);
	void SaveFile(int card, bool overwrite, int slot = 0);
	void DelFile(int card, int filenum);
};

// ****************************************************

class NamesMenu : public FEMultiMenu
{
public:
	friend class SaveLoadFrontEnd;
	static const int num = SaveLoadFrontEnd::num_each;

private:
	int type;
	bool save;
	vector2d card_center;
	SaveLoadFrontEnd* sl_parent;
	int check_cards_counter;
	static const int arrow_timer = 3;
	int arrow_num;
	int arrow_counter;
	bool empty[num];		// is this file (not card) empty?
	MemCard cards[NUMCARDS];
	stringx blank_name;

	// is -1 if xbox & harddrive
	// is passed in to game saver functions
	int active_card;

	// == active_card, unless xbox then active_card+1
	// is index into menu entries (when added to num) and savedConfig arrays
	int adjusted_active_card;

	bool cardSide;
	PanelQuad* box;
	PanelQuad* arrows[2][2];
	PanelQuad* lines[10], *top_line, *side_line;
	FEMenuEntry* entry[num+NUMCARDS];
	BoxText* message;
	BoxText* freeSpace;
	bool draw_message;
public:
	NamesMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf);
	virtual ~NamesMenu();
	virtual void Init();
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void DrawHeader();
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnCross(int c);
	virtual void OnActivate();
	virtual void OnUnactivate(FEMenu* m);
	void AskFormat(bool start);
	void Format();
	void RefreshDisplay();
	int ActiveCard() { return active_card; }
	int ActiveFile();
	bool Available(int card, bool unformatted_ok);
	bool AvailAndSavedGames(int card);
	void SetPQIndices();
	void TurnPQ(bool on);
	void TurnPQLines(bool on);
	void ResetCardFormat();
	stringx MakeCardName(int active);
	void MakeSpaceFreeMessage(int free);
//	void FindAvailableSlot();
	static int FindAdjusted(int active);
	static int FindActive(int adjusted);
	void SetSecondaryCursor(FEMenuEntry* e);
private:
	void GetFileList(int active, int adjusted);
	void CheckCardNum(int c);
//	void CheckCardAvailability(bool set_active);
	void SetActiveCard();
	void UpdateMessage();
	void OnCardSwitch();
	void SetVis(FEMenuEntry* first);
public:
	static void ReplaceBadCharacters(stringx &tmp);
};

// ****************************************************

class DialogMenu : public FEMultiMenu
{
public:
	enum {
		DM_TYPE_MNG,
		DM_TYPE_FMT,    // Format/Cancel display
		DM_TYPE_YES,		// yes/no displayed
		DM_TYPE_OK,			// ok displayed
		DM_TYPE_PROGRESS,	// progress bar displayed
		DM_TYPE_EMPTY,		// nothing displayed (usually for deleting & formatting)
	};

private:
	enum { DM_YES, DM_NO, DM_OK, DM_FMT, DM_CNL, DM_MNG, DM_CNL2, DM_NUM };	// 320, 270
	BoxText* message;
	BoxText* prompt;
	int type;
	FEMenuEntry* entry[DM_NUM];
	PanelQuad* bar[4], *box;
	int draw_count;			// used for other menus to determine if a message has drawn enough times
	static int const DRAW_COUNT_MAX = 3;

public:
	DialogMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf);
	virtual ~DialogMenu();
	virtual void Draw();
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c) {}
	virtual void OnCross(int c);
	void SetPQIndices();

	// must be called before OnActivate
	void SetTypeAndMessage(int ty, stringx str);
	virtual void OnActivate();
	virtual void OnUnactivate(FEMenu* m) { TurnPQ(false); }
	bool DrawnEnough() { return draw_count >= DRAW_COUNT_MAX; }
	void TurnPQ(bool on);
	void SetProgress(float fl) { bar[0]->Mask(fl); bar[0]->Update(0);}
};

// ****************************************************

class KeyboardMenu : public FEMultiMenu
{
private:
	static const int num = 40;	// 26+10+space+clear+enter+cancel
	static const int space_idx = 36;
	static const int back_idx = 37;
	static const int enter_idx = 38;
	static const int cancel_idx = 39;
	static const int row_size = 8;
	FEMenuEntry* ent[num];		// for positioning
	TextString* filename;
	TextString* enter_text, *name_text;
	SaveLoadFrontEnd* sl_parent;
	PanelQuad* keys[num][3];	// enter & cancel don't have PQ's, but clear has 3
	PanelQuad* name_box[3];

	static const int name_size = 10;
	stringx name;
	bool default_cleared;  // true if the user has moved the highlight to any of the alphanumeric buttons
public:
	KeyboardMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf);
	virtual ~KeyboardMenu();
	virtual void Init();
	virtual void Draw();
	virtual void Update(time_value_t time_inc);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnCross(int c);
	virtual void OnActivate();
	virtual void OnUnactivate(FEMenu* m);
	void SetPQIndices();
	void TurnPQ(bool on);
private:
	void Switch(FEMenuEntry* before, FEMenuEntry* after);
	void KeyOn(int index, bool on);
};


#endif
