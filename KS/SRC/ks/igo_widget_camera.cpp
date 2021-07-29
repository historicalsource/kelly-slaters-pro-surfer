
#include "global.h"
#include "igo_widget_camera.h"
#include "SoundScript.h"
#include "Sounddata.h"
//	CameraWidget()
// Default constructor.
CameraWidget::CameraWidget()
{
	int	i;

	for (i = 0; i < NUM_RETICLE_PQS; i++)
		reticlePQs[i] = NULL;

	fade = 1.0f;
	showTimer = 0.0f;
	showTime = 0.0f;
}

//	~CameraWidget()
// Destructor.
CameraWidget::~CameraWidget()
{

}

//	SetDisplay()
// Overridden from base class.
void CameraWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Called well after the constructor.
void CameraWidget::Init(PanelFile & panel)
{
	int	i = 0;
	
	reticlePQs[i++] = panel.GetPointer("igo_camera_count_0");
	reticlePQs[i++] = panel.GetPointer("igo_camera_count_1");
	reticlePQs[i++] = panel.GetPointer("igo_camera_count_2");
	reticlePQs[i++] = panel.GetPointer("igo_camera_count_3");
	reticlePQs[i++] = panel.GetPointer("igo_camera_1a");
	reticlePQs[i++] = panel.GetPointer("igo_camera_1b");
	reticlePQs[i++] = panel.GetPointer("igo_camera_1c");
	reticlePQs[i++] = panel.GetPointer("igo_camera_1d");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2a");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2b");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2c");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2d");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2e");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2f");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2g");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2h");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2i");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2j");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2k");
	reticlePQs[i++] = panel.GetPointer("igo_camera_2l");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3a");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3b");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3c");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3d");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3e");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3f");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3g");
	reticlePQs[i++] = panel.GetPointer("igo_camera_3h");
	assert(i == NUM_RETICLE_PQS);

	for (i = 0; i < NUM_RETICLE_PQS; i++)
		reticlePQs[i]->TurnOn(false);

	fade = 1.0f;

	SetDisplay(true);
}

//	Reset()
// Resets the camera widget.
void CameraWidget::Reset(void)
{
	Hide();
}

//	Update()
// Call every frame - update widget with time elapsed since last frame.
void CameraWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	if (showTimer > 0.0f)
	{
		showTimer -= dt;
		if (showTimer < 0.0f) showTimer = 0.0f;

		FadeReticle();

		if (showTimer == 0.0f)
			Hide();
	}
}

//	Draw()
// Sends the widget's quads to NGL.
// Should be called every frame.
void CameraWidget::Draw(void)
{
	int	i;

	IGOWidget::Draw();

	if (!display)
		return;

	for (i = 0; i < NUM_RETICLE_PQS; i++)
		reticlePQs[i]->Draw(0);
}

/*
//	Show()
// Enables the camera reticle.
void CameraWidget::Show(const float fadeAmt)
{
	int	i;
	
	fade = fadeAmt;
	
	for (i = 0; i < NUM_RETICLE_PQS; i++)
	{
		reticlePQs[i]->TurnOn(true);
		reticlePQs[i]->SetFade(fadeAmt);
	}
}
*/

//	Show()
// Enables the camera reticle.
void CameraWidget::Show(const float time)
{
	showTimer = time;
	showTime = time;

	FadeReticle();
}

//	Hide()
// Makes the camera reticle go away instantly.
void CameraWidget::Hide(void)
{
	int	i;
	
	for (i = 0; i < NUM_RETICLE_PQS; i++)
		reticlePQs[i]->TurnOn(false);

	showTimer = 0.0f;
	showTime = 0.0f;
}

//	FadeReticle()
// Private helper function that makes the reticle fade in as time elapses.
void CameraWidget::FadeReticle(void)
{
	const static float TIME_STAGE_1 = 0.2f;
	const static float TIME_STAGE_2 = TIME_STAGE_1 + 1.0f;
	const static float TIME_STAGE_3 = TIME_STAGE_2 + 1.0f;
	const static float TIME_STAGE_4 = TIME_STAGE_3 + 1.0f;
	int	i;
	static int stage = 4;

	fade = (showTime-showTimer)/showTime;

	if (showTimer > TIME_STAGE_4)
		stage = 4;

	for (i = 0; i < NUM_RETICLE_PQS; i++)
	{
		if (i == 0)
		{
			if (showTimer <= TIME_STAGE_1) 
			{
				if (stage == 1)
				{
					SoundScriptManager::inst()->playEvent(SS_CAMERA_LIGHT4);
					stage -- ;
				} 
				reticlePQs[i]->SetColor(color(0.0f, 1.0f, 0.0f, 1.0f));
			}
			else reticlePQs[i]->SetColor(color(1.0f, 1.0f, 1.0f, 1.0f));
			
		}
		else if (i == 1)
		{
			if (showTimer <= TIME_STAGE_2) 
			{
				if (stage == 2)
				{
					SoundScriptManager::inst()->playEvent(SS_CAMERA_LIGHT3);
					stage--;
				}
				
				reticlePQs[i]->SetColor(color(0.79296875f, 0.17578125f, 0.17578125f, 1.0f));
			}
			else reticlePQs[i]->SetColor(color(1.0f, 1.0f, 1.0f, 1.0f));
		}
		else if (i == 2)
		{
			if (showTimer <= TIME_STAGE_3)
			{
				if (stage == 3)
				{
					SoundScriptManager::inst()->playEvent(SS_CAMERA_LIGHT2);
					stage--;
				}
				reticlePQs[i]->SetColor(color(0.79296875f, 0.17578125f, 0.17578125f, 1.0f));
			}
			else reticlePQs[i]->SetColor(color(1.0f, 1.0f, 1.0f, 1.0f));
			
		}
		else if (i == 3)
		{
			if (showTimer <= TIME_STAGE_4)
			{
				if (stage == 4)
				{
					SoundScriptManager::inst()->playEvent(SS_CAMERA_LIGHT1);

					stage--;
				}
				reticlePQs[i]->SetColor(color(0.79296875f, 0.17578125f, 0.17578125f, 1.0f));
			}
			else reticlePQs[i]->SetColor(color(1.0f, 1.0f, 1.0f, 1.0f));
		}
		
		reticlePQs[i]->TurnOn(true);
		reticlePQs[i]->SetFade(fade);
	}
}