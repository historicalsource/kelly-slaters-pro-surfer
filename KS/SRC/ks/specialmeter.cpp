
#include "global.h"
#include "specialmeter.h"

// Default config values - these are overwritten by a data file.
float SpecialMeter::ZONE_BOUNDARY = 0.5f;
float SpecialMeter::DROP_RATE_SPECIAL = 0.01f;
float SpecialMeter::DROP_RATE_SPECIAL_FACE = 0.02f;
float SpecialMeter::DROP_RATE = 0.01f;
float SpecialMeter::PERFECT_BONUS = 5.0f;

//	SpecialMeter()
// Default constructor.
SpecialMeter::SpecialMeter()
{
	playerIdx = 0;
	fillage = 0.0f;
	isEnabled = false;
	trickRegion = TREGION_FACE;
	isDoingTrick = false;
	faceLinkTimer = 0.0f;
	faceLink = 0;
	specialTime = 0.0f;
	numPerfects = 0;
}

//	~SpecialMeter()
// Destructor.
SpecialMeter::~SpecialMeter()
{

}

//	Initialize()
// Sets this meter's player index.
void SpecialMeter::Initialize(const int pIdx)
{
	playerIdx = pIdx;
}

//	Reset()
// Resets the meter to its default state.
void SpecialMeter::Reset(void)
{
	fillage = 0.0f;
	isEnabled = false;
	trickRegion = TREGION_FACE;
	isDoingTrick = false;
	faceLinkTimer = 0.0f;
	faceLink = 0;
	specialTime = 0.0f;
	numPerfects = 0;
}


//	Update()
// Should be called every frame.
void SpecialMeter::Update(const float dt, const TRICKREGION region, const int trickIdx, const int trickCount)
{	
	float	modifiedSpecialTime;
	
	trickRegion = region;
	isDoingTrick = (trickRegion == TREGION_AIR) || (trickRegion == TREGION_TUBE) || (trickIdx != -1);

	if (CanRegionLink())
		current_special_time += dt;
	else
		current_special_time = 0;

	// Drop rate for unenabled meter.
	if (!isEnabled)
	{
		if (region != TREGION_TUBE)
			Decrease(DROP_RATE*dt);
	}
	// Drop rate for enabled meter.
	else
	{
		// Enabled special meter drop rate for air region.
		if (trickRegion == TREGION_AIR)
		{
			// Do not decrase meter while in the air.
		}
		// Enabled special meter drop rate for tube region.
		else if (trickRegion == TREGION_TUBE)
		{
			// Do not decrase meter while in the tube.
		}
		// Enabled special meter drop rate for face region.
		else
		{
			// Decrease meter if doing a special face trick.
			if (trickIdx != -1 && GTrickList[trickIdx].flags & SpecialFlag)
			{				
				Decrease(DROP_RATE_SPECIAL_FACE*dt);
			}
			// Decrease meter if not doing a special face trick.
			else if (!isDoingTrick || trickCount >= 4)
			{
				specialTime += dt;
				
				// Calculate how long meter has been enabled, deducting perfect bonus.
				modifiedSpecialTime = specialTime;
				if (specialTime >= numPerfects*PERFECT_BONUS)
				{
					modifiedSpecialTime -= numPerfects*PERFECT_BONUS;
					if (modifiedSpecialTime < 0.0f)
						modifiedSpecialTime = 0.0f;
				}
				
				Decrease(modifiedSpecialTime*DROP_RATE_SPECIAL);
			}
		}
	}
	
	
	// Decrease face linking timer.
	if (trickRegion == TREGION_FACE)
	{
		if (!isDoingTrick && faceLinkTimer > 0.0f)
		{
			faceLinkTimer -= dt;
			
			// Time ran out to link face tricks.
			if (faceLinkTimer < 0.0f)
			{
				faceLink = 0;
				faceLinkTimer = 0.0f;
			}
		}	
	}

#if defined(DEBUG) && defined(TOBY)
	if (playerIdx == 0)
		frontendmanager.IGO->SetDebugText(stringx(faceLinkTimer));
#endif
}


void SpecialMeter::SetUpSpecialTimer() 
{
	current_special_time = 0;
	//frontendmanager.IGO->TurnOnTubeTimer(0, true);
}


//	OnEvent()
// Responds to all events.
void SpecialMeter::OnEvent(const EVENT event, const int param1, const int param2)
{	
	if (param1 != playerIdx)
		return;
	
	if (event == EVT_TRICK_FACE_BEGIN)
	{
		switch (faceLink)
		{
		case 0 : faceLinkTimer = 1.0f; break;
		case 1 : faceLinkTimer = 0.75f; break;
		case 2 : faceLinkTimer = 0.50f; break;
		case 3 : faceLinkTimer = 0.25f; break;
		case 4 : faceLinkTimer = 0.10f; break;
		case 5 : faceLinkTimer = 0.05f; break;
		default : faceLinkTimer = 0.01f; break;
		}
			
		faceLink++;
	}
	else if (event == EVT_TRICK_FACE_END)
	{
		
	}
	else if (event == EVT_SURFER_LAND)
	{
		if (param2 & 0x01)
			numPerfects++;
		else
			numPerfects = 0;
	}
	else if (event == EVT_SCORING_CHAIN_END)
	{
		numPerfects = 0;
	}
}

//	Increase()
// Increases the meter by the specified amount.
void SpecialMeter::Increase(const float f)
{
	// Breakpoints are fun!
	if (f < 0.0f)
	{
		int i = 0;
		i++;
	}

	if (f > 0.0f)
		SetFillage(fillage+f);
}

//	Decrease()
// Decreases the meter by the specified amount.
void SpecialMeter::Decrease(const float f)
{
	// Breakpoints are fun!
	if (f < 0.0f)
	{
		int i = 0;
		i++;
	}
	
	if (f > 0.0f)
		SetFillage(fillage-f);
}

//	SetFillage()
// Sets how filled the meter is (%).
void SpecialMeter::SetFillage(const float f)
{
	fillage = f;

	if (fillage < ZONE_BOUNDARY)
	{
		specialTime = 0.0f;
		isEnabled = false;
	}

	if (fillage < 0.0f)
		fillage = 0.0f;
	else if (fillage > 1.0f)
		fillage = 1.0f;

	if (fillage == 1.0f)
	{
		if (!isEnabled)
		{
			SoundScriptManager::inst()->playEvent(SS_SPECIAL_MAXED);
		}
		isEnabled = true;
	}
}

//	CanRegionLink()
// Returns true if player can link tricks between regions.
bool SpecialMeter::CanRegionLink(void) const
{
	//return fillage >= ZONE_BOUNDARY;
	return isEnabled;
}

//	CanFaceLink()
// Returns true if player can continue linking face tricks.
bool SpecialMeter::CanFaceLink(void) const
{
	return faceLinkTimer > 0.01f;
}
