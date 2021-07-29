
#include "global.h"
#include "igo_widget_splitter.h"

//	SplitterWidget()
// Default constructor.
SplitterWidget::SplitterWidget()
{
	barPQ = NULL;
}

//	~SplitterWidget()
// Destructor.
SplitterWidget::~SplitterWidget()
{

}

//	SetDisplay()
// Overridden from base class.
void SplitterWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Called well after the constructor.
void SplitterWidget::Init(PanelFile & panel)
{
	barPQ = panel.GetPointer("divider line");
	barPQ->TurnOn(true);

	SetDisplay(true);
}

//	Update()
// Call every frame - update widget with time elapsed since last frame.
void SplitterWidget::Update(const float dt)
{
	IGOWidget::Update(dt);
}

//	Draw()
// Sends the widget's quads to NGL.
void SplitterWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	barPQ->Draw(0);
}

//	SetPosition()
// Sets the vertical line's position.
void SplitterWidget::SetPosition(const int x)
{
	barPQ->SetCenterX(x);

	if (x <= 5 || x >= 635)
		barPQ->TurnOn(false);
	else
		barPQ->TurnOn(true);
}