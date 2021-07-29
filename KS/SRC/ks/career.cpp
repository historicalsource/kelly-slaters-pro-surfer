// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "career.h"
#include "GlobalData.h"
// Global career variable.
Career	*g_career=NULL;

#define SUPER_LEVEL_BOARD MAP_LOC_ANTARCTICA

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Career class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CAREER_StaticInit( void )
{
	g_career=NEW Career();
}

//	Career()
// Default constructor.
Career::Career()
{
	int i;
	totalStatPointsIncreased=0;
	initKSConfigStruct(&cfg);
	// set all the surfer boards to be locked
	for(i = 0; i < MAX_BOARDS; i++)
		boards[i] = false;
	spin = speed = jump = balance = 0;
	my_id = SURFER_LAST;
	strcpy(myInitials, "");
}

void Career::init()
{
	int i, j;
	//bool level_available;

	for(j = 0; j < LEVEL_LAST; j++)
		levels[j].SetMyId(j);
	for(j = 0; j < BEACH_LAST; j++)
		beaches[j].SetMyId(j);
	for(j = 0; j < MAP_LOC_LAST; j++)
		locations[j].SetMyId(j);
	
	for (j=0; j < MAX_GOALS_PER_LEVEL; j++)
	{
		new_goal_passed[j] = false;
		goal_passed[j] = false;
		awarded[j] = false;
	}
	SetMyId(SURFER_KELLY_SLATER);
	StartNewCareer(); // we do this just to make sure we don't inadvertently use weird data

	cfg = *(ksConfigData *)StoredConfigData::inst()->getGameConfig();

	// count up the environmental challenges
	total_env_challenges = 0;
	env_challenges_passed = 0;
	super_board_was_unlocked = -1;
	bails_movie_was_unlocked = -1;
	espn_movie_was_unlocked  = -1;

	for(i = 0; i < LEVEL_LAST; i++)
	{
		levels_completed_order[i] = -1;

		for(j = 0; j < MAX_GOALS_PER_LEVEL; j++)
			// if it's an environmental challenge
			if(CareerDataArray[i].goal[j] >= GOAL_ENV_SPRAY_WINDSURFERS && 
			   CareerDataArray[i].goal[j] <= GOAL_ENV_JUMP_PIER)
			{
				total_env_challenges++;
			}
	}
}

void Career::SetMyId(int id)
{
	int j;
	my_id		= id;
	spin    = SurferDataArray[my_id].attr_spin;
	speed   = SurferDataArray[my_id].attr_speed;
	jump    = SurferDataArray[my_id].attr_air;
	balance = SurferDataArray[my_id].attr_balance;
	UnlockBoard(0);
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD))
		for(j = 2; j < MAX_BOARDS; j++)
			UnlockBoard(j);

}

int Career::GetSurferIdx()
{
	return my_id;
}
void Career::StartNewCareer()
{
	int i, j, k;
	bool level_available;

	// So we know that it hasn't been set yet
	my_id = SURFER_LAST;
	my_board_id = 0;

	SetCurrentBeach(BEACH_INDOOR);


	for(i = 0; i < LEVEL_LAST; i++)
	{
		levels[i].SetMyId(i);
		levels[i].Reset();
	}
	for(i = 0; i < BEACH_LAST; i++)
	{
		beaches[i].SetMyId(i);
		//beaches[i].Reset();
	}
	for(i = 0; i < MAP_LOC_LAST; i++)
	{
		locations[i].SetMyId(i);
		//locations[i].Reset();
	}
	for(i = 0; i < MAX_BOARDS; i++)
	{
		boards[i] = false;
	}

	for (i = 0; i < MAX_PHOTOS; i++)
	{
		compressedPhotos[i].Reset ();
	}

	for(i = 0; i < MAP_LOC_LAST; i++)
	{
		locations[i].Reset();
	}

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
				levels[j].Unlock(); // unlock the level
				beaches[CareerDataArray[j].beach].Unlock(); // and its associated beach
			}
	}

	num_special_challenges_completed = 0;
	num_earned_tricks = 0;
	using_personality_suit = false;
	for(i = 0; i < (NUM_LEARNED_TRICKS_PER_SURFER + NUM_EARNED_TRICKS_PER_SURFER); i++)
		learned_tricks[i] = false;

	// Clear out the environmental challenge counter
	env_challenges_passed = 0;
}

