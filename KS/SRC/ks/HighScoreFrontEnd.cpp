// HighScoreFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if _XBOX
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "FrontEndManager.h"
#include "HighScoreFrontEnd.h"
#include "GlobalData.h"
#include "unlock_manager.h"

HighScoreFrontEnd::HighScoreFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name, bool i_g)
{
	cons(s, man, p, pf_name);
	in_game = i_g;

	float sc_lg = 0.888f*1.18f;
	float sc_sm = 0.666f*1.18f;
	scale = scale_high = sc_lg;
	color = color_high = manager->col_highlight2;
	labelColor = manager->col_menu_header;

	Font * font = &manager->font_bold_old;

	for(int i=0; i<BEACH_LAST*2; i++)
	{
		int bn = i;
		if(i >= BEACH_LAST) bn -= BEACH_LAST;
		stringx tmp = BeachDataArray[bn].fe_name;

		if(i >= BEACH_LAST) tmp = tmp + " " + ksGlobalTextArray[GT_FE_MENU_ICON];
		all_beaches[i] = NEW FEMenuEntry(tmp, this, false, font);
		Add(all_beaches[i]);
		all_beaches[i]->SetPos(320, 74);
	}

	FEMenuEntry* tmp = entries;
	while(tmp)
	{
		tmp->left = tmp->previous;
		tmp->right = tmp->next;
		if(tmp->next == NULL) tmp->right = entries;
		tmp = tmp->next;
	}

	int col_centers[NUM_COLUMNS] = { 102, 159, 308, 466, 535 };

	for(int i=0; i<NUM_COLUMNS; i++)
	{
		column_labels[i] = NEW TextString(font, ksGlobalTextArray[GT_FE_MENU_RANK+i], col_centers[i], 101, 0, sc_sm, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, labelColor);
		for(int j=0; j<NUM_ROWS; j++)
		{
			stringx str = i == 0 ? stringx(j+1) : stringx("");
			color32 col = (i == 1 || i == 3) ? manager->col_info_b : manager->col_info_g;
			info[j][i] = NEW TextString(font, str, col_centers[i], 121 + j*20, 0, sc_sm, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, col);
		}
	}

	if(!in_game) sys = (GraphicalMenuSystem*) s;
	arrow_counter = -5;
	first_line = -1;
	second_line = -1;
	flash_timer = 0;

	// Initialize help text at bottom of screen.
	if(in_game)
	{
		SetHelpText(HF_CONTINUE);
		helpText->changePos(320, 424);
	}
	else SetHelpText(0);

	nem = NEW NameEntryMenu(s, man, in_game);
	AddSubmenu(nem);
	nem->ClearName();
}

HighScoreFrontEnd::~HighScoreFrontEnd()
{
	for(int i=0; i<NUM_COLUMNS; i++)
	{
		for(int j=0; j<NUM_ROWS; j++)
			delete info[j][i];
		delete column_labels[i];
	}

	delete nem;
}

void HighScoreFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();
	if(in_game) nem->SetPQIndices();
}

void HighScoreFrontEnd::Update(time_value_t time_inc)
{
	if(active) active->Update(time_inc);
	else
	{
		if(in_game && first_line != -1)
		{
			// flashing
			float period = 
				(float)(os_developer_options::inst()->get_int(os_developer_options::INT_MENU_HIGHLIGHT_PERIOD)) / 
				MENU_HIGHLIGHT_MULTIPLIER;
			
			flash_timer += time_inc;
			float fl = THROB_INTENSITY * sinf(6.28f*(flash_timer/period));

			color32 c;
			float r, g, b, a;
			for(int i=0; i<NUM_COLUMNS; i++)
			{
				c = (i==1 || i==3) ? manager->col_info_b : manager->col_info_g;
				r = FTOI(c.get_red() + (fl+.5f)*(labelColor.get_red() - c.get_red()));
				g = FTOI(c.get_green() + (fl+.5f)*(labelColor.get_green() - c.get_green()));
				b = FTOI(c.get_blue() + (fl+.5f)*(labelColor.get_blue() - c.get_blue()));
				a = labelColor.get_alpha();
				info[first_line][i]->color = color32(r, g, b, a);
			}
		}
		FEMultiMenu::Update(time_inc);
	}
}

