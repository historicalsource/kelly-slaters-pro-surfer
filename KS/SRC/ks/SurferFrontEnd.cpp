// SurferFrontEnd.cpp

// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if _XBOX
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "FrontEndManager.h"
#include "SurferFrontEnd.h"
#include "hwmovieplayer.h"
#include "wds.h"
#include "osdevopts.h"
#include "career.h"
//#include <game.h>
#include "geomgr.h"
#include "MusicMan.h"
#include "ksnsl.h"
#include "osgamesaver.h"
#include "globaldata.h"
#include "unlock_manager.h"
#ifdef TARGET_GC
#include <time.h>
#endif
extern world_dynamics_system* g_world_ptr;
extern game* g_game_ptr;

bool SurferFrontEnd::personality_up;



SurferFrontEnd::SurferFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf)
{
	int done_once = false;
	int i = 0;
	int first_index = -1;
	int min_sort_order = 1000;
	cons(s, man, p, pf);
	sys = (GraphicalMenuSystem*) s;
	manager = man;

	color = manager->col_unselected;
	color_high = manager->col_highlight;
	color_high_alt = manager->col_highlight2;
	flags |= FEMENU_HAS_COLOR_HIGH_ALT;

	bio_menu = NEW SurferBioFrontEnd(s, man, "levels\\frontend\\overlays\\", "biobook.PANEL", this);
	for (i=0; i < SURFER_LAST; i++)
	{
		if (SurferDataArray[i].sort_order >= 0 && SurferDataArray[i].sort_order < min_sort_order)
		{
			first_index = i;
			current_surfer_index = i;
			min_sort_order = SurferDataArray[i].sort_order;
		}
	}
	i = first_index;
	while (!done_once || i != first_index)
	{
		stringx name = SurferDataArray[i].firstname;
		name.append('\n');
		name.append(SurferDataArray[i].lastname);
		name.to_upper();
		FEMenuEntry* tmp = NEW FEMenuEntry(name, this);
		if(i == SURFER_KELLY_SLATER) ks = tmp;
		tmp->SetPos(365, 126);
		tmp->SetSpecialScale(0.8f, 0.8f);
		tmp->SetSpecialColor(manager->col_highlight2, manager->col_highlight);
		tmp->SetLineSpacing(24);
		tmp->SetHJustify(Font::HORIZJUST_LEFT);
		tmp->SetFont(&manager->font_thin);
		tmp->AddFont(1, &manager->font_bold);
		if(!done_once) first = tmp;
		Add(tmp);

		if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_PRESS_BUILD) &&
			i == SURFER_TRAVIS_PASTRANA)
			tmp->Disable(true);
		i = g_game_ptr->get_next_surfer_index(i);
		done_once = true;
		if (!(!done_once || i != first_index))
			last = tmp;

	}

	Bio = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_BIO], this, false, &manager->font_hand);
	Trick = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_TRICK_BOOK], this, false, &manager->font_hand);
	Personality = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PERSONALITY_SUIT], this, false, &manager->font_hand);
	Continue = NEW FEMenuEntry(ksGlobalTextArray[GT_MENU_CONTINUE], this, false, &manager->font_hand);
	ScrapBook = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_SCRAPBOOK], this, false, &manager->font_hand);
	Handicap = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_HANDICAP], this, false, &manager->font_hand);

	float sc_hand = 1.0f; // 0.5f;
	Bio->SetSpecialScale(sc_hand, sc_hand);
	Trick->SetSpecialScale(sc_hand, sc_hand);
	Personality->SetSpecialScale(sc_hand, sc_hand);
	ScrapBook->SetSpecialScale(sc_hand, sc_hand);
	Continue->SetSpecialScale(sc_hand, sc_hand);
	Handicap->SetSpecialScale(sc_hand, sc_hand);

	Bio->SetHJustify(Font::HORIZJUST_LEFT);
	Trick->SetHJustify(Font::HORIZJUST_LEFT);
	Personality->SetHJustify(Font::HORIZJUST_LEFT);
	ScrapBook->SetHJustify(Font::HORIZJUST_LEFT);
	Continue->SetHJustify(Font::HORIZJUST_LEFT);
	Handicap->SetHJustify(Font::HORIZJUST_LEFT);
	
	ScrapBook->Disable(true);
	Continue->Disable(true);
	Handicap->Disable(true);

	Add(Bio);
	Add(Trick);
	Add(Personality);
	Add(ScrapBook);
	Add(Continue);
	Add(Handicap);

	players[0] = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_PLAYER_1], 460, 82, 0, 0.89f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_info_b);
	players[1] = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_PLAYER_2], 460, 82, 0, 0.89f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_info_b);
	surfer_select = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_SURFER_SELECT], 460, 82, 0, 0.89f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->col_info_b);
	
	for(int i=0; i<4; i++)
	{
		stringx tmp = ksGlobalTextArray[GT_FE_MENU_SPIN_ATTR+i];
		tmp.to_lower();
		float size = 1.0f; //0.67f;
		gauge_labels[i] = NEW TextString(&manager->font_info, tmp, 365, 169+i*16, 0, size, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->col_info_b);
	}

	FEMenuEntry* tmp = first;
	while(tmp != last->next)
	{
		tmp->down = Continue;
		tmp->up = Personality;
		if(tmp != last) tmp->right = tmp->next;
		else tmp->right = first;
		if(tmp != first) tmp->left = tmp->previous;
		else tmp->left = last;
		tmp = tmp->next;
	}
	disp_state = -1;
	Bio->down = Trick;
	Bio->up = ScrapBook;
	Trick->down = Personality;
	Trick->up = Bio;
	Personality->up = Trick;
	Personality->down = first;
	ScrapBook->up = Continue;
	ScrapBook->down = Bio;
	Continue->up = first;
	Continue->down=ScrapBook;
	Handicap->down = Bio;
	Handicap->up = first;

	first_time_through = true;
	personality_up = false;
