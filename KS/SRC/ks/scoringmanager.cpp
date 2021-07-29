
#include "global.h"
#include "scoringmanager.h"
#include "combodata.h"

// Default values for scoring manager configuration.
// These are overwritten when scoring.dat is loaded.
float ScoringManager::MOUTH_DISTANCES[2] = { 10.0f, 20.0f };
float ScoringManager::SCALE_LIP_DISTS[3] = { 1.2f, 1.0f, 0.8f };
float ScoringManager::SCALE_MOUTH_DISTS[3] = { 1.2f, 1.0f, 0.8f };
float ScoringManager::SCALE_SPINS[9] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
float ScoringManager::SCALE_LANDINGS[4] = { 2.0f, 1.0f, 0.5f, 0.2f};
float ScoringManager::SCALE_SERIES_MODS[2] = { 2.0f, 2.0f };
float ScoringManager::SCALE_REPEATS[5] = { 1.0f, 0.90f, 0.70f, 0.50f, 0.30f };
float ScoringManager::SCALE_REPEATS_FACE[5] = { 1.0f, 0.95f, 0.85f, 0.75f, 0.65f };
float ScoringManager::SCALE_WAVE[3] = { 1.2f, 1.0f, 0.8f };
int	  ScoringManager::METER_FILL_RATE = 7500;
int	  ScoringManager::METER_FILL_RATE_FACE = 3500;
float ScoringManager::METER_PENALTY_SLOPPY = 0.5f;
float ScoringManager::METER_PENALTY_JUNK = 0.9f;
float ScoringManager::SCALE_AT_TUBE = { 1.20 };
int   ScoringManager::CAP_COOL = 4;
int	  ScoringManager::CAP_LINK = 7;

//	ScoringManager()
// Default constructor.
ScoringManager::ScoringManager()
{
	ksctrl = NULL;
	playerIdx = -1;
	specialMeter = NULL;

	for (int i = 0; i < TRICK_NUM; i++)
		levelTricks[i].numLandings = 0;

	bestChainScore = 0;
	longestTubeRide = 0.0f;
	longestFloater = 0.0f;
	longestAir = 0.0f;
	score = 0;
	airPoints = 0;
	facePoints = 0;
	tubePoints = 0;
	specialTrick = SPECIALTRICK_NONE;
	chain.levelTricks = levelTricks;
	prevTrickRegion = TREGION_FACE;

	lastChainInfo.Reset();
}

//	~ScoringManger()
// Destructor.
ScoringManager::~ScoringManager()
{

}

#define MAX_SERIES_PER_CHAIN   32
#define MAX_TRICKS_PER_SERIES  64

// Called to preallocate memory for a bunch of STL stuff
void ScoringManager::stl_prealloc( void )
{
	SeriesList slist;
	Series series;
	for ( int i=0; i<MAX_SERIES_PER_CHAIN; i++ )
	{
		slist.push_back(series);
	}
	slist.resize(0);
	TrickList tlist;
	Trick trick;
	for ( int i=0; i<MAX_TRICKS_PER_SERIES; i++ )
	{
		tlist.push_back(trick);
	}
	tlist.resize(0);
}

//	SetKSCtrl()
// Sets this scoring manager's owner.
void ScoringManager::SetKsctrl(kellyslater_controller * ks)
{
	ksctrl = ks;
	playerIdx = ksctrl->get_player_num();
	specialMeter = ksctrl->get_special_meter();
}

//	Reset()
// Clears the trick chain and clears the trick count for every trick type.
// This method should be called whenever the level changes.
void ScoringManager::Reset(void)
{
	Series	series;
	
	for (int i = 0; i < TRICK_NUM; i++)
		levelTricks[i].numLandings = 0;

	bestChain.series.resize(0);
	bestChainScore = 0;
	longestTubeRide = 0.0f;
	longestFloater = 0.0f;
	longestAir = 0.0f;

	chain.series.resize(0);
	series.levelTricks = levelTricks;
	chain.series.push_back(series);

	score = 0;
	facePoints = 0;
	airPoints = 0;
	tubePoints = 0;
	num360spins = 0;
	num540spins = 0;

	//sickMeter = 0.0f;
	//sickZone = SICK_ZONE_LOW;

	specialTrick = SPECIALTRICK_NONE;
}

//	Update()
// Updates the scoring manager in response to time.
void ScoringManager::Update(const float dt)
{
	bool	finishChain = false;
	
	// Make it so that special trick state only lasts for 1 frame.
	if (specialTrick == SPECIALTRICK_SUCCEEDED || specialTrick == SPECIALTRICK_FAILED)
		specialTrick = SPECIALTRICK_NONE;
	
	// If player is still in the face trick region, face linking is required to maintain his chain.
	if (ksctrl->GetTrickRegion() == TREGION_FACE)
	{
		if (!specialMeter->CanFaceLink() && !specialMeter->CanRegionLink() &&
			chain.IsInteresting())
			finishChain = true;
	}

	// Players can end their chain prematurely by pressing R3.
	if (
#ifdef TARGET_GC
	    input_mgr::inst()->get_control_state(ksctrl->get_joystick_num(), USERCAM_DOWN ) <= ( -AXIS_MAX  * 0.5f ) &&
#else
	    input_mgr::inst()->get_control_state(ksctrl->get_joystick_num(), PSX_R3) == AXIS_MAX &&
#endif
		!ksctrl->IsDoingTrick() && !ksctrl->get_board_controller().InAir() &&
		specialMeter->CanRegionLink())
	{
		finishChain = true;
		specialMeter->Decrease(1.0f);
	}

	// Time to end the chain?
	if (finishChain)
	{
		FinishChain(true);
	}

	// Keep track of previous frame's trick region.
	prevTrickRegion = ksctrl->GetTrickRegion();
}

//	OnEvent()
// Responds to all events.
void ScoringManager::OnEvent(const EVENT event, const int param1, const int param2)
{
	if (param1 != playerIdx)
		return;

	// Notice when surfer lands on the wave.
	if (event == EVT_SURFER_LAND)
	{
		LANDING	land = LAND_REGULAR;
		
		//  Check to see how many 540s there have been.
//		SeriesList::const_iterator	sit;	// series iterator
//		for (sit = chain.series.begin(); sit != chain.series.end(); ++sit)
		{
			if (chain.series.back().numSpins >= 2)
				num360spins++;
			if (chain.series.back().numSpins >= 3)
				num540spins++;
		}

		// The current series has been landed.
		if (param2 & 0x01)
			land = LAND_PERFECT;
		if (param2 & 0x02)
			land = LAND_SLOPPY;
		if (param2 & 0x04)
			land = LAND_JUNK;
		UpdateLastSeries(ATTR_LANDING, int(land));

		// Landed fakey?
		if (param2 & 0x10)
			UpdateLastSeries(ATTR_TO_FAKEY, true);

		// Bad landings reduce special meter.
		if (land == LAND_SLOPPY && chain.IsInteresting())
			specialMeter->Decrease(METER_PENALTY_SLOPPY);
		else if (land == LAND_JUNK && chain.IsInteresting())
			specialMeter->Decrease(METER_PENALTY_JUNK);
	}

	// Notice player wipeout
	if (event == EVT_SURFER_WIPEOUT)
	{
		FinishChain(false);
	}

	// Notice when surfer moves into a different trick region.
	if (event == EVT_TRICK_REGION_CHANGE)
	{
		if (specialMeter->CanRegionLink())
		{
			Series	series;
			
			// Start a new series.
			g_eventManager.DispatchEvent(EVT_SCORING_SERIES_END, playerIdx, 1);
			series.levelTricks = levelTricks;
			if (chain.series.size() < MAX_SERIES_PER_CHAIN)
				chain.series.push_back(series);
		}
		else
			FinishChain(true);
	}
}

