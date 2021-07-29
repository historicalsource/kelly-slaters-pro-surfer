// IGOFrontEnd.h

#ifndef IGOFRONTEND_H
#define IGOFRONTEND_H

#include "FEMenu.h"
#include "beach.h"
#include "rumblemanager.h"
#include "career.h"
#include "igo_widget_simple.h"
#include "igo_widget_analogclock.h"
#include "igo_widget_specialmeter.h"
//#include "igo_widget_breakindicator.h"c
#include "igo_widget_waveindicator.h"
#include "igo_widget_balance.h"
#include "igo_widget_splitter.h"
#include "igo_widget_fanmeter.h"
#include "igo_widget_camera.h"
#include "igo_widget_photo.h"
#include "igo_widget_replay.h"
#include "igo_widget_splitscore.h"
#include "igo_widget_splitmeter.h"
#include "igo_widget_splitclock.h"
#include "igo_widget_timeattack.h"
#include "igo_widget_iconradar.h"
#include "igo_widget_iconcount.h"
#include "igo_widget_skillchallenge.h"
#include "igo_widget_meterchallenge.h"
#include "igo_widget_objectalert.h"
#include "igo_widget_grid.h"
#include "igoiconmanager.h"
#include "igolearn_new_trickmanager.h"
#include "tutorialmanager.h"
#include "igohintmanager.h"
#include "trickdata.h"

#define MAX_QUEUE_SIZE 16 // max number of elements in the queue

typedef struct 
{
	stringx str;
	EventType e;
} soundMessageObject;
class IGOPrintQueue
{
private:
	soundMessageObject strings[MAX_QUEUE_SIZE];
	int start, end;  // if start == ned then the queue is empty.  Be careful to watch for overflow.
	uint8 size;
public:
	IGOPrintQueue();
	soundMessageObject *pop();
	bool push(stringx message, EventType e=SS_LAST); // returns false if the queue is full.
	void clear();
};

class IGOFrontEnd : public FrontEnd, public EventRecipient
{
public:
	
	static color32 COLOR_POINTS_EDGE;
	static color32 COLOR_POINTS_MAIN;
	static color32 COLOR_POINTS_MOUTH;
	static color32 COLOR_PERFECT;
	static color32 COLOR_BLOOD;
	
	static const int POS_COMP_TOP;
	static const int POS_COMP_LEFT;
	static const int POS_COMP_HEAT;
	static const int POS_COMP_TOTAL;
	static const int POS_REWARDS_LEFT;
	static const int POS_REWARDS_RIGHT;
	static const int POS_REWARDS_TOP;
	static const int POS_GOALS_LEFT;
	static const int POS_GOALS_TOP;
	static const int POS_GOALS_RIGHT;
	static const int POS_STATS_LEFT;
	static const int POS_STATS_RIGHT;
	static const int POS_STATS_TOP;
	static const int POS_PUSH_TOP;
	static const int POS_HEAD_TO_HEAD_LEFT;
	static const int POS_HEAD_TO_HEAD_RIGHT;
	static const int POS_HEAD_TO_HEAD_BOTTOM;
	static const int POS_TA_TOP;
	static const int POS_TA_LEFT;
	static const int POS_TA_RIGHT;
	static const int POS_TA_BOTTOM;
	static const int POS_METER_ATTACK_TOP;
	static const int POS_METER_ATTACK_LEFT;
	static const int POS_METER_ATTACK_RIGHT;

	static const int LINE_SPACING;

	static const int TRICK_BOX_HEIGHT;
	static const int TRICK_BOX_MARGIN;
	
	enum RUNSTATE
	{
		RUNSTATE_NORMAL,		// normal play: no special display
		RUNSTATE_GOALS,			// end of run: show completed goals
		RUNSTATE_REWARDS,		// end of run: show earned rewards
		RUNSTATE_STATS,			// end of run: show best combo, longest tube ride, etc
		RUNSTATE_HIGHSCORE,		// end of run: show high score
		RUNSTATE_PHOTOS,		// end of run: show photoshoot results
		RUNSTATE_SAVE,			// end or run: save career
		RUNSTATE_JUDGE,			// end of comp run: show judges' score
		RUNSTATE_RANK,			// end of comp run: show ranking
		RUNSTATE_MEDAL,			// end of comp run: show trophy
		RUNSTATE_PUSH,			// end of multiplayer push run: show who won
		RUNSTATE_TIME_ATTACK,	// end of multiplayer time attack run: show curent results
		RUNSTATE_METER_ATTACK,	// end of multiplayer meter attack run: show curent results
		RUNSTATE_HEAD_TO_HEAD,	// end of multiplayer head to head run: show who won
		RUNSTATE_LAST,			// end of all runs: popup pause menu
	};

