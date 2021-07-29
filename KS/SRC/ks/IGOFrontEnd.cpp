// IGOFrontEnd.cpp

#include "global.h"

#ifdef _XBOX
#include "refptr.h"
#include "hwrasterize.h"
#include "FrontEndManager.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "IGOFrontEnd.h"
#include "conglom.h"
#include "kellyslater_controller.h"
#include "VOEngine.h"
#include "random.h"
#include "ini_parser.h"
#include "wds.h"
#include "ksngl.h"
#include "demomode.h"

// IGOPrintQueue -----------------------------------------------------------------

IGOPrintQueue::IGOPrintQueue()
{
	clear();
}

soundMessageObject *IGOPrintQueue::pop()
{
	if(size == 0)
		return NULL;
	
	int return_idx = start;
	
	size--;
	if(size != 0)
	{
		start++;
		if(start == MAX_QUEUE_SIZE)
			start = 0;
	}
	else
		start = end = 0;
	
	return &strings[return_idx];
}

bool IGOPrintQueue::push(stringx message, EventType e)
{
	soundMessageObject s;
	s.str = message;
	s.e = e;

	if(size == MAX_QUEUE_SIZE)
		return false; // we failed to add something to the queue because it's full
	
	if(size != 0)
	{
		end++;
		if(end == MAX_QUEUE_SIZE)
			end = 0;
	}
	size++;
	
	strings[end] = s;
	
	return true; // push successful
}

void IGOPrintQueue::clear()  // clears the queue
{
	start = end = size = 0;	
}

// IGOFrontEnd -------------------------------------------------------------------

const int IGOFrontEnd::NUM_PQS_MENU = 9;
const int IGOFrontEnd::NUM_PQS_ACCOMP = 9;
const int IGOFrontEnd::NUM_PQS_ACCOMP_GRID = 3;
const int IGOFrontEnd::NUM_PQS_CONTINUE = 1;
const int IGOFrontEnd::NUM_PQS_MEDAL = 2;

int IGOFrontEnd::TRICK_WAIT_MAX = 3;
int IGOFrontEnd::TEXT_WAIT_MAX = 2;

//color32 IGOFrontEnd::COLOR_SCORE = color32(0, 0, 128, 255);
//color32 IGOFrontEnd::COLOR_JUNK = color32(64, 64, 64, 255);
//color32 IGOFrontEnd::COLOR_STANDARD_FADED = color32(173, 248, 126, 128);
//color32 IGOFrontEnd::COLOR_INFO_FADED = color32(247, 248, 125, 128);
//color32 IGOFrontEnd::COLOR_CHALLENGE = color32(255, 255, 0, 255);
//color32 IGOFrontEnd::COLOR_SLOPPY = color32(255, 0, 0, 255);
//color32 IGOFrontEnd::COLOR_TA_LOSE = color32(235, 53, 8, 255);
//color32 IGOFrontEnd::COLOR_HEADER = color32(228, 161, 115, 255);

//color32 IGOFrontEnd::COLOR_POINTS_MOUTH = color32(255, 255, 127, 255);
color32 IGOFrontEnd::COLOR_POINTS_MOUTH = color32(255, 255, 102, 255);
//color32 IGOFrontEnd::COLOR_POINTS_MAIN = color32(255, 255, 255, 255);
color32 IGOFrontEnd::COLOR_POINTS_MAIN = color32(255, 255, 255, 255);
//color32 IGOFrontEnd::COLOR_POINTS_EDGE = color32(140, 140, 140, 255);
color32 IGOFrontEnd::COLOR_POINTS_EDGE = color32(153, 163, 171, 255);
//color32 IGOFrontEnd::COLOR_PERFECT = color32(255, 255, 127, 255);
color32 IGOFrontEnd::COLOR_PERFECT = color32(255, 255, 102, 255);
//color32 IGOFrontEnd::COLOR_BLOOD = color32(0, 255, 255, 255);
color32 IGOFrontEnd::COLOR_BLOOD = color32(0, 255, 255, 255);

const int IGOFrontEnd::POS_COMP_TOP = 127;
const int IGOFrontEnd::POS_COMP_LEFT = 75;
const int IGOFrontEnd::POS_COMP_HEAT = 430;
const int IGOFrontEnd::POS_COMP_TOTAL = 535;
const int IGOFrontEnd::POS_REWARDS_LEFT = 75;
const int IGOFrontEnd::POS_REWARDS_RIGHT = 565;
const int IGOFrontEnd::POS_REWARDS_TOP = 130;
//const int IGOFrontEnd::POS_PHOTOS_LEFT = 75;
//const int IGOFrontEnd::POS_PHOTOS_RIGHT = 410;
const int IGOFrontEnd::POS_GOALS_LEFT = 75;
const int IGOFrontEnd::POS_GOALS_RIGHT = 410;
const int IGOFrontEnd::POS_GOALS_TOP = 105;
const int IGOFrontEnd::POS_STATS_LEFT = 75;
const int IGOFrontEnd::POS_STATS_RIGHT = 565;
const int IGOFrontEnd::POS_STATS_TOP = 129;
const int IGOFrontEnd::POS_PUSH_TOP = 250;
const int IGOFrontEnd::POS_HEAD_TO_HEAD_LEFT = 75;
const int IGOFrontEnd::POS_HEAD_TO_HEAD_RIGHT = 565;
const int IGOFrontEnd::POS_HEAD_TO_HEAD_BOTTOM = 392;
const int IGOFrontEnd::POS_TA_TOP = 250;
const int IGOFrontEnd::POS_TA_LEFT = 75;
const int IGOFrontEnd::POS_TA_RIGHT = 565;
const int IGOFrontEnd::POS_TA_BOTTOM = 392;
const int IGOFrontEnd::POS_METER_ATTACK_TOP = 250;
const int IGOFrontEnd::POS_METER_ATTACK_LEFT = 165;
const int IGOFrontEnd::POS_METER_ATTACK_RIGHT = 475;
const int IGOFrontEnd::LINE_SPACING = 24;

const int IGOFrontEnd::TRICK_BOX_HEIGHT = 60;
const int IGOFrontEnd::TRICK_BOX_MARGIN = 50;

