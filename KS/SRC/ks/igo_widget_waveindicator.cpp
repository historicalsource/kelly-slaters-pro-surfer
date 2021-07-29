
#include "global.h"
#include "igo_widget_waveindicator.h"

const float WaveIndicatorWidget::TIME_FADE = 4.0f;
const float WaveIndicatorWidget::TIME_TUTORIAL_FADE = 1.0f;
const float WaveIndicatorWidget::TIME_ANIMATE = 0.5f;
const float WaveIndicatorWidget::SPEED_HILITE_FLASH = 2.0f;

//	WaveIndicatorWidget()
// Default constructor.
WaveIndicatorWidget::WaveIndicatorWidget()
{
	for (int d = 0; d < 2; d++)
	{
		for (int i = 0; i < NUM_WAVES; i++)
			wavePQ[d][i] = NULL;

		for (int i = 0; i < NUM_TONGUES; i++)
			tonguePQ[d][i] = NULL;

		hilitePQ[d][0] = NULL;
		hilitePQ[d][1] = NULL;
		hilitePQ[d][2] = NULL;

		arrowPQ[d] = NULL;
	}
	heightPQ = NULL;

	heightText = NEW TextString(NULL, "13.3", 405, 423, 10, 0.80f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));
	unitText = NEW TextString(NULL, ksGlobalTextArray[GT_IGO_FT], 406, 423, 10, 0.60f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));
	nextHeightText = NEW TextString(NULL, "19.9", 406, 394, 10, 0.65f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));
	nextUnitText = NEW TextString(NULL, ksGlobalTextArray[GT_IGO_FT], 407, 394, 10, 0.50f, Font::HORIZJUST_LEFT, Font::VERTJUST_BOTTOM, color32(255, 255, 255, 255));

	dirIdx = 0;
	waveIdx = 0;
	tongueIdx = 0;

	fade = 1.0f;
	fadeDir = 0;
	state = STATE_NONE;
}

//	~WaveIndicatorWidget()
// Destructor.
WaveIndicatorWidget::~WaveIndicatorWidget()
{
	delete heightText;
	delete unitText;
	delete nextHeightText;
	delete nextUnitText;
}

