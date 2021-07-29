
#include "global.h"
#include "igo_widget_splitmeter.h"

const color SplitMeterWidget::COLOR_NORMAL = color(0, 1, 0, 1);
const color SplitMeterWidget::COLOR_SPECIAL = color(0.9686f, 0.9725f, 0.4901f, 1);

//	SplitMeterWidget()
// Default constructor.
SplitMeterWidget::SplitMeterWidget()
{
	for (int i = 0; i < NUM_FRAMES; i++)
		framePQs[i] = NULL;
	
	colorPQ = NULL;
	meter = NULL;
}

//	~SplitMeterWidget()
// Destructor.
SplitMeterWidget::~SplitMeterWidget()
{

}

//	SetDisplay()
// Toggles this widget on/off.
void SplitMeterWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void SplitMeterWidget::Init(PanelFile & panel, SpecialMeter * specialMeter, const bool left)
{
	meter = specialMeter;
	meter->SetFillage(0.0f);
	
	if (left)
	{
		framePQs[0] = panel.GetPointer("left_meter_01");
		framePQs[1] = panel.GetPointer("left_meter_03");
		framePQs[2] = panel.GetPointer("left_meter_04");
		colorPQ = panel.GetPointer("left_meter_02");
	}
	else
	{
		framePQs[0] = panel.GetPointer("right_meter_01");
		framePQs[1] = panel.GetPointer("right_meter_03");
		framePQs[2] = panel.GetPointer("right_meter_04");
		colorPQ = panel.GetPointer("right_meter_02");
	}

	for (int i = 0; i < NUM_FRAMES; i++)
		framePQs[i]->TurnOn(true);
	colorPQ->TurnOn(true);

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void SplitMeterWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	if (meter) SetFillage(meter->GetFillage());
}

//	Draw()
// Sends this widget's quads to NGL.
void SplitMeterWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	for (int i = 0; i < NUM_FRAMES; i++)
		framePQs[i]->Draw(0);
	colorPQ->Draw(0);
}

//	SetFillage()
// Sets the percentage amout that meter is filled.
void SplitMeterWidget::SetFillage(const float amt)
{
	// Meter color changes if it is enabled.
	if (!meter->CanRegionLink())
		colorPQ->SetColor(COLOR_NORMAL);
	else
		colorPQ->SetColor(COLOR_SPECIAL);
	
	// Set filled amount.
	colorPQ->Mask(amt, true);
}