/*
	fade_in = false;
	fade_out = false;
*/
	arrow_counter = -5;
	wait_for_camera = false;
	in_tb_or_bio = false;

	if (!getPersonalityUp() && g_game_ptr->GetUsingPersonalitySuit(0))
	{
			personality_up = true;
	} 



}

void SurferFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();

//	ents[0] = entity_manager::inst()->find_entity("W2_CHARACTER", IGNORE_FLAVOR);
//	ents[0]->set_render_color(color32(255, 255, 255, front_start));

	bio_menu->Load();
}

SurferFrontEnd::~SurferFrontEnd()
{
	for(int i=0; i<4; i++)
	{
		delete gauge_labels[i];
		delete red_gauges[i];
	}

	delete players[0];
	delete players[1];
	delete surfer_select;
	delete bio_menu;
	delete hcap_color[2];
}

void SurferFrontEnd::SetPQIndices()
{
	for(int i=0; i<9; i++)
		ss_box[i] = GetPointer(("box_0"+stringx(i+1)).data());

	gauges[0][0] = GetPointer("gauge_spin_a");
	gauges[0][1] = GetPointer("gauge_spin_b");
	gauges[0][1]->SetColor(0, 0, 1, 1);
	gauges[0][2] = GetPointer("gauge_spin_c");
	gauges[1][0] = GetPointer("gauge_speed_a");
	gauges[1][1] = GetPointer("gauge_speed_b");
	gauges[1][1]->SetColor(0, 0, 1, 1);
	gauges[1][2] = GetPointer("gauge_speed_c");
	gauges[2][0] = GetPointer("gauge_air_a");
	gauges[2][1] = GetPointer("gauge_air_b");
	gauges[2][1]->SetColor(0, 0, 1, 1);
	gauges[2][2] = GetPointer("gauge_air_c");
	gauges[3][0] = GetPointer("gauge_balance_a");
	gauges[3][1] = GetPointer("gauge_balance_b");
	gauges[3][1]->SetColor(0, 0, 1, 1);
	gauges[3][2] = GetPointer("gauge_balance_c");

	for(int i=0; i<4; i++)
	{
		red_gauges[i] = NEW PanelQuad(*gauges[i][1]);
		red_gauges[i]->SetColor(2, 0, 0, 1);
		red_gauges[i]->SetZ(gauges[i][1]->z + 20);
	}

	horiz_arrows[0][0] = GetPointer("button_off_left");
	horiz_arrows[0][1] = GetPointer("button_on_left");
	horiz_arrows[1][0] = GetPointer("button_off_right");
	horiz_arrows[1][1] = GetPointer("button_on_right");

	horiz_arrows[0][1]->TurnOn(false);
	horiz_arrows[1][1]->TurnOn(false);

	ss_lines[0] = GetPointer("line_a");
	ss_lines[1] = GetPointer("line_b");
	ss_lines[2] = GetPointer("line_c");

	hcap_gauge = GetPointer("ss_handicap_gauge");		// static stuff
	hcap_slider = GetPointer("ss_handicap_indicator");	// slider
	hcap_color[0] = GetPointer("ss_handicap_minus");	// colored bar
	hcap_color[1] = GetPointer("ss_handicap_plus");		// colored bar
	hcap_color[0]->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//	hcap_color[1]->SetColor(1.0f, 0.5f, 0.5f, 1.0f);
	hcap_color[1]->SetColor(1.0f, 0.0f, 1.0f, 1.0f);
	hcap_color[2] = NEW PanelQuad(*hcap_color[1]);
	hcap_color[2]->SetZ(hcap_color[1]->z + 60);
	hcap_color[2]->SetColor(0.5f, 0.5f, 0.5f, 1.0f);

	// reset handicap here
	AdjustHandicapAbso(0);
}

void SurferFrontEnd::TurnPQ(bool on)
{
	for(int i=0; i<9; i++)
		ss_box[i]->TurnOn(on);
	for(int i=0; i<4; i++)
		for(int j=0; j<3; j++)
			gauges[i][j]->TurnOn(on);
	if (disp_state != DISP_STATS)
	{
		horiz_arrows[0][0]->TurnOn(on);
		horiz_arrows[1][0]->TurnOn(on);
		horiz_arrows[0][1]->TurnOn(false);
		horiz_arrows[1][1]->TurnOn(false);
		ss_lines[0]->TurnOn(on);
	}	

}

