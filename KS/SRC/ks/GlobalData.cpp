	
#include "Cheat.h"
#include "GlobalData.h"

GlobalDataClass globalCareerData;
GlobalDataClass::GlobalDataClass()
{
	int i, j;

	bailsMovieUnlocked = false;
	espnMovieUnlocked = false;

	for (i=0; i < MAP_LOC_LAST; i++)
	{
		globalLocations[i].boardUnlocked = false;
		globalLocations[i].movieShown = false;
		globalLocations[i].movieUnlocked = false;
	}

	for (i=0; i < SURFER_LAST; i++)
	{
		globalSurfers[i].boardsUnlocked[0] = true;	
		for (j=1; j < MAX_BOARDS; j++)
		{
			globalSurfers[i].boardsUnlocked[j] = false;
		}
		globalSurfers[i].personalityUnlocked = false;
		globalSurfers[i].unlocked = false;
		globalSurfers[i].movieUnlocked = false;
		globalSurfers[i].finished_last_level = false;
	}

	HighScoreData temp;
	temp.initials[0] = '\0';
	temp.character[0] = '\0';
	temp.score = 0;
	temp.icons = 0;
	for (i=0; i < BEACH_LAST; i++)
	{
		for (j = 0; j < HighScoreFrontEnd::NUM_ROWS; j++)
		{
			globalBeaches[i].high_scores[j] = temp;
			globalBeaches[i].high_icons[j] = temp;
		}

		globalBeaches[i].unlocked = false;
	}
	for (i=0; i < CHEAT_LAST; i++)
	{
		globalCheats[i].setLockedState(true);
		globalCheats[i].turnOn(false);
	}


}
void GlobalDataClass::init()
{
	int j,k;
	bool level_available = true;
	// determine level availability, etc.
	for(j = 0; j < LEVEL_LAST; j++)
	{
		level_available = true;
		for(k = 0; k < MAX_PREV_LEVELS; k++)
			if(CareerDataArray[j].prev_levels[k] != -1)
				level_available = false;
			
			// If there were no previous levels then unlock this one
			if(level_available)
			{
				this->unlockBeach(CareerDataArray[j].beach);
			}
	}
}
int GlobalDataClass::getMaxHandicap(int SurferIdx)
{
	return globalSurfers[SurferIdx].handicap;
}

void GlobalDataClass::setMaxHandicap(int SurferIdx, int hcap)
{
	globalSurfers[SurferIdx].handicap = hcap > MAX_HANDICAP ? MAX_HANDICAP : hcap;
}

// All surfer related ones
bool GlobalDataClass::isSurferUnlocked(int surfer) const
{ 
	assert(surfer < SURFER_LAST); 
	return globalSurfers[surfer].unlocked; 
}
void GlobalDataClass::unlockSurfer(int surfer)
{ 
	assert(surfer < SURFER_LAST); 
	globalSurfers[surfer].unlocked = true; 
}

bool GlobalDataClass::isSurferPersUnlocked(int surfer) const
{ 
	assert(surfer < SURFER_LAST); 
	return (globalSurfers[surfer].personalityUnlocked || g_session_cheats[CHEAT_ALL_PERSONALITY_SUITS].isOn());
}


void GlobalDataClass::unlockSurferPers(int surfer)		
{ 
	assert(surfer < SURFER_LAST); 
	globalSurfers[surfer].personalityUnlocked = true; 

}

bool GlobalDataClass::isSurferBoardUnlocked(int surfer, int board) const
{ 
	assert(surfer < SURFER_LAST); 
	return globalSurfers[surfer].boardsUnlocked[board]; 
}

void GlobalDataClass::unlockSurferBoard(int surfer, int board)
{ 
	assert(surfer < SURFER_LAST); 
	globalSurfers[surfer].boardsUnlocked[board] = true; 
}

bool GlobalDataClass::isSurferTrickUnlocked(int surfer, int trick) const
{ 
	assert(surfer < SURFER_LAST); 
	return globalSurfers[surfer].tricksUnlocked[trick]; 
}

void GlobalDataClass::unlockSurferTrick(int surfer, int trick)
{ 
	assert(surfer < SURFER_LAST); 
	globalSurfers[surfer].tricksUnlocked[trick] = true; 
}

bool GlobalDataClass::isSurferMovieUnlocked(int surfer) const
{ 
	assert(surfer < SURFER_LAST); 
	return globalSurfers[surfer].movieUnlocked; 
}

void GlobalDataClass::unlockSurferMovie(int surfer)
{ 
	assert(surfer < SURFER_LAST); 
	globalSurfers[surfer].movieUnlocked = true; 
}

// All beach related ones
bool GlobalDataClass::isBeachUnlocked(int beach) const
{ 
	assert(beach < BEACH_LAST); return globalBeaches[beach].unlocked; 
}
void GlobalDataClass::unlockBeach(int beach)				
{ 
	assert(beach < BEACH_LAST); globalBeaches[beach].unlocked = true; 
}

bool GlobalDataClass::isBeachBoardUnlocked(int beach) const
{ 
	assert(beach < BEACH_LAST); return globalLocations[BeachDataArray[beach].map_location].boardUnlocked; 
}