IGOFrontEnd::IGOFrontEnd(FEManager* man, stringx p, stringx pf_name)
{
	int		i;
	
	MAX_TRICK_SCALE = 1.0f;
	MAX_TEXT_SCALE = 1.1f;
	
	clock_sec = TIMER_IsInfiniteDuration() ? 0 : TIMER_GetRemainingLevelSec();
	if (clock_sec < 0.0f) clock_sec = 0.0f;
	clock_min = (int) (clock_sec / 60);
	clock_sec -= clock_min * 60;
	debug_menu_displayed = false;
	replay_mode = false;
	cons(man, p, pf_name);
	trickFont = &manager->trick_font;
	numberFont = &manager->numberFont;
	stdFont = &manager->font_info;
	boldFont = &manager->font_bold;
	display = true;
	runState = RUNSTATE_NORMAL;
	proceedPressed = true;
	firstTimeAttackState = true;
	numPlayers = g_game_ptr->get_num_players();
	numActivePlayers = g_game_ptr->get_num_active_players();
	otherText = NULL;
	replayText = NULL;

	COLOR_STANDARD = manager->col_info_g;
	COLOR_ALT = manager->col_info_b;
	COLOR_LO = manager->col_highlight2;
	COLOR_HI = manager->col_highlight;
	COLOR_BAD = manager->col_bad;
	
	menuBGWidget = NULL;
	accompWidget = NULL;
	gridWidget = NULL;
	continueWidget = NULL;
	analogClockWidget = NULL;
	centerClockWidget = NULL;
	splitterWidget = NULL;
	waveIndicatorWidget = NULL;
	bigWaveMeterWidget = NULL;
	cameraWidget = NULL;
	photoWidget = NULL;
	replayWidget = NULL;
	timeAttackWidget = NULL;
	meterAttackWidget = NULL;
	iconRadarWidget = NULL;
	iconCountWidget = NULL;
	skillChallengeWidget = NULL;
	meterChallengeWidget = NULL;
	objectAlertWidget	 = NULL;
	
	iconManager = NULL;
	learn_new_trickManager = NULL;
	tutorialManager = NULL;
	hintManager = NULL;
	
	mpPanel = NULL;
	ghostIconPanel = NULL;
	compPanel = NULL;
	
	// Preload SFX.
	nslSourceId srcSound;
	srcSound = nslLoadSource("ICONDONE");

	// Allocate help text for the end run screens.
	helpText = NEW TextString(stdFont, ksGlobalButtonArray[GT_PadCross]+stringx(" ")+ksGlobalTextArray[GT_MENU_CONTINUE], 320, 424, 20, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
	helpText->setButtonScale(1.0f);
	
	// Load multiplayer panelquads.
	if (numPlayers > 1)
		mpPanel = NEW PanelFile("igo_mp.panel", "interface\\IGO\\");

	// Load competition panelquads.
	if (g_game_ptr->is_competition_level())
		compPanel = NEW PanelFile("comp_results.panel", "challenges\\competition\\");
	
	// Time Attack starts in a special state.
	if (g_game_ptr->get_game_mode() == GAME_MODE_TIME_ATTACK)
		runState = RUNSTATE_TIME_ATTACK;

	// Meter Attack starts in a special state.
	if (g_game_ptr->get_game_mode() == GAME_MODE_METER_ATTACK)
		runState = RUNSTATE_METER_ATTACK;
	
	// Initialize using career goals.
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		for (i = 0; i < MAX_GOALS_PER_LEVEL; i++)
		{
			int goal = CareerDataArray[g_game_ptr->get_level_id()].goal[i];
			// Icon manager needed for icon tetris goals.
			if (!iconManager && (goal == GOAL_ICON_TETRIS) && !dmm.inDemoMode())
			{
				iconManager = NEW IGOIconManager();
				iconManager->setFont(stdFont);
			}
			
			if (!iconCountWidget && (goal == GOAL_ICON_TETRIS || (i == 0 && goal == GOAL_SKILL_360_SPIN)
						|| (i == 0 && goal == GOAL_SKILL_360_SPIN_SCORE) || (i == 0 && goal == GOAL_SKILL_540_SPIN)
						|| (i == 0 && goal == GOAL_SKILL_540_SPIN_SCORE)) && !dmm.inDemoMode())
			{
				iconCountWidget = NEW IconCountWidget(goal);
			}

			// Icon manager needed for icon tetris goals.
			if (!learn_new_trickManager && goal == GOAL_LEARN_NEW_TRICK)
			{
				learn_new_trickManager = NEW IGOLearnNewTrickManager();
				learn_new_trickManager->setFont(stdFont);
			}
			
			// Icon target widget needed for ghose icon challenges.
			if (0 && !iconRadarWidget && goal == GOAL_ICON_3D)
			{
				
				iconRadarWidget = NEW IconRadarWidget();
				ghostIconPanel = NEW PanelFile("radar.panel", "challenges\\ghosticon\\");
			}

			if (!meterChallengeWidget && (goal == GOAL_SPECIAL_METER) && !dmm.inDemoMode())
			{
				meterChallengeWidget = NEW MeterChallengeWidget();
			}

			if (!objectAlertWidget && !dmm.inDemoMode())
				objectAlertWidget = NEW ObjectAlertWidget();

			if ((i == 0) && !skillChallengeWidget && !dmm.inDemoMode())
			{
				if ((goal == GOAL_SKILL_FACE) || (goal == GOAL_SKILL_AIR)
					|| (goal == GOAL_SKILL_TUBE) || (goal == GOAL_SKILL_AIR_SCORE)
					|| (goal == GOAL_SKILL_TUBE_SCORE) || (goal == GOAL_SKILL_FACE_SCORE)
					|| (goal == GOAL_PHOTO_1) || (goal == GOAL_PHOTO_2) || (goal == GOAL_PHOTO_3))
				{
					skillChallengeWidget = NEW SkillChallengeWidget(goal);
				}

			}
		}

	}

	hintManager = NEW IGOHintManager();

	if (g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON)
	{
		if (!iconManager && !dmm.inDemoMode())
		{
			iconManager = NEW IGOIconManager();
			iconManager->setFont(stdFont);
		}

		if (!iconCountWidget && !dmm.inDemoMode())
			iconCountWidget = NEW IconCountWidget(GOAL_ICON_TETRIS);
  }

	//  Set up the tutorial mode.
//	if (g_game_ptr->get_game_mode() == GAME_MODE_PRACTICE)
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
	{
		tutorialManager = NEW IGOTutorialManager();
		tutorialManager->SetTutorialSection(g_game_ptr->get_level_id());
	}

	// Initialize competition results text.
	competition.faceScoreText = NEW TextString(stdFont, ksGlobalTextArray[GT_COMP_FACE_SCORE], 385, 170, 800, 1.2f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	competition.faceScoreNumerals = NEW TextString(stdFont, "", 395, 170, 800, 1.2f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_ALT);
	competition.airScoreText = NEW TextString(stdFont, ksGlobalTextArray[GT_COMP_AIR_SCORE], 385, 200, 800, 1.2f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	competition.airScoreNumerals = NEW TextString(stdFont, "", 395, 200, 800, 1.2f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_ALT);
	competition.tubeScoreText = NEW TextString(stdFont, ksGlobalTextArray[GT_COMP_TUBE_SCORE], 385, 230, 800, 1.2f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	competition.tubeScoreNumerals = NEW TextString(stdFont, "", 395, 230, 800, 1.2f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_ALT);
	competition.avgScoreText = NEW TextString(stdFont, ksGlobalTextArray[GT_COMP_AVERAGE_SCORE], 385, 270, 800, 1.2f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	competition.avgScoreNumerals = NEW TextString(stdFont, "", 395, 270, 800, 1.2f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_HI);
	competition.title = NEW TextString(boldFont, ksGlobalTextArray[GT_COMP_RESULTS], 320, 75, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
	competition.name = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_COMP_NAME], POS_COMP_LEFT, POS_COMP_TOP-LINE_SPACING, 800, 0.8f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_LO);
	competition.heats = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_COMP_HEATS], POS_COMP_HEAT, POS_COMP_TOP-LINE_SPACING, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, COLOR_LO);
	competition.total = NEW TextString(&manager->font_bold_old, ksGlobalTextArray[GT_COMP_TOTAL], POS_COMP_TOTAL, POS_COMP_TOP-LINE_SPACING, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, COLOR_LO);
	competition.congrat = NEW TextString(stdFont, "", POS_COMP_LEFT, 95, 800, 1.3f, Font::HORIZJUST_LEFT, Font::VERTJUST_TOP, COLOR_HI);
	competition.wonText = NEW BoxText(&manager->font_body, "", POS_COMP_LEFT, 170, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_TOP);
	if (g_game_ptr->is_competition_level())
	{
		competition.medalWidgets[0][0] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[0][1] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[0][2] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[1][0] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[1][1] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[1][2] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[2][0] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[2][1] = NEW SimpleWidget(NUM_PQS_MEDAL);
		competition.medalWidgets[2][2] = NEW SimpleWidget(NUM_PQS_MEDAL);
	}
	else
	{
		competition.medalWidgets[0][0] = NULL;
		competition.medalWidgets[0][1] = NULL;
		competition.medalWidgets[0][2] = NULL;
		competition.medalWidgets[1][0] = NULL;
		competition.medalWidgets[1][1] = NULL;
		competition.medalWidgets[1][2] = NULL;
		competition.medalWidgets[2][0] = NULL;
		competition.medalWidgets[2][1] = NULL;
		competition.medalWidgets[2][2] = NULL;
	}
	competition.numSorted = 0;
	competition.draw = false;
	for (i = 0; i < SURFER_LAST; i++)
	{
		color32 nameColor = COLOR_ALT;
		color32	runColor = COLOR_STANDARD;
		color32	totalColor = COLOR_ALT;
		
		if (i == g_game_ptr->GetSurferIdx(0))
		{
			nameColor = COLOR_HI;
			runColor = COLOR_HI;
			totalColor = COLOR_HI;
		}
		
		// These get re-initialized later anyway.
		competition.results[i].name = NEW TextString(&manager->font_bold_old, GetCompStrSurfer(i), POS_COMP_LEFT, POS_COMP_TOP+LINE_SPACING*i, 800, .8f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, nameColor);
		competition.results[i].runs[0] = NEW TextString(&manager->font_bold_old, GetCompStrRun(i, 0), POS_COMP_HEAT-50, POS_COMP_TOP+LINE_SPACING*i, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, runColor);
		competition.results[i].runs[1] = NEW TextString(&manager->font_bold_old, GetCompStrRun(i, 1), POS_COMP_HEAT, POS_COMP_TOP+LINE_SPACING*i, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, runColor);
		competition.results[i].runs[2] = NEW TextString(&manager->font_bold_old, GetCompStrRun(i, 2), POS_COMP_HEAT+50, POS_COMP_TOP+LINE_SPACING*i, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, runColor);
		competition.results[i].total = NEW TextString(&manager->font_bold_old, GetCompStrTotal(i), POS_COMP_TOTAL, POS_COMP_TOP+LINE_SPACING*i, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, totalColor);
		
		competition.results[i].rank = 0;
	}

	// Allocate completed goals text.
	goals.draw = false;
	rewards.draw = false;
	rewards.initialized = false;
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		stringx	s;
		s.printf(ksGlobalTextArray[GT_GOALS_TITLE].c_str(), BeachDataArray[g_game_ptr->get_beach_id()].fe_name);
		goals.title = NEW TextString(boldFont, s, 320, 75, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
		rewards.title = NEW TextString(boldFont, ksGlobalTextArray[GT_REWARDS_TITLE], 320, 75, 800, 1.0, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
		
		int extra_lines = 0;
		int spacing = 55;
		for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
		{
			g_career->GetGoalText(g_game_ptr->get_level_id(), g, s);
			int y = POS_GOALS_TOP + (g*spacing) + extra_lines*20;
			goals.names[g] = NEW BoxText(stdFont, s, 320, y, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
			extra_lines += goals.names[g]->makeBox(502, 200) - 1;
			y = IGOFrontEnd::POS_GOALS_TOP + (g*spacing) + (extra_lines+1)*20;
			goals.status[g] = NEW TextString(stdFont, "", 320, y, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		}
		for (int g = 0; g < MAX_REWARDS; g++)
		{
			rewards.names[g] = NEW BoxText(stdFont, "", 320, POS_REWARDS_TOP+(g*30), 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		}
	}
	else
	{
		goals.title = NULL;
		rewards.title = NULL;
		for (int g = 0; g < 5; g++)
		{
			goals.names[g] = NULL;
			goals.status[g] = NULL;
		}
		for (int g = 0; g < MAX_REWARDS; g++)
		{
			rewards.names[g] = NULL;
		}
	}
	
	// Allocate run statistics text.
	int line = 0;
	stats.draw = false;
	stats.title = NEW TextString(boldFont, ksGlobalTextArray[GT_STATS_TITLE], 320, 75, 800, 1.0, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
	stats.totalText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_SCORE], POS_STATS_LEFT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	stats.totalNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	line++;
	stats.tubeText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_TUBE], POS_STATS_LEFT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	stats.tubeNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	line++;
	stats.tricksText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_TRICKS], POS_STATS_LEFT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	stats.tricksNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	line++;
	if (iconManager)
	{
		stats.iconsText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_ICONS], POS_STATS_LEFT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		stats.iconsNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		line++;
	}
	else
	{
		stats.iconsText = NULL;
		stats.iconsNumerals = NULL;
	}
	stats.comboText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_COMBO], POS_STATS_LEFT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
	stats.comboNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	line++;
	stats.comboDetails = NEW TrickBoxText(trickFont, "", POS_STATS_LEFT+20, POS_STATS_TOP+LINE_SPACING*line, 800, 1.0f);
	
	// Allocate push mode text.
	push.draw = false;
	if (g_game_ptr->get_game_mode() == GAME_MODE_PUSH)
	{
		push.titleText = NEW TextString(boldFont, ksGlobalTextArray[GT_PUSH_TITLE], 320, 75, 800, 1.0, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
		push.playerText = NEW TextString(stdFont, "", 320, 220, 800, 1.5f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		push.timeText = NEW TextString(stdFont, "", 320, 220+40, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
	}
	else
	{
		push.titleText = NULL;
		push.playerText = NULL;
		push.timeText = NULL;
	}
	
	// Allocate head to head mode text.
	headToHead.draw = false;
	if (g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD)
	{
		headToHead.titleText = NEW TextString(boldFont, ksGlobalTextArray[GT_HEAD_TO_HEAD_TITLE], 320, 75, 800, 1.0, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
		headToHead.winText = NEW BouncingText(stdFont, "", 320, 135, 800, 1.3f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		headToHead.player1Text = NEW TextString(stdFont, "", POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*8, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player1ScoreText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_SCORE], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*7, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		headToHead.player1ScoreNumerals = NEW TextString(stdFont, "", POS_HEAD_TO_HEAD_RIGHT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*7, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player1CountText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_TRICKS], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*6, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		headToHead.player1CountNumerals = NEW TextString(stdFont, "", POS_HEAD_TO_HEAD_RIGHT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*6, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player1ComboText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_COMBO], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*5, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		headToHead.player1ComboNumerals = NEW TextString(stdFont, "", POS_HEAD_TO_HEAD_RIGHT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*5, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player2Text = NEW TextString(stdFont, ksGlobalTextArray[GT_MULTI_P2], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*3, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player2ScoreText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_SCORE], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*2, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		headToHead.player2ScoreNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*2, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player2CountText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_TRICKS], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*1, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		headToHead.player2CountNumerals = NEW TextString(stdFont, "", POS_STATS_RIGHT, POS_HEAD_TO_HEAD_BOTTOM-LINE_SPACING*1, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		headToHead.player2ComboText = NEW TextString(stdFont, ksGlobalTextArray[GT_STATS_COMBO], POS_HEAD_TO_HEAD_LEFT, POS_HEAD_TO_HEAD_BOTTOM, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		headToHead.player2ComboNumerals = NEW TextString(stdFont, "", POS_HEAD_TO_HEAD_RIGHT, POS_HEAD_TO_HEAD_BOTTOM, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	}
	else
	{
		headToHead.titleText = NULL;
		headToHead.winText = NULL;
		headToHead.player1Text = NULL;
		headToHead.player1ScoreText = NULL;
		headToHead.player1ScoreNumerals = NULL;
		headToHead.player1CountText = NULL;
		headToHead.player1CountNumerals = NULL;
		headToHead.player1ComboText = NULL;
		headToHead.player1ComboNumerals = NULL;
		headToHead.player2Text = NULL;
		headToHead.player2ScoreText = NULL;
		headToHead.player2ScoreNumerals = NULL;
		headToHead.player2CountText = NULL;
		headToHead.player2CountNumerals = NULL;
		headToHead.player2ComboText = NULL;
		headToHead.player2ComboNumerals = NULL;
	}
	
	// Allocate time attack text.
	timeAttack.draw = false;
	if (g_game_ptr->get_game_mode() == GAME_MODE_TIME_ATTACK)
	{
		timeAttack.title = NEW TextString(boldFont, ksGlobalTextArray[GT_TIME_ATTACK_TITLE], 320, 75, 800, 1.0, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
		timeAttack.ready = NEW BouncingText(stdFont, "", 320, 165, 800, 1.7f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		timeAttack.player1Text = NEW TextString(stdFont, ksGlobalTextArray[GT_MULTI_P1], POS_TA_LEFT, POS_TA_BOTTOM-LINE_SPACING*6, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_HI);
		timeAttack.player1Score = NEW TextString(stdFont, "", POS_TA_LEFT, POS_TA_BOTTOM-LINE_SPACING*5, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		timeAttack.player1ScoreNumerals = NEW TextString(stdFont, "", POS_TA_RIGHT, POS_TA_BOTTOM-LINE_SPACING*5, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		timeAttack.player1Time = NEW TextString(stdFont, ksGlobalTextArray[GT_TIME_ATTACK_REMAINING], POS_TA_LEFT, POS_TA_BOTTOM-LINE_SPACING*4, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		timeAttack.player1TimeNumerals = NEW TextString(stdFont, "", POS_TA_RIGHT, POS_TA_BOTTOM-LINE_SPACING*4, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		timeAttack.player2Text = NEW TextString(stdFont, ksGlobalTextArray[GT_MULTI_P2], POS_TA_LEFT, POS_TA_BOTTOM-LINE_SPACING*2, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_HI);
		timeAttack.player2Score = NEW TextString(stdFont, ksGlobalTextArray[GT_TIME_ATTACK_SCORE_P2], POS_TA_LEFT, POS_TA_BOTTOM-LINE_SPACING*1, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		timeAttack.player2ScoreNumerals = NEW TextString(stdFont, "", POS_TA_RIGHT, POS_TA_BOTTOM-LINE_SPACING*1, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
		timeAttack.player2Time = NEW TextString(stdFont, ksGlobalTextArray[GT_TIME_ATTACK_REMAINING], POS_TA_LEFT, POS_TA_BOTTOM, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, COLOR_STANDARD);
		timeAttack.player2TimeNumerals = NEW TextString(stdFont, "", POS_TA_RIGHT, POS_TA_BOTTOM, 800, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_HI);
	}
	else
	{
		timeAttack.title = NULL;
		timeAttack.ready = NULL;
		timeAttack.player1Text = NULL;
		timeAttack.player1Score = NULL;
		timeAttack.player1ScoreNumerals = NULL;
		timeAttack.player1Time = NULL;
		timeAttack.player1TimeNumerals = NULL;
		timeAttack.player2Text = NULL;
		timeAttack.player2Score = NULL;
		timeAttack.player2ScoreNumerals = NULL;
		timeAttack.player2Time = NULL;
		timeAttack.player2TimeNumerals = NULL;
	}

	// Meter attack text.
	meterAttack.draw = false;
	if (g_game_ptr->get_game_mode() == GAME_MODE_METER_ATTACK)
	{
		meterAttack.title = NEW TextString(boldFont, ksGlobalTextArray[GT_METER_ATTACK_TITLE], 320, 75, 800, 1.0, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_LO);
		meterAttack.ready = NEW BouncingText(stdFont, "", 320, 165, 800, 1.7f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		meterAttack.player1Meter = NEW TextString(stdFont, "", POS_METER_ATTACK_LEFT, POS_METER_ATTACK_TOP, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		meterAttack.player1MeterNumerals = NEW TextString(stdFont, "", POS_METER_ATTACK_LEFT, POS_METER_ATTACK_TOP+30, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		meterAttack.player1Time = NEW TextString(stdFont, ksGlobalTextArray[GT_METER_ATTACK_REMAINING], POS_METER_ATTACK_LEFT, POS_METER_ATTACK_TOP+60, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		meterAttack.player1TimeNumerals = NEW TextString(stdFont, "", POS_METER_ATTACK_LEFT, POS_METER_ATTACK_TOP+90, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		meterAttack.player2Meter = NEW TextString(stdFont, ksGlobalTextArray[GT_METER_ATTACK_METER_P2], POS_METER_ATTACK_RIGHT, POS_METER_ATTACK_TOP, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		meterAttack.player2MeterNumerals = NEW TextString(stdFont, "", POS_METER_ATTACK_RIGHT, POS_METER_ATTACK_TOP+30, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		meterAttack.player2Time = NEW TextString(stdFont, ksGlobalTextArray[GT_METER_ATTACK_REMAINING], POS_METER_ATTACK_RIGHT, POS_METER_ATTACK_TOP+60, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		meterAttack.player2TimeNumerals = NEW TextString(stdFont, "", POS_METER_ATTACK_RIGHT, POS_METER_ATTACK_TOP+90, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
	}
	else
	{
		meterAttack.title = NULL;
		meterAttack.ready = NULL;
		meterAttack.player1Meter = NULL;
		meterAttack.player1MeterNumerals = NULL;
		meterAttack.player1Time = NULL;
		meterAttack.player1TimeNumerals = NULL;
		meterAttack.player2Meter = NULL;
		meterAttack.player2MeterNumerals = NULL;
		meterAttack.player2Time = NULL;
		meterAttack.player2TimeNumerals = NULL;
	}
	
	// Allocate IGOs for all modes.
	menuBGWidget = NEW SimpleWidget(NUM_PQS_MENU);
	accompWidget = NEW SimpleWidget(NUM_PQS_ACCOMP);
	gridWidget = NEW GridWidget();
	//continueWidget = NEW SimpleWidget(NUM_PQS_CONTINUE);
	
	// Allocate player IGOs for all modes.
	players = NEW PLAYER[numPlayers];
	assert(numPlayers <= 2);
	for (i = 0; i < numPlayers; i++)
	{
		players[i].splitScoreWidget = NULL;
		players[i].splitMeterWidget = NULL;
		players[i].meterWidget = NULL;
		
		players[i].trickAttrText = NEW TextString(trickFont, "", 0, 0, 900, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, COLOR_PERFECT);
		players[i].trickAttrText->checkTime = false;
		players[i].trickPointText = NEW RandomText(numberFont, "", 0, 0, 900, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_POINTS_MAIN);
		players[i].trickPointText->checkTime = false;
		players[i].trickText = NEW TrickBoxText(trickFont, "", 0, 0, 850, MAX_TRICK_SCALE);
		players[i].trickText->checkTime = false;
		players[i].trickBurstText = NEW BurstTrickText();
		players[i].trickBurstText->checkTime = false;
		players[i].pointBurstText = NEW BurstText();
		players[i].pointBurstText->checkTime = false;
		
		players[i].horizBalanceWidget = NEW HorizBalanceWidget();
		
		//players[i].tubeTimer = NEW TextString(stdFont, "", x1+100, y1+420, 850, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_CENTER);
		players[i].tubeTimer = NULL;
		players[i].drawTubeTimer = false;

		players[i].viewport.tl.x = 0;
		players[i].viewport.tl.y = 0;
		players[i].viewport.br.x = 639;
		players[i].viewport.br.y = 479;
	}
	
	// Allocate IGOs for specific modes.
	assert(numPlayers <= 2);
	switch (g_game_ptr->get_game_mode())
	{
	case GAME_MODE_PRACTICE :
	case GAME_MODE_CAREER :
	case GAME_MODE_FREESURF_INFINITE :
	case GAME_MODE_FREESURF_HIGHSCORE :
	case GAME_MODE_FREESURF_ICON :
	case GAME_MODE_AI :
		// General IGOs.
		analogClockWidget = NEW AnalogClockWidget();
		waveIndicatorWidget = NEW WaveIndicatorWidget();
		//bigWaveMeterWidget = NEW FanMeterWidget();
		cameraWidget = NEW CameraWidget();
		photoWidget = NEW PhotoWidget();
		replayWidget = NEW ReplayWidget();
		otherText = NEW BoxText(stdFont, "", 320, 340, 850, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		replayText = NEW TextString(stdFont, "", 320, 380, 850, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_HI);
		
		// Player 1 IGOs.
		players[0].meterWidget = NEW SpecialMeterWidget();
		break;
		
	case GAME_MODE_TIME_ATTACK :
		// General IGOs.
		timeAttackWidget = NEW TimeAttackWidget();
		
		// Player 1 IGOs.
		players[0].meterWidget = NEW SpecialMeterWidget();
		
		// Player 2 IGOs.
		players[1].meterWidget = NEW SpecialMeterWidget();
		break;

	case GAME_MODE_METER_ATTACK :
		// General IGOs.
		meterAttackWidget = NEW TimeAttackWidget();
		
		// Player 1 IGOs.
		players[0].meterWidget = NEW SpecialMeterWidget();
		
		// Player 2 IGOs.
		players[1].meterWidget = NEW SpecialMeterWidget();
		break;
		
	case GAME_MODE_HEAD_TO_HEAD :
	case GAME_MODE_PUSH :
		
		// General IGOs.
		splitterWidget = NEW SplitterWidget();
		centerClockWidget = NEW SplitClockWidget();
		
		// Player 1 IGOs.
		players[0].splitScoreWidget = NEW SplitScoreWidget();
		players[0].splitMeterWidget = NEW SplitMeterWidget();
		
		// Player 2 IGOs.
		players[1].splitScoreWidget = NEW SplitScoreWidget();
		players[1].splitMeterWidget = NEW SplitMeterWidget();
		break;
		
	default : assert(false);
	}

#ifdef DEBUG
	debugText = NEW TextString(stdFont, "", 50, 50, 850, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_TOP);
#endif
}

IGOFrontEnd::~IGOFrontEnd()
{
	int	i;
	
	delete mpPanel;
	delete ghostIconPanel;
	delete compPanel;
	
	// Deallocate player IGOs.
	for (i = 0; i < numPlayers; i++)
	{
		delete players[i].horizBalanceWidget;
		delete players[i].meterWidget;
		delete players[i].splitScoreWidget;
		delete players[i].splitMeterWidget;
		delete players[i].trickText;
		delete players[i].trickBurstText;
		delete players[i].pointBurstText;
		delete players[i].trickAttrText;
		delete players[i].trickPointText;
		delete players[i].tubeTimer;
	}
	delete [] players;
	
	// Deallocate general text.
	delete otherText;
	delete replayText;
	delete helpText;
	
	// Deallocate general widgets.
	delete menuBGWidget;
	delete accompWidget;
	delete continueWidget;
	delete analogClockWidget;
	delete centerClockWidget;
	delete splitterWidget;
	delete waveIndicatorWidget;
	delete bigWaveMeterWidget;
	delete cameraWidget;
	delete photoWidget;
	delete replayWidget;
	delete timeAttackWidget;
	delete meterAttackWidget;
	delete iconRadarWidget;
	delete iconCountWidget;
	delete skillChallengeWidget;
	delete meterChallengeWidget;
	delete objectAlertWidget;
	delete gridWidget;
	
	delete iconManager;
	delete learn_new_trickManager;
	if (tutorialManager) delete tutorialManager;
	if (hintManager) delete hintManager;
	
	// Deallocate competition text.
	for (i = 0; i < SURFER_LAST; i++)
	{
		delete competition.results[i].name;
		delete competition.results[i].runs[0];
		delete competition.results[i].runs[1];
		delete competition.results[i].runs[2];
		delete competition.results[i].total;
	}
	delete competition.congrat;
	delete competition.wonText;
	delete competition.medalWidgets[0][0];
	delete competition.medalWidgets[0][1];
	delete competition.medalWidgets[0][2];
	delete competition.medalWidgets[1][0];
	delete competition.medalWidgets[1][1];
	delete competition.medalWidgets[1][2];
	delete competition.medalWidgets[2][0];
	delete competition.medalWidgets[2][1];
	delete competition.medalWidgets[2][2];
	delete competition.faceScoreText;
	delete competition.faceScoreNumerals;
	delete competition.airScoreText;
	delete competition.airScoreNumerals;
	delete competition.tubeScoreText;
	delete competition.tubeScoreNumerals;
	delete competition.avgScoreText;
	delete competition.avgScoreNumerals;
	delete competition.title;
	delete competition.name;
	delete competition.heats;
	delete competition.total;

	// Deallocate goaland reward text.
	delete goals.title;
	delete rewards.title;
	for(i = 0; i < MAX_GOALS_PER_LEVEL; i++)
	{
		delete goals.names[i];
		delete goals.status[i];
	}
	for(i = 0; i < MAX_REWARDS; i++)
	{
		delete rewards.names[i];
	}
	
	// Deallocate stats text.
	delete stats.title;
	delete stats.totalText;
	delete stats.totalNumerals;
	delete stats.tubeText;
	delete stats.tubeNumerals;
	delete stats.tricksText;
	delete stats.tricksNumerals;
	delete stats.iconsText;
	delete stats.iconsNumerals;
	delete stats.comboText;
	delete stats.comboNumerals;
	delete stats.comboDetails;
	
	// Deallocate push text.
	delete push.titleText;
	delete push.playerText;
	delete push.timeText;
	
	// Deallocate head to head text.
	delete headToHead.titleText;
	delete headToHead.winText;
	delete headToHead.player1Text;
	delete headToHead.player1ScoreText;
	delete headToHead.player1ScoreNumerals;
	delete headToHead.player1CountText;
	delete headToHead.player1CountNumerals;
	delete headToHead.player1ComboText;
	delete headToHead.player1ComboNumerals;
	delete headToHead.player2Text;
	delete headToHead.player2ScoreText;
	delete headToHead.player2ScoreNumerals;
	delete headToHead.player2CountText;
	delete headToHead.player2CountNumerals;
	delete headToHead.player2ComboText;
	delete headToHead.player2ComboNumerals;
	
	// Deallocate time attack text.
	delete timeAttack.title;
	delete timeAttack.ready;
	delete timeAttack.player1Text;
	delete timeAttack.player1Score;
	delete timeAttack.player1ScoreNumerals;
	delete timeAttack.player1Time;
	delete timeAttack.player1TimeNumerals;
	delete timeAttack.player2Text;
	delete timeAttack.player2Score;
	delete timeAttack.player2ScoreNumerals;
	delete timeAttack.player2Time;
	delete timeAttack.player2TimeNumerals;

	// Deallocate meter attack text.
	delete meterAttack.title;
	delete meterAttack.ready;
	delete meterAttack.player1Meter;
	delete meterAttack.player1MeterNumerals;
	delete meterAttack.player1Time;
	delete meterAttack.player1TimeNumerals;
	delete meterAttack.player2Meter;
	delete meterAttack.player2MeterNumerals;
	delete meterAttack.player2Time;
	delete meterAttack.player2TimeNumerals;

#ifdef DEBUG
	delete debugText;
#endif
}

//	Init()
// One-time initialization to load IGO's panel quads.
void IGOFrontEnd::Init(void)
{
	stringx		menuPQNames[NUM_PQS_MENU] = { "pause_01", "pause_02", "pause_03", "pause_04", "pause_05", "pause_06", "pause_07", "pause_08", "pause_09" };
	stringx		accompPQNames[NUM_PQS_ACCOMP] = { "Accomp1", "Accomp2", "Accomp3", "Accomp4", "Accomp5", "Accomp6", "Accomp7", "Accomp8", "Accomp9" };
	stringx		accompGridPQNames[NUM_PQS_ACCOMP_GRID] = { "gridline2", "gridline3", "gridline4" };
	stringx		continuePQNames[NUM_PQS_CONTINUE] = { "accept" };
	stringx		medal00PQNames[NUM_PQS_MEDAL] = { "cr_prize_01c", "cr_prize_bamboo BONK" };
	stringx		medal01PQNames[NUM_PQS_MEDAL] = { "cr_prize_02c", "cr_prize_bamboo BONK" };
	stringx		medal02PQNames[NUM_PQS_MEDAL] = { "cr_prize_03c", "cr_prize_bamboo BONK" };
	stringx		medal10PQNames[NUM_PQS_MEDAL] = { "cr_prize_01b", "cr_prize_bamboo BONK" };
	stringx		medal11PQNames[NUM_PQS_MEDAL] = { "cr_prize_02b", "cr_prize_bamboo BONK" };
	stringx		medal12PQNames[NUM_PQS_MEDAL] = { "cr_prize_03b", "cr_prize_bamboo BONK" };
	stringx		medal20PQNames[NUM_PQS_MEDAL] = { "cr_prize_01a", "cr_prize_bamboo BONK" };
	stringx		medal21PQNames[NUM_PQS_MEDAL] = { "cr_prize_02a", "cr_prize_bamboo BONK" };
	stringx		medal22PQNames[NUM_PQS_MEDAL] = { "cr_prize_03a", "cr_prize_bamboo BONK" };
	PanelFile *	photoPanel = ((PhotoFrontEnd *) manager->pms->menus[PauseMenuSystem::PhotoMenu])->GetPanel();
	
	// Load panel files.
	//g_file_finder->push_path_back("challenges\\Photo\\textures\\");
	panel.Load();
	//g_file_finder->pop_path_back();
	if (mpPanel) mpPanel->Load();
	if (ghostIconPanel) ghostIconPanel->Load();
	if (compPanel) compPanel->Load();
	
	// Initialize widgets.
	if (menuBGWidget) menuBGWidget->Init(panel, menuPQNames);
	if (accompWidget) accompWidget->Init(panel, accompPQNames);
	if (continueWidget) continueWidget->Init(panel, continuePQNames);
	if (analogClockWidget) analogClockWidget->Init(panel, numberFont);
	if (centerClockWidget) centerClockWidget->Init(*mpPanel, numberFont);
	if (splitterWidget) splitterWidget->Init(*mpPanel);
	if (waveIndicatorWidget) waveIndicatorWidget->Init(panel, !BeachDataArray[g_game_ptr->get_beach_id()].bdir, numberFont, color32(255, 255, 102, 255), color32(0, 255, 255, 255)/*color32(142, 190, 203, 255)*/);
	if (bigWaveMeterWidget) bigWaveMeterWidget->Init(panel);
	if (cameraWidget) cameraWidget->Init(*photoPanel);
	if (gridWidget) gridWidget->Init(panel);
	if (photoWidget)
	{
		photoWidget->Init(photoPanel->GetPointer("igo_camera_polaroid"), stdFont);
		photoWidget->SetPosition(430, 150, 300);
	}
	if (replayWidget) replayWidget->Init(panel, stdFont);
	if (timeAttackWidget) timeAttackWidget->Init(*mpPanel, numberFont, numberFont);
	if (meterAttackWidget) meterAttackWidget->Init(*mpPanel, numberFont, numberFont);
	if (iconRadarWidget) iconRadarWidget->Init(*ghostIconPanel, g_beach_ptr->get_challenges()->icon);
	if (iconCountWidget) iconCountWidget->Init(panel, numberFont, numberFont, COLOR_HI, COLOR_ALT);
	if (skillChallengeWidget) skillChallengeWidget->Init(panel, numberFont, numberFont, COLOR_HI, COLOR_ALT);
	if (meterChallengeWidget) meterChallengeWidget->Init(panel, numberFont, stdFont, COLOR_HI, COLOR_ALT);
	if (objectAlertWidget) objectAlertWidget->Init(panel);
	if (competition.medalWidgets[0][0]) competition.medalWidgets[0][0]->Init(*compPanel, medal00PQNames);
	if (competition.medalWidgets[0][1]) competition.medalWidgets[0][1]->Init(*compPanel, medal01PQNames);
	if (competition.medalWidgets[0][2]) competition.medalWidgets[0][2]->Init(*compPanel, medal02PQNames);
	if (competition.medalWidgets[1][0]) competition.medalWidgets[1][0]->Init(*compPanel, medal10PQNames);
	if (competition.medalWidgets[1][1]) competition.medalWidgets[1][1]->Init(*compPanel, medal11PQNames);
	if (competition.medalWidgets[1][2]) competition.medalWidgets[1][2]->Init(*compPanel, medal12PQNames);
	if (competition.medalWidgets[2][0]) competition.medalWidgets[2][0]->Init(*compPanel, medal20PQNames);
	if (competition.medalWidgets[2][1]) competition.medalWidgets[2][1]->Init(*compPanel, medal21PQNames);
	if (competition.medalWidgets[2][2]) competition.medalWidgets[2][2]->Init(*compPanel, medal22PQNames);
	
	// Initialize player IGOs.
	for (int i = 0; i < numPlayers; i++)
	{
		TurnOnTubeTimer(i, false);
		
		if (players[i].meterWidget)
		{
			if (g_game_ptr->get_game_mode() == GAME_MODE_TIME_ATTACK || g_game_ptr->get_game_mode() == GAME_MODE_METER_ATTACK)
				players[i].meterWidget->Init(*mpPanel, g_world_ptr->get_ks_controller(i)->get_special_meter());
			else
				players[i].meterWidget->Init(panel, g_world_ptr->get_ks_controller(i)->get_special_meter());
		}
		
		if (players[i].horizBalanceWidget)
			players[i].horizBalanceWidget->Init(panel, i);
		if (players[i].splitScoreWidget) players[i].splitScoreWidget->Init(*mpPanel, numberFont, i == 0);
		if (players[i].splitMeterWidget) players[i].splitMeterWidget->Init(*mpPanel, g_world_ptr->get_ks_controller(i)->get_special_meter(), i == 0);
	}
	
	OnViewportChange();

	prevWaveIndicatorState = WaveIndicatorWidget::STATE_NONE;
	prevIconModeState = false;

	photos.waitingOnMenu = -1;
	highScore.waitingOnMenu = -1;
	save.waitingOnMenu = -1;
}

// OnSurferReset()
// Called when the specified player's physics are reset.
void IGOFrontEnd::OnSurferReset(const int playerIdx, const bool waveDirection)
{
	assert(playerIdx >= 0 && playerIdx < numPlayers);

	//if (TIMER_GetLevelSec() < 10.0f)
	{
		if (waveIndicatorWidget) waveIndicatorWidget->ShowDirection(true, WAVE_GetHeight(), WAVE_GetNextHeight());
	}
	if (cameraWidget) cameraWidget->Reset();
	if (analogClockWidget) analogClockWidget->HideElapsedTime();	
	
	if (players[playerIdx].trickText)
	{
		players[playerIdx].trickText->checkTime = true;
		players[playerIdx].trickText->time = 0.0f;
		players[playerIdx].trickText->changeText("");
	}
	if (players[playerIdx].trickBurstText)
	{
		players[playerIdx].trickBurstText->checkTime = true;
		players[playerIdx].trickBurstText->time = 0.0f;
		players[playerIdx].trickBurstText->changeText("");
	}
	if (players[playerIdx].trickPointText)
	{
		players[playerIdx].trickPointText->checkTime = true;
		players[playerIdx].trickPointText->time = 0.0f;
		players[playerIdx].trickPointText->changeText("");
	}
	if (players[playerIdx].pointBurstText)
	{
		players[playerIdx].pointBurstText->checkTime = true;
		players[playerIdx].pointBurstText->time = 0.0f;
		players[playerIdx].pointBurstText->changeText("");
	}
	if (players[playerIdx].trickAttrText)
	{
		players[playerIdx].trickAttrText->checkTime = true;
		players[playerIdx].trickAttrText->time = 0.0f;
		players[playerIdx].trickAttrText->changeText("");
	}
}

void IGOFrontEnd::OnModeReset(void)
{
	game_mode_t	mode = g_game_ptr->get_game_mode();
	
	if (mode == GAME_MODE_TIME_ATTACK)
	{
		firstTimeAttackState = true;
		runState = RUNSTATE_TIME_ATTACK;
	}
	else if (mode == GAME_MODE_METER_ATTACK)
	{
		firstMeterAttackState = true;
		runState = RUNSTATE_METER_ATTACK;
	}
	
	if (photoWidget) photoWidget->Reset();
}

// time_value_t measures seconds
void IGOFrontEnd::Update(time_value_t time_inc)
{
	if (debug_menu_displayed || manager->IsMenuActivated() || !manager->map->IsSlideOutDone())
		return;
	
	// Normal play: update heads up displays.
	if (runState == RUNSTATE_NORMAL)
	{
		UpdateStateNormal(time_inc);
	}
	// End run state: display completed goals.
	else if (runState == RUNSTATE_GOALS)
	{
		UpdateStateGoals(time_inc);
	}
	// End run state: display unlocked boards/beaches/etc.
	else if (runState == RUNSTATE_REWARDS)
	{
		UpdateStateRewards(time_inc);
	}
	// End run state: display stats for this run.
	else if (runState == RUNSTATE_STATS)
	{
		UpdateStateStats(time_inc);
	}
	// End run state: display high score menu.
	else if (runState == RUNSTATE_HIGHSCORE)
	{
		UpdateStateHighScore(time_inc);
	}
	// End run state: display photoshoot results.
	else if (runState == RUNSTATE_PHOTOS)
	{
		UpdateStatePhotos(time_inc);
	}
	// End run state: save career.
	else if (runState == RUNSTATE_SAVE)
	{
		UpdateStateSave(time_inc);
	}
	// End run state: display competition results
	else if (runState == RUNSTATE_JUDGE || runState == RUNSTATE_RANK || runState == RUNSTATE_MEDAL)
	{
		UpdateStateComp(time_inc);
	}
	// Push state: display winner of Push mode.
	else if (runState == RUNSTATE_PUSH)
	{
		UpdateStatePush(time_inc);
	}
	// Head to head state: display winner of Head to Head mode.
	else if (runState == RUNSTATE_HEAD_TO_HEAD)
	{
		UpdateStateHeadToHead(time_inc);
	}
	// Time attack state: display winner of Time Attack mode.
	else if (runState == RUNSTATE_TIME_ATTACK)
	{
		UpdateStateTimeAttack(time_inc);
	}
	// Meter attack state: display winner of Meter Attack mode.
	else if (runState == RUNSTATE_METER_ATTACK)
	{
		UpdateStateMeterAttack(time_inc);
	}
	else
		UpdateStateLast(time_inc);
}

void IGOFrontEnd::UpdateStateNormal(time_value_t time_inc)
{
	int							i;
	bool						interesting = false;

#if defined(DEBUG) && defined(TOBY)
	//SetDebugText(stringx(g_world_ptr->get_ks_controller(0)->GetCurrentTrick()));
#endif
	
	// Convert level time elapsed to seconds and minutes.
	if (g_game_ptr->get_game_mode() == GAME_MODE_PUSH)
	{
		if (g_world_ptr->get_ks_controller(0)->get_super_state() != SUPER_STATE_CPU_CONTROLLED &&
			g_world_ptr->get_ks_controller(1)->get_super_state() != SUPER_STATE_CPU_CONTROLLED)

		{
			clock_sec = TIMER_IsInfiniteDuration() ? 0 : TIMER_GetTotalSec();
			clock_min = (int) (clock_sec / 60);
			clock_sec -= clock_min * 60;
			
			if (centerClockWidget) centerClockWidget->SetTime(clock_min, clock_sec);
		}
	}
	else
	{
		clock_sec = TIMER_IsInfiniteDuration() ? 0 : TIMER_GetRemainingLevelSec();
		if (clock_sec < 0.0f) clock_sec = 0.0f;
		clock_min = (int) (clock_sec / 60);
		clock_sec -= clock_min * 60;
	}
	
	// Set time for time attack mode.
	if (timeAttackWidget)
	{
		float attackSec = g_game_ptr->get_play_mode_time_attack()->GetAttackSec(g_game_ptr->get_active_player());
		int attackMin = int(attackSec/60.0f);
		attackSec -= attackMin*60;
		
		timeAttackWidget->SetTime(clock_min, clock_sec);
		timeAttackWidget->SetAttackTime(attackMin, attackSec);
	}

	// Set time for meter attack mode.
	if (meterAttackWidget)
	{
		int score = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore();
		float attackSec = score/MeterAttackMode::SCORE_ATTACK_STRENGTH;
		if (attackSec > MeterAttackMode::TIME_INITIAL)
			attackSec = MeterAttackMode::TIME_INITIAL;
		int attackMin = int(attackSec/60.0f);
		attackSec -= attackMin*60;
		
		meterAttackWidget->SetTime(clock_min, clock_sec);
		meterAttackWidget->SetAttackTime(attackMin, attackSec);
	}
	
	// Display time.
	if (centerClockWidget) centerClockWidget->SetTime(clock_min, clock_sec);
	
	// Display score.
	if (analogClockWidget) analogClockWidget->SetScore(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore());
	if (timeAttackWidget) timeAttackWidget->SetScore(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore());
	if (meterAttackWidget) meterAttackWidget->SetScore(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore());
	for (i = 0; i < numPlayers; i++)
	{
		if (players[i].splitScoreWidget) players[i].splitScoreWidget->SetScore(g_world_ptr->get_ks_controller(i)->get_my_scoreManager().GetScore());
	}
	
	// Don't end the run if a player is still doing a trick.
	for (i = 0; i < numPlayers; i++)
	{
		kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(i);
		
		if (ksctrl && ksctrl->is_active()/* && (ksctrl->get_current_state() != STATE_CELEBRATE)
			&& (ksctrl->get_current_state() != STATE_DISAPPOINTED) && (ksctrl->get_current_state() != STATE_DONTCELEBRATE)*/)
		{
			ksctrl->check_celebration();
			if (ksctrl->IsDoingSomething())
			{
				interesting = true;
			}
		}
	}
	
	// Notice end run in single player modes.
	if (numPlayers == 1)
	{
		if (!TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() <= 0.0f &&
			!WAVE_IsStatic() &&                     // retarted Activision hack request
			!interesting &&
			!replay_mode
			)
		{
			// check for celebration / disappointment
			//kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
			//ksctrl->check_celebration();
			
			//if ((ksctrl->get_super_state() != SUPER_STATE_CPU_CONTROLLED) || ksctrl->IsCelebrationDone())
			{
				EndRun();
			}
		}
	}
	// Notice end run in multiplayer modes.
	else
	{
		// Notice end run in push mode.
		if (g_game_ptr->get_game_mode() == GAME_MODE_PUSH && !g_game_ptr->is_paused() && !interesting)
		{
			// End run when time runs out or on fatality.
			if ((!TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() <= 0.0f) ||
				(!g_world_ptr->get_ks_controller(0)->is_active() || !g_world_ptr->get_ks_controller(1)->is_active()))
			{
				if (g_game_ptr->get_play_mode_push()->InCombat())
					g_game_ptr->get_play_mode_push()->FinishCombat();
				else
					EndRun();
			}
		}
		
		// End run in time attack mode.
		if (g_game_ptr->get_game_mode() == GAME_MODE_TIME_ATTACK && !g_game_ptr->is_paused())
		{
			if (!interesting && !ksreplay.IsPlaying() && !TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() <= 0.0f)
			{
				EndRun();
			}
		}
		
		// End run in head to head mode.
		if (g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD && !g_game_ptr->is_paused())
		{
			if (!interesting && !ksreplay.IsPlaying() && !TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() <= 0.0f)
			{
				EndRun();
			}
		}
	}
	
	// End run if failure during Tetris Icon Mode - now only in freesurf icon
	if(g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON && iconManager && iconManager->Failed() && !replay_mode)
		EndRun();
	
	//  End run if the tutorial is finished.
	if(tutorialManager && tutorialManager->Finished() && !interesting && !replay_mode)
		EndRun();
	
	// Make wave indicator show wave changes.
	if (waveIndicatorWidget)
	{
		WaveBreakInfoStruct wbi;
		
		WAVE_GetBreakInfo(&wbi);
		if (wbi.onoff && wbi.stage >= WAVE_PerturbStageDo && wbi.stage < WAVE_PerturbStageUndo &&
			waveIndicatorWidget->GetState() == WaveIndicatorWidget::STATE_NONE)
		{
			if (wbi.type == WAVE_PerturbTypeRush || wbi.type == WAVE_PerturbTypeBigRush ||
				wbi.type == WAVE_PerturbTypeSurge || wbi.type == WAVE_PerturbTypeBigSurge)
			{
				waveIndicatorWidget->ShowSurge();
			}
			else if (wbi.type == WAVE_PerturbTypeTongue || wbi.type == WAVE_PerturbTypeBigTongue)
			{
				waveIndicatorWidget->ShowTongue();
			}
		}
		else if ((!wbi.onoff || wbi.stage >= WAVE_PerturbStageUndo) &&
			(waveIndicatorWidget->GetState() == WaveIndicatorWidget::STATE_SURGE || waveIndicatorWidget->GetState() == WaveIndicatorWidget::STATE_TONGUE) &&
			!waveIndicatorWidget->IsHiding())
		{
			waveIndicatorWidget->Hide();
		}
	}

	// Notice when wave indicator changes for trick text.
	if (waveIndicatorWidget)
	{
		if (prevWaveIndicatorState != waveIndicatorWidget->GetState())
			OnViewportChange();
		prevWaveIndicatorState = waveIndicatorWidget->GetState();
	}

	// Notice icon mode state changes for trick text.
	if (iconManager)
	{
		if (prevIconModeState != iconManager->Failed())
			OnViewportChange();
		prevIconModeState = iconManager->Failed();
	}
	
	// Update general text.
	if (otherText) otherText->Update(time_inc);
	if (otherText && otherText->time <= 0.0f)
	{
		soundMessageObject * msg_text = printQueue.pop();
		if (msg_text)
		{
			MakeOtherText(msg_text->str);
			if (msg_text->e != SS_LAST)
			{
				SoundScriptManager::inst()->playEvent(msg_text->e);
			}
		}
	}
	
	// Update general widgets.
	if (analogClockWidget) analogClockWidget->Update(time_inc);
	if (centerClockWidget) centerClockWidget->Update(time_inc);
	if (splitterWidget) splitterWidget->Update(time_inc);
	if (waveIndicatorWidget) waveIndicatorWidget->Update(time_inc);
	if (bigWaveMeterWidget) bigWaveMeterWidget->Update(time_inc);
	if (cameraWidget) cameraWidget->Update(time_inc);
	if (photoWidget) photoWidget->Update(time_inc);
	if (replayWidget) replayWidget->Update(time_inc);
	if (timeAttackWidget) timeAttackWidget->Update(time_inc);
	if (meterAttackWidget) meterAttackWidget->Update(time_inc);
	if (iconRadarWidget) iconRadarWidget->Update(time_inc);
	if (iconCountWidget) iconCountWidget->Update(time_inc);
	if (skillChallengeWidget) skillChallengeWidget->Update(time_inc);
	if (meterChallengeWidget) meterChallengeWidget->Update(time_inc);
	if (objectAlertWidget) objectAlertWidget->Update(time_inc);
	
	// Update player IGOs.
	for (i = 0; i < numPlayers; i++)
	{
		kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(i);
		
		if (!ksctrl->is_active())
			continue;
		
		// Update player's widgets.
		if (players[i].meterWidget) players[i].meterWidget->Update(time_inc);
		if (players[i].horizBalanceWidget) players[i].horizBalanceWidget->Update(time_inc);
		if (players[i].splitScoreWidget) players[i].splitScoreWidget->Update(time_inc);
		if (players[i].splitMeterWidget) players[i].splitMeterWidget->Update(time_inc);
		

		// Update player's text during normal situations.
		if (players[i].trickText && !players[i].trickText->IsRand()) players[i].trickText->Update(time_inc);
		if (players[i].trickBurstText) players[i].trickBurstText->Update(time_inc);
		if (players[i].pointBurstText) players[i].pointBurstText->Update(time_inc);
		if (players[i].trickPointText && !players[i].trickPointText->IsRand()) players[i].trickPointText->Update(time_inc);
		if (players[i].trickAttrText) players[i].trickAttrText->Update(time_inc);

		// Update player's text during a wipeout: make trick text do break apart water
		// effect once the camera goes underwater.
		if (players[i].trickPointText && players[i].trickPointText->IsRand() &&
			players[i].trickText && players[i].trickText->IsRand())
		{
			//WAVE_CameraChecks(i);
			if (UNDERWATER_CameraUnderwater(i))
			{
				players[i].trickText->color = COLOR_BLOOD;
				players[i].trickText->no_color = false;
				players[i].trickText->Update(time_inc);
				
				players[i].trickPointText->color = COLOR_BLOOD;
				players[i].trickPointText->Update(time_inc);
			}
		}
	}
	
	// Make challenge text bounce.
	if (otherText)
	{
		// all this is to make the other text pop up (animate)
		float total_time = 0.3f;
		float peak_time = (1.2f/1.4f)*total_time;
		float slope = 1.4f/total_time;
		float offset = 2.4f;
		float current_time = TEXT_WAIT_MAX-otherText->time;
		if(current_time <= peak_time)
			otherText->changeScale(slope*current_time*MAX_TEXT_SCALE);
		else if(current_time < total_time)
			otherText->changeScale((-slope*current_time+offset)*MAX_TEXT_SCALE);
		else otherText->changeScale(MAX_TEXT_SCALE);
	}
	
	if (mpPanel)  mpPanel->Update(time_inc);
	if (compPanel) compPanel->Update(time_inc);
	if (iconManager) 
		iconManager->Update(time_inc);
	if (learn_new_trickManager) 
		learn_new_trickManager->Update(time_inc);
	if (tutorialManager) 
		tutorialManager->Update(time_inc);
	if (hintManager) 
		hintManager->Update(time_inc);
	FrontEnd::Update(time_inc);
}

void IGOFrontEnd::UpdateStateGoals(time_value_t time_inc)
{
	bool advanceState = false;
	
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		// Enable just-in-time initialization of goal state.
		if (goals.status[0]->getText().empty())
		{
			SetDisplay(false);
			ShowAccompBackground(true);
			goals.draw = true;
			
			for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
			{
				if(CareerDataArray[g_game_ptr->get_level_id()].goal[g] != GOAL_NOTHING)
				{
					if (g_career->levels[g_game_ptr->get_level_id()].IsGoalDone(g))
					{
						goals.status[g]->changeText(ksGlobalTextArray[GT_GOALS_DONE]);
						goals.status[g]->color = COLOR_HI;
					}
					else
					{
						goals.status[g]->changeText(ksGlobalTextArray[GT_GOALS_NOT_DONE]);
						goals.status[g]->color = COLOR_ALT;
					}
				}
			}
		}
		
		// Check for user input.
		if (proceedPressed && !GetProceedButtonState())
			proceedPressed = false;
		if (!proceedPressed && GetProceedButtonState())
		{
			SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			SoundScriptManager::inst()->pause();
			proceedPressed = true;
			advanceState = true;
		}
	}
	else
		advanceState = true;

	if (advanceState)
	{
		ShowAccompBackground(false);
		goals.draw = false;
		if (goals.status[0]) goals.status[0]->changeText("");
		runState = RUNSTATE_REWARDS;
		UpdateStateRewards(time_inc);
	}
}

void IGOFrontEnd::UpdateStateRewards(time_value_t time_inc)
{
	stringx	reward_text;
	int		g;
	bool	advanceState = true;

	/*
	int extra_lines = 0;
	int spacing = 55;
	for (int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
	{
		g_career->GetGoalText(g_game_ptr->get_level_id(), g, s);
		int y = POS_GOALS_TOP + (g*spacing) + extra_lines*20;
		goals.names[g] = NEW BoxText(stdFont, s, 320, y, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
		extra_lines += goals.names[g]->makeBox(502, 200) - 1;
		y = IGOFrontEnd::POS_GOALS_TOP + (g*spacing) + (extra_lines+1)*20;
		goals.status[g] = NEW TextString(stdFont, "", 320, y, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, COLOR_STANDARD);
	*/

	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_career->WasNewGoalPassed())
	{
		if (!rewards.initialized)
		{
			int		next_reward = 0; // the next reward text object to load text into
			int		lines = 0;
			int		spacing = 30;
			int		width = 502;
			int		height = 200;
			bool	reminders[REWARD_LAST];
			bool	superReminder = false;
			bool	movieReminder = false;
			
			// first time through, so initialize the reward text
			SetDisplay(false);
			ShowAccompBackground(true);
			rewards.draw = true;

			// No reminders yet.
			for (g = 0; g < REWARD_LAST; g++)
				reminders[g] = false;

			if (g_career->WasNewGoalPassed(0)) // if the first goal was passed on this run
			{
				int num_new_levels = g_career->WasNewLevelUnlocked();
				int num_new_beaches = g_career->WasNewBeachUnlocked();

				// Find out how many new beaches were unlocked
				if (num_new_beaches != 0)
				{
					// get the text for this reward
					if (num_new_beaches > 1)
						rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_NEW_BEACHES_UNLOCKED]);
					else
						rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_NEW_BEACH_UNLOCKED]);
					rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
					lines += rewards.names[next_reward]->makeBox(width, height);
					rewards.names[next_reward]->color = COLOR_HI;
					advanceState = false;
					next_reward++;
				}
				else if (num_new_levels != 0)  // find out how many new levels were unlocked
				{
					// get the text for this reward
					if(num_new_levels > 1)
						rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_NEW_LEVELS_UNLOCKED]);
					else
						rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_NEW_LEVEL_UNLOCKED]);
					rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
					lines += rewards.names[next_reward]->makeBox(width, height);
					rewards.names[next_reward]->color = COLOR_HI;
					advanceState = false;
					next_reward++;
				}

			}

			if (g_career->WasSuperBoardUnlocked())
			{
				rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_SUPER_BOARD_UNLOCKED]);
				rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
				lines += rewards.names[next_reward]->makeBox(width, height);
				rewards.names[next_reward]->color = COLOR_HI;
				next_reward++;
				advanceState = false;
				superReminder = true;
			}
			
			if (g_career->WasEspnMovieUnlocked())
			{
				rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_ESPN_MOVIE_UNLOCKED]);
				rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
				lines += rewards.names[next_reward]->makeBox(width, height);
				rewards.names[next_reward]->color = COLOR_HI;
				next_reward++;
				advanceState = false;
				movieReminder = true;
			}
			
			if (g_career->WasBailsMovieUnlocked())
			{
				rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_BAILS_MOVIE_UNLOCKED]);
				rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
				lines += rewards.names[next_reward]->makeBox(width, height);
				rewards.names[next_reward]->color = COLOR_HI;
				next_reward++;
				advanceState = false;
				movieReminder = true;
			}
			
			// Now deal with rewards other than new levels and beaches
			for (g = 0; g < MAX_GOALS_PER_LEVEL; g++)
			{
				if (CareerDataArray[g_game_ptr->get_level_id()].reward[g] != REWARD_NOTHING)
				{
					if (g_career->WasNewGoalPassed(g)) // if this goal was passed on this run
					{
						// get the text for this reward
						if (g_career->GetRewardText(g_game_ptr->get_level_id(), g, reward_text))
						{
							rewards.names[next_reward]->changeText(reward_text);
							rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
							lines += rewards.names[next_reward]->makeBox(width, height);
							rewards.names[next_reward]->color = COLOR_HI;
							advanceState = false;
							next_reward++;

							// Save reminders.
							reminders[CareerDataArray[g_game_ptr->get_level_id()].reward[g]] = true;
						}
					}
				}
			}

			// Add a space between rewards and reminders.
			lines++;

			// Show reminder text for standard rewards.
			for (g = 0; g < REWARD_LAST; g++)
			{
				if (reminders[g] && !ksGlobalTextArray[GT_REMIND_NOTHING+g].empty())
				{
					rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_REMIND_NOTHING+g]);
					rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
					lines += rewards.names[next_reward]->makeBox(width, height);
					rewards.names[next_reward]->color = COLOR_STANDARD;
					next_reward++;
				}
			}

			// Show reminder for super board.
			if (superReminder)
			{
				rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_REMIND_SUPER_BOARD]);
				rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
				lines += rewards.names[next_reward]->makeBox(width, height);
				rewards.names[next_reward]->color = COLOR_STANDARD;
				next_reward++;
			}

			// Show reminder for extra movies.
			if (movieReminder)
			{
				rewards.names[next_reward]->changeText(ksGlobalTextArray[GT_REMIND_MOVIE]);
				rewards.names[next_reward]->changePos(320, POS_REWARDS_TOP + lines*spacing);
				lines += rewards.names[next_reward]->makeBox(width, height);
				rewards.names[next_reward]->color = COLOR_STANDARD;
				next_reward++;
			}

			// Clear out the text for the rest of the rewards.
			for (g = next_reward; g < MAX_REWARDS; g++)
				rewards.names[g]->changeText("");

			rewards.initialized = true;
		}
		else
			advanceState = false;

		// Check for user input.
		if (proceedPressed && !GetProceedButtonState())
			proceedPressed = false;
		if (!proceedPressed && GetProceedButtonState())
		{
			SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			SoundScriptManager::inst()->pause();
			proceedPressed = true;
			advanceState = true;
		}
	}

	if (advanceState)
	{
		ShowAccompBackground(false);
		rewards.draw = false;
		rewards.initialized = false;
		runState = RUNSTATE_STATS;
		UpdateStateStats(time_inc);
	}
}

void IGOFrontEnd::UpdateStateStats(time_value_t time_inc)
{
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
	bool	advanceState = false;
	
	if (!tutorialManager &&
		(g_game_ptr->get_game_mode() == GAME_MODE_CAREER ||
		 g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_HIGHSCORE ||
		 g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON))
	{
		// Enable just-in-time initialization.
		if (stats.totalNumerals->getText().empty())
		{
			const ScoringManager::Chain & combo = ksctrl->get_my_scoreManager().GetBestChain();
			int			comboPoints = ksctrl->get_my_scoreManager().GetBestChainScore();
			stringx		s;
			int			lineFlags = GridWidget::H01 | GridWidget::H02 | GridWidget::H03 | GridWidget::H04;

			if (stats.iconsText)
				lineFlags |= GridWidget::H05;
			
			SetDisplay(false);
			ShowAccompBackground(true, lineFlags);
			stats.draw = true;
			
			stats.totalNumerals->changeText(stringx(ksctrl->get_my_scoreManager().GetScore()));
			s.printf("%.1f %s", ksctrl->get_my_scoreManager().GetLongestTubeRide(), ksGlobalTextArray[GT_STATS_SECONDS].c_str());
			stats.tubeNumerals->changeText(s);
			stats.tricksNumerals->changeText(stringx(ksctrl->get_my_scoreManager().GetNumTrickLandings()));
			if (stats.iconsNumerals) stats.iconsNumerals->changeText(stringx(iconManager->IconsCleared()));
			s.printf(ksGlobalTextArray[GT_STATS_COMBO_TRICKS].c_str(), combo.GetNumTricks(), comboPoints);
			stats.comboNumerals->changeText(s);
			stats.comboDetails->SetSize(POS_STATS_RIGHT-POS_STATS_LEFT-40, 160);
			stats.comboDetails->MakeTrickBox(combo, true);
		}
		
		// Check for user input.
		if (proceedPressed && !GetProceedButtonState())
			proceedPressed = false;
		if (!proceedPressed && GetProceedButtonState())
		{
			SoundScriptManager::inst()->unpause();
			SoundScriptManager::inst()->playEvent(SS_FE_ONX);
			SoundScriptManager::inst()->pause();
			proceedPressed = true;
			advanceState = true;
		}
	}
	else
		advanceState = true;

	if (advanceState)
	{
		ShowAccompBackground(false);
		stats.draw = false;
		stats.totalNumerals->changeText("");
		runState = RUNSTATE_HIGHSCORE;
		UpdateStateHighScore(time_inc);
	}
}

void IGOFrontEnd::UpdateStateHighScore(time_value_t time_inc)
{
	int		score = 0;
	int		icons = 0;
	bool	advanceState = false;
	bool	popup = true;
	
	if (highScore.waitingOnMenu == -1)
	{
		score = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_my_scoreManager().GetScore();
		if (iconManager)
			icons = iconManager->IconsCleared();
		
		// Only popup menu in highscore mode, or in career mode but not tutorial. (dc 06/10/02)
		if (HighScoreFrontEnd::IsNewHighScore(score, icons))
		{
			popup = (g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_HIGHSCORE ||
				g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON ||
				(g_game_ptr->get_game_mode() == GAME_MODE_CAREER && !tutorialManager));
		}
		else
			popup = false;
	}
	
	if (popup)
	{
		// First frame: popup high score menu.
		if ((manager->pms->active != PauseMenuSystem::HighScoreMenu || !manager->pms->draw) &&
			highScore.waitingOnMenu == -1)
		{
			ShowAccompBackground(true);
			manager->pms->startDraw(PauseMenuSystem::HighScoreMenu);
			manager->pms->UpdateButtonDown();
			highScore.waitingOnMenu = manager->pms->active;
		}
		// Wait for high score menu to finish.
		else if (highScore.waitingOnMenu != -1 && !manager->pms->draw)
		{
			advanceState = true;
		}
	}
	else
	{
		advanceState = true;
	}

	// High score menu finished, proceed with normal end run states.
	if (advanceState)
	{
		ShowAccompBackground(false);
		highScore.waitingOnMenu = -1;
		runState = RUNSTATE_PHOTOS;
		UpdateStatePhotos(time_inc);
	}
}

void IGOFrontEnd::UpdateStatePhotos(time_value_t time_inc)
{
	bool	advanceState = false;
	bool	popup = true;
	
	if (photos.waitingOnMenu == -1)
	{
		// Only popup menu in career mode beaches with photo challenges.
		if (g_game_ptr->get_game_mode() != GAME_MODE_CAREER || !g_beach_ptr->get_challenges()->photo)
			popup = false;
		// Only popup menu if some photos were taken this run.
		else if (g_beach_ptr->get_challenges()->photo->GetNumTaken() == 0)
			popup = false;
		// If there is nothing to overwrite and we only took one photo, then don't popup menu.
/*		else if (g_beach_ptr->get_challenges()->photo->GetNumTaken() == 1 && !g_career->PhotoExistsForLevel (g_game_ptr->get_level_id()))
			popup = false;*/
	}
	
	if (popup)
	{
		// First frame: popup photo menu.
		if ((manager->pms->active != PauseMenuSystem::PhotoMenu || !manager->pms->draw) &&
			photos.waitingOnMenu == -1)
		{
			manager->pms->startDraw(PauseMenuSystem::PhotoMenu);
			manager->pms->UpdateButtonDown();
			photos.waitingOnMenu = manager->pms->active;
			ShowAccompBackground(true);
		}
		// Wait for photo menu to finish.
		else if (photos.waitingOnMenu != -1 && !manager->pms->draw)
		{
			advanceState = true;
		}
	}
	else
	{
		advanceState = true;
	}

	// Photo menu finished, proceed with normal end run states.
	if (advanceState)
	{
		ShowAccompBackground(false);
		photos.waitingOnMenu = -1;
		runState = RUNSTATE_SAVE;
		UpdateStateSave(time_inc);
	}
}

void IGOFrontEnd::UpdateStateSave(time_value_t time_inc)
{
	int		idx = ((PhotoFrontEnd *) manager->pms->menus[PauseMenuSystem::PhotoMenu])->GetSelectedPhotoIdx();
	bool	advanceState = false;
	bool	popup = true;
	
	if (save.waitingOnMenu == -1)
	{
		// Only popup menu in career mode.
		if (g_game_ptr->get_game_mode() != GAME_MODE_CAREER)
			popup = false;
		// Only popup menu when we have something to save.
		else if (idx == -1 && !(manager->unsaved_career))
			popup = false;
	}
	
	if (popup)
	{
		// First frame: popup save menu.
		if ((manager->pms->active != PauseMenuSystem::SaveConfirmMenu || !manager->pms->draw) &&
			save.waitingOnMenu == -1)
		{
			manager->pms->startDraw(PauseMenuSystem::SaveConfirmMenu);
			manager->pms->UpdateButtonDown();
			save.waitingOnMenu = manager->pms->active;
		}
		// Wait for photo menu to finish.
		else if (save.waitingOnMenu != -1 && !manager->pms->draw)
		{
			advanceState = true;
		}
	}
	else
	{
		advanceState = true;
	}

	// Save menu finished, proceed with normal end run states.
	if (advanceState)
	{
		save.waitingOnMenu = -1;
		runState = RUNSTATE_LAST;
		UpdateStateLast(time_inc);
	}
}

void IGOFrontEnd::UpdateStateComp(time_value_t time_inc)
{
	bool	advanceState = false;
	
	// Judge State: show player's run score and wait before showing all results.
	if (runState == RUNSTATE_JUDGE)
	{
		if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->is_competition_level())
		{
			// Initialize "your score" text.
			if (competition.faceScoreNumerals->getText().empty())
			{
				JudgingSystem::RESULT *	results = g_beach_ptr->judges.GetCompetitionResults();
				int						runIdx;
				int						surferIdx = g_game_ptr->GetSurferIdx(0);
				stringx					s;
				
				SetDisplay(false);
				ShowAccompBackground(true);
				competition.draw = true;
				g_beach_ptr->judges.JudgeRun();
				runIdx = g_beach_ptr->judges.GetCurrRunIdx()-1;
				if (runIdx == -1)
					runIdx = 2;

				s.printf("%.1f", results[surferIdx].runs[runIdx].regionScores[0]);
				competition.faceScoreNumerals->changeText(s);
				s.printf("%.1f", results[surferIdx].runs[runIdx].regionScores[1]);
				competition.airScoreNumerals->changeText(s);
				s.printf("%.1f", results[surferIdx].runs[runIdx].regionScores[2]);
				competition.tubeScoreNumerals->changeText(s);
				s.printf("%.1f", results[surferIdx].runs[runIdx].GetAverageScore());
				competition.avgScoreNumerals->changeText(s);
			}
			
			// Check for user input.
			if (proceedPressed && !GetProceedButtonState())
				proceedPressed = false;
			if (!proceedPressed && GetProceedButtonState())
			{
				proceedPressed = true;
				advanceState = true;
			}
		}
		else
			advanceState = true;

		if (advanceState)
		{
			runState = RUNSTATE_RANK;
			competition.sortTimer = 1.0f;
			competition.faceScoreNumerals->changeText("");
			advanceState = false;
		}
	}
	
	// Rank State: show results of all surfers.
	if (runState == RUNSTATE_RANK)
	{
		if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->is_competition_level())
		{
			ShowAccompBackground(true, GridWidget::H00 | GridWidget::H01 | GridWidget::H02 | GridWidget::H03 | GridWidget::H04 | GridWidget::H05 | GridWidget::H06 | GridWidget::H07 | GridWidget::H08 | GridWidget::H09, GridWidget::V00 | GridWidget::V01 | GridWidget::V02 | GridWidget::V03);
			if (competition.numSorted < g_beach_ptr->judges.GetNumCompetitors())
			{
				if (continueWidget)
					continueWidget->Show(false);
			}
			else
			{
				if (continueWidget)
					continueWidget->Show(true);
			}
			
			// Sort another surfer once per second.
			if (competition.numSorted < g_beach_ptr->judges.GetNumCompetitors())
				competition.sortTimer += time_inc;
			if (competition.sortTimer >= 1.0f)
			{
				competition.sortTimer = 0.0f;
				if (competition.numSorted < g_beach_ptr->judges.GetNumCompetitors())
					SortCompetition(competition.numSorted+1);
			}
			
			// Check for user input.
			if (proceedPressed && !GetProceedButtonState())
				proceedPressed = false;
			if (!proceedPressed && GetProceedButtonState())
			{
				proceedPressed = true;
				
				// Show player competition ranking results.
				if (competition.numSorted < g_beach_ptr->judges.GetNumCompetitors())
					SortCompetition(g_beach_ptr->judges.GetNumCompetitors());
				// Done showing ranking, start a new run or pop up menu.
				else
					advanceState = true;
			}
		}
		else
			advanceState = true;

		if (advanceState)
		{
			ShowAccompBackground(false);
			competition.numSorted = 0;
			if (continueWidget)
				continueWidget->Show(false);
			runState = RUNSTATE_MEDAL;
			advanceState = false;
		}
	}

	if (runState == RUNSTATE_MEDAL)
	{
		if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->is_competition_level() &&
			g_beach_ptr->judges.IsCompetitionOver() && competition.results[g_game_ptr->GetSurferIdx(0)].rank < 4)
		{	
			// Just-in-time initialization.
			if (competition.congrat->getText().empty())
			{
				stringx	beach;
				stringx	place;
				stringx	s1, s2;
				int		beachMedalIdx;

				ShowAccompBackground(true);

				if (g_game_ptr->get_beach_id() == BEACH_MAVERICKS)
					beachMedalIdx = 0;
				else if (g_game_ptr->get_beach_id() == BEACH_PIPELINE)
					beachMedalIdx = 1;
				else
					beachMedalIdx = 2;

				// Change text and prize picture based on fist place, second place, etc.
				if (competition.results[g_game_ptr->GetSurferIdx(0)].rank == 1)
				{
					place = ksGlobalTextArray[GT_COMP_FIRST];
					competition.medalWidgets[beachMedalIdx][0]->Show(true);
				}
				else if (competition.results[g_game_ptr->GetSurferIdx(0)].rank == 2)
				{
					place = ksGlobalTextArray[GT_COMP_SECOND];
					competition.medalWidgets[beachMedalIdx][1]->Show(true);
				}
				else
				{
					place = ksGlobalTextArray[GT_COMP_THIRD];
					competition.medalWidgets[beachMedalIdx][2]->Show(true);
				}

				// Build sub text.
				place = stringx("@8")+place+stringx("@9");
				beach = stringx("@9")+stringx(BeachDataArray[g_game_ptr->get_beach_id()].fe_name)+stringx("@9");
				s1 = stringx("@9")+ksGlobalTextArray[GT_COMP_WON];
				
				// Build you won, etc. text.
				competition.congrat->changeText(ksGlobalTextArray[GT_REWARDS_CONGRAT]);
				s2.printf(s1.c_str(), place.c_str(), beach.c_str());
				competition.wonText->changeText(s2);
				competition.wonText->makeBox(240, 200);
			}
			
			// Check for user input.
			if (proceedPressed && !GetProceedButtonState())
				proceedPressed = false;
			if (!proceedPressed && GetProceedButtonState())
			{
				proceedPressed = true;
				advanceState = true;
			}
		}
		else
			advanceState = true;
		
		if (advanceState)
		{
			// Turn stuff off.
			ShowAccompBackground(false);
			if (competition.medalWidgets[0][0]) competition.medalWidgets[0][0]->Show(false);
			if (competition.medalWidgets[0][1]) competition.medalWidgets[0][1]->Show(false);
			if (competition.medalWidgets[0][2]) competition.medalWidgets[0][2]->Show(false);
			if (competition.medalWidgets[1][0]) competition.medalWidgets[1][0]->Show(false);
			if (competition.medalWidgets[1][1]) competition.medalWidgets[1][1]->Show(false);
			if (competition.medalWidgets[1][2]) competition.medalWidgets[1][2]->Show(false);
			if (competition.medalWidgets[2][0]) competition.medalWidgets[2][0]->Show(false);
			if (competition.medalWidgets[2][1]) competition.medalWidgets[2][1]->Show(false);
			if (competition.medalWidgets[2][2]) competition.medalWidgets[2][2]->Show(false);
			if (competition.congrat) competition.congrat->changeText("");
			if (continueWidget) continueWidget->Show(false);
			competition.draw = false;	
			
			// Proceed to next state.
			runState = RUNSTATE_GOALS;
			UpdateStateGoals(time_inc);
		}
	}
}

void IGOFrontEnd::UpdateStatePush(time_value_t time_inc)
{
	bool					advanceState = false;
	int						winnerIdx = 0;
	
	// Enable just-in-time initialization.
	if (push.playerText->getText().empty())
	{
		SetDisplay(false);
		ShowAccompBackground(true);
		push.draw = true;
		
		// Determine winning player.
		assert(numPlayers == 2);
		if (g_game_ptr->get_play_mode_push()->GetPlayerShare(0) >
			g_game_ptr->get_play_mode_push()->GetPlayerShare(1))
			winnerIdx = 0;
		else if (g_game_ptr->get_play_mode_push()->GetPlayerShare(1) >
			g_game_ptr->get_play_mode_push()->GetPlayerShare(0))
			winnerIdx = 1;
		else
			winnerIdx = -1;
		
		
		// Display winner text.
		if (winnerIdx == 0)
			push.playerText->changeText(ksGlobalTextArray[GT_MULTI_P1_WIN]);
		else if (winnerIdx == 1)
			push.playerText->changeText(ksGlobalTextArray[GT_MULTI_P2_WIN]);
		else if (winnerIdx == -1)
			push.playerText->changeText(ksGlobalTextArray[GT_MULTI_TIE]);
		else
			assert(false);
		
		// Display other text.	
		if (winnerIdx != -1)
		{
			push.timeText->changeText(GetTimeText(clock_min, clock_sec));
		}
		else
			push.timeText->changeText("");
	}
	
	// Check for user input.
	if (proceedPressed && !GetProceedButtonState())
		proceedPressed = false;
	if (!proceedPressed && GetProceedButtonState())
	{
		proceedPressed = true;
		advanceState = true;
	}
	
	// Proceed to next IGO state?
	if (advanceState)
	{
		ShowAccompBackground(false);
		push.draw = false;
		push.playerText->changeText("");
		runState = RUNSTATE_LAST;
	}
}

void IGOFrontEnd::UpdateStateHeadToHead(time_value_t time_inc)
{
	ScoringManager &	p1ScoreManager = g_world_ptr->get_ks_controller(0)->get_my_scoreManager();
	ScoringManager &	p2ScoreManager = g_world_ptr->get_ks_controller(1)->get_my_scoreManager();
	bool				advanceState = false;
	int					winnerIdx = 0;
	stringx				s;

	headToHead.winText->Update(time_inc);
	
	// Enable just-in-time initialization.
	if (headToHead.player1Text->getText().empty())
	{
		SetDisplay(false);
		ShowAccompBackground(true, GridWidget::H04 | GridWidget::H05 | GridWidget::H06 | GridWidget::H07 | GridWidget::H09 | GridWidget::H10 | GridWidget::H11 | GridWidget::H12);
		headToHead.draw = true;
		headToHead.player1Text->changeText(ksGlobalTextArray[GT_MULTI_P1]);
		
		// Determine winning player.
		if (p1ScoreManager.GetScore() > p2ScoreManager.GetScore())
			winnerIdx = 0;
		else if (p1ScoreManager.GetScore() < p2ScoreManager.GetScore())
			winnerIdx = 1;
		else
			winnerIdx = -1;
		
		// Build player texts.
		if (winnerIdx == 0)
		{
			//headToHead.player1Text->changeText(ksGlobalTextArray[GT_MULTI_P1_WIN]);
			//headToHead.player1Text->changeScale(1.5f);
			//headToHead.player2Text->changeText(ksGlobalTextArray[GT_MULTI_P2]);
			//headToHead.player2Text->changeScale(1.0f);
			headToHead.winText->changeText(ksGlobalTextArray[GT_MULTI_P1_WIN]);
			headToHead.winText->Bounce(1.5f, 0.5f);
			headToHead.player1ScoreText->color = COLOR_ALT;
			headToHead.player1CountText->color = COLOR_ALT;
			headToHead.player1ComboText->color = COLOR_ALT;
			headToHead.player2ScoreText->color = COLOR_STANDARD;
			headToHead.player2CountText->color = COLOR_STANDARD;
			headToHead.player2ComboText->color = COLOR_STANDARD;
		}
		else if (winnerIdx == 1)
		{
			//headToHead.player1Text->changeText(ksGlobalTextArray[GT_MULTI_P1]);
			//headToHead.player1Text->changeScale(1.0f);
			//headToHead.player2Text->changeText(ksGlobalTextArray[GT_MULTI_P2_WIN]);
			//headToHead.player2Text->changeScale(1.5f);
			headToHead.winText->changeText(ksGlobalTextArray[GT_MULTI_P2_WIN]);
			headToHead.winText->Bounce(1.5f, 0.5f);
			headToHead.player1ScoreText->color = COLOR_STANDARD;
			headToHead.player1CountText->color = COLOR_STANDARD;
			headToHead.player1ComboText->color = COLOR_STANDARD;
			headToHead.player2ScoreText->color = COLOR_ALT;
			headToHead.player2CountText->color = COLOR_ALT;
			headToHead.player2ComboText->color = COLOR_ALT;
		}
		else
		{
			//headToHead.player1Text->changeText(ksGlobalTextArray[GT_MULTI_P1_TIE]);
			//headToHead.player1Text->changeScale(1.0f);
			//headToHead.player2Text->changeText(ksGlobalTextArray[GT_MULTI_P2_TIE]);
			//headToHead.player2Text->changeScale(1.0f);
			headToHead.winText->changeText(ksGlobalTextArray[GT_MULTI_TIE]);
			headToHead.winText->Bounce(1.5f, 0.5f);
			headToHead.player1ScoreText->color = COLOR_STANDARD;
			headToHead.player1CountText->color = COLOR_STANDARD;
			headToHead.player1ComboText->color = COLOR_STANDARD;
			headToHead.player2ScoreText->color = COLOR_STANDARD;
			headToHead.player2CountText->color = COLOR_STANDARD;
			headToHead.player2ComboText->color = COLOR_STANDARD;
		}
		
		// Build stat texts.
		headToHead.player1ScoreNumerals->changeText(stringx(p1ScoreManager.GetScore()));
		headToHead.player1CountNumerals->changeText(stringx(p1ScoreManager.GetNumTrickLandings()));
		s.printf(ksGlobalTextArray[GT_STATS_COMBO_TRICKS].c_str(), p1ScoreManager.GetBestChain().GetNumTricks(), p1ScoreManager.GetBestChain().GetScore());
		headToHead.player1ComboNumerals->changeText(s);
		headToHead.player2ScoreNumerals->changeText(stringx(p2ScoreManager.GetScore()));
		headToHead.player2CountNumerals->changeText(stringx(p2ScoreManager.GetNumTrickLandings()));
		s.printf(ksGlobalTextArray[GT_STATS_COMBO_TRICKS].c_str(), p2ScoreManager.GetBestChain().GetNumTricks(), p2ScoreManager.GetBestChain().GetScore());
		headToHead.player2ComboNumerals->changeText(s);
	}
	
	// Check for user input.
	if (proceedPressed && !GetProceedButtonState())
		proceedPressed = false;
	if (!proceedPressed && GetProceedButtonState())
	{
		proceedPressed = true;
		advanceState = true;
	}
	
	// Proceed to next IGO state?
	if (advanceState)
	{
		ShowAccompBackground(false);
		headToHead.draw = false;
		headToHead.player1Text->changeText("");
		runState = RUNSTATE_LAST;
	}
}

void IGOFrontEnd::UpdateStateTimeAttack(time_value_t time_inc)
{
	TimeAttackMode *	ta = g_game_ptr->get_play_mode_time_attack();
	bool				matchOver = ta->GetSetNum()%2 == 1;
	bool				someoneWon = false;
	int					winnerIdx = 0;
	bool				advanceState = false;
	//int current_controller;
	
	timeAttack.ready->Update(time_inc);
	
	// Enable just-in-time initialization.
	if (timeAttack.player1Score->getText().empty())
	{
		SetDisplay(false);
		ShowAccompBackground(true, GridWidget::H06 | GridWidget::H07 | GridWidget::H08 | GridWidget::H10 | GridWidget::H11 | GridWidget::H12);
		timeAttack.draw = true;
		if (firstTimeAttackState) g_game_ptr->pause();
		timeAttack.player1Score->changeText(ksGlobalTextArray[GT_TIME_ATTACK_SCORE_P1]);
		
		// Player 1 completed his run, but player 2 has not played yet.
		assert(numPlayers == 2);
		if (firstTimeAttackState)
		{
			timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 0 );
			timeAttack.ready->changeText(ksGlobalTextArray[GT_TIME_ATTACK_READY_P1]);
			timeAttack.ready->Bounce(1.7f, 0.5f);
		}
		else if (!matchOver)
		{
			timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 1 );
			timeAttack.ready->changeText(ksGlobalTextArray[GT_TIME_ATTACK_READY_P2]);
			timeAttack.ready->Bounce(1.7f, 0.5f);
		}
		else
		{
			timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 0 );
			timeAttack.device_flags |= 1 << app::inst()->get_game()->get_player_device( 1 );
			timeAttack.ready->changeText("");
		}

		timeAttack.player1Score->color = COLOR_STANDARD;
		timeAttack.player1Time->color = COLOR_STANDARD;
		timeAttack.player2Score->color = COLOR_STANDARD;
		timeAttack.player2Time->color = COLOR_STANDARD;
		
		ta->BeginCombat();
		if (matchOver)
		{
			ta->BeginAttacking(0);
		}
	}
	
	// Determine end-of-match text.
	if (matchOver)
	{
		// If player 1's attack finished without player interaction, start player 2's attack.
		if (!ta->IsAttacking(0) && !ta->IsAttacking(1) && !ta->IsDoneAttacking(1))
		{
			ta->BeginAttacking(1);
		}
		
		// If both players are done attacking, determine if we have a winner.
		if (ta->IsDoneAttacking(0) && ta->IsDoneAttacking(1))
		{
			if (ta->GetRemainingTime(0) <= ta->TIME_LOSE &&
				(ta->GetRemainingTime(1) > ta->GetRemainingTime(0) || ta->GetScore(1) > ta->GetScore(0)))
			{
				someoneWon = true;
				winnerIdx = 1;
				if (timeAttack.ready->getText().empty())
				{
					timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 0 );
					timeAttack.device_flags |= 1 << app::inst()->get_game()->get_player_device( 1 );
					timeAttack.ready->changeText(ksGlobalTextArray[GT_MULTI_P2_WIN]);
					timeAttack.ready->Bounce(1.7f, 0.5f);
				}
				timeAttack.player2Score->color = COLOR_ALT;
				timeAttack.player2Time->color = COLOR_ALT;
			}
			else if (ta->GetRemainingTime(1) <= ta->TIME_LOSE &&
				(ta->GetRemainingTime(0) > ta->GetRemainingTime(1) || ta->GetScore(0) > ta->GetScore(1)))
			{
				someoneWon = true;
				winnerIdx = 0;
				if (timeAttack.ready->getText().empty())
				{
					timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 0 );
					timeAttack.device_flags |= 1 << app::inst()->get_game()->get_player_device( 1 );
					timeAttack.ready->changeText(ksGlobalTextArray[GT_MULTI_P1_WIN]);
					timeAttack.ready->Bounce(1.7f, 0.5f);
				}
				timeAttack.player1Score->color = COLOR_ALT;
				timeAttack.player1Time->color = COLOR_ALT;
			}
			else if (ta->GetRemainingTime(0) <= ta->TIME_LOSE && ta->GetRemainingTime(1) <= ta->TIME_LOSE)
			{
				someoneWon = true;
				winnerIdx = -1;
				if (timeAttack.ready->getText().empty())
				{
					timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 0 );
					timeAttack.device_flags |= 1 << app::inst()->get_game()->get_player_device( 1 );
					timeAttack.ready->changeText(ksGlobalTextArray[GT_MULTI_TIE]);
					timeAttack.ready->Bounce(1.7f, 0.5f);
				}
			}
			else
			{
				someoneWon = false;
				if (timeAttack.ready->getText().empty())
				{
					timeAttack.device_flags = 1 << app::inst()->get_game()->get_player_device( 0 );
					timeAttack.ready->changeText(ksGlobalTextArray[GT_TIME_ATTACK_READY_P1]);
					timeAttack.ready->Bounce(1.7f, 0.5f);
				}
			}
		}
	}
	
	timeAttack.player1TimeNumerals->changeText(GetTimeText(ta->GetRemainingTime(0)));
	if (ta->GetRemainingTime(0) > ta->TIME_LOSE)
		timeAttack.player1TimeNumerals->color = COLOR_HI;
	else
		timeAttack.player1TimeNumerals->color = COLOR_BAD;
	timeAttack.player2TimeNumerals->changeText(GetTimeText(ta->GetRemainingTime(1)));
	if (ta->GetRemainingTime(1) > ta->TIME_LOSE)
		timeAttack.player2TimeNumerals->color = COLOR_HI;
	else
		timeAttack.player2TimeNumerals->color = COLOR_BAD;
	timeAttack.player1ScoreNumerals->changeText(stringx(ta->GetScore(0)));
	timeAttack.player2ScoreNumerals->changeText(stringx(ta->GetScore(1)));
	
	// Check for user input.
	manager->pms->SetDeviceFlags( timeAttack.device_flags );
	
	bool button_state = false;

	// OR all active controllers;
	for( int i = 0; i < NUM_CONTROLLER_PORTS; i++ )
	{
		if( timeAttack.device_flags & ( 1 << i ) )
			button_state |= GetProceedButtonState( i );
	}
	
	if (proceedPressed && !button_state )
		proceedPressed = false;
	if (!proceedPressed && button_state )
	{
		proceedPressed = true;
		
		if (matchOver && ta->IsAttacking(0))
		{
			ta->FinishAttacking(0);
			ta->BeginAttacking(1);
		}
		else if (matchOver && ta->IsAttacking(1))
		{
			ta->FinishAttacking(1);
		}
		else
			advanceState = true;
	}
	
	// Proceed to next IGO state?
	if (advanceState)
	{
		SetDisplay(frontendmanager.score_display);
		ShowAccompBackground(false);
		timeAttack.draw = false;
		timeAttack.player1Score->changeText("");
		if (matchOver)
			g_world_ptr->get_ks_controller(1)->get_my_scoreManager().Reset();
		
		// Continue playing?
		if (!someoneWon)
		{
			g_game_ptr->unpause();			
			runState = RUNSTATE_NORMAL;

			if (!firstTimeAttackState)
			{
				ta->AdvanceSet();
				g_game_ptr->retry_level();
			}
			else
				firstTimeAttackState = false;
			
		}
		// Match is over.
		else
		{
			runState = RUNSTATE_LAST;
		}
	}
}

