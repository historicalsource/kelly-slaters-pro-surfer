// TrickBookFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"
#include "unlock_manager.h"
#include "TrickBookFrontEnd.h"

#if defined(TARGET_XBOX)
#include "FrontEndManager.h"
#endif /* TARGET_XBOX JIV DEBUG */

extern SurferTrick GTrickList[TRICK_NUM];

TrickBookFrontEnd::TrickBookFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;

	float sc_lg = 0.888f*1.18f;
	float sc_sm = 0.666f*1.18f;
	color32 black = manager->gray_dk;
	color = black;
	color_high = manager->red_dk;
	color_high_alt = color;
//	color32 button_color = manager->blue;
	color32 dk_grn = color32(0, 50, 0, 255);
	scale = sc_sm;
	scale_high = sc_sm;

	trickbook = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_FE_MENU_TRICK_BOOK], 304, 116, 0, sc_lg, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER, black);
	buttons = NEW TextString(&manager->font_body, "", 441, 348, 0, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);
	buttons->setButtonScale(1.0f);

	for(int i=0; i<NUM_TYPES; i++)
		State2[i] = NEW FETextMultiMenu(s, color, color, 0.6f*1.5f, 0.6f*1.5f);

	FEMenuEntry* tmp_entries[NUM_TYPES];
	for(int i=0; i<NUM_TYPES; i++)
	{
		tmp_entries[i] = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_AERIALS+i], this, false, &manager->font_bold_old);
		tmp_entries[i]->SetPos(304, 148 + 18 * i);
		tmp_entries[i]->SetHJustify(Font::HORIZJUST_RIGHT);
		Add(tmp_entries[i]);
	}

	int surfer = manager->current_surfer;
	if(surfer < 0 || surfer > 9) surfer = 0;

	SurferTrick* st;
	int inst[NUM_TYPES];
	for(int i=0; i<NUM_TYPES; i++) inst[i] = 0;

	for(int i=0; i<TRICK_NUM; i++)
		if(TrickOK(i)) inst[GTrickList[i].trickbook_type]++;

	for(int i=0; i<NUM_TYPES; i++)
	{
		if(inst[i] == 0) tricks[i] = NULL;
		else tricks[i] = NEW SurferTrick*[inst[i]];
		inst[i] = 0;
	}

	// add special tricks to a waiting list to be added later
	FEMenuEntry* tmp_sp_entries[NUM_TYPES][64];
	SurferTrick* tmp_sp_tricks[NUM_TYPES][64];
	int tmp_sp_count[NUM_TYPES];
	for(int i=0; i<NUM_TYPES; i++)	tmp_sp_count[i] = 0;

	for(int i=0; i<TRICK_NUM; i++)
	{
		if(TrickOK(i))
		{
			st = &(GTrickList[i]);
			int type = st->trickbook_type;
			FEMenuEntry* tmp = NEW FEMenuEntry(ksGlobalTrickTextArray[st->trick_id], State2[type], false, &manager->font_body);
			tmp->SetPos(441, 321);
			if(st->flags & SpecialFlag)
			{
				tmp->SetSpecialColor(dk_grn, dk_grn);
				tmp_sp_entries[type][tmp_sp_count[type]] = tmp;
				tmp_sp_tricks[type][tmp_sp_count[type]] = st;
				tmp_sp_count[type]++;
			}
			else
			{
				State2[type]->Add(tmp);
				tricks[type][inst[type]] = st;
				inst[type]++;
			}
		}
	}

	for(int i=0; i<NUM_TYPES; i++)
		for(int j=0; j<tmp_sp_count[i]; j++)
		{
			State2[i]->Add(tmp_sp_entries[i][j]);
			tricks[i][inst[i]] = tmp_sp_tricks[i][j];
			inst[i]++;
		}

	for(int i=0; i<NUM_TYPES; i++)
	{
		if(inst[i] == 0)
			tmp_entries[i]->Disable();
		else
		{
			FEMenuEntry* tmp = State2[i]->entries;
			while(tmp)
			{
				tmp->left = tmp->previous;
				tmp->right = tmp->next;
				if(tmp->next == NULL) tmp->right = State2[i]->entries;
				tmp = tmp->next;
			}
		}

		if(i != 0) tmp_entries[i]->up = tmp_entries[i]->previous;
		else tmp_entries[i]->up = tmp_entries[NUM_TYPES-1];
		if(i != NUM_TYPES-1) tmp_entries[i]->down = tmp_entries[i]->next;
		else tmp_entries[i]->down = tmp_entries[0];
	}

	arrow_counter = -5;
}

