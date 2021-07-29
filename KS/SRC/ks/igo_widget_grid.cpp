
#include "global.h"
#include "igo_widget_grid.h"

// Default constructor.
GridWidget::GridWidget()
{
	int	i;
	
	// Initialize PQs to null.
	for (i = 0; i < NUM_H_LINES; i++)
		hLinePQs[i] = NULL;
	for (i = 0; i < NUM_V_LINES; i++)
		vLinePQs[i] = NULL;
}

// One-time initialization after construction.
void GridWidget::Init(PanelFile & panel)
{
	stringx	name;
	int		i;

	// Get horizontal line PQs from the panel file.
	for (i = 0; i < NUM_H_LINES; i++)
	{
		if (i < 9)
			name = stringx("line_h_0") + stringx(i+1);
		else
			name = stringx("line_h_") + stringx(i+1);
		hLinePQs[i] = panel.GetPointer(name.c_str());
		hLinePQs[i]->TurnOn(false);
	}

	// Get vertical line PQs from the panel file.
	for (i = 0; i < NUM_V_LINES; i++)
	{
		if (i < 9)
			name = stringx("line_results_v_0") + stringx(i+1);
		else
			name = stringx("line_results_v_") + stringx(i+1);
		vLinePQs[i] = panel.GetPointer(name.c_str());
		vLinePQs[i]->TurnOn(false);
	}
}

// Renders the grid's panel quads.
void GridWidget::Draw(void)
{
	int	i;
	
	// Don't draw if display is toggled off.
	if (!display)
		return;

	// Draw grid PQs.
	for (i = 0; i < NUM_H_LINES; i++)
		hLinePQs[i]->Draw(0);
	for (i = 0; i < NUM_V_LINES; i++)
		vLinePQs[i]->Draw(0);
}

// Turns on the specified horizontal lines.
void GridWidget::ShowHLines(const int flags)
{
	int	i;

	// Turn on specified PQs.
	for (i = 0; i < NUM_H_LINES; i++)
	{
		if (flags & (1 << i))
			hLinePQs[i]->TurnOn(true);
	}
}

// Turns on the specified vertical lines.
void GridWidget::ShowVLines(const int flags)
{
	int i;

	// Turn on specified PQs.
	for (i = 0; i < NUM_V_LINES; i++)
	{
		if (flags & (1 << i))
			vLinePQs[i]->TurnOn(true);
	}
}

// Hides all the grid lines.
void GridWidget::Hide(void)
{
	int i;
	
	// Turn off all PQs.
	for (i = 0; i < NUM_H_LINES; i++)
		hLinePQs[i]->TurnOn(false);
	for (i = 0; i < NUM_V_LINES; i++)
		vLinePQs[i]->TurnOn(false);
}