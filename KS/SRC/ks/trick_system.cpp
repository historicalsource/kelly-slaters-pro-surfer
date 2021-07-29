//#ifdef _XBOX
#include "global.h"
#include "ngl.h"
#include "inputmgr.h"
#include "conglom.h"
#include "kellyslater_controller.h"
#include "game.h"
#include "FrontEndManager.h"
#include "refptr.h"
#include "hwrasterize.h"
#include "text_parser.h"
#include "file_finder.h"
#include "wds.h"
#include "osdevopts.h"
#include "ini_parser.h"
#include "unlock_manager.h"
//#endif /* TARGET_XBOX JIV DEBUG */


#include "trick_system.h"
#include "inputmgr.h"
#include "trickdata.h"
#define DIR_UP		1
#define DIR_DOWN	2
#define DIR_LEFT  4
#define DIR_RIGHT	8

#define 	MAX_TRICK_QUEUE_TIME	3.0f			// time we can have a trick queued for

// type-flags (aka locational)
const int FaceFlag			= 0x00001;
const int GrindFlag			= 0x00002;	// aka floaters
const int TubeFlag			= 0x00004;
const int AirFlag			= 0x00008;	// needed?

// Extra flags
const int ManualFlag		= 0x00010;
const int NoInterruptFlag	= 0x00020;
const int BalanceBlendFlag	= 0x00040;
const int SpecialFlag		= 0x00080;

// More extra flags
const int TakeoffFlag		= 0x00100;
const int ExitFlag			= 0x00200;
const int SkipFlag			= 0x00400;

// Tube ride flags
const int NormalRideFlag	= 0x01000;
const int ModRideFlag		= 0x02000;

// Scoring flags
const int NoScoreFlag		= 0x10000;
const int NoMultFlag		= 0x20000;

const int NullFlag			= 0x00000;


TrickManager::TrickManager()
{
	int n;
	for (n = 0; n < NUM_BUTTONS; n++)
		CurrentButtonState[n] = false;

	for (n = 0; n < MAX_EVENT; n++)
	{
		mEventQueue[n].Button = 0;
		mEventQueue[n].Pressed = false;
		mEventQueue[n].Time = 0.0f;
	}

	for (n = 0; n < MAX_TRICKS_QUEUED; n++)
	{
		mTrickQueue[n].trick = NULL;
		mTrickQueue[n].Button = 0;
		mTrickQueue[n].EventTime = 0.0f;
		mTrickQueue[n].Time = 0.0f;
	}

	//frames = 0;
	time = 0.0f;
	mTricksQueued = 0;
	mTrickQueueTail = 0;
	mEventQueuePos = 0;
	TimePerFrame = 1.0f/60.0f;
	LastFrameTrickQueued = -1;
	trick_this_frame = false;
    ksctrl=NULL;
}

void TrickManager::ClearTrickManager(void)
{
    assert(ksctrl);
	for (int e=0;e<MAX_EVENT;e++)
	{
		mEventQueue[e].Button = 0;
	}

	for (int n = 0; n < MAX_TRICKS_QUEUED; n++)
	{
		mTrickQueue[n].trick = NULL;
		mTrickQueue[n].Button = 0;
		mTrickQueue[n].EventTime = 0.0f;
		mTrickQueue[n].Time = 0.0f;
	}

	mActionButton = 0;
	mActionButtonEventTime = 0.0f;
	mTricksQueued = 0;
	mTrickQueueTail = 0;
	mEventQueuePos = 0;
}

