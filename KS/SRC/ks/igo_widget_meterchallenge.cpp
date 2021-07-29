#include "global.h"
#include "igo_widget_meterchallenge.h"

const float MeterChallengeWidget::TIME_FADE = 0.30f;
const float MeterChallengeWidget::TIME_ANIMATE = 0.5f;
const float MeterChallengeWidget::SPEED_HILITE_FLASH = 2.0f;

MeterChallengeWidget::MeterChallengeWidget()
{
	objectRoot = NULL;

	Text = NEW TextString(NULL, "0", 516, 104, 10, 0.68f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));

	fade = 0.0f;
	fadeDir = -1;

	display = false;
}

//	~SkillChallengeWidget()
// Destructor.
MeterChallengeWidget::~MeterChallengeWidget()
{
//	delete objectRoot;
	delete Text;
}

//	SetDisplay()
// Toggles this widget on/off.
void MeterChallengeWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
	display = d;
}

//	Init()
// Must be called after constructing.
void MeterChallengeWidget::Init(PanelFile & panel, Font * numberfont, Font * textfont, const color32 & textColor1,  const color32 & textColor2)
{
	objectRoot = panel.GetPointer("specialmetertimer");

	Text->setFont(numberfont);
	Text->color = textColor1;

	SetDisplay(true);
	Hide();
	Update(0.0f);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void MeterChallengeWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	// Fade overlay in and out.
	fade += (dt/TIME_FADE)*float(fadeDir);
	if (fade >= 1.0f)
		fade = 1.0f;
	else if (fade <= 0.0f)
	{
		fade = 0.0f;
	}

	if ((fade <= 1.0f) && (fade >= 0.0f))
	{
		objectRoot->SetFade(fade);
		Text->SetFade(fade);
	}

	int player = g_game_ptr->get_active_player();
	kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);
	bool display_meter = ksctrl->get_special_meter()->CanRegionLink();
	if (display_meter && (fade <= 1.0f))
		fadeDir = 1;
	else if (!display_meter)
		fadeDir = -1;

	if (fade != 0.0f)
	{
		float time = ksctrl->get_special_meter()->GetCurrentSpecialTime();
		stringx s;
		s.printf("%.1f", time);
		Text->changeText(s);
	}
}

//	Draw()
// Sends this widget's quads to NGL.
void MeterChallengeWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	objectRoot->Draw(0);
	Text->Draw();
}

//	Hide()
// Deactivates the wave indicator overlay.
void MeterChallengeWidget::Hide(const bool fadeOut)
{
	if (fadeOut)
		fadeDir = -1;
}

//	Show()
// Deactivates the wave indicator overlay.
void MeterChallengeWidget::Show(const bool fadeIn)
{
	if (fadeIn)
		fadeDir = 1;
}