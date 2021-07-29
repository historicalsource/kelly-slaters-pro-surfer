// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

// MultiFrontEnd.cpp


// not currently being used
#if 0


#include "FrontEndManager.h"
#include "MultiFrontEnd.h"

#if defined(TARGET_XBOX)
#include "osdevopts.h"
#include "MusicMan.h"
#include "ksnsl.h"
#endif /* TARGET_XBOX JIV DEBUG */


MultiFrontEnd::MultiFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	int i;
	FEGraphicalMenuEntry* entry[num_ops];
	for(i=0; i<num_ops; i++)
	{
		entry[i] = NEW FEGraphicalMenuEntry(this);
		Add(entry[i]);
	}
	for(i=0; i<num_ops; i++)
	{
		entry[i]->left = NULL;
		entry[i]->right = NULL;
		if(i == 0) entry[i]->up = entry[num_ops-1];
		else entry[i]->up = entry[i-1];
		if(i == num_ops-1) entry[i]->down = entry[0];
		else entry[i]->down = entry[i+1];
	}
/*
	push_title = NEW TextString(&manager->bio_font_new, ksGlobalTextArray[GT_MultiPush], 316, 82, 0,
		manager->bio_scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	attack_title = NEW TextString(&manager->bio_font_new, ksGlobalTextArray[GT_MultiAttack], 316, 82, 0,
		manager->bio_scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	horse_title = NEW TextString(&manager->bio_font_new, ksGlobalTextArray[GT_MultiHorse], 316, 82, 0,
		manager->bio_scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	push = NEW TextString(&manager->bio_font_new, ksGlobalTextArray[GT_MultiPushDes], 316, 130, 0,
		manager->bio_scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	attack = NEW TextString(&manager->bio_font_new, ksGlobalTextArray[GT_MultiAttackDes], 316, 130, 0,
		manager->bio_scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	horse = NEW TextString(&manager->bio_font_new, ksGlobalTextArray[GT_MultiHorseDes], 316, 130, 0,
		manager->bio_scale, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
		*/
	float sc = manager->bio_scale*1.5f;

	pushTitle = NEW TextString(&manager->bold, ksGlobalTextArray[GT_MODE_PUSH], 316, 82, 0, sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	headToHeadTitle = NEW TextString(&manager->bold, ksGlobalTextArray[GT_MODE_HEAD_TO_HEAD], 316, 82, 0, sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	timeAttackTitle = NEW TextString(&manager->bold, ksGlobalTextArray[GT_MODE_TIME_ATTACK], 316, 82, 0, sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	//meterAttackTitle = NEW TextString(&manager->bold, ksGlobalTextArray[GT_MODE_METER_ATTACK], 316, 82, 0, sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);

	pushDesc = NEW BoxText(&manager->body, ksGlobalTextArray[GT_MODE_PUSH_DESC], 316, 130, 0, sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	headToHeadDesc = NEW BoxText(&manager->body, ksGlobalTextArray[GT_MODE_HEAD_TO_HEAD_DESC], 316, 130, 0, sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	timeAttackDesc = NEW BoxText(&manager->body, ksGlobalTextArray[GT_MODE_TIME_ATTACK_DESC], 316, 130, 0,sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	//meterAttackDesc = NEW BoxText(&manager->body, ksGlobalTextArray[GT_MODE_METER_ATTACK_DESC], 316, 130, 0,sc, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);

	pushDesc->makeBox(240, 260);
	headToHeadDesc->makeBox(240, 260);
	timeAttackDesc->makeBox(240, 260);
	//meterAttackDesc->makeBox(240, 260);
}

MultiFrontEnd::~MultiFrontEnd()
{
	delete pushTitle;
	delete headToHeadTitle;
	delete timeAttackTitle;
	//delete meterAttackTitle;
	delete pushDesc;
	delete headToHeadDesc;
	delete timeAttackDesc;
	//delete meterAttackDesc;
}

void MultiFrontEnd::Draw()
{
	if(cur_index == MultiPushEntry)
	{
		pushTitle->Draw();
		pushDesc->Draw();
	}
	if(cur_index == MultiHeadToHeadEntry)
	{
		headToHeadTitle->Draw();
		headToHeadDesc->Draw();
	}
	if (cur_index == MultiTimeAttackEntry)
	{
		timeAttackTitle->Draw();
		timeAttackDesc->Draw();
	}
	/*
	if (cur_index == MultiMeterAttackEntry)
	{
		meterAttackTitle->Draw();
		meterAttackDesc->Draw();
	}
	*/
	FEMultiMenu::Draw();
}

void MultiFrontEnd::Load()
{
	FEGraphicalMenu::Load();
	SetPQIndices();

	FEGraphicalMenuEntry* tmp = (FEGraphicalMenuEntry*)entries;
	for(int i=0; i<num_ops; i++)
	{
		tmp->Load(pq[i], pqhi[i]);
		tmp = (FEGraphicalMenuEntry*) tmp->next;
	}
}

void MultiFrontEnd::Select(int entry_index)
{
	switch(cur_index)
	{
	case MultiPushEntry :	g_game_ptr->set_game_mode(GAME_MODE_PUSH); break;
	case MultiHeadToHeadEntry :	g_game_ptr->set_game_mode(GAME_MODE_HEAD_TO_HEAD); break;
	case MultiTimeAttackEntry :	g_game_ptr->set_game_mode(GAME_MODE_TIME_ATTACK); break;
	//case MultiMeterAttackEntry :	g_game_ptr->set_game_mode(GAME_MODE_METER_ATTACK); break;
	default: assert(0);	break;
	}

	manager->em->ExitState();
	system->MakeActive(GraphicalMenuSystem::SurferMenu);
}

void MultiFrontEnd::SetPQIndices()
{
	pq[MultiPushEntry] = GetPointer("btn_push1");
	pq[MultiHeadToHeadEntry] = GetPointer("btn_head1");
	pq[MultiTimeAttackEntry] = GetPointer("btn_attack1");
	//pq[MultiMeterAttackEntry] = GetPointer("btn_attack1");
	pqhi[MultiPushEntry] = GetPointer("btn_push2");
	pqhi[MultiHeadToHeadEntry] = GetPointer("btn_head2");
	pqhi[MultiTimeAttackEntry] = GetPointer("btn_attack2");
	//pqhi[MultiMeterAttackEntry] = GetPointer("btn_attack2");

	for(int i=0; i<9; i++)
		box[i] = GetPointer(("mp_box_0"+stringx(i+1)).data());
}

void MultiFrontEnd::OnUp(int c)
{
	cur_index--;
	if(cur_index < 0) cur_index = MultiEndEntry-1;
	FEMultiMenu::OnUp(c);
}

void MultiFrontEnd::OnDown(int c)
{
	cur_index++;
	if(cur_index >= MultiEndEntry) cur_index = 0;
	FEMultiMenu::OnDown(c);
}

void MultiFrontEnd::OnActivate()
{
	cur_index = MultiPushEntry;
	FEMultiMenu::OnActivate();
}

void MultiFrontEnd::OnTriangle(int c)
{
//	manager->em->ExitState();
	system->MakeActive(GraphicalMenuSystem::MainMenu);
}



#endif // 0