void IGOFrontEnd::UpdateStateMeterAttack(time_value_t time_inc)
{
	MeterAttackMode *	ma = g_game_ptr->get_play_mode_meter_attack();
	bool				matchOver = ma->GetSetNum()%2 == 1;
	bool				someoneWon = false;
	int					winnerIdx = 0;
	bool				advanceState = false;
	int					current_controller;
	
	meterAttack.ready->Update(time_inc);
	
	if (firstMeterAttackState )
	{
		current_controller = app::inst()->get_game()->get_player_device( 0 );
	}
	else if (!matchOver)
	{
		current_controller = app::inst()->get_game()->get_player_device( 1 );
	}
	else
	{
		current_controller = app::inst()->get_game()->get_player_device( 0 );
	}
	
	// Enable just-in-time initialization.
	if (meterAttack.player1Meter->getText().empty())
	{
		SetDisplay(false);
		ShowAccompBackground(true);
		meterAttack.draw = true;
		if (firstMeterAttackState) g_game_ptr->pause();
		meterAttack.player1Meter->changeText(ksGlobalTextArray[GT_METER_ATTACK_METER_P1]);
		
		// Player 1 completed his run, but player 2 has not played yet.
		assert(numPlayers == 2);
		if (firstMeterAttackState)
		{
			meterAttack.ready->changeText(ksGlobalTextArray[GT_METER_ATTACK_READY_P1]);
			meterAttack.ready->Bounce(1.7f, 0.5f);
		}
		else if (!matchOver)
		{
			meterAttack.ready->changeText(ksGlobalTextArray[GT_METER_ATTACK_READY_P2]);
			meterAttack.ready->Bounce(1.7f, 0.5f);
		}
		else
			meterAttack.ready->changeText("");
		
		ma->BeginCombat();
		if (matchOver)
		{
			ma->BeginAttacking(0);
		}
	}
	
	// Determine end-of-match text.
	if (matchOver)
	{
		// If player 1's attack finished without player interaction, start player 2's attack.
		if (!ma->IsAttacking(0) &&
			!ma->IsAttacking(1) &&
			ma->GetScore(1) != 0)
		{
			ma->BeginAttacking(1);
		}
		
		// If both players are done attacking, determine if we have a winner.
		if (!ma->IsAttacking(0) &&
			!ma->IsAttacking(1))
		{
			if (ma->GetRemainingTime(0) <= ma->TIME_LOSE &&
				ma->GetRemainingTime(1) > ma->TIME_LOSE)
			{
				someoneWon = true;
				winnerIdx = 1;
				if (meterAttack.ready->getText().empty())
				{
					meterAttack.ready->changeText(ksGlobalTextArray[GT_MULTI_P2_WIN]);
					meterAttack.ready->Bounce(1.7f, 0.5f);
				}
			}
			else if (ma->GetRemainingTime(0) > ma->TIME_LOSE &&
				ma->GetRemainingTime(1) <= ma->TIME_LOSE)
			{
				someoneWon = true;
				winnerIdx = 0;
				if (meterAttack.ready->getText().empty())
				{
					meterAttack.ready->changeText(ksGlobalTextArray[GT_MULTI_P1_WIN]);
					meterAttack.ready->Bounce(1.7f, 0.5f);
				}
			}
			else if (ma->GetRemainingTime(0) <= ma->TIME_LOSE &&
				ma->GetRemainingTime(1) <= ma->TIME_LOSE)
			{
				someoneWon = true;
				winnerIdx = -1;
				if (meterAttack.ready->getText().empty())
				{
					meterAttack.ready->changeText(ksGlobalTextArray[GT_MULTI_TIE]);
					meterAttack.ready->Bounce(1.7f, 0.5f);
				}
			}
			else
			{
				someoneWon = false;
				if (meterAttack.ready->getText().empty())
				{
					meterAttack.ready->changeText(ksGlobalTextArray[GT_TIME_ATTACK_READY_P1]);
					meterAttack.ready->Bounce(1.7f, 0.5f);
				}
			}
		}
	}
	
	meterAttack.player1TimeNumerals->changeText(GetTimeText(ma->GetRemainingTime(0)));
	meterAttack.player2TimeNumerals->changeText(GetTimeText(ma->GetRemainingTime(1)));
	meterAttack.player1MeterNumerals->changeText(stringx(ma->GetScore(0)));
	meterAttack.player2MeterNumerals->changeText(stringx(ma->GetScore(1)));
	
	// Check for user input.
	bool button_state;

	if (current_controller == -1)
		button_state = GetProceedButtonState();
	else
		button_state = GetProceedButtonState(current_controller);
	
	if (proceedPressed && !button_state )
		proceedPressed = false;
	if (!proceedPressed && button_state )
	{
		proceedPressed = true;
		
		if (matchOver && ma->IsAttacking(0))
		{
			ma->FinishAttacking(0);
			ma->BeginAttacking(1);
		}
		else if (matchOver && ma->IsAttacking(1))
		{
			ma->FinishAttacking(1);
		}
		else
			advanceState = true;
	}
	
	// Proceed to next IGO state?
	if (advanceState)
	{
		SetDisplay(frontendmanager.score_display);
		ShowAccompBackground(false);
		meterAttack.draw = false;
		meterAttack.player1Meter->changeText("");
		if (matchOver)
			g_world_ptr->get_ks_controller(1)->get_my_scoreManager().Reset();
		
		// Continue playing?
		if (!someoneWon)
		{
			g_game_ptr->unpause();			
			runState = RUNSTATE_NORMAL;

			if (!firstMeterAttackState)
			{
				ma->AdvanceSet();
				g_game_ptr->retry_level();
			}
			else
				firstMeterAttackState = false;
			
		}
		// Match is over.
		else
		{
			runState = RUNSTATE_LAST;
		}
	}
}

