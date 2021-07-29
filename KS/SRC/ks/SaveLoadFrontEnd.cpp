// SaveLoadFrontEnd.cpp
//
// Note that this file isn't used on the xbox.
// Instead, HWPOSXB\xb_SaveLoadFrontEnd.cpp is used.

#include "SaveLoadFrontEnd.h"
#include "FEPAnel.h"
#include <time.h>
#include "FEMenu.h"

#include "FrontEndManager.h"

#ifdef TARGET_PS2
#include <libcdvd.h>
#endif

#include "GameData.h"
extern bool careerStarted;
SaveLoadFrontEnd::SaveLoadFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq, bool in_fe)
{
	cons(s, man, p, pq);
	in_frontend = in_fe;
	if(in_frontend) sys = (GraphicalMenuSystem*) s;
	progressval = 0;
	fileType = 0;

	color = man->col_unselected;
	color_high = man->col_highlight;
	color_high_alt = man->col_highlight2;
	flags |= FEMENU_HAS_COLOR_HIGH_ALT;
	scale = scale_high = 0.8f;

	entry[SL_SAVE] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_SAVE], this);
	entry[SL_LOAD] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_LOAD], this);
	entry[SL_DELETE] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_DELETE], this);
	for(int i=0; i<lsd_num; i++)
	{
		Add(entry[i]);
		entry[i]->SetPos(235, 131+24*i);
		entry[i]->SetHJustify(Font::HORIZJUST_RIGHT);
		entry[i]->SetFont(&manager->font_bold);
	}

	for(int i=0; i<lsd_num; i++)
	{
		if(i == lsd_num-1) entry[i]->down = entry[0];
		else entry[i]->down = entry[i+1];
		if(i == 0) entry[i]->up = entry[lsd_num-1];
		else entry[i]->up = entry[i-1];
	}

	setCfg = false;
	NameMenu = NEW NamesMenu(s, man, p, pq);
	AddSubmenu(NameMenu);
	KeyMenu = NEW KeyboardMenu(s, man, p, pq);
	AddSubmenu(KeyMenu);
	dialogMenu = NEW DialogMenu(s, man, p, pq);
	AddSubmenu(dialogMenu);
	sftd = NEW MultiLineString(&frontendmanager.font_bold, ksGlobalTextArray[GT_SL_SFTD], 235, 220, 0, .666, Font::HORIZJUST_RIGHT,Font::VERTJUST_CENTER,frontendmanager.blue);
	sftd->setLineSpacing(frontendmanager.font_bold.getGlyph('A')->cell_height/2);
	
	overwrite = true;
	error_thrown = false;
	highlighted = NULL;
	forward_menu = -1;
	forward_sub_menu = -1;
	back_menu = -1;
	back_sub_menu = -1;
}

SaveLoadFrontEnd::~SaveLoadFrontEnd()
{
	delete NameMenu;
	delete KeyMenu;
	delete dialogMenu;
	delete sftd;
#ifdef TARGET_PS2
	GenericGameSaver::inst()->releaseIconData();
#endif
}

// active is the actual card num (is -1 for xbox harddrive)
// adjusted is index into array (never negative)
void SaveLoadFrontEnd::getFileListing(int active_card, int adjusted_card)
{
	numGames = GenericGameSaver::inst()->getFileListing((active_card!=-1)?active_card/NUM_MEMORY_SLOTS:-1, (active_card!=-1)?active_card%NUM_MEMORY_SLOTS:0, SavedData[adjusted_card], NULL, NULL);
	
	// Now we filter out into two types of data
	fileType = 0;
	numGameData[adjusted_card] = 0;
	numGameConfig[adjusted_card] = 0;
#ifdef BETH
	int invalid_games = 0;
	int invalid_configs = 0;
#endif
	for (int i=0; i < numGames; i++)
	{
		if(SavedData[adjusted_card][i].valid == GSOk)
		{
			if (SavedData[adjusted_card][i].type == 1)
			{
					numGameData[adjusted_card]++;
			}
			else if (SavedData[adjusted_card][i].type == 0)
			{
				if(SavedData[adjusted_card][i].valid == GSOk)
					numGameConfig[adjusted_card]++;
			}
		}
		else
		{
#ifdef BETH
			invalid_games++;
#endif
		}	
		
	}

#ifdef BETH
	if(invalid_games != 0)
		nglPrintf("WARNING: number of invalid games on this card is %d\n", invalid_games);
	if(invalid_configs != 0)
		nglPrintf("WARNING: number of invalid configs on this card is %d\n", invalid_configs);
#endif

	int count1=0, count2=0;

	for (int i=0; i < numGames; i++)
	{
		if (SavedData[adjusted_card][i].type == 1 && count1 < num_each && SavedData[adjusted_card][i].valid == GSOk)
		{
			savedGames[adjusted_card][count1] = SavedData[adjusted_card][i];
			count1++;
		}
		else if (SavedData[adjusted_card][i].type == 0 && count2 < num_each && SavedData[adjusted_card][i].valid == GSOk)
		{
			savedConfigs[adjusted_card][count2] = SavedData[adjusted_card][i];
			count2++;
		}
	}
}

bool SaveLoadFrontEnd::ExistsAndHasSpace(int card)
{
	int n = GenericGameSaver::inst()->getFileListing((card!=-1)?card/NUM_MEMORY_SLOTS:-1, (card!=-1)?card%NUM_MEMORY_SLOTS:0, SavedData[card], NULL, NULL);
	return (n >= 0 && n < num_each);
}

void SaveLoadFrontEnd::Init()
{
	NameMenu->Init();
	KeyMenu->Init();
	dialogMenu->Init();
	FEMultiMenu::Init();
}


void SaveLoadFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();
	SetDState(DSTATE_LSD);

#ifdef TARGET_PS2
	GenericGameSaver::inst()->getIconData();
#endif
}

void SaveLoadFrontEnd::Update(time_value_t time_inc)
{
	if(active) active->Update(time_inc);
	if(active == dialogMenu && 
		(dstate == DSTATE_SAVE || dstate == DSTATE_LOAD || dstate == DSTATE_DELETE || 
		 dstate == DSTATE_SAVE_PICK || dstate == DSTATE_LOAD_PICK || dstate == DSTATE_DELETE_PICK))
		NameMenu->Update(time_inc);

	// only begin the lengthy process if "deleting..." or "formatting..."
	//  has already been drawn a few times
	if(dialogMenu->DrawnEnough())
	{
		if(dstate == DSTATE_DELETING)
			DelFile(NameMenu->ActiveCard(), NameMenu->ActiveFile());
		if(dstate == DSTATE_FORMATTING) NameMenu->Format();
	}
	if(!active) FEMultiMenu::Update(time_inc);
	if (dstate == DSTATE_SAVE_GLOBAL)
	{
		SetDState(DSTATE_SAVING_GLOBAL);
		GenericGameSaver::inst()->saveSystemFile(savePort, saveSlot, &globalCareerData, SaveLoadFrontEnd::SetGlobalSaveProgess, this);
		
	}
	else if (dstate == DSTATE_SAVE_PHOTO)
	{
		//int idx = ((PhotoFrontEnd *) system->menus[PauseMenuSystem::PhotoMenu])->GetSelectedPhotoIdx();
		SetDState(DSTATE_SAVING_PHOTO);

		// Moved to a different spot
		//g_career->GetPhotoForLevel (g_game_ptr->get_level_id())->CopyFromTexture(g_beach_ptr->get_challenges()->photo->GetPhotoTexture(idx));

		// If this were async, we'd do this elsewhere
		SetDState(DSTATE_SAVE_DONE);

	}
	else if (dstate == DSTATE_LOAD_GLOBAL)
	{
		SetDState(DSTATE_LOADING_GLOBAL);
		GenericGameSaver::inst()->readSystemFile(savePort, saveSlot, &globalCareerData,
			(SaveLoadFrontEnd::SetLoadProgressGlobal), (void *)this);

	}
	else if (dstate == DSTATE_LOAD_DONE)
	{
		if (!setCfg)
		{
			StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
			setCfg = true;
		}
	}
#ifdef TARGET_PS2
	RotateThreadReadyQueue(1);
#endif
}

void SaveLoadFrontEnd::Draw()
{
	// set progress bar
	if(dstate == DSTATE_SAVING || dstate == DSTATE_LOADING || dstate == DSTATE_SAVING_GLOBAL)
		dialogMenu->SetProgress(progressval);

	panel.Draw(0);

	if(active) active->Draw();
	if ((dstate == DSTATE_DELETE) ||
      (dstate == DSTATE_DELETE_PICK) ||
	    (dstate == DSTATE_DELETING) ||
	    (dstate == DSTATE_DEL_DONE))
	sftd->Draw();

	if(active == dialogMenu && dstate != DSTATE_LSD && dstate != DSTATE_SAVE_NAME)
		NameMenu->DrawHeader();

	// always draw this
	FEMenuEntry* tmp = entries;
	while(tmp)
	{
		tmp->Draw();
		tmp = tmp->next;
	}
}

