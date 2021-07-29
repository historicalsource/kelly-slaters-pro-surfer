
#include "global.h"
#include "igo_widget_timeattack.h"

const color32 TimeAttackWidget::COLOR_TIME = color32(0, 255, 255, 255);
const color32 TimeAttackWidget::COLOR_TIME_ATTACK = color32(255, 255, 102, 255);
const color32 TimeAttackWidget::COLOR_SCORE = color32(0, 255, 255, 255);

//	TimeAttackWidget()
// Default constructor.
TimeAttackWidget::TimeAttackWidget()
{
	bgPQ = NULL;

	clockSec = 0.0f;
	clockMin = 0;
	timeText = NEW TextString(NULL, GetTimeText(clockMin, clockSec), 543, 65, 0, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, COLOR_TIME);

	attackClockSec = 0.0f;
	attackClockMin = 0;
	attackText = NEW TextString(NULL, GetTimeText(attackClockMin, attackClockSec), 543, 80, 0, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, COLOR_TIME_ATTACK);

	score = 0;
	scoreText = NEW TextString(NULL, GetScoreText(score), 492, 59, 0, 0.8f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_SCORE);
}

//	~TimeAttackWidget()
// Destructor.
TimeAttackWidget::~TimeAttackWidget()
{
	delete timeText;
	delete attackText;
	delete scoreText;
}

//	SetDisplay()
// Toggles this widget on/off.
void TimeAttackWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void TimeAttackWidget::Init(PanelFile & panel, Font * numberFont, Font * clockFont)
{
	timeText->setFont(clockFont);
	attackText->setFont(clockFont);
	scoreText->setFont(numberFont);

	bgPQ = panel.GetPointer("time_attack");
	bgPQ->TurnOn(true);

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void TimeAttackWidget::Update(const float dt)
{
	//float	newClockSec;
	//int		newClockMin;
	
	IGOWidget::Update(dt);

	//newClockSec = TIMER_GetLevelSec();
	//newClockMin = (int) (newClockSec/60);
	//newClockSec -= newClockMin * 60;
}

//	Draw()
// Sends this widget's quads to NGL.
void TimeAttackWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	bgPQ->Draw(0);
	timeText->Draw();
	attackText->Draw();
	scoreText->Draw();
}

//	SetTime()
// Sets the seconds and minutes of the main clock.
void TimeAttackWidget::SetTime(const int newClockMin, const float newClockSec)
{
	if (newClockSec != clockSec || newClockMin != clockMin)
	{
		clockSec = newClockSec;
		clockMin = newClockMin;
		timeText->changeText(GetTimeText(clockMin, clockSec));
	}
}

//	SetAttackTime()
// Sets the seconds and minutes of the attack clock.
void TimeAttackWidget::SetAttackTime(const int newClockMin, const float newClockSec)
{
	if (newClockSec != attackClockSec || newClockMin != attackClockMin)
	{
		attackClockSec = newClockSec;
		attackClockMin = newClockMin;
		attackText->changeText(GetTimeText(attackClockMin, attackClockSec));
	}
}

//	SetScore()
// Sets the displayed score.
void TimeAttackWidget::SetScore(const int sc)
{
	if (score != sc)
	{
		score = sc;
		scoreText->changeText(GetScoreText(score));
	}
}

// GetTimeText()
// Private helper function - converts minutes and seconds into a pretty stringx.
stringx TimeAttackWidget::GetTimeText(const int min, const float sec) const
{
	stringx	clockText((int)sec);
		
	if (sec < 10)
		clockText = stringx("0") + clockText;
	clockText = stringx(min) + stringx(":") + clockText;

	return clockText;
}

//	GetScoreText()
// Private helper function that returns a formatted score string.
stringx TimeAttackWidget::GetScoreText(const int sc) const
{
	stringx	txt = stringx(sc);

	//if (sc < 100000) txt = "0"+txt;
	//if (sc < 10000)  txt = "0"+txt;
	//if (sc < 1000)   txt = "0"+txt;
	//if (sc < 100)    txt = "0"+txt;
	//if (sc < 10)     txt = "0"+txt;

	return txt;
}