void Career::ClearAllGoals()
{
	int i;	//, j, k;

	for(i = 0; i < LEVEL_LAST; i++)
	{
		levels[i].ResetGoals();
	}
}

void Career::IncreaseSpin(int inc)
{
	spin    += inc; 
	totalStatPointsIncreased+= inc;
	if (globalCareerData.getMaxHandicap(my_id) < totalStatPointsIncreased / 4)
		globalCareerData.setMaxHandicap(my_id, totalStatPointsIncreased / 4);
}

void Career::IncreaseSpeed(int inc)
{
	speed   += inc; 
	totalStatPointsIncreased+= inc;
	if (globalCareerData.getMaxHandicap(my_id) < totalStatPointsIncreased / 4)
		globalCareerData.setMaxHandicap(my_id,totalStatPointsIncreased / 4);
}
stringx Career::GetInitials()
{
	return stringx(myInitials);
}
void Career::SetInitials(stringx initials)
{
	assert(initials.length() <= 3);
	strcpy(myInitials, initials.c_str());
}

void Career::IncreaseJump(int inc)
{ 
	jump    += inc; 
	totalStatPointsIncreased+= inc;
	if (globalCareerData.getMaxHandicap(my_id) < totalStatPointsIncreased / 4)
		globalCareerData.setMaxHandicap(my_id,totalStatPointsIncreased / 4);
}

void Career::IncreaseBalance(int inc)
{ 
	balance += inc; 
	totalStatPointsIncreased+= inc;
	if (globalCareerData.getMaxHandicap(my_id) < totalStatPointsIncreased / 4)
		globalCareerData.setMaxHandicap(my_id,totalStatPointsIncreased / 4);
}


bool Career::IsTrickOpen(int trick_num, bool ignore_current_level)
{
	int i;
	int current_surfer = g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player());

	// Check through all the tricks in this surfer's trickbook.  If the trick isn't there, return false
	bool trick_exists = false;
	for(i = 0; i < TRICKBOOK_SIZE; i++)
	{
		if(SurferDataArray[current_surfer].trickBook[i] == trick_num)
			trick_exists = true;
	}
	if(!trick_exists)
		return false;

	// Check through all the tricks in the learned trickbook.  
	// For each: if it's been unlocked and it's the trick in question, return true.
	// If none of them are the trick in question, return true (since that means that 
	// this trick doesn't have to be learned).
	trick_exists = false;
	for(i = 0; i < (NUM_EARNED_TRICKS_PER_SURFER + NUM_LEARNED_TRICKS_PER_SURFER); i++)
	{
		if(trick_num == SurferDataArray[current_surfer].learnedtrickBook[i])
		{
			trick_exists = true;
			if(learned_tricks[i])
				return true;
		}
	}
	if(!trick_exists)
		// the trick wasn't in the learned tricks list, so it is available
		return true;

	// this code will fail (and rightly so) inside the frontend -beth
	if(FEDone())
	{
		// Otherwise, return true only if we're on a level where that trick is the goal_param for a LEARN_NEW_TRICK goal
		int current_level = g_game_ptr->get_level_id();
 		if (!ignore_current_level && 
			CareerDataArray[current_level].goal[0] == GOAL_LEARN_NEW_TRICK && // Are we on a learn_new_trick beach?
			g_game_ptr->get_game_mode() == GAME_MODE_CAREER &&                // Are we in career mode?
			trick_num == SurferDataArray[current_surfer].learnedtrickBook[CareerDataArray[current_level].goal_param[0]]) // is this the right trick?
			return true;  // it's available for the surfer to learn the trick
	}

	// Otherwise the trick is unavailable
	return false;
}

