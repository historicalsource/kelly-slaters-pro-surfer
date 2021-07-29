
#include "global.h"
#include "igo_widget_splitscore.h"

//	SplitScoreWidget()
// Default constructor.
SplitScoreWidget::SplitScoreWidget()
{
	bgPQ = NULL;
	scoreText = NEW TextString(NULL, NULL, 0, 0, 0, 1.0f, Font::HORIZJUST_RIGHT, Font::VERTJUST_BOTTOM, color32(0, 255, 255, 255));
}

//	~SplitScoreWidget()
// Destructor.
SplitScoreWidget::~SplitScoreWidget()
{
	delete scoreText;
}

//	SetDisplay()
// Toggles this widget on/off.
void SplitScoreWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);

	bgPQ->TurnOn(display);
}

//	Init()
// Must be called after constructing.
void SplitScoreWidget::Init(PanelFile & panel, Font * font, const bool left)
{
	scoreText->setFont(font);

	if (left)
	{
		bgPQ = panel.GetPointer("left_counter");
		scoreText->changePos(213, 66);
	}
	else
	{
		bgPQ = panel.GetPointer("right_counter");
		scoreText->changePos(529, 66);
	}
	bgPQ->TurnOn(true);

	SetScore(0);

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void SplitScoreWidget::Update(const float dt)
{
	IGOWidget::Update(dt);
}

//	Draw()
// Sends this widget's quads to NGL.
void SplitScoreWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	bgPQ->Draw(0);
	scoreText->Draw();
}

//	SetScore()
// Changes the displayed score.
void SplitScoreWidget::SetScore(const int score)
{
	stringx	sc = stringx(score);

	//if (score < 100000) sc = "0"+sc;
	//if (score < 10000)  sc = "0"+sc;
	//if (score < 1000)   sc = "0"+sc;
	//if (score < 100)    sc = "0"+sc;
	//if (score < 10)     sc = "0"+sc;
	
	scoreText->changeText(sc);
}