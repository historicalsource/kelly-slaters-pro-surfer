// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX)
#include "rumbleManager.h"
#include "trick_system.h"
#include "FrontEndManager.h"
#include "conglom.h"
#include "kellyslater_controller.h"
#include "GameData.h"
#include "WaveSound.h"
#include "MusicMan.h"
#include "PlaylistMenu.h"
#include "trickdata.h"
#include "osdevopts.h"
#include "wds.h"
#include "ksfx.h"
#include "SoundData.h"
#include "SoundScript.h"
#include "unlock_manager.h"
extern SurferTrick GTrickList[TRICK_NUM];
#endif /* TARGET_XBOX JIV DEBUG */

#include "ks_camera.h"
#include "FrontEndMenus.h"
#include "osGameSaver.h"

static bool done = false;
void SetSaveProgress(void *userData, int val)
{
	if (val == 100)
		done = true;
	else if (val < 0)
		done = true;
}
bool hasPrevious(FEMenu *m)
{
	if(!(m->flags & FEMENU_WRAP) && m->highlighted == m->entries) return false;
	if (m->highlighted->previous) return true;
	return false;
}

bool hasNext(FEMenu *m)
{
	
	if (m->highlighted->next) return true;
	return false;
}

void SaveCurrentGame()
{
	done = false;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
	long int t;
#ifndef TARGET_GC
	STUB("SAVECURRENTGAME::Select()");
#endif
	
	// Create a time stamp
	t = time(NULL);
#else
	
#endif /* TARGET_XBOX JIV DEBUG */
	
	// career is saved so no unsaved data left
	frontendmanager.unsaved_career = false;
	GenericGameSaver::inst()->setFileInfo(currentGame);
	g_career->cfg = *(ksConfigData*)(StoredConfigData::inst()->getGameConfig());
	
	GenericGameSaver::inst()->saveFile(savePort, saveSlot,(void *)g_career,
		sizeof(*g_career), true,
		(SetSaveProgress), NULL);
	while(!done)
	{
#ifdef TARGET_PS2
		RotateThreadReadyQueue(1); 
#endif
	};
}



/********************** PauseMenu *************************/

PauseMenuClass::PauseMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CONTINUE], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_RETRY], this));
	goals = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_GOALS], this);
	Add(goals);
	tip = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TIP], this);
	Add(tip);
	options = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(options);
	trickbook = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TRICKBOOK], this);
	Add(trickbook);
	if (g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_INFINITE)
		Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_SELECT], this));
	else
		Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_END_RUN], this));
	returnFE = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_RETURN_FE], this);
	Add(returnFE);
	scale_high = 1.0f;

	// to avoid showing Beach Goals option on freesurf
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
}

void PauseMenuClass::OnActivate()
{
	FEMenuEntry *	oldHigh;
	
	goals->Disable(g_game_ptr->get_game_mode() != GAME_MODE_CAREER);
	tip->Disable(g_game_ptr->get_game_mode() != GAME_MODE_CAREER);
	trickbook->Disable( sys->pause_player == -1 );
	options->Disable( sys->pause_player == -1 );
	returnFE->Disable(g_game_ptr->get_game_mode() != GAME_MODE_FREESURF_INFINITE);

	// Reposition entries.
	oldHigh = highlighted;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();

	//FEMenu::OnActivate();
}

void PauseMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	if (hasPrevious(this))
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	else
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}

void PauseMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	//if (hasNext(this))
		sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	/*else
		sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ERROR);*/
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);
}

void PauseMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	switch(entry_index)
	{
	case Continue:		break;
	case Retry:		sys->Retry(); break;
	case Goals:		sys->MakeActive(PauseMenuSystem::GoalsMenu); break;
	case Tip:			sys->MakeActive(PauseMenuSystem::TipMenu); break;
	case Options:		sys->MakeActive(PauseMenuSystem::OptionsMenu); break;
	case TrickBook:	sys->MakeActive(PauseMenuSystem::TrickTypeMenu); break;
	case EndRun:
		if (g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_INFINITE)
			sys->EndLevel();
		else
			sys->EndRun();
		break;
	case ReturnToFE:	sys->MakeActive(PauseMenuSystem::QuitConfirmMenu); break;
	default: assert(0);
	}
}

void PauseMenuClass::OnButtonRelease(int c, int b)
{
	//Only retry if the pause menu wasn't created from a controller disconnect
	//and the controller is currently connected
	if( sys->GetDisconnect() && g_game_ptr->get_disconnect_status() )
		return;

	// Continue does nothing until button released
	if (b == FEMENUCMD_CROSS && highlighted->entry_num == Continue)
		sys->endDraw();
}

void PauseMenuClass::OnStart(int c)
{
	FEMenu::OnStart(c);

/*
#if defined(TARGET_XBOX)
	if (highlighted->entry_num == Continue)
		sys->endDraw();
#endif
*/
}

/********************** TimeAttackPauseMenu *************************/

TimeAttackPauseMenuClass::TimeAttackPauseMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CONTINUE], this));
	options = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(options);
	trickbook = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TRICKBOOK], this);
	Add(trickbook);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_END_RUN], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_RESTART_MATCH], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_SELECT], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_RETURN_FE], this));

	scale_high = 1.0f;
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
}

void TimeAttackPauseMenuClass::OnActivate(void)
{
	FEMenuEntry *	oldHigh;
	
	trickbook->Disable( sys->pause_player == -1 );
	options->Disable( sys->pause_player == -1 );

	// Reposition entries.
	oldHigh = highlighted;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();

	//FEMenu::OnActivate();
}

void TimeAttackPauseMenuClass::Select(int entry_index)
{
	switch(entry_index)
	{
	case Continue:	break;
	case Options:	sys->MakeActive(PauseMenuSystem::OptionsMenu); break;
	case TrickBook:	sys->MakeActive(PauseMenuSystem::TrickTypeMenu); break;
	case EndRun :	sys->EndRun(); break;
	case Restart:	sys->Restart(); break;
	case Quit:		sys->EndLevel(); break;
	case ReturnToFE:sys->MakeActive(PauseMenuSystem::QuitConfirmMenu); break;
	default: assert(0);
	}
}

void TimeAttackPauseMenuClass::OnButtonRelease(int c, int b)
{
	// Continue does nothing until button released
	if (b == FEMENUCMD_CROSS && highlighted->entry_num == Continue)
		sys->endDraw();
}

void TimeAttackPauseMenuClass::OnStart(int c)
{
	FEMenu::OnStart(c);

/*
#if defined(TARGET_XBOX)
	if (highlighted->entry_num == Continue)
		sys->endDraw();
#endif
*/
}

/********************** TutorialAttackPauseMenu *************************/

TutorialPauseMenuClass::TutorialPauseMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_NEXT_TIP], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CONTINUE], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_RETRY], this));
	options = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(options);
	trickbook = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TRICKBOOK], this);
	Add(trickbook);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_END_RUN], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_RETURN_FE], this));

	scale_high = 1.0f;
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
}

void TutorialPauseMenuClass::OnActivate(void)
{
	FEMenuEntry *	oldHigh;
	
	trickbook->Disable( sys->pause_player == -1 );
	options->Disable( sys->pause_player == -1 );

	// Reposition entries.
	oldHigh = highlighted;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();

	//FEMenu::OnActivate();
}

void TutorialPauseMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();
	switch(entry_index)
	{
	case NextTip:	
		sys->MakeActive(PauseMenuSystem::TutorialMenu);  
		sys->UpdateButtonDown();
		break;
	case Continue:	break;
	case Retry:		sys->Retry(); break;
	case Options:	sys->MakeActive(PauseMenuSystem::OptionsMenu); break;
	case TrickBook:	sys->MakeActive(PauseMenuSystem::TrickTypeMenu); break;
	case EndRun:	sys->EndRun(); break;
	case ReturnToFE:	sys->MakeActive(PauseMenuSystem::QuitConfirmMenu); break;
	default: assert(0);
	}
}

void TutorialPauseMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);
}

void TutorialPauseMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}

void TutorialPauseMenuClass::OnButtonRelease(int c, int b)
{	
	// Continue does nothing until button released
	if (b == FEMENUCMD_CROSS && highlighted->entry_num == Continue)
		sys->endDraw();
}

void TutorialPauseMenuClass::OnStart(int c)
{
	FEMenu::OnStart(c);

/*
#if defined(TARGET_XBOX)
	if (highlighted->entry_num == Continue)
		sys->endDraw();
#endif
*/
}

/********************** SaveCareerPromptClass *************************/

SaveCareerPromptClass::SaveCareerPromptClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;
	
	yes = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_YES], this);
	Add(yes);
	no = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_NO], this);
	Add(no);

	message = NEW BoxText(sys->font, ksGlobalTextArray[GT_IGO_SAVE_CAREER_PROMPT], 320, 170, 0.0f, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_TOP, color32(255,255,255,255), 7);
	message->makeBox(300, 200);

	scale_high = 1.0f;
	SetHelpText(HF_UP | HF_DOWN | HF_SELECT);
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
}

void SaveCareerPromptClass::OnActivate()
{
	frontendmanager.unsaved_career = false; 
	message->changeText(ksGlobalTextArray[GT_IGO_SAVE_CAREER_PROMPT]);
	message->makeBox(300,200);
	yes->Disable(false);
	no->Disable(false);
	sys->manager->IGO->ShowMenuBackground(true);
	setHigh(yes);
	yes->SetText(ksGlobalTextArray[GT_FE_MENU_YES]);
	no->SetText(ksGlobalTextArray[GT_FE_MENU_NO]);
	SetState(STATE_NONE);
}

