// THIS LITTLE FILE TAKES CARE OF AUTOLOADING AT START
// IT CURRENTLY IS NAMED AS IF IT ONLY LOADS CONFIGS,
// BUT IT ACTUALLY LOADS CAREERS.  TO BE FIXED
// KES 11/28/01

// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "MCDetectFrontEnd.h"
#include "FEPAnel.h"
#include <time.h>
#include "FEMenu.h"
#if defined(TARGET_PS2)
#include <libcdvd.h>
#endif /* TARGET_XBOX JIV DEBUG */
#include "GameData.h"
#include "FrontEndManager.h"
#include "osGameSaver.h"
static int onlyGoToMCScreenOnce=0;
static int formatCounter = 100;
int loadCount =0;
int notEnoughSpacePort=-9999, notEnoughSpaceSlot=-9999;
MCDetectFrontEnd::MCDetectFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq)
{
	cons(s, man, p, pq);
	flags |= FEMENU_DONT_SHOW_DISABLED | FEMENU_HAS_COLOR_HIGH_ALT;

	color = manager->col_unselected;
	color_high = manager->col_highlight;
	color_high_alt = manager->col_highlight2;
	scale = scale_high = 1.2f;

	entries[MCContinue] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_YES], this, false, &man->font_body);
	Add(entries[MCContinue] );
	entries[MCRetry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_NO], this, false, &man->font_body);
	Add(	entries[MCRetry]);
	
	mc_state = MC_NONE;
	free_blocks = -1;
}

void MCDetectFrontEnd::OnTriangle(int c)
{
}

void MCDetectFrontEnd::OnRight(int c)
{
	if (highlighted == entries[MCContinue] && !entries[MCContinue]->GetDisable())
	{
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	FEMultiMenu::OnRight(c);
}

void MCDetectFrontEnd::OnLeft(int c)
{
	if (highlighted == entries[MCRetry] && !entries[MCRetry]->GetDisable())
	{
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	FEMultiMenu::OnLeft(c);
}

MCDetectFrontEnd::~MCDetectFrontEnd()
{
	delete error;
}
static bool foundGame = false;

void MCDetectFrontEnd::SetSystem(FEMenuSystem *s)
{
	system = s;
}
void MCDetectFrontEnd::Init()
{
	
	leftx = 75;
	uppery = 100;
	width = 640 - leftx*2 - 90;	// 90 for extra spacing -beth
	height = 300;
	entries[MCContinue]->SetPos(leftx + 140, 300);
	entries[MCContinue]->right = entries[MCRetry];
	entries[MCRetry]->left = entries[MCContinue];
	entries[MCRetry]->SetPos(640 - leftx - 140, 300);
	
	error = NEW BoxText(&frontendmanager.font_body, "", 320,uppery+80);
	error->changeScale(.75f*1.5f);
//	error->color = manager->white;
	error->color = manager->col_info_g;
	error->no_color = false;
		
	error->makeBox(width, height);
	percent = 0;
	current_sel = 0;
	//LoadPanel();
	//bkg = GetPointer("title_screen_bkg");
	
	wait = 0;
	
	//  OnActivate();
	FEGraphicalMenu::Init();
}
void MCDetectFrontEnd::goState(int which)
{
	char errorText[500];
	char errorText2[500];
	entries[MCContinue]->SetText(ksGlobalTextArray[GT_FE_MENU_YES]);
	entries[MCRetry]->SetText(ksGlobalTextArray[GT_FE_MENU_NO]);
	last_state = mc_state;
	switch (which)
	{
	case MC_INSERT_CARD:
		sprintf(errorText, ksGlobalTextArray[GT_MC_NO_CARD].c_str(),  
			GenericGameSaver::inst()->getCardString(GenericGameSaver::inst()->getFirstCard(), 0).c_str(), 
			GenericGameSaver::inst()->getSavedGameSize());
		error->changeText(errorText);
		error->makeBox(width, height);
		
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);

		mc_state = MC_INSERT_CARD;
		break;
		
	case MC_FORMAT_CARD:
		assert(0);
		// savePort is base 0, so add one for display
		/*sprintf(errorText, ksGlobalTextArray[GT_MC_FORMAT_CARD_PS2].c_str(), savePort + 1);
		error->changeText(errorText);
		error->makeBox(width, height);
		mc_state = MC_FORMAT_CARD;
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_FE_MENU_YES]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_FE_MENU_NO]);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		*/
		break;
		
	case MC_LOADING:
		if (last_state == MC_LOAD_GLOBAL)
			error->changeText(GenericGameSaver::inst()->getLoadingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA])+ stringx("... ") );
		else if (last_state != MC_LOADING)
			error->changeText(GenericGameSaver::inst()->getLoadingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA])+ stringx("... ") );

		error->makeBox(width, height);
		mc_state = MC_LOADING;
		
		entries[MCContinue]->Disable(true);
		entries[MCRetry]->Disable(true);
		break;
		
	case MC_LOAD_GLOBAL:
		// I don't know what the right text is for this situation.  Currently, we're
		// getting a blank screen.  (dc 06/13/02)
		error->changeText(GenericGameSaver::inst()->getLoadingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA])+ stringx("... ") );
		error->makeBox(width, height);

		mc_state = MC_LOAD_GLOBAL;
		entries[MCContinue]->Disable(true);
		entries[MCRetry]->Disable(true);
		break;
	case MC_NO_DISK_SPACE:
		assert(notEnoughSpacePort != -9999 && notEnoughSpaceSlot != -9999);	// else uninitialized (dc 07/06/02)
