// ExtrasFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "FrontEndManager.h"
#include "ExtrasFrontEnd.h"
#include "profiler.h"
#include "unlock_manager.h"

#ifdef TARGET_PS2
#include <sifcmd.h>
#endif

#if defined(TARGET_XBOX)
#include "osdevopts.h"
#include "MusicMan.h"
#include "ksnsl.h"
#endif /* TARGET_XBOX JIV DEBUG */

ExtrasFrontEnd::ExtrasFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;
	int i;

	flags |= FEMENU_USE_SCALE | FEMENU_HAS_COLOR_HIGH_ALT;
	scale = scale_high = 0.9f; //0.666f;
	float sc_lg = 0.8f;

//	color = manager->yel_dk;
//	color_high = manager->yel_lt;
	color = manager->col_unselected;
	color_high = manager->col_highlight;
	color_high_alt = manager->col_highlight2;

	for(i=0; i<ExtrasDemoEntryYes; i++)
	{
		entry[i] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_BEACHES+i], this);
		entry[i]->SetPos(180, 170+i*18);
		entry[i]->SetFont(&manager->font_hand);
		Add(entry[i]);
	}
	entry[ExtrasDemoEntryYes] =  NEW FEMenuEntry(ksGlobalTextArray[GT_MC_CONTINUE], this);
	entry[ExtrasDemoEntryYes]->SetPos(180, 227);
	entry[ExtrasDemoEntryYes]->SetFont(&manager->font_hand);
	entry[ExtrasDemoEntryYes]->Disable(true);
	Add(entry[ExtrasDemoEntryYes]);

	entry[ExtrasDemoEntryNo] =  NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_BACK], this);
	entry[ExtrasDemoEntryNo]->SetPos(180, 240);
	entry[ExtrasDemoEntryNo]->SetFont(&manager->font_hand);
	entry[ExtrasDemoEntryNo]->Disable(true);
	Add(entry[ExtrasDemoEntryNo]);

	prompt = NEW BoxText(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_KILL_CAREER], 180, 180, 0, sc_lg, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_info_b);
	prompt->makeBox(200,100);
	saveCareerPrompt = false;
	FEMenuEntry* tmp = entries;
	while(tmp)
	{
		tmp->down = tmp->next;
		tmp->up = tmp->previous;
		if(tmp->next == NULL) tmp->down = entries;
		tmp = tmp->next;
	}

	surfers = NEW FETextMultiMenu(s, color, color_high, sc_lg, sc_lg);
	int count = 0;
	for(i=0; i<SURFER_LAST; i++)
	{
		stringx str = SurferDataArray[i].firstname + stringx("\n") + SurferDataArray[i].lastname;
		FEMenuEntry* tmp = NEW FEMenuEntry(str, surfers);
		tmp->SetLineSpacing(20);
		surfers->Add(tmp);
		tmp->SetPos(448, 220);
		if(SurferDataArray[i].has_movie == 1)
			count++;
		else tmp->Disable();
		tmp->left = tmp->previous;
		if(tmp->left) tmp->left->right = tmp;
	}

	// to setup the wraparound correctly
	surfers->entries->left = surfers->entries->previous;
	surfers->entries->left->right = surfers->entries;

	if(count == 0) entry[ExtrasSurfersEntry]->Disable();
	surfers->Init();
	surfers->flags |= FEMENU_DONT_SHOW_DISABLED;

	beaches = NEW FETextMultiMenu(s, color, color_high, sc_lg, sc_lg);
	count = 0;
	for(i=0; i<BEACH_LAST; i++)
	{
		FEMenuEntry* tmp = NEW FEMenuEntry(BeachDataArray[i].fe_name, beaches);
		beaches->Add(tmp);
		tmp->SetPos(448, 220);
		if(BeachDataArray[i].has_movie) count++;
		else tmp->Disable();
		tmp->left = tmp->previous;
		if(tmp->left) tmp->left->right = tmp;
	}

	beaches->entries->left = beaches->entries->previous;
	beaches->entries->left->right = beaches->entries;

	if(count == 0) entry[ExtrasBeachesEntry]->Disable();
	beaches->Init();
	beaches->flags |= FEMENU_DONT_SHOW_DISABLED;

	trailers = NEW FETextMultiMenu(s, color, color_high, sc_lg, sc_lg);

	tmp = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_TRAILERS_KSINTRO], trailers);
	trailers->Add(tmp);
	tmp->SetPos(448, 220);

	tmp = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_TRAILERS_BAILS], trailers);
	trailers->Add(tmp);
	tmp->SetPos(448, 220);
	tmp->left = tmp->previous;
	tmp->left->right = tmp;

	tmp = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_TRAILERS_ESPN], trailers);
	trailers->Add(tmp);
	tmp->SetPos(448, 220);
	tmp->left = tmp->previous;
	tmp->left->right = tmp;

  trailers->entries->left = trailers->entries->previous;
	trailers->entries->left->right = trailers->entries;

	trailers->Init();
	trailers->flags |= FEMENU_DONT_SHOW_DISABLED;

	cheat_menu = NEW CheatFrontEnd(s, man, p, pf_name);
	cheat_menu->Init();
	this->AddSubmenu(cheat_menu);
	//entry[ExtrasTrailersEntry]->Disable();

	extras = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_EXTRAS], 180, 116, 0, sc_lg*1.18f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_info_b);
	websites = NEW BoxText(&manager->font_info, "", 460, 130, 0, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_info_g, 20);
	arrow_counter = -5;
}

