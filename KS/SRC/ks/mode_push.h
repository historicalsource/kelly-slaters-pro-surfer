
#ifndef INCLUDED_MODE_PUSH_H
#define INCLUDED_MODE_PUSH_H

#include "global.h"
#include "mode.h"
#include "kellyslater_controller.h"

// PushMode class - manages the state for Push multiplayer mode.
class PushMode
{
protected:
	static const int SCORE_REQUIRED;
	static const float SHARE_DELTA;
	static const float SHARE_MAX;
	static const float TIME_PUSH_EFFECT;
	static const int MAX_TIE_SHARES;

protected:
	struct PLAYER
	{
		float						share;		// percentage of screen ownership, total of all players = 1.0
		int							extraShare;	// leftovers not added to share yet
		recti						viewport;
		kellyslater_controller *	ks;
		int							prevScore;
	};

protected:
	PLAYER	players[MAX_PLAYERS];
	float	attackTimer;
	int		scoreRequired;

protected:
	void CalcExtraShares(const int playerIdx);
	bool Attack(const int playerIdx);
	void CalcViewports(void);

public:
	// Creators.
	PushMode();
	~PushMode();

	// Modifiers.
	void Initialize(kellyslater_controller ** controllers);
	void SetDifficulty(const int points);
	void Update(const float dt);
	void FinishCombat(void);

	// Accessors.
	float GetPlayerShare(const int playerIdx) const;
	const recti & GetPlayerViewport(const int playerIdx) const;
	bool InCombat(void) const;
};

#endif INCLUDED_MODE_PUSH_H