void HighScoreFrontEnd::Draw()
{
	if(arrow_counter >= 0) arrow_counter--;
	if(arrow_counter < 0 && arrow_counter > -5)
	{
		arrow_counter = -5;
		arrows[0][0]->TurnOn(!in_game);
		arrows[0][1]->TurnOn(false);
		arrows[1][0]->TurnOn(!in_game);
		arrows[1][1]->TurnOn(false);
	}

	panel.Draw(0);

	if (active)
		active->Draw();
	else
	{
		highlighted->Draw();

		for(int i=0; i<NUM_COLUMNS; i++)
		{
			// don't draw last row, unless on Icon Challenge screen
			if(i != NUM_COLUMNS-1 || highlighted->entry_num >= BEACH_LAST)
			{
				for(int j=0; j<NUM_ROWS; j++)
					info[j][i]->Draw();
				column_labels[i]->Draw();
			}
		}

		helpText->Draw();
	}
}

void HighScoreFrontEnd::Select(int entry_index)
{
	if(in_game)
	{
		if(first_line != -1)
		{
			// reset the flashing color correctly
			for(int i=0; i<NUM_COLUMNS; i++)
				info[first_line][i]->color = (i==1 || i==3) ? manager->col_info_b : manager->col_info_g;
		}

		if(second_line != -1)
		{
			first_line = second_line;
			second_line = -1;
			setHigh(all_beaches[g_game_ptr->get_beach_id()]);
			OnSwitchBeach();
		}
		else
		{
			//manager->IGO->ShowMenuBackground(true);
			//system->MakeActive(next_menu);
			SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			SoundScriptManager::inst()->pause();
			system->endDraw(false);
		}
	}
}

bool HighScoreFrontEnd::IsNewHighScore(const int score, const int icons)
{
	bool icon_mode = g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON;
	int lowest_high = globalCareerData.getBeachHighScore(g_game_ptr->get_beach_id(), NUM_ROWS-1, icon_mode).score;
	int lowest_high_i = globalCareerData.getBeachHighScore(g_game_ptr->get_beach_id(), NUM_ROWS-1, icon_mode).icons;
	
	// Dont' allow high scores if player cheated.
	for(int i=0; i<CHEAT_LAST; i++)
	{
		if (g_session_cheats[i].isOn())
			return false;
	}
	// don't allow new high score if any cheats are used
	if ((!icon_mode && score > lowest_high || (icon_mode && icons > lowest_high_i)))
		return true;
	
	return false;	
}

void HighScoreFrontEnd::OnActivate()
{
	first_line = -1;
	second_line = -1;

	if(!in_game)
	{
		for(int i=0; i<BEACH_LAST; i++)
		{
			all_beaches[i]->Disable(!unlockManager.isBeachUnlocked(i) || BeachDataArray[i].map_location == MAP_LOC_NOWHERE);
			all_beaches[i+BEACH_LAST]->Disable(!unlockManager.isBeachUnlocked(i) || BeachDataArray[i].map_location == MAP_LOC_NOWHERE);
		}
	}

	FEMultiMenu::OnActivate();

	if(in_game)
	{
		bool icon_mode = g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON;
		int score = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore();
		int icons = 0;
		
		if (manager->IGO->GetIconManager())
			icons = manager->IGO->GetIconManager()->IconsCleared();

		if (IsNewHighScore(score, icons))
		{
			if (nem->NameEntered())
				SetString(nem->GetName());
			else if (nem->WasCanceled())
				nem->ResetCanceled();
			else
				MakeActive(nem);
		}
		
		if (icon_mode)
			setHigh(all_beaches[g_game_ptr->get_beach_id() + BEACH_LAST]);
		else
			setHigh(all_beaches[g_game_ptr->get_beach_id()]);

		// make sure we get a new name next time we ask for initials
		nem->ClearName();

		TurnHighScore(!active);
		nem->TurnNameEntry(active);
	}

	OnSwitchBeach();

	if (!in_game)
	{
		manager->helpbar->Reset();
		manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
		manager->helpbar->RemoveX();
		manager->helpbar->Reformat();
	}
}

void HighScoreFrontEnd::OnTriangle(int c)
{
	if(!in_game) 
	{
		sys->MakeActive(GraphicalMenuSystem::ExtrasMenu);
		SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	}
}

