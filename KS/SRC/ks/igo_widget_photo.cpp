
#include "global.h"
#include "igo_widget_photo.h"

const float PhotoWidget::TIME_SHOWN = 5.0f;	// show picutre onscreen for X seconds
const float PhotoWidget::TIME_FADE_IN = 4.0f;	// time it takes for polaroid to colorize
const float PhotoWidget::TIME_FADE_OUT = 1.0f;	// time it takes for polaroid to fade out

//	PhotoWidget()
// Default constructor.
PhotoWidget::PhotoWidget()
{
	borderPQ = NULL;
	
	photoTexture = NULL;
	shownTimer = 0.0f;
	timed = false;
	fadeOutAlpha = 1.0f;
	photoNum = 0;

	borderCenterX = 437;
	borderCenterY = 265;
	borderZ = 400;

	scorePtr = NULL;
	pointText = NEW TextString(NULL, "", 0, 0, 300, 0.9f, Font::HORIZJUST_CENTER, Font::VERTJUST_BOTTOM, color32(149, 251, 149, 255));

	nglInitQuad(&photoQuad);
	nglSetQuadUV(&photoQuad, 0, 0, 1, 1);
	nglSetQuadColor(&photoQuad, NGL_RGBA32(255, 255, 255, 255));
	nglSetQuadZ(&photoQuad, 300);
	nglSetQuadBlend(&photoQuad, NGLBM_OPAQUE, 0);

	nglInitQuad(&darkQuad);
	nglSetQuadUV(&darkQuad, 0, 0, 1, 1);
	nglSetQuadColor(&darkQuad, NGL_RGBA32(64, 64, 64, FTOI(darkFade*255.0f)));
	nglSetQuadZ(&darkQuad, 250);
	nglSetQuadBlend(&darkQuad, NGLBM_BLEND, 0);
}

//	~PhotoWidget()
// Destructor.
PhotoWidget::~PhotoWidget()
{
	delete pointText;
}

//	SetDisplay()
// Overridden from base class.
void PhotoWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Called well after the constructor.
void PhotoWidget::Init(PanelQuad * pq, Font * font)
{
	float x1, y1, x2, y2;
	
	photoTexture = NULL;
	shownTimer = 0.0f;
	timed = false;
	darkFade = 1.0f;
	fadeOutAlpha = 1.0f;
	
	assert(pq);
	borderPQ = pq;
	borderPQ->GetPos(x1, y1, x2, y2);
	SetPosition(x1, y1, borderPQ->z);

	pointText->setFont(font);
	
	SetDisplay(true);
}

//	Reset()
// Called when level is restarted.
void PhotoWidget::Reset(void)
{
	photoTexture = NULL;
	scorePtr = NULL;
	score = 0;
	photoNum = 0;
	shownTimer = 0.0f;
	darkFade = 1.0f;
	fadeOutAlpha = 1.0f;
	timed = false;
}

//	Update()
// Call every frame - update widget with time elapsed since last frame.
void PhotoWidget::Update(const float dt)
{
	float	fadeTimeRemaining = 0.0f;
	
	IGOWidget::Update(dt);

	// Count time photo is visible.
	if (timed && photoTexture)
	{
		shownTimer -= dt;

		//if (shownTimer < TIME_FADE_OUT)
		//	fadeOutAlpha = shownTimer/TIME_FADE_OUT;
		//else
			fadeOutAlpha = 1.0f;

		// Make photo go away if time runs out.
		if (shownTimer <= 0.0f)
		{
			shownTimer = 0.0f;
			Hide();
		}
	}

	if (scorePtr && (*scorePtr != score))
	{
		score = *scorePtr;
		SetPointText();
	}
	
	if (timed)
	{
		if (shownTimer > 0.0f)
		{
			fadeTimeRemaining = TIME_FADE_IN-(TIME_SHOWN-shownTimer);
			if (fadeTimeRemaining > TIME_FADE_IN) fadeTimeRemaining = TIME_FADE_IN;
			if (fadeTimeRemaining < 0.0f) fadeTimeRemaining = 0.0f;
			
			darkFade = fadeTimeRemaining/TIME_FADE_IN;
		}
	}
}

