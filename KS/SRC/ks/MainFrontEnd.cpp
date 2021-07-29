// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX)
#include "refptr.h"
#include "hwrasterize.h"
#include "FrontEndManager.h"
#include "osdevopts.h"
#endif // TARGET_XBOX JIV DEBUG

// MainFrontEnd.cpp
#include <string.h>
#include "MainFrontEnd.h"
#include "VOEngine.h"
#include "rumbleManager.h"
#include "inputmgr.h"

#define RUMBLETIME .15f

extern game* g_game_ptr;

MainFrontEnd::MainFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;
	flags |= FEMENU_1_ENTRY_SHOWN | FEMENU_DONT_SKIP_DISABLED | FEMENU_HAS_COLOR_HIGH_ALT;
	manager = man;

#if 0
	// please don't delete this, it is used for formatting the bios
	stringx tmp_name_before = "";
	stringx tmp_name_after = "";
	nglFileBuf F;
	os_file out;
	for(int i=0; i<SURFER_LAST; i++)
	{
		tmp_name_after = stringx("bios\\") + SurferDataArray[i].abbr + "_bio";
		tmp_name_before = tmp_name_after + "_before";
		for(int j=0; j<3; j++)
		{
			stringx tmp = ".txt";
			if(j == 1) tmp = "_fr" + tmp;
			else if(j == 2) tmp = "_ge" + tmp;

			if(!file_finder_exists((tmp_name_before+tmp).data(), ""))
				continue;

			KSReadFile((tmp_name_before+tmp).data(), &F, 1);
			char *buffer = new char[(int)(F.Size*3)];
			if(!buffer) continue;
			FormatBIO(&frontendmanager.font_body, 190,(char *)F.Buf, F.Size, buffer, (int)(F.Size*3), 1.0f);
			out.open((tmp_name_after+tmp), os_file::FILE_WRITE);
			out.write(buffer, strlen(buffer));
			out.close();
			delete[] buffer;
			KSReleaseFile(&F);

			nglPrintf("read file %s\n", tmp_name_before.data());
		}
	}
#endif



	color = manager->col_unselected;
	color_high = manager->col_highlight2;
	color_high_alt = manager->col_highlight;
	scale = 0.78f;
	Options = NEW OptionsMenu(system, manager, p, "options.PANEL");
	Multi = NEW MultiplayerMenu(system, 479, 279);
	Freesurf = NEW FreesurfMenu(system, 295, 161);
	career_menu = NEW CareerMenu(system, 185, 191);
	multi_sub = NEW MultiSubMenu(system, 479, 279);
	AddSubmenu(Multi);
	AddSubmenu(Options);
	AddSubmenu(Freesurf);
	AddSubmenu(career_menu);
	AddSubmenu(multi_sub);

	entry_list[MainCareerEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_CAREER], this, false, &manager->font_bold);
	entry_list[MainFreeEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_FREESURF], this, false, &manager->font_bold);
	entry_list[MainMultiEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_MULTI], this, false, &manager->font_bold);
	entry_list[MainExtrasEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_EXTRAS], this, false, &manager->font_bold);
	entry_list[MainOpEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OPTIONS], this, false, &manager->font_bold);
	entry_list[MainCareerEntry]->SetPos(185, 145);
	entry_list[MainFreeEntry]->SetPos(295, 115);
	entry_list[MainMultiEntry]->SetPos(479, 233);
	entry_list[MainExtrasEntry]->SetPos(346, 156);
	entry_list[MainOpEntry]->SetPos(368, 216);

	for(int i=0; i<num_entries; i++)
	{
		Add(entry_list[i]);
		if(i < num_entries-1)	entry_list[i]->right = entry_list[i+1];
		else					entry_list[i]->right = entry_list[0];
		if(i != 0)				entry_list[i]->left = entry_list[i-1];
		else					entry_list[i]->left = entry_list[num_entries-1];
	}
	returnToHighlighted = -1;
}

MainFrontEnd::~MainFrontEnd()
{
	delete Options;
	delete Multi;
	delete Freesurf;
	delete career_menu;
	delete multi_sub;
}

void MainFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();

	stringx names[num_entries];
	names[MainCareerEntry] = "W1_CAREER";
	names[MainFreeEntry] = "W1_FREESURF";
	names[MainMultiEntry] = "W1_MULTIPLAYER";
	names[MainExtrasEntry] = "W1_EXTRAS";
	names[MainOpEntry] = "W1_OPTIONS";

	color32 color_high = manager->white;
	color32 color_norm = color32(180, 180, 180, 255);

	FEMenuEntry* tmp = entries;
	int i=0;
	while(tmp)
	{
		ents[i] = entity_manager::inst()->find_entity(entity_id(names[i].c_str()), IGNORE_FLAVOR);
		tmp->AddEntity(ents[i], color_high, color_norm);
		tmp = tmp->next;
		i++;
	}
	Options->Load();
	Multi->Load();
	Freesurf->Load();
	career_menu->Load();
	multi_sub->Load();
}

void MainFrontEnd::SetPQIndices()
{
	stringx tmp[MainEnd];
	tmp[MainFreeEntry] = "freesurf";
	tmp[MainCareerEntry] = "career";
	tmp[MainMultiEntry] = "multi";
	tmp[MainExtrasEntry] = "extras";
	tmp[MainOpEntry] = "option";

	for(int i=0; i<MainEnd; i++)
	{
		for(int j=0; j<num_boxes; j++)
			boxes[i][j] = GetPointer((tmp[i]+"_box_0"+stringx(j+1)).data());
		for(int j=0; j<num_circles; j++)
			circles[i][j] = GetPointer((tmp[i]+"_circle_0"+stringx(j+1)).data());
		lines[i][0] = GetPointer((tmp[i]+"_line_a").data());
		lines[i][1] = GetPointer((tmp[i]+"_line_b").data());
		lines[i][2] = GetPointer((tmp[i]+"_line_c").data());
		if(lines[i][2] == frontendmanager.GetDefaultPQ()) lines[i][2] = NULL;

		if(i == MainCareerEntry)
		{
			float xa, ya, xb, yb;
			boxes[i][4]->GetPos(xa, ya, xb, yb);
			career_menu->SetBoxBottom((int) yb);
		}
	}
}