void HighScoreFrontEnd::OnUp(int c)
{
	if(in_game && active) active->OnUp(c);
}

void HighScoreFrontEnd::OnDown(int c)
{
	if(in_game && active) active->OnDown(c);
}

void HighScoreFrontEnd::OnLeft(int c)
{
	if(in_game)
	{
		if(active) active->OnLeft(c);
		return;
	}

	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	FEMultiMenu::OnLeft(c);
	OnSwitchBeach();

	arrows[0][1]->TurnOn(!in_game);
	arrows[0][0]->TurnOn(false);
	arrow_num = 0;
	arrow_counter = arrow_timer;
}

void HighScoreFrontEnd::OnRight(int c)
{
	if(in_game)
	{
		if(active) active->OnRight(c);
		return;
	}

	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	FEMultiMenu::OnRight(c);
	OnSwitchBeach();
	
	arrow_counter = arrow_timer;
	arrows[1][1]->TurnOn(!in_game);
	arrows[1][0]->TurnOn(false);
	arrow_num = 1;
}

void HighScoreFrontEnd::OnStart(int c)
{
	if (in_game)
	{
		if (active)
			active->OnStart(c);
#if defined(TARGET_XBOX)
		else
			Select(0);
#endif
	}
}

void HighScoreFrontEnd::SetPQIndices()
{
	arrows[0][0] = GetPointer("hs_button_off_01");
	arrows[0][1] = GetPointer("hs_button_on_01");
	arrows[1][0] = GetPointer("hs_button_off_02");
	arrows[1][1] = GetPointer("hs_button_on_02");

	static const int arrow_offset = 50;
	float x, y;
	arrows[0][0]->GetCenterPos(x, y);
	arrows[0][0]->SetCenterPos(x-arrow_offset, y);
	arrows[0][1]->GetCenterPos(x, y);
	arrows[0][1]->SetCenterPos(x-arrow_offset, y);
	arrows[1][0]->GetCenterPos(x, y);
	arrows[1][0]->SetCenterPos(x+arrow_offset, y);
	arrows[1][1]->GetCenterPos(x, y);
	arrows[1][1]->SetCenterPos(x+arrow_offset, y);

	arrows[0][1]->TurnOn(false);
	arrows[1][1]->TurnOn(false);
	arrows[0][0]->TurnOn(!in_game);
	arrows[1][0]->TurnOn(!in_game);

	line = GetPointer("hs_line_19");

	if(in_game)
	{
		for(int i=0; i<19; i++)
			if(i < 9) lines[i] = GetPointer(("hs_line_0"+stringx(i+1)).data());
			else lines[i] = GetPointer(("hs_line_"+stringx(i+1)).data());
		fade = GetPointer("hs_fade");
	}
}

void HighScoreFrontEnd::TurnHighScore(bool on)
{
	arrows[0][1]->TurnOn(false);
	arrows[1][1]->TurnOn(false);
	arrows[0][0]->TurnOn(!in_game && on);
	arrows[1][0]->TurnOn(!in_game && on);

	if(in_game)
	{
		for(int i=0; i<19; i++)
			lines[i]->TurnOn(on);
		fade->TurnOn(on);
	}
}

void HighScoreFrontEnd::SetString(stringx str)
{
	int score = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore();
	int icons = -1;
	IGOIconManager* iim = manager->IGO->GetIconManager();
	if(iim && g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON)
	{
		icons = iim->IconsCleared();
		int lowest_high_i = globalCareerData.getBeachHighScore(g_game_ptr->get_beach_id(), NUM_ROWS-1, true).icons;
		if(icons <= lowest_high_i) icons = -1;
	}
	int character = g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player());
	first_line = manager->AddHighScore(str, score, icons, character);
}

void HighScoreFrontEnd::OnSwitchBeach()
{
	bool icon_mode = highlighted->entry_num >= BEACH_LAST;
	HighScoreData hsd;
	for(int i=0; i<NUM_ROWS; i++)
	{
		int bn = highlighted->entry_num;
		if(icon_mode) bn -= BEACH_LAST;
		hsd = globalCareerData.getBeachHighScore(bn, i, icon_mode);
		info[i][1]->changeText(hsd.initials);
		info[i][2]->changeText(hsd.character);
		info[i][3]->changeText(stringx(hsd.score));
		if(icon_mode)
			info[i][4]->changeText(stringx(hsd.icons));
	}
	line->TurnOn(!active && icon_mode);

	// recenter the 4th column
	int center = (!icon_mode) ? 489 : 466;
	column_labels[3]->changePos(center, 101);
	for(int j=0; j<NUM_ROWS; j++)
		info[j][3]->changePos(center, 121 + j*20);

	flash_timer = 0;
}

