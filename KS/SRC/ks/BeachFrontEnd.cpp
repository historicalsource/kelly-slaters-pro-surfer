// BeachFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "BeachFrontEnd.h"
#include "Map.h"
#include "PathData.h"
#include "VOEngine.h"
#include "unlock_manager.h"
#if defined(TARGET_XBOX)
#include "FrontEndManager.h"
#endif /* TARGET_XBOX JIV DEBUG */
extern nslSoundId feMusic;
bool dampened = false;
BeachFrontEnd::BeachFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);

	exitingWithoutSelect = false;
	scale = scale_high = 0.89f;
	float sc_chall = 1.0f; //0.78f;
	float sc_of = 0.8f;//0.6f;
	float sc_sm = 0.666f;
//	float sc_tiny = 0.5f;
	float sc_goal = 0.8f; //0.5f;

	color = manager->col_unselected;
	color_high = manager->col_highlight;
	color_high_alt = manager->col_highlight2;

	flags |= FEMENU_NORM_COLOR_NO_FLASH | FEMENU_HAS_COLOR_HIGH_ALT;
	voiceOverStage = 0;
	for(int i=0; i<MAP_LOC_LAST; i++)
	{
		for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
		{
			locations[i].levels[j] = -1;
			locations[i].beaches[j] = -1;
		}
		locations[i].all_levels_locked = true;
		locations[i].all_beaches_locked = true;
	}
	for(int i=0; i<BEACH_LAST; i++)
	{
		beaches[i].locked = false;
		beaches[i].main_beach = -1;

		for(int j=0; j<MAX_BEACHES_PER_LOC-1; j++)
			beaches[i].secondary_beaches[j] = -1;

		int loc = BeachDataArray[i].map_location;
		beaches[i].location = loc;

		if(loc == MAP_LOC_NOWHERE) continue;

		if(locations[loc].beaches[0] == -1)
			locations[loc].beaches[0] = i;
		else if(locations[loc].beaches[1] == -1)
		{
			locations[loc].beaches[1] = i;
			beaches[i].main_beach = locations[loc].beaches[0];
			beaches[locations[loc].beaches[0]].secondary_beaches[0] = i;
		}
		else
		{
			locations[loc].beaches[2] = i;
			beaches[i].main_beach = locations[loc].beaches[0];
			beaches[locations[loc].beaches[0]].secondary_beaches[1] = i;
		}
	}
	for(int i=0; i<LEVEL_LAST; i++)
	{
		levels[i].locked = false;
		levels[i].beach = CareerDataArray[i].beach;

		int loc = beaches[levels[i].beach].location;
		if(loc == MAP_LOC_NOWHERE) continue;
		if(locations[loc].levels[0] == -1)
			locations[loc].levels[0] = i;
		else if(locations[loc].levels[1] == -1)
			locations[loc].levels[1] = i;
		else locations[loc].levels[2] = i;
	}

	Font* font = &manager->font_bold;
	for(int i=0; i<BEACH_LAST; i++)
	{
		entry_list[i] = NEW FEMenuEntry(BeachDataArray[i].fe_name, this, false, font);
		entry_list[i]->SetNoFlash(true);
		if(i == 0) first = entry_list[i];
		if(i == BEACH_LAST-1) last = entry_list[i];
		Add(entry_list[i]);
	}



/*	Save/load no longer part of map screen.  (dc 07/01/02)
	save_load = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_SAVE_LOAD], this, false, font);
	Add(save_load);
	save_load->SetSpecialScale(0.6f, 0.6f);
	save_load->SetHJustify(Font::HORIZJUST_LEFT);
*/

	fe_return = NEW FEMenuEntry("", this, false, font);
	Add(fe_return);
	fe_return->SetSpecialScale(0.7f, 0.7f);
	fe_return->SetHJustify(Font::HORIZJUST_CENTER);
	fe_return->SetNoFlash(true);
	fe_return->SetSpecialColor(color, color);

	return_question = NEW BoxText(font, ksGlobalTextArray[GT_FE_MENU_RETURN_FE_QUESTION], 320, 200, 0, 0.7f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_unselected);

	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
	{
		chall_text[i] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_CHALLENGE]+" "+stringx(i+1), this, false, &manager->font_info);
		Add(chall_text[i]);
		chall_text[i]->SetSpecialScale(sc_chall, sc_chall);
		chall_text[i]->SetHJustify(Font::HORIZJUST_LEFT);
		chall_text[i]->SetVJustify(Font::VERTJUST_TOP);
		chall_text[i]->SetLineSpacing(17);

		goals_completed[i] = NEW TextString(&manager->font_info, "", check_r_x, check_y, 0, sc_of, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->col_unselected);

		for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			color32 c = j==0 ? manager->blue : manager->white;
			float s = j==0 ? 1.0f : sc_goal;
			goal_text[i][j] = NEW BoxText(&manager->font_hand, "", goal_r_x, check_y, 0, s, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, c, 7);
		}
	}

	// only for fe_return prompt
	yes_entry = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_YES], this, false, font);
	no_entry = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_NO], this, false, font);
	yes_entry->SetSpecialScale(0.8f, 0.8f);
	no_entry->SetSpecialScale(0.8f, 0.8f);
	Add(yes_entry);
	Add(no_entry);
	yes_entry->Disable(true);
	no_entry->Disable(true);

	current = first;
	current_beach_index = 0;
	current_location_index = beaches[current_beach_index].location;
	current_level_index = locations[current_location_index].levels[0];
	on_challenge = 0;
	home_beach = -1;
	in_frontend = true;

	blink_counter = 0;
	blink_index = 0;
	blink_on = true;

	no_progress_path = false;
	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
		goal_num[i] = 0;
	offset_set = false;

/*	Save/load no longer part of map screen.  (dc 07/01/02)
	if(Realistic(true)) save_load->Disable(true);
*/

	description = NEW BoxText(&frontendmanager.font_body, "", box_d_l_x+30, box_d_t_y+15, 0, scale*1.2f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color32(0, 0, 0, 0), 20);
	beach_bio = NEW BoxText(&frontendmanager.font_body, "", 336, 130, 0, sc_sm*1.5f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->gray_dk, 60);
	bio_title = NEW TextString(&frontendmanager.font_bold_old, "", 336, 92, 0, scale*1.18f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->gray_dk);

	in_description_mode = false;
	in_bio_mode = false;
	first_time = true;
	map_wait_loading = -1;
}

BeachFrontEnd::~BeachFrontEnd()
{
	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
	{
		for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			delete goal_text[i][j];
			delete completed_lights[i][j][0];
			delete completed_lights[i][j][1];
		}
		delete goals_completed[i];
	}
	delete chall_div_line;
	delete description;
	delete beach_bio;
	delete bio_title;
	delete return_question;
}

void BeachFrontEnd::Load()
{
	FEMultiMenu::Load();
	path_pq = GetPointer("dot_path");
	map.Load("interface\\map\\fe_path.map", path_pq);
	path_pq->TurnOn(false);

	SetPQIndices();

	// adjust west_east_order & desc_at_right
	float lowest = 1000;
	float higher_than = -1000;
	int lowest_index = 0;
	for(int i=0; i<BEACH_LAST; i++)
	{
		for(int j=0; j<BEACH_LAST; j++)
		{
			float z = map.beaches[j].loc2d.x;
			if(z < lowest && ((z > higher_than) ||
				(i != 0 && z == higher_than && j > map.west_east_order[i-1])))
			{
				lowest = z;
				lowest_index = j;
			}
		}
		map.west_east_order[i] = lowest_index;
		higher_than = lowest;
		lowest = 1000;

		// beaches far to the left (g-land, bells, & currens) should not
		// have the description to the right
		map.beaches[i].desc_at_right = map.beaches[i].loc2d.x < 320;
	}

	FEMenuEntry* tmp;
	// looping through west_east_order here
	for(int i=0; i<BEACH_LAST; i++)
	{
		tmp = entry_list[map.west_east_order[i]];
		if(i == 0) tmp->left = entry_list[map.west_east_order[BEACH_LAST-1]];  //save_load;
		else tmp->left = entry_list[map.west_east_order[i-1]];
		if(i == BEACH_LAST-1) tmp->right = entry_list[map.west_east_order[0]];	//save_load;
		else tmp->right = entry_list[map.west_east_order[i+1]];
	}

/*	Save/load no longer part of map screen.  (dc 07/01/02)
	save_load->right = entry_list[map.west_east_order[0]];
	save_load->left = entry_list[map.west_east_order[BEACH_LAST-1]];
*/

	boxes_on = true;
	sliding_in = true;
	ignore_controller = false;
}