void SurferFrontEnd::SetDisplay(int d_state)
{
	if(disp_state == d_state) return;

	ScrapBook->Disable(d_state != DISP_STATS);
	Continue->Disable(d_state != DISP_STATS);
	Handicap->Disable(d_state != DISP_HANDICAP);
/*
	FEMenuEntry *tmp=first;
	int i=0;
	while(tmp && tmp != Bio)
	{
		tmp->Disable(d_state == DISP_STATS);
		tmp = tmp->next;
		i++;
	}
*/
	ss_lines[2]->TurnOn(d_state == DISP_HANDICAP);
	horiz_arrows[0][0]->TurnOn(d_state != DISP_STATS);
	horiz_arrows[1][0]->TurnOn(d_state != DISP_STATS);

	hcap_gauge->TurnOn(d_state == DISP_HANDICAP);
	hcap_slider->TurnOn(d_state == DISP_HANDICAP);
	hcap_color[0]->TurnOn(d_state == DISP_HANDICAP);
	hcap_color[1]->TurnOn(d_state == DISP_HANDICAP);

	if(d_state == DISP_STATS)
		surfer_select->changeText(ksGlobalTextArray[GT_FE_MENU_SURFER_STAT]);
	else surfer_select->changeText(ksGlobalTextArray[GT_FE_MENU_SURFER_SELECT]);

	switch(d_state)
	{
	case DISP_SELECT:
		Bio->SetPos(365, 244);
		Trick->SetPos(365, 262);
		Personality->SetPos(365, 280);
		Bio->up = current_surfer;
		current_surfer->down = Bio;
		setHigh(current_surfer);
		break;
	case DISP_HANDICAP:
		Bio->SetPos(365, 319);
		Trick->SetPos(365, 337);
		Personality->SetPos(365, 355);
		Handicap->SetPos(365, 244);
		Handicap->up = current_surfer;
		Handicap->down = Bio;
		Bio->up = Handicap;
		current_surfer->down = Handicap;
		current_surfer->up = Personality;
		Personality->down = current_surfer;
		setHigh(current_surfer);
		hcap = 0;
		break;
	case DISP_STATS:
		Continue->SetPos(365, 244);
		ScrapBook->SetPos(365, 262);
		Bio->SetPos(365, 280);
		Trick->SetPos(365, 298);
		Personality->SetPos(365, 316);
		Continue->up = Personality;
		Personality->down = Continue;
		ScrapBook->up = Continue;
		Continue->down = ScrapBook;
		Bio->up = ScrapBook;
		ScrapBook->down = Bio;
		setHigh(Continue);
		break;
	default: assert(0); break;
	}

	disp_state = d_state;
}

void SurferFrontEnd::Select(int entry_index)
{
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);

	if(state == BioHigh || state == TrickHigh || state == ScrapHigh) in_tb_or_bio = true;

	switch(state)
	{
	case OnSurfer:
		if(manager->em->CamIsMoving())
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
		else
		{
			manager->current_surfer = current_surfer_index;
			manager->em->SurferSelected();
			Pick(current_surfer_index);
		}
		break;
	case BioHigh:
		MakeActive(bio_menu);
		if(manager->em->CamIsMoving())
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_OUT);
		else manager->em->BioTBZoom(true);
		break;
	case TrickHigh:
		sys->MakeActive(GraphicalMenuSystem::TrickBookMenu);
		if(manager->em->CamIsMoving())
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_OUT);
		else manager->em->BioTBZoom(true);
		break;
	case PersHigh:
		if(manager->em->CamIsMoving())
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
		SetPersonality(!personality_up);
		break;
	case ScrapHigh:
		((AccompFrontEnd*)sys->menus[GraphicalMenuSystem::AccompMenu])->setBack(this);
		sys->MakeActive(GraphicalMenuSystem::AccompMenu);
		if(manager->em->CamIsMoving())
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_OUT);
		else manager->em->BioTBZoom(true);
		break;
	case ContHigh:
		if(manager->em->CamIsMoving())
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
		else
		{
			manager->current_surfer = current_surfer_index;
			manager->em->SurferSelected();
			Pick(current_surfer_index);
		}
		break;
	case HCapHigh: break;	// do nothing
	default: assert(0);
	}
}

struct nglMeshBatchInfo;

void SurferFrontEnd::Update(time_value_t time_inc)
{
	if(active) active->Update(time_inc);
	else
	{
		if(wait_for_camera && manager->em->OKtoDrawSurferSelect()) wait_for_camera = false;
		else if(!wait_for_camera && !manager->em->OKtoDrawSurferSelect()) wait_for_camera = true;
		FEGraphicalMenu::Update(time_inc);
		for(int i=0; i<4; i++)
		{
			red_gauges[i]->Update(time_inc);
			if(i<3) hcap_color[i]->Update(time_inc);
		}
	}
}