int TrickManager::GetDirection(void)
{

//	input_mgr* inputmgr = input_mgr::inst();
	float deadzone = 0.4f;
	int Direction = 0;
	float fup = ksctrl->CtrlEvent(PSX_UD);
	if (fup < -deadzone)
		Direction |= DIR_UP;
	else if (fup > deadzone)
		Direction |= DIR_DOWN;

	float flr = ksctrl->CtrlEvent(PSX_LR);
	if (flr < -deadzone)
		Direction |= DIR_LEFT;
	else if (flr > deadzone)
		Direction |= DIR_RIGHT;

   static int  DirectionMap[16] =
   {
      // RLDU
      0,				// 0000
         PAD_U,		// 0001
         PAD_D,		// 0010
         PAD_D,		// 0011	 *
         PAD_L,		// 0100
         PAD_UL,		// 0101
         PAD_DL,		// 0110
         PAD_DL,		// 0111	 *
         PAD_R,		// 1000
         PAD_UR,		// 1001
         PAD_DR,		// 1010
         PAD_DR,		// 1011	 *
         PAD_R,		// 1100	 *
         PAD_UR,		// 1101	 *
         PAD_DR,		// 1110	 *
         PAD_DR,		// 1111	 *
   };

   return DirectionMap[Direction];

}

void TrickManager::ButtonRecord(uint8 button, bool pressed)
{
    assert(ksctrl);
	if (pressed)
		button_pressed_this_frame = true;

	if (CurrentButtonState[button] == pressed)
      return;

   // state has changed, so update it
   CurrentButtonState[button] = pressed;

   // hey, an event has occured, so add it to the event queue

   //	printf("[%d] Added %d, P[%d]\n",mEventQueuePos, button, pressed);

   mEventQueue[mEventQueuePos].Button = button;
   mEventQueue[mEventQueuePos].Pressed = pressed;
   mEventQueue[mEventQueuePos].Time = time;

   //	printf (" **** Ev %d , B %d, P %d, T %d\n",mEventQueuePos,button,pressed,frames);

   mEventQueuePos++;
   if (mEventQueuePos == MAX_EVENT)
      mEventQueuePos = 0;
}

void TrickManager::UpdateButtons(void)
{
   assert(ksctrl);
//   input_mgr* inputmgr = input_mgr::inst();

	// Update the direction buttons
   int Direction = GetDirection();

   ButtonRecord(PAD_U,Direction==PAD_U);
   ButtonRecord(PAD_D,Direction==PAD_D);
   ButtonRecord(PAD_L,Direction==PAD_L);
   ButtonRecord(PAD_R,Direction==PAD_R);
   ButtonRecord(PAD_UL,Direction==PAD_UL);
   ButtonRecord(PAD_UR,Direction==PAD_UR);
   ButtonRecord(PAD_DL,Direction==PAD_DL);
   ButtonRecord(PAD_DR,Direction==PAD_DR);

   // Update the action buttons
   ButtonRecord(PAD_CIRCLE, ksctrl->CtrlEvent(PSX_CIRCLE));
   ButtonRecord(PAD_SQUARE, ksctrl->CtrlEvent(PSX_SQUARE));
   ButtonRecord(PAD_TRIANGLE, ksctrl->CtrlEvent(PSX_TRIANGLE));
   ButtonRecord(PAD_CROSS, ksctrl->CtrlEvent(PSX_X));

   // Update the shoulder buttons

   ButtonRecord(PAD_L1, ksctrl->CtrlEvent(PSX_L1));
   ButtonRecord(PAD_L2, ksctrl->CtrlEvent(PSX_L2));
   ButtonRecord(PAD_R1, ksctrl->CtrlEvent(PSX_R1));
   ButtonRecord(PAD_R2, ksctrl->CtrlEvent(PSX_R2));

}


// Return true if the action button has been held continuosly since it was pressed
// we search backwards through the queue, looking for a "Not Pressed" event that
// is newer than "Pressed" event that we have recorded as being the one that triggered the last move
bool TrickManager::ActionButtonHeld()
{
   if (mActionButton == 0)
	   return false;

   int Event = mEventQueuePos-1;				// Start on the last event triggered
   if (Event < 0)
      Event = MAX_EVENT-1;
   while (Event != mEventQueuePos)			// Traverse back through the entire queue
   {
      // if it's a released event for the action button since the action button pressed event time
      // then we've not held the button the whole time, so we can't
      if ( (mEventQueue[Event].Button == mActionButton) && !mEventQueue[Event].Pressed && mEventQueue[Event].Time > mActionButtonEventTime)
      {
         return false;
      }

      Event--;
      if (Event < 0)
         Event = MAX_EVENT-1;
   }
   return true;
}


