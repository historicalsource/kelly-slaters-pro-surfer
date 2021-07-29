
#ifndef INCLUDED_HINTMANAGER_H
#define INCLUDED_HINTMANAGER_H

#include "global.h"
#include "ngl.h"
#include "FEPanel.h"
#include "eventmanager.h"

#define MAX_HINTS 15
#define MAX_HINT_APPEARANCES 2

// IGOHintManager: Tells the user useful things, like why he crashed.
class IGOHintManager
{
private:

	stringx hint_text;
	TextString *instruction_text;
	float current_hint_time;
	int current_hint;
	bool competition_help;

	int hint_use[MAX_HINTS];

public:

	enum
	{
		StandUp,
		ObjectApproaching,
		CompetitionNeedFace,
		CompetitionNeedTube,
		CompetitionNeedAir,
		LandedSideways,
		SurfingBackwards,
		LostBalance,
		TooHighOnTubeWall,
		LandedDuringTrick,
		StoodTooFarOut,
		DidNotStand,
		TryPaddlingForward,
		HitTubeWall,
		SpunTooMuch,
		NUM_HINTS
	};

	// Creators.
	IGOHintManager();
	~IGOHintManager();

	// Modifiers.
	void Reset();
	void Update(const float dt);
	void Draw(void);
	void process_help_string(stringx &dest, const char *source);

	
	// Accessors.
	void SetHint(int new_hint);
};

#endif INCLUDED_HINTMANAGER_H