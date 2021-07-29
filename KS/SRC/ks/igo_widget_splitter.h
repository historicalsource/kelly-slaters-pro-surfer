
#ifndef INCLUDED_IGO_WIDGET_SPLITTER_H
#define INCLUDED_IGO_WIDGET_SPLITTER_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// SplitterWidget - vertical bar between players in Push mode
class SplitterWidget : public IGOWidget
{
private:

private:
	PanelQuad *		barPQ;
	
public:
	// Creators.
	SplitterWidget();
	virtual ~SplitterWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void SetPosition(const int x);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_SPLITTER_H