void SurferFrontEnd::Draw()
{
	if(active)
	{
		if(manager->em->OKtoDrawMain() || active == bio_menu)
		{
			// Other menus do all their drawing themselves, including the panel
			active->Draw();
		}
	}
	else
	{
		if(wait_for_camera) return;

		if(!parent) panel.Draw(0);

		if (disp_state == DISP_STATS)
		{
			ScrapBook->Draw();
			Continue->Draw();
		}

		current_surfer->Draw();
		if(!sys->multiplayer) surfer_select->Draw();

		if(disp_state == DISP_HANDICAP)
		{
			Handicap->Draw();
			hcap_color[2]->Draw(0);
		}

		Bio->Draw();
		Trick->Draw();
		if (stricmp(SurferDataArray[current_surfer_index].name_ps, "NONE"))
			Personality->Draw();

		for(int i=0; i<4; i++)
		{
			gauge_labels[i]->Draw();
			red_gauges[i]->Draw(0);
		}

		if(sys->multiplayer)
			if(sys->multi_1) players[0]->Draw();
			else players[1]->Draw();

		if(arrow_counter >= 0) arrow_counter--;
		if(arrow_counter < 0 && arrow_counter > -5)
		{
			arrow_counter = -5;
			horiz_arrows[0][0]->TurnOn(true);
			horiz_arrows[0][1]->TurnOn(false);
			horiz_arrows[1][0]->TurnOn(true);
			horiz_arrows[1][1]->TurnOn(false);
		}
		if(!parent) panel.Draw(1);
	}
}

stringx SurferFrontEnd::getName(int index)
{
	stringx	tmp(SurferDataArray[index].name);
	return tmp;
}

stringx SurferFrontEnd::getAbbr(int index)
{
	stringx	tmp(SurferDataArray[index].abbr);
	return tmp;
}

void SurferFrontEnd::Pick(int entry_index)
{
	const int &surferIdx = entry_index;
	stringx tmp;
	if(personality_up)
		tmp = stringx(SurferDataArray[entry_index].name_ps);
	else
		tmp = stringx(SurferDataArray[entry_index].name);

	if(sys->multiplayer)
		if(sys->multi_1)
		{
			app::inst()->get_game()->setHeroname(0,tmp);
			app::inst()->get_game()->SetSurferIdx(0,surferIdx);
			app::inst()->get_game()->set_player_device( 0, most_recent_controller );
			sys->multi_1 = false;
#ifdef REAL_ASYNC_LOAD
			stash::WaitForStashLoad();
#endif
			sys->MakeActive(GraphicalMenuSystem::SurferMenu);
		}
		else
		{
			app::inst()->get_game()->setHeroname(1,tmp);
			app::inst()->get_game()->SetSurferIdx(1,surferIdx);
			app::inst()->get_game()->set_player_device( 1, most_recent_controller );
			sys->multi_1 = true;
			sys->MakeActive(GraphicalMenuSystem::BeachMenu);
		}
	else
	{
		app::inst()->get_game()->setHeroname(0,tmp);
		app::inst()->get_game()->SetSurferIdx(0,surferIdx);
		app::inst()->get_game()->set_player_device( 0, most_recent_controller );
		if(manager->tmp_game_mode == GAME_MODE_CAREER && disp_state != DISP_STATS)
		{
			g_career->SetMyId(surferIdx);
			SetDisplay(DISP_STATS);
		}
		if(manager->tmp_game_mode == GAME_MODE_PRACTICE)
			sys->MakeActive(GraphicalMenuSystem::BoardMenu);
		else sys->MakeActive(GraphicalMenuSystem::BeachMenu);
	}
}

// called with left-right movement
void SurferFrontEnd::OnHighlightHero(int old_index)
{
	switch(disp_state)
	{
	case DISP_STATS:
		Personality->down = Continue;
		Continue->up = Personality;
		break;
	case DISP_HANDICAP:
		Handicap->up = current_surfer;
		current_surfer->down = Handicap;
		if((sys->multiplayer && sys->multi_1) || (!sys->multiplayer))
			AdjustHandicapAbso(g_game_ptr->get_player_handicap(0));
		else
			AdjustHandicapAbso(g_game_ptr->get_player_handicap(1));
		break;
	case DISP_SELECT:
		Bio->up = current_surfer;
		current_surfer->down = Bio;
		break;
	}

	if(disp_state != DISP_STATS)
	{
		Personality->down = current_surfer;
		current_surfer->up = Personality;
	}

	assert(current_surfer_index >= 0 && current_surfer_index < SURFER_LAST);

	g_game_ptr->SetSurferIdx(sys->multiplayer && !sys->multi_1 ? 1 : 0, current_surfer_index);

	Personality->Disable(!personality_unlocked[current_surfer_index]);
	bool use_pers = (manager->tmp_game_mode == GAME_MODE_CAREER && g_career->IsUsingPersonality() && personality_unlocked[current_surfer_index]);

	// also calls UpdateSurferIndex in EntityManager
	SetPersonality(use_pers);

	for(int i=0; i<4; i++)
		MaskWave(i);

	Bio->Disable(!BioAvailable());
}

bool SurferFrontEnd::BioAvailable()
{
	bool bio_exists = file_finder_exists("interface\\FE\\surfermenu\\" + getAbbr(current_surfer_index) + "_bio.txt", "");
	bool viewable = availability[current_surfer_index] == Viewable;
	return (bio_exists && !viewable);
}