ExtrasFrontEnd::~ExtrasFrontEnd()
{
	delete surfers;
	delete beaches;
	delete trailers;
	delete extras;
	delete websites;
	delete cheat_menu;
	for(int i=0; i<9; i++)
		delete website_box[i];
	delete prompt;
}

void ExtrasFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();

	stringx file = "websites";
//	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
//		file += "_fr";
//	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
//		file += "_ge";
	file += ".txt";

	char* tmp = (char*) (path+file).data();
	websites->ReadFromFile(tmp, false);
	int sp = 15;
	websites->makeBox(300, 300, false, -1, sp);

	arrows[0][1]->TurnOn(false);
	arrows[1][1]->TurnOn(false);

	cheat_menu->Load();
	cheat_menu->TurnOnPhone(false);
}

void ExtrasFrontEnd::SetPQIndices()
{
	arrows[0][0] = GetPointer("ex_button_off_01");
	arrows[0][1] = GetPointer("ex_button_on_01");
	arrows[1][0] = GetPointer("ex_button_off_02");
	arrows[1][1] = GetPointer("ex_button_on_02");

	for(int i=0; i<ExtrasEndEntry; i++)
		videos[i] = NULL;

	videos[ExtrasBeachesEntry] = GetPointer("extras_case_beaches");
	videos[ExtrasSurfersEntry] = GetPointer("extras_case_surfers");
	videos[ExtrasTrailersEntry] = GetPointer("extras_case_trailer");
	videos[ExtrasCreditsEntry] = GetPointer("extras_case_credits");

	PanelQuad* tmp_box;
	for(int i=0; i<9; i++)
	{
		tmp_box = GetPointer(("ex_box_0"+stringx(i+1)).data());
		website_box[i] = NEW PanelQuad(*tmp_box);
	}

	float x_left, x_right, x_tmp, y1, y2;
	website_box[0]->GetPos(x_left, y1, x_tmp, y2);
	website_box[2]->GetPos(x_tmp, y1, x_right, y2);
	float flipped_right_edge = 640 - x_right;
	float website_box_offset = flipped_right_edge - x_left;

	for(int i=0; i<9; i++)
	{
		website_box[i]->GetCenterPos(x_tmp, y1);
		website_box[i]->SetCenterPos(x_tmp + website_box_offset, y1);
	}
}