saveInfo loading;
void SaveLoadFrontEnd::SetLoadProgressGame(void *userData, int val)
{
	if(val >= 0)
		((SaveLoadFrontEnd *)userData)->progressval = val/200.0f;
	if(val >= 100)
	{
		currentGame = loading;
		currentGame.valid = GSOk;
		((SaveLoadFrontEnd *)userData)->setCfg = false;
		((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_LOAD_GLOBAL);
		((SaveLoadFrontEnd *)userData)->progressval = 50;
	}
	if(val < 0)
  		((SaveLoadFrontEnd *)userData)->StartError(SE_LOAD_ERROR);
}

void SaveLoadFrontEnd::SetLoadProgressGlobal(void *userData, int val)
{
	if(val >= 0)
		((SaveLoadFrontEnd *)userData)->progressval = val/200.0f + 50;
	if(val >= 100)
	{
		((SaveLoadFrontEnd *)userData)->setCfg = false;
		((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_LOAD_DONE);
		((SaveLoadFrontEnd *)userData)->progressval = 0;
	}
	if(val < 0)
  		((SaveLoadFrontEnd *)userData)->StartError(SE_LOAD_ERROR);
}

void SaveLoadFrontEnd::SetGlobalSaveProgess(void *userData, int val)
{
	if(val >= 0)
		((SaveLoadFrontEnd *)userData)->progressval = (50.0f + val/2.0f)/100.0f;
	if(val >= 100)
	{
		if (((SaveLoadFrontEnd *)userData)->GetInFrontEnd())
			((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_SAVE_DONE);
		else
		{
			((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_SAVE_DONE);
		}
		((SaveLoadFrontEnd *)userData)->progressval = 0;
	}
	if(val < 0)
	{
		((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_SAVE_DONE);
	}
}
void SaveLoadFrontEnd::SetSaveProgress(void *userData, int val)
{
	// This is the first half
	if(val >= 0)
		((SaveLoadFrontEnd *)userData)->progressval = val/100.0f/2;
	if(val >= 100)
	{
		//
		((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_SAVE_GLOBAL);
		//((SaveLoadFrontEnd *)userData)->progressval = 0;
	}
	if(val < 0)
	{
		((SaveLoadFrontEnd *)userData)->StartError(SE_SAVE_ERROR);
	}
}


void SaveLoadFrontEnd::Select()
{
	int tmp = highlighted->entry_num;
	if (!in_frontend)
		SoundScriptManager::inst()->unpause();
	
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	if (!in_frontend)
			SoundScriptManager::inst()->pause();
	SetDState(tmp==SL_LOAD ? DSTATE_LOAD : (tmp==SL_SAVE ? DSTATE_SAVE : DSTATE_DELETE));
}

void SaveLoadFrontEnd::OnActivate()
{
	if(in_frontend) manager->em->StopCameraRoll();

	forward_menu = -1;
	forward_sub_menu = -1;
	back_menu = -1;
	back_sub_menu = -1;
	FEMenuEntry* tmp = highlighted;
	SetDState(DSTATE_LSD, false);
	for(int i=0; i<lsd_num; i++)
		entry[i]->SetSpecialColor(frontendmanager.col_unselected, frontendmanager.col_highlight);
	FEMultiMenu::OnActivate();
	if(tmp) setHigh(tmp);
	NameMenu->ResetCardFormat();

	if(in_frontend)
	{
		manager->helpbar->Reset();
		manager->helpbar->AddArrowV();
		manager->helpbar->Reformat();
	}
}

void SaveLoadFrontEnd::OnUnactivate(FEMenu* m)
{
	color32 c = frontendmanager.col_info_b;

	if (!highlighted)
	{
		for(int i=0; i<lsd_num; i++)
			entry[i]->SetSpecialColor(c, c);
		return;
	}
	switch(highlighted->entry_num)
	{
	case SL_LOAD:
		c.c.a = 127;
		entry[SL_SAVE]->SetSpecialColor(c,c);
		
		c.c.a = 255;
		entry[SL_LOAD]->SetSpecialColor(c,c);
		
		c.c.a = 127;
		entry[SL_DELETE]->SetSpecialColor(c,c);
		break;
		
	case SL_SAVE:
		c.c.a = 127;
		entry[SL_LOAD]->SetSpecialColor(c,c);
		
		c.c.a = 255;
		entry[SL_SAVE]->SetSpecialColor(c,c);
		
		c.c.a = 127;
		entry[SL_DELETE]->SetSpecialColor(c,c);
		break;
		
	case SL_DELETE:
		c.c.a = 127;
		entry[SL_SAVE]->SetSpecialColor(c,c);
		
		c.c.a = 255;
		entry[SL_DELETE]->SetSpecialColor(c,c);
		
		c.c.a = 127;
		entry[SL_LOAD]->SetSpecialColor(c,c);
		break;
	}
}

void SaveLoadFrontEnd::OnActivate(int submenu)
{
	int i=0, j=0;
	bool found =false;
	forward_menu = -1;
	forward_sub_menu = -1;
	back_menu = -1;
	back_sub_menu = -1;
	
	switch(submenu)
	{
	case ACT_SAVE:
		


		setHigh(entry[SL_SAVE]);
		SetDState(DSTATE_SAVE);
		forward_menu = PauseMenuSystem::SaveLoadMenu;
		forward_sub_menu = ACT_SAVE_NAME;
		if (currentGame.valid == GSOk)
		{
			while( i < NUMCARDS  && !found )
			{
				NameMenu->CheckCardNum(NameMenu->FindActive(i));
				NameMenu->GetFileList(NameMenu->FindActive(i), i);
				j=0;
				while ( j < numGameConfig[i] && !found )
				{
					if (stricmp(savedConfigs[i][j].shortname, currentGame.shortname) == 0)
					{
						found = true;
					}
					j++;
				}
				i++;
			}
		}

		if (found)
		{
			// Fix these
			i--; j--;
			NameMenu->adjusted_active_card = i;
			NameMenu->active_card = NamesMenu::FindActive(i);

			NameMenu->setHigh(NameMenu->entry[NamesMenu::num + i]);
			NameMenu->SetSecondaryCursor(NameMenu->entry[j]);
		}

		break;
	case ACT_LOAD:
		setHigh(entry[SL_LOAD]);
		SetDState(DSTATE_LOAD);
		back_menu = GraphicalMenuSystem::SurferMenu;
		back_sub_menu = SurferFrontEnd::ACT_CAREER;
		forward_menu = GraphicalMenuSystem::SurferMenu;
		break;
	case ACT_DELETE:
		setHigh(entry[SL_DELETE]);
		SetDState(DSTATE_DELETE);
		break;
	case ACT_SAVE_NAME:
//		NameMenu->FindAvailableSlot();
		setHigh(entry[SL_SAVE]);
		SetDState(DSTATE_SAVE_NAME);
		back_menu = PauseMenuSystem::SaveLoadMenu;
		back_sub_menu = ACT_SAVE;
		break;
	default: OnActivate(); break;
	}
}

void SaveLoadFrontEnd::OnLeft(int c)
{
//	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if(active) active->OnLeft(c);
	else 
	{
		FEMultiMenu::OnLeft(c);
		if (!in_frontend)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		if (!in_frontend)
			SoundScriptManager::inst()->pause();
	}
}

void SaveLoadFrontEnd::OnRight(int c)
{
//	
	if(active) active->OnRight(c);
	else
	{
		FEMultiMenu::OnRight(c);
		if (!in_frontend)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		if (!in_frontend)
			SoundScriptManager::inst()->pause();
	}
}

void SaveLoadFrontEnd::OnDown(int c)
{
	if(active) active->OnDown(c);
	else
	{
		FEMultiMenu::OnDown(c);
		if (!in_frontend)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		if (!in_frontend)
			SoundScriptManager::inst()->pause();
	}
}

void SaveLoadFrontEnd::OnUp(int c)
{
	if(active) active->OnUp(c);
	else
	{
		FEMultiMenu::OnUp(c);
		if (!in_frontend)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		if (!in_frontend)
			SoundScriptManager::inst()->pause();

	}
}

void SaveLoadFrontEnd::OnTriangle(int c)
{
	// If we're in the front end, then triangle backs up to the previous menu.
	if (in_frontend)
	{
		if (back_menu == -1)
		{
			if (active) // If we're in a sub-menu
			{
				active->OnTriangle(c);
				if(active == dialogMenu && error_thrown)
				{
					if(current_error == SE_NO_MEMORY_CARDS)
						SetDState(DSTATE_LSD);
				}
			}
			else // we're in the main save/load menu
			{
				if (!in_frontend)
					SoundScriptManager::inst()->unpause();
				SoundScriptManager::inst()->playEvent(SS_FE_BACK);
				if (!in_frontend)
					SoundScriptManager::inst()->pause();

				if (dstate == DSTATE_LSD)
				{
					highlighted->Highlight(false);
					highlighted = NULL;
					// Unmount all the memory units
					GenericGameSaver::inst()->ReleaseAll();
					if (in_frontend)
					{
						manager->em->ExitState(); 
						sys->MakeActive(GraphicalMenuSystem::MainMenu,MainFrontEnd::MainCareerEntry);
					}
					else system->MakeActive(PauseMenuSystem::MapMenu);
				}
				else assert(0);	// if the state is something else, then there's something really wrong
			}
		}
		else if (back_sub_menu == -1) system->MakeActive(back_menu);
		else system->MakeActive(back_menu, back_sub_menu);
	}
	// If we're in game, then triangle makes the menu go away.
	else
	{
		if (active && ((active == dialogMenu) || (active == KeyMenu)))
		{
			if (active == KeyMenu)
			{
				SetDState(DSTATE_SAVE);
			}
			else
				active->OnTriangle(c);
			if(active == dialogMenu && error_thrown)
			{
				if(current_error == SE_NO_MEMORY_CARDS)
					SetDState(DSTATE_SAVE);
			}
		}
		else
		{
			// Unmount all the memory units
			GenericGameSaver::inst()->ReleaseAll();
			// If no back menu specified, then deactivate.
			if (back_menu == -1)
				system->endDraw(false);
			// Otherwise, popup back menu.
			else
			{
				if (back_sub_menu == -1)
					system->MakeActive(back_menu);
				else
					system->MakeActive(back_sub_menu);
			}	
		}
	}
}

void SaveLoadFrontEnd::OnCross(int c)
{
	
	if(active) 
		active->OnCross(c);
	else 
	{
#ifdef TARGET_XBOX
		// Mount all present the memory units
		GenericGameSaver::inst()->init();
#endif
		Select();
	}
}

void SaveLoadFrontEnd::StartError(int id)
{
	stringx message = "";
	char temp[100];
	int port = NameMenu->ActiveCard()==-1?-1:NameMenu->ActiveCard()/NUM_MEMORY_SLOTS;
	int slot = NameMenu->ActiveCard()==-1?-1:NameMenu->ActiveCard()%NUM_MEMORY_SLOTS;
	switch(id)
	{
	case SE_NOT_ENOUGH_SPACE:
		next_state = DSTATE_SAVE;
		message = GenericGameSaver::inst()->getNotEnoughRoomString(port, slot);
		break;
	case SE_NO_MEMORY_CARDS:
		if(dstate == DSTATE_LSD)
			next_state = (cur_type == SL_SAVE ? DSTATE_SAVE : (cur_type == SL_LOAD ? DSTATE_LOAD : DSTATE_DELETE));
		else next_state = dstate;
#ifdef TARGET_GC
		message = GenericGameSaver::inst()->getErrorString( port, slot, GSErrorNoMedia );
#else
		message = GenericGameSaver::inst()->getInsertCardString(port, slot);
#endif //TARGET_GC
		break;
	case SE_UNFORMAT:
		next_state = DSTATE_SAVE;
		message = stringx(stringx::fmt, ksGlobalTextArray[GT_FE_MENU_ASK_FORMAT].c_str(), SaveLoadFrontEnd::MakeStringMemCard(NameMenu->ActiveCard()).c_str());
		break;
	case SE_SAVE_ERROR:
		next_state = DSTATE_SAVE;
		sprintf(temp, ksGlobalTextArray[GT_FE_MENU_SAVE_FAIL].c_str(), GenericGameSaver::inst()->getCardString(port, slot).c_str());
		message = temp;
		break;
	case SE_LOAD_ERROR:
		g_career->init();
		careerStarted = false;
		globalCareerData.init();
		next_state = DSTATE_LOAD;
#ifdef TARGET_XBOX
		message = ksGlobalTextArray[GT_FE_MENU_LOAD_FAIL_XB];
#else
		sprintf(temp, ksGlobalTextArray[GT_FE_MENU_LOAD_FAIL].c_str(), GenericGameSaver::inst()->getCardString(port, slot).c_str());
		message = temp;
#endif
		break;
	case SE_FORM_ERROR:
		next_state = DSTATE_LSD;
		sprintf(temp, ksGlobalTextArray[GT_FE_MENU_FORM_FAIL].c_str(), GenericGameSaver::inst()->getCardString(port, slot).c_str());
		message = temp;
		break;
	case SE_DELETE_ERROR:
		next_state = DSTATE_DELETE;
		sprintf(temp, ksGlobalTextArray[GT_FE_MENU_DEL_FAIL].c_str(), GenericGameSaver::inst()->getCardString(port, slot).c_str());
		message = temp;
		break;
	case SE_NO_FILENAME:
		next_state = DSTATE_SAVE_NAME;
		message = ksGlobalTextArray[GT_FE_MENU_NO_FILENAME];
		break;
	case SE_UNIQUE_FILENAME:
		next_state = DSTATE_SAVE_NAME;
		message = ksGlobalTextArray[GT_FE_MENU_NOT_UNIQUE];
		break;
#ifdef TARGET_GC
	case SE_GC_CORRUPT:
		next_state = dstate;
		message = GenericGameSaver::inst()->getErrorString( port, slot, GSErrorUnformatted );
		break;
	case SE_GC_REGION:
		next_state = dstate;
		message = GenericGameSaver::inst()->getErrorString( port, slot, GSErrorWrongRegion );
		break;
#endif //TARGET_GC
	default: assert(0);
	}

	int type = DialogMenu::DM_TYPE_OK;
	if(id == SE_NO_MEMORY_CARDS)
		type = DialogMenu::DM_TYPE_EMPTY;
	else if( id == SE_UNFORMAT )
		type = DialogMenu::DM_TYPE_YES;
#ifdef TARGET_GC
	else if( id == SE_GC_CORRUPT || id == SE_GC_REGION )
		type = DialogMenu::DM_TYPE_FMT;
	else if( id == SE_NOT_ENOUGH_SPACE )
		type = DialogMenu::DM_TYPE_MNG;
#endif //TARGET_GC
	dialogMenu->SetTypeAndMessage(type, message);
	MakeActive(dialogMenu);
	NameMenu->TurnPQLines(false);
	current_error = id;
	error_thrown = true;
}

void SaveLoadFrontEnd::EndError(int id)
{
	if(error_thrown)
	{
		error_thrown = false;
		if(current_error != id) return;
		if(id == SE_UNFORMAT)
		{
			if (NameMenu->cards[NameMenu->FindAdjusted(NameMenu->ActiveCard())].exists)
			{
				post_format_state = DSTATE_SAVING;
				SetDState(DSTATE_FORMATTING);
			}
			else
			{
				SetDState(DSTATE_SAVE);
				NameMenu->ResetCardFormat();
			}
		}
		else
			SetDState(next_state, true, true);
	}
}

int SaveLoadFrontEnd::getActiveCard()
{
	return NameMenu->ActiveCard();
}

void SaveLoadFrontEnd::SetPQIndices()
{
	NameMenu->SetPQIndices();
	dialogMenu->SetPQIndices();
	KeyMenu->SetPQIndices();
}

void SaveLoadFrontEnd::TurnOffAll()
{
	dialogMenu->TurnPQ(false);
	NameMenu->TurnPQ(false);
	KeyMenu->TurnPQ(false);
}

void SaveLoadFrontEnd::SetDState(int s, bool activate, bool end_error)
{
	int last_state = dstate;

	dstate = s;
	int savePort = NameMenu->ActiveCard()>=0?NameMenu->ActiveCard()/NUM_MEMORY_SLOTS:-1;
	int saveSlot = NameMenu->ActiveCard()>=0?NameMenu->ActiveCard()%NUM_MEMORY_SLOTS:-1;

	switch(s)
	{
	case DSTATE_LSD:
		TurnOffAll();
		if(activate) active = NULL;
		break;
	case DSTATE_LOAD:		cur_type = SL_LOAD; break;
	case DSTATE_LOAD_PICK:
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_YES, ksGlobalTextArray[GT_FE_MENU_ASK_LOAD]);
		break;
	case DSTATE_LOADING:
#ifndef TARGET_XBOX	// Xbox loads are fast enough to go directly to "Done".  (dc 07/07/02)
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_EMPTY, GenericGameSaver::inst()->getLoadingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA]));
#endif
		LoadFile(NameMenu->ActiveCard(), NameMenu->ActiveFile());
		break;
	case DSTATE_LOAD_DONE:
		globalCareerData.updateFromCareer(g_career);
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_OK, ksGlobalTextArray[GT_FE_MENU_LOAD_DONE]);
		break;
	case DSTATE_SAVE:		cur_type = SL_SAVE; break;
	case DSTATE_SAVE_PICK:
		dialogMenu->SetProgress(0);
		progressval = 0;
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_YES, GenericGameSaver::inst()->getOverwriteString(savePort, saveSlot));
		break;
	case DSTATE_SAVING:
		if (!NameMenu->cards[NameMenu->FindAdjusted(NameMenu->ActiveCard())].available)
		{
			StartError(SE_UNFORMAT);
		}
		else
		{
#ifndef TARGET_XBOX	// Xbox saves are fast enough to go directly to "Done".  (dc 07/07/02)
			dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_PROGRESS, GenericGameSaver::inst()->getSavingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA]));
