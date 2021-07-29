// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "DemoMode.h"
#include "FrontEndManager.h"
#include "random.h"
#include "MusicMan.h"

DemoModeManager dmm;


// In FEMenu.cpp
extern int getButtonPressed(int button);
extern int getAnalogState(int button);
extern int getButtonState(int button);

DemoModeManager::DemoModeManager()
{
	inDemo = false;
	totalTime = 0;
	played = false;
	timeoutDelay = 999.0f;
	demoDuration = 0.0f;
	demoStarted = false;
} 

void DemoModeManager::Init()
{
	timeoutDelay = (float)os_developer_options::inst()->get_int(os_developer_options::INT_DEMO_MODE_TIMEOUT);
	demoDuration = (float)os_developer_options::inst()->get_int(os_developer_options::INT_DEMO_MODE_DURATION);
	
	if(timeoutDelay == 0)
		timeoutDelay = 30.0f;
	wasInTitle = false;
	totalTime = 0;
	demoStarted = false;
}

// Enter the game
void DemoModeManager::GoGame()
{
	// Setup state
	os_file peek;
	bool worked = 0;
	int searchCount = 0;
	// Generate the name
	int replay = random(NUMBER_OF_REPLAYS);
	sprintf(fname, "DEMOS\\REPLAY%d.RPL", replay);
	if (	frontendmanager.gms->active == GraphicalMenuSystem::TitleMenu )
		wasInTitle = true;
	else
		wasInTitle = false;
	// Try to open this one.  
	// Start searching for a good copy.
	while ((!worked) && (searchCount < NUMBER_OF_REPLAYS))
	{
		if (os_file::file_exists(fname))
		{
      int i;

			int	surferIdx;
			int	boardIdx;
			int	beachIdx;
      char beachName[32];

			stringx tmp;

      // Disable the stash only flag if set (since demo files are not in stashes)
      bool stashOnly = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY);
      os_developer_options::inst()->set_flag(os_developer_options::FLAG_STASH_ONLY, false);

      KSReadFile(fname, &replayFile, 1);

      surferIdx = *((int *)(&replayFile.Buf[0]));
      boardIdx  = *((int *)(&replayFile.Buf[4]));
      strcpy(beachName, (char *)(&replayFile.Buf[8]));

      // Find the beach index from the name
      int numEntries = sizeof(CareerDataArray) / sizeof(CareerDataArray[0]);
      for(i=0; i<numEntries; i++)
      {
        if(0 == strcmp(beachName, CareerDataArray[i].name))
          break;
      }

      if(i == numEntries)
        assert(0 && "Beach file not found");

      beachIdx = i;

      int offset = 4 + 4 + 32 + sizeof(KSWaterState);
      replayFrames        = *((int *)(&replayFile.Buf[offset]));
      replayMainPOFrames  = *((int *)(&replayFile.Buf[offset+4]));
      replayAIPOFrames    = *((int *)(&replayFile.Buf[offset+8]));

      // Must release here and read a second time after the level is loaded (or there's a mem leak)
      KSReleaseFile(&replayFile);
      os_developer_options::inst()->set_flag(os_developer_options::FLAG_STASH_ONLY, stashOnly);

			frontendmanager.current_surfer = ksreplay.sfr;
			tmp = SurferDataArray[surferIdx].name;
			app::inst()->get_game()->setHeroname(0, tmp);
			app::inst()->get_game()->SetSurferIdx(0, surferIdx);
			app::inst()->get_game()->SetBoardIdx(0, boardIdx);
			((BeachFrontEnd *)frontendmanager.gms->menus[GraphicalMenuSystem::BeachMenu])->PickDemo(ksreplay.bch);
			app::inst()->get_game()->set_level(beachIdx);
			app::inst()->get_game()->set_game_mode(GAME_MODE_CAREER);
			//((BeachFrontEnd *)frontendmanager.gms->menus[GraphicalMenuSystem::BeachMenu])->Select(ksreplay.bch);
			//frontendmanager.map_loading_screen = true;
			frontendmanager.gms->PrepareToExit();
			worked = 1;
		}
		else
		{
			// Keep looking
			searchCount++;
			replay++;
			if (replay == NUMBER_OF_REPLAYS)
				replay = 0;
			sprintf(fname, "DEMOS\\REPLAY%d.RPL", replay);
		}
	}
	
	// Search failed.
	if (!worked)
	{
		inDemo = false;
		demoStarted = false;
		totalTime = 0;
		return;
	}
	
	// Go to it
	//g_game_ptr->unpause();
	//g_game_ptr->go_next_state();
	//frontendmanager.gms->Exit();
}

// Clear the timer
void DemoModeManager::clear()
{
	totalTime = 0;
}

// A key was hit
bool DemoModeManager::keyHit()
{
	if (inDemo && g_game_ptr->level_is_loaded())
	{   
		ksreplay.Stop();
		g_game_ptr->unpause();
		
		played = false;
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			MusicMan::inst()->stop();
			nslSoundId snd = MusicMan::inst()->playNext();
			if (snd != NSL_INVALID_ID)
				nslPauseSound(snd);
		}
		ExitToFrontEnd();
	}
	
	totalTime = 0;
	demoTime = 0;
	
	return true;
}


