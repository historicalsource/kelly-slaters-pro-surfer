
#include "global.h"
#include "globaltextenum.h"
#include "igohintmanager.h"
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


float hint_fade_in_time = 0.5f;
float hint_fade_out_time = 1.5f;
float hint_total_time = 4.0f;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOHintManager class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	IGOHintManager()
// Default constructor.
IGOHintManager::IGOHintManager()
{
	//ksGlobalTextArray[GT_TUTORIAL_PRESS_START]
	instruction_text = NEW TextString(&frontendmanager.font_info, "", 382, 125, 800, 0.9f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color32(235, 53, 8, 255));

	hint_text = "";
	current_hint = -1;

	//  Show that none of the hints have been used yet.
	for (int index = 0; index < MAX_HINTS; index++)
		hint_use[index] = 0;

	competition_help = false;
}

//	~IGOHintManager()
// Destructor.
IGOHintManager::~IGOHintManager()
{
	delete instruction_text;
}


//	Reset()
// Clears the icons off the screen and resets timers.  Should be called when a new run begins.
void IGOHintManager::Reset()
{
	hint_text = "";
	current_hint = -1;
	instruction_text->SetFade(0.0);
	current_hint_time = 2 * hint_total_time;
	competition_help = false;

	//  Show that none of the hints have been used yet.
	for (int index = 0; index < MAX_HINTS; index++)
		hint_use[index] = 0;
}


//	Draw()
// Draws help message onscreen.
void IGOHintManager::Draw(void)
{
	// Draw help message
	if (!g_game_ptr->is_paused())
		instruction_text->Draw();
}


float competition_hint_time = 60.0f;
float hint_boundary = 0.1f;		//  If the different competition scores are pretty close, don't give a hint.

//	Update()
// Call often with time delta.
void IGOHintManager::Update(const float dt)
{
	kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());

	//  In competitions, check to see if the player needs some help in one specific area.
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->is_competition_level())
	{
		if (TIMER_GetLevelSec() > competition_hint_time && !competition_help)
		{
			competition_help = true;

			int levelIdx = g_game_ptr->get_level_id();
			int facePoints, airPoints, tubePoints;
			float facePercent = 1.0f, airPercent = 1.0f, tubePercent = 1.0f;
			ksctrl->get_my_scoreManager().GetPartialScores(facePoints, airPoints,tubePoints);

			if (CareerDataArray[levelIdx].goal_param_2[0])
				facePercent = float(facePoints)/float(CareerDataArray[levelIdx].goal_param_2[0]);
			if (CareerDataArray[levelIdx].goal_param_2[1])
				airPercent = float(airPoints)/float(CareerDataArray[levelIdx].goal_param_2[1]);
			if (CareerDataArray[levelIdx].goal_param_2[2])
				tubePercent = float(tubePoints)/float(CareerDataArray[levelIdx].goal_param_2[2]);
			else
			{
				if (airPercent > facePercent)	//  Set it equal to the highest one, since there is no tube score here.
					tubePercent = airPercent;
				else
					tubePercent = facePercent;
			}

			if (tubePercent <= airPercent && tubePercent <= facePercent && 
				(tubePercent + hint_boundary < airPercent || tubePercent + hint_boundary < facePercent))
				SetHint(CompetitionNeedTube);
			else if (facePercent <= airPercent && facePercent <= tubePercent && 
				(facePercent + hint_boundary < airPercent || facePercent + hint_boundary < tubePercent))
				SetHint(CompetitionNeedFace);
			else if (airPercent <= tubePercent && airPercent <= facePercent && 
				(airPercent + hint_boundary < tubePercent || airPercent + hint_boundary < facePercent))
				SetHint(CompetitionNeedAir);
		}
	}


	//  Check to see if there's nothing left to do.
	if (current_hint_time > hint_total_time)
		return;

	current_hint_time += dt;	//  Update the time for this hint.

	//  Special case for standing.
	if (current_hint_time < hint_total_time - hint_fade_out_time &&
		current_hint == StandUp)
	{
		if (ksctrl->get_super_state() != SUPER_STATE_LIE_ON_BOARD)
			current_hint_time = hint_total_time - hint_fade_out_time;	//  Start fading out.
	}

	//  Now fade it in as needed.
	if (current_hint_time < hint_fade_in_time)
		instruction_text->SetFade(current_hint_time/hint_fade_in_time);
	else if (current_hint_time > hint_total_time)
		instruction_text->SetFade(0.0f);
	else if (current_hint_time > hint_total_time - hint_fade_out_time)
		instruction_text->SetFade((hint_total_time - current_hint_time)/hint_fade_out_time);
	else 
		instruction_text->SetFade(1.0f);	//  fully opaque.
}


void IGOHintManager::process_help_string(stringx &dest, const char *source)
{
	char temp[80];
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


void IGOHintManager::SetHint(int new_hint)
{
	assert(new_hint < NUM_HINTS);
	int current_beach = g_game_ptr->get_beach_id();

	//  No hints in multiplayer.  the screen is too small for two sets of these.
	if (g_game_ptr->is_splitscreen ())
		return;

	//  Only show the stand up hint on the first beaches.
	if ((new_hint == StandUp || new_hint == ObjectApproaching) &&
		!(current_beach == BEACH_SEBASTIAN || current_beach == BEACH_MAVERICKS || 
		current_beach == BEACH_TRESTLES || current_beach == BEACH_ANTARCTICA))
		return;

	//  The "did not stand" message should change to a different message if this keeps happening.
	if (new_hint == DidNotStand && hint_use[new_hint] >= MAX_HINT_APPEARANCES)
		new_hint = TryPaddlingForward;

	//  Don't let the hint appear more than a few times.
	if (hint_use[new_hint] >= MAX_HINT_APPEARANCES)
		return;
	else
		hint_use[new_hint]++;

	//  Set the text to the new hint.
	process_help_string(hint_text, ksGlobalTextArray[GT_HINT_STAND + new_hint].c_str());
	instruction_text->changeText(hint_text);

	//  Make it visible
	instruction_text->SetFade(0.0);
	current_hint_time = 0.0f;
	current_hint = new_hint;
}
