
#ifndef INCLUDED_MODE_METERATTACK_H
#define INCLUDED_MODE_METERATTACK_H

#include "global.h"
#include "mode.h"
#include "kellyslater_controller.h"

// MeterAttackMode class - manages the state for Meter Attack multiplayer mode.
class MeterAttackMode
{
public:
	static float TIME_INITIAL;
	static int TOLERANCE;
	static int SCORE_DROP_SPEED;
	static int SCORE_ATTACK_STRENGTH;
	static float TIME_LOSE;

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

protected:
	void Attack(const int playerIdx, const float dt);

public:
	// Creators.
	MeterAttackMode();
	~MeterAttackMode();

	// Modifiers.
	void Initialize(kellyslater_controller ** controllers);
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
	int GetActivePlayerIdx(void) const { return setNum; }
};

#endif INCLUDED_MODE_METERATTACK_H