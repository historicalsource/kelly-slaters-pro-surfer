
#include "global.h"

#if defined(TARGET_XBOX)
#include "game.h"
#include "random.h"
#endif

#include "judge.h"

//	JudgingSystem()
// Default constructor.
JudgingSystem::JudgingSystem()
{
	OnCompetitionReset();
}

//	~JudgingSystem()
// Destructor.
JudgingSystem::~JudgingSystem()
{
	
}

//	OnCompetitionReset()
// Resets the competitors' scores and difficulties.
// This method must be called at the beginning of every competition.
void JudgingSystem::OnCompetitionReset(void)
{
	float	sampleJudgedScore;
	int		sampleScores[3];
	float	oppTopJudgedScore;
	float	oppSeparation;
	int		rank;
	int		i, j;

	// Read competition difficulty values from the career database.
	sampleJudgedScore = float(CareerDataArray[g_game_ptr->get_level_id()].goal_param[0])/10.0f;
	sampleScores[0] = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[0];
	sampleScores[1] = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[1];
	sampleScores[2] = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[2];
	oppTopJudgedScore = float(CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[3])/10.0f;
	oppSeparation = float(CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[4])/10.0f;

	// Determine valid competitors.
	for (i = 0; i < SURFER_LAST; i++)
	{
		if (i < SURFER_MISC_SURFER || i == g_game_ptr->GetSurferIdx(0))
			isCompetitor[i] = true;
		else
			isCompetitor[i] = false;
	}

	// If player is using a misc surfer, then remove a random normal surfer
	// so that the total competitors will always be nine.
	if (g_game_ptr->GetSurferIdx(0) >= SURFER_MISC_SURFER)
		isCompetitor[random(SURFER_MISC_SURFER)] = false;

	// Use rank variable to predetermine which surfers will be the best.
	for (i = 0; i < SURFER_LAST; i++)
		results[i].rank = 0;
	for (i = 0; i < SURFER_LAST; i++)
	{
		// Ignore invalid surfers.
		if (!isCompetitor[i] || i == g_game_ptr->GetSurferIdx(0)) continue;

		// Choose an unused rank for this surfer.
		bool ok = false;
		while (!ok)
		{
			// Pick a random one.
			rank = random(1, NUM_COMPETITORS);
			ok = true;
			
			// Make sure this one isn't used already.
			for (j = 0; j < SURFER_LAST; j++)
			{
				if (!isCompetitor[j] || i == g_game_ptr->GetSurferIdx(0)) continue;
				if (results[j].rank == rank)
				{
					ok = false;
					break;
				}
			}
		}
		results[i].rank = rank;
	}

	// Generate AI surfer skills using the rank variable as a temporary ordering.
	for (i = 0; i < SURFER_LAST; i++)
	{
		// Ignore invalid surfers.
		if (!isCompetitor[i] || i == g_game_ptr->GetSurferIdx(0)) continue;

		// Generate AI surfer skills.
		skills[i] = oppTopJudgedScore-oppSeparation*(results[i].rank-1);
	}

	// Calculate difficulty constants for each trick region.
	for (j = 0; j < 3; j++)
		difficulties[j] = -float(sampleScores[j])*(sampleJudgedScore - 10.0f);

	// Reset results.
	for (i = 0; i < SURFER_LAST; i++)
	{
		results[i].runs[0].regionScores[0] = 0.0f;
		results[i].runs[0].regionScores[1] = 0.0f;
		results[i].runs[0].regionScores[2] = 0.0f;
		results[i].runs[1].regionScores[0] = 0.0f;
		results[i].runs[1].regionScores[1] = 0.0f;
		results[i].runs[1].regionScores[2] = 0.0f;
		results[i].runs[2].regionScores[0] = 0.0f;
		results[i].runs[2].regionScores[1] = 0.0f;
		results[i].runs[2].regionScores[2] = 0.0f;
		results[i].rank = 0;
	}

	currRunIdx = 0;
	over = false;
	numPlayerWipeouts = 0;
}