#endif
			SaveFile(NameMenu->ActiveCard(), overwrite, NameMenu->ActiveFile());
		}
		break;
	case DSTATE_SAVE_DONE:
		NameMenu->cards[NamesMenu::FindAdjusted(NameMenu->ActiveCard())].changed = true;
		NameMenu->RefreshDisplay();
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_OK, ksGlobalTextArray[GT_FE_MENU_SAVE_DONE]);
		break;
	case DSTATE_DELETE:		cur_type = SL_DELETE; break;
	case DSTATE_DELETE_PICK:
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_YES, ksGlobalTextArray[GT_FE_MENU_ASK_DEL]);
		break;
	case DSTATE_DELETING:
#ifndef TARGET_XBOX	// Xbox deletes are fast enough to go directly to "Done".  (dc 07/07/02)
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_EMPTY,GenericGameSaver::inst()->getDeletingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA]));
#endif
		break;
	case DSTATE_DEL_DONE:
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_OK, ksGlobalTextArray[GT_FE_MENU_DEL_DONE]);
		break;
	case DSTATE_FORMATTING:
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_EMPTY, GenericGameSaver::inst()->getFormattingString(savePort, saveSlot));
		break;
	case DSTATE_FORM_DONE:
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_OK, ksGlobalTextArray[GT_FE_MENU_FORM_DONE]);
		break;
	case DSTATE_SAVING_GLOBAL:
	case DSTATE_SAVE_GLOBAL:
#ifndef TARGET_XBOX	// Xbox saves are fast enough to go directly to "Done".  (dc 07/07/02)
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_PROGRESS, GenericGameSaver::inst()->getSavingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA]));
#endif
		break;
	case DSTATE_SAVE_PHOTO:
	case DSTATE_SAVING_PHOTO:
#ifndef TARGET_XBOX	// Xbox saves are fast enough to go directly to "Done".  (dc 07/07/02)
		dialogMenu->SetTypeAndMessage(DialogMenu::DM_TYPE_PROGRESS, ksGlobalTextArray[GT_FE_MENU_SAVING]);
#endif
		break;
	}

	if(s == DSTATE_LOAD || s == DSTATE_SAVE || s == DSTATE_DELETE)
	{
		MakeActive(NameMenu);
		if( end_error || last_state == DSTATE_LSD )
			NameMenu->OnCardSwitch();
	}
	else if(s == DSTATE_SAVE_NAME)
		MakeActive(KeyMenu);
	else if(s != DSTATE_LSD)
	{
		NameMenu->TurnPQLines(false);
		MakeActive(dialogMenu);
	}

}

void SaveLoadFrontEnd::DialogYesOKPressed()
{
	if(error_thrown)
	{
#ifdef TARGET_GC
		if( current_error == SE_GC_CORRUPT || current_error == SE_GC_REGION || current_error == SE_NOT_ENOUGH_SPACE )
		{
			if( current_error == SE_NOT_ENOUGH_SPACE )
			{
				extern void KSGCReset( bool );
				KSGCReset( true );
			}
			post_format_state = next_state;
			EndError( current_error );
			SetDState( DSTATE_FORMATTING );
		}
		else
#endif //TARGET_GC
			EndError( current_error );
	}
	else
	{
		switch(dstate)
		{
			case DSTATE_LOAD_PICK:		SetDState(DSTATE_LOADING); break;
			case DSTATE_LOAD_DONE:
				if (forward_menu == -1)
					SetDState(DSTATE_LOAD);
				else if (forward_sub_menu == -1) system->MakeActive(forward_menu);
				else system->MakeActive(forward_menu, forward_sub_menu);
				break;
			case DSTATE_SAVE_PICK:		
				//forward_menu=back_menu; 
				SetOverwrite(); 
				SetDState(DSTATE_SAVING); 
				break;
			case DSTATE_SAVE_DONE:
				if (in_frontend)
				{
					if (forward_menu == -1)
						SetDState(DSTATE_SAVE);
					else if (forward_sub_menu == -1) system->MakeActive(forward_menu);
					else system->MakeActive(forward_menu, forward_sub_menu);
				}
				else
				{
					if (forward_menu == -1)
						system->endDraw(false);
					else
					{
						if (forward_sub_menu == -1)
							system->MakeActive(forward_menu);
						else
							system->MakeActive(forward_menu, forward_sub_menu);
					}
				}
				break;
			case DSTATE_DELETE_PICK:	SetDState(DSTATE_DELETING); break;
			case DSTATE_DEL_DONE:		SetDState(DSTATE_DELETE); break;
			//case DSTATE_FORM_DONE:		SetDState(DSTATE_SAVING); break;
		}
	}
}