void Career::StartNewRun()
{
	int j;
	for (j=0; j < MAX_GOALS_PER_LEVEL; j++)
	{
		new_goal_passed[j] = false;
		goal_passed[j] = false;
		awarded[j] = false;
	}
	super_board_was_unlocked = -1;
	bails_movie_was_unlocked = -1;
	espn_movie_was_unlocked  = -1;
}
void Career::OnGoalReDone(const int levelIdx, const int goalIdx)
{
	goal_passed[goalIdx] = true;
}

void Career::Location::UnlockBoard()   
{ 
	globalCareerData.unlockLocationBoard(my_id);
	board_unlocked = true; 
}

void Career::Location::UnlockMovie()   
{ 
	movie_unlocked = true; 
}

void Career::Location::SetMovieShown() 
{
	movie_shown    = true; 
}

CompressedPhoto* Career::GetPhotoForLevel (int level)
{
	int index = 0;

	for (int i = 0; i < LEVEL_LAST; i++)
	{	
		if (CareerDataArray[i].goal[0] == GOAL_PHOTO_1 || CareerDataArray[i].goal[0] == GOAL_PHOTO_2 || CareerDataArray[i].goal[0] == GOAL_PHOTO_3)
		{
			index++;

			if (level == i)
				return &compressedPhotos[index];
		}
	}

	return NULL;
}

bool Career::PhotoExistsForLevel (int level)
{
	CompressedPhoto* photo;

	photo = GetPhotoForLevel (level);

	return (photo != NULL) && (photo->IsValid ());
}

void Career::SavePhotoForLevel (CompressedPhoto* new_photo, int level)
{
	CompressedPhoto* photo;

	photo = GetPhotoForLevel (level);

	if (photo != NULL)
		*photo = *new_photo;
}