void BeachFrontEnd::ReloadPanel()
{
	FEMultiMenu::ReloadPanel();

	chall_div_line->setMaterialFlags(NGLMAT_CLAMP_U | NGLMAT_CLAMP_V | NGLMAT_TEXTURE_MAP | NGLMAT_BILINEAR_FILTER);
	chall_div_line->setTexture(line_across->GetTexture());
	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
		for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			completed_lights[i][j][0]->setMaterialFlags(NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
			completed_lights[i][j][0]->setTexture(j == 0 ? locked_first_light_template->GetTexture() : locked_light_template->GetTexture());
			completed_lights[i][j][1]->setMaterialFlags(NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
			completed_lights[i][j][1]->setTexture(unlocked_light_template->GetTexture());
		}
	
}

void BeachFrontEnd::SetPQIndices()
{
	line_across = GetPointer("line_left");
	line_down = GetPointer("line_right");
	chall_div_line = NEW PanelQuad(*line_across);

	float x1, x2, y1, y2;
	line_across->GetPos(x1, y1, x2, y2);
	line_l_x = (int) x1;
	line_down->GetPos(x1, y1, x2, y2);
	line_r_x = (int) x2;
	line_y = (int)((y2 - y1)/2.0f + y1);
	
	for(int i=0; i<9; i++)
	{
		box[i][0] = GetPointer(("box_left_0"+stringx(i+1)).data());
		box[i][1] = GetPointer(("box_right_0"+stringx(i+1)).data());
	}

	// sets the edges of the box
	// assumes 0 is top-right corner and 8 is bottom-left
	box[0][0]->GetPos(x1, y1, x2, y2);
	box_l_x[0] = x1;
	box_l_x[1] = x2;
	box_y[0] = y1;
	box_y[1] = y2;
	box[8][0]->GetPos(x1, y1, x2, y2);
	box_l_x[2] = x1;
	box_l_x[3] = x2;
	box_y[2] = y1;
	box_y[3] = y2;
	box[0][1]->GetPos(x1, y1, x2, y2);
	box_r_x[0] = x1;
	box_r_x[1] = x2;
	box[8][1]->GetPos(x1, y1, x2, y2);
	box_r_x[2] = x1;
	box_r_x[3] = x2;

	box_d_y[0] = box_y[0];
	box_d_y[1] = box_y[1];
	box_d_y[2] = box_d_b_y - (box_y[1] - box_y[0]);
	box_d_y[3] = box_d_b_y;
/*
	// setting key box points rather arbitrarily
	box_k_x[0] = box_l_x[0];
	box_k_x[1] = box_l_x[1];
	box_k_x[2] = box_k_x[1] + 120;
	box_k_x[3] = box_k_x[2] + (box_k_x[1] - box_k_x[0]);
	box_k_y[0] = 240;
	box_k_y[1] = box_k_y[0] + (box_y[1] - box_y[0]);
	box_k_y[2] = box_k_y[1] + 60;
	box_k_y[3] = box_k_y[2] + (box_k_y[1] - box_k_y[0]);
*/

	// setting key box points rather arbitrarily
	float normal_width = box_l_x[1] - box_l_x[0];
	float normal_height = box_y[1] - box_y[0];
	float width = 200;
	float height = 100;
	box_k_x[0] = 320 - width/2 - normal_width;
	box_k_x[1] = box_k_x[0] + normal_width;
	box_k_x[2] = box_k_x[1] + width;
	box_k_x[3] = box_k_x[2] + normal_width;
	box_k_y[0] = 240 - height/2 - normal_height;
	box_k_y[1] = box_k_y[0] + normal_height;
	box_k_y[2] = box_k_y[1] + height;
	box_k_y[3] = box_k_y[2] + normal_height;

//	fe_return->SetPos(320, 210);
//	fe_return->MakeBox(width, 100);
	return_question->makeBox((int) width, 100);
	yes_entry->SetPos(280, 280);
	no_entry->SetPos(360, 280);

	beach_l_l_x = (int)(box_l_x[0]+box_offset);
	beach_l_r_x = (int)(box_l_x[3]-box_offset);
	beach_r_l_x = (int)(box_r_x[0]+box_offset);
	beach_r_r_x = (int)(box_r_x[3]-box_offset);
	box_z = (int)(box[0][0]->z - 50);

	check_l_x = beach_l_l_x + check_offset;
	check_r_x = beach_r_l_x + check_offset;
	goal_l_x = beach_l_l_x + goal_offset;
	goal_r_x = beach_r_l_x + goal_offset;

	bio_book = GetPointer("book");
	bio_circle = GetPointer("book_circle");

	for(int i=0; i<3; i++)
		bio_scr[i] = GetPointer(("scroll_body_0"+stringx(i+1)).data());
	bio_scr_marker = GetPointer("scroll_marker");
	bio_scr_marker->GetCenterPos(bio_scroll_x, bio_scroll_y_t);
	bio_scroll_y_b = 357.0f;

	// bio arrows not needed
	GetPointer("button_b_off_01")->TurnOn(false);
	GetPointer("button_b_on_01")->TurnOn(false);
	GetPointer("button_b_off_02")->TurnOn(false);
	GetPointer("button_b_on_02")->TurnOn(false);

	for(int i=0; i<BEACH_LAST; i++)
	{
		here[i] = GetPointer((stringx(i)+"_dot_yellow").data());
		selected[i] = GetPointer((stringx(i)+"_dot_green").data());
		open[i] = GetPointer((stringx(i)+"_dot_orange").data());
		done[i] = GetPointer((stringx(i)+"_x").data());
		select_circle[i][0] = GetPointer((stringx(i)+"_circle_green_a").data());
		new_open_circle[i][0] = GetPointer((stringx(i)+"_circle_orange_a").data());
		select_circle[i][1] = GetPointer((stringx(i)+"_circle_green_b").data());
		new_open_circle[i][1] = GetPointer((stringx(i)+"_circle_orange_b").data());
		select_circle[i][2] = GetPointer((stringx(i)+"_circle_green_c").data());
		new_open_circle[i][2] = GetPointer((stringx(i)+"_circle_orange_c").data());
		if(here[i] == frontendmanager.GetDefaultPQ()) here[i] = NULL;
		if(selected[i] == frontendmanager.GetDefaultPQ()) selected[i] = NULL;
		if(open[i] == frontendmanager.GetDefaultPQ()) open[i] = NULL;
		if(done[i] == frontendmanager.GetDefaultPQ()) done[i] = NULL;
		for(int j=0; j<max_blink_index; j++)
		{
			if(select_circle[i][j] == frontendmanager.GetDefaultPQ()) select_circle[i][j] = NULL;
			if(new_open_circle[i][j] == frontendmanager.GetDefaultPQ()) new_open_circle[i][j] = NULL;
		}
		float x, y;
		if(beaches[i].main_beach == -1)
		{
			if(!here[i]) { x = 0; y = 0; }
			else here[i]->GetCenterPos(x, y);
			map.beaches[i].loc2d = vector2d(x, y);
		}
		else map.beaches[i].loc2d = map.beaches[beaches[i].main_beach].loc2d;
	}

	bkg = GetPointer("Earthmap_bkg");
	bkg->TurnOn(!in_frontend);
	cell_phone = GetPointer("cellphone");
	float x, y;
	cell_phone->GetCenterPos(x, y);
	cell_phone->SetCenterPos(x-30, y-50);
	cell_phone->SetZ(0);

	locked_light_template = GetPointer("subchallenge_locked_left");
	locked_first_light_template = GetPointer("subchallenge_square_left");
	unlocked_light_template = GetPointer("subchallenge_unlocked_left");
	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
		for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			completed_lights[i][j][0] = NEW PanelQuad(j == 0 ? *locked_first_light_template : *locked_light_template);
			completed_lights[i][j][1] = NEW PanelQuad(*unlocked_light_template);
			completed_lights[i][j][0]->TurnOn(false);
			completed_lights[i][j][1]->TurnOn(false);
			completed_lights[i][j][0]->SetZ(box_z);
			completed_lights[i][j][1]->SetZ(box_z-20);
		}

	locked_light_template->TurnOn(false);
	locked_first_light_template->TurnOn(false);
	unlocked_light_template->TurnOn(false);
	GetPointer("subchallenge_locked_right")->TurnOn(false);
	GetPointer("subchallenge_unlocked_right")->TurnOn(false);
	GetPointer("subchallenge_square_right")->TurnOn(false);
}

void BeachFrontEnd::UpdateInScene()
{
	if(in_frontend && manager->em->OKtoDrawBeachSelect() && !offset_set)
		SetOffset();
}

void BeachFrontEnd::Update(time_value_t time_inc)
{
	if(in_bio_mode)
	{
		bio_counter++;
		if(bio_up_pressed && bio_counter > bio_count_max)
		{
			bio_counter = 0;
			beach_bio->OnUp(0);
			UpdateBeachBioScrollbar();
		}
		else if(bio_down_pressed && bio_counter > bio_count_max)
		{
			bio_counter = 0;
			beach_bio->OnDown(0);
			UpdateBeachBioScrollbar();
		}
	}

	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		if (voiceOverStage == 1)
		{
			if (nslGetSoundStatus(voiceOverSound) == NSL_INVALID_ID)
			{
				VoiceOvers.playVO();
				voiceOverSound = VoiceOvers.getCurrentSound();
				voiceOverEvent = SS_LAST;
				voiceOverStage++;
			}
		}
		else if (voiceOverStage == 2)
		{
			if (nslGetSoundStatus(voiceOverSound) == NSL_INVALID_ID)
			{
				voiceOverEvent = SoundScriptManager::inst()->playEvent(SS_PHONE_HANGUP);
				voiceOverSound = SoundScriptManager::inst()->getSoundId(voiceOverEvent);
				voiceOverStage++;
			}
		}
		else if (voiceOverStage == 3)
		{
			if (nslGetSoundStatus(voiceOverSound) == NSL_INVALID_ID)
			{
				
				dampened = false;
				nslStopSound(voiceOverSound);
				voiceOverStage = 0;
			}
		}
		else if (!dampened && (nslGetSoundStatus(feMusic) != NSL_SOUNDSTATUS_INVALID) &&
			nslGetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME) < 1.0f)
		{
			float vol = (nslGetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME) + time_inc / 3.0f);
			if (vol >  1.0f)
				vol = 1.0f;
			nslSetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME, vol);
		}
	}





	//VoiceOvers.frameAdvance(time_inc);
	blink_counter += time_inc;
	if(blink_counter > 0.1f)
	{
		blink_counter = 0;
		blink_on = !blink_on;
		blink_index++;
		if(blink_index >= max_blink_index) blink_index = 0;

//		if (here[home_beach])	
//			here[home_beach]->TurnOn(blink_on);
		if(in_career_mode && !dmm.inDemoMode() && !dmm.wasInDemo)
		{
			for(int i=0; i<BEACH_LAST; i++)
				if(beaches[i].new_challenges)
					for(int j=0; j<max_blink_index; j++)
						if(new_open_circle[i][j]) new_open_circle[i][j]->TurnOn(j == blink_index);
		}
	}

	if (exitingWithoutSelect && !panel.IsSliding())
	{
		highlighted = fe_return;
		secondary_cursor = yes_entry;
		exitingWithoutSelect = false;
		Select(0);
	}
	if(in_career_mode)
	{
		for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
		{
			int current_chall = locations[current_location_index].levels[i];
			CareerData* level = &CareerDataArray[current_chall];
			for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
				if(level->goal[j] != GOAL_NOTHING)
					goal_text[i][j]->Update(time_inc);
		}
		line_across->Update(time_inc);
		line_down->Update(time_inc);
	}

	if (!in_frontend && frontendmanager.map_loading_screen == false && !sliding_in &&
		!panel.IsSliding() && !panel.IsSlideOutDone())
		panel.StartSlide(false);

	FEMultiMenu::Update(time_inc);

	// update flashing of goals_completed text
	if(
		in_career_mode 
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		&& highlighted != save_load 
*/
		&& highlighted != fe_return
	)
		goals_completed[on_challenge]->color = chall_text[on_challenge]->GetColor();
}
void BeachFrontEnd::SkipSlide(bool in)
{
	panel.ForceDoneSlide(in);
}

