
#include "global.h"
#include "globaltextenum.h"
#include "tutorialmanager.h"
#include "tutorialdata.h"
#include "trickdata.h"
#include "wds.h"

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
//#endif /* TARGET_XBOX JIV DEBUG */


#include "trick_system.h"
#include "inputmgr.h"
#include "trickdata.h"

#define FIRST_SET_END 8
#define SECOND_SET_END 30


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOTutorialManager class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	IGOTutorialManager()
// Default constructor.
IGOTutorialManager::IGOTutorialManager()
{
	//  Load all the VO
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		for (int index = 0; index < Tutorial_Step_Num; index++)
		{
			if (strcmp(GTutorialStep[index].vo_name_string, "TUTORIAL_NONE"))
			{
				nslSourceId s = nslLoadSource(GTutorialStep[index].vo_name_string);
				if (s == NSL_INVALID_ID)
					debug_print("FIXME: MISSING %s SOUND\n", GTutorialStep[index].vo_name_string);
			}
		}

	process_help_string(ins_text, ksGlobalTextArray[GT_TUTORIAL_PRESS_START].c_str());
	instruction_text = NEW BoxText(&frontendmanager.font_info, ins_text, 40, 40, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color32(255, 255, 125, 255), 15);
	instruction_text->makeBox(330,200);

	process_help_string(button_text, ksGlobalTextArray[GT_TUTORIAL_PAUSE_BUTTONS].c_str());

	finished = false;
	first_step = true;
	needs_to_be_in_tube = false;
	current_trick = -1;
	current_gap = -1;
	current_step = 0;
	current_step_time = 0.0f;
	air_trick_in_chain = false;
	face_trick_in_chain = false;
	is_perfect = false;
	show_advancement_text = false;
	show_hint_text = false;
	current_VO = NSL_INVALID_ID;
}

//	~IGOTutorialManager()
// Destructor.
IGOTutorialManager::~IGOTutorialManager()
{
	delete instruction_text;
}


//	Reset()
// Clears the icons off the screen and resets timers.  Should be called when a new run begins.
void IGOTutorialManager::Reset()
{
	finished = false;
	first_step = true;
	needs_to_be_in_tube = false;
	current_trick = -1;
	current_gap = -1;
	current_step = 0;
	current_step_time = 0.0f;
	air_trick_in_chain = false;
	face_trick_in_chain = false;
	is_perfect = false;
	show_advancement_text = false;
	show_hint_text = false;
	current_VO = NSL_INVALID_ID;

	SetTutorialSection(g_game_ptr->get_level_id());
}


void IGOTutorialManager::SetTutorialSection(int tutorial_level)
{
	if (tutorial_level == LEVEL_INDOOR_1)
	{
		current_step = 0;
		last_step = FIRST_SET_END;
	}
	else if (tutorial_level == LEVEL_INDOOR_2)
	{
		current_step = FIRST_SET_END;
		last_step = SECOND_SET_END;
	}
	else
	{
		current_step = SECOND_SET_END;
		last_step = Tutorial_Step_Num;
	}
}


//	Draw()
// Draws all the icons onscreen.
void IGOTutorialManager::Draw(void)
{
	// Draw help message
	if (!g_game_ptr->is_paused() && (show_advancement_text || show_hint_text) && !AlmostFinished() &&
		(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player()))->get_super_state() != SUPER_STATE_FLYBY)
		instruction_text->Draw();
}


float hint_time = 20.0f;