void GlobalDataClass::unlockBeachBoard(int beach)				
{ 
	assert(beach < BEACH_LAST); globalLocations[BeachDataArray[beach].map_location].boardUnlocked = true; 
}

bool GlobalDataClass::isLocationBoardUnlocked(int location) const
{ 
	assert(location < MAP_LOC_LAST); return globalLocations[location].boardUnlocked; 
}

void GlobalDataClass::unlockLocationBoard(int location)				
{ 
	assert(location < MAP_LOC_LAST); globalLocations[location].boardUnlocked = true; 
}

bool GlobalDataClass::isCheatUnlocked(int whichCheat) const
{
	assert(whichCheat < CHEAT_LAST);
	return !globalCheats[whichCheat].getLockedState() || !g_session_cheats[whichCheat].getLockedState();
}

void GlobalDataClass::unlockCheat(int whichCheat)
{
	assert(whichCheat < CHEAT_LAST);
	globalCheats[whichCheat].setLockedState(false);
	g_session_cheats[whichCheat].setLockedState(false);
}

HighScoreData GlobalDataClass::getBeachHighScore(int beach, int whichScore, bool icons) 
{ 
	assert(beach < BEACH_LAST);
	if(icons)
		return globalBeaches[beach].high_icons[whichScore]; 
	else return globalBeaches[beach].high_scores[whichScore]; 
} 

void GlobalDataClass::setBeachHighScore(int beach, int whichScore, bool icons, HighScoreData newScore) 
{ 
	assert(beach < BEACH_LAST);
	if(icons)
		globalBeaches[beach].high_icons[whichScore] = newScore; 
	else globalBeaches[beach].high_scores[whichScore] = newScore; 
}
bool GlobalDataClass::updateFromCareer(Career *c)
{
	int i;
	bool changed =false;
	if (c->GetSurferIdx() == SURFER_LAST)
		return false;
	for (i=0; i < BEACH_LAST; i++)
	{
		if (c->beaches[i].IsUnlocked())
		{
			unlockBeach(CareerDataArray[i].beach);		
			changed = true;
		}
	}
	globalSurfers[c->GetSurferIdx()].unlocked = true;
	
	for (i=0; i < MAX_BOARDS; i++)
	{
		if (c->IsBoardUnlocked(i))
		{
			globalSurfers[c->GetSurferIdx()].boardsUnlocked[i] = true;
			changed = true;
		}
	}

	for (i=0; i < MAP_LOC_LAST; i++)
	{
		if (c->locations[i].IsBoardUnlocked())
		{
			globalLocations[i].boardUnlocked = true;
			changed = true;
		}
		if (c->locations[i].IsMovieUnlocked()) 
		{
			changed = true;
			globalLocations[i].movieUnlocked = true;
		}
		if (c->locations[i].IsMovieShown())
		{
			changed = true;
			globalLocations[i].movieUnlocked = true;
		}
	}
	return changed;
}
void GlobalDataClass::unlockEverything()
{
	int i, j;
	for(i = 0; i < LEVEL_LAST; i++)
		if (g_career->IsStarted())
			g_career->levels[i].Unlock();
	for(i = 0; i < BEACH_LAST; i++)
	{
		if (g_career->IsStarted())
		{
			g_career->beaches[i].Unlock();
			g_career->beaches[i].Show();
		}
		unlockBeach(i);
	}
	for(i = 0; i < MAX_BOARDS; i++)
	{
		if (g_career->IsStarted())
		{
			g_career->UnlockBoard(i);
		}
		for(j = 0; j < SURFER_LAST; j++)
			unlockSurferBoard(i, j);
	}
	for(i = 0; i < SURFER_LAST; i++)
	{
		setMaxHandicap(i, MAX_HANDICAP);
		if(SurferDataArray[i].selectable)
		{
			unlockSurfer(i);
			unlockSurferPers(i);
		}
		if(SurferDataArray[i].initially_unlocked) // Only the normal surfers have a movie associated with them
		{
			globalCareerData.unlockSurferMovie(i);
		}
	}

	for(i = 0; i < CHEAT_LAST; i++)
	{
		unlockCheat(i);
	}

	for(j = 0; j < MAP_LOC_LAST; j++)
	{
		if (g_career->IsStarted())
		{
			g_career->locations[j].UnlockBoard();
			g_career->locations[j].UnlockMovie();
		}
		unlockBeachBoard(j);
		unlockLocationMovie(j);
	}

	// unlock tricks
	for(i = 0; i < (NUM_LEARNED_TRICKS_PER_SURFER + NUM_EARNED_TRICKS_PER_SURFER); i++)
	{
		if(frontendmanager.tmp_game_mode == GAME_MODE_CAREER && g_career->GetSurferIdx() != SURFER_LAST)
			g_career->UnlockTrick(i);

		for(j = 0; j < SURFER_LAST; j++)
			globalCareerData.unlockSurferTrick(j, i);
	}

	// Unlock misc movies
	globalCareerData.unlockBailsMovie();
	globalCareerData.unlockEspnMovie();
	
	
}
