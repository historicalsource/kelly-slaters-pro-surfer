
#include "global.h"
#include "igo_widget_simple.h"

//	SimpleWidget()
// Default constructor.
SimpleWidget::SimpleWidget(const int size)
{
	numPQs = size;
	pqs = NEW PanelQuad *[numPQs];
	for (int i = 0; i < numPQs; i++)
		pqs[i] = NULL;
}

//	~SimpleWidget()
// Destructor.
SimpleWidget::~SimpleWidget()
{
	delete [] pqs;
}

//	Init()
// One-time initialization for the widget.
// The size of the specified array must match that passed into the constructor.
void SimpleWidget::Init(PanelFile & panel, const stringx * pqNames)
{
	for (int i = 0; i < numPQs; i++)
		pqs[i] = panel.GetPointer(pqNames[i].c_str());

	Show(false);
}

//	Draw()
// Renders the widget.
void SimpleWidget::Draw(void)
{
	if (!display)
		return;

	// Draw each panel quad.
	for (int i = 0; i < numPQs; i++)
		pqs[i]->Draw(0);
}

//	Show()
// Toggles the widget on or off.
void SimpleWidget::Show(const bool on)
{
	for (int i = 0; i < numPQs; i++)
		pqs[i]->TurnOn(on);
}

//	IsShown()
// Returns true if the widget is toggled on.
bool SimpleWidget::IsShown(void) const
{
	return numPQs > 0 && pqs[0]->IsOn();
}