void SurferFrontEnd::MaskWave(int wave)
{
//	int level = SurferDataArray[current_surfer_index].stats[wave];
	int level=0;
	int red_level=0;
	switch(wave)
	{
	case 0:
		level = SurferDataArray[current_surfer_index].attr_spin;
		if (g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			red_level = level + 6;
		else
			red_level = g_career->GetSpin();
		break;
	case 1:
		level = SurferDataArray[current_surfer_index].attr_speed;
		if (g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			red_level = level + 6;
		else
			red_level = g_career->GetSpeed();
		break;
	case 2:
		level = SurferDataArray[current_surfer_index].attr_air;
		if (g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			red_level = level + 6;
		else
			red_level = g_career->GetJump();
		break;
	case 3:
		level = SurferDataArray[current_surfer_index].attr_balance;
		if (g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())
			red_level = level + 6;
		else
			red_level = g_career->GetBalance();
		break;
	default:
		assert(false);
		level = 0;
		red_level = 0;
		break;
	}

	// only show red gauges if same surfer
	if((frontendmanager.tmp_game_mode != GAME_MODE_CAREER || g_career->GetSurferIdx() != current_surfer_index) &&
		!(frontendmanager.tmp_game_mode == GAME_MODE_CAREER && g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn()))
		red_level = level;

	// adjust for handicap
	if(disp_state == DISP_HANDICAP)
	{
		if(hcap < 0) level += hcap;
		red_level += hcap;
	}

	static const int MAX_LEVEL = 20;
	float val = ((float) level)/MAX_LEVEL;
	float red_val = ((float) red_level)/MAX_LEVEL;

	if(val > 1) val = 1;
	if(red_val > 1) red_val = 1;

	gauges[wave][1]->Mask(val);
	red_gauges[wave]->Mask(red_val);
}

void SurferFrontEnd::OnActivate()
{
	int i;
	
	// okay, cue screwy logic chain....
	if( !sys->multiplayer )
	{
		sys->SetDeviceFlags( FEMenuSystem::ALL_DEVICE_FLAGS );
	}
	else
	{
		if( sys->multi_1 || manager->tmp_game_mode == GAME_MODE_TIME_ATTACK ||
			manager->tmp_game_mode == GAME_MODE_METER_ATTACK )
			sys->SetDeviceFlags( FEMenuSystem::ALL_DEVICE_FLAGS );
		else
		{
			int player_1_device = 1 << app::inst()->get_game()->get_player_device( 0 );
			sys->SetDeviceFlags( ~player_1_device );
		}
	}
	
	if (manager->tmp_game_mode == GAME_MODE_CAREER)
	{
		g_game_ptr->set_player_handicap(0,0);
		g_game_ptr->set_player_handicap(1,0);
	}

	manager->em->ToSurferSelect();

	TurnPQ(true);

	for(i=0; i<SURFER_LAST; i++)
	{
		if(unlockManager.isSurferUnlocked(i) || SurferDataArray[i].initially_unlocked)
		{
			if (SurferDataArray[i].initially_unlocked)
				globalCareerData.unlockSurfer(i);

			availability[i] = Pickable;
		}
		else
			availability[i] = Hidden;

		// if not realistic_fe, then always allow personality suit, assuming it exists
		if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_REALISTIC_FE))
			personality_unlocked[i] = unlockManager.isSurferPersUnlocked(i) && stricmp(SurferDataArray[i].name_ps, "NONE") != 0 ;
		else personality_unlocked[i] = stricmp(SurferDataArray[i].name_ps, "NONE") != 0;
	}

	wait_for_camera = true;

	state = OnSurfer;
	current_surfer = first;
	/*if (!sys->multiplayer || (sys->multiplayer && sys->multi_1))
		g_game_ptr->set_player_handicap(0,0);
	else
		g_game_ptr->set_player_handicap(1,0);*/

	/*current_surfer_index = current_surfer->entry_num;
	if (!sys->multiplayer || (sys->multiplayer && sys->multi_1))
	{
		for (i=0; i < g_game_ptr->GetSurferIdx(0); i++)
		{
			current_surfer = current_surfer->next;
			current_surfer_index = current_surfer->entry_num;
		}
	}
	else
	{
		for (i=0; i < g_game_ptr->GetSurferIdx(1); i++)
		{
			current_surfer = current_surfer->next;
			current_surfer_index = current_surfer->entry_num;
		}
	}*/
	if(state == OnSurfer && first_time_through)
	{
		while(availability[current_surfer_index] == Hidden)
		{
			current_surfer_index++;
			current_surfer = current_surfer->next;
			// if this assert hits, then all surfers are Hidden; huge problem
			assert(current_surfer_index < SURFER_LAST);
		}
		first_time_through = false;
	}
	active = NULL;

	Bio->Disable(!BioAvailable());

	if(!sys->multiplayer) checkHigh(false);
	
	if (g_career->GetSurferIdx() != SURFER_LAST && (manager->tmp_game_mode == GAME_MODE_CAREER))
	{
		SetDisplay(DISP_STATS);
		if (getPersonalityUp() && !stricmp(SurferDataArray[g_career->GetSurferIdx()].name_ps, "NONE"))
			SetPersonality(false);
	

		manager->em->UpdateSurferIndex(g_career->GetSurferIdx());
		current_surfer_index = g_career->GetSurferIdx();
		current_surfer = first;
		for (i=g_game_ptr->get_first_surfer_index(); i != current_surfer_index; i = g_game_ptr->get_next_surfer_index(i))
		{
			current_surfer = current_surfer->next;
		}
	}
	else
	{
		if (getPersonalityUp() && !stricmp(SurferDataArray[current_surfer_index].name_ps, "NONE"))
			SetPersonality(false);

		manager->em->UpdateSurferIndex(current_surfer_index);
		current_surfer = first;
		for (i=g_game_ptr->get_first_surfer_index(); i != current_surfer_index; i = g_game_ptr->get_next_surfer_index(i))
		{
			current_surfer = current_surfer->next;
		}
		if(manager->tmp_game_mode == GAME_MODE_CAREER)
			SetDisplay(DISP_SELECT);
		else SetDisplay(DISP_HANDICAP);
	}
	if (disp_state == DISP_STATS)
		setHigh(Continue, false);
	else
		setHigh(current_surfer, false);
	
	// OnHighlightHero must be called after current_surfer_index is set
	// don't call OnHighlightHero if coming from Bio or TrickBook
	if(in_tb_or_bio) in_tb_or_bio = false;
	else OnHighlightHero();

	
	
	manager->helpbar->Reset();
	if(disp_state != DISP_STATS) manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
	manager->helpbar->AddArrowV();
	manager->helpbar->Reformat();
}

void SurferFrontEnd::SetState(int s)
{
	manager->helpbar->Reset();
	switch(s)
	{
	case OnSurfer:
		manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
		break;
	case BioHigh:
	case TrickHigh:
	case PersHigh:
	case ScrapHigh:
	case ContHigh:
		break;
	case HCapHigh:
		manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_CHANGE]);
		manager->helpbar->RemoveX();
		break;
	default: assert(0);
	}
	manager->helpbar->AddArrowV();
	manager->helpbar->Reformat();
	state = s;
}

