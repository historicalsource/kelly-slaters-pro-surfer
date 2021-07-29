#include "global.h"
#include "igo_widget_iconcount.h"

const float IconCountWidget::TIME_FADE = 0.3f;
const float IconCountWidget::TIME_ANIMATE = 0.5f;
const float IconCountWidget::SPEED_HILITE_FLASH = 2.0f;

IconCountWidget::IconCountWidget(int type)
{
	objectRoot = NULL;

	iconText = NEW TextString(NULL, "ICON", 214, 62, 10, 0.72f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));
	iconCountText = NEW TextString(NULL, "0", 288, 62, 10, 0.9f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));

	fade = 1.0f;
	fadeDir = 1;

	if (type == GOAL_SKILL_360_SPIN)
	{
		iconText->changeText("SPIN");
		state = STATE_360_SPIN;
	}
	else if (type == GOAL_SKILL_360_SPIN_SCORE)
	{
		iconText->changeText("SPIN");
		state = STATE_360_SPIN_SCORE;
	}
	else if (type == GOAL_SKILL_540_SPIN)
	{
		iconText->changeText("SPIN");
		state = STATE_540_SPIN;
	}
	else if (type == GOAL_SKILL_540_SPIN_SCORE)
	{
		iconText->changeText("SPIN");
		state = STATE_540_SPIN_SCORE;
	}
	else if (type == GOAL_ICON_TETRIS)
		state = STATE_ICON;
	else
		state = STATE_NONE;

	display = false;
	timer = -1.0f;
}

//	~IconCountWidget()
// Destructor.
IconCountWidget::~IconCountWidget()
{
	// cannot delete this because it is part of the PanelFile -beth
//	delete objectRoot;
	delete iconText;
	delete iconCountText;
}

//	SetDisplay()
// Toggles this widget on/off.
void IconCountWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void IconCountWidget::Init(PanelFile & panel, Font * numberfont, Font * textfont, const color32 & textColor1,  const color32 & textColor2)
{
	objectRoot = panel.GetPointer("iconmeter");

	iconText->setFont(textfont);
	iconText->color = textColor2;
	iconCountText->setFont(numberfont);
	iconCountText->color = textColor1;

	if (state != STATE_NONE)
		SetDisplay(true);

	num_icons = 0;
	num_spins = 0;
}

//float g_icon_text_x = 275;
//	Update()
// Called every frame with the time elapsed since the previous frame.
void IconCountWidget::Update(const float dt)
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
		iconText->SetFade(fade);
		iconCountText->SetFade(fade);
	}

	if (state == STATE_ICON)
	{
		if (timer >= 0.0f)
			timer += dt;

		//iconCountText->changePos(g_icon_text_x, 62);
		bool failed = frontendmanager.IGO->GetIconManager()->Failed();
		int param = CareerDataArray[g_game_ptr->get_level_id()].goal_param[0];
		bool goal_done = (num_icons >= param);
		int icons = frontendmanager.IGO->GetIconManager()->IconsCleared();
		if (icons != num_icons)
		{
			if (icons > 9999)
				icons = 9999;

			stringx s;
			s.printf("%d", icons);
			iconCountText->changeText(s);
			num_icons = icons;
		}

		if ((timer < 0.0f) && (fadeDir != -1) && (failed || goal_done))
			timer = 0.0f;

		if (timer > 3.0f)
		{
			Hide();
			timer = -1.0f;
		}

		if ((fadeDir != 1) && !(failed || goal_done))
			Show();
	}
	else if ((state == STATE_360_SPIN) || (state == STATE_360_SPIN_SCORE))
	{
		if (timer >= 0.0f)
			timer += dt;

		int player = g_game_ptr->get_active_player();
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);
		int spins = ksctrl->get_my_scoreManager().Get360Spins();
		if (spins != num_spins)
		{
			stringx s;
			s.printf("%d", spins);
			iconCountText->changeText(s);
			num_spins = spins;
		}

		int param = CareerDataArray[g_game_ptr->get_level_id()].goal_param[0];
		if (state == STATE_360_SPIN_SCORE)
			param = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[0];

		bool goal_done = (num_spins >= param);
		if ((timer < 0.0f) && goal_done && (fadeDir != -1))
			timer = 0.0f;

		if (timer > 3.0f)
		{
			Hide();
			timer = -1.0f;
		}

		if (!goal_done && (fadeDir != 1))
			Show();
	}
	else if ((state == STATE_540_SPIN) || (state == STATE_540_SPIN_SCORE))
	{
		if (timer >= 0.0f)
			timer += dt;

		int player = g_game_ptr->get_active_player();
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);
		int spins = ksctrl->get_my_scoreManager().Get540Spins();
		if (spins != num_spins)
		{
			stringx s;
			s.printf("%d", spins);
			iconCountText->changeText(s);
			num_spins = spins;
		}

		int param = CareerDataArray[g_game_ptr->get_level_id()].goal_param[0];
		if (state == STATE_540_SPIN_SCORE)
			param = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[0];

		bool goal_done = (num_spins >= param);
		if ((timer < 0.0f) && goal_done && (fadeDir != -1))
			timer = 0.0f;

		if (timer > 3.0f)
		{
			Hide();
			timer = -1.0f;
		}

		if (!goal_done && (fadeDir != 1))
			Show();
	}
}

//	Draw()
// Sends this widget's quads to NGL.
void IconCountWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	objectRoot->Draw(0);
	iconText->Draw();
	iconCountText->Draw();
}

//	Hide()
// Deactivates the wave indicator overlay.
void IconCountWidget::Hide(const bool fadeOut)
{
	if (fadeOut)
		fadeDir = -1;
}

//	Show()
// Deactivates the wave indicator overlay.
void IconCountWidget::Show(const bool fadeIn)
{
	if (fadeIn)
		fadeDir = 1;
}