void IGOFrontEnd::UpdateStateLast(time_value_t time_inc)
{
	ShowAccompBackground(false);
	runState = RUNSTATE_NORMAL;

	// clear the print queue so no remaining messages will be shown on the next run
	printQueue.clear(); 
	if (otherText) otherText->time = 0.0f;

	// clear the print queue so no remaining messages will be shown on the next run
	printQueue.clear(); 
	if (otherText) otherText->time = 0.0f;

	if (g_game_ptr->is_competition_level() && g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		if (!g_beach_ptr->judges.IsCompetitionOver())
		{
			manager->pms->startDraw(PauseMenuSystem::HeatEndMenu);
			manager->pms->UpdateButtonDown();
		}
		else
		{
			manager->pms->startDraw(PauseMenuSystem::CompEndMenu);
			manager->pms->UpdateButtonDown();
		}
	}
	else
	{
		manager->pms->startDraw(PauseMenuSystem::EndRunMenu);
		manager->pms->UpdateButtonDown();
	}
}

//	OnEvent()
// Responds to all events.
void IGOFrontEnd::OnEvent(const EVENT event, const int param1, const int param2)
{
	if (event == EVT_SURFER_WIPEOUT)
	{
		if (analogClockWidget && param2 == 0) analogClockWidget->ShowElapsedTime(SurfBoardObjectClass::CLOCKTIME_WIPEOUT);
		if (objectAlertWidget) objectAlertWidget->Hide(false);
	}
	else if (event == EVT_SURFER_DUCK_DIVE)
	{
		if (analogClockWidget) analogClockWidget->ShowElapsedTime(kellyslater_controller::CLOCKTIME_DUCK_DIVE);
	}
}