void SaveLoadFrontEnd::DialogNoPressed()
{
	if(error_thrown)
	{
		if(current_error == SE_UNFORMAT)
		{
			error_thrown = false;
			SetDState(next_state, true);
		}
#ifdef TARGET_GC
		if( current_error == SE_GC_CORRUPT || current_error == SE_GC_REGION || current_error == SE_NOT_ENOUGH_SPACE )
		{
			error_thrown = false;
			SetDState( next_state, true );
		}
#endif //TARGET_GC
	}
	else
	{
		switch(dstate)
		{
			case DSTATE_LOAD_PICK:		SetDState(DSTATE_LOAD); break;
			case DSTATE_SAVE_PICK:		SetDState(DSTATE_SAVE); break;
			case DSTATE_DELETE_PICK:	SetDState(DSTATE_DELETE); break;
		}
	}
}

void SaveLoadFrontEnd::CancelDialog()
{
	if(!DialogActive()) return;
	// approximately the same, for now
	DialogNoPressed();
}

bool SaveLoadFrontEnd::DialogActive()
{
	return (active == dialogMenu);
}

bool SaveLoadFrontEnd::SavedGamesExist()
{
	for(int i=LOWEST_CARD; i<HIGHEST_CARD; i++)
	{
		int numGames = -1;
		int adjusted = NamesMenu::FindAdjusted(i);

		numGames = GenericGameSaver::inst()->getFileListing((i!=-1)?i/NUM_MEMORY_SLOTS:-1, (i!=-1)?i%NUM_MEMORY_SLOTS:0, SavedData[adjusted], NULL, NULL);


		if(numGames > 0) return true;
	}
	return false;
}

void SaveLoadFrontEnd::SetOverwrite(const char* filename)
{
	overwrite = (filename == NULL);
	if(!overwrite) strcpy(desc, filename);
}

void SaveLoadFrontEnd::LoadFile(int card, int filenum)
{
	assert((fileType == 0) && (numGameConfig[NameMenu->FindAdjusted(card)] > 0));
	
	GenericGameSaver::inst()->setFileInfo(savedConfigs[NameMenu->FindAdjusted(card)][filenum]);
	loading = savedConfigs[NameMenu->FindAdjusted(card)][filenum];
	// unsaved_career data flag set to false as loading a previously saved career
	frontendmanager.unsaved_career = false;
	savePort = (card!=-1)?card/NUM_MEMORY_SLOTS:-1;
	saveSlot = (card!=-1)?card%NUM_MEMORY_SLOTS:0;
	GenericGameSaver::inst()->readFile(savePort, saveSlot , (void*)g_career,
		sizeof(*g_career),
		(SaveLoadFrontEnd::SetLoadProgressGame), (void *)this);
}

void SaveLoadFrontEnd::SaveFile(int card, bool overwrite, int slot)
{
    // Saving over the top
    long int t;
    char longtime[20];
    saveInfo fileDef;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
#ifndef TARGET_GC
    STUB("SaveLoadFrontEnd::Select()");
#endif
	
	// Create a time stamp
	t = time(NULL);
#else
	sceCdCLOCK cloc;
	
	// Create a time stamp
	sceCdReadClock(&cloc);
	t= cloc.second + cloc.minute*60 + cloc.hour*3600 + cloc.day*3600*24 + cloc.month*31*24*3600;
#endif /* TARGET_XBOX JIV DEBUG */
	
	if(overwrite && slot != -1)
	{
		fileDef = savedConfigs[NameMenu->FindAdjusted(card)][slot];
	}
	else
	{
		// Use the timestamp's last 7 digits as a filename
		sprintf(longtime, "%ld", t);
		if (strlen(longtime) > 7)
			strcpy(fileDef.shortname, longtime + strlen(longtime) - 7);
		else
			strcpy(fileDef.shortname, longtime);
		
		strcpy(fileDef.desc, desc);
		//    fileDef.type = fileType;
		fileDef.type = 0;
	}

	// career is saved so no unsaved data left
	frontendmanager.unsaved_career = false;

	currentGame = fileDef;
	currentGame.valid= GSOk;
	savePort = card!=-1?card/NUM_MEMORY_SLOTS:-1;
	saveSlot = card!=-1?card%NUM_MEMORY_SLOTS:0;
	GenericGameSaver::inst()->setFileInfo(fileDef);
	loading = fileDef;
    g_career->cfg = *(ksConfigData*)(StoredConfigData::inst()->getGameConfig());
	
    if (fileType != 1)
    {
			NameMenu->cards[card].changed = true;
			GenericGameSaver::inst()->saveFile((card!=-1?card/NUM_MEMORY_SLOTS:-1), (card!=-1?card%NUM_MEMORY_SLOTS:0) ,(void *)g_career,
			   sizeof(*g_career), true,
			   (SaveLoadFrontEnd::SetSaveProgress), (void *)this);
			
		
    }
//	SetDState(DSTATE_SAVING);
}

void SaveLoadFrontEnd::DelFile(int card, int filenum)
{
	/*
	if (currentGame.valid == GSOk)
	{
		if (stringx(currentGame.shortname) == stringx(SavedData[card][filenum].shortname) &&
				stringx(currentGame.desc) == stringx(SavedData[card][filenum].desc) &&
				currentGame.timestamp == SavedData[card][filenum].timestamp)
		{
			currentGame.valid = GSErrorOther;
		}
	}
	int ret = GenericGameSaver::inst()->deleteFile(card, card, SavedData[card][filenum]);
	*/

	// SavedData was changed to savedConfigs because otherwise filenum is out of sync
	if (currentGame.valid == GSOk)
	{
		if (stringx(currentGame.shortname) == stringx(savedConfigs[NameMenu->FindAdjusted(card)][filenum].shortname) &&
				stringx(currentGame.desc) == stringx(savedConfigs[NameMenu->FindAdjusted(card)][filenum].desc) &&
				currentGame.timestamp == savedConfigs[NameMenu->FindAdjusted(card)][filenum].timestamp)
		{
			currentGame.valid = GSErrorOther;
		}
	}
	int ret = GenericGameSaver::inst()->deleteFile((card!=-1?card/NUM_MEMORY_SLOTS:-1), (card!=-1?card%NUM_MEMORY_SLOTS:0), savedConfigs[NameMenu->FindAdjusted(card)][filenum]);
	NameMenu->cards[card].changed = true;
	if(ret != 0)
	{
		StartError( SE_DELETE_ERROR );
	}
	else
	{
		NameMenu->RefreshDisplay();
		SetDState(DSTATE_DEL_DONE);
	}
}

stringx SaveLoadFrontEnd::MakeStringCardInSlot(int active_card)
{
	stringx tmp = "";
	/*
#ifdef TARGET_PS2
	tmp = ksGlobalTextArray[GT_FE_MENU_PS2_MEM_CARD]+" "+ksGlobalTextArray[GT_FE_MENU_PS2_MEM_IN_SLOT]+" "+stringx(active_card+1);
#elif defined(TARGET_XBOX)
	if(active_card == -1)
		tmp = ksGlobalTextArray[GT_FE_MENU_XBOX_HARD_DRIVE];
	else
		tmp = ksGlobalTextArray[GT_FE_MENU_XBOX_MEM_CARD_1]+" "+stringx(active_card/NUM_MEMORY_SLOTS+1)+ksGlobalTextArray[GT_FE_MENU_XBOX_MEM_CARD_2]+" "+
			stringx(active_card%NUM_MEMORY_SLOTS+1)+" "+ksGlobalTextArray[GT_FE_MENU_MEMORY_CARD];
#else
	tmp = ksGlobalTextArray[GT_FE_MENU_MEMORY_CARD]+" "+stringx(active_card+1);
#endif*/
	GenericGameSaver::inst()->getCardString(active_card>=0?active_card/NUM_MEMORY_SLOTS:-1, active_card>=0?active_card%NUM_MEMORY_SLOTS:0);
	NamesMenu::ReplaceBadCharacters(tmp);
	return tmp;
}

stringx SaveLoadFrontEnd::MakeStringSlot(int active_card)
{
	stringx tmp = "";
/*#ifdef TARGET_PS2
	tmp = ksGlobalTextArray[GT_FE_MENU_PS2_MEM_SLOT]+" "+stringx(active_card+1);
#elif defined(TARGET_XBOX)
	if(active_card == -1)
		tmp = ksGlobalTextArray[GT_FE_MENU_XBOX_HARD_DRIVE];
	else
		tmp = ksGlobalTextArray[GT_FE_MENU_XBOX_MEM_CARD_1]+" "+stringx(active_card/NUM_MEMORY_SLOTS+1)+ksGlobalTextArray[GT_FE_MENU_XBOX_MEM_CARD_2]+" "+
			stringx(active_card%NUM_MEMORY_SLOTS+1);
#else
	tmp = ksGlobalTextArray[GT_FE_MENU_MEMORY_CARD]+" "+stringx(active_card+1);
#endif*/
	tmp = GenericGameSaver::inst()->getShortCardString(active_card==-1?-1:active_card/NUM_MEMORY_SLOTS, active_card==-1?-1:active_card%NUM_MEMORY_SLOTS);
	NamesMenu::ReplaceBadCharacters(tmp);
	return tmp;
}

stringx SaveLoadFrontEnd::MakeStringMemCard(int active_card)
{
	stringx tmp = "";
/*#ifdef TARGET_PS2
	tmp = ksGlobalTextArray[GT_FE_MENU_PS2_MEM_CARD];
#else
	tmp = ksGlobalTextArray[GT_FE_MENU_MEMORY_CARD];
#endif*/
	tmp = GenericGameSaver::inst()->getCardString(active_card==-1?-1:active_card/NUM_MEMORY_SLOTS, active_card==-1?0:active_card%NUM_MEMORY_SLOTS);
	NamesMenu::ReplaceBadCharacters(tmp);
	return tmp;
}

/*********************** Dialog Menu ***********************/