void MainFrontEnd::ResizeCareerBox(int h)
{
	float xa, ya, xb, yb;
	for(int i=3; i<num_boxes; i++)
	{
		boxes[MainCareerEntry][i]->GetPos(xa, ya, xb, yb);
		if(i < 6) boxes[MainCareerEntry][i]->SetPos(xa, ya, xb, h);
		else boxes[MainCareerEntry][i]->SetPos(xa, h, xb, h+(yb-ya));
	}
}

void MainFrontEnd::Init()
{
	Options->Init();
	Multi->Init();
	Freesurf->Init();
}

void MainFrontEnd::Update(time_value_t time_inc)
{
	if(active) active->Update(time_inc);
	else
	{
		FEMultiMenu::Update(time_inc);
		switch(highlighted->entry_num)
		{
		case MainFreeEntry: Freesurf->Update(time_inc); break;
		case MainCareerEntry: career_menu->Update(time_inc); break;
		case MainMultiEntry: Multi->Update(time_inc); break;
		}
	}
}

void MainFrontEnd::UpdateInScene()
{
	if(active)
		active->UpdateInScene();
}

void MainFrontEnd::Draw()
{
	if(manager->em->InFlyin() || !manager->em->OKtoDrawMain()) return;
	if(active)
	{
		active->Draw();
		if(active == multi_sub)
			panel.Draw(0);
	}
	else
	{
		panel.Draw(0);
		// draw only highlighted entry
		highlighted->Draw();

		if(highlighted->entry_num == MainFreeEntry)
			Freesurf->Draw();
		else if(highlighted->entry_num == MainCareerEntry)
			career_menu->Draw();
		else if(highlighted->entry_num == MainMultiEntry)
			Multi->Draw();
	}
}

void MainFrontEnd::OnDown(int c)
{
	if(active) active->OnDown(c);
	else
	{
		switch(highlighted->entry_num)
		{
		case MainFreeEntry:		Freesurf->OnDown(c); break;
		case MainCareerEntry:	career_menu->OnDown(c); break;
		case MainMultiEntry:	Multi->OnDown(c); break;
		}
		if(highlighted->entry_num == MainFreeEntry)
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);	

	}
}

void MainFrontEnd::OnUp(int c)
{
	if(active) active->OnUp(c);
	else
	{
		switch(highlighted->entry_num)
		{
		case MainFreeEntry:		Freesurf->OnUp(c); break;
		case MainCareerEntry:	career_menu->OnUp(c); break;
		case MainMultiEntry:	Multi->OnUp(c); break;
		}
		if(highlighted->entry_num == MainFreeEntry)
		{
			if(highlighted->entry_num == MainFreeEntry)
				SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);	
		}
	}
}

void MainFrontEnd::OnLeft(int c)
{
	if(active) active->OnLeft(c);
	else 
	{
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		Left();
		UpdateHighlight();
	}
}

void MainFrontEnd::OnRight(int c)
{
	if(active) active->OnRight(c);
	else
	{
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		Right();
		UpdateHighlight();
	}
}

void MainFrontEnd::UpdateHighlight()
{
	if(highlighted->entry_num == MainFreeEntry)
		Freesurf->OnActivate();
	else if(highlighted->entry_num == MainCareerEntry)
		career_menu->OnActivate();
	else if(highlighted->entry_num == MainMultiEntry)
		Multi->OnActivate();

	for(int i=0; i<MainEnd; i++)
	{
		for(int j=0; j<num_boxes; j++)
			boxes[i][j]->TurnOn(highlighted->entry_num == i);
		for(int j=0; j<num_circles; j++)
			circles[i][j]->TurnOn(highlighted->entry_num == i);
		for(int j=0; j<num_lines; j++)
			if(lines[i][j]) lines[i][j]->TurnOn(highlighted->entry_num == i);
	}

	// update helpbar here, because it changes depending on highlighted entry
	manager->helpbar->Reset();
	manager->helpbar->RemoveTriangle();
	if(highlighted->entry_num == MainExtrasEntry || highlighted->entry_num == MainOpEntry)
		manager->helpbar->AddArrowH();
	manager->helpbar->Reformat();
}

void MainFrontEnd::OnTriangle(int c)
{
	//SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	if(active) active->OnTriangle(c);
	else FEGraphicalMenu::OnTriangle(c);
}

void MainFrontEnd::OnCross(int c) 
{ 
	input_mgr::inst()->SetDefaultController(c);
	if(active) active->OnCross(c);
	else
	{
		if (highlighted->GetDisable() == false)
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		FEGraphicalMenu::OnCross(c); 
	}
}

void MainFrontEnd::OnAnyButtonPress(int c, int b)
{
	if(!manager->em->CamIsMoving()) return;
	if(b == FEMENUCMD_CROSS && !manager->em->InFlyin()) manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
	else manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_1);
}

void MainFrontEnd::Select(int entry_index)
{
	if(manager->em->InFlyin()) return;
	if(active)
		active->Select(active->highlighted->entry_num);
	else
	{
		if(entry_index != MainExtrasEntry && entry_index != MainOpEntry && entry_index != MainMultiEntry)
			manager->em->ExitState();
		sys->multiplayer = (entry_index == MainMultiEntry);

		stringx tmp;
		switch(entry_index)
		{
		case MainCareerEntry:	career_menu->Select(career_menu->highlighted->entry_num); break;
		case MainFreeEntry:		Freesurf->Select(Freesurf->highlighted->entry_num); break;
		case MainMultiEntry:	Multi->Select(Multi->highlighted->entry_num); break;
		case MainExtrasEntry:	sys->MakeActive(GraphicalMenuSystem::ExtrasMenu); break;
		case MainOpEntry:		MakeActive(Options); break;
		default: assert(0);
		}
	}
}