//	AddTrick()
// Adds the specified trick to the current chain.
void ScoringManager::AddTrick(const int trickIdx)
{
	Trick							trick;
	//Series							series;
	float							oldSickness;
	TrickList::reverse_iterator		tit;
	SeriesList::reverse_iterator	sit;

	// Amount of tricks in the chain is limited by memory.
	if (chain.series.back().tricks.size() >= MAX_TRICKS_PER_SERIES)
		return;

	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
		frontendmanager.IGO->GetTutorialManager()->SetCurrentTrick(trickIdx);

	assert(trickIdx >= 0 && trickIdx < TRICK_NUM);
	
	// Tell tetris icon the trick that was done
	if(frontendmanager.IGO->GetIconManager())
		frontendmanager.IGO->GetIconManager()->TrickChain(trickIdx);
	
	// Tell learn a new trick manager the trick that was done
	if(frontendmanager.IGO->GetLearnNewTrickManager())
		frontendmanager.IGO->GetLearnNewTrickManager()->TrickChain(trickIdx);
	
	// Play sfx for trick's coolness.


	// Bit of a hack.
	if (GTrickList[trickIdx].flags & FaceFlag)
	{
		g_eventManager.DispatchEvent(EVT_TRICK_FACE_BEGIN, playerIdx);
		g_eventManager.DispatchEvent(EVT_TRICK_FACE_END, playerIdx);
	}

	// Save chain's current sickness.
	oldSickness = chain.GetSickness();

	// Initialize trick to add.
	trick.type = Trick::TYPE_TRICK;
	trick.index = trickIdx;
	trick.mouthDist = mouthDist;
	trick.repetitions = levelTricks[trickIdx].numLandings;
	if (chain.GetTrickCount(trickIdx) >= CAP_COOL && !(GTrickList[trickIdx].flags & NormalRideFlag) && !(GTrickList[trickIdx].flags & ModRideFlag))
		trick.flags |= (1 << MOD_LAME);

	// Add the trick to the end of the last series.
	chain.series.back().tricks.push_back(trick);

	// Coolness of trick adds to sick meter.
	specialMeter->Increase(chain.GetSickness()-oldSickness);

	// Tell IGO to update trick display.
	frontendmanager.IGO->OnTrickChange(playerIdx, true);

	// Count trick repetitions.
	levelTricks[trickIdx].numLandings++;

	// Check for exit trick combo.
	if (GTrickList[trickIdx].flags & ExitFlag)
	{
		for (tit = chain.series.back().tricks.rbegin(); tit != chain.series.back().tricks.rend(); ++tit)
		{
			if ((*tit).type == Trick::TYPE_TRICK && (GTrickList[(*tit).index].flags & AirFlag) &&
				!(GTrickList[(*tit).index].flags & ExitFlag))
			{
				AddGap(GAP_EXIT_COMBO);
				break;
			}
		}
	}

	// Check if player just did a trick combo.
	for (int comboIdx = 0; comboIdx < COMBO_COUNT; comboIdx++)
	{
		int		varIdx = MAX_TRICKS_PER_COMBO-1;	// -1 for 'not found', any other value means 'unknown'
		bool	performed = true;				// true for 'found,' false for 'unknown' 

		// Start varIdx at last variable in combo.
		while (varIdx >= 0 && g_comboDataArray[comboIdx].vars[varIdx] == NONE) varIdx--;
		sit = chain.series.rbegin();
		tit = (*sit).tricks.rbegin();

		for (; varIdx >= 0; varIdx--)  //  scan backwards through combo variables.
		{
			if (g_comboDataArray[comboIdx].type[varIdx] == TYPE_TRICK)
			{
				if (sit == chain.series.rend() ||  //  out of tricks in the chain, so no match.
					(*tit).type != Trick::TYPE_TRICK || (*tit).index != g_comboDataArray[comboIdx].vars[varIdx])
				{
					performed = false;
					break;
				}
				else  //  that one is fine, so back up the trick pointer to the previous trick.
				{
					++tit;
					if (tit == (*sit).tricks.rend())  //  the trick chain is done, so go back to the previous chain.
					{
						 ++sit;
						 tit = (*sit).tricks.rbegin();
					}
				}
			}
			if (g_comboDataArray[comboIdx].type[varIdx] == TYPE_TRICK_TYPE)
			{
				if (sit == chain.series.rend() ||  //  out of tricks in the chain, so no match.
					(*tit).type != Trick::TYPE_TRICK || (*tit).type != g_comboDataArray[comboIdx].vars[varIdx])
				{
					performed = false;
					break;
				}
				else  //  that one is fine, so back up the trick pointer to the previous trick.
				{
					++tit;
					if (tit == (*sit).tricks.rend())  //  the trick chain is done, so go back to the previous chain.
					{
						 ++sit;
						 tit = (*sit).tricks.rbegin();
					}
				}
			}
			else if (g_comboDataArray[comboIdx].type[varIdx] == TYPE_GAP)
			{
				if (sit == chain.series.rend() ||  //  out of tricks in the chain, so no match.
					(*tit).type != Trick::TYPE_GAP || (*tit).index != g_comboDataArray[comboIdx].vars[varIdx])
				{
					performed = false;
					break;
				}
				else  //  that one is fine, so back up the trick pointer to the previous trick.
				{
					++tit;
					if (tit == (*sit).tricks.rend())  //  the trick chain is done, so go back to the previous chain.
					{
						 ++sit;
						 tit = (*sit).tricks.rbegin();
					}
				}
			}
			else if (g_comboDataArray[comboIdx].type[varIdx] == TYPE_REGION)	//  Where are we on the wave.
			{
				if (ksctrl->get_board_controller().GetRegion() != g_comboDataArray[comboIdx].vars[varIdx])
					performed = false;

			}

		}

		// Scan backwards in trick chain.
/*		for (sit = chain.series.rbegin(); sit != chain.series.rend(); ++sit)
		{
			if (performed || varIdx < 0) break;
			
			for (tit = (*sit).tricks.rbegin(); tit != (*sit).tricks.rend(); ++tit)
			{
				if (performed || varIdx < 0) break;
				
				// This combo was definitely not performed.
				if ((*tit).type != Trick::TYPE_TRICK || (*tit).index != g_comboDataArray[comboIdx].vars[varIdx])
					varIdx = -1;
				// Currently Unknown, keep searching.
				else
				{
					// This combo definitely was performed.
					if (varIdx == 0)
						performed = true;
					// Currently Unknown, keep searching.
					else
						varIdx--;
				}
			}
		}*/

		// Found a combo.
		if (performed)
		{
			AddGap(g_comboDataArray[comboIdx].gap);
			break;
		}
	}
}

//	AddGap()
// Adds the specified gap to the current chain.

// Temporary hack to handle the addition of new beaches.  (dc 01/31/02)
// The gap stuff needs to go in the beach database.
void ScoringManager::AddGap(int gapIdx)
//void ScoringManager::AddGap(const int gapIdx)
{
	Trick	trick;
	Series	series;
	float	oldSickness;

	// Amount of tricks in the chain is limited by memory.
	if (chain.series.back().tricks.size() >= MAX_TRICKS_PER_SERIES)
		return;
	
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
		frontendmanager.IGO->GetTutorialManager()->SetCurrentGap(gapIdx);

	// Temporary hack to handle the addition of new beaches.  (dc 01/31/02)
	// The gap stuff needs to go in the beach database.
	if (gapIdx < 0 || gapIdx >= GAP_NUM)
	{
		gapIdx = 0;
	}

	assert(gapIdx >= 0 && gapIdx < GAP_NUM);

	// Bit of a hack.
	if (1)
	{
		g_eventManager.DispatchEvent(EVT_TRICK_FACE_BEGIN, playerIdx);
		g_eventManager.DispatchEvent(EVT_TRICK_FACE_END, playerIdx);
	}

	// Save chain's current sickness.
	oldSickness = chain.GetSickness();

	// Add trick.
	trick.type = Trick::TYPE_GAP;
	trick.index = gapIdx;
	//trick.sickZone = sickZone;
	trick.mouthDist = mouthDist;

	// Add the trick to the end of the last series.
	chain.series.back().tricks.push_back(trick);

	// Coolness of trick adds to sick meter.
	specialMeter->Increase(chain.GetSickness()-oldSickness);

	// Tell IGO to update trick display.
	frontendmanager.IGO->OnTrickChange(playerIdx, true);
}

//	UpdateLastTrick()
// Changes the properties non-gap trick most recently added to the chain that has the specified flags.
void ScoringManager::UpdateLastTrick(const ScoringManager::ATTRIBUTE attr, const float f, const int flags)
{
	float								oldSickness;
	TrickList::reverse_iterator			tit;
	SeriesList::reverse_iterator		sit;

	// Probably only need to search the last series.
	// No point in checking multiple series...?

	// Search backwards through the series.
	for (sit = chain.series.rbegin(); sit != chain.series.rend(); ++sit)
	{
		if ((*sit).tricks.empty())
			continue;

		// Search backwards for the last non-gap trick in this series that matches the specified
		// type and flags.
		for (tit = (*sit).tricks.rbegin(); tit != (*sit).tricks.rend(); ++tit)
		{
			// Ignore gaps.
			if ((*tit).type != Trick::TYPE_TRICK)
				continue;

			// Optional requirement: flag match.
			if (flags && ((GTrickList[(*tit).index].flags & flags) != flags))
				continue;

			oldSickness = chain.GetSickness();

			switch (attr)
			{
			case ATTR_TIME_TOTAL :
				(*tit).time = f;
				frontendmanager.IGO->OnTrickPointChange(playerIdx);
				break;
			case ATTR_TIME_DELTA :
				(*tit).time += f;
				frontendmanager.IGO->OnTrickPointChange(playerIdx);
				break;
			case ATTR_DIST_LIP :
				(*tit).lipDist = f;
				frontendmanager.IGO->OnTrickChange(playerIdx, false);
				break;
			case ATTR_DIST_MOUTH :
				(*tit).mouthDist = f;
				frontendmanager.IGO->OnTrickPointChange(playerIdx);
				break;
			default : assert(0);
			}

			specialMeter->Increase(chain.GetSickness()-oldSickness);

			return;
		}
	}
}

