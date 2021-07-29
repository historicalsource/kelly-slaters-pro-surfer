
#ifndef INCLUDED_IGO_WIDGET_GRID_H
#define INCLUDED_IGO_WIDGET_GRID_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// GridWidget - the onscreen horizontal and vertical lines shown in some end run screens.
class GridWidget : public IGOWidget
{
public:
	static const int H00 = 0x00001;
	static const int H01 = 0x00002;
	static const int H02 = 0x00004;
	static const int H03 = 0x00008;
	static const int H04 = 0x00010;
	static const int H05 = 0x00020;
	static const int H06 = 0x00040;
	static const int H07 = 0x00080;
	static const int H08 = 0x00100;
	static const int H09 = 0x00200;
	static const int H10 = 0x00400;
	static const int H11 = 0x00800;
	static const int H12 = 0x01000;
	static const int H13 = 0x02000;
	static const int H14 = 0x04000;

	static const int V00 = 0x00001;
	static const int V01 = 0x00002;
	static const int V02 = 0x00004;
	static const int V03 = 0x00008;

	enum { NUM_H_LINES = 15 };
	enum { NUM_V_LINES = 4 };

private:
	PanelQuad *	hLinePQs[NUM_H_LINES];
	PanelQuad * vLinePQs[NUM_V_LINES];

public:
	// Creators.
	GridWidget();

	// Modifiers.
	virtual void Init(PanelFile & panel);
	virtual void Draw(void);
	virtual void ShowHLines(const int flags);
	virtual void ShowVLines(const int flags);
	virtual void Hide(void);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_GRID_H