void MainFrontEnd::OnActivate()
{
	manager->em->ToMainScreen();
	manager->em->ResumeCameraRoll();
	Options->setPQMain(false, true);
	FEMenu* ac_before = active;

	// if Freesurf or Multiplayer were the last active menus before another screen
	// was activated (SurferMenu) then re-activate them
	if(active && active != Options && active->menu_num != manager->gms->active)
		MakeActive((FEGraphicalMenu*) active);
	else
	{
		FEMultiMenu::OnActivate();
		if(sys->active == GraphicalMenuSystem::ExtrasMenu)
			setHigh(entry_list[MainExtrasEntry]);
		else
		{
			// if previous active was Options, highlight options entry
			if(ac_before == Options)
				setHigh(entry_list[MainOpEntry]);
			else if (returnToHighlighted != -1)
			{
				highlighted = entry_list[MainCareerEntry];
				returnToHighlighted = -1;
			}
			else
			{
				switch(manager->tmp_game_mode)
				{
				case GAME_MODE_CAREER: setHigh(entry_list[MainCareerEntry]); break;
				case GAME_MODE_FREESURF_INFINITE:
				case GAME_MODE_FREESURF_HIGHSCORE:
				case GAME_MODE_FREESURF_ICON:	setHigh(entry_list[MainFreeEntry]); break;
				case GAME_MODE_TIME_ATTACK:
				case GAME_MODE_HEAD_TO_HEAD:
				case GAME_MODE_PUSH: setHigh(entry_list[MainMultiEntry]); break;
				default: break;
				}
			}
		}
		UpdateHighlight();

		// if we go all the way back to main screen, then the mode might change, so 
		// we'll need to re-find the proper beach
		((BeachFrontEnd*) sys->menus[GraphicalMenuSystem::BeachMenu])->ResetFirstTime();
	}

	entry_list[MainFreeEntry]->SetNoFlash(true);
	entry_list[MainMultiEntry]->SetNoFlash(true);
	entry_list[MainCareerEntry]->SetNoFlash(true);
}

void MainFrontEnd::MakeActive(FEMenu* a)
{
	if(!a && active == Options)
		Options->setPQMain(false);
	FEGraphicalMenu::MakeActive(a);
}

/********* CareerMenu **********************************************/

CareerMenu::CareerMenu(FEMenuSystem* s, int x, int y)
{
	cons(s, x, y);
	scale = scale_high = 0.9f; //0.5f;

	sys = (GraphicalMenuSystem*) s;
	color = s->manager->col_unselected;
	color_high = s->manager->col_highlight;
	flags |= FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH | FEMENU_HAS_COLOR_HIGH_ALT | FEMENU_WRAP;
	color_high_alt = s->manager->col_highlight2;

	for(int i=0; i<NumEntries; i++)
	{
		stringx tmp = "";
		switch(i)
		{
		case ContinueEntry: tmp = ksGlobalTextArray[GT_FE_MENU_CONTINUE_CAREER]; break;
		case NewEntry:		tmp = ksGlobalTextArray[GT_FE_MENU_NEW_CAREER]; break;
		case LoadEntry:		tmp = ksGlobalTextArray[GT_FE_MENU_SAVE] + " / " + ksGlobalTextArray[GT_FE_MENU_LOAD]; break;
		case SaveEntry:		tmp = ksGlobalTextArray[GT_MENU_SAVE_CAREER]; break;
		case OKEntry:		tmp = ksGlobalTextArray[GT_FE_MENU_OK]; break;
		case CancelEntry:	tmp = ksGlobalTextArray[GT_FE_MENU_CANCEL]; break;
		default: assert(0);
		}
			
		entry[i] = NEW FEMenuEntry(tmp, this, false, &s->manager->font_hand);
		int y1 = y+(i-1)*spacing;
		if(i >= SaveEntry) y1 -= 30;
		entry[i]->SetPos(x, y1);
		Add(entry[i]);
	}

	for(int i=0; i<NumEntries; i++)
	{
		if(i != NumEntries-1) entry[i]->down = entry[i+1];
		else entry[i]->down = entry[0];
		if(i != 0) entry[i]->up = entry[i-1];
		else entry[i]->up = entry[NumEntries-1];
	}

	entry[ContinueEntry]->up = entry[NewEntry];
	entry[NewEntry]->down = entry[ContinueEntry];
	entry[SaveEntry]->up = entry[CancelEntry];
	entry[CancelEntry]->down = entry[SaveEntry];

	warning = NEW BoxText(&s->manager->font_info, ksGlobalTextArray[GT_FE_MENU_KILL_CAREER], 103, 180, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, frontendmanager.col_info_b, 7);
	int line_num = warning->makeBox(164, 100, false, -1, spacing);
	int height = 180 + (line_num+1)*spacing;

	for(int i=SaveEntry; i<NumEntries; i++)
		entry[i]->SetPos(x, height+(i-SaveEntry)*spacing);
	warning_bottom = height+(NumEntries-SaveEntry)*spacing;

	SetHelpText(0);
}

bool careerStarted = false;
void CareerMenu::Select(int entry_index)
{
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	if(warning_up)
	{
		switch(entry_index)
		{
		case OKEntry:
			// create new game
			frontendmanager.tmp_game_mode = GAME_MODE_CAREER;
			currentGame.valid = GSErrorOther;
			savePort = saveSlot = INVALID_CARD_VALUE;
			g_career->StartNewCareer();
			frontendmanager.unsaved_career = false;
			careerStarted = true;
			g_career->init();
			savePort = INVALID_CARD_VALUE;
			saveSlot = INVALID_CARD_VALUE;
			sys->MakeActive(GraphicalMenuSystem::SurferMenu);
			break;
		case CancelEntry:
			EndWarning();
			break;
		case SaveEntry:
			sys->MakeActive(GraphicalMenuSystem::SaveLoadMenu, SaveLoadFrontEnd::ACT_SAVE);
			((SaveLoadFrontEnd *) sys->menus[GraphicalMenuSystem::SaveLoadMenu])->forward_menu = GraphicalMenuSystem::SurferMenu;
			((SaveLoadFrontEnd *) sys->menus[GraphicalMenuSystem::SaveLoadMenu])->back_menu = GraphicalMenuSystem::MainMenu;
			((MainFrontEnd *) sys->menus[GraphicalMenuSystem::MainMenu])->returnToHighlighted = MainFrontEnd::MainCareerEntry;
			break;
		default: assert(0);
		}
	}
	else
	{
		switch(entry_index)
		{
		case ContinueEntry:
			frontendmanager.tmp_game_mode = GAME_MODE_CAREER;
			sys->MakeActive(GraphicalMenuSystem::SurferMenu);
			break;
		case LoadEntry:
			sys->MakeActive(GraphicalMenuSystem::SaveLoadMenu);
			break;
		case NewEntry:
			if((currentGame.valid == GSOk || careerStarted) && frontendmanager.unsaved_career)
				StartWarning();
			else
			{
				currentGame.valid = GSErrorOther;
				frontendmanager.tmp_game_mode = GAME_MODE_CAREER;
				careerStarted = true;
				g_career->init();
				savePort = INVALID_CARD_VALUE;
				saveSlot = INVALID_CARD_VALUE;
				sys->MakeActive(GraphicalMenuSystem::SurferMenu);
				frontendmanager.unsaved_career = false;
			}
			break;
		default:
			assert(0);
		}
	}
}

