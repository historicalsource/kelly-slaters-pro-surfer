
#ifndef INCLUDED_IGO_WIDGET_SIMPLE_H
#define INCLUDED_IGO_WIDGET_SIMPLE_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// SimpleWidget - an onscreen widget with a variable number of panel quads that
// can be toggled on and off.
class SimpleWidget : public IGOWidget
{
private:
	int				numPQs;
	PanelQuad **	pqs;

public:
	// Creators.
	SimpleWidget(const int size);
	virtual ~SimpleWidget();

	// Modifiers.
	virtual void Init(PanelFile & panel, const stringx * pqNames);
	virtual void Draw(void);
	virtual void Show(const bool on = true);

	// Accessors.
	virtual bool IsShown(void) const;
};

#endif INCLUDED_IGO_WIDGET_SIMPLE_H