// Beach Select Front End Screen

#ifndef GCMCDDETECTFRONTEND_H
#define GCMCDDETECTFRONTEND_H

#include "FEMenu.h"
#include "osGameSaver.h"
#include "FrontEndManager.h"

class GCMCDetectFrontEnd : public FEMultiMenu
{
public:
	friend class DemoModeManager;
	
private:
	enum MC_STATES
	{
		MC_NONE=0,
		MC_EXIT,
		MC_INSERT_CARD,
		MC_CORRUPT,
		MC_REGION,
		MC_DAMAGED,
		MC_WRONG,
		MC_COMPAT,
		MC_NO_DISK_SPACE, 
		MC_FORMAT_CARD,
		MC_FORMATTING,
		MC_LOADING,
		MC_ERROR_LOADING
	};
	
	enum {
		MCContinue,
		MCOther,
		MCRetry,
		MCLastEntry
	};
	FEMenuEntry* entries[MCLastEntry];
	
	int mc_state;
	int last_state;
	int format_count;
	bool foundGame;

	PanelQuad* bkg;
	BoxText* error;
	TextString* yes, *no;
	int leftx, uppery, width, height;
public:
	void goState(int which);
	bool drawMenu();
	int validCards();
	int checkForErrors();
	GCMCDetectFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq);
	virtual ~GCMCDetectFrontEnd();
	virtual void Init();
	virtual void Draw();
	virtual void Select(int entry_index);
	virtual void Update(time_value_t time_inc);
	virtual void OnActivate();
	virtual void SetSystem(FEMenuSystem *s); 
	int loadMostRecentGame();
	int loadGlobalData();
	bool findGlobalData( int &foundPort, int &foundSlot );
	bool findMostRecentGame( int &savePort, int &saveSlot, saveInfo &mostRecent );
	static void globalLoadCallback( void *userData, int percent );
	static void configLoadCallback( void *userData, int percent );
};


#endif