void SaveCareerPromptClass::Update(time_value_t time_inc)
{
	int		idx;
	
	
	FEMenu::Update(time_inc);
	if (myState == START_SAVING_PHOTO && myFrameTimer == 0)
	{
		
		idx = ((PhotoFrontEnd *) system->menus[PauseMenuSystem::PhotoMenu])->GetSelectedPhotoIdx();
		if (idx != -1)
		{
			g_career->GetPhotoForLevel (g_game_ptr->get_level_id())->CopyFromTexture(g_beach_ptr->get_challenges()->photo->GetPhotoTexture(idx));
			SetState(SAVING_DONE);
		}
		else
			SetState(SAVING_DONE);
	}
	else if (myState == START_SAVING_CAREER)
	{
		SetState(SAVING_CAREER);
		GenericGameSaver::inst()->setFileInfo(currentGame);
		GenericGameSaver::inst()->saveFile(savePort, saveSlot, (void *) g_career, sizeof(Career), true, SaveCareerPromptClass::SaveProgressFunc, (void *) this);
	}
	else if (myState == START_SAVING_GLOBAL)
	{
		SetState(SAVING_GLOBAL);
		GenericGameSaver::inst()->saveSystemFile(savePort, saveSlot, &globalCareerData, SaveCareerPromptClass::SaveProgressFunc, this);
	}

}
void SaveCareerPromptClass::SetState(int state) 
{ 	
	myState =state;

	if (myState == START_SAVING_CAREER)
	{
		stringx tmp = GenericGameSaver::inst()->getSavingString(savePort, saveSlot, ksGlobalTextArray[GT_FE_MENU_CAREER]);
		tmp.to_upper();
		message->changeText(tmp);
		message->makeBox(400, 300);
		yes->Disable(true);
		no->Disable(true);
	}
	else if (myState == START_SAVING_GLOBAL)
	{
		stringx tmp = GenericGameSaver::inst()->getSavingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA]);
		tmp.to_upper();
		message->changeText(tmp);
		message->makeBox(400, 300);
		yes->Disable(true);
		no->Disable(true);
	}
	else if (myState == START_SAVING_PHOTO)
	{
		myFrameTimer = 3;
		stringx tmp = GenericGameSaver::inst()->getSavingString(savePort, saveSlot, ksGlobalTextArray[GT_FE_MENU_PHOTO]);
		tmp.to_upper();
		message->changeText(tmp);
		message->makeBox(400, 300);
		yes->Disable(true);
		no->Disable(true);
	}
	else if (myState == SAVING_DONE)
	{
		message->changeText(ksGlobalTextArray[GT_FE_MENU_SAVE_DONE]);
		message->makeBox(400, 300);
		no->Disable(false);
		no->SetText(ksGlobalTextArray[GT_FE_MENU_OK]);
		setHigh(no);
	}
}
void SaveCareerPromptClass::SaveProgressFunc(void *userData, int progress)
{

	if (progress >= 100 || progress < 0)
	{
		if (((SaveCareerPromptClass *)userData)->GetState() == SAVING_CAREER)
		{
			((SaveCareerPromptClass *)userData)->SetState(START_SAVING_GLOBAL);
		}
		else if (((SaveCareerPromptClass *)userData)->GetState() == SAVING_GLOBAL)
		{
			int idx;
			idx = ((PhotoFrontEnd *) frontendmanager.pms->menus[PauseMenuSystem::PhotoMenu])->GetSelectedPhotoIdx();
			if (idx != -1)
				((SaveCareerPromptClass *)userData)->SetState(START_SAVING_PHOTO);
			else
				((SaveCareerPromptClass *)userData)->SetState(SAVING_DONE);
		}
	}
}
void SaveCareerPromptClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	switch (entry_index)
	{
		case Yes:
			{
				sys->manager->IGO->ShowMenuBackground(false);
				sys->MakeActive(PauseMenuSystem::SaveLoadMenu, SaveLoadFrontEnd::ACT_SAVE);
				((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->forward_menu = -1;
				((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->forward_sub_menu = -1;
				((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->back_menu = PauseMenuSystem::SaveConfirmMenu;
				((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->back_sub_menu = -1;
			}

		break;
		case No:
			sys->endDraw(false);
		break;
	}
}

void SaveCareerPromptClass::Draw()
{
	if (myFrameTimer > 0)
		myFrameTimer--;
	message->Draw();
	FEMenu::Draw();
}

void SaveCareerPromptClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	if (myState == STATE_NONE)
		if (hasPrevious(this))
			sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		else
			sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}
void SaveCareerPromptClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	if (myState == STATE_NONE)
		//if (hasNext(this))
			sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		/*else
			sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ERROR);*/
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);
}
void SaveCareerPromptClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	if (myState == STATE_NONE)
		sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	SoundScriptManager::inst()->pause();

	FEMenu::OnTriangle(c);
}
SaveCareerPromptClass::~SaveCareerPromptClass()
{
	delete message;
}

/********************** EndRunMenu *************************/

EndRunMenuClass::EndRunMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_RETRY], this));
	goalsEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_GOALS], this);
	Add(goalsEntry);
	optionsEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(optionsEntry);
	replayEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_REPLAY], this);
	Add(replayEntry);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_SELECT], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_RETURN_FE], this));

	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_REPLAY))
		replayEntry->Disable(true);

	scale_high = 1.0f;
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
	SetHelpText(HF_UP | HF_DOWN | HF_SELECT);
}

void EndRunMenuClass::OnActivate()
{
	FEMenuEntry *	oldHigh;
	
	// Disable entries that we don't need.
	if(g_game_ptr->get_num_players() != 1/* || g_game_ptr->get_num_ai_players() == 1*/)
		replayEntry->Disable(true);
	else
		replayEntry->Disable(false);
	if(g_game_ptr->get_game_mode() != GAME_MODE_CAREER)
		goalsEntry->Disable(true);
	else
		goalsEntry->Disable(false);
	//optionsEntry->Disable(sys->pause_player == -1);
	
	if (ksreplay.GetStatus() == KSReplay::REPLAY_RECORD || ksreplay.Done())
		replayEntry->SetText(ksGlobalTextArray[GT_MENU_REPLAY]);
	else
		replayEntry->SetText(ksGlobalTextArray[GT_MENU_REPLAY_RESUME]);
	
	// Reposition entries.
	oldHigh = highlighted;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();
	
	system->manager->IGO->ShowMenuBackground(true);
	
	// check if the photo challenge was passed this run (to go back to scrapbook if necessary)
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		int goal_num = -1;
		CareerDataStruct l = CareerDataArray[g_game_ptr->get_level_id()];
		
		for (int i=0; i<MAX_GOALS_PER_LEVEL; i++)
		{
			if (l.goal[i] == GOAL_PHOTO_1 || l.goal[i] == GOAL_PHOTO_1 || l.goal[i] == GOAL_PHOTO_2)
				goal_num = i;
		}
			
		if (goal_num != -1)
			frontendmanager.new_photo |= g_career->WasNewGoalPassed(goal_num);
	}
}

void EndRunMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);
}
void EndRunMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}

void EndRunMenuClass::Select(int entry_index)
{	
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	switch(entry_index)
	{
	case Retry:			sys->Retry(); break;
	case BeachGoals:	sys->MakeActive(PauseMenuSystem::GoalsMenu); break;
	case Options:		sys->MakeActive(PauseMenuSystem::OptionsMenu); break;
	case Replay:		sys->MakeActive(PauseMenuSystem::ReplayMenu); break;
	case Quit:			sys->EndLevel(); break;
	case ReturnToFE:	sys->MakeActive(PauseMenuSystem::QuitConfirmMenu); break;
	default: assert(0);
	}
}

/********************** HeatMidMenu *************************/

HeatMidMenuClass::HeatMidMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CONTINUE], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_RESTART_COMP], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_GOALS], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TIP], this));
	options = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(options);
	trickbook = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TRICKBOOK], this);
	Add(trickbook);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_END_RUN], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_END_COMP], this));

	scale_high = 1.0f;
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
}

void HeatMidMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	if (hasPrevious(this))
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	else
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}

void HeatMidMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	//if (hasNext(this))
		sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	/*else
		sys->navigationEvent=SoundScriptManager::inst()->playEvent(SS_FE_ERROR);*/
	SoundScriptManager::inst()->pause();
	FEMenu::OnDown(c);
}

void HeatMidMenuClass::OnActivate(void)
{
	FEMenuEntry *	oldHigh;
	
	trickbook->Disable( sys->pause_player == -1 );
	options->Disable( sys->pause_player == -1 );

	// Reposition entries.
	oldHigh = highlighted;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();

	//FEMenu::OnActivate();
}

void HeatMidMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();
	
	switch(entry_index)
	{
	case Continue:	break;
	case Restart:	sys->RestartComp(); break;
	case Goals:     sys->MakeActive(PauseMenuSystem::GoalsMenu); break;
	case Tip :		sys->MakeActive(PauseMenuSystem::TipMenu); break;
	case TrickBook:	sys->MakeActive(PauseMenuSystem::TrickTypeMenu); break;
	case Options:   sys->MakeActive(PauseMenuSystem::OptionsMenu);  break;
	case EndRun:	sys->EndRun(); break;
	case EndComp:   sys->EndComp(); break;
	default: assert(0);
	}
}

void HeatMidMenuClass::OnButtonRelease(int c, int b)
{	
	// Continue does nothing until button released
	if (b == FEMENUCMD_CROSS && highlighted->entry_num == Continue)
		sys->endDraw();
}

void HeatMidMenuClass::OnStart(int c)
{
	FEMenu::OnStart(c);

/*
#if defined(TARGET_XBOX)
	if (highlighted->entry_num == Continue)
		sys->endDraw();
#endif
*/
}

/********************** HeatEndMenu *************************/

HeatEndMenuClass::HeatEndMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_NEXT_HEAT], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_GOALS], this));
	//Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_TIP], this));
	optionsEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(optionsEntry);
	FEMenuEntry *replayEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_REPLAY], this);
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_REPLAY))
		replayEntry->Disable(true);
	Add(replayEntry);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_END_COMP], this));

	scale_high = 1.0f;
	flags |= FEMENU_WRAP;
	SetHelpText(HF_UP | HF_DOWN | HF_SELECT);
}

void HeatEndMenuClass::OnActivate(void)
{
	//FEMenu::OnActivate();

	system->manager->IGO->ShowMenuBackground(true);

	//optionsEntry->Disable(sys->pause_player == -1);
}
void HeatEndMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();

	if (hasPrevious(this))
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	else
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}
void HeatEndMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	//if (hasNext(this))
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	/*else
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);*/
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);

}
void HeatEndMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	switch(entry_index)
	{
	case NextRun:	sys->Retry(); break;
	case Goals:		sys->MakeActive(PauseMenuSystem::GoalsMenu); break;
	//case Tip :		sys->MakeActive(PauseMenuSystem::TipMenu); break;
	case Options:	sys->MakeActive(PauseMenuSystem::OptionsMenu);  break;
	case Replay:	sys->MakeActive(PauseMenuSystem::ReplayMenu);  break;
	case EndComp:	g_world_ptr->get_ks_controller(0)->get_my_scoreManager().Reset(); sys->EndComp(); break;
	default: assert(0);
	}
}

/********************** CompEndMenu *************************/

CompEndMenuClass::CompEndMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_RESTART_COMP], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_GOALS], this));
	optionsEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_OPTIONS], this);
	Add(optionsEntry);
	FEMenuEntry *replayEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_REPLAY], this);
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_REPLAY))
		replayEntry->Disable(true);
	Add(replayEntry);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BEACH_SELECT], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_RETURN_FE], this));
	
	scale_high = 1.0f;
	flags |= FEMENU_WRAP;
	SetHelpText(HF_UP | HF_DOWN | HF_SELECT);
}

void CompEndMenuClass::OnActivate(void)
{
	sys->manager->IGO->ShowMenuBackground(true);

	//optionsEntry->Disable(sys->pause_player == -1);
}
void CompEndMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMenu::OnUp(c);
}
void CompEndMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMenu::OnDown(c);
}
void CompEndMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();
	switch(entry_index)
	{
	case Restart:		sys->RestartComp(); break;
	case Goals:			sys->MakeActive(PauseMenuSystem::GoalsMenu); break;
	case Options:		sys->MakeActive(PauseMenuSystem::OptionsMenu); break;
	case Replay:		sys->MakeActive(PauseMenuSystem::ReplayMenu); break;
	case Quit:			sys->EndLevel(); break;
	case ReturnToFE:	sys->MakeActive(PauseMenuSystem::QuitConfirmMenu); break;
	default: assert(0);
	}
}

/********************** OptionsMenu *************************/
#ifdef TARGET_GC
#define NATIVE_RUMBLE_ON GT_MENU_RUMBLE_ON_GC
#define NATIVE_RUMBLE_OFF GT_MENU_RUMBLE_OFF_GC
#else
#define NATIVE_RUMBLE_ON GT_MENU_RUMBLE_ON
#define NATIVE_RUMBLE_OFF GT_MENU_RUMBLE_OFF
#endif

OptionsMenuClass::OptionsMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;
	cameraEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CAMERA], this);
	Add(cameraEntry);
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_SOUND], this));
	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_PLAYLIST], this));

	rumbleEntry = NEW FEMenuEntry(ksGlobalTextArray[NATIVE_RUMBLE_ON], this);
	Add(rumbleEntry);
	displayEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_DISPLAY_ON], this);
	Add(displayEntry);	

	setBack(sys->menus[PauseMenuSystem::PauseMenu]);

	scale_high = 1.0f;
	SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
	flags |= FEMENU_WRAP | FEMENU_DONT_SHOW_DISABLED;
}

void OptionsMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	switch(entry_index)
	{
	case Camera:	sys->MakeActive(PauseMenuSystem::CameraMenu); break;
	case Sound:		sys->MakeActive(PauseMenuSystem::SoundMenu); break;
	case Playlist:	sys->MakeActive(PauseMenuSystem::PlaylistMenu); break;
	case Rumble:	ToggleRumble(); break;
	case DisplayOn:
		frontendmanager.score_display = !frontendmanager.score_display;
		displayEntry->SetText(ksGlobalTextArray[frontendmanager.score_display ? GT_MENU_DISPLAY_ON : GT_MENU_DISPLAY_OFF]);
		break;
	default: assert(0);
	}
}
void OptionsMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}
void OptionsMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);
}
void OptionsMenuClass::Update(time_value_t time_inc)
{
	FEMenu::Update(time_inc);

	if (rumbleTimer > 0)
	{
		rumbleTimer -= time_inc;
	}
	if (rumbleTimer < 0)
	{
		RumbleOn(false, g_game_ptr->get_player_device( sys->pause_player ) );
	}
}
void OptionsMenuClass::RumbleOn(bool on, int controller)
{
	input_mgr* inputmgr = input_mgr::inst();
	input_device *joyjoy=inputmgr->get_joystick(JOYSTICK_TO_DEVICE_ID(controller+JOYSTICK1_DEVICE));
	if (on)
	{
		rumbleTimer = 0.3f;
		if ( joyjoy )
    	joyjoy->vibrate( 0.0f,255.0f,1.0f,0.0f);
	}
	else
		if ( joyjoy )
		{
    	joyjoy->vibrate(0.0f,0.0f,0.0f,0.0f);
			rumbleTimer = -1.0f;
		}
}

void OptionsMenuClass::OnActivate()
{
	FEMenuEntry *	oldHigh;
	rumbleTimer = -1;
	if (sys->pause_player != -1)
	{
		cameraEntry->Disable(false);
		
		rumble = rumbleMan.isOn( g_game_ptr->get_player_device( sys->pause_player ) );
		rumbleEntry->SetText( ksGlobalTextArray[ rumble ? NATIVE_RUMBLE_ON : NATIVE_RUMBLE_OFF ] );
		rumbleEntry->Disable(false);
	}
	else
	{
		cameraEntry->Disable(true);
		rumbleEntry->Disable(true);
	}

	// Reposition entries.
	oldHigh = highlighted;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();
	
	// Only let Start = Continue if we can continue playing. (dc 07/12/02)
	if (sys->IsResumable(this))
		SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
	else
		SetHelpText(HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
}
void OptionsMenuClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();
	RumbleOn(false, g_game_ptr->get_player_device( sys->pause_player ) );
	FEMenu::OnTriangle(c);
}
void OptionsMenuClass::ToggleRumble()
{
	assert( sys->pause_player != -1 );
	rumble = !rumble;
	if (rumble)
		RumbleOn(true, g_game_ptr->get_player_device( sys->pause_player ) );
	rumbleEntry->SetText( ksGlobalTextArray[ rumble ? NATIVE_RUMBLE_ON : NATIVE_RUMBLE_OFF ] );
	rumbleMan.turnOn( rumble, g_game_ptr->get_player_device( sys->pause_player ) );
}

/********************** SoundMenu *************************/

SoundMenuClass::SoundMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	// Adding all sound options
	// always start with it unmuted
	sound_mute = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_SOUND_ON], this);
	muted = false;
	Add(sound_mute);

	stringx tmp;
	switch(nslGetSpeakerMode())
	{
	case NSL_SPEAKER_MONO:		soundType = 2; tmp = ksGlobalTextArray[GT_MENU_SOUND_MONO]; break;
	case NSL_SPEAKER_STEREO:	soundType = 0; tmp = ksGlobalTextArray[GT_MENU_SOUND_STEREO]; break;
	case NSL_SPEAKER_PROLOGIC:	soundType = 1; tmp = ksGlobalTextArray[GT_MENU_SOUND_PROLOGIC]; break;
	default: assert(0);
	}
	sound_type = NEW FEMenuEntry(tmp, this);
	this->flags |= FEMENU_DONT_SHOW_DISABLED;
	Add(sound_type);

#ifdef TARGET_XBOX
	sound_type->Disable(true);
#endif
	
	sounds[0] = (int)(nslGetMasterVolume() * MAX_VOL);
	sounds[1] = (int)(nslGetVolume(NSL_SOURCETYPE_AMBIENT) * MAX_VOL);
	sounds[2] = (int)(nslGetVolume(NSL_SOURCETYPE_SFX) * MAX_VOL);
	sounds[3] = (int)(nslGetVolume(NSL_SOURCETYPE_VOICE) * MAX_VOL);
	sounds[4] = (int)(nslGetVolume(NSL_SOURCETYPE_MUSIC) * MAX_VOL);

	for(int i=0; i<5; i++)
	{
		sound_levels[i] = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_SOUND_MASTER+i]+stringx(" ")+stringx(sounds[i]), this);
		Add(sound_levels[i]);
	}

	setBack(sys->menus[PauseMenuSystem::OptionsMenu]);

	scale_high = 1.0f;
	flags |= FEMENU_WRAP;
}

void SoundMenuClass::OnActivate(void)
{
	if (sys->IsResumable(this))
		SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SWITCH | HF_BACK);
	else
		SetHelpText(HF_UP | HF_DOWN | HF_SWITCH | HF_BACK);
}

void SoundMenuClass::Select(int entry_index)
{
	
}

void SoundMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMenu::OnDown(c);
}

void SoundMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMenu::OnUp(c);
}
void SoundMenuClass::Change(int entry_index, bool up)
{
	int tmp;

	switch(entry_index)
	{
	case 0:
		muted = !muted;
		sound_mute->SetText(ksGlobalTextArray[muted ? GT_MENU_SOUND_OFF : GT_MENU_SOUND_ON]);
		for(int i=0; i<5; i++) sound_levels[i]->Disable(muted);
#ifndef TARGET_XBOX
		sound_type->Disable(muted);
#endif
		if(muted) nslSetMasterVolume(0.0f);
		else nslSetMasterVolume(sounds[0]/((float) MAX_VOL));
		
		SoundScriptManager::inst()->unpause();
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		SoundScriptManager::inst()->pause();

		break;
	case 1:
		if(up) soundType++;
		else   soundType--;
		if(soundType > 2) soundType = 0;
		if(soundType < 0) soundType = 2;
			
		SoundScriptManager::inst()->unpause();
		sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		SoundScriptManager::inst()->pause();

		if(soundType == 0) sound_type->SetText(ksGlobalTextArray[GT_MENU_SOUND_STEREO]);
		else if(soundType == 1) sound_type->SetText(ksGlobalTextArray[GT_MENU_SOUND_PROLOGIC]);
		else sound_type->SetText(ksGlobalTextArray[GT_MENU_SOUND_MONO]);

		if(soundType == 0) nslSetSpeakerMode(NSL_SPEAKER_STEREO);
		else if(soundType == 1) nslSetSpeakerMode(NSL_SPEAKER_PROLOGIC);
		else nslSetSpeakerMode(NSL_SPEAKER_MONO); break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		tmp = entry_index - 2;
		if(up) sounds[tmp]++;
		else   sounds[tmp]--;
		if(sounds[tmp] > MAX_VOL)
		{
			SoundScriptManager::inst()->unpause();
			sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
			SoundScriptManager::inst()->pause();

			sounds[tmp] = MAX_VOL;
		}
		else if(sounds[tmp] < 0) 
		{
			SoundScriptManager::inst()->unpause();
			sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
			SoundScriptManager::inst()->pause();

			sounds[tmp] = 0;	
		}
		else
		{
			EventType soundtoPlay = SS_FE_LEFTRIGHT;
			switch (entry_index)
			{
				case 3: soundtoPlay = SS_FE_LEFTRIGHT_AMBIENT; break;
				case 4: soundtoPlay = SS_FE_LEFTRIGHT;         break;
				case 5: soundtoPlay = SS_FE_LEFTRIGHT_VOICE;   break;
				case 6: soundtoPlay = SS_FE_LEFTRIGHT_MUSIC;   break;
			}
			SoundScriptManager::inst()->unpause();
			sys->navigationEvent=  SoundScriptManager::inst()->playEvent(soundtoPlay);
			SoundScriptManager::inst()->pause();
		}
		sound_levels[tmp]->SetText(ksGlobalTextArray[GT_MENU_SOUND_MASTER+tmp]+ stringx(" ") +stringx(sounds[tmp]));
		break;
	default: assert(0);
	}

	switch(entry_index)
	{
	case 2:	nslSetMasterVolume(sounds[0]/((float) MAX_VOL)); break;
	case 3: nslSetVolume(NSL_SOURCETYPE_AMBIENT, sounds[1]/((float) MAX_VOL)); break;
	case 4:	nslSetVolume(NSL_SOURCETYPE_SFX, sounds[2]/((float) MAX_VOL)); break;
	case 5:	nslSetVolume(NSL_SOURCETYPE_VOICE, sounds[3]/((float) MAX_VOL)); break;
	case 6:	nslSetVolume(NSL_SOURCETYPE_MUSIC, sounds[4]/((float) MAX_VOL)); break;
	}
}

void SoundMenuClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();
	FEMenu::OnTriangle(c);
}

void SoundMenuClass::OnLeft(int c)
{
	Change(highlighted->entry_num, false);
}

void SoundMenuClass::OnRight(int c)
{
	Change(highlighted->entry_num, true);
}

/********************** CameraMenu *************************/

CameraMenuClass::CameraMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	int i;
	
	FEMenu::cons(m, x, y, c, ch, cha);
	
	sys = (PauseMenuSystem*) m;
	
	for(i = 0; i < 3; i++)
		menu_item[i] = NULL;
	
	setBack(sys->menus[PauseMenuSystem::OptionsMenu]);

	scale_high = 1.0f;
	SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
	flags |= FEMENU_WRAP;
}

CameraMenuClass::~CameraMenuClass()
{
}

void CameraMenuClass::Init()
{
	int num_menu_items;
	multiplayer = false;
	num_menu_items = 2;
	if (g_session_cheats[CHEAT_FPS_CAM].isOn())
	{	
		num_menu_items = 3;

		// Add the menu items
		menu_item[0] = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CAMERA_FOLLOW], this);
		menu_item[1] = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CAMERA_BEACH], this);
		menu_item[2] = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CAMERA_FPS], this);
		Add(menu_item[0]);
		Add(menu_item[1]);
		Add(menu_item[2]);
	}
	else
	{
		menu_item[0] = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CAMERA_FOLLOW], this);
		menu_item[1] = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CAMERA_BEACH], this);
		Add(menu_item[0]);
		Add(menu_item[1]);
	}
	FEMenu::Init();
}

void CameraMenuClass::OnActivate(void)
{
	if (sys->IsResumable(this))
		SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
	else
		SetHelpText(HF_UP | HF_DOWN | HF_SELECT | HF_BACK);

	HighlightDefault();
}

void CameraMenuClass::Select(int entry_index)
{
	int							ply = sys->pause_player;
	stringx						name;
	
	assert(sys->pause_player != -1);
	
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	switch(entry_index)
	{
	case Follow:
		if (ply == 0) name = "FOLLOW_CLOSE_CAM0";
		else name = "FOLLOW_CLOSE_CAM1";	
		break;
	case Beach:
		if (ply == 0) name = "BEACH_CAM0";
		else name = "BEACH_CAM1";	
		break;
	case FPS:
		if (ply == 0) name = "FPS_CAM0";
		else name = "FPS_CAM1";	
		break;
	default: assert(0);
	}

	g_world_ptr->get_ks_controller(ply)->SetPlayerCamera((game_camera *)find_camera(entity_id(name.c_str())));
	StoredConfigData::inst()->setLastCamera(ply, name.c_str());
}

void CameraMenuClass::Draw()
{
	FEMenu::Draw();
}

void CameraMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnDown(c);
}

void CameraMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();

	FEMenu::OnUp(c);
}

void CameraMenuClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();
	FEMenu::OnTriangle(c);
}