void BeachFrontEnd::Draw()
{
	if(in_frontend && (!manager->em->OKtoDrawBeachSelect() || !offset_set)) return;

	if (!in_frontend && frontendmanager.map_loading_screen == false && !sliding_in && panel.IsSlideOutDone())
	{
		((PauseMenuSystem *) system)->endDraw();
		return;
	}

	if(in_bio_mode)
		bio_title->Draw();

#ifdef TARGET_XBOX
	// On Xbox, you can see around the edges of the map on some TVs.  (dc 05/19/02)
	if(!panel.IsSliding() && bkg->IsOn()) 
	{
		nglListBeginScene();
		nglSetClearFlags(NGLCLEAR_COLOR);
		nglSetClearColor(0, 0, 0, 0);
		nglListEndScene();
	}
#endif

	panel.Draw(0);


	// don't draw stuff below this line if panel is sliding or immediately exiting
	if(!in_frontend && (panel.IsSliding() || exitingWithoutSelect)) return;


	ignore_controller = false;	// no slides are in progress (dc 04/29/02)

	manager->helpbar->Draw();

	// Presumably this was put here to turn the boxes on after a slide finishes?
	if (!boxes_on)
		UpdateBeach(false);

	if(
		!in_description_mode 
		&& !in_bio_mode 
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		&& highlighted != save_load 
*/
		&& highlighted != fe_return 
		&& in_career_mode
	)
	{
		for(int i=0; i<goal_num[on_challenge]; i++)
			goal_text[on_challenge][i]->Draw();

		// draw chall_text[0] if first challenge has unlocked goals, *or*
		// if no unlocked goals at all (in which case it reads "LOCKED")
		if(goal_num[0] != 0 || (goal_num[1] == 0 && goal_num[2] == 0))
			chall_text[0]->Draw();
		for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
			if(goal_num[j] != 0)
			{
				if(j != 0) chall_text[j]->Draw();
				goals_completed[j]->Draw();
			}

		chall_div_line->Draw(0);

		for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			completed_lights[on_challenge][j][0]->Draw(0);
			completed_lights[on_challenge][j][1]->Draw(0);
		}
		line_across->Draw(0);
		line_down->Draw(0);
	}

	if(in_description_mode)
		description->Draw();
	if(in_bio_mode)
		beach_bio->Draw();

	// draw only highlighted entry, and "key" menu
/*	Save/load no longer part of map screen.  (dc 07/01/02)
	if(!in_description_mode && !in_bio_mode && highlighted != save_load && !save_load->GetDisable() && key_box_up)
		save_load->Draw();
*/
	if(!in_description_mode && !in_bio_mode && highlighted != fe_return)
		highlighted->Draw();
//	if(!in_description_mode && !in_frontend && highlighted != fe_return && key_box_up)
//		fe_return->Draw();

	if(highlighted == fe_return)
	{
		return_question->Draw();

		if(map_wait_loading > 0)
			map_wait_loading--;
		else if(map_wait_loading == 0)
		{
			map_wait_loading = -1;
			return_question->changeText(ksGlobalTextArray[GT_FE_MENU_RETURN_FE_QUESTION]);
			return_question->makeBox(200, 100);
			return_question->changePos(320, 200);
			ReturnToFE();
		}
		else
		{
			yes_entry->Draw();
			no_entry->Draw();
		}
	}

	panel.Draw(1);
}


void BeachFrontEnd::DrawMap(float loading_progress)
{
	if(highlighted == fe_return)
	{
		panel.Draw(0);
		
		return_question->Draw();
		
		if(map_wait_loading > 0)
			map_wait_loading--;
		else if(map_wait_loading == 0)
		{
			map_wait_loading = -1;
			return_question->changeText(ksGlobalTextArray[GT_FE_MENU_RETURN_FE_QUESTION]);
			return_question->makeBox(200, 100);
			return_question->changePos(320, 200);
		}
		
		panel.Draw(1);
	}
	else
	{
		blink_index++;
		if(blink_index >= max_blink_index) blink_index = 0;
		
		int current_main_beach = current_beach_index;
		if(beaches[current_beach_index].main_beach != -1)
			current_main_beach = beaches[current_beach_index].main_beach;
		
		for(int j=0; j<max_blink_index; j++)
			if(select_circle[current_main_beach][j])
				select_circle[current_main_beach][j]->TurnOn(j == blink_index);
			
		if(home_beach != -1 && here[home_beach]) here[home_beach]->TurnOn(true);
		
		panel.Draw(0);
		
		// draw map path
		if(!no_progress_path)
			map.DrawPath(loading_progress);
		
		panel.Draw(1);
	}
}

void BeachFrontEnd::OnLevelLoaded()
{
	// Start drawing as overlay, rather than loading screen. (dc 04/28/02)
	sliding_in = false;	// must precede startDraw
	ignore_controller = true;
	((PauseMenuSystem *) system)->startDraw(PauseMenuSystem::MapMenu);
	frontendmanager.map_loading_screen = false;
}

void BeachFrontEnd::OnLevelEnding()
{
	sliding_in = true;
}

void BeachFrontEnd::Select(int entry_index)
{
	if (!in_frontend)
		SoundScriptManager::inst()->unpause();
	
/*	Save/load no longer part of map screen.  (dc 07/01/02)
	if(highlighted == save_load)
	{

		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		if(in_frontend) system->MakeActive(GraphicalMenuSystem::SaveLoadMenu);
		else 
		{
			system->MakeActive(PauseMenuSystem::SaveLoadMenu);
			((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->forward_menu = PauseMenuSystem::MapMenu;
			((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->forward_sub_menu = -1;
			((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->back_menu = PauseMenuSystem::MapMenu;
			((SaveLoadFrontEnd *)system->menus[PauseMenuSystem::SaveLoadMenu])->back_sub_menu = -1;
		}
	}
	else 
*/
	if(!in_frontend && highlighted == fe_return)
	{
		SoundScriptManager::inst()->playEvent(SS_FE_ONX);
		if(secondary_cursor == yes_entry)
		{
			map_wait_loading = MAX_MAP_LOADING;
			return_question->changeText(ksGlobalTextArray[GT_FE_MENU_LOADING]);
			return_question->makeBox(200, 100);
			return_question->changePos(320, 240);
			frontendmanager.helpbar->DisableHelpbar();
		}
		else FromFEReturn();
	}
	else Pick();
	if (!in_frontend)
		SoundScriptManager::inst()->pause();

}

void BeachFrontEnd::ReturnToFE()
{
	frontendmanager.return_to_fe = true;
	//app::inst()->get_game()->end_level();
	((PauseMenuSystem*) system)->PrepareToEndLevel();
}

