// Beach Select Front End Screen

#ifndef XB_SAVELOADFRONTEND_H
#define XB_SAVELOADFRONTEND_H

#include "FEMenu.h"
#include "osGameSaver.h"

// show only 20 characters of the memory card name
#define MAX_CARDNAME 15

class GraphicalMenuSystem;

enum SaveErrors
{
	SE_NO_MEMORY_CARDS,		// no memory cards inserted
	SE_UNFORMAT_1,			// unformatted memory card 1
	SE_UNFORMAT_2,
	SE_SAVE_ERROR,			// error during saving
	SE_LOAD_ERROR,			// error during loading
};

class SaveLoadFrontEnd;

class NamesMenu : public FETextMultiMenu
{
public:
	static const int num = 20;
	static const int num_2 = 10;	// entries per memory card
private:
	FEMenuEntry* firstm[2];
	FEMenuEntry* lastm[2];
	int center_x1;					// center of the 1st list
	int center_y1;
	int center_x2;					// center of the 2nd list
	int center_y2;
	bool empty[num];				// is this slot empty?
	bool save;
	int card_active;				// 0 = card 0; 1 = card 1; -1 = neither available
	bool all_empty[2];
	bool mem_card_avail[2];
	bool mc_unformat[2];
	TextString* mem_empty[2];
	TextString* CardName[2];
	TextString* unavail[2];
	TextString* unformat[2];
	SaveLoadFrontEnd* sl_parent;
	int check_cards_counter;
  int current_port;
	FEMenuEntry* controller_select[NUM_MEMORY_PORTS + 1]; // one for each controller plus the HD
  bool selecting_controller;  // true when a controller menu entry is highlighted

public:
	NamesMenu(FEMenuSystem* s);
	~NamesMenu();
	void Init();
	void Update(time_value_t time_inc);
	void Draw();
	void Select();
	void OnDown(int c);
	void OnUp(int c);
	void OnLeft(int c);
	void OnRight(int c);
	virtual void OnTriangle(int c) { FETextMultiMenu::OnTriangle(c); }
	virtual void OnCross(int c) { FETextMultiMenu::OnCross(c); }
	void OnActivate(int type);
	void Refresh();
	int ActiveCard() { assert(card_active >= 0); return card_active; }
	int ActiveFile() { return highlighted->entry_num - ActiveCard()*num_2; }
  bool SelectingController() { return selecting_controller; }
	bool Available(int card);
	bool AvailAndSavedGames(int card);
  void SetPort(int new_port) {current_port = new_port;}
  int  GetPort() {return current_port;}
private:
  void SetSavePort(int port);
  void UpdateControllerSelect();
	void SwitchCards(bool left);
	void GetFileList(int card);
	void CheckCards();
};

class KeyboardMenu : public FETextMultiMenu
{
private:
	static const int num = 39;	// 26+10+3
	FEMenuEntry* ent[num];		// for positioning
	bool ent_can_active;		// are either enter or cancel active?
	bool enter_active;			// is enter active?
	TextString* title;
	TextString* filename;
	SaveLoadFrontEnd* sl_parent;

	static const int name_size = 24;
	char name[name_size+1];
	int current_place;
public:
	KeyboardMenu(FEMenuSystem* s);
	~KeyboardMenu();
	void Init();
	void Draw();
	void Select();
	void OnDown(int c);
	void OnUp(int c);
	void OnLeft(int c);
	void OnRight(int c);
	virtual void OnTriangle(int c) { if(c == input_mgr::inst()->GetDefaultController()) FETextMultiMenu::OnTriangle(c); }
	virtual void OnCross(int c) { if(c == input_mgr::inst()->GetDefaultController()) FETextMultiMenu::OnCross(c); }
	void OnActivate();
private:
};

class SaveLoadFrontEnd : public FEMultiMenu
{
public:
	enum disp_states
	{
		DSTATE_LSD,			// choosing load/save/delete (default, nothing on)
		DSTATE_LOAD,		// selecting a game to load
		DSTATE_LOAD_PICK,	// dialog box confirm in load
		DSTATE_LOADING,		// loading with progress bar
		DSTATE_LOAD_DONE,	// loading done
		DSTATE_SAVE,		// selecting a game to save
		DSTATE_SAVE_NAME,	// entering name of save (may be skipped)
		DSTATE_SAVE_PICK,	// dialog box confirm in save
		DSTATE_SAVING,		// saving, with progress bar
		DSTATE_SAVE_DONE,	// saving done
		DSTATE_DELETE,		// selecting a game to delete
		DSTATE_DELETE_PICK,	// dialog box confirm in delete
		DSTATE_DELETING,	// deleting; no progress bar
		DSTATE_DEL_DONE,	// deleting done
		DSTATE_ERROR,		// error dialog
	};
	enum type
	{
		SL_SAVE,
		SL_LOAD,
		SL_DELETE,
	};
	static const int num_each = NamesMenu::num_2;
	int numGameData[2], numGameConfig[2];
	saveInfo SavedData[2][num_each*2];
	saveInfo savedGames[2][num_each];
	saveInfo savedConfigs[2][num_each];
	char desc[20];

private:
	int fileType;
	int numGames;
	int dstate;
	float progressval;
	char *thedata;
	GraphicalMenuSystem* sys;