void CameraMenuClass::HighlightDefault(void)
{
	if (sys->pause_player == -1)
		FEMenu::HighlightDefault();
	else
	{
		kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(sys->pause_player);
		
		if (ksctrl->GetPlayerCam() == ksctrl->get_fps_cam_ptr() && num_entries >= 3)
			setHigh(menu_item[2], false);
		else if (ksctrl->GetPlayerCam() == ksctrl->get_beach_cam_ptr())
			setHigh(menu_item[1], false);
		else
			setHigh(menu_item[0], false);
	}
}

/********************** TrickTypeMenu *************************/

TrickTypeMenuClass::TrickTypeMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;
	setBack(sys->menus[PauseMenuSystem::PauseMenu]);
	color32 col_sp = frontendmanager.col_info_g;

	int sub_menu_count[NUM_TYPES];
	for(int i=0; i<NUM_TYPES; i++)
	{
		sub_menus[i] = NEW TrickMenuClass(m, x, y, c, ch, cha);
		sub_menus[i]->flags |= FEMENU_DONT_SHOW_DISABLED | FEMENU_HAS_COLOR_HIGH_ALT | FEMENU_WRAP;
		sub_menus[i]->color = frontendmanager.white;
		sub_menus[i]->color_high = frontendmanager.col_highlight;
		sub_menus[i]->color_high_alt = frontendmanager.col_highlight2;
		AddSubmenu(sub_menus[i]);
		entry[i] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_AERIALS+i], this);
		Add(entry[i]);
		sub_menu_count[i] = 0;
		sub_menus[i]->SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_BACK);
	}

	// Add entries to appropriate sub menu
	stringx tmp;
	SurferTrick* st;

	// temporary holding bin for special tricks to be added after
	FEMenuEntry* tmp_entries[NUM_TYPES][MAX_ENTRIES];
	int tmp_sp_num[NUM_TYPES][MAX_ENTRIES];
	int tmp_entries_count[NUM_TYPES];
	for(int i=0; i<NUM_TYPES; i++)
		tmp_entries_count[i] = 0;

	for(int i=0; i<TRICK_NUM; i++)
	{
		st = &GTrickList[i];
		bool has_buttons = (st->button1 != PAD_NONE);
		bool has_anim = st->anim_id != -1;
		int type = st->trickbook_type;

		if(has_buttons && has_anim && type != TRICKBOOKTYPE_NOTYPE)
		{
			tmp = ksGlobalTrickTextArray[st->trick_id];
			if(st->button1 != PAD_NONE)
			{
				tmp = tmp + ": ";
				
				// One-button tricks.
				if (st->button2 == PAD_NONE)
					tmp = tmp + " " + ksGlobalButtonArray[st->button1-1];
				// Two-button tricks.
				else if (st->button3 == PAD_NONE)
				{
					// Hack: display floater without the "+"
					if (i == TRICK_FLOATER)
						tmp = tmp + " " + ksGlobalButtonArray[st->button1-1] + " " + ksGlobalButtonArray[st->button2-1];
					else if (st->trickbook_type == TRICKBOOKTYPE_FACETRICK)
						tmp = tmp + " " + ksGlobalButtonArray[st->button1-1] + " , " + ksGlobalButtonArray[st->button2-1];
					// Normal display for all other tricks.
					else
						tmp = tmp + " " + ksGlobalButtonArray[st->button1-1] + " + " + ksGlobalButtonArray[st->button2-1];
				}
				// Three-button tricks.
				else
					tmp = tmp + ksGlobalButtonArray[st->button1-1] + ", " + ksGlobalButtonArray[st->button2-1] + ", " + ksGlobalButtonArray[st->button3-1];
			}
			FEMenuEntry* feme = NEW FEMenuEntry(tmp, sub_menus[type]);
			if(st->flags & SpecialFlag)
			{
				// set special tricks green
				feme->SetSpecialColor(col_sp, sub_menus[type]->color_high);
				tmp_entries[type][tmp_entries_count[type]] = feme;
				tmp_sp_num[type][tmp_entries_count[type]] = i;
				tmp_entries_count[type]++;
			}
			else
			{
				sub_menus[type]->Add(feme);
				trick_list_index[type][sub_menu_count[type]] = i;
				sub_menu_count[type]++;
			}
			assert(sub_menu_count[type] < MAX_ENTRIES);
		}
	}

	for(int i=0; i<NUM_TYPES; i++)
	{
		// add special tricks after everything else
		for(int j=0; j<tmp_entries_count[i]; j++)
		{
			sub_menus[i]->Add(tmp_entries[i][j]);
			trick_list_index[i][sub_menu_count[i]] = tmp_sp_num[i][j];
			sub_menu_count[i]++;
		}

		if(sub_menu_count[i] == 0) entry[i]->Disable(true);
	}

	scale_high = 1.0f;
	SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
	flags |= FEMENU_WRAP;
}

TrickTypeMenuClass::~TrickTypeMenuClass()
{
	for(int i=0; i<NUM_TYPES; i++)
		delete sub_menus[i];
}

void TrickTypeMenuClass::Init()
{
	FEMenu::Init();
	for(int i=0; i<NUM_TYPES; i++)
		if(!entry[i]->GetDisable())
			sub_menus[i]->Init();
}

void TrickTypeMenuClass::OnActivate()
{
	FEMenuEntry* tmp;
	int surfer_idx = -1;

	if( g_game_ptr->get_num_active_players() == 1 )
	{
		int active_player = g_game_ptr->get_active_player();
		assert( g_game_ptr->get_player_device( active_player ) == sys->pause_controller );
		surfer_idx = g_game_ptr->GetSurferIdx( active_player );
	}
	else
	{
		int active_player = g_game_ptr->get_player_from_device( sys->pause_controller );
		assert( g_game_ptr->get_player_device( active_player ) == sys->pause_controller );
		surfer_idx = g_game_ptr->GetSurferIdx( active_player );
	}
	// Disable tricks that haven't been learned yet, or don't exist for current surfer;
	for(int j=0; j<NUM_TYPES; j++)
	{
		tmp = sub_menus[j]->entries;

		int i=0;
		while(tmp)
		{
			int index = trick_list_index[j][i];
			bool not_learned = GTrickList[index].flags & SpecialFlag && !unlockManager.isSurferTrickUnlocked(index, true);  //  Check to see if a trick is open, but only if it it not currently being learned.
			bool not_avail = true;

			for(int k=0; k<TRICKBOOK_SIZE; k++)
				if(index == SurferDataArray[surfer_idx].trickBook[k])
				{
					not_avail = false;
					break;
				}

			tmp->Disable(not_avail || not_learned);
			tmp = tmp->next;
			i++;
		}
	}
	//FEMenu::OnActivate();
}

void TrickTypeMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	MakeActive(sub_menus[entry_index]);
}

void TrickTypeMenuClass::HighlightDefault(void)
{
	FEMenu::HighlightDefault();

	for (int i = 0; i < NUM_TYPES; i++)
	{
		sub_menus[i]->HighlightDefault();
		sub_menus[i]->setVis(sub_menus[i]->highlighted);
	}

	active = NULL;
}

void TrickTypeMenuClass::Update(time_value_t time_inc)
{
	if(active) active->Update(time_inc);
	else FEMenu::Update(time_inc);
}
void TrickTypeMenuClass::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMenu::OnUp(c);
}
void TrickTypeMenuClass::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMenu::OnDown(c);
}
void TrickTypeMenuClass::OnCross(int c)
{
	SoundScriptManager::inst()->unpause();
	if((!active) && (!highlighted->GetDisable())) Select(highlighted->entry_num);
	SoundScriptManager::inst()->pause();
}

void TrickTypeMenuClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();

	if(active)
		MakeActive(NULL);
	else FEMenu::OnTriangle(c);
}

/********************** TrickMenu *************************/

TrickMenuClass::TrickMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	scale_high = 1.0f;
}

void TrickMenuClass::OnActivate(void)
{
	FEMenuEntry *	oldHigh;
	FEMenuEntry *	oldVis;
	
	// Reposition entries.
	oldHigh = highlighted;
	oldVis = first_vis_entry;
	Init();
	if (oldHigh && !oldHigh->GetDisable())
		setHigh(oldHigh, false);
	else
		HighlightDefault();
	if (oldVis)
		setVis(oldVis);
}

void TrickMenuClass::Select(int entry_num)
{
	// Do nothing.
}

void TrickMenuClass::OnCross(int c)
{
	// Do nothing.
}

/****************PLAYLISTMENU****************************/

PlaylistMenuClass::PlaylistMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	active = 0; 
	tweaked = false;
	col = c; 
	colh= ch; 
	offset=0; 
	pos=0; 
	FEMenu::cons(m, x, y, c, ch, cha);
	upArrow   = NULL;
	downArrow = NULL;
	helpText2 = NULL;
	playing   = NULL;
	currentSong = NULL;
	currentArtist = NULL;
	for (int i=0; i < SONGS_PER_SCREEN; i++)
	{
		lineNumbers[i] = NULL;
		onOff[i]       = NULL;
	}
	
	sys = (PauseMenuSystem*) m;
	setBack(sys->menus[PauseMenuSystem::OptionsMenu]);
	SetHelpText(0);
	scale_high = 1.0f;
} 

PlaylistMenuClass::~PlaylistMenuClass()
{
	if (upArrow)
		delete upArrow;
	if (downArrow)
		delete downArrow;
	if (helpText2)
		delete helpText2;
	
	if (playing)
		delete playing;
	if (currentSong)
		delete currentSong;
	if (currentArtist)
		delete currentArtist;
	
	for (int i=0; i < SONGS_PER_SCREEN; i++)
	{
		if (lineNumbers[i])
			delete lineNumbers[i];
		if (onOff[i])
			delete onOff[i];
	}
	
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		delete[] song_names;
}

int pauseCount = 0;
void PlaylistMenuClass::OnActivate()
{
	int i;
	pauseCount =0;
	active = 0; 
	offset=0; 
	pos=0; 
	MusicMan::inst()->unpause();

	if (numSongs == 0)
		return;

	while (nslGetSoundStatus(MusicMan::inst()->musicTrack.getSoundId()) == NSL_SOUNDSTATUS_PAUSED)
	{
		nslUnpauseSound(MusicMan::inst()->musicTrack.getSoundId());
		pauseCount++;
	}


	if (!tweaked)
		songName[0]->SetPos(156, songName[0]->GetY() - 35);
	
	songName[0]->SetHJustify(Font::HORIZJUST_LEFT);
	songName[0]->SetText(MusicMan::inst()->getSongName(0));
	for ( i = 1; i < SONGS_PER_SCREEN; i++)
	{
		songName[i]->SetText(MusicMan::inst()->getSongName(i));

		songName[i]->Highlight(false,false);
		songName[i]->SetColor(col);
		if (!tweaked)
			songName[i]->SetPos(156, songName[i]->GetY() - 35);
		songName[i]->SetHJustify(Font::HORIZJUST_LEFT);
	}
		songName[0]->Highlight(true, true);

	tweaked = true;
	upArrow->changePos(upArrow->getX(), songName[0]->GetY());
	downArrow->changePos(downArrow->getX(), songName[SONGS_PER_SCREEN - 1]->GetY() + 4);

	stringx	str;
	str = stringx(ksGlobalButtonArray[GT_PadL1]) + stringx(" / ") +  stringx(ksGlobalButtonArray[GT_PadR1]) + 
		stringx(" ") + ksGlobalTextArray[GT_PLAYLIST_SWITCH] + 
		stringx("  ") + stringx(ksGlobalButtonArray[GT_PadCross]) + stringx(" ") + ksGlobalTextArray[GT_PLAYLIST_PLAY] + stringx("\n") +  ksGlobalButtonArray[GT_PadSquare] + 
		stringx(" ") + ksGlobalTextArray[GT_PLAYLIST_ENABLE] + stringx(" / ")  + ksGlobalTextArray[GT_PLAYLIST_DISABLE]  + stringx("  ") + 
		stringx(ksGlobalButtonArray[GT_PadBack]) + stringx(" ") + stringx(ksGlobalTextArray[GT_MENU_BACK]);
	if (sys->IsResumable(this))
		str += stringx(" ") + stringx(ksGlobalButtonArray[GT_PadStart]) + stringx(" ") + ksGlobalTextArray[GT_MC_CONTINUE];
	helpText2->changeText(str);
}