//	SetDisplay()
// Toggles this widget on/off.
void WaveIndicatorWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Must be called after constructing.
void WaveIndicatorWidget::Init(PanelFile & panel, const bool left, Font * font, const color32 & textColor, const color32 & nextTextColor)
{
	wavePQ[0][0] = panel.GetPointer("wi_wave_L_1");
	wavePQ[0][1] = panel.GetPointer("wi_wave_L_2");
	wavePQ[0][2] = panel.GetPointer("wi_wave_L_3");
	wavePQ[1][0] = panel.GetPointer("wi_wave_R_1");
	wavePQ[1][1] = panel.GetPointer("wi_wave_R_2");
	wavePQ[1][2] = panel.GetPointer("wi_wave_R_3");

	tonguePQ[0][0] = panel.GetPointer("wi_tongue_L_0");
	tonguePQ[0][1] = panel.GetPointer("wi_tongue_L_1");
	tonguePQ[0][2] = panel.GetPointer("wi_tongue_L_2");
	tonguePQ[0][3] = panel.GetPointer("wi_tongue_L_3");
	tonguePQ[0][4] = panel.GetPointer("wi_tongue_L_4");
	tonguePQ[0][5] = panel.GetPointer("wi_tongue_L_5");
	tonguePQ[1][0] = panel.GetPointer("wi_tongue_R_0");
	tonguePQ[1][1] = panel.GetPointer("wi_tongue_R_1");
	tonguePQ[1][2] = panel.GetPointer("wi_tongue_R_2");
	tonguePQ[1][3] = panel.GetPointer("wi_tongue_R_3");
	tonguePQ[1][4] = panel.GetPointer("wi_tongue_R_4");
	tonguePQ[1][5] = panel.GetPointer("wi_tongue_R_5");

	arrowPQ[0] = panel.GetPointer("wi_arrow_left");
	arrowPQ[1] = panel.GetPointer("wi_arrow_right");

	hilitePQ[0][0] = panel.GetPointer("wi_hilite_face_L");
	hilitePQ[0][1] = panel.GetPointer("wi_hilite_lip_L");
	hilitePQ[0][2] = panel.GetPointer("wi_hilite_tube_L");
	hilitePQ[1][0] = panel.GetPointer("wi_hilite_face_R");
	hilitePQ[1][1] = panel.GetPointer("wi_hilite_lip_R");
	hilitePQ[1][2] = panel.GetPointer("wi_hilite_tube_R");

#if !defined(TARGET_PS2)
	
	for (int i = 0; i < NUM_TONGUES; i++)
	{
		tonguePQ[0][i]->getQuad()->Verts[0].X -= 2.0f;
		tonguePQ[0][i]->getQuad()->Verts[2].X -= 2.0f;
	}
	
	for (int i = 0; i < NUM_WAVES; i++)
	{
		wavePQ[1][i]->getQuad()->Verts[0].X -= 2.0f;
		wavePQ[1][i]->getQuad()->Verts[2].X -= 2.0f;
	}

	hilitePQ[1][2]->getQuad()->Verts[0].X -= 2.0f;
	hilitePQ[1][2]->getQuad()->Verts[2].X -= 2.0f;
	hilitePQ[0][0]->getQuad()->Verts[0].X -= 2.0f;
	hilitePQ[0][0]->getQuad()->Verts[2].X -= 2.0f;
	hilitePQ[0][1]->getQuad()->Verts[0].X -= 2.0f;
	hilitePQ[0][1]->getQuad()->Verts[2].X -= 2.0f;

#endif

	heightPQ = panel.GetPointer("wave_height_meter");

	heightText->setFont(font);
	heightText->color = textColor;
	unitText->setFont(font);
	unitText->color = textColor;
	nextHeightText->setFont(font);
	nextHeightText->color = nextTextColor;
	nextUnitText->setFont(font);
	nextUnitText->color = nextTextColor;

	if (left) dirIdx = 0;
	else dirIdx = 1;
	waveIdx = 0;
	tongueIdx = 0;
	hiliteTime = 0.0f;

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void WaveIndicatorWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	if (state != STATE_NONE)
	{
		// Fade overlay in and out.
		if (state == STATE_HILITE_FACE || state == STATE_HILITE_LIP || state == STATE_HILITE_TUBE)
			fade += (dt/TIME_TUTORIAL_FADE)*float(fadeDir);
		else
			fade += (dt/TIME_FADE)*float(fadeDir);
		if (fade >= 1.0f)
			fade = 1.0f;
		if (fade <= 0.0f)
		{
			fade = 0.0f;
			state = STATE_NONE;
		}

		// Animate overlay.
		if (state == STATE_SURGE)
		{
			waveIdx += dt/TIME_ANIMATE;
			if (int(waveIdx) >= NUM_WAVES)
				waveIdx = 0;
		}
		else if (state == STATE_TONGUE)
		{
			tongueIdx += dt/TIME_ANIMATE;
			if (int(tongueIdx) >= NUM_TONGUES)
				tongueIdx = 0;
		}
		
		if (state == STATE_HILITE_FACE || state == STATE_HILITE_LIP || state == STATE_HILITE_TUBE)
			hiliteTime += dt;
	}
}

//	Draw()
// Sends this widget's quads to NGL.
void WaveIndicatorWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;
	
	
	// Directional state draws the wave with an arrow and the wave heigts.
	if (state == STATE_DIR)
	{
		wavePQ[dirIdx][0]->SetFade(fade);
		wavePQ[dirIdx][0]->Draw(0);
		tonguePQ[dirIdx][0]->SetFade(fade);
		tonguePQ[dirIdx][0]->Draw(0);
		
		arrowPQ[dirIdx]->SetFade(fade);
		arrowPQ[dirIdx]->Draw(0);
		heightPQ->SetFade(fade);
		heightPQ->Draw(0);
		
		heightText->SetFade(fade);
		heightText->Draw();
		unitText->SetFade(fade);
		unitText->Draw();
		nextHeightText->SetFade(fade);
		nextHeightText->Draw();
		nextUnitText->SetFade(fade);
		nextUnitText->Draw();
	}
	// Surge state draws the wave only.
	else if (state == STATE_SURGE)
	{
		wavePQ[dirIdx][int(waveIdx)]->SetFade(fade);
		wavePQ[dirIdx][int(waveIdx)]->Draw(0);
		tonguePQ[dirIdx][0]->SetFade(fade);
		tonguePQ[dirIdx][0]->Draw(0);
	}
	// Tongue state draws the wave only.
	else if (state == STATE_TONGUE)
	{
		wavePQ[dirIdx][0]->SetFade(fade);
		wavePQ[dirIdx][0]->Draw(0);
		tonguePQ[dirIdx][int(tongueIdx)]->SetFade(fade);
		tonguePQ[dirIdx][int(tongueIdx)]->Draw(0);
	}
	// No highlight state draws just the tinted wave.
	else if (state == STATE_HILITE_NONE)
	{
		tonguePQ[dirIdx][0]->SetFade(fade);
		tonguePQ[dirIdx][0]->Draw(0);
		wavePQ[dirIdx][0]->SetFade(fade);
		wavePQ[dirIdx][0]->Draw(0);
	}
	// Face highlight state draws just the tinted wave.
	else if (state == STATE_HILITE_FACE)
	{
		if (int(hiliteTime*SPEED_HILITE_FLASH)%2 == 0)
		{
			hilitePQ[dirIdx][0]->SetFade(fade);
			hilitePQ[dirIdx][0]->Draw(0);
		}
		else
		{
			tonguePQ[dirIdx][0]->SetFade(fade);
			tonguePQ[dirIdx][0]->Draw(0);
		}
		wavePQ[dirIdx][0]->SetFade(fade);
		wavePQ[dirIdx][0]->Draw(0);
	}
	// Lip highlight state draws just the tinted wave.
	else if (state == STATE_HILITE_LIP)
	{
		if (int(hiliteTime*SPEED_HILITE_FLASH)%2 == 0)
		{
			hilitePQ[dirIdx][1]->SetFade(fade);
			hilitePQ[dirIdx][1]->Draw(0);
		}
		else
		{
			tonguePQ[dirIdx][0]->SetFade(fade);
			tonguePQ[dirIdx][0]->Draw(0);
		}
		wavePQ[dirIdx][0]->SetFade(fade);
		wavePQ[dirIdx][0]->Draw(0);
	}
	// Tube highlight state draws just the tinted wave.
	else if (state == STATE_HILITE_TUBE)
	{
		if (int(hiliteTime*SPEED_HILITE_FLASH)%2 == 0)
		{
			hilitePQ[dirIdx][2]->SetFade(fade);
			hilitePQ[dirIdx][2]->Draw(0);
		}
		else
		{
			wavePQ[dirIdx][0]->SetFade(fade);
			wavePQ[dirIdx][0]->Draw(0);
		}
		tonguePQ[dirIdx][0]->SetFade(fade);
		tonguePQ[dirIdx][0]->Draw(0);
	}
}