// find the last event that is "Skip" events before the event numbered "Event"
// that happend within "Time" frames.
// return -1 if none found
int TrickManager::GetPreviousPressedEvent(int Event, uint8 Skip, float Time, uint8 Top, bool IgnoreDiagonals)
{
   Event -= Skip;														// skip any that are indicated
   if (Event < 0)														// might need to wrap around
      Event += MAX_EVENT;
   while (1)
   {
      if (mEventQueue[Event].Button)							// must be a valid button
      {
         if (!IgnoreDiagonals
            || (mEventQueue[Event].Button != PAD_UR
            && mEventQueue[Event].Button != PAD_UL
            && mEventQueue[Event].Button != PAD_DL
            && mEventQueue[Event].Button != PAD_DR
            )
            )
         {

            if (time - mEventQueue[Event].Time > Time)	// found an event that is too old
            {
               return -1;												// so none found
            }
            if (mEventQueue[Event].Pressed)						// found a valid pressed event
            {
               return Event;											// so return it
            }
         }
      }
      Event--;															// next event
      if (Event < 0)													// might need to wrap around
         Event = MAX_EVENT-1;
      if (Event == Top)												// if back to start
      {
         return -1;													// then none found
      }
   }
}


// search back through the queue to find an event that matches this
// then remove it from the queue
void TrickManager::ButtonRemoveLast(uint8 Button, bool Pressed)
{
   int Event = mEventQueuePos-1;				// Start on the last event triggered
   if (Event < 0)
      Event = MAX_EVENT-1;
   while (Event != mEventQueuePos)			// Traverse back through the entire queue
   {
      if ( (mEventQueue[Event].Button == Button) && mEventQueue[Event].Pressed == Pressed)
      {
         //			printf("[%d] removed %d\n",Event,Button);
         mEventQueue[Event].Button = 0;
         return;
      }
      Event--;
      if (Event < 0)
         Event = MAX_EVENT-1;
   }
}

void TrickManager::QueueTrick(SurferTrick *trick, uint8 Button, float EventTime)
{
  assert(ksctrl);
  if (!trick || (mTricksQueued == MAX_TRICKS_QUEUED)
	  || ((trick->flags & ManualFlag) && (trick->trick_id == LastFrameTrickQueued)))
  {
    return;
  }

  trick_this_frame = true;
  if (trick->flags & GrindFlag)
  {
	ksctrl->SetFloatTrick(trick->trick_id, trick->anim_id, trick->board_anim_id);
	mActionButtonEventTime = EventTime;
	mActionButton = Button;
  }
  else if (trick->flags & TubeFlag && ksctrl->get_board_controller().GetWaveRegion() == WAVE_REGIONTUBE &&
            ksctrl->get_current_state() != STATE_TUBE_TRICK)
  {
	ksctrl->SetTubeTrick(trick->trick_id, trick->anim_id, trick->board_anim_id);
	mActionButtonEventTime = EventTime;
	mActionButton = Button;
  }
  else if (trick->flags)
  {
	int Head = (mTrickQueueTail + mTricksQueued) % MAX_TRICKS_QUEUED;
	mTrickQueue[Head].trick = trick;
	mTrickQueue[Head].Time = time;
	mTrickQueue[Head].EventTime = EventTime;
	mTrickQueue[Head].Button = Button;
	mTricksQueued++;
	LastFrameTrickQueued = trick->trick_id;
  }
}