void ExtrasFrontEnd::Draw()
{
	if(active)
		active->Draw();
	panel.Draw(0);
	if (!saveCareerPrompt)
	{
		if(arrow_counter >= 0) arrow_counter--;
		if(arrow_counter < 0 && arrow_counter > -5)
		{
			arrow_counter = -5;
			arrows[0][0]->TurnOn(highlighted->entry_num == ExtrasSurfersEntry || highlighted->entry_num == ExtrasBeachesEntry || highlighted->entry_num == ExtrasTrailersEntry);
			arrows[0][1]->TurnOn(false);
			arrows[1][0]->TurnOn(highlighted->entry_num == ExtrasSurfersEntry || highlighted->entry_num == ExtrasBeachesEntry || highlighted->entry_num == ExtrasTrailersEntry);
			arrows[1][1]->TurnOn(false);
		}

		
		FEMenuEntry* tmp = entries;
		while(tmp != NULL)
		{
			if (!(flags & FEMENU_DONT_SHOW_DISABLED && tmp->GetDisable()))
			{
				if (tmp != entry[ExtrasDemoEntryYes] && 
				    tmp != entry[ExtrasDemoEntryNo])
				tmp->Draw();
			}
			tmp = tmp->next;
		}

		if(highlighted->entry_num == ExtrasSurfersEntry) surfers->highlighted->Draw();
		else if(highlighted->entry_num == ExtrasBeachesEntry) beaches->highlighted->Draw();
		else if(highlighted->entry_num == ExtrasTrailersEntry) trailers->highlighted->Draw();
		else if(highlighted->entry_num == ExtrasWebsitesEntry)
		{
			websites->Draw();
			for(int i=0; i<9; i++)
				website_box[i]->Draw(0);
		}
	}
	else
	{
		prompt->Draw();
		entry[ExtrasDemoEntryYes]->Draw();
		entry[ExtrasDemoEntryNo]->Draw();
	}

	extras->Draw();
}


extern char *g_ksps_path;
extern const char *g_thps4_path;

void ExtrasFrontEnd::Select(int entry_index)
{
	int i;
#ifdef TARGET_PS2
  char *args[] = { "bootstrap", g_ksps_path };
#endif
  switch(highlighted->entry_num)
	{
	case ExtrasBeachesEntry:	
		if(!beaches->highlighted->GetDisable())
		{
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			PlayMovie(1); 
		}
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		break;
	case ExtrasSurfersEntry:	
		if(!surfers->highlighted->GetDisable()) 
		{
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			PlayMovie(0); 
		}
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		break;
	case ExtrasTrailersEntry:
		if(!trailers->highlighted->GetDisable()) 
			PlayMovie(2); 
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		break;
	case ExtrasHighScoreEntry:				
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		sys->MakeActive(GraphicalMenuSystem::HighScoreMenu); 
		break;
	case ExtrasScrapbookEntry:	
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		sys->menus[GraphicalMenuSystem::AccompMenu]->setBack(this);   
		sys->MakeActive(GraphicalMenuSystem::AccompMenu); 
		break;
	case ExtrasCheatsEntry:		
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		MakeActive(cheat_menu); 
		break;
	case ExtrasCreditsEntry:	
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		sys->MakeActive(GraphicalMenuSystem::CreditsMenu); 
		break;
	case ExtrasWebsitesEntry:	
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		break;
	case ExtrasLogbookEntry:
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		sys->MakeActive(GraphicalMenuSystem::LogbookMenu); 
		break;

#ifdef TARGET_PS2
	case ExtrasTHDemoEntry:
		for (i=0; i < 	ExtrasDemoEntryYes; i++)
			entry[i]->Disable(true);
		entry[ExtrasDemoEntryYes]->Disable(false);
		entry[ExtrasDemoEntryNo]->Disable(false);
		saveCareerPrompt = true;
		setHigh(entry[ExtrasDemoEntryYes]);
		break;
#endif
	case ExtrasDemoEntryYes:
#ifdef TARGET_PS2
		scePcStart( 0, 0, 0 );
		sceCdStop();				// stop Cd
		sceCdStStop();				// also stop streaming....	   Yes!!
		sceCdSync(0);				// wait for commands to execute
		sceCdDiskReady(0);		   	// wait for Cd to be ready again
	
		sceSifExitCmd();			// Sony suggested fix to make LoadExecPS2 work more reliably. 
		LoadExecPS2( g_thps4_path, 2, args );
#endif
		break;
#ifdef TARGET_PS2
	case ExtrasDemoEntryNo:
		for (i=0; i < 	ExtrasDemoEntryYes; i++)
			entry[i]->Disable(false);
		entry[ExtrasDemoEntryYes]->Disable(true);
		entry[ExtrasDemoEntryNo]->Disable(true);
		saveCareerPrompt = false;
		setHigh(entry[ExtrasTHDemoEntry]);
		break;
#endif
	default: assert(0);
	}
}