//	UpdateLastTrick()
// Changes the properties non-gap trick most recently added to the chain that has the specified flags.
void ScoringManager::UpdateLastTrick(const ScoringManager::ATTRIBUTE attr, const int i, const int flags)
{
	float								oldSickness;
	TrickList::reverse_iterator			tit;
	SeriesList::reverse_iterator		sit;

	// Probably only need to search the last series.
	// No point in checking multiple series...?

	// Search backwards through the series.
	for (sit = chain.series.rbegin(); sit != chain.series.rend(); ++sit)
	{
		if ((*sit).tricks.empty())
			continue;

		// Search backwards for the last non-gap trick in this series that matches the specified
		// type and flags.
		for (tit = (*sit).tricks.rbegin(); tit != (*sit).tricks.rend(); ++tit)
		{
			// Ignore gaps.
			if ((*tit).type != Trick::TYPE_TRICK)
				continue;

			// Optional requirement: flag match.
			if (flags && ((GTrickList[(*tit).index].flags & flags) != flags))
				continue;

			oldSickness = chain.GetSickness();

			switch (attr)
			{
			case ATTR_NUM_SPINS_DELTA :
				(*tit).numSpins += i;
				frontendmanager.IGO->OnTrickChange(playerIdx, false);
				break;
			case ATTR_NUM_SPINS_TOTAL :
				(*tit).numSpins = i;
				frontendmanager.IGO->OnTrickChange(playerIdx, false);
				break;
			default : assert(0);
			}

			specialMeter->Increase(chain.GetSickness()-oldSickness);

			return;
		}
	}
}

//	UpdateLastTrick()
// Changes the properties non-gap trick most recently added to the chain that has the specified flags.
void ScoringManager::UpdateLastTrick(const ScoringManager::ATTRIBUTE attr, const bool b, const int flags)
{
	float								oldSickness;
	TrickList::reverse_iterator			tit;
	SeriesList::reverse_iterator		sit;

	// Probably only need to search the last series.
	// No point in checking multiple series...?

	// Search backwards through the series.
	for (sit = chain.series.rbegin(); sit != chain.series.rend(); ++sit)
	{
		if ((*sit).tricks.empty())
			continue;

		// Search backwards for the last non-gap trick in this series that matches the specified
		// type and flags.
		for (tit = (*sit).tricks.rbegin(); tit != (*sit).tricks.rend(); ++tit)
		{
			// Ignore gaps.
			if ((*tit).type != Trick::TYPE_TRICK)
				continue;

			// Optional requirement: flag match.
			if (flags && ((GTrickList[(*tit).index].flags & flags) != flags))
				continue;

			oldSickness = chain.GetSickness();

			switch (attr)
			{
			case ATTR_AT_MOUTH :
				if (b)
					(*tit).flags |= (1 << MOD_AT_MOUTH);
				else
					(*tit).flags &= ~(1 << MOD_AT_MOUTH);
				break;
			default : assert(0);
			}

			specialMeter->Increase(chain.GetSickness() - oldSickness);

			return;
		}
	}
}

//	UpdateLastSeries()
// Changes the properties of the trick series most recently added to the chain.
void ScoringManager::UpdateLastSeries(const ScoringManager::ATTRIBUTE attr, const int i)
{
	float		oldSickness;
	Series &	series = chain.series.back();

	oldSickness = chain.GetSickness();

	switch (attr)
	{
	case ATTR_NUM_SPINS_DELTA :
		series.numSpins += i;

		// If surfer launched and spun but didn't do any tricks, insert air trick.
		if (series.tricks.empty() && series.numSpins > 0)
		{
			if (series.flags & (1 << MOD_HOP))
				AddTrick(TRICK_HOP);
			else
				AddTrick(TRICK_AIR);

		}

		frontendmanager.IGO->OnTrickChange(playerIdx, false);
		break;

	case ATTR_NUM_SPINS_TOTAL :
		series.numSpins = i;

		// If surfer launched and spun but didn't do any tricks, insert air trick.
		if (series.tricks.empty() && series.numSpins > 0)
		{
			if (series.flags & (1 << MOD_HOP))
				AddTrick(TRICK_HOP);
			else
				AddTrick(TRICK_AIR);

		}

		frontendmanager.IGO->OnTrickChange(playerIdx, false);
		break;

	case ATTR_LANDING :
		series.landing = (LANDING) i;
		frontendmanager.IGO->OnTrickChange(playerIdx, false);
		break;

	case ATTR_FLAG_SET :
		series.flags |= i;
		frontendmanager.IGO->OnTrickChange(playerIdx, true);
		break;

	default : assert(0);
	}

	specialMeter->Increase(chain.GetSickness() - oldSickness);
}

//	UpdateLastSeries()
// Changes the properties of the trick series most recently added to the chain.
void ScoringManager::UpdateLastSeries(const ScoringManager::ATTRIBUTE attr, const bool b)
{
	Series &	series = chain.series.back();
	float		oldSickness;
	int			flag;

	oldSickness = chain.GetSickness();

	switch (attr)
	{
	case ATTR_FROM_FLOATER :
		flag = 1 << MOD_FROM_FLOATER;
		if (b)
			series.flags |= flag;
		else
			series.flags &= ~flag;
		frontendmanager.IGO->OnTrickChange(playerIdx, false);
		break;

	case ATTR_TO_FAKEY :
		flag = 1 << MOD_TO_FAKEY;
		if (b)
			series.flags |= flag;
		else
			series.flags &= ~flag;
		frontendmanager.IGO->OnTrickChange(playerIdx, false);
		break;

	case ATTR_HOP :
		flag = 1 << MOD_HOP;
		if (b)
			series.flags |= flag;
		else
			series.flags &= ~flag;
		frontendmanager.IGO->OnTrickChange(playerIdx, false);
		break;

	default : assert(0);
	}

	specialMeter->Increase(chain.GetSickness() - oldSickness);
}

//	SetMouthDist()
// Sets the player's current distance from the mouth of the tube.
// All added tricks will use this danger rating to modify their score value.
void ScoringManager::SetMouthDist(const float dist)
{
	assert(dist >= 0.0f && dist <= 1.0f);

	mouthDist = dist;
}

//	SetLipDist()
// Sets the player's current distance from the lip of the wave.
// All added tricks will use this danger rating to modify their score value.
void ScoringManager::SetLipDist(const float dist)
{
	assert(dist >= 0.0f && dist <= 1.0f);

	lipDist = dist;
}

//	GetNumTrickLandings()
// Returns the total number of tricks the specified player has landed since the
// level started.
int ScoringManager::GetNumTrickLandings(void) const
{
	int total = 0;

	for (int i = 0; i < TRICK_NUM; i++)
		total += levelTricks[i].numLandings;

	return total;
}