//	JudgeRun()
// Judges the previous run for all surfers in the competition.
void JudgingSystem::JudgeRun(void)
{
	int		score[3];
	float	judgedScore[3];
	float	deviation;
	int		i;
	
	// Judge each surfer's run.
	for (i = 0; i < SURFER_LAST; i++)
	{
		// Ignore invalid surfers in the array.
		if (!isCompetitor[i]) continue;

		// If this surfer is the player, then use his run.
		if (i == g_game_ptr->GetSurferIdx(0))
		{
			// Use player score.
			g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetPartialScores(score[0], score[1], score[2]);

			// Deduct points for wipeouts.
			//score -= WIPEOUT_PENALTY*numPlayerWipeouts;

			judgedScore[0] = Judge(score[0], 0);
			judgedScore[1] = Judge(score[1], 1);
			judgedScore[2] = Judge(score[2], 2);
		}
		// If this surfer is an AI opponent, then generate a run.
		else
		{
			deviation = float(random(1, 5))/10.0f;
			if (random(2) == 1)
				deviation = -deviation;
			judgedScore[0] = skills[i] + deviation;
			judgedScore[1] = skills[i] + deviation;
			judgedScore[2] = skills[i] + deviation;
		}

		// Apply the surfer's judged scores.
		if (judgedScore[0] > 9.9f)
			judgedScore[0] = 9.9f;
		if (judgedScore[0] < 0.0f)
			judgedScore[0] = 0.0f;
		if (judgedScore[1] > 9.9f)
			judgedScore[1] = 9.9f;
		if (judgedScore[1] < 0.0f)
			judgedScore[1] = 0.0f;
		if (judgedScore[2] > 9.9f)
			judgedScore[2] = 9.9f;
		if (judgedScore[2] < 0.0f)
			judgedScore[2] = 0.0f;
		results[i].runs[currRunIdx].regionScores[0] = judgedScore[0];
		results[i].runs[currRunIdx].regionScores[1] = judgedScore[1];
		results[i].runs[currRunIdx].regionScores[2] = judgedScore[2];
	}
	
	CalcRank();
	AdvanceRun();
}

//	AdvanceRun()
// Private helper function: automatically called after JudgeRun().
void JudgingSystem::AdvanceRun(void)
{
	int	levelIdx = g_game_ptr->get_level_id();
	int	requiredRank;

	numPlayerWipeouts = 0;
	currRunIdx++;

	// End of competition?
	if (currRunIdx == 3)
	{
		currRunIdx = 0;
		over = true;

		if (CareerDataArray[levelIdx].goal[0] == GOAL_COMPETITION_3)
			requiredRank = 3;
		else if (CareerDataArray[levelIdx].goal[0] == GOAL_COMPETITION_2)
			requiredRank = 2;
		else
			requiredRank = 1;

		//  Most goals are completed during the game.  This is completed after the level is over,
		//  so handle the career stuff here instead of in beach.cpp
		if (results[g_game_ptr->GetSurferIdx(0)].rank <= requiredRank)
		{
			if (!g_career->levels[levelIdx].IsGoalDone(0))
			{
				stringx completion_message;
				g_career->GetGoalText(levelIdx, 0, completion_message); // get the goal text
				g_career->levels[levelIdx].SetGoalDone(0);             // set the goal done
				g_career->OnGoalDone(levelIdx, 0);
			}
			else // remember that a goal was reached but it was already done....
				g_career->OnGoalReDone(levelIdx, 0);
		}
	}
}

//	CalcRank()
// Private helper function that computes each surfer's rank.
void JudgingSystem::CalcRank(void)
{
	int	i;
	
	// Reset ranks.
	for (i = 0; i < SURFER_LAST; i++)
		results[i].rank = 0;
	
	// Compute each surfer's rank.
	for (int rank = 1; rank <= NUM_COMPETITORS; rank++)
	{
		int largestIdx = -1;
		for (i = 0; i < SURFER_LAST; i++)
		{
			// Ignore invalid surfers.
			if (!isCompetitor[i]) continue;
			
			if (results[i].rank == 0)
			{
				if (largestIdx == -1 || results[i].GetRunTotal() > results[largestIdx].GetRunTotal())
					largestIdx = i;
			}
		}
		
		results[largestIdx].rank = rank;
	}
}

//	Judge()
// Private helper function: given a surfer's points, this method returns the judge's score.
// Region: 0 for face, 1 for air, 2 for tube.
float JudgingSystem::Judge(const int score, const int region) const
{
	//
	// y = -a/x + 10
	//
	// x is the input surfer's score
	// y is the output judged score
	// a is a difficulty constant
	
	if (score > 0)
		return -difficulties[int(region)]/score + 10.0f;
	else
		return 0.0f;
}