void ExtrasFrontEnd::OnTriangle(int c)
{
	int i;
	if(active)
		active->OnTriangle(c);
	else
	{
		if (!saveCareerPrompt)
			sys->MakeActive(GraphicalMenuSystem::MainMenu);
#ifdef TARGET_PS2
		else
		{
			for (i=0; i < 	ExtrasDemoEntryYes; i++)
				entry[i]->Disable(false);
			entry[ExtrasDemoEntryYes]->Disable(true);
			entry[ExtrasDemoEntryNo]->Disable(true);
			saveCareerPrompt = false;
			setHigh(entry[ExtrasTHDemoEntry]);
		}
#endif
		SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	}
}

void ExtrasFrontEnd::OnActivate()
{
	manager->em->ToMainScreen();
	cur_index = 1;
	
	int count = 0;	// count of valid movies
	FEMenuEntry* tmp = surfers->entries;
	for(int i=0; i<SURFER_LAST; i++)
	{
		if(SurferDataArray[i].has_movie == 1) // && )
		{
			count++;
			tmp->Disable(false);
			if(unlockManager.isSurferMovieUnlocked(i))
				tmp->SetText(SurferDataArray[i].firstname + stringx("\n") + SurferDataArray[i].lastname + stringx("\n") + ksGlobalTextArray[GT_FE_MENU_MOVIE_FULL]);
			else
				tmp->SetText(SurferDataArray[i].firstname + stringx("\n") + SurferDataArray[i].lastname + stringx("\n") + ksGlobalTextArray[GT_FE_MENU_MOVIE_SHORT]);
		}
		else
			tmp->Disable(true);
		
		tmp = tmp->next;
	}
	entry[ExtrasSurfersEntry]->Disable(count == 0);

	count = 0;
	tmp = beaches->entries;
	for(int i=0; i<BEACH_LAST; i++)
	{
		if(BeachDataArray[i].has_movie == 1 && unlockManager.isLocationMovieUnlocked(BeachDataArray[i].map_location))
		{
			count++;
			tmp->Disable(false);
		}
		else tmp->Disable(true);
		tmp = tmp->next;
	}
	entry[ExtrasBeachesEntry]->Disable(count == 0);

  tmp = trailers->entries->next;
  if(unlockManager.isBailsMovieUnlocked())
    tmp->Disable(false);
  else
    tmp->Disable(true);

  tmp = tmp->next;
  if(unlockManager.isEspnMovieUnlocked())
    tmp->Disable(false);
  else
    tmp->Disable(true);

	// disable scrapbook and logbook if no valid career
	entry[ExtrasScrapbookEntry]->Disable(!g_career->IsStarted());
	entry[ExtrasLogbookEntry]->Disable(!g_career->IsStarted());
	entry[ExtrasHighScoreEntry]->Disable(!g_career->IsStarted());
	FEMultiMenu::OnActivate();
	
	if(manager->extras_movie_sub != -1)
		setHigh(entry[manager->extras_movie_sub]);
	
	UpdateState();
	
	// If returning from an FE movie, return to the menu entry for that movie - rbroner
	if(manager->extras_movie_sub_sub != -1)
	{
		FEMenuEntry *tmp;
		if(manager->extras_movie_sub == ExtrasBeachesEntry)
		{
			tmp = beaches->entries;
			for(int e=0; e<beaches->num_entries; e++)
			{
				if(tmp == NULL || tmp->entry_num == manager->extras_movie_sub_sub)
					break;
				
				tmp = tmp->next;
			}
			if(tmp && tmp->entry_num == manager->extras_movie_sub_sub)
				beaches->setHigh(tmp);
		}
		else if(manager->extras_movie_sub == ExtrasSurfersEntry)
		{
			tmp = surfers->entries;
			for(int e=0; e<surfers->num_entries; e++)
			{
				if(tmp == NULL || tmp->entry_num == manager->extras_movie_sub_sub)
					break;
				
				tmp = tmp->next;
			}
			if(tmp && tmp->entry_num == manager->extras_movie_sub_sub)
				surfers->setHigh(tmp);
		}
	}
	manager->extras_movie_sub     = -1;
	manager->extras_movie_sub_sub = -1;
}