	enum { MAX_REWARDS = 20 };
	
private:
	static const int NUM_PQS_MENU;
	static const int NUM_PQS_ACCOMP;
	static const int NUM_PQS_ACCOMP_GRID;
	static const int NUM_PQS_CONTINUE;
	static const int NUM_PQS_MEDAL;
	
	enum IGO_REWARD
	{
		IGO_REWARD_NONE,
		IGO_REWARD_BOARD,
		IGO_REWARD_SURFER,
		IGO_REWARD_BEACH
	};
	
private:
	struct GOALS
	{
		TextString *	title;
		BoxText *		names[5];		// goal title
		TextString *	status[5];		// done or not done
		TextString *	cont;			// continue
		bool			draw;
	};

	struct STATS
	{
		TextString *	title;
		TextString *	totalText,  * totalNumerals;
		TextString *	tubeText, * tubeNumerals;
		TextString *	tricksText, * tricksNumerals;
		TextString *	iconsText, * iconsNumerals;
		TextString *	comboText, * comboNumerals;
		TrickBoxText *	comboDetails;
		TextString *	cont;
		bool			draw;
	};

	struct HIGHSCORE
	{
		int				waitingOnMenu;	// index of menu that we're waiting for, -1 for not waiting
	};

	struct PHOTOS
	{
		int				waitingOnMenu;	// index of menu that we're waiting for, -1 for not waiting
	};

	struct SAVE
	{
		int				waitingOnMenu;	// index of menu that we're waiting for, -1 for not waiting
	};

	// Competition result display strings for a single surfer.
	struct COMP_RESULT
	{
		TextString *	name;		// name of surfer
		TextString *	runs[3];	// surfer's run results
		TextString *	total;		// total of 3 runs
		int				rank;		// 1st, 2nd, etc.
	};
	
	struct COMPETITION
	{
		TextString *	faceScoreText;
		TextString *	faceScoreNumerals;
		TextString *	airScoreText;
		TextString *	airScoreNumerals;
		TextString *	tubeScoreText;
		TextString *	tubeScoreNumerals;
		TextString *	avgScoreText;
		TextString *	avgScoreNumerals;
		
		COMP_RESULT		results[SURFER_LAST];
		TextString *	title;						// "Competition results"
		TextString *	name;						// "surfer"
		TextString *	heats;						// "heats"
		TextString *	total;						// "total"
		float			sortTimer;					// allows rank sorting once per second
		int				numSorted;					// currently "sorting" comp results

		SimpleWidget *	medalWidgets[3][3];			// pictures of 1st, 2nd, and 3rd place medals
		TextString *	congrat;					// "Congratulations!"
		BoxText *		wonText;					// "You got first place at G-land."
		
		bool			draw;
	};
	
	struct PUSH
	{
		TextString *	titleText;
		TextString *	playerText;
		TextString *	timeText;
		bool			draw;
		float			timer;
	};

	struct HEAD_TO_HEAD
	{
		TextString *	titleText;
		BouncingText *	winText;
		TextString *	player1Text;
		TextString *	player1ScoreText, * player1ScoreNumerals;
		TextString *	player1CountText, * player1CountNumerals;
		TextString *	player1ComboText, *	player1ComboNumerals;
		TextString *	player2Text;
		TextString *	player2ScoreText, * player2ScoreNumerals;
		TextString *	player2CountText, * player2CountNumerals;
		TextString *	player2ComboText, * player2ComboNumerals;
		bool			draw;
	};
	
	struct TIME_ATTACK
	{
		TextString *	title;
		BouncingText *	ready;
		TextString *	player1Text;
		TextString *	player1Score;
		TextString *	player1ScoreNumerals;
		TextString *	player1Time;
		TextString *	player1TimeNumerals;
		TextString *	player2Text;
		TextString *	player2Score;
		TextString *	player2ScoreNumerals;
		TextString *	player2Time;
		TextString *	player2TimeNumerals;
		bool draw;
		int device_flags;
	};

	struct METER_ATTACK
	{
		TextString *	title;
		BouncingText *	ready;
		TextString *	player1Meter;
		TextString *	player1MeterNumerals;
		TextString *	player1Time;
		TextString *	player1TimeNumerals;
		TextString *	player2Meter;
		TextString *	player2MeterNumerals;
		TextString *	player2Time;
		TextString *	player2TimeNumerals;
		bool draw;
	};
	
	struct REWARDS
	{
		TextString *    title;
		BoxText *		names[MAX_REWARDS];
		TextString *    cont;
		bool            draw;
		bool            initialized;
	};
	