TrickBookFrontEnd::~TrickBookFrontEnd()
{
	delete buttons;
	delete trickbook;
	for(int i=0; i<NUM_TYPES; i++)
	{
		delete State2[i];
		delete[] tricks[i];
	}
}

void TrickBookFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();
}

void TrickBookFrontEnd::Init()
{
	FEMultiMenu::Init();
	FEMenuEntry* tmp = entries;
	int i=0;
	while(tmp)
	{
		if(!tmp->GetDisable())
//			State2[i]->Init();
			State2[i]->setHigh(State2[i]->entries);
		tmp = tmp->next;
		i++;
	}
}

void TrickBookFrontEnd::SetPQIndices()
{
	arrows[0][0] = GetPointer("button_off_01");
	arrows[0][1] = GetPointer("button_on_01");
	arrows[1][0] = GetPointer("button_off_02");
	arrows[1][1] = GetPointer("button_on_02");

	arrows[0][1]->TurnOn(false);
	arrows[1][1]->TurnOn(false);

	bkg = GetPointer("trickbook_window");
	bkg->TurnOn(false);	// drawn separately
}

void TrickBookFrontEnd::Select(int entry_index)
{
	// don't allow new trick to be played until old one is done
	if(!manager->em->OKtoDrawBio() || !manager->em->TrickAnimDone()) return;
	SurferTrick* st = tricks[entry_index][State2[entry_index]->highlighted->entry_num];
	manager->em->PlaySurferTrick(st);
}

void TrickBookFrontEnd::OnTriangle(int c)
{
	manager->em->ExitState();
	sys->MakeActive(GraphicalMenuSystem::SurferMenu, SurferFrontEnd::ACT_SURFER);
	manager->em->BioTBZoom(false);
}

void TrickBookFrontEnd::Update(time_value_t time_inc)
{
	if(wait_for_camera && manager->em->OKtoDrawBio()) wait_for_camera = false;
	else if(!wait_for_camera && !manager->em->OKtoDrawBio()) wait_for_camera = true;
	State2[highlighted->entry_num]->Update(time_inc);
	FEMultiMenu::Update(time_inc);
}

void TrickBookFrontEnd::OnUp(int c)
{
	FEMultiMenu::OnUp(c);
	int high = highlighted->entry_num;
	ChangeButtonText(tricks[high][State2[high]->highlighted->entry_num]);
}

void TrickBookFrontEnd::OnDown(int c)
{
	FEMultiMenu::OnDown(c);
	int high = highlighted->entry_num;
	ChangeButtonText(tricks[high][State2[high]->highlighted->entry_num]);
}

void TrickBookFrontEnd::OnLeft(int c)
{
	arrows[0][1]->TurnOn(true);
	arrows[0][0]->TurnOn(false);
	arrow_num = 0;
	arrow_counter = arrow_timer;

	int high = highlighted->entry_num;
	int num = State2[high]->highlighted->entry_num;
	State2[high]->OnLeft(c);
	if(num != State2[high]->highlighted->entry_num)
		ChangeButtonText(tricks[high][State2[high]->highlighted->entry_num]);
}

