// Beach Select Front End Screen

#ifndef MCDDETECTFRONTENDH
#define MCDDETECTFRONTENDH

#include "FEMenu.h"
#include "osGameSaver.h"

#if defined(TARGET_XBOX)
#include "GameData.h"
#endif /* TARGET_XBOX JIV DEBUG */

enum MC_STATES
{
	MC_NONE=0,
	MC_INSERT_CARD =1,
	MC_FORMAT_CARD,
	MC_LOADING,
	MC_DONE, 
	MC_ERROR_LOADING, 
	MC_NO_DISK_SPACE, 
	MC_FORMATTING,
	MC_EXIT,
	MC_LOAD_GLOBAL
};

// How long to leave the auto-loading message up (dc 07/07/02)
#ifdef TARGET_XBOX
#define MESSAGE_DELAY 0.0f
#else
#define MESSAGE_DELAY 3.0f
#endif

class MCDetectFrontEnd : public FEMultiMenu
{
public:
	friend class DemoModeManager;
	
private:
	enum {
		MCContinue,
		MCRetry,
		MCLastEntry
	};
	FEMenuEntry* entries[MCLastEntry];
	int wait;
	
	int current_sel;
	int mc_state;
	int last_state;
	int percent;
	ksConfigData k;
	PanelQuad* bkg;
	BoxText* error;
	TextString* yes, *no;
	int leftx, uppery, width, height;
	int free_blocks;
public:
	void goState(int which);
	bool drawMenu();
	int validCards();
	int loadMostRecentGame();
	bool findMostRecentGame(int &port, int &slot,saveInfo &newest);
	int loadGlobalData();
	bool findGlobalData(int &port, int &slot);
#ifdef TARGET_XBOX
	void rebootToDashboard(u_int blocksToFreeUp);
#endif
	void tryToLoadMostRecent();
	static void configLoadCallback(void *userData, int percent);
	static void globalLoadCallback(void *userData, int percent);
	MCDetectFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq);
	virtual ~MCDetectFrontEnd();
	virtual void Init();
	virtual void Draw();
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void Select(int entry_index);
	virtual void Update(time_value_t time_inc);
	virtual void OnActivate();
	virtual void OnTriangle(int c);
	virtual void SetSystem(FEMenuSystem *s); 
	
};


#endif