#ifdef TARGET_PS2
		sprintf(errorText, ksGlobalTextArray[GT_MC_NOT_ENOUGH_SPACE].c_str(),  
			GenericGameSaver::inst()->getCardString(notEnoughSpacePort, notEnoughSpaceSlot).c_str(), 
			GenericGameSaver::inst()->getSavedGameSize(), 
			ksGlobalTextArray[GT_MC_KB].c_str()
		);
#else
		sprintf(errorText, 	ksGlobalTextArray[GT_MC_NOT_ENOUGH_SPACE_XB].c_str());
#endif
		assert(strlen(errorText) < countof(errorText) - 1);
#ifdef TARGET_XBOX
		errorText2[0] = 0; 
#else
		sprintf(errorText2, " %s", ksGlobalTextArray[GT_MC_RETRY_LC].c_str());
#endif
		assert(strlen(errorText2) < countof(errorText) - 1);
		strcat(errorText, errorText2);
		assert(strlen(errorText) < countof(errorText) - 1);
		error->changeText(errorText);
		error->makeBox(width, height);
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
#ifdef TARGET_XBOX
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_REBOOT]);
#else
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
#endif
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		mc_state = MC_NO_DISK_SPACE;
		break;
		
	case MC_FORMATTING:
		assert(0);
		/*sprintf(errorText, ksGlobalTextArray[GT_MC_FORMATTING].c_str(), savePort + 1);
		error->changeText(errorText);
		error->makeBox(width, height);
		mc_state = MC_FORMATTING;
		
#ifndef TARGET_XBOX
		entries[MCContinue]->Disable(true);
		entries[MCRetry]->Disable(true);
#endif
		*/
		break;
		
	case MC_DONE:
		error->changeText(GenericGameSaver::inst()->getLoadingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA]) + stringx("... ") + ksGlobalTextArray[GT_GOALS_DONE]);

		error->makeBox(width, height);
		mc_state = MC_DONE;
		
		entries[MCContinue]->Disable(true);
		entries[MCRetry]->Disable(true);
		break;
		
	case MC_ERROR_LOADING:
		if (last_state == MC_LOAD_GLOBAL)
			sprintf(errorText, ksGlobalTextArray[GT_MC_ERROR_LOADING].c_str(), ksGlobalTextArray[GT_MC_GAME_DATA].c_str(),  GenericGameSaver::inst()->getCardString(savePort, saveSlot).c_str());	
		else
			sprintf(errorText, ksGlobalTextArray[GT_MC_ERROR_LOADING].c_str(), ksGlobalTextArray[GT_MC_GAME_DATA].c_str(),  GenericGameSaver::inst()->getCardString(savePort, saveSlot).c_str());	
		error->changeText(errorText);
		error->makeBox(width, height);
		mc_state = MC_ERROR_LOADING;
		
		entries[MCContinue]->Disable(true);
		entries[MCRetry]->Disable(true);
		currentGame.valid = GSErrorOther;
		break;
	case MC_EXIT:
		system->MakeActive(GraphicalMenuSystem::MainMenu);
		break;
	}
	
}

