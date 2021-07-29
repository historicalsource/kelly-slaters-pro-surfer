// SaveLoadFrontEnd.cpp
//

#include "global.h"
#include "SaveLoadFrontEnd.h"
#include "FEPAnel.h"
#include <time.h>
#include "FEMenu.h"

#include "FrontEndManager.h"

#include "GameData.h"

SaveLoadFrontEnd::SaveLoadFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq)
{
	cons(s, man, p, pq);
	sys = (GraphicalMenuSystem*) s;
	progressval = 0;
	thedata = NULL;
	fileType = 0;

#if defined(TARGET_PS2)
	GenericGameSaver::inst()->getIconData();

#endif

//	error = NEW TextString(&manager->bio_font_new, "", 320, 230, 0, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->bio_font_blue);
	error = NEW BoxText(&manager->bold, "", 320, 230, 0, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->bio_font_blue);

	for(int i=0; i<lsd_num; i++)
	{
		FEGraphicalMenuEntry* tmp = NEW FEGraphicalMenuEntry(this);
		if(i == 0) first = tmp;
		if(i == lsd_num-1) last = tmp;
		Add(tmp);
	}

	FEMenuEntry* tmp = first;
	while(tmp != last->next)
	{
		tmp->down = NULL;
		tmp->up = NULL;
		if(tmp != last) tmp->right = tmp->next;
		else tmp->right = first;
		if(tmp != first) tmp->left = tmp->previous;
		else tmp->left = last;
		tmp = tmp->next;
	}


	NameMenu = NEW NamesMenu(s);
	AddSubmenu(NameMenu);
	KeyMenu = NEW KeyboardMenu(s);
	AddSubmenu(KeyMenu);
}

SaveLoadFrontEnd::~SaveLoadFrontEnd()
{
	delete error;
//	delete ok;
//	delete deleting;
//	delete formatting;
//	delete save_done;
//	delete load_done;
//	delete del_done;
//	delete form_done;
	delete NameMenu;
	delete KeyMenu;
	delete[] thedata;
}

void SaveLoadFrontEnd::getFileListing(int port, int slot)
{
	numGames = GenericGameSaver::inst()->getFileListing(port, slot, SavedData[slot], NULL, NULL);

	// Now we filter out into two types of data
	fileType = 0;
	numGameData[slot] = 0;
	numGameConfig[slot] = 0;
	for (int i=0; i < numGames; i++)
	{
		if (SavedData[slot][i].type == 1) numGameData[slot]++;
	    else if (SavedData[slot][i].type == 0) numGameConfig[slot]++;
		else assert(0);
	}

	int count1=0, count2=0;

	for (int i=0; i < numGames; i++)
	{
		if (SavedData[slot][i].type == 1)
		{
			savedGames[slot][count1] = SavedData[slot][i];
			count1++;
		}
		else if (SavedData[slot][i].type == 0)
		{
			savedConfigs[slot][count2] = SavedData[slot][i];
			count2++;
		}
	}
}


void SaveLoadFrontEnd::Init()
{
	NameMenu->Init();
	KeyMenu->Init();
	FEMultiMenu::Init();
}


void SaveLoadFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();
	SetDState(DSTATE_LSD);

	FEGraphicalMenuEntry* tmp = (FEGraphicalMenuEntry*) entries;
	for(int i=0; i<lsd_num; i++)
	{
		tmp->Load(lsdPQ[i][0], lsdPQ[i][1]);
		tmp = (FEGraphicalMenuEntry*) tmp->next;
	}

}

void SaveLoadFrontEnd::Update(time_value_t time_inc)
{
	if(active) active->Update(time_inc);

	// only begin the lengthy process if "deleting..." or "formatting..."
	//  has already been drawn
	if(progress_drawn > 5)
	{
		progress_drawn = 0;
		if(dstate == DSTATE_DELETING)
			DelFile(NameMenu->ActiveCard(), NameMenu->ActiveFile());
	}
	FEMultiMenu::Update(time_inc);
}

void SaveLoadFrontEnd::Draw()
{
	// set progress bar
	if(dstate == DSTATE_SAVING)
		SetProgress(progressval);


	panel.Draw(0);

	if(active != NULL) active->Draw();

	// always draw this
	FEMenuEntry* tmp = entries;
	while(tmp != NULL)
	{
		tmp->Draw();
		tmp = tmp->next;
	}
	if(dstate == DSTATE_ERROR)
		error->Draw();

	if(dstate == DSTATE_DELETING)
		progress_drawn++;

}