//	ShowDirection()
// Activates the wave direction overlay.
void WaveIndicatorWidget::ShowDirection(const bool fadeIn, const float waveHeight, const float nextWaveHeight)
{
	stringx	s;
	
	if (fadeIn)
	{
		fade = 0.0f;
		fadeDir = 1;
	}
	else
	{
		fade = 1.0f;
		fadeDir = 0;
	}

	// Convert meter heights to feet.
	float wave_scale = 1.0f;
	float next_wave_scale = 1.0f;

	wave_scale = WAVE_GetHeightFudgeFactor(WAVE_GetScheduleIndex());
	next_wave_scale = WAVE_GetHeightFudgeFactor(WAVE_GetNextScheduleIndex());

	s.printf("%.1f", waveHeight*3.28084f*wave_scale);
	heightText->changeText(s);
	s.printf("%.1f", nextWaveHeight*3.28084f*next_wave_scale);
	nextHeightText->changeText(s);

	waveIdx = 0;
	tongueIdx = 0;
	state = STATE_DIR;
}

//	ShowSurge()
// Activates the animated wave surge overlay.
void WaveIndicatorWidget::ShowSurge(const bool fadeIn)
{
	if (fadeIn)
	{
		fade = 0.0f;
		fadeDir = 1;
	}
	else
	{
		fade = 1.0f;
		fadeDir = 0;
	}

	waveIdx = 0;
	tongueIdx = 0;
	state = STATE_SURGE;
}

//	ShowTongue()
// Activates the animated wave tongue overlay.
void WaveIndicatorWidget::ShowTongue(const bool fadeIn)
{
	if (fadeIn)
	{
		fade = 0.0f;
		fadeDir = 1;
	}
	else
	{
		fade = 1.0f;
		fadeDir = 0;
	}

	waveIdx = 0;
	tongueIdx = 0;
	state = STATE_TONGUE;
}

//	ShowHighlight()
// Activates the wave overlay with one section flashing.
// section: 0 = none, 1 = face, 2 = lip, 3 = tube
void WaveIndicatorWidget::ShowHighlight(const int section, const bool fadeIn)
{
	if (fadeIn)
	{
		fade = 0.0f;
		fadeDir = 1;
	}
	else
	{
		fade = 1.0f;
		fadeDir = 0;
	}

	waveIdx = 0;
	tongueIdx = 0;
	hiliteTime = 0.0f;
	if (section == 0)
		state = STATE_HILITE_NONE;
	else if (section == 1)
		state = STATE_HILITE_FACE;
	else if (section == 2)
		state = STATE_HILITE_LIP;
	else
		state = STATE_HILITE_TUBE;
}

//	Hide()
// Deactivates the wave indicator overlay.
void WaveIndicatorWidget::Hide(const bool fadeOut)
{
	if (fadeOut)
		fadeDir = -1;
	else
		state = STATE_NONE;
}