/************************************************************************/

NameEntryMenu::NameEntryMenu(FEMenuSystem* s, FEManager* man, const bool in_g)
{
	in_game = in_g;
	cons(s, man, "", "");

	Font* font = &manager->font_bold_old;

	float sc_sm = 0.666f;
	scale = scale_high = sc_sm;

	color = manager->blue_dk;
	color_high = manager->col_highlight2;
	color_high_alt = manager->col_highlight;

	int x_pos = 230;
	int y_pos = 152;
	int inc = 30;

	for(int i=0; i<41; i++)
	{
		stringx str = "";
		if(i < 26)
		{
			char c[2] = "A";
			c[0] = 'A'+i;
			str = stringx(c);
		}
		else if(i < 36) str = stringx(i-26);
		else if(i == 36) str = "<";
		else if(i == 37) str = "_";
		else if(i == 38) str = ksGlobalTextArray[GT_FE_MENU_CLEAR];
		else if(i == 39) str = ksGlobalTextArray[GT_FE_MENU_ENTER];
		else str = ksGlobalTextArray[GT_FE_MENU_CANCEL];

		entry[i] = NEW FEMenuEntry(str, this, false, font);
		Add(entry[i]);
		if(i < 36) entry[i]->SetPos(x_pos+inc*(i%7), y_pos+inc*(i/7));
		else if(i == 36) entry[i]->SetPos(x_pos+inc*(i%7), y_pos+inc*(i/7)-2);
		else if(i == 37) entry[i]->SetPos(x_pos+inc*(i%7), y_pos+inc*(i/7)-5);
		else if(i == 38) entry[i]->SetPos(x_pos+inc*4, y_pos+inc*(i/7));
		else if(i == 39) entry[i]->SetPos(275, 358);
		else entry[i]->SetPos(357, 358);

		if(i > 38) entry[i]->SetSpecialColor(manager->col_unselected, color_high);
	}

	for(int i=0; i<41; i++)
	{
		entry[i]->right = entry[i]->next;
		if(!entry[i]->right) entry[i]->right = entry[0];
		entry[i]->left = entry[i]->previous;

		if(i < 38)
		{
			if(i >= 7) entry[i]->up = entry[i-7];
			else if(i > 2) entry[i]->up = entry[40];
			else entry[i]->up = entry[39];
			if(i < 31) entry[i]->down = entry[i+7];
			else if(i < 35) entry[i]->down = entry[38];
			else entry[i]->down = entry[39];
		}
	}

	entry[38]->up = entry[32];
	entry[38]->down = entry[40];
	entry[39]->up = entry[37];
	entry[39]->down = entry[2];
	entry[40]->up = entry[38];
	entry[40]->down = entry[4];

	high_score = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_HIGH_SCORES], 320, 73, 0, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_menu_header);
	enter_name = NEW TextString(font, ksGlobalTextArray[GT_FE_MENU_ENTER_INITIALS], 320, 116, 0, sc_sm, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_unselected);
	name = NEW TextString(font, "", 320, 333, 0, sc_sm, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->blue_dk);
	name_count = 0;
	name_entered = false;
	canceled = false;

	// Initialize help text at the bottom of menu.
	SetHelpText(HF_UP | HF_DOWN | HF_LEFT | HF_RIGHT | HF_SELECT);
	helpText->changePos(320, 424);
}
void NameEntryMenu::Update(time_value_t time_inc)
{
	// this was to disable ENTER when nothing was entered or only spaces entered
	/*
	stringx s = name->getText();
	s.remove_surrounding_whitespace();
	if (s.length() == 0)
		entry[39]->Disable(true);
	else
		entry[39]->Disable(false);
		*/
	FEMultiMenu::Update(time_inc);
}

NameEntryMenu::~NameEntryMenu()
{
	delete high_score;
	delete enter_name;
	delete name;
}

