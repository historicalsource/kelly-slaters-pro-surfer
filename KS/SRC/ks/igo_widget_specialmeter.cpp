
#include "global.h"
#include "igo_widget_specialmeter.h"

const color SpecialMeterWidget::COLOR_NORMAL = color(0, 1, 0, 1);
const color SpecialMeterWidget::COLOR_SPECIAL = color(0.9686f, 0.9725f, 0.4901f, 1);
const float SpecialMeterWidget::SPEED_COLOR_FLASH = 2.0f;
const float SpecialMeterWidget::FLASH_MIN = 0.5f;
const float SpecialMeterWidget::FLASH_MAX = 1.4f;

//	SpecialMeterWidget()
// Default constructor.
SpecialMeterWidget::SpecialMeterWidget()
{
	bgPQ = NULL;
	colorPQ = NULL;
	fgPQ = NULL;

	meter = NULL;

	flashDir = 1;
	flashAmt = 1.0f;
}

//	~SpecialMeterWidget()
// Destructor.
SpecialMeterWidget::~SpecialMeterWidget()
{

}

//	SetDisplay()
// Toggles this widget on/off.
void SpecialMeterWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void SpecialMeterWidget::Init(PanelFile & panel, SpecialMeter * specialMeter)
{
	meter = specialMeter;
	meter->SetFillage(0.0f);
	
	bgPQ = panel.GetPointer("special_meter_1_bkg");
	colorPQ = panel.GetPointer("special_meter_1_color");
	fgPQ = panel.GetPointer("special_meter_1");
	bgPQ->TurnOn(true);
	colorPQ->TurnOn(true);
	fgPQ->TurnOn(true);

	SetDisplay(true);

	flashDir = 1;
	flashAmt = 1.0f;
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void SpecialMeterWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	// Flash special meter color.
	if (flashDir > 0)
		flashAmt += dt*SPEED_COLOR_FLASH;
	else
		flashAmt -= dt*SPEED_COLOR_FLASH;
	if (flashAmt > FLASH_MAX)
	{
		flashDir = -flashDir;
		flashAmt = FLASH_MAX;
	}
	else if (flashAmt < FLASH_MIN)
	{
		flashDir = -flashDir;
		flashAmt = FLASH_MIN;
	}

	if (meter) SetFillage(meter->GetFillage());
}

//	Draw()
// Sends this widget's quads to NGL.
void SpecialMeterWidget::Draw(void)
{
	color	meterColor;
	
	IGOWidget::Draw();

	if (!display)
		return;

	// Calculate meter's color.
	if (!meter->CanRegionLink())
		meterColor = COLOR_NORMAL;
	else
	{
		meterColor.r = COLOR_SPECIAL.r*flashAmt;
		meterColor.g = COLOR_SPECIAL.g*flashAmt;
		meterColor.b = COLOR_SPECIAL.b*flashAmt;
		meterColor.a = COLOR_SPECIAL.a;

		if (meterColor.r > 1.0f) meterColor.r = 1.0f;
		if (meterColor.g > 1.0f) meterColor.g = 1.0f;
		if (meterColor.b > 1.0f) meterColor.b = 1.0f;
	}

	// Set meter's color.
	colorPQ->SetColor(meterColor);

	// Draw components.
	bgPQ->Draw(0);
	colorPQ->Draw(0);
	fgPQ->Draw(0);
}

//	SetFillage()
// Sets the percentage amout that meter is filled.
void SpecialMeterWidget::SetFillage(const float amt)
{
	// Set filled amount.
	colorPQ->Mask(amt, true);
}