bool MCDetectFrontEnd::drawMenu()
{
#ifdef TARGET_XBOX	// Only display the autoload menu if something goes wrong.  (dc 07/07/02)
	switch (mc_state)
	{
	case MC_ERROR_LOADING:
	case MC_NO_DISK_SPACE:
	case MC_INSERT_CARD:
	case MC_FORMAT_CARD:
		return true;
	default:
		return false;
	}
#else
	return true;
#endif
}

void MCDetectFrontEnd::Draw()
{
	if (drawMenu()) 
	{
		//bkg->Draw(0);
		error->Draw();
		FEGraphicalMenu::Draw();
	}
}

void MCDetectFrontEnd::Update(time_value_t time_inc)
{
	static float delayTimer = 0;

	switch (mc_state)
	{
	case MC_DONE:	
		delayTimer += time_inc;
		StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
		if (delayTimer > MESSAGE_DELAY)
			goState(MC_EXIT);
		break;	
	case MC_FORMATTING:
		formatCounter++;
		if (formatCounter > 5)
		{
			if (GenericGameSaver::inst()->format(savePort,saveSlot) != GSOk)
				goState(MC_INSERT_CARD);  // PROBLEM?
			else
				goState(MC_DONE);  // SUCCESS
		}
		break;
	case MC_ERROR_LOADING:
		delayTimer+= time_inc;
		if (delayTimer > MESSAGE_DELAY)
			goState(MC_EXIT);
		break;
	case MC_LOAD_GLOBAL:
		
		if (findGlobalData(savePort, saveSlot))
		{
			loadGlobalData();		
		}
		else if (savePort == INVALID_CARD_VALUE)
			goState(MC_EXIT);
		else
			goState(MC_DONE);
		break;
	}
	
#if defined(TARGET_PS2)
	RotateThreadReadyQueue(1);
#endif
	FEMultiMenu::Update(time_inc);
}

