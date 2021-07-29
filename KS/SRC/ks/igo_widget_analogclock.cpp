
#include "global.h"
#include "igo_widget_analogclock.h"
#include "timer.h"

const float AnalogClockWidget::SEG_TURN = 2*PI/(float)MAX_TIME_SEGMENTS;
const float AnalogClockWidget::CLOCK_CENTER_X = 556.0f;					// 640x480 coords
const float AnalogClockWidget::CLOCK_CENTER_Y = 73.0f;					// 640x480 coords
const float AnalogClockWidget::CLOCK_RADIUS = 23.0;					// 640x480 coords
const color AnalogClockWidget::COLOR_CLOCK = color(0, 0, 0, 1);
const color AnalogClockWidget::COLOR_ELAPSED = color(1, 0, 0, 1);
const color AnalogClockWidget::COLOR_FLASH = color(0, 1, 1, 1);
//const color32 AnalogClockWidget::COLOR_SCORE = color32(0, 255, 235, 255);
const color32 AnalogClockWidget::COLOR_SCORE = color32(0, 255, 255, 255);

//	AnalogClockWidget()
// Default constructor.
AnalogClockWidget::AnalogClockWidget()
{
	int	i;
	
	clockSec = TIMER_IsInfiniteDuration() ? 0 : TIMER_GetRemainingLevelSec();
	if (clockSec < 0.0f) clockSec = 0.0f;
	clockMin = (int) (clockSec / 60);
	clockSec -= clockMin * 60;

	clockHandPQ = NULL;
	clockFramePQ = NULL;
	clockFacePQ = NULL;
	num1PQ = NULL;
	num2PQ = NULL;
	num3PQ = NULL;
	num4PQ = NULL;
	num5PQ = NULL;
	infPQ = NULL;
	
	// Allocate analog clock time segments.
	for (i = 0; i < MAX_TIME_SEGMENTS; i++)
	{
		timeSegs[i] = NEW PanelQuad4("");
	}
	
	score = 0;
	scoreText = NEW TextString(NULL, GetScoreText(score), 522, 63, 0, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, COLOR_SCORE);

	elapsedInterval = 0.0f;
}

//	~AnalogClockWidget()
// Destructor.
AnalogClockWidget::~AnalogClockWidget()
{
	int	i;
	
	// Delete analog clock time segments.
	for (i = 0; i < MAX_TIME_SEGMENTS; i++)
		delete timeSegs[i];

	delete scoreText;
}

//	SetDisplay()
// Overridden from base class.
void AnalogClockWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Called well after the constructor.
void AnalogClockWidget::Init(PanelFile & panel, Font * numberFont)
{
	float d = CLOCK_RADIUS * tanf(SEG_TURN) * 1.2f; //1.02f;
	float ccx = CLOCK_CENTER_X;
	float ccy = CLOCK_CENTER_Y;

	clockHandPQ = panel.GetPointer("hand01");;
	clockHandPQ->TurnOn(true);
	clockHandPQ->GetCenterPos(handCenterX, handCenterY);
	clockFramePQ = panel.GetPointer("timeframe_1");
	clockFramePQ->TurnOn(true);
	clockFacePQ = panel.GetPointer("face01");
	clockFacePQ->TurnOn(true);
	num1PQ = panel.GetPointer("minute_1");
	num1PQ->TurnOn(false);
	num2PQ = panel.GetPointer("minute_2");
	num2PQ->TurnOn(false);
	num3PQ = panel.GetPointer("minute_3");
	num3PQ->TurnOn(false);
	num4PQ = panel.GetPointer("minute_4");
	num4PQ->TurnOn(false);
	num5PQ = panel.GetPointer("minute_5");
	num5PQ->TurnOn(false);
	infPQ = panel.GetPointer("minute_infinty");
	infPQ->TurnOn(true);

	// z-ordering: segments go way in the back.
	for(int i=0; i<MAX_TIME_SEGMENTS; i++)
	{
		timeSegs[i]->Init(ccx-d, ccx, ccx, ccx, ccy-CLOCK_RADIUS, ccy-CLOCK_RADIUS, ccy, ccy, 0.0f, 0.0f, .5f, 1.0f, 300);
	}
	for(int i=0; i<MAX_TIME_SEGMENTS; i++)
	{
		timeSegs[i]->RotateOnce(ccx, ccy, -i*SEG_TURN);
	}

	scoreText->setFont(numberFont);
	elapsedInterval = 0.0f;

	SetDisplay(true);
}