void CareerMenu::OnUp(int c)
{
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	FEMenu::OnUp(c);
}

void CareerMenu::OnDown(int c)
{
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	FEMenu::OnDown(c);
}

void CareerMenu::OnActivate()
{
	entry[SaveEntry]->Disable(true);
	entry[OKEntry]->Disable(true);
	entry[CancelEntry]->Disable(true);
	entry[ContinueEntry]->Disable(false);
	entry[LoadEntry]->Disable(false);
	entry[NewEntry]->Disable(false);

	// disable Continue option if no career was loaded
	entry[ContinueEntry]->Disable(!(careerStarted || currentGame.valid == GSOk));

	// Disable if there are no saved games
	//entry[LoadEntry]->Disable(!((SaveLoadFrontEnd*) manager->gms->menus[GraphicalMenuSystem::SaveLoadMenu])->SavedGamesExist());

	FEMenu::OnActivate();
	//system->manager->em->ToMainScreen();
	warning_up = false;
	((MainFrontEnd*) parent)->ResizeCareerBox(regular_bottom);
}

void CareerMenu::Draw()
{
	if(warning_up)
	{
		warning->Draw();
		entry[SaveEntry]->Draw();
		entry[OKEntry]->Draw();
		entry[CancelEntry]->Draw();
		
	}
	else
	{
		for(int i=0; i<SaveEntry; i++)
			entry[i]->Draw();
	}
}

void CareerMenu::OnTriangle(int c)
{
	if(warning_up)
	{
		EndWarning();
		return;
	}
	SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	system->MakeActive(GraphicalMenuSystem::MainMenu);
}

void CareerMenu::StartWarning()
{
	warning_up = true;
	setHigh(entry[SaveEntry]);
	entry[ContinueEntry]->Disable(true);
	entry[LoadEntry]->Disable(true);
	entry[NewEntry]->Disable(true);
	entry[SaveEntry]->Disable(false);
	entry[OKEntry]->Disable(false);
	entry[CancelEntry]->Disable(false);
	((MainFrontEnd*) parent)->ResizeCareerBox(warning_bottom);
}

void CareerMenu::EndWarning()
{
	warning_up = false;
	OnActivate();
}


/**************************************************************/

void OptionsMenu::cons(FEMenuSystem* s, FEManager* man, stringx path, stringx pf_name)
{
	FEGraphicalMenu::cons(s, man, path, pf_name);

	flags |= FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH;
	color = manager->gray_dk;
	color_high = manager->red_dk;
	color_high_alt = color;
	scale = scale_high = 0.65f; //0.5f*1.5f;

	options_list[OpScoreEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_SCORE], this, false, &manager->font_bold_old);
#ifdef TARGET_GC
	options_list[OpRumble1Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM1_GC], this, false, &manager->font_bold_old);
	options_list[OpRumble2Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM2_GC], this, false, &manager->font_bold_old);
	options_list[OpRumble3Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM3_GC], this, false, &manager->font_bold_old);
	options_list[OpRumble4Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM4_GC], this, false, &manager->font_bold_old);
#else
	options_list[OpRumble1Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM1], this, false, &manager->font_bold_old);
	options_list[OpRumble2Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM2], this, false, &manager->font_bold_old);
	options_list[OpRumble3Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM3], this, false, &manager->font_bold_old);
	options_list[OpRumble4Entry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_RUM4], this, false, &manager->font_bold_old);
#endif
	options_list[OpStereoEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_AUDIO_MODE], this, false, &manager->font_bold_old);
	options_list[OpMasterEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_MASTER], this, false, &manager->font_bold_old);
	options_list[OpAmbientEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_AMBIENT], this, false, &manager->font_bold_old);
	options_list[OpSFXEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_SFX], this, false, &manager->font_bold_old);
	options_list[OpVoiceEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_VOICE], this, false, &manager->font_bold_old);
	options_list[OpMusicEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_OP_MUSIC], this, false, &manager->font_bold_old);

	for(int i=0; i<OpEnd; i++)
	{
		Add(options_list[i]);
		if(i < OpStereoEntry) options_list[i]->SetPos(348, 138+i*15);//331, 153+i*15);
		else if(i == OpStereoEntry) options_list[i]->SetPos(520, 185);
		else options_list[i]->SetPos(176, 245+(i-OpStereoEntry-1)*15);// 171
		options_list[i]->SetHJustify(Font::HORIZJUST_RIGHT);
#ifdef TARGET_XBOX
		if (i == OpStereoEntry)
		{
			options_list[i]->Disable(true);
		}
#endif
	}
	for(int i=0; i<num_switches; i++)
	{
		switch_text[i][0] = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_OFF], 396, /*153*/138+i*15, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);
		switch_text[i][1] = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_ON], 396, /*153*/138+i*15, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);
	}
	stereo_text[0] = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_AM_MONO], 482, 161, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);
	stereo_text[1] = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_AM_STER], 482, 161, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);
	stereo_text[2] = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_AM_PRO], 482, 161, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);
	options_text = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_OPTIONS], 127, 129, 0, .5*1.5f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color);
	slevels_text = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_OP_SLEVELS], 123, 222, 0, .4*1.5f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color);
}

OptionsMenu::~OptionsMenu()
{
	for(int i=0; i<num_switches; i++)
	{
		if(i < 3) delete stereo_text[i];
		delete switch_text[i][0];
		delete switch_text[i][1];
	}
	delete options_text;
	delete slevels_text;
}

void OptionsMenu::Load()
{
	panel.Load();
	SetPQIndices();
}

