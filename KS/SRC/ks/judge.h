
#ifndef INCLUDED_JUDGE_H
#define INCLUDED_JUDGE_H

#include "global.h"

#ifndef BEACHDATA_H
#include "beachdata.h"
#endif
#include "surferdata.h"
#include "game.h"

// Used to judge and rank player during competitions.
class JudgingSystem
{
public:
	enum { NUM_COMPETITORS = 9 };

public:
	struct RUN
	{
		float	regionScores[3];	// 0 = face, 1 = air, 2 = tube

		float GetAverageScore(void) const
		{
			if (g_game_ptr->get_beach_id() != BEACH_MAVERICKS)
				return (regionScores[0]+regionScores[1]+regionScores[2])/3.0f;
			else
				return (regionScores[0]+regionScores[1])/2.0f;

		}
	};

	struct RESULT
	{
		RUN		runs[3];	// judged score for surfer's three runs [0.0, 9.9)
		int		rank;		// this surfer's rank among the other competitors

		float GetRunTotal(void) const { return runs[0].GetAverageScore() + runs[1].GetAverageScore() + runs[2].GetAverageScore(); }
	};

protected:
	float	skills[SURFER_LAST];		// surfers' likely final scores for each run
	bool	isCompetitor[SURFER_LAST];	// true for each surfer that is competing in the competition
	float	difficulties[3];			// a difficulty constant for each trick region

	RESULT	results[SURFER_LAST];		// stored values for previously judged runs
	int		currRunIdx;
	bool	over;						// is competition over?
	int		numPlayerWipeouts;

protected:
	void AdvanceRun(void);
	void CalcRank(void);
	float Judge(const int score, const int region) const;

public:
	// Creators.
	JudgingSystem();
	~JudgingSystem();

	// Modifiers.
	void OnCompetitionReset(void);
	void JudgeRun(void);

	// Accessors.
	RESULT * GetCompetitionResults(void) { return results; }
	bool IsCompetitor(const int surferIdx) const { assert(surferIdx >= 0 && surferIdx < SURFER_LAST); return isCompetitor[surferIdx]; }
	int GetNumCompetitors(void) const { return NUM_COMPETITORS; }
	int GetCurrRunIdx(void) const { return currRunIdx; }
	bool IsCompetitionOver(void) const { return over; }
	void IncPlayerWipeouts(void) { numPlayerWipeouts++; }
};

#endif INCLUDED_JUDGE_H