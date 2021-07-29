
#include "global.h"
#include "igo_widget_splitclock.h"

//	SplitClockWidget()
// Default constructor.
SplitClockWidget::SplitClockWidget()
{
	bgPQ = NULL;
	timeText = NEW TextString(NULL, NULL, 320, 63, 0, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(255, 255, 102, 255));

	//clockSec = TIMER_GetLevelSec();
	//clockMin = (int) (clockSec / 60);
	//clockSec -= clockMin * 60;
	clockSec = 0.0f;
	clockMin = 0;
}

//	~SplitClockWidget()
// Destructor.
SplitClockWidget::~SplitClockWidget()
{
	delete timeText;
}

//	SetDisplay()
// Toggles this widget on/off.
void SplitClockWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);

	bgPQ->TurnOn(display);
}

//	Init()
// Must be called after constructing.
void SplitClockWidget::Init(PanelFile & panel, Font * font)
{
	timeText->setFont(font);
	bgPQ = panel.GetPointer("clock");
	bgPQ->TurnOn(true);

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void SplitClockWidget::Update(const float dt)
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
void SplitClockWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	bgPQ->Draw(0);
	timeText->Draw();
}

//	SetTime()
// Sets the seconds and minutes of the clock.
void SplitClockWidget::SetTime(const int newClockMin, const float newClockSec)
{
	if (newClockSec != clockSec || newClockMin != clockMin)
	{
		clockSec = newClockSec;
		clockMin = newClockMin;
		timeText->changeText(GetTimeText(clockMin, clockSec));
	}
}

// GetTimeText()
// Private helper function - converts minutes and seconds into a pretty stringx.
stringx SplitClockWidget::GetTimeText(const int min, const float sec) const
{
	stringx	clockText((int)sec);
		
	if (sec < 10)
		clockText = stringx("0") + clockText;
	clockText = stringx(min) + stringx(":") + clockText;

	return clockText;
}
