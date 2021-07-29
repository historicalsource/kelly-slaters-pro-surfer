
#include "global.h"
#include "igo_widget.h"

//	IGOWidget()
// Default constructor.
IGOWidget::IGOWidget()
{
	display = true;
}

//	~IGOWidget()
// Destructor.
IGOWidget::~IGOWidget()
{

}


//	SetDisplay()
// Toggles the display of this widget.
void IGOWidget::SetDisplay(const bool d)
{
	display = d;
}


//	Update()
// Called every frame - updates the widget with the time elapsed since the last frame.
void IGOWidget::Update(const float dt)
{

}

//	Draw()
// Sends this widget's quads to NGL.
void IGOWidget::Draw(void)
{

}

//	GetDisplay()
// Returns the display flag for this widget.
bool IGOWidget::GetDisplay(void) const
{
	return display;
}