void PlaylistMenuClass::OnDown(int c)
{
	if (numSongs == 0)
		return;
	SoundScriptManager::inst()->unpause();
	if (pos < SONGS_PER_SCREEN - 1)
	{
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		if (!active)
		{
			songName[pos]->Highlight(false, false);
			pos++;
			songName[pos]->Highlight();
		}
		else
		{
			songName[pos]->Highlight(false, false);
			songName[pos]->SetColor(col);
			songName[pos + 1]->Highlight(true, true);
			if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				MusicMan::inst()->swap(pos + offset, pos + offset + 1);

			stringx tmp = song_names[pos+offset];
			song_names[pos+offset] = song_names[pos+offset+1];
			song_names[pos+offset+1] = tmp;

			for (int i=0; i < SONGS_PER_SCREEN; i++)
				songName[i]->SetText(song_names[i+offset]);

			pos++;
		}
	}
	else if(offset < numSongs - SONGS_PER_SCREEN)
	{
		offset++;
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		if (active)
		{
			songName[pos - 1]->Highlight(false, false);
			songName[pos - 1]->SetColor(col);
			songName[pos]->Highlight(true, true);
			if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				MusicMan::inst()->swap(pos - 1 + offset, pos + offset);
			
			stringx tmp = song_names[pos+offset];
			song_names[pos+offset] = song_names[pos+offset-1];
			song_names[pos+offset-1] = tmp;

			for (int i=0; i < SONGS_PER_SCREEN; i++)
				songName[i]->SetText(song_names[i+offset]);
		}
		for (int i=offset; i < offset + SONGS_PER_SCREEN; i++)
		{
			songName[i-offset]->SetText(song_names[i]);
			if ((i-offset) == pos)
				songName[i-offset]->SetColor(colh);
			else
				songName[i-offset]->SetColor(col);
		}
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	
	SoundScriptManager::inst()->pause();
}

void PlaylistMenuClass::OnUp(int c)
{
	if (numSongs == 0)
		return;
	SoundScriptManager::inst()->unpause();	
	if (pos > 0)
	{
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		if (!active)
		{
			songName[pos]->Highlight(false, false);
			pos--;
			songName[pos]->Highlight();
		}
		else
		{
			songName[pos]->Highlight(false, false);
			songName[pos]->SetColor(col);
			songName[pos - 1]->Highlight(true, true);
			if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				MusicMan::inst()->swap(pos + offset, pos + offset - 1);

			stringx tmp = song_names[pos+offset];
			song_names[pos+offset] = song_names[pos+offset-1];
			song_names[pos+offset-1] = tmp;

			for (int i=0; i < SONGS_PER_SCREEN; i++)
				songName[i]->SetText(song_names[i+offset]);

			pos--;
		}
	}
	else if (offset > 0)
	{
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		offset--;
		
		if (active)
		{
			songName[pos + 1]->Highlight(false, false);
			songName[pos + 1]->SetColor(col);
			
			songName[pos]->Highlight(true, true);
			if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				MusicMan::inst()->swap(pos + 1 + offset, pos + offset);

			stringx tmp = song_names[pos+offset];
			song_names[pos+offset] = song_names[pos+offset+1];
			song_names[pos+offset+1] = tmp;

			for (int i=0; i < SONGS_PER_SCREEN; i++)
				songName[i]->SetText(song_names[i+offset]);
		}
		for (int i=offset; i < offset + SONGS_PER_SCREEN; i++)
		{
			songName[i-offset]->SetText(song_names[i]);
			if ((i-offset) == pos)
				songName[i-offset]->SetColor(colh);
			else
				songName[i-offset]->SetColor(col);
		}
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);

	SoundScriptManager::inst()->pause();
}

void PlaylistMenuClass::OnTriangle(int c)
{
	MusicMan::inst()->pause();
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();
	while ((pauseCount > 0) && (nslGetSoundStatus(MusicMan::inst()->musicTrack.getSoundId()) != NSL_SOUNDSTATUS_INVALID))
	{
		nslPauseSound(MusicMan::inst()->musicTrack.getSoundId());
		pauseCount--;
	}
	FEMenu::OnTriangle(c);
}
void PlaylistMenuClass::Select(int entry_index)
{
	if (numSongs == 0)
		return;
	
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		SoundScriptManager::inst()->unpause();
		MusicMan::inst()->stop();
		MusicMan::inst()->setCurrent(pos + offset);
		if (MusicMan::inst()->isDisabled(pos + offset))
			MusicMan::inst()->disable(pos + offset, false);
		MusicMan::inst()->play();
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		SoundScriptManager::inst()->pause();
	}
	return;
}

void PlaylistMenuClass::AddSongsToList()
{
	char number[3];
	upArrow = NEW TextString(system->font, ksGlobalButtonArray[GT_PadU], 500, 126);
	downArrow = NEW TextString(system->font, ksGlobalButtonArray[GT_PadD], 500, 325);
	
	upArrow->no_color   = false;
	downArrow->no_color = false;

	playing = NEW TextString(system->font, stringx("@r") + ksGlobalTextArray[GT_PLAYLIST_NOW_PLAYING] + stringx(": "), 115, 300, 0.0f, .8f);
	playing->setHJustify(Font::HORIZJUST_LEFT);
	playing->no_color = true;
	
	
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{

		songName[0] = NEW FEMenuEntry(ksGlobalTextArray[GT_PLAYLIST_NO_SONGS], this, 1);  
		Add(songName[0]);
		numSongs = 0;
	}
	else
	{
		numSongs = MusicMan::inst()->getNumSongs();
		if (numSongs == 0)
		{
			songName[0] = NEW FEMenuEntry(ksGlobalTextArray[GT_PLAYLIST_NO_SONGS], this, 1);  
			Add(songName[0]);
		}
		song_names = NEW stringx[numSongs];
	}
	downArrow->color = col; 
	upArrow->color = col;

	helpText2 = NEW MultiLineString(system->font, "", 320, 353, 0.0f, .8f);
	helpText2->setHJustify(Font::HORIZJUST_CENTER);
	helpText2->no_color = false;
	helpText2->color = frontendmanager.col_info_g;
	
	if (numSongs == 0)
		return;
	
	currentSong = NEW TextString(system->font, stringx(MusicMan::inst()->getSongName(MusicMan::inst()->musicTrack.getCurrent())), 247, 300);
	currentSong->no_color = false;
	currentSong->color = col;
	currentSong->setHJustify(Font::HORIZJUST_LEFT);

	currentArtist = NEW TextString(system->font, stringx("@r")+ksGlobalTextArray[GT_PLAYLIST_BY] + stringx(" @w") + stringx(MusicMan::inst()->getArtistName(MusicMan::inst()->musicTrack.getCurrent())), 115, 320, 0.0, .8);
	currentArtist->no_color = true;
	currentArtist->color = col;
	currentArtist->setHJustify(Font::HORIZJUST_LEFT);
	
	for (int i=0; i < numSongs; i++)
		song_names[i] = MusicMan::inst()->getSongName(i);

	for (int j=0; j < SONGS_PER_SCREEN; j++)
	{
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			songName[j] = NEW FEMenuEntry(song_names[j], this);
			Add(songName[j]);
			
			sprintf(number, "%d.", j);
			lineNumbers[j] = NEW TextString(system->font, number, 0, 0);
			lineNumbers[j]->no_color = false;
			lineNumbers[j]->color = col;
			
			onOff[j] = NEW TextString(system->font, "{", 0, 0);
			onOff[j]->no_color = false;
		}
		else
		{
			songName[j] = NEW FEMenuEntry(ksGlobalTextArray[GT_PLAYLIST_NO_AUDIO], this, 1);  
		}
	}
}

void PlaylistMenuClass::Draw()
{
	char numbers[3];
	
	helpText2->Draw();
	FEMenu::Draw();
	
	if (numSongs == 0)
	{
		return;
	}
	for (int i=offset; i < offset + SONGS_PER_SCREEN; i++)
	{
		sprintf(numbers, "%d.", i+1);
		lineNumbers[i-offset]->changeText(numbers);
		lineNumbers[i-offset]->setHJustify(Font::HORIZJUST_RIGHT);
		lineNumbers[i-offset]->changePos(songName[i-offset]->GetX() - 10, songName[i-offset]->GetY());
		lineNumbers[i-offset]->Draw();
		onOff[i-offset]->setHJustify(Font::HORIZJUST_RIGHT);
		if(MusicMan::inst()->isDisabled(i))
			onOff[i-offset]->color = color32(255, 0,0,255);
		else
			onOff[i-offset]->color = color32(0,255,0,255);
		onOff[i-offset]->changePos(upArrow->getX() - 15,  songName[i-offset]->GetY());
		onOff[i-offset]->Draw();
	}
	
	playing->Draw();
	currentSong->Draw();
	currentArtist->Draw();
	if (offset != 0)
		upArrow->Draw();
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		if (offset < MusicMan::inst()->getNumSongs() - SONGS_PER_SCREEN)
			downArrow->Draw();
	}
}

void PlaylistMenuClass::Update(float time)
{
	FEMenu::Update(time);
	bool oneEnabled = false;
	if (numSongs == 0)
		return;
	
	for (int i=0; i < numSongs; i++)
		if (!MusicMan::inst()->musicTrack.isDisabled(i))
			oneEnabled = true;
		
		if (oneEnabled)
		{
			currentSong->changeText(MusicMan::inst()->getSongName(MusicMan::inst()->musicTrack.getCurrent()));
			currentArtist->changeText(stringx("@r") + ksGlobalTextArray[GT_PLAYLIST_BY] + stringx(": @w") + MusicMan::inst()->getArtistName(MusicMan::inst()->musicTrack.getCurrent()));
		}
		else
		{

			currentSong->changeText(ksGlobalTextArray[GT_PLAYLIST_NO_SONGS_ENABLED]);
			currentArtist->changeText("");    
		}
}

void PlaylistMenuClass::OnSquare(int c)
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		SoundScriptManager::inst()->pause();

		if (!MusicMan::inst()->isDisabled(pos+offset))
		{
			MusicMan::inst()->disable(pos+offset, true);
			if (MusicMan::inst()->musicTrack.getCurrent() == pos + offset)
			{
				MusicMan::inst()->stop();
				MusicMan::inst()->playNext();
			}
		}
		else
		{
			MusicMan::inst()->disable(pos+offset, false);
		}
	}
}

void PlaylistMenuClass::OnL1(int c)
{
    active = true;
    OnUp(c);
    active = false;
}

void PlaylistMenuClass::OnR1(int c)
{
    active = true;
    OnDown(c);
    active = false;
}


/********************** ReplayMenuClass *************************/

ReplayMenuClass::ReplayMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;
	setBack(sys->menus[PauseMenuSystem::EndRunMenu]);

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_PLAY], this));
	FEMenuEntry *saveEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_SAVE], this);
	saveEntry->Disable(true);
	Add(saveEntry);
	replayEvent=-1;
	
	scale_high = 1.0f;
	SetHelpText(HF_RESUME | HF_UP | HF_DOWN | HF_SELECT | HF_BACK);
	flags |= FEMENU_WRAP;
}

void ReplayMenuClass::OnActivate()
{
	ReplayStart();  // Bad hack to get rid of replay Play/Save menu
}