//	Update()
// Call often with time delta.
void IGOTutorialManager::Update(const float dt)
{
	//  Start out thinking that we're ready to go to the next step, they try to disprove that.
	bool nextMessage = true;  
	kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
	bool currently_in_tube = ksctrl->get_super_state() == SUPER_STATE_IN_TUBE;

	//  Don't even update the timer in this condition.
	if (GTutorialStep[current_step].flags == ONLY_TUBE_UPDATE_TIME_FLAG && 
		(!currently_in_tube || show_advancement_text))
		return;

	//  Some steps must be in the tube before any of the criteria should be met.
	if (needs_to_be_in_tube && (!currently_in_tube || show_advancement_text))
		return;
	else if (currently_in_tube && !show_advancement_text)
		needs_to_be_in_tube = false;

	//  Make sure that the current sound cannot be dampened by a wipeout.
	if (nslGetSoundStatus(current_VO) != NSL_SOUNDSTATUS_INVALID)
		nslDampenGuardSound(current_VO);

	current_step_time += dt;

	//  If the player has been struggling with this for awhile, then put up a help message.
	if (current_step_time >= hint_time && !show_advancement_text)
	{
		show_hint_text = true;
		process_help_string(ins_text, ksGlobalTextArray[GTutorialStep[current_step].hint_text_id].c_str());
		instruction_text->changeText(ins_text);
		instruction_text->makeBox(330,200);
	}


	////////  Check each type of advancement criteria, and if there is an entry, make sure that condition has been met.

	//  Specific button pressed.  Check both button slots.
	if ((GTutorialStep[current_step].advancement_button != PAD_NONE &&
		!ksctrl->CtrlEvent(GTutorialStep[current_step].advancement_button)) ||
		(GTutorialStep[current_step].advancement_button2 != PAD_NONE &&
		!ksctrl->CtrlEvent(GTutorialStep[current_step].advancement_button2)))
		nextMessage = false;

	//  Game is in a certain state.  There are two possible advancement states, check both.
	if (GTutorialStep[current_step].advancement_state != STATE_NOSTATE &&
		GTutorialStep[current_step].advancement_state != ksctrl->get_current_state() &&
		GTutorialStep[current_step].advancement_state2 != ksctrl->get_current_state())
		nextMessage = false;

	//  Game is in a certain super state.
	if (GTutorialStep[current_step].advancement_super_state != SUPER_STATE_NO_SUPERSTATE &&
		GTutorialStep[current_step].advancement_super_state != ksctrl->get_super_state() &&
		GTutorialStep[current_step].advancement_super_state2 != ksctrl->get_super_state())
		nextMessage = false;

	//  Minimun message time has passed.
	if (GTutorialStep[current_step].advancement_time != 0 &&
		GTutorialStep[current_step].advancement_time > current_step_time)
		nextMessage = false;

	//  Check for specific tricks.
	if (GTutorialStep[current_step].advancement_trick != TRICK_NONE &&
		GTutorialStep[current_step].advancement_trick != current_trick &&
		GTutorialStep[current_step].advancement_trick2 != current_trick &&
		GTutorialStep[current_step].advancement_trick3 != current_trick &&
		GTutorialStep[current_step].advancement_trick4 != current_trick)
		nextMessage = false;

	if (GTutorialStep[current_step].advancement_gap != GAP_NONE &&
		GTutorialStep[current_step].advancement_gap != current_gap)
		nextMessage = false;

	//  Check to see if we're supposed to advance when the special meter is active.
	if (GTutorialStep[current_step].flags == SPECIAL_ACTIVE_FLAG &&
		!ksctrl->get_special_meter()->CanRegionLink())
		nextMessage = false;

	//  Check to see if we're supposed to advance when the an air trick is linked to a face trick.
	if (GTutorialStep[current_step].flags == AIR_AND_FACE_FLAG &&
		(current_trick == -1 || !air_trick_in_chain || !face_trick_in_chain))
		nextMessage = false;

	//  Check to see if we're supposed to advance when the an air trick is linked to a face trick.
	if (GTutorialStep[current_step].flags == PERFECT_TRICK_FLAG && !is_perfect)
		nextMessage = false;

	if (GTutorialStep[current_step].flags == SPECIAL_TRICK_FLAG &&
		(current_trick == -1 ||
		!(GTrickList[current_trick].flags & SpecialFlag)))
		nextMessage = false;

	//  After the congratulations VO, don't update the current step until the user has hit pause.
	if (show_advancement_text && GTutorialStep[current_step].text_id != GT_TUTORIAL_NONE)
		nextMessage = false;

	if (nextMessage || first_step)
	{
		//  Don't advance the step the first time, leave it at zero.
		if (!first_step)
		{
			current_step++;

			//  Show a message telling the player that a new tip is available.
			show_hint_text = false;
			show_advancement_text = true;
			process_help_string(ins_text, ksGlobalTextArray[GT_TUTORIAL_PRESS_START].c_str());
			instruction_text->changeText(ins_text);
			instruction_text->makeBox(330,200);
		}
		else
		{
			device_id_t dev = g_world_ptr->get_ks_controller( 0 )->get_joystick_num();
			frontendmanager.pms->pause_controller = DEVICE_ID_TO_JOYSTICK_INDEX( dev );
			frontendmanager.pms->startDraw(PauseMenuSystem::TutorialMenu);
		}

		//  Some steps require starting out in the tube.
		if (GTutorialStep[current_step].flags == BEGIN_IN_TUBE_FLAG)
			needs_to_be_in_tube = true;
		else
			needs_to_be_in_tube = false;

		if (current_step >= last_step)  //  Check to see if this is the last step.
		{
			finished = true;
			current_step_time = 0.0f;
			return;
		}

		//  Get the lines of text.  If at the end, then replace the first line with a "done" message.
		if (!AlmostFinished())
			process_help_string(help_text, ksGlobalTextArray[GTutorialStep[current_step].text_id].c_str());
		else
			process_help_string(help_text, ksGlobalTextArray[GT_TUTORIAL_DONE].c_str());

		//  Stop the old sound when the current step is complete.  This plays the success sounds.
		if (GTutorialStep[current_step].text_id == GT_TUTORIAL_NONE)
		{
			if (nslGetSoundStatus(current_VO) != NSL_SOUNDSTATUS_INVALID)
				nslStopSound(current_VO);

			//  Start the new sound
			if (strcmp(GTutorialStep[current_step].vo_name_string, "TUTORIAL_NONE"))
				current_VO = play_sound(GTutorialStep[current_step].vo_name_string);
		}

		first_step = false;
		current_step_time = 0.0f;
	}
	
	if (nslGetSoundStatus(current_VO) != NSL_SOUNDSTATUS_INVALID && !wSound.isdampened())
	{
		wSound.dampen(.6);
	}
	else if (nslGetSoundStatus(current_VO) == NSL_SOUNDSTATUS_INVALID && wSound.isdampened())
	{
		wSound.undampen();
	}
	current_trick = -1;
	current_gap = -1;
	is_perfect = false;
}


