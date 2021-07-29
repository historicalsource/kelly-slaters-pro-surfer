// BoardFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if _XBOX
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "FrontEndManager.h"
#include "BoardFrontEnd.h"
#include "unlock_manager.h"

extern game* g_game_ptr;

BoardFrontEnd::BoardFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);

	color32 lt_blu = manager->col_info_b;
	color32 lt_grn = manager->col_info_g;
	color32 dk_yel = manager->col_highlight2;

	float sc_title = 0.89f;
	float sc_hand = 1.0f; //0.5f;
	float sc_info = 0.9f;	//0.67f;

	header = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_BOARD_SELECT], 460, 66, 0, sc_title, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, dk_yel);
	players[0] = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_PLAYER_1], 460, 66, 0, sc_title, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, dk_yel);
	players[1] = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_PLAYER_2], 460, 66, 0, sc_title, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, dk_yel);
	height_label = NEW TextString(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_HEIGHT], 365, 195, 0, sc_hand, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_grn);
	width_label = NEW TextString(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_WIDTH], 365, 211, 0, sc_hand, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_grn);
	thick_label = NEW TextString(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_THICKNESS], 365, 227, 0, sc_hand, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_grn);
	tail_label = NEW TextString(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_TAIL_TYPE], 365, 243, 0, sc_hand, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_grn);
	height = NEW TextString(&manager->font_info, "", 555, 195, 0, sc_info, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, lt_blu);
	width = NEW TextString(&manager->font_info, "", 555, 211, 0, sc_info, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, lt_blu);
	thick = NEW TextString(&manager->font_info, "", 555, 227, 0, sc_info, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, lt_blu);
	tail = NEW TextString(&manager->font_info, "", 555, 243, 0, sc_info, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, lt_blu);

	board_stats = NEW TextString(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_BOARD_STATS], 365, 96, 0, sc_hand, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_grn);
	comb_stats = NEW TextString(&manager->font_hand, ksGlobalTextArray[GT_FE_MENU_COMB_STATS], 365, 277, 0, sc_hand, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_grn);

	for(int i=0; i<4; i++)
	{
		stringx tmp = ksGlobalTextArray[GT_FE_MENU_SPIN_ATTR+i];
		tmp.to_lower();
		gauge_labels[i] = NEW TextString(&manager->font_info, tmp, 365, 297+16*i, 0, sc_info, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_blu);
		top_gauge_labels[i] = NEW TextString(&manager->font_info, tmp, 365, 115+16*i, 0, sc_info, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, lt_blu);
	}
	
	sys = (GraphicalMenuSystem*) s;
	arrow_counter = -5;
}

BoardFrontEnd::~BoardFrontEnd()
{
	delete header;
	delete height;
	delete width;
	delete thick;
	delete tail;
	delete height_label;
	delete width_label;
	delete thick_label;
	delete tail_label;
	delete players[0];
	delete players[1];
	delete board_stats;
	delete comb_stats;

	for(int i=0; i<4; i++)
	{
		delete gauge_labels[i];
		delete top_gauge_labels[i];
		delete red_gauges[i];
	}
}

void BoardFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();
}

void BoardFrontEnd::Update(time_value_t time_inc)
{
	if(wait_for_camera && manager->em->OKtoDrawBoardSelect()) wait_for_camera = false;
	else if(!wait_for_camera && !manager->em->OKtoDrawBoardSelect()) wait_for_camera = true;

	for(int i=0; i<4; i++)
		red_gauges[i]->Update(time_inc);
	FEMultiMenu::Update(time_inc);
}

void BoardFrontEnd::Draw()
{
	if(wait_for_camera) return;
	if(arrow_counter >= 0) arrow_counter--;
	if(arrow_counter < 0 && arrow_counter > -5)
	{
		arrow_counter = -5;
		arrows[0][0]->TurnOn(true);
		arrows[0][1]->TurnOn(false);
		arrows[1][0]->TurnOn(true);
		arrows[1][1]->TurnOn(false);
	}

	FEMultiMenu::Draw();

	// says "LOCKED" if not unlocked
	board_stats->Draw();

	if(unlocked)
	{
		for(int i=0; i<4; i++)
		{
			gauge_labels[i]->Draw();
			top_gauge_labels[i]->Draw();
			red_gauges[i]->Draw(0);
		}

		height->Draw();
		width->Draw();
		thick->Draw();
		tail->Draw();
		height_label->Draw();
		width_label->Draw();
		thick_label->Draw();
		tail_label->Draw();
		comb_stats->Draw();
	}

	if(sys->multiplayer)
	{
		if(sys->multi_1) players[0]->Draw();
		else players[1]->Draw();
	}
	else header->Draw();
}