void Career::OnGoalDone(const int levelIdx, const int goalIdx)
{
	if (g_game_ptr) 
	{
		g_game_ptr->on_goal_completed();
	}

	// take note of the fact that a goal has been passed
	goal_passed[goalIdx] = true;
	new_goal_passed[goalIdx] = true;

	// if it was a main goal, make note in the level order array
	if(goalIdx == 0)
	{
		for(int i=0; i<LEVEL_LAST; i++)
			if(levels_completed_order[i] == -1)
			{
				levels_completed_order[i] = levelIdx;
				break;
			}
	}

	// if it's an environmental challenge, check to see if all 
	// the environmental challenges have been passed
	if(CareerDataArray[levelIdx].goal[goalIdx] >= GOAL_ENV_SPRAY_WINDSURFERS && 
	   CareerDataArray[levelIdx].goal[goalIdx] <= GOAL_ENV_JUMP_PIER)
	{
		env_challenges_passed++;
		if(total_env_challenges == env_challenges_passed)
		{
			bails_movie_was_unlocked = goalIdx;
			if(!globalCareerData.isBailsMovieUnlocked())
				awarded[goalIdx] = true;
			globalCareerData.unlockBailsMovie();
		}
	}

	// Check to see if all the goals have been passed everywhere
	bool all_goals_done = true;
	for(int lev_num = 0; lev_num < LEVEL_LAST && all_goals_done; lev_num++)
	{
		for(int goal_num = 0; goal_num < MAX_GOALS_PER_LEVEL; goal_num++)
		{
			if(CareerDataArray[lev_num].goal[goal_num] != GOAL_NOTHING && !levels[lev_num].IsGoalDone(goal_num))
				all_goals_done = false;
		}
	}

	// Dole out the rewards

	if(CareerDataArray[levelIdx].goal[goalIdx] == GOAL_LEARN_NEW_TRICK)
		UnlockTrick(CareerDataArray[levelIdx].goal_param[goalIdx]);

	// if this is the first goal item, unlock the next level(s) if all the prerequisites have been fulfilled
	if(goalIdx == 0)
		CheckUnlockNextLevels(levelIdx);

	// If all goals everywhere have been passed then award the super board
	if(all_goals_done && !locations[SUPER_LEVEL_BOARD].IsBoardUnlocked())
	{
		super_board_was_unlocked = goalIdx;
		locations[SUPER_LEVEL_BOARD].UnlockBoard();
	}

	// Check if all the surfers have passed the final Cosmos challenge
	if(levelIdx == LEVEL_LAST - 1 && !globalCareerData.isEspnMovieUnlocked())
	{
		bool all_passed = true;
		for(int srfr = 0; srfr < SURFER_LAST; srfr++)
		{
			// If it's an initially-selectable surfer and he/she hasn't finished the goal of the last level
			if(SurferDataArray[srfr].initially_unlocked && !globalCareerData.isLastLevelCompleted(srfr))
				// then clearly all of them haven't completed the last level
				all_passed = false;
		}
		if(all_passed)
		{
			espn_movie_was_unlocked = goalIdx;
			globalCareerData.unlockEspnMovie();
		}
	}

	ScoringManager &score_keeper = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager();

	switch(CareerDataArray[levelIdx].reward[goalIdx])
	{
	case REWARD_NOTHING:
		break; // do nothing
		
	case REWARD_LEVEL_BOARD:
		// Unlock the board for this level
		awarded[goalIdx] = true;
		locations[BeachDataArray[CareerDataArray[levelIdx].beach].map_location].UnlockBoard();
		break;
		
	case REWARD_PERSONALITY_SUIT:
		awarded[goalIdx] = true;
		// Unlock the personality suit
		globalCareerData.unlockSurferPers(my_id);
		break;
		
	case REWARD_POINTS:
		awarded[goalIdx] = true;
		score_keeper.AddPoints(CareerDataArray[levelIdx].reward_param[goalIdx]);
		break;
		
	case REWARD_SURFER_BOARD:
		awarded[goalIdx] = true;

		int board_to_unlock;
		board_to_unlock = 0;

		// figure out what the next board we want to unlock is
		while(IsBoardUnlocked(board_to_unlock) && board_to_unlock < MAX_BOARDS - 1)
			board_to_unlock++;
		// and unlock it
		UnlockBoard(board_to_unlock);
		break;
	
	case REWARD_JUMP_POINTS:
		awarded[goalIdx] = true;
		IncreaseJump(CareerDataArray[levelIdx].reward_param[goalIdx]);
		break;
	case REWARD_SPEED_POINTS:
		awarded[goalIdx] = true;
		IncreaseSpeed(CareerDataArray[levelIdx].reward_param[goalIdx]);
		break;
	case REWARD_SPIN_POINTS:
		awarded[goalIdx] = true;
		IncreaseSpin(CareerDataArray[levelIdx].reward_param[goalIdx]);
		break;
	case REWARD_BALANCE_POINTS:
		awarded[goalIdx] = true;
		IncreaseBalance(CareerDataArray[levelIdx].reward_param[goalIdx]);
		break;
	case REWARD_CHEAT:
		// if the cheat wasn't already unlocked
		if(!globalCareerData.isCheatUnlocked(CareerDataArray[levelIdx].reward_param[goalIdx]))
		{
			awarded[goalIdx] = true;
			globalCareerData.unlockCheat(CareerDataArray[levelIdx].reward_param[goalIdx]);
		}
		break;
		
	case REWARD_SURFER:
		if(!globalCareerData.isSurferUnlocked(CareerDataArray[levelIdx].reward_param[goalIdx]))
		{
			awarded[goalIdx] = true;
			// unlock the appropriate surfer
			globalCareerData.unlockSurfer(CareerDataArray[levelIdx].reward_param[goalIdx]);
		}
		break;
		
	case REWARD_LEVEL_BOARD_AND_SURFER:
		// DAJ 7/8/02: This reward has been changed to be "appease the 
		// Tiki God" and now unlocks all the level boards, not just one.

		if(!globalCareerData.isSurferUnlocked(CareerDataArray[levelIdx].reward_param[goalIdx]))
		{
			awarded[goalIdx] = true;
			// Unlock the level boards
			for(int i = 0; i < MAP_LOC_LAST; i++)
			{
				if(i != SUPER_LEVEL_BOARD)
					locations[i].UnlockBoard();
			}
			// unlock the appropriate surfer
			globalCareerData.unlockSurfer(CareerDataArray[levelIdx].reward_param[goalIdx]);
		}
		break;

	case REWARD_SURFER_MOVIE:
		if(globalCareerData.isSurferMovieUnlocked(my_id))
		{
			awarded[goalIdx] = true;
			globalCareerData.unlockSurferMovie(my_id);
		}
		break;

	case REWARD_SPECIAL_TRICK:
		num_special_challenges_completed++;
		if(!(num_special_challenges_completed & 1)) // if an even number of special meter challenges have been passed
		{
			awarded[goalIdx] = true;
			EarnNextTrick();
		}
	}
}
// CheckUnlockNextLevels()
// Checks to see if the next levels can be unlocked.  If so, it unlocks them.