void TrickBookFrontEnd::OnRight(int c)
{
	arrows[1][1]->TurnOn(true);
	arrows[1][0]->TurnOn(false);
	arrow_num = 1;
	arrow_counter = arrow_timer;

	int high = highlighted->entry_num;
	int num = State2[high]->highlighted->entry_num;
	State2[high]->OnRight(c);
	if(num != State2[high]->highlighted->entry_num)
		ChangeButtonText(tricks[high][State2[high]->highlighted->entry_num]);
}

void TrickBookFrontEnd::Draw()
{
	if(wait_for_camera) return;

	bkg->TurnOn(true);
	bkg->Draw(0);
	bkg->TurnOn(false);
}

void TrickBookFrontEnd::DrawTop()
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
	State2[highlighted->entry_num]->highlighted->Draw();
	buttons->Draw();
	trickbook->Draw();
}

void TrickBookFrontEnd::OnActivate()
{
	manager->em->ToTrickBook();

	// Disable tricks that haven't been learned yet, or don't exist for current surfer;
	//  but only in the Realistic FE
	if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_REALISTIC_FE))
	{
		for(int i=0; i<NUM_TYPES; i++)
		{
			FEMenuEntry* tmp = State2[i]->entries;
			int j=0;
			while(tmp)
			{
				int id = tricks[i][j]->trick_id;
				bool not_learned = GTrickList[id].flags & SpecialFlag && !unlockManager.isSurferTrickUnlocked(id);
				bool not_avail = true;

				for(int i=0; i<TRICKBOOK_SIZE; i++)
					if(id == SurferDataArray[manager->em->GetCurrentSurfer()].trickBook[i])
					{
						not_avail = false;
						break;
					}

				tmp->Disable(not_avail || not_learned);
 				j++;
				tmp = tmp->next;
			}
		}
	}

	FEMultiMenu::OnActivate();
	ChangeButtonText(tricks[highlighted->entry_num][0]);
	State2[highlighted->entry_num]->OnActivate();
	wait_for_camera = true;

	manager->helpbar->Reset();
	manager->helpbar->AddArrowV();
	manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
	manager->helpbar->SetXText(ksGlobalTextArray[GT_FE_MENU_PLAY]);
	manager->helpbar->Reformat();
}

void TrickBookFrontEnd::ChangeButtonText(SurferTrick* st)
{
	stringx tmp = "";
	if(st->button1 != PAD_NONE)
	{
		// One-button tricks.
		if (st->button2 == PAD_NONE)
			tmp = ksGlobalButtonArray[st->button1-1];
		// Two-button tricks.
		else if (st->button3 == PAD_NONE)
		{
			// Hack: don't show the "+" for floater trick.
			if (st->trick_id == TRICK_FLOATER)
				tmp = ksGlobalButtonArray[st->button1-1] + " " + ksGlobalButtonArray[st->button2-1];
			else if (st->trickbook_type == TRICKBOOKTYPE_FACETRICK)
				tmp = ksGlobalButtonArray[st->button1-1] + " , " + ksGlobalButtonArray[st->button2-1];
			else
				tmp = ksGlobalButtonArray[st->button1-1] + " + " + ksGlobalButtonArray[st->button2-1];
		}
		// Three-button tricks.
		else
			tmp = ksGlobalButtonArray[st->button1-1] + ", " + ksGlobalButtonArray[st->button2-1] + ", " + ksGlobalButtonArray[st->button3-1];
	}
	buttons->changeText(tmp);
}

bool TrickBookFrontEnd::TrickOK(int t)
{
	SurferTrick* st = &(GTrickList[t]);
	bool right_type = st->trickbook_type != TRICKBOOKTYPE_NOTYPE;
	bool has_buttons = (st->button1 != PAD_NONE);
	bool has_anim = st->anim_id != -1;
	bool bad_trick = false;
/*
	// having problems with this trick
	if(ksGlobalTrickTextArray[st->trick_id] == "GRAB N DRAG")
		bad_trick = true;
*/
	return (right_type && has_buttons && has_anim && !bad_trick);
}