DialogMenu::DialogMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf)
{
	cons(s, man, p, pf);
	color = frontendmanager.col_unselected;
	color_high = frontendmanager.col_highlight;
	color_high_alt = frontendmanager.col_highlight2;
	flags |= FEMENU_HAS_COLOR_HIGH_ALT;
	scale = scale_high = 1.1f; //0.5f;

	entry[DM_YES] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_YES], this);
	entry[DM_NO] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_NO], this);
	entry[DM_OK] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OK], this);
	entry[DM_FMT] = NEW FEMenuEntry(ksGlobalTextArray[GT_MC_FORMAT], this);
	entry[DM_CNL] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_CANCEL], this);
	entry[DM_MNG] = NEW FEMenuEntry(ksGlobalTextArray[GT_MC_MANAGE], this);
	entry[DM_CNL2] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_CANCEL], this);

	for(int i=0; i<DM_NUM; i++)
	{
		Add(entry[i]);
		entry[i]->SetFont(&manager->font_hand);
	}
	entry[DM_YES]->right = entry[DM_NO];
	entry[DM_NO]->left = entry[DM_YES];

	entry[DM_FMT]->right = entry[DM_CNL];
	entry[DM_CNL]->left = entry[DM_FMT];

	entry[DM_MNG]->right = entry[DM_CNL2];
	entry[DM_CNL2]->left = entry[DM_MNG];
	
	// center of box is 375, 206
	entry[DM_YES]->SetPos(340, 272);
	entry[DM_NO]->SetPos(410, 272);
	entry[DM_OK]->SetPos(375, 272);
	entry[DM_FMT]->SetPos( 320, 272 );
	entry[DM_CNL]->SetPos( 430, 272 );
	entry[DM_MNG]->SetPos( 320, 272 );
	entry[DM_CNL2]->SetPos( 430, 272 );
	
	float mess_sc = 0.7f; //0.67f;
	float prompt_sc = 0.9f;	// 0.89f;
	message = NEW BoxText(&manager->font_info, "", 375, 165, 0, mess_sc, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, frontendmanager.col_info_b, 12);
	prompt  = NEW BoxText(&manager->font_info, "", 375, 200, 0, prompt_sc, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, frontendmanager.col_info_b, 10);
	type = DM_TYPE_YES;
}

DialogMenu::~DialogMenu()
{
	delete message;
	delete prompt;
}

void DialogMenu::Draw()
{
	draw_count++;
	if(type == DM_TYPE_YES)
	{
		entry[DM_YES]->Draw();
		entry[DM_NO]->Draw();
	}
	else if(type == DM_TYPE_OK)
	{
		entry[DM_OK]->Draw();
	}
	else if( type == DM_TYPE_FMT )
	{
		entry[DM_FMT]->Draw();
		entry[DM_CNL]->Draw();
	}
	else if( type == DM_TYPE_MNG )
	{
		entry[DM_MNG]->Draw();
		entry[DM_CNL2]->Draw();
	}
	message->Draw();
	prompt->Draw();
}
void DialogMenu::OnLeft(int c)
{
	
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->unpause();

	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->pause();

	FEMultiMenu::OnLeft(c);
}

void DialogMenu::OnRight(int c)
{
	
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->pause();

	FEMultiMenu::OnRight(c);
}
void DialogMenu::OnCross(int c)
{
	if( type == DM_TYPE_YES || type == DM_TYPE_OK || type == DM_TYPE_FMT || type == DM_TYPE_MNG )
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();

		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();


		if( type == DM_TYPE_FMT )
		{
			if( highlighted->entry_num == DM_CNL )
				((SaveLoadFrontEnd*) parent)->DialogNoPressed();
			else if( highlighted->entry_num == DM_FMT )
				((SaveLoadFrontEnd*) parent)->DialogYesOKPressed();
		}
		else if( type == DM_TYPE_MNG )
		{
			if( highlighted->entry_num == DM_CNL2 )
				((SaveLoadFrontEnd*) parent)->DialogNoPressed();
			else if( highlighted->entry_num == DM_MNG )
				((SaveLoadFrontEnd*) parent)->DialogYesOKPressed();
		}
		else
		{
			if(type == DM_TYPE_YES && highlighted->entry_num == DM_NO)
				((SaveLoadFrontEnd*) parent)->DialogNoPressed();
			else
				((SaveLoadFrontEnd*) parent)->DialogYesOKPressed();
		}
	}
}

void DialogMenu::SetPQIndices()
{
	bar[0] = GetPointer("bar_meter");
	bar[1] = GetPointer("bar_bkg");
	bar[2] = GetPointer("bar_frame");
	bar[3] = GetPointer("bar_shd");
	box = GetPointer("box_right");
}

void DialogMenu::SetTypeAndMessage(int ty, stringx str)
{
	type = ty;
	if (str.length() > 40)
	{
		message->changeText(str);
		prompt->changeText("");
		message->makeBox(240, 75, false);
		prompt->makeBox(240, 75, false);
	}
	else
	{
		str.to_lower();
		prompt->changeText(str);
		message->changeText("");
		message->makeBox(240, 75, false);
		prompt->makeBox(240, 75, false);
	}
}

void DialogMenu::OnActivate()
{
	TurnPQ(true);
	if(type == DM_TYPE_YES)
		setHigh(entry[DM_NO]);
	else if( type == DM_TYPE_FMT )
		setHigh( entry[DM_CNL] );
	else if( type == DM_TYPE_MNG )
		setHigh( entry[DM_CNL2] );
	else
		setHigh(entry[DM_OK]);
	draw_count = 0;

	if(((SaveLoadFrontEnd*) parent)->in_frontend)
	{
		manager->helpbar->Reset();
		if(type == DM_TYPE_YES || type == DM_TYPE_FMT || type == DM_TYPE_MNG) manager->helpbar->AddArrowH();
		else manager->helpbar->RemoveArrowBoth();
		if(type == DM_TYPE_EMPTY) manager->helpbar->RemoveX();
		else manager->helpbar->RemoveTriangle();
		manager->helpbar->Reformat();
	}
}

void DialogMenu::TurnPQ(bool on)
{
	for(int i=0; i<4; i++)
		bar[i]->TurnOn(on && type == DM_TYPE_PROGRESS);
	box->TurnOn(on);
}

/*********************** Names Menu ************************/

NamesMenu::NamesMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf)
{
	cons(s, man, p, pf);
	dy = 25;  // down from normal 28 in FEMenu
	color = frontendmanager.col_unselected;
	color_high = frontendmanager.col_highlight;
	color_high_alt = frontendmanager.col_highlight2;
	flags |= FEMENU_HAS_COLOR_HIGH_ALT | FEMENU_SCROLLING;
	scale = scale_high = 1.0f;	//0.78f;
	Font* f = &system->manager->font_info;
	max_vis_entries = 10;

	cardSide = true;
	freeSpace = NEW BoxText(&system->manager->font_bold, "", 235, 285, 300, .666, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, frontendmanager.col_info_b, 10);
	MakeSpaceFreeMessage(0);

	blank_name = "- "+ksGlobalTextArray[GT_FE_MENU_BLANK]+" -";
	blank_name.to_lower();

	center_x = 260;
	center_y = 165;
	dy = 18;

	// first <num> entries are the saved games
	for(int i=0; i<num; i++)
	{
		entry[i] = NEW FEMenuEntry(blank_name, this);
		entry[i]->SetZ(440);
		entry[i]->SetFont(f);
		entry[i]->SetPos(center_x, center_y+dy*i);
		entry[i]->SetHJustify(Font::HORIZJUST_LEFT);
		empty[i] = true;
		Add(entry[i]);
	}

	// beyond that entries represent all the supported memory cards
	for(int i=0; i<NUMCARDS; i++)
	{
		int active = NamesMenu::FindActive(i);

		entry[i+num] = NEW FEMenuEntry(MakeCardName(active), this);
		entry[i+num]->SetLineSpacing(f->getGlyph('A')->cell_height);
		entry[i+num]->SetFont(&manager->font_thin);
		entry[i+num]->SetPos(375, 132);
		entry[i+num]->SetSpecialScale(0.78f, 0.78f);
		Add(entry[i+num]);
		cards[i].exists = false;
		cards[i].changed = false;
		cards[i].available = true;
		cards[i].saved_games = false;
		cards[i].status = GSErrorOther;
	}


	for(int i=0; i<num; i++)
	{
		if(i == 0)		entry[i]->up = entry[num-1];
		else			entry[i]->up = entry[i-1];
		if(i == num-1)	entry[i]->down = entry[0];
		else			entry[i]->down = entry[i+1];
	}
	for(int i=num; i<NUMCARDS+num; i++)
	{
		if(i == num)	entry[i]->left = entry[num+NUMCARDS-1];
		else			entry[i]->left = entry[i-1];
		if(i == num+NUMCARDS-1)	entry[i]->right = entry[num];
		else			entry[i]->right = entry[i+1];
	}
	
	check_cards_counter = 0;
	arrow_counter = -5;
	message = NEW BoxText(&system->manager->font_info, "", 260, 185, 300, scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, frontendmanager.col_info_b, 10);
	ResetCardFormat();
	draw_message = false;
}

stringx NamesMenu::MakeCardName(int active)
{
	int len;
	char tmp[100];
	strcpy(tmp, SaveLoadFrontEnd::MakeStringSlot(active).c_str());
	len = strlen(tmp);
	if (len > 15)
	{
		char *curr_ptr1, *curr_ptr2 = tmp + len/2;
		bool done = false;
		curr_ptr1 = curr_ptr2;
		while (curr_ptr1 != tmp && curr_ptr2 != tmp + len && !done)
		{	
			
			if (*curr_ptr1 == ' ')
			{
				*curr_ptr1 = '~';
				done = true;
			}
			else
			{
				curr_ptr1--;
				if (*curr_ptr2 == ' ')
				{
					*curr_ptr2 = '~';
					done = true;
				}
				else
					curr_ptr2++;
			}


		}
	}
		
	return stringx(tmp);
}
NamesMenu::~NamesMenu()
{
	delete message;
	delete freeSpace;
}

void NamesMenu::MakeSpaceFreeMessage(int freeBytes)
{
#ifdef TARGET_XBOX
	if (freeBytes > 50000)
		freeSpace->changeText(stringx(50000) +stringx("+ ") +ksGlobalTextArray[GT_MC_FREE] + " " + ksGlobalTextArray[GT_MC_BLOCKS] );
	else
		freeSpace->changeText(stringx(freeBytes) + stringx(" ") + ksGlobalTextArray[GT_MC_FREE] + " " + ksGlobalTextArray[GT_MC_BLOCKS]);
#elif defined(TARGET_GC)
	freeSpace->changeText(stringx(freeBytes) + stringx(" ") + ksGlobalTextArray[GT_MC_BLOCKS] + " " + ksGlobalTextArray[GT_MC_FREE] );
#else
	freeSpace->changeText(stringx(freeBytes) + stringx(" ") + ksGlobalTextArray[GT_MC_KB] + " " + ksGlobalTextArray[GT_MC_FREE] );
#endif

	freeSpace->makeBox(100,50);
}


void NamesMenu::SetPQIndices()
{
	box = GetPointer("box_right");
	arrows[0][0] = GetPointer("button_off_01");
	arrows[0][1] = GetPointer("button_on_01");
	arrows[1][0] = GetPointer("button_off_02");
	arrows[1][1] = GetPointer("button_on_02");
	top_line = GetPointer("line_right_01");
	side_line = GetPointer("line_right_12");
	for(int i=0; i<10; i++)
	{
		stringx tmp = stringx(i+2);
		if(i+2 < 10) tmp = "0"+tmp;
		lines[i] = GetPointer(("line_right_"+tmp).data());
	}
}