void TrickManager::ProcessEvents(void)
{
	int	trickListIdx;
	int	trickBookIdx;

	bool all_tricks = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_ALL_TRICKS);
	for (trickBookIdx = 0; trickBookIdx < (all_tricks?TRICK_NUM:TRICKBOOK_SIZE); trickBookIdx++)
	{
		if (all_tricks)
		{
			trickListIdx = trickBookIdx;
			if ((GTrickList[trickListIdx].button1 == PAD_NONE) || (GTrickList[trickListIdx].flags & SkipFlag))
				continue;

			// Special tricks are only allowed if the sick meter is high enough.
			if ((GTrickList[trickListIdx].flags & SpecialFlag) && (!ksctrl->get_special_meter()->CanRegionLink()))
				continue;
		}
		else
		{
			trickListIdx = SurferDataArray[g_game_ptr->GetSurferIdx(ksctrl->get_player_num())].trickBook[trickBookIdx];

			// Special tricks are only allowed if the sick meter is high enough.
			if ((GTrickList[trickListIdx].flags & SpecialFlag) && (!ksctrl->get_special_meter()->CanRegionLink()))
				continue;

			//  check the learned tricks to see if this is one and it is not open yet.
			if (GTrickList[trickListIdx].flags & SpecialFlag && 
				!unlockManager.isSurferTrickUnlocked(trickListIdx))
				continue;
		}

		// Ignore non-implemented tricks.
		if ((GTrickList[trickListIdx].button1 == PAD_NONE) || (GTrickList[trickListIdx].flags & SkipFlag))
			continue;

		if ((GTrickList[trickListIdx].trick_type == TRICKTYPE_EXIT) && 
				(!ksctrl->get_board_controller().InAir() || !ksctrl->get_board_controller().exit_jump))
				continue;

		if ((GTrickList[trickListIdx].trick_type == TRICKTYPE_TUBE)
			&& (ksctrl->get_board_controller().get_ks_controller()->get_super_state() != SUPER_STATE_IN_TUBE))
			continue;

		if ((GTrickList[trickListIdx].trick_type != TRICKTYPE_TUBE)
			&& (ksctrl->get_board_controller().get_ks_controller()->get_super_state() == SUPER_STATE_IN_TUBE))
			continue;

		if ((GTrickList[trickListIdx].trick_type == TRICKTYPE_FLOATER)
			&& !((ksctrl->get_current_state() == STATE_GRIND)
			|| (ksctrl->get_current_state() == STATE_FLOAT)
			|| ksctrl->get_board_controller().InAir()
			|| ksctrl->get_board_controller().IsGrindingObject()))
			continue;

		if (((GTrickList[trickListIdx].trick_type == TRICKTYPE_FACE)
			|| (GTrickList[trickListIdx].trick_type == TRICKTYPE_TUBE)
			|| (GTrickList[trickListIdx].trick_type == TRICKTYPE_TUBE_AND_OUT))
			&& ksctrl->get_board_controller().InAir())
			continue;

		if (((GTrickList[trickListIdx].trick_type == TRICKTYPE_AERIAL) ||
			(GTrickList[trickListIdx].trick_type == TRICKTYPE_GRAB)) &&
			!ksctrl->get_board_controller().InAir())
			continue;

		uint8 num_buttons = (GTrickList[trickListIdx].button3 == PAD_NONE)?2:3;
		uint8 ActionButton;
		uint8 Button1;
		if (num_buttons == 2)
		{
			Button1 = GTrickList[trickListIdx].button1;
			ActionButton = GTrickList[trickListIdx].button2;
		}
		else
		{
			Button1 = GTrickList[trickListIdx].button2;
			ActionButton = GTrickList[trickListIdx].button3;
		}

        int LastEvent = mEventQueuePos-1;							// Start on the last event triggered
        if (LastEvent < 0)												// might need to wrap around
			LastEvent = MAX_EVENT-1;
        float ActionButtonEventTime;							// time at which the action button was pressed
		int IgnoreDiagonals = false;
		float Time = TimePerFrame*(GTrickList[trickListIdx].Time);
		uint8 Flags = GTrickList[trickListIdx].flags;

		int Manual = Flags & ManualFlag;
		int Grind = Flags & GrindFlag;

		if (num_buttons == 2 						  					// two buttons
            && (Button1>=PAD_U && Button1<=PAD_DR) 			// previous  button is a direction
            && (ActionButton<PAD_U || ActionButton>PAD_DR) // action button is NOT a direction
            )

         {
            // Need to find if the direction button is held
            // and if the other button was pressed in the time available


            // there must have been at least one event, so get it
            int Event2,Direction;
            int Event1 = GetPreviousPressedEvent(LastEvent,0,Time,LastEvent,IgnoreDiagonals);		// note use of 0, so we don't skip last event
            if (Event1 == -1)		  // if no events at all, then no trick
               continue;
            // there might be another event, so get it, as we mgiht need to deal with it
            Event2 = GetPreviousPressedEvent(Event1,1,Time,LastEvent,IgnoreDiagonals);

            Direction = GetDirection();			// get currently HELD direction, as we might need it
            //printf("E1-%d,E2-%d,Dir-%d  B1-%d, Act-%d\n",Event1,Event2,Direction,Button1,ActionButton);
            if (Direction == 0 && Event2 == -1)
               continue;


            // Now, either
            //  there have been two events, and they match the requested buttons (2 cases)
            // or
            //  there where two events, one matches the action button, and the direction is held (2 cases)
            // or
            // there was one event, which matches the action button, and the direction is held (2 cases)
            if (Event2 != -1)	 // check fro two events
            {
               if (mEventQueue[Event1].Button == Button1 && mEventQueue[Event2].Button == ActionButton)
               {

                  // two matching events, with action button being event2
                  ActionButtonEventTime = mEventQueue[Event2].Time;
                  if (!Manual && !Grind)
                     ButtonRemoveLast(Button1,1);
               }
               else if (mEventQueue[Event2].Button == Button1 && mEventQueue[Event1].Button == ActionButton)
               {
                  // two matching events, with action button being event1
                  if (!Manual && !Grind)
                     ButtonRemoveLast(Button1,1);
                  ActionButtonEventTime = mEventQueue[Event1].Time;
               }
               /*else if (Direction == Button1 && mEventQueue[Event1].Button == ActionButton)
               {
                  // two events but only with action button being event1, yet direction is held down
                  ActionButtonEventTime = mEventQueue[Event1].Time;
               }
               else if (Direction == Button1 && mEventQueue[Event2].Button == ActionButton)
               {
                  // two events but only with action button being event2, yet direction is held down
                  ActionButtonEventTime = mEventQueue[Event2].Time;
               }*/
               else continue;
            }
            else if (Direction == Button1 && mEventQueue[Event1].Button == ActionButton)
            {
               // one events, with action button being event1, yet direction is held down
               ActionButtonEventTime = mEventQueue[Event1].Time;
            }
			else if (Direction == Button1 && (Grind) && (mActionButton == ActionButton) && ActionButtonHeld())
			{
				ActionButtonEventTime = time;
			}
            else continue;

            if (!Manual && !Grind)
               ButtonRemoveLast(ActionButton,1);
            QueueTrick(&GTrickList[trickListIdx], ActionButton,ActionButtonEventTime);
			break;
         }
		else	// not two buttons starting with a direction, so check the more general case
         {
            // Going backwards through the list and the queue at the same time
            // find events that match up, within the time allowed, with no interspersed events
            // the first two can be in any order

#define ISDIR(x) ((x)>=PAD_U && (x)<=PAD_DR)

            // set "SameType" if the last two buttons are of the same type
            // like two directions, or two STCX buttons
//            int SameType = (ISDIR(ActionButton) && ISDIR(Button1) || (!ISDIR(ActionButton) && !ISDIR(Button1)));

            // get the most recent "pressed" event
            // if there are none, then nothing doing
            int Event1 = GetPreviousPressedEvent(LastEvent,0,Time,LastEvent,IgnoreDiagonals);		// note use of 0, so we don't skip last event
            if (Event1 == -1)
               break;			// note break here, as if there are no events at all, then we can't possibly trick
            int Event2 = GetPreviousPressedEvent(Event1,1,Time,LastEvent,IgnoreDiagonals);
            if (Event2 == -1)
               continue;		// but continue here, as we we need to still check for the direction tricks which can have jsut one event
#define MAXOTHERBUTTONS	10				// stupifilingly unlikely that we would need this many, but safe even if we go over.
            int OtherButton[MAXOTHERBUTTONS];
            int OtherButtonIndex = 0;

            // check last two buttons matching
            // if not same type, then allow in either order

            if (    (mEventQueue[Event2].Button == Button1 && mEventQueue[Event1].Button == ActionButton)
               /*|| (!SameType && mEventQueue[Event1].Button == Button1 && mEventQueue[Event2].Button == ActionButton)*/  )
            {

               // get the time of most recent event that is the action button (will be Event1 or Event2).
               // note - this correctly handles the case of double taps commands.
               if (mEventQueue[Event1].Button == ActionButton)
                  ActionButtonEventTime = mEventQueue[Event1].Time;
               else
                  ActionButtonEventTime = mEventQueue[Event2].Time;

               // now we know the last two pressed events were the last two buttons of the sequence
               // so go through all the previous pressed events
               int ButtonsLeft = num_buttons - 3;
               int EventX = Event2;
               while (ButtonsLeft >= 0)
               {
                  //uint8 ButtonX = GTrickList[trickListIdx].node->buttons[ButtonsLeft];
				  uint8 ButtonX = GTrickList[trickListIdx].button1;  //  this is a hack,  fixme
                  EventX = GetPreviousPressedEvent(EventX,1,Time,LastEvent,IgnoreDiagonals);
                  if (EventX != -1 && mEventQueue[EventX].Button == ButtonX)
                  {
                     // record all additional buttons that had a "pressed" event, so we can remove them later
                     if (OtherButtonIndex<MAXOTHERBUTTONS)
                        OtherButton[OtherButtonIndex++] = ButtonX;
                     ButtonsLeft--;
                  }
                  else
                     break;
               }

               if (ButtonsLeft < 0)
               {
                  // we have got a trick!!!
                  // remove all "pressed" events buttons (that counted) from the queue
                  if (!Manual && !Grind)
                  {
                     ButtonRemoveLast(Button1,1);
                     ButtonRemoveLast(ActionButton,1);
                     for (int i=0;i<OtherButtonIndex;i++)
                     {
                        ButtonRemoveLast(OtherButton[trickListIdx],1);
                     }
                  }
                  QueueTrick(&GTrickList[trickListIdx], ActionButton,ActionButtonEventTime);
                  break;
               }
            }
         }
	}
}