void MCDetectFrontEnd::configLoadCallback(void *userData, int percent)
{
	if (percent >= 100)
	{
		((MCDetectFrontEnd *)userData)->goState(MC_LOAD_GLOBAL);
		currentGame.valid = GSOk;
	}
	else if (percent < 0)
	{
		g_career->init();
		globalCareerData.init();
		((MCDetectFrontEnd *)userData)->goState(MC_ERROR_LOADING);
	}
	else // Scale 
		((MCDetectFrontEnd *)userData)->percent = percent/2;
	
}
void MCDetectFrontEnd::globalLoadCallback(void *userData, int percent)
{
	if (percent >= 100)
	{
		((MCDetectFrontEnd *)userData)->goState(MC_DONE);
		globalCareerData.updateFromCareer(g_career);
	}
	else if (percent < 0)
	{
		globalCareerData.init();
		globalCareerData.updateFromCareer(g_career);
		((MCDetectFrontEnd *)userData)->goState(MC_DONE);
	}
	else
		((MCDetectFrontEnd *)userData)->percent = 50 + percent/2;
}
bool MCDetectFrontEnd::findMostRecentGame(int &savePort, int &saveSlot, saveInfo &mostRecent)
{
	// Try to find an actual game to load
	int port, slot, retVal = GSErrorOther;
	int type, free, formatted, oldestDate = 0;
	saveInfo SavedData[20];
	mostRecent.valid = GSErrorOther;
	for(port = GenericGameSaver::inst()->getFirstCard(); port < NUM_MEMORY_PORTS; port++)
		for(slot = 0; slot < NUM_MEMORY_SLOTS; slot++)
		{
			if (port == -1 && slot == 1)
				continue;
			retVal = GenericGameSaver::inst()->getInfo(port,slot, &type, &free, &formatted);

			if (retVal == GSOk)
			{
				int numGames = GenericGameSaver::inst()->getFileListing(port,slot, SavedData, NULL, NULL);
				for (int i=0; i < numGames; i++)
				{
					if (SavedData[i].valid == GSOk)
					{
						if (SavedData[i].timestamp > oldestDate)
						{
							oldestDate = SavedData[i].timestamp;
							savePort = port;
							saveSlot = slot;
							
							memcpy(&mostRecent, &(SavedData[i]), sizeof(saveInfo));
						}
					}
				}
			}

		}

	if (mostRecent.valid == GSOk)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool MCDetectFrontEnd::findGlobalData(int &foundPort, int &foundSlot)
{
	int port, slot;
	if (foundPort == INVALID_CARD_VALUE && foundSlot == INVALID_CARD_VALUE)
	{
		// Try to find an actual game to load
		for(port = GenericGameSaver::inst()->getFirstCard(); port < NUM_MEMORY_PORTS; port++)
			for(slot = 0; slot < NUM_MEMORY_SLOTS; slot++)
			{
				if (port == -1 && slot == 1)
					continue;
				if (GenericGameSaver::inst()->hasSystemFile(port, slot))
				{
					foundPort = port;
					foundSlot = slot;
					return true;
				}
			}
			return false;
	}
	else
	{
		if (GenericGameSaver::inst()->hasSystemFile(foundPort, foundSlot))
		{

			return true;
		}
		return false;
	}
}



int MCDetectFrontEnd::loadGlobalData()
{
	if (findGlobalData(savePort, saveSlot))
	{
		goState(MC_LOADING);
		GenericGameSaver::inst()->readSystemFile( savePort, saveSlot, &globalCareerData, MCDetectFrontEnd::globalLoadCallback, this);			
		
		return GSOk;
	}
	else
		goState(MC_EXIT);

	return GSErrorOther;
}

int MCDetectFrontEnd::validCards()
{
	int port, slot, retVal, numCards = 0;
	int type, free, formatted;
	int numTooLittleSpace=0, numUnformatted=0;
	for(port = GenericGameSaver::inst()->getFirstCard(); port < NUM_MEMORY_PORTS; port++)
		for(slot = 0; slot < NUM_MEMORY_SLOTS; slot++)
		{
			retVal = GenericGameSaver::inst()->getInfo(port,slot, &type, &free, &formatted);
			if (retVal == GSOk && (unsigned int)free > (unsigned int)GenericGameSaver::inst()->getSavedGameSize())
			{
				numCards++;
			}
			else if (retVal == GSErrorUnformatted)
			{
				numUnformatted++;
			}
			else if (retVal == GSOk && (unsigned int)free <= (unsigned int)GenericGameSaver::inst()->getSavedGameSize())
			{
				numTooLittleSpace++;
				if (notEnoughSpacePort == -9999)
				{
					notEnoughSpacePort = port;
					notEnoughSpaceSlot = slot;
				}
			}
			// Otherwise we don't have a card
		}

	if (numCards)
	{
		return numCards;
	}
	else if (numUnformatted)
	{
		return 0;
	}
	else if (numTooLittleSpace)
	{
		return GSErrorNotEnoughSpace;
	}
	else
		return GSErrorNoMedia;

}

int MCDetectFrontEnd::loadMostRecentGame()
{
	saveInfo  mostRecent;
	mostRecent.valid = -1;
	
	findMostRecentGame(savePort, saveSlot, mostRecent);

	// Now that we're done, check how many valid cards
	if (mostRecent.valid != GSOk)
	{
		g_career->init();
		globalCareerData.init();
		if (findGlobalData(savePort, saveSlot))
			goState(MC_LOAD_GLOBAL);
		else if (savePort != INVALID_CARD_VALUE)
			goState(MC_DONE);
		else
			goState(MC_EXIT);
		return GSOk;
	}
	else 
	{
		foundGame = true; 
		memcpy(&currentGame, &mostRecent, sizeof(saveInfo));
		GenericGameSaver::inst()->setFileInfo(mostRecent);
		goState(MC_LOADING);
		GenericGameSaver::inst()->readFile(savePort,saveSlot,g_career,sizeof(Career), MCDetectFrontEnd::configLoadCallback, this);
		return GSOk;
	}
	
}


void MCDetectFrontEnd::Select(int entry_index)
{
	if (mc_state == MC_FORMAT_CARD)  // IN THE FORMAT SCREEN
	{
#ifdef TARGET_XBOX
		assertmsg(false, "Xbox should never end up in formatting screen.");
#else
		if (entry_index == MCContinue)
		{
			goState( MC_FORMATTING );
			formatCounter = 0;
		}
		else
		{
			tryToLoadMostRecent();
		}
#endif
	}
	else if (mc_state == MC_INSERT_CARD)
	{
#ifdef TARGET_XBOX
		assertmsg(false, "Xbox should never end up in insert card screen.");
#else
		if (entry_index == MCContinue)
		{
			//system->MakeActive(((GraphicalMenuSystem*) system)->first_screen);
			goState(MC_EXIT);
		}
		else
		{
			tryToLoadMostRecent();		
		}
#endif
	}
	else if (mc_state == MC_DONE)
	{
#ifdef TARGET_XBOX
		GenericGameSaver::inst()->ReleaseAll();
#endif // TARGET_XBOX
		StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
		system->MakeActive(GraphicalMenuSystem::MainMenu);
	}
	else if (mc_state == MC_ERROR_LOADING)
	{
		system->MakeActive(GraphicalMenuSystem::MainMenu);
	}
	else if (mc_state == MC_NO_DISK_SPACE)
	{
		if (entry_index == MCRetry)
		{
#ifdef TARGET_XBOX
			rebootToDashboard(GenericGameSaver::inst()->getSavedGameSize() - free_blocks);
#else
			tryToLoadMostRecent();
#endif
		}
		else  // CONTINUE
		{
			loadMostRecentGame();
		}
	}
}

void MCDetectFrontEnd::tryToLoadMostRecent()
{
	int retVal;
	currentGame.valid = GSErrorOther;
	foundGame =false;
	// First check the hard drive
	
#ifdef TARGET_XBOX
	int type, formatted;
	retVal = GenericGameSaver::getInfo(-1,0, &type, &free_blocks, &formatted);
	if((unsigned int)free_blocks < (unsigned int)GenericGameSaver::inst()->getSavedGameSize())
	{
		notEnoughSpacePort = HARD_DRIVE_PORT; notEnoughSpaceSlot = 0;	// else uninitialized (dc 07/02/02)
		goState( MC_NO_DISK_SPACE );
		return;
	}
#elif defined (TARGET_PS2)
	/*retVal = GenericGameSaver::getInfo(0,0, &type, &free, &formatted);
	if (!formatted)
	{
	formatPort = 0;
	formatSlot = 0;
	
	  mc_state=MC_FORMAT_CARD;
	  return;
	  }
	  
		if (retVal != GSOk)
		{
		retVal = GenericGameSaver::getInfo(1,1, &type, &free, &formatted);
		if (retVal != GSOk)
		{
		mc_state=MC_INSERT_CARD;
		return;
		}
		}
		if((unsigned int)free < (unsigned int)GenericGameSaver::inst()->getSavedGameSize())
		{
		mc_state = MC_NO_DISK_SPACE;
		return;
}*/
#endif
	// increment num_cards so we don't get the "insert a card" error
	
	// try to load the most recent save game from the disk
	
		int cards = validCards();
		switch (cards)
		{
#ifndef TARGET_GC
			case GSErrorNoMedia:
				goState(MC_INSERT_CARD);
				return;
				break;
			case GSErrorNotEnoughSpace:
				goState(MC_NO_DISK_SPACE);
				return;
				break;
#endif //TARGET_GC
			default:
				break;
		}

    retVal=loadMostRecentGame();
}

#ifdef TARGET_XBOX
void MCDetectFrontEnd::rebootToDashboard(u_int blocksToFreeUp)
{
	// Reboot to Dashboard.  Copied from XDK documentation.  (dc 07/02/02)
	LD_LAUNCH_DASHBOARD LaunchDash;
	LaunchDash.dwReason = XLD_LAUNCH_DASHBOARD_MEMORY;
	LaunchDash.dwContext = 0;
	LaunchDash.dwParameter1 = DWORD('U');	// The Xbox Hard Drive (dc 07/02/02)
	LaunchDash.dwParameter2 = blocksToFreeUp;
	
	XLaunchNewImage(NULL, (PLAUNCH_DATA)(&LaunchDash));
}
#endif

void MCDetectFrontEnd::OnActivate()
{
	// Some hacks here
	//bkg->AddedToMenu();
#ifdef TARGET_XBOX
	setHigh(entries[MCContinue],true);
#else
	setHigh(entries[MCRetry],true);
#endif
	if ((os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD)))
	{
		onlyGoToMCScreenOnce = 1;
		system->MakeActive(GraphicalMenuSystem::MainMenu);
		return;
	}
	if (onlyGoToMCScreenOnce)
	{
		system->MakeActive(GraphicalMenuSystem::MainMenu);
		return;
	}
	
	onlyGoToMCScreenOnce=1;
	tryToLoadMostRecent();
}