void OptionsMenu::Draw()
{
	options_text->Draw();
	slevels_text->Draw();
	stereo_text[stereo_op]->Draw();
	for(int i=0; i<num_switches; i++)
		switch_text[i][switches[i]]->Draw();
	panel.Draw(0);
	FEGraphicalMenu::Draw();
}

void OptionsMenu::OnTriangle(int c)
{
	FEGraphicalMenu::OnTriangle( c );
	RumbleOn( false, 0 );
	RumbleOn( false, 1 );
	RumbleOn( false, 2 );
	RumbleOn( false, 3 );
}
	
void OptionsMenu::OnActivate()
{
	FEGraphicalMenu::OnActivate();
	lights[highlighted->entry_num]->TurnOn(true);

	// read in switches here
	switches[OpScoreEntry] = ((FEGraphicalMenu*)parent)->manager->score_display;
	for( int i = 0; i < 4; i++ )
		switches[OpRumble1Entry + i] = rumbleMan.isOn(i);	

	volumes[OpMasterEntry-first_vol] = (int)(nslGetMasterVolume()*MAX_VOLUME + .5f);
	volumes[OpAmbientEntry-first_vol] = (int)(nslGetVolume(NSL_SOURCETYPE_AMBIENT)*MAX_VOLUME+ .5f);
	volumes[OpSFXEntry-first_vol] = (int)(nslGetVolume(NSL_SOURCETYPE_SFX)*MAX_VOLUME+ .5f);
	volumes[OpVoiceEntry-first_vol] = (int)(nslGetVolume(NSL_SOURCETYPE_VOICE)*MAX_VOLUME+ .5f);
	volumes[OpMusicEntry-first_vol] = (int)(nslGetVolume(NSL_SOURCETYPE_MUSIC)*MAX_VOLUME+ .5f);

	float v[5];
	for(int i=0; i<NUM_VOLUME; i++)
		v[i] = (float)volumes[i]/(float)MAX_VOLUME;

	for(int i=0; i<NUM_VOLUME; i++)
		MaskVolume(i);

	int control_count = 0;

	for (int i=JOYSTICK1_DEVICE; i <= JOYSTICK8_DEVICE; i++)
		if(input_mgr::inst()->get_device((device_id_t)i))
			if(input_mgr::inst()->get_device((device_id_t)i)->is_connected())
				control_count++;


  
	rumbleTimer[0] = -1;
	rumbleTimer[1] = -1;
	rumbleTimer[2] = -1;
	rumbleTimer[3] = -1;
	
	switch(nslGetSpeakerMode())
	{
	case NSL_SPEAKER_MONO:		stereo_op = 0; break;
	case NSL_SPEAKER_STEREO:	stereo_op = 1; break;
	case NSL_SPEAKER_PROLOGIC:	stereo_op = 2; break;
	default: assert(0);
	}

	setPQMain(true);

	manager->helpbar->Reset();
	manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_CHANGE]);
	manager->helpbar->AddArrowV();
	manager->helpbar->RemoveX();
	manager->helpbar->Reformat();
}

void OptionsMenu::Update(time_value_t time_inc)
{
	int i;
	
	
	// Update rumble status
	for( int j = 0; j < 4; j++ )
	{
		input_device* joy = input_mgr::inst()->get_joystick_index( j );
		if( joy && joy->is_connected() )
			options_list[ OpRumble1Entry + j ]->Disable( false );
		else
		{
			if (highlighted == options_list[ OpRumble1Entry + j ])
				setHigh(options_list[OpScoreEntry]);
			options_list[ OpRumble1Entry + j ]->Disable( true );
		}
	}
	
	options_text->Update(time_inc);
	slevels_text->Update(time_inc);
	stereo_text[stereo_op]->Update(time_inc);

	for (i=0; i < 4; i++)
		if (rumbleTimer[i] >= 0)
		{
			rumbleTimer[i] += time_inc;
			if (rumbleTimer[i] > RUMBLETIME)
				RumbleOn(false, i);
		}

	for(i=0; i<num_switches; i++)
		switch_text[i][switches[i]]->Update(time_inc);
	FEGraphicalMenu::Update(time_inc);
}

void OptionsMenu::OnUp(int c)
{
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	lights[highlighted->entry_num]->TurnOn(false);
	int fade;
	float alpha, timer;
	lights[highlighted->entry_num]->GetFade(fade, alpha, timer);
	FEGraphicalMenu::OnUp(c);
	lights[highlighted->entry_num]->TurnOn(true);
	lights[highlighted->entry_num]->SetFade(fade, alpha, timer);
}

void OptionsMenu::OnDown(int c)
{
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	lights[highlighted->entry_num]->TurnOn(false);
	int fade;
	float alpha, timer;
	lights[highlighted->entry_num]->GetFade(fade, alpha, timer);
	FEGraphicalMenu::OnDown(c);
	lights[highlighted->entry_num]->TurnOn(true);
	lights[highlighted->entry_num]->SetFade(fade, alpha, timer);
}

void OptionsMenu::OnLeft(int c)
{
	if(highlighted->entry_num < OpMasterEntry)
		ChangeSwitch(false);
	else ChangeVolume(false);
}

void OptionsMenu::OnRight(int c)
{
	if(highlighted->entry_num < OpMasterEntry)
		ChangeSwitch(true);
	else ChangeVolume(true);
}