void SurferFrontEnd::OnUp(int c)
{
	if(active) active->OnUp(c);
	else
	{
		if(availability[current_surfer_index] != Pickable) return;
		Up();
		checkHigh(true);
	}
}

void SurferFrontEnd::OnDown(int c)
{
	if(active) active->OnDown(c);
	else
	{
		if(availability[current_surfer_index] != Pickable) return;
		Down();
		checkHigh(true);
	}
}

void SurferFrontEnd::OnLeft(int c)
{
	if(active) active->OnLeft(c);
	else
	{
		if (disp_state == DISP_STATS) return;
		int old;
		switch(state)
		{
		case OnSurfer:
			SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT); 
			old = current_surfer_index;
			SetPersonality(false);
			do
			{
				if(current_surfer == first)
				{
					current_surfer = last;
					current_surfer_index = SURFER_LAST-1;
				}
				else
				{
					current_surfer = current_surfer->previous;
					current_surfer_index = g_game_ptr->get_prev_surfer_index(current_surfer_index);
				}
			} while(availability[current_surfer_index] == Hidden);

			setHigh(current_surfer);
			OnHighlightHero(old);
			horiz_arrows[0][1]->TurnOn(true);
			horiz_arrows[0][0]->TurnOn(false);
			arrow_num = 0;
			arrow_counter = arrow_timer;
			break;
		case TrickHigh:	
		case PersHigh:
		case BioHigh:	
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR); 
			Left(); checkHigh(true); 
			break;
		case HCapHigh:	AdjustHandicap(true); break;
		default: assert(0);
		}
	}
}

void SurferFrontEnd::OnRight(int c)
{
	if(active) active->OnRight(c);
	else
	{
		if (disp_state == DISP_STATS) return;
		int old;
		switch(state)
		{
		case OnSurfer:
			SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT); 
			SetPersonality(false);
			old = current_surfer_index;
			do
			{
				if(current_surfer == last)
				{
					current_surfer = first;
					current_surfer_index = g_game_ptr->get_first_surfer_index();
				}
				else
				{
					current_surfer = current_surfer->next;
					current_surfer_index = g_game_ptr->get_next_surfer_index(current_surfer_index);
				}
			} while(availability[current_surfer_index] == Hidden);

			setHigh(current_surfer);
			OnHighlightHero(old);
			arrow_counter = arrow_timer;
			horiz_arrows[1][1]->TurnOn(true);
			horiz_arrows[1][0]->TurnOn(false);
			arrow_num = 1;
			break;
		case TrickHigh:
		case PersHigh:
		case BioHigh:	
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR); 
			Right(); checkHigh(true); 
			break;
		case HCapHigh:	AdjustHandicap(false); break;
		default: assert(0);
		}
	}
}

void SurferFrontEnd::AdjustHandicap(bool left)
{
	if(left) AdjustHandicapAbso(hcap-1);
	else AdjustHandicapAbso(hcap+1);
}

