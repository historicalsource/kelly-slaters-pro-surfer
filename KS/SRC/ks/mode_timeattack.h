
#ifndef INCLUDED_MODE_TIMEATTACK_H
#define INCLUDED_MODE_TIMEATTACK_H

#include "global.h"
#include "mode.h"
#include "kellyslater_controller.h"

// TimeAttackMode class - manages the state for Time Attack multiplayer mode.
class TimeAttackMode
{
public:
	static float	TIME_INITIAL;
	static int		TOLERANCE;
	static float	TIME_LOSE;

protected:
	struct PLAYER
	{	
		kellyslater_controller *	ks;
		float						time;
		int							prevScore;
		int							score;
		bool						attacking;
		int							attackScore;
	};

protected:
	PLAYER	players[MAX_PLAYERS];
	int		gameNum, setNum;
	int		scoreAttackStrength;
	int		scoreDropSpeed;

protected:
	void Attack(const int playerIdx, const float dt);

public:
	// Creators.
	TimeAttackMode();
	~TimeAttackMode();

	// Modifiers.
	void Initialize(kellyslater_controller ** controllers);
	void SetDifficulty(const int points);
	void Update(const float dt);
	void AdvanceSet(void);
	void Reset(void);
	void BeginCombat(void);
	void BeginAttacking(const int playerIdx);
	void FinishAttacking(const int playerIdx);

	// Accessors.
	float GetLevelDuration(const int playerIdx) const;
	float GetRemainingTime(const int playerIdx) const;
	int GetScore(const int playerIdx) const;
	int GetGameNum(void) const { return gameNum; }
	int GetSetNum(void) const { return setNum; }
	bool IsAttacking(const int playerIdx) const;
	bool IsDoneAttacking(const int playerIdx) const;
	float GetAttackSec(const int playerIdx) const;
	int GetActivePlayerIdx(void) const { return setNum; }
};

#endif INCLUDED_MODE_TIMEATTACK_H