void NamesMenu::TurnPQ(bool on)
{
	box->TurnOn(on);
	top_line->TurnOn(on);
	side_line->TurnOn(on);
	arrows[0][0]->TurnOn(on);
	arrows[0][1]->TurnOn(false);
	arrows[1][0]->TurnOn(on);
	arrows[1][1]->TurnOn(false);
	TurnPQLines(on);
}

void NamesMenu::TurnPQLines(bool on)
{
	for(int i=0; i<10; i++)
		lines[i]->TurnOn(on);
}

void NamesMenu::Init()
{
	sl_parent = (SaveLoadFrontEnd*) parent;
}

void NamesMenu::Update(time_value_t time_inc)
{
	if(sl_parent->Busy()) return;

	// execute CheckCurrentCard only every 10 times, because it's slow
	check_cards_counter++;
	if(check_cards_counter >= 10)
	{
		CheckCardNum(active_card);
		MakeSpaceFreeMessage(cards[adjusted_active_card].free);
		check_cards_counter = 0;
	}

	// if the card that previously needed formatting has been pulled out, end
	// the format loop
/*	if(!cards[adjusted_active_card].exists && cards[adjusted_active_card].changed && !cards[adjusted_active_card].available)
		AskFormat(false);*/

	if(cards[adjusted_active_card].changed && !cards[adjusted_active_card].exists && sl_parent->DialogActive())
		sl_parent->CancelDialog();

#ifdef TARGET_XBOX	// Not supposed to display empty memory unit slots on Xbox (dc 07/07/02)
	if(!cards[adjusted_active_card].exists)
	{
		OnRight(0);
	}
#endif
	
	// if card has been put in or pulled out
	if(cards[adjusted_active_card].changed)
		RefreshDisplay();

	if (!sl_parent->DialogActive())
		UpdateMessage();
	FEMultiMenu::Update(time_inc);
}

void NamesMenu::Draw()
{
	if(arrow_counter >= 0) arrow_counter--;
	if(arrow_counter < 0 && arrow_counter > -5)
	{
		arrow_counter = -5;
		arrows[0][0]->TurnOn(true);
		arrows[0][1]->TurnOn(false);
		arrows[1][0]->TurnOn(true);
		arrows[1][1]->TurnOn(false);
	}

	if (cards[adjusted_active_card].exists && cards[adjusted_active_card].available)
	{
#ifndef TARGET_PS2 // Sony recommends we don't bother drawing this
		freeSpace->Draw();
#endif
	}
	highlighted->Draw();
	if(!draw_message)
	{
		FEMenuEntry* tmp = first_vis_entry;
		for(int i=0; i<max_vis_entries; i++)
		{
			tmp->Draw();
			tmp = tmp->next;
		}
	}
	else message->Draw();
}

void NamesMenu::DrawHeader()
{
	highlighted->Draw();
}

void NamesMenu::OnCross(int c)
{
	if(save && g_career->GetSurferIdx() == SURFER_LAST)
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();

		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();

		return;
	}
	if(!save && !AvailAndSavedGames(active_card))
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();

		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();

		return;
	}
#ifdef TARGET_GC
	if(save && (!cards[adjusted_active_card].exists || !cards[adjusted_active_card].available))
#else
	if(save && !cards[adjusted_active_card].exists)
#endif
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();

		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();

		return;
	}
	
	if(save && empty[secondary_cursor->entry_num])
	{
		int free, type, formatted;
		GenericGameSaver::inst()->getInfo((active_card!=-1?active_card/NUM_MEMORY_SLOTS:-1), (active_card!=-1?active_card%NUM_MEMORY_SLOTS:0), &type, &free, &formatted);
		if ((unsigned int) free < (unsigned int) GenericGameSaver::inst()->getSavedGameSize() && formatted)
		{
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->unpause();

			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->pause();

			sl_parent->StartError(SE_NOT_ENOUGH_SPACE);
		}
		else
		{
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->unpause();
	
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->pause();

			sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE_NAME);
		}
	}
	else
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();

		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();

		strcpy(sl_parent->desc, highlighted->GetText().data());
		switch(type)
		{
		case SaveLoadFrontEnd::SL_SAVE: sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE_PICK); break;
		case SaveLoadFrontEnd::SL_LOAD: sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_LOAD_PICK); break;
		case SaveLoadFrontEnd::SL_DELETE: sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_DELETE_PICK); break;
		}
	}
}

void NamesMenu::OnDown(int c)
{
	if(secondary_cursor->down)
	{
		if((flags & FEMENU_SCROLLING) && secondary_cursor == last_vis_entry)
		{
			FEMenuEntry* first = first_vis_entry->down;

			while(first && first->GetDisable())
				first = first->next;
			if(first->entry_num <= num - max_vis_entries)
				SetVis(first);
		}

		FEMenuEntry* tmp = secondary_cursor->down;
		while(tmp && tmp->GetDisable() && tmp != entry[0])
			tmp = tmp->down;

		if(tmp && tmp->entry_num < num && tmp != entry[0])
		{
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->unpause();

			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->pause();

			SetSecondaryCursor(tmp);
		}
	}
}

void NamesMenu::OnUp(int c)
{
	if(secondary_cursor->up)
	{
		if((flags & FEMENU_SCROLLING) && secondary_cursor == first_vis_entry &&
			first_vis_entry != entry[0])
		{
			FEMenuEntry* first = first_vis_entry->up;

			while(first && first->GetDisable() && first != entry[0])
				first = first->previous;
			if(first->entry_num >= 0)
				SetVis(first);
		}

		FEMenuEntry* tmp = secondary_cursor->up;
		while(tmp && tmp->GetDisable() && tmp->down != entry[0])
			tmp = tmp->up;

		if(tmp && tmp->entry_num < num && tmp->down != entry[0])
		{
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->unpause();

			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->pause();

			SetSecondaryCursor(tmp);
		}
	}
}

void NamesMenu::SetVis(FEMenuEntry* first)
{
	first_vis_entry = first;
	FEMenuEntry* tmp = first;
	int y;
	for(int i=0; i<max_vis_entries; i++)
	{
		if(tmp == NULL) tmp = entry[0];
		while(flags & FEMENU_DONT_SHOW_DISABLED && tmp->GetDisable())
		{
			tmp = tmp->next;
			if(tmp == NULL) tmp = entry[0];
		}
		y = center_y + i * dy;
		tmp->SetPos(center_x, y);
		if(i == max_vis_entries-1) last_vis_entry = tmp;
		tmp = tmp->next;
	}
}

void NamesMenu::OnLeft(int c)
{
	FEMenuEntry* initial = highlighted;
#ifdef TARGET_XBOX
	do 
	{
#endif
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnLeft(c);
	if(before != highlighted)
	{
		OnCardSwitch();
	}
#ifdef TARGET_XBOX
	} while (cards[adjusted_active_card].exists == false);
#endif
	if(initial != highlighted)
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		if (frontendmanager.fe_done)
				SoundScriptManager::inst()->pause();
		arrows[0][1]->TurnOn(true);
		arrows[0][0]->TurnOn(false);
		arrow_num = 0;
		arrow_counter = arrow_timer;
		cards[adjusted_active_card].changed = true;
/*		if (!cards[adjusted_active_card].available)
			cards[adjusted_active_card].ask_format = true;*/
		MakeSpaceFreeMessage(cards[adjusted_active_card].free);
	}
}

void NamesMenu::OnRight(int c)
{
	FEMenuEntry* initial = highlighted;
#ifdef TARGET_XBOX
	do 
	{
#endif
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnRight(c);
	if(before != highlighted)
	{
		OnCardSwitch();
	}
#ifdef TARGET_XBOX
	} while (cards[adjusted_active_card].exists == false);
#endif
	if(initial != highlighted)
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();
		arrows[1][1]->TurnOn(true);
		arrows[1][0]->TurnOn(false);
		arrow_num = 1;
		arrow_counter = arrow_timer;
		cards[adjusted_active_card].changed = true;
/*		if (!cards[adjusted_active_card].available)
			cards[adjusted_active_card].ask_format = true;*/
		MakeSpaceFreeMessage(cards[adjusted_active_card].free);
	}
}

void NamesMenu::OnCardSwitch()
{
	adjusted_active_card = highlighted->entry_num - num;
	active_card = NamesMenu::FindActive(adjusted_active_card);
	highlighted->SetSpecialColor(frontendmanager.col_highlight, frontendmanager.col_highlight2);
	SetSecondaryCursor(entry[0]);
	cards[active_card].changed = true;
	CheckCardNum(active_card);
	RefreshDisplay();
}

void NamesMenu::OnActivate()
{
	TurnPQ(true);
	type = sl_parent->cur_type;
	save = (type == SaveLoadFrontEnd::SL_SAVE);
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnActivate();
	draw_message = false;

	// card was set previously, return to that card
	if(before)
	{
		setHigh(before);
		adjusted_active_card = before->entry_num - num;
		active_card = NamesMenu::FindActive(adjusted_active_card);
		CheckCardNum(active_card);

		// In case the file list has changed since the previous viewing.  (dc 07/07/02)
		GetFileList(active_card, adjusted_active_card);
	}
	else // use default card
	{
		for(int i=LOWEST_CARD; i<HIGHEST_CARD; i++)
		{
			CheckCardNum(i);
			int adjust = NamesMenu::FindAdjusted(i);
			GetFileList(i, adjust);
		}
		SetActiveCard();

		FEMenuEntry* tmp = entry[NamesMenu::FindAdjusted(active_card)+num];
		setHigh(tmp);
	}

	OnCardSwitch();

	UpdateMessage();	
	//cards[adjusted_active_card].changed = true;
	SetSecondaryCursor(entry[0]);
	first_vis_entry = secondary_cursor;
	last_vis_entry = entry[9];
	highlighted->SetSpecialColor(frontendmanager.col_highlight, frontendmanager.col_highlight2);

	if(sl_parent->in_frontend)
	{
		manager->helpbar->Reset();
		manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
		manager->helpbar->AddArrowV();
		manager->helpbar->Reformat();
	}
	
}

void NamesMenu::SetActiveCard()
{
	// set the default option, but look for a better card
	active_card = LOWEST_CARD;
	adjusted_active_card = NamesMenu::FindAdjusted(active_card);

	for(int i=LOWEST_CARD; i<HIGHEST_CARD; i++)
	{
		if((save && Available(i, true)) || AvailAndSavedGames(i))
		{
			active_card = i;
			adjusted_active_card = NamesMenu::FindAdjusted(i);
			break;
		}
	}
}