void BeachFrontEnd::Pick()
{
	if ((g_game_ptr->get_game_mode() == GAME_MODE_CAREER && !in_frontend) ||
			(frontendmanager.tmp_game_mode == GAME_MODE_CAREER && in_frontend))
	{
		if (!in_description_mode)
		{
			if (current_level_index >= 0)
			{
				if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				{

					VoiceOvers.setCurrentLevel(current_level_index);
					VoiceOvers.setCurrentSurfer(g_game_ptr->GetSurferIdx(0));
					//VoiceOvers.playVO();
					voiceOverStage++;
					voiceOverEvent = SoundScriptManager::inst()->playEvent(SS_PHONE_RING);
					voiceOverSound = SoundScriptManager::inst()->getSoundId(voiceOverEvent);
					if (nslGetSoundStatus(voiceOverSound) != NSL_SOUNDSTATUS_INVALID && 
						nslGetSoundStatus(feMusic) != NSL_INVALID_ID)
					{
						//nslDampenGuardSound(voiceOverSound);
						nslSetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME, .5f);
						
						dampened = true;
					}
				}
			}
			SetDescription(true);
			return;
		}
		
		dampened = false;
		
		voiceOverStage = 0;
		if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			nslStopSound(voiceOverSound);
			if ((nslGetSoundStatus(feMusic) != NSL_SOUNDSTATUS_INVALID) &&
				nslGetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME) < 1.0f)
			{
				float vol = (nslGetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME));
				if (vol <  1.0f)
					vol = 1.0f;
				nslSetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME, vol);
			}
		}
	}


	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD) && current_beach_index == BEACH_INDOOR)
	{
		if(in_frontend) 
			manager->tmp_game_mode = GAME_MODE_PRACTICE;
		else 
			g_game_ptr->set_game_mode(GAME_MODE_PRACTICE);
	}

	if(in_career_mode)
	{
		
		// don't allow locked beaches to be selected in press build
		if(Realistic(false) && (current_level_index == -1 || levels[current_level_index].locked)) 
		{
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
			return;
		}
	}
	else
	{
		
		// don't allow locked beaches to be selected in press build
		if(Realistic(false) && beaches[current_beach_index].locked)
		{
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
			return;
		}
	}
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SetProgressPath();

	if(in_career_mode)
	{
		for(int i=0; i<BEACH_LAST; i++)
			for(int j=0; j<max_blink_index; j++)
				if(new_open_circle[i][j]) new_open_circle[i][j]->TurnOn(false);
		if (current_level_index == g_game_ptr->get_level_id() && !in_frontend)
		{
			g_game_ptr->retry_mode(true);
			if (g_game_ptr->is_competition_level())
			  g_beach_ptr->judges.OnCompetitionReset();
			
			// old method
			//((PauseMenuSystem*) system)->endDraw();
			
			// new method
			frontendmanager.in_game_map_up = false;
			return;
		}
		else
		{
			assert(current_level_index != -1);
			app::inst()->get_game()->set_level(current_level_index);
		}
	}
	else 
	{
		if (current_beach_index == g_game_ptr->get_beach_id() && !in_frontend)
		{
			g_game_ptr->retry_mode(true);
			
			// old method
			//((PauseMenuSystem*) system)->endDraw();

			// new method
			frontendmanager.in_game_map_up = false;
			return;
		}
		else
			app::inst()->get_game()->set_beach(current_beach_index);
	}

	if(in_frontend)
	{
		manager->em->ExitState();
		((GraphicalMenuSystem*) sys)->beach = current_beach_index;
		((GraphicalMenuSystem*) sys)->MakeActive(GraphicalMenuSystem::BoardMenu);
	}
	else
	{
		frontendmanager.return_to_fe = false;
		frontendmanager.map_loading_screen = true;
		PrepareForLoading();
		//app::inst()->get_game()->end_level();
		((PauseMenuSystem*) system)->PrepareToEndLevel();
	}
}
void BeachFrontEnd::HideAllDots()
{
	for(int i=0; i<BEACH_LAST; i++)
  {
		for(int j=0; j<max_blink_index; j++)
    {
      if(new_open_circle[i][j])   new_open_circle[i][j]->TurnOn(false);
      if(select_circle[i][j])     select_circle[i][j]->TurnOn(false);
    }
    if(here[i])       here[i]->TurnOn(false);;
    if(selected[i])   selected[i]->TurnOn(false);;
    if(open[i])       open[i]->TurnOn(false);;
    if(done[i])       done[i]->TurnOn(false);;
  }

}
void BeachFrontEnd::PickDemo(int beach)
{
	in_career_mode = true;
	in_frontend = true;
	in_demo_mode = true;
/*	Save/load no longer part of map screen.  (dc 07/01/02)
	save_load->down = NULL;
*/
	offset_set = false;

	home_beach = -1;
	current_level_index = beach; //locations[current_location_index].levels[0];
	
	SetBeachData();

	FEMultiMenu::OnActivate();
	setHigh(current);
	SetSecondaryCursor(goal_num[0] != 0 ? chall_text[0] : NULL);
	if(in_frontend) 
	{
		UpdateBeach(false);
	}
	else //if(in_career_mode)
	{
		panel.StartSlide(sliding_in);
		TurnBox(current_beach_index, true, 0);	// we will call UpdateBeach after the slide-in (dc 04/27/02)
//		mem_lock_malloc(false);
//		g_game_ptr->play_movie("MAPROLL", true);
	}

//	StartFrontendFade();
	
	int l;
	for(l=0; l<MAP_LOC_LAST; l++)
	{
		int b;
		for(b=0; b<MAX_BEACHES_PER_LOC; b++)
			if(locations[l].levels[b] == current_level_index)
			{
				current_beach_index = locations[l].beaches[0];
				break;
			}
			
			if(b < MAX_BEACHES_PER_LOC)
				break;
	}
	current_location_index = l;
	
  no_progress_path = true;

  //SetProgressPath();

  // Turn off all blinking lights except for destination
	HideAllDots();

  // Turn on green destination dot
  if(selected[locations[current_location_index].beaches[0]]) selected[locations[current_location_index].beaches[0]]->TurnOn(true);
	
  frontendmanager.return_to_fe = false;
	frontendmanager.map_loading_screen = true;
	manager->tmp_game_mode = GAME_MODE_CAREER;
	PrepareForLoading();
}

void BeachFrontEnd::OnTriangle(int c)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_description_mode)
	{
		if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			nslStopSound(voiceOverSound);
		voiceOverStage = 0;
		
		dampened = false;
		SetDescription(false);
		return;
	}
	if(in_bio_mode)
	{
		SetBeachBio(false);
		return;
	}
	if(in_frontend)
	{
		if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{

			if ((nslGetSoundStatus(feMusic) != NSL_SOUNDSTATUS_INVALID) &&
				nslGetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME) < 1.0f)
			{
				float vol = (nslGetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME));
				if (vol <  1.0f)
					vol = 1.0f;
				nslSetSoundParam(feMusic, NSL_SOUNDPARAM_VOLUME, vol);
			}
			nslStopSound(voiceOverSound);
		}
		GraphicalMenuSystem* stm = ((GraphicalMenuSystem*) sys);
		
		
		voiceOverStage = 0;
		dampened = false;
		
		manager->em->ExitState();
		
		if(manager->tmp_game_mode == GAME_MODE_CAREER && !Realistic(true))
			stm->MakeActive(GraphicalMenuSystem::SurferMenu, SurferFrontEnd::ACT_SURFER);
		else stm->MakeActive(GraphicalMenuSystem::SurferMenu);
	}
	else
	{
		if(fe_return == highlighted) FromFEReturn();
		else ToFEReturn();
	}
}

void BeachFrontEnd::ToFEReturn()
{
	former_high = highlighted;
	former_secondary = secondary_cursor;
	former_we_index = cur_we_index;
	former_beach_index = current_beach_index;
	former_level_index = current_level_index;

	yes_entry->Disable(false);
	no_entry->Disable(false);
	setHigh(fe_return);
	SetSecondaryCursor(no_entry);
	current = fe_return;
	cur_we_index = BEACH_LAST;
	UpdateBeach(false);
}

void BeachFrontEnd::FromFEReturn()
{
	cur_we_index = former_we_index;
	current_beach_index = former_beach_index;
	current_level_index = former_level_index;

	setHigh(former_high);
	current = former_high;
	SetSecondaryCursor(former_secondary);

	yes_entry->Disable(true);
	no_entry->Disable(true);
	UpdateBeach(false);
}

void BeachFrontEnd::PutUpLoadingScreen()
{
	SetKeyBox(true);
	setHigh(fe_return);
	map_wait_loading = MAX_MAP_LOADING;
	return_question->changeText(ksGlobalTextArray[GT_FE_MENU_LOADING]);
	return_question->makeBox(200, 100);
	return_question->changePos(320, 240);
	frontendmanager.helpbar->DisableHelpbar();
}

void BeachFrontEnd::OnCross(int c)
{
	if(in_frontend && (!manager->em->OKtoDrawBeachSelect() || !offset_set)) return;
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_bio_mode) return;
	FEMultiMenu::OnCross(c);
}

void BeachFrontEnd::OnCircle(int c)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_description_mode || in_bio_mode || !in_career_mode) return;
	if(!in_frontend) SetBeachBio(true);
	else if(manager->em->OKtoDrawBeachSelect()) SetBeachBio(true);
}

void BeachFrontEnd::OnAnyButtonPress(int c, int b)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(!in_frontend) return;

	if(manager->em->CamIsMoving() && b != FEMENUCMD_TRIANGLE)	// triangle case handled elsewhere
		manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_3_MAP);
}

void BeachFrontEnd::OnButtonRelease(int c, int b)
{
	if(in_bio_mode)
	{
		if(b == FEMENUCMD_UP)
			bio_up_pressed = false;
		else if(b == FEMENUCMD_DOWN)
			bio_down_pressed = false;
	}
}