void BoardFrontEnd::Select()
{
	game *game_ptr = app::inst()->get_game();
	int player_1_device = 1 << game_ptr->get_player_device( 0 );
	int player_2_device = 1 << game_ptr->get_player_device( 1 );
	
	int maxBoards = MAX_BOARDS + MAP_LOC_LAST;

	
	assert(current_board_index >= 0 && current_board_index < maxBoards);

	if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_REALISTIC_FE) && !unlocked) 
	{
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);		
		return;
	}
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);		
	if(sys->multiplayer)
		if(sys->multi_1)
		{
			g_game_ptr->SetBoardIdx(0,current_board_index);
			sys->multi_1 = false;
			sys->MakeActive(GraphicalMenuSystem::BoardMenu);

			BoardData bd = SurferBoardArray[g_game_ptr->GetSurferIdx(1)][0];
			int size = bd.height_feet * 12 + bd.height_inches;

			manager->em->UpdateBoardIndex(0, ROUND,size, true);
			
		}
		else
		{
			sys->SetDeviceFlags( player_1_device | player_2_device );
			manager->em->ExitState();
			g_game_ptr->SetBoardIdx(1,current_board_index);
			sys->multi_1 = true;
			frontendmanager.map_loading_screen = true;
			sys->PrepareToExit();
		}
	else
	{
		sys->SetDeviceFlags( player_1_device );
		manager->em->ExitState();
		g_game_ptr->SetBoardIdx(0,current_board_index);
		if(manager->tmp_game_mode == GAME_MODE_PRACTICE)
			g_game_ptr->set_level("indoor");
		frontendmanager.map_loading_screen = true;
		sys->PrepareToExit();
		if (frontendmanager.tmp_game_mode == GAME_MODE_CAREER)
			g_career->SetMyBoardId(current_board_index);
	}
}

// called with left-right movement
void BoardFrontEnd::OnHighlightBoard()
{
	int maxBoards = MAX_BOARDS+MAP_LOC_LAST;

	assert(current_board_index >= 0 && current_board_index < maxBoards);
	BoardData bd;
	
	if (current_board_index >= MAX_BOARDS)
		bd = LocationBoardArray[current_board_index - MAX_BOARDS];
	else
		bd = SurferBoardArray[surfer_index][current_board_index];

	int size = bd.height_feet * 12 + bd.height_inches;
	manager->em->UpdateBoardIndex(current_board_index, ROUND, size, false);
//	manager->em->UpdateBoardIndex(current_board_index, d.tail_type, size, availability[current_board_index] == Pickable);
	if (current_board_index < MAX_BOARDS)
	{
		/*if (frontendmanager.tmp_game_mode == GAME_MODE_CAREER)
			unlocked = g_career->IsBoardUnlocked(current_board_index);
		else*/
			unlocked = unlockManager.isSurferBoardUnlocked(surfer_index, current_board_index);
	}
	else
	{
		unlocked = IsLevelBoardUnlocked(current_board_index - MAX_BOARDS);
	}

	line_a->TurnOn(unlocked);
	line_b->TurnOn(unlocked);

	SetDisplay(bd);
}

void BoardFrontEnd::SetDisplay(BoardData bd)
{
	height->changeText(stringx(bd.height_feet)+"'"+stringx(bd.height_inches)+"\"");
	width->changeText(stringx(bd.width)+"\"");

	char tmp[16];
	sprintf(tmp, "%1.1f\"", bd.thickness);
	thick->changeText(tmp);

	stringx tt = "";
	switch(bd.tail_type)
	{
	case ROUND:		tt = ksGlobalTextArray[GT_FE_MENU_ROUND]; break;
	case SWALLOW:	tt = ksGlobalTextArray[GT_FE_MENU_SWALLOW]; break;
	case POINT:		tt = ksGlobalTextArray[GT_FE_MENU_PIN]; break;
	case FLAT:		tt = ksGlobalTextArray[GT_FE_MENU_SQUARE]; break;
	default: assert(0); break;
	}

	tt.to_lower();
	tail->changeText(tt);

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4; j++)
		{
			gauges[i][j]->TurnOn(unlocked);
			top_gauges[i][j]->TurnOn(unlocked);
		}
		if(unlocked) MaskGauge(i, bd);
	}

	if(unlocked) board_stats->changeText(ksGlobalTextArray[GT_FE_MENU_BOARD_STATS]);
	else board_stats->changeText(ksGlobalTextArray[GT_FE_MENU_LOCKED]);
}