void ReplayMenuClass::Update(time_value_t time_inc)
{
	if(sys->replay_mode && ksreplay.Done())
	{
		//int active = frontendmanager.IGO->VCRGetButton();
		ReplayEnd();
    ksreplay.SetStatus(KSReplay::REPLAY_RECORD);
		
		/*switch(active)
		{
		case ReplayWidget::VCR_PAUSE:
			ksreplay.Pause(true);
			break;
		case ReplayWidget::VCR_SLOW:
			ksreplay.SpeedSlow();
			break;
		case ReplayWidget::VCR_FASTFORWARD:
			ksreplay.SpeedFast();
			break;
		}*/
	}
}

void ReplayMenuClass::OnLeft(int c)
{
	if(sys->replay_mode) 
	{
		frontendmanager.IGO->VCRHighlightLeft();
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		SoundScriptManager::inst()->pause();

	}
	else FEMenu::OnLeft(c);
}

void ReplayMenuClass::OnRight(int c)
{
	if(sys->replay_mode) 
	{
		frontendmanager.IGO->VCRHighlightRight();
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		SoundScriptManager::inst()->pause();

	}
	else FEMenu::OnRight(c);
}

void ReplayMenuClass::OnL2(int c)
{
	if(sys->replay_mode);// g_world_ptr->get_replay_cam_ptr()->toggle_debug_cam();
	else FEMenu::OnL2(c);
}

void ReplayMenuClass::OnCross(int c)
{
  static nslSoundId rwSnd=NSL_INVALID_ID;
	if(sys->replay_mode)
	{
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->endEvent(replayEvent);

		int active = frontendmanager.IGO->VCRGetButton();
		int highlight = frontendmanager.IGO->VCRGetHighlight();
		switch(highlight)
		{
		case ReplayWidget::VCR_RESTART:
      if(nslGetSoundStatus(rwSnd) != NSL_SOUNDSTATUS_INVALID)
        nslStopSound(rwSnd);
			SoundScriptManager::inst()->playSound(SS_REPLAY_CLICK);
			rwSnd = SoundScriptManager::inst()->playSound(SS_REPLAY_RW);
			ksreplay.Play();
			break;
		case ReplayWidget::VCR_PAUSE:
			replayEvent = SoundScriptManager::inst()->playEvent(SS_REPLAY_CLICK);
			ksreplay.Pause(true);
			break;
		case ReplayWidget::VCR_PLAY:
			replayEvent = SoundScriptManager::inst()->playEvent(SS_REPLAY_CLICK);
			ksreplay.SpeedNormal();
			ksreplay.Pause(false);
			break;
		case ReplayWidget::VCR_SLOW:
			replayEvent = SoundScriptManager::inst()->playEvent(SS_REPLAY_CLICK);
			ksreplay.SpeedSlow();
			ksreplay.Pause(false);
			break;
		case ReplayWidget::VCR_FASTFORWARD:
			SoundScriptManager::inst()->playEvent(SS_REPLAY_CLICK);
			replayEvent = SoundScriptManager::inst()->playEvent(SS_REPLAY_FF);
			ksreplay.SpeedFast();
			ksreplay.Pause(false);
			break;
		}

		if(highlight == ReplayWidget::VCR_RESTART)
		{
			switch(active)
			{
			case ReplayWidget::VCR_PAUSE:
				ksreplay.Pause(true);
				break;
			case ReplayWidget::VCR_SLOW:
				ksreplay.SpeedSlow();
				break;
			case ReplayWidget::VCR_FASTFORWARD:
				ksreplay.SpeedFast();
				break;
			}
		}
		else
		{
			active = highlight;
		}
		SoundScriptManager::inst()->pause();
		frontendmanager.IGO->VCRButtonSelect(active);
	}
	else FEMenu::OnCross(c);
}

void ReplayMenuClass::OnCircle(int c)
{
#ifdef BRONER
	if(sys->replay_mode)
  {
    //g_world_ptr->get_replay_cam_ptr()->toggle_player();
	  return;
  }
#endif
  FEMenu::OnCircle(c);
}


void ReplayMenuClass::OnStart(int c)
{
	if(sys->replay_mode) ReplayEnd();
	else FEMenu::OnStart(c);
}

void ReplayMenuClass::OnTriangle(int c)
{
	if(!sys->replay_mode)
	  FEMenu::OnTriangle(c);
}

void ReplayMenuClass::ReplayStart()
{
	// from EndRunMenu
	sys->manager->IGO->SetDisplay(sys->manager->score_display);
	sys->draw = false;
	sys->replay_mode = true;
  
	//replay_camera *replayCam = g_world_ptr->get_replay_cam_ptr();
	g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_board_controller().ResetPhysics();
	//replayCam->set_last_camera(app::inst()->get_game()->get_player_camera(app::inst()->get_game()->get_active_player()));

	//app::inst()->get_game()->set_player_camera(app::inst()->get_game()->get_active_player(), (camera*)replayCam);

	//app::inst()->get_game()->pause();
  
	frontendmanager.IGO->VCRButtonSelect(ReplayWidget::VCR_PLAY);
	frontendmanager.IGO->VCRHighlightSelect(ReplayWidget::VCR_PLAY);
  if(ksreplay.GetStatus() == KSReplay::REPLAY_RECORD || ksreplay.Done())
  {
	  ksreplay.Play();
	  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	  {
			MusicMan::inst()->unpause();
		  MusicMan::inst()->stop();
		  MusicMan::inst()->playNext();
	  }
  }
  else
  {
	  ksreplay.Resume();
  }
}

void ReplayMenuClass::ReplayChange(ReplayType option)
{
	// from EndRunMenu
	switch(option)
	{
	case ReplayPlay: 
		if (ksreplay.IsPlaying()) 
			ksreplay.Restart(); 
	case ReplaySlow: ksreplay.SpeedSlow();	break;
	case ReplayFF:   ksreplay.SpeedFast();break;
	}
}

void ReplayMenuClass::ReplayEnd()
{
	// from EndRunMenu
	ksreplay.Pause(true);
	sys->replay_mode = false;
	sys->draw = true;
	sys->manager->IGO->SetDisplay(false);
	sys->manager->IGO->OnReplayEnd();
	sys->manager->pms->replay_mode = false;
	//replay_camera *replayCam = g_world_ptr->get_replay_cam_ptr();
	//app::inst()->get_game()->set_player_camera(app::inst()->get_game()->get_active_player(), replayCam->get_last_camera());

	//app::inst()->get_game()->unpause();
  
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		MusicMan::inst()->stop();
		MusicMan::inst()->pause();
	}

  OnTriangle(0);  // Bad hack to get rid of replay Play/Save menu
}

/********************** GoalsMenuClass *************************/

GoalsMenuClass::GoalsMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	stringx	s;
	
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem *) m;

	// Allocate text.
	s.printf(ksGlobalTextArray[GT_GOALS_TITLE].c_str(), BeachDataArray[g_game_ptr->get_beach_id()].fe_name);
	title = NEW TextString(&sys->manager->font_bold, s, 320, 75, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, sys->manager->col_highlight2);

	int extra_lines = 0;
	int spacing = 55;
	for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
	{
		g_career->GetGoalText(g_game_ptr->get_level_id(), g, s);
		int y = IGOFrontEnd::POS_GOALS_TOP + (g*spacing) + extra_lines*20;
		names[g] = NEW BoxText(sys->font, s, 320, y, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, sys->manager->col_info_g);
		extra_lines += names[g]->makeBox(502, 200) - 1;
		y = IGOFrontEnd::POS_GOALS_TOP + (g*spacing) + (extra_lines+1)*20;
		status[g] = NEW TextString(sys->font, "", 320, y, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, sys->manager->col_info_g);
	}

	scale_high = 1.0f;
	helpText->changePos(320, 424);

	tip = NEW BoxText(&frontendmanager.font_bold_old, "", 148, 355, 0, 0.666f*1.18f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, c);
	tip->makeBox(344, 100);
}

GoalsMenuClass::~GoalsMenuClass()
{
	delete title;
	for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
	{
		delete names[g];
		delete status[g];
	}
	delete tip;
}

void GoalsMenuClass::Load(PanelFile* panel)
{
	float x, y;
	for(int i=0; i<9; i++)
	{
		tip_box[i] = panel->GetPointer(("tip_box_0"+stringx(i+1)).data());
		tip_box[i]->GetCenterPos(x, y);
		tip_box[i]->SetCenterPos(x, y+80);
	}
}

void GoalsMenuClass::Draw()
{
	title->Draw();
	for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
	{
		names[g]->Draw();
		status[g]->Draw();
	}
	helpText->Draw();

	if(show_tip)
	{
		for(int i=0; i<9; i++)
			tip_box[i]->Draw(0);
		tip->Draw();
	}
}

void GoalsMenuClass::SetAsTip()
{
	static const int spacing = 40;
	static const int extra_spacing = 20;
	int extra_lines = 0;
	float scale = 0.9f;

	for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
	{
		int y = IGOFrontEnd::POS_GOALS_TOP + (i*spacing) + extra_lines*extra_spacing;
		names[i]->changePos(320, y);
		names[i]->changeScale(scale);
		extra_lines += names[i]->makeBox(502, 200) - 1;
		y = IGOFrontEnd::POS_GOALS_TOP + (i*spacing) + (extra_lines+1)*extra_spacing;
		status[i]->changePos(320, y);
		status[i]->changeScale(scale);
	}
}

void GoalsMenuClass::SetNoTip()
{
	static const int spacing = 50;
	static const int extra_spacing = 20;
	int extra_lines = 0;
	float scale = 1.0f;

	for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
	{
		int y = IGOFrontEnd::POS_GOALS_TOP + (i*spacing) + extra_lines*extra_spacing;
		names[i]->changePos(320, y);
		names[i]->changeScale(scale);
		extra_lines += names[i]->makeBox(502, 200) - 1;
		y = IGOFrontEnd::POS_GOALS_TOP + (i*spacing) + (extra_lines+1)*extra_spacing;
		status[i]->changePos(320, y);
		status[i]->changeScale(scale);
	}
}

void GoalsMenuClass::OnActivate()
{
	// Don't show help text during flyby.
	if (g_world_ptr->get_ks_controller(0) && g_world_ptr->get_ks_controller(0)->get_super_state() == SUPER_STATE_FLYBY)
	{
		SetHelpText(0);
		tip->changeText(((TipMenuClass*) sys->menus[PauseMenuSystem::TipMenu])->NextTip());
		tip->makeBox(344, 100);
		SetAsTip();
		show_tip = true;
	}
	else
	{
		// If this menu was activated from the pause menu, then allow resume option.
		if (sys->IsResumable(this))
			SetHelpText(HF_BACK | HF_RESUME);
		else
			SetHelpText(HF_BACK);
		show_tip = false;
		SetNoTip();
	}

	// Set status to show if goal is done or not.
	for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
	{
		if(CareerDataArray[g_game_ptr->get_level_id()].goal[g] != GOAL_NOTHING)
		{
			if (g_career->levels[g_game_ptr->get_level_id()].IsGoalDone(g))
			{
				status[g]->changeText(ksGlobalTextArray[GT_GOALS_DONE]);
				status[g]->color = sys->manager->col_highlight;
			}
			else
			{
				status[g]->changeText(ksGlobalTextArray[GT_GOALS_NOT_DONE]);
				status[g]->color = sys->manager->col_info_b;
			}
		}
	}

	wasMenuBGOn = sys->manager->IGO->IsMenuBGShown();
	sys->manager->IGO->ShowMenuBackground(false);
	sys->manager->IGO->ShowAccompBackground(true);
}

void GoalsMenuClass::OnUnactivate(FEMenu * m)
{
	if (m && wasMenuBGOn)
		sys->manager->IGO->ShowMenuBackground(true);
	sys->manager->IGO->ShowAccompBackground(false);
}

void GoalsMenuClass::OnUp(int c)
{

}

void GoalsMenuClass::OnDown(int c)
{

}

void GoalsMenuClass::OnCross(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	SoundScriptManager::inst()->pause();
}

void GoalsMenuClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();

	FEMenu::OnTriangle(c);
}

/********************** Tips Menu *************************/