void BeachFrontEnd::OnActivate()
{
	game *game_ptr = app::inst()->get_game();
	int player_1_device = 1 << game_ptr->get_player_device( 0 );
	int player_2_device = 1 << game_ptr->get_player_device( 1 );
	bool multiplayer;
	
	// Yucks
	if( in_frontend )
	{
		GraphicalMenuSystem *gm = (GraphicalMenuSystem *)sys;
		multiplayer = gm->multiplayer;
	}
	else
		multiplayer = game_ptr->get_num_players() > 1;
	
	if( multiplayer )
		sys->SetDeviceFlags( player_1_device | player_2_device );
	else
		sys->SetDeviceFlags( player_1_device );
	
	SetDescription(false, false);
	SetBeachBio(false);
	bio_up_pressed = false;
	bio_down_pressed = false;
	in_demo_mode = false;

	if(in_frontend)
	{
		in_career_mode = frontendmanager.tmp_game_mode == GAME_MODE_CAREER;
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		save_load->down = NULL;
*/
		offset_set = false;
	}
	else
	{
		in_career_mode = g_game_ptr->get_game_mode() == GAME_MODE_CAREER;
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		save_load->down = fe_return;
*/
	}

	for(int i=0; i<BEACH_LAST; i++)
		entry_list[i]->SetNoFlash(in_career_mode);

	if(in_career_mode)
	{
		home_beach = g_career->GetCurrentBeach();
	}
	else
	{
		home_beach = g_game_ptr->GetMostRecentBeach();
		// reset home beach if it's locked
		if(beaches[home_beach].locked && Realistic(false))
			home_beach = BEACH_SEBASTIAN;
	}
	assert(home_beach >= 0 && home_beach < BEACH_LAST);


	for(int i=0; i<BEACH_LAST; i++)
	{
		if(in_career_mode)
			entry_list[i]->SetText(BeachDataArray[i].career_name);
		else entry_list[i]->SetText(BeachDataArray[i].fe_name);
	}


	SetBeachData();

	if(in_frontend) manager->em->ToBeachSelect();

	if(first_time)
	{
		// also calls SetOnChallenge
		FindFirstAvailable(g_game_ptr->GetMostRecentLevel());
		first_time = false;
	}

	if(g_game_ptr->get_game_mode() != GAME_MODE_CAREER)
		for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
		{
			for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
			{
				completed_lights[j][i][0]->TurnOn(false);
				completed_lights[j][i][1]->TurnOn(false);
			}
		}

	FEMultiMenu::OnActivate();
	setHigh(current);
	if(in_frontend) 
	{
		UpdateBeach(false);
	}
	else //if(in_career_mode)
	{
		panel.StartSlide(sliding_in);
		TurnBox(current_beach_index, true, 0);	// we will call UpdateBeach after the slide-in (dc 04/27/02)
//		mem_lock_malloc(false);
//		g_game_ptr->play_movie("MAPROLL", true);
	}

	StartFrontendFade();
	ResetHelpbar();
	bio_counter = 0;
	key_box_up = false;
	map_wait_loading = -1;
}

void BeachFrontEnd::OnLeft(int c)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_description_mode || in_bio_mode) return;

	if (!in_frontend)
		SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if (!in_frontend)
		SoundScriptManager::inst()->pause();
	
	if(!in_frontend && fe_return == highlighted)
	{
		if(secondary_cursor == yes_entry)
			SetSecondaryCursor(no_entry);
		else SetSecondaryCursor(yes_entry);
		return;
	}

	SwitchBeach(false);
	UpdateBeach(true);
}

void BeachFrontEnd::OnRight(int c)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_description_mode || in_bio_mode) return;
	if (!in_frontend)
		SoundScriptManager::inst()->unpause();

	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if (!in_frontend)
		SoundScriptManager::inst()->pause();

	if(!in_frontend && fe_return == highlighted)
	{
		if(secondary_cursor == yes_entry)
			SetSecondaryCursor(no_entry);
		else SetSecondaryCursor(yes_entry);
		return;
	}


	SwitchBeach(true);
	UpdateBeach(true);
}

void BeachFrontEnd::SwitchBeach(bool right)
{
	dampened = false;
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		nslStopSound(voiceOverSound);
		voiceOverStage = 0;

	bool dis = false;
	bool same = false;
	bool nowhere = false;
	bool notabeach = false;

	do
	{
		cur_we_index += right ? 1 : -1;
		if(right && cur_we_index > BEACH_LAST) cur_we_index = 0;
		if(!right && cur_we_index < 0) cur_we_index = BEACH_LAST;
		if(cur_we_index != BEACH_LAST)
		{
			current_beach_index = map.west_east_order[cur_we_index];

			int loc = beaches[current_beach_index].location;
			dis = entry_list[current_beach_index]->GetDisable() && 
				(Realistic(false) || (in_career_mode && locations[loc].no_existing_levels) || current_beach_index == BEACH_OPENSEA);

			same = in_career_mode && beaches[current_beach_index].main_beach != -1;
			nowhere = beaches[current_beach_index].location == MAP_LOC_NOWHERE;
			notabeach = false;
		}
		else
		{
			notabeach = true;
			// return to boat not accessable through left-right now
			dis = true;//in_frontend;	// "return to boat" disabled if we're at the boat already (dc 07/01/02)
		}
	} while((!notabeach || dis) && (dis || same || nowhere));

	if (notabeach)
		current = fe_return;
	else 
		current = entry_list[current_beach_index];

	current_location_index = beaches[current_beach_index].location;
	current_level_index = locations[current_location_index].levels[0];
	setHigh(current);
	VoiceOvers.setCurrentLevel(current_level_index);
}

void BeachFrontEnd::OnUp(int c)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_description_mode) return;
	if(in_bio_mode)
	{
//		beach_bio->OnUp(c);
		bio_up_pressed = true;
		bio_counter = bio_count_max;
		return;
	}
	
	dampened = false;
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		nslStopSound(voiceOverSound);
		voiceOverStage = 0;
	if (!in_frontend)
		SoundScriptManager::inst()->unpause();

//	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	bool on_beach = 
		highlighted != fe_return 
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		&& highlighted != save_load
*/
	;
	if(on_beach)
	{
		if(goal_num[0] != 0 && (on_challenge == 1 || (on_challenge == 2 && goal_num[1] == 0)))
		{
			SetOnChallenge(0);
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);		
		}
		else if(on_challenge == 2 && goal_num[1] != 0)
		{
			SetOnChallenge(1);
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);		
		}
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);		
	}
	else
	{
		FEMultiMenu::OnUp(c);
	}
	if (!in_frontend)
		SoundScriptManager::inst()->pause();

}

void BeachFrontEnd::OnDown(int c)
{
	if((ignore_controller || panel.IsSliding()) && !in_frontend) return;
	if(in_description_mode) return;
	if(in_bio_mode)
	{
//		beach_bio->OnDown(c);
		bio_down_pressed = true;
		bio_counter = bio_count_max;
		return;
	}
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		nslStopSound(voiceOverSound);
		voiceOverStage = 0;
//	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	bool on_beach = 
		highlighted != fe_return 
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		&& highlighted != save_load
*/
	;
	if (!in_frontend)
		SoundScriptManager::inst()->unpause();

	if(on_beach)
	{
		if(goal_num[1] != 0 && on_challenge == 0)
		{
			SetOnChallenge(1);
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);		
		}
		else if(goal_num[2] != 0 && (on_challenge == 1 || (on_challenge == 0 && goal_num[1] == 0)))
		{
			SetOnChallenge(2);
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);		
		}
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);		
	}
	else 
	{
/*	Save/load no longer part of map screen.  (dc 07/01/02)
		if (highlighted == save_load)
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		else
*/
		FEMultiMenu::OnDown(c);
	}
	if (!in_frontend)
		SoundScriptManager::inst()->pause();

}

void BeachFrontEnd::UpdateBeach(bool reset_challenge)
{
	bool on_beach = highlighted != fe_return;

	SetBeachPoints();

	int offset = 0;
	if(on_beach)
	{
		bool right = map.beaches[current_beach_index].desc_at_right;

		if(in_career_mode)
		{
			if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				if (nslGetSoundStatus(VoiceOvers.getCurrentSound()) != NSL_SOUNDSTATUS_INVALID && dampened)
				{
					dampened = false;
				}

			BFE_Location* loc = &locations[beaches[current_beach_index].location];

			// reset Challenge 1 to be the default
			if(reset_challenge)
			{
				bool challenge_found = false;	// an unlocked, uncompleted challenge was found
				for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
				{
					if(loc->levels[i] != -1 && !levels[loc->levels[i]].locked && !levels[loc->levels[i]].done)
					{
//						if(on_challenge != i)
							SetOnChallenge(i);
						challenge_found = true;
						break;
					}
				}

				// if an uncompleted challenge wasn't found in first run, go again looking for completed challenge
				if(!challenge_found)
				{
					for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
					{
						if(loc->levels[i] != -1 && !levels[loc->levels[i]].locked)
						{
//							if(on_challenge != i)
								SetOnChallenge(i);
							break;
						}
					}
				}
			}
//			current->SetPos(right ? beach_r_l_x : beach_l_l_x, beach_y);
			current->SetPos(right ? beach_r_l_x+2 : beach_l_l_x+2, beach_y-3);
			current->SetHJustify(Font::HORIZJUST_LEFT);

			bool levels_unlocked[MAX_BEACHES_PER_LOC];
			for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
				levels_unlocked[j] = loc->levels[j] != -1 && !levels[loc->levels[j]].locked;

			if(entry_list[current_beach_index]->GetDisable())
			{
				color32 dis = color;
				dis.set_alpha(127);
				current->SetColor(dis);
				chall_text[0]->SetPos(right ? beach_r_l_x : beach_l_l_x, chall_y);
				chall_text[0]->SetText(ksGlobalTextArray[GT_FE_MENU_LOCKED]);
				color32 col = chall_text[0]->GetColor();
				col.set_alpha(127);
				chall_text[0]->SetColor(col);
				chall_text[0]->Disable(true);
				chall_text[0]->SetNoFlash(true);

				for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
				{
					goal_num[i] = 0;
					for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
					{
						completed_lights[i][j][0]->TurnOn(false);
						completed_lights[i][j][1]->TurnOn(false);
					}
				}
			}
			else
			{
				offset = chall_y;

				for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
				{
					if((levels_unlocked[j] || !Realistic(false)) && locations[current_location_index].levels[j] >= 0)
					{
						offset = UpdateChallenges(j, offset, right);
						if(reset_challenge && ((j==1 && !levels_unlocked[0]) || (j==2 && !levels_unlocked[0] && !levels_unlocked[1])))
							SetOnChallenge(j);
					}
					else
					{
						goal_num[j] = 0;
						for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
						{
							completed_lights[j][i][0]->TurnOn(false);
							completed_lights[j][i][1]->TurnOn(false);
						}
					}
				}
				offset = UpdateGoals(on_challenge, offset, right);
			}
		}
		else	// not career mode
		{
			if(Realistic(false) && beaches[current_beach_index].locked)
			{
				color32 tmp = manager->yel_dk;
				tmp.set_alpha(127);
				current->SetColor(tmp);
			}
			else
				current->SetColor(manager->yel_dk);

			vector2d loc = map.beaches[current_beach_index].loc2d;
			if(loc.x == 0 && loc.y == 0)
			{
				loc.x = 50;
				loc.y = 100;
			}
			else
				loc.x = loc.x + (right ? 20 : -20);
			loc = ApplyOffset(loc);
			float y = loc.y;
			if(y >= 390) y = 390;		// to avoid text that is too low
			current->SetPos(loc.x, y);
			current->SetHJustify(right ? Font::HORIZJUST_LEFT : Font::HORIZJUST_RIGHT);
		}
	}

	if(on_beach)
	{
		if(key_box_up) SetKeyBox(false);
		TurnBox(current_beach_index, !on_beach, offset);
	}
	else SetKeyBox(true);

	ResetHelpbar();
}

