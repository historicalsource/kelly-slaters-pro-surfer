#include "global.h"
#include "igo_widget_skillchallenge.h"

const float SkillChallengeWidget::TIME_FADE = 0.4f;
const float SkillChallengeWidget::TIME_ANIMATE = 0.5f;
const float SkillChallengeWidget::SPEED_HILITE_FLASH = 2.0f;

SkillChallengeWidget::SkillChallengeWidget(int type)
{
	objectRoot = NULL;

	skillText = NEW TextString(NULL, "ICON", 119, 62, 10, 0.72f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));
	pointText = NEW TextString(NULL, "0", 222, 62, 10, 0.9f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));

	fade = 1.0f;
	fadeDir = 1;

	float photo_x = 128;
	if (type == GOAL_SKILL_FACE)
	{
		skillText->changeText("FACE");
		state = STATE_FACE;
	}
	else if (type == GOAL_SKILL_FACE_SCORE)
	{
		skillText->changeText("FACE");
		state = STATE_FACE_SCORE;
	}
	else if (type == GOAL_SKILL_AIR)
	{
		skillText->changeText("AIR");
		state = STATE_AIR;
	}
	else if (type == GOAL_SKILL_AIR_SCORE)
	{
		skillText->changeText("AIR");
		state = STATE_AIR_SCORE;
	}
	else if (type == GOAL_SKILL_TUBE)
	{
		skillText->changeText("TUBE");
		state = STATE_TUBE;
	}
	else if (type == GOAL_SKILL_TUBE_SCORE)
	{
		skillText->changeText("TUBE");
		state = STATE_TUBE_SCORE;
	}
	else if (type == GOAL_PHOTO_1)
	{
		skillText->changeText("PHOTO");
		state = STATE_PHOTO1;
		skillText->changePos(photo_x, 62);
	}
	else if (type == GOAL_PHOTO_2)
	{
		skillText->changeText("PHOTO");
		state = STATE_PHOTO2;
		skillText->changePos(photo_x, 62);
	}
	else if (type == GOAL_PHOTO_3)
	{
		skillText->changeText("PHOTO");
		state = STATE_PHOTO3;
		skillText->changePos(photo_x, 62);
	}
	else
		state = STATE_NONE;
	
	display = false;
	num_photos_taken = 0;
	timer = -1.0f;
}

//	~SkillChallengeWidget()
// Destructor.
SkillChallengeWidget::~SkillChallengeWidget()
{
	//delete objectRoot;
	delete skillText;
	delete pointText;
}

//	SetDisplay()
// Toggles this widget on/off.
void SkillChallengeWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void SkillChallengeWidget::Init(PanelFile & panel, Font * numberfont, Font * textfont, const color32 & textColor1,  const color32 & textColor2)
{
	objectRoot = panel.GetPointer("challengemeter");

	skillText->setFont(textfont);
	skillText->color = textColor2;
	pointText->setFont(numberfont);
	pointText->color = textColor1;

	if (state != STATE_NONE)
		SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void SkillChallengeWidget::Update(const float dt)
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
		skillText->SetFade(fade);
		pointText->SetFade(fade);
	}

	PhotoChallenge *photoChallenge = g_beach_ptr->get_challenges()->photo;
	if ((state == STATE_PHOTO2) || (state == STATE_PHOTO3))
	{
		if (timer >= 0.0f)
			timer += dt;

		int player = g_game_ptr->get_active_player();
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);
		int score = ksctrl->get_my_scoreManager().GetScore();
		int param = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[0];

		bool complete = false;
		int num_taken = photoChallenge->GetNumTaken();
		bool photo_complete = photoChallenge->CheckForCompletion();
		bool score_complete = (score >= param);
		if (photo_complete && score_complete)
			complete = true;
		else if ((num_taken >= NUM_PHOTOS) && !photo_complete)
			complete = true;

		if ((num_taken != num_photos_taken) || (complete && (timer < 0.0f)))
			timer = 0.0f;

		num_photos_taken = num_taken;
		if (num_photos_taken != 0)
		{
			int temp_points = *(photoChallenge->GetPhotoScore(num_photos_taken - 1));
			if (state == STATE_PHOTO2)
			{
				if (temp_points > points)
					points = temp_points;
			}
			else
			{
				if (photoChallenge->GetPhotoIsOfSpecialTrick(num_photos_taken - 1) && (temp_points > points))
					points = temp_points;
			}
		}
		else
			points = 0;

		if (complete && (timer > (3.5f - TIME_FADE)))
		{
			Hide();
		}
		else if (!complete && ((timer > 3.5f) || (timer == -1.0f)))
		{
			timer = -1.0f;
			/*int temp_points;
			for (int n = num_photos_taken - 1; n >= 0; n--)
			{
				temp_points = *(photoChallenge->GetPhotoScore(n));
				if (temp_points > points)
					points = temp_points;
			}*/

			if (!photoChallenge->CheckForCompletion())
				points = 0;
		}

		if (!complete && (fadeDir != 1))
			Show();
		
		stringx s;
		s.printf("%d", points);
		pointText->changeText(s);

	}
	else if (state == STATE_PHOTO1)
	{
		if (timer >= 0.0f)
			timer += dt;

		int num_taken = photoChallenge->GetNumTaken();
		int player = g_game_ptr->get_active_player();
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);
		int score = ksctrl->get_my_scoreManager().GetScore();
		int param = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[0];

		bool complete = ((num_taken >= NUM_PHOTOS) || (photoChallenge->CheckForCompletion() && (score >= param)));
		if ((timer < 0.0f) && complete)
			timer = 0.0f;

		if (timer > 3.0f)
		{
			Hide();
			timer = -1.0f;
		}

		stringx s;
		points = *(photoChallenge->GetPhotoScore(0)) + *(photoChallenge->GetPhotoScore(1))
									+ *(photoChallenge->GetPhotoScore(2));
		s.printf("%d", points);
		pointText->changeText(s);

		if (!complete && (fadeDir != 1))
			Show();
	}
	else if (state != STATE_NONE)
	{
		if (timer >= 0.0f)
			timer += dt;

		int player = g_game_ptr->get_active_player();
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);

		int face, tube, air, ch_points;
		ksctrl->get_my_scoreManager().GetPartialScores(face, air, tube);
		stringx s;
		if ((state == STATE_AIR) || (state == STATE_AIR_SCORE))
			ch_points = air;
		else if ((state == STATE_TUBE) || (state == STATE_TUBE_SCORE))
			ch_points = tube;
		else//(state == STATE_FACE)
			ch_points = face;

		s.printf("%d", ch_points);
		pointText->changeText(s);

		int param = CareerDataArray[g_game_ptr->get_level_id()].goal_param[0];
		if ((state == STATE_AIR_SCORE) || (state == STATE_TUBE_SCORE) || (state == STATE_FACE_SCORE))
			param = CareerDataArray[g_game_ptr->get_level_id()].goal_param_2[0];

		bool goal_done = (ch_points >= param);
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
void SkillChallengeWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	objectRoot->Draw(0);
	skillText->Draw();
	pointText->Draw();
}

//	Hide()
// Deactivates the wave indicator overlay.
void SkillChallengeWidget::Hide(const bool fadeOut)
{
	if (fadeOut)
		fadeDir = -1;
}

//	Show()
// Deactivates the wave indicator overlay.
void SkillChallengeWidget::Show(const bool fadeIn)
{
	if (fadeIn)
		fadeDir = 1;
}