//	EndRun()
// Pauses the game and pops up an appropriate End Level menu.
// Note: some of this functionality should really be placed somewhere else...
void IGOFrontEnd::EndRun(void)
{
	// One-player modes.
	if (numPlayers == 1)
	{
		runState = RUNSTATE_JUDGE;
		g_game_ptr->pause();
	}
	// Multiplayer modes.
	else if (g_game_ptr->get_game_mode() == GAME_MODE_PUSH)
	{
		runState = RUNSTATE_PUSH;
		g_game_ptr->pause();
	}
	// Multiplayer modes.
	else if (g_game_ptr->get_game_mode() == GAME_MODE_TIME_ATTACK)
	{
		runState = RUNSTATE_TIME_ATTACK;
		g_game_ptr->pause();
	}
	// Multiplayer modes.
	else if (g_game_ptr->get_game_mode() == GAME_MODE_METER_ATTACK)
	{
		runState = RUNSTATE_METER_ATTACK;
		g_game_ptr->pause();
	}
	// Multiplayer modes.
	else if (g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD)
	{
		runState = RUNSTATE_HEAD_TO_HEAD;
		g_game_ptr->pause();
	}
	
	((PhotoFrontEnd *) manager->pms->menus[PauseMenuSystem::PhotoMenu])->OnEndRun();
}