void ExtrasFrontEnd::UpdateHelpbar()
{
	manager->helpbar->Reset();

	switch(highlighted->entry_num)
	{
	case ExtrasBeachesEntry:
	case ExtrasSurfersEntry:
	case ExtrasTrailersEntry:
		manager->helpbar->AddArrowBoth();
		break;
	case ExtrasCreditsEntry:
	case ExtrasCheatsEntry:
	case ExtrasScrapbookEntry:
	case ExtrasLogbookEntry:
#ifdef TARGET_PS2
	case ExtrasTHDemoEntry:
#endif
	case ExtrasHighScoreEntry:
		manager->helpbar->AddArrowV();
		break;
	case ExtrasWebsitesEntry:
		manager->helpbar->AddArrowV();
		manager->helpbar->RemoveX();
		break;
	}

	manager->helpbar->Reformat();
}

void ExtrasFrontEnd::OnUp(int c)
{
	if(active)
		active->OnUp(c);
	else
	{
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		FEMultiMenu::OnUp(c);
		UpdateState();
	}
}

void ExtrasFrontEnd::OnDown(int c)
{
	if(active)
		active->OnDown(c);
	else
	{
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		FEMultiMenu::OnDown(c);
		UpdateState();
	}
}

void ExtrasFrontEnd::OnLeft(int c)
{
	if(active)
		active->OnLeft(c);
	switch(highlighted->entry_num)
	{
	case ExtrasBeachesEntry:	
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		beaches->OnLeft(c); break;
	case ExtrasSurfersEntry:	
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		surfers->OnLeft(c); break;
	case ExtrasTrailersEntry:	
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		trailers->OnLeft(c); break;
	}

	if(highlighted->entry_num <= ExtrasTrailersEntry)
	{
		arrows[0][1]->TurnOn(true);
		arrows[0][0]->TurnOn(false);
		arrow_num = 0;
		arrow_counter = arrow_timer;
	}
}

void ExtrasFrontEnd::OnRight(int c)
{
	if(active)
		active->OnRight(c);
	switch(highlighted->entry_num)
	{
	case ExtrasBeachesEntry:
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		beaches->OnRight(c); break;
	case ExtrasSurfersEntry:
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		surfers->OnRight(c); break;
	case ExtrasTrailersEntry:
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		trailers->OnRight(c); break;
	}

	if(highlighted->entry_num <= ExtrasTrailersEntry)
	{
		arrows[1][1]->TurnOn(true);
		arrows[1][0]->TurnOn(false);
		arrow_num = 1;
		arrow_counter = arrow_timer;
	}
}