// updates the message ("UNFORMATTED" e.g.) for current card
void NamesMenu::UpdateMessage()
{
	draw_message = true;
	if(g_career->GetSurferIdx() == SURFER_LAST && sl_parent->GetDState()== SaveLoadFrontEnd::DSTATE_SAVE)
	{
		message->changeText(ksGlobalTextArray[GT_FE_MENU_NO_CAREER]);

	}
	else if(cards[adjusted_active_card].exists)
	{
#if defined( TARGET_GC )
		if( !cards[adjusted_active_card].available )
		{
			switch( cards[adjusted_active_card].status )
			{
				case GSErrorUnformatted:
				case GSErrorUnknownMedia:
				case GSErrorNotEnoughSpace:
				case GSErrorIncompatible:
				case GSErrorWrongRegion:
				case GSErrorDamaged:
					message->changeText( GenericGameSaver::inst()->getErrorString( active_card, 0, cards[adjusted_active_card].status ) );
					break;
				default:
					assert( 0 );
					break;
			}
		}
		else
#elif defined( TARGET_XBOX )
		if (!cards[adjusted_active_card].available)
			message->changeText(GenericGameSaver::inst()->getUnavailableCardString(active_card==-1?-1:active_card/NUM_MEMORY_SLOTS, active_card==-1?0:active_card%NUM_MEMORY_SLOTS));
		else 
#endif
		if(!save && !cards[adjusted_active_card].saved_games)
			message->changeText(ksGlobalTextArray[GT_FE_MENU_NO_SAVED_GAMES]);
		else 
			draw_message = false;
	}
	else 
		message->changeText(GenericGameSaver::inst()->getInsertCardString(active_card==-1?-1:active_card/NUM_MEMORY_SLOTS, active_card==-1?0:active_card%NUM_MEMORY_SLOTS));
	if(draw_message) 
		message->makeBox(240, 75, false, -1, 17);
	TurnPQLines(!draw_message);
}

void NamesMenu::OnUnactivate(FEMenu* m)
{
	TurnPQ(false);
	highlighted->SetSpecialColor(frontendmanager.col_info_b, frontendmanager.col_info_b);
}

void NamesMenu::RefreshDisplay()
{
	if(cards[adjusted_active_card].exists)
	{
		GetFileList(active_card, adjusted_active_card);
	}
	UpdateMessage();
}

void NamesMenu::GetFileList(int active, int adjusted)
{
	if (cards[adjusted].available)
		sl_parent->getFileListing(active, adjusted);
	else
		sl_parent->numGameConfig[adjusted]=0;
	// determine which entries are empty, and change the labels of those
	// that aren't empty
	for(int i=0; i<num; i++)
	{
		if(i < sl_parent->numGameConfig[adjusted])
		{
			stringx tmp;
			if (strlen(sl_parent->savedConfigs[adjusted][i].desc) > 22)
			{
				char name[26];
				strncpy(name, sl_parent->savedConfigs[adjusted][i].desc, 9);
				name[22] = '.';name[23] = '.'; name[24] = '.';
				name[25]= '\0';
				tmp = stringx(name);
			}
			else
				tmp = sl_parent->savedConfigs[adjusted][i].desc;

			ReplaceBadCharacters(tmp);
			tmp.to_upper();
			entry[i]->SetText(tmp);
		}
		else entry[i]->SetText(blank_name);
		empty[i] = (i >= sl_parent->numGameConfig[adjusted]);
	}

	cards[adjusted].saved_games = false;
	for(int i=0; i<num; i++)
	{
		// disable if in load or delete mode; enable if save mode
		if(empty[i]) 
		{
			if (!save)
			{
				entry[i]->Disable(true);
				entry[i]->SetText("");
			}
			else
			{
				entry[i]->Disable(false);
				entry[i]->SetText(blank_name);
			}
		}
		else
		{
			entry[i]->Disable(false);		// in case they were ever previously disabled
			cards[adjusted].saved_games = true;
		}
	}
}

// replace all non-alphanumeric (or space) characters with the bad character symbol,
// which is represented by '#'
void NamesMenu::ReplaceBadCharacters(stringx &tmp)
{
	for(int i=0; i<tmp.length(); i++)
	{
		char c = tmp[i];
		if(!(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z') && 
			!(c >= '0' && c <= '9') && c != ' ' && c!='(' && c!= ')')
			tmp[i] = '#';
	}
}

void NamesMenu::CheckCardNum(int card_num)
{
	int adjust = NamesMenu::FindAdjusted(card_num);
	int type=0, formatted=0;
	cards[adjust].free = 0;
	int ret;
	bool allow_save_errors = true;
	
	if( g_career->GetSurferIdx() == SURFER_LAST && sl_parent->GetDState()== SaveLoadFrontEnd::DSTATE_SAVE )
		allow_save_errors = false;

	ret = GenericGameSaver::inst()->getInfo((card_num!=-1)?card_num/NUM_MEMORY_SLOTS:-1, (card_num!=-1)?card_num%NUM_MEMORY_SLOTS:0, &type, &(cards[adjust].free), &formatted);

	int status_before = cards[adjust].status;
	
	cards[adjust].status = ret;
	
#ifdef TARGET_GC
	if( GenericGameSaver::inst()->getFileListing( card_num / NUM_MEMORY_SLOTS, card_num % NUM_MEMORY_SLOTS, NULL, NULL, NULL ) )
	{
		if( cards[adjust].free < GenericGameSaver::inst()->getSavedGameSize() )
			cards[adjust].status = GSErrorNotEnoughSpace;
	}
#endif //TARGET_GC

	cards[adjust].exists = true;
	cards[adjust].available = false;
	
	switch(ret)
	{
	case GSOk:
		cards[adjust].exists = true;
		cards[adjust].available = true;
		break;
#if defined( TARGET_XBOX )
	case GSErrorUnformatted:
	case GSErrorNotEnoughSpace:
#elif defined( TARGET_GC )
	case GSErrorUnformatted:
		if( cards[adjust].changed && allow_save_errors )
		{
			sl_parent->StartError( SE_GC_CORRUPT );
		}
		break;
	case GSErrorWrongRegion:
		if( cards[adjust].changed && allow_save_errors )
		{
			sl_parent->StartError( SE_GC_REGION );
		}
		break;
	case GSErrorNotEnoughSpace:
		if( cards[adjust].changed && allow_save_errors )
		{
			sl_parent->StartError( SE_NOT_ENOUGH_SPACE );
		}
		break;
	case GSErrorUnknownMedia:
	case GSErrorIncompatible:
	case GSErrorDamaged:
#else
	case GSErrorUnformatted:
#endif	
		//The default
		break;
	default:
		cards[adjust].exists = false;
		cards[adjust].available = false;
		break;
	}
	cards[adjust].changed = cards[adjust].status != status_before;
	if (cards[adjust].changed)
		entry[adjust+num]->SetText(MakeCardName(card_num));
	
	if( cards[adjust].changed )
		sl_parent->EndError( -1 );
//	entry[adjust+num]->Disable(!cards[adjust].exists && !save);
}

bool NamesMenu::Available(int card, bool unformatted_ok)
{
	int adjust = NamesMenu::FindAdjusted(card);
	return (cards[adjust].exists && (cards[adjust].available || unformatted_ok));
}

bool NamesMenu::AvailAndSavedGames(int card)
{
	int adjust = NamesMenu::FindAdjusted(card);
	return (cards[adjust].exists && cards[adjust].available && cards[adjust].saved_games);
}

void NamesMenu::AskFormat(bool start)
{
	// only bug about formatting if in save mode
	if(save)
	{
#ifndef TARGET_XBOX	// On Xbox, you have to go to the dashboard to format the MU (dc 07/02/02)
		if(start) sl_parent->StartError(SE_UNFORMAT);
		else 
#endif
			sl_parent->EndError(SE_UNFORMAT);
	}
}

void NamesMenu::Format()
{
	int ret = GenericGameSaver::inst()->format((active_card!=-1)?active_card/NUM_MEMORY_SLOTS:-1, (active_card!=-1)?active_card%NUM_MEMORY_SLOTS:0);
	cards[FindAdjusted(ActiveCard())].status = ret;
	if(ret != GSOk)
		sl_parent->StartError(SE_FORM_ERROR);
	else 
	{
		cards[FindAdjusted(ActiveCard())].available = true;
		sl_parent->SetDState( sl_parent->post_format_state );
	}
}

int NamesMenu::ActiveFile()
{
	if(highlighted)
		return secondary_cursor->entry_num;
	else return -1;
}

void NamesMenu::ResetCardFormat()
{
	for(int i=0; i<NUMCARDS; i++)
		cards[i].ask_format = true;
}

/*
void NamesMenu::FindAvailableSlot()
{
	if(sl_parent->ExistsAndHasSpace(0)) card_active = 0;
	else if(sl_parent->ExistsAndHasSpace(1)) card_active = 1;
	//else card_active = -1;
}
*/

int NamesMenu::FindAdjusted(int active)
{
#ifdef TARGET_XBOX
	assert(active >= -1 && active <= HIGHEST_CARD - 1);
	return active+1;
#else
	return active;
#endif
}

int NamesMenu::FindActive(int adjusted)
{
#ifdef TARGET_XBOX
	assert(adjusted >= 0 && adjusted <= HIGHEST_CARD);
	return adjusted-1;
#else
	return adjusted;
#endif
}

void NamesMenu::SetSecondaryCursor(FEMenuEntry* e)
{
	FEMultiMenu::SetSecondaryCursor(e);
	if(e->entry_num < max_vis_entries) SetVis(entry[0]);
	else if(e->entry_num + max_vis_entries <= num) SetVis(e);
	else SetVis(entry[num - max_vis_entries]);
}

/*********************** Keyboard Menu ************************/

KeyboardMenu::KeyboardMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf)
{
	cons(s, man, p, pf);
	color = frontendmanager.col_unselected;
	color_high = frontendmanager.col_highlight;
	color_high_alt = frontendmanager.col_highlight2;
	flags |= FEMENU_HAS_COLOR_HIGH_ALT;
	scale = scale_high = 0.9f;	//0.67f;
	Font* f = &manager->font_bold_old;

	char name[2];
	name[1] = '\0';
	for(int i=0; i<36; i++)
	{
		if(i < 26) name[0] = 'A'+i;
		else if(i < 36) name[0] = '0'+i-26;
		ent[i] = NEW FEMenuEntry(name, this);
		ent[i]->SetPos(271+30*(i%row_size), 142+30*(i/row_size));
	}
	ent[space_idx] = NEW FEMenuEntry("_", this);
	ent[space_idx]->SetPos(391, 257);
	ent[back_idx] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_BACK], this);
	ent[back_idx]->SetPos(451, 262);
	ent[enter_idx] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_ENTER], this);
	ent[enter_idx]->SetPos(321, 322);
	ent[enter_idx]->SetFont(&manager->font_hand);
	ent[cancel_idx] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_CANCEL], this);
	ent[cancel_idx]->SetPos(421, 322);
	ent[cancel_idx]->SetFont(&manager->font_hand);
	first = ent[0];

	for(int i=0; i<num; i++)
	{
		Add(ent[i]);
		ent[i]->SetZ(300);
		ent[i]->SetFont(f);

		if(i == num-1)			ent[i]->right = ent[0];
		else					ent[i]->right = ent[i+1];
		if(i == 0)				ent[i]->left = ent[num-1];
		else					ent[i]->left = ent[i-1];
		if(i < row_size/2)		ent[i]->up = ent[enter_idx];
		else if(i < row_size)	ent[i]->up = ent[cancel_idx];
		else if(i < enter_idx)	ent[i]->up = ent[i-row_size];
		else if(i == enter_idx) ent[i]->up = ent[i-4];
		else					ent[i]->up = ent[i-2];
		if(i < enter_idx-8)		ent[i]->down = ent[i+8];
		else if(i < enter_idx-6)ent[i]->down = ent[back_idx];
		else if(i < enter_idx-2)ent[i]->down = ent[enter_idx];
		else if(i < enter_idx)	ent[i]->down = ent[cancel_idx];
		else					ent[i]->down = ent[0];
	}

	filename = NEW TextString(f, "", 375, 293, 300, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, frontendmanager.col_bio);
	enter_text = NEW TextString(&system->manager->font_bold, ksGlobalTextArray[GT_FE_MENU_ENTER_NAME1], 235, 203, 300, 0.8f, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, manager->col_info_b);
	name_text = NEW TextString(&system->manager->font_bold, ksGlobalTextArray[GT_FE_MENU_ENTER_NAME2], 235, 227, 300, 0.8f, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, manager->col_info_b);
}

