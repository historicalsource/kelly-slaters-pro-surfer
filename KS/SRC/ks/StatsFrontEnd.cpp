// StatsFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if _XBOX
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */



// not currently being used
#if 0


#include "FrontEndManager.h"
#include "StatsFrontEnd.h"
#include "wds.h"
#include "osdevopts.h"
#include "geomgr.h"
#include "MusicMan.h"
#include "career.h"

extern game* g_game_ptr;


StatsFrontEnd::StatsFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;

	FEMenuEntry* tmp;
	for(int i=0; i<num_beaches; i++)
	{
		tmp = NEW FEGraphicalMenuEntry(this);
		if(i == 0) first = tmp;
		if(i == SURFER_LAST-1) last = tmp;
		Add(tmp);
		pq_beach[i] = NULL;
	}

	tmp = first;
	while(tmp != last->next)
	{
		tmp->up = NULL;
		tmp->down = NULL;
		if(tmp != last) tmp->right = tmp->next;
		else tmp->right = first;
		if(tmp != first) tmp->left = tmp->previous;
		else tmp->left = last;
		tmp = tmp->next;
	}

/*	ts_float = NEW TextString(&manager->bio_font_new, "", 370, 202, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_air = NEW TextString(&manager->bio_font_new, "", 370, 234, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_tube = NEW TextString(&manager->bio_font_new, "", 370, 265, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_score = NEW TextString(&manager->bio_font_new, "", 370, 297, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_trick = NEW TextString(&manager->bio_font_new, "", 370, 330, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
*/
	ts_float = NEW TextString(&manager->bold, "", 370, 202, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_air = NEW TextString(&manager->bold, "", 370, 234, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_tube = NEW TextString(&manager->bold, "", 370, 265, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_score = NEW TextString(&manager->bold, "", 370, 297, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
	ts_trick = NEW TextString(&manager->bold, "", 370, 330, 0, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
}

void StatsFrontEnd::Load()
{
	FEGraphicalMenu::Load();
	SetPQIndices();

	FEGraphicalMenuEntry* tmp = (FEGraphicalMenuEntry*)entries;
	for(int i=0; i<num_beaches; i++)
	{
		if(pq_beach[i]) tmp->Load(pq_beach[i]);
		tmp = (FEGraphicalMenuEntry*) tmp->next;
	}
}

StatsFrontEnd::~StatsFrontEnd()
{
	delete ts_tube;
	delete ts_float;
	delete ts_air;
	delete ts_score;
	delete ts_trick;

	delete[] extras;
	delete[] extra_indices;
	delete[] extras_b;
	delete[] extra_indices_b;
}

void StatsFrontEnd::SetPQIndices()
{
	num_extras = 0;

	for (int i = 0; i < SURFER_LAST; ++i)
	{
		stringx name = stringx("ss_name_") + stringx(SurferDataArray[i].lastname);
		name.to_lower();
		pq_name[i] = GetPointer(name.c_str());
		if(!pq_name[i]) num_extras++;
	}

	extra_indices = NEW int[num_extras];
	extras = NEW TextString[num_extras];

	int count = 0;

	for(int i=0; i<SURFER_LAST; i++)
		if(!pq_name[i])
		{
			extra_indices[count] = i;
//			extras[count] = TextString(&manager->bio_font_new, SurferDataArray[i].lastname, 
			extras[count] = TextString(&manager->bold, SurferDataArray[i].lastname, 
				320, 100, 0, 1, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER);
			count++;
		}

	num_extras_b = 0;

	for(int i=0; i<BEACH_LAST; i++)
	{
		stringx name = stringx("option_") + stringx(BeachDataArray[i].folder);
		name.to_lower();
		pq_beach[i] = GetPointer(name.c_str());
		if(!pq_beach[i]) num_extras_b++;
	}

	extra_indices_b = NEW int[num_extras_b];
	extras_b = NEW TextString[num_extras_b];

	count = 0;

	for(int i=0; i<BEACH_LAST; i++)
		if(!pq_beach[i])
		{
			extra_indices_b[count] = i;
//			extras_b[count] = TextString(&manager->bio_font_new, BeachDataArray[i].folder, 
			extras_b[count] = TextString(&manager->bold, BeachDataArray[i].folder, 
				340, 165, 0, .8, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, manager->bio_font_blue);
			count++;
		}

	pq_beach[BeachOverallEntry] = GetPointer("option_overall");
	
	

	/*
	// This needs to be an entry in the beach database! (dc 01/17/02)
	pq_beach[BEACH_BELLS] = GetPointer("option_bells");
	pq_beach[BEACH_GLAND] = GetPointer("option_gland");
	pq_beach[BEACH_TRESTLES] = GetPointer("option_trestles");
	pq_beach[BEACH_JEFFERSONBAY] = GetPointer("option_jeffery'sbay");
	pq_beach[BEACH_JAWS] = GetPointer("option_jaws");
	pq_beach[BEACH_MAVERICKS] = GetPointer("option_mavericks");
	pq_beach[BEACH_MUNDAKA] = GetPointer("option_mundaka");
	pq_beach[BEACH_PIPELINE] = GetPointer("option_pipeline");
	pq_beach[BEACH_SEBASTIAN] = GetPointer("option_sebastian");
	pq_beach[BEACH_TEAHUPOO] = GetPointer("option_teahupoo");
	pq_beach[BEACH_ANTARCTICA] = GetPointer("option_antarctica");
	pq_beach[BeachOverallEntry] = GetPointer("option_overall");
	*/
}

// called with left-right movement
void StatsFrontEnd::OnSwitchBeach()
{
	//float	tube, floater, air;
	//int		score;
	//int		t_score;
	
	// change the values for float, air, tube, etc. here
	for (int i=0; i<num_beaches; i++)
	{
		if (pq_beach[i])
			pq_beach[i]->TurnOn(cur_beach == i);
	}
		
	//	Career::Trick* trick = NEW Career::Trick();
		
		
	if(!pq_beach[cur_beach])
	{
		for(int i=0; i<num_extras_b; i++)
		{
			if(extra_indices_b[i] == cur_beach)
			{
				current_extra_index_b = i;
				break;
			}
		}
	}
				
	if(cur_beach != BeachOverallEntry)
	{
	/*		g_career->GetLongestTubeRide(cur_beach, tube);
				g_career->GetLongestFloater(cur_beach, floater);
				g_career->GetLongestAir(cur_beach, air);
				//		g_career->GetBestTrick(cur_beach, trick, t_score);
				g_career->GetBestScore(cur_beach, score);
				ts_tube->changeText(stringx(tube));
				ts_float->changeText(stringx(floater));
				ts_air->changeText(stringx(air));
				ts_score->changeText(stringx(score));
				//		ts_trick->changeText(stringx(t_score));
	*/
	}
	else
	{
		float max_tube=0, max_float=0, max_air=0;
		int max_score=0;//, max_t_score=0;
		for(int i=0; i<num_beaches-1; i++)
		{
		/*			g_career->GetLongestTubeRide(i, tube);
		g_career->GetLongestFloater(i, floater);
		g_career->GetLongestAir(i, air);
		//			g_career->GetBestTrick(i, trick, t_score);
		g_career->GetBestScore(i, score);
		if(tube > max_tube) max_tube = tube;
		if(floater > max_float) max_float = floater;
		if(air > max_air) max_air = air;
		if(score > max_score) max_score = score;
		//			if(t_score > max_t_score) max_t_score = t_score;
		*/
		}
		ts_tube->changeText(stringx(max_tube));
		ts_float->changeText(stringx(max_float));
		ts_air->changeText(stringx(max_air));
		ts_score->changeText(stringx(max_score));
		//		ts_trick->changeText(stringx(max_t_score));
	}
}

void StatsFrontEnd::Draw()
{
	ts_tube->Draw();
	ts_float->Draw();
	ts_air->Draw();
	ts_score->Draw();

	if(!pq_name[cur_surfer])
		extras[current_extra_index].Draw();
	if(!pq_beach[cur_beach])
		extras_b[current_extra_index_b].Draw();

//	ts_trick->Draw();
	FEMultiMenu::Draw();
}

void StatsFrontEnd::OnActivate()
{
//	manager->em->ToOtherSetState();
	cur_surfer = manager->current_surfer;
	cur_beach = BeachOverallEntry;
	for(int i=0; i<SURFER_LAST; i++)
		if(pq_name[i]) pq_name[i]->TurnOn(cur_surfer == i);


	if(!pq_name[cur_surfer])
	{
		for(int i=0; i<num_extras; i++)
			if(extra_indices[i] == cur_surfer)
			{
				current_extra_index = i;
				break;
			}
	}

	OnSwitchBeach();
	FEMultiMenu::OnActivate();
}

void StatsFrontEnd::OnTriangle(int c)
{
//	manager->em->ExitState();
	sys->MakeActive(GraphicalMenuSystem::SurferMenu, 2);
}

void StatsFrontEnd::OnLeft(int c)
{
	if(cur_beach == 0) cur_beach = num_beaches-1;
	else cur_beach--;
	OnSwitchBeach();
	FEMultiMenu::OnLeft(c);
}

void StatsFrontEnd::OnRight(int c)
{
	if(cur_beach == num_beaches-1) cur_beach = 0;
	else cur_beach++;
	OnSwitchBeach();
	FEMultiMenu::OnRight(c);
}


#endif // 0