// Tick 
bool DemoModeManager::tick(float timeInc)
{
	static bool loaded = false;

  GraphicalMenuSystem *gms  = frontendmanager.gms;
	FEEntityManager     *em   = frontendmanager.em;
	TitleFrontEnd       *tfe  = frontendmanager.tfe;
	
	// Increment the time
	if (g_game_ptr->get_cur_state() == GSTATE_FRONT_END && gms &&
		((gms->active == GraphicalMenuSystem::TitleMenu
#ifdef TARGET_GC
			&&  tfe->mc->mc_state == GCMCDetectFrontEnd::MC_NONE
#else	
			&&  tfe->mc->mc_state == MC_NONE
#endif //TARGET_GC
			) ||
		((gms->active == GraphicalMenuSystem::MainMenu) && !(gms->menus && (gms->menus[0])->active) && !em->InFlyin()) &&
    ((gms->menus[GraphicalMenuSystem::MainMenu]->highlighted && gms->menus[GraphicalMenuSystem::MainMenu]->highlighted->entry_num != MainFrontEnd::MainCareerEntry) || 
     (((MainFrontEnd *)gms->menus[0])->career_menu->highlighted && ((MainFrontEnd *)gms->menus[0])->career_menu->highlighted->entry_num <= CareerMenu::LoadEntry))))
  {
		totalTime += timeInc;
	}
	
	demoTime += timeInc;
	
	// Check button states
	if (getButtonPressed(PSX_SELECT))         { keyHit(); return false; }
	else if (getButtonPressed(PSX_START))     { keyHit(); return false; }
#ifdef TARGET_XBOX // dpad and stick can't interrupt attract mode on xbox
  else if(!g_game_ptr->level_is_loaded())
  {
	  if (getAnalogState(PSX_UD))          { keyHit(); return false; }
	  else if (getAnalogState(PSX_LR))          { keyHit(); return false; }
  }
#else
	else if (getAnalogState(PSX_UD))          { keyHit(); return false; }
	else if (getAnalogState(PSX_LR))          { keyHit(); return false; }
#endif
	else if (getButtonPressed(PSX_X))         { keyHit(); return false; }
	else if (getButtonPressed(PSX_TRIANGLE))  { keyHit(); return false; }
	else if (getButtonPressed(PSX_SQUARE))    { keyHit(); return false; }
	else if (getButtonPressed(PSX_CIRCLE))    { keyHit(); return false; }
	else if (getButtonPressed(PSX_L1))        { keyHit(); return false; }
	else if (getButtonPressed(PSX_R1))        { keyHit(); return false; }
	else if (getButtonPressed(PSX_L2))        { keyHit(); return false; }
	else if (getButtonPressed(PSX_R2))        { keyHit(); return false; }
#ifdef TARGET_XBOX
	else if (getButtonPressed(PSX_L3))        { keyHit(); return false; }
	else if (getButtonPressed(PSX_R3))        { keyHit(); return false; }

	// check for memory unit insertion	
	DWORD removals, insertions;
	bool changed;
	if (this->inDemo) 
	{
		changed= XGetDeviceChanges(XDEVICE_TYPE_MEMORY_UNIT, &insertions, &removals);
		if(insertions != 0)	                      
		{
			keyHit(); 
			return false;
		}
	}
#endif

	if (!inDemo)
	{
		// Check to see if we should enter the demo
		if (totalTime > timeoutDelay)
		{
#ifndef DEMOMODE_OFF
			inDemo = true;
			loaded = false;
#endif
		}
	}
	else
	{  
		// Has the level loaded?
		if (g_game_ptr->level_is_loaded())
		{
			if(!loaded)
			{
				loaded = true;
				totalTime = 0.0f;
				demoTime = 0.0f;
			}
			
			if(loaded && (demoDuration != 0.0f) && (demoTime > demoDuration))
			{
				ksreplay.Stop();
				played = true;
			}
			
			// Start play
			if ((!ksreplay.IsPlaying()) && (!played))
			{
				g_game_ptr->pause();
				
				played = true;
				// Load up new demo
				ksreplay.LoadFile(fname);
				// Fire it off
				ksreplay.Play();
				app::inst()->get_game()->set_player_camera(0,find_camera(entity_id("REPLAY_CAM")));
				
				// Turn off IGO
				frontendmanager.IGO->SetDisplay(false);
				frontendmanager.IGO->SetReplayText(ksGlobalTextArray[GT_IGO_DEMO]);
				if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				{
					
					MusicMan::inst()->stop();
					MusicMan::inst()->playNext();
				}
			}
			// End of play.. go back to main menu
			else if ((!ksreplay.IsPlaying()) && (played))
			{
				// Stop it
				ksreplay.Stop();
				// Unpause the game
				g_game_ptr->unpause();
				// set the entry flag
				played = false;
				// Stop the music (if playing)
				if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				{
					
					MusicMan::inst()->stop();
				}
				// End the game
				ExitToFrontEnd();
			}
		}
		// Level hasn't loaded
		else
		{
			// Go to the game
			if(!demoStarted)
			{
				demoStarted = true;
				GoGame();
			}
		}
	}
	return inDemo;
}

bool DemoModeManager::inDemoMode()
{
	// Status query
	return inDemo;
}
bool DemoModeManager::ReturnFromDemoToTitle()
{
	return (wasInDemo && wasInTitle);
}
bool DemoModeManager::ReturnFromDemoToMain()
{
	return (wasInDemo && !wasInTitle);
}

void DemoModeManager::ExitToFrontEnd()
{
	inDemo = false;
	wasInDemo = true;
	totalTime = 0;
	// Go back to FE
	frontendmanager.pms->ActivateAndExit();
	//app::inst()->get_game()->end_level();
}