//	FinishChain()
// Private helper function that closes the current chain.
// If successful, the trick landings are incremented.
void ScoringManager::FinishChain(const bool successful)
{
	SeriesList::const_iterator	sit;	// series iterator
	TrickList::const_iterator	tit;	// trick iterator
	SSEventId e;
	nslSoundId snd;
	int							points, fPoints, aPoints, tPoints;
	bool						locFloat = false, locFace = false, locAir = false, locTube = false;
	float						tubeTime = 0.0f;
	int							retVal = g_game_ptr->get_num_ai_players();
	Series						series;

	lastChainInfo.Reset();

	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
		frontendmanager.IGO->GetTutorialManager()->EndChain();
	
	// Tell tetris icon that a trick chain has ended
	if (frontendmanager.IGO->GetIconManager())
		frontendmanager.IGO->GetIconManager()->FinishChain(successful);
	
	// Tell learn a new trick manager that a trick chain has ended
	//	if(frontendmanager.IGO->GetLearnNewTrickManager())
	//		frontendmanager.IGO->GetLearnNewTrickManager()->FinishChain(successful);
	
	// Landed chain successfully.
	if (successful)
	{
		points = chain.GetScore();
		chain.GetPartialScores(fPoints, aPoints, tPoints);
		
		// Record player's best chain.
		if (points > bestChainScore)
		{
			bestChainScore = points;
			bestChain = chain;
		}
		if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER &&
		    g_game_ptr->is_competition_level())
		{
			if ((points > (BIG_CHEER_POIMTS + random(MED_CHEER_POINTS))))
			{
				e = SoundScriptManager::inst()->playEvent(SS_CROWD_CHEER_LARGE);
				snd = SoundScriptManager::inst()->getSoundId(e);
				if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
				{
					nslSetSoundEmitter(wSound.behindTheCamera, snd);
				}
			}
			else if ((points > (MED_CHEER_POINTS + random(SM_CHEER_POINTS))) && (random() > .5f))
			{
				e = SoundScriptManager::inst()->playEvent(SS_CROWD_CHEER_MEDIUM);
				snd = SoundScriptManager::inst()->getSoundId(e);
				if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
				{
					nslSetSoundEmitter(wSound.behindTheCamera, snd);
				}
			}
			else if ((chain.series.size() == 1) && (points > SM_CHEER_POINTS + random((float)(.1*SM_CHEER_POINTS))))
			{
				e = SoundScriptManager::inst()->playEvent(SS_CROWD_CHEER_SMALL);
				snd = SoundScriptManager::inst()->getSoundId(e);
				if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
				{
					nslSetSoundEmitter(wSound.behindTheCamera, snd);
				}
			}
		}

		// Process every trick in this chain.
		for (sit = chain.series.begin(); sit != chain.series.end(); ++sit)
		{
			for (tit = (*sit).tricks.begin(); tit != (*sit).tricks.end(); ++tit)
			{
				if ((*tit).type == Trick::TYPE_TRICK)
				{
					// Record longest tube ride.
					if ((GTrickList[(*tit).index].flags & NormalRideFlag) ||
						(GTrickList[(*tit).index].flags & ModRideFlag))
						tubeTime += (*tit).time;

					// Record longest floater.
					if ((GTrickList[(*tit).index].flags & GrindFlag) && (*tit).time > longestFloater)
						longestFloater = (*tit).time;

					// Record longest air (wtf?).

					// Trigger special trick success.
					if ((GTrickList[(*tit).index].flags & SpecialFlag))
						specialTrick = SPECIALTRICK_SUCCEEDED;

					// Save chain locations.
					if (GTrickList[(*tit).index].flags & FaceFlag) locFace = true;
					if (GTrickList[(*tit).index].flags & AirFlag) locAir = true;
					if (GTrickList[(*tit).index].flags & GrindFlag) locFloat = true;
					if (GTrickList[(*tit).index].flags & TubeFlag) locTube = true;
					if (locFace && locAir && locTube && locFloat)
						lastChainInfo.multiLocation = true;

					// Count trick repetitions.
					//levelTricks[(*tit).index].numLandings++;

					// Terrible hack - moved to ScoringManager::AddTrick
					//frontendmanager.IGO->IconHack((*tit).index);
				}
			}
		}

		// Record longest tube ride.
		if (tubeTime > longestTubeRide)
			longestTubeRide = tubeTime;

		// Add chain's points to player score.
		if (g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_INFINITE)
		{
			score = points;
			facePoints = fPoints;
			airPoints = aPoints;
			tubePoints = tPoints;
		}
		else
		{
			score += points;
			facePoints += fPoints;
			airPoints += aPoints;
			tubePoints += tPoints;
		}
		frontendmanager.IGO->OnScoreChange(playerIdx);

		lastChainInfo.points = points;
		lastChainInfo.facePoints = fPoints;
		lastChainInfo.airPoints = aPoints;
		lastChainInfo.tubePoints = tPoints;
		lastChainInfo.numTricks = chain.GetNumTricks();
	}
	// Wipeout!
	else
	{
		// Process every trick in this chain.
		for (sit = chain.series.begin(); sit != chain.series.end(); ++sit)
		{
			for (tit = (*sit).tricks.begin(); tit != (*sit).tricks.end(); ++tit)
			{
				if ((*tit).type == Trick::TYPE_TRICK)
				{
					// These were added in when trick was executed, so now they need to be removed.
					levelTricks[(*tit).index].numLandings--;

					// Trigger special trick failure.
					if (GTrickList[(*tit).index].flags & NoInterruptFlag)
						specialTrick = SPECIALTRICK_FAILED;
				}
			}
		}

		if (g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_INFINITE)
		{
			score = 0;
			facePoints = 0;
			airPoints = 0;
			tubePoints = 0; 
		}
	}
	
	if (!(retVal && playerIdx))
	{
		if (successful)
			frontendmanager.IGO->OnTrickComplete(playerIdx);
		else
			frontendmanager.IGO->OnTrickFail(playerIdx);
	}
	
	if (successful)
	{
		g_eventManager.DispatchEvent(EVT_SCORING_SERIES_END, playerIdx, 1);
		g_eventManager.DispatchEvent(EVT_SCORING_CHAIN_END, playerIdx, 1);
	}
	else
	{
		g_eventManager.DispatchEvent(EVT_SCORING_SERIES_END, playerIdx, 0);
		g_eventManager.DispatchEvent(EVT_SCORING_CHAIN_END, playerIdx, 0);
	}
	
	chain.series.resize(0);
	series.levelTricks = levelTricks;
	chain.series.push_back(series);
	chain.SetMultAdder(0.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ScoringManager::Chain class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Chain()
// Default constructor.
ScoringManager::Chain::Chain()
{
	levelTricks = NULL;
	multAdder = 0.0f;
}

//	GetScore()
// Returns the final score of this chain. (affected by multiplier)
int ScoringManager::Chain::GetScore(void) const
{
	return int(float(GetRawScore())*GetMultiplier());
}

//	GetSickness()
// Returns the amount that this chain adds to the sick meter.
float ScoringManager::Chain::GetSickness(void) const
{
	//return GetRawSickness()*GetMultiplier();
	return GetRawSickness();	// multipliers make it too easy to keep meter maxed
}

//	GetPartialScores()
// Retrieves the partial scores of face tricks, air tricks, and tube tricks. (multiplier applied)
void ScoringManager::Chain::GetPartialScores(int & facePoints, int & airPoints, int & tubePoints) const
{
	GetPartialRawScores(facePoints, airPoints, tubePoints);
	facePoints = int(float(facePoints)*GetMultiplier());
	airPoints = int(float(airPoints)*GetMultiplier());
	tubePoints = int(float(tubePoints)*GetMultiplier());
}

//	GetRawScore()
// Returns the raw score of this chain.  (not affected by multiplier)
int ScoringManager::Chain::GetRawScore(void) const
{
	SeriesList::const_iterator	it;
	float						rawScore = 0.0f;

	for (it = series.begin(); it != series.end(); ++it)
		rawScore += (*it).GetRawScore();

	return int(rawScore*GetScale());
}

//	GetRawSickness()
// Returns the amount that this chain adds to the sick meter. (not affected by multiplier)
float ScoringManager::Chain::GetRawSickness(void) const
{
	SeriesList::const_iterator	it;
	float						rawSickness = 0.0f;

	for (it = series.begin(); it != series.end(); ++it)
		rawSickness += (*it).GetRawSickness();

	return rawSickness*GetScale();
}

//	GetPartialRawScores()
// Retrieves the partial raw scores of face tricks, air tricks, and tube tricks. (multiplier not applied)
void ScoringManager::Chain::GetPartialRawScores(int & facePoints, int & airPoints, int & tubePoints) const
{
	SeriesList::const_iterator	it;
	int							rawFaceScore = 0;
	int							rawAirScore = 0;
	int							rawTubeScore = 0;

	facePoints = 0;
	airPoints = 0;
	tubePoints = 0;

	for (it = series.begin(); it != series.end(); ++it)
	{
		(*it).GetPartialRawScores(rawFaceScore, rawAirScore, rawTubeScore);
		facePoints += rawFaceScore;
		airPoints += rawAirScore;
		tubePoints += rawTubeScore;
	}

	facePoints = int(float(facePoints)*GetScale());
	airPoints = int(float(airPoints)*GetScale());
	tubePoints = int(float(tubePoints)*GetScale());
}

//	GetMultiplier()
// Returns the multiplier of this chain.
float ScoringManager::Chain::GetMultiplier(void) const
{
	SeriesList::const_iterator	it;
	float						mult = 0.0f;
	int							add;

	// Multiplier: 1, 2, 3, ..., 7, 7.5, 8.0, 8.5, ...
	for (it = series.begin(); it != series.end(); ++it)
	{
		add = (*it).GetNumMultTricks();

		while (add > 0)
		{
			if (mult < CAP_LINK)
				mult += 1.0f;
			else
				mult += 0.5f;
			add--;
		}
	}

	if (mult <= 1.0f)
		mult = 1.0f;

	return mult + multAdder;
}

//	GetNumTricks()
// Returns the total number of tricks in all series.
int ScoringManager::Chain::GetNumTricks(void) const
{
	SeriesList::const_iterator	sit;
	TrickList::const_iterator	tit;
	int							total = 0;
	u_int						n;

	for (sit = series.begin(); sit != series.end(); ++sit)
	{
		for (tit = (*sit).tricks.begin(), n = 0; tit != (*sit).tricks.end(); ++tit, n++)
		{
			if ((*tit).IsInteresting())
				total++;
		}
	}

	return total;
}

//	IsInteresting()
// Returns true if this chain contains tricks that should be displayed onscreen.
bool ScoringManager::Chain::IsInteresting(void) const
{
	SeriesList::const_iterator	it;

	for (it = series.begin(); it != series.end(); ++it)
	{
		if ((*it).IsInteresting())
			return true;
	}

	return false;
}

//	HasTrick()
// Returns true if a trick with the specified flags is in the chain
bool ScoringManager::Chain::HasTrick(const int flags) const
{
	SeriesList::const_iterator	it;

	for (it = series.begin(); it != series.end(); ++it)
	{
		if ((*it).HasTrick(flags))
			return true;
	}

	return false;
}

//	GetTrickCount()
// Returns the number of times the specified trick appears in the chain.
int ScoringManager::Chain::GetTrickCount(const int trickIdx) const
{
	SeriesList::const_iterator	it;
	int							count = 0;

	for (it = series.begin(); it != series.end(); ++it)
		count += (*it).GetTrickCount(trickIdx);

	return count;
}

void ScoringManager::Chain::SetMultAdder(float m)
{
	multAdder = m;
}

void ScoringManager::Chain::AddMultAdder(float m)
{
	multAdder += m;
}

//	operator=()
// Assignment operator.
ScoringManager::Chain & ScoringManager::Chain::operator=(const ScoringManager::Chain & right)
{
	if (this != &right)
	{
		levelTricks = right.levelTricks;
		series = right.series;
		multAdder = right.multAdder;
	}

	return *this;
}

//	GetScale()
// Private helper function - returns bonus percentage applied to the chain's score.
float ScoringManager::Chain::GetScale(void) const
{
	float	waveScale = 1.0f;

	switch (WAVE_GetScoringType())
	{
	case 'A' : waveScale = SCALE_WAVE[0]; break;
	case 'B' : waveScale = SCALE_WAVE[1]; break;
	case 'C' : waveScale = SCALE_WAVE[2]; break;
	default : waveScale = SCALE_WAVE[1];
	}

	return waveScale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ScoringManager::Series class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Series()
// Default constructor.
ScoringManager::Series::Series()
{
	levelTricks = NULL;
	numSpins = 0;
	landing = LAND_REGULAR;
	flags = 0;
}

//	GetRawScore()
// Returns the raw score of this series.  (not affected by multiplier)
int ScoringManager::Series::GetRawScore(void) const
{
	TrickList::const_iterator	it;
	float						score = 0;

	// Sum points of all tricks in this series.
	for (it = tricks.begin(); it != tricks.end(); ++it)
	{
		if ((*it).IsInteresting())
			score += (*it).GetRawScore(levelTricks);
	}

	return int(GetScale()*score);
}

//	GetRawSickness()
// Returns the amount that this series adds to the sick meter.
float ScoringManager::Series::GetRawSickness(void) const
{
	TrickList::const_iterator	it;
	float						sickness = 0.0f;

	for (it = tricks.begin(); it != tricks.end(); ++it)
		sickness += (*it).GetRawSickness(levelTricks);

	// toby fixme: hack to make perfects add extra to sick meter.
	if (landing == LAND_PERFECT)
		return GetScale()*sickness*2.0f;
	else
		return GetScale()*sickness;
}

//	GetPartialRawScores()
// Retrieves the partial scores of face tricks, air tricks, and tube tricks in this series.
void ScoringManager::Series::GetPartialRawScores(int & facePoints, int & airPoints, int & tubePoints) const
{
	TrickList::const_iterator	it;

	facePoints = 0;
	airPoints = 0;
	tubePoints = 0;

	// Sum points of all tricks in this series.
	for (it = tricks.begin(); it != tricks.end(); ++it)
	{
		if ((*it).IsInteresting())
		{
			// Partial scores currently ignore gaps.
			if ((*it).type == Trick::TYPE_TRICK)
			{
				// This trick counts toward the face partial score.
				if ((GTrickList[(*it).index].flags & FaceFlag) || (GTrickList[(*it).index].flags & GrindFlag))
					facePoints += (*it).GetRawScore(levelTricks);
				// This trick counts toward the air partial score.
				else if (GTrickList[(*it).index].flags & AirFlag)
					airPoints += (*it).GetRawScore(levelTricks);
				// This trick counts toward the air partial score.
				else if (GTrickList[(*it).index].flags & TubeFlag)
					tubePoints += (*it).GetRawScore(levelTricks);
			}
		}
	}

	facePoints = int(GetScale()*float(facePoints));
	airPoints = int(GetScale()*float(airPoints));
	tubePoints = int(GetScale()*float(tubePoints));
}

//	GetNumMultTricks()
// Returns the number of tricks in this series that add to the chain's multiplier.
int ScoringManager::Series::GetNumMultTricks(void) const
{
	TrickList::const_iterator	it;
	int							count = 0;

	for (it = tricks.begin(); it != tricks.end(); ++it)
	{
		if ((*it).type == Trick::TYPE_TRICK &&					// gaps don't add to multiplier
			!(GTrickList[(*it).index].flags & NoMultFlag) &&	// certain tricks don't add to multiplier
			!((*it).flags & (1 << MOD_LAME)))					// lame tricks don't add to multiplier
		{
			// Exit tricks give a bonus.
			if (GTrickList[(*it).index].flags & ExitFlag)
				count += 1;
			// Normal tricks.
			else
				count++;
		}
		else if ((*it).type == Trick::TYPE_GAP && (*it).index == GAP_EXIT_COMBO)	// exit combo gaps add to your multiplier
			count++;
	}

	return count;
}

//	IsInteresting()
// Returns true if this series contains at least one trick that needs to be displayed onscreen.
bool ScoringManager::Series::IsInteresting(void) const
{
	TrickList::const_iterator	it;

	for (it = tricks.begin(); it != tricks.end(); ++it)
	{
		if ((*it).IsInteresting())
			return true;
	}

	return false;
}

//	HasTrick()
// Returns true if this series contains a trick with the specified flags.
bool ScoringManager::Series::HasTrick(const int flags) const
{
	TrickList::const_iterator	it;

	for (it = tricks.begin(); it != tricks.end(); ++it)
	{
		if ((*it).type == Trick::TYPE_TRICK && (GTrickList[(*it).index].flags & flags))
			return true;
	}

	return false;
}

//	GetTrickCount()
// Returns the number of times the specified trick appears in the series.
int ScoringManager::Series::GetTrickCount(const int trickIdx) const
{
	TrickList::const_iterator	it;
	int							count = 0;

	for (it = tricks.begin(); it != tricks.end(); ++it)
	{
		if ((*it).type == Trick::TYPE_TRICK && (*it).index == trickIdx)
			count++;
	}

	return count;
}

//	GetTubeTime()
// Returns the length of this series' tube time.
float ScoringManager::Series::GetTubeTime(void) const
{
	TrickList::const_iterator	tit;
	float						tubeTime = 0.0f;

	for (tit = tricks.begin(); tit != tricks.end(); ++tit)
	{
		if ((*tit).type == Trick::TYPE_TRICK)
		{
			if ((GTrickList[(*tit).index].flags & NormalRideFlag) ||
				(GTrickList[(*tit).index].flags & ModRideFlag))
				tubeTime += (*tit).time;
		}
	}

	return tubeTime;
}

//	operator=()
// Assignment operator.
ScoringManager::Series & ScoringManager::Series::operator=(const Series & right)
{
	if (this != &right)
	{
		levelTricks = right.levelTricks;
		tricks = right.tricks;
		numSpins = right.numSpins;
		landing = right.landing;
		flags = right.flags;
	}

	return *this;
}

//	GetScale()
// Private helper function - return the bonus percentage applied to this series.
float ScoringManager::Series::GetScale(void) const
{
	float						fromFloaterScale = 1.0f;
	float						toFakeyScale = 1.0f;
	float						landingScale = 1.0f;

	if (flags & (1 << MOD_FROM_FLOATER))
		fromFloaterScale = SCALE_SERIES_MODS[MOD_FROM_FLOATER];
	if (flags & (1 << MOD_TO_FAKEY))
		toFakeyScale = SCALE_SERIES_MODS[MOD_TO_FAKEY];

	landingScale = SCALE_LANDINGS[landing];

	return SCALE_SPINS[numSpins]*fromFloaterScale*toFakeyScale*landingScale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ScoringManager::Trick class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Trick()
// Default constructor.
ScoringManager::Trick::Trick()
{
	type = TYPE_TRICK;
	index = -1;
	flags = 0;
	time = 0.0f;
	numSpins = 0;
	mouthDist = 0.5f;
	lipDist = 0.5f;
	repetitions = 0;
}

//	GetRawScore()
// Returns the score of this trick.
int ScoringManager::Trick::GetRawScore(const ScoringManager::LevelTrick * levelTricks, const bool applySick) const
{
	float	points = 0.0f;		// base score
	float	dangerRate = 1.0f;	// bonus for tricks near tube mouth
	float	timeRate = 1.0f;
	float	spinRate = 1.0f;
	float	repeatRate = 1.0f;
	float	atRate = 1.0f;
	float	lipRate = 1.0f;

	// Lame tricks are worth no points.
	if (flags & (1 << MOD_LAME))
		return 0;

	// Calc points for normal tricks.
	if (type == TYPE_TRICK)
	{
		// Get base points of this trick
		points = GTrickList[index].Points;

		// Modify points based on distance from the mouth of the tube.
		if ((GTrickList[index].flags & TubeFlag) == 0)
		{
			if (mouthDist < 0.333333f)
				dangerRate = SCALE_MOUTH_DISTS[0];
			else if (mouthDist >= 0.333333f && mouthDist <= 0.6666666f)
				dangerRate = SCALE_MOUTH_DISTS[1];
			else
				dangerRate = SCALE_MOUTH_DISTS[2];
		}
		
		// Modify points based on length of time trick was held.
		if ((GTrickList[index].flags & ManualFlag) || (GTrickList[index].flags & GrindFlag))
			timeRate = time;
		
		// Modify points based on revolutions.
		if (numSpins >= 1)
			spinRate = SCALE_SPINS[numSpins];
		
		// Lower points if this trick has been repeated a bunch of times.
		if (g_game_ptr->get_game_mode() != GAME_MODE_FREESURF_INFINITE)	// tricks do not degrade in Freesurf Infinite
		{
			// Normal tricks.
			if (!(GTrickList[index].flags & FaceFlag))
			{
				if (repetitions >= 4)
					repeatRate = SCALE_REPEATS[4];
				else if (repetitions >= 3)
					repeatRate = SCALE_REPEATS[3];
				else if (repetitions >= 2)
					repeatRate = SCALE_REPEATS[2];
				else if (repetitions >= 1)
					repeatRate = SCALE_REPEATS[1];
				else
					repeatRate = SCALE_REPEATS[0];
			}
			// Face tricks.
			else
			{
				if (repetitions >= 4)
					repeatRate = SCALE_REPEATS_FACE[4];
				else if (repetitions >= 3)
					repeatRate = SCALE_REPEATS_FACE[3];
				else if (repetitions >= 2)
					repeatRate = SCALE_REPEATS_FACE[2];
				else if (repetitions >= 1)
					repeatRate = SCALE_REPEATS_FACE[1];
				else
					repeatRate = SCALE_REPEATS_FACE[0];
			}
		}
		
		// Face tricks performed towards the tube modify points.
		if ((flags & (1 << MOD_AT_MOUTH)) && (GTrickList[index].flags & FaceFlag))
		{
			atRate = SCALE_AT_TUBE;
		}
		
		// Face tricks closer to the lip modify points.
		if (GTrickList[index].flags & FaceFlag)
		{
			if (lipDist < 0.333333f)
				lipRate = SCALE_LIP_DISTS[0];
			else if (lipDist >= 0.333333f && lipDist <= 0.6666666f)
				lipRate = SCALE_LIP_DISTS[1];
			else
				lipRate = SCALE_LIP_DISTS[2];
		}
		
		// Apply modifiers to base points.
		points *= repeatRate*timeRate*spinRate*dangerRate*atRate*lipRate;	
	}
	// This trick is actually a gap.
	else
	{
		// Gaps do not have modifiers.
		points = g_gapList[index].points;
	}

	// Return modified value.
	return int(points);
}

//	GetRawSickness()
// Returns the amount that this trick adds to the sick meter.
float ScoringManager::Trick::GetRawSickness(const ScoringManager::LevelTrick * levelTricks) const
{
	// Stupid Activision hack: snaps give exactly 50% to meter.
	if (type == TYPE_TRICK && index == TRICK_SNAP)
		return 0.5f;
	
	// Face tricks.
	if (type == TYPE_TRICK && (GTrickList[index].flags & FaceFlag))
		return float(GetRawScore(levelTricks, false))/float(METER_FILL_RATE_FACE);
	// Normal tricks.
	else
		return float(GetRawScore(levelTricks, false))/float(METER_FILL_RATE);
}

//	IsInteresting()
// Returns false if this node has a score of zero and should not be displayed.
bool ScoringManager::Trick::IsInteresting(void) const
{
	return true;
}

//	GetText()
// Returns a text description of this trick.
stringx ScoringManager::Trick::GetText(void) const
{
	stringx	text;
	
	// invalid indices
	assert( index >= 0 );
	//assert( index < GT_End );

	// Non-gap tricks.
	if (type == TYPE_TRICK)
	{
		// Add takeoff modifier to text.
		if (GTrickList[index].flags & TakeoffFlag)
		{
			if (lipDist > 0.666666666f)
				text += ksGlobalTextArray[GT_TRICK_WEAK] + " ";
			else if (lipDist < 0.333333333f)
				text += ksGlobalTextArray[GT_TRICK_LATE] + " ";
		}

		// Lame tricks suck.
		if (flags & (1 << MOD_LAME))
			text += ksGlobalTextArray[GT_TRICK_LAME] + " ";

		// Trick name.
		text += ksGlobalTrickTextArray[index];

		// Add spin text.
		if (numSpins >= 8)
			text += " 1440";
		else if (numSpins >= 7)
			text += " 1260";
		else if (numSpins >= 6)
			text += " 1080";
		else if (numSpins >= 5)
			text += " 900";
		else if (numSpins >= 4)
			text += " 720";
		else if (numSpins >= 3)
			text += " 540";
		else if (numSpins >= 2)
			text += " 360";
		else if (numSpins >= 1)
			text += " 180";
	}
	// Gaps.
	else
		text += ksGlobalGapTextArray[index];

	return text;
}

//	operator=
// Assignment operator for Trick.
ScoringManager::Trick & ScoringManager::Trick::operator=(const ScoringManager::Trick & right)
{
	if (this != &right)
	{
		type = right.type;
		index = right.index;
		flags = right.flags;
		time = right.time;
		numSpins = right.numSpins;
		mouthDist = right.mouthDist;
		lipDist = right.lipDist;
		repetitions = right.repetitions;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Global functions
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	sceWrite()
// Overloaded method to write a stringx to a file.
int sceWrite(const int fd, const stringx & s)
{
#if defined(TARGET_PS2)
	return sceWrite(fd, s.c_str(), s.size());
#else
	return 0;
#endif
}

//	LoadScoringSystem()
// Global function - loads static configuration values for scoring manager and special meter.
void LoadScoringSystem(void)
{
	stringx		filename;
	text_parser	file;
	float		f;

	// Find the scoring.dat file.
	if (!file_finder_exists(stringx("scoring"), stringx(".dat"), &filename))
	{
		nglPrintf("Couldn't open scoring.dat\n");
		assert(false);
	}

	// Load the data file.
	if (!file.load_file(filename))
	{
		nglPrintf("Couldn't load scoring system file %s\n", filename.c_str());
		assert(false);
	}

	// Read modifier data.
	while (file.get_token(true, true))
	{
		//////////////////////////////////////////////////////////////////////////////
		//	ScoringManager values
		//////////////////////////////////////////////////////////////////////////////
		
		// Revolutions.
		if (!stricmp(file.token, "spin_180"))
			file.get_float(&ScoringManager::SCALE_SPINS[1], false, false);
		else if (!stricmp(file.token, "spin_360"))
			file.get_float(&ScoringManager::SCALE_SPINS[2], false, false);
		else if (!stricmp(file.token, "spin_540"))
			file.get_float(&ScoringManager::SCALE_SPINS[3], false, false);
		else if (!stricmp(file.token, "spin_720"))
			file.get_float(&ScoringManager::SCALE_SPINS[4], false, false);
		else if (!stricmp(file.token, "spin_900"))
			file.get_float(&ScoringManager::SCALE_SPINS[5], false, false);
		else if (!stricmp(file.token, "spin_1080"))
			file.get_float(&ScoringManager::SCALE_SPINS[6], false, false);
		else if (!stricmp(file.token, "spin_1260"))
			file.get_float(&ScoringManager::SCALE_SPINS[7], false, false);
		else if (!stricmp(file.token, "spin_1440"))
			file.get_float(&ScoringManager::SCALE_SPINS[8], false, false);

		// Repetitions.
		else if (!stricmp(file.token, "rep_1"))
			file.get_float(&ScoringManager::SCALE_REPEATS[1], false, false);
		else if (!stricmp(file.token, "rep_2"))
			file.get_float(&ScoringManager::SCALE_REPEATS[2], false, false);
		else if (!stricmp(file.token, "rep_3"))
			file.get_float(&ScoringManager::SCALE_REPEATS[3], false, false);
		else if (!stricmp(file.token, "rep_n"))
			file.get_float(&ScoringManager::SCALE_REPEATS[4], false, false);

		// Wave scales.
		else if (!stricmp(file.token, "wave_a"))
			file.get_float(&ScoringManager::SCALE_WAVE[0], false, false);
		else if (!stricmp(file.token, "wave_b"))
			file.get_float(&ScoringManager::SCALE_WAVE[1], false, false);
		else if (!stricmp(file.token, "wave_c"))
			file.get_float(&ScoringManager::SCALE_WAVE[2], false, false);

		// Distance from the mouth of the tube.
		else if (!stricmp(file.token, "mouth_near"))
			file.get_float(&ScoringManager::MOUTH_DISTANCES[0], false, false);
		else if (!stricmp(file.token, "mouth_far"))
			file.get_float(&ScoringManager::MOUTH_DISTANCES[1], false, false);

		// Point bonuses for distance from mouth.
		else if (!stricmp(file.token, "mouth_high"))
			file.get_float(&ScoringManager::SCALE_MOUTH_DISTS[0], false, false);
		else if (!stricmp(file.token, "mouth_med"))
			file.get_float(&ScoringManager::SCALE_MOUTH_DISTS[1], false, false);
		else if (!stricmp(file.token, "mouth_low"))
			file.get_float(&ScoringManager::SCALE_MOUTH_DISTS[2], false, false);

		// Point bonuses for distance from lip.
		else if (!stricmp(file.token, "face_lip_high"))
			file.get_float(&ScoringManager::SCALE_LIP_DISTS[0], false, false);
		else if (!stricmp(file.token, "face_lip_med"))
			file.get_float(&ScoringManager::SCALE_LIP_DISTS[1], false, false);
		else if (!stricmp(file.token, "face_lip_low"))
			file.get_float(&ScoringManager::SCALE_LIP_DISTS[2], false, false);

		// Landings.
		else if (!stricmp(file.token, "land_perfect"))
			file.get_float(&ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_PERFECT], false, false);
		else if (!stricmp(file.token, "land_regular"))
			file.get_float(&ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_REGULAR], false, false);
		else if (!stricmp(file.token, "land_sloppy"))
			file.get_float(&ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_SLOPPY], false, false);
		else if (!stricmp(file.token, "land_junk"))
			file.get_float(&ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_JUNK], false, false);

		// Meter config.
		else if (!stricmp(file.token, "meter_fill_rate"))
		{
			file.get_float(&f, false, false);
			ScoringManager::METER_FILL_RATE = int(f);
		}
		else if (!stricmp(file.token, "meter_fill_rate_face"))
		{
			file.get_float(&f, false, false);
			ScoringManager::METER_FILL_RATE_FACE = int(f);
		}
		else if (!stricmp(file.token, "meter_drop_rate"))
			file.get_float(&SpecialMeter::DROP_RATE, false, false);
		else if (!stricmp(file.token, "meter_drop_rate_special"))
			file.get_float(&SpecialMeter::DROP_RATE_SPECIAL, false, false);
		else if (!stricmp(file.token, "meter_drop_rate_special_face"))
			file.get_float(&SpecialMeter::DROP_RATE_SPECIAL_FACE, false, false);
		else if (!stricmp(file.token, "meter_penalty_sloppy"))
			file.get_float(&ScoringManager::METER_PENALTY_SLOPPY, false, false);
		else if (!stricmp(file.token, "meter_penalty_junk"))
			file.get_float(&ScoringManager::METER_PENALTY_JUNK, false, false);
		else if (!stricmp(file.token, "meter_perfect_bonus"))
			file.get_float(&SpecialMeter::PERFECT_BONUS, false, false);

		// Series flags.
		else if (!stricmp(file.token, "flag_series_from_floater"))
			file.get_float(&ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_FROM_FLOATER], false, false);
		else if (!stricmp(file.token, "flag_series_to_fakey"))
			file.get_float(&ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_TO_FAKEY], false, false);

		// Face trick at mouth bonus.
		else if (!stricmp(file.token, "face_at_mouth"))
			file.get_float(&ScoringManager::SCALE_AT_TUBE, false, false);

		// Caps.
		else if (!stricmp(file.token, "cap_link"))
		{
			file.get_float(&f, false, false);
			ScoringManager::CAP_LINK = int(f);
		}
		else if (!stricmp(file.token, "cap_cool"))
		{
			file.get_float(&f, false, false);
			ScoringManager::CAP_COOL = int(f);
		}

		//////////////////////////////////////////////////////////////////////////////
		//	SpecialManager values
		//////////////////////////////////////////////////////////////////////////////
	}
}