	FEMultiMenu* LSDMenu;	// load/save/delete
	static const int lsd_num = 3;
	int cur_type;
	FEMenuEntry* cur_entry;
	NamesMenu* NameMenu;	// Name lists on memcards
	KeyboardMenu* KeyMenu;	// keyboard menu, for PickName

	PanelQuad* memcard1[9], *memcard2[9], *savebg[9];
	PanelQuad* dialogPQ[9], *whitePQ[9];
	PanelQuad* yesPQ[2], *noPQ[2], *enterPQ[2], *cancelPQ[2], *okPQ;
	PanelQuad* delPQ, *loadPQ, *savePQ, *deletingPQ, *savingPQ, *loadingPQ;
	PanelQuad* delDonePQ, *loadDonePQ, *saveDonePQ;
	PanelQuad* memName1PQ, *memName2PQ;
	PanelQuad* lsdPQ[lsd_num][2];
	PanelQuad* barPQ[3];

//	TextString* deleting;
//	TextString* formatting;
//	TextString* save_done;
//	TextString* load_done;
//	TextString* del_done;
//	TextString* form_done;
//	TextString* ok;
	BoxText* error;
	int progress_drawn;	// for non-progress bar things (delete & format)
	int current_error;
	int error_ok_yesno;
	bool yes_active;
	int next_state;


//  void drawFileListing(bool Saving);
//  void keySelect();  

public:
	SaveLoadFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq);
	virtual ~SaveLoadFrontEnd();
	virtual void Init();
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void Select();
	virtual void OnActivate();
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnDown(int c);
	virtual void OnUp(int c);
	void getFileListing(int port, int slot);
	virtual void OnTriangle(int c) { if(c == input_mgr::inst()->GetDefaultController()) Back(); }
	virtual void OnCross(int c) { if(c == input_mgr::inst()->GetDefaultController()) Select(); }
	void Back();
	static void SetSaveProgress(void *userData, int val);
	static void SetLoadProgress(void *userData, int val);
	// the id is to confirm you're ending the same error you started
	// yesno = 0 for none, 1 for "ok", 2 for "yes/no"
	void StartError(stringx text, int id, int yesno=0);
	void EndError(int id);
	void TurnEntCan(bool enter, bool cancel);	// to swap between enter & cancel for keyboard
	void TurnYesNo(bool yes, bool no);			// to swap between yes & no
	// the activate bool is just to prevent OnActivate infinite loops
	void SetDState(int s, bool activate = true, bool end_error=false);
	void SaveFile(char* filename) { strcpy(desc, filename); SaveFile(NameMenu->ActiveCard(), false); }
	bool Busy() { return (dstate == DSTATE_SAVING || dstate == DSTATE_LOADING || dstate == DSTATE_DELETING ); }
private:
	void SelectLSD();
	void SelectError();
	void BackError();
	void SetPQIndices();
	void MemcardBGOn(bool on);
	void SaveBGOn(bool on);
	void delBoxOn(bool on);
	void loadBoxOn(bool on);
	void saveBoxOn(bool on);
	void savingOn(bool on) { TurnProgress(on); TurnDialog(on); savingPQ->TurnOn(on); }
	void loadingOn(bool on) { TurnProgress(on); TurnDialog(on); loadingPQ->TurnOn(on); }
	void SetProgress(float f) { barPQ[0]->Mask(f); }
	void TurnDialog(bool on) { for(int i=0; i<9; i++) dialogPQ[i]->TurnOn(on); }
	void TurnTextBox(bool on) { for(int i=0; i<9; i++) whitePQ[i]->TurnOn(on); }
	void TurnProgress(bool on) { for(int i=0; i<3; i++) barPQ[i]->TurnOn(on); }
	void TurnOffAll();
	void LoadFile(int card, int filenum);
	void SaveFile(int card, bool overwrite, int entry = 0);
	void DelFile(int card, int filenum);
};


#endif
