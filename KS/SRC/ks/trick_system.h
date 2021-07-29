#ifndef _TRICK_SYS_H
#define _TRICK_SYS_H

#include "types.h"
#include "trickdata.h"
#include "VOEngine.h"		// YES.. KEVIN HAS INVADED THIS FILE!  MUAH HA HA!

class kellyslater_controller;

struct TrickEvent
{
	SurferTrick *trick;
	uint8 Button;
	float EventTime;
	float Time;
};

struct ButtonEvent
{
	uint8 Button;
	bool Pressed;
	float Time;
};

#define	MAX_EVENT 	32
#define  MAX_TRICKS_QUEUED     10
#define NUM_BUTTONS	20

// type-flags (aka locational)
extern const int FaceFlag			;
extern const int GrindFlag			;	// aka floaters
extern const int TubeFlag			;
extern const int AirFlag			;	// needed?
extern const int ExitFlag			;

// Extra flags
extern const int ManualFlag			;
extern const int NoInterruptFlag	;
extern const int BalanceBlendFlag	;
extern const int SpecialFlag		;

// More extra flags
extern const int TakeoffFlag		;

// Tube ride flags
extern const int NormalRideFlag		;
extern const int ModRideFlag		;

// Scoring flags
extern const int NoScoreFlag		;
extern const int NoMultFlag			;
extern const int NullFlag			;


class TrickManager
{
private:
	void ProcessEvents(void);  //   decides if a trick was entered at a valid time
	void QueueTrick(SurferTrick *trick, uint8 Button, float EventTime);
	void UpdateQueue(void);
	void UpdateButtons(void);
	void ButtonRecord(uint8 button, bool pressed);
	void ButtonRemoveLast(uint8 Button, bool Pressed);
	int GetPreviousPressedEvent(int Event, uint8 Skip, float Time, uint8 Top, bool IgnoreDiagonals = false);
	int GetDirection(void);

	ButtonEvent	mEventQueue[MAX_EVENT];
	TrickEvent	mTrickQueue[MAX_TRICKS_QUEUED];
	bool CurrentButtonState[NUM_BUTTONS];  //  we really only use 1 - 16 inclusive

	//int frames;		//  frames of non-paused game, may reset on wipeout
	float time;		//  frames of non-paused game, may reset on wipeout
	int mTricksQueued;
	int mTrickQueueTail;
	int mEventQueuePos;

	float mActionButtonEventTime;
	uint8 mActionButton;

	int LastFrameTrickQueued;
	float TimePerFrame;
	bool trick_this_frame;  //  this is true whena trick has been queued this frame;
	bool button_pressed_this_frame;

    kellyslater_controller *ksctrl;     // my owner

public:
	TrickManager();
	void ManuallyQueueTrick(SurferTrick *trick) {QueueTrick( trick, 0, 0);}
	void ClearTrickManager(void);
	void FrameAdvance(float dt);
	bool ActionButtonHeld(void);
	bool ActionButtonNonZero(void) { return (mActionButton != 0); }
    void set_ksctrl(kellyslater_controller *kscptr)   {ksctrl=kscptr;}
	bool ButtonPressed(void) { return button_pressed_this_frame; }
};

#define MAX_CARVE_FRAMES 20

class CarveManager
{

private:

	void ParseCarveInfo();
	void DetermineTrick(int start, int end);

	struct CarveNode
	{
		float time;
		float angle;
		vector3d board_vec;
		vector3d position;
		uint8 state;
		int region;
	};

	float board_angle;
	vector3d cur_board_vec;
	vector3d last_board_vec;

	int TurnType;
	int Head;
	int Tail;

	float angle;
	float time;
	float last_record_time;

	int hardest_region;
	//float x_coord;  //  this is determine distance from tube

    kellyslater_controller *ksctrl;

public:
	CarveManager();
	void FrameAdvance(float dt, bool force_update = false);
	void ClearCarveInfo()
	{
		Head = Tail = 0;
		last_record_time = time = 0.0f;
	}

	void SetInternalVars();

	CarveNode CarveArray[MAX_CARVE_FRAMES];
    void set_ksctrl(kellyslater_controller *kscptr)   {ksctrl=kscptr;}

};


#endif  //  _TRICK_SYS_H