	struct PLAYER
	{
		recti					viewport;
		SpecialMeterWidget *	meterWidget;
		HorizBalanceWidget *	horizBalanceWidget;
		SplitScoreWidget *		splitScoreWidget;
		SplitMeterWidget *		splitMeterWidget;
		TrickBoxText *			trickText;
		TextString *			trickAttrText;
		RandomText *			trickPointText;
		BurstTrickText *		trickBurstText;
		BurstText *				pointBurstText;
		TextString *			tubeTimer;
		bool					drawTubeTimer;
	};	
	
private:
	static int TRICK_WAIT_MAX;
	static int TEXT_WAIT_MAX;
	float MAX_TRICK_SCALE;// = 1.3f;
	float MAX_TEXT_SCALE;
	
	PanelFile *	mpPanel;
	PanelFile *	ghostIconPanel;
	PanelFile * compPanel;

	int		clock_min;
	float	clock_sec;
	bool	debug_menu_displayed;
	bool	firstTimeAttackState;
	bool	firstMeterAttackState;
	bool	display;			// true: draw IGO, false: don't draw IGO
	bool	proceedPressed;
	
	WaveIndicatorWidget::STATE	prevWaveIndicatorState;
	bool						prevIconModeState;
	
	int				numPlayers;
	int				numActivePlayers;
	PLAYER *		players;
	
	RUNSTATE		runState;			// currently displaying special text/menu?
	GOALS 			goals;
	REWARDS 		rewards;
	STATS 			stats;
	HIGHSCORE		highScore;
	PHOTOS			photos;
	SAVE			save;
	COMPETITION  	competition;		// text of result heats
	PUSH			push;
	HEAD_TO_HEAD	headToHead;
	TIME_ATTACK		timeAttack;
	METER_ATTACK	meterAttack;
	
	IGOIconManager *			iconManager;
	IGOLearnNewTrickManager *	learn_new_trickManager;
	IGOTutorialManager *		tutorialManager;
	IGOHintManager *			hintManager;
	
	IGOPrintQueue   printQueue; // holds the text to be displayed on-screen so none is lost
	BoxText *		otherText;
	TextString *	replayText;
	TextString *	helpText;
	
	Font *	trickFont;
	Font *	stdFont;
	Font *	numberFont;
	Font *	boldFont;

	color32	COLOR_STANDARD;
	color32 COLOR_ALT;
	color32 COLOR_HI;
	color32 COLOR_LO;
	color32 COLOR_BAD;