void SaveScoringSystem(void)
{
#if defined(TARGET_PS2)

	int		fout;
	stringx	s;
	
	// Open the file.
	fout = sceOpen("host:scoring.dat", SCE_WRONLY | SCE_TRUNC | SCE_CREAT);
	if (fout < 0)
		return;

	sceWrite(fout, "//\n");
	sceWrite(fout, "//	scoring.dat\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Scoring system data file.\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Spin revolutions:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	180s are worth "+stringx(int(ScoringManager::SCALE_SPINS[1]*100.0f))+"%\n");
	sceWrite(fout, "//	360s are worth "+stringx(int(ScoringManager::SCALE_SPINS[2]*100.0f))+"%\n");
	sceWrite(fout, "//	540s are worth "+stringx(int(ScoringManager::SCALE_SPINS[3]*100.0f))+"%\n");
	sceWrite(fout, "//	720s are worth "+stringx(int(ScoringManager::SCALE_SPINS[4]*100.0f))+"%\n");
	sceWrite(fout, "//	900s are worth "+stringx(int(ScoringManager::SCALE_SPINS[5]*100.0f))+"%\n");
	sceWrite(fout, "//	1080s are worth "+stringx(int(ScoringManager::SCALE_SPINS[6]*100.0f))+"%\n");
	sceWrite(fout, "//	1260s are worth "+stringx(int(ScoringManager::SCALE_SPINS[7]*100.0f))+"%\n");
	sceWrite(fout, "//	1440s are worth "+stringx(int(ScoringManager::SCALE_SPINS[8]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "spin_180	"+stringx(ScoringManager::SCALE_SPINS[1])+"\n");
	sceWrite(fout, "spin_360	"+stringx(ScoringManager::SCALE_SPINS[2])+"\n");
	sceWrite(fout, "spin_540	"+stringx(ScoringManager::SCALE_SPINS[3])+"\n");
	sceWrite(fout, "spin_720	"+stringx(ScoringManager::SCALE_SPINS[4])+"\n");
	sceWrite(fout, "spin_900	"+stringx(ScoringManager::SCALE_SPINS[5])+"\n");
	sceWrite(fout, "spin_1080	"+stringx(ScoringManager::SCALE_SPINS[6])+"\n");
	sceWrite(fout, "spin_1260	"+stringx(ScoringManager::SCALE_SPINS[7])+"\n");
	sceWrite(fout, "spin_1440	"+stringx(ScoringManager::SCALE_SPINS[8])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// All tricks except tube tricks:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//		Distance from mouth of tube:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//			High bonus zone: [0, "+stringx(ScoringManager::MOUTH_DISTANCES[0])+") units from mouth\n");
	sceWrite(fout, "//			Med bonus zone:  ["+stringx(ScoringManager::MOUTH_DISTANCES[0])+", "+stringx(ScoringManager::MOUTH_DISTANCES[1])+") units from mouth\n");
	sceWrite(fout, "//			Low bonus zone:  ["+stringx(ScoringManager::MOUTH_DISTANCES[1])+", inf) units from mouth\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//		Tricks in high bonus zone are worth "+stringx(int(ScoringManager::SCALE_MOUTH_DISTS[0]*100.0f))+"%\n");
	sceWrite(fout, "//		Tricks in medium bonus zone are worth "+stringx(int(ScoringManager::SCALE_MOUTH_DISTS[1]*100.0f))+"%\n");
	sceWrite(fout, "//		Ticks in the low bonus zone are worth "+stringx(int(ScoringManager::SCALE_MOUTH_DISTS[2]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "mouth_near	"+stringx(ScoringManager::MOUTH_DISTANCES[0])+"\n");
	sceWrite(fout, "mouth_far	"+stringx(ScoringManager::MOUTH_DISTANCES[1])+"\n");
	sceWrite(fout, "mouth_high	"+stringx(ScoringManager::SCALE_MOUTH_DISTS[0])+"\n");
	sceWrite(fout, "mouth_med	"+stringx(ScoringManager::SCALE_MOUTH_DISTS[1])+"\n");
	sceWrite(fout, "mouth_low	"+stringx(ScoringManager::SCALE_MOUTH_DISTS[2])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Face tricks:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Tricks in high bonus zone are worth "+stringx(int(ScoringManager::SCALE_LIP_DISTS[0]*100.0f))+"%\n");
	sceWrite(fout, "//	Tricks in medium bonus zone are worth "+stringx(int(ScoringManager::SCALE_LIP_DISTS[1]*100.0f))+"%\n");
	sceWrite(fout, "//	Ticks in the low bonus zone are worth "+stringx(int(ScoringManager::SCALE_LIP_DISTS[2]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "face_lip_high	"+stringx(ScoringManager::SCALE_LIP_DISTS[0])+"\n");
	sceWrite(fout, "face_lip_med	"+stringx(ScoringManager::SCALE_LIP_DISTS[1])+"\n");
	sceWrite(fout, "face_lip_low	"+stringx(ScoringManager::SCALE_LIP_DISTS[2])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Series flags:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Tricks performed off a floater are worth "+stringx(int(ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_FROM_FLOATER]*100.0f))+"%\n");
	sceWrite(fout, "//	Tricks landed fakey are worth "+stringx(int(ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_TO_FAKEY]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "flag_series_from_floater	"+stringx(ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_FROM_FLOATER])+"\n");
	sceWrite(fout, "flag_series_to_fakey		"+stringx(ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_TO_FAKEY])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Meter configuration:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Face tricks are divided by "+stringx(ScoringManager::METER_FILL_RATE_FACE)+" then added to the meter.\n");
	sceWrite(fout, "//	Other tricks are divided by "+stringx(ScoringManager::METER_FILL_RATE)+" then added to the meter.\n");
	sceWrite(fout, "//	Normal meter drops at a rate of "+stringx(SpecialMeter::DROP_RATE)+" per second.\n");
	sceWrite(fout, "//	Meter in linking zone drops at a rate of ("+stringx(SpecialMeter::DROP_RATE_SPECIAL)+")*(time) per second.\n");
	sceWrite(fout, "//	Meter drops at a rate of "+stringx(SpecialMeter::DROP_RATE_SPECIAL_FACE)+" per second while doing special face tricks.\n");
	sceWrite(fout, "//	Meter drops by "+stringx(ScoringManager::METER_PENALTY_SLOPPY)+" when surfer lands sloppy.\n");
	sceWrite(fout, "//	Meter drops by "+stringx(ScoringManager::METER_PENALTY_JUNK)+" when surfer lands junk.\n");
	sceWrite(fout, "//	Consecutive perfects deduct "+stringx(SpecialMeter::PERFECT_BONUS)+" seconds from amount of time meter has been enabled.\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "meter_fill_rate_face         "+stringx(ScoringManager::METER_FILL_RATE_FACE)+"\n");
	sceWrite(fout, "meter_fill_rate              "+stringx(ScoringManager::METER_FILL_RATE)+"\n");
	sceWrite(fout, "meter_drop_rate              "+stringx(SpecialMeter::DROP_RATE)+"\n");
	sceWrite(fout, "meter_drop_rate_special      "+stringx(SpecialMeter::DROP_RATE_SPECIAL)+"\n");
	sceWrite(fout, "meter_drop_rate_special_face "+stringx(SpecialMeter::DROP_RATE_SPECIAL_FACE)+"\n");
	sceWrite(fout, "meter_penalty_sloppy         "+stringx(ScoringManager::METER_PENALTY_SLOPPY)+"\n");
	sceWrite(fout, "meter_penalty_junk           "+stringx(ScoringManager::METER_PENALTY_JUNK)+"\n");
	sceWrite(fout, "meter_perfect_bonus		     "+stringx(SpecialMeter::PERFECT_BONUS)+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Facing the mouth bonus:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Some face tricks are worth "+stringx(int(ScoringManager::SCALE_AT_TUBE*100.0f))+"% if performed facing the mouth of the tube\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "face_at_mouth	"+stringx(ScoringManager::SCALE_AT_TUBE)+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Trick repetitions:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	1st	trick repetition is worth "+stringx(int(ScoringManager::SCALE_REPEATS[1]*100.0f))+"%\n");
	sceWrite(fout, "//	2nd	trick repetition is worth "+stringx(int(ScoringManager::SCALE_REPEATS[2]*100.0f))+"%\n");
	sceWrite(fout, "//	3rd	trick repetition is worth "+stringx(int(ScoringManager::SCALE_REPEATS[3]*100.0f))+"%\n");
	sceWrite(fout, "//	nth	trick repetition is worth "+stringx(int(ScoringManager::SCALE_REPEATS[4]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "rep_1	"+stringx(ScoringManager::SCALE_REPEATS[1])+"\n");
	sceWrite(fout, "rep_2	"+stringx(ScoringManager::SCALE_REPEATS[2])+"\n");
	sceWrite(fout, "rep_3	"+stringx(ScoringManager::SCALE_REPEATS[3])+"\n");
	sceWrite(fout, "rep_n	"+stringx(ScoringManager::SCALE_REPEATS[4])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Wave scales:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Waves of type A are worth "+stringx(int(ScoringManager::SCALE_WAVE[0]*100.0f))+"%\n");
	sceWrite(fout, "//	Waves of type B are worth "+stringx(int(ScoringManager::SCALE_WAVE[1]*100.0f))+"%\n");
	sceWrite(fout, "//	Waves of type C are worth "+stringx(int(ScoringManager::SCALE_WAVE[2]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "wave_a	"+stringx(ScoringManager::SCALE_WAVE[0])+"\n");
	sceWrite(fout, "wave_b	"+stringx(ScoringManager::SCALE_WAVE[1])+"\n");
	sceWrite(fout, "wave_c	"+stringx(ScoringManager::SCALE_WAVE[2])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Landings:\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Perfect landings are worth "+stringx(int(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_PERFECT]*100.0f))+"%\n");
	sceWrite(fout, "//	Regular landings are worth "+stringx(int(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_REGULAR]*100.0f))+"%\n");
	sceWrite(fout, "//	Sloppy landings are worth "+stringx(int(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_SLOPPY]*100.0f))+"%\n");
	sceWrite(fout, "//	Junk landings are worth "+stringx(int(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_JUNK]*100.0f))+"%\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "land_perfect	"+stringx(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_PERFECT])+"\n");
	sceWrite(fout, "land_regular	"+stringx(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_REGULAR])+"\n");
	sceWrite(fout, "land_sloppy		"+stringx(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_SLOPPY])+"\n");
	sceWrite(fout, "land_junk		"+stringx(ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_JUNK])+"\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "\n");
	sceWrite(fout, "// Caps\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "//	Multipliers beyond trick #"+stringx(ScoringManager::CAP_LINK)+" are halved.\n");
	sceWrite(fout, "//	Trick becomes lame after "+stringx(ScoringManager::CAP_COOL)+" repetitions in the same chain.\n");
	sceWrite(fout, "//\n");
	sceWrite(fout, "cap_link	"+stringx(ScoringManager::CAP_LINK)+"\n");
	sceWrite(fout, "cap_cool	"+stringx(ScoringManager::CAP_COOL)+"\n");
	sceWrite(fout, "\n");
	
	// All done.
	sceClose(fout);

#endif // !defined(TARGET_XBOX)
}