//	EndCompetition()
// Pauses the game and pops up and appropriate competition termination menu.
void IGOFrontEnd::EndCompetition(void)
{
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
	
	if (g_game_ptr->is_competition_level() && g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		g_beach_ptr->judges.JudgeRun();
		
		while (!g_beach_ptr->judges.IsCompetitionOver())
		{
			ksctrl->get_my_scoreManager().Reset();
			g_beach_ptr->judges.JudgeRun();
		}
		
		runState = RUNSTATE_RANK;
		competition.sortTimer = 1.0f;
		g_game_ptr->pause();
		competition.draw = true;
	}
}

void IGOFrontEnd::Draw()
{
	int		i;	// dummy var

	/////////////////////////////////////////////////////////////////
	// Draw IGO Widgets
	/////////////////////////////////////////////////////////////////

	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);

	if (display && !replay_mode)
	{
		if (analogClockWidget) analogClockWidget->Draw();
		if (centerClockWidget) centerClockWidget->Draw();
		if (waveIndicatorWidget) waveIndicatorWidget->Draw();
		if (bigWaveMeterWidget) bigWaveMeterWidget->Draw();
		if (cameraWidget) cameraWidget->Draw();
		if (photoWidget) photoWidget->Draw();
		if (timeAttackWidget) timeAttackWidget->Draw();
		if (meterAttackWidget) meterAttackWidget->Draw();
		if (iconRadarWidget) iconRadarWidget->Draw();
		if (iconCountWidget) iconCountWidget->Draw();
		if (skillChallengeWidget) skillChallengeWidget->Draw();
		if (meterChallengeWidget) meterChallengeWidget->Draw();
		if (objectAlertWidget) objectAlertWidget->Draw();
		for (i = 0; i < numPlayers; i++)
		{
			if (!g_world_ptr->get_ks_controller(i)->is_active())
				continue;
			
			if (players[i].meterWidget) players[i].meterWidget->Draw();
			if (players[i].horizBalanceWidget) players[i].horizBalanceWidget->Draw();
			if (players[i].splitScoreWidget) players[i].splitScoreWidget->Draw();
			if (players[i].splitMeterWidget) players[i].splitMeterWidget->Draw();
		}
	}

	if (splitterWidget && !replay_mode) splitterWidget->Draw();
	if (accompWidget && !replay_mode) accompWidget->Draw();
	if (menuBGWidget) menuBGWidget->Draw();
	if (gridWidget && !replay_mode) gridWidget->Draw();
	if (continueWidget && !replay_mode) continueWidget->Draw();
	if (replayWidget && replay_mode) replayWidget->Draw();
	
	nglListEndScene();

	/////////////////////////////////////////////////////////////////
	// Draw IGO Text
	/////////////////////////////////////////////////////////////////

	nglListBeginScene();
	
	// This is always drawn even if IGO is toggled off.
	if (replayText) replayText->Draw();