void ExtrasFrontEnd::UpdateState()
{
	bool on_beach = highlighted->entry_num == ExtrasBeachesEntry;
	bool on_surfer = highlighted->entry_num == ExtrasSurfersEntry;
	bool on_trailer = highlighted->entry_num == ExtrasTrailersEntry;
	if(on_beach) beaches->OnActivate();
	else if(on_surfer) surfers->OnActivate();
	else if(on_trailer) trailers->OnActivate();

	arrows[0][1]->TurnOn(false);
	arrows[0][0]->TurnOn(on_beach || on_surfer || on_trailer);
	arrows[1][1]->TurnOn(false);
	arrows[1][0]->TurnOn(on_beach || on_surfer || on_trailer);

	for(int i=0; i<ExtrasEndEntry; i++)
		if(videos[i]) videos[i]->TurnOn(i == highlighted->entry_num);

	UpdateHelpbar();
}

void ExtrasFrontEnd::PlayMovie(int sbt)
{
	stringx tmp;
	switch(sbt)
	{
	case 0:
    // Play short movie if long movie hasn't been unlocked 
    if(unlockManager.isSurferMovieUnlocked(surfers->highlighted->entry_num))
      tmp = stringx("SURFERS\\MOVIE_")+SurferDataArray[surfers->highlighted->entry_num].abbr;
    else
      tmp = stringx("SURFERS\\SHORT_")+SurferDataArray[surfers->highlighted->entry_num].abbr;
    break;
	case 1: tmp = stringx("BEACHES\\")+BeachDataArray[beaches->highlighted->entry_num].stashfile; break;
	case 2:
    if(trailers->highlighted->entry_num == 0)
      tmp = stringx("KSINTRO");
    else if(trailers->highlighted->entry_num == 1)
      tmp = stringx("DROWNING");
    else if(trailers->highlighted->entry_num == 2)
      tmp = stringx("ESPN");
    else
      return;
    break;
	default: return;
	}

	// play special surfer movie
	manager->gms->killMovie();
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{      
		MusicMan::inst()->stop();
		MusicMan::inst()->pause();
	}
	manager->extras_movie = true;
	switch(sbt)
	{
	case 0: manager->extras_movie_sub_sub = surfers->highlighted->entry_num; break;
	case 1: manager->extras_movie_sub_sub = beaches->highlighted->entry_num; break;
	case 2: manager->extras_movie_sub_sub = trailers->highlighted->entry_num; break;
	default: return;
  }
  manager->extras_movie_sub = highlighted->entry_num;


#if defined(TARGET_GC)
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		memcpy(&g_career->cfg, StoredConfigData::inst()->getGameConfig(), sizeof(ksConfigData));
		nslShutdown( );
	}

	g_game_ptr->play_movie( tmp.c_str( ), true );

	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		if (!nslInit())
		{
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
			// Xbox requires a message on all disk read failures.  (dc 07/11/02)
			disk_read_error();
#endif
		}
		nslSetRootDir( "SOUNDS" );
		nslReset( "FRONTEND", NSL_LANGUAGE_ENGLISH );
		StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
	}
#elif defined(TARGET_PS2)
	manager->gms->PrepareToExit(true);
	g_game_ptr->set_movie(tmp);
	
#else
	g_game_ptr->play_movie(tmp.data(),true);
#endif
	
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		MusicMan::inst()->loadSources();
		MusicMan::inst()->unpause();
		MusicMan::inst()->playNext();
	}
	manager->gms->restartMovie();
}


/*********************** CreditsFrontEnd ********************************************/