//	Update()
// Call every frame - update widget with time elapsed since last frame.
void AnalogClockWidget::Update(const float dt)
{
	float	d = 60.0f/(float) MAX_TIME_SEGMENTS;
	int		i;

	IGOWidget::Update(dt);

	// Notice time change.
	clockSec = TIMER_IsInfiniteDuration() ? 0 : TIMER_GetRemainingLevelSec();
	if (clockSec < 0.0f) clockSec = 0.0f;
	clockMin = (int) (clockSec/60);
	clockSec -= clockMin * 60;

	// Show minute digit.
	num1PQ->TurnOn(display && clockMin == 1);
	num2PQ->TurnOn(display && clockMin == 2);
	num3PQ->TurnOn(display && clockMin == 3);
	num4PQ->TurnOn(display && clockMin == 4);
	num5PQ->TurnOn(display && clockMin == 5);

	// Move clock hand back to its default position then rotate it.
	clockHandPQ->SetCenterPos(handCenterX, handCenterY);
	clockHandPQ->Rotate(CLOCK_CENTER_X, CLOCK_CENTER_Y, -clockSec*PI/30.0f);

	for (i = 0; i < MAX_TIME_SEGMENTS; i++)
	{
		// Make clock flash when there is only ten seconds left.
		if (clockMin == 0 && clockSec <= 10)
		{
			timeSegs[i]->TurnOn(true);
			if (!(clockSec >= i*d + d/2.0f))
			{
				timeSegs[i]->SetColor(COLOR_FLASH);
				if (((int)(clockSec*4))%2 == 0)
					timeSegs[i]->TurnOn(false);
			}
		}
		else
		{
			// Show normal clock.
			if (elapsedInterval == 0.0f)
			{
				timeSegs[i]->TurnOn(clockSec >= i*d + d/2.0f);
				timeSegs[i]->SetColor(COLOR_CLOCK);
			}
			// Show clock with an elapsed time interval.
			else
			{
				if (clockSec >= i*d + d/2.0f)
				{
					timeSegs[i]->TurnOn(true);
					timeSegs[i]->SetColor(COLOR_CLOCK);
				}
				else if (clockSec+elapsedInterval >= i*d + d/2.0f)
				{
					timeSegs[i]->TurnOn(true);
					timeSegs[i]->SetColor(COLOR_ELAPSED);
				}
				else
					timeSegs[i]->TurnOn(false);
			}
		}
	}

	for (i=0; i<MAX_TIME_SEGMENTS; i++)
		timeSegs[i]->Update(dt);
}

//	Draw()
// Sends the widget's quads to NGL.
void AnalogClockWidget::Draw(void)
{
	int i;

	IGOWidget::Draw();

	if (!display)
		return;

	if (!TIMER_IsInfiniteDuration())
	{
		num1PQ->Draw(0);
		num2PQ->Draw(0);
		num3PQ->Draw(0);
		num4PQ->Draw(0);
		num5PQ->Draw(0);
	}
	else
		infPQ->Draw(0);
	
	clockFramePQ->Draw(0);
	clockFacePQ->Draw(0);
	if (!TIMER_IsInfiniteDuration())
		clockHandPQ->Draw(0);

	for (i = 0; i<MAX_TIME_SEGMENTS; i++)
	{
		timeSegs[i]->Draw(0);
	}

	scoreText->Draw();
}

//	SetScore()
// Sets the displayed score.
void AnalogClockWidget::SetScore(const int sc)
{
	if (score != sc)
	{
		score = sc;
		scoreText->changeText(GetScoreText(score));
	}
}

//	ShowElapsedTime()
// Shows an interval of time as "wiping out."
void AnalogClockWidget::ShowElapsedTime(const float t)
{
	elapsedInterval = t;
}

//	HideElapsedTime()
// Hides the special interval display.
void AnalogClockWidget::HideElapsedTime(void)
{
	elapsedInterval = 0.0f;
}

//	GetScoreText()
// Private helper function that returns a formatted score string.
stringx AnalogClockWidget::GetScoreText(const int sc) const
{
	stringx	txt = stringx(sc);

	//if (sc < 100000) txt = "0"+txt;
	//if (sc < 10000)  txt = "0"+txt;
	//if (sc < 1000)   txt = "0"+txt;
	//if (sc < 100)    txt = "0"+txt;
	//if (sc < 10)     txt = "0"+txt;

	return txt;
}