#ifdef DEBUG
	if (debugText) debugText->Draw();
#endif
	
	// Normal state: display gameplay widgets and text.
	if (display && runState == RUNSTATE_NORMAL && !replay_mode)
	{
		for (i = 0; i < numPlayers; i++)
		{
			if (!g_world_ptr->get_ks_controller(i)->is_active())
				continue;
			
			if (players[i].drawTubeTimer && players[i].tubeTimer) players[i].tubeTimer->Draw();
			
			//  The paused check is because almost every overlay needs to come up in Tutorial, but not this text.
			if (!g_game_ptr->is_paused())
			{
				if (players[i].trickPointText) players[i].trickPointText->Draw();
				if (players[i].trickText) players[i].trickText->Draw();
				if (players[i].trickBurstText) players[i].trickBurstText->Draw();
				if (players[i].pointBurstText) players[i].pointBurstText->Draw();
				if (players[i].trickAttrText) players[i].trickAttrText->Draw();
			}	
		}
		
		if (otherText && otherText->time > 0.0f) otherText->Draw();
		
		if (iconManager) 
			iconManager->Draw();
		if (learn_new_trickManager) 
			learn_new_trickManager->Draw();
		if (tutorialManager) 
			tutorialManager->Draw();
		if (hintManager) 
			hintManager->Draw();
	}
	// Goal state: display completed goals.
	else if (runState == RUNSTATE_GOALS && goals.draw && !replay_mode)
	{
		goals.title->Draw();
		for(int g = 0; g < MAX_GOALS_PER_LEVEL; g++)
		{
			goals.names[g]->Draw();
			goals.status[g]->Draw();
		}
		helpText->Draw();
	}
	// Stat state: display stats for this run.
	else if (runState == RUNSTATE_STATS && stats.draw &&!replay_mode)
	{
		stats.title->Draw();
		stats.totalText->Draw();
		stats.totalNumerals->Draw();
		stats.tubeText->Draw();
		stats.tubeNumerals->Draw();
		stats.tricksText->Draw();
		stats.tricksNumerals->Draw();
		if (stats.iconsText) stats.iconsText->Draw();
		if (stats.iconsNumerals) stats.iconsNumerals->Draw();
		stats.comboText->Draw();
		stats.comboNumerals->Draw();
		stats.comboDetails->Draw();
		helpText->Draw();
	}
	// Judge state: display this heat.
	else if (runState == RUNSTATE_JUDGE && competition.draw && !replay_mode)
	{
		competition.title->Draw();
		competition.faceScoreText->Draw();
		competition.faceScoreNumerals->Draw();
		competition.airScoreText->Draw();
		competition.airScoreNumerals->Draw();
		if (g_game_ptr->get_beach_id() != BEACH_MAVERICKS)
		{
			competition.tubeScoreText->Draw();
			competition.tubeScoreNumerals->Draw();
		}
		competition.avgScoreText->Draw();
		competition.avgScoreNumerals->Draw();
		helpText->Draw();
	}
	// Rank state: display competition rank.
	else if (runState == RUNSTATE_RANK && competition.draw && !replay_mode)
	{
		competition.title->Draw();
		competition.name->Draw();
		competition.heats->Draw();
		competition.total->Draw();
		
		for (i = 0; i < SURFER_LAST; i++)
		{
			// Ignore invalid surfers.
			if (!g_beach_ptr->judges.IsCompetitor(i)) continue;

			// Ignore unranked surfers.
			if (competition.results[i].rank == 0) continue;
			
			competition.results[i].name->Draw();
			competition.results[i].runs[0]->Draw();
			competition.results[i].runs[1]->Draw();
			competition.results[i].runs[2]->Draw();
			competition.results[i].total->Draw();
		}

		helpText->Draw();
	}
	// Medal state: display earned medal or trophy or whatever.
	else if (runState == RUNSTATE_MEDAL && competition.draw && !replay_mode)
	{
		competition.congrat->Draw();
		competition.wonText->Draw();
		if (competition.medalWidgets[0][0]) competition.medalWidgets[0][0]->Draw();
		if (competition.medalWidgets[0][1]) competition.medalWidgets[0][1]->Draw();
		if (competition.medalWidgets[0][2]) competition.medalWidgets[0][2]->Draw();
		if (competition.medalWidgets[1][0]) competition.medalWidgets[1][0]->Draw();
		if (competition.medalWidgets[1][1]) competition.medalWidgets[1][1]->Draw();
		if (competition.medalWidgets[1][2]) competition.medalWidgets[1][2]->Draw();
		if (competition.medalWidgets[2][0]) competition.medalWidgets[2][0]->Draw();
		if (competition.medalWidgets[2][1]) competition.medalWidgets[2][1]->Draw();
		if (competition.medalWidgets[2][2]) competition.medalWidgets[2][2]->Draw();
		helpText->Draw();
	}
	// Push state: display which player won.
	else if (runState == RUNSTATE_PUSH && push.draw && !replay_mode)
	{
		if (push.titleText) push.titleText->Draw();
		if (push.playerText) push.playerText->Draw();
		if (push.timeText) push.timeText->Draw();
		helpText->Draw();
	}
	// Head to head: display which player won.
	else if (runState == RUNSTATE_HEAD_TO_HEAD && headToHead.draw && !replay_mode)
	{
		if (headToHead.titleText) headToHead.titleText->Draw();
		if (headToHead.winText) headToHead.winText->Draw();
		if (headToHead.player1Text) headToHead.player1Text->Draw();
		if (headToHead.player1ScoreText) headToHead.player1ScoreText->Draw();
		if (headToHead.player1ScoreNumerals) headToHead.player1ScoreNumerals->Draw();
		if (headToHead.player1CountText) headToHead.player1CountText->Draw();
		if (headToHead.player1CountNumerals) headToHead.player1CountNumerals->Draw();
		if (headToHead.player1ComboText) headToHead.player1ComboText->Draw();
		if (headToHead.player1ComboNumerals) headToHead.player1ComboNumerals->Draw();
		if (headToHead.player2Text) headToHead.player2Text->Draw();
		if (headToHead.player2ScoreText) headToHead.player2ScoreText->Draw();
		if (headToHead.player2ScoreNumerals) headToHead.player2ScoreNumerals->Draw();
		if (headToHead.player2CountText) headToHead.player2CountText->Draw();
		if (headToHead.player2CountNumerals) headToHead.player2CountNumerals->Draw();
		if (headToHead.player2ComboText) headToHead.player2ComboText->Draw();
		if (headToHead.player2ComboNumerals) headToHead.player2ComboNumerals->Draw();
		helpText->Draw();
	}
	// Time attack state: display current player results.
	else if (runState == RUNSTATE_TIME_ATTACK && timeAttack.draw && !replay_mode)
	{
		timeAttack.title->Draw();
		if (timeAttack.ready) timeAttack.ready->Draw();
		if (timeAttack.player1Text) timeAttack.player1Text->Draw();
		if (timeAttack.player1Score) timeAttack.player1Score->Draw();
		if (timeAttack.player1ScoreNumerals) timeAttack.player1ScoreNumerals->Draw();
		if (timeAttack.player1Time) timeAttack.player1Time->Draw();
		if (timeAttack.player1TimeNumerals) timeAttack.player1TimeNumerals->Draw();
		if (timeAttack.player2Text) timeAttack.player2Text->Draw();
		if (timeAttack.player2Score) timeAttack.player2Score->Draw();
		if (timeAttack.player2ScoreNumerals) timeAttack.player2ScoreNumerals->Draw();
		if (timeAttack.player2Time) timeAttack.player2Time->Draw();
		if (timeAttack.player2TimeNumerals) timeAttack.player2TimeNumerals->Draw();
		helpText->Draw();
	}
	// Meter attack state: display current player results.
	else if (runState == RUNSTATE_METER_ATTACK && meterAttack.draw && !replay_mode)
	{
		meterAttack.title->Draw();
		if (meterAttack.ready) meterAttack.ready->Draw();
		if (meterAttack.player1Meter) meterAttack.player1Meter->Draw();
		if (meterAttack.player1MeterNumerals) meterAttack.player1MeterNumerals->Draw();
		if (meterAttack.player1Time) meterAttack.player1Time->Draw();
		if (meterAttack.player1TimeNumerals) meterAttack.player1TimeNumerals->Draw();
		if (meterAttack.player2Meter) meterAttack.player2Meter->Draw();
		if (meterAttack.player2MeterNumerals) meterAttack.player2MeterNumerals->Draw();
		if (meterAttack.player2Time) meterAttack.player2Time->Draw();
		if (meterAttack.player2TimeNumerals) meterAttack.player2TimeNumerals->Draw();
		helpText->Draw();
	}
	// Reward state: display earned rewards.
	else if (runState == RUNSTATE_REWARDS && rewards.draw && !replay_mode)
	{
		rewards.title->Draw();
		for (i = 0; i < MAX_REWARDS; i++)
			rewards.names[i]->Draw();
		helpText->Draw();
	}
	
	nglListEndScene();
}

void IGOFrontEnd::SetDisplay(const bool disp)
{
	int	i;
	
	display = disp;
	
	// Display widgets.
	if (analogClockWidget) analogClockWidget->SetDisplay(display);
	if (centerClockWidget) centerClockWidget->SetDisplay(display);
	//if (splitterWidget) splitterWidget->SetDisplay(display);
	if (waveIndicatorWidget) waveIndicatorWidget->SetDisplay(display);
	if (bigWaveMeterWidget) bigWaveMeterWidget->SetDisplay(display);
	if (cameraWidget) cameraWidget->SetDisplay(display);
	if (photoWidget) photoWidget->SetDisplay(display);
	if (replayWidget) replayWidget->SetDisplay(display);
	if (timeAttackWidget) timeAttackWidget->SetDisplay(display);
	if (meterAttackWidget) meterAttackWidget->SetDisplay(display);
	if (iconRadarWidget) iconRadarWidget->SetDisplay(display);
	for (i = 0; i < numPlayers; i++)
	{
		if (players[i].meterWidget) players[i].meterWidget->SetDisplay(display);
		if (players[i].horizBalanceWidget) players[i].horizBalanceWidget->SetDisplay(display);
	}
}

void IGOFrontEnd::Print(stringx t, EventType e)
{
	t.to_upper();
	
	if (otherText && otherText->time > 0.0f) 
		printQueue.push(t,e);
	else if (otherText)
	{
		MakeOtherText(t);
		if (e != SS_LAST)
			SoundScriptManager::inst()->playEvent(e);
	}
}

// Note that passing in an empty string will remove the text.
void IGOFrontEnd::SetReplayText(const stringx & t)
{
	if (!t.empty() && replayText)
	{
		replayText->changeText(t);
		replayText->checkTime = false;
	}
}

//	OnTrickChange()
// Event response: refreshes the trick display and points.
void IGOFrontEnd::OnTrickChange(const int playerIdx, const bool bounce)
{
	if (g_game_ptr->get_num_ai_players() && playerIdx)
		return;
	
	assert(playerIdx >= 0 && playerIdx < numPlayers);
	
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(playerIdx);
	int							rawScore = ksctrl->get_my_scoreManager().GetChain().GetRawScore();
	float						multiplier = ksctrl->get_my_scoreManager().GetChain().GetMultiplier();
	stringx						pointText(rawScore);
	
	// Display format: "9999  X9.9"
	if (multiplier > 1.0f)
	{
		if (float(int(multiplier)) == multiplier)
			pointText += stringx(" @@") + stringx(int(multiplier));
		else
		{
			char	s[128] = "";
			sprintf(s, "%.1f", multiplier);
			pointText += stringx(" @@") + s;
		}
	}
	
	// Create new trick texts.
	if(ksctrl->get_my_scoreManager().GetChain().IsInteresting())
	{
		players[playerIdx].trickText->MakeTrickBox(ksctrl->get_my_scoreManager().GetChain(), false, true);
		players[playerIdx].trickText->checkTime = false;
		players[playerIdx].trickText->unmakeRand();
		players[playerIdx].trickText->no_color = true;
		//players[playerIdx].trickBurstText->checkTime = true;
		//players[playerIdx].trickBurstText->time = 0.0f;
		
		MakeTrickPointText(playerIdx, pointText);
		players[playerIdx].trickPointText->checkTime = false;
		//players[playerIdx].pointBurstText->checkTime = true;
		//players[playerIdx].pointBurstText->time = 0.0f;

		MakeTrickAttrText(playerIdx);
	}
}

//	OnTrickComplete()
// Event response: keeps trick display up for a few seconds and display's trick's final score.
void IGOFrontEnd::OnTrickComplete(const int playerIdx)
{
    assert(playerIdx >= 0 && playerIdx < numPlayers);
	
	if (g_game_ptr->get_num_ai_players() && playerIdx)
		return;
	
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(playerIdx);
	
	OnTrickChange(false);
	
	if (ksctrl->get_my_scoreManager().GetChain().IsInteresting())
	{
		players[playerIdx].trickText->no_color = true;
		players[playerIdx].trickText->checkTime = true;
		players[playerIdx].trickText->time = TRICK_WAIT_MAX;
		players[playerIdx].trickBurstText->Burst(players[playerIdx].trickText);
		
		MakeTrickPointText(playerIdx, stringx(ksctrl->get_my_scoreManager().GetChain().GetScore()));
		players[playerIdx].trickPointText->checkTime = true;
		players[playerIdx].trickPointText->time = TRICK_WAIT_MAX;
		players[playerIdx].pointBurstText->Burst(players[playerIdx].trickPointText);

		players[playerIdx].trickAttrText->time = TRICK_WAIT_MAX;
	}
}

//	OnScoreChange()
// Event response: refreshes the score display.
void IGOFrontEnd::OnScoreChange(const int playerIdx)
{
}

//	OnTrickFail()
// Event response: keeps trick display up for a few seconds, display's trick's final score,
// and makes the text bleed off the screen.
void IGOFrontEnd::OnTrickFail(const int playerIdx)
{
	assert(playerIdx >= 0 && playerIdx < numPlayers);
	
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(playerIdx);
	
	if (g_game_ptr->get_num_ai_players() && playerIdx)
		return;
	
	// Make trick text red and have it bleed off screen.
	if (ksctrl->get_my_scoreManager().GetChain().IsInteresting())
	{
		players[playerIdx].trickText->makeRand();
		players[playerIdx].trickText->checkTime = true;
		players[playerIdx].trickText->time = TRICK_WAIT_MAX;
		players[playerIdx].trickText->Break();
		//players[playerIdx].trickBurstText->checkTime = true;
		//players[playerIdx].trickBurstText->time = 0.0f;
		
		MakeTrickPointText(playerIdx, stringx(ksctrl->get_my_scoreManager().GetChain().GetScore()));
		players[playerIdx].trickPointText->makeRand();
		players[playerIdx].trickPointText->checkTime = true;
		players[playerIdx].trickPointText->time = TRICK_WAIT_MAX;
		players[playerIdx].trickPointText->Break();
		//players[playerIdx].pointBurstText->checkTime = true;
		//players[playerIdx].pointBurstText->time = 0.0f;
	}
	
	players[playerIdx].trickAttrText->changeText("");
}

//	OnTrickPointChange()
// Event response: refreshes the trick's point display.
void IGOFrontEnd::OnTrickPointChange(const int playerIdx)
{
	if (g_game_ptr->get_num_ai_players() && playerIdx)
		return;
	assert(playerIdx >= 0 && playerIdx < numPlayers);
	
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(playerIdx);
	stringx						pointText(ksctrl->get_my_scoreManager().GetChain().GetRawScore());
	float						multiplier = ksctrl->get_my_scoreManager().GetChain().GetMultiplier();
	
	// Display format: "9999  X9.9"
	if (multiplier > 1.0f)
	{
		if (float(int(multiplier)) == multiplier)
			pointText += stringx(" @@") + stringx(int(multiplier));
		else
		{
			char	s[128] = "";
			sprintf(s, "%.1f", multiplier);
			pointText += stringx(" @@") + s;
		}
	}

	if (players[playerIdx].trickText) players[playerIdx].trickText->checkTime = false;
	if (players[playerIdx].trickBurstText)
	{
		//players[playerIdx].trickBurstText->checkTime = true;
		//players[playerIdx].trickBurstText->time = 0.0f;
	}
	
	MakeTrickPointText(playerIdx, pointText);
	if (players[playerIdx].trickPointText) players[playerIdx].trickPointText->checkTime = false;
	if (players[playerIdx].pointBurstText)
	{
		//players[playerIdx].pointBurstText->checkTime = true;
		//players[playerIdx].pointBurstText->time = 0.0f;
	}
}