void TrickManager::UpdateQueue(void)
{
	if (!ksctrl)
	{
		mTricksQueued = 0;
		return;
	}

	if (!mTricksQueued || ksctrl->IsDoingSpecialTrick()) // if there are no tricks queued or a special trick in progress
		return;

	// check to see if the oldest trick was recent enough
   if (time - mTrickQueue[mTrickQueueTail].Time < MAX_TRICK_QUEUE_TIME)
   {
      // get the button that was pressed, and when it was pressed
      // so later we can see if it has been released any time
      // since it has been pressed (See ActionButtonHeld(), below).
      mActionButton = mTrickQueue[mTrickQueueTail].Button;
      mActionButtonEventTime = mTrickQueue[mTrickQueueTail].EventTime;
      // Now we've triggered a trick, we cna discard any "pressed" event from the
      // button queue that is older then this
      // this will stop you from pre-queuing manuals
      for (int e=0;e<MAX_EVENT;e++)
      {
		 if (mEventQueue[e].Pressed && (mEventQueue[e].Button != 0) && (mEventQueue[e].Button != mActionButton)
								/*&& (mEventQueue[e].Time <= mActionButtonEventTime)*/) 
         {
            mEventQueue[e].Button = 0;
			mEventQueue[e].Pressed = false;
         }
      }
	  if (mTrickQueue[mTrickQueueTail].trick->flags & NoInterruptFlag)
		  ksctrl->SetDoingSpecialTrick(true);

	  ksctrl->SetNewTrick(mTrickQueue[mTrickQueueTail].trick->trick_id);
   }
   // always kill the oldest trick, either we just executed it, or it was too old.
   mTrickQueueTail++;
   mTrickQueueTail %= MAX_TRICKS_QUEUED;
   mTricksQueued--;
}