int BeachFrontEnd::UpdateChallenges(int cn, int offset, bool right)
{
	int new_offset = offset;
	int current_chall = locations[current_location_index].levels[cn];

	if(current_chall != -1 && (!levels[current_chall].locked || !Realistic(false)))
	{
		// the -6 is to adjust for the vertical-top justification
		chall_text[cn]->SetPos(right ? beach_r_l_x : beach_l_l_x, new_offset-6);
		stringx t = CareerDataArray[current_chall].challenge_name;
		t.to_lower();
		chall_text[cn]->SetText(t);
		color32 col = chall_text[cn]->GetColor();
		col.set_alpha(255);
		chall_text[cn]->SetColor(col);
		chall_text[cn]->Disable(false);
		chall_text[cn]->SetNoFlash(false);
		new_offset += 18*chall_text[cn]->getLineNum();

		goals_completed[cn]->changePos(right ? beach_r_l_x : beach_l_l_x, new_offset);
		new_offset += 18;

		CareerData* level = &CareerDataArray[current_chall];
		goal_num[cn] = 0;
		int completed_goals = 0;
		for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
		{
			if(level->goal[i] != GOAL_NOTHING)
			{
				bool completed = g_career->levels[current_chall].IsGoalDone(i);
				if(completed) completed_goals++;
				goal_num[cn]++;
			}
			else
			{
				completed_lights[cn][i][0]->TurnOn(false);
				completed_lights[cn][i][1]->TurnOn(false);
			}
		}

		// text is changed here to await completed goal count
		stringx t1 = stringx(completed_goals)+" "+ksGlobalTextArray[GT_FE_MENU_OF]+" "+stringx(goal_num[cn]);
		t1.to_lower();
		goals_completed[cn]->changeText(t1);
	}
	else
	{
		for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
			for(int j=0; j<2; j++)
				completed_lights[cn][i][j]->TurnOn(false);
		goal_num[cn] = 0;
	}
	return new_offset;
}

int BeachFrontEnd::UpdateGoals(int cn, int offset, bool right)
{
	int current_chall = locations[current_location_index].levels[cn];
	if (current_chall == -1) return offset;	// Is this the right action?  (dc 06/17/02)
	assert(current_chall >= 0 && current_chall < LEVEL_LAST);
	offset += 2;	// add spacing above and below the line
	chall_div_line->SetCenterPos(right? (beach_r_r_x - beach_r_l_x)/2+beach_r_l_x : (beach_l_r_x - beach_l_l_x)/2+beach_l_l_x, offset-8);
	offset += 2;
	CareerData* level = &CareerDataArray[current_chall];

	int count = 0;
	for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
	{
		stringx goal_string;

		if(level->goal[i] != GOAL_NOTHING && g_career->GetGoalText(current_chall, i, goal_string))
		{
			bool completed = g_career->levels[current_chall].IsGoalDone(i);
			int ind = count;
			goal_text[cn][ind]->changeText(goal_string);
			goal_text[cn][ind]->changePos(right ? goal_r_x : goal_l_x, offset);
			int line_count = goal_text[cn][ind]->makeBox(beach_l_r_x - goal_l_x, 40, false, -1, 17);
			
			int x = right ? check_r_x : check_l_x;
			int y = offset;
			x -= 7;
			y -= 7;
			completed_lights[cn][i][0]->SetPos(x, y);
			x -= 7;
			y -= 7;
			completed_lights[cn][i][1]->SetPos(x, y);
			
			completed_lights[cn][i][0]->TurnOn(true);
			completed_lights[cn][i][1]->TurnOn(completed);
			offset = offset + 18*line_count;
			count++;
		}
	}
	return offset;
}

void BeachFrontEnd::TurnBox(int b_num, bool all_off, float offset)
{
	bool right = map.beaches[b_num].desc_at_right;

	// all the box parts should be turned off if mode is not career
	all_off = all_off || !in_career_mode;

	boxes_on = !all_off;
	line_across->TurnOn(!all_off);
	line_down->TurnOn(!all_off);

	if(!all_off)
	{
		float x1, x2, y1, y2;
		int side = right ? 1 : 0;
		offset -= 16;
		for(int j=0; j<9; j++)
		{
			for(int i=0; i<2; i++)
				box[j][i]->TurnOn((right && i == 1) || (!right && i == 0));

			// resize box
			if(j >= 3 && j <= 5)
			{
				box[j][side]->GetPos(x1, y1, x2, y2);
				box[j][side]->SetPos(x1, y1, x2, offset);
			}
			else if(j >=6 && j <= 8)
			{
				box[j][side]->GetPos(x1, y1, x2, y2);
				box[j][side]->SetPos(x1, offset, x2, offset + (y2-y1));
			}
		}

		vector2d p = map.beaches[b_num].loc2d;
		p = ApplyOffset(p);
		if(right) line_across->SetPos(p.x, line_y, line_r_x, line_y+2);
		else line_across->SetPos(line_l_x, line_y, p.x, line_y+2);
		line_down->SetPos(p.x-1, line_y, p.x+1, p.y);
	}
	else
	{
		for(int i=0; i<9; i++)
		{
			box[i][0]->TurnOn(false);
			box[i][1]->TurnOn(false);
		}
	}
}

// called when going into the front end (in_fe == true) or in game (in_fe == false)
void BeachFrontEnd::SwitchState(FEMenuSystem* s, bool in_fe)
{
	sys = s;
	system = sys;
	in_frontend = in_fe;
	bkg->TurnOn(!in_frontend);

/*	Save/load no longer part of map screen.  (dc 07/01/02)
	if(!in_frontend)
	{
		save_load->down = fe_return;
		fe_return->up = save_load;
	}
	else save_load->down = NULL;
*/
}

// called when the 2d loading screen appears
void BeachFrontEnd::To2dFEMap()
{
	in_frontend = false;
	bkg->TurnOn(!in_frontend);
	OnActivate();
}

void BeachFrontEnd::PrepareForLoading()
{
	bkg->TurnOn(true);
	in_frontend = false;
	cell_phone->TurnOn(false);
	bio_book->TurnOn(false);
	bio_circle->TurnOn(false);
	for(int i=0; i<3; i++)
		bio_scr[i]->TurnOn(false);
	bio_scr_marker->TurnOn(false);

	TurnBox(0, true, 0);
	ReadjustBeachPoints();

	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
		for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			completed_lights[i][j][0]->TurnOn(false);
			completed_lights[i][j][1]->TurnOn(false);
		}
}

void BeachFrontEnd::SetForPractice()
{
	current_beach_index = BEACH_INDOOR;
	cell_phone->TurnOn(false);

	SetBeachData();
	UpdateBeach(false);
}