void SaveLoadFrontEnd::SetLoadProgress(void *userData, int val)
{
	if(val >= 0)
		((SaveLoadFrontEnd *)userData)->progressval = val;
	if(val >= 100)
	{
		StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
		((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_LOAD_DONE);
		((SaveLoadFrontEnd *)userData)->progressval = 0;
	}
	if(val < 0)
  		((SaveLoadFrontEnd *)userData)->StartError(ksGlobalTextArray[GT_FE_MENU_LOAD_FAIL], SE_LOAD_ERROR, 1);
}

void SaveLoadFrontEnd::SetSaveProgress(void *userData, int val)
{
	if(val >= 0)
		((SaveLoadFrontEnd *)userData)->progressval = val/100.0f;
	if(val >= 100)
	{
#ifdef TARGET_PS2
    GenericGameSaver::inst()->releaseIconData();
#endif // TARGET_PS2
		((SaveLoadFrontEnd *)userData)->SetDState(DSTATE_SAVE_DONE);
		((SaveLoadFrontEnd *)userData)->progressval = 0;
	}
	if(val < 0)
  {
#ifdef TARGET_PS2
    GenericGameSaver::inst()->releaseIconData();
#endif // TARGET_PS2
		((SaveLoadFrontEnd *)userData)->StartError(ksGlobalTextArray[GT_FE_MENU_SAVE_FAIL], SE_SAVE_ERROR, 1);
  }
}


void SaveLoadFrontEnd::SelectLSD()
{
	int tmp = highlighted->entry_num;

  switch(tmp)
  {
  case SL_LOAD:
    SetDState(DSTATE_LOAD);
    break;
  case SL_SAVE:
    SetDState(DSTATE_SAVE);
    break;
  case SL_DELETE:
    SetDState(DSTATE_DELETE);
  }
}

void SaveLoadFrontEnd::SelectError()
{
	// ignore a select if there are no "yes/no" or "ok" buttons
	if(error_ok_yesno != 0)
	{
		switch(current_error)
		{
		case SE_NO_MEMORY_CARDS: break;
		case SE_UNFORMAT_1:
			break;
		case SE_UNFORMAT_2:
			break;
		case SE_SAVE_ERROR: EndError(SE_SAVE_ERROR); break;
		case SE_LOAD_ERROR: EndError(SE_LOAD_ERROR); break;
		default: assert(0);
		}
	}
}

void SaveLoadFrontEnd::Select()
{
	switch(dstate)
	{
	case DSTATE_LSD: SelectLSD(); break;
	case DSTATE_LOAD:
		if(NameMenu->AvailAndSavedGames(0) || NameMenu->AvailAndSavedGames(1))
		{
			cur_type = SL_LOAD;
			NameMenu->Select();
			SetDState(DSTATE_LOAD_PICK);
		}
		break;
	case DSTATE_LOAD_PICK:
		if(yes_active) LoadFile(NameMenu->ActiveCard(), NameMenu->ActiveFile());
		else Back();
		break;
	case DSTATE_LOADING: break;
	case DSTATE_LOAD_DONE: SetDState(DSTATE_LSD); break;
	case DSTATE_SAVE:
		cur_type = SL_SAVE;
		NameMenu->Select();
		break;
	case DSTATE_SAVE_NAME: KeyMenu->Select(); break;
	case DSTATE_SAVE_PICK:
		if(yes_active) SaveFile(NameMenu->ActiveCard(), true, NameMenu->ActiveFile());
		else Back();
		break;
	case DSTATE_SAVING: break;
	case DSTATE_SAVE_DONE: SetDState(DSTATE_LSD); break;
	case DSTATE_DELETE:
		if(NameMenu->AvailAndSavedGames(0) || NameMenu->AvailAndSavedGames(1))
		{
			cur_type = SL_DELETE;
			NameMenu->Select();
			if(!NameMenu->SelectingController())
        SetDState(DSTATE_DELETE_PICK);
		}
		break;
	case DSTATE_DELETE_PICK:
		if(yes_active) //DelFile(NameMenu->ActiveCard(), NameMenu->ActiveFile());
			SetDState(DSTATE_DELETING);
		else Back(); break;
	case DSTATE_DELETING: break;
	case DSTATE_DEL_DONE:
		okPQ->TurnOn(false);
		delDonePQ->TurnOn(false);
		SetDState(DSTATE_DELETE);
		break;
	case DSTATE_ERROR: SelectError(); break;
	default: assert(0);
	}
}

void SaveLoadFrontEnd::BackError()
{
	// almost exactly the same as a select, except no Formatting.
	switch(current_error)
	{
	case SE_NO_MEMORY_CARDS: next_state = DSTATE_LSD; break;
	case SE_SAVE_ERROR:
	case SE_LOAD_ERROR:
	default: assert(0);
	}
}

void SaveLoadFrontEnd::Back()
{
	FEMenuEntry* high;
	switch(dstate)
	{
	case DSTATE_LSD:		
    GenericGameSaver::inst()->ReleaseAll();
    manager->em->ExitState(); 
    sys->MakeActive(GraphicalMenuSystem::SurferMenu); 
    break;
	case DSTATE_LOAD_PICK:	SetDState(DSTATE_LOAD); break;
	case DSTATE_SAVE_NAME:	SetDState(DSTATE_SAVE); break;
	case DSTATE_SAVE_PICK:	SetDState(DSTATE_SAVE); break;
	case DSTATE_DELETE_PICK:SetDState(DSTATE_DELETE); break;
	case DSTATE_SAVING:		break;
	case DSTATE_LOADING:	break;
	case DSTATE_DELETING:	break;
	case DSTATE_SAVE_DONE:	SetDState(DSTATE_SAVE); break;
	case DSTATE_LOAD_DONE:	SetDState(DSTATE_LOAD); break;
	case DSTATE_DEL_DONE:	SetDState(DSTATE_DELETE); break;
	case DSTATE_ERROR:		BackError(); break;
	case DSTATE_LOAD:
	case DSTATE_SAVE:
	case DSTATE_DELETE:
		high = highlighted;
		SetDState(DSTATE_LSD);
		setHigh(high);		// reset highlighted to what it was before
		break;
	default: assert(0);
	}
}

void SaveLoadFrontEnd::OnActivate()
{
	SetDState(DSTATE_LSD, false);
	FEMultiMenu::OnActivate();
}

void SaveLoadFrontEnd::OnLeft(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if((dstate == DSTATE_ERROR && error_ok_yesno == 2) ||
		dstate == DSTATE_SAVE_PICK || dstate == DSTATE_LOAD_PICK ||
		dstate == DSTATE_DELETE_PICK)
	{
		TurnYesNo(true, false);
		yes_active = true;
	}
	else
	{
		if(dstate == DSTATE_LSD)
			FEMultiMenu::OnLeft(c);
		if(dstate == DSTATE_LOAD || dstate == DSTATE_SAVE || dstate == DSTATE_DELETE)
			NameMenu->OnLeft(c);
		if(dstate == DSTATE_SAVE_NAME)
			KeyMenu->OnLeft(c);
	}
}

void SaveLoadFrontEnd::OnRight(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if((dstate == DSTATE_ERROR && error_ok_yesno == 2) ||
		dstate == DSTATE_SAVE_PICK || dstate == DSTATE_LOAD_PICK ||
		dstate == DSTATE_DELETE_PICK)
	{
		TurnYesNo(false, true);
		yes_active = false;
	}
	else
	{
		if(dstate == DSTATE_LSD)
			FEMultiMenu::OnRight(c);
		if(dstate == DSTATE_LOAD || dstate == DSTATE_SAVE || dstate == DSTATE_DELETE)
			NameMenu->OnRight(c);
		if(dstate == DSTATE_SAVE_NAME)
			KeyMenu->OnRight(c);
	}
}

void SaveLoadFrontEnd::OnDown(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if(dstate == DSTATE_LOAD || dstate == DSTATE_SAVE || dstate == DSTATE_DELETE)
		NameMenu->OnDown(c);
	if(dstate == DSTATE_SAVE_NAME)
		KeyMenu->OnDown(c);
}

void SaveLoadFrontEnd::OnUp(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if(dstate == DSTATE_LOAD || dstate == DSTATE_SAVE || dstate == DSTATE_DELETE)
		NameMenu->OnUp(c);
	if(dstate == DSTATE_SAVE_NAME)
		KeyMenu->OnUp(c);
}

void SaveLoadFrontEnd::StartError(stringx text, int id, int yesno)
{
	error->changeText(text);
	// I put the reverse=true in here so that the bottom line
	// would be centered vertically
	error->makeBox(260, 50, true);
	switch(id)
	{
	case SE_NO_MEMORY_CARDS:
		if(dstate == DSTATE_LSD)
			next_state = (cur_type == SL_SAVE ? DSTATE_SAVE : (cur_type == SL_LOAD ? DSTATE_LOAD : DSTATE_DELETE));
		else next_state = dstate;
		break;
	case SE_UNFORMAT_1:
	case SE_UNFORMAT_2:
	case SE_SAVE_ERROR:		savingOn(false); next_state = DSTATE_SAVE; break;
	case SE_LOAD_ERROR:		loadingOn(false); next_state = DSTATE_LOAD; break;
	default: assert(0);
	}

	SetDState(DSTATE_ERROR);
	if(yesno == 2) TurnYesNo(true, false);
	if(yesno == 1) okPQ->TurnOn(true);
	yes_active = true;
	error_ok_yesno = yesno;
	current_error = id;
}

void SaveLoadFrontEnd::EndError(int id)
{
	if(current_error == id)
	{
		if(error_ok_yesno == 2)
		{
			yesPQ[0]->TurnOn(false);
			yesPQ[1]->TurnOn(false);
			noPQ[0]->TurnOn(false);
			noPQ[1]->TurnOn(false);
		}
		else if(error_ok_yesno == 1) okPQ->TurnOn(false);
		TurnDialog(false);
		if(dstate == DSTATE_ERROR)
			SetDState(next_state, true);
	}
}

void SaveLoadFrontEnd::SetPQIndices()
{
	int i;
	for(i=0; i<9; i++)
	{
		stringx tmp = stringx(i+1);
		memcard1[i] = GetPointer(("mem0_"+tmp).data());
		memcard2[i] = GetPointer(("mem1_"+tmp).data());
		savebg[i] = GetPointer(("save"+tmp).data());
		dialogPQ[i] = GetPointer(("dialog"+tmp).data());
		whitePQ[i] = GetPointer(("text"+tmp).data());
	}
	stringx tmp2 = "sl_btn_";
	for(i=0; i<2; i++)
	{
		stringx tmp = "white";
		if(i) tmp = "yellow";
		yesPQ[i] = GetPointer((tmp2+"yes_"+tmp).data());
		noPQ[i] = GetPointer((tmp2+"no_"+tmp).data());
		enterPQ[i] = GetPointer((tmp2+"enter_"+tmp).data());
		cancelPQ[i] = GetPointer((tmp2+"cancel_"+tmp).data());
		lsdPQ[0][i] = GetPointer((tmp2+"save_"+tmp).data());
		lsdPQ[1][i] = GetPointer((tmp2+"load_"+tmp).data());
		lsdPQ[2][i] = GetPointer((tmp2+"delete_"+tmp).data());
	}
	barPQ[0] = GetPointer("sl_bar_red");
	barPQ[1] = GetPointer("sl_bar_outline");
	barPQ[2] = GetPointer("sl_bar_background");
	delPQ = GetPointer("sl_deletegame");
	loadPQ = GetPointer("sl_loadgame");
	savePQ = GetPointer("sl_overwrite");
	deletingPQ = GetPointer("sl_deleting");
	savingPQ = GetPointer("sl_saving");
	loadingPQ = GetPointer("sl_loading");
	delDonePQ = GetPointer("sl_deletedone");
	loadDonePQ = GetPointer("sl_loadingdone");
	memName1PQ = GetPointer("sl_mmc1");
	memName2PQ = GetPointer("sl_mmc2");
	okPQ = GetPointer("sl_ok");
	saveDonePQ = GetPointer("sl_savingdone");
}

void SaveLoadFrontEnd::TurnEntCan(bool enter, bool cancel)
{
	enterPQ[0]->TurnOn(!enter);
	enterPQ[1]->TurnOn(enter);
	cancelPQ[0]->TurnOn(!cancel);
	cancelPQ[1]->TurnOn(cancel);
}

void SaveLoadFrontEnd::TurnYesNo(bool yes, bool no)
{
	yesPQ[0]->TurnOn(!yes);
	yesPQ[1]->TurnOn(yes);
	noPQ[0]->TurnOn(!no);
	noPQ[1]->TurnOn(no);
}

void SaveLoadFrontEnd::MemcardBGOn(bool on)
{
	for(int i=0; i<9; i++)
	{
		memcard1[i]->TurnOn(on);
		memcard2[i]->TurnOn(on);
	}
}

void SaveLoadFrontEnd::SaveBGOn(bool on)
{
	for(int i=0; i<9; i++)
		savebg[i]->TurnOn(on);
	TurnTextBox(on);
	enterPQ[0]->TurnOn(on);
	cancelPQ[0]->TurnOn(on);
	enterPQ[1]->TurnOn(false);
	cancelPQ[1]->TurnOn(false);
}

void SaveLoadFrontEnd::delBoxOn(bool on)
{
	TurnDialog(on);
	delPQ->TurnOn(on);
	noPQ[1]->TurnOn(on);
	yesPQ[0]->TurnOn(on);
	noPQ[0]->TurnOn(false);
	yesPQ[1]->TurnOn(false);
}

void SaveLoadFrontEnd::loadBoxOn(bool on)
{
	TurnDialog(on);
	loadPQ->TurnOn(on);
	noPQ[1]->TurnOn(on);
	yesPQ[0]->TurnOn(on);
	noPQ[0]->TurnOn(false);
	yesPQ[1]->TurnOn(false);
	yes_active = false;
}

void SaveLoadFrontEnd::saveBoxOn(bool on)
{
	TurnDialog(on);
	savePQ->TurnOn(on);
	noPQ[1]->TurnOn(on);
	yesPQ[0]->TurnOn(on);
	noPQ[0]->TurnOn(false);
	yesPQ[1]->TurnOn(false);
	yes_active = false;
}

void SaveLoadFrontEnd::TurnOffAll()
{
	MemcardBGOn(false);
	SaveBGOn(false);
	delBoxOn(false);
	loadPQ->TurnOn(false);
	savePQ->TurnOn(false);
	deletingPQ->TurnOn(false);
	savingPQ->TurnOn(false);
	loadingPQ->TurnOn(false);
	okPQ->TurnOn(false);
	saveDonePQ->TurnOn(false);
	loadDonePQ->TurnOn(false);
	delDonePQ->TurnOn(false);
	memName1PQ->TurnOn(false);
	memName2PQ->TurnOn(false);
	for(int i=0; i<2; i++)
	{
		enterPQ[i]->TurnOn(false);
		cancelPQ[i]->TurnOn(false);
	}
	TurnTextBox(false);
	TurnProgress(false);

	GetPointer("sl_formatting")->TurnOn(false);
	GetPointer("sl_formatdone")->TurnOn(false);
	GetPointer("hb_bar")->TurnOn(false);
	GetPointer("hb_back")->TurnOn(false);
//	GetPointer("hb_start")->TurnOn(false);
	GetPointer("hb_triangle")->TurnOn(false);
	GetPointer("hb_cross")->TurnOn(false);
	GetPointer("hb_select")->TurnOn(false);
	GetPointer("hb_select2")->TurnOn(false);
	GetPointer("hb_accept")->TurnOn(false);
	GetPointer("hb_vertical")->TurnOn(false);
	GetPointer("hb_horizontal")->TurnOn(false);
	GetPointer("hb_change")->TurnOn(false);
}

void SaveLoadFrontEnd::SetDState(int s, bool activate, bool end_error)
{
	bool is_already_error = (dstate == DSTATE_ERROR);// && !end_error);
	switch(s)
	{
	case DSTATE_LSD:
		TurnOffAll();
//		memName1PQ->TurnOn(false);
//		memName2PQ->TurnOn(false);
		if(activate) active = NULL; //MakeActive(NULL);
		break;
	case DSTATE_LOAD:
		if(dstate == DSTATE_LSD)
		{
			MakeActive(NameMenu);
//			memName1PQ->TurnOn(true);
//			memName2PQ->TurnOn(true);
			NameMenu->OnActivate(SL_LOAD);
			MemcardBGOn(true);
		}
		else loadBoxOn(false); break;
	case DSTATE_LOAD_PICK:	loadBoxOn(true); break;
	case DSTATE_LOADING:
		loadBoxOn(false);
		loadingOn(true);
		SetProgress(0); break;
	case DSTATE_LOAD_DONE:
		loadingOn(false);
		loadDonePQ->TurnOn(true);
		okPQ->TurnOn(true);
		TurnDialog(true); break;
	case DSTATE_SAVE:
		if(dstate == DSTATE_LSD)
		{
			MakeActive(NameMenu);
//			memName1PQ->TurnOn(true);
//			memName2PQ->TurnOn(true);
			NameMenu->OnActivate(SL_SAVE);
			MemcardBGOn(true);
		}
		else if(dstate == DSTATE_SAVE_PICK)
			saveBoxOn(false);
		else
		{
			MakeActive(NameMenu);
			active = NameMenu;	// so that NameMenu::OnActivate won't be called
			MemcardBGOn(true);
			SaveBGOn(false);
		} break;
	case DSTATE_SAVE_NAME:
		MemcardBGOn(false);
		SaveBGOn(true);
//		memName1PQ->TurnOn(false);
//		memName2PQ->TurnOn(false);
		MakeActive(KeyMenu);
		break;
	case DSTATE_SAVE_PICK:	saveBoxOn(true); break;
	case DSTATE_SAVING:
		saveBoxOn(false);
		savingOn(true);
		SetProgress(0); break;
	case DSTATE_SAVE_DONE:
		savingOn(false);
		saveDonePQ->TurnOn(true);
		okPQ->TurnOn(true);
		TurnDialog(true); break;
	case DSTATE_DELETE:
		if(dstate == DSTATE_LSD)
		{
			MakeActive(NameMenu);
//			memName1PQ->TurnOn(true);
//			memName2PQ->TurnOn(true);
			NameMenu->OnActivate(SL_DELETE);
			MemcardBGOn(true);
		}
		else delBoxOn(false); break;
	case DSTATE_DELETE_PICK: delBoxOn(true); break;
	case DSTATE_DELETING:
		progress_drawn = 0;
		delBoxOn(false);
		TurnDialog(true);
		deletingPQ->TurnOn(true);
		break;
	case DSTATE_DEL_DONE:
		delBoxOn(false);
		deletingPQ->TurnOn(false);
		delDonePQ->TurnOn(true);
		okPQ->TurnOn(true);
		TurnDialog(true); break;
	case DSTATE_ERROR:
		if(dstate == DSTATE_SAVE_PICK)
			saveBoxOn(false);
		if(dstate == DSTATE_LOAD_PICK)
			loadBoxOn(false);
		if(dstate == DSTATE_DELETE_PICK)
			delBoxOn(false);
		TurnDialog(true);
		break;
	default: assert(0);
	}

	// if the current state is error, but the state wasn't error when the
	// function started, then an error was just thrown, and we shouldn't change
	// the state just yet.
	if(dstate != DSTATE_ERROR || is_already_error)
		dstate = s;
}

void SaveLoadFrontEnd::LoadFile(int card, int filenum)
{
	assert((fileType == 0) && (numGameConfig[card] > 0));
	delete[] thedata;
	thedata =  NEW char[StoredConfigData::inst()->getSize()];

	GenericGameSaver::inst()->setFileInfo(savedConfigs[card][filenum]);

	GenericGameSaver::inst()->readFile(NameMenu->GetPort(), card, (void*)g_career,
    sizeof(*g_career),
		(SaveLoadFrontEnd::SetLoadProgress), (void *)this);
  if(dstate != DSTATE_ERROR)
  	SetDState(DSTATE_LOADING);
}

void SaveLoadFrontEnd::SaveFile(int card, bool overwrite, int entry)
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

	if(overwrite)
	{
		fileDef = savedConfigs[card][entry];
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
	GenericGameSaver::inst()->setFileInfo(fileDef);

    g_career->cfg = *(ksConfigData*)(StoredConfigData::inst()->getGameConfig());

    if (fileType == 1)
		;// GenericGameSaver::inst()->saveFile(0,0,(void *)thedata, 6, true, (SaveLoadFrontEnd::SetSaveProgress), (void *)this);
    else
    {
#ifdef TARGET_PS2
      GenericGameSaver::inst()->getIconData();
#endif // TARGET_PS2
		  GenericGameSaver::inst()->saveFile(NameMenu->GetPort(), card,(void *)g_career,
                                          sizeof(*g_career), true,
                                          (SaveLoadFrontEnd::SetSaveProgress), (void *)this);
    }
	SetDState(DSTATE_SAVING);
}

void SaveLoadFrontEnd::DelFile(int card, int filenum)
{
	int ret = GenericGameSaver::inst()->deleteFile(NameMenu->GetPort(), card, SavedData[card][filenum]);
	if(ret != 0)
		nglPrintf("BETH: bad delete return: %d\n", ret);
	SetDState(DSTATE_DEL_DONE);
	NameMenu->Refresh();
}

/*********************** Names Menu ************************/

NamesMenu::NamesMenu(FEMenuSystem* s)
{
	cons(s, s->manager->bio_font_blue, s->manager->bio_font_red, .75, .95);
	dy = 25;  // down from normal 28 in FEMenu

	for(int i=0; i<num; i++)
	{
		FEMenuEntry* tmp = NEW FEMenuEntry(stringx("- ") + ksGlobalTextArray[GT_FE_MENU_BLANK] + stringx(" -"), this);
		tmp->SetZ(440);
		if(i == 0) first = tmp;
		if(i == num_2-1) lastm[0] = tmp;
		if(i == num_2) firstm[1] = tmp;
		if(i == num-1) last = tmp;
		Add(tmp);
		empty[i] = true;
	}
	firstm[0] = first;
	lastm[1] = last;

	FEMenuEntry* tmp = first;
	for(int i=0; i<num; i++)
	{
		if(tmp == lastm[0])			tmp->down = firstm[0];
		else if(tmp == lastm[1])	tmp->down = firstm[1];
		else						tmp->down = tmp->next;
		if(tmp == firstm[0])		tmp->up = lastm[0];
		else if(tmp == firstm[1])	tmp->up = lastm[1];
		else						tmp->up = tmp->previous;
		tmp->right = NULL;
		tmp->left = NULL;
		tmp = tmp->next;
	}
	center_x1 = 190;	// center of the 1st memory card
	center_y1 = 241;
	center_x2 = 450;	// center of the 2nd memory card
	center_y2 = 241;

  controller_select[0] = NEW FEMenuEntry(ksGlobalTextArray[GT_HARDDRIVE_XBOX], this);
  Add(controller_select[0]);
  for(i = 1; i <= NUM_MEMORY_PORTS + 1; i++)
  {
    char item_name[20];
    sprintf(item_name, "%s %d", ksGlobalTextArray[GT_FE_MENU_XBOX_MEM_CARD_1].c_str(), i);
    controller_select[i] = NEW FEMenuEntry(item_name, this);
    Add(controller_select[i]);
  }

  for(i = 0; i < NUM_MEMORY_PORTS + 1; i++)
  {
    controller_select[i]->SetPos(110 + i * 105, 410);
    controller_select[i]->right = controller_select[(i + 1) % (NUM_MEMORY_PORTS + 1)];
    controller_select[i]->left = controller_select[(i + 4) % (NUM_MEMORY_PORTS + 1)];
    controller_select[i]->up = lastm[0];
    controller_select[i]->down = firstm[0];
  }

	mem_card_avail[0] = false;
	mem_card_avail[1] = false;
	mc_unformat[0] = false;
	mc_unformat[1] = false;

	CardName[0] = NEW TextString(&system->manager->body, "", center_x1, 110, 0, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(0, 0, 255));
	CardName[1] = NEW TextString(&system->manager->body, "", center_x2, 110, 0, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(0, 0, 255));

	check_cards_counter = 0;
/*
	mem_empty[0] = NEW TextString(&system->manager->bio_font_new, ksGlobalTextArray[GT_FE_MENU_NO_SAVED_GAMES], center_x1, center_y1, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	mem_empty[1] = NEW TextString(&system->manager->bio_font_new, ksGlobalTextArray[GT_FE_MENU_NO_SAVED_GAMES], center_x2, center_y2, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unavail[0] = NEW TextString(&system->manager->bio_font_new, "", center_x1, center_y1, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unavail[1] = NEW TextString(&system->manager->bio_font_new, "", center_x2, center_y2, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unformat[0] = NEW TextString(&system->manager->bio_font_new, ksGlobalTextArray[GT_FE_MENU_UNFORMATTED], center_x1, center_y1, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unformat[1] = NEW TextString(&system->manager->bio_font_new, ksGlobalTextArray[GT_FE_MENU_UNFORMATTED], center_x2, center_y2, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
*/
	mem_empty[0] = NEW TextString(&system->manager->bold, ksGlobalTextArray[GT_FE_MENU_NO_SAVED_GAMES], center_x1, center_y1, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	mem_empty[1] = NEW TextString(&system->manager->bold, ksGlobalTextArray[GT_FE_MENU_NO_SAVED_GAMES], center_x2, center_y2, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unavail[0] = NEW TextString(&system->manager->bold, "", center_x1, center_y1, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unavail[1] = NEW TextString(&system->manager->bold, "", center_x2, center_y2, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unformat[0] = NEW TextString(&system->manager->bold, ksGlobalTextArray[GT_FE_MENU_UNFORMATTED], center_x1, center_y1, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	unformat[1] = NEW TextString(&system->manager->bold, ksGlobalTextArray[GT_FE_MENU_UNFORMATTED], center_x2, center_y2, 450, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);

  sl_parent = NULL;
  SetSavePort(0);
  selecting_controller = false;
  card_active = 0;
}

NamesMenu::~NamesMenu()
{
	for(int i=0; i<2; i++)
	{
		delete mem_empty[i];
//		delete mem_card[i];
		delete unavail[i];
		delete unformat[i];
    delete CardName[i];
	}
}

void NamesMenu::Init()
{
	half = (dy*(num_2-1))/2;
	FEMenuEntry* tmp1 = firstm[0];
	FEMenuEntry* tmp2 = firstm[1];
	int y1, y2;
	for(int i=0; i<num_2; i++)
	{
		// the 13 is to account for the "Memory Card 1" shift
		y1 = center_y1 + 13 + tmp1->entry_num * dy - half;
		y2 = center_y2 + 13 + (tmp2->entry_num-num_2) * dy - half;
		tmp1->SetPos(center_x1, y1);
		tmp2->SetPos(center_x2, y2);
		tmp1 = tmp1->next;
		tmp2 = tmp2->next;
	}
	sl_parent = (SaveLoadFrontEnd*) parent;
}

void NamesMenu::Update(time_value_t time_inc)
{
	if(!sl_parent->Busy())
	{
		bool before[2];
		before[0] = mem_card_avail[0];
		before[1] = mem_card_avail[1];

		// execute CheckCards only every 10 times, because it's slow
		check_cards_counter++;
		if(check_cards_counter >= 10)
		{
			CheckCards();
			check_cards_counter = 0;
		}

		// if both memory cards are not inserted
/*		if((before[0] || before[1]) && !mem_card_avail[0] && !mem_card_avail[1])
			sl_parent->StartError(INVALID TEXT REMOVED (dc 07/02/02), SE_NO_MEMORY_CARDS);
		if(!before[0] && !before[1] && (mem_card_avail[0] || mem_card_avail[1]))
			sl_parent->EndError(SE_NO_MEMORY_CARDS);*/

		// check for recent card insertions, and get a new listing
		for(int i=0; i<2; i++)
			if(!before[i] && mem_card_avail[i] && !mc_unformat[i])
					GetFileList(i);
	}
}

void NamesMenu::Draw()
{
	for(int i=0; i<2; i++)
	{
		CardName[i]->Draw();
		if(!mem_card_avail[i]) unavail[i]->Draw();
		else if(mc_unformat[i]) unformat[i]->Draw();
		else if(!save && all_empty[i]) mem_empty[i]->Draw();
		else
		{
			FEMenuEntry* tmp = firstm[i];
			for(int i=0; i<num_2; i++)
			{
				tmp->Draw();
				tmp = tmp->next;
			}
		}
	}

  for(i = 0; i < NUM_MEMORY_PORTS + 1; i++)
  {
    controller_select[i]->Draw();
  }
}

void NamesMenu::Select()
{
  if(selecting_controller)
  {
    OnDown(0);
    return;
  }
	if(!save && !AvailAndSavedGames(0) && !AvailAndSavedGames(1))
    return;
	if(save && card_active != -1 && empty[highlighted->entry_num])
		sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE_NAME);
	else
	{
		strcpy(sl_parent->desc, highlighted->GetText().data());
		if(save) sl_parent->SetDState(SaveLoadFrontEnd::DSTATE_SAVE_PICK);
	}
}

void NamesMenu::SetSavePort(int port)
{
  current_port = port - 1;

  lastm[0]->down = controller_select[port];
  lastm[1]->down = controller_select[port];
  firstm[0]->up  = controller_select[port];
  firstm[1]->up  = controller_select[port];

  if(!save && !AvailAndSavedGames(0) && !AvailAndSavedGames(1))
  {
    controller_select[port]->up = NULL;
    controller_select[port]->down = NULL;
  }
}

void NamesMenu::UpdateControllerSelect()
{
  if(highlighted == controller_select[0] ||
     highlighted == controller_select[1] ||
     highlighted == controller_select[2] ||
     highlighted == controller_select[3] ||
     highlighted == controller_select[4])
  {
    selecting_controller = true;
    SetSavePort(highlighted->entry_num - controller_select[0]->entry_num);
  }
  else
    selecting_controller = false;

  if(sl_parent)
    Refresh();
}

void NamesMenu::OnDown(int c)
{
	FETextMultiMenu::OnDown(c);

  UpdateControllerSelect();
}

void NamesMenu::OnUp(int c)
{
	FETextMultiMenu::OnUp(c);

  UpdateControllerSelect();
}

void NamesMenu::OnLeft(int c)
{
  if(!selecting_controller)
    SwitchCards(true);
  else
  {
    FETextMultiMenu::OnLeft(c);
  }

  UpdateControllerSelect();
}

void NamesMenu::OnRight(int c)
{
  if(!selecting_controller)
    SwitchCards(false);
  else
    FETextMultiMenu::OnRight(c);    

  UpdateControllerSelect();
}

void NamesMenu::SwitchCards(bool left)
{
	if(card_active != 1 && !left && ((Available(1) && save) || AvailAndSavedGames(1)))
	{
	  if(!selecting_controller)
    	setHigh(firstm[1]);
		card_active = 1;
    for(int i = 0; i < NUM_MEMORY_PORTS + 1; i++)
    {
      controller_select[i]->up = lastm[1];
      controller_select[i]->down = firstm[1];
    }
	}
	else if(card_active != 0 && left && ((Available(0) && save) || AvailAndSavedGames(0)))
	{
	  if(!selecting_controller)
  		setHigh(firstm[0]);
		card_active = 0;
    for(int i = 0; i < NUM_MEMORY_PORTS + 1; i++)
    {
      controller_select[i]->up = lastm[0];
      controller_select[i]->down = firstm[0];
    }
	}
}

void NamesMenu::OnActivate(int ty)
{
	save = (ty == SaveLoadFrontEnd::SL_SAVE);
	card_active = 0;
  selecting_controller = false;
	FETextMultiMenu::OnActivate();
	Refresh();
  if(!save && !AvailAndSavedGames(0) && !AvailAndSavedGames(1))
    setHigh(controller_select[current_port + 1]);
}

void NamesMenu::Refresh()
{
	CheckCards();
	if(mem_card_avail[0])
		if(!mc_unformat[0])
			GetFileList(0);
	if(mem_card_avail[1])
		if(!mc_unformat[1])
			GetFileList(1);

  if(highlighted->GetDisable())
  {
    // If the currently highlighted menu item is actually disabled then we
    // need to switch to the nearest item.  If we're currently selecting
    // a controller, switch ot another controller.  Otherwise, first try to
    // switch to an existing save name and then if that doesn't work try to
    // switch to the current controller.
    if(!selecting_controller)
    {
      // if this card isn't empty or if both cards are empty
      if(!all_empty[card_active] || (all_empty[card_active] && all_empty[card_active ^ 1]))
      {
        if(highlighted != firstm[card_active])
          OnUp(0);
        else
          OnDown(0);
      }
      else // the other card isn't empty
      {
        if(card_active == 0)
          SwitchCards(false);
        else
          SwitchCards(true);
      }
    }
    else // no card is active.  A controller was disabled?
      // this should never happen currently
      OnLeft(0);
  }
/*	if(!save)
	{
		if((card_active != 0) && AvailAndSavedGames(0) && !AvailAndSavedGames(1))
		{
			setHigh(first);
			card_active = 0;
		}
		if((card_active != 1) && AvailAndSavedGames(1) && !AvailAndSavedGames(0))
		{
			setHigh(firstm[1]);
			card_active = 1;
		}
		if((card_active != -1) && !AvailAndSavedGames(0) && !AvailAndSavedGames(1))
			card_active = -1;
	}
	if(!save && card_active != -1 && empty[highlighted->entry_num])
		setHigh(firstm[card_active]);
	if(save && card_active == -1)
		sl_parent->StartError(INVALID TEXT REMOVED (dc 07/02/02), SE_NO_MEMORY_CARDS);*/
}

void NamesMenu::GetFileList(int card)
{
  sl_parent->getFileListing(current_port, card);

	// determine which entries are empty, and change the labels of those
	// that aren't empty
	FEMenuEntry* tmp = firstm[card];
	for(int i=0; i<num_2; i++)
	{
		if(i < sl_parent->numGameConfig[card])
    {
      if (strlen(sl_parent->savedConfigs[card][i].desc) > 14)
      {
        char name[13];
        strncpy(name, sl_parent->savedConfigs[card][i].desc, 9);
        name[9] = '.';name[10] = '.'; name[11] = '.';
        name[12]= '\0';
        tmp->SetText(name);
      }
      else
			  tmp->SetText(sl_parent->savedConfigs[card][i].desc);
    }
		else tmp->SetText(stringx("- ") + ksGlobalTextArray[GT_FE_MENU_BLANK] + stringx(" -"));
		empty[i+card*num_2] = (i >= sl_parent->numGameConfig[card]);
		tmp = tmp->next;
	}
	all_empty[card] = true;

	tmp = firstm[card];
	for(int i=0; i<num_2; i++)
	{
		// disable if in load or delete mode; enable if save mode
		if(empty[i+card*num_2]) tmp->Disable(!save);
		else
		{
			tmp->Disable(false);		// in case they were ever previously disabled
			all_empty[card] = false;
		}
		tmp = tmp->next;
	}
}

void NamesMenu::CheckCards()
{
	int type, free, formatted;
	char card_name[MAX_MUNAME];
	
	for(int i=0; i<2; i++)
	{
		int ret = GenericGameSaver::getInfo(current_port, i, &type, &free, &formatted, card_name);
		switch(ret)
		{
		case GSOk:
			mem_card_avail[i] = true;
			mc_unformat[i] = false;
			if(card_name[0] == 0)
			{
				// put in a default name based on the card's location
				sprintf(card_name, ksGlobalTextArray[GT_MEMORY_CARD_XBOX].c_str(), current_port + 1, ((char)i) + 'A');
			}
			// Make sure the card name isn't too long
			if(strlen(card_name) > MAX_CARDNAME)
			{
				card_name[MAX_CARDNAME] = card_name[MAX_CARDNAME + 1] = card_name[MAX_CARDNAME + 2] = '.';
				card_name[MAX_CARDNAME + 3] = 0;
			}
			CardName[i]->changeText(card_name);
			break;
		case GSErrorUnformatted:
			mem_card_avail[i] = true;
			mc_unformat[i] = true;
			break;
		default:
			strcpy(card_name, ksGlobalTextArray[GT_MEMORY_NO_CARD_XBOX].c_str());
			CardName[i]->changeText(card_name);
			mem_card_avail[i] = false;
			break;
		}
	}
	
	// checks to see if one card is unavailable, if so, set the other card active
	if((card_active != 0) && Available(0) && !Available(1))
	{
    SwitchCards(true);
	}
	if((card_active != 1) && Available(1) && !Available(0))
	{
    SwitchCards(false);
	}
	if((card_active == -1) && ((Available(1) && Available(0) && save) ||
                            (AvailAndSavedGames(1) && AvailAndSavedGames(0) && !save)))
	{
    card_active = 0;
    for(int i = 0; i < NUM_MEMORY_PORTS + 1; i++)
    {
      controller_select[i]->up = lastm[card_active];
      controller_select[i]->down = firstm[card_active];
    }
	}
	if((card_active == -1) && AvailAndSavedGames(1) && AvailAndSavedGames(0) && !save)
	{
    card_active = 0;
    for(int i = 0; i < NUM_MEMORY_PORTS + 1; i++)
    {
      controller_select[i]->up = lastm[card_active];
      controller_select[i]->down = firstm[card_active];
    }
	}
	if((card_active != -1) && !Available(0) && !Available(1))
  {
		card_active = -1;
    for(i = 0; i < NUM_MEMORY_PORTS + 1; i++)
    {
      controller_select[i]->up = NULL;
      controller_select[i]->down = NULL;
    }
  }
}

bool NamesMenu::Available(int card)
{
	if(mem_card_avail[card] && !mc_unformat[card])
		return true;
	else return false;
}

// only returns true on delete & load if there are saved games
bool NamesMenu::AvailAndSavedGames(int card)
{
	if(mem_card_avail[card] && !mc_unformat[card])
		return (save || !all_empty[card]);
	else return false;
}

/*********************** Keyboard Menu ************************/

KeyboardMenu::KeyboardMenu(FEMenuSystem* s)
{
	cons(s, s->manager->bio_font_blue, s->manager->bio_font_red, .75, .95);

	char name[2];
	name[1] = '\0';
	for(int i=0; i<26; i++)
	{
		name[0] = 'A'+i;
		ent[i] = NEW FEMenuEntry(name, this);
	}
	for(int i=0; i<10; i++)
	{
		name[0] = '0'+i;
		ent[i+26] = NEW FEMenuEntry(name, this);
	}
	ent[36] = NEW FEMenuEntry(ksGlobalTextArray[GT_SPACE], this);
	ent[37] = NEW FEMenuEntry(ksGlobalTextArray[GT_BACKSPACE], this);
	ent[38] = NEW FEMenuEntry(ksGlobalTextArray[GT_CLEAR], this);
	first = ent[0];
	last = ent[38];

	for(int i=0; i<num; i++)
	{
		Add(ent[i]);
		ent[i]->SetZ(300);
	}

	for(int i=0; i<num; i++)
	{
		if(i == num-1)		ent[i]->right = first;
		else				ent[i]->right = ent[i]->next;
		if(i == 0)			ent[i]->left = last;
		else				ent[i]->left = ent[i]->previous;
		if(i <= 13)			ent[i]->down = ent[i+13];
		else if(i <= 19)	ent[i]->down = ent[i+12];
		else if(i <= 24)	ent[i]->down = ent[i+11];
		else if(i == 25)	ent[i]->down = ent[i+10];
		else if(i <= 28)	ent[i]->down = ent[36];
		else if(i <= 32)	ent[i]->down = ent[37];
		else if(i <= 35)	ent[i]->down = ent[38];
		else				ent[i]->down = NULL;
		if(i <= 12)			ent[i]->up = NULL;
		else if(i <= 25)	ent[i]->up = ent[i-13];
		else if(i <= 31)	ent[i]->up = ent[i-12];
		else if(i <= 35)	ent[i]->up = ent[i-11];
		else if(i == 36)	ent[i]->up = ent[27];
		else if(i == 37)	ent[i]->up = ent[30];
		else				ent[i]->up = ent[34];
	}
	center_x = 320;
	center_y = 240;

//	title = NEW TextString(&system->manager->bio_font_new, ksGlobalTextArray[GT_FE_MENU_NO_FILENAME_XBOX], 320, 120, 300, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
//	filename = NEW TextString(&system->manager->bio_font_new, "", 320, 308, 300, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(255, 255, 255, 255));
	title = NEW TextString(&system->manager->bold, ksGlobalTextArray[GT_FE_MENU_NO_FILENAME_XBOX], 320, 120, 300, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->bio_font_blue);
	filename = NEW TextString(&system->manager->bold, "", 320, 308, 300, .8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(255, 255, 255, 255));
}

KeyboardMenu::~KeyboardMenu()
{
	delete title;
	delete filename;
}

void KeyboardMenu::Init()
{
	// half = (dy*(num-1))/2;
	dy = 36;
	int dx = 37;
	int half_y = (dy*4)/2;
	int half_x = (dx*12)/2;
	int x, y;
	for(int i=0; i<13; i++)
	{
		x = center_x + i * dx - half_x;
		y = center_y - half_y;
		ent[i]->SetPos(x, y);
	}
	for(int i=13; i<26; i++)
	{
		x = center_x + (i-13) * dx - half_x;
		y = center_y + dy - half_y;
		ent[i]->SetPos(x, y);
	}
	for(int i=26; i<36; i++)
	{
		x = center_x + (i-26) * dx - (dx*9)/2;
		y = center_y +2*dy - half_y;
		ent[i]->SetPos(x, y);
	}
	for(int i=36; i<39; i++)
	{
		x = center_x + (i-36) * 140 - 140;
		y = center_y + 3*dy - half_y;
		ent[i]->SetPos(x, y);
	}
	sl_parent = (SaveLoadFrontEnd*) parent;
}

void KeyboardMenu::Draw()
{
	title->Draw();
	filename->Draw();
	FETextMultiMenu::Draw();
}

void KeyboardMenu::Select()
{
	if(!ent_can_active)
	{
		int n = highlighted->entry_num;
		if(current_place < name_size || n == 37 || n == 38)
		{
			if(n < 26) name[current_place] = 'A'+n;
			else if(n < 36) name[current_place] = '0'+(n-26);
			else if(n == 36) name[current_place] = ' ';
			else if(n == 37) current_place = current_place-2;
			else current_place = -1;
			current_place++;
			name[current_place] = '\0';
			filename->changeText(name);
		}
	}
	else if(enter_active) 
    sl_parent->SaveFile(name);
	else 
    sl_parent->Back();
}

void KeyboardMenu::OnDown(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if(!ent_can_active)
	{
		if(highlighted == ent[36] || highlighted == ent[37] || highlighted == ent[38])
		{
			ent_can_active = true;
			enter_active = true;
			sl_parent->TurnEntCan(true, false);
		}
		else FETextMultiMenu::OnDown(c);
	}
}

void KeyboardMenu::OnUp(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if(ent_can_active)
	{
		ent_can_active = false;
		sl_parent->TurnEntCan(false, false);
	}
	else FETextMultiMenu::OnUp(c);
}

void KeyboardMenu::OnLeft(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if(ent_can_active)
	{
		sl_parent->TurnEntCan(true, false);
		enter_active = true;
	}
	else FETextMultiMenu::OnLeft(c);
}

void KeyboardMenu::OnRight(int c)
{
	if(c != input_mgr::inst()->GetDefaultController()) return;
	if(ent_can_active)
	{
		sl_parent->TurnEntCan(false, true);
		enter_active = false;
	}
	else FETextMultiMenu::OnRight(c);
}

void KeyboardMenu::OnActivate()
{
	name[0] = '\0';
	filename->changeText(name);
	current_place = 0;
	ent_can_active = false;
	FETextMultiMenu::OnActivate();
}