void BoardFrontEnd::MaskGauge(int gauge, BoardData bd)
{
	SurferData sd = SurferDataArray[surfer_index];

	int blue_level = 0;
	int red_level = 0;
	int top_level = 0;

	bool career = frontendmanager.tmp_game_mode == GAME_MODE_CAREER;
	switch(gauge+GT_FE_MENU_SPIN_ATTR)
	{
	case GT_FE_MENU_BALANCE_ATTR:
		if (career && g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			blue_level = sd.attr_balance + 6;
		else if(career) blue_level = g_career->GetBalance();
		else blue_level = sd.attr_balance;
		top_level = bd.balance;
		break;
	case GT_FE_MENU_SPEED_ATTR:
		if (career && g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			blue_level = sd.attr_speed + 6;
		else if(career) blue_level = g_career->GetSpeed();
		else blue_level = sd.attr_speed;
		top_level = bd.speed;
		break;
	case GT_FE_MENU_JUMP_ATTR:
		if (career && g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			blue_level = sd.attr_air + 6;
		else if(career) blue_level = g_career->GetJump();
		else blue_level = sd.attr_air;
		top_level = bd.air;
		break;
	case GT_FE_MENU_SPIN_ATTR:
		if (career && g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			blue_level = sd.attr_spin + 6;
		else if(career) blue_level = g_career->GetSpin();
		else blue_level = sd.attr_spin;
		top_level = bd.spin;
		break;
	}

	// adjust blue level for handicap
	int hcap;
	if(sys->multiplayer && !sys->multi_1)
		hcap = g_game_ptr->get_player_handicap(1);
	else hcap = g_game_ptr->get_player_handicap(0);
	blue_level += hcap;

	red_level = blue_level + top_level;

	static const int MAX_LEVEL = 30;
	static const int MAX_TOP_LEVEL = 10;
	if(blue_level > MAX_LEVEL) blue_level = MAX_LEVEL;
	float blue_val = ((float) blue_level)/MAX_LEVEL;
	if(red_level > MAX_LEVEL) red_level = MAX_LEVEL;
	float red_val = ((float) red_level)/MAX_LEVEL;
	float top_val = ((float) top_level)/MAX_TOP_LEVEL;

	gauges[gauge][1]->Mask(blue_val);
	top_gauges[gauge][1]->Mask(top_val);
	red_gauges[gauge]->Mask(red_val);
}

void BoardFrontEnd::OnActivate()
{
	int index = 0;
	if(sys->multiplayer && !sys->multi_1) index = 1;
	surfer_index = g_game_ptr->GetSurferIdx(index);

	if (frontendmanager.tmp_game_mode == GAME_MODE_CAREER)
	{
		current_board_index = g_career->GetMyBoardId();
		if (current_board_index >= MAX_BOARDS)
		{
			if (!IsLevelBoardUnlocked(current_board_index-MAP_LOC_LAST))
				current_board_index = 0;
		}
		else if (!g_career->IsBoardUnlocked(current_board_index))
			current_board_index = 0;
	}
	else
		current_board_index = 0;

	sys->SetDeviceFlags( 1 << app::inst()->get_game()->get_player_device( index ) );
	sys->manager->em->LoadAuxStash();
	OnHighlightBoard();
	BoardData bd = SurferBoardArray[surfer_index][current_board_index];
	int size = bd.height_feet * 12 + bd.height_inches;

	manager->em->ToBoardSelect(current_board_index, ROUND, size);
	wait_for_camera = true;

	manager->helpbar->Reset();
	manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
	manager->helpbar->Reformat();
}

void BoardFrontEnd::OnTriangle(int c)
{
	if(sys->multiplayer && !sys->multi_1)
	{
		sys->multi_1 = true;
		sys->MakeActive(GraphicalMenuSystem::BoardMenu);
	}
	else
	{
		manager->em->ExitState();
		if(manager->tmp_game_mode == GAME_MODE_PRACTICE)
			system->MakeActive(GraphicalMenuSystem::SurferMenu);
		else system->MakeActive(GraphicalMenuSystem::BeachMenu);
	}
}

void BoardFrontEnd::OnCross(int c)
{
	if(manager->em->CamIsMoving()) return;
	Select();
}

void BoardFrontEnd::OnLeft(int c)
{
	bool oneBoard=false;
	int maxBoards = MAX_BOARDS + MAP_LOC_LAST;

	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);		
	if (sys->multiplayer)
	{
		if ((sys->multi_1) && (g_game_ptr->GetUsingPersonalitySuit(0) || (NumBoards[g_game_ptr->GetSurferIdx(0)] == 1)))
			oneBoard = true;	
		else if ((!sys->multi_1) && (g_game_ptr->GetUsingPersonalitySuit(1) || (NumBoards[g_game_ptr->GetSurferIdx(1)] == 1)))
			oneBoard = true;
	}
	else
		oneBoard = g_game_ptr->GetUsingPersonalitySuit(0) || (NumBoards[g_game_ptr->GetSurferIdx(0)] == 1);


		
	if (oneBoard)
	{
		if (current_board_index == MAX_BOARDS)
			current_board_index = PERSONALITY_BOARD;
		else
			current_board_index--;
	}
	else if (current_board_index > 0)
		current_board_index--;
	else 
		current_board_index = maxBoards -1;
	
	if (current_board_index < 0)
		current_board_index = maxBoards - 1;


	if (current_board_index >= MAX_BOARDS)
	{
		while (!IsLevelBoardUnlocked( current_board_index - MAX_BOARDS ) && current_board_index >= MAX_BOARDS)
		{
			if (current_board_index > MAX_BOARDS)
				current_board_index--;
			else if (oneBoard)
			{
				current_board_index = PERSONALITY_BOARD;
			}
			else
				current_board_index--;
		}
	}


	
	arrows[0][1]->TurnOn(true);
	arrows[0][0]->TurnOn(false);
	arrow_num = 0;
	arrow_counter = arrow_timer;

	OnHighlightBoard();
}

bool BoardFrontEnd::IsLevelBoardUnlocked(int whichLoc)
{
	if (whichLoc == MAP_LOC_NOWHERE || whichLoc == MAP_LOC_INDOOR)
		return false;

	return unlockManager.isLocationBoardUnlocked(whichLoc);
	/*if (manager->tmp_game_mode== GAME_MODE_CAREER)
		return g_career->locations[whichLoc].IsBoardUnlocked();
	else
		return globalCareerData.isLocationBoardUnlocked(whichLoc);*/
}

stringx BoardFrontEnd::GetLevelBoardName()
{
	int i;
	int location = BeachDataArray[g_game_ptr->get_beach_id()].map_location;
	for (i=0; i < BEACH_LAST; i++)
	{
		if (BeachDataArray[i].map_location == location)
			return BeachDataArray[i].stashfile;
	}
	return stringx("");
}

void BoardFrontEnd::OnRight(int c)
{
	int maxBoards = MAX_BOARDS + MAP_LOC_LAST;

	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);		
	bool oneBoard = false;
	if (sys->multiplayer)
	{
		if ((sys->multi_1) && (g_game_ptr->GetUsingPersonalitySuit(0) || (NumBoards[g_game_ptr->GetSurferIdx(0)] == 1)))
			oneBoard = true;	
		else if ((!sys->multi_1) && (g_game_ptr->GetUsingPersonalitySuit(1) || (NumBoards[g_game_ptr->GetSurferIdx(1)] == 1)))
			oneBoard = true;
	}
	else
		oneBoard = g_game_ptr->GetUsingPersonalitySuit(0) || (NumBoards[g_game_ptr->GetSurferIdx(0)] == 1);


	
	if (oneBoard)
	{
		if (current_board_index == PERSONALITY_BOARD)
			current_board_index = MAX_BOARDS;
		else 
			current_board_index++;
	}
	else
		current_board_index++;

	if (current_board_index >= maxBoards)
		current_board_index = 0;

	if (current_board_index >= MAX_BOARDS)
	{
		while (!IsLevelBoardUnlocked( current_board_index - MAX_BOARDS ) && 
		       current_board_index < (MAX_BOARDS + MAP_LOC_LAST) && 
					 current_board_index >= MAX_BOARDS)
		{
			current_board_index++;
			if (current_board_index == MAX_BOARDS + MAP_LOC_LAST)
			{
				current_board_index = 0;
			}
			
		}
	}

	arrow_counter = arrow_timer;
	arrows[1][1]->TurnOn(true);
	arrows[1][0]->TurnOn(false);
	arrow_num = 1;

	OnHighlightBoard();
}

void BoardFrontEnd::OnAnyButtonPress(int c, int b)
{
	if(!manager->em->CamIsMoving()) return;
	if(b == FEMENUCMD_TRIANGLE) manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_3_MAP);
	else manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_3_CLOSET);
}

void BoardFrontEnd::SetPQIndices()
{
	arrows[0][0] = GetPointer("button_left_off");
	arrows[0][1] = GetPointer("button_left_on");
	arrows[1][0] = GetPointer("button_right_off");
	arrows[1][1] = GetPointer("button_right_on");

	arrows[0][1]->TurnOn(false);
	arrows[1][1]->TurnOn(false);

	stringx types[] = { "spin", "speed", "air", "balance" };
	stringx abc[] = { "a", "b", "c", "hilite" };
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4; j++)
		{
			gauges[i][j] = GetPointer(("gauge_"+types[i]+"_"+abc[j]).data());
			top_gauges[i][j] = GetPointer(("gauge_"+types[i]+"_"+abc[j]+"_top").data());
		}

		gauges[i][1]->SetColor(0, 0, 1, 1);
		top_gauges[i][1]->SetColor(1, 0, 0, 1);
		red_gauges[i] = NEW PanelQuad(*gauges[i][1]);
		red_gauges[i]->SetColor(1, 0, 0, 1);
		red_gauges[i]->SetZ(285);
	}

	line_a = GetPointer("line_3");
	line_b = GetPointer("line_2");
}