// sets most variables in BFE_Level, BFE_Location, and BFE_Beach
void BeachFrontEnd::SetBeachData()
{
	for(int i=0; i<LEVEL_LAST; i++)
	{
		//levels[i].locked = !g_career->levels[i].IsUnlocked();
		levels[i].locked = !unlockManager.isLevelUnlocked(i);
	}
	for(int i=0; i<BEACH_LAST; i++)
	{
		/*if (manager->tmp_game_mode == GAME_MODE_CAREER)
			beaches[i].locked = !g_career->beaches[i].IsUnlocked();
		else
			beaches[i].locked = !globalCareerData.isBeachUnlocked(i);*/
		beaches[i].locked = !unlockManager.isBeachUnlocked(i);

		if(!in_career_mode)
			entry_list[i]->Disable((Realistic(false) && beaches[i].locked) || beaches[i].location == MAP_LOC_NOWHERE);
		else
		{
			BFE_Location* loc = &locations[beaches[i].location];
			bool levels_locked[MAX_BEACHES_PER_LOC];
			bool levels_unlocked[MAX_BEACHES_PER_LOC];
			loc->no_existing_levels = true;

			// levels_locked means it doesn't exist or is locked
			// levels_unlocked means it DOES exist and isn't locked (so they aren't opposites)
			for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
			{
				levels_locked[j] = loc->levels[j] == -1 || (loc->levels[j] != -1 && levels[loc->levels[j]].locked);
				levels_unlocked[j] = loc->levels[j] != -1 && !levels[loc->levels[j]].locked;
				if(loc->levels[j] != -1) loc->no_existing_levels = false;
			}

			loc->all_levels_locked = levels_locked[0] && levels_locked[1] && levels_locked[2];

			// always disable if there are no levels
			entry_list[i]->Disable((Realistic(false) && loc->all_levels_locked) || loc->no_existing_levels);

			if(loc->levels[0] != -1)
			{
				Career::Level level1 = g_career->levels[loc->levels[0]];
				
				bool done1 = true;
				// check if any goals haven't been completed
				for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
					if(CareerDataArray[loc->levels[0]].goal[j] != GOAL_NOTHING)
						if(!level1.IsGoalDone(j)) done1 = false;

				levels[loc->levels[0]].done = done1;
						
				if(loc->levels[1] != -1)
				{
					Career::Level level2 = g_career->levels[loc->levels[1]];
					
					bool done2 = true;
					// check if any goals haven't been completed
					for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
						if(CareerDataArray[loc->levels[1]].goal[j] != GOAL_NOTHING)
							if(!level2.IsGoalDone(j)) done2 = false;

					levels[loc->levels[1]].done = done2;
							
					if(loc->levels[2] != -1)
					{
						Career::Level level3 = g_career->levels[loc->levels[2]];
						
						bool done3 = true;
						// check if any goals haven't been completed
						for(int j=0; j<MAX_GOALS_PER_LEVEL; j++)
							if(CareerDataArray[loc->levels[2]].goal[j] != GOAL_NOTHING)
								if(!level3.IsGoalDone(j)) done3 = false;

						levels[loc->levels[2]].done = done3;
								
						beaches[i].new_challenges = level1.IsNew() || level2.IsNew() || level3.IsNew();
						beaches[i].done = done1 && (!levels_unlocked[1] || done2) && (!levels_unlocked[2] || done3);
					}
					else	// 2 levels
					{
						beaches[i].new_challenges = level1.IsNew() || level2.IsNew();
						beaches[i].done = done1 && (!levels_unlocked[1] || done2);
					}
				}
				else	// 1 level
				{
					beaches[i].new_challenges = level1.IsNew();
					beaches[i].done = done1;
				}
			}
			else	// no levels
			{
				beaches[i].new_challenges = false;
				beaches[i].done = true;
			}
		}
	}

	for(int i=0; i<MAP_LOC_LAST; i++)
	{
		locations[i].all_beaches_locked = true;
		for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
			if(locations[i].beaches[j] != -1 && !beaches[locations[i].beaches[j]].locked)
				locations[i].all_beaches_locked = false;
	}
}

void BeachFrontEnd::FindFirstAvailable(int level)
{
	// start at current beach, if coming back from gameplay (dc 04/27/02)
	assert(home_beach >= 0 && home_beach < BEACH_LAST);
	current_beach_index = home_beach;
	for(int i=0; i<BEACH_LAST; i++)
	{
		current_beach_index = (home_beach+i)%BEACH_LAST;
		if(in_career_mode)
		{
			int loc = beaches[current_beach_index].location;
			if(!locations[loc].all_levels_locked || !Realistic(false)) break;
		}
		else if(!beaches[current_beach_index].locked) break;
	}

	// loop through to start on first available
//	Save/load no longer part of map screen.  (dc 07/01/02)
//	save_load->Disable(!in_career_mode);
	if(in_career_mode)
	{
		current_location_index = beaches[current_beach_index].location;

		// start at most recent level, as long as it is the same beach and it's valid
		if(current_beach_index == home_beach && level != -1 && 
			(!levels[level].locked || !Realistic(false)) && current_beach_index == levels[level].beach)
			current_level_index = level;
		else
		{
			for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
			{
				current_level_index = locations[current_location_index].levels[i];
				if(current_level_index != -1 && (!levels[current_level_index].locked || !Realistic(false)))
					break;
			}
		}

		for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
			if(current_level_index == locations[current_location_index].levels[i])
			{
				SetOnChallenge(i);
				break;
			}

		// if the current beach is a secondary beach, reset to main beach
		if(beaches[current_beach_index].main_beach != -1)
			current_beach_index = beaches[current_beach_index].main_beach;

		current = first;
		for(int i=0; i<current_beach_index; i++)
			current = current->next;
	}
	else
	{
		current = first;
		for(int i=0; i<current_beach_index; i++)
			current = current->next;

		while(Realistic(false) && beaches[current_beach_index].locked)
		{
			current = current->next;
			current_beach_index++;
			if(current_beach_index >= BEACH_LAST)
			{
				nglPrintf("WARNING: all beaches are locked\n");
				current = first;
				current_beach_index = 0;
				break;
			}
		}
		current_location_index = beaches[current_beach_index].location;
		current_level_index = locations[current_location_index].levels[0];
	}

	for(int i=0; i<BEACH_LAST; i++)
		if(map.west_east_order[i] == current_beach_index)
		{
			cur_we_index = i;
			break;
		}
}

void BeachFrontEnd::StartFrontendFade()
{
	// start fade in FE
	if(in_frontend)
	{
		if(in_career_mode)
		{
			for(int j=0; j<MAX_BEACHES_PER_LOC; j++)
			{
				chall_text[j]->SetFade(true, true, 1);
				int current_chall = locations[current_location_index].levels[j];
				CareerData* level = &CareerDataArray[current_chall];
				for(int i=0; i<MAX_GOALS_PER_LEVEL; i++)
					if(level->goal[i] != GOAL_NOTHING)
						goal_text[j][i]->ChangeFade(true, true, 1);
			}

			int tmp = 0;
			if(map.beaches[current_beach_index].desc_at_right) tmp = 1;

			for(int i=0; i<9; i++)
				box[i][tmp]->ChangeFade(true, true, 1);
			line_across->ChangeFade(true, true, 1);
			line_down->ChangeFade(true, true, 1);
		}
		highlighted->SetFade(true, true, 1);
//	Save/load no longer part of map screen.  (dc 07/01/02)
//		save_load->SetFade(true, true, 1);
	}
}
	
void BeachFrontEnd::SetProgressPath()
{
	no_progress_path = current_location_index == beaches[home_beach].location;

	// also set for no path if too close together
	if(!no_progress_path)
	{
		if((current_location_index == MAP_LOC_CORTESBANK || 
			current_location_index == MAP_LOC_MAVERICKS ||
			current_location_index == MAP_LOC_TRESTLES) &&
			(beaches[home_beach].location == MAP_LOC_CORTESBANK || 
			beaches[home_beach].location == MAP_LOC_MAVERICKS ||
			beaches[home_beach].location == MAP_LOC_TRESTLES))
			no_progress_path = true;
		else if((current_location_index == MAP_LOC_PIPELINE && 
			beaches[home_beach].location == MAP_LOC_JAWS) || 
			(beaches[home_beach].location == MAP_LOC_PIPELINE && 
			current_location_index == MAP_LOC_JAWS))
			no_progress_path = true;
	}

	if(!no_progress_path)
	{
		BFE_Location loc1 = locations[current_location_index];
		BFE_Location loc2 = locations[beaches[home_beach].location];
		bool ret = map.setPath(loc1.beaches[0], loc2.beaches[0]);
		if(!ret) ret = map.setPath(loc1.beaches[1], loc2.beaches[0]);
		if(!ret) ret = map.setPath(loc1.beaches[0], loc2.beaches[1]);
		if(!ret) ret = map.setPath(loc1.beaches[1], loc2.beaches[1]);
	}
}

void BeachFrontEnd::SetOnChallenge(int chall)
{
	// update flashing of goals_completed text
	if(
		in_career_mode 
//	Save/load no longer part of map screen.  (dc 07/01/02)
//		&& highlighted != save_load 
		&& highlighted != fe_return
	)
		goals_completed[on_challenge]->color = manager->col_unselected;

	on_challenge = chall;
	SetSecondaryCursor(chall_text[chall]);
	current_level_index = locations[current_location_index].levels[on_challenge];
	VoiceOvers.setCurrentLevel(current_level_index);

	float offset = chall_y;
	for(int i=0; i<MAX_BEACHES_PER_LOC; i++)
		offset = UpdateChallenges(i, offset, map.beaches[current_beach_index].desc_at_right);
	offset = UpdateGoals(on_challenge, offset, map.beaches[current_beach_index].desc_at_right);

	TurnBox(current_beach_index, false, offset);

	// update flashing of goals_completed text
	if(
		in_career_mode 
//	Save/load no longer part of map screen.  (dc 07/01/02)
//		&& highlighted != save_load 
		&& highlighted != fe_return
	)
		goals_completed[on_challenge]->color = chall_text[on_challenge]->GetColor();
}

void BeachFrontEnd::SetBeachPoints()
{
	bool is_current = false;
	bool is_home = false;
	bool unlocked = true;
	bool challenges_complete = false;
	bool new_challenges = false;
	bool on_beach = highlighted != fe_return;

	for(int i=0; i<BEACH_LAST; i++)
	{
		int loc = beaches[i].location;
		is_current = beaches[i].location == beaches[current_beach_index].location && on_beach;
		is_home = beaches[i].location == beaches[home_beach].location;

		if(in_career_mode)
		{
			new_challenges = beaches[i].new_challenges;
			challenges_complete = beaches[i].done;
		}

		if(Realistic(false))
		{
			if(in_career_mode) unlocked = !locations[loc].all_levels_locked && !locations[loc].no_existing_levels;
			else unlocked = !locations[loc].all_beaches_locked;
		}
		else
		{
			if(in_career_mode) unlocked = !locations[loc].no_existing_levels;
			else unlocked = true;
		}

		if(selected[i])			selected[i]->TurnOn(is_current);
		if(done[i])				done[i]->TurnOn(!is_current && unlocked && challenges_complete);
		if(here[i])				here[i]->TurnOn(!is_current && unlocked && !challenges_complete && is_home);
		if(open[i])				open[i]->TurnOn(!is_current && unlocked && !challenges_complete && !is_home);

		for(int j=0; j<max_blink_index; j++)
		{
			if(new_open_circle[i][j]) new_open_circle[i][j]->TurnOn((j == blink_index) && new_challenges && unlocked);
			if(select_circle[i][j]) select_circle[i][j]->TurnOn((j == max_blink_index-1) && is_current && !new_challenges);
		}
	}
}