void OptionsMenu::setPQMain(bool on, bool from_main)
{
	if(from_main)
	{
		back->TurnOn(on);

		for(int i=0; i<OpEnd; i++)
			lights[i]->TurnOn(on);

		for(int i=0; i<NUM_VOLUME; i++)
			for(int j=0; j<MAX_VOLUME; j++)
				levels[i][j]->TurnOn(on);
	}
	else
	{
		back->ChangeFade(true, on, 1.0f);
		lights[highlighted->entry_num]->ChangeFade(true, on, 1.0f);

		options_text->ChangeFade(true, on, 1.0f);
		slevels_text->ChangeFade(true, on, 1.0f);
		stereo_text[stereo_op]->ChangeFade(true, on, 1.0f);
		for(int i=0; i<num_switches; i++)
			switch_text[i][switches[i]]->ChangeFade(true, on, 1.0f);

		for(int i=0; i<OpEnd; i++)
			options_list[i]->SetFade(true, on, 1.0f);

		for(int i=0; i<NUM_VOLUME; i++)
			for(int j=0; j<MAX_VOLUME; j++)
				if(volumes[i] > j)
					levels[i][j]->ChangeFade(true, on, 1.0f);
	}
}
void OptionsMenu::RumbleOn(bool on, int controller)
{
	input_mgr* inputmgr = input_mgr::inst();
	input_device *joyjoy=inputmgr->get_joystick(JOYSTICK_TO_DEVICE_ID(controller+JOYSTICK1_DEVICE));
	if (on)
	{
		rumbleTimer[controller] = 0.0f;
		if ( joyjoy )
    	joyjoy->vibrate( 0.0f,255.0f,1.0f,0.0f);
	}
	else
		if ( joyjoy )
		{
    	joyjoy->vibrate(0.0f,0.0f,0.0f,0.0f);
			rumbleTimer[controller] = -1.0f;
		}
}
void OptionsMenu::ChangeSwitch(bool up)
{
	int index = highlighted->entry_num;

	if(index == OpStereoEntry)
	{
		if(up)
		{
			stereo_op++;
			if(stereo_op == 3) stereo_op = 0;
		}
		else
		{
			stereo_op--;
			if(stereo_op == -1) stereo_op = 2;
		}
	}
	else switches[index] = !switches[index];
  
	play_sound("LEFTRIGHT");

	switch(index)
	{
	case OpScoreEntry:   ((FEGraphicalMenu*)parent)->manager->enableScoreDisplay(switches[index]); break;
	case OpStereoEntry:
		if(stereo_op == 0) nslSetSpeakerMode(NSL_SPEAKER_MONO);
		else if(stereo_op == 1) nslSetSpeakerMode(NSL_SPEAKER_STEREO);
		else nslSetSpeakerMode(NSL_SPEAKER_PROLOGIC); break;
	case OpRumble1Entry: RumbleOn(switches[index], 0);  rumbleMan.turnOn(switches[index], 0); break;
	case OpRumble2Entry: RumbleOn(switches[index], 1);  rumbleMan.turnOn(switches[index], 1); break;
	case OpRumble3Entry: RumbleOn(switches[index], 2);  rumbleMan.turnOn(switches[index], 2); break;
	case OpRumble4Entry: RumbleOn(switches[index], 3);  rumbleMan.turnOn(switches[index], 3); break;
	default: assert(0);
	}
}

void OptionsMenu::SetPQIndices()
{
	stringx tmp;
	for(int i=0; i<NUM_VOLUME; i++)
	{
		if(i == 0) tmp = "op_level_a";
		if(i == 1) tmp = "op_level_b";
		if(i == 2) tmp = "op_level_c";
		if(i == 3) tmp = "op_level_d";
		if(i == 4) tmp = "op_level_e";

		for(int j=0; j<MAX_VOLUME; j++)
		{
			if(j+1 < 10) levels[i][j] = GetPointer((tmp+"0"+stringx(j+1)).data());
			else levels[i][j] = GetPointer((tmp+stringx(j+1)).data());
		}
	}

	lights[0] = GetPointer("op_light10");
	lights[1] = GetPointer("op_light01");
	lights[2] = GetPointer("op_light02");
	lights[3] = GetPointer("op_light03");
	lights[4] = GetPointer("op_light11");
	lights[5] = GetPointer("op_light04");
	lights[6] = GetPointer("op_light05");
	lights[7] = GetPointer("op_light06");
	lights[8] = GetPointer("op_light07");
	lights[9] = GetPointer("op_light08");
	lights[10] = GetPointer("op_light09");

	back = GetPointer("op_radioface");
}

void OptionsMenu::ChangeVolume(bool up)
{
	int index = highlighted->entry_num-first_vol;
	if(up)
	{
		if(volumes[index] < MAX_VOLUME)
		{
			volumes[index]++;
			EventType soundtoPlay = SS_FE_LEFTRIGHT;
			switch (index+first_vol)
			{
				case OpAmbientEntry: soundtoPlay = SS_FE_LEFTRIGHT_AMBIENT; break;
				case OpVoiceEntry: soundtoPlay = SS_FE_LEFTRIGHT_VOICE;   break;
				case OpMusicEntry: soundtoPlay = SS_FE_LEFTRIGHT_MUSIC;   break;
			}
			//SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(soundtoPlay);
			//SoundScriptManager::inst()->pause();
		}
		else play_sound("ERROR");
	}
	else
	{
		if(volumes[index] > 0)
		{
			volumes[index]--;
			EventType soundtoPlay = SS_FE_LEFTRIGHT;
			switch (index+first_vol)
			{
				case OpAmbientEntry: soundtoPlay = SS_FE_LEFTRIGHT_AMBIENT; break;
				case OpVoiceEntry: soundtoPlay = SS_FE_LEFTRIGHT_VOICE;   break;
				case OpMusicEntry: soundtoPlay = SS_FE_LEFTRIGHT_MUSIC;   break;
			}
			//SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(soundtoPlay);
			//SoundScriptManager::inst()->pause();
		}
		else play_sound("ERROR");
	}

	MaskVolume(index);

	float level = ((float)volumes[index] +.5f)/(float)MAX_VOLUME;
	if (level > 1) level = 1;
	switch(index+first_vol)
	{
	case OpMasterEntry:		nslSetMasterVolume(level); break;
	case OpAmbientEntry:	nslSetVolume(NSL_SOURCETYPE_AMBIENT, level); break;
	case OpSFXEntry:		nslSetVolume(NSL_SOURCETYPE_SFX, level); break;
	case OpVoiceEntry:		nslSetVolume(NSL_SOURCETYPE_VOICE, level); break;
	case OpMusicEntry:		nslSetVolume(NSL_SOURCETYPE_MUSIC, level); break;
	default: assert(0);
	}
}

// level should be between 0 & MAX_VOLUME inclusive
void OptionsMenu::MaskVolume(int index)
{
	for(int i=0; i<MAX_VOLUME; i++)
		levels[index][i]->TurnOn(volumes[index] > i);
}

/**************** Multiplayer *************************************************/