void TrickManager::FrameAdvance(float dt)
{
	trick_this_frame = false;
	button_pressed_this_frame = false;
	UpdateButtons();
	ProcessEvents();
	UpdateQueue();
	//frames++;
	time += dt;
	if (trick_this_frame)
		LastFrameTrickQueued = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									  //
//								Carving Manager Class:  this class parses surfer path info			  //
//														to determine carve tricks					  //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////


void CarveManager::SetInternalVars()
{
  ClearCarveInfo();
  if (ksctrl)
  {
    cur_board_vec = ksctrl->get_board_controller().GetForwardDir();
    last_board_vec = cur_board_vec;
  }

  TurnType = REGULAR_TURN;
  angle = 0.0f;
}

CarveManager::CarveManager()
{
  ksctrl=NULL;
  SetInternalVars();
}


void CarveManager::FrameAdvance(float dt, bool force_update)
{
  float record_delta = 0.075f;
  int state = ksctrl->get_current_state();
  bool correct_state = ((state == STATE_LEFTTURN) || (state == STATE_RIGHTTURN) || (state == STATE_STAND));
  int tType = ksctrl->get_board_controller().GetTurnType();
  if ((TurnType != CARVE_TURN) && (tType == CARVE_TURN) && correct_state)
    SetInternalVars();
  else if ((TurnType != GRAB_TURN) && (tType == GRAB_TURN) && correct_state)
	  SetInternalVars();
  else if (!correct_state || ((tType != CARVE_TURN) && (tType != GRAB_TURN)))
    return;

  TurnType = tType;
  last_board_vec = cur_board_vec;
  cur_board_vec = ksctrl->get_board_controller().GetForwardDir();
  int sign = CarveArray[Tail].state = ksctrl->get_board_controller().GetBoardState();
  float f_sign = (sign == BOARD_TURN_LEFT?1.0f:-1.0f);
  float dot_p = dot(cur_board_vec, last_board_vec);
  if (dot_p > 1.0f)
    dot_p = 1.0f;
  else if (dot_p < -1.0f)
    dot_p = -1.0f;

  angle += f_sign*fast_acos(dot_p);

  time += dt;
  if (force_update || ((time - last_record_time) > record_delta))
  {
    last_record_time = time;
    CarveArray[Tail].board_vec = cur_board_vec;
    CarveArray[Tail].position = ksctrl->get_board_controller().my_board->get_abs_position();
    CarveArray[Tail].time = time;
    CarveArray[Tail].state = ksctrl->get_board_controller().GetBoardState();
    CarveArray[Tail].angle = angle;
    CarveArray[Tail].region = ksctrl->get_board_controller().GetWaveRegion();

    ParseCarveInfo();
  }
}


void CarveManager::ParseCarveInfo()
{
	int index = -1;	// otherwise uninitialized (dc 01/29/02)
	int end = Tail;
	int start = Head;

	float delta_angle;
	float delta_time;
//	bool lip_carve = false;
//	bool tube_carve = false;
	bool possible_trick = false;

	float trick_time = 1.0f;
	if (TurnType == GRAB_TURN)
		trick_time = 0.7f;

	float trick_angle = 2.5f;

	// first determine head of list
	for (int n = end; n != start; n--)
	{
		delta_time = CarveArray[end].time - CarveArray[n].time;
		if (delta_time > trick_time)
		{
			start = Head = n;
			break;
		}

		if (n == 0)
			n = MAX_CARVE_FRAMES - 1;
	}

	hardest_region = WAVE_REGIONPOCKET;
	for (int n = end; n != start; n--)
	{
		if ((hardest_region == WAVE_REGIONPOCKET) && ((CarveArray[n].region == WAVE_REGIONFACE) ||
			(CarveArray[n].region == WAVE_REGIONCHIN) || (CarveArray[n].region == WAVE_REGIONLIP) ||
			(CarveArray[n].region == WAVE_REGIONLIP2)))
		{
			hardest_region = CarveArray[n].region;
		}
		else if ((hardest_region == WAVE_REGIONFACE) && ((CarveArray[n].region == WAVE_REGIONCHIN) ||
			(CarveArray[n].region == WAVE_REGIONLIP) || (CarveArray[n].region == WAVE_REGIONLIP2)))
		{
			hardest_region = CarveArray[n].region;
		}

		/*if (tube_dist_set)
		{
			if (right_break && (CarveArray[n].position.x > x_coord))
				x_coord = CarveArray[n].position.x;
			else if (CarveArray[n].position.x < x_coord)
				x_coord = CarveArray[n].position.x;
		}
		else
			x_coord = CarveArray[n].position.x;*/

		delta_angle = fabs(CarveArray[end].angle - CarveArray[n].angle);
		//delta_angle = dot(CarveArray[end].board_vec, CarveArray[n].board_vec);
		//if (delta_angle < trick_angle)
		if (delta_angle > trick_angle)
		{
			index = n;
			possible_trick = true;
			break;
		}

		if (n == 0)
			n = MAX_CARVE_FRAMES - 1;
	}

	if (possible_trick)
	{
		assert(index != -1);	// check that it got initialized (dc 01/29/02)

		DetermineTrick(index, end);
		SetInternalVars();
		return;
	}

	Tail++;
	if (Tail == MAX_CARVE_FRAMES)
		Tail = 0;
}

void CarveManager::DetermineTrick(int start, int end)
{
//	float angle = CarveArray[end].angle - CarveArray[start].angle;
	vector3d vec = CarveArray[start].board_vec;
	vec -= YVEC*dot(YVEC, vec);
	vec.normalize();

    assert(ksctrl);

	if (dot(vec, ZVEC) > 0.707f)
	{
		ksctrl->get_my_scoreManager().AddTrick((TurnType == GRAB_TURN)?TRICK_TOPGRAB:TRICK_TOPCARVE);
	}
	else if (fabs(dot(vec, XVEC)) > 0.707)
	{
		ksctrl->get_my_scoreManager().AddTrick((TurnType == GRAB_TURN)?TRICK_GRABROUNDHOUSE:TRICK_ROUNDHOUSECARVE);
	}
	else if (dot(vec, -ZVEC) > 0.707f)
	{
		ksctrl->get_my_scoreManager().AddTrick((TurnType == GRAB_TURN)?TRICK_BOTTOMGRAB:TRICK_BOTTOMCARVE);
	}

	float	vert_mod;
	if ((hardest_region == WAVE_REGIONLIP) || (hardest_region == WAVE_REGIONLIP2) || (hardest_region == WAVE_REGIONCHIN))
		vert_mod = 1.0f;
	else if (hardest_region == WAVE_REGIONFACE)
		vert_mod = 0.5f;
	else if (hardest_region == WAVE_REGIONPOCKET)
		vert_mod = 0.0f;
	else
		{ assert(0); return; }	// vert_mod was uninitialized (dc 01/29/02)

	ksctrl->get_my_scoreManager().UpdateLastTrick(ScoringManager::ATTR_DIST_LIP, vert_mod);

	float scale;
	float tube_dist = ksctrl->Tube_Distance();
	if (tube_dist < -0.2f)
		scale = 1.0f;
	else
		scale = 0.8f - tube_dist;

	if (scale < 0.0f)
		scale = 0.0f;
	else if (scale > 1.0f)
		scale = 1.0f;
}