CreditsFrontEnd::CreditsFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;

	flags |= FEMENU_USE_SCALE;
	scale = scale_high = 0.666f;

	color = manager->white;
	color_high = manager->yel_dk;

	// default menu entry; not used
	Add(NEW FEMenuEntry("", this));

	x_all = 80;
	credits = NEW PreformatText(&frontendmanager.font_bold, "", x_all, 130, 0, scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color);

	first_line_y = 320;
	up_pressed = false;
	down_pressed = false;
}

void CreditsFrontEnd::Load()
{
	FEMultiMenu::Load();

	stringx file = "credits";
	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
		file += "_fr";
	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
		file += "_ge";
	file += ".txt";

	char* tmp = (char*) (path+file).data();

	credits->readText(tmp, num_lines, 10);
}

void CreditsFrontEnd::Update(time_value_t time_inc)
{
	static const float speed = 32.0f;
	if(up_pressed)
	{
		first_line_y += 4*speed*time_inc;

		if (first_line_y > 320)
			first_line_y = 320;
	}
	else if(down_pressed)
		first_line_y -= 4*speed*time_inc;
	else first_line_y -= speed*time_inc;

#if defined(TARGET_XBOX)
  // If screen is running at 60 fps, advance once. Else advance twice.
  int ticks = (int)(time_inc*60.0f + 0.5f);
  nvlAdvance();
  if(ticks > 1)
    nvlAdvance();
#endif
}

void CreditsFrontEnd::Draw()
{
#if !defined(TARGET_XBOX)
	FEMultiMenu::Draw();
#endif

	for(int i=0; i<num_lines; i++)
	{
		float y = first_line_y+i*20;
		if(y > -10 && y < 490) credits->DrawLine(i, x_all, y);
	}

	// go back to Extras Screen if credits finish
	if(first_line_y+(credits->GetActualLines()-1)*20 < 0)
  {
#if defined(TARGET_XBOX)
    movieplayer::inst()->stop();
    movieplayer::inst()->shutdown();
#endif
    sys->MakeActive(GraphicalMenuSystem::ExtrasMenu);
  }
#if defined(TARGET_XBOX)
	else
	{
		// if we add the last frame and destroy the texture right away it's going to crash when we render -lz (6/27/02)
	  movieplayer::inst()->start_frame(false);
		movieplayer::inst()->end_frame(false, false);
	}
#endif
}

void CreditsFrontEnd::OnTriangle(int c)
{
#if defined(TARGET_XBOX)
	movieplayer::inst()->stop();
	movieplayer::inst()->shutdown();
#endif
	sys->MakeActive(GraphicalMenuSystem::ExtrasMenu);
}

void CreditsFrontEnd::OnCross(int c)
{
  OnTriangle(c);
}

void CreditsFrontEnd::OnStart(int c)
{
  OnTriangle(c);
}


void CreditsFrontEnd::OnActivate()
{
	FEMultiMenu::OnActivate();
	up_pressed = false;
	down_pressed = false;
	first_line_y = 320;

	manager->helpbar->DisableHelpbar();

//#if defined(TARGET_PS2)
//	movieplayer::inst()->init();
//	movieplayer::inst()->play("\\MOVIES\\CREDITS.IPU;1", true, true, true, 0x100000);
//	if(!movieplayer::inst()->is_playing())
//		movieplayer::inst()->play("host0:MOVIES\\CREDITS.IPU", true, true, true, 0x100000);
//	nvlAdvance();
//	nvlAdvance();   // Advance the movie a couple frames so it doesn't display a white screen the first frame
//#endif
	
#if defined(TARGET_XBOX)
	movieplayer::inst()->init();
	movieplayer::inst()->play("d:\\MOVIES\\CREDITS.bik", true, true, true);
	nvlAdvance();
	nvlAdvance();   // Advance the movie a couple frames so it doesn't display a white screen the first frame
#endif
}

void CreditsFrontEnd::OnButtonRelease(int c, int b)
{
	if(b == FEMENUCMD_UP) up_pressed = false;
	else if(b == FEMENUCMD_DOWN) down_pressed = false;
}