bool BeachFrontEnd::Realistic(bool press_build_only)
{
	bool ret;
	if(press_build_only)
		ret = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD);
	else
		ret = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD) ||
			os_developer_options::inst()->is_flagged(os_developer_options::FLAG_REALISTIC_FE);
	return ret;
}

void BeachFrontEnd::SetOffset()
{
	if(!in_frontend) return;

	// corresponds to 0, 0; max coords were (207.93, 167.62, 208.0)
	nglVector start_in(-2.079f, 2.080f, -1.676f);

	// corresponds to 640, 480; max coords were (207.93, 60.40, 127.6)
	nglVector end_in(-2.079f, 1.276f, -0.604f);
	nglVector start_out, end_out;

	nglProjectPoint(start_out, start_in);
	nglProjectPoint(end_out, end_in);

	// convert to 640x480 coords
	unadjustCoords(start_out[0], start_out[1]);
	unadjustCoords(end_out[0], end_out[1]);

	map_points_offset.x = start_out[0];
	map_points_offset.y = start_out[1];

	map_points_multiplier.x = (end_out[0] - start_out[0])/640.0f;
	map_points_multiplier.y = (end_out[1] - start_out[1])/480.0f;

	offset_set = true;
	ReadjustBeachPoints();
	UpdateBeach(false);
}

// applies offset to all beach points
void BeachFrontEnd::ReadjustBeachPoints()
{
	// don't adjust points if sliding
	if(panel.IsSliding()) return;
	vector2d new_pos;
	for(int i=0; i<BEACH_LAST; i++)
	{
		new_pos = ApplyOffset(map.beaches[i].loc2d);
		if(here[i]) here[i]->SetCenterPos(new_pos.x, new_pos.y);
		if(selected[i]) selected[i]->SetCenterPos(new_pos.x, new_pos.y);
		if(open[i]) open[i]->SetCenterPos(new_pos.x, new_pos.y);
		if(done[i]) done[i]->SetCenterPos(new_pos.x, new_pos.y);

		for(int j=0; j<max_blink_index; j++)
		{
			if(select_circle[i][j]) select_circle[i][j]->SetCenterPos(new_pos.x, new_pos.y);
			if(new_open_circle[i][j]) new_open_circle[i][j]->SetCenterPos(new_pos.x, new_pos.y);
		}
	}
}

vector2d BeachFrontEnd::ApplyOffset(vector2d v)
{
	if(in_frontend && offset_set)
		return vector2d(v.x * map_points_multiplier.x, v.y * map_points_multiplier.y) + map_points_offset;
	else return v;
}

void BeachFrontEnd::SetDescription(bool on, bool update_beach)
{
	in_description_mode = on;

	for(int i=0; i<9; i++)
	{
		box[i][0]->TurnOn(on);
		box[i][1]->TurnOn(false);
	}

	line_across->TurnOn(!on);
	line_down->TurnOn(!on);
	cell_phone->TurnOn(on);

	if(on)
	{
		stringx tmp = LevelNameArray[current_level_index];

		if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
			tmp += "_fr";
		else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
			tmp += "_ge";

		char tmp2[100];
		strcpy(tmp2, (stringx("interface\\map\\goals\\" + tmp + ".txt")).c_str());

		if(!file_finder_exists(tmp2, ""))
			description->changeText("");
		else description->ReadFromFile(tmp2, false);
		description->makeBox(box_d_r_x - box_d_l_x - 45, box_d_b_y - box_d_t_y - 60);

		for(int i=0; i<9; i++)
		{
			float x1 = (i%3 < 2) ? box_l_x[i%3] : box_r_x[i%3];
			float x2 = (i%3+1 < 2) ? box_l_x[i%3+1] : box_r_x[i%3+1];
			box[i][0]->SetPos(x1, box_d_y[i/3], x2, box_d_y[i/3+1]);
		}
	}
	else
	{
		// reset box position back to normal
		for(int i=0; i<9; i++)
			box[i][0]->SetPos(box_l_x[i%3], box_y[i/3], box_l_x[i%3+1], box_y[i/3+1]);

		if(update_beach) UpdateBeach(false);
	}

	if(on)
	{
		manager->helpbar->Reset();
		manager->helpbar->RemoveArrowBoth();
		manager->helpbar->Reformat();
	}
	else ResetHelpbar();
}

void BeachFrontEnd::SetKeyBox(bool on)
{
	key_box_up = on;
	for(int i=0; i<9; i++)
	{
		box[i][0]->TurnOn(on);
		box[i][1]->TurnOn(false);
	}

	line_across->TurnOn(!on);
	line_down->TurnOn(!on);

	if(on)
	{
		for(int i=0; i<9; i++)
			box[i][0]->SetPos(box_k_x[i%3], box_k_y[i/3], box_k_x[i%3+1], box_k_y[i/3+1]);
	}
	else
	{
		// reset box position back to normal
		for(int i=0; i<9; i++)
			box[i][0]->SetPos(box_l_x[i%3], box_y[i/3], box_l_x[i%3+1], box_y[i/3+1]);
	}

	if(on)
	{
		if(map_wait_loading < 0)
		{
			manager->helpbar->Reset();
			if(!in_frontend) manager->helpbar->RemoveTriangle();
			manager->helpbar->Reformat();
		}
	}
	else ResetHelpbar();
}

void BeachFrontEnd::SetBeachBio(bool on)
{
	if(on)
	{
		stringx tmp;
		// first check that the file exists (some beaches don't have bios)
		//   and return immediately if it doesn't
		if(!BeachBioExists(tmp)) return;

		char tmp2[100];
		strcpy(tmp2, (stringx("interface\\map\\beaches\\" + tmp)).c_str());
		beach_bio->ReadFromFile(tmp2, true);
		bio_total_lines = beach_bio->makeBox(194, 1000, false, -1, 18);	// scrollable, so it may overrun
		bio_total_visible = 12;
		beach_bio->SetScrollable(bio_total_visible);
		bio_title->changeText(BeachDataArray[highlighted->entry_num].fe_name);
		UpdateBeachBioScrollbar();
	}

	for(int i=0; i<9; i++)
	{
		box[i][0]->TurnOn(!on && !map.beaches[current_beach_index].desc_at_right);
		box[i][1]->TurnOn(!on && map.beaches[current_beach_index].desc_at_right);
	}
	line_across->TurnOn(!on);
	line_down->TurnOn(!on);

	in_bio_mode = on;
	bio_book->TurnOn(on);
	bio_circle->TurnOn(false);
	bio_scr[0]->TurnOn(on);
	bio_scr[1]->TurnOn(on);
	bio_scr[2]->TurnOn(on);
	bio_scr_marker->TurnOn(on);

//	if(in_frontend)
		if(on)
		{
			manager->helpbar->Reset();
			manager->helpbar->AddArrowV(ksGlobalTextArray[GT_FE_MENU_SCROLL]);
			manager->helpbar->RemoveX();
			manager->helpbar->Reformat();
		}
		else ResetHelpbar();
}

void BeachFrontEnd::UpdateBeachBioScrollbar()
{
	float per = beach_bio->GetFirstVisable()/((float) (bio_total_lines - bio_total_visible));
	float y = (bio_scroll_y_b - bio_scroll_y_t)*per + bio_scroll_y_t;
	bio_scr_marker->SetCenterPos(bio_scroll_x, y);
}

bool BeachFrontEnd::BeachBioExists(stringx& tmp)
{
	if(highlighted == fe_return) return false;
	tmp = stringx("interface\\map\\beaches\\") + BeachDataArray[highlighted->entry_num].stashfile;
	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
		tmp += "_fr";
	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
		tmp += "_ge";
	tmp += ".txt";
	return file_finder_exists(tmp, "");
}

void BeachFrontEnd::ResetHelpbar()
{
	if(!in_demo_mode && map_wait_loading < 0)
	{
		bool on_beach = highlighted != fe_return;
		stringx tmp;
		bool has_bio = in_career_mode && BeachBioExists(tmp);
		manager->helpbar->Reset();
		if(on_beach && has_bio)
			manager->helpbar->AddCircle(ksGlobalTextArray[GT_FE_MENU_HB_BIO]);

		if(!in_frontend && on_beach)
			manager->helpbar->SetTriangleText(ksGlobalTextArray[GT_FE_MENU_BOAT]);

		// determine proper arrows
		if(!in_frontend && highlighted == fe_return)
			manager->helpbar->AddArrowH();
		else
		{
			if(in_career_mode && on_beach)
			{
				bool others = false;
				switch(on_challenge)
				{
				case 0: others = (goal_num[1] != 0 || goal_num[2] != 0); break;
				case 1: others = (goal_num[0] != 0 || goal_num[2] != 0); break;
				case 2: others = (goal_num[0] != 0 || goal_num[1] != 0); break;
				default: assert(0); break;
				}
				if(others) manager->helpbar->AddArrowBoth();
				else manager->helpbar->AddArrowH();
			}
			else
			{
				if(!on_beach && !in_frontend)
					manager->helpbar->AddArrowBoth();
				else manager->helpbar->AddArrowH();
			}
		}

		manager->helpbar->Reformat();
	}
}