	SimpleWidget *			menuBGWidget;
	SimpleWidget *			accompWidget;
	SimpleWidget *			accompGridWidget;
	SimpleWidget *			continueWidget;
	AnalogClockWidget *		analogClockWidget;
	SplitClockWidget *		centerClockWidget;
	SplitterWidget *		splitterWidget;
	//BreakIndicatorWidget *	breakIndicatorWidget;
	WaveIndicatorWidget *	waveIndicatorWidget;
	FanMeterWidget *		bigWaveMeterWidget;
	CameraWidget *			cameraWidget;
	PhotoWidget *			photoWidget;
	ReplayWidget *			replayWidget;
	TimeAttackWidget *		timeAttackWidget;
	TimeAttackWidget *		meterAttackWidget;
	IconRadarWidget *		iconRadarWidget;
	IconCountWidget *		iconCountWidget;
	SkillChallengeWidget *	skillChallengeWidget;
	MeterChallengeWidget *	meterChallengeWidget;
	ObjectAlertWidget	 *	objectAlertWidget;
	GridWidget *			gridWidget;

#ifdef DEBUG
	TextString * debugText;
#endif
	
public:
	IGOFrontEnd(FEManager* man, stringx p, stringx pf_name);
	~IGOFrontEnd();
	void Init(void);
	void ResetIconManager() { if(iconManager)  iconManager->Reset();}
	void ResetLearnNewTrickManager() { if(learn_new_trickManager)  learn_new_trickManager->Reset();}
	void ResetTutorialManager() { if(tutorialManager)  tutorialManager->Reset();}
	void ResetHintManager() { if(hintManager)  hintManager->Reset();}
	void OnModeReset(void);
	void Update(time_value_t time_inc);
	void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);
	void OnReplayStart() { ShowMenuBackground(false); replay_mode = true; }
	void OnReplayEnd() { ShowMenuBackground(true); replay_mode = false; }
	void Draw();
	void Print(stringx text, EventType t=SS_LAST);
	void SetReplayText(const stringx & t);
	void DebugMenu(bool on) { debug_menu_displayed = on; }
	void SetDisplay(const bool disp);
	bool GetDisplay(void) const { return display; }
	
	void OnSurferReset(const int playerIdx, const bool waveDirection);
	void OnSurferStandUp(void);
	void OnTrickChange(const int playerIdx, const bool bounce = true);
	void OnTrickPointChange(const int playerIdx);
	void OnTrickComplete(const int playerIdx);
	void OnTrickFail(const int playerIdx);
	void OnScoreChange(const int playerIdx);
	void OnViewportChange(void);

	WaveIndicatorWidget * GetWaveIndicatorWidget(void) { return waveIndicatorWidget; }
	
	void TurnBalanceMeterOn(const int playerIdx, bool vert, bool on);
	void SetBalanceMeter(const int playerIdx, bool vert, float f);  // -1 to 1
	void SetTubeDepthMeter(const int playerIdx, const float f);
	
	void ShowBigWaveMeter(const bool on);
	void SetBigWaveMeterPos(const float f);
	void SetBigWaveMeterSize(const float f);
	
	void TurnOnTubeTimer(const int playerIdx, bool on);
	void SetTubeTimer(const int playerIdx, float f);		// # of seconds
	void TurnOnTubeIndicator(const int playerIdx, bool on);
	
	void ShowCameraReticle(const float time);
	void HideCameraReticle(void);
	float GetCameraReticleFade(void) const;
	
	void ShowPhoto(nglTexture * texture, int * scorePtr, const int photoNum);
	void HidePhoto(void);
	bool IsPhotoShown(void) const;
	
	void EndRun(void);
	void EndCompetition(void);
	void ResetRunState(void) { runState = RUNSTATE_NORMAL; }
	RUNSTATE GetRunState(void) const { return runState; }
	
	IGOIconManager *GetIconManager() {return iconManager;}
	IGOLearnNewTrickManager *GetLearnNewTrickManager() {return learn_new_trickManager;}
	IGOTutorialManager *GetTutorialManager() {return tutorialManager;}
	IGOHintManager *GetHintManager() {return hintManager;}
	int VCRGetButton()  {return (replayWidget) ? replayWidget->GetButton() : -1;}
	int VCRGetHighlight()  {return (replayWidget) ? replayWidget->GetHighlight() : -1;}
	void VCRButtonSelect(int button)  {if(replayWidget) replayWidget->Select(button);}
	void VCRHighlightSelect(int highlight)  {if(replayWidget) replayWidget->SelectHighlight(highlight);}
	void VCRHighlightLeft()  {if(replayWidget) replayWidget->HighlightLeft();}
	void VCRHighlightRight()  {if(replayWidget) replayWidget->HighlightRight();}
	
//	void ApplyCareer(void) { oldCareer = *g_career; }
	void IconHack(const int trickIdx);
	void ShowMenuBackground(const bool on = true);
	void ShowAccompBackground(const bool bgOn, const int hLineFlags = 0, const int vLineFlags = 0);
	bool IsMenuBGShown(void) const;

#ifdef DEBUG
	void SetDebugText(const stringx s) { if (debugText) debugText->changeText(s); }
#endif
	
private:
	bool replay_mode;
	void TrickStarted();
	void TrickFinished();
	void TrickFailed();
	void MakeTrickPointText(const int playerIdx, const stringx & text);
	void MakeTrickAttrText(const int playerIdx);
	void MakeOtherText(const stringx & t);
	
	stringx FindName(int name) { return (ksGlobalTextArray[name]); }
	
	void GetRewardStrings(stringx & str1, stringx & str2, IGO_REWARD & type);
	void SortCompetition(const int num);
	stringx	GetCompStrSurfer(const int surferIdx) const;
	stringx	GetCompStrRun(const int surferIdx, const int runIdx) const;
	stringx	GetCompStrTotal(const int surferIdx) const;
	stringx GetTimeText(const int min, const float sec) const;
	stringx GetTimeText(const float t) const;
	
	void UpdateStateNormal(time_value_t time_inc);
	void UpdateStateGoals(time_value_t time_inc);
	void UpdateStateRewards(time_value_t time_inc);
	void UpdateStateStats(time_value_t time_inc);
	void UpdateStateHighScore(time_value_t time_inc);
	void UpdateStatePhotos(time_value_t time_inc);
	void UpdateStateSave(time_value_t time_inc);
	void UpdateStateComp(time_value_t time_inc);
	void UpdateStatePush(time_value_t time_inc);
	void UpdateStateHeadToHead(time_value_t time_inc);
	void UpdateStateTimeAttack(time_value_t time_inc);
	void UpdateStateMeterAttack(time_value_t time_inc);
	void UpdateStateLast(time_value_t time_inc);

	bool GetProceedButtonState(void);
	bool GetProceedButtonState(const int controller);
};

#endif