KeyboardMenu::~KeyboardMenu()
{
	delete filename;
	delete enter_text;
	delete name_text;
}

void KeyboardMenu::SetPQIndices()
{
	for(int i=0; i<26; i++)
	{
		char c[2];
		c[0] = 'a'+i;
		c[1] = '\0';
		stringx tmp = stringx(c);
		keys[i][0] = GetPointer(("key_off_"+tmp).data());
		keys[i][1] = GetPointer(("key_on_"+tmp).data());
		keys[i][2] = GetPointer(("key_grey_"+tmp).data());
	}
	for(int i=26; i<36; i++)
	{
		if(i != 35)
		{
			keys[i][0] = GetPointer(("key_off_"+stringx(i-26+1)).data());
			keys[i][1] = GetPointer(("key_on_"+stringx(i-26+1)).data());
			keys[i][2] = GetPointer(("key_grey_"+stringx(i-26+1)).data());
		}
		else
		{
			// the nine is placed where the art thinks the 0 is placed
			keys[i][0] = GetPointer("key_off_0");
			keys[i][1] = GetPointer("key_on_0");
			keys[i][2] = GetPointer("key_grey_0");
		}
	}
	keys[space_idx][0] = GetPointer("key_off_dash");
	keys[space_idx][1] = GetPointer("key_on_dash");
	keys[space_idx][2] = GetPointer("key_grey_dash");
	keys[back_idx][0] = GetPointer("key_off_clear1");
	keys[back_idx][1] = GetPointer("key_on_clear1");
	keys[back_idx][2] = GetPointer("key_grey_clear1");
	keys[back_idx+1][0] = GetPointer("key_off_clear2");
	keys[back_idx+1][1] = GetPointer("key_on_clear2");
	keys[back_idx+1][2] = GetPointer("key_grey_clear2");
	keys[back_idx+2][0] = GetPointer("key_off_clear3");
	keys[back_idx+2][1] = GetPointer("key_on_clear3");
	keys[back_idx+2][2] = GetPointer("key_grey_clear3");

	for(int i=0; i<3; i++)
		name_box[i] = GetPointer(("name_box"+stringx(i+1)).data());
}

void KeyboardMenu::TurnPQ(bool on)
{
	for(int i=0; i<num; i++)
	{
		keys[i][0]->TurnOn(false);
		keys[i][1]->TurnOn(false);
		keys[i][2]->TurnOn(on);
	}
	for(int i=0; i<3; i++)
		name_box[i]->TurnOn(on);
}

void KeyboardMenu::Init()
{
	sl_parent = (SaveLoadFrontEnd*) parent;
}

void KeyboardMenu::Draw()
{
	filename->Draw();
	enter_text->Draw();
	name_text->Draw();
	FEMultiMenu::Draw();
}

void KeyboardMenu::Update(time_value_t time_inc)
{
	FEMultiMenu::Update(time_inc);
	if(highlighted->entry_num != enter_idx && highlighted->entry_num != cancel_idx)
	{
		float tmp = highlighted->GetHighlightIntensity();
		keys[highlighted->entry_num][0]->TurnOn(tmp < 0);
		keys[highlighted->entry_num][1]->TurnOn(tmp > 0);
		if(highlighted->entry_num == back_idx)
		{
			keys[highlighted->entry_num+1][0]->TurnOn(tmp < 0);
			keys[highlighted->entry_num+1][1]->TurnOn(tmp > 0);
			keys[highlighted->entry_num+2][0]->TurnOn(tmp < 0);
			keys[highlighted->entry_num+2][1]->TurnOn(tmp > 0);
		}
	}
}

void KeyboardMenu::OnCross(int c)
{
	char n = highlighted->entry_num;
	int j;
	if(n <= back_idx && !default_cleared) // if it's a number, letter, or space, and the default name hasn't been cleared
	{
		default_cleared = true;
		name = "";
		filename->changeText(name);
	}

	if(name.length() < name_size || n > space_idx)
	{
		if(n < 26) name.append('A'+ n);
		else if(n < 36) name.append('0'+ (n - 26));
		else if(n == space_idx) name.append(' ');
		else if(n == back_idx)
		{
			int t = name.length();
			if(t >= 1) name.truncate(t-1);
		}
		else if(n == enter_idx)
		{
			name.remove_surrounding_whitespace();

			if (name.length() == 0)
			{
				sl_parent->StartError(SE_NO_FILENAME);
				if (frontendmanager.fe_done)
					SoundScriptManager::inst()->unpause();
				SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
				if (frontendmanager.fe_done)
					SoundScriptManager::inst()->pause();
				return;
			}
			else 
			{
#ifdef TARGET_XBOX
				for (int i=0; i < sl_parent->numGameConfig[sl_parent->getActiveCard()+1]; i++)
					if (name == sl_parent->savedConfigs[sl_parent->getActiveCard()+1][i].desc)
#else
				for (int i=0; i < sl_parent->numGameConfig[sl_parent->getActiveCard()]; i++)
					if (name == sl_parent->savedConfigs[sl_parent->getActiveCard()][i].desc)
#endif
					{
						FEMenuEntry *f;
						f = sl_parent->NameMenu->entries;
						for (j=0; j < i; j++)
							f = f->next;
					
						strcpy(sl_parent->desc, name.c_str());
						sl_parent->NameMenu->SetSecondaryCursor(f);
						sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE_PICK);
						return;
					}
				
				sl_parent->SetOverwrite(name.c_str());
				sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVING);
			}
		}
		else	// cancel
		{
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			if (frontendmanager.fe_done)
				SoundScriptManager::inst()->pause();


			if (frontendmanager.fe_done)
			{
				sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE);
			}
			else
			{
				if(sl_parent->back_menu == -1) 
					sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE);
				else if(sl_parent->back_sub_menu == -1)
					system->MakeActive(sl_parent->back_menu);
				else system->MakeActive(sl_parent->back_menu, sl_parent->back_sub_menu);
			}
		}

		if(n < enter_idx)
		{
			filename->changeText(name);
		}
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();
	}
	else
	{
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		if (frontendmanager.fe_done)
			SoundScriptManager::inst()->pause();
	}
}

void KeyboardMenu::OnUp(int c)
{
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnUp(c);
	Switch(before, highlighted);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->pause();
}

void KeyboardMenu::OnDown(int c)
{
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnDown(c);
	Switch(before, highlighted);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->pause();
}

void KeyboardMenu::OnLeft(int c)
{
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnLeft(c);
	Switch(before, highlighted);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->pause();
}

void KeyboardMenu::OnRight(int c)
{
	FEMenuEntry* before = highlighted;
	FEMultiMenu::OnRight(c);
	Switch(before, highlighted);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if (frontendmanager.fe_done)
		SoundScriptManager::inst()->pause();
}

void KeyboardMenu::Switch(FEMenuEntry* before, FEMenuEntry* after)
{
	if(before == after) return;

	// if index is clear, have to turn on/off the three PQ's of the clear button
	// if enter or cancel, ignore because they don't have associated PQ's
	if(before->entry_num == back_idx)
	{
		KeyOn(back_idx, false);
		KeyOn(enter_idx, false);
		KeyOn(cancel_idx, false);
	}
	else if(before->entry_num < back_idx)
		KeyOn(before->entry_num, false);

	if(after->entry_num == back_idx)
	{
		KeyOn(back_idx, true);
		KeyOn(enter_idx, true);
		KeyOn(cancel_idx, true);
	}
	else if(after->entry_num < back_idx)
		KeyOn(after->entry_num, true);
}

void KeyboardMenu::KeyOn(int index, bool on)
{
	keys[index][2]->TurnOn(!on);
	keys[index][1]->TurnOn(on);
	keys[index][0]->TurnOn(on);
}

void KeyboardMenu::OnActivate()
{
	
	TurnPQ(true);
	FEMultiMenu::OnActivate();
	KeyOn(highlighted->entry_num, true);

	if(sl_parent->in_frontend)
	{
		manager->helpbar->Reset();
		manager->helpbar->Reformat();
	}

	name = "";
	if (strlen(SurferDataArray[g_career->GetSurferIdx()].lastname) > 0)
	{
		name.append(SurferDataArray[g_career->GetSurferIdx()].firstname[0]);
		name.append(' ');
		name.append(SurferDataArray[g_career->GetSurferIdx()].lastname);
		name.to_upper();
	}
	else
	{
		name.append(SurferDataArray[g_career->GetSurferIdx()].firstname);
		name.to_upper();
	}
	filename->changeText(name);
	default_cleared = false;

	Switch(highlighted, ent[enter_idx]);
	setHigh(ent[enter_idx]);
}

void KeyboardMenu::OnUnactivate(FEMenu* m)
{
	TurnPQ(false);
}