void Career::CheckUnlockNextLevels(const int levelIdx)
{
	int i, j;
	int next_level, prev_level;
	//int level_index;
	bool level_open;
	
	// For every level that comes after this one, check to see if _that_ 
	// level's prerequisites have been completed.  If so, unlock that level.
	for(i = 0; i < MAX_NEXT_LEVELS; i++)
	{
		next_level = CareerDataArray[levelIdx].next_levels[i];
		level_open = true;
		if(next_level != -1)
		{
			// Check whether all this level's previous entries have had their advance goal completed
			for(j = 0; j < MAX_PREV_LEVELS; j++)
			{
				prev_level = CareerDataArray[next_level].prev_levels[j];
				if(prev_level != -1 && !IsAdvanceGoalDone(prev_level))
					level_open = false;
			}
			// If all the previous levels have been completed then unlock this level
			if(level_open)
				levels[next_level].Unlock();
		}
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Career::Location class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Career::Location::Location()
{
	Reset();
}

void Career::Location::Reset()
{
	movie_unlocked = false;
	movie_shown = false;
	board_unlocked = false;
}

// CheckShowBeachMovie()
// Checks if the movie has been shown for the given beach.  If it hasn't, 
// the function sets the flag as shown and returns true (indicating that 
// the movie _should_ be shown now).  Otherwise it returns false.

bool Career::Location::CheckShowMovie()
{
	if(IsMovieShown())
		return false;
	else
	{
		SetMovieShown();
		return true;
	}
}


bool Career::WasNewGoalPassed(int goalIdx)
{
	int i;
	assert(goalIdx < MAX_GOALS_PER_LEVEL);

	if(goalIdx < 0) // check all the goals
	{
		for(i = 0; i < MAX_GOALS_PER_LEVEL; i++)
			if(new_goal_passed[i])
				return true;
		return false;
	}
	return new_goal_passed[goalIdx];
}

bool Career::WasAnyGoalPassed(int goalIdx)
{
	int i;
	assert(goalIdx < MAX_GOALS_PER_LEVEL);

	if(goalIdx < 0) // check all the goals
	{
		for(i = 0; i < MAX_GOALS_PER_LEVEL; i++)
			if(goal_passed[i])
				return true;
		return false;
	}
	else
		return goal_passed[goalIdx];
}

int Career::WasNewLevelUnlocked()
{
	int i, num_new_levels = 0;

	if(WasNewGoalPassed(0)) // if the first goal was passed
	{
		for(i = 0; i < MAX_NEXT_LEVELS; i++)
		{
			if(CareerDataArray[g_game_ptr->get_level_id()].next_levels[i] != -1 &&
			   levels[CareerDataArray[g_game_ptr->get_level_id()].next_levels[i]].IsUnlocked())
				num_new_levels++;
		}
	}
	return num_new_levels;
}

int Career::WasNewBeachUnlocked() // ******* DONE BUT NEEDS TO BE TESTED
{
	int i, j;
	int num_new_beaches = 0; // the number of new beaches that have been unlocked
	int beach_to_check;      // which beach the unlocked level is on.  Check for this beach in 
	                         // all the previously unlocked levels
	int current_level = g_game_ptr->get_level_id();

	if(WasNewLevelUnlocked()) // if a new level was unlocked
	{
		for(i = 0; i < MAX_NEXT_LEVELS; i++)
		{
			if(CareerDataArray[current_level].next_levels[i] != -1 &&
			   levels[CareerDataArray[current_level].next_levels[i]].IsUnlocked())
			{
				// which beach does this level use?
				beach_to_check = CareerDataArray[CareerDataArray[current_level].next_levels[i]].beach;
				// now check every other level.  If every unlocked level's beach is different than 
				// this one then this beach must have just been unlocked (since the level was unlocked).
				for(j = 0; j < LEVEL_LAST; j++)
					if(j != CareerDataArray[current_level].next_levels[i] &&
					   levels[j].IsUnlocked()  &&
					   CareerDataArray[j].beach == beach_to_check)
						break;  // if the beaches are the same then break
				// Now if j < LEVEL_LAST then there must have been a break, so the beach isn't a new one.
				if(j == LEVEL_LAST)
					num_new_beaches++;
			}
		}
	}
	return num_new_beaches;
}

bool Career::GetGoalText(int level_num, int goal_num, stringx &goal_text)
{
	assert(level_num >= 0 && level_num < LEVEL_LAST);
	assert(goal_num >= 0 && goal_num < (int) countof(CareerDataArray[level_num].goal));
	int gt_goal_index = GT_GOAL_NOTHING + CareerDataArray[level_num].goal[goal_num];
	bool use_param2 = false;

	// First figure out if this is a competition.  If so, and the 
	// surfer isn't a competitor, treat this goal as a photo shoot.
	if(gt_goal_index >= GT_GOAL_COMPETITION_1 && 
	   gt_goal_index <= GT_GOAL_COMPETITION_3 &&
	   !SurferDataArray[g_game_ptr->GetSurferIdx(0)].competitor)
	{
		// If you have to get 1st place in the competition then make it a photo 3
		// If you have to get 2nd place in the competition then make it a photo 2
		// If you have to get 3rd place in the competition then make it a photo 1
		gt_goal_index = GT_GOAL_PHOTO_3 - (gt_goal_index - GT_GOAL_COMPETITION_1);
		use_param2 = true;
	}
	
	// if the goal text needs a single integer parameter
	// else if it needs two integer parameters
	if(gt_goal_index == GT_GOAL_ICON_TETRIS || gt_goal_index == GT_GOAL_LONGEST_RIDE ||
     gt_goal_index == GT_GOAL_PHOTO_1 || gt_goal_index == GT_GOAL_PHOTO_2 || gt_goal_index == GT_GOAL_PHOTO_3)
	{
		goal_text.printf(ksGlobalTextArray[gt_goal_index].c_str(), 
			CareerDataArray[level_num].goal_param[goal_num],    // # of seconds
			CareerDataArray[level_num].goal_param_2[goal_num]); // # of points
	}
	else if((gt_goal_index >= GT_GOAL_SPECIAL_METER && gt_goal_index <= GT_GOAL_ENV_SPRAY_RAFTERS))
	{
		int param;
		if(use_param2) 
			param = CareerDataArray[level_num].goal_param_2[goal_num];
		else
			param = CareerDataArray[level_num].goal_param[goal_num];

		goal_text.printf(ksGlobalTextArray[gt_goal_index].c_str(), param);
	}
	// else if it needs a trick name inserted in the string
	else if(gt_goal_index == GT_GOAL_LEARN_NEW_TRICK)
	{
		//  Check to see if the trick starts with a vowel (for English).  If so, then use the test that says "an" trick instead of "a" trick.
		char first_letter = (ksGlobalTrickTextArray[SurferDataArray[g_game_ptr->GetSurferIdx(0)].learnedtrickBook[CareerDataArray[level_num].goal_param[goal_num]]].c_str())[0];
		if (ksGlobalTextLanguage == LANGUAGE_ENGLISH &&
			(first_letter == 'A' || first_letter == 'E' || first_letter == 'I' || first_letter == 'O' || first_letter == 'U'))
			goal_text.printf(ksGlobalTextArray[GT_GOAL_LEARN_NEW_TRICK_VOWEL].c_str(),
				ksGlobalTrickTextArray[SurferDataArray[g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player())].learnedtrickBook[CareerDataArray[level_num].goal_param[goal_num]]].c_str(),
				CareerDataArray[level_num].goal_param_2[goal_num]);
		else
			goal_text.printf(ksGlobalTextArray[gt_goal_index].c_str(), 
				ksGlobalTrickTextArray[SurferDataArray[g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player())].learnedtrickBook[CareerDataArray[level_num].goal_param[goal_num]]].c_str(),
				CareerDataArray[level_num].goal_param_2[goal_num]);
	}
	else if (gt_goal_index >= GT_GOAL_SKILL_FACE_SCORE &&
		gt_goal_index <= GT_GOAL_SKILL_540_SPIN_SCORE)
	{
		goal_text.printf(ksGlobalTextArray[gt_goal_index].c_str(), 
			CareerDataArray[level_num].goal_param_2[goal_num],    // # of spins
			CareerDataArray[level_num].goal_param[goal_num]); // # of points
	}
/*
	// else if it's "SPRAY TUBERS AND RAFTERS" and needs the same thing inserted twice
	else if(gt_goal_index == GT_GOAL_ENV_SPRAY_TUBERS_RAFTERS)
	{
		goal_text.printf(ksGlobalTextArray[gt_goal_index].c_str(), 
			CareerDataArray[level_num].goal_param[goal_num],  // # of tubers
			CareerDataArray[level_num].goal_param[goal_num]); // # of rafters
	}
*/
	// else it doesn't need any parameters
	else
	{
		goal_text.copy(ksGlobalTextArray[gt_goal_index]);
	}

	// return false if this goal is nothing
	return (gt_goal_index != GT_GOAL_NOTHING);
}