//	Draw()
// Sends the widget's quads to NGL.
// Should be called every frame.
void PhotoWidget::Draw(void)
{	
	IGOWidget::Draw();
	
	if (!display)
		return;

	if (photoTexture)
	{
		borderPQ->SetCenterPos(borderCenterX, borderCenterY);
		borderPQ->SetZ(borderZ);
		borderPQ->SetFade(fadeOutAlpha);
		borderPQ->Draw(0);

		if (fadeOutAlpha < 1.0f)
		{
			nglSetQuadColor(&photoQuad, NGL_RGBA32(255, 255, 255, 255.0f*fadeOutAlpha));
			nglSetQuadBlend(&photoQuad, NGLBM_BLEND, 0);
		}
		else
		{
			nglSetQuadColor(&photoQuad, NGL_RGBA32(255, 255, 255, 255));
			nglSetQuadBlend(&photoQuad, NGLBM_OPAQUE, 0);
		}
		nglListAddQuad(&photoQuad);
				
		nglSetQuadColor(&darkQuad, NGL_RGBA32(64, 64, 64, FTOI(darkFade*255.0f)));
		nglListAddQuad(&darkQuad);

		pointText->color.set_alpha(255.0f*fadeOutAlpha);
		pointText->Draw();
	}
}

//	Show()
// Fades in the phot widget, displaying the specified texture as the picture.
void PhotoWidget::Show(nglTexture * tex, int * sc, const int num)
{
	photoTexture = tex;
	nglSetQuadMapFlags(&photoQuad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex(&photoQuad, photoTexture);
	darkFade = 1.0f;
	
	timed = true;
	shownTimer = TIME_SHOWN;
	fadeOutAlpha = 1.0f;

	scorePtr = sc;
	if (scorePtr) score = *scorePtr;
	else score = 0;
	photoNum = num;
	SetPointText();
}

//	Show()
// Shows the photo widget with the specified polaroid alpha.
void PhotoWidget::Show(nglTexture * tex, int * sc, const int num, const float fade)
{
	photoTexture = tex;
	nglSetQuadMapFlags(&photoQuad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex(&photoQuad, photoTexture);
	darkFade = fade;
	
	timed = false;
	fadeOutAlpha = 1.0f;

	scorePtr = sc;
	if (scorePtr) score = *scorePtr;
	else score = 0;
	photoNum = num;
	SetPointText();
}

//	Hide()
// Causes the widget to disappear.
void PhotoWidget::Hide(void)
{
	photoTexture = NULL;
	nglSetQuadMapFlags(&photoQuad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex(&photoQuad, photoTexture);

	shownTimer = 0.0f;
	timed = false;
}

//	SetPosition()
// Sets the upper-left positon of the polaroid image.
void PhotoWidget::SetPosition(const int x, const int y, const int z)
{
	float x1, y1, x2, y2;

	// Move panel quad.
	borderPQ->SetPos(x, y);
	borderZ = z;
	borderPQ->SetZ(borderZ);
	
	// Get new position.
	borderPQ->GetCenterPos(x1, y1);
	borderCenterX = int(x1);
	borderCenterY = int(y1);
	borderPQ->GetPos(x1, y1, x2, y2);

	// Move text.
	pointText->changePos((x2-x1)/2+x1, y2-25);
	pointText->changeZ(z-10);
	
	// Move picture quads.
	x1 += 27;
	y1 += 23;
	x2 = x1+128;
	y2 = y1+128;
	adjustCoords(x1, y1);
	adjustCoords(x2, y2);
	nglSetQuadRect(&photoQuad, x1, y1, x2, y2);
	nglSetQuadZ(&photoQuad, z-10);
	nglSetQuadRect(&darkQuad, x1, y1, x2, y2);
	nglSetQuadZ(&darkQuad, z-20);
}


//	SetPointText()
// Private helper function - changes the point text to reflect the score.
void PhotoWidget::SetPointText(void)
{
	stringx	text;

	if (scorePtr)
	{
		text += stringx(score);
		text += stringx(" ")+ksGlobalTextArray[GT_PHOTOSHOOT_POINTS];
		pointText->changeText(text);
	}
	else
		pointText->changeText("");
}