//	OnViewportChange()
// Call this function whenever the player's viewport changes dimensions.
void IGOFrontEnd::OnViewportChange(void) 
{
	float	share = 0.0f;
	int		x1, y1, x2, y2, tx1, ty1, tx2, ty2;
	int		center = 640/2, tCenter = 640/2;
	int		width, tWidth;
	int		bottom = 480-TRICK_BOX_MARGIN;
	int		boxWidth, boxHeight;
	int		i;	
	
	// Move splitter.
	if (splitterWidget)
	{
		if (g_game_ptr->get_play_mode_push())
			splitterWidget->SetPosition(640.0f*g_game_ptr->get_play_mode_push()->GetPlayerShare(0));
		else if (g_game_ptr->get_play_mode_head_to_head())
			splitterWidget->SetPosition(640.0f*0.5f);
		else
			splitterWidget->SetPosition(640.0f*1.0f);
	}
	
	
	// Player IGO adjustments.
	for (i = 0; i < numPlayers; i++)
	{
		// Get this player's share of the screen.
		if (g_game_ptr->get_play_mode_push())
			share = g_game_ptr->get_play_mode_push()->GetPlayerShare(i);
		else if (g_game_ptr->get_play_mode_head_to_head())
			share = 0.5f;
		else
			share = 1.0f;
		if (share == 0.0f) continue;
		
		// Calculate hardware-independent viewport for this player.
		if (g_game_ptr->get_game_mode() == GAME_MODE_PUSH || g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD)
		{
			if (i == 0)
			{
				tx1 = x1 = 0;
				ty1 = y1 = 0;
				tx2 = x2 = int(640.0f*share)-1;
				ty2 = y2 = 479;
			}
			else
			{
				tx1 = x1 = int(640.0f*(1.0f-share));
				ty1 = y1 = 0;
				tx2 = x2 = 639;
				y2 = 479;
			}
		}
		else
		{
			tx1 = x1 = 0;
			ty1 = y1 = 0;
			tx2 = x2 = 639;
			ty2 = y2 = 479;
			
			if (!iconManager || iconManager->Failed())
				tx1 = 0;
			else
				tx1 = 85;
			if (!waveIndicatorWidget)
				tx2 = 639;
			else
			{
				switch (waveIndicatorWidget->GetState())
				{
				case WaveIndicatorWidget::STATE_DIR : tx2 = 395; break;
				case WaveIndicatorWidget::STATE_SURGE : tx2 = 480; break;
				case WaveIndicatorWidget::STATE_TONGUE : tx2 = 480; break;
				default : tx2 = 639;
				}
			}
		}

		players[i].viewport.tl.x = x1;
		players[i].viewport.tl.y = y1;
		players[i].viewport.br.x = x2;
		players[i].viewport.br.y = y2;

		width = x2-x1+1;
		center = x1+(width/2);
		tWidth = tx2-tx1+1;
		tCenter = tx1+(tWidth/2);

		// Calculate size of trick text box.
		boxWidth = tWidth-TRICK_BOX_MARGIN*2;
		boxHeight = TRICK_BOX_HEIGHT;
		
		// Move onscreen trick text.
		if (players[i].trickText)
		{
			players[i].trickText->changePos(tCenter-boxWidth/2.0f, bottom-boxHeight);
			players[i].trickText->SetSize(boxWidth, boxHeight);
		}
		if (players[i].trickBurstText)
		{
			players[i].trickBurstText->checkTime = true;
			players[i].trickBurstText->time = 0.0f;
		}
		if (players[i].trickPointText) players[i].trickPointText->changePos(tCenter, bottom-5);
		if (players[i].pointBurstText)
		{
			players[i].pointBurstText->checkTime = true;
			players[i].pointBurstText->time = 0.0f;
		}
		if (players[i].trickAttrText) players[i].trickAttrText->changeX(tCenter);
		OnTrickChange(i);
		
		if (players[i].horizBalanceWidget) players[i].horizBalanceWidget->FitToViewport(x1, y1, x2, y2);
	}
}

void IGOFrontEnd::OnSurferStandUp(void)
{
	if (waveIndicatorWidget) waveIndicatorWidget->Hide();
}

void IGOFrontEnd::TurnBalanceMeterOn(const int playerIdx, bool vert, bool on)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	if (players[playerIdx].horizBalanceWidget) players[playerIdx].horizBalanceWidget->Show(on);
}

void IGOFrontEnd::SetBalanceMeter(const int playerIdx, bool vert, float f)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	if (players[playerIdx].horizBalanceWidget) players[playerIdx].horizBalanceWidget->SetArrow(f);
}

void IGOFrontEnd::SetTubeDepthMeter(const int playerIdx, const float f)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	if (players[playerIdx].horizBalanceWidget) players[playerIdx].horizBalanceWidget->SetFillage(f);
}

//	ShowBigWaveMeter()
// Turns the display of the big wave meter on/off.
void IGOFrontEnd::ShowBigWaveMeter(const bool on)
{
	if (bigWaveMeterWidget) bigWaveMeterWidget->Show(on);
}

//	SetBigWaveMeterPos()
// Sets the arrow's position on the meter [-1, 1].
void IGOFrontEnd::SetBigWaveMeterPos(const float f)
{
	if (bigWaveMeterWidget) bigWaveMeterWidget->SetArrowPos(f);
}

//	SetBigWaveMeterRange()
// Sets the size of the big wave meter (0, 1].
void IGOFrontEnd::SetBigWaveMeterSize(const float f)
{
	if (bigWaveMeterWidget) bigWaveMeterWidget->SetSize(f);
}

//	ShowAccompBackground()
// This private helper function toggles the the accomplishments background display.
void IGOFrontEnd::ShowAccompBackground(const bool bgOn, const int hGridFlags, const int vGridFlags)
{
	if (accompWidget) accompWidget->Show(bgOn);
	if (gridWidget)
	{
		if (bgOn)
		{
			gridWidget->ShowHLines(hGridFlags);
			gridWidget->ShowVLines(vGridFlags);
		}
		else
			gridWidget->Hide();
	}
}

void IGOFrontEnd::TurnOnTubeTimer(const int playerIdx, bool on)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	if (players[playerIdx].tubeTimer)
	{
		players[playerIdx].drawTubeTimer = on;
		players[playerIdx].tubeTimer->no_color = false;
		players[playerIdx].tubeTimer->color = COLOR_STANDARD;
	}
}

void IGOFrontEnd::SetTubeTimer(const int playerIdx, float f)
{
	stringx	t;
	int		s = (int) f;
	
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	t = t+stringx(s)+".";
	f -= s;
	int ms = (int)(f*10.0f);
	t = t+stringx(ms);
	if (players[playerIdx].tubeTimer) players[playerIdx].tubeTimer->changeText(t);
}

void IGOFrontEnd::TurnOnTubeIndicator(const int playerIdx, bool on)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	if (players[playerIdx].tubeTimer)
	{
		if (on) players[playerIdx].tubeTimer->color = COLOR_POINTS_MAIN;
		else players[playerIdx].tubeTimer->color = COLOR_STANDARD;
	}
}

//	ShowCameraReticle()
// Fades in the camera reticle IGO.
void IGOFrontEnd::ShowCameraReticle(const float time)
{
	if (cameraWidget) cameraWidget->Show(time);
}

//	HideCameraReticle()
// Makes the camera reticle IGO go away immediately.
void IGOFrontEnd::HideCameraReticle(void)
{
	if (cameraWidget) cameraWidget->Hide();
}

//	GetCameraReticleFade()
// Returns the reticle's alpha.
float IGOFrontEnd::GetCameraReticleFade(void) const
{
	if (cameraWidget)
		return cameraWidget->GetFade();
	else
		return 0.0f;
}

//	ShowPhoto()
// Fades in a picture photo with the specified texture.
void IGOFrontEnd::ShowPhoto(nglTexture * texture, int * scorePtr, const int photoNum)
{
	if (photoWidget) photoWidget->Show(texture, scorePtr, photoNum);
}

//	HidePhoto()
// Makes the photo picture IGO go away immediately.
void IGOFrontEnd::HidePhoto(void)
{
	if (photoWidget) photoWidget->Hide();
}

//	IsPhotoShown()
// Returns true if a photo is being displayed onscreen.
bool IGOFrontEnd::IsPhotoShown(void) const
{
	return photoWidget && photoWidget->IsShown();
}

//	ShowMenuBackground()
// Toggles the pause menu background on/off.
void IGOFrontEnd::ShowMenuBackground(const bool on)
{
	if (runState != RUNSTATE_NORMAL && on)
	{
		int i = 0;
	}
	
	if (menuBGWidget) menuBGWidget->Show(on);
}

bool IGOFrontEnd::IsMenuBGShown(void) const
{
	return menuBGWidget && menuBGWidget->IsShown();
}

//	MakeTrickAttrText()
// Private helper function that recreates the trick attribute text.
void IGOFrontEnd::MakeTrickAttrText(const int playerIdx)
{
	kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(playerIdx);
	
	if (playerIdx && g_game_ptr->get_num_ai_players()) return;
	
	if (ksctrl->get_my_scoreManager().GetChain().series.empty())
	{
		players[playerIdx].trickAttrText->changeText("");
		return;
	}
	
	const ScoringManager::Series &	series = ksctrl->get_my_scoreManager().GetChain().series.back();
	
	// Set text.
	if (series.landing == ScoringManager::LAND_PERFECT)
	{
		players[playerIdx].trickAttrText->changeText(ksGlobalTextArray[GT_TRICK_PERFECT]);
		players[playerIdx].trickAttrText->color = COLOR_PERFECT;
		players[playerIdx].trickAttrText->checkTime = true;
		players[playerIdx].trickAttrText->time = TRICK_WAIT_MAX;
	}
	else if (series.landing == ScoringManager::LAND_SLOPPY)
	{
		players[playerIdx].trickAttrText->changeText(ksGlobalTextArray[GT_TRICK_SLOPPY]);
		players[playerIdx].trickAttrText->color = COLOR_BAD;
		players[playerIdx].trickAttrText->checkTime = true;
		players[playerIdx].trickAttrText->time = TRICK_WAIT_MAX;
	}
	/*
	else if (series.landing == ScoringManager::LAND_JUNK)
	{
		players[playerIdx].trickAttrText->changeText(ksGlobalTextArray[GT_TRICK_JUNK]);
		players[playerIdx].trickAttrText->color = COLOR_JUNK;
		players[playerIdx].trickAttrText->checkTime = true;
		players[playerIdx].trickAttrText->time = TRICK_WAIT_MAX;
	}
	*/
	else
		players[playerIdx].trickAttrText->changeText("");
	
	// Set position of text.
	TrickBoxText * t = players[playerIdx].trickText;
	players[playerIdx].trickAttrText->changeY(t->getY()+t->getHeight()-t->GetTextHeight()-17);
}

//	MakeTrickPointText()
// Private helper function that recreates the trick point text with the specified string.
void IGOFrontEnd::MakeTrickPointText(const int playerIdx, const stringx & text)
{
	assert(playerIdx >= 0 && playerIdx < numPlayers);
	
	if (g_game_ptr->get_num_ai_players() && playerIdx)
		return;
	
	//kellyslater_controller *	ksctrl = g_world_ptr->get_ks_controller(playerIdx);
	//float						dist = ksctrl->get_my_scoreManager().GetMouthDist();
	color32						c;
	
	// Determine color of trick point text.
	/*
	if (dist < 0.333333f)
		c = COLOR_POINTS_MOUTH;
	else if (dist >= 0.333333f && dist <= 0.666666f)
		c = COLOR_POINTS_MAIN;
	else if (dist > 0.666666f)
		c = COLOR_POINTS_EDGE;
	else
		c = color32(0, 0, 0, 0);
	*/
	c = COLOR_POINTS_MAIN;
	
	// Recreate trick point text.
	players[playerIdx].trickPointText->unmakeRand();
	players[playerIdx].trickPointText->changeText(text);
	players[playerIdx].trickPointText->color = c;
	players[playerIdx].trickPointText->time = TRICK_WAIT_MAX;
}

void IGOFrontEnd::MakeOtherText(const stringx & t)
{
	int	x1, x2;
	
	otherText->time = TEXT_WAIT_MAX;
	otherText->changeText(t);

	if (iconManager && !iconManager->Failed())
		x1 = 50+70;
	else
		x1 = 50;
	if ((photoWidget && photoWidget->IsShown()) ||
		(g_beach_ptr->get_challenges()->photo && g_beach_ptr->get_challenges()->photo->IsTakingPhoto()))
		x2 = 640-50-140;
	else
		x2 = 640-50;
	otherText->changePos(x1+(x2-x1)/2, 300);
	otherText->makeBox(x2-x1, 100);
}


//	SortCompetition()
// Private helper function: sorts/ranks the first n surfers.
void IGOFrontEnd::SortCompetition(const int num)
{
	JudgingSystem::RESULT *	results = g_beach_ptr->judges.GetCompetitionResults();
	color32	nameColor = COLOR_ALT;
	color32 runColor = COLOR_STANDARD;
	color32	totalColor = COLOR_ALT;
	int		rank;
	int		i, j;
	
	// Reset rank for all surfers.
	for (i = 0; i < SURFER_LAST; i++)
		competition.results[i].rank = 0;
	
	// Rank up to num surfers.
	for (rank = 1; rank <= num; rank++)
	{
		// Find the unranked surfer with the highest score.
		int largestIdx = -1;
		for (i = 0; i < SURFER_LAST; i++)
		{
			// Ignore invalid surfers.
			if (!g_beach_ptr->judges.IsCompetitor(i)) continue;

			if (competition.results[i].rank == 0)
			{
				if (largestIdx == -1 || results[i].GetRunTotal() > results[largestIdx].GetRunTotal())
					largestIdx = i;
			}
		}

		// Rank this surfer.
		assert(largestIdx >= 0 && largestIdx < SURFER_LAST);
		competition.results[largestIdx].rank = rank;
	}

	// Update result text to correspond to new order.
	for (i = 0; i < SURFER_LAST; i++)
	{
		// Ignore invalid surfers.
		if (!g_beach_ptr->judges.IsCompetitor(i)) continue;
		
		// Ignore unranked surfers.
		if (competition.results[i].rank == 0) continue;
		
		j = competition.results[i].rank-1;
		
		// Set name's color.
		if (i == g_game_ptr->GetSurferIdx(0))
		{
			nameColor = COLOR_HI;
			runColor = COLOR_HI;
			totalColor = COLOR_HI;
		}
		else
		{
			nameColor = COLOR_ALT;
			runColor = COLOR_STANDARD;
			totalColor = COLOR_ALT;
		}
		
		competition.results[i].name->changeText(GetCompStrSurfer(i));
		competition.results[i].name->changePos(POS_COMP_LEFT, POS_COMP_TOP+LINE_SPACING*j);
		competition.results[i].name->color = nameColor;
		competition.results[i].runs[0]->changeText(GetCompStrRun(i, 0));
		competition.results[i].runs[0]->changePos(POS_COMP_HEAT-50, POS_COMP_TOP+LINE_SPACING*j);
		competition.results[i].runs[0]->color = runColor;
		competition.results[i].runs[1]->changeText(GetCompStrRun(i, 1));
		competition.results[i].runs[1]->changePos(POS_COMP_HEAT, POS_COMP_TOP+LINE_SPACING*j);
		competition.results[i].runs[1]->color = runColor;
		competition.results[i].runs[2]->changeText(GetCompStrRun(i, 2));
		competition.results[i].runs[2]->changePos(POS_COMP_HEAT+50, POS_COMP_TOP+LINE_SPACING*j);
		competition.results[i].runs[2]->color = runColor;
		competition.results[i].total->changeText(GetCompStrTotal(i));
		competition.results[i].total->changePos(POS_COMP_TOTAL, POS_COMP_TOP+LINE_SPACING*j);
		competition.results[i].total->color = totalColor;
	}
	
	competition.numSorted = num;
}

//	GetCompStrSurfer()
// Returns the specified surfer's name, formatted as a competition string.
stringx	IGOFrontEnd::GetCompStrSurfer(const int surferIdx) const
{
	stringx	name;

	assert(surferIdx >= 0 && surferIdx < SURFER_LAST);
	name = SurferDataArray[surferIdx].fullname;
	name.to_upper();
	
	return name;
}

//	GetCompStrRun()
// Returns the specified surfer's run, formatted as a competition string.
stringx IGOFrontEnd::GetCompStrRun(const int surferIdx, const int runIdx) const
{
	JudgingSystem::RESULT *	results = g_beach_ptr->judges.GetCompetitionResults();
	char	s[256] = "";

	assert(surferIdx >= 0 && surferIdx < SURFER_LAST);
	if (runIdx < g_beach_ptr->judges.GetCurrRunIdx() || g_beach_ptr->judges.IsCompetitionOver())
		sprintf(s, "%.1f", results[surferIdx].runs[runIdx].GetAverageScore());
	else
		sprintf(s, " . ");
	
	return stringx(s);
}

//	GetCompStrTotal()
// Returns the specified surfer's total, formatted as a competition string.
stringx IGOFrontEnd::GetCompStrTotal(const int surferIdx) const
{
	JudgingSystem::RESULT *	results = g_beach_ptr->judges.GetCompetitionResults();
	char	s[256] = "";

	assert(surferIdx >= 0 && surferIdx < SURFER_LAST);
	sprintf(s, "%.1f", results[surferIdx].GetRunTotal());
	
	return stringx(s);
}

// GetTimeText()
// Private helper function - converts minutes and seconds into a pretty stringx.
stringx IGOFrontEnd::GetTimeText(const int min, const float sec) const
{
	stringx	clockText((int)sec);
	
	if (sec < 10)
		clockText = stringx("0") + clockText;
	clockText = stringx(min) + stringx(":") + clockText;
	
	return clockText;
}

//	GetTimeText()
// Private helper function - converts a time float into a pretty stringx.
stringx IGOFrontEnd::GetTimeText(const float t) const
{
	int min = int(t/60.0f);
	
	return GetTimeText(min, int(t)-min*60);
}

bool IGOFrontEnd::GetProceedButtonState(void)
{
#if defined(TARGET_XBOX)
	return getButtonState(FEMENUCMD_CROSS) || getButtonState(FEMENUCMD_START);
#else
	return getButtonState(FEMENUCMD_CROSS);
#endif
}

bool IGOFrontEnd::GetProceedButtonState(const int controller)
{
#if defined(TARGET_XBOX)
	return getButtonState(FEMENUCMD_CROSS, controller) || getButtonState(FEMENUCMD_START, controller);
#else
	return getButtonState(FEMENUCMD_CROSS, controller);
#endif
}