// This should only be called to inform the player of what has just been 
// unlocked -- either at the end of a run or when it happens.
bool Career::GetRewardText(int level_num, int goal_num, stringx &reward_text)
{
	int reward_idx = CareerDataArray[level_num].reward[goal_num];
	int gt_reward_index = GT_REWARD_NOTHING + reward_idx;
	bool retval = reward_idx != REWARD_NOTHING;

	// There are certain rewards that special characters can't win.
	if(!SurferDataArray[my_id].initially_unlocked &&
	   (reward_idx == REWARD_SURFER_BOARD     ||
	    reward_idx == REWARD_PERSONALITY_SUIT ||
		reward_idx == REWARD_SURFER_MOVIE     ||
		(reward_idx == REWARD_SURFER && CareerDataArray[level_num].reward_param[goal_num] == my_id)))
	{
		return false;
	}

	// Only go on if this goal actually recieved a reward on this run (so 
	// that we don't, for instance, print that a movie was unlocked twice)
	if(!g_career->WasAwarded(goal_num))
	{
		return false;
	}

	if(reward_idx > REWARD_NOTHING && reward_idx <= REWARD_BALANCE_POINTS)
		reward_text = ksGlobalTextArray[gt_reward_index];
	else if(reward_idx == REWARD_CHEAT)
	{
		reward_text.printf((ksGlobalTextArray[GT_CHEAT_RAINBOW + CareerDataArray[level_num].reward_param[goal_num]] + " " + ksGlobalTextArray[gt_reward_index]).c_str(),
							ksGlobalTextArray[GT_CHEAT_RAINBOW_NUMBER + CareerDataArray[level_num].reward_param[goal_num]].c_str());
	}
	else if(reward_idx >= REWARD_SURFER && reward_idx <= REWARD_SURFER_MOVIE)
	{
		stringx upper_case_name(SurferDataArray[CareerDataArray[level_num].reward_param[goal_num]].fullname);
		upper_case_name.to_upper();
		reward_text.printf(ksGlobalTextArray[gt_reward_index].c_str(), upper_case_name.c_str());
	}
	else if(reward_idx == REWARD_SPECIAL_TRICK)
	{
		// figure out if a new trick has been unlocked
		// One has iff an even number of REWARD_SPECIAL_TRICK rewards have been recieved
		// We know we must be in career mode if we're in this function, so just query the career struct directly.
		if(!(num_special_challenges_completed & 1)) // if it's even
			reward_text.printf(ksGlobalTextArray[gt_reward_index].c_str(), 
								ksGlobalTrickTextArray[SurferDataArray[g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player())].learnedtrickBook[num_earned_tricks + 1]].c_str()); 
		else
			retval = false;
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Career::Level class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Level()
// Default constructor.
Career::Level::Level()
{
	Reset();
}

void Career::Level::Reset()
{
	int i;

	unlocked = false;
	is_new = false;
	for(i = 0; i < MAX_GOALS_PER_LEVEL; i++)
		goals[i] = false;
}

void Career::Level::ResetGoals()
{
	int i;

	for(i = 0; i < MAX_GOALS_PER_LEVEL; i++)
		goals[i] = false;
}

//	Unlock()
// Unlocks this level.
void Career::Level::Unlock(void)
{
	unlocked = true;
	is_new = true;
	g_career->beaches[CareerDataArray[my_id].beach].Unlock();
}

bool Career::Level::IsGoalDone(const int goalIdx) const 
{ 
	assert(goalIdx >= 0 && goalIdx < MAX_GOALS_PER_LEVEL);

	return goals[goalIdx]; 
}

//	SetGoalDone()
// Sets the specifed goal as completd.
void Career::Level::SetGoalDone(const int goalIdx)
{
	assert(goalIdx >= 0 && goalIdx < MAX_GOALS_PER_LEVEL);
	
	// if goal hasn't been completed before, set unsaved_career data flag true
	if(!goals[goalIdx]) frontendmanager.unsaved_career = true;
	goals[goalIdx] = true;
	if(is_new) is_new = false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Career::Beach class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//	Beach()
// Default constructor.
Career::Beach::Beach()
{
	Reset();
}

void Career::Beach::Reset()
{
	unlocked = false;
	shown = false;
}

//	Unlock()
// Unlocks this beach.
void Career::Beach::Unlock(void)
{
	unlocked = true;
	// unlock the beach movie
	g_career->locations[BeachDataArray[my_id].map_location].UnlockMovie();
	Show();
	globalCareerData.unlockBeach(my_id);
}

//	UnlockBoard()
// Unlocks the surfer's specified board.
void Career::UnlockBoard(const int boardIdx)
{
	assert(boardIdx >= 0 && boardIdx < MAX_BOARDS);
	
	boards[boardIdx] = true;
	globalCareerData.unlockSurferBoard(my_id, boardIdx);
}

//	UnlockTrick()
// Unlocks the surfer's specified trick.
void Career::UnlockTrick(const int trickIdx)
{
	assert(trickIdx >= 0 && trickIdx < (NUM_LEARNED_TRICKS_PER_SURFER + NUM_EARNED_TRICKS_PER_SURFER));
	
	learned_tricks[trickIdx] = true;
	globalCareerData.unlockSurferTrick(my_id, trickIdx);
}

void Career::EarnNextTrick() 
{ 
	assert(num_earned_tricks < NUM_EARNED_TRICKS_PER_SURFER); 
	UnlockTrick(num_earned_tricks + NUM_LEARNED_TRICKS_PER_SURFER);
	num_earned_tricks++; 
}

//	IsBoardUnlocked()
// Returns true if this surfer's specified boars is unlocked.
bool Career::IsBoardUnlocked(const int boardIdx) const
{
	assert(boardIdx >= 0 && boardIdx < MAX_BOARDS);
	
	return boards[boardIdx];
}