TipMenuClass::TipMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	Add(NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_NEXT_TIP], this));
	tip_list = NULL;
	tip_list_size = -1;
	tip = NEW BoxText(&frontendmanager.font_bold_old, "", 148, 200, 0, 0.666f*1.18f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, c);

	scale_high = 1.0f;
	SetHelpText(HF_RESUME | HF_SELECT | HF_BACK);
	flags |= FEMENU_WRAP;
}

TipMenuClass::~TipMenuClass()
{
	delete tip;
	delete[] tip_list;
}

void TipMenuClass::Init()
{
	FEMenu::Init();
	// only 1 entry to place
	entries->SetPos(320, 280);
}

void TipMenuClass::Load()
{
	static const int last_beginner_lev = LEVEL_ANTARCTICA_1;
	static const int last_inter_lev = LEVEL_CURRENSPOINT_1;

	FEMenu::Load();
	assert(tip_list == NULL);

	stringx filename;
	if(g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		int lev = g_game_ptr->get_level_id();
		if(lev == LEVEL_INDOOR_1) filename = "interface\\igo\\tips_tutorial";
		else if(lev == LEVEL_INDOOR_2) filename = "interface\\igo\\tips_tutorial2";
		else if(lev == LEVEL_INDOOR_3) filename = "interface\\igo\\tips_tutorial3";
		else if(lev <= last_beginner_lev) filename = "interface\\igo\\tips";
		else if(lev <= last_inter_lev) filename = "interface\\igo\\tips2";
		else filename = "interface\\igo\\tips3";
	}
	else filename = "interface\\igo\\tips";

	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
		filename += "_fr.txt";
	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
		filename += "_ge.txt";
	else filename += ".txt";

	nglFileBuf file;
	file.Buf=NULL; file.Size=0;
	KSReadFile(filename.data(), &file, 1);
	unsigned char* buffer = (unsigned char*) file.Buf;
	if(!buffer)
	{
		KSReleaseFile(&file);
		tip_list_size = -1;
		tip_list = NULL;
		return;
	}

	buffer[file.Size] = '\0';	// terminate string, using the extra byte allocated in wds_readfile (dc 05/19/02)

	unsigned int cur_char_index = 0;
	int tip_line_index = 0;
	int index_within_line = 0;
	tip_list_size = -1;
	char current_line[max_line_size];

	while(cur_char_index < file.Size && buffer[cur_char_index] != '\x04')
	{
		while(buffer[cur_char_index] != '\n' && buffer[cur_char_index] != '\r' && buffer[cur_char_index] != '\x04')
		{
			if(index_within_line < max_line_size)
				current_line[index_within_line++] = buffer[cur_char_index++];
			else
			{
				index_within_line++;
				cur_char_index++;
			}
		}

		if(buffer[cur_char_index] == '\r' && buffer[cur_char_index+1] == '\n')
			cur_char_index++;	// skip over the \n as well
		cur_char_index++;

		if(index_within_line >= max_line_size)
			current_line[max_line_size-1] = '\0';
		else
			current_line[index_within_line] = '\0';

		if(tip_list_size == -1)
		{
			tip_list_size = atoi(current_line);
			tip_list = NEW stringx[tip_list_size];
			current_line[0] = '\0';
			index_within_line = 0;
		}
		else if(current_line[0] != '\0')
		{
			assert(tip_list);
			assert(tip_line_index < tip_list_size);
			tip_list[tip_line_index++] = stringx(current_line);
			current_line[0] = '\0';
			index_within_line = 0;
		}
	}

	KSReleaseFile(&file);
}

void TipMenuClass::OnActivate(void)
{
	SetTip();

	// If this menu was activated from the pause menu, then allow resume option.
	if (sys->IsResumable(this))
		SetHelpText(HF_BACK | HF_RESUME);
	else
		SetHelpText(HF_BACK);

	//FEMenu::OnActivate();
}

void TipMenuClass::Select(int entry_index)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();

	assert(entry_index == 0);
	SetTip();
}

void TipMenuClass::SetTip()
{
	tip->changeText(NextTip());
	tip->makeBox(344, 100);
}

stringx TipMenuClass::NextTip()
{
	if(tip_list_size == 0 || tip_list == NULL) return stringx("");
	int r = random(tip_list_size);
	assert(r >= 0 && r < tip_list_size);
	return tip_list[r];
}

void TipMenuClass::Draw()
{
	FEMenu::Draw();
	tip->Draw();
}

void TipMenuClass::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	sys->navigationEvent=  SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();
	FEMenu::OnTriangle(c);

}

/********************** QuitConfirmMenuClass *******************/

QuitConfirmMenuClass::QuitConfirmMenuClass(FEMenuSystem* m, int x, int y, color32 c, color32 ch, color32 cha)
{
	FEMenu::cons(m, x, y, c, ch, cha);
	sys = (PauseMenuSystem*) m;

	yesEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_YES], this);
	Add(yesEntry);
	noEntry = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_NO], this);
	Add(noEntry);

	question = NEW BoxText(sys->font, ksGlobalTextArray[GT_MENU_QUIT_CONFIRM], 320, 140, 0, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, frontendmanager.white, 10);
	question->makeBox(300, 200);

	SetHelpText(HF_SELECT | HF_BACK);
	flags |= FEMENU_WRAP;
	scale_high = 1.0f;
}

void QuitConfirmMenuClass::OnActivate(void)
{
	FEMenu::OnActivate();

	setHigh(noEntry, false);
}

void QuitConfirmMenuClass::Draw()
{
	FEMenu::Draw();
	question->Draw();
}

void QuitConfirmMenuClass::Select(int entry_index)
{
	switch(entry_index)
	{
	case YES:	sys->ActivateAndExit(); break;
	case NO:	FEMenu::OnTriangle(0); break;
	default: assert(0); break;
	}
}

/********************** GoalsMenuClass *************************/

LostControllerMenuClass::LostControllerMenuClass(FEMenuSystem* m, int x, int y, color32 c)
{
	FEMenu::cons(m, x, y, c, c, c);
	sys = (PauseMenuSystem*) m;

	// add the "reinsert controller" message
	FEMenuEntry *bob = NEW FEMenuEntry(ksGlobalTextArray[GT_LOST_CONTROLLER], this);
	Add(bob);
	bob->SetHJustify(Font::HORIZJUST_CENTER);

	scale_high = 1.0f;
	SetHelpText(0);
	flags |= FEMENU_WRAP;
}

void LostControllerMenuClass::Init()
{
	flags &= ~FEMENU_USE_SCALE;
	flags |= FEMENU_NO_FLASHING;

	FEMenu::Init();
}

void LostControllerMenuClass::OnActivate()
{
	sys->manager->IGO->ShowMenuBackground(true);
}

void LostControllerMenuClass::Update()
{
	input_mgr *inputmgr = input_mgr::inst();
	// if controller is re-connected
	if(inputmgr->get_device(g_world_ptr->get_ks_controller(0)->get_joystick_num())->is_connected())
	{
		if(g_game_ptr->get_num_active_players()  != 2 ||
		   inputmgr->get_device(g_world_ptr->get_ks_controller(1)->get_joystick_num())->is_connected())
		{
			sys->MakeActive(PauseMenuSystem::PauseMenu);
		}
	}
}

/********************** PauseMenuSystem *************************/

PauseMenuSystem::PauseMenuSystem(FEManager* man, Font* f)
{
	cons(NumMenus, man, f);
	int x = 320;
	int y = 240;
	color32 col = man->col_unselected;
	color32 colh = man->col_highlight;
	color32 colha = man->col_highlight2;

	old_device_flags = 0;
	pause_controller = -1;
	pause_player = -1;
	
	Add(NEW PauseMenuClass(this, x, y, col, colh, colha));
	Add(NEW TimeAttackPauseMenuClass(this, x, y, col, colh, colha));
	Add(NEW TutorialPauseMenuClass(this, x, y, col, colh, colha));
	Add(NEW EndRunMenuClass(this, x, y, col, colh, colha));
	Add(NEW HeatMidMenuClass(this, x, y, col, colh, colha));
	Add(NEW HeatEndMenuClass(this, x, y, col, colh, colha));
	Add(NEW CompEndMenuClass(this, x, y, col, colh, colha));
	Add(NEW OptionsMenuClass(this, x, y, col, colh, colha));
	Add(NEW GoalsMenuClass(this, x, y, col, colh, colha));
	Add(NEW LostControllerMenuClass(this, x, y, col));
	
	SoundMenuClass* sound = NEW SoundMenuClass(this, x, y, col, colh, colha);
	Add(sound);
	Add(NEW CameraMenuClass(this, x, y, col, colh, colha));
	Add(NEW TrickTypeMenuClass(this, x, y, col, colh, colha));
	PlaylistMenuClass* musiclist = NEW PlaylistMenuClass(this, x, y, col, colh, colha);
	musiclist->AddSongsToList();
	Add(musiclist);
	Add(NEW ReplayMenuClass(this, x, y, col, colh, colha));
	Add(man->map);
	Add(NEW HighScoreFrontEnd(this, man, "interface\\igo\\", "high_score.PANEL", true));
	Add(NEW PhotoFrontEnd(this, man, "challenges\\photo\\", "photo.panel"));
	Add(NEW TutorialFrontEnd(this, man));
	Add(NEW SaveLoadFrontEnd(this, man, "levels\\frontend\\overlays\\", "save_load.PANEL", false));
	Add(NEW SaveCareerPromptClass(this, x, y, col, colh, colha));
	Add(NEW TipMenuClass(this, x, y, col, colh, colha));
	Add(NEW QuitConfirmMenuClass(this, x, y, col, colh, colha));
	MakeActive(PauseMenu);
	draw = false;
	replay_mode = false;
	end_level = false;
	InitAll();
	popupEvent = -1;
	navigationEvent = -1;
	controller_disconnected = false;

	player = NEW TextString(&manager->font_bold, "", 320, 75, 0, 1.2f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, col);
}

PauseMenuSystem::~PauseMenuSystem()
{
	// delete all menus except for the map menu
	for(int i=0; i<size; i++)
		if(i != MapMenu) delete menus[i];
	delete player;
}

void PauseMenuSystem::InitAll()
{
	// Setup inital controller flags
	game *game_ptr = app::inst()->get_game();
	int player_1_device = 1 << game_ptr->get_player_device( 0 );
	int player_2_device = 1 << game_ptr->get_player_device( 1 );
	if( game_ptr->get_num_players() > 1 )
		SetDeviceFlags( player_1_device | player_2_device );
	else
		SetDeviceFlags( player_1_device );
	
	for(int i=0; i<FEMENUCMD_END; i++)
		for(int j=0; j<NUM_CONTROLLER_PORTS; j++)
			button_down[i][j] = getButtonState(i, j);
	for(int i=0; i<count; i++)
	{
		if (i != MapMenu)
			menus[i]->Init();
	}
	menus[HighScoreMenu]->Load();
	if (g_beach_ptr->get_challenges()->photo) 
		menus[PhotoMenu]->Load();
	menus[SaveLoadMenu]->Load();
	menus[TipMenu]->Load();
}

void PauseMenuSystem::Load()
{
	((GoalsMenuClass*) menus[GoalsMenu])->Load(&manager->IGO->panel);
}

void PauseMenuSystem::ActivateAndExit()
{
	this->startDraw();
	manager->IGO->ShowMenuBackground(false);	// Turn off pause menu overlay (dc 05/19/02)
	((BeachFrontEnd *)menus[MapMenu])->exitingWithoutSelect=true;	// must come before next line (dc 05/18/02)
	EndLevel();
	((BeachFrontEnd*) menus[MapMenu])->PrepareForLoading();
}