void IGOTutorialManager::process_help_string(stringx &dest, const char *source)
{
	char temp[500];
	int index = 0;

	strcpy(temp, source);

	while (temp[index] != 0)
	{
		if (temp[index] == '@')
		{
			char insert[3];
			switch (temp[index + 1])
			{
			case 'T':
				strcpy(insert, ksGlobalButtonArray[GT_PadTriangle].c_str());
				break;
			case 'S':
				strcpy(insert, ksGlobalButtonArray[GT_PadSquare].c_str());
				break;
			case 'C':
				strcpy(insert, ksGlobalButtonArray[GT_PadCircle].c_str());
				break;
			case 'X':
				strcpy(insert, ksGlobalButtonArray[GT_PadCross].c_str());
				break;
			case 'B':
				strcpy(insert, ksGlobalButtonArray[GT_PadBack].c_str());
				break;
			case 'A':
				strcpy(insert, ksGlobalButtonArray[GT_PadStart].c_str());
				break;
			default:
				break;
			}

			temp[index] = insert[0];
			temp[index + 1] = insert[1];
		}

		index++;
	}

	dest = temp;
}


void IGOTutorialManager::EndChain()
{
	air_trick_in_chain = false;
	face_trick_in_chain = false;
}


void IGOTutorialManager::SetCurrentTrick(int trick_num)
{
	current_trick = trick_num;

	if (GTrickList[current_trick].trick_type == TRICKTYPE_AERIAL)
	{
		air_trick_in_chain = true;
		face_trick_in_chain = false;  //  always do a air trick first, then the face trick.
	}
	else if (GTrickList[current_trick].trick_type == TRICKTYPE_FACE)
		face_trick_in_chain = true;
}


void IGOTutorialManager::SetCurrentGap(int gap_num)
{
	current_gap = gap_num;
}


nslSoundId IGOTutorialManager::play_sound(const char* name)
{
	nslSoundId snd = NSL_INVALID_ID;
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		nslSourceId s = nslLoadSource(name);
		if (s != NSL_INVALID_ID)
		{
			snd = nslAddSound(s);
			if (snd != NSL_INVALID_ID)
			{
				nslPlaySound(snd);
			}
		}
		else debug_print("FIXME: MISSING %s SOUND\n", name);
	}

	return snd;
}

void IGOTutorialManager::PlayCurrentVO(bool intro)
{
	//  Stop the old sound.
	if (nslGetSoundStatus(current_VO) != NSL_SOUNDSTATUS_INVALID)
		nslStopSound(current_VO);

	//  Start the new sound
	if (!intro)
	{
		if (strcmp(GTutorialStep[current_step].vo_name_string, "TUTORIAL_NONE"))
			current_VO = play_sound(GTutorialStep[current_step].vo_name_string);
	}
	else
	{
		if (g_game_ptr->get_level_id() == LEVEL_INDOOR_1)
			current_VO = play_sound("TUTORIAL_OPENING_BEGINNER");
		else if (g_game_ptr->get_level_id() == LEVEL_INDOOR_2)
			current_VO = play_sound("TUTORIAL_OPENING_INTERMEDIATE");
		else
			current_VO = play_sound("TUTORIAL_OPENING_ADVANCE");
	}
}


void IGOTutorialManager::StopCurrentVO()
{
	//  Stop the old sound.
	if (nslGetSoundStatus(current_VO) != NSL_SOUNDSTATUS_INVALID)
		nslStopSound(current_VO);

}

void IGOTutorialManager::OnEvent(const EVENT event, const int param1, const int param2)
{
	if (event == EVT_SURFER_LAND && param2 & SurfBoardObjectClass::LANDING_FLAG_PERFECT)
		is_perfect = true;
}