void SurferFrontEnd::AdjustHandicapAbso(int absolute)
{
	hcap = absolute;
	if((hcap > hcap_max)  || (hcap > globalCareerData.getMaxHandicap(current_surfer_index)))
	{
		hcap = hcap_max > globalCareerData.getMaxHandicap(current_surfer_index)?globalCareerData.getMaxHandicap(current_surfer_index):hcap_max;
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	} 
	else if((hcap < -hcap_max))
	{
		hcap = -hcap_max;
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);

		

	if((sys->multiplayer && sys->multi_1) || (!sys->multiplayer))
		g_game_ptr->set_player_handicap(0, hcap); 
	else
		g_game_ptr->set_player_handicap(1, hcap); 

	static float hcap_inc = 15.83333f;
	static float hcap_x_width = hcap_max*2*hcap_inc;
	static float hcap_x_min = hcap_slider_x - hcap_x_width/2.0f;
	float x = hcap_slider_x + hcap*hcap_inc;
	hcap_slider->SetCenterPos(x, hcap_slider_y);
	hcap_color[0]->Mask((x - hcap_x_min)/hcap_x_width);
	x = hcap_slider_x + globalCareerData.getMaxHandicap(current_surfer_index)*hcap_inc;
	hcap_color[1]->Mask((x - hcap_x_min)/hcap_x_width);

	for(int i=0; i<4; i++)
		MaskWave(i);
}

void SurferFrontEnd::OnTriangle(int c)
{
	if(active)
	{
		if(manager->em->CamIsMoving() && in_tb_or_bio)
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
		active->OnTriangle(c);
	}
	else
		if(sys->multiplayer && !sys->multi_1)
		{
			if(manager->em->CamIsMoving())
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
			sys->multi_1 = true;
			sys->MakeActive(GraphicalMenuSystem::SurferMenu);
		}
		else
		{
			if(manager->em->CamIsMoving())
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_1);
			manager->em->ExitState();

			if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD))
				sys->MakeActive(GraphicalMenuSystem::MainMenu);
			else
				sys->MakeActive(GraphicalMenuSystem::MainMenu);
		}
}

void SurferFrontEnd::OnCross(int c)
{
	most_recent_controller = c;
	if(active)
	{
		if(manager->em->CamIsMoving())
		{
			if(in_tb_or_bio)
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_OUT);
		}
		else active->OnCross(c);
	}
	else FEMultiMenu::OnCross(c);
}

void SurferFrontEnd::SetPersonality(bool up)
{
	if(personality_unlocked[current_surfer_index] || !up)
	{
		personality_up = up;
		Personality->SetText(ksGlobalTextArray[personality_up ? GT_FE_MENU_REGULAR_SUIT : GT_FE_MENU_PERSONALITY_SUIT]);
		if (sys->multiplayer)
			g_game_ptr->SetUsingPersonalitySuit(sys->multi_1?0:1, personality_up);
		else g_game_ptr->SetUsingPersonalitySuit(0, personality_up);
		manager->em->UpdateSurferIndex(current_surfer_index);
		if(manager->tmp_game_mode == GAME_MODE_CAREER)
			g_career->SetUsingPersonality(personality_up);
	}
}

void SurferFrontEnd::OnAnyButtonPress(int c, int b)
{
	if(!manager->em->CamIsMoving()) return;

	// cross & triangle cases handled elsewhere
	if(b != FEMENUCMD_CROSS && b != FEMENUCMD_TRIANGLE)
	{
		if(in_tb_or_bio)
			manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_OUT);
		else manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
	}
}

void SurferFrontEnd::Up()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->up;
	while(tmp && tmp->GetDisable()) tmp = tmp->up;

	if(tmp)
	{
		setHigh(tmp, false);
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	}
	else if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
}

void SurferFrontEnd::Down()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->down;
	while(tmp && tmp->GetDisable()) tmp = tmp->down;

	if(tmp)
	{
		setHigh(tmp, false);
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		
	}
	else if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
}

void SurferFrontEnd::checkHigh(bool set_state)
{
	if(set_state) 
		SetState((highlighted == Bio) ? BioHigh : 
				(highlighted == Trick) ? TrickHigh : 
				(highlighted == Personality) ? PersHigh :
				(highlighted == ScrapBook) ? ScrapHigh : 
				(highlighted == Continue) ? ContHigh : 
				(highlighted == Handicap) ? HCapHigh : OnSurfer);
}

/********* SurferBioFrontEnd **********************************************/

SurferBioFrontEnd::SurferBioFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf, SurferFrontEnd* sfe)
{
	cons(s, man, p, pf);
	sys = (GraphicalMenuSystem*) s;
	bio_parent = sfe;
	parent = bio_parent;

	float sc_lg = 0.888f*1.18f;
	float sc_sm = 0.666f*1.5f;
	color32 c = manager->gray_dk;

	for(int i=0; i<SURFER_LAST; i++)
		bios[i] = NEW PreformatText(&manager->font_body, "", 336, 148, 0, sc_sm, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, c);

	color32 name_col = color32(29, 53, 74, 255);
	firstname = NEW TextString(&manager->font_bold_old, "", 336, 92, 0, sc_lg, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, name_col);
	lastname = NEW TextString(&manager->font_bold_old, "", 336, 116, 0, sc_lg, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, name_col);

	// allow up to 7 lines for the intro
	intro = NEW BoxText(&manager->font_body, "", 124, 288, 0, 0.6f*1.5f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, c, 7);
	counter = 0;
}