void PauseMenuSystem::startDraw(int menu_num, const bool pauseGame)
{
	//  Don't allow the secondary camera to be active while paused.
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
	ksctrl->end_secondary_cam();

	// Enable this controller only for the pause menu
	old_device_flags = GetDeviceFlags();
	if( pause_controller != -1 )
		SetDeviceFlags( 1 << pause_controller );
	
	if( g_game_ptr->get_num_active_players() == 1 )
	{
		pause_player = g_game_ptr->get_active_player();
	}
	else
	{
		if( pause_controller != -1 )
			pause_player = g_game_ptr->get_player_from_device( pause_controller );
		else
			pause_player = -1;
	}
	
	assert( pause_player >= -1 );
	assert( pause_player < 2 );
		
	if (pauseGame)
		app::inst()->get_game()->pause();
	
	if(menu_num == -1)
	{
		// Popup HeatMidMenu for competitions.
		if (g_game_ptr->is_competition_level() &&
			g_game_ptr->get_game_mode() == GAME_MODE_CAREER &&
			manager->IGO->GetRunState() == IGOFrontEnd::RUNSTATE_NORMAL)
			MakeActive(HeatMidMenu);
		// Popup TimeAttackPauseMenu for Time Atttack mode.
		else if (g_game_ptr->get_game_mode() == GAME_MODE_TIME_ATTACK)
			MakeActive(TimeAttackPauseMenu);
		// Popup TimeAttackPauseMenu for Meter Attack mode.
		else if (g_game_ptr->get_game_mode() == GAME_MODE_METER_ATTACK)
			MakeActive(TimeAttackPauseMenu);
		// Popup TutorialPauseMenu for Tutorial mode.
		else if (g_game_ptr->get_beach_id() == BEACH_INDOOR &&
			g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
			MakeActive(TutorialPauseMenu);
		// Popup PauseMenu otherwise.
		else
			MakeActive(PauseMenu);
		SoundScriptManager::inst()->unpause();
	
		SoundScriptManager::inst()->endEvent(popupEvent);
		popupEvent = SoundScriptManager::inst()->playEvent(SS_PAUSE_POPUP);
		SoundScriptManager::inst()->pause();

		manager->IGO->ShowMenuBackground(true);
	}
	else
	{
		MakeActive(menu_num);
		if (menu_num == TutorialMenu)
			manager->IGO->ShowMenuBackground(true);
		/*
		// bring up high score if run has ended, and not multiplayer
		// Ouch, this is a bad hack --- overriding the input! (dc 04/28/02)
		if(g_game_ptr->get_num_players() != 2 && (menu_num == HeatEndMenu || menu_num == EndRunMenu || menu_num == CompEndMenu) &&
			!manager->IGO->GetTutorialManager())
		{
			int idx;
			idx = ((PhotoFrontEnd *) menus[PauseMenuSystem::PhotoMenu])->GetSelectedPhotoIdx();
			// show high scores first
			MakeActive(HighScoreMenu);
			if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && (g_career->WasNewGoalPassed() || (idx != -1)))
			{
				((HighScoreFrontEnd*) menus[HighScoreMenu])->SetNextMenu(SaveConfirmMenu);
				((SaveCareerPromptClass*) menus[SaveConfirmMenu])->SetNextMenu(menu_num);
			}
			else
				((HighScoreFrontEnd*) menus[HighScoreMenu])->SetNextMenu(menu_num);
		}
		else
		{			
			//manager->IGO->ShowMenuBackground(true);	// causes menu frame to draw while map slides off (dc 04/28/01)
		}
		*/
	}
	
	//  Usually turn off the regular IGO when paused, but not for the tutorial message screen.
	if (menu_num != TutorialMenu)
		manager->IGO->SetDisplay(false);
	//manager->IGO->ShowMenuBackground(true);


	if(g_game_ptr->get_game_mode() == GAME_MODE_PUSH || g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD)
	{
		if( pause_player != -1 )
		{
			if(pause_player == 0)
				player->changeText(ksGlobalTextArray[GT_FE_MENU_PLAYER_1]);
			else player->changeText(ksGlobalTextArray[GT_FE_MENU_PLAYER_2]);
		}
	}

	draw = true;
}

void PauseMenuSystem::endDraw(const bool unpause)
{
	int	i;
	
	if (!draw)
		return;
	
	// Reenable old devices when pause menu is cleared
	SetDeviceFlags( old_device_flags );
	pause_controller = -1;
	pause_player = -1;

	if (!replay_mode)
	{
		kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());

		if (unpause)
			app::inst()->get_game()->unpause();

		if (g_game_ptr->get_beach_id() == BEACH_INDOOR && g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
			manager->IGO->SetDisplay(true);
		else if (ksctrl->get_super_state() != SUPER_STATE_FLYBY && manager->IGO->GetRunState() == IGOFrontEnd::RUNSTATE_NORMAL)
			manager->IGO->SetDisplay(manager->score_display);

		manager->IGO->ShowMenuBackground(false);
	}

	// Reset all menus' highlighted entries to defaults.
	for (i = 0; i < NumMenus; i++)
		menus[i]->HighlightDefault();

	menus[active]->OnUnactivate(NULL);


	draw = false;
}


void PauseMenuSystem::Update(time_value_t time_inc)
{
	if (IsDebugMenuDisplayed())
	{
		FEMenuSystem::UpdateButtonDown();  // Must call this when Update() is not called or FEMenuSystem::button_down array may become invalid
		return;
	}
	FEMenuSystem::Update(time_inc);

	if (end_level)
	{
		end_level = false;
		app::inst()->get_game()->end_level();
	}
}

void PauseMenuSystem::UpdateInScene()
{
	if(!draw) return;
	assert(menus[active]);
	menus[active]->UpdateInScene();
}

void PauseMenuSystem::Draw()
{
	if (draw)
	{
		FEMenuSystem::Draw();

		if (g_game_ptr->get_num_active_players() > 1 && pause_player != -1)
		{
			if(active != LostControllerMenu && active != MapMenu && active != HighScoreMenu && active != PhotoMenu
				&& active != SaveLoadMenu && active != EndRunMenu)
				player->Draw();
		}
	}
}

void PauseMenuSystem::MakeActive(int m, int sm)
{
	if((m == OptionsMenu || m == ReplayMenu || m == TrickTypeMenu || m == GoalsMenu || m == TipMenu || m == QuitConfirmMenu) &&
	   (active == PauseMenu || active == TimeAttackPauseMenu || active == TutorialPauseMenu || active == EndRunMenu || active == HeatMidMenu || active == HeatEndMenu || active == CompEndMenu))
		menus[m]->setBack(menus[active]);
	FEMenuSystem::MakeActive(m, sm);
}

void PauseMenuSystem::Select(int menu_index, int entry_index)
{
	assert(menus[menu_index]);
	menus[menu_index]->Select(entry_index);
}

bool PauseMenuSystem::SetDisconnect(bool b)
{
	if( !b )
	{
		controller_disconnected = false;
		return false;
	}
	
	if( manager->IGO->GetRunState() == IGOFrontEnd::RUNSTATE_NORMAL
			&& !replay_mode && !ksreplay.IsPlaying()
			&& active != HighScoreMenu )
		controller_disconnected = true;

	return controller_disconnected;
}
	
void PauseMenuSystem::OnButtonPress(int b, int c)
{
	bool	inFlyby = false;

	// Check if we are in flyby mode.
	if (g_game_ptr->get_num_active_players() == 1 && g_world_ptr->get_ks_controller(0) && g_world_ptr->get_ks_controller(0)->get_super_state() == SUPER_STATE_FLYBY)
		inFlyby = true;
	
	if( GetDisconnect() )
		inFlyby = false;
	
	// Toggle the current menu.
	if (b == FEMENUCMD_START &&
		!replay_mode && !ksreplay.IsPlaying() &&	// except during replay
		!inFlyby && // except during flybys
		manager->IGO->GetRunState() == IGOFrontEnd::RUNSTATE_NORMAL &&	// except during end run screens
		active != HighScoreMenu)	// except while entering/viewing a high score
	{
		// If off, turn it on.
		if (!draw)
		{
			pause_controller = c;
			startDraw();
#if defined(TARGET_XBOX)
			//return;
#endif
		}
		// If on, turn it off.
		else
		{
			bool	end = true;
			bool still_disconnected;
			
			still_disconnected = ( GetDisconnect() && g_game_ptr->get_disconnect_status() );
			
			// Shouldn't be able to start-button your way out of these menus
			if (active == MapMenu || active == HeatEndMenu || active == EndRunMenu || active == CompEndMenu || active == SaveLoadMenu || active == QuitConfirmMenu)
				end = false;

			// Special case (Options, Goals, or Tip menu at the end of a heat or run)
			if((active == GoalsMenu || active == TipMenu || active == OptionsMenu) && 
				(menus[active]->back != menus[PauseMenu] &&
				menus[active]->back != menus[HeatMidMenu] &&
				menus[active]->back != menus[TimeAttackPauseMenu] &&
				menus[active]->back != menus[TutorialPauseMenu]))
				end = false;

			// Similar special case (Camera, Sound, or Playlist menu, inside Options menu 
			// at the end of a heat or run)
			if((active == CameraMenu || active == SoundMenu || active == PlaylistMenu) && 
				(menus[active]->back->back != menus[PauseMenu] &&
				menus[active]->back->back != menus[HeatMidMenu] &&
				menus[active]->back->back != menus[TimeAttackPauseMenu] &&
				menus[active]->back->back != menus[TutorialPauseMenu]))
				end = false;

			// Start button does not toggle the pause menu off on Xbox.
#if defined(TARGET_XBOX)
			//end = false;
#endif

			// If ok to close the current menu, then do so.
			if (end && !still_disconnected)
			{
				SetDisconnect( false );
				endDraw();
			}
		}
	}

	if (draw || replay_mode)
		FEMenuSystem::OnButtonPress(b, c);
}

void PauseMenuSystem::Retry()
{
	//Only retry if the pause menu wasn't created from a controller disconnect
	//and the controller is currently connected
	if( !GetDisconnect() || !g_game_ptr->get_disconnect_status() )
	{
		endDraw();
		g_game_ptr->retry_mode();
	}
}

void PauseMenuSystem::EndLevel()
{
	frontendmanager.OnLevelEnding();

	// Toby change: we always want to be able to select a different level after ending.
	//
	//if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER || ((BeachFrontEnd*)menus[MapMenu])->exitingWithoutSelect)
	//{
		MakeActive(MapMenu);
		manager->in_game_map_up = true;
		manager->IGO->ShowMenuBackground(false);
	//}
	//else
	//	end_level = true;
}

void PauseMenuSystem::EndRun()
{
	endDraw();
	manager->IGO->SetDisplay(false);
	manager->IGO->EndRun();
}

void PauseMenuSystem::RestartComp()
{
	endDraw();
	g_game_ptr->retry_level();
	g_beach_ptr->judges.OnCompetitionReset();
}

void PauseMenuSystem::EndComp()
{
	endDraw();
	manager->IGO->SetDisplay(false);
	manager->IGO->EndCompetition();
}

void PauseMenuSystem::Restart(void)
{
	endDraw();
	g_game_ptr->retry_mode();
}

bool PauseMenuSystem::IsResumable(FEMenu * m) const
{
	if (m == menus[PauseMenu] ||
		m == menus[HeatMidMenu] ||
		m == menus[TimeAttackPauseMenu] ||
		m == menus[TutorialPauseMenu])
		return true;

	if (m)
	{
		if (m->back == menus[PauseMenu] ||
			m->back == menus[HeatMidMenu] ||
			m->back == menus[TimeAttackPauseMenu] ||
			m->back == menus[TutorialPauseMenu])
			return true;

		if (m->back)
		{
			if (m->back->back == menus[PauseMenu] ||
				m->back->back == menus[HeatMidMenu] ||
				m->back->back == menus[TimeAttackPauseMenu] ||
				m->back->back == menus[TutorialPauseMenu])
				return true;
		}
	}
	
	return false;
}

bool IsDebugMenuDisplayed(void)
{
	return menus->IsActive();
}