MultiplayerMenu::MultiplayerMenu(FEMenuSystem* s, int x, int y)
{
	cons(s, x, y);
	color = s->manager->col_unselected;
	color_high = s->manager->col_highlight;
	flags |= FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH | FEMENU_HAS_COLOR_HIGH_ALT | FEMENU_WRAP;
	color_high_alt = s->manager->col_highlight2;
	scale = scale_high = 0.9f; //0.5f;
	sys = (GraphicalMenuSystem*) s;

	entry[MultiPushEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_MODE_PUSH], this);
	entry[MultiHeadToHeadEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_MODE_HEAD_TO_HEAD], this);
	entry[MultiTimeAttackEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_MODE_TIME_ATTACK], this);
	//entry[MultiMeterAttackEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_MODE_METER_ATTACK], this);

	for(int i=0; i<MultiEnd; i++)
	{
		entry[i]->SetPos(x, y+(i-1)*spacing);
		entry[i]->SetFont(&s->manager->font_hand);
		Add(entry[i]);
	}
	dy = spacing;
	SetHelpText(0);
}

void MultiplayerMenu::OnActivate()
{
	UpdateDisable();
	FEMenu::OnActivate();
	int act = -1;

	// set default according to tmp_game_mode
	switch(system->manager->tmp_game_mode)
	{
	case GAME_MODE_TIME_ATTACK:		act = MultiTimeAttackEntry; break;
	case GAME_MODE_HEAD_TO_HEAD:	act = MultiHeadToHeadEntry; break;
	case GAME_MODE_PUSH: 			act = MultiPushEntry; break;
	default: break;
	}
	if(act != -1 && !entry[act]->GetDisable())
		setHigh(entry[act]);
}

void MultiplayerMenu::Update(time_value_t time_inc)
{
	FEMenu::Update(time_inc);
	UpdateDisable();
}
void MultiplayerMenu::OnUp(int c)
{
	FEMenuEntry *old = highlighted;
	FEMenu::OnUp(c);
	if (highlighted == old)
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	else
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
}
void MultiplayerMenu::OnDown(int c)
{
	FEMenuEntry *old = highlighted;
	FEMenu::OnDown(c);
	if (highlighted == old)
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	else
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
}
// Head2Head and Push are not available without both controllers
void MultiplayerMenu::UpdateDisable()
{
	int num_controllers = 0;
	
	for( int j = 0; j < DEVICE_ID_TO_JOYSTICK_INDEX( AI_JOYSTICK ); j++ )
	{
		input_device* joy = input_mgr::inst()->get_joystick_index( j );
		if( joy && joy->is_connected() )
			num_controllers++;
	}
	
	entry[MultiHeadToHeadEntry]->Disable( num_controllers < 2 );
	entry[MultiPushEntry]->Disable( num_controllers < 2 );

	// if highlighted entry is currently disabled, set the next available entry highlighted
	if(highlighted->GetDisable())
	{
		FEMenuEntry* tmp = highlighted->next;
		if(!tmp) tmp = entries;
		while(tmp != highlighted)
		{
			if(!tmp->GetDisable())
			{
				setHigh(tmp);
				break;
			}
			tmp = tmp->next;
			if(!tmp) tmp = entries;
		}
	}
}

void MultiplayerMenu::Select(int entry_index)
{
	switch(entry_index)
	{
	case MultiPushEntry:	   sys->manager->tmp_game_mode = GAME_MODE_PUSH; break;
	case MultiHeadToHeadEntry: sys->manager->tmp_game_mode = GAME_MODE_HEAD_TO_HEAD; break;
	case MultiTimeAttackEntry: sys->manager->tmp_game_mode = GAME_MODE_TIME_ATTACK; break;
	//case MultiMeterAttackEntry: sys->manager->tmp_game_mode = GAME_MODE_METER_ATTACK; break;
	default: assert(0); break;
	}
	if(entry_index == MultiHeadToHeadEntry)
		system->MakeActive(GraphicalMenuSystem::SurferMenu);
	else parent->MakeActive(((MainFrontEnd*) parent)->multi_sub);
}

void MultiplayerMenu::OnTriangle(int c)
{
	parent->MakeActive(NULL);
}

/**************** MultiSubMenu *************************************************/

MultiSubMenu::MultiSubMenu(FEMenuSystem* s, int x, int y)
{
	cons(s, x, y);
	color = s->manager->col_unselected;
	color_high = s->manager->col_highlight;
	flags |= FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH | FEMENU_HAS_COLOR_HIGH_ALT | FEMENU_WRAP;
	color_high_alt = s->manager->col_highlight2;
	scale = scale_high = 0.9f; //0.5f;
	sys = (GraphicalMenuSystem*) s;

	entry[SubEasyEntry] = NEW FEMenuEntry("", this);
	entry[SubMediumEntry] = NEW FEMenuEntry("", this);
	entry[SubHardEntry] = NEW FEMenuEntry("", this);

	for(int i=0; i<SubEnd; i++)
	{
		entry[i]->SetPos(x, y+(i-1)*spacing);
		entry[i]->SetFont(&s->manager->font_hand);
		Add(entry[i]);
	}

	float diff_scale = 0.65f;
	difficulty = NEW TextString(&s->manager->font_bold, ksGlobalTextArray[GT_FE_MENU_DIFFICULTY], 479, 233, 0, diff_scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, s->manager->col_info_b);

	push[SubEasyEntry]		= 2500;
	push[SubMediumEntry]	= 5000;
	push[SubHardEntry]		= 10000;
	time[SubEasyEntry]		= 1000;
	time[SubMediumEntry]	= 10000;
	time[SubHardEntry]		= 30000;

	dy = spacing;
	SetHelpText(0);
}

void MultiSubMenu::OnActivate()
{
	FEMenu::OnActivate();

	// set default according to tmp_game_mode
	if(system->manager->tmp_game_mode == GAME_MODE_TIME_ATTACK)
	{
		char tmp[100];
		for(int i=0; i<SubEnd; i++)
		{
			sprintf(tmp, ksGlobalTextArray[GT_FE_MENU_TIME_LEVEL].data(), time[i]);
			entry[i]->SetText(tmp);
		}
	}
	else
	{
		char tmp[100];
		for(int i=0; i<SubEnd; i++)
		{
			sprintf(tmp, ksGlobalTextArray[GT_FE_MENU_PUSH_LEVEL].data(), push[i]);
			entry[i]->SetText(tmp);
		}
	}

	frontendmanager.helpbar->Reset();
	frontendmanager.helpbar->AddArrowV();
	frontendmanager.helpbar->Reformat();
}