SurferBioFrontEnd::~SurferBioFrontEnd()
{
	for(int i=0; i<SURFER_LAST; i++)
		delete bios[i];
	delete firstname;
	delete lastname;
	delete intro;
}

void SurferBioFrontEnd::Load()
{
	FrontEnd::LoadPanel();
	SetPQIndices();
}

void SurferBioFrontEnd::Update(time_value_t time_inc)
{
	if(wait_for_camera && manager->em->OKtoDrawBio()) wait_for_camera = false;
	else if(!wait_for_camera && !manager->em->OKtoDrawBio()) wait_for_camera = true;

	if(up_pressed || down_pressed)
	{
		counter++;
		if(counter > count_max)
		{
			counter = 0;
			bios[bio_parent->current_surfer_index]->scroll(up_pressed, 1);
			UpdateScrollbar();
		}
	}
}

void SurferBioFrontEnd::UpdateScrollbar()
{
//	float per = bios[bio_parent->current_surfer_index]->GetFirstVisable()/((float) (num_lines - BIO_VIS_LINES));
	float per = bios[bio_parent->current_surfer_index]->getPercentage();
	float y = (scroll_marker_y_b - scroll_marker_y_t)*per + scroll_marker_y_t;
	scroll_marker->SetCenterPos(scroll_marker_x, y);
}

void SurferBioFrontEnd::Draw()
{
	if(wait_for_camera) return;

	panel.Draw(0);
	bios[bio_parent->current_surfer_index]->Draw();
	firstname->Draw();
	lastname->Draw();
	intro->Draw();
}

void SurferBioFrontEnd::OnActivate()
{
	FEMultiMenu::OnActivate();
	int index = bio_parent->current_surfer_index;
	firstname->changeText(SurferDataArray[index].firstname);
	lastname->changeText(SurferDataArray[index].lastname);

	stringx dir;
	if(((SurferFrontEnd*) parent)->personality_up)
		dir = stringx("characters\\personality") + SurferFrontEnd::getAbbr(index) + "\\";
	else dir = stringx("characters\\") + SurferDataArray[index].name + "\\";
	stringx bio_name = dir + SurferFrontEnd::getAbbr(index) + "_bio";
	stringx intro_name = bio_name+"_intro";

	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
	{
		bio_name += "_fr";
		intro_name += "_fr";
	}
	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
	{
		bio_name += "_ge";
		intro_name += "_ge";
	}

	bio_name += ".txt";
	bio_name.to_lower();
	bios[index]->readText((char*)(bio_name.data()), 350, BIO_VIS_LINES);

	intro_name += ".txt";
	intro_name.to_lower();
	intro->ReadFromFile((char*)(intro_name.data()), false);
	intro->makeBox(190, 150, false, -1, 16);

	for(int i=0; i<SURFER_LAST; i++)
		if(images[i]) images[i]->TurnOn(i == index);

	wait_for_camera = true;

	manager->helpbar->Reset();
	manager->helpbar->AddArrowV(ksGlobalTextArray[GT_FE_MENU_SCROLL]);
	manager->helpbar->RemoveX();
	manager->helpbar->Reformat();

	up_pressed = false;
	down_pressed = false;
	UpdateScrollbar();
}

void SurferBioFrontEnd::OnButtonRelease(int c, int b)
{
	if(b == FEMENUCMD_UP)
		up_pressed = false;
	else if(b == FEMENUCMD_DOWN)
		down_pressed = false;
}

void SurferBioFrontEnd::OnTriangle(int c)
{
	bios[bio_parent->current_surfer_index]->unreadText();
	manager->em->BioTBZoom(false);
	FEMultiMenu::OnTriangle(c);
}

void SurferBioFrontEnd::SetPQIndices()
{
	for(int i=0; i<SURFER_LAST; i++)
	{
		stringx tmp = SurferFrontEnd::getAbbr(i);
		tmp[1] = '_';		// want just first character
		tmp = "bio_image_" + tmp + stringx(SurferDataArray[i].lastname);

		// yeah, yeah i know.  but i just gave up on consistency
		if(i == SURFER_TRAVIS_PASTRANA)
			tmp = "bio_image_pastrana";
		else if(i == SURFER_TIKI_GOD)
			tmp = "bio_image_tiki";
		else if(i == SURFER_TONY_HAWK)
			tmp = "bio_image_tonyhawk";
		else if(i == SURFER_SURFREAK)
			tmp = "bio_image_surfreak";

		tmp.to_lower();
		images[i] = GetPointer(tmp.data());
		if(images[i] == frontendmanager.GetDefaultPQ()) images[i] = NULL;
	}

	scroll_marker = GetPointer("scroll_marker");
	scroll_marker->GetCenterPos(scroll_marker_x, scroll_marker_y_t);
	scroll_marker_y_b = 357.0f;

	GetPointer("button_b_off_01")->TurnOn(false);
	GetPointer("button_b_on_01")->TurnOn(false);
	GetPointer("button_b_off_02")->TurnOn(false);
	GetPointer("button_b_on_02")->TurnOn(false);
}
