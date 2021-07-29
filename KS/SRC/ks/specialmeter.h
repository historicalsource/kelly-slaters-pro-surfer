
#ifndef INCLUDED_SPECIALMETER_H
#define INCLUDED_SPECIALMETER_H

#include "global.h"
#include "eventmanager.h"
#include "trickregion.h"

// SpecialMeter class: functionality of a player's trick linking meter
class SpecialMeter : public EventRecipient
{
public:
	static float ZONE_BOUNDARY;
	static float DROP_RATE;
	static float DROP_RATE_SPECIAL;
	static float DROP_RATE_SPECIAL_FACE;
	static float PERFECT_BONUS;

private:
	int			playerIdx;
	float		fillage;			// percent of meter filled
	bool		isEnabled;
	float		specialTime;
	TRICKREGION	trickRegion;
	bool		isDoingTrick;
	int			faceLink;
	float		faceLinkTimer;
	float		current_special_time;
	int			numPerfects;			// number of consecutive perfects in the chain

public:
	// Creators.
	SpecialMeter();
	virtual ~SpecialMeter();

	// Modifiers.
	void Initialize(const int pIdx);
	void Reset(void);
	void Update(const float dt, const TRICKREGION region, const int trickIdx, const int trickCount);
	void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);
	void SetFillage(const float f);
	void Increase(const float f);
	void Decrease(const float f);
	void SetUpSpecialTimer();
	float GetCurrentSpecialTime() {return current_special_time;}

	// Accessors.
	bool CanRegionLink(void) const;
	bool CanFaceLink(void) const;
	float GetFillage(void) const { return fillage; }
};

#endif INCLUDED_SPECIALMETER_H