void NameEntryMenu::Select(int entry_index)
{
	stringx str = name->getText();

	if(entry_index > 38)
	{
		if(entry_index == 39)// <enter>
		{	
			g_career->SetInitials(name->getText());
			name_entered = true;
			SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			SoundScriptManager::inst()->pause();
		}
		else
		{
			canceled = true;
			SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			SoundScriptManager::inst()->pause();
		}
		parent->MakeActive(NULL);
		return;
	}


	char tmp[4];
	for(int i=0; i<name_count; i++)
		tmp[i] = str.data()[i];
	tmp[name_count] = '\0';

	if(name_count < 3 && (entry_index < 36 || entry_index == 37))
	{
		if(entry_index < 26) tmp[name_count] = 'A'+entry_index;
		else if(entry_index < 36) tmp[name_count] = '0'+(entry_index-26);
		else tmp[name_count] = ' ';
		name_count++;
		tmp[name_count] = '\0';
		if(name_count == 3)
		{
			stringx s = name->getText();
			s.remove_surrounding_whitespace();
			if (s.length() != 0)
				setHigh(entry[39]);
		}
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		SoundScriptManager::inst()->pause();
	}
	else if(entry_index == 36)
	{
		name_count--;
		if(name_count < 0) name_count = 0;
		tmp[name_count] = '\0';
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		SoundScriptManager::inst()->pause();
	}
	else if(entry_index == 38)
	{
		name_count = 0;
		tmp[0] = '\0';
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		SoundScriptManager::inst()->pause();
	}
	else
	{
		SoundScriptManager::inst()->unpause();
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		SoundScriptManager::inst()->pause();
	}

	name->changeText(tmp);
}

void NameEntryMenu::Draw()
{
	FEMultiMenu::Draw();
	high_score->Draw();
	enter_name->Draw();
	name->Draw();
	helpText->Draw();
}

void NameEntryMenu::OnActivate()
{
	FEMultiMenu::OnActivate();
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		name->changeText(g_career->GetInitials());
		if (name->getText().length() > 0)
			setHigh(entry[39]);
		else 
			setHigh(entry[0]);
	}
	else
		name->changeText("");
	name_count = name->getText().length();
	name_entered = false;
	
}

void NameEntryMenu::SetPQIndices()
{
	for(int i=0; i<9; i++)
		box[i] = GetPointer(("hs_box_0"+stringx(i+1)).data());
	line = GetPointer("hs_line_top_01");
	for(int i=0; i<3; i++)
		name_box[i] = GetPointer(("hs_name_box"+stringx(i+1)).data());
	for(int i=0; i<41; i++)
	{
		stringx str = "";
		if(i < 26)
		{
			char c[2] = "a";
			c[0] = 'a'+i;
			str = stringx(c);
		}
		else if(i < 36) str = stringx(i-26);
		else if(i == 36) str = "back";
		else if(i == 37) str = "space";
		else str = "clear"+stringx(i-37);
		keys[i][0] = GetPointer(("hs_key_btn_off_"+str).data());
		keys[i][1] = GetPointer(("hs_key_btn_on_"+str).data());
	}
}

void NameEntryMenu::OnUp(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMultiMenu::OnUp(c);
}

void NameEntryMenu::OnDown(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	SoundScriptManager::inst()->pause();
	FEMultiMenu::OnDown(c);
}

void NameEntryMenu::OnLeft(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	SoundScriptManager::inst()->pause();
	FEMultiMenu::OnLeft(c);
}

void NameEntryMenu::OnRight(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	SoundScriptManager::inst()->pause();
	FEMultiMenu::OnRight(c);
}

void NameEntryMenu::OnStart(int c)
{
	if (!in_game)
		FEMenu::OnStart(c);
	else
	{
#if defined(TARGET_XBOX)
		OnCross(c);
#endif
	}
}

void NameEntryMenu::TurnNameEntry(bool on)
{
	for(int i=0; i<9; i++)
		box[i]->TurnOn(on);
	for(int i=0; i<3; i++)
		name_box[i]->TurnOn(on);
	line->TurnOn(on);
	for(int i=0; i<41; i++)
	{
		keys[i][0]->TurnOn(on);
		keys[i][1]->TurnOn(false);
	}
}