void MultiSubMenu::Draw()
{
	FEMenu::Draw();
	difficulty->Draw();
}

void MultiSubMenu::OnUp(int c)
{
	FEMenuEntry *old = highlighted;
	FEMenu::OnUp(c);
	if (highlighted == old)
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	else
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
}

void MultiSubMenu::OnDown(int c)
{
	FEMenuEntry *old = highlighted;
	FEMenu::OnDown(c);
	if (highlighted == old)
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	else
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
}

void MultiSubMenu::Select(int entry_index)
{
	if(frontendmanager.tmp_game_mode == GAME_MODE_TIME_ATTACK)
		g_game_ptr->multiplayer_difficulty = time[entry_index];
	else g_game_ptr->multiplayer_difficulty = push[entry_index];

	system->MakeActive(GraphicalMenuSystem::SurferMenu);
}

void MultiSubMenu::OnTriangle(int c)
{
	parent->MakeActive(NULL);
}

/**************** FreesurfMenu *************************************************/

FreesurfMenu::FreesurfMenu(FEMenuSystem* s, int x, int y)
{
	cons(s, x, y);
	color = s->manager->col_unselected;
	color_high = s->manager->col_highlight;
	flags |= FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH | FEMENU_HAS_COLOR_HIGH_ALT | FEMENU_WRAP;
	color_high_alt = s->manager->col_highlight2;
	scale = scale_high = 0.9f; //0.5f;
	sys = (GraphicalMenuSystem*) s;

	entry[FreeRegularEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_FREE_REGULAR], this);
	entry[FreeHighScoreEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_FREE_HS], this);
	entry[FreeIconEntry] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_FREE_ICON], this);

	for(int i=0; i<FreeEnd; i++)
	{
		entry[i]->SetPos(x, y+(i-1)*spacing);
		entry[i]->SetFont(&s->manager->font_hand);
		Add(entry[i]);
	}
	dy = spacing;
	SetHelpText(0);
}

void FreesurfMenu::OnActivate()
{
	FEMenu::OnActivate();
	int act = -1;

	// set default according to tmp_game_mode
	switch(system->manager->tmp_game_mode)
	{
	case GAME_MODE_FREESURF_INFINITE:	act = FreeRegularEntry; break;
	case GAME_MODE_FREESURF_HIGHSCORE:	act = FreeHighScoreEntry; break;
	case GAME_MODE_FREESURF_ICON: 		act = FreeIconEntry; break;
	default: break;
	}
	if(act != -1 && !entry[act]->GetDisable())
		setHigh(entry[act]);
}

void FreesurfMenu::Select(int entry_index)
{
	// when new game modes are added, this should be changed.
	switch(entry_index)
	{
	case FreeRegularEntry:		sys->manager->tmp_game_mode = GAME_MODE_FREESURF_INFINITE; break;
	case FreeHighScoreEntry:	sys->manager->tmp_game_mode = GAME_MODE_FREESURF_HIGHSCORE; break;
	case FreeIconEntry:			sys->manager->tmp_game_mode = GAME_MODE_FREESURF_ICON; break;
	default: assert(0); break;
	}
	system->MakeActive(GraphicalMenuSystem::SurferMenu);
}

void FreesurfMenu::OnTriangle(int c)
{
	parent->MakeActive(NULL);
}

/******************************************************************************/


// this is only used in conjunction with the "ONE-TIME HACK DEAL"
// (which is used to properly format the bios, which only needs to be 
// done when something changes about the bio
void MainFrontEnd::FormatBIO(Font *fon, int maxPixelWidth, char *in, int inSize, char *out, int outSize, float scale)
{
#ifdef TARGET_PS2
	float runningWidth;
	unsigned int width = 100;

	// for ps2 formatting
	float scale_x = scale*.8f;
//	float scale_y = scale*.933f;
	maxPixelWidth = FTOI(maxPixelWidth*0.8f);
		
	char line[width];
	char nextLine[width];
	
	char *donePtr  = in + inSize; 
	memset(out, 0, outSize);
	memset(line, 0, width);
	memset(nextLine, 0, width);

	while (in != donePtr)
	{
		runningWidth = 0;
		memset(line, 0, width);
		strcpy(line, nextLine);
		for (unsigned int i=0; i < strlen(line); i++)
		{
			if (fon->getGlyph(line[i]))
				runningWidth += fon->getGlyph(line[i])->cell_width*scale_x;
		}
		unsigned int c = 0;
		
		while ((!fon->getGlyph((char)*in) || (runningWidth+ fon->getGlyph((char)*in)->cell_width*scale_x) < (unsigned int)(maxPixelWidth))
			&& (in != donePtr) && (c != '\n'))
		{
			c = *in;
			in++;
			line[strlen(line)] = c;
			line[strlen(line)+1] = 0;
			if (fon->getGlyph(c))
				runningWidth += fon->getGlyph(c)->cell_width*scale_x;
		}
		
		if (line[0] == '\n')
			line[0] = 0;
		
		if (in == donePtr)
		{
			line[strlen(line)-1] = 0;
			strcat(out, line);
			strcat(out, "\n");
			return;
		}
		line[width] = '\0';
		if (line[strlen(line)-1] != '\n')
		{
			char* space = strrchr(line, ' ');
			char* dash = strrchr(line, '-');
			if (space || dash)
			{
				char* tmp;
				if(space == NULL)		tmp = dash;
				else if(dash == NULL)	tmp = space;
				else if(space > dash)	tmp = space;
				else					tmp = dash;
				strcpy(nextLine, tmp + 1);
				line[tmp-line+1] = '\0';
			}
			else
			{
				memset(nextLine, 0, width);
			}
		}
		else
		{
			memset(nextLine, 0, width);
		}
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = 0;
		
		strcat(out, line);
		outSize -= strlen(line);
		strcat(out, "\n");
		outSize--;
		assert(outSize >=0);
	}
	
	return;
#endif // TARGET_PS2
}

