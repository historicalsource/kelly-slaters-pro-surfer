
#include "global.h"
#include "timer.h"
#include "igo_widget_balance.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	HorizBalanceWidget class
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//	HorizBalanceWidget()
// Default constructor.
HorizBalanceWidget::HorizBalanceWidget()
{
	meterPQ = NULL;
	arrowPQ = NULL;
	leftKnobPQ = NULL;
	rightKnobPQ = NULL;
	colorLeftPQ = NULL;
	colorRightPQ = NULL;
	
	flashing = false;
}

//	~HorizBalanceWidget()
// Destructor.
HorizBalanceWidget::~HorizBalanceWidget()
{

}

//	SetDisplay()
// Toggles this widget on/off.
void HorizBalanceWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void HorizBalanceWidget::Init(PanelFile & panel, const int playerIdx)
{
	if (playerIdx == 0)
	{
		meterPQ = panel.GetPointer("balance_1_frame");
		arrowPQ = panel.GetPointer("balance_1_arrow");
		leftKnobPQ = panel.GetPointer("balance_1_circle_left");
		rightKnobPQ = panel.GetPointer("balance_1_circle_right");
		colorLeftPQ = panel.GetPointer("balance_1_color");
		colorRightPQ = panel.GetPointer("balance_1_color2");
	}
	else
	{
		meterPQ = panel.GetPointer("balance_2_frame");
		arrowPQ = panel.GetPointer("balance_2_arrow");
		leftKnobPQ = panel.GetPointer("balance_2_circle_left");
		rightKnobPQ = panel.GetPointer("balance_2_circle_right");
		colorLeftPQ = panel.GetPointer("balance_2_color");
		colorRightPQ = panel.GetPointer("balance_2_color2");
	}
	meterPQ->TurnOn(false);
	arrowPQ->TurnOn(false);
	leftKnobPQ->TurnOn(false);
	rightKnobPQ->TurnOn(false);
	colorLeftPQ->TurnOn(false);
	colorRightPQ->TurnOn(false);

	// Tsk, tsk...
	colorLeftPQ->Mask(0, 1);
	colorRightPQ->Mask(0, 2);

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void HorizBalanceWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	// Make knobs flash.
	if (flashing && 0)
	{
		if (((int) (TIMER_GetTotalSec()*4))%2 == 0)
		{
			leftKnobPQ->SetColor(1.0f, 1.0f, 0.0f, 1.0f);
			rightKnobPQ->SetColor(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else
		{
			leftKnobPQ->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
			rightKnobPQ->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		}
	}

	arrowPQ->Update(dt);
}

//	Draw()
// Sends this widget's quads to NGL.
void HorizBalanceWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;
	
	meterPQ->Draw(0);
	arrowPQ->Draw(0);
	leftKnobPQ->Draw(0);
	rightKnobPQ->Draw(0);
	colorLeftPQ->Draw(0);
	colorRightPQ->Draw(0);
}

//	Show()
//	Makes the balance meter popup.
void HorizBalanceWidget::Show(const bool s)
{
	meterPQ->TurnOn(s);
	arrowPQ->TurnOn(s);
	leftKnobPQ->TurnOn(s);
	rightKnobPQ->TurnOn(s);
	colorLeftPQ->TurnOn(s);
	colorRightPQ->TurnOn(s);

	flashing = false;
}

//	SetArrow()
// Positions the arrow on the meter [-1, 1]
void HorizBalanceWidget::SetArrow(float f)
{
	float	fp = f, r = 1.0f, g = 1.0f;
	float	x, y;

	if (f > 1.0f) f = 1.0f;
	if (f < -1.0f) f = -1.0f;
	if (f < 0.0f) fp = -f;
	
	// Animate arrow.
	arrowPQ->GetCenterPos(x, y);
	arrowPQ->SetCenterPos(x, y);
	arrowPQ->Rotate(meterCenterX, 243, f*PI/6.0f);
	
	// Change knob colors.
	g = 2-2*fp;
	if (fp < .5f)
	{
		r = 2*fp;
		g = 1.0f;
	}
	if (f < 0)
	{
		leftKnobPQ->SetColor(r, g, 0, 1.0f);
		rightKnobPQ->SetColor(0, 1.0f, 0, 1.0f);
		flashing = (f > -.05f);
	}
	else
	{
		leftKnobPQ->SetColor(0, 1.0f, 0, 1.0f);
		rightKnobPQ->SetColor(r, g, 0, 1.0f);
		flashing = (f < .05f);
	}
}

//	SetFillage.
// Sets the depth meter amount.
void HorizBalanceWidget::SetFillage(const float f)
{
	float	mask = f;
	
	// Correct out of range numbers.
	if (mask < 0.0f)
		mask = 0.0f;
	if (mask > 1.0f)
		mask = 1.0f;

	// Mask left and right color bars.
	colorLeftPQ->Mask(mask/2.0f, 1);
	colorRightPQ->Mask(mask/2.0f, 2);
}

//	FitToViewport()
// Repositions the meter onscreen to fit the specified viewport.
void HorizBalanceWidget::FitToViewport(const int x1, const int y1, const int x2, const int y2)
{
	float	meter_height_base = 100.0f;
	float	multiplayer_meter_height_base = 110.0f;
	float	width = x2-x1+1;
	float	cx = x1+width/2.0f;
	float	y_bas_pos = meter_height_base;

	if (g_game_ptr->is_splitscreen())
		y_bas_pos = multiplayer_meter_height_base;  //  multiplayer balance meters need to be a little lower.
	
	meterCenterX = cx;

	meterPQ->SetCenterPos(meterCenterX, y_bas_pos + 30.0f);
	arrowPQ->SetCenterPos(meterCenterX, y_bas_pos);
	leftKnobPQ->SetCenterPos(meterCenterX-65.0f, y_bas_pos + 40.0f);
	rightKnobPQ->SetCenterPos(meterCenterX+65.0f, y_bas_pos + 40.0f);
	colorLeftPQ->SetCenterPos(meterCenterX, y_bas_pos + 26.0f);
	colorRightPQ->SetCenterPos(meterCenterX, y_bas_pos + 26.0f);

	// Clip widget to the specified viewport.
	meterPQ->SetClip(recti(x1, y1, x2, y2));
	arrowPQ->SetClip(recti(x1, y1, x2, y2));
	leftKnobPQ->SetClip(recti(x1, y1, x2, y2));
	rightKnobPQ->SetClip(recti(x1, y1, x2, y2));
	colorLeftPQ->SetClip(recti(x1, y1, x2, y2));
	colorRightPQ->SetClip(recti(